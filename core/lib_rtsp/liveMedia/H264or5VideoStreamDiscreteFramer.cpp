#include "H264or5VideoStreamDiscreteFramer.hh"
H264or5VideoStreamDiscreteFramer
::H264or5VideoStreamDiscreteFramer(int hNumber, UsageEnvironment& env, FramedSource* inputSource)
    : H264or5VideoStreamFramer(hNumber, env, inputSource, False, False)
{}

H264or5VideoStreamDiscreteFramer::~H264or5VideoStreamDiscreteFramer()
{}

void H264or5VideoStreamDiscreteFramer::doGetNextFrame()
{
    fInputSource->getNextFrame(fTo, fMaxSize,
                               afterGettingFrame, this,
                               FramedSource::handleClosure, this);
}

void H264or5VideoStreamDiscreteFramer
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds)
{
    H264or5VideoStreamDiscreteFramer* source = (H264or5VideoStreamDiscreteFramer *) clientData;

    source->afterGettingFrame1(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void H264or5VideoStreamDiscreteFramer
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                     struct timeval presentationTime,
                     unsigned durationInMicroseconds)
{
    u_int8_t nal_unit_type;

    if (fHNumber == 264 && frameSize >= 1) {
        nal_unit_type = fTo[0] & 0x1F;
    } else if (fHNumber == 265 && frameSize >= 2) {
        nal_unit_type = (fTo[0] & 0x7E) >> 1;
    } else {
        nal_unit_type = 0xFF;
    }
    if (frameSize >= 4 && fTo[0] == 0 && fTo[1] == 0 && ((fTo[2] == 0 && fTo[3] == 1) || fTo[2] == 1)) {
        envir() << "H264or5VideoStreamDiscreteFramer error: MPEG 'start code' seen in the input\n";
    } else if (isVPS(nal_unit_type)) {
        saveCopyOfVPS(fTo, frameSize);
    } else if (isSPS(nal_unit_type)) {
        saveCopyOfSPS(fTo, frameSize);
    } else if (isPPS(nal_unit_type)) {
        saveCopyOfPPS(fTo, frameSize);
    }
    if (isVCL(nal_unit_type)) {
        fPictureEndMarker = True;
    }
    fFrameSize              = frameSize;
    fNumTruncatedBytes      = numTruncatedBytes;
    fPresentationTime       = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    afterGetting(this);
}
