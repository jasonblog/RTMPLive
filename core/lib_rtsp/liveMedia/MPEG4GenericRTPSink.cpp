#include "MPEG4GenericRTPSink.hh"
#include "Locale.hh"
#include <ctype.h>
MPEG4GenericRTPSink
::MPEG4GenericRTPSink(UsageEnvironment& env, Groupsock* RTPgs,
                      u_int8_t rtpPayloadFormat,
                      u_int32_t rtpTimestampFrequency,
                      char const* sdpMediaTypeString,
                      char const* mpeg4Mode, char const* configString,
                      unsigned numChannels)
    : MultiFramedRTPSink(env, RTPgs, rtpPayloadFormat,
                         rtpTimestampFrequency, "MPEG4-GENERIC", numChannels),
    fSDPMediaTypeString(strDup(sdpMediaTypeString)),
    fMPEG4Mode(strDup(mpeg4Mode)), fConfigString(strDup(configString))
{
    if (mpeg4Mode == NULL) {
        env << "MPEG4GenericRTPSink error: NULL \"mpeg4Mode\" parameter\n";
    } else {
        size_t const len = strlen(mpeg4Mode) + 1;
        char* m = new char[len];
        Locale l("POSIX");
        for (size_t i = 0; i < len; ++i) {
            m[i] = tolower(mpeg4Mode[i]);
        }
        if (strcmp(m, "aac-hbr") != 0) {
            env << "MPEG4GenericRTPSink error: Unknown \"mpeg4Mode\" parameter: \"" << mpeg4Mode << "\"\n";
        }
        delete[] m;
    }
    char const* fmtpFmt =
        "a=fmtp:%d "
        "streamtype=%d;profile-level-id=1;"
        "mode=%s;sizelength=13;indexlength=3;indexdeltalength=3;"
        "config=%s\r\n";
    unsigned fmtpFmtSize = strlen(fmtpFmt)
                           + 3
                           + 3
                           + strlen(fMPEG4Mode)
                           + strlen(fConfigString);
    char* fmtp = new char[fmtpFmtSize];
    sprintf(fmtp, fmtpFmt,
            rtpPayloadType(),
            strcmp(fSDPMediaTypeString, "video") == 0 ? 4 : 5,
            fMPEG4Mode,
            fConfigString);
    fFmtpSDPLine = strDup(fmtp);
    delete[] fmtp;
}

MPEG4GenericRTPSink::~MPEG4GenericRTPSink()
{
    delete[] fFmtpSDPLine;
    delete[](char *) fConfigString;
    delete[](char *) fMPEG4Mode;
    delete[](char *) fSDPMediaTypeString;
}

MPEG4GenericRTPSink * MPEG4GenericRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                     u_int8_t rtpPayloadFormat,
                                                     u_int32_t rtpTimestampFrequency,
                                                     char const* sdpMediaTypeString,
                                                     char const* mpeg4Mode,
                                                     char const* configString, unsigned numChannels)
{
    return new MPEG4GenericRTPSink(env, RTPgs, rtpPayloadFormat,
                                   rtpTimestampFrequency,
                                   sdpMediaTypeString, mpeg4Mode,
                                   configString, numChannels);
}

Boolean MPEG4GenericRTPSink
::frameCanAppearAfterPacketStart(unsigned char const *,
                                 unsigned) const
{
    return False;
}

void MPEG4GenericRTPSink
::doSpecialFrameHandling(unsigned       fragmentationOffset,
                         unsigned char* frameStart,
                         unsigned       numBytesInFrame,
                         struct timeval framePresentationTime,
                         unsigned       numRemainingBytes)
{
    unsigned fullFrameSize =
        fragmentationOffset + numBytesInFrame + numRemainingBytes;
    unsigned char headers[4];

    headers[0] = 0;
    headers[1] = 16;
    headers[2] = fullFrameSize >> 5;
    headers[3] = (fullFrameSize & 0x1F) << 3;
    setSpecialHeaderBytes(headers, sizeof headers);
    if (numRemainingBytes == 0) {
        setMarkerBit();
    }
    MultiFramedRTPSink::doSpecialFrameHandling(fragmentationOffset,
                                               frameStart, numBytesInFrame,
                                               framePresentationTime,
                                               numRemainingBytes);
}

unsigned MPEG4GenericRTPSink::specialHeaderSize() const
{
    return 2 + 2;
}

char const * MPEG4GenericRTPSink::sdpMediaType() const
{
    return fSDPMediaTypeString;
}

char const * MPEG4GenericRTPSink::auxSDPLine()
{
    return fFmtpSDPLine;
}
