#include "MPEGVideoStreamParser.hh"
#include <GroupsockHelper.hh>
TimeCode::TimeCode()
    : days(0), hours(0), minutes(0), seconds(0), pictures(0)
{}

TimeCode::~TimeCode()
{}

int TimeCode::operator == (TimeCode const& arg2)
{
    return pictures == arg2.pictures && seconds == arg2.seconds &&
           minutes == arg2.minutes && hours == arg2.hours && days == arg2.days;
}

MPEGVideoStreamFramer::MPEGVideoStreamFramer(UsageEnvironment& env,
                                             FramedSource*   inputSource)
    : FramedFilter(env, inputSource),
    fFrameRate(0.0),
    fParser(NULL)
{
    reset();
}

MPEGVideoStreamFramer::~MPEGVideoStreamFramer()
{
    delete fParser;
}

void MPEGVideoStreamFramer::flushInput()
{
    reset();
    if (fParser != NULL) {
        fParser->flushInput();
    }
}

void MPEGVideoStreamFramer::reset()
{
    fPictureCount          = 0;
    fPictureEndMarker      = False;
    fPicturesAdjustment    = 0;
    fPictureTimeBase       = 0.0;
    fTcSecsBase            = 0;
    fHaveSeenFirstTimeCode = False;
    gettimeofday(&fPresentationTimeBase, NULL);
}

#ifdef DEBUG
static struct timeval firstPT;
#endif
void MPEGVideoStreamFramer
::computePresentationTime(unsigned numAdditionalPictures)
{
    TimeCode& tc    = fCurGOPTimeCode;
    unsigned tcSecs =
        (((tc.days * 24) + tc.hours) * 60 + tc.minutes) * 60 + tc.seconds - fTcSecsBase;
    double pictureTime = fFrameRate == 0.0 ? 0.0 :
                         (tc.pictures + fPicturesAdjustment + numAdditionalPictures) / fFrameRate;

    while (pictureTime < fPictureTimeBase) {
        if (tcSecs > 0) {
            tcSecs -= 1;
        }
        pictureTime += 1.0;
    }
    pictureTime -= fPictureTimeBase;
    if (pictureTime < 0.0) {
        pictureTime = 0.0;
    }
    unsigned pictureSeconds        = (unsigned) pictureTime;
    double pictureFractionOfSecond = pictureTime - (double) pictureSeconds;
    fPresentationTime          = fPresentationTimeBase;
    fPresentationTime.tv_sec  += tcSecs + pictureSeconds;
    fPresentationTime.tv_usec += (long) (pictureFractionOfSecond * 1000000.0);
    if (fPresentationTime.tv_usec >= 1000000) {
        fPresentationTime.tv_usec -= 1000000;
        ++fPresentationTime.tv_sec;
    }
    #ifdef DEBUG
    if (firstPT.tv_sec == 0 && firstPT.tv_usec == 0) {
        firstPT = fPresentationTime;
    }
    struct timeval diffPT;
    diffPT.tv_sec  = fPresentationTime.tv_sec - firstPT.tv_sec;
    diffPT.tv_usec = fPresentationTime.tv_usec - firstPT.tv_usec;
    if (fPresentationTime.tv_usec < firstPT.tv_usec) {
        --diffPT.tv_sec;
        diffPT.tv_usec += 1000000;
    }
    fprintf(stderr, "MPEGVideoStreamFramer::computePresentationTime(%d) -> %lu.%06ld [%lu.%06ld]\n",
            numAdditionalPictures, fPresentationTime.tv_sec, fPresentationTime.tv_usec, diffPT.tv_sec, diffPT.tv_usec);
    #endif // ifdef DEBUG
} // MPEGVideoStreamFramer::computePresentationTime

void MPEGVideoStreamFramer
::setTimeCode(unsigned hours, unsigned minutes, unsigned seconds,
              unsigned pictures, unsigned picturesSinceLastGOP)
{
    TimeCode& tc  = fCurGOPTimeCode;
    unsigned days = tc.days;

    if (hours < tc.hours) {
        ++days;
    }
    tc.days     = days;
    tc.hours    = hours;
    tc.minutes  = minutes;
    tc.seconds  = seconds;
    tc.pictures = pictures;
    if (!fHaveSeenFirstTimeCode) {
        fPictureTimeBase       = fFrameRate == 0.0 ? 0.0 : tc.pictures / fFrameRate;
        fTcSecsBase            = (((tc.days * 24) + tc.hours) * 60 + tc.minutes) * 60 + tc.seconds;
        fHaveSeenFirstTimeCode = True;
    } else if (fCurGOPTimeCode == fPrevGOPTimeCode) {
        fPicturesAdjustment += picturesSinceLastGOP;
    } else {
        fPrevGOPTimeCode    = tc;
        fPicturesAdjustment = 0;
    }
}

void MPEGVideoStreamFramer::doGetNextFrame()
{
    fParser->registerReadInterest(fTo, fMaxSize);
    continueReadProcessing();
}

void MPEGVideoStreamFramer
::continueReadProcessing(void* clientData,
                         unsigned char *, unsigned,
                         struct timeval)
{
    MPEGVideoStreamFramer* framer = (MPEGVideoStreamFramer *) clientData;

    framer->continueReadProcessing();
}

void MPEGVideoStreamFramer::continueReadProcessing()
{
    unsigned acquiredFrameSize = fParser->parse();

    if (acquiredFrameSize > 0) {
        fFrameSize              = acquiredFrameSize;
        fNumTruncatedBytes      = fParser->numTruncatedBytes();
        fDurationInMicroseconds =
            (fFrameRate == 0.0 || ((int) fPictureCount) < 0) ? 0 :
            (unsigned) ((fPictureCount * 1000000) / fFrameRate);
        #ifdef DEBUG
        fprintf(stderr, "%d bytes @%u.%06d, fDurationInMicroseconds: %d ((%d*1000000)/%f)\n", acquiredFrameSize,
                fPresentationTime.tv_sec, fPresentationTime.tv_usec, fDurationInMicroseconds, fPictureCount,
                fFrameRate);
        #endif
        fPictureCount = 0;
        afterGetting(this);
    } else {}
}
