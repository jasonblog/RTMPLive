#include <jni.h>
#undef JNIEXPORT
#define JNIEXPORT __attribute__((visibility("default")))

#include "webrtc/api/java/jni/classreferenceholder.h"
#include "webrtc/api/java/jni/jni_helpers.h"

namespace webrtc_jni
{
extern "C" jint JNIEXPORT JNICALL JNI_OnLoad(JavaVM* jvm, void* reserved)
{
    jint ret = InitGlobalJniVariables(jvm);

    RTC_DCHECK_GE(ret, 0);

    if (ret < 0) {
        return -1;
    }

    LoadGlobalClassReferenceHolder();

    return ret;
}

extern "C" void JNIEXPORT JNICALL JNI_OnUnLoad(JavaVM* jvm, void* reserved)
{
    FreeGlobalClassReferenceHolder();
}
}
