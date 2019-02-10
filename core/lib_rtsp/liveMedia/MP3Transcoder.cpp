#include "MP3Transcoder.hh"
MP3Transcoder::MP3Transcoder(UsageEnvironment  & env,
                             MP3ADUTranscoder* aduTranscoder)
    : MP3FromADUSource(env, aduTranscoder, False)
{}

MP3Transcoder::~MP3Transcoder()
{}

MP3Transcoder * MP3Transcoder::createNew(UsageEnvironment& env,
                                         unsigned        outBitrate,
                                         FramedSource*   inputSource)
{
    MP3Transcoder* newSource = NULL;

    do {
        ADUFromMP3Source* aduFromMP3 =
            ADUFromMP3Source::createNew(env, inputSource, False);
        if (aduFromMP3 == NULL) {
            break;
        }
        MP3ADUTranscoder* aduTranscoder =
            MP3ADUTranscoder::createNew(env, outBitrate, aduFromMP3);
        if (aduTranscoder == NULL) {
            break;
        }
        newSource = new MP3Transcoder(env, aduTranscoder);
    } while (0);
    return newSource;
}
