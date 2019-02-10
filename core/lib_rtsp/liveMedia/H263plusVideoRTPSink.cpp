#include "H263plusVideoRTPSink.hh"
H263plusVideoRTPSink
::H263plusVideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs,
                       unsigned char rtpPayloadFormat,
                       u_int32_t rtpTimestampFrequency)
    : VideoRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, "H263-1998")
{}

H263plusVideoRTPSink::~H263plusVideoRTPSink()
{}

H263plusVideoRTPSink * H263plusVideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                       unsigned char rtpPayloadFormat,
                                                       u_int32_t rtpTimestampFrequency)
{
    return new H263plusVideoRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency);
}

Boolean H263plusVideoRTPSink
::frameCanAppearAfterPacketStart(unsigned char const *,
                                 unsigned) const
{
    return False;
}

void H263plusVideoRTPSink
::doSpecialFrameHandling(unsigned       fragmentationOffset,
                         unsigned char* frameStart,
                         unsigned       numBytesInFrame,
                         struct timeval framePresentationTime,
                         unsigned       numRemainingBytes)
{
    if (fragmentationOffset == 0) {
        unsigned short specialHeader = 0x0400;
        if (numBytesInFrame < 2) {
            envir() << "H263plusVideoRTPSink::doSpecialFrameHandling(): bad frame size "
                    << numBytesInFrame << "\n";
            return;
        }
        if (frameStart[0] != 0 || frameStart[1] != 0) {
            envir() << "H263plusVideoRTPSink::doSpecialFrameHandling(): unexpected non-zero first two bytes!\n";
        }
        frameStart[0] = specialHeader >> 8;
        frameStart[1] = (unsigned char) specialHeader;
    } else {
        unsigned short specialHeader = 0;
        setSpecialHeaderBytes((unsigned char *) &specialHeader, 2);
    }
    if (numRemainingBytes == 0) {
        setMarkerBit();
    }
    setTimestamp(framePresentationTime);
}

unsigned H263plusVideoRTPSink::specialHeaderSize() const
{
    return (curFragmentationOffset() == 0) ? 0 : 2;
}
