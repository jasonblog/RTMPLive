#ifndef __RTMP_HOSTER_H__
#define __RTMP_HOSTER_H__
#include <string>
#include "LIV_Export.h"
#include "RTMPCommon.h"
class RTMPHosterEvent
{
public:
    RTMPHosterEvent(void) {};
    virtual ~RTMPHosterEvent(void) {};
public:
    virtual void OnRtmpStreamOK() = 0;
    virtual void OnRtmpStreamReconnecting(int times) = 0;
    virtual void OnRtmpStreamStatus(int delayMs, int netBand) = 0;
    virtual void OnRtmpStreamFailed(int code) = 0;
    virtual void OnRtmpStreamClosed()         = 0;
};
class LIV_API RTMPHoster
{
public:
    static RTMPHoster * Create(RTMPHosterEvent& callback);
    static void Destory(RTMPHoster* hoster);
    virtual void SetAudioEnable(bool enabled)          = 0;
    virtual void SetVideoEnable(bool enabled)          = 0;
    virtual void SetVideoRender(void* render)          = 0;
    virtual void SetVideoCapturer(void* handle)        = 0;
    virtual void SetVideoMode(RTMPVideoMode videoMode) = 0;
    virtual void StartRtmpStream(const char* url)      = 0;
    virtual void StopRtmpStream() = 0;
protected:
    virtual void * GotSelfPtr() = 0;
protected:
    RTMPHoster() {};
    virtual ~RTMPHoster() {};
};
#endif // ifndef __RTMP_HOSTER_H__
