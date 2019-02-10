#ifndef WEBRTC_VIDEO_FRAME_H_
#define WEBRTC_VIDEO_FRAME_H_
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/common_types.h"
#include "webrtc/common_video/include/video_frame_buffer.h"
#include "webrtc/common_video/rotation.h"
#include "webrtc/typedefs.h"
namespace webrtc
{
class VideoFrame
{
public:
    VideoFrame();
    VideoFrame(const rtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer,
               uint32_t timestamp,
               int64_t render_time_ms,
               VideoRotation rotation);
    void CreateEmptyFrame(int width,
                          int height,
                          int stride_y,
                          int stride_u,
                          int stride_v);
    void CreateFrame(const uint8_t* buffer_y,
                     const uint8_t* buffer_u,
                     const uint8_t* buffer_v,
                     int width,
                     int height,
                     int stride_y,
                     int stride_u,
                     int stride_v,
                     VideoRotation rotation);
    void CreateFrame(const uint8_t* buffer,
                     int width,
                     int height,
                     VideoRotation rotation);
    void CopyFrame(const VideoFrame& videoFrame);
    void ShallowCopy(const VideoFrame& videoFrame);
    int allocated_size(PlaneType type) const;
    int width() const;
    int height() const;
    void set_timestamp(uint32_t timestamp)
    {
        timestamp_ = timestamp;
    }
    uint32_t timestamp() const
    {
        return timestamp_;
    }
    void set_ntp_time_ms(int64_t ntp_time_ms)
    {
        ntp_time_ms_ = ntp_time_ms;
    }
    int64_t ntp_time_ms() const
    {
        return ntp_time_ms_;
    }
    VideoRotation rotation() const
    {
        return rotation_;
    }
    void set_rotation(VideoRotation rotation)
    {
        rotation_ = rotation;
    }
    void set_render_time_ms(int64_t render_time_ms)
    {
        render_time_ms_ = render_time_ms;
    }
    int64_t render_time_ms() const
    {
        return render_time_ms_;
    }
    bool IsZeroSize() const;
    const rtc::scoped_refptr<webrtc::VideoFrameBuffer>& video_frame_buffer()
    const;
    bool is_texture()
    {
        return video_frame_buffer() &&
               video_frame_buffer()->native_handle() != nullptr;
    }
private:
    rtc::scoped_refptr<webrtc::VideoFrameBuffer> video_frame_buffer_;
    uint32_t timestamp_;
    int64_t ntp_time_ms_;
    int64_t render_time_ms_;
    VideoRotation rotation_;
};
class EncodedImage
{
public:
    static const size_t kBufferPaddingBytesH264;
    static size_t GetBufferPaddingBytes(VideoCodecType codec_type);
    EncodedImage() : EncodedImage(nullptr, 0, 0) {}
    EncodedImage(uint8_t* buffer, size_t length, size_t size)
        : _buffer(buffer), _length(length), _size(size) {}
    struct AdaptReason {
        AdaptReason()
            : quality_resolution_downscales(-1),
              bw_resolutions_disabled(-1) {}
        int quality_resolution_downscales;  
        int bw_resolutions_disabled;  
    };
    uint32_t _encodedWidth = 0;
    uint32_t _encodedHeight = 0;
    uint32_t _timeStamp = 0;
    int64_t ntp_time_ms_ = 0;
    int64_t capture_time_ms_ = 0;
    FrameType _frameType = kVideoFrameDelta;
    uint8_t* _buffer;
    size_t _length;
    size_t _size;
    VideoRotation rotation_ = kVideoRotation_0;
    bool _completeFrame = false;
    AdaptReason adapt_reason_;
    int qp_ = -1;  
    PlayoutDelay playout_delay_ = {-1, -1};
};
}  
#endif  
