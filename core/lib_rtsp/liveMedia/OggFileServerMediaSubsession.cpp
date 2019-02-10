#include "OggFileServerMediaSubsession.hh"
#include "OggDemuxedTrack.hh"
#include "FramedFilter.hh"
OggFileServerMediaSubsession * OggFileServerMediaSubsession
::createNew(OggFileServerDemux& demux, OggTrack* track)
{
    return new OggFileServerMediaSubsession(demux, track);
}

OggFileServerMediaSubsession
::OggFileServerMediaSubsession(OggFileServerDemux& demux, OggTrack* track)
    : FileServerMediaSubsession(demux.envir(), demux.fileName(), False),
    fOurDemux(demux), fTrack(track), fNumFiltersInFrontOfTrack(0)
{}

OggFileServerMediaSubsession::~OggFileServerMediaSubsession()
{}

FramedSource * OggFileServerMediaSubsession
::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
    FramedSource* baseSource = fOurDemux.newDemuxedTrack(clientSessionId, fTrack->trackNumber);

    if (baseSource == NULL) {
        return NULL;
    }
    return fOurDemux.ourOggFile()
           ->createSourceForStreaming(baseSource, fTrack->trackNumber,
                                      estBitrate, fNumFiltersInFrontOfTrack);
}

RTPSink * OggFileServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource *)
{
    return fOurDemux.ourOggFile()
           ->createRTPSinkForTrackNumber(fTrack->trackNumber, rtpGroupsock, rtpPayloadTypeIfDynamic);
}
