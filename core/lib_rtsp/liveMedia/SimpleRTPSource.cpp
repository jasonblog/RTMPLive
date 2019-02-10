#include "SimpleRTPSource.hh"
#include <string.h>
SimpleRTPSource * SimpleRTPSource::createNew(UsageEnvironment& env,
                                             Groupsock* RTPgs,
                                             unsigned char rtpPayloadFormat,
                                             unsigned rtpTimestampFrequency,
                                             char const* mimeTypeString,
                                             unsigned offset, Boolean doNormalMBitRule)
{
    return new SimpleRTPSource(env, RTPgs, rtpPayloadFormat,
                               rtpTimestampFrequency,
                               mimeTypeString, offset, doNormalMBitRule);
}

SimpleRTPSource
::SimpleRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                  unsigned char rtpPayloadFormat,
                  unsigned rtpTimestampFrequency,
                  char const* mimeTypeString,
                  unsigned offset, Boolean doNormalMBitRule)
    : MultiFramedRTPSource(env, RTPgs,
                           rtpPayloadFormat, rtpTimestampFrequency),
    fMIMEtypeString(strDup(mimeTypeString)), fOffset(offset)
{
    fUseMBitForFrameEnd = doNormalMBitRule && strncmp(mimeTypeString, "audio/", 6) != 0;
}

SimpleRTPSource::~SimpleRTPSource()
{
    delete[](char *) fMIMEtypeString;
}

Boolean SimpleRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    fCurrentPacketCompletesFrame =
        !fUseMBitForFrameEnd || packet->rtpMarkerBit();
    resultSpecialHeaderSize = fOffset;
    return True;
}

char const * SimpleRTPSource::MIMEtype() const
{
    if (fMIMEtypeString == NULL) {
        return MultiFramedRTPSource::MIMEtype();
    }
    return fMIMEtypeString;
}
