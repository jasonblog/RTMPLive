#include "ByteStreamMultiFileSource.hh"
ByteStreamMultiFileSource
::ByteStreamMultiFileSource(UsageEnvironment& env, char const** fileNameArray,
                            unsigned preferredFrameSize, unsigned playTimePerFrame)
    : FramedSource(env),
    fPreferredFrameSize(preferredFrameSize), fPlayTimePerFrame(playTimePerFrame),
    fCurrentlyReadSourceNumber(0), fHaveStartedNewFile(False)
{
    for (fNumSources = 0;; ++fNumSources) {
        if (fileNameArray[fNumSources] == NULL) {
            break;
        }
    }
    fFileNameArray = new char const *[fNumSources];
    if (fFileNameArray == NULL) {
        return;
    }
    unsigned i;
    for (i = 0; i < fNumSources; ++i) {
        fFileNameArray[i] = strDup(fileNameArray[i]);
    }
    fSourceArray = new ByteStreamFileSource *[fNumSources];
    if (fSourceArray == NULL) {
        return;
    }
    for (i = 0; i < fNumSources; ++i) {
        fSourceArray[i] = NULL;
    }
}

ByteStreamMultiFileSource::~ByteStreamMultiFileSource()
{
    unsigned i;

    for (i = 0; i < fNumSources; ++i) {
        Medium::close(fSourceArray[i]);
    }
    delete[] fSourceArray;
    for (i = 0; i < fNumSources; ++i) {
        delete[](char *) (fFileNameArray[i]);
    }
    delete[] fFileNameArray;
}

ByteStreamMultiFileSource * ByteStreamMultiFileSource
::createNew(UsageEnvironment& env, char const** fileNameArray,
            unsigned preferredFrameSize, unsigned playTimePerFrame)
{
    ByteStreamMultiFileSource* newSource =
        new ByteStreamMultiFileSource(env, fileNameArray,
                                      preferredFrameSize, playTimePerFrame);

    return newSource;
}

void ByteStreamMultiFileSource::doGetNextFrame()
{
    do {
        if (fCurrentlyReadSourceNumber >= fNumSources) {
            break;
        }
        fHaveStartedNewFile = False;
        ByteStreamFileSource *& source =
            fSourceArray[fCurrentlyReadSourceNumber];
        if (source == NULL) {
            source = ByteStreamFileSource::createNew(envir(),
                                                     fFileNameArray[fCurrentlyReadSourceNumber],
                                                     fPreferredFrameSize, fPlayTimePerFrame);
            if (source == NULL) {
                break;
            }
            fHaveStartedNewFile = True;
        }
        source->getNextFrame(fTo, fMaxSize,
                             afterGettingFrame, this,
                             onSourceClosure, this);
        return;
    } while (0);
    handleClosure();
}

void ByteStreamMultiFileSource
::afterGettingFrame(void* clientData,
                    unsigned frameSize, unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds)
{
    ByteStreamMultiFileSource* source =
        (ByteStreamMultiFileSource *) clientData;

    source->fFrameSize              = frameSize;
    source->fNumTruncatedBytes      = numTruncatedBytes;
    source->fPresentationTime       = presentationTime;
    source->fDurationInMicroseconds = durationInMicroseconds;
    FramedSource::afterGetting(source);
}

void ByteStreamMultiFileSource::onSourceClosure(void* clientData)
{
    ByteStreamMultiFileSource* source =
        (ByteStreamMultiFileSource *) clientData;

    source->onSourceClosure1();
}

void ByteStreamMultiFileSource::onSourceClosure1()
{
    ByteStreamFileSource *& source =
        fSourceArray[fCurrentlyReadSourceNumber++];

    Medium::close(source);
    source = NULL;
    doGetNextFrame();
}
