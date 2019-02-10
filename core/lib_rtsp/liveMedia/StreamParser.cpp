#include "StreamParser.hh"
#include <string.h>
#include <stdlib.h>
#define BANK_SIZE 150000
void StreamParser::flushInput()
{
    fCurParserIndex = fSavedParserIndex = 0;
    fSavedRemainingUnparsedBits = fRemainingUnparsedBits = 0;
    fTotNumValidBytes = 0;
}

StreamParser::StreamParser(FramedSource*              inputSource,
                           FramedSource::onCloseFunc* onInputCloseFunc,
                           void*                      onInputCloseClientData,
                           clientContinueFunc*        clientContinueFunc,
                           void*                      clientContinueClientData)
    : fInputSource(inputSource), fClientOnInputCloseFunc(onInputCloseFunc),
    fClientOnInputCloseClientData(onInputCloseClientData),
    fClientContinueFunc(clientContinueFunc),
    fClientContinueClientData(clientContinueClientData),
    fSavedParserIndex(0), fSavedRemainingUnparsedBits(0),
    fCurParserIndex(0), fRemainingUnparsedBits(0),
    fTotNumValidBytes(0), fHaveSeenEOF(False)
{
    fBank[0]    = new unsigned char[BANK_SIZE];
    fBank[1]    = new unsigned char[BANK_SIZE];
    fCurBankNum = 0;
    fCurBank    = fBank[fCurBankNum];
    fLastSeenPresentationTime.tv_sec  = 0;
    fLastSeenPresentationTime.tv_usec = 0;
}

StreamParser::~StreamParser()
{
    delete[] fBank[0];
    delete[] fBank[1];
}

void StreamParser::saveParserState()
{
    fSavedParserIndex = fCurParserIndex;
    fSavedRemainingUnparsedBits = fRemainingUnparsedBits;
}

void StreamParser::restoreSavedParserState()
{
    fCurParserIndex        = fSavedParserIndex;
    fRemainingUnparsedBits = fSavedRemainingUnparsedBits;
}

void StreamParser::skipBits(unsigned numBits)
{
    if (numBits <= fRemainingUnparsedBits) {
        fRemainingUnparsedBits -= numBits;
    } else {
        numBits -= fRemainingUnparsedBits;
        unsigned numBytesToExamine = (numBits + 7) / 8;
        ensureValidBytes(numBytesToExamine);
        fCurParserIndex       += numBytesToExamine;
        fRemainingUnparsedBits = 8 * numBytesToExamine - numBits;
    }
}

unsigned StreamParser::getBits(unsigned numBits)
{
    if (numBits <= fRemainingUnparsedBits) {
        unsigned char lastByte = *lastParsed();
        lastByte >>= (fRemainingUnparsedBits - numBits);
        fRemainingUnparsedBits -= numBits;
        return (unsigned) lastByte & ~((~0u) << numBits);
    } else {
        unsigned char lastByte;
        if (fRemainingUnparsedBits > 0) {
            lastByte = *lastParsed();
        } else {
            lastByte = 0;
        }
        unsigned remainingBits = numBits - fRemainingUnparsedBits;
        unsigned result        = test4Bytes();
        result >>= (32 - remainingBits);
        result  |= (lastByte << remainingBits);
        if (numBits < 32) {
            result &= ~((~0u) << numBits);
        }
        unsigned const numRemainingBytes = (remainingBits + 7) / 8;
        fCurParserIndex       += numRemainingBytes;
        fRemainingUnparsedBits = 8 * numRemainingBytes - remainingBits;
        return result;
    }
}

unsigned StreamParser::bankSize() const
{
    return BANK_SIZE;
}

#define NO_MORE_BUFFERED_INPUT 1
void StreamParser::ensureValidBytes1(unsigned numBytesNeeded)
{
    unsigned maxInputFrameSize = fInputSource->maxFrameSize();

    if (maxInputFrameSize > numBytesNeeded) {
        numBytesNeeded = maxInputFrameSize;
    }
    if (fCurParserIndex + numBytesNeeded > BANK_SIZE) {
        unsigned numBytesToSave   = fTotNumValidBytes - fSavedParserIndex;
        unsigned char const* from = &curBank()[fSavedParserIndex];
        fCurBankNum = (fCurBankNum + 1) % 2;
        fCurBank    = fBank[fCurBankNum];
        memmove(curBank(), from, numBytesToSave);
        fCurParserIndex   = fCurParserIndex - fSavedParserIndex;
        fSavedParserIndex = 0;
        fTotNumValidBytes = numBytesToSave;
    }
    if (fCurParserIndex + numBytesNeeded > BANK_SIZE) {
        fInputSource->envir() << "StreamParser internal error ("
                              << fCurParserIndex << " + "
                              << numBytesNeeded << " > "
                              << BANK_SIZE << ")\n";
        fInputSource->envir().internalError();
    }
    unsigned maxNumBytesToRead = BANK_SIZE - fTotNumValidBytes;
    fInputSource->getNextFrame(&curBank()[fTotNumValidBytes],
                               maxNumBytesToRead,
                               afterGettingBytes, this,
                               onInputClosure, this);
    throw NO_MORE_BUFFERED_INPUT;
}

void StreamParser::afterGettingBytes(void*          clientData,
                                     unsigned       numBytesRead,
                                     unsigned,
                                     struct timeval presentationTime,
                                     unsigned)
{
    StreamParser* parser = (StreamParser *) clientData;

    if (parser != NULL) {
        parser->afterGettingBytes1(numBytesRead, presentationTime);
    }
}

void StreamParser::afterGettingBytes1(unsigned numBytesRead, struct timeval presentationTime)
{
    if (fTotNumValidBytes + numBytesRead > BANK_SIZE) {
        fInputSource->envir()
            << "StreamParser::afterGettingBytes() warning: read "
            << numBytesRead << " bytes; expected no more than "
            << BANK_SIZE - fTotNumValidBytes << "\n";
    }
    fLastSeenPresentationTime = presentationTime;
    unsigned char* ptr = &curBank()[fTotNumValidBytes];
    fTotNumValidBytes += numBytesRead;
    restoreSavedParserState();
    fClientContinueFunc(fClientContinueClientData, ptr, numBytesRead, presentationTime);
}

void StreamParser::onInputClosure(void* clientData)
{
    StreamParser* parser = (StreamParser *) clientData;

    if (parser != NULL) {
        parser->onInputClosure1();
    }
}

void StreamParser::onInputClosure1()
{
    if (!fHaveSeenEOF) {
        fHaveSeenEOF = True;
        afterGettingBytes1(0, fLastSeenPresentationTime);
    } else {
        fHaveSeenEOF = False;
        if (fClientOnInputCloseFunc != NULL) {
            (*fClientOnInputCloseFunc)(fClientOnInputCloseClientData);
        }
    }
}
