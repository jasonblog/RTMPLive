#include "RTPSink.hh"
#include "GroupsockHelper.hh"
Boolean RTPSink::lookupByName(UsageEnvironment& env, char const* sinkName,
                              RTPSink *& resultSink)
{
    resultSink = NULL;
    MediaSink* sink;
    if (!MediaSink::lookupByName(env, sinkName, sink)) {
        return False;
    }
    if (!sink->isRTPSink()) {
        env.setResultMsg(sinkName, " is not a RTP sink");
        return False;
    }
    resultSink = (RTPSink *) sink;
    return True;
}

Boolean RTPSink::isRTPSink() const
{
    return True;
}

RTPSink::RTPSink(UsageEnvironment& env,
                 Groupsock* rtpGS, unsigned char rtpPayloadType,
                 unsigned rtpTimestampFrequency,
                 char const* rtpPayloadFormatName,
                 unsigned numChannels)
    : MediaSink(env), fRTPInterface(this, rtpGS),
    fRTPPayloadType(rtpPayloadType),
    fPacketCount(0), fOctetCount(0), fTotalOctetCount(0),
    fTimestampFrequency(rtpTimestampFrequency), fNextTimestampHasBeenPreset(False), fEnableRTCPReports(True),
    fNumChannels(numChannels), fEstimatedBitrate(0)
{
    fRTPPayloadFormatName =
        strDup(rtpPayloadFormatName == NULL ? "???" : rtpPayloadFormatName);
    gettimeofday(&fCreationTime, NULL);
    fTotalOctetCountStartTime = fCreationTime;
    resetPresentationTimes();
    fSeqNo               = (u_int16_t) our_random();
    fSSRC                = our_random32();
    fTimestampBase       = our_random32();
    fTransmissionStatsDB = new RTPTransmissionStatsDB(*this);
}

RTPSink::~RTPSink()
{
    delete fTransmissionStatsDB;
    delete[](char *) fRTPPayloadFormatName;
}

u_int32_t RTPSink::convertToRTPTimestamp(struct timeval tv)
{
    u_int32_t timestampIncrement = (fTimestampFrequency * tv.tv_sec);

    timestampIncrement += (u_int32_t) (fTimestampFrequency * (tv.tv_usec / 1000000.0) + 0.5);
    if (fNextTimestampHasBeenPreset) {
        fTimestampBase -= timestampIncrement;
        fNextTimestampHasBeenPreset = False;
    }
    u_int32_t const rtpTimestamp = fTimestampBase + timestampIncrement;
    #ifdef DEBUG_TIMESTAMPS
    fprintf(stderr, "fTimestampBase: 0x%08x, tv: %lu.%06ld\n\t=> RTP timestamp: 0x%08x\n",
            fTimestampBase, tv.tv_sec, tv.tv_usec, rtpTimestamp);
    fflush(stderr);
    #endif
    return rtpTimestamp;
}

u_int32_t RTPSink::presetNextTimestamp()
{
    struct timeval timeNow;

    gettimeofday(&timeNow, NULL);
    u_int32_t tsNow = convertToRTPTimestamp(timeNow);
    fTimestampBase = tsNow;
    fNextTimestampHasBeenPreset = True;
    return tsNow;
}

void RTPSink::getTotalBitrate(unsigned& outNumBytes, double& outElapsedTime)
{
    struct timeval timeNow;

    gettimeofday(&timeNow, NULL);
    outNumBytes    = fTotalOctetCount;
    outElapsedTime = (double) (timeNow.tv_sec - fTotalOctetCountStartTime.tv_sec)
                     + (timeNow.tv_usec - fTotalOctetCountStartTime.tv_usec) / 1000000.0;
    fTotalOctetCount = 0;
    fTotalOctetCountStartTime = timeNow;
}

void RTPSink::resetPresentationTimes()
{
    fInitialPresentationTime.tv_sec  = fMostRecentPresentationTime.tv_sec = 0;
    fInitialPresentationTime.tv_usec = fMostRecentPresentationTime.tv_usec = 0;
}

char const * RTPSink::sdpMediaType() const
{
    return "data";
}

char * RTPSink::rtpmapLine() const
{
    if (rtpPayloadType() >= 96) {
        char* encodingParamsPart;
        if (numChannels() != 1) {
            encodingParamsPart = new char[1 + 20 ];
            sprintf(encodingParamsPart, "/%d", numChannels());
        } else {
            encodingParamsPart = strDup("");
        }
        char const * const rtpmapFmt = "a=rtpmap:%d %s/%d%s\r\n";
        unsigned rtpmapFmtSize       = strlen(rtpmapFmt)
                                       + 3  + strlen(rtpPayloadFormatName())
                                       + 20  + strlen(encodingParamsPart);
        char* rtpmapLine = new char[rtpmapFmtSize];
        sprintf(rtpmapLine, rtpmapFmt,
                rtpPayloadType(), rtpPayloadFormatName(),
                rtpTimestampFrequency(), encodingParamsPart);
        delete[] encodingParamsPart;
        return rtpmapLine;
    } else {
        return strDup("");
    }
}

char const * RTPSink::auxSDPLine()
{
    return NULL;
}

RTPTransmissionStatsDB::RTPTransmissionStatsDB(RTPSink& rtpSink)
    : fOurRTPSink(rtpSink),
    fTable(HashTable::create(ONE_WORD_HASH_KEYS))
{
    fNumReceivers = 0;
}

RTPTransmissionStatsDB::~RTPTransmissionStatsDB()
{
    RTPTransmissionStats* stats;

    while ((stats = (RTPTransmissionStats *) fTable->RemoveNext()) != NULL) {
        delete stats;
    }
    delete fTable;
}

void RTPTransmissionStatsDB
::noteIncomingRR(u_int32_t SSRC, struct sockaddr_in const& lastFromAddress,
                 unsigned lossStats, unsigned lastPacketNumReceived,
                 unsigned jitter, unsigned lastSRTime, unsigned diffSR_RRTime)
{
    RTPTransmissionStats* stats = lookup(SSRC);

    if (stats == NULL) {
        stats = new RTPTransmissionStats(fOurRTPSink, SSRC);
        if (stats == NULL) {
            return;
        }
        add(SSRC, stats);
        #ifdef DEBUG_RR
        fprintf(stderr, "Adding new entry for SSRC %x in RTPTransmissionStatsDB\n", SSRC);
        #endif
    }
    stats->noteIncomingRR(lastFromAddress,
                          lossStats, lastPacketNumReceived, jitter,
                          lastSRTime, diffSR_RRTime);
}

void RTPTransmissionStatsDB::removeRecord(u_int32_t SSRC)
{
    RTPTransmissionStats* stats = lookup(SSRC);

    if (stats != NULL) {
        long SSRC_long = (long) SSRC;
        fTable->Remove((char const *) SSRC_long);
        --fNumReceivers;
        delete stats;
    }
}

RTPTransmissionStatsDB::Iterator
::Iterator(RTPTransmissionStatsDB& receptionStatsDB)
    : fIter(HashTable::Iterator::create(*(receptionStatsDB.fTable)))
{}

RTPTransmissionStatsDB::Iterator::~Iterator()
{
    delete fIter;
}

RTPTransmissionStats * RTPTransmissionStatsDB::Iterator::next()
{
    char const* key;

    return (RTPTransmissionStats *) (fIter->next(key));
}

RTPTransmissionStats * RTPTransmissionStatsDB::lookup(u_int32_t SSRC) const
{
    long SSRC_long = (long) SSRC;

    return (RTPTransmissionStats *) (fTable->Lookup((char const *) SSRC_long));
}

void RTPTransmissionStatsDB::add(u_int32_t SSRC, RTPTransmissionStats* stats)
{
    long SSRC_long = (long) SSRC;

    fTable->Add((char const *) SSRC_long, stats);
    ++fNumReceivers;
}

RTPTransmissionStats::RTPTransmissionStats(RTPSink& rtpSink, u_int32_t SSRC)
    : fOurRTPSink(rtpSink), fSSRC(SSRC), fLastPacketNumReceived(0),
    fPacketLossRatio(0), fTotNumPacketsLost(0), fJitter(0),
    fLastSRTime(0), fDiffSR_RRTime(0), fAtLeastTwoRRsHaveBeenReceived(False), fFirstPacket(True),
    fTotalOctetCount_hi(0), fTotalOctetCount_lo(0),
    fTotalPacketCount_hi(0), fTotalPacketCount_lo(0)
{
    gettimeofday(&fTimeCreated, NULL);
    fLastOctetCount  = rtpSink.octetCount();
    fLastPacketCount = rtpSink.packetCount();
}

RTPTransmissionStats::~RTPTransmissionStats() {}

void RTPTransmissionStats
::noteIncomingRR(struct sockaddr_in const& lastFromAddress,
                 unsigned lossStats, unsigned lastPacketNumReceived,
                 unsigned jitter, unsigned lastSRTime,
                 unsigned diffSR_RRTime)
{
    if (fFirstPacket) {
        fFirstPacket = False;
        fFirstPacketNumReported = lastPacketNumReceived;
    } else {
        fAtLeastTwoRRsHaveBeenReceived = True;
        fOldLastPacketNumReceived      = fLastPacketNumReceived;
        fOldTotNumPacketsLost = fTotNumPacketsLost;
    }
    gettimeofday(&fTimeReceived, NULL);
    fLastFromAddress       = lastFromAddress;
    fPacketLossRatio       = lossStats >> 24;
    fTotNumPacketsLost     = lossStats & 0xFFFFFF;
    fLastPacketNumReceived = lastPacketNumReceived;
    fJitter        = jitter;
    fLastSRTime    = lastSRTime;
    fDiffSR_RRTime = diffSR_RRTime;
    #ifdef DEBUG_RR
    fprintf(stderr,
            "RTCP RR data (received at %lu.%06ld): lossStats 0x%08x, lastPacketNumReceived 0x%08x, jitter 0x%08x, lastSRTime 0x%08x, diffSR_RRTime 0x%08x\n",
            fTimeReceived.tv_sec, fTimeReceived.tv_usec, lossStats, lastPacketNumReceived, jitter, lastSRTime,
            diffSR_RRTime);
    unsigned rtd = roundTripDelay();
    fprintf(stderr, "=> round-trip delay: 0x%04x (== %f seconds)\n", rtd, rtd / 65536.0);
    #endif
    u_int32_t newOctetCount  = fOurRTPSink.octetCount();
    u_int32_t octetCountDiff = newOctetCount - fLastOctetCount;
    fLastOctetCount = newOctetCount;
    u_int32_t prevTotalOctetCount_lo = fTotalOctetCount_lo;
    fTotalOctetCount_lo += octetCountDiff;
    if (fTotalOctetCount_lo < prevTotalOctetCount_lo) {
        ++fTotalOctetCount_hi;
    }
    u_int32_t newPacketCount  = fOurRTPSink.packetCount();
    u_int32_t packetCountDiff = newPacketCount - fLastPacketCount;
    fLastPacketCount = newPacketCount;
    u_int32_t prevTotalPacketCount_lo = fTotalPacketCount_lo;
    fTotalPacketCount_lo += packetCountDiff;
    if (fTotalPacketCount_lo < prevTotalPacketCount_lo) {
        ++fTotalPacketCount_hi;
    }
} // RTPTransmissionStats::noteIncomingRR

unsigned RTPTransmissionStats::roundTripDelay() const
{
    if (fLastSRTime == 0) {
        return 0;
    }
    unsigned lastReceivedTimeNTP_high =
        fTimeReceived.tv_sec + 0x83AA7E80;
    double fractionalPart        = (fTimeReceived.tv_usec * 0x0400) / 15625.0;
    unsigned lastReceivedTimeNTP =
        (unsigned) ((lastReceivedTimeNTP_high << 16) + fractionalPart + 0.5);
    int rawResult = lastReceivedTimeNTP - fLastSRTime - fDiffSR_RRTime;
    if (rawResult < 0) {
        rawResult = 0;
    }
    return (unsigned) rawResult;
}

void RTPTransmissionStats::getTotalOctetCount(u_int32_t& hi, u_int32_t& lo)
{
    hi = fTotalOctetCount_hi;
    lo = fTotalOctetCount_lo;
}

void RTPTransmissionStats::getTotalPacketCount(u_int32_t& hi, u_int32_t& lo)
{
    hi = fTotalPacketCount_hi;
    lo = fTotalPacketCount_lo;
}

unsigned RTPTransmissionStats::packetsReceivedSinceLastRR() const
{
    if (!fAtLeastTwoRRsHaveBeenReceived) {
        return 0;
    }
    return fLastPacketNumReceived - fOldLastPacketNumReceived;
}

int RTPTransmissionStats::packetsLostBetweenRR() const
{
    if (!fAtLeastTwoRRsHaveBeenReceived) {
        return 0;
    }
    return fTotNumPacketsLost - fOldTotNumPacketsLost;
}
