#include "AC3AudioFileServerMediaSubsession.hh"
#include "ByteStreamFileSource.hh"
#include "AC3AudioStreamFramer.hh"
#include "AC3AudioRTPSink.hh"
AC3AudioFileServerMediaSubsession * AC3AudioFileServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                                 char const*     fileName,
                                                                                 Boolean         reuseFirstSource)
{
    return new AC3AudioFileServerMediaSubsession(env, fileName, reuseFirstSource);
}

AC3AudioFileServerMediaSubsession
::AC3AudioFileServerMediaSubsession(UsageEnvironment& env,
                                    char const* fileName, Boolean reuseFirstSource)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource)
{}

AC3AudioFileServerMediaSubsession::~AC3AudioFileServerMediaSubsession()
{}

FramedSource * AC3AudioFileServerMediaSubsession
::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    estBitrate = 48;
    ByteStreamFileSource* fileSource = ByteStreamFileSource::createNew(envir(), fFileName);
    if (fileSource == NULL) {
        return NULL;
    }
    return AC3AudioStreamFramer::createNew(envir(), fileSource);
}

RTPSink * AC3AudioFileServerMediaSubsession
::createNewRTPSink(Groupsock*    rtpGroupsock,
                   unsigned char rtpPayloadTypeIfDynamic,
                   FramedSource* inputSource)
{
    AC3AudioStreamFramer* audioSource = (AC3AudioStreamFramer *) inputSource;

    return AC3AudioRTPSink::createNew(envir(), rtpGroupsock,
                                      rtpPayloadTypeIfDynamic,
                                      audioSource->samplingRate());
}
