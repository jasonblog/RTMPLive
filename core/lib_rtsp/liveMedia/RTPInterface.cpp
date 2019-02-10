#include "RTPInterface.hh"
#include <GroupsockHelper.hh>
#include <stdio.h>
static HashTable * socketHashTable(UsageEnvironment& env, Boolean createIfNotPresent = True)
{
    _Tables* ourTables = _Tables::getOurTables(env, createIfNotPresent);

    if (ourTables == NULL) {
        return NULL;
    }
    if (ourTables->socketTable == NULL) {
        ourTables->socketTable = HashTable::create(ONE_WORD_HASH_KEYS);
    }
    return (HashTable *) (ourTables->socketTable);
}

class SocketDescriptor
{
public:
    SocketDescriptor(UsageEnvironment& env, int socketNum);
    virtual ~SocketDescriptor();
    void registerRTPInterface(unsigned char streamChannelId,
                              RTPInterface* rtpInterface);
    RTPInterface * lookupRTPInterface(unsigned char streamChannelId);
    void deregisterRTPInterface(unsigned char streamChannelId);
    void setServerRequestAlternativeByteHandler(ServerRequestAlternativeByteHandler* handler, void* clientData)
    {
        fServerRequestAlternativeByteHandler = handler;
        fServerRequestAlternativeByteHandlerClientData = clientData;
    }

private:
    static void tcpReadHandler(SocketDescriptor *, int mask);
    Boolean tcpReadHandler1(int mask);
private:
    UsageEnvironment& fEnv;
    int fOurSocketNum;
    HashTable* fSubChannelHashTable;
    ServerRequestAlternativeByteHandler* fServerRequestAlternativeByteHandler;
    void* fServerRequestAlternativeByteHandlerClientData;
    u_int8_t fStreamChannelId, fSizeByte1;
    Boolean fReadErrorOccurred, fDeleteMyselfNext, fAreInReadHandlerLoop;
    enum { AWAITING_DOLLAR, AWAITING_STREAM_CHANNEL_ID, AWAITING_SIZE1, AWAITING_SIZE2,
           AWAITING_PACKET_DATA } fTCPReadingState;
};
static SocketDescriptor * lookupSocketDescriptor(UsageEnvironment& env, int sockNum, Boolean createIfNotFound = True)
{
    HashTable* table = socketHashTable(env, createIfNotFound);

    if (table == NULL) {
        return NULL;
    }
    char const* key = (char const *) (long) sockNum;
    SocketDescriptor* socketDescriptor = (SocketDescriptor *) (table->Lookup(key));
    if (socketDescriptor == NULL) {
        if (createIfNotFound) {
            socketDescriptor = new SocketDescriptor(env, sockNum);
            table->Add((char const *) (long) (sockNum), socketDescriptor);
        } else if (table->IsEmpty()) {
            _Tables* ourTables = _Tables::getOurTables(env);
            delete table;
            ourTables->socketTable = NULL;
            ourTables->reclaimIfPossible();
        }
    }
    return socketDescriptor;
}

static void removeSocketDescription(UsageEnvironment& env, int sockNum)
{
    char const* key  = (char const *) (long) sockNum;
    HashTable* table = socketHashTable(env);

    table->Remove(key);
    if (table->IsEmpty()) {
        _Tables* ourTables = _Tables::getOurTables(env);
        delete table;
        ourTables->socketTable = NULL;
        ourTables->reclaimIfPossible();
    }
}

RTPInterface::RTPInterface(Medium* owner, Groupsock* gs)
    : fOwner(owner), fGS(gs),
    fTCPStreams(NULL),
    fNextTCPReadSize(0), fNextTCPReadStreamSocketNum(-1),
    fNextTCPReadStreamChannelId(0xFF), fReadHandlerProc(NULL),
    fAuxReadHandlerFunc(NULL), fAuxReadHandlerClientData(NULL)
{
    makeSocketNonBlocking(fGS->socketNum());
    increaseSendBufferTo(envir(), fGS->socketNum(), 50 * 1024);
}

RTPInterface::~RTPInterface()
{
    stopNetworkReading();
    delete fTCPStreams;
}

void RTPInterface::setStreamSocket(int           sockNum,
                                   unsigned char streamChannelId)
{
    fGS->removeAllDestinations();
    envir().taskScheduler().disableBackgroundHandling(fGS->socketNum());
    fGS->reset();
    addStreamSocket(sockNum, streamChannelId);
}

void RTPInterface::addStreamSocket(int           sockNum,
                                   unsigned char streamChannelId)
{
    if (sockNum < 0) {
        return;
    }
    for (tcpStreamRecord* streams = fTCPStreams; streams != NULL;
         streams = streams->fNext)
    {
        if (streams->fStreamSocketNum == sockNum &&
            streams->fStreamChannelId == streamChannelId)
        {
            return;
        }
    }
    fTCPStreams = new tcpStreamRecord(sockNum, streamChannelId, fTCPStreams);
    SocketDescriptor* socketDescriptor = lookupSocketDescriptor(envir(), sockNum);
    socketDescriptor->registerRTPInterface(streamChannelId, this);
}

static void deregisterSocket(UsageEnvironment& env, int sockNum, unsigned char streamChannelId)
{
    SocketDescriptor* socketDescriptor = lookupSocketDescriptor(env, sockNum, False);

    if (socketDescriptor != NULL) {
        socketDescriptor->deregisterRTPInterface(streamChannelId);
    }
}

void RTPInterface::removeStreamSocket(int           sockNum,
                                      unsigned char streamChannelId)
{
    while (1) {
        tcpStreamRecord** streamsPtr = &fTCPStreams;
        while (*streamsPtr != NULL) {
            if ((*streamsPtr)->fStreamSocketNum == sockNum &&
                (streamChannelId == 0xFF || streamChannelId == (*streamsPtr)->fStreamChannelId))
            {
                tcpStreamRecord* next = (*streamsPtr)->fNext;
                (*streamsPtr)->fNext = NULL;
                delete (*streamsPtr);
                *streamsPtr = next;
                deregisterSocket(envir(), sockNum, streamChannelId);
                if (streamChannelId != 0xFF) {
                    return;
                }
                break;
            } else {
                streamsPtr = &((*streamsPtr)->fNext);
            }
        }
        if (*streamsPtr == NULL) {
            break;
        }
    }
}

void RTPInterface::setServerRequestAlternativeByteHandler(UsageEnvironment& env, int socketNum,
                                                          ServerRequestAlternativeByteHandler* handler,
                                                          void* clientData)
{
    SocketDescriptor* socketDescriptor = lookupSocketDescriptor(env, socketNum, False);

    if (socketDescriptor != NULL) {
        socketDescriptor->setServerRequestAlternativeByteHandler(handler, clientData);
    }
}

void RTPInterface::clearServerRequestAlternativeByteHandler(UsageEnvironment& env, int socketNum)
{
    setServerRequestAlternativeByteHandler(env, socketNum, NULL, NULL);
}

Boolean RTPInterface::sendPacket(unsigned char* packet, unsigned packetSize)
{
    Boolean success = True;

    if (!fGS->output(envir(), fGS->ttl(), packet, packetSize)) {
        success = False;
    }
    tcpStreamRecord* nextStream;
    for (tcpStreamRecord* stream = fTCPStreams; stream != NULL; stream = nextStream) {
        nextStream = stream->fNext;
        if (!sendRTPorRTCPPacketOverTCP(packet, packetSize,
                                        stream->fStreamSocketNum, stream->fStreamChannelId))
        {
            success = False;
        }
    }
    return success;
}

void RTPInterface
::startNetworkReading(TaskScheduler::BackgroundHandlerProc* handlerProc)
{
    envir().taskScheduler().
    turnOnBackgroundReadHandling(fGS->socketNum(), handlerProc, fOwner);
    fReadHandlerProc = handlerProc;
    for (tcpStreamRecord* streams = fTCPStreams; streams != NULL;
         streams = streams->fNext)
    {
        SocketDescriptor* socketDescriptor = lookupSocketDescriptor(envir(), streams->fStreamSocketNum);
        socketDescriptor->registerRTPInterface(streams->fStreamChannelId, this);
    }
}

Boolean RTPInterface::handleRead(unsigned char* buffer, unsigned bufferMaxSize,
                                 unsigned& bytesRead, struct sockaddr_in& fromAddress,
                                 int& tcpSocketNum, unsigned char& tcpStreamChannelId,
                                 Boolean& packetReadWasIncomplete)
{
    packetReadWasIncomplete = False;
    Boolean readSuccess;
    if (fNextTCPReadStreamSocketNum < 0) {
        tcpSocketNum = -1;
        readSuccess  = fGS->handleRead(buffer, bufferMaxSize, bytesRead, fromAddress);
    } else {
        tcpSocketNum       = fNextTCPReadStreamSocketNum;
        tcpStreamChannelId = fNextTCPReadStreamChannelId;
        bytesRead = 0;
        unsigned totBytesToRead = fNextTCPReadSize;
        if (totBytesToRead > bufferMaxSize) {
            totBytesToRead = bufferMaxSize;
        }
        unsigned curBytesToRead = totBytesToRead;
        int curBytesRead;
        while ((curBytesRead = readSocket(envir(), fNextTCPReadStreamSocketNum,
                                          &buffer[bytesRead], curBytesToRead,
                                          fromAddress)) > 0)
        {
            bytesRead += curBytesRead;
            if (bytesRead >= totBytesToRead) {
                break;
            }
            curBytesToRead -= curBytesRead;
        }
        fNextTCPReadSize -= bytesRead;
        if (fNextTCPReadSize == 0) {
            readSuccess = True;
        } else if (curBytesRead < 0) {
            bytesRead   = 0;
            readSuccess = False;
        } else {
            packetReadWasIncomplete = True;
            return True;
        }
        fNextTCPReadStreamSocketNum = -1;
    }
    if (readSuccess && fAuxReadHandlerFunc != NULL) {
        (*fAuxReadHandlerFunc)(fAuxReadHandlerClientData, buffer, bytesRead);
    }
    return readSuccess;
} // RTPInterface::handleRead

void RTPInterface::stopNetworkReading()
{
    envir().taskScheduler().turnOffBackgroundReadHandling(fGS->socketNum());
    for (tcpStreamRecord* streams = fTCPStreams; streams != NULL; streams = streams->fNext) {
        deregisterSocket(envir(), streams->fStreamSocketNum, streams->fStreamChannelId);
    }
}

Boolean RTPInterface::sendRTPorRTCPPacketOverTCP(u_int8_t* packet, unsigned packetSize,
                                                 int socketNum, unsigned char streamChannelId)
{
    #ifdef DEBUG_SEND
    fprintf(stderr, "sendRTPorRTCPPacketOverTCP: %d bytes over channel %d (socket %d)\n",
            packetSize, streamChannelId, socketNum);
    fflush(stderr);
    #endif
    do {
        u_int8_t framingHeader[4];
        framingHeader[0] = '$';
        framingHeader[1] = streamChannelId;
        framingHeader[2] = (u_int8_t) ((packetSize & 0xFF00) >> 8);
        framingHeader[3] = (u_int8_t) (packetSize & 0xFF);
        if (!sendDataOverTCP(socketNum, framingHeader, 4, False)) {
            break;
        }
        if (!sendDataOverTCP(socketNum, packet, packetSize, True)) {
            break;
        }
        #ifdef DEBUG_SEND
        fprintf(stderr, "sendRTPorRTCPPacketOverTCP: completed\n");
        fflush(stderr);
        #endif
        return True;
    } while (0);
    #ifdef DEBUG_SEND
    fprintf(stderr, "sendRTPorRTCPPacketOverTCP: failed! (errno %d)\n", envir().getErrno());
    fflush(stderr);
    #endif
    return False;
}

#ifndef RTPINTERFACE_BLOCKING_WRITE_TIMEOUT_MS
# define RTPINTERFACE_BLOCKING_WRITE_TIMEOUT_MS 500
#endif
Boolean RTPInterface::sendDataOverTCP(int socketNum, u_int8_t const* data, unsigned dataSize,
                                      Boolean forceSendToSucceed)
{
    int sendResult = send(socketNum, (char const *) data, dataSize, 0);

    if (sendResult < (int) dataSize) {
        unsigned numBytesSentSoFar = sendResult < 0 ? 0 : (unsigned) sendResult;
        if (numBytesSentSoFar > 0 || (forceSendToSucceed && envir().getErrno() == EAGAIN)) {
            unsigned numBytesRemainingToSend = dataSize - numBytesSentSoFar;
            #ifdef DEBUG_SEND
            fprintf(stderr, "sendDataOverTCP: resending %d-byte send (blocking)\n", numBytesRemainingToSend);
            fflush(stderr);
            #endif
            makeSocketBlocking(socketNum, RTPINTERFACE_BLOCKING_WRITE_TIMEOUT_MS);
            sendResult = send(socketNum, (char const *) (&data[numBytesSentSoFar]), numBytesRemainingToSend, 0);
            if ((unsigned) sendResult != numBytesRemainingToSend) {
                #ifdef DEBUG_SEND
                fprintf(stderr,
                        "sendDataOverTCP: blocking send() failed (delivering %d bytes out of %d); closing socket %d\n",
                        sendResult, numBytesRemainingToSend, socketNum);
                fflush(stderr);
                #endif
                removeStreamSocket(socketNum, 0xFF);
                return False;
            }
            makeSocketNonBlocking(socketNum);
            return True;
        } else if (sendResult < 0) {
            removeStreamSocket(socketNum, 0xFF);
        }
        return False;
    }
    return True;
}

SocketDescriptor::SocketDescriptor(UsageEnvironment& env, int socketNum)
    : fEnv(env), fOurSocketNum(socketNum),
    fSubChannelHashTable(HashTable::create(ONE_WORD_HASH_KEYS)),
    fServerRequestAlternativeByteHandler(NULL), fServerRequestAlternativeByteHandlerClientData(NULL),
    fReadErrorOccurred(False), fDeleteMyselfNext(False), fAreInReadHandlerLoop(False), fTCPReadingState(AWAITING_DOLLAR)
{}

SocketDescriptor::~SocketDescriptor()
{
    fEnv.taskScheduler().turnOffBackgroundReadHandling(fOurSocketNum);
    removeSocketDescription(fEnv, fOurSocketNum);
    if (fSubChannelHashTable != NULL) {
        HashTable::Iterator* iter = HashTable::Iterator::create(*fSubChannelHashTable);
        RTPInterface* rtpInterface;
        char const* key;
        while ((rtpInterface = (RTPInterface *) (iter->next(key))) != NULL) {
            u_int64_t streamChannelIdLong = (u_int64_t) key;
            unsigned char streamChannelId = (unsigned char) streamChannelIdLong;
            rtpInterface->removeStreamSocket(fOurSocketNum, streamChannelId);
        }
        delete iter;
        while (fSubChannelHashTable->RemoveNext() != NULL) {}
        delete fSubChannelHashTable;
    }
    if (fServerRequestAlternativeByteHandler != NULL) {
        u_int8_t specialChar = fReadErrorOccurred ? 0xFF : 0xFE;
        (*fServerRequestAlternativeByteHandler)(fServerRequestAlternativeByteHandlerClientData, specialChar);
    }
}

void SocketDescriptor::registerRTPInterface(unsigned char streamChannelId,
                                            RTPInterface* rtpInterface)
{
    Boolean isFirstRegistration = fSubChannelHashTable->IsEmpty();

    #if defined(DEBUG_SEND) || defined(DEBUG_RECEIVE)
    fprintf(stderr, "SocketDescriptor(socket %d)::registerRTPInterface(channel %d): isFirstRegistration %d\n",
            fOurSocketNum, streamChannelId, isFirstRegistration);
    #endif
    fSubChannelHashTable->Add((char const *) (long) streamChannelId,
                              rtpInterface);
    if (isFirstRegistration) {
        TaskScheduler::BackgroundHandlerProc* handler =
            (TaskScheduler::BackgroundHandlerProc *) &tcpReadHandler;
        fEnv.taskScheduler().
        setBackgroundHandling(fOurSocketNum, SOCKET_READABLE | SOCKET_EXCEPTION, handler, this);
    }
}

RTPInterface * SocketDescriptor
::lookupRTPInterface(unsigned char streamChannelId)
{
    char const* lookupArg = (char const *) (long) streamChannelId;

    return (RTPInterface *) (fSubChannelHashTable->Lookup(lookupArg));
}

void SocketDescriptor
::deregisterRTPInterface(unsigned char streamChannelId)
{
    #if defined(DEBUG_SEND) || defined(DEBUG_RECEIVE)
    fprintf(stderr, "SocketDescriptor(socket %d)::deregisterRTPInterface(channel %d)\n", fOurSocketNum,
            streamChannelId);
    #endif
    fSubChannelHashTable->Remove((char const *) (long) streamChannelId);
    if (fSubChannelHashTable->IsEmpty() || streamChannelId == 0xFF) {
        if (fAreInReadHandlerLoop) {
            fDeleteMyselfNext = True;
        } else {
            delete this;
        }
    }
}

void SocketDescriptor::tcpReadHandler(SocketDescriptor* socketDescriptor, int mask)
{
    unsigned count = 2000;

    socketDescriptor->fAreInReadHandlerLoop = True;
    while (!socketDescriptor->fDeleteMyselfNext && socketDescriptor->tcpReadHandler1(mask) && --count > 0) {}
    socketDescriptor->fAreInReadHandlerLoop = False;
    if (socketDescriptor->fDeleteMyselfNext) {
        delete socketDescriptor;
    }
}

Boolean SocketDescriptor::tcpReadHandler1(int mask)
{
    u_int8_t c;
    struct sockaddr_in fromAddress;

    if (fTCPReadingState != AWAITING_PACKET_DATA) {
        int result = readSocket(fEnv, fOurSocketNum, &c, 1, fromAddress);
        if (result == 0) {
            return False;
        } else if (result != 1) {
            #ifdef DEBUG_RECEIVE
            fprintf(stderr, "SocketDescriptor(socket %d)::tcpReadHandler(): readSocket(1 byte) returned %d (error)\n",
                    fOurSocketNum, result);
            #endif
            fReadErrorOccurred = True;
            fDeleteMyselfNext  = True;
            return False;
        }
    }
    Boolean callAgain = True;
    switch (fTCPReadingState) {
        case AWAITING_DOLLAR: {
            if (c == '$') {
                #ifdef DEBUG_RECEIVE
                fprintf(stderr, "SocketDescriptor(socket %d)::tcpReadHandler(): Saw '$'\n", fOurSocketNum);
                #endif
                fTCPReadingState = AWAITING_STREAM_CHANNEL_ID;
            } else {
                if (fServerRequestAlternativeByteHandler != NULL && c != 0xFF && c != 0xFE) {
                    (*fServerRequestAlternativeByteHandler)(fServerRequestAlternativeByteHandlerClientData, c);
                }
            }
            break;
        }
        case AWAITING_STREAM_CHANNEL_ID: {
            if (lookupRTPInterface(c) != NULL) {
                fStreamChannelId = c;
                fTCPReadingState = AWAITING_SIZE1;
            } else {
                #ifdef DEBUG_RECEIVE
                fprintf(stderr,
                        "SocketDescriptor(socket %d)::tcpReadHandler(): Saw nonexistent stream channel id: 0x%02x\n",
                        fOurSocketNum, c);
                #endif
                fTCPReadingState = AWAITING_DOLLAR;
            }
            break;
        }
        case AWAITING_SIZE1: {
            fSizeByte1       = c;
            fTCPReadingState = AWAITING_SIZE2;
            break;
        }
        case AWAITING_SIZE2: {
            unsigned short size        = (fSizeByte1 << 8) | c;
            RTPInterface* rtpInterface = lookupRTPInterface(fStreamChannelId);
            if (rtpInterface != NULL) {
                rtpInterface->fNextTCPReadSize = size;
                rtpInterface->fNextTCPReadStreamSocketNum = fOurSocketNum;
                rtpInterface->fNextTCPReadStreamChannelId = fStreamChannelId;
            }
            fTCPReadingState = AWAITING_PACKET_DATA;
            break;
        }
        case AWAITING_PACKET_DATA: {
            callAgain        = False;
            fTCPReadingState = AWAITING_DOLLAR;
            RTPInterface* rtpInterface = lookupRTPInterface(fStreamChannelId);
            if (rtpInterface != NULL) {
                if (rtpInterface->fNextTCPReadSize == 0) {
                    break;
                }
                if (rtpInterface->fReadHandlerProc != NULL) {
                    #ifdef DEBUG_RECEIVE
                    fprintf(stderr, "SocketDescriptor(socket %d)::tcpReadHandler(): reading %d bytes on channel %d\n",
                            fOurSocketNum, rtpInterface->fNextTCPReadSize, rtpInterface->fNextTCPReadStreamChannelId);
                    #endif
                    fTCPReadingState = AWAITING_PACKET_DATA;
                    rtpInterface->fReadHandlerProc(rtpInterface->fOwner, mask);
                } else {
                    #ifdef DEBUG_RECEIVE
                    fprintf(stderr,
                            "SocketDescriptor(socket %d)::tcpReadHandler(): No handler proc for \"rtpInterface\" for channel %d; need to skip %d remaining bytes\n", fOurSocketNum, fStreamChannelId,
                            rtpInterface->fNextTCPReadSize);
                    #endif
                    int result = readSocket(fEnv, fOurSocketNum, &c, 1, fromAddress);
                    if (result < 0) {
                        #ifdef DEBUG_RECEIVE
                        fprintf(stderr,
                                "SocketDescriptor(socket %d)::tcpReadHandler(): readSocket(1 byte) returned %d (error)\n", fOurSocketNum,
                                result);
                        #endif
                        fReadErrorOccurred = True;
                        fDeleteMyselfNext  = True;
                        return False;
                    } else {
                        fTCPReadingState = AWAITING_PACKET_DATA;
                        if (result == 1) {
                            --rtpInterface->fNextTCPReadSize;
                            callAgain = True;
                        }
                    }
                }
            }
            #ifdef DEBUG_RECEIVE
            else {
                fprintf(stderr, "SocketDescriptor(socket %d)::tcpReadHandler(): No \"rtpInterface\" for channel %d\n",
                        fOurSocketNum, fStreamChannelId);
            }
            #endif
        }
    }
    return callAgain;
} // SocketDescriptor::tcpReadHandler1

tcpStreamRecord
::tcpStreamRecord(int streamSocketNum, unsigned char streamChannelId,
                  tcpStreamRecord* next)
    : fNext(next),
    fStreamSocketNum(streamSocketNum), fStreamChannelId(streamChannelId)
{}

tcpStreamRecord::~tcpStreamRecord()
{
    delete fNext;
}
