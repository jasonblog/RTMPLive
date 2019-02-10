#include "RTSPServerSupportingHTTPStreaming.hh"
#include "RTSPCommon.hh"
#ifndef _WIN32_WCE
# include <sys/stat.h>
#endif
#include <time.h>
RTSPServerSupportingHTTPStreaming * RTSPServerSupportingHTTPStreaming::createNew(UsageEnvironment& env, Port rtspPort,
                                                                                 UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds)
{
    int ourSocket = setUpOurSocket(env, rtspPort);

    if (ourSocket == -1) {
        return NULL;
    }
    return new RTSPServerSupportingHTTPStreaming(env, ourSocket, rtspPort, authDatabase, reclamationTestSeconds);
}

RTSPServerSupportingHTTPStreaming
::RTSPServerSupportingHTTPStreaming(UsageEnvironment& env, int ourSocket, Port rtspPort,
                                    UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds)
    : RTSPServer(env, ourSocket, rtspPort, authDatabase, reclamationTestSeconds)
{}

RTSPServerSupportingHTTPStreaming::~RTSPServerSupportingHTTPStreaming()
{}

RTSPServer::RTSPClientConnection * RTSPServerSupportingHTTPStreaming::createNewClientConnection(
    int                clientSocket,
    struct sockaddr_in clientAddr)
{
    return new RTSPClientConnectionSupportingHTTPStreaming(*this, clientSocket, clientAddr);
}

RTSPServerSupportingHTTPStreaming::RTSPClientConnectionSupportingHTTPStreaming
::RTSPClientConnectionSupportingHTTPStreaming(RTSPServer& ourServer, int clientSocket, struct sockaddr_in clientAddr)
    : RTSPClientConnection(ourServer, clientSocket, clientAddr),
    fClientSessionId(0), fPlaylistSource(NULL), fTCPSink(NULL)
{}

RTSPServerSupportingHTTPStreaming::RTSPClientConnectionSupportingHTTPStreaming::~
RTSPClientConnectionSupportingHTTPStreaming()
{
    Medium::close(fPlaylistSource);
    Medium::close(fTCPSink);
}

static char const * lastModifiedHeader(char const* fileName)
{
    static char buf[200];

    buf[0] = '\0';
    #ifndef _WIN32_WCE
    struct stat sb;
    int statResult = stat(fileName, &sb);
    if (statResult == 0) {
        strftime(buf, sizeof buf, "Last-Modified: %a, %b %d %Y %H:%M:%S GMT\r\n",
                 gmtime((const time_t *) &sb.st_mtime));
    }
    #endif
    return buf;
}

void RTSPServerSupportingHTTPStreaming::RTSPClientConnectionSupportingHTTPStreaming
::handleHTTPCmd_StreamingGET(char const* urlSuffix, char const *)
{
    do {
        char const* questionMarkPos = strrchr(urlSuffix, '?');
        if (questionMarkPos == NULL) {
            break;
        }
        unsigned offsetInSeconds, durationInSeconds;
        if (sscanf(questionMarkPos, "?segment=%u,%u", &offsetInSeconds, &durationInSeconds) != 2) {
            break;
        }
        char* streamName = strDup(urlSuffix);
        streamName[questionMarkPos - urlSuffix] = '\0';
        do {
            ServerMediaSession* session = fOurServer.lookupServerMediaSession(streamName);
            if (session == NULL) {
                handleHTTPCmd_notFound();
                break;
            }
            ServerMediaSubsessionIterator iter(*session);
            ServerMediaSubsession* subsession = iter.next();
            if (subsession == NULL) {
                handleHTTPCmd_notFound();
                break;
            }
            ++fClientSessionId;
            Port clientRTPPort(0), clientRTCPPort(0), serverRTPPort(0), serverRTCPPort(0);
            netAddressBits destinationAddress = 0;
            u_int8_t destinationTTL = 0;
            Boolean isMulticast     = False;
            void* streamToken;
            subsession->getStreamParameters(fClientSessionId, 0, clientRTPPort, clientRTCPPort, -1, 0, 0,
                                            destinationAddress, destinationTTL, isMulticast, serverRTPPort,
                                            serverRTCPPort, streamToken);
            double dOffsetInSeconds = (double) offsetInSeconds;
            u_int64_t numBytes;
            subsession->seekStream(fClientSessionId, streamToken, dOffsetInSeconds, (double) durationInSeconds,
                                   numBytes);
            unsigned numTSBytesToStream = (unsigned) numBytes;
            if (numTSBytesToStream == 0) {
                handleHTTPCmd_notSupported();
                break;
            }
            snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
                     "HTTP/1.1 200 OK\r\n"
                     "%s"
                     "Server: XRtsp Media v%s\r\n"
                     "%s"
                     "Content-Length: %d\r\n"
                     "Content-Type: text/plain; charset=ISO-8859-1\r\n"
                     "\r\n",
                     dateHeader(),
                     LIVEMEDIA_LIBRARY_VERSION_STRING,
                     lastModifiedHeader(streamName),
                     numTSBytesToStream);
            send(fClientOutputSocket, (char const *) fResponseBuffer, strlen((char *) fResponseBuffer), 0);
            fResponseBuffer[0] = '\0';
            FramedSource* mediaSource = subsession->getStreamSource(streamToken);
            if (mediaSource != NULL) {
                if (fTCPSink == NULL) {
                    fTCPSink = TCPStreamSink::createNew(envir(), fClientOutputSocket);
                }
                fTCPSink->startPlaying(*mediaSource, afterStreaming, this);
            }
        } while (0);
        delete[] streamName;
        return;
    } while (0);
    ServerMediaSession* session = fOurServer.lookupServerMediaSession(urlSuffix);
    if (session == NULL) {
        handleHTTPCmd_notFound();
        return;
    }
    float duration = session->duration();
    if (duration <= 0.0) {
        handleHTTPCmd_notSupported();
        return;
    }
    unsigned const maxIntLen = 10;
    char const * const playlistPrefixFmt =
        "#EXTM3U\r\n"
        "#EXT-X-ALLOW-CACHE:YES\r\n"
        "#EXT-X-MEDIA-SEQUENCE:0\r\n"
        "#EXT-X-TARGETDURATION:%d\r\n";
    unsigned const playlistPrefixFmt_maxLen     = strlen(playlistPrefixFmt) + maxIntLen;
    char const * const playlistMediaFileSpecFmt =
        "#EXTINF:%d,\r\n"
        "%s?segment=%d,%d\r\n";
    unsigned const playlistMediaFileSpecFmt_maxLen = strlen(playlistMediaFileSpecFmt) + maxIntLen + strlen(urlSuffix)
                                                     + 2 * maxIntLen;
    char const * const playlistSuffixFmt =
        "#EXT-X-ENDLIST\r\n";
    unsigned const playlistSuffixFmt_maxLen = strlen(playlistSuffixFmt);
    unsigned const playlistMaxSize       = 10000;
    unsigned const mediaFileSpecsMaxSize = playlistMaxSize - (playlistPrefixFmt_maxLen + playlistSuffixFmt_maxLen);
    unsigned const maxNumMediaFileSpecs  = mediaFileSpecsMaxSize / playlistMediaFileSpecFmt_maxLen;
    unsigned targetDuration = (unsigned) (duration / maxNumMediaFileSpecs + 1);
    if (targetDuration < 10) {
        targetDuration = 10;
    }
    char* playlist = new char[playlistMaxSize];
    char* s        = playlist;
    sprintf(s, playlistPrefixFmt, targetDuration);
    s += strlen(s);
    unsigned durSoFar = 0;
    while (1) {
        unsigned dur = targetDuration < duration ? targetDuration : (unsigned) duration;
        duration -= dur;
        sprintf(s, playlistMediaFileSpecFmt, dur, urlSuffix, durSoFar, dur);
        s += strlen(s);
        if (duration < 1.0) {
            break;
        }
        durSoFar += dur;
    }
    sprintf(s, playlistSuffixFmt);
    s += strlen(s);
    unsigned playlistLen = s - playlist;
    snprintf((char *) fResponseBuffer, sizeof fResponseBuffer,
             "HTTP/1.1 200 OK\r\n"
             "%s"
             "Server: XRtsp Media v%s\r\n"
             "%s"
             "Content-Length: %d\r\n"
             "Content-Type: application/vnd.apple.mpegurl\r\n"
             "\r\n",
             dateHeader(),
             LIVEMEDIA_LIBRARY_VERSION_STRING,
             lastModifiedHeader(urlSuffix),
             playlistLen);
    send(fClientOutputSocket, (char const *) fResponseBuffer, strlen((char *) fResponseBuffer), 0);
    fResponseBuffer[0] = '\0';
    if (fPlaylistSource != NULL) {
        if (fTCPSink != NULL) {
            fTCPSink->stopPlaying();
        }
        Medium::close(fPlaylistSource);
    }
    fPlaylistSource = ByteStreamMemoryBufferSource::createNew(envir(), (u_int8_t *) playlist, playlistLen);
    if (fTCPSink == NULL) {
        fTCPSink = TCPStreamSink::createNew(envir(), fClientOutputSocket);
    }
    fTCPSink->startPlaying(*fPlaylistSource, afterStreaming, this);
} // RTSPServerSupportingHTTPStreaming::RTSPClientConnectionSupportingHTTPStreaming

void RTSPServerSupportingHTTPStreaming::RTSPClientConnectionSupportingHTTPStreaming::afterStreaming(void* clientData)
{
    RTSPServerSupportingHTTPStreaming::RTSPClientConnectionSupportingHTTPStreaming* clientConnection =
        (RTSPServerSupportingHTTPStreaming::RTSPClientConnectionSupportingHTTPStreaming *) clientData;

    if (clientConnection->fRecursionCount > 0) {
        clientConnection->fIsActive = False;
    } else {
        delete clientConnection;
    }
}
