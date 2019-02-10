#include "OggFileParser.hh"
#include "OggDemuxedTrack.hh"
#include "ByteStreamFileSource.hh"
#include "VorbisAudioRTPSink.hh"
#include "SimpleRTPSink.hh"
#include "TheoraVideoRTPSink.hh"
class OggTrackTable
{
public:
    OggTrackTable();
    virtual ~OggTrackTable();
    void add(OggTrack* newTrack);
    OggTrack * lookup(u_int32_t trackNumber);
    unsigned numTracks() const;
private:
    friend class OggTrackTableIterator;
    HashTable* fTable;
};
void OggFile::createNew(UsageEnvironment& env, char const* fileName,
                        onCreationFunc* onCreation, void* onCreationClientData)
{
    new OggFile(env, fileName, onCreation, onCreationClientData);
}

OggTrack * OggFile::lookup(u_int32_t trackNumber)
{
    return fTrackTable->lookup(trackNumber);
}

OggDemux * OggFile::newDemux()
{
    OggDemux* demux = new OggDemux(*this);

    fDemuxesTable->Add((char const *) demux, demux);
    return demux;
}

unsigned OggFile::numTracks() const
{
    return fTrackTable->numTracks();
}

FramedSource * OggFile
::createSourceForStreaming(FramedSource* baseSource, u_int32_t trackNumber,
                           unsigned& estBitrate, unsigned& numFiltersInFrontOfTrack)
{
    if (baseSource == NULL) {
        return NULL;
    }
    FramedSource* result = baseSource;
    numFiltersInFrontOfTrack = 0;
    OggTrack* track = lookup(trackNumber);
    if (track != NULL) {
        estBitrate = track->estBitrate;
    }
    return result;
}

RTPSink * OggFile
::createRTPSinkForTrackNumber(u_int32_t trackNumber, Groupsock* rtpGroupsock,
                              unsigned char rtpPayloadTypeIfDynamic)
{
    OggTrack* track = lookup(trackNumber);

    if (track == NULL || track->mimeType == NULL) {
        return NULL;
    }
    RTPSink* result = NULL;
    if (strcmp(track->mimeType, "audio/VORBIS") == 0) {
        result = VorbisAudioRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                               track->samplingFrequency, track->numChannels,
                                               track->vtoHdrs.header[0], track->vtoHdrs.headerSize[0],
                                               track->vtoHdrs.header[1], track->vtoHdrs.headerSize[1],
                                               track->vtoHdrs.header[2], track->vtoHdrs.headerSize[2]);
    } else if (strcmp(track->mimeType, "audio/OPUS") == 0) {
        result = SimpleRTPSink
                 ::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                             48000, "audio", "OPUS", 2, False);
    } else if (strcmp(track->mimeType, "video/THEORA") == 0) {
        result = TheoraVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                               track->vtoHdrs.header[0], track->vtoHdrs.headerSize[0],
                                               track->vtoHdrs.header[1], track->vtoHdrs.headerSize[1],
                                               track->vtoHdrs.header[2], track->vtoHdrs.headerSize[2]);
    }
    return result;
}

OggFile::OggFile(UsageEnvironment& env, char const* fileName,
                 onCreationFunc* onCreation, void* onCreationClientData)
    : Medium(env),
    fFileName(strDup(fileName)),
    fOnCreation(onCreation), fOnCreationClientData(onCreationClientData)
{
    fTrackTable   = new OggTrackTable;
    fDemuxesTable = HashTable::create(ONE_WORD_HASH_KEYS);
    FramedSource* inputSource = ByteStreamFileSource::createNew(envir(), fileName);
    if (inputSource == NULL) {
        fParserForInitialization = NULL;
        handleEndOfBosPageParsing();
    } else {
        fParserForInitialization =
            new OggFileParser(*this, inputSource, handleEndOfBosPageParsing, this);
    }
}

OggFile::~OggFile()
{
    delete fParserForInitialization;
    OggDemux* demux;
    while ((demux = (OggDemux *) fDemuxesTable->RemoveNext()) != NULL) {
        delete demux;
    }
    delete fDemuxesTable;
    delete fTrackTable;
    delete[](char *) fFileName;
}

void OggFile::handleEndOfBosPageParsing(void* clientData)
{
    ((OggFile *) clientData)->handleEndOfBosPageParsing();
}

void OggFile::handleEndOfBosPageParsing()
{
    delete fParserForInitialization;
    fParserForInitialization = NULL;
    if (fOnCreation != NULL) {
        (*fOnCreation)(this, fOnCreationClientData);
    }
}

void OggFile::addTrack(OggTrack* newTrack)
{
    fTrackTable->add(newTrack);
}

void OggFile::removeDemux(OggDemux* demux)
{
    fDemuxesTable->Remove((char const *) demux);
}

OggTrackTable::OggTrackTable()
    : fTable(HashTable::create(ONE_WORD_HASH_KEYS))
{}

OggTrackTable::~OggTrackTable()
{
    OggTrack* track;

    while ((track = (OggTrack *) fTable->RemoveNext()) != NULL) {
        delete track;
    }
    delete fTable;
}

void OggTrackTable::add(OggTrack* newTrack)
{
    OggTrack* existingTrack =
        (OggTrack *) fTable->Add((char const *) newTrack->trackNumber, newTrack);

    delete existingTrack;
}

OggTrack * OggTrackTable::lookup(u_int32_t trackNumber)
{
    return (OggTrack *) fTable->Lookup((char const *) trackNumber);
}

unsigned OggTrackTable::numTracks() const
{
    return fTable->numEntries();
}

OggTrackTableIterator::OggTrackTableIterator(OggTrackTable& ourTable)
{
    fIter = HashTable::Iterator::create(*(ourTable.fTable));
}

OggTrackTableIterator::~OggTrackTableIterator()
{
    delete fIter;
}

OggTrack * OggTrackTableIterator::next()
{
    char const* key;

    return (OggTrack *) fIter->next(key);
}

OggTrack::OggTrack()
    : trackNumber(0), mimeType(NULL),
    samplingFrequency(48000), numChannels(2), estBitrate(100)
{
    vtoHdrs.header[0]             = vtoHdrs.header[1] = vtoHdrs.header[2] = NULL;
    vtoHdrs.headerSize[0]         = vtoHdrs.headerSize[1] = vtoHdrs.headerSize[2] = 0;
    vtoHdrs.vorbis_mode_count     = 0;
    vtoHdrs.vorbis_mode_blockflag = NULL;
}

OggTrack::~OggTrack()
{
    delete[] vtoHdrs.header[0];
    delete[] vtoHdrs.header[1];
    delete[] vtoHdrs.header[2];
    delete[] vtoHdrs.vorbis_mode_blockflag;
}

FramedSource * OggDemux::newDemuxedTrack(u_int32_t& resultTrackNumber)
{
    OggTrack* nextTrack;

    do {
        nextTrack = fIter->next();
    } while (nextTrack != NULL && nextTrack->mimeType == NULL);
    if (nextTrack == NULL) {
        resultTrackNumber = 0;
        return NULL;
    }
    resultTrackNumber = nextTrack->trackNumber;
    FramedSource* trackSource = new OggDemuxedTrack(envir(), resultTrackNumber, *this);
    fDemuxedTracksTable->Add((char const *) resultTrackNumber, trackSource);
    return trackSource;
}

FramedSource * OggDemux::newDemuxedTrackByTrackNumber(unsigned trackNumber)
{
    if (trackNumber == 0) {
        return NULL;
    }
    FramedSource* trackSource = new OggDemuxedTrack(envir(), trackNumber, *this);
    fDemuxedTracksTable->Add((char const *) trackNumber, trackSource);
    return trackSource;
}

OggDemuxedTrack * OggDemux::lookupDemuxedTrack(u_int32_t trackNumber)
{
    return (OggDemuxedTrack *) fDemuxedTracksTable->Lookup((char const *) trackNumber);
}

OggDemux::OggDemux(OggFile& ourFile)
    : Medium(ourFile.envir()),
    fOurFile(ourFile), fDemuxedTracksTable(HashTable::create(ONE_WORD_HASH_KEYS)),
    fIter(new OggTrackTableIterator(*fOurFile.fTrackTable))
{
    FramedSource* fileSource = ByteStreamFileSource::createNew(envir(), ourFile.fileName());

    fOurParser = new OggFileParser(ourFile, fileSource, handleEndOfFile, this, this);
}

OggDemux::~OggDemux()
{
    handleEndOfFile();
    delete fDemuxedTracksTable;
    delete fIter;
    delete fOurParser;
    fOurFile.removeDemux(this);
}

void OggDemux::removeTrack(u_int32_t trackNumber)
{
    fDemuxedTracksTable->Remove((char const *) trackNumber);
    if (fDemuxedTracksTable->numEntries() == 0) {
        delete this;
    }
}

void OggDemux::continueReading()
{
    fOurParser->continueParsing();
}

void OggDemux::handleEndOfFile(void* clientData)
{
    ((OggDemux *) clientData)->handleEndOfFile();
}

void OggDemux::handleEndOfFile()
{
    unsigned numTracks = fDemuxedTracksTable->numEntries();

    if (numTracks == 0) {
        return;
    }
    OggDemuxedTrack** tracks  = new OggDemuxedTrack *[numTracks];
    HashTable::Iterator* iter = HashTable::Iterator::create(*fDemuxedTracksTable);
    unsigned i;
    char const* trackNumber;
    for (i = 0; i < numTracks; ++i) {
        tracks[i] = (OggDemuxedTrack *) iter->next(trackNumber);
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
