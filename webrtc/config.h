#ifndef WEBRTC_CONFIG_H_
#define WEBRTC_CONFIG_H_
#include <string>
#include <vector>
#include "webrtc/common.h"
#include "webrtc/common_types.h"
#include "webrtc/typedefs.h"
namespace webrtc
{
struct NackConfig {
    NackConfig() : rtp_history_ms(0) {}
    std::string ToString() const;
    int rtp_history_ms;
};
struct FecConfig {
    FecConfig()
        : ulpfec_payload_type(-1),
          red_payload_type(-1),
          red_rtx_payload_type(-1) {}
    std::string ToString() const;
    int ulpfec_payload_type;
    int red_payload_type;
    int red_rtx_payload_type;
};
struct RtpExtension {
    RtpExtension() : id(0) {}
    RtpExtension(const std::string& uri, int id) : uri(uri), id(id) {}
    std::string ToString() const;
    bool operator==(const RtpExtension& rhs) const
    {
        return uri == rhs.uri && id == rhs.id;
    }
    static bool IsSupportedForAudio(const std::string& uri);
    static bool IsSupportedForVideo(const std::string& uri);
    static const char* kAudioLevelUri;
    static const int kAudioLevelDefaultId;
    static const char* kTimestampOffsetUri;
    static const int kTimestampOffsetDefaultId;
    static const char* kAbsSendTimeUri;
    static const int kAbsSendTimeDefaultId;
    static const char* kVideoRotationUri;
    static const int kVideoRotationDefaultId;
    static const char* kTransportSequenceNumberUri;
    static const int kTransportSequenceNumberDefaultId;
    static const char* kPlayoutDelayUri;
    static const int kPlayoutDelayDefaultId;
    std::string uri;
    int id;
};
struct VideoStream {
    VideoStream();
    ~VideoStream();
    std::string ToString() const;
    size_t width;
    size_t height;
    int max_framerate;
    int min_bitrate_bps;
    int target_bitrate_bps;
    int max_bitrate_bps;
    int max_qp;
    std::vector<int> temporal_layer_thresholds_bps;
};
struct VideoEncoderConfig {
    enum class ContentType {
        kRealtimeVideo,
        kScreen,
    };
    VideoEncoderConfig();
    ~VideoEncoderConfig();
    std::string ToString() const;
    std::vector<VideoStream> streams;
    std::vector<SpatialLayer> spatial_layers;
    ContentType content_type;
    void* encoder_specific_settings;
    int min_transmit_bitrate_bps;
    bool expect_encode_from_texture;
};
struct NetEqCapacityConfig {
    NetEqCapacityConfig() : enabled(false), capacity(0) {}
    explicit NetEqCapacityConfig(int value) : enabled(true), capacity(value) {}
    static const ConfigOptionID identifier = ConfigOptionID::kNetEqCapacityConfig;
    bool enabled;
    int capacity;
};
struct NetEqFastAccelerate {
    NetEqFastAccelerate() : enabled(false) {}
    explicit NetEqFastAccelerate(bool value) : enabled(value) {}
    static const ConfigOptionID identifier = ConfigOptionID::kNetEqFastAccelerate;
    bool enabled;
};
struct VoicePacing {
    VoicePacing() : enabled(false) {}
    explicit VoicePacing(bool value) : enabled(value) {}
    static const ConfigOptionID identifier = ConfigOptionID::kVoicePacing;
    bool enabled;
};
}  
#endif  
