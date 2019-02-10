#include "MPEG1or2VideoStreamDiscreteFramer.hh"
MPEG1or2VideoStreamDiscreteFramer * MPEG1or2VideoStreamDiscreteFramer::createNew(UsageEnvironment& env,
                                                                                 FramedSource*   inputSource,
                                                                                 Boolean         iFramesOnly,
                                                                                 double          vshPeriod,
                                                                                 Boolean         leavePresentationTimesUnmodified)
{
    return new MPEG1or2VideoStreamDiscreteFramer(env, inputSource,
                                                 iFramesOnly, vshPeriod, leavePresentationTimesUnmodified);
}

MPEG1or2VideoStreamDiscreteFramer
::MPEG1or2VideoStreamDiscreteFramer(UsageEnvironment& env,
                                    FramedSource* inputSource,
                                    Boolean iFramesOnly, double vshPeriod, Boolean leavePresentationTimesUnmodified)
    : MPEG1or2VideoStreamFramer(env, inputSource, iFramesOnly, vshPeriod,
                                False),
    fLeavePresentationTimesUnmodified(leavePresentationTimesUnmodified),
    fLastNonBFrameTemporal_reference(0),
    fSavedVSHSize(0), fSavedVSHTimestamp(0.0),
    fIFramesOnly(iFramesOnly), fVSHPeriod(vshPeriod)
{
    fLastNonBFramePresentationTime.tv_sec  = 0;
    fLastNonBFramePresentationTime.tv_usec = 0;
}

MPEG1or2VideoStreamDiscreteFramer::~MPEG1or2VideoStreamDiscreteFramer()
{}

void MPEG1or2VideoStreamDiscreteFramer::doGetNextFrame()
{
    fInputSource->getNextFrame(fTo, fMaxSize,
                               afterGettingFrame, this,
                               FramedSource::handleClosure, this);
}

void MPEG1or2VideoStreamDiscreteFramer
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds)
{
    MPEG1or2VideoStreamDiscreteFramer* source =
        (MPEG1or2VideoStreamDiscreteFramer *) clientData;

    source->afterGettingFrame1(frameSize, numTruncatedBytes,
                               presentationTime, durationInMicroseconds);
}

static double const frameRateFromCode[] = {
    0.0,
    24000 / 1001.0,
    24.0,
    25.0,
    30000 / 1001.0,
    30.0,
    50.0,
    60000 / 1001.0,
    60.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0
};
#define MILLION 1000000
void MPEG1or2VideoStreamDiscreteFramer
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                     struct timeval presentationTime,
                     unsigned durationInMicroseconds)
{
    if (frameSize >= 4 && fTo[0] == 0 && fTo[1] == 0 && fTo[2] == 1) {
        fPictureEndMarker = True;
        u_int8_t nextCode = fTo[3];
        if (nextCode == 0xB3) {
            if (frameSize >= 8) {
                u_int8_t frame_rate_code = fTo[7] & 0x0F;
                fFrameRate = frameRateFromCode[frame_rate_code];
            }
            unsigned vshSize;
            for (vshSize = 4; vshSize < frameSize - 3; ++vshSize) {
                if (fTo[vshSize] == 0 && fTo[vshSize + 1] == 0 && fTo[vshSize + 2] == 1 &&
                    (fTo[vshSize + 3] == 0xB8 || fTo[vshSize + 3] == 0x00))
                {
                    break;
                }
            }
            if (vshSize == frameSize - 3) {
                vshSize = frameSize;
            }
            if (vshSize <= sizeof fSavedVSHBuffer) {
                memmove(fSavedVSHBuffer, fTo, vshSize);
                fSavedVSHSize      = vshSize;
                fSavedVSHTimestamp =
                    presentationTime.tv_sec + presentationTime.tv_usec / (double) MILLION;
            }
        } else if (nextCode == 0xB8) {
            double pts = presentationTime.tv_sec + presentationTime.tv_usec / (double) MILLION;
            if (pts > fSavedVSHTimestamp + fVSHPeriod &&
                fSavedVSHSize + frameSize <= fMaxSize)
            {
                memmove(&fTo[fSavedVSHSize], &fTo[0], frameSize);
                memmove(&fTo[0], fSavedVSHBuffer, fSavedVSHSize);
                frameSize += fSavedVSHSize;
                fSavedVSHTimestamp = pts;
            }
        }
        unsigned i = 3;
        if (nextCode == 0xB3 ||
            nextCode == 0xB8)
        {
            for (i += 4; i < frameSize; ++i) {
                if (fTo[i] == 0x00 &&
                    fTo[i - 1] == 1 && fTo[i - 2] == 0 && fTo[i - 3] == 0)
                {
                    nextCode = fTo[i];
                    break;
                }
            }
        }
        if (nextCode == 0x00 && i + 2 < frameSize) {
            ++i;
            unsigned short temporal_reference = (fTo[i] << 2) | (fTo[i + 1] >> 6);
            unsigned char picture_coding_type = (fTo[i + 1] & 0x38) >> 3;
            if (fIFramesOnly && picture_coding_type != 1) {
                doGetNextFrame();
                return;
            }
            if (!fLeavePresentationTimesUnmodified && picture_coding_type == 3 &&
                (fLastNonBFramePresentationTime.tv_usec > 0 ||
                 fLastNonBFramePresentationTime.tv_sec > 0))
            {
                int trIncrement =
                    fLastNonBFrameTemporal_reference - temporal_reference;
                if (trIncrement < 0) {
                    trIncrement += 1024;
                }
                unsigned usIncrement = fFrameRate == 0.0 ? 0 :
                                       (unsigned) ((trIncrement * MILLION) / fFrameRate);
                unsigned secondsToSubtract  = usIncrement / MILLION;
                unsigned uSecondsToSubtract = usIncrement % MILLION;
                presentationTime = fLastNonBFramePresentationTime;
                if ((unsigned) presentationTime.tv_usec < uSecondsToSubtract) {
                    presentationTime.tv_usec += MILLION;
                    if (presentationTime.tv_sec > 0) {
                        --presentationTime.tv_sec;
                    }
                }
                presentationTime.tv_usec -= uSecondsToSubtract;
                if ((unsigned) presentationTime.tv_sec > secondsToSubtract) {
                    presentationTime.tv_sec -= secondsToSubtract;
                } else {
                    presentationTime.tv_sec = presentationTime.tv_usec = 0;
                }
            } else {
                fLastNonBFramePresentationTime   = presentationTime;
                fLastNonBFrameTemporal_reference = temporal_reference;
            }
        }
    }
    fFrameSize              = frameSize;
    fNumTruncatedBytes      = numTruncatedBytes;
    fPresentationTime       = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    afterGetting(this);
} // MPEG1or2VideoStreamDiscreteFramer::afterGettingFrame1
