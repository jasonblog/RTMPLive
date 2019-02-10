#include "MultiFramedRTPSource.hh"
#include "RTCP.hh"
#include "GroupsockHelper.hh"
#include <string.h>
class ReorderingPacketBuffer
{
public:
    ReorderingPacketBuffer(BufferedPacketFactory* packetFactory);
    virtual ~ReorderingPacketBuffer();
    void reset();
    BufferedPacket * getFreePacket(MultiFramedRTPSource* ourSource);
    Boolean storePacket(BufferedPacket* bPacket);
    BufferedPacket * getNextCompletedPacket(Boolean& packetLossPreceded);
    void releaseUsedPacket(BufferedPacket* packet);
    void freePacket(BufferedPacket* packet)
    {
        if (packet != fSavedPacket) {
            delete packet;
        } else {
            fSavedPacketFree = True;
        }
    }

    Boolean isEmpty() const
    {
        return fHeadPacket == NULL;
    }

    void setThresholdTime(unsigned uSeconds)
    {
        fThresholdTime = uSeconds;
    }

    void resetHaveSeenFirstPacket()
    {
        fHaveSeenFirstPacket = False;
    }

private:
    BufferedPacketFactory* fPacketFactory;
    unsigned fThresholdTime;
    Boolean fHaveSeenFirstPacket;
    unsigned short fNextExpectedSeqNo;
    BufferedPacket* fHeadPacket;
    BufferedPacket* fTailPacket;
    BufferedPacket* fSavedPacket;
    Boolean fSavedPacketFree;
};
MultiFramedRTPSource
::MultiFramedRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                       unsigned char rtpPayloadFormat,
                       unsigned rtpTimestampFrequency,
                       BufferedPacketFactory* packetFactory)
    : RTPSource(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency)
{
    reset();
    fReorderingBuffer = new ReorderingPacketBuffer(packetFactory);
    increaseReceiveBufferTo(env, RTPgs->socketNum(), 50 * 1024);
}

void MultiFramedRTPSource::reset()
{
    fCurrentPacketBeginsFrame    = True;
    fCurrentPacketCompletesFrame = True;
    fAreDoingNetworkReads        = False;
    fPacketReadInProgress        = NULL;
    fNeedDelivery = False;
    fPacketLossInFragmentedFrame = False;
}

MultiFramedRTPSource::~MultiFramedRTPSource()
{
    delete fReorderingBuffer;
}

Boolean MultiFramedRTPSource
::processSpecialHeader(BufferedPacket *,
                       unsigned& resultSpecialHeaderSize)
{
    resultSpecialHeaderSize = 0;
    return True;
}

Boolean MultiFramedRTPSource
::packetIsUsableInJitterCalculation(unsigned char *,
                                    unsigned)
{
    return True;
}

void MultiFramedRTPSource::doStopGettingFrames()
{
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    fRTPInterface.stopNetworkReading();
    fReorderingBuffer->reset();
    reset();
}

void MultiFramedRTPSource::doGetNextFrame()
{
    if (!fAreDoingNetworkReads) {
        fAreDoingNetworkReads = True;
        TaskScheduler::BackgroundHandlerProc* handler =
            (TaskScheduler::BackgroundHandlerProc *) &networkReadHandler;
        fRTPInterface.startNetworkReading(handler);
    }
    fSavedTo      = fTo;
    fSavedMaxSize = fMaxSize;
    fFrameSize    = 0;
    fNeedDelivery = True;
    doGetNextFrame1();
}

void MultiFramedRTPSource::doGetNextFrame1()
{
    while (fNeedDelivery) {
        Boolean packetLossPrecededThis;
        BufferedPacket* nextPacket =
            fReorderingBuffer->getNextCompletedPacket(packetLossPrecededThis);
        if (nextPacket == NULL) {
            break;
        }
        fNeedDelivery = False;
        if (nextPacket->useCount() == 0) {
            unsigned specialHeaderSize;
            if (!processSpecialHeader(nextPacket, specialHeaderSize)) {
                fReorderingBuffer->releaseUsedPacket(nextPacket);
                fNeedDelivery = True;
                break;
            }
            nextPacket->skip(specialHeaderSize);
        }
        if (fCurrentPacketBeginsFrame) {
            if (packetLossPrecededThis || fPacketLossInFragmentedFrame) {
                fTo        = fSavedTo;
                fMaxSize   = fSavedMaxSize;
                fFrameSize = 0;
            }
            fPacketLossInFragmentedFrame = False;
        } else if (packetLossPrecededThis) {
            fPacketLossInFragmentedFrame = True;
        }
        if (fPacketLossInFragmentedFrame) {
            fReorderingBuffer->releaseUsedPacket(nextPacket);
            fNeedDelivery = True;
            break;
        }
        unsigned frameSize;
        nextPacket->use(fTo, fMaxSize, frameSize, fNumTruncatedBytes,
                        fCurPacketRTPSeqNum, fCurPacketRTPTimestamp,
                        fPresentationTime, fCurPacketHasBeenSynchronizedUsingRTCP,
                        fCurPacketMarkerBit);
        fFrameSize += frameSize;
        if (!nextPacket->hasUsableData()) {
            fReorderingBuffer->releaseUsedPacket(nextPacket);
        }
        if (fCurrentPacketCompletesFrame) {
            if (fNumTruncatedBytes > 0) {
                envir()
                    <<
                    "MultiFramedRTPSource::doGetNextFrame1(): The total received frame size exceeds the client's buffer size ("
                    << fSavedMaxSize << ").  "
                    << fNumTruncatedBytes << " bytes of trailing data will be dropped!\n";
            }
            if (fReorderingBuffer->isEmpty()) {
                afterGetting(this);
            } else {
                nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                                                                         (TaskFunc *) FramedSource::afterGetting, this);
            }
        } else {
            fTo          += frameSize;
            fMaxSize     -= frameSize;
            fNeedDelivery = True;
        }
    }
} // MultiFramedRTPSource::doGetNextFrame1

void MultiFramedRTPSource
::setPacketReorderingThresholdTime(unsigned uSeconds)
{
    fReorderingBuffer->setThresholdTime(uSeconds);
}

#define ADVANCE(n) do { bPacket->skip(n); } while (0)
void MultiFramedRTPSource::networkReadHandler(MultiFramedRTPSource* source, int)
{
    source->networkReadHandler1();
}

void MultiFramedRTPSource::networkReadHandler1()
{
    BufferedPacket* bPacket = fPacketReadInProgress;

    if (bPacket == NULL) {
        bPacket = fReorderingBuffer->getFreePacket(this);
    }
    Boolean readSuccess = False;
    do {
        struct sockaddr_in fromAddress;
        Boolean packetReadWasIncomplete = fPacketReadInProgress != NULL;
        if (!bPacket->fillInData(fRTPInterface, fromAddress, packetReadWasIncomplete)) {
            if (bPacket->bytesAvailable() == 0) {
                envir()
                    <<
                    "MultiFramedRTPSource error: Hit limit when reading incoming packet over TCP. Increase \"MAX_PACKET_SIZE\"\n";
            }
            fPacketReadInProgress = NULL;
            break;
        }
        if (packetReadWasIncomplete) {
            fPacketReadInProgress = bPacket;
            return;
        } else {
            fPacketReadInProgress = NULL;
        }
        #ifdef TEST_LOSS
        setPacketReorderingThresholdTime(0);
        if ((our_random() % 10) == 0) {
            break;
        }
        #endif
        if (bPacket->dataSize() < 12) {
            break;
        }
        unsigned rtpHdr = ntohl(*(u_int32_t *) (bPacket->data()));
        ADVANCE(4);
        Boolean rtpMarkerBit  = (rtpHdr & 0x00800000) != 0;
        unsigned rtpTimestamp = ntohl(*(u_int32_t *) (bPacket->data()));
        ADVANCE(4);
        unsigned rtpSSRC = ntohl(*(u_int32_t *) (bPacket->data()));
        ADVANCE(4);
        if ((rtpHdr & 0xC0000000) != 0x80000000) {
            break;
        }
        unsigned char rtpPayloadType = (unsigned char) ((rtpHdr & 0x007F0000) >> 16);
        if (rtpPayloadType != rtpPayloadFormat()) {
            if (fRTCPInstanceForMultiplexedRTCPPackets != NULL &&
                rtpPayloadType >= 64 && rtpPayloadType <= 95)
            {
                fRTCPInstanceForMultiplexedRTCPPackets
                ->injectReport(bPacket->data() - 12, bPacket->dataSize() + 12, fromAddress);
            }
            break;
        }
        unsigned cc = (rtpHdr >> 24) & 0xF;
        if (bPacket->dataSize() < cc) {
            break;
        }
        ADVANCE(cc * 4);
        if (rtpHdr & 0x10000000) {
            if (bPacket->dataSize() < 4) {
                break;
            }
            unsigned extHdr = ntohl(*(u_int32_t *) (bPacket->data()));
            ADVANCE(4);
            unsigned remExtSize = 4 * (extHdr & 0xFFFF);
            if (bPacket->dataSize() < remExtSize) {
                break;
            }
            ADVANCE(remExtSize);
        }
        if (rtpHdr & 0x20000000) {
            if (bPacket->dataSize() == 0) {
                break;
            }
            unsigned numPaddingBytes =
                (unsigned) (bPacket->data())[bPacket->dataSize() - 1];
            if (bPacket->dataSize() < numPaddingBytes) {
                break;
            }
            bPacket->removePadding(numPaddingBytes);
        }
        if (rtpSSRC != fLastReceivedSSRC) {
            fLastReceivedSSRC = rtpSSRC;
            fReorderingBuffer->resetHaveSeenFirstPacket();
        }
        unsigned short rtpSeqNo = (unsigned short) (rtpHdr & 0xFFFF);
        Boolean usableInJitterCalculation =
            packetIsUsableInJitterCalculation((bPacket->data()),
                                              bPacket->dataSize());
        struct timeval presentationTime;
        Boolean hasBeenSyncedUsingRTCP;
        receptionStatsDB()
        .noteIncomingPacket(rtpSSRC, rtpSeqNo, rtpTimestamp,
                            timestampFrequency(),
                            usableInJitterCalculation, presentationTime,
                            hasBeenSyncedUsingRTCP, bPacket->dataSize());
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        bPacket->assignMiscParams(rtpSeqNo, rtpTimestamp, presentationTime,
                                  hasBeenSyncedUsingRTCP, rtpMarkerBit,
                                  timeNow);
        if (!fReorderingBuffer->storePacket(bPacket)) {
            break;
        }
        readSuccess = True;
    } while (0);
    if (!readSuccess) {
        fReorderingBuffer->freePacket(bPacket);
    }
    doGetNextFrame1();
} // MultiFramedRTPSource::networkReadHandler1

#define MAX_PACKET_SIZE 20000
BufferedPacket::BufferedPacket()
    : fPacketSize(MAX_PACKET_SIZE),
    fBuf(new unsigned char[MAX_PACKET_SIZE]),
    fNextPacket(NULL)
{}

BufferedPacket::~BufferedPacket()
{
    delete fNextPacket;
    delete[] fBuf;
}

void BufferedPacket::reset()
{
    fHead          = fTail = 0;
    fUseCount      = 0;
    fIsFirstPacket = False;
}

unsigned BufferedPacket
::nextEnclosedFrameSize(unsigned char *&, unsigned dataSize)
{
    return dataSize;
}

void BufferedPacket
::getNextEnclosedFrameParameters(unsigned char *& framePtr, unsigned dataSize,
                                 unsigned& frameSize,
                                 unsigned& frameDurationInMicroseconds)
{
    frameSize = nextEnclosedFrameSize(framePtr, dataSize);
    frameDurationInMicroseconds = 0;
}

Boolean BufferedPacket::fillInData(RTPInterface& rtpInterface, struct sockaddr_in& fromAddress,
                                   Boolean& packetReadWasIncomplete)
{
    if (!packetReadWasIncomplete) {
        reset();
    }
    unsigned const maxBytesToRead = bytesAvailable();
    if (maxBytesToRead == 0) {
        return False;
    }
    unsigned numBytesRead;
    int tcpSocketNum;
    unsigned char tcpStreamChannelId;
    if (!rtpInterface.handleRead(&fBuf[fTail], maxBytesToRead,
                                 numBytesRead, fromAddress,
                                 tcpSocketNum, tcpStreamChannelId,
                                 packetReadWasIncomplete))
    {
        return False;
    }
    fTail += numBytesRead;
    return True;
}

void BufferedPacket
::assignMiscParams(unsigned short rtpSeqNo, unsigned rtpTimestamp,
                   struct timeval presentationTime,
                   Boolean hasBeenSyncedUsingRTCP, Boolean rtpMarkerBit,
                   struct timeval timeReceived)
{
    fRTPSeqNo               = rtpSeqNo;
    fRTPTimestamp           = rtpTimestamp;
    fPresentationTime       = presentationTime;
    fHasBeenSyncedUsingRTCP = hasBeenSyncedUsingRTCP;
    fRTPMarkerBit           = rtpMarkerBit;
    fTimeReceived           = timeReceived;
}

void BufferedPacket::skip(unsigned numBytes)
{
    fHead += numBytes;
    if (fHead > fTail) {
        fHead = fTail;
    }
}

void BufferedPacket::removePadding(unsigned numBytes)
{
    if (numBytes > fTail - fHead) {
        numBytes = fTail - fHead;
    }
    fTail -= numBytes;
}

void BufferedPacket::appendData(unsigned char* newData, unsigned numBytes)
{
    if (numBytes > fPacketSize - fTail) {
        numBytes = fPacketSize - fTail;
    }
    memmove(&fBuf[fTail], newData, numBytes);
    fTail += numBytes;
}

void BufferedPacket::use(unsigned char* to, unsigned toSize,
                         unsigned& bytesUsed, unsigned& bytesTruncated,
                         unsigned short& rtpSeqNo, unsigned& rtpTimestamp,
                         struct timeval& presentationTime,
                         Boolean& hasBeenSyncedUsingRTCP,
                         Boolean& rtpMarkerBit)
{
    unsigned char* origFramePtr = &fBuf[fHead];
    unsigned char* newFramePtr = origFramePtr;
    unsigned frameSize, frameDurationInMicroseconds;

    getNextEnclosedFrameParameters(newFramePtr, fTail - fHead,
                                   frameSize, frameDurationInMicroseconds);
    if (frameSize > toSize) {
        bytesTruncated += frameSize - toSize;
        bytesUsed       = toSize;
    } else {
        bytesTruncated = 0;
        bytesUsed      = frameSize;
    }
    memmove(to, newFramePtr, bytesUsed);
    fHead += (newFramePtr - origFramePtr) + frameSize;
    ++fUseCount;
    rtpSeqNo                   = fRTPSeqNo;
    rtpTimestamp               = fRTPTimestamp;
    presentationTime           = fPresentationTime;
    hasBeenSyncedUsingRTCP     = fHasBeenSyncedUsingRTCP;
    rtpMarkerBit               = fRTPMarkerBit;
    fPresentationTime.tv_usec += frameDurationInMicroseconds;
    if (fPresentationTime.tv_usec >= 1000000) {
        fPresentationTime.tv_sec += fPresentationTime.tv_usec / 1000000;
        fPresentationTime.tv_usec = fPresentationTime.tv_usec % 1000000;
    }
}

BufferedPacketFactory::BufferedPacketFactory()
{}

BufferedPacketFactory::~BufferedPacketFactory()
{}

BufferedPacket * BufferedPacketFactory
::createNewPacket(MultiFramedRTPSource *)
{
    return new BufferedPacket;
}

ReorderingPacketBuffer
::ReorderingPacketBuffer(BufferedPacketFactory* packetFactory)
    : fThresholdTime(100000),
    fHaveSeenFirstPacket(False), fHeadPacket(NULL), fTailPacket(NULL), fSavedPacket(NULL), fSavedPacketFree(True)
{
    fPacketFactory = (packetFactory == NULL) ?
                     (new BufferedPacketFactory) :
                     packetFactory;
}

ReorderingPacketBuffer::~ReorderingPacketBuffer()
{
    reset();
    delete fPacketFactory;
}

void ReorderingPacketBuffer::reset()
{
    if (fSavedPacketFree) {
        delete fSavedPacket;
    }
    delete fHeadPacket;
    resetHaveSeenFirstPacket();
    fHeadPacket = fTailPacket = fSavedPacket = NULL;
}

BufferedPacket * ReorderingPacketBuffer::getFreePacket(MultiFramedRTPSource* ourSource)
{
    if (fSavedPacket == NULL) {
        fSavedPacket     = fPacketFactory->createNewPacket(ourSource);
        fSavedPacketFree = True;
    }
    if (fSavedPacketFree == True) {
        fSavedPacketFree = False;
        return fSavedPacket;
    } else {
        return fPacketFactory->createNewPacket(ourSource);
    }
}

Boolean ReorderingPacketBuffer::storePacket(BufferedPacket* bPacket)
{
    unsigned short rtpSeqNo = bPacket->rtpSeqNo();

    if (!fHaveSeenFirstPacket) {
        fNextExpectedSeqNo       = rtpSeqNo;
        bPacket->isFirstPacket() = True;
        fHaveSeenFirstPacket     = True;
    }
    if (seqNumLT(rtpSeqNo, fNextExpectedSeqNo)) {
        return False;
    }
    if (fTailPacket == NULL) {
        bPacket->nextPacket() = NULL;
        fHeadPacket = fTailPacket = bPacket;
        return True;
    }
    if (seqNumLT(fTailPacket->rtpSeqNo(), rtpSeqNo)) {
        bPacket->nextPacket()     = NULL;
        fTailPacket->nextPacket() = bPacket;
        fTailPacket = bPacket;
        return True;
    }
    if (rtpSeqNo == fTailPacket->rtpSeqNo()) {
        return False;
    }
    BufferedPacket* beforePtr = NULL;
    BufferedPacket* afterPtr  = fHeadPacket;
    while (afterPtr != NULL) {
        if (seqNumLT(rtpSeqNo, afterPtr->rtpSeqNo())) {
            break;
        }
        if (rtpSeqNo == afterPtr->rtpSeqNo()) {
            return False;
        }
        beforePtr = afterPtr;
        afterPtr  = afterPtr->nextPacket();
    }
    bPacket->nextPacket() = afterPtr;
    if (beforePtr == NULL) {
        fHeadPacket = bPacket;
    } else {
        beforePtr->nextPacket() = bPacket;
    }
    return True;
} // ReorderingPacketBuffer::storePacket

void ReorderingPacketBuffer::releaseUsedPacket(BufferedPacket* packet)
{
    ++fNextExpectedSeqNo;
    fHeadPacket = fHeadPacket->nextPacket();
    if (!fHeadPacket) {
        fTailPacket = NULL;
    }
    packet->nextPacket() = NULL;
    freePacket(packet);
}

BufferedPacket * ReorderingPacketBuffer
::getNextCompletedPacket(Boolean& packetLossPreceded)
{
    if (fHeadPacket == NULL) {
        return NULL;
    }
    if (fHeadPacket->rtpSeqNo() == fNextExpectedSeqNo) {
        packetLossPreceded = fHeadPacket->isFirstPacket();
        return fHeadPacket;
    }
    Boolean timeThresholdHasBeenExceeded;
    if (fThresholdTime == 0) {
        timeThresholdHasBeenExceeded = True;
    } else {
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        unsigned uSecondsSinceReceived =
            (timeNow.tv_sec - fHeadPacket->timeReceived().tv_sec) * 1000000
            + (timeNow.tv_usec - fHeadPacket->timeReceived().tv_usec);
        timeThresholdHasBeenExceeded = uSecondsSinceReceived > fThresholdTime;
    }
    if (timeThresholdHasBeenExceeded) {
        fNextExpectedSeqNo = fHeadPacket->rtpSeqNo();
        packetLossPreceded = True;
        return fHeadPacket;
    }
    return NULL;
}
