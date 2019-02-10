#include "RTSPClient.hh"
#include "RTSPCommon.hh"
#include "Base64.hh"
#include "Locale.hh"
#include <GroupsockHelper.hh>
#include "ourMD5.hh"
RTSPClient * RTSPClient::createNew(UsageEnvironment& env, char const* rtspURL,
                                   int verbosityLevel,
                                   char const* applicationName,
                                   portNumBits tunnelOverHTTPPortNum,
                                   int socketNumToServer)
{
    return new RTSPClient(env, rtspURL,
                          verbosityLevel, applicationName, tunnelOverHTTPPortNum, socketNumToServer);
}

unsigned RTSPClient::sendDescribeCommand(responseHandler* responseHandler, Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    return sendRequest(new RequestRecord(++fCSeq, "DESCRIBE", responseHandler));
}

unsigned RTSPClient::sendOptionsCommand(responseHandler* responseHandler, Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    return sendRequest(new RequestRecord(++fCSeq, "OPTIONS", responseHandler));
}

unsigned RTSPClient::sendAnnounceCommand(char const* sdpDescription, responseHandler* responseHandler,
                                         Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    return sendRequest(new RequestRecord(++fCSeq, "ANNOUNCE", responseHandler, NULL, NULL, False, 0.0, 0.0, 0.0,
                                         sdpDescription));
}

unsigned RTSPClient::sendSetupCommand(MediaSubsession& subsession, responseHandler* responseHandler,
                                      Boolean streamOutgoing, Boolean streamUsingTCP,
                                      Boolean forceMulticastOnUnspecified,
                                      Authenticator* authenticator)
{
    if (fTunnelOverHTTPPortNum != 0) {
        streamUsingTCP = True;
    }
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    u_int32_t booleanFlags = 0;
    if (streamUsingTCP) {
        booleanFlags |= 0x1;
    }
    if (streamOutgoing) {
        booleanFlags |= 0x2;
    }
    if (forceMulticastOnUnspecified) {
        booleanFlags |= 0x4;
    }
    return sendRequest(new RequestRecord(++fCSeq, "SETUP", responseHandler, NULL, &subsession, booleanFlags));
}

unsigned RTSPClient::sendPlayCommand(MediaSession& session, responseHandler* responseHandler,
                                     double start, double end, float scale,
                                     Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    sendDummyUDPPackets(session);
    return sendRequest(new RequestRecord(++fCSeq, "PLAY", responseHandler, &session, NULL, 0, start, end, scale));
}

unsigned RTSPClient::sendPlayCommand(MediaSubsession& subsession, responseHandler* responseHandler,
                                     double start, double end, float scale,
                                     Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    sendDummyUDPPackets(subsession);
    return sendRequest(new RequestRecord(++fCSeq, "PLAY", responseHandler, NULL, &subsession, 0, start, end, scale));
}

unsigned RTSPClient::sendPlayCommand(MediaSession& session, responseHandler* responseHandler,
                                     char const* absStartTime, char const* absEndTime, float scale,
                                     Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    sendDummyUDPPackets(session);
    return sendRequest(new RequestRecord(++fCSeq, responseHandler, absStartTime, absEndTime, scale, &session, NULL));
}

unsigned RTSPClient::sendPlayCommand(MediaSubsession& subsession, responseHandler* responseHandler,
                                     char const* absStartTime, char const* absEndTime, float scale,
                                     Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    sendDummyUDPPackets(subsession);
    return sendRequest(new RequestRecord(++fCSeq, responseHandler, absStartTime, absEndTime, scale, NULL, &subsession));
}

unsigned RTSPClient::sendPauseCommand(MediaSession& session, responseHandler* responseHandler,
                                      Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    return sendRequest(new RequestRecord(++fCSeq, "PAUSE", responseHandler, &session));
}

unsigned RTSPClient::sendPauseCommand(MediaSubsession& subsession, responseHandler* responseHandler,
                                      Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    return sendRequest(new RequestRecord(++fCSeq, "PAUSE", responseHandler, NULL, &subsession));
}

unsigned RTSPClient::sendRecordCommand(MediaSession& session, responseHandler* responseHandler,
                                       Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    return sendRequest(new RequestRecord(++fCSeq, "RECORD", responseHandler, &session));
}

unsigned RTSPClient::sendRecordCommand(MediaSubsession& subsession, responseHandler* responseHandler,
                                       Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    return sendRequest(new RequestRecord(++fCSeq, "RECORD", responseHandler, NULL, &subsession));
}

unsigned RTSPClient::sendTeardownCommand(MediaSession& session, responseHandler* responseHandler,
                                         Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    return sendRequest(new RequestRecord(++fCSeq, "TEARDOWN", responseHandler, &session));
}

unsigned RTSPClient::sendTeardownCommand(MediaSubsession& subsession, responseHandler* responseHandler,
                                         Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    return sendRequest(new RequestRecord(++fCSeq, "TEARDOWN", responseHandler, NULL, &subsession));
}

unsigned RTSPClient::sendSetParameterCommand(MediaSession& session, responseHandler* responseHandler,
                                             char const* parameterName, char const* parameterValue,
                                             Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    char* paramString = new char[strlen(parameterName) + strlen(parameterValue) + 10];
    sprintf(paramString, "%s: %s\r\n", parameterName, parameterValue);
    unsigned result =
        sendRequest(new RequestRecord(++fCSeq, "SET_PARAMETER", responseHandler, &session, NULL, False, 0.0, 0.0, 0.0,
                                      paramString));
    delete[] paramString;
    return result;
}

unsigned RTSPClient::sendGetParameterCommand(MediaSession& session, responseHandler* responseHandler,
                                             char const* parameterName,
                                             Authenticator* authenticator)
{
    if (authenticator != NULL) {
        fCurrentAuthenticator = *authenticator;
    }
    unsigned parameterNameLen = parameterName == NULL ? 0 : strlen(parameterName);
    char* paramString         = new char[parameterNameLen + 3];
    if (parameterName == NULL) {
        paramString[0] = '\0';
    } else {
        sprintf(paramString, "%s\r\n", parameterName);
    }
    unsigned result =
        sendRequest(new RequestRecord(++fCSeq, "GET_PARAMETER", responseHandler, &session, NULL, False, 0.0, 0.0, 0.0,
                                      paramString));
    delete[] paramString;
    return result;
}

void RTSPClient::sendDummyUDPPackets(MediaSession& session, unsigned numDummyPackets)
{
    MediaSubsessionIterator iter(session);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
        sendDummyUDPPackets(*subsession, numDummyPackets);
    }
}

void RTSPClient::sendDummyUDPPackets(MediaSubsession& subsession, unsigned numDummyPackets)
{
    Groupsock* gs1 = NULL;
    Groupsock* gs2 = NULL;

    if (subsession.rtpSource() != NULL) {
        gs1 = subsession.rtpSource()->RTPgs();
    }
    if (subsession.rtcpInstance() != NULL) {
        gs2 = subsession.rtcpInstance()->RTCPgs();
    }
    u_int32_t const dummy = 0xFEEDFACE;
    for (unsigned i = 0; i < numDummyPackets; ++i) {
        if (gs1 != NULL) {
            gs1->output(envir(), 255, (unsigned char *) &dummy, sizeof dummy);
        }
        if (gs2 != NULL) {
            gs2->output(envir(), 255, (unsigned char *) &dummy, sizeof dummy);
        }
    }
}

Boolean RTSPClient::changeResponseHandler(unsigned cseq, responseHandler* newResponseHandler)
{
    RequestRecord* request;

    if ((request = fRequestsAwaitingConnection.findByCSeq(cseq)) != NULL ||
        (request = fRequestsAwaitingHTTPTunneling.findByCSeq(cseq)) != NULL ||
        (request = fRequestsAwaitingResponse.findByCSeq(cseq)) != NULL)
    {
        request->handler() = newResponseHandler;
        return True;
    }
    return False;
}

Boolean RTSPClient::lookupByName(UsageEnvironment& env,
                                 char const*     instanceName,
                                 RTSPClient *    & resultClient)
{
    resultClient = NULL;
    Medium* medium;
    if (!Medium::lookupByName(env, instanceName, medium)) {
        return False;
    }
    if (!medium->isRTSPClient()) {
        env.setResultMsg(instanceName, " is not a RTSP client");
        return False;
    }
    resultClient = (RTSPClient *) medium;
    return True;
}

Boolean RTSPClient::parseRTSPURL(UsageEnvironment& env, char const* url,
                                 char *& username, char *& password,
                                 NetAddress& address,
                                 portNumBits& portNum,
                                 char const** urlSuffix)
{
    do {
        char const* prefix = "rtsp://";
        unsigned const prefixLength = 7;
        if (_strncasecmp(url, prefix, prefixLength) != 0) {
            env.setResultMsg("URL is not of the form \"", prefix, "\"");
            break;
        }
        unsigned const parseBufferSize = 100;
        char parseBuffer[parseBufferSize];
        char const* from = &url[prefixLength];
        username = password = NULL;
        char const* colonPasswordStart = NULL;
        char const* p;
        for (p = from; *p != '\0' && *p != '/'; ++p) {
            if (*p == ':' && colonPasswordStart == NULL) {
                colonPasswordStart = p;
            } else if (*p == '@') {
                if (colonPasswordStart == NULL) {
                    colonPasswordStart = p;
                }
                char const* usernameStart = from;
                unsigned usernameLen      = colonPasswordStart - usernameStart;
                username = new char[usernameLen + 1];
                for (unsigned i = 0; i < usernameLen; ++i) {
                    username[i] = usernameStart[i];
                }
                username[usernameLen] = '\0';
                char const* passwordStart = colonPasswordStart;
                if (passwordStart < p) {
                    ++passwordStart;
                }
                unsigned passwordLen = p - passwordStart;
                password = new char[passwordLen + 1];
                for (unsigned j = 0; j < passwordLen; ++j) {
                    password[j] = passwordStart[j];
                }
                password[passwordLen] = '\0';
                from = p + 1;
                break;
            }
        }
        char* to = &parseBuffer[0];
        unsigned i;
        for (i = 0; i < parseBufferSize; ++i) {
            if (*from == '\0' || *from == ':' || *from == '/') {
                *to = '\0';
                break;
            }
            *to++ = *from++;
        }
        if (i == parseBufferSize) {
            env.setResultMsg("URL is too long");
            break;
        }
        NetAddressList addresses(parseBuffer);
        if (addresses.numAddresses() == 0) {
            env.setResultMsg("Failed to find network address for \"",
                             parseBuffer, "\"");
            break;
        }
        address = *(addresses.firstAddress());
        portNum = 554;
        char nextChar = *from;
        if (nextChar == ':') {
            int portNumInt;
            if (sscanf(++from, "%d", &portNumInt) != 1) {
                env.setResultMsg("No port number follows ':'");
                break;
            }
            if (portNumInt < 1 || portNumInt > 65535) {
                env.setResultMsg("Bad port number");
                break;
            }
            portNum = (portNumBits) portNumInt;
            while (*from >= '0' && *from <= '9') {
                ++from;
            }
        }
        if (urlSuffix != NULL) {
            *urlSuffix = from;
        }
        return True;
    } while (0);
    return False;
} // RTSPClient::parseRTSPURL

void RTSPClient::setUserAgentString(char const* userAgentName)
{
    if (userAgentName == NULL) {
        return;
    }
    char const * const formatStr = "User-Agent: %s\r\n";
    unsigned const headerSize    = strlen(formatStr) + strlen(userAgentName);
    delete[] fUserAgentHeaderStr;
    fUserAgentHeaderStr = new char[headerSize];
    sprintf(fUserAgentHeaderStr, formatStr, userAgentName);
    fUserAgentHeaderStrLen = strlen(fUserAgentHeaderStr);
}

unsigned RTSPClient::responseBufferSize = 20000;
RTSPClient::RTSPClient(UsageEnvironment& env, char const* rtspURL,
                       int verbosityLevel, char const* applicationName,
                       portNumBits tunnelOverHTTPPortNum, int socketNumToServer)
    : Medium(env),
    fVerbosityLevel(verbosityLevel), fCSeq(1), fServerAddress(0),
    fTunnelOverHTTPPortNum(tunnelOverHTTPPortNum), fUserAgentHeaderStr(NULL), fUserAgentHeaderStrLen(0),
    fInputSocketNum(-1), fOutputSocketNum(-1), fBaseURL(NULL), fTCPStreamIdCount(0),
    fLastSessionId(NULL), fSessionTimeoutParameter(0), fSessionCookieCounter(0),
    fHTTPTunnelingConnectionIsPending(False)
{
    setBaseURL(rtspURL);
    fResponseBuffer = new char[responseBufferSize + 1];
    resetResponseBuffer();
    if (socketNumToServer >= 0) {
        fInputSocketNum = fOutputSocketNum = socketNumToServer;
        envir().taskScheduler().setBackgroundHandling(fInputSocketNum, SOCKET_READABLE | SOCKET_EXCEPTION,
                                                      (TaskScheduler::BackgroundHandlerProc *) &incomingDataHandler,
                                                      this);
    }
    char const * const libName       = "XRtsp Media v";
    char const * const libVersionStr = LIVEMEDIA_LIBRARY_VERSION_STRING;
    char const* libPrefix;
    char const* libSuffix;
    if (applicationName == NULL || applicationName[0] == '\0') {
        applicationName = libPrefix = libSuffix = "";
    } else {
        libPrefix = " (";
        libSuffix = ")";
    }
    unsigned userAgentNameSize =
        strlen(applicationName) + strlen(libPrefix) + strlen(libName) + strlen(libVersionStr) + strlen(libSuffix) + 1;
    char* userAgentName = new char[userAgentNameSize];
    sprintf(userAgentName, "%s%s%s%s%s", applicationName, libPrefix, libName, libVersionStr, libSuffix);
    setUserAgentString(userAgentName);
    delete[] userAgentName;
}

RTSPClient::~RTSPClient()
{
    RTPInterface::clearServerRequestAlternativeByteHandler(envir(), fInputSocketNum);
    reset();
    delete[] fResponseBuffer;
    delete[] fUserAgentHeaderStr;
}

void RTSPClient::reset()
{
    resetTCPSockets();
    resetResponseBuffer();
    fServerAddress = 0;
    setBaseURL(NULL);
    fCurrentAuthenticator.reset();
    delete[] fLastSessionId;
    fLastSessionId = NULL;
}

void RTSPClient::setBaseURL(char const* url)
{
    delete[] fBaseURL;
    fBaseURL = strDup(url);
}

int RTSPClient::grabSocket()
{
    int inputSocket = fInputSocketNum;

    fInputSocketNum = -1;
    return inputSocket;
}

unsigned RTSPClient::sendRequest(RequestRecord* request)
{
    char* cmd = NULL;

    do {
        Boolean connectionIsPending = False;
        if (!fRequestsAwaitingConnection.isEmpty()) {
            connectionIsPending = True;
        } else if (fInputSocketNum < 0) {
            int connectResult = openConnection();
            if (connectResult < 0) {
                break;
            } else if (connectResult == 0) {
                connectionIsPending = True;
            }
        }
        if (connectionIsPending) {
            fRequestsAwaitingConnection.enqueue(request);
            return request->cseq();
        }
        if (fTunnelOverHTTPPortNum != 0 &&
            strcmp(request->commandName(), "GET") != 0 && fOutputSocketNum == fInputSocketNum)
        {
            if (!setupHTTPTunneling1()) {
                break;
            }
            fRequestsAwaitingHTTPTunneling.enqueue(request);
            return request->cseq();
        }
        char* cmdURL = fBaseURL;
        Boolean cmdURLWasAllocated              = False;
        char const* protocolStr                 = "RTSP/1.0";
        char* extraHeaders                      = (char *) "";
        Boolean extraHeadersWereAllocated       = False;
        char* contentLengthHeader               = (char *) "";
        Boolean contentLengthHeaderWasAllocated = False;
        if (!setRequestFields(request,
                              cmdURL, cmdURLWasAllocated,
                              protocolStr,
                              extraHeaders, extraHeadersWereAllocated))
        {
            break;
        }
        char const* contentStr = request->contentStr();
        if (contentStr == NULL) {
            contentStr = "";
        }
        unsigned contentStrLen = strlen(contentStr);
        if (contentStrLen > 0) {
            char const* contentLengthHeaderFmt =
                "Content-Length: %d\r\n";
            unsigned contentLengthHeaderSize = strlen(contentLengthHeaderFmt)
                                               + 20;
            contentLengthHeader = new char[contentLengthHeaderSize];
            sprintf(contentLengthHeader, contentLengthHeaderFmt, contentStrLen);
            contentLengthHeaderWasAllocated = True;
        }
        char* authenticatorStr    = createAuthenticatorString(request->commandName(), fBaseURL);
        char const * const cmdFmt =
            "%s %s %s\r\n"
            "CSeq: %d\r\n"
            "%s"
            "%s"
            "%s"
            "%s"
            "\r\n"
            "%s";
        unsigned cmdSize = strlen(cmdFmt)
                           + strlen(request->commandName()) + strlen(cmdURL) + strlen(protocolStr)
                           + 20
                           + strlen(authenticatorStr)
                           + fUserAgentHeaderStrLen
                           + strlen(extraHeaders)
                           + strlen(contentLengthHeader)
                           + contentStrLen;
        cmd = new char[cmdSize];
        sprintf(cmd, cmdFmt,
                request->commandName(), cmdURL, protocolStr,
                request->cseq(),
                authenticatorStr,
                fUserAgentHeaderStr,
                extraHeaders,
                contentLengthHeader,
                contentStr);
        delete[] authenticatorStr;
        if (cmdURLWasAllocated) {
            delete[] cmdURL;
        }
        if (extraHeadersWereAllocated) {
            delete[] extraHeaders;
        }
        if (contentLengthHeaderWasAllocated) {
            delete[] contentLengthHeader;
        }
        if (fVerbosityLevel >= 1) {
            envir() << "Sending request: " << cmd << "\n";
        }
        if (fTunnelOverHTTPPortNum != 0 &&
            strcmp(request->commandName(), "GET") != 0 && strcmp(request->commandName(), "POST") != 0)
        {
            char* origCmd = cmd;
            cmd = base64Encode(origCmd, strlen(cmd));
            if (fVerbosityLevel >= 1) {
                envir() << "\tThe request was base-64 encoded to: " << cmd << "\n\n";
            }
            delete[] origCmd;
        }
        if (send(fOutputSocketNum, cmd, strlen(cmd), 0) < 0) {
            char const* errFmt       = "%s send() failed: ";
            unsigned const errLength = strlen(errFmt) + strlen(request->commandName());
            char* err = new char[errLength];
            sprintf(err, errFmt, request->commandName());
            envir().setResultErrMsg(err);
            delete[] err;
            break;
        }
        int cseq = request->cseq();
        if (fTunnelOverHTTPPortNum == 0 || strcmp(request->commandName(), "POST") != 0) {
            fRequestsAwaitingResponse.enqueue(request);
        } else {
            delete request;
        }
        delete[] cmd;
        return cseq;
    } while (0);
    delete[] cmd;
    handleRequestError(request);
    delete request;
    return 0;
} // RTSPClient::sendRequest

static char * createSessionString(char const* sessionId)
{
    char* sessionStr;

    if (sessionId != NULL) {
        sessionStr = new char[20 + strlen(sessionId)];
        sprintf(sessionStr, "Session: %s\r\n", sessionId);
    } else {
        sessionStr = strDup("");
    }
    return sessionStr;
}

static char * createScaleString(float scale, float currentScale)
{
    char buf[100];

    if (scale == 1.0f && currentScale == 1.0f) {
        buf[0] = '\0';
    } else {
        Locale l("C", Numeric);
        sprintf(buf, "Scale: %f\r\n", scale);
    }
    return strDup(buf);
}

static char * createRangeString(double start, double end, char const* absStartTime, char const* absEndTime)
{
    char buf[100];

    if (absStartTime != NULL) {
        if (absEndTime == NULL) {
            snprintf(buf, sizeof buf, "Range: clock=%s-\r\n", absStartTime);
        } else {
            snprintf(buf, sizeof buf, "Range: clock=%s-%s\r\n", absStartTime, absEndTime);
        }
    } else {
        if (start < 0) {
            buf[0] = '\0';
        } else if (end < 0) {
            Locale l("C", Numeric);
            sprintf(buf, "Range: npt=%.3f-\r\n", start);
        } else {
            Locale l("C", Numeric);
            sprintf(buf, "Range: npt=%.3f-%.3f\r\n", start, end);
        }
    }
    return strDup(buf);
}

Boolean RTSPClient::setRequestFields(RequestRecord* request,
                                     char *& cmdURL, Boolean& cmdURLWasAllocated,
                                     char const *& protocolStr,
                                     char *& extraHeaders, Boolean& extraHeadersWereAllocated
)
{
    if (strcmp(request->commandName(), "DESCRIBE") == 0) {
        extraHeaders = (char *) "Accept: application/sdp\r\n";
    } else if (strcmp(request->commandName(), "OPTIONS") == 0) {
        extraHeaders = createSessionString(fLastSessionId);
        extraHeadersWereAllocated = True;
    } else if (strcmp(request->commandName(), "ANNOUNCE") == 0) {
        extraHeaders = (char *) "Content-Type: application/sdp\r\n";
    } else if (strcmp(request->commandName(), "SETUP") == 0) {
        MediaSubsession& subsession = *request->subsession();
        Boolean streamUsingTCP = (request->booleanFlags() & 0x1) != 0;
        Boolean streamOutgoing = (request->booleanFlags() & 0x2) != 0;
        Boolean forceMulticastOnUnspecified = (request->booleanFlags() & 0x4) != 0;
        char const* prefix, * separator, * suffix;
        constructSubsessionURL(subsession, prefix, separator, suffix);
        char const* transportFmt;
        if (strcmp(subsession.protocolName(), "UDP") == 0) {
            suffix       = "";
            transportFmt = "Transport: RAW/RAW/UDP%s%s%s=%d-%d\r\n";
        } else {
            transportFmt = "Transport: RTP/AVP%s%s%s=%d-%d\r\n";
        }
        cmdURL = new char[strlen(prefix) + strlen(separator) + strlen(suffix) + 1];
        cmdURLWasAllocated = True;
        sprintf(cmdURL, "%s%s%s", prefix, separator, suffix);
        char const* transportTypeStr;
        char const* modeStr = streamOutgoing ? ";mode=receive" : "";
        char const* portTypeStr;
        portNumBits rtpNumber, rtcpNumber;
        if (streamUsingTCP) {
            transportTypeStr = "/TCP;unicast";
            portTypeStr      = ";interleaved";
            rtpNumber        = fTCPStreamIdCount++;
            rtcpNumber       = fTCPStreamIdCount++;
        } else {
            unsigned connectionAddress        = subsession.connectionEndpointAddress();
            Boolean requestMulticastStreaming =
                IsMulticastAddress(connectionAddress) || (connectionAddress == 0 && forceMulticastOnUnspecified);
            transportTypeStr = requestMulticastStreaming ? ";multicast" : ";unicast";
            portTypeStr      = ";client_port";
            rtpNumber        = subsession.clientPortNum();
            if (rtpNumber == 0) {
                envir().setResultMsg("Client port number unknown\n");
                delete[] cmdURL;
                return False;
            }
            rtcpNumber = subsession.rtcpIsMuxed() ? rtpNumber : rtpNumber + 1;
        }
        unsigned transportSize = strlen(transportFmt)
                                 + strlen(transportTypeStr) + strlen(modeStr) + strlen(portTypeStr) + 2 * 5;
        char* transportStr = new char[transportSize];
        sprintf(transportStr, transportFmt,
                transportTypeStr, modeStr, portTypeStr, rtpNumber, rtcpNumber);
        char* sessionStr = createSessionString(fLastSessionId);
        extraHeaders = new char[transportSize + strlen(sessionStr)];
        extraHeadersWereAllocated = True;
        sprintf(extraHeaders, "%s%s", transportStr, sessionStr);
        delete[] transportStr;
        delete[] sessionStr;
    } else if (strcmp(request->commandName(), "GET") == 0 || strcmp(request->commandName(), "POST") == 0) {
        char* username;
        char* password;
        NetAddress destAddress;
        portNumBits urlPortNum;
        if (!parseRTSPURL(envir(), fBaseURL, username, password, destAddress, urlPortNum, (char const **) &cmdURL)) {
            return False;
        }
        if (cmdURL[0] == '\0') {
            cmdURL = (char *) "/";
        }
        delete[] username;
        delete[] password;
        netAddressBits serverAddress = *(netAddressBits *) (destAddress.data());
        AddressString serverAddressString(serverAddress);
        protocolStr = "HTTP/1.1";
        if (strcmp(request->commandName(), "GET") == 0) {
            struct {
                struct timeval timestamp;
                unsigned       counter;
            } seedData;
            gettimeofday(&seedData.timestamp, NULL);
            seedData.counter = ++fSessionCookieCounter;
            our_MD5Data((unsigned char *) (&seedData), sizeof seedData, fSessionCookie);
            fSessionCookie[23] = '\0';
            char const * const extraHeadersFmt =
                "Host: %s\r\n"
                "x-sessioncookie: %s\r\n"
                "Accept: application/x-rtsp-tunnelled\r\n"
                "Pragma: no-cache\r\n"
                "Cache-Control: no-cache\r\n";
            unsigned extraHeadersSize = strlen(extraHeadersFmt)
                                        + strlen(serverAddressString.val())
                                        + strlen(fSessionCookie);
            extraHeaders = new char[extraHeadersSize];
            extraHeadersWereAllocated = True;
            sprintf(extraHeaders, extraHeadersFmt,
                    serverAddressString.val(),
                    fSessionCookie);
        } else {
            char const * const extraHeadersFmt =
                "Host: %s\r\n"
                "x-sessioncookie: %s\r\n"
                "Content-Type: application/x-rtsp-tunnelled\r\n"
                "Pragma: no-cache\r\n"
                "Cache-Control: no-cache\r\n"
                "Content-Length: 32767\r\n"
                "Expires: Sun, 9 Jan 1972 00:00:00 GMT\r\n";
            unsigned extraHeadersSize = strlen(extraHeadersFmt)
                                        + strlen(serverAddressString.val())
                                        + strlen(fSessionCookie);
            extraHeaders = new char[extraHeadersSize];
            extraHeadersWereAllocated = True;
            sprintf(extraHeaders, extraHeadersFmt,
                    serverAddressString.val(),
                    fSessionCookie);
        }
    } else {
        if (fLastSessionId == NULL) {
            envir().setResultMsg("No RTSP session is currently in progress\n");
            return False;
        }
        char const* sessionId;
        float originalScale;
        if (request->session() != NULL) {
            cmdURL        = (char *) sessionURL(*request->session());
            sessionId     = fLastSessionId;
            originalScale = request->session()->scale();
        } else {
            char const* prefix, * separator, * suffix;
            constructSubsessionURL(*request->subsession(), prefix, separator, suffix);
            cmdURL = new char[strlen(prefix) + strlen(separator) + strlen(suffix) + 1];
            cmdURLWasAllocated = True;
            sprintf(cmdURL, "%s%s%s", prefix, separator, suffix);
            sessionId     = request->subsession()->sessionId();
            originalScale = request->subsession()->scale();
        }
        if (strcmp(request->commandName(), "PLAY") == 0) {
            char* sessionStr = createSessionString(sessionId);
            char* scaleStr   = createScaleString(request->scale(), originalScale);
            char* rangeStr   = createRangeString(request->start(), request->end(),
                                                 request->absStartTime(), request->absEndTime());
            extraHeaders = new char[strlen(sessionStr) + strlen(scaleStr) + strlen(rangeStr) + 1];
            extraHeadersWereAllocated = True;
            sprintf(extraHeaders, "%s%s%s", sessionStr, scaleStr, rangeStr);
            delete[] sessionStr;
            delete[] scaleStr;
            delete[] rangeStr;
        } else {
            extraHeaders = createSessionString(sessionId);
            extraHeadersWereAllocated = True;
        }
    }
    return True;
} // RTSPClient::setRequestFields

Boolean RTSPClient::isRTSPClient() const
{
    return True;
}

void RTSPClient::resetTCPSockets()
{
    if (fInputSocketNum >= 0) {
        envir().taskScheduler().disableBackgroundHandling(fInputSocketNum);
        ::closeSocket(fInputSocketNum);
        if (fOutputSocketNum != fInputSocketNum) {
            envir().taskScheduler().disableBackgroundHandling(fOutputSocketNum);
            ::closeSocket(fOutputSocketNum);
        }
    }
    fInputSocketNum = fOutputSocketNum = -1;
}

void RTSPClient::resetResponseBuffer()
{
    fResponseBytesAlreadySeen = 0;
    fResponseBufferBytesLeft  = responseBufferSize;
}

int RTSPClient::openConnection()
{
    do {
        char* username;
        char* password;
        NetAddress destAddress;
        portNumBits urlPortNum;
        char const* urlSuffix;
        if (!parseRTSPURL(envir(), fBaseURL, username, password, destAddress, urlPortNum, &urlSuffix)) {
            break;
        }
        portNumBits destPortNum = fTunnelOverHTTPPortNum == 0 ? urlPortNum : fTunnelOverHTTPPortNum;
        if (username != NULL || password != NULL) {
            fCurrentAuthenticator.setUsernameAndPassword(username, password);
            delete[] username;
            delete[] password;
        }
        fInputSocketNum = fOutputSocketNum = setupStreamSocket(envir(), 0);
        if (fInputSocketNum < 0) {
            break;
        }
        ignoreSigPipeOnSocket(fInputSocketNum);
        fServerAddress = *(netAddressBits *) (destAddress.data());
        int connectResult = connectToServer(fInputSocketNum, destPortNum);
        if (connectResult < 0) {
            break;
        } else if (connectResult > 0) {
            envir().taskScheduler().setBackgroundHandling(fInputSocketNum, SOCKET_READABLE | SOCKET_EXCEPTION,
                                                          (TaskScheduler::BackgroundHandlerProc *) &incomingDataHandler,
                                                          this);
        }
        return connectResult;
    } while (0);
    resetTCPSockets();
    return -1;
} // RTSPClient::openConnection

int RTSPClient::connectToServer(int socketNum, portNumBits remotePortNum)
{
    MAKE_SOCKADDR_IN(remoteName, fServerAddress, htons(remotePortNum));
    if (fVerbosityLevel >= 1) {
        envir() << "Opening connection to " << AddressString(remoteName).val() << ", port " << remotePortNum << "...\n";
    }
    if (connect(socketNum, (struct sockaddr *) &remoteName, sizeof remoteName) != 0) {
        int const err = envir().getErrno();
        if (err == EINPROGRESS || err == EWOULDBLOCK) {
            envir().taskScheduler().setBackgroundHandling(socketNum, SOCKET_WRITABLE | SOCKET_EXCEPTION,
                                                          (TaskScheduler::BackgroundHandlerProc *) &connectionHandler,
                                                          this);
            return 0;
        }
        envir().setResultErrMsg("connect() failed: ");
        if (fVerbosityLevel >= 1) {
            envir() << "..." << envir().getResultMsg() << "\n";
        }
        return -1;
    }
    if (fVerbosityLevel >= 1) {
        envir() << "...local connection opened\n";
    }
    return 1;
}

char * RTSPClient::createAuthenticatorString(char const* cmd, char const* url)
{
    Authenticator& auth = fCurrentAuthenticator;

    if (auth.realm() != NULL && auth.username() != NULL && auth.password() != NULL) {
        char* authenticatorStr;
        if (auth.nonce() != NULL) {
            char const * const authFmt =
                "Authorization: Digest username=\"%s\", realm=\"%s\", "
                "nonce=\"%s\", uri=\"%s\", response=\"%s\"\r\n";
            char const* response = auth.computeDigestResponse(cmd, url);
            unsigned authBufSize = strlen(authFmt)
                                   + strlen(auth.username()) + strlen(auth.realm())
                                   + strlen(auth.nonce()) + strlen(url) + strlen(response);
            authenticatorStr = new char[authBufSize];
            sprintf(authenticatorStr, authFmt,
                    auth.username(), auth.realm(),
                    auth.nonce(), url, response);
            auth.reclaimDigestResponse(response);
        } else {
            char const * const authFmt      = "Authorization: Basic %s\r\n";
            unsigned usernamePasswordLength = strlen(auth.username()) + 1 + strlen(auth.password());
            char* usernamePassword = new char[usernamePasswordLength + 1];
            sprintf(usernamePassword, "%s:%s", auth.username(), auth.password());
            char* response = base64Encode(usernamePassword, usernamePasswordLength);
            unsigned const authBufSize = strlen(authFmt) + strlen(response) + 1;
            authenticatorStr = new char[authBufSize];
            sprintf(authenticatorStr, authFmt, response);
            delete[] response;
            delete[] usernamePassword;
        }
        return authenticatorStr;
    }
    return strDup("");
} // RTSPClient::createAuthenticatorString

void RTSPClient::handleRequestError(RequestRecord* request)
{
    int resultCode = -envir().getErrno();

    if (resultCode == 0) {
        #if defined(__WIN32__) || defined(_WIN32) || defined(_QNX4)
        resultCode = -WSAENOTCONN;
        #else
        resultCode = -ENOTCONN;
        #endif
    }
    if (request->handler() != NULL) {
        (*request->handler())(this, resultCode, strDup(envir().getResultMsg()));
    }
}

Boolean RTSPClient
::parseResponseCode(char const* line, unsigned& responseCode, char const *& responseString)
{
    if (sscanf(line, "RTSP/%*s%u", &responseCode) != 1 &&
        sscanf(line, "HTTP/%*s%u", &responseCode) != 1)
    {
        return False;
    }
    responseString = line;
    while (responseString[0] != '\0' && responseString[0] != ' ' && responseString[0] != '\t') {
        ++responseString;
    }
    while (responseString[0] != '\0' && (responseString[0] == ' ' || responseString[0] == '\t')) {
        ++responseString;
    }
    return True;
}

void RTSPClient::handleIncomingRequest()
{
    char cmdName[RTSP_PARAM_STRING_MAX];
    char urlPreSuffix[RTSP_PARAM_STRING_MAX];
    char urlSuffix[RTSP_PARAM_STRING_MAX];
    char cseq[RTSP_PARAM_STRING_MAX];
    char sessionId[RTSP_PARAM_STRING_MAX];
    unsigned contentLength;

    if (!parseRTSPRequestString(fResponseBuffer, fResponseBytesAlreadySeen,
                                cmdName, sizeof cmdName,
                                urlPreSuffix, sizeof urlPreSuffix,
                                urlSuffix, sizeof urlSuffix,
                                cseq, sizeof cseq,
                                sessionId, sizeof sessionId,
                                contentLength))
    {
        return;
    } else {
        if (fVerbosityLevel >= 1) {
            envir() << "Received incoming RTSP request: " << fResponseBuffer << "\n";
        }
        char tmpBuf[2 * RTSP_PARAM_STRING_MAX];
        snprintf((char *) tmpBuf, sizeof tmpBuf,
                 "RTSP/1.0 405 Method Not Allowed\r\nCSeq: %s\r\n\r\n", cseq);
        send(fOutputSocketNum, tmpBuf, strlen(tmpBuf), 0);
    }
}

Boolean RTSPClient::checkForHeader(char const* line, char const* headerName, unsigned headerNameLength,
                                   char const *& headerParams)
{
    if (_strncasecmp(line, headerName, headerNameLength) != 0) {
        return False;
    }
    unsigned paramIndex = headerNameLength;
    while (line[paramIndex] != '\0' && (line[paramIndex] == ' ' || line[paramIndex] == '\t')) {
        ++paramIndex;
    }
    if (line[paramIndex] == '\0') {
        return False;
    }
    headerParams = &line[paramIndex];
    return True;
}

Boolean RTSPClient::parseTransportParams(char const* paramsStr,
                                         char *& serverAddressStr, portNumBits& serverPortNum,
                                         unsigned char& rtpChannelId, unsigned char& rtcpChannelId)
{
    serverAddressStr = NULL;
    serverPortNum    = 0;
    rtpChannelId     = rtcpChannelId = 0xFF;
    if (paramsStr == NULL) {
        return False;
    }
    char* foundServerAddressStr = NULL;
    Boolean foundServerPortNum = False;
    portNumBits clientPortNum = 0;
    Boolean foundClientPortNum = False;
    Boolean foundChannelIds = False;
    unsigned rtpCid, rtcpCid;
    Boolean isMulticast = True;
    char* foundDestinationStr = NULL;
    portNumBits multicastPortNumRTP, multicastPortNumRTCP;
    Boolean foundMulticastPortNum = False;
    char const* fields = paramsStr;
    char* field        = strDupSize(fields);
    while (sscanf(fields, "%[^;]", field) == 1) {
        if (sscanf(field, "server_port=%hu", &serverPortNum) == 1) {
            foundServerPortNum = True;
        } else if (sscanf(field, "client_port=%hu", &clientPortNum) == 1) {
            foundClientPortNum = True;
        } else if (_strncasecmp(field, "source=", 7) == 0) {
            delete[] foundServerAddressStr;
            foundServerAddressStr = strDup(field + 7);
        } else if (sscanf(field, "interleaved=%u-%u", &rtpCid, &rtcpCid) == 2) {
            rtpChannelId    = (unsigned char) rtpCid;
            rtcpChannelId   = (unsigned char) rtcpCid;
            foundChannelIds = True;
        } else if (strcmp(field, "unicast") == 0) {
            isMulticast = False;
        } else if (_strncasecmp(field, "destination=", 12) == 0) {
            delete[] foundDestinationStr;
            foundDestinationStr = strDup(field + 12);
        } else if (sscanf(field, "port=%hu-%hu", &multicastPortNumRTP, &multicastPortNumRTCP) == 2 ||
                   sscanf(field, "port=%hu", &multicastPortNumRTP) == 1)
        {
            foundMulticastPortNum = True;
        }
        fields += strlen(field);
        while (fields[0] == ';') {
            ++fields;
        }
        if (fields[0] == '\0') {
            break;
        }
    }
    delete[] field;
    if (isMulticast && foundDestinationStr != NULL && foundMulticastPortNum) {
        delete[] foundServerAddressStr;
        serverAddressStr = foundDestinationStr;
        serverPortNum    = multicastPortNumRTP;
        return True;
    }
    delete[] foundDestinationStr;
    if (foundChannelIds || foundServerPortNum || foundClientPortNum) {
        if (foundClientPortNum && !foundServerPortNum) {
            serverPortNum = clientPortNum;
        }
        serverAddressStr = foundServerAddressStr;
        return True;
    }
    delete[] foundServerAddressStr;
    return False;
} // RTSPClient::parseTransportParams

Boolean RTSPClient::parseScaleParam(char const* paramStr, float& scale)
{
    Locale l("C", Numeric);

    return sscanf(paramStr, "%f", &scale) == 1;
}

Boolean RTSPClient::parseRTPInfoParams(char const *& paramsStr, u_int16_t& seqNum, u_int32_t& timestamp)
{
    if (paramsStr == NULL || paramsStr[0] == '\0') {
        return False;
    }
    while (paramsStr[0] == ',') {
        ++paramsStr;
    }
    char* field = strDupSize(paramsStr);
    Boolean sawSeq = False, sawRtptime = False;
    while (sscanf(paramsStr, "%[^;,]", field) == 1) {
        if (sscanf(field, "seq=%hu", &seqNum) == 1) {
            sawSeq = True;
        } else if (sscanf(field, "rtptime=%u", &timestamp) == 1) {
            sawRtptime = True;
        }
        paramsStr += strlen(field);
        if (paramsStr[0] == '\0' || paramsStr[0] == ',') {
            break;
        }
        ++paramsStr;
    }
    delete[] field;
    return sawSeq && sawRtptime;
}

Boolean RTSPClient::handleSETUPResponse(MediaSubsession& subsession, char const* sessionParamsStr,
                                        char const* transportParamsStr,
                                        Boolean streamUsingTCP)
{
    char* sessionId = new char[responseBufferSize];
    Boolean success = False;

    do {
        if (sessionParamsStr == NULL || sscanf(sessionParamsStr, "%[^;]", sessionId) != 1) {
            envir().setResultMsg("Missing or bad \"Session:\" header");
            break;
        }
        subsession.setSessionId(sessionId);
        delete[] fLastSessionId;
        fLastSessionId = strDup(sessionId);
        char const* afterSessionId = sessionParamsStr + strlen(sessionId);
        int timeoutVal;
        if (sscanf(afterSessionId, "; timeout = %d", &timeoutVal) == 1) {
            fSessionTimeoutParameter = timeoutVal;
        }
        char* serverAddressStr;
        portNumBits serverPortNum;
        unsigned char rtpChannelId, rtcpChannelId;
        if (!parseTransportParams(transportParamsStr, serverAddressStr, serverPortNum, rtpChannelId, rtcpChannelId)) {
            envir().setResultMsg("Missing or bad \"Transport:\" header");
            break;
        }
        delete[] subsession.connectionEndpointName();
        subsession.connectionEndpointName() = serverAddressStr;
        subsession.serverPortNum = serverPortNum;
        subsession.rtpChannelId  = rtpChannelId;
        subsession.rtcpChannelId = rtcpChannelId;
        if (streamUsingTCP) {
            if (subsession.rtpSource() != NULL) {
                subsession.rtpSource()->setStreamSocket(fInputSocketNum, subsession.rtpChannelId);
                subsession.rtpSource()->enableRTCPReports() = False;
            }
            if (subsession.rtcpInstance() != NULL) {
                subsession.rtcpInstance()->setStreamSocket(fInputSocketNum, subsession.rtcpChannelId);
            }
            RTPInterface::setServerRequestAlternativeByteHandler(
                envir(), fInputSocketNum, handleAlternativeRequestByte, this);
        } else {
            netAddressBits destAddress = subsession.connectionEndpointAddress();
            if (destAddress == 0) {
                destAddress = fServerAddress;
            }
            subsession.setDestinations(destAddress);
        }
        success = True;
    } while (0);
    delete[] sessionId;
    return success;
} // RTSPClient::handleSETUPResponse

Boolean RTSPClient::handlePLAYResponse(MediaSession& session, MediaSubsession& subsession,
                                       char const* scaleParamsStr, char const* rangeParamsStr,
                                       char const* rtpInfoParamsStr)
{
    Boolean scaleOK = False, rangeOK = False;

    do {
        if (&session != NULL) {
            if (scaleParamsStr != NULL && !parseScaleParam(scaleParamsStr, session.scale())) {
                break;
            }
            scaleOK = True;
            Boolean startTimeIsNow;
            if (rangeParamsStr != NULL &&
                !parseRangeParam(rangeParamsStr,
                                 session.playStartTime(), session.playEndTime(),
                                 session._absStartTime(), session._absEndTime(),
                                 startTimeIsNow))
            {
                break;
            }
            rangeOK = True;
            MediaSubsessionIterator iter(session);
            MediaSubsession* subsession;
            while ((subsession = iter.next()) != NULL) {
                u_int16_t seqNum;
                u_int32_t timestamp;
                subsession->rtpInfo.infoIsNew = False;
                if (parseRTPInfoParams(rtpInfoParamsStr, seqNum, timestamp)) {
                    subsession->rtpInfo.seqNum    = seqNum;
                    subsession->rtpInfo.timestamp = timestamp;
                    subsession->rtpInfo.infoIsNew = True;
                }
                if (subsession->rtpSource() != NULL) {
                    subsession->rtpSource()->enableRTCPReports() = True;
                }
            }
        } else {
            if (scaleParamsStr != NULL && !parseScaleParam(scaleParamsStr, subsession.scale())) {
                break;
            }
            scaleOK = True;
            Boolean startTimeIsNow;
            if (rangeParamsStr != NULL &&
                !parseRangeParam(rangeParamsStr,
                                 subsession._playStartTime(), subsession._playEndTime(),
                                 subsession._absStartTime(), subsession._absEndTime(),
                                 startTimeIsNow))
            {
                break;
            }
            rangeOK = True;
            u_int16_t seqNum;
            u_int32_t timestamp;
            subsession.rtpInfo.infoIsNew = False;
            if (parseRTPInfoParams(rtpInfoParamsStr, seqNum, timestamp)) {
                subsession.rtpInfo.seqNum    = seqNum;
                subsession.rtpInfo.timestamp = timestamp;
                subsession.rtpInfo.infoIsNew = True;
            }
            if (subsession.rtpSource() != NULL) {
                subsession.rtpSource()->enableRTCPReports() = True;
            }
        }
        return True;
    } while (0);
    if (!scaleOK) {
        envir().setResultMsg("Bad \"Scale:\" header");
    } else if (!rangeOK) {
        envir().setResultMsg("Bad \"Range:\" header");
    } else {
        envir().setResultMsg("Bad \"RTP-Info:\" header");
    }
    return False;
} // RTSPClient::handlePLAYResponse

Boolean RTSPClient::handleTEARDOWNResponse(MediaSession&, MediaSubsession&)
{
    return True;
}

Boolean RTSPClient::handleGET_PARAMETERResponse(char const* parameterName, char *& resultValueString)
{
    do {
        if (parameterName != NULL && parameterName[0] != '\0') {
            if (parameterName[1] == '\0') {
                break;
            }
            unsigned parameterNameLen = strlen(parameterName);
            parameterNameLen -= 2;
            if (_strncasecmp(resultValueString, parameterName, parameterNameLen) == 0) {
                resultValueString += parameterNameLen;
                if (resultValueString[0] == ':') {
                    ++resultValueString;
                }
                while (resultValueString[0] == ' ' || resultValueString[0] == '\t') {
                    ++resultValueString;
                }
            }
        }
        unsigned resultLen = strlen(resultValueString);
        while (resultLen > 0 &&
               (resultValueString[resultLen - 1] == '\r' || resultValueString[resultLen - 1] == '\n'))
        {
            --resultLen;
        }
        resultValueString[resultLen] = '\0';
        return True;
    } while (0);
    envir().setResultMsg("Bad \"GET_PARAMETER\" response");
    return False;
}

Boolean RTSPClient::handleAuthenticationFailure(char const* paramsStr)
{
    if (paramsStr == NULL) {
        return False;
    }
    Boolean alreadyHadRealm = fCurrentAuthenticator.realm() != NULL;
    char* realm     = strDupSize(paramsStr);
    char* nonce     = strDupSize(paramsStr);
    Boolean success = True;
    if (sscanf(paramsStr, "Digest realm=\"%[^\"]\", nonce=\"%[^\"]\"", realm, nonce) == 2) {
        fCurrentAuthenticator.setRealmAndNonce(realm, nonce);
    } else if (sscanf(paramsStr, "Basic realm=\"%[^\"]\"", realm) == 1) {
        fCurrentAuthenticator.setRealmAndNonce(realm, NULL);
    } else {
        success = False;
    }
    delete[] realm;
    delete[] nonce;
    if (alreadyHadRealm || fCurrentAuthenticator.username() == NULL || fCurrentAuthenticator.password() == NULL) {
        success = False;
    }
    return success;
}

Boolean RTSPClient::resendCommand(RequestRecord* request)
{
    if (fVerbosityLevel >= 1) {
        envir() << "Resending...\n";
    }
    if (request != NULL && strcmp(request->commandName(), "GET") != 0) {
        request->cseq() = ++fCSeq;
    }
    return sendRequest(request) != 0;
}

char const * RTSPClient::sessionURL(MediaSession const& session) const
{
    char const* url = session.controlPath();

    if (url == NULL || strcmp(url, "*") == 0) {
        url = fBaseURL;
    }
    return url;
}

void RTSPClient::handleAlternativeRequestByte(void* rtspClient, u_int8_t requestByte)
{
    ((RTSPClient *) rtspClient)->handleAlternativeRequestByte1(requestByte);
}

void RTSPClient::handleAlternativeRequestByte1(u_int8_t requestByte)
{
    if (requestByte == 0xFF) {
        handleResponseBytes(-1);
    } else if (requestByte == 0xFE) {
        envir().taskScheduler().setBackgroundHandling(fInputSocketNum, SOCKET_READABLE | SOCKET_EXCEPTION,
                                                      (TaskScheduler::BackgroundHandlerProc *) &incomingDataHandler,
                                                      this);
    } else {
        fResponseBuffer[fResponseBytesAlreadySeen] = requestByte;
        handleResponseBytes(1);
    }
}

static Boolean isAbsoluteURL(char const* url)
{
    while (*url != '\0' && *url != '/') {
        if (*url == ':') {
            return True;
        }
        ++url;
    }
    return False;
}

void RTSPClient::constructSubsessionURL(MediaSubsession const& subsession,
                                        char const *         & prefix,
                                        char const *         & separator,
                                        char const *         & suffix)
{
    prefix = sessionURL(subsession.parentSession());
    if (prefix == NULL) {
        prefix = "";
    }
    suffix = subsession.controlPath();
    if (suffix == NULL) {
        suffix = "";
    }
    if (isAbsoluteURL(suffix)) {
        prefix = separator = "";
    } else {
        unsigned prefixLen = strlen(prefix);
        separator = (prefixLen == 0 || prefix[prefixLen - 1] == '/' || suffix[0] == '/') ? "" : "/";
    }
}

Boolean RTSPClient::setupHTTPTunneling1()
{
    if (fVerbosityLevel >= 1) {
        envir() << "Requesting RTSP-over-HTTP tunneling (on port " << fTunnelOverHTTPPortNum << ")\n\n";
    }
    return sendRequest(new RequestRecord(1, "GET", responseHandlerForHTTP_GET)) != 0;
}

void RTSPClient::responseHandlerForHTTP_GET(RTSPClient* rtspClient, int responseCode, char* responseString)
{
    if (rtspClient != NULL) {
        rtspClient->responseHandlerForHTTP_GET1(responseCode, responseString);
    }
}

void RTSPClient::responseHandlerForHTTP_GET1(int responseCode, char* responseString)
{
    RequestRecord* request;

    do {
        delete[] responseString;
        if (responseCode != 0) {
            break;
        }
        fOutputSocketNum = setupStreamSocket(envir(), 0);
        if (fOutputSocketNum < 0) {
            break;
        }
        ignoreSigPipeOnSocket(fOutputSocketNum);
        fHTTPTunnelingConnectionIsPending = True;
        int connectResult = connectToServer(fOutputSocketNum, fTunnelOverHTTPPortNum);
        if (connectResult < 0) {
            break;
        } else if (connectResult == 0) {
            while ((request = fRequestsAwaitingHTTPTunneling.dequeue()) != NULL) {
                fRequestsAwaitingConnection.enqueue(request);
            }
            return;
        }
        if (!setupHTTPTunneling2()) {
            break;
        }
        while ((request = fRequestsAwaitingHTTPTunneling.dequeue()) != NULL) {
            sendRequest(request);
        }
        return;
    } while (0);
    fHTTPTunnelingConnectionIsPending = False;
    resetTCPSockets();
    RequestQueue requestQueue(fRequestsAwaitingHTTPTunneling);
    while ((request = requestQueue.dequeue()) != NULL) {
        handleRequestError(request);
        delete request;
    }
} // RTSPClient::responseHandlerForHTTP_GET1

Boolean RTSPClient::setupHTTPTunneling2()
{
    fHTTPTunnelingConnectionIsPending = False;
    return sendRequest(new RequestRecord(1, "POST", NULL)) != 0;
}

void RTSPClient::connectionHandler(void* instance, int)
{
    RTSPClient* client = (RTSPClient *) instance;

    client->connectionHandler1();
}

void RTSPClient::connectionHandler1()
{
    envir().taskScheduler().disableBackgroundHandling(fOutputSocketNum);
    envir().taskScheduler().setBackgroundHandling(fInputSocketNum, SOCKET_READABLE | SOCKET_EXCEPTION,
                                                  (TaskScheduler::BackgroundHandlerProc *) &incomingDataHandler, this);
    RequestQueue tmpRequestQueue(fRequestsAwaitingConnection);
    RequestRecord* request;
    do {
        int err       = 0;
        SOCKLEN_T len = sizeof err;
        if (getsockopt(fInputSocketNum, SOL_SOCKET, SO_ERROR, (char *) &err, &len) < 0 || err != 0) {
            envir().setResultErrMsg("Connection to server failed: ", err);
            if (fVerbosityLevel >= 1) {
                envir() << "..." << envir().getResultMsg() << "\n";
            }
            break;
        }
        if (fVerbosityLevel >= 1) {
            envir() << "...remote connection opened\n";
        }
        if (fHTTPTunnelingConnectionIsPending && !setupHTTPTunneling2()) {
            break;
        }
        while ((request = tmpRequestQueue.dequeue()) != NULL) {
            sendRequest(request);
        }
        return;
    } while (0);
    resetTCPSockets();
    while ((request = tmpRequestQueue.dequeue()) != NULL) {
        handleRequestError(request);
        delete request;
    }
} // RTSPClient::connectionHandler1

void RTSPClient::incomingDataHandler(void* instance, int)
{
    RTSPClient* client = (RTSPClient *) instance;

    client->incomingDataHandler1();
}

void RTSPClient::incomingDataHandler1()
{
    struct sockaddr_in dummy;
    int bytesRead = readSocket(
        envir(), fInputSocketNum, (unsigned char *) &fResponseBuffer[fResponseBytesAlreadySeen], fResponseBufferBytesLeft,
        dummy);

    handleResponseBytes(bytesRead);
}

static char * getLine(char* startOfLine)
{
    for (char* ptr = startOfLine; *ptr != '\0'; ++ptr) {
        if (*ptr == '\r' || *ptr == '\n') {
            if (*ptr == '\r') {
                *ptr++ = '\0';
                if (*ptr == '\n') {
                    ++ptr;
                }
            } else {
                *ptr++ = '\0';
            }
            return ptr;
        }
    }
    return NULL;
}

void RTSPClient::handleResponseBytes(int newBytesRead)
{
    do {
        if (newBytesRead >= 0 && (unsigned) newBytesRead < fResponseBufferBytesLeft) {
            break;
        }
        if (newBytesRead >= (int) fResponseBufferBytesLeft) {
            envir().setResultMsg("RTSP response was truncated. Increase \"RTSPClient::responseBufferSize\"");
        }
        resetResponseBuffer();
        RequestRecord* request;
        if (newBytesRead > 0) {
            if ((request = fRequestsAwaitingResponse.dequeue()) != NULL) {
                handleRequestError(request);
                delete request;
            }
        } else {
            RequestQueue requestQueue(fRequestsAwaitingResponse);
            resetTCPSockets();
            while ((request = requestQueue.dequeue()) != NULL) {
                handleRequestError(request);
                delete request;
            }
        }
        return;
    } while (0);
    fResponseBufferBytesLeft  -= newBytesRead;
    fResponseBytesAlreadySeen += newBytesRead;
    fResponseBuffer[fResponseBytesAlreadySeen] = '\0';
    if (fVerbosityLevel >= 1 && newBytesRead > 1) {
        envir() << "Received " << newBytesRead << " new bytes of response data.\n";
    }
    unsigned numExtraBytesAfterResponse = 0;
    Boolean responseSuccess = False;
    do {
        Boolean endOfHeaders = False;
        char const* ptr      = fResponseBuffer;
        if (fResponseBytesAlreadySeen > 3) {
            char const * const ptrEnd = &fResponseBuffer[fResponseBytesAlreadySeen - 3];
            while (ptr < ptrEnd) {
                if (*ptr++ == '\r' && *ptr++ == '\n' && *ptr++ == '\r' && *ptr++ == '\n') {
                    endOfHeaders = True;
                    break;
                }
            }
        }
        if (!endOfHeaders) {
            return;
        }
        char* headerDataCopy;
        unsigned responseCode                = 200;
        char const* responseStr              = NULL;
        RequestRecord* foundRequest          = NULL;
        char const* sessionParamsStr         = NULL;
        char const* transportParamsStr       = NULL;
        char const* scaleParamsStr           = NULL;
        char const* rangeParamsStr           = NULL;
        char const* rtpInfoParamsStr         = NULL;
        char const* wwwAuthenticateParamsStr = NULL;
        char const* publicParamsStr          = NULL;
        char* bodyStart       = NULL;
        unsigned numBodyBytes = 0;
        responseSuccess = False;
        do {
            headerDataCopy = new char[responseBufferSize];
            strncpy(headerDataCopy, fResponseBuffer, fResponseBytesAlreadySeen);
            headerDataCopy[fResponseBytesAlreadySeen] = '\0';
            char* lineStart     = headerDataCopy;
            char* nextLineStart = getLine(lineStart);
            if (!parseResponseCode(lineStart, responseCode, responseStr)) {
                handleIncomingRequest();
                break;
            }
            Boolean reachedEndOfHeaders;
            unsigned cseq = 0;
            unsigned contentLength = 0;
            while (1) {
                reachedEndOfHeaders = True;
                lineStart = nextLineStart;
                if (lineStart == NULL) {
                    break;
                }
                nextLineStart = getLine(lineStart);
                if (lineStart[0] == '\0') {
                    break;
                }
                reachedEndOfHeaders = False;
                char const* headerParamsStr;
                if (checkForHeader(lineStart, "CSeq:", 5, headerParamsStr)) {
                    if (sscanf(headerParamsStr, "%u", &cseq) != 1 || cseq <= 0) {
                        envir().setResultMsg("Bad \"CSeq:\" header: \"", lineStart, "\"");
                        break;
                    }
                    RequestRecord* request;
                    while ((request = fRequestsAwaitingResponse.dequeue()) != NULL) {
                        if (request->cseq() < cseq) {
                            if (fVerbosityLevel >= 1 && strcmp(request->commandName(), "POST") != 0) {
                                envir() << "WARNING: The server did not respond to our \"" << request->commandName()
                                        << "\" request (CSeq: "
                                        << request->cseq()
                                        <<
                                    ").  The server appears to be buggy (perhaps not handling pipelined requests properly).\n";
                            }
                            delete request;
                        } else if (request->cseq() == cseq) {
                            foundRequest = request;
                            break;
                        } else {
                            break;
                        }
                    }
                } else if (checkForHeader(lineStart, "Content-Length:", 15, headerParamsStr)) {
                    if (sscanf(headerParamsStr, "%u", &contentLength) != 1) {
                        envir().setResultMsg("Bad \"Content-Length:\" header: \"", lineStart, "\"");
                        break;
                    }
                } else if (checkForHeader(lineStart, "Content-Base:", 13, headerParamsStr)) {
                    setBaseURL(headerParamsStr);
                } else if (checkForHeader(lineStart, "Session:", 8,
                                          sessionParamsStr))
                {} else if (checkForHeader(lineStart, "Transport:", 10,
                                           transportParamsStr))
                {} else if (checkForHeader(lineStart, "Scale:", 6,
                                           scaleParamsStr))
                {} else if (checkForHeader(lineStart, "Range:", 6,
                                           rangeParamsStr))
                {} else if (checkForHeader(lineStart, "RTP-Info:", 9,
                                           rtpInfoParamsStr))
                {} else if (checkForHeader(lineStart, "WWW-Authenticate:", 17, headerParamsStr)) {
                    if (wwwAuthenticateParamsStr == NULL || _strncasecmp(headerParamsStr, "Digest", 6) == 0) {
                        wwwAuthenticateParamsStr = headerParamsStr;
                    }
                } else if (checkForHeader(lineStart, "Public:", 7,
                                          publicParamsStr))
                {} else if (checkForHeader(lineStart, "Allow:", 6,
                                           publicParamsStr))
                {} else if (checkForHeader(lineStart, "Location:", 9, headerParamsStr)) {
                    setBaseURL(headerParamsStr);
                }
            }
            if (!reachedEndOfHeaders) {
                break;
            }
            if (foundRequest == NULL) {
                foundRequest = fRequestsAwaitingResponse.dequeue();
            }
            unsigned bodyOffset = nextLineStart == NULL ? fResponseBytesAlreadySeen : nextLineStart - headerDataCopy;
            bodyStart    = &fResponseBuffer[bodyOffset];
            numBodyBytes = fResponseBytesAlreadySeen - bodyOffset;
            if (contentLength > numBodyBytes) {
                unsigned numExtraBytesNeeded = contentLength - numBodyBytes;
                unsigned remainingBufferSize = responseBufferSize - fResponseBytesAlreadySeen;
                if (numExtraBytesNeeded > remainingBufferSize) {
                    char tmpBuf[200];
                    sprintf(tmpBuf,
                            "Response buffer size (%d) is too small for \"Content-Length:\" %d (need a buffer size of >= %d bytes\n",
                            responseBufferSize, contentLength, fResponseBytesAlreadySeen + numExtraBytesNeeded);
                    envir().setResultMsg(tmpBuf);
                    break;
                }
                if (fVerbosityLevel >= 1) {
                    envir() << "Have received " << fResponseBytesAlreadySeen << " total bytes of a "
                            << (foundRequest != NULL ? foundRequest->commandName() : "(unknown)")
                            << " RTSP response; awaiting " << numExtraBytesNeeded << " bytes more.\n";
                }
                delete[] headerDataCopy;
                if (foundRequest != NULL) {
                    fRequestsAwaitingResponse.putAtHead(foundRequest);
                }
                return;
            }
            char* responseEnd = bodyStart + contentLength;
            numExtraBytesAfterResponse = &fResponseBuffer[fResponseBytesAlreadySeen] - responseEnd;
            if (fVerbosityLevel >= 1) {
                char saved = *responseEnd;
                *responseEnd = '\0';
                envir() << "Received a complete "
                        << (foundRequest != NULL ? foundRequest->commandName() : "(unknown)")
                        << " response:\n" << fResponseBuffer << "\n";
                if (numExtraBytesAfterResponse > 0) {
                    envir() << "\t(plus " << numExtraBytesAfterResponse << " additional bytes)\n";
                }
                *responseEnd = saved;
            }
            if (foundRequest != NULL) {
                Boolean needToResendCommand = False;
                if (responseCode == 200) {
                    if (strcmp(foundRequest->commandName(), "SETUP") == 0) {
                        if (!handleSETUPResponse(*foundRequest->subsession(), sessionParamsStr, transportParamsStr,
                                                 foundRequest->booleanFlags() & 0x1))
                        {
                            break;
                        }
                    } else if (strcmp(foundRequest->commandName(), "PLAY") == 0) {
                        if (!handlePLAYResponse(*foundRequest->session(), *foundRequest->subsession(), scaleParamsStr,
                                                rangeParamsStr, rtpInfoParamsStr))
                        {
                            break;
                        }
                    } else if (strcmp(foundRequest->commandName(), "TEARDOWN") == 0) {
                        if (!handleTEARDOWNResponse(*foundRequest->session(), *foundRequest->subsession())) {
                            break;
                        }
                    } else if (strcmp(foundRequest->commandName(), "GET_PARAMETER") == 0) {
                        if (!handleGET_PARAMETERResponse(foundRequest->contentStr(), bodyStart)) {
                            break;
                        }
                    }
                } else if (responseCode == 401 && handleAuthenticationFailure(wwwAuthenticateParamsStr)) {
                    needToResendCommand = True;
                    if (strcmp(foundRequest->commandName(), "GET") == 0) {
                        resetTCPSockets();
                    }
                } else if (responseCode == 301 || responseCode == 302) {
                    resetTCPSockets();
                    needToResendCommand = True;
                }
                if (needToResendCommand) {
                    resetResponseBuffer();
                    if (!resendCommand(foundRequest)) {
                        break;
                    }
                    delete[] headerDataCopy;
                    return;
                }
            }
            responseSuccess = True;
        } while (0);
        if (numExtraBytesAfterResponse > 0) {
            char* responseEnd = &fResponseBuffer[fResponseBytesAlreadySeen - numExtraBytesAfterResponse];
            numBodyBytes -= numExtraBytesAfterResponse;
            if (numBodyBytes > 0) {
                char saved = *responseEnd;
                *responseEnd = '\0';
                bodyStart    = strDup(bodyStart);
                *responseEnd = saved;
            }
            memmove(fResponseBuffer, responseEnd, numExtraBytesAfterResponse);
            fResponseBytesAlreadySeen = numExtraBytesAfterResponse;
            fResponseBufferBytesLeft  = responseBufferSize - numExtraBytesAfterResponse;
            fResponseBuffer[numExtraBytesAfterResponse] = '\0';
        } else {
            resetResponseBuffer();
        }
        if (foundRequest != NULL && foundRequest->handler() != NULL) {
            int resultCode;
            char* resultString;
            if (responseSuccess) {
                if (responseCode == 200) {
                    resultCode   = 0;
                    resultString = numBodyBytes > 0 ? strDup(bodyStart) : strDup(publicParamsStr);
                } else {
                    resultCode   = responseCode;
                    resultString = strDup(responseStr);
                    envir().setResultMsg(responseStr);
                }
                (*foundRequest->handler())(this, resultCode, resultString);
            } else {
                handleRequestError(foundRequest);
            }
        }
        delete foundRequest;
        delete[] headerDataCopy;
        if (numExtraBytesAfterResponse > 0 && numBodyBytes > 0) {
            delete[] bodyStart;
        }
    } while (numExtraBytesAfterResponse > 0 && responseSuccess);
} // RTSPClient::handleResponseBytes

RTSPClient::RequestRecord::RequestRecord(unsigned cseq, char const* commandName, responseHandler* handler,
                                         MediaSession* session, MediaSubsession* subsession, u_int32_t booleanFlags,
                                         double start, double end, float scale, char const* contentStr)
    : fNext(NULL), fCSeq(cseq), fCommandName(commandName), fSession(session), fSubsession(subsession), fBooleanFlags(
        booleanFlags),
    fStart(start), fEnd(end), fAbsStartTime(NULL), fAbsEndTime(NULL), fScale(scale), fContentStr(strDup(contentStr)),
    fHandler(handler)
{}

RTSPClient::RequestRecord::RequestRecord(unsigned cseq, responseHandler* handler,
                                         char const* absStartTime, char const* absEndTime, float scale,
                                         MediaSession* session, MediaSubsession* subsession)
    : fNext(NULL), fCSeq(cseq), fCommandName("PLAY"), fSession(session), fSubsession(subsession), fBooleanFlags(0),
    fStart(0.0f), fEnd(-1.0f), fAbsStartTime(strDup(absStartTime)), fAbsEndTime(strDup(absEndTime)), fScale(scale),
    fContentStr(NULL), fHandler(handler)
{}

RTSPClient::RequestRecord::~RequestRecord()
{
    delete fNext;
    delete[] fAbsStartTime;
    delete[] fAbsEndTime;
    delete[] fContentStr;
}

RTSPClient::RequestQueue::RequestQueue()
    : fHead(NULL), fTail(NULL)
{}

RTSPClient::RequestQueue::RequestQueue(RequestQueue& origQueue)
    : fHead(NULL), fTail(NULL)
{
    RequestRecord* request;

    while ((request = origQueue.dequeue()) != NULL) {
        enqueue(request);
    }
}

RTSPClient::RequestQueue::~RequestQueue()
{
    delete fHead;
}

void RTSPClient::RequestQueue::enqueue(RequestRecord* request)
{
    if (fTail == NULL) {
        fHead = request;
    } else {
        fTail->next() = request;
    }
    fTail = request;
}

RTSPClient::RequestRecord * RTSPClient::RequestQueue::dequeue()
{
    RequestRecord* request = fHead;

    if (fHead == fTail) {
        fHead = NULL;
        fTail = NULL;
    } else {
        fHead = fHead->next();
    }
    if (request != NULL) {
        request->next() = NULL;
    }
    return request;
}

void RTSPClient::RequestQueue::putAtHead(RequestRecord* request)
{
    request->next() = fHead;
    fHead = request;
    if (fTail == NULL) {
        fTail = request;
    }
}

RTSPClient::RequestRecord * RTSPClient::RequestQueue::findByCSeq(unsigned cseq)
{
    RequestRecord* request;

    for (request = fHead; request != NULL; request = request->next()) {
        if (request->cseq() == cseq) {
            return request;
        }
    }
    return NULL;
}

HandlerServerForREGISTERCommand * HandlerServerForREGISTERCommand
::createNew(UsageEnvironment& env, onRTSPClientCreationFunc* creationFunc, Port ourPort,
            UserAuthenticationDatabase* authDatabase, int verbosityLevel, char const* applicationName)
{
    int ourSocket = setUpOurSocket(env, ourPort);

    if (ourSocket == -1) {
        return NULL;
    }
    return new HandlerServerForREGISTERCommand(env, creationFunc, ourSocket, ourPort, authDatabase, verbosityLevel,
                                               applicationName);
}

HandlerServerForREGISTERCommand
::HandlerServerForREGISTERCommand(UsageEnvironment& env, onRTSPClientCreationFunc* creationFunc, int ourSocket,
                                  Port ourPort,
                                  UserAuthenticationDatabase* authDatabase, int verbosityLevel,
                                  char const* applicationName)
    : RTSPServer(env, ourSocket, ourPort, authDatabase, 30),
    fCreationFunc(creationFunc), fVerbosityLevel(verbosityLevel), fApplicationName(strDup(applicationName))
{}

HandlerServerForREGISTERCommand::~HandlerServerForREGISTERCommand()
{
    delete[] fApplicationName;
}

RTSPClient * HandlerServerForREGISTERCommand
::createNewRTSPClient(char const* rtspURL, int verbosityLevel, char const* applicationName, int socketNumToServer)
{
    return RTSPClient::createNew(envir(), rtspURL, verbosityLevel, applicationName, 0, socketNumToServer);
}

char const * HandlerServerForREGISTERCommand::allowedCommandNames()
{
    return "OPTIONS, REGISTER";
}

Boolean HandlerServerForREGISTERCommand::weImplementREGISTER(char const* proxyURLSuffix, char *& responseStr)
{
    responseStr = NULL;
    return True;
}

void HandlerServerForREGISTERCommand::implementCmd_REGISTER(char const* url, char const* urlSuffix,
                                                            int socketToRemoteServer,
                                                            Boolean deliverViaTCP, char const *)
{
    RTSPClient* newRTSPClient = createNewRTSPClient(url, fVerbosityLevel, fApplicationName, socketToRemoteServer);

    if (fCreationFunc != NULL) {
        (*fCreationFunc)(newRTSPClient, deliverViaTCP);
    }
}
