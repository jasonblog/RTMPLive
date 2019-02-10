#include "TextRTPSink.hh"
TextRTPSink::TextRTPSink(UsageEnvironment& env,
                         Groupsock* rtpgs, unsigned char rtpPayloadType,
                         unsigned rtpTimestampFrequency,
                         char const* rtpPayloadFormatName)
    : MultiFramedRTPSink(env, rtpgs, rtpPayloadType, rtpTimestampFrequency,
                         rtpPayloadFormatName)
{}

TextRTPSink::~TextRTPSink()
{}

char const * TextRTPSink::sdpMediaType() const
{
    return "text";
}
