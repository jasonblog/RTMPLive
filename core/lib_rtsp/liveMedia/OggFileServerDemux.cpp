#include "OggFileServerDemux.hh"
#include "OggFileServerMediaSubsession.hh"
void OggFileServerDemux
::createNew(UsageEnvironment& env, char const* fileName,
            onCreationFunc* onCreation, void* onCreationClientData)
{
    (void) new OggFileServerDemux(env, fileName,
                                  onCreation, onCreationClientData);
}

ServerMediaSubsession * OggFileServerDemux::newServerMediaSubsession()
{
    u_int32_t dummyResultTrackNumber;

    return newServerMediaSubsession(dummyResultTrackNumber);
}

ServerMediaSubsession * OggFileServerDemux
::newServerMediaSubsession(u_int32_t& resultTrackNumber)
{
    resultTrackNumber = 0;
    OggTrack* nextTrack = fIter->next();
    if (nextTrack == NULL) {
        return NULL;
    }
    return newServerMediaSubsessionByTrackNumber(nextTrack->trackNumber);
}

ServerMediaSubsession * OggFileServerDemux
::newServerMediaSubsessionByTrackNumber(u_int32_t trackNumber)
{
    OggTrack* track = fOurOggFile->lookup(trackNumber);

    if (track == NULL) {
        return NULL;
    }
    ServerMediaSubsession* result = OggFileServerMediaSubsession::createNew(*this, track);
    if (result != NULL) {
        #ifdef DEBUG
        fprintf(stderr, "Created 'ServerMediaSubsession' object for track #%d: (%s)\n", track->trackNumber,
                track->mimeType);
        #endif
    }
    return result;
}

FramedSource * OggFileServerDemux::newDemuxedTrack(unsigned clientSessionId, u_int32_t trackNumber)
{
    OggDemux* demuxToUse = NULL;

    if (clientSessionId != 0 && clientSessionId == fLastClientSessionId) {
        demuxToUse = fLastCreatedDemux;
    }
    if (demuxToUse == NULL) {
        demuxToUse = fOurOggFile->newDemux();
    }
    fLastClientSessionId = clientSessionId;
    fLastCreatedDemux    = demuxToUse;
    return demuxToUse->newDemuxedTrackByTrackNumber(trackNumber);
}

OggFileServerDemux
::OggFileServerDemux(UsageEnvironment& env, char const* fileName,
                     onCreationFunc* onCreation, void* onCreationClientData)
    : Medium(env),
    fFileName(fileName), fOnCreation(onCreation), fOnCreationClientData(onCreationClientData),
    fIter(NULL),
    fLastClientSessionId(0), fLastCreatedDemux(NULL)
{
    OggFile::createNew(env, fileName, onOggFileCreation, this);
}

OggFileServerDemux::~OggFileServerDemux()
{
    Medium::close(fOurOggFile);
    delete fIter;
}

void OggFileServerDemux::onOggFileCreation(OggFile* newFile, void* clientData)
{
    ((OggFileServerDemux *) clientData)->onOggFileCreation(newFile);
}

void OggFileServerDemux::onOggFileCreation(OggFile* newFile)
{
    fOurOggFile = newFile;
    fIter       = new OggTrackTableIterator(fOurOggFile->trackTable());
    if (fOnCreation != NULL) {
        (*fOnCreation)(this, fOnCreationClientData);
    }
}
