#include "TheoraVideoRTPSink.hh"
#include "Base64.hh"
#include "VorbisAudioRTPSource.hh"
#include "VorbisAudioRTPSink.hh"
TheoraVideoRTPSink * TheoraVideoRTPSink
::createNew(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
            u_int8_t* identificationHeader, unsigned identificationHeaderSize,
            u_int8_t* commentHeader, unsigned commentHeaderSize,
            u_int8_t* setupHeader, unsigned setupHeaderSize,
            u_int32_t identField)
{
    return new TheoraVideoRTPSink(env, RTPgs,
                                  rtpPayloadFormat,
                                  identificationHeader, identificationHeaderSize,
                                  commentHeader, commentHeaderSize,
                                  setupHeader, setupHeaderSize, identField);
}

TheoraVideoRTPSink * TheoraVideoRTPSink
::createNew(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
            char const* configStr)
{
    u_int8_t* identificationHeader;
    unsigned identificationHeaderSize;
    u_int8_t* commentHeader;
    unsigned commentHeaderSize;
    u_int8_t* setupHeader;
    unsigned setupHeaderSize;
    u_int32_t identField;

    parseVorbisOrTheoraConfigStr(configStr,
                                 identificationHeader, identificationHeaderSize,
                                 commentHeader, commentHeaderSize,
                                 setupHeader, setupHeaderSize,
                                 identField);
    TheoraVideoRTPSink* resultSink =
        new TheoraVideoRTPSink(env, RTPgs, rtpPayloadFormat,
                               identificationHeader, identificationHeaderSize,
                               commentHeader, commentHeaderSize,
                               setupHeader, setupHeaderSize,
                               identField);
    delete[] identificationHeader;
    delete[] commentHeader;
    delete[] setupHeader;
    return resultSink;
}

TheoraVideoRTPSink
::TheoraVideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
                     u_int8_t* identificationHeader, unsigned identificationHeaderSize,
                     u_int8_t* commentHeader, unsigned commentHeaderSize,
                     u_int8_t* setupHeader, unsigned setupHeaderSize,
                     u_int32_t identField)
    : VideoRTPSink(env, RTPgs, rtpPayloadFormat, 90000, "THEORA"),
    fIdent(identField), fFmtpSDPLine(NULL)
{
    static const char* pf_to_str[] = {
        "YCbCr-4:2:0",
        "Reserved",
        "YCbCr-4:2:2",
        "YCbCr-4:4:4",
    };
    unsigned width  = 1280;
    unsigned height = 720;
    unsigned pf     = 0;

    if (identificationHeaderSize >= 42) {
        u_int8_t* p = identificationHeader;
        width  = (p[14] << 16) | (p[15] << 8) | p[16];
        height = (p[17] << 16) | (p[18] << 8) | p[19];
        pf     = (p[41] & 0x18) >> 3;
        unsigned nominalBitrate = (p[37] << 16) | (p[38] << 8) | p[39];
        if (nominalBitrate > 0) {
            estimatedBitrate() = nominalBitrate / 1000;
        }
    }
    char* base64PackedHeaders =
        generateVorbisOrTheoraConfigStr(identificationHeader, identificationHeaderSize,
                                        commentHeader, commentHeaderSize,
                                        setupHeader, setupHeaderSize,
                                        identField);
    if (base64PackedHeaders == NULL) {
        return;
    }
    unsigned fmtpSDPLineMaxSize = 200 + strlen(base64PackedHeaders);
    fFmtpSDPLine = new char[fmtpSDPLineMaxSize];
    sprintf(fFmtpSDPLine, "a=fmtp:%d sampling=%s;width=%u;height=%u;delivery-method=out_band/rtsp;configuration=%s\r\n",
            rtpPayloadType(), pf_to_str[pf], width, height, base64PackedHeaders);
    delete[] base64PackedHeaders;
}

TheoraVideoRTPSink::~TheoraVideoRTPSink()
{
    delete[] fFmtpSDPLine;
}

char const * TheoraVideoRTPSink::auxSDPLine()
{
    return fFmtpSDPLine;
}

void TheoraVideoRTPSink
::doSpecialFrameHandling(unsigned       fragmentationOffset,
                         unsigned char* frameStart,
                         unsigned       numBytesInFrame,
                         struct timeval framePresentationTime,
                         unsigned       numRemainingBytes)
{
    u_int8_t header[6];

    header[0] = fIdent >> 16;
    header[1] = fIdent >> 8;
    header[2] = fIdent;
    u_int8_t F;
    if (numRemainingBytes > 0) {
        if (fragmentationOffset > 0) {
            F = 2 << 6;
        } else {
            F = 1 << 6;
        }
    } else {
        if (fragmentationOffset > 0) {
            F = 3 << 6;
        } else {
            F = 0 << 6;
        }
    }
    u_int8_t const TDT = 0 << 4;
    u_int8_t numPkts   = F == 0 ? (numFramesUsedSoFar() + 1) : 0;
    header[3] = F | TDT | numPkts;
    header[4] = numBytesInFrame >> 8;
    header[5] = numBytesInFrame;
    setSpecialHeaderBytes(header, sizeof(header));
    if (numRemainingBytes == 0) {
        setMarkerBit();
    }
    MultiFramedRTPSink::doSpecialFrameHandling(fragmentationOffset,
                                               frameStart, numBytesInFrame,
                                               framePresentationTime,
                                               numRemainingBytes);
} // TheoraVideoRTPSink::doSpecialFrameHandling

Boolean TheoraVideoRTPSink::frameCanAppearAfterPacketStart(unsigned char const *,
                                                           unsigned) const
{
    return False;
}

unsigned TheoraVideoRTPSink::specialHeaderSize() const
{
    return 6;
}
