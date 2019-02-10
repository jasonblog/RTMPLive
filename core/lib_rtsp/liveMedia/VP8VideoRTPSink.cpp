#include "VP8VideoRTPSink.hh"
VP8VideoRTPSink
::VP8VideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat)
    : VideoRTPSink(env, RTPgs, rtpPayloadFormat, 90000, "VP8")
{}

VP8VideoRTPSink::~VP8VideoRTPSink()
{}

VP8VideoRTPSink * VP8VideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat)
{
    return new VP8VideoRTPSink(env, RTPgs, rtpPayloadFormat);
}

Boolean VP8VideoRTPSink
::frameCanAppearAfterPacketStart(unsigned char const *,
                                 unsigned) const
{
    return False;
}

void VP8VideoRTPSink
::doSpecialFrameHandling(unsigned fragmentationOffset,
                         unsigned char *,
                         unsigned,
                         struct timeval framePresentationTime,
                         unsigned       numRemainingBytes)
{
    u_int8_t vp8PayloadDescriptor = fragmentationOffset == 0 ? 0x10 : 0x00;

    setSpecialHeaderBytes(&vp8PayloadDescriptor, 1);
    if (numRemainingBytes == 0) {
        setMarkerBit();
    }
    setTimestamp(framePresentationTime);
}

unsigned VP8VideoRTPSink::specialHeaderSize() const
{
    return 1;
}
