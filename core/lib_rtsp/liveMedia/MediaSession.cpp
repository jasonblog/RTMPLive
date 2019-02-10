#include "liveMedia.hh"
#include "Locale.hh"
#include "GroupsockHelper.hh"
#include <ctype.h>
MediaSession * MediaSession::createNew(UsageEnvironment& env,
                                       char const*     sdpDescription)
{
    MediaSession* newSession = new MediaSession(env);

    if (newSession != NULL) {
        if (!newSession->initializeWithSDP(sdpDescription)) {
            delete newSession;
            return NULL;
        }
    }
    return newSession;
}

Boolean MediaSession::lookupByName(UsageEnvironment& env,
                                   char const*     instanceName,
                                   MediaSession *  & resultSession)
{
    resultSession = NULL;
    Medium* medium;
    if (!Medium::lookupByName(env, instanceName, medium)) {
        return False;
    }
    if (!medium->isMediaSession()) {
        env.setResultMsg(instanceName, " is not a 'MediaSession' object");
        return False;
    }
    resultSession = (MediaSession *) medium;
    return True;
}

MediaSession::MediaSession(UsageEnvironment& env)
    : Medium(env),
    fSubsessionsHead(NULL), fSubsessionsTail(NULL),
    fConnectionEndpointName(NULL),
    fMaxPlayStartTime(0.0f), fMaxPlayEndTime(0.0f), fAbsStartTime(NULL), fAbsEndTime(NULL),
    fScale(1.0f), fMediaSessionType(NULL), fSessionName(NULL), fSessionDescription(NULL),
    fControlPath(NULL)
{
    fSourceFilterAddr.s_addr = 0;
    const unsigned maxCNAMElen = 100;
    char CNAME[maxCNAMElen + 1];
    #ifndef CRIS
    gethostname((char *) CNAME, maxCNAMElen);
    #else
    sprintf(CNAME, "unknown host %d", (unsigned) (our_random() * 0x7FFFFFFF));
    #endif
    CNAME[maxCNAMElen] = '\0';
    fCNAME = strDup(CNAME);
}

MediaSession::~MediaSession()
{
    delete fSubsessionsHead;
    delete[] fCNAME;
    delete[] fConnectionEndpointName;
    delete[] fAbsStartTime;
    delete[] fAbsEndTime;
    delete[] fMediaSessionType;
    delete[] fSessionName;
    delete[] fSessionDescription;
    delete[] fControlPath;
}

Boolean MediaSession::isMediaSession() const
{
    return True;
}

MediaSubsession * MediaSession::createNewMediaSubsession()
{
    return new MediaSubsession(*this);
}

Boolean MediaSession::initializeWithSDP(char const* sdpDescription)
{
    if (sdpDescription == NULL) {
        return False;
    }
    char const* sdpLine = sdpDescription;
    char const* nextSDPLine;
    while (1) {
        if (!parseSDPLine(sdpLine, nextSDPLine)) {
            return False;
        }
        if (sdpLine[0] == 'm') {
            break;
        }
        sdpLine = nextSDPLine;
        if (sdpLine == NULL) {
            break;
        }
        if (parseSDPLine_s(sdpLine)) {
            continue;
        }
        if (parseSDPLine_i(sdpLine)) {
            continue;
        }
        if (parseSDPLine_c(sdpLine)) {
            continue;
        }
        if (parseSDPAttribute_control(sdpLine)) {
            continue;
        }
        if (parseSDPAttribute_range(sdpLine)) {
            continue;
        }
        if (parseSDPAttribute_type(sdpLine)) {
            continue;
        }
        if (parseSDPAttribute_source_filter(sdpLine)) {
            continue;
        }
    }
    while (sdpLine != NULL) {
        MediaSubsession* subsession = createNewMediaSubsession();
        if (subsession == NULL) {
            envir().setResultMsg("Unable to create new MediaSubsession");
            return False;
        }
        char* mediumName         = strDupSize(sdpLine);
        char const* protocolName = NULL;
        unsigned payloadFormat;
        if ((sscanf(sdpLine, "m=%s %hu RTP/AVP %u",
                    mediumName, &subsession->fClientPortNum, &payloadFormat) == 3 ||
             sscanf(sdpLine, "m=%s %hu/%*u RTP/AVP %u",
                    mediumName, &subsession->fClientPortNum, &payloadFormat) == 3) &&
            payloadFormat <= 127)
        {
            protocolName = "RTP";
        } else if ((sscanf(sdpLine, "m=%s %hu UDP %u",
                           mediumName, &subsession->fClientPortNum, &payloadFormat) == 3 ||
                    sscanf(sdpLine, "m=%s %hu udp %u",
                           mediumName, &subsession->fClientPortNum, &payloadFormat) == 3 ||
                    sscanf(sdpLine, "m=%s %hu RAW/RAW/UDP %u",
                           mediumName, &subsession->fClientPortNum, &payloadFormat) == 3) &&
                   payloadFormat <= 127)
        {
            protocolName = "UDP";
        } else {
            char* sdpLineStr;
            if (nextSDPLine == NULL) {
                sdpLineStr = (char *) sdpLine;
            } else {
                sdpLineStr = strDup(sdpLine);
                sdpLineStr[nextSDPLine - sdpLine] = '\0';
            }
            envir() << "Bad SDP \"m=\" line: " <<  sdpLineStr << "\n";
            if (sdpLineStr != (char *) sdpLine) {
                delete[] sdpLineStr;
            }
            delete[] mediumName;
            delete subsession;
            while (1) {
                sdpLine = nextSDPLine;
                if (sdpLine == NULL) {
                    break;
                }
                if (!parseSDPLine(sdpLine, nextSDPLine)) {
                    return False;
                }
                if (sdpLine[0] == 'm') {
                    break;
                }
            }
            continue;
        }
        if (fSubsessionsTail == NULL) {
            fSubsessionsHead = fSubsessionsTail = subsession;
        } else {
            fSubsessionsTail->setNext(subsession);
            fSubsessionsTail = subsession;
        }
        subsession->serverPortNum = subsession->fClientPortNum;
        char const* mStart = sdpLine;
        subsession->fSavedSDPLines = strDup(mStart);
        subsession->fMediumName    = strDup(mediumName);
        delete[] mediumName;
        subsession->fProtocolName     = strDup(protocolName);
        subsession->fRTPPayloadFormat = payloadFormat;
        while (1) {
            sdpLine = nextSDPLine;
            if (sdpLine == NULL) {
                break;
            }
            if (!parseSDPLine(sdpLine, nextSDPLine)) {
                return False;
            }
            if (sdpLine[0] == 'm') {
                break;
            }
            if (subsession->parseSDPLine_c(sdpLine)) {
                continue;
            }
            if (subsession->parseSDPLine_b(sdpLine)) {
                continue;
            }
            if (subsession->parseSDPAttribute_rtpmap(sdpLine)) {
                continue;
            }
            if (subsession->parseSDPAttribute_rtcpmux(sdpLine)) {
                continue;
            }
            if (subsession->parseSDPAttribute_control(sdpLine)) {
                continue;
            }
            if (subsession->parseSDPAttribute_range(sdpLine)) {
                continue;
            }
            if (subsession->parseSDPAttribute_fmtp(sdpLine)) {
                continue;
            }
            if (subsession->parseSDPAttribute_source_filter(sdpLine)) {
                continue;
            }
            if (subsession->parseSDPAttribute_x_dimensions(sdpLine)) {
                continue;
            }
            if (subsession->parseSDPAttribute_framerate(sdpLine)) {
                continue;
            }
        }
        if (sdpLine != NULL) {
            subsession->fSavedSDPLines[sdpLine - mStart] = '\0';
        }
        if (subsession->fCodecName == NULL) {
            subsession->fCodecName =
                lookupPayloadFormat(subsession->fRTPPayloadFormat,
                                    subsession->fRTPTimestampFrequency,
                                    subsession->fNumChannels);
            if (subsession->fCodecName == NULL) {
                char typeStr[20];
                sprintf(typeStr, "%d", subsession->fRTPPayloadFormat);
                envir().setResultMsg("Unknown codec name for RTP payload type ",
                                     typeStr);
                return False;
            }
        }
        if (subsession->fRTPTimestampFrequency == 0) {
            subsession->fRTPTimestampFrequency =
                guessRTPTimestampFrequency(subsession->fMediumName,
                                           subsession->fCodecName);
        }
    }
    return True;
} // MediaSession::initializeWithSDP

Boolean MediaSession::parseSDPLine(char const* inputLine,
                                   char const *& nextLine)
{
    nextLine = NULL;
    for (char const* ptr = inputLine; *ptr != '\0'; ++ptr) {
        if (*ptr == '\r' || *ptr == '\n') {
            ++ptr;
            while (*ptr == '\r' || *ptr == '\n') {
                ++ptr;
            }
            nextLine = ptr;
            if (nextLine[0] == '\0') {
                nextLine = NULL;
            }
            break;
        }
    }
    if (inputLine[0] == '\r' || inputLine[0] == '\n') {
        return True;
    }
    if (strlen(inputLine) < 2 || inputLine[1] != '=' ||
        inputLine[0] < 'a' || inputLine[0] > 'z')
    {
        envir().setResultMsg("Invalid SDP line: ", inputLine);
        return False;
    }
    return True;
}

static char * parseCLine(char const* sdpLine)
{
    char* resultStr = NULL;
    char* buffer    = strDupSize(sdpLine);

    if (sscanf(sdpLine, "c=IN IP4 %[^/\r\n]", buffer) == 1) {
        resultStr = strDup(buffer);
    }
    delete[] buffer;
    return resultStr;
}

Boolean MediaSession::parseSDPLine_s(char const* sdpLine)
{
    char* buffer         = strDupSize(sdpLine);
    Boolean parseSuccess = False;

    if (sscanf(sdpLine, "s=%[^\r\n]", buffer) == 1) {
        delete[] fSessionName;
        fSessionName = strDup(buffer);
        parseSuccess = True;
    }
    delete[] buffer;
    return parseSuccess;
}

Boolean MediaSession::parseSDPLine_i(char const* sdpLine)
{
    char* buffer         = strDupSize(sdpLine);
    Boolean parseSuccess = False;

    if (sscanf(sdpLine, "i=%[^\r\n]", buffer) == 1) {
        delete[] fSessionDescription;
        fSessionDescription = strDup(buffer);
        parseSuccess        = True;
    }
    delete[] buffer;
    return parseSuccess;
}

Boolean MediaSession::parseSDPLine_c(char const* sdpLine)
{
    char* connectionEndpointName = parseCLine(sdpLine);

    if (connectionEndpointName != NULL) {
        delete[] fConnectionEndpointName;
        fConnectionEndpointName = connectionEndpointName;
        return True;
    }
    return False;
}

Boolean MediaSession::parseSDPAttribute_type(char const* sdpLine)
{
    Boolean parseSuccess = False;
    char* buffer         = strDupSize(sdpLine);

    if (sscanf(sdpLine, "a=type: %[^ ]", buffer) == 1) {
        delete[] fMediaSessionType;
        fMediaSessionType = strDup(buffer);
        parseSuccess      = True;
    }
    delete[] buffer;
    return parseSuccess;
}

Boolean MediaSession::parseSDPAttribute_control(char const* sdpLine)
{
    Boolean parseSuccess = False;
    char* controlPath    = strDupSize(sdpLine);

    if (sscanf(sdpLine, "a=control: %s", controlPath) == 1) {
        parseSuccess = True;
        delete[] fControlPath;
        fControlPath = strDup(controlPath);
    }
    delete[] controlPath;
    return parseSuccess;
}

static Boolean parseRangeAttribute(char const* sdpLine, double& startTime, double& endTime)
{
    return sscanf(sdpLine, "a=range: npt = %lg - %lg", &startTime, &endTime) == 2;
}

static Boolean parseRangeAttribute(char const* sdpLine, char *& absStartTime, char *& absEndTime)
{
    size_t len       = strlen(sdpLine) + 1;
    char* as         = new char[len];
    char* ae         = new char[len];
    int sscanfResult = sscanf(sdpLine, "a=range: clock = %[^-\r\n]-%[^\r\n]", as, ae);

    if (sscanfResult == 2) {
        absStartTime = as;
        absEndTime   = ae;
    } else if (sscanfResult == 1) {
        absStartTime = as;
        delete[] ae;
    } else {
        delete[] as;
        delete[] ae;
        return False;
    }
    return True;
}

Boolean MediaSession::parseSDPAttribute_range(char const* sdpLine)
{
    Boolean parseSuccess = False;
    double playStartTime;
    double playEndTime;

    if (parseRangeAttribute(sdpLine, playStartTime, playEndTime)) {
        parseSuccess = True;
        if (playStartTime > fMaxPlayStartTime) {
            fMaxPlayStartTime = playStartTime;
        }
        if (playEndTime > fMaxPlayEndTime) {
            fMaxPlayEndTime = playEndTime;
        }
    } else if (parseRangeAttribute(sdpLine, _absStartTime(), _absEndTime())) {
        parseSuccess = True;
    }
    return parseSuccess;
}

static Boolean parseSourceFilterAttribute(char const*   sdpLine,
                                          struct in_addr& sourceAddr)
{
    Boolean result   = False;
    char* sourceName = strDupSize(sdpLine);

    do {
        if (sscanf(sdpLine, "a=source-filter: incl IN IP4 %*s %s",
                   sourceName) != 1)
        {
            break;
        }
        NetAddressList addresses(sourceName);
        if (addresses.numAddresses() == 0) {
            break;
        }
        netAddressBits sourceAddrBits =
            *(netAddressBits *) (addresses.firstAddress()->data());
        if (sourceAddrBits == 0) {
            break;
        }
        sourceAddr.s_addr = sourceAddrBits;
        result = True;
    } while (0);
    delete[] sourceName;
    return result;
}

Boolean MediaSession
::parseSDPAttribute_source_filter(char const* sdpLine)
{
    return parseSourceFilterAttribute(sdpLine, fSourceFilterAddr);
}

char * MediaSession::lookupPayloadFormat(unsigned char rtpPayloadType,
                                         unsigned& freq, unsigned& nCh)
{
    char const* temp = NULL;

    switch (rtpPayloadType) {
        case 0: {
            temp = "PCMU";
            freq = 8000;
            nCh  = 1;
            break;
        }
        case 2: {
            temp = "G726-32";
            freq = 8000;
            nCh  = 1;
            break;
        }
        case 3: {
            temp = "GSM";
            freq = 8000;
            nCh  = 1;
            break;
        }
        case 4: {
            temp = "G723";
            freq = 8000;
            nCh  = 1;
            break;
        }
        case 5: {
            temp = "DVI4";
            freq = 8000;
            nCh  = 1;
            break;
        }
        case 6: {
            temp = "DVI4";
            freq = 16000;
            nCh  = 1;
            break;
        }
        case 7: {
            temp = "LPC";
            freq = 8000;
            nCh  = 1;
            break;
        }
        case 8: {
            temp = "PCMA";
            freq = 8000;
            nCh  = 1;
            break;
        }
        case 9: {
            temp = "G722";
            freq = 8000;
            nCh  = 1;
            break;
        }
        case 10: {
            temp = "L16";
            freq = 44100;
            nCh  = 2;
            break;
        }
        case 11: {
            temp = "L16";
            freq = 44100;
            nCh  = 1;
            break;
        }
        case 12: {
            temp = "QCELP";
            freq = 8000;
            nCh  = 1;
            break;
        }
        case 14: {
            temp = "MPA";
            freq = 90000;
            nCh  = 1;
            break;
        }
        case 15: {
            temp = "G728";
            freq = 8000;
            nCh  = 1;
            break;
        }
        case 16: {
            temp = "DVI4";
            freq = 11025;
            nCh  = 1;
            break;
        }
        case 17: {
            temp = "DVI4";
            freq = 22050;
            nCh  = 1;
            break;
        }
        case 18: {
            temp = "G729";
            freq = 8000;
            nCh  = 1;
            break;
        }
        case 25: {
            temp = "CELB";
            freq = 90000;
            nCh  = 1;
            break;
        }
        case 26: {
            temp = "JPEG";
            freq = 90000;
            nCh  = 1;
            break;
        }
        case 28: {
            temp = "NV";
            freq = 90000;
            nCh  = 1;
            break;
        }
        case 31: {
            temp = "H261";
            freq = 90000;
            nCh  = 1;
            break;
        }
        case 32: {
            temp = "MPV";
            freq = 90000;
            nCh  = 1;
            break;
        }
        case 33: {
            temp = "MP2T";
            freq = 90000;
            nCh  = 1;
            break;
        }
        case 34: {
            temp = "H263";
            freq = 90000;
            nCh  = 1;
            break;
        }
    }
    ;
    return strDup(temp);
} // MediaSession::lookupPayloadFormat

unsigned MediaSession::guessRTPTimestampFrequency(char const* mediumName,
                                                  char const* codecName)
{
    if (strcmp(codecName, "L16") == 0) {
        return 44100;
    }
    if (strcmp(codecName, "MPA") == 0 ||
        strcmp(codecName, "MPA-ROBUST") == 0 ||
        strcmp(codecName, "X-MP3-DRAFT-00") == 0)
    {
        return 90000;
    }
    if (strcmp(mediumName, "video") == 0) {
        return 90000;
    } else if (strcmp(mediumName, "text") == 0) {
        return 1000;
    }
    return 8000;
}

char * MediaSession::absStartTime() const
{
    if (fAbsStartTime != NULL) {
        return fAbsStartTime;
    }
    MediaSubsessionIterator iter(*this);
    MediaSubsession* subsession;
    while ((subsession = iter.next()) != NULL) {
        if (subsession->_absStartTime() != NULL) {
            return subsession->_absStartTime();
        }
    }
    return NULL;
}

char * MediaSession::absEndTime() const
{
    if (fAbsEndTime != NULL) {
        return fAbsEndTime;
    }
    MediaSubsessionIterator iter(*this);
    MediaSubsession* subsession;
    while ((subsession = iter.next()) != NULL) {
        if (subsession->_absEndTime() != NULL) {
            return subsession->_absEndTime();
        }
    }
    return NULL;
}

Boolean MediaSession
::initiateByMediaType(char const*      mimeType,
                      MediaSubsession *& resultSubsession,
                      int              useSpecialRTPoffset)
{
    resultSubsession = NULL;
    MediaSubsessionIterator iter(*this);
    MediaSubsession* subsession;
    while ((subsession = iter.next()) != NULL) {
        Boolean wasAlreadyInitiated = subsession->readSource() != NULL;
        if (!wasAlreadyInitiated) {
            if (!subsession->initiate(useSpecialRTPoffset)) {
                return False;
            }
        }
        if (strcmp(subsession->readSource()->MIMEtype(), mimeType) != 0) {
            if (!wasAlreadyInitiated) {
                subsession->deInitiate();
            }
            continue;
        }
        resultSubsession = subsession;
        break;
    }
    if (resultSubsession == NULL) {
        envir().setResultMsg("Session has no usable media subsession");
        return False;
    }
    return True;
}

MediaSubsessionIterator::MediaSubsessionIterator(MediaSession const& session)
    : fOurSession(session)
{
    reset();
}

MediaSubsessionIterator::~MediaSubsessionIterator()
{}

MediaSubsession * MediaSubsessionIterator::next()
{
    MediaSubsession* result = fNextPtr;

    if (fNextPtr != NULL) {
        fNextPtr = fNextPtr->fNext;
    }
    return result;
}

void MediaSubsessionIterator::reset()
{
    fNextPtr = fOurSession.fSubsessionsHead;
}

class SDPAttribute
{
public:
    SDPAttribute(char const* strValue, Boolean valueIsHexadecimal);
    virtual ~SDPAttribute();
    char const * strValue() const
    {
        return fStrValue;
    }

    char const * strValueToLower() const
    {
        return fStrValueToLower;
    }

    int intValue() const
    {
        return fIntValue;
    }

    Boolean valueIsHexadecimal() const
    {
        return fValueIsHexadecimal;
    }

private:
    char* fStrValue;
    char* fStrValueToLower;
    int fIntValue;
    Boolean fValueIsHexadecimal;
};
MediaSubsession::MediaSubsession(MediaSession& parent)
    : serverPortNum(0), sink(NULL), miscPtr(NULL),
    fParent(parent), fNext(NULL),
    fConnectionEndpointName(NULL),
    fClientPortNum(0), fRTPPayloadFormat(0xFF),
    fSavedSDPLines(NULL), fMediumName(NULL), fCodecName(NULL), fProtocolName(NULL),
    fRTPTimestampFrequency(0), fMultiplexRTCPWithRTP(False), fControlPath(NULL),
    fSourceFilterAddr(parent.sourceFilterAddr()), fBandwidth(0),
    fPlayStartTime(0.0), fPlayEndTime(0.0), fAbsStartTime(NULL), fAbsEndTime(NULL),
    fVideoWidth(0), fVideoHeight(0), fVideoFPS(0), fNumChannels(1), fScale(1.0f), fNPT_PTS_Offset(0.0f),
    fAttributeTable(HashTable::create(STRING_HASH_KEYS)),
    fRTPSocket(NULL), fRTCPSocket(NULL),
    fRTPSource(NULL), fRTCPInstance(NULL), fReadSource(NULL),
    fReceiveRawMP3ADUs(False), fReceiveRawJPEGFrames(False),
    fSessionId(NULL)
{
    rtpInfo.seqNum    = 0;
    rtpInfo.timestamp = 0;
    rtpInfo.infoIsNew = False;
    setAttribute("profile-level-id", "0", True);
    setAttribute("profile-id", "1");
    setAttribute("level-id", "93");
    setAttribute("interop-constraints", "B00000000000");
}

MediaSubsession::~MediaSubsession()
{
    deInitiate();
    delete[] fConnectionEndpointName;
    delete[] fSavedSDPLines;
    delete[] fMediumName;
    delete[] fCodecName;
    delete[] fProtocolName;
    delete[] fControlPath;
    delete[] fAbsStartTime;
    delete[] fAbsEndTime;
    delete[] fSessionId;
    SDPAttribute* attr;
    while ((attr = (SDPAttribute *) fAttributeTable->RemoveNext()) != NULL) {
        delete attr;
    }
    delete fAttributeTable;
    delete fNext;
}

void MediaSubsession::addFilter(FramedFilter* filter)
{
    if (filter == NULL || filter->inputSource() != fReadSource) {
        return;
    }
    fReadSource = filter;
}

double MediaSubsession::playStartTime() const
{
    if (fPlayStartTime > 0) {
        return fPlayStartTime;
    }
    return fParent.playStartTime();
}

double MediaSubsession::playEndTime() const
{
    if (fPlayEndTime > 0) {
        return fPlayEndTime;
    }
    return fParent.playEndTime();
}

char * MediaSubsession::absStartTime() const
{
    if (fAbsStartTime != NULL) {
        return fAbsStartTime;
    }
    return fParent.absStartTime();
}

char * MediaSubsession::absEndTime() const
{
    if (fAbsEndTime != NULL) {
        return fAbsEndTime;
    }
    return fParent.absEndTime();
}

static Boolean const honorSDPPortChoice
#ifdef IGNORE_UNICAST_SDP_PORTS
    = False;
#else
    = True;
#endif
Boolean MediaSubsession::initiate(int useSpecialRTPoffset)
{
    if (fReadSource != NULL) {
        return True;
    }
    do {
        if (fCodecName == NULL) {
            env().setResultMsg("Codec is unspecified");
            break;
        }
        struct in_addr tempAddr;
        tempAddr.s_addr = connectionEndpointAddress();
        if (fClientPortNum != 0 && (honorSDPPortChoice || IsMulticastAddress(tempAddr.s_addr))) {
            Boolean const protocolIsRTP = strcmp(fProtocolName, "RTP") == 0;
            if (protocolIsRTP && !fMultiplexRTCPWithRTP) {
                fClientPortNum = fClientPortNum & ~1;
            }
            if (isSSM()) {
                fRTPSocket = new Groupsock(env(), tempAddr, fSourceFilterAddr, fClientPortNum);
            } else {
                fRTPSocket = new Groupsock(env(), tempAddr, fClientPortNum, 255);
            }
            if (fRTPSocket == NULL) {
                env().setResultMsg("Failed to create RTP socket");
                break;
            }
            if (protocolIsRTP) {
                if (fMultiplexRTCPWithRTP) {
                    fRTCPSocket = fRTPSocket;
                } else {
                    portNumBits const rtcpPortNum = fClientPortNum | 1;
                    if (isSSM()) {
                        fRTCPSocket = new Groupsock(env(), tempAddr, fSourceFilterAddr, rtcpPortNum);
                    } else {
                        fRTCPSocket = new Groupsock(env(), tempAddr, rtcpPortNum, 255);
                    }
                }
            }
        } else {
            HashTable* socketHashTable = HashTable::create(ONE_WORD_HASH_KEYS);
            if (socketHashTable == NULL) {
                break;
            }
            Boolean success = False;
            NoReuse dummy(env());
            while (1) {
                if (isSSM()) {
                    fRTPSocket = new Groupsock(env(), tempAddr, fSourceFilterAddr, 0);
                } else {
                    fRTPSocket = new Groupsock(env(), tempAddr, 0, 255);
                }
                if (fRTPSocket == NULL) {
                    env().setResultMsg("MediaSession::initiate(): unable to create RTP and RTCP sockets");
                    break;
                }
                Port clientPort(0);
                if (!getSourcePort(env(), fRTPSocket->socketNum(), clientPort)) {
                    break;
                }
                fClientPortNum = ntohs(clientPort.num());
                if (fMultiplexRTCPWithRTP) {
                    fRTCPSocket = fRTPSocket;
                    success     = True;
                    break;
                }
                if ((fClientPortNum & 1) != 0) {
                    unsigned key        = (unsigned) fClientPortNum;
                    Groupsock* existing = (Groupsock *) socketHashTable->Add((char const *) key, fRTPSocket);
                    delete existing;
                    continue;
                }
                portNumBits rtcpPortNum = fClientPortNum | 1;
                if (isSSM()) {
                    fRTCPSocket = new Groupsock(env(), tempAddr, fSourceFilterAddr, rtcpPortNum);
                } else {
                    fRTCPSocket = new Groupsock(env(), tempAddr, rtcpPortNum, 255);
                }
                if (fRTCPSocket != NULL && fRTCPSocket->socketNum() >= 0) {
                    success = True;
                    break;
                } else {
                    delete fRTCPSocket;
                    fRTCPSocket = NULL;
                    unsigned key        = (unsigned) fClientPortNum;
                    Groupsock* existing = (Groupsock *) socketHashTable->Add((char const *) key, fRTPSocket);
                    delete existing;
                    continue;
                }
            }
            Groupsock* oldGS;
            while ((oldGS = (Groupsock *) socketHashTable->RemoveNext()) != NULL) {
                delete oldGS;
            }
            delete socketHashTable;
            if (!success) {
                break;
            }
        }
        unsigned rtpBufSize = fBandwidth * 25 / 2;
        if (rtpBufSize < 50 * 1024) {
            rtpBufSize = 50 * 1024;
        }
        increaseReceiveBufferTo(env(), fRTPSocket->socketNum(), rtpBufSize);
        if (isSSM() && fRTCPSocket != NULL) {
            fRTCPSocket->changeDestinationParameters(fSourceFilterAddr, 0, ~0);
        }
        if (!createSourceObjects(useSpecialRTPoffset)) {
            break;
        }
        if (fReadSource == NULL) {
            env().setResultMsg("Failed to create read source");
            break;
        }
        if (fRTPSource != NULL && fRTCPSocket != NULL) {
            unsigned totSessionBandwidth =
                fBandwidth ? fBandwidth + fBandwidth / 20 : 500;
            fRTCPInstance = RTCPInstance::createNew(env(), fRTCPSocket,
                                                    totSessionBandwidth,
                                                    (unsigned char const *)
                                                    fParent.CNAME(),
                                                    NULL,
                                                    fRTPSource);
            if (fRTCPInstance == NULL) {
                env().setResultMsg("Failed to create RTCP instance");
                break;
            }
        }
        return True;
    } while (0);
    deInitiate();
    fClientPortNum = 0;
    return False;
} // MediaSubsession::initiate

void MediaSubsession::deInitiate()
{
    Medium::close(fRTCPInstance);
    fRTCPInstance = NULL;
    Medium::close(fReadSource);
    fReadSource = NULL;
    fRTPSource  = NULL;
    delete fRTPSocket;
    if (fRTCPSocket != fRTPSocket) {
        delete fRTCPSocket;
    }
    fRTPSocket  = NULL;
    fRTCPSocket = NULL;
}

Boolean MediaSubsession::setClientPortNum(unsigned short portNum)
{
    if (fReadSource != NULL) {
        env().setResultMsg("A read source has already been created");
        return False;
    }
    fClientPortNum = portNum;
    return True;
}

char const * MediaSubsession::attrVal_str(char const* attrName) const
{
    SDPAttribute* attr = (SDPAttribute *) (fAttributeTable->Lookup(attrName));

    if (attr == NULL) {
        return "";
    }
    return attr->strValue();
}

char const * MediaSubsession::attrVal_strToLower(char const* attrName) const
{
    SDPAttribute* attr = (SDPAttribute *) (fAttributeTable->Lookup(attrName));

    if (attr == NULL) {
        return "";
    }
    return attr->strValueToLower();
}

unsigned MediaSubsession::attrVal_int(char const* attrName) const
{
    SDPAttribute* attr = (SDPAttribute *) (fAttributeTable->Lookup(attrName));

    if (attr == NULL) {
        return 0;
    }
    return attr->intValue();
}

char const * MediaSubsession::fmtp_config() const
{
    char const* result = attrVal_str("config");

    if (result[0] == '\0') {
        result = attrVal_str("configuration");
    }
    return result;
}

netAddressBits MediaSubsession::connectionEndpointAddress() const
{
    do {
        char const* endpointString = connectionEndpointName();
        if (endpointString == NULL) {
            endpointString = parentSession().connectionEndpointName();
        }
        if (endpointString == NULL) {
            break;
        }
        NetAddressList addresses(endpointString);
        if (addresses.numAddresses() == 0) {
            break;
        }
        return *(netAddressBits *) (addresses.firstAddress()->data());
    } while (0);
    return 0;
}

void MediaSubsession::setDestinations(netAddressBits defaultDestAddress)
{
    netAddressBits destAddress = connectionEndpointAddress();

    if (destAddress == 0) {
        destAddress = defaultDestAddress;
    }
    struct in_addr destAddr;
    destAddr.s_addr = destAddress;
    int destTTL = ~0;
    if (fRTPSocket != NULL) {
        Port destPort(serverPortNum);
        fRTPSocket->changeDestinationParameters(destAddr, destPort, destTTL);
    }
    if (fRTCPSocket != NULL && !isSSM() && !fMultiplexRTCPWithRTP) {
        Port destPort(serverPortNum + 1);
        fRTCPSocket->changeDestinationParameters(destAddr, destPort, destTTL);
    }
}

void MediaSubsession::setSessionId(char const* sessionId)
{
    delete[] fSessionId;
    fSessionId = strDup(sessionId);
}

double MediaSubsession::getNormalPlayTime(struct timeval const& presentationTime)
{
    if (rtpSource() == NULL || rtpSource()->timestampFrequency() == 0) {
        return 0.0;
    }
    if (!rtpSource()->hasBeenSynchronizedUsingRTCP()) {
        if (!rtpInfo.infoIsNew) {
            return 0.0;
        }
        u_int32_t timestampOffset = rtpSource()->curPacketRTPTimestamp() - rtpInfo.timestamp;
        double nptOffset = (timestampOffset / (double) (rtpSource()->timestampFrequency())) * scale();
        double npt       = playStartTime() + nptOffset;
        return npt;
    } else {
        double ptsDouble = (double) (presentationTime.tv_sec + presentationTime.tv_usec / 1000000.0);
        if (rtpInfo.infoIsNew) {
            if (seqNumLT(rtpSource()->curPacketRTPSeqNum(), rtpInfo.seqNum)) {
                return -0.1;
            }
            u_int32_t timestampOffset = rtpSource()->curPacketRTPTimestamp() - rtpInfo.timestamp;
            double nptOffset = (timestampOffset / (double) (rtpSource()->timestampFrequency())) * scale();
            double npt       = playStartTime() + nptOffset;
            fNPT_PTS_Offset   = npt - ptsDouble * scale();
            rtpInfo.infoIsNew = False;
            return npt;
        } else {
            if (fNPT_PTS_Offset == 0.0) {
                return 0.0;
            }
            return (double) (ptsDouble * scale() + fNPT_PTS_Offset);
        }
    }
}

void MediaSubsession
::setAttribute(char const* name, char const* value, Boolean valueIsHexadecimal)
{
    SDPAttribute* oldAttr = (SDPAttribute *) fAttributeTable->Lookup(name);

    if (oldAttr != NULL) {
        valueIsHexadecimal = oldAttr->valueIsHexadecimal();
        fAttributeTable->Remove(name);
        delete oldAttr;
    }
    SDPAttribute* newAttr = new SDPAttribute(value, valueIsHexadecimal);
    (void) fAttributeTable->Add(name, newAttr);
}

Boolean MediaSubsession::parseSDPLine_c(char const* sdpLine)
{
    char* connectionEndpointName = parseCLine(sdpLine);

    if (connectionEndpointName != NULL) {
        delete[] fConnectionEndpointName;
        fConnectionEndpointName = connectionEndpointName;
        return True;
    }
    return False;
}

Boolean MediaSubsession::parseSDPLine_b(char const* sdpLine)
{
    return sscanf(sdpLine, "b=AS:%u", &fBandwidth) == 1;
}

Boolean MediaSubsession::parseSDPAttribute_rtpmap(char const* sdpLine)
{
    Boolean parseSuccess = False;
    unsigned rtpmapPayloadFormat;
    char* codecName = strDupSize(sdpLine);
    unsigned rtpTimestampFrequency = 0;
    unsigned numChannels = 1;

    if (sscanf(sdpLine, "a=rtpmap: %u %[^/]/%u/%u",
               &rtpmapPayloadFormat, codecName, &rtpTimestampFrequency,
               &numChannels) == 4 ||
        sscanf(sdpLine, "a=rtpmap: %u %[^/]/%u",
               &rtpmapPayloadFormat, codecName, &rtpTimestampFrequency) == 3 ||
        sscanf(sdpLine, "a=rtpmap: %u %s",
               &rtpmapPayloadFormat, codecName) == 2)
    {
        parseSuccess = True;
        if (rtpmapPayloadFormat == fRTPPayloadFormat) {
            {
                Locale l("POSIX");
                for (char* p = codecName; *p != '\0'; ++p) {
                    *p = toupper(*p);
                }
            }
            delete[] fCodecName;
            fCodecName = strDup(codecName);
            fRTPTimestampFrequency = rtpTimestampFrequency;
            fNumChannels = numChannels;
        }
    }
    delete[] codecName;
    return parseSuccess;
}

Boolean MediaSubsession::parseSDPAttribute_rtcpmux(char const* sdpLine)
{
    if (strncmp(sdpLine, "a=rtcp-mux", 10) == 0) {
        fMultiplexRTCPWithRTP = True;
        return True;
    }
    return False;
}

Boolean MediaSubsession::parseSDPAttribute_control(char const* sdpLine)
{
    Boolean parseSuccess = False;
    char* controlPath    = strDupSize(sdpLine);

    if (sscanf(sdpLine, "a=control: %s", controlPath) == 1) {
        parseSuccess = True;
        delete[] fControlPath;
        fControlPath = strDup(controlPath);
    }
    delete[] controlPath;
    return parseSuccess;
}

Boolean MediaSubsession::parseSDPAttribute_range(char const* sdpLine)
{
    Boolean parseSuccess = False;
    double playStartTime;
    double playEndTime;

    if (parseRangeAttribute(sdpLine, playStartTime, playEndTime)) {
        parseSuccess = True;
        if (playStartTime > fPlayStartTime) {
            fPlayStartTime = playStartTime;
            if (playStartTime > fParent.playStartTime()) {
                fParent.playStartTime() = playStartTime;
            }
        }
        if (playEndTime > fPlayEndTime) {
            fPlayEndTime = playEndTime;
            if (playEndTime > fParent.playEndTime()) {
                fParent.playEndTime() = playEndTime;
            }
        }
    } else if (parseRangeAttribute(sdpLine, _absStartTime(), _absEndTime())) {
        parseSuccess = True;
    }
    return parseSuccess;
}

Boolean MediaSubsession::parseSDPAttribute_fmtp(char const* sdpLine)
{
    do {
        if (strncmp(sdpLine, "a=fmtp:", 7) != 0) {
            break;
        }
        sdpLine += 7;
        while (isdigit(*sdpLine)) {
            ++sdpLine;
        }
        unsigned const sdpLineLen = strlen(sdpLine);
        char* nameStr  = new char[sdpLineLen + 1];
        char* valueStr = new char[sdpLineLen + 1];
        while (*sdpLine != '\0' && *sdpLine != '\r' && *sdpLine != '\n') {
            int sscanfResult = sscanf(sdpLine, " %[^=; \t\r\n] = %[^; \t\r\n]", nameStr, valueStr);
            if (sscanfResult >= 1) {
                Locale l("POSIX");
                for (char* c = nameStr; *c != '\0'; ++c) {
                    *c = tolower(*c);
                }
                if (sscanfResult == 1) {
                    setAttribute(nameStr);
                } else {
                    setAttribute(nameStr, valueStr);
                }
            }
            while (*sdpLine != '\0' && *sdpLine != '\r' && *sdpLine != '\n' && *sdpLine != ';') {
                ++sdpLine;
            }
            while (*sdpLine == ';') {
                ++sdpLine;
            }
        }
        delete[] nameStr;
        delete[] valueStr;
        return True;
    } while (0);
    return False;
} // MediaSubsession::parseSDPAttribute_fmtp

Boolean MediaSubsession
::parseSDPAttribute_source_filter(char const* sdpLine)
{
    return parseSourceFilterAttribute(sdpLine, fSourceFilterAddr);
}

Boolean MediaSubsession::parseSDPAttribute_x_dimensions(char const* sdpLine)
{
    Boolean parseSuccess = False;
    int width, height;

    if (sscanf(sdpLine, "a=x-dimensions:%d,%d", &width, &height) == 2) {
        parseSuccess = True;
        fVideoWidth  = (unsigned short) width;
        fVideoHeight = (unsigned short) height;
    }
    return parseSuccess;
}

Boolean MediaSubsession::parseSDPAttribute_framerate(char const* sdpLine)
{
    Boolean parseSuccess = False;
    float frate;
    int rate;

    if (sscanf(sdpLine, "a=framerate: %f", &frate) == 1 || sscanf(sdpLine, "a=framerate:%f", &frate) == 1) {
        parseSuccess = True;
        fVideoFPS    = (unsigned) frate;
    } else if (sscanf(sdpLine, "a=x-framerate: %d", &rate) == 1) {
        parseSuccess = True;
        fVideoFPS    = (unsigned) rate;
    }
    return parseSuccess;
}

Boolean MediaSubsession::createSourceObjects(int useSpecialRTPoffset)
{
    do {
        if (strcmp(fProtocolName, "UDP") == 0) {
            fReadSource = BasicUDPSource::createNew(env(), fRTPSocket);
            fRTPSource  = NULL;
            if (strcmp(fCodecName, "MP2T") == 0) {
                fReadSource = MPEG2TransportStreamFramer::createNew(env(), fReadSource);
            }
        } else {
            Boolean createSimpleRTPSource = False;
            Boolean doNormalMBitRule      = False;
            if (strcmp(fCodecName, "QCELP") == 0) {
                fReadSource =
                    QCELPAudioRTPSource::createNew(env(), fRTPSocket, fRTPSource,
                                                   fRTPPayloadFormat,
                                                   fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "AMR") == 0) {
                fReadSource =
                    AMRAudioRTPSource::createNew(env(), fRTPSocket, fRTPSource,
                                                 fRTPPayloadFormat, False,
                                                 fNumChannels, attrVal_bool("octet-align"),
                                                 attrVal_unsigned("interleaving"),
                                                 attrVal_bool("robust-sorting"),
                                                 attrVal_bool("crc"));
            } else if (strcmp(fCodecName, "AMR-WB") == 0) {
                fReadSource =
                    AMRAudioRTPSource::createNew(env(), fRTPSocket, fRTPSource,
                                                 fRTPPayloadFormat, True,
                                                 fNumChannels, attrVal_bool("octet-align"),
                                                 attrVal_unsigned("interleaving"),
                                                 attrVal_bool("robust-sorting"),
                                                 attrVal_bool("crc"));
            } else if (strcmp(fCodecName, "MPA") == 0) {
                fReadSource = fRTPSource =
                    MPEG1or2AudioRTPSource::createNew(env(), fRTPSocket,
                                                      fRTPPayloadFormat,
                                                      fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "MPA-ROBUST") == 0) {
                fReadSource = fRTPSource =
                    MP3ADURTPSource::createNew(env(), fRTPSocket, fRTPPayloadFormat,
                                               fRTPTimestampFrequency);
                if (fRTPSource == NULL) {
                    break;
                }
                if (!fReceiveRawMP3ADUs) {
                    MP3ADUdeinterleaver* deinterleaver =
                        MP3ADUdeinterleaver::createNew(env(), fRTPSource);
                    if (deinterleaver == NULL) {
                        break;
                    }
                    fReadSource = MP3FromADUSource::createNew(env(), deinterleaver);
                }
            } else if (strcmp(fCodecName, "X-MP3-DRAFT-00") == 0) {
                fRTPSource =
                    SimpleRTPSource::createNew(env(), fRTPSocket, fRTPPayloadFormat,
                                               fRTPTimestampFrequency,
                                               "audio/MPA-ROBUST");
                if (fRTPSource == NULL) {
                    break;
                }
                fReadSource = MP3FromADUSource::createNew(env(), fRTPSource,
                                                          False);
            } else if (strcmp(fCodecName, "MP4A-LATM") == 0) {
                fReadSource = fRTPSource =
                    MPEG4LATMAudioRTPSource::createNew(env(), fRTPSocket,
                                                       fRTPPayloadFormat,
                                                       fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "VORBIS") == 0) {
                fReadSource = fRTPSource =
                    VorbisAudioRTPSource::createNew(env(), fRTPSocket,
                                                    fRTPPayloadFormat,
                                                    fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "THEORA") == 0) {
                fReadSource = fRTPSource =
                    TheoraVideoRTPSource::createNew(env(), fRTPSocket, fRTPPayloadFormat);
            } else if (strcmp(fCodecName, "VP8") == 0) {
                fReadSource = fRTPSource =
                    VP8VideoRTPSource::createNew(env(), fRTPSocket,
                                                 fRTPPayloadFormat,
                                                 fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "AC3") == 0 || strcmp(fCodecName, "EAC3") == 0) {
                fReadSource = fRTPSource =
                    AC3AudioRTPSource::createNew(env(), fRTPSocket,
                                                 fRTPPayloadFormat,
                                                 fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "MP4V-ES") == 0) {
                fReadSource = fRTPSource =
                    MPEG4ESVideoRTPSource::createNew(env(), fRTPSocket,
                                                     fRTPPayloadFormat,
                                                     fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "MPEG4-GENERIC") == 0) {
                fReadSource = fRTPSource =
                    MPEG4GenericRTPSource::createNew(env(), fRTPSocket,
                                                     fRTPPayloadFormat,
                                                     fRTPTimestampFrequency,
                                                     fMediumName, attrVal_strToLower("mode"),
                                                     attrVal_unsigned("sizelength"),
                                                     attrVal_unsigned("indexlength"),
                                                     attrVal_unsigned("indexdeltalength"));
            } else if (strcmp(fCodecName, "MPV") == 0) {
                fReadSource = fRTPSource =
                    MPEG1or2VideoRTPSource::createNew(env(), fRTPSocket,
                                                      fRTPPayloadFormat,
                                                      fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "MP2T") == 0) {
                fRTPSource = SimpleRTPSource::createNew(env(), fRTPSocket, fRTPPayloadFormat,
                                                        fRTPTimestampFrequency, "video/MP2T",
                                                        0, False);
                fReadSource = MPEG2TransportStreamFramer::createNew(env(), fRTPSource);
            } else if (strcmp(fCodecName, "H261") == 0) {
                fReadSource = fRTPSource =
                    H261VideoRTPSource::createNew(env(), fRTPSocket,
                                                  fRTPPayloadFormat,
                                                  fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "H263-1998") == 0 ||
                       strcmp(fCodecName, "H263-2000") == 0)
            {
                fReadSource = fRTPSource =
                    H263plusVideoRTPSource::createNew(env(), fRTPSocket,
                                                      fRTPPayloadFormat,
                                                      fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "H264") == 0) {
                fReadSource = fRTPSource =
                    H264VideoRTPSource::createNew(env(), fRTPSocket,
                                                  fRTPPayloadFormat,
                                                  fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "H265") == 0) {
                Boolean expectDONFields = attrVal_unsigned("sprop-depack-buf-nalus") > 0;
                fReadSource = fRTPSource =
                    H265VideoRTPSource::createNew(env(), fRTPSocket,
                                                  fRTPPayloadFormat,
                                                  expectDONFields,
                                                  fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "DV") == 0) {
                fReadSource = fRTPSource =
                    DVVideoRTPSource::createNew(env(), fRTPSocket,
                                                fRTPPayloadFormat,
                                                fRTPTimestampFrequency);
            } else if (strcmp(fCodecName, "JPEG") == 0) {
                if (fReceiveRawJPEGFrames) {
                    fReadSource = fRTPSource =
                        SimpleRTPSource::createNew(env(), fRTPSocket, fRTPPayloadFormat,
                                                   fRTPTimestampFrequency, "video/JPEG",
                                                   0, False);
                } else {
                    fReadSource = fRTPSource =
                        JPEGVideoRTPSource::createNew(env(), fRTPSocket,
                                                      fRTPPayloadFormat,
                                                      fRTPTimestampFrequency,
                                                      videoWidth(),
                                                      videoHeight());
                }
            } else if (strcmp(fCodecName, "X-QT") == 0 ||
                       strcmp(fCodecName, "X-QUICKTIME") == 0)
            {
                char* mimeType =
                    new char[strlen(mediumName()) + strlen(codecName()) + 2];
                sprintf(mimeType, "%s/%s", mediumName(), codecName());
                fReadSource = fRTPSource =
                    QuickTimeGenericRTPSource::createNew(env(), fRTPSocket,
                                                         fRTPPayloadFormat,
                                                         fRTPTimestampFrequency,
                                                         mimeType);
                delete[] mimeType;
            } else if (strcmp(fCodecName, "PCMU") == 0 ||
                       strcmp(fCodecName, "GSM") == 0 ||
                       strcmp(fCodecName, "DVI4") == 0 ||
                       strcmp(fCodecName, "PCMA") == 0 ||
                       strcmp(fCodecName, "MP1S") == 0 ||
                       strcmp(fCodecName, "MP2P") == 0 ||
                       strcmp(fCodecName, "L8") == 0 ||
                       strcmp(fCodecName, "L16") == 0 ||
                       strcmp(fCodecName, "L20") == 0 ||
                       strcmp(fCodecName, "L24") == 0 ||
                       strcmp(fCodecName, "G722") == 0 ||
                       strcmp(fCodecName, "G726-16") == 0 ||
                       strcmp(fCodecName, "G726-24") == 0 ||
                       strcmp(fCodecName, "G726-32") == 0 ||
                       strcmp(fCodecName, "G726-40") == 0 ||
                       strcmp(fCodecName, "SPEEX") == 0 ||
                       strcmp(fCodecName, "ILBC") == 0 ||
                       strcmp(fCodecName, "OPUS") == 0 ||
                       strcmp(fCodecName, "T140") == 0 ||
                       strcmp(fCodecName, "DAT12") == 0 ||
                       strcmp(fCodecName, "VND.ONVIF.METADATA") == 0
            )
            {
                createSimpleRTPSource = True;
                useSpecialRTPoffset   = 0;
                if (strcmp(fCodecName, "VND.ONVIF.METADATA") == 0) {
                    doNormalMBitRule = True;
                }
            } else if (useSpecialRTPoffset >= 0) {
                createSimpleRTPSource = True;
            } else {
                env().setResultMsg("RTP payload format unknown or not supported");
                break;
            }
            if (createSimpleRTPSource) {
                char* mimeType =
                    new char[strlen(mediumName()) + strlen(codecName()) + 2];
                sprintf(mimeType, "%s/%s", mediumName(), codecName());
                fReadSource = fRTPSource =
                    SimpleRTPSource::createNew(env(), fRTPSocket, fRTPPayloadFormat,
                                               fRTPTimestampFrequency, mimeType,
                                               (unsigned) useSpecialRTPoffset,
                                               doNormalMBitRule);
                delete[] mimeType;
            }
        }
        return True;
    } while (0);
    return False;
} // MediaSubsession::createSourceObjects

SDPAttribute::SDPAttribute(char const* strValue, Boolean valueIsHexadecimal)
    : fStrValue(strDup(strValue)), fStrValueToLower(NULL), fValueIsHexadecimal(valueIsHexadecimal)
{
    if (fStrValue == NULL) {
        fIntValue = 1;
    } else {
        Locale l("POSIX");
        size_t strSize;
        fStrValueToLower = strDupSize(fStrValue, strSize);
        for (unsigned i = 0; i < strSize - 1; ++i) {
            fStrValueToLower[i] = tolower(fStrValue[i]);
        }
        fStrValueToLower[strSize - 1] = '\0';
        if (sscanf(fStrValueToLower, valueIsHexadecimal ? "%x" : "%d", &fIntValue) != 1) {
            fIntValue = 0;
        }
    }
}

SDPAttribute::~SDPAttribute()
{
    delete[] fStrValue;
    delete[] fStrValueToLower;
}
