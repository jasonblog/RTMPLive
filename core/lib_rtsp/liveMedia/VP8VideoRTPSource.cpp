#include "VP8VideoRTPSource.hh"
VP8VideoRTPSource * VP8VideoRTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                 unsigned char rtpPayloadFormat,
                                                 unsigned rtpTimestampFrequency)
{
    return new VP8VideoRTPSource(env, RTPgs, rtpPayloadFormat,
                                 rtpTimestampFrequency);
}

VP8VideoRTPSource
::VP8VideoRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                    unsigned char rtpPayloadFormat,
                    unsigned rtpTimestampFrequency)
    : MultiFramedRTPSource(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency)
{}

VP8VideoRTPSource::~VP8VideoRTPSource()
{}

Boolean VP8VideoRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    unsigned char* headerStart = packet->data();
    unsigned packetSize        = packet->dataSize();

    if (packetSize == 0) {
        return False;
    }
    resultSpecialHeaderSize = 1;
    u_int8_t const byte1  = headerStart[0];
    Boolean const X       = (byte1 & 0x80) != 0;
    Boolean const S       = (byte1 & 0x10) != 0;
    u_int8_t const PartID = byte1 & 0x0F;
    fCurrentPacketBeginsFrame    = S && PartID == 0;
    fCurrentPacketCompletesFrame = packet->rtpMarkerBit();
    if (X) {
        ++resultSpecialHeaderSize;
        u_int8_t const byte2 = headerStart[1];
        Boolean const I      = (byte2 & 0x80) != 0;
        Boolean const L      = (byte2 & 0x40) != 0;
        Boolean const T      = (byte2 & 0x20) != 0;
        Boolean const K      = (byte2 & 0x10) != 0;
        if (I) {
            ++resultSpecialHeaderSize;
            if (headerStart[2] & 0x80) {
                ++resultSpecialHeaderSize;
            }
        }
        if (L) {
            ++resultSpecialHeaderSize;
        }
        if (T || K) {
            ++resultSpecialHeaderSize;
        }
    }
    return True;
} // VP8VideoRTPSource::processSpecialHeader

char const * VP8VideoRTPSource::MIMEtype() const
{
    return "video/VP8";
}
