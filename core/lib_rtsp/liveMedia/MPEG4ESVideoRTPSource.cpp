#include "MPEG4ESVideoRTPSource.hh"
MPEG4ESVideoRTPSource * MPEG4ESVideoRTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                         unsigned char rtpPayloadFormat,
                                                         unsigned rtpTimestampFrequency)
{
    return new MPEG4ESVideoRTPSource(env, RTPgs, rtpPayloadFormat,
                                     rtpTimestampFrequency);
}

MPEG4ESVideoRTPSource
::MPEG4ESVideoRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                        unsigned char rtpPayloadFormat,
                        unsigned rtpTimestampFrequency)
    : MultiFramedRTPSource(env, RTPgs,
                           rtpPayloadFormat, rtpTimestampFrequency)
{}

MPEG4ESVideoRTPSource::~MPEG4ESVideoRTPSource()
{}

Boolean MPEG4ESVideoRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    fCurrentPacketBeginsFrame =
        packet->dataSize() >= 4 && (packet->data())[0] == 0 &&
        (packet->data())[1] == 0 && (packet->data())[2] == 1;
    fCurrentPacketCompletesFrame = packet->rtpMarkerBit();
    resultSpecialHeaderSize      = 0;
    return True;
}

char const * MPEG4ESVideoRTPSource::MIMEtype() const
{
    return "video/MP4V-ES";
}
