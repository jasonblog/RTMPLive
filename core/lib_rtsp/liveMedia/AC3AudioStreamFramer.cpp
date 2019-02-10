#include "AC3AudioStreamFramer.hh"
#include "StreamParser.hh"
#include <GroupsockHelper.hh>
class AC3FrameParams
{
public:
    AC3FrameParams() : samplingFreq(0) {}

    unsigned hdr0, hdr1;
    unsigned kbps, samplingFreq, frameSize;
    void setParamsFromHeader();
};
class AC3AudioStreamParser : public StreamParser
{
public:
    AC3AudioStreamParser(AC3AudioStreamFramer* usingSource,
                         FramedSource*         inputSource);
    virtual ~AC3AudioStreamParser();
public:
    void testStreamCode(unsigned char ourStreamCode,
                        unsigned char* ptr, unsigned size);
    unsigned parseFrame(unsigned& numTruncatedBytes);
    void registerReadInterest(unsigned char* to, unsigned maxSize);
    AC3FrameParams const& currentFrame() const
    {
        return fCurrentFrame;
    }

    Boolean haveParsedAFrame() const
    {
        return fHaveParsedAFrame;
    }

    void readAndSaveAFrame();
private:
    static void afterGettingSavedFrame(void* clientData, unsigned frameSize,
                                       unsigned numTruncatedBytes,
                                       struct timeval presentationTime,
                                       unsigned durationInMicroseconds);
    void afterGettingSavedFrame1(unsigned frameSize);
    static void onSavedFrameClosure(void* clientData);
    void onSavedFrameClosure1();
private:
    AC3AudioStreamFramer* fUsingSource;
    unsigned char* fTo;
    unsigned fMaxSize;
    Boolean fHaveParsedAFrame;
    unsigned char* fSavedFrame;
    unsigned fSavedFrameSize;
    char fSavedFrameFlag;
    AC3FrameParams fCurrentFrame;
};
AC3AudioStreamFramer::AC3AudioStreamFramer(UsageEnvironment& env,
                                           FramedSource*   inputSource,
                                           unsigned char   streamCode)
    : FramedFilter(env, inputSource), fOurStreamCode(streamCode)
{
    gettimeofday(&fNextFramePresentationTime, NULL);
    fParser = new AC3AudioStreamParser(this, inputSource);
}

AC3AudioStreamFramer::~AC3AudioStreamFramer()
{
    delete fParser;
}

AC3AudioStreamFramer * AC3AudioStreamFramer::createNew(UsageEnvironment& env,
                                                       FramedSource*   inputSource,
                                                       unsigned char   streamCode)
{
    return new AC3AudioStreamFramer(env, inputSource, streamCode);
}

unsigned AC3AudioStreamFramer::samplingRate()
{
    if (!fParser->haveParsedAFrame()) {
        fParser->readAndSaveAFrame();
    }
    return fParser->currentFrame().samplingFreq;
}

void AC3AudioStreamFramer::flushInput()
{
    fParser->flushInput();
}

void AC3AudioStreamFramer::doGetNextFrame()
{
    fParser->registerReadInterest(fTo, fMaxSize);
    parseNextFrame();
}

#define MILLION 1000000
struct timeval AC3AudioStreamFramer::currentFramePlayTime() const
{
    AC3FrameParams const& fr  = fParser->currentFrame();
    unsigned const numSamples = 1536;
    unsigned const freq       = fr.samplingFreq;
    unsigned const uSeconds   = (freq == 0) ? 0 :
                                ((numSamples * 2 * MILLION) / freq + 1) / 2;
    struct timeval result;

    result.tv_sec  = uSeconds / MILLION;
    result.tv_usec = uSeconds % MILLION;
    return result;
}

void AC3AudioStreamFramer
::handleNewData(void* clientData, unsigned char* ptr, unsigned size,
                struct timeval)
{
    AC3AudioStreamFramer* framer = (AC3AudioStreamFramer *) clientData;

    framer->handleNewData(ptr, size);
}

void AC3AudioStreamFramer
::handleNewData(unsigned char* ptr, unsigned size)
{
    fParser->testStreamCode(fOurStreamCode, ptr, size);
    parseNextFrame();
}

void AC3AudioStreamFramer::parseNextFrame()
{
    unsigned acquiredFrameSize = fParser->parseFrame(fNumTruncatedBytes);

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

static int const kbpsTable[] = { 32,  40,  48,  56,  64,  80,  96,  112,
                                 128, 160, 192, 224, 256, 320, 384, 448,
                                 512, 576, 640
};
void AC3FrameParams::setParamsFromHeader()
{
    unsigned char byte4     = hdr1 >> 24;
    unsigned char kbpsIndex = (byte4 & 0x3E) >> 1;

    if (kbpsIndex > 18) {
        kbpsIndex = 18;
    }
    kbps = kbpsTable[kbpsIndex];
    unsigned char samplingFreqIndex = (byte4 & 0xC0) >> 6;
    switch (samplingFreqIndex) {
        case 0:
            samplingFreq = 48000;
            frameSize    = 4 * kbps;
            break;
        case 1:
            samplingFreq = 44100;
            frameSize    = 2 * (320 * kbps / 147 + (byte4 & 1));
            break;
        case 2:
        case 3:
            samplingFreq = 32000;
            frameSize    = 6 * kbps;
    }
}

AC3AudioStreamParser
::AC3AudioStreamParser(AC3AudioStreamFramer* usingSource,
                       FramedSource*         inputSource)
    : StreamParser(inputSource, FramedSource::handleClosure, usingSource,
                   &AC3AudioStreamFramer::handleNewData, usingSource),
    fUsingSource(usingSource), fHaveParsedAFrame(False),
    fSavedFrame(NULL), fSavedFrameSize(0)
{}

AC3AudioStreamParser::~AC3AudioStreamParser()
{}

void AC3AudioStreamParser::registerReadInterest(unsigned char* to,
                                                unsigned       maxSize)
{
    fTo      = to;
    fMaxSize = maxSize;
}

void AC3AudioStreamParser
::testStreamCode(unsigned char ourStreamCode,
                 unsigned char* ptr, unsigned size)
{
    if (ourStreamCode == 0) {
        return;
    }
    if (size < 4) {
        return;
    }
    unsigned char streamCode = *ptr;
    if (streamCode == ourStreamCode) {
        memmove(ptr, ptr + 4, size - 4);
        totNumValidBytes() = totNumValidBytes() - 4;
    } else {
        totNumValidBytes() = totNumValidBytes() - size;
    }
}

unsigned AC3AudioStreamParser::parseFrame(unsigned& numTruncatedBytes)
{
    if (fSavedFrameSize > 0) {
        memmove(fTo, fSavedFrame, fSavedFrameSize);
        delete[] fSavedFrame;
        fSavedFrame = NULL;
        unsigned frameSize = fSavedFrameSize;
        fSavedFrameSize = 0;
        return frameSize;
    }
    try {
        saveParserState();
        while (1) {
            unsigned next4Bytes = test4Bytes();
            if (next4Bytes >> 16 == 0x0B77) {
                break;
            }
            skipBytes(1);
            saveParserState();
        }
        fCurrentFrame.hdr0 = get4Bytes();
        fCurrentFrame.hdr1 = test4Bytes();
        fCurrentFrame.setParamsFromHeader();
        fHaveParsedAFrame = True;
        unsigned frameSize = fCurrentFrame.frameSize;
        if (frameSize > fMaxSize) {
            numTruncatedBytes = frameSize - fMaxSize;
            frameSize         = fMaxSize;
        } else {
            numTruncatedBytes = 0;
        }
        fTo[0] = fCurrentFrame.hdr0 >> 24;
        fTo[1] = fCurrentFrame.hdr0 >> 16;
        fTo[2] = fCurrentFrame.hdr0 >> 8;
        fTo[3] = fCurrentFrame.hdr0;
        getBytes(&fTo[4], frameSize - 4);
        skipBytes(numTruncatedBytes);
        return frameSize;
    } catch (int) {
        #ifdef DEBUG
        fUsingSource->envir()
            << "AC3AudioStreamParser::parseFrame() EXCEPTION (This is normal behavior - *not* an error)\n";
        #endif
        return 0;
    }
} // AC3AudioStreamParser::parseFrame

void AC3AudioStreamParser::readAndSaveAFrame()
{
    unsigned const maxAC3FrameSize = 4000;

    fSavedFrame     = new unsigned char[maxAC3FrameSize];
    fSavedFrameSize = 0;
    fSavedFrameFlag = 0;
    fUsingSource->getNextFrame(fSavedFrame, maxAC3FrameSize,
                               afterGettingSavedFrame, this,
                               onSavedFrameClosure, this);
    fUsingSource->envir().taskScheduler().doEventLoop(&fSavedFrameFlag);
}

void AC3AudioStreamParser
::afterGettingSavedFrame(void* clientData, unsigned frameSize,
                         unsigned,
                         struct timeval,
                         unsigned)
{
    AC3AudioStreamParser* parser = (AC3AudioStreamParser *) clientData;

    parser->afterGettingSavedFrame1(frameSize);
}

void AC3AudioStreamParser
::afterGettingSavedFrame1(unsigned frameSize)
{
    fSavedFrameSize = frameSize;
    fSavedFrameFlag = ~0;
}

void AC3AudioStreamParser::onSavedFrameClosure(void* clientData)
{
    AC3AudioStreamParser* parser = (AC3AudioStreamParser *) clientData;

    parser->onSavedFrameClosure1();
}

void AC3AudioStreamParser::onSavedFrameClosure1()
{
    delete[] fSavedFrame;
    fSavedFrame     = NULL;
    fSavedFrameSize = 0;
    fSavedFrameFlag = ~0;
}
