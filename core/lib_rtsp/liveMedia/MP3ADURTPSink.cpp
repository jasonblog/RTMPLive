#include "MP3ADURTPSink.hh"
MP3ADURTPSink::MP3ADURTPSink(UsageEnvironment& env, Groupsock* RTPgs,
                             unsigned char RTPPayloadType)
    : AudioRTPSink(env, RTPgs, RTPPayloadType, 90000, "MPA-ROBUST")
{}

MP3ADURTPSink::~MP3ADURTPSink()
{}

MP3ADURTPSink * MP3ADURTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                         unsigned char RTPPayloadType)
{
    return new MP3ADURTPSink(env, RTPgs, RTPPayloadType);
}

static void badDataSize(UsageEnvironment& env, unsigned numBytesInFrame)
{
    env << "MP3ADURTPSink::doSpecialFrameHandling(): invalid size ("
        << numBytesInFrame << ") of non-fragmented input ADU!\n";
}

void MP3ADURTPSink::doSpecialFrameHandling(unsigned       fragmentationOffset,
                                           unsigned char* frameStart,
                                           unsigned       numBytesInFrame,
                                           struct timeval framePresentationTime,
                                           unsigned       numRemainingBytes)
{
    if (fragmentationOffset == 0) {
        unsigned aduDescriptorSize;
        if (numBytesInFrame < 1) {
            badDataSize(envir(), numBytesInFrame);
            return;
        }
        if (frameStart[0] & 0x40) {
            aduDescriptorSize = 2;
            if (numBytesInFrame < 2) {
                badDataSize(envir(), numBytesInFrame);
                return;
            }
            fCurADUSize = ((frameStart[0] & ~0xC0) << 8) | frameStart[1];
        } else {
            aduDescriptorSize = 1;
            fCurADUSize       = frameStart[0] & ~0x80;
        }
        if (frameStart[0] & 0x80) {
            envir() << "Unexpected \"C\" bit seen on non-fragment input ADU!\n";
            return;
        }
        unsigned expectedADUSize =
            fragmentationOffset + numBytesInFrame + numRemainingBytes
            - aduDescriptorSize;
        if (fCurADUSize != expectedADUSize) {
            envir() << "MP3ADURTPSink::doSpecialFrameHandling(): Warning: Input ADU size "
                    << expectedADUSize << " (=" << fragmentationOffset
                    << "+" << numBytesInFrame << "+" << numRemainingBytes
                    << "-" << aduDescriptorSize
                    << ") did not match the value (" << fCurADUSize
                    << ") in the ADU descriptor!\n";
            fCurADUSize = expectedADUSize;
        }
    } else {
        unsigned char aduDescriptor[2];
        aduDescriptor[0] = 0xC0 | (fCurADUSize >> 8);
        aduDescriptor[1] = fCurADUSize & 0xFF;
        setSpecialHeaderBytes(aduDescriptor, 2);
    }
    MultiFramedRTPSink::doSpecialFrameHandling(fragmentationOffset,
                                               frameStart, numBytesInFrame,
                                               framePresentationTime,
                                               numRemainingBytes);
} // MP3ADURTPSink::doSpecialFrameHandling

unsigned MP3ADURTPSink::specialHeaderSize() const
{
    unsigned specialHeaderSize = 0;

    if (curFragmentationOffset() > 0) {
        specialHeaderSize = 2;
    }
    return specialHeaderSize;
}
