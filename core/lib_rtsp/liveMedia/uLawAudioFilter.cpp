#include "uLawAudioFilter.hh"
uLawFromPCMAudioSource * uLawFromPCMAudioSource
::createNew(UsageEnvironment& env, FramedSource* inputSource, int byteOrdering)
{
    if (byteOrdering < 0 || byteOrdering > 2) {
        env.setResultMsg("uLawFromPCMAudioSource::createNew(): bad \"byteOrdering\" parameter");
        return NULL;
    }
    return new uLawFromPCMAudioSource(env, inputSource, byteOrdering);
}

uLawFromPCMAudioSource
::uLawFromPCMAudioSource(UsageEnvironment& env, FramedSource* inputSource,
                         int byteOrdering)
    : FramedFilter(env, inputSource),
    fByteOrdering(byteOrdering), fInputBuffer(NULL), fInputBufferSize(0)
{}

uLawFromPCMAudioSource::~uLawFromPCMAudioSource()
{
    delete[] fInputBuffer;
}

void uLawFromPCMAudioSource::doGetNextFrame()
{
    unsigned bytesToRead = fMaxSize * 2;

    if (bytesToRead > fInputBufferSize) {
        delete[] fInputBuffer;
        fInputBuffer     = new unsigned char[bytesToRead];
        fInputBufferSize = bytesToRead;
    }
    fInputSource->getNextFrame(fInputBuffer, bytesToRead,
                               afterGettingFrame, this,
                               FramedSource::handleClosure, this);
}

void uLawFromPCMAudioSource
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds)
{
    uLawFromPCMAudioSource* source = (uLawFromPCMAudioSource *) clientData;

    source->afterGettingFrame1(frameSize, numTruncatedBytes,
                               presentationTime, durationInMicroseconds);
}

#define BIAS 0x84
#define CLIP 32635
static unsigned char uLawFrom16BitLinear(u_int16_t sample)
{
    static int const exp_lut[256] = { 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
                                      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                                      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                                      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                                      6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                      6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                      6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                      6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                      7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
    };
    unsigned char sign = (sample >> 8) & 0x80;

    if (sign != 0) {
        sample = -sample;
    }
    if (sample > CLIP) {
        sample = CLIP;
    }
    sample += BIAS;
    unsigned char exponent = exp_lut[(sample >> 7) & 0xFF];
    unsigned char mantissa = (sample >> (exponent + 3)) & 0x0F;
    unsigned char result   = ~(sign | (exponent << 4) | mantissa);
    if (result == 0) {
        result = 0x02;
    }
    return result;
} // uLawFrom16BitLinear

void uLawFromPCMAudioSource
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                     struct timeval presentationTime,
                     unsigned durationInMicroseconds)
{
    unsigned numSamples = frameSize / 2;

    switch (fByteOrdering) {
        case 0: {
            u_int16_t* inputSample = (u_int16_t *) fInputBuffer;
            for (unsigned i = 0; i < numSamples; ++i) {
                fTo[i] = uLawFrom16BitLinear(inputSample[i]);
            }
            break;
        }
        case 1: {
            for (unsigned i = 0; i < numSamples; ++i) {
                u_int16_t const newValue = (fInputBuffer[2 * i + 1] << 8) | fInputBuffer[2 * i];
                fTo[i] = uLawFrom16BitLinear(newValue);
            }
            break;
        }
        case 2: {
            for (unsigned i = 0; i < numSamples; ++i) {
                u_int16_t const newValue = (fInputBuffer[2 * i] << 8) | fInputBuffer[2 * i + i];
                fTo[i] = uLawFrom16BitLinear(newValue);
            }
            break;
        }
    }
    fFrameSize              = numSamples;
    fNumTruncatedBytes      = numTruncatedBytes;
    fPresentationTime       = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    afterGetting(this);
}

PCMFromuLawAudioSource * PCMFromuLawAudioSource
::createNew(UsageEnvironment& env, FramedSource* inputSource)
{
    return new PCMFromuLawAudioSource(env, inputSource);
}

PCMFromuLawAudioSource
::PCMFromuLawAudioSource(UsageEnvironment& env,
                         FramedSource*   inputSource)
    : FramedFilter(env, inputSource),
    fInputBuffer(NULL), fInputBufferSize(0)
{}

PCMFromuLawAudioSource::~PCMFromuLawAudioSource()
{
    delete[] fInputBuffer;
}

void PCMFromuLawAudioSource::doGetNextFrame()
{
    unsigned bytesToRead = fMaxSize / 2;

    if (bytesToRead > fInputBufferSize) {
        delete[] fInputBuffer;
        fInputBuffer     = new unsigned char[bytesToRead];
        fInputBufferSize = bytesToRead;
    }
    fInputSource->getNextFrame(fInputBuffer, bytesToRead,
                               afterGettingFrame, this,
                               FramedSource::handleClosure, this);
}

void PCMFromuLawAudioSource
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds)
{
    PCMFromuLawAudioSource* source = (PCMFromuLawAudioSource *) clientData;

    source->afterGettingFrame1(frameSize, numTruncatedBytes,
                               presentationTime, durationInMicroseconds);
}

static u_int16_t linear16FromuLaw(unsigned char uLawByte)
{
    static int const exp_lut[8] = { 0, 132, 396, 924, 1980, 4092, 8316, 16764 };

    uLawByte = ~uLawByte;
    Boolean sign = (uLawByte & 0x80) != 0;
    unsigned char exponent = (uLawByte >> 4) & 0x07;
    unsigned char mantissa = uLawByte & 0x0F;
    u_int16_t result       = exp_lut[exponent] + (mantissa << (exponent + 3));
    if (sign) {
        result = -result;
    }
    return result;
}

void PCMFromuLawAudioSource
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                     struct timeval presentationTime,
                     unsigned durationInMicroseconds)
{
    unsigned numSamples     = frameSize;
    u_int16_t* outputSample = (u_int16_t *) fTo;

    for (unsigned i = 0; i < numSamples; ++i) {
        outputSample[i] = linear16FromuLaw(fInputBuffer[i]);
    }
    fFrameSize              = numSamples * 2;
    fNumTruncatedBytes      = numTruncatedBytes;
    fPresentationTime       = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    afterGetting(this);
}

NetworkFromHostOrder16 * NetworkFromHostOrder16
::createNew(UsageEnvironment& env, FramedSource* inputSource)
{
    return new NetworkFromHostOrder16(env, inputSource);
}

NetworkFromHostOrder16
::NetworkFromHostOrder16(UsageEnvironment& env,
                         FramedSource*   inputSource)
    : FramedFilter(env, inputSource)
{}

NetworkFromHostOrder16::~NetworkFromHostOrder16()
{}

void NetworkFromHostOrder16::doGetNextFrame()
{
    fInputSource->getNextFrame(fTo, fMaxSize,
                               afterGettingFrame, this,
                               FramedSource::handleClosure, this);
}

void NetworkFromHostOrder16
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds)
{
    NetworkFromHostOrder16* source = (NetworkFromHostOrder16 *) clientData;

    source->afterGettingFrame1(frameSize, numTruncatedBytes,
                               presentationTime, durationInMicroseconds);
}

void NetworkFromHostOrder16
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                     struct timeval presentationTime,
                     unsigned durationInMicroseconds)
{
    unsigned numValues = frameSize / 2;
    u_int16_t* value   = (u_int16_t *) fTo;

    for (unsigned i = 0; i < numValues; ++i) {
        value[i] = htons(value[i]);
    }
    fFrameSize              = numValues * 2;
    fNumTruncatedBytes      = numTruncatedBytes;
    fPresentationTime       = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    afterGetting(this);
}

HostFromNetworkOrder16 * HostFromNetworkOrder16
::createNew(UsageEnvironment& env, FramedSource* inputSource)
{
    return new HostFromNetworkOrder16(env, inputSource);
}

HostFromNetworkOrder16
::HostFromNetworkOrder16(UsageEnvironment& env,
                         FramedSource*   inputSource)
    : FramedFilter(env, inputSource)
{}

HostFromNetworkOrder16::~HostFromNetworkOrder16()
{}

void HostFromNetworkOrder16::doGetNextFrame()
{
    fInputSource->getNextFrame(fTo, fMaxSize,
                               afterGettingFrame, this,
                               FramedSource::handleClosure, this);
}

void HostFromNetworkOrder16
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds)
{
    HostFromNetworkOrder16* source = (HostFromNetworkOrder16 *) clientData;

    source->afterGettingFrame1(frameSize, numTruncatedBytes,
                               presentationTime, durationInMicroseconds);
}

void HostFromNetworkOrder16
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                     struct timeval presentationTime,
                     unsigned durationInMicroseconds)
{
    unsigned numValues = frameSize / 2;
    u_int16_t* value   = (u_int16_t *) fTo;

    for (unsigned i = 0; i < numValues; ++i) {
        value[i] = ntohs(value[i]);
    }
    fFrameSize              = numValues * 2;
    fNumTruncatedBytes      = numTruncatedBytes;
    fPresentationTime       = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    afterGetting(this);
}

EndianSwap16 * EndianSwap16::createNew(UsageEnvironment& env, FramedSource* inputSource)
{
    return new EndianSwap16(env, inputSource);
}

EndianSwap16::EndianSwap16(UsageEnvironment& env,
                           FramedSource*   inputSource)
    : FramedFilter(env, inputSource)
{}

EndianSwap16::~EndianSwap16()
{}

void EndianSwap16::doGetNextFrame()
{
    fInputSource->getNextFrame(fTo, fMaxSize,
                               afterGettingFrame, this,
                               FramedSource::handleClosure, this);
}

void EndianSwap16::afterGettingFrame(void* clientData, unsigned frameSize,
                                     unsigned numTruncatedBytes,
                                     struct timeval presentationTime,
                                     unsigned durationInMicroseconds)
{
    EndianSwap16* source = (EndianSwap16 *) clientData;

    source->afterGettingFrame1(frameSize, numTruncatedBytes,
                               presentationTime, durationInMicroseconds);
}

void EndianSwap16::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                                      struct timeval presentationTime,
                                      unsigned durationInMicroseconds)
{
    unsigned numValues = frameSize / 2;
    u_int16_t* value   = (u_int16_t *) fTo;

    for (unsigned i = 0; i < numValues; ++i) {
        u_int16_t const orig = value[i];
        value[i] = ((orig & 0xFF) << 8) | ((orig & 0xFF00) >> 8);
    }
    fFrameSize              = numValues * 2;
    fNumTruncatedBytes      = numTruncatedBytes + (frameSize - fFrameSize);
    fPresentationTime       = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    afterGetting(this);
}

EndianSwap24 * EndianSwap24::createNew(UsageEnvironment& env, FramedSource* inputSource)
{
    return new EndianSwap24(env, inputSource);
}

EndianSwap24::EndianSwap24(UsageEnvironment& env,
                           FramedSource*   inputSource)
    : FramedFilter(env, inputSource)
{}

EndianSwap24::~EndianSwap24()
{}

void EndianSwap24::doGetNextFrame()
{
    fInputSource->getNextFrame(fTo, fMaxSize,
                               afterGettingFrame, this,
                               FramedSource::handleClosure, this);
}

void EndianSwap24::afterGettingFrame(void* clientData, unsigned frameSize,
                                     unsigned numTruncatedBytes,
                                     struct timeval presentationTime,
                                     unsigned durationInMicroseconds)
{
    EndianSwap24* source = (EndianSwap24 *) clientData;

    source->afterGettingFrame1(frameSize, numTruncatedBytes,
                               presentationTime, durationInMicroseconds);
}

void EndianSwap24::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                                      struct timeval presentationTime,
                                      unsigned durationInMicroseconds)
{
    unsigned const numValues = frameSize / 3;
    u_int8_t* p = fTo;

    for (unsigned i = 0; i < numValues; ++i) {
        u_int8_t tmp = p[0];
        p[0] = p[2];
        p[2] = tmp;
        p   += 3;
    }
    fFrameSize              = numValues * 3;
    fNumTruncatedBytes      = numTruncatedBytes + (frameSize - fFrameSize);
    fPresentationTime       = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    afterGetting(this);
}
