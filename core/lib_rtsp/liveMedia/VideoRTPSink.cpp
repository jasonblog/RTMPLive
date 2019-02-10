#include "VideoRTPSink.hh"
VideoRTPSink::VideoRTPSink(UsageEnvironment& env,
                           Groupsock* rtpgs, unsigned char rtpPayloadType,
                           unsigned rtpTimestampFrequency,
                           char const* rtpPayloadFormatName)
    : MultiFramedRTPSink(env, rtpgs, rtpPayloadType, rtpTimestampFrequency,
                         rtpPayloadFormatName)
{}

VideoRTPSink::~VideoRTPSink()
{}

char const * VideoRTPSink::sdpMediaType() const
{
    return "video";
}
