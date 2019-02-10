#include "WAVAudioFileServerMediaSubsession.hh"
#include "WAVAudioFileSource.hh"
#include "uLawAudioFilter.hh"
#include "SimpleRTPSink.hh"
WAVAudioFileServerMediaSubsession * WAVAudioFileServerMediaSubsession
::createNew(UsageEnvironment& env, char const* fileName, Boolean reuseFirstSource,
            Boolean convertToULaw)
{
    return new WAVAudioFileServerMediaSubsession(env, fileName,
                                                 reuseFirstSource, convertToULaw);
}

WAVAudioFileServerMediaSubsession
::WAVAudioFileServerMediaSubsession(UsageEnvironment& env, char const* fileName,
                                    Boolean reuseFirstSource, Boolean convertToULaw)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource),
    fConvertToULaw(convertToULaw)
{}

WAVAudioFileServerMediaSubsession
::~WAVAudioFileServerMediaSubsession()
{}

void WAVAudioFileServerMediaSubsession
::seekStreamSource(FramedSource* inputSource, double& seekNPT, double streamDuration, u_int64_t& numBytes)
{
    WAVAudioFileSource* wavSource;

    if (fBitsPerSample > 8) {
        wavSource = (WAVAudioFileSource *) (((FramedFilter *) inputSource)->inputSource());
    } else {
        wavSource = (WAVAudioFileSource *) inputSource;
    }
    unsigned seekSampleNumber = (unsigned) (seekNPT * fSamplingFrequency);
    unsigned seekByteNumber   = seekSampleNumber * ((fNumChannels * fBitsPerSample) / 8);
    wavSource->seekToPCMByte(seekByteNumber);
    setStreamSourceDuration(inputSource, streamDuration, numBytes);
}

void WAVAudioFileServerMediaSubsession
::setStreamSourceDuration(FramedSource* inputSource, double streamDuration, u_int64_t& numBytes)
{
    WAVAudioFileSource* wavSource;

    if (fBitsPerSample > 8) {
        wavSource = (WAVAudioFileSource *) (((FramedFilter *) inputSource)->inputSource());
    } else {
        wavSource = (WAVAudioFileSource *) inputSource;
    }
    unsigned numDurationSamples = (unsigned) (streamDuration * fSamplingFrequency);
    unsigned numDurationBytes   = numDurationSamples * ((fNumChannels * fBitsPerSample) / 8);
    numBytes = (u_int64_t) numDurationBytes;
    wavSource->limitNumBytesToStream(numDurationBytes);
}

void WAVAudioFileServerMediaSubsession
::setStreamSourceScale(FramedSource* inputSource, float scale)
{
    int iScale = (int) scale;
    WAVAudioFileSource* wavSource;

    if (fBitsPerSample > 8) {
        wavSource = (WAVAudioFileSource *) (((FramedFilter *) inputSource)->inputSource());
    } else {
        wavSource = (WAVAudioFileSource *) inputSource;
    }
    wavSource->setScaleFactor(iScale);
}

FramedSource * WAVAudioFileServerMediaSubsession
::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    FramedSource* resultSource = NULL;

    do {
        WAVAudioFileSource* wavSource = WAVAudioFileSource::createNew(envir(), fFileName);
        if (wavSource == NULL) {
            break;
        }
        fAudioFormat   = wavSource->getAudioFormat();
        fBitsPerSample = wavSource->bitsPerSample();
        if (fBitsPerSample % 4 != 0 || fBitsPerSample < 4 || fBitsPerSample > 24 || fBitsPerSample == 12) {
            envir() << "The input file contains " << fBitsPerSample << " bit-per-sample audio, which we don't handle\n";
            break;
        }
        fSamplingFrequency = wavSource->samplingFrequency();
        fNumChannels       = wavSource->numChannels();
        unsigned bitsPerSecond = fSamplingFrequency * fBitsPerSample * fNumChannels;
        fFileDuration =
            (float) ((8.0 * wavSource->numPCMBytes()) / (fSamplingFrequency * fNumChannels * fBitsPerSample));
        resultSource = wavSource;
        if (fAudioFormat == WA_PCM) {
            if (fBitsPerSample == 16) {
                if (fConvertToULaw) {
                    resultSource   = uLawFromPCMAudioSource::createNew(envir(), wavSource, 1);
                    bitsPerSecond /= 2;
                } else {
                    resultSource = EndianSwap16::createNew(envir(), wavSource);
                }
            } else if (fBitsPerSample == 20 || fBitsPerSample == 24) {
                resultSource = EndianSwap24::createNew(envir(), wavSource);
            }
        }
        estBitrate = (bitsPerSecond + 500) / 1000;
        return resultSource;
    } while (0);
    Medium::close(resultSource);
    return NULL;
} // WAVAudioFileServerMediaSubsession::createNewStreamSource

RTPSink * WAVAudioFileServerMediaSubsession
::createNewRTPSink(Groupsock*    rtpGroupsock,
                   unsigned char rtpPayloadTypeIfDynamic,
                   FramedSource *)
{
    do {
        char const* mimeType;
        unsigned char payloadFormatCode = rtpPayloadTypeIfDynamic;
        if (fAudioFormat == WA_PCM) {
            if (fBitsPerSample == 16) {
                if (fConvertToULaw) {
                    mimeType = "PCMU";
                    if (fSamplingFrequency == 8000 && fNumChannels == 1) {
                        payloadFormatCode = 0;
                    }
                } else {
                    mimeType = "L16";
                    if (fSamplingFrequency == 44100 && fNumChannels == 2) {
                        payloadFormatCode = 10;
                    } else if (fSamplingFrequency == 44100 && fNumChannels == 1) {
                        payloadFormatCode = 11;
                    }
                }
            } else if (fBitsPerSample == 20) {
                mimeType = "L20";
            } else if (fBitsPerSample == 24) {
                mimeType = "L24";
            } else {
                mimeType = "L8";
            }
        } else if (fAudioFormat == WA_PCMU) {
            mimeType = "PCMU";
            if (fSamplingFrequency == 8000 && fNumChannels == 1) {
                payloadFormatCode = 0;
            }
        } else if (fAudioFormat == WA_PCMA) {
            mimeType = "PCMA";
            if (fSamplingFrequency == 8000 && fNumChannels == 1) {
                payloadFormatCode = 8;
            }
        } else if (fAudioFormat == WA_IMA_ADPCM) {
            mimeType = "DVI4";
            if (fNumChannels == 1) {
                if (fSamplingFrequency == 8000) {
                    payloadFormatCode = 5;
                } else if (fSamplingFrequency == 16000) {
                    payloadFormatCode = 6;
                } else if (fSamplingFrequency == 11025) {
                    payloadFormatCode = 16;
                } else if (fSamplingFrequency == 22050) {
                    payloadFormatCode = 17;
                }
            }
        } else {
            break;
        }
        return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                        payloadFormatCode, fSamplingFrequency,
                                        "audio", mimeType, fNumChannels);
    } while (0);
    return NULL;
} // WAVAudioFileServerMediaSubsession::createNewRTPSink

void WAVAudioFileServerMediaSubsession::testScaleFactor(float& scale)
{
    if (fFileDuration <= 0.0) {
        scale = 1;
    } else {
        int iScale = scale < 0.0 ? (int) (scale - 0.5) : (int) (scale + 0.5);
        if (iScale == 0) {
            iScale = 1;
        }
        scale = (float) iScale;
    }
}

float WAVAudioFileServerMediaSubsession::duration() const
{
    return fFileDuration;
}
