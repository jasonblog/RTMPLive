#include "MPEG2TransportStreamFramer.hh"
#include <GroupsockHelper.hh>
#define TRANSPORT_PACKET_SIZE 188
#if !defined(NEW_DURATION_WEIGHT)
# define NEW_DURATION_WEIGHT  0.5
#endif
#if !defined(TIME_ADJUSTMENT_FACTOR)
# define TIME_ADJUSTMENT_FACTOR 0.8
#endif
#if !defined(MAX_PLAYOUT_BUFFER_DURATION)
# define MAX_PLAYOUT_BUFFER_DURATION 0.1
#endif
#if !defined(PCR_PERIOD_VARIATION_RATIO)
# define PCR_PERIOD_VARIATION_RATIO 0.5
#endif
class PIDStatus
{
public:
    PIDStatus(double _firstClock, double _firstRealTime)
        : firstClock(_firstClock), lastClock(_firstClock),
        firstRealTime(_firstRealTime), lastRealTime(_firstRealTime),
        lastPacketNum(0)
    {}

    double firstClock, lastClock, firstRealTime, lastRealTime;
    u_int64_t lastPacketNum;
};
MPEG2TransportStreamFramer * MPEG2TransportStreamFramer
::createNew(UsageEnvironment& env, FramedSource* inputSource)
{
    return new MPEG2TransportStreamFramer(env, inputSource);
}

MPEG2TransportStreamFramer
::MPEG2TransportStreamFramer(UsageEnvironment& env, FramedSource* inputSource)
    : FramedFilter(env, inputSource),
    fTSPacketCount(0), fTSPacketDurationEstimate(0.0), fTSPCRCount(0),
    fLimitNumTSPacketsToStream(False), fNumTSPacketsToStream(0),
    fLimitTSPacketsToStreamByPCR(False), fPCRLimit(0.0)
{
    fPIDStatusTable = HashTable::create(ONE_WORD_HASH_KEYS);
}

MPEG2TransportStreamFramer::~MPEG2TransportStreamFramer()
{
    clearPIDStatusTable();
    delete fPIDStatusTable;
}

void MPEG2TransportStreamFramer::clearPIDStatusTable()
{
    PIDStatus* pidStatus;

    while ((pidStatus = (PIDStatus *) fPIDStatusTable->RemoveNext()) != NULL) {
        delete pidStatus;
    }
}

void MPEG2TransportStreamFramer::setNumTSPacketsToStream(unsigned long numTSRecordsToStream)
{
    fNumTSPacketsToStream      = numTSRecordsToStream;
    fLimitNumTSPacketsToStream = numTSRecordsToStream > 0;
}

void MPEG2TransportStreamFramer::setPCRLimit(float pcrLimit)
{
    fPCRLimit = pcrLimit;
    fLimitTSPacketsToStreamByPCR = pcrLimit != 0.0;
}

void MPEG2TransportStreamFramer::doGetNextFrame()
{
    if (fLimitNumTSPacketsToStream) {
        if (fNumTSPacketsToStream == 0) {
            handleClosure();
            return;
        }
        if (fNumTSPacketsToStream * TRANSPORT_PACKET_SIZE < fMaxSize) {
            fMaxSize = fNumTSPacketsToStream * TRANSPORT_PACKET_SIZE;
        }
    }
    fFrameSize = 0;
    fInputSource->getNextFrame(fTo, fMaxSize,
                               afterGettingFrame, this,
                               FramedSource::handleClosure, this);
}

void MPEG2TransportStreamFramer::doStopGettingFrames()
{
    FramedFilter::doStopGettingFrames();
    fTSPacketCount = 0;
    fTSPCRCount    = 0;
    clearPIDStatusTable();
}

void MPEG2TransportStreamFramer
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned,
                    struct timeval presentationTime,
                    unsigned)
{
    MPEG2TransportStreamFramer* framer = (MPEG2TransportStreamFramer *) clientData;

    framer->afterGettingFrame1(frameSize, presentationTime);
}

#define TRANSPORT_SYNC_BYTE 0x47
void MPEG2TransportStreamFramer::afterGettingFrame1(unsigned       frameSize,
                                                    struct timeval presentationTime)
{
    fFrameSize += frameSize;
    unsigned const numTSPackets = fFrameSize / TRANSPORT_PACKET_SIZE;
    fNumTSPacketsToStream -= numTSPackets;
    fFrameSize = numTSPackets * TRANSPORT_PACKET_SIZE;
    if (fFrameSize == 0) {
        handleClosure();
        return;
    }
    unsigned syncBytePosition;
    for (syncBytePosition = 0; syncBytePosition < fFrameSize; ++syncBytePosition) {
        if (fTo[syncBytePosition] == TRANSPORT_SYNC_BYTE) {
            break;
        }
    }
    if (syncBytePosition == fFrameSize) {
        envir() << "No Transport Stream sync byte in data.";
        handleClosure();
        return;
    } else if (syncBytePosition > 0) {
        memmove(fTo, &fTo[syncBytePosition], fFrameSize - syncBytePosition);
        fFrameSize -= syncBytePosition;
        fInputSource->getNextFrame(&fTo[fFrameSize], syncBytePosition,
                                   afterGettingFrame, this,
                                   FramedSource::handleClosure, this);
        return;
    }
    fPresentationTime = presentationTime;
    struct timeval tvNow;
    gettimeofday(&tvNow, NULL);
    double timeNow = tvNow.tv_sec + tvNow.tv_usec / 1000000.0;
    for (unsigned i = 0; i < numTSPackets; ++i) {
        if (!updateTSPacketDurationEstimate(&fTo[i * TRANSPORT_PACKET_SIZE], timeNow)) {
            handleClosure();
            return;
        }
    }
    fDurationInMicroseconds =
        numTSPackets * (unsigned) (fTSPacketDurationEstimate * 1000000);
    afterGetting(this);
} // MPEG2TransportStreamFramer::afterGettingFrame1

Boolean MPEG2TransportStreamFramer::updateTSPacketDurationEstimate(unsigned char* pkt, double timeNow)
{
    if (pkt[0] != TRANSPORT_SYNC_BYTE) {
        envir() << "Missing sync byte!\n";
        return True;
    }
    ++fTSPacketCount;
    u_int8_t const adaptation_field_control = (pkt[3] & 0x30) >> 4;
    if (adaptation_field_control != 2 && adaptation_field_control != 3) {
        return True;
    }
    u_int8_t const adaptation_field_length = pkt[4];
    if (adaptation_field_length == 0) {
        return True;
    }
    u_int8_t const discontinuity_indicator = pkt[5] & 0x80;
    u_int8_t const pcrFlag = pkt[5] & 0x10;
    if (pcrFlag == 0) {
        return True;
    }
    ++fTSPCRCount;
    u_int32_t pcrBaseHigh = (pkt[6] << 24) | (pkt[7] << 16) | (pkt[8] << 8) | pkt[9];
    double clock = pcrBaseHigh / 45000.0;
    if ((pkt[10] & 0x80) != 0) {
        clock += 1 / 90000.0;
    }
    unsigned short pcrExt = ((pkt[10] & 0x01) << 8) | pkt[11];
    clock += pcrExt / 27000000.0;
    if (fLimitTSPacketsToStreamByPCR) {
        if (clock > fPCRLimit) {
            return False;
        }
    }
    unsigned pid         = ((pkt[1] & 0x1F) << 8) | pkt[2];
    PIDStatus* pidStatus = (PIDStatus *) (fPIDStatusTable->Lookup((char *) pid));
    if (pidStatus == NULL) {
        pidStatus = new PIDStatus(clock, timeNow);
        fPIDStatusTable->Add((char *) pid, pidStatus);
        #ifdef DEBUG_PCR
        fprintf(stderr, "PID 0x%x, FIRST PCR 0x%08x+%d:%03x == %f @ %f, pkt #%lu\n", pid, pcrBaseHigh, pkt[10] >> 7,
                pcrExt, clock, timeNow, fTSPacketCount);
        #endif
    } else {
        int64_t packetsSinceLast = (int64_t) (fTSPacketCount - pidStatus->lastPacketNum);
        double durationPerPacket = (clock - pidStatus->lastClock) / packetsSinceLast;
        double meanPCRPeriod     = 0.0;
        if (fTSPCRCount > 0) {
            double tsPacketCount = (double) (int64_t) fTSPacketCount;
            double tsPCRCount    = (double) (int64_t) fTSPCRCount;
            meanPCRPeriod = tsPacketCount / tsPCRCount;
            if (packetsSinceLast < meanPCRPeriod * PCR_PERIOD_VARIATION_RATIO) {
                return True;
            }
        }
        if (fTSPacketDurationEstimate == 0.0) {
            fTSPacketDurationEstimate = durationPerPacket;
        } else if (discontinuity_indicator == 0 && durationPerPacket >= 0.0) {
            fTSPacketDurationEstimate =
                durationPerPacket * NEW_DURATION_WEIGHT
                + fTSPacketDurationEstimate * (1 - NEW_DURATION_WEIGHT);
            double transmitDuration = timeNow - pidStatus->firstRealTime;
            double playoutDuration  = clock - pidStatus->firstClock;
            if (transmitDuration > playoutDuration) {
                fTSPacketDurationEstimate *= TIME_ADJUSTMENT_FACTOR;
            } else if (transmitDuration + MAX_PLAYOUT_BUFFER_DURATION < playoutDuration) {
                fTSPacketDurationEstimate /= TIME_ADJUSTMENT_FACTOR;
            }
        } else {
            pidStatus->firstClock    = clock;
            pidStatus->firstRealTime = timeNow;
        }
        #ifdef DEBUG_PCR
        fprintf(stderr,
                "PID 0x%x, PCR 0x%08x+%d:%03x == %f @ %f (diffs %f @ %f), pkt #%lu, discon %d => this duration %f, new estimate %f, mean PCR period=%f\n", pid, pcrBaseHigh,
                pkt[10] >> 7, pcrExt, clock, timeNow, clock - pidStatus->firstClock, timeNow - pidStatus->firstRealTime, fTSPacketCount, discontinuity_indicator != 0, durationPerPacket, fTSPacketDurationEstimate,
                meanPCRPeriod);
        #endif
    }
    pidStatus->lastClock     = clock;
    pidStatus->lastRealTime  = timeNow;
    pidStatus->lastPacketNum = fTSPacketCount;
    return True;
} // MPEG2TransportStreamFramer::updateTSPacketDurationEstimate
