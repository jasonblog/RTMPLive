#include "H264VideoFileServerMediaSubsession.hh"
#include "H264VideoRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "H264VideoStreamFramer.hh"
H264VideoFileServerMediaSubsession * H264VideoFileServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                                   char const*     fileName,
                                                                                   Boolean         reuseFirstSource)
{
    return new H264VideoFileServerMediaSubsession(env, fileName, reuseFirstSource);
}

H264VideoFileServerMediaSubsession::H264VideoFileServerMediaSubsession(UsageEnvironment& env,
                                                                       char const* fileName, Boolean reuseFirstSource)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource),
    fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL)
{}

H264VideoFileServerMediaSubsession::~H264VideoFileServerMediaSubsession()
{
    delete[] fAuxSDPLine;
}

static void afterPlayingDummy(void* clientData)
{
    H264VideoFileServerMediaSubsession* subsess = (H264VideoFileServerMediaSubsession *) clientData;

    subsess->afterPlayingDummy1();
}

void H264VideoFileServerMediaSubsession::afterPlayingDummy1()
{
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData)
{
    H264VideoFileServerMediaSubsession* subsess = (H264VideoFileServerMediaSubsession *) clientData;

    subsess->checkForAuxSDPLine1();
}

void H264VideoFileServerMediaSubsession::checkForAuxSDPLine1()
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

char const * H264VideoFileServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
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

FramedSource * H264VideoFileServerMediaSubsession::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    estBitrate = 500;
    ByteStreamFileSource* fileSource = ByteStreamFileSource::createNew(envir(), fFileName);
    if (fileSource == NULL) {
        return NULL;
    }
    fFileSize = fileSource->fileSize();
    return H264VideoStreamFramer::createNew(envir(), fileSource);
}

RTPSink * H264VideoFileServerMediaSubsession
::createNewRTPSink(Groupsock*    rtpGroupsock,
                   unsigned char rtpPayloadTypeIfDynamic,
                   FramedSource *)
{
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
