#include "plydecoder.h"
#include "anyrtmpcore.h"
#include "webrtc/base/logging.h"
#include "webrtc/media/engine/webrtcvideoframe.h"
#ifndef WEBRTC_WIN
enum Frametype_e {
    FRAME_I = 15,
    FRAME_P = 16,
    FRAME_B = 17
};
typedef struct Tag_bs_t {
    unsigned char* p_start;
    unsigned char* p;
    unsigned char* p_end;
    int            i_left;
} bs_t;
void bs_init(bs_t* s, void* p_data, int i_data)
{
    s->p_start = (unsigned char *) p_data;
    s->p       = (unsigned char *) p_data;
    s->p_end   = s->p + i_data;
    s->i_left  = 8;
}

int bs_read(bs_t* s, int i_count)
{
    static uint32_t i_mask[33] = { 0x00,
                                   0x01,      0x03,        0x07,       0x0f,
                                   0x1f,      0x3f,        0x7f,       0xff,
                                   0x1ff,     0x3ff,       0x7ff,      0xfff,
                                   0x1fff,    0x3fff,      0x7fff,     0xffff,
                                   0x1ffff,   0x3ffff,     0x7ffff,    0xfffff,
                                   0x1fffff,  0x3fffff,    0x7fffff,   0xffffff,
                                   0x1ffffff, 0x3ffffff,   0x7ffffff,  0xfffffff,
                                   0x1fffffff,0x3fffffff,  0x7fffffff, 0xffffffff
    };
    int i_shr;
    int i_result = 0;

    while (i_count > 0) {
        if (s->p >= s->p_end) {
            break;
        }
        if ((i_shr = s->i_left - i_count) >= 0) {
            i_result  |= (*s->p >> i_shr)&i_mask[i_count];
            s->i_left -= i_count;
            if (s->i_left == 0) {
                s->p++;
                s->i_left = 8;
            }
            return (i_result);
        } else {
            i_result |= (*s->p & i_mask[s->i_left]) << -i_shr;
            i_count  -= s->i_left;
            s->p++;
            s->i_left = 8;
        }
    }
    return (i_result);
} // bs_read

int bs_read1(bs_t* s)
{
    if (s->p < s->p_end) {
        unsigned int i_result;
        s->i_left--;
        i_result = (*s->p >> s->i_left) & 0x01;
        if (s->i_left == 0) {
            s->p++;
            s->i_left = 8;
        }
        return i_result;
    }
    return 0;
}

int bs_read_ue(bs_t* s)
{
    int i = 0;

    while (bs_read1(s) == 0 && s->p < s->p_end && i < 32) {
        i++;
    }
    return ((1 << i) - 1 + bs_read(s, i));
}

#endif // ifndef WEBRTC_WIN
PlyDecoder::PlyDecoder()
    : running_(false),
    playing_(false),
    h264_decoder_(NULL),
    video_render_(NULL),
    aac_decoder_(NULL),
    a_cache_len_(0),
    aac_sample_hz_(44100),
    aac_channels_(2),
    aac_frame_per10ms_size_(0)
{
    {
        h264_decoder_ = webrtc::H264Decoder::Create();
        webrtc::VideoCodec codecSetting;
        codecSetting.codecType = webrtc::kVideoCodecH264;
        codecSetting.width     = 640;
        codecSetting.height    = 480;
        h264_decoder_->InitDecode(&codecSetting, 1);
        h264_decoder_->RegisterDecodeCompleteCallback(this);
        webrtc::VideoCodec setting;
        setting.width        = 640;
        setting.height       = 480;
        setting.codecType    = webrtc::kVideoCodecH264;
        setting.maxFramerate = 30;
        if (h264_decoder_->InitDecode(&setting, 1) != 0) {}
    }
    aac_frame_per10ms_size_ = (aac_sample_hz_ / 100) * sizeof(int16_t) * aac_channels_;
    running_ = true;
    rtc::Thread::Start();
    ply_buffer_ = new PlyBuffer(*this, this);
}

PlyDecoder::~PlyDecoder()
{
    running_ = false;
    rtc::Thread::Stop();
    if (ply_buffer_) {
        delete ply_buffer_;
        ply_buffer_ = NULL;
    }
    if (aac_decoder_) {
        aac_decoder_close(aac_decoder_);
        aac_decoder_ = NULL;
    }
    if (h264_decoder_) {
        delete h264_decoder_;
        h264_decoder_ = NULL;
    }
}

bool PlyDecoder::IsPlaying()
{
    if (ply_buffer_ == NULL) {
        return false;
    }
    if (ply_buffer_->PlayerStatus() == PS_Cache) {
        return false;
    }
    return true;
}

int PlyDecoder::CacheTime()
{
    if (ply_buffer_ != NULL) {
        return ply_buffer_->GetPlayCacheTime();
    }
    return 0;
}

void PlyDecoder::AddH264Data(const uint8_t* pdata, int len, uint32_t ts)
{
    if (ply_buffer_) {
        ply_buffer_->CacheH264Data(pdata, len, ts);
    }
}

void PlyDecoder::AddAACData(const uint8_t* pdata, int len, uint32_t ts)
{
    if (ply_buffer_) {
        if (aac_decoder_ == NULL) {
            aac_decoder_ = aac_decoder_open((unsigned char *) pdata, len, &aac_channels_, &aac_sample_hz_);
            if (aac_channels_ == 0) {
                aac_channels_ = 1;
            }
            aac_frame_per10ms_size_ = (aac_sample_hz_ / 100) * sizeof(int16_t) * aac_channels_;
        } else {
            unsigned int outlen = 0;
            if (aac_decoder_decode_frame(aac_decoder_, (unsigned char *) pdata, len, audio_cache_ + a_cache_len_,
                                         &outlen) > 0)
            {
                a_cache_len_ += outlen;
                int ct    = 0;
                int fsize = aac_frame_per10ms_size_;
                while (a_cache_len_ > fsize) {
                    ply_buffer_->CachePcmData(audio_cache_ + ct * fsize, fsize, ts);
                    a_cache_len_ -= fsize;
                    ct++;
                }
                memmove(audio_cache_, audio_cache_ + ct * fsize, a_cache_len_);
            }
        }
    }
}

int PlyDecoder::GetPcmData(void* audioSamples, uint32_t& samplesPerSec, size_t& nChannels)
{
    if (!playing_) {
        return 0;
    }
    samplesPerSec = aac_sample_hz_;
    nChannels     = aac_channels_;
    return ply_buffer_->GetPlayAudio(audioSamples);
}

void PlyDecoder::Run()
{
    while (running_) {
        {
            this->ProcessMessages(1);
        }
        PlyPacket* pkt = NULL;
        {
            rtc::CritScope cs(&cs_list_h264_);
            if (lst_h264_buffer_.size() > 0) {
                pkt = lst_h264_buffer_.front();
                lst_h264_buffer_.pop_front();
            }
        }
        if (pkt != NULL) {
            if (h264_decoder_) {
                int frameType = pkt->_data[4] & 0x1f;
                webrtc::EncodedImage encoded_image;
                encoded_image._buffer = (uint8_t *) pkt->_data;
                encoded_image._length = pkt->_data_len;
                encoded_image._size   = pkt->_data_len + 8;
                if (frameType == 7) {
                    encoded_image._frameType = webrtc::kVideoFrameKey;
                } else {
                    encoded_image._frameType = webrtc::kVideoFrameDelta;
                }
                encoded_image._completeFrame = true;
                webrtc::RTPFragmentationHeader frag_info;
                int ret = h264_decoder_->Decode(encoded_image, false, &frag_info);
                if (ret != 0) {}
            }
            delete pkt;
        }
    }
} // PlyDecoder::Run

void PlyDecoder::OnPlay()
{
    playing_ = true;
}

void PlyDecoder::OnPause()
{
    playing_ = false;
}

bool PlyDecoder::OnNeedDecodeData(PlyPacket* pkt)
{
    const uint8_t* pdata = pkt->_data;

    if (pkt->_b_video) {
        #ifndef WEBRTC_WIN
        bs_t s;
        bs_init(&s, pkt->_data + 4 + 1, pkt->_data_len - 4 - 1);
        {
            bs_read_ue(&s);
            int frame_type = bs_read_ue(&s);
            Frametype_e ft = FRAME_P;
            switch (frame_type) {
                case 0:
                case 5:
                    ft = FRAME_P;
                    break;
                case 1:
                case 6:
                    ft = FRAME_B;
                    break;
                case 3:
                case 8:
                    ft = FRAME_P;
                    break;
                case 2:
                case 7:
                    ft = FRAME_I;
                    break;
                case 4:
                case 9:
                    ft = FRAME_I;
                    break;
            }
            if (ft == FRAME_B) {
                return false;
            }
        }
        #endif // ifndef WEBRTC_WIN
        int type = pdata[4] & 0x1f;
        rtc::CritScope cs(&cs_list_h264_);
        if (type == 7) {
            std::list<PlyPacket *>::iterator iter = lst_h264_buffer_.begin();
            while (iter != lst_h264_buffer_.end()) {
                PlyPacket* plypkt = *iter;
                lst_h264_buffer_.erase(iter++);
                delete plypkt;
            }
        }
        lst_h264_buffer_.push_back(pkt);
    }
    return true;
} // PlyDecoder::OnNeedDecodeData

int32_t PlyDecoder::Decoded(webrtc::VideoFrame& decodedImage)
{
    const cricket::WebRtcVideoFrame render_frame(
        decodedImage.video_frame_buffer(),
        decodedImage.render_time_ms() * rtc::kNumNanosecsPerMillisec, decodedImage.rotation());

    if (video_render_ != NULL) {
        video_render_->OnFrame(render_frame);
    }
    return 0;
}
