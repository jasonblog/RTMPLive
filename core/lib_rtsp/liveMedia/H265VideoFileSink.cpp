#include "H265VideoFileSink.hh"
#include "OutputFile.hh"
H265VideoFileSink
::H265VideoFileSink(UsageEnvironment& env, FILE* fid,
                    char const* sPropVPSStr,
                    char const* sPropSPSStr,
                    char const* sPropPPSStr,
                    unsigned bufferSize, char const* perFrameFileNamePrefix)
    : H264or5VideoFileSink(env, fid, bufferSize, perFrameFileNamePrefix,
                           sPropVPSStr, sPropSPSStr, sPropPPSStr)
{}

H265VideoFileSink::~H265VideoFileSink()
{}

H265VideoFileSink * H265VideoFileSink::createNew(UsageEnvironment& env, char const* fileName,
                                                 char const* sPropVPSStr,
                                                 char const* sPropSPSStr,
                                                 char const* sPropPPSStr,
                                                 unsigned bufferSize, Boolean oneFilePerFrame)
{
    do {
        FILE* fid;
        char const* perFrameFileNamePrefix;
        if (oneFilePerFrame) {
            fid = NULL;
            perFrameFileNamePrefix = fileName;
        } else {
            fid = OpenOutputFile(env, fileName);
            if (fid == NULL) {
                break;
            }
            perFrameFileNamePrefix = NULL;
        }
        return new H265VideoFileSink(env, fid, sPropVPSStr, sPropSPSStr, sPropPPSStr, bufferSize,
                                     perFrameFileNamePrefix);
    } while (0);
    return NULL;
}
