#include "H263plusVideoRTPSource.hh"
H263plusVideoRTPSource * H263plusVideoRTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                           unsigned char rtpPayloadFormat,
                                                           unsigned rtpTimestampFrequency)
{
    return new H263plusVideoRTPSource(env, RTPgs, rtpPayloadFormat,
                                      rtpTimestampFrequency);
}

H263plusVideoRTPSource
::H263plusVideoRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                         unsigned char rtpPayloadFormat,
                         unsigned rtpTimestampFrequency)
    : MultiFramedRTPSource(env, RTPgs,
                           rtpPayloadFormat, rtpTimestampFrequency),
    fNumSpecialHeaders(0), fSpecialHeaderBytesLength(0)
{}

H263plusVideoRTPSource::~H263plusVideoRTPSource()
{}

Boolean H263plusVideoRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    unsigned char* headerStart  = packet->data();
    unsigned packetSize         = packet->dataSize();
    unsigned expectedHeaderSize = 2;

    if (packetSize < expectedHeaderSize) {
        return False;
    }
    Boolean P = (headerStart[0] & 0x4) != 0;
    Boolean V = (headerStart[0] & 0x2) != 0;
    unsigned char PLEN = ((headerStart[0] & 0x1) << 5) | (headerStart[1] >> 3);
    if (V) {
        ++expectedHeaderSize;
        if (packetSize < expectedHeaderSize) {
            return False;
        }
    }
    if (PLEN > 0) {
        expectedHeaderSize += PLEN;
        if (packetSize < expectedHeaderSize) {
            return False;
        }
    }
    fCurrentPacketBeginsFrame = P;
    if (fCurrentPacketBeginsFrame) {
        fNumSpecialHeaders = fSpecialHeaderBytesLength = 0;
    }
    unsigned bytesAvailable =
        SPECIAL_HEADER_BUFFER_SIZE - fSpecialHeaderBytesLength - 1;
    if (expectedHeaderSize <= bytesAvailable) {
        fSpecialHeaderBytes[fSpecialHeaderBytesLength++] = expectedHeaderSize;
        for (unsigned i = 0; i < expectedHeaderSize; ++i) {
            fSpecialHeaderBytes[fSpecialHeaderBytesLength++] = headerStart[i];
        }
        fPacketSizes[fNumSpecialHeaders++] = packetSize;
    }
    if (P) {
        expectedHeaderSize -= 2;
        headerStart[expectedHeaderSize]     = 0;
        headerStart[expectedHeaderSize + 1] = 0;
    }
    fCurrentPacketCompletesFrame = packet->rtpMarkerBit();
    resultSpecialHeaderSize      = expectedHeaderSize;
    return True;
} // H263plusVideoRTPSource::processSpecialHeader

char const * H263plusVideoRTPSource::MIMEtype() const
{
    return "video/H263-1998";
}
