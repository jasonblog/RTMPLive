#include "video_renderer.h"


#include "webrtc/typedefs.h"
#include "webrtc/media/base/videosinkinterface.h"
#include "webrtc/media/engine/webrtcvideoframe.h"

namespace webrtc
{
class NullRenderer : public VideoRenderer
{
    void OnFrame(const cricket::VideoFrame& video_frame) override {}
};

class AndroidRender : public VideoRenderer
{
public:
    AndroidRender(rtc::VideoSinkInterface<cricket::VideoFrame>* render)
    {
        video_renderer_ = render;
    };
    virtual ~AndroidRender(void) {};

    void OnFrame(const cricket::VideoFrame& nativeVideoFrame) override
    {
        if (video_renderer_ != NULL) {
            video_renderer_->OnFrame(nativeVideoFrame);
        }
    }

public:
    bool Init(size_t width, size_t height)
    {
        return true;
    };
    void Resize(size_t width, size_t height) {};
    void Destroy() {};

private:
    rtc::VideoSinkInterface<cricket::VideoFrame>* video_renderer_;
};

VideoRenderer * VideoRenderer::CreatePlatformRenderer(const void* hwnd,
                                                      size_t      width,
                                                      size_t      height)
{
    AndroidRender* render = new AndroidRender((rtc::VideoSinkInterface<cricket::VideoFrame> *)hwnd);

    render->Init(width, height);
    return render;
}

VideoRenderer * VideoRenderer::Create(const void* hwnd,
                                      size_t      width,
                                      size_t      height)
{
    VideoRenderer* renderer = CreatePlatformRenderer(hwnd, width, height);

    if (renderer != NULL) {
        return renderer;
    }

    return new NullRenderer();
}
}
