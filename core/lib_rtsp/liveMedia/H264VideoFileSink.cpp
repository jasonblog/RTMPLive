#include "H264VideoFileSink.hh"
#include "OutputFile.hh"
H264VideoFileSink
::H264VideoFileSink(UsageEnvironment& env, FILE* fid,
                    char const* sPropParameterSetsStr,
                    unsigned bufferSize, char const* perFrameFileNamePrefix)
    : H264or5VideoFileSink(env, fid, bufferSize, perFrameFileNamePrefix,
                           sPropParameterSetsStr, NULL, NULL)
{}

H264VideoFileSink::~H264VideoFileSink()
{}

H264VideoFileSink * H264VideoFileSink::createNew(UsageEnvironment& env, char const* fileName,
                                                 char const* sPropParameterSetsStr,
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
        return new H264VideoFileSink(env, fid, sPropParameterSetsStr, bufferSize, perFrameFileNamePrefix);
    } while (0);
    return NULL;
}
