#include "RtmpGuesterImpl.h"

RTMPGuester * RTMPGuester::Create(RTMPGuesterEvent& callback)
{
    return new RtmpGuesterImpl(callback);
}

void RTMPGuester::Destory(RTMPGuester* guester)
{
    RtmpGuesterImpl* impl = (RtmpGuesterImpl *) guester->GotSelfPtr();

    delete impl;
    guester = NULL;
}

RtmpGuesterImpl::RtmpGuesterImpl(RTMPGuesterEvent& callback)
    : callback_(callback),
    worker_thread_(&webrtc::AnyRtmpCore::Inst()),
    av_rtmp_started_(NULL),
    av_rtmp_player_(NULL),
    video_render_(NULL)
{
    av_rtmp_player_ = AnyRtmplayer::Create(*this);
}

RtmpGuesterImpl::~RtmpGuesterImpl()
{
    StopRtmpPlay();
    if (av_rtmp_player_ != NULL) {
        av_rtmp_player_->StopPlay();
        delete av_rtmp_player_;
        av_rtmp_player_ = NULL;
    }
    if (video_render_ != NULL) {
        delete video_render_;
        video_render_ = NULL;
    }
}

void RtmpGuesterImpl::StartRtmpPlay(const char* url, void* render)
{
    if (!av_rtmp_started_) {
        rtmp_url_        = url;
        av_rtmp_started_ = true;
        video_render_    = webrtc::VideoRenderer::Create(render, 640, 480);
        av_rtmp_player_->SetVideoRender(video_render_);
        av_rtmp_player_->StartPlay(url);
        webrtc::AnyRtmpCore::Inst().StartAudioTrack(this);
    }
}

void RtmpGuesterImpl::StopRtmpPlay()
{
    if (av_rtmp_started_) {
        av_rtmp_started_ = false;
        rtmp_url_        = "";
        webrtc::AnyRtmpCore::Inst().StopAudioTrack();
        av_rtmp_player_->StopPlay();
        if (video_render_ != NULL) {
            delete video_render_;
            video_render_ = NULL;
        }
    }
}

void RtmpGuesterImpl::OnRtmplayerOK()
{
    callback_.OnRtmplayerOK();
}

void RtmpGuesterImpl::OnRtmplayerStatus(int cacheTime, int curBitrate)
{
    callback_.OnRtmplayerStatus(cacheTime, curBitrate);
}

void RtmpGuesterImpl::OnRtmplayerCache(int time)
{
    callback_.OnRtmplayerCache(time);
}

void RtmpGuesterImpl::OnRtmplayerClose(int errcode)
{
    callback_.OnRtmplayerClosed(errcode);
}

int RtmpGuesterImpl::OnNeedPlayAudio(void* audioSamples, uint32_t& samplesPerSec, size_t& nChannels)
{
    return ((webrtc::AnyRtmplayerImpl *) av_rtmp_player_)->GetNeedPlayAudio(audioSamples, samplesPerSec, nChannels);
}
