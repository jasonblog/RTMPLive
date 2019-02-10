#include "MPEG1or2VideoRTPSource.hh"
MPEG1or2VideoRTPSource * MPEG1or2VideoRTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                           unsigned char rtpPayloadFormat,
                                                           unsigned rtpTimestampFrequency)
{
    return new MPEG1or2VideoRTPSource(env, RTPgs, rtpPayloadFormat,
                                      rtpTimestampFrequency);
}

MPEG1or2VideoRTPSource::MPEG1or2VideoRTPSource(UsageEnvironment& env,
                                               Groupsock*      RTPgs,
                                               unsigned char   rtpPayloadFormat,
                                               unsigned        rtpTimestampFrequency)
    : MultiFramedRTPSource(env, RTPgs,
                           rtpPayloadFormat, rtpTimestampFrequency)
{}

MPEG1or2VideoRTPSource::~MPEG1or2VideoRTPSource()
{}

Boolean MPEG1or2VideoRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    if (packet->dataSize() < 4) {
        return False;
    }
    u_int32_t header = ntohl(*(u_int32_t *) (packet->data()));
    u_int32_t sBit   = header & 0x00002000;
    u_int32_t bBit   = header & 0x00001000;
    u_int32_t eBit   = header & 0x00000800;
    fCurrentPacketBeginsFrame    = (sBit | bBit) != 0;
    fCurrentPacketCompletesFrame = ((sBit != 0) && (bBit == 0)) || (eBit != 0);
    resultSpecialHeaderSize      = 4;
    return True;
}

Boolean MPEG1or2VideoRTPSource
::packetIsUsableInJitterCalculation(unsigned char* packet,
                                    unsigned       packetSize)
{
    if (packetSize < 4) {
        return False;
    }
    unsigned header = ntohl(*(u_int32_t *) packet);
    unsigned short pictureType = (header >> 8) & 0x7;
    if (pictureType == 1) {
        return True;
    } else {
        return False;
    }
}

char const * MPEG1or2VideoRTPSource::MIMEtype() const
{
    return "video/MPEG";
}
