#ifndef __ANY_RTMP_PLAYER_INTERFACE_H__
#define __ANY_RTMP_PLAYER_INTERFACE_H__
#include "LIV_Export.h"
#include <stdint.h>
class AnyRtmplayerEvent;
class LIV_API AnyRtmplayer
{
public:
    virtual ~AnyRtmplayer(void) {};
    static AnyRtmplayer * Create(AnyRtmplayerEvent& callback);
    virtual void StartPlay(const char* url)   = 0;
    virtual void SetVideoRender(void* handle) = 0;
    virtual void StopPlay() = 0;
protected:
    AnyRtmplayer(AnyRtmplayerEvent& callback) : callback_(callback) {};
protected:
    AnyRtmplayerEvent& callback_;
};
class AnyRtmplayerEvent
{
public:
    AnyRtmplayerEvent(void) {};
    virtual ~AnyRtmplayerEvent(void) {};
    virtual void OnRtmplayerOK() = 0;
    virtual void OnRtmplayerStatus(int cacheTime, int curBitrate) = 0;
    virtual void OnRtmplayerCache(int time)    = 0;
    virtual void OnRtmplayerClose(int errcode) = 0;
};
#endif // ifndef __ANY_RTMP_PLAYER_INTERFACE_H__
