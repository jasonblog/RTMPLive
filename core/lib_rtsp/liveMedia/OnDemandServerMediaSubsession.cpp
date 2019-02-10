#include "OnDemandServerMediaSubsession.hh"
#include <GroupsockHelper.hh>
OnDemandServerMediaSubsession
::OnDemandServerMediaSubsession(UsageEnvironment& env,
                                Boolean         reuseFirstSource,
                                portNumBits     initialPortNum,
                                Boolean         multiplexRTCPWithRTP)
    : ServerMediaSubsession(env),
    fSDPLines(NULL), fReuseFirstSource(reuseFirstSource),
    fMultiplexRTCPWithRTP(multiplexRTCPWithRTP), fLastStreamToken(NULL)
{
    fDestinationsHashTable = HashTable::create(ONE_WORD_HASH_KEYS);
    if (fMultiplexRTCPWithRTP) {
        fInitialPortNum = initialPortNum;
    } else {
        fInitialPortNum = (initialPortNum + 1) & ~1;
    }
    gethostname(fCNAME, sizeof fCNAME);
    fCNAME[sizeof fCNAME - 1] = '\0';
}

OnDemandServerMediaSubsession::~OnDemandServerMediaSubsession()
{
    delete[] fSDPLines;
    while (1) {
        Destinations* destinations =
            (Destinations *) (fDestinationsHashTable->RemoveNext());
        if (destinations == NULL) {
            break;
        }
        delete destinations;
    }
    delete fDestinationsHashTable;
}

char const * OnDemandServerMediaSubsession::sdpLines()
{
    if (fSDPLines == NULL) {
        unsigned estBitrate;
        FramedSource* inputSource = createNewStreamSource(0, estBitrate);
        if (inputSource == NULL) {
            return NULL;
        }
        struct in_addr dummyAddr;
        dummyAddr.s_addr = 0;
        Groupsock dummyGroupsock(envir(), dummyAddr, 0, 0);
        unsigned char rtpPayloadType = 96 + trackNumber() - 1;
        RTPSink* dummyRTPSink        =
            createNewRTPSink(&dummyGroupsock, rtpPayloadType, inputSource);
        if (dummyRTPSink != NULL && dummyRTPSink->estimatedBitrate() > 0) {
            estBitrate = dummyRTPSink->estimatedBitrate();
        }
        setSDPLinesFromRTPSink(dummyRTPSink, inputSource, estBitrate);
        Medium::close(dummyRTPSink);
        closeStreamSource(inputSource);
    }
    return fSDPLines;
}

void OnDemandServerMediaSubsession
::getStreamParameters(unsigned       clientSessionId,
                      netAddressBits clientAddress,
                      Port const     & clientRTPPort,
                      Port const     & clientRTCPPort,
                      int            tcpSocketNum,
                      unsigned char  rtpChannelId,
                      unsigned char  rtcpChannelId,
                      netAddressBits & destinationAddress,
                      u_int8_t&,
                      Boolean        & isMulticast,
                      Port           & serverRTPPort,
                      Port           & serverRTCPPort,
                      void *         & streamToken)
{
    if (destinationAddress == 0) {
        destinationAddress = clientAddress;
    }
    struct in_addr destinationAddr;
    destinationAddr.s_addr = destinationAddress;
    isMulticast = False;
    if (fLastStreamToken != NULL && fReuseFirstSource) {
        serverRTPPort  = ((StreamState *) fLastStreamToken)->serverRTPPort();
        serverRTCPPort = ((StreamState *) fLastStreamToken)->serverRTCPPort();
        ++((StreamState *) fLastStreamToken)->referenceCount();
        streamToken = fLastStreamToken;
    } else {
        unsigned streamBitrate;
        FramedSource* mediaSource =
            createNewStreamSource(clientSessionId, streamBitrate);
        RTPSink* rtpSink         = NULL;
        BasicUDPSink* udpSink    = NULL;
        Groupsock* rtpGroupsock  = NULL;
        Groupsock* rtcpGroupsock = NULL;
        if (clientRTPPort.num() != 0 || tcpSocketNum >= 0) {
            portNumBits serverPortNum;
            if (clientRTCPPort.num() == 0) {
                NoReuse dummy(envir());
                for (serverPortNum = fInitialPortNum;; ++serverPortNum) {
                    struct in_addr dummyAddr;
                    dummyAddr.s_addr = 0;
                    serverRTPPort    = serverPortNum;
                    rtpGroupsock     = new Groupsock(envir(), dummyAddr, serverRTPPort, 255);
                    if (rtpGroupsock->socketNum() >= 0) {
                        break;
                    }
                }
                udpSink = BasicUDPSink::createNew(envir(), rtpGroupsock);
            } else {
                NoReuse dummy(envir());
                for (portNumBits serverPortNum = fInitialPortNum;; ++serverPortNum) {
                    struct in_addr dummyAddr;
                    dummyAddr.s_addr = 0;
                    serverRTPPort    = serverPortNum;
                    rtpGroupsock     = new Groupsock(envir(), dummyAddr, serverRTPPort, 255);
                    if (rtpGroupsock->socketNum() < 0) {
                        delete rtpGroupsock;
                        continue;
                    }
                    if (fMultiplexRTCPWithRTP) {
                        serverRTCPPort = serverRTPPort;
                        rtcpGroupsock  = rtpGroupsock;
                    } else {
                        serverRTCPPort = ++serverPortNum;
                        rtcpGroupsock  = new Groupsock(envir(), dummyAddr, serverRTCPPort, 255);
                        if (rtcpGroupsock->socketNum() < 0) {
                            delete rtpGroupsock;
                            delete rtcpGroupsock;
                            continue;
                        }
                    }
                    break;
                }
                unsigned char rtpPayloadType = 96 + trackNumber() - 1;
                rtpSink = createNewRTPSink(rtpGroupsock, rtpPayloadType, mediaSource);
                if (rtpSink != NULL && rtpSink->estimatedBitrate() > 0) {
                    streamBitrate = rtpSink->estimatedBitrate();
                }
            }
            if (rtpGroupsock != NULL) {
                rtpGroupsock->removeAllDestinations();
            }
            if (rtcpGroupsock != NULL) {
                rtcpGroupsock->removeAllDestinations();
            }
            if (rtpGroupsock != NULL) {
                unsigned rtpBufSize = streamBitrate * 25 / 2;
                if (rtpBufSize < 50 * 1024) {
                    rtpBufSize = 50 * 1024;
                }
                increaseSendBufferTo(envir(), rtpGroupsock->socketNum(), rtpBufSize);
            }
        }
        streamToken = fLastStreamToken =
            new StreamState(*this, serverRTPPort, serverRTCPPort, rtpSink, udpSink,
                            streamBitrate, mediaSource,
                            rtpGroupsock, rtcpGroupsock);
    }
    Destinations* destinations;
    if (tcpSocketNum < 0) {
        destinations = new Destinations(destinationAddr, clientRTPPort, clientRTCPPort);
    } else {
        destinations = new Destinations(tcpSocketNum, rtpChannelId, rtcpChannelId);
    }
    fDestinationsHashTable->Add((char const *) clientSessionId, destinations);
} // OnDemandServerMediaSubsession::getStreamParameters

void OnDemandServerMediaSubsession::startStream(unsigned                             clientSessionId,
                                                void*                                streamToken,
                                                TaskFunc*                            rtcpRRHandler,
                                                void*                                rtcpRRHandlerClientData,
                                                unsigned short                       & rtpSeqNum,
                                                unsigned                             & rtpTimestamp,
                                                ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                                                void*                                serverRequestAlternativeByteHandlerClientData)
{
    StreamState* streamState   = (StreamState *) streamToken;
    Destinations* destinations =
        (Destinations *) (fDestinationsHashTable->Lookup((char const *) clientSessionId));

    if (streamState != NULL) {
        streamState->startPlaying(destinations,
                                  rtcpRRHandler, rtcpRRHandlerClientData,
                                  serverRequestAlternativeByteHandler, serverRequestAlternativeByteHandlerClientData);
        RTPSink* rtpSink = streamState->rtpSink();
        if (rtpSink != NULL) {
            rtpSeqNum    = rtpSink->currentSeqNo();
            rtpTimestamp = rtpSink->presetNextTimestamp();
        }
    }
}

void OnDemandServerMediaSubsession::pauseStream(unsigned,
                                                void* streamToken)
{
    if (fReuseFirstSource) {
        return;
    }
    StreamState* streamState = (StreamState *) streamToken;
    if (streamState != NULL) {
        streamState->pause();
    }
}

void OnDemandServerMediaSubsession::seekStream(unsigned,
                                               void* streamToken, double& seekNPT, double streamDuration,
                                               u_int64_t& numBytes)
{
    numBytes = 0;
    if (fReuseFirstSource) {
        return;
    }
    StreamState* streamState = (StreamState *) streamToken;
    if (streamState != NULL && streamState->mediaSource() != NULL) {
        seekStreamSource(streamState->mediaSource(), seekNPT, streamDuration, numBytes);
        streamState->startNPT() = (float) seekNPT;
        RTPSink* rtpSink = streamState->rtpSink();
        if (rtpSink != NULL) {
            rtpSink->resetPresentationTimes();
        }
    }
}

void OnDemandServerMediaSubsession::seekStream(unsigned,
                                               void* streamToken, char *& absStart, char *& absEnd)
{
    if (fReuseFirstSource) {
        return;
    }
    StreamState* streamState = (StreamState *) streamToken;
    if (streamState != NULL && streamState->mediaSource() != NULL) {
        seekStreamSource(streamState->mediaSource(), absStart, absEnd);
    }
}

void OnDemandServerMediaSubsession::nullSeekStream(unsigned, void* streamToken,
                                                   double streamEndTime, u_int64_t& numBytes)
{
    numBytes = 0;
    StreamState* streamState = (StreamState *) streamToken;
    if (streamState != NULL && streamState->mediaSource() != NULL) {
        streamState->startNPT() = getCurrentNPT(streamToken);
        double duration = streamEndTime - streamState->startNPT();
        if (duration < 0.0) {
            duration = 0.0;
        }
        setStreamSourceDuration(streamState->mediaSource(), duration, numBytes);
        RTPSink* rtpSink = streamState->rtpSink();
        if (rtpSink != NULL) {
            rtpSink->resetPresentationTimes();
        }
    }
}

void OnDemandServerMediaSubsession::setStreamScale(unsigned,
                                                   void* streamToken, float scale)
{
    if (fReuseFirstSource) {
        return;
    }
    StreamState* streamState = (StreamState *) streamToken;
    if (streamState != NULL && streamState->mediaSource() != NULL) {
        setStreamSourceScale(streamState->mediaSource(), scale);
    }
}

float OnDemandServerMediaSubsession::getCurrentNPT(void* streamToken)
{
    do {
        if (streamToken == NULL) {
            break;
        }
        StreamState* streamState = (StreamState *) streamToken;
        RTPSink* rtpSink         = streamState->rtpSink();
        if (rtpSink == NULL) {
            break;
        }
        return streamState->startNPT()
               + (rtpSink->mostRecentPresentationTime().tv_sec - rtpSink->initialPresentationTime().tv_sec)
               + (rtpSink->mostRecentPresentationTime().tv_sec - rtpSink->initialPresentationTime().tv_sec)
               / 1000000.0f;
    } while (0);
    return 0.0;
}

FramedSource * OnDemandServerMediaSubsession::getStreamSource(void* streamToken)
{
    if (streamToken == NULL) {
        return NULL;
    }
    StreamState* streamState = (StreamState *) streamToken;
    return streamState->mediaSource();
}

void OnDemandServerMediaSubsession::deleteStream(unsigned clientSessionId,
                                                 void *   & streamToken)
{
    StreamState* streamState   = (StreamState *) streamToken;
    Destinations* destinations =
        (Destinations *) (fDestinationsHashTable->Lookup((char const *) clientSessionId));

    if (destinations != NULL) {
        fDestinationsHashTable->Remove((char const *) clientSessionId);
        if (streamState != NULL) {
            streamState->endPlaying(destinations);
        }
    }
    if (streamState != NULL) {
        if (streamState->referenceCount() > 0) {
            --streamState->referenceCount();
        }
        if (streamState->referenceCount() == 0) {
            delete streamState;
            streamToken = NULL;
        }
    }
    delete destinations;
}

char const * OnDemandServerMediaSubsession
::getAuxSDPLine(RTPSink* rtpSink, FramedSource *)
{
    return rtpSink == NULL ? NULL : rtpSink->auxSDPLine();
}

void OnDemandServerMediaSubsession::seekStreamSource(FramedSource *,
                                                     double&, double, u_int64_t& numBytes)
{
    numBytes = 0;
}

void OnDemandServerMediaSubsession::seekStreamSource(FramedSource *,
                                                     char *& absStart, char *& absEnd)
{
    delete[] absStart;
    absStart = NULL;
    delete[] absEnd;
    absEnd = NULL;
}

void OnDemandServerMediaSubsession
::setStreamSourceScale(FramedSource *, float)
{}

void OnDemandServerMediaSubsession
::setStreamSourceDuration(FramedSource *, double, u_int64_t& numBytes)
{
    numBytes = 0;
}

void OnDemandServerMediaSubsession::closeStreamSource(FramedSource* inputSource)
{
    Medium::close(inputSource);
}

void OnDemandServerMediaSubsession
::setSDPLinesFromRTPSink(RTPSink* rtpSink, FramedSource* inputSource, unsigned estBitrate)
{
    if (rtpSink == NULL) {
        return;
    }
    char const* mediaType        = rtpSink->sdpMediaType();
    unsigned char rtpPayloadType = rtpSink->rtpPayloadType();
    AddressString ipAddressStr(fServerAddressForSDP);
    char* rtpmapLine        = rtpSink->rtpmapLine();
    char const* rtcpmuxLine = fMultiplexRTCPWithRTP ? "a=rtcp-mux\r\n" : "";
    char const* rangeLine   = rangeSDPLine();
    char const* auxSDPLine  = getAuxSDPLine(rtpSink, inputSource);
    if (auxSDPLine == NULL) {
        auxSDPLine = "";
    }
    char const * const sdpFmt =
        "m=%s %u RTP/AVP %d\r\n"
        "c=IN IP4 %s\r\n"
        "b=AS:%u\r\n"
        "%s"
        "%s"
        "%s"
        "%s"
        "a=control:%s\r\n";
    unsigned sdpFmtSize = strlen(sdpFmt)
                          + strlen(mediaType) + 5  + 3
                          + strlen(ipAddressStr.val())
                          + 20
                          + strlen(rtpmapLine)
                          + strlen(rtcpmuxLine)
                          + strlen(rangeLine)
                          + strlen(auxSDPLine)
                          + strlen(trackId());
    char* sdpLines = new char[sdpFmtSize];
    sprintf(sdpLines, sdpFmt,
            mediaType,
            fPortNumForSDP,
            rtpPayloadType,
            ipAddressStr.val(),
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
} // OnDemandServerMediaSubsession::setSDPLinesFromRTPSink

static void afterPlayingStreamState(void* clientData)
{
    StreamState* streamState = (StreamState *) clientData;

    if (streamState->streamDuration() == 0.0) {
        streamState->reclaim();
    }
}

StreamState::StreamState(OnDemandServerMediaSubsession& master,
                         Port const& serverRTPPort, Port const& serverRTCPPort,
                         RTPSink* rtpSink, BasicUDPSink* udpSink,
                         unsigned totalBW, FramedSource* mediaSource,
                         Groupsock* rtpGS, Groupsock* rtcpGS)
    : fMaster(master), fAreCurrentlyPlaying(False), fReferenceCount(1),
    fServerRTPPort(serverRTPPort), fServerRTCPPort(serverRTCPPort),
    fRTPSink(rtpSink), fUDPSink(udpSink), fStreamDuration(master.duration()),
    fTotalBW(totalBW), fRTCPInstance(NULL),
    fMediaSource(mediaSource), fStartNPT(0.0), fRTPgs(rtpGS), fRTCPgs(rtcpGS)
{}

StreamState::~StreamState()
{
    reclaim();
}

void StreamState
::startPlaying(Destinations* dests,
               TaskFunc* rtcpRRHandler, void* rtcpRRHandlerClientData,
               ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
               void* serverRequestAlternativeByteHandlerClientData)
{
    if (dests == NULL) {
        return;
    }
    if (fRTCPInstance == NULL && fRTPSink != NULL) {
        fRTCPInstance =
            RTCPInstance::createNew(fRTPSink->envir(), fRTCPgs,
                                    fTotalBW, (unsigned char *) fMaster.fCNAME,
                                    fRTPSink, NULL);
    }
    if (dests->isTCP) {
        if (fRTPSink != NULL) {
            fRTPSink->addStreamSocket(dests->tcpSocketNum, dests->rtpChannelId);
            RTPInterface
            ::setServerRequestAlternativeByteHandler(
                fRTPSink->envir(), dests->tcpSocketNum,
                serverRequestAlternativeByteHandler,
                serverRequestAlternativeByteHandlerClientData);
        }
        if (fRTCPInstance != NULL) {
            fRTCPInstance->addStreamSocket(dests->tcpSocketNum, dests->rtcpChannelId);
            fRTCPInstance->setSpecificRRHandler(dests->tcpSocketNum, dests->rtcpChannelId,
                                                rtcpRRHandler, rtcpRRHandlerClientData);
        }
    } else {
        if (fRTPgs != NULL) {
            fRTPgs->addDestination(dests->addr, dests->rtpPort);
        }
        if (fRTCPgs != NULL) {
            fRTCPgs->addDestination(dests->addr, dests->rtcpPort);
        }
        if (fRTCPInstance != NULL) {
            fRTCPInstance->setSpecificRRHandler(dests->addr.s_addr, dests->rtcpPort,
                                                rtcpRRHandler, rtcpRRHandlerClientData);
        }
    }
    if (fRTCPInstance != NULL) {
        fRTCPInstance->sendReport();
    }
    if (!fAreCurrentlyPlaying && fMediaSource != NULL) {
        if (fRTPSink != NULL) {
            fRTPSink->startPlaying(*fMediaSource, afterPlayingStreamState, this);
            fAreCurrentlyPlaying = True;
        } else if (fUDPSink != NULL) {
            fUDPSink->startPlaying(*fMediaSource, afterPlayingStreamState, this);
            fAreCurrentlyPlaying = True;
        }
    }
} // StreamState::startPlaying

void StreamState::pause()
{
    if (fRTPSink != NULL) {
        fRTPSink->stopPlaying();
    }
    if (fUDPSink != NULL) {
        fUDPSink->stopPlaying();
    }
    fAreCurrentlyPlaying = False;
}

void StreamState::endPlaying(Destinations* dests)
{
    #if 0
    if (fRTCPInstance != NULL) {
        fRTCPInstance->sendBYE();
    }
    #endif
    if (dests->isTCP) {
        if (fRTPSink != NULL) {
            fRTPSink->removeStreamSocket(dests->tcpSocketNum, dests->rtpChannelId);
        }
        if (fRTCPInstance != NULL) {
            fRTCPInstance->removeStreamSocket(dests->tcpSocketNum, dests->rtcpChannelId);
            fRTCPInstance->unsetSpecificRRHandler(dests->tcpSocketNum, dests->rtcpChannelId);
        }
    } else {
        if (fRTPgs != NULL) {
            fRTPgs->removeDestination(dests->addr, dests->rtpPort);
        }
        if (fRTCPgs != NULL) {
            fRTCPgs->removeDestination(dests->addr, dests->rtcpPort);
        }
        if (fRTCPInstance != NULL) {
            fRTCPInstance->unsetSpecificRRHandler(dests->addr.s_addr, dests->rtcpPort);
        }
    }
}

void StreamState::reclaim()
{
    Medium::close(fRTCPInstance);
    fRTCPInstance = NULL;
    Medium::close(fRTPSink);
    fRTPSink = NULL;
    Medium::close(fUDPSink);
    fUDPSink = NULL;
    fMaster.closeStreamSource(fMediaSource);
    fMediaSource = NULL;
    if (fMaster.fLastStreamToken == this) {
        fMaster.fLastStreamToken = NULL;
    }
    delete fRTPgs;
    if (fRTCPgs != fRTPgs) {
        delete fRTCPgs;
    }
    fRTPgs  = NULL;
    fRTCPgs = NULL;
}
