#include "BasicUDPSink.hh"
#include <GroupsockHelper.hh>
BasicUDPSink * BasicUDPSink::createNew(UsageEnvironment& env, Groupsock* gs,
                                       unsigned maxPayloadSize)
{
    return new BasicUDPSink(env, gs, maxPayloadSize);
}

BasicUDPSink::BasicUDPSink(UsageEnvironment& env, Groupsock* gs,
                           unsigned maxPayloadSize)
    : MediaSink(env),
    fGS(gs), fMaxPayloadSize(maxPayloadSize)
{
    fOutputBuffer = new unsigned char[fMaxPayloadSize];
}

BasicUDPSink::~BasicUDPSink()
{
    delete[] fOutputBuffer;
}

Boolean BasicUDPSink::continuePlaying()
{
    gettimeofday(&fNextSendTime, NULL);
    continuePlaying1();
    return True;
}

void BasicUDPSink::continuePlaying1()
{
    if (fSource != NULL) {
        fSource->getNextFrame(fOutputBuffer, fMaxPayloadSize,
                              afterGettingFrame, this,
                              onSourceClosure, this);
    }
}

void BasicUDPSink::afterGettingFrame(void* clientData, unsigned frameSize,
                                     unsigned numTruncatedBytes,
                                     struct timeval,
                                     unsigned durationInMicroseconds)
{
    BasicUDPSink* sink = (BasicUDPSink *) clientData;

    sink->afterGettingFrame1(frameSize, numTruncatedBytes, durationInMicroseconds);
}

void BasicUDPSink::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                                      unsigned durationInMicroseconds)
{
    if (numTruncatedBytes > 0) {
        envir()
            <<
            "BasicUDPSink::afterGettingFrame1(): The input frame data was too large for our spcified maximum payload size ("
            << fMaxPayloadSize << ").  "
            << numTruncatedBytes << " bytes of trailing data was dropped!\n";
    }
    fGS->output(envir(), fGS->ttl(), fOutputBuffer, frameSize);
    fNextSendTime.tv_usec += durationInMicroseconds;
    fNextSendTime.tv_sec  += fNextSendTime.tv_usec / 1000000;
    fNextSendTime.tv_usec %= 1000000;
    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);
    int secsDiff         = fNextSendTime.tv_sec - timeNow.tv_sec;
    int64_t uSecondsToGo = secsDiff * 1000000 + (fNextSendTime.tv_usec - timeNow.tv_usec);
    if (uSecondsToGo < 0 || secsDiff < 0) {
        uSecondsToGo = 0;
    }
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecondsToGo,
                                                             (TaskFunc *) sendNext, this);
}

void BasicUDPSink::sendNext(void* firstArg)
{
    BasicUDPSink* sink = (BasicUDPSink *) firstArg;

    sink->continuePlaying1();
}
