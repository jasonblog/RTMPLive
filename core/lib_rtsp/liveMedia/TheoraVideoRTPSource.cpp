#include "TheoraVideoRTPSource.hh"
class TheoraBufferedPacket : public BufferedPacket
{
public:
    TheoraBufferedPacket();
    virtual ~TheoraBufferedPacket();
private:
    virtual unsigned nextEnclosedFrameSize(unsigned char *& framePtr,
                                           unsigned       dataSize);
};
class TheoraBufferedPacketFactory : public BufferedPacketFactory
{
private:
    virtual BufferedPacket * createNewPacket(MultiFramedRTPSource* ourSource);
};
TheoraVideoRTPSource * TheoraVideoRTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                                       unsigned char rtpPayloadFormat)
{
    return new TheoraVideoRTPSource(env, RTPgs, rtpPayloadFormat);
}

TheoraVideoRTPSource
::TheoraVideoRTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                       unsigned char rtpPayloadFormat)
    : MultiFramedRTPSource(env, RTPgs, rtpPayloadFormat, 90000,
                           new TheoraBufferedPacketFactory),
    fCurPacketIdent(0)
{}

TheoraVideoRTPSource::~TheoraVideoRTPSource()
{}

Boolean TheoraVideoRTPSource
::processSpecialHeader(BufferedPacket* packet,
                       unsigned        & resultSpecialHeaderSize)
{
    unsigned char* headerStart = packet->data();
    unsigned packetSize        = packet->dataSize();

    resultSpecialHeaderSize = 4;
    if (packetSize < resultSpecialHeaderSize) {
        return False;
    }
    fCurPacketIdent = (headerStart[0] << 16) | (headerStart[1] << 8) | headerStart[2];
    if ((headerStart[3] & 0x30) == 0x30) {
        return False;
    }
    u_int8_t F = headerStart[3] >> 6;
    fCurrentPacketBeginsFrame    = F <= 1;
    fCurrentPacketCompletesFrame = F == 0 || F == 3;
    return True;
}

char const * TheoraVideoRTPSource::MIMEtype() const
{
    return "video/THEORA";
}

TheoraBufferedPacket::TheoraBufferedPacket()
{}

TheoraBufferedPacket::~TheoraBufferedPacket()
{}

unsigned TheoraBufferedPacket
::nextEnclosedFrameSize(unsigned char *& framePtr, unsigned dataSize)
{
    if (dataSize < 2) {
        return dataSize;
    }
    unsigned frameSize = (framePtr[0] << 8) | framePtr[1];
    framePtr += 2;
    if (frameSize > dataSize - 2) {
        return dataSize - 2;
    }
    return frameSize;
}

BufferedPacket * TheoraBufferedPacketFactory
::createNewPacket(MultiFramedRTPSource *)
{
    return new TheoraBufferedPacket();
}
