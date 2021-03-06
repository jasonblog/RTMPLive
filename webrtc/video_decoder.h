#ifndef WEBRTC_VIDEO_DECODER_H_
#define WEBRTC_VIDEO_DECODER_H_
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
class DecodedImageCallback
{
public:
    virtual ~DecodedImageCallback() {}
    virtual int32_t Decoded(VideoFrame& decodedImage) = 0;
    virtual int32_t Decoded(VideoFrame& decodedImage, int64_t decode_time_ms)
    {
        return Decoded(decodedImage);
    }
    virtual int32_t ReceivedDecodedReferenceFrame(const uint64_t pictureId)
    {
        return -1;
    }
    virtual int32_t ReceivedDecodedFrame(const uint64_t pictureId)
    {
        return -1;
    }
};
class VideoDecoder
{
public:
    enum DecoderType {
        kH264,
        kVp8,
        kVp9,
        kUnsupportedCodec,
    };
    static VideoDecoder* Create(DecoderType codec_type);
    virtual ~VideoDecoder() {}
    virtual int32_t InitDecode(const VideoCodec* codec_settings,
                               int32_t number_of_cores) = 0;
    virtual int32_t Decode(const EncodedImage& input_image,
                           bool missing_frames,
                           const RTPFragmentationHeader* fragmentation,
                           const CodecSpecificInfo* codec_specific_info = NULL,
                           int64_t render_time_ms = -1) = 0;
    virtual int32_t RegisterDecodeCompleteCallback(
        DecodedImageCallback* callback) = 0;
    virtual int32_t Release() = 0;
    virtual bool PrefersLateDecoding() const
    {
        return true;
    }
    virtual const char* ImplementationName() const
    {
        return "unknown";
    }
};
class VideoDecoderSoftwareFallbackWrapper : public webrtc::VideoDecoder
{
public:
    VideoDecoderSoftwareFallbackWrapper(VideoCodecType codec_type,
                                        VideoDecoder* decoder);
    int32_t InitDecode(const VideoCodec* codec_settings,
                       int32_t number_of_cores) override;
    int32_t Decode(const EncodedImage& input_image,
                   bool missing_frames,
                   const RTPFragmentationHeader* fragmentation,
                   const CodecSpecificInfo* codec_specific_info,
                   int64_t render_time_ms) override;
    int32_t RegisterDecodeCompleteCallback(
        DecodedImageCallback* callback) override;
    int32_t Release() override;
    bool PrefersLateDecoding() const override;
    const char* ImplementationName() const override;
private:
    bool InitFallbackDecoder();
    const DecoderType decoder_type_;
    VideoDecoder* const decoder_;
    VideoCodec codec_settings_;
    int32_t number_of_cores_;
    std::string fallback_implementation_name_;
    std::unique_ptr<VideoDecoder> fallback_decoder_;
    DecodedImageCallback* callback_;
};
class NullVideoDecoder : public VideoDecoder
{
public:
    NullVideoDecoder();
    int32_t InitDecode(const VideoCodec* codec_settings,
                       int32_t number_of_cores) override;
    int32_t Decode(const EncodedImage& input_image,
                   bool missing_frames,
                   const RTPFragmentationHeader* fragmentation,
                   const CodecSpecificInfo* codec_specific_info,
                   int64_t render_time_ms) override;
    int32_t RegisterDecodeCompleteCallback(
        DecodedImageCallback* callback) override;
    int32_t Release() override;
    const char* ImplementationName() const override;
};
}  
#endif  
