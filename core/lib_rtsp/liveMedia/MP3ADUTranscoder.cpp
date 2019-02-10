#include "MP3ADUTranscoder.hh"
#include "MP3Internals.hh"
#include <string.h>
MP3ADUTranscoder::MP3ADUTranscoder(UsageEnvironment& env,
                                   unsigned        outBitrate,
                                   FramedSource*   inputSource)
    : FramedFilter(env, inputSource),
    fOutBitrate(outBitrate),
    fAvailableBytesForBackpointer(0),
    fOrigADU(new unsigned char[MAX_MP3_FRAME_SIZE])
{}

MP3ADUTranscoder::~MP3ADUTranscoder()
{
    delete[] fOrigADU;
}

MP3ADUTranscoder * MP3ADUTranscoder::createNew(UsageEnvironment& env,
                                               unsigned        outBitrate,
                                               FramedSource*   inputSource)
{
    if (strcmp(inputSource->MIMEtype(), "audio/MPA-ROBUST") != 0) {
        env.setResultMsg(inputSource->name(), " is not an MP3 ADU source");
        return NULL;
    }
    return new MP3ADUTranscoder(env, outBitrate, inputSource);
}

void MP3ADUTranscoder::getAttributes() const
{
    fInputSource->getAttributes();
    char buffer[30];
    sprintf(buffer, " bandwidth %d", outBitrate());
    envir().appendToResultMsg(buffer);
}

void MP3ADUTranscoder::doGetNextFrame()
{
    fInputSource->getNextFrame(fOrigADU, MAX_MP3_FRAME_SIZE,
                               afterGettingFrame, this, handleClosure, this);
}

void MP3ADUTranscoder::afterGettingFrame(void*          clientData,
                                         unsigned       numBytesRead,
                                         unsigned       numTruncatedBytes,
                                         struct timeval presentationTime,
                                         unsigned       durationInMicroseconds)
{
    MP3ADUTranscoder* transcoder = (MP3ADUTranscoder *) clientData;

    transcoder->afterGettingFrame1(numBytesRead, numTruncatedBytes,
                                   presentationTime, durationInMicroseconds);
}

void MP3ADUTranscoder::afterGettingFrame1(unsigned       numBytesRead,
                                          unsigned       numTruncatedBytes,
                                          struct timeval presentationTime,
                                          unsigned       durationInMicroseconds)
{
    fNumTruncatedBytes      = numTruncatedBytes;
    fPresentationTime       = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    fFrameSize = TranscodeMP3ADU(fOrigADU, numBytesRead, fOutBitrate,
                                 fTo, fMaxSize, fAvailableBytesForBackpointer);
    if (fFrameSize == 0) {
        handleClosure();
        return;
    }
    afterGetting(this);
}
