#include "OggFileSink.hh"
#include "OutputFile.hh"
#include "VorbisAudioRTPSource.hh"
#include "MPEG2TransportStreamMultiplexor.hh"
#include "FramedSource.hh"
OggFileSink * OggFileSink
::createNew(UsageEnvironment& env, char const* fileName,
            unsigned samplingFrequency, char const* configStr,
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
        return new OggFileSink(env, fid, samplingFrequency, configStr, bufferSize, perFrameFileNamePrefix);
    } while (0);
    return NULL;
}

OggFileSink::OggFileSink(UsageEnvironment& env, FILE* fid,
                         unsigned samplingFrequency, char const* configStr,
                         unsigned bufferSize, char const* perFrameFileNamePrefix)
    : FileSink(env, fid, bufferSize, perFrameFileNamePrefix),
    fSamplingFrequency(samplingFrequency), fConfigStr(configStr),
    fHaveWrittenFirstFrame(False), fHaveSeenEOF(False),
    fGranulePosition(0), fGranulePositionAdjustment(0), fPageSequenceNumber(0),
    fIsTheora(False), fGranuleIncrementPerFrame(1),
    fAltFrameSize(0), fAltNumTruncatedBytes(0)
{
    fAltBuffer = new unsigned char[bufferSize];
    u_int8_t* p = fPageHeaderBytes;
    *p++ = 0x4f;
    *p++ = 0x67;
    *p++ = 0x67;
    *p++ = 0x53;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 1;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p   = 0;
}

OggFileSink::~OggFileSink()
{
    fHaveSeenEOF = True;
    OggFileSink::addData(fAltBuffer, fAltFrameSize, fAltPresentationTime);
    delete[] fAltBuffer;
}

Boolean OggFileSink::continuePlaying()
{
    if (fSource == NULL) {
        return False;
    }
    fSource->getNextFrame(fBuffer, fBufferSize,
                          FileSink::afterGettingFrame, this,
                          ourOnSourceClosure, this);
    return True;
}

#define PAGE_DATA_MAX_SIZE (255*255)
void OggFileSink::addData(unsigned char const* data, unsigned dataSize,
                          struct timeval presentationTime)
{
    if (dataSize == 0) {
        return;
    }
    if (fIsTheora) {
        if (!(data[0] >= 0x80 && data[0] <= 0x82)) {
            fGranulePosition += fGranuleIncrementPerFrame;
        }
    } else {
        double ptDiff =
            (presentationTime.tv_sec - fFirstPresentationTime.tv_sec)
            + (presentationTime.tv_usec - fFirstPresentationTime.tv_usec) / 1000000.0;
        int64_t newGranulePosition =
            (int64_t) (fSamplingFrequency * ptDiff) + fGranulePositionAdjustment;
        if (newGranulePosition < fGranulePosition) {
            fGranulePositionAdjustment += fGranulePosition - newGranulePosition;
        } else {
            fGranulePosition = newGranulePosition;
        }
    }
    unsigned numPagesToWrite = dataSize / PAGE_DATA_MAX_SIZE + 1;
    for (unsigned i = 0; i < numPagesToWrite; ++i) {
        u_int8_t header_type_flag = 0x0;
        if (!fHaveWrittenFirstFrame && i == 0) {
            header_type_flag      |= 0x02;
            fHaveWrittenFirstFrame = True;
        }
        if (i > 0) {
            header_type_flag |= 0x01;
        }
        if (fHaveSeenEOF && i == numPagesToWrite - 1) {
            header_type_flag |= 0x04;
        }
        fPageHeaderBytes[5] = header_type_flag;
        if (i < numPagesToWrite - 1) {
            fPageHeaderBytes[6]      = fPageHeaderBytes[7] = fPageHeaderBytes[8] = fPageHeaderBytes[9] =
                fPageHeaderBytes[10] = fPageHeaderBytes[11] = fPageHeaderBytes[12] = fPageHeaderBytes[13] =
                    0xFF;
        } else {
            fPageHeaderBytes[6]  = (u_int8_t) fGranulePosition;
            fPageHeaderBytes[7]  = (u_int8_t) (fGranulePosition >> 8);
            fPageHeaderBytes[8]  = (u_int8_t) (fGranulePosition >> 16);
            fPageHeaderBytes[9]  = (u_int8_t) (fGranulePosition >> 24);
            fPageHeaderBytes[10] = (u_int8_t) (fGranulePosition >> 32);
            fPageHeaderBytes[11] = (u_int8_t) (fGranulePosition >> 40);
            fPageHeaderBytes[12] = (u_int8_t) (fGranulePosition >> 48);
            fPageHeaderBytes[13] = (u_int8_t) (fGranulePosition >> 56);
        }
        fPageHeaderBytes[18] = (u_int8_t) fPageSequenceNumber;
        fPageHeaderBytes[19] = (u_int8_t) (fPageSequenceNumber >> 8);
        fPageHeaderBytes[20] = (u_int8_t) (fPageSequenceNumber >> 16);
        fPageHeaderBytes[21] = (u_int8_t) (fPageSequenceNumber >> 24);
        ++fPageSequenceNumber;
        unsigned pageDataSize;
        u_int8_t number_page_segments;
        if (dataSize >= PAGE_DATA_MAX_SIZE) {
            pageDataSize         = PAGE_DATA_MAX_SIZE;
            number_page_segments = 255;
        } else {
            pageDataSize         = dataSize;
            number_page_segments = (pageDataSize + 255) / 255;
        }
        fPageHeaderBytes[26] = number_page_segments;
        u_int8_t segment_table[255];
        for (unsigned j = 0; j < (unsigned) (number_page_segments - 1); ++j) {
            segment_table[j] = 255;
        }
        segment_table[number_page_segments - 1] = pageDataSize % 255;
        u_int32_t crc = 0;
        fPageHeaderBytes[22] = fPageHeaderBytes[23] = fPageHeaderBytes[24] = fPageHeaderBytes[25] = 0;
        crc = calculateCRC(fPageHeaderBytes, 27, 0);
        crc = calculateCRC(segment_table, number_page_segments, crc);
        crc = calculateCRC(data, pageDataSize, crc);
        fPageHeaderBytes[22] = (u_int8_t) crc;
        fPageHeaderBytes[23] = (u_int8_t) (crc >> 8);
        fPageHeaderBytes[24] = (u_int8_t) (crc >> 16);
        fPageHeaderBytes[25] = (u_int8_t) (crc >> 24);
        FileSink::addData(fPageHeaderBytes, 27, presentationTime);
        FileSink::addData(segment_table, number_page_segments, presentationTime);
        FileSink::addData(data, pageDataSize, presentationTime);
        data     += pageDataSize;
        dataSize -= pageDataSize;
    }
} // OggFileSink::addData

void OggFileSink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime)
{
    if (!fHaveWrittenFirstFrame) {
        fFirstPresentationTime = presentationTime;
        if (fConfigStr != NULL && fConfigStr[0] != '\0') {
            u_int8_t* identificationHdr;
            unsigned identificationHdrSize;
            u_int8_t* commentHdr;
            unsigned commentHdrSize;
            u_int8_t* setupHdr;
            unsigned setupHdrSize;
            u_int32_t identField;
            parseVorbisOrTheoraConfigStr(fConfigStr,
                                         identificationHdr, identificationHdrSize,
                                         commentHdr, commentHdrSize,
                                         setupHdr, setupHdrSize,
                                         identField);
            if (identificationHdrSize >= 42 &&
                strncmp((const char *) &identificationHdr[1], "theora", 6) == 0)
            {
                fIsTheora = True;
                u_int8_t const KFGSHIFT = ((identificationHdr[40] & 3) << 3) | (identificationHdr[41] >> 5);
                fGranuleIncrementPerFrame = (u_int64_t) (1 << KFGSHIFT);
            }
            OggFileSink::addData(identificationHdr, identificationHdrSize, presentationTime);
            OggFileSink::addData(commentHdr, commentHdrSize, presentationTime);
            if (setupHdrSize > fBufferSize) {
                fAltFrameSize         = fBufferSize;
                fAltNumTruncatedBytes = setupHdrSize - fBufferSize;
            } else {
                fAltFrameSize         = setupHdrSize;
                fAltNumTruncatedBytes = 0;
            }
            memmove(fAltBuffer, setupHdr, fAltFrameSize);
            fAltPresentationTime = presentationTime;
            delete[] identificationHdr;
            delete[] commentHdr;
            delete[] setupHdr;
        }
    }
    unsigned char* tmpPtr = fBuffer;
    fBuffer    = fAltBuffer;
    fAltBuffer = tmpPtr;
    unsigned prevFrameSize = fAltFrameSize;
    fAltFrameSize = frameSize;
    unsigned prevNumTruncatedBytes = fAltNumTruncatedBytes;
    fAltNumTruncatedBytes = numTruncatedBytes;
    struct timeval prevPresentationTime = fAltPresentationTime;
    fAltPresentationTime = presentationTime;
    FileSink::afterGettingFrame(prevFrameSize, prevNumTruncatedBytes, prevPresentationTime);
} // OggFileSink::afterGettingFrame

void OggFileSink::ourOnSourceClosure(void* clientData)
{
    ((OggFileSink *) clientData)->ourOnSourceClosure();
}

void OggFileSink::ourOnSourceClosure()
{
    fHaveSeenEOF = True;
    OggFileSink::addData(fAltBuffer, fAltFrameSize, fAltPresentationTime);
    onSourceClosure();
}
