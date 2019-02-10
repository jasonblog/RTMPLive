#ifndef __PLAYER_BUFER_H__
#define __PLAYER_BUFER_H__
#include <list>
#include <stdint.h>
#include "webrtc/base/criticalsection.h"
#include "webrtc/base/messagehandler.h"
#include "webrtc/base/thread.h"
typedef struct PlyPacket {
    PlyPacket(bool isvideo) : _data(NULL), _data_len(0),
        _b_video(isvideo), _dts(0) {}

    virtual ~PlyPacket(void)
    {
        if (_data) {
            delete[] _data;
        }
    }

    void SetData(const uint8_t* pdata, int len, uint32_t ts)
    {
        _dts = ts;
        if (len > 0 && pdata != NULL) {
            if (_data) {
                delete[] _data;
            }
            if (_b_video) {
                _data = new uint8_t[len + 8];
            } else {
                _data = new uint8_t[len];
            }
            memcpy(_data, pdata, len);
            _data_len = len;
        }
    }

    uint8_t* _data;
    int      _data_len;
    bool     _b_video;
    uint32_t _dts;
} PlyPacket;
enum PlyStuts {
    PS_Fast = 0,
    PS_Normal,
    PS_Cache,
};
class PlyBufferCallback
{
public:
    PlyBufferCallback(void) {};
    virtual ~PlyBufferCallback(void) {};
    virtual void OnPlay()  = 0;
    virtual void OnPause() = 0;
    virtual bool OnNeedDecodeData(PlyPacket* pkt) = 0;
};
class PlyBuffer : public rtc::MessageHandler
{
public:
    PlyBuffer(PlyBufferCallback& callback, rtc::Thread* worker);
    virtual ~PlyBuffer();
    void SetCacheSize(int miliseconds);
    int GetPlayAudio(void* audioSamples);
    PlyStuts PlayerStatus()
    {
        return ply_status_;
    };
    int GetPlayCacheTime()
    {
        return buf_cache_time_;
    };
    void CacheH264Data(const uint8_t* pdata, int len, uint32_t ts);
    void CachePcmData(const uint8_t* pdata, int len, uint32_t ts);
protected:
    virtual void OnMessage(rtc::Message* msg);
    int GetCacheTime();
    void DoDecode();
private:
    PlyBufferCallback& callback_;
    bool got_audio_;
    int cache_time_;
    int cache_delta_;
    int buf_cache_time_;
    PlyStuts ply_status_;
    rtc::Thread* worker_thread_;
    uint32_t sys_fast_video_time_;
    uint32_t rtmp_fast_video_time_;
    uint32_t rtmp_cache_time_;
    uint32_t play_cur_time_;
    rtc::CriticalSection cs_list_audio_;
    std::list<PlyPacket *> lst_audio_buffer_;
    rtc::CriticalSection cs_list_video_;
    std::list<PlyPacket *> lst_video_buffer_;
};
#endif // ifndef __PLAYER_BUFER_H__
