#include "ServerMediaSession.hh"
#include <GroupsockHelper.hh>
#include <math.h>
#if defined(__WIN32__) || defined(_WIN32) || defined(_QNX4)
# define snprintf _snprintf
#endif
ServerMediaSession * ServerMediaSession
::createNew(UsageEnvironment& env,
            char const* streamName, char const* info,
            char const* description, Boolean isSSM, char const* miscSDPLines)
{
    return new ServerMediaSession(env, streamName, info, description,
                                  isSSM, miscSDPLines);
}

Boolean ServerMediaSession
::lookupByName(UsageEnvironment& env, char const* mediumName,
               ServerMediaSession *& resultSession)
{
    resultSession = NULL;
    Medium* medium;
    if (!Medium::lookupByName(env, mediumName, medium)) {
        return False;
    }
    if (!medium->isServerMediaSession()) {
        env.setResultMsg(mediumName, " is not a 'ServerMediaSession' object");
        return False;
    }
    resultSession = (ServerMediaSession *) medium;
    return True;
}

static char const * const libNameStr = "XRtsp Media v";
char const * const libVersionStr     = LIVEMEDIA_LIBRARY_VERSION_STRING;
ServerMediaSession::ServerMediaSession(UsageEnvironment& env,
                                       char const* streamName,
                                       char const* info,
                                       char const* description,
                                       Boolean isSSM, char const* miscSDPLines)
    : Medium(env), fIsSSM(isSSM), fSubsessionsHead(NULL),
    fSubsessionsTail(NULL), fSubsessionCounter(0),
    fReferenceCount(0), fDeleteWhenUnreferenced(False)
{
    fStreamName = strDup(streamName == NULL ? "" : streamName);
    char* libNamePlusVersionStr = NULL;
    if (info == NULL || description == NULL) {
        libNamePlusVersionStr = new char[strlen(libNameStr) + strlen(libVersionStr) + 1];
        sprintf(libNamePlusVersionStr, "%s%s", libNameStr, libVersionStr);
    }
    fInfoSDPString        = strDup(info == NULL ? libNamePlusVersionStr : info);
    fDescriptionSDPString = strDup(description == NULL ? libNamePlusVersionStr : description);
    delete[] libNamePlusVersionStr;
    fMiscSDPLines = strDup(miscSDPLines == NULL ? "" : miscSDPLines);
    gettimeofday(&fCreationTime, NULL);
}

ServerMediaSession::~ServerMediaSession()
{
    deleteAllSubsessions();
    delete[] fStreamName;
    delete[] fInfoSDPString;
    delete[] fDescriptionSDPString;
    delete[] fMiscSDPLines;
}

Boolean ServerMediaSession::addSubsession(ServerMediaSubsession* subsession)
{
    if (subsession->fParentSession != NULL) {
        return False;
    }
    if (fSubsessionsTail == NULL) {
        fSubsessionsHead = subsession;
    } else {
        fSubsessionsTail->fNext = subsession;
    }
    fSubsessionsTail = subsession;
    subsession->fParentSession = this;
    subsession->fTrackNumber   = ++fSubsessionCounter;
    return True;
}

void ServerMediaSession::testScaleFactor(float& scale)
{
    float minSSScale      = 1.0;
    float maxSSScale      = 1.0;
    float bestSSScale     = 1.0;
    float bestDistanceTo1 = 0.0;
    ServerMediaSubsession* subsession;

    for (subsession = fSubsessionsHead; subsession != NULL;
         subsession = subsession->fNext)
    {
        float ssscale = scale;
        subsession->testScaleFactor(ssscale);
        if (subsession == fSubsessionsHead) {
            minSSScale      = maxSSScale = bestSSScale = ssscale;
            bestDistanceTo1 = (float) fabs(ssscale - 1.0f);
        } else {
            if (ssscale < minSSScale) {
                minSSScale = ssscale;
            } else if (ssscale > maxSSScale) {
                maxSSScale = ssscale;
            }
            float distanceTo1 = (float) fabs(ssscale - 1.0f);
            if (distanceTo1 < bestDistanceTo1) {
                bestSSScale     = ssscale;
                bestDistanceTo1 = distanceTo1;
            }
        }
    }
    if (minSSScale == maxSSScale) {
        scale = minSSScale;
        return;
    }
    for (subsession = fSubsessionsHead; subsession != NULL;
         subsession = subsession->fNext)
    {
        float ssscale = bestSSScale;
        subsession->testScaleFactor(ssscale);
        if (ssscale != bestSSScale) {
            break;
        }
    }
    if (subsession == NULL) {
        scale = bestSSScale;
        return;
    }
    for (subsession = fSubsessionsHead; subsession != NULL;
         subsession = subsession->fNext)
    {
        float ssscale = 1;
        subsession->testScaleFactor(ssscale);
    }
    scale = 1;
} // ServerMediaSession::testScaleFactor

float ServerMediaSession::duration() const
{
    float minSubsessionDuration = 0.0;
    float maxSubsessionDuration = 0.0;

    for (ServerMediaSubsession* subsession = fSubsessionsHead; subsession != NULL;
         subsession = subsession->fNext)
    {
        char* absStartTime = NULL;
        char* absEndTime   = NULL;
        subsession->getAbsoluteTimeRange(absStartTime, absEndTime);
        if (absStartTime != NULL) {
            return -1.0f;
        }
        float ssduration = subsession->duration();
        if (subsession == fSubsessionsHead) {
            minSubsessionDuration = maxSubsessionDuration = ssduration;
        } else if (ssduration < minSubsessionDuration) {
            minSubsessionDuration = ssduration;
        } else if (ssduration > maxSubsessionDuration) {
            maxSubsessionDuration = ssduration;
        }
    }
    if (maxSubsessionDuration != minSubsessionDuration) {
        return -maxSubsessionDuration;
    } else {
        return maxSubsessionDuration;
    }
}

void ServerMediaSession::deleteAllSubsessions()
{
    Medium::close(fSubsessionsHead);
    fSubsessionsHead   = fSubsessionsTail = NULL;
    fSubsessionCounter = 0;
}

Boolean ServerMediaSession::isServerMediaSession() const
{
    return True;
}

char * ServerMediaSession::generateSDPDescription()
{
    AddressString ipAddressStr(ourIPAddress(envir()));
    unsigned ipAddressStrSize = strlen(ipAddressStr.val());
    char* sourceFilterLine;

    if (fIsSSM) {
        char const * const sourceFilterFmt =
            "a=source-filter: incl IN IP4 * %s\r\n"
            "a=rtcp-unicast: reflection\r\n";
        unsigned const sourceFilterFmtSize = strlen(sourceFilterFmt) + ipAddressStrSize + 1;
        sourceFilterLine = new char[sourceFilterFmtSize];
        sprintf(sourceFilterLine, sourceFilterFmt, ipAddressStr.val());
    } else {
        sourceFilterLine = strDup("");
    }
    char* rangeLine = NULL;
    char* sdp       = NULL;
    do {
        unsigned sdpLength = 0;
        ServerMediaSubsession* subsession;
        for (subsession = fSubsessionsHead; subsession != NULL;
             subsession = subsession->fNext)
        {
            char const* sdpLines = subsession->sdpLines();
            if (sdpLines == NULL) {
                continue;
            }
            sdpLength += strlen(sdpLines);
        }
        if (sdpLength == 0) {
            break;
        }
        float dur = duration();
        if (dur == 0.0) {
            rangeLine = strDup("a=range:npt=0-\r\n");
        } else if (dur > 0.0) {
            char buf[100];
            sprintf(buf, "a=range:npt=0-%.3f\r\n", dur);
            rangeLine = strDup(buf);
        } else {
            rangeLine = strDup("");
        }
        char const * const sdpPrefixFmt =
            "v=0\r\n"
            "o=- %ld%06ld %d IN IP4 %s\r\n"
            "s=%s\r\n"
            "i=%s\r\n"
            "t=0 0\r\n"
            "a=tool:%s%s\r\n"
            "a=type:broadcast\r\n"
            "a=control:*\r\n"
            "%s"
            "%s"
            "a=x-qt-text-nam:%s\r\n"
            "a=x-qt-text-inf:%s\r\n"
            "%s";
        sdpLength += strlen(sdpPrefixFmt)
                     + 20 + 6 + 20 + ipAddressStrSize
                     + strlen(fDescriptionSDPString)
                     + strlen(fInfoSDPString)
                     + strlen(libNameStr) + strlen(libVersionStr)
                     + strlen(sourceFilterLine)
                     + strlen(rangeLine)
                     + strlen(fDescriptionSDPString)
                     + strlen(fInfoSDPString)
                     + strlen(fMiscSDPLines);
        sdpLength += 1000;
        sdp        = new char[sdpLength];
        if (sdp == NULL) {
            break;
        }
        snprintf(sdp, sdpLength, sdpPrefixFmt,
                 fCreationTime.tv_sec, fCreationTime.tv_usec,
                 1,
                 ipAddressStr.val(),
                 fDescriptionSDPString,
                 fInfoSDPString,
                 libNameStr, libVersionStr,
                 sourceFilterLine,
                 rangeLine,
                 fDescriptionSDPString,
                 fInfoSDPString,
                 fMiscSDPLines);
        char* mediaSDP = sdp;
        for (subsession = fSubsessionsHead; subsession != NULL;
             subsession = subsession->fNext)
        {
            unsigned mediaSDPLength = strlen(mediaSDP);
            mediaSDP  += mediaSDPLength;
            sdpLength -= mediaSDPLength;
            if (sdpLength <= 1) {
                break;
            }
            char const* sdpLines = subsession->sdpLines();
            if (sdpLines != NULL) {
                snprintf(mediaSDP, sdpLength, "%s", sdpLines);
            }
        }
    } while (0);
    delete[] rangeLine;
    delete[] sourceFilterLine;
    return sdp;
} // ServerMediaSession::generateSDPDescription

ServerMediaSubsessionIterator
::ServerMediaSubsessionIterator(ServerMediaSession& session)
    : fOurSession(session)
{
    reset();
}

ServerMediaSubsessionIterator::~ServerMediaSubsessionIterator()
{}

ServerMediaSubsession * ServerMediaSubsessionIterator::next()
{
    ServerMediaSubsession* result = fNextPtr;

    if (fNextPtr != NULL) {
        fNextPtr = fNextPtr->fNext;
    }
    return result;
}

void ServerMediaSubsessionIterator::reset()
{
    fNextPtr = fOurSession.fSubsessionsHead;
}

ServerMediaSubsession::ServerMediaSubsession(UsageEnvironment& env)
    : Medium(env),
    fParentSession(NULL), fServerAddressForSDP(0), fPortNumForSDP(0),
    fNext(NULL), fTrackNumber(0), fTrackId(NULL)
{}

ServerMediaSubsession::~ServerMediaSubsession()
{
    delete[](char *) fTrackId;
    Medium::close(fNext);
}

char const * ServerMediaSubsession::trackId()
{
    if (fTrackNumber == 0) {
        return NULL;
    }
    if (fTrackId == NULL) {
        char buf[100];
        sprintf(buf, "track%d", fTrackNumber);
        fTrackId = strDup(buf);
    }
    return fTrackId;
}

void ServerMediaSubsession::pauseStream(unsigned,
                                        void *)
{}

void ServerMediaSubsession::seekStream(unsigned,
                                       void *, double&, double, u_int64_t& numBytes)
{
    numBytes = 0;
}

void ServerMediaSubsession::seekStream(unsigned,
                                       void *, char *& absStart, char *& absEnd)
{
    delete[] absStart;
    absStart = NULL;
    delete[] absEnd;
    absEnd = NULL;
}

void ServerMediaSubsession::nullSeekStream(unsigned, void *,
                                           double streamEndTime, u_int64_t& numBytes)
{
    numBytes = 0;
}

void ServerMediaSubsession::setStreamScale(unsigned,
                                           void *, float)
{}

float ServerMediaSubsession::getCurrentNPT(void *)
{
    return 0.0;
}

FramedSource * ServerMediaSubsession::getStreamSource(void *)
{
    return NULL;
}

void ServerMediaSubsession::deleteStream(unsigned,
                                         void *&)
{}

void ServerMediaSubsession::testScaleFactor(float& scale)
{
    scale = 1;
}

float ServerMediaSubsession::duration() const
{
    return 0.0;
}

void ServerMediaSubsession::getAbsoluteTimeRange(char *& absStartTime, char *& absEndTime) const
{
    absStartTime = absEndTime = NULL;
}

void ServerMediaSubsession::setServerAddressAndPortForSDP(netAddressBits addressBits,
                                                          portNumBits    portBits)
{
    fServerAddressForSDP = addressBits;
    fPortNumForSDP       = portBits;
}

char const * ServerMediaSubsession::rangeSDPLine() const
{
    char* absStart = NULL;
    char* absEnd   = NULL;

    getAbsoluteTimeRange(absStart, absEnd);
    if (absStart != NULL) {
        char buf[100];
        if (absEnd != NULL) {
            sprintf(buf, "a=range:clock=%s-%s\r\n", absStart, absEnd);
        } else {
            sprintf(buf, "a=range:clock=%s-\r\n", absStart);
        }
        return strDup(buf);
    }
    if (fParentSession == NULL) {
        return NULL;
    }
    if (fParentSession->duration() >= 0.0) {
        return strDup("");
    }
    float ourDuration = duration();
    if (ourDuration == 0.0) {
        return strDup("a=range:npt=0-\r\n");
    } else {
        char buf[100];
        sprintf(buf, "a=range:npt=0-%.3f\r\n", ourDuration);
        return strDup(buf);
    }
}
