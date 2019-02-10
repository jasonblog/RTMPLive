#include "VorbisAudioRTPSource.hh"
#include "Base64.hh"
class VorbisBufferedPacket : public BufferedPacket
{
public:
    VorbisBufferedPacket();
    virtual ~VorbisBufferedPacket();
private:
    virtual unsigned nextEnclosedFrameSize(unsigned char *& framePtr,
                                           unsigned       dataSize);
};
class VorbisBufferedPacketFactory : public BufferedPacketFactory
{
private:
    virtual BufferedPacket * createNewPacket(MultiFramedRTPSource* ourSource);
};
VorbisAudioRTPSource * VorbisAudioRTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                       unsigned char rtpPayloadFormat,
                                                       unsigned rtpTimestampFrequency)
{
    return new VorbisAudioRTPSource(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency);
}

VorbisAudioRTPSource
::VorbisAudioRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                       unsigned char rtpPayloadFormat,
                       unsigned rtpTimestampFrequency)
    : MultiFramedRTPSource(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency,
                           new VorbisBufferedPacketFactory),
    fCurPacketIdent(0)
{}

VorbisAudioRTPSource::~VorbisAudioRTPSource()
{}

Boolean VorbisAudioRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    unsigned char* headerStart = packet->data();
    unsigned packetSize        = packet->dataSize();

    resultSpecialHeaderSize = 4;
    if (packetSize < resultSpecialHeaderSize) {
        return False;
    }
    fCurPacketIdent = (headerStart[0] << 16) | (headerStart[1] << 8) | headerStart[2];
    if ((headerStart[3] & 0x30) == 0x30) {
        return False;
    }
    u_int8_t F = headerStart[3] >> 6;
    fCurrentPacketBeginsFrame    = F <= 1;
    fCurrentPacketCompletesFrame = F == 0 || F == 3;
    return True;
}

char const * VorbisAudioRTPSource::MIMEtype() const
{
    return "audio/VORBIS";
}

VorbisBufferedPacket::VorbisBufferedPacket()
{}

VorbisBufferedPacket::~VorbisBufferedPacket()
{}

unsigned VorbisBufferedPacket
::nextEnclosedFrameSize(unsigned char *& framePtr, unsigned dataSize)
{
    if (dataSize < 2) {
        return dataSize;
    }
    unsigned frameSize = (framePtr[0] << 8) | framePtr[1];
    framePtr += 2;
    if (frameSize > dataSize - 2) {
        return dataSize - 2;
    }
    return frameSize;
}

BufferedPacket * VorbisBufferedPacketFactory
::createNewPacket(MultiFramedRTPSource *)
{
    return new VorbisBufferedPacket();
}

#define ADVANCE(n) do { p += (n); rem -= (n); } while (0)
#define GET_ENCODED_VAL(n) \
    do { u_int8_t byte; n = 0; \
         do { if (rem == 0) break; byte = *p; n = (n*128) +(byte&0x7F); ADVANCE(1); \
         } while (byte&0x80); } while (0); \
    if (rem == 0) break
void parseVorbisOrTheoraConfigStr(char const* configStr,
                                  u_int8_t *& identificationHdr, unsigned& identificationHdrSize,
                                  u_int8_t *& commentHdr, unsigned& commentHdrSize,
                                  u_int8_t *& setupHdr, unsigned& setupHdrSize,
                                  u_int32_t& identField)
{
    identificationHdr     = commentHdr = setupHdr = NULL;
    identificationHdrSize = commentHdrSize = setupHdrSize = 0;
    identField = 0;
    unsigned configDataSize;
    u_int8_t* configData = base64Decode(configStr, configDataSize);
    u_int8_t* p  = configData;
    unsigned rem = configDataSize;
    do {
        if (rem < 4) {
            break;
        }
        u_int32_t numPackedHeaders = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
        ADVANCE(4);
        if (numPackedHeaders == 0) {
            break;
        }
        if (rem < 3) {
            break;
        }
        identField = (p[0] << 16) | (p[1] << 8) | p[2];
        ADVANCE(3);
        if (rem < 2) {
            break;
        }
        u_int16_t length = (p[0] << 8) | p[1];
        ADVANCE(2);
        unsigned numHeaders;
        GET_ENCODED_VAL(numHeaders);
        Boolean success = False;
        for (unsigned i = 0; i < numHeaders + 1 && i < 3; ++i) {
            success = False;
            unsigned headerSize;
            if (i < numHeaders) {
                GET_ENCODED_VAL(headerSize);
                if (headerSize > length) {
                    break;
                }
                length -= headerSize;
            } else {
                headerSize = length;
            }
            if (i == 0) {
                identificationHdrSize = headerSize;
                identificationHdr     = new u_int8_t[identificationHdrSize];
            } else if (i == 1) {
                commentHdrSize = headerSize;
                commentHdr     = new u_int8_t[commentHdrSize];
            } else {
                setupHdrSize = headerSize;
                setupHdr     = new u_int8_t[setupHdrSize];
            }
            success = True;
        }
        if (!success) {
            break;
        }
        if (identificationHdr != NULL) {
            memmove(identificationHdr, p, identificationHdrSize);
            ADVANCE(identificationHdrSize);
            if (commentHdr != NULL) {
                memmove(commentHdr, p, commentHdrSize);
                ADVANCE(commentHdrSize);
                if (setupHdr != NULL) {
                    memmove(setupHdr, p, setupHdrSize);
                    ADVANCE(setupHdrSize);
                }
            }
        }
    } while (0);
    delete[] configData;
} // parseVorbisOrTheoraConfigStr
