#include "H263plusVideoFileServerMediaSubsession.hh"
#include "H263plusVideoRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "H263plusVideoStreamFramer.hh"
H263plusVideoFileServerMediaSubsession * H263plusVideoFileServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                                           char const*     fileName,
                                                                                           Boolean         reuseFirstSource)
{
    return new H263plusVideoFileServerMediaSubsession(env, fileName, reuseFirstSource);
}

H263plusVideoFileServerMediaSubsession
::H263plusVideoFileServerMediaSubsession(UsageEnvironment& env,
                                         char const*     fileName,
                                         Boolean         reuseFirstSource)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource)
{}

H263plusVideoFileServerMediaSubsession::~H263plusVideoFileServerMediaSubsession()
{}

FramedSource * H263plusVideoFileServerMediaSubsession
::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    estBitrate = 500;
    ByteStreamFileSource* fileSource = ByteStreamFileSource::createNew(envir(), fFileName);
    if (fileSource == NULL) {
        return NULL;
    }
    fFileSize = fileSource->fileSize();
    return H263plusVideoStreamFramer::createNew(envir(), fileSource);
}

RTPSink * H263plusVideoFileServerMediaSubsession::createNewRTPSink(Groupsock*    rtpGroupsock,
                                                                   unsigned char rtpPayloadTypeIfDynamic,
                                                                   FramedSource *)
{
    return H263plusVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
