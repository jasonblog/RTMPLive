#include "QuickTimeGenericRTPSource.hh"
class QTGenericBufferedPacket : public BufferedPacket
{
public:
    QTGenericBufferedPacket(QuickTimeGenericRTPSource& ourSource);
    virtual ~QTGenericBufferedPacket();
private:
    virtual unsigned nextEnclosedFrameSize(unsigned char *& framePtr,
                                           unsigned       dataSize);
private:
    QuickTimeGenericRTPSource& fOurSource;
};
class QTGenericBufferedPacketFactory : public BufferedPacketFactory
{
private:
    virtual BufferedPacket * createNewPacket(MultiFramedRTPSource* ourSource);
};
QuickTimeGenericRTPSource * QuickTimeGenericRTPSource::createNew(UsageEnvironment& env,
                                                                 Groupsock*      RTPgs,
                                                                 unsigned char   rtpPayloadFormat,
                                                                 unsigned        rtpTimestampFrequency,
                                                                 char const*     mimeTypeString)
{
    return new QuickTimeGenericRTPSource(env, RTPgs, rtpPayloadFormat,
                                         rtpTimestampFrequency,
                                         mimeTypeString);
}

QuickTimeGenericRTPSource
::QuickTimeGenericRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                            unsigned char rtpPayloadFormat,
                            unsigned rtpTimestampFrequency,
                            char const* mimeTypeString)
    : MultiFramedRTPSource(env, RTPgs,
                           rtpPayloadFormat, rtpTimestampFrequency,
                           new QTGenericBufferedPacketFactory),
    fMIMEtypeString(strDup(mimeTypeString))
{
    qtState.PCK        = 0;
    qtState.timescale  = 0;
    qtState.sdAtom     = NULL;
    qtState.sdAtomSize = qtState.width = qtState.height = 0;
}

QuickTimeGenericRTPSource::~QuickTimeGenericRTPSource()
{
    delete[] qtState.sdAtom;
    delete[](char *) fMIMEtypeString;
}

Boolean QuickTimeGenericRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    unsigned char* headerStart  = packet->data();
    unsigned packetSize         = packet->dataSize();
    unsigned expectedHeaderSize = 4;

    if (packetSize < expectedHeaderSize) {
        return False;
    }
    unsigned char VER = (headerStart[0] & 0xF0) >> 4;
    if (VER > 1) {
        return False;
    }
    qtState.PCK = (headerStart[0] & 0x0C) >> 2;
    #ifdef DEBUG
    Boolean S = (headerStart[0] & 0x02) != 0;
    #endif
    Boolean Q = (headerStart[0] & 0x01) != 0;
    Boolean L = (headerStart[1] & 0x80) != 0;
    #ifdef DEBUG
    Boolean D = (headerStart[2] & 0x80) != 0;
    unsigned short payloadId = ((headerStart[2] & 0x7F) << 8) | headerStart[3];
    #endif
    headerStart += 4;
    #ifdef DEBUG
    fprintf(stderr, "PCK: %d, S: %d, Q: %d, L: %d, D: %d, payloadId: %d\n", qtState.PCK, S, Q, L, D, payloadId);
    #endif
    if (Q) {
        expectedHeaderSize += 4;
        if (packetSize < expectedHeaderSize) {
            return False;
        }
        #ifdef DEBUG
        Boolean K = (headerStart[0] & 0x80) != 0;
        Boolean F = (headerStart[0] & 0x40) != 0;
        Boolean A = (headerStart[0] & 0x20) != 0;
        Boolean Z = (headerStart[0] & 0x10) != 0;
        #endif
        unsigned payloadDescriptionLength = (headerStart[2] << 8) | headerStart[3];
        headerStart += 4;
        #ifdef DEBUG
        fprintf(stderr, "\tK: %d, F: %d, A: %d, Z: %d, payloadDescriptionLength: %d\n", K, F, A, Z,
                payloadDescriptionLength);
        #endif
        if (payloadDescriptionLength < 12) {
            return False;
        }
        expectedHeaderSize += (payloadDescriptionLength - 4);
        unsigned nonPaddedSize = expectedHeaderSize;
        expectedHeaderSize += 3;
        expectedHeaderSize -= expectedHeaderSize % 4;
        if (packetSize < expectedHeaderSize) {
            return False;
        }
        unsigned char padding = expectedHeaderSize - nonPaddedSize;
        #ifdef DEBUG
        unsigned mediaType = (headerStart[0] << 24) | (headerStart[1] << 16)
                             | (headerStart[2] << 8) | headerStart[3];
        #endif
        qtState.timescale = (headerStart[4] << 24) | (headerStart[5] << 16)
                            | (headerStart[6] << 8) | headerStart[7];
        headerStart += 8;
        payloadDescriptionLength -= 12;
        #ifdef DEBUG
        fprintf(stderr, "\tmediaType: '%c%c%c%c', timescale: %d, %d bytes of TLVs left\n", mediaType >> 24,
                (mediaType & 0xFF0000) >> 16, (mediaType & 0xFF00) >> 8, mediaType & 0xFF, qtState.timescale,
                payloadDescriptionLength);
        #endif
        while (payloadDescriptionLength > 3) {
            unsigned short tlvLength = (headerStart[0] << 8) | headerStart[1];
            unsigned short tlvType   = (headerStart[2] << 8) | headerStart[3];
            payloadDescriptionLength -= 4;
            if (tlvLength > payloadDescriptionLength) {
                return False;
            }
            headerStart += 4;
            #ifdef DEBUG
            fprintf(stderr, "\t\tTLV '%c%c', length %d, leaving %d remaining bytes\n", tlvType >> 8, tlvType & 0xFF,
                    tlvLength, payloadDescriptionLength - tlvLength);
            for (int i = 0; i < tlvLength; ++i) {
                fprintf(stderr, "%02x:", headerStart[i]);
            }
            fprintf(stderr, "\n");
            #endif
            switch (tlvType) {
                case ('s'<<8|'d'): {
                    unsigned atomLength = (headerStart[0] << 24) | (headerStart[1] << 16)
                                          | (headerStart[2] << 8) | (headerStart[3]);
                    if (atomLength != (unsigned) tlvLength) {
                        break;
                    }
                    delete[] qtState.sdAtom;
                    qtState.sdAtom = new char[tlvLength];
                    memmove(qtState.sdAtom, headerStart, tlvLength);
                    qtState.sdAtomSize = tlvLength;
                    break;
                }
                case ('t'<<8|'w'): {
                    qtState.width = (headerStart[0] << 8) | headerStart[1];
                    break;
                }
                case ('t'<<8|'h'): {
                    qtState.height = (headerStart[0] << 8) | headerStart[1];
                    break;
                }
            }
            payloadDescriptionLength -= tlvLength;
            headerStart += tlvLength;
        }
        if (payloadDescriptionLength > 0) {
            return False;
        }
        headerStart += padding;
    }
    if (L) {
        expectedHeaderSize += 4;
        if (packetSize < expectedHeaderSize) {
            return False;
        }
        unsigned ssInfoLength = (headerStart[2] << 8) | headerStart[3];
        headerStart += 4;
        #ifdef DEBUG
        fprintf(stderr, "\tssInfoLength: %d\n", ssInfoLength);
        #endif
        if (ssInfoLength < 4) {
            return False;
        }
        expectedHeaderSize += (ssInfoLength - 4);
        unsigned nonPaddedSize = expectedHeaderSize;
        expectedHeaderSize += 3;
        expectedHeaderSize -= expectedHeaderSize % 4;
        if (packetSize < expectedHeaderSize) {
            return False;
        }
        unsigned char padding = expectedHeaderSize - nonPaddedSize;
        ssInfoLength -= 4;
        while (ssInfoLength > 3) {
            unsigned short tlvLength = (headerStart[0] << 8) | headerStart[1];
            #ifdef DEBUG
            unsigned short tlvType = (headerStart[2] << 8) | headerStart[3];
            #endif
            ssInfoLength -= 4;
            if (tlvLength > ssInfoLength) {
                return False;
            }
            #ifdef DEBUG
            fprintf(stderr, "\t\tTLV '%c%c', length %d, leaving %d remaining bytes\n", tlvType >> 8, tlvType & 0xFF,
                    tlvLength, ssInfoLength - tlvLength);
            for (int i = 0; i < tlvLength; ++i) {
                fprintf(stderr, "%02x:", headerStart[4 + i]);
            }
            fprintf(stderr, "\n");
            #endif
            ssInfoLength -= tlvLength;
            headerStart  += 4 + tlvLength;
        }
        if (ssInfoLength > 0) {
            return False;
        }
        headerStart += padding;
    }
    fCurrentPacketBeginsFrame    = fCurrentPacketCompletesFrame;
    fCurrentPacketCompletesFrame = packet->rtpMarkerBit();
    resultSpecialHeaderSize      = expectedHeaderSize;
    #ifdef DEBUG
    fprintf(stderr, "Result special header size: %d\n", resultSpecialHeaderSize);
    #endif
    return True;
} // QuickTimeGenericRTPSource::processSpecialHeader

char const * QuickTimeGenericRTPSource::MIMEtype() const
{
    if (fMIMEtypeString == NULL) {
        return MultiFramedRTPSource::MIMEtype();
    }
    return fMIMEtypeString;
}

QTGenericBufferedPacket
::QTGenericBufferedPacket(QuickTimeGenericRTPSource& ourSource)
    : fOurSource(ourSource)
{}

QTGenericBufferedPacket::~QTGenericBufferedPacket()
{}

unsigned QTGenericBufferedPacket::
nextEnclosedFrameSize(unsigned char *& framePtr, unsigned dataSize)
{
    if (fOurSource.qtState.PCK != 2) {
        return dataSize;
    }
    if (dataSize < 8) {
        return 0;
    }
    unsigned short sampleLength = (framePtr[2] << 8) | framePtr[3];
    framePtr += 8;
    dataSize -= 8;
    return sampleLength < dataSize ? sampleLength : dataSize;
}

BufferedPacket * QTGenericBufferedPacketFactory
::createNewPacket(MultiFramedRTPSource* ourSource)
{
    return new QTGenericBufferedPacket((QuickTimeGenericRTPSource&) (*ourSource));
}
