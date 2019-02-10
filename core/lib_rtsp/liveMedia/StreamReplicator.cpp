#include "StreamReplicator.hh"
class StreamReplica : public FramedSource
{
protected:
    friend class StreamReplicator;
    StreamReplica(StreamReplicator& ourReplicator);
    virtual ~StreamReplica();
private:
    virtual void doGetNextFrame();
    virtual void doStopGettingFrames();
private:
    static void copyReceivedFrame(StreamReplica* toReplica, StreamReplica* fromReplica);
private:
    StreamReplicator& fOurReplicator;
    int fFrameIndex;
    Boolean fDeliveryInProgress;
    StreamReplica* fNext;
};
StreamReplicator * StreamReplicator::createNew(UsageEnvironment& env, FramedSource* inputSource,
                                               Boolean deleteWhenLastReplicaDies)
{
    return new StreamReplicator(env, inputSource, deleteWhenLastReplicaDies);
}

StreamReplicator::StreamReplicator(UsageEnvironment& env, FramedSource* inputSource, Boolean deleteWhenLastReplicaDies)
    : Medium(env),
    fInputSource(inputSource), fDeleteWhenLastReplicaDies(deleteWhenLastReplicaDies), fInputSourceHasClosed(False),
    fNumReplicas(0), fNumActiveReplicas(0), fNumDeliveriesMadeSoFar(0),
    fFrameIndex(0), fMasterReplica(NULL), fReplicasAwaitingCurrentFrame(NULL), fReplicasAwaitingNextFrame(NULL)
{}

StreamReplicator::~StreamReplicator()
{
    Medium::close(fInputSource);
}

FramedSource * StreamReplicator::createStreamReplica()
{
    ++fNumReplicas;
    return new StreamReplica(*this);
}

void StreamReplicator::getNextFrame(StreamReplica* replica)
{
    if (fInputSourceHasClosed) {
        replica->handleClosure();
        return;
    }
    if (replica->fFrameIndex == -1) {
        replica->fFrameIndex = fFrameIndex;
        ++fNumActiveReplicas;
    }
    if (fMasterReplica == NULL) {
        fMasterReplica = replica;
        if (fInputSource != NULL) fInputSource->getNextFrame(fMasterReplica->fTo, fMasterReplica->fMaxSize,
                                                             afterGettingFrame, this, onSourceClosure, this);
    } else if (replica->fFrameIndex != fFrameIndex) {
        replica->fNext = fReplicasAwaitingNextFrame;
        fReplicasAwaitingNextFrame = replica;
    } else {
        replica->fNext = fReplicasAwaitingCurrentFrame;
        fReplicasAwaitingCurrentFrame = replica;
        if (fInputSource != NULL && !fInputSource->isCurrentlyAwaitingData()) {
            deliverReceivedFrame();
        }
    }
}

void StreamReplicator::deactivateStreamReplica(StreamReplica* replicaBeingDeactivated)
{
    if (fNumReplicas == 0) {
        fprintf(stderr, "StreamReplicator::deactivateStreamReplica() Internal Error!\n");
    }
    --fNumActiveReplicas;
    if (replicaBeingDeactivated->fDeliveryInProgress) {
        --fNumDeliveriesMadeSoFar;
    }
    if (replicaBeingDeactivated == fMasterReplica) {
        if (fReplicasAwaitingCurrentFrame == NULL) {
            fMasterReplica = NULL;
        } else {
            fMasterReplica = fReplicasAwaitingCurrentFrame;
            fReplicasAwaitingCurrentFrame = fReplicasAwaitingCurrentFrame->fNext;
            fMasterReplica->fNext         = NULL;
        }
        if (fInputSource != NULL) {
            if (fInputSource->isCurrentlyAwaitingData()) {
                fInputSource->stopGettingFrames();
                if (fMasterReplica != NULL) {
                    fInputSource->getNextFrame(fMasterReplica->fTo, fMasterReplica->fMaxSize,
                                               afterGettingFrame, this, onSourceClosure, this);
                }
            } else {
                if (fMasterReplica != NULL) {
                    StreamReplica::copyReceivedFrame(fMasterReplica, replicaBeingDeactivated);
                } else {}
            }
        }
    } else {
        if (fReplicasAwaitingCurrentFrame != NULL) {
            if (replicaBeingDeactivated == fReplicasAwaitingCurrentFrame) {
                fReplicasAwaitingCurrentFrame  = replicaBeingDeactivated->fNext;
                replicaBeingDeactivated->fNext = NULL;
            } else {
                for (StreamReplica* r1 = fReplicasAwaitingCurrentFrame; r1->fNext != NULL; r1 = r1->fNext) {
                    if (r1->fNext == replicaBeingDeactivated) {
                        r1->fNext = replicaBeingDeactivated->fNext;
                        replicaBeingDeactivated->fNext = NULL;
                        break;
                    }
                }
            }
        }
        if (fReplicasAwaitingNextFrame != NULL) {
            if (replicaBeingDeactivated == fReplicasAwaitingNextFrame) {
                fReplicasAwaitingNextFrame     = replicaBeingDeactivated->fNext;
                replicaBeingDeactivated->fNext = NULL;
            } else {
                for (StreamReplica* r2 = fReplicasAwaitingNextFrame; r2->fNext != NULL; r2 = r2->fNext) {
                    if (r2->fNext == replicaBeingDeactivated) {
                        r2->fNext = replicaBeingDeactivated->fNext;
                        replicaBeingDeactivated->fNext = NULL;
                        break;
                    }
                }
            }
        }
    }
    if (fNumActiveReplicas == 0 && fInputSource != NULL) {
        fInputSource->stopGettingFrames();
    }
} // StreamReplicator::deactivateStreamReplica

void StreamReplicator::removeStreamReplica(StreamReplica* replicaBeingRemoved)
{
    if (fNumReplicas == 0) {
        fprintf(stderr, "StreamReplicator::removeStreamReplica() Internal Error!\n");
    }
    --fNumReplicas;
    if (fNumReplicas == 0 && fDeleteWhenLastReplicaDies) {
        Medium::close(this);
        return;
    }
    if (replicaBeingRemoved->fFrameIndex != -1) {
        deactivateStreamReplica(replicaBeingRemoved);
    }
}

void StreamReplicator::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                         struct timeval presentationTime, unsigned durationInMicroseconds)
{
    ((StreamReplicator *) clientData)->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime,
                                                         durationInMicroseconds);
}

void StreamReplicator::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                                         struct timeval presentationTime, unsigned durationInMicroseconds)
{
    fMasterReplica->fFrameSize              = frameSize;
    fMasterReplica->fNumTruncatedBytes      = numTruncatedBytes;
    fMasterReplica->fPresentationTime       = presentationTime;
    fMasterReplica->fDurationInMicroseconds = durationInMicroseconds;
    deliverReceivedFrame();
}

void StreamReplicator::onSourceClosure(void* clientData)
{
    ((StreamReplicator *) clientData)->onSourceClosure();
}

void StreamReplicator::onSourceClosure()
{
    fInputSourceHasClosed = True;
    StreamReplica* replica;
    while ((replica = fReplicasAwaitingCurrentFrame) != NULL) {
        fReplicasAwaitingCurrentFrame = replica->fNext;
        replica->fNext = NULL;
        replica->handleClosure();
    }
    while ((replica = fReplicasAwaitingNextFrame) != NULL) {
        fReplicasAwaitingNextFrame = replica->fNext;
        replica->fNext = NULL;
        replica->handleClosure();
    }
    if ((replica = fMasterReplica) != NULL) {
        fMasterReplica = NULL;
        replica->handleClosure();
    }
}

void StreamReplicator::deliverReceivedFrame()
{
    StreamReplica* replica;

    while ((replica = fReplicasAwaitingCurrentFrame) != NULL) {
        fReplicasAwaitingCurrentFrame = replica->fNext;
        replica->fNext = NULL;
        replica->fDeliveryInProgress = True;
        if (fMasterReplica == NULL) {
            fprintf(stderr, "StreamReplicator::deliverReceivedFrame() Internal Error 1!\n");
        }
        StreamReplica::copyReceivedFrame(replica, fMasterReplica);
        replica->fFrameIndex = 1 - replica->fFrameIndex;
        ++fNumDeliveriesMadeSoFar;
        if (!(fNumDeliveriesMadeSoFar < fNumActiveReplicas)) {
            fprintf(stderr, "StreamReplicator::deliverReceivedFrame() Internal Error 2(%d,%d)!\n",
                    fNumDeliveriesMadeSoFar, fNumActiveReplicas);
        }
        FramedSource::afterGetting(replica);
        replica->fDeliveryInProgress = False;
    }
    if (fNumDeliveriesMadeSoFar == fNumActiveReplicas - 1 && fMasterReplica != NULL) {
        replica                 = fMasterReplica;
        fMasterReplica          = NULL;
        replica->fFrameIndex    = 1 - replica->fFrameIndex;
        fFrameIndex             = 1 - fFrameIndex;
        fNumDeliveriesMadeSoFar = 0;
        if (fReplicasAwaitingNextFrame != NULL) {
            fMasterReplica = fReplicasAwaitingNextFrame;
            fReplicasAwaitingNextFrame = fReplicasAwaitingNextFrame->fNext;
            fMasterReplica->fNext      = NULL;
            if (fInputSource != NULL) fInputSource->getNextFrame(fMasterReplica->fTo, fMasterReplica->fMaxSize,
                                                                 afterGettingFrame, this, onSourceClosure, this);
        }
        if (!(fReplicasAwaitingCurrentFrame == NULL)) {
            fprintf(stderr, "StreamReplicator::deliverReceivedFrame() Internal Error 3!\n");
        }
        fReplicasAwaitingCurrentFrame = fReplicasAwaitingNextFrame;
        fReplicasAwaitingNextFrame    = NULL;
        FramedSource::afterGetting(replica);
    }
} // StreamReplicator::deliverReceivedFrame

StreamReplica::StreamReplica(StreamReplicator& ourReplicator)
    : FramedSource(ourReplicator.envir()),
    fOurReplicator(ourReplicator),
    fFrameIndex(-1), fDeliveryInProgress(False), fNext(NULL)
{}

StreamReplica::~StreamReplica()
{
    fOurReplicator.removeStreamReplica(this);
}

void StreamReplica::doGetNextFrame()
{
    fOurReplicator.getNextFrame(this);
}

void StreamReplica::doStopGettingFrames()
{
    if (fFrameIndex != -1) {
        fFrameIndex = -1;
        fOurReplicator.deactivateStreamReplica(this);
    }
}

void StreamReplica::copyReceivedFrame(StreamReplica* toReplica, StreamReplica* fromReplica)
{
    unsigned numNewBytesToTruncate =
        toReplica->fMaxSize < fromReplica->fFrameSize ? fromReplica->fFrameSize - toReplica->fMaxSize : 0;

    toReplica->fFrameSize         = fromReplica->fFrameSize - numNewBytesToTruncate;
    toReplica->fNumTruncatedBytes = fromReplica->fNumTruncatedBytes + numNewBytesToTruncate;
    memmove(toReplica->fTo, fromReplica->fTo, toReplica->fFrameSize);
    toReplica->fPresentationTime       = fromReplica->fPresentationTime;
    toReplica->fDurationInMicroseconds = fromReplica->fDurationInMicroseconds;
}
