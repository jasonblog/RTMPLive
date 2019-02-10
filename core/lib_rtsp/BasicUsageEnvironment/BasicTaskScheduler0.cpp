#include "BasicUsageEnvironment0.hh"
#include "HandlerSet.hh"
class AlarmHandler : public DelayQueueEntry
{
public:
    AlarmHandler(TaskFunc* proc, void* clientData, DelayInterval timeToDelay)
        : DelayQueueEntry(timeToDelay), fProc(proc), fClientData(clientData)
    {}

private:
    virtual void handleTimeout()
    {
        (*fProc)(fClientData);
        DelayQueueEntry::handleTimeout();
    }

private:
    TaskFunc* fProc;
    void* fClientData;
};
BasicTaskScheduler0::BasicTaskScheduler0()
    : fLastHandledSocketNum(-1), fTriggersAwaitingHandling(0), fLastUsedTriggerMask(1), fLastUsedTriggerNum(
        MAX_NUM_EVENT_TRIGGERS - 1)
{
    fHandlers = new HandlerSet;
    for (unsigned i = 0; i < MAX_NUM_EVENT_TRIGGERS; ++i) {
        fTriggeredEventHandlers[i]    = NULL;
        fTriggeredEventClientDatas[i] = NULL;
    }
}

BasicTaskScheduler0::~BasicTaskScheduler0()
{
    delete fHandlers;
}

TaskToken BasicTaskScheduler0::scheduleDelayedTask(int64_t   microseconds,
                                                   TaskFunc* proc,
                                                   void*     clientData)
{
    if (microseconds < 0) {
        microseconds = 0;
    }
    DelayInterval timeToDelay((long) (microseconds / 1000000), (long) (microseconds % 1000000));
    AlarmHandler* alarmHandler = new AlarmHandler(proc, clientData, timeToDelay);
    fDelayQueue.addEntry(alarmHandler);
    return (void *) (alarmHandler->token());
}

void BasicTaskScheduler0::unscheduleDelayedTask(TaskToken& prevTask)
{
    DelayQueueEntry* alarmHandler = fDelayQueue.removeEntry((intptr_t) prevTask);

    prevTask = NULL;
    delete alarmHandler;
}

void BasicTaskScheduler0::doEventOnce()
{
    SingleStep();
}

void BasicTaskScheduler0::doEventLoop(char* watchVariable)
{
    while (1) {
        if (watchVariable != NULL && *watchVariable != 0) {
            break;
        }
        SingleStep();
    }
}

EventTriggerId BasicTaskScheduler0::createEventTrigger(TaskFunc* eventHandlerProc)
{
    unsigned i = fLastUsedTriggerNum;
    EventTriggerId mask = fLastUsedTriggerMask;

    do {
        i      = (i + 1) % MAX_NUM_EVENT_TRIGGERS;
        mask >>= 1;
        if (mask == 0) {
            mask = 0x80000000;
        }
        if (fTriggeredEventHandlers[i] == NULL) {
            fTriggeredEventHandlers[i]    = eventHandlerProc;
            fTriggeredEventClientDatas[i] = NULL;
            fLastUsedTriggerMask = mask;
            fLastUsedTriggerNum  = i;
            return mask;
        }
    } while (i != fLastUsedTriggerNum);
    return 0;
}

void BasicTaskScheduler0::deleteEventTrigger(EventTriggerId eventTriggerId)
{
    fTriggersAwaitingHandling &= ~eventTriggerId;
    if (eventTriggerId == fLastUsedTriggerMask) {
        fTriggeredEventHandlers[fLastUsedTriggerNum]    = NULL;
        fTriggeredEventClientDatas[fLastUsedTriggerNum] = NULL;
    } else {
        EventTriggerId mask = 0x80000000;
        for (unsigned i = 0; i < MAX_NUM_EVENT_TRIGGERS; ++i) {
            if ((eventTriggerId & mask) != 0) {
                fTriggeredEventHandlers[i]    = NULL;
                fTriggeredEventClientDatas[i] = NULL;
            }
            mask >>= 1;
        }
    }
}

void BasicTaskScheduler0::triggerEvent(EventTriggerId eventTriggerId, void* clientData)
{
    EventTriggerId mask = 0x80000000;

    for (unsigned i = 0; i < MAX_NUM_EVENT_TRIGGERS; ++i) {
        if ((eventTriggerId & mask) != 0) {
            fTriggeredEventClientDatas[i] = clientData;
        }
        mask >>= 1;
    }
    fTriggersAwaitingHandling |= eventTriggerId;
}

HandlerDescriptor::HandlerDescriptor(HandlerDescriptor* nextHandler)
    : conditionSet(0), handlerProc(NULL)
{
    if (nextHandler == this) {
        fNextHandler = fPrevHandler = this;
    } else {
        fNextHandler = nextHandler;
        fPrevHandler = nextHandler->fPrevHandler;
        nextHandler->fPrevHandler  = this;
        fPrevHandler->fNextHandler = this;
    }
}

HandlerDescriptor::~HandlerDescriptor()
{
    fNextHandler->fPrevHandler = fPrevHandler;
    fPrevHandler->fNextHandler = fNextHandler;
}

HandlerSet::HandlerSet()
    : fHandlers(&fHandlers)
{
    fHandlers.socketNum = -1;
}

HandlerSet::~HandlerSet()
{
    while (fHandlers.fNextHandler != &fHandlers) {
        delete fHandlers.fNextHandler;
    }
}

void HandlerSet
::assignHandler(int socketNum, int conditionSet, TaskScheduler::BackgroundHandlerProc* handlerProc, void* clientData)
{
    HandlerDescriptor* handler = lookupHandler(socketNum);

    if (handler == NULL) {
        handler = new HandlerDescriptor(fHandlers.fNextHandler);
        handler->socketNum = socketNum;
    }
    handler->conditionSet = conditionSet;
    handler->handlerProc  = handlerProc;
    handler->clientData   = clientData;
}

void HandlerSet::clearHandler(int socketNum)
{
    HandlerDescriptor* handler = lookupHandler(socketNum);

    delete handler;
}

void HandlerSet::moveHandler(int oldSocketNum, int newSocketNum)
{
    HandlerDescriptor* handler = lookupHandler(oldSocketNum);

    if (handler != NULL) {
        handler->socketNum = newSocketNum;
    }
}

HandlerDescriptor * HandlerSet::lookupHandler(int socketNum)
{
    HandlerDescriptor* handler;
    HandlerIterator iter(*this);

    while ((handler = iter.next()) != NULL) {
        if (handler->socketNum == socketNum) {
            break;
        }
    }
    return handler;
}

HandlerIterator::HandlerIterator(HandlerSet& handlerSet)
    : fOurSet(handlerSet)
{
    reset();
}

HandlerIterator::~HandlerIterator()
{}

void HandlerIterator::reset()
{
    fNextPtr = fOurSet.fHandlers.fNextHandler;
}

HandlerDescriptor * HandlerIterator::next()
{
    HandlerDescriptor* result = fNextPtr;

    if (result == &fOurSet.fHandlers) {
        result = NULL;
    } else {
        fNextPtr = fNextPtr->fNextHandler;
    }
    return result;
}
