#include "BasicUDPSource.hh"
#include <GroupsockHelper.hh>
BasicUDPSource * BasicUDPSource::createNew(UsageEnvironment& env,
                                           Groupsock*      inputGS)
{
    return new BasicUDPSource(env, inputGS);
}

BasicUDPSource::BasicUDPSource(UsageEnvironment& env, Groupsock* inputGS)
    : FramedSource(env), fInputGS(inputGS), fHaveStartedReading(False)
{
    increaseReceiveBufferTo(env, inputGS->socketNum(), 50 * 1024);
    makeSocketNonBlocking(fInputGS->socketNum());
}

BasicUDPSource::~BasicUDPSource()
{
    envir().taskScheduler().turnOffBackgroundReadHandling(fInputGS->socketNum());
}

void BasicUDPSource::doGetNextFrame()
{
    if (!fHaveStartedReading) {
        envir().taskScheduler().turnOnBackgroundReadHandling(
            fInputGS->socketNum(),
            (TaskScheduler::BackgroundHandlerProc *) &incomingPacketHandler,
            this);
        fHaveStartedReading = True;
    }
}

void BasicUDPSource::doStopGettingFrames()
{
    envir().taskScheduler().turnOffBackgroundReadHandling(fInputGS->socketNum());
    fHaveStartedReading = False;
}

void BasicUDPSource::incomingPacketHandler(BasicUDPSource* source, int)
{
    source->incomingPacketHandler1();
}

void BasicUDPSource::incomingPacketHandler1()
{
    if (!isCurrentlyAwaitingData()) {
        return;
    }
    struct sockaddr_in fromAddress;
    if (!fInputGS->handleRead(fTo, fMaxSize, fFrameSize, fromAddress)) {
        return;
    }
    afterGetting(this);
}
