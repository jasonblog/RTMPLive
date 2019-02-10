#include "MPEG1or2FileServerDemux.hh"
#include "MPEG1or2DemuxedServerMediaSubsession.hh"
#include "ByteStreamFileSource.hh"
MPEG1or2FileServerDemux * MPEG1or2FileServerDemux::createNew(UsageEnvironment& env, char const* fileName,
                                                             Boolean reuseFirstSource)
{
    return new MPEG1or2FileServerDemux(env, fileName, reuseFirstSource);
}

static float MPEG1or2ProgramStreamFileDuration(UsageEnvironment& env,
                                               char const*     fileName,
                                               unsigned        & fileSize);
MPEG1or2FileServerDemux
::MPEG1or2FileServerDemux(UsageEnvironment& env, char const* fileName,
                          Boolean reuseFirstSource)
    : Medium(env),
    fReuseFirstSource(reuseFirstSource),
    fSession0Demux(NULL), fLastCreatedDemux(NULL), fLastClientSessionId(~0)
{
    fFileName     = strDup(fileName);
    fFileDuration = MPEG1or2ProgramStreamFileDuration(env, fileName, fFileSize);
}

MPEG1or2FileServerDemux::~MPEG1or2FileServerDemux()
{
    Medium::close(fSession0Demux);
    delete[](char *) fFileName;
}

ServerMediaSubsession * MPEG1or2FileServerDemux::newAudioServerMediaSubsession()
{
    return MPEG1or2DemuxedServerMediaSubsession::createNew(*this, 0xC0, fReuseFirstSource);
}

ServerMediaSubsession * MPEG1or2FileServerDemux::newVideoServerMediaSubsession(Boolean iFramesOnly,
                                                                               double  vshPeriod)
{
    return MPEG1or2DemuxedServerMediaSubsession::createNew(*this, 0xE0, fReuseFirstSource,
                                                           iFramesOnly, vshPeriod);
}

ServerMediaSubsession * MPEG1or2FileServerDemux::newAC3AudioServerMediaSubsession()
{
    return MPEG1or2DemuxedServerMediaSubsession::createNew(*this, 0xBD, fReuseFirstSource);
}

MPEG1or2DemuxedElementaryStream * MPEG1or2FileServerDemux::newElementaryStream(unsigned clientSessionId,
                                                                               u_int8_t streamIdTag)
{
    MPEG1or2Demux* demuxToUse;

    if (clientSessionId == 0) {
        if (fSession0Demux == NULL) {
            ByteStreamFileSource* fileSource =
                ByteStreamFileSource::createNew(envir(), fFileName);
            if (fileSource == NULL) {
                return NULL;
            }
            fSession0Demux = MPEG1or2Demux::createNew(envir(), fileSource, False);
        }
        demuxToUse = fSession0Demux;
    } else {
        if (clientSessionId != fLastClientSessionId) {
            ByteStreamFileSource* fileSource =
                ByteStreamFileSource::createNew(envir(), fFileName);
            if (fileSource == NULL) {
                return NULL;
            }
            fLastCreatedDemux    = MPEG1or2Demux::createNew(envir(), fileSource, True);
            fLastClientSessionId = clientSessionId;
        }
        demuxToUse = fLastCreatedDemux;
    }
    if (demuxToUse == NULL) {
        return NULL;
    }
    return demuxToUse->newElementaryStream(streamIdTag);
}

static Boolean getMPEG1or2TimeCode(FramedSource* dataSource,
                                   MPEG1or2Demux & parentDemux,
                                   Boolean       returnFirstSeenCode,
                                   float         & timeCode);
static float MPEG1or2ProgramStreamFileDuration(UsageEnvironment& env,
                                               char const*     fileName,
                                               unsigned        & fileSize)
{
    FramedSource* dataSource = NULL;
    float duration = 0.0;

    fileSize = 0;
    do {
        ByteStreamFileSource* fileSource = ByteStreamFileSource::createNew(env, fileName);
        if (fileSource == NULL) {
            break;
        }
        dataSource = fileSource;
        fileSize   = (unsigned) (fileSource->fileSize());
        if (fileSize == 0) {
            break;
        }
        MPEG1or2Demux* baseDemux = MPEG1or2Demux::createNew(env, dataSource, True);
        if (baseDemux == NULL) {
            break;
        }
        dataSource = baseDemux->newRawPESStream();
        float firstTimeCode;
        if (!getMPEG1or2TimeCode(dataSource, *baseDemux, True, firstTimeCode)) {
            break;
        }
        baseDemux->flushInput();
        unsigned const startByteFromEnd = 100000;
        unsigned newFilePosition        =
            fileSize < startByteFromEnd ? 0 : fileSize - startByteFromEnd;
        if (newFilePosition > 0) {
            fileSource->seekToByteAbsolute(newFilePosition);
        }
        float lastTimeCode;
        if (!getMPEG1or2TimeCode(dataSource, *baseDemux, False, lastTimeCode)) {
            break;
        }
        float timeCodeDiff = lastTimeCode - firstTimeCode;
        if (timeCodeDiff < 0) {
            break;
        }
        duration = timeCodeDiff;
    } while (0);
    Medium::close(dataSource);
    return duration;
} // MPEG1or2ProgramStreamFileDuration

#define MFSD_DUMMY_SINK_BUFFER_SIZE (6+65535)
class MFSD_DummySink : public MediaSink
{
public:
    MFSD_DummySink(MPEG1or2Demux& demux, Boolean returnFirstSeenCode);
    virtual ~MFSD_DummySink();
    char watchVariable;
private:
    virtual Boolean continuePlaying();
private:
    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);
    void afterGettingFrame1();
private:
    MPEG1or2Demux& fOurDemux;
    Boolean fReturnFirstSeenCode;
    unsigned char fBuf[MFSD_DUMMY_SINK_BUFFER_SIZE];
};
static void afterPlayingMFSD_DummySink(MFSD_DummySink* sink);
static float computeSCRTimeCode(MPEG1or2Demux::SCR const& scr);
static Boolean getMPEG1or2TimeCode(FramedSource* dataSource,
                                   MPEG1or2Demux & parentDemux,
                                   Boolean       returnFirstSeenCode,
                                   float         & timeCode)
{
    parentDemux.lastSeenSCR().isValid = False;
    UsageEnvironment& env = dataSource->envir();
    MFSD_DummySink sink(parentDemux, returnFirstSeenCode);
    sink.startPlaying(*dataSource,
                      (MediaSink::afterPlayingFunc *) afterPlayingMFSD_DummySink, &sink);
    env.taskScheduler().doEventLoop(&sink.watchVariable);
    timeCode = computeSCRTimeCode(parentDemux.lastSeenSCR());
    return parentDemux.lastSeenSCR().isValid;
}

MFSD_DummySink::MFSD_DummySink(MPEG1or2Demux& demux, Boolean returnFirstSeenCode)
    : MediaSink(demux.envir()),
    watchVariable(0), fOurDemux(demux), fReturnFirstSeenCode(returnFirstSeenCode)
{}

MFSD_DummySink::~MFSD_DummySink()
{}

Boolean MFSD_DummySink::continuePlaying()
{
    if (fSource == NULL) {
        return False;
    }
    fSource->getNextFrame(fBuf, sizeof fBuf,
                          afterGettingFrame, this,
                          onSourceClosure, this);
    return True;
}

void MFSD_DummySink::afterGettingFrame(void* clientData, unsigned,
                                       unsigned,
                                       struct timeval,
                                       unsigned)
{
    MFSD_DummySink* sink = (MFSD_DummySink *) clientData;

    sink->afterGettingFrame1();
}

void MFSD_DummySink::afterGettingFrame1()
{
    if (fReturnFirstSeenCode && fOurDemux.lastSeenSCR().isValid) {
        onSourceClosure();
        return;
    }
    continuePlaying();
}

static void afterPlayingMFSD_DummySink(MFSD_DummySink* sink)
{
    sink->watchVariable = ~0;
}

static float computeSCRTimeCode(MPEG1or2Demux::SCR const& scr)
{
    double result = scr.remainingBits / 90000.0 + scr.extension / 300.0;

    if (scr.highBit) {
        double const highBitValue = (256 * 1024 * 1024) / 5625.0;
        result += highBitValue;
    }
    return (float) result;
}
