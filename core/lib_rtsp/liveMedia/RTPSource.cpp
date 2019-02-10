#include "RTPSource.hh"
#include "GroupsockHelper.hh"
Boolean RTPSource::lookupByName(UsageEnvironment& env,
                                char const*     sourceName,
                                RTPSource *     & resultSource)
{
    resultSource = NULL;
    MediaSource* source;
    if (!MediaSource::lookupByName(env, sourceName, source)) {
        return False;
    }
    if (!source->isRTPSource()) {
        env.setResultMsg(sourceName, " is not a RTP source");
        return False;
    }
    resultSource = (RTPSource *) source;
    return True;
}

Boolean RTPSource::hasBeenSynchronizedUsingRTCP()
{
    return fCurPacketHasBeenSynchronizedUsingRTCP;
}

Boolean RTPSource::isRTPSource() const
{
    return True;
}

RTPSource::RTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                     unsigned char rtpPayloadFormat,
                     u_int32_t rtpTimestampFrequency)
    : FramedSource(env),
    fRTPInterface(this, RTPgs),
    fCurPacketHasBeenSynchronizedUsingRTCP(False), fLastReceivedSSRC(0),
    fRTCPInstanceForMultiplexedRTCPPackets(NULL),
    fRTPPayloadFormat(rtpPayloadFormat), fTimestampFrequency(rtpTimestampFrequency),
    fSSRC(our_random32()), fEnableRTCPReports(True)
{
    fReceptionStatsDB = new RTPReceptionStatsDB();
}

RTPSource::~RTPSource()
{
    delete fReceptionStatsDB;
}

void RTPSource::getAttributes() const
{
    envir().setResultMsg("");
}

RTPReceptionStatsDB::RTPReceptionStatsDB()
    : fTable(HashTable::create(ONE_WORD_HASH_KEYS)), fTotNumPacketsReceived(0)
{
    reset();
}

void RTPReceptionStatsDB::reset()
{
    fNumActiveSourcesSinceLastReset = 0;
    Iterator iter(*this);
    RTPReceptionStats* stats;
    while ((stats = iter.next()) != NULL) {
        stats->reset();
    }
}

RTPReceptionStatsDB::~RTPReceptionStatsDB()
{
    RTPReceptionStats* stats;

    while ((stats = (RTPReceptionStats *) fTable->RemoveNext()) != NULL) {
        delete stats;
    }
    delete fTable;
}

void RTPReceptionStatsDB
::noteIncomingPacket(u_int32_t SSRC, u_int16_t seqNum,
                     u_int32_t rtpTimestamp, unsigned timestampFrequency,
                     Boolean useForJitterCalculation,
                     struct timeval& resultPresentationTime,
                     Boolean& resultHasBeenSyncedUsingRTCP,
                     unsigned packetSize)
{
    ++fTotNumPacketsReceived;
    RTPReceptionStats* stats = lookup(SSRC);
    if (stats == NULL) {
        stats = new RTPReceptionStats(SSRC, seqNum);
        if (stats == NULL) {
            return;
        }
        add(SSRC, stats);
    }
    if (stats->numPacketsReceivedSinceLastReset() == 0) {
        ++fNumActiveSourcesSinceLastReset;
    }
    stats->noteIncomingPacket(seqNum, rtpTimestamp, timestampFrequency,
                              useForJitterCalculation,
                              resultPresentationTime,
                              resultHasBeenSyncedUsingRTCP, packetSize);
}

void RTPReceptionStatsDB
::noteIncomingSR(u_int32_t SSRC,
                 u_int32_t ntpTimestampMSW, u_int32_t ntpTimestampLSW,
                 u_int32_t rtpTimestamp)
{
    RTPReceptionStats* stats = lookup(SSRC);

    if (stats == NULL) {
        stats = new RTPReceptionStats(SSRC);
        if (stats == NULL) {
            return;
        }
        add(SSRC, stats);
    }
    stats->noteIncomingSR(ntpTimestampMSW, ntpTimestampLSW, rtpTimestamp);
}

void RTPReceptionStatsDB::removeRecord(u_int32_t SSRC)
{
    RTPReceptionStats* stats = lookup(SSRC);

    if (stats != NULL) {
        long SSRC_long = (long) SSRC;
        fTable->Remove((char const *) SSRC_long);
        delete stats;
    }
}

RTPReceptionStatsDB::Iterator
::Iterator(RTPReceptionStatsDB& receptionStatsDB)
    : fIter(HashTable::Iterator::create(*(receptionStatsDB.fTable)))
{}

RTPReceptionStatsDB::Iterator::~Iterator()
{
    delete fIter;
}

RTPReceptionStats * RTPReceptionStatsDB::Iterator::next(Boolean includeInactiveSources)
{
    char const* key;
    RTPReceptionStats* stats;

    do {
        stats = (RTPReceptionStats *) (fIter->next(key));
    } while (stats != NULL && !includeInactiveSources &&
             stats->numPacketsReceivedSinceLastReset() == 0);
    return stats;
}

RTPReceptionStats * RTPReceptionStatsDB::lookup(u_int32_t SSRC) const
{
    long SSRC_long = (long) SSRC;

    return (RTPReceptionStats *) (fTable->Lookup((char const *) SSRC_long));
}

void RTPReceptionStatsDB::add(u_int32_t SSRC, RTPReceptionStats* stats)
{
    long SSRC_long = (long) SSRC;

    fTable->Add((char const *) SSRC_long, stats);
}

RTPReceptionStats::RTPReceptionStats(u_int32_t SSRC, u_int16_t initialSeqNum)
{
    initSeqNum(initialSeqNum);
    init(SSRC);
}

RTPReceptionStats::RTPReceptionStats(u_int32_t SSRC)
{
    init(SSRC);
}

RTPReceptionStats::~RTPReceptionStats()
{}

void RTPReceptionStats::init(u_int32_t SSRC)
{
    fSSRC = SSRC;
    fTotNumPacketsReceived         = 0;
    fTotBytesReceived_hi           = fTotBytesReceived_lo = 0;
    fBaseExtSeqNumReceived         = 0;
    fHighestExtSeqNumReceived      = 0;
    fHaveSeenInitialSequenceNumber = False;
    fLastTransit = ~0;
    fPreviousPacketRTPTimestamp = 0;
    fJitter = 0.0;
    fLastReceivedSR_NTPmsw          = fLastReceivedSR_NTPlsw = 0;
    fLastReceivedSR_time.tv_sec     = fLastReceivedSR_time.tv_usec = 0;
    fLastPacketReceptionTime.tv_sec = fLastPacketReceptionTime.tv_usec = 0;
    fMinInterPacketGapUS         = 0x7FFFFFFF;
    fMaxInterPacketGapUS         = 0;
    fTotalInterPacketGaps.tv_sec = fTotalInterPacketGaps.tv_usec = 0;
    fHasBeenSynchronized         = False;
    fSyncTime.tv_sec = fSyncTime.tv_usec = 0;
    reset();
}

void RTPReceptionStats::initSeqNum(u_int16_t initialSeqNum)
{
    fBaseExtSeqNumReceived         = 0x10000 | initialSeqNum;
    fHighestExtSeqNumReceived      = 0x10000 | initialSeqNum;
    fHaveSeenInitialSequenceNumber = True;
}

#ifndef MILLION
# define MILLION 1000000
#endif
void RTPReceptionStats
::noteIncomingPacket(u_int16_t seqNum, u_int32_t rtpTimestamp,
                     unsigned timestampFrequency,
                     Boolean useForJitterCalculation,
                     struct timeval& resultPresentationTime,
                     Boolean& resultHasBeenSyncedUsingRTCP,
                     unsigned packetSize)
{
    if (!fHaveSeenInitialSequenceNumber) {
        initSeqNum(seqNum);
    }
    ++fNumPacketsReceivedSinceLastReset;
    ++fTotNumPacketsReceived;
    u_int32_t prevTotBytesReceived_lo = fTotBytesReceived_lo;
    fTotBytesReceived_lo += packetSize;
    if (fTotBytesReceived_lo < prevTotBytesReceived_lo) {
        ++fTotBytesReceived_hi;
    }
    unsigned oldSeqNum        = (fHighestExtSeqNumReceived & 0xFFFF);
    unsigned seqNumCycle      = (fHighestExtSeqNumReceived & 0xFFFF0000);
    unsigned seqNumDifference = (unsigned) ((int) seqNum - (int) oldSeqNum);
    unsigned newSeqNum        = 0;
    if (seqNumLT((u_int16_t) oldSeqNum, seqNum)) {
        if (seqNumDifference >= 0x8000) {
            seqNumCycle += 0x10000;
        }
        newSeqNum = seqNumCycle | seqNum;
        if (newSeqNum > fHighestExtSeqNumReceived) {
            fHighestExtSeqNumReceived = newSeqNum;
        }
    } else if (fTotNumPacketsReceived > 1) {
        if ((int) seqNumDifference >= 0x8000) {
            seqNumCycle -= 0x10000;
        }
        newSeqNum = seqNumCycle | seqNum;
        if (newSeqNum < fBaseExtSeqNumReceived) {
            fBaseExtSeqNumReceived = newSeqNum;
        }
    }
    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);
    if (fLastPacketReceptionTime.tv_sec != 0 ||
        fLastPacketReceptionTime.tv_usec != 0)
    {
        unsigned gap =
            (timeNow.tv_sec - fLastPacketReceptionTime.tv_sec) * MILLION
            + timeNow.tv_usec - fLastPacketReceptionTime.tv_usec;
        if (gap > fMaxInterPacketGapUS) {
            fMaxInterPacketGapUS = gap;
        }
        if (gap < fMinInterPacketGapUS) {
            fMinInterPacketGapUS = gap;
        }
        fTotalInterPacketGaps.tv_usec += gap;
        if (fTotalInterPacketGaps.tv_usec >= MILLION) {
            ++fTotalInterPacketGaps.tv_sec;
            fTotalInterPacketGaps.tv_usec -= MILLION;
        }
    }
    fLastPacketReceptionTime = timeNow;
    if (useForJitterCalculation &&
        rtpTimestamp != fPreviousPacketRTPTimestamp)
    {
        unsigned arrival = (timestampFrequency * timeNow.tv_sec);
        arrival += (unsigned)
                   ((2.0 * timestampFrequency * timeNow.tv_usec + 1000000.0) / 2000000);
        int transit = arrival - rtpTimestamp;
        if (fLastTransit == (~0)) {
            fLastTransit = transit;
        }
        int d = transit - fLastTransit;
        fLastTransit = transit;
        if (d < 0) {
            d = -d;
        }
        fJitter += (1.0 / 16.0) * ((double) d - fJitter);
    }
    if (fSyncTime.tv_sec == 0 && fSyncTime.tv_usec == 0) {
        fSyncTimestamp = rtpTimestamp;
        fSyncTime      = timeNow;
    }
    int timestampDiff = rtpTimestamp - fSyncTimestamp;
    double timeDiff = timestampDiff / (double) timestampFrequency;
    unsigned const million = 1000000;
    unsigned seconds, uSeconds;
    if (timeDiff >= 0.0) {
        seconds  = fSyncTime.tv_sec + (unsigned) (timeDiff);
        uSeconds = fSyncTime.tv_usec
                   + (unsigned) ((timeDiff - (unsigned) timeDiff) * million);
        if (uSeconds >= million) {
            uSeconds -= million;
            ++seconds;
        }
    } else {
        timeDiff = -timeDiff;
        seconds  = fSyncTime.tv_sec - (unsigned) (timeDiff);
        uSeconds = fSyncTime.tv_usec
                   - (unsigned) ((timeDiff - (unsigned) timeDiff) * million);
        if ((int) uSeconds < 0) {
            uSeconds += million;
            --seconds;
        }
    }
    resultPresentationTime.tv_sec  = seconds;
    resultPresentationTime.tv_usec = uSeconds;
    resultHasBeenSyncedUsingRTCP   = fHasBeenSynchronized;
    fSyncTimestamp = rtpTimestamp;
    fSyncTime      = resultPresentationTime;
    fPreviousPacketRTPTimestamp = rtpTimestamp;
} // RTPReceptionStats::noteIncomingPacket

void RTPReceptionStats::noteIncomingSR(u_int32_t ntpTimestampMSW,
                                       u_int32_t ntpTimestampLSW,
                                       u_int32_t rtpTimestamp)
{
    fLastReceivedSR_NTPmsw = ntpTimestampMSW;
    fLastReceivedSR_NTPlsw = ntpTimestampLSW;
    gettimeofday(&fLastReceivedSR_time, NULL);
    fSyncTimestamp   = rtpTimestamp;
    fSyncTime.tv_sec = ntpTimestampMSW - 0x83AA7E80;
    double microseconds = (ntpTimestampLSW * 15625.0) / 0x04000000;
    fSyncTime.tv_usec    = (unsigned) (microseconds + 0.5);
    fHasBeenSynchronized = True;
}

double RTPReceptionStats::totNumKBytesReceived() const
{
    double const hiMultiplier = 0x20000000 / 125.0;

    return fTotBytesReceived_hi * hiMultiplier + fTotBytesReceived_lo / 1000.0;
}

unsigned RTPReceptionStats::jitter() const
{
    return (unsigned) fJitter;
}

void RTPReceptionStats::reset()
{
    fNumPacketsReceivedSinceLastReset = 0;
    fLastResetExtSeqNumReceived       = fHighestExtSeqNumReceived;
}

Boolean seqNumLT(u_int16_t s1, u_int16_t s2)
{
    int diff = s2 - s1;

    if (diff > 0) {
        return (diff < 0x8000);
    } else if (diff < 0) {
        return (diff < -0x8000);
    } else {
        return False;
    }
}
