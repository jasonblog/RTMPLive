#ifndef WEBRTC_TEST_VIDEO_RENDERER_H_
#define WEBRTC_TEST_VIDEO_RENDERER_H_
#include <stddef.h>
#include "webrtc/media/base/videosinkinterface.h"
#include "webrtc/media/base/videoframe.h"
namespace webrtc
{
class VideoRenderer : public rtc::VideoSinkInterface<cricket::VideoFrame>
{
public:
    static VideoRenderer * Create(const void* hwnd, size_t width,
                                  size_t height);
    static VideoRenderer * CreatePlatformRenderer(const void* hwnd,
                                                  size_t width, size_t height);
    virtual ~VideoRenderer() {}

protected:
    VideoRenderer() {}
};
}
#endif // ifndef WEBRTC_TEST_VIDEO_RENDERER_H_
