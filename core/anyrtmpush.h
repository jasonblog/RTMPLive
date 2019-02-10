#ifndef __ANY_RTMP_PUSH_H__
#define __ANY_RTMP_PUSH_H__
#include "webrtc/base/thread.h"
enum RTMP_STATUS {
    RS_STM_Init,
    RS_STM_Handshaked,
    RS_STM_Connected,
    RS_STM_Published,
    RS_STM_Closed
};
enum ENC_DATA_TYPE {
    VIDEO_DATA,
    AUDIO_DATA,
    META_DATA
};
typedef struct EncData {
    EncData(void) : _data(NULL), _dataLen(0),
        _bVideo(false), _dts(0) {}

    uint8_t*      _data;
    int           _dataLen;
    bool          _bVideo;
    uint32_t      _dts;
    ENC_DATA_TYPE _type;
} EncData;
class AnyRtmpushCallback
{
public:
    AnyRtmpushCallback(void) {};
    virtual ~AnyRtmpushCallback(void) {};
    virtual void OnRtmpConnected() = 0;
    virtual void OnRtmpReconnecting(int times) = 0;
    virtual void OnRtmpDisconnect() = 0;
    virtual void OnRtmpStatusEvent(int delayMs, int netBand) = 0;
};
class AnyRtmpPush : public rtc::Thread
{
public:
    AnyRtmpPush(AnyRtmpushCallback& callback, const std::string& url);
    virtual ~AnyRtmpPush(void);
    void Close();
    void EnableOnlyAudioMode();
    void SetAudioParameter(int samplerate, int pcmbitsize, int channel);
    void SetVideoParameter(int width, int height, int videodatarate, int framerate);
    void SetH264Data(uint8_t* pdata, int len, uint32_t ts);
    void SetAacData(uint8_t* pdata, int len, uint32_t ts);
    void GotH264Nal(uint8_t* pdata, int len, uint32_t ts);
protected:
    virtual void Run();
    void CallConnect();
    void CallDisconnect();
    void CallStatusEvent(int delayMs, int netBand);
    void DoSendData();
    void setMetaData();
    void setMetaData(uint8_t* pData, int nLen, uint32_t ts);
private:
    AnyRtmpushCallback& callback_;
    bool running_;
    bool need_keyframe_;
    bool only_audio_mode_;
    int retrys_;
    std::string str_url_;
    uint32_t stat_time_;
    uint32_t net_band_;
    rtc::CriticalSection cs_list_enc_;
    std::list<EncData *> lst_enc_data_;
private:
    char sound_format_;
    char sound_rate_;
    char sound_size_;
    char sound_type_;
    rtc::CriticalSection cs_rtmp_;
    RTMP_STATUS rtmp_status_;
    void* rtmp_;
    int video_width_;
    int video_height_;
    int video_framerate_;
    int video_datarate_;
    int sound_samplerate_;
};
#endif // ifndef __ANY_RTMP_PUSH_H__
