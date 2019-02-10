#include "DarwinInjector.hh"
#include <GroupsockHelper.hh>
class SubstreamDescriptor
{
public:
    SubstreamDescriptor(RTPSink* rtpSink, RTCPInstance* rtcpInstance, unsigned trackId);
    ~SubstreamDescriptor();
    SubstreamDescriptor *& next()
    {
        return fNext;
    }

    RTPSink * rtpSink() const
    {
        return fRTPSink;
    }

    RTCPInstance * rtcpInstance() const
    {
        return fRTCPInstance;
    }

    char const * sdpLines() const
    {
        return fSDPLines;
    }

private:
    SubstreamDescriptor* fNext;
    RTPSink* fRTPSink;
    RTCPInstance* fRTCPInstance;
    char* fSDPLines;
};
DarwinInjector * DarwinInjector::createNew(UsageEnvironment& env,
                                           char const*     applicationName,
                                           int             verbosityLevel)
{
    return new DarwinInjector(env, applicationName, verbosityLevel);
}

Boolean DarwinInjector::lookupByName(UsageEnvironment& env, char const* name,
                                     DarwinInjector *& result)
{
    result = NULL;
    Medium* medium;
    if (!Medium::lookupByName(env, name, medium)) {
        return False;
    }
    if (!medium->isDarwinInjector()) {
        env.setResultMsg(name, " is not a 'Darwin injector'");
        return False;
    }
    result = (DarwinInjector *) medium;
    return True;
}

DarwinInjector::DarwinInjector(UsageEnvironment& env,
                               char const* applicationName, int verbosityLevel)
    : Medium(env),
    fApplicationName(strDup(applicationName)), fVerbosityLevel(verbosityLevel),
    fRTSPClient(NULL), fSubstreamSDPSizes(0),
    fHeadSubstream(NULL), fTailSubstream(NULL), fSession(NULL), fLastTrackId(0), fResultString(NULL)
{}

DarwinInjector::~DarwinInjector()
{
    if (fSession != NULL) {
        fRTSPClient->sendTeardownCommand(*fSession, NULL);
        Medium::close(fSession);
    }
    delete fHeadSubstream;
    delete[](char *) fApplicationName;
    Medium::close(fRTSPClient);
}

void DarwinInjector::addStream(RTPSink* rtpSink, RTCPInstance* rtcpInstance)
{
    if (rtpSink == NULL) {
        return;
    }
    SubstreamDescriptor* newDescriptor = new SubstreamDescriptor(rtpSink, rtcpInstance, ++fLastTrackId);
    if (fHeadSubstream == NULL) {
        fHeadSubstream = fTailSubstream = newDescriptor;
    } else {
        fTailSubstream->next() = newDescriptor;
        fTailSubstream         = newDescriptor;
    }
    fSubstreamSDPSizes += strlen(newDescriptor->sdpLines());
}

class RTSPClientForDarwinInjector : public RTSPClient
{
public:
    RTSPClientForDarwinInjector(UsageEnvironment& env, char const* rtspURL, int verbosityLevel,
                                char const* applicationName,
                                DarwinInjector* ourDarwinInjector)
        : RTSPClient(env, rtspURL, verbosityLevel, applicationName, 0, -1),
        fOurDarwinInjector(ourDarwinInjector) {}

    virtual ~RTSPClientForDarwinInjector() {}

    DarwinInjector* fOurDarwinInjector;
};
Boolean DarwinInjector
::setDestination(char const* remoteRTSPServerNameOrAddress,
                 char const* remoteFileName,
                 char const* sessionName,
                 char const* sessionInfo,
                 portNumBits remoteRTSPServerPortNumber,
                 char const* remoteUserName,
                 char const* remotePassword,
                 char const* sessionAuthor,
                 char const* sessionCopyright,
                 int         timeout)
{
    char* sdp       = NULL;
    char* url       = NULL;
    Boolean success = False;

    do {
        char const * const urlFmt = "rtsp://%s:%u/%s";
        unsigned urlLen =
            strlen(urlFmt) + strlen(remoteRTSPServerNameOrAddress) + 5  + strlen(remoteFileName);
        url = new char[urlLen];
        sprintf(url, urlFmt, remoteRTSPServerNameOrAddress, remoteRTSPServerPortNumber, remoteFileName);
        fRTSPClient = new RTSPClientForDarwinInjector(envir(), url, fVerbosityLevel, fApplicationName, this);
        if (fRTSPClient == NULL) {
            break;
        }
        struct in_addr addr;
        {
            NetAddressList addresses(remoteRTSPServerNameOrAddress);
            if (addresses.numAddresses() == 0) {
                break;
            }
            NetAddress const* address = addresses.firstAddress();
            addr.s_addr = *(unsigned *) (address->data());
        }
        AddressString remoteRTSPServerAddressStr(addr);
        char const * const sdpFmt =
            "v=0\r\n"
            "o=- %u %u IN IP4 127.0.0.1\r\n"
            "s=%s\r\n"
            "i=%s\r\n"
            "c=IN IP4 %s\r\n"
            "t=0 0\r\n"
            "a=x-qt-text-nam:%s\r\n"
            "a=x-qt-text-inf:%s\r\n"
            "a=x-qt-text-cmt:source application:%s\r\n"
            "a=x-qt-text-aut:%s\r\n"
            "a=x-qt-text-cpy:%s\r\n";
        unsigned sdpLen = strlen(sdpFmt)
                          + 20  + 20
                          + strlen(sessionName)
                          + strlen(sessionInfo)
                          + strlen(remoteRTSPServerAddressStr.val())
                          + strlen(sessionName)
                          + strlen(sessionInfo)
                          + strlen(fApplicationName)
                          + strlen(sessionAuthor)
                          + strlen(sessionCopyright)
                          + fSubstreamSDPSizes;
        unsigned const sdpSessionId = our_random32();
        unsigned const sdpVersion   = sdpSessionId;
        sdp = new char[sdpLen];
        sprintf(sdp, sdpFmt,
                sdpSessionId, sdpVersion,
                sessionName,
                sessionInfo,
                remoteRTSPServerAddressStr.val(),
                sessionName,
                sessionInfo,
                fApplicationName,
                sessionAuthor,
                sessionCopyright
        );
        char* p = &sdp[strlen(sdp)];
        SubstreamDescriptor* ss;
        for (ss = fHeadSubstream; ss != NULL; ss = ss->next()) {
            sprintf(p, "%s", ss->sdpLines());
            p += strlen(p);
        }
        Authenticator auth;
        Authenticator* authToUse = NULL;
        if (remoteUserName[0] != '\0' || remotePassword[0] != '\0') {
            auth.setUsernameAndPassword(remoteUserName, remotePassword);
            authToUse = &auth;
        }
        fWatchVariable = 0;
        (void) fRTSPClient->sendAnnounceCommand(sdp, genericResponseHandler, authToUse);
        envir().taskScheduler().doEventLoop(&fWatchVariable);
        delete[] fResultString;
        if (fResultCode != 0) {
            break;
        }
        fSession = MediaSession::createNew(envir(), sdp);
        if (fSession == NULL) {
            break;
        }
        ss = fHeadSubstream;
        MediaSubsessionIterator iter(*fSession);
        MediaSubsession* subsession;
        ss = fHeadSubstream;
        unsigned streamChannelId = 0;
        while ((subsession = iter.next()) != NULL) {
            if (!subsession->initiate()) {
                break;
            }
            fWatchVariable = 0;
            (void) fRTSPClient->sendSetupCommand(*subsession, genericResponseHandler,
                                                 True,
                                                 True);
            envir().taskScheduler().doEventLoop(&fWatchVariable);
            delete[] fResultString;
            if (fResultCode != 0) {
                break;
            }
            ss->rtpSink()->setStreamSocket(fRTSPClient->socketNum(), streamChannelId++);
            if (ss->rtcpInstance() != NULL) {
                ss->rtcpInstance()->setStreamSocket(fRTSPClient->socketNum(),
                                                    streamChannelId++);
            }
            ss = ss->next();
        }
        if (subsession != NULL) {
            break;
        }
        fWatchVariable = 0;
        (void) fRTSPClient->sendPlayCommand(*fSession, genericResponseHandler);
        envir().taskScheduler().doEventLoop(&fWatchVariable);
        delete[] fResultString;
        if (fResultCode != 0) {
            break;
        }
        increaseSendBufferTo(envir(), fRTSPClient->socketNum(), 100 * 1024);
        success = True;
    } while (0);
    delete[] sdp;
    delete[] url;
    return success;
} // DarwinInjector::setDestination

Boolean DarwinInjector::isDarwinInjector() const
{
    return True;
}

void DarwinInjector::genericResponseHandler(RTSPClient* rtspClient, int responseCode, char* responseString)
{
    DarwinInjector* di = ((RTSPClientForDarwinInjector *) rtspClient)->fOurDarwinInjector;

    di->genericResponseHandler1(responseCode, responseString);
}

void DarwinInjector::genericResponseHandler1(int responseCode, char* responseString)
{
    fResultCode    = responseCode;
    fResultString  = responseString;
    fWatchVariable = ~0;
}

SubstreamDescriptor::SubstreamDescriptor(RTPSink* rtpSink,
                                         RTCPInstance* rtcpInstance, unsigned trackId)
    : fNext(NULL), fRTPSink(rtpSink), fRTCPInstance(rtcpInstance)
{
    char const* mediaType            = fRTPSink->sdpMediaType();
    unsigned char rtpPayloadType     = fRTPSink->rtpPayloadType();
    char const* rtpPayloadFormatName = fRTPSink->rtpPayloadFormatName();
    unsigned rtpTimestampFrequency   = fRTPSink->rtpTimestampFrequency();
    unsigned numChannels = fRTPSink->numChannels();
    char* rtpmapLine;

    if (rtpPayloadType >= 96) {
        char* encodingParamsPart;
        if (numChannels != 1) {
            encodingParamsPart = new char[1 + 20 ];
            sprintf(encodingParamsPart, "/%d", numChannels);
        } else {
            encodingParamsPart = strDup("");
        }
        char const * const rtpmapFmt = "a=rtpmap:%d %s/%d%s\r\n";
        unsigned rtpmapFmtSize       = strlen(rtpmapFmt)
                                       + 3  + strlen(rtpPayloadFormatName)
                                       + 20  + strlen(encodingParamsPart);
        rtpmapLine = new char[rtpmapFmtSize];
        sprintf(rtpmapLine, rtpmapFmt,
                rtpPayloadType, rtpPayloadFormatName,
                rtpTimestampFrequency, encodingParamsPart);
        delete[] encodingParamsPart;
    } else {
        rtpmapLine = strDup("");
    }
    unsigned rtpmapLineSize = strlen(rtpmapLine);
    char const* auxSDPLine  = fRTPSink->auxSDPLine();
    if (auxSDPLine == NULL) {
        auxSDPLine = "";
    }
    unsigned auxSDPLineSize   = strlen(auxSDPLine);
    char const * const sdpFmt =
        "m=%s 0 RTP/AVP %u\r\n"
        "%s"
        "%s"
        "a=control:trackID=%u\r\n";
    unsigned sdpFmtSize = strlen(sdpFmt)
                          + strlen(mediaType) + 3
                          + rtpmapLineSize
                          + auxSDPLineSize
                          + 20;
    char* sdpLines = new char[sdpFmtSize];
    sprintf(sdpLines, sdpFmt,
            mediaType,
            rtpPayloadType,
            rtpmapLine,
            auxSDPLine,
            trackId);
    fSDPLines = strDup(sdpLines);
    delete[] sdpLines;
    delete[] rtpmapLine;
}

SubstreamDescriptor::~SubstreamDescriptor()
{
    delete fSDPLines;
    delete fNext;
}
