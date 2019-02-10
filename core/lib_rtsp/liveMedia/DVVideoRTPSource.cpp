#include "DVVideoRTPSource.hh"
DVVideoRTPSource * DVVideoRTPSource::createNew(UsageEnvironment& env,
                                               Groupsock*      RTPgs,
                                               unsigned char   rtpPayloadFormat,
                                               unsigned        rtpTimestampFrequency)
{
    return new DVVideoRTPSource(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency);
}

DVVideoRTPSource::DVVideoRTPSource(UsageEnvironment& env,
                                   Groupsock*      rtpGS,
                                   unsigned char   rtpPayloadFormat,
                                   unsigned        rtpTimestampFrequency)
    : MultiFramedRTPSource(env, rtpGS,
                           rtpPayloadFormat, rtpTimestampFrequency)
{}

DVVideoRTPSource::~DVVideoRTPSource()
{}

#define DV_DIF_BLOCK_SIZE 80
#define DV_SECTION_HEADER 0x1F
Boolean DVVideoRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    unsigned const packetSize = packet->dataSize();

    if (packetSize < DV_DIF_BLOCK_SIZE) {
        return False;
    }
    u_int8_t const* data = packet->data();
    fCurrentPacketBeginsFrame    = data[0] == DV_SECTION_HEADER && (data[1] & 0xf8) == 0 && data[2] == 0;
    fCurrentPacketCompletesFrame = packet->rtpMarkerBit();
    resultSpecialHeaderSize      = 0;
    return True;
}

char const * DVVideoRTPSource::MIMEtype() const
{
    return "video/DV";
}
