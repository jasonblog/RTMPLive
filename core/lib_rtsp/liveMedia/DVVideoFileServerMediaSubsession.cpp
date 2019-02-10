#include "DVVideoFileServerMediaSubsession.hh"
#include "DVVideoRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "DVVideoStreamFramer.hh"
DVVideoFileServerMediaSubsession * DVVideoFileServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                               char const*     fileName,
                                                                               Boolean         reuseFirstSource)
{
    return new DVVideoFileServerMediaSubsession(env, fileName, reuseFirstSource);
}

DVVideoFileServerMediaSubsession
::DVVideoFileServerMediaSubsession(UsageEnvironment& env, char const* fileName, Boolean reuseFirstSource)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource),
    fFileDuration(0.0)
{}

DVVideoFileServerMediaSubsession::~DVVideoFileServerMediaSubsession()
{}

FramedSource * DVVideoFileServerMediaSubsession
::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    ByteStreamFileSource* fileSource = ByteStreamFileSource::createNew(envir(), fFileName);

    if (fileSource == NULL) {
        return NULL;
    }
    fFileSize = fileSource->fileSize();
    DVVideoStreamFramer* framer = DVVideoStreamFramer::createNew(envir(), fileSource, True);
    unsigned frameSize;
    double frameDuration;
    if (framer->getFrameParameters(frameSize, frameDuration)) {
        fFileDuration = (float) (((int64_t) fFileSize * frameDuration) / (frameSize * 1000000.0));
        estBitrate    = (unsigned) ((8000.0 * frameSize) / frameDuration);
    } else {
        estBitrate = 50000;
    }
    return framer;
}

RTPSink * DVVideoFileServerMediaSubsession::createNewRTPSink(Groupsock*    rtpGroupsock,
                                                             unsigned char rtpPayloadTypeIfDynamic,
                                                             FramedSource *)
{
    return DVVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

char const * DVVideoFileServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
    return ((DVVideoRTPSink *) rtpSink)->auxSDPLineFromFramer((DVVideoStreamFramer *) inputSource);
}

float DVVideoFileServerMediaSubsession::duration() const
{
    return fFileDuration;
}

void DVVideoFileServerMediaSubsession
::seekStreamSource(FramedSource* inputSource, double& seekNPT, double streamDuration, u_int64_t& numBytes)
{
    DVVideoStreamFramer* framer      = (DVVideoStreamFramer *) inputSource;
    ByteStreamFileSource* fileSource = (ByteStreamFileSource *) (framer->inputSource());

    if (fFileDuration > 0.0) {
        u_int64_t seekByteNumber = (u_int64_t) (((int64_t) fFileSize * seekNPT) / fFileDuration);
        numBytes = (u_int64_t) (((int64_t) fFileSize * streamDuration) / fFileDuration);
        fileSource->seekToByteAbsolute(seekByteNumber, numBytes);
    }
}

void DVVideoFileServerMediaSubsession
::setStreamSourceDuration(FramedSource* inputSource, double streamDuration, u_int64_t& numBytes)
{
    DVVideoStreamFramer* framer      = (DVVideoStreamFramer *) inputSource;
    ByteStreamFileSource* fileSource = (ByteStreamFileSource *) (framer->inputSource());

    if (fFileDuration > 0.0) {
        numBytes = (u_int64_t) (((int64_t) fFileSize * streamDuration) / fFileDuration);
        fileSource->seekToByteRelative(0, numBytes);
    }
}
