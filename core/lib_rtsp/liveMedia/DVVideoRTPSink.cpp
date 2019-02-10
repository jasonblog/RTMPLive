#include "DVVideoRTPSink.hh"
DVVideoRTPSink
::DVVideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat)
    : VideoRTPSink(env, RTPgs, rtpPayloadFormat, 90000, "DV"),
    fFmtpSDPLine(NULL)
{}

DVVideoRTPSink::~DVVideoRTPSink()
{
    delete[] fFmtpSDPLine;
}

DVVideoRTPSink * DVVideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat)
{
    return new DVVideoRTPSink(env, RTPgs, rtpPayloadFormat);
}

Boolean DVVideoRTPSink::sourceIsCompatibleWithUs(MediaSource& source)
{
    return source.isDVVideoStreamFramer();
}

void DVVideoRTPSink::doSpecialFrameHandling(unsigned fragmentationOffset,
                                            unsigned char *,
                                            unsigned,
                                            struct timeval framePresentationTime,
                                            unsigned       numRemainingBytes)
{
    if (numRemainingBytes == 0) {
        setMarkerBit();
    }
    setTimestamp(framePresentationTime);
}

unsigned DVVideoRTPSink::computeOverflowForNewFrame(unsigned newFrameSize) const
{
    unsigned initialOverflow   = MultiFramedRTPSink::computeOverflowForNewFrame(newFrameSize);
    unsigned numFrameBytesUsed = newFrameSize - initialOverflow;

    initialOverflow += numFrameBytesUsed % DV_DIF_BLOCK_SIZE;
    return initialOverflow;
}

char const * DVVideoRTPSink::auxSDPLine()
{
    DVVideoStreamFramer* framerSource = (DVVideoStreamFramer *) fSource;

    if (framerSource == NULL) {
        return NULL;
    }
    return auxSDPLineFromFramer(framerSource);
}

char const * DVVideoRTPSink::auxSDPLineFromFramer(DVVideoStreamFramer* framerSource)
{
    char const * const profileName = framerSource->profileName();

    if (profileName == NULL) {
        return NULL;
    }
    char const * const fmtpSDPFmt = "a=fmtp:%d encode=%s;audio=bundled\r\n";
    unsigned fmtpSDPFmtSize       = strlen(fmtpSDPFmt)
                                    + 3
                                    + strlen(profileName);
    delete[] fFmtpSDPLine;
    fFmtpSDPLine = new char[fmtpSDPFmtSize];
    sprintf(fFmtpSDPLine, fmtpSDPFmt, rtpPayloadType(), profileName);
    return fFmtpSDPLine;
}
