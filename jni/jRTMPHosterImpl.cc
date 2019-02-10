#include "jRTMPHosterImpl.h"
#include "webrtc/api/java/jni/classreferenceholder.h"
#include "webrtc/api/java/jni/jni_helpers.h"
#include "webrtc/modules/utility/include/helpers_android.h"

JRTMPHosterImpl::JRTMPHosterImpl(jobject javaObj)
    : m_pHoster(NULL),
    m_jJavaObj(NULL),
    m_jClass(NULL)
{
    if (javaObj) {
        webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
        m_jJavaObj = ats.env()->NewGlobalRef(javaObj);
        m_jClass   = reinterpret_cast<jclass>(ats.env()->NewGlobalRef(ats.env()->GetObjectClass(m_jJavaObj)));
    }

    m_pHoster = RTMPHoster::Create(*this);
}

JRTMPHosterImpl::~JRTMPHosterImpl(void)
{
    Close();
}

void JRTMPHosterImpl::Close()
{
    if (m_pHoster) {
        m_pHoster->SetVideoCapturer(NULL);
        m_pHoster->StopRtmpStream();
        RTMPHoster::Destory(m_pHoster);
        m_pHoster = NULL;
    }

    if (m_jJavaObj) {
        webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
        ats.env()->DeleteGlobalRef(m_jClass);
        m_jClass = NULL;
        ats.env()->DeleteGlobalRef(m_jJavaObj);
        m_jJavaObj = NULL;
    }
}

void JRTMPHosterImpl::OnRtmpStreamOK()
{
    webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
    JNIEnv* jni = ats.env();
    {
        jmethodID j_callJavaMId = webrtc_jni::GetMethodID(jni, m_jClass, "OnRtmpStreamOK", "()V");

        jni->CallVoidMethod(m_jJavaObj, j_callJavaMId);
    }
}

void JRTMPHosterImpl::OnRtmpStreamReconnecting(int times)
{
    webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
    JNIEnv* jni = ats.env();
    {
        jmethodID j_callJavaMId = webrtc_jni::GetMethodID(jni, m_jClass, "OnRtmpStreamReconnecting", "(I)V");

        jni->CallVoidMethod(m_jJavaObj, j_callJavaMId, times);
    }
}

void JRTMPHosterImpl::OnRtmpStreamStatus(int delayMs, int netBand)
{
    webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
    JNIEnv* jni = ats.env();
    {
        jmethodID j_callJavaMId = webrtc_jni::GetMethodID(jni, m_jClass, "OnRtmpStreamStatus", "(II)V");

        jni->CallVoidMethod(m_jJavaObj, j_callJavaMId, delayMs, netBand);
    }
}

void JRTMPHosterImpl::OnRtmpStreamFailed(int code)
{
    webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
    JNIEnv* jni = ats.env();
    {
        jmethodID j_callJavaMId = webrtc_jni::GetMethodID(jni, m_jClass, "OnRtmpStreamFailed", "(I)V");

        jni->CallVoidMethod(m_jJavaObj, j_callJavaMId, code);
    }
}

void JRTMPHosterImpl::OnRtmpStreamClosed()
{
    webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
    JNIEnv* jni = ats.env();
    {
        jmethodID j_callJavaMId = webrtc_jni::GetMethodID(jni, m_jClass, "OnRtmpStreamClosed", "()V");

        jni->CallVoidMethod(m_jJavaObj, j_callJavaMId);
    }
}
