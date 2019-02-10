#include "FramedFilter.hh"
#include <string.h>
void FramedFilter::detachInputSource()
{
    if (fInputSource != NULL) {
        fInputSource->stopGettingFrames();
        reassignInputSource(NULL);
    }
}

FramedFilter::FramedFilter(UsageEnvironment& env,
                           FramedSource*   inputSource)
    : FramedSource(env),
    fInputSource(inputSource)
{}

FramedFilter::~FramedFilter()
{
    Medium::close(fInputSource);
}

char const * FramedFilter::MIMEtype() const
{
    if (fInputSource == NULL) {
        return "";
    }
    return fInputSource->MIMEtype();
}

void FramedFilter::getAttributes() const
{
    if (fInputSource != NULL) {
        fInputSource->getAttributes();
    }
}

void FramedFilter::doStopGettingFrames()
{
    FramedSource::doStopGettingFrames();
    if (fInputSource != NULL) {
        fInputSource->stopGettingFrames();
    }
}
