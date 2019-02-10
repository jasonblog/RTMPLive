#if (defined(__WIN32__) || defined(_WIN32)) && !defined(_WIN32_WCE)
# include <io.h>
# include <fcntl.h>
#endif
#include "FileSink.hh"
#include "GroupsockHelper.hh"
#include "OutputFile.hh"
FileSink::FileSink(UsageEnvironment& env, FILE* fid, unsigned bufferSize,
                   char const* perFrameFileNamePrefix)
    : MediaSink(env), fOutFid(fid), fBufferSize(bufferSize), fSamePresentationTimeCounter(0)
{
    fBuffer = new unsigned char[bufferSize];
    if (perFrameFileNamePrefix != NULL) {
        fPerFrameFileNamePrefix = strDup(perFrameFileNamePrefix);
        fPerFrameFileNameBuffer = new char[strlen(perFrameFileNamePrefix) + 100];
    } else {
        fPerFrameFileNamePrefix = NULL;
        fPerFrameFileNameBuffer = NULL;
    }
    fPrevPresentationTime.tv_sec  = ~0;
    fPrevPresentationTime.tv_usec = 0;
}

FileSink::~FileSink()
{
    delete[] fPerFrameFileNameBuffer;
    delete[] fPerFrameFileNamePrefix;
    delete[] fBuffer;
    if (fOutFid != NULL) {
        fclose(fOutFid);
    }
}

FileSink * FileSink::createNew(UsageEnvironment& env, char const* fileName,
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
        return new FileSink(env, fid, bufferSize, perFrameFileNamePrefix);
    } while (0);
    return NULL;
}

Boolean FileSink::continuePlaying()
{
    if (fSource == NULL) {
        return False;
    }
    fSource->getNextFrame(fBuffer, fBufferSize,
                          afterGettingFrame, this,
                          onSourceClosure, this);
    return True;
}

void FileSink::afterGettingFrame(void* clientData, unsigned frameSize,
                                 unsigned numTruncatedBytes,
                                 struct timeval presentationTime,
                                 unsigned)
{
    FileSink* sink = (FileSink *) clientData;

    sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime);
}

void FileSink::addData(unsigned char const* data, unsigned dataSize,
                       struct timeval presentationTime)
{
    if (fPerFrameFileNameBuffer != NULL && fOutFid == NULL) {
        if (presentationTime.tv_usec == fPrevPresentationTime.tv_usec &&
            presentationTime.tv_sec == fPrevPresentationTime.tv_sec)
        {
            sprintf(fPerFrameFileNameBuffer, "%s-%lu.%06lu-%u", fPerFrameFileNamePrefix,
                    presentationTime.tv_sec, presentationTime.tv_usec, ++fSamePresentationTimeCounter);
        } else {
            sprintf(fPerFrameFileNameBuffer, "%s-%lu.%06lu", fPerFrameFileNamePrefix,
                    presentationTime.tv_sec, presentationTime.tv_usec);
            fPrevPresentationTime        = presentationTime;
            fSamePresentationTimeCounter = 0;
        }
        fOutFid = OpenOutputFile(envir(), fPerFrameFileNameBuffer);
    }
    #ifdef TEST_LOSS
    static unsigned const framesPerPacket = 10;
    static unsigned const frameCount      = 0;
    static Boolean const packetIsLost;
    if ((frameCount++) % framesPerPacket == 0) {
        packetIsLost = (our_random() % 10 == 0);
    }
    if (!packetIsLost)
    #endif
    if (fOutFid != NULL && data != NULL) {
        fwrite(data, 1, dataSize, fOutFid);
    }
}

void FileSink::afterGettingFrame(unsigned       frameSize,
                                 unsigned       numTruncatedBytes,
                                 struct timeval presentationTime)
{
    if (numTruncatedBytes > 0) {
        envir() << "FileSink::afterGettingFrame(): The input frame data was too large for our buffer size ("
                << fBufferSize << ").  "
                << numTruncatedBytes
                <<
            " bytes of trailing data was dropped!  Correct this by increasing the \"bufferSize\" parameter in the \"createNew()\" call to at least "
                << fBufferSize + numTruncatedBytes << "\n";
    }
    addData(fBuffer, frameSize, presentationTime);
    if (fOutFid == NULL || fflush(fOutFid) == EOF) {
        if (fSource != NULL) {
            fSource->stopGettingFrames();
        }
        onSourceClosure();
        return;
    }
    if (fPerFrameFileNameBuffer != NULL) {
        if (fOutFid != NULL) {
            fclose(fOutFid);
            fOutFid = NULL;
        }
    }
    continuePlaying();
}
