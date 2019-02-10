#include "MediaSource.hh"
MediaSource::MediaSource(UsageEnvironment& env)
    : Medium(env)
{}

MediaSource::~MediaSource()
{}

Boolean MediaSource::isSource() const
{
    return True;
}

char const * MediaSource::MIMEtype() const
{
    return "application/OCTET-STREAM";
}

Boolean MediaSource::isFramedSource() const
{
    return False;
}

Boolean MediaSource::isRTPSource() const
{
    return False;
}

Boolean MediaSource::isMPEG1or2VideoStreamFramer() const
{
    return False;
}

Boolean MediaSource::isMPEG4VideoStreamFramer() const
{
    return False;
}

Boolean MediaSource::isH264VideoStreamFramer() const
{
    return False;
}

Boolean MediaSource::isH265VideoStreamFramer() const
{
    return False;
}

Boolean MediaSource::isDVVideoStreamFramer() const
{
    return False;
}

Boolean MediaSource::isJPEGVideoSource() const
{
    return False;
}

Boolean MediaSource::isAMRAudioSource() const
{
    return False;
}

Boolean MediaSource::lookupByName(UsageEnvironment& env,
                                  char const*     sourceName,
                                  MediaSource *   & resultSource)
{
    resultSource = NULL;
    Medium* medium;
    if (!Medium::lookupByName(env, sourceName, medium)) {
        return False;
    }
    if (!medium->isSource()) {
        env.setResultMsg(sourceName, " is not a media source");
        return False;
    }
    resultSource = (MediaSource *) medium;
    return True;
}

void MediaSource::getAttributes() const
{
    envir().setResultMsg("");
}
