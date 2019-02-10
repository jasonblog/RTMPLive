#include "MPEG1or2VideoFileServerMediaSubsession.hh"
#include "MPEG1or2VideoRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "MPEG1or2VideoStreamFramer.hh"
MPEG1or2VideoFileServerMediaSubsession * MPEG1or2VideoFileServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                                           char const*     fileName,
                                                                                           Boolean         reuseFirstSource,
                                                                                           Boolean         iFramesOnly,
                                                                                           double          vshPeriod)
{
    return new MPEG1or2VideoFileServerMediaSubsession(env, fileName, reuseFirstSource,
                                                      iFramesOnly, vshPeriod);
}

MPEG1or2VideoFileServerMediaSubsession
::MPEG1or2VideoFileServerMediaSubsession(UsageEnvironment& env,
                                         char const*     fileName,
                                         Boolean         reuseFirstSource,
                                         Boolean         iFramesOnly,
                                         double          vshPeriod)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource),
    fIFramesOnly(iFramesOnly), fVSHPeriod(vshPeriod)
{}

MPEG1or2VideoFileServerMediaSubsession
::~MPEG1or2VideoFileServerMediaSubsession()
{}

FramedSource * MPEG1or2VideoFileServerMediaSubsession
::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    estBitrate = 500;
    ByteStreamFileSource* fileSource =
        ByteStreamFileSource::createNew(envir(), fFileName);
    if (fileSource == NULL) {
        return NULL;
    }
    fFileSize = fileSource->fileSize();
    return MPEG1or2VideoStreamFramer
           ::createNew(envir(), fileSource, fIFramesOnly, fVSHPeriod);
}

RTPSink * MPEG1or2VideoFileServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
                   unsigned char,
                   FramedSource *)
{
    return MPEG1or2VideoRTPSink::createNew(envir(), rtpGroupsock);
}
