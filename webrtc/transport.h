#ifndef WEBRTC_TRANSPORT_H_
#define WEBRTC_TRANSPORT_H_
#include <stddef.h>
#include "webrtc/typedefs.h"
namespace webrtc
{
struct PacketOptions {
    int packet_id = -1;
};
class Transport
{
public:
    virtual bool SendRtp(const uint8_t* packet,
                         size_t length,
                         const PacketOptions& options) = 0;
    virtual bool SendRtcp(const uint8_t* packet, size_t length) = 0;
protected:
    virtual ~Transport() {}
};
}  
#endif  
