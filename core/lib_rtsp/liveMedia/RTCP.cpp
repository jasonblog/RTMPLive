#include "RTCP.hh"
#include "GroupsockHelper.hh"
#include "rtcp_from_spec.h"
class RTCPMemberDatabase
{
public:
    RTCPMemberDatabase(RTCPInstance& ourRTCPInstance)
        : fOurRTCPInstance(ourRTCPInstance), fNumMembers(1),
        fTable(HashTable::create(ONE_WORD_HASH_KEYS))
    {}

    virtual ~RTCPMemberDatabase()
    {
        delete fTable;
    }

    Boolean isMember(unsigned ssrc) const
    {
        return fTable->Lookup((char *) (long) ssrc) != NULL;
    }

    Boolean noteMembership(unsigned ssrc, unsigned curTimeCount)
    {
        Boolean isNew = !isMember(ssrc);

        if (isNew) {
            ++fNumMembers;
        }
        fTable->Add((char *) (long) ssrc, (void *) (long) curTimeCount);
        return isNew;
    }

    Boolean remove(unsigned ssrc)
    {
        Boolean wasPresent = fTable->Remove((char *) (long) ssrc);

        if (wasPresent) {
            --fNumMembers;
        }
        return wasPresent;
    }

    unsigned numMembers() const
    {
        return fNumMembers;
    }

    void reapOldMembers(unsigned threshold);
private:
    RTCPInstance& fOurRTCPInstance;
    unsigned fNumMembers;
    HashTable* fTable;
};
void RTCPMemberDatabase::reapOldMembers(unsigned threshold)
{
    Boolean foundOldMember;
    u_int32_t oldSSRC = 0;

    do {
        foundOldMember = False;
        HashTable::Iterator* iter =
            HashTable::Iterator::create(*fTable);
        uintptr_t timeCount;
        char const* key;
        while ((timeCount = (uintptr_t) (iter->next(key))) != 0) {
            #ifdef DEBUG
            fprintf(stderr, "reap: checking SSRC 0x%lx: %ld (threshold %d)\n", (unsigned long) key, timeCount,
                    threshold);
            #endif
            if (timeCount < (uintptr_t) threshold) {
                uintptr_t ssrc = (uintptr_t) key;
                oldSSRC        = (u_int32_t) ssrc;
                foundOldMember = True;
            }
        }
        delete iter;
        if (foundOldMember) {
            #ifdef DEBUG
            fprintf(stderr, "reap: removing SSRC 0x%x\n", oldSSRC);
            #endif
            fOurRTCPInstance.removeSSRC(oldSSRC, True);
        }
    } while (foundOldMember);
}

static double dTimeNow()
{
    struct timeval timeNow;

    gettimeofday(&timeNow, NULL);
    return (double) (timeNow.tv_sec + timeNow.tv_usec / 1000000.0);
}

static unsigned const maxRTCPPacketSize       = 1450;
static unsigned const preferredRTCPPacketSize = 1000;
RTCPInstance::RTCPInstance(UsageEnvironment& env, Groupsock* RTCPgs,
                           unsigned totSessionBW,
                           unsigned char const* cname,
                           RTPSink* sink, RTPSource* source,
                           Boolean isSSMSource)
    : Medium(env), fRTCPInterface(this, RTCPgs), fTotSessionBW(totSessionBW),
    fSink(sink), fSource(source), fIsSSMSource(isSSMSource),
    fCNAME(RTCP_SDES_CNAME, cname), fOutgoingReportCount(1),
    fAveRTCPSize(0), fIsInitial(1), fPrevNumMembers(0),
    fLastSentSize(0), fLastReceivedSize(0), fLastReceivedSSRC(0),
    fTypeOfEvent(EVENT_UNKNOWN), fTypeOfPacket(PACKET_UNKNOWN_TYPE),
    fHaveJustSentPacket(False), fLastPacketSentSize(0),
    fByeHandlerTask(NULL), fByeHandlerClientData(NULL),
    fSRHandlerTask(NULL), fSRHandlerClientData(NULL),
    fRRHandlerTask(NULL), fRRHandlerClientData(NULL),
    fSpecificRRHandlerTable(NULL)
{
    #ifdef DEBUG
    fprintf(stderr, "RTCPInstance[%p]::RTCPInstance()\n", this);
    #endif
    if (fTotSessionBW == 0) {
        env << "RTCPInstance::RTCPInstance error: totSessionBW parameter should not be zero!\n";
        fTotSessionBW = 1;
    }
    if (isSSMSource) {
        RTCPgs->multicastSendOnly();
    }
    double timeNow = dTimeNow();
    fPrevReportTime = fNextReportTime = timeNow;
    fKnownMembers   = new RTCPMemberDatabase(*this);
    fInBuf = new unsigned char[maxRTCPPacketSize];
    if (fKnownMembers == NULL || fInBuf == NULL) {
        return;
    }
    fNumBytesAlreadyRead = 0;
    fOutBuf = new OutPacketBuffer(preferredRTCPPacketSize, maxRTCPPacketSize, maxRTCPPacketSize);
    if (fOutBuf == NULL) {
        return;
    }
    if (fSource != NULL && fSource->RTPgs() == RTCPgs) {
        fSource->registerForMultiplexedRTCPPackets(this);
    } else {
        TaskScheduler::BackgroundHandlerProc* handler =
            (TaskScheduler::BackgroundHandlerProc *) &incomingReportHandler;
        fRTCPInterface.startNetworkReading(handler);
    }
    fTypeOfEvent = EVENT_REPORT;
    onExpire(this);
}

struct RRHandlerRecord {
    TaskFunc* rrHandlerTask;
    void*     rrHandlerClientData;
};
RTCPInstance::~RTCPInstance()
{
    #ifdef DEBUG
    fprintf(stderr, "RTCPInstance[%p]::~RTCPInstance()\n", this);
    #endif
    if (fSource != NULL) {
        fSource->deregisterForMultiplexedRTCPPackets();
    }
    fTypeOfEvent = EVENT_BYE;
    sendBYE();
    if (fSpecificRRHandlerTable != NULL) {
        AddressPortLookupTable::Iterator iter(*fSpecificRRHandlerTable);
        RRHandlerRecord* rrHandler;
        while ((rrHandler = (RRHandlerRecord *) iter.next()) != NULL) {
            delete rrHandler;
        }
        delete fSpecificRRHandlerTable;
    }
    delete fKnownMembers;
    delete fOutBuf;
    delete[] fInBuf;
}

RTCPInstance * RTCPInstance::createNew(UsageEnvironment& env, Groupsock* RTCPgs,
                                       unsigned totSessionBW,
                                       unsigned char const* cname,
                                       RTPSink* sink, RTPSource* source,
                                       Boolean isSSMSource)
{
    return new RTCPInstance(env, RTCPgs, totSessionBW, cname, sink, source,
                            isSSMSource);
}

Boolean RTCPInstance::lookupByName(UsageEnvironment& env,
                                   char const*     instanceName,
                                   RTCPInstance *  & resultInstance)
{
    resultInstance = NULL;
    Medium* medium;
    if (!Medium::lookupByName(env, instanceName, medium)) {
        return False;
    }
    if (!medium->isRTCPInstance()) {
        env.setResultMsg(instanceName, " is not a RTCP instance");
        return False;
    }
    resultInstance = (RTCPInstance *) medium;
    return True;
}

Boolean RTCPInstance::isRTCPInstance() const
{
    return True;
}

unsigned RTCPInstance::numMembers() const
{
    if (fKnownMembers == NULL) {
        return 0;
    }
    return fKnownMembers->numMembers();
}

void RTCPInstance::setByeHandler(TaskFunc* handlerTask, void* clientData,
                                 Boolean handleActiveParticipantsOnly)
{
    fByeHandlerTask       = handlerTask;
    fByeHandlerClientData = clientData;
    fByeHandleActiveParticipantsOnly = handleActiveParticipantsOnly;
}

void RTCPInstance::setSRHandler(TaskFunc* handlerTask, void* clientData)
{
    fSRHandlerTask       = handlerTask;
    fSRHandlerClientData = clientData;
}

void RTCPInstance::setRRHandler(TaskFunc* handlerTask, void* clientData)
{
    fRRHandlerTask       = handlerTask;
    fRRHandlerClientData = clientData;
}

void RTCPInstance
::setSpecificRRHandler(netAddressBits fromAddress, Port fromPort,
                       TaskFunc* handlerTask, void* clientData)
{
    if (handlerTask == NULL && clientData == NULL) {
        unsetSpecificRRHandler(fromAddress, fromPort);
        return;
    }
    RRHandlerRecord* rrHandler = new RRHandlerRecord;
    rrHandler->rrHandlerTask       = handlerTask;
    rrHandler->rrHandlerClientData = clientData;
    if (fSpecificRRHandlerTable == NULL) {
        fSpecificRRHandlerTable = new AddressPortLookupTable;
    }
    RRHandlerRecord* existingRecord = (RRHandlerRecord *) fSpecificRRHandlerTable->Add(fromAddress, (~0), fromPort,
                                                                                       rrHandler);
    delete existingRecord;
}

void RTCPInstance
::unsetSpecificRRHandler(netAddressBits fromAddress, Port fromPort)
{
    if (fSpecificRRHandlerTable == NULL) {
        return;
    }
    RRHandlerRecord* rrHandler =
        (RRHandlerRecord *) (fSpecificRRHandlerTable->Lookup(fromAddress, (~0), fromPort));
    if (rrHandler != NULL) {
        fSpecificRRHandlerTable->Remove(fromAddress, (~0), fromPort);
        delete rrHandler;
    }
}

void RTCPInstance::setStreamSocket(int           sockNum,
                                   unsigned char streamChannelId)
{
    fRTCPInterface.stopNetworkReading();
    fRTCPInterface.setStreamSocket(sockNum, streamChannelId);
    TaskScheduler::BackgroundHandlerProc* handler =
        (TaskScheduler::BackgroundHandlerProc *) &incomingReportHandler;
    fRTCPInterface.startNetworkReading(handler);
}

void RTCPInstance::addStreamSocket(int           sockNum,
                                   unsigned char streamChannelId)
{
    envir().taskScheduler().turnOffBackgroundReadHandling(fRTCPInterface.gs()->socketNum());
    fRTCPInterface.addStreamSocket(sockNum, streamChannelId);
    TaskScheduler::BackgroundHandlerProc* handler =
        (TaskScheduler::BackgroundHandlerProc *) &incomingReportHandler;
    fRTCPInterface.startNetworkReading(handler);
}

void RTCPInstance
::injectReport(u_int8_t const* packet, unsigned packetSize, struct sockaddr_in const& fromAddress)
{
    if (packetSize > maxRTCPPacketSize) {
        packetSize = maxRTCPPacketSize;
    }
    memmove(fInBuf, packet, packetSize);
    processIncomingReport(packetSize, fromAddress, -1, 0xFF);
}

static unsigned const IP_UDP_HDR_SIZE = 28;
#define ADVANCE(n) pkt += (n); packetSize -= (n)
void RTCPInstance::incomingReportHandler(RTCPInstance* instance,
                                         int)
{
    instance->incomingReportHandler1();
}

void RTCPInstance::incomingReportHandler1()
{
    do {
        if (fNumBytesAlreadyRead >= maxRTCPPacketSize) {
            envir()
                <<
                "RTCPInstance error: Hit limit when reading incoming packet over TCP. Increase \"maxRTCPPacketSize\"\n";
            break;
        }
        unsigned numBytesRead;
        struct sockaddr_in fromAddress;
        int tcpSocketNum;
        unsigned char tcpStreamChannelId;
        Boolean packetReadWasIncomplete;
        Boolean readResult =
            fRTCPInterface.handleRead(&fInBuf[fNumBytesAlreadyRead], maxRTCPPacketSize - fNumBytesAlreadyRead,
                                      numBytesRead, fromAddress,
                                      tcpSocketNum, tcpStreamChannelId,
                                      packetReadWasIncomplete);
        unsigned packetSize = 0;
        if (packetReadWasIncomplete) {
            fNumBytesAlreadyRead += numBytesRead;
            return;
        } else {
            packetSize = fNumBytesAlreadyRead + numBytesRead;
            fNumBytesAlreadyRead = 0;
        }
        if (!readResult) {
            break;
        }
        Boolean packetWasFromOurHost = False;
        if (RTCPgs()->wasLoopedBackFromUs(envir(), fromAddress)) {
            packetWasFromOurHost = True;
            if (fHaveJustSentPacket && fLastPacketSentSize == packetSize) {
                fHaveJustSentPacket = False;
                break;
            }
        }
        if (fIsSSMSource && !packetWasFromOurHost) {
            fRTCPInterface.sendPacket(fInBuf, packetSize);
            fHaveJustSentPacket = True;
            fLastPacketSentSize = packetSize;
        }
        processIncomingReport(packetSize, fromAddress, tcpSocketNum, tcpStreamChannelId);
    } while (0);
} // RTCPInstance::incomingReportHandler1

void RTCPInstance
::processIncomingReport(unsigned packetSize, struct sockaddr_in const& fromAddress,
                        int tcpSocketNum, unsigned char tcpStreamChannelId)
{
    do {
        Boolean callByeHandler = False;
        unsigned char* pkt     = fInBuf;
        #ifdef DEBUG
        fprintf(stderr, "[%p]saw incoming RTCP packet (from ", this);
        if (tcpSocketNum < 0) {
            fprintf(stderr, "address %s, port %d", AddressString(fromAddress).val(), ntohs(fromAddress.sin_port));
        } else {
            fprintf(stderr, "TCP socket #%d, stream channel id %d", tcpSocketNum, tcpStreamChannelId);
        }
        fprintf(stderr, ")\n");
        for (unsigned i = 0; i < packetSize; ++i) {
            if (i % 4 == 0) {
                fprintf(stderr, " ");
            }
            fprintf(stderr, "%02x", pkt[i]);
        }
        fprintf(stderr, "\n");
        #endif // ifdef DEBUG
        int totPacketSize = IP_UDP_HDR_SIZE + packetSize;
        if (packetSize < 4) {
            break;
        }
        unsigned rtcpHdr = ntohl(*(u_int32_t *) pkt);
        if ((rtcpHdr & 0xE0FE0000) != (0x80000000 | (RTCP_PT_SR << 16))) {
            #ifdef DEBUG
            fprintf(stderr, "rejected bad RTCP packet: header 0x%08x\n", rtcpHdr);
            #endif
            break;
        }
        int typeOfPacket = PACKET_UNKNOWN_TYPE;
        unsigned reportSenderSSRC = 0;
        Boolean packetOK = False;
        while (1) {
            unsigned rc     = (rtcpHdr >> 24) & 0x1F;
            unsigned pt     = (rtcpHdr >> 16) & 0xFF;
            unsigned length = 4 * (rtcpHdr & 0xFFFF);
            ADVANCE(4);
            if (length > packetSize) {
                break;
            }
            if (length < 4) {
                break;
            }
            length -= 4;
            reportSenderSSRC = ntohl(*(u_int32_t *) pkt);
            ADVANCE(4);
            Boolean subPacketOK = False;
            switch (pt) {
                case RTCP_PT_SR: {
                    #ifdef DEBUG
                    fprintf(stderr, "SR\n");
                    #endif
                    if (length < 20) {
                        break;
                    }
                    length -= 20;
                    unsigned NTPmsw = ntohl(*(u_int32_t *) pkt);
                    ADVANCE(4);
                    unsigned NTPlsw = ntohl(*(u_int32_t *) pkt);
                    ADVANCE(4);
                    unsigned rtpTimestamp = ntohl(*(u_int32_t *) pkt);
                    ADVANCE(4);
                    if (fSource != NULL) {
                        RTPReceptionStatsDB& receptionStats =
                            fSource->receptionStatsDB();
                        receptionStats.noteIncomingSR(reportSenderSSRC,
                                                      NTPmsw, NTPlsw, rtpTimestamp);
                    }
                    ADVANCE(8);
                    if (fSRHandlerTask != NULL) {
                        (*fSRHandlerTask)(fSRHandlerClientData);
                    }
                }
                case RTCP_PT_RR: {
                    #ifdef DEBUG
                    fprintf(stderr, "RR\n");
                    #endif
                    unsigned reportBlocksSize = rc * (6 * 4);
                    if (length < reportBlocksSize) {
                        break;
                    }
                    length -= reportBlocksSize;
                    if (fSink != NULL) {
                        RTPTransmissionStatsDB& transmissionStats = fSink->transmissionStatsDB();
                        for (unsigned i = 0; i < rc; ++i) {
                            unsigned senderSSRC = ntohl(*(u_int32_t *) pkt);
                            ADVANCE(4);
                            if (senderSSRC == fSink->SSRC()) {
                                unsigned lossStats = ntohl(*(u_int32_t *) pkt);
                                ADVANCE(4);
                                unsigned highestReceived = ntohl(*(u_int32_t *) pkt);
                                ADVANCE(4);
                                unsigned jitter = ntohl(*(u_int32_t *) pkt);
                                ADVANCE(4);
                                unsigned timeLastSR = ntohl(*(u_int32_t *) pkt);
                                ADVANCE(4);
                                unsigned timeSinceLastSR = ntohl(*(u_int32_t *) pkt);
                                ADVANCE(4);
                                transmissionStats.noteIncomingRR(reportSenderSSRC, fromAddress,
                                                                 lossStats,
                                                                 highestReceived, jitter,
                                                                 timeLastSR, timeSinceLastSR);
                            } else {
                                ADVANCE(4 * 5);
                            }
                        }
                    } else {
                        ADVANCE(reportBlocksSize);
                    }
                    if (pt == RTCP_PT_RR) {
                        if (fSpecificRRHandlerTable != NULL) {
                            netAddressBits fromAddr;
                            portNumBits fromPortNum;
                            if (tcpSocketNum < 0) {
                                fromAddr    = fromAddress.sin_addr.s_addr;
                                fromPortNum = ntohs(fromAddress.sin_port);
                            } else {
                                fromAddr    = tcpSocketNum;
                                fromPortNum = tcpStreamChannelId;
                            }
                            Port fromPort(fromPortNum);
                            RRHandlerRecord* rrHandler =
                                (RRHandlerRecord *) (fSpecificRRHandlerTable->Lookup(fromAddr, (~0), fromPort));
                            if (rrHandler != NULL) {
                                if (rrHandler->rrHandlerTask != NULL) {
                                    (*(rrHandler->rrHandlerTask))(rrHandler->rrHandlerClientData);
                                }
                            }
                        }
                        if (fRRHandlerTask != NULL) {
                            (*fRRHandlerTask)(fRRHandlerClientData);
                        }
                    }
                    subPacketOK  = True;
                    typeOfPacket = PACKET_RTCP_REPORT;
                    break;
                }
                case RTCP_PT_BYE: {
                    #ifdef DEBUG
                    fprintf(stderr, "BYE\n");
                    #endif
                    if (fByeHandlerTask != NULL &&
                        (!fByeHandleActiveParticipantsOnly ||
                         (fSource != NULL &&
                          fSource->receptionStatsDB().lookup(reportSenderSSRC) != NULL) ||
                         (fSink != NULL &&
                          fSink->transmissionStatsDB().lookup(reportSenderSSRC) != NULL)))
                    {
                        callByeHandler = True;
                    }
                    subPacketOK  = True;
                    typeOfPacket = PACKET_BYE;
                    break;
                }
                default:
                    #ifdef DEBUG
                    fprintf(stderr, "UNSUPPORTED TYPE(0x%x)\n", pt);
                    #endif
                    subPacketOK = True;
                    break;
            }
            if (!subPacketOK) {
                break;
            }
            #ifdef DEBUG
            fprintf(stderr, "validated RTCP subpacket (type %d): %d, %d, %d, 0x%08x\n", typeOfPacket, rc, pt, length,
                    reportSenderSSRC);
            #endif
            ADVANCE(length);
            if (packetSize == 0) {
                packetOK = True;
                break;
            } else if (packetSize < 4) {
                #ifdef DEBUG
                fprintf(stderr, "extraneous %d bytes at end of RTCP packet!\n", packetSize);
                #endif
                break;
            }
            rtcpHdr = ntohl(*(u_int32_t *) pkt);
            if ((rtcpHdr & 0xC0000000) != 0x80000000) {
                #ifdef DEBUG
                fprintf(stderr, "bad RTCP subpacket: header 0x%08x\n", rtcpHdr);
                #endif
                break;
            }
        }
        if (!packetOK) {
            #ifdef DEBUG
            fprintf(stderr, "rejected bad RTCP subpacket: header 0x%08x\n", rtcpHdr);
            #endif
            break;
        } else {
            #ifdef DEBUG
            fprintf(stderr, "validated entire RTCP packet\n");
            #endif
        }
        onReceive(typeOfPacket, totPacketSize, reportSenderSSRC);
        if (callByeHandler && fByeHandlerTask != NULL) {
            TaskFunc* byeHandler = fByeHandlerTask;
            fByeHandlerTask = NULL;
            (*byeHandler)(fByeHandlerClientData);
        }
    } while (0);
} // RTCPInstance::processIncomingReport

void RTCPInstance::onReceive(int typeOfPacket, int totPacketSize,
                             unsigned ssrc)
{
    fTypeOfPacket     = typeOfPacket;
    fLastReceivedSize = totPacketSize;
    fLastReceivedSSRC = ssrc;
    int members = (int) numMembers();
    int senders = (fSink != NULL) ? 1 : 0;
    OnReceive(this,
              this,
              &members,
              &fPrevNumMembers,
              &senders,
              &fAveRTCPSize,
              &fPrevReportTime,
              dTimeNow(),
              fNextReportTime);
}

void RTCPInstance::sendReport()
{
    #ifdef DEBUG
    fprintf(stderr, "sending REPORT\n");
    #endif
    if (!addReport()) {
        return;
    }
    addSDES();
    sendBuiltPacket();
    const unsigned membershipReapPeriod = 5;
    if ((++fOutgoingReportCount) % membershipReapPeriod == 0) {
        unsigned threshold = fOutgoingReportCount - membershipReapPeriod;
        fKnownMembers->reapOldMembers(threshold);
    }
}

void RTCPInstance::sendBYE()
{
    #ifdef DEBUG
    fprintf(stderr, "sending BYE\n");
    #endif
    (void) addReport(True);
    addBYE();
    sendBuiltPacket();
}

void RTCPInstance::sendBuiltPacket()
{
    #ifdef DEBUG
    fprintf(stderr, "sending RTCP packet\n");
    unsigned char* p = fOutBuf->packet();
    for (unsigned i = 0; i < fOutBuf->curPacketSize(); ++i) {
        if (i % 4 == 0) {
            fprintf(stderr, " ");
        }
        fprintf(stderr, "%02x", p[i]);
    }
    fprintf(stderr, "\n");
    #endif
    unsigned reportSize = fOutBuf->curPacketSize();
    fRTCPInterface.sendPacket(fOutBuf->packet(), reportSize);
    fOutBuf->resetOffset();
    fLastSentSize       = IP_UDP_HDR_SIZE + reportSize;
    fHaveJustSentPacket = True;
    fLastPacketSentSize = reportSize;
}

int RTCPInstance::checkNewSSRC()
{
    return fKnownMembers->noteMembership(fLastReceivedSSRC,
                                         fOutgoingReportCount);
}

void RTCPInstance::removeLastReceivedSSRC()
{
    removeSSRC(fLastReceivedSSRC, False);
}

void RTCPInstance::removeSSRC(u_int32_t ssrc, Boolean alsoRemoveStats)
{
    fKnownMembers->remove(ssrc);
    if (alsoRemoveStats) {
        if (fSource != NULL) {
            fSource->receptionStatsDB().removeRecord(ssrc);
        }
        if (fSink != NULL) {
            fSink->transmissionStatsDB().removeRecord(ssrc);
        }
    }
}

void RTCPInstance::onExpire(RTCPInstance* instance)
{
    instance->onExpire1();
}

Boolean RTCPInstance::addReport(Boolean alwaysAdd)
{
    if (fSink != NULL) {
        if (!alwaysAdd) {
            if (!fSink->enableRTCPReports()) {
                return False;
            }
            if (fSink->nextTimestampHasBeenPreset()) {
                return False;
            }
        }
        addSR();
    } else if (fSource != NULL) {
        if (!alwaysAdd) {
            if (!fSource->enableRTCPReports()) {
                return False;
            }
        }
        addRR();
    }
    return True;
}

void RTCPInstance::addSR()
{
    enqueueCommonReportPrefix(RTCP_PT_SR, fSink->SSRC(),
                              5);
    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);
    fOutBuf->enqueueWord(timeNow.tv_sec + 0x83AA7E80);
    double fractionalPart = (timeNow.tv_usec / 15625.0) * 0x04000000;
    fOutBuf->enqueueWord((unsigned) (fractionalPart + 0.5));
    unsigned rtpTimestamp = fSink->convertToRTPTimestamp(timeNow);
    fOutBuf->enqueueWord(rtpTimestamp);
    fOutBuf->enqueueWord(fSink->packetCount());
    fOutBuf->enqueueWord(fSink->octetCount());
    enqueueCommonReportSuffix();
}

void RTCPInstance::addRR()
{
    enqueueCommonReportPrefix(RTCP_PT_RR, fSource->SSRC());
    enqueueCommonReportSuffix();
}

void RTCPInstance::enqueueCommonReportPrefix(unsigned char packetType,
                                             unsigned      SSRC,
                                             unsigned      numExtraWords)
{
    unsigned numReportingSources;

    if (fSource == NULL) {
        numReportingSources = 0;
    } else {
        RTPReceptionStatsDB& allReceptionStats =
            fSource->receptionStatsDB();
        numReportingSources = allReceptionStats.numActiveSourcesSinceLastReset();
        if (numReportingSources >= 32) {
            numReportingSources = 32;
        }
    }
    unsigned rtcpHdr = 0x80000000;
    rtcpHdr |= (numReportingSources << 24);
    rtcpHdr |= (packetType << 16);
    rtcpHdr |= (1 + numExtraWords + 6 * numReportingSources);
    fOutBuf->enqueueWord(rtcpHdr);
    fOutBuf->enqueueWord(SSRC);
}

void RTCPInstance::enqueueCommonReportSuffix()
{
    if (fSource != NULL) {
        RTPReceptionStatsDB& allReceptionStats =
            fSource->receptionStatsDB();
        RTPReceptionStatsDB::Iterator iterator(allReceptionStats);
        while (1) {
            RTPReceptionStats* receptionStats = iterator.next();
            if (receptionStats == NULL) {
                break;
            }
            enqueueReportBlock(receptionStats);
        }
        allReceptionStats.reset();
    }
}

void RTCPInstance::enqueueReportBlock(RTPReceptionStats* stats)
{
    fOutBuf->enqueueWord(stats->SSRC());
    unsigned highestExtSeqNumReceived = stats->highestExtSeqNumReceived();
    unsigned totNumExpected =
        highestExtSeqNumReceived - stats->baseExtSeqNumReceived();
    int totNumLost = totNumExpected - stats->totNumPacketsReceived();
    if (totNumLost > 0x007FFFFF) {
        totNumLost = 0x007FFFFF;
    } else if (totNumLost < 0) {
        if (totNumLost < -0x00800000) {
            totNumLost = 0x00800000;
        }
        totNumLost &= 0x00FFFFFF;
    }
    unsigned numExpectedSinceLastReset =
        highestExtSeqNumReceived - stats->lastResetExtSeqNumReceived();
    int numLostSinceLastReset =
        numExpectedSinceLastReset - stats->numPacketsReceivedSinceLastReset();
    unsigned char lossFraction;
    if (numExpectedSinceLastReset == 0 || numLostSinceLastReset < 0) {
        lossFraction = 0;
    } else {
        lossFraction = (unsigned char)
                       ((numLostSinceLastReset << 8) / numExpectedSinceLastReset);
    }
    fOutBuf->enqueueWord((lossFraction << 24) | totNumLost);
    fOutBuf->enqueueWord(highestExtSeqNumReceived);
    fOutBuf->enqueueWord(stats->jitter());
    unsigned NTPmsw = stats->lastReceivedSR_NTPmsw();
    unsigned NTPlsw = stats->lastReceivedSR_NTPlsw();
    unsigned LSR    = ((NTPmsw & 0xFFFF) << 16) | (NTPlsw >> 16);
    fOutBuf->enqueueWord(LSR);
    struct timeval const& LSRtime = stats->lastReceivedSR_time();
    struct timeval timeNow, timeSinceLSR;
    gettimeofday(&timeNow, NULL);
    if (timeNow.tv_usec < LSRtime.tv_usec) {
        timeNow.tv_usec += 1000000;
        timeNow.tv_sec  -= 1;
    }
    timeSinceLSR.tv_sec  = timeNow.tv_sec - LSRtime.tv_sec;
    timeSinceLSR.tv_usec = timeNow.tv_usec - LSRtime.tv_usec;
    unsigned DLSR;
    if (LSR == 0) {
        DLSR = 0;
    } else {
        DLSR = (timeSinceLSR.tv_sec << 16)
               | ((((timeSinceLSR.tv_usec << 11) + 15625) / 31250) & 0xFFFF);
    }
    fOutBuf->enqueueWord(DLSR);
} // RTCPInstance::enqueueReportBlock

void RTCPInstance::addSDES()
{
    unsigned numBytes = 4;

    numBytes += fCNAME.totalSize();
    numBytes += 1;
    unsigned num4ByteWords = (numBytes + 3) / 4;
    unsigned rtcpHdr       = 0x81000000;
    rtcpHdr |= (RTCP_PT_SDES << 16);
    rtcpHdr |= num4ByteWords;
    fOutBuf->enqueueWord(rtcpHdr);
    if (fSource != NULL) {
        fOutBuf->enqueueWord(fSource->SSRC());
    } else if (fSink != NULL) {
        fOutBuf->enqueueWord(fSink->SSRC());
    }
    fOutBuf->enqueue(fCNAME.data(), fCNAME.totalSize());
    unsigned numPaddingBytesNeeded = 4 - (fOutBuf->curPacketSize() % 4);
    unsigned char const zero       = '\0';
    while (numPaddingBytesNeeded-- > 0) {
        fOutBuf->enqueue(&zero, 1);
    }
}

void RTCPInstance::addBYE()
{
    unsigned rtcpHdr = 0x81000000;

    rtcpHdr |= (RTCP_PT_BYE << 16);
    rtcpHdr |= 1;
    fOutBuf->enqueueWord(rtcpHdr);
    if (fSource != NULL) {
        fOutBuf->enqueueWord(fSource->SSRC());
    } else if (fSink != NULL) {
        fOutBuf->enqueueWord(fSink->SSRC());
    }
}

void RTCPInstance::schedule(double nextTime)
{
    fNextReportTime = nextTime;
    double secondsToDelay = nextTime - dTimeNow();
    if (secondsToDelay < 0) {
        secondsToDelay = 0;
    }
    #ifdef DEBUG
    fprintf(stderr, "schedule(%f->%f)\n", secondsToDelay, nextTime);
    #endif
    int64_t usToGo = (int64_t) (secondsToDelay * 1000000);
    nextTask() = envir().taskScheduler().scheduleDelayedTask(usToGo,
                                                             (TaskFunc *) RTCPInstance::onExpire, this);
}

void RTCPInstance::reschedule(double nextTime)
{
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    schedule(nextTime);
}

void RTCPInstance::onExpire1()
{
    double rtcpBW = 0.05 * fTotSessionBW * 1024 / 8;

    OnExpire(this,
             numMembers(),
             (fSink != NULL) ? 1 : 0,
             rtcpBW,
             (fSink != NULL) ? 1 : 0,
             &fAveRTCPSize,
             &fIsInitial,
             dTimeNow(),
             &fPrevReportTime,
             &fPrevNumMembers
    );
}

SDESItem::SDESItem(unsigned char tag, unsigned char const* value)
{
    unsigned length = strlen((char const *) value);

    if (length > 0xFF) {
        length = 0xFF;
    }
    fData[0] = tag;
    fData[1] = (unsigned char) length;
    memmove(&fData[2], value, length);
}

unsigned SDESItem::totalSize() const
{
    return 2 + (unsigned) fData[1];
}

extern "C" void Schedule(double nextTime, event e)
{
    RTCPInstance* instance = (RTCPInstance *) e;

    if (instance == NULL) {
        return;
    }
    instance->schedule(nextTime);
}

extern "C" void Reschedule(double nextTime, event e)
{
    RTCPInstance* instance = (RTCPInstance *) e;

    if (instance == NULL) {
        return;
    }
    instance->reschedule(nextTime);
}

extern "C" void SendRTCPReport(event e)
{
    RTCPInstance* instance = (RTCPInstance *) e;

    if (instance == NULL) {
        return;
    }
    instance->sendReport();
}

extern "C" void SendBYEPacket(event e)
{
    RTCPInstance* instance = (RTCPInstance *) e;

    if (instance == NULL) {
        return;
    }
    instance->sendBYE();
}

extern "C" int TypeOfEvent(event e)
{
    RTCPInstance* instance = (RTCPInstance *) e;

    if (instance == NULL) {
        return EVENT_UNKNOWN;
    }
    return instance->typeOfEvent();
}

extern "C" int SentPacketSize(event e)
{
    RTCPInstance* instance = (RTCPInstance *) e;

    if (instance == NULL) {
        return 0;
    }
    return instance->sentPacketSize();
}

extern "C" int PacketType(packet p)
{
    RTCPInstance* instance = (RTCPInstance *) p;

    if (instance == NULL) {
        return PACKET_UNKNOWN_TYPE;
    }
    return instance->packetType();
}

extern "C" int ReceivedPacketSize(packet p)
{
    RTCPInstance* instance = (RTCPInstance *) p;

    if (instance == NULL) {
        return 0;
    }
    return instance->receivedPacketSize();
}

extern "C" int NewMember(packet p)
{
    RTCPInstance* instance = (RTCPInstance *) p;

    if (instance == NULL) {
        return 0;
    }
    return instance->checkNewSSRC();
}

extern "C" int NewSender(packet)
{
    return 0;
}

extern "C" void AddMember(packet)
{}

extern "C" void AddSender(packet)
{}

extern "C" void RemoveMember(packet p)
{
    RTCPInstance* instance = (RTCPInstance *) p;

    if (instance == NULL) {
        return;
    }
    instance->removeLastReceivedSSRC();
}

extern "C" void RemoveSender(packet)
{}

extern "C" double drand30()
{
    unsigned tmp = our_random() & 0x3FFFFFFF;

    return tmp / (double) (1024 * 1024 * 1024);
}
