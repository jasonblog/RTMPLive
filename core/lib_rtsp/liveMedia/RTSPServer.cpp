#include "RTSPServer.hh"
#include "RTSPCommon.hh"
#include "RTSPRegisterSender.hh"
#include "ProxyServerMediaSession.hh"
#include "Base64.hh"
#include <GroupsockHelper.hh>
RTSPServer * RTSPServer::createNew(UsageEnvironment& env, Port ourPort,
                                   UserAuthenticationDatabase* authDatabase,
                                   unsigned reclamationTestSeconds)
{
    int ourSocket = setUpOurSocket(env, ourPort);

    if (ourSocket == -1) {
        return NULL;
    }
    return new RTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

Boolean RTSPServer::lookupByName(UsageEnvironment& env,
                                 char const*     name,
                                 RTSPServer *    & resultServer)
{
    resultServer = NULL;
    Medium* medium;
    if (!Medium::lookupByName(env, name, medium)) {
        return False;
    }
    if (!medium->isRTSPServer()) {
        env.setResultMsg(name, " is not a RTSP server");
        return False;
    }
    resultServer = (RTSPServer *) medium;
    return True;
}

void RTSPServer::addServerMediaSession(ServerMediaSession* serverMediaSession)
{
    if (serverMediaSession == NULL) {
        return;
    }
    char const* sessionName = serverMediaSession->streamName();
    if (sessionName == NULL) {
        sessionName = "";
    }
    removeServerMediaSession(sessionName);
    fServerMediaSessions->Add(sessionName, (void *) serverMediaSession);
}

ServerMediaSession * RTSPServer
::lookupServerMediaSession(char const* streamName, Boolean)
{
    return (ServerMediaSession *) (fServerMediaSessions->Lookup(streamName));
}

void RTSPServer::removeServerMediaSession(ServerMediaSession* serverMediaSession)
{
    if (serverMediaSession == NULL) {
        return;
    }
    fServerMediaSessions->Remove(serverMediaSession->streamName());
    if (serverMediaSession->referenceCount() == 0) {
        Medium::close(serverMediaSession);
    } else {
        serverMediaSession->deleteWhenUnreferenced() = True;
    }
}

void RTSPServer::removeServerMediaSession(char const* streamName)
{
    removeServerMediaSession((ServerMediaSession *) (fServerMediaSessions->Lookup(streamName)));
}

void RTSPServer::closeAllClientSessionsForServerMediaSession(ServerMediaSession* serverMediaSession)
{
    if (serverMediaSession == NULL) {
        return;
    }
    HashTable::Iterator* iter = HashTable::Iterator::create(*fClientSessions);
    RTSPServer::RTSPClientSession* clientSession;
    char const* key;
    while ((clientSession = (RTSPServer::RTSPClientSession *) (iter->next(key))) != NULL) {
        if (clientSession->fOurServerMediaSession == serverMediaSession) {
            delete clientSession;
        }
    }
    delete iter;
}

void RTSPServer::closeAllClientSessionsForServerMediaSession(char const* streamName)
{
    closeAllClientSessionsForServerMediaSession((ServerMediaSession *) (fServerMediaSessions->Lookup(streamName)));
}

void RTSPServer::deleteServerMediaSession(ServerMediaSession* serverMediaSession)
{
    if (serverMediaSession == NULL) {
        return;
    }
    closeAllClientSessionsForServerMediaSession(serverMediaSession);
    removeServerMediaSession(serverMediaSession);
}

void RTSPServer::deleteServerMediaSession(char const* streamName)
{
    deleteServerMediaSession((ServerMediaSession *) (fServerMediaSessions->Lookup(streamName)));
}

void rtspRegisterResponseHandler(RTSPClient* rtspClient, int resultCode, char* resultString);
class RegisterRequestRecord : public RTSPRegisterSender
{
public:
    RegisterRequestRecord(RTSPServer& ourServer, unsigned requestId,
                          char const* remoteClientNameOrAddress, portNumBits remoteClientPortNum,
                          char const* rtspURLToRegister,
                          RTSPServer::responseHandlerForREGISTER* responseHandler, Authenticator* authenticator,
                          Boolean requestStreamingViaTCP, char const* proxyURLSuffix)
        : RTSPRegisterSender(ourServer.envir(), remoteClientNameOrAddress, remoteClientPortNum, rtspURLToRegister,
                             rtspRegisterResponseHandler, authenticator,
                             requestStreamingViaTCP, proxyURLSuffix, True,
                             #ifdef DEBUG
                             1,
                             #else
                             0,
                             #endif
                             NULL),
        fOurServer(ourServer), fRequestId(requestId), fResponseHandler(responseHandler)
    {
        ourServer.fPendingRegisterRequests->Add((char const *) this, this);
    }

    virtual ~RegisterRequestRecord()
    {
        fOurServer.fPendingRegisterRequests->Remove((char const *) this);
    }

    void handleResponse(int resultCode, char* resultString)
    {
        if (resultCode == 0) {
            int sock;
            struct sockaddr_in remoteAddress;
            grabConnection(sock, remoteAddress);
            if (sock >= 0) {
                (void) fOurServer.createNewClientConnection(sock, remoteAddress);
            }
        }
        if (fResponseHandler != NULL) {
            (*fResponseHandler)(&fOurServer, fRequestId, resultCode, resultString);
        } else {
            delete[] resultString;
        }
        delete this;
    }

private:
    RTSPServer& fOurServer;
    unsigned fRequestId;
    RTSPServer::responseHandlerForREGISTER* fResponseHandler;
};
void rtspRegisterResponseHandler(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    RegisterRequestRecord* registerRequestRecord = (RegisterRequestRecord *) rtspClient;

    registerRequestRecord->handleResponse(resultCode, resultString);
}

unsigned RTSPServer::registerStream(ServerMediaSession* serverMediaSession,
                                    char const* remoteClientNameOrAddress, portNumBits remoteClientPortNum,
                                    responseHandlerForREGISTER* responseHandler,
                                    char const* username, char const* password,
                                    Boolean receiveOurStreamViaTCP, char const* proxyURLSuffix)
{
    Authenticator* authenticator = NULL;

    if (username != NULL) {
        if (password == NULL) {
            password = "";
        }
        authenticator = new Authenticator(username, password);
    }
    unsigned requestId = ++fRegisterRequestCounter;
    new RegisterRequestRecord(*this, requestId,
                              remoteClientNameOrAddress, remoteClientPortNum, rtspURL(serverMediaSession),
                              responseHandler, authenticator,
                              receiveOurStreamViaTCP, proxyURLSuffix);
    delete authenticator;
    return requestId;
}

char * RTSPServer
::rtspURL(ServerMediaSession const* serverMediaSession, int clientSocket) const
{
    char* urlPrefix         = rtspURLPrefix(clientSocket);
    char const* sessionName = serverMediaSession->streamName();
    char* resultURL         = new char[strlen(urlPrefix) + strlen(sessionName) + 1];

    sprintf(resultURL, "%s%s", urlPrefix, sessionName);
    delete[] urlPrefix;
    return resultURL;
}

char * RTSPServer::rtspURLPrefix(int clientSocket) const
{
    struct sockaddr_in ourAddress;

    if (clientSocket < 0) {
        ourAddress.sin_addr.s_addr = ReceivingInterfaceAddr != 0 ?
                                     ReceivingInterfaceAddr :
                                     ourIPAddress(envir());
    } else {
        SOCKLEN_T namelen = sizeof ourAddress;
        getsockname(clientSocket, (struct sockaddr *) &ourAddress, &namelen);
    }
    char urlBuffer[100];
    portNumBits portNumHostOrder = ntohs(fRTSPServerPort.num());
    if (portNumHostOrder == 554) {
        sprintf(urlBuffer, "rtsp://%s/", AddressString(ourAddress).val());
    } else {
        sprintf(urlBuffer, "rtsp://%s:%hu/",
                AddressString(ourAddress).val(), portNumHostOrder);
    }
    return strDup(urlBuffer);
}

UserAuthenticationDatabase * RTSPServer::setAuthenticationDatabase(UserAuthenticationDatabase* newDB)
{
    UserAuthenticationDatabase* oldDB = fAuthDB;

    fAuthDB = newDB;
    return oldDB;
}

Boolean RTSPServer::setUpTunnelingOverHTTP(Port httpPort)
{
    fHTTPServerSocket = setUpOurSocket(envir(), httpPort);
    if (fHTTPServerSocket >= 0) {
        fHTTPServerPort = httpPort;
        envir().taskScheduler().turnOnBackgroundReadHandling(fHTTPServerSocket,
                                                             (TaskScheduler::BackgroundHandlerProc *) &incomingConnectionHandlerHTTP,
                                                             this);
        return True;
    }
    return False;
}

portNumBits RTSPServer::httpServerPortNum() const
{
    return ntohs(fHTTPServerPort.num());
}

#define LISTEN_BACKLOG_SIZE 20
int RTSPServer::setUpOurSocket(UsageEnvironment& env, Port& ourPort)
{
    int ourSocket = -1;

    do {
        #ifndef ALLOW_RTSP_SERVER_PORT_REUSE
        NoReuse dummy(env);
        #endif
        ourSocket = setupStreamSocket(env, ourPort);
        if (ourSocket < 0) {
            break;
        }
        if (!increaseSendBufferTo(env, ourSocket, 50 * 1024)) {
            break;
        }
        if (listen(ourSocket, LISTEN_BACKLOG_SIZE) < 0) {
            env.setResultErrMsg("listen() failed: ");
            break;
        }
        if (ourPort.num() == 0) {
            if (!getSourcePort(env, ourSocket, ourPort)) {
                break;
            }
        }
        return ourSocket;
    } while (0);
    if (ourSocket != -1) {
        ::closeSocket(ourSocket);
    }
    return -1;
}

char const * RTSPServer::allowedCommandNames()
{
    return "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER";
}

Boolean RTSPServer::weImplementREGISTER(char const *, char *& responseStr)
{
    responseStr = NULL;
    return False;
}

void RTSPServer::implementCmd_REGISTER(char const *, char const *, int,
                                       Boolean, char const *)
{}

UserAuthenticationDatabase * RTSPServer::getAuthenticationDatabaseForCommand(char const *)
{
    return fAuthDB;
}

Boolean RTSPServer::specialClientAccessCheck(int, struct sockaddr_in&, char const *)
{
    return True;
}

Boolean RTSPServer::specialClientUserAccessCheck(int, struct sockaddr_in&,
                                                 char const *, char const *)
{
    return True;
}

RTSPServer::RTSPServer(UsageEnvironment& env,
                       int ourSocket, Port ourPort,
                       UserAuthenticationDatabase* authDatabase,
                       unsigned reclamationTestSeconds)
    : Medium(env),
    fRTSPServerPort(ourPort), fRTSPServerSocket(ourSocket), fHTTPServerSocket(-1), fHTTPServerPort(0),
    fServerMediaSessions(HashTable::create(STRING_HASH_KEYS)),
    fClientConnections(HashTable::create(ONE_WORD_HASH_KEYS)),
    fClientConnectionsForHTTPTunneling(NULL),
    fClientSessions(HashTable::create(STRING_HASH_KEYS)),
    fPendingRegisterRequests(HashTable::create(ONE_WORD_HASH_KEYS)), fRegisterRequestCounter(0),
    fAuthDB(authDatabase), fReclamationTestSeconds(reclamationTestSeconds),
    fAllowStreamingRTPOverTCP(True)
{
    ignoreSigPipeOnSocket(ourSocket);
    env.taskScheduler().turnOnBackgroundReadHandling(fRTSPServerSocket,
                                                     (TaskScheduler::BackgroundHandlerProc *) &incomingConnectionHandlerRTSP,
                                                     this);
}

RTSPServer::~RTSPServer()
{
    envir().taskScheduler().turnOffBackgroundReadHandling(fRTSPServerSocket);
    ::closeSocket(fRTSPServerSocket);
    envir().taskScheduler().turnOffBackgroundReadHandling(fHTTPServerSocket);
    ::closeSocket(fHTTPServerSocket);
    RTSPServer::RTSPClientSession* clientSession;
    while ((clientSession = (RTSPServer::RTSPClientSession *) fClientSessions->getFirst()) != NULL) {
        delete clientSession;
    }
    delete fClientSessions;
    RTSPServer::RTSPClientConnection* connection;
    while ((connection = (RTSPServer::RTSPClientConnection *) fClientConnections->getFirst()) != NULL) {
        delete connection;
    }
    delete fClientConnections;
    delete fClientConnectionsForHTTPTunneling;
    ServerMediaSession* serverMediaSession;
    while ((serverMediaSession = (ServerMediaSession *) fServerMediaSessions->getFirst()) != NULL) {
        removeServerMediaSession(serverMediaSession);
    }
    delete fServerMediaSessions;
    RegisterRequestRecord* registerRequest;
    while ((registerRequest = (RegisterRequestRecord *) fPendingRegisterRequests->getFirst()) != NULL) {
        delete registerRequest;
    }
    delete fPendingRegisterRequests;
}

Boolean RTSPServer::isRTSPServer() const
{
    return True;
}

void RTSPServer::incomingConnectionHandlerRTSP(void* instance, int)
{
    RTSPServer* server = (RTSPServer *) instance;

    server->incomingConnectionHandlerRTSP1();
}

void RTSPServer::incomingConnectionHandlerRTSP1()
{
    incomingConnectionHandler(fRTSPServerSocket);
}

void RTSPServer::incomingConnectionHandlerHTTP(void* instance, int)
{
    RTSPServer* server = (RTSPServer *) instance;

    server->incomingConnectionHandlerHTTP1();
}

void RTSPServer::incomingConnectionHandlerHTTP1()
{
    incomingConnectionHandler(fHTTPServerSocket);
}

void RTSPServer::incomingConnectionHandler(int serverSocket)
{
    struct sockaddr_in clientAddr;
    SOCKLEN_T clientAddrLen = sizeof clientAddr;
    int clientSocket        = accept(serverSocket, (struct sockaddr *) &clientAddr, &clientAddrLen);

    if (clientSocket < 0) {
        int err = envir().getErrno();
        if (err != EWOULDBLOCK) {
            envir().setResultErrMsg("accept() failed: ");
        }
        return;
    }
    makeSocketNonBlocking(clientSocket);
    increaseSendBufferTo(envir(), clientSocket, 50 * 1024);
    #ifdef DEBUG
    envir() << "accept()ed connection from " << AddressString(clientAddr).val() << "\n";
    #endif
    (void) createNewClientConnection(clientSocket, clientAddr);
}

RTSPServer::RTSPClientConnection
::RTSPClientConnection(RTSPServer& ourServer, int clientSocket, struct sockaddr_in clientAddr)
    : fOurServer(ourServer), fIsActive(True),
    fClientInputSocket(clientSocket), fClientOutputSocket(clientSocket), fClientAddr(clientAddr),
    fRecursionCount(0), fOurSessionCookie(NULL)
{
    fOurServer.fClientConnections->Add((char const *) this, this);
    resetRequestBuffer();
    envir().taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE | SOCKET_EXCEPTION,
                                                  (TaskScheduler::BackgroundHandlerProc *) &incomingRequestHandler,
                                                  this);
}

RTSPServer::RTSPClientConnection::~RTSPClientConnection()
{
    fOurServer.fClientConnections->Remove((char const *) this);
    if (fOurSessionCookie != NULL) {
        fOurServer.fClientConnectionsForHTTPTunneling->Remove(fOurSessionCookie);
        delete[] fOurSessionCookie;
    }
    closeSockets();
}

RTSPServer::RTSPClientConnection::ParamsForREGISTER
::ParamsForREGISTER(RTSPServer::RTSPClientConnection* ourConnection, char const* url, char const* urlSuffix,
                    Boolean reuseConnection, Boolean deliverViaTCP, char const* proxyURLSuffix)
    : fOurConnection(ourConnection), fURL(strDup(url)), fURLSuffix(strDup(urlSuffix)),
    fReuseConnection(reuseConnection), fDeliverViaTCP(deliverViaTCP), fProxyURLSuffix(strDup(proxyURLSuffix))
{}

RTSPServer::RTSPClientConnection::ParamsForREGISTER::~ParamsForREGISTER()
{
    delete[] fURL;
    delete[] fURLSuffix;
    delete[] fProxyURLSuffix;
}

void RTSPServer::RTSPClientConnection::handleCmd_OPTIONS()
{
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sPublic: %s\r\n\r\n",
             fCurrentCSeq, dateHeader(), fOurServer.allowedCommandNames());
}

void RTSPServer::RTSPClientConnection
::handleCmd_GET_PARAMETER(char const *)
{
    setRTSPResponse("200 OK", LIVEMEDIA_LIBRARY_VERSION_STRING);
}

void RTSPServer::RTSPClientConnection
::handleCmd_SET_PARAMETER(char const *)
{
    setRTSPResponse("200 OK");
}

void RTSPServer::RTSPClientConnection
::handleCmd_DESCRIBE(char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr)
{
    char* sdpDescription = NULL;
    char* rtspURL        = NULL;

    do {
        char urlTotalSuffix[RTSP_PARAM_STRING_MAX];
        if (strlen(urlPreSuffix) + strlen(urlSuffix) + 2 > sizeof urlTotalSuffix) {
            handleCmd_bad();
            break;
        }
        urlTotalSuffix[0] = '\0';
        if (urlPreSuffix[0] != '\0') {
            strcat(urlTotalSuffix, urlPreSuffix);
            strcat(urlTotalSuffix, "/");
        }
        strcat(urlTotalSuffix, urlSuffix);
        if (!authenticationOK("DESCRIBE", urlTotalSuffix, fullRequestStr)) {
            break;
        }
        ServerMediaSession* session = fOurServer.lookupServerMediaSession(urlTotalSuffix);
        if (session == NULL) {
            handleCmd_notFound();
            break;
        }
        sdpDescription = session->generateSDPDescription();
        if (sdpDescription == NULL) {
            setRTSPResponse("404 File Not Found, Or In Incorrect Format");
            break;
        }
        unsigned sdpDescriptionSize = strlen(sdpDescription);
        rtspURL = fOurServer.rtspURL(session, fClientInputSocket);
        snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
                 "RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
                 "%s"
                 "Content-Base: %s/\r\n"
                 "Content-Type: application/sdp\r\n"
                 "Content-Length: %d\r\n\r\n"
                 "%s",
                 fCurrentCSeq,
                 dateHeader(),
                 rtspURL,
                 sdpDescriptionSize,
                 sdpDescription);
    } while (0);
    delete[] sdpDescription;
    delete[] rtspURL;
} // RTSPServer::RTSPClientConnection

static void lookForHeader(char const* headerName, char const* source, unsigned sourceLen, char* resultStr,
                          unsigned resultMaxSize)
{
    resultStr[0] = '\0';
    unsigned headerNameLen = strlen(headerName);
    for (int i = 0; i < (int) (sourceLen - headerNameLen); ++i) {
        if (strncmp(&source[i], headerName, headerNameLen) == 0 && source[i + headerNameLen] == ':') {
            for (i += headerNameLen + 1; i < (int) sourceLen && (source[i] == ' ' || source[i] == '\t'); ++i) {}
            for (unsigned j = i; j < sourceLen; ++j) {
                if (source[j] == '\r' || source[j] == '\n') {
                    if (j - i + 1 > resultMaxSize) {
                        break;
                    }
                    char const* resultSource    = &source[i];
                    char const* resultSourceEnd = &source[j];
                    while (resultSource < resultSourceEnd) {
                        *resultStr++ = *resultSource++;
                    }
                    *resultStr = '\0';
                    break;
                }
            }
        }
    }
}

void RTSPServer
::RTSPClientConnection::handleCmd_REGISTER(char const* url, char const* urlSuffix, char const* fullRequestStr,
                                           Boolean reuseConnection, Boolean deliverViaTCP, char const* proxyURLSuffix)
{
    char* responseStr;

    if (fOurServer.weImplementREGISTER(proxyURLSuffix, responseStr)) {
        if (!authenticationOK("REGISTER", urlSuffix, fullRequestStr)) {
            return;
        }
        setRTSPResponse(responseStr == NULL ? "200 OK" : responseStr);
        delete[] responseStr;
        ParamsForREGISTER* registerParams = new ParamsForREGISTER(this, url, urlSuffix, reuseConnection, deliverViaTCP,
                                                                  proxyURLSuffix);
        envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc *) continueHandlingREGISTER, registerParams);
    } else if (responseStr != NULL) {
        setRTSPResponse(responseStr);
        delete[] responseStr;
    } else {
        handleCmd_notSupported();
    }
}

void RTSPServer::RTSPClientConnection::handleCmd_bad()
{
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "RTSP/1.0 400 Bad Request\r\n%sAllow: %s\r\n\r\n",
             dateHeader(), fOurServer.allowedCommandNames());
}

void RTSPServer::RTSPClientConnection::handleCmd_notSupported()
{
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "RTSP/1.0 405 Method Not Allowed\r\nCSeq: %s\r\n%sAllow: %s\r\n\r\n",
             fCurrentCSeq, dateHeader(), fOurServer.allowedCommandNames());
}

void RTSPServer::RTSPClientConnection::handleCmd_notFound()
{
    setRTSPResponse("404 Stream Not Found");
}

void RTSPServer::RTSPClientConnection::handleCmd_sessionNotFound()
{
    setRTSPResponse("454 Session Not Found");
}

void RTSPServer::RTSPClientConnection::handleCmd_unsupportedTransport()
{
    setRTSPResponse("461 Unsupported Transport");
}

Boolean RTSPServer::RTSPClientConnection::parseHTTPRequestString(char* resultCmdName, unsigned resultCmdNameMaxSize,
                                                                 char* urlSuffix, unsigned urlSuffixMaxSize,
                                                                 char* sessionCookie, unsigned sessionCookieMaxSize,
                                                                 char* acceptStr, unsigned acceptStrMaxSize)
{
    char const* reqStr        = (char const *) fRequestBuffer;
    unsigned const reqStrSize = fRequestBytesAlreadySeen;
    Boolean parseSucceeded    = False;
    unsigned i;

    for (i = 0; i < resultCmdNameMaxSize - 1 && i < reqStrSize; ++i) {
        char c = reqStr[i];
        if (c == ' ' || c == '\t') {
            parseSucceeded = True;
            break;
        }
        resultCmdName[i] = c;
    }
    resultCmdName[i] = '\0';
    if (!parseSucceeded) {
        return False;
    }
    parseSucceeded = False;
    for (; i < reqStrSize - 5 && reqStr[i] != '\r' && reqStr[i] != '\n'; ++i) {
        if (reqStr[i] == 'H' && reqStr[i + 1] == 'T' && reqStr[i + 2] == 'T' && reqStr[i + 3] == 'P' &&
            reqStr[i + 4] == '/')
        {
            i += 5;
            parseSucceeded = True;
            break;
        }
    }
    if (!parseSucceeded) {
        return False;
    }
    unsigned k = i - 6;
    while (k > 0 && reqStr[k] == ' ') {
        --k;
    }
    unsigned j = k;
    while (j > 0 && reqStr[j] != ' ' && reqStr[j] != '/') {
        --j;
    }
    if (k - j + 1 > urlSuffixMaxSize) {
        return False;
    }
    unsigned n = 0;
    while (++j <= k) {
        urlSuffix[n++] = reqStr[j];
    }
    urlSuffix[n] = '\0';
    lookForHeader("x-sessioncookie", &reqStr[i], reqStrSize - i, sessionCookie, sessionCookieMaxSize);
    lookForHeader("Accept", &reqStr[i], reqStrSize - i, acceptStr, acceptStrMaxSize);
    return True;
} // RTSPServer::RTSPClientConnection

void RTSPServer::RTSPClientConnection::handleHTTPCmd_notSupported()
{
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "HTTP/1.1 405 Method Not Allowed\r\n%s\r\n\r\n",
             dateHeader());
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_notFound()
{
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "HTTP/1.1 404 Not Found\r\n%s\r\n\r\n",
             dateHeader());
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_OPTIONS()
{
    #ifdef DEBUG
    fprintf(stderr, "Handled HTTP \"OPTIONS\" request\n");
    #endif
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "HTTP/1.1 200 OK\r\n"
             "%s"
             "Access-Control-Allow-Origin: *\r\n"
             "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
             "Access-Control-Allow-Headers: x-sessioncookie, Pragma, Cache-Control\r\n"
             "Access-Control-Max-Age: 1728000\r\n"
             "\r\n",
             dateHeader());
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_TunnelingGET(char const* sessionCookie)
{
    if (fOurServer.fClientConnectionsForHTTPTunneling == NULL) {
        fOurServer.fClientConnectionsForHTTPTunneling = HashTable::create(STRING_HASH_KEYS);
    }
    delete[] fOurSessionCookie;
    fOurSessionCookie = strDup(sessionCookie);
    fOurServer.fClientConnectionsForHTTPTunneling->Add(sessionCookie, (void *) this);
    #ifdef DEBUG
    fprintf(stderr, "Handled HTTP \"GET\" request (client output socket: %d)\n", fClientOutputSocket);
    #endif
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "HTTP/1.1 200 OK\r\n"
             "%s"
             "Cache-Control: no-cache\r\n"
             "Pragma: no-cache\r\n"
             "Content-Type: application/x-rtsp-tunnelled\r\n"
             "\r\n",
             dateHeader());
}

Boolean RTSPServer::RTSPClientConnection
::handleHTTPCmd_TunnelingPOST(char const* sessionCookie, unsigned char const* extraData, unsigned extraDataSize)
{
    if (fOurServer.fClientConnectionsForHTTPTunneling == NULL) {
        fOurServer.fClientConnectionsForHTTPTunneling = HashTable::create(STRING_HASH_KEYS);
    }
    RTSPServer::RTSPClientConnection* prevClientConnection =
        (RTSPServer::RTSPClientConnection *) (fOurServer.fClientConnectionsForHTTPTunneling->Lookup(sessionCookie));
    if (prevClientConnection == NULL) {
        handleHTTPCmd_notSupported();
        fIsActive = False;
        return False;
    }
    #ifdef DEBUG
    fprintf(stderr, "Handled HTTP \"POST\" request (client input socket: %d)\n", fClientInputSocket);
    #endif
    prevClientConnection->changeClientInputSocket(fClientInputSocket, extraData, extraDataSize);
    fClientInputSocket = fClientOutputSocket = -1;
    return True;
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_StreamingGET(char const *, char const *)
{
    handleHTTPCmd_notSupported();
}

void RTSPServer::RTSPClientConnection::resetRequestBuffer()
{
    fRequestBytesAlreadySeen = 0;
    fRequestBufferBytesLeft  = sizeof fRequestBuffer;
    fLastCRLF = &fRequestBuffer[-3];
    fBase64RemainderCount = 0;
}

void RTSPServer::RTSPClientConnection::closeSockets()
{
    if (fClientOutputSocket != fClientInputSocket) {
        envir().taskScheduler().disableBackgroundHandling(fClientOutputSocket);
        ::closeSocket(fClientOutputSocket);
    }
    envir().taskScheduler().disableBackgroundHandling(fClientInputSocket);
    ::closeSocket(fClientInputSocket);
    fClientInputSocket = fClientOutputSocket = -1;
}

void RTSPServer::RTSPClientConnection::incomingRequestHandler(void* instance, int)
{
    RTSPClientConnection* session = (RTSPClientConnection *) instance;

    session->incomingRequestHandler1();
}

void RTSPServer::RTSPClientConnection::incomingRequestHandler1()
{
    struct sockaddr_in dummy;
    int bytesRead = readSocket(
        envir(), fClientInputSocket, &fRequestBuffer[fRequestBytesAlreadySeen], fRequestBufferBytesLeft, dummy);

    handleRequestBytes(bytesRead);
}

void RTSPServer::RTSPClientConnection::handleAlternativeRequestByte(void* instance, u_int8_t requestByte)
{
    RTSPClientConnection* session = (RTSPClientConnection *) instance;

    session->handleAlternativeRequestByte1(requestByte);
}

void RTSPServer::RTSPClientConnection::handleAlternativeRequestByte1(u_int8_t requestByte)
{
    if (requestByte == 0xFF) {
        handleRequestBytes(-1);
    } else if (requestByte == 0xFE) {
        envir().taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE | SOCKET_EXCEPTION,
                                                      (TaskScheduler::BackgroundHandlerProc *) &incomingRequestHandler,
                                                      this);
    } else {
        if (fRequestBufferBytesLeft == 0 || fRequestBytesAlreadySeen >= RTSP_BUFFER_SIZE) {
            return;
        }
        fRequestBuffer[fRequestBytesAlreadySeen] = requestByte;
        handleRequestBytes(1);
    }
}

static void parseTransportHeaderForREGISTER(char const* buf,
                                            Boolean     & reuseConnection,
                                            Boolean     & deliverViaTCP,
                                            char *      & proxyURLSuffix)
{
    reuseConnection = False;
    deliverViaTCP   = False;
    proxyURLSuffix  = NULL;
    while (1) {
        if (*buf == '\0') {
            return;
        }
        if (*buf == '\r' && *(buf + 1) == '\n' && *(buf + 2) == '\r') {
            return;
        }
        if (_strncasecmp(buf, "Transport:", 10) == 0) {
            break;
        }
        ++buf;
    }
    char const* fields = buf + 10;
    while (*fields == ' ') {
        ++fields;
    }
    char* field = strDupSize(fields);
    while (sscanf(fields, "%[^;\r\n]", field) == 1) {
        if (strcmp(field, "reuse_connection") == 0) {
            reuseConnection = True;
        } else if (_strncasecmp(field, "preferred_delivery_protocol=udp", 31) == 0) {
            deliverViaTCP = False;
        } else if (_strncasecmp(field, "preferred_delivery_protocol=interleaved", 39) == 0) {
            deliverViaTCP = True;
        } else if (_strncasecmp(field, "proxy_url_suffix=", 17) == 0) {
            delete[] proxyURLSuffix;
            proxyURLSuffix = strDup(field + 17);
        }
        fields += strlen(field);
        while (*fields == ';' || *fields == ' ' || *fields == '\t') {
            ++fields;
        }
        if (*fields == '\0' || *fields == '\r' || *fields == '\n') {
            break;
        }
    }
    delete[] field;
} // parseTransportHeaderForREGISTER

void RTSPServer::RTSPClientConnection::handleRequestBytes(int newBytesRead)
{
    int numBytesRemaining = 0;

    ++fRecursionCount;
    do {
        RTSPServer::RTSPClientSession* clientSession = NULL;
        if (newBytesRead < 0 || (unsigned) newBytesRead >= fRequestBufferBytesLeft) {
            #ifdef DEBUG
            fprintf(stderr,
                    "RTSPClientConnection[%p]::handleRequestBytes() read %d new bytes (of %d); terminating connection!\n", this, newBytesRead,
                    fRequestBufferBytesLeft);
            #endif
            fIsActive = False;
            break;
        }
        Boolean endOfMsg   = False;
        unsigned char* ptr = &fRequestBuffer[fRequestBytesAlreadySeen];
        #ifdef DEBUG
        ptr[newBytesRead] = '\0';
        fprintf(stderr, "RTSPClientConnection[%p]::handleRequestBytes() %s %d new bytes:%s\n",
                this, numBytesRemaining > 0 ? "processing" : "read", newBytesRead, ptr);
        #endif
        if (fClientOutputSocket != fClientInputSocket && numBytesRemaining == 0) {
            unsigned toIndex = 0;
            for (int fromIndex = 0; fromIndex < newBytesRead; ++fromIndex) {
                char c = ptr[fromIndex];
                if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
                    ptr[toIndex++] = c;
                }
            }
            newBytesRead = toIndex;
            unsigned numBytesToDecode        = fBase64RemainderCount + newBytesRead;
            unsigned newBase64RemainderCount = numBytesToDecode % 4;
            numBytesToDecode -= newBase64RemainderCount;
            if (numBytesToDecode > 0) {
                ptr[newBytesRead] = '\0';
                unsigned decodedSize;
                unsigned char* decodedBytes = base64Decode((char const *) (ptr - fBase64RemainderCount),
                                                           numBytesToDecode, decodedSize);
                #ifdef DEBUG
                fprintf(stderr, "Base64-decoded %d input bytes into %d new bytes:", numBytesToDecode, decodedSize);
                for (unsigned k = 0; k < decodedSize; ++k) {
                    fprintf(stderr, "%c", decodedBytes[k]);
                }
                fprintf(stderr, "\n");
                #endif
                unsigned char* to = ptr - fBase64RemainderCount;
                for (unsigned i = 0; i < decodedSize; ++i) {
                    *to++ = decodedBytes[i];
                }
                for (unsigned j = 0; j < newBase64RemainderCount; ++j) {
                    *to++ = (ptr - fBase64RemainderCount + numBytesToDecode)[j];
                }
                newBytesRead = decodedSize - fBase64RemainderCount + newBase64RemainderCount;
                delete[] decodedBytes;
            }
            fBase64RemainderCount = newBase64RemainderCount;
        }
        unsigned char* tmpPtr = fLastCRLF + 2;
        if (fBase64RemainderCount == 0) {
            if (tmpPtr < fRequestBuffer) {
                tmpPtr = fRequestBuffer;
            }
            while (tmpPtr < &ptr[newBytesRead - 1]) {
                if (*tmpPtr == '\r' && *(tmpPtr + 1) == '\n') {
                    if (tmpPtr - fLastCRLF == 2) {
                        endOfMsg = True;
                        break;
                    }
                    fLastCRLF = tmpPtr;
                }
                ++tmpPtr;
            }
        }
        fRequestBufferBytesLeft  -= newBytesRead;
        fRequestBytesAlreadySeen += newBytesRead;
        if (!endOfMsg) {
            break;
        }
        fRequestBuffer[fRequestBytesAlreadySeen] = '\0';
        char cmdName[RTSP_PARAM_STRING_MAX];
        char urlPreSuffix[RTSP_PARAM_STRING_MAX];
        char urlSuffix[RTSP_PARAM_STRING_MAX];
        char cseq[RTSP_PARAM_STRING_MAX];
        char sessionIdStr[RTSP_PARAM_STRING_MAX];
        unsigned contentLength = 0;
        fLastCRLF[2] = '\0';
        Boolean parseSucceeded = parseRTSPRequestString((char *) fRequestBuffer, fLastCRLF + 2 - fRequestBuffer,
                                                        cmdName, sizeof cmdName,
                                                        urlPreSuffix, sizeof urlPreSuffix,
                                                        urlSuffix, sizeof urlSuffix,
                                                        cseq, sizeof cseq,
                                                        sessionIdStr, sizeof sessionIdStr,
                                                        contentLength);
        fLastCRLF[2] = '\r';
        Boolean playAfterSetup = False;
        if (parseSucceeded) {
            #ifdef DEBUG
            fprintf(stderr,
                    "parseRTSPRequestString() succeeded, returning cmdName \"%s\", urlPreSuffix \"%s\", urlSuffix \"%s\", CSeq \"%s\", Content-Length %u, with %ld bytes following the message.\n", cmdName, urlPreSuffix, urlSuffix, cseq, contentLength,
                    ptr + newBytesRead - (tmpPtr + 2));
            #endif
            if (ptr + newBytesRead < tmpPtr + 2 + contentLength) {
                break;
            }
            Boolean const requestIncludedSessionId = sessionIdStr[0] != '\0';
            if (requestIncludedSessionId) {
                clientSession = (RTSPServer::RTSPClientSession *) (fOurServer.fClientSessions->Lookup(sessionIdStr));
                if (clientSession != NULL) {
                    clientSession->noteLiveness();
                }
            }
            fCurrentCSeq = cseq;
            if (strcmp(cmdName, "OPTIONS") == 0) {
                if (requestIncludedSessionId && clientSession == NULL) {
                    handleCmd_sessionNotFound();
                } else {
                    handleCmd_OPTIONS();
                }
            } else if (urlPreSuffix[0] == '\0' && urlSuffix[0] == '*' && urlSuffix[1] == '\0') {
                if (strcmp(cmdName, "GET_PARAMETER") == 0) {
                    handleCmd_GET_PARAMETER((char const *) fRequestBuffer);
                } else if (strcmp(cmdName, "SET_PARAMETER") == 0) {
                    handleCmd_SET_PARAMETER((char const *) fRequestBuffer);
                } else {
                    handleCmd_notSupported();
                }
            } else if (strcmp(cmdName, "DESCRIBE") == 0) {
                handleCmd_DESCRIBE(urlPreSuffix, urlSuffix, (char const *) fRequestBuffer);
            } else if (strcmp(cmdName, "SETUP") == 0) {
                if (!requestIncludedSessionId) {
                    u_int32_t sessionId;
                    do {
                        sessionId = (u_int32_t) our_random32();
                        sprintf(sessionIdStr, "%08X", sessionId);
                    } while (sessionId == 0 || fOurServer.fClientSessions->Lookup(sessionIdStr) != NULL);
                    clientSession = fOurServer.createNewClientSession(sessionId);
                    fOurServer.fClientSessions->Add(sessionIdStr, clientSession);
                }
                if (clientSession != NULL) {
                    clientSession->handleCmd_SETUP(this, urlPreSuffix, urlSuffix, (char const *) fRequestBuffer);
                    playAfterSetup = clientSession->fStreamAfterSETUP;
                } else {
                    handleCmd_sessionNotFound();
                }
            } else if (strcmp(cmdName, "TEARDOWN") == 0 ||
                       strcmp(cmdName, "PLAY") == 0 ||
                       strcmp(cmdName, "PAUSE") == 0 ||
                       strcmp(cmdName, "GET_PARAMETER") == 0 ||
                       strcmp(cmdName, "SET_PARAMETER") == 0)
            {
                if (clientSession != NULL) {
                    clientSession->handleCmd_withinSession(this, cmdName, urlPreSuffix, urlSuffix,
                                                           (char const *) fRequestBuffer);
                } else {
                    handleCmd_sessionNotFound();
                }
            } else if (strcmp(cmdName, "REGISTER") == 0) {
                char* url = strDupSize((char *) fRequestBuffer);
                if (sscanf((char *) fRequestBuffer, "%*s %s", url) == 1) {
                    Boolean reuseConnection, deliverViaTCP;
                    char* proxyURLSuffix;
                    parseTransportHeaderForREGISTER((const char *) fRequestBuffer, reuseConnection, deliverViaTCP,
                                                    proxyURLSuffix);
                    handleCmd_REGISTER(url, urlSuffix, (char const *) fRequestBuffer, reuseConnection, deliverViaTCP,
                                       proxyURLSuffix);
                    delete[] proxyURLSuffix;
                } else {
                    handleCmd_bad();
                }
                delete[] url;
            } else {
                handleCmd_notSupported();
            }
        } else {
            #ifdef DEBUG
            fprintf(stderr,
                    "parseRTSPRequestString() failed; checking now for HTTP commands (for RTSP-over-HTTP tunneling)...\n");
            #endif
            char sessionCookie[RTSP_PARAM_STRING_MAX];
            char acceptStr[RTSP_PARAM_STRING_MAX];
            *fLastCRLF     = '\0';
            parseSucceeded = parseHTTPRequestString(cmdName, sizeof cmdName,
                                                    urlSuffix, sizeof urlPreSuffix,
                                                    sessionCookie, sizeof sessionCookie,
                                                    acceptStr, sizeof acceptStr);
            *fLastCRLF = '\r';
            if (parseSucceeded) {
                #ifdef DEBUG
                fprintf(stderr,
                        "parseHTTPRequestString() succeeded, returning cmdName \"%s\", urlSuffix \"%s\", sessionCookie \"%s\", acceptStr \"%s\"\n", cmdName, urlSuffix, sessionCookie,
                        acceptStr);
                #endif
                Boolean isValidHTTPCmd = True;
                if (strcmp(cmdName, "OPTIONS") == 0) {
                    handleHTTPCmd_OPTIONS();
                } else if (sessionCookie[0] == '\0') {
                    if (strcmp(acceptStr, "application/x-rtsp-tunnelled") == 0) {
                        isValidHTTPCmd = False;
                    } else {
                        handleHTTPCmd_StreamingGET(urlSuffix, (char const *) fRequestBuffer);
                    }
                } else if (strcmp(cmdName, "GET") == 0) {
                    handleHTTPCmd_TunnelingGET(sessionCookie);
                } else if (strcmp(cmdName, "POST") == 0) {
                    unsigned char const* extraData = fLastCRLF + 4;
                    unsigned extraDataSize         = &fRequestBuffer[fRequestBytesAlreadySeen] - extraData;
                    if (handleHTTPCmd_TunnelingPOST(sessionCookie, extraData, extraDataSize)) {
                        fIsActive = False;
                        break;
                    }
                } else {
                    isValidHTTPCmd = False;
                }
                if (!isValidHTTPCmd) {
                    handleHTTPCmd_notSupported();
                }
            } else {
                #ifdef DEBUG
                fprintf(stderr, "parseHTTPRequestString() failed!\n");
                #endif
                handleCmd_bad();
            }
        }
        #ifdef DEBUG
        fprintf(stderr, "sending response: %s", fResponseBuffer);
        #endif
        send(fClientOutputSocket, (char const *) fResponseBuffer, strlen((char *) fResponseBuffer), 0);
        if (playAfterSetup) {
            clientSession->handleCmd_withinSession(this, "PLAY", urlPreSuffix, urlSuffix,
                                                   (char const *) fRequestBuffer);
        }
        unsigned requestSize = (fLastCRLF + 4 - fRequestBuffer) + contentLength;
        numBytesRemaining = fRequestBytesAlreadySeen - requestSize;
        resetRequestBuffer();
        if (numBytesRemaining > 0) {
            memmove(fRequestBuffer, &fRequestBuffer[requestSize], numBytesRemaining);
            newBytesRead = numBytesRemaining;
        }
    } while (numBytesRemaining > 0);
    --fRecursionCount;
    if (!fIsActive) {
        if (fRecursionCount > 0) {
            closeSockets();
        } else {
            delete this;
        }
    }
} // RTSPServer::RTSPClientConnection

static Boolean parseAuthorizationHeader(char const* buf,
                                        char const *& username,
                                        char const *& realm,
                                        char const *& nonce, char const *& uri,
                                        char const *& response)
{
    username = realm = nonce = uri = response = NULL;
    while (1) {
        if (*buf == '\0') {
            return False;
        }
        if (_strncasecmp(buf, "Authorization: Digest ", 22) == 0) {
            break;
        }
        ++buf;
    }
    char const* fields = buf + 22;
    while (*fields == ' ') {
        ++fields;
    }
    char* parameter = strDupSize(fields);
    char* value     = strDupSize(fields);
    while (1) {
        value[0] = '\0';
        if (sscanf(fields, "%[^=]=\"%[^\"]\"", parameter, value) != 2 &&
            sscanf(fields, "%[^=]=\"\"", parameter) != 1)
        {
            break;
        }
        if (strcmp(parameter, "username") == 0) {
            username = strDup(value);
        } else if (strcmp(parameter, "realm") == 0) {
            realm = strDup(value);
        } else if (strcmp(parameter, "nonce") == 0) {
            nonce = strDup(value);
        } else if (strcmp(parameter, "uri") == 0) {
            uri = strDup(value);
        } else if (strcmp(parameter, "response") == 0) {
            response = strDup(value);
        }
        fields += strlen(parameter) + 2  + strlen(value) + 1;
        while (*fields == ',' || *fields == ' ') {
            ++fields;
        }
        if (*fields == '\0' || *fields == '\r' || *fields == '\n') {
            break;
        }
    }
    delete[] parameter;
    delete[] value;
    return True;
} // parseAuthorizationHeader

Boolean RTSPServer::RTSPClientConnection
::authenticationOK(char const* cmdName, char const* urlSuffix, char const* fullRequestStr)
{
    if (!fOurServer.specialClientAccessCheck(fClientInputSocket, fClientAddr, urlSuffix)) {
        setRTSPResponse("401 Unauthorized");
        return False;
    }
    UserAuthenticationDatabase* authDB = fOurServer.getAuthenticationDatabaseForCommand(cmdName);
    if (authDB == NULL) {
        return True;
    }
    char const* username = NULL;
    char const* realm    = NULL;
    char const* nonce    = NULL;
    char const* uri      = NULL;
    char const* response = NULL;
    Boolean success      = False;
    do {
        if (fCurrentAuthenticator.nonce() == NULL) {
            break;
        }
        if (!parseAuthorizationHeader(fullRequestStr,
                                      username, realm, nonce, uri, response) ||
            username == NULL ||
            realm == NULL || strcmp(realm, fCurrentAuthenticator.realm()) != 0 ||
            nonce == NULL || strcmp(nonce, fCurrentAuthenticator.nonce()) != 0 ||
            uri == NULL || response == NULL)
        {
            break;
        }
        char const* password = authDB->lookupPassword(username);
        #ifdef DEBUG
        fprintf(stderr, "lookupPassword(%s) returned password %s\n", username, password);
        #endif
        if (password == NULL) {
            break;
        }
        fCurrentAuthenticator.setUsernameAndPassword(username, password, authDB->passwordsAreMD5());
        char const* ourResponse =
            fCurrentAuthenticator.computeDigestResponse(cmdName, uri);
        success = (strcmp(ourResponse, response) == 0);
        fCurrentAuthenticator.reclaimDigestResponse(ourResponse);
    } while (0);
    delete[](char *) realm;
    delete[](char *) nonce;
    delete[](char *) uri;
    delete[](char *) response;
    if (success) {
        if (!fOurServer.specialClientUserAccessCheck(fClientInputSocket, fClientAddr, urlSuffix, username)) {
            setRTSPResponse("401 Unauthorized");
            delete[](char *) username;
            return False;
        }
    }
    delete[](char *) username;
    if (success) {
        return True;
    }
    fCurrentAuthenticator.setRealmAndRandomNonce(authDB->realm());
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "RTSP/1.0 401 Unauthorized\r\n"
             "CSeq: %s\r\n"
             "%s"
             "WWW-Authenticate: Digest realm=\"%s\", nonce=\"%s\"\r\n\r\n",
             fCurrentCSeq,
             dateHeader(),
             fCurrentAuthenticator.realm(), fCurrentAuthenticator.nonce());
    return False;
} // RTSPServer::RTSPClientConnection

void RTSPServer::RTSPClientConnection
::setRTSPResponse(char const* responseStr)
{
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "RTSP/1.0 %s\r\n"
             "CSeq: %s\r\n"
             "%s\r\n",
             responseStr,
             fCurrentCSeq,
             dateHeader());
}

void RTSPServer::RTSPClientConnection
::setRTSPResponse(char const* responseStr, u_int32_t sessionId)
{
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "RTSP/1.0 %s\r\n"
             "CSeq: %s\r\n"
             "%s"
             "Session: %08X\r\n\r\n",
             responseStr,
             fCurrentCSeq,
             dateHeader(),
             sessionId);
}

void RTSPServer::RTSPClientConnection
::setRTSPResponse(char const* responseStr, char const* contentStr)
{
    if (contentStr == NULL) {
        contentStr = "";
    }
    unsigned const contentLen = strlen(contentStr);
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "RTSP/1.0 %s\r\n"
             "CSeq: %s\r\n"
             "%s"
             "Content-Length: %d\r\n\r\n"
             "%s",
             responseStr,
             fCurrentCSeq,
             dateHeader(),
             contentLen,
             contentStr);
}

void RTSPServer::RTSPClientConnection
::setRTSPResponse(char const* responseStr, u_int32_t sessionId, char const* contentStr)
{
    if (contentStr == NULL) {
        contentStr = "";
    }
    unsigned const contentLen = strlen(contentStr);
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "RTSP/1.0 %s\r\n"
             "CSeq: %s\r\n"
             "%s"
             "Session: %08X\r\n"
             "Content-Length: %d\r\n\r\n"
             "%s",
             responseStr,
             fCurrentCSeq,
             dateHeader(),
             sessionId,
             contentLen,
             contentStr);
}

void RTSPServer::RTSPClientConnection
::changeClientInputSocket(int newSocketNum, unsigned char const* extraData, unsigned extraDataSize)
{
    envir().taskScheduler().disableBackgroundHandling(fClientInputSocket);
    fClientInputSocket = newSocketNum;
    envir().taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE | SOCKET_EXCEPTION,
                                                  (TaskScheduler::BackgroundHandlerProc *) &incomingRequestHandler,
                                                  this);
    if (extraDataSize > 0 && extraDataSize <= fRequestBufferBytesLeft) {
        unsigned char* ptr = &fRequestBuffer[fRequestBytesAlreadySeen];
        for (unsigned i = 0; i < extraDataSize; ++i) {
            ptr[i] = extraData[i];
        }
        handleRequestBytes(extraDataSize);
    }
}

void RTSPServer::RTSPClientConnection::continueHandlingREGISTER(ParamsForREGISTER* params)
{
    params->fOurConnection->continueHandlingREGISTER1(params);
}

void RTSPServer::RTSPClientConnection::continueHandlingREGISTER1(ParamsForREGISTER* params)
{
    int socketNumToBackEndServer = params->fReuseConnection ? fClientOutputSocket : -1;
    RTSPServer* ourServer        = &fOurServer;

    if (socketNumToBackEndServer >= 0) {
        fClientInputSocket = fClientOutputSocket = -1;
        delete this;
    }
    ourServer->implementCmd_REGISTER(params->fURL, params->fURLSuffix, socketNumToBackEndServer,
                                     params->fDeliverViaTCP, params->fProxyURLSuffix);
    delete params;
}

RTSPServer::RTSPClientSession
::RTSPClientSession(RTSPServer& ourServer, u_int32_t sessionId)
    : fOurServer(ourServer), fOurSessionId(sessionId), fOurServerMediaSession(NULL), fIsMulticast(False),
    fStreamAfterSETUP(False),
    fTCPStreamIdCount(0), fLivenessCheckTask(NULL), fNumStreamStates(0), fStreamStates(NULL)
{
    noteLiveness();
}

RTSPServer::RTSPClientSession::~RTSPClientSession()
{
    envir().taskScheduler().unscheduleDelayedTask(fLivenessCheckTask);
    char sessionIdStr[9];
    sprintf(sessionIdStr, "%08X", fOurSessionId);
    fOurServer.fClientSessions->Remove(sessionIdStr);
    reclaimStreamStates();
    if (fOurServerMediaSession != NULL) {
        fOurServerMediaSession->decrementReferenceCount();
        if (fOurServerMediaSession->referenceCount() == 0 &&
            fOurServerMediaSession->deleteWhenUnreferenced())
        {
            fOurServer.removeServerMediaSession(fOurServerMediaSession);
            fOurServerMediaSession = NULL;
        }
    }
}

void RTSPServer::RTSPClientSession::reclaimStreamStates()
{
    for (unsigned i = 0; i < fNumStreamStates; ++i) {
        if (fStreamStates[i].subsession != NULL) {
            fStreamStates[i].subsession->deleteStream(fOurSessionId, fStreamStates[i].streamToken);
        }
    }
    delete[] fStreamStates;
    fStreamStates    = NULL;
    fNumStreamStates = 0;
}

typedef enum StreamingMode {
    RTP_UDP,
    RTP_TCP,
    RAW_UDP
} StreamingMode;
static void parseTransportHeader(char const*  buf,
                                 StreamingMode& streamingMode,
                                 char *       & streamingModeString,
                                 char *       & destinationAddressStr,
                                 u_int8_t     & destinationTTL,
                                 portNumBits  & clientRTPPortNum,
                                 portNumBits  & clientRTCPPortNum,
                                 unsigned char& rtpChannelId,
                                 unsigned char& rtcpChannelId
)
{
    streamingMode         = RTP_UDP;
    streamingModeString   = NULL;
    destinationAddressStr = NULL;
    destinationTTL        = 255;
    clientRTPPortNum      = 0;
    clientRTCPPortNum     = 1;
    rtpChannelId = rtcpChannelId = 0xFF;
    portNumBits p1, p2;
    unsigned ttl, rtpCid, rtcpCid;
    while (1) {
        if (*buf == '\0') {
            return;
        }
        if (*buf == '\r' && *(buf + 1) == '\n' && *(buf + 2) == '\r') {
            return;
        }
        if (_strncasecmp(buf, "Transport:", 10) == 0) {
            break;
        }
        ++buf;
    }
    char const* fields = buf + 10;
    while (*fields == ' ') {
        ++fields;
    }
    char* field = strDupSize(fields);
    while (sscanf(fields, "%[^;\r\n]", field) == 1) {
        if (strcmp(field, "RTP/AVP/TCP") == 0) {
            streamingMode = RTP_TCP;
        } else if (strcmp(field, "RAW/RAW/UDP") == 0 ||
                   strcmp(field, "MP2T/H2221/UDP") == 0)
        {
            streamingMode       = RAW_UDP;
            streamingModeString = strDup(field);
        } else if (_strncasecmp(field, "destination=", 12) == 0) {
            delete[] destinationAddressStr;
            destinationAddressStr = strDup(field + 12);
        } else if (sscanf(field, "ttl%u", &ttl) == 1) {
            destinationTTL = (u_int8_t) ttl;
        } else if (sscanf(field, "client_port=%hu-%hu", &p1, &p2) == 2) {
            clientRTPPortNum  = p1;
            clientRTCPPortNum = streamingMode == RAW_UDP ? 0 : p2;
        } else if (sscanf(field, "client_port=%hu", &p1) == 1) {
            clientRTPPortNum  = p1;
            clientRTCPPortNum = streamingMode == RAW_UDP ? 0 : p1 + 1;
        } else if (sscanf(field, "interleaved=%u-%u", &rtpCid, &rtcpCid) == 2) {
            rtpChannelId  = (unsigned char) rtpCid;
            rtcpChannelId = (unsigned char) rtcpCid;
        }
        fields += strlen(field);
        while (*fields == ';' || *fields == ' ' || *fields == '\t') {
            ++fields;
        }
        if (*fields == '\0' || *fields == '\r' || *fields == '\n') {
            break;
        }
    }
    delete[] field;
} // parseTransportHeader

static Boolean parsePlayNowHeader(char const* buf)
{
    while (1) {
        if (*buf == '\0') {
            return False;
        }
        if (_strncasecmp(buf, "x-playNow:", 10) == 0) {
            break;
        }
        ++buf;
    }
    return True;
}

void RTSPServer::RTSPClientSession
::handleCmd_SETUP(RTSPServer::RTSPClientConnection* ourClientConnection,
                  char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr)
{
    char const* streamName       = urlPreSuffix;
    char const* trackId          = urlSuffix;
    char* concatenatedStreamName = NULL;

    do {
        ServerMediaSession* sms =
            fOurServer.lookupServerMediaSession(streamName, fOurServerMediaSession == NULL);
        if (sms == NULL) {
            if (urlPreSuffix[0] == '\0') {
                streamName = urlSuffix;
            } else {
                concatenatedStreamName = new char[strlen(urlPreSuffix) + strlen(urlSuffix) + 2];
                sprintf(concatenatedStreamName, "%s/%s", urlPreSuffix, urlSuffix);
                streamName = concatenatedStreamName;
            }
            trackId = NULL;
            sms     = fOurServer.lookupServerMediaSession(streamName, fOurServerMediaSession == NULL);
        }
        if (sms == NULL) {
            if (fOurServerMediaSession == NULL) {
                ourClientConnection->handleCmd_notFound();
            } else {
                ourClientConnection->handleCmd_bad();
            }
            break;
        } else {
            if (fOurServerMediaSession == NULL) {
                fOurServerMediaSession = sms;
                fOurServerMediaSession->incrementReferenceCount();
            } else if (sms != fOurServerMediaSession) {
                ourClientConnection->handleCmd_bad();
                break;
            }
        }
        if (fStreamStates == NULL) {
            ServerMediaSubsessionIterator iter(*fOurServerMediaSession);
            for (fNumStreamStates = 0; iter.next() != NULL; ++fNumStreamStates) {}
            fStreamStates = new struct streamState[fNumStreamStates];
            iter.reset();
            ServerMediaSubsession* subsession;
            for (unsigned i = 0; i < fNumStreamStates; ++i) {
                subsession = iter.next();
                fStreamStates[i].subsession  = subsession;
                fStreamStates[i].streamToken = NULL;
            }
        }
        ServerMediaSubsession* subsession = NULL;
        unsigned streamNum;
        if (trackId != NULL && trackId[0] != '\0') {
            for (streamNum = 0; streamNum < fNumStreamStates; ++streamNum) {
                subsession = fStreamStates[streamNum].subsession;
                if (subsession != NULL && strcmp(trackId, subsession->trackId()) == 0) {
                    break;
                }
            }
            if (streamNum >= fNumStreamStates) {
                ourClientConnection->handleCmd_notFound();
                break;
            }
        } else {
            if (fNumStreamStates != 1 || fStreamStates[0].subsession == NULL) {
                ourClientConnection->handleCmd_bad();
                break;
            }
            streamNum  = 0;
            subsession = fStreamStates[streamNum].subsession;
        }
        void *& token = fStreamStates[streamNum].streamToken;
        if (token != NULL) {
            subsession->pauseStream(fOurSessionId, token);
            subsession->deleteStream(fOurSessionId, token);
        }
        StreamingMode streamingMode;
        char* streamingModeString = NULL;
        char* clientsDestinationAddressStr;
        u_int8_t clientsDestinationTTL;
        portNumBits clientRTPPortNum, clientRTCPPortNum;
        unsigned char rtpChannelId, rtcpChannelId;
        parseTransportHeader(fullRequestStr, streamingMode, streamingModeString,
                             clientsDestinationAddressStr, clientsDestinationTTL,
                             clientRTPPortNum, clientRTCPPortNum,
                             rtpChannelId, rtcpChannelId);
        if ((streamingMode == RTP_TCP && rtpChannelId == 0xFF) ||
            (streamingMode != RTP_TCP &&
             ourClientConnection->fClientOutputSocket != ourClientConnection->fClientInputSocket))
        {
            streamingMode = RTP_TCP;
            rtpChannelId  = fTCPStreamIdCount;
            rtcpChannelId = fTCPStreamIdCount + 1;
        }
        if (streamingMode == RTP_TCP) {
            fTCPStreamIdCount += 2;
        }
        Port clientRTPPort(clientRTPPortNum);
        Port clientRTCPPort(clientRTCPPortNum);
        double rangeStart = 0.0, rangeEnd = 0.0;
        char* absStart = NULL;
        char* absEnd   = NULL;
        Boolean startTimeIsNow;
        if (parseRangeHeader(fullRequestStr, rangeStart, rangeEnd, absStart, absEnd, startTimeIsNow)) {
            delete[] absStart;
            delete[] absEnd;
            fStreamAfterSETUP = True;
        } else if (parsePlayNowHeader(fullRequestStr)) {
            fStreamAfterSETUP = True;
        } else {
            fStreamAfterSETUP = False;
        }
        int tcpSocketNum = streamingMode == RTP_TCP ? ourClientConnection->fClientOutputSocket : -1;
        netAddressBits destinationAddress = 0;
        u_int8_t destinationTTL = 255;
        #ifdef RTSP_ALLOW_CLIENT_DESTINATION_SETTING
        if (clientsDestinationAddressStr != NULL) {
            destinationAddress = our_inet_addr(clientsDestinationAddressStr);
        }
        destinationTTL = clientsDestinationTTL;
        #endif
        delete[] clientsDestinationAddressStr;
        Port serverRTPPort(0);
        Port serverRTCPPort(0);
        struct sockaddr_in sourceAddr;
        SOCKLEN_T namelen = sizeof sourceAddr;
        getsockname(ourClientConnection->fClientInputSocket, (struct sockaddr *) &sourceAddr, &namelen);
        netAddressBits origSendingInterfaceAddr   = SendingInterfaceAddr;
        netAddressBits origReceivingInterfaceAddr = ReceivingInterfaceAddr;
        #ifdef HACK_FOR_MULTIHOMED_SERVERS
        ReceivingInterfaceAddr = SendingInterfaceAddr = sourceAddr.sin_addr.s_addr;
        #endif
        subsession->getStreamParameters(fOurSessionId, ourClientConnection->fClientAddr.sin_addr.s_addr,
                                        clientRTPPort, clientRTCPPort,
                                        tcpSocketNum, rtpChannelId, rtcpChannelId,
                                        destinationAddress, destinationTTL, fIsMulticast,
                                        serverRTPPort, serverRTCPPort,
                                        fStreamStates[streamNum].streamToken);
        SendingInterfaceAddr   = origSendingInterfaceAddr;
        ReceivingInterfaceAddr = origReceivingInterfaceAddr;
        AddressString destAddrStr(destinationAddress);
        AddressString sourceAddrStr(sourceAddr);
        char timeoutParameterString[100];
        if (fOurServer.fReclamationTestSeconds > 0) {
            sprintf(timeoutParameterString, ";timeout=%u", fOurServer.fReclamationTestSeconds);
        } else {
            timeoutParameterString[0] = '\0';
        }
        if (fIsMulticast) {
            switch (streamingMode) {
                case RTP_UDP: {
                    snprintf((char *) ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
                             "RTSP/1.0 200 OK\r\n"
                             "CSeq: %s\r\n"
                             "%s"
                             "Transport: RTP/AVP;multicast;destination=%s;source=%s;port=%d-%d;ttl=%d\r\n"
                             "Session: %08X%s\r\n\r\n",
                             ourClientConnection->fCurrentCSeq,
                             dateHeader(),
                             destAddrStr.val(), sourceAddrStr.val(), ntohs(serverRTPPort.num()),
                             ntohs(serverRTCPPort.num()), destinationTTL,
                             fOurSessionId, timeoutParameterString);
                    break;
                }
                case RTP_TCP: {
                    ourClientConnection->handleCmd_unsupportedTransport();
                    break;
                }
                case RAW_UDP: {
                    snprintf((char *) ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
                             "RTSP/1.0 200 OK\r\n"
                             "CSeq: %s\r\n"
                             "%s"
                             "Transport: %s;multicast;destination=%s;source=%s;port=%d;ttl=%d\r\n"
                             "Session: %08X%s\r\n\r\n",
                             ourClientConnection->fCurrentCSeq,
                             dateHeader(),
                             streamingModeString, destAddrStr.val(), sourceAddrStr.val(), ntohs(
                                 serverRTPPort.num()), destinationTTL,
                             fOurSessionId, timeoutParameterString);
                    break;
                }
            }
        } else {
            switch (streamingMode) {
                case RTP_UDP: {
                    snprintf((char *) ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
                             "RTSP/1.0 200 OK\r\n"
                             "CSeq: %s\r\n"
                             "%s"
                             "Transport: RTP/AVP;unicast;destination=%s;source=%s;client_port=%d-%d;server_port=%d-%d\r\n"
                             "Session: %08X%s\r\n\r\n",
                             ourClientConnection->fCurrentCSeq,
                             dateHeader(),
                             destAddrStr.val(), sourceAddrStr.val(), ntohs(clientRTPPort.num()),
                             ntohs(clientRTCPPort.num()), ntohs(serverRTPPort.num()), ntohs(serverRTCPPort.num()),
                             fOurSessionId, timeoutParameterString);
                    break;
                }
                case RTP_TCP: {
                    if (!fOurServer.fAllowStreamingRTPOverTCP) {
                        ourClientConnection->handleCmd_unsupportedTransport();
                    } else {
                        snprintf((char *) ourClientConnection->fResponseBuffer,
                                 sizeof ourClientConnection->fResponseBuffer,
                                 "RTSP/1.0 200 OK\r\n"
                                 "CSeq: %s\r\n"
                                 "%s"
                                 "Transport: RTP/AVP/TCP;unicast;destination=%s;source=%s;interleaved=%d-%d\r\n"
                                 "Session: %08X%s\r\n\r\n",
                                 ourClientConnection->fCurrentCSeq,
                                 dateHeader(),
                                 destAddrStr.val(), sourceAddrStr.val(), rtpChannelId, rtcpChannelId,
                                 fOurSessionId, timeoutParameterString);
                    }
                    break;
                }
                case RAW_UDP: {
                    snprintf((char *) ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
                             "RTSP/1.0 200 OK\r\n"
                             "CSeq: %s\r\n"
                             "%s"
                             "Transport: %s;unicast;destination=%s;source=%s;client_port=%d;server_port=%d\r\n"
                             "Session: %08X%s\r\n\r\n",
                             ourClientConnection->fCurrentCSeq,
                             dateHeader(),
                             streamingModeString, destAddrStr.val(), sourceAddrStr.val(), ntohs(
                                 clientRTPPort.num()), ntohs(serverRTPPort.num()),
                             fOurSessionId, timeoutParameterString);
                    break;
                }
            }
        }
        delete[] streamingModeString;
    } while (0);
    delete[] concatenatedStreamName;
} // RTSPServer::RTSPClientSession

void RTSPServer::RTSPClientSession
::handleCmd_withinSession(RTSPServer::RTSPClientConnection* ourClientConnection,
                          char const* cmdName,
                          char const* urlPreSuffix, char const* urlSuffix,
                          char const* fullRequestStr)
{
    ServerMediaSubsession* subsession;

    if (fOurServerMediaSession == NULL) {
        ourClientConnection->handleCmd_notSupported();
        return;
    } else if (urlSuffix[0] != '\0' && strcmp(fOurServerMediaSession->streamName(), urlPreSuffix) == 0) {
        ServerMediaSubsessionIterator iter(*fOurServerMediaSession);
        while ((subsession = iter.next()) != NULL) {
            if (strcmp(subsession->trackId(), urlSuffix) == 0) {
                break;
            }
        }
        if (subsession == NULL) {
            ourClientConnection->handleCmd_notFound();
            return;
        }
    } else if (strcmp(fOurServerMediaSession->streamName(), urlSuffix) == 0 ||
               (urlSuffix[0] == '\0' && strcmp(fOurServerMediaSession->streamName(), urlPreSuffix) == 0))
    {
        subsession = NULL;
    } else if (urlPreSuffix[0] != '\0' && urlSuffix[0] != '\0') {
        unsigned const urlPreSuffixLen = strlen(urlPreSuffix);
        if (strncmp(fOurServerMediaSession->streamName(), urlPreSuffix, urlPreSuffixLen) == 0 &&
            fOurServerMediaSession->streamName()[urlPreSuffixLen] == '/' &&
            strcmp(&(fOurServerMediaSession->streamName())[urlPreSuffixLen + 1], urlSuffix) == 0)
        {
            subsession = NULL;
        } else {
            ourClientConnection->handleCmd_notFound();
            return;
        }
    } else {
        ourClientConnection->handleCmd_notFound();
        return;
    }
    if (strcmp(cmdName, "TEARDOWN") == 0) {
        handleCmd_TEARDOWN(ourClientConnection, subsession);
    } else if (strcmp(cmdName, "PLAY") == 0) {
        handleCmd_PLAY(ourClientConnection, subsession, fullRequestStr);
    } else if (strcmp(cmdName, "PAUSE") == 0) {
        handleCmd_PAUSE(ourClientConnection, subsession);
    } else if (strcmp(cmdName, "GET_PARAMETER") == 0) {
        handleCmd_GET_PARAMETER(ourClientConnection, subsession, fullRequestStr);
    } else if (strcmp(cmdName, "SET_PARAMETER") == 0) {
        handleCmd_SET_PARAMETER(ourClientConnection, subsession, fullRequestStr);
    }
} // RTSPServer::RTSPClientSession

void RTSPServer::RTSPClientSession
::handleCmd_TEARDOWN(RTSPServer::RTSPClientConnection* ourClientConnection,
                     ServerMediaSubsession*            subsession)
{
    unsigned i;

    for (i = 0; i < fNumStreamStates; ++i) {
        if (subsession == NULL ||
            subsession == fStreamStates[i].subsession)
        {
            if (fStreamStates[i].subsession != NULL) {
                fStreamStates[i].subsession->deleteStream(fOurSessionId, fStreamStates[i].streamToken);
                fStreamStates[i].subsession = NULL;
            }
        }
    }
    setRTSPResponse(ourClientConnection, "200 OK");
    Boolean noSubsessionsRemain = True;
    for (i = 0; i < fNumStreamStates; ++i) {
        if (fStreamStates[i].subsession != NULL) {
            noSubsessionsRemain = False;
            break;
        }
    }
    if (noSubsessionsRemain) {
        delete this;
    }
}

void RTSPServer::RTSPClientSession
::handleCmd_PLAY(RTSPServer::RTSPClientConnection* ourClientConnection,
                 ServerMediaSubsession* subsession, char const* fullRequestStr)
{
    char* rtspURL        = fOurServer.rtspURL(fOurServerMediaSession, ourClientConnection->fClientInputSocket);
    unsigned rtspURLSize = strlen(rtspURL);
    float scale;
    Boolean sawScaleHeader = parseScaleHeader(fullRequestStr, scale);

    if (subsession == NULL) {
        fOurServerMediaSession->testScaleFactor(scale);
    } else {
        subsession->testScaleFactor(scale);
    }
    char buf[100];
    char* scaleHeader;
    if (!sawScaleHeader) {
        buf[0] = '\0';
    } else {
        sprintf(buf, "Scale: %f\r\n", scale);
    }
    scaleHeader = strDup(buf);
    float duration = 0.0;
    double rangeStart = 0.0, rangeEnd = 0.0;
    char* absStart = NULL;
    char* absEnd   = NULL;
    Boolean startTimeIsNow;
    Boolean sawRangeHeader =
        parseRangeHeader(fullRequestStr, rangeStart, rangeEnd, absStart, absEnd, startTimeIsNow);
    if (sawRangeHeader && absStart == NULL) {
        duration = subsession == NULL ?
                   fOurServerMediaSession->duration() : subsession->duration();
        if (duration < 0.0) {
            duration = -duration;
        }
        if (rangeStart < 0.0) {
            rangeStart = 0.0;
        } else if (rangeStart > duration) {
            rangeStart = duration;
        }
        if (rangeEnd < 0.0) {
            rangeEnd = 0.0;
        } else if (rangeEnd > duration) {
            rangeEnd = duration;
        }
        if ((scale > 0.0 && rangeStart > rangeEnd && rangeEnd > 0.0) ||
            (scale < 0.0 && rangeStart < rangeEnd))
        {
            double tmp = rangeStart;
            rangeStart = rangeEnd;
            rangeEnd   = tmp;
        }
    }
    char const* rtpInfoFmt =
        "%s"
        "%s"
        "url=%s/%s"
        ";seq=%d"
        ";rtptime=%u"
    ;
    unsigned rtpInfoFmtSize = strlen(rtpInfoFmt);
    char* rtpInfo = strDup("RTP-Info: ");
    unsigned i, numRTPInfoItems = 0;
    for (i = 0; i < fNumStreamStates; ++i) {
        if (subsession == NULL || fNumStreamStates == 1) {
            if (fStreamStates[i].subsession != NULL) {
                if (sawScaleHeader) {
                    fStreamStates[i].subsession->setStreamScale(fOurSessionId, fStreamStates[i].streamToken, scale);
                }
                if (absStart != NULL) {
                    fStreamStates[i].subsession->seekStream(fOurSessionId, fStreamStates[i].streamToken, absStart,
                                                            absEnd);
                } else {
                    u_int64_t numBytes;
                    if (!sawRangeHeader || startTimeIsNow) {
                        fStreamStates[i].subsession->nullSeekStream(fOurSessionId, fStreamStates[i].streamToken,
                                                                    rangeEnd, numBytes);
                    } else {
                        double streamDuration = 0.0;
                        if (rangeEnd > 0.0 && (rangeEnd + 0.001) < duration) {
                            streamDuration = rangeEnd - rangeStart;
                            if (streamDuration < 0.0) {
                                streamDuration = -streamDuration;
                            }
                        }
                        fStreamStates[i].subsession->seekStream(fOurSessionId, fStreamStates[i].streamToken,
                                                                rangeStart, streamDuration, numBytes);
                    }
                }
            }
        }
    }
    if (absStart != NULL) {
        if (absEnd == NULL) {
            sprintf(buf, "Range: clock=%s-\r\n", absStart);
        } else {
            sprintf(buf, "Range: clock=%s-%s\r\n", absStart, absEnd);
        }
        delete[] absStart;
        delete[] absEnd;
    } else {
        if (!sawRangeHeader || startTimeIsNow) {
            float curNPT = 0.0;
            for (i = 0; i < fNumStreamStates; ++i) {
                if (subsession == NULL ||
                    subsession == fStreamStates[i].subsession)
                {
                    if (fStreamStates[i].subsession == NULL) {
                        continue;
                    }
                    float npt = fStreamStates[i].subsession->getCurrentNPT(fStreamStates[i].streamToken);
                    if (npt > curNPT) {
                        curNPT = npt;
                    }
                }
            }
            rangeStart = curNPT;
        }
        if (rangeEnd == 0.0 && scale >= 0.0) {
            sprintf(buf, "Range: npt=%.3f-\r\n", rangeStart);
        } else {
            sprintf(buf, "Range: npt=%.3f-%.3f\r\n", rangeStart, rangeEnd);
        }
    }
    char* rangeHeader = strDup(buf);
    for (i = 0; i < fNumStreamStates; ++i) {
        if (subsession == NULL ||
            subsession == fStreamStates[i].subsession)
        {
            unsigned short rtpSeqNum = 0;
            unsigned rtpTimestamp    = 0;
            if (fStreamStates[i].subsession == NULL) {
                continue;
            }
            fStreamStates[i].subsession->startStream(fOurSessionId,
                                                     fStreamStates[i].streamToken,
                                                     (TaskFunc *) noteClientLiveness, this,
                                                     rtpSeqNum, rtpTimestamp,
                                                     RTSPServer::RTSPClientConnection::handleAlternativeRequestByte,
                                                     ourClientConnection);
            const char* urlSuffix = fStreamStates[i].subsession->trackId();
            char* prevRTPInfo     = rtpInfo;
            unsigned rtpInfoSize  = rtpInfoFmtSize
                                    + strlen(prevRTPInfo)
                                    + 1
                                    + rtspURLSize + strlen(urlSuffix)
                                    + 5
                                    + 10
                                    + 2;
            rtpInfo = new char[rtpInfoSize];
            sprintf(rtpInfo, rtpInfoFmt,
                    prevRTPInfo,
                    numRTPInfoItems++ == 0 ? "" : ",",
                    rtspURL, urlSuffix,
                    rtpSeqNum,
                    rtpTimestamp
            );
            delete[] prevRTPInfo;
        }
    }
    if (numRTPInfoItems == 0) {
        rtpInfo[0] = '\0';
    } else {
        unsigned rtpInfoLen = strlen(rtpInfo);
        rtpInfo[rtpInfoLen]     = '\r';
        rtpInfo[rtpInfoLen + 1] = '\n';
        rtpInfo[rtpInfoLen + 2] = '\0';
    }
    snprintf((char *) ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
             "RTSP/1.0 200 OK\r\n"
             "CSeq: %s\r\n"
             "%s"
             "%s"
             "%s"
             "Session: %08X\r\n"
             "%s\r\n",
             ourClientConnection->fCurrentCSeq,
             dateHeader(),
             scaleHeader,
             rangeHeader,
             fOurSessionId,
             rtpInfo);
    delete[] rtpInfo;
    delete[] rangeHeader;
    delete[] scaleHeader;
    delete[] rtspURL;
} // RTSPServer::RTSPClientSession

void RTSPServer::RTSPClientSession
::handleCmd_PAUSE(RTSPServer::RTSPClientConnection* ourClientConnection,
                  ServerMediaSubsession*            subsession)
{
    for (unsigned i = 0; i < fNumStreamStates; ++i) {
        if (subsession == NULL ||
            subsession == fStreamStates[i].subsession)
        {
            if (fStreamStates[i].subsession != NULL) {
                fStreamStates[i].subsession->pauseStream(fOurSessionId, fStreamStates[i].streamToken);
            }
        }
    }
    setRTSPResponse(ourClientConnection, "200 OK", fOurSessionId);
}

void RTSPServer::RTSPClientSession
::handleCmd_GET_PARAMETER(RTSPServer::RTSPClientConnection* ourClientConnection,
                          ServerMediaSubsession *, char const *)
{
    setRTSPResponse(ourClientConnection, "200 OK", fOurSessionId, LIVEMEDIA_LIBRARY_VERSION_STRING);
}

void RTSPServer::RTSPClientSession
::handleCmd_SET_PARAMETER(RTSPServer::RTSPClientConnection* ourClientConnection,
                          ServerMediaSubsession *, char const *)
{
    setRTSPResponse(ourClientConnection, "200 OK", fOurSessionId);
}

RTSPServer::RTSPClientConnection * RTSPServer::createNewClientConnection(int                clientSocket,
                                                                         struct sockaddr_in clientAddr)
{
    return new RTSPClientConnection(*this, clientSocket, clientAddr);
}

RTSPServer::RTSPClientSession * RTSPServer::createNewClientSession(u_int32_t sessionId)
{
    return new RTSPClientSession(*this, sessionId);
}

void RTSPServer::RTSPClientSession::noteLiveness()
{
    if (fOurServer.fReclamationTestSeconds > 0) {
        envir().taskScheduler()
        .rescheduleDelayedTask(fLivenessCheckTask,
                               fOurServer.fReclamationTestSeconds * 1000000,
                               (TaskFunc *) livenessTimeoutTask, this);
    }
}

void RTSPServer::RTSPClientSession
::noteClientLiveness(RTSPClientSession* clientSession)
{
    #ifdef DEBUG
    char const* streamName =
        (clientSession->fOurServerMediaSession == NULL) ? "???" : clientSession->fOurServerMediaSession->streamName();
    fprintf(stderr, "RTSP client session (id \"%08X\", stream name \"%s\"): Liveness indication\n",
            clientSession->fOurSessionId, streamName);
    #endif
    clientSession->noteLiveness();
}

void RTSPServer::RTSPClientSession
::livenessTimeoutTask(RTSPClientSession* clientSession)
{
    #ifdef DEBUG
    char const* streamName =
        (clientSession->fOurServerMediaSession == NULL) ? "???" : clientSession->fOurServerMediaSession->streamName();
    fprintf(stderr, "RTSP client session (id \"%08X\", stream name \"%s\") has timed out (due to inactivity)\n",
            clientSession->fOurSessionId, streamName);
    #endif
    delete clientSession;
}

RTSPServer::ServerMediaSessionIterator
::ServerMediaSessionIterator(RTSPServer& server)
    : fOurIterator((server.fServerMediaSessions == NULL) ?
                   NULL : HashTable::Iterator::create(*server.fServerMediaSessions))
{}

RTSPServer::ServerMediaSessionIterator::~ServerMediaSessionIterator()
{
    delete fOurIterator;
}

ServerMediaSession * RTSPServer::ServerMediaSessionIterator::next()
{
    if (fOurIterator == NULL) {
        return NULL;
    }
    char const* key;
    return (ServerMediaSession *) (fOurIterator->next(key));
}

UserAuthenticationDatabase::UserAuthenticationDatabase(char const* realm,
                                                       Boolean     passwordsAreMD5)
    : fTable(HashTable::create(STRING_HASH_KEYS)),
    fRealm(strDup(realm == NULL ? "XRtsp Media" : realm)),
    fPasswordsAreMD5(passwordsAreMD5)
{}

UserAuthenticationDatabase::~UserAuthenticationDatabase()
{
    delete[] fRealm;
    char* password;
    while ((password = (char *) fTable->RemoveNext()) != NULL) {
        delete[] password;
    }
    delete fTable;
}

void UserAuthenticationDatabase::addUserRecord(char const* username,
                                               char const* password)
{
    fTable->Add(username, (void *) (strDup(password)));
}

void UserAuthenticationDatabase::removeUserRecord(char const* username)
{
    char* password = (char *) (fTable->Lookup(username));

    fTable->Remove(username);
    delete[] password;
}

char const * UserAuthenticationDatabase::lookupPassword(char const* username)
{
    return (char const *) (fTable->Lookup(username));
}

RTSPServerWithREGISTERProxying * RTSPServerWithREGISTERProxying
::createNew(UsageEnvironment& env, Port ourPort,
            UserAuthenticationDatabase* authDatabase, UserAuthenticationDatabase* authDatabaseForREGISTER,
            unsigned reclamationTestSeconds,
            Boolean streamRTPOverTCP, int verbosityLevelForProxying)
{
    int ourSocket = setUpOurSocket(env, ourPort);

    if (ourSocket == -1) {
        return NULL;
    }
    return new RTSPServerWithREGISTERProxying(env, ourSocket, ourPort, authDatabase, authDatabaseForREGISTER,
                                              reclamationTestSeconds,
                                              streamRTPOverTCP, verbosityLevelForProxying);
}

RTSPServerWithREGISTERProxying
::RTSPServerWithREGISTERProxying(UsageEnvironment& env, int ourSocket, Port ourPort,
                                 UserAuthenticationDatabase* authDatabase,
                                 UserAuthenticationDatabase* authDatabaseForREGISTER,
                                 unsigned reclamationTestSeconds,
                                 Boolean streamRTPOverTCP, int verbosityLevelForProxying)
    : RTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds),
    fStreamRTPOverTCP(streamRTPOverTCP), fVerbosityLevelForProxying(verbosityLevelForProxying),
    fRegisteredProxyCounter(0), fAllowedCommandNames(NULL), fAuthDBForREGISTER(authDatabaseForREGISTER)
{}

RTSPServerWithREGISTERProxying::~RTSPServerWithREGISTERProxying()
{
    delete[] fAllowedCommandNames;
}

char const * RTSPServerWithREGISTERProxying::allowedCommandNames()
{
    if (fAllowedCommandNames == NULL) {
        char const* baseAllowedCommandNames = RTSPServer::allowedCommandNames();
        char const* newAllowedCommandName   = ", REGISTER";
        fAllowedCommandNames = new char[strlen(baseAllowedCommandNames) + strlen(newAllowedCommandName) + 1];
        sprintf(fAllowedCommandNames, "%s%s", baseAllowedCommandNames, newAllowedCommandName);
    }
    return fAllowedCommandNames;
}

Boolean RTSPServerWithREGISTERProxying::weImplementREGISTER(char const* proxyURLSuffix, char *& responseStr)
{
    if (proxyURLSuffix != NULL && lookupServerMediaSession(proxyURLSuffix) != NULL) {
        responseStr = strDup("451 Invalid parameter");
        return False;
    }
    responseStr = NULL;
    return True;
}

void RTSPServerWithREGISTERProxying::implementCmd_REGISTER(char const* url, char const *, int socketToRemoteServer,
                                                           Boolean deliverViaTCP, char const* proxyURLSuffix)
{
    char const* proxyStreamName;
    char proxyStreamNameBuf[100];

    if (proxyURLSuffix == NULL) {
        sprintf(proxyStreamNameBuf, "registeredProxyStream-%u", ++fRegisteredProxyCounter);
        proxyStreamName = proxyStreamNameBuf;
    } else {
        proxyStreamName = proxyURLSuffix;
    }
    if (fStreamRTPOverTCP) {
        deliverViaTCP = True;
    }
    portNumBits tunnelOverHTTPPortNum = deliverViaTCP ? (portNumBits) (~0) : 0;
    ServerMediaSession* sms =
        ProxyServerMediaSession::createNew(envir(), this, url, proxyStreamName, NULL, NULL,
                                           tunnelOverHTTPPortNum, fVerbosityLevelForProxying, socketToRemoteServer);
    addServerMediaSession(sms);
    char* proxyStreamURL = rtspURL(sms);
    envir() << "Proxying the registered back-end stream \"" << url << "\".\n";
    envir() << "\tPlay this stream using the URL: " << proxyStreamURL << "\n";
    delete[] proxyStreamURL;
}

UserAuthenticationDatabase * RTSPServerWithREGISTERProxying::getAuthenticationDatabaseForCommand(char const* cmdName)
{
    if (strcmp(cmdName, "REGISTER") == 0) {
        return fAuthDBForREGISTER;
    }
    return RTSPServer::getAuthenticationDatabaseForCommand(cmdName);
}
