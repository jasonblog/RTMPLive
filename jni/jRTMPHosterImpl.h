#ifndef __J_RTMP_HOSTER_IMPL_H__
#define __J_RTMP_HOSTER_IMPL_H__
#include <jni.h>
#include "RtmpHoster.h"

class JRTMPHosterImpl : public RTMPHosterEvent
{
public:
    JRTMPHosterImpl(jobject javaObj);
    virtual ~JRTMPHosterImpl(void);

    void Close();
    RTMPHoster * Hoster()
    {
        return m_pHoster;
    };

public:

    virtual void OnRtmpStreamOK();
    virtual void OnRtmpStreamReconnecting(int times);
    virtual void OnRtmpStreamStatus(int delayMs, int netBand);
    virtual void OnRtmpStreamFailed(int code);
    virtual void OnRtmpStreamClosed();

private:
    jobject m_jJavaObj;
    jclass m_jClass;

    RTMPHoster* m_pHoster;
};

#endif // ifndef __J_RTMP_HOSTER_IMPL_H__
