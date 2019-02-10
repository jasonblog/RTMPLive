#include "AMRAudioRTPSink.hh"
#include "AMRAudioSource.hh"
AMRAudioRTPSink * AMRAudioRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                             unsigned char rtpPayloadFormat,
                                             Boolean sourceIsWideband,
                                             unsigned numChannelsInSource)
{
    return new AMRAudioRTPSink(env, RTPgs, rtpPayloadFormat,
                               sourceIsWideband, numChannelsInSource);
}

AMRAudioRTPSink
::AMRAudioRTPSink(UsageEnvironment& env, Groupsock* RTPgs,
                  unsigned char rtpPayloadFormat,
                  Boolean sourceIsWideband, unsigned numChannelsInSource)
    : AudioRTPSink(env, RTPgs, rtpPayloadFormat,
                   sourceIsWideband ? 16000 : 8000,
                   sourceIsWideband ? "AMR-WB" : "AMR",
                   numChannelsInSource),
    fSourceIsWideband(sourceIsWideband), fFmtpSDPLine(NULL)
{}

AMRAudioRTPSink::~AMRAudioRTPSink()
{
    delete[] fFmtpSDPLine;
}

Boolean AMRAudioRTPSink::sourceIsCompatibleWithUs(MediaSource& source)
{
    if (!source.isAMRAudioSource()) {
        return False;
    }
    AMRAudioSource& amrSource = (AMRAudioSource&) source;
    if ((amrSource.isWideband()^fSourceIsWideband) != 0) {
        return False;
    }
    if (amrSource.numChannels() != numChannels()) {
        return False;
    }
    if (amrSource.numChannels() > 1) {
        envir() << "AMRAudioRTPSink: Warning: Input source has " << amrSource.numChannels()
                <<
            " audio channels.  In the current implementation, the multi-frame frame-block will be split over multiple RTP packets\n";
    }
    return True;
}

void AMRAudioRTPSink::doSpecialFrameHandling(unsigned       fragmentationOffset,
                                             unsigned char* frameStart,
                                             unsigned       numBytesInFrame,
                                             struct timeval framePresentationTime,
                                             unsigned       numRemainingBytes)
{
    if (isFirstPacket() && isFirstFrameInPacket()) {
        setMarkerBit();
    }
    if (isFirstFrameInPacket()) {
        u_int8_t payloadHeader = 0xF0;
        setSpecialHeaderBytes(&payloadHeader, 1, 0);
    }
    AMRAudioSource* amrSource = (AMRAudioSource *) fSource;
    if (amrSource == NULL) {
        return;
    }
    u_int8_t toc = amrSource->lastFrameHeader();
    toc &= ~0x80;
    setSpecialHeaderBytes(&toc, 1, 1 + numFramesUsedSoFar());
    MultiFramedRTPSink::doSpecialFrameHandling(fragmentationOffset,
                                               frameStart, numBytesInFrame,
                                               framePresentationTime,
                                               numRemainingBytes);
}

Boolean AMRAudioRTPSink
::frameCanAppearAfterPacketStart(unsigned char const *,
                                 unsigned) const
{
    return False;
}

unsigned AMRAudioRTPSink::specialHeaderSize() const
{
    return 2;
}

char const * AMRAudioRTPSink::auxSDPLine()
{
    if (fFmtpSDPLine == NULL) {
        char buf[100];
        sprintf(buf, "a=fmtp:%d octet-align=1\r\n", rtpPayloadType());
        delete[] fFmtpSDPLine;
        fFmtpSDPLine = strDup(buf);
    }
    return fFmtpSDPLine;
}
