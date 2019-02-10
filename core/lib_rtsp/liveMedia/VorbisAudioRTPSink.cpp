#include "VorbisAudioRTPSink.hh"
#include "Base64.hh"
#include "VorbisAudioRTPSource.hh"
VorbisAudioRTPSink * VorbisAudioRTPSink
::createNew(UsageEnvironment& env, Groupsock* RTPgs,
            u_int8_t rtpPayloadFormat, u_int32_t rtpTimestampFrequency, unsigned numChannels,
            u_int8_t* identificationHeader, unsigned identificationHeaderSize,
            u_int8_t* commentHeader, unsigned commentHeaderSize,
            u_int8_t* setupHeader, unsigned setupHeaderSize,
            u_int32_t identField)
{
    return new VorbisAudioRTPSink(env, RTPgs,
                                  rtpPayloadFormat, rtpTimestampFrequency, numChannels,
                                  identificationHeader, identificationHeaderSize,
                                  commentHeader, commentHeaderSize,
                                  setupHeader, setupHeaderSize,
                                  identField);
}

VorbisAudioRTPSink * VorbisAudioRTPSink
::createNew(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
            u_int32_t rtpTimestampFrequency, unsigned numChannels,
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
    VorbisAudioRTPSink* resultSink =
        new VorbisAudioRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, numChannels,
                               identificationHeader, identificationHeaderSize,
                               commentHeader, commentHeaderSize,
                               setupHeader, setupHeaderSize,
                               identField);
    delete[] identificationHeader;
    delete[] commentHeader;
    delete[] setupHeader;
    return resultSink;
}

VorbisAudioRTPSink
::VorbisAudioRTPSink(UsageEnvironment& env, Groupsock* RTPgs, u_int8_t rtpPayloadFormat,
                     u_int32_t rtpTimestampFrequency, unsigned numChannels,
                     u_int8_t* identificationHeader, unsigned identificationHeaderSize,
                     u_int8_t* commentHeader, unsigned commentHeaderSize,
                     u_int8_t* setupHeader, unsigned setupHeaderSize,
                     u_int32_t identField)
    : AudioRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, "VORBIS", numChannels),
    fIdent(identField), fFmtpSDPLine(NULL)
{
    if (identificationHeaderSize >= 28) {
        u_int32_t val;
        u_int8_t* p;
        p   = &identificationHeader[16];
        val = ((p[3] * 256 + p[2]) * 256 + p[1]) * 256 + p[0];
        int bitrate_maximum = (int) val;
        if (bitrate_maximum < 0) {
            bitrate_maximum = 0;
        }
        p   = &identificationHeader[20];
        val = ((p[3] * 256 + p[2]) * 256 + p[1]) * 256 + p[0];
        int bitrate_nominal = (int) val;
        if (bitrate_nominal < 0) {
            bitrate_nominal = 0;
        }
        p   = &identificationHeader[24];
        val = ((p[3] * 256 + p[2]) * 256 + p[1]) * 256 + p[0];
        int bitrate_minimum = (int) val;
        if (bitrate_minimum < 0) {
            bitrate_minimum = 0;
        }
        int bitrate =
            bitrate_nominal > 0 ? bitrate_nominal :
            bitrate_maximum > 0 ? bitrate_maximum :
            bitrate_minimum > 0 ? bitrate_minimum : 0;
        if (bitrate > 0) {
            estimatedBitrate() = ((unsigned) bitrate) / 1000;
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
    unsigned fmtpSDPLineMaxSize = 50 + strlen(base64PackedHeaders);
    fFmtpSDPLine = new char[fmtpSDPLineMaxSize];
    sprintf(fFmtpSDPLine, "a=fmtp:%d configuration=%s\r\n", rtpPayloadType(), base64PackedHeaders);
    delete[] base64PackedHeaders;
}

VorbisAudioRTPSink::~VorbisAudioRTPSink()
{
    delete[] fFmtpSDPLine;
}

char const * VorbisAudioRTPSink::auxSDPLine()
{
    return fFmtpSDPLine;
}

void VorbisAudioRTPSink
::doSpecialFrameHandling(unsigned       fragmentationOffset,
                         unsigned char* frameStart,
                         unsigned       numBytesInFrame,
                         struct timeval framePresentationTime,
                         unsigned       numRemainingBytes)
{
    u_int8_t header[4];

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
    u_int8_t const VDT = 0 << 4;
    u_int8_t numPkts   = F == 0 ? (numFramesUsedSoFar() + 1) : 0;
    header[3] = F | VDT | numPkts;
    setSpecialHeaderBytes(header, sizeof header);
    u_int8_t frameSpecificHeader[2];
    frameSpecificHeader[0] = numBytesInFrame >> 8;
    frameSpecificHeader[1] = numBytesInFrame;
    setFrameSpecificHeaderBytes(frameSpecificHeader, 2);
    MultiFramedRTPSink::doSpecialFrameHandling(fragmentationOffset,
                                               frameStart, numBytesInFrame,
                                               framePresentationTime,
                                               numRemainingBytes);
} // VorbisAudioRTPSink::doSpecialFrameHandling

Boolean VorbisAudioRTPSink::frameCanAppearAfterPacketStart(unsigned char const *,
                                                           unsigned) const
{
    return numFramesUsedSoFar() <= 15;
}

unsigned VorbisAudioRTPSink::specialHeaderSize() const
{
    return 4;
}

unsigned VorbisAudioRTPSink::frameSpecificHeaderSize() const
{
    return 2;
}

char * generateVorbisOrTheoraConfigStr(u_int8_t* identificationHeader, unsigned identificationHeaderSize,
                                       u_int8_t* commentHeader, unsigned commentHeaderSize,
                                       u_int8_t* setupHeader, unsigned setupHeaderSize,
                                       u_int32_t identField)
{
    unsigned numHeaders = 0;
    unsigned sizeSize[2];

    sizeSize[0] = sizeSize[1] = 0;
    if (identificationHeaderSize > 0) {
        sizeSize[numHeaders++] = identificationHeaderSize < 128 ? 1 : identificationHeaderSize < 16384 ? 2 : 3;
    }
    if (commentHeaderSize > 0) {
        sizeSize[numHeaders++] = commentHeaderSize < 128 ? 1 : commentHeaderSize < 16384 ? 2 : 3;
    }
    if (setupHeaderSize > 0) {
        ++numHeaders;
    } else {
        sizeSize[1] = 0;
    }
    if (numHeaders == 0) {
        return NULL;
    }
    if (numHeaders == 1) {
        sizeSize[0] = 0;
    }
    unsigned length = identificationHeaderSize + commentHeaderSize + setupHeaderSize;
    if (length > (unsigned) 0xFFFF) {
        return NULL;
    }
    unsigned packedHeadersSize =
        4
        + 3
        + 2
        + 1
        + sizeSize[0] + sizeSize[1]
        + length;
    u_int8_t* packedHeaders = new u_int8_t[packedHeadersSize];
    if (packedHeaders == NULL) {
        return NULL;
    }
    u_int8_t* p = packedHeaders;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 1;
    *p++ = identField >> 16;
    *p++ = identField >> 8;
    *p++ = identField;
    *p++ = length >> 8;
    *p++ = length;
    *p++ = numHeaders - 1;
    if (numHeaders > 1) {
        unsigned length1 = identificationHeaderSize > 0 ? identificationHeaderSize : commentHeaderSize;
        if (length1 >= 16384) {
            *p++ = 0x80;
        }
        if (length1 >= 128) {
            *p++ = 0x80 | ((length1 & 0x3F80) >> 7);
        }
        *p++ = length1 & 0x7F;
        if (numHeaders > 2) {
            unsigned length2 = commentHeaderSize;
            if (length2 >= 16384) {
                *p++ = 0x80;
            }
            if (length2 >= 128) {
                *p++ = 0x80 | ((length2 & 0x3F80) >> 7);
            }
            *p++ = length2 & 0x7F;
        }
    }
    if (identificationHeader != NULL) {
        memmove(p, identificationHeader, identificationHeaderSize);
    }
    p += identificationHeaderSize;
    if (commentHeader != NULL) {
        memmove(p, commentHeader, commentHeaderSize);
    }
    p += commentHeaderSize;
    if (setupHeader != NULL) {
        memmove(p, setupHeader, setupHeaderSize);
    }
    char* base64PackedHeaders = base64Encode((char const *) packedHeaders, packedHeadersSize);
    delete[] packedHeaders;
    return base64PackedHeaders;
} // generateVorbisOrTheoraConfigStr
