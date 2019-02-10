#include "MP3ADUinterleaving.hh"
#include "MP3ADUdescriptor.hh"
#include <string.h>
#ifdef TEST_LOSS
# include "GroupsockHelper.hh"
#endif
Interleaving::Interleaving(unsigned             cycleSize,
                           unsigned char const* cycleArray)
    : fCycleSize(cycleSize)
{
    for (unsigned i = 0; i < fCycleSize; ++i) {
        fInverseCycle[cycleArray[i]] = i;
    }
}

Interleaving::~Interleaving()
{}

MP3ADUinterleaverBase::MP3ADUinterleaverBase(UsageEnvironment& env,
                                             FramedSource*   inputSource)
    : FramedFilter(env, inputSource)
{}

MP3ADUinterleaverBase::~MP3ADUinterleaverBase()
{}

FramedSource * MP3ADUinterleaverBase::getInputSource(UsageEnvironment& env,
                                                     char const*     inputSourceName)
{
    FramedSource* inputSource;

    if (!FramedSource::lookupByName(env, inputSourceName, inputSource)) {
        return NULL;
    }
    if (strcmp(inputSource->MIMEtype(), "audio/MPA-ROBUST") != 0) {
        env.setResultMsg(inputSourceName, " is not an MP3 ADU source");
        return NULL;
    }
    return inputSource;
}

void MP3ADUinterleaverBase::afterGettingFrame(void*          clientData,
                                              unsigned       numBytesRead,
                                              unsigned,
                                              struct timeval presentationTime,
                                              unsigned       durationInMicroseconds)
{
    MP3ADUinterleaverBase* interleaverBase = (MP3ADUinterleaverBase *) clientData;

    interleaverBase->afterGettingFrame(numBytesRead,
                                       presentationTime, durationInMicroseconds);
    interleaverBase->doGetNextFrame();
}

class InterleavingFrames
{
public:
    InterleavingFrames(unsigned maxCycleSize);
    virtual ~InterleavingFrames();
    Boolean haveReleaseableFrame();
    void getIncomingFrameParams(unsigned char  index,
                                unsigned char *& dataPtr,
                                unsigned       & bytesAvailable);
    void getReleasingFrameParams(unsigned char  index,
                                 unsigned char *& dataPtr,
                                 unsigned       & bytesInUse,
                                 struct timeval & presentationTime,
                                 unsigned       & durationInMicroseconds);
    void setFrameParams(unsigned char index,
                        unsigned char icc, unsigned char ii,
                        unsigned frameSize, struct timeval presentationTime,
                        unsigned durationInMicroseconds);
    unsigned nextIndexToRelease()
    {
        return fNextIndexToRelease;
    }

    void releaseNext();
private:
    unsigned fMaxCycleSize;
    unsigned fNextIndexToRelease;
    class InterleavingFrameDescriptor* fDescriptors;
};
MP3ADUinterleaver::MP3ADUinterleaver(UsageEnvironment  & env,
                                     Interleaving const& interleaving,
                                     FramedSource*     inputSource)
    : MP3ADUinterleaverBase(env, inputSource),
    fInterleaving(interleaving),
    fFrames(new InterleavingFrames(interleaving.cycleSize())),
    fII(0), fICC(0)
{}

MP3ADUinterleaver::~MP3ADUinterleaver()
{
    delete fFrames;
}

MP3ADUinterleaver * MP3ADUinterleaver::createNew(UsageEnvironment  & env,
                                                 Interleaving const& interleaving,
                                                 FramedSource*     inputSource)
{
    return new MP3ADUinterleaver(env, interleaving, inputSource);
}

void MP3ADUinterleaver::doGetNextFrame()
{
    if (fFrames->haveReleaseableFrame()) {
        releaseOutgoingFrame();
        afterGetting(this);
    } else {
        fPositionOfNextIncomingFrame = fInterleaving.lookupInverseCycle(fII);
        unsigned char* dataPtr;
        unsigned bytesAvailable;
        fFrames->getIncomingFrameParams(fPositionOfNextIncomingFrame,
                                        dataPtr, bytesAvailable);
        fInputSource->getNextFrame(dataPtr, bytesAvailable,
                                   &MP3ADUinterleaverBase::afterGettingFrame, this,
                                   handleClosure, this);
    }
}

void MP3ADUinterleaver::releaseOutgoingFrame()
{
    unsigned char* fromPtr;

    fFrames->getReleasingFrameParams(fFrames->nextIndexToRelease(),
                                     fromPtr, fFrameSize,
                                     fPresentationTime, fDurationInMicroseconds);
    if (fFrameSize > fMaxSize) {
        fNumTruncatedBytes = fFrameSize - fMaxSize;
        fFrameSize         = fMaxSize;
    }
    memmove(fTo, fromPtr, fFrameSize);
    fFrames->releaseNext();
}

void MP3ADUinterleaver::afterGettingFrame(unsigned       numBytesRead,
                                          struct timeval presentationTime,
                                          unsigned       durationInMicroseconds)
{
    fFrames->setFrameParams(fPositionOfNextIncomingFrame,
                            fICC, fII, numBytesRead,
                            presentationTime, durationInMicroseconds);
    if (++fII == fInterleaving.cycleSize()) {
        fII  = 0;
        fICC = (fICC + 1) % 8;
    }
}

class DeinterleavingFrames
{
public:
    DeinterleavingFrames();
    virtual ~DeinterleavingFrames();
    Boolean haveReleaseableFrame();
    void getIncomingFrameParams(unsigned char *& dataPtr,
                                unsigned       & bytesAvailable);
    void getIncomingFrameParamsAfter(unsigned frameSize,
                                     struct timeval presentationTime,
                                     unsigned durationInMicroseconds,
                                     unsigned char& icc, unsigned char& ii);
    void getReleasingFrameParams(unsigned char *& dataPtr,
                                 unsigned       & bytesInUse,
                                 struct timeval & presentationTime,
                                 unsigned       & durationInMicroseconds);
    void moveIncomingFrameIntoPlace();
    void releaseNext();
    void startNewCycle();
private:
    unsigned fNextIndexToRelease;
    Boolean fHaveEndedCycle;
    unsigned fIIlastSeen;
    unsigned fMinIndexSeen, fMaxIndexSeen;
    class DeinterleavingFrameDescriptor* fDescriptors;
};
MP3ADUdeinterleaver::MP3ADUdeinterleaver(UsageEnvironment& env,
                                         FramedSource*   inputSource)
    : MP3ADUinterleaverBase(env, inputSource),
    fFrames(new DeinterleavingFrames),
    fIIlastSeen(~0), fICClastSeen(~0)
{}

MP3ADUdeinterleaver::~MP3ADUdeinterleaver()
{
    delete fFrames;
}

MP3ADUdeinterleaver * MP3ADUdeinterleaver::createNew(UsageEnvironment& env,
                                                     FramedSource*   inputSource)
{
    return new MP3ADUdeinterleaver(env, inputSource);
}

void MP3ADUdeinterleaver::doGetNextFrame()
{
    if (fFrames->haveReleaseableFrame()) {
        releaseOutgoingFrame();
        afterGetting(this);
    } else {
        #ifdef TEST_LOSS
NOTE:
        This code no longer works, because it uses synchronous reads,
        which are no longer supported.
        static unsigned const framesPerPacket = 3;
        static unsigned const frameCount = 0;
        static Boolean packetIsLost;
        while (1) {
            unsigned packetCount = frameCount / framesPerPacket;
            if ((frameCount++) % framesPerPacket == 0) {
                packetIsLost = (our_random() % 10 == 0);
            }
            if (packetIsLost) {
                unsigned char dummyBuf[2000];
                unsigned numBytesRead;
                struct timeval presentationTime;
                fInputSource->syncGetNextFrame(dummyBuf, sizeof dummyBuf,
                                               numBytesRead, presentationTime);
            } else {
                break;
            }
        }
        #endif // ifdef TEST_LOSS
        unsigned char* dataPtr;
        unsigned bytesAvailable;
        fFrames->getIncomingFrameParams(dataPtr, bytesAvailable);
        fInputSource->getNextFrame(dataPtr, bytesAvailable,
                                   &MP3ADUinterleaverBase::afterGettingFrame, this,
                                   handleClosure, this);
    }
} // MP3ADUdeinterleaver::doGetNextFrame

void MP3ADUdeinterleaver::afterGettingFrame(unsigned       numBytesRead,
                                            struct timeval presentationTime,
                                            unsigned       durationInMicroseconds)
{
    unsigned char icc, ii;

    fFrames->getIncomingFrameParamsAfter(numBytesRead,
                                         presentationTime, durationInMicroseconds,
                                         icc, ii);
    if (icc != fICClastSeen || ii == fIIlastSeen) {
        fFrames->startNewCycle();
    } else {
        fFrames->moveIncomingFrameIntoPlace();
    }
    fICClastSeen = icc;
    fIIlastSeen  = ii;
}

void MP3ADUdeinterleaver::releaseOutgoingFrame()
{
    unsigned char* fromPtr;

    fFrames->getReleasingFrameParams(fromPtr, fFrameSize,
                                     fPresentationTime, fDurationInMicroseconds);
    if (fFrameSize > fMaxSize) {
        fNumTruncatedBytes = fFrameSize - fMaxSize;
        fFrameSize         = fMaxSize;
    }
    memmove(fTo, fromPtr, fFrameSize);
    fFrames->releaseNext();
}

#define MAX_FRAME_SIZE 2000
class InterleavingFrameDescriptor
{
public:
    InterleavingFrameDescriptor()
    {
        frameDataSize = 0;
    }

    unsigned frameDataSize;
    struct timeval presentationTime;
    unsigned durationInMicroseconds;
    unsigned char frameData[MAX_FRAME_SIZE];
};
InterleavingFrames::InterleavingFrames(unsigned maxCycleSize)
    : fMaxCycleSize(maxCycleSize), fNextIndexToRelease(0),
    fDescriptors(new InterleavingFrameDescriptor[maxCycleSize])
{}

InterleavingFrames::~InterleavingFrames()
{
    delete[] fDescriptors;
}

Boolean InterleavingFrames::haveReleaseableFrame()
{
    return fDescriptors[fNextIndexToRelease].frameDataSize > 0;
}

void InterleavingFrames::getIncomingFrameParams(unsigned char  index,
                                                unsigned char *& dataPtr,
                                                unsigned       & bytesAvailable)
{
    InterleavingFrameDescriptor& desc = fDescriptors[index];

    dataPtr        = &desc.frameData[0];
    bytesAvailable = MAX_FRAME_SIZE;
}

void InterleavingFrames::getReleasingFrameParams(unsigned char  index,
                                                 unsigned char *& dataPtr,
                                                 unsigned       & bytesInUse,
                                                 struct timeval & presentationTime,
                                                 unsigned       & durationInMicroseconds)
{
    InterleavingFrameDescriptor& desc = fDescriptors[index];

    dataPtr                = &desc.frameData[0];
    bytesInUse             = desc.frameDataSize;
    presentationTime       = desc.presentationTime;
    durationInMicroseconds = desc.durationInMicroseconds;
}

void InterleavingFrames::setFrameParams(unsigned char  index,
                                        unsigned char  icc,
                                        unsigned char  ii,
                                        unsigned       frameSize,
                                        struct timeval presentationTime,
                                        unsigned       durationInMicroseconds)
{
    InterleavingFrameDescriptor& desc = fDescriptors[index];

    desc.frameDataSize          = frameSize;
    desc.presentationTime       = presentationTime;
    desc.durationInMicroseconds = durationInMicroseconds;
    unsigned char* ptr = &desc.frameData[0];
    (void) ADUdescriptor::getRemainingFrameSize(ptr);
    *ptr++ = ii;
    *ptr  &= ~0xE0;
    *ptr  |= (icc << 5);
}

void InterleavingFrames::releaseNext()
{
    fDescriptors[fNextIndexToRelease].frameDataSize = 0;
    fNextIndexToRelease = (fNextIndexToRelease + 1) % fMaxCycleSize;
}

class DeinterleavingFrameDescriptor
{
public:
    DeinterleavingFrameDescriptor()
    {
        frameDataSize = 0;
        frameData     = NULL;
    }

    virtual ~DeinterleavingFrameDescriptor()
    {
        delete[] frameData;
    }

    unsigned frameDataSize;
    struct timeval presentationTime;
    unsigned durationInMicroseconds;
    unsigned char* frameData;
};
DeinterleavingFrames::DeinterleavingFrames()
    : fNextIndexToRelease(0), fHaveEndedCycle(False),
    fMinIndexSeen(MAX_CYCLE_SIZE), fMaxIndexSeen(0),
    fDescriptors(new DeinterleavingFrameDescriptor[MAX_CYCLE_SIZE + 1])
{}

DeinterleavingFrames::~DeinterleavingFrames()
{
    delete[] fDescriptors;
}

Boolean DeinterleavingFrames::haveReleaseableFrame()
{
    if (!fHaveEndedCycle) {
        return fDescriptors[fNextIndexToRelease].frameDataSize > 0;
    } else {
        if (fNextIndexToRelease < fMinIndexSeen) {
            fNextIndexToRelease = fMinIndexSeen;
        }
        while (fNextIndexToRelease < fMaxIndexSeen &&
               fDescriptors[fNextIndexToRelease].frameDataSize == 0)
        {
            ++fNextIndexToRelease;
        }
        if (fNextIndexToRelease >= fMaxIndexSeen) {
            for (unsigned i = fMinIndexSeen; i < fMaxIndexSeen; ++i) {
                fDescriptors[i].frameDataSize = 0;
            }
            fMinIndexSeen = MAX_CYCLE_SIZE;
            fMaxIndexSeen = 0;
            moveIncomingFrameIntoPlace();
            fHaveEndedCycle     = False;
            fNextIndexToRelease = 0;
            return False;
        }
        return True;
    }
}

void DeinterleavingFrames::getIncomingFrameParams(unsigned char *& dataPtr,
                                                  unsigned       & bytesAvailable)
{
    DeinterleavingFrameDescriptor& desc = fDescriptors[MAX_CYCLE_SIZE];

    if (desc.frameData == NULL) {
        desc.frameData = new unsigned char[MAX_FRAME_SIZE];
    }
    dataPtr        = desc.frameData;
    bytesAvailable = MAX_FRAME_SIZE;
}

void DeinterleavingFrames
::getIncomingFrameParamsAfter(unsigned frameSize,
                              struct timeval presentationTime,
                              unsigned durationInMicroseconds,
                              unsigned char& icc, unsigned char& ii)
{
    DeinterleavingFrameDescriptor& desc = fDescriptors[MAX_CYCLE_SIZE];

    desc.frameDataSize          = frameSize;
    desc.presentationTime       = presentationTime;
    desc.durationInMicroseconds = durationInMicroseconds;
    unsigned char* ptr = desc.frameData;
    (void) ADUdescriptor::getRemainingFrameSize(ptr);
    fIIlastSeen = ii = *ptr;
    *ptr++      = 0xFF;
    icc         = (*ptr & 0xE0) >> 5;
    *ptr       |= 0xE0;
}

void DeinterleavingFrames::getReleasingFrameParams(unsigned char *& dataPtr,
                                                   unsigned       & bytesInUse,
                                                   struct timeval & presentationTime,
                                                   unsigned       & durationInMicroseconds)
{
    DeinterleavingFrameDescriptor& desc = fDescriptors[fNextIndexToRelease];

    dataPtr                = desc.frameData;
    bytesInUse             = desc.frameDataSize;
    presentationTime       = desc.presentationTime;
    durationInMicroseconds = desc.durationInMicroseconds;
}

void DeinterleavingFrames::moveIncomingFrameIntoPlace()
{
    DeinterleavingFrameDescriptor& fromDesc = fDescriptors[MAX_CYCLE_SIZE];
    DeinterleavingFrameDescriptor& toDesc   = fDescriptors[fIIlastSeen];

    toDesc.frameDataSize    = fromDesc.frameDataSize;
    toDesc.presentationTime = fromDesc.presentationTime;
    unsigned char* tmp = toDesc.frameData;
    toDesc.frameData   = fromDesc.frameData;
    fromDesc.frameData = tmp;
    if (fIIlastSeen < fMinIndexSeen) {
        fMinIndexSeen = fIIlastSeen;
    }
    if (fIIlastSeen + 1 > fMaxIndexSeen) {
        fMaxIndexSeen = fIIlastSeen + 1;
    }
}

void DeinterleavingFrames::releaseNext()
{
    fDescriptors[fNextIndexToRelease].frameDataSize = 0;
    fNextIndexToRelease = (fNextIndexToRelease + 1) % MAX_CYCLE_SIZE;
}

void DeinterleavingFrames::startNewCycle()
{
    fHaveEndedCycle = True;
}
