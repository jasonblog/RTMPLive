#include "ADTSAudioFileServerMediaSubsession.hh"
#include "ADTSAudioFileSource.hh"
#include "MPEG4GenericRTPSink.hh"
ADTSAudioFileServerMediaSubsession * ADTSAudioFileServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                                   char const*     fileName,
                                                                                   Boolean         reuseFirstSource)
{
    return new ADTSAudioFileServerMediaSubsession(env, fileName, reuseFirstSource);
}

ADTSAudioFileServerMediaSubsession
::ADTSAudioFileServerMediaSubsession(UsageEnvironment& env,
                                     char const* fileName, Boolean reuseFirstSource)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource)
{}

ADTSAudioFileServerMediaSubsession
::~ADTSAudioFileServerMediaSubsession()
{}

FramedSource * ADTSAudioFileServerMediaSubsession
::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    estBitrate = 96;
    return ADTSAudioFileSource::createNew(envir(), fFileName);
}

RTPSink * ADTSAudioFileServerMediaSubsession
::createNewRTPSink(Groupsock*    rtpGroupsock,
                   unsigned char rtpPayloadTypeIfDynamic,
                   FramedSource* inputSource)
{
    ADTSAudioFileSource* adtsSource = (ADTSAudioFileSource *) inputSource;

    return MPEG4GenericRTPSink::createNew(envir(), rtpGroupsock,
                                          rtpPayloadTypeIfDynamic,
                                          adtsSource->samplingFrequency(),
                                          "audio", "AAC-hbr", adtsSource->configStr(),
                                          adtsSource->numChannels());
}
