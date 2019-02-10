#include "AMRAudioFileSink.hh"
#include "AMRAudioSource.hh"
#include "OutputFile.hh"
AMRAudioFileSink
::AMRAudioFileSink(UsageEnvironment& env, FILE* fid, unsigned bufferSize,
                   char const* perFrameFileNamePrefix)
    : FileSink(env, fid, bufferSize, perFrameFileNamePrefix),
    fHaveWrittenHeader(False)
{}

AMRAudioFileSink::~AMRAudioFileSink()
{}

AMRAudioFileSink * AMRAudioFileSink::createNew(UsageEnvironment& env, char const* fileName,
                                               unsigned bufferSize, Boolean oneFilePerFrame)
{
    do {
        FILE* fid;
        char const* perFrameFileNamePrefix;
        if (oneFilePerFrame) {
            fid = NULL;
            perFrameFileNamePrefix = fileName;
        } else {
            fid = OpenOutputFile(env, fileName);
            if (fid == NULL) {
                break;
            }
            perFrameFileNamePrefix = NULL;
        }
        return new AMRAudioFileSink(env, fid, bufferSize, perFrameFileNamePrefix);
    } while (0);
    return NULL;
}

Boolean AMRAudioFileSink::sourceIsCompatibleWithUs(MediaSource& source)
{
    return source.isAMRAudioSource();
}

void AMRAudioFileSink::afterGettingFrame(unsigned       frameSize,
                                         unsigned       numTruncatedBytes,
                                         struct timeval presentationTime)
{
    AMRAudioSource* source = (AMRAudioSource *) fSource;

    if (source == NULL) {
        return;
    }
    if (!fHaveWrittenHeader && fPerFrameFileNameBuffer == NULL) {
        char headerBuffer[100];
        sprintf(headerBuffer, "#!AMR%s%s\n",
                source->isWideband() ? "-WB" : "",
                source->numChannels() > 1 ? "_MC1.0" : "");
        unsigned headerLength = strlen(headerBuffer);
        if (source->numChannels() > 1) {
            headerBuffer[headerLength++] = 0;
            headerBuffer[headerLength++] = 0;
            headerBuffer[headerLength++] = 0;
            headerBuffer[headerLength++] = source->numChannels();
        }
        addData((unsigned char *) headerBuffer, headerLength, presentationTime);
    }
    fHaveWrittenHeader = True;
    if (fPerFrameFileNameBuffer == NULL) {
        u_int8_t frameHeader = source->lastFrameHeader();
        addData(&frameHeader, 1, presentationTime);
    }
    FileSink::afterGettingFrame(frameSize, numTruncatedBytes, presentationTime);
}
