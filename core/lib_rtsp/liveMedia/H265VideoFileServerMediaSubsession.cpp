#include "H265VideoFileServerMediaSubsession.hh"
#include "H265VideoRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "H265VideoStreamFramer.hh"
H265VideoFileServerMediaSubsession * H265VideoFileServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                                   char const*     fileName,
                                                                                   Boolean         reuseFirstSource)
{
    return new H265VideoFileServerMediaSubsession(env, fileName, reuseFirstSource);
}

H265VideoFileServerMediaSubsession::H265VideoFileServerMediaSubsession(UsageEnvironment& env,
                                                                       char const* fileName, Boolean reuseFirstSource)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource),
    fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL)
{}

H265VideoFileServerMediaSubsession::~H265VideoFileServerMediaSubsession()
{
    delete[] fAuxSDPLine;
}

static void afterPlayingDummy(void* clientData)
{
    H265VideoFileServerMediaSubsession* subsess = (H265VideoFileServerMediaSubsession *) clientData;

    subsess->afterPlayingDummy1();
}

void H265VideoFileServerMediaSubsession::afterPlayingDummy1()
{
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData)
{
    H265VideoFileServerMediaSubsession* subsess = (H265VideoFileServerMediaSubsession *) clientData;

    subsess->checkForAuxSDPLine1();
}

void H265VideoFileServerMediaSubsession::checkForAuxSDPLine1()
{
    char const* dasl;

    if (fAuxSDPLine != NULL) {
        setDoneFlag();
    } else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
        fAuxSDPLine   = strDup(dasl);
        fDummyRTPSink = NULL;
        setDoneFlag();
    } else if (!fDoneFlag) {
        int uSecsToDelay = 100000;
        nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
                                                                 (TaskFunc *) checkForAuxSDPLine, this);
    }
}

char const * H265VideoFileServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
    if (fAuxSDPLine != NULL) {
        return fAuxSDPLine;
    }
    if (fDummyRTPSink == NULL) {
        fDummyRTPSink = rtpSink;
        fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);
        checkForAuxSDPLine(this);
    }
    envir().taskScheduler().doEventLoop(&fDoneFlag);
    return fAuxSDPLine;
}

FramedSource * H265VideoFileServerMediaSubsession::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    estBitrate = 500;
    ByteStreamFileSource* fileSource = ByteStreamFileSource::createNew(envir(), fFileName);
    if (fileSource == NULL) {
        return NULL;
    }
    fFileSize = fileSource->fileSize();
    return H265VideoStreamFramer::createNew(envir(), fileSource);
}

RTPSink * H265VideoFileServerMediaSubsession
::createNewRTPSink(Groupsock*    rtpGroupsock,
                   unsigned char rtpPayloadTypeIfDynamic,
                   FramedSource *)
{
    return H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
