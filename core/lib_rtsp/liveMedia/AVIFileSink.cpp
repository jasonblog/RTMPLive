#include "AVIFileSink.hh"
#include "InputFile.hh"
#include "OutputFile.hh"
#include "GroupsockHelper.hh"
#define fourChar(x, y, z, w) ( ((w)<<24)|((z)<<16)|((y)<<8)|(x) )
class SubsessionBuffer
{
public:
    SubsessionBuffer(unsigned bufferSize)
        : fBufferSize(bufferSize)
    {
        reset();
        fData = new unsigned char[bufferSize];
    }

    virtual ~SubsessionBuffer()
    {
        delete[] fData;
    }

    void reset()
    {
        fBytesInUse = 0;
    }

    void addBytes(unsigned numBytes)
    {
        fBytesInUse += numBytes;
    }

    unsigned char * dataStart()
    {
        return &fData[0];
    }

    unsigned char * dataEnd()
    {
        return &fData[fBytesInUse];
    }

    unsigned bytesInUse() const
    {
        return fBytesInUse;
    }

    unsigned bytesAvailable() const
    {
        return fBufferSize - fBytesInUse;
    }

    void setPresentationTime(struct timeval const& presentationTime)
    {
        fPresentationTime = presentationTime;
    }

    struct timeval const& presentationTime() const
    {
        return fPresentationTime;
    }

private:
    unsigned fBufferSize;
    struct timeval fPresentationTime;
    unsigned char* fData;
    unsigned fBytesInUse;
};
class AVISubsessionIOState
{
public:
    AVISubsessionIOState(AVIFileSink& sink, MediaSubsession& subsession);
    virtual ~AVISubsessionIOState();
    void setAVIstate(unsigned subsessionIndex);
    void setFinalAVIstate();
    void afterGettingFrame(unsigned       packetDataSize,
                           struct timeval presentationTime);
    void onSourceClosure();
    UsageEnvironment& envir() const
    {
        return fOurSink.envir();
    }

public:
    SubsessionBuffer* fBuffer, * fPrevBuffer;
    AVIFileSink& fOurSink;
    MediaSubsession& fOurSubsession;
    unsigned short fLastPacketRTPSeqNum;
    Boolean fOurSourceIsActive;
    struct timeval fPrevPresentationTime;
    unsigned fMaxBytesPerSecond;
    Boolean fIsVideo, fIsAudio, fIsByteSwappedAudio;
    unsigned fAVISubsessionTag;
    unsigned fAVICodecHandlerType;
    unsigned fAVISamplingFrequency;
    u_int16_t fWAVCodecTag;
    unsigned fAVIScale;
    unsigned fAVIRate;
    unsigned fAVISize;
    unsigned fNumFrames;
    unsigned fSTRHFrameCountPosition;
private:
    void useFrame(SubsessionBuffer& buffer);
};
class AVIIndexRecord
{
public:
    AVIIndexRecord(unsigned chunkId, unsigned flags, unsigned offset, unsigned size)
        : fNext(NULL), fChunkId(chunkId), fFlags(flags), fOffset(offset), fSize(size)
    {}

    AVIIndexRecord *& next()
    {
        return fNext;
    }

    unsigned chunkId() const
    {
        return fChunkId;
    }

    unsigned flags() const
    {
        return fFlags;
    }

    unsigned offset() const
    {
        return fOffset;
    }

    unsigned size() const
    {
        return fSize;
    }

private:
    AVIIndexRecord* fNext;
    unsigned fChunkId;
    unsigned fFlags;
    unsigned fOffset;
    unsigned fSize;
};
AVIFileSink::AVIFileSink(UsageEnvironment& env,
                         MediaSession& inputSession,
                         char const* outputFileName,
                         unsigned bufferSize,
                         unsigned short movieWidth, unsigned short movieHeight,
                         unsigned movieFPS, Boolean packetLossCompensate)
    : Medium(env), fInputSession(inputSession),
    fIndexRecordsHead(NULL), fIndexRecordsTail(NULL), fNumIndexRecords(0),
    fBufferSize(bufferSize), fPacketLossCompensate(packetLossCompensate),
    fAreCurrentlyBeingPlayed(False), fNumSubsessions(0), fNumBytesWritten(0),
    fHaveCompletedOutputFile(False),
    fMovieWidth(movieWidth), fMovieHeight(movieHeight), fMovieFPS(movieFPS)
{
    fOutFid = OpenOutputFile(env, outputFileName);
    if (fOutFid == NULL) {
        return;
    }
    MediaSubsessionIterator iter(fInputSession);
    MediaSubsession* subsession;
    while ((subsession = iter.next()) != NULL) {
        FramedSource* subsessionSource = subsession->readSource();
        if (subsessionSource == NULL) {
            continue;
        }
        if (subsession->videoWidth() != 0) {
            fMovieWidth = subsession->videoWidth();
        }
        if (subsession->videoHeight() != 0) {
            fMovieHeight = subsession->videoHeight();
        }
        if (subsession->videoFPS() != 0) {
            fMovieFPS = subsession->videoFPS();
        }
        AVISubsessionIOState* ioState =
            new AVISubsessionIOState(*this, *subsession);
        subsession->miscPtr = (void *) ioState;
        if (subsession->rtcpInstance() != NULL) {
            subsession->rtcpInstance()->setByeHandler(onRTCPBye, ioState);
        }
        ++fNumSubsessions;
    }
    addFileHeader_AVI();
}

AVIFileSink::~AVIFileSink()
{
    completeOutputFile();
    MediaSubsessionIterator iter(fInputSession);
    MediaSubsession* subsession;
    while ((subsession = iter.next()) != NULL) {
        if (subsession->readSource() != NULL) {
            subsession->readSource()->stopGettingFrames();
        }
        AVISubsessionIOState* ioState =
            (AVISubsessionIOState *) (subsession->miscPtr);
        if (ioState == NULL) {
            continue;
        }
        delete ioState;
    }
    AVIIndexRecord* cur = fIndexRecordsHead;
    while (cur != NULL) {
        AVIIndexRecord* next = cur->next();
        delete cur;
        cur = next;
    }
    CloseOutputFile(fOutFid);
}

AVIFileSink * AVIFileSink
::createNew(UsageEnvironment& env, MediaSession& inputSession,
            char const* outputFileName,
            unsigned bufferSize,
            unsigned short movieWidth, unsigned short movieHeight,
            unsigned movieFPS, Boolean packetLossCompensate)
{
    AVIFileSink* newSink =
        new AVIFileSink(env, inputSession, outputFileName, bufferSize,
                        movieWidth, movieHeight, movieFPS, packetLossCompensate);

    if (newSink == NULL || newSink->fOutFid == NULL) {
        Medium::close(newSink);
        return NULL;
    }
    return newSink;
}

Boolean AVIFileSink::startPlaying(afterPlayingFunc* afterFunc,
                                  void*             afterClientData)
{
    if (fAreCurrentlyBeingPlayed) {
        envir().setResultMsg("This sink has already been played");
        return False;
    }
    fAreCurrentlyBeingPlayed = True;
    fAfterFunc       = afterFunc;
    fAfterClientData = afterClientData;
    return continuePlaying();
}

Boolean AVIFileSink::continuePlaying()
{
    Boolean haveActiveSubsessions = False;
    MediaSubsessionIterator iter(fInputSession);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
        FramedSource* subsessionSource = subsession->readSource();
        if (subsessionSource == NULL) {
            continue;
        }
        if (subsessionSource->isCurrentlyAwaitingData()) {
            continue;
        }
        AVISubsessionIOState* ioState =
            (AVISubsessionIOState *) (subsession->miscPtr);
        if (ioState == NULL) {
            continue;
        }
        haveActiveSubsessions = True;
        unsigned char* toPtr = ioState->fBuffer->dataEnd();
        unsigned toSize      = ioState->fBuffer->bytesAvailable();
        subsessionSource->getNextFrame(toPtr, toSize,
                                       afterGettingFrame, ioState,
                                       onSourceClosure, ioState);
    }
    if (!haveActiveSubsessions) {
        envir().setResultMsg("No subsessions are currently active");
        return False;
    }
    return True;
}

void AVIFileSink
::afterGettingFrame(void* clientData, unsigned packetDataSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned)
{
    AVISubsessionIOState* ioState = (AVISubsessionIOState *) clientData;

    if (numTruncatedBytes > 0) {
        ioState->envir() << "AVIFileSink::afterGettingFrame(): The input frame data was too large for our buffer.  "
                         << numTruncatedBytes
                         <<
            " bytes of trailing data was dropped!  Correct this by increasing the \"bufferSize\" parameter in the \"createNew()\" call.\n";
    }
    ioState->afterGettingFrame(packetDataSize, presentationTime);
}

void AVIFileSink::onSourceClosure(void* clientData)
{
    AVISubsessionIOState* ioState = (AVISubsessionIOState *) clientData;

    ioState->onSourceClosure();
}

void AVIFileSink::onSourceClosure1()
{
    MediaSubsessionIterator iter(fInputSession);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
        AVISubsessionIOState* ioState =
            (AVISubsessionIOState *) (subsession->miscPtr);
        if (ioState == NULL) {
            continue;
        }
        if (ioState->fOurSourceIsActive) {
            return;
        }
    }
    completeOutputFile();
    if (fAfterFunc != NULL) {
        (*fAfterFunc)(fAfterClientData);
    }
}

void AVIFileSink::onRTCPBye(void* clientData)
{
    AVISubsessionIOState* ioState = (AVISubsessionIOState *) clientData;
    struct timeval timeNow;

    gettimeofday(&timeNow, NULL);
    unsigned secsDiff =
        timeNow.tv_sec - ioState->fOurSink.fStartTime.tv_sec;
    MediaSubsession& subsession = ioState->fOurSubsession;
    ioState->envir() << "Received RTCP \"BYE\" on \""
                     << subsession.mediumName()
                     << "/" << subsession.codecName()
                     << "\" subsession (after "
                     << secsDiff << " seconds)\n";
    ioState->onSourceClosure();
}

void AVIFileSink::addIndexRecord(AVIIndexRecord* newIndexRecord)
{
    if (fIndexRecordsHead == NULL) {
        fIndexRecordsHead = newIndexRecord;
    } else {
        fIndexRecordsTail->next() = newIndexRecord;
    }
    fIndexRecordsTail = newIndexRecord;
    ++fNumIndexRecords;
}

void AVIFileSink::completeOutputFile()
{
    if (fHaveCompletedOutputFile || fOutFid == NULL) {
        return;
    }
    unsigned maxBytesPerSecond = 0;
    unsigned numVideoFrames    = 0;
    unsigned numAudioFrames    = 0;
    MediaSubsessionIterator iter(fInputSession);
    MediaSubsession* subsession;
    while ((subsession = iter.next()) != NULL) {
        AVISubsessionIOState* ioState =
            (AVISubsessionIOState *) (subsession->miscPtr);
        if (ioState == NULL) {
            continue;
        }
        maxBytesPerSecond += ioState->fMaxBytesPerSecond;
        setWord(ioState->fSTRHFrameCountPosition, ioState->fNumFrames);
        if (ioState->fIsVideo) {
            numVideoFrames = ioState->fNumFrames;
        } else if (ioState->fIsAudio) {
            numAudioFrames = ioState->fNumFrames;
        }
    }
    add4ByteString("idx1");
    addWord(fNumIndexRecords * 4 * 4);
    for (AVIIndexRecord* indexRecord = fIndexRecordsHead; indexRecord != NULL; indexRecord = indexRecord->next()) {
        addWord(indexRecord->chunkId());
        addWord(indexRecord->flags());
        addWord(indexRecord->offset());
        addWord(indexRecord->size());
    }
    fRIFFSizeValue += fNumBytesWritten;
    setWord(fRIFFSizePosition, fRIFFSizeValue);
    setWord(fAVIHMaxBytesPerSecondPosition, maxBytesPerSecond);
    setWord(fAVIHFrameCountPosition,
            numVideoFrames > 0 ? numVideoFrames : numAudioFrames);
    fMoviSizeValue += fNumBytesWritten;
    setWord(fMoviSizePosition, fMoviSizeValue);
    fHaveCompletedOutputFile = True;
} // AVIFileSink::completeOutputFile

AVISubsessionIOState::AVISubsessionIOState(AVIFileSink    & sink,
                                           MediaSubsession& subsession)
    : fOurSink(sink), fOurSubsession(subsession),
    fMaxBytesPerSecond(0), fIsVideo(False), fIsAudio(False), fIsByteSwappedAudio(False), fNumFrames(0)
{
    fBuffer     = new SubsessionBuffer(fOurSink.fBufferSize);
    fPrevBuffer = sink.fPacketLossCompensate ?
                  new SubsessionBuffer(fOurSink.fBufferSize) : NULL;
    FramedSource* subsessionSource = subsession.readSource();
    fOurSourceIsActive = subsessionSource != NULL;
    fPrevPresentationTime.tv_sec  = 0;
    fPrevPresentationTime.tv_usec = 0;
}

AVISubsessionIOState::~AVISubsessionIOState()
{
    delete fBuffer;
    delete fPrevBuffer;
}

void AVISubsessionIOState::setAVIstate(unsigned subsessionIndex)
{
    fIsVideo = strcmp(fOurSubsession.mediumName(), "video") == 0;
    fIsAudio = strcmp(fOurSubsession.mediumName(), "audio") == 0;
    if (fIsVideo) {
        fAVISubsessionTag =
            fourChar('0' + subsessionIndex / 10, '0' + subsessionIndex % 10, 'd', 'c');
        if (strcmp(fOurSubsession.codecName(), "JPEG") == 0) {
            fAVICodecHandlerType = fourChar('m', 'j', 'p', 'g');
        } else if (strcmp(fOurSubsession.codecName(), "MP4V-ES") == 0) {
            fAVICodecHandlerType = fourChar('D', 'I', 'V', 'X');
        } else if (strcmp(fOurSubsession.codecName(), "MPV") == 0) {
            fAVICodecHandlerType = fourChar('m', 'p', 'g', '1');
        } else if (strcmp(fOurSubsession.codecName(), "H263-1998") == 0 ||
                   strcmp(fOurSubsession.codecName(), "H263-2000") == 0)
        {
            fAVICodecHandlerType = fourChar('H', '2', '6', '3');
        } else if (strcmp(fOurSubsession.codecName(), "H264") == 0) {
            fAVICodecHandlerType = fourChar('H', '2', '6', '4');
        } else {
            fAVICodecHandlerType = fourChar('?', '?', '?', '?');
        }
        fAVIScale = 1;
        fAVIRate  = fOurSink.fMovieFPS;
        fAVISize  = fOurSink.fMovieWidth * fOurSink.fMovieHeight * 3;
    } else if (fIsAudio) {
        fIsByteSwappedAudio = False;
        fAVISubsessionTag   =
            fourChar('0' + subsessionIndex / 10, '0' + subsessionIndex % 10, 'w', 'b');
        fAVICodecHandlerType = 1;
        unsigned numChannels = fOurSubsession.numChannels();
        fAVISamplingFrequency = fOurSubsession.rtpTimestampFrequency();
        if (strcmp(fOurSubsession.codecName(), "L16") == 0) {
            fIsByteSwappedAudio = True;
            fWAVCodecTag        = 0x0001;
            fAVIScale = fAVISize = 2 * numChannels;
            fAVIRate  = fAVISize * fAVISamplingFrequency;
        } else if (strcmp(fOurSubsession.codecName(), "L8") == 0) {
            fWAVCodecTag = 0x0001;
            fAVIScale    = fAVISize = numChannels;
            fAVIRate     = fAVISize * fAVISamplingFrequency;
        } else if (strcmp(fOurSubsession.codecName(), "PCMA") == 0) {
            fWAVCodecTag = 0x0006;
            fAVIScale    = fAVISize = numChannels;
            fAVIRate     = fAVISize * fAVISamplingFrequency;
        } else if (strcmp(fOurSubsession.codecName(), "PCMU") == 0) {
            fWAVCodecTag = 0x0007;
            fAVIScale    = fAVISize = numChannels;
            fAVIRate     = fAVISize * fAVISamplingFrequency;
        } else if (strcmp(fOurSubsession.codecName(), "MPA") == 0) {
            fWAVCodecTag = 0x0050;
            fAVIScale    = fAVISize = 1;
            fAVIRate     = 0;
        } else {
            fWAVCodecTag = 0x0001;
            fAVIScale    = fAVISize = 1;
            fAVIRate     = 0;
        }
    } else {
        fAVISubsessionTag =
            fourChar('0' + subsessionIndex / 10, '0' + subsessionIndex % 10, '?', '?');
        fAVICodecHandlerType = 0;
        fAVIScale = fAVISize = 1;
        fAVIRate  = 0;
    }
} // AVISubsessionIOState::setAVIstate

void AVISubsessionIOState::afterGettingFrame(unsigned       packetDataSize,
                                             struct timeval presentationTime)
{
    unsigned short rtpSeqNum =
        fOurSubsession.rtpSource()->curPacketRTPSeqNum();

    if (fOurSink.fPacketLossCompensate && fPrevBuffer->bytesInUse() > 0) {
        short seqNumGap = rtpSeqNum - fLastPacketRTPSeqNum;
        for (short i = 1; i < seqNumGap; ++i) {
            useFrame(*fPrevBuffer);
        }
    }
    fLastPacketRTPSeqNum = rtpSeqNum;
    if (fBuffer->bytesInUse() == 0) {
        fBuffer->setPresentationTime(presentationTime);
    }
    fBuffer->addBytes(packetDataSize);
    useFrame(*fBuffer);
    if (fOurSink.fPacketLossCompensate) {
        SubsessionBuffer* tmp = fPrevBuffer;
        fPrevBuffer = fBuffer;
        fBuffer     = tmp;
    }
    fBuffer->reset();
    fOurSink.continuePlaying();
}

void AVISubsessionIOState::useFrame(SubsessionBuffer& buffer)
{
    unsigned char * const frameSource = buffer.dataStart();
    unsigned const frameSize = buffer.bytesInUse();
    struct timeval const& presentationTime = buffer.presentationTime();

    if (fPrevPresentationTime.tv_usec != 0 || fPrevPresentationTime.tv_sec != 0) {
        int uSecondsDiff =
            (presentationTime.tv_sec - fPrevPresentationTime.tv_sec) * 1000000
            + (presentationTime.tv_usec - fPrevPresentationTime.tv_usec);
        if (uSecondsDiff > 0) {
            unsigned bytesPerSecond = (unsigned) ((frameSize * 1000000.0) / uSecondsDiff);
            if (bytesPerSecond > fMaxBytesPerSecond) {
                fMaxBytesPerSecond = bytesPerSecond;
            }
        }
    }
    fPrevPresentationTime = presentationTime;
    if (fIsByteSwappedAudio) {
        for (unsigned i = 0; i < frameSize; i += 2) {
            unsigned char tmp = frameSource[i];
            frameSource[i]     = frameSource[i + 1];
            frameSource[i + 1] = tmp;
        }
    }
    AVIIndexRecord* newIndexRecord =
        new AVIIndexRecord(fAVISubsessionTag,
                           frameSource[0] == 0x67 ? 0x10 : 0,
                           fOurSink.fMoviSizePosition + 8 + fOurSink.fNumBytesWritten,
                           frameSize + 4);
    fOurSink.addIndexRecord(newIndexRecord);
    fOurSink.fNumBytesWritten += fOurSink.addWord(fAVISubsessionTag);
    if (strcmp(fOurSubsession.codecName(), "H264") == 0) {
        fOurSink.fNumBytesWritten += fOurSink.addWord(4 + frameSize);
        fOurSink.fNumBytesWritten += fOurSink.addWord(fourChar(0x00, 0x00, 0x00, 0x01));
    } else {
        fOurSink.fNumBytesWritten += fOurSink.addWord(frameSize);
    }
    fwrite(frameSource, 1, frameSize, fOurSink.fOutFid);
    fOurSink.fNumBytesWritten += frameSize;
    if (frameSize % 2 != 0) {
        fOurSink.fNumBytesWritten += fOurSink.addByte(0);
    }
    ++fNumFrames;
} // AVISubsessionIOState::useFrame

void AVISubsessionIOState::onSourceClosure()
{
    fOurSourceIsActive = False;
    fOurSink.onSourceClosure1();
}

unsigned AVIFileSink::addWord(unsigned word)
{
    addByte(word);
    addByte(word >> 8);
    addByte(word >> 16);
    addByte(word >> 24);
    return 4;
}

unsigned AVIFileSink::addHalfWord(unsigned short halfWord)
{
    addByte((unsigned char) halfWord);
    addByte((unsigned char) (halfWord >> 8));
    return 2;
}

unsigned AVIFileSink::addZeroWords(unsigned numWords)
{
    for (unsigned i = 0; i < numWords; ++i) {
        addWord(0);
    }
    return numWords * 4;
}

unsigned AVIFileSink::add4ByteString(char const* str)
{
    addByte(str[0]);
    addByte(str[1]);
    addByte(str[2]);
    addByte(str[3] == '\0' ? ' ' : str[3]);
    return 4;
}

void AVIFileSink::setWord(unsigned filePosn, unsigned size)
{
    do {
        if (SeekFile64(fOutFid, filePosn, SEEK_SET) < 0) {
            break;
        }
        addWord(size);
        if (SeekFile64(fOutFid, 0, SEEK_END) < 0) {
            break;
        }
        return;
    } while (0);
    envir() << "AVIFileSink::setWord(): SeekFile64 failed (err "
            << envir().getErrno() << ")\n";
}

#define addFileHeader(tag, name) \
    unsigned AVIFileSink::addFileHeader_ ## name() { \
        add4ByteString("" #tag ""); \
        unsigned headerSizePosn = (unsigned) TellFile64(fOutFid); addWord(0); \
        add4ByteString("" #name ""); \
        unsigned ignoredSize = 8; \
        unsigned size        = 12
#define addFileHeader1(name) \
    unsigned AVIFileSink::addFileHeader_ ## name() { \
        add4ByteString("" #name ""); \
        unsigned headerSizePosn = (unsigned) TellFile64(fOutFid); addWord(0); \
        unsigned ignoredSize    = 8; \
        unsigned size = 8
#define addFileHeaderEnd \
    setWord(headerSizePosn, size-ignoredSize); \
    return size; \
}
addFileHeader(RIFF, AVI);
size += addFileHeader_hdrl();
size += addFileHeader_movi();
fRIFFSizePosition = headerSizePosn;
fRIFFSizeValue    = size - ignoredSize;
addFileHeaderEnd;
addFileHeader(LIST, hdrl);
size += addFileHeader_avih();
unsigned subsessionCount = 0;
MediaSubsessionIterator iter(fInputSession);
MediaSubsession* subsession;
while ((subsession = iter.next()) != NULL) {
    fCurrentIOState = (AVISubsessionIOState *) (subsession->miscPtr);
    if (fCurrentIOState == NULL) {
        continue;
    }
    if (strcmp(subsession->mediumName(), "video") != 0) {
        continue;
    }
    fCurrentIOState->setAVIstate(subsessionCount++);
    size += addFileHeader_strl();
}
iter.reset();
while ((subsession = iter.next()) != NULL) {
    fCurrentIOState = (AVISubsessionIOState *) (subsession->miscPtr);
    if (fCurrentIOState == NULL) {
        continue;
    }
    if (strcmp(subsession->mediumName(), "video") == 0) {
        continue;
    }
    fCurrentIOState->setAVIstate(subsessionCount++);
    size += addFileHeader_strl();
}
+ +fJunkNumber;
size += addFileHeader_JUNK();
addFileHeaderEnd;
#define AVIF_HASINDEX       0x00000010
#define AVIF_MUSTUSEINDEX   0x00000020
#define AVIF_ISINTERLEAVED  0x00000100
#define AVIF_TRUSTCKTYPE    0x00000800
#define AVIF_WASCAPTUREFILE 0x00010000
#define AVIF_COPYRIGHTED    0x00020000
addFileHeader1(avih);
unsigned usecPerFrame = fMovieFPS == 0 ? 0 : 1000000 / fMovieFPS;
size += addWord(usecPerFrame);
fAVIHMaxBytesPerSecondPosition = (unsigned) TellFile64(fOutFid);
size += addWord(0);
size += addWord(0);
size += addWord(AVIF_TRUSTCKTYPE | AVIF_HASINDEX | AVIF_ISINTERLEAVED);
fAVIHFrameCountPosition = (unsigned) TellFile64(fOutFid);
size += addWord(0);
size += addWord(0);
size += addWord(fNumSubsessions);
size += addWord(fBufferSize);
size += addWord(fMovieWidth);
size += addWord(fMovieHeight);
size += addZeroWords(4);
addFileHeaderEnd;
addFileHeader(LIST, strl);
size       += addFileHeader_strh();
size       += addFileHeader_strf();
fJunkNumber = 0;
size       += addFileHeader_JUNK();
addFileHeaderEnd;
addFileHeader1(strh);
size += add4ByteString(fCurrentIOState->fIsVideo ? "vids" :
                       fCurrentIOState->fIsAudio ? "auds" :
                       "????");
size += addWord(fCurrentIOState->fAVICodecHandlerType);
size += addWord(0);
size += addWord(0);
size += addWord(0);
size += addWord(fCurrentIOState->fAVIScale);
size += addWord(fCurrentIOState->fAVIRate);
size += addWord(0);
fCurrentIOState->fSTRHFrameCountPosition = (unsigned) TellFile64(fOutFid);
size += addWord(0);
size += addWord(fBufferSize);
size += addWord((unsigned) -1);
size += addWord(fCurrentIOState->fAVISize);
size += addWord(0);
if (fCurrentIOState->fIsVideo) {
    size += addHalfWord(fMovieWidth);
    size += addHalfWord(fMovieHeight);
} else {
    size += addWord(0);
}
addFileHeaderEnd;
addFileHeader1(strf);
if (fCurrentIOState->fIsVideo) {
    unsigned extraDataSize = 0;
    size += addWord(10 * 4 + extraDataSize);
    size += addWord(fMovieWidth);
    size += addWord(fMovieHeight);
    size += addHalfWord(1);
    size += addHalfWord(24);
    size += addWord(fCurrentIOState->fAVICodecHandlerType);
    size += addWord(fCurrentIOState->fAVISize);
    size += addZeroWords(4);
} else if (fCurrentIOState->fIsAudio) {
    size += addHalfWord(fCurrentIOState->fWAVCodecTag);
    unsigned numChannels = fCurrentIOState->fOurSubsession.numChannels();
    size += addHalfWord(numChannels);
    size += addWord(fCurrentIOState->fAVISamplingFrequency);
    size += addWord(fCurrentIOState->fAVIRate);
    size += addHalfWord(fCurrentIOState->fAVISize);
    unsigned bitsPerSample = (fCurrentIOState->fAVISize * 8) / numChannels;
    size += addHalfWord(bitsPerSample);
    if (strcmp(fCurrentIOState->fOurSubsession.codecName(), "MPA") == 0) {
        size += addHalfWord(22);
        size += addHalfWord(2);
        size += addWord(8 * fCurrentIOState->fAVIRate);
        size += addHalfWord(numChannels == 2 ? 1 : 8);
        size += addHalfWord(0);
        size += addHalfWord(1);
        size += addHalfWord(16);
        size += addWord(0);
        size += addWord(0);
    }
}
addFileHeaderEnd;
#define AVI_MASTER_INDEX_SIZE 256
addFileHeader1(JUNK);
if (fJunkNumber == 0) {
    size += addHalfWord(4);
    size += addHalfWord(0);
    size += addWord(0);
    size += addWord(fCurrentIOState->fAVISubsessionTag);
    size += addZeroWords(2);
    size += addZeroWords(AVI_MASTER_INDEX_SIZE * 4);
} else {
    size += add4ByteString("odml");
    size += add4ByteString("dmlh");
    unsigned wtfCount = 248;
    size += addWord(wtfCount);
    size += addZeroWords(wtfCount / 4);
}
addFileHeaderEnd;
addFileHeader(LIST, movi);
fMoviSizePosition = headerSizePosn;
fMoviSizeValue    = size - ignoredSize;
addFileHeaderEnd;
