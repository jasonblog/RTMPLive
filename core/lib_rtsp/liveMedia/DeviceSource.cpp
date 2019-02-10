#include "DeviceSource.hh"
#include <GroupsockHelper.hh>
DeviceSource * DeviceSource::createNew(UsageEnvironment & env,
                                       DeviceParameters params)
{
    return new DeviceSource(env, params);
}

EventTriggerId DeviceSource::eventTriggerId = 0;
unsigned DeviceSource::referenceCount       = 0;
DeviceSource::DeviceSource(UsageEnvironment & env,
                           DeviceParameters params)
    : FramedSource(env), fParams(params)
{
    if (referenceCount == 0) {}
    ++referenceCount;
    if (eventTriggerId == 0) {
        eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    }
}

DeviceSource::~DeviceSource()
{
    --referenceCount;
    if (referenceCount == 0) {
        envir().taskScheduler().deleteEventTrigger(eventTriggerId);
        eventTriggerId = 0;
    }
}

void DeviceSource::doGetNextFrame()
{
    if (0) {
        handleClosure();
        return;
    }
    if (0) {
        deliverFrame();
    }
}

void DeviceSource::deliverFrame0(void* clientData)
{
    ((DeviceSource *) clientData)->deliverFrame();
}

void DeviceSource::deliverFrame()
{
    if (!isCurrentlyAwaitingData()) {
        return;
    }
    u_int8_t* newFrameDataStart = (u_int8_t *) 0xDEADBEEF;
    unsigned newFrameSize       = 0;
    if (newFrameSize > fMaxSize) {
        fFrameSize         = fMaxSize;
        fNumTruncatedBytes = newFrameSize - fMaxSize;
    } else {
        fFrameSize = newFrameSize;
    }
    gettimeofday(&fPresentationTime, NULL);
    memmove(fTo, newFrameDataStart, fFrameSize);
    FramedSource::afterGetting(this);
}

void signalNewFrameData()
{
    TaskScheduler* ourScheduler = NULL;
    DeviceSource* ourDevice     = NULL;

    if (ourScheduler != NULL) {
        ourScheduler->triggerEvent(DeviceSource::eventTriggerId, ourDevice);
    }
}
