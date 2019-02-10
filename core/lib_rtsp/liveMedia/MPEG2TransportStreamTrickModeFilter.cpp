#include "MPEG2TransportStreamTrickModeFilter.hh"
#include <ByteStreamFileSource.hh>
#define KEEP_ORIGINAL_FRAME_RATE False
MPEG2TransportStreamTrickModeFilter * MPEG2TransportStreamTrickModeFilter
::createNew(UsageEnvironment& env, FramedSource* inputSource,
            MPEG2TransportStreamIndexFile* indexFile, int scale)
{
    return new MPEG2TransportStreamTrickModeFilter(env, inputSource, indexFile, scale);
}

MPEG2TransportStreamTrickModeFilter
::MPEG2TransportStreamTrickModeFilter(UsageEnvironment& env, FramedSource* inputSource,
                                      MPEG2TransportStreamIndexFile* indexFile, int scale)
    : FramedFilter(env, inputSource),
    fHaveStarted(False), fIndexFile(indexFile), fScale(scale), fDirection(1),
    fState(SKIPPING_FRAME), fFrameCount(0),
    fNextIndexRecordNum(0), fNextTSPacketNum(0),
    fCurrentTSPacketNum((unsigned long) (-1)), fUseSavedFrameNextTime(False)
{
    if (fScale < 0) {
        fScale     = -fScale;
        fDirection = -1;
    }
}

MPEG2TransportStreamTrickModeFilter::~MPEG2TransportStreamTrickModeFilter()
{}

Boolean MPEG2TransportStreamTrickModeFilter::seekTo(unsigned long tsPacketNumber,
                                                    unsigned long indexRecordNumber)
{
    seekToTransportPacket(tsPacketNumber);
    fNextIndexRecordNum = indexRecordNumber;
    return True;
}

#define isIFrameStart(type)    ((type) == 0x81 || (type) == 0x85 || (type) == 0x8B)
#define isNonIFrameStart(type) ((type) == 0x83 || (type) == 0x88 || (type) == 0x8E)
void MPEG2TransportStreamTrickModeFilter::doGetNextFrame()
{
    if (fMaxSize < TRANSPORT_PACKET_SIZE) {
        fFrameSize = 0;
        afterGetting(this);
        return;
    }
    while (1) {
        u_int8_t recordType;
        float recordPCR;
        Boolean endOfIndexFile = False;
        if (!fIndexFile->readIndexRecordValues(fNextIndexRecordNum,
                                               fDesiredTSPacketNum, fDesiredDataOffset,
                                               fDesiredDataSize, recordPCR,
                                               recordType))
        {
            if (fState != DELIVERING_SAVED_FRAME) {
                onSourceClosure1();
                return;
            }
            endOfIndexFile = True;
        } else if (!fHaveStarted) {
            fFirstPCR    = recordPCR;
            fHaveStarted = True;
        }
        fNextIndexRecordNum +=
            (fState == DELIVERING_SAVED_FRAME) ? 1 : fDirection;
        switch (fState) {
            case SKIPPING_FRAME:
            case SAVING_AND_DELIVERING_FRAME: {
                if (isIFrameStart(recordType)) {
                    fSavedFrameIndexRecordStart = fNextIndexRecordNum - fDirection;
                    fUseSavedFrameNextTime      = True;
                    if ((fFrameCount++) % fScale == 0 && fUseSavedFrameNextTime) {
                        fFrameCount = 1;
                        if (fDirection > 0) {
                            fState = SAVING_AND_DELIVERING_FRAME;
                            fDesiredDataPCR = recordPCR;
                            attemptDeliveryToClient();
                            return;
                        } else {
                            fState = DELIVERING_SAVED_FRAME;
                            fSavedSequentialIndexRecordNum = fNextIndexRecordNum;
                            fDesiredDataPCR     = recordPCR;
                            fNextIndexRecordNum = fSavedFrameIndexRecordStart;
                        }
                    } else {
                        fState = SKIPPING_FRAME;
                    }
                } else if (isNonIFrameStart(recordType)) {
                    if ((fFrameCount++) % fScale == 0 && fUseSavedFrameNextTime) {
                        fFrameCount = 1;
                        fState      = DELIVERING_SAVED_FRAME;
                        fSavedSequentialIndexRecordNum = fNextIndexRecordNum;
                        fDesiredDataPCR     = recordPCR;
                        fNextIndexRecordNum = fSavedFrameIndexRecordStart;
                    } else {
                        fState = SKIPPING_FRAME;
                    }
                } else {
                    if (fState == SAVING_AND_DELIVERING_FRAME) {
                        fDesiredDataPCR = recordPCR;
                        attemptDeliveryToClient();
                        return;
                    }
                }
                break;
            }
            case DELIVERING_SAVED_FRAME: {
                if (endOfIndexFile ||
                    (isIFrameStart(recordType) &&
                     fNextIndexRecordNum - 1 != fSavedFrameIndexRecordStart) ||
                    isNonIFrameStart(recordType))
                {
                    fNextIndexRecordNum    = fSavedSequentialIndexRecordNum;
                    fUseSavedFrameNextTime = KEEP_ORIGINAL_FRAME_RATE;
                    fState = SKIPPING_FRAME;
                } else {
                    attemptDeliveryToClient();
                    return;
                }
                break;
            }
        }
    }
} // MPEG2TransportStreamTrickModeFilter::doGetNextFrame

void MPEG2TransportStreamTrickModeFilter::doStopGettingFrames()
{
    FramedFilter::doStopGettingFrames();
    fIndexFile->stopReading();
}

void MPEG2TransportStreamTrickModeFilter::attemptDeliveryToClient()
{
    if (fCurrentTSPacketNum == fDesiredTSPacketNum) {
        memmove(fTo, &fInputBuffer[fDesiredDataOffset], fDesiredDataSize);
        fFrameSize = fDesiredDataSize;
        float deliveryPCR = fDirection * (fDesiredDataPCR - fFirstPCR) / fScale;
        if (deliveryPCR < 0.0) {
            deliveryPCR = 0.0;
        }
        fPresentationTime.tv_sec  = (unsigned long) deliveryPCR;
        fPresentationTime.tv_usec =
            (unsigned long) ((deliveryPCR - fPresentationTime.tv_sec) * 1000000.0f);
        afterGetting(this);
    } else {
        readTransportPacket(fDesiredTSPacketNum);
    }
}

void MPEG2TransportStreamTrickModeFilter::seekToTransportPacket(unsigned long tsPacketNum)
{
    if (tsPacketNum == fNextTSPacketNum) {
        return;
    }
    ByteStreamFileSource* tsFile = (ByteStreamFileSource *) fInputSource;
    u_int64_t tsPacketNum64      = (u_int64_t) tsPacketNum;
    tsFile->seekToByteAbsolute(tsPacketNum64 * TRANSPORT_PACKET_SIZE);
    fNextTSPacketNum = tsPacketNum;
}

void MPEG2TransportStreamTrickModeFilter::readTransportPacket(unsigned long tsPacketNum)
{
    seekToTransportPacket(tsPacketNum);
    fInputSource->getNextFrame(fInputBuffer, TRANSPORT_PACKET_SIZE,
                               afterGettingFrame, this,
                               onSourceClosure, this);
}

void MPEG2TransportStreamTrickModeFilter
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned,
                    struct timeval presentationTime,
                    unsigned)
{
    MPEG2TransportStreamTrickModeFilter* filter = (MPEG2TransportStreamTrickModeFilter *) clientData;

    filter->afterGettingFrame1(frameSize);
}

void MPEG2TransportStreamTrickModeFilter::afterGettingFrame1(unsigned frameSize)
{
    if (frameSize != TRANSPORT_PACKET_SIZE) {
        onSourceClosure1();
        return;
    }
    fCurrentTSPacketNum = fNextTSPacketNum;
    ++fNextTSPacketNum;
    attemptDeliveryToClient();
}

void MPEG2TransportStreamTrickModeFilter::onSourceClosure(void* clientData)
{
    MPEG2TransportStreamTrickModeFilter* filter = (MPEG2TransportStreamTrickModeFilter *) clientData;

    filter->onSourceClosure1();
}

void MPEG2TransportStreamTrickModeFilter::onSourceClosure1()
{
    fIndexFile->stopReading();
    handleClosure();
}
