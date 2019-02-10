#ifndef __ANY_RTMP_PLAYER_H__
#define __ANY_RTMP_PLAYER_H__
#include "anyrtmplayer_interface.h"
#include "anyrtmpull.h"
#include "plydecoder.h"
#include "webrtc/base/messagehandler.h"
#include "webrtc/api/mediastreaminterface.h"
namespace webrtc
{
class AnyRtmplayerImpl : public AnyRtmplayer, public rtc::Thread, public rtc::MessageHandler
    , public AnyRtmpPullCallback
{
public:
    AnyRtmplayerImpl(AnyRtmplayerEvent& callback);
    virtual ~AnyRtmplayerImpl(void);
    virtual void StartPlay(const char* url);
    virtual void SetVideoRender(void* handle);
    virtual void StopPlay();
    int GetNeedPlayAudio(void* audioSamples, uint32_t& samplesPerSec, size_t& nChannels);
protected:
    virtual void OnMessage(rtc::Message* msg);
    virtual void OnRtmpullConnected();
    virtual void OnRtmpullFailed();
    virtual void OnRtmpullDisconnect();
    virtual void OnRtmpullH264Data(const uint8_t* pdata, int len, uint32_t ts);
    virtual void OnRtmpullAACData(const uint8_t* pdata, int len, uint32_t ts);
private:
    AnyRtmpPull* rtmp_pull_;
    PlyDecoder* ply_decoder_;
    int cur_bitrate_;
    std::string str_url_;
    rtc::VideoSinkInterface<cricket::VideoFrame>* video_renderer_;
};
}
#endif // ifndef __ANY_RTMP_PLAYER_H__
