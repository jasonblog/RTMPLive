#include "MPEGVideoStreamParser.hh"
MPEGVideoStreamParser
::MPEGVideoStreamParser(MPEGVideoStreamFramer* usingSource,
                        FramedSource*          inputSource)
    : StreamParser(inputSource, FramedSource::handleClosure, usingSource,
                   &MPEGVideoStreamFramer::continueReadProcessing, usingSource),
    fUsingSource(usingSource)
{}

MPEGVideoStreamParser::~MPEGVideoStreamParser()
{}

void MPEGVideoStreamParser::restoreSavedParserState()
{
    StreamParser::restoreSavedParserState();
    fTo = fSavedTo;
    fNumTruncatedBytes = fSavedNumTruncatedBytes;
}

void MPEGVideoStreamParser::registerReadInterest(unsigned char* to,
                                                 unsigned       maxSize)
{
    fStartOfFrame      = fTo = fSavedTo = to;
    fLimit             = to + maxSize;
    fNumTruncatedBytes = fSavedNumTruncatedBytes = 0;
}
