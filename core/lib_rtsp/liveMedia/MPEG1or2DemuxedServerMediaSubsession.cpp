#include "MPEG1or2DemuxedServerMediaSubsession.hh"
#include "MPEG1or2AudioStreamFramer.hh"
#include "MPEG1or2AudioRTPSink.hh"
#include "MPEG1or2VideoStreamFramer.hh"
#include "MPEG1or2VideoRTPSink.hh"
#include "AC3AudioStreamFramer.hh"
#include "AC3AudioRTPSink.hh"
#include "ByteStreamFileSource.hh"
MPEG1or2DemuxedServerMediaSubsession * MPEG1or2DemuxedServerMediaSubsession
::createNew(MPEG1or2FileServerDemux& demux, u_int8_t streamIdTag,
            Boolean reuseFirstSource, Boolean iFramesOnly, double vshPeriod)
{
    return new MPEG1or2DemuxedServerMediaSubsession(demux, streamIdTag,
                                                    reuseFirstSource,
                                                    iFramesOnly, vshPeriod);
}

MPEG1or2DemuxedServerMediaSubsession
::MPEG1or2DemuxedServerMediaSubsession(MPEG1or2FileServerDemux& demux,
                                       u_int8_t streamIdTag, Boolean reuseFirstSource,
                                       Boolean iFramesOnly, double vshPeriod)
    : OnDemandServerMediaSubsession(demux.envir(), reuseFirstSource),
    fOurDemux(demux), fStreamIdTag(streamIdTag),
    fIFramesOnly(iFramesOnly), fVSHPeriod(vshPeriod)
{}

MPEG1or2DemuxedServerMediaSubsession::~MPEG1or2DemuxedServerMediaSubsession()
{}

FramedSource * MPEG1or2DemuxedServerMediaSubsession
::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
    FramedSource* es = NULL;

    do {
        es = fOurDemux.newElementaryStream(clientSessionId, fStreamIdTag);
        if (es == NULL) {
            break;
        }
        if ((fStreamIdTag & 0xF0) == 0xC0) {
            estBitrate = 128;
            return MPEG1or2AudioStreamFramer::createNew(envir(), es);
        } else if ((fStreamIdTag & 0xF0) == 0xE0) {
            estBitrate = 500;
            return MPEG1or2VideoStreamFramer::createNew(envir(), es,
                                                        fIFramesOnly, fVSHPeriod);
        } else if (fStreamIdTag == 0xBD) {
            estBitrate = 192;
            return AC3AudioStreamFramer::createNew(envir(), es, 0x80);
        } else {
            break;
        }
    } while (0);
    Medium::close(es);
    return NULL;
}

RTPSink * MPEG1or2DemuxedServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                   FramedSource* inputSource)
{
    if ((fStreamIdTag & 0xF0) == 0xC0) {
        return MPEG1or2AudioRTPSink::createNew(envir(), rtpGroupsock);
    } else if ((fStreamIdTag & 0xF0) == 0xE0) {
        return MPEG1or2VideoRTPSink::createNew(envir(), rtpGroupsock);
    } else if (fStreamIdTag == 0xBD) {
        AC3AudioStreamFramer* audioSource =
            (AC3AudioStreamFramer *) inputSource;
        return AC3AudioRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                          audioSource->samplingRate());
    } else {
        return NULL;
    }
}

void MPEG1or2DemuxedServerMediaSubsession
::seekStreamSource(FramedSource* inputSource, double& seekNPT, double, u_int64_t&)
{
    float const dur          = duration();
    unsigned const size      = fOurDemux.fileSize();
    unsigned absBytePosition = dur == 0.0 ? 0 : (unsigned) ((seekNPT / dur) * size);

    if ((fStreamIdTag & 0xF0) == 0xC0) {
        MPEG1or2AudioStreamFramer* framer = (MPEG1or2AudioStreamFramer *) inputSource;
        framer->flushInput();
    } else if ((fStreamIdTag & 0xF0) == 0xE0) {
        MPEG1or2VideoStreamFramer* framer = (MPEG1or2VideoStreamFramer *) inputSource;
        framer->flushInput();
    }
    MPEG1or2DemuxedElementaryStream* elemStreamSource =
        (MPEG1or2DemuxedElementaryStream *) (((FramedFilter *) inputSource)->inputSource());
    MPEG1or2Demux& sourceDemux = elemStreamSource->sourceDemux();
    sourceDemux.flushInput();
    ByteStreamFileSource* inputFileSource =
        (ByteStreamFileSource *) (sourceDemux.inputSource());
    inputFileSource->seekToByteAbsolute(absBytePosition);
}

float MPEG1or2DemuxedServerMediaSubsession::duration() const
{
    return fOurDemux.fileDuration();
}
