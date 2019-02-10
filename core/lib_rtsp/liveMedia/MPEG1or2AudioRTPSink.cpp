#include "MPEG1or2AudioRTPSink.hh"
MPEG1or2AudioRTPSink::MPEG1or2AudioRTPSink(UsageEnvironment& env, Groupsock* RTPgs)
    : AudioRTPSink(env, RTPgs, 14, 90000, "MPA")
{}

MPEG1or2AudioRTPSink::~MPEG1or2AudioRTPSink()
{}

MPEG1or2AudioRTPSink * MPEG1or2AudioRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs)
{
    return new MPEG1or2AudioRTPSink(env, RTPgs);
}

void MPEG1or2AudioRTPSink::doSpecialFrameHandling(unsigned       fragmentationOffset,
                                                  unsigned char* frameStart,
                                                  unsigned       numBytesInFrame,
                                                  struct timeval framePresentationTime,
                                                  unsigned       numRemainingBytes)
{
    if (isFirstPacket() && isFirstFrameInPacket()) {
        setMarkerBit();
    }
    if (isFirstFrameInPacket()) {
        setSpecialHeaderWord(fragmentationOffset & 0xFFFF);
    }
    MultiFramedRTPSink::doSpecialFrameHandling(fragmentationOffset,
                                               frameStart, numBytesInFrame,
                                               framePresentationTime,
                                               numRemainingBytes);
}

unsigned MPEG1or2AudioRTPSink::specialHeaderSize() const
{
    return 4;
}
