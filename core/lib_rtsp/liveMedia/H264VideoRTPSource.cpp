#include "H264VideoRTPSource.hh"
#include "Base64.hh"
class H264BufferedPacket : public BufferedPacket
{
public:
    H264BufferedPacket(H264VideoRTPSource& ourSource);
    virtual ~H264BufferedPacket();
private:
    virtual unsigned nextEnclosedFrameSize(unsigned char *& framePtr,
                                           unsigned       dataSize);
private:
    H264VideoRTPSource& fOurSource;
};
class H264BufferedPacketFactory : public BufferedPacketFactory
{
private:
    virtual BufferedPacket * createNewPacket(MultiFramedRTPSource* ourSource);
};
H264VideoRTPSource * H264VideoRTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                   unsigned char rtpPayloadFormat,
                                                   unsigned rtpTimestampFrequency)
{
    return new H264VideoRTPSource(env, RTPgs, rtpPayloadFormat,
                                  rtpTimestampFrequency);
}

H264VideoRTPSource
::H264VideoRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                     unsigned char rtpPayloadFormat,
                     unsigned rtpTimestampFrequency)
    : MultiFramedRTPSource(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency,
                           new H264BufferedPacketFactory)
{}

H264VideoRTPSource::~H264VideoRTPSource()
{}

Boolean H264VideoRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    unsigned char* headerStart = packet->data();
    unsigned packetSize        = packet->dataSize();
    unsigned numBytesToSkip;

    if (packetSize < 1) {
        return False;
    }
    fCurPacketNALUnitType = (headerStart[0] & 0x1F);
    switch (fCurPacketNALUnitType) {
        case 24: {
            numBytesToSkip = 1;
            break;
        }
        case 25:
        case 26:
        case 27: {
            numBytesToSkip = 3;
            break;
        }
        case 28:
        case 29: {
            if (packetSize < 2) {
                return False;
            }
            unsigned char startBit = headerStart[1] & 0x80;
            unsigned char endBit   = headerStart[1] & 0x40;
            if (startBit) {
                fCurrentPacketBeginsFrame = True;
                headerStart[1] = (headerStart[0] & 0xE0) | (headerStart[1] & 0x1F);
                numBytesToSkip = 1;
            } else {
                fCurrentPacketBeginsFrame = False;
                numBytesToSkip = 2;
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
    resultSpecialHeaderSize = numBytesToSkip;
    return True;
} // H264VideoRTPSource::processSpecialHeader

char const * H264VideoRTPSource::MIMEtype() const
{
    return "video/H264";
}

SPropRecord * parseSPropParameterSets(char const* sPropParameterSetsStr,
                                      unsigned    & numSPropRecords)
{
    char* inStr = strDup(sPropParameterSetsStr);

    if (inStr == NULL) {
        numSPropRecords = 0;
        return NULL;
    }
    numSPropRecords = 1;
    char* s;
    for (s = inStr; *s != '\0'; ++s) {
        if (*s == ',') {
            ++numSPropRecords;
            *s = '\0';
        }
    }
    SPropRecord* resultArray = new SPropRecord[numSPropRecords];
    s = inStr;
    for (unsigned i = 0; i < numSPropRecords; ++i) {
        resultArray[i].sPropBytes = base64Decode(s, resultArray[i].sPropLength);
        s += strlen(s) + 1;
    }
    delete[] inStr;
    return resultArray;
}

H264BufferedPacket::H264BufferedPacket(H264VideoRTPSource& ourSource)
    : fOurSource(ourSource)
{}

H264BufferedPacket::~H264BufferedPacket()
{}

unsigned H264BufferedPacket
::nextEnclosedFrameSize(unsigned char *& framePtr, unsigned dataSize)
{
    unsigned resultNALUSize = 0;

    switch (fOurSource.fCurPacketNALUnitType) {
        case 24:
        case 25: {
            if (dataSize < 2) {
                break;
            }
            resultNALUSize = (framePtr[0] << 8) | framePtr[1];
            framePtr      += 2;
            break;
        }
        case 26: {
            if (dataSize < 5) {
                break;
            }
            resultNALUSize = (framePtr[0] << 8) | framePtr[1];
            framePtr      += 5;
            break;
        }
        case 27: {
            if (dataSize < 6) {
                break;
            }
            resultNALUSize = (framePtr[0] << 8) | framePtr[1];
            framePtr      += 6;
            break;
        }
        default: {
            return dataSize;
        }
    }
    return (resultNALUSize <= dataSize) ? resultNALUSize : dataSize;
} // H264BufferedPacket::nextEnclosedFrameSize

BufferedPacket * H264BufferedPacketFactory
::createNewPacket(MultiFramedRTPSource* ourSource)
{
    return new H264BufferedPacket((H264VideoRTPSource&) (*ourSource));
}
