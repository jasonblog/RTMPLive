#include "TCPStreamSink.hh"
#include "RTSPCommon.hh"
TCPStreamSink * TCPStreamSink::createNew(UsageEnvironment& env, int socketNum)
{
    return new TCPStreamSink(env, socketNum);
}

TCPStreamSink::TCPStreamSink(UsageEnvironment& env, int socketNum)
    : MediaSink(env),
    fUnwrittenBytesStart(0), fUnwrittenBytesEnd(0),
    fInputSourceIsOpen(False), fOutputSocketIsWritable(True),
    fOutputSocketNum(socketNum)
{
    ignoreSigPipeOnSocket(socketNum);
}

TCPStreamSink::~TCPStreamSink()
{
    envir().taskScheduler().disableBackgroundHandling(fOutputSocketNum);
}

Boolean TCPStreamSink::continuePlaying()
{
    fInputSourceIsOpen = fSource != NULL;
    processBuffer();
    return True;
}

#define TCP_STREAM_SINK_MIN_READ_SIZE 1000
void TCPStreamSink::processBuffer()
{
    if (fOutputSocketIsWritable && numUnwrittenBytes() > 0) {
        int numBytesWritten =
            send(fOutputSocketNum, (const char *) &fBuffer[fUnwrittenBytesStart], numUnwrittenBytes(), 0);
        if (numBytesWritten < (int) numUnwrittenBytes()) {
            fOutputSocketIsWritable = False;
            if (envir().getErrno() != EPIPE) {
                envir().taskScheduler().setBackgroundHandling(fOutputSocketNum, SOCKET_WRITABLE, socketWritableHandler,
                                                              this);
            }
        }
        if (numBytesWritten > 0) {
            fUnwrittenBytesStart += numBytesWritten;
            if (fUnwrittenBytesStart > fUnwrittenBytesEnd) {
                fUnwrittenBytesStart = fUnwrittenBytesEnd;
            }
            if (fUnwrittenBytesStart == fUnwrittenBytesEnd &&
                (!fInputSourceIsOpen || !fSource->isCurrentlyAwaitingData()))
            {
                fUnwrittenBytesStart = fUnwrittenBytesEnd = 0;
            }
        }
    }
    if (fInputSourceIsOpen && freeBufferSpace() >= TCP_STREAM_SINK_MIN_READ_SIZE &&
        !fSource->isCurrentlyAwaitingData())
    {
        fSource->getNextFrame(&fBuffer[fUnwrittenBytesEnd],
                              freeBufferSpace(), afterGettingFrame, this, ourOnSourceClosure, this);
    } else if (!fInputSourceIsOpen && numUnwrittenBytes() == 0) {
        onSourceClosure();
    }
}

void TCPStreamSink::socketWritableHandler(void* clientData, int)
{
    TCPStreamSink* sink = (TCPStreamSink *) clientData;

    sink->socketWritableHandler1();
}

void TCPStreamSink::socketWritableHandler1()
{
    envir().taskScheduler().disableBackgroundHandling(fOutputSocketNum);
    fOutputSocketIsWritable = True;
    processBuffer();
}

void TCPStreamSink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                      struct timeval, unsigned)
{
    TCPStreamSink* sink = (TCPStreamSink *) clientData;

    sink->afterGettingFrame(frameSize, numTruncatedBytes);
}

void TCPStreamSink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes)
{
    if (numTruncatedBytes > 0) {
        envir() << "TCPStreamSink::afterGettingFrame(): The input frame data was too large for our buffer.  "
                << numTruncatedBytes
                <<
            " bytes of trailing data was dropped!  Correct this by increasing the definition of \"TCP_STREAM_SINK_BUFFER_SIZE\" in \"include/TCPStreamSink.hh\".\n";
    }
    fUnwrittenBytesEnd += frameSize;
    processBuffer();
}

void TCPStreamSink::ourOnSourceClosure(void* clientData)
{
    TCPStreamSink* sink = (TCPStreamSink *) clientData;

    sink->ourOnSourceClosure1();
}

void TCPStreamSink::ourOnSourceClosure1()
{
    fInputSourceIsOpen = False;
    processBuffer();
}
