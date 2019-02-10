#include "GSMAudioRTPSink.hh"
GSMAudioRTPSink::GSMAudioRTPSink(UsageEnvironment& env, Groupsock* RTPgs)
    : AudioRTPSink(env, RTPgs, 3, 8000, "GSM")
{}

GSMAudioRTPSink::~GSMAudioRTPSink()
{}

GSMAudioRTPSink * GSMAudioRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs)
{
    return new GSMAudioRTPSink(env, RTPgs);
}

Boolean GSMAudioRTPSink
::frameCanAppearAfterPacketStart(unsigned char const *,
                                 unsigned) const
{
    return numFramesUsedSoFar() < 5;
}
