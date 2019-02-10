#include "MPEG1or2VideoRTPSink.hh"
#include "MPEG1or2VideoStreamFramer.hh"
MPEG1or2VideoRTPSink::MPEG1or2VideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs)
    : VideoRTPSink(env, RTPgs, 32, 90000, "MPV")
{
    fPictureState.temporal_reference  = 0;
    fPictureState.picture_coding_type = fPictureState.vector_code_bits = 0;
}

MPEG1or2VideoRTPSink::~MPEG1or2VideoRTPSink()
{}

MPEG1or2VideoRTPSink * MPEG1or2VideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs)
{
    return new MPEG1or2VideoRTPSink(env, RTPgs);
}

Boolean MPEG1or2VideoRTPSink::sourceIsCompatibleWithUs(MediaSource& source)
{
    return source.isMPEG1or2VideoStreamFramer();
}

Boolean MPEG1or2VideoRTPSink::allowFragmentationAfterStart() const
{
    return True;
}

Boolean MPEG1or2VideoRTPSink
::frameCanAppearAfterPacketStart(unsigned char const* frameStart,
                                 unsigned             numBytesInFrame) const
{
    if (!fPreviousFrameWasSlice) {
        return True;
    }
    return numBytesInFrame >= 4 &&
           frameStart[0] == 0 && frameStart[1] == 0 && frameStart[2] == 1 &&
           frameStart[3] >= 1 && frameStart[3] <= 0xAF;
}

#define VIDEO_SEQUENCE_HEADER_START_CODE 0x000001B3
#define PICTURE_START_CODE               0x00000100
void MPEG1or2VideoRTPSink
::doSpecialFrameHandling(unsigned       fragmentationOffset,
                         unsigned char* frameStart,
                         unsigned       numBytesInFrame,
                         struct timeval framePresentationTime,
                         unsigned       numRemainingBytes)
{
    Boolean thisFrameIsASlice = False;

    if (isFirstFrameInPacket()) {
        fSequenceHeaderPresent = fPacketBeginsSlice = fPacketEndsSlice = False;
    }
    if (fragmentationOffset == 0) {
        if (numBytesInFrame < 4) {
            return;
        }
        unsigned startCode = (frameStart[0] << 24) | (frameStart[1] << 16)
                             | (frameStart[2] << 8) | frameStart[3];
        if (startCode == VIDEO_SEQUENCE_HEADER_START_CODE) {
            fSequenceHeaderPresent = True;
        } else if (startCode == PICTURE_START_CODE) {
            if (numBytesInFrame < 8) {
                return;
            }
            unsigned next4Bytes = (frameStart[4] << 24) | (frameStart[5] << 16)
                                  | (frameStart[6] << 8) | frameStart[7];
            unsigned char byte8 = numBytesInFrame == 8 ? 0 : frameStart[8];
            fPictureState.temporal_reference  = (next4Bytes & 0xFFC00000) >> (32 - 10);
            fPictureState.picture_coding_type = (next4Bytes & 0x00380000) >> (32 - (10 + 3));
            unsigned char FBV, BFC, FFV, FFC;
            FBV = BFC = FFV = FFC = 0;
            switch (fPictureState.picture_coding_type) {
                case 3:
                    FBV = (byte8 & 0x40) >> 6;
                    BFC = (byte8 & 0x38) >> 3;
                case 2:
                    FFV = (next4Bytes & 0x00000004) >> 2;
                    FFC = ((next4Bytes & 0x00000003) << 1) | ((byte8 & 0x80) >> 7);
            }
            fPictureState.vector_code_bits = (FBV << 7) | (BFC << 4) | (FFV << 3) | FFC;
        } else if ((startCode & 0xFFFFFF00) == 0x00000100) {
            unsigned char lastCodeByte = startCode & 0xFF;
            if (lastCodeByte <= 0xAF) {
                thisFrameIsASlice = True;
            } else {}
        } else {
            envir() << "Warning: MPEG1or2VideoRTPSink::doSpecialFrameHandling saw strange first 4 bytes "
                    << (void *) startCode << ", but we're not a fragment\n";
        }
    } else {
        thisFrameIsASlice = True;
    }
    if (thisFrameIsASlice) {
        fPacketBeginsSlice = (fragmentationOffset == 0);
        fPacketEndsSlice   = (numRemainingBytes == 0);
    }
    unsigned videoSpecificHeader =
        (fPictureState.temporal_reference << 16)
        |(fSequenceHeaderPresent << 13)
        |(fPacketBeginsSlice << 12)
        |(fPacketEndsSlice << 11)
        |(fPictureState.picture_coding_type << 8)
        |fPictureState.vector_code_bits;
    setSpecialHeaderWord(videoSpecificHeader);
    setTimestamp(framePresentationTime);
    MPEG1or2VideoStreamFramer* framerSource = (MPEG1or2VideoStreamFramer *) fSource;
    if (framerSource != NULL && framerSource->pictureEndMarker() &&
        numRemainingBytes == 0)
    {
        setMarkerBit();
        framerSource->pictureEndMarker() = False;
    }
    fPreviousFrameWasSlice = thisFrameIsASlice;
} // MPEG1or2VideoRTPSink::doSpecialFrameHandling

unsigned MPEG1or2VideoRTPSink::specialHeaderSize() const
{
    return 4;
}
