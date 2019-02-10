#include "MPEG1or2AudioStreamFramer.hh"
#include "StreamParser.hh"
#include "MP3Internals.hh"
#include <GroupsockHelper.hh>
class MPEG1or2AudioStreamParser : public StreamParser
{
public:
    MPEG1or2AudioStreamParser(MPEG1or2AudioStreamFramer* usingSource,
                              FramedSource*              inputSource);
    virtual ~MPEG1or2AudioStreamParser();
public:
    unsigned parse(unsigned& numTruncatedBytes);
    void registerReadInterest(unsigned char* to, unsigned maxSize);
    MP3FrameParams const& currentFrame() const
    {
        return fCurrentFrame;
    }

private:
    unsigned char* fTo;
    unsigned fMaxSize;
    MP3FrameParams fCurrentFrame;
};
MPEG1or2AudioStreamFramer
::MPEG1or2AudioStreamFramer(UsageEnvironment& env, FramedSource* inputSource,
                            Boolean syncWithInputSource)
    : FramedFilter(env, inputSource),
    fSyncWithInputSource(syncWithInputSource)
{
    reset();
    fParser = new MPEG1or2AudioStreamParser(this, inputSource);
}

MPEG1or2AudioStreamFramer::~MPEG1or2AudioStreamFramer()
{
    delete fParser;
}

MPEG1or2AudioStreamFramer * MPEG1or2AudioStreamFramer::createNew(UsageEnvironment& env,
                                                                 FramedSource*   inputSource,
                                                                 Boolean         syncWithInputSource)
{
    return new MPEG1or2AudioStreamFramer(env, inputSource, syncWithInputSource);
}

void MPEG1or2AudioStreamFramer::flushInput()
{
    reset();
    fParser->flushInput();
}

void MPEG1or2AudioStreamFramer::reset()
{
    struct timeval timeNow;

    gettimeofday(&timeNow, NULL);
    resetPresentationTime(timeNow);
}

void MPEG1or2AudioStreamFramer
::resetPresentationTime(struct timeval newPresentationTime)
{
    fNextFramePresentationTime = newPresentationTime;
}

void MPEG1or2AudioStreamFramer::doGetNextFrame()
{
    fParser->registerReadInterest(fTo, fMaxSize);
    continueReadProcessing();
}

#define MILLION 1000000
static unsigned const numSamplesByLayer[4] = { 0, 384, 1152, 1152 };
struct timeval MPEG1or2AudioStreamFramer::currentFramePlayTime() const
{
    MP3FrameParams const& fr  = fParser->currentFrame();
    unsigned const numSamples = numSamplesByLayer[fr.layer];
    struct timeval result;
    unsigned const freq = fr.samplingFreq * (1 + fr.isMPEG2);

    if (freq == 0) {
        result.tv_sec  = 0;
        result.tv_usec = 0;
        return result;
    }
    unsigned const uSeconds =
        ((numSamples * 2 * MILLION) / freq + 1) / 2;
    result.tv_sec  = uSeconds / MILLION;
    result.tv_usec = uSeconds % MILLION;
    return result;
}

void MPEG1or2AudioStreamFramer
::continueReadProcessing(void* clientData,
                         unsigned char *, unsigned,
                         struct timeval presentationTime)
{
    MPEG1or2AudioStreamFramer* framer = (MPEG1or2AudioStreamFramer *) clientData;

    if (framer->fSyncWithInputSource) {
        framer->resetPresentationTime(presentationTime);
    }
    framer->continueReadProcessing();
}

void MPEG1or2AudioStreamFramer::continueReadProcessing()
{
    unsigned acquiredFrameSize = fParser->parse(fNumTruncatedBytes);

    if (acquiredFrameSize > 0) {
        fFrameSize        = acquiredFrameSize;
        fPresentationTime = fNextFramePresentationTime;
        struct timeval framePlayTime = currentFramePlayTime();
        fDurationInMicroseconds = framePlayTime.tv_sec * MILLION + framePlayTime.tv_usec;
        fNextFramePresentationTime.tv_usec += framePlayTime.tv_usec;
        fNextFramePresentationTime.tv_sec  +=
            framePlayTime.tv_sec + fNextFramePresentationTime.tv_usec / MILLION;
        fNextFramePresentationTime.tv_usec %= MILLION;
        afterGetting(this);
    } else {}
}

MPEG1or2AudioStreamParser
::MPEG1or2AudioStreamParser(MPEG1or2AudioStreamFramer* usingSource,
                            FramedSource*              inputSource)
    : StreamParser(inputSource, FramedSource::handleClosure, usingSource,
                   &MPEG1or2AudioStreamFramer::continueReadProcessing, usingSource)
{}

MPEG1or2AudioStreamParser::~MPEG1or2AudioStreamParser()
{}

void MPEG1or2AudioStreamParser::registerReadInterest(unsigned char* to,
                                                     unsigned       maxSize)
{
    fTo      = to;
    fMaxSize = maxSize;
}

unsigned MPEG1or2AudioStreamParser::parse(unsigned& numTruncatedBytes)
{
    try {
        saveParserState();
        while (((fCurrentFrame.hdr = test4Bytes()) & 0xFFE00000) != 0xFFE00000) {
            skipBytes(1);
            saveParserState();
        }
        fCurrentFrame.setParamsFromHeader();
        unsigned frameSize = fCurrentFrame.frameSize + 4;
        if (frameSize > fMaxSize) {
            numTruncatedBytes = frameSize - fMaxSize;
            frameSize         = fMaxSize;
        } else {
            numTruncatedBytes = 0;
        }
        getBytes(fTo, frameSize);
        skipBytes(numTruncatedBytes);
        return frameSize;
    } catch (int) {
        #ifdef DEBUG
        fprintf(stderr, "MPEG1or2AudioStreamParser::parse() EXCEPTION (This is normal behavior - *not* an error)\n");
        #endif
        return 0;
    }
}
