#include "FramedSource.hh"
#include <stdlib.h>
FramedSource::FramedSource(UsageEnvironment& env)
    : MediaSource(env),
    fAfterGettingFunc(NULL), fAfterGettingClientData(NULL),
    fOnCloseFunc(NULL), fOnCloseClientData(NULL),
    fIsCurrentlyAwaitingData(False)
{
    fPresentationTime.tv_sec = fPresentationTime.tv_usec = 0;
}

FramedSource::~FramedSource()
{}

Boolean FramedSource::isFramedSource() const
{
    return True;
}

Boolean FramedSource::lookupByName(UsageEnvironment& env, char const* sourceName,
                                   FramedSource *& resultSource)
{
    resultSource = NULL;
    MediaSource* source;
    if (!MediaSource::lookupByName(env, sourceName, source)) {
        return False;
    }
    if (!source->isFramedSource()) {
        env.setResultMsg(sourceName, " is not a framed source");
        return False;
    }
    resultSource = (FramedSource *) source;
    return True;
}

void FramedSource::getNextFrame(unsigned char* to, unsigned maxSize,
                                afterGettingFunc* afterGettingFunc,
                                void* afterGettingClientData,
                                onCloseFunc* onCloseFunc,
                                void* onCloseClientData)
{
    if (fIsCurrentlyAwaitingData) {
        envir() << "FramedSource[" << this
                << "]::getNextFrame(): attempting to read more than once at the same time!\n";
        envir().internalError();
    }
    fTo      = to;
    fMaxSize = maxSize;
    fNumTruncatedBytes      = 0;
    fDurationInMicroseconds = 0;
    fAfterGettingFunc       = afterGettingFunc;
    fAfterGettingClientData = afterGettingClientData;
    fOnCloseFunc             = onCloseFunc;
    fOnCloseClientData       = onCloseClientData;
    fIsCurrentlyAwaitingData = True;
    doGetNextFrame();
}

void FramedSource::afterGetting(FramedSource* source)
{
    source->fIsCurrentlyAwaitingData = False;
    if (source->fAfterGettingFunc != NULL) {
        (*(source->fAfterGettingFunc))(source->fAfterGettingClientData,
                                       source->fFrameSize, source->fNumTruncatedBytes,
                                       source->fPresentationTime,
                                       source->fDurationInMicroseconds);
    }
}

void FramedSource::handleClosure(void* clientData)
{
    FramedSource* source = (FramedSource *) clientData;

    source->handleClosure();
}

void FramedSource::handleClosure()
{
    fIsCurrentlyAwaitingData = False;
    if (fOnCloseFunc != NULL) {
        (*fOnCloseFunc)(fOnCloseClientData);
    }
}

void FramedSource::stopGettingFrames()
{
    fIsCurrentlyAwaitingData = False;
    fAfterGettingFunc        = NULL;
    fOnCloseFunc = NULL;
    doStopGettingFrames();
}

void FramedSource::doStopGettingFrames()
{
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

unsigned FramedSource::maxFrameSize() const
{
    return 0;
}
