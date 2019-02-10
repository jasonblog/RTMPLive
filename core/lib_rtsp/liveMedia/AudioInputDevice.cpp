#include <AudioInputDevice.hh>
AudioInputDevice
::AudioInputDevice(UsageEnvironment& env, unsigned char bitsPerSample,
                   unsigned char numChannels,
                   unsigned samplingFrequency, unsigned granularityInMS)
    : FramedSource(env), fBitsPerSample(bitsPerSample),
    fNumChannels(numChannels), fSamplingFrequency(samplingFrequency),
    fGranularityInMS(granularityInMS)
{}

AudioInputDevice::~AudioInputDevice()
{}

char ** AudioInputDevice::allowedDeviceNames = NULL;
AudioPortNames::AudioPortNames()
    : numPorts(0), portName(NULL)
{}

AudioPortNames::~AudioPortNames()
{
    for (unsigned i = 0; i < numPorts; ++i) {
        delete portName[i];
    }
    delete portName;
}
