#ifndef WEBRTC_VIDEO_ENCODER_H_
#define WEBRTC_VIDEO_ENCODER_H_
#include <memory>
#include <string>
#include <vector>
#include "webrtc/common_types.h"
#include "webrtc/typedefs.h"
#include "webrtc/video_frame.h"
namespace webrtc
{
class RTPFragmentationHeader;
struct CodecSpecificInfo;
struct VideoCodec;
class EncodedImageCallback
{
public:
    virtual ~EncodedImageCallback() {}
    virtual int32_t Encoded(const EncodedImage& encoded_image,
                            const CodecSpecificInfo* codec_specific_info,
                            const RTPFragmentationHeader* fragmentation) = 0;
};
class VideoEncoder
{
public:
    enum EncoderType {
        kH264,
        kVp8,
        kVp9,
        kUnsupportedCodec,
    };
    static VideoEncoder* Create(EncoderType codec_type);
    static VideoCodecVP8 GetDefaultVp8Settings();
    static VideoCodecVP9 GetDefaultVp9Settings();
    static VideoCodecH264 GetDefaultH264Settings();
    virtual ~VideoEncoder() {}
    virtual int32_t InitEncode(const VideoCodec* codec_settings,
                               int32_t number_of_cores,
                               size_t max_payload_size) = 0;
    virtual int32_t RegisterEncodeCompleteCallback(
        EncodedImageCallback* callback) = 0;
    virtual int32_t Release() = 0;
    virtual int32_t Encode(const VideoFrame& frame,
                           const CodecSpecificInfo* codec_specific_info,
                           const std::vector<FrameType>* frame_types) = 0;
    virtual int32_t SetChannelParameters(uint32_t packet_loss, int64_t rtt) = 0;
    virtual int32_t SetRates(uint32_t bitrate, uint32_t framerate) = 0;
    virtual int32_t SetPeriodicKeyFrames(bool enable)
    {
        return -1;
    }
    virtual void OnDroppedFrame() {}
    virtual bool SupportsNativeHandle() const
    {
        return false;
    }
    virtual const char* ImplementationName() const
    {
        return "unknown";
    }
};
class VideoEncoderSoftwareFallbackWrapper : public VideoEncoder
{
public:
    VideoEncoderSoftwareFallbackWrapper(VideoCodecType codec_type,
                                        webrtc::VideoEncoder* encoder);
    int32_t InitEncode(const VideoCodec* codec_settings,
                       int32_t number_of_cores,
                       size_t max_payload_size) override;
    int32_t RegisterEncodeCompleteCallback(
        EncodedImageCallback* callback) override;
    int32_t Release() override;
    int32_t Encode(const VideoFrame& frame,
                   const CodecSpecificInfo* codec_specific_info,
                   const std::vector<FrameType>* frame_types) override;
    int32_t SetChannelParameters(uint32_t packet_loss, int64_t rtt) override;
    int32_t SetRates(uint32_t bitrate, uint32_t framerate) override;
    void OnDroppedFrame() override;
    bool SupportsNativeHandle() const override;
    const char* ImplementationName() const override;
private:
    bool InitFallbackEncoder();
    VideoCodec codec_settings_;
    int32_t number_of_cores_;
    size_t max_payload_size_;
    bool rates_set_;
    uint32_t bitrate_;
    uint32_t framerate_;
    bool channel_parameters_set_;
    uint32_t packet_loss_;
    int64_t rtt_;
    const EncoderType encoder_type_;
    webrtc::VideoEncoder* const encoder_;
    std::unique_ptr<webrtc::VideoEncoder> fallback_encoder_;
    std::string fallback_implementation_name_;
    EncodedImageCallback* callback_;
};
}  
#endif  
