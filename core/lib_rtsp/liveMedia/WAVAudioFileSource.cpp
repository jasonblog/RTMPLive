#include "WAVAudioFileSource.hh"
#include "InputFile.hh"
#include "GroupsockHelper.hh"
WAVAudioFileSource * WAVAudioFileSource::createNew(UsageEnvironment& env, char const* fileName)
{
    do {
        FILE* fid = OpenInputFile(env, fileName);
        if (fid == NULL) {
            break;
        }
        WAVAudioFileSource* newSource = new WAVAudioFileSource(env, fid);
        if (newSource != NULL && newSource->bitsPerSample() == 0) {
            Medium::close(newSource);
            break;
        }
        newSource->fFileSize = (unsigned) GetFileSize(fileName, fid);
        return newSource;
    } while (0);
    return NULL;
}

unsigned WAVAudioFileSource::numPCMBytes() const
{
    if (fFileSize < fWAVHeaderSize) {
        return 0;
    }
    return fFileSize - fWAVHeaderSize;
}

void WAVAudioFileSource::setScaleFactor(int scale)
{
    if (!fFidIsSeekable) {
        return;
    }
    fScaleFactor = scale;
    if (fScaleFactor < 0 && TellFile64(fFid) > 0) {
        int bytesPerSample = (fNumChannels * fBitsPerSample) / 8;
        if (bytesPerSample == 0) {
            bytesPerSample = 1;
        }
        SeekFile64(fFid, -bytesPerSample, SEEK_CUR);
    }
}

void WAVAudioFileSource::seekToPCMByte(unsigned byteNumber)
{
    byteNumber += fWAVHeaderSize;
    if (byteNumber > fFileSize) {
        byteNumber = fFileSize;
    }
    SeekFile64(fFid, byteNumber, SEEK_SET);
}

void WAVAudioFileSource::limitNumBytesToStream(unsigned numBytesToStream)
{
    fNumBytesToStream      = numBytesToStream;
    fLimitNumBytesToStream = fNumBytesToStream > 0;
}

unsigned char WAVAudioFileSource::getAudioFormat()
{
    return fAudioFormat;
}

#define nextc fgetc(fid)
static Boolean get4Bytes(FILE* fid, u_int32_t& result)
{
    int c0, c1, c2, c3;

    if ((c0 = nextc) == EOF || (c1 = nextc) == EOF ||
        (c2 = nextc) == EOF || (c3 = nextc) == EOF)
    {
        return False;
    }
    result = (c3 << 24) | (c2 << 16) | (c1 << 8) | c0;
    return True;
}

static Boolean get2Bytes(FILE* fid, u_int16_t& result)
{
    int c0, c1;

    if ((c0 = nextc) == EOF || (c1 = nextc) == EOF) {
        return False;
    }
    result = (c1 << 8) | c0;
    return True;
}

static Boolean skipBytes(FILE* fid, int num)
{
    while (num-- > 0) {
        if (nextc == EOF) {
            return False;
        }
    }
    return True;
}

WAVAudioFileSource::WAVAudioFileSource(UsageEnvironment& env, FILE* fid)
    : AudioInputDevice(env, 0, 0, 0, 0),
    fFid(fid), fFidIsSeekable(False), fLastPlayTime(0), fHaveStartedReading(False), fWAVHeaderSize(0), fFileSize(0),
    fScaleFactor(1), fLimitNumBytesToStream(False), fNumBytesToStream(0), fAudioFormat(WA_UNKNOWN)
{
    Boolean success = False;

    do {
        if (nextc != 'R' || nextc != 'I' || nextc != 'F' || nextc != 'F') {
            break;
        }
        if (!skipBytes(fid, 4)) {
            break;
        }
        if (nextc != 'W' || nextc != 'A' || nextc != 'V' || nextc != 'E') {
            break;
        }
        u_int32_t tmp;
        if (!get4Bytes(fid, tmp)) {
            break;
        }
        if (tmp != 0x20746d66) {
            if (!get4Bytes(fid, tmp)) {
                break;
            }
            if (!skipBytes(fid, tmp)) {
                break;
            }
        }
        unsigned formatLength;
        if (!get4Bytes(fid, formatLength)) {
            break;
        }
        unsigned short audioFormat;
        if (!get2Bytes(fid, audioFormat)) {
            break;
        }
        fAudioFormat = (unsigned char) audioFormat;
        if (fAudioFormat != WA_PCM && fAudioFormat != WA_PCMA && fAudioFormat != WA_PCMU &&
            fAudioFormat != WA_IMA_ADPCM)
        {
            env.setResultMsg("Audio format is not one that we handle (PCM/PCMU/PCMA or IMA ADPCM)");
            break;
        }
        unsigned short numChannels;
        if (!get2Bytes(fid, numChannels)) {
            break;
        }
        fNumChannels = (unsigned char) numChannels;
        if (fNumChannels < 1 || fNumChannels > 2) {
            char errMsg[100];
            sprintf(errMsg, "Bad # channels: %d", fNumChannels);
            env.setResultMsg(errMsg);
            break;
        }
        if (!get4Bytes(fid, fSamplingFrequency)) {
            break;
        }
        if (fSamplingFrequency == 0) {
            env.setResultMsg("Bad sampling frequency: 0");
            break;
        }
        if (!skipBytes(fid, 6)) {
            break;
        }
        unsigned short bitsPerSample;
        if (!get2Bytes(fid, bitsPerSample)) {
            break;
        }
        fBitsPerSample = (unsigned char) bitsPerSample;
        if (fBitsPerSample == 0) {
            env.setResultMsg("Bad bits-per-sample: 0");
            break;
        }
        if (!skipBytes(fid, formatLength - 16)) {
            break;
        }
        int c = nextc;
        if (c == 'f') {
            if (nextc != 'a' || nextc != 'c' || nextc != 't') {
                break;
            }
            unsigned factLength;
            if (!get4Bytes(fid, factLength)) {
                break;
            }
            if (!skipBytes(fid, factLength)) {
                break;
            }
            c = nextc;
        }
        if (c != 'd' || nextc != 'a' || nextc != 't' || nextc != 'a') {
            break;
        }
        if (!skipBytes(fid, 4)) {
            break;
        }
        fWAVHeaderSize = (unsigned) TellFile64(fid);
        success        = True;
    } while (0);
    if (!success) {
        env.setResultMsg("Bad WAV file format");
        fBitsPerSample = 0;
        return;
    }
    fPlayTimePerSample = 1e6 / (double) fSamplingFrequency;
    unsigned maxSamplesPerFrame     = (1400 * 8) / (fNumChannels * fBitsPerSample);
    unsigned desiredSamplesPerFrame = (unsigned) (0.02 * fSamplingFrequency);
    unsigned samplesPerFrame        = desiredSamplesPerFrame <
                                      maxSamplesPerFrame ? desiredSamplesPerFrame : maxSamplesPerFrame;
    fPreferredFrameSize = (samplesPerFrame * fNumChannels * fBitsPerSample) / 8;
    fFidIsSeekable      = FileIsSeekable(fFid);
    #ifndef READ_FROM_FILES_SYNCHRONOUSLY
    makeSocketNonBlocking(fileno(fFid));
    #endif
}

WAVAudioFileSource::~WAVAudioFileSource()
{
    if (fFid == NULL) {
        return;
    }
    #ifndef READ_FROM_FILES_SYNCHRONOUSLY
    envir().taskScheduler().turnOffBackgroundReadHandling(fileno(fFid));
    #endif
    CloseInputFile(fFid);
}

void WAVAudioFileSource::doGetNextFrame()
{
    if (feof(fFid) || ferror(fFid) || (fLimitNumBytesToStream && fNumBytesToStream == 0)) {
        handleClosure();
        return;
    }
    fFrameSize = 0;
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

void WAVAudioFileSource::doStopGettingFrames()
{
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    #ifndef READ_FROM_FILES_SYNCHRONOUSLY
    envir().taskScheduler().turnOffBackgroundReadHandling(fileno(fFid));
    fHaveStartedReading = False;
    #endif
}

void WAVAudioFileSource::fileReadableHandler(WAVAudioFileSource* source, int)
{
    if (!source->isCurrentlyAwaitingData()) {
        source->doStopGettingFrames();
        return;
    }
    source->doReadFromFile();
}

void WAVAudioFileSource::doReadFromFile()
{
    if (fLimitNumBytesToStream && fNumBytesToStream < fMaxSize) {
        fMaxSize = fNumBytesToStream;
    }
    if (fPreferredFrameSize < fMaxSize) {
        fMaxSize = fPreferredFrameSize;
    }
    unsigned bytesPerSample = (fNumChannels * fBitsPerSample) / 8;
    if (bytesPerSample == 0) {
        bytesPerSample = 1;
    }
    unsigned bytesToRead = fScaleFactor == 1 ? fMaxSize - fMaxSize % bytesPerSample : bytesPerSample;
    unsigned numBytesRead;
    while (1) {
        #ifdef READ_FROM_FILES_SYNCHRONOUSLY
        numBytesRead = fread(fTo, 1, bytesToRead, fFid);
        #else
        if (fFidIsSeekable) {
            numBytesRead = fread(fTo, 1, bytesToRead, fFid);
        } else {
            numBytesRead = read(fileno(fFid), fTo, bytesToRead);
        }
        #endif
        if (numBytesRead == 0) {
            handleClosure();
            return;
        }
        fFrameSize        += numBytesRead;
        fTo               += numBytesRead;
        fMaxSize          -= numBytesRead;
        fNumBytesToStream -= numBytesRead;
        #ifndef READ_FROM_FILES_SYNCHRONOUSLY
        if (fFrameSize % bytesPerSample > 0) {
            return;
        }
        #endif
        if (fScaleFactor != 1) {
            SeekFile64(fFid, (fScaleFactor - 1)*bytesPerSample, SEEK_CUR);
            if (fMaxSize < bytesPerSample) {
                break;
            }
        } else {
            break;
        }
    }
    if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
        gettimeofday(&fPresentationTime, NULL);
    } else {
        unsigned uSeconds = fPresentationTime.tv_usec + fLastPlayTime;
        fPresentationTime.tv_sec += uSeconds / 1000000;
        fPresentationTime.tv_usec = uSeconds % 1000000;
    }
    fDurationInMicroseconds = fLastPlayTime =
        (unsigned) ((fPlayTimePerSample * fFrameSize) / bytesPerSample);
    #ifdef READ_FROM_FILES_SYNCHRONOUSLY
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                                                             (TaskFunc *) FramedSource::afterGetting, this);
    #else
    FramedSource::afterGetting(this);
    #endif
} // WAVAudioFileSource::doReadFromFile

Boolean WAVAudioFileSource::setInputPort(int)
{
    return True;
}

double WAVAudioFileSource::getAverageLevel() const
{
    return 0.0;
}
