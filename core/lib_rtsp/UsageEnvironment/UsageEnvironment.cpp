#include "UsageEnvironment.hh"
Boolean UsageEnvironment::reclaim()
{
    if (liveMediaPriv == NULL && groupsockPriv == NULL) {
        delete this;
        return True;
    }
    return False;
}

UsageEnvironment::UsageEnvironment(TaskScheduler& scheduler)
    : liveMediaPriv(NULL), groupsockPriv(NULL), fScheduler(scheduler)
{}

UsageEnvironment::~UsageEnvironment()
{}

void UsageEnvironment::internalError()
{
    abort();
}

TaskScheduler::TaskScheduler()
{}

TaskScheduler::~TaskScheduler()
{}

void TaskScheduler::rescheduleDelayedTask(TaskToken& task,
                                          int64_t microseconds, TaskFunc* proc,
                                          void* clientData)
{
    unscheduleDelayedTask(task);
    task = scheduleDelayedTask(microseconds, proc, clientData);
}

void TaskScheduler::internalError()
{
    abort();
}
