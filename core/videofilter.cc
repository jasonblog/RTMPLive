#include "videofilter.h"
static uint32_t sFpsCt = 0;
VideoFilter::VideoFilter()
    : v_width_(640),
    v_height_(480)
{}

VideoFilter::~VideoFilter()
{}

void VideoFilter::OnFrame(const cricket::VideoFrame& frame)
{
    #ifdef ANDROID
    if (sFpsCt++ % 3 == 0) {
        return;
    }
    #endif
    int dst_width_  = frame.video_frame_buffer()->width();
    int dst_height_ = frame.video_frame_buffer()->height();
    if (dst_width_ < dst_height_) {
        dst_width_ = dst_height_ * 9 / 16;
    }
    video_frame_.InitToEmptyBuffer(dst_width_, dst_height_);
    video_frame_.SetTimeStamp(rtc::TimeMillis());
    {
        int offsetW     = abs((int) (dst_width_ - frame.video_frame_buffer()->width())) / 2;
        int offsetH     = abs((int) (dst_height_ - frame.video_frame_buffer()->height())) / 2;
        uint8_t* buf0_y = (uint8_t *) frame.video_frame_buffer()->DataY()
                          + (frame.video_frame_buffer()->width() * offsetH);
        uint8_t* buf0_u = (uint8_t *) frame.video_frame_buffer()->DataU()
                          + (frame.video_frame_buffer()->width() * offsetH) / 4;
        uint8_t* buf0_v = (uint8_t *) frame.video_frame_buffer()->DataV()
                          + (frame.video_frame_buffer()->width() * offsetH) / 4;
        uint8_t* buf1_y = (uint8_t *) video_frame_.video_frame_buffer()->DataY();
        uint8_t* buf1_u = (uint8_t *) video_frame_.video_frame_buffer()->DataU();
        uint8_t* buf1_v = (uint8_t *) video_frame_.video_frame_buffer()->DataV();
        for (int i = 0; i < dst_height_; i++) {
            memcpy(buf1_y, buf0_y + offsetW, dst_width_);
            buf0_y += frame.video_frame_buffer()->width();
            buf1_y += dst_width_;
        }
        for (int i = 0; i < dst_height_ / 2; i++) {
            memcpy(buf1_u, buf0_u + offsetW / 2, dst_width_ / 2);
            memcpy(buf1_v, buf0_v + offsetW / 2, dst_width_ / 2);
            buf0_u += frame.video_frame_buffer()->width() / 2;
            buf0_v += frame.video_frame_buffer()->width() / 2;
            buf1_u += dst_width_ / 2;
            buf1_v += dst_width_ / 2;
        }
    }
    broadcaster_.OnFrame(video_frame_);
} // VideoFilter::OnFrame
