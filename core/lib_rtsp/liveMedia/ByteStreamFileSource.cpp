#include "ByteStreamFileSource.hh"
#include "InputFile.hh"
#include "GroupsockHelper.hh"
ByteStreamFileSource * ByteStreamFileSource::createNew(UsageEnvironment& env, char const* fileName,
                                                       unsigned preferredFrameSize,
                                                       unsigned playTimePerFrame)
{
    FILE* fid = OpenInputFile(env, fileName);

    if (fid == NULL) {
        return NULL;
    }
    ByteStreamFileSource* newSource =
        new ByteStreamFileSource(env, fid, preferredFrameSize, playTimePerFrame);
    newSource->fFileSize = GetFileSize(fileName, fid);
    return newSource;
}

ByteStreamFileSource * ByteStreamFileSource::createNew(UsageEnvironment& env, FILE* fid,
                                                       unsigned preferredFrameSize,
                                                       unsigned playTimePerFrame)
{
    if (fid == NULL) {
        return NULL;
    }
    ByteStreamFileSource* newSource = new ByteStreamFileSource(env, fid, preferredFrameSize, playTimePerFrame);
    newSource->fFileSize = GetFileSize(NULL, fid);
    return newSource;
}

void ByteStreamFileSource::seekToByteAbsolute(u_int64_t byteNumber, u_int64_t numBytesToStream)
{
    SeekFile64(fFid, (int64_t) byteNumber, SEEK_SET);
    fNumBytesToStream      = numBytesToStream;
    fLimitNumBytesToStream = fNumBytesToStream > 0;
}

void ByteStreamFileSource::seekToByteRelative(int64_t offset, u_int64_t numBytesToStream)
{
    SeekFile64(fFid, offset, SEEK_CUR);
    fNumBytesToStream      = numBytesToStream;
    fLimitNumBytesToStream = fNumBytesToStream > 0;
}

void ByteStreamFileSource::seekToEnd()
{
    SeekFile64(fFid, 0, SEEK_END);
}

ByteStreamFileSource::ByteStreamFileSource(UsageEnvironment& env, FILE* fid,
                                           unsigned preferredFrameSize,
                                           unsigned playTimePerFrame)
    : FramedFileSource(env, fid), fFileSize(0), fPreferredFrameSize(preferredFrameSize),
    fPlayTimePerFrame(playTimePerFrame), fLastPlayTime(0),
    fHaveStartedReading(False), fLimitNumBytesToStream(False), fNumBytesToStream(0)
{
    #ifndef READ_FROM_FILES_SYNCHRONOUSLY
    makeSocketNonBlocking(fileno(fFid));
    #endif
    fFidIsSeekable = FileIsSeekable(fFid);
}

ByteStreamFileSource::~ByteStreamFileSource()
{
    if (fFid == NULL) {
        return;
    }
    #ifndef READ_FROM_FILES_SYNCHRONOUSLY
    envir().taskScheduler().turnOffBackgroundReadHandling(fileno(fFid));
    #endif
    CloseInputFile(fFid);
}

void ByteStreamFileSource::doGetNextFrame()
{
    if (feof(fFid) || ferror(fFid) || (fLimitNumBytesToStream && fNumBytesToStream == 0)) {
        handleClosure();
        return;
    }
    #ifdef READ_FROM_FILES_SYNCHRONOUSLY
    doReadFromFile();
    #else
    if (!fHaveStartedReading) {
        envir().taskScheduler().turnOnBackgroundReadHandling(fileno(
                                                                 fFid),
                                                             (TaskScheduler::BackgroundHandlerProc *) &fileReadableHandler,
                                                             this);
        fHaveStartedReading = True;
    }
    #endif
}

void ByteStreamFileSource::doStopGettingFrames()
{
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    #ifndef READ_FROM_FILES_SYNCHRONOUSLY
    envir().taskScheduler().turnOffBackgroundReadHandling(fileno(fFid));
    fHaveStartedReading = False;
    #endif
}

void ByteStreamFileSource::fileReadableHandler(ByteStreamFileSource* source, int)
{
    if (!source->isCurrentlyAwaitingData()) {
        source->doStopGettingFrames();
        return;
    }
    source->doReadFromFile();
}

void ByteStreamFileSource::doReadFromFile()
{
    if (fLimitNumBytesToStream && fNumBytesToStream < (u_int64_t) fMaxSize) {
        fMaxSize = (unsigned) fNumBytesToStream;
    }
    if (fPreferredFrameSize > 0 && fPreferredFrameSize < fMaxSize) {
        fMaxSize = fPreferredFrameSize;
    }
    #ifdef READ_FROM_FILES_SYNCHRONOUSLY
    fFrameSize = fread(fTo, 1, fMaxSize, fFid);
    #else
    if (fFidIsSeekable) {
        fFrameSize = fread(fTo, 1, fMaxSize, fFid);
    } else {
        fFrameSize = read(fileno(fFid), fTo, fMaxSize);
    }
    #endif
    if (fFrameSize == 0) {
        handleClosure();
        return;
    }
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
    #ifdef READ_FROM_FILES_SYNCHRONOUSLY
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                                                             (TaskFunc *) FramedSource::afterGetting, this);
    #else
    FramedSource::afterGetting(this);
    #endif
} // ByteStreamFileSource::doReadFromFile
