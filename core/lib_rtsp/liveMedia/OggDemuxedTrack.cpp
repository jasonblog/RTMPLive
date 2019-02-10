#include "OggDemuxedTrack.hh"
#include "OggFile.hh"
OggDemuxedTrack::OggDemuxedTrack(UsageEnvironment& env, unsigned trackNumber, OggDemux& sourceDemux)
    : FramedSource(env),
    fOurTrackNumber(trackNumber), fOurSourceDemux(sourceDemux),
    fCurrentPageIsContinuation(False)
{
    fNextPresentationTime.tv_sec  = 0;
    fNextPresentationTime.tv_usec = 0;
}

OggDemuxedTrack::~OggDemuxedTrack()
{
    fOurSourceDemux.removeTrack(fOurTrackNumber);
}

void OggDemuxedTrack::doGetNextFrame()
{
    fOurSourceDemux.continueReading();
}

char const * OggDemuxedTrack::MIMEtype() const
{
    OggTrack* track = fOurSourceDemux.fOurFile.lookup(fOurTrackNumber);

    if (track == NULL) {
        return "(unknown)";
    }
    return track->mimeType;
}
