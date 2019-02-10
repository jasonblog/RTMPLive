#include "H263plusVideoStreamParser.hh"
#include "H263plusVideoStreamFramer.hh"
H263plusVideoStreamParser::H263plusVideoStreamParser(
    H263plusVideoStreamFramer* usingSource,
    FramedSource*              inputSource)
    : StreamParser(inputSource,
                   FramedSource::handleClosure,
                   usingSource,
                   &H263plusVideoStreamFramer::continueReadProcessing,
                   usingSource),
    fUsingSource(usingSource),
    fnextTR(0),
    fcurrentPT(0)
{
    memset(fStates, 0, sizeof(fStates));
    memset(&fNextInfo, 0, sizeof(fNextInfo));
    memset(&fCurrentInfo, 0, sizeof(fCurrentInfo));
    memset(&fMaxBitrateCtx, 0, sizeof(fMaxBitrateCtx));
    memset(fNextHeader, 0, H263_REQUIRE_HEADER_SIZE_BYTES);
}

H263plusVideoStreamParser::~H263plusVideoStreamParser()
{}

void H263plusVideoStreamParser::restoreSavedParserState()
{
    StreamParser::restoreSavedParserState();
    fTo = fSavedTo;
    fNumTruncatedBytes = fSavedNumTruncatedBytes;
}

void H263plusVideoStreamParser::setParseState()
{
    fSavedTo = fTo;
    fSavedNumTruncatedBytes = fNumTruncatedBytes;
    saveParserState();
}

void H263plusVideoStreamParser::registerReadInterest(
    unsigned char* to,
    unsigned       maxSize)
{
    fStartOfFrame      = fTo = fSavedTo = to;
    fLimit             = to + maxSize;
    fMaxSize           = maxSize;
    fNumTruncatedBytes = fSavedNumTruncatedBytes = 0;
}

unsigned H263plusVideoStreamParser::parse(u_int64_t& currentDuration)
{
    u_int32_t frameSize;
    u_int8_t trDifference;

    try {
        fCurrentInfo    = fNextInfo;
        frameSize       = parseH263Frame();
        currentDuration = 0;
        if ((frameSize > 0)) {
            if (!ParseShortHeader(fTo, &fNextInfo)) {
                #ifdef DEBUG
                fprintf(stderr, "H263plusVideoStreamParser: Fatal error\n");
                #endif
            }
            trDifference    = GetTRDifference(fNextInfo.tr, fCurrentInfo.tr);
            currentDuration = CalculateDuration(trDifference);
            setParseState();
        }
    } catch (int) {
        #ifdef DEBUG
        fprintf(stderr, "H263plusVideoStreamParser::parse() EXCEPTION (This is normal behavior - *not* an error)\n");
        #endif
        frameSize = 0;
    }
    return frameSize;
}

int H263plusVideoStreamParser::parseH263Frame()
{
    char row = 0;
    u_int8_t* bufferIndex = fTo;
    u_int8_t* bufferEnd   = fTo + fMaxSize - ADDITIONAL_BYTES_NEEDED - 1;

    memcpy(fTo, fNextHeader, H263_REQUIRE_HEADER_SIZE_BYTES);
    bufferIndex += H263_REQUIRE_HEADER_SIZE_BYTES;
    if (!fStates[0][0]) {
        fStates[0][0]   = 1;
        fStates[1][0]   = fStates[2][0] = 2;
        fStates[2][128] = fStates[2][129] = fStates[2][130] = fStates[2][131] = -1;
    }
    do {
        *bufferIndex = get1Byte();
    } while ((bufferIndex < bufferEnd) &&
             ((row = fStates[(unsigned char) row][*(bufferIndex++)]) != -1));
    if (row != -1) {
        fprintf(stderr, "%s: Buffer too small (%lu)\n",
                "h263reader:", bufferEnd - fTo + ADDITIONAL_BYTES_NEEDED);
        return 0;
    }
    getBytes(bufferIndex, ADDITIONAL_BYTES_NEEDED);
    memcpy(fNextHeader, bufferIndex - H263_STARTCODE_SIZE_BYTES, H263_REQUIRE_HEADER_SIZE_BYTES);
    int sz = bufferIndex - fTo - H263_STARTCODE_SIZE_BYTES;
    if (sz == 5) {
        memcpy(fTo, fTo + H263_REQUIRE_HEADER_SIZE_BYTES, H263_REQUIRE_HEADER_SIZE_BYTES);
    }
    return sz;
}

bool H263plusVideoStreamParser::ParseShortHeader(
    u_int8_t* headerBuffer,
    H263INFO* outputInfoStruct)
{
    u_int8_t fmt = 0;

    outputInfoStruct->tr  = (headerBuffer[2] << 6) & 0xC0;
    outputInfoStruct->tr |= (headerBuffer[3] >> 2) & 0x3F;
    fmt = (headerBuffer[4] >> 2) & 0x07;
    if (fmt == 0x07) {
        return false;
    }
    if (!GetWidthAndHeight(fmt, &(outputInfoStruct->width),
                           &(outputInfoStruct->height)))
    {
        return false;
    }
    outputInfoStruct->isSyncFrame = !(headerBuffer[4] & 0x02);
    return true;
}

void H263plusVideoStreamParser::GetMaxBitrate(MaxBitrate_CTX* ctx,
                                              u_int32_t       frameSize,
                                              u_int8_t        frameTRDiff)
{
    if (frameTRDiff == 0) {
        return;
    }
    u_int32_t frameBitrate = frameSize * 8 / frameTRDiff + 1;
    while (frameTRDiff--) {
        ctx->windowBitrate -= ctx->bitrateTable[ctx->tableIndex];
        ctx->bitrateTable[ctx->tableIndex] = frameBitrate;
        ctx->windowBitrate += frameBitrate;
        if (ctx->windowBitrate > ctx->maxBitrate) {
            ctx->maxBitrate = ctx->windowBitrate;
        }
        ctx->tableIndex = (ctx->tableIndex + 1)
                          %(sizeof(ctx->bitrateTable) / sizeof(ctx->bitrateTable[0]));
    }
}

u_int64_t H263plusVideoStreamParser::CalculateDuration(u_int8_t trDiff)
{
    u_int64_t nextPT;
    u_int64_t duration;

    fnextTR   += trDiff;
    nextPT     = (fnextTR * 1001) / H263_BASIC_FRAME_RATE;
    duration   = nextPT - fcurrentPT;
    fcurrentPT = nextPT;
    return duration;
}

bool H263plusVideoStreamParser::GetWidthAndHeight(u_int8_t   fmt,
                                                  u_int16_t* width,
                                                  u_int16_t* height)
{
    static struct {
        u_int16_t width;
        u_int16_t height;
    } const dimensionsTable[8] = {
        { 0,    0    },
        { 128,  96   },
        { 176,  144  },
        { 352,  288  },
        { 704,  576  },
        { 1409, 1152 },
        { 0,    0    },
        { 0,    0    }
    };

    if (fmt > 7) {
        return false;
    }
    *width  = dimensionsTable[fmt].width;
    *height = dimensionsTable[fmt].height;
    if (*width == 0) {
        return false;
    }
    return true;
}

u_int8_t H263plusVideoStreamParser::GetTRDifference(
    u_int8_t nextTR,
    u_int8_t currentTR)
{
    if (currentTR > nextTR) {
        return nextTR + (256 - currentTR);
    } else {
        return nextTR - currentTR;
    }
}
