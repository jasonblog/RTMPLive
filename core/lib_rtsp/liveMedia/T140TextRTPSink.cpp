#include "T140TextRTPSink.hh"
#include <GroupsockHelper.hh>
T140TextRTPSink::T140TextRTPSink(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat)
    : TextRTPSink(env, RTPgs, rtpPayloadFormat, 1000, "T140"),
    fOurIdleFilter(NULL), fAreInIdlePeriod(True)
{}

T140TextRTPSink::~T140TextRTPSink()
{
    fSource = fOurIdleFilter;
    stopPlaying();
    Medium::close(fOurIdleFilter);
    fSource = NULL;
}

T140TextRTPSink * T140TextRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                             unsigned char rtpPayloadFormat)
{
    return new T140TextRTPSink(env, RTPgs, rtpPayloadFormat);
}

Boolean T140TextRTPSink::continuePlaying()
{
    if (fOurIdleFilter == NULL) {
        fOurIdleFilter = new T140IdleFilter(envir(), fSource);
    } else {
        fOurIdleFilter->reassignInputSource(fSource);
    }
    fSource = fOurIdleFilter;
    return MultiFramedRTPSink::continuePlaying();
}

void T140TextRTPSink::doSpecialFrameHandling(unsigned,
                                             unsigned char *,
                                             unsigned       numBytesInFrame,
                                             struct timeval framePresentationTime,
                                             unsigned)
{
    if (fAreInIdlePeriod && numBytesInFrame > 0) {
        setMarkerBit();
    }
    fAreInIdlePeriod = numBytesInFrame == 0;
    setTimestamp(framePresentationTime);
}

Boolean T140TextRTPSink::frameCanAppearAfterPacketStart(unsigned char const *, unsigned) const
{
    return False;
}

T140IdleFilter::T140IdleFilter(UsageEnvironment& env, FramedSource* inputSource)
    : FramedFilter(env, inputSource),
    fIdleTimerTask(NULL),
    fBufferSize(OutPacketBuffer::maxSize), fNumBufferedBytes(0)
{
    fBuffer = new char[fBufferSize];
}

T140IdleFilter::~T140IdleFilter()
{
    envir().taskScheduler().unscheduleDelayedTask(fIdleTimerTask);
    delete[] fBuffer;
    detachInputSource();
}

#define IDLE_TIMEOUT_MICROSECONDS 300000
void T140IdleFilter::doGetNextFrame()
{
    if (fNumBufferedBytes > 0) {
        deliverFromBuffer();
        return;
    }
    fIdleTimerTask = envir().taskScheduler().scheduleDelayedTask(IDLE_TIMEOUT_MICROSECONDS, handleIdleTimeout, this);
    if (fInputSource != NULL && !fInputSource->isCurrentlyAwaitingData()) {
        fInputSource->getNextFrame((unsigned char *) fBuffer, fBufferSize, afterGettingFrame, this, onSourceClosure,
                                   this);
    }
}

void T140IdleFilter::afterGettingFrame(void* clientData, unsigned frameSize,
                                       unsigned numTruncatedBytes,
                                       struct timeval presentationTime,
                                       unsigned durationInMicroseconds)
{
    ((T140IdleFilter *) clientData)->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime,
                                                       durationInMicroseconds);
}

void T140IdleFilter::afterGettingFrame(unsigned       frameSize,
                                       unsigned       numTruncatedBytes,
                                       struct timeval presentationTime,
                                       unsigned       durationInMicroseconds)
{
    envir().taskScheduler().unscheduleDelayedTask(fIdleTimerTask);
    fNumBufferedBytes = frameSize;
    fBufferedNumTruncatedBytes          = numTruncatedBytes;
    fBufferedDataPresentationTime       = presentationTime;
    fBufferedDataDurationInMicroseconds = durationInMicroseconds;
    if (isCurrentlyAwaitingData()) {
        (void) deliverFromBuffer();
    }
}

void T140IdleFilter::doStopGettingFrames()
{
    envir().taskScheduler().unscheduleDelayedTask(fIdleTimerTask);
    FramedFilter::doStopGettingFrames();
}

void T140IdleFilter::handleIdleTimeout(void* clientData)
{
    ((T140IdleFilter *) clientData)->handleIdleTimeout();
}

void T140IdleFilter::handleIdleTimeout()
{
    deliverEmptyFrame();
}

void T140IdleFilter::deliverFromBuffer()
{
    if (fNumBufferedBytes <= fMaxSize) {
        fNumTruncatedBytes = fBufferedNumTruncatedBytes;
        fFrameSize         = fNumBufferedBytes;
    } else {
        fNumTruncatedBytes = fBufferedNumTruncatedBytes + fNumBufferedBytes - fMaxSize;
        fFrameSize         = fMaxSize;
    }
    memmove(fTo, fBuffer, fFrameSize);
    fPresentationTime       = fBufferedDataPresentationTime;
    fDurationInMicroseconds = fBufferedDataDurationInMicroseconds;
    fNumBufferedBytes       = 0;
    FramedSource::afterGetting(this);
}

void T140IdleFilter::deliverEmptyFrame()
{
    fFrameSize = fNumTruncatedBytes = 0;
    gettimeofday(&fPresentationTime, NULL);
    FramedSource::afterGetting(this);
}

void T140IdleFilter::onSourceClosure(void* clientData)
{
    ((T140IdleFilter *) clientData)->onSourceClosure();
}

void T140IdleFilter::onSourceClosure()
{
    envir().taskScheduler().unscheduleDelayedTask(fIdleTimerTask);
    fIdleTimerTask = NULL;
    handleClosure();
}
