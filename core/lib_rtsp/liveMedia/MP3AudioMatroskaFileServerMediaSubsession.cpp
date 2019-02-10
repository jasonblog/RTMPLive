#include "MP3AudioMatroskaFileServerMediaSubsession.hh"
#include "MatroskaDemuxedTrack.hh"
MP3AudioMatroskaFileServerMediaSubsession * MP3AudioMatroskaFileServerMediaSubsession
::createNew(MatroskaFileServerDemux& demux, MatroskaTrack* track,
            Boolean generateADUs, Interleaving* interleaving)
{
    return new MP3AudioMatroskaFileServerMediaSubsession(demux, track, generateADUs, interleaving);
}

MP3AudioMatroskaFileServerMediaSubsession
::MP3AudioMatroskaFileServerMediaSubsession(MatroskaFileServerDemux& demux, MatroskaTrack* track,
                                            Boolean generateADUs, Interleaving* interleaving)
    : MP3AudioFileServerMediaSubsession(demux.envir(), demux.fileName(), False, generateADUs, interleaving),
    fOurDemux(demux), fTrackNumber(track->trackNumber)
{
    fFileDuration = fOurDemux.fileDuration();
}

MP3AudioMatroskaFileServerMediaSubsession::~MP3AudioMatroskaFileServerMediaSubsession()
{}

void MP3AudioMatroskaFileServerMediaSubsession
::seekStreamSource(FramedSource* inputSource, double& seekNPT, double, u_int64_t&)
{
    FramedSource* sourceMP3Stream;
    ADUFromMP3Source* aduStream;

    getBaseStreams(inputSource, sourceMP3Stream, aduStream);
    if (aduStream != NULL) {
        aduStream->resetInput();
    }
    ((MatroskaDemuxedTrack *) sourceMP3Stream)->seekToTime(seekNPT);
}

FramedSource * MP3AudioMatroskaFileServerMediaSubsession
::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
    FramedSource* baseMP3Source = fOurDemux.newDemuxedTrack(clientSessionId, fTrackNumber);

    return createNewStreamSourceCommon(baseMP3Source, 0, estBitrate);
}
