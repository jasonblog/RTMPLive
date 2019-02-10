#include "PassiveServerMediaSubsession.hh"
#include <GroupsockHelper.hh>
PassiveServerMediaSubsession * PassiveServerMediaSubsession::createNew(RTPSink       & rtpSink,
                                                                       RTCPInstance* rtcpInstance)
{
    return new PassiveServerMediaSubsession(rtpSink, rtcpInstance);
}

PassiveServerMediaSubsession
::PassiveServerMediaSubsession(RTPSink& rtpSink, RTCPInstance* rtcpInstance)
    : ServerMediaSubsession(rtpSink.envir()),
    fSDPLines(NULL), fRTPSink(rtpSink), fRTCPInstance(rtcpInstance)
{
    fClientRTCPSourceRecords = HashTable::create(ONE_WORD_HASH_KEYS);
}

class RTCPSourceRecord
{
public:
    RTCPSourceRecord(netAddressBits addr, Port const& port)
        : addr(addr), port(port)
    {}

    netAddressBits addr;
    Port port;
};
PassiveServerMediaSubsession::~PassiveServerMediaSubsession()
{
    delete[] fSDPLines;
    while (1) {
        RTCPSourceRecord* source = (RTCPSourceRecord *) (fClientRTCPSourceRecords->RemoveNext());
        if (source == NULL) {
            break;
        }
        delete source;
    }
    delete fClientRTCPSourceRecords;
}

Boolean PassiveServerMediaSubsession::rtcpIsMuxed()
{
    if (fRTCPInstance == NULL) {
        return False;
    }
    return &(fRTPSink.groupsockBeingUsed()) == fRTCPInstance->RTCPgs();
}

char const * PassiveServerMediaSubsession::sdpLines()
{
    if (fSDPLines == NULL) {
        Groupsock const& gs = fRTPSink.groupsockBeingUsed();
        AddressString groupAddressStr(gs.groupAddress());
        unsigned short portNum       = ntohs(gs.port().num());
        unsigned char ttl            = gs.ttl();
        unsigned char rtpPayloadType = fRTPSink.rtpPayloadType();
        char const* mediaType        = fRTPSink.sdpMediaType();
        unsigned estBitrate          =
            fRTCPInstance == NULL ? 50 : fRTCPInstance->totSessionBW();
        char* rtpmapLine        = fRTPSink.rtpmapLine();
        char const* rtcpmuxLine = rtcpIsMuxed() ? "a=rtcp-mux\r\n" : "";
        char const* rangeLine   = rangeSDPLine();
        char const* auxSDPLine  = fRTPSink.auxSDPLine();
        if (auxSDPLine == NULL) {
            auxSDPLine = "";
        }
        char const * const sdpFmt =
            "m=%s %d RTP/AVP %d\r\n"
            "c=IN IP4 %s/%d\r\n"
            "b=AS:%u\r\n"
            "%s"
            "%s"
            "%s"
            "%s"
            "a=control:%s\r\n";
        unsigned sdpFmtSize = strlen(sdpFmt)
                              + strlen(mediaType) + 5  + 3
                              + strlen(groupAddressStr.val()) + 3
                              + 20
                              + strlen(rtpmapLine)
                              + strlen(rtcpmuxLine)
                              + strlen(rangeLine)
                              + strlen(auxSDPLine)
                              + strlen(trackId());
        char* sdpLines = new char[sdpFmtSize];
        sprintf(sdpLines, sdpFmt,
                mediaType,
                portNum,
                rtpPayloadType,
                groupAddressStr.val(),
                ttl,
                estBitrate,
                rtpmapLine,
                rtcpmuxLine,
                rangeLine,
                auxSDPLine,
                trackId());
        delete[](char *) rangeLine;
        delete[] rtpmapLine;
        fSDPLines = strDup(sdpLines);
        delete[] sdpLines;
    }
    return fSDPLines;
} // PassiveServerMediaSubsession::sdpLines

void PassiveServerMediaSubsession
::getStreamParameters(unsigned       clientSessionId,
                      netAddressBits clientAddress,
                      Port const&,
                      Port const     & clientRTCPPort,
                      int,
                      unsigned char,
                      unsigned char,
                      netAddressBits& destinationAddress,
                      u_int8_t      & destinationTTL,
                      Boolean       & isMulticast,
                      Port          & serverRTPPort,
                      Port          & serverRTCPPort,
                      void *        & streamToken)
{
    isMulticast = True;
    Groupsock& gs = fRTPSink.groupsockBeingUsed();
    if (destinationTTL == 255) {
        destinationTTL = gs.ttl();
    }
    if (destinationAddress == 0) {
        destinationAddress = gs.groupAddress().s_addr;
    } else {
        struct in_addr destinationAddr;
        destinationAddr.s_addr = destinationAddress;
        gs.changeDestinationParameters(destinationAddr, 0, destinationTTL);
        if (fRTCPInstance != NULL) {
            Groupsock* rtcpGS = fRTCPInstance->RTCPgs();
            rtcpGS->changeDestinationParameters(destinationAddr, 0, destinationTTL);
        }
    }
    serverRTPPort = gs.port();
    if (fRTCPInstance != NULL) {
        Groupsock* rtcpGS = fRTCPInstance->RTCPgs();
        serverRTCPPort = rtcpGS->port();
    }
    streamToken = NULL;
    RTCPSourceRecord* source = new RTCPSourceRecord(clientAddress, clientRTCPPort);
    fClientRTCPSourceRecords->Add((char const *) clientSessionId, source);
}

void PassiveServerMediaSubsession::startStream(unsigned      clientSessionId,
                                               void *,
                                               TaskFunc*     rtcpRRHandler,
                                               void*         rtcpRRHandlerClientData,
                                               unsigned short& rtpSeqNum,
                                               unsigned      & rtpTimestamp,
                                               ServerRequestAlternativeByteHandler *,
                                               void *)
{
    rtpSeqNum    = fRTPSink.currentSeqNo();
    rtpTimestamp = fRTPSink.presetNextTimestamp();
    unsigned streamBitrate = fRTCPInstance == NULL ? 50 : fRTCPInstance->totSessionBW();
    unsigned rtpBufSize    = streamBitrate * 25 / 2;
    if (rtpBufSize < 50 * 1024) {
        rtpBufSize = 50 * 1024;
    }
    increaseSendBufferTo(envir(), fRTPSink.groupsockBeingUsed().socketNum(), rtpBufSize);
    if (fRTCPInstance != NULL) {
        fRTCPInstance->sendReport();
        RTCPSourceRecord* source = (RTCPSourceRecord *) (fClientRTCPSourceRecords->Lookup(
                                                             (char const *) clientSessionId));
        if (source != NULL) {
            fRTCPInstance->setSpecificRRHandler(source->addr, source->port,
                                                rtcpRRHandler, rtcpRRHandlerClientData);
        }
    }
}

float PassiveServerMediaSubsession::getCurrentNPT(void* streamToken)
{
    struct timeval const& creationTime = fRTPSink.creationTime();
    struct timeval timeNow;

    gettimeofday(&timeNow, NULL);
    return (float) (timeNow.tv_sec - creationTime.tv_sec + (timeNow.tv_usec - creationTime.tv_usec) / 1000000.0);
}

void PassiveServerMediaSubsession::deleteStream(unsigned clientSessionId, void *&)
{
    RTCPSourceRecord* source = (RTCPSourceRecord *) (fClientRTCPSourceRecords->Lookup((char const *) clientSessionId));

    if (source != NULL) {
        if (fRTCPInstance != NULL) {
            fRTCPInstance->unsetSpecificRRHandler(source->addr, source->port);
        }
        fClientRTCPSourceRecords->Remove((char const *) clientSessionId);
        delete source;
    }
}
