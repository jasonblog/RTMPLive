#include "H264or5VideoFileSink.hh"
#include "H264VideoRTPSource.hh"
H264or5VideoFileSink
::H264or5VideoFileSink(UsageEnvironment& env, FILE* fid,
                       unsigned bufferSize, char const* perFrameFileNamePrefix,
                       char const* sPropParameterSetsStr1,
                       char const* sPropParameterSetsStr2,
                       char const* sPropParameterSetsStr3)
    : FileSink(env, fid, bufferSize, perFrameFileNamePrefix),
    fHaveWrittenFirstFrame(False)
{
    fSPropParameterSetsStr[0] = sPropParameterSetsStr1;
    fSPropParameterSetsStr[1] = sPropParameterSetsStr2;
    fSPropParameterSetsStr[2] = sPropParameterSetsStr3;
}

H264or5VideoFileSink::~H264or5VideoFileSink()
{}

void H264or5VideoFileSink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                                             struct timeval presentationTime)
{
    unsigned char const start_code[4] = { 0x00, 0x00, 0x00, 0x01 };

    if (!fHaveWrittenFirstFrame) {
        for (unsigned j = 0; j < 3; ++j) {
            unsigned numSPropRecords;
            SPropRecord* sPropRecords =
                parseSPropParameterSets(fSPropParameterSetsStr[j], numSPropRecords);
            for (unsigned i = 0; i < numSPropRecords; ++i) {
                addData(start_code, 4, presentationTime);
                addData(sPropRecords[i].sPropBytes, sPropRecords[i].sPropLength, presentationTime);
            }
            delete[] sPropRecords;
        }
        fHaveWrittenFirstFrame = True;
    }
    addData(start_code, 4, presentationTime);
    FileSink::afterGettingFrame(frameSize, numTruncatedBytes, presentationTime);
}
