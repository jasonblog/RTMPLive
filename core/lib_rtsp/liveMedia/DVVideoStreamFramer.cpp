#include "DVVideoStreamFramer.hh"
#include "GroupsockHelper.hh"
DVVideoStreamFramer::DVVideoStreamFramer(UsageEnvironment& env, FramedSource* inputSource,
                                         Boolean sourceIsSeekable, Boolean leavePresentationTimesUnmodified)
    : FramedFilter(env, inputSource),
    fLeavePresentationTimesUnmodified(leavePresentationTimesUnmodified),
    fOurProfile(NULL), fInitialBlocksPresent(False), fSourceIsSeekable(sourceIsSeekable)
{
    fTo = NULL;
    gettimeofday(&fNextFramePresentationTime, NULL);
}

DVVideoStreamFramer::~DVVideoStreamFramer()
{}

DVVideoStreamFramer * DVVideoStreamFramer::createNew(UsageEnvironment& env, FramedSource* inputSource,
                                                     Boolean sourceIsSeekable, Boolean leavePresentationTimesUnmodified)
{
    return new DVVideoStreamFramer(env, inputSource, sourceIsSeekable, leavePresentationTimesUnmodified);
}

struct DVVideoProfile {
    char const* name;
    unsigned    apt;
    unsigned    sType;
    unsigned    sequenceCount;
    unsigned    channelCount;
    unsigned    dvFrameSize;
    double      frameDuration;
};
static DVVideoProfile const profiles[] = {
    { "SD-VCR/525-60",  0, 0x00, 10, 1, 120000, (1000000 * 1001) / 30000.0      },
    { "SD-VCR/625-50",  0, 0x00, 12, 1, 144000, 1000000 / 25.0                  },
    { "314M-25/525-60", 1, 0x00, 10, 1, 120000, (1000000 * 1001) / 30000.0      },
    { "314M-25/625-50", 1, 0x00, 12, 1, 144000, 1000000 / 25.0                  },
    { "314M-50/525-60", 1, 0x04, 10, 2, 240000, (1000000 * 1001) / 30000.0      },
    { "314M-50/625-50", 1, 0x04, 12, 2, 288000, 1000000 / 25.0                  },
    { "370M/1080-60i",  1, 0x14, 10, 4, 480000, (1000000 * 1001) / 30000.0      },
    { "370M/1080-50i",  1, 0x14, 12, 4, 576000, 1000000 / 25.0                  },
    { "370M/720-60p",   1, 0x18, 10, 2, 240000, (1000000 * 1001) / 60000.0      },
    { "370M/720-50p",   1, 0x18, 12, 2, 288000, 1000000 / 50.0                  },
    { NULL,             0, 0,    0,  0, 0,      0.0                             }
};
char const * DVVideoStreamFramer::profileName()
{
    if (fOurProfile == NULL) {
        getProfile();
    }
    return fOurProfile != NULL ? ((DVVideoProfile const *) fOurProfile)->name : NULL;
}

Boolean DVVideoStreamFramer::getFrameParameters(unsigned& frameSize, double& frameDuration)
{
    if (fOurProfile == NULL) {
        getProfile();
    }
    if (fOurProfile == NULL) {
        return False;
    }
    frameSize     = ((DVVideoProfile const *) fOurProfile)->dvFrameSize;
    frameDuration = ((DVVideoProfile const *) fOurProfile)->frameDuration;
    return True;
}

void DVVideoStreamFramer::getProfile()
{
    fInputSource->getNextFrame(fSavedInitialBlocks, DV_SAVED_INITIAL_BLOCKS_SIZE,
                               afterGettingFrame, this, FramedSource::handleClosure, this);
    envir().taskScheduler().doEventLoop(&fInitialBlocksPresent);
}

Boolean DVVideoStreamFramer::isDVVideoStreamFramer() const
{
    return True;
}

void DVVideoStreamFramer::doGetNextFrame()
{
    fFrameSize = 0;
    if (fInitialBlocksPresent && !fSourceIsSeekable) {
        if (fMaxSize < DV_SAVED_INITIAL_BLOCKS_SIZE) {
            fNumTruncatedBytes = fMaxSize;
            afterGetting(this);
            return;
        }
        memmove(fTo, fSavedInitialBlocks, DV_SAVED_INITIAL_BLOCKS_SIZE);
        fFrameSize = DV_SAVED_INITIAL_BLOCKS_SIZE;
        fTo       += DV_SAVED_INITIAL_BLOCKS_SIZE;
        fInitialBlocksPresent = False;
    }
    fMaxSize -= fMaxSize % DV_DIF_BLOCK_SIZE;
    getAndDeliverData();
}

#define DV_SMALLEST_POSSIBLE_FRAME_SIZE 120000
void DVVideoStreamFramer::getAndDeliverData()
{
    unsigned const totFrameSize =
        fOurProfile != NULL ? ((DVVideoProfile const *) fOurProfile)->dvFrameSize : DV_SMALLEST_POSSIBLE_FRAME_SIZE;
    unsigned totBytesToDeliver = totFrameSize < fMaxSize ? totFrameSize : fMaxSize;
    unsigned numBytesToRead    = totBytesToDeliver - fFrameSize;

    fInputSource->getNextFrame(fTo, numBytesToRead, afterGettingFrame, this, FramedSource::handleClosure, this);
}

void DVVideoStreamFramer::afterGettingFrame(void* clientData, unsigned frameSize,
                                            unsigned numTruncatedBytes,
                                            struct timeval presentationTime, unsigned)
{
    DVVideoStreamFramer* source = (DVVideoStreamFramer *) clientData;

    source->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime);
}

#define DVSectionId(n) ptr[(n)*DV_DIF_BLOCK_SIZE + 0]
#define DVData(n, i)   ptr[(n)*DV_DIF_BLOCK_SIZE + 3+(i)]
#define DV_SECTION_HEADER    0x1F
#define DV_PACK_HEADER_10    0x3F
#define DV_PACK_HEADER_12    0xBF
#define DV_SECTION_VAUX_MIN  0x50
#define DV_SECTION_VAUX_MAX  0x5F
#define DV_PACK_VIDEO_SOURCE 60
#ifndef MILLION
# define MILLION             1000000
#endif
void DVVideoStreamFramer::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                                            struct timeval presentationTime)
{
    if (fOurProfile == NULL && frameSize >= DV_SAVED_INITIAL_BLOCKS_SIZE) {
        u_int8_t const* data = (fTo == NULL) ? fSavedInitialBlocks : fTo;
        for (u_int8_t const* ptr = data;
             ptr + 6 * DV_DIF_BLOCK_SIZE <= &data[DV_SAVED_INITIAL_BLOCKS_SIZE];
             ptr += DV_DIF_BLOCK_SIZE)
        {
            u_int8_t const sectionHeader = DVSectionId(0);
            u_int8_t const sectionVAUX   = DVSectionId(5);
            u_int8_t const packHeaderNum = DVData(0, 0);
            if (sectionHeader == DV_SECTION_HEADER &&
                (packHeaderNum == DV_PACK_HEADER_10 || packHeaderNum == DV_PACK_HEADER_12) &&
                (sectionVAUX >= DV_SECTION_VAUX_MIN && sectionVAUX <= DV_SECTION_VAUX_MAX))
            {
                u_int8_t const apt           = DVData(0, 1) & 0x07;
                u_int8_t const sType         = DVData(5, 48) & 0x1F;
                u_int8_t const sequenceCount = (packHeaderNum == DV_PACK_HEADER_10) ? 10 : 12;
                for (DVVideoProfile const* profile = profiles; profile->name != NULL; ++profile) {
                    if (profile->apt == apt && profile->sType == sType && profile->sequenceCount == sequenceCount) {
                        fOurProfile = profile;
                        break;
                    }
                }
                break;
            }
        }
    }
    if (fTo != NULL) {
        unsigned const totFrameSize =
            fOurProfile != NULL ? ((DVVideoProfile const *) fOurProfile)->dvFrameSize : DV_SMALLEST_POSSIBLE_FRAME_SIZE;
        fFrameSize       += frameSize;
        fTo              += frameSize;
        fPresentationTime = presentationTime;
        if (fFrameSize < totFrameSize && fFrameSize < fMaxSize && numTruncatedBytes == 0) {
            getAndDeliverData();
        } else {
            fNumTruncatedBytes = totFrameSize - fFrameSize;
            if (fOurProfile != NULL) {
                if (!fLeavePresentationTimesUnmodified) {
                    fPresentationTime = fNextFramePresentationTime;
                }
                DVVideoProfile const* ourProfile = (DVVideoProfile const *) fOurProfile;
                double durationInMicroseconds    = (fFrameSize * ourProfile->frameDuration) / ourProfile->dvFrameSize;
                fDurationInMicroseconds = (unsigned) durationInMicroseconds;
                fNextFramePresentationTime.tv_usec += fDurationInMicroseconds;
                fNextFramePresentationTime.tv_sec  += fNextFramePresentationTime.tv_usec / MILLION;
                fNextFramePresentationTime.tv_usec %= MILLION;
            }
            afterGetting(this);
        }
    } else {
        fInitialBlocksPresent = True;
    }
} // DVVideoStreamFramer::afterGettingFrame
