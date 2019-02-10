#ifndef __VIDEO_FILTER_H__
#define __VIDEO_FILTER_H__
#include "webrtc/video_frame.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/media/base/videoframe.h"
#include "webrtc/media/base/videobroadcaster.h"
#include "webrtc/media/engine/webrtcvideoframe.h"
class VideoFilter : public rtc::VideoSinkInterface<cricket::VideoFrame>
{
public:
    VideoFilter();
    virtual ~VideoFilter();
    rtc::VideoBroadcaster& VBroadcaster()
    {
        return broadcaster_;
    };
    virtual void OnFrame(const cricket::VideoFrame& frame);
private:
    int v_width_;
    int v_height_;
    rtc::VideoBroadcaster broadcaster_;
    cricket::WebRtcVideoFrame video_frame_;
};
#endif // ifndef __VIDEO_FILTER_H__
