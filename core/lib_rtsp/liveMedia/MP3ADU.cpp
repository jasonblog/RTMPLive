#include "MP3ADU.hh"
#include "MP3ADUdescriptor.hh"
#include "MP3Internals.hh"
#include <string.h>
#ifdef TEST_LOSS
# include "GroupsockHelper.hh"
#endif
#define SegmentBufSize 2000
class Segment
{
public:
    unsigned char buf[SegmentBufSize];
    unsigned char * dataStart()
    {
        return &buf[descriptorSize];
    }

    unsigned frameSize;
    unsigned dataHere();
    unsigned descriptorSize;
    static unsigned const headerSize;
    unsigned sideInfoSize, aduSize;
    unsigned backpointer;
    struct timeval presentationTime;
    unsigned durationInMicroseconds;
};
unsigned const Segment::headerSize = 4;
#define SegmentQueueSize 20
class SegmentQueue
{
public:
    SegmentQueue(Boolean directionIsToADU, Boolean includeADUdescriptors)
        : fDirectionIsToADU(directionIsToADU),
        fIncludeADUdescriptors(includeADUdescriptors)
    {
        reset();
    }

    Segment s[SegmentQueueSize];
    unsigned headIndex()
    {
        return fHeadIndex;
    }

    Segment& headSegment()
    {
        return s[fHeadIndex];
    }

    unsigned nextFreeIndex()
    {
        return fNextFreeIndex;
    }

    Segment& nextFreeSegment()
    {
        return s[fNextFreeIndex];
    }

    Boolean isEmpty()
    {
        return isEmptyOrFull() && totalDataSize() == 0;
    }

    Boolean isFull()
    {
        return isEmptyOrFull() && totalDataSize() > 0;
    }

    static unsigned nextIndex(unsigned ix)
    {
        return (ix + 1) % SegmentQueueSize;
    }

    static unsigned prevIndex(unsigned ix)
    {
        return (ix + SegmentQueueSize - 1) % SegmentQueueSize;
    }

    unsigned totalDataSize()
    {
        return fTotalDataSize;
    }

    void enqueueNewSegment(FramedSource* inputSource, FramedSource* usingSource);
    Boolean dequeue();
    Boolean insertDummyBeforeTail(unsigned backpointer);
    void reset()
    {
        fHeadIndex = fNextFreeIndex = fTotalDataSize = 0;
    }

private:
    static void sqAfterGettingSegment(void*          clientData,
                                      unsigned       numBytesRead,
                                      unsigned       numTruncatedBytes,
                                      struct timeval presentationTime,
                                      unsigned       durationInMicroseconds);
    Boolean sqAfterGettingCommon(Segment& seg, unsigned numBytesRead);
    Boolean isEmptyOrFull()
    {
        return headIndex() == nextFreeIndex();
    }

    unsigned fHeadIndex, fNextFreeIndex, fTotalDataSize;
    FramedSource* fUsingSource;
    Boolean fDirectionIsToADU;
    Boolean fIncludeADUdescriptors;
};
ADUFromMP3Source::ADUFromMP3Source(UsageEnvironment& env,
                                   FramedSource*   inputSource,
                                   Boolean         includeADUdescriptors)
    : FramedFilter(env, inputSource),
    fAreEnqueueingMP3Frame(False),
    fSegments(new SegmentQueue(True,
                               False)),
    fIncludeADUdescriptors(includeADUdescriptors),
    fTotalDataSizeBeforePreviousRead(0), fScale(1), fFrameCounter(0)
{}

ADUFromMP3Source::~ADUFromMP3Source()
{
    delete fSegments;
}

char const * ADUFromMP3Source::MIMEtype() const
{
    return "audio/MPA-ROBUST";
}

ADUFromMP3Source * ADUFromMP3Source::createNew(UsageEnvironment& env,
                                               FramedSource*   inputSource,
                                               Boolean         includeADUdescriptors)
{
    if (strcmp(inputSource->MIMEtype(), "audio/MPEG") != 0) {
        env.setResultMsg(inputSource->name(), " is not an MPEG audio source");
        return NULL;
    }
    return new ADUFromMP3Source(env, inputSource, includeADUdescriptors);
}

void ADUFromMP3Source::resetInput()
{
    fSegments->reset();
}

Boolean ADUFromMP3Source::setScaleFactor(int scale)
{
    if (scale < 1) {
        return False;
    }
    fScale = scale;
    return True;
}

void ADUFromMP3Source::doGetNextFrame()
{
    if (!fAreEnqueueingMP3Frame) {
        fTotalDataSizeBeforePreviousRead = fSegments->totalDataSize();
        fAreEnqueueingMP3Frame = True;
        fSegments->enqueueNewSegment(fInputSource, this);
    } else {
        fAreEnqueueingMP3Frame = False;
        if (!doGetNextFrame1()) {
            handleClosure();
        }
    }
}

Boolean ADUFromMP3Source::doGetNextFrame1()
{
    unsigned tailIndex;
    Segment* tailSeg;
    Boolean needMoreData;

    if (fSegments->isEmpty()) {
        needMoreData = True;
        tailSeg      = NULL;
        tailIndex    = 0;
    } else {
        tailIndex    = SegmentQueue::prevIndex(fSegments->nextFreeIndex());
        tailSeg      = &(fSegments->s[tailIndex]);
        needMoreData =
            fTotalDataSizeBeforePreviousRead < tailSeg->backpointer ||
            tailSeg->backpointer + tailSeg->dataHere() < tailSeg->aduSize;
    }
    if (needMoreData) {
        doGetNextFrame();
        return True;
    }
    fFrameSize              = tailSeg->headerSize + tailSeg->sideInfoSize + tailSeg->aduSize;
    fPresentationTime       = tailSeg->presentationTime;
    fDurationInMicroseconds = tailSeg->durationInMicroseconds;
    unsigned descriptorSize =
        fIncludeADUdescriptors ? ADUdescriptor::computeSize(fFrameSize) : 0;
    #ifdef DEBUG
    fprintf(stderr, "m->a:outputting ADU %d<-%d, nbr:%d, sis:%d, dh:%d, (descriptor size: %d)\n", tailSeg->aduSize,
            tailSeg->backpointer, fFrameSize, tailSeg->sideInfoSize, tailSeg->dataHere(), descriptorSize);
    #endif
    if (descriptorSize + fFrameSize > fMaxSize) {
        envir() << "ADUFromMP3Source::doGetNextFrame1(): not enough room ("
                << descriptorSize + fFrameSize << ">"
                << fMaxSize << ")\n";
        fFrameSize = 0;
        return False;
    }
    unsigned char* toPtr = fTo;
    if (fIncludeADUdescriptors) {
        fFrameSize += ADUdescriptor::generateDescriptor(toPtr, fFrameSize);
    }
    memmove(toPtr, tailSeg->dataStart(),
            tailSeg->headerSize + tailSeg->sideInfoSize);
    toPtr += tailSeg->headerSize + tailSeg->sideInfoSize;
    unsigned offset    = 0;
    unsigned i         = tailIndex;
    unsigned prevBytes = tailSeg->backpointer;
    while (prevBytes > 0) {
        i = SegmentQueue::prevIndex(i);
        unsigned dataHere = fSegments->s[i].dataHere();
        if (dataHere < prevBytes) {
            prevBytes -= dataHere;
        } else {
            offset = dataHere - prevBytes;
            break;
        }
    }
    while (fSegments->headIndex() != i) {
        fSegments->dequeue();
    }
    unsigned bytesToUse = tailSeg->aduSize;
    while (bytesToUse > 0) {
        Segment& seg = fSegments->s[i];
        unsigned char* fromPtr =
            &seg.dataStart()[seg.headerSize + seg.sideInfoSize + offset];
        unsigned dataHere      = seg.dataHere() - offset;
        unsigned bytesUsedHere = dataHere < bytesToUse ? dataHere : bytesToUse;
        memmove(toPtr, fromPtr, bytesUsedHere);
        bytesToUse -= bytesUsedHere;
        toPtr      += bytesUsedHere;
        offset      = 0;
        i = SegmentQueue::nextIndex(i);
    }
    if (fFrameCounter++ % fScale == 0) {
        afterGetting(this);
    } else {
        doGetNextFrame();
    }
    return True;
} // ADUFromMP3Source::doGetNextFrame1

MP3FromADUSource::MP3FromADUSource(UsageEnvironment& env,
                                   FramedSource*   inputSource,
                                   Boolean         includeADUdescriptors)
    : FramedFilter(env, inputSource),
    fAreEnqueueingADU(False),
    fSegments(new SegmentQueue(False,
                               includeADUdescriptors))
{}

MP3FromADUSource::~MP3FromADUSource()
{
    delete fSegments;
}

char const * MP3FromADUSource::MIMEtype() const
{
    return "audio/MPEG";
}

MP3FromADUSource * MP3FromADUSource::createNew(UsageEnvironment& env,
                                               FramedSource*   inputSource,
                                               Boolean         includeADUdescriptors)
{
    if (strcmp(inputSource->MIMEtype(), "audio/MPA-ROBUST") != 0) {
        env.setResultMsg(inputSource->name(), " is not an MP3 ADU source");
        return NULL;
    }
    return new MP3FromADUSource(env, inputSource, includeADUdescriptors);
}

void MP3FromADUSource::doGetNextFrame()
{
    if (fAreEnqueueingADU) {
        insertDummyADUsIfNecessary();
    }
    fAreEnqueueingADU = False;
    if (needToGetAnADU()) {
        #ifdef TEST_LOSS
NOTE:
        This code no longer works, because it uses synchronous reads,
        which are no longer supported.
        static unsigned const framesPerPacket = 10;
        static unsigned const frameCount = 0;
        static Boolean packetIsLost;
        while (1) {
            if ((frameCount++) % framesPerPacket == 0) {
                packetIsLost = (our_random() % 10 == 0);
            }
            if (packetIsLost) {
                Segment dummySegment;
                unsigned numBytesRead;
                struct timeval presentationTime;
                fInputSource->syncGetNextFrame(dummySegment.buf,
                                               sizeof dummySegment.buf, numBytesRead,
                                               presentationTime);
            } else {
                break;
            }
        }
        #endif // ifdef TEST_LOSS
        fAreEnqueueingADU = True;
        fSegments->enqueueNewSegment(fInputSource, this);
    } else {
        generateFrameFromHeadADU();
        afterGetting(this);
    }
} // MP3FromADUSource::doGetNextFrame

Boolean MP3FromADUSource::needToGetAnADU()
{
    Boolean needToEnqueue = True;

    if (!fSegments->isEmpty()) {
        unsigned index = fSegments->headIndex();
        Segment* seg   = &(fSegments->headSegment());
        int const endOfHeadFrame = (int) seg->dataHere();
        unsigned frameOffset     = 0;
        while (1) {
            int endOfData = frameOffset - seg->backpointer + seg->aduSize;
            if (endOfData >= endOfHeadFrame) {
                needToEnqueue = False;
                break;
            }
            frameOffset += seg->dataHere();
            index        = SegmentQueue::nextIndex(index);
            if (index == fSegments->nextFreeIndex()) {
                break;
            }
            seg = &(fSegments->s[index]);
        }
    }
    return needToEnqueue;
}

void MP3FromADUSource::insertDummyADUsIfNecessary()
{
    if (fSegments->isEmpty()) {
        return;
    }
    unsigned tailIndex =
        SegmentQueue::prevIndex(fSegments->nextFreeIndex());
    Segment* tailSeg = &(fSegments->s[tailIndex]);
    while (1) {
        unsigned prevADUend;
        if (fSegments->headIndex() != tailIndex) {
            unsigned prevIndex   = SegmentQueue::prevIndex(tailIndex);
            Segment& prevSegment = fSegments->s[prevIndex];
            prevADUend = prevSegment.dataHere() + prevSegment.backpointer;
            if (prevSegment.aduSize > prevADUend) {
                prevADUend = 0;
            } else {
                prevADUend -= prevSegment.aduSize;
            }
        } else {
            prevADUend = 0;
        }
        if (tailSeg->backpointer > prevADUend) {
            #ifdef DEBUG
            fprintf(stderr, "a->m:need to insert a dummy ADU (%d, %d, %d) [%d, %d]\n", tailSeg->backpointer, prevADUend,
                    tailSeg->dataHere(), fSegments->headIndex(), fSegments->nextFreeIndex());
            #endif
            tailIndex = fSegments->nextFreeIndex();
            if (!fSegments->insertDummyBeforeTail(prevADUend)) {
                return;
            }
            tailSeg = &(fSegments->s[tailIndex]);
        } else {
            break;
        }
    }
} // MP3FromADUSource::insertDummyADUsIfNecessary

Boolean MP3FromADUSource::generateFrameFromHeadADU()
{
    if (fSegments->isEmpty()) {
        return False;
    }
    unsigned index = fSegments->headIndex();
    Segment* seg   = &(fSegments->headSegment());
    #ifdef DEBUG
    fprintf(stderr, "a->m:outputting frame for %d<-%d (fs %d, dh %d), (descriptorSize: %d)\n", seg->aduSize,
            seg->backpointer, seg->frameSize, seg->dataHere(), seg->descriptorSize);
    #endif
    unsigned char* toPtr = fTo;
    fFrameSize              = seg->frameSize;
    fPresentationTime       = seg->presentationTime;
    fDurationInMicroseconds = seg->durationInMicroseconds;
    memmove(toPtr, seg->dataStart(), seg->headerSize + seg->sideInfoSize);
    toPtr += seg->headerSize + seg->sideInfoSize;
    unsigned bytesToZero = seg->dataHere();
    for (unsigned i = 0; i < bytesToZero; ++i) {
        toPtr[i] = '\0';
    }
    unsigned frameOffset = 0;
    unsigned toOffset    = 0;
    unsigned const endOfHeadFrame = seg->dataHere();
    while (toOffset < endOfHeadFrame) {
        int startOfData = frameOffset - seg->backpointer;
        if (startOfData > (int) endOfHeadFrame) {
            break;
        }
        int endOfData = startOfData + seg->aduSize;
        if (endOfData > (int) endOfHeadFrame) {
            endOfData = endOfHeadFrame;
        }
        unsigned fromOffset;
        if (startOfData <= (int) toOffset) {
            fromOffset  = toOffset - startOfData;
            startOfData = toOffset;
            if (endOfData < startOfData) {
                endOfData = startOfData;
            }
        } else {
            fromOffset = 0;
            unsigned bytesToZero = startOfData - toOffset;
            #ifdef DEBUG
            if (bytesToZero > 0) {
                fprintf(stderr, "a->m:outputting %d zero bytes (%d, %d, %d, %d)\n", bytesToZero, startOfData, toOffset,
                        frameOffset, seg->backpointer);
            }
            #endif
            toOffset += bytesToZero;
        }
        unsigned char* fromPtr =
            &seg->dataStart()[seg->headerSize + seg->sideInfoSize + fromOffset];
        unsigned bytesUsedHere = endOfData - startOfData;
        #ifdef DEBUG
        if (bytesUsedHere > 0) {
            fprintf(stderr, "a->m:outputting %d bytes from %d<-%d\n", bytesUsedHere, seg->aduSize, seg->backpointer);
        }
        #endif
        memmove(toPtr + toOffset, fromPtr, bytesUsedHere);
        toOffset    += bytesUsedHere;
        frameOffset += seg->dataHere();
        index        = SegmentQueue::nextIndex(index);
        if (index == fSegments->nextFreeIndex()) {
            break;
        }
        seg = &(fSegments->s[index]);
    }
    fSegments->dequeue();
    return True;
} // MP3FromADUSource::generateFrameFromHeadADU

unsigned Segment::dataHere()
{
    int result = frameSize - (headerSize + sideInfoSize);

    if (result < 0) {
        return 0;
    }
    return (unsigned) result;
}

void SegmentQueue::enqueueNewSegment(FramedSource* inputSource,
                                     FramedSource* usingSource)
{
    if (isFull()) {
        usingSource->envir() << "SegmentQueue::enqueueNewSegment() overflow\n";
        usingSource->handleClosure();
        return;
    }
    fUsingSource = usingSource;
    Segment& seg = nextFreeSegment();
    inputSource->getNextFrame(seg.buf, sizeof seg.buf,
                              sqAfterGettingSegment, this,
                              FramedSource::handleClosure, usingSource);
}

void SegmentQueue::sqAfterGettingSegment(void*          clientData,
                                         unsigned       numBytesRead,
                                         unsigned,
                                         struct timeval presentationTime,
                                         unsigned       durationInMicroseconds)
{
    SegmentQueue* segQueue = (SegmentQueue *) clientData;
    Segment& seg = segQueue->nextFreeSegment();

    seg.presentationTime       = presentationTime;
    seg.durationInMicroseconds = durationInMicroseconds;
    if (segQueue->sqAfterGettingCommon(seg, numBytesRead)) {
        #ifdef DEBUG
        char const* direction = segQueue->fDirectionIsToADU ? "m->a" : "a->m";
        fprintf(stderr, "%s:read frame %d<-%d, fs:%d, sis:%d, dh:%d, (descriptor size: %d)\n", direction, seg.aduSize,
                seg.backpointer, seg.frameSize, seg.sideInfoSize, seg.dataHere(), seg.descriptorSize);
        #endif
    }
    segQueue->fUsingSource->doGetNextFrame();
}

Boolean SegmentQueue::sqAfterGettingCommon(Segment  & seg,
                                           unsigned numBytesRead)
{
    unsigned char* fromPtr = seg.buf;

    if (fIncludeADUdescriptors) {
        (void) ADUdescriptor::getRemainingFrameSize(fromPtr);
        seg.descriptorSize = (unsigned) (fromPtr - seg.buf);
    } else {
        seg.descriptorSize = 0;
    }
    unsigned hdr;
    MP3SideInfo sideInfo;
    if (!GetADUInfoFromMP3Frame(fromPtr, numBytesRead,
                                hdr, seg.frameSize,
                                sideInfo, seg.sideInfoSize,
                                seg.backpointer, seg.aduSize))
    {
        return False;
    }
    if (!fDirectionIsToADU) {
        unsigned newADUSize =
            numBytesRead - seg.descriptorSize - 4 - seg.sideInfoSize;
        if (newADUSize > seg.aduSize) {
            seg.aduSize = newADUSize;
        }
    }
    fTotalDataSize += seg.dataHere();
    fNextFreeIndex  = nextIndex(fNextFreeIndex);
    return True;
}

Boolean SegmentQueue::dequeue()
{
    if (isEmpty()) {
        fUsingSource->envir() << "SegmentQueue::dequeue(): underflow!\n";
        return False;
    }
    Segment& seg = s[headIndex()];
    fTotalDataSize -= seg.dataHere();
    fHeadIndex      = nextIndex(fHeadIndex);
    return True;
}

Boolean SegmentQueue::insertDummyBeforeTail(unsigned backpointer)
{
    if (isEmptyOrFull()) {
        return False;
    }
    unsigned newTailIndex = nextFreeIndex();
    Segment& newTailSeg   = s[newTailIndex];
    unsigned oldTailIndex = prevIndex(newTailIndex);
    Segment& oldTailSeg   = s[oldTailIndex];
    newTailSeg = oldTailSeg;
    unsigned char* ptr = oldTailSeg.buf;
    if (fIncludeADUdescriptors) {
        unsigned remainingFrameSize =
            oldTailSeg.headerSize + oldTailSeg.sideInfoSize + 0;
        unsigned currentDescriptorSize = oldTailSeg.descriptorSize;
        if (currentDescriptorSize == 2) {
            ADUdescriptor::generateTwoByteDescriptor(ptr, remainingFrameSize);
        } else {
            (void) ADUdescriptor::generateDescriptor(ptr, remainingFrameSize);
        }
    }
    if (!ZeroOutMP3SideInfo(ptr, oldTailSeg.frameSize,
                            backpointer))
    {
        return False;
    }
    unsigned dummyNumBytesRead =
        oldTailSeg.descriptorSize + 4 + oldTailSeg.sideInfoSize;
    return sqAfterGettingCommon(oldTailSeg, dummyNumBytesRead);
}
