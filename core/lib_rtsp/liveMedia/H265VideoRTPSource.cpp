#include "H265VideoRTPSource.hh"
class H265BufferedPacket : public BufferedPacket
{
public:
    H265BufferedPacket(H265VideoRTPSource& ourSource);
    virtual ~H265BufferedPacket();
private:
    virtual unsigned nextEnclosedFrameSize(unsigned char *& framePtr,
                                           unsigned       dataSize);
private:
    H265VideoRTPSource& fOurSource;
};
class H265BufferedPacketFactory : public BufferedPacketFactory
{
private:
    virtual BufferedPacket * createNewPacket(MultiFramedRTPSource* ourSource);
};
H265VideoRTPSource * H265VideoRTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                   unsigned char rtpPayloadFormat,
                                                   Boolean expectDONFields,
                                                   unsigned rtpTimestampFrequency)
{
    return new H265VideoRTPSource(env, RTPgs, rtpPayloadFormat,
                                  expectDONFields, rtpTimestampFrequency);
}

H265VideoRTPSource
::H265VideoRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                     unsigned char rtpPayloadFormat,
                     Boolean expectDONFields,
                     unsigned rtpTimestampFrequency)
    : MultiFramedRTPSource(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency,
                           new H265BufferedPacketFactory),
    fExpectDONFields(expectDONFields),
    fPreviousNALUnitDON(0), fCurrentNALUnitAbsDon((u_int64_t) (~0))
{}

H265VideoRTPSource::~H265VideoRTPSource()
{}

Boolean H265VideoRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    unsigned char* headerStart = packet->data();
    unsigned packetSize        = packet->dataSize();
    u_int16_t DONL = 0;
    unsigned numBytesToSkip;

    if (packetSize < 2) {
        return False;
    }
    fCurPacketNALUnitType = (headerStart[0] & 0x7E) >> 1;
    switch (fCurPacketNALUnitType) {
        case 48: {
            if (fExpectDONFields) {
                if (packetSize < 4) {
                    return False;
                }
                DONL = (headerStart[2] << 8) | headerStart[3];
                numBytesToSkip = 4;
            } else {
                numBytesToSkip = 2;
            }
            break;
        }
        case 49: {
            if (packetSize < 3) {
                return False;
            }
            u_int8_t startBit = headerStart[2] & 0x80;
            u_int8_t endBit   = headerStart[2] & 0x40;
            if (startBit) {
                fCurrentPacketBeginsFrame = True;
                u_int8_t nal_unit_type = headerStart[2] & 0x3F;
                u_int8_t newNALHeader[2];
                newNALHeader[0] = (headerStart[0] & 0x81) | (nal_unit_type << 1);
                newNALHeader[1] = headerStart[1];
                if (fExpectDONFields) {
                    if (packetSize < 5) {
                        return False;
                    }
                    DONL = (headerStart[3] << 8) | headerStart[4];
                    headerStart[3] = newNALHeader[0];
                    headerStart[4] = newNALHeader[1];
                    numBytesToSkip = 3;
                } else {
                    headerStart[1] = newNALHeader[0];
                    headerStart[2] = newNALHeader[1];
                    numBytesToSkip = 1;
                }
            } else {
                fCurrentPacketBeginsFrame = False;
                if (fExpectDONFields) {
                    if (packetSize < 5) {
                        return False;
                    }
                    DONL = (headerStart[3] << 8) | headerStart[4];
                    numBytesToSkip = 5;
                } else {
                    numBytesToSkip = 3;
                }
            }
            fCurrentPacketCompletesFrame = (endBit != 0);
            break;
        }
        default: {
            fCurrentPacketBeginsFrame = fCurrentPacketCompletesFrame = True;
            numBytesToSkip = 0;
            break;
        }
    }
    computeAbsDonFromDON(DONL);
    resultSpecialHeaderSize = numBytesToSkip;
    return True;
} // H265VideoRTPSource::processSpecialHeader

char const * H265VideoRTPSource::MIMEtype() const
{
    return "video/H265";
}

void H265VideoRTPSource::computeAbsDonFromDON(u_int16_t DON)
{
    if (!fExpectDONFields) {
        ++fCurrentNALUnitAbsDon;
    } else {
        if (fCurrentNALUnitAbsDon == (u_int64_t) (~0)) {
            fCurrentNALUnitAbsDon = (u_int64_t) DON;
        } else {
            short signedDiff16   = (short) (DON - fPreviousNALUnitDON);
            int64_t signedDiff64 = (int64_t) signedDiff16;
            fCurrentNALUnitAbsDon += signedDiff64;
        }
        fPreviousNALUnitDON = DON;
    }
}

H265BufferedPacket::H265BufferedPacket(H265VideoRTPSource& ourSource)
    : fOurSource(ourSource)
{}

H265BufferedPacket::~H265BufferedPacket()
{}

unsigned H265BufferedPacket
::nextEnclosedFrameSize(unsigned char *& framePtr, unsigned dataSize)
{
    unsigned resultNALUSize = 0;

    switch (fOurSource.fCurPacketNALUnitType) {
        case 48: {
            if (useCount() > 0) {
                u_int16_t DONL = 0;
                if (fOurSource.fExpectDONFields) {
                    if (dataSize < 1) {
                        break;
                    }
                    u_int8_t DOND = framePtr[0];
                    DONL = fOurSource.fPreviousNALUnitDON + (u_int16_t) (DOND + 1);
                    ++framePtr;
                    --dataSize;
                }
                fOurSource.computeAbsDonFromDON(DONL);
            }
            if (dataSize < 2) {
                break;
            }
            resultNALUSize = (framePtr[0] << 8) | framePtr[1];
            framePtr      += 2;
            break;
        }
        default: {
            return dataSize;
        }
    }
    return (resultNALUSize <= dataSize) ? resultNALUSize : dataSize;
}

BufferedPacket * H265BufferedPacketFactory
::createNewPacket(MultiFramedRTPSource* ourSource)
{
    return new H265BufferedPacket((H265VideoRTPSource&) (*ourSource));
}
