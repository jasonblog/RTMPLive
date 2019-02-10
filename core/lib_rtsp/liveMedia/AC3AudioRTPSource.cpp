#include "AC3AudioRTPSource.hh"
AC3AudioRTPSource * AC3AudioRTPSource::createNew(UsageEnvironment& env,
                                                 Groupsock*      RTPgs,
                                                 unsigned char   rtpPayloadFormat,
                                                 unsigned        rtpTimestampFrequency)
{
    return new AC3AudioRTPSource(env, RTPgs, rtpPayloadFormat,
                                 rtpTimestampFrequency);
}

AC3AudioRTPSource::AC3AudioRTPSource(UsageEnvironment& env,
                                     Groupsock*      rtpGS,
                                     unsigned char   rtpPayloadFormat,
                                     unsigned        rtpTimestampFrequency)
    : MultiFramedRTPSource(env, rtpGS,
                           rtpPayloadFormat, rtpTimestampFrequency)
{}

AC3AudioRTPSource::~AC3AudioRTPSource()
{}

Boolean AC3AudioRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    unsigned char* headerStart = packet->data();
    unsigned packetSize        = packet->dataSize();

    if (packetSize < 2) {
        return False;
    }
    resultSpecialHeaderSize = 2;
    unsigned char FT = headerStart[0] & 0x03;
    fCurrentPacketBeginsFrame    = FT != 3;
    fCurrentPacketCompletesFrame = packet->rtpMarkerBit() || FT == 0;
    return True;
}

char const * AC3AudioRTPSource::MIMEtype() const
{
    return "audio/AC3";
}
