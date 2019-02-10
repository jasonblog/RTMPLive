#include "MPEG2TransportStreamFromPESSource.hh"
#define MAX_PES_PACKET_SIZE (6+65535)
MPEG2TransportStreamFromPESSource * MPEG2TransportStreamFromPESSource
::createNew(UsageEnvironment& env, MPEG1or2DemuxedElementaryStream* inputSource)
{
    return new MPEG2TransportStreamFromPESSource(env, inputSource);
}

MPEG2TransportStreamFromPESSource
::MPEG2TransportStreamFromPESSource(UsageEnvironment                 & env,
                                    MPEG1or2DemuxedElementaryStream* inputSource)
    : MPEG2TransportStreamMultiplexor(env),
    fInputSource(inputSource)
{
    fInputBuffer = new unsigned char[MAX_PES_PACKET_SIZE];
}

MPEG2TransportStreamFromPESSource::~MPEG2TransportStreamFromPESSource()
{
    Medium::close(fInputSource);
    delete[] fInputBuffer;
}

void MPEG2TransportStreamFromPESSource::doStopGettingFrames()
{
    fInputSource->stopGettingFrames();
}

void MPEG2TransportStreamFromPESSource
::awaitNewBuffer(unsigned char *)
{
    fInputSource->getNextFrame(fInputBuffer, MAX_PES_PACKET_SIZE,
                               afterGettingFrame, this,
                               FramedSource::handleClosure, this);
}

void MPEG2TransportStreamFromPESSource
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds)
{
    MPEG2TransportStreamFromPESSource* source =
        (MPEG2TransportStreamFromPESSource *) clientData;

    source->afterGettingFrame1(frameSize, numTruncatedBytes,
                               presentationTime, durationInMicroseconds);
}

void MPEG2TransportStreamFromPESSource
::afterGettingFrame1(unsigned frameSize,
                     unsigned,
                     struct timeval,
                     unsigned)
{
    if (frameSize < 4) {
        return;
    }
    handleNewBuffer(fInputBuffer, frameSize,
                    fInputSource->mpegVersion(), fInputSource->lastSeenSCR());
}
