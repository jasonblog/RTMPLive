#include "AMRAudioFileSource.hh"
#include "InputFile.hh"
#include "GroupsockHelper.hh"
AMRAudioFileSource * AMRAudioFileSource::createNew(UsageEnvironment& env, char const* fileName)
{
    FILE* fid = NULL;
    Boolean magicNumberOK = True;

    do {
        fid = OpenInputFile(env, fileName);
        if (fid == NULL) {
            break;
        }
        magicNumberOK = False;
        Boolean isWideband   = False;
        unsigned numChannels = 1;
        char buf[100];
        if (fread(buf, 1, 6, fid) < 6) {
            break;
        }
        if (strncmp(buf, "#!AMR", 5) != 0) {
            break;
        }
        unsigned bytesRead = 6;
        if (buf[5] == '-') {
            if (fread(&buf[bytesRead], 1, 3, fid) < 3) {
                break;
            }
            if (strncmp(&buf[bytesRead], "WB", 2) != 0) {
                break;
            }
            isWideband = True;
            bytesRead += 3;
        }
        if (buf[bytesRead - 1] == '_') {
            if (fread(&buf[bytesRead], 1, 6, fid) < 6) {
                break;
            }
            if (strncmp(&buf[bytesRead], "MC1.0\n", 6) != 0) {
                break;
            }
            bytesRead += 6;
            char channelDesc[4];
            if (fread(channelDesc, 1, 4, fid) < 4) {
                break;
            }
            numChannels = channelDesc[3] & 0xF;
        } else if (buf[bytesRead - 1] != '\n') {
            break;
        }
        magicNumberOK = True;
        #ifdef DEBUG
        fprintf(stderr, "isWideband: %d, numChannels: %d\n",
                isWideband, numChannels);
        #endif
        return new AMRAudioFileSource(env, fid, isWideband, numChannels);
    } while (0);
    CloseInputFile(fid);
    if (!magicNumberOK) {
        env.setResultMsg("Bad (or nonexistent) AMR file header");
    }
    return NULL;
} // AMRAudioFileSource::createNew

AMRAudioFileSource
::AMRAudioFileSource(UsageEnvironment& env, FILE* fid,
                     Boolean isWideband, unsigned numChannels)
    : AMRAudioSource(env, isWideband, numChannels),
    fFid(fid)
{}

AMRAudioFileSource::~AMRAudioFileSource()
{
    CloseInputFile(fFid);
}

#define FT_INVALID 65535
static unsigned short const frameSize[16] = {
    12,         13,         15,         17,
    19,         20,         26,         31,
    5,          FT_INVALID, FT_INVALID, FT_INVALID,
    FT_INVALID, FT_INVALID, FT_INVALID, 0
};
static unsigned short const frameSizeWideband[16] = {
    17,         23,         32,         36,
    40,         46,         50,         58,
    60,         5,          FT_INVALID, FT_INVALID,
    FT_INVALID, FT_INVALID, 0,          0
};
void AMRAudioFileSource::doGetNextFrame()
{
    if (feof(fFid) || ferror(fFid)) {
        handleClosure();
        return;
    }
    while (1) {
        if (fread(&fLastFrameHeader, 1, 1, fFid) < 1) {
            handleClosure();
            return;
        }
        if ((fLastFrameHeader & 0x83) != 0) {
            #ifdef DEBUG
            fprintf(stderr, "Invalid frame header 0x%02x (padding bits (0x83) are not zero)\n", fLastFrameHeader);
            #endif
        } else {
            unsigned char ft = (fLastFrameHeader & 0x78) >> 3;
            fFrameSize = fIsWideband ? frameSizeWideband[ft] : frameSize[ft];
            if (fFrameSize == FT_INVALID) {
                #ifdef DEBUG
                fprintf(stderr, "Invalid FT field %d (from frame header 0x%02x)\n",
                        ft, fLastFrameHeader);
                #endif
            } else {
                #ifdef DEBUG
                fprintf(stderr, "Valid frame header 0x%02x -> ft %d -> frame size %d\n", fLastFrameHeader, ft,
                        fFrameSize);
                #endif
                break;
            }
        }
    }
    fFrameSize *= fNumChannels;
    if (fFrameSize > fMaxSize) {
        fNumTruncatedBytes = fFrameSize - fMaxSize;
        fFrameSize         = fMaxSize;
    }
    fFrameSize = fread(fTo, 1, fFrameSize, fFid);
    if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
        gettimeofday(&fPresentationTime, NULL);
    } else {
        unsigned uSeconds = fPresentationTime.tv_usec + 20000;
        fPresentationTime.tv_sec += uSeconds / 1000000;
        fPresentationTime.tv_usec = uSeconds % 1000000;
    }
    fDurationInMicroseconds = 20000;
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                                                             (TaskFunc *) FramedSource::afterGetting, this);
} // AMRAudioFileSource::doGetNextFrame
