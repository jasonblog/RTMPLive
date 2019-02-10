#ifndef __J_RTMP_GUEST_IMPL_H__
#define __J_RTMP_GUEST_IMPL_H__
#include <jni.h>
#include "RtmpGuester.h"

class JRTMPGuestImpl : public RTMPGuesterEvent
{
public:
    JRTMPGuestImpl(jobject javaObj);
    virtual ~JRTMPGuestImpl(void);

    void Close();
    RTMPGuester * Guest()
    {
        return m_pGuest;
    };

public:

    virtual void OnRtmplayerOK();
    virtual void OnRtmplayerStatus(int cacheTime, int curBitrate);
    virtual void OnRtmplayerCache(int time);
    virtual void OnRtmplayerClosed(int errcode);

private:
    jobject m_jJavaObj;
    jclass m_jClass;

    RTMPGuester* m_pGuest;
};

#endif // ifndef __J_RTMP_GUEST_IMPL_H__
