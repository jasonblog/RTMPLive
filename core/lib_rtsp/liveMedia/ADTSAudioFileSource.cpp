#include "ADTSAudioFileSource.hh"
#include "InputFile.hh"
#include <GroupsockHelper.hh>
static unsigned const samplingFrequencyTable[16] = {
    96000, 88200, 64000, 48000,
    44100, 32000, 24000, 22050,
    16000, 12000, 11025, 8000,
    7350,  0,     0,     0
};
ADTSAudioFileSource * ADTSAudioFileSource::createNew(UsageEnvironment& env, char const* fileName)
{
    FILE* fid = NULL;

    do {
        fid = OpenInputFile(env, fileName);
        if (fid == NULL) {
            break;
        }
        unsigned char fixedHeader[4];
        if (fread(fixedHeader, 1, sizeof fixedHeader, fid) < sizeof fixedHeader) {
            break;
        }
        if (!(fixedHeader[0] == 0xFF && (fixedHeader[1] & 0xF0) == 0xF0)) {
            env.setResultMsg("Bad 'syncword' at start of ADTS file");
            break;
        }
        u_int8_t profile = (fixedHeader[2] & 0xC0) >> 6;
        if (profile == 3) {
            env.setResultMsg("Bad (reserved) 'profile': 3 in first frame of ADTS file");
            break;
        }
        u_int8_t sampling_frequency_index = (fixedHeader[2] & 0x3C) >> 2;
        if (samplingFrequencyTable[sampling_frequency_index] == 0) {
            env.setResultMsg("Bad 'sampling_frequency_index' in first frame of ADTS file");
            break;
        }
        u_int8_t channel_configuration =
            ((fixedHeader[2] & 0x01) << 2) | ((fixedHeader[3] & 0xC0) >> 6);
        #ifndef _WIN32_WCE
        rewind(fid);
        #else
        SeekFile64(fid, SEEK_SET, 0);
        #endif
        #ifdef DEBUG
        fprintf(stderr, "Read first frame: profile %d, "
                "sampling_frequency_index %d => samplingFrequency %d, "
                "channel_configuration %d\n",
                profile,
                sampling_frequency_index, samplingFrequencyTable[sampling_frequency_index],
                channel_configuration);
        #endif
        return new ADTSAudioFileSource(env, fid, profile,
                                       sampling_frequency_index, channel_configuration);
    } while (0);
    CloseInputFile(fid);
    return NULL;
} // ADTSAudioFileSource::createNew

ADTSAudioFileSource
::ADTSAudioFileSource(UsageEnvironment& env, FILE* fid, u_int8_t profile,
                      u_int8_t samplingFrequencyIndex, u_int8_t channelConfiguration)
    : FramedFileSource(env, fid)
{
    fSamplingFrequency = samplingFrequencyTable[samplingFrequencyIndex];
    fNumChannels       = channelConfiguration == 0 ? 2 : channelConfiguration;
    fuSecsPerFrame     =
        (1024 * 1000000) / fSamplingFrequency;
    unsigned char audioSpecificConfig[2];
    u_int8_t const audioObjectType = profile + 1;
    audioSpecificConfig[0] = (audioObjectType << 3) | (samplingFrequencyIndex >> 1);
    audioSpecificConfig[1] = (samplingFrequencyIndex << 7) | (channelConfiguration << 3);
    sprintf(fConfigStr, "%02X%02x", audioSpecificConfig[0], audioSpecificConfig[1]);
}

ADTSAudioFileSource::~ADTSAudioFileSource()
{
    CloseInputFile(fFid);
}

void ADTSAudioFileSource::doGetNextFrame()
{
    unsigned char headers[7];

    if (fread(headers, 1, sizeof headers, fFid) < sizeof headers ||
        feof(fFid) || ferror(fFid))
    {
        handleClosure();
        return;
    }
    Boolean protection_absent = headers[1] & 0x01;
    u_int16_t frame_length    =
        ((headers[3] & 0x03) << 11) | (headers[4] << 3) | ((headers[5] & 0xE0) >> 5);
    #ifdef DEBUG
    u_int16_t syncword = (headers[0] << 4) | (headers[1] >> 4);
    fprintf(stderr, "Read frame: syncword 0x%x, protection_absent %d, frame_length %d\n", syncword, protection_absent,
            frame_length);
    if (syncword != 0xFFF) {
        fprintf(stderr, "WARNING: Bad syncword!\n");
    }
    #endif
    unsigned numBytesToRead =
        frame_length > sizeof headers ? frame_length - sizeof headers : 0;
    if (!protection_absent) {
        SeekFile64(fFid, 2, SEEK_CUR);
        numBytesToRead = numBytesToRead > 2 ? numBytesToRead - 2 : 0;
    }
    if (numBytesToRead > fMaxSize) {
        fNumTruncatedBytes = numBytesToRead - fMaxSize;
        numBytesToRead     = fMaxSize;
    }
    int numBytesRead = fread(fTo, 1, numBytesToRead, fFid);
    if (numBytesRead < 0) {
        numBytesRead = 0;
    }
    fFrameSize          = numBytesRead;
    fNumTruncatedBytes += numBytesToRead - numBytesRead;
    if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
        gettimeofday(&fPresentationTime, NULL);
    } else {
        unsigned uSeconds = fPresentationTime.tv_usec + fuSecsPerFrame;
        fPresentationTime.tv_sec += uSeconds / 1000000;
        fPresentationTime.tv_usec = uSeconds % 1000000;
    }
    fDurationInMicroseconds = fuSecsPerFrame;
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0,
                                                             (TaskFunc *) FramedSource::afterGetting, this);
} // ADTSAudioFileSource::doGetNextFrame
