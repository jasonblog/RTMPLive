#include "MPEG4LATMAudioRTPSink.hh"
MPEG4LATMAudioRTPSink
::MPEG4LATMAudioRTPSink(UsageEnvironment& env, Groupsock* RTPgs,
                        u_int8_t rtpPayloadFormat,
                        u_int32_t rtpTimestampFrequency,
                        char const* streamMuxConfigString,
                        unsigned numChannels,
                        Boolean allowMultipleFramesPerPacket)
    : AudioRTPSink(env, RTPgs, rtpPayloadFormat,
                   rtpTimestampFrequency, "MP4A-LATM", numChannels),
    fStreamMuxConfigString(strDup(streamMuxConfigString)),
    fAllowMultipleFramesPerPacket(allowMultipleFramesPerPacket)
{
    char const* fmtpFmt =
        "a=fmtp:%d "
        "cpresent=0;config=%s\r\n";
    unsigned fmtpFmtSize = strlen(fmtpFmt)
                           + 3
                           + strlen(fStreamMuxConfigString);
    char* fmtp = new char[fmtpFmtSize];

    sprintf(fmtp, fmtpFmt,
            rtpPayloadType(),
            fStreamMuxConfigString);
    fFmtpSDPLine = strDup(fmtp);
    delete[] fmtp;
}

MPEG4LATMAudioRTPSink::~MPEG4LATMAudioRTPSink()
{
    delete[] fFmtpSDPLine;
    delete[](char *) fStreamMuxConfigString;
}

MPEG4LATMAudioRTPSink * MPEG4LATMAudioRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                         u_int8_t rtpPayloadFormat,
                                                         u_int32_t rtpTimestampFrequency,
                                                         char const* streamMuxConfigString,
                                                         unsigned numChannels,
                                                         Boolean allowMultipleFramesPerPacket)
{
    return new MPEG4LATMAudioRTPSink(env, RTPgs, rtpPayloadFormat,
                                     rtpTimestampFrequency, streamMuxConfigString,
                                     numChannels,
                                     allowMultipleFramesPerPacket);
}

Boolean MPEG4LATMAudioRTPSink
::frameCanAppearAfterPacketStart(unsigned char const *,
                                 unsigned) const
{
    return fAllowMultipleFramesPerPacket;
}

void MPEG4LATMAudioRTPSink
::doSpecialFrameHandling(unsigned       fragmentationOffset,
                         unsigned char* frameStart,
                         unsigned       numBytesInFrame,
                         struct timeval framePresentationTime,
                         unsigned       numRemainingBytes)
{
    if (numRemainingBytes == 0) {
        setMarkerBit();
    }
    MultiFramedRTPSink::doSpecialFrameHandling(fragmentationOffset,
                                               frameStart, numBytesInFrame,
                                               framePresentationTime,
                                               numRemainingBytes);
}

char const * MPEG4LATMAudioRTPSink::auxSDPLine()
{
    return fFmtpSDPLine;
}
