#include "MPEG2TransportUDPServerMediaSubsession.hh"
#include "BasicUDPSource.hh"
#include "SimpleRTPSource.hh"
#include "MPEG2TransportStreamFramer.hh"
#include "SimpleRTPSink.hh"
#include "GroupsockHelper.hh"
MPEG2TransportUDPServerMediaSubsession * MPEG2TransportUDPServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                                           char const*     inputAddressStr,
                                                                                           Port const      & inputPort,
                                                                                           Boolean         inputStreamIsRawUDP)
{
    return new MPEG2TransportUDPServerMediaSubsession(env, inputAddressStr, inputPort, inputStreamIsRawUDP);
}

MPEG2TransportUDPServerMediaSubsession
::MPEG2TransportUDPServerMediaSubsession(UsageEnvironment& env,
                                         char const* inputAddressStr, Port const& inputPort,
                                         Boolean inputStreamIsRawUDP)
    : OnDemandServerMediaSubsession(env, True),
    fInputPort(inputPort), fInputGroupsock(NULL), fInputStreamIsRawUDP(inputStreamIsRawUDP)
{
    fInputAddressStr = strDup(inputAddressStr);
}

MPEG2TransportUDPServerMediaSubsession::
~MPEG2TransportUDPServerMediaSubsession()
{
    delete fInputGroupsock;
    delete[](char *) fInputAddressStr;
}

FramedSource * MPEG2TransportUDPServerMediaSubsession
::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    estBitrate = 5000;
    if (fInputGroupsock == NULL) {
        struct in_addr inputAddress;
        inputAddress.s_addr = fInputAddressStr == NULL ? 0 : our_inet_addr(fInputAddressStr);
        fInputGroupsock     = new Groupsock(envir(), inputAddress, fInputPort, 255);
    }
    FramedSource* transportStreamSource;
    if (fInputStreamIsRawUDP) {
        transportStreamSource = BasicUDPSource::createNew(envir(), fInputGroupsock);
    } else {
        transportStreamSource = SimpleRTPSource::createNew(envir(), fInputGroupsock, 33, 90000, "video/MP2T", 0, False);
    }
    return MPEG2TransportStreamFramer::createNew(envir(), transportStreamSource);
}

RTPSink * MPEG2TransportUDPServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char, FramedSource *)
{
    return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                    33, 90000, "video", "MP2T",
                                    1, True, False);
}
