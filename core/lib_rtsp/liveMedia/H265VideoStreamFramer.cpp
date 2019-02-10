#include "H265VideoStreamFramer.hh"
H265VideoStreamFramer * H265VideoStreamFramer
::createNew(UsageEnvironment& env, FramedSource* inputSource, Boolean includeStartCodeInOutput)
{
    return new H265VideoStreamFramer(env, inputSource, True, includeStartCodeInOutput);
}

H265VideoStreamFramer
::H265VideoStreamFramer(UsageEnvironment& env, FramedSource* inputSource, Boolean createParser,
                        Boolean includeStartCodeInOutput)
    : H264or5VideoStreamFramer(265, env, inputSource, createParser, includeStartCodeInOutput)
{}

H265VideoStreamFramer::~H265VideoStreamFramer()
{}

Boolean H265VideoStreamFramer::isH265VideoStreamFramer() const
{
    return True;
}
