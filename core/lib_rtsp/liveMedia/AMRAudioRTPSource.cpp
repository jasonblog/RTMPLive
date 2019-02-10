#include "AMRAudioRTPSource.hh"
#include "MultiFramedRTPSource.hh"
#include "BitVector.hh"
#include <string.h>
#include <stdlib.h>
class RawAMRRTPSource : public MultiFramedRTPSource
{
public:
    static RawAMRRTPSource * createNew(UsageEnvironment& env,
                                       Groupsock* RTPgs, unsigned char rtpPayloadFormat,
                                       Boolean isWideband, Boolean isOctetAligned,
                                       Boolean isInterleaved, Boolean CRCsArePresent);
    Boolean isWideband() const
    {
        return fIsWideband;
    }

    unsigned char ILL() const
    {
        return fILL;
    }

    unsigned char ILP() const
    {
        return fILP;
    }

    unsigned TOCSize() const
    {
        return fTOCSize;
    }

    unsigned char * TOC() const
    {
        return fTOC;
    }

    unsigned& frameIndex()
    {
        return fFrameIndex;
    }

    Boolean& isSynchronized()
    {
        return fIsSynchronized;
    }

private:
    RawAMRRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                    unsigned char rtpPayloadFormat,
                    Boolean isWideband, Boolean isOctetAligned,
                    Boolean isInterleaved, Boolean CRCsArePresent);
    virtual ~RawAMRRTPSource();
private:
    virtual Boolean hasBeenSynchronizedUsingRTCP();
    virtual Boolean processSpecialHeader(BufferedPacket* packet,
                                         unsigned        & resultSpecialHeaderSize);
    virtual char const * MIMEtype() const;
private:
    Boolean fIsWideband, fIsOctetAligned, fIsInterleaved, fCRCsArePresent;
    unsigned char fILL, fILP;
    unsigned fTOCSize;
    unsigned char* fTOC;
    unsigned fFrameIndex;
    Boolean fIsSynchronized;
};
class AMRDeinterleaver : public AMRAudioSource
{
public:
    static AMRDeinterleaver * createNew(UsageEnvironment& env,
                                        Boolean isWideband, unsigned numChannels, unsigned maxInterleaveGroupSize,
                                        RawAMRRTPSource* inputSource);
private:
    AMRDeinterleaver(UsageEnvironment& env,
                     Boolean isWideband, unsigned numChannels,
                     unsigned maxInterleaveGroupSize, RawAMRRTPSource* inputSource);
    virtual ~AMRDeinterleaver();
    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);
    void afterGettingFrame1(unsigned frameSize, struct timeval presentationTime);
private:
    void doGetNextFrame();
    virtual void doStopGettingFrames();
private:
    RawAMRRTPSource* fInputSource;
    class AMRDeinterleavingBuffer* fDeinterleavingBuffer;
    Boolean fNeedAFrame;
};
#define MAX_NUM_CHANNELS            20
#define MAX_INTERLEAVING_GROUP_SIZE 1000
AMRAudioSource * AMRAudioRTPSource::createNew(UsageEnvironment& env,
                                              Groupsock*      RTPgs,
                                              RTPSource *     & resultRTPSource,
                                              unsigned char   rtpPayloadFormat,
                                              Boolean         isWideband,
                                              unsigned        numChannels,
                                              Boolean         isOctetAligned,
                                              unsigned        interleaving,
                                              Boolean         robustSortingOrder,
                                              Boolean         CRCsArePresent)
{
    if (robustSortingOrder) {
        env << "AMRAudioRTPSource::createNew(): 'Robust sorting order' was specified, but we don't yet support this!\n";
        return NULL;
    } else if (numChannels > MAX_NUM_CHANNELS) {
        env << "AMRAudioRTPSource::createNew(): The \"number of channels\" parameter ("
            << numChannels << ") is much too large!\n";
        return NULL;
    } else if (interleaving > MAX_INTERLEAVING_GROUP_SIZE) {
        env << "AMRAudioRTPSource::createNew(): The \"interleaving\" parameter ("
            << interleaving << ") is much too large!\n";
        return NULL;
    }
    if (!isOctetAligned) {
        if (interleaving > 0 || robustSortingOrder || CRCsArePresent) {
            env
                <<
                "AMRAudioRTPSource::createNew(): 'Bandwidth-efficient mode' was specified, along with interleaving, 'robust sorting order', and/or CRCs, so we assume 'octet-aligned mode' instead.\n";
            isOctetAligned = True;
        }
    }
    Boolean isInterleaved;
    unsigned maxInterleaveGroupSize;
    if (interleaving > 0) {
        isInterleaved = True;
        maxInterleaveGroupSize = interleaving * numChannels;
    } else {
        isInterleaved = False;
        maxInterleaveGroupSize = numChannels;
    }
    RawAMRRTPSource* rawRTPSource;
    resultRTPSource = rawRTPSource =
        RawAMRRTPSource::createNew(env, RTPgs, rtpPayloadFormat,
                                   isWideband, isOctetAligned,
                                   isInterleaved, CRCsArePresent);
    if (resultRTPSource == NULL) {
        return NULL;
    }
    AMRDeinterleaver* deinterleaver =
        AMRDeinterleaver::createNew(env, isWideband, numChannels,
                                    maxInterleaveGroupSize, rawRTPSource);
    if (deinterleaver == NULL) {
        Medium::close(resultRTPSource);
        resultRTPSource = NULL;
    }
    return deinterleaver;
} // AMRAudioRTPSource::createNew

class AMRBufferedPacket : public BufferedPacket
{
public:
    AMRBufferedPacket(RawAMRRTPSource& ourSource);
    virtual ~AMRBufferedPacket();
private:
    virtual unsigned nextEnclosedFrameSize(unsigned char *& framePtr,
                                           unsigned       dataSize);
private:
    RawAMRRTPSource& fOurSource;
};
class AMRBufferedPacketFactory : public BufferedPacketFactory
{
private:
    virtual BufferedPacket * createNewPacket(MultiFramedRTPSource* ourSource);
};
RawAMRRTPSource * RawAMRRTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                             unsigned char rtpPayloadFormat,
                                             Boolean isWideband, Boolean isOctetAligned,
                                             Boolean isInterleaved, Boolean CRCsArePresent)
{
    return new RawAMRRTPSource(env, RTPgs, rtpPayloadFormat,
                               isWideband, isOctetAligned,
                               isInterleaved, CRCsArePresent);
}

RawAMRRTPSource
::RawAMRRTPSource(UsageEnvironment& env,
                  Groupsock* RTPgs, unsigned char rtpPayloadFormat,
                  Boolean isWideband, Boolean isOctetAligned,
                  Boolean isInterleaved, Boolean CRCsArePresent)
    : MultiFramedRTPSource(env, RTPgs, rtpPayloadFormat,
                           isWideband ? 16000 : 8000,
                           new AMRBufferedPacketFactory),
    fIsWideband(isWideband), fIsOctetAligned(isOctetAligned),
    fIsInterleaved(isInterleaved), fCRCsArePresent(CRCsArePresent),
    fILL(0), fILP(0), fTOCSize(0), fTOC(NULL), fFrameIndex(0), fIsSynchronized(False)
{}

RawAMRRTPSource::~RawAMRRTPSource()
{
    delete[] fTOC;
}

#define FT_SPEECH_LOST 14
#define FT_NO_DATA     15
static void unpackBandwidthEfficientData(BufferedPacket* packet,
                                         Boolean         isWideband);
Boolean RawAMRRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    if (!fIsOctetAligned) {
        unpackBandwidthEfficientData(packet, fIsWideband);
    }
    unsigned char* headerStart = packet->data();
    unsigned packetSize        = packet->dataSize();
    if (packetSize < 1) {
        return False;
    }
    resultSpecialHeaderSize = 1;
    if (fIsInterleaved) {
        if (packetSize < 2) {
            return False;
        }
        unsigned char const secondByte = headerStart[1];
        fILL = (secondByte & 0xF0) >> 4;
        fILP = secondByte & 0x0F;
        if (fILP > fILL) {
            return False;
        }
        ++resultSpecialHeaderSize;
    }
    #ifdef DEBUG
    fprintf(stderr, "packetSize: %d, ILL: %d, ILP: %d\n", packetSize, fILL, fILP);
    #endif
    fFrameIndex = 0;
    unsigned numFramesPresent = 0, numNonEmptyFramesPresent = 0;
    unsigned tocStartIndex = resultSpecialHeaderSize;
    Boolean F;
    do {
        if (resultSpecialHeaderSize >= packetSize) {
            return False;
        }
        unsigned char const tocByte = headerStart[resultSpecialHeaderSize++];
        F = (tocByte & 0x80) != 0;
        unsigned char const FT = (tocByte & 0x78) >> 3;
        #ifdef DEBUG
        unsigned char Q = (tocByte & 0x04) >> 2;
        fprintf(stderr, "\tTOC entry: F %d, FT %d, Q %d\n", F, FT, Q);
        #endif
        ++numFramesPresent;
        if (FT != FT_SPEECH_LOST && FT != FT_NO_DATA) {
            ++numNonEmptyFramesPresent;
        }
    } while (F);
    #ifdef DEBUG
    fprintf(stderr, "TOC contains %d entries (%d non-empty)\n", numFramesPresent, numNonEmptyFramesPresent);
    #endif
    if (numFramesPresent > fTOCSize) {
        delete[] fTOC;
        fTOC = new unsigned char[numFramesPresent];
    }
    fTOCSize = numFramesPresent;
    for (unsigned i = 0; i < fTOCSize; ++i) {
        unsigned char const tocByte = headerStart[tocStartIndex + i];
        fTOC[i] = tocByte & 0x7C;
    }
    if (fCRCsArePresent) {
        resultSpecialHeaderSize += numNonEmptyFramesPresent;
        #ifdef DEBUG
        fprintf(stderr, "Ignoring %d following CRC bytes\n", numNonEmptyFramesPresent);
        #endif
        if (resultSpecialHeaderSize > packetSize) {
            return False;
        }
    }
    #ifdef DEBUG
    fprintf(stderr, "Total special header size: %d\n", resultSpecialHeaderSize);
    #endif
    return True;
} // RawAMRRTPSource::processSpecialHeader

char const * RawAMRRTPSource::MIMEtype() const
{
    return fIsWideband ? "audio/AMR-WB" : "audio/AMR";
}

Boolean RawAMRRTPSource::hasBeenSynchronizedUsingRTCP()
{
    return fIsSynchronized;
}

AMRBufferedPacket::AMRBufferedPacket(RawAMRRTPSource& ourSource)
    : fOurSource(ourSource)
{}

AMRBufferedPacket::~AMRBufferedPacket()
{}

#define FT_INVALID 65535
static unsigned short const frameBytesFromFT[16] = {
    12,         13,         15,         17,
    19,         20,         26,         31,
    5,          FT_INVALID, FT_INVALID, FT_INVALID,
    FT_INVALID, FT_INVALID, FT_INVALID, 0
};
static unsigned short const frameBytesFromFTWideband[16] = {
    17,         23,         32,         36,
    40,         46,         50,         58,
    60,         5,          FT_INVALID, FT_INVALID,
    FT_INVALID, FT_INVALID, 0,          0
};
unsigned AMRBufferedPacket::
nextEnclosedFrameSize(unsigned char *& framePtr, unsigned dataSize)
{
    if (dataSize == 0) {
        return 0;
    }
    unsigned const tocIndex = fOurSource.frameIndex();
    if (tocIndex >= fOurSource.TOCSize()) {
        return 0;
    }
    unsigned char const tocByte = fOurSource.TOC()[tocIndex];
    unsigned char const FT      = (tocByte & 0x78) >> 3;
    unsigned short frameSize    =
        fOurSource.isWideband() ? frameBytesFromFTWideband[FT] : frameBytesFromFT[FT];
    if (frameSize == FT_INVALID) {
        fOurSource.envir() << "AMRBufferedPacket::nextEnclosedFrameSize(): invalid FT: " << FT << "\n";
        frameSize = 0;
    }
    #ifdef DEBUG
    fprintf(stderr,
            "AMRBufferedPacket::nextEnclosedFrameSize(): frame #: %d, FT: %d, isWideband: %d => frameSize: %d (dataSize: %d)\n", tocIndex, FT,
            fOurSource.isWideband(), frameSize, dataSize);
    #endif
    ++fOurSource.frameIndex();
    if (dataSize < frameSize) {
        return 0;
    }
    return frameSize;
}

BufferedPacket * AMRBufferedPacketFactory
::createNewPacket(MultiFramedRTPSource* ourSource)
{
    return new AMRBufferedPacket((RawAMRRTPSource&) (*ourSource));
}

#define AMR_MAX_FRAME_SIZE 60
class AMRDeinterleavingBuffer
{
public:
    AMRDeinterleavingBuffer(unsigned numChannels, unsigned maxInterleaveGroupSize);
    virtual ~AMRDeinterleavingBuffer();
    void deliverIncomingFrame(unsigned frameSize, RawAMRRTPSource* source,
                              struct timeval presentationTime);
    Boolean retrieveFrame(unsigned char* to, unsigned maxSize,
                          unsigned& resultFrameSize, unsigned& resultNumTruncatedBytes,
                          u_int8_t& resultFrameHeader,
                          struct timeval& resultPresentationTime,
                          Boolean& resultIsSynchronized);
    unsigned char * inputBuffer()
    {
        return fInputBuffer;
    }

    unsigned inputBufferSize() const
    {
        return AMR_MAX_FRAME_SIZE;
    }

private:
    unsigned char * createNewBuffer();
    class FrameDescriptor
    {
public:
        FrameDescriptor();
        virtual ~FrameDescriptor();
        unsigned frameSize;
        unsigned char* frameData;
        u_int8_t frameHeader;
        struct timeval presentationTime;
        Boolean fIsSynchronized;
    };
    unsigned fNumChannels, fMaxInterleaveGroupSize;
    FrameDescriptor* fFrames[2];
    unsigned char fIncomingBankId;
    unsigned char fIncomingBinMax;
    unsigned char fOutgoingBinMax;
    unsigned char fNextOutgoingBin;
    Boolean fHaveSeenPackets;
    u_int16_t fLastPacketSeqNumForGroup;
    unsigned char* fInputBuffer;
    struct timeval fLastRetrievedPresentationTime;
    unsigned fNumSuccessiveSyncedFrames;
    unsigned char fILL;
};
AMRDeinterleaver * AMRDeinterleaver
::createNew(UsageEnvironment& env,
            Boolean isWideband, unsigned numChannels, unsigned maxInterleaveGroupSize,
            RawAMRRTPSource* inputSource)
{
    return new AMRDeinterleaver(env, isWideband, numChannels, maxInterleaveGroupSize, inputSource);
}

AMRDeinterleaver::AMRDeinterleaver(UsageEnvironment& env,
                                   Boolean isWideband, unsigned numChannels,
                                   unsigned maxInterleaveGroupSize,
                                   RawAMRRTPSource* inputSource)
    : AMRAudioSource(env, isWideband, numChannels),
    fInputSource(inputSource), fNeedAFrame(False)
{
    fDeinterleavingBuffer =
        new AMRDeinterleavingBuffer(numChannels, maxInterleaveGroupSize);
}

AMRDeinterleaver::~AMRDeinterleaver()
{
    delete fDeinterleavingBuffer;
    Medium::close(fInputSource);
}

static unsigned const uSecsPerFrame = 20000;
void AMRDeinterleaver::doGetNextFrame()
{
    if (fDeinterleavingBuffer->retrieveFrame(fTo, fMaxSize,
                                             fFrameSize, fNumTruncatedBytes,
                                             fLastFrameHeader, fPresentationTime,
                                             fInputSource->isSynchronized()))
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

void AMRDeinterleaver::doStopGettingFrames()
{
    fNeedAFrame = False;
    fInputSource->stopGettingFrames();
}

void AMRDeinterleaver
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned,
                    struct timeval presentationTime,
                    unsigned)
{
    AMRDeinterleaver* deinterleaver = (AMRDeinterleaver *) clientData;

    deinterleaver->afterGettingFrame1(frameSize, presentationTime);
}

void AMRDeinterleaver
::afterGettingFrame1(unsigned frameSize, struct timeval presentationTime)
{
    RawAMRRTPSource* source = (RawAMRRTPSource *) fInputSource;

    fDeinterleavingBuffer->deliverIncomingFrame(frameSize, source, presentationTime);
    if (fNeedAFrame) {
        doGetNextFrame();
    }
}

AMRDeinterleavingBuffer
::AMRDeinterleavingBuffer(unsigned numChannels, unsigned maxInterleaveGroupSize)
    : fNumChannels(numChannels), fMaxInterleaveGroupSize(maxInterleaveGroupSize),
    fIncomingBankId(0), fIncomingBinMax(0),
    fOutgoingBinMax(0), fNextOutgoingBin(0),
    fHaveSeenPackets(False), fNumSuccessiveSyncedFrames(0), fILL(0)
{
    fFrames[0]   = new FrameDescriptor[fMaxInterleaveGroupSize];
    fFrames[1]   = new FrameDescriptor[fMaxInterleaveGroupSize];
    fInputBuffer = createNewBuffer();
}

AMRDeinterleavingBuffer::~AMRDeinterleavingBuffer()
{
    delete[] fInputBuffer;
    delete[] fFrames[0];
    delete[] fFrames[1];
}

void AMRDeinterleavingBuffer
::deliverIncomingFrame(unsigned frameSize, RawAMRRTPSource* source,
                       struct timeval presentationTime)
{
    fILL = source->ILL();
    unsigned char const ILP     = source->ILP();
    unsigned frameIndex         = source->frameIndex();
    unsigned short packetSeqNum = source->curPacketRTPSeqNum();
    if (ILP > fILL || frameIndex == 0) {
        #ifdef DEBUG
        fprintf(stderr, "AMRDeinterleavingBuffer::deliverIncomingFrame() param sanity check failed (%d,%d,%d,%d)\n",
                frameSize, fILL, ILP, frameIndex);
        #endif
        source->envir().internalError();
    }
    --frameIndex;
    u_int8_t frameHeader;
    if (frameIndex >= source->TOCSize()) {
        frameHeader = FT_NO_DATA << 3;
    } else {
        frameHeader = source->TOC()[frameIndex];
    }
    unsigned frameBlockIndex       = frameIndex / fNumChannels;
    unsigned frameWithinFrameBlock = frameIndex % fNumChannels;
    unsigned uSecIncrement         = frameBlockIndex * (fILL + 1) * uSecsPerFrame;
    presentationTime.tv_usec += uSecIncrement;
    presentationTime.tv_sec  += presentationTime.tv_usec / 1000000;
    presentationTime.tv_usec  = presentationTime.tv_usec % 1000000;
    if (!fHaveSeenPackets ||
        seqNumLT(fLastPacketSeqNumForGroup, packetSeqNum + frameBlockIndex))
    {
        #ifdef DEBUG
        fprintf(stderr, "AMRDeinterleavingBuffer::deliverIncomingFrame(): new interleave group\n");
        #endif
        fHaveSeenPackets = True;
        fLastPacketSeqNumForGroup = packetSeqNum + fILL - ILP;
        fIncomingBankId ^= 1;
        unsigned char tmp = fIncomingBinMax;
        fIncomingBinMax  = fOutgoingBinMax;
        fOutgoingBinMax  = tmp;
        fNextOutgoingBin = 0;
    }
    unsigned const binNumber =
        ((ILP + frameBlockIndex * (fILL + 1)) * fNumChannels + frameWithinFrameBlock)
        % fMaxInterleaveGroupSize;
    #ifdef DEBUG
    fprintf(stderr,
            "AMRDeinterleavingBuffer::deliverIncomingFrame(): frameIndex %d (%d,%d) put in bank %d, bin %d (%d): size %d, header 0x%02x, presentationTime %lu.%06ld\n", frameIndex, frameBlockIndex, frameWithinFrameBlock, fIncomingBankId, binNumber, fMaxInterleaveGroupSize, frameSize, frameHeader, presentationTime.tv_sec,
            presentationTime.tv_usec);
    #endif
    FrameDescriptor& inBin   = fFrames[fIncomingBankId][binNumber];
    unsigned char* curBuffer = inBin.frameData;
    inBin.frameData        = fInputBuffer;
    inBin.frameSize        = frameSize;
    inBin.frameHeader      = frameHeader;
    inBin.presentationTime = presentationTime;
    inBin.fIsSynchronized  = ((RTPSource *) source)->RTPSource::hasBeenSynchronizedUsingRTCP();
    if (curBuffer == NULL) {
        curBuffer = createNewBuffer();
    }
    fInputBuffer = curBuffer;
    if (binNumber >= fIncomingBinMax) {
        fIncomingBinMax = binNumber + 1;
    }
} // AMRDeinterleavingBuffer::deliverIncomingFrame

Boolean AMRDeinterleavingBuffer
::retrieveFrame(unsigned char* to, unsigned maxSize,
                unsigned& resultFrameSize, unsigned& resultNumTruncatedBytes,
                u_int8_t& resultFrameHeader,
                struct timeval& resultPresentationTime,
                Boolean& resultIsSynchronized)
{
    if (fNextOutgoingBin >= fOutgoingBinMax) {
        return False;
    }
    FrameDescriptor& outBin = fFrames[fIncomingBankId ^ 1][fNextOutgoingBin];
    unsigned char* fromPtr  = outBin.frameData;
    unsigned char fromSize  = outBin.frameSize;
    outBin.frameSize     = 0;
    resultIsSynchronized = False;
    if (outBin.fIsSynchronized) {
        if (++fNumSuccessiveSyncedFrames > fILL) {
            resultIsSynchronized       = True;
            fNumSuccessiveSyncedFrames = fILL + 1;
        }
    } else {
        fNumSuccessiveSyncedFrames = 0;
    }
    if (fromSize == 0) {
        resultFrameHeader               = FT_NO_DATA << 3;
        resultPresentationTime          = fLastRetrievedPresentationTime;
        resultPresentationTime.tv_usec += uSecsPerFrame;
        if (resultPresentationTime.tv_usec >= 1000000) {
            ++resultPresentationTime.tv_sec;
            resultPresentationTime.tv_usec -= 1000000;
        }
    } else {
        resultFrameHeader      = outBin.frameHeader;
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
    #ifdef DEBUG
    fprintf(stderr,
            "AMRDeinterleavingBuffer::retrieveFrame(): from bank %d, bin %d: size %d, header 0x%02x, presentationTime %lu.%06ld\n", fIncomingBankId ^ 1, fNextOutgoingBin, resultFrameSize, resultFrameHeader, resultPresentationTime.tv_sec,
            resultPresentationTime.tv_usec);
    #endif
    ++fNextOutgoingBin;
    return True;
} // AMRDeinterleavingBuffer::retrieveFrame

unsigned char * AMRDeinterleavingBuffer::createNewBuffer()
{
    return new unsigned char[inputBufferSize()];
}

AMRDeinterleavingBuffer::FrameDescriptor::FrameDescriptor()
    : frameSize(0), frameData(NULL)
{}

AMRDeinterleavingBuffer::FrameDescriptor::~FrameDescriptor()
{
    delete[] frameData;
}

static unsigned short const frameBitsFromFT[16] = {
    95,  103, 118, 134,
    148, 159, 204, 244,
    39,  0,   0,   0,
    0,   0,   0,   0
};
static unsigned short const frameBitsFromFTWideband[16] = {
    132, 177, 253, 285,
    317, 365, 397, 461,
    477, 40,  0,   0,
    0,   0,   0,   0
};
static void unpackBandwidthEfficientData(BufferedPacket* packet,
                                         Boolean         isWideband)
{
    #ifdef DEBUG
    fprintf(stderr, "Unpacking 'bandwidth-efficient' payload (%d bytes):\n", packet->dataSize());
    for (unsigned j = 0; j < packet->dataSize(); ++j) {
        fprintf(stderr, "%02x:", (packet->data())[j]);
    }
    fprintf(stderr, "\n");
    #endif
    BitVector fromBV(packet->data(), 0, 8 * packet->dataSize());
    unsigned const toBufferSize = 2 * packet->dataSize();
    unsigned char* toBuffer     = new unsigned char[toBufferSize];
    unsigned toCount = 0;
    unsigned CMR     = fromBV.getBits(4);
    toBuffer[toCount++] = CMR << 4;
    while (1) {
        unsigned toc = fromBV.getBits(6);
        toBuffer[toCount++] = toc << 2;
        if ((toc & 0x20) == 0) {
            break;
        }
    }
    unsigned const tocSize = toCount - 1;
    for (unsigned i = 1; i <= tocSize; ++i) {
        unsigned char tocByte        = toBuffer[i];
        unsigned char const FT       = (tocByte & 0x78) >> 3;
        unsigned short frameSizeBits =
            isWideband ? frameBitsFromFTWideband[FT] : frameBitsFromFT[FT];
        unsigned short frameSizeBytes = (frameSizeBits + 7) / 8;
        shiftBits(&toBuffer[toCount], 0,
                  packet->data(), fromBV.curBitIndex(),
                  frameSizeBits
        );
        #ifdef DEBUG
        if (frameSizeBits > fromBV.numBitsRemaining()) {
            fprintf(stderr, "\tWarning: Unpacking frame %d of %d: want %d bits, but only %d are available!\n", i,
                    tocSize, frameSizeBits, fromBV.numBitsRemaining());
        }
        #endif
        fromBV.skipBits(frameSizeBits);
        toCount += frameSizeBytes;
    }
    #ifdef DEBUG
    if (fromBV.numBitsRemaining() > 7) {
        fprintf(stderr, "\tWarning: %d bits remain unused!\n", fromBV.numBitsRemaining());
    }
    #endif
    packet->removePadding(packet->dataSize());
    packet->appendData(toBuffer, toCount);
    delete[] toBuffer;
} // unpackBandwidthEfficientData
