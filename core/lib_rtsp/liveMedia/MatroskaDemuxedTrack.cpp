#include "MatroskaDemuxedTrack.hh"
#include "MatroskaFile.hh"
void MatroskaDemuxedTrack::seekToTime(double& seekNPT)
{
    fOurSourceDemux.seekToTime(seekNPT);
}

MatroskaDemuxedTrack::MatroskaDemuxedTrack(UsageEnvironment& env, unsigned trackNumber, MatroskaDemux& sourceDemux)
    : FramedSource(env),
    fOurTrackNumber(trackNumber), fOurSourceDemux(sourceDemux), fDurationImbalance(0),
    fOpusTrackNumber(0)
{
    fPrevPresentationTime.tv_sec  = 0;
    fPrevPresentationTime.tv_usec = 0;
}

MatroskaDemuxedTrack::~MatroskaDemuxedTrack()
{
    fOurSourceDemux.removeTrack(fOurTrackNumber);
}

void MatroskaDemuxedTrack::doGetNextFrame()
{
    fOurSourceDemux.continueReading();
}

char const * MatroskaDemuxedTrack::MIMEtype() const
{
    MatroskaTrack* track = fOurSourceDemux.fOurFile.lookup(fOurTrackNumber);

    if (track == NULL) {
        return "(unknown)";
    }
    return track->mimeType;
}
