#include "MP3ADURTPSource.hh"
#include "MP3ADUdescriptor.hh"
class ADUBufferedPacket : public BufferedPacket
{
private:
    virtual unsigned nextEnclosedFrameSize(unsigned char *& framePtr,
                                           unsigned       dataSize);
};
class ADUBufferedPacketFactory : public BufferedPacketFactory
{
private:
    virtual BufferedPacket * createNewPacket(MultiFramedRTPSource* ourSource);
};
MP3ADURTPSource * MP3ADURTPSource::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                                             unsigned char rtpPayloadFormat,
                                             unsigned rtpTimestampFrequency)
{
    return new MP3ADURTPSource(env, RTPgs, rtpPayloadFormat,
                               rtpTimestampFrequency);
}

MP3ADURTPSource::MP3ADURTPSource(UsageEnvironment& env, Groupsock* RTPgs,
                                 unsigned char rtpPayloadFormat,
                                 unsigned rtpTimestampFrequency)
    : MultiFramedRTPSource(env, RTPgs,
                           rtpPayloadFormat, rtpTimestampFrequency,
                           new ADUBufferedPacketFactory)
{}

MP3ADURTPSource::~MP3ADURTPSource()
{}

char const * MP3ADURTPSource::MIMEtype() const
{
    return "audio/MPA-ROBUST";
}

unsigned ADUBufferedPacket
::nextEnclosedFrameSize(unsigned char *& framePtr, unsigned dataSize)
{
    unsigned char* frameDataPtr = framePtr;
    unsigned remainingFrameSize =
        ADUdescriptor::getRemainingFrameSize(frameDataPtr);
    unsigned descriptorSize = (unsigned) (frameDataPtr - framePtr);
    unsigned fullADUSize    = descriptorSize + remainingFrameSize;

    return (fullADUSize <= dataSize) ? fullADUSize : dataSize;
}

BufferedPacket * ADUBufferedPacketFactory
::createNewPacket(MultiFramedRTPSource *)
{
    return new ADUBufferedPacket;
}
