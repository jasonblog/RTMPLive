#include "QuickTimeFileSink.hh"
#include "QuickTimeGenericRTPSource.hh"
#include "GroupsockHelper.hh"
#include "InputFile.hh"
#include "OutputFile.hh"
#include "H263plusVideoRTPSource.hh"
#include "MPEG4GenericRTPSource.hh"
#include "MPEG4LATMAudioRTPSource.hh"
#include "Base64.hh"
#include <ctype.h>
#define fourChar(x, y, z, w) ( ((x)<<24)|((y)<<16)|((z)<<8)|(w) )
#define H264_IDR_FRAME 0x65
class ChunkDescriptor
{
public:
    ChunkDescriptor(int64_t offsetInFile, unsigned size,
                    unsigned frameSize, unsigned frameDuration,
                    struct timeval presentationTime);
    ChunkDescriptor * extendChunk(int64_t newOffsetInFile, unsigned newSize,
                                  unsigned newFrameSize,
                                  unsigned newFrameDuration,
                                  struct timeval newPresentationTime);
public:
    ChunkDescriptor* fNextChunk;
    int64_t fOffsetInFile;
    unsigned fNumFrames;
    unsigned fFrameSize;
    unsigned fFrameDuration;
    struct timeval fPresentationTime;
};
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
class SyncFrame
{
public:
    SyncFrame(unsigned frameNum);
public:
    class SyncFrame* nextSyncFrame;
    unsigned sfFrameNum;
};
class Count64
{
public:
    Count64()
        : hi(0), lo(0)
    {}

    void operator += (unsigned arg);
    u_int32_t hi, lo;
};
class SubsessionIOState
{
public:
    SubsessionIOState(QuickTimeFileSink& sink, MediaSubsession& subsession);
    virtual ~SubsessionIOState();
    Boolean setQTstate();
    void setFinalQTstate();
    void afterGettingFrame(unsigned       packetDataSize,
                           struct timeval presentationTime);
    void onSourceClosure();
    Boolean syncOK(struct timeval presentationTime);
    static void setHintTrack(SubsessionIOState* hintedTrack,
                             SubsessionIOState* hintTrack);
    Boolean isHintTrack() const
    {
        return fTrackHintedByUs != NULL;
    }

    Boolean hasHintTrack() const
    {
        return fHintTrackForUs != NULL;
    }

    UsageEnvironment& envir() const
    {
        return fOurSink.envir();
    }

public:
    static unsigned fCurrentTrackNumber;
    unsigned fTrackID;
    SubsessionIOState* fHintTrackForUs;
    SubsessionIOState* fTrackHintedByUs;
    SubsessionBuffer* fBuffer, * fPrevBuffer;
    QuickTimeFileSink& fOurSink;
    MediaSubsession& fOurSubsession;
    unsigned short fLastPacketRTPSeqNum;
    Boolean fOurSourceIsActive;
    Boolean fHaveBeenSynced;
    struct timeval fSyncTime;
    Boolean fQTEnableTrack;
    unsigned fQTcomponentSubtype;
    char const* fQTcomponentName;
    typedef unsigned (QuickTimeFileSink::* atomCreationFunc)();
    atomCreationFunc fQTMediaInformationAtomCreator;
    atomCreationFunc fQTMediaDataAtomCreator;
    char const* fQTAudioDataType;
    unsigned short fQTSoundSampleVersion;
    unsigned fQTTimeScale;
    unsigned fQTTimeUnitsPerSample;
    unsigned fQTBytesPerFrame;
    unsigned fQTSamplesPerFrame;
    unsigned fQTTotNumSamples;
    unsigned fQTDurationM;
    unsigned fQTDurationT;
    int64_t fTKHD_durationPosn;
    unsigned fQTInitialOffsetDuration;
    ChunkDescriptor* fHeadChunk, * fTailChunk;
    unsigned fNumChunks;
    SyncFrame* fHeadSyncFrame, * fTailSyncFrame;
    struct hinf {
        Count64  trpy;
        Count64  nump;
        Count64  tpyl;
        Count64  dmed;
        Count64  dimm;
        unsigned pmax;
        unsigned dmax;
    } fHINF;
private:
    void useFrame(SubsessionBuffer& buffer);
    void useFrameForHinting(unsigned       frameSize,
                            struct timeval presentationTime,
                            unsigned       startSampleNumber);
    unsigned useFrame1(unsigned sourceDataSize,
                       struct timeval presentationTime,
                       unsigned frameDuration, int64_t destFileOffset);
private:
    struct {
        unsigned       frameSize;
        struct timeval presentationTime;
        int64_t        destFileOffset;
        unsigned       startSampleNumber;
        unsigned short seqNum;
        unsigned       rtpHeader;
        unsigned char  numSpecialHeaders;
        unsigned       specialHeaderBytesLength;
        unsigned char  specialHeaderBytes[SPECIAL_HEADER_BUFFER_SIZE];
        unsigned       packetSizes[256];
    } fPrevFrameState;
};
QuickTimeFileSink::QuickTimeFileSink(UsageEnvironment& env,
                                     MediaSession    & inputSession,
                                     char const*     outputFileName,
                                     unsigned        bufferSize,
                                     unsigned short  movieWidth,
                                     unsigned short  movieHeight,
                                     unsigned        movieFPS,
                                     Boolean         packetLossCompensate,
                                     Boolean         syncStreams,
                                     Boolean         generateHintTracks,
                                     Boolean         generateMP4Format)
    : Medium(env), fInputSession(inputSession),
    fBufferSize(bufferSize), fPacketLossCompensate(packetLossCompensate),
    fSyncStreams(syncStreams), fGenerateMP4Format(generateMP4Format),
    fAreCurrentlyBeingPlayed(False),
    fLargestRTPtimestampFrequency(0),
    fNumSubsessions(0), fNumSyncedSubsessions(0),
    fHaveCompletedOutputFile(False),
    fMovieWidth(movieWidth), fMovieHeight(movieHeight),
    fMovieFPS(movieFPS), fMaxTrackDurationM(0)
{
    fOutFid = OpenOutputFile(env, outputFileName);
    if (fOutFid == NULL) {
        return;
    }
    fNewestSyncTime.tv_sec = fNewestSyncTime.tv_usec = 0;
    fFirstDataTime.tv_sec  = fFirstDataTime.tv_usec = (unsigned) (~0);
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
        SubsessionIOState* ioState =
            new SubsessionIOState(*this, *subsession);
        if (ioState == NULL || !ioState->setQTstate()) {
            delete ioState;
            ioState = NULL;
            continue;
        }
        subsession->miscPtr = (void *) ioState;
        if (generateHintTracks) {
            SubsessionIOState* hintTrack =
                new SubsessionIOState(*this, *subsession);
            SubsessionIOState::setHintTrack(ioState, hintTrack);
            if (!hintTrack->setQTstate()) {
                delete hintTrack;
                SubsessionIOState::setHintTrack(ioState, NULL);
            }
        }
        if (subsession->rtcpInstance() != NULL) {
            subsession->rtcpInstance()->setByeHandler(onRTCPBye, ioState);
        }
        unsigned rtpTimestampFrequency = subsession->rtpTimestampFrequency();
        if (rtpTimestampFrequency > fLargestRTPtimestampFrequency) {
            fLargestRTPtimestampFrequency = rtpTimestampFrequency;
        }
        ++fNumSubsessions;
    }
    gettimeofday(&fStartTime, NULL);
    fAppleCreationTime = fStartTime.tv_sec - 0x83dac000;
    fMDATposition      = TellFile64(fOutFid);
    addAtomHeader64("mdat");
    fMDATposition += 8;
}

QuickTimeFileSink::~QuickTimeFileSink()
{
    completeOutputFile();
    MediaSubsessionIterator iter(fInputSession);
    MediaSubsession* subsession;
    while ((subsession = iter.next()) != NULL) {
        if (subsession->readSource() != NULL) {
            subsession->readSource()->stopGettingFrames();
        }
        SubsessionIOState* ioState =
            (SubsessionIOState *) (subsession->miscPtr);
        if (ioState == NULL) {
            continue;
        }
        delete ioState->fHintTrackForUs;
        delete ioState;
    }
    CloseOutputFile(fOutFid);
}

QuickTimeFileSink * QuickTimeFileSink::createNew(UsageEnvironment& env,
                                                 MediaSession    & inputSession,
                                                 char const*     outputFileName,
                                                 unsigned        bufferSize,
                                                 unsigned short  movieWidth,
                                                 unsigned short  movieHeight,
                                                 unsigned        movieFPS,
                                                 Boolean         packetLossCompensate,
                                                 Boolean         syncStreams,
                                                 Boolean         generateHintTracks,
                                                 Boolean         generateMP4Format)
{
    QuickTimeFileSink* newSink =
        new QuickTimeFileSink(env, inputSession, outputFileName, bufferSize, movieWidth, movieHeight, movieFPS,
                              packetLossCompensate, syncStreams, generateHintTracks, generateMP4Format);

    if (newSink == NULL || newSink->fOutFid == NULL) {
        Medium::close(newSink);
        return NULL;
    }
    return newSink;
}

Boolean QuickTimeFileSink::startPlaying(afterPlayingFunc* afterFunc,
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

Boolean QuickTimeFileSink::continuePlaying()
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
        SubsessionIOState* ioState =
            (SubsessionIOState *) (subsession->miscPtr);
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

void QuickTimeFileSink
::afterGettingFrame(void* clientData, unsigned packetDataSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned)
{
    SubsessionIOState* ioState = (SubsessionIOState *) clientData;

    if (!ioState->syncOK(presentationTime)) {
        ioState->fOurSink.continuePlaying();
        return;
    }
    if (numTruncatedBytes > 0) {
        ioState->envir()
            <<
            "QuickTimeFileSink::afterGettingFrame(): The input frame data was too large for our buffer.  "
            << numTruncatedBytes
            <<
            " bytes of trailing data was dropped!  Correct this by increasing the \"bufferSize\" parameter in the \"createNew()\" call.\n";
    }
    ioState->afterGettingFrame(packetDataSize, presentationTime);
}

void QuickTimeFileSink::onSourceClosure(void* clientData)
{
    SubsessionIOState* ioState = (SubsessionIOState *) clientData;

    ioState->onSourceClosure();
}

void QuickTimeFileSink::onSourceClosure1()
{
    MediaSubsessionIterator iter(fInputSession);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
        SubsessionIOState* ioState =
            (SubsessionIOState *) (subsession->miscPtr);
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

void QuickTimeFileSink::onRTCPBye(void* clientData)
{
    SubsessionIOState* ioState = (SubsessionIOState *) clientData;
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

static Boolean timevalGE(struct timeval const& tv1,
                         struct timeval const& tv2)
{
    return (unsigned) tv1.tv_sec > (unsigned) tv2.tv_sec ||
           (tv1.tv_sec == tv2.tv_sec &&
            (unsigned) tv1.tv_usec >= (unsigned) tv2.tv_usec);
}

void QuickTimeFileSink::completeOutputFile()
{
    if (fHaveCompletedOutputFile || fOutFid == NULL) {
        return;
    }
    int64_t curFileSize = TellFile64(fOutFid);
    setWord64(fMDATposition, (u_int64_t) curFileSize);
    MediaSubsessionIterator iter(fInputSession);
    MediaSubsession* subsession;
    while ((subsession = iter.next()) != NULL) {
        SubsessionIOState* ioState =
            (SubsessionIOState *) (subsession->miscPtr);
        if (ioState == NULL) {
            continue;
        }
        ChunkDescriptor * const headChunk = ioState->fHeadChunk;
        if (headChunk != NULL &&
            timevalGE(fFirstDataTime, headChunk->fPresentationTime))
        {
            fFirstDataTime = headChunk->fPresentationTime;
        }
    }
    iter.reset();
    while ((subsession = iter.next()) != NULL) {
        SubsessionIOState* ioState =
            (SubsessionIOState *) (subsession->miscPtr);
        if (ioState == NULL) {
            continue;
        }
        ioState->setFinalQTstate();
        if (ioState->hasHintTrack()) {
            ioState->fHintTrackForUs->setFinalQTstate();
        }
    }
    if (fGenerateMP4Format) {
        addAtom_ftyp();
    }
    addAtom_moov();
    fHaveCompletedOutputFile = True;
} // QuickTimeFileSink::completeOutputFile

unsigned SubsessionIOState::fCurrentTrackNumber = 0;
SubsessionIOState::SubsessionIOState(QuickTimeFileSink& sink,
                                     MediaSubsession  & subsession)
    : fHintTrackForUs(NULL), fTrackHintedByUs(NULL),
    fOurSink(sink), fOurSubsession(subsession),
    fLastPacketRTPSeqNum(0), fHaveBeenSynced(False), fQTTotNumSamples(0),
    fHeadChunk(NULL), fTailChunk(NULL), fNumChunks(0),
    fHeadSyncFrame(NULL), fTailSyncFrame(NULL)
{
    fTrackID    = ++fCurrentTrackNumber;
    fBuffer     = new SubsessionBuffer(fOurSink.fBufferSize);
    fPrevBuffer = sink.fPacketLossCompensate ?
                  new SubsessionBuffer(fOurSink.fBufferSize) : NULL;
    FramedSource* subsessionSource = subsession.readSource();
    fOurSourceIsActive = subsessionSource != NULL;
    fPrevFrameState.presentationTime.tv_sec  = 0;
    fPrevFrameState.presentationTime.tv_usec = 0;
    fPrevFrameState.seqNum = 0;
}

SubsessionIOState::~SubsessionIOState()
{
    delete fBuffer;
    delete fPrevBuffer;
    ChunkDescriptor* chunk = fHeadChunk;
    while (chunk != NULL) {
        ChunkDescriptor* next = chunk->fNextChunk;
        delete chunk;
        chunk = next;
    }
    SyncFrame* syncFrame = fHeadSyncFrame;
    while (syncFrame != NULL) {
        SyncFrame* next = syncFrame->nextSyncFrame;
        delete syncFrame;
        syncFrame = next;
    }
}

Boolean SubsessionIOState::setQTstate()
{
    char const* noCodecWarning1 = "Warning: We don't implement a QuickTime ";
    char const* noCodecWarning2 = " Media Data Type for the \"";
    char const* noCodecWarning3 =
        "\" track, so we'll insert a dummy \"????\" Media Data Atom instead.  A separate, codec-specific editing pass will be needed before this track can be played.\n";

    do {
        fQTEnableTrack        = True;
        fQTTimeScale          = fOurSubsession.rtpTimestampFrequency();
        fQTTimeUnitsPerSample = 1;
        fQTBytesPerFrame      = 0;
        fQTSamplesPerFrame    = 1;
        if (isHintTrack()) {
            fQTEnableTrack      = False;
            fQTcomponentSubtype = fourChar('h', 'i', 'n', 't');
            fQTcomponentName    = "hint media handler";
            fQTMediaInformationAtomCreator = &QuickTimeFileSink::addAtom_gmhd;
            fQTMediaDataAtomCreator        = &QuickTimeFileSink::addAtom_rtp;
        } else if (strcmp(fOurSubsession.mediumName(), "audio") == 0) {
            fQTcomponentSubtype = fourChar('s', 'o', 'u', 'n');
            fQTcomponentName    = "Apple Sound Media Handler";
            fQTMediaInformationAtomCreator = &QuickTimeFileSink::addAtom_smhd;
            fQTMediaDataAtomCreator        =
                &QuickTimeFileSink::addAtom_soundMediaGeneral;
            fQTSoundSampleVersion = 0;
            if (strcmp(fOurSubsession.codecName(), "X-QT") == 0 ||
                strcmp(fOurSubsession.codecName(), "X-QUICKTIME") == 0)
            {
                fQTMediaDataAtomCreator = &QuickTimeFileSink::addAtom_genericMedia;
            } else if (strcmp(fOurSubsession.codecName(), "PCMU") == 0) {
                fQTAudioDataType = "ulaw";
                fQTBytesPerFrame = 1;
            } else if (strcmp(fOurSubsession.codecName(), "GSM") == 0) {
                fQTAudioDataType   = "agsm";
                fQTBytesPerFrame   = 33;
                fQTSamplesPerFrame = 160;
            } else if (strcmp(fOurSubsession.codecName(), "PCMA") == 0) {
                fQTAudioDataType = "alaw";
                fQTBytesPerFrame = 1;
            } else if (strcmp(fOurSubsession.codecName(), "QCELP") == 0) {
                fQTMediaDataAtomCreator = &QuickTimeFileSink::addAtom_Qclp;
                fQTSamplesPerFrame      = 160;
            } else if (strcmp(fOurSubsession.codecName(), "MPEG4-GENERIC") == 0 ||
                       strcmp(fOurSubsession.codecName(), "MP4A-LATM") == 0)
            {
                fQTMediaDataAtomCreator = &QuickTimeFileSink::addAtom_mp4a;
                fQTTimeUnitsPerSample   = 1024;
                unsigned frequencyFromConfig =
                    samplingFrequencyFromAudioSpecificConfig(fOurSubsession.fmtp_config());
                if (frequencyFromConfig != 0) {
                    fQTTimeScale = frequencyFromConfig;
                }
            } else {
                envir() << noCodecWarning1 << "Audio" << noCodecWarning2
                        << fOurSubsession.codecName() << noCodecWarning3;
                fQTMediaDataAtomCreator = &QuickTimeFileSink::addAtom_dummy;
                fQTEnableTrack = False;
            }
        } else if (strcmp(fOurSubsession.mediumName(), "video") == 0) {
            fQTcomponentSubtype = fourChar('v', 'i', 'd', 'e');
            fQTcomponentName    = "Apple Video Media Handler";
            fQTMediaInformationAtomCreator = &QuickTimeFileSink::addAtom_vmhd;
            if (strcmp(fOurSubsession.codecName(), "X-QT") == 0 ||
                strcmp(fOurSubsession.codecName(), "X-QUICKTIME") == 0)
            {
                fQTMediaDataAtomCreator = &QuickTimeFileSink::addAtom_genericMedia;
            } else if (strcmp(fOurSubsession.codecName(), "H263-1998") == 0 ||
                       strcmp(fOurSubsession.codecName(), "H263-2000") == 0)
            {
                fQTMediaDataAtomCreator = &QuickTimeFileSink::addAtom_h263;
                fQTTimeScale = 600;
                fQTTimeUnitsPerSample = fQTTimeScale / fOurSink.fMovieFPS;
            } else if (strcmp(fOurSubsession.codecName(), "H264") == 0) {
                fQTMediaDataAtomCreator = &QuickTimeFileSink::addAtom_avc1;
                fQTTimeScale = 600;
                fQTTimeUnitsPerSample = fQTTimeScale / fOurSink.fMovieFPS;
            } else if (strcmp(fOurSubsession.codecName(), "MP4V-ES") == 0) {
                fQTMediaDataAtomCreator = &QuickTimeFileSink::addAtom_mp4v;
                fQTTimeScale = 600;
                fQTTimeUnitsPerSample = fQTTimeScale / fOurSink.fMovieFPS;
            } else {
                envir() << noCodecWarning1 << "Video" << noCodecWarning2
                        << fOurSubsession.codecName() << noCodecWarning3;
                fQTMediaDataAtomCreator = &QuickTimeFileSink::addAtom_dummy;
                fQTEnableTrack = False;
            }
        } else {
            envir() << "Warning: We don't implement a QuickTime Media Handler for media type \""
                    << fOurSubsession.mediumName() << "\"";
            break;
        }
        #ifdef QT_SUPPORT_PARTIALLY_ONLY
        envir()
            <<
            "Warning: We don't have sufficient codec-specific information (e.g., sample sizes) to fully generate the \""
            << fOurSubsession.mediumName() << "/" << fOurSubsession.codecName()
            <<
            "\" track, so we'll disable this track in the movie.  A separate, codec-specific editing pass will be needed before this track can be played\n";
        fQTEnableTrack = False;
        #endif
        return True;
    } while (0);
    envir() << ", so a track for the \"" << fOurSubsession.mediumName()
            << "/" << fOurSubsession.codecName()
            << "\" subsession will not be included in the output QuickTime file\n";
    return False;
} // SubsessionIOState::setQTstate

void SubsessionIOState::setFinalQTstate()
{
    fQTDurationT = 0;
    ChunkDescriptor* chunk = fHeadChunk;
    while (chunk != NULL) {
        unsigned const numFrames = chunk->fNumFrames;
        unsigned const dur       = numFrames * chunk->fFrameDuration;
        fQTDurationT += dur;
        chunk         = chunk->fNextChunk;
    }
    double scaleFactor = fOurSink.movieTimeScale() / (double) fQTTimeScale;
    fQTDurationM = (unsigned) (fQTDurationT * scaleFactor);
    if (fQTDurationM > fOurSink.fMaxTrackDurationM) {
        fOurSink.fMaxTrackDurationM = fQTDurationM;
    }
}

void SubsessionIOState::afterGettingFrame(unsigned       packetDataSize,
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
    if (fQTMediaDataAtomCreator == &QuickTimeFileSink::addAtom_genericMedia) {
        QuickTimeGenericRTPSource* rtpSource =
            (QuickTimeGenericRTPSource *) fOurSubsession.rtpSource();
        QuickTimeGenericRTPSource::QTState& qtState = rtpSource->qtState;
        fQTTimeScale = qtState.timescale;
        if (qtState.width != 0) {
            fOurSink.fMovieWidth = qtState.width;
        }
        if (qtState.height != 0) {
            fOurSink.fMovieHeight = qtState.height;
        }
        if (qtState.sdAtomSize >= 8) {
            char const* atom   = qtState.sdAtom;
            unsigned mediaType = fourChar(atom[4], atom[5], atom[6], atom[7]);
            switch (mediaType) {
                case fourChar('a', 'g', 's', 'm'): {
                    fQTBytesPerFrame   = 33;
                    fQTSamplesPerFrame = 160;
                    break;
                }
                case fourChar('Q', 'c', 'l', 'p'): {
                    fQTBytesPerFrame   = 35;
                    fQTSamplesPerFrame = 160;
                    break;
                }
                case fourChar('H', 'c', 'l', 'p'): {
                    fQTBytesPerFrame   = 17;
                    fQTSamplesPerFrame = 160;
                    break;
                }
                case fourChar('h', '2', '6', '3'): {
                    fQTTimeUnitsPerSample = fQTTimeScale / fOurSink.fMovieFPS;
                    break;
                }
            }
        }
    } else if (fQTMediaDataAtomCreator == &QuickTimeFileSink::addAtom_Qclp) {
        fQTBytesPerFrame = packetDataSize;
    }
    useFrame(*fBuffer);
    if (fOurSink.fPacketLossCompensate) {
        SubsessionBuffer* tmp = fPrevBuffer;
        fPrevBuffer = fBuffer;
        fBuffer     = tmp;
    }
    fBuffer->reset();
    fOurSink.continuePlaying();
} // SubsessionIOState::afterGettingFrame

void SubsessionIOState::useFrame(SubsessionBuffer& buffer)
{
    unsigned char * const frameSource = buffer.dataStart();
    unsigned const frameSize = buffer.bytesInUse();
    struct timeval const& presentationTime = buffer.presentationTime();
    int64_t const destFileOffset      = TellFile64(fOurSink.fOutFid);
    unsigned sampleNumberOfFrameStart = fQTTotNumSamples + 1;
    Boolean avcHack = fQTMediaDataAtomCreator == &QuickTimeFileSink::addAtom_avc1;

    if (!fOurSink.fSyncStreams ||
        fQTcomponentSubtype != fourChar('v', 'i', 'd', 'e'))
    {
        unsigned const frameDuration = fQTTimeUnitsPerSample * fQTSamplesPerFrame;
        unsigned frameSizeToUse      = frameSize;
        if (avcHack) {
            frameSizeToUse += 4;
        }
        fQTTotNumSamples += useFrame1(frameSizeToUse, presentationTime, frameDuration, destFileOffset);
    } else {
        struct timeval const& ppt = fPrevFrameState.presentationTime;
        if (ppt.tv_sec != 0 || ppt.tv_usec != 0) {
            double duration = (presentationTime.tv_sec - ppt.tv_sec)
                              + (presentationTime.tv_usec - ppt.tv_usec) / 1000000.0;
            if (duration < 0.0) {
                duration = 0.0;
            }
            unsigned frameDuration =
                (unsigned) ((2 * duration * fQTTimeScale + 1) / 2);
            unsigned frameSizeToUse = fPrevFrameState.frameSize;
            if (avcHack) {
                frameSizeToUse += 4;
            }
            unsigned numSamples =
                useFrame1(frameSizeToUse, ppt, frameDuration, fPrevFrameState.destFileOffset);
            fQTTotNumSamples        += numSamples;
            sampleNumberOfFrameStart = fQTTotNumSamples + 1;
        }
        if (avcHack && (*frameSource == H264_IDR_FRAME)) {
            SyncFrame* newSyncFrame = new SyncFrame(fQTTotNumSamples + 1);
            if (fTailSyncFrame == NULL) {
                fHeadSyncFrame = newSyncFrame;
            } else {
                fTailSyncFrame->nextSyncFrame = newSyncFrame;
            }
            fTailSyncFrame = newSyncFrame;
        }
        fPrevFrameState.frameSize        = frameSize;
        fPrevFrameState.presentationTime = presentationTime;
        fPrevFrameState.destFileOffset   = destFileOffset;
    }
    if (avcHack) {
        fOurSink.addWord(frameSize);
    }
    fwrite(frameSource, 1, frameSize, fOurSink.fOutFid);
    if (hasHintTrack()) {
        if (!fHaveBeenSynced) {
            fHaveBeenSynced =
                fOurSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP();
        }
        if (fHaveBeenSynced) {
            fHintTrackForUs->useFrameForHinting(frameSize, presentationTime,
                                                sampleNumberOfFrameStart);
        }
    }
} // SubsessionIOState::useFrame

void SubsessionIOState::useFrameForHinting(unsigned       frameSize,
                                           struct timeval presentationTime,
                                           unsigned       startSampleNumber)
{
    Boolean hack263         = strcmp(fOurSubsession.codecName(), "H263-1998") == 0;
    Boolean hackm4a_generic = strcmp(fOurSubsession.mediumName(), "audio") == 0 &&
                              strcmp(fOurSubsession.codecName(), "MPEG4-GENERIC") == 0;
    Boolean hackm4a_latm = strcmp(fOurSubsession.mediumName(), "audio") == 0 &&
                           strcmp(fOurSubsession.codecName(), "MP4A-LATM") == 0;
    Boolean hackm4a = hackm4a_generic || hackm4a_latm;
    Boolean haveSpecialHeaders = (hack263 || hackm4a_generic);
    RTPSource * const rs       = fOurSubsession.rtpSource();
    struct timeval const& ppt  = fPrevFrameState.presentationTime;

    if (ppt.tv_sec != 0 || ppt.tv_usec != 0) {
        double duration = (presentationTime.tv_sec - ppt.tv_sec)
                          + (presentationTime.tv_usec - ppt.tv_usec) / 1000000.0;
        if (duration < 0.0) {
            duration = 0.0;
        }
        unsigned msDuration = (unsigned) (duration * 1000);
        if (msDuration > fHINF.dmax) {
            fHINF.dmax = msDuration;
        }
        unsigned hintSampleDuration =
            (unsigned) ((2 * duration * fQTTimeScale + 1) / 2);
        if (hackm4a) {
            hintSampleDuration = fTrackHintedByUs->fQTTimeUnitsPerSample;
            if (fTrackHintedByUs->fQTTimeScale != fOurSubsession.rtpTimestampFrequency()) {
                unsigned const scalingFactor =
                    fOurSubsession.rtpTimestampFrequency() / fTrackHintedByUs->fQTTimeScale;
                hintSampleDuration *= scalingFactor;
            }
        }
        int64_t const hintSampleDestFileOffset = TellFile64(fOurSink.fOutFid);
        unsigned const maxPacketSize = 1450;
        unsigned short numPTEntries  =
            (fPrevFrameState.frameSize + (maxPacketSize - 1)) / maxPacketSize;
        unsigned char* immediateDataPtr      = NULL;
        unsigned immediateDataBytesRemaining = 0;
        if (haveSpecialHeaders) {
            numPTEntries     = fPrevFrameState.numSpecialHeaders;
            immediateDataPtr = fPrevFrameState.specialHeaderBytes;
            immediateDataBytesRemaining =
                fPrevFrameState.specialHeaderBytesLength;
        }
        unsigned hintSampleSize =
            fOurSink.addHalfWord(numPTEntries);
        hintSampleSize += fOurSink.addHalfWord(0x0000);
        unsigned offsetWithinSample = 0;
        for (unsigned i = 0; i < numPTEntries; ++i) {
            unsigned short numDTEntries = 1;
            unsigned short seqNum       = fPrevFrameState.seqNum++;
            unsigned rtpHeader = fPrevFrameState.rtpHeader;
            if (i + 1 < numPTEntries) {
                rtpHeader &= ~(1 << 23);
            }
            unsigned dataFrameSize = (i + 1 < numPTEntries) ?
                                     maxPacketSize : fPrevFrameState.frameSize - i * maxPacketSize;
            unsigned sampleNumber = fPrevFrameState.startSampleNumber;
            unsigned char immediateDataLen = 0;
            if (haveSpecialHeaders) {
                ++numDTEntries;
                if (immediateDataBytesRemaining > 0) {
                    if (hack263) {
                        immediateDataLen = *immediateDataPtr++;
                        --immediateDataBytesRemaining;
                        if (immediateDataLen > immediateDataBytesRemaining) {
                            immediateDataLen = immediateDataBytesRemaining;
                        }
                    } else {
                        immediateDataLen = fPrevFrameState.specialHeaderBytesLength;
                    }
                }
                dataFrameSize = fPrevFrameState.packetSizes[i] - immediateDataLen;
                if (hack263) {
                    Boolean PbitSet =
                        immediateDataLen >= 1 && (immediateDataPtr[0] & 0x4) != 0;
                    if (PbitSet) {
                        offsetWithinSample += 2;
                    }
                }
            }
            hintSampleSize += fOurSink.addWord(0);
            hintSampleSize += fOurSink.addWord(rtpHeader | seqNum);
            hintSampleSize += fOurSink.addHalfWord(0x0000);
            hintSampleSize += fOurSink.addHalfWord(numDTEntries);
            unsigned totalPacketSize = 0;
            if (haveSpecialHeaders) {
                hintSampleSize += fOurSink.addByte(1);
                unsigned char len = immediateDataLen > 14 ? 14 : immediateDataLen;
                hintSampleSize  += fOurSink.addByte(len);
                totalPacketSize += len;
                fHINF.dimm      += len;
                unsigned char j;
                for (j = 0; j < len; ++j) {
                    hintSampleSize += fOurSink.addByte(immediateDataPtr[j]);
                }
                for (j = len; j < 14; ++j) {
                    hintSampleSize += fOurSink.addByte(0);
                }
                immediateDataPtr += immediateDataLen;
                immediateDataBytesRemaining -= immediateDataLen;
            }
            hintSampleSize  += fOurSink.addByte(2);
            hintSampleSize  += fOurSink.addByte(0);
            hintSampleSize  += fOurSink.addHalfWord(dataFrameSize);
            totalPacketSize += dataFrameSize;
            fHINF.dmed      += dataFrameSize;
            hintSampleSize  += fOurSink.addWord(sampleNumber);
            hintSampleSize  += fOurSink.addWord(offsetWithinSample);
            unsigned short const bytesPerCompressionBlock =
                fTrackHintedByUs->fQTBytesPerFrame;
            unsigned short const samplesPerCompressionBlock =
                fTrackHintedByUs->fQTSamplesPerFrame;
            hintSampleSize     += fOurSink.addHalfWord(bytesPerCompressionBlock);
            hintSampleSize     += fOurSink.addHalfWord(samplesPerCompressionBlock);
            offsetWithinSample += dataFrameSize;
            fHINF.nump         += 1;
            fHINF.tpyl         += totalPacketSize;
            totalPacketSize    += 12;
            fHINF.trpy         += totalPacketSize;
            if (totalPacketSize > fHINF.pmax) {
                fHINF.pmax = totalPacketSize;
            }
        }
        fQTTotNumSamples += useFrame1(hintSampleSize, ppt, hintSampleDuration,
                                      hintSampleDestFileOffset);
    }
    fPrevFrameState.frameSize         = frameSize;
    fPrevFrameState.presentationTime  = presentationTime;
    fPrevFrameState.startSampleNumber = startSampleNumber;
    fPrevFrameState.rtpHeader         =
        rs->curPacketMarkerBit() << 23
            | (rs->rtpPayloadFormat() & 0x7F) << 16;
    if (hack263) {
        H263plusVideoRTPSource* rs_263 = (H263plusVideoRTPSource *) rs;
        fPrevFrameState.numSpecialHeaders        = rs_263->fNumSpecialHeaders;
        fPrevFrameState.specialHeaderBytesLength = rs_263->fSpecialHeaderBytesLength;
        unsigned i;
        for (i = 0; i < rs_263->fSpecialHeaderBytesLength; ++i) {
            fPrevFrameState.specialHeaderBytes[i] = rs_263->fSpecialHeaderBytes[i];
        }
        for (i = 0; i < rs_263->fNumSpecialHeaders; ++i) {
            fPrevFrameState.packetSizes[i] = rs_263->fPacketSizes[i];
        }
    } else if (hackm4a_generic) {
        unsigned const sizeLength  = fOurSubsession.attrVal_unsigned("sizelength");
        unsigned const indexLength = fOurSubsession.attrVal_unsigned("indexlength");
        if (sizeLength + indexLength != 16) {
            envir() << "Warning: unexpected 'sizeLength' " << sizeLength
                    << " and 'indexLength' " << indexLength
                    << "seen when creating hint track\n";
        }
        fPrevFrameState.numSpecialHeaders        = 1;
        fPrevFrameState.specialHeaderBytesLength = 4;
        fPrevFrameState.specialHeaderBytes[0]    = 0;
        fPrevFrameState.specialHeaderBytes[1]    = 16;
        fPrevFrameState.specialHeaderBytes[2]    = ((frameSize << indexLength) & 0xFF00) >> 8;
        fPrevFrameState.specialHeaderBytes[3]    = (frameSize << indexLength);
        fPrevFrameState.packetSizes[0] =
            fPrevFrameState.specialHeaderBytesLength + frameSize;
    }
} // SubsessionIOState::useFrameForHinting

unsigned SubsessionIOState::useFrame1(unsigned       sourceDataSize,
                                      struct timeval presentationTime,
                                      unsigned       frameDuration,
                                      int64_t        destFileOffset)
{
    unsigned frameSize = fQTBytesPerFrame;

    if (frameSize == 0) {
        frameSize = sourceDataSize;
    }
    unsigned const numFrames  = sourceDataSize / frameSize;
    unsigned const numSamples = numFrames * fQTSamplesPerFrame;
    ChunkDescriptor* newTailChunk;
    if (fTailChunk == NULL) {
        newTailChunk = fHeadChunk =
            new ChunkDescriptor(destFileOffset, sourceDataSize,
                                frameSize, frameDuration, presentationTime);
    } else {
        newTailChunk = fTailChunk->extendChunk(destFileOffset, sourceDataSize,
                                               frameSize, frameDuration,
                                               presentationTime);
    }
    if (newTailChunk != fTailChunk) {
        ++fNumChunks;
        fTailChunk = newTailChunk;
    }
    return numSamples;
}

void SubsessionIOState::onSourceClosure()
{
    fOurSourceIsActive = False;
    fOurSink.onSourceClosure1();
}

Boolean SubsessionIOState::syncOK(struct timeval presentationTime)
{
    QuickTimeFileSink& s = fOurSink;

    if (!s.fSyncStreams) {
        return True;
    }
    if (s.fNumSyncedSubsessions < s.fNumSubsessions) {
        if (!fHaveBeenSynced) {
            if (fOurSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
                if (fQTMediaDataAtomCreator == &QuickTimeFileSink::addAtom_avc1) {
                    if ((s.fNumSubsessions == 2) && (s.fNumSyncedSubsessions < (s.fNumSubsessions - 1))) {
                        return False;
                    }
                    unsigned char * const frameSource = fBuffer->dataStart();
                    if (*frameSource != H264_IDR_FRAME) {
                        return False;
                    }
                }
                fHaveBeenSynced = True;
                fSyncTime       = presentationTime;
                ++s.fNumSyncedSubsessions;
                if (timevalGE(fSyncTime, s.fNewestSyncTime)) {
                    s.fNewestSyncTime = fSyncTime;
                }
            }
        }
    }
    if (s.fNumSyncedSubsessions < s.fNumSubsessions) {
        return False;
    }
    return timevalGE(presentationTime, s.fNewestSyncTime);
}

void SubsessionIOState::setHintTrack(SubsessionIOState* hintedTrack,
                                     SubsessionIOState* hintTrack)
{
    if (hintedTrack != NULL) {
        hintedTrack->fHintTrackForUs = hintTrack;
    }
    if (hintTrack != NULL) {
        hintTrack->fTrackHintedByUs = hintedTrack;
    }
}

SyncFrame::SyncFrame(unsigned frameNum)
    : nextSyncFrame(NULL), sfFrameNum(frameNum)
{}

void Count64::operator += (unsigned arg)
{
    unsigned newLo = lo + arg;

    if (newLo < lo) {
        ++hi;
    }
    lo = newLo;
}

ChunkDescriptor
::ChunkDescriptor(int64_t offsetInFile, unsigned size,
                  unsigned frameSize, unsigned frameDuration,
                  struct timeval presentationTime)
    : fNextChunk(NULL), fOffsetInFile(offsetInFile),
    fNumFrames(size / frameSize),
    fFrameSize(frameSize), fFrameDuration(frameDuration),
    fPresentationTime(presentationTime)
{}

ChunkDescriptor * ChunkDescriptor
::extendChunk(int64_t newOffsetInFile, unsigned newSize,
              unsigned newFrameSize, unsigned newFrameDuration,
              struct timeval newPresentationTime)
{
    if (newOffsetInFile == fOffsetInFile + fNumFrames * fFrameSize) {
        if (newFrameSize == fFrameSize && newFrameDuration == fFrameDuration) {
            fNumFrames += newSize / fFrameSize;
            return this;
        }
    }
    ChunkDescriptor* newDescriptor =
        new ChunkDescriptor(newOffsetInFile, newSize,
                            newFrameSize, newFrameDuration,
                            newPresentationTime);
    fNextChunk = newDescriptor;
    return newDescriptor;
}

unsigned QuickTimeFileSink::addWord64(u_int64_t word)
{
    addByte((unsigned char) (word >> 56));
    addByte((unsigned char) (word >> 48));
    addByte((unsigned char) (word >> 40));
    addByte((unsigned char) (word >> 32));
    addByte((unsigned char) (word >> 24));
    addByte((unsigned char) (word >> 16));
    addByte((unsigned char) (word >> 8));
    addByte((unsigned char) (word));
    return 8;
}

unsigned QuickTimeFileSink::addWord(unsigned word)
{
    addByte(word >> 24);
    addByte(word >> 16);
    addByte(word >> 8);
    addByte(word);
    return 4;
}

unsigned QuickTimeFileSink::addHalfWord(unsigned short halfWord)
{
    addByte((unsigned char) (halfWord >> 8));
    addByte((unsigned char) halfWord);
    return 2;
}

unsigned QuickTimeFileSink::addZeroWords(unsigned numWords)
{
    for (unsigned i = 0; i < numWords; ++i) {
        addWord(0);
    }
    return numWords * 4;
}

unsigned QuickTimeFileSink::add4ByteString(char const* str)
{
    addByte(str[0]);
    addByte(str[1]);
    addByte(str[2]);
    addByte(str[3]);
    return 4;
}

unsigned QuickTimeFileSink::addArbitraryString(char const* str,
                                               Boolean     oneByteLength)
{
    unsigned size = 0;

    if (oneByteLength) {
        unsigned strLength = strlen(str);
        if (strLength >= 256) {
            envir() << "QuickTimeFileSink::addArbitraryString(\""
                    << str << "\") saw string longer than we know how to handle ("
                    << strLength << "\n";
        }
        size += addByte((unsigned char) strLength);
    }
    while (*str != '\0') {
        size += addByte(*str++);
    }
    return size;
}

unsigned QuickTimeFileSink::addAtomHeader(char const* atomName)
{
    addWord(0);
    add4ByteString(atomName);
    return 8;
}

unsigned QuickTimeFileSink::addAtomHeader64(char const* atomName)
{
    addWord(1);
    add4ByteString(atomName);
    addWord64(0);
    return 16;
}

void QuickTimeFileSink::setWord(int64_t filePosn, unsigned size)
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
    envir() << "QuickTimeFileSink::setWord(): SeekFile64 failed (err "
            << envir().getErrno() << ")\n";
}

void QuickTimeFileSink::setWord64(int64_t filePosn, u_int64_t size)
{
    do {
        if (SeekFile64(fOutFid, filePosn, SEEK_SET) < 0) {
            break;
        }
        addWord64(size);
        if (SeekFile64(fOutFid, 0, SEEK_END) < 0) {
            break;
        }
        return;
    } while (0);
    envir() << "QuickTimeFileSink::setWord64(): SeekFile64 failed (err "
            << envir().getErrno() << ")\n";
}

#define addAtom(name) \
    unsigned QuickTimeFileSink::addAtom_ ## name() { \
        int64_t initFilePosn = TellFile64(fOutFid); \
        unsigned size        = addAtomHeader("" #name "")
#define addAtomEnd \
    setWord(initFilePosn, size); \
    return size; \
}
addAtom(ftyp);
size += add4ByteString("mp42");
size += addWord(0x00000000);
size += add4ByteString("mp42");
size += add4ByteString("isom");
addAtomEnd;
addAtom(moov);
size += addAtom_mvhd();
if (fGenerateMP4Format) {
    size += addAtom_iods();
}
MediaSubsessionIterator iter(fInputSession);
MediaSubsession* subsession;
while ((subsession = iter.next()) != NULL) {
    fCurrentIOState = (SubsessionIOState *) (subsession->miscPtr);
    if (fCurrentIOState == NULL) {
        continue;
    }
    if (strcmp(subsession->mediumName(), "audio") != 0) {
        continue;
    }
    size += addAtom_trak();
    if (fCurrentIOState->hasHintTrack()) {
        fCurrentIOState = fCurrentIOState->fHintTrackForUs;
        size += addAtom_trak();
    }
}
iter.reset();
while ((subsession = iter.next()) != NULL) {
    fCurrentIOState = (SubsessionIOState *) (subsession->miscPtr);
    if (fCurrentIOState == NULL) {
        continue;
    }
    if (strcmp(subsession->mediumName(), "audio") == 0) {
        continue;
    }
    size += addAtom_trak();
    if (fCurrentIOState->hasHintTrack()) {
        fCurrentIOState = fCurrentIOState->fHintTrackForUs;
        size += addAtom_trak();
    }
}
addAtomEnd;
addAtom(mvhd);
size += addWord(0x00000000);
size += addWord(fAppleCreationTime);
size += addWord(fAppleCreationTime);
size += addWord(movieTimeScale());
unsigned const duration = fMaxTrackDurationM;
fMVHD_durationPosn = TellFile64(fOutFid);
size += addWord(duration);
size += addWord(0x00010000);
size += addWord(0x01000000);
size += addZeroWords(2);
size += addWord(0x00010000);
size += addZeroWords(3);
size += addWord(0x00010000);
size += addZeroWords(3);
size += addWord(0x40000000);
size += addZeroWords(6);
size += addWord(SubsessionIOState::fCurrentTrackNumber + 1);
addAtomEnd;
addAtom(iods);
size += addWord(0x00000000);
size += addWord(0x10808080);
size += addWord(0x07004FFF);
size += addWord(0xFF0FFFFF);
addAtomEnd;
addAtom(trak);
size += addAtom_tkhd();
if (fCurrentIOState->fHeadChunk != NULL &&
    (fSyncStreams || fCurrentIOState->isHintTrack()))
{
    size += addAtom_edts();
}
if (fCurrentIOState->isHintTrack()) {
    size += addAtom_tref();
}
size += addAtom_mdia();
if (fCurrentIOState->isHintTrack()) {
    size += addAtom_udta();
}
addAtomEnd;
addAtom(tkhd);
if (fCurrentIOState->fQTEnableTrack) {
    size += addWord(0x0000000F);
} else {
    size += addWord(0x00000000);
}
size += addWord(fAppleCreationTime);
size += addWord(fAppleCreationTime);
size += addWord(fCurrentIOState->fTrackID);
size += addWord(0x00000000);
unsigned const duration = fCurrentIOState->fQTDurationM;
fCurrentIOState->fTKHD_durationPosn = TellFile64(fOutFid);
size += addWord(duration);
size += addZeroWords(3);
size += addWord(0x01000000);
size += addWord(0x00010000);
size += addZeroWords(3);
size += addWord(0x00010000);
size += addZeroWords(3);
size += addWord(0x40000000);
if (strcmp(fCurrentIOState->fOurSubsession.mediumName(), "video") == 0) {
    size += addWord(fMovieWidth << 16);
    size += addWord(fMovieHeight << 16);
} else {
    size += addZeroWords(2);
}
addAtomEnd;
addAtom(edts);
size += addAtom_elst();
addAtomEnd;
#define addEdit1(duration, trackPosition) \
    do { \
        unsigned trackDuration \
            = (unsigned) ((2*(duration)*movieTimeScale()+1)/2); \
        \
        size += addWord(trackDuration);  \
        totalDurationOfEdits += trackDuration; \
        size += addWord(trackPosition);  \
        size += addWord(0x00010000);  \
        ++numEdits; \
    } while (0)
#define addEdit(duration)      addEdit1((duration), editTrackPosition)
#define addEmptyEdit(duration) addEdit1((duration), (~0))
addAtom(elst);
size += addWord(0x00000000);
int64_t numEntriesPosition = TellFile64(fOutFid);
size += addWord(0);
unsigned numEdits = 0;
unsigned totalDurationOfEdits = 0;
double const syncThreshold    = 0.1;
struct timeval editStartTime  = fFirstDataTime;
unsigned editTrackPosition    = 0;
unsigned currentTrackPosition = 0;
double trackDurationOfEdit    = 0.0;
unsigned chunkDuration        = 0;
ChunkDescriptor* chunk        = fCurrentIOState->fHeadChunk;
while (chunk != NULL) {
    struct timeval const& chunkStartTime = chunk->fPresentationTime;
    double movieDurationOfEdit =
        (chunkStartTime.tv_sec - editStartTime.tv_sec)
        + (chunkStartTime.tv_usec - editStartTime.tv_usec) / 1000000.0;
    trackDurationOfEdit = (currentTrackPosition - editTrackPosition)
                          / (double) (fCurrentIOState->fQTTimeScale);
    double outOfSync = movieDurationOfEdit - trackDurationOfEdit;
    if (outOfSync > syncThreshold) {
        if (trackDurationOfEdit > 0.0) {
            addEdit(trackDurationOfEdit);
        }
        addEmptyEdit(outOfSync);
        editStartTime     = chunkStartTime;
        editTrackPosition = currentTrackPosition;
    } else if (outOfSync < -syncThreshold) {
        if (movieDurationOfEdit > 0.0) {
            addEdit(movieDurationOfEdit);
        }
        editStartTime     = chunkStartTime;
        editTrackPosition = currentTrackPosition;
    }
    unsigned numChannels = fCurrentIOState->fOurSubsession.numChannels();
    chunkDuration         = chunk->fNumFrames * chunk->fFrameDuration / numChannels;
    currentTrackPosition += chunkDuration;
    chunk = chunk->fNextChunk;
}
trackDurationOfEdit
+ = (double) chunkDuration / fCurrentIOState->fQTTimeScale;
if (trackDurationOfEdit > 0.0) {
    addEdit(trackDurationOfEdit);
}
setWord(numEntriesPosition, numEdits);
if (totalDurationOfEdits > fCurrentIOState->fQTDurationM) {
    fCurrentIOState->fQTDurationM = totalDurationOfEdits;
    setWord(fCurrentIOState->fTKHD_durationPosn, totalDurationOfEdits);
    if (totalDurationOfEdits > fMaxTrackDurationM) {
        fMaxTrackDurationM = totalDurationOfEdits;
        setWord(fMVHD_durationPosn, totalDurationOfEdits);
    }
    double scaleFactor =
        fCurrentIOState->fQTTimeScale / (double) movieTimeScale();
    fCurrentIOState->fQTDurationT =
        (unsigned) (totalDurationOfEdits * scaleFactor);
}
addAtomEnd;
addAtom(tref);
size += addAtom_hint();
addAtomEnd;
addAtom(hint);
SubsessionIOState* hintedTrack = fCurrentIOState->fTrackHintedByUs;
size += addWord(hintedTrack->fTrackID);
addAtomEnd;
addAtom(mdia);
size += addAtom_mdhd();
size += addAtom_hdlr();
size += addAtom_minf();
addAtomEnd;
addAtom(mdhd);
size += addWord(0x00000000);
size += addWord(fAppleCreationTime);
size += addWord(fAppleCreationTime);
unsigned const timeScale = fCurrentIOState->fQTTimeScale;
size += addWord(timeScale);
unsigned const duration = fCurrentIOState->fQTDurationT;
size += addWord(duration);
size += addWord(0x00000000);
addAtomEnd;
addAtom(hdlr);
size += addWord(0x00000000);
size += add4ByteString("mhlr");
size += addWord(fCurrentIOState->fQTcomponentSubtype);
size += add4ByteString("appl");
size += addWord(0x00000000);
size += addWord(0x00000000);
size += addArbitraryString(fCurrentIOState->fQTcomponentName);
addAtomEnd;
addAtom(minf);
SubsessionIOState::atomCreationFunc mediaInformationAtomCreator =
    fCurrentIOState->fQTMediaInformationAtomCreator;
size += (this->*mediaInformationAtomCreator)();
size += addAtom_hdlr2();
size += addAtom_dinf();
size += addAtom_stbl();
addAtomEnd;
addAtom(smhd);
size += addZeroWords(2);
addAtomEnd;
addAtom(vmhd);
size += addWord(0x00000001);
size += addWord(0x00408000);
size += addWord(0x80008000);
addAtomEnd;
addAtom(gmhd);
size += addAtom_gmin();
addAtomEnd;
addAtom(gmin);
size += addWord(0x00000000);
size += addWord(0x00408000);
size += addWord(0x80008000);
size += addWord(0x00000000);
addAtomEnd;
unsigned QuickTimeFileSink::addAtom_hdlr2()
{
    int64_t initFilePosn = TellFile64(fOutFid);
    unsigned size        = addAtomHeader("hdlr");

    size += addWord(0x00000000);
    size += add4ByteString("dhlr");
    size += add4ByteString("alis");
    size += add4ByteString("appl");
    size += addZeroWords(2);
    size += addArbitraryString("Apple Alias Data Handler");
    addAtomEnd;
    addAtom(dinf);
    size += addAtom_dref();
    addAtomEnd;
    addAtom(dref);
    size += addWord(0x00000000);
    size += addWord(0x00000001);
    size += addAtom_alis();
    addAtomEnd;
    addAtom(alis);
    size += addWord(0x00000001);
    addAtomEnd;
    addAtom(stbl);
    size += addAtom_stsd();
    size += addAtom_stts();
    if (fCurrentIOState->fQTcomponentSubtype == fourChar('v', 'i', 'd', 'e')) {
        size += addAtom_stss();
    }
    size += addAtom_stsc();
    size += addAtom_stsz();
    size += addAtom_co64();
    addAtomEnd;
    addAtom(stsd);
    size += addWord(0x00000000);
    size += addWord(0x00000001);
    SubsessionIOState::atomCreationFunc mediaDataAtomCreator =
        fCurrentIOState->fQTMediaDataAtomCreator;
    size += (this->*mediaDataAtomCreator)();
    addAtomEnd;
    unsigned QuickTimeFileSink::addAtom_genericMedia()
    {
        int64_t initFilePosn = TellFile64(fOutFid);
        QuickTimeGenericRTPSource* rtpSource = (QuickTimeGenericRTPSource *)
                                               fCurrentIOState->fOurSubsession.rtpSource();
        QuickTimeGenericRTPSource::QTState& qtState = rtpSource->qtState;
        char const* from = qtState.sdAtom;
        unsigned size    = qtState.sdAtomSize;

        for (unsigned i = 0; i < size; ++i) {
            addByte(from[i]);
        }
        addAtomEnd;
        unsigned QuickTimeFileSink::addAtom_soundMediaGeneral()
        {
            int64_t initFilePosn = TellFile64(fOutFid);
            unsigned size        = addAtomHeader(fCurrentIOState->fQTAudioDataType);

            size += addWord(0x00000000);
            size += addWord(0x00000001);
            unsigned short const version = fCurrentIOState->fQTSoundSampleVersion;
            size += addWord(version << 16);
            size += addWord(0x00000000);
            unsigned short numChannels =
                (unsigned short) (fCurrentIOState->fOurSubsession.numChannels());
            size += addHalfWord(numChannels);
            size += addHalfWord(0x0010);
            size += addWord(0xfffe0000);
            unsigned const sampleRateFixedPoint = fCurrentIOState->fQTTimeScale << 16;
            size += addWord(sampleRateFixedPoint);
            addAtomEnd;
            unsigned QuickTimeFileSink::addAtom_Qclp()
            {
                int64_t initFilePosn = TellFile64(fOutFid);

                fCurrentIOState->fQTAudioDataType      = "Qclp";
                fCurrentIOState->fQTSoundSampleVersion = 1;
                unsigned size = addAtom_soundMediaGeneral();
                size += addWord(0x000000a0);
                size += addWord(0x00000000);
                size += addWord(0x00000000);
                size += addWord(0x00000002);
                size += addAtom_wave();
                addAtomEnd;
                addAtom(wave);
                size += addAtom_frma();
                if (strcmp(fCurrentIOState->fQTAudioDataType, "Qclp") == 0) {
                    size += addWord(0x00000014);
                    size += add4ByteString("Qclp");
                    if (fCurrentIOState->fQTBytesPerFrame == 35) {
                        size += addAtom_Fclp();
                    } else {
                        size += addAtom_Hclp();
                    }
                    size += addWord(0x00000008);
                    size += addWord(0x00000000);
                    size += addWord(0x00000000);
                    size += addWord(0x00000008);
                } else if (strcmp(fCurrentIOState->fQTAudioDataType, "mp4a") == 0) {
                    size += addWord(0x0000000c);
                    size += add4ByteString("mp4a");
                    size += addWord(0x00000000);
                    size += addAtom_esds();
                    size += addWord(0x00000008);
                    size += addWord(0x00000000);
                }
                addAtomEnd;
                addAtom(frma);
                size += add4ByteString(fCurrentIOState->fQTAudioDataType);
                addAtomEnd;
                addAtom(Fclp);
                size += addWord(0x00000000);
                addAtomEnd;
                addAtom(Hclp);
                size += addWord(0x00000000);
                addAtomEnd;
                unsigned QuickTimeFileSink::addAtom_mp4a()
                {
                    unsigned size        = 0;
                    int64_t initFilePosn = TellFile64(fOutFid);

                    fCurrentIOState->fQTAudioDataType = "mp4a";
                    if (fGenerateMP4Format) {
                        fCurrentIOState->fQTSoundSampleVersion = 0;
                        size  = addAtom_soundMediaGeneral();
                        size += addAtom_esds();
                    } else {
                        fCurrentIOState->fQTSoundSampleVersion = 1;
                        size  = addAtom_soundMediaGeneral();
                        size += addWord(fCurrentIOState->fQTTimeUnitsPerSample);
                        size += addWord(0x00000001);
                        size += addWord(0x00000001);
                        size += addWord(0x00000002);
                        size += addAtom_wave();
                    }
                    addAtomEnd;
                    addAtom(esds);
                    MediaSubsession& subsession = fCurrentIOState->fOurSubsession;
                    if (strcmp(subsession.mediumName(), "audio") == 0) {
                        size += addWord(0x00000000);
                        size += addWord(0x03808080);
                        size += addWord(0x2a000000);
                        size += addWord(0x04808080);
                        size += addWord(0x1c401500);
                        size += addWord(0x18000000);
                        size += addWord(0x6d600000);
                        size += addWord(0x6d600580);
                        size += addByte(0x80);
                        size += addByte(0x80);
                    } else if (strcmp(subsession.mediumName(), "video") == 0) {
                        size += addWord(0x00000000);
                        size += addWord(0x03330000);
                        size += addWord(0x1f042b20);
                        size += addWord(0x1104fd46);
                        size += addWord(0x000d4e10);
                        size += addWord(0x000d4e10);
                        size += addByte(0x05);
                    }
                    unsigned configSize;
                    unsigned char* config =
                        parseGeneralConfigStr(subsession.fmtp_config(), configSize);
                    size += addByte(configSize);
                    for (unsigned i = 0; i < configSize; ++i) {
                        size += addByte(config[i]);
                    }
                    delete[] config;
                    if (strcmp(subsession.mediumName(), "audio") == 0) {
                        size += addWord(0x06808080);
                        size += addHalfWord(0x0102);
                    } else {
                        size += addHalfWord(0x0601);
                        size += addByte(0x02);
                    }
                    addAtomEnd;
                    addAtom(srcq);
                    size += addWord(0x00000040);
                    addAtomEnd;
                    addAtom(h263);
                    size += addWord(0x00000000);
                    size += addWord(0x00000001);
                    size += addWord(0x00020001);
                    size += add4ByteString("appl");
                    size += addWord(0x00000000);
                    size += addWord(0x000002fc);
                    unsigned const widthAndHeight = (fMovieWidth << 16) | fMovieHeight;
                    size += addWord(widthAndHeight);
                    size += addWord(0x00480000);
                    size += addWord(0x00480000);
                    size += addWord(0x00000000);
                    size += addWord(0x00010548);
                    size += addWord(0x2e323633);
                    size += addZeroWords(6);
                    size += addWord(0x00000018);
                    size += addHalfWord(0xffff);
                    addAtomEnd;
                    addAtom(avc1);
                    size += addWord(0x00000000);
                    size += addWord(0x00000001);
                    size += addWord(0x00000000);
                    size += add4ByteString("appl");
                    size += addWord(0x00000000);
                    size += addWord(0x00000000);
                    unsigned const widthAndHeight = (fMovieWidth << 16) | fMovieHeight;
                    size += addWord(widthAndHeight);
                    size += addWord(0x00480000);
                    size += addWord(0x00480000);
                    size += addWord(0x00000000);
                    size += addWord(0x00010548);
                    size += addWord(0x2e323634);
                    size += addZeroWords(6);
                    size += addWord(0x00000018);
                    size += addHalfWord(0xffff);
                    size += addAtom_avcC();
                    addAtomEnd;
                    addAtom(avcC);
                    char* psets = strDup(fCurrentIOState->fOurSubsession.fmtp_spropparametersets());
                    if (psets == NULL) {
                        return 0;
                    }
                    size_t comma_pos = strcspn(psets, ",");
                    psets[comma_pos] = '\0';
                    char const* sps_b64 = psets;
                    char const* pps_b64 = &psets[comma_pos + 1];
                    unsigned sps_count;
                    unsigned char* sps_data = base64Decode(sps_b64, sps_count, false);
                    unsigned pps_count;
                    unsigned char* pps_data = base64Decode(pps_b64, pps_count, false);
                    size += addByte(0x01);
                    size += addByte(sps_data[1]);
                    size += addByte(sps_data[2]);
                    size += addByte(sps_data[3]);
                    size += addByte(0xff);
                    size += addByte(0xe0 | (sps_count > 0 ? 1 : 0));
                    if (sps_count > 0) {
                        size += addHalfWord(sps_count);
                        for (unsigned i = 0; i < sps_count; i++) {
                            size += addByte(sps_data[i]);
                        }
                    }
                    size += addByte(pps_count > 0 ? 1 : 0);
                    if (pps_count > 0) {
                        size += addHalfWord(pps_count);
                        for (unsigned i = 0; i < pps_count; i++) {
                            size += addByte(pps_data[i]);
                        }
                    }
                    delete[] pps_data;
                    delete[] sps_data;
                    delete[] psets;
                    addAtomEnd;
                    addAtom(mp4v);
                    size += addWord(0x00000000);
                    size += addWord(0x00000001);
                    size += addWord(0x00020001);
                    size += add4ByteString("appl");
                    size += addWord(0x00000200);
                    size += addWord(0x00000400);
                    unsigned const widthAndHeight = (fMovieWidth << 16) | fMovieHeight;
                    size += addWord(widthAndHeight);
                    size += addWord(0x00480000);
                    size += addWord(0x00480000);
                    size += addWord(0x00000000);
                    size += addWord(0x00010c4d);
                    size += addWord(0x5045472d);
                    size += addWord(0x34205669);
                    size += addWord(0x64656f00);
                    size += addZeroWords(4);
                    size += addWord(0x00000018);
                    size += addHalfWord(0xffff);
                    size += addAtom_esds();
                    size += addWord(0x00000000);
                    addAtomEnd;
                    unsigned QuickTimeFileSink::addAtom_rtp()
                    {
                        int64_t initFilePosn = TellFile64(fOutFid);
                        unsigned size        = addAtomHeader("rtp ");

                        size += addWord(0x00000000);
                        size += addWord(0x00000001);
                        size += addWord(0x00010001);
                        size += addWord(1450);
                        size += addAtom_tims();
                        addAtomEnd;
                        addAtom(tims);
                        size += addWord(fCurrentIOState->fOurSubsession.rtpTimestampFrequency());
                        addAtomEnd;
                        addAtom(stts);
                        size += addWord(0x00000000);
                        int64_t numEntriesPosition = TellFile64(fOutFid);
                        size += addWord(0);
                        unsigned numEntries = 0, numSamplesSoFar = 0;
                        unsigned prevSampleDuration    = 0;
                        unsigned const samplesPerFrame = fCurrentIOState->fQTSamplesPerFrame;
                        ChunkDescriptor* chunk         = fCurrentIOState->fHeadChunk;
                        while (chunk != NULL) {
                            unsigned const sampleDuration = chunk->fFrameDuration / samplesPerFrame;
                            if (sampleDuration != prevSampleDuration) {
                                if (chunk != fCurrentIOState->fHeadChunk) {
                                    ++numEntries;
                                    size += addWord(numSamplesSoFar);
                                    size += addWord(prevSampleDuration);
                                    numSamplesSoFar = 0;
                                }
                            }
                            unsigned const numSamples = chunk->fNumFrames * samplesPerFrame;
                            numSamplesSoFar   += numSamples;
                            prevSampleDuration = sampleDuration;
                            chunk = chunk->fNextChunk;
                        }
                        ++numEntries;
                        size += addWord(numSamplesSoFar);
                        size += addWord(prevSampleDuration);
                        setWord(numEntriesPosition, numEntries);
                        addAtomEnd;
                        addAtom(stss);
                        size += addWord(0x00000000);
                        int64_t numEntriesPosition = TellFile64(fOutFid);
                        size += addWord(0);
                        unsigned numEntries = 0, numSamplesSoFar = 0;
                        if (fCurrentIOState->fHeadSyncFrame != NULL) {
                            SyncFrame* currentSyncFrame = fCurrentIOState->fHeadSyncFrame;
                            while (currentSyncFrame != NULL) {
                                ++numEntries;
                                size += addWord(currentSyncFrame->sfFrameNum);
                                currentSyncFrame = currentSyncFrame->nextSyncFrame;
                            }
                        } else {
                            unsigned const samplesPerFrame = fCurrentIOState->fQTSamplesPerFrame;
                            ChunkDescriptor* chunk         = fCurrentIOState->fHeadChunk;
                            while (chunk != NULL) {
                                unsigned const numSamples = chunk->fNumFrames * samplesPerFrame;
                                numSamplesSoFar += numSamples;
                                chunk = chunk->fNextChunk;
                            }
                            unsigned i;
                            for (i = 0; i < numSamplesSoFar; i += 12) {
                                size += addWord(i + 1);
                                ++numEntries;
                            }
                            if (i != (numSamplesSoFar - 1)) {
                                size += addWord(numSamplesSoFar);
                                ++numEntries;
                            }
                        }
                        setWord(numEntriesPosition, numEntries);
                        addAtomEnd;
                        addAtom(stsc);
                        size += addWord(0x00000000);
                        int64_t numEntriesPosition = TellFile64(fOutFid);
                        size += addWord(0);
                        unsigned numEntries = 0, chunkNumber = 0;
                        unsigned prevSamplesPerChunk   = ~0;
                        unsigned const samplesPerFrame = fCurrentIOState->fQTSamplesPerFrame;
                        ChunkDescriptor* chunk         = fCurrentIOState->fHeadChunk;
                        while (chunk != NULL) {
                            ++chunkNumber;
                            unsigned const samplesPerChunk = chunk->fNumFrames * samplesPerFrame;
                            if (samplesPerChunk != prevSamplesPerChunk) {
                                ++numEntries;
                                size += addWord(chunkNumber);
                                size += addWord(samplesPerChunk);
                                size += addWord(0x00000001);
                                prevSamplesPerChunk = samplesPerChunk;
                            }
                            chunk = chunk->fNextChunk;
                        }
                        setWord(numEntriesPosition, numEntries);
                        addAtomEnd;
                        addAtom(stsz);
                        size += addWord(0x00000000);
                        Boolean haveSingleEntryTable = True;
                        double firstBPS        = 0.0;
                        ChunkDescriptor* chunk = fCurrentIOState->fHeadChunk;
                        while (chunk != NULL) {
                            double bps =
                                (double) (chunk->fFrameSize) / (fCurrentIOState->fQTSamplesPerFrame);
                            if (bps < 1.0) {
                                break;
                            }
                            if (firstBPS == 0.0) {
                                firstBPS = bps;
                            } else if (bps != firstBPS) {
                                haveSingleEntryTable = False;
                                break;
                            }
                            chunk = chunk->fNextChunk;
                        }
                        unsigned sampleSize;
                        if (haveSingleEntryTable) {
                            if (fCurrentIOState->isHintTrack() &&
                                fCurrentIOState->fHeadChunk != NULL)
                            {
                                sampleSize = fCurrentIOState->fHeadChunk->fFrameSize
                                             / fCurrentIOState->fQTSamplesPerFrame;
                            } else {
                                sampleSize = fCurrentIOState->fQTTimeUnitsPerSample;
                            }
                        } else {
                            sampleSize = 0;
                        }
                        size += addWord(sampleSize);
                        unsigned const totNumSamples = fCurrentIOState->fQTTotNumSamples;
                        size += addWord(totNumSamples);
                        if (!haveSingleEntryTable) {
                            ChunkDescriptor* chunk = fCurrentIOState->fHeadChunk;
                            while (chunk != NULL) {
                                unsigned numSamples =
                                    chunk->fNumFrames * (fCurrentIOState->fQTSamplesPerFrame);
                                unsigned sampleSize =
                                    chunk->fFrameSize / (fCurrentIOState->fQTSamplesPerFrame);
                                for (unsigned i = 0; i < numSamples; ++i) {
                                    size += addWord(sampleSize);
                                }
                                chunk = chunk->fNextChunk;
                            }
                        }
                        addAtomEnd;
                        addAtom(co64);
                        size += addWord(0x00000000);
                        size += addWord(fCurrentIOState->fNumChunks);
                        ChunkDescriptor* chunk = fCurrentIOState->fHeadChunk;
                        while (chunk != NULL) {
                            size += addWord64(chunk->fOffsetInFile);
                            chunk = chunk->fNextChunk;
                        }
                        addAtomEnd;
                        addAtom(udta);
                        size += addAtom_name();
                        size += addAtom_hnti();
                        size += addAtom_hinf();
                        addAtomEnd;
                        addAtom(name);
                        char description[100];
                        sprintf(description, "Hinted %s track",
                                fCurrentIOState->fOurSubsession.mediumName());
                        size += addArbitraryString(description, False);
                        addAtomEnd;
                        addAtom(hnti);
                        size += addAtom_sdp();
                        addAtomEnd;
                        unsigned QuickTimeFileSink::addAtom_sdp()
                        {
                            int64_t initFilePosn = TellFile64(fOutFid);
                            unsigned size = addAtomHeader("sdp ");
                            char const* sdpLines = fCurrentIOState->fOurSubsession.savedSDPLines();
                            char* newSDPLines = new char[strlen(sdpLines) + 100];
                            char const* searchStr = "a=control:trackid=";
                            Boolean foundSearchString = False;
                            char const* p1, * p2, * p3;

                            for (p1 = sdpLines; *p1 != '\0'; ++p1) {
                                for (p2 = p1, p3 = searchStr; tolower(*p2) == *p3; ++p2, ++p3) {}
                                if (*p3 == '\0') {
                                    int beforeTrackNumPosn = p2 - sdpLines;
                                    int trackNumLength;
                                    if (sscanf(p2, " %*d%n", &trackNumLength) < 0) {
                                        break;
                                    }
                                    int afterTrackNumPosn = beforeTrackNumPosn + trackNumLength;
                                    int i;
                                    for (i = 0; i < beforeTrackNumPosn; ++i) {
                                        newSDPLines[i] = sdpLines[i];
                                    }
                                    sprintf(&newSDPLines[i], "%d", fCurrentIOState->fTrackID);
                                    i = afterTrackNumPosn;
                                    int j = i + strlen(&newSDPLines[i]);
                                    while (1) {
                                        if ((newSDPLines[j] = sdpLines[i]) == '\0') {
                                            break;
                                        }
                                        ++i;
                                        ++j;
                                    }
                                    foundSearchString = True;
                                    break;
                                }
                            }
                            if (!foundSearchString) {
                                sprintf(newSDPLines, "%s%s%d\r\n",
                                        sdpLines, searchStr, fCurrentIOState->fTrackID);
                            }
                            size += addArbitraryString(newSDPLines, False);
                            delete[] newSDPLines;
                            addAtomEnd;
                            addAtom(hinf);
                            size += addAtom_totl();
                            size += addAtom_npck();
                            size += addAtom_tpay();
                            size += addAtom_trpy();
                            size += addAtom_nump();
                            size += addAtom_tpyl();
                            size += addAtom_dmed();
                            size += addAtom_dimm();
                            size += addAtom_drep();
                            size += addAtom_tmin();
                            size += addAtom_tmax();
                            size += addAtom_pmax();
                            size += addAtom_dmax();
                            size += addAtom_payt();
                            addAtomEnd;
                            addAtom(totl);
                            size += addWord(fCurrentIOState->fHINF.trpy.lo);
                            addAtomEnd;
                            addAtom(npck);
                            size += addWord(fCurrentIOState->fHINF.nump.lo);
                            addAtomEnd;
                            addAtom(tpay);
                            size += addWord(fCurrentIOState->fHINF.tpyl.lo);
                            addAtomEnd;
                            addAtom(trpy);
                            size += addWord(fCurrentIOState->fHINF.trpy.hi);
                            size += addWord(fCurrentIOState->fHINF.trpy.lo);
                            addAtomEnd;
                            addAtom(nump);
                            size += addWord(fCurrentIOState->fHINF.nump.hi);
                            size += addWord(fCurrentIOState->fHINF.nump.lo);
                            addAtomEnd;
                            addAtom(tpyl);
                            size += addWord(fCurrentIOState->fHINF.tpyl.hi);
                            size += addWord(fCurrentIOState->fHINF.tpyl.lo);
                            addAtomEnd;
                            addAtom(dmed);
                            size += addWord(fCurrentIOState->fHINF.dmed.hi);
                            size += addWord(fCurrentIOState->fHINF.dmed.lo);
                            addAtomEnd;
                            addAtom(dimm);
                            size += addWord(fCurrentIOState->fHINF.dimm.hi);
                            size += addWord(fCurrentIOState->fHINF.dimm.lo);
                            addAtomEnd;
                            addAtom(drep);
                            size += addWord(0);
                            size += addWord(0);
                            addAtomEnd;
                            addAtom(tmin);
                            size += addWord(0);
                            addAtomEnd;
                            addAtom(tmax);
                            size += addWord(0);
                            addAtomEnd;
                            addAtom(pmax);
                            size += addWord(fCurrentIOState->fHINF.pmax);
                            addAtomEnd;
                            addAtom(dmax);
                            size += addWord(fCurrentIOState->fHINF.dmax);
                            addAtomEnd;
                            addAtom(payt);
                            MediaSubsession& ourSubsession = fCurrentIOState->fOurSubsession;
                            RTPSource* rtpSource           = ourSubsession.rtpSource();
                            size += addWord(rtpSource->rtpPayloadFormat());
                            unsigned rtpmapStringLength = strlen(ourSubsession.codecName()) + 20;
                            char* rtpmapString = new char[rtpmapStringLength];
                            sprintf(rtpmapString, "%s/%d",
                                    ourSubsession.codecName(), rtpSource->timestampFrequency());
                            size += addArbitraryString(rtpmapString);
                            delete[] rtpmapString;
                            addAtomEnd;
                            unsigned QuickTimeFileSink::addAtom_dummy()
                            {
                                int64_t initFilePosn = TellFile64(fOutFid);
                                unsigned size        = addAtomHeader("????");

                                addAtomEnd;
