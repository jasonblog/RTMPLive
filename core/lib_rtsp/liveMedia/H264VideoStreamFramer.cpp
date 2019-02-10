#include "H264VideoStreamFramer.hh"
H264VideoStreamFramer * H264VideoStreamFramer
::createNew(UsageEnvironment& env, FramedSource* inputSource, Boolean includeStartCodeInOutput)
{
    return new H264VideoStreamFramer(env, inputSource, True, includeStartCodeInOutput);
}

H264VideoStreamFramer
::H264VideoStreamFramer(UsageEnvironment& env, FramedSource* inputSource, Boolean createParser,
                        Boolean includeStartCodeInOutput)
    : H264or5VideoStreamFramer(264, env, inputSource, createParser, includeStartCodeInOutput)
{}

H264VideoStreamFramer::~H264VideoStreamFramer()
{}

Boolean H264VideoStreamFramer::isH264VideoStreamFramer() const
{
    return True;
}
