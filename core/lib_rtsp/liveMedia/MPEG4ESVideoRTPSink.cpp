#include "MPEG4ESVideoRTPSink.hh"
#include "MPEG4VideoStreamFramer.hh"
#include "MPEG4LATMAudioRTPSource.hh"
MPEG4ESVideoRTPSink
::MPEG4ESVideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs, unsigned char rtpPayloadFormat,
                      u_int32_t rtpTimestampFrequency,
                      u_int8_t profileAndLevelIndication, char const* configStr)
    : VideoRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, "MP4V-ES"),
    fVOPIsPresent(False), fProfileAndLevelIndication(profileAndLevelIndication), fFmtpSDPLine(NULL)
{
    fConfigBytes = parseGeneralConfigStr(configStr, fNumConfigBytes);
}

MPEG4ESVideoRTPSink::~MPEG4ESVideoRTPSink()
{
    delete[] fFmtpSDPLine;
    delete[] fConfigBytes;
}

MPEG4ESVideoRTPSink * MPEG4ESVideoRTPSink::createNew(UsageEnvironment& env,
                                                     Groupsock* RTPgs, unsigned char rtpPayloadFormat,
                                                     u_int32_t rtpTimestampFrequency)
{
    return new MPEG4ESVideoRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency);
}

MPEG4ESVideoRTPSink * MPEG4ESVideoRTPSink::createNew(UsageEnvironment& env,
                                                     Groupsock* RTPgs, unsigned char rtpPayloadFormat,
                                                     u_int32_t rtpTimestampFrequency,
                                                     u_int8_t profileAndLevelIndication, char const* configStr)
{
    return new MPEG4ESVideoRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, profileAndLevelIndication,
                                   configStr);
}

Boolean MPEG4ESVideoRTPSink::sourceIsCompatibleWithUs(MediaSource& source)
{
    return source.isMPEG4VideoStreamFramer();
}

#define VOP_START_CODE 0x000001B6
void MPEG4ESVideoRTPSink
::doSpecialFrameHandling(unsigned       fragmentationOffset,
                         unsigned char* frameStart,
                         unsigned       numBytesInFrame,
                         struct timeval framePresentationTime,
                         unsigned       numRemainingBytes)
{
    if (fragmentationOffset == 0) {
        if (numBytesInFrame < 4) {
            return;
        }
        unsigned startCode = (frameStart[0] << 24) | (frameStart[1] << 16)
                             | (frameStart[2] << 8) | frameStart[3];
        fVOPIsPresent = startCode == VOP_START_CODE;
    }
    MPEG4VideoStreamFramer* framerSource = (MPEG4VideoStreamFramer *) fSource;
    if (framerSource != NULL && framerSource->pictureEndMarker() &&
        numRemainingBytes == 0)
    {
        setMarkerBit();
        framerSource->pictureEndMarker() = False;
    }
    setTimestamp(framePresentationTime);
}

Boolean MPEG4ESVideoRTPSink::allowFragmentationAfterStart() const
{
    return True;
}

Boolean MPEG4ESVideoRTPSink
::frameCanAppearAfterPacketStart(unsigned char const *,
                                 unsigned) const
{
    return !fVOPIsPresent;
}

char const * MPEG4ESVideoRTPSink::auxSDPLine()
{
    unsigned configLength = fNumConfigBytes;
    unsigned char* config = fConfigBytes;

    if (fProfileAndLevelIndication == 0 || config == NULL) {
        MPEG4VideoStreamFramer* framerSource = (MPEG4VideoStreamFramer *) fSource;
        if (framerSource == NULL) {
            return NULL;
        }
        fProfileAndLevelIndication = framerSource->profile_and_level_indication();
        if (fProfileAndLevelIndication == 0) {
            return NULL;
        }
        config = framerSource->getConfigBytes(configLength);
        if (config == NULL) {
            return NULL;
        }
    }
    char const* fmtpFmt =
        "a=fmtp:%d "
        "profile-level-id=%d;"
        "config=";
    unsigned fmtpFmtSize = strlen(fmtpFmt)
                           + 3
                           + 3
                           + 2 * configLength
                           + 2;
    char* fmtp = new char[fmtpFmtSize];
    sprintf(fmtp, fmtpFmt, rtpPayloadType(), fProfileAndLevelIndication);
    char* endPtr = &fmtp[strlen(fmtp)];
    for (unsigned i = 0; i < configLength; ++i) {
        sprintf(endPtr, "%02X", config[i]);
        endPtr += 2;
    }
    sprintf(endPtr, "\r\n");
    delete[] fFmtpSDPLine;
    fFmtpSDPLine = strDup(fmtp);
    delete[] fmtp;
    return fFmtpSDPLine;
} // MPEG4ESVideoRTPSink::auxSDPLine
