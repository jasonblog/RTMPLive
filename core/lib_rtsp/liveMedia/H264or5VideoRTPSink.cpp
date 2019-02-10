#include "H264or5VideoRTPSink.hh"
#include "H264or5VideoStreamFramer.hh"
class H264or5Fragmenter : public FramedFilter
{
public:
    H264or5Fragmenter(int hNumber, UsageEnvironment& env, FramedSource* inputSource,
                      unsigned inputBufferMax, unsigned maxOutputPacketSize);
    virtual ~H264or5Fragmenter();
    Boolean lastFragmentCompletedNALUnit() const
    {
        return fLastFragmentCompletedNALUnit;
    }

private:
    virtual void doGetNextFrame();
private:
    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);
    void afterGettingFrame1(unsigned       frameSize,
                            unsigned       numTruncatedBytes,
                            struct timeval presentationTime,
                            unsigned       durationInMicroseconds);
private:
    int fHNumber;
    unsigned fInputBufferSize;
    unsigned fMaxOutputPacketSize;
    unsigned char* fInputBuffer;
    unsigned fNumValidDataBytes;
    unsigned fCurDataOffset;
    unsigned fSaveNumTruncatedBytes;
    Boolean fLastFragmentCompletedNALUnit;
};
H264or5VideoRTPSink
::H264or5VideoRTPSink(int hNumber,
                      UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat,
                      u_int8_t const* vps, unsigned vpsSize,
                      u_int8_t const* sps, unsigned spsSize,
                      u_int8_t const* pps, unsigned ppsSize)
    : VideoRTPSink(env, RTPgs, rtpPayloadFormat, 90000, hNumber == 264 ? "H264" : "H265"),
    fHNumber(hNumber), fOurFragmenter(NULL), fFmtpSDPLine(NULL)
{
    if (vps != NULL) {
        fVPSSize = vpsSize;
        fVPS     = new u_int8_t[fVPSSize];
        memmove(fVPS, vps, fVPSSize);
    } else {
        fVPSSize = 0;
        fVPS     = NULL;
    }
    if (sps != NULL) {
        fSPSSize = spsSize;
        fSPS     = new u_int8_t[fSPSSize];
        memmove(fSPS, sps, fSPSSize);
    } else {
        fSPSSize = 0;
        fSPS     = NULL;
    }
    if (pps != NULL) {
        fPPSSize = ppsSize;
        fPPS     = new u_int8_t[fPPSSize];
        memmove(fPPS, pps, fPPSSize);
    } else {
        fPPSSize = 0;
        fPPS     = NULL;
    }
}

H264or5VideoRTPSink::~H264or5VideoRTPSink()
{
    fSource = fOurFragmenter;
    delete[] fFmtpSDPLine;
    delete[] fVPS;
    delete[] fSPS;
    delete[] fPPS;
    stopPlaying();
    Medium::close(fOurFragmenter);
    fSource = NULL;
}

Boolean H264or5VideoRTPSink::continuePlaying()
{
    if (fOurFragmenter == NULL) {
        fOurFragmenter = new H264or5Fragmenter(fHNumber, envir(), fSource, OutPacketBuffer::maxSize,
                                               ourMaxPacketSize() - 12);
    } else {
        fOurFragmenter->reassignInputSource(fSource);
    }
    fSource = fOurFragmenter;
    return MultiFramedRTPSink::continuePlaying();
}

void H264or5VideoRTPSink::doSpecialFrameHandling(unsigned,
                                                 unsigned char *,
                                                 unsigned,
                                                 struct timeval framePresentationTime,
                                                 unsigned)
{
    if (fOurFragmenter != NULL) {
        H264or5VideoStreamFramer* framerSource =
            (H264or5VideoStreamFramer *) (fOurFragmenter->inputSource());
        if (((H264or5Fragmenter *) fOurFragmenter)->lastFragmentCompletedNALUnit() &&
            framerSource != NULL && framerSource->pictureEndMarker())
        {
            setMarkerBit();
            framerSource->pictureEndMarker() = False;
        }
    }
    setTimestamp(framePresentationTime);
}

Boolean H264or5VideoRTPSink
::frameCanAppearAfterPacketStart(unsigned char const *,
                                 unsigned) const
{
    return False;
}

H264or5Fragmenter::H264or5Fragmenter(int hNumber,
                                     UsageEnvironment& env, FramedSource* inputSource,
                                     unsigned inputBufferMax, unsigned maxOutputPacketSize)
    : FramedFilter(env, inputSource),
    fHNumber(hNumber),
    fInputBufferSize(inputBufferMax + 1), fMaxOutputPacketSize(maxOutputPacketSize),
    fNumValidDataBytes(1), fCurDataOffset(1), fSaveNumTruncatedBytes(0),
    fLastFragmentCompletedNALUnit(True)
{
    fInputBuffer = new unsigned char[fInputBufferSize];
}

H264or5Fragmenter::~H264or5Fragmenter()
{
    delete[] fInputBuffer;
    detachInputSource();
}

void H264or5Fragmenter::doGetNextFrame()
{
    if (fNumValidDataBytes == 1) {
        fInputSource->getNextFrame(&fInputBuffer[1], fInputBufferSize - 1,
                                   afterGettingFrame, this,
                                   FramedSource::handleClosure, this);
    } else {
        if (fMaxSize < fMaxOutputPacketSize) {
            envir() << "H264or5Fragmenter::doGetNextFrame(): fMaxSize ("
                    << fMaxSize << ") is smaller than expected\n";
        } else {
            fMaxSize = fMaxOutputPacketSize;
        }
        fLastFragmentCompletedNALUnit = True;
        if (fCurDataOffset == 1) {
            if (fNumValidDataBytes - 1 <= fMaxSize) {
                memmove(fTo, &fInputBuffer[1], fNumValidDataBytes - 1);
                fFrameSize     = fNumValidDataBytes - 1;
                fCurDataOffset = fNumValidDataBytes;
            } else {
                if (fHNumber == 264) {
                    fInputBuffer[0] = (fInputBuffer[1] & 0xE0) | 28;
                    fInputBuffer[1] = 0x80 | (fInputBuffer[1] & 0x1F);
                } else {
                    u_int8_t nal_unit_type = (fInputBuffer[1] & 0x7E) >> 1;
                    fInputBuffer[0] = (fInputBuffer[1] & 0x81) | (49 << 1);
                    fInputBuffer[1] = fInputBuffer[2];
                    fInputBuffer[2] = 0x80 | nal_unit_type;
                }
                memmove(fTo, fInputBuffer, fMaxSize);
                fFrameSize      = fMaxSize;
                fCurDataOffset += fMaxSize - 1;
                fLastFragmentCompletedNALUnit = False;
            }
        } else {
            unsigned numExtraHeaderBytes;
            if (fHNumber == 264) {
                fInputBuffer[fCurDataOffset - 2] = fInputBuffer[0];
                fInputBuffer[fCurDataOffset - 1] = fInputBuffer[1] & ~0x80;
                numExtraHeaderBytes = 2;
            } else {
                fInputBuffer[fCurDataOffset - 3] = fInputBuffer[0];
                fInputBuffer[fCurDataOffset - 2] = fInputBuffer[1];
                fInputBuffer[fCurDataOffset - 1] = fInputBuffer[2] & ~0x80;
                numExtraHeaderBytes = 3;
            }
            unsigned numBytesToSend = numExtraHeaderBytes + (fNumValidDataBytes - fCurDataOffset);
            if (numBytesToSend > fMaxSize) {
                numBytesToSend = fMaxSize;
                fLastFragmentCompletedNALUnit = False;
            } else {
                fInputBuffer[fCurDataOffset - 1] |= 0x40;
                fNumTruncatedBytes = fSaveNumTruncatedBytes;
            }
            memmove(fTo, &fInputBuffer[fCurDataOffset - numExtraHeaderBytes], numBytesToSend);
            fFrameSize      = numBytesToSend;
            fCurDataOffset += numBytesToSend - numExtraHeaderBytes;
        }
        if (fCurDataOffset >= fNumValidDataBytes) {
            fNumValidDataBytes = fCurDataOffset = 1;
        }
        FramedSource::afterGetting(this);
    }
} // H264or5Fragmenter::doGetNextFrame

void H264or5Fragmenter::afterGettingFrame(void* clientData, unsigned frameSize,
                                          unsigned numTruncatedBytes,
                                          struct timeval presentationTime,
                                          unsigned durationInMicroseconds)
{
    H264or5Fragmenter* fragmenter = (H264or5Fragmenter *) clientData;

    fragmenter->afterGettingFrame1(frameSize, numTruncatedBytes, presentationTime,
                                   durationInMicroseconds);
}

void H264or5Fragmenter::afterGettingFrame1(unsigned       frameSize,
                                           unsigned       numTruncatedBytes,
                                           struct timeval presentationTime,
                                           unsigned       durationInMicroseconds)
{
    fNumValidDataBytes     += frameSize;
    fSaveNumTruncatedBytes  = numTruncatedBytes;
    fPresentationTime       = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    doGetNextFrame();
}
