#include "BasicUsageEnvironment0.hh"
#include <stdio.h>
BasicUsageEnvironment0::BasicUsageEnvironment0(TaskScheduler& taskScheduler)
    : UsageEnvironment(taskScheduler),
    fBufferMaxSize(RESULT_MSG_BUFFER_MAX)
{
    reset();
}

BasicUsageEnvironment0::~BasicUsageEnvironment0()
{}

void BasicUsageEnvironment0::reset()
{
    fCurBufferSize = 0;
    fResultMsgBuffer[fCurBufferSize] = '\0';
}

char const * BasicUsageEnvironment0::getResultMsg() const
{
    return fResultMsgBuffer;
}

void BasicUsageEnvironment0::setResultMsg(MsgString msg)
{
    reset();
    appendToResultMsg(msg);
}

void BasicUsageEnvironment0::setResultMsg(MsgString msg1, MsgString msg2)
{
    setResultMsg(msg1);
    appendToResultMsg(msg2);
}

void BasicUsageEnvironment0::setResultMsg(MsgString msg1, MsgString msg2,
                                          MsgString msg3)
{
    setResultMsg(msg1, msg2);
    appendToResultMsg(msg3);
}

void BasicUsageEnvironment0::setResultErrMsg(MsgString msg, int err)
{
    setResultMsg(msg);
    #ifndef _WIN32_WCE
    appendToResultMsg(strerror(err == 0 ? getErrno() : err));
    #endif
}

void BasicUsageEnvironment0::appendToResultMsg(MsgString msg)
{
    char* curPtr = &fResultMsgBuffer[fCurBufferSize];
    unsigned spaceAvailable = fBufferMaxSize - fCurBufferSize;
    unsigned msgLength      = strlen(msg);

    if (msgLength > spaceAvailable - 1) {
        msgLength = spaceAvailable - 1;
    }
    memmove(curPtr, (char *) msg, msgLength);
    fCurBufferSize += msgLength;
    fResultMsgBuffer[fCurBufferSize] = '\0';
}

void BasicUsageEnvironment0::reportBackgroundError()
{
    fputs(getResultMsg(), stderr);
}
