#include "H263plusVideoStreamFramer.hh"
#include "H263plusVideoStreamParser.hh"
#include <string.h>
#include <GroupsockHelper.hh>
H263plusVideoStreamFramer * H263plusVideoStreamFramer::createNew(
    UsageEnvironment& env,
    FramedSource*   inputSource)
{
    H263plusVideoStreamFramer* fr;

    fr = new H263plusVideoStreamFramer(env, inputSource);
    return fr;
}

H263plusVideoStreamFramer::H263plusVideoStreamFramer(
    UsageEnvironment& env,
    FramedSource*   inputSource,
    Boolean         createParser)
    : FramedFilter(env, inputSource),
    fFrameRate(0.0),
    fPictureEndMarker(False)
{
    gettimeofday(&fPresentationTimeBase, NULL);
    fParser = createParser ? new H263plusVideoStreamParser(this, inputSource) : NULL;
}

H263plusVideoStreamFramer::~H263plusVideoStreamFramer()
{
    delete   fParser;
}

void H263plusVideoStreamFramer::doGetNextFrame()
{
    fParser->registerReadInterest(fTo, fMaxSize);
    continueReadProcessing();
}

Boolean H263plusVideoStreamFramer::isH263plusVideoStreamFramer() const
{
    return True;
}

void H263plusVideoStreamFramer::continueReadProcessing(
    void* clientData,
    unsigned char *, unsigned,
    struct timeval)
{
    H263plusVideoStreamFramer* framer = (H263plusVideoStreamFramer *) clientData;

    framer->continueReadProcessing();
}

void H263plusVideoStreamFramer::continueReadProcessing()
{
    unsigned acquiredFrameSize;
    u_int64_t frameDuration;

    acquiredFrameSize = fParser->parse(frameDuration);
    if (acquiredFrameSize > 0) {
        fFrameSize = acquiredFrameSize;
        fFrameRate = frameDuration == 0 ? 0.0 : 1000. / (long) frameDuration;
        if (acquiredFrameSize == 5) {
            fPresentationTime = fPresentationTimeBase;
        } else {
            fPresentationTime.tv_usec += (long) frameDuration * 1000;
        }
        while (fPresentationTime.tv_usec >= 1000000) {
            fPresentationTime.tv_usec -= 1000000;
            ++fPresentationTime.tv_sec;
        }
        fDurationInMicroseconds = (unsigned int) frameDuration * 1000;
        ;
        afterGetting(this);
    } else {}
}
