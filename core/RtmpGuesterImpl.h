#ifndef __RTMP_GUSTER_IMPL_H__
#define __RTMP_GUSTER_IMPL_H__
#include "RtmpGuester.h"
#include "anyrtmpcore.h"
#include "anyrtmplayer.h"
#include "video_renderer.h"
class RtmpGuesterImpl : public RTMPGuester, public AnyRtmplayerEvent, public webrtc::AVAudioTrackCallback
{
public:
    RtmpGuesterImpl(RTMPGuesterEvent& callback);
    virtual ~RtmpGuesterImpl();
public:
    virtual void StartRtmpPlay(const char* url, void* render);
    virtual void StopRtmpPlay();
    virtual void * GotSelfPtr()
    {
        return this;
    };
protected:
    virtual void OnRtmplayerOK();
    virtual void OnRtmplayerStatus(int cacheTime, int curBitrate);
    virtual void OnRtmplayerCache(int time);
    virtual void OnRtmplayerClose(int errcode);
    virtual int OnNeedPlayAudio(void* audioSamples, uint32_t& samplesPerSec, size_t& nChannels);
private:
    RTMPGuesterEvent& callback_;
    rtc::Thread* worker_thread_;
    std::string rtmp_url_;
    bool av_rtmp_started_;
    AnyRtmplayer* av_rtmp_player_;
    webrtc::VideoRenderer* video_render_;
};
#endif // ifndef __RTMP_GUSTER_IMPL_H__
