#include "MatroskaFileServerMediaSubsession.hh"
#include "MatroskaDemuxedTrack.hh"
#include "FramedFilter.hh"
MatroskaFileServerMediaSubsession * MatroskaFileServerMediaSubsession
::createNew(MatroskaFileServerDemux& demux, MatroskaTrack* track)
{
    return new MatroskaFileServerMediaSubsession(demux, track);
}

MatroskaFileServerMediaSubsession
::MatroskaFileServerMediaSubsession(MatroskaFileServerDemux& demux, MatroskaTrack* track)
    : FileServerMediaSubsession(demux.envir(), demux.fileName(), False),
    fOurDemux(demux), fTrack(track), fNumFiltersInFrontOfTrack(0)
{}

MatroskaFileServerMediaSubsession::~MatroskaFileServerMediaSubsession()
{}

float MatroskaFileServerMediaSubsession::duration() const
{
    return fOurDemux.fileDuration();
}

void MatroskaFileServerMediaSubsession
::seekStreamSource(FramedSource* inputSource, double& seekNPT, double, u_int64_t&)
{
    for (unsigned i = 0; i < fNumFiltersInFrontOfTrack; ++i) {
        inputSource = ((FramedFilter *) inputSource)->inputSource();
    }
    ((MatroskaDemuxedTrack *) inputSource)->seekToTime(seekNPT);
}

FramedSource * MatroskaFileServerMediaSubsession
::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
    FramedSource* baseSource = fOurDemux.newDemuxedTrack(clientSessionId, fTrack->trackNumber);

    if (baseSource == NULL) {
        return NULL;
    }
    return fOurDemux.ourMatroskaFile()
           ->createSourceForStreaming(baseSource, fTrack->trackNumber,
                                      estBitrate, fNumFiltersInFrontOfTrack);
}

RTPSink * MatroskaFileServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource *)
{
    return fOurDemux.ourMatroskaFile()
           ->createRTPSinkForTrackNumber(fTrack->trackNumber, rtpGroupsock, rtpPayloadTypeIfDynamic);
}
