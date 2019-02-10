#ifndef __RTMP_GUESTER_H__
#define __RTMP_GUESTER_H__
#include <string>
#include "LIV_Export.h"
#include "RTMPCommon.h"
class RTMPGuesterEvent
{
public:
    RTMPGuesterEvent(void) {};
    virtual ~RTMPGuesterEvent(void) {};
    virtual void OnRtmplayerOK() = 0;
    virtual void OnRtmplayerStatus(int cacheTime, int curBitrate) = 0;
    virtual void OnRtmplayerCache(int time)     = 0;
    virtual void OnRtmplayerClosed(int errcode) = 0;
};
class LIV_API RTMPGuester
{
public:
    static RTMPGuester * Create(RTMPGuesterEvent& callback);
    static void Destory(RTMPGuester* guester);
    virtual void StartRtmpPlay(const char* url, void* render) = 0;
    virtual void StopRtmpPlay() = 0;
protected:
    virtual void * GotSelfPtr() = 0;
protected:
    RTMPGuester() {};
    virtual ~RTMPGuester() {};
};
#endif // ifndef __RTMP_GUESTER_H__
