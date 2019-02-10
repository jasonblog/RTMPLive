#include "QCELPAudioRTPSource.hh"
#include "MultiFramedRTPSource.hh"
#include "FramedFilter.hh"
#include <string.h>
#include <stdlib.h>
class RawQCELPRTPSource : public MultiFramedRTPSource
{
public:
    static RawQCELPRTPSource * createNew(UsageEnvironment& env,
                                         Groupsock*      RTPgs,
                                         unsigned char   rtpPayloadFormat,
                                         unsigned        rtpTimestampFrequency);
    unsigned char interleaveL() const
    {
        return fInterleaveL;
    }

    unsigned char interleaveN() const
    {
        return fInterleaveN;
    }

    unsigned char& frameIndex()
    {
        return fFrameIndex;
    }

private:
    RawQCELPRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                      unsigned char rtpPayloadFormat,
                      unsigned rtpTimestampFrequency);
    virtual ~RawQCELPRTPSource();
private:
    virtual Boolean processSpecialHeader(BufferedPacket* packet,
                                         unsigned        & resultSpecialHeaderSize);
    virtual char const * MIMEtype() const;
    virtual Boolean hasBeenSynchronizedUsingRTCP();
private:
    unsigned char fInterleaveL, fInterleaveN, fFrameIndex;
    unsigned fNumSuccessiveSyncedPackets;
};
class QCELPDeinterleaver : public FramedFilter
{
public:
    static QCELPDeinterleaver * createNew(UsageEnvironment   & env,
                                          RawQCELPRTPSource* inputSource);
private:
    QCELPDeinterleaver(UsageEnvironment   & env,
                       RawQCELPRTPSource* inputSource);
    virtual ~QCELPDeinterleaver();
    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);
    void afterGettingFrame1(unsigned frameSize, struct timeval presentationTime);
private:
    void doGetNextFrame();
    virtual void doStopGettingFrames();
private:
    class QCELPDeinterleavingBuffer* fDeinterleavingBuffer;
    Boolean fNeedAFrame;
};
FramedSource * QCELPAudioRTPSource::createNew(UsageEnvironment& env,
                                              Groupsock*      RTPgs,
                                              RTPSource *     & resultRTPSource,
                                              unsigned char   rtpPayloadFormat,
                                              unsigned        rtpTimestampFrequency)
{
    RawQCELPRTPSource* rawRTPSource;

    resultRTPSource = rawRTPSource =
        RawQCELPRTPSource::createNew(env, RTPgs, rtpPayloadFormat,
                                     rtpTimestampFrequency);
    if (resultRTPSource == NULL) {
        return NULL;
    }
    QCELPDeinterleaver* deinterleaver =
        QCELPDeinterleaver::createNew(env, rawRTPSource);
    if (deinterleaver == NULL) {
        Medium::close(resultRTPSource);
        resultRTPSource = NULL;
    }
    return deinterleaver;
}

class QCELPBufferedPacket : public BufferedPacket
{
public:
    QCELPBufferedPacket(RawQCELPRTPSource& ourSource);
    virtual ~QCELPBufferedPacket();
private:
    virtual unsigned nextEnclosedFrameSize(unsigned char *& framePtr,
                                           unsigned       dataSize);
private:
    RawQCELPRTPSource& fOurSource;
};
class QCELPBufferedPacketFactory : public BufferedPacketFactory
{
private:
    virtual BufferedPacket * createNewPacket(MultiFramedRTPSource* ourSource);
};
RawQCELPRTPSource * RawQCELPRTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                 unsigned char rtpPayloadFormat,
                                                 unsigned rtpTimestampFrequency)
{
    return new RawQCELPRTPSource(env, RTPgs, rtpPayloadFormat,
                                 rtpTimestampFrequency);
}

RawQCELPRTPSource::RawQCELPRTPSource(UsageEnvironment& env,
                                     Groupsock*      RTPgs,
                                     unsigned char   rtpPayloadFormat,
                                     unsigned        rtpTimestampFrequency)
    : MultiFramedRTPSource(env, RTPgs, rtpPayloadFormat,
                           rtpTimestampFrequency,
                           new QCELPBufferedPacketFactory),
    fInterleaveL(0), fInterleaveN(0), fFrameIndex(0),
    fNumSuccessiveSyncedPackets(0)
{}

RawQCELPRTPSource::~RawQCELPRTPSource()
{}

Boolean RawQCELPRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    unsigned char* headerStart = packet->data();
    unsigned packetSize        = packet->dataSize();

    if (RTPSource::hasBeenSynchronizedUsingRTCP()) {
        ++fNumSuccessiveSyncedPackets;
    } else {
        fNumSuccessiveSyncedPackets = 0;
    }
    if (packetSize < 1) {
        return False;
    }
    unsigned char const firstByte   = headerStart[0];
    unsigned char const interleaveL = (firstByte & 0x38) >> 3;
    unsigned char const interleaveN = firstByte & 0x07;
    #ifdef DEBUG
    fprintf(stderr, "packetSize: %d, interleaveL: %d, interleaveN: %d\n", packetSize, interleaveL, interleaveN);
    #endif
    if (interleaveL > 5 || interleaveN > interleaveL) {
        return False;
    }
    fInterleaveL = interleaveL;
    fInterleaveN = interleaveN;
    fFrameIndex  = 0;
    resultSpecialHeaderSize = 1;
    return True;
}

char const * RawQCELPRTPSource::MIMEtype() const
{
    return "audio/QCELP";
}

Boolean RawQCELPRTPSource::hasBeenSynchronizedUsingRTCP()
{
    if (fNumSuccessiveSyncedPackets > (unsigned) (fInterleaveL + 1)) {
        fNumSuccessiveSyncedPackets = fInterleaveL + 2;
        return True;
    }
    return False;
}

QCELPBufferedPacket::QCELPBufferedPacket(RawQCELPRTPSource& ourSource)
    : fOurSource(ourSource)
{}

QCELPBufferedPacket::~QCELPBufferedPacket()
{}

unsigned QCELPBufferedPacket::
nextEnclosedFrameSize(unsigned char *& framePtr, unsigned dataSize)
{
    if (dataSize == 0) {
        return 0;
    }
    unsigned char const firstByte = framePtr[0];
    unsigned frameSize;
    switch (firstByte) {
        case 0: {
            frameSize = 1;
            break;
        }
        case 1: {
            frameSize = 4;
            break;
        }
        case 2: {
            frameSize = 8;
            break;
        }
        case 3: {
            frameSize = 17;
            break;
        }
        case 4: {
            frameSize = 35;
            break;
        }
        default: {
            frameSize = 0;
            break;
        }
    }
    #ifdef DEBUG
    fprintf(stderr, "QCELPBufferedPacket::nextEnclosedFrameSize(): frameSize: %d, dataSize: %d\n", frameSize, dataSize);
    #endif
    if (dataSize < frameSize) {
        return 0;
    }
    ++fOurSource.frameIndex();
    return frameSize;
} // QCELPBufferedPacket::nextEnclosedFrameSize

BufferedPacket * QCELPBufferedPacketFactory
::createNewPacket(MultiFramedRTPSource* ourSource)
{
    return new QCELPBufferedPacket((RawQCELPRTPSource&) (*ourSource));
}

#define QCELP_MAX_FRAME_SIZE        35
#define QCELP_MAX_INTERLEAVE_L      5
#define QCELP_MAX_FRAMES_PER_PACKET 10
#define QCELP_MAX_INTERLEAVE_GROUP_SIZE \
    ((QCELP_MAX_INTERLEAVE_L+1)*QCELP_MAX_FRAMES_PER_PACKET)
class QCELPDeinterleavingBuffer
{
public:
    QCELPDeinterleavingBuffer();
    virtual ~QCELPDeinterleavingBuffer();
    void deliverIncomingFrame(unsigned       frameSize,
                              unsigned char  interleaveL,
                              unsigned char  interleaveN,
                              unsigned char  frameIndex,
                              unsigned short packetSeqNum,
                              struct timeval presentationTime);
    Boolean retrieveFrame(unsigned char* to, unsigned maxSize,
                          unsigned& resultFrameSize, unsigned& resultNumTruncatedBytes,
                          struct timeval& resultPresentationTime);
    unsigned char * inputBuffer()
    {
        return fInputBuffer;
    }

    unsigned inputBufferSize() const
    {
        return QCELP_MAX_FRAME_SIZE;
    }

private:
    class FrameDescriptor
    {
public:
        FrameDescriptor();
        virtual ~FrameDescriptor();
        unsigned frameSize;
        unsigned char* frameData;
        struct timeval presentationTime;
    };
    FrameDescriptor fFrames[QCELP_MAX_INTERLEAVE_GROUP_SIZE][2];
    unsigned char fIncomingBankId;
    unsigned char fIncomingBinMax;
    unsigned char fOutgoingBinMax;
    unsigned char fNextOutgoingBin;
    Boolean fHaveSeenPackets;
    u_int16_t fLastPacketSeqNumForGroup;
    unsigned char* fInputBuffer;
    struct timeval fLastRetrievedPresentationTime;
};
QCELPDeinterleaver * QCELPDeinterleaver::createNew(UsageEnvironment   & env,
                                                   RawQCELPRTPSource* inputSource)
{
    return new QCELPDeinterleaver(env, inputSource);
}

QCELPDeinterleaver::QCELPDeinterleaver(UsageEnvironment   & env,
                                       RawQCELPRTPSource* inputSource)
    : FramedFilter(env, inputSource),
    fNeedAFrame(False)
{
    fDeinterleavingBuffer = new QCELPDeinterleavingBuffer();
}

QCELPDeinterleaver::~QCELPDeinterleaver()
{
    delete fDeinterleavingBuffer;
}

static unsigned const uSecsPerFrame = 20000;
void QCELPDeinterleaver::doGetNextFrame()
{
    if (fDeinterleavingBuffer->retrieveFrame(fTo, fMaxSize,
                                             fFrameSize, fNumTruncatedBytes,
                                             fPresentationTime))
    {
        fNeedAFrame = False;
        fDurationInMicroseconds = uSecsPerFrame;
        afterGetting(this);
        return;
    }
    fNeedAFrame = True;
    if (!fInputSource->isCurrentlyAwaitingData()) {
        fInputSource->getNextFrame(fDeinterleavingBuffer->inputBuffer(),
                                   fDeinterleavingBuffer->inputBufferSize(),
                                   afterGettingFrame, this,
                                   FramedSource::handleClosure, this);
    }
}

void QCELPDeinterleaver::doStopGettingFrames()
{
    fNeedAFrame = False;
    fInputSource->stopGettingFrames();
}

void QCELPDeinterleaver
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned,
                    struct timeval presentationTime,
                    unsigned)
{
    QCELPDeinterleaver* deinterleaver = (QCELPDeinterleaver *) clientData;

    deinterleaver->afterGettingFrame1(frameSize, presentationTime);
}

void QCELPDeinterleaver
::afterGettingFrame1(unsigned frameSize, struct timeval presentationTime)
{
    RawQCELPRTPSource* source = (RawQCELPRTPSource *) fInputSource;

    fDeinterleavingBuffer
    ->deliverIncomingFrame(frameSize, source->interleaveL(),
                           source->interleaveN(), source->frameIndex(),
                           source->curPacketRTPSeqNum(),
                           presentationTime);
    if (fNeedAFrame) {
        doGetNextFrame();
    }
}

QCELPDeinterleavingBuffer::QCELPDeinterleavingBuffer()
    : fIncomingBankId(0), fIncomingBinMax(0),
    fOutgoingBinMax(0), fNextOutgoingBin(0),
    fHaveSeenPackets(False)
{
    fInputBuffer = new unsigned char[QCELP_MAX_FRAME_SIZE];
}

QCELPDeinterleavingBuffer::~QCELPDeinterleavingBuffer()
{
    delete[] fInputBuffer;
}

void QCELPDeinterleavingBuffer
::deliverIncomingFrame(unsigned       frameSize,
                       unsigned char  interleaveL,
                       unsigned char  interleaveN,
                       unsigned char  frameIndex,
                       unsigned short packetSeqNum,
                       struct timeval presentationTime)
{
    if (frameSize > QCELP_MAX_FRAME_SIZE ||
        interleaveL > QCELP_MAX_INTERLEAVE_L || interleaveN > interleaveL ||
        frameIndex == 0 || frameIndex > QCELP_MAX_FRAMES_PER_PACKET)
    {
        #ifdef DEBUG
        fprintf(stderr, "QCELPDeinterleavingBuffer::deliverIncomingFrame() param sanity check failed (%d,%d,%d,%d)\n",
                frameSize, interleaveL, interleaveN, frameIndex);
        #endif
        return;
    }
    unsigned uSecIncrement = (frameIndex - 1) * (interleaveL + 1) * uSecsPerFrame;
    presentationTime.tv_usec += uSecIncrement;
    presentationTime.tv_sec  += presentationTime.tv_usec / 1000000;
    presentationTime.tv_usec  = presentationTime.tv_usec % 1000000;
    if (!fHaveSeenPackets ||
        seqNumLT(fLastPacketSeqNumForGroup, packetSeqNum))
    {
        fHaveSeenPackets = True;
        fLastPacketSeqNumForGroup = packetSeqNum + interleaveL - interleaveN;
        fIncomingBankId ^= 1;
        unsigned char tmp = fIncomingBinMax;
        fIncomingBinMax  = fOutgoingBinMax;
        fOutgoingBinMax  = tmp;
        fNextOutgoingBin = 0;
    }
    unsigned const binNumber =
        interleaveN + (frameIndex - 1) * (interleaveL + 1);
    FrameDescriptor& inBin   = fFrames[binNumber][fIncomingBankId];
    unsigned char* curBuffer = inBin.frameData;
    inBin.frameData        = fInputBuffer;
    inBin.frameSize        = frameSize;
    inBin.presentationTime = presentationTime;
    if (curBuffer == NULL) {
        curBuffer = new unsigned char[QCELP_MAX_FRAME_SIZE];
    }
    fInputBuffer = curBuffer;
    if (binNumber >= fIncomingBinMax) {
        fIncomingBinMax = binNumber + 1;
    }
} // QCELPDeinterleavingBuffer::deliverIncomingFrame

Boolean QCELPDeinterleavingBuffer
::retrieveFrame(unsigned char* to, unsigned maxSize,
                unsigned& resultFrameSize, unsigned& resultNumTruncatedBytes,
                struct timeval& resultPresentationTime)
{
    if (fNextOutgoingBin >= fOutgoingBinMax) {
        return False;
    }
    FrameDescriptor& outBin = fFrames[fNextOutgoingBin][fIncomingBankId ^ 1];
    unsigned char* fromPtr;
    unsigned char fromSize = outBin.frameSize;
    outBin.frameSize = 0;
    unsigned char erasure = 14;
    if (fromSize == 0) {
        fromPtr  = &erasure;
        fromSize = 1;
        resultPresentationTime          = fLastRetrievedPresentationTime;
        resultPresentationTime.tv_usec += uSecsPerFrame;
        if (resultPresentationTime.tv_usec >= 1000000) {
            ++resultPresentationTime.tv_sec;
            resultPresentationTime.tv_usec -= 1000000;
        }
    } else {
        fromPtr = outBin.frameData;
        resultPresentationTime = outBin.presentationTime;
    }
    fLastRetrievedPresentationTime = resultPresentationTime;
    if (fromSize > maxSize) {
        resultNumTruncatedBytes = fromSize - maxSize;
        resultFrameSize         = maxSize;
    } else {
        resultNumTruncatedBytes = 0;
        resultFrameSize         = fromSize;
    }
    memmove(to, fromPtr, resultFrameSize);
    ++fNextOutgoingBin;
    return True;
} // QCELPDeinterleavingBuffer::retrieveFrame

QCELPDeinterleavingBuffer::FrameDescriptor::FrameDescriptor()
    : frameSize(0), frameData(NULL)
{}

QCELPDeinterleavingBuffer::FrameDescriptor::~FrameDescriptor()
{
    delete[] frameData;
}
