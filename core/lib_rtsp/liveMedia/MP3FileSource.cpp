#include "MP3FileSource.hh"
#include "MP3StreamState.hh"
#include "InputFile.hh"
MP3FileSource::MP3FileSource(UsageEnvironment& env, FILE* fid)
    : FramedFileSource(env, fid),
    fStreamState(new MP3StreamState(env))
{}

MP3FileSource::~MP3FileSource()
{
    delete fStreamState;
}

char const * MP3FileSource::MIMEtype() const
{
    return "audio/MPEG";
}

MP3FileSource * MP3FileSource::createNew(UsageEnvironment& env, char const* fileName)
{
    MP3FileSource* newSource = NULL;

    do {
        FILE* fid;
        fid = OpenInputFile(env, fileName);
        if (fid == NULL) {
            break;
        }
        newSource = new MP3FileSource(env, fid);
        if (newSource == NULL) {
            break;
        }
        unsigned fileSize = (unsigned) GetFileSize(fileName, fid);
        newSource->assignStream(fid, fileSize);
        if (!newSource->initializeStream()) {
            break;
        }
        return newSource;
    } while (0);
    Medium::close(newSource);
    return NULL;
}

float MP3FileSource::filePlayTime() const
{
    return fStreamState->filePlayTime();
}

unsigned MP3FileSource::fileSize() const
{
    return fStreamState->fileSize();
}

void MP3FileSource::setPresentationTimeScale(unsigned scale)
{
    fStreamState->setPresentationTimeScale(scale);
}

void MP3FileSource::seekWithinFile(double seekNPT, double streamDuration)
{
    float fileDuration = filePlayTime();

    if (seekNPT < 0.0) {
        seekNPT = 0.0;
    } else if (seekNPT > fileDuration) {
        seekNPT = fileDuration;
    }
    if (streamDuration < 0.0) {
        streamDuration = 0.0;
    } else if (seekNPT + streamDuration > fileDuration) {
        streamDuration = fileDuration - seekNPT;
    }
    float seekFraction      = (float) seekNPT / fileDuration;
    unsigned seekByteNumber = fStreamState->getByteNumberFromPositionFraction(seekFraction);
    fStreamState->seekWithinFile(seekByteNumber);
    fLimitNumBytesToStream = False;
    if (streamDuration > 0.0) {
        float endFraction      = (float) (seekNPT + streamDuration) / fileDuration;
        unsigned endByteNumber = fStreamState->getByteNumberFromPositionFraction(endFraction);
        if (endByteNumber > seekByteNumber) {
            fNumBytesToStream      = endByteNumber - seekByteNumber;
            fLimitNumBytesToStream = True;
        }
    }
}

void MP3FileSource::getAttributes() const
{
    char buffer[200];

    fStreamState->getAttributes(buffer, sizeof buffer);
    envir().setResultMsg(buffer);
}

void MP3FileSource::doGetNextFrame()
{
    if (!doGetNextFrame1()) {
        handleClosure();
        return;
    }
    #if defined(__WIN32__) || defined(_WIN32)
    afterGetting(this);
    #else
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                                                             (TaskFunc *) afterGetting, this);
    #endif
}

Boolean MP3FileSource::doGetNextFrame1()
{
    if (fLimitNumBytesToStream && fNumBytesToStream == 0) {
        return False;
    }
    if (!fHaveJustInitialized) {
        if (fStreamState->findNextHeader(fPresentationTime) == 0) {
            return False;
        }
    } else {
        fPresentationTime    = fFirstFramePresentationTime;
        fHaveJustInitialized = False;
    }
    if (!fStreamState->readFrame(fTo, fMaxSize, fFrameSize, fDurationInMicroseconds)) {
        char tmp[200];
        sprintf(tmp,
                "Insufficient buffer size %d for reading MPEG audio frame (needed %d)\n",
                fMaxSize, fFrameSize);
        envir().setResultMsg(tmp);
        fFrameSize = fMaxSize;
        return False;
    }
    if (fNumBytesToStream > fFrameSize) {
        fNumBytesToStream -= fFrameSize;
    } else {
        fNumBytesToStream = 0;
    }
    return True;
}

void MP3FileSource::assignStream(FILE* fid, unsigned fileSize)
{
    fStreamState->assignStream(fid, fileSize);
}

Boolean MP3FileSource::initializeStream()
{
    if (fStreamState->findNextHeader(fFirstFramePresentationTime) == 0) {
        envir().setResultMsg("not an MPEG audio file");
        return False;
    }
    fStreamState->checkForXingHeader();
    fHaveJustInitialized   = True;
    fLimitNumBytesToStream = False;
    fNumBytesToStream      = 0;
    envir().setResultMsg(name());
    return True;
}
