#include "MediaSink.hh"
#include "GroupsockHelper.hh"
#include <string.h>
MediaSink::MediaSink(UsageEnvironment& env)
    : Medium(env), fSource(NULL)
{}

MediaSink::~MediaSink()
{
    stopPlaying();
}

Boolean MediaSink::isSink() const
{
    return True;
}

Boolean MediaSink::lookupByName(UsageEnvironment& env, char const* sinkName,
                                MediaSink *& resultSink)
{
    resultSink = NULL;
    Medium* medium;
    if (!Medium::lookupByName(env, sinkName, medium)) {
        return False;
    }
    if (!medium->isSink()) {
        env.setResultMsg(sinkName, " is not a media sink");
        return False;
    }
    resultSink = (MediaSink *) medium;
    return True;
}

Boolean MediaSink::sourceIsCompatibleWithUs(MediaSource& source)
{
    return source.isFramedSource();
}

Boolean MediaSink::startPlaying(MediaSource       & source,
                                afterPlayingFunc* afterFunc,
                                void*             afterClientData)
{
    if (fSource != NULL) {
        envir().setResultMsg("This sink is already being played");
        return False;
    }
    if (!sourceIsCompatibleWithUs(source)) {
        envir().setResultMsg("MediaSink::startPlaying(): source is not compatible!");
        return False;
    }
    fSource          = (FramedSource *) &source;
    fAfterFunc       = afterFunc;
    fAfterClientData = afterClientData;
    return continuePlaying();
}

void MediaSink::stopPlaying()
{
    if (fSource != NULL) {
        fSource->stopGettingFrames();
    }
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    fSource    = NULL;
    fAfterFunc = NULL;
}

void MediaSink::onSourceClosure(void* clientData)
{
    MediaSink* sink = (MediaSink *) clientData;

    sink->onSourceClosure();
}

void MediaSink::onSourceClosure()
{
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    fSource = NULL;
    if (fAfterFunc != NULL) {
        (*fAfterFunc)(fAfterClientData);
    }
}

Boolean MediaSink::isRTPSink() const
{
    return False;
}

unsigned OutPacketBuffer::maxSize = 60000;
OutPacketBuffer
::OutPacketBuffer(unsigned preferredPacketSize, unsigned maxPacketSize, unsigned maxBufferSize)
    : fPreferred(preferredPacketSize), fMax(maxPacketSize),
    fOverflowDataSize(0)
{
    if (maxBufferSize == 0) {
        maxBufferSize = maxSize;
    }
    unsigned maxNumPackets = (maxBufferSize + (maxPacketSize - 1)) / maxPacketSize;
    fLimit = maxNumPackets * maxPacketSize;
    fBuf   = new unsigned char[fLimit];
    resetPacketStart();
    resetOffset();
    resetOverflowData();
}

OutPacketBuffer::~OutPacketBuffer()
{
    delete[] fBuf;
}

void OutPacketBuffer::enqueue(unsigned char const* from, unsigned numBytes)
{
    if (numBytes > totalBytesAvailable()) {
        #ifdef DEBUG
        fprintf(stderr, "OutPacketBuffer::enqueue() warning: %d > %d\n", numBytes, totalBytesAvailable());
        #endif
        numBytes = totalBytesAvailable();
    }
    if (curPtr() != from) {
        memmove(curPtr(), from, numBytes);
    }
    increment(numBytes);
}

void OutPacketBuffer::enqueueWord(u_int32_t word)
{
    u_int32_t nWord = htonl(word);

    enqueue((unsigned char *) &nWord, 4);
}

void OutPacketBuffer::insert(unsigned char const* from, unsigned numBytes,
                             unsigned toPosition)
{
    unsigned realToPosition = fPacketStart + toPosition;

    if (realToPosition + numBytes > fLimit) {
        if (realToPosition > fLimit) {
            return;
        }
        numBytes = fLimit - realToPosition;
    }
    memmove(&fBuf[realToPosition], from, numBytes);
    if (toPosition + numBytes > fCurOffset) {
        fCurOffset = toPosition + numBytes;
    }
}

void OutPacketBuffer::insertWord(u_int32_t word, unsigned toPosition)
{
    u_int32_t nWord = htonl(word);

    insert((unsigned char *) &nWord, 4, toPosition);
}

void OutPacketBuffer::extract(unsigned char* to, unsigned numBytes,
                              unsigned fromPosition)
{
    unsigned realFromPosition = fPacketStart + fromPosition;

    if (realFromPosition + numBytes > fLimit) {
        if (realFromPosition > fLimit) {
            return;
        }
        numBytes = fLimit - realFromPosition;
    }
    memmove(to, &fBuf[realFromPosition], numBytes);
}

u_int32_t OutPacketBuffer::extractWord(unsigned fromPosition)
{
    u_int32_t nWord;

    extract((unsigned char *) &nWord, 4, fromPosition);
    return ntohl(nWord);
}

void OutPacketBuffer::skipBytes(unsigned numBytes)
{
    if (numBytes > totalBytesAvailable()) {
        numBytes = totalBytesAvailable();
    }
    increment(numBytes);
}

void OutPacketBuffer
::setOverflowData(unsigned            overflowDataOffset,
                  unsigned            overflowDataSize,
                  struct timeval const& presentationTime,
                  unsigned            durationInMicroseconds)
{
    fOverflowDataOffset             = overflowDataOffset;
    fOverflowDataSize               = overflowDataSize;
    fOverflowPresentationTime       = presentationTime;
    fOverflowDurationInMicroseconds = durationInMicroseconds;
}

void OutPacketBuffer::useOverflowData()
{
    enqueue(&fBuf[fPacketStart + fOverflowDataOffset], fOverflowDataSize);
    fCurOffset -= fOverflowDataSize;
    resetOverflowData();
}

void OutPacketBuffer::adjustPacketStart(unsigned numBytes)
{
    fPacketStart += numBytes;
    if (fOverflowDataOffset >= numBytes) {
        fOverflowDataOffset -= numBytes;
    } else {
        fOverflowDataOffset = 0;
        fOverflowDataSize   = 0;
    }
}

void OutPacketBuffer::resetPacketStart()
{
    if (fOverflowDataSize > 0) {
        fOverflowDataOffset += fPacketStart;
    }
    fPacketStart = 0;
}
