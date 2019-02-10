#include "MPEG1or2AudioRTPSource.hh"
MPEG1or2AudioRTPSource * MPEG1or2AudioRTPSource::createNew(UsageEnvironment& env,
                                                           Groupsock*      RTPgs,
                                                           unsigned char   rtpPayloadFormat,
                                                           unsigned        rtpTimestampFrequency)
{
    return new MPEG1or2AudioRTPSource(env, RTPgs, rtpPayloadFormat,
                                      rtpTimestampFrequency);
}

MPEG1or2AudioRTPSource::MPEG1or2AudioRTPSource(UsageEnvironment& env,
                                               Groupsock*      rtpGS,
                                               unsigned char   rtpPayloadFormat,
                                               unsigned        rtpTimestampFrequency)
    : MultiFramedRTPSource(env, rtpGS,
                           rtpPayloadFormat, rtpTimestampFrequency)
{}

MPEG1or2AudioRTPSource::~MPEG1or2AudioRTPSource()
{}

Boolean MPEG1or2AudioRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    if (packet->dataSize() < 4) {
        return False;
    }
    resultSpecialHeaderSize = 4;
    return True;
}

char const * MPEG1or2AudioRTPSource::MIMEtype() const
{
    return "audio/MPEG";
}
