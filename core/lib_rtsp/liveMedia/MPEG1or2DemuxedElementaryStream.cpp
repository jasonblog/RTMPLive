#include "MPEG1or2DemuxedElementaryStream.hh"
MPEG1or2DemuxedElementaryStream::
MPEG1or2DemuxedElementaryStream(UsageEnvironment& env, u_int8_t streamIdTag,
                                MPEG1or2Demux& sourceDemux)
    : FramedSource(env),
    fOurStreamIdTag(streamIdTag), fOurSourceDemux(sourceDemux), fMPEGversion(0)
{
    if ((streamIdTag & 0xE0) == 0xC0) {
        fMIMEtype = "audio/MPEG";
    } else if ((streamIdTag & 0xF0) == 0xE0) {
        fMIMEtype = "video/MPEG";
    } else {
        fMIMEtype = MediaSource::MIMEtype();
    }
}

MPEG1or2DemuxedElementaryStream::~MPEG1or2DemuxedElementaryStream()
{
    fOurSourceDemux.noteElementaryStreamDeletion(this);
}

void MPEG1or2DemuxedElementaryStream::doGetNextFrame()
{
    fOurSourceDemux.getNextFrame(fOurStreamIdTag, fTo, fMaxSize,
                                 afterGettingFrame, this,
                                 handleClosure, this);
}

void MPEG1or2DemuxedElementaryStream::doStopGettingFrames()
{
    fOurSourceDemux.stopGettingFrames(fOurStreamIdTag);
}

char const * MPEG1or2DemuxedElementaryStream::MIMEtype() const
{
    return fMIMEtype;
}

unsigned MPEG1or2DemuxedElementaryStream::maxFrameSize() const
{
    return 6 + 65535;
}

void MPEG1or2DemuxedElementaryStream
::afterGettingFrame(void* clientData,
                    unsigned frameSize, unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds)
{
    MPEG1or2DemuxedElementaryStream* stream =
        (MPEG1or2DemuxedElementaryStream *) clientData;

    stream->afterGettingFrame1(frameSize, numTruncatedBytes,
                               presentationTime, durationInMicroseconds);
}

void MPEG1or2DemuxedElementaryStream
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                     struct timeval presentationTime,
                     unsigned durationInMicroseconds)
{
    fFrameSize              = frameSize;
    fNumTruncatedBytes      = numTruncatedBytes;
    fPresentationTime       = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    fLastSeenSCR            = fOurSourceDemux.lastSeenSCR();
    fMPEGversion            = fOurSourceDemux.mpegVersion();
    FramedSource::afterGetting(this);
}
