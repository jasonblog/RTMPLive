#include "jRTMPGuestImpl.h"
#include "webrtc/api/java/jni/classreferenceholder.h"
#include "webrtc/api/java/jni/jni_helpers.h"
#include "webrtc/modules/utility/include/helpers_android.h"

JRTMPGuestImpl::JRTMPGuestImpl(jobject javaObj)
    : m_pGuest(NULL),
    m_jJavaObj(NULL),
    m_jClass(NULL)
{
    if (javaObj) {
        webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
        m_jJavaObj = ats.env()->NewGlobalRef(javaObj);
        m_jClass   = reinterpret_cast<jclass>(ats.env()->NewGlobalRef(ats.env()->GetObjectClass(m_jJavaObj)));
    }

    m_pGuest = RTMPGuester::Create(*this);
}

JRTMPGuestImpl::~JRTMPGuestImpl(void)
{
    Close();
}

void JRTMPGuestImpl::Close()
{
    if (m_pGuest) {
        m_pGuest->StopRtmpPlay();
        RTMPGuester::Destory(m_pGuest);
        m_pGuest = NULL;
    }

    if (m_jJavaObj) {
        webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
        ats.env()->DeleteGlobalRef(m_jClass);
        m_jClass = NULL;
        ats.env()->DeleteGlobalRef(m_jJavaObj);
        m_jJavaObj = NULL;
    }
}

void JRTMPGuestImpl::OnRtmplayerOK()
{
    webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
    JNIEnv* jni = ats.env();
    {
        jmethodID j_callJavaMId = webrtc_jni::GetMethodID(jni, m_jClass, "OnRtmplayerOK", "()V");

        jni->CallVoidMethod(m_jJavaObj, j_callJavaMId);
    }
}

void JRTMPGuestImpl::OnRtmplayerStatus(int cacheTime, int curBitrate)
{
    webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
    JNIEnv* jni = ats.env();
    {
        jmethodID j_callJavaMId = webrtc_jni::GetMethodID(jni, m_jClass, "OnRtmplayerStatus", "(II)V");

        jni->CallVoidMethod(m_jJavaObj, j_callJavaMId, cacheTime, curBitrate);
    }
}

void JRTMPGuestImpl::OnRtmplayerCache(int time)
{
    webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
    JNIEnv* jni = ats.env();
    {
        jmethodID j_callJavaMId = webrtc_jni::GetMethodID(jni, m_jClass, "OnRtmplayerCache", "(I)V");

        jni->CallVoidMethod(m_jJavaObj, j_callJavaMId, time);
    }
}

void JRTMPGuestImpl::OnRtmplayerClosed(int errcode)
{
    webrtc::AttachThreadScoped ats(webrtc_jni::GetJVM());
    JNIEnv* jni = ats.env();
    {
        jmethodID j_callJavaMId = webrtc_jni::GetMethodID(jni, m_jClass, "OnRtmplayerClosed", "(I)V");

        jni->CallVoidMethod(m_jJavaObj, j_callJavaMId, errcode);
    }
}
