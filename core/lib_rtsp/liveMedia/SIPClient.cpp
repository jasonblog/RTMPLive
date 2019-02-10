#include "SIPClient.hh"
#include "GroupsockHelper.hh"
#if defined(__WIN32__) || defined(_WIN32) || defined(_QNX4)
# define _strncasecmp _strnicmp
#else
# define _strncasecmp strncasecmp
#endif
SIPClient * SIPClient
::createNew(UsageEnvironment& env,
            unsigned char desiredAudioRTPPayloadFormat,
            char const* mimeSubtype,
            int verbosityLevel, char const* applicationName)
{
    return new SIPClient(env, desiredAudioRTPPayloadFormat, mimeSubtype,
                         verbosityLevel, applicationName);
}

void SIPClient::setUserAgentString(char const* userAgentName)
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

SIPClient::SIPClient(UsageEnvironment& env,
                     unsigned char desiredAudioRTPPayloadFormat,
                     char const* mimeSubtype,
                     int verbosityLevel, char const* applicationName)
    : Medium(env),
    fT1(500000),
    fDesiredAudioRTPPayloadFormat(desiredAudioRTPPayloadFormat),
    fVerbosityLevel(verbosityLevel), fCSeq(0),
    fUserAgentHeaderStr(NULL), fUserAgentHeaderStrLen(0),
    fURL(NULL), fURLSize(0),
    fToTagStr(NULL), fToTagStrSize(0),
    fUserName(NULL), fUserNameSize(0),
    fInviteSDPDescription(NULL), fInviteSDPDescriptionReturned(NULL),
    fInviteCmd(NULL), fInviteCmdSize(0)
{
    if (mimeSubtype == NULL) {
        mimeSubtype = "";
    }
    fMIMESubtype     = strDup(mimeSubtype);
    fMIMESubtypeSize = strlen(fMIMESubtype);
    if (applicationName == NULL) {
        applicationName = "";
    }
    fApplicationName     = strDup(applicationName);
    fApplicationNameSize = strlen(fApplicationName);
    struct in_addr ourAddress;
    ourAddress.s_addr  = ourIPAddress(env);
    fOurAddressStr     = strDup(AddressString(ourAddress).val());
    fOurAddressStrSize = strlen(fOurAddressStr);
    fOurSocket         = new Groupsock(env, ourAddress, 0, 255);
    if (fOurSocket == NULL) {
        env << "ERROR: Failed to create socket for addr "
            << fOurAddressStr << ": "
            << env.getResultMsg() << "\n";
    }
    fOurSocket->output(envir(), 255, (unsigned char *) "", 0);
    Port srcPort(0);
    getSourcePort(env, fOurSocket->socketNum(), srcPort);
    if (srcPort.num() != 0) {
        fOurPortNum = ntohs(srcPort.num());
    } else {
        fOurPortNum = 5060;
        delete fOurSocket;
        fOurSocket = new Groupsock(env, ourAddress, fOurPortNum, 255);
        if (fOurSocket == NULL) {
            env << "ERROR: Failed to create socket for addr "
                << fOurAddressStr << ", port "
                << fOurPortNum << ": "
                << env.getResultMsg() << "\n";
        }
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
        fApplicationNameSize + strlen(libPrefix) + strlen(libName) + strlen(libVersionStr) + strlen(libSuffix) + 1;
    char* userAgentName = new char[userAgentNameSize];
    sprintf(userAgentName, "%s%s%s%s%s",
            applicationName, libPrefix, libName, libVersionStr, libSuffix);
    setUserAgentString(userAgentName);
    delete[] userAgentName;
    reset();
}

SIPClient::~SIPClient()
{
    reset();
    delete[] fUserAgentHeaderStr;
    delete fOurSocket;
    delete[](char *) fOurAddressStr;
    delete[](char *) fApplicationName;
    delete[](char *) fMIMESubtype;
}

void SIPClient::reset()
{
    fWorkingAuthenticator = NULL;
    delete[] fInviteCmd;
    fInviteCmd     = NULL;
    fInviteCmdSize = 0;
    delete[] fInviteSDPDescription;
    fInviteSDPDescription = NULL;
    delete[](char *) fUserName;
    fUserName     = strDup(fApplicationName);
    fUserNameSize = strlen(fUserName);
    fValidAuthenticator.reset();
    delete[](char *) fToTagStr;
    fToTagStr             = NULL;
    fToTagStrSize         = 0;
    fServerPortNum        = 0;
    fServerAddress.s_addr = 0;
    delete[](char *) fURL;
    fURL     = NULL;
    fURLSize = 0;
}

void SIPClient::setProxyServer(unsigned    proxyServerAddress,
                               portNumBits proxyServerPortNum)
{
    fServerAddress.s_addr = proxyServerAddress;
    fServerPortNum        = proxyServerPortNum;
    if (fOurSocket != NULL) {
        fOurSocket->changeDestinationParameters(fServerAddress,
                                                fServerPortNum, 255);
    }
}

static char * getLine(char* startOfLine)
{
    for (char* ptr = startOfLine; *ptr != '\0'; ++ptr) {
        if (*ptr == '\r' || *ptr == '\n') {
            *ptr++ = '\0';
            if (*ptr == '\n') {
                ++ptr;
            }
            return ptr;
        }
    }
    return NULL;
}

char * SIPClient::invite(char const* url, Authenticator* authenticator)
{
    char* username;
    char* password;

    if (authenticator == NULL &&
        parseSIPURLUsernamePassword(url, username, password))
    {
        char* result = inviteWithPassword(url, username, password);
        delete[] username;
        delete[] password;
        return result;
    }
    if (!processURL(url)) {
        return NULL;
    }
    delete[](char *) fURL;
    fURL     = strDup(url);
    fURLSize = strlen(fURL);
    fCallId  = our_random32();
    fFromTag = our_random32();
    return invite1(authenticator);
}

char * SIPClient::invite1(Authenticator* authenticator)
{
    do {
        fValidAuthenticator.reset();
        fWorkingAuthenticator = authenticator;
        char* authenticatorStr =
            createAuthenticatorString(fWorkingAuthenticator, "INVITE", fURL);
        char* rtpmapLine;
        unsigned rtpmapLineSize;
        if (fMIMESubtypeSize > 0) {
            char const * const rtpmapFmt =
                "a=rtpmap:%u %s/8000\r\n";
            unsigned rtpmapFmtSize = strlen(rtpmapFmt)
                                     + 3  + fMIMESubtypeSize;
            rtpmapLine = new char[rtpmapFmtSize];
            sprintf(rtpmapLine, rtpmapFmt,
                    fDesiredAudioRTPPayloadFormat, fMIMESubtype);
            rtpmapLineSize = strlen(rtpmapLine);
        } else {
            rtpmapLine     = strDup("");
            rtpmapLineSize = 0;
        }
        char const * const inviteSDPFmt =
            "v=0\r\n"
            "o=- %u %u IN IP4 %s\r\n"
            "s=%s session\r\n"
            "c=IN IP4 %s\r\n"
            "t=0 0\r\n"
            "m=audio %u RTP/AVP %u\r\n"
            "%s";
        unsigned inviteSDPFmtSize = strlen(inviteSDPFmt)
                                    + 20  + 20 + fOurAddressStrSize
                                    + fApplicationNameSize
                                    + fOurAddressStrSize
                                    + 5  + 3
                                    + rtpmapLineSize;
        delete[] fInviteSDPDescription;
        fInviteSDPDescription = new char[inviteSDPFmtSize];
        sprintf(fInviteSDPDescription, inviteSDPFmt,
                fCallId, fCSeq, fOurAddressStr,
                fApplicationName,
                fOurAddressStr,
                fClientStartPortNum, fDesiredAudioRTPPayloadFormat,
                rtpmapLine);
        unsigned inviteSDPSize = strlen(fInviteSDPDescription);
        delete[] rtpmapLine;
        char const * const cmdFmt =
            "INVITE %s SIP/2.0\r\n"
            "From: %s <sip:%s@%s>;tag=%u\r\n"
            "Via: SIP/2.0/UDP %s:%u\r\n"
            "Max-Forwards: 70\r\n"
            "To: %s\r\n"
            "Contact: sip:%s@%s:%u\r\n"
            "Call-ID: %u@%s\r\n"
            "CSeq: %d INVITE\r\n"
            "Content-Type: application/sdp\r\n"
            "%s"
            "%s"
            "Content-Length: %d\r\n\r\n"
            "%s";
        unsigned inviteCmdSize = strlen(cmdFmt)
                                 + fURLSize
                                 + 2 * fUserNameSize + fOurAddressStrSize + 20
                                 + fOurAddressStrSize + 5
                                 + fURLSize
                                 + fUserNameSize + fOurAddressStrSize + 5
                                 + 20 + fOurAddressStrSize
                                 + 20
                                 + strlen(authenticatorStr)
                                 + fUserAgentHeaderStrLen
                                 + 20
                                 + inviteSDPSize;
        delete[] fInviteCmd;
        fInviteCmd = new char[inviteCmdSize];
        sprintf(fInviteCmd, cmdFmt,
                fURL,
                fUserName, fUserName, fOurAddressStr, fFromTag,
                fOurAddressStr, fOurPortNum,
                fURL,
                fUserName, fOurAddressStr, fOurPortNum,
                fCallId, fOurAddressStr,
                ++fCSeq,
                authenticatorStr,
                fUserAgentHeaderStr,
                inviteSDPSize,
                fInviteSDPDescription);
        fInviteCmdSize = strlen(fInviteCmd);
        delete[] authenticatorStr;
        fInviteClientState = Calling;
        fEventLoopStopFlag = 0;
        TaskScheduler& sched = envir().taskScheduler();
        sched.turnOnBackgroundReadHandling(fOurSocket->socketNum(),
                                           &inviteResponseHandler, this);
        fTimerALen   = 1 * fT1;
        fTimerACount = 0;
        fTimerA      = sched.scheduleDelayedTask(fTimerALen, timerAHandler, this);
        fTimerB      = sched.scheduleDelayedTask(64 * fT1, timerBHandler, this);
        fTimerD      = NULL;
        if (!sendINVITE()) {
            break;
        }
        envir().taskScheduler().doEventLoop(&fEventLoopStopFlag);
        sched.turnOffBackgroundReadHandling(fOurSocket->socketNum());
        sched.unscheduleDelayedTask(fTimerA);
        sched.unscheduleDelayedTask(fTimerB);
        sched.unscheduleDelayedTask(fTimerD);
        if (fInviteSDPDescription != NULL) {
            return strDup(fInviteSDPDescription);
        }
    } while (0);
    return NULL;
} // SIPClient::invite1

void SIPClient::inviteResponseHandler(void* clientData, int)
{
    SIPClient* client     = (SIPClient *) clientData;
    unsigned responseCode = client->getResponseCode();

    client->doInviteStateMachine(responseCode);
}

unsigned const timerAFires = 0xAAAAAAAA;
unsigned const timerBFires = 0xBBBBBBBB;
unsigned const timerDFires = 0xDDDDDDDD;
void SIPClient::timerAHandler(void* clientData)
{
    SIPClient* client = (SIPClient *) clientData;

    if (client->fVerbosityLevel >= 1) {
        client->envir() << "RETRANSMISSION " << ++client->fTimerACount
                        << ", after " << client->fTimerALen / 1000000.0
                        << " additional seconds\n";
    }
    client->doInviteStateMachine(timerAFires);
}

void SIPClient::timerBHandler(void* clientData)
{
    SIPClient* client = (SIPClient *) clientData;

    if (client->fVerbosityLevel >= 1) {
        client->envir() << "RETRANSMISSION TIMEOUT, after "
                        << 64* client->fT1 / 1000000.0 << " seconds\n";
        fflush(stderr);
    }
    client->doInviteStateMachine(timerBFires);
}

void SIPClient::timerDHandler(void* clientData)
{
    SIPClient* client = (SIPClient *) clientData;

    if (client->fVerbosityLevel >= 1) {
        client->envir() << "TIMER D EXPIRED\n";
    }
    client->doInviteStateMachine(timerDFires);
}

void SIPClient::doInviteStateMachine(unsigned responseCode)
{
    TaskScheduler& sched = envir().taskScheduler();

    switch (fInviteClientState) {
        case Calling: {
            if (responseCode == timerAFires) {
                fTimerALen *= 2;
                fTimerA     =
                    sched.scheduleDelayedTask(fTimerALen, timerAHandler, this);
                fInviteClientState = Calling;
                if (!sendINVITE()) {
                    doInviteStateTerminated(0);
                }
            } else {
                sched.unscheduleDelayedTask(fTimerA);
                sched.unscheduleDelayedTask(fTimerB);
                if (responseCode == timerBFires) {
                    envir().setResultMsg("No response from server");
                    doInviteStateTerminated(0);
                } else if (responseCode >= 100 && responseCode <= 199) {
                    fInviteClientState = Proceeding;
                } else if (responseCode >= 200 && responseCode <= 299) {
                    doInviteStateTerminated(responseCode);
                } else if (responseCode >= 400 && responseCode <= 499) {
                    doInviteStateTerminated(responseCode);
                } else if (responseCode >= 300 && responseCode <= 699) {
                    fInviteClientState = Completed;
                    fTimerD =
                        sched.scheduleDelayedTask(32000000, timerDHandler, this);
                    if (!sendACK()) {
                        doInviteStateTerminated(0);
                    }
                }
            }
            break;
        }
        case Proceeding: {
            if (responseCode >= 100 && responseCode <= 199) {
                fInviteClientState = Proceeding;
            } else if (responseCode >= 200 && responseCode <= 299) {
                doInviteStateTerminated(responseCode);
            } else if (responseCode >= 400 && responseCode <= 499) {
                doInviteStateTerminated(responseCode);
            } else if (responseCode >= 300 && responseCode <= 699) {
                fInviteClientState = Completed;
                fTimerD = sched.scheduleDelayedTask(32000000, timerDHandler, this);
                if (!sendACK()) {
                    doInviteStateTerminated(0);
                }
            }
            break;
        }
        case Completed: {
            if (responseCode == timerDFires) {
                envir().setResultMsg("Transaction terminated");
                doInviteStateTerminated(0);
            } else if (responseCode >= 300 && responseCode <= 699) {
                fInviteClientState = Completed;
                if (!sendACK()) {
                    doInviteStateTerminated(0);
                }
            }
            break;
        }
        case Terminated: {
            doInviteStateTerminated(responseCode);
            break;
        }
    }
} // SIPClient::doInviteStateMachine

void SIPClient::doInviteStateTerminated(unsigned responseCode)
{
    fInviteClientState = Terminated;
    if (responseCode < 200 || responseCode > 299) {
        delete[] fInviteSDPDescription;
        fInviteSDPDescription = NULL;
        delete[] fInviteSDPDescriptionReturned;
        fInviteSDPDescriptionReturned = NULL;
    }
    fEventLoopStopFlag = ~0;
}

Boolean SIPClient::sendINVITE()
{
    if (!sendRequest(fInviteCmd, fInviteCmdSize)) {
        envir().setResultErrMsg("INVITE send() failed: ");
        return False;
    }
    return True;
}

unsigned SIPClient::getResponseCode()
{
    unsigned responseCode = 0;

    do {
        unsigned const readBufSize = 10000;
        char readBuffer[readBufSize + 1];
        char* readBuf       = readBuffer;
        char* firstLine     = NULL;
        char* nextLineStart = NULL;
        unsigned bytesRead  = getResponse(readBuf, readBufSize);
        if (bytesRead == 0) {
            break;
        }
        if (fVerbosityLevel >= 1) {
            envir() << "Received INVITE response: " << readBuf << "\n";
        }
        firstLine     = readBuf;
        nextLineStart = getLine(firstLine);
        if (!parseResponseCode(firstLine, responseCode)) {
            break;
        }
        if (responseCode != 200) {
            if (responseCode >= 400 && responseCode <= 499 &&
                fWorkingAuthenticator != NULL)
            {
                char* lineStart;
                while (1) {
                    lineStart = nextLineStart;
                    if (lineStart == NULL) {
                        break;
                    }
                    nextLineStart = getLine(lineStart);
                    if (lineStart[0] == '\0') {
                        break;
                    }
                    char* realm = strDupSize(lineStart);
                    char* nonce = strDupSize(lineStart);
                    Boolean foundAuthenticateHeader = False;
                    if (
                        sscanf(lineStart, "Proxy-Authenticate: Digest realm=\"%[^\"]\", nonce=\"%[^\"]\"",
                               realm, nonce) == 2 ||
                        sscanf(lineStart,
                               "Proxy-Authenticate: Digest algorithm=MD5,domain=\"%*[^\"]\",nonce=\"%[^\"]\", realm=\"%[^\"]\"",
                               nonce, realm) == 2)
                    {
                        fWorkingAuthenticator->setRealmAndNonce(realm, nonce);
                        foundAuthenticateHeader = True;
                    }
                    delete[] realm;
                    delete[] nonce;
                    if (foundAuthenticateHeader) {
                        break;
                    }
                }
            }
            envir().setResultMsg("cannot handle INVITE response: ", firstLine);
            break;
        }
        int contentLength = -1;
        char* lineStart;
        while (1) {
            lineStart = nextLineStart;
            if (lineStart == NULL) {
                break;
            }
            nextLineStart = getLine(lineStart);
            if (lineStart[0] == '\0') {
                break;
            }
            char* toTagStr = strDupSize(lineStart);
            if (sscanf(lineStart, "To:%*[^;]; tag=%s", toTagStr) == 1) {
                delete[](char *) fToTagStr;
                fToTagStr     = strDup(toTagStr);
                fToTagStrSize = strlen(fToTagStr);
            }
            delete[] toTagStr;
            if (sscanf(lineStart, "Content-Length: %d", &contentLength) == 1 ||
                sscanf(lineStart, "Content-length: %d", &contentLength) == 1)
            {
                if (contentLength < 0) {
                    envir().setResultMsg("Bad \"Content-Length:\" header: \"",
                                         lineStart, "\"");
                    break;
                }
            }
        }
        if (lineStart == NULL) {
            envir().setResultMsg("no content following header lines: ", readBuf);
            break;
        }
        char* bodyStart = nextLineStart;
        if (bodyStart != NULL && contentLength >= 0) {
            unsigned numBodyBytes = &readBuf[bytesRead] - bodyStart;
            if (contentLength > (int) numBodyBytes) {
                unsigned numExtraBytesNeeded = contentLength - numBodyBytes;
                #ifdef USING_TCP
                unsigned remainingBufferSize =
                    readBufSize - (bytesRead + (readBuf - readBuffer));
                if (numExtraBytesNeeded > remainingBufferSize) {
                    char tmpBuf[200];
                    sprintf(tmpBuf,
                            "Read buffer size (%d) is too small for \"Content-Length:\" %d (need a buffer size of >= %d bytes\n",
                            readBufSize, contentLength,
                            readBufSize + numExtraBytesNeeded - remainingBufferSize);
                    envir().setResultMsg(tmpBuf);
                    break;
                }
                if (fVerbosityLevel >= 1) {
                    envir() << "Need to read " << numExtraBytesNeeded
                            << " extra bytes\n";
                }
                while (numExtraBytesNeeded > 0) {
                    char* ptr = &readBuf[bytesRead];
                    unsigned bytesRead2;
                    struct sockaddr_in fromAddr;
                    Boolean readSuccess =
                        fOurSocket->handleRead((unsigned char *) ptr,
                                               numExtraBytesNeeded,
                                               bytesRead2, fromAddr);
                    if (!readSuccess) {
                        break;
                    }
                    ptr[bytesRead2] = '\0';
                    if (fVerbosityLevel >= 1) {
                        envir() << "Read " << bytesRead2
                                << " extra bytes: " << ptr << "\n";
                    }
                    bytesRead += bytesRead2;
                    numExtraBytesNeeded -= bytesRead2;
                }
                #endif // ifdef USING_TCP
                if (numExtraBytesNeeded > 0) {
                    break;
                }
            }
            bodyStart[contentLength] = '\0';
            delete[] fInviteSDPDescriptionReturned;
            fInviteSDPDescriptionReturned = strDup(bodyStart);
        }
    } while (0);
    return responseCode;
} // SIPClient::getResponseCode

char * SIPClient::inviteWithPassword(char const* url, char const* username,
                                     char const* password)
{
    delete[](char *) fUserName;
    fUserName     = strDup(username);
    fUserNameSize = strlen(fUserName);
    Authenticator authenticator(username, password);
    char* inviteResult = invite(url, &authenticator);
    if (inviteResult != NULL) {
        return inviteResult;
    }
    if (authenticator.realm() == NULL || authenticator.nonce() == NULL) {
        return NULL;
    }
    inviteResult = invite1(&authenticator);
    if (inviteResult != NULL) {
        fValidAuthenticator = authenticator;
    }
    return inviteResult;
}

Boolean SIPClient::sendACK()
{
    char* cmd = NULL;

    do {
        char const * const cmdFmt =
            "ACK %s SIP/2.0\r\n"
            "From: %s <sip:%s@%s>;tag=%u\r\n"
            "Via: SIP/2.0/UDP %s:%u\r\n"
            "Max-Forwards: 70\r\n"
            "To: %s;tag=%s\r\n"
            "Call-ID: %u@%s\r\n"
            "CSeq: %d ACK\r\n"
            "Content-Length: 0\r\n\r\n";
        unsigned cmdSize = strlen(cmdFmt)
                           + fURLSize
                           + 2 * fUserNameSize + fOurAddressStrSize + 20
                           + fOurAddressStrSize + 5
                           + fURLSize + fToTagStrSize
                           + 20 + fOurAddressStrSize
                           + 20;
        cmd = new char[cmdSize];
        sprintf(cmd, cmdFmt,
                fURL,
                fUserName, fUserName, fOurAddressStr, fFromTag,
                fOurAddressStr, fOurPortNum,
                fURL, fToTagStr,
                fCallId, fOurAddressStr,
                fCSeq);
        if (!sendRequest(cmd, strlen(cmd))) {
            envir().setResultErrMsg("ACK send() failed: ");
            break;
        }
        delete[] cmd;
        return True;
    } while (0);
    delete[] cmd;
    return False;
} // SIPClient::sendACK

Boolean SIPClient::sendBYE()
{
    char* cmd = NULL;

    do {
        char const * const cmdFmt =
            "BYE %s SIP/2.0\r\n"
            "From: %s <sip:%s@%s>;tag=%u\r\n"
            "Via: SIP/2.0/UDP %s:%u\r\n"
            "Max-Forwards: 70\r\n"
            "To: %s;tag=%s\r\n"
            "Call-ID: %u@%s\r\n"
            "CSeq: %d BYE\r\n"
            "Content-Length: 0\r\n\r\n";
        unsigned cmdSize = strlen(cmdFmt)
                           + fURLSize
                           + 2 * fUserNameSize + fOurAddressStrSize + 20
                           + fOurAddressStrSize + 5
                           + fURLSize + fToTagStrSize
                           + 20 + fOurAddressStrSize
                           + 20;
        cmd = new char[cmdSize];
        sprintf(cmd, cmdFmt,
                fURL,
                fUserName, fUserName, fOurAddressStr, fFromTag,
                fOurAddressStr, fOurPortNum,
                fURL, fToTagStr,
                fCallId, fOurAddressStr,
                ++fCSeq);
        if (!sendRequest(cmd, strlen(cmd))) {
            envir().setResultErrMsg("BYE send() failed: ");
            break;
        }
        delete[] cmd;
        return True;
    } while (0);
    delete[] cmd;
    return False;
} // SIPClient::sendBYE

Boolean SIPClient::processURL(char const* url)
{
    do {
        if (fServerAddress.s_addr == 0) {
            NetAddress destAddress;
            if (!parseSIPURL(envir(), url, destAddress, fServerPortNum)) {
                break;
            }
            fServerAddress.s_addr = *(unsigned *) (destAddress.data());
            if (fOurSocket != NULL) {
                fOurSocket->changeDestinationParameters(fServerAddress,
                                                        fServerPortNum, 255);
            }
        }
        return True;
    } while (0);
    return False;
}

Boolean SIPClient::parseSIPURL(UsageEnvironment& env, char const* url,
                               NetAddress& address,
                               portNumBits& portNum)
{
    do {
        char const* prefix = "sip:";
        unsigned const prefixLength = 4;
        if (_strncasecmp(url, prefix, prefixLength) != 0) {
            env.setResultMsg("URL is not of the form \"", prefix, "\"");
            break;
        }
        unsigned const parseBufferSize = 100;
        char parseBuffer[parseBufferSize];
        unsigned addressStartIndex = prefixLength;
        while (url[addressStartIndex] != '\0' &&
               url[addressStartIndex++] != '@')
        {}
        char const* from  = &url[addressStartIndex];
        char const* from1 = from;
        while (*from1 != '\0' && *from1 != '/') {
            if (*from1 == '@') {
                from = ++from1;
                break;
            }
            ++from1;
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
        portNum = 5060;
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
        }
        return True;
    } while (0);
    return False;
} // SIPClient::parseSIPURL

Boolean SIPClient::parseSIPURLUsernamePassword(char const* url,
                                               char *      & username,
                                               char *      & password)
{
    username = password = NULL;
    do {
        char const* prefix = "sip:";
        unsigned const prefixLength = 4;
        if (_strncasecmp(url, prefix, prefixLength) != 0) {
            break;
        }
        unsigned usernameIndex = prefixLength;
        unsigned colonIndex = 0, atIndex = 0;
        for (unsigned i = usernameIndex; url[i] != '\0' && url[i] != '/'; ++i) {
            if (url[i] == ':' && colonIndex == 0) {
                colonIndex = i;
            } else if (url[i] == '@') {
                atIndex = i;
                break;
            }
        }
        if (atIndex == 0) {
            break;
        }
        char* urlCopy = strDup(url);
        urlCopy[atIndex] = '\0';
        if (colonIndex > 0) {
            urlCopy[colonIndex] = '\0';
            password = strDup(&urlCopy[colonIndex + 1]);
        } else {
            password = strDup("");
        }
        username = strDup(&urlCopy[usernameIndex]);
        delete[] urlCopy;
        return True;
    } while (0);
    return False;
} // SIPClient::parseSIPURLUsernamePassword

char * SIPClient::createAuthenticatorString(Authenticator const* authenticator,
                                            char const* cmd, char const* url)
{
    if (authenticator != NULL && authenticator->realm() != NULL &&
        authenticator->nonce() != NULL && authenticator->username() != NULL &&
        authenticator->password() != NULL)
    {
        char const * const authFmt =
            "Proxy-Authorization: Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", response=\"%s\", uri=\"%s\"\r\n";
        char const* response = authenticator->computeDigestResponse(cmd, url);
        unsigned authBufSize = strlen(authFmt)
                               + strlen(authenticator->username()) + strlen(authenticator->realm())
                               + strlen(authenticator->nonce()) + strlen(url) + strlen(response);
        char* authenticatorStr = new char[authBufSize];
        sprintf(authenticatorStr, authFmt,
                authenticator->username(), authenticator->realm(),
                authenticator->nonce(), response, url);
        authenticator->reclaimDigestResponse(response);
        return authenticatorStr;
    }
    return strDup("");
}

Boolean SIPClient::sendRequest(char const* requestString,
                               unsigned    requestLength)
{
    if (fVerbosityLevel >= 1) {
        envir() << "Sending request: " << requestString << "\n";
    }
    return fOurSocket->output(envir(), 255, (unsigned char *) requestString,
                              requestLength);
}

unsigned SIPClient::getResponse(char *   & responseBuffer,
                                unsigned responseBufferSize)
{
    if (responseBufferSize == 0) {
        return 0;
    }
    responseBuffer[0] = '\0';
    char* p = responseBuffer;
    Boolean haveSeenNonCRLF = False;
    int bytesRead = 0;
    while (bytesRead < (int) responseBufferSize) {
        unsigned bytesReadNow;
        struct sockaddr_in fromAddr;
        unsigned char* toPosn = (unsigned char *) (responseBuffer + bytesRead);
        Boolean readSuccess   =
            fOurSocket->handleRead(toPosn, responseBufferSize - bytesRead,
                                   bytesReadNow, fromAddr);
        if (!readSuccess || bytesReadNow == 0) {
            envir().setResultMsg("SIP response was truncated");
            break;
        }
        bytesRead += bytesReadNow;
        char* lastToCheck = responseBuffer + bytesRead - 4;
        if (lastToCheck < responseBuffer) {
            continue;
        }
        for (; p <= lastToCheck; ++p) {
            if (haveSeenNonCRLF) {
                if (*p == '\r' && *(p + 1) == '\n' &&
                    *(p + 2) == '\r' && *(p + 3) == '\n')
                {
                    responseBuffer[bytesRead] = '\0';
                    while (*responseBuffer == '\r' || *responseBuffer == '\n') {
                        ++responseBuffer;
                        --bytesRead;
                    }
                    return bytesRead;
                }
            } else {
                if (*p != '\r' && *p != '\n') {
                    haveSeenNonCRLF = True;
                }
            }
        }
    }
    return 0;
} // SIPClient::getResponse

Boolean SIPClient::parseResponseCode(char const* line,
                                     unsigned    & responseCode)
{
    if (sscanf(line, "%*s%u", &responseCode) != 1) {
        envir().setResultMsg("no response code in line: \"", line, "\"");
        return False;
    }
    return True;
}
