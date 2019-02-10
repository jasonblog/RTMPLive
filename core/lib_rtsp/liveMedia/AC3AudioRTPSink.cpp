#include "AC3AudioRTPSink.hh"
AC3AudioRTPSink::AC3AudioRTPSink(UsageEnvironment& env, Groupsock* RTPgs,
                                 u_int8_t rtpPayloadFormat,
                                 u_int32_t rtpTimestampFrequency)
    : AudioRTPSink(env, RTPgs, rtpPayloadFormat,
                   rtpTimestampFrequency, "AC3"),
    fTotNumFragmentsUsed(0)
{}

AC3AudioRTPSink::~AC3AudioRTPSink()
{}

AC3AudioRTPSink * AC3AudioRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                             u_int8_t rtpPayloadFormat,
                                             u_int32_t rtpTimestampFrequency)
{
    return new AC3AudioRTPSink(env, RTPgs,
                               rtpPayloadFormat, rtpTimestampFrequency);
}

Boolean AC3AudioRTPSink
::frameCanAppearAfterPacketStart(unsigned char const *,
                                 unsigned) const
{
    return False;
}

void AC3AudioRTPSink
::doSpecialFrameHandling(unsigned       fragmentationOffset,
                         unsigned char* frameStart,
                         unsigned       numBytesInFrame,
                         struct timeval framePresentationTime,
                         unsigned       numRemainingBytes)
{
    unsigned char headers[2];
    Boolean isFragment = numRemainingBytes > 0 || fragmentationOffset > 0;

    if (!isFragment) {
        headers[0] = 0;
        headers[1] = 1;
    } else {
        if (fragmentationOffset > 0) {
            headers[0] = 3;
        } else {
            unsigned const totalFrameSize   = fragmentationOffset + numBytesInFrame + numRemainingBytes;
            unsigned const fiveEighthsPoint = totalFrameSize / 2 + totalFrameSize / 8;
            headers[0] = numBytesInFrame >= fiveEighthsPoint ? 1 : 2;
            fTotNumFragmentsUsed = (totalFrameSize + (numBytesInFrame - 1)) / numBytesInFrame;
        }
        headers[1] = fTotNumFragmentsUsed;
    }
    setSpecialHeaderBytes(headers, sizeof headers);
    if (numRemainingBytes == 0) {
        setMarkerBit();
    }
    MultiFramedRTPSink::doSpecialFrameHandling(fragmentationOffset,
                                               frameStart, numBytesInFrame,
                                               framePresentationTime,
                                               numRemainingBytes);
}

unsigned AC3AudioRTPSink::specialHeaderSize() const
{
    return 2;
}
