#include "H264VideoStreamDiscreteFramer.hh"
H264VideoStreamDiscreteFramer * H264VideoStreamDiscreteFramer::createNew(UsageEnvironment& env,
                                                                         FramedSource*   inputSource)
{
    return new H264VideoStreamDiscreteFramer(env, inputSource);
}

H264VideoStreamDiscreteFramer
::H264VideoStreamDiscreteFramer(UsageEnvironment& env, FramedSource* inputSource)
    : H264or5VideoStreamDiscreteFramer(264, env, inputSource)
{}

H264VideoStreamDiscreteFramer::~H264VideoStreamDiscreteFramer()
{}

Boolean H264VideoStreamDiscreteFramer::isH264VideoStreamFramer() const
{
    return True;
}
