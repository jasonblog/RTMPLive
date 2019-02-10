#include "IOHandlers.hh"
#include "TunnelEncaps.hh"
static unsigned const maxPacketLength = 50 * 1024;
static unsigned const ioBufferSize    =
    maxPacketLength + TunnelEncapsulationTrailerMaxSize;
static unsigned char ioBuffer[ioBufferSize];
void socketReadHandler(Socket* sock, int)
{
    unsigned bytesRead;
    struct sockaddr_in fromAddress;
    UsageEnvironment& saveEnv = sock->env();

    if (!sock->handleRead(ioBuffer, ioBufferSize, bytesRead, fromAddress)) {
        saveEnv.reportBackgroundError();
    }
}
