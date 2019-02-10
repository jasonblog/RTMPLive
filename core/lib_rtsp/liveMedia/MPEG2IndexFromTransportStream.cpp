#include "MPEG2IndexFromTransportStream.hh"
enum RecordType {
    RECORD_UNPARSED            = 0,
    RECORD_VSH                 = 1,
    RECORD_GOP                 = 2,
    RECORD_PIC_NON_IFRAME      = 3,
    RECORD_PIC_IFRAME          = 4,
    RECORD_NAL_H264_SPS        = 5,
    RECORD_NAL_H264_PPS        = 6,
    RECORD_NAL_H264_SEI        = 7,
    RECORD_NAL_H264_NON_IFRAME = 8,
    RECORD_NAL_H264_IFRAME     = 9,
    RECORD_NAL_H264_OTHER      = 10,
    RECORD_NAL_H265_VPS        = 11,
    RECORD_NAL_H265_SPS        = 12,
    RECORD_NAL_H265_PPS        = 13,
    RECORD_NAL_H265_NON_IFRAME = 14,
    RECORD_NAL_H265_IFRAME     = 15,
    RECORD_NAL_H265_OTHER      = 16,
    RECORD_JUNK
};
class IndexRecord
{
public:
    IndexRecord(u_int8_t startOffset, u_int8_t size,
                unsigned long transportPacketNumber, float pcr);
    virtual ~IndexRecord();
    RecordType& recordType()
    {
        return fRecordType;
    }

    void setFirstFlag()
    {
        fRecordType = (RecordType) (((u_int8_t) fRecordType) | 0x80);
    }

    u_int8_t startOffset() const
    {
        return fStartOffset;
    }

    u_int8_t& size()
    {
        return fSize;
    }

    float pcr() const
    {
        return fPCR;
    }

    unsigned long transportPacketNumber() const
    {
        return fTransportPacketNumber;
    }

    IndexRecord * next() const
    {
        return fNext;
    }

    void addAfter(IndexRecord* prev);
    void unlink();
private:
    IndexRecord* fNext;
    IndexRecord* fPrev;
    RecordType fRecordType;
    u_int8_t fStartOffset;
    u_int8_t fSize;
    float fPCR;
    unsigned long fTransportPacketNumber;
};
#ifdef DEBUG
static char const* recordTypeStr[] = {
    "UNPARSED",
    "VSH",
    "GOP",
    "PIC(non-I-frame)",
    "PIC(I-frame)",
    "SPS (H.264)",
    "PPS (H.264)",
    "SEI (H.264)",
    "H.264 non-I-frame",
    "H.264 I-frame",
    "other NAL unit (H.264)",
    "VPS (H.265)",
    "SPS (H.265)",
    "PPS (H.265)",
    "H.265 non-I-frame",
    "H.265 I-frame",
    "other NAL unit (H.265)",
    "JUNK"
};
UsageEnvironment& operator << (UsageEnvironment& env, IndexRecord& r)
{
    return env << "[" << ((r.recordType() & 0x80) != 0 ? "1" : "")
               << recordTypeStr[r.recordType() & 0x7F] << ":"
               << (unsigned) r.transportPacketNumber() << ":" << r.startOffset()
               << "(" << r.size() << ")@" << r.pcr() << "]";
}

#endif // ifdef DEBUG
MPEG2IFrameIndexFromTransportStream * MPEG2IFrameIndexFromTransportStream::createNew(UsageEnvironment& env,
                                                                                     FramedSource*   inputSource)
{
    return new MPEG2IFrameIndexFromTransportStream(env, inputSource);
}

#define MAX_FRAME_SIZE    400000
#define PARSE_BUFFER_SIZE (2*MAX_FRAME_SIZE)
#define PAT_PID           0
MPEG2IFrameIndexFromTransportStream
::MPEG2IFrameIndexFromTransportStream(UsageEnvironment& env,
                                      FramedSource*   inputSource)
    : FramedFilter(env, inputSource),
    fIsH264(False), fIsH265(False),
    fInputTransportPacketCounter((unsigned) -1), fClosureNumber(0), fLastContinuityCounter(~0),
    fFirstPCR(0.0), fLastPCR(0.0), fHaveSeenFirstPCR(False),
    fPMT_PID(0x10), fVideo_PID(0xE0),
    fParseBufferSize(PARSE_BUFFER_SIZE),
    fParseBufferFrameStart(0), fParseBufferParseEnd(4), fParseBufferDataEnd(0),
    fHeadIndexRecord(NULL), fTailIndexRecord(NULL)
{
    fParseBuffer = new unsigned char[fParseBufferSize];
}

MPEG2IFrameIndexFromTransportStream::~MPEG2IFrameIndexFromTransportStream()
{
    delete fHeadIndexRecord;
    delete[] fParseBuffer;
}

void MPEG2IFrameIndexFromTransportStream::doGetNextFrame()
{
    if (deliverIndexRecord()) {
        return;
    }
    if (parseFrame()) {
        doGetNextFrame();
        return;
    }
    if (fParseBufferSize - fParseBufferDataEnd < TRANSPORT_PACKET_SIZE) {
        compactParseBuffer();
        if (fParseBufferSize - fParseBufferDataEnd < TRANSPORT_PACKET_SIZE) {
            envir() << "ERROR: parse buffer full; increase MAX_FRAME_SIZE\n";
            handleInputClosure1();
            return;
        }
    }
    fInputSource->getNextFrame(fInputBuffer, sizeof fInputBuffer,
                               afterGettingFrame, this,
                               handleInputClosure, this);
}

void MPEG2IFrameIndexFromTransportStream
::afterGettingFrame(void* clientData, unsigned frameSize,
                    unsigned numTruncatedBytes,
                    struct timeval presentationTime,
                    unsigned durationInMicroseconds)
{
    MPEG2IFrameIndexFromTransportStream* source =
        (MPEG2IFrameIndexFromTransportStream *) clientData;

    source->afterGettingFrame1(frameSize, numTruncatedBytes,
                               presentationTime, durationInMicroseconds);
}

#define TRANSPORT_SYNC_BYTE 0x47
void MPEG2IFrameIndexFromTransportStream
::afterGettingFrame1(unsigned       frameSize,
                     unsigned       numTruncatedBytes,
                     struct timeval presentationTime,
                     unsigned       durationInMicroseconds)
{
    if (frameSize < TRANSPORT_PACKET_SIZE || fInputBuffer[0] != TRANSPORT_SYNC_BYTE) {
        if (fInputBuffer[0] != TRANSPORT_SYNC_BYTE) {
            envir() << "Bad TS sync byte: 0x" << fInputBuffer[0] << "\n";
        }
        handleInputClosure1();
        return;
    }
    ++fInputTransportPacketCounter;
    u_int8_t adaptation_field_control = (fInputBuffer[3] & 0x30) >> 4;
    u_int8_t totalHeaderSize =
        adaptation_field_control <= 1 ? 4 : 5 + fInputBuffer[4];
    if (adaptation_field_control == 2 && totalHeaderSize != TRANSPORT_PACKET_SIZE ||
        adaptation_field_control == 3 && totalHeaderSize >= TRANSPORT_PACKET_SIZE)
    {
        envir() << "Bad \"adaptation_field_length\": " << fInputBuffer[4] << "\n";
        doGetNextFrame();
        return;
    }
    if (totalHeaderSize > 5 && (fInputBuffer[5] & 0x10) != 0) {
        u_int32_t pcrBaseHigh =
            (fInputBuffer[6] << 24) | (fInputBuffer[7] << 16)
            | (fInputBuffer[8] << 8) | fInputBuffer[9];
        float pcr = pcrBaseHigh / 45000.0f;
        if ((fInputBuffer[10] & 0x80) != 0) {
            pcr += 1 / 90000.0f;
        }
        unsigned short pcrExt = ((fInputBuffer[10] & 0x01) << 8) | fInputBuffer[11];
        pcr += pcrExt / 27000000.0f;
        if (!fHaveSeenFirstPCR) {
            fFirstPCR         = pcr;
            fHaveSeenFirstPCR = True;
        } else if (pcr < fLastPCR) {
            envir() << "\nWarning: At about " << fLastPCR - fFirstPCR
                    << " seconds into the file, the PCR timestamp decreased - from "
                    << fLastPCR << " to " << pcr << "\n";
            fFirstPCR -= (fLastPCR - pcr);
        }
        fLastPCR = pcr;
    }
    u_int16_t PID = ((fInputBuffer[1] & 0x1F) << 8) | fInputBuffer[2];
    if (PID == PAT_PID) {
        analyzePAT(&fInputBuffer[totalHeaderSize], TRANSPORT_PACKET_SIZE - totalHeaderSize);
    } else if (PID == fPMT_PID) {
        analyzePMT(&fInputBuffer[totalHeaderSize], TRANSPORT_PACKET_SIZE - totalHeaderSize);
    }
    u_int8_t continuity_counter = fInputBuffer[3] & 0x0F;
    if ((PID != fVideo_PID) ||
        !(adaptation_field_control == 1 || adaptation_field_control == 3) ||
        continuity_counter == fLastContinuityCounter)
    {
        doGetNextFrame();
        return;
    }
    fLastContinuityCounter = continuity_counter;
    Boolean payload_unit_start_indicator = (fInputBuffer[1] & 0x40) != 0;
    if (payload_unit_start_indicator && totalHeaderSize < TRANSPORT_PACKET_SIZE - 8 &&
        fInputBuffer[totalHeaderSize] == 0x00 && fInputBuffer[totalHeaderSize + 1] == 0x00 &&
        fInputBuffer[totalHeaderSize + 2] == 0x01)
    {
        u_int8_t PES_header_data_length = fInputBuffer[totalHeaderSize + 8];
        totalHeaderSize += 9 + PES_header_data_length;
        if (totalHeaderSize >= TRANSPORT_PACKET_SIZE) {
            envir() << "Unexpectedly large PES header size: " << PES_header_data_length << "\n";
            handleInputClosure1();
            return;
        }
    }
    unsigned vesSize = TRANSPORT_PACKET_SIZE - totalHeaderSize;
    memmove(&fParseBuffer[fParseBufferDataEnd], &fInputBuffer[totalHeaderSize], vesSize);
    fParseBufferDataEnd += vesSize;
    addToTail(new IndexRecord(totalHeaderSize, vesSize, fInputTransportPacketCounter,
                              fLastPCR - fFirstPCR));
    doGetNextFrame();
} // MPEG2IFrameIndexFromTransportStream::afterGettingFrame1

void MPEG2IFrameIndexFromTransportStream::handleInputClosure(void* clientData)
{
    MPEG2IFrameIndexFromTransportStream* source =
        (MPEG2IFrameIndexFromTransportStream *) clientData;

    source->handleInputClosure1();
}

#define VIDEO_SEQUENCE_START_CODE         0xB3
#define VISUAL_OBJECT_SEQUENCE_START_CODE 0xB0
#define GROUP_START_CODE                  0xB8
#define GROUP_VOP_START_CODE              0xB3
#define PICTURE_START_CODE                0x00
#define VOP_START_CODE                    0xB6
void MPEG2IFrameIndexFromTransportStream::handleInputClosure1()
{
    if (++fClosureNumber == 1 && fParseBufferDataEnd > fParseBufferFrameStart &&
        fParseBufferDataEnd <= fParseBufferSize - 4)
    {
        fParseBuffer[fParseBufferDataEnd++] = 0;
        fParseBuffer[fParseBufferDataEnd++] = 0;
        fParseBuffer[fParseBufferDataEnd++] = 1;
        fParseBuffer[fParseBufferDataEnd++] = PICTURE_START_CODE;
        doGetNextFrame();
    } else {
        handleClosure();
    }
}

void MPEG2IFrameIndexFromTransportStream
::analyzePAT(unsigned char* pkt, unsigned size)
{
    while (size >= 17) {
        u_int16_t program_number = (pkt[9] << 8) | pkt[10];
        if (program_number != 0) {
            fPMT_PID = ((pkt[11] & 0x1F) << 8) | pkt[12];
            return;
        }
        pkt  += 4;
        size -= 4;
    }
}

void MPEG2IFrameIndexFromTransportStream
::analyzePMT(unsigned char* pkt, unsigned size)
{
    u_int16_t section_length = ((pkt[2] & 0x0F) << 8) | pkt[3];

    if ((unsigned) (4 + section_length) < size) {
        size = (4 + section_length);
    }
    if (size < 22) {
        return;
    }
    unsigned program_info_length = ((pkt[11] & 0x0F) << 8) | pkt[12];
    pkt  += 13;
    size -= 13;
    if (size < program_info_length) {
        return;
    }
    pkt  += program_info_length;
    size -= program_info_length;
    while (size >= 9) {
        u_int8_t stream_type     = pkt[0];
        u_int16_t elementary_PID = ((pkt[1] & 0x1F) << 8) | pkt[2];
        if (stream_type == 1 || stream_type == 2 ||
            stream_type == 0x1B || stream_type == 0x24)
        {
            if (stream_type == 0x1B) {
                fIsH264 = True;
            } else if (stream_type == 0x24) {
                fIsH265 = True;
            }
            fVideo_PID = elementary_PID;
            return;
        }
        u_int16_t ES_info_length = ((pkt[3] & 0x0F) << 8) | pkt[4];
        pkt  += 5;
        size -= 5;
        if (size < ES_info_length) {
            return;
        }
        pkt  += ES_info_length;
        size -= ES_info_length;
    }
} // MPEG2IFrameIndexFromTransportStream::analyzePMT

Boolean MPEG2IFrameIndexFromTransportStream::deliverIndexRecord()
{
    IndexRecord* head = fHeadIndexRecord;

    if (head == NULL) {
        return False;
    }
    if (head->recordType() == RECORD_UNPARSED) {
        return False;
    }
    IndexRecord* next = head->next();
    head->unlink();
    if (next == head) {
        fHeadIndexRecord = fTailIndexRecord = NULL;
    } else {
        fHeadIndexRecord = next;
    }
    if (head->recordType() == RECORD_JUNK) {
        delete head;
        return deliverIndexRecord();
    }
    #ifdef DEBUG
    envir() << "delivering: " << *head << "\n";
    #endif
    if (fMaxSize < 11) {
        fFrameSize = 0;
    } else {
        fTo[0] = (u_int8_t) (head->recordType());
        fTo[1] = head->startOffset();
        fTo[2] = head->size();
        float pcr         = head->pcr();
        unsigned pcr_int  = (unsigned) pcr;
        u_int8_t pcr_frac = (u_int8_t) (256 * (pcr - pcr_int));
        fTo[3] = (unsigned char) (pcr_int);
        fTo[4] = (unsigned char) (pcr_int >> 8);
        fTo[5] = (unsigned char) (pcr_int >> 16);
        fTo[6] = (unsigned char) (pcr_frac);
        unsigned long tpn = head->transportPacketNumber();
        fTo[7]     = (unsigned char) (tpn);
        fTo[8]     = (unsigned char) (tpn >> 8);
        fTo[9]     = (unsigned char) (tpn >> 16);
        fTo[10]    = (unsigned char) (tpn >> 24);
        fFrameSize = 11;
    }
    delete head;
    afterGetting(this);
    return True;
} // MPEG2IFrameIndexFromTransportStream::deliverIndexRecord

Boolean MPEG2IFrameIndexFromTransportStream::parseFrame()
{
    if (fParseBufferDataEnd - fParseBufferFrameStart < 4) {
        return False;
    }
    unsigned numInitialBadBytes = 0;
    unsigned char const* p      = &fParseBuffer[fParseBufferFrameStart];
    if (!(p[0] == 0 && p[1] == 0 && p[2] == 1)) {
        if (fParseBufferParseEnd == fParseBufferFrameStart + 4) {
            fParseBufferParseEnd = fParseBufferFrameStart;
        }
        unsigned char nextCode;
        if (!parseToNextCode(nextCode)) {
            return False;
        }
        numInitialBadBytes     = fParseBufferParseEnd - fParseBufferFrameStart;
        fParseBufferFrameStart = fParseBufferParseEnd;
        fParseBufferParseEnd  += 4;
        p = &fParseBuffer[fParseBufferFrameStart];
    }
    unsigned char curCode = p[3];
    if (fIsH264) {
        curCode &= 0x1F;
    } else if (fIsH265) {
        curCode = (curCode & 0x7E) >> 1;
    }
    RecordType curRecordType;
    unsigned char nextCode;
    if (fIsH264) {
        switch (curCode) {
            case 1:
                curRecordType = RECORD_NAL_H264_NON_IFRAME;
                if (!parseToNextCode(nextCode)) {
                    return False;
                }
                break;
            case 5:
                curRecordType = RECORD_NAL_H264_IFRAME;
                if (!parseToNextCode(nextCode)) {
                    return False;
                }
                break;
            case 6:
                curRecordType = RECORD_NAL_H264_SEI;
                if (!parseToNextCode(nextCode)) {
                    return False;
                }
                break;
            case 7:
                curRecordType = RECORD_NAL_H264_SPS;
                if (!parseToNextCode(nextCode)) {
                    return False;
                }
                break;
            case 8:
                curRecordType = RECORD_NAL_H264_PPS;
                if (!parseToNextCode(nextCode)) {
                    return False;
                }
                break;
            default:
                curRecordType = RECORD_NAL_H264_OTHER;
                if (!parseToNextCode(nextCode)) {
                    return False;
                }
                break;
        }
    } else if (fIsH265) {
        switch (curCode) {
            case 19:
            case 20:
                curRecordType = RECORD_NAL_H265_IFRAME;
                if (!parseToNextCode(nextCode)) {
                    return False;
                }
                break;
            case 32:
                curRecordType = RECORD_NAL_H265_VPS;
                if (!parseToNextCode(nextCode)) {
                    return False;
                }
                break;
            case 33:
                curRecordType = RECORD_NAL_H265_SPS;
                if (!parseToNextCode(nextCode)) {
                    return False;
                }
                break;
            case 34:
                curRecordType = RECORD_NAL_H265_PPS;
                if (!parseToNextCode(nextCode)) {
                    return False;
                }
                break;
            default:
                curRecordType = (curCode <= 31) ? RECORD_NAL_H265_NON_IFRAME : RECORD_NAL_H265_OTHER;
                if (!parseToNextCode(nextCode)) {
                    return False;
                }
                break;
        }
    } else {
        switch (curCode) {
            case VIDEO_SEQUENCE_START_CODE:
            case VISUAL_OBJECT_SEQUENCE_START_CODE:
                curRecordType = RECORD_VSH;
                while (1) {
                    if (!parseToNextCode(nextCode)) {
                        return False;
                    }
                    if (nextCode == GROUP_START_CODE ||
                        nextCode == PICTURE_START_CODE || nextCode == VOP_START_CODE)
                    {
                        break;
                    }
                    fParseBufferParseEnd += 4;
                }
                break;
            case GROUP_START_CODE:
                curRecordType = RECORD_GOP;
                while (1) {
                    if (!parseToNextCode(nextCode)) {
                        return False;
                    }
                    if (nextCode == PICTURE_START_CODE || nextCode == VOP_START_CODE) {
                        break;
                    }
                    fParseBufferParseEnd += 4;
                }
                break;
            default:
                curRecordType = RECORD_PIC_NON_IFRAME;
                while (1) {
                    if (!parseToNextCode(nextCode)) {
                        return False;
                    }
                    if (nextCode == VIDEO_SEQUENCE_START_CODE ||
                        nextCode == VISUAL_OBJECT_SEQUENCE_START_CODE ||
                        nextCode == GROUP_START_CODE || nextCode == GROUP_VOP_START_CODE ||
                        nextCode == PICTURE_START_CODE || nextCode == VOP_START_CODE)
                    {
                        break;
                    }
                    fParseBufferParseEnd += 4;
                }
                break;
        }
    }
    if (curRecordType == RECORD_PIC_NON_IFRAME) {
        if (curCode == VOP_START_CODE) {
            if ((fParseBuffer[fParseBufferFrameStart + 4] & 0xC0) == 0) {
                curRecordType = RECORD_PIC_IFRAME;
            }
        } else {
            if ((fParseBuffer[fParseBufferFrameStart + 5] & 0x38) == 0x08) {
                curRecordType = RECORD_PIC_IFRAME;
            }
        }
    }
    unsigned frameSize = fParseBufferParseEnd - fParseBufferFrameStart + numInitialBadBytes;
    #ifdef DEBUG
    envir() << "parsed " << recordTypeStr[curRecordType] << "; length "
            << frameSize << "\n";
    #endif
    for (IndexRecord* r = fHeadIndexRecord;; r = r->next()) {
        if (numInitialBadBytes >= r->size()) {
            r->recordType()     = RECORD_JUNK;
            numInitialBadBytes -= r->size();
        } else {
            r->recordType() = curRecordType;
        }
        if (r == fHeadIndexRecord) {
            r->setFirstFlag();
        }
        if (r->size() > frameSize) {
            u_int8_t newOffset = r->startOffset() + frameSize;
            u_int8_t newSize   = r->size() - frameSize;
            r->size() = frameSize;
            #ifdef DEBUG
            envir() << "tagged record (modified): " << *r << "\n";
            #endif
            IndexRecord* newRecord =
                new IndexRecord(newOffset, newSize, r->transportPacketNumber(), r->pcr());
            newRecord->addAfter(r);
            if (fTailIndexRecord == r) {
                fTailIndexRecord = newRecord;
            }
            #ifdef DEBUG
            envir() << "added extra record: " << *newRecord << "\n";
            #endif
        } else {
            #ifdef DEBUG
            envir() << "tagged record: " << *r << "\n";
            #endif
        }
        frameSize -= r->size();
        if (frameSize == 0) {
            break;
        }
        if (r == fTailIndexRecord) {
            envir() << "!!!!!Internal consistency error!!!!!\n";
            return False;
        }
    }
    fParseBufferFrameStart = fParseBufferParseEnd;
    fParseBufferParseEnd  += 4;
    return True;
} // MPEG2IFrameIndexFromTransportStream::parseFrame

Boolean MPEG2IFrameIndexFromTransportStream
::parseToNextCode(unsigned char& nextCode)
{
    unsigned char const* p   = &fParseBuffer[fParseBufferParseEnd];
    unsigned char const* end = &fParseBuffer[fParseBufferDataEnd];

    while (p <= end - 4) {
        if (p[2] > 1) {
            p += 3;
        } else if (p[2] == 0) {
            ++p;
        } else if (p[0] == 0 && p[1] == 0) {
            nextCode = p[3];
            fParseBufferParseEnd = p - &fParseBuffer[0];
            return True;
        } else {
            p += 3;
        }
    }
    fParseBufferParseEnd = p - &fParseBuffer[0];
    return False;
}

void MPEG2IFrameIndexFromTransportStream::compactParseBuffer()
{
    #ifdef DEBUG
    envir() << "Compacting parse buffer: [" << fParseBufferFrameStart
            << "," << fParseBufferParseEnd << "," << fParseBufferDataEnd << "]";
    #endif
    memmove(&fParseBuffer[0], &fParseBuffer[fParseBufferFrameStart],
            fParseBufferDataEnd - fParseBufferFrameStart);
    fParseBufferDataEnd   -= fParseBufferFrameStart;
    fParseBufferParseEnd  -= fParseBufferFrameStart;
    fParseBufferFrameStart = 0;
    #ifdef DEBUG
    envir() << "-> [" << fParseBufferFrameStart
            << "," << fParseBufferParseEnd << "," << fParseBufferDataEnd << "]\n";
    #endif
}

void MPEG2IFrameIndexFromTransportStream::addToTail(IndexRecord* newIndexRecord)
{
    #ifdef DEBUG
    envir() << "adding new: " << *newIndexRecord << "\n";
    #endif
    if (fTailIndexRecord == NULL) {
        fHeadIndexRecord = fTailIndexRecord = newIndexRecord;
    } else {
        newIndexRecord->addAfter(fTailIndexRecord);
        fTailIndexRecord = newIndexRecord;
    }
}

IndexRecord::IndexRecord(u_int8_t startOffset, u_int8_t size,
                         unsigned long transportPacketNumber, float pcr)
    : fNext(this), fPrev(this), fRecordType(RECORD_UNPARSED),
    fStartOffset(startOffset), fSize(size),
    fPCR(pcr), fTransportPacketNumber(transportPacketNumber)
{}

IndexRecord::~IndexRecord()
{
    IndexRecord* nextRecord = next();

    unlink();
    if (nextRecord != this) {
        delete nextRecord;
    }
}

void IndexRecord::addAfter(IndexRecord* prev)
{
    fNext = prev->fNext;
    fPrev = prev;
    prev->fNext->fPrev = this;
    prev->fNext        = this;
}

void IndexRecord::unlink()
{
    fNext->fPrev = fPrev;
    fPrev->fNext = fNext;
    fNext        = fPrev = this;
}
