#include "AudioRTPSink.hh"
AudioRTPSink::AudioRTPSink(UsageEnvironment& env,
                           Groupsock* rtpgs, unsigned char rtpPayloadType,
                           unsigned rtpTimestampFrequency,
                           char const* rtpPayloadFormatName,
                           unsigned numChannels)
    : MultiFramedRTPSink(env, rtpgs, rtpPayloadType, rtpTimestampFrequency,
                         rtpPayloadFormatName, numChannels)
{}

AudioRTPSink::~AudioRTPSink()
{}

char const * AudioRTPSink::sdpMediaType() const
{
    return "audio";
}
