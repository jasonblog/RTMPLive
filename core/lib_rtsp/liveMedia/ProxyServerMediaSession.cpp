#include "liveMedia.hh"
#include "RTSPCommon.hh"
#include "GroupsockHelper.hh"
#ifndef MILLION
# define MILLION 1000000
#endif
class ProxyServerMediaSubsession : public OnDemandServerMediaSubsession
{
public:
    ProxyServerMediaSubsession(MediaSubsession& mediaSubsession);
    virtual ~ProxyServerMediaSubsession();
    char const * codecName() const
    {
        return fClientMediaSubsession.codecName();
    }

private:
    virtual FramedSource * createNewStreamSource(unsigned clientSessionId,
                                                 unsigned & estBitrate);
    virtual void closeStreamSource(FramedSource* inputSource);
    virtual RTPSink * createNewRTPSink(Groupsock*    rtpGroupsock,
                                       unsigned char rtpPayloadTypeIfDynamic,
                                       FramedSource* inputSource);
private:
    static void subsessionByeHandler(void* clientData);
    void subsessionByeHandler();
    int verbosityLevel() const
    {
        return ((ProxyServerMediaSession *) fParentSession)->fVerbosityLevel;
    }

private:
    friend class ProxyRTSPClient;
    MediaSubsession& fClientMediaSubsession;
    ProxyServerMediaSubsession* fNext;
    Boolean fHaveSetupStream;
};
UsageEnvironment& operator << (UsageEnvironment& env, const ProxyServerMediaSession& psms)
{
    return env << "ProxyServerMediaSession[\"" << psms.url() << "\"]";
}

ProxyRTSPClient * defaultCreateNewProxyRTSPClientFunc(ProxyServerMediaSession& ourServerMediaSession,
                                                      char const* rtspURL,
                                                      char const* username, char const* password,
                                                      portNumBits tunnelOverHTTPPortNum, int verbosityLevel,
                                                      int socketNumToServer)
{
    return new ProxyRTSPClient(ourServerMediaSession, rtspURL, username, password,
                               tunnelOverHTTPPortNum, verbosityLevel, socketNumToServer);
}

ProxyServerMediaSession * ProxyServerMediaSession
::createNew(UsageEnvironment& env, RTSPServer* ourRTSPServer,
            char const* inputStreamURL, char const* streamName,
            char const* username, char const* password,
            portNumBits tunnelOverHTTPPortNum, int verbosityLevel, int socketNumToServer)
{
    return new ProxyServerMediaSession(env, ourRTSPServer, inputStreamURL, streamName, username, password,
                                       tunnelOverHTTPPortNum, verbosityLevel, socketNumToServer);
}

ProxyServerMediaSession
::ProxyServerMediaSession(UsageEnvironment& env, RTSPServer* ourRTSPServer,
                          char const* inputStreamURL, char const* streamName,
                          char const* username, char const* password,
                          portNumBits tunnelOverHTTPPortNum, int verbosityLevel,
                          int socketNumToServer,
                          createNewProxyRTSPClientFunc* ourCreateNewProxyRTSPClientFunc)
    : ServerMediaSession(env, streamName, NULL, NULL, False, NULL),
    describeCompletedFlag(0), fOurRTSPServer(ourRTSPServer), fClientMediaSession(NULL),
    fVerbosityLevel(verbosityLevel),
    fPresentationTimeSessionNormalizer(new PresentationTimeSessionNormalizer(envir())),
    fCreateNewProxyRTSPClientFunc(ourCreateNewProxyRTSPClientFunc)
{
    fProxyRTSPClient =
        (*fCreateNewProxyRTSPClientFunc)(*this, inputStreamURL, username, password,
                                         tunnelOverHTTPPortNum,
                                         verbosityLevel > 0 ? verbosityLevel - 1 : verbosityLevel,
                                         socketNumToServer);
    ProxyRTSPClient::sendDESCRIBE(fProxyRTSPClient);
}

ProxyServerMediaSession::~ProxyServerMediaSession()
{
    if (fVerbosityLevel > 0) {
        envir() << *this << "::~ProxyServerMediaSession()\n";
    }
    if (fProxyRTSPClient != NULL) {
        fProxyRTSPClient->sendTeardownCommand(*fClientMediaSession, NULL, fProxyRTSPClient->auth());
    }
    Medium::close(fClientMediaSession);
    Medium::close(fProxyRTSPClient);
    delete fPresentationTimeSessionNormalizer;
}

char const * ProxyServerMediaSession::url() const
{
    return fProxyRTSPClient == NULL ? NULL : fProxyRTSPClient->url();
}

void ProxyServerMediaSession::continueAfterDESCRIBE(char const* sdpDescription)
{
    describeCompletedFlag = 1;
    do {
        fClientMediaSession = MediaSession::createNew(envir(), sdpDescription);
        if (fClientMediaSession == NULL) {
            break;
        }
        MediaSubsessionIterator iter(*fClientMediaSession);
        for (MediaSubsession* mss = iter.next(); mss != NULL; mss = iter.next()) {
            ServerMediaSubsession* smss = new ProxyServerMediaSubsession(*mss);
            addSubsession(smss);
            if (fVerbosityLevel > 0) {
                envir() << *this << " added new \"ProxyServerMediaSubsession\" for "
                        << mss->protocolName() << "/" << mss->mediumName() << "/" << mss->codecName() << " track\n";
            }
        }
    } while (0);
}

void ProxyServerMediaSession::resetDESCRIBEState()
{
    if (fOurRTSPServer != NULL) {
        fOurRTSPServer->closeAllClientSessionsForServerMediaSession(this);
    }
    deleteAllSubsessions();
    Medium::close(fClientMediaSession);
    fClientMediaSession = NULL;
}

static void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    char const* res;

    if (resultCode == 0) {
        res = resultString;
    } else {
        res = NULL;
    }
    ((ProxyRTSPClient *) rtspClient)->continueAfterDESCRIBE(res);
    delete[] resultString;
}

static void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    if (resultCode == 0) {
        ((ProxyRTSPClient *) rtspClient)->continueAfterSETUP();
    }
    delete[] resultString;
}

static void continueAfterOPTIONS(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    Boolean serverSupportsGetParameter = False;

    if (resultCode == 0) {
        serverSupportsGetParameter = RTSPOptionIsSupported("GET_PARAMETER", resultString);
    }
    ((ProxyRTSPClient *) rtspClient)->continueAfterLivenessCommand(resultCode, serverSupportsGetParameter);
    delete[] resultString;
}

#ifdef SEND_GET_PARAMETER_IF_SUPPORTED
static void continueAfterGET_PARAMETER(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    ((ProxyRTSPClient *) rtspClient)->continueAfterLivenessCommand(resultCode, True);
    delete[] resultString;
}

#endif
UsageEnvironment& operator << (UsageEnvironment& env, const ProxyRTSPClient& proxyRTSPClient)
{
    return env << "ProxyRTSPClient[\"" << proxyRTSPClient.url() << "\"]";
}

ProxyRTSPClient::ProxyRTSPClient(ProxyServerMediaSession& ourServerMediaSession, char const* rtspURL,
                                 char const* username, char const* password,
                                 portNumBits tunnelOverHTTPPortNum, int verbosityLevel, int socketNumToServer)
    : RTSPClient(ourServerMediaSession.envir(), rtspURL, verbosityLevel, "ProxyRTSPClient",
                 tunnelOverHTTPPortNum == (portNumBits) (~0) ? 0 : tunnelOverHTTPPortNum, socketNumToServer),
    fOurServerMediaSession(ourServerMediaSession), fOurURL(strDup(rtspURL)), fStreamRTPOverTCP(
        tunnelOverHTTPPortNum != 0),
    fSetupQueueHead(NULL), fSetupQueueTail(NULL), fNumSetupsDone(0), fNextDESCRIBEDelay(1),
    fServerSupportsGetParameter(False), fLastCommandWasPLAY(False),
    fLivenessCommandTask(NULL), fDESCRIBECommandTask(NULL), fSubsessionTimerTask(NULL)
{
    if (username != NULL && password != NULL) {
        fOurAuthenticator = new Authenticator(username, password);
    } else {
        fOurAuthenticator = NULL;
    }
}

void ProxyRTSPClient::reset()
{
    envir().taskScheduler().unscheduleDelayedTask(fLivenessCommandTask);
    fLivenessCommandTask = NULL;
    envir().taskScheduler().unscheduleDelayedTask(fDESCRIBECommandTask);
    fDESCRIBECommandTask = NULL;
    envir().taskScheduler().unscheduleDelayedTask(fSubsessionTimerTask);
    fSubsessionTimerTask = NULL;
    fSetupQueueHead      = fSetupQueueTail = NULL;
    fNumSetupsDone       = 0;
    fNextDESCRIBEDelay   = 1;
    fLastCommandWasPLAY  = False;
    RTSPClient::reset();
}

ProxyRTSPClient::~ProxyRTSPClient()
{
    reset();
    delete fOurAuthenticator;
    delete[] fOurURL;
}

void ProxyRTSPClient::continueAfterDESCRIBE(char const* sdpDescription)
{
    if (sdpDescription != NULL) {
        fOurServerMediaSession.continueAfterDESCRIBE(sdpDescription);
        scheduleLivenessCommand();
    } else {
        scheduleDESCRIBECommand();
    }
}

void ProxyRTSPClient::continueAfterLivenessCommand(int resultCode, Boolean serverSupportsGetParameter)
{
    if (resultCode != 0) {
        fServerSupportsGetParameter = False;
        if (resultCode < 0) {
            if (fVerbosityLevel > 0) {
                envir() << *this << ": lost connection to server ('errno': " << -resultCode << ").  Resetting...\n";
            }
        }
        reset();
        fOurServerMediaSession.resetDESCRIBEState();
        setBaseURL(fOurURL);
        sendDESCRIBE(this);
        return;
    }
    fServerSupportsGetParameter = serverSupportsGetParameter;
    scheduleLivenessCommand();
}

#define SUBSESSION_TIMEOUT_SECONDS 10
void ProxyRTSPClient::continueAfterSETUP()
{
    if (fVerbosityLevel > 0) {
        envir() << *this << "::continueAfterSETUP(): head codec: "
                << fSetupQueueHead->fClientMediaSubsession.codecName()
                << "; numSubsessions " << fSetupQueueHead->fParentSession->numSubsessions() << "\n\tqueue:";
        for (ProxyServerMediaSubsession* p = fSetupQueueHead; p != NULL; p = p->fNext) {
            envir() << "\t" << p->fClientMediaSubsession.codecName();
        }
        envir() << "\n";
    }
    envir().taskScheduler().unscheduleDelayedTask(fSubsessionTimerTask);
    ProxyServerMediaSubsession* smss = fSetupQueueHead;
    fSetupQueueHead = fSetupQueueHead->fNext;
    if (fSetupQueueHead == NULL) {
        fSetupQueueTail = NULL;
    }
    if (fSetupQueueHead != NULL) {
        sendSetupCommand(fSetupQueueHead->fClientMediaSubsession, ::continueAfterSETUP,
                         False, fStreamRTPOverTCP, False, fOurAuthenticator);
        ++fNumSetupsDone;
        fSetupQueueHead->fHaveSetupStream = True;
    } else {
        if (fNumSetupsDone >= smss->fParentSession->numSubsessions()) {
            sendPlayCommand(smss->fClientMediaSubsession.parentSession(), NULL, -1.0f, -1.0f, 1.0f, fOurAuthenticator);
            fLastCommandWasPLAY = True;
        } else {
            fSubsessionTimerTask =
                envir().taskScheduler().scheduleDelayedTask(SUBSESSION_TIMEOUT_SECONDS * MILLION,
                                                            (TaskFunc *) subsessionTimeout, this);
        }
    }
}

void ProxyRTSPClient::scheduleLivenessCommand()
{
    unsigned delayMax = sessionTimeoutParameter();

    if (delayMax == 0) {
        delayMax = 60;
    }
    unsigned const us_1stPart = delayMax * 500000;
    unsigned uSecondsToDelay;
    if (us_1stPart <= 1000000) {
        uSecondsToDelay = us_1stPart;
    } else {
        unsigned const us_2ndPart = us_1stPart - 1000000;
        uSecondsToDelay = us_1stPart + (us_2ndPart * our_random()) % us_2ndPart;
    }
    fLivenessCommandTask = envir().taskScheduler().scheduleDelayedTask(uSecondsToDelay, sendLivenessCommand, this);
}

void ProxyRTSPClient::sendLivenessCommand(void* clientData)
{
    ProxyRTSPClient* rtspClient = (ProxyRTSPClient *) clientData;

    #ifdef SEND_GET_PARAMETER_IF_SUPPORTED
    MediaSession* sess = rtspClient->fOurServerMediaSession.fClientMediaSession;
    if (rtspClient->fServerSupportsGetParameter && rtspClient->fNumSetupsDone > 0 && sess != NULL) {
        rtspClient->sendGetParameterCommand(*sess, ::continueAfterGET_PARAMETER, "", rtspClient->auth());
    } else {
    #endif
    rtspClient->sendOptionsCommand(::continueAfterOPTIONS, rtspClient->auth());
    #ifdef SEND_GET_PARAMETER_IF_SUPPORTED
}

    #endif
}
void ProxyRTSPClient::scheduleDESCRIBECommand()
{
    unsigned secondsToDelay;

    if (fNextDESCRIBEDelay <= 256) {
        secondsToDelay      = fNextDESCRIBEDelay;
        fNextDESCRIBEDelay *= 2;
    } else {
        secondsToDelay = 256 + (our_random() & 0xFF);
    }
    if (fVerbosityLevel > 0) {
        envir() << *this << ": RTSP \"DESCRIBE\" command failed; trying again in " << secondsToDelay << " seconds\n";
    }
    fDESCRIBECommandTask = envir().taskScheduler().scheduleDelayedTask(secondsToDelay * MILLION, sendDESCRIBE, this);
}

void ProxyRTSPClient::sendDESCRIBE(void* clientData)
{
    ProxyRTSPClient* rtspClient = (ProxyRTSPClient *) clientData;

    if (rtspClient != NULL) {
        rtspClient->sendDescribeCommand(::continueAfterDESCRIBE, rtspClient->auth());
    }
}

void ProxyRTSPClient::subsessionTimeout(void* clientData)
{
    ((ProxyRTSPClient *) clientData)->handleSubsessionTimeout();
}

void ProxyRTSPClient::handleSubsessionTimeout()
{
    MediaSession* sess = fOurServerMediaSession.fClientMediaSession;

    if (sess != NULL) {
        sendPlayCommand(*sess, NULL, -1.0f, -1.0f, 1.0f, fOurAuthenticator);
    }
    fLastCommandWasPLAY = True;
}

ProxyServerMediaSubsession::ProxyServerMediaSubsession(MediaSubsession& mediaSubsession)
    : OnDemandServerMediaSubsession(mediaSubsession.parentSession().envir(), True),
    fClientMediaSubsession(mediaSubsession), fNext(NULL), fHaveSetupStream(False)
{}

UsageEnvironment& operator << (UsageEnvironment& env, const ProxyServerMediaSubsession& psmss)
{
    return env << "ProxyServerMediaSubsession[\"" << psmss.codecName() << "\"]";
}

ProxyServerMediaSubsession::~ProxyServerMediaSubsession()
{
    if (verbosityLevel() > 0) {
        envir() << *this << "::~ProxyServerMediaSubsession()\n";
    }
}

FramedSource * ProxyServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
    ProxyServerMediaSession * const sms = (ProxyServerMediaSession *) fParentSession;

    if (verbosityLevel() > 0) {
        envir() << *this << "::createNewStreamSource(session id " << clientSessionId << ")\n";
    }
    if (fClientMediaSubsession.readSource() == NULL) {
        fClientMediaSubsession.receiveRawMP3ADUs();
        fClientMediaSubsession.receiveRawJPEGFrames();
        fClientMediaSubsession.initiate();
        if (verbosityLevel() > 0) {
            envir() << "\tInitiated: " << *this << "\n";
        }
        if (fClientMediaSubsession.readSource() != NULL) {
            char const * const codecName   = fClientMediaSubsession.codecName();
            FramedFilter* normalizerFilter = sms->fPresentationTimeSessionNormalizer
                                             ->createNewPresentationTimeSubsessionNormalizer(
                fClientMediaSubsession.readSource(), fClientMediaSubsession.rtpSource(),
                codecName);
            fClientMediaSubsession.addFilter(normalizerFilter);
            if (strcmp(codecName, "H264") == 0) {
                fClientMediaSubsession.addFilter(H264VideoStreamDiscreteFramer
                                                 ::createNew(envir(), fClientMediaSubsession.readSource()));
            } else if (strcmp(codecName, "H265") == 0) {
                fClientMediaSubsession.addFilter(H265VideoStreamDiscreteFramer
                                                 ::createNew(envir(), fClientMediaSubsession.readSource()));
            } else if (strcmp(codecName, "MP4V-ES") == 0) {
                fClientMediaSubsession.addFilter(MPEG4VideoStreamDiscreteFramer
                                                 ::createNew(envir(), fClientMediaSubsession.readSource(),
                                                             True));
            } else if (strcmp(codecName, "MPV") == 0) {
                fClientMediaSubsession.addFilter(MPEG1or2VideoStreamDiscreteFramer
                                                 ::createNew(envir(), fClientMediaSubsession.readSource(),
                                                             False, 5.0, True));
            } else if (strcmp(codecName, "DV") == 0) {
                fClientMediaSubsession.addFilter(DVVideoStreamFramer
                                                 ::createNew(envir(), fClientMediaSubsession.readSource(),
                                                             False, True));
            }
        }
        if (fClientMediaSubsession.rtcpInstance() != NULL) {
            fClientMediaSubsession.rtcpInstance()->setByeHandler(subsessionByeHandler, this);
        }
    }
    ProxyRTSPClient * const proxyRTSPClient = sms->fProxyRTSPClient;
    if (clientSessionId != 0) {
        if (!fHaveSetupStream) {
            Boolean queueWasEmpty = proxyRTSPClient->fSetupQueueHead == NULL;
            if (queueWasEmpty) {
                proxyRTSPClient->fSetupQueueHead = this;
            } else {
                proxyRTSPClient->fSetupQueueTail->fNext = this;
            }
            proxyRTSPClient->fSetupQueueTail = this;
            if (queueWasEmpty) {
                proxyRTSPClient->sendSetupCommand(fClientMediaSubsession, ::continueAfterSETUP,
                                                  False, proxyRTSPClient->fStreamRTPOverTCP, False,
                                                  proxyRTSPClient->auth());
                ++proxyRTSPClient->fNumSetupsDone;
                fHaveSetupStream = True;
            }
        } else {
            if (!proxyRTSPClient->fLastCommandWasPLAY) {
                proxyRTSPClient->sendPlayCommand(fClientMediaSubsession.parentSession(), NULL, -1.0f,
                                                 -1.0f, 1.0f, proxyRTSPClient->auth());
                proxyRTSPClient->fLastCommandWasPLAY = True;
            }
        }
    }
    estBitrate = fClientMediaSubsession.bandwidth();
    if (estBitrate == 0) {
        estBitrate = 50;
    }
    return fClientMediaSubsession.readSource();
} // ProxyServerMediaSubsession::createNewStreamSource

void ProxyServerMediaSubsession::closeStreamSource(FramedSource* inputSource)
{
    if (verbosityLevel() > 0) {
        envir() << *this << "::closeStreamSource()\n";
    }
    if (fHaveSetupStream) {
        ProxyServerMediaSession * const sms     = (ProxyServerMediaSession *) fParentSession;
        ProxyRTSPClient * const proxyRTSPClient = sms->fProxyRTSPClient;
        if (proxyRTSPClient->fLastCommandWasPLAY) {
            proxyRTSPClient->sendPauseCommand(fClientMediaSubsession.parentSession(), NULL, proxyRTSPClient->auth());
            proxyRTSPClient->fLastCommandWasPLAY = False;
        }
    }
}

RTPSink * ProxyServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource)
{
    if (verbosityLevel() > 0) {
        envir() << *this << "::createNewRTPSink()\n";
    }
    RTPSink* newSink;
    char const * const codecName = fClientMediaSubsession.codecName();
    if (strcmp(codecName, "AC3") == 0 || strcmp(codecName, "EAC3") == 0) {
        newSink = AC3AudioRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                             fClientMediaSubsession.rtpTimestampFrequency());
        #if 0
    } else if (strcmp(codecName, "AMR") == 0 || strcmp(codecName, "AMR-WB") == 0) {
        Boolean isWideband = strcmp(codecName, "AMR-WB") == 0;
        newSink = AMRAudioRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                             isWideband, fClientMediaSubsession.numChannels());
        #endif
    } else if (strcmp(codecName, "DV") == 0) {
        newSink = DVVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
    } else if (strcmp(codecName, "GSM") == 0) {
        newSink = GSMAudioRTPSink::createNew(envir(), rtpGroupsock);
    } else if (strcmp(codecName, "H263-1998") == 0 || strcmp(codecName, "H263-2000") == 0) {
        newSink = H263plusVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                                  fClientMediaSubsession.rtpTimestampFrequency());
    } else if (strcmp(codecName, "H264") == 0) {
        newSink = H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                              fClientMediaSubsession.fmtp_spropparametersets());
    } else if (strcmp(codecName, "H265") == 0) {
        newSink = H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                              fClientMediaSubsession.fmtp_spropvps(),
                                              fClientMediaSubsession.fmtp_spropsps(),
                                              fClientMediaSubsession.fmtp_sproppps());
    } else if (strcmp(codecName, "JPEG") == 0) {
        newSink = SimpleRTPSink::createNew(envir(), rtpGroupsock, 26, 90000, "video", "JPEG",
                                           1, False, False);
    } else if (strcmp(codecName, "MP4A-LATM") == 0) {
        newSink = MPEG4LATMAudioRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                                   fClientMediaSubsession.rtpTimestampFrequency(),
                                                   fClientMediaSubsession.fmtp_config(),
                                                   fClientMediaSubsession.numChannels());
    } else if (strcmp(codecName, "MP4V-ES") == 0) {
        newSink = MPEG4ESVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                                 fClientMediaSubsession.rtpTimestampFrequency(),
                                                 fClientMediaSubsession.attrVal_unsigned("profile-level-id"),
                                                 fClientMediaSubsession.fmtp_config());
    } else if (strcmp(codecName, "MPA") == 0) {
        newSink = MPEG1or2AudioRTPSink::createNew(envir(), rtpGroupsock);
    } else if (strcmp(codecName, "MPA-ROBUST") == 0) {
        newSink = MP3ADURTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
    } else if (strcmp(codecName, "MPEG4-GENERIC") == 0) {
        newSink = MPEG4GenericRTPSink::createNew(envir(), rtpGroupsock,
                                                 rtpPayloadTypeIfDynamic,
                                                 fClientMediaSubsession.rtpTimestampFrequency(),
                                                 fClientMediaSubsession.mediumName(),
                                                 fClientMediaSubsession.attrVal_strToLower("mode"),
                                                 fClientMediaSubsession.fmtp_config(),
                                                 fClientMediaSubsession.numChannels());
    } else if (strcmp(codecName, "MPV") == 0) {
        newSink = MPEG1or2VideoRTPSink::createNew(envir(), rtpGroupsock);
    } else if (strcmp(codecName, "OPUS") == 0) {
        newSink = SimpleRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                           48000, "audio", "OPUS", 2, False);
    } else if (strcmp(codecName, "T140") == 0) {
        newSink = T140TextRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
    } else if (strcmp(codecName, "THEORA") == 0) {
        newSink = TheoraVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                                fClientMediaSubsession.fmtp_config());
    } else if (strcmp(codecName, "VORBIS") == 0) {
        newSink = VorbisAudioRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                                fClientMediaSubsession.rtpTimestampFrequency(),
                                                fClientMediaSubsession.numChannels(),
                                                fClientMediaSubsession.fmtp_config());
    } else if (strcmp(codecName, "VP8") == 0) {
        newSink = VP8VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
    } else if (strcmp(codecName, "AMR") == 0 || strcmp(codecName, "AMR-WB") == 0) {
        if (verbosityLevel() > 0) {
            envir() << "\treturns NULL (because we currently don't support the proxying of \""
                    << fClientMediaSubsession.mediumName() << "/" << codecName << "\" streams)\n";
        }
        return NULL;
    } else if (strcmp(codecName, "QCELP") == 0 ||
               strcmp(codecName, "H261") == 0 ||
               strcmp(codecName, "H263-1998") == 0 || strcmp(codecName, "H263-2000") == 0 ||
               strcmp(codecName, "X-QT") == 0 || strcmp(codecName, "X-QUICKTIME") == 0)
    {
        if (verbosityLevel() > 0) {
            envir() << "\treturns NULL (because we don't have a \"RTPSink\" subclass for this RTP payload format)\n";
        }
        return NULL;
    } else {
        Boolean allowMultipleFramesPerPacket = True;
        Boolean doNormalMBitRule = True;
        if (strcmp(codecName, "MP2T") == 0) {
            doNormalMBitRule = False;
        }
        newSink = SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                           rtpPayloadTypeIfDynamic, fClientMediaSubsession.rtpTimestampFrequency(),
                                           fClientMediaSubsession.mediumName(), fClientMediaSubsession.codecName(),
                                           fClientMediaSubsession.numChannels(), allowMultipleFramesPerPacket,
                                           doNormalMBitRule);
    }
    newSink->enableRTCPReports() = False;
    PresentationTimeSubsessionNormalizer* ssNormalizer;
    if (strcmp(codecName, "H264") == 0 ||
        strcmp(codecName, "H265") == 0 ||
        strcmp(codecName, "MP4V-ES") == 0 ||
        strcmp(codecName, "MPV") == 0 ||
        strcmp(codecName, "DV") == 0)
    {
        ssNormalizer = (PresentationTimeSubsessionNormalizer *) (((FramedFilter *) inputSource)->inputSource());
    } else {
        ssNormalizer = (PresentationTimeSubsessionNormalizer *) inputSource;
    }
    ssNormalizer->setRTPSink(newSink);
    return newSink;
} // ProxyServerMediaSubsession::createNewRTPSink

void ProxyServerMediaSubsession::subsessionByeHandler(void* clientData)
{
    ((ProxyServerMediaSubsession *) clientData)->subsessionByeHandler();
}

void ProxyServerMediaSubsession::subsessionByeHandler()
{
    if (verbosityLevel() > 0) {
        envir() << *this << ": received RTCP \"BYE\".  (The back-end stream has ended.)\n";
    }
    fHaveSetupStream = False;
    fClientMediaSubsession.readSource()->handleClosure();
    ProxyServerMediaSession * const sms     = (ProxyServerMediaSession *) fParentSession;
    ProxyRTSPClient * const proxyRTSPClient = sms->fProxyRTSPClient;
    proxyRTSPClient->continueAfterLivenessCommand(1, proxyRTSPClient->fServerSupportsGetParameter);
}

PresentationTimeSessionNormalizer::PresentationTimeSessionNormalizer(UsageEnvironment& env)
    : Medium(env),
    fSubsessionNormalizers(NULL), fMasterSSNormalizer(NULL)
{}

PresentationTimeSessionNormalizer::~PresentationTimeSessionNormalizer()
{
    while (fSubsessionNormalizers != NULL) {
        delete fSubsessionNormalizers;
    }
}

PresentationTimeSubsessionNormalizer * PresentationTimeSessionNormalizer::createNewPresentationTimeSubsessionNormalizer(
    FramedSource* inputSource, RTPSource* rtpSource,
    char const* codecName)
{
    fSubsessionNormalizers =
        new PresentationTimeSubsessionNormalizer(*this, inputSource, rtpSource, codecName, fSubsessionNormalizers);
    return fSubsessionNormalizers;
}

void PresentationTimeSessionNormalizer::normalizePresentationTime(PresentationTimeSubsessionNormalizer* ssNormalizer,
                                                                  struct timeval& toPT, struct timeval const& fromPT)
{
    Boolean const hasBeenSynced = ssNormalizer->fRTPSource->hasBeenSynchronizedUsingRTCP();

    if (!hasBeenSynced) {
        toPT = fromPT;
    } else {
        if (fMasterSSNormalizer == NULL) {
            fMasterSSNormalizer = ssNormalizer;
            struct timeval timeNow;
            gettimeofday(&timeNow, NULL);
            fPTAdjustment.tv_sec  = timeNow.tv_sec - fromPT.tv_sec;
            fPTAdjustment.tv_usec = timeNow.tv_usec - fromPT.tv_usec;
        }
        toPT.tv_sec  = fromPT.tv_sec + fPTAdjustment.tv_sec - 1;
        toPT.tv_usec = fromPT.tv_usec + fPTAdjustment.tv_usec + MILLION;
        while (toPT.tv_usec > MILLION) {
            ++toPT.tv_sec;
            toPT.tv_usec -= MILLION;
        }
        RTPSink * const rtpSink = ssNormalizer->fRTPSink;
        if (rtpSink != NULL) {
            rtpSink->enableRTCPReports() = True;
        }
    }
}

void PresentationTimeSessionNormalizer
::removePresentationTimeSubsessionNormalizer(PresentationTimeSubsessionNormalizer* ssNormalizer)
{
    if (fSubsessionNormalizers == ssNormalizer) {
        fSubsessionNormalizers = fSubsessionNormalizers->fNext;
    } else {
        PresentationTimeSubsessionNormalizer** ssPtrPtr = &(fSubsessionNormalizers->fNext);
        while (*ssPtrPtr != ssNormalizer) {
            ssPtrPtr = &((*ssPtrPtr)->fNext);
        }
        *ssPtrPtr = (*ssPtrPtr)->fNext;
    }
}

PresentationTimeSubsessionNormalizer
::PresentationTimeSubsessionNormalizer(PresentationTimeSessionNormalizer& parent, FramedSource* inputSource,
                                       RTPSource* rtpSource,
                                       char const* codecName, PresentationTimeSubsessionNormalizer* next)
    : FramedFilter(parent.envir(), inputSource),
    fParent(parent), fRTPSource(rtpSource), fRTPSink(NULL), fCodecName(codecName), fNext(next)
{}

PresentationTimeSubsessionNormalizer::~PresentationTimeSubsessionNormalizer()
{
    fParent.removePresentationTimeSubsessionNormalizer(this);
}

void PresentationTimeSubsessionNormalizer::afterGettingFrame(void* clientData, unsigned frameSize,
                                                             unsigned numTruncatedBytes,
                                                             struct timeval presentationTime,
                                                             unsigned durationInMicroseconds)
{
    ((PresentationTimeSubsessionNormalizer *) clientData)
    ->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void PresentationTimeSubsessionNormalizer::afterGettingFrame(unsigned       frameSize,
                                                             unsigned       numTruncatedBytes,
                                                             struct timeval presentationTime,
                                                             unsigned       durationInMicroseconds)
{
    fFrameSize              = frameSize;
    fNumTruncatedBytes      = numTruncatedBytes;
    fDurationInMicroseconds = durationInMicroseconds;
    fParent.normalizePresentationTime(this, fPresentationTime, presentationTime);
    if (fRTPSource->curPacketMarkerBit() && strcmp(fCodecName, "JPEG") == 0) {
        ((SimpleRTPSink *) fRTPSink)->setMBitOnNextPacket();
    }
    FramedSource::afterGetting(this);
}

void PresentationTimeSubsessionNormalizer::doGetNextFrame()
{
    fInputSource->getNextFrame(fTo, fMaxSize, afterGettingFrame, this, FramedSource::handleClosure, this);
}
