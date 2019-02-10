#include "H261VideoRTPSource.hh"
H261VideoRTPSource * H261VideoRTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                   unsigned char rtpPayloadFormat,
                                                   unsigned rtpTimestampFrequency)
{
    return new H261VideoRTPSource(env, RTPgs, rtpPayloadFormat,
                                  rtpTimestampFrequency);
}

H261VideoRTPSource
::H261VideoRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                     unsigned char rtpPayloadFormat,
                     unsigned rtpTimestampFrequency)
    : MultiFramedRTPSource(env, RTPgs,
                           rtpPayloadFormat, rtpTimestampFrequency),
    fLastSpecialHeader(0)
{}

H261VideoRTPSource::~H261VideoRTPSource()
{}

Boolean H261VideoRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    if (packet->dataSize() < 4) {
        return False;
    }
    unsigned char* headerStart = packet->data();
    fLastSpecialHeader =
        (headerStart[0] << 24) | (headerStart[1] << 16) | (headerStart[2] << 8) | headerStart[3];
    #ifdef DELIVER_COMPLETE_FRAMES
    fCurrentPacketBeginsFrame    = fCurrentPacketCompletesFrame;
    fCurrentPacketCompletesFrame = packet->rtpMarkerBit();
    #endif
    resultSpecialHeaderSize = 4;
    return True;
}

char const * H261VideoRTPSource::MIMEtype() const
{
    return "video/H261";
}
