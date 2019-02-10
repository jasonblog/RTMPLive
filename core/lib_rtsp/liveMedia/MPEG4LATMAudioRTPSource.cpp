#include "MPEG4LATMAudioRTPSource.hh"
class LATMBufferedPacket : public BufferedPacket
{
public:
    LATMBufferedPacket(Boolean includeLATMDataLengthField);
    virtual ~LATMBufferedPacket();
private:
    virtual unsigned nextEnclosedFrameSize(unsigned char *& framePtr,
                                           unsigned       dataSize);
private:
    Boolean fIncludeLATMDataLengthField;
};
class LATMBufferedPacketFactory : public BufferedPacketFactory
{
private:
    virtual BufferedPacket * createNewPacket(MultiFramedRTPSource* ourSource);
};
MPEG4LATMAudioRTPSource * MPEG4LATMAudioRTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                             unsigned char rtpPayloadFormat,
                                                             unsigned rtpTimestampFrequency)
{
    return new MPEG4LATMAudioRTPSource(env, RTPgs, rtpPayloadFormat,
                                       rtpTimestampFrequency);
}

MPEG4LATMAudioRTPSource
::MPEG4LATMAudioRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                          unsigned char rtpPayloadFormat,
                          unsigned rtpTimestampFrequency)
    : MultiFramedRTPSource(env, RTPgs,
                           rtpPayloadFormat, rtpTimestampFrequency,
                           new LATMBufferedPacketFactory),
    fIncludeLATMDataLengthField(True)
{}

MPEG4LATMAudioRTPSource::~MPEG4LATMAudioRTPSource()
{}

void MPEG4LATMAudioRTPSource::omitLATMDataLengthField()
{
    fIncludeLATMDataLengthField = False;
}

Boolean MPEG4LATMAudioRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    fCurrentPacketBeginsFrame    = fCurrentPacketCompletesFrame;
    fCurrentPacketCompletesFrame = packet->rtpMarkerBit();
    resultSpecialHeaderSize      = 0;
    return True;
}

char const * MPEG4LATMAudioRTPSource::MIMEtype() const
{
    return "audio/MP4A-LATM";
}

LATMBufferedPacket::LATMBufferedPacket(Boolean includeLATMDataLengthField)
    : fIncludeLATMDataLengthField(includeLATMDataLengthField)
{}

LATMBufferedPacket::~LATMBufferedPacket()
{}

unsigned LATMBufferedPacket
::nextEnclosedFrameSize(unsigned char *& framePtr, unsigned dataSize)
{
    unsigned resultFrameSize = 0;
    unsigned i;

    for (i = 0; i < dataSize; ++i) {
        resultFrameSize += framePtr[i];
        if (framePtr[i] != 0xFF) {
            break;
        }
    }
    ++i;
    if (fIncludeLATMDataLengthField) {
        resultFrameSize += i;
    } else {
        framePtr += i;
        dataSize -= i;
    }
    return (resultFrameSize <= dataSize) ? resultFrameSize : dataSize;
}

BufferedPacket * LATMBufferedPacketFactory
::createNewPacket(MultiFramedRTPSource* ourSource)
{
    MPEG4LATMAudioRTPSource* source = (MPEG4LATMAudioRTPSource *) ourSource;

    return new LATMBufferedPacket(source->returnedFrameIncludesLATMDataLengthField());
}

static Boolean getNibble(char const * & configStr,
                         unsigned char& resultNibble)
{
    char c = configStr[0];

    if (c == '\0') {
        return False;
    }
    if (c >= '0' && c <= '9') {
        resultNibble = c - '0';
    } else if (c >= 'A' && c <= 'F') {
        resultNibble = 10 + c - 'A';
    } else if (c >= 'a' && c <= 'f') {
        resultNibble = 10 + c - 'a';
    } else {
        return False;
    }
    ++configStr;
    return True;
}

static Boolean getByte(char const *& configStr, unsigned char& resultByte)
{
    resultByte = 0;
    unsigned char firstNibble;
    if (!getNibble(configStr, firstNibble)) {
        return False;
    }
    resultByte = firstNibble << 4;
    unsigned char secondNibble = 0;
    if (!getNibble(configStr, secondNibble) && configStr[0] != '\0') {
        return False;
    }
    resultByte |= secondNibble;
    return True;
}

Boolean parseStreamMuxConfigStr(char const*    configStr,
                                Boolean        & audioMuxVersion,
                                Boolean        & allStreamsSameTimeFraming,
                                unsigned char  & numSubFrames,
                                unsigned char  & numProgram,
                                unsigned char  & numLayer,
                                unsigned char *& audioSpecificConfig,
                                unsigned       & audioSpecificConfigSize)
{
    audioMuxVersion = False;
    allStreamsSameTimeFraming = True;
    numSubFrames            = numProgram = numLayer = 0;
    audioSpecificConfig     = NULL;
    audioSpecificConfigSize = 0;
    do {
        if (configStr == NULL) {
            break;
        }
        unsigned char nextByte;
        if (!getByte(configStr, nextByte)) {
            break;
        }
        audioMuxVersion = (nextByte & 0x80) != 0;
        if (audioMuxVersion) {
            break;
        }
        allStreamsSameTimeFraming = ((nextByte & 0x40) >> 6) != 0;
        numSubFrames = (nextByte & 0x3F);
        if (!getByte(configStr, nextByte)) {
            break;
        }
        numProgram = (nextByte & 0xF0) >> 4;
        numLayer   = (nextByte & 0x0E) >> 1;
        unsigned char remainingBit = nextByte & 1;
        unsigned ascSize = (strlen(configStr) + 1) / 2 + 1;
        audioSpecificConfig = new unsigned char[ascSize];
        Boolean parseSuccess;
        unsigned i = 0;
        do {
            nextByte     = 0;
            parseSuccess = getByte(configStr, nextByte);
            audioSpecificConfig[i++] = (remainingBit << 7) | ((nextByte & 0xFE) >> 1);
            remainingBit = nextByte & 1;
        } while (parseSuccess);
        if (i != ascSize) {
            break;
        }
        audioSpecificConfigSize = ascSize;
        return True;
    } while (0);
    delete[] audioSpecificConfig;
    return False;
} // parseStreamMuxConfigStr

unsigned char * parseStreamMuxConfigStr(char const* configStr,
                                        unsigned    & audioSpecificConfigSize)
{
    Boolean audioMuxVersion, allStreamsSameTimeFraming;
    unsigned char numSubFrames, numProgram, numLayer;
    unsigned char* audioSpecificConfig;

    if (!parseStreamMuxConfigStr(configStr,
                                 audioMuxVersion, allStreamsSameTimeFraming,
                                 numSubFrames, numProgram, numLayer,
                                 audioSpecificConfig, audioSpecificConfigSize))
    {
        audioSpecificConfigSize = 0;
        return NULL;
    }
    return audioSpecificConfig;
}

unsigned char * parseGeneralConfigStr(char const* configStr,
                                      unsigned    & configSize)
{
    unsigned char* config = NULL;

    do {
        if (configStr == NULL) {
            break;
        }
        configSize = (strlen(configStr) + 1) / 2;
        config     = new unsigned char[configSize];
        if (config == NULL) {
            break;
        }
        unsigned i;
        for (i = 0; i < configSize; ++i) {
            if (!getByte(configStr, config[i])) {
                break;
            }
        }
        if (i != configSize) {
            break;
        }
        return config;
    } while (0);
    configSize = 0;
    delete[] config;
    return NULL;
}
