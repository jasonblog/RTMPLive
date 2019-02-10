#include "ByteStreamMemoryBufferSource.hh"
#include "GroupsockHelper.hh"
ByteStreamMemoryBufferSource * ByteStreamMemoryBufferSource::createNew(UsageEnvironment& env,
                                                                       u_int8_t* buffer, u_int64_t bufferSize,
                                                                       Boolean deleteBufferOnClose,
                                                                       unsigned preferredFrameSize,
                                                                       unsigned playTimePerFrame)
{
    if (buffer == NULL) {
        return NULL;
    }
    return new ByteStreamMemoryBufferSource(env, buffer, bufferSize, deleteBufferOnClose, preferredFrameSize,
                                            playTimePerFrame);
}

ByteStreamMemoryBufferSource::ByteStreamMemoryBufferSource(UsageEnvironment& env,
                                                           u_int8_t* buffer, u_int64_t bufferSize,
                                                           Boolean deleteBufferOnClose,
                                                           unsigned preferredFrameSize,
                                                           unsigned playTimePerFrame)
    : FramedSource(env), fBuffer(buffer), fBufferSize(bufferSize), fCurIndex(0), fDeleteBufferOnClose(
        deleteBufferOnClose),
    fPreferredFrameSize(preferredFrameSize), fPlayTimePerFrame(playTimePerFrame), fLastPlayTime(0),
    fLimitNumBytesToStream(False), fNumBytesToStream(0)
{}

ByteStreamMemoryBufferSource::~ByteStreamMemoryBufferSource()
{
    if (fDeleteBufferOnClose) {
        delete[] fBuffer;
    }
}

void ByteStreamMemoryBufferSource::seekToByteAbsolute(u_int64_t byteNumber, u_int64_t numBytesToStream)
{
    fCurIndex = byteNumber;
    if (fCurIndex > fBufferSize) {
        fCurIndex = fBufferSize;
    }
    fNumBytesToStream      = numBytesToStream;
    fLimitNumBytesToStream = fNumBytesToStream > 0;
}

void ByteStreamMemoryBufferSource::seekToByteRelative(int64_t offset, u_int64_t numBytesToStream)
{
    int64_t newIndex = fCurIndex + offset;

    if (newIndex < 0) {
        fCurIndex = 0;
    } else {
        fCurIndex = (u_int64_t) offset;
        if (fCurIndex > fBufferSize) {
            fCurIndex = fBufferSize;
        }
    }
    fNumBytesToStream      = numBytesToStream;
    fLimitNumBytesToStream = fNumBytesToStream > 0;
}

void ByteStreamMemoryBufferSource::doGetNextFrame()
{
    if (fCurIndex >= fBufferSize || (fLimitNumBytesToStream && fNumBytesToStream == 0)) {
        handleClosure();
        return;
    }
    fFrameSize = fMaxSize;
    if (fLimitNumBytesToStream && fNumBytesToStream < (u_int64_t) fFrameSize) {
        fFrameSize = (unsigned) fNumBytesToStream;
    }
    if (fPreferredFrameSize > 0 && fPreferredFrameSize < fFrameSize) {
        fFrameSize = fPreferredFrameSize;
    }
    if (fCurIndex + fFrameSize > fBufferSize) {
        fFrameSize = (unsigned) (fBufferSize - fCurIndex);
    }
    memmove(fTo, &fBuffer[fCurIndex], fFrameSize);
    fCurIndex         += fFrameSize;
    fNumBytesToStream -= fFrameSize;
    if (fPlayTimePerFrame > 0 && fPreferredFrameSize > 0) {
        if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
            gettimeofday(&fPresentationTime, NULL);
        } else {
            unsigned uSeconds = fPresentationTime.tv_usec + fLastPlayTime;
            fPresentationTime.tv_sec += uSeconds / 1000000;
            fPresentationTime.tv_usec = uSeconds % 1000000;
        }
        fLastPlayTime = (fPlayTimePerFrame * fFrameSize) / fPreferredFrameSize;
        fDurationInMicroseconds = fLastPlayTime;
    } else {
        gettimeofday(&fPresentationTime, NULL);
    }
    FramedSource::afterGetting(this);
} // ByteStreamMemoryBufferSource::doGetNextFrame
