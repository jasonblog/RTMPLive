#include "AMRAudioSource.hh"
AMRAudioSource::AMRAudioSource(UsageEnvironment& env,
                               Boolean isWideband, unsigned numChannels)
    : FramedSource(env),
    fIsWideband(isWideband), fNumChannels(numChannels), fLastFrameHeader(0)
{}

AMRAudioSource::~AMRAudioSource()
{}

char const * AMRAudioSource::MIMEtype() const
{
    return "audio/AMR";
}

Boolean AMRAudioSource::isAMRAudioSource() const
{
    return True;
}
