#include "MultiFramedRTPSink.hh"
#include "GroupsockHelper.hh"
void MultiFramedRTPSink::setPacketSizes(unsigned preferredPacketSize,
                                        unsigned maxPacketSize)
{
    if (preferredPacketSize > maxPacketSize || preferredPacketSize == 0) {
        return;
    }
    delete fOutBuf;
    fOutBuf = new OutPacketBuffer(preferredPacketSize, maxPacketSize);
    fOurMaxPacketSize = maxPacketSize;
}

MultiFramedRTPSink::MultiFramedRTPSink(UsageEnvironment& env,
                                       Groupsock*      rtpGS,
                                       unsigned char   rtpPayloadType,
                                       unsigned        rtpTimestampFrequency,
                                       char const*     rtpPayloadFormatName,
                                       unsigned        numChannels)
    : RTPSink(env, rtpGS, rtpPayloadType, rtpTimestampFrequency,
              rtpPayloadFormatName, numChannels),
    fOutBuf(NULL), fCurFragmentationOffset(0), fPreviousFrameEndedFragmentation(False),
    fOnSendErrorFunc(NULL), fOnSendErrorData(NULL)
{
    setPacketSizes(1000, 1448);
}

MultiFramedRTPSink::~MultiFramedRTPSink()
{
    delete fOutBuf;
}

void MultiFramedRTPSink
::doSpecialFrameHandling(unsigned,
                         unsigned char *,
                         unsigned,
                         struct timeval framePresentationTime,
                         unsigned)
{
    if (isFirstFrameInPacket()) {
        setTimestamp(framePresentationTime);
    }
}

Boolean MultiFramedRTPSink::allowFragmentationAfterStart() const
{
    return False;
}

Boolean MultiFramedRTPSink::allowOtherFramesAfterLastFragment() const
{
    return False;
}

Boolean MultiFramedRTPSink
::frameCanAppearAfterPacketStart(unsigned char const *,
                                 unsigned) const
{
    return True;
}

unsigned MultiFramedRTPSink::specialHeaderSize() const
{
    return 0;
}

unsigned MultiFramedRTPSink::frameSpecificHeaderSize() const
{
    return 0;
}

unsigned MultiFramedRTPSink::computeOverflowForNewFrame(unsigned newFrameSize) const
{
    return fOutBuf->numOverflowBytes(newFrameSize);
}

void MultiFramedRTPSink::setMarkerBit()
{
    unsigned rtpHdr = fOutBuf->extractWord(0);

    rtpHdr |= 0x00800000;
    fOutBuf->insertWord(rtpHdr, 0);
}

void MultiFramedRTPSink::setTimestamp(struct timeval framePresentationTime)
{
    fCurrentTimestamp = convertToRTPTimestamp(framePresentationTime);
    fOutBuf->insertWord(fCurrentTimestamp, fTimestampPosition);
}

void MultiFramedRTPSink::setSpecialHeaderWord(unsigned word,
                                              unsigned wordPosition)
{
    fOutBuf->insertWord(word, fSpecialHeaderPosition + 4 * wordPosition);
}

void MultiFramedRTPSink::setSpecialHeaderBytes(unsigned char const* bytes,
                                               unsigned             numBytes,
                                               unsigned             bytePosition)
{
    fOutBuf->insert(bytes, numBytes, fSpecialHeaderPosition + bytePosition);
}

void MultiFramedRTPSink::setFrameSpecificHeaderWord(unsigned word,
                                                    unsigned wordPosition)
{
    fOutBuf->insertWord(word, fCurFrameSpecificHeaderPosition + 4 * wordPosition);
}

void MultiFramedRTPSink::setFrameSpecificHeaderBytes(unsigned char const* bytes,
                                                     unsigned             numBytes,
                                                     unsigned             bytePosition)
{
    fOutBuf->insert(bytes, numBytes, fCurFrameSpecificHeaderPosition + bytePosition);
}

void MultiFramedRTPSink::setFramePadding(unsigned numPaddingBytes)
{
    if (numPaddingBytes > 0) {
        unsigned char paddingBuffer[255];
        memset(paddingBuffer, 0, numPaddingBytes);
        paddingBuffer[numPaddingBytes - 1] = numPaddingBytes;
        fOutBuf->enqueue(paddingBuffer, numPaddingBytes);
        unsigned rtpHdr = fOutBuf->extractWord(0);
        rtpHdr |= 0x20000000;
        fOutBuf->insertWord(rtpHdr, 0);
    }
}

Boolean MultiFramedRTPSink::continuePlaying()
{
    buildAndSendPacket(True);
    return True;
}

void MultiFramedRTPSink::stopPlaying()
{
    fOutBuf->resetPacketStart();
    fOutBuf->resetOffset();
    fOutBuf->resetOverflowData();
    MediaSink::stopPlaying();
}

void MultiFramedRTPSink::buildAndSendPacket(Boolean isFirstPacket)
{
    fIsFirstPacket = isFirstPacket;
    unsigned rtpHdr = 0x80000000;
    rtpHdr |= (fRTPPayloadType << 16);
    rtpHdr |= fSeqNo;
    fOutBuf->enqueueWord(rtpHdr);
    fTimestampPosition = fOutBuf->curPacketSize();
    fOutBuf->skipBytes(4);
    fOutBuf->enqueueWord(SSRC());
    fSpecialHeaderPosition = fOutBuf->curPacketSize();
    fSpecialHeaderSize     = specialHeaderSize();
    fOutBuf->skipBytes(fSpecialHeaderSize);
    fTotalFrameSpecificHeaderSizes = 0;
    fNoFramesLeft       = False;
    fNumFramesUsedSoFar = 0;
    packFrame();
}

void MultiFramedRTPSink::packFrame()
{
    if (fOutBuf->haveOverflowData()) {
        unsigned frameSize = fOutBuf->overflowDataSize();
        struct timeval presentationTime = fOutBuf->overflowPresentationTime();
        unsigned durationInMicroseconds = fOutBuf->overflowDurationInMicroseconds();
        fOutBuf->useOverflowData();
        afterGettingFrame1(frameSize, 0, presentationTime, durationInMicroseconds);
    } else {
        if (fSource == NULL) {
            return;
        }
        fCurFrameSpecificHeaderPosition = fOutBuf->curPacketSize();
        fCurFrameSpecificHeaderSize     = frameSpecificHeaderSize();
        fOutBuf->skipBytes(fCurFrameSpecificHeaderSize);
        fTotalFrameSpecificHeaderSizes += fCurFrameSpecificHeaderSize;
        fSource->getNextFrame(fOutBuf->curPtr(), fOutBuf->totalBytesAvailable(),
                              afterGettingFrame, this, ourHandleClosure, this);
    }
}

void MultiFramedRTPSink
::afterGettingFrame(void* clientData, unsigned numBytesRead,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds)
{
    MultiFramedRTPSink* sink = (MultiFramedRTPSink *) clientData;

    sink->afterGettingFrame1(numBytesRead, numTruncatedBytes,
                             presentationTime, durationInMicroseconds);
}

void MultiFramedRTPSink
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                     struct timeval presentationTime,
                     unsigned durationInMicroseconds)
{
    if (fIsFirstPacket) {
        gettimeofday(&fNextSendTime, NULL);
    }
    fMostRecentPresentationTime = presentationTime;
    if (fInitialPresentationTime.tv_sec == 0 && fInitialPresentationTime.tv_usec == 0) {
        fInitialPresentationTime = presentationTime;
    }
    if (numTruncatedBytes > 0) {
        unsigned const bufferSize = fOutBuf->totalBytesAvailable();
        envir() << "MultiFramedRTPSink::afterGettingFrame1(): The input frame data was too large for our buffer size ("
                << bufferSize << ").  "
                << numTruncatedBytes
                <<
            " bytes of trailing data was dropped!  Correct this by increasing \"OutPacketBuffer::maxSize\" to at least "
                << OutPacketBuffer::maxSize + numTruncatedBytes
                << ", *before* creating this 'RTPSink'.  (Current value is "
                << OutPacketBuffer::maxSize << ".)\n";
    }
    unsigned curFragmentationOffset = fCurFragmentationOffset;
    unsigned numFrameBytesToUse     = frameSize;
    unsigned overflowBytes = 0;
    if (fNumFramesUsedSoFar > 0) {
        if ((fPreviousFrameEndedFragmentation &&
             !allowOtherFramesAfterLastFragment()) ||
            !frameCanAppearAfterPacketStart(fOutBuf->curPtr(), frameSize))
        {
            numFrameBytesToUse = 0;
            fOutBuf->setOverflowData(fOutBuf->curPacketSize(), frameSize,
                                     presentationTime, durationInMicroseconds);
        }
    }
    fPreviousFrameEndedFragmentation = False;
    if (numFrameBytesToUse > 0) {
        if (fOutBuf->wouldOverflow(frameSize)) {
            if (isTooBigForAPacket(frameSize) &&
                (fNumFramesUsedSoFar == 0 || allowFragmentationAfterStart()))
            {
                overflowBytes            = computeOverflowForNewFrame(frameSize);
                numFrameBytesToUse      -= overflowBytes;
                fCurFragmentationOffset += numFrameBytesToUse;
            } else {
                overflowBytes      = frameSize;
                numFrameBytesToUse = 0;
            }
            fOutBuf->setOverflowData(fOutBuf->curPacketSize() + numFrameBytesToUse,
                                     overflowBytes, presentationTime, durationInMicroseconds);
        } else if (fCurFragmentationOffset > 0) {
            fCurFragmentationOffset = 0;
            fPreviousFrameEndedFragmentation = True;
        }
    }
    if (numFrameBytesToUse == 0 && frameSize > 0) {
        sendPacketIfNecessary();
    } else {
        unsigned char* frameStart = fOutBuf->curPtr();
        fOutBuf->increment(numFrameBytesToUse);
        doSpecialFrameHandling(curFragmentationOffset, frameStart,
                               numFrameBytesToUse, presentationTime,
                               overflowBytes);
        ++fNumFramesUsedSoFar;
        if (overflowBytes == 0) {
            fNextSendTime.tv_usec += durationInMicroseconds;
            fNextSendTime.tv_sec  += fNextSendTime.tv_usec / 1000000;
            fNextSendTime.tv_usec %= 1000000;
        }
        if (fOutBuf->isPreferredSize() ||
            fOutBuf->wouldOverflow(numFrameBytesToUse) ||
            (fPreviousFrameEndedFragmentation &&
             !allowOtherFramesAfterLastFragment()) ||
            !frameCanAppearAfterPacketStart(fOutBuf->curPtr() - frameSize,
                                            frameSize))
        {
            sendPacketIfNecessary();
        } else {
            packFrame();
        }
    }
} // MultiFramedRTPSink::afterGettingFrame1

static unsigned const rtpHeaderSize = 12;
Boolean MultiFramedRTPSink::isTooBigForAPacket(unsigned numBytes) const
{
    numBytes += rtpHeaderSize + specialHeaderSize() + frameSpecificHeaderSize();
    return fOutBuf->isTooBigForAPacket(numBytes);
}

void MultiFramedRTPSink::sendPacketIfNecessary()
{
    if (fNumFramesUsedSoFar > 0) {
        #ifdef TEST_LOSS
        if ((our_random() % 10) != 0)
        #endif
        if (!fRTPInterface.sendPacket(fOutBuf->packet(), fOutBuf->curPacketSize())) {
            if (fOnSendErrorFunc != NULL) {
                (*fOnSendErrorFunc)(fOnSendErrorData);
            }
        }
        ++fPacketCount;
        fTotalOctetCount += fOutBuf->curPacketSize();
        fOctetCount      += fOutBuf->curPacketSize()
                            - rtpHeaderSize - fSpecialHeaderSize - fTotalFrameSpecificHeaderSizes;
        ++fSeqNo;
    }
    if (fOutBuf->haveOverflowData() &&
        fOutBuf->totalBytesAvailable() > fOutBuf->totalBufferSize() / 2)
    {
        unsigned newPacketStart = fOutBuf->curPacketSize()
                                  - (rtpHeaderSize + fSpecialHeaderSize + frameSpecificHeaderSize());
        fOutBuf->adjustPacketStart(newPacketStart);
    } else {
        fOutBuf->resetPacketStart();
    }
    fOutBuf->resetOffset();
    fNumFramesUsedSoFar = 0;
    if (fNoFramesLeft) {
        onSourceClosure();
    } else {
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        int secsDiff         = fNextSendTime.tv_sec - timeNow.tv_sec;
        int64_t uSecondsToGo = secsDiff * 1000000 + (fNextSendTime.tv_usec - timeNow.tv_usec);
        if (uSecondsToGo < 0 || secsDiff < 0) {
            uSecondsToGo = 0;
        }
        nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecondsToGo, (TaskFunc *) sendNext, this);
    }
} // MultiFramedRTPSink::sendPacketIfNecessary

void MultiFramedRTPSink::sendNext(void* firstArg)
{
    MultiFramedRTPSink* sink = (MultiFramedRTPSink *) firstArg;

    sink->buildAndSendPacket(False);
}

void MultiFramedRTPSink::ourHandleClosure(void* clientData)
{
    MultiFramedRTPSink* sink = (MultiFramedRTPSink *) clientData;

    sink->fNoFramesLeft = True;
    sink->sendPacketIfNecessary();
}
