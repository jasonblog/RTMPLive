#include "webrtc/api/java/jni/jni_helpers.h"

#include "webrtc/api/java/jni/classreferenceholder.h"

#include <asm/unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace webrtc_jni
{
static JavaVM* g_jvm = nullptr;

static pthread_once_t g_jni_ptr_once = PTHREAD_ONCE_INIT;



static pthread_key_t g_jni_ptr;

JavaVM * GetJVM()
{
    RTC_CHECK(g_jvm) << "JNI_OnLoad failed to run?";
    return g_jvm;
}

JNIEnv * GetEnv()
{
    void* env   = nullptr;
    jint status = g_jvm->GetEnv(&env, JNI_VERSION_1_6);

    RTC_CHECK(((env != nullptr) && (status == JNI_OK)) ||
              ((env == nullptr) && (status == JNI_EDETACHED)))
        << "Unexpected GetEnv return: " << status << ":" << env;
    return reinterpret_cast<JNIEnv *>(env);
}

static void ThreadDestructor(void* prev_jni_ptr)
{
    if (!GetEnv()) {
        return;
    }

    RTC_CHECK(GetEnv() == prev_jni_ptr)
        << "Detaching from another thread: " << prev_jni_ptr << ":" << GetEnv();
    jint status = g_jvm->DetachCurrentThread();
    RTC_CHECK(status == JNI_OK) << "Failed to detach thread: " << status;
    RTC_CHECK(!GetEnv()) << "Detaching was a successful no-op???";
}

static void CreateJNIPtrKey()
{
    RTC_CHECK(!pthread_key_create(&g_jni_ptr, &ThreadDestructor))
        << "pthread_key_create";
}

jint InitGlobalJniVariables(JavaVM* jvm)
{
    RTC_CHECK(!g_jvm) << "InitGlobalJniVariables!";
    g_jvm = jvm;
    RTC_CHECK(g_jvm) << "InitGlobalJniVariables handed NULL?";

    RTC_CHECK(!pthread_once(&g_jni_ptr_once, &CreateJNIPtrKey)) << "pthread_once";

    JNIEnv* jni = nullptr;

    if (jvm->GetEnv(reinterpret_cast<void **>(&jni), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    return JNI_VERSION_1_6;
}

static std::string GetThreadId()
{
    char buf[21];

    RTC_CHECK_LT(snprintf(buf, sizeof(buf), "%ld",
                          static_cast<long>(syscall(__NR_gettid))),
                 sizeof(buf))
        << "Thread id is bigger than uint64??";
    return std::string(buf);
}

static std::string GetThreadName()
{
    char name[17] = { 0 };

    if (prctl(PR_GET_NAME, name) != 0) {
        return std::string("<noname>");
    }

    return std::string(name);
}

JNIEnv * AttachCurrentThreadIfNeeded()
{
    JNIEnv* jni = GetEnv();

    if (jni) {
        return jni;
    }

    RTC_CHECK(!pthread_getspecific(g_jni_ptr))
        << "TLS has a JNIEnv* but not attached?";

    std::string name(GetThreadName() + " - " + GetThreadId());
    JavaVMAttachArgs args;
    args.version = JNI_VERSION_1_6;
    args.name    = &name[0];
    args.group   = nullptr;

    #ifdef _JAVASOFT_JNI_H_
    void* env = nullptr;
    #else
    JNIEnv* env = nullptr;
    #endif
    RTC_CHECK(!g_jvm->AttachCurrentThread(&env, &args))
        << "Failed to attach thread";
    RTC_CHECK(env) << "AttachCurrentThread handed back NULL!";
    jni = reinterpret_cast<JNIEnv *>(env);
    RTC_CHECK(!pthread_setspecific(g_jni_ptr, jni)) << "pthread_setspecific";
    return jni;
}

jlong jlongFromPointer(void* ptr)
{
    static_assert(sizeof(intptr_t) <= sizeof(jlong),
                  "Time to rethink the use of jlongs");



    jlong ret = reinterpret_cast<intptr_t>(ptr);
    RTC_DCHECK(reinterpret_cast<void *>(ret) == ptr);
    return ret;
}

jmethodID GetMethodID(
    JNIEnv* jni, jclass c, const std::string& name, const char* signature)
{
    jmethodID m = jni->GetMethodID(c, name.c_str(), signature);

    CHECK_EXCEPTION(jni) << "error during GetMethodID: " << name << ", "
                         << signature;
    RTC_CHECK(m) << name << ", " << signature;
    return m;
}

jmethodID GetStaticMethodID(
    JNIEnv* jni, jclass c, const char* name, const char* signature)
{
    jmethodID m = jni->GetStaticMethodID(c, name, signature);

    CHECK_EXCEPTION(jni) << "error during GetStaticMethodID: " << name << ", "
                         << signature;
    RTC_CHECK(m) << name << ", " << signature;
    return m;
}

jfieldID GetFieldID(
    JNIEnv* jni, jclass c, const char* name, const char* signature)
{
    jfieldID f = jni->GetFieldID(c, name, signature);

    CHECK_EXCEPTION(jni) << "error during GetFieldID";
    RTC_CHECK(f) << name << ", " << signature;
    return f;
}

jclass GetObjectClass(JNIEnv* jni, jobject object)
{
    jclass c = jni->GetObjectClass(object);

    CHECK_EXCEPTION(jni) << "error during GetObjectClass";
    RTC_CHECK(c) << "GetObjectClass returned NULL";
    return c;
}

jobject GetObjectField(JNIEnv* jni, jobject object, jfieldID id)
{
    jobject o = jni->GetObjectField(object, id);

    CHECK_EXCEPTION(jni) << "error during GetObjectField";
    RTC_CHECK(!IsNull(jni, o)) << "GetObjectField returned NULL";
    return o;
}

jobject GetNullableObjectField(JNIEnv* jni, jobject object, jfieldID id)
{
    jobject o = jni->GetObjectField(object, id);

    CHECK_EXCEPTION(jni) << "error during GetObjectField";
    return o;
}

jstring GetStringField(JNIEnv* jni, jobject object, jfieldID id)
{
    return static_cast<jstring>(GetObjectField(jni, object, id));
}

jlong GetLongField(JNIEnv* jni, jobject object, jfieldID id)
{
    jlong l = jni->GetLongField(object, id);

    CHECK_EXCEPTION(jni) << "error during GetLongField";
    return l;
}

jint GetIntField(JNIEnv* jni, jobject object, jfieldID id)
{
    jint i = jni->GetIntField(object, id);

    CHECK_EXCEPTION(jni) << "error during GetIntField";
    return i;
}

bool GetBooleanField(JNIEnv* jni, jobject object, jfieldID id)
{
    jboolean b = jni->GetBooleanField(object, id);

    CHECK_EXCEPTION(jni) << "error during GetBooleanField";
    return b;
}

bool IsNull(JNIEnv* jni, jobject obj)
{
    return jni->IsSameObject(obj, nullptr);
}

jstring JavaStringFromStdString(JNIEnv* jni, const std::string& native)
{
    jstring jstr = jni->NewStringUTF(native.c_str());

    CHECK_EXCEPTION(jni) << "error during NewStringUTF";
    return jstr;
}

std::string JavaToStdString(JNIEnv* jni, const jstring& j_string)
{
    const char* chars = jni->GetStringUTFChars(j_string, nullptr);

    CHECK_EXCEPTION(jni) << "Error during GetStringUTFChars";
    std::string str(chars, jni->GetStringUTFLength(j_string));
    CHECK_EXCEPTION(jni) << "Error during GetStringUTFLength";
    jni->ReleaseStringUTFChars(j_string, chars);
    CHECK_EXCEPTION(jni) << "Error during ReleaseStringUTFChars";
    return str;
}

jobject JavaEnumFromIndex(JNIEnv* jni, jclass state_class,
                          const std::string& state_class_name, int index)
{
    jmethodID state_values_id = GetStaticMethodID(
        jni, state_class, "values", ("()[L" + state_class_name  + ";").c_str());
    jobjectArray state_values = static_cast<jobjectArray>(
        jni->CallStaticObjectMethod(state_class, state_values_id));

    CHECK_EXCEPTION(jni) << "error during CallStaticObjectMethod";
    jobject ret = jni->GetObjectArrayElement(state_values, index);
    CHECK_EXCEPTION(jni) << "error during GetObjectArrayElement";
    return ret;
}

std::string GetJavaEnumName(JNIEnv*          jni,
                            const std::string& className,
                            jobject          j_enum)
{
    jclass enumClass     = FindClass(jni, className.c_str());
    jmethodID nameMethod =
        GetMethodID(jni, enumClass, "name", "()Ljava/lang/String;");
    jstring name =
        reinterpret_cast<jstring>(jni->CallObjectMethod(j_enum, nameMethod));

    CHECK_EXCEPTION(jni) << "error during CallObjectMethod for " << className
                         << ".name";
    return JavaToStdString(jni, name);
}

jobject NewGlobalRef(JNIEnv* jni, jobject o)
{
    jobject ret = jni->NewGlobalRef(o);

    CHECK_EXCEPTION(jni) << "error during NewGlobalRef";
    RTC_CHECK(ret);
    return ret;
}

void DeleteGlobalRef(JNIEnv* jni, jobject o)
{
    jni->DeleteGlobalRef(o);
    CHECK_EXCEPTION(jni) << "error during DeleteGlobalRef";
}

ScopedLocalRefFrame::ScopedLocalRefFrame(JNIEnv* jni) : jni_(jni)
{
    RTC_CHECK(!jni_->PushLocalFrame(0)) << "Failed to PushLocalFrame";
}

ScopedLocalRefFrame::~ScopedLocalRefFrame()
{
    jni_->PopLocalFrame(nullptr);
}

Iterable::Iterator::Iterator() : iterator_(nullptr) {}


Iterable::Iterator::Iterator(JNIEnv* jni, jobject iterable) : jni_(jni)
{
    jclass j_class        = GetObjectClass(jni, iterable);
    jmethodID iterator_id =
        GetMethodID(jni, j_class, "iterator", "()Ljava/util/Iterator;");

    iterator_ = jni->CallObjectMethod(iterable, iterator_id);
    CHECK_EXCEPTION(jni) << "error during CallObjectMethod";
    RTC_CHECK(iterator_ != nullptr);

    jclass iterator_class = GetObjectClass(jni, iterator_);
    has_next_id_ = GetMethodID(jni, iterator_class, "hasNext", "()Z");
    next_id_     = GetMethodID(jni, iterator_class, "next", "()Ljava/lang/Object;");


    ++(*this);
}

Iterable::Iterator::Iterator(Iterator&& other)
    : jni_(std::move(other.jni_)),
    iterator_(std::move(other.iterator_)),
    value_(std::move(other.value_)),
    has_next_id_(std::move(other.has_next_id_)),
    next_id_(std::move(other.next_id_)),
    thread_checker_(std::move(other.thread_checker_)) {};


Iterable::Iterator& Iterable::Iterator::operator ++ ()
{
    RTC_CHECK(thread_checker_.CalledOnValidThread());

    if (AtEnd()) {
        return *this;
    }

    bool has_next = jni_->CallBooleanMethod(iterator_, has_next_id_);
    CHECK_EXCEPTION(jni_) << "error during CallBooleanMethod";

    if (!has_next) {
        iterator_ = nullptr;
        value_    = nullptr;
        return *this;
    }

    value_ = jni_->CallObjectMethod(iterator_, next_id_);
    CHECK_EXCEPTION(jni_) << "error during CallObjectMethod";
    return *this;
}

bool Iterable::Iterator::operator == (const Iterable::Iterator& other)
{
    RTC_DCHECK(this == &other || AtEnd() || other.AtEnd());
    return AtEnd() == other.AtEnd();
}

jobject Iterable::Iterator::operator * ()
{
    RTC_CHECK(!AtEnd());
    return value_;
}

bool Iterable::Iterator::AtEnd() const
{
    RTC_CHECK(thread_checker_.CalledOnValidThread());
    return jni_ == nullptr || IsNull(jni_, iterator_);
}
}
