#include "MPEG4VideoStreamFramer.hh"
#include "MPEGVideoStreamParser.hh"
#include "MPEG4LATMAudioRTPSource.hh"
#include <string.h>
enum MPEGParseState {
    PARSING_VISUAL_OBJECT_SEQUENCE,
    PARSING_VISUAL_OBJECT_SEQUENCE_SEEN_CODE,
    PARSING_VISUAL_OBJECT,
    PARSING_VIDEO_OBJECT_LAYER,
    PARSING_GROUP_OF_VIDEO_OBJECT_PLANE,
    PARSING_VIDEO_OBJECT_PLANE,
    PARSING_VISUAL_OBJECT_SEQUENCE_END_CODE
};
class MPEG4VideoStreamParser : public MPEGVideoStreamParser
{
public:
    MPEG4VideoStreamParser(MPEG4VideoStreamFramer* usingSource,
                           FramedSource*           inputSource);
    virtual ~MPEG4VideoStreamParser();
private:
    virtual void flushInput();
    virtual unsigned parse();
private:
    MPEG4VideoStreamFramer * usingSource()
    {
        return (MPEG4VideoStreamFramer *) fUsingSource;
    }

    void setParseState(MPEGParseState parseState);
    unsigned parseVisualObjectSequence(Boolean haveSeenStartCode = False);
    unsigned parseVisualObject();
    unsigned parseVideoObjectLayer();
    unsigned parseGroupOfVideoObjectPlane();
    unsigned parseVideoObjectPlane();
    unsigned parseVisualObjectSequenceEndCode();
    Boolean getNextFrameBit(u_int8_t& result);
    Boolean getNextFrameBits(unsigned numBits, u_int32_t& result);
    void analyzeVOLHeader();
private:
    MPEGParseState fCurrentParseState;
    unsigned fNumBitsSeenSoFar;
    u_int32_t vop_time_increment_resolution;
    unsigned fNumVTIRBits;
    u_int8_t fixed_vop_rate;
    unsigned fixed_vop_time_increment;
    unsigned fSecondsSinceLastTimeCode, fTotalTicksSinceLastTimeCode, fPrevNewTotalTicks;
    unsigned fPrevPictureCountDelta;
    Boolean fJustSawTimeCode;
};
MPEG4VideoStreamFramer * MPEG4VideoStreamFramer::createNew(UsageEnvironment& env,
                                                           FramedSource*   inputSource)
{
    return new MPEG4VideoStreamFramer(env, inputSource);
}

unsigned char * MPEG4VideoStreamFramer
::getConfigBytes(unsigned& numBytes) const
{
    numBytes = fNumConfigBytes;
    return fConfigBytes;
}

void MPEG4VideoStreamFramer
::setConfigInfo(u_int8_t profileAndLevelIndication, char const* configStr)
{
    fProfileAndLevelIndication = profileAndLevelIndication;
    delete[] fConfigBytes;
    fConfigBytes = parseGeneralConfigStr(configStr, fNumConfigBytes);
}

MPEG4VideoStreamFramer::MPEG4VideoStreamFramer(UsageEnvironment& env,
                                               FramedSource*   inputSource,
                                               Boolean         createParser)
    : MPEGVideoStreamFramer(env, inputSource),
    fProfileAndLevelIndication(0),
    fConfigBytes(NULL), fNumConfigBytes(0),
    fNewConfigBytes(NULL), fNumNewConfigBytes(0)
{
    fParser = createParser ?
              new MPEG4VideoStreamParser(this, inputSource) :
              NULL;
}

MPEG4VideoStreamFramer::~MPEG4VideoStreamFramer()
{
    delete[] fConfigBytes;
    delete[] fNewConfigBytes;
}

void MPEG4VideoStreamFramer::startNewConfig()
{
    delete[] fNewConfigBytes;
    fNewConfigBytes    = NULL;
    fNumNewConfigBytes = 0;
}

void MPEG4VideoStreamFramer
::appendToNewConfig(unsigned char* newConfigBytes, unsigned numNewBytes)
{
    unsigned char* configNew =
        new unsigned char[fNumNewConfigBytes + numNewBytes];

    memmove(configNew, fNewConfigBytes, fNumNewConfigBytes);
    memmove(&configNew[fNumNewConfigBytes], newConfigBytes, numNewBytes);
    delete[] fNewConfigBytes;
    fNewConfigBytes     = configNew;
    fNumNewConfigBytes += numNewBytes;
}

void MPEG4VideoStreamFramer::completeNewConfig()
{
    delete[] fConfigBytes;
    fConfigBytes       = fNewConfigBytes;
    fNewConfigBytes    = NULL;
    fNumConfigBytes    = fNumNewConfigBytes;
    fNumNewConfigBytes = 0;
}

Boolean MPEG4VideoStreamFramer::isMPEG4VideoStreamFramer() const
{
    return True;
}

MPEG4VideoStreamParser
::MPEG4VideoStreamParser(MPEG4VideoStreamFramer* usingSource,
                         FramedSource*           inputSource)
    : MPEGVideoStreamParser(usingSource, inputSource),
    fCurrentParseState(PARSING_VISUAL_OBJECT_SEQUENCE),
    vop_time_increment_resolution(0), fNumVTIRBits(0),
    fixed_vop_rate(0), fixed_vop_time_increment(0),
    fSecondsSinceLastTimeCode(0), fTotalTicksSinceLastTimeCode(0),
    fPrevNewTotalTicks(0), fPrevPictureCountDelta(1), fJustSawTimeCode(False)
{}

MPEG4VideoStreamParser::~MPEG4VideoStreamParser()
{}

void MPEG4VideoStreamParser::setParseState(MPEGParseState parseState)
{
    fCurrentParseState = parseState;
    MPEGVideoStreamParser::setParseState();
}

void MPEG4VideoStreamParser::flushInput()
{
    fSecondsSinceLastTimeCode    = 0;
    fTotalTicksSinceLastTimeCode = 0;
    fPrevNewTotalTicks     = 0;
    fPrevPictureCountDelta = 1;
    StreamParser::flushInput();
    if (fCurrentParseState != PARSING_VISUAL_OBJECT_SEQUENCE) {
        setParseState(PARSING_VISUAL_OBJECT_SEQUENCE);
    }
}

unsigned MPEG4VideoStreamParser::parse()
{
    try {
        switch (fCurrentParseState) {
            case PARSING_VISUAL_OBJECT_SEQUENCE: {
                return parseVisualObjectSequence();
            }
            case PARSING_VISUAL_OBJECT_SEQUENCE_SEEN_CODE: {
                return parseVisualObjectSequence(True);
            }
            case PARSING_VISUAL_OBJECT: {
                return parseVisualObject();
            }
            case PARSING_VIDEO_OBJECT_LAYER: {
                return parseVideoObjectLayer();
            }
            case PARSING_GROUP_OF_VIDEO_OBJECT_PLANE: {
                return parseGroupOfVideoObjectPlane();
            }
            case PARSING_VIDEO_OBJECT_PLANE: {
                return parseVideoObjectPlane();
            }
            case PARSING_VISUAL_OBJECT_SEQUENCE_END_CODE: {
                return parseVisualObjectSequenceEndCode();
            }
            default: {
                return 0;
            }
        }
    } catch (int) {
        #ifdef DEBUG
        fprintf(stderr, "MPEG4VideoStreamParser::parse() EXCEPTION (This is normal behavior - *not* an error)\n");
        #endif
        return 0;
    }
} // MPEG4VideoStreamParser::parse

#define VISUAL_OBJECT_SEQUENCE_START_CODE 0x000001B0
#define VISUAL_OBJECT_SEQUENCE_END_CODE   0x000001B1
#define GROUP_VOP_START_CODE              0x000001B3
#define VISUAL_OBJECT_START_CODE          0x000001B5
#define VOP_START_CODE                    0x000001B6
unsigned MPEG4VideoStreamParser
::parseVisualObjectSequence(Boolean haveSeenStartCode)
{
    #ifdef DEBUG
    fprintf(stderr, "parsing VisualObjectSequence\n");
    #endif
    usingSource()->startNewConfig();
    u_int32_t first4Bytes;
    if (!haveSeenStartCode) {
        while ((first4Bytes = test4Bytes()) != VISUAL_OBJECT_SEQUENCE_START_CODE) {
            #ifdef DEBUG
            fprintf(stderr, "ignoring non VS header: 0x%08x\n", first4Bytes);
            #endif
            get1Byte();
            setParseState(PARSING_VISUAL_OBJECT_SEQUENCE);
        }
        first4Bytes = get4Bytes();
    } else {
        first4Bytes = VISUAL_OBJECT_SEQUENCE_START_CODE;
    }
    save4Bytes(first4Bytes);
    u_int8_t pali = get1Byte();
    #ifdef DEBUG
    fprintf(stderr, "profile_and_level_indication: %02x\n", pali);
    #endif
    saveByte(pali);
    usingSource()->fProfileAndLevelIndication = pali;
    u_int32_t next4Bytes = get4Bytes();
    while (next4Bytes != VISUAL_OBJECT_START_CODE) {
        saveToNextCode(next4Bytes);
    }
    setParseState(PARSING_VISUAL_OBJECT);
    usingSource()->computePresentationTime(fTotalTicksSinceLastTimeCode);
    usingSource()->appendToNewConfig(fStartOfFrame, curFrameSize());
    return curFrameSize();
} // MPEG4VideoStreamParser::parseVisualObjectSequence

static inline Boolean isVideoObjectStartCode(u_int32_t code)
{
    return code >= 0x00000100 && code <= 0x0000011F;
}

unsigned MPEG4VideoStreamParser::parseVisualObject()
{
    #ifdef DEBUG
    fprintf(stderr, "parsing VisualObject\n");
    #endif
    save4Bytes(VISUAL_OBJECT_START_CODE);
    u_int8_t nextByte = get1Byte();
    saveByte(nextByte);
    Boolean is_visual_object_identifier = (nextByte & 0x80) != 0;
    u_int8_t visual_object_type;
    if (is_visual_object_identifier) {
        #ifdef DEBUG
        fprintf(stderr, "visual_object_verid: 0x%x; visual_object_priority: 0x%x\n", (nextByte & 0x78) >> 3,
                (nextByte & 0x07));
        #endif
        nextByte = get1Byte();
        saveByte(nextByte);
        visual_object_type = (nextByte & 0xF0) >> 4;
    } else {
        visual_object_type = (nextByte & 0x78) >> 3;
    }
    #ifdef DEBUG
    fprintf(stderr, "visual_object_type: 0x%x\n", visual_object_type);
    #endif
    if (visual_object_type != 1) {
        usingSource()->envir()
            << "MPEG4VideoStreamParser::parseVisualObject(): Warning: We don't handle visual_object_type "
            << visual_object_type << "\n";
    }
    u_int32_t next4Bytes = get4Bytes();
    while (!isVideoObjectStartCode(next4Bytes)) {
        saveToNextCode(next4Bytes);
    }
    save4Bytes(next4Bytes);
    #ifdef DEBUG
    fprintf(stderr, "saw a video_object_start_code: 0x%08x\n", next4Bytes);
    #endif
    setParseState(PARSING_VIDEO_OBJECT_LAYER);
    usingSource()->computePresentationTime(fTotalTicksSinceLastTimeCode);
    usingSource()->appendToNewConfig(fStartOfFrame, curFrameSize());
    return curFrameSize();
} // MPEG4VideoStreamParser::parseVisualObject

static inline Boolean isVideoObjectLayerStartCode(u_int32_t code)
{
    return code >= 0x00000120 && code <= 0x0000012F;
}

Boolean MPEG4VideoStreamParser::getNextFrameBit(u_int8_t& result)
{
    if (fNumBitsSeenSoFar / 8 >= curFrameSize()) {
        return False;
    }
    u_int8_t nextByte = fStartOfFrame[fNumBitsSeenSoFar / 8];
    result = (nextByte >> (7 - fNumBitsSeenSoFar % 8)) & 1;
    ++fNumBitsSeenSoFar;
    return True;
}

Boolean MPEG4VideoStreamParser::getNextFrameBits(unsigned numBits,
                                                 u_int32_t& result)
{
    result = 0;
    for (unsigned i = 0; i < numBits; ++i) {
        u_int8_t nextBit;
        if (!getNextFrameBit(nextBit)) {
            return False;
        }
        result = (result << 1) | nextBit;
    }
    return True;
}

void MPEG4VideoStreamParser::analyzeVOLHeader()
{
    fNumBitsSeenSoFar = 41;
    do {
        u_int8_t is_object_layer_identifier;
        if (!getNextFrameBit(is_object_layer_identifier)) {
            break;
        }
        if (is_object_layer_identifier) {
            fNumBitsSeenSoFar += 7;
        }
        u_int32_t aspect_ratio_info;
        if (!getNextFrameBits(4, aspect_ratio_info)) {
            break;
        }
        if (aspect_ratio_info == 15) {
            fNumBitsSeenSoFar += 16;
        }
        u_int8_t vol_control_parameters;
        if (!getNextFrameBit(vol_control_parameters)) {
            break;
        }
        if (vol_control_parameters) {
            fNumBitsSeenSoFar += 3;
            u_int8_t vbw_parameters;
            if (!getNextFrameBit(vbw_parameters)) {
                break;
            }
            if (vbw_parameters) {
                fNumBitsSeenSoFar += 79;
            }
        }
        fNumBitsSeenSoFar += 2;
        u_int8_t marker_bit;
        if (!getNextFrameBit(marker_bit)) {
            break;
        }
        if (marker_bit != 1) {
            usingSource()->envir() << "MPEG4VideoStreamParser::analyzeVOLHeader(): marker_bit 1 not set!\n";
            break;
        }
        if (!getNextFrameBits(16, vop_time_increment_resolution)) {
            break;
        }
        #ifdef DEBUG
        fprintf(stderr, "vop_time_increment_resolution: %d\n", vop_time_increment_resolution);
        #endif
        if (vop_time_increment_resolution == 0) {
            usingSource()->envir()
                << "MPEG4VideoStreamParser::analyzeVOLHeader(): vop_time_increment_resolution is zero!\n";
            break;
        }
        fNumVTIRBits = 0;
        for (unsigned test = vop_time_increment_resolution; test > 0; test /= 2) {
            ++fNumVTIRBits;
        }
        if (!getNextFrameBit(marker_bit)) {
            break;
        }
        if (marker_bit != 1) {
            usingSource()->envir() << "MPEG4VideoStreamParser::analyzeVOLHeader(): marker_bit 2 not set!\n";
            break;
        }
        if (!getNextFrameBit(fixed_vop_rate)) {
            break;
        }
        if (fixed_vop_rate) {
            if (!getNextFrameBits(fNumVTIRBits, fixed_vop_time_increment)) {
                break;
            }
            #ifdef DEBUG
            fprintf(stderr, "fixed_vop_time_increment: %d\n", fixed_vop_time_increment);
            if (fixed_vop_time_increment == 0) {
                usingSource()->envir()
                    << "MPEG4VideoStreamParser::analyzeVOLHeader(): fixed_vop_time_increment is zero!\n";
            }
            #endif
        }
        usingSource()->fFrameRate = (double) vop_time_increment_resolution;
        #ifdef DEBUG
        fprintf(stderr, "fixed_vop_rate: %d; 'frame' (really tick) rate: %f\n", fixed_vop_rate,
                usingSource()->fFrameRate);
        #endif
        return;
    } while (0);
    if (fNumBitsSeenSoFar / 8 >= curFrameSize()) {
        char errMsg[200];
        sprintf(errMsg, "Not enough bits in VOL header: %d/8 >= %d\n", fNumBitsSeenSoFar, curFrameSize());
        usingSource()->envir() << errMsg;
    }
} // MPEG4VideoStreamParser::analyzeVOLHeader

unsigned MPEG4VideoStreamParser::parseVideoObjectLayer()
{
    #ifdef DEBUG
    fprintf(stderr, "parsing VideoObjectLayer\n");
    #endif
    u_int32_t next4Bytes = get4Bytes();
    if (!isVideoObjectLayerStartCode(next4Bytes)) {
        usingSource()->envir()
            <<
            "MPEG4VideoStreamParser::parseVideoObjectLayer(): This appears to be a 'short video header', which we current don't support\n";
    }
    do {
        saveToNextCode(next4Bytes);
    } while (next4Bytes != GROUP_VOP_START_CODE &&
             next4Bytes != VOP_START_CODE);
    analyzeVOLHeader();
    setParseState((next4Bytes == GROUP_VOP_START_CODE) ?
                  PARSING_GROUP_OF_VIDEO_OBJECT_PLANE :
                  PARSING_VIDEO_OBJECT_PLANE);
    usingSource()->computePresentationTime(fTotalTicksSinceLastTimeCode);
    usingSource()->appendToNewConfig(fStartOfFrame, curFrameSize());
    usingSource()->completeNewConfig();
    return curFrameSize();
}

unsigned MPEG4VideoStreamParser::parseGroupOfVideoObjectPlane()
{
    #ifdef DEBUG
    fprintf(stderr, "parsing GroupOfVideoObjectPlane\n");
    #endif
    save4Bytes(GROUP_VOP_START_CODE);
    u_int8_t next3Bytes[3];
    getBytes(next3Bytes, 3);
    saveByte(next3Bytes[0]);
    saveByte(next3Bytes[1]);
    saveByte(next3Bytes[2]);
    unsigned time_code =
        (next3Bytes[0] << 10) | (next3Bytes[1] << 2) | (next3Bytes[2] >> 6);
    unsigned time_code_hours   = (time_code & 0x0003E000) >> 13;
    unsigned time_code_minutes = (time_code & 0x00001F80) >> 7;
    #if defined(DEBUG) || defined(DEBUG_TIMESTAMPS)
    Boolean marker_bit = (time_code & 0x00000040) != 0;
    #endif
    unsigned time_code_seconds = (time_code & 0x0000003F);
    #if defined(DEBUG) || defined(DEBUG_TIMESTAMPS)
    fprintf(stderr, "time_code: 0x%05x, hours %d, minutes %d, marker_bit %d, seconds %d\n", time_code, time_code_hours,
            time_code_minutes, marker_bit, time_code_seconds);
    #endif
    fJustSawTimeCode = True;
    u_int32_t next4Bytes = get4Bytes();
    while (next4Bytes != VOP_START_CODE) {
        saveToNextCode(next4Bytes);
    }
    usingSource()->computePresentationTime(fTotalTicksSinceLastTimeCode);
    usingSource()->setTimeCode(time_code_hours, time_code_minutes,
                               time_code_seconds, 0, 0);
    fSecondsSinceLastTimeCode = 0;
    if (fixed_vop_rate) {
        fTotalTicksSinceLastTimeCode = 0;
    }
    setParseState(PARSING_VIDEO_OBJECT_PLANE);
    return curFrameSize();
} // MPEG4VideoStreamParser::parseGroupOfVideoObjectPlane

unsigned MPEG4VideoStreamParser::parseVideoObjectPlane()
{
    #ifdef DEBUG
    fprintf(stderr, "#parsing VideoObjectPlane\n");
    #endif
    save4Bytes(VOP_START_CODE);
    u_int8_t nextByte = get1Byte();
    saveByte(nextByte);
    u_int8_t vop_coding_type  = nextByte >> 6;
    u_int32_t next4Bytes      = get4Bytes();
    u_int32_t timeInfo        = (nextByte << (32 - 6)) | (next4Bytes >> 6);
    unsigned modulo_time_base = 0;
    u_int32_t mask = 0x80000000;
    while ((timeInfo & mask) != 0) {
        ++modulo_time_base;
        mask >>= 1;
    }
    mask >>= 1;
    if ((timeInfo & mask) == 0) {
        usingSource()->envir() << "MPEG4VideoStreamParser::parseVideoObjectPlane(): marker bit not set!\n";
    }
    mask >>= 1;
    if ((mask >> (fNumVTIRBits - 1)) == 0) {
        usingSource()->envir()
            <<
            "MPEG4VideoStreamParser::parseVideoObjectPlane(): 32-bits are not enough to get \"vop_time_increment\"!\n";
    }
    unsigned vop_time_increment = 0;
    for (unsigned i = 0; i < fNumVTIRBits; ++i) {
        vop_time_increment |= timeInfo & mask;
        mask >>= 1;
    }
    while (mask != 0) {
        vop_time_increment >>= 1;
        mask >>= 1;
    }
    #ifdef DEBUG
    fprintf(stderr, "vop_coding_type: %d(%c), modulo_time_base: %d, vop_time_increment: %d\n", vop_coding_type,
            "IPBS"[vop_coding_type], modulo_time_base, vop_time_increment);
    #endif
    saveToNextCode(next4Bytes);
    if (fixed_vop_time_increment > 0) {
        usingSource()->fPictureCount += fixed_vop_time_increment;
        if (vop_time_increment > 0 || modulo_time_base > 0) {
            fTotalTicksSinceLastTimeCode += fixed_vop_time_increment;
        }
    } else {
        unsigned newTotalTicks =
            (fSecondsSinceLastTimeCode + modulo_time_base) * vop_time_increment_resolution
            + vop_time_increment;
        if (newTotalTicks == fPrevNewTotalTicks && fPrevNewTotalTicks > 0) {
            #ifdef DEBUG
            fprintf(stderr, "Buggy MPEG-4 video stream: \"vop_time_increment\" did not change!\n");
            #endif
            usingSource()->fPictureCount += vop_time_increment;
            fTotalTicksSinceLastTimeCode += vop_time_increment;
            fSecondsSinceLastTimeCode    += modulo_time_base;
        } else {
            if (newTotalTicks < fPrevNewTotalTicks && vop_coding_type != 2 &&
                modulo_time_base == 0 && vop_time_increment == 0 && !fJustSawTimeCode)
            {
                #ifdef DEBUG
                fprintf(stderr,
                        "Buggy MPEG-4 video stream: \"vop_time_increment\" wrapped around, but without \"modulo_time_base\" changing!\n");
                #endif
                ++fSecondsSinceLastTimeCode;
                newTotalTicks += vop_time_increment_resolution;
            }
            fPrevNewTotalTicks = newTotalTicks;
            if (vop_coding_type != 2) {
                int pictureCountDelta = newTotalTicks - fTotalTicksSinceLastTimeCode;
                if (pictureCountDelta <= 0) {
                    pictureCountDelta = fPrevPictureCountDelta;
                }
                usingSource()->fPictureCount += pictureCountDelta;
                fPrevPictureCountDelta        = pictureCountDelta;
                fTotalTicksSinceLastTimeCode  = newTotalTicks;
                fSecondsSinceLastTimeCode    += modulo_time_base;
            }
        }
    }
    fJustSawTimeCode = False;
    usingSource()->fPictureEndMarker = True;
    switch (next4Bytes) {
        case VISUAL_OBJECT_SEQUENCE_END_CODE: {
            setParseState(PARSING_VISUAL_OBJECT_SEQUENCE_END_CODE);
            break;
        }
        case VISUAL_OBJECT_SEQUENCE_START_CODE: {
            setParseState(PARSING_VISUAL_OBJECT_SEQUENCE_SEEN_CODE);
            break;
        }
        case VISUAL_OBJECT_START_CODE: {
            setParseState(PARSING_VISUAL_OBJECT);
            break;
        }
        case GROUP_VOP_START_CODE: {
            setParseState(PARSING_GROUP_OF_VIDEO_OBJECT_PLANE);
            break;
        }
        case VOP_START_CODE: {
            setParseState(PARSING_VIDEO_OBJECT_PLANE);
            break;
        }
        default: {
            if (isVideoObjectStartCode(next4Bytes)) {
                setParseState(PARSING_VIDEO_OBJECT_LAYER);
            } else if (isVideoObjectLayerStartCode(next4Bytes)) {
                u_int32_t next4Bytes = get4Bytes();
                while (next4Bytes != VOP_START_CODE) {
                    saveToNextCode(next4Bytes);
                }
                setParseState(PARSING_VIDEO_OBJECT_PLANE);
            } else {
                usingSource()->envir() << "MPEG4VideoStreamParser::parseVideoObjectPlane(): Saw unexpected code "
                                       << (void *) next4Bytes << "\n";
                setParseState(PARSING_VIDEO_OBJECT_PLANE);
            }
            break;
        }
    }
    usingSource()->computePresentationTime(fTotalTicksSinceLastTimeCode);
    return curFrameSize();
} // MPEG4VideoStreamParser::parseVideoObjectPlane

unsigned MPEG4VideoStreamParser::parseVisualObjectSequenceEndCode()
{
    #ifdef DEBUG
    fprintf(stderr, "parsing VISUAL_OBJECT_SEQUENCE_END_CODE\n");
    #endif
    save4Bytes(VISUAL_OBJECT_SEQUENCE_END_CODE);
    setParseState(PARSING_VISUAL_OBJECT_SEQUENCE);
    usingSource()->fPictureEndMarker = True;
    return curFrameSize();
}
