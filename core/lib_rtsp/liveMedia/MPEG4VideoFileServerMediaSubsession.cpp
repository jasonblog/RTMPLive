#include "MPEG4VideoFileServerMediaSubsession.hh"
#include "MPEG4ESVideoRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "MPEG4VideoStreamFramer.hh"
MPEG4VideoFileServerMediaSubsession * MPEG4VideoFileServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                                     char const*     fileName,
                                                                                     Boolean         reuseFirstSource)
{
    return new MPEG4VideoFileServerMediaSubsession(env, fileName, reuseFirstSource);
}

MPEG4VideoFileServerMediaSubsession
::MPEG4VideoFileServerMediaSubsession(UsageEnvironment& env,
                                      char const* fileName, Boolean reuseFirstSource)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource),
    fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL)
{}

MPEG4VideoFileServerMediaSubsession::~MPEG4VideoFileServerMediaSubsession()
{
    delete[] fAuxSDPLine;
}

static void afterPlayingDummy(void* clientData)
{
    MPEG4VideoFileServerMediaSubsession* subsess =
        (MPEG4VideoFileServerMediaSubsession *) clientData;

    subsess->afterPlayingDummy1();
}

void MPEG4VideoFileServerMediaSubsession::afterPlayingDummy1()
{
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData)
{
    MPEG4VideoFileServerMediaSubsession* subsess =
        (MPEG4VideoFileServerMediaSubsession *) clientData;

    subsess->checkForAuxSDPLine1();
}

void MPEG4VideoFileServerMediaSubsession::checkForAuxSDPLine1()
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

char const * MPEG4VideoFileServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
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

FramedSource * MPEG4VideoFileServerMediaSubsession
::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    estBitrate = 500;
    ByteStreamFileSource* fileSource =
        ByteStreamFileSource::createNew(envir(), fFileName);
    if (fileSource == NULL) {
        return NULL;
    }
    fFileSize = fileSource->fileSize();
    return MPEG4VideoStreamFramer::createNew(envir(), fileSource);
}

RTPSink * MPEG4VideoFileServerMediaSubsession
::createNewRTPSink(Groupsock*    rtpGroupsock,
                   unsigned char rtpPayloadTypeIfDynamic,
                   FramedSource *)
{
    return MPEG4ESVideoRTPSink::createNew(envir(), rtpGroupsock,
                                          rtpPayloadTypeIfDynamic);
}
