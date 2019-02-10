#include "AMRAudioFileServerMediaSubsession.hh"
#include "AMRAudioRTPSink.hh"
#include "AMRAudioFileSource.hh"
AMRAudioFileServerMediaSubsession * AMRAudioFileServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                                 char const*     fileName,
                                                                                 Boolean         reuseFirstSource)
{
    return new AMRAudioFileServerMediaSubsession(env, fileName, reuseFirstSource);
}

AMRAudioFileServerMediaSubsession
::AMRAudioFileServerMediaSubsession(UsageEnvironment& env,
                                    char const* fileName, Boolean reuseFirstSource)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource)
{}

AMRAudioFileServerMediaSubsession
::~AMRAudioFileServerMediaSubsession()
{}

FramedSource * AMRAudioFileServerMediaSubsession
::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    estBitrate = 10;
    return AMRAudioFileSource::createNew(envir(), fFileName);
}

RTPSink * AMRAudioFileServerMediaSubsession
::createNewRTPSink(Groupsock*    rtpGroupsock,
                   unsigned char rtpPayloadTypeIfDynamic,
                   FramedSource* inputSource)
{
    AMRAudioFileSource* amrSource = (AMRAudioFileSource *) inputSource;

    return AMRAudioRTPSink::createNew(envir(), rtpGroupsock,
                                      rtpPayloadTypeIfDynamic,
                                      amrSource->isWideband(),
                                      amrSource->numChannels());
}
