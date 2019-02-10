#include "JPEGVideoRTPSink.hh"
#include "JPEGVideoSource.hh"
JPEGVideoRTPSink
::JPEGVideoRTPSink(UsageEnvironment& env, Groupsock* RTPgs)
    : VideoRTPSink(env, RTPgs, 26, 90000, "JPEG")
{}

JPEGVideoRTPSink::~JPEGVideoRTPSink()
{}

JPEGVideoRTPSink * JPEGVideoRTPSink::createNew(UsageEnvironment& env, Groupsock* RTPgs)
{
    return new JPEGVideoRTPSink(env, RTPgs);
}

Boolean JPEGVideoRTPSink::sourceIsCompatibleWithUs(MediaSource& source)
{
    return source.isJPEGVideoSource();
}

Boolean JPEGVideoRTPSink
::frameCanAppearAfterPacketStart(unsigned char const *,
                                 unsigned) const
{
    return False;
}

void JPEGVideoRTPSink
::doSpecialFrameHandling(unsigned fragmentationOffset,
                         unsigned char *,
                         unsigned,
                         struct timeval framePresentationTime,
                         unsigned       numRemainingBytes)
{
    JPEGVideoSource* source = (JPEGVideoSource *) fSource;

    if (source == NULL) {
        return;
    }
    u_int8_t mainJPEGHeader[8];
    u_int8_t const type = source->type();
    mainJPEGHeader[0] = 0;
    mainJPEGHeader[1] = fragmentationOffset >> 16;
    mainJPEGHeader[2] = fragmentationOffset >> 8;
    mainJPEGHeader[3] = fragmentationOffset;
    mainJPEGHeader[4] = type;
    mainJPEGHeader[5] = source->qFactor();
    mainJPEGHeader[6] = source->width();
    mainJPEGHeader[7] = source->height();
    setSpecialHeaderBytes(mainJPEGHeader, sizeof mainJPEGHeader);
    unsigned restartMarkerHeaderSize = 0;
    if (type >= 64 && type <= 127) {
        restartMarkerHeaderSize = 4;
        u_int16_t const restartInterval = source->restartInterval();
        u_int8_t restartMarkerHeader[4];
        restartMarkerHeader[0] = restartInterval >> 8;
        restartMarkerHeader[1] = restartInterval & 0xFF;
        restartMarkerHeader[2] = restartMarkerHeader[3] = 0xFF;
        setSpecialHeaderBytes(restartMarkerHeader, restartMarkerHeaderSize,
                              sizeof mainJPEGHeader);
    }
    if (fragmentationOffset == 0 && source->qFactor() >= 128) {
        u_int8_t precision;
        u_int16_t length;
        u_int8_t const* quantizationTables =
            source->quantizationTables(precision, length);
        unsigned const quantizationHeaderSize = 4 + length;
        u_int8_t* quantizationHeader = new u_int8_t[quantizationHeaderSize];
        quantizationHeader[0] = 0;
        quantizationHeader[1] = precision;
        quantizationHeader[2] = length >> 8;
        quantizationHeader[3] = length & 0xFF;
        if (quantizationTables != NULL) {
            for (u_int16_t i = 0; i < length; ++i) {
                quantizationHeader[4 + i] = quantizationTables[i];
            }
        }
        setSpecialHeaderBytes(quantizationHeader, quantizationHeaderSize,
                              sizeof mainJPEGHeader + restartMarkerHeaderSize);
        delete[] quantizationHeader;
    }
    if (numRemainingBytes == 0) {
        setMarkerBit();
    }
    setTimestamp(framePresentationTime);
} // JPEGVideoRTPSink::doSpecialFrameHandling

unsigned JPEGVideoRTPSink::specialHeaderSize() const
{
    JPEGVideoSource* source = (JPEGVideoSource *) fSource;

    if (source == NULL) {
        return 0;
    }
    unsigned headerSize = 8;
    u_int8_t const type = source->type();
    if (type >= 64 && type <= 127) {
        headerSize += 4;
    }
    if (curFragmentationOffset() == 0 && source->qFactor() >= 128) {
        u_int8_t dummy;
        u_int16_t quantizationTablesSize;
        (void) (source->quantizationTables(dummy, quantizationTablesSize));
        headerSize += 4 + quantizationTablesSize;
    }
    return headerSize;
}
