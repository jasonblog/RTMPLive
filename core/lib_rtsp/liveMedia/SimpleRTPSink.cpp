#include "SimpleRTPSink.hh"
SimpleRTPSink::SimpleRTPSink(UsageEnvironment& env, Groupsock* RTPgs,
                             unsigned char rtpPayloadFormat,
                             unsigned rtpTimestampFrequency,
                             char const* sdpMediaTypeString,
                             char const* rtpPayloadFormatName,
                             unsigned numChannels,
                             Boolean allowMultipleFramesPerPacket,
                             Boolean doNormalMBitRule)
    : MultiFramedRTPSink(env, RTPgs, rtpPayloadFormat,
                         rtpTimestampFrequency, rtpPayloadFormatName,
                         numChannels),
    fAllowMultipleFramesPerPacket(allowMultipleFramesPerPacket), fSetMBitOnNextPacket(False)
{
    fSDPMediaTypeString =
        strDup(sdpMediaTypeString == NULL ? "unknown" : sdpMediaTypeString);
    fSetMBitOnLastFrames = doNormalMBitRule && strcmp(fSDPMediaTypeString, "audio") != 0;
}

SimpleRTPSink::~SimpleRTPSink()
{
    delete[](char *) fSDPMediaTypeString;
}

SimpleRTPSink * SimpleRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                         unsigned char rtpPayloadFormat,
                                         unsigned rtpTimestampFrequency,
                                         char const* sdpMediaTypeString,
                                         char const* rtpPayloadFormatName,
                                         unsigned numChannels,
                                         Boolean allowMultipleFramesPerPacket,
                                         Boolean doNormalMBitRule)
{
    return new SimpleRTPSink(env, RTPgs,
                             rtpPayloadFormat, rtpTimestampFrequency,
                             sdpMediaTypeString, rtpPayloadFormatName,
                             numChannels,
                             allowMultipleFramesPerPacket,
                             doNormalMBitRule);
}

void SimpleRTPSink::doSpecialFrameHandling(unsigned       fragmentationOffset,
                                           unsigned char* frameStart,
                                           unsigned       numBytesInFrame,
                                           struct timeval framePresentationTime,
                                           unsigned       numRemainingBytes)
{
    if (numRemainingBytes == 0) {
        if (fSetMBitOnLastFrames) {
            setMarkerBit();
        }
    }
    if (fSetMBitOnNextPacket) {
        setMarkerBit();
        fSetMBitOnNextPacket = False;
    }
    MultiFramedRTPSink::doSpecialFrameHandling(fragmentationOffset,
                                               frameStart, numBytesInFrame,
                                               framePresentationTime,
                                               numRemainingBytes);
}

Boolean SimpleRTPSink::
frameCanAppearAfterPacketStart(unsigned char const *,
                               unsigned) const
{
    return fAllowMultipleFramesPerPacket;
}

char const * SimpleRTPSink::sdpMediaType() const
{
    return fSDPMediaTypeString;
}
