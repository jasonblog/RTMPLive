#include "H264VideoRTPSink.hh"
#include "H264VideoStreamFramer.hh"
#include "Base64.hh"
#include "H264VideoRTPSource.hh"
H264VideoRTPSink
::H264VideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat,
                   u_int8_t const* sps, unsigned spsSize, u_int8_t const* pps, unsigned ppsSize)
    : H264or5VideoRTPSink(264, env, RTPgs, rtpPayloadFormat,
                          NULL, 0, sps, spsSize, pps, ppsSize)
{}

H264VideoRTPSink::~H264VideoRTPSink()
{}

H264VideoRTPSink * H264VideoRTPSink
::createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat)
{
    return new H264VideoRTPSink(env, RTPgs, rtpPayloadFormat);
}

H264VideoRTPSink * H264VideoRTPSink
::createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat,
            u_int8_t const* sps, unsigned spsSize, u_int8_t const* pps, unsigned ppsSize)
{
    return new H264VideoRTPSink(env, RTPgs, rtpPayloadFormat, sps, spsSize, pps, ppsSize);
}

H264VideoRTPSink * H264VideoRTPSink
::createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat,
            char const* sPropParameterSetsStr)
{
    u_int8_t* sps    = NULL;
    unsigned spsSize = 0;
    u_int8_t* pps    = NULL;
    unsigned ppsSize = 0;
    unsigned numSPropRecords;
    SPropRecord* sPropRecords = parseSPropParameterSets(sPropParameterSetsStr, numSPropRecords);

    for (unsigned i = 0; i < numSPropRecords; ++i) {
        if (sPropRecords[i].sPropLength == 0) {
            continue;
        }
        u_int8_t nal_unit_type = (sPropRecords[i].sPropBytes[0]) & 0x1F;
        if (nal_unit_type == 7) {
            sps     = sPropRecords[i].sPropBytes;
            spsSize = sPropRecords[i].sPropLength;
        } else if (nal_unit_type == 8) {
            pps     = sPropRecords[i].sPropBytes;
            ppsSize = sPropRecords[i].sPropLength;
        }
    }
    H264VideoRTPSink* result =
        new H264VideoRTPSink(env, RTPgs, rtpPayloadFormat, sps, spsSize, pps, ppsSize);
    delete[] sPropRecords;
    return result;
}

Boolean H264VideoRTPSink::sourceIsCompatibleWithUs(MediaSource& source)
{
    return source.isH264VideoStreamFramer();
}

char const * H264VideoRTPSink::auxSDPLine()
{
    H264or5VideoStreamFramer* framerSource = NULL;
    u_int8_t* vpsDummy    = NULL;
    unsigned vpsDummySize = 0;
    u_int8_t* sps         = fSPS;
    unsigned spsSize      = fSPSSize;
    u_int8_t* pps         = fPPS;
    unsigned ppsSize      = fPPSSize;

    if (sps == NULL || pps == NULL) {
        if (fOurFragmenter == NULL) {
            return NULL;
        }
        framerSource = (H264or5VideoStreamFramer *) (fOurFragmenter->inputSource());
        if (framerSource == NULL) {
            return NULL;
        }
        framerSource->getVPSandSPSandPPS(vpsDummy, vpsDummySize, sps, spsSize, pps, ppsSize);
        if (sps == NULL || pps == NULL) {
            return NULL;
        }
    }
    u_int8_t* spsWEB    = new u_int8_t[spsSize];
    unsigned spsWEBSize = removeH264or5EmulationBytes(spsWEB, spsSize, sps, spsSize);
    if (spsWEBSize < 4) {
        delete[] spsWEB;
        return NULL;
    }
    u_int32_t profileLevelId = (spsWEB[1] << 16) | (spsWEB[2] << 8) | spsWEB[3];
    delete[] spsWEB;
    char* sps_base64    = base64Encode((char *) sps, spsSize);
    char* pps_base64    = base64Encode((char *) pps, ppsSize);
    char const* fmtpFmt =
        "a=fmtp:%d packetization-mode=1"
        ";profile-level-id=%06X"
        ";sprop-parameter-sets=%s,%s\r\n";
    unsigned fmtpFmtSize = strlen(fmtpFmt)
                           + 3
                           + 6
                           + strlen(sps_base64) + strlen(pps_base64);
    char* fmtp = new char[fmtpFmtSize];
    sprintf(fmtp, fmtpFmt,
            rtpPayloadType(),
            profileLevelId,
            sps_base64, pps_base64);
    delete[] sps_base64;
    delete[] pps_base64;
    delete[] fFmtpSDPLine;
    fFmtpSDPLine = fmtp;
    return fFmtpSDPLine;
} // H264VideoRTPSink::auxSDPLine
