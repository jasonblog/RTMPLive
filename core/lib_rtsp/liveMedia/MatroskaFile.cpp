#include "MatroskaFileParser.hh"
#include "MatroskaDemuxedTrack.hh"
#include <ByteStreamFileSource.hh>
#include <H264VideoStreamDiscreteFramer.hh>
#include <H265VideoStreamDiscreteFramer.hh>
#include <MPEG1or2AudioRTPSink.hh>
#include <MPEG4GenericRTPSink.hh>
#include <AC3AudioRTPSink.hh>
#include <SimpleRTPSink.hh>
#include <VorbisAudioRTPSink.hh>
#include <H264VideoRTPSink.hh>
#include <H265VideoRTPSink.hh>
#include <VP8VideoRTPSink.hh>
#include <TheoraVideoRTPSink.hh>
#include <T140TextRTPSink.hh>
class CuePoint
{
public:
    CuePoint(double cueTime, u_int64_t clusterOffsetInFile, unsigned blockNumWithinCluster);
    virtual ~CuePoint();
    static void addCuePoint(CuePoint *& root, double cueTime, u_int64_t clusterOffsetInFile,
                            unsigned blockNumWithinCluster,
                            Boolean& needToReviseBalanceOfParent);
    Boolean lookup(double& cueTime, u_int64_t& resultClusterOffsetInFile, unsigned& resultBlockNumWithinCluster);
    static void fprintf(FILE* fid, CuePoint* cuePoint);
private:
    CuePoint* fSubTree[2];
    CuePoint * left() const
    {
        return fSubTree[0];
    }

    CuePoint * right() const
    {
        return fSubTree[1];
    }

    char fBalance;
    static void rotate(unsigned direction, CuePoint *& root);
    double fCueTime;
    u_int64_t fClusterOffsetInFile;
    unsigned fBlockNumWithinCluster;
};
UsageEnvironment& operator << (UsageEnvironment& env, const CuePoint* cuePoint);
class MatroskaTrackTable
{
public:
    MatroskaTrackTable();
    virtual ~MatroskaTrackTable();
    void add(MatroskaTrack* newTrack, unsigned trackNumber);
    MatroskaTrack * lookup(unsigned trackNumber);
    unsigned numTracks() const;
    class Iterator
    {
public:
        Iterator(MatroskaTrackTable& ourTable);
        virtual ~Iterator();
        MatroskaTrack * next();
private:
        HashTable::Iterator* fIter;
    };
private:
    friend class Iterator;
    HashTable* fTable;
};
void MatroskaFile
::createNew(UsageEnvironment& env, char const* fileName, onCreationFunc* onCreation, void* onCreationClientData,
            char const* preferredLanguage)
{
    new MatroskaFile(env, fileName, onCreation, onCreationClientData, preferredLanguage);
}

MatroskaFile::MatroskaFile(UsageEnvironment& env, char const* fileName, onCreationFunc* onCreation,
                           void* onCreationClientData,
                           char const* preferredLanguage)
    : Medium(env),
    fFileName(strDup(fileName)), fOnCreation(onCreation), fOnCreationClientData(onCreationClientData),
    fPreferredLanguage(strDup(preferredLanguage)),
    fTimecodeScale(1000000), fSegmentDuration(0.0), fSegmentDataOffset(0), fClusterOffset(0), fCuesOffset(0),
    fCuePoints(NULL),
    fChosenVideoTrackNumber(0), fChosenAudioTrackNumber(0), fChosenSubtitleTrackNumber(0)
{
    fTrackTable   = new MatroskaTrackTable;
    fDemuxesTable = HashTable::create(ONE_WORD_HASH_KEYS);
    FramedSource* inputSource = ByteStreamFileSource::createNew(envir(), fileName);
    if (inputSource == NULL) {
        fParserForInitialization = NULL;
        handleEndOfTrackHeaderParsing();
    } else {
        fParserForInitialization =
            new MatroskaFileParser(*this, inputSource, handleEndOfTrackHeaderParsing, this, NULL);
    }
}

MatroskaFile::~MatroskaFile()
{
    delete fParserForInitialization;
    delete fCuePoints;
    MatroskaDemux* demux;
    while ((demux = (MatroskaDemux *) fDemuxesTable->RemoveNext()) != NULL) {
        delete demux;
    }
    delete fDemuxesTable;
    delete fTrackTable;
    delete[](char *) fPreferredLanguage;
    delete[](char *) fFileName;
}

void MatroskaFile::handleEndOfTrackHeaderParsing(void* clientData)
{
    ((MatroskaFile *) clientData)->handleEndOfTrackHeaderParsing();
}

class TrackChoiceRecord
{
public:
    unsigned trackNumber;
    u_int8_t trackType;
    unsigned choiceFlags;
};
void MatroskaFile::handleEndOfTrackHeaderParsing()
{
    unsigned numTracks = fTrackTable->numTracks();

    if (numTracks > 0) {
        TrackChoiceRecord* trackChoice = new TrackChoiceRecord[numTracks];
        unsigned numEnabledTracks      = 0;
        MatroskaTrackTable::Iterator iter(*fTrackTable);
        MatroskaTrack* track;
        while ((track = iter.next()) != NULL) {
            if (!track->isEnabled || track->trackType == 0 || track->mimeType[0] == '\0') {
                continue;
            }
            trackChoice[numEnabledTracks].trackNumber = track->trackNumber;
            trackChoice[numEnabledTracks].trackType   = track->trackType;
            unsigned choiceFlags = 0;
            if (fPreferredLanguage != NULL && track->language != NULL &&
                strcmp(fPreferredLanguage, track->language) == 0)
            {
                choiceFlags |= 1;
            }
            if (track->isForced) {
                choiceFlags |= 4;
            } else if (track->isDefault) {
                choiceFlags |= 2;
            }
            trackChoice[numEnabledTracks].choiceFlags = choiceFlags;
            ++numEnabledTracks;
        }
        for (u_int8_t trackType = 0x01; trackType != MATROSKA_TRACK_TYPE_OTHER; trackType <<= 1) {
            int bestNum         = -1;
            int bestChoiceFlags = -1;
            for (unsigned i = 0; i < numEnabledTracks; ++i) {
                if (trackChoice[i].trackType == trackType && (int) trackChoice[i].choiceFlags > bestChoiceFlags) {
                    bestNum         = i;
                    bestChoiceFlags = (int) trackChoice[i].choiceFlags;
                }
            }
            if (bestChoiceFlags >= 0) {
                if (trackType == MATROSKA_TRACK_TYPE_VIDEO) {
                    fChosenVideoTrackNumber = trackChoice[bestNum].trackNumber;
                } else if (trackType == MATROSKA_TRACK_TYPE_AUDIO) {
                    fChosenAudioTrackNumber = trackChoice[bestNum].trackNumber;
                } else {
                    fChosenSubtitleTrackNumber = trackChoice[bestNum].trackNumber;
                }
            }
        }
        delete[] trackChoice;
    }
    #ifdef DEBUG
    if (fChosenVideoTrackNumber > 0) {
        fprintf(stderr, "Chosen video track: #%d\n", fChosenVideoTrackNumber);
    } else {
        fprintf(stderr, "No chosen video track\n");
    }
    if (fChosenAudioTrackNumber > 0) {
        fprintf(stderr, "Chosen audio track: #%d\n", fChosenAudioTrackNumber);
    } else {
        fprintf(stderr, "No chosen audio track\n");
    }
    if (fChosenSubtitleTrackNumber > 0) {
        fprintf(stderr, "Chosen subtitle track: #%d\n", fChosenSubtitleTrackNumber);
    } else {
        fprintf(stderr, "No chosen subtitle track\n");
    }
    #endif // ifdef DEBUG
    delete fParserForInitialization;
    fParserForInitialization = NULL;
    if (fOnCreation != NULL) {
        (*fOnCreation)(this, fOnCreationClientData);
    }
} // MatroskaFile::handleEndOfTrackHeaderParsing

MatroskaTrack * MatroskaFile::lookup(unsigned trackNumber) const
{
    return fTrackTable->lookup(trackNumber);
}

MatroskaDemux * MatroskaFile::newDemux()
{
    MatroskaDemux* demux = new MatroskaDemux(*this);

    fDemuxesTable->Add((char const *) demux, demux);
    return demux;
}

void MatroskaFile::removeDemux(MatroskaDemux* demux)
{
    fDemuxesTable->Remove((char const *) demux);
}

float MatroskaFile::fileDuration()
{
    if (fCuePoints == NULL) {
        return 0.0;
    }
    return segmentDuration() * (timecodeScale() / 1000000000.0f);
}

FramedSource * MatroskaFile
::createSourceForStreaming(FramedSource* baseSource, unsigned trackNumber,
                           unsigned& estBitrate, unsigned& numFiltersInFrontOfTrack)
{
    if (baseSource == NULL) {
        return NULL;
    }
    FramedSource* result = baseSource;
    estBitrate = 100;
    numFiltersInFrontOfTrack = 0;
    MatroskaTrack* track = lookup(trackNumber);
    if (track != NULL) {
        if (strcmp(track->mimeType, "audio/MPEG") == 0) {
            estBitrate = 128;
        } else if (strcmp(track->mimeType, "audio/AAC") == 0) {
            estBitrate = 96;
        } else if (strcmp(track->mimeType, "audio/AC3") == 0) {
            estBitrate = 48;
        } else if (strcmp(track->mimeType, "audio/VORBIS") == 0) {
            estBitrate = 96;
        } else if (strcmp(track->mimeType, "video/H264") == 0) {
            estBitrate = 500;
            OutPacketBuffer::increaseMaxSizeTo(300000);
            result = H264VideoStreamDiscreteFramer::createNew(envir(), result);
            ++numFiltersInFrontOfTrack;
        } else if (strcmp(track->mimeType, "video/H265") == 0) {
            estBitrate = 500;
            OutPacketBuffer::increaseMaxSizeTo(300000);
            result = H265VideoStreamDiscreteFramer::createNew(envir(), result);
            ++numFiltersInFrontOfTrack;
        } else if (strcmp(track->mimeType, "video/VP8") == 0) {
            estBitrate = 500;
        } else if (strcmp(track->mimeType, "video/THEORA") == 0) {
            estBitrate = 500;
        } else if (strcmp(track->mimeType, "text/T140") == 0) {
            estBitrate = 48;
        }
    }
    return result;
} // MatroskaFile::createSourceForStreaming

#define getPrivByte(b) if (n == 0) break; else do { b = * p ++; --n; } while (0)
#define CHECK_PTR           if (ptr >= limit) break
#define NUM_BYTES_REMAINING (unsigned) (limit - ptr)
RTPSink * MatroskaFile
::createRTPSinkForTrackNumber(unsigned trackNumber, Groupsock* rtpGroupsock,
                              unsigned char rtpPayloadTypeIfDynamic)
{
    RTPSink* result = NULL;

    do {
        MatroskaTrack* track = lookup(trackNumber);
        if (track == NULL) {
            break;
        }
        if (strcmp(track->mimeType, "audio/MPEG") == 0) {
            result = MPEG1or2AudioRTPSink::createNew(envir(), rtpGroupsock);
        } else if (strcmp(track->mimeType, "audio/AAC") == 0) {
            char* configStr = new char[2 * track->codecPrivateSize + 1];
            if (configStr == NULL) {
                break;
            }
            for (unsigned i = 0; i < track->codecPrivateSize; ++i) {
                sprintf(&configStr[2 * i], "%02X", track->codecPrivate[i]);
            }
            result = MPEG4GenericRTPSink::createNew(envir(), rtpGroupsock,
                                                    rtpPayloadTypeIfDynamic,
                                                    track->samplingFrequency,
                                                    "audio", "AAC-hbr", configStr,
                                                    track->numChannels);
            delete[] configStr;
        } else if (strcmp(track->mimeType, "audio/AC3") == 0) {
            result = AC3AudioRTPSink
                     ::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, track->samplingFrequency);
        } else if (strcmp(track->mimeType, "audio/OPUS") == 0) {
            result = SimpleRTPSink
                     ::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                 48000, "audio", "OPUS", 2, False);
        } else if (strcmp(track->mimeType, "audio/VORBIS") == 0 || strcmp(track->mimeType, "video/THEORA") == 0) {
            u_int8_t* identificationHeader    = NULL;
            unsigned identificationHeaderSize = 0;
            u_int8_t* commentHeader    = NULL;
            unsigned commentHeaderSize = 0;
            u_int8_t* setupHeader      = NULL;
            unsigned setupHeaderSize   = 0;
            Boolean isTheora = strcmp(track->mimeType, "video/THEORA") == 0;
            do {
                u_int8_t* p = track->codecPrivate;
                unsigned n  = track->codecPrivateSize;
                if (n == 0 || p == NULL) {
                    break;
                }
                u_int8_t numHeaders;
                getPrivByte(numHeaders);
                unsigned headerSize[3];
                unsigned sizesSum = 0;
                Boolean success   = True;
                unsigned i;
                for (i = 0; i < numHeaders && i < 3; ++i) {
                    unsigned len = 0;
                    u_int8_t c;
                    do {
                        success = False;
                        getPrivByte(c);
                        success = True;
                        len    += c;
                    } while (c == 255);
                    if (!success || len == 0) {
                        break;
                    }
                    headerSize[i] = len;
                    sizesSum     += len;
                }
                if (!success) {
                    break;
                }
                if (numHeaders < 3) {
                    int finalHeaderSize = n - sizesSum;
                    if (finalHeaderSize <= 0) {
                        break;
                    }
                    headerSize[numHeaders] = (unsigned) finalHeaderSize;
                    ++numHeaders;
                } else {
                    numHeaders = 3;
                }
                for (i = 0; i < numHeaders; ++i) {
                    success = False;
                    unsigned newHeaderSize = headerSize[i];
                    u_int8_t* newHeader    = new u_int8_t[newHeaderSize];
                    if (newHeader == NULL) {
                        break;
                    }
                    u_int8_t* hdr = newHeader;
                    while (newHeaderSize-- > 0) {
                        success = False;
                        getPrivByte(*hdr++);
                        success = True;
                    }
                    if (!success) {
                        delete[] newHeader;
                        break;
                    }
                    u_int8_t headerType = newHeader[0];
                    if (headerType == 1 || (isTheora && headerType == 0x80)) {
                        delete[] identificationHeader;
                        identificationHeader     = newHeader;
                        identificationHeaderSize = headerSize[i];
                    } else if (headerType == 3 || (isTheora && headerType == 0x81)) {
                        delete[] commentHeader;
                        commentHeader     = newHeader;
                        commentHeaderSize = headerSize[i];
                    } else if (headerType == 5 || (isTheora && headerType == 0x82)) {
                        delete[] setupHeader;
                        setupHeader     = newHeader;
                        setupHeaderSize = headerSize[i];
                    } else {
                        delete[] newHeader;
                    }
                }
                if (!success) {
                    break;
                }
                if (isTheora) {
                    result = TheoraVideoRTPSink
                             ::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                         identificationHeader, identificationHeaderSize,
                                         commentHeader, commentHeaderSize,
                                         setupHeader, setupHeaderSize);
                } else {
                    result = VorbisAudioRTPSink
                             ::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                         track->samplingFrequency, track->numChannels,
                                         identificationHeader, identificationHeaderSize,
                                         commentHeader, commentHeaderSize,
                                         setupHeader, setupHeaderSize);
                }
            } while (0);
            delete[] identificationHeader;
            delete[] commentHeader;
            delete[] setupHeader;
        } else if (strcmp(track->mimeType, "video/H264") == 0) {
            u_int8_t* SPS              = NULL;
            unsigned SPSSize           = NULL;
            u_int8_t* PPS              = NULL;
            unsigned PPSSize           = NULL;
            u_int8_t* SPSandPPSBytes   = NULL;
            unsigned numSPSandPPSBytes = 0;
            do {
                if (track->codecPrivateSize < 6) {
                    break;
                }
                numSPSandPPSBytes = track->codecPrivateSize - 5;
                SPSandPPSBytes    = &track->codecPrivate[5];
                unsigned i;
                u_int8_t* ptr    = SPSandPPSBytes;
                u_int8_t* limit  = &SPSandPPSBytes[numSPSandPPSBytes];
                unsigned numSPSs = (*ptr++) & 0x1F;
                CHECK_PTR;
                for (i = 0; i < numSPSs; ++i) {
                    unsigned spsSize = (*ptr++) << 8;
                    CHECK_PTR;
                    spsSize |= *ptr++;
                    CHECK_PTR;
                    if (spsSize > NUM_BYTES_REMAINING) {
                        break;
                    }
                    u_int8_t nal_unit_type = ptr[0] & 0x1F;
                    if (SPS == NULL && nal_unit_type == 7) {
                        SPSSize = spsSize;
                        SPS     = new u_int8_t[spsSize];
                        memmove(SPS, ptr, spsSize);
                    }
                    ptr += spsSize;
                }
                unsigned numPPSs = (*ptr++) & 0x1F;
                CHECK_PTR;
                for (i = 0; i < numPPSs; ++i) {
                    unsigned ppsSize = (*ptr++) << 8;
                    CHECK_PTR;
                    ppsSize |= *ptr++;
                    CHECK_PTR;
                    if (ppsSize > NUM_BYTES_REMAINING) {
                        break;
                    }
                    u_int8_t nal_unit_type = ptr[0] & 0x1F;
                    if (PPS == NULL && nal_unit_type == 8) {
                        PPSSize = ppsSize;
                        PPS     = new u_int8_t[ppsSize];
                        memmove(PPS, ptr, ppsSize);
                    }
                    ptr += ppsSize;
                }
            } while (0);
            result = H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                                 SPS, SPSSize, PPS, PPSSize);
            delete[] SPS;
            delete[] PPS;
        } else if (strcmp(track->mimeType, "video/H265") == 0) {
            u_int8_t* VPS    = NULL;
            unsigned VPSSize = NULL;
            u_int8_t* SPS    = NULL;
            unsigned SPSSize = NULL;
            u_int8_t* PPS    = NULL;
            unsigned PPSSize = NULL;
            u_int8_t* VPS_SPS_PPSBytes   = NULL;
            unsigned numVPS_SPS_PPSBytes = 0;
            unsigned i;
            do {
                if (track->codecPrivateUsesH264FormatForH265) {
                    if (track->codecPrivateSize >= 6) {
                        numVPS_SPS_PPSBytes = track->codecPrivateSize - 5;
                        VPS_SPS_PPSBytes    = &track->codecPrivate[5];
                    }
                } else {
                    if (track->codecPrivateSize >= 23) {
                        numVPS_SPS_PPSBytes = track->codecPrivateSize - 22;
                        VPS_SPS_PPSBytes    = &track->codecPrivate[22];
                    }
                }
                if (numVPS_SPS_PPSBytes == 0 || VPS_SPS_PPSBytes == NULL) {
                    break;
                }
                u_int8_t* ptr   = VPS_SPS_PPSBytes;
                u_int8_t* limit = &VPS_SPS_PPSBytes[numVPS_SPS_PPSBytes];
                if (track->codecPrivateUsesH264FormatForH265) {
                    while (NUM_BYTES_REMAINING > 0) {
                        unsigned numNALUnits = (*ptr++) & 0x1F;
                        CHECK_PTR;
                        for (i = 0; i < numNALUnits; ++i) {
                            unsigned nalUnitLength = (*ptr++) << 8;
                            CHECK_PTR;
                            nalUnitLength |= *ptr++;
                            CHECK_PTR;
                            if (nalUnitLength > NUM_BYTES_REMAINING) {
                                break;
                            }
                            u_int8_t nal_unit_type = (ptr[0] & 0x7E) >> 1;
                            if (nal_unit_type == 32) {
                                VPSSize = nalUnitLength;
                                delete[] VPS;
                                VPS = new u_int8_t[nalUnitLength];
                                memmove(VPS, ptr, nalUnitLength);
                            } else if (nal_unit_type == 33) {
                                SPSSize = nalUnitLength;
                                delete[] SPS;
                                SPS = new u_int8_t[nalUnitLength];
                                memmove(SPS, ptr, nalUnitLength);
                            } else if (nal_unit_type == 34) {
                                PPSSize = nalUnitLength;
                                delete[] PPS;
                                PPS = new u_int8_t[nalUnitLength];
                                memmove(PPS, ptr, nalUnitLength);
                            }
                            ptr += nalUnitLength;
                        }
                    }
                } else {
                    unsigned numOfArrays = *ptr++;
                    CHECK_PTR;
                    for (unsigned j = 0; j < numOfArrays; ++j) {
                        ++ptr;
                        CHECK_PTR;
                        unsigned numNalus = (*ptr++) << 8;
                        CHECK_PTR;
                        numNalus |= *ptr++;
                        CHECK_PTR;
                        for (i = 0; i < numNalus; ++i) {
                            unsigned nalUnitLength = (*ptr++) << 8;
                            CHECK_PTR;
                            nalUnitLength |= *ptr++;
                            CHECK_PTR;
                            if (nalUnitLength > NUM_BYTES_REMAINING) {
                                break;
                            }
                            u_int8_t nal_unit_type = (ptr[0] & 0x7E) >> 1;
                            if (nal_unit_type == 32) {
                                VPSSize = nalUnitLength;
                                delete[] VPS;
                                VPS = new u_int8_t[nalUnitLength];
                                memmove(VPS, ptr, nalUnitLength);
                            } else if (nal_unit_type == 33) {
                                SPSSize = nalUnitLength;
                                delete[] SPS;
                                SPS = new u_int8_t[nalUnitLength];
                                memmove(SPS, ptr, nalUnitLength);
                            } else if (nal_unit_type == 34) {
                                PPSSize = nalUnitLength;
                                delete[] PPS;
                                PPS = new u_int8_t[nalUnitLength];
                                memmove(PPS, ptr, nalUnitLength);
                            }
                            ptr += nalUnitLength;
                        }
                    }
                }
            } while (0);
            result = H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                                 VPS, VPSSize, SPS, SPSSize, PPS, PPSSize);
            delete[] VPS;
            delete[] SPS;
            delete[] PPS;
        } else if (strcmp(track->mimeType, "video/VP8") == 0) {
            result = VP8VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
        } else if (strcmp(track->mimeType, "text/T140") == 0) {
            result = T140TextRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
        }
    } while (0);
    return result;
} // MatroskaFile::createRTPSinkForTrackNumber

void MatroskaFile::addTrack(MatroskaTrack* newTrack, unsigned trackNumber)
{
    fTrackTable->add(newTrack, trackNumber);
}

void MatroskaFile::addCuePoint(double cueTime, u_int64_t clusterOffsetInFile, unsigned blockNumWithinCluster)
{
    Boolean dummy = False;

    CuePoint::addCuePoint(fCuePoints, cueTime, clusterOffsetInFile, blockNumWithinCluster, dummy);
}

Boolean MatroskaFile::lookupCuePoint(double& cueTime, u_int64_t& resultClusterOffsetInFile,
                                     unsigned& resultBlockNumWithinCluster)
{
    if (fCuePoints == NULL) {
        return False;
    }
    (void) fCuePoints->lookup(cueTime, resultClusterOffsetInFile, resultBlockNumWithinCluster);
    return True;
}

void MatroskaFile::printCuePoints(FILE* fid)
{
    CuePoint::fprintf(fid, fCuePoints);
}

MatroskaTrackTable::MatroskaTrackTable()
    : fTable(HashTable::create(ONE_WORD_HASH_KEYS))
{}

MatroskaTrackTable::~MatroskaTrackTable()
{
    MatroskaTrack* track;

    while ((track = (MatroskaTrack *) fTable->RemoveNext()) != NULL) {
        delete track;
    }
    delete fTable;
}

void MatroskaTrackTable::add(MatroskaTrack* newTrack, unsigned trackNumber)
{
    if (newTrack != NULL && newTrack->trackNumber != 0) {
        fTable->Remove((char const *) newTrack->trackNumber);
    }
    MatroskaTrack* existingTrack = (MatroskaTrack *) fTable->Add((char const *) trackNumber, newTrack);
    delete existingTrack;
}

MatroskaTrack * MatroskaTrackTable::lookup(unsigned trackNumber)
{
    return (MatroskaTrack *) fTable->Lookup((char const *) trackNumber);
}

unsigned MatroskaTrackTable::numTracks() const
{
    return fTable->numEntries();
}

MatroskaTrackTable::Iterator::Iterator(MatroskaTrackTable& ourTable)
{
    fIter = HashTable::Iterator::create(*(ourTable.fTable));
}

MatroskaTrackTable::Iterator::~Iterator()
{
    delete fIter;
}

MatroskaTrack * MatroskaTrackTable::Iterator::next()
{
    char const* key;

    return (MatroskaTrack *) fIter->next(key);
}

MatroskaTrack::MatroskaTrack()
    : trackNumber(0), trackType(0),
    isEnabled(True), isDefault(True), isForced(False),
    defaultDuration(0),
    name(NULL), language(NULL), codecID(NULL),
    samplingFrequency(0), numChannels(2), mimeType(""),
    codecPrivateSize(0), codecPrivate(NULL),
    codecPrivateUsesH264FormatForH265(False), codecIsOpus(False),
    headerStrippedBytesSize(0), headerStrippedBytes(NULL),
    subframeSizeSize(0)
{}

MatroskaTrack::~MatroskaTrack()
{
    delete[] name;
    delete[] language;
    delete[] codecID;
    delete[] codecPrivate;
    delete[] headerStrippedBytes;
}

MatroskaDemux::MatroskaDemux(MatroskaFile& ourFile)
    : Medium(ourFile.envir()),
    fOurFile(ourFile), fDemuxedTracksTable(HashTable::create(ONE_WORD_HASH_KEYS)),
    fNextTrackTypeToCheck(0x1)
{
    fOurParser = new MatroskaFileParser(ourFile, ByteStreamFileSource::createNew(envir(), ourFile.fileName()),
                                        handleEndOfFile, this, this);
}

MatroskaDemux::~MatroskaDemux()
{
    handleEndOfFile();
    delete fDemuxedTracksTable;
    delete fOurParser;
    fOurFile.removeDemux(this);
}

FramedSource * MatroskaDemux::newDemuxedTrack()
{
    unsigned dummyResultTrackNumber;

    return newDemuxedTrack(dummyResultTrackNumber);
}

FramedSource * MatroskaDemux::newDemuxedTrack(unsigned& resultTrackNumber)
{
    FramedSource* result;

    resultTrackNumber = 0;
    for (result = NULL; result == NULL && fNextTrackTypeToCheck != MATROSKA_TRACK_TYPE_OTHER;
         fNextTrackTypeToCheck <<= 1)
    {
        if (fNextTrackTypeToCheck == MATROSKA_TRACK_TYPE_VIDEO) {
            resultTrackNumber = fOurFile.chosenVideoTrackNumber();
        } else if (fNextTrackTypeToCheck == MATROSKA_TRACK_TYPE_AUDIO) {
            resultTrackNumber = fOurFile.chosenAudioTrackNumber();
        } else if (fNextTrackTypeToCheck == MATROSKA_TRACK_TYPE_SUBTITLE) {
            resultTrackNumber = fOurFile.chosenSubtitleTrackNumber();
        }
        result = newDemuxedTrackByTrackNumber(resultTrackNumber);
    }
    return result;
}

FramedSource * MatroskaDemux::newDemuxedTrackByTrackNumber(unsigned trackNumber)
{
    if (trackNumber == 0) {
        return NULL;
    }
    FramedSource* trackSource = new MatroskaDemuxedTrack(envir(), trackNumber, *this);
    fDemuxedTracksTable->Add((char const *) trackNumber, trackSource);
    return trackSource;
}

MatroskaDemuxedTrack * MatroskaDemux::lookupDemuxedTrack(unsigned trackNumber)
{
    return (MatroskaDemuxedTrack *) fDemuxedTracksTable->Lookup((char const *) trackNumber);
}

void MatroskaDemux::removeTrack(unsigned trackNumber)
{
    fDemuxedTracksTable->Remove((char const *) trackNumber);
    if (fDemuxedTracksTable->numEntries() == 0) {
        Medium::close(this);
    }
}

void MatroskaDemux::continueReading()
{
    fOurParser->continueParsing();
}

void MatroskaDemux::seekToTime(double& seekNPT)
{
    if (fOurParser != NULL) {
        fOurParser->seekToTime(seekNPT);
    }
}

void MatroskaDemux::handleEndOfFile(void* clientData)
{
    ((MatroskaDemux *) clientData)->handleEndOfFile();
}

void MatroskaDemux::handleEndOfFile()
{
    unsigned numTracks = fDemuxedTracksTable->numEntries();

    if (numTracks == 0) {
        return;
    }
    MatroskaDemuxedTrack** tracks = new MatroskaDemuxedTrack *[numTracks];
    HashTable::Iterator* iter     = HashTable::Iterator::create(*fDemuxedTracksTable);
    unsigned i;
    char const* trackNumber;
    for (i = 0; i < numTracks; ++i) {
        tracks[i] = (MatroskaDemuxedTrack *) iter->next(trackNumber);
    }
    delete iter;
    for (i = 0; i < numTracks; ++i) {
        if (tracks[i] == NULL) {
            continue;
        }
        tracks[i]->handleClosure();
    }
    delete[] tracks;
}

CuePoint::CuePoint(double cueTime, u_int64_t clusterOffsetInFile, unsigned blockNumWithinCluster)
    : fBalance(0),
    fCueTime(cueTime), fClusterOffsetInFile(clusterOffsetInFile), fBlockNumWithinCluster(blockNumWithinCluster - 1)
{
    fSubTree[0] = fSubTree[1] = NULL;
}

CuePoint::~CuePoint()
{
    delete fSubTree[0];
    delete fSubTree[1];
}

void CuePoint::addCuePoint(CuePoint *& root, double cueTime, u_int64_t clusterOffsetInFile,
                           unsigned blockNumWithinCluster,
                           Boolean& needToReviseBalanceOfParent)
{
    needToReviseBalanceOfParent = False;
    if (root == NULL) {
        root = new CuePoint(cueTime, clusterOffsetInFile, blockNumWithinCluster);
        needToReviseBalanceOfParent = True;
    } else if (cueTime == root->fCueTime) {
        root->fClusterOffsetInFile   = clusterOffsetInFile;
        root->fBlockNumWithinCluster = blockNumWithinCluster - 1;
    } else {
        int direction = cueTime > root->fCueTime;
        Boolean needToReviseOurBalance = False;
        addCuePoint(root->fSubTree[direction], cueTime, clusterOffsetInFile, blockNumWithinCluster,
                    needToReviseOurBalance);
        if (needToReviseOurBalance) {
            if (root->fBalance == 0) {
                root->fBalance = -1 + 2 * direction;
                needToReviseBalanceOfParent = True;
            } else if (root->fBalance == 1 - 2 * direction) {
                root->fBalance = 0;
            } else {
                if (root->fSubTree[direction]->fBalance == -1 + 2 * direction) {
                    root->fBalance = root->fSubTree[direction]->fBalance = 0;
                    rotate(1 - direction, root);
                } else {
                    char newParentCurBalance = root->fSubTree[direction]->fSubTree[1 - direction]->fBalance;
                    if (newParentCurBalance == 1 - 2 * direction) {
                        root->fBalance = 0;
                        root->fSubTree[direction]->fBalance = -1 + 2 * direction;
                    } else if (newParentCurBalance == 0) {
                        root->fBalance = 0;
                        root->fSubTree[direction]->fBalance = 0;
                    } else {
                        root->fBalance = 1 - 2 * direction;
                        root->fSubTree[direction]->fBalance = 0;
                    }
                    rotate(direction, root->fSubTree[direction]);
                    root->fSubTree[direction]->fBalance = 0;
                    rotate(1 - direction, root);
                }
            }
        }
    }
} // CuePoint::addCuePoint

Boolean CuePoint::lookup(double& cueTime, u_int64_t& resultClusterOffsetInFile, unsigned& resultBlockNumWithinCluster)
{
    if (cueTime < fCueTime) {
        if (left() == NULL) {
            resultClusterOffsetInFile   = 0;
            resultBlockNumWithinCluster = 0;
            return False;
        } else {
            return left()->lookup(cueTime, resultClusterOffsetInFile, resultBlockNumWithinCluster);
        }
    } else {
        if (right() == NULL || !right()->lookup(cueTime, resultClusterOffsetInFile, resultBlockNumWithinCluster)) {
            cueTime = fCueTime;
            resultClusterOffsetInFile   = fClusterOffsetInFile;
            resultBlockNumWithinCluster = fBlockNumWithinCluster;
        }
        return True;
    }
}

void CuePoint::fprintf(FILE* fid, CuePoint* cuePoint)
{
    if (cuePoint != NULL) {
        ::fprintf(fid, "[");
        fprintf(fid, cuePoint->left());
        ::fprintf(fid, ",%.1f{%d},", cuePoint->fCueTime, cuePoint->fBalance);
        fprintf(fid, cuePoint->right());
        ::fprintf(fid, "]");
    }
}

void CuePoint::rotate(unsigned direction, CuePoint *& root)
{
    CuePoint* pivot = root->fSubTree[1 - direction];

    root->fSubTree[1 - direction] = pivot->fSubTree[direction];
    pivot->fSubTree[direction]    = root;
    root = pivot;
}
