#include "H265VideoRTPSink.hh"
#include "H265VideoStreamFramer.hh"
#include "Base64.hh"
#include "BitVector.hh"
#include "H264VideoRTPSource.hh"
H265VideoRTPSink
::H265VideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat,
                   u_int8_t const* vps, unsigned vpsSize,
                   u_int8_t const* sps, unsigned spsSize,
                   u_int8_t const* pps, unsigned ppsSize)
    : H264or5VideoRTPSink(265, env, RTPgs, rtpPayloadFormat,
                          vps, vpsSize, sps, spsSize, pps, ppsSize)
{}

H265VideoRTPSink::~H265VideoRTPSink()
{}

H265VideoRTPSink * H265VideoRTPSink
::createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat)
{
    return new H265VideoRTPSink(env, RTPgs, rtpPayloadFormat);
}

H265VideoRTPSink * H265VideoRTPSink
::createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat,
            u_int8_t const* vps, unsigned vpsSize,
            u_int8_t const* sps, unsigned spsSize,
            u_int8_t const* pps, unsigned ppsSize)
{
    return new H265VideoRTPSink(env, RTPgs, rtpPayloadFormat,
                                vps, vpsSize, sps, spsSize, pps, ppsSize);
}

H265VideoRTPSink * H265VideoRTPSink
::createNew(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat,
            char const* sPropVPSStr, char const* sPropSPSStr, char const* sPropPPSStr)
{
    u_int8_t* vps    = NULL;
    unsigned vpsSize = 0;
    u_int8_t* sps    = NULL;
    unsigned spsSize = 0;
    u_int8_t* pps    = NULL;
    unsigned ppsSize = 0;
    SPropRecord* sPropRecords[3];
    unsigned numSPropRecords[3];

    sPropRecords[0] = parseSPropParameterSets(sPropVPSStr, numSPropRecords[0]);
    sPropRecords[1] = parseSPropParameterSets(sPropSPSStr, numSPropRecords[1]);
    sPropRecords[2] = parseSPropParameterSets(sPropPPSStr, numSPropRecords[2]);
    for (unsigned j = 0; j < 3; ++j) {
        SPropRecord* records = sPropRecords[j];
        unsigned numRecords  = numSPropRecords[j];
        for (unsigned i = 0; i < numRecords; ++i) {
            if (records[i].sPropLength == 0) {
                continue;
            }
            u_int8_t nal_unit_type = ((records[i].sPropBytes[0]) & 0x7E) >> 1;
            if (nal_unit_type == 32) {
                vps     = records[i].sPropBytes;
                vpsSize = records[i].sPropLength;
            } else if (nal_unit_type == 33) {
                sps     = records[i].sPropBytes;
                spsSize = records[i].sPropLength;
            } else if (nal_unit_type == 34) {
                pps     = records[i].sPropBytes;
                ppsSize = records[i].sPropLength;
            }
        }
    }
    H265VideoRTPSink* result = new H265VideoRTPSink(env, RTPgs, rtpPayloadFormat,
                                                    vps, vpsSize, sps, spsSize, pps, ppsSize);
    delete[] sPropRecords[0];
    delete[] sPropRecords[1];
    delete[] sPropRecords[2];
    return result;
} // H265VideoRTPSink::createNew

Boolean H265VideoRTPSink::sourceIsCompatibleWithUs(MediaSource& source)
{
    return source.isH265VideoStreamFramer();
}

char const * H265VideoRTPSink::auxSDPLine()
{
    H264or5VideoStreamFramer* framerSource = NULL;
    u_int8_t* vps    = fVPS;
    unsigned vpsSize = fVPSSize;
    u_int8_t* sps    = fSPS;
    unsigned spsSize = fSPSSize;
    u_int8_t* pps    = fPPS;
    unsigned ppsSize = fPPSSize;

    if (vps == NULL || sps == NULL || pps == NULL) {
        if (fOurFragmenter == NULL) {
            return NULL;
        }
        framerSource = (H264or5VideoStreamFramer *) (fOurFragmenter->inputSource());
        if (framerSource == NULL) {
            return NULL;
        }
        framerSource->getVPSandSPSandPPS(vps, vpsSize, sps, spsSize, pps, ppsSize);
        if (vps == NULL || sps == NULL || pps == NULL) {
            return NULL;
        }
    }
    u_int8_t* vpsWEB    = new u_int8_t[vpsSize];
    unsigned vpsWEBSize = removeH264or5EmulationBytes(vpsWEB, vpsSize, vps, vpsSize);
    if (vpsWEBSize < 6 + 12) {
        delete[] vpsWEB;
        return NULL;
    }
    u_int8_t const* profileTierLevelHeaderBytes = &vpsWEB[6];
    unsigned profileSpace = profileTierLevelHeaderBytes[0] >> 6;
    unsigned profileId    = profileTierLevelHeaderBytes[0] & 0x1F;
    unsigned tierFlag     = (profileTierLevelHeaderBytes[0] >> 5) & 0x1;
    unsigned levelId      = profileTierLevelHeaderBytes[11];
    u_int8_t const* interop_constraints = &profileTierLevelHeaderBytes[5];
    char interopConstraintsStr[100];
    sprintf(interopConstraintsStr, "%02X%02X%02X%02X%02X%02X",
            interop_constraints[0], interop_constraints[1], interop_constraints[2],
            interop_constraints[3], interop_constraints[4], interop_constraints[5]);
    delete[] vpsWEB;
    char* sprop_vps     = base64Encode((char *) vps, vpsSize);
    char* sprop_sps     = base64Encode((char *) sps, spsSize);
    char* sprop_pps     = base64Encode((char *) pps, ppsSize);
    char const* fmtpFmt =
        "a=fmtp:%d profile-space=%u"
        ";profile-id=%u"
        ";tier-flag=%u"
        ";level-id=%u"
        ";interop-constraints=%s"
        ";sprop-vps=%s"
        ";sprop-sps=%s"
        ";sprop-pps=%s\r\n";
    unsigned fmtpFmtSize = strlen(fmtpFmt)
                           + 3  + 20
                           + 20
                           + 20
                           + 20
                           + strlen(interopConstraintsStr)
                           + strlen(sprop_vps)
                           + strlen(sprop_sps)
                           + strlen(sprop_pps);
    char* fmtp = new char[fmtpFmtSize];
    sprintf(fmtp, fmtpFmt,
            rtpPayloadType(), profileSpace,
            profileId,
            tierFlag,
            levelId,
            interopConstraintsStr,
            sprop_vps,
            sprop_sps,
            sprop_pps);
    delete[] sprop_vps;
    delete[] sprop_sps;
    delete[] sprop_pps;
    delete[] fFmtpSDPLine;
    fFmtpSDPLine = fmtp;
    return fFmtpSDPLine;
} // H265VideoRTPSink::auxSDPLine
