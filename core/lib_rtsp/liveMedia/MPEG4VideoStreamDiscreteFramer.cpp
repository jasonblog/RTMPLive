#include "MPEG4VideoStreamDiscreteFramer.hh"
MPEG4VideoStreamDiscreteFramer * MPEG4VideoStreamDiscreteFramer::createNew(UsageEnvironment& env,
                                                                           FramedSource*   inputSource,
                                                                           Boolean         leavePresentationTimesUnmodified)
{
    return new MPEG4VideoStreamDiscreteFramer(env, inputSource, leavePresentationTimesUnmodified);
}

MPEG4VideoStreamDiscreteFramer
::MPEG4VideoStreamDiscreteFramer(UsageEnvironment& env,
                                 FramedSource* inputSource, Boolean leavePresentationTimesUnmodified)
    : MPEG4VideoStreamFramer(env, inputSource, False),
    fLeavePresentationTimesUnmodified(leavePresentationTimesUnmodified), vop_time_increment_resolution(0), fNumVTIRBits(
        0),
    fLastNonBFrameVop_time_increment(0)
{
    fLastNonBFramePresentationTime.tv_sec  = 0;
    fLastNonBFramePresentationTime.tv_usec = 0;
}

MPEG4VideoStreamDiscreteFramer::~MPEG4VideoStreamDiscreteFramer()
{}

void MPEG4VideoStreamDiscreteFramer::doGetNextFrame()
{
    fInputSource->getNextFrame(fTo, fMaxSize,
                               afterGettingFrame, this,
                               FramedSource::handleClosure, this);
}

void MPEG4VideoStreamDiscreteFramer
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds)
{
    MPEG4VideoStreamDiscreteFramer* source = (MPEG4VideoStreamDiscreteFramer *) clientData;

    source->afterGettingFrame1(frameSize, numTruncatedBytes,
                               presentationTime, durationInMicroseconds);
}

void MPEG4VideoStreamDiscreteFramer
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                     struct timeval presentationTime,
                     unsigned durationInMicroseconds)
{
    if (frameSize >= 4 && fTo[0] == 0 && fTo[1] == 0 && fTo[2] == 1) {
        fPictureEndMarker = True;
        unsigned i = 3;
        if (fTo[i] == 0xB0) {
            if (frameSize >= 5) {
                fProfileAndLevelIndication = fTo[4];
            }
            for (i = 7; i < frameSize; ++i) {
                if ((fTo[i] == 0xB3 ||
                     fTo[i] == 0xB6) &&
                    fTo[i - 1] == 1 && fTo[i - 2] == 0 && fTo[i - 3] == 0)
                {
                    break;
                }
            }
            fNumConfigBytes = i < frameSize ? i - 3 : frameSize;
            delete[] fConfigBytes;
            fConfigBytes = new unsigned char[fNumConfigBytes];
            for (unsigned j = 0; j < fNumConfigBytes; ++j) {
                fConfigBytes[j] = fTo[j];
            }
            analyzeVOLHeader();
        }
        if (i < frameSize) {
            u_int8_t nextCode = fTo[i];
            if (nextCode == 0xB3) {
                for (i += 4; i < frameSize; ++i) {
                    if (fTo[i] == 0xB6 &&
                        fTo[i - 1] == 1 && fTo[i - 2] == 0 && fTo[i - 3] == 0)
                    {
                        nextCode = fTo[i];
                        break;
                    }
                }
            }
            if (nextCode == 0xB6 && i + 5 < frameSize) {
                ++i;
                u_int8_t nextByte        = fTo[i++];
                u_int8_t vop_coding_type = nextByte >> 6;
                u_int32_t next4Bytes     =
                    (fTo[i] << 24) | (fTo[i + 1] << 16) | (fTo[i + 2] << 8) | fTo[i + 3];
                i += 4;
                u_int32_t timeInfo        = (nextByte << (32 - 6)) | (next4Bytes >> 6);
                unsigned modulo_time_base = 0;
                u_int32_t mask = 0x80000000;
                while ((timeInfo & mask) != 0) {
                    ++modulo_time_base;
                    mask >>= 1;
                }
                mask >>= 2;
                unsigned vop_time_increment = 0;
                if ((mask >> (fNumVTIRBits - 1)) != 0) {
                    for (unsigned i = 0; i < fNumVTIRBits; ++i) {
                        vop_time_increment |= timeInfo & mask;
                        mask >>= 1;
                    }
                    while (mask != 0) {
                        vop_time_increment >>= 1;
                        mask >>= 1;
                    }
                }
                if (!fLeavePresentationTimesUnmodified && vop_coding_type == 2 &&
                    (fLastNonBFramePresentationTime.tv_usec > 0 ||
                     fLastNonBFramePresentationTime.tv_sec > 0))
                {
                    int timeIncrement =
                        fLastNonBFrameVop_time_increment - vop_time_increment;
                    if (timeIncrement < 0) {
                        timeIncrement += vop_time_increment_resolution;
                    }
                    unsigned const MILLION = 1000000;
                    double usIncrement     = vop_time_increment_resolution == 0 ? 0.0 :
                                             ((double) timeIncrement * MILLION) / vop_time_increment_resolution;
                    unsigned secondsToSubtract  = (unsigned) (usIncrement / MILLION);
                    unsigned uSecondsToSubtract = ((unsigned) usIncrement) % MILLION;
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
                    fLastNonBFrameVop_time_increment = vop_time_increment;
                }
            }
        }
    }
    fFrameSize              = frameSize;
    fNumTruncatedBytes      = numTruncatedBytes;
    fPresentationTime       = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    afterGetting(this);
} // MPEG4VideoStreamDiscreteFramer::afterGettingFrame1

Boolean MPEG4VideoStreamDiscreteFramer::getNextFrameBit(u_int8_t& result)
{
    if (fNumBitsSeenSoFar / 8 >= fNumConfigBytes) {
        return False;
    }
    u_int8_t nextByte = fConfigBytes[fNumBitsSeenSoFar / 8];
    result = (nextByte >> (7 - fNumBitsSeenSoFar % 8)) & 1;
    ++fNumBitsSeenSoFar;
    return True;
}

Boolean MPEG4VideoStreamDiscreteFramer::getNextFrameBits(unsigned numBits,
                                                         u_int32_t& result)
{
    result = 0;
    for (unsigned i = 0; i < numBits; ++i) {
        u_int8_t nextBit;
        if (!getNextFrameBit(nextBit)) {
            return False;
        }
        result = (result << 1) | nextBit;
    }
    return True;
}

void MPEG4VideoStreamDiscreteFramer::analyzeVOLHeader()
{
    unsigned i;

    for (i = 3; i < fNumConfigBytes; ++i) {
        if (fConfigBytes[i] >= 0x20 && fConfigBytes[i] <= 0x2F &&
            fConfigBytes[i - 1] == 1 &&
            fConfigBytes[i - 2] == 0 && fConfigBytes[i - 3] == 0)
        {
            ++i;
            break;
        }
    }
    fNumBitsSeenSoFar = 8 * i + 9;
    do {
        u_int8_t is_object_layer_identifier;
        if (!getNextFrameBit(is_object_layer_identifier)) {
            break;
        }
        if (is_object_layer_identifier) {
            fNumBitsSeenSoFar += 7;
        }
        u_int32_t aspect_ratio_info;
        if (!getNextFrameBits(4, aspect_ratio_info)) {
            break;
        }
        if (aspect_ratio_info == 15) {
            fNumBitsSeenSoFar += 16;
        }
        u_int8_t vol_control_parameters;
        if (!getNextFrameBit(vol_control_parameters)) {
            break;
        }
        if (vol_control_parameters) {
            fNumBitsSeenSoFar += 3;
            u_int8_t vbw_parameters;
            if (!getNextFrameBit(vbw_parameters)) {
                break;
            }
            if (vbw_parameters) {
                fNumBitsSeenSoFar += 79;
            }
        }
        fNumBitsSeenSoFar += 2;
        u_int8_t marker_bit;
        if (!getNextFrameBit(marker_bit)) {
            break;
        }
        if (marker_bit != 1) {
            break;
        }
        if (!getNextFrameBits(16, vop_time_increment_resolution)) {
            break;
        }
        if (vop_time_increment_resolution == 0) {
            break;
        }
        fNumVTIRBits = 0;
        for (unsigned test = vop_time_increment_resolution; test > 0; test /= 2) {
            ++fNumVTIRBits;
        }
    } while (0);
} // MPEG4VideoStreamDiscreteFramer::analyzeVOLHeader
