#include "anyrtmpull.h"
#include "srs_librtmp.h"
#include "webrtc/base/logging.h"
#ifndef _WIN32
# define ERROR_SUCCESS 0
#endif
#define MAX_RETRY_TIME 3
static u_int8_t fresh_nalu_header[] = { 0x00, 0x00, 0x00, 0x01 };
static u_int8_t cont_nalu_header[]  = { 0x00, 0x00, 0x01 };
AnyRtmpPull::AnyRtmpPull(AnyRtmpPullCallback& callback, const std::string& url)
    : callback_(callback),
    srs_codec_(NULL),
    running_(false),
    connected_(false),
    retry_ct_(0),
    rtmp_status_(RS_PLY_Init),
    rtmp_(NULL),
    audio_payload_(NULL),
    video_payload_(NULL)
{
    str_url_       = url;
    rtmp_          = srs_rtmp_create(url.c_str());
    srs_codec_     = new SrsAvcAacCodec();
    audio_payload_ = new DemuxData(1024);
    video_payload_ = new DemuxData(384 * 1024);
    running_       = true;
    rtc::Thread::Start();
}

AnyRtmpPull::~AnyRtmpPull(void)
{
    running_     = false;
    rtmp_status_ = RS_PLY_Closed;
    rtc::Thread::SleepMs(100);
    {
        rtc::CritScope l(&cs_rtmp_);
        if (rtmp_) {
            srs_rtmp_disconnect_server(rtmp_);
        }
    }
    rtc::Thread::Stop();
    if (rtmp_) {
        srs_rtmp_destroy(rtmp_);
        rtmp_ = NULL;
    }
    if (srs_codec_) {
        delete srs_codec_;
        srs_codec_ = NULL;
    }
    if (audio_payload_) {
        delete audio_payload_;
        audio_payload_ = NULL;
    }
    if (video_payload_) {
        delete video_payload_;
        video_payload_ = NULL;
    }
}

void AnyRtmpPull::Run()
{
    while (running_) {
        {
            this->ProcessMessages(10);
        }
        if (rtmp_ != NULL) {
            switch (rtmp_status_) {
                case RS_PLY_Init: {
                    if (srs_rtmp_handshake(rtmp_) == 0) {
                        srs_human_trace("SRS: simple handshake ok.");
                        rtmp_status_ = RS_PLY_Handshaked;
                    } else {
                        CallDisconnect();
                    }
                }
                break;
                case RS_PLY_Handshaked: {
                    if (srs_rtmp_connect_app(rtmp_) == 0) {
                        srs_human_trace("SRS: connect vhost/app ok.");
                        rtmp_status_ = RS_PLY_Connected;
                    } else {
                        CallDisconnect();
                    }
                }
                break;
                case RS_PLY_Connected: {
                    if (srs_rtmp_play_stream(rtmp_) == 0) {
                        srs_human_trace("SRS: play stream ok.");
                        rtmp_status_ = RS_PLY_Played;
                        CallConnect();
                    } else {
                        CallDisconnect();
                    }
                }
                break;
                case RS_PLY_Played: {
                    DoReadData();
                }
                break;
            }
        }
    }
} // AnyRtmpPull::Run

void AnyRtmpPull::DoReadData()
{
    int size;
    char type;
    char* data;
    u_int32_t timestamp;

    if (srs_rtmp_read_packet(rtmp_, &type, &timestamp, &data, &size) != 0) {}
    if (type == SRS_RTMP_TYPE_VIDEO) {
        SrsCodecSample sample;
        if (srs_codec_->video_avc_demux(data, size, &sample) == ERROR_SUCCESS) {
            if (srs_codec_->video_codec_id == SrsCodecVideoAVC) {
                GotVideoSample(timestamp, &sample);
            } else {
                LOG(LS_ERROR) << "Don't support video format!";
            }
        }
    } else if (type == SRS_RTMP_TYPE_AUDIO) {
        SrsCodecSample sample;
        if (srs_codec_->audio_aac_demux(data, size, &sample) != ERROR_SUCCESS) {
            if (sample.acodec == SrsCodecAudioMP3 &&
                srs_codec_->audio_mp3_demux(data, size, &sample) != ERROR_SUCCESS)
            {
                free(data);
                return;
            }
            free(data);
            return;
        }
        SrsCodecAudio acodec = (SrsCodecAudio) srs_codec_->audio_codec_id;
        if (acodec != SrsCodecAudioAAC && acodec != SrsCodecAudioMP3) {
            free(data);
            return;
        }
        if (acodec == SrsCodecAudioAAC && sample.aac_packet_type == SrsCodecAudioTypeSequenceHeader ||
            srs_codec_->aac_object == SrsAacObjectTypeReserved)
        {
            free(data);
            return;
        }
        GotAudioSample(timestamp, &sample);
    } else if (type == SRS_RTMP_TYPE_SCRIPT) {
        if (!srs_rtmp_is_onMetaData(type, data, size)) {
            LOG(LS_ERROR) << "No flv";
            srs_human_trace("drop message type=%#x, size=%dB", type, size);
        }
    }
    free(data);
} // AnyRtmpPull::DoReadData

int AnyRtmpPull::GotVideoSample(u_int32_t timestamp, SrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;

    if (sample->frame_type == SrsCodecVideoAVCFrameVideoInfoFrame) {
        return ret;
    }
    if (sample->frame_type == SrsCodecVideoAVCFrameKeyFrame &&
        sample->avc_packet_type == SrsCodecVideoAVCTypeSequenceHeader)
    {
        return ret;
    }
    if (sample->has_idr) {
        if (srs_codec_->sequenceParameterSetLength > 0) {
            video_payload_->append((const char *) fresh_nalu_header, 4);
            video_payload_->append(srs_codec_->sequenceParameterSetNALUnit, srs_codec_->sequenceParameterSetLength);
        }
        if (srs_codec_->pictureParameterSetLength > 0) {
            video_payload_->append((const char *) fresh_nalu_header, 4);
            video_payload_->append(srs_codec_->pictureParameterSetNALUnit, srs_codec_->pictureParameterSetLength);
        }
    }
    for (int i = 0; i < sample->nb_sample_units; i++) {
        SrsCodecSampleUnit* sample_unit = &sample->sample_units[i];
        int32_t size = sample_unit->size;
        if (!sample_unit->bytes || size <= 0) {
            ret = -1;
            return ret;
        }
        SrsAvcNaluType nal_unit_type = (SrsAvcNaluType) (sample_unit->bytes[0] & 0x1f);
        switch (nal_unit_type) {
            case SrsAvcNaluTypeSPS:
            case SrsAvcNaluTypePPS:
            case SrsAvcNaluTypeSEI:
            case SrsAvcNaluTypeAccessUnitDelimiter:
                continue;
            default: {
                if (nal_unit_type == SrsAvcNaluTypeReserved) {
                    RescanVideoframe(sample_unit->bytes, sample_unit->size, timestamp);
                    continue;
                }
            }
            break;
        }
        if (nal_unit_type == SrsAvcNaluTypeIDR) {
            #ifdef WEBRTC_IOS
            video_payload_->append((const char *) fresh_nalu_header, 4);
            #else
            video_payload_->append((const char *) cont_nalu_header, 3);
            #endif
        } else {
            video_payload_->append((const char *) fresh_nalu_header, 4);
        }
        video_payload_->append(sample_unit->bytes, sample_unit->size);
    }
    if (video_payload_->_data_len != 0) {
        callback_.OnRtmpullH264Data((uint8_t *) video_payload_->_data, video_payload_->_data_len, timestamp);
    }
    video_payload_->reset();
    return ret;
} // AnyRtmpPull::GotVideoSample

int AnyRtmpPull::GotAudioSample(u_int32_t timestamp, SrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;

    for (int i = 0; i < sample->nb_sample_units; i++) {
        SrsCodecSampleUnit* sample_unit = &sample->sample_units[i];
        int32_t size = sample_unit->size;
        if (!sample_unit->bytes || size <= 0 || size > 0x1fff) {
            ret = -1;
            return ret;
        }
        int32_t frame_length      = size + 7;
        u_int8_t adts_header[7]   = { 0xff, 0xf9, 0x00, 0x00, 0x00, 0x0f, 0xfc };
        SrsAacProfile aac_profile = srs_codec_aac_rtmp2ts(srs_codec_->aac_object);
        adts_header[2]  = (aac_profile << 6) & 0xc0;
        adts_header[2] |= (srs_codec_->aac_sample_rate << 2) & 0x3c;
        adts_header[2] |= (srs_codec_->aac_channels >> 2) & 0x01;
        adts_header[3]  = (srs_codec_->aac_channels << 6) & 0xc0;
        adts_header[3] |= (frame_length >> 11) & 0x03;
        adts_header[4]  = (frame_length >> 3) & 0xff;
        adts_header[5]  = ((frame_length << 5) & 0xe0);
        adts_header[5] |= 0x1f;
        audio_payload_->append((const char *) adts_header, sizeof(adts_header));
        audio_payload_->append(sample_unit->bytes, sample_unit->size);
        callback_.OnRtmpullAACData((uint8_t *) audio_payload_->_data, audio_payload_->_data_len, timestamp);
        audio_payload_->reset();
    }
    return ret;
}

void AnyRtmpPull::RescanVideoframe(const char* pdata, int len, uint32_t timestamp)
{
    int nal_type  = pdata[4] & 0x1f;
    const char* p = pdata;

    if (nal_type == 7) {
        int find7        = 0;
        const char* ptr7 = NULL;
        int size7        = 0;
        int find8        = 0;
        const char* ptr8 = NULL;
        int size8        = 0;
        const char* ptr5 = NULL;
        int size5        = 0;
        int head01       = 4;
        for (int i = 4; i < len - 4; i++) {
            if ((p[i] == 0x0 && p[i + 1] == 0x0 && p[i + 2] == 0x0 && p[i + 3] == 0x1) ||
                (p[i] == 0x0 && p[i + 1] == 0x0 && p[i + 2] == 0x1))
            {
                if (p[i + 2] == 0x01) {
                    head01 = 3;
                } else {
                    head01 = 4;
                }
                if (find7 == 0) {
                    find7 = i;
                    ptr7  = p;
                    size7 = find7;
                    i++;
                } else if (find8 == 0) {
                    find8 = i;
                    ptr8  = p + find7;
                    size8 = find8 - find7;
                    const char* ptr = p + i;
                    if ((ptr[head01] & 0x1f) == 5) {
                        ptr5  = p + find8 + head01;
                        size5 = len - find8 - head01;
                        break;
                    }
                } else {
                    ptr5  = p + i + head01;
                    size5 = len - i - head01;
                    break;
                }
            }
        }
        video_payload_->append(ptr7, size7);
        video_payload_->append(ptr8, size8);
        video_payload_->append((const char *) fresh_nalu_header, 4);
        video_payload_->append(ptr5, size5);
        callback_.OnRtmpullH264Data((uint8_t *) video_payload_->_data, video_payload_->_data_len, timestamp);
        video_payload_->reset();
    } else {
        video_payload_->append(pdata, len);
        callback_.OnRtmpullH264Data((uint8_t *) video_payload_->_data, video_payload_->_data_len, timestamp);
        video_payload_->reset();
    }
} // AnyRtmpPull::RescanVideoframe

void AnyRtmpPull::CallConnect()
{
    retry_ct_  = 0;
    connected_ = true;
    callback_.OnRtmpullConnected();
}

void AnyRtmpPull::CallDisconnect()
{
    rtc::CritScope l(&cs_rtmp_);

    if (rtmp_) {
        srs_rtmp_destroy(rtmp_);
        rtmp_ = NULL;
    }
    if (rtmp_status_ != RS_PLY_Closed) {
        rtmp_status_ = RS_PLY_Init;
        retry_ct_++;
        if (retry_ct_ <= MAX_RETRY_TIME) {
            rtmp_ = srs_rtmp_create(str_url_.c_str());
        } else {
            if (connected_) {
                callback_.OnRtmpullDisconnect();
            } else {
                callback_.OnRtmpullFailed();
            }
        }
    }
}
