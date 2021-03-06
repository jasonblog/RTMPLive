#ifndef WEBRTC_AUDIO_SINK_H_
#define WEBRTC_AUDIO_SINK_H_
#if defined(WEBRTC_POSIX) && !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <stddef.h>
namespace webrtc
{
class AudioSinkInterface
{
public:
    virtual ~AudioSinkInterface() {}
    struct Data {
        Data(int16_t* data,
             size_t samples_per_channel,
             int sample_rate,
             size_t channels,
             uint32_t timestamp)
            : data(data),
              samples_per_channel(samples_per_channel),
              sample_rate(sample_rate),
              channels(channels),
              timestamp(timestamp) {}
        int16_t* data;               
        size_t samples_per_channel;  
        int sample_rate;             
        size_t channels;             
        uint32_t timestamp;          
    };
    virtual void OnData(const Data& audio) = 0;
};
}  
#endif  
