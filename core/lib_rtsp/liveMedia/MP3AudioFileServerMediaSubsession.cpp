#include "MP3AudioFileServerMediaSubsession.hh"
#include "MPEG1or2AudioRTPSink.hh"
#include "MP3ADURTPSink.hh"
#include "MP3FileSource.hh"
#include "MP3ADU.hh"
MP3AudioFileServerMediaSubsession * MP3AudioFileServerMediaSubsession
::createNew(UsageEnvironment& env, char const* fileName, Boolean reuseFirstSource,
            Boolean generateADUs, Interleaving* interleaving)
{
    return new MP3AudioFileServerMediaSubsession(env, fileName, reuseFirstSource,
                                                 generateADUs, interleaving);
}

MP3AudioFileServerMediaSubsession
::MP3AudioFileServerMediaSubsession(UsageEnvironment& env,
                                    char const* fileName, Boolean reuseFirstSource,
                                    Boolean generateADUs,
                                    Interleaving* interleaving)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource),
    fGenerateADUs(generateADUs), fInterleaving(interleaving), fFileDuration(0.0)
{}

MP3AudioFileServerMediaSubsession
::~MP3AudioFileServerMediaSubsession()
{
    delete fInterleaving;
}

FramedSource * MP3AudioFileServerMediaSubsession
::createNewStreamSourceCommon(FramedSource* baseMP3Source, unsigned mp3NumBytes, unsigned& estBitrate)
{
    FramedSource* streamSource;

    do {
        streamSource = baseMP3Source;
        if (streamSource == NULL) {
            break;
        }
        if (mp3NumBytes > 0 && fFileDuration > 0.0) {
            estBitrate = (unsigned) (mp3NumBytes / (125 * fFileDuration) + 0.5);
        } else {
            estBitrate = 128;
        }
        if (fGenerateADUs) {
            streamSource = ADUFromMP3Source::createNew(envir(), streamSource);
            if (streamSource == NULL) {
                break;
            }
            if (fInterleaving != NULL) {
                streamSource = MP3ADUinterleaver::createNew(envir(), *fInterleaving,
                                                            streamSource);
                if (streamSource == NULL) {
                    break;
                }
            }
        } else if (fFileDuration > 0.0) {
            streamSource = ADUFromMP3Source::createNew(envir(), streamSource);
            if (streamSource == NULL) {
                break;
            }
            streamSource = MP3FromADUSource::createNew(envir(), streamSource);
            if (streamSource == NULL) {
                break;
            }
        }
    } while (0);
    return streamSource;
} // MP3AudioFileServerMediaSubsession::createNewStreamSourceCommon

void MP3AudioFileServerMediaSubsession::getBaseStreams(FramedSource* frontStream,
                                                       FramedSource *& sourceMP3Stream, ADUFromMP3Source *& aduStream)
{
    if (fGenerateADUs) {
        if (fInterleaving != NULL) {
            aduStream = (ADUFromMP3Source *) (((FramedFilter *) frontStream)->inputSource());
        } else {
            aduStream = (ADUFromMP3Source *) frontStream;
        }
        sourceMP3Stream = (MP3FileSource *) (aduStream->inputSource());
    } else if (fFileDuration > 0.0) {
        aduStream       = (ADUFromMP3Source *) (((FramedFilter *) frontStream)->inputSource());
        sourceMP3Stream = (MP3FileSource *) (aduStream->inputSource());
    } else {
        aduStream       = NULL;
        sourceMP3Stream = frontStream;
    }
}

void MP3AudioFileServerMediaSubsession
::seekStreamSource(FramedSource* inputSource, double& seekNPT, double streamDuration, u_int64_t&)
{
    FramedSource* sourceMP3Stream;
    ADUFromMP3Source* aduStream;

    getBaseStreams(inputSource, sourceMP3Stream, aduStream);
    if (aduStream != NULL) {
        aduStream->resetInput();
    }
    ((MP3FileSource *) sourceMP3Stream)->seekWithinFile(seekNPT, streamDuration);
}

void MP3AudioFileServerMediaSubsession
::setStreamSourceScale(FramedSource* inputSource, float scale)
{
    FramedSource* sourceMP3Stream;
    ADUFromMP3Source* aduStream;

    getBaseStreams(inputSource, sourceMP3Stream, aduStream);
    if (aduStream == NULL) {
        return;
    }
    int iScale = (int) scale;
    aduStream->setScaleFactor(iScale);
    ((MP3FileSource *) sourceMP3Stream)->setPresentationTimeScale(iScale);
}

FramedSource * MP3AudioFileServerMediaSubsession
::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    MP3FileSource* mp3Source = MP3FileSource::createNew(envir(), fFileName);

    if (mp3Source == NULL) {
        return NULL;
    }
    fFileDuration = mp3Source->filePlayTime();
    return createNewStreamSourceCommon(mp3Source, mp3Source->fileSize(), estBitrate);
}

RTPSink * MP3AudioFileServerMediaSubsession
::createNewRTPSink(Groupsock*    rtpGroupsock,
                   unsigned char rtpPayloadTypeIfDynamic,
                   FramedSource *)
{
    if (fGenerateADUs) {
        return MP3ADURTPSink::createNew(envir(), rtpGroupsock,
                                        rtpPayloadTypeIfDynamic);
    } else {
        return MPEG1or2AudioRTPSink::createNew(envir(), rtpGroupsock);
    }
}

void MP3AudioFileServerMediaSubsession::testScaleFactor(float& scale)
{
    if (fFileDuration <= 0.0) {
        scale = 1;
    } else {
        int iScale = (int) (scale + 0.5);
        if (iScale < 1) {
            iScale = 1;
        }
        scale = (float) iScale;
    }
}

float MP3AudioFileServerMediaSubsession::duration() const
{
    return fFileDuration;
}
