#include "MPEG1or2VideoStreamFramer.hh"
#include "MPEGVideoStreamParser.hh"
#include <string.h>
enum MPEGParseState {
    PARSING_VIDEO_SEQUENCE_HEADER,
    PARSING_VIDEO_SEQUENCE_HEADER_SEEN_CODE,
    PARSING_GOP_HEADER,
    PARSING_GOP_HEADER_SEEN_CODE,
    PARSING_PICTURE_HEADER,
    PARSING_SLICE
};
#define VSH_MAX_SIZE 1000
class MPEG1or2VideoStreamParser : public MPEGVideoStreamParser
{
public:
    MPEG1or2VideoStreamParser(MPEG1or2VideoStreamFramer* usingSource,
                              FramedSource* inputSource,
                              Boolean iFramesOnly, double vshPeriod);
    virtual ~MPEG1or2VideoStreamParser();
private:
    virtual void flushInput();
    virtual unsigned parse();
private:
    void reset();
    MPEG1or2VideoStreamFramer * usingSource()
    {
        return (MPEG1or2VideoStreamFramer *) fUsingSource;
    }

    void setParseState(MPEGParseState parseState);
    unsigned parseVideoSequenceHeader(Boolean haveSeenStartCode);
    unsigned parseGOPHeader(Boolean haveSeenStartCode);
    unsigned parsePictureHeader();
    unsigned parseSlice();
private:
    MPEGParseState fCurrentParseState;
    unsigned fPicturesSinceLastGOP;
    unsigned short fCurPicTemporalReference;
    unsigned char fCurrentSliceNumber;
    unsigned char fSavedVSHBuffer[VSH_MAX_SIZE];
    unsigned fSavedVSHSize;
    double fSavedVSHTimestamp;
    double fVSHPeriod;
    Boolean fIFramesOnly, fSkippingCurrentPicture;
    void saveCurrentVSH();
    Boolean needToUseSavedVSH();
    unsigned useSavedVSH();
};
MPEG1or2VideoStreamFramer::MPEG1or2VideoStreamFramer(UsageEnvironment& env,
                                                     FramedSource*   inputSource,
                                                     Boolean         iFramesOnly,
                                                     double          vshPeriod,
                                                     Boolean         createParser)
    : MPEGVideoStreamFramer(env, inputSource)
{
    fParser = createParser ?
              new MPEG1or2VideoStreamParser(this, inputSource,
                                            iFramesOnly, vshPeriod) :
              NULL;
}

MPEG1or2VideoStreamFramer::~MPEG1or2VideoStreamFramer()
{}

MPEG1or2VideoStreamFramer * MPEG1or2VideoStreamFramer::createNew(UsageEnvironment& env,
                                                                 FramedSource*   inputSource,
                                                                 Boolean         iFramesOnly,
                                                                 double          vshPeriod)
{
    return new MPEG1or2VideoStreamFramer(env, inputSource, iFramesOnly, vshPeriod);
}

double MPEG1or2VideoStreamFramer::getCurrentPTS() const
{
    return fPresentationTime.tv_sec + fPresentationTime.tv_usec / 1000000.0;
}

Boolean MPEG1or2VideoStreamFramer::isMPEG1or2VideoStreamFramer() const
{
    return True;
}

MPEG1or2VideoStreamParser
::MPEG1or2VideoStreamParser(MPEG1or2VideoStreamFramer* usingSource,
                            FramedSource* inputSource,
                            Boolean iFramesOnly, double vshPeriod)
    : MPEGVideoStreamParser(usingSource, inputSource),
    fCurrentParseState(PARSING_VIDEO_SEQUENCE_HEADER),
    fVSHPeriod(vshPeriod), fIFramesOnly(iFramesOnly)
{
    reset();
}

MPEG1or2VideoStreamParser::~MPEG1or2VideoStreamParser()
{}

void MPEG1or2VideoStreamParser::setParseState(MPEGParseState parseState)
{
    fCurrentParseState = parseState;
    MPEGVideoStreamParser::setParseState();
}

void MPEG1or2VideoStreamParser::reset()
{
    fPicturesSinceLastGOP    = 0;
    fCurPicTemporalReference = 0;
    fCurrentSliceNumber      = 0;
    fSavedVSHSize = 0;
    fSkippingCurrentPicture = False;
}

void MPEG1or2VideoStreamParser::flushInput()
{
    reset();
    StreamParser::flushInput();
    if (fCurrentParseState != PARSING_VIDEO_SEQUENCE_HEADER) {
        setParseState(PARSING_GOP_HEADER);
    }
}

unsigned MPEG1or2VideoStreamParser::parse()
{
    try {
        switch (fCurrentParseState) {
            case PARSING_VIDEO_SEQUENCE_HEADER: {
                return parseVideoSequenceHeader(False);
            }
            case PARSING_VIDEO_SEQUENCE_HEADER_SEEN_CODE: {
                return parseVideoSequenceHeader(True);
            }
            case PARSING_GOP_HEADER: {
                return parseGOPHeader(False);
            }
            case PARSING_GOP_HEADER_SEEN_CODE: {
                return parseGOPHeader(True);
            }
            case PARSING_PICTURE_HEADER: {
                return parsePictureHeader();
            }
            case PARSING_SLICE: {
                return parseSlice();
            }
            default: {
                return 0;
            }
        }
    } catch (int) {
        #ifdef DEBUG
        fprintf(stderr, "MPEG1or2VideoStreamParser::parse() EXCEPTION (This is normal behavior - *not* an error)\n");
        #endif
        return 0;
    }
}

void MPEG1or2VideoStreamParser::saveCurrentVSH()
{
    unsigned frameSize = curFrameSize();

    if (frameSize > sizeof fSavedVSHBuffer) {
        return;
    }
    memmove(fSavedVSHBuffer, fStartOfFrame, frameSize);
    fSavedVSHSize      = frameSize;
    fSavedVSHTimestamp = usingSource()->getCurrentPTS();
}

Boolean MPEG1or2VideoStreamParser::needToUseSavedVSH()
{
    return usingSource()->getCurrentPTS() > fSavedVSHTimestamp + fVSHPeriod &&
           fSavedVSHSize > 0;
}

unsigned MPEG1or2VideoStreamParser::useSavedVSH()
{
    unsigned bytesToUse    = fSavedVSHSize;
    unsigned maxBytesToUse = fLimit - fStartOfFrame;

    if (bytesToUse > maxBytesToUse) {
        bytesToUse = maxBytesToUse;
    }
    memmove(fStartOfFrame, fSavedVSHBuffer, bytesToUse);
    fSavedVSHTimestamp = usingSource()->getCurrentPTS();
    #ifdef DEBUG
    fprintf(stderr, "used saved video_sequence_header (%d bytes)\n", bytesToUse);
    #endif
    return bytesToUse;
}

#define VIDEO_SEQUENCE_HEADER_START_CODE 0x000001B3
#define GROUP_START_CODE                 0x000001B8
#define PICTURE_START_CODE               0x00000100
#define SEQUENCE_END_CODE                0x000001B7
static double const frameRateFromCode[] = {
    0.0,
    24000 / 1001.0,
    24.0,
    25.0,
    30000 / 1001.0,
    30.0,
    50.0,
    60000 / 1001.0,
    60.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0
};
unsigned MPEG1or2VideoStreamParser
::parseVideoSequenceHeader(Boolean haveSeenStartCode)
{
    #ifdef DEBUG
    fprintf(stderr, "parsing video sequence header\n");
    #endif
    unsigned first4Bytes;
    if (!haveSeenStartCode) {
        while ((first4Bytes = test4Bytes()) != VIDEO_SEQUENCE_HEADER_START_CODE) {
            #ifdef DEBUG
            fprintf(stderr, "ignoring non video sequence header: 0x%08x\n", first4Bytes);
            #endif
            get1Byte();
            setParseState(PARSING_VIDEO_SEQUENCE_HEADER);
        }
        first4Bytes = get4Bytes();
    } else {
        first4Bytes = VIDEO_SEQUENCE_HEADER_START_CODE;
    }
    save4Bytes(first4Bytes);
    unsigned paramWord1 = get4Bytes();
    save4Bytes(paramWord1);
    unsigned next4Bytes = get4Bytes();
    #ifdef DEBUG
    unsigned short horizontal_size_value   = (paramWord1 & 0xFFF00000) >> (32 - 12);
    unsigned short vertical_size_value     = (paramWord1 & 0x000FFF00) >> 8;
    unsigned char aspect_ratio_information = (paramWord1 & 0x000000F0) >> 4;
    #endif
    unsigned char frame_rate_code = (paramWord1 & 0x0000000F);
    usingSource()->fFrameRate = frameRateFromCode[frame_rate_code];
    #ifdef DEBUG
    unsigned bit_rate_value        = (next4Bytes & 0xFFFFC000) >> (32 - 18);
    unsigned vbv_buffer_size_value = (next4Bytes & 0x00001FF8) >> 3;
    fprintf(stderr,
            "horizontal_size_value: %d, vertical_size_value: %d, aspect_ratio_information: %d, frame_rate_code: %d (=>%f fps), bit_rate_value: %d (=>%d bps), vbv_buffer_size_value: %d\n", horizontal_size_value, vertical_size_value, aspect_ratio_information, frame_rate_code,
            usingSource()->fFrameRate, bit_rate_value, bit_rate_value * 400, vbv_buffer_size_value);
    #endif
    do {
        saveToNextCode(next4Bytes);
    } while (next4Bytes != GROUP_START_CODE && next4Bytes != PICTURE_START_CODE);
    setParseState((next4Bytes == GROUP_START_CODE) ?
                  PARSING_GOP_HEADER_SEEN_CODE : PARSING_PICTURE_HEADER);
    usingSource()->computePresentationTime(fPicturesSinceLastGOP);
    saveCurrentVSH();
    return curFrameSize();
} // MPEG1or2VideoStreamParser::parseVideoSequenceHeader

unsigned MPEG1or2VideoStreamParser::parseGOPHeader(Boolean haveSeenStartCode)
{
    if (needToUseSavedVSH()) {
        return useSavedVSH();
    }
    #ifdef DEBUG
    fprintf(stderr, "parsing GOP header\n");
    #endif
    unsigned first4Bytes;
    if (!haveSeenStartCode) {
        while ((first4Bytes = test4Bytes()) != GROUP_START_CODE) {
            #ifdef DEBUG
            fprintf(stderr, "ignoring non GOP start code: 0x%08x\n", first4Bytes);
            #endif
            get1Byte();
            setParseState(PARSING_GOP_HEADER);
        }
        first4Bytes = get4Bytes();
    } else {
        first4Bytes = GROUP_START_CODE;
    }
    save4Bytes(first4Bytes);
    unsigned next4Bytes = get4Bytes();
    unsigned time_code  = (next4Bytes & 0xFFFFFF80) >> (32 - 25);
    #if defined(DEBUG) || defined(DEBUG_TIMESTAMPS)
    Boolean drop_frame_flag = (time_code & 0x01000000) != 0;
    #endif
    unsigned time_code_hours    = (time_code & 0x00F80000) >> 19;
    unsigned time_code_minutes  = (time_code & 0x0007E000) >> 13;
    unsigned time_code_seconds  = (time_code & 0x00000FC0) >> 6;
    unsigned time_code_pictures = (time_code & 0x0000003F);
    #if defined(DEBUG) || defined(DEBUG_TIMESTAMPS)
    fprintf(stderr, "time_code: 0x%07x, drop_frame %d, hours %d, minutes %d, seconds %d, pictures %d\n", time_code,
            drop_frame_flag, time_code_hours, time_code_minutes, time_code_seconds, time_code_pictures);
    #endif
    #ifdef DEBUG
    Boolean closed_gop  = (next4Bytes & 0x00000040) != 0;
    Boolean broken_link = (next4Bytes & 0x00000020) != 0;
    fprintf(stderr, "closed_gop: %d, broken_link: %d\n", closed_gop, broken_link);
    #endif
    do {
        saveToNextCode(next4Bytes);
    } while (next4Bytes != PICTURE_START_CODE);
    usingSource()->setTimeCode(time_code_hours, time_code_minutes,
                               time_code_seconds, time_code_pictures,
                               fPicturesSinceLastGOP);
    fPicturesSinceLastGOP = 0;
    usingSource()->computePresentationTime(0);
    setParseState(PARSING_PICTURE_HEADER);
    return curFrameSize();
} // MPEG1or2VideoStreamParser::parseGOPHeader

inline Boolean isSliceStartCode(unsigned fourBytes)
{
    if ((fourBytes & 0xFFFFFF00) != 0x00000100) {
        return False;
    }
    unsigned char lastByte = fourBytes & 0xFF;
    return lastByte <= 0xAF && lastByte >= 1;
}

unsigned MPEG1or2VideoStreamParser::parsePictureHeader()
{
    #ifdef DEBUG
    fprintf(stderr, "parsing picture header\n");
    #endif
    unsigned next4Bytes = get4Bytes();
    unsigned short temporal_reference = (next4Bytes & 0xFFC00000) >> (32 - 10);
    unsigned char picture_coding_type = (next4Bytes & 0x00380000) >> 19;
    #ifdef DEBUG
    unsigned short vbv_delay = (next4Bytes & 0x0007FFF8) >> 3;
    fprintf(stderr, "temporal_reference: %d, picture_coding_type: %d, vbv_delay: %d\n", temporal_reference,
            picture_coding_type, vbv_delay);
    #endif
    fSkippingCurrentPicture = fIFramesOnly && picture_coding_type != 1;
    if (fSkippingCurrentPicture) {
        do {
            skipToNextCode(next4Bytes);
        } while (!isSliceStartCode(next4Bytes));
    } else {
        save4Bytes(PICTURE_START_CODE);
        do {
            saveToNextCode(next4Bytes);
        } while (!isSliceStartCode(next4Bytes));
    }
    setParseState(PARSING_SLICE);
    fCurrentSliceNumber      = next4Bytes & 0xFF;
    fCurPicTemporalReference = temporal_reference;
    usingSource()->computePresentationTime(fCurPicTemporalReference);
    if (fSkippingCurrentPicture) {
        return parse();
    } else {
        return curFrameSize();
    }
} // MPEG1or2VideoStreamParser::parsePictureHeader

unsigned MPEG1or2VideoStreamParser::parseSlice()
{
    unsigned next4Bytes = PICTURE_START_CODE | fCurrentSliceNumber;

    #ifdef DEBUG_SLICE
    fprintf(stderr, "parsing slice: 0x%08x\n", next4Bytes);
    #endif
    if (fSkippingCurrentPicture) {
        skipToNextCode(next4Bytes);
    } else {
        saveToNextCode(next4Bytes);
    }
    if (isSliceStartCode(next4Bytes)) {
        setParseState(PARSING_SLICE);
        fCurrentSliceNumber = next4Bytes & 0xFF;
    } else {
        ++fPicturesSinceLastGOP;
        ++usingSource()->fPictureCount;
        usingSource()->fPictureEndMarker = True;
        switch (next4Bytes) {
            case SEQUENCE_END_CODE: {
                setParseState(PARSING_VIDEO_SEQUENCE_HEADER);
                break;
            }
            case VIDEO_SEQUENCE_HEADER_START_CODE: {
                setParseState(PARSING_VIDEO_SEQUENCE_HEADER_SEEN_CODE);
                break;
            }
            case GROUP_START_CODE: {
                setParseState(PARSING_GOP_HEADER_SEEN_CODE);
                break;
            }
            case PICTURE_START_CODE: {
                setParseState(PARSING_PICTURE_HEADER);
                break;
            }
            default: {
                usingSource()->envir() << "MPEG1or2VideoStreamParser::parseSlice(): Saw unexpected code "
                                       << (void *) next4Bytes << "\n";
                setParseState(PARSING_SLICE);
                break;
            }
        }
    }
    usingSource()->computePresentationTime(fCurPicTemporalReference);
    if (fSkippingCurrentPicture) {
        return parse();
    } else {
        return curFrameSize();
    }
} // MPEG1or2VideoStreamParser::parseSlice
