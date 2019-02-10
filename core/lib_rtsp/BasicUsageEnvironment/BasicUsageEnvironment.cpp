#include "BasicUsageEnvironment.hh"
#include <stdio.h>
#ifdef ANDROID
# include <android/log.h>
# define TAG "XRTSP"
# define LGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
# define LGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
# define LGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#endif
#if defined(__WIN32__) || defined(_WIN32)
extern "C" int initializeWinsockIfNecessary();
#endif
BasicUsageEnvironment::BasicUsageEnvironment(TaskScheduler& taskScheduler)
    : BasicUsageEnvironment0(taskScheduler)
{
    #if defined(__WIN32__) || defined(_WIN32)
    if (!initializeWinsockIfNecessary()) {
        setResultErrMsg("Failed to initialize 'winsock': ");
        reportBackgroundError();
        internalError();
    }
    #endif
}

BasicUsageEnvironment::~BasicUsageEnvironment()
{}

BasicUsageEnvironment * BasicUsageEnvironment::createNew(TaskScheduler& taskScheduler)
{
    return new BasicUsageEnvironment(taskScheduler);
}

int BasicUsageEnvironment::getErrno() const
{
    #if defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_WCE)
    return WSAGetLastError();

    #else
    return errno;

    #endif
}

UsageEnvironment& BasicUsageEnvironment::operator << (char const* str)
{
    if (str == NULL) {
        str = "(NULL)";
    }
    #ifdef ANDROID
    LGI("%s", str);
    #else
    fprintf(stderr, "%s", str);
    #endif
    return *this;
}

UsageEnvironment& BasicUsageEnvironment::operator << (int i)
{
    #ifdef ANDROID
    LGI("%d", i);
    #else
    fprintf(stderr, "%d", i);
    #endif
    return *this;
}

UsageEnvironment& BasicUsageEnvironment::operator << (unsigned u)
{
    #ifdef ANDROID
    LGI("%u", u);
    #else
    fprintf(stderr, "%u", u);
    #endif
    return *this;
}

UsageEnvironment& BasicUsageEnvironment::operator << (double d)
{
    #ifdef ANDROID
    LGI("%f", d);
    #else
    fprintf(stderr, "%f", d);
    #endif
    return *this;
}

UsageEnvironment& BasicUsageEnvironment::operator << (void* p)
{
    #ifdef ANDROID
    LGI("%p", p);
    #else
    fprintf(stderr, "%p", p);
    #endif
    return *this;
}
