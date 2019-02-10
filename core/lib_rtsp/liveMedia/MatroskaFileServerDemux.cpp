#include "MatroskaFileServerDemux.hh"
#include "MP3AudioMatroskaFileServerMediaSubsession.hh"
#include "MatroskaFileServerMediaSubsession.hh"
void MatroskaFileServerDemux
::createNew(UsageEnvironment& env, char const* fileName,
            onCreationFunc* onCreation, void* onCreationClientData,
            char const* preferredLanguage)
{
    (void) new MatroskaFileServerDemux(env, fileName,
                                       onCreation, onCreationClientData,
                                       preferredLanguage);
}

ServerMediaSubsession * MatroskaFileServerDemux::newServerMediaSubsession()
{
    unsigned dummyResultTrackNumber;

    return newServerMediaSubsession(dummyResultTrackNumber);
}

ServerMediaSubsession * MatroskaFileServerDemux
::newServerMediaSubsession(unsigned& resultTrackNumber)
{
    ServerMediaSubsession* result;

    resultTrackNumber = 0;
    for (result = NULL;
         result == NULL && fNextTrackTypeToCheck != MATROSKA_TRACK_TYPE_OTHER;
         fNextTrackTypeToCheck <<= 1)
    {
        if (fNextTrackTypeToCheck == MATROSKA_TRACK_TYPE_VIDEO) {
            resultTrackNumber = fOurMatroskaFile->chosenVideoTrackNumber();
        } else if (fNextTrackTypeToCheck == MATROSKA_TRACK_TYPE_AUDIO) {
            resultTrackNumber = fOurMatroskaFile->chosenAudioTrackNumber();
        } else if (fNextTrackTypeToCheck == MATROSKA_TRACK_TYPE_SUBTITLE) {
            resultTrackNumber = fOurMatroskaFile->chosenSubtitleTrackNumber();
        }
        result = newServerMediaSubsessionByTrackNumber(resultTrackNumber);
    }
    return result;
}

ServerMediaSubsession * MatroskaFileServerDemux
::newServerMediaSubsessionByTrackNumber(unsigned trackNumber)
{
    MatroskaTrack* track = fOurMatroskaFile->lookup(trackNumber);

    if (track == NULL) {
        return NULL;
    }
    ServerMediaSubsession* result = NULL;
    if (strcmp(track->mimeType, "audio/MPEG") == 0) {
        result = MP3AudioMatroskaFileServerMediaSubsession::createNew(*this, track);
    } else {
        result = MatroskaFileServerMediaSubsession::createNew(*this, track);
    }
    if (result != NULL) {
        #ifdef DEBUG
        fprintf(stderr, "Created 'ServerMediaSubsession' object for track #%d: %s (%s)\n", track->trackNumber,
                track->codecID, track->mimeType);
        #endif
    }
    return result;
}

FramedSource * MatroskaFileServerDemux::newDemuxedTrack(unsigned clientSessionId, unsigned trackNumber)
{
    MatroskaDemux* demuxToUse = NULL;

    if (clientSessionId != 0 && clientSessionId == fLastClientSessionId) {
        demuxToUse = fLastCreatedDemux;
    }
    if (demuxToUse == NULL) {
        demuxToUse = fOurMatroskaFile->newDemux();
    }
    fLastClientSessionId = clientSessionId;
    fLastCreatedDemux    = demuxToUse;
    return demuxToUse->newDemuxedTrackByTrackNumber(trackNumber);
}

MatroskaFileServerDemux
::MatroskaFileServerDemux(UsageEnvironment& env, char const* fileName,
                          onCreationFunc* onCreation, void* onCreationClientData,
                          char const* preferredLanguage)
    : Medium(env),
    fFileName(fileName), fOnCreation(onCreation), fOnCreationClientData(onCreationClientData),
    fNextTrackTypeToCheck(0x1), fLastClientSessionId(0), fLastCreatedDemux(NULL)
{
    MatroskaFile::createNew(env, fileName, onMatroskaFileCreation, this, preferredLanguage);
}

MatroskaFileServerDemux::~MatroskaFileServerDemux()
{
    Medium::close(fOurMatroskaFile);
}

void MatroskaFileServerDemux::onMatroskaFileCreation(MatroskaFile* newFile, void* clientData)
{
    ((MatroskaFileServerDemux *) clientData)->onMatroskaFileCreation(newFile);
}

void MatroskaFileServerDemux::onMatroskaFileCreation(MatroskaFile* newFile)
{
    fOurMatroskaFile = newFile;
    if (fOnCreation != NULL) {
        (*fOnCreation)(this, fOnCreationClientData);
    }
}
