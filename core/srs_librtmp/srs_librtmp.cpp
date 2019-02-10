#include "srs_librtmp.h"
#include "srs_kernel_codec.h"
#include "srs_lib_simple_socket.hpp"
#ifndef SRS_AUTO_HEADER_HPP
# define SRS_AUTO_HEADER_HPP
# define SRS_AUTO_BUILD_TS       "1471494191"
# define SRS_AUTO_BUILD_DATE     "2016-08-18 12:23:11"
# define SRS_AUTO_UNAME \
    "Darwin winlin.lan 15.6.0 Darwin Kernel Version 15.6.0: Thu Jun 23 18:25:34 PDT 2016; root:xnu-3248.60.10~1/RELEASE_X86_64 x86_64"
# define SRS_AUTO_USER_CONFIGURE "--x86-x64  --export-librtmp-single=/Users/winlin/git/srs-librtmp/src/srs"
# define SRS_AUTO_CONFIGURE \
    "--prefix=/usr/local/srs --without-hls --without-hds --without-dvr --without-nginx --without-ssl --without-ffmpeg --without-transcode --without-ingest --without-stat --without-http-callback --without-http-server --without-stream-caster --without-http-api --with-librtmp --with-research --without-utest --without-gperf --without-gmc --without-gmp --without-gcp --without-gprof --without-arm-ubuntu12 --without-mips-ubuntu12 --log-trace"
# define SRS_X86_X64
# define SRS_EXPORT_LIBRTMP
# define SRS_AUTO_EMBEDED_TOOL_CHAIN "normal x86/x64 gcc"
# undef SRS_AUTO_HTTP_CORE
# undef SRS_AUTO_HTTP_SERVER
# undef SRS_AUTO_STREAM_CASTER
# undef SRS_AUTO_HTTP_API
# undef SRS_AUTO_NGINX
# undef SRS_AUTO_DVR
# undef SRS_AUTO_HLS
# undef SRS_AUTO_HDS
# undef SRS_AUTO_HTTP_CALLBACK
# undef SRS_AUTO_SSL
# undef SRS_AUTO_MEM_WATCH
# undef SRS_AUTO_FFMPEG_TOOL
# define SRS_AUTO_FFMPEG_STUB
# undef SRS_AUTO_TRANSCODE
# undef SRS_AUTO_INGEST
# undef SRS_AUTO_STAT
# undef SRS_AUTO_GPERF
# undef SRS_AUTO_GPERF_MC
# undef SRS_AUTO_GPERF_MP
# undef SRS_AUTO_GPERF_CP
# undef SRS_AUTO_EMBEDED_CPU
# undef SRS_AUTO_ARM_UBUNTU12
# undef SRS_AUTO_MIPS_UBUNTU12
# undef SRS_AUTO_VERBOSE
# undef SRS_AUTO_INFO
# define SRS_AUTO_TRACE
# define SRS_AUTO_PREFIX "/usr/local/srs"
# define SRS_AUTO_CONSTRIBUTORS \
    "\
winlin<winlin@vip.126.com> \
wenjie.zhao<740936897@qq.com> \
xiangcheng.liu<liuxc0116@foxmail.com> \
naijia.liu<youngcow@youngcow.net> \
alcoholyi<alcoholyi@qq.com> \
byteman<wangchen2011@gmail.com> \
chad.wang<chad.wang.cn@gmail.com> \
suhetao<suhetao@gmail.com> \
Johnny<fengjihu@163.com> \
karthikeyan<keyanmca@gmail.com> \
StevenLiu<lq@chinaffmpeg.org> \
zhengfl<zhengfl_1989@126.com> \
tufang14<breadbean1449@gmail.com> \
allspace<allspace@gmail.com> \
niesongsong<nie950@gmail.com> \
rudeb0t<nimrod@themanxgroup.tw> \
CallMeNP<np.liamg@gmail.com> \
synote<synote@qq.com> \
lovecat<littlefawn@163.com> \
panda1986<542638787@qq.com> \
YueHonghui<hongf.yue@hotmail.com> \
"
#endif // ifndef SRS_AUTO_HEADER_HPP
#ifndef SRS_CORE_HPP
# define SRS_CORE_HPP
# define VERSION_MAJOR                 2
# define VERSION_MINOR                 0
# define VERSION_REVISION              210
# define RTMP_SIG_SRS_KEY              "SRS"
# define RTMP_SIG_SRS_CODE             "ZhouGuowen"
# define RTMP_SIG_SRS_AUTHROS          "winlin,wenjie.zhao"
# define RTMP_SIG_SRS_WEB              "http://ossrs.net"
# define RTMP_SIG_SRS_EMAIL            "winlin@vip.126.com"
# define RTMP_SIG_SRS_ROLE             "cluster"
# define RTMP_SIG_SRS_NAME             RTMP_SIG_SRS_KEY "(Simple RTMP Server)"
# define RTMP_SIG_SRS_URL_SHORT        "github.com/ossrs/srs"
# define RTMP_SIG_SRS_URL              "https://" RTMP_SIG_SRS_URL_SHORT
# define RTMP_SIG_SRS_LICENSE          "The MIT License (MIT)"
# define RTMP_SIG_SRS_COPYRIGHT        "Copyright (c) 2013-2015 SRS(ossrs)"
# define RTMP_SIG_SRS_PRIMARY          RTMP_SIG_SRS_KEY "/" VERSION_STABLE_BRANCH
# define RTMP_SIG_SRS_CONTRIBUTORS_URL RTMP_SIG_SRS_URL "/blob/master/AUTHORS.txt"
# define RTMP_SIG_SRS_HANDSHAKE        RTMP_SIG_SRS_KEY "(" RTMP_SIG_SRS_VERSION ")"
# define RTMP_SIG_SRS_RELEASE          RTMP_SIG_SRS_URL "/tree/" VERSION_STABLE_BRANCH ".0release"
# define RTMP_SIG_SRS_ISSUES(id) RTMP_SIG_SRS_URL "/issues/"#id
# define RTMP_SIG_SRS_VERSION          SRS_XSTR(VERSION_MAJOR) "." SRS_XSTR(VERSION_MINOR) "." SRS_XSTR(VERSION_REVISION)
# define RTMP_SIG_SRS_SERVER           RTMP_SIG_SRS_KEY "/" RTMP_SIG_SRS_VERSION "(" RTMP_SIG_SRS_CODE ")"
# define VERSION_STABLE                1
# define VERSION_STABLE_BRANCH         SRS_XSTR(VERSION_STABLE) ".0release"
# define SRS_XSTR(v)         SRS_INTERNAL_STR(v)
# define SRS_INTERNAL_STR(v) #v
# ifndef _FILE_OFFSET_BITS
#  define _FILE_OFFSET_BITS 64
# endif
# ifndef __STDC_FORMAT_MACROS
#  define __STDC_FORMAT_MACROS
# endif
# ifndef _WIN32
#  include <inttypes.h>
# endif
# include <assert.h>
# define srs_assert(expression) assert(expression)
# include <stddef.h>
# include <sys/types.h>
# define srs_freep(p) \
    if (p) { \
        delete p; \
        p = NULL; \
    } \
    (void) 0
# define srs_freepa(pa) \
    if (pa) { \
        delete[] pa; \
        pa = NULL; \
    } \
    (void) 0
# define disable_default_copy(className) \
private: \
     \
    className(const className&); \
    className& operator = (const className&)
# if !defined(__amd64__) && !defined(__x86_64__) && !defined(__i386__) && !defined(__arm__)
# endif
# if defined(__arm__) && (__GLIBC__ != 2 || __GLIBC_MINOR__ > 15)
# endif
#endif // ifndef SRS_CORE_HPP
#ifndef SRS_CORE_AUTO_FREE_HPP
# define SRS_CORE_AUTO_FREE_HPP
# define SrsAutoFree(className, instance) \
    impl__SrsAutoFree<className> _auto_free_ ## instance(&instance, false)
# define SrsAutoFreeA(className, instance) \
    impl__SrsAutoFree<className> _auto_free_array_ ## instance(&instance, true)
template <class T>
class impl__SrsAutoFree
{
private:
    T** ptr;
    bool is_array;
public:
    impl__SrsAutoFree(T** p, bool array)
    {
        ptr      = p;
        is_array = array;
    }

    virtual ~impl__SrsAutoFree()
    {
        if (ptr == NULL || *ptr == NULL) {
            return;
        }
        if (is_array) {
            delete[] *ptr;
        } else {
            delete *ptr;
        }
        *ptr = NULL;
    }
};
#endif // ifndef SRS_CORE_AUTO_FREE_HPP
#ifndef SRS_CORE_PERFORMANCE_HPP
# define SRS_CORE_PERFORMANCE_HPP
# define SRS_PERF_MERGED_READ
# define SRS_PERF_MR_ENABLED false
# define SRS_PERF_MR_SLEEP   350
# define SRS_PERF_MW_SLEEP   350
# define SRS_PERF_MW_MSGS    128
# define SRS_PERF_MW_SO_SNDBUF
# undef SRS_PERF_MW_SO_RCVBUF
# define SRS_PERF_QUEUE_FAST_VECTOR
# define SRS_PERF_QUEUE_COND_WAIT
# ifdef SRS_PERF_QUEUE_COND_WAIT
#  define SRS_PERF_MW_MIN_MSGS        8
# endif
# define SRS_PERF_MIN_LATENCY_ENABLED false
# define SRS_PERF_CHUNK_STREAM_CACHE  16
# define SRS_PERF_GOP_CACHE           true
# define SRS_PERF_PLAY_QUEUE          30
# define SRS_PERF_COMPLEX_SEND
# undef SRS_PERF_TCP_NODELAY
# define SRS_PERF_TCP_NODELAY
# ifdef SRS_PERF_MW_SO_SNDBUF
#  undef SRS_PERF_SO_SNDBUF_SIZE
# endif
# undef SRS_PERF_FAST_FLV_ENCODER
# define SRS_PERF_FAST_FLV_ENCODER
#endif // ifndef SRS_CORE_PERFORMANCE_HPP
#ifndef SRS_CORE_MEM_WATCH_HPP
# define SRS_CORE_MEM_WATCH_HPP
# ifdef SRS_AUTO_MEM_WATCH
#  include <string>
extern void srs_memory_watch(void* ptr, std::string category, int size);
extern void srs_memory_unwatch(void* ptr);
extern void srs_memory_report();
# endif
#endif // ifndef SRS_CORE_MEM_WATCH_HPP
#ifndef SRS_KERNEL_ERROR_HPP
# define SRS_KERNEL_ERROR_HPP
# ifndef _WIN32
#  define ERROR_SUCCESS                   0
# endif
# define ERROR_SOCKET_CREATE              1000
# define ERROR_SOCKET_SETREUSE            1001
# define ERROR_SOCKET_BIND                1002
# define ERROR_SOCKET_LISTEN              1003
# define ERROR_SOCKET_CLOSED              1004
# define ERROR_SOCKET_GET_PEER_NAME       1005
# define ERROR_SOCKET_GET_PEER_IP         1006
# define ERROR_SOCKET_READ                1007
# define ERROR_SOCKET_READ_FULLY          1008
# define ERROR_SOCKET_WRITE               1009
# define ERROR_SOCKET_WAIT                1010
# define ERROR_SOCKET_TIMEOUT             1011
# define ERROR_SOCKET_CONNECT             1012
# define ERROR_ST_SET_EPOLL               1013
# define ERROR_ST_INITIALIZE              1014
# define ERROR_ST_OPEN_SOCKET             1015
# define ERROR_ST_CREATE_LISTEN_THREAD    1016
# define ERROR_ST_CREATE_CYCLE_THREAD     1017
# define ERROR_ST_CONNECT                 1018
# define ERROR_SYSTEM_PACKET_INVALID      1019
# define ERROR_SYSTEM_CLIENT_INVALID      1020
# define ERROR_SYSTEM_ASSERT_FAILED       1021
# define ERROR_READER_BUFFER_OVERFLOW     1022
# define ERROR_SYSTEM_CONFIG_INVALID      1023
# define ERROR_SYSTEM_CONFIG_DIRECTIVE    1024
# define ERROR_SYSTEM_CONFIG_BLOCK_START  1025
# define ERROR_SYSTEM_CONFIG_BLOCK_END    1026
# define ERROR_SYSTEM_CONFIG_EOF          1027
# define ERROR_SYSTEM_STREAM_BUSY         1028
# define ERROR_SYSTEM_IP_INVALID          1029
# define ERROR_SYSTEM_FORWARD_LOOP        1030
# define ERROR_SYSTEM_WAITPID             1031
# define ERROR_SYSTEM_BANDWIDTH_KEY       1032
# define ERROR_SYSTEM_BANDWIDTH_DENIED    1033
# define ERROR_SYSTEM_PID_ACQUIRE         1034
# define ERROR_SYSTEM_PID_ALREADY_RUNNING 1035
# define ERROR_SYSTEM_PID_LOCK            1036
# define ERROR_SYSTEM_PID_TRUNCATE_FILE   1037
# define ERROR_SYSTEM_PID_WRITE_FILE      1038
# define ERROR_SYSTEM_PID_GET_FILE_INFO   1039
# define ERROR_SYSTEM_PID_SET_FILE_INFO   1040
# define ERROR_SYSTEM_FILE_ALREADY_OPENED 1041
# define ERROR_SYSTEM_FILE_OPENE          1042
# define ERROR_SYSTEM_FILE_CLOSE          1043
# define ERROR_SYSTEM_FILE_READ           1044
# define ERROR_SYSTEM_FILE_WRITE          1045
# define ERROR_SYSTEM_FILE_EOF            1046
# define ERROR_SYSTEM_FILE_RENAME         1047
# define ERROR_SYSTEM_CREATE_PIPE         1048
# define ERROR_SYSTEM_FILE_SEEK           1049
# define ERROR_SYSTEM_IO_INVALID          1050
# define ERROR_ST_EXCEED_THREADS          1051
# define ERROR_SYSTEM_SECURITY            1052
# define ERROR_SYSTEM_SECURITY_DENY       1053
# define ERROR_SYSTEM_SECURITY_ALLOW      1054
# define ERROR_SYSTEM_TIME                1055
# define ERROR_SYSTEM_DIR_EXISTS          1056
# define ERROR_SYSTEM_CREATE_DIR          1057
# define ERROR_SYSTEM_KILL                1058
# define ERROR_RTMP_PLAIN_REQUIRED        2000
# define ERROR_RTMP_CHUNK_START           2001
# define ERROR_RTMP_MSG_INVALID_SIZE      2002
# define ERROR_RTMP_AMF0_DECODE           2003
# define ERROR_RTMP_AMF0_INVALID          2004
# define ERROR_RTMP_REQ_CONNECT           2005
# define ERROR_RTMP_REQ_TCURL             2006
# define ERROR_RTMP_MESSAGE_DECODE        2007
# define ERROR_RTMP_MESSAGE_ENCODE        2008
# define ERROR_RTMP_AMF0_ENCODE           2009
# define ERROR_RTMP_CHUNK_SIZE            2010
# define ERROR_RTMP_TRY_SIMPLE_HS         2011
# define ERROR_RTMP_CH_SCHEMA             2012
# define ERROR_RTMP_PACKET_SIZE           2013
# define ERROR_RTMP_VHOST_NOT_FOUND       2014
# define ERROR_RTMP_ACCESS_DENIED         2015
# define ERROR_RTMP_HANDSHAKE             2016
# define ERROR_RTMP_NO_REQUEST            2017
# define ERROR_RTMP_HS_SSL_REQUIRE        2018
# define ERROR_RTMP_DURATION_EXCEED       2019
# define ERROR_RTMP_EDGE_PLAY_STATE       2020
# define ERROR_RTMP_EDGE_PUBLISH_STATE    2021
# define ERROR_RTMP_EDGE_PROXY_PULL       2022
# define ERROR_RTMP_EDGE_RELOAD           2023
# define ERROR_RTMP_AGGREGATE             2024
# define ERROR_RTMP_BWTC_DATA             2025
# define ERROR_OpenSslCreateDH            2026
# define ERROR_OpenSslCreateP             2027
# define ERROR_OpenSslCreateG             2028
# define ERROR_OpenSslParseP1024          2029
# define ERROR_OpenSslSetG                2030
# define ERROR_OpenSslGenerateDHKeys      2031
# define ERROR_OpenSslCopyKey             2032
# define ERROR_OpenSslSha256Update        2033
# define ERROR_OpenSslSha256Init          2034
# define ERROR_OpenSslSha256Final         2035
# define ERROR_OpenSslSha256EvpDigest     2036
# define ERROR_OpenSslSha256DigestSize    2037
# define ERROR_OpenSslGetPeerPublicKey    2038
# define ERROR_OpenSslComputeSharedKey    2039
# define ERROR_RTMP_MIC_CHUNKSIZE_CHANGED 2040
# define ERROR_RTMP_MIC_CACHE_OVERFLOW    2041
# define ERROR_RTSP_TOKEN_NOT_NORMAL      2042
# define ERROR_RTSP_REQUEST_HEADER_EOF    2043
# define ERROR_RTP_HEADER_CORRUPT         2044
# define ERROR_RTP_TYPE96_CORRUPT         2045
# define ERROR_RTP_TYPE97_CORRUPT         2046
# define ERROR_RTSP_AUDIO_CONFIG          2047
# define ERROR_RTMP_STREAM_NOT_FOUND      2048
# define ERROR_RTMP_CLIENT_NOT_FOUND      2049
# define ERROR_CONTROL_RTMP_CLOSE         2998
# define ERROR_CONTROL_REPUBLISH          2999
# define ERROR_HLS_METADATA               3000
# define ERROR_HLS_DECODE_ERROR           3001
# define ERROR_HLS_OPEN_FAILED            3003
# define ERROR_HLS_WRITE_FAILED           3004
# define ERROR_HLS_AAC_FRAME_LENGTH       3005
# define ERROR_HLS_AVC_SAMPLE_SIZE        3006
# define ERROR_HTTP_PARSE_URI             3007
# define ERROR_HTTP_DATA_INVALID          3008
# define ERROR_HTTP_PARSE_HEADER          3009
# define ERROR_HTTP_HANDLER_MATCH_URL     3010
# define ERROR_HTTP_HANDLER_INVALID       3011
# define ERROR_HTTP_API_LOGS              3012
# define ERROR_HTTP_REMUX_SEQUENCE_HEADER 3013
# define ERROR_HTTP_REMUX_OFFSET_OVERFLOW 3014
# define ERROR_ENCODER_VCODEC             3015
# define ERROR_ENCODER_OUTPUT             3016
# define ERROR_ENCODER_ACHANNELS          3017
# define ERROR_ENCODER_ASAMPLE_RATE       3018
# define ERROR_ENCODER_ABITRATE           3019
# define ERROR_ENCODER_ACODEC             3020
# define ERROR_ENCODER_VPRESET            3021
# define ERROR_ENCODER_VPROFILE           3022
# define ERROR_ENCODER_VTHREADS           3023
# define ERROR_ENCODER_VHEIGHT            3024
# define ERROR_ENCODER_VWIDTH             3025
# define ERROR_ENCODER_VFPS               3026
# define ERROR_ENCODER_VBITRATE           3027
# define ERROR_ENCODER_FORK               3028
# define ERROR_ENCODER_LOOP               3029
# define ERROR_ENCODER_OPEN               3030
# define ERROR_ENCODER_DUP2               3031
# define ERROR_ENCODER_PARSE              3032
# define ERROR_ENCODER_NO_INPUT           3033
# define ERROR_ENCODER_NO_OUTPUT          3034
# define ERROR_ENCODER_INPUT_TYPE         3035
# define ERROR_KERNEL_FLV_HEADER          3036
# define ERROR_KERNEL_FLV_STREAM_CLOSED   3037
# define ERROR_KERNEL_STREAM_INIT         3038
# define ERROR_EDGE_VHOST_REMOVED         3039
# define ERROR_HLS_AVC_TRY_OTHERS         3040
# define ERROR_H264_API_NO_PREFIXED       3041
# define ERROR_FLV_INVALID_VIDEO_TAG      3042
# define ERROR_H264_DROP_BEFORE_SPS_PPS   3043
# define ERROR_H264_DUPLICATED_SPS        3044
# define ERROR_H264_DUPLICATED_PPS        3045
# define ERROR_AAC_REQUIRED_ADTS          3046
# define ERROR_AAC_ADTS_HEADER            3047
# define ERROR_AAC_DATA_INVALID           3048
# define ERROR_HLS_TRY_MP3                3049
# define ERROR_HTTP_DVR_DISABLED          3050
# define ERROR_HTTP_DVR_REQUEST           3051
# define ERROR_HTTP_JSON_REQUIRED         3052
# define ERROR_HTTP_DVR_CREATE_REQUEST    3053
# define ERROR_HTTP_DVR_NO_TAEGET         3054
# define ERROR_ADTS_ID_NOT_AAC            3055
# define ERROR_HDS_OPEN_F4M_FAILED        3056
# define ERROR_HDS_WRITE_F4M_FAILED       3057
# define ERROR_HDS_OPEN_BOOTSTRAP_FAILED  3058
# define ERROR_HDS_WRITE_BOOTSTRAP_FAILED 3059
# define ERROR_HDS_OPEN_FRAGMENT_FAILED   3060
# define ERROR_HDS_WRITE_FRAGMENT_FAILED  3061
# define ERROR_HLS_NO_STREAM              3062
# define ERROR_JSON_LOADS                 3063
# define ERROR_RESPONSE_CODE              3064
# define ERROR_RESPONSE_DATA              3065
# define ERROR_REQUEST_DATA               3066
# define ERROR_HTTP_PATTERN_EMPTY         4000
# define ERROR_HTTP_PATTERN_DUPLICATED    4001
# define ERROR_HTTP_URL_NOT_CLEAN         4002
# define ERROR_HTTP_CONTENT_LENGTH        4003
# define ERROR_HTTP_LIVE_STREAM_EXT       4004
# define ERROR_HTTP_STATUS_INVALID        4005
# define ERROR_KERNEL_AAC_STREAM_CLOSED   4006
# define ERROR_AAC_DECODE_ERROR           4007
# define ERROR_KERNEL_MP3_STREAM_CLOSED   4008
# define ERROR_MP3_DECODE_ERROR           4009
# define ERROR_STREAM_CASTER_ENGINE       4010
# define ERROR_STREAM_CASTER_PORT         4011
# define ERROR_STREAM_CASTER_TS_HEADER    4012
# define ERROR_STREAM_CASTER_TS_SYNC_BYTE 4013
# define ERROR_STREAM_CASTER_TS_AF        4014
# define ERROR_STREAM_CASTER_TS_CRC32     4015
# define ERROR_STREAM_CASTER_TS_PSI       4016
# define ERROR_STREAM_CASTER_TS_PAT       4017
# define ERROR_STREAM_CASTER_TS_PMT       4018
# define ERROR_STREAM_CASTER_TS_PSE       4019
# define ERROR_STREAM_CASTER_TS_ES        4020
# define ERROR_STREAM_CASTER_TS_CODEC     4021
# define ERROR_STREAM_CASTER_AVC_SPS      4022
# define ERROR_STREAM_CASTER_AVC_PPS      4023
# define ERROR_STREAM_CASTER_FLV_TAG      4024
# define ERROR_HTTP_RESPONSE_EOF          4025
# define ERROR_HTTP_INVALID_CHUNK_HEADER  4026
# define ERROR_AVC_NALU_UEV               4027
# define ERROR_AAC_BYTES_INVALID          4028
# define ERROR_HTTP_REQUEST_EOF           4029
# define ERROR_USER_START                 9000
# define ERROR_USER_DISCONNECT            9001
# define ERROR_SOURCE_NOT_FOUND           9002
# define ERROR_USER_END                   9999
extern bool srs_is_system_control_error(int error_code);
extern bool srs_is_client_gracefully_close(int error_code);
#endif // ifndef SRS_KERNEL_ERROR_HPP
#ifndef SRS_KERNEL_LOG_HPP
# define SRS_KERNEL_LOG_HPP
# include <stdio.h>
# include <errno.h>
# include <string.h>
class SrsLogLevel
{
public:
    static const int Verbose  = 0x01;
    static const int Info     = 0x02;
    static const int Trace    = 0x03;
    static const int Warn     = 0x04;
    static const int Error    = 0x05;
    static const int Disabled = 0x06;
};
class ISrsLog
{
public:
    ISrsLog();
    virtual ~ISrsLog();
public:
    virtual int initialize();
public:
    virtual void verbose(const char* tag, int context_id, const char* fmt, ...);
    virtual void info(const char* tag, int context_id, const char* fmt, ...);
    virtual void trace(const char* tag, int context_id, const char* fmt, ...);
    virtual void warn(const char* tag, int context_id, const char* fmt, ...);
    virtual void error(const char* tag, int context_id, const char* fmt, ...);
};
class ISrsThreadContext
{
public:
    ISrsThreadContext();
    virtual ~ISrsThreadContext();
public:
    virtual int generate_id();
    virtual int get_id();
    virtual int set_id(int v);
};
extern ISrsLog* _srs_log;
extern ISrsThreadContext* _srs_context;
# if 1
#  define srs_verbose(msg, ...) _srs_log->verbose(NULL, _srs_context->get_id(), msg, ## __VA_ARGS__)
#  define srs_info(msg, ...)    _srs_log->info(NULL, _srs_context->get_id(), msg, ## __VA_ARGS__)
#  define srs_trace(msg, ...)   _srs_log->trace(NULL, _srs_context->get_id(), msg, ## __VA_ARGS__)
#  define srs_warn(msg, ...)    _srs_log->warn(NULL, _srs_context->get_id(), msg, ## __VA_ARGS__)
#  define srs_error(msg, ...)   _srs_log->error(NULL, _srs_context->get_id(), msg, ## __VA_ARGS__)
# elif 0
#  define srs_verbose(msg, ...) _srs_log->verbose(__FUNCTION__, _srs_context->get_id(), msg, ## __VA_ARGS__)
#  define srs_info(msg, ...)    _srs_log->info(__FUNCTION__, _srs_context->get_id(), msg, ## __VA_ARGS__)
#  define srs_trace(msg, ...)   _srs_log->trace(__FUNCTION__, _srs_context->get_id(), msg, ## __VA_ARGS__)
#  define srs_warn(msg, ...)    _srs_log->warn(__FUNCTION__, _srs_context->get_id(), msg, ## __VA_ARGS__)
#  define srs_error(msg, ...)   _srs_log->error(__FUNCTION__, _srs_context->get_id(), msg, ## __VA_ARGS__)
# else // if 1
#  define srs_verbose(msg, ...) _srs_log->verbose(__PRETTY_FUNCTION__, _srs_context->get_id(), msg, ## __VA_ARGS__)
#  define srs_info(msg, ...)    _srs_log->info(__PRETTY_FUNCTION__, _srs_context->get_id(), msg, ## __VA_ARGS__)
#  define srs_trace(msg, ...)   _srs_log->trace(__PRETTY_FUNCTION__, _srs_context->get_id(), msg, ## __VA_ARGS__)
#  define srs_warn(msg, ...)    _srs_log->warn(__PRETTY_FUNCTION__, _srs_context->get_id(), msg, ## __VA_ARGS__)
#  define srs_error(msg, ...)   _srs_log->error(__PRETTY_FUNCTION__, _srs_context->get_id(), msg, ## __VA_ARGS__)
# endif // if 1
# ifndef SRS_AUTO_VERBOSE
#  undef srs_verbose
#  define srs_verbose(msg, ...) (void) 0
# endif
# ifndef SRS_AUTO_INFO
#  undef srs_info
#  define srs_info(msg, ...) (void) 0
# endif
# ifndef SRS_AUTO_TRACE
#  undef srs_trace
#  define srs_trace(msg, ...) (void) 0
# endif
#endif // ifndef SRS_KERNEL_LOG_HPP
#ifndef SRS_KERNEL_STREAM_HPP
# define SRS_KERNEL_STREAM_HPP
# include <sys/types.h>
# include <string>
class SrsStream
{
private:
    char* p;
    char* bytes;
    int nb_bytes;
public:
    SrsStream();
    virtual ~SrsStream();
public:
    virtual int initialize(char* b, int nb);
public:
    virtual char * data();
    virtual int size();
    virtual int pos();
    virtual bool empty();
    virtual bool require(int required_size);
public:
    virtual void skip(int size);
public:
    virtual int8_t read_1bytes();
    virtual int16_t read_2bytes();
    virtual int32_t read_3bytes();
    virtual int32_t read_4bytes();
    virtual int64_t read_8bytes();
    virtual std::string read_string(int len);
    virtual void read_bytes(char* data, int size);
public:
    virtual void write_1bytes(int8_t value);
    virtual void write_2bytes(int16_t value);
    virtual void write_4bytes(int32_t value);
    virtual void write_3bytes(int32_t value);
    virtual void write_8bytes(int64_t value);
    virtual void write_string(std::string value);
    virtual void write_bytes(char* data, int size);
};
class SrsBitStream
{
private:
    int8_t cb;
    u_int8_t cb_left;
    SrsStream* stream;
public:
    SrsBitStream();
    virtual ~SrsBitStream();
public:
    virtual int initialize(SrsStream* s);
    virtual bool empty();
    virtual int8_t read_bit();
};
#endif // ifndef SRS_KERNEL_STREAM_HPP
#ifndef SRS_KERNEL_UTILITY_HPP
# define SRS_KERNEL_UTILITY_HPP
# include <string>
class SrsStream;
class SrsBitStream;
# define srs_min(a, b) (((a) < (b)) ? (a) : (b))
# define srs_max(a, b) (((a) < (b)) ? (b) : (a))
extern int srs_avc_nalu_read_uev(SrsBitStream* stream, int32_t& v);
extern int srs_avc_nalu_read_bit(SrsBitStream* stream, int8_t& v);
extern int64_t srs_get_system_time_ms();
extern int64_t srs_get_system_startup_time_ms();
extern int64_t srs_update_system_time_ms();
extern std::string srs_dns_resolve(std::string host);
extern bool srs_is_little_endian();
extern std::string srs_string_replace(std::string str, std::string old_str, std::string new_str);
extern std::string srs_string_trim_end(std::string str, std::string trim_chars);
extern std::string srs_string_trim_start(std::string str, std::string trim_chars);
extern std::string srs_string_remove(std::string str, std::string remove_chars);
extern bool srs_string_ends_with(std::string str, std::string flag);
extern bool srs_string_starts_with(std::string str, std::string flag);
extern bool srs_string_starts_with(std::string str, std::string flag0, std::string flag1);
extern bool srs_string_contains(std::string str, std::string flag);
extern bool srs_string_contains(std::string str, std::string flag0, std::string flag1);
extern bool srs_string_contains(std::string str, std::string flag0, std::string flag1, std::string flag2);
extern int srs_create_dir_recursively(std::string dir);
extern bool srs_path_exists(std::string path);
extern std::string srs_path_dirname(std::string path);
extern std::string srs_path_basename(std::string path);
extern bool srs_avc_startswith_annexb(SrsStream* stream, int* pnb_start_code = NULL);
extern bool srs_aac_startswith_adts(SrsStream* stream);
extern u_int32_t srs_crc32(const void* buf, int size);
extern int srs_av_base64_decode(u_int8_t* out, const char* in, int out_size);
extern char * srs_av_base64_encode(char* out, int out_size, const u_int8_t* in, int in_size);
# define SRS_AV_BASE64_SIZE(x) (((x)+2) / 3 * 4 + 1)
extern int ff_hex_to_data(u_int8_t* data, const char* p);
extern int srs_chunk_header_c0(
    int perfer_cid, u_int32_t timestamp, int32_t payload_length,
    int8_t message_type, int32_t stream_id,
    char* cache, int nb_cache
);
extern int srs_chunk_header_c3(
    int perfer_cid, u_int32_t timestamp,
    char* cache, int nb_cache
);
#endif // ifndef SRS_KERNEL_UTILITY_HPP
#ifndef SRS_KERNEL_FLV_HPP
# define SRS_KERNEL_FLV_HPP
# include <string>
# ifndef _WIN32
#  include <sys/uio.h>
# endif
class SrsStream;
class SrsFileWriter;
class SrsFileReader;
# define SRS_FLV_TAG_HEADER_SIZE             11
# define SRS_FLV_PREVIOUS_TAG_SIZE           4
# define RTMP_MSG_SetChunkSize               0x01
# define RTMP_MSG_AbortMessage               0x02
# define RTMP_MSG_Acknowledgement            0x03
# define RTMP_MSG_UserControlMessage         0x04
# define RTMP_MSG_WindowAcknowledgementSize  0x05
# define RTMP_MSG_SetPeerBandwidth           0x06
# define RTMP_MSG_EdgeAndOriginServerCommand 0x07
# define RTMP_MSG_AMF3CommandMessage         17
# define RTMP_MSG_AMF0CommandMessage         20
# define RTMP_MSG_AMF0DataMessage            18
# define RTMP_MSG_AMF3DataMessage            15
# define RTMP_MSG_AMF3SharedObject           16
# define RTMP_MSG_AMF0SharedObject           19
# define RTMP_MSG_AudioMessage               8
# define RTMP_MSG_VideoMessage               9
# define RTMP_MSG_AggregateMessage           22
# define RTMP_CID_ProtocolControl            0x02
# define RTMP_CID_OverConnection             0x03
# define RTMP_CID_OverConnection2            0x04
# define RTMP_CID_OverStream                 0x05
# define RTMP_CID_OverStream2                0x08
# define RTMP_CID_Video                      0x06
# define RTMP_CID_Audio                      0x07
# define RTMP_EXTENDED_TIMESTAMP             0xFFFFFF
class SrsMessageHeader
{
public:
    int32_t timestamp_delta;
    int32_t payload_length;
    int8_t message_type;
    int32_t stream_id;
    int64_t timestamp;
public:
    int perfer_cid;
public:
    SrsMessageHeader();
    virtual ~SrsMessageHeader();
public:
    bool is_audio();
    bool is_video();
    bool is_amf0_command();
    bool is_amf0_data();
    bool is_amf3_command();
    bool is_amf3_data();
    bool is_window_ackledgement_size();
    bool is_ackledgement();
    bool is_set_chunk_size();
    bool is_user_control_message();
    bool is_set_peer_bandwidth();
    bool is_aggregate();
public:
    void initialize_amf0_script(int size, int stream);
    void initialize_audio(int size, u_int32_t time, int stream);
    void initialize_video(int size, u_int32_t time, int stream);
};
class SrsCommonMessage
{
public:
    SrsMessageHeader header;
public:
    int size;
    char* payload;
public:
    SrsCommonMessage();
    virtual ~SrsCommonMessage();
public:
    virtual void create_payload(int size);
};
struct SrsSharedMessageHeader {
    int32_t payload_length;
    int8_t  message_type;
    int     perfer_cid;
};
class SrsSharedPtrMessage
{
public:
    int64_t timestamp;
    int32_t stream_id;
public:
    int size;
    char* payload;
private:
    class SrsSharedPtrPayload
    {
public:
        SrsSharedMessageHeader header;
        char* payload;
        int size;
        int shared_count;
public:
        SrsSharedPtrPayload();
        virtual ~SrsSharedPtrPayload();
    };
    SrsSharedPtrPayload* ptr;
public:
    SrsSharedPtrMessage();
    virtual ~SrsSharedPtrMessage();
public:
    virtual int create(SrsCommonMessage* msg);
    virtual int create(SrsMessageHeader* pheader, char* payload, int size);
    virtual int count();
    virtual bool check(int stream_id);
public:
    virtual bool is_av();
    virtual bool is_audio();
    virtual bool is_video();
public:
    virtual int chunk_header(char* cache, int nb_cache, bool c0);
public:
    virtual SrsSharedPtrMessage * copy();
};
class SrsFlvEncoder
{
private:
    SrsFileWriter* reader;
private:
    SrsStream* tag_stream;
    char tag_header[SRS_FLV_TAG_HEADER_SIZE];
public:
    SrsFlvEncoder();
    virtual ~SrsFlvEncoder();
public:
    virtual int initialize(SrsFileWriter* fr);
public:
    virtual int write_header();
    virtual int write_header(char flv_header[9]);
    virtual int write_metadata(char type, char* data, int size);
    virtual int write_audio(int64_t timestamp, char* data, int size);
    virtual int write_video(int64_t timestamp, char* data, int size);
public:
    static int size_tag(int data_size);
    # ifdef SRS_PERF_FAST_FLV_ENCODER
private:
    int nb_tag_headers;
    char* tag_headers;
    int nb_ppts;
    char* ppts;
    int nb_iovss_cache;
    iovec* iovss_cache;
public:
    virtual int write_tags(SrsSharedPtrMessage** msgs, int count);
    # endif // ifdef SRS_PERF_FAST_FLV_ENCODER
private:
    virtual int write_metadata_to_cache(char type, char* data, int size, char* cache);
    virtual int write_audio_to_cache(int64_t timestamp, char* data, int size, char* cache);
    virtual int write_video_to_cache(int64_t timestamp, char* data, int size, char* cache);
    virtual int write_pts_to_cache(int size, char* cache);
    virtual int write_tag(char* header, int header_size, char* tag, int tag_size);
};
class SrsFlvDecoder
{
private:
    SrsFileReader* reader;
private:
    SrsStream* tag_stream;
public:
    SrsFlvDecoder();
    virtual ~SrsFlvDecoder();
public:
    virtual int initialize(SrsFileReader* fr);
public:
    virtual int read_header(char header[9]);
    virtual int read_tag_header(char* ptype, int32_t* pdata_size, u_int32_t* ptime);
    virtual int read_tag_data(char* data, int32_t size);
    virtual int read_previous_tag_size(char previous_tag_size[4]);
};
class SrsFlvVodStreamDecoder
{
private:
    SrsFileReader* reader;
private:
    SrsStream* tag_stream;
public:
    SrsFlvVodStreamDecoder();
    virtual ~SrsFlvVodStreamDecoder();
public:
    virtual int initialize(SrsFileReader* fr);
public:
    virtual int read_header_ext(char header[13]);
    virtual int read_sequence_header_summary(int64_t* pstart, int* psize);
public:
    virtual int lseek(int64_t offset);
};
#endif // ifndef SRS_KERNEL_FLV_HPP
#ifndef SRS_KERNEL_FILE_HPP
# define SRS_KERNEL_FILE_HPP
# include <string>
# ifndef _WIN32
#  include <sys/uio.h>
# endif
class SrsFileWriter
{
private:
    std::string path;
    int fd;
public:
    SrsFileWriter();
    virtual ~SrsFileWriter();
public:
    virtual int open(std::string p);
    virtual int open_append(std::string p);
    virtual void close();
public:
    virtual bool is_open();
    virtual void lseek(int64_t offset);
    virtual int64_t tellg();
public:
    virtual int write(void* buf, size_t count, ssize_t* pnwrite);
    virtual int writev(iovec* iov, int iovcnt, ssize_t* pnwrite);
};
class SrsFileReader
{
private:
    std::string path;
    int fd;
public:
    SrsFileReader();
    virtual ~SrsFileReader();
public:
    virtual int open(std::string p);
    virtual void close();
public:
    virtual bool is_open();
    virtual int64_t tellg();
    virtual void skip(int64_t size);
    virtual int64_t lseek(int64_t offset);
    virtual int64_t filesize();
public:
    virtual int read(void* buf, size_t count, ssize_t* pnread);
};
#endif // ifndef SRS_KERNEL_FILE_HPP
#ifndef SRS_KERNEL_CONSTS_HPP
# define SRS_KERNEL_CONSTS_HPP
# define SRS_CONSTS_RTMP_DEFAULT_VHOST                      "__defaultVhost__"
# define SRS_CONSTS_RTMP_DEFAULT_PORT                       "1935"
# define SRS_CONSTS_RTMP_SRS_CHUNK_SIZE                     60000
# define SRS_CONSTS_RTMP_PROTOCOL_CHUNK_SIZE                128
# define SRS_CONSTS_RTMP_MIN_CHUNK_SIZE                     128
# define SRS_CONSTS_RTMP_MAX_CHUNK_SIZE                     65536
# define SRS_CONSTS_RTMP_SEND_TIMEOUT_US                    (int64_t) (30*1000*1000LL)
# define SRS_CONSTS_RTMP_RECV_TIMEOUT_US                    (int64_t) (30*1000*1000LL)
# define SRS_CONSTS_RTMP_PULSE_TIMEOUT_US                   (int64_t) (500*1000LL)
# define SRS_CONSTS_RTMP_MAX_FMT0_HEADER_SIZE               16
# define SRS_CONSTS_RTMP_MAX_FMT3_HEADER_SIZE               5
# define SRS_CONSTS_IOVS_MAX                                (SRS_PERF_MW_MSGS * 2)
# define SRS_CONSTS_C0C3_HEADERS_MAX                        (SRS_PERF_MW_MSGS * 32)
# define SRS_CONSTS_NULL_FILE                               "/dev/null"
# define SRS_CONSTS_LOCALHOST                               "127.0.0.1"
# define SRS_CONSTS_LOG_EDGE_PLAY                           "EIG"
# define SRS_CONSTS_LOG_EDGE_PUBLISH                        "EFW"
# define SRS_CONSTS_LOG_FOWARDER                            "FWR"
# define SRS_CONSTS_LOG_PLAY                                "PLA"
# define SRS_CONSTS_LOG_CLIENT_PUBLISH                      "CPB"
# define SRS_CONSTS_LOG_WEB_PUBLISH                         "WPB"
# define SRS_CONSTS_LOG_INGESTER                            "IGS"
# define SRS_CONSTS_LOG_HLS                                 "HLS"
# define SRS_CONSTS_LOG_ENCODER                             "ENC"
# define SRS_CONSTS_LOG_HTTP_STREAM                         "HTS"
# define SRS_CONSTS_LOG_HTTP_STREAM_CACHE                   "HTC"
# define SRS_CONSTS_LOG_STREAM_CASTER                       "SCS"
# define SRS_CONSTS_RTMP_SET_DATAFRAME                      "@setDataFrame"
# define SRS_CONSTS_RTMP_ON_METADATA                        "onMetaData"
# define SRS_CONSTS_CR                                      '\r'
# define SRS_CONSTS_LF                                      '\n'
# define SRS_CONSTS_HTTP_PATH_SEP                           '/'
# define SRS_CONSTS_HTTP_QUERY_SEP                          '?'
# define SRS_HTTP_RECV_TIMEOUT_US                           60 * 1000 * 1000
# define SRS_CONSTS_HTTP_Continue                           100
# define SRS_CONSTS_HTTP_SwitchingProtocols                 101
# define SRS_CONSTS_HTTP_OK                                 200
# define SRS_CONSTS_HTTP_Created                            201
# define SRS_CONSTS_HTTP_Accepted                           202
# define SRS_CONSTS_HTTP_NonAuthoritativeInformation        203
# define SRS_CONSTS_HTTP_NoContent                          204
# define SRS_CONSTS_HTTP_ResetContent                       205
# define SRS_CONSTS_HTTP_PartialContent                     206
# define SRS_CONSTS_HTTP_MultipleChoices                    300
# define SRS_CONSTS_HTTP_MovedPermanently                   301
# define SRS_CONSTS_HTTP_Found                              302
# define SRS_CONSTS_HTTP_SeeOther                           303
# define SRS_CONSTS_HTTP_NotModified                        304
# define SRS_CONSTS_HTTP_UseProxy                           305
# define SRS_CONSTS_HTTP_TemporaryRedirect                  307
# define SRS_CONSTS_HTTP_BadRequest                         400
# define SRS_CONSTS_HTTP_Unauthorized                       401
# define SRS_CONSTS_HTTP_PaymentRequired                    402
# define SRS_CONSTS_HTTP_Forbidden                          403
# define SRS_CONSTS_HTTP_NotFound                           404
# define SRS_CONSTS_HTTP_MethodNotAllowed                   405
# define SRS_CONSTS_HTTP_NotAcceptable                      406
# define SRS_CONSTS_HTTP_ProxyAuthenticationRequired        407
# define SRS_CONSTS_HTTP_RequestTimeout                     408
# define SRS_CONSTS_HTTP_Conflict                           409
# define SRS_CONSTS_HTTP_Gone                               410
# define SRS_CONSTS_HTTP_LengthRequired                     411
# define SRS_CONSTS_HTTP_PreconditionFailed                 412
# define SRS_CONSTS_HTTP_RequestEntityTooLarge              413
# define SRS_CONSTS_HTTP_RequestURITooLarge                 414
# define SRS_CONSTS_HTTP_UnsupportedMediaType               415
# define SRS_CONSTS_HTTP_RequestedRangeNotSatisfiable       416
# define SRS_CONSTS_HTTP_ExpectationFailed                  417
# define SRS_CONSTS_HTTP_InternalServerError                500
# define SRS_CONSTS_HTTP_NotImplemented                     501
# define SRS_CONSTS_HTTP_BadGateway                         502
# define SRS_CONSTS_HTTP_ServiceUnavailable                 503
# define SRS_CONSTS_HTTP_GatewayTimeout                     504
# define SRS_CONSTS_HTTP_HTTPVersionNotSupported            505
# define SRS_CONSTS_HTTP_Continue_str                       "Continue"
# define SRS_CONSTS_HTTP_SwitchingProtocols_str             "Switching Protocols"
# define SRS_CONSTS_HTTP_OK_str                             "OK"
# define SRS_CONSTS_HTTP_Created_str                        "Created"
# define SRS_CONSTS_HTTP_Accepted_str                       "Accepted"
# define SRS_CONSTS_HTTP_NonAuthoritativeInformation_str    "Non Authoritative Information"
# define SRS_CONSTS_HTTP_NoContent_str                      "No Content"
# define SRS_CONSTS_HTTP_ResetContent_str                   "Reset Content"
# define SRS_CONSTS_HTTP_PartialContent_str                 "Partial Content"
# define SRS_CONSTS_HTTP_MultipleChoices_str                "Multiple Choices"
# define SRS_CONSTS_HTTP_MovedPermanently_str               "Moved Permanently"
# define SRS_CONSTS_HTTP_Found_str                          "Found"
# define SRS_CONSTS_HTTP_SeeOther_str                       "See Other"
# define SRS_CONSTS_HTTP_NotModified_str                    "Not Modified"
# define SRS_CONSTS_HTTP_UseProxy_str                       "Use Proxy"
# define SRS_CONSTS_HTTP_TemporaryRedirect_str              "Temporary Redirect"
# define SRS_CONSTS_HTTP_BadRequest_str                     "Bad Request"
# define SRS_CONSTS_HTTP_Unauthorized_str                   "Unauthorized"
# define SRS_CONSTS_HTTP_PaymentRequired_str                "Payment Required"
# define SRS_CONSTS_HTTP_Forbidden_str                      "Forbidden"
# define SRS_CONSTS_HTTP_NotFound_str                       "Not Found"
# define SRS_CONSTS_HTTP_MethodNotAllowed_str               "Method Not Allowed"
# define SRS_CONSTS_HTTP_NotAcceptable_str                  "Not Acceptable"
# define SRS_CONSTS_HTTP_ProxyAuthenticationRequired_str    "Proxy Authentication Required"
# define SRS_CONSTS_HTTP_RequestTimeout_str                 "Request Timeout"
# define SRS_CONSTS_HTTP_Conflict_str                       "Conflict"
# define SRS_CONSTS_HTTP_Gone_str                           "Gone"
# define SRS_CONSTS_HTTP_LengthRequired_str                 "Length Required"
# define SRS_CONSTS_HTTP_PreconditionFailed_str             "Precondition Failed"
# define SRS_CONSTS_HTTP_RequestEntityTooLarge_str          "Request Entity Too Large"
# define SRS_CONSTS_HTTP_RequestURITooLarge_str             "Request URI Too Large"
# define SRS_CONSTS_HTTP_UnsupportedMediaType_str           "Unsupported Media Type"
# define SRS_CONSTS_HTTP_RequestedRangeNotSatisfiable_str   "Requested Range Not Satisfiable"
# define SRS_CONSTS_HTTP_ExpectationFailed_str              "Expectation Failed"
# define SRS_CONSTS_HTTP_InternalServerError_str            "Internal Server Error"
# define SRS_CONSTS_HTTP_NotImplemented_str                 "Not Implemented"
# define SRS_CONSTS_HTTP_BadGateway_str                     "Bad Gateway"
# define SRS_CONSTS_HTTP_ServiceUnavailable_str             "Service Unavailable"
# define SRS_CONSTS_HTTP_GatewayTimeout_str                 "Gateway Timeout"
# define SRS_CONSTS_HTTP_HTTPVersionNotSupported_str        "HTTP Version Not Supported"
# define SRS_CONSTS_RTSP_Continue                           100
# define SRS_CONSTS_RTSP_OK                                 200
# define SRS_CONSTS_RTSP_Created                            201
# define SRS_CONSTS_RTSP_LowOnStorageSpace                  250
# define SRS_CONSTS_RTSP_MultipleChoices                    300
# define SRS_CONSTS_RTSP_MovedPermanently                   301
# define SRS_CONSTS_RTSP_MovedTemporarily                   302
# define SRS_CONSTS_RTSP_SeeOther                           303
# define SRS_CONSTS_RTSP_NotModified                        304
# define SRS_CONSTS_RTSP_UseProxy                           305
# define SRS_CONSTS_RTSP_BadRequest                         400
# define SRS_CONSTS_RTSP_Unauthorized                       401
# define SRS_CONSTS_RTSP_PaymentRequired                    402
# define SRS_CONSTS_RTSP_Forbidden                          403
# define SRS_CONSTS_RTSP_NotFound                           404
# define SRS_CONSTS_RTSP_MethodNotAllowed                   405
# define SRS_CONSTS_RTSP_NotAcceptable                      406
# define SRS_CONSTS_RTSP_ProxyAuthenticationRequired        407
# define SRS_CONSTS_RTSP_RequestTimeout                     408
# define SRS_CONSTS_RTSP_Gone                               410
# define SRS_CONSTS_RTSP_LengthRequired                     411
# define SRS_CONSTS_RTSP_PreconditionFailed                 412
# define SRS_CONSTS_RTSP_RequestEntityTooLarge              413
# define SRS_CONSTS_RTSP_RequestURITooLarge                 414
# define SRS_CONSTS_RTSP_UnsupportedMediaType               415
# define SRS_CONSTS_RTSP_ParameterNotUnderstood             451
# define SRS_CONSTS_RTSP_ConferenceNotFound                 452
# define SRS_CONSTS_RTSP_NotEnoughBandwidth                 453
# define SRS_CONSTS_RTSP_SessionNotFound                    454
# define SRS_CONSTS_RTSP_MethodNotValidInThisState          455
# define SRS_CONSTS_RTSP_HeaderFieldNotValidForResource     456
# define SRS_CONSTS_RTSP_InvalidRange                       457
# define SRS_CONSTS_RTSP_ParameterIsReadOnly                458
# define SRS_CONSTS_RTSP_AggregateOperationNotAllowed       459
# define SRS_CONSTS_RTSP_OnlyAggregateOperationAllowed      460
# define SRS_CONSTS_RTSP_UnsupportedTransport               461
# define SRS_CONSTS_RTSP_DestinationUnreachable             462
# define SRS_CONSTS_RTSP_InternalServerError                500
# define SRS_CONSTS_RTSP_NotImplemented                     501
# define SRS_CONSTS_RTSP_BadGateway                         502
# define SRS_CONSTS_RTSP_ServiceUnavailable                 503
# define SRS_CONSTS_RTSP_GatewayTimeout                     504
# define SRS_CONSTS_RTSP_RTSPVersionNotSupported            505
# define SRS_CONSTS_RTSP_OptionNotSupported                 551
# define SRS_CONSTS_RTSP_Continue_str                       "Continue"
# define SRS_CONSTS_RTSP_OK_str                             "OK"
# define SRS_CONSTS_RTSP_Created_str                        "Created"
# define SRS_CONSTS_RTSP_LowOnStorageSpace_str              "Low on Storage Space"
# define SRS_CONSTS_RTSP_MultipleChoices_str                "Multiple Choices"
# define SRS_CONSTS_RTSP_MovedPermanently_str               "Moved Permanently"
# define SRS_CONSTS_RTSP_MovedTemporarily_str               "Moved Temporarily"
# define SRS_CONSTS_RTSP_SeeOther_str                       "See Other"
# define SRS_CONSTS_RTSP_NotModified_str                    "Not Modified"
# define SRS_CONSTS_RTSP_UseProxy_str                       "Use Proxy"
# define SRS_CONSTS_RTSP_BadRequest_str                     "Bad Request"
# define SRS_CONSTS_RTSP_Unauthorized_str                   "Unauthorized"
# define SRS_CONSTS_RTSP_PaymentRequired_str                "Payment Required"
# define SRS_CONSTS_RTSP_Forbidden_str                      "Forbidden"
# define SRS_CONSTS_RTSP_NotFound_str                       "Not Found"
# define SRS_CONSTS_RTSP_MethodNotAllowed_str               "Method Not Allowed"
# define SRS_CONSTS_RTSP_NotAcceptable_str                  "Not Acceptable"
# define SRS_CONSTS_RTSP_ProxyAuthenticationRequired_str    "Proxy Authentication Required"
# define SRS_CONSTS_RTSP_RequestTimeout_str                 "Request Timeout"
# define SRS_CONSTS_RTSP_Gone_str                           "Gone"
# define SRS_CONSTS_RTSP_LengthRequired_str                 "Length Required"
# define SRS_CONSTS_RTSP_PreconditionFailed_str             "Precondition Failed"
# define SRS_CONSTS_RTSP_RequestEntityTooLarge_str          "Request Entity Too Large"
# define SRS_CONSTS_RTSP_RequestURITooLarge_str             "Request URI Too Large"
# define SRS_CONSTS_RTSP_UnsupportedMediaType_str           "Unsupported Media Type"
# define SRS_CONSTS_RTSP_ParameterNotUnderstood_str         "Invalid parameter"
# define SRS_CONSTS_RTSP_ConferenceNotFound_str             "Illegal Conference Identifier"
# define SRS_CONSTS_RTSP_NotEnoughBandwidth_str             "Not Enough Bandwidth"
# define SRS_CONSTS_RTSP_SessionNotFound_str                "Session Not Found"
# define SRS_CONSTS_RTSP_MethodNotValidInThisState_str      "Method Not Valid In This State"
# define SRS_CONSTS_RTSP_HeaderFieldNotValidForResource_str "Header Field Not Valid"
# define SRS_CONSTS_RTSP_InvalidRange_str                   "Invalid Range"
# define SRS_CONSTS_RTSP_ParameterIsReadOnly_str            "Parameter Is Read-Only"
# define SRS_CONSTS_RTSP_AggregateOperationNotAllowed_str   "Aggregate Operation Not Allowed"
# define SRS_CONSTS_RTSP_OnlyAggregateOperationAllowed_str  "Only Aggregate Operation Allowed"
# define SRS_CONSTS_RTSP_UnsupportedTransport_str           "Unsupported Transport"
# define SRS_CONSTS_RTSP_DestinationUnreachable_str         "Destination Unreachable"
# define SRS_CONSTS_RTSP_InternalServerError_str            "Internal Server Error"
# define SRS_CONSTS_RTSP_NotImplemented_str                 "Not Implemented"
# define SRS_CONSTS_RTSP_BadGateway_str                     "Bad Gateway"
# define SRS_CONSTS_RTSP_ServiceUnavailable_str             "Service Unavailable"
# define SRS_CONSTS_RTSP_GatewayTimeout_str                 "Gateway Timeout"
# define SRS_CONSTS_RTSP_RTSPVersionNotSupported_str        "RTSP Version Not Supported"
# define SRS_CONSTS_RTSP_OptionNotSupported_str             "Option not support"
#endif // ifndef SRS_KERNEL_CONSTS_HPP
#ifndef SRS_KERNEL_AAC_HPP
# define SRS_KERNEL_AAC_HPP
# if !defined(SRS_EXPORT_LIBRTMP)
#  include <string>
class SrsStream;
class SrsFileWriter;
class SrsFileReader;
class SrsAacEncoder
{
private:
    SrsFileWriter* _fs;
private:
    SrsAacObjectType aac_object;
    int8_t aac_sample_rate;
    int8_t aac_channels;
    bool got_sequence_header;
private:
    SrsStream* tag_stream;
public:
    SrsAacEncoder();
    virtual ~SrsAacEncoder();
public:
    virtual int initialize(SrsFileWriter* fs);
public:
    virtual int write_audio(int64_t timestamp, char* data, int size);
};
# endif // if !defined(SRS_EXPORT_LIBRTMP)
#endif  // ifndef SRS_KERNEL_AAC_HPP
#ifndef SRS_KERNEL_MP3_HPP
# define SRS_KERNEL_MP3_HPP
# if !defined(SRS_EXPORT_LIBRTMP)
#  include <string>
class SrsStream;
class SrsFileWriter;
class SrsMp3Encoder
{
private:
    SrsFileWriter* writer;
private:
    SrsStream* tag_stream;
public:
    SrsMp3Encoder();
    virtual ~SrsMp3Encoder();
public:
    virtual int initialize(SrsFileWriter* fw);
public:
    virtual int write_header();
    virtual int write_audio(int64_t timestamp, char* data, int size);
};
# endif // if !defined(SRS_EXPORT_LIBRTMP)
#endif  // ifndef SRS_KERNEL_MP3_HPP
#ifndef SRS_KERNEL_TS_HPP
# define SRS_KERNEL_TS_HPP
# if !defined(SRS_EXPORT_LIBRTMP)
#  include <string>
#  include <map>
#  include <vector>
class SrsStream;
class SrsTsCache;
class SrsTSMuxer;
class SrsFileWriter;
class SrsFileReader;
class SrsAvcAacCodec;
class SrsCodecSample;
class SrsSimpleBuffer;
class SrsTsAdaptationField;
class SrsTsPayload;
class SrsTsMessage;
class SrsTsPacket;
class SrsTsContext;
#  define SRS_TS_PACKET_SIZE                  188
#  define SRS_CONSTS_HLS_PURE_AUDIO_AGGREGATE 720 * 90
enum SrsTsPid {
    SrsTsPidPAT           = 0x00,
    SrsTsPidCAT           = 0x01,
    SrsTsPidTSDT          = 0x02,
    SrsTsPidReservedStart = 0x03,
    SrsTsPidReservedEnd   = 0x0f,
    SrsTsPidAppStart      = 0x10,
    SrsTsPidAppEnd        = 0x1ffe,
    SrsTsPidNULL          = 0x01FFF,
};
enum SrsTsScrambled {
    SrsTsScrambledDisabled     = 0x00,
    SrsTsScrambledUserDefined1 = 0x01,
    SrsTsScrambledUserDefined2 = 0x02,
    SrsTsScrambledUserDefined3 = 0x03,
};
enum SrsTsAdaptationFieldType {
    SrsTsAdaptationFieldTypeReserved     = 0x00,
    SrsTsAdaptationFieldTypePayloadOnly  = 0x01,
    SrsTsAdaptationFieldTypeAdaptionOnly = 0x02,
    SrsTsAdaptationFieldTypeBoth         = 0x03,
};
enum SrsTsPidApply {
    SrsTsPidApplyReserved = 0,
    SrsTsPidApplyPAT,
    SrsTsPidApplyPMT,
    SrsTsPidApplyVideo,
    SrsTsPidApplyAudio,
};
enum SrsTsStream {
    SrsTsStreamReserved   = 0x00,
    SrsTsStreamAudioMp3   = 0x04,
    SrsTsStreamAudioAAC   = 0x0f,
    SrsTsStreamVideoMpeg4 = 0x10,
    SrsTsStreamAudioMpeg4 = 0x11,
    SrsTsStreamVideoH264  = 0x1b,
    SrsTsStreamAudioAC3   = 0x81,
    SrsTsStreamAudioDTS   = 0x8a,
};
std::string srs_ts_stream2string(SrsTsStream stream);
struct SrsTsChannel {
    int           pid;
    SrsTsPidApply apply;
    SrsTsStream   stream;
    SrsTsMessage* msg;
    SrsTsContext* context;
    u_int8_t      continuity_counter;
    SrsTsChannel();
    virtual ~SrsTsChannel();
};
enum SrsTsPESStreamId {
    SrsTsPESStreamIdProgramStreamMap       = 0xbc,
    SrsTsPESStreamIdPrivateStream1         = 0xbd,
    SrsTsPESStreamIdPaddingStream          = 0xbe,
    SrsTsPESStreamIdPrivateStream2         = 0xbf,
    SrsTsPESStreamIdAudioChecker           = 0x06,
    SrsTsPESStreamIdAudioCommon            = 0xc0,
    SrsTsPESStreamIdVideoChecker           = 0x0e,
    SrsTsPESStreamIdVideoCommon            = 0xe0,
    SrsTsPESStreamIdEcmStream              = 0xf0,
    SrsTsPESStreamIdEmmStream              = 0xf1,
    SrsTsPESStreamIdDsmccStream            = 0xf2,
    SrsTsPESStreamId13522Stream            = 0xf3,
    SrsTsPESStreamIdH2221TypeA             = 0xf4,
    SrsTsPESStreamIdH2221TypeB             = 0xf5,
    SrsTsPESStreamIdH2221TypeC             = 0xf6,
    SrsTsPESStreamIdH2221TypeD             = 0xf7,
    SrsTsPESStreamIdH2221TypeE             = 0xf8,
    SrsTsPESStreamIdAncillaryStream        = 0xf9,
    SrsTsPESStreamIdSlPacketizedStream     = 0xfa,
    SrsTsPESStreamIdFlexMuxStream          = 0xfb,
    SrsTsPESStreamIdProgramStreamDirectory = 0xff,
};
class SrsTsMessage
{
public:
    SrsTsChannel* channel;
    SrsTsPacket* packet;
public:
    int64_t start_pts;
    bool write_pcr;
    bool is_discontinuity;
public:
    int64_t dts;
    int64_t pts;
    SrsTsPESStreamId sid;
    u_int16_t PES_packet_length;
    u_int8_t continuity_counter;
    SrsSimpleBuffer* payload;
public:
    SrsTsMessage(SrsTsChannel* c = NULL, SrsTsPacket* p = NULL);
    virtual ~SrsTsMessage();
public:
    virtual int dump(SrsStream* stream, int* pnb_bytes);
    virtual bool completed(int8_t payload_unit_start_indicator);
    virtual bool fresh();
public:
    virtual bool is_audio();
    virtual bool is_video();
    virtual int stream_number();
public:
    virtual SrsTsMessage * detach();
};
class ISrsTsHandler
{
public:
    ISrsTsHandler();
    virtual ~ISrsTsHandler();
public:
    virtual int on_ts_message(SrsTsMessage* msg) = 0;
};
class SrsTsContext
{
private:
    std::map<int, SrsTsChannel *> pids;
    bool pure_audio;
private:
    SrsCodecVideo vcodec;
    SrsCodecAudio acodec;
public:
    SrsTsContext();
    virtual ~SrsTsContext();
public:
    virtual bool is_pure_audio();
    virtual void on_pmt_parsed();
    virtual void reset();
public:
    virtual SrsTsChannel * get(int pid);
    virtual void set(int pid, SrsTsPidApply apply_pid, SrsTsStream stream = SrsTsStreamReserved);
public:
    virtual int decode(SrsStream* stream, ISrsTsHandler* handler);
public:
    virtual int encode(SrsFileWriter* writer, SrsTsMessage* msg, SrsCodecVideo vc, SrsCodecAudio ac);
private:
    virtual int encode_pat_pmt(SrsFileWriter* writer, int16_t vpid, SrsTsStream vs, int16_t apid, SrsTsStream as);
    virtual int encode_pes(SrsFileWriter* writer, SrsTsMessage* msg, int16_t pid, SrsTsStream sid, bool pure_audio);
};
class SrsTsPacket
{
public:
    int8_t sync_byte;
    int8_t transport_error_indicator;
    int8_t payload_unit_start_indicator;
    int8_t transport_priority;
    SrsTsPid pid;
    SrsTsScrambled transport_scrambling_control;
    SrsTsAdaptationFieldType adaption_field_control;
    u_int8_t continuity_counter;
private:
    SrsTsAdaptationField* adaptation_field;
    SrsTsPayload* payload;
public:
    SrsTsContext* context;
public:
    SrsTsPacket(SrsTsContext* c);
    virtual ~SrsTsPacket();
public:
    virtual int decode(SrsStream* stream, SrsTsMessage** ppmsg);
public:
    virtual int size();
    virtual int encode(SrsStream* stream);
    virtual void padding(int nb_stuffings);
public:
    static SrsTsPacket * create_pat(SrsTsContext* context,
                                    int16_t pmt_number, int16_t pmt_pid
    );
    static SrsTsPacket * create_pmt(SrsTsContext* context,
                                    int16_t pmt_number, int16_t pmt_pid, int16_t vpid, SrsTsStream vs,
                                    int16_t apid, SrsTsStream as
    );
    static SrsTsPacket * create_pes_first(SrsTsContext* context,
                                          int16_t pid, SrsTsPESStreamId sid, u_int8_t continuity_counter,
                                          bool discontinuity,
                                          int64_t pcr, int64_t dts, int64_t pts, int size
    );
    static SrsTsPacket * create_pes_continue(SrsTsContext* context,
                                             int16_t pid, SrsTsPESStreamId sid, u_int8_t continuity_counter
    );
};
class SrsTsAdaptationField
{
public:
    u_int8_t adaption_field_length;
    int8_t discontinuity_indicator;
    int8_t random_access_indicator;
    int8_t elementary_stream_priority_indicator;
    int8_t PCR_flag;
    int8_t OPCR_flag;
    int8_t splicing_point_flag;
    int8_t transport_private_data_flag;
    int8_t adaptation_field_extension_flag;
    int64_t program_clock_reference_base;
    int8_t const1_value0;
    int16_t program_clock_reference_extension;
    int64_t original_program_clock_reference_base;
    int8_t const1_value2;
    int16_t original_program_clock_reference_extension;
    int8_t splice_countdown;
    u_int8_t transport_private_data_length;
    char* transport_private_data;
    u_int8_t adaptation_field_extension_length;
    int8_t ltw_flag;
    int8_t piecewise_rate_flag;
    int8_t seamless_splice_flag;
    int8_t const1_value1;
    int8_t ltw_valid_flag;
    int16_t ltw_offset;
    int32_t piecewise_rate;
    int8_t splice_type;
    int8_t DTS_next_AU0;
    int8_t marker_bit0;
    int16_t DTS_next_AU1;
    int8_t marker_bit1;
    int16_t DTS_next_AU2;
    int8_t marker_bit2;
    int nb_af_ext_reserved;
    int nb_af_reserved;
private:
    SrsTsPacket* packet;
public:
    SrsTsAdaptationField(SrsTsPacket* pkt);
    virtual ~SrsTsAdaptationField();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int size();
    virtual int encode(SrsStream* stream);
};
enum SrsTsPsiId {
    SrsTsPsiIdPas            = 0x00,
    SrsTsPsiIdCas            = 0x01,
    SrsTsPsiIdPms            = 0x02,
    SrsTsPsiIdDs             = 0x03,
    SrsTsPsiIdSds            = 0x04,
    SrsTsPsiIdOds            = 0x05,
    SrsTsPsiIdIso138181Start = 0x06,
    SrsTsPsiIdIso138181End   = 0x37,
    SrsTsPsiIdIso138186Start = 0x38,
    SrsTsPsiIdIso138186End   = 0x3F,
    SrsTsPsiIdUserStart      = 0x40,
    SrsTsPsiIdUserEnd        = 0xFE,
    SrsTsPsiIdForbidden      = 0xFF,
};
class SrsTsPayload
{
protected:
    SrsTsPacket* packet;
public:
    SrsTsPayload(SrsTsPacket* p);
    virtual ~SrsTsPayload();
public:
    virtual int decode(SrsStream* stream, SrsTsMessage** ppmsg) = 0;
public:
    virtual int size() = 0;
    virtual int encode(SrsStream* stream) = 0;
};
class SrsTsPayloadPES : public SrsTsPayload
{
public:
    int32_t packet_start_code_prefix;
    u_int8_t stream_id;
    u_int16_t PES_packet_length;
    int8_t const2bits;
    int8_t PES_scrambling_control;
    int8_t PES_priority;
    int8_t data_alignment_indicator;
    int8_t copyright;
    int8_t original_or_copy;
    int8_t PTS_DTS_flags;
    int8_t ESCR_flag;
    int8_t ES_rate_flag;
    int8_t DSM_trick_mode_flag;
    int8_t additional_copy_info_flag;
    int8_t PES_CRC_flag;
    int8_t PES_extension_flag;
    u_int8_t PES_header_data_length;
    int64_t pts;
    int64_t dts;
    int64_t ESCR_base;
    int16_t ESCR_extension;
    int32_t ES_rate;
    int8_t trick_mode_control;
    int8_t trick_mode_value;
    int8_t additional_copy_info;
    int16_t previous_PES_packet_CRC;
    int8_t PES_private_data_flag;
    int8_t pack_header_field_flag;
    int8_t program_packet_sequence_counter_flag;
    int8_t P_STD_buffer_flag;
    int8_t const1_value0;
    int8_t PES_extension_flag_2;
    char* PES_private_data;
    u_int8_t pack_field_length;
    char* pack_field;
    int8_t program_packet_sequence_counter;
    int8_t MPEG1_MPEG2_identifier;
    int8_t original_stuff_length;
    int8_t P_STD_buffer_scale;
    int16_t P_STD_buffer_size;
    u_int8_t PES_extension_field_length;
    char* PES_extension_field;
    int nb_stuffings;
    int nb_bytes;
    int nb_paddings;
public:
    SrsTsPayloadPES(SrsTsPacket* p);
    virtual ~SrsTsPayloadPES();
public:
    virtual int decode(SrsStream* stream, SrsTsMessage** ppmsg);
public:
    virtual int size();
    virtual int encode(SrsStream* stream);
private:
    virtual int decode_33bits_dts_pts(SrsStream* stream, int64_t* pv);
    virtual int encode_33bits_dts_pts(SrsStream* stream, u_int8_t fb, int64_t v);
};
class SrsTsPayloadPSI : public SrsTsPayload
{
public:
    int8_t pointer_field;
public:
    SrsTsPsiId table_id;
    int8_t section_syntax_indicator;
    int8_t const0_value;
    int8_t const1_value;
    u_int16_t section_length;
public:
public:
    int32_t CRC_32;
public:
    SrsTsPayloadPSI(SrsTsPacket* p);
    virtual ~SrsTsPayloadPSI();
public:
    virtual int decode(SrsStream* stream, SrsTsMessage** ppmsg);
public:
    virtual int size();
    virtual int encode(SrsStream* stream);
protected:
    virtual int psi_size() = 0;
    virtual int psi_encode(SrsStream* stream) = 0;
    virtual int psi_decode(SrsStream* stream) = 0;
};
class SrsTsPayloadPATProgram
{
public:
    int16_t number;
    int8_t const1_value;
    int16_t pid;
public:
    SrsTsPayloadPATProgram(int16_t n = 0, int16_t p = 0);
    virtual ~SrsTsPayloadPATProgram();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int size();
    virtual int encode(SrsStream* stream);
};
class SrsTsPayloadPAT : public SrsTsPayloadPSI
{
public:
    u_int16_t transport_stream_id;
    int8_t const3_value;
    int8_t version_number;
    int8_t current_next_indicator;
    u_int8_t section_number;
    u_int8_t last_section_number;
    std::vector<SrsTsPayloadPATProgram *> programs;
public:
    SrsTsPayloadPAT(SrsTsPacket* p);
    virtual ~SrsTsPayloadPAT();
protected:
    virtual int psi_decode(SrsStream* stream);
protected:
    virtual int psi_size();
    virtual int psi_encode(SrsStream* stream);
};
class SrsTsPayloadPMTESInfo
{
public:
    SrsTsStream stream_type;
    int8_t const1_value0;
    int16_t elementary_PID;
    int8_t const1_value1;
    int16_t ES_info_length;
    char* ES_info;
public:
    SrsTsPayloadPMTESInfo(SrsTsStream st = SrsTsStreamReserved, int16_t epid = 0);
    virtual ~SrsTsPayloadPMTESInfo();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int size();
    virtual int encode(SrsStream* stream);
};
class SrsTsPayloadPMT : public SrsTsPayloadPSI
{
public:
    u_int16_t program_number;
    int8_t const1_value0;
    int8_t version_number;
    int8_t current_next_indicator;
    u_int8_t section_number;
    u_int8_t last_section_number;
    int8_t const1_value1;
    int16_t PCR_PID;
    int8_t const1_value2;
    u_int16_t program_info_length;
    char* program_info_desc;
    std::vector<SrsTsPayloadPMTESInfo *> infos;
public:
    SrsTsPayloadPMT(SrsTsPacket* p);
    virtual ~SrsTsPayloadPMT();
protected:
    virtual int psi_decode(SrsStream* stream);
protected:
    virtual int psi_size();
    virtual int psi_encode(SrsStream* stream);
};
class SrsTSMuxer
{
private:
    SrsCodecVideo vcodec;
    SrsCodecAudio acodec;
private:
    SrsTsContext* context;
    SrsFileWriter* writer;
    std::string path;
public:
    SrsTSMuxer(SrsFileWriter* w, SrsTsContext* c, SrsCodecAudio ac, SrsCodecVideo vc);
    virtual ~SrsTSMuxer();
public:
    virtual int open(std::string p);
    virtual int update_acodec(SrsCodecAudio ac);
    virtual int write_audio(SrsTsMessage* audio);
    virtual int write_video(SrsTsMessage* video);
    virtual void close();
public:
    virtual SrsCodecVideo video_codec();
};
class SrsTsCache
{
public:
    SrsTsMessage* audio;
    SrsTsMessage* video;
public:
    SrsTsCache();
    virtual ~SrsTsCache();
public:
    virtual int cache_audio(SrsAvcAacCodec* codec, int64_t dts, SrsCodecSample* sample);
    virtual int cache_video(SrsAvcAacCodec* codec, int64_t dts, SrsCodecSample* sample);
private:
    virtual int do_cache_mp3(SrsAvcAacCodec* codec, SrsCodecSample* sample);
    virtual int do_cache_aac(SrsAvcAacCodec* codec, SrsCodecSample* sample);
    virtual int do_cache_avc(SrsAvcAacCodec* codec, SrsCodecSample* sample);
};
class SrsTsEncoder
{
private:
    SrsFileWriter* writer;
private:
    SrsAvcAacCodec* codec;
    SrsCodecSample* sample;
    SrsTsCache* cache;
    SrsTSMuxer* muxer;
    SrsTsContext* context;
public:
    SrsTsEncoder();
    virtual ~SrsTsEncoder();
public:
    virtual int initialize(SrsFileWriter* fw);
public:
    virtual int write_audio(int64_t timestamp, char* data, int size);
    virtual int write_video(int64_t timestamp, char* data, int size);
private:
    virtual int flush_audio();
    virtual int flush_video();
};
# endif // if !defined(SRS_EXPORT_LIBRTMP)
#endif  // ifndef SRS_KERNEL_TS_HPP
#ifndef SRS_KERNEL_BUFFER_HPP
# define SRS_KERNEL_BUFFER_HPP
# include <vector>
class SrsSimpleBuffer
{
private:
    std::vector<char> data;
public:
    SrsSimpleBuffer();
    virtual ~SrsSimpleBuffer();
public:
    virtual int length();
    virtual char * bytes();
    virtual void erase(int size);
    virtual void append(const char* bytes, int size);
};
#endif // ifndef SRS_KERNEL_BUFFER_HPP
#ifndef SRS_RTMP_PROTOCOL_AMF0_HPP
# define SRS_RTMP_PROTOCOL_AMF0_HPP
# include <string>
# include <vector>
class SrsStream;
class SrsAmf0Object;
class SrsAmf0EcmaArray;
class SrsAmf0StrictArray;
namespace _srs_internal
{
class SrsUnSortedHashtable;
class SrsAmf0ObjectEOF;
class SrsAmf0Date;
}
class SrsAmf0Any
{
public:
    char marker;
public:
    SrsAmf0Any();
    virtual ~SrsAmf0Any();
public:
    virtual bool is_string();
    virtual bool is_boolean();
    virtual bool is_number();
    virtual bool is_null();
    virtual bool is_undefined();
    virtual bool is_object();
    virtual bool is_object_eof();
    virtual bool is_ecma_array();
    virtual bool is_strict_array();
    virtual bool is_date();
    virtual bool is_complex_object();
public:
    virtual std::string to_str();
    virtual const char * to_str_raw();
    virtual bool to_boolean();
    virtual double to_number();
    virtual int64_t to_date();
    virtual int16_t to_date_time_zone();
    virtual SrsAmf0Object * to_object();
    virtual SrsAmf0EcmaArray * to_ecma_array();
    virtual SrsAmf0StrictArray * to_strict_array();
public:
    virtual void set_number(double value);
public:
    virtual int total_size() = 0;
    virtual int read(SrsStream* stream)  = 0;
    virtual int write(SrsStream* stream) = 0;
    virtual SrsAmf0Any * copy() = 0;
    virtual char * human_print(char** pdata, int* psize);
public:
    static SrsAmf0Any * str(const char* value = NULL);
    static SrsAmf0Any * boolean(bool value    = false);
    static SrsAmf0Any * number(double value   = 0.0);
    static SrsAmf0Any * date(int64_t value    = 0);
    static SrsAmf0Any * null();
    static SrsAmf0Any * undefined();
    static SrsAmf0Object * object();
    static SrsAmf0Any * object_eof();
    static SrsAmf0EcmaArray * ecma_array();
    static SrsAmf0StrictArray * strict_array();
public:
    static int discovery(SrsStream* stream, SrsAmf0Any** ppvalue);
};
class SrsAmf0Object : public SrsAmf0Any
{
private:
    _srs_internal::SrsUnSortedHashtable* properties;
    _srs_internal::SrsAmf0ObjectEOF* eof;
private:
    friend class SrsAmf0Any;
    SrsAmf0Object();
public:
    virtual ~SrsAmf0Object();
public:
    virtual int total_size();
    virtual int read(SrsStream* stream);
    virtual int write(SrsStream* stream);
    virtual SrsAmf0Any * copy();
public:
    virtual void clear();
    virtual int count();
    virtual std::string key_at(int index);
    virtual const char * key_raw_at(int index);
    virtual SrsAmf0Any * value_at(int index);
public:
    virtual void set(std::string key, SrsAmf0Any* value);
    virtual SrsAmf0Any * get_property(std::string name);
    virtual SrsAmf0Any * ensure_property_string(std::string name);
    virtual SrsAmf0Any * ensure_property_number(std::string name);
    virtual void remove(std::string name);
};
class SrsAmf0EcmaArray : public SrsAmf0Any
{
private:
    _srs_internal::SrsUnSortedHashtable* properties;
    _srs_internal::SrsAmf0ObjectEOF* eof;
    int32_t _count;
private:
    friend class SrsAmf0Any;
    SrsAmf0EcmaArray();
public:
    virtual ~SrsAmf0EcmaArray();
public:
    virtual int total_size();
    virtual int read(SrsStream* stream);
    virtual int write(SrsStream* stream);
    virtual SrsAmf0Any * copy();
public:
    virtual void clear();
    virtual int count();
    virtual std::string key_at(int index);
    virtual const char * key_raw_at(int index);
    virtual SrsAmf0Any * value_at(int index);
public:
    virtual void set(std::string key, SrsAmf0Any* value);
    virtual SrsAmf0Any * get_property(std::string name);
    virtual SrsAmf0Any * ensure_property_string(std::string name);
    virtual SrsAmf0Any * ensure_property_number(std::string name);
};
class SrsAmf0StrictArray : public SrsAmf0Any
{
private:
    std::vector<SrsAmf0Any *> properties;
    int32_t _count;
private:
    friend class SrsAmf0Any;
    SrsAmf0StrictArray();
public:
    virtual ~SrsAmf0StrictArray();
public:
    virtual int total_size();
    virtual int read(SrsStream* stream);
    virtual int write(SrsStream* stream);
    virtual SrsAmf0Any * copy();
public:
    virtual void clear();
    virtual int count();
    virtual SrsAmf0Any * at(int index);
public:
    virtual void append(SrsAmf0Any* any);
};
class SrsAmf0Size
{
public:
    static int utf8(std::string value);
    static int str(std::string value);
    static int number();
    static int date();
    static int null();
    static int undefined();
    static int boolean();
    static int object(SrsAmf0Object* obj);
    static int object_eof();
    static int ecma_array(SrsAmf0EcmaArray* arr);
    static int strict_array(SrsAmf0StrictArray* arr);
    static int any(SrsAmf0Any* o);
};
extern int srs_amf0_read_any(SrsStream* stream, SrsAmf0Any** ppvalue);
extern int srs_amf0_read_string(SrsStream* stream, std::string& value);
extern int srs_amf0_write_string(SrsStream* stream, std::string value);
extern int srs_amf0_read_boolean(SrsStream* stream, bool& value);
extern int srs_amf0_write_boolean(SrsStream* stream, bool value);
extern int srs_amf0_read_number(SrsStream* stream, double& value);
extern int srs_amf0_write_number(SrsStream* stream, double value);
extern int srs_amf0_read_null(SrsStream* stream);
extern int srs_amf0_write_null(SrsStream* stream);
extern int srs_amf0_read_undefined(SrsStream* stream);
extern int srs_amf0_write_undefined(SrsStream* stream);
namespace _srs_internal
{
class SrsAmf0String : public SrsAmf0Any
{
public:
    std::string value;
private:
    friend class SrsAmf0Any;
    SrsAmf0String(const char* _value);
public:
    virtual ~SrsAmf0String();
public:
    virtual int total_size();
    virtual int read(SrsStream* stream);
    virtual int write(SrsStream* stream);
    virtual SrsAmf0Any * copy();
};
class SrsAmf0Boolean : public SrsAmf0Any
{
public:
    bool value;
private:
    friend class SrsAmf0Any;
    SrsAmf0Boolean(bool _value);
public:
    virtual ~SrsAmf0Boolean();
public:
    virtual int total_size();
    virtual int read(SrsStream* stream);
    virtual int write(SrsStream* stream);
    virtual SrsAmf0Any * copy();
};
class SrsAmf0Number : public SrsAmf0Any
{
public:
    double value;
private:
    friend class SrsAmf0Any;
    SrsAmf0Number(double _value);
public:
    virtual ~SrsAmf0Number();
public:
    virtual int total_size();
    virtual int read(SrsStream* stream);
    virtual int write(SrsStream* stream);
    virtual SrsAmf0Any * copy();
};
class SrsAmf0Date : public SrsAmf0Any
{
private:
    int64_t _date_value;
    int16_t _time_zone;
private:
    friend class SrsAmf0Any;
    SrsAmf0Date(int64_t value);
public:
    virtual ~SrsAmf0Date();
public:
    virtual int total_size();
    virtual int read(SrsStream* stream);
    virtual int write(SrsStream* stream);
    virtual SrsAmf0Any * copy();
public:
    virtual int64_t date();
    virtual int16_t time_zone();
};
class SrsAmf0Null : public SrsAmf0Any
{
private:
    friend class SrsAmf0Any;
    SrsAmf0Null();
public:
    virtual ~SrsAmf0Null();
public:
    virtual int total_size();
    virtual int read(SrsStream* stream);
    virtual int write(SrsStream* stream);
    virtual SrsAmf0Any * copy();
};
class SrsAmf0Undefined : public SrsAmf0Any
{
private:
    friend class SrsAmf0Any;
    SrsAmf0Undefined();
public:
    virtual ~SrsAmf0Undefined();
public:
    virtual int total_size();
    virtual int read(SrsStream* stream);
    virtual int write(SrsStream* stream);
    virtual SrsAmf0Any * copy();
};
class SrsUnSortedHashtable
{
private:
    typedef std::pair<std::string, SrsAmf0Any *> SrsAmf0ObjectPropertyType;
    std::vector<SrsAmf0ObjectPropertyType> properties;
public:
    SrsUnSortedHashtable();
    virtual ~SrsUnSortedHashtable();
public:
    virtual int count();
    virtual void clear();
    virtual std::string key_at(int index);
    virtual const char * key_raw_at(int index);
    virtual SrsAmf0Any * value_at(int index);
    virtual void set(std::string key, SrsAmf0Any* value);
public:
    virtual SrsAmf0Any * get_property(std::string name);
    virtual SrsAmf0Any * ensure_property_string(std::string name);
    virtual SrsAmf0Any * ensure_property_number(std::string name);
    virtual void remove(std::string name);
public:
    virtual void copy(SrsUnSortedHashtable* src);
};
class SrsAmf0ObjectEOF : public SrsAmf0Any
{
public:
    SrsAmf0ObjectEOF();
    virtual ~SrsAmf0ObjectEOF();
public:
    virtual int total_size();
    virtual int read(SrsStream* stream);
    virtual int write(SrsStream* stream);
    virtual SrsAmf0Any * copy();
};
extern int srs_amf0_read_utf8(SrsStream* stream, std::string& value);
extern int srs_amf0_write_utf8(SrsStream* stream, std::string value);
extern bool srs_amf0_is_object_eof(SrsStream* stream);
extern int srs_amf0_write_object_eof(SrsStream* stream, SrsAmf0ObjectEOF* value);
extern int srs_amf0_write_any(SrsStream* stream, SrsAmf0Any* value);
};
#endif // ifndef SRS_RTMP_PROTOCOL_AMF0_HPP
#ifndef SRS_RTMP_PROTOCOL_STACK_HPP
# define SRS_RTMP_PROTOCOL_STACK_HPP
# include <map>
# include <vector>
# include <string>
# ifndef _WIN32
#  include <sys/uio.h>
# endif
class ISrsProtocolReaderWriter;
class SrsFastBuffer;
class SrsPacket;
class SrsStream;
class SrsAmf0Object;
class SrsAmf0Any;
class SrsMessageHeader;
class SrsCommonMessage;
class SrsChunkStream;
class SrsSharedPtrMessage;
class IMergeReadHandler;
class SrsProtocol;
class ISrsProtocolReaderWriter;
class SrsCommonMessage;
class SrsCreateStreamPacket;
class SrsFMLEStartPacket;
class SrsPublishPacket;
class SrsOnMetaDataPacket;
class SrsPlayPacket;
class SrsCommonMessage;
class SrsPacket;
class SrsAmf0Object;
class IMergeReadHandler;
# define RTMP_AMF0_COMMAND_CONNECT        "connect"
# define RTMP_AMF0_COMMAND_CREATE_STREAM  "createStream"
# define RTMP_AMF0_COMMAND_CLOSE_STREAM   "closeStream"
# define RTMP_AMF0_COMMAND_PLAY           "play"
# define RTMP_AMF0_COMMAND_PAUSE          "pause"
# define RTMP_AMF0_COMMAND_ON_BW_DONE     "onBWDone"
# define RTMP_AMF0_COMMAND_ON_STATUS      "onStatus"
# define RTMP_AMF0_COMMAND_RESULT         "_result"
# define RTMP_AMF0_COMMAND_ERROR          "_error"
# define RTMP_AMF0_COMMAND_RELEASE_STREAM "releaseStream"
# define RTMP_AMF0_COMMAND_FC_PUBLISH     "FCPublish"
# define RTMP_AMF0_COMMAND_UNPUBLISH      "FCUnpublish"
# define RTMP_AMF0_COMMAND_PUBLISH        "publish"
# define RTMP_AMF0_DATA_SAMPLE_ACCESS     "|RtmpSampleAccess"
# define RTMP_SIG_FMS_VER                 "3,5,3,888"
# define RTMP_SIG_AMF0_VER                0
# define RTMP_SIG_CLIENT_ID               "ASAICiss"
# define StatusLevel                      "level"
# define StatusCode                       "code"
# define StatusDescription                "description"
# define StatusDetails                    "details"
# define StatusClientId                   "clientid"
# define StatusLevelStatus                "status"
# define StatusLevelError                 "error"
# define StatusCodeConnectSuccess         "NetConnection.Connect.Success"
# define StatusCodeConnectRejected        "NetConnection.Connect.Rejected"
# define StatusCodeStreamReset            "NetStream.Play.Reset"
# define StatusCodeStreamStart            "NetStream.Play.Start"
# define StatusCodeStreamPause            "NetStream.Pause.Notify"
# define StatusCodeStreamUnpause          "NetStream.Unpause.Notify"
# define StatusCodePublishStart           "NetStream.Publish.Start"
# define StatusCodeDataStart              "NetStream.Data.Start"
# define StatusCodeUnpublishSuccess       "NetStream.Unpublish.Success"
class SrsPacket
{
public:
    SrsPacket();
    virtual ~SrsPacket();
public:
    virtual int encode(int& size, char *& payload);
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsProtocol
{
private:
    class AckWindowSize
    {
public:
        int ack_window_size;
        int64_t acked_size;
        AckWindowSize();
    };
private:
    ISrsProtocolReaderWriter* skt;
    std::map<double, std::string> requests;
private:
    std::map<int, SrsChunkStream *> chunk_streams;
    SrsChunkStream** cs_cache;
    SrsFastBuffer* in_buffer;
    int32_t in_chunk_size;
    AckWindowSize in_ack_size;
    bool auto_response_when_recv;
    std::vector<SrsPacket *> manual_response_queue;
private:
    iovec* out_iovs;
    int nb_out_iovs;
    char out_c0c3_caches[SRS_CONSTS_C0C3_HEADERS_MAX];
    bool warned_c0c3_cache_dry;
    int32_t out_chunk_size;
public:
    SrsProtocol(ISrsProtocolReaderWriter* io);
    virtual ~SrsProtocol();
public:
    virtual void set_auto_response(bool v);
    virtual int manual_response_flush();
public:
    # ifdef SRS_PERF_MERGED_READ
    virtual void set_merge_read(bool v, IMergeReadHandler* handler);
    virtual void set_recv_buffer(int buffer_size);
    # endif
public:
    virtual void set_recv_timeout(int64_t timeout_us);
    virtual int64_t get_recv_timeout();
    virtual void set_send_timeout(int64_t timeout_us);
    virtual int64_t get_send_timeout();
    virtual int64_t get_recv_bytes();
    virtual int64_t get_send_bytes();
public:
    virtual int recv_message(SrsCommonMessage** pmsg);
    virtual int decode_message(SrsCommonMessage* msg, SrsPacket** ppacket);
    virtual int send_and_free_message(SrsSharedPtrMessage* msg, int stream_id);
    virtual int send_and_free_messages(SrsSharedPtrMessage** msgs, int nb_msgs, int stream_id);
    virtual int send_and_free_packet(SrsPacket* packet, int stream_id);
public:
    template <class T>
    int expect_message(SrsCommonMessage** pmsg, T** ppacket)
    {
        *pmsg    = NULL;
        *ppacket = NULL;
        int ret = ERROR_SUCCESS;
        while (true) {
            SrsCommonMessage* msg = NULL;
            if ((ret = recv_message(&msg)) != ERROR_SUCCESS) {
                if (ret != ERROR_SOCKET_TIMEOUT && !srs_is_client_gracefully_close(ret)) {
                    srs_error("recv message failed. ret=%d", ret);
                }
                return ret;
            }
            srs_verbose("recv message success.");
            SrsPacket* packet = NULL;
            if ((ret = decode_message(msg, &packet)) != ERROR_SUCCESS) {
                srs_error("decode message failed. ret=%d", ret);
                srs_freep(msg);
                srs_freep(packet);
                return ret;
            }
            T* pkt = dynamic_cast<T *>(packet);
            if (!pkt) {
                srs_info("drop message(type=%d, size=%d, time=%" PRId64 ", sid=%d).",
                         msg->header.message_type, msg->header.payload_length,
                         msg->header.timestamp, msg->header.stream_id);
                srs_freep(msg);
                srs_freep(packet);
                continue;
            }
            *pmsg    = msg;
            *ppacket = pkt;
            break;
        }
        return ret;
    } // expect_message

private:
    virtual int do_send_messages(SrsSharedPtrMessage** msgs, int nb_msgs);
    virtual int do_iovs_send(iovec* iovs, int size);
    virtual int do_send_and_free_packet(SrsPacket* packet, int stream_id);
    virtual int do_simple_send(SrsMessageHeader* mh, char* payload, int size);
    virtual int do_decode_message(SrsMessageHeader& header, SrsStream* stream, SrsPacket** ppacket);
    virtual int recv_interlaced_message(SrsCommonMessage** pmsg);
    virtual int read_basic_header(char& fmt, int& cid);
    virtual int read_message_header(SrsChunkStream* chunk, char fmt);
    virtual int read_message_payload(SrsChunkStream* chunk, SrsCommonMessage** pmsg);
    virtual int on_recv_message(SrsCommonMessage* msg);
    virtual int on_send_packet(SrsMessageHeader* mh, SrsPacket* packet);
private:
    virtual int response_acknowledgement_message();
    virtual int response_ping_message(int32_t timestamp);
};
class SrsChunkStream
{
public:
    char fmt;
    int cid;
    SrsMessageHeader header;
    bool extended_timestamp;
    SrsCommonMessage* msg;
    int64_t msg_count;
public:
    SrsChunkStream(int _cid);
    virtual ~SrsChunkStream();
};
class SrsRequest
{
public:
    std::string ip;
public:
    std::string tcUrl;
    std::string pageUrl;
    std::string swfUrl;
    double objectEncoding;
public:
    std::string schema;
    std::string vhost;
    std::string host;
    std::string port;
    std::string app;
    std::string param;
    std::string stream;
    double duration;
    SrsAmf0Object* args;
public:
    SrsRequest();
    virtual ~SrsRequest();
public:
    virtual SrsRequest * copy();
    virtual void update_auth(SrsRequest* req);
    virtual std::string get_stream_url();
    virtual void strip();
};
class SrsResponse
{
public:
    int stream_id;
public:
    SrsResponse();
    virtual ~SrsResponse();
};
enum SrsRtmpConnType {
    SrsRtmpConnUnknown,
    SrsRtmpConnPlay,
    SrsRtmpConnFMLEPublish,
    SrsRtmpConnFlashPublish,
};
std::string srs_client_type_string(SrsRtmpConnType type);
bool srs_client_type_is_publish(SrsRtmpConnType type);
class SrsHandshakeBytes
{
public:
    char* c0c1;
    char* s0s1s2;
    char* c2;
public:
    SrsHandshakeBytes();
    virtual ~SrsHandshakeBytes();
public:
    virtual int read_c0c1(ISrsProtocolReaderWriter* io);
    virtual int read_s0s1s2(ISrsProtocolReaderWriter* io);
    virtual int read_c2(ISrsProtocolReaderWriter* io);
    virtual int create_c0c1();
    virtual int create_s0s1s2(const char* c1 = NULL);
    virtual int create_c2();
};
class SrsRtmpClient
{
private:
    SrsHandshakeBytes* hs_bytes;
protected:
    SrsProtocol* protocol;
    ISrsProtocolReaderWriter* io;
public:
    SrsRtmpClient(ISrsProtocolReaderWriter* skt);
    virtual ~SrsRtmpClient();
public:
    virtual void set_recv_timeout(int64_t timeout_us);
    virtual void set_send_timeout(int64_t timeout_us);
    virtual int64_t get_recv_bytes();
    virtual int64_t get_send_bytes();
    virtual int recv_message(SrsCommonMessage** pmsg);
    virtual int decode_message(SrsCommonMessage* msg, SrsPacket** ppacket);
    virtual int send_and_free_message(SrsSharedPtrMessage* msg, int stream_id);
    virtual int send_and_free_messages(SrsSharedPtrMessage** msgs, int nb_msgs, int stream_id);
    virtual int send_and_free_packet(SrsPacket* packet, int stream_id);
public:
    virtual int handshake();
    virtual int simple_handshake();
    virtual int complex_handshake();
    virtual int connect_app(std::string app, std::string tc_url, SrsRequest* req, bool debug_srs_upnode);
    virtual int connect_app2(
        std::string app, std::string tc_url, SrsRequest* req, bool debug_srs_upnode,
        std::string& srs_server_ip, std::string& srs_server, std::string& srs_primary,
        std::string& srs_authors, std::string& srs_version, int& srs_id,
        int& srs_pid
    );
    virtual int create_stream(int& stream_id);
    virtual int play(std::string stream, int stream_id);
    virtual int publish(std::string stream, int stream_id);
    virtual int fmle_publish(std::string stream, int& stream_id);
public:
    template <class T>
    int expect_message(SrsCommonMessage** pmsg, T** ppacket)
    {
        return protocol->expect_message<T>(pmsg, ppacket);
    }
};
class SrsRtmpServer
{
private:
    SrsHandshakeBytes* hs_bytes;
    SrsProtocol* protocol;
    ISrsProtocolReaderWriter* io;
public:
    SrsRtmpServer(ISrsProtocolReaderWriter* skt);
    virtual ~SrsRtmpServer();
public:
    virtual void set_auto_response(bool v);
    # ifdef SRS_PERF_MERGED_READ
    virtual void set_merge_read(bool v, IMergeReadHandler* handler);
    virtual void set_recv_buffer(int buffer_size);
    # endif
    virtual void set_recv_timeout(int64_t timeout_us);
    virtual int64_t get_recv_timeout();
    virtual void set_send_timeout(int64_t timeout_us);
    virtual int64_t get_send_timeout();
    virtual int64_t get_recv_bytes();
    virtual int64_t get_send_bytes();
    virtual int recv_message(SrsCommonMessage** pmsg);
    virtual int decode_message(SrsCommonMessage* msg, SrsPacket** ppacket);
    virtual int send_and_free_message(SrsSharedPtrMessage* msg, int stream_id);
    virtual int send_and_free_messages(SrsSharedPtrMessage** msgs, int nb_msgs, int stream_id);
    virtual int send_and_free_packet(SrsPacket* packet, int stream_id);
public:
    virtual int handshake();
    virtual int connect_app(SrsRequest* req);
    virtual int set_window_ack_size(int ack_size);
    virtual int set_peer_bandwidth(int bandwidth, int type);
    virtual int response_connect_app(SrsRequest* req, const char* server_ip = NULL);
    virtual void response_connect_reject(SrsRequest* req, const char* desc);
    virtual int on_bw_done();
    virtual int identify_client(int stream_id, SrsRtmpConnType& type, std::string& stream_name, double& duration);
    virtual int set_chunk_size(int chunk_size);
    virtual int start_play(int stream_id);
    virtual int on_play_client_pause(int stream_id, bool is_pause);
    virtual int start_fmle_publish(int stream_id);
    virtual int fmle_unpublish(int stream_id, double unpublish_tid);
    virtual int start_flash_publish(int stream_id);
public:
    template <class T>
    int expect_message(SrsCommonMessage** pmsg, T** ppacket)
    {
        return protocol->expect_message<T>(pmsg, ppacket);
    }

private:
    virtual int identify_create_stream_client(SrsCreateStreamPacket* req, int stream_id, SrsRtmpConnType& type,
                                              std::string& stream_name, double& duration);
    virtual int identify_fmle_publish_client(SrsFMLEStartPacket* req, SrsRtmpConnType& type, std::string& stream_name);
    virtual int identify_flash_publish_client(SrsPublishPacket* req, SrsRtmpConnType& type, std::string& stream_name);
private:
    virtual int identify_play_client(SrsPlayPacket* req, SrsRtmpConnType& type, std::string& stream_name,
                                     double& duration);
};
class SrsConnectAppPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Object* command_object;
    SrsAmf0Object* args;
public:
    SrsConnectAppPacket();
    virtual ~SrsConnectAppPacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsConnectAppResPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Object* props;
    SrsAmf0Object* info;
public:
    SrsConnectAppResPacket();
    virtual ~SrsConnectAppResPacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsCallPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* command_object;
    SrsAmf0Any* arguments;
public:
    SrsCallPacket();
    virtual ~SrsCallPacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsCallResPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* command_object;
    SrsAmf0Any* response;
public:
    SrsCallResPacket(double _transaction_id);
    virtual ~SrsCallResPacket();
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsCreateStreamPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* command_object;
public:
    SrsCreateStreamPacket();
    virtual ~SrsCreateStreamPacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsCreateStreamResPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* command_object;
    double stream_id;
public:
    SrsCreateStreamResPacket(double _transaction_id, double _stream_id);
    virtual ~SrsCreateStreamResPacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsCloseStreamPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* command_object;
public:
    SrsCloseStreamPacket();
    virtual ~SrsCloseStreamPacket();
public:
    virtual int decode(SrsStream* stream);
};
class SrsFMLEStartPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* command_object;
    std::string stream_name;
public:
    SrsFMLEStartPacket();
    virtual ~SrsFMLEStartPacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
public:
    static SrsFMLEStartPacket * create_release_stream(std::string stream);
    static SrsFMLEStartPacket * create_FC_publish(std::string stream);
};
class SrsFMLEStartResPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* command_object;
    SrsAmf0Any* args;
public:
    SrsFMLEStartResPacket(double _transaction_id);
    virtual ~SrsFMLEStartResPacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsPublishPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* command_object;
    std::string stream_name;
    std::string type;
public:
    SrsPublishPacket();
    virtual ~SrsPublishPacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsPausePacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* command_object;
    bool is_pause;
    double time_ms;
public:
    SrsPausePacket();
    virtual ~SrsPausePacket();
public:
    virtual int decode(SrsStream* stream);
};
class SrsPlayPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* command_object;
    std::string stream_name;
    double start;
    double duration;
    bool reset;
public:
    SrsPlayPacket();
    virtual ~SrsPlayPacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsPlayResPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* command_object;
    SrsAmf0Object* desc;
public:
    SrsPlayResPacket();
    virtual ~SrsPlayResPacket();
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsOnBWDonePacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* args;
public:
    SrsOnBWDonePacket();
    virtual ~SrsOnBWDonePacket();
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsOnStatusCallPacket : public SrsPacket
{
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* args;
    SrsAmf0Object* data;
public:
    SrsOnStatusCallPacket();
    virtual ~SrsOnStatusCallPacket();
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsBandwidthPacket : public SrsPacket
{
private:
    disable_default_copy(SrsBandwidthPacket);
public:
    std::string command_name;
    double transaction_id;
    SrsAmf0Any* args;
    SrsAmf0Object* data;
public:
    SrsBandwidthPacket();
    virtual ~SrsBandwidthPacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
public:
    virtual bool is_start_play();
    virtual bool is_starting_play();
    virtual bool is_stop_play();
    virtual bool is_stopped_play();
    virtual bool is_start_publish();
    virtual bool is_starting_publish();
    virtual bool is_stop_publish();
    virtual bool is_stopped_publish();
    virtual bool is_finish();
    virtual bool is_final();
    static SrsBandwidthPacket * create_start_play();
    static SrsBandwidthPacket * create_starting_play();
    static SrsBandwidthPacket * create_playing();
    static SrsBandwidthPacket * create_stop_play();
    static SrsBandwidthPacket * create_stopped_play();
    static SrsBandwidthPacket * create_start_publish();
    static SrsBandwidthPacket * create_starting_publish();
    static SrsBandwidthPacket * create_publishing();
    static SrsBandwidthPacket * create_stop_publish();
    static SrsBandwidthPacket * create_stopped_publish();
    static SrsBandwidthPacket * create_finish();
    static SrsBandwidthPacket * create_final();
private:
    virtual SrsBandwidthPacket * set_command(std::string command);
};
class SrsOnStatusDataPacket : public SrsPacket
{
public:
    std::string command_name;
    SrsAmf0Object* data;
public:
    SrsOnStatusDataPacket();
    virtual ~SrsOnStatusDataPacket();
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsSampleAccessPacket : public SrsPacket
{
public:
    std::string command_name;
    bool video_sample_access;
    bool audio_sample_access;
public:
    SrsSampleAccessPacket();
    virtual ~SrsSampleAccessPacket();
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsOnMetaDataPacket : public SrsPacket
{
public:
    std::string name;
    SrsAmf0Object* metadata;
public:
    SrsOnMetaDataPacket();
    virtual ~SrsOnMetaDataPacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsSetWindowAckSizePacket : public SrsPacket
{
public:
    int32_t ackowledgement_window_size;
public:
    SrsSetWindowAckSizePacket();
    virtual ~SrsSetWindowAckSizePacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsAcknowledgementPacket : public SrsPacket
{
public:
    int32_t sequence_number;
public:
    SrsAcknowledgementPacket();
    virtual ~SrsAcknowledgementPacket();
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
class SrsSetChunkSizePacket : public SrsPacket
{
public:
    int32_t chunk_size;
public:
    SrsSetChunkSizePacket();
    virtual ~SrsSetChunkSizePacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
enum SrsPeerBandwidthType {
    SrsPeerBandwidthHard    = 0,
    SrsPeerBandwidthSoft    = 1,
    SrsPeerBandwidthDynamic = 2,
};
class SrsSetPeerBandwidthPacket : public SrsPacket
{
public:
    int32_t bandwidth;
    int8_t type;
public:
    SrsSetPeerBandwidthPacket();
    virtual ~SrsSetPeerBandwidthPacket();
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
enum SrcPCUCEventType {
    SrcPCUCStreamBegin      = 0x00,
    SrcPCUCStreamEOF        = 0x01,
    SrcPCUCStreamDry        = 0x02,
    SrcPCUCSetBufferLength  = 0x03,
    SrcPCUCStreamIsRecorded = 0x04,
    SrcPCUCPingRequest      = 0x06,
    SrcPCUCPingResponse     = 0x07,
    SrsPCUCFmsEvent0        = 0x1a,
};
class SrsUserControlPacket : public SrsPacket
{
public:
    int16_t event_type;
    int32_t event_data;
    int32_t extra_data;
public:
    SrsUserControlPacket();
    virtual ~SrsUserControlPacket();
public:
    virtual int decode(SrsStream* stream);
public:
    virtual int get_prefer_cid();
    virtual int get_message_type();
protected:
    virtual int get_size();
    virtual int encode_packet(SrsStream* stream);
};
#endif // ifndef SRS_RTMP_PROTOCOL_STACK_HPP
#ifndef SRS_RTMP_PROTOCOL_HANDSHKAE_HPP
# define SRS_RTMP_PROTOCOL_HANDSHKAE_HPP
class ISrsProtocolReaderWriter;
class SrsComplexHandshake;
class SrsHandshakeBytes;
class SrsStream;
# ifdef SRS_AUTO_SSL
#  include <openssl/hmac.h>
namespace _srs_internal
{
#  define SRS_OpensslHashSize 512
extern u_int8_t SrsGenuineFMSKey[];
extern u_int8_t SrsGenuineFPKey[];
int openssl_HMACsha256(const void* key, int key_size, const void* data, int data_size, void* digest);
int openssl_generate_key(char* public_key, int32_t size);
class SrsDH
{
private:
    DH* pdh;
public:
    SrsDH();
    virtual ~SrsDH();
public:
    virtual int initialize(bool ensure_128bytes_public_key = false);
    virtual int copy_public_key(char* pkey, int32_t& pkey_size);
    virtual int copy_shared_key(const char* ppkey, int32_t ppkey_size, char* skey, int32_t& skey_size);
private:
    virtual int do_initialize();
};
enum srs_schema_type {
    srs_schema_invalid = 2,
    srs_schema0        = 0,
    srs_schema1        = 1,
};
class key_block
{
public:
    char* random0;
    int random0_size;
    char key[128];
    char* random1;
    int random1_size;
    int32_t offset;
public:
    key_block();
    virtual ~key_block();
public:
    int parse(SrsStream* stream);
private:
    int calc_valid_offset();
};
class digest_block
{
public:
    int32_t offset;
    char* random0;
    int random0_size;
    char digest[32];
    char* random1;
    int random1_size;
public:
    digest_block();
    virtual ~digest_block();
public:
    int parse(SrsStream* stream);
private:
    int calc_valid_offset();
};
class c1s1;
class c1s1_strategy
{
protected:
    key_block key;
    digest_block digest;
public:
    c1s1_strategy();
    virtual ~c1s1_strategy();
public:
    virtual srs_schema_type schema() = 0;
    virtual char * get_digest();
    virtual char * get_key();
    virtual int dump(c1s1* owner, char* _c1s1, int size);
    virtual int parse(char* _c1s1, int size) = 0;
public:
    virtual int c1_create(c1s1* owner);
    virtual int c1_validate_digest(c1s1* owner, bool& is_valid);
    virtual int s1_create(c1s1* owner, c1s1* c1);
    virtual int s1_validate_digest(c1s1* owner, bool& is_valid);
public:
    virtual int calc_c1_digest(c1s1* owner, char *& c1_digest);
    virtual int calc_s1_digest(c1s1* owner, char *& s1_digest);
    virtual int copy_to(c1s1* owner, char* bytes, int size, bool with_digest) = 0;
    virtual void copy_time_version(SrsStream* stream, c1s1* owner);
    virtual void copy_key(SrsStream* stream);
    virtual void copy_digest(SrsStream* stream, bool with_digest);
};
class c1s1_strategy_schema0 : public c1s1_strategy
{
public:
    c1s1_strategy_schema0();
    virtual ~c1s1_strategy_schema0();
public:
    virtual srs_schema_type schema();
    virtual int parse(char* _c1s1, int size);
public:
    virtual int copy_to(c1s1* owner, char* bytes, int size, bool with_digest);
};
class c1s1_strategy_schema1 : public c1s1_strategy
{
public:
    c1s1_strategy_schema1();
    virtual ~c1s1_strategy_schema1();
public:
    virtual srs_schema_type schema();
    virtual int parse(char* _c1s1, int size);
public:
    virtual int copy_to(c1s1* owner, char* bytes, int size, bool with_digest);
};
class c1s1
{
public:
    int32_t time;
    int32_t version;
    c1s1_strategy* payload;
public:
    c1s1();
    virtual ~c1s1();
public:
    virtual srs_schema_type schema();
    virtual char * get_digest();
    virtual char * get_key();
public:
    virtual int dump(char* _c1s1, int size);
    virtual int parse(char* _c1s1, int size, srs_schema_type _schema);
public:
    virtual int c1_create(srs_schema_type _schema);
    virtual int c1_validate_digest(bool& is_valid);
public:
    virtual int s1_create(c1s1* c1);
    virtual int s1_validate_digest(bool& is_valid);
};
class c2s2
{
public:
    char random[1504];
    char digest[32];
public:
    c2s2();
    virtual ~c2s2();
public:
    virtual int dump(char* _c2s2, int size);
    virtual int parse(char* _c2s2, int size);
public:
    virtual int c2_create(c1s1* s1);
    virtual int c2_validate(c1s1* s1, bool& is_valid);
public:
    virtual int s2_create(c1s1* c1);
    virtual int s2_validate(c1s1* c1, bool& is_valid);
};
}
# endif // ifdef SRS_AUTO_SSL
class SrsSimpleHandshake
{
public:
    SrsSimpleHandshake();
    virtual ~SrsSimpleHandshake();
public:
    virtual int handshake_with_client(SrsHandshakeBytes* hs_bytes, ISrsProtocolReaderWriter* io);
    virtual int handshake_with_server(SrsHandshakeBytes* hs_bytes, ISrsProtocolReaderWriter* io);
};
class SrsComplexHandshake
{
public:
    SrsComplexHandshake();
    virtual ~SrsComplexHandshake();
public:
    virtual int handshake_with_client(SrsHandshakeBytes* hs_bytes, ISrsProtocolReaderWriter* io);
    virtual int handshake_with_server(SrsHandshakeBytes* hs_bytes, ISrsProtocolReaderWriter* io);
};
#endif // ifndef SRS_RTMP_PROTOCOL_HANDSHKAE_HPP
#ifndef SRS_RTMP_PROTOCOL_CONSTS_HPP
# define SRS_RTMP_PROTOCOL_CONSTS_HPP
# ifndef _WIN32
#  include <sys/uio.h>
# endif
# include <string>
class SrsMessageHeader;
class SrsSharedPtrMessage;
class ISrsProtocolReaderWriter;
extern void srs_discovery_tc_url(
    std::string tcUrl,
    std::string& schema, std::string& host, std::string& vhost,
    std::string& app, std::string& port, std::string& param
);
extern void srs_vhost_resolve(
    std::string& vhost, std::string& app, std::string& param
);
extern void srs_random_generate(char* bytes, int size);
extern std::string srs_generate_tc_url(
    std::string ip, std::string vhost, std::string app, std::string port,
    std::string param
);
extern bool srs_bytes_equals(void* pa, void* pb, int size);
extern int srs_rtmp_create_msg(char type, u_int32_t timestamp, char* data, int size, int stream_id,
                               SrsSharedPtrMessage** ppmsg);
extern std::string srs_generate_stream_url(std::string vhost, std::string app, std::string stream);
extern int srs_write_large_iovs(ISrsProtocolReaderWriter* skt, iovec* iovs, int size, ssize_t* pnwrite = NULL);
#endif // ifndef SRS_RTMP_PROTOCOL_CONSTS_HPP
#ifndef SRS_RTMP_PROTOCOL_MSG_ARRAY_HPP
# define SRS_RTMP_PROTOCOL_MSG_ARRAY_HPP
class SrsSharedPtrMessage;
class SrsMessageArray
{
public:
    SrsSharedPtrMessage** msgs;
    int max;
public:
    SrsMessageArray(int max_msgs);
    virtual ~SrsMessageArray();
public:
    virtual void free(int count);
private:
    virtual void zero(int count);
};
#endif // ifndef SRS_RTMP_PROTOCOL_MSG_ARRAY_HPP
#ifndef SRS_PROTOCOL_BUFFER_HPP
# define SRS_PROTOCOL_BUFFER_HPP
# ifdef SRS_PERF_MERGED_READ
class IMergeReadHandler
{
public:
    IMergeReadHandler();
    virtual ~IMergeReadHandler();
public:
    virtual void on_read(ssize_t nread) = 0;
};
# endif // ifdef SRS_PERF_MERGED_READ
class SrsFastBuffer
{
private:
    # ifdef SRS_PERF_MERGED_READ
    bool merged_read;
    IMergeReadHandler* _handler;
    # endif
    char* p;
    char* end;
    char* buffer;
    int nb_buffer;
public:
    SrsFastBuffer();
    virtual ~SrsFastBuffer();
public:
    virtual int size();
    virtual char * bytes();
    virtual void set_buffer(int buffer_size);
public:
    virtual char read_1byte();
    virtual char * read_slice(int size);
    virtual void skip(int size);
public:
    virtual int grow(ISrsBufferReader* reader, int required_size);
public:
    # ifdef SRS_PERF_MERGED_READ
    virtual void set_merge_read(bool v, IMergeReadHandler* handler);
    # endif
};
#endif // ifndef SRS_PROTOCOL_BUFFER_HPP
#ifndef SRS_PROTOCOL_RAW_AVC_HPP
# define SRS_PROTOCOL_RAW_AVC_HPP
# include <string>
class SrsStream;
class SrsRawH264Stream
{
public:
    SrsRawH264Stream();
    virtual ~SrsRawH264Stream();
    virtual int annexb_demux_pri(SrsStream* stream, char** pframe, int* pnb_frame);
public:
    virtual int annexb_demux(SrsStream* stream, char** pframe, int* pnb_frame);
    virtual bool is_sps(char* frame, int nb_frame);
    virtual bool is_pps(char* frame, int nb_frame);
    virtual int sps_demux(char* frame, int nb_frame, std::string& sps);
    virtual int pps_demux(char* frame, int nb_frame, std::string& pps);
public:
    virtual int mux_sequence_header(std::string sps, std::string pps, u_int32_t dts, u_int32_t pts, std::string& sh);
    virtual int mux_ipb_frame(char* frame, int nb_frame, std::string& ibp);
    virtual int mux_avc2flv(std::string video, int8_t frame_type, int8_t avc_packet_type, u_int32_t dts, u_int32_t pts,
                            char** flv, int* nb_flv);
};
struct SrsRawAacStreamCodec {
    int8_t           protection_absent;
    SrsAacObjectType aac_object;
    int8_t           sampling_frequency_index;
    int8_t           channel_configuration;
    int16_t          frame_length;
    char             sound_format;
    char             sound_rate;
    char             sound_size;
    char             sound_type;
    int8_t           aac_packet_type;
};
class SrsRawAacStream
{
public:
    SrsRawAacStream();
    virtual ~SrsRawAacStream();
public:
    virtual int adts_demux(SrsStream* stream, char** pframe, int* pnb_frame, SrsRawAacStreamCodec& codec);
    virtual int mux_sequence_header(SrsRawAacStreamCodec* codec, std::string& sh);
    virtual int mux_aac2flv(char* frame, int nb_frame, SrsRawAacStreamCodec* codec, u_int32_t dts, char** flv,
                            int* nb_flv);
};
#endif // ifndef SRS_PROTOCOL_RAW_AVC_HPP
#ifndef SRS_PROTOCOL_RTSP_STACK_HPP
# define SRS_PROTOCOL_RTSP_STACK_HPP
# if !defined(SRS_EXPORT_LIBRTMP)
#  include <string>
#  include <sstream>
#  ifdef SRS_AUTO_STREAM_CASTER
class SrsStream;
class SrsSimpleBuffer;
class SrsCodecSample;
class ISrsProtocolReaderWriter;
#   define SRS_RTSP_CR                   SRS_CONSTS_CR
#   define SRS_RTSP_LF                   SRS_CONSTS_LF
#   define SRS_RTSP_SP                   ' '
#   define SRS_RTSP_CRLF                 "\r\n"
#   define SRS_RTSP_CRLFCRLF             "\r\n\r\n"
#   define SRS_RTSP_TOKEN_CSEQ           "CSeq"
#   define SRS_RTSP_TOKEN_PUBLIC         "Public"
#   define SRS_RTSP_TOKEN_CONTENT_TYPE   "Content-Type"
#   define SRS_RTSP_TOKEN_CONTENT_LENGTH "Content-Length"
#   define SRS_RTSP_TOKEN_TRANSPORT      "Transport"
#   define SRS_RTSP_TOKEN_SESSION        "Session"
#   define SRS_METHOD_OPTIONS            "OPTIONS"
#   define SRS_METHOD_DESCRIBE           "DESCRIBE"
#   define SRS_METHOD_ANNOUNCE           "ANNOUNCE"
#   define SRS_METHOD_SETUP              "SETUP"
#   define SRS_METHOD_PLAY               "PLAY"
#   define SRS_METHOD_PAUSE              "PAUSE"
#   define SRS_METHOD_TEARDOWN           "TEARDOWN"
#   define SRS_METHOD_GET_PARAMETER      "GET_PARAMETER"
#   define SRS_METHOD_SET_PARAMETER      "SET_PARAMETER"
#   define SRS_METHOD_REDIRECT           "REDIRECT"
#   define SRS_METHOD_RECORD             "RECORD"
#   define SRS_RTSP_VERSION              "RTSP/1.0"
enum SrsRtspSdpState {
    SrsRtspSdpStateOthers,
    SrsRtspSdpStateAudio,
    SrsRtspSdpStateVideo,
};
enum SrsRtspMethod {
    SrsRtspMethodDescribe     = 0x0001,
    SrsRtspMethodAnnounce     = 0x0002,
    SrsRtspMethodGetParameter = 0x0004,
    SrsRtspMethodOptions      = 0x0008,
    SrsRtspMethodPause        = 0x0010,
    SrsRtspMethodPlay         = 0x0020,
    SrsRtspMethodRecord       = 0x0040,
    SrsRtspMethodRedirect     = 0x0080,
    SrsRtspMethodSetup        = 0x0100,
    SrsRtspMethodSetParameter = 0x0200,
    SrsRtspMethodTeardown     = 0x0400,
};
enum SrsRtspTokenState {
    SrsRtspTokenStateError  = 100,
    SrsRtspTokenStateNormal = 101,
    SrsRtspTokenStateEOF    = 102,
};
class SrsRtpPacket
{
public:
    int8_t version;
    int8_t padding;
    int8_t extension;
    int8_t csrc_count;
    int8_t marker;
    int8_t payload_type;
    u_int16_t sequence_number;
    u_int32_t timestamp;
    u_int32_t ssrc;
    SrsSimpleBuffer* payload;
    bool chunked;
    bool completed;
    SrsCodecSample* audio_samples;
public:
    SrsRtpPacket();
    virtual ~SrsRtpPacket();
public:
    virtual void copy(SrsRtpPacket* src);
    virtual void reap(SrsRtpPacket* src);
    virtual int decode(SrsStream* stream);
private:
    virtual int decode_97(SrsStream* stream);
    virtual int decode_96(SrsStream* stream);
};
class SrsRtspSdp
{
private:
    SrsRtspSdpState state;
public:
    std::string version;
    std::string owner_username;
    std::string owner_session_id;
    std::string owner_session_version;
    std::string owner_network_type;
    std::string owner_address_type;
    std::string owner_address;
    std::string session_name;
    std::string connection_network_type;
    std::string connection_address_type;
    std::string connection_address;
    std::string tool;
    std::string video_port;
    std::string video_protocol;
    std::string video_transport_format;
    std::string video_bandwidth_kbps;
    std::string video_codec;
    std::string video_sample_rate;
    std::string video_stream_id;
    std::string video_packetization_mode;
    std::string video_sps;
    std::string video_pps;
    std::string audio_port;
    std::string audio_protocol;
    std::string audio_transport_format;
    std::string audio_bandwidth_kbps;
    std::string audio_codec;
    std::string audio_sample_rate;
    std::string audio_channel;
    std::string audio_stream_id;
    std::string audio_profile_level_id;
    std::string audio_mode;
    std::string audio_size_length;
    std::string audio_index_length;
    std::string audio_index_delta_length;
    std::string audio_sh;
public:
    SrsRtspSdp();
    virtual ~SrsRtspSdp();
public:
    virtual int parse(std::string token);
private:
    virtual int parse_fmtp_attribute(std::string attr);
    virtual int parse_control_attribute(std::string attr);
    virtual std::string base64_decode(std::string value);
};
class SrsRtspTransport
{
public:
    std::string transport;
    std::string profile;
    std::string lower_transport;
    std::string cast_type;
    std::string mode;
    int client_port_min;
    int client_port_max;
public:
    SrsRtspTransport();
    virtual ~SrsRtspTransport();
public:
    virtual int parse(std::string attr);
};
class SrsRtspRequest
{
public:
    std::string method;
    std::string uri;
    std::string version;
    long seq;
    std::string content_type;
    long content_length;
    std::string session;
    SrsRtspSdp* sdp;
    SrsRtspTransport* transport;
    int stream_id;
public:
    SrsRtspRequest();
    virtual ~SrsRtspRequest();
public:
    virtual bool is_options();
    virtual bool is_announce();
    virtual bool is_setup();
    virtual bool is_record();
};
class SrsRtspResponse
{
public:
    int status;
    long seq;
    std::string session;
public:
    SrsRtspResponse(int cseq);
    virtual ~SrsRtspResponse();
public:
    virtual int encode(std::stringstream& ss);
protected:
    virtual int encode_header(std::stringstream& ss);
};
class SrsRtspOptionsResponse : public SrsRtspResponse
{
public:
    SrsRtspMethod methods;
public:
    SrsRtspOptionsResponse(int cseq);
    virtual ~SrsRtspOptionsResponse();
protected:
    virtual int encode_header(std::stringstream& ss);
};
class SrsRtspSetupResponse : public SrsRtspResponse
{
public:
    int client_port_min;
    int client_port_max;
    int local_port_min;
    int local_port_max;
    std::string session;
public:
    SrsRtspSetupResponse(int cseq);
    virtual ~SrsRtspSetupResponse();
protected:
    virtual int encode_header(std::stringstream& ss);
};
class SrsRtspStack
{
private:
    SrsSimpleBuffer* buf;
    ISrsProtocolReaderWriter* skt;
public:
    SrsRtspStack(ISrsProtocolReaderWriter* s);
    virtual ~SrsRtspStack();
public:
    virtual int recv_message(SrsRtspRequest** preq);
    virtual int send_message(SrsRtspResponse* res);
private:
    virtual int do_recv_message(SrsRtspRequest* req);
    virtual int recv_token_normal(std::string& token);
    virtual int recv_token_eof(std::string& token);
    virtual int recv_token_util_eof(std::string& token, int* pconsumed = NULL);
    virtual int recv_token(std::string& token, SrsRtspTokenState& state, char normal_ch = SRS_RTSP_SP,
                           int* pconsumed = NULL);
};
#  endif // ifdef SRS_AUTO_STREAM_CASTER
# endif  // if !defined(SRS_EXPORT_LIBRTMP)
#endif   // ifndef SRS_PROTOCOL_RTSP_STACK_HPP
#ifndef SRS_PROTOCOL_HTTP_HPP
# define SRS_PROTOCOL_HTTP_HPP
# if !defined(SRS_EXPORT_LIBRTMP)
#  include <map>
#  include <string>
#  include <vector>
#  ifndef _WIN32
#   include <sys/uio.h>
#  endif
class SrsFileReader;
class SrsHttpHeader;
class ISrsHttpMessage;
class SrsHttpMuxEntry;
class ISrsHttpResponseWriter;
#  define SRS_HTTP_CR                  SRS_CONSTS_CR
#  define SRS_HTTP_LF                  SRS_CONSTS_LF
#  define SRS_HTTP_SP                  ' '
#  define SRS_HTTP_HT                  '\x09'
#  define SRS_HTTP_CRLF                "\r\n"
#  define SRS_HTTP_CRLFCRLF            "\r\n\r\n"
#  define SRS_HTTP_TS_SEND_BUFFER_SIZE 4096
#  define SRS_HTTP_READ_CACHE_BYTES    4096
#  define SRS_DEFAULT_HTTP_PORT        80
#  define SRS_CONSTS_HTTP_OPTIONS      HTTP_OPTIONS
#  define SRS_CONSTS_HTTP_GET          HTTP_GET
#  define SRS_CONSTS_HTTP_POST         HTTP_POST
#  define SRS_CONSTS_HTTP_PUT          HTTP_PUT
#  define SRS_CONSTS_HTTP_DELETE       HTTP_DELETE
extern int srs_go_http_error(ISrsHttpResponseWriter* w, int code);
extern int srs_go_http_error(ISrsHttpResponseWriter* w, int code, std::string error);
extern std::string srs_generate_http_status_text(int status);
extern bool srs_go_http_body_allowd(int status);
extern std::string srs_go_http_detect(char* data, int size);
enum SrsHttpParseState {
    SrsHttpParseStateInit = 0,
    SrsHttpParseStateStart,
    SrsHttpParseStateHeaderComplete,
    SrsHttpParseStateMessageComplete
};
class SrsHttpHeader
{
private:
    std::map<std::string, std::string> headers;
public:
    SrsHttpHeader();
    virtual ~SrsHttpHeader();
public:
    virtual void set(std::string key, std::string value);
    virtual std::string get(std::string key);
public:
    virtual int64_t content_length();
    virtual void set_content_length(int64_t size);
public:
    virtual std::string content_type();
    virtual void set_content_type(std::string ct);
public:
    virtual void write(std::stringstream& ss);
};
class ISrsHttpResponseWriter
{
public:
    ISrsHttpResponseWriter();
    virtual ~ISrsHttpResponseWriter();
public:
    virtual int final_request()             = 0;
    virtual SrsHttpHeader * header()        = 0;
    virtual int write(char* data, int size) = 0;
    virtual int writev(iovec* iov, int iovcnt, ssize_t* pnwrite) = 0;
    virtual void write_header(int code) = 0;
};
class ISrsHttpResponseReader
{
public:
    ISrsHttpResponseReader();
    virtual ~ISrsHttpResponseReader();
public:
    virtual bool eof() = 0;
    virtual int read(char* data, int nb_data, int* nb_read) = 0;
};
class ISrsHttpHandler
{
public:
    SrsHttpMuxEntry* entry;
public:
    ISrsHttpHandler();
    virtual ~ISrsHttpHandler();
public:
    virtual bool is_not_found();
    virtual int serve_http(ISrsHttpResponseWriter* w, ISrsHttpMessage* r) = 0;
};
class SrsHttpRedirectHandler : public ISrsHttpHandler
{
private:
    std::string url;
    int code;
public:
    SrsHttpRedirectHandler(std::string u, int c);
    virtual ~SrsHttpRedirectHandler();
public:
    virtual int serve_http(ISrsHttpResponseWriter* w, ISrsHttpMessage* r);
};
class SrsHttpNotFoundHandler : public ISrsHttpHandler
{
public:
    SrsHttpNotFoundHandler();
    virtual ~SrsHttpNotFoundHandler();
public:
    virtual bool is_not_found();
    virtual int serve_http(ISrsHttpResponseWriter* w, ISrsHttpMessage* r);
};
class SrsHttpFileServer : public ISrsHttpHandler
{
protected:
    std::string dir;
public:
    SrsHttpFileServer(std::string root_dir);
    virtual ~SrsHttpFileServer();
public:
    virtual int serve_http(ISrsHttpResponseWriter* w, ISrsHttpMessage* r);
private:
    virtual int serve_file(ISrsHttpResponseWriter* w, ISrsHttpMessage* r, std::string fullpath);
    virtual int serve_flv_file(ISrsHttpResponseWriter* w, ISrsHttpMessage* r, std::string fullpath);
    virtual int serve_mp4_file(ISrsHttpResponseWriter* w, ISrsHttpMessage* r, std::string fullpath);
protected:
    virtual int serve_flv_stream(ISrsHttpResponseWriter* w, ISrsHttpMessage* r, std::string fullpath, int offset);
    virtual int serve_mp4_stream(ISrsHttpResponseWriter* w, ISrsHttpMessage* r, std::string fullpath, int start,
                                 int end);
protected:
    virtual int copy(ISrsHttpResponseWriter* w, SrsFileReader* fs, ISrsHttpMessage* r, int size);
};
class SrsHttpMuxEntry
{
public:
    bool explicit_match;
    ISrsHttpHandler* handler;
    std::string pattern;
    bool enabled;
public:
    SrsHttpMuxEntry();
    virtual ~SrsHttpMuxEntry();
};
class ISrsHttpMatchHijacker
{
public:
    ISrsHttpMatchHijacker();
    virtual ~ISrsHttpMatchHijacker();
public:
    virtual int hijack(ISrsHttpMessage* request, ISrsHttpHandler** ph) = 0;
};
class ISrsHttpServeMux
{
public:
    ISrsHttpServeMux();
    virtual ~ISrsHttpServeMux();
public:
    virtual int serve_http(ISrsHttpResponseWriter* w, ISrsHttpMessage* r) = 0;
};
class SrsHttpServeMux : public ISrsHttpServeMux
{
private:
    std::map<std::string, SrsHttpMuxEntry *> entries;
    std::map<std::string, ISrsHttpHandler *> vhosts;
    std::vector<ISrsHttpMatchHijacker *> hijackers;
public:
    SrsHttpServeMux();
    virtual ~SrsHttpServeMux();
public:
    virtual int initialize();
    virtual void hijack(ISrsHttpMatchHijacker* h);
    virtual void unhijack(ISrsHttpMatchHijacker* h);
public:
    virtual int handle(std::string pattern, ISrsHttpHandler* handler);
    virtual bool can_serve(ISrsHttpMessage* r);
public:
    virtual int serve_http(ISrsHttpResponseWriter* w, ISrsHttpMessage* r);
private:
    virtual int find_handler(ISrsHttpMessage* r, ISrsHttpHandler** ph);
    virtual int match(ISrsHttpMessage* r, ISrsHttpHandler** ph);
    virtual bool path_match(std::string pattern, std::string path);
};
typedef std::pair<std::string, std::string> SrsHttpHeaderField;
class ISrsHttpMessage
{
private:
    char* _http_ts_send_buffer;
public:
    ISrsHttpMessage();
    virtual ~ISrsHttpMessage();
public:
    virtual char * http_ts_send_buffer();
public:
    virtual u_int8_t method()        = 0;
    virtual u_int16_t status_code()  = 0;
    virtual std::string method_str() = 0;
    virtual bool is_http_get()       = 0;
    virtual bool is_http_put()       = 0;
    virtual bool is_http_post()      = 0;
    virtual bool is_http_delete()    = 0;
    virtual bool is_http_options()   = 0;
public:
    virtual bool is_keep_alive() = 0;
    virtual std::string uri()    = 0;
    virtual std::string url()    = 0;
    virtual std::string host()   = 0;
    virtual std::string path()   = 0;
    virtual std::string query()  = 0;
    virtual std::string ext()    = 0;
    virtual int parse_rest_id(std::string pattern) = 0;
public:
    virtual int enter_infinite_chunked()           = 0;
    virtual int body_read_all(std::string& body)   = 0;
    virtual ISrsHttpResponseReader * body_reader() = 0;
    virtual int64_t content_length() = 0;
public:
    virtual std::string query_get(std::string key) = 0;
    virtual int request_header_count() = 0;
    virtual std::string request_header_key_at(int index)   = 0;
    virtual std::string request_header_value_at(int index) = 0;
public:
    virtual bool is_jsonp() = 0;
};
# endif // if !defined(SRS_EXPORT_LIBRTMP)
#endif  // ifndef SRS_PROTOCOL_HTTP_HPP
#ifndef SRS_PROTOCOL_KBPS_HPP
# define SRS_PROTOCOL_KBPS_HPP
class SrsKbpsSample
{
public:
    int64_t bytes;
    int64_t time;
    int kbps;
public:
    SrsKbpsSample();
};
class SrsKbpsSlice
{
private:
    union slice_io {
        ISrsProtocolStatistic* in;
        ISrsProtocolStatistic* out;
    };
public:
    slice_io io;
    int64_t bytes;
    int64_t starttime;
    int64_t io_bytes_base;
    int64_t last_bytes;
    SrsKbpsSample sample_30s;
    SrsKbpsSample sample_1m;
    SrsKbpsSample sample_5m;
    SrsKbpsSample sample_60m;
public:
    int64_t delta_bytes;
public:
    SrsKbpsSlice();
    virtual ~SrsKbpsSlice();
public:
    virtual int64_t get_total_bytes();
    virtual void sample();
};
class IKbpsDelta
{
public:
    IKbpsDelta();
    virtual ~IKbpsDelta();
public:
    virtual void resample() = 0;
    virtual int64_t get_send_bytes_delta() = 0;
    virtual int64_t get_recv_bytes_delta() = 0;
    virtual void cleanup() = 0;
};
class SrsKbps : public virtual ISrsProtocolStatistic, public virtual IKbpsDelta
{
private:
    SrsKbpsSlice is;
    SrsKbpsSlice os;
public:
    SrsKbps();
    virtual ~SrsKbps();
public:
    virtual void set_io(ISrsProtocolStatistic* in, ISrsProtocolStatistic* out);
public:
    virtual int get_send_kbps();
    virtual int get_recv_kbps();
    virtual int get_send_kbps_30s();
    virtual int get_recv_kbps_30s();
    virtual int get_send_kbps_5m();
    virtual int get_recv_kbps_5m();
public:
    virtual int64_t get_send_bytes();
    virtual int64_t get_recv_bytes();
public:
    virtual void resample();
    virtual int64_t get_send_bytes_delta();
    virtual int64_t get_recv_bytes_delta();
    virtual void cleanup();
public:
    virtual void add_delta(IKbpsDelta* delta);
    virtual void sample();
};
#endif // ifndef SRS_PROTOCOL_KBPS_HPP
#ifndef SRS_PROTOCOL_JSON_HPP
# define SRS_PROTOCOL_JSON_HPP
# include <string>
# include <vector>
# undef SRS_JSON_USE_NXJSON
# define SRS_JSON_USE_NXJSON
class SrsJsonArray;
class SrsJsonObject;
class SrsJsonAny
{
public:
    char marker;
protected:
    SrsJsonAny();
public:
    virtual ~SrsJsonAny();
public:
    virtual bool is_string();
    virtual bool is_boolean();
    virtual bool is_integer();
    virtual bool is_number();
    virtual bool is_object();
    virtual bool is_array();
    virtual bool is_null();
public:
    virtual std::string to_str();
    virtual bool to_boolean();
    virtual int64_t to_integer();
    virtual double to_number();
    virtual SrsJsonObject * to_object();
    virtual SrsJsonArray * to_array();
public:
    static SrsJsonAny * str(const char* value = NULL);
    static SrsJsonAny * boolean(bool value    = false);
    static SrsJsonAny * ingeter(int64_t value = 0);
    static SrsJsonAny * number(double value   = 0.0);
    static SrsJsonAny * null();
    static SrsJsonObject * object();
    static SrsJsonArray * array();
public:
    static SrsJsonAny * loads(char* str);
};
class SrsJsonObject : public SrsJsonAny
{
private:
    typedef std::pair<std::string, SrsJsonAny *> SrsJsonObjectPropertyType;
    std::vector<SrsJsonObjectPropertyType> properties;
private:
    friend class SrsJsonAny;
    SrsJsonObject();
public:
    virtual ~SrsJsonObject();
public:
    virtual int count();
    virtual std::string key_at(int index);
    virtual SrsJsonAny * value_at(int index);
public:
    virtual void set(std::string key, SrsJsonAny* value);
    virtual SrsJsonAny * get_property(std::string name);
    virtual SrsJsonAny * ensure_property_string(std::string name);
    virtual SrsJsonAny * ensure_property_integer(std::string name);
    virtual SrsJsonAny * ensure_property_boolean(std::string name);
};
class SrsJsonArray : public SrsJsonAny
{
private:
    std::vector<SrsJsonAny *> properties;
private:
    friend class SrsJsonAny;
    SrsJsonArray();
public:
    virtual ~SrsJsonArray();
public:
    virtual int count();
    virtual SrsJsonAny * at(int index);
    virtual void add(SrsJsonAny* value);
};
# define SRS_JOBJECT_START "{"
# define SRS_JFIELD_NAME(k)    "\"" << k << "\":"
# define SRS_JFIELD_OBJ(k)     SRS_JFIELD_NAME(k) << SRS_JOBJECT_START
# define SRS_JFIELD_STR(k, v)  SRS_JFIELD_NAME(k) << "\"" << v << "\""
# define SRS_JFIELD_ORG(k, v)  SRS_JFIELD_NAME(k) << std::dec << v
# define SRS_JFIELD_BOOL(k, v) SRS_JFIELD_ORG(k, (v ? "true" : "false"))
# define SRS_JFIELD_NULL(k)    SRS_JFIELD_NAME(k) << "null"
# define SRS_JFIELD_ERROR(ret) "\"" << "code" << "\":" << ret
# define SRS_JFIELD_CONT  ","
# define SRS_JOBJECT_END  "}"
# define SRS_JARRAY_START "["
# define SRS_JARRAY_END   "]"
#endif // ifndef SRS_PROTOCOL_JSON_HPP
#ifndef SRS_LIB_RTMP_HPP
# define SRS_LIB_RTMP_HPP
# ifdef _WIN32
#  include <windows.h>
typedef unsigned long long u_int64_t;
typedef long long int64_t;
typedef unsigned int u_int32_t;
typedef u_int32_t uint32_t;
typedef int int32_t;
typedef unsigned char u_int8_t;
typedef char int8_t;
typedef unsigned short u_int16_t;
typedef short int16_t;
typedef int64_t ssize_t;
struct iovec {
    void*  iov_base;
    size_t iov_len;
};
# endif // ifdef _WIN32
# include <sys/types.h>
# ifdef __cplusplus
extern "C" {
# endif
typedef int srs_bool;
extern int srs_version_major();
extern int srs_version_minor();
extern int srs_version_revision();
typedef void * srs_rtmp_t;
typedef void * srs_amf0_t;
extern srs_rtmp_t srs_rtmp_create(const char* url);
extern srs_rtmp_t srs_rtmp_create2(const char* url);
extern int srs_rtmp_set_timeout(srs_rtmp_t rtmp, int recv_timeout_ms, int send_timeout_ms);
extern void srs_rtmp_destroy(srs_rtmp_t rtmp);
extern int srs_rtmp_handshake(srs_rtmp_t rtmp);
extern int srs_rtmp_dns_resolve(srs_rtmp_t rtmp);
extern int srs_rtmp_connect_server(srs_rtmp_t rtmp);
extern int srs_rtmp_do_simple_handshake(srs_rtmp_t rtmp);
extern int srs_rtmp_do_complex_handshake(srs_rtmp_t rtmp);
extern int srs_rtmp_set_connect_args(srs_rtmp_t rtmp,
                                     const char* tcUrl, const char* swfUrl, const char* pageUrl, srs_amf0_t args
);
extern int srs_rtmp_connect_app(srs_rtmp_t rtmp);
extern int srs_rtmp_connect_app2(srs_rtmp_t rtmp,
                                 char srs_server_ip[128], char srs_server[128],
                                 char srs_primary[128], char srs_authors[128],
                                 char srs_version[32], int* srs_id, int* srs_pid
);
extern int srs_rtmp_play_stream(srs_rtmp_t rtmp);
extern int srs_rtmp_publish_stream(srs_rtmp_t rtmp);
extern int srs_rtmp_bandwidth_check(srs_rtmp_t rtmp,
                                    int64_t* start_time, int64_t* end_time,
                                    int* play_kbps, int* publish_kbps,
                                    int* play_bytes, int* publish_bytes,
                                    int* play_duration, int* publish_duration
);
# define SRS_RTMP_TYPE_AUDIO  8
# define SRS_RTMP_TYPE_VIDEO  9
# define SRS_RTMP_TYPE_SCRIPT 18
extern int srs_rtmp_read_packet(srs_rtmp_t rtmp,
                                char* type, u_int32_t* timestamp, char** data, int* size
);
extern int srs_rtmp_write_packet(srs_rtmp_t rtmp,
                                 char type, u_int32_t timestamp, char* data, int size
);
extern srs_bool srs_rtmp_is_onMetaData(char type, char* data, int size);
extern int srs_audio_write_raw_frame(srs_rtmp_t rtmp,
                                     char sound_format, char sound_rate, char sound_size, char sound_type,
                                     char* frame, int frame_size, u_int32_t timestamp
);
extern srs_bool srs_aac_is_adts(char* aac_raw_data, int ac_raw_size);
extern int srs_aac_adts_frame_size(char* aac_raw_data, int ac_raw_size);
extern int srs_h264_write_raw_frames(srs_rtmp_t rtmp,
                                     char* frames, int frames_size, u_int32_t dts, u_int32_t pts
);
extern srs_bool srs_h264_is_dvbsp_error(int error_code);
extern srs_bool srs_h264_is_duplicated_sps_error(int error_code);
extern srs_bool srs_h264_is_duplicated_pps_error(int error_code);
extern srs_bool srs_h264_startswith_annexb(
    char* h264_raw_data, int h264_raw_size,
    int* pnb_start_code
);
typedef void * srs_flv_t;
extern srs_flv_t srs_flv_open_read(const char* file);
extern srs_flv_t srs_flv_open_write(const char* file);
extern void srs_flv_close(srs_flv_t flv);
extern int srs_flv_read_header(srs_flv_t flv, char header[9]);
extern int srs_flv_read_tag_header(srs_flv_t flv,
                                   char* ptype, int32_t* pdata_size, u_int32_t* ptime
);
extern int srs_flv_read_tag_data(srs_flv_t flv, char* data, int32_t size);
extern int srs_flv_write_header(srs_flv_t flv, char header[9]);
extern int srs_flv_write_tag(srs_flv_t flv,
                             char type, int32_t time, char* data, int size
);
extern int srs_flv_size_tag(int data_size);
extern int64_t srs_flv_tellg(srs_flv_t flv);
extern void srs_flv_lseek(srs_flv_t flv, int64_t offset);
extern srs_bool srs_flv_is_eof(int error_code);
extern srs_bool srs_flv_is_sequence_header(char* data, int32_t size);
extern srs_bool srs_flv_is_keyframe(char* data, int32_t size);
typedef double srs_amf0_number;
extern srs_amf0_t srs_amf0_parse(char* data, int size, int* nparsed);
extern srs_amf0_t srs_amf0_create_string(const char* value);
extern srs_amf0_t srs_amf0_create_number(srs_amf0_number value);
extern srs_amf0_t srs_amf0_create_ecma_array();
extern srs_amf0_t srs_amf0_create_strict_array();
extern srs_amf0_t srs_amf0_create_object();
extern srs_amf0_t srs_amf0_ecma_array_to_object(srs_amf0_t ecma_arr);
extern void srs_amf0_free(srs_amf0_t amf0);
extern int srs_amf0_size(srs_amf0_t amf0);
extern int srs_amf0_serialize(srs_amf0_t amf0, char* data, int size);
extern srs_bool srs_amf0_is_string(srs_amf0_t amf0);
extern srs_bool srs_amf0_is_boolean(srs_amf0_t amf0);
extern srs_bool srs_amf0_is_number(srs_amf0_t amf0);
extern srs_bool srs_amf0_is_null(srs_amf0_t amf0);
extern srs_bool srs_amf0_is_object(srs_amf0_t amf0);
extern srs_bool srs_amf0_is_ecma_array(srs_amf0_t amf0);
extern srs_bool srs_amf0_is_strict_array(srs_amf0_t amf0);
extern const char * srs_amf0_to_string(srs_amf0_t amf0);
extern srs_bool srs_amf0_to_boolean(srs_amf0_t amf0);
extern srs_amf0_number srs_amf0_to_number(srs_amf0_t amf0);
extern void srs_amf0_set_number(srs_amf0_t amf0, srs_amf0_number value);
extern int srs_amf0_object_property_count(srs_amf0_t amf0);
extern const char * srs_amf0_object_property_name_at(srs_amf0_t amf0, int index);
extern srs_amf0_t srs_amf0_object_property_value_at(srs_amf0_t amf0, int index);
extern srs_amf0_t srs_amf0_object_property(srs_amf0_t amf0, const char* name);
extern void srs_amf0_object_property_set(srs_amf0_t amf0, const char* name, srs_amf0_t value);
extern void srs_amf0_object_clear(srs_amf0_t amf0);
extern int srs_amf0_ecma_array_property_count(srs_amf0_t amf0);
extern const char * srs_amf0_ecma_array_property_name_at(srs_amf0_t amf0, int index);
extern srs_amf0_t srs_amf0_ecma_array_property_value_at(srs_amf0_t amf0, int index);
extern srs_amf0_t srs_amf0_ecma_array_property(srs_amf0_t amf0, const char* name);
extern void srs_amf0_ecma_array_property_set(srs_amf0_t amf0, const char* name, srs_amf0_t value);
extern int srs_amf0_strict_array_property_count(srs_amf0_t amf0);
extern srs_amf0_t srs_amf0_strict_array_property_at(srs_amf0_t amf0, int index);
extern void srs_amf0_strict_array_append(srs_amf0_t amf0, srs_amf0_t value);
extern int64_t srs_utils_time_ms();
extern int64_t srs_utils_send_bytes(srs_rtmp_t rtmp);
extern int64_t srs_utils_recv_bytes(srs_rtmp_t rtmp);
extern int srs_utils_parse_timestamp(
    u_int32_t time, char type, char* data, int size,
    u_int32_t* ppts
);
extern srs_bool srs_utils_flv_tag_is_ok(char type);
extern srs_bool srs_utils_flv_tag_is_audio(char type);
extern srs_bool srs_utils_flv_tag_is_video(char type);
extern srs_bool srs_utils_flv_tag_is_av(char type);
extern char srs_utils_flv_video_codec_id(char* data, int size);
extern char srs_utils_flv_video_avc_packet_type(char* data, int size);
extern char srs_utils_flv_video_frame_type(char* data, int size);
extern char srs_utils_flv_audio_sound_format(char* data, int size);
extern char srs_utils_flv_audio_sound_rate(char* data, int size);
extern char srs_utils_flv_audio_sound_size(char* data, int size);
extern char srs_utils_flv_audio_sound_type(char* data, int size);
extern char srs_utils_flv_audio_aac_packet_type(char* data, int size);
extern char * srs_human_amf0_print(srs_amf0_t amf0, char** pdata, int* psize);
extern const char * srs_human_flv_tag_type2string(char type);
extern const char * srs_human_flv_video_codec_id2string(char codec_id);
extern const char * srs_human_flv_video_avc_packet_type2string(char avc_packet_type);
extern const char * srs_human_flv_video_frame_type2string(char frame_type);
extern const char * srs_human_flv_audio_sound_format2string(char sound_format);
extern const char * srs_human_flv_audio_sound_rate2string(char sound_rate);
extern const char * srs_human_flv_audio_sound_size2string(char sound_size);
extern const char * srs_human_flv_audio_sound_type2string(char sound_type);
extern const char * srs_human_flv_audio_aac_packet_type2string(char aac_packet_type);
extern int srs_human_print_rtmp_packet(char type, u_int32_t timestamp, char* data, int size);
extern int srs_human_print_rtmp_packet2(char type, u_int32_t timestamp, char* data, int size, u_int32_t pre_timestamp);
extern int srs_human_print_rtmp_packet3(char type, u_int32_t timestamp, char* data, int size, u_int32_t pre_timestamp,
                                        int64_t pre_now);
extern int srs_human_print_rtmp_packet4(char type, u_int32_t timestamp, char* data, int size, u_int32_t pre_timestamp,
                                        int64_t pre_now, int64_t starttime, int64_t nb_packets);
extern const char * srs_human_format_time();
# ifdef SRS_DISABLE_LOG
#  define srs_human_trace(msg, ...)   (void) 0
#  define srs_human_verbose(msg, ...) (void) 0
#  define srs_human_raw(msg, ...)     (void) 0
# else
#  define srs_human_trace(msg, ...) \
    printf("[%s] ", srs_human_format_time()); printf(msg, ## __VA_ARGS__); \
    printf("\n")
#  define srs_human_verbose(msg, ...) \
    printf("[%s] ", srs_human_format_time()); printf(msg, ## __VA_ARGS__); \
    printf("\n")
#  define srs_human_raw(msg, ...) printf(msg, ## __VA_ARGS__)
# endif // ifdef SRS_DISABLE_LOG
typedef void * srs_hijack_io_t;
# ifdef SRS_HIJACK_IO
#  ifndef _WIN32
#   include <sys/uio.h>
#  endif
extern srs_hijack_io_t srs_hijack_io_get(srs_rtmp_t rtmp);
# endif
# ifdef SRS_HIJACK_IO
extern srs_hijack_io_t srs_hijack_io_create();
extern void srs_hijack_io_destroy(srs_hijack_io_t ctx);
extern int srs_hijack_io_create_socket(srs_hijack_io_t ctx);
extern int srs_hijack_io_connect(srs_hijack_io_t ctx, const char* server_ip, int port);
extern int srs_hijack_io_read(srs_hijack_io_t ctx, void* buf, size_t size, ssize_t* nread);
extern int srs_hijack_io_set_recv_timeout(srs_hijack_io_t ctx, int64_t timeout_us);
extern int64_t srs_hijack_io_get_recv_timeout(srs_hijack_io_t ctx);
extern int64_t srs_hijack_io_get_recv_bytes(srs_hijack_io_t ctx);
extern int srs_hijack_io_set_send_timeout(srs_hijack_io_t ctx, int64_t timeout_us);
extern int64_t srs_hijack_io_get_send_timeout(srs_hijack_io_t ctx);
extern int64_t srs_hijack_io_get_send_bytes(srs_hijack_io_t ctx);
extern int srs_hijack_io_writev(srs_hijack_io_t ctx, const iovec* iov, int iov_size, ssize_t* nwrite);
extern bool srs_hijack_io_is_never_timeout(srs_hijack_io_t ctx, int64_t timeout_us);
extern int srs_hijack_io_read_fully(srs_hijack_io_t ctx, void* buf, size_t size, ssize_t* nread);
extern int srs_hijack_io_write(srs_hijack_io_t ctx, void* buf, size_t size, ssize_t* nwrite);
# endif // ifdef SRS_HIJACK_IO
# ifdef _WIN32
#  define _CRT_SECURE_NO_WARNINGS
#  include <time.h>
int gettimeofday(struct timeval* tv, struct timezone* tz);
#  define PRId64 "lld"
typedef int socklen_t;
const char * inet_ntop_win32(int af, const void* src, char* dst, socklen_t size);
#  include <direct.h>
typedef int mode_t;
#  define S_IRUSR 0
#  define S_IWUSR 0
#  define S_IXUSR 0
#  define S_IRGRP 0
#  define S_IWGRP 0
#  define S_IXGRP 0
#  define S_IROTH 0
#  define S_IXOTH 0
#  include <io.h>
#  include <fcntl.h>
#  define open  _open
#  define close _close
#  define lseek _lseek
#  define write _write
#  define read  _read
typedef int pid_t;
pid_t getpid(void);
ssize_t writev(int fd, const struct iovec* iov, int iovcnt);
typedef int64_t useconds_t;
int usleep(useconds_t usec);
int socket_setup();
int socket_cleanup();
#  define snprintf _snprintf
# endif // ifdef _WIN32
# ifdef __cplusplus
}
# endif
#endif // ifndef SRS_LIB_RTMP_HPP
#ifndef SRS_LIB_BANDWIDTH_HPP
# define SRS_LIB_BANDWIDTH_HPP
class SrsRtmpClient;
class SrsBandwidthPacket;
class SrsBandwidthClient
{
private:
    SrsRtmpClient* _rtmp;
public:
    SrsBandwidthClient();
    virtual ~SrsBandwidthClient();
public:
    virtual int initialize(SrsRtmpClient* rtmp);
    virtual int bandwidth_check(
        int64_t* start_time, int64_t* end_time,
        int* play_kbps, int* publish_kbps,
        int* play_bytes, int* publish_bytes,
        int* play_duration, int* publish_duration
    );
private:
    virtual int play_start();
    virtual int play_checking();
    virtual int play_stop();
    virtual int publish_start(int& duration_ms, int& play_kbps);
    virtual int publish_checking(int duration_ms, int play_kbps);
    virtual int publish_stop();
    virtual int final (SrsBandwidthPacket**ppkt);
};
#endif // ifndef SRS_LIB_BANDWIDTH_HPP
#ifndef SRS_LIB_SIMPLE_SOCKET_IMPL_HPP
# define SRS_LIB_SIMPLE_SOCKET_IMPL_HPP
# ifndef _WIN32
#  define SOCKET int
# endif
class SimpleSocketStreamImpl : public SimpleSocketStream
{
private:
    srs_hijack_io_t io;
public:
    SimpleSocketStreamImpl();
    virtual ~SimpleSocketStreamImpl();
public:
    virtual srs_hijack_io_t hijack_io();
    virtual int create_socket();
    virtual int connect(const char* server, int port);
    virtual int disconnect();
public:
    virtual int read(void* buf, size_t size, ssize_t* nread);
public:
    virtual void set_recv_timeout(int64_t timeout_us);
    virtual int64_t get_recv_timeout();
    virtual int64_t get_recv_bytes();
public:
    virtual void set_send_timeout(int64_t timeout_us);
    virtual int64_t get_send_timeout();
    virtual int64_t get_send_bytes();
    virtual int writev(const iovec* iov, int iov_size, ssize_t* nwrite);
public:
    virtual bool is_never_timeout(int64_t timeout_us);
    virtual int read_fully(void* buf, size_t size, ssize_t* nread);
    virtual int write(void* buf, size_t size, ssize_t* nwrite);
};
#endif // ifndef SRS_LIB_SIMPLE_SOCKET_IMPL_HPP
#ifdef SRS_AUTO_MEM_WATCH
# include <map>
# include <stdio.h>
using namespace std;
struct SrsMemoryObject {
    void*       ptr;
    std::string category;
    int         size;
};
std::map<void *, SrsMemoryObject *> _srs_ptrs;
void srs_memory_watch(void* ptr, string category, int size)
{
    SrsMemoryObject* obj = NULL;
    std::map<void *, SrsMemoryObject *>::iterator it;

    if ((it = _srs_ptrs.find(ptr)) != _srs_ptrs.end()) {
        obj = it->second;
    } else {
        obj = new SrsMemoryObject();
        _srs_ptrs[ptr] = obj;
    }
    obj->ptr      = ptr;
    obj->category = category;
    obj->size     = size;
}

void srs_memory_unwatch(void* ptr)
{
    std::map<void *, SrsMemoryObject *>::iterator it;

    if ((it = _srs_ptrs.find(ptr)) != _srs_ptrs.end()) {
        SrsMemoryObject* obj = it->second;
        srs_freep(obj);
        _srs_ptrs.erase(it);
    }
}

void srs_memory_report()
{
    printf("srs memory watch leak report:\n");
    int total = 0;
    std::map<void *, SrsMemoryObject *>::iterator it;
    for (it = _srs_ptrs.begin(); it != _srs_ptrs.end(); ++it) {
        SrsMemoryObject* obj = it->second;
        printf("    %s: %#" PRIx64 ", %dB\n", obj->category.c_str(), (int64_t) obj->ptr, obj->size);
        total += obj->size;
    }
    printf("%d objects leak %dKB.\n", (int) _srs_ptrs.size(), total / 1024);
    printf("@remark use script to cleanup for memory watch: ./etc/init.d/srs stop\n");
}

#endif // ifdef SRS_AUTO_MEM_WATCH
bool srs_is_system_control_error(int error_code)
{
    return error_code == ERROR_CONTROL_RTMP_CLOSE ||
           error_code == ERROR_CONTROL_REPUBLISH;
}

bool srs_is_client_gracefully_close(int error_code)
{
    return error_code == ERROR_SOCKET_READ ||
           error_code == ERROR_SOCKET_READ_FULLY ||
           error_code == ERROR_SOCKET_WRITE ||
           error_code == ERROR_SOCKET_TIMEOUT;
}

ISrsLog::ISrsLog()
{}

ISrsLog::~ISrsLog()
{}

int ISrsLog::initialize()
{
    return ERROR_SUCCESS;
}

void ISrsLog::verbose(const char *, int, const char *, ...)
{}

void ISrsLog::info(const char *, int, const char *, ...)
{}

void ISrsLog::trace(const char *, int, const char *, ...)
{}

void ISrsLog::warn(const char *, int, const char *, ...)
{}

void ISrsLog::error(const char *, int, const char *, ...)
{}

ISrsThreadContext::ISrsThreadContext()
{}

ISrsThreadContext::~ISrsThreadContext()
{}

int ISrsThreadContext::generate_id()
{
    return 0;
}

int ISrsThreadContext::get_id()
{
    return 0;
}

int ISrsThreadContext::set_id(int)
{
    return 0;
}

using namespace std;
SrsStream::SrsStream()
{
    p        = bytes = NULL;
    nb_bytes = 0;
    srs_assert(srs_is_little_endian());
}

SrsStream::~SrsStream()
{}

int SrsStream::initialize(char* b, int nb)
{
    int ret = ERROR_SUCCESS;

    if (!b) {
        ret = ERROR_KERNEL_STREAM_INIT;
        srs_error("stream param bytes must not be NULL. ret=%d", ret);
        return ret;
    }
    if (nb <= 0) {
        ret = ERROR_KERNEL_STREAM_INIT;
        srs_error("stream param size must be positive. ret=%d", ret);
        return ret;
    }
    nb_bytes = nb;
    p        = bytes = b;
    srs_info("init stream ok, size=%d", size());
    return ret;
}

char * SrsStream::data()
{
    return bytes;
}

int SrsStream::size()
{
    return nb_bytes;
}

int SrsStream::pos()
{
    return (int) (p - bytes);
}

bool SrsStream::empty()
{
    return !bytes || (p >= bytes + nb_bytes);
}

bool SrsStream::require(int required_size)
{
    srs_assert(required_size >= 0);
    return required_size <= nb_bytes - (p - bytes);
}

void SrsStream::skip(int size)
{
    srs_assert(p);
    p += size;
}

int8_t SrsStream::read_1bytes()
{
    srs_assert(require(1));
    return (int8_t) *p++;
}

int16_t SrsStream::read_2bytes()
{
    srs_assert(require(2));
    int16_t value;
    char* pp = (char *) &value;
    pp[1] = *p++;
    pp[0] = *p++;
    return value;
}

int32_t SrsStream::read_3bytes()
{
    srs_assert(require(3));
    int32_t value = 0x00;
    char* pp      = (char *) &value;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;
    return value;
}

int32_t SrsStream::read_4bytes()
{
    srs_assert(require(4));
    int32_t value;
    char* pp = (char *) &value;
    pp[3] = *p++;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;
    return value;
}

int64_t SrsStream::read_8bytes()
{
    srs_assert(require(8));
    int64_t value;
    char* pp = (char *) &value;
    pp[7] = *p++;
    pp[6] = *p++;
    pp[5] = *p++;
    pp[4] = *p++;
    pp[3] = *p++;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;
    return value;
}

string SrsStream::read_string(int len)
{
    srs_assert(require(len));
    std::string value;
    value.append(p, len);
    p += len;
    return value;
}

void SrsStream::read_bytes(char* data, int size)
{
    srs_assert(require(size));
    memcpy(data, p, size);
    p += size;
}

void SrsStream::write_1bytes(int8_t value)
{
    srs_assert(require(1));
    *p++ = value;
}

void SrsStream::write_2bytes(int16_t value)
{
    srs_assert(require(2));
    char* pp = (char *) &value;
    *p++ = pp[1];
    *p++ = pp[0];
}

void SrsStream::write_4bytes(int32_t value)
{
    srs_assert(require(4));
    char* pp = (char *) &value;
    *p++ = pp[3];
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
}

void SrsStream::write_3bytes(int32_t value)
{
    srs_assert(require(3));
    char* pp = (char *) &value;
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
}

void SrsStream::write_8bytes(int64_t value)
{
    srs_assert(require(8));
    char* pp = (char *) &value;
    *p++ = pp[7];
    *p++ = pp[6];
    *p++ = pp[5];
    *p++ = pp[4];
    *p++ = pp[3];
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
}

void SrsStream::write_string(string value)
{
    srs_assert(require((int) value.length()));
    memcpy(p, value.data(), value.length());
    p += value.length();
}

void SrsStream::write_bytes(char* data, int size)
{
    srs_assert(require(size));
    memcpy(p, data, size);
    p += size;
}

SrsBitStream::SrsBitStream()
{
    cb      = 0;
    cb_left = 0;
    stream  = NULL;
}

SrsBitStream::~SrsBitStream()
{}

int SrsBitStream::initialize(SrsStream* s)
{
    stream = s;
    return ERROR_SUCCESS;
}

bool SrsBitStream::empty()
{
    if (cb_left) {
        return false;
    }
    return stream->empty();
}

int8_t SrsBitStream::read_bit()
{
    if (!cb_left) {
        srs_assert(!stream->empty());
        cb      = stream->read_1bytes();
        cb_left = 8;
    }
    int8_t v = (cb >> (cb_left - 1)) & 0x01;
    cb_left--;
    return v;
}

#ifndef _WIN32
# include <unistd.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <sys/time.h>
#endif
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;
#define SYS_TIME_RESOLUTION_US 300*1000
int srs_avc_nalu_read_uev(SrsBitStream* stream, int32_t& v)
{
    int ret = ERROR_SUCCESS;

    if (stream->empty()) {
        return ERROR_AVC_NALU_UEV;
    }
    int leadingZeroBits = -1;
    for (int8_t b = 0; !b && !stream->empty(); leadingZeroBits++) {
        b = stream->read_bit();
    }
    if (leadingZeroBits >= 31) {
        return ERROR_AVC_NALU_UEV;
    }
    v = (1 << leadingZeroBits) - 1;
    for (int i = 0; i < leadingZeroBits; i++) {
        int32_t b = stream->read_bit();
        v += b << (leadingZeroBits - 1 - i);
    }
    return ret;
}

int srs_avc_nalu_read_bit(SrsBitStream* stream, int8_t& v)
{
    int ret = ERROR_SUCCESS;

    if (stream->empty()) {
        return ERROR_AVC_NALU_UEV;
    }
    v = stream->read_bit();
    return ret;
}

static int64_t _srs_system_time_us_cache     = 0;
static int64_t _srs_system_time_startup_time = 0;
int64_t srs_get_system_time_ms()
{
    if (_srs_system_time_us_cache <= 0) {
        srs_update_system_time_ms();
    }
    return _srs_system_time_us_cache / 1000;
}

int64_t srs_get_system_startup_time_ms()
{
    if (_srs_system_time_startup_time <= 0) {
        srs_update_system_time_ms();
    }
    return _srs_system_time_startup_time / 1000;
}

int64_t srs_update_system_time_ms()
{
    timeval now;

    if (gettimeofday(&now, NULL) < 0) {
        srs_warn("gettimeofday failed, ignore");
        return -1;
    }
    int64_t now_us = ((int64_t) now.tv_sec) * 1000 * 1000 + (int64_t) now.tv_usec;
    if (_srs_system_time_us_cache <= 0) {
        _srs_system_time_us_cache     = now_us;
        _srs_system_time_startup_time = now_us;
        return _srs_system_time_us_cache / 1000;
    }
    int64_t diff = now_us - _srs_system_time_us_cache;
    diff = srs_max(0, diff);
    if (diff < 0 || diff > 1000 * SYS_TIME_RESOLUTION_US) {
        srs_warn("system time jump, history=%" PRId64 "us, now=%" PRId64 "us, diff=%" PRId64 "us",
                 _srs_system_time_us_cache, now_us, diff);
        _srs_system_time_startup_time += diff;
    }
    _srs_system_time_us_cache = now_us;
    srs_info("system time updated, startup=%" PRId64 "us, now=%" PRId64 "us",
             _srs_system_time_startup_time, _srs_system_time_us_cache);
    return _srs_system_time_us_cache / 1000;
}

string srs_dns_resolve(string host)
{
    if (inet_addr(host.c_str()) != INADDR_NONE) {
        return host;
    }
    hostent* answer = gethostbyname(host.c_str());
    if (answer == NULL) {
        return "";
    }
    char ipv4[16];
    memset(ipv4, 0, sizeof(ipv4));
    for (int i = 0; i < answer->h_length; i++) {
        #ifdef _WIN32
        inet_ntop_win32(AF_INET, answer->h_addr_list[i], ipv4, sizeof(ipv4));
        #else
        inet_ntop(AF_INET, answer->h_addr_list[i], ipv4, sizeof(ipv4));
        #endif
        break;
    }
    return ipv4;
}

bool srs_is_little_endian()
{
    static int little_endian_check = -1;

    if (little_endian_check == -1) {
        union {
            int32_t i;
            int8_t  c;
        } little_check_union;
        little_check_union.i = 0x01;
        little_endian_check  = little_check_union.c;
    }
    return (little_endian_check == 1);
}

string srs_string_replace(string str, string old_str, string new_str)
{
    std::string ret = str;

    if (old_str == new_str) {
        return ret;
    }
    size_t pos = 0;
    while ((pos = ret.find(old_str, pos)) != std::string::npos) {
        ret = ret.replace(pos, old_str.length(), new_str);
    }
    return ret;
}

string srs_string_trim_end(string str, string trim_chars)
{
    std::string ret = str;

    for (int i = 0; i < (int) trim_chars.length(); i++) {
        char ch = trim_chars.at(i);
        while (!ret.empty() && ret.at(ret.length() - 1) == ch) {
            ret.erase(ret.end() - 1);
            i = 0;
        }
    }
    return ret;
}

string srs_string_trim_start(string str, string trim_chars)
{
    std::string ret = str;

    for (int i = 0; i < (int) trim_chars.length(); i++) {
        char ch = trim_chars.at(i);
        while (!ret.empty() && ret.at(0) == ch) {
            ret.erase(ret.begin());
            i = 0;
        }
    }
    return ret;
}

string srs_string_remove(string str, string remove_chars)
{
    std::string ret = str;

    for (int i = 0; i < (int) remove_chars.length(); i++) {
        char ch = remove_chars.at(i);
        for (std::string::iterator it = ret.begin(); it != ret.end();) {
            if (ch == *it) {
                it = ret.erase(it);
                i  = 0;
            } else {
                ++it;
            }
        }
    }
    return ret;
}

bool srs_string_ends_with(string str, string flag)
{
    return str.rfind(flag) == str.length() - flag.length();
}

bool srs_string_starts_with(string str, string flag)
{
    return str.find(flag) == 0;
}

bool srs_string_starts_with(string str, string flag0, string flag1)
{
    return srs_string_starts_with(str, flag0) || srs_string_starts_with(str, flag1);
}

bool srs_string_contains(string str, string flag)
{
    return str.find(flag) != string::npos;
}

bool srs_string_contains(string str, string flag0, string flag1)
{
    return str.find(flag0) != string::npos || str.find(flag1) != string::npos;
}

bool srs_string_contains(string str, string flag0, string flag1, string flag2)
{
    return str.find(flag0) != string::npos || str.find(flag1) != string::npos || str.find(flag2) != string::npos;
}

int srs_do_create_dir_recursively(string dir)
{
    int ret = ERROR_SUCCESS;

    if (srs_path_exists(dir)) {
        return ERROR_SYSTEM_DIR_EXISTS;
    }
    size_t pos;
    if ((pos = dir.rfind("/")) != std::string::npos) {
        std::string parent = dir.substr(0, pos);
        ret = srs_do_create_dir_recursively(parent);
        if (ret != ERROR_SUCCESS && ret != ERROR_SYSTEM_DIR_EXISTS) {
            return ret;
        }
        ret = ERROR_SUCCESS;
    }
    #ifndef _WIN32
    mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH;
    if (::mkdir(dir.c_str(), mode) < 0) {
    #else
    if (_mkdir(dir.c_str()) < 0) {
        #endif
        if (errno == EEXIST) {
            return ERROR_SYSTEM_DIR_EXISTS;
        }
        ret = ERROR_SYSTEM_CREATE_DIR;
        srs_error("create dir %s failed. ret=%d", dir.c_str(), ret);
        return ret;
    }
    srs_info("create dir %s success.", dir.c_str());
    return ret;
} // srs_do_create_dir_recursively

int srs_create_dir_recursively(string dir)
{
    int ret = ERROR_SUCCESS;

    ret = srs_do_create_dir_recursively(dir);
    if (ret == ERROR_SYSTEM_DIR_EXISTS) {
        return ERROR_SUCCESS;
    }
    return ret;
}

bool srs_path_exists(std::string path)
{
    struct stat st;

    if (stat(path.c_str(), &st) == 0) {
        return true;
    }
    return false;
}

string srs_path_dirname(string path)
{
    std::string dirname = path;
    size_t pos = string::npos;

    if ((pos = dirname.rfind("/")) != string::npos) {
        if (pos == 0) {
            return "/";
        }
        dirname = dirname.substr(0, pos);
    }
    return dirname;
}

string srs_path_basename(string path)
{
    std::string dirname = path;
    size_t pos = string::npos;

    if ((pos = dirname.rfind("/")) != string::npos) {
        if (dirname.length() == 1) {
            return dirname;
        }
        dirname = dirname.substr(pos + 1);
    }
    return dirname;
}

bool srs_avc_startswith_annexb(SrsStream* stream, int* pnb_start_code)
{
    char* bytes = stream->data() + stream->pos();
    char* p     = bytes;

    for (;;) {
        if (!stream->require(p - bytes + 3)) {
            return false;
        }
        if (p[0] != (char) 0x00 || p[1] != (char) 0x00) {
            return false;
        }
        if (p[2] == (char) 0x01) {
            if (pnb_start_code) {
                *pnb_start_code = (int) (p - bytes) + 3;
            }
            return true;
        }
        p++;
    }
    return false;
}

bool srs_aac_startswith_adts(SrsStream* stream)
{
    char* bytes = stream->data() + stream->pos();
    char* p     = bytes;

    if (!stream->require((int) (p - bytes) + 2)) {
        return false;
    }
    if (p[0] != (char) 0xff || (char) (p[1] & 0xf0) != (char) 0xf0) {
        return false;
    }
    return true;
}

static const u_int32_t crc_table[256] = {
    0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
    0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
    0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
    0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
    0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
    0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
    0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
    0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
    0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
    0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
    0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
    0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
    0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
    0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
    0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
    0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
    0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
    0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
    0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
    0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
    0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
    0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
    0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
    0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
    0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
    0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
    0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
    0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
    0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
    0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
    0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
    0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
    0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
    0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
    0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
    0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
    0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
    0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
    0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
    0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
    0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
    0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
    0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};
unsigned int mpegts_crc32(const u_int8_t* data, int len)
{
    register int i;
    unsigned int crc = 0xffffffff;

    for (i = 0; i < len; i++) {
        crc = (crc << 8) ^ crc_table[((crc >> 24) ^ *data++) & 0xff];
    }
    return crc;
}

u_int32_t srs_crc32(const void* buf, int size)
{
    return mpegts_crc32((const u_int8_t *) buf, size);
}

#ifndef UINT_MAX
# define UINT_MAX 0xffffffff
#endif
#ifndef AV_RB32
# define AV_RB32(x)                                \
    (((uint32_t) ((const u_int8_t *) (x))[0] << 24)      \
     |(((const u_int8_t *) (x))[1] << 16)      \
     |(((const u_int8_t *) (x))[2] <<  8)      \
     |((const u_int8_t *) (x))[3])
#endif
#ifndef AV_WL32
# define AV_WL32(p, darg) \
    do {                \
        unsigned d = (darg);                    \
        ((u_int8_t *) (p))[0] = (d);               \
        ((u_int8_t *) (p))[1] = (d)>>8;            \
        ((u_int8_t *) (p))[2] = (d)>>16;           \
        ((u_int8_t *) (p))[3] = (d)>>24;           \
    } while (0)
#endif
#define AV_WN(s, p, v) AV_WL ## s(p, v)
#if    defined(AV_WN32) && !defined(AV_WL32)
# define AV_WL32(p, v) AV_WN32(p, v)
#elif !defined(AV_WN32) && defined(AV_WL32)
# define AV_WN32(p, v) AV_WL32(p, v)
#endif
#ifndef AV_WN32
# define AV_WN32(p, v) AV_WN(32, p, v)
#endif
#define AV_BSWAP16C(x) (((x) << 8 & 0xff00)  | ((x) >> 8 & 0x00ff))
#define AV_BSWAP32C(x) (AV_BSWAP16C(x) << 16 | AV_BSWAP16C((x) >> 16))
#ifndef av_bswap32
static const u_int32_t av_bswap32(u_int32_t x)
{
    return AV_BSWAP32C(x);
}

#endif
#define av_be2ne32(x) av_bswap32(x)
static const u_int8_t map2[256] = {
    0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff,
    0x3e, 0xff, 0xff, 0xff, 0x3f, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff,
    0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x01,
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x1b,
    0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
    0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33,
    0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};
#define BASE64_DEC_STEP(i) \
    do { \
        bits = map2[in[i]]; \
        if (bits & 0x80) \
        goto out ## i; \
        v = i ? (v << 6) +bits : bits; \
    } while (0)
int srs_av_base64_decode(u_int8_t* out, const char* in_str, int out_size)
{
    u_int8_t* dst      = out;
    u_int8_t* end      = out + out_size;
    const u_int8_t* in = (const u_int8_t *) in_str;
    unsigned bits      = 0xff;
    unsigned v;

    while (end - dst > 3) {
        BASE64_DEC_STEP(0);
        BASE64_DEC_STEP(1);
        BASE64_DEC_STEP(2);
        BASE64_DEC_STEP(3);
        v = av_be2ne32(v << 8);
        AV_WN32(dst, v);
        dst += 3;
        in  += 4;
    }
    if (end - dst) {
        BASE64_DEC_STEP(0);
        BASE64_DEC_STEP(1);
        BASE64_DEC_STEP(2);
        BASE64_DEC_STEP(3);
        *dst++ = v >> 16;
        if (end - dst) {
            *dst++ = v >> 8;
        }
        if (end - dst) {
            *dst++ = v;
        }
        in += 4;
    }
    while (1) {
        BASE64_DEC_STEP(0);
        in++;
        BASE64_DEC_STEP(0);
        in++;
        BASE64_DEC_STEP(0);
        in++;
        BASE64_DEC_STEP(0);
        in++;
    }
out3:
    *dst++ = v >> 10;
    v    <<= 2;
out2:
    *dst++ = v >> 4;
out1:
out0:
    return bits & 1 ? -1 : dst - out;
} // srs_av_base64_decode

char * srs_av_base64_encode(char* out, int out_size, const u_int8_t* in, int in_size)
{
    static const char b64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char* ret, * dst;
    unsigned i_bits     = 0;
    int i_shift         = 0;
    int bytes_remaining = in_size;

    if (in_size >= (int) (UINT_MAX / 4) ||
        out_size < SRS_AV_BASE64_SIZE(in_size))
    {
        return NULL;
    }
    ret = dst = out;
    while (bytes_remaining > 3) {
        i_bits = AV_RB32(in);
        in    += 3;
        bytes_remaining -= 3;
        *dst++ = b64[ i_bits >> 26        ];
        *dst++ = b64[(i_bits >> 20) & 0x3F];
        *dst++ = b64[(i_bits >> 14) & 0x3F];
        *dst++ = b64[(i_bits >> 8) & 0x3F];
    }
    i_bits = 0;
    while (bytes_remaining) {
        i_bits = (i_bits << 8) + *in++;
        bytes_remaining--;
        i_shift += 8;
    }
    while (i_shift > 0) {
        *dst++   = b64[(i_bits << 6 >> i_shift) & 0x3f];
        i_shift -= 6;
    }
    while ((dst - ret) & 3) {
        *dst++ = '=';
    }
    *dst = '\0';
    return ret;
} // srs_av_base64_encode

#define SPACE_CHARS " \t\r\n"
int av_toupper(int c)
{
    if (c >= 'a' && c <= 'z') {
        c ^= 0x20;
    }
    return c;
}

int ff_hex_to_data(u_int8_t* data, const char* p)
{
    int c, len, v;

    len = 0;
    v   = 1;
    for (;;) {
        p += strspn(p, SPACE_CHARS);
        if (*p == '\0') {
            break;
        }
        c = av_toupper((unsigned char) *p++);
        if (c >= '0' && c <= '9') {
            c = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            c = c - 'A' + 10;
        } else {
            break;
        }
        v = (v << 4) | c;
        if (v & 0x100) {
            if (data) {
                data[len] = v;
            }
            len++;
            v = 1;
        }
    }
    return len;
}

int srs_chunk_header_c0(
    int perfer_cid, u_int32_t timestamp, int32_t payload_length,
    int8_t message_type, int32_t stream_id,
    char* cache, int nb_cache
)
{
    char* pp = NULL;
    char* p  = cache;

    if (nb_cache < SRS_CONSTS_RTMP_MAX_FMT0_HEADER_SIZE) {
        return 0;
    }
    *p++ = 0x00 | (perfer_cid & 0x3F);
    if (timestamp < RTMP_EXTENDED_TIMESTAMP) {
        pp   = (char *) &timestamp;
        *p++ = pp[2];
        *p++ = pp[1];
        *p++ = pp[0];
    } else {
        *p++ = 0xFF;
        *p++ = 0xFF;
        *p++ = 0xFF;
    }
    pp   = (char *) &payload_length;
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
    *p++ = message_type;
    pp   = (char *) &stream_id;
    *p++ = pp[0];
    *p++ = pp[1];
    *p++ = pp[2];
    *p++ = pp[3];
    if (timestamp >= RTMP_EXTENDED_TIMESTAMP) {
        pp   = (char *) &timestamp;
        *p++ = pp[3];
        *p++ = pp[2];
        *p++ = pp[1];
        *p++ = pp[0];
    }
    return p - cache;
} // srs_chunk_header_c0

int srs_chunk_header_c3(
    int perfer_cid, u_int32_t timestamp,
    char* cache, int nb_cache
)
{
    char* pp = NULL;
    char* p  = cache;

    if (nb_cache < SRS_CONSTS_RTMP_MAX_FMT3_HEADER_SIZE) {
        return 0;
    }
    *p++ = 0xC0 | (perfer_cid & 0x3F);
    if (timestamp >= RTMP_EXTENDED_TIMESTAMP) {
        pp   = (char *) &timestamp;
        *p++ = pp[3];
        *p++ = pp[2];
        *p++ = pp[1];
        *p++ = pp[0];
    }
    return p - cache;
}

#ifndef _WIN32
# include <unistd.h>
#endif
#include <fcntl.h>
#include <sstream>
using namespace std;
SrsMessageHeader::SrsMessageHeader()
{
    message_type    = 0;
    payload_length  = 0;
    timestamp_delta = 0;
    stream_id       = 0;
    timestamp       = 0;
    perfer_cid      = RTMP_CID_OverConnection;
}

SrsMessageHeader::~SrsMessageHeader()
{}

bool SrsMessageHeader::is_audio()
{
    return message_type == RTMP_MSG_AudioMessage;
}

bool SrsMessageHeader::is_video()
{
    return message_type == RTMP_MSG_VideoMessage;
}

bool SrsMessageHeader::is_amf0_command()
{
    return message_type == RTMP_MSG_AMF0CommandMessage;
}

bool SrsMessageHeader::is_amf0_data()
{
    return message_type == RTMP_MSG_AMF0DataMessage;
}

bool SrsMessageHeader::is_amf3_command()
{
    return message_type == RTMP_MSG_AMF3CommandMessage;
}

bool SrsMessageHeader::is_amf3_data()
{
    return message_type == RTMP_MSG_AMF3DataMessage;
}

bool SrsMessageHeader::is_window_ackledgement_size()
{
    return message_type == RTMP_MSG_WindowAcknowledgementSize;
}

bool SrsMessageHeader::is_ackledgement()
{
    return message_type == RTMP_MSG_Acknowledgement;
}

bool SrsMessageHeader::is_set_chunk_size()
{
    return message_type == RTMP_MSG_SetChunkSize;
}

bool SrsMessageHeader::is_user_control_message()
{
    return message_type == RTMP_MSG_UserControlMessage;
}

bool SrsMessageHeader::is_set_peer_bandwidth()
{
    return message_type == RTMP_MSG_SetPeerBandwidth;
}

bool SrsMessageHeader::is_aggregate()
{
    return message_type == RTMP_MSG_AggregateMessage;
}

void SrsMessageHeader::initialize_amf0_script(int size, int stream)
{
    message_type    = RTMP_MSG_AMF0DataMessage;
    payload_length  = (int32_t) size;
    timestamp_delta = (int32_t) 0;
    timestamp       = (int64_t) 0;
    stream_id       = (int32_t) stream;
    perfer_cid      = RTMP_CID_OverConnection2;
}

void SrsMessageHeader::initialize_audio(int size, u_int32_t time, int stream)
{
    message_type    = RTMP_MSG_AudioMessage;
    payload_length  = (int32_t) size;
    timestamp_delta = (int32_t) time;
    timestamp       = (int64_t) time;
    stream_id       = (int32_t) stream;
    perfer_cid      = RTMP_CID_Audio;
}

void SrsMessageHeader::initialize_video(int size, u_int32_t time, int stream)
{
    message_type    = RTMP_MSG_VideoMessage;
    payload_length  = (int32_t) size;
    timestamp_delta = (int32_t) time;
    timestamp       = (int64_t) time;
    stream_id       = (int32_t) stream;
    perfer_cid      = RTMP_CID_Video;
}

SrsCommonMessage::SrsCommonMessage()
{
    payload = NULL;
    size    = 0;
}

SrsCommonMessage::~SrsCommonMessage()
{
    #ifdef SRS_AUTO_MEM_WATCH
    srs_memory_unwatch(payload);
    #endif
    srs_freepa(payload);
}

void SrsCommonMessage::create_payload(int size)
{
    srs_freepa(payload);
    payload = new char[size];
    srs_verbose("create payload for RTMP message. size=%d", size);
    #ifdef SRS_AUTO_MEM_WATCH
    srs_memory_watch(payload, "RTMP.msg.payload", size);
    #endif
}

SrsSharedPtrMessage::SrsSharedPtrPayload::SrsSharedPtrPayload()
{
    payload      = NULL;
    size         = 0;
    shared_count = 0;
}

SrsSharedPtrMessage::SrsSharedPtrPayload::~SrsSharedPtrPayload()
{
    #ifdef SRS_AUTO_MEM_WATCH
    srs_memory_unwatch(payload);
    #endif
    srs_freepa(payload);
}

SrsSharedPtrMessage::SrsSharedPtrMessage()
{
    ptr = NULL;
}

SrsSharedPtrMessage::~SrsSharedPtrMessage()
{
    if (ptr) {
        if (ptr->shared_count == 0) {
            srs_freep(ptr);
        } else {
            ptr->shared_count--;
        }
    }
}

int SrsSharedPtrMessage::create(SrsCommonMessage* msg)
{
    int ret = ERROR_SUCCESS;

    if ((ret = create(&msg->header, msg->payload, msg->size)) != ERROR_SUCCESS) {
        return ret;
    }
    msg->payload = NULL;
    msg->size    = 0;
    return ret;
}

int SrsSharedPtrMessage::create(SrsMessageHeader* pheader, char* payload, int size)
{
    int ret = ERROR_SUCCESS;

    if (ptr) {
        ret = ERROR_SYSTEM_ASSERT_FAILED;
        srs_error("should not set the payload twice. ret=%d", ret);
        srs_assert(false);
        return ret;
    }
    ptr = new SrsSharedPtrPayload();
    if (pheader) {
        ptr->header.message_type   = pheader->message_type;
        ptr->header.payload_length = size;
        ptr->header.perfer_cid     = pheader->perfer_cid;
        this->timestamp = pheader->timestamp;
        this->stream_id = pheader->stream_id;
    }
    ptr->payload  = payload;
    ptr->size     = size;
    this->payload = ptr->payload;
    this->size    = ptr->size;
    return ret;
}

int SrsSharedPtrMessage::count()
{
    srs_assert(ptr);
    return ptr->shared_count;
}

bool SrsSharedPtrMessage::check(int stream_id)
{
    if (ptr->header.perfer_cid < 2) {
        srs_info("change the chunk_id=%d to default=%d",
                 ptr->header.perfer_cid, RTMP_CID_ProtocolControl);
        ptr->header.perfer_cid = RTMP_CID_ProtocolControl;
    }
    if (this->stream_id == stream_id) {
        return true;
    }
    this->stream_id = stream_id;
    return false;
}

bool SrsSharedPtrMessage::is_av()
{
    return ptr->header.message_type == RTMP_MSG_AudioMessage ||
           ptr->header.message_type == RTMP_MSG_VideoMessage;
}

bool SrsSharedPtrMessage::is_audio()
{
    return ptr->header.message_type == RTMP_MSG_AudioMessage;
}

bool SrsSharedPtrMessage::is_video()
{
    return ptr->header.message_type == RTMP_MSG_VideoMessage;
}

int SrsSharedPtrMessage::chunk_header(char* cache, int nb_cache, bool c0)
{
    if (c0) {
        return srs_chunk_header_c0(
            ptr->header.perfer_cid, timestamp, ptr->header.payload_length,
            ptr->header.message_type, stream_id,
            cache, nb_cache);
    } else {
        return srs_chunk_header_c3(
            ptr->header.perfer_cid, timestamp,
            cache, nb_cache);
    }
}

SrsSharedPtrMessage * SrsSharedPtrMessage::copy()
{
    srs_assert(ptr);
    SrsSharedPtrMessage* copy = new SrsSharedPtrMessage();
    copy->ptr = ptr;
    ptr->shared_count++;
    copy->timestamp = timestamp;
    copy->stream_id = stream_id;
    copy->payload   = ptr->payload;
    copy->size      = ptr->size;
    return copy;
}

SrsFlvEncoder::SrsFlvEncoder()
{
    reader     = NULL;
    tag_stream = new SrsStream();
    #ifdef SRS_PERF_FAST_FLV_ENCODER
    nb_tag_headers = 0;
    tag_headers    = NULL;
    nb_iovss_cache = 0;
    iovss_cache    = NULL;
    nb_ppts        = 0;
    ppts = NULL;
    #endif
}

SrsFlvEncoder::~SrsFlvEncoder()
{
    srs_freep(tag_stream);
    #ifdef SRS_PERF_FAST_FLV_ENCODER
    srs_freepa(tag_headers);
    srs_freepa(iovss_cache);
    srs_freepa(ppts);
    #endif
}

int SrsFlvEncoder::initialize(SrsFileWriter* fr)
{
    int ret = ERROR_SUCCESS;

    srs_assert(fr);
    if (!fr->is_open()) {
        ret = ERROR_KERNEL_FLV_STREAM_CLOSED;
        srs_warn("stream is not open for encoder. ret=%d", ret);
        return ret;
    }
    reader = fr;
    return ret;
}

int SrsFlvEncoder::write_header()
{
    int ret = ERROR_SUCCESS;
    char flv_header[] = {
        'F',         'L',         'V',
        (char) 0x01,
        (char) 0x05,
        (char) 0x00, (char) 0x00, (char) 0x00, (char) 0x09
    };

    if ((ret = write_header(flv_header)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int SrsFlvEncoder::write_header(char flv_header[9])
{
    int ret = ERROR_SUCCESS;

    if ((ret = reader->write(flv_header, 9, NULL)) != ERROR_SUCCESS) {
        srs_error("write flv header failed. ret=%d", ret);
        return ret;
    }
    char pts[] = { (char) 0x00, (char) 0x00, (char) 0x00, (char) 0x00 };
    if ((ret = reader->write(pts, 4, NULL)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int SrsFlvEncoder::write_metadata(char type, char* data, int size)
{
    int ret = ERROR_SUCCESS;

    srs_assert(data);
    if ((ret = write_metadata_to_cache(type, data, size, tag_header)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = write_tag(tag_header, sizeof(tag_header), data, size)) != ERROR_SUCCESS) {
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("write flv data tag failed. ret=%d", ret);
        }
        return ret;
    }
    return ret;
}

int SrsFlvEncoder::write_audio(int64_t timestamp, char* data, int size)
{
    int ret = ERROR_SUCCESS;

    srs_assert(data);
    if ((ret = write_audio_to_cache(timestamp, data, size, tag_header)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = write_tag(tag_header, sizeof(tag_header), data, size)) != ERROR_SUCCESS) {
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("write flv audio tag failed. ret=%d", ret);
        }
        return ret;
    }
    return ret;
}

int SrsFlvEncoder::write_video(int64_t timestamp, char* data, int size)
{
    int ret = ERROR_SUCCESS;

    srs_assert(data);
    if ((ret = write_video_to_cache(timestamp, data, size, tag_header)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = write_tag(tag_header, sizeof(tag_header), data, size)) != ERROR_SUCCESS) {
        srs_error("write flv video tag failed. ret=%d", ret);
        return ret;
    }
    return ret;
}

int SrsFlvEncoder::size_tag(int data_size)
{
    srs_assert(data_size >= 0);
    return SRS_FLV_TAG_HEADER_SIZE + data_size + SRS_FLV_PREVIOUS_TAG_SIZE;
}

#ifdef SRS_PERF_FAST_FLV_ENCODER
int SrsFlvEncoder::write_tags(SrsSharedPtrMessage** msgs, int count)
{
    int ret      = ERROR_SUCCESS;
    int nb_iovss = 3 * count;
    iovec* iovss = iovss_cache;

    if (nb_iovss_cache < nb_iovss) {
        srs_freepa(iovss_cache);
        nb_iovss_cache = nb_iovss;
        iovss = iovss_cache = new iovec[nb_iovss];
    }
    char* cache = tag_headers;
    if (nb_tag_headers < count) {
        srs_freepa(tag_headers);
        nb_tag_headers = count;
        cache = tag_headers = new char[SRS_FLV_TAG_HEADER_SIZE * count];
    }
    char* pts = ppts;
    if (nb_ppts < count) {
        srs_freepa(ppts);
        nb_ppts = count;
        pts     = ppts = new char[SRS_FLV_PREVIOUS_TAG_SIZE * count];
    }
    iovec* iovs = iovss;
    for (int i = 0; i < count; i++) {
        SrsSharedPtrMessage* msg = msgs[i];
        if (msg->is_audio()) {
            if ((ret = write_audio_to_cache(msg->timestamp, msg->payload, msg->size, cache)) != ERROR_SUCCESS) {
                return ret;
            }
        } else if (msg->is_video()) {
            if ((ret = write_video_to_cache(msg->timestamp, msg->payload, msg->size, cache)) != ERROR_SUCCESS) {
                return ret;
            }
        } else {
            if ((ret =
                     write_metadata_to_cache(SrsCodecFlvTagScript, msg->payload, msg->size, cache)) != ERROR_SUCCESS)
            {
                return ret;
            }
        }
        if ((ret = write_pts_to_cache(SRS_FLV_TAG_HEADER_SIZE + msg->size, pts)) != ERROR_SUCCESS) {
            return ret;
        }
        iovs[0].iov_base = cache;
        iovs[0].iov_len  = SRS_FLV_TAG_HEADER_SIZE;
        iovs[1].iov_base = msg->payload;
        iovs[1].iov_len  = msg->size;
        iovs[2].iov_base = pts;
        iovs[2].iov_len  = SRS_FLV_PREVIOUS_TAG_SIZE;
        cache += SRS_FLV_TAG_HEADER_SIZE;
        pts   += SRS_FLV_PREVIOUS_TAG_SIZE;
        iovs  += 3;
    }
    if ((ret = reader->writev(iovss, nb_iovss, NULL)) != ERROR_SUCCESS) {
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("write flv tags failed. ret=%d", ret);
        }
        return ret;
    }
    return ret;
} // SrsFlvEncoder::write_tags

#endif // ifdef SRS_PERF_FAST_FLV_ENCODER
int SrsFlvEncoder::write_metadata_to_cache(char type, char* data, int size, char* cache)
{
    int ret = ERROR_SUCCESS;

    srs_assert(data);
    if ((ret = tag_stream->initialize(cache, 11)) != ERROR_SUCCESS) {
        return ret;
    }
    tag_stream->write_1bytes(type);
    tag_stream->write_3bytes(size);
    tag_stream->write_3bytes(0x00);
    tag_stream->write_1bytes(0x00);
    tag_stream->write_3bytes(0x00);
    return ret;
}

int SrsFlvEncoder::write_audio_to_cache(int64_t timestamp, char* data, int size, char* cache)
{
    int ret = ERROR_SUCCESS;

    srs_assert(data);
    timestamp &= 0x7fffffff;
    if ((ret = tag_stream->initialize(cache, 11)) != ERROR_SUCCESS) {
        return ret;
    }
    tag_stream->write_1bytes(SrsCodecFlvTagAudio);
    tag_stream->write_3bytes(size);
    tag_stream->write_3bytes((int32_t) timestamp);
    tag_stream->write_1bytes((timestamp >> 24) & 0xFF);
    tag_stream->write_3bytes(0x00);
    return ret;
}

int SrsFlvEncoder::write_video_to_cache(int64_t timestamp, char* data, int size, char* cache)
{
    int ret = ERROR_SUCCESS;

    srs_assert(data);
    timestamp &= 0x7fffffff;
    if ((ret = tag_stream->initialize(cache, 11)) != ERROR_SUCCESS) {
        return ret;
    }
    tag_stream->write_1bytes(SrsCodecFlvTagVideo);
    tag_stream->write_3bytes(size);
    tag_stream->write_3bytes((int32_t) timestamp);
    tag_stream->write_1bytes((timestamp >> 24) & 0xFF);
    tag_stream->write_3bytes(0x00);
    return ret;
}

int SrsFlvEncoder::write_pts_to_cache(int size, char* cache)
{
    int ret = ERROR_SUCCESS;

    if ((ret = tag_stream->initialize(cache, SRS_FLV_PREVIOUS_TAG_SIZE)) != ERROR_SUCCESS) {
        return ret;
    }
    tag_stream->write_4bytes(size);
    return ret;
}

int SrsFlvEncoder::write_tag(char* header, int header_size, char* tag, int tag_size)
{
    int ret = ERROR_SUCCESS;
    char pre_size[SRS_FLV_PREVIOUS_TAG_SIZE];

    if ((ret = write_pts_to_cache(tag_size + header_size, pre_size)) != ERROR_SUCCESS) {
        return ret;
    }
    iovec iovs[3];
    iovs[0].iov_base = header;
    iovs[0].iov_len  = header_size;
    iovs[1].iov_base = tag;
    iovs[1].iov_len  = tag_size;
    iovs[2].iov_base = pre_size;
    iovs[2].iov_len  = SRS_FLV_PREVIOUS_TAG_SIZE;
    if ((ret = reader->writev(iovs, 3, NULL)) != ERROR_SUCCESS) {
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("write flv tag failed. ret=%d", ret);
        }
        return ret;
    }
    return ret;
}

SrsFlvDecoder::SrsFlvDecoder()
{
    reader     = NULL;
    tag_stream = new SrsStream();
}

SrsFlvDecoder::~SrsFlvDecoder()
{
    srs_freep(tag_stream);
}

int SrsFlvDecoder::initialize(SrsFileReader* fr)
{
    int ret = ERROR_SUCCESS;

    srs_assert(fr);
    if (!fr->is_open()) {
        ret = ERROR_KERNEL_FLV_STREAM_CLOSED;
        srs_warn("stream is not open for decoder. ret=%d", ret);
        return ret;
    }
    reader = fr;
    return ret;
}

int SrsFlvDecoder::read_header(char header[9])
{
    int ret = ERROR_SUCCESS;

    srs_assert(header);
    if ((ret = reader->read(header, 9, NULL)) != ERROR_SUCCESS) {
        return ret;
    }
    char* h = header;
    if (h[0] != 'F' || h[1] != 'L' || h[2] != 'V') {
        ret = ERROR_KERNEL_FLV_HEADER;
        srs_warn("flv header must start with FLV. ret=%d", ret);
        return ret;
    }
    return ret;
}

int SrsFlvDecoder::read_tag_header(char* ptype, int32_t* pdata_size, u_int32_t* ptime)
{
    int ret = ERROR_SUCCESS;

    srs_assert(ptype);
    srs_assert(pdata_size);
    srs_assert(ptime);
    char th[11];
    if ((ret = reader->read(th, 11, NULL)) != ERROR_SUCCESS) {
        if (ret != ERROR_SYSTEM_FILE_EOF) {
            srs_error("read flv tag header failed. ret=%d", ret);
        }
        return ret;
    }
    *ptype = (th[0] & 0x1F);
    char* pp = (char *) pdata_size;
    pp[3] = 0;
    pp[2] = th[1];
    pp[1] = th[2];
    pp[0] = th[3];
    pp    = (char *) ptime;
    pp[2] = th[4];
    pp[1] = th[5];
    pp[0] = th[6];
    pp[3] = th[7];
    return ret;
} // SrsFlvDecoder::read_tag_header

int SrsFlvDecoder::read_tag_data(char* data, int32_t size)
{
    int ret = ERROR_SUCCESS;

    srs_assert(data);
    if ((ret = reader->read(data, size, NULL)) != ERROR_SUCCESS) {
        if (ret != ERROR_SYSTEM_FILE_EOF) {
            srs_error("read flv tag header failed. ret=%d", ret);
        }
        return ret;
    }
    return ret;
}

int SrsFlvDecoder::read_previous_tag_size(char previous_tag_size[4])
{
    int ret = ERROR_SUCCESS;

    srs_assert(previous_tag_size);
    if ((ret = reader->read(previous_tag_size, 4, NULL)) != ERROR_SUCCESS) {
        if (ret != ERROR_SYSTEM_FILE_EOF) {
            srs_error("read flv previous tag size failed. ret=%d", ret);
        }
        return ret;
    }
    return ret;
}

SrsFlvVodStreamDecoder::SrsFlvVodStreamDecoder()
{
    reader     = NULL;
    tag_stream = new SrsStream();
}

SrsFlvVodStreamDecoder::~SrsFlvVodStreamDecoder()
{
    srs_freep(tag_stream);
}

int SrsFlvVodStreamDecoder::initialize(SrsFileReader* fr)
{
    int ret = ERROR_SUCCESS;

    srs_assert(fr);
    if (!fr->is_open()) {
        ret = ERROR_KERNEL_FLV_STREAM_CLOSED;
        srs_warn("stream is not open for decoder. ret=%d", ret);
        return ret;
    }
    reader = fr;
    return ret;
}

int SrsFlvVodStreamDecoder::read_header_ext(char header[13])
{
    int ret = ERROR_SUCCESS;

    srs_assert(header);
    int size = 13;
    if ((ret = reader->read(header, size, NULL)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int SrsFlvVodStreamDecoder::read_sequence_header_summary(int64_t* pstart, int* psize)
{
    int ret = ERROR_SUCCESS;

    srs_assert(pstart);
    srs_assert(psize);
    char tag_header[] = {
        (char) 0x00,
        (char) 0x00,(char) 0x00,  (char) 0x00,
        (char) 0x00,(char) 0x00,  (char) 0x00,
        (char) 0x00,
        (char) 0x00,(char) 0x00,  (char) 0x00,
    };
    bool got_video = false;
    bool got_audio = false;
    int64_t av_sequence_offset_start = -1;
    int64_t av_sequence_offset_end   = -1;
    for (;;) {
        if ((ret = reader->read(tag_header, SRS_FLV_TAG_HEADER_SIZE, NULL)) != ERROR_SUCCESS) {
            return ret;
        }
        if ((ret = tag_stream->initialize(tag_header, SRS_FLV_TAG_HEADER_SIZE)) != ERROR_SUCCESS) {
            return ret;
        }
        int8_t tag_type   = tag_stream->read_1bytes();
        int32_t data_size = tag_stream->read_3bytes();
        bool is_video     = tag_type == 0x09;
        bool is_audio     = tag_type == 0x08;
        bool is_not_av    = !is_video && !is_audio;
        if (is_not_av) {
            reader->skip(data_size + SRS_FLV_PREVIOUS_TAG_SIZE);
            continue;
        }
        if (is_video && got_video) {
            break;
        }
        if (is_audio && got_audio) {
            break;
        }
        if (is_video) {
            srs_assert(!got_video);
            got_video = true;
            if (av_sequence_offset_start < 0) {
                av_sequence_offset_start = reader->tellg() - SRS_FLV_TAG_HEADER_SIZE;
            }
            av_sequence_offset_end = reader->tellg() + data_size + SRS_FLV_PREVIOUS_TAG_SIZE;
            reader->skip(data_size + SRS_FLV_PREVIOUS_TAG_SIZE);
        }
        if (is_audio) {
            srs_assert(!got_audio);
            got_audio = true;
            if (av_sequence_offset_start < 0) {
                av_sequence_offset_start = reader->tellg() - SRS_FLV_TAG_HEADER_SIZE;
            }
            av_sequence_offset_end = reader->tellg() + data_size + SRS_FLV_PREVIOUS_TAG_SIZE;
            reader->skip(data_size + SRS_FLV_PREVIOUS_TAG_SIZE);
        }
    }
    if (av_sequence_offset_start > 0) {
        reader->lseek(av_sequence_offset_start);
        *pstart = av_sequence_offset_start;
        *psize  = (int) (av_sequence_offset_end - av_sequence_offset_start);
    }
    return ret;
} // SrsFlvVodStreamDecoder::read_sequence_header_summary

int SrsFlvVodStreamDecoder::lseek(int64_t offset)
{
    int ret = ERROR_SUCCESS;

    if (offset >= reader->filesize()) {
        ret = ERROR_SYSTEM_FILE_EOF;
        srs_warn("flv fast decoder seek overflow file, "
                 "size=%" PRId64 ", offset=%" PRId64 ", ret=%d",
                 reader->filesize(), offset, ret);
        return ret;
    }
    if (reader->lseek(offset) < 0) {
        ret = ERROR_SYSTEM_FILE_SEEK;
        srs_warn("flv fast decoder seek error, "
                 "size=%" PRId64 ", offset=%" PRId64 ", ret=%d",
                 reader->filesize(), offset, ret);
        return ret;
    }
    return ret;
}

#include <string.h>
#include <stdlib.h>
using namespace std;
string srs_codec_video2str(SrsCodecVideo codec)
{
    switch (codec) {
        case SrsCodecVideoAVC:
            return "H264";

        case SrsCodecVideoOn2VP6:
        case SrsCodecVideoOn2VP6WithAlphaChannel:
            return "VP6";

        case SrsCodecVideoReserved:
        case SrsCodecVideoReserved1:
        case SrsCodecVideoReserved2:
        case SrsCodecVideoDisabled:
        case SrsCodecVideoSorensonH263:
        case SrsCodecVideoScreenVideo:
        case SrsCodecVideoScreenVideoVersion2:
        default:
            return "Other";
    }
}

string srs_codec_audio2str(SrsCodecAudio codec)
{
    switch (codec) {
        case SrsCodecAudioAAC:
            return "AAC";

        case SrsCodecAudioMP3:
            return "MP3";

        case SrsCodecAudioReserved1:
        case SrsCodecAudioLinearPCMPlatformEndian:
        case SrsCodecAudioADPCM:
        case SrsCodecAudioLinearPCMLittleEndian:
        case SrsCodecAudioNellymoser16kHzMono:
        case SrsCodecAudioNellymoser8kHzMono:
        case SrsCodecAudioNellymoser:
        case SrsCodecAudioReservedG711AlawLogarithmicPCM:
        case SrsCodecAudioReservedG711MuLawLogarithmicPCM:
        case SrsCodecAudioReserved:
        case SrsCodecAudioSpeex:
        case SrsCodecAudioReservedMP3_8kHz:
        case SrsCodecAudioReservedDeviceSpecificSound:
        default:
            return "Other";
    }
}

string srs_codec_aac_profile2str(SrsAacProfile aac_profile)
{
    switch (aac_profile) {
        case SrsAacProfileMain:
            return "Main";

        case SrsAacProfileLC:
            return "LC";

        case SrsAacProfileSSR:
            return "SSR";

        default:
            return "Other";
    }
}

string srs_codec_aac_object2str(SrsAacObjectType aac_object)
{
    switch (aac_object) {
        case SrsAacObjectTypeAacMain:
            return "Main";

        case SrsAacObjectTypeAacHE:
            return "HE";

        case SrsAacObjectTypeAacHEV2:
            return "HEv2";

        case SrsAacObjectTypeAacLC:
            return "LC";

        case SrsAacObjectTypeAacSSR:
            return "SSR";

        default:
            return "Other";
    }
}

SrsAacObjectType srs_codec_aac_ts2rtmp(SrsAacProfile profile)
{
    switch (profile) {
        case SrsAacProfileMain:
            return SrsAacObjectTypeAacMain;

        case SrsAacProfileLC:
            return SrsAacObjectTypeAacLC;

        case SrsAacProfileSSR:
            return SrsAacObjectTypeAacSSR;

        default:
            return SrsAacObjectTypeReserved;
    }
}

SrsAacProfile srs_codec_aac_rtmp2ts(SrsAacObjectType object_type)
{
    switch (object_type) {
        case SrsAacObjectTypeAacMain:
            return SrsAacProfileMain;

        case SrsAacObjectTypeAacHE:
        case SrsAacObjectTypeAacHEV2:
        case SrsAacObjectTypeAacLC:
            return SrsAacProfileLC;

        case SrsAacObjectTypeAacSSR:
            return SrsAacProfileSSR;

        default:
            return SrsAacProfileReserved;
    }
}

string srs_codec_avc_profile2str(SrsAvcProfile profile)
{
    switch (profile) {
        case SrsAvcProfileBaseline:
            return "Baseline";

        case SrsAvcProfileConstrainedBaseline:
            return "Baseline(Constrained)";

        case SrsAvcProfileMain:
            return "Main";

        case SrsAvcProfileExtended:
            return "Extended";

        case SrsAvcProfileHigh:
            return "High";

        case SrsAvcProfileHigh10:
            return "High(10)";

        case SrsAvcProfileHigh10Intra:
            return "High(10+Intra)";

        case SrsAvcProfileHigh422:
            return "High(422)";

        case SrsAvcProfileHigh422Intra:
            return "High(422+Intra)";

        case SrsAvcProfileHigh444:
            return "High(444)";

        case SrsAvcProfileHigh444Predictive:
            return "High(444+Predictive)";

        case SrsAvcProfileHigh444Intra:
            return "High(444+Intra)";

        default:
            return "Other";
    }
} // srs_codec_avc_profile2str

string srs_codec_avc_level2str(SrsAvcLevel level)
{
    switch (level) {
        case SrsAvcLevel_1:
            return "1";

        case SrsAvcLevel_11:
            return "1.1";

        case SrsAvcLevel_12:
            return "1.2";

        case SrsAvcLevel_13:
            return "1.3";

        case SrsAvcLevel_2:
            return "2";

        case SrsAvcLevel_21:
            return "2.1";

        case SrsAvcLevel_22:
            return "2.2";

        case SrsAvcLevel_3:
            return "3";

        case SrsAvcLevel_31:
            return "3.1";

        case SrsAvcLevel_32:
            return "3.2";

        case SrsAvcLevel_4:
            return "4";

        case SrsAvcLevel_41:
            return "4.1";

        case SrsAvcLevel_5:
            return "5";

        case SrsAvcLevel_51:
            return "5.1";

        default:
            return "Other";
    }
} // srs_codec_avc_level2str

int flv_sample_rates[] = { 5512, 11025, 22050, 44100 };
int aac_sample_rates[] = {
    96000, 88200, 64000, 48000,
    44100, 32000, 24000, 22050,
    16000, 12000, 11025, 8000,
    7350,  0,     0,     0
};
SrsFlvCodec::SrsFlvCodec()
{}

SrsFlvCodec::~SrsFlvCodec()
{}

bool SrsFlvCodec::video_is_keyframe(char* data, int size)
{
    if (size < 1) {
        return false;
    }
    char frame_type = data[0];
    frame_type = (frame_type >> 4) & 0x0F;
    return frame_type == SrsCodecVideoAVCFrameKeyFrame;
}

bool SrsFlvCodec::video_is_sequence_header(char* data, int size)
{
    if (!video_is_h264(data, size)) {
        return false;
    }
    if (size < 2) {
        return false;
    }
    char frame_type = data[0];
    frame_type = (frame_type >> 4) & 0x0F;
    char avc_packet_type = data[1];
    return frame_type == SrsCodecVideoAVCFrameKeyFrame &&
           avc_packet_type == SrsCodecVideoAVCTypeSequenceHeader;
}

bool SrsFlvCodec::audio_is_sequence_header(char* data, int size)
{
    if (!audio_is_aac(data, size)) {
        return false;
    }
    if (size < 2) {
        return false;
    }
    char aac_packet_type = data[1];
    return aac_packet_type == SrsCodecAudioTypeSequenceHeader;
}

bool SrsFlvCodec::video_is_h264(char* data, int size)
{
    if (size < 1) {
        return false;
    }
    char codec_id = data[0];
    codec_id = codec_id & 0x0F;
    return codec_id == SrsCodecVideoAVC;
}

bool SrsFlvCodec::audio_is_aac(char* data, int size)
{
    if (size < 1) {
        return false;
    }
    char sound_format = data[0];
    sound_format = (sound_format >> 4) & 0x0F;
    return sound_format == SrsCodecAudioAAC;
}

bool SrsFlvCodec::video_is_acceptable(char* data, int size)
{
    if (size < 1) {
        return false;
    }
    char frame_type = data[0];
    char codec_id   = frame_type & 0x0f;
    frame_type = (frame_type >> 4) & 0x0f;
    if (frame_type < 1 || frame_type > 5) {
        return false;
    }
    if (codec_id < 2 || codec_id > 7) {
        return false;
    }
    return true;
}

string srs_codec_avc_nalu2str(SrsAvcNaluType nalu_type)
{
    switch (nalu_type) {
        case SrsAvcNaluTypeNonIDR:
            return "NonIDR";

        case SrsAvcNaluTypeDataPartitionA:
            return "DataPartitionA";

        case SrsAvcNaluTypeDataPartitionB:
            return "DataPartitionB";

        case SrsAvcNaluTypeDataPartitionC:
            return "DataPartitionC";

        case SrsAvcNaluTypeIDR:
            return "IDR";

        case SrsAvcNaluTypeSEI:
            return "SEI";

        case SrsAvcNaluTypeSPS:
            return "SPS";

        case SrsAvcNaluTypePPS:
            return "PPS";

        case SrsAvcNaluTypeAccessUnitDelimiter:
            return "AccessUnitDelimiter";

        case SrsAvcNaluTypeEOSequence:
            return "EOSequence";

        case SrsAvcNaluTypeEOStream:
            return "EOStream";

        case SrsAvcNaluTypeFilterData:
            return "FilterData";

        case SrsAvcNaluTypeSPSExt:
            return "SPSExt";

        case SrsAvcNaluTypePrefixNALU:
            return "PrefixNALU";

        case SrsAvcNaluTypeSubsetSPS:
            return "SubsetSPS";

        case SrsAvcNaluTypeLayerWithoutPartition:
            return "LayerWithoutPartition";

        case SrsAvcNaluTypeCodedSliceExt:
            return "CodedSliceExt";

        case SrsAvcNaluTypeReserved:
        default:
            return "Other";
    }
} // srs_codec_avc_nalu2str

SrsCodecSampleUnit::SrsCodecSampleUnit()
{
    size  = 0;
    bytes = NULL;
}

SrsCodecSampleUnit::~SrsCodecSampleUnit()
{}

SrsCodecSample::SrsCodecSample()
{
    clear();
}

SrsCodecSample::~SrsCodecSample()
{}

void SrsCodecSample::clear()
{
    is_video        = false;
    nb_sample_units = 0;
    cts             = 0;
    frame_type      = SrsCodecVideoAVCFrameReserved;
    avc_packet_type = SrsCodecVideoAVCTypeReserved;
    has_idr         = false;
    first_nalu_type = SrsAvcNaluTypeReserved;
    acodec          = SrsCodecAudioReserved1;
    sound_rate      = SrsCodecAudioSampleRateReserved;
    sound_size      = SrsCodecAudioSampleSizeReserved;
    sound_type      = SrsCodecAudioSoundTypeReserved;
    aac_packet_type = SrsCodecAudioTypeReserved;
}

int SrsCodecSample::add_sample_unit(char* bytes, int size)
{
    int ret = ERROR_SUCCESS;

    if (nb_sample_units >= SRS_SRS_MAX_CODEC_SAMPLE) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("hls decode samples error, "
                  "exceed the max count: %d, ret=%d", SRS_SRS_MAX_CODEC_SAMPLE, ret);
        return ret;
    }
    SrsCodecSampleUnit* sample_unit = &sample_units[nb_sample_units++];
    sample_unit->bytes = bytes;
    sample_unit->size  = size;
    if (is_video) {
        SrsAvcNaluType nal_unit_type = (SrsAvcNaluType) (bytes[0] & 0x1f);
        if (nal_unit_type == SrsAvcNaluTypeIDR) {
            has_idr = true;
        }
        if (first_nalu_type == SrsAvcNaluTypeReserved) {
            first_nalu_type = nal_unit_type;
        }
    }
    return ret;
}

#if !defined(SRS_EXPORT_LIBRTMP) || 1
SrsAvcAacCodec::SrsAvcAacCodec()
{
    avc_parse_sps   = true;
    width           = 0;
    height          = 0;
    duration        = 0;
    NAL_unit_length = 0;
    frame_rate      = 0;
    video_data_rate = 0;
    video_codec_id  = 0;
    audio_data_rate = 0;
    audio_codec_id  = 0;
    avc_profile     = SrsAvcProfileReserved;
    avc_level       = SrsAvcLevelReserved;
    aac_object      = SrsAacObjectTypeReserved;
    aac_sample_rate = SRS_AAC_SAMPLE_RATE_UNSET;
    aac_channels    = 0;
    avc_extra_size  = 0;
    avc_extra_data  = NULL;
    aac_extra_size  = 0;
    aac_extra_data  = NULL;
    sequenceParameterSetLength  = 0;
    sequenceParameterSetNALUnit = NULL;
    pictureParameterSetLength   = 0;
    pictureParameterSetNALUnit  = NULL;
    payload_format = SrsAvcPayloadFormatGuess;
    stream         = new SrsStream();
}

SrsAvcAacCodec::~SrsAvcAacCodec()
{
    srs_freepa(avc_extra_data);
    srs_freepa(aac_extra_data);
    srs_freep(stream);
    srs_freepa(sequenceParameterSetNALUnit);
    srs_freepa(pictureParameterSetNALUnit);
}

bool SrsAvcAacCodec::is_avc_codec_ok()
{
    return avc_extra_size > 0 && avc_extra_data;
}

bool SrsAvcAacCodec::is_aac_codec_ok()
{
    return aac_extra_size > 0 && aac_extra_data;
}

int SrsAvcAacCodec::audio_aac_demux(char* data, int size, SrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;

    sample->is_video = false;
    if (!data || size <= 0) {
        srs_trace("no audio present, ignore it.");
        return ret;
    }
    if ((ret = stream->initialize(data, size)) != ERROR_SUCCESS) {
        return ret;
    }
    if (!stream->require(1)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("aac decode sound_format failed. ret=%d", ret);
        return ret;
    }
    int8_t sound_format = stream->read_1bytes();
    int8_t sound_type   = sound_format & 0x01;
    int8_t sound_size   = (sound_format >> 1) & 0x01;
    int8_t sound_rate   = (sound_format >> 2) & 0x03;
    sound_format       = (sound_format >> 4) & 0x0f;
    audio_codec_id     = sound_format;
    sample->acodec     = (SrsCodecAudio) audio_codec_id;
    sample->sound_type = (SrsCodecAudioSoundType) sound_type;
    sample->sound_rate = (SrsCodecAudioSampleRate) sound_rate;
    sample->sound_size = (SrsCodecAudioSampleSize) sound_size;
    if (audio_codec_id == SrsCodecAudioMP3) {
        return ERROR_HLS_TRY_MP3;
    }
    if (audio_codec_id != SrsCodecAudioAAC) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("aac only support mp3/aac codec. actual=%d, ret=%d", audio_codec_id, ret);
        return ret;
    }
    if (!stream->require(1)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("aac decode aac_packet_type failed. ret=%d", ret);
        return ret;
    }
    int8_t aac_packet_type = stream->read_1bytes();
    sample->aac_packet_type = (SrsCodecAudioType) aac_packet_type;
    if (aac_packet_type == SrsCodecAudioTypeSequenceHeader) {
        aac_extra_size = stream->size() - stream->pos();
        if (aac_extra_size > 0) {
            srs_freepa(aac_extra_data);
            aac_extra_data = new char[aac_extra_size];
            memcpy(aac_extra_data, stream->data() + stream->pos(), aac_extra_size);
            if ((ret = audio_aac_sequence_header_demux(aac_extra_data, aac_extra_size)) != ERROR_SUCCESS) {
                return ret;
            }
        }
    } else if (aac_packet_type == SrsCodecAudioTypeRawData) {
        if (!is_aac_codec_ok()) {
            srs_warn("aac ignore type=%d for no sequence header. ret=%d", aac_packet_type, ret);
            return ret;
        }
        if ((ret =
                 sample->add_sample_unit(stream->data() + stream->pos(),
                                         stream->size() - stream->pos())) != ERROR_SUCCESS)
        {
            srs_error("aac add sample failed. ret=%d", ret);
            return ret;
        }
    } else {}
    if (aac_sample_rate != SRS_AAC_SAMPLE_RATE_UNSET) {
        static int aac_sample_rates[] = {
            96000, 88200, 64000, 48000,
            44100, 32000, 24000, 22050,
            16000, 12000, 11025, 8000,
            7350,  0,     0,     0
        };
        switch (aac_sample_rates[aac_sample_rate]) {
            case 11025:
                sample->sound_rate = SrsCodecAudioSampleRate11025;
                break;
            case 22050:
                sample->sound_rate = SrsCodecAudioSampleRate22050;
                break;
            case 44100:
                sample->sound_rate = SrsCodecAudioSampleRate44100;
                break;
            default:
                break;
        }
        ;
    }
    srs_info("aac decoded, type=%d, codec=%d, asize=%d, rate=%d, format=%d, size=%d",
             sound_type, audio_codec_id, sound_size, sound_rate, sound_format, size);
    return ret;
} // SrsAvcAacCodec::audio_aac_demux

int SrsAvcAacCodec::audio_mp3_demux(char* data, int size, SrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;

    srs_assert(sample->acodec == SrsCodecAudioMP3);
    if (!data || size <= 1) {
        srs_trace("no mp3 audio present, ignore it.");
        return ret;
    }
    if ((ret = sample->add_sample_unit(data + 1, size - 1)) != ERROR_SUCCESS) {
        srs_error("audio codec add mp3 sample failed. ret=%d", ret);
        return ret;
    }
    srs_info("audio decoded, type=%d, codec=%d, asize=%d, rate=%d, format=%d, size=%d",
             sample->sound_type, audio_codec_id, sample->sound_size, sample->sound_rate, sample->acodec, size);
    return ret;
}

int SrsAvcAacCodec::audio_aac_sequence_header_demux(char* data, int size)
{
    int ret = ERROR_SUCCESS;

    if ((ret = stream->initialize(data, size)) != ERROR_SUCCESS) {
        return ret;
    }
    if (!stream->require(2)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("audio codec decode aac sequence header failed. ret=%d", ret);
        return ret;
    }
    u_int8_t profile_ObjectType     = stream->read_1bytes();
    u_int8_t samplingFrequencyIndex = stream->read_1bytes();
    aac_channels = (samplingFrequencyIndex >> 3) & 0x0f;
    samplingFrequencyIndex = ((profile_ObjectType << 1) & 0x0e) | ((samplingFrequencyIndex >> 7) & 0x01);
    profile_ObjectType     = (profile_ObjectType >> 3) & 0x1f;
    aac_sample_rate        = samplingFrequencyIndex;
    aac_object = (SrsAacObjectType) profile_ObjectType;
    if (aac_object == SrsAacObjectTypeReserved) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("audio codec decode aac sequence header failed, "
                  "adts object=%d invalid. ret=%d", profile_ObjectType, ret);
        return ret;
    }
    return ret;
} // SrsAvcAacCodec::audio_aac_sequence_header_demux

int SrsAvcAacCodec::video_avc_demux(char* data, int size, SrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;

    sample->is_video = true;
    if (!data || size <= 0) {
        srs_trace("no video present, ignore it.");
        return ret;
    }
    if ((ret = stream->initialize(data, size)) != ERROR_SUCCESS) {
        return ret;
    }
    if (!stream->require(1)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc decode frame_type failed. ret=%d", ret);
        return ret;
    }
    int8_t frame_type = stream->read_1bytes();
    int8_t codec_id   = frame_type & 0x0f;
    frame_type         = (frame_type >> 4) & 0x0f;
    sample->frame_type = (SrsCodecVideoAVCFrame) frame_type;
    if (sample->frame_type == SrsCodecVideoAVCFrameVideoInfoFrame) {
        srs_warn("avc igone the info frame, ret=%d", ret);
        return ret;
    }
    if (codec_id != SrsCodecVideoAVC) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc only support video h.264/avc codec. actual=%d, ret=%d", codec_id, ret);
        return ret;
    }
    video_codec_id = codec_id;
    if (!stream->require(4)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc decode avc_packet_type failed. ret=%d", ret);
        return ret;
    }
    int8_t avc_packet_type   = stream->read_1bytes();
    int32_t composition_time = stream->read_3bytes();
    sample->cts = composition_time;
    sample->avc_packet_type = (SrsCodecVideoAVCType) avc_packet_type;
    if (avc_packet_type == SrsCodecVideoAVCTypeSequenceHeader) {
        if ((ret = avc_demux_sps_pps(stream)) != ERROR_SUCCESS) {
            return ret;
        }
    } else if (avc_packet_type == SrsCodecVideoAVCTypeNALU) {
        if (!is_avc_codec_ok()) {
            srs_warn("avc ignore type=%d for no sequence header. ret=%d", avc_packet_type, ret);
            return ret;
        }
        if (payload_format == SrsAvcPayloadFormatGuess) {
            if ((ret = avc_demux_annexb_format(stream, sample)) != ERROR_SUCCESS) {
                if (ret != ERROR_HLS_AVC_TRY_OTHERS) {
                    srs_error("avc demux for annexb failed. ret=%d", ret);
                    return ret;
                }
                if ((ret = avc_demux_ibmf_format(stream, sample)) != ERROR_SUCCESS) {
                    return ret;
                } else {
                    payload_format = SrsAvcPayloadFormatIbmf;
                    srs_info("hls guess avc payload is ibmf format.");
                }
            } else {
                payload_format = SrsAvcPayloadFormatAnnexb;
                srs_info("hls guess avc payload is annexb format.");
            }
        } else if (payload_format == SrsAvcPayloadFormatIbmf) {
            if ((ret = avc_demux_ibmf_format(stream, sample)) != ERROR_SUCCESS) {
                return ret;
            }
            srs_info("hls decode avc payload in ibmf format.");
        } else {
            if ((ret = avc_demux_annexb_format(stream, sample)) != ERROR_SUCCESS) {
                if (ret != ERROR_HLS_AVC_TRY_OTHERS) {
                    srs_error("avc demux for annexb failed. ret=%d", ret);
                    return ret;
                }
                if ((ret = avc_demux_ibmf_format(stream, sample)) != ERROR_SUCCESS) {
                    return ret;
                } else {
                    payload_format = SrsAvcPayloadFormatIbmf;
                    srs_warn("hls avc payload change from annexb to ibmf format.");
                }
            }
            srs_info("hls decode avc payload in annexb format.");
        }
    } else {}
    srs_info("avc decoded, type=%d, codec=%d, avc=%d, cts=%d, size=%d",
             frame_type, video_codec_id, avc_packet_type, composition_time, size);
    return ret;
} // SrsAvcAacCodec::video_avc_demux

int SrsAvcAacCodec::avc_demux_sps_pps(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    avc_extra_size = stream->size() - stream->pos();
    if (avc_extra_size > 0) {
        srs_freepa(avc_extra_data);
        avc_extra_data = new char[avc_extra_size];
        memcpy(avc_extra_data, stream->data() + stream->pos(), avc_extra_size);
    }
    if (!stream->require(6)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc decode sequenc header failed. ret=%d", ret);
        return ret;
    }
    stream->read_1bytes();
    avc_profile = (SrsAvcProfile) stream->read_1bytes();
    stream->read_1bytes();
    avc_level = (SrsAvcLevel) stream->read_1bytes();
    int8_t lengthSizeMinusOne = stream->read_1bytes();
    lengthSizeMinusOne &= 0x03;
    NAL_unit_length     = lengthSizeMinusOne;
    if (NAL_unit_length == 2) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("sps lengthSizeMinusOne should never be 2. ret=%d", ret);
        return ret;
    }
    if (!stream->require(1)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc decode sequenc header sps failed. ret=%d", ret);
        return ret;
    }
    int8_t numOfSequenceParameterSets = stream->read_1bytes();
    numOfSequenceParameterSets &= 0x1f;
    if (numOfSequenceParameterSets != 1) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc decode sequenc header sps failed. ret=%d", ret);
        return ret;
    }
    if (!stream->require(2)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc decode sequenc header sps size failed. ret=%d", ret);
        return ret;
    }
    sequenceParameterSetLength = stream->read_2bytes();
    if (!stream->require(sequenceParameterSetLength)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc decode sequenc header sps data failed. ret=%d", ret);
        return ret;
    }
    if (sequenceParameterSetLength > 0) {
        srs_freepa(sequenceParameterSetNALUnit);
        sequenceParameterSetNALUnit = new char[sequenceParameterSetLength];
        stream->read_bytes(sequenceParameterSetNALUnit, sequenceParameterSetLength);
    }
    if (!stream->require(1)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc decode sequenc header pps failed. ret=%d", ret);
        return ret;
    }
    int8_t numOfPictureParameterSets = stream->read_1bytes();
    numOfPictureParameterSets &= 0x1f;
    if (numOfPictureParameterSets != 1) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc decode sequenc header pps failed. ret=%d", ret);
        return ret;
    }
    if (!stream->require(2)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc decode sequenc header pps size failed. ret=%d", ret);
        return ret;
    }
    pictureParameterSetLength = stream->read_2bytes();
    if (!stream->require(pictureParameterSetLength)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc decode sequenc header pps data failed. ret=%d", ret);
        return ret;
    }
    if (pictureParameterSetLength > 0) {
        srs_freepa(pictureParameterSetNALUnit);
        pictureParameterSetNALUnit = new char[pictureParameterSetLength];
        stream->read_bytes(pictureParameterSetNALUnit, pictureParameterSetLength);
    }
    return avc_demux_sps();
} // SrsAvcAacCodec::avc_demux_sps_pps

int SrsAvcAacCodec::avc_demux_sps()
{
    int ret = ERROR_SUCCESS;

    if (!sequenceParameterSetLength) {
        return ret;
    }
    SrsStream stream;
    if ((ret = stream.initialize(sequenceParameterSetNALUnit, sequenceParameterSetLength)) != ERROR_SUCCESS) {
        return ret;
    }
    if (!stream.require(1)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("avc decode sps failed. ret=%d", ret);
        return ret;
    }
    int8_t nutv = stream.read_1bytes();
    int8_t forbidden_zero_bit = (nutv >> 7) & 0x01;
    if (forbidden_zero_bit) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("forbidden_zero_bit shall be equal to 0. ret=%d", ret);
        return ret;
    }
    int8_t nal_ref_idc = (nutv >> 5) & 0x03;
    if (!nal_ref_idc) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("for sps, nal_ref_idc shall be not be equal to 0. ret=%d", ret);
        return ret;
    }
    SrsAvcNaluType nal_unit_type = (SrsAvcNaluType) (nutv & 0x1f);
    if (nal_unit_type != 7) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("for sps, nal_unit_type shall be equal to 7. ret=%d", ret);
        return ret;
    }
    int8_t* rbsp = new int8_t[sequenceParameterSetLength];
    SrsAutoFreeA(int8_t, rbsp);
    int nb_rbsp = 0;
    while (!stream.empty()) {
        rbsp[nb_rbsp] = stream.read_1bytes();
        if (nb_rbsp > 2 && rbsp[nb_rbsp - 2] == 0 && rbsp[nb_rbsp - 1] == 0 && rbsp[nb_rbsp] == 3) {
            if (stream.empty()) {
                break;
            }
            rbsp[nb_rbsp] = stream.read_1bytes();
            nb_rbsp++;
            continue;
        }
        nb_rbsp++;
    }
    return avc_demux_sps_rbsp((char *) rbsp, nb_rbsp);
} // SrsAvcAacCodec::avc_demux_sps

int SrsAvcAacCodec::avc_demux_sps_rbsp(char* rbsp, int nb_rbsp)
{
    int ret = ERROR_SUCCESS;

    if (!avc_parse_sps) {
        return ret;
    }
    SrsStream stream;
    if ((ret = stream.initialize(rbsp, nb_rbsp)) != ERROR_SUCCESS) {
        return ret;
    }
    if (!stream.require(3)) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("sps shall atleast 3bytes. ret=%d", ret);
        return ret;
    }
    u_int8_t profile_idc = stream.read_1bytes();
    if (!profile_idc) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("sps the profile_idc invalid. ret=%d", ret);
        return ret;
    }
    int8_t flags = stream.read_1bytes();
    if (flags & 0x03) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("sps the flags invalid. ret=%d", ret);
        return ret;
    }
    u_int8_t level_idc = stream.read_1bytes();
    if (!level_idc) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("sps the level_idc invalid. ret=%d", ret);
        return ret;
    }
    SrsBitStream bs;
    if ((ret = bs.initialize(&stream)) != ERROR_SUCCESS) {
        return ret;
    }
    int32_t seq_parameter_set_id = -1;
    if ((ret = srs_avc_nalu_read_uev(&bs, seq_parameter_set_id)) != ERROR_SUCCESS) {
        return ret;
    }
    if (seq_parameter_set_id < 0) {
        ret = ERROR_HLS_DECODE_ERROR;
        srs_error("sps the seq_parameter_set_id invalid. ret=%d", ret);
        return ret;
    }
    srs_info("sps parse profile=%d, level=%d, sps_id=%d", profile_idc, level_idc, seq_parameter_set_id);
    int32_t chroma_format_idc = -1;
    if (profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 244 ||
        profile_idc == 44 || profile_idc == 83 || profile_idc == 86 || profile_idc == 118 ||
        profile_idc == 128
    )
    {
        if ((ret = srs_avc_nalu_read_uev(&bs, chroma_format_idc)) != ERROR_SUCCESS) {
            return ret;
        }
        if (chroma_format_idc == 3) {
            int8_t separate_colour_plane_flag = -1;
            if ((ret = srs_avc_nalu_read_bit(&bs, separate_colour_plane_flag)) != ERROR_SUCCESS) {
                return ret;
            }
        }
        int32_t bit_depth_luma_minus8 = -1;
        if ((ret = srs_avc_nalu_read_uev(&bs, bit_depth_luma_minus8)) != ERROR_SUCCESS) {
            return ret;
        }
        int32_t bit_depth_chroma_minus8 = -1;
        if ((ret = srs_avc_nalu_read_uev(&bs, bit_depth_chroma_minus8)) != ERROR_SUCCESS) {
            return ret;
        }
        int8_t qpprime_y_zero_transform_bypass_flag = -1;
        if ((ret = srs_avc_nalu_read_bit(&bs, qpprime_y_zero_transform_bypass_flag)) != ERROR_SUCCESS) {
            return ret;
        }
        int8_t seq_scaling_matrix_present_flag = -1;
        if ((ret = srs_avc_nalu_read_bit(&bs, seq_scaling_matrix_present_flag)) != ERROR_SUCCESS) {
            return ret;
        }
        if (seq_scaling_matrix_present_flag) {
            int nb_scmpfs = ((chroma_format_idc != 3) ? 8 : 12);
            for (int i = 0; i < nb_scmpfs; i++) {
                int8_t seq_scaling_matrix_present_flag_i = -1;
                if ((ret = srs_avc_nalu_read_bit(&bs, seq_scaling_matrix_present_flag_i)) != ERROR_SUCCESS) {
                    return ret;
                }
            }
        }
    }
    int32_t log2_max_frame_num_minus4 = -1;
    if ((ret = srs_avc_nalu_read_uev(&bs, log2_max_frame_num_minus4)) != ERROR_SUCCESS) {
        return ret;
    }
    int32_t pic_order_cnt_type = -1;
    if ((ret = srs_avc_nalu_read_uev(&bs, pic_order_cnt_type)) != ERROR_SUCCESS) {
        return ret;
    }
    if (pic_order_cnt_type == 0) {
        int32_t log2_max_pic_order_cnt_lsb_minus4 = -1;
        if ((ret = srs_avc_nalu_read_uev(&bs, log2_max_pic_order_cnt_lsb_minus4)) != ERROR_SUCCESS) {
            return ret;
        }
    } else if (pic_order_cnt_type == 1) {
        int8_t delta_pic_order_always_zero_flag = -1;
        if ((ret = srs_avc_nalu_read_bit(&bs, delta_pic_order_always_zero_flag)) != ERROR_SUCCESS) {
            return ret;
        }
        int32_t offset_for_non_ref_pic = -1;
        if ((ret = srs_avc_nalu_read_uev(&bs, offset_for_non_ref_pic)) != ERROR_SUCCESS) {
            return ret;
        }
        int32_t offset_for_top_to_bottom_field = -1;
        if ((ret = srs_avc_nalu_read_uev(&bs, offset_for_top_to_bottom_field)) != ERROR_SUCCESS) {
            return ret;
        }
        int32_t num_ref_frames_in_pic_order_cnt_cycle = -1;
        if ((ret = srs_avc_nalu_read_uev(&bs, num_ref_frames_in_pic_order_cnt_cycle)) != ERROR_SUCCESS) {
            return ret;
        }
        if (num_ref_frames_in_pic_order_cnt_cycle < 0) {
            ret = ERROR_HLS_DECODE_ERROR;
            srs_error("sps the num_ref_frames_in_pic_order_cnt_cycle invalid. ret=%d", ret);
            return ret;
        }
        for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
            int32_t offset_for_ref_frame_i = -1;
            if ((ret = srs_avc_nalu_read_uev(&bs, offset_for_ref_frame_i)) != ERROR_SUCCESS) {
                return ret;
            }
        }
    }
    int32_t max_num_ref_frames = -1;
    if ((ret = srs_avc_nalu_read_uev(&bs, max_num_ref_frames)) != ERROR_SUCCESS) {
        return ret;
    }
    int8_t gaps_in_frame_num_value_allowed_flag = -1;
    if ((ret = srs_avc_nalu_read_bit(&bs, gaps_in_frame_num_value_allowed_flag)) != ERROR_SUCCESS) {
        return ret;
    }
    int32_t pic_width_in_mbs_minus1 = -1;
    if ((ret = srs_avc_nalu_read_uev(&bs, pic_width_in_mbs_minus1)) != ERROR_SUCCESS) {
        return ret;
    }
    int32_t pic_height_in_map_units_minus1 = -1;
    if ((ret = srs_avc_nalu_read_uev(&bs, pic_height_in_map_units_minus1)) != ERROR_SUCCESS) {
        return ret;
    }
    width  = (int) (pic_width_in_mbs_minus1 + 1) * 16;
    height = (int) (pic_height_in_map_units_minus1 + 1) * 16;
    return ret;
} // SrsAvcAacCodec::avc_demux_sps_rbsp

int SrsAvcAacCodec::avc_demux_annexb_format(SrsStream* stream, SrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;

    if (!srs_avc_startswith_annexb(stream, NULL)) {
        return ERROR_HLS_AVC_TRY_OTHERS;
    }
    while (!stream->empty()) {
        int nb_start_code = 0;
        if (!srs_avc_startswith_annexb(stream, &nb_start_code)) {
            return ret;
        }
        if (nb_start_code > 0) {
            stream->skip(nb_start_code);
        }
        char* p = stream->data() + stream->pos();
        while (!stream->empty()) {
            if (srs_avc_startswith_annexb(stream, NULL)) {
                break;
            }
            stream->skip(1);
        }
        char* pp = stream->data() + stream->pos();
        if (pp - p <= 0) {
            continue;
        }
        if ((ret = sample->add_sample_unit(p, pp - p)) != ERROR_SUCCESS) {
            srs_error("annexb add video sample failed. ret=%d", ret);
            return ret;
        }
    }
    return ret;
} // SrsAvcAacCodec::avc_demux_annexb_format

int SrsAvcAacCodec::avc_demux_ibmf_format(SrsStream* stream, SrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;
    int PictureLength = stream->size() - stream->pos();

    srs_assert(NAL_unit_length != 2);
    for (int i = 0; i < PictureLength;) {
        if (!stream->require(NAL_unit_length + 1)) {
            ret = ERROR_HLS_DECODE_ERROR;
            srs_error("avc decode NALU size failed. ret=%d", ret);
            return ret;
        }
        int32_t NALUnitLength = 0;
        if (NAL_unit_length == 3) {
            NALUnitLength = stream->read_4bytes();
        } else if (NAL_unit_length == 1) {
            NALUnitLength = stream->read_2bytes();
        } else {
            NALUnitLength = stream->read_1bytes();
        }
        if (NALUnitLength < 0) {
            ret = ERROR_HLS_DECODE_ERROR;
            srs_error("maybe stream is AnnexB format. ret=%d", ret);
            return ret;
        }
        if (!stream->require(NALUnitLength)) {
            ret = ERROR_HLS_DECODE_ERROR;
            srs_error("avc decode NALU data failed. ret=%d", ret);
            return ret;
        }
        if ((ret = sample->add_sample_unit(stream->data() + stream->pos(), NALUnitLength)) != ERROR_SUCCESS) {
            srs_error("avc add video sample failed. ret=%d", ret);
            return ret;
        }
        stream->skip(NALUnitLength);
        i += NAL_unit_length + 1 + NALUnitLength;
    }
    return ret;
} // SrsAvcAacCodec::avc_demux_ibmf_format

#endif // if !defined(SRS_EXPORT_LIBRTMP) || 1
#ifndef _WIN32
# include <unistd.h>
# include <sys/uio.h>
#endif
#include <fcntl.h>
#include <sstream>
using namespace std;
SrsFileWriter::SrsFileWriter()
{
    fd = -1;
}

SrsFileWriter::~SrsFileWriter()
{
    close();
}

int SrsFileWriter::open(string p)
{
    int ret = ERROR_SUCCESS;

    if (fd > 0) {
        ret = ERROR_SYSTEM_FILE_ALREADY_OPENED;
        srs_error("file %s already opened. ret=%d", path.c_str(), ret);
        return ret;
    }
    int flags   = O_CREAT | O_WRONLY | O_TRUNC;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    if ((fd = ::open(p.c_str(), flags, mode)) < 0) {
        ret = ERROR_SYSTEM_FILE_OPENE;
        srs_error("open file %s failed. ret=%d", p.c_str(), ret);
        return ret;
    }
    path = p;
    return ret;
}

int SrsFileWriter::open_append(string p)
{
    int ret = ERROR_SUCCESS;

    if (fd > 0) {
        ret = ERROR_SYSTEM_FILE_ALREADY_OPENED;
        srs_error("file %s already opened. ret=%d", path.c_str(), ret);
        return ret;
    }
    int flags   = O_APPEND | O_WRONLY;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    if ((fd = ::open(p.c_str(), flags, mode)) < 0) {
        ret = ERROR_SYSTEM_FILE_OPENE;
        srs_error("open file %s failed. ret=%d", p.c_str(), ret);
        return ret;
    }
    path = p;
    return ret;
}

void SrsFileWriter::close()
{
    int ret = ERROR_SUCCESS;

    if (fd < 0) {
        return;
    }
    if (::close(fd) < 0) {
        ret = ERROR_SYSTEM_FILE_CLOSE;
        srs_error("close file %s failed. ret=%d", path.c_str(), ret);
        return;
    }
    fd = -1;
}

bool SrsFileWriter::is_open()
{
    return fd > 0;
}

void SrsFileWriter::lseek(int64_t offset)
{
    ::lseek(fd, (off_t) offset, SEEK_SET);
}

int64_t SrsFileWriter::tellg()
{
    return (int64_t) ::lseek(fd, 0, SEEK_CUR);
}

int SrsFileWriter::write(void* buf, size_t count, ssize_t* pnwrite)
{
    int ret = ERROR_SUCCESS;
    ssize_t nwrite;

    if ((nwrite = ::write(fd, buf, count)) < 0) {
        ret = ERROR_SYSTEM_FILE_WRITE;
        srs_error("write to file %s failed. ret=%d", path.c_str(), ret);
        return ret;
    }
    if (pnwrite != NULL) {
        *pnwrite = nwrite;
    }
    return ret;
}

int SrsFileWriter::writev(iovec* iov, int iovcnt, ssize_t* pnwrite)
{
    int ret        = ERROR_SUCCESS;
    ssize_t nwrite = 0;

    for (int i = 0; i < iovcnt; i++) {
        iovec* piov         = iov + i;
        ssize_t this_nwrite = 0;
        if ((ret = write(piov->iov_base, piov->iov_len, &this_nwrite)) != ERROR_SUCCESS) {
            return ret;
        }
        nwrite += this_nwrite;
    }
    if (pnwrite) {
        *pnwrite = nwrite;
    }
    return ret;
}

SrsFileReader::SrsFileReader()
{
    fd = -1;
}

SrsFileReader::~SrsFileReader()
{
    close();
}

int SrsFileReader::open(string p)
{
    int ret = ERROR_SUCCESS;

    if (fd > 0) {
        ret = ERROR_SYSTEM_FILE_ALREADY_OPENED;
        srs_error("file %s already opened. ret=%d", path.c_str(), ret);
        return ret;
    }
    if ((fd = ::open(p.c_str(), O_RDONLY)) < 0) {
        ret = ERROR_SYSTEM_FILE_OPENE;
        srs_error("open file %s failed. ret=%d", p.c_str(), ret);
        return ret;
    }
    path = p;
    return ret;
}

void SrsFileReader::close()
{
    int ret = ERROR_SUCCESS;

    if (fd < 0) {
        return;
    }
    if (::close(fd) < 0) {
        ret = ERROR_SYSTEM_FILE_CLOSE;
        srs_error("close file %s failed. ret=%d", path.c_str(), ret);
        return;
    }
    fd = -1;
}

bool SrsFileReader::is_open()
{
    return fd > 0;
}

int64_t SrsFileReader::tellg()
{
    return (int64_t) ::lseek(fd, 0, SEEK_CUR);
}

void SrsFileReader::skip(int64_t size)
{
    ::lseek(fd, (off_t) size, SEEK_CUR);
}

int64_t SrsFileReader::lseek(int64_t offset)
{
    return (int64_t) ::lseek(fd, (off_t) offset, SEEK_SET);
}

int64_t SrsFileReader::filesize()
{
    int64_t cur  = tellg();
    int64_t size = (int64_t) ::lseek(fd, 0, SEEK_END);

    ::lseek(fd, (off_t) cur, SEEK_SET);
    return size;
}

int SrsFileReader::read(void* buf, size_t count, ssize_t* pnread)
{
    int ret = ERROR_SUCCESS;
    ssize_t nread;

    if ((nread = ::read(fd, buf, count)) < 0) {
        ret = ERROR_SYSTEM_FILE_READ;
        srs_error("read from file %s failed. ret=%d", path.c_str(), ret);
        return ret;
    }
    if (nread == 0) {
        ret = ERROR_SYSTEM_FILE_EOF;
        return ret;
    }
    if (pnread != NULL) {
        *pnread = nread;
    }
    return ret;
}

#if !defined(SRS_EXPORT_LIBRTMP)
# ifndef _WIN32
#  include <unistd.h>
# endif
# include <fcntl.h>
# include <sstream>
using namespace std;
SrsAacEncoder::SrsAacEncoder()
{
    _fs = NULL;
    got_sequence_header = false;
    tag_stream = new SrsStream();
    aac_object = SrsAacObjectTypeReserved;
}

SrsAacEncoder::~SrsAacEncoder()
{
    srs_freep(tag_stream);
}

int SrsAacEncoder::initialize(SrsFileWriter* fs)
{
    int ret = ERROR_SUCCESS;

    srs_assert(fs);
    if (!fs->is_open()) {
        ret = ERROR_KERNEL_AAC_STREAM_CLOSED;
        srs_warn("stream is not open for encoder. ret=%d", ret);
        return ret;
    }
    _fs = fs;
    return ret;
}

int SrsAacEncoder::write_audio(int64_t timestamp, char* data, int size)
{
    int ret = ERROR_SUCCESS;

    srs_assert(data);
    timestamp &= 0x7fffffff;
    SrsStream* stream = tag_stream;
    if ((ret = stream->initialize(data, size)) != ERROR_SUCCESS) {
        return ret;
    }
    if (!stream->require(1)) {
        ret = ERROR_AAC_DECODE_ERROR;
        srs_error("aac decode audio sound_format failed. ret=%d", ret);
        return ret;
    }
    int8_t sound_format = stream->read_1bytes();
    sound_format = (sound_format >> 4) & 0x0f;
    if ((SrsCodecAudio) sound_format != SrsCodecAudioAAC) {
        ret = ERROR_AAC_DECODE_ERROR;
        srs_error("aac required, format=%d. ret=%d", sound_format, ret);
        return ret;
    }
    if (!stream->require(1)) {
        ret = ERROR_AAC_DECODE_ERROR;
        srs_error("aac decode aac_packet_type failed. ret=%d", ret);
        return ret;
    }
    SrsCodecAudioType aac_packet_type = (SrsCodecAudioType) stream->read_1bytes();
    if (aac_packet_type == SrsCodecAudioTypeSequenceHeader) {
        if (!stream->require(2)) {
            ret = ERROR_AAC_DECODE_ERROR;
            srs_error("aac decode sequence header failed. ret=%d", ret);
            return ret;
        }
        int8_t audioObjectType = stream->read_1bytes();
        aac_sample_rate     = stream->read_1bytes();
        aac_channels        = (aac_sample_rate >> 3) & 0x0f;
        aac_sample_rate     = ((audioObjectType << 1) & 0x0e) | ((aac_sample_rate >> 7) & 0x01);
        audioObjectType     = (audioObjectType >> 3) & 0x1f;
        aac_object          = (SrsAacObjectType) audioObjectType;
        got_sequence_header = true;
        return ret;
    }
    if (!got_sequence_header) {
        ret = ERROR_AAC_DECODE_ERROR;
        srs_error("aac no sequence header. ret=%d", ret);
        return ret;
    }
    int16_t aac_raw_length = stream->size() - stream->pos();
    char aac_fixed_header[7];
    if (true) {
        char* pp = aac_fixed_header;
        int16_t aac_frame_length = aac_raw_length + 7;
        *pp++ = 0xff;
        *pp++ = 0xf1;
        SrsAacProfile aac_profile = srs_codec_aac_rtmp2ts(aac_object);
        *pp++ = ((aac_profile << 6) & 0xc0) | ((aac_sample_rate << 2) & 0x3c) | ((aac_channels >> 2) & 0x01);
        *pp++ = ((aac_channels << 6) & 0xc0) | ((aac_frame_length >> 11) & 0x03);
        *pp++ = aac_frame_length >> 3;
        *pp++ = (aac_frame_length << 5) & 0xe0;
        *pp++ = 0xfc;
    }
    if ((ret = _fs->write(aac_fixed_header, 7, NULL)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = _fs->write(data + stream->pos(), aac_raw_length, NULL)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
} // SrsAacEncoder::write_audio

#endif // if !defined(SRS_EXPORT_LIBRTMP)
#if !defined(SRS_EXPORT_LIBRTMP)
# ifndef _WIN32
#  include <unistd.h>
# endif
# include <fcntl.h>
# include <sstream>
using namespace std;
SrsMp3Encoder::SrsMp3Encoder()
{
    writer     = NULL;
    tag_stream = new SrsStream();
}

SrsMp3Encoder::~SrsMp3Encoder()
{
    srs_freep(tag_stream);
}

int SrsMp3Encoder::initialize(SrsFileWriter* fw)
{
    int ret = ERROR_SUCCESS;

    srs_assert(fw);
    if (!fw->is_open()) {
        ret = ERROR_KERNEL_MP3_STREAM_CLOSED;
        srs_warn("stream is not open for encoder. ret=%d", ret);
        return ret;
    }
    writer = fw;
    return ret;
}

int SrsMp3Encoder::write_header()
{
    char id3[] = {
        (char) 0x49, (char) 0x44, (char) 0x33,
        (char) 0x03, (char) 0x00,
        (char) 0x00,
        (char) 0x00, (char) 0x00, (char) 0x00,(char) 0x0a,
        (char) 0x00, (char) 0x00, (char) 0x00,(char) 0x00,
        (char) 0x00, (char) 0x00, (char) 0x00,(char) 0x00,
        (char) 0x00, (char) 0x00
    };

    return writer->write(id3, sizeof(id3), NULL);
}

int SrsMp3Encoder::write_audio(int64_t timestamp, char* data, int size)
{
    int ret = ERROR_SUCCESS;

    srs_assert(data);
    timestamp &= 0x7fffffff;
    SrsStream* stream = tag_stream;
    if ((ret = stream->initialize(data, size)) != ERROR_SUCCESS) {
        return ret;
    }
    if (!stream->require(1)) {
        ret = ERROR_MP3_DECODE_ERROR;
        srs_error("mp3 decode audio sound_format failed. ret=%d", ret);
        return ret;
    }
    int8_t sound_format = stream->read_1bytes();
    sound_format = (sound_format >> 4) & 0x0f;
    if ((SrsCodecAudio) sound_format != SrsCodecAudioMP3) {
        ret = ERROR_MP3_DECODE_ERROR;
        srs_error("mp3 required, format=%d. ret=%d", sound_format, ret);
        return ret;
    }
    if (!stream->require(1)) {
        ret = ERROR_MP3_DECODE_ERROR;
        srs_error("mp3 decode aac_packet_type failed. ret=%d", ret);
        return ret;
    }
    return writer->write(data + stream->pos(), size - stream->pos(), NULL);
} // SrsMp3Encoder::write_audio

#endif // if !defined(SRS_EXPORT_LIBRTMP)
#if !defined(SRS_EXPORT_LIBRTMP)
# ifndef _WIN32
#  include <unistd.h>
# endif
# include <fcntl.h>
# include <sstream>
using namespace std;
# define SRS_CONF_DEFAULT_AAC_SYNC 100
# define _SRS_AAC_SAMPLE_SIZE      1024
# define TS_PMT_NUMBER             1
# define TS_PMT_PID                0x1001
# define TS_VIDEO_AVC_PID          0x100
# define TS_AUDIO_AAC_PID          0x101
# define TS_AUDIO_MP3_PID          0x102
string srs_ts_stream2string(SrsTsStream stream)
{
    switch (stream) {
        case SrsTsStreamReserved:
            return "Reserved";

        case SrsTsStreamAudioMp3:
            return "MP3";

        case SrsTsStreamAudioAAC:
            return "AAC";

        case SrsTsStreamAudioAC3:
            return "AC3";

        case SrsTsStreamAudioDTS:
            return "AudioDTS";

        case SrsTsStreamVideoH264:
            return "H.264";

        case SrsTsStreamVideoMpeg4:
            return "MP4";

        case SrsTsStreamAudioMpeg4:
            return "MP4A";

        default:
            return "Other";
    }
}

SrsTsChannel::SrsTsChannel()
{
    pid    = 0;
    apply  = SrsTsPidApplyReserved;
    stream = SrsTsStreamReserved;
    msg    = NULL;
    continuity_counter = 0;
    context = NULL;
}

SrsTsChannel::~SrsTsChannel()
{
    srs_freep(msg);
}

SrsTsMessage::SrsTsMessage(SrsTsChannel* c, SrsTsPacket* p)
{
    channel = c;
    packet  = p;
    dts     = pts = 0;
    sid     = (SrsTsPESStreamId) 0x00;
    continuity_counter = 0;
    PES_packet_length  = 0;
    payload = new SrsSimpleBuffer();
    is_discontinuity = false;
    start_pts        = 0;
    write_pcr        = false;
}

SrsTsMessage::~SrsTsMessage()
{
    srs_freep(payload);
}

int SrsTsMessage::dump(SrsStream* stream, int* pnb_bytes)
{
    int ret = ERROR_SUCCESS;

    if (stream->empty()) {
        return ret;
    }
    int nb_bytes = stream->size() - stream->pos();
    if (PES_packet_length > 0) {
        nb_bytes = srs_min(nb_bytes, PES_packet_length - payload->length());
    }
    if (nb_bytes > 0) {
        if (!stream->require(nb_bytes)) {
            ret = ERROR_STREAM_CASTER_TS_PSE;
            srs_error("ts: dump PSE bytes failed, requires=%dB. ret=%d", nb_bytes, ret);
            return ret;
        }
        payload->append(stream->data() + stream->pos(), nb_bytes);
        stream->skip(nb_bytes);
    }
    *pnb_bytes = nb_bytes;
    return ret;
}

bool SrsTsMessage::completed(int8_t payload_unit_start_indicator)
{
    if (PES_packet_length == 0) {
        return payload_unit_start_indicator;
    }
    return payload->length() >= PES_packet_length;
}

bool SrsTsMessage::fresh()
{
    return payload->length() == 0;
}

bool SrsTsMessage::is_audio()
{
    return ((sid >> 5) & 0x07) == SrsTsPESStreamIdAudioChecker;
}

bool SrsTsMessage::is_video()
{
    return ((sid >> 4) & 0x0f) == SrsTsPESStreamIdVideoChecker;
}

int SrsTsMessage::stream_number()
{
    if (is_audio()) {
        return sid & 0x1f;
    } else if (is_video()) {
        return sid & 0x0f;
    }
    return -1;
}

SrsTsMessage * SrsTsMessage::detach()
{
    SrsTsMessage* cp = new SrsTsMessage(channel, NULL);

    cp->start_pts        = start_pts;
    cp->write_pcr        = write_pcr;
    cp->is_discontinuity = is_discontinuity;
    cp->dts = dts;
    cp->pts = pts;
    cp->sid = sid;
    cp->PES_packet_length  = PES_packet_length;
    cp->continuity_counter = continuity_counter;
    cp->payload = payload;
    payload     = NULL;
    return cp;
}

ISrsTsHandler::ISrsTsHandler()
{}

ISrsTsHandler::~ISrsTsHandler()
{}

SrsTsContext::SrsTsContext()
{
    pure_audio = false;
    vcodec     = SrsCodecVideoReserved;
    acodec     = SrsCodecAudioReserved1;
}

SrsTsContext::~SrsTsContext()
{
    std::map<int, SrsTsChannel *>::iterator it;

    for (it = pids.begin(); it != pids.end(); ++it) {
        SrsTsChannel* channel = it->second;
        srs_freep(channel);
    }
    pids.clear();
}

bool SrsTsContext::is_pure_audio()
{
    return pure_audio;
}

void SrsTsContext::on_pmt_parsed()
{
    pure_audio = true;
    std::map<int, SrsTsChannel *>::iterator it;
    for (it = pids.begin(); it != pids.end(); ++it) {
        SrsTsChannel* channel = it->second;
        if (channel->apply == SrsTsPidApplyVideo) {
            pure_audio = false;
        }
    }
}

void SrsTsContext::reset()
{
    vcodec = SrsCodecVideoReserved;
    acodec = SrsCodecAudioReserved1;
}

SrsTsChannel * SrsTsContext::get(int pid)
{
    if (pids.find(pid) == pids.end()) {
        return NULL;
    }
    return pids[pid];
}

void SrsTsContext::set(int pid, SrsTsPidApply apply_pid, SrsTsStream stream)
{
    SrsTsChannel* channel = NULL;

    if (pids.find(pid) == pids.end()) {
        channel = new SrsTsChannel();
        channel->context = this;
        pids[pid]        = channel;
    } else {
        channel = pids[pid];
    }
    channel->pid    = pid;
    channel->apply  = apply_pid;
    channel->stream = stream;
}

int SrsTsContext::decode(SrsStream* stream, ISrsTsHandler* handler)
{
    int ret = ERROR_SUCCESS;

    while (!stream->empty()) {
        SrsTsPacket* packet = new SrsTsPacket(this);
        SrsAutoFree(SrsTsPacket, packet);
        SrsTsMessage* msg = NULL;
        if ((ret = packet->decode(stream, &msg)) != ERROR_SUCCESS) {
            srs_error("mpegts: decode ts packet failed. ret=%d", ret);
            return ret;
        }
        if (!msg) {
            continue;
        }
        SrsAutoFree(SrsTsMessage, msg);
        if ((ret = handler->on_ts_message(msg)) != ERROR_SUCCESS) {
            srs_error("mpegts: handler ts message failed. ret=%d", ret);
            return ret;
        }
    }
    return ret;
}

int SrsTsContext::encode(SrsFileWriter* writer, SrsTsMessage* msg, SrsCodecVideo vc, SrsCodecAudio ac)
{
    int ret = ERROR_SUCCESS;
    SrsTsStream vs, as;
    int16_t video_pid = 0, audio_pid = 0;

    switch (vc) {
        case SrsCodecVideoAVC:
            vs        = SrsTsStreamVideoH264;
            video_pid = TS_VIDEO_AVC_PID;
            break;
        case SrsCodecVideoDisabled:
            vs = SrsTsStreamReserved;
            break;
        case SrsCodecVideoReserved:
        case SrsCodecVideoReserved1:
        case SrsCodecVideoReserved2:
        case SrsCodecVideoSorensonH263:
        case SrsCodecVideoScreenVideo:
        case SrsCodecVideoOn2VP6:
        case SrsCodecVideoOn2VP6WithAlphaChannel:
        case SrsCodecVideoScreenVideoVersion2:
            vs = SrsTsStreamReserved;
            break;
    }
    switch (ac) {
        case SrsCodecAudioAAC:
            as        = SrsTsStreamAudioAAC;
            audio_pid = TS_AUDIO_AAC_PID;
            break;
        case SrsCodecAudioMP3:
            as        = SrsTsStreamAudioMp3;
            audio_pid = TS_AUDIO_MP3_PID;
            break;
        case SrsCodecAudioDisabled:
            as = SrsTsStreamReserved;
            break;
        case SrsCodecAudioReserved1:
        case SrsCodecAudioLinearPCMPlatformEndian:
        case SrsCodecAudioADPCM:
        case SrsCodecAudioLinearPCMLittleEndian:
        case SrsCodecAudioNellymoser16kHzMono:
        case SrsCodecAudioNellymoser8kHzMono:
        case SrsCodecAudioNellymoser:
        case SrsCodecAudioReservedG711AlawLogarithmicPCM:
        case SrsCodecAudioReservedG711MuLawLogarithmicPCM:
        case SrsCodecAudioReserved:
        case SrsCodecAudioSpeex:
        case SrsCodecAudioReservedMP3_8kHz:
        case SrsCodecAudioReservedDeviceSpecificSound:
            as = SrsTsStreamReserved;
            break;
    }
    if (as == SrsTsStreamReserved && vs == SrsTsStreamReserved) {
        ret = ERROR_HLS_NO_STREAM;
        srs_error("hls: no video or audio stream, vcodec=%d, acodec=%d. ret=%d", vc, ac, ret);
        return ret;
    }
    if (vcodec != vc || acodec != ac) {
        vcodec = vc;
        acodec = ac;
        if ((ret = encode_pat_pmt(writer, video_pid, vs, audio_pid, as)) != ERROR_SUCCESS) {
            return ret;
        }
    }
    if (msg->is_audio()) {
        return encode_pes(writer, msg, audio_pid, as, vs == SrsTsStreamReserved);
    } else {
        return encode_pes(writer, msg, video_pid, vs, vs == SrsTsStreamReserved);
    }
} // SrsTsContext::encode

int SrsTsContext::encode_pat_pmt(SrsFileWriter* writer, int16_t vpid, SrsTsStream vs, int16_t apid, SrsTsStream as)
{
    int ret = ERROR_SUCCESS;

    if (vs != SrsTsStreamVideoH264 && as != SrsTsStreamAudioAAC && as != SrsTsStreamAudioMp3) {
        ret = ERROR_HLS_NO_STREAM;
        srs_error("hls: no pmt pcr pid, vs=%d, as=%d. ret=%d", vs, as, ret);
        return ret;
    }
    int16_t pmt_number = TS_PMT_NUMBER;
    int16_t pmt_pid    = TS_PMT_PID;
    if (true) {
        SrsTsPacket* pkt = SrsTsPacket::create_pat(this, pmt_number, pmt_pid);
        SrsAutoFree(SrsTsPacket, pkt);
        char* buf = new char[SRS_TS_PACKET_SIZE];
        SrsAutoFreeA(char, buf);
        int nb_buf = pkt->size();
        srs_assert(nb_buf < SRS_TS_PACKET_SIZE);
        memset(buf + nb_buf, 0xFF, SRS_TS_PACKET_SIZE - nb_buf);
        SrsStream stream;
        if ((ret = stream.initialize(buf, nb_buf)) != ERROR_SUCCESS) {
            return ret;
        }
        if ((ret = pkt->encode(&stream)) != ERROR_SUCCESS) {
            srs_error("ts encode ts packet failed. ret=%d", ret);
            return ret;
        }
        if ((ret = writer->write(buf, SRS_TS_PACKET_SIZE, NULL)) != ERROR_SUCCESS) {
            srs_error("ts write ts packet failed. ret=%d", ret);
            return ret;
        }
    }
    if (true) {
        SrsTsPacket* pkt = SrsTsPacket::create_pmt(this, pmt_number, pmt_pid, vpid, vs, apid, as);
        SrsAutoFree(SrsTsPacket, pkt);
        char* buf = new char[SRS_TS_PACKET_SIZE];
        SrsAutoFreeA(char, buf);
        int nb_buf = pkt->size();
        srs_assert(nb_buf < SRS_TS_PACKET_SIZE);
        memset(buf + nb_buf, 0xFF, SRS_TS_PACKET_SIZE - nb_buf);
        SrsStream stream;
        if ((ret = stream.initialize(buf, nb_buf)) != ERROR_SUCCESS) {
            return ret;
        }
        if ((ret = pkt->encode(&stream)) != ERROR_SUCCESS) {
            srs_error("ts encode ts packet failed. ret=%d", ret);
            return ret;
        }
        if ((ret = writer->write(buf, SRS_TS_PACKET_SIZE, NULL)) != ERROR_SUCCESS) {
            srs_error("ts write ts packet failed. ret=%d", ret);
            return ret;
        }
    }
    return ret;
} // SrsTsContext::encode_pat_pmt

int SrsTsContext::encode_pes(SrsFileWriter* writer, SrsTsMessage* msg, int16_t pid, SrsTsStream sid, bool pure_audio)
{
    int ret = ERROR_SUCCESS;

    if (msg->payload->length() == 0) {
        return ret;
    }
    if (sid != SrsTsStreamVideoH264 && sid != SrsTsStreamAudioMp3 && sid != SrsTsStreamAudioAAC) {
        srs_info("ts: ignore the unknown stream, sid=%d", sid);
        return ret;
    }
    SrsTsChannel* channel = get(pid);
    srs_assert(channel);
    char* start = msg->payload->bytes();
    char* end   = start + msg->payload->length();
    char* p     = start;
    while (p < end) {
        SrsTsPacket* pkt = NULL;
        if (p == start) {
            bool write_pcr = msg->write_pcr;
            if (pure_audio && msg->is_audio()) {
                write_pcr = true;
            }
            int64_t pcr = write_pcr ? msg->dts : -1;
            pkt = SrsTsPacket::create_pes_first(this,
                                                pid, msg->sid, channel->continuity_counter++, msg->is_discontinuity,
                                                pcr, msg->dts, msg->pts, msg->payload->length()
            );
        } else {
            pkt = SrsTsPacket::create_pes_continue(this,
                                                   pid, msg->sid, channel->continuity_counter++
            );
        }
        SrsAutoFree(SrsTsPacket, pkt);
        char* buf = new char[SRS_TS_PACKET_SIZE];
        SrsAutoFreeA(char, buf);
        int nb_buf = pkt->size();
        srs_assert(nb_buf < SRS_TS_PACKET_SIZE);
        int left         = (int) srs_min(end - p, SRS_TS_PACKET_SIZE - nb_buf);
        int nb_stuffings = SRS_TS_PACKET_SIZE - nb_buf - left;
        if (nb_stuffings > 0) {
            memset(buf, 0xFF, SRS_TS_PACKET_SIZE);
            pkt->padding(nb_stuffings);
            nb_buf = pkt->size();
            srs_assert(nb_buf < SRS_TS_PACKET_SIZE);
            left         = (int) srs_min(end - p, SRS_TS_PACKET_SIZE - nb_buf);
            nb_stuffings = SRS_TS_PACKET_SIZE - nb_buf - left;
            srs_assert(nb_stuffings == 0);
        }
        memcpy(buf + nb_buf, p, left);
        p += left;
        SrsStream stream;
        if ((ret = stream.initialize(buf, nb_buf)) != ERROR_SUCCESS) {
            return ret;
        }
        if ((ret = pkt->encode(&stream)) != ERROR_SUCCESS) {
            srs_error("ts encode ts packet failed. ret=%d", ret);
            return ret;
        }
        if ((ret = writer->write(buf, SRS_TS_PACKET_SIZE, NULL)) != ERROR_SUCCESS) {
            srs_error("ts write ts packet failed. ret=%d", ret);
            return ret;
        }
    }
    return ret;
} // SrsTsContext::encode_pes

SrsTsPacket::SrsTsPacket(SrsTsContext* c)
{
    context   = c;
    sync_byte = 0;
    transport_error_indicator    = 0;
    payload_unit_start_indicator = 0;
    transport_priority = 0;
    pid = SrsTsPidPAT;
    transport_scrambling_control = SrsTsScrambledDisabled;
    adaption_field_control       = SrsTsAdaptationFieldTypeReserved;
    continuity_counter = 0;
    adaptation_field   = NULL;
    payload = NULL;
}

SrsTsPacket::~SrsTsPacket()
{
    srs_freep(adaptation_field);
    srs_freep(payload);
}

int SrsTsPacket::decode(SrsStream* stream, SrsTsMessage** ppmsg)
{
    int ret = ERROR_SUCCESS;
    int pos = stream->pos();

    if (!stream->require(4)) {
        ret = ERROR_STREAM_CASTER_TS_HEADER;
        srs_error("ts: demux header failed. ret=%d", ret);
        return ret;
    }
    sync_byte = stream->read_1bytes();
    if (sync_byte != 0x47) {
        ret = ERROR_STREAM_CASTER_TS_SYNC_BYTE;
        srs_error("ts: sync_bytes must be 0x47, actual=%#x. ret=%d", sync_byte, ret);
        return ret;
    }
    int16_t pidv = stream->read_2bytes();
    transport_error_indicator    = (pidv >> 15) & 0x01;
    payload_unit_start_indicator = (pidv >> 14) & 0x01;
    transport_priority = (pidv >> 13) & 0x01;
    pid = (SrsTsPid) (pidv & 0x1FFF);
    int8_t ccv = stream->read_1bytes();
    transport_scrambling_control = (SrsTsScrambled) ((ccv >> 6) & 0x03);
    adaption_field_control       = (SrsTsAdaptationFieldType) ((ccv >> 4) & 0x03);
    continuity_counter = ccv & 0x0F;
    srs_info("ts: header sync=%#x error=%d unit_start=%d priotiry=%d pid=%d scrambling=%d adaption=%d counter=%d",
             sync_byte, transport_error_indicator, payload_unit_start_indicator, transport_priority, pid,
             transport_scrambling_control, adaption_field_control, continuity_counter);
    if (adaption_field_control == SrsTsAdaptationFieldTypeAdaptionOnly ||
        adaption_field_control == SrsTsAdaptationFieldTypeBoth)
    {
        srs_freep(adaptation_field);
        adaptation_field = new SrsTsAdaptationField(this);
        if ((ret = adaptation_field->decode(stream)) != ERROR_SUCCESS) {
            srs_error("ts: demux af faield. ret=%d", ret);
            return ret;
        }
        srs_verbose("ts: demux af ok.");
    }
    int nb_payload = SRS_TS_PACKET_SIZE - (stream->pos() - pos);
    if (adaption_field_control == SrsTsAdaptationFieldTypePayloadOnly ||
        adaption_field_control == SrsTsAdaptationFieldTypeBoth)
    {
        if (pid == SrsTsPidPAT) {
            srs_freep(payload);
            payload = new SrsTsPayloadPAT(this);
        } else {
            SrsTsChannel* channel = context->get(pid);
            if (channel && channel->apply == SrsTsPidApplyPMT) {
                srs_freep(payload);
                payload = new SrsTsPayloadPMT(this);
            } else if (channel && (channel->apply == SrsTsPidApplyVideo || channel->apply == SrsTsPidApplyAudio)) {
                srs_freep(payload);
                payload = new SrsTsPayloadPES(this);
            } else {
                stream->skip(nb_payload);
            }
        }
        if (payload && (ret = payload->decode(stream, ppmsg)) != ERROR_SUCCESS) {
            srs_error("ts: demux payload failed. ret=%d", ret);
            return ret;
        }
    }
    return ret;
} // SrsTsPacket::decode

int SrsTsPacket::size()
{
    int sz = 4;

    sz += adaptation_field ? adaptation_field->size() : 0;
    sz += payload ? payload->size() : 0;
    return sz;
}

int SrsTsPacket::encode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(4)) {
        ret = ERROR_STREAM_CASTER_TS_HEADER;
        srs_error("ts: mux header failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(sync_byte);
    int16_t pidv = pid & 0x1FFF;
    pidv |= (transport_priority << 13) & 0x2000;
    pidv |= (transport_error_indicator << 15) & 0x8000;
    pidv |= (payload_unit_start_indicator << 14) & 0x4000;
    stream->write_2bytes(pidv);
    int8_t ccv = continuity_counter & 0x0F;
    ccv |= (transport_scrambling_control << 6) & 0xC0;
    ccv |= (adaption_field_control << 4) & 0x30;
    stream->write_1bytes(ccv);
    srs_info("ts: header sync=%#x error=%d unit_start=%d priotiry=%d pid=%d scrambling=%d adaption=%d counter=%d",
             sync_byte, transport_error_indicator, payload_unit_start_indicator, transport_priority, pid,
             transport_scrambling_control, adaption_field_control, continuity_counter);
    if (adaptation_field) {
        if ((ret = adaptation_field->encode(stream)) != ERROR_SUCCESS) {
            srs_error("ts: mux af faield. ret=%d", ret);
            return ret;
        }
        srs_verbose("ts: mux af ok.");
    }
    if (payload) {
        if ((ret = payload->encode(stream)) != ERROR_SUCCESS) {
            srs_error("ts: mux payload failed. ret=%d", ret);
            return ret;
        }
        srs_verbose("ts: mux payload ok.");
    }
    return ret;
} // SrsTsPacket::encode

void SrsTsPacket::padding(int nb_stuffings)
{
    if (!adaptation_field) {
        SrsTsAdaptationField* af = new SrsTsAdaptationField(this);
        adaptation_field = af;
        af->adaption_field_length   = 0;
        af->discontinuity_indicator = 0;
        af->random_access_indicator = 0;
        af->elementary_stream_priority_indicator = 0;
        af->PCR_flag  = 0;
        af->OPCR_flag = 0;
        af->splicing_point_flag             = 0;
        af->transport_private_data_flag     = 0;
        af->adaptation_field_extension_flag = 0;
        nb_stuffings = srs_max(0, nb_stuffings - af->size());
    }
    adaptation_field->nb_af_reserved = nb_stuffings;
    if (adaption_field_control == SrsTsAdaptationFieldTypePayloadOnly) {
        adaption_field_control = SrsTsAdaptationFieldTypeBoth;
    }
}

SrsTsPacket * SrsTsPacket::create_pat(SrsTsContext* context, int16_t pmt_number, int16_t pmt_pid)
{
    SrsTsPacket* pkt = new SrsTsPacket(context);

    pkt->sync_byte = 0x47;
    pkt->transport_error_indicator    = 0;
    pkt->payload_unit_start_indicator = 1;
    pkt->transport_priority = 0;
    pkt->pid = SrsTsPidPAT;
    pkt->transport_scrambling_control = SrsTsScrambledDisabled;
    pkt->adaption_field_control       = SrsTsAdaptationFieldTypePayloadOnly;
    pkt->continuity_counter = 0;
    pkt->adaptation_field   = NULL;
    SrsTsPayloadPAT* pat = new SrsTsPayloadPAT(pkt);
    pkt->payload       = pat;
    pat->pointer_field = 0;
    pat->table_id      = SrsTsPsiIdPas;
    pat->section_syntax_indicator = 1;
    pat->section_length         = 0;
    pat->transport_stream_id    = 1;
    pat->version_number         = 0;
    pat->current_next_indicator = 1;
    pat->section_number         = 0;
    pat->last_section_number    = 0;
    pat->programs.push_back(new SrsTsPayloadPATProgram(pmt_number, pmt_pid));
    pat->CRC_32 = 0;
    return pkt;
}

SrsTsPacket * SrsTsPacket::create_pmt(SrsTsContext* context, int16_t pmt_number, int16_t pmt_pid, int16_t vpid,
                                      SrsTsStream vs, int16_t apid, SrsTsStream as)
{
    SrsTsPacket* pkt = new SrsTsPacket(context);

    pkt->sync_byte = 0x47;
    pkt->transport_error_indicator    = 0;
    pkt->payload_unit_start_indicator = 1;
    pkt->transport_priority = 0;
    pkt->pid = (SrsTsPid) pmt_pid;
    pkt->transport_scrambling_control = SrsTsScrambledDisabled;
    pkt->adaption_field_control       = SrsTsAdaptationFieldTypePayloadOnly;
    pkt->continuity_counter = 0;
    pkt->adaptation_field   = NULL;
    SrsTsPayloadPMT* pmt = new SrsTsPayloadPMT(pkt);
    pkt->payload       = pmt;
    pmt->pointer_field = 0;
    pmt->table_id      = SrsTsPsiIdPms;
    pmt->section_syntax_indicator = 1;
    pmt->section_length         = 0;
    pmt->program_number         = pmt_number;
    pmt->version_number         = 0;
    pmt->current_next_indicator = 1;
    pmt->section_number         = 0;
    pmt->last_section_number    = 0;
    pmt->program_info_length    = 0;
    srs_assert(vs == SrsTsStreamVideoH264 || as == SrsTsStreamAudioAAC || as == SrsTsStreamAudioMp3);
    if (as == SrsTsStreamAudioAAC || as == SrsTsStreamAudioMp3) {
        pmt->PCR_PID = apid;
        pmt->infos.push_back(new SrsTsPayloadPMTESInfo(as, apid));
    }
    if (vs == SrsTsStreamVideoH264) {
        pmt->PCR_PID = vpid;
        pmt->infos.push_back(new SrsTsPayloadPMTESInfo(vs, vpid));
    }
    pmt->CRC_32 = 0;
    return pkt;
} // SrsTsPacket::create_pmt

SrsTsPacket * SrsTsPacket::create_pes_first(SrsTsContext* context,
                                            int16_t pid, SrsTsPESStreamId sid, u_int8_t continuity_counter,
                                            bool discontinuity,
                                            int64_t pcr, int64_t dts, int64_t pts, int size
)
{
    SrsTsPacket* pkt = new SrsTsPacket(context);

    pkt->sync_byte = 0x47;
    pkt->transport_error_indicator    = 0;
    pkt->payload_unit_start_indicator = 1;
    pkt->transport_priority = 0;
    pkt->pid = (SrsTsPid) pid;
    pkt->transport_scrambling_control = SrsTsScrambledDisabled;
    pkt->adaption_field_control       = SrsTsAdaptationFieldTypePayloadOnly;
    pkt->continuity_counter = continuity_counter;
    pkt->adaptation_field   = NULL;
    SrsTsPayloadPES* pes = new SrsTsPayloadPES(pkt);
    pkt->payload = pes;
    if (pcr >= 0) {
        SrsTsAdaptationField* af = new SrsTsAdaptationField(pkt);
        pkt->adaptation_field       = af;
        pkt->adaption_field_control = SrsTsAdaptationFieldTypeBoth;
        af->adaption_field_length   = 0;
        af->discontinuity_indicator = discontinuity;
        af->random_access_indicator = 0;
        af->elementary_stream_priority_indicator = 0;
        af->PCR_flag  = 1;
        af->OPCR_flag = 0;
        af->splicing_point_flag               = 0;
        af->transport_private_data_flag       = 0;
        af->adaptation_field_extension_flag   = 0;
        af->program_clock_reference_base      = pcr;
        af->program_clock_reference_extension = 0;
    }
    pes->packet_start_code_prefix = 0x01;
    pes->stream_id                 = (u_int8_t) sid;
    pes->PES_packet_length         = (size > 0xFFFF) ? 0 : size;
    pes->PES_scrambling_control    = 0;
    pes->PES_priority              = 0;
    pes->data_alignment_indicator  = 0;
    pes->copyright                 = 0;
    pes->original_or_copy          = 0;
    pes->PTS_DTS_flags             = (dts == pts) ? 0x02 : 0x03;
    pes->ESCR_flag                 = 0;
    pes->ES_rate_flag              = 0;
    pes->DSM_trick_mode_flag       = 0;
    pes->additional_copy_info_flag = 0;
    pes->PES_CRC_flag              = 0;
    pes->PES_extension_flag        = 0;
    pes->PES_header_data_length    = 0;
    pes->pts = pts;
    pes->dts = dts;
    return pkt;
} // SrsTsPacket::create_pes_first

SrsTsPacket * SrsTsPacket::create_pes_continue(SrsTsContext* context,
                                               int16_t pid, SrsTsPESStreamId sid, u_int8_t continuity_counter
)
{
    SrsTsPacket* pkt = new SrsTsPacket(context);

    pkt->sync_byte = 0x47;
    pkt->transport_error_indicator    = 0;
    pkt->payload_unit_start_indicator = 0;
    pkt->transport_priority = 0;
    pkt->pid = (SrsTsPid) pid;
    pkt->transport_scrambling_control = SrsTsScrambledDisabled;
    pkt->adaption_field_control       = SrsTsAdaptationFieldTypePayloadOnly;
    pkt->continuity_counter = continuity_counter;
    pkt->adaptation_field   = NULL;
    pkt->payload = NULL;
    return pkt;
}

SrsTsAdaptationField::SrsTsAdaptationField(SrsTsPacket* pkt)
{
    packet = pkt;
    adaption_field_length   = 0;
    discontinuity_indicator = 0;
    random_access_indicator = 0;
    elementary_stream_priority_indicator = 0;
    PCR_flag  = 0;
    OPCR_flag = 0;
    splicing_point_flag                        = 0;
    transport_private_data_flag                = 0;
    adaptation_field_extension_flag            = 0;
    program_clock_reference_base               = 0;
    program_clock_reference_extension          = 0;
    original_program_clock_reference_base      = 0;
    original_program_clock_reference_extension = 0;
    splice_countdown = 0;
    transport_private_data_length     = 0;
    transport_private_data            = NULL;
    adaptation_field_extension_length = 0;
    ltw_flag = 0;
    piecewise_rate_flag  = 0;
    seamless_splice_flag = 0;
    ltw_valid_flag       = 0;
    ltw_offset         = 0;
    piecewise_rate     = 0;
    splice_type        = 0;
    DTS_next_AU0       = 0;
    marker_bit0        = 0;
    DTS_next_AU1       = 0;
    marker_bit1        = 0;
    DTS_next_AU2       = 0;
    marker_bit2        = 0;
    nb_af_ext_reserved = 0;
    nb_af_reserved     = 0;
    const1_value0      = 0x3F;
    const1_value1      = 0x1F;
    const1_value2      = 0x3F;
}

SrsTsAdaptationField::~SrsTsAdaptationField()
{
    srs_freepa(transport_private_data);
}

int SrsTsAdaptationField::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(2)) {
        ret = ERROR_STREAM_CASTER_TS_AF;
        srs_error("ts: demux af failed. ret=%d", ret);
        return ret;
    }
    adaption_field_length = stream->read_1bytes();
    if (packet->adaption_field_control == SrsTsAdaptationFieldTypeBoth && adaption_field_length > 182) {
        ret = ERROR_STREAM_CASTER_TS_AF;
        srs_error("ts: demux af length failed, must in [0, 182], actual=%d. ret=%d", adaption_field_length, ret);
        return ret;
    }
    if (packet->adaption_field_control == SrsTsAdaptationFieldTypeAdaptionOnly && adaption_field_length != 183) {
        ret = ERROR_STREAM_CASTER_TS_AF;
        srs_error("ts: demux af length failed, must be 183, actual=%d. ret=%d", adaption_field_length, ret);
        return ret;
    }
    if (adaption_field_length == 0) {
        srs_info("ts: demux af empty.");
        return ret;
    }
    int pos_af  = stream->pos();
    int8_t tmpv = stream->read_1bytes();
    discontinuity_indicator = (tmpv >> 7) & 0x01;
    random_access_indicator = (tmpv >> 6) & 0x01;
    elementary_stream_priority_indicator = (tmpv >> 5) & 0x01;
    PCR_flag  = (tmpv >> 4) & 0x01;
    OPCR_flag = (tmpv >> 3) & 0x01;
    splicing_point_flag             = (tmpv >> 2) & 0x01;
    transport_private_data_flag     = (tmpv >> 1) & 0x01;
    adaptation_field_extension_flag = tmpv & 0x01;
    if (PCR_flag) {
        if (!stream->require(6)) {
            ret = ERROR_STREAM_CASTER_TS_AF;
            srs_error("ts: demux af PCR_flag failed. ret=%d", ret);
            return ret;
        }
        char* pp = NULL;
        char* p  = stream->data() + stream->pos();
        stream->skip(6);
        int64_t pcrv = 0;
        pp    = (char *) &pcrv;
        pp[5] = *p++;
        pp[4] = *p++;
        pp[3] = *p++;
        pp[2] = *p++;
        pp[1] = *p++;
        pp[0] = *p++;
        program_clock_reference_extension = pcrv & 0x1ff;
        const1_value0 = (pcrv >> 9) & 0x3F;
        program_clock_reference_base = (pcrv >> 15) & 0x1ffffffffLL;
    }
    if (OPCR_flag) {
        if (!stream->require(6)) {
            ret = ERROR_STREAM_CASTER_TS_AF;
            srs_error("ts: demux af OPCR_flag failed. ret=%d", ret);
            return ret;
        }
        char* pp = NULL;
        char* p  = stream->data() + stream->pos();
        stream->skip(6);
        int64_t opcrv = 0;
        pp    = (char *) &opcrv;
        pp[5] = *p++;
        pp[4] = *p++;
        pp[3] = *p++;
        pp[2] = *p++;
        pp[1] = *p++;
        pp[0] = *p++;
        original_program_clock_reference_extension = opcrv & 0x1ff;
        const1_value2 = (opcrv >> 9) & 0x3F;
        original_program_clock_reference_base = (opcrv >> 15) & 0x1ffffffffLL;
    }
    if (splicing_point_flag) {
        if (!stream->require(1)) {
            ret = ERROR_STREAM_CASTER_TS_AF;
            srs_error("ts: demux af splicing_point_flag failed. ret=%d", ret);
            return ret;
        }
        splice_countdown = stream->read_1bytes();
    }
    if (transport_private_data_flag) {
        if (!stream->require(1)) {
            ret = ERROR_STREAM_CASTER_TS_AF;
            srs_error("ts: demux af transport_private_data_flag failed. ret=%d", ret);
            return ret;
        }
        transport_private_data_length = (u_int8_t) stream->read_1bytes();
        if (transport_private_data_length > 0) {
            if (!stream->require(transport_private_data_length)) {
                ret = ERROR_STREAM_CASTER_TS_AF;
                srs_error("ts: demux af transport_private_data_flag failed. ret=%d", ret);
                return ret;
            }
            srs_freepa(transport_private_data);
            transport_private_data = new char[transport_private_data_length];
            stream->read_bytes(transport_private_data, transport_private_data_length);
        }
    }
    if (adaptation_field_extension_flag) {
        int pos_af_ext = stream->pos();
        if (!stream->require(2)) {
            ret = ERROR_STREAM_CASTER_TS_AF;
            srs_error("ts: demux af adaptation_field_extension_flag failed. ret=%d", ret);
            return ret;
        }
        adaptation_field_extension_length = (u_int8_t) stream->read_1bytes();
        int8_t ltwfv = stream->read_1bytes();
        piecewise_rate_flag  = (ltwfv >> 6) & 0x01;
        seamless_splice_flag = (ltwfv >> 5) & 0x01;
        ltw_flag      = (ltwfv >> 7) & 0x01;
        const1_value1 = ltwfv & 0x1F;
        if (ltw_flag) {
            if (!stream->require(2)) {
                ret = ERROR_STREAM_CASTER_TS_AF;
                srs_error("ts: demux af ltw_flag failed. ret=%d", ret);
                return ret;
            }
            ltw_offset     = stream->read_2bytes();
            ltw_valid_flag = (ltw_offset >> 15) & 0x01;
            ltw_offset    &= 0x7FFF;
        }
        if (piecewise_rate_flag) {
            if (!stream->require(3)) {
                ret = ERROR_STREAM_CASTER_TS_AF;
                srs_error("ts: demux af piecewise_rate_flag failed. ret=%d", ret);
                return ret;
            }
            piecewise_rate  = stream->read_3bytes();
            piecewise_rate &= 0x3FFFFF;
        }
        if (seamless_splice_flag) {
            if (!stream->require(5)) {
                ret = ERROR_STREAM_CASTER_TS_AF;
                srs_error("ts: demux af seamless_splice_flag failed. ret=%d", ret);
                return ret;
            }
            marker_bit0  = stream->read_1bytes();
            DTS_next_AU1 = stream->read_2bytes();
            DTS_next_AU2 = stream->read_2bytes();
            splice_type  = (marker_bit0 >> 4) & 0x0F;
            DTS_next_AU0 = (marker_bit0 >> 1) & 0x07;
            marker_bit0 &= 0x01;
            marker_bit1  = DTS_next_AU1 & 0x01;
            DTS_next_AU1 = (DTS_next_AU1 >> 1) & 0x7FFF;
            marker_bit2  = DTS_next_AU2 & 0x01;
            DTS_next_AU2 = (DTS_next_AU2 >> 1) & 0x7FFF;
        }
        nb_af_ext_reserved = adaptation_field_extension_length - (stream->pos() - pos_af_ext);
        stream->skip(nb_af_ext_reserved);
    }
    nb_af_reserved = adaption_field_length - (stream->pos() - pos_af);
    stream->skip(nb_af_reserved);
    srs_info(
        "ts: af parsed, discontinuity=%d random=%d priority=%d PCR=%d OPCR=%d slicing=%d private=%d extension=%d/%d pcr=%" PRId64 "/%d opcr=%" PRId64 "/%d",
        discontinuity_indicator, random_access_indicator, elementary_stream_priority_indicator, PCR_flag, OPCR_flag, splicing_point_flag,
        transport_private_data_flag, adaptation_field_extension_flag, adaptation_field_extension_length, program_clock_reference_base,
        program_clock_reference_extension, original_program_clock_reference_base,
        original_program_clock_reference_extension);
    return ret;
} // SrsTsAdaptationField::decode

int SrsTsAdaptationField::size()
{
    int sz = 2;

    sz += PCR_flag ? 6 : 0;
    sz += OPCR_flag ? 6 : 0;
    sz += splicing_point_flag ? 1 : 0;
    sz += transport_private_data_flag ? 1 + transport_private_data_length : 0;
    sz += adaptation_field_extension_flag ? 2 + adaptation_field_extension_length : 0;
    sz += nb_af_ext_reserved;
    sz += nb_af_reserved;
    adaption_field_length = sz - 1;
    return sz;
}

int SrsTsAdaptationField::encode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(2)) {
        ret = ERROR_STREAM_CASTER_TS_AF;
        srs_error("ts: mux af failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(adaption_field_length);
    if (packet->adaption_field_control == SrsTsAdaptationFieldTypeBoth && adaption_field_length > 182) {
        ret = ERROR_STREAM_CASTER_TS_AF;
        srs_error("ts: mux af length failed, must in [0, 182], actual=%d. ret=%d", adaption_field_length, ret);
        return ret;
    }
    if (packet->adaption_field_control == SrsTsAdaptationFieldTypeAdaptionOnly && adaption_field_length != 183) {
        ret = ERROR_STREAM_CASTER_TS_AF;
        srs_error("ts: mux af length failed, must be 183, actual=%d. ret=%d", adaption_field_length, ret);
        return ret;
    }
    if (adaption_field_length == 0) {
        srs_info("ts: mux af empty.");
        return ret;
    }
    int8_t tmpv = adaptation_field_extension_flag & 0x01;
    tmpv |= (discontinuity_indicator << 7) & 0x80;
    tmpv |= (random_access_indicator << 6) & 0x40;
    tmpv |= (elementary_stream_priority_indicator << 5) & 0x20;
    tmpv |= (PCR_flag << 4) & 0x10;
    tmpv |= (OPCR_flag << 3) & 0x08;
    tmpv |= (splicing_point_flag << 2) & 0x04;
    tmpv |= (transport_private_data_flag << 1) & 0x02;
    stream->write_1bytes(tmpv);
    if (PCR_flag) {
        if (!stream->require(6)) {
            ret = ERROR_STREAM_CASTER_TS_AF;
            srs_error("ts: mux af PCR_flag failed. ret=%d", ret);
            return ret;
        }
        char* pp = NULL;
        char* p  = stream->data() + stream->pos();
        stream->skip(6);
        int64_t pcrv = program_clock_reference_extension & 0x1ff;
        pcrv |= (const1_value0 << 9) & 0x7E00;
        pcrv |= (program_clock_reference_base << 15) & 0x1FFFFFFFF000000LL;
        pp    = (char *) &pcrv;
        *p++  = pp[5];
        *p++  = pp[4];
        *p++  = pp[3];
        *p++  = pp[2];
        *p++  = pp[1];
        *p++  = pp[0];
    }
    if (OPCR_flag) {
        if (!stream->require(6)) {
            ret = ERROR_STREAM_CASTER_TS_AF;
            srs_error("ts: demux af OPCR_flag failed. ret=%d", ret);
            return ret;
        }
        stream->skip(6);
        srs_warn("ts: mux af ignore OPCR");
    }
    if (splicing_point_flag) {
        if (!stream->require(1)) {
            ret = ERROR_STREAM_CASTER_TS_AF;
            srs_error("ts: mux af splicing_point_flag failed. ret=%d", ret);
            return ret;
        }
        stream->write_1bytes(splice_countdown);
    }
    if (transport_private_data_flag) {
        if (!stream->require(1)) {
            ret = ERROR_STREAM_CASTER_TS_AF;
            srs_error("ts: mux af transport_private_data_flag failed. ret=%d", ret);
            return ret;
        }
        stream->write_1bytes(transport_private_data_length);
        if (transport_private_data_length > 0) {
            if (!stream->require(transport_private_data_length)) {
                ret = ERROR_STREAM_CASTER_TS_AF;
                srs_error("ts: mux af transport_private_data_flag failed. ret=%d", ret);
                return ret;
            }
            stream->write_bytes(transport_private_data, transport_private_data_length);
        }
    }
    if (adaptation_field_extension_flag) {
        if (!stream->require(2)) {
            ret = ERROR_STREAM_CASTER_TS_AF;
            srs_error("ts: mux af adaptation_field_extension_flag failed. ret=%d", ret);
            return ret;
        }
        stream->write_1bytes(adaptation_field_extension_length);
        int8_t ltwfv = const1_value1 & 0x1F;
        ltwfv |= (ltw_flag << 7) & 0x80;
        ltwfv |= (piecewise_rate_flag << 6) & 0x40;
        ltwfv |= (seamless_splice_flag << 5) & 0x20;
        stream->write_1bytes(ltwfv);
        if (ltw_flag) {
            if (!stream->require(2)) {
                ret = ERROR_STREAM_CASTER_TS_AF;
                srs_error("ts: mux af ltw_flag failed. ret=%d", ret);
                return ret;
            }
            stream->skip(2);
            srs_warn("ts: mux af ignore ltw");
        }
        if (piecewise_rate_flag) {
            if (!stream->require(3)) {
                ret = ERROR_STREAM_CASTER_TS_AF;
                srs_error("ts: mux af piecewise_rate_flag failed. ret=%d", ret);
                return ret;
            }
            stream->skip(3);
            srs_warn("ts: mux af ignore piecewise_rate");
        }
        if (seamless_splice_flag) {
            if (!stream->require(5)) {
                ret = ERROR_STREAM_CASTER_TS_AF;
                srs_error("ts: mux af seamless_splice_flag failed. ret=%d", ret);
                return ret;
            }
            stream->skip(5);
            srs_warn("ts: mux af ignore seamless_splice");
        }
        if (nb_af_ext_reserved) {
            stream->skip(nb_af_ext_reserved);
        }
    }
    if (nb_af_reserved) {
        stream->skip(nb_af_reserved);
    }
    srs_info(
        "ts: af parsed, discontinuity=%d random=%d priority=%d PCR=%d OPCR=%d slicing=%d private=%d extension=%d/%d pcr=%" PRId64 "/%d opcr=%" PRId64 "/%d",
        discontinuity_indicator, random_access_indicator, elementary_stream_priority_indicator, PCR_flag, OPCR_flag, splicing_point_flag,
        transport_private_data_flag, adaptation_field_extension_flag, adaptation_field_extension_length, program_clock_reference_base,
        program_clock_reference_extension, original_program_clock_reference_base,
        original_program_clock_reference_extension);
    return ret;
} // SrsTsAdaptationField::encode

SrsTsPayload::SrsTsPayload(SrsTsPacket* p)
{
    packet = p;
}

SrsTsPayload::~SrsTsPayload()
{}

SrsTsPayloadPES::SrsTsPayloadPES(SrsTsPacket* p) : SrsTsPayload(p)
{
    PES_private_data    = NULL;
    pack_field          = NULL;
    PES_extension_field = NULL;
    nb_stuffings        = 0;
    nb_bytes      = 0;
    nb_paddings   = 0;
    const2bits    = 0x02;
    const1_value0 = 0x07;
}

SrsTsPayloadPES::~SrsTsPayloadPES()
{
    srs_freepa(PES_private_data);
    srs_freepa(pack_field);
    srs_freepa(PES_extension_field);
}

int SrsTsPayloadPES::decode(SrsStream* stream, SrsTsMessage** ppmsg)
{
    int ret = ERROR_SUCCESS;
    SrsTsChannel* channel = packet->context->get(packet->pid);

    if (!channel) {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: demux PES no channel for pid=%#x. ret=%d", packet->pid, ret);
        return ret;
    }
    SrsTsMessage* msg = channel->msg;
    if (!msg) {
        msg = new SrsTsMessage(channel, packet);
        channel->msg = msg;
    }
    bool is_fresh_msg = msg->fresh();
    if (is_fresh_msg && !packet->payload_unit_start_indicator) {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: PES fresh packet length=%d, us=%d, cc=%d. ret=%d",
                  msg->PES_packet_length, packet->payload_unit_start_indicator, packet->continuity_counter,
                  ret);
        return ret;
    }
    if (!is_fresh_msg && msg->PES_packet_length > 0 &&
        !msg->completed(packet->payload_unit_start_indicator) &&
        packet->payload_unit_start_indicator
    )
    {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: PES packet length=%d, payload=%d, us=%d, cc=%d. ret=%d",
                  msg->PES_packet_length, msg->payload->length(), packet->payload_unit_start_indicator,
                  packet->continuity_counter, ret);
        stream->skip(stream->pos() * -1);
        srs_freep(msg);
        channel->msg = NULL;
        return ERROR_SUCCESS;
    }
    if (!is_fresh_msg) {
        if (msg->continuity_counter >= packet->continuity_counter &&
            ((msg->continuity_counter + 1) & 0x0f) > packet->continuity_counter
        )
        {
            srs_warn("ts: drop PES %dB for duplicated cc=%#x", msg->continuity_counter);
            stream->skip(stream->size() - stream->pos());
            return ret;
        }
        if (((msg->continuity_counter + 1) & 0x0f) != packet->continuity_counter) {
            ret = ERROR_STREAM_CASTER_TS_PSE;
            srs_error("ts: continuity must be continous, msg=%#x, packet=%#x. ret=%d",
                      msg->continuity_counter, packet->continuity_counter, ret);
            stream->skip(stream->pos() * -1);
            srs_freep(msg);
            channel->msg = NULL;
            return ERROR_SUCCESS;
        }
    }
    msg->continuity_counter = packet->continuity_counter;
    if (!is_fresh_msg && msg->completed(packet->payload_unit_start_indicator)) {
        *ppmsg       = msg;
        channel->msg = NULL;
        stream->skip(stream->pos() * -1);
        return ret;
    }
    if (!packet->payload_unit_start_indicator) {
        if ((ret = msg->dump(stream, &nb_bytes)) != ERROR_SUCCESS) {
            return ret;
        }
    }
    if (packet->payload_unit_start_indicator) {
        if (!stream->require(6)) {
            ret = ERROR_STREAM_CASTER_TS_PSE;
            srs_error("ts: demux PSE failed. ret=%d", ret);
            return ret;
        }
        packet_start_code_prefix = stream->read_3bytes();
        stream_id                 = stream->read_1bytes();
        PES_packet_length         = stream->read_2bytes();
        packet_start_code_prefix &= 0xFFFFFF;
        if (packet_start_code_prefix != 0x01) {
            ret = ERROR_STREAM_CASTER_TS_PSE;
            srs_error("ts: demux PES start code failed, expect=0x01, actual=%#x. ret=%d", packet_start_code_prefix,
                      ret);
            return ret;
        }
        int pos_packet       = stream->pos();
        SrsTsPESStreamId sid = (SrsTsPESStreamId) stream_id;
        msg->sid = sid;
        if (sid != SrsTsPESStreamIdProgramStreamMap &&
            sid != SrsTsPESStreamIdPaddingStream &&
            sid != SrsTsPESStreamIdPrivateStream2 &&
            sid != SrsTsPESStreamIdEcmStream &&
            sid != SrsTsPESStreamIdEmmStream &&
            sid != SrsTsPESStreamIdProgramStreamDirectory &&
            sid != SrsTsPESStreamIdDsmccStream &&
            sid != SrsTsPESStreamIdH2221TypeE
        )
        {
            if (!stream->require(3)) {
                ret = ERROR_STREAM_CASTER_TS_PSE;
                srs_error("ts: demux PES flags failed. ret=%d", ret);
                return ret;
            }
            int8_t oocv = stream->read_1bytes();
            int8_t pefv = stream->read_1bytes();
            PES_header_data_length = stream->read_1bytes();
            int pos_header = stream->pos();
            const2bits = (oocv >> 6) & 0x03;
            PES_scrambling_control = (oocv >> 4) & 0x03;
            PES_priority = (oocv >> 3) & 0x01;
            data_alignment_indicator = (oocv >> 2) & 0x01;
            copyright                 = (oocv >> 1) & 0x01;
            original_or_copy          = oocv & 0x01;
            PTS_DTS_flags             = (pefv >> 6) & 0x03;
            ESCR_flag                 = (pefv >> 5) & 0x01;
            ES_rate_flag              = (pefv >> 4) & 0x01;
            DSM_trick_mode_flag       = (pefv >> 3) & 0x01;
            additional_copy_info_flag = (pefv >> 2) & 0x01;
            PES_CRC_flag              = (pefv >> 1) & 0x01;
            PES_extension_flag        = pefv & 0x01;
            int nb_required = 0;
            nb_required += (PTS_DTS_flags == 0x2) ? 5 : 0;
            nb_required += (PTS_DTS_flags == 0x3) ? 10 : 0;
            nb_required += ESCR_flag ? 6 : 0;
            nb_required += ES_rate_flag ? 3 : 0;
            nb_required += DSM_trick_mode_flag ? 1 : 0;
            nb_required += additional_copy_info_flag ? 1 : 0;
            nb_required += PES_CRC_flag ? 2 : 0;
            nb_required += PES_extension_flag ? 1 : 0;
            if (!stream->require(nb_required)) {
                ret = ERROR_STREAM_CASTER_TS_PSE;
                srs_error("ts: demux PES payload failed. ret=%d", ret);
                return ret;
            }
            if (PTS_DTS_flags == 0x2) {
                if ((ret = decode_33bits_dts_pts(stream, &pts)) != ERROR_SUCCESS) {
                    return ret;
                }
                dts      = pts;
                msg->dts = dts;
                msg->pts = pts;
            }
            if (PTS_DTS_flags == 0x3) {
                if ((ret = decode_33bits_dts_pts(stream, &pts)) != ERROR_SUCCESS) {
                    return ret;
                }
                if ((ret = decode_33bits_dts_pts(stream, &dts)) != ERROR_SUCCESS) {
                    return ret;
                }
                if (dts - pts > 90000 || pts - dts > 90000) {
                    srs_warn("ts: sync dts=%" PRId64 ", pts=%" PRId64, dts, pts);
                }
                msg->dts = dts;
                msg->pts = pts;
            }
            if (ESCR_flag) {
                ESCR_extension = 0;
                ESCR_base      = 0;
                stream->skip(6);
                srs_warn("ts: demux PES, ignore the escr.");
            }
            if (ES_rate_flag) {
                ES_rate  = stream->read_3bytes();
                ES_rate  = ES_rate >> 1;
                ES_rate &= 0x3FFFFF;
            }
            if (DSM_trick_mode_flag) {
                trick_mode_control = stream->read_1bytes();
                trick_mode_value   = trick_mode_control & 0x1f;
                trick_mode_control = (trick_mode_control >> 5) & 0x03;
            }
            if (additional_copy_info_flag) {
                additional_copy_info  = stream->read_1bytes();
                additional_copy_info &= 0x7f;
            }
            if (PES_CRC_flag) {
                previous_PES_packet_CRC = stream->read_2bytes();
            }
            if (PES_extension_flag) {
                int8_t efv = stream->read_1bytes();
                PES_private_data_flag  = (efv >> 7) & 0x01;
                pack_header_field_flag = (efv >> 6) & 0x01;
                program_packet_sequence_counter_flag = (efv >> 5) & 0x01;
                P_STD_buffer_flag    = (efv >> 4) & 0x01;
                const1_value0        = (efv >> 1) & 0x07;
                PES_extension_flag_2 = efv & 0x01;
                nb_required  = 0;
                nb_required += PES_private_data_flag ? 16 : 0;
                nb_required += pack_header_field_flag ? 1 : 0;
                nb_required += program_packet_sequence_counter_flag ? 2 : 0;
                nb_required += P_STD_buffer_flag ? 2 : 0;
                nb_required += PES_extension_flag_2 ? 1 : 0;
                if (!stream->require(nb_required)) {
                    ret = ERROR_STREAM_CASTER_TS_PSE;
                    srs_error("ts: demux PSE ext payload failed. ret=%d", ret);
                    return ret;
                }
                if (PES_private_data_flag) {
                    srs_freepa(PES_private_data);
                    PES_private_data = new char[16];
                    stream->read_bytes(PES_private_data, 16);
                }
                if (pack_header_field_flag) {
                    pack_field_length = stream->read_1bytes();
                    if (pack_field_length > 0) {
                        nb_required = nb_required - 16 - 1 + pack_field_length;
                        if (!stream->require(nb_required)) {
                            ret = ERROR_STREAM_CASTER_TS_PSE;
                            srs_error("ts: demux PSE ext pack failed. ret=%d", ret);
                            return ret;
                        }
                        srs_freepa(pack_field);
                        pack_field = new char[pack_field_length];
                        stream->read_bytes(pack_field, pack_field_length);
                    }
                }
                if (program_packet_sequence_counter_flag) {
                    program_packet_sequence_counter  = stream->read_1bytes();
                    program_packet_sequence_counter &= 0x7f;
                    original_stuff_length  = stream->read_1bytes();
                    MPEG1_MPEG2_identifier = (original_stuff_length >> 6) & 0x01;
                    original_stuff_length &= 0x3f;
                }
                if (P_STD_buffer_flag) {
                    P_STD_buffer_size  = stream->read_2bytes();
                    P_STD_buffer_scale = (P_STD_buffer_scale >> 13) & 0x01;
                    P_STD_buffer_size &= 0x1FFF;
                }
                if (PES_extension_flag_2) {
                    PES_extension_field_length  = stream->read_1bytes();
                    PES_extension_field_length &= 0x07;
                    if (PES_extension_field_length > 0) {
                        if (!stream->require(PES_extension_field_length)) {
                            ret = ERROR_STREAM_CASTER_TS_PSE;
                            srs_error("ts: demux PSE ext field failed. ret=%d", ret);
                            return ret;
                        }
                        srs_freepa(PES_extension_field);
                        PES_extension_field = new char[PES_extension_field_length];
                        stream->read_bytes(PES_extension_field, PES_extension_field_length);
                    }
                }
            }
            nb_stuffings = PES_header_data_length - (stream->pos() - pos_header);
            if (nb_stuffings > 0) {
                if (!stream->require(nb_stuffings)) {
                    ret = ERROR_STREAM_CASTER_TS_PSE;
                    srs_error("ts: demux PSE stuffings failed. ret=%d", ret);
                    return ret;
                }
                stream->skip(nb_stuffings);
            }
            if (PES_packet_length > 0) {
                int nb_packet = PES_packet_length - (stream->pos() - pos_packet);
                msg->PES_packet_length = srs_max(0, nb_packet);
            }
            if ((ret = msg->dump(stream, &nb_bytes)) != ERROR_SUCCESS) {
                return ret;
            }
        } else if (sid == SrsTsPESStreamIdProgramStreamMap ||
                   sid == SrsTsPESStreamIdPrivateStream2 ||
                   sid == SrsTsPESStreamIdEcmStream ||
                   sid == SrsTsPESStreamIdEmmStream ||
                   sid == SrsTsPESStreamIdProgramStreamDirectory ||
                   sid == SrsTsPESStreamIdDsmccStream ||
                   sid == SrsTsPESStreamIdH2221TypeE
        )
        {
            if ((ret = msg->dump(stream, &nb_bytes)) != ERROR_SUCCESS) {
                return ret;
            }
        } else if (sid == SrsTsPESStreamIdPaddingStream) {
            nb_paddings = stream->size() - stream->pos();
            stream->skip(nb_paddings);
            srs_info("ts: drop %dB padding bytes", nb_paddings);
        } else {
            int nb_drop = stream->size() - stream->pos();
            stream->skip(nb_drop);
            srs_warn("ts: drop the pes packet %dB for stream_id=%#x", nb_drop, stream_id);
        }
    }
    if (is_fresh_msg && msg->PES_packet_length == 0) {
        return ret;
    }
    if (msg->completed(packet->payload_unit_start_indicator)) {
        *ppmsg       = msg;
        channel->msg = NULL;
        srs_info("ts: reap msg for completed.");
    }
    return ret;
} // SrsTsPayloadPES::decode

int SrsTsPayloadPES::size()
{
    int sz = 0;

    PES_header_data_length = 0;
    SrsTsPESStreamId sid = (SrsTsPESStreamId) stream_id;
    if (sid != SrsTsPESStreamIdProgramStreamMap &&
        sid != SrsTsPESStreamIdPaddingStream &&
        sid != SrsTsPESStreamIdPrivateStream2 &&
        sid != SrsTsPESStreamIdEcmStream &&
        sid != SrsTsPESStreamIdEmmStream &&
        sid != SrsTsPESStreamIdProgramStreamDirectory &&
        sid != SrsTsPESStreamIdDsmccStream &&
        sid != SrsTsPESStreamIdH2221TypeE
    )
    {
        sz += 6;
        sz += 3;
        PES_header_data_length = sz;
        sz += (PTS_DTS_flags == 0x2) ? 5 : 0;
        sz += (PTS_DTS_flags == 0x3) ? 10 : 0;
        sz += ESCR_flag ? 6 : 0;
        sz += ES_rate_flag ? 3 : 0;
        sz += DSM_trick_mode_flag ? 1 : 0;
        sz += additional_copy_info_flag ? 1 : 0;
        sz += PES_CRC_flag ? 2 : 0;
        sz += PES_extension_flag ? 1 : 0;
        if (PES_extension_flag) {
            sz += PES_private_data_flag ? 16 : 0;
            sz += pack_header_field_flag ? 1 + pack_field_length : 0;
            sz += program_packet_sequence_counter_flag ? 2 : 0;
            sz += P_STD_buffer_flag ? 2 : 0;
            sz += PES_extension_flag_2 ? 1 + PES_extension_field_length : 0;
        }
        PES_header_data_length = sz - PES_header_data_length;
        sz += nb_stuffings;
    } else if (sid == SrsTsPESStreamIdProgramStreamMap ||
               sid == SrsTsPESStreamIdPrivateStream2 ||
               sid == SrsTsPESStreamIdEcmStream ||
               sid == SrsTsPESStreamIdEmmStream ||
               sid == SrsTsPESStreamIdProgramStreamDirectory ||
               sid == SrsTsPESStreamIdDsmccStream ||
               sid == SrsTsPESStreamIdH2221TypeE
    )
    {} else {}
    return sz;
} // SrsTsPayloadPES::size

int SrsTsPayloadPES::encode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(6)) {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: mux PSE failed. ret=%d", ret);
        return ret;
    }
    stream->write_3bytes(packet_start_code_prefix);
    stream->write_1bytes(stream_id);
    int32_t pplv = 0;
    if (PES_packet_length > 0) {
        pplv = PES_packet_length + 3 + PES_header_data_length;
        pplv = (pplv > 0xFFFF) ? 0 : pplv;
    }
    stream->write_2bytes(pplv);
    packet_start_code_prefix &= 0xFFFFFF;
    if (packet_start_code_prefix != 0x01) {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: mux PSE start code failed, expect=0x01, actual=%#x. ret=%d", packet_start_code_prefix, ret);
        return ret;
    }
    if (!stream->require(3)) {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: mux PSE flags failed. ret=%d", ret);
        return ret;
    }
    int8_t oocv = original_or_copy & 0x01;
    oocv |= (const2bits << 6) & 0xC0;
    oocv |= (PES_scrambling_control << 4) & 0x30;
    oocv |= (PES_priority << 3) & 0x08;
    oocv |= (data_alignment_indicator << 2) & 0x04;
    oocv |= (copyright << 1) & 0x02;
    stream->write_1bytes(oocv);
    int8_t pefv = PES_extension_flag & 0x01;
    pefv |= (PTS_DTS_flags << 6) & 0xC0;
    pefv |= (ESCR_flag << 5) & 0x20;
    pefv |= (ES_rate_flag << 4) & 0x10;
    pefv |= (DSM_trick_mode_flag << 3) & 0x08;
    pefv |= (additional_copy_info_flag << 2) & 0x04;
    pefv |= (PES_CRC_flag << 1) & 0x02;
    stream->write_1bytes(pefv);
    stream->write_1bytes(PES_header_data_length);
    int nb_required = 0;
    nb_required += (PTS_DTS_flags == 0x2) ? 5 : 0;
    nb_required += (PTS_DTS_flags == 0x3) ? 10 : 0;
    nb_required += ESCR_flag ? 6 : 0;
    nb_required += ES_rate_flag ? 3 : 0;
    nb_required += DSM_trick_mode_flag ? 1 : 0;
    nb_required += additional_copy_info_flag ? 1 : 0;
    nb_required += PES_CRC_flag ? 2 : 0;
    nb_required += PES_extension_flag ? 1 : 0;
    if (!stream->require(nb_required)) {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: mux PSE payload failed. ret=%d", ret);
        return ret;
    }
    if (PTS_DTS_flags == 0x2) {
        if ((ret = encode_33bits_dts_pts(stream, 0x02, pts)) != ERROR_SUCCESS) {
            return ret;
        }
    }
    if (PTS_DTS_flags == 0x3) {
        if ((ret = encode_33bits_dts_pts(stream, 0x03, pts)) != ERROR_SUCCESS) {
            return ret;
        }
        if ((ret = encode_33bits_dts_pts(stream, 0x01, dts)) != ERROR_SUCCESS) {
            return ret;
        }
        if (dts - pts > 90000 || pts - dts > 90000) {
            srs_warn("ts: sync dts=%" PRId64 ", pts=%" PRId64, dts, pts);
        }
    }
    if (ESCR_flag) {
        stream->skip(6);
        srs_warn("ts: demux PES, ignore the escr.");
    }
    if (ES_rate_flag) {
        stream->skip(3);
        srs_warn("ts: demux PES, ignore the ES_rate.");
    }
    if (DSM_trick_mode_flag) {
        stream->skip(1);
        srs_warn("ts: demux PES, ignore the DSM_trick_mode.");
    }
    if (additional_copy_info_flag) {
        stream->skip(1);
        srs_warn("ts: demux PES, ignore the additional_copy_info.");
    }
    if (PES_CRC_flag) {
        stream->skip(2);
        srs_warn("ts: demux PES, ignore the PES_CRC.");
    }
    if (PES_extension_flag) {
        int8_t efv = PES_extension_flag_2 & 0x01;
        efv |= (PES_private_data_flag << 7) & 0x80;
        efv |= (pack_header_field_flag << 6) & 0x40;
        efv |= (program_packet_sequence_counter_flag << 5) & 0x20;
        efv |= (P_STD_buffer_flag << 4) & 0x10;
        efv |= (const1_value0 << 1) & 0xE0;
        stream->write_1bytes(efv);
        nb_required  = 0;
        nb_required += PES_private_data_flag ? 16 : 0;
        nb_required += pack_header_field_flag ? 1 + pack_field_length : 0;
        nb_required += program_packet_sequence_counter_flag ? 2 : 0;
        nb_required += P_STD_buffer_flag ? 2 : 0;
        nb_required += PES_extension_flag_2 ? 1 + PES_extension_field_length : 0;
        if (!stream->require(nb_required)) {
            ret = ERROR_STREAM_CASTER_TS_PSE;
            srs_error("ts: mux PSE ext payload failed. ret=%d", ret);
            return ret;
        }
        stream->skip(nb_required);
        srs_warn("ts: demux PES, ignore the PES_extension.");
    }
    if (nb_stuffings) {
        stream->skip(nb_stuffings);
        srs_warn("ts: demux PES, ignore the stuffings.");
    }
    return ret;
} // SrsTsPayloadPES::encode

int SrsTsPayloadPES::decode_33bits_dts_pts(SrsStream* stream, int64_t* pv)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(5)) {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: demux PSE dts/pts failed. ret=%d", ret);
        return ret;
    }
    int64_t dts_pts_30_32 = stream->read_1bytes();
    if ((dts_pts_30_32 & 0x01) != 0x01) {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: demux PSE dts/pts 30-32 failed. ret=%d", ret);
        return ret;
    }
    if (((dts_pts_30_32 >> 4) & 0x0f) == 0x00) {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: demux PSE dts/pts 30-32 failed. ret=%d", ret);
        return ret;
    }
    dts_pts_30_32 = (dts_pts_30_32 >> 1) & 0x07;
    int64_t dts_pts_15_29 = stream->read_2bytes();
    if ((dts_pts_15_29 & 0x01) != 0x01) {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: demux PSE dts/pts 15-29 failed. ret=%d", ret);
        return ret;
    }
    dts_pts_15_29 = (dts_pts_15_29 >> 1) & 0x7fff;
    int64_t dts_pts_0_14 = stream->read_2bytes();
    if ((dts_pts_0_14 & 0x01) != 0x01) {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: demux PSE dts/pts 0-14 failed. ret=%d", ret);
        return ret;
    }
    dts_pts_0_14 = (dts_pts_0_14 >> 1) & 0x7fff;
    int64_t v = 0x00;
    v  |= (dts_pts_30_32 << 30) & 0x1c0000000LL;
    v  |= (dts_pts_15_29 << 15) & 0x3fff8000LL;
    v  |= dts_pts_0_14 & 0x7fff;
    *pv = v;
    return ret;
} // SrsTsPayloadPES::decode_33bits_dts_pts

int SrsTsPayloadPES::encode_33bits_dts_pts(SrsStream* stream, u_int8_t fb, int64_t v)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(5)) {
        ret = ERROR_STREAM_CASTER_TS_PSE;
        srs_error("ts: mux PSE dts/pts failed. ret=%d", ret);
        return ret;
    }
    char* p = stream->data() + stream->pos();
    stream->skip(5);
    int32_t val = 0;
    val  = fb << 4 | (((v >> 30) & 0x07) << 1) | 1;
    *p++ = val;
    val  = (((v >> 15) & 0x7fff) << 1) | 1;
    *p++ = (val >> 8);
    *p++ = val;
    val  = (((v) & 0x7fff) << 1) | 1;
    *p++ = (val >> 8);
    *p++ = val;
    return ret;
}

SrsTsPayloadPSI::SrsTsPayloadPSI(SrsTsPacket* p) : SrsTsPayload(p)
{
    pointer_field = 0;
    const0_value  = 0;
    const1_value  = 3;
    CRC_32        = 0;
}

SrsTsPayloadPSI::~SrsTsPayloadPSI()
{}

int SrsTsPayloadPSI::decode(SrsStream* stream, SrsTsMessage **)
{
    int ret = ERROR_SUCCESS;

    if (packet->payload_unit_start_indicator) {
        if (!stream->require(1)) {
            ret = ERROR_STREAM_CASTER_TS_PSI;
            srs_error("ts: demux PSI failed. ret=%d", ret);
            return ret;
        }
        pointer_field = stream->read_1bytes();
    }
    char* ppat  = stream->data() + stream->pos();
    int pat_pos = stream->pos();
    if (!stream->require(3)) {
        ret = ERROR_STREAM_CASTER_TS_PSI;
        srs_error("ts: demux PSI failed. ret=%d", ret);
        return ret;
    }
    table_id = (SrsTsPsiId) stream->read_1bytes();
    int16_t slv = stream->read_2bytes();
    section_syntax_indicator = (slv >> 15) & 0x01;
    const0_value   = (slv >> 14) & 0x01;
    const1_value   = (slv >> 12) & 0x03;
    section_length = slv & 0x0FFF;
    if (section_length == 0) {
        srs_warn("ts: demux PAT ignore empty section");
        return ret;
    }
    if (!stream->require(section_length)) {
        ret = ERROR_STREAM_CASTER_TS_PSI;
        srs_error("ts: demux PAT section failed. ret=%d", ret);
        return ret;
    }
    if ((ret = psi_decode(stream)) != ERROR_SUCCESS) {
        return ret;
    }
    if (!stream->require(4)) {
        ret = ERROR_STREAM_CASTER_TS_PSI;
        srs_error("ts: demux PSI crc32 failed. ret=%d", ret);
        return ret;
    }
    CRC_32 = stream->read_4bytes();
    int32_t crc32 = srs_crc32(ppat, stream->pos() - pat_pos - 4);
    if (crc32 != CRC_32) {
        ret = ERROR_STREAM_CASTER_TS_CRC32;
        srs_error("ts: verify PSI crc32 failed. ret=%d", ret);
        return ret;
    }
    if (!stream->empty()) {
        int nb_stuffings = stream->size() - stream->pos();
        char* stuffing   = stream->data() + stream->pos();
        for (int i = 0; i < nb_stuffings; i++) {
            if ((u_int8_t) stuffing[i] != 0xff) {
                srs_warn("ts: stuff is not 0xff, actual=%#x", stuffing[i]);
                break;
            }
        }
        stream->skip(nb_stuffings);
    }
    return ret;
} // SrsTsPayloadPSI::decode

int SrsTsPayloadPSI::size()
{
    int sz = 0;

    section_length = psi_size() + 4;
    sz += packet->payload_unit_start_indicator ? 1 : 0;
    sz += 3;
    sz += section_length;
    return sz;
}

int SrsTsPayloadPSI::encode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (packet->payload_unit_start_indicator) {
        if (!stream->require(1)) {
            ret = ERROR_STREAM_CASTER_TS_PSI;
            srs_error("ts: mux PSI failed. ret=%d", ret);
            return ret;
        }
        stream->write_1bytes(pointer_field);
    }
    char* ppat  = stream->data() + stream->pos();
    int pat_pos = stream->pos();
    if (!stream->require(3)) {
        ret = ERROR_STREAM_CASTER_TS_PSI;
        srs_error("ts: mux PSI failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(table_id);
    int16_t slv = section_length & 0x0FFF;
    slv |= (section_syntax_indicator << 15) & 0x8000;
    slv |= (const0_value << 14) & 0x4000;
    slv |= (const1_value << 12) & 0x3000;
    stream->write_2bytes(slv);
    if (section_length == 0) {
        srs_warn("ts: mux PAT ignore empty section");
        return ret;
    }
    if (!stream->require(section_length)) {
        ret = ERROR_STREAM_CASTER_TS_PSI;
        srs_error("ts: mux PAT section failed. ret=%d", ret);
        return ret;
    }
    if ((ret = psi_encode(stream)) != ERROR_SUCCESS) {
        return ret;
    }
    if (!stream->require(4)) {
        ret = ERROR_STREAM_CASTER_TS_PSI;
        srs_error("ts: mux PSI crc32 failed. ret=%d", ret);
        return ret;
    }
    CRC_32 = srs_crc32(ppat, stream->pos() - pat_pos);
    stream->write_4bytes(CRC_32);
    return ret;
} // SrsTsPayloadPSI::encode

SrsTsPayloadPATProgram::SrsTsPayloadPATProgram(int16_t n, int16_t p)
{
    number       = n;
    pid          = p;
    const1_value = 0x07;
}

SrsTsPayloadPATProgram::~SrsTsPayloadPATProgram()
{}

int SrsTsPayloadPATProgram::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(4)) {
        ret = ERROR_STREAM_CASTER_TS_PAT;
        srs_error("ts: demux PAT failed. ret=%d", ret);
        return ret;
    }
    int tmpv = stream->read_4bytes();
    number       = (int16_t) ((tmpv >> 16) & 0xFFFF);
    const1_value = (int16_t) ((tmpv >> 13) & 0x07);
    pid = (int16_t) (tmpv & 0x1FFF);
    return ret;
}

int SrsTsPayloadPATProgram::size()
{
    return 4;
}

int SrsTsPayloadPATProgram::encode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(4)) {
        ret = ERROR_STREAM_CASTER_TS_PAT;
        srs_error("ts: mux PAT failed. ret=%d", ret);
        return ret;
    }
    int tmpv = pid & 0x1FFF;
    tmpv |= (number << 16) & 0xFFFF0000;
    tmpv |= (const1_value << 13) & 0xE000;
    stream->write_4bytes(tmpv);
    return ret;
}

SrsTsPayloadPAT::SrsTsPayloadPAT(SrsTsPacket* p) : SrsTsPayloadPSI(p)
{
    const3_value = 3;
}

SrsTsPayloadPAT::~SrsTsPayloadPAT()
{
    std::vector<SrsTsPayloadPATProgram *>::iterator it;

    for (it = programs.begin(); it != programs.end(); ++it) {
        SrsTsPayloadPATProgram* program = *it;
        srs_freep(program);
    }
    programs.clear();
}

int SrsTsPayloadPAT::psi_decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(5)) {
        ret = ERROR_STREAM_CASTER_TS_PAT;
        srs_error("ts: demux PAT failed. ret=%d", ret);
        return ret;
    }
    int pos = stream->pos();
    transport_stream_id = stream->read_2bytes();
    int8_t cniv = stream->read_1bytes();
    const3_value           = (cniv >> 6) & 0x03;
    version_number         = (cniv >> 1) & 0x1F;
    current_next_indicator = cniv & 0x01;
    section_number         = stream->read_1bytes();
    last_section_number    = stream->read_1bytes();
    int program_bytes = section_length - 4 - (stream->pos() - pos);
    for (int i = 0; i < program_bytes; i += 4) {
        SrsTsPayloadPATProgram* program = new SrsTsPayloadPATProgram();
        if ((ret = program->decode(stream)) != ERROR_SUCCESS) {
            return ret;
        }
        packet->context->set(program->pid, SrsTsPidApplyPMT);
        programs.push_back(program);
    }
    packet->context->set(packet->pid, SrsTsPidApplyPAT);
    packet->context->on_pmt_parsed();
    return ret;
} // SrsTsPayloadPAT::psi_decode

int SrsTsPayloadPAT::psi_size()
{
    int sz = 5;

    for (int i = 0; i < (int) programs.size(); i++) {
        SrsTsPayloadPATProgram* program = programs.at(i);
        sz += program->size();
    }
    return sz;
}

int SrsTsPayloadPAT::psi_encode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(5)) {
        ret = ERROR_STREAM_CASTER_TS_PAT;
        srs_error("ts: mux PAT failed. ret=%d", ret);
        return ret;
    }
    stream->write_2bytes(transport_stream_id);
    int8_t cniv = current_next_indicator & 0x01;
    cniv |= (version_number << 1) & 0x3E;
    cniv |= (const1_value << 6) & 0xC0;
    stream->write_1bytes(cniv);
    stream->write_1bytes(section_number);
    stream->write_1bytes(last_section_number);
    for (int i = 0; i < (int) programs.size(); i++) {
        SrsTsPayloadPATProgram* program = programs.at(i);
        if ((ret = program->encode(stream)) != ERROR_SUCCESS) {
            return ret;
        }
        packet->context->set(program->pid, SrsTsPidApplyPMT);
    }
    packet->context->set(packet->pid, SrsTsPidApplyPAT);
    return ret;
}

SrsTsPayloadPMTESInfo::SrsTsPayloadPMTESInfo(SrsTsStream st, int16_t epid)
{
    stream_type    = st;
    elementary_PID = epid;
    const1_value0  = 7;
    const1_value1  = 0x0f;
    ES_info_length = 0;
    ES_info        = NULL;
}

SrsTsPayloadPMTESInfo::~SrsTsPayloadPMTESInfo()
{
    srs_freepa(ES_info);
}

int SrsTsPayloadPMTESInfo::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(5)) {
        ret = ERROR_STREAM_CASTER_TS_PMT;
        srs_error("ts: demux PMT es info failed. ret=%d", ret);
        return ret;
    }
    stream_type = (SrsTsStream) stream->read_1bytes();
    int16_t epv = stream->read_2bytes();
    const1_value0  = (epv >> 13) & 0x07;
    elementary_PID = epv & 0x1FFF;
    int16_t eilv = stream->read_2bytes();
    const1_value1  = (epv >> 12) & 0x0f;
    ES_info_length = eilv & 0x0FFF;
    if (ES_info_length > 0) {
        if (!stream->require(ES_info_length)) {
            ret = ERROR_STREAM_CASTER_TS_PMT;
            srs_error("ts: demux PMT es info data failed. ret=%d", ret);
            return ret;
        }
        srs_freepa(ES_info);
        ES_info = new char[ES_info_length];
        stream->read_bytes(ES_info, ES_info_length);
    }
    return ret;
}

int SrsTsPayloadPMTESInfo::size()
{
    return 5 + ES_info_length;
}

int SrsTsPayloadPMTESInfo::encode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(5)) {
        ret = ERROR_STREAM_CASTER_TS_PMT;
        srs_error("ts: mux PMT es info failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(stream_type);
    int16_t epv = elementary_PID & 0x1FFF;
    epv |= (const1_value0 << 13) & 0xE000;
    stream->write_2bytes(epv);
    int16_t eilv = ES_info_length & 0x0FFF;
    eilv |= (const1_value1 << 12) & 0xF000;
    stream->write_2bytes(eilv);
    if (ES_info_length > 0) {
        if (!stream->require(ES_info_length)) {
            ret = ERROR_STREAM_CASTER_TS_PMT;
            srs_error("ts: mux PMT es info data failed. ret=%d", ret);
            return ret;
        }
        stream->write_bytes(ES_info, ES_info_length);
    }
    return ret;
}

SrsTsPayloadPMT::SrsTsPayloadPMT(SrsTsPacket* p) : SrsTsPayloadPSI(p)
{
    const1_value0       = 3;
    const1_value1       = 7;
    const1_value2       = 0x0f;
    program_info_length = 0;
    program_info_desc   = NULL;
}

SrsTsPayloadPMT::~SrsTsPayloadPMT()
{
    srs_freepa(program_info_desc);
    std::vector<SrsTsPayloadPMTESInfo *>::iterator it;
    for (it = infos.begin(); it != infos.end(); ++it) {
        SrsTsPayloadPMTESInfo* info = *it;
        srs_freep(info);
    }
    infos.clear();
}

int SrsTsPayloadPMT::psi_decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(9)) {
        ret = ERROR_STREAM_CASTER_TS_PMT;
        srs_error("ts: demux PMT failed. ret=%d", ret);
        return ret;
    }
    program_number = stream->read_2bytes();
    int8_t cniv = stream->read_1bytes();
    const1_value0          = (cniv >> 6) & 0x03;
    version_number         = (cniv >> 1) & 0x1F;
    current_next_indicator = cniv & 0x01;
    section_number         = stream->read_1bytes();
    last_section_number    = stream->read_1bytes();
    int16_t ppv = stream->read_2bytes();
    const1_value1 = (ppv >> 13) & 0x07;
    PCR_PID       = ppv & 0x1FFF;
    int16_t pilv = stream->read_2bytes();
    const1_value2       = (pilv >> 12) & 0x0F;
    program_info_length = pilv & 0xFFF;
    if (program_info_length > 0) {
        if (!stream->require(program_info_length)) {
            ret = ERROR_STREAM_CASTER_TS_PMT;
            srs_error("ts: demux PMT program info failed. ret=%d", ret);
            return ret;
        }
        srs_freepa(program_info_desc);
        program_info_desc = new char[program_info_length];
        stream->read_bytes(program_info_desc, program_info_length);
    }
    int ES_EOF_pos = stream->pos() + section_length - 4 - 9 - program_info_length;
    while (stream->pos() < ES_EOF_pos) {
        SrsTsPayloadPMTESInfo* info = new SrsTsPayloadPMTESInfo();
        infos.push_back(info);
        if ((ret = info->decode(stream)) != ERROR_SUCCESS) {
            return ret;
        }
        switch (info->stream_type) {
            case SrsTsStreamVideoH264:
            case SrsTsStreamVideoMpeg4:
                packet->context->set(info->elementary_PID, SrsTsPidApplyVideo, info->stream_type);
                break;
            case SrsTsStreamAudioAAC:
            case SrsTsStreamAudioAC3:
            case SrsTsStreamAudioDTS:
            case SrsTsStreamAudioMp3:
                packet->context->set(info->elementary_PID, SrsTsPidApplyAudio, info->stream_type);
                break;
            default:
                srs_warn("ts: drop pid=%#x, stream=%#x", info->elementary_PID, info->stream_type);
                break;
        }
    }
    packet->context->set(packet->pid, SrsTsPidApplyPMT);
    return ret;
} // SrsTsPayloadPMT::psi_decode

int SrsTsPayloadPMT::psi_size()
{
    int sz = 9;

    sz += program_info_length;
    for (int i = 0; i < (int) infos.size(); i++) {
        SrsTsPayloadPMTESInfo* info = infos.at(i);
        sz += info->size();
    }
    return sz;
}

int SrsTsPayloadPMT::psi_encode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(9)) {
        ret = ERROR_STREAM_CASTER_TS_PMT;
        srs_error("ts: mux PMT failed. ret=%d", ret);
        return ret;
    }
    stream->write_2bytes(program_number);
    int8_t cniv = current_next_indicator & 0x01;
    cniv |= (const1_value0 << 6) & 0xC0;
    cniv |= (version_number << 1) & 0xFE;
    stream->write_1bytes(cniv);
    stream->write_1bytes(section_number);
    stream->write_1bytes(last_section_number);
    int16_t ppv = PCR_PID & 0x1FFF;
    ppv |= (const1_value1 << 13) & 0xE000;
    stream->write_2bytes(ppv);
    int16_t pilv = program_info_length & 0xFFF;
    pilv |= (const1_value2 << 12) & 0xF000;
    stream->write_2bytes(pilv);
    if (program_info_length > 0) {
        if (!stream->require(program_info_length)) {
            ret = ERROR_STREAM_CASTER_TS_PMT;
            srs_error("ts: mux PMT program info failed. ret=%d", ret);
            return ret;
        }
        stream->write_bytes(program_info_desc, program_info_length);
    }
    for (int i = 0; i < (int) infos.size(); i++) {
        SrsTsPayloadPMTESInfo* info = infos.at(i);
        if ((ret = info->encode(stream)) != ERROR_SUCCESS) {
            return ret;
        }
        switch (info->stream_type) {
            case SrsTsStreamVideoH264:
            case SrsTsStreamVideoMpeg4:
                packet->context->set(info->elementary_PID, SrsTsPidApplyVideo, info->stream_type);
                break;
            case SrsTsStreamAudioAAC:
            case SrsTsStreamAudioAC3:
            case SrsTsStreamAudioDTS:
            case SrsTsStreamAudioMp3:
                packet->context->set(info->elementary_PID, SrsTsPidApplyAudio, info->stream_type);
                break;
            default:
                srs_warn("ts: drop pid=%#x, stream=%#x", info->elementary_PID, info->stream_type);
                break;
        }
    }
    packet->context->set(packet->pid, SrsTsPidApplyPMT);
    return ret;
} // SrsTsPayloadPMT::psi_encode

SrsTSMuxer::SrsTSMuxer(SrsFileWriter* w, SrsTsContext* c, SrsCodecAudio ac, SrsCodecVideo vc)
{
    writer  = w;
    context = c;
    acodec  = ac;
    vcodec  = vc;
}

SrsTSMuxer::~SrsTSMuxer()
{
    close();
}

int SrsTSMuxer::open(string p)
{
    int ret = ERROR_SUCCESS;

    path = p;
    close();
    context->reset();
    if ((ret = writer->open(path)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int SrsTSMuxer::update_acodec(SrsCodecAudio ac)
{
    acodec = ac;
    return ERROR_SUCCESS;
}

int SrsTSMuxer::write_audio(SrsTsMessage* audio)
{
    int ret = ERROR_SUCCESS;

    srs_info("hls: write audio pts=%" PRId64 ", dts=%" PRId64 ", size=%d",
             audio->pts, audio->dts, audio->PES_packet_length);
    if ((ret = context->encode(writer, audio, vcodec, acodec)) != ERROR_SUCCESS) {
        srs_error("hls encode audio failed. ret=%d", ret);
        return ret;
    }
    srs_info("hls encode audio ok");
    return ret;
}

int SrsTSMuxer::write_video(SrsTsMessage* video)
{
    int ret = ERROR_SUCCESS;

    srs_info("hls: write video pts=%" PRId64 ", dts=%" PRId64 ", size=%d",
             video->pts, video->dts, video->PES_packet_length);
    if ((ret = context->encode(writer, video, vcodec, acodec)) != ERROR_SUCCESS) {
        srs_error("hls encode video failed. ret=%d", ret);
        return ret;
    }
    srs_info("hls encode video ok");
    return ret;
}

void SrsTSMuxer::close()
{
    writer->close();
}

SrsCodecVideo SrsTSMuxer::video_codec()
{
    return vcodec;
}

SrsTsCache::SrsTsCache()
{
    audio = NULL;
    video = NULL;
}

SrsTsCache::~SrsTsCache()
{
    srs_freep(audio);
    srs_freep(video);
}

int SrsTsCache::cache_audio(SrsAvcAacCodec* codec, int64_t dts, SrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;

    if (!audio) {
        audio = new SrsTsMessage();
        audio->write_pcr = false;
        audio->dts       = audio->pts = audio->start_pts = dts;
    }
    audio->sid = SrsTsPESStreamIdAudioCommon;
    SrsCodecAudio acodec = (SrsCodecAudio) codec->audio_codec_id;
    srs_assert(acodec == SrsCodecAudioAAC || acodec == SrsCodecAudioMP3);
    if (codec->audio_codec_id == SrsCodecAudioAAC) {
        if ((ret = do_cache_aac(codec, sample)) != ERROR_SUCCESS) {
            return ret;
        }
    } else {
        if ((ret = do_cache_mp3(codec, sample)) != ERROR_SUCCESS) {
            return ret;
        }
    }
    return ret;
}

int SrsTsCache::cache_video(SrsAvcAacCodec* codec, int64_t dts, SrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;

    if (!video) {
        video = new SrsTsMessage();
        video->write_pcr = sample->frame_type == SrsCodecVideoAVCFrameKeyFrame;
        video->start_pts = dts;
    }
    video->dts = dts;
    video->pts = video->dts + sample->cts * 90;
    video->sid = SrsTsPESStreamIdVideoCommon;
    if ((ret = do_cache_avc(codec, sample)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int SrsTsCache::do_cache_mp3(SrsAvcAacCodec* codec, SrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;

    for (int i = 0; i < sample->nb_sample_units; i++) {
        SrsCodecSampleUnit* sample_unit = &sample->sample_units[i];
        audio->payload->append(sample_unit->bytes, sample_unit->size);
    }
    return ret;
}

int SrsTsCache::do_cache_aac(SrsAvcAacCodec* codec, SrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;

    for (int i = 0; i < sample->nb_sample_units; i++) {
        SrsCodecSampleUnit* sample_unit = &sample->sample_units[i];
        int32_t size = sample_unit->size;
        if (!sample_unit->bytes || size <= 0 || size > 0x1fff) {
            ret = ERROR_HLS_AAC_FRAME_LENGTH;
            srs_error("invalid aac frame length=%d, ret=%d", size, ret);
            return ret;
        }
        int32_t frame_length      = size + 7;
        u_int8_t adts_header[7]   = { 0xff, 0xf9, 0x00, 0x00, 0x00, 0x0f, 0xfc };
        SrsAacProfile aac_profile = srs_codec_aac_rtmp2ts(codec->aac_object);
        adts_header[2]  = (aac_profile << 6) & 0xc0;
        adts_header[2] |= (codec->aac_sample_rate << 2) & 0x3c;
        adts_header[2] |= (codec->aac_channels >> 2) & 0x01;
        adts_header[3]  = (codec->aac_channels << 6) & 0xc0;
        adts_header[3] |= (frame_length >> 11) & 0x03;
        adts_header[4]  = (frame_length >> 3) & 0xff;
        adts_header[5]  = ((frame_length << 5) & 0xe0);
        adts_header[5] |= 0x1f;
        audio->payload->append((const char *) adts_header, sizeof(adts_header));
        audio->payload->append(sample_unit->bytes, sample_unit->size);
    }
    return ret;
} // SrsTsCache::do_cache_aac

int SrsTsCache::do_cache_avc(SrsAvcAacCodec* codec, SrsCodecSample* sample)
{
    int ret = ERROR_SUCCESS;
    static u_int8_t fresh_nalu_header[] = { 0x00, 0x00, 0x00, 0x01 };
    static u_int8_t cont_nalu_header[]  = { 0x00, 0x00, 0x01 };
    static u_int8_t aud_nalu_7[]        = { 0x09, 0xf0 };

    video->payload->append((const char *) fresh_nalu_header, 4);
    video->payload->append((const char *) aud_nalu_7, 2);
    if (sample->has_idr) {
        if (codec->sequenceParameterSetLength > 0) {
            video->payload->append((const char *) fresh_nalu_header, 4);
            video->payload->append(codec->sequenceParameterSetNALUnit, codec->sequenceParameterSetLength);
        }
        if (codec->pictureParameterSetLength > 0) {
            video->payload->append((const char *) cont_nalu_header, 3);
            video->payload->append(codec->pictureParameterSetNALUnit, codec->pictureParameterSetLength);
        }
    }
    for (int i = 0; i < sample->nb_sample_units; i++) {
        SrsCodecSampleUnit* sample_unit = &sample->sample_units[i];
        int32_t size = sample_unit->size;
        if (!sample_unit->bytes || size <= 0) {
            ret = ERROR_HLS_AVC_SAMPLE_SIZE;
            srs_error("invalid avc sample length=%d, ret=%d", size, ret);
            return ret;
        }
        SrsAvcNaluType nal_unit_type = (SrsAvcNaluType) (sample_unit->bytes[0] & 0x1f);
        switch (nal_unit_type) {
            case SrsAvcNaluTypeSPS:
            case SrsAvcNaluTypePPS:
            case SrsAvcNaluTypeAccessUnitDelimiter:
                continue;
            default:
                break;
        }
        video->payload->append((const char *) cont_nalu_header, 3);
        video->payload->append(sample_unit->bytes, sample_unit->size);
    }
    return ret;
} // SrsTsCache::do_cache_avc

SrsTsEncoder::SrsTsEncoder()
{
    writer  = NULL;
    codec   = new SrsAvcAacCodec();
    sample  = new SrsCodecSample();
    cache   = new SrsTsCache();
    context = new SrsTsContext();
    muxer   = NULL;
}

SrsTsEncoder::~SrsTsEncoder()
{
    srs_freep(codec);
    srs_freep(sample);
    srs_freep(cache);
    srs_freep(muxer);
    srs_freep(context);
}

int SrsTsEncoder::initialize(SrsFileWriter* fw)
{
    int ret = ERROR_SUCCESS;

    srs_assert(fw);
    if (!fw->is_open()) {
        ret = ERROR_KERNEL_FLV_STREAM_CLOSED;
        srs_warn("stream is not open for encoder. ret=%d", ret);
        return ret;
    }
    writer = fw;
    srs_freep(muxer);
    muxer = new SrsTSMuxer(fw, context, SrsCodecAudioAAC, SrsCodecVideoAVC);
    if ((ret = muxer->open("")) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int SrsTsEncoder::write_audio(int64_t timestamp, char* data, int size)
{
    int ret = ERROR_SUCCESS;

    sample->clear();
    if ((ret = codec->audio_aac_demux(data, size, sample)) != ERROR_SUCCESS) {
        if (ret != ERROR_HLS_TRY_MP3) {
            srs_error("http: ts aac demux audio failed. ret=%d", ret);
            return ret;
        }
        if ((ret = codec->audio_mp3_demux(data, size, sample)) != ERROR_SUCCESS) {
            srs_error("http: ts mp3 demux audio failed. ret=%d", ret);
            return ret;
        }
    }
    SrsCodecAudio acodec = (SrsCodecAudio) codec->audio_codec_id;
    if (acodec != SrsCodecAudioAAC && acodec != SrsCodecAudioMP3) {
        return ret;
    }
    if ((ret = muxer->update_acodec(acodec)) != ERROR_SUCCESS) {
        srs_error("http: ts audio write header failed. ret=%d", ret);
        return ret;
    }
    if (acodec == SrsCodecAudioAAC && sample->aac_packet_type == SrsCodecAudioTypeSequenceHeader) {
        return ret;
    }
    int64_t dts = timestamp * 90;
    if ((ret = cache->cache_audio(codec, dts, sample)) != ERROR_SUCCESS) {
        return ret;
    }
    return flush_audio();
} // SrsTsEncoder::write_audio

int SrsTsEncoder::write_video(int64_t timestamp, char* data, int size)
{
    int ret = ERROR_SUCCESS;

    sample->clear();
    if ((ret = codec->video_avc_demux(data, size, sample)) != ERROR_SUCCESS) {
        srs_error("http: ts codec demux video failed. ret=%d", ret);
        return ret;
    }
    if (sample->frame_type == SrsCodecVideoAVCFrameVideoInfoFrame) {
        return ret;
    }
    if (codec->video_codec_id != SrsCodecVideoAVC) {
        return ret;
    }
    if (sample->frame_type == SrsCodecVideoAVCFrameKeyFrame &&
        sample->avc_packet_type == SrsCodecVideoAVCTypeSequenceHeader)
    {
        return ret;
    }
    int64_t dts = timestamp * 90;
    if ((ret = cache->cache_video(codec, dts, sample)) != ERROR_SUCCESS) {
        return ret;
    }
    return flush_video();
}

int SrsTsEncoder::flush_audio()
{
    int ret = ERROR_SUCCESS;

    if ((ret = muxer->write_audio(cache->audio)) != ERROR_SUCCESS) {
        return ret;
    }
    srs_freep(cache->audio);
    return ret;
}

int SrsTsEncoder::flush_video()
{
    int ret = ERROR_SUCCESS;

    if ((ret = muxer->write_video(cache->video)) != ERROR_SUCCESS) {
        return ret;
    }
    srs_freep(cache->video);
    return ret;
}

#endif // if !defined(SRS_EXPORT_LIBRTMP)
SrsSimpleBuffer::SrsSimpleBuffer()
{}

SrsSimpleBuffer::~SrsSimpleBuffer()
{}

int SrsSimpleBuffer::length()
{
    int len = (int) data.size();

    srs_assert(len >= 0);
    return len;
}

char * SrsSimpleBuffer::bytes()
{
    return (length() == 0) ? NULL : &data.at(0);
}

void SrsSimpleBuffer::erase(int size)
{
    if (size <= 0) {
        return;
    }
    if (size >= length()) {
        data.clear();
        return;
    }
    data.erase(data.begin(), data.begin() + size);
}

void SrsSimpleBuffer::append(const char* bytes, int size)
{
    srs_assert(size > 0);
    data.insert(data.end(), bytes, bytes + size);
}

#include <utility>
#include <vector>
#include <sstream>
using namespace std;
using namespace _srs_internal;
#define RTMP_AMF0_Number            0x00
#define RTMP_AMF0_Boolean           0x01
#define RTMP_AMF0_String            0x02
#define RTMP_AMF0_Object            0x03
#define RTMP_AMF0_MovieClip         0x04
#define RTMP_AMF0_Null              0x05
#define RTMP_AMF0_Undefined         0x06
#define RTMP_AMF0_Reference         0x07
#define RTMP_AMF0_EcmaArray         0x08
#define RTMP_AMF0_ObjectEnd         0x09
#define RTMP_AMF0_StrictArray       0x0A
#define RTMP_AMF0_Date              0x0B
#define RTMP_AMF0_LongString        0x0C
#define RTMP_AMF0_UnSupported       0x0D
#define RTMP_AMF0_RecordSet         0x0E
#define RTMP_AMF0_XmlDocument       0x0F
#define RTMP_AMF0_TypedObject       0x10
#define RTMP_AMF0_AVMplusObject     0x11
#define RTMP_AMF0_OriginStrictArray 0x20
#define RTMP_AMF0_Invalid           0x3F
SrsAmf0Any::SrsAmf0Any()
{
    marker = RTMP_AMF0_Invalid;
}

SrsAmf0Any::~SrsAmf0Any()
{}

bool SrsAmf0Any::is_string()
{
    return marker == RTMP_AMF0_String;
}

bool SrsAmf0Any::is_boolean()
{
    return marker == RTMP_AMF0_Boolean;
}

bool SrsAmf0Any::is_number()
{
    return marker == RTMP_AMF0_Number;
}

bool SrsAmf0Any::is_null()
{
    return marker == RTMP_AMF0_Null;
}

bool SrsAmf0Any::is_undefined()
{
    return marker == RTMP_AMF0_Undefined;
}

bool SrsAmf0Any::is_object()
{
    return marker == RTMP_AMF0_Object;
}

bool SrsAmf0Any::is_ecma_array()
{
    return marker == RTMP_AMF0_EcmaArray;
}

bool SrsAmf0Any::is_strict_array()
{
    return marker == RTMP_AMF0_StrictArray;
}

bool SrsAmf0Any::is_date()
{
    return marker == RTMP_AMF0_Date;
}

bool SrsAmf0Any::is_complex_object()
{
    return is_object() || is_object_eof() || is_ecma_array() || is_strict_array();
}

string SrsAmf0Any::to_str()
{
    SrsAmf0String* p = dynamic_cast<SrsAmf0String *>(this);

    srs_assert(p != NULL);
    return p->value;
}

const char * SrsAmf0Any::to_str_raw()
{
    SrsAmf0String* p = dynamic_cast<SrsAmf0String *>(this);

    srs_assert(p != NULL);
    return p->value.data();
}

bool SrsAmf0Any::to_boolean()
{
    SrsAmf0Boolean* p = dynamic_cast<SrsAmf0Boolean *>(this);

    srs_assert(p != NULL);
    return p->value;
}

double SrsAmf0Any::to_number()
{
    SrsAmf0Number* p = dynamic_cast<SrsAmf0Number *>(this);

    srs_assert(p != NULL);
    return p->value;
}

int64_t SrsAmf0Any::to_date()
{
    SrsAmf0Date* p = dynamic_cast<SrsAmf0Date *>(this);

    srs_assert(p != NULL);
    return p->date();
}

int16_t SrsAmf0Any::to_date_time_zone()
{
    SrsAmf0Date* p = dynamic_cast<SrsAmf0Date *>(this);

    srs_assert(p != NULL);
    return p->time_zone();
}

SrsAmf0Object * SrsAmf0Any::to_object()
{
    SrsAmf0Object* p = dynamic_cast<SrsAmf0Object *>(this);

    srs_assert(p != NULL);
    return p;
}

SrsAmf0EcmaArray * SrsAmf0Any::to_ecma_array()
{
    SrsAmf0EcmaArray* p = dynamic_cast<SrsAmf0EcmaArray *>(this);

    srs_assert(p != NULL);
    return p;
}

SrsAmf0StrictArray * SrsAmf0Any::to_strict_array()
{
    SrsAmf0StrictArray* p = dynamic_cast<SrsAmf0StrictArray *>(this);

    srs_assert(p != NULL);
    return p;
}

void SrsAmf0Any::set_number(double value)
{
    SrsAmf0Number* p = dynamic_cast<SrsAmf0Number *>(this);

    srs_assert(p != NULL);
    p->value = value;
}

bool SrsAmf0Any::is_object_eof()
{
    return marker == RTMP_AMF0_ObjectEnd;
}

void srs_fill_level_spaces(stringstream& ss, int level)
{
    for (int i = 0; i < level; i++) {
        ss << "    ";
    }
}

void srs_amf0_do_print(SrsAmf0Any* any, stringstream& ss, int level)
{
    if (any->is_boolean()) {
        ss << "Boolean " << (any->to_boolean() ? "true" : "false") << endl;
    } else if (any->is_number()) {
        ss << "Number " << std::fixed << any->to_number() << endl;
    } else if (any->is_string()) {
        ss << "String " << any->to_str() << endl;
    } else if (any->is_date()) {
        ss << "Date " << std::hex << any->to_date()
           << "/" << std::hex << any->to_date_time_zone() << endl;
    } else if (any->is_null()) {
        ss << "Null" << endl;
    } else if (any->is_ecma_array()) {
        SrsAmf0EcmaArray* obj = any->to_ecma_array();
        ss << "EcmaArray " << "(" << obj->count() << " items)" << endl;
        for (int i = 0; i < obj->count(); i++) {
            srs_fill_level_spaces(ss, level + 1);
            ss << "Elem '" << obj->key_at(i) << "' ";
            if (obj->value_at(i)->is_complex_object()) {
                srs_amf0_do_print(obj->value_at(i), ss, level + 1);
            } else {
                srs_amf0_do_print(obj->value_at(i), ss, 0);
            }
        }
    } else if (any->is_strict_array()) {
        SrsAmf0StrictArray* obj = any->to_strict_array();
        ss << "StrictArray " << "(" << obj->count() << " items)" << endl;
        for (int i = 0; i < obj->count(); i++) {
            srs_fill_level_spaces(ss, level + 1);
            ss << "Elem ";
            if (obj->at(i)->is_complex_object()) {
                srs_amf0_do_print(obj->at(i), ss, level + 1);
            } else {
                srs_amf0_do_print(obj->at(i), ss, 0);
            }
        }
    } else if (any->is_object()) {
        SrsAmf0Object* obj = any->to_object();
        ss << "Object " << "(" << obj->count() << " items)" << endl;
        for (int i = 0; i < obj->count(); i++) {
            srs_fill_level_spaces(ss, level + 1);
            ss << "Property '" << obj->key_at(i) << "' ";
            if (obj->value_at(i)->is_complex_object()) {
                srs_amf0_do_print(obj->value_at(i), ss, level + 1);
            } else {
                srs_amf0_do_print(obj->value_at(i), ss, 0);
            }
        }
    } else {
        ss << "Unknown" << endl;
    }
} // srs_amf0_do_print

char * SrsAmf0Any::human_print(char** pdata, int* psize)
{
    stringstream ss;

    ss.precision(1);
    srs_amf0_do_print(this, ss, 0);
    string str = ss.str();
    if (str.empty()) {
        return NULL;
    }
    char* data = new char[str.length() + 1];
    memcpy(data, str.data(), str.length());
    data[str.length()] = 0;
    if (pdata) {
        *pdata = data;
    }
    if (psize) {
        *psize = str.length();
    }
    return data;
}

SrsAmf0Any * SrsAmf0Any::str(const char* value)
{
    return new SrsAmf0String(value);
}

SrsAmf0Any * SrsAmf0Any::boolean(bool value)
{
    return new SrsAmf0Boolean(value);
}

SrsAmf0Any * SrsAmf0Any::number(double value)
{
    return new SrsAmf0Number(value);
}

SrsAmf0Any * SrsAmf0Any::null()
{
    return new SrsAmf0Null();
}

SrsAmf0Any * SrsAmf0Any::undefined()
{
    return new SrsAmf0Undefined();
}

SrsAmf0Object * SrsAmf0Any::object()
{
    return new SrsAmf0Object();
}

SrsAmf0Any * SrsAmf0Any::object_eof()
{
    return new SrsAmf0ObjectEOF();
}

SrsAmf0EcmaArray * SrsAmf0Any::ecma_array()
{
    return new SrsAmf0EcmaArray();
}

SrsAmf0StrictArray * SrsAmf0Any::strict_array()
{
    return new SrsAmf0StrictArray();
}

SrsAmf0Any * SrsAmf0Any::date(int64_t value)
{
    return new SrsAmf0Date(value);
}

int SrsAmf0Any::discovery(SrsStream* stream, SrsAmf0Any** ppvalue)
{
    int ret = ERROR_SUCCESS;

    if (srs_amf0_is_object_eof(stream)) {
        *ppvalue = new SrsAmf0ObjectEOF();
        return ret;
    }
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read any marker failed. ret=%d", ret);
        return ret;
    }
    char marker = stream->read_1bytes();
    srs_verbose("amf0 any marker success");
    stream->skip(-1);
    switch (marker) {
        case RTMP_AMF0_String: {
            *ppvalue = SrsAmf0Any::str();
            return ret;
        }
        case RTMP_AMF0_Boolean: {
            *ppvalue = SrsAmf0Any::boolean();
            return ret;
        }
        case RTMP_AMF0_Number: {
            *ppvalue = SrsAmf0Any::number();
            return ret;
        }
        case RTMP_AMF0_Null: {
            *ppvalue = SrsAmf0Any::null();
            return ret;
        }
        case RTMP_AMF0_Undefined: {
            *ppvalue = SrsAmf0Any::undefined();
            return ret;
        }
        case RTMP_AMF0_Object: {
            *ppvalue = SrsAmf0Any::object();
            return ret;
        }
        case RTMP_AMF0_EcmaArray: {
            *ppvalue = SrsAmf0Any::ecma_array();
            return ret;
        }
        case RTMP_AMF0_StrictArray: {
            *ppvalue = SrsAmf0Any::strict_array();
            return ret;
        }
        case RTMP_AMF0_Date: {
            *ppvalue = SrsAmf0Any::date();
            return ret;
        }
        case RTMP_AMF0_Invalid:
        default: {
            ret = ERROR_RTMP_AMF0_INVALID;
            srs_error("invalid amf0 message type. marker=%#x, ret=%d", marker, ret);
            return ret;
        }
    }
} // SrsAmf0Any::discovery

SrsUnSortedHashtable::SrsUnSortedHashtable()
{}

SrsUnSortedHashtable::~SrsUnSortedHashtable()
{
    clear();
}

int SrsUnSortedHashtable::count()
{
    return (int) properties.size();
}

void SrsUnSortedHashtable::clear()
{
    std::vector<SrsAmf0ObjectPropertyType>::iterator it;

    for (it = properties.begin(); it != properties.end(); ++it) {
        SrsAmf0ObjectPropertyType& elem = *it;
        SrsAmf0Any* any = elem.second;
        srs_freep(any);
    }
    properties.clear();
}

string SrsUnSortedHashtable::key_at(int index)
{
    srs_assert(index < count());
    SrsAmf0ObjectPropertyType& elem = properties[index];
    return elem.first;
}

const char * SrsUnSortedHashtable::key_raw_at(int index)
{
    srs_assert(index < count());
    SrsAmf0ObjectPropertyType& elem = properties[index];
    return elem.first.data();
}

SrsAmf0Any * SrsUnSortedHashtable::value_at(int index)
{
    srs_assert(index < count());
    SrsAmf0ObjectPropertyType& elem = properties[index];
    return elem.second;
}

void SrsUnSortedHashtable::set(string key, SrsAmf0Any* value)
{
    std::vector<SrsAmf0ObjectPropertyType>::iterator it;

    for (it = properties.begin(); it != properties.end(); ++it) {
        SrsAmf0ObjectPropertyType& elem = *it;
        std::string name = elem.first;
        SrsAmf0Any* any  = elem.second;
        if (key == name) {
            srs_freep(any);
            properties.erase(it);
            break;
        }
    }
    if (value) {
        properties.push_back(std::make_pair(key, value));
    }
}

SrsAmf0Any * SrsUnSortedHashtable::get_property(string name)
{
    std::vector<SrsAmf0ObjectPropertyType>::iterator it;

    for (it = properties.begin(); it != properties.end(); ++it) {
        SrsAmf0ObjectPropertyType& elem = *it;
        std::string key = elem.first;
        SrsAmf0Any* any = elem.second;
        if (key == name) {
            return any;
        }
    }
    return NULL;
}

SrsAmf0Any * SrsUnSortedHashtable::ensure_property_string(string name)
{
    SrsAmf0Any* prop = get_property(name);

    if (!prop) {
        return NULL;
    }
    if (!prop->is_string()) {
        return NULL;
    }
    return prop;
}

SrsAmf0Any * SrsUnSortedHashtable::ensure_property_number(string name)
{
    SrsAmf0Any* prop = get_property(name);

    if (!prop) {
        return NULL;
    }
    if (!prop->is_number()) {
        return NULL;
    }
    return prop;
}

void SrsUnSortedHashtable::remove(string name)
{
    std::vector<SrsAmf0ObjectPropertyType>::iterator it;

    for (it = properties.begin(); it != properties.end();) {
        std::string key = it->first;
        SrsAmf0Any* any = it->second;
        if (key == name) {
            srs_freep(any);
            it = properties.erase(it);
        } else {
            ++it;
        }
    }
}

void SrsUnSortedHashtable::copy(SrsUnSortedHashtable* src)
{
    std::vector<SrsAmf0ObjectPropertyType>::iterator it;

    for (it = src->properties.begin(); it != src->properties.end(); ++it) {
        SrsAmf0ObjectPropertyType& elem = *it;
        std::string key = elem.first;
        SrsAmf0Any* any = elem.second;
        set(key, any->copy());
    }
}

SrsAmf0ObjectEOF::SrsAmf0ObjectEOF()
{
    marker = RTMP_AMF0_ObjectEnd;
}

SrsAmf0ObjectEOF::~SrsAmf0ObjectEOF()
{}

int SrsAmf0ObjectEOF::total_size()
{
    return SrsAmf0Size::object_eof();
}

int SrsAmf0ObjectEOF::read(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(2)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read object eof value failed. ret=%d", ret);
        return ret;
    }
    int16_t temp = stream->read_2bytes();
    if (temp != 0x00) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read object eof value check failed. "
                  "must be 0x00, actual is %#x, ret=%d", temp, ret);
        return ret;
    }
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read object eof marker failed. ret=%d", ret);
        return ret;
    }
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_ObjectEnd) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 check object eof marker failed. "
                  "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_ObjectEnd, ret);
        return ret;
    }
    srs_verbose("amf0 read object eof marker success");
    srs_verbose("amf0 read object eof success");
    return ret;
}

int SrsAmf0ObjectEOF::write(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(2)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write object eof value failed. ret=%d", ret);
        return ret;
    }
    stream->write_2bytes(0x00);
    srs_verbose("amf0 write object eof value success");
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write object eof marker failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(RTMP_AMF0_ObjectEnd);
    srs_verbose("amf0 read object eof success");
    return ret;
}

SrsAmf0Any * SrsAmf0ObjectEOF::copy()
{
    return new SrsAmf0ObjectEOF();
}

SrsAmf0Object::SrsAmf0Object()
{
    properties = new SrsUnSortedHashtable();
    eof        = new SrsAmf0ObjectEOF();
    marker     = RTMP_AMF0_Object;
}

SrsAmf0Object::~SrsAmf0Object()
{
    srs_freep(properties);
    srs_freep(eof);
}

int SrsAmf0Object::total_size()
{
    int size = 1;

    for (int i = 0; i < properties->count(); i++) {
        std::string name  = key_at(i);
        SrsAmf0Any* value = value_at(i);
        size += SrsAmf0Size::utf8(name);
        size += SrsAmf0Size::any(value);
    }
    size += SrsAmf0Size::object_eof();
    return size;
}

int SrsAmf0Object::read(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read object marker failed. ret=%d", ret);
        return ret;
    }
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Object) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 check object marker failed. "
                  "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_Object, ret);
        return ret;
    }
    srs_verbose("amf0 read object marker success");
    while (!stream->empty()) {
        if (srs_amf0_is_object_eof(stream)) {
            SrsAmf0ObjectEOF pbj_eof;
            if ((ret = pbj_eof.read(stream)) != ERROR_SUCCESS) {
                srs_error("amf0 object read eof failed. ret=%d", ret);
                return ret;
            }
            srs_info("amf0 read object EOF.");
            break;
        }
        std::string property_name;
        if ((ret = srs_amf0_read_utf8(stream, property_name)) != ERROR_SUCCESS) {
            srs_error("amf0 object read property name failed. ret=%d", ret);
            return ret;
        }
        SrsAmf0Any* property_value = NULL;
        if ((ret = srs_amf0_read_any(stream, &property_value)) != ERROR_SUCCESS) {
            srs_error("amf0 object read property_value failed. "
                      "name=%s, ret=%d", property_name.c_str(), ret);
            srs_freep(property_value);
            return ret;
        }
        this->set(property_name, property_value);
    }
    return ret;
} // SrsAmf0Object::read

int SrsAmf0Object::write(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write object marker failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(RTMP_AMF0_Object);
    srs_verbose("amf0 write object marker success");
    for (int i = 0; i < properties->count(); i++) {
        std::string name = this->key_at(i);
        SrsAmf0Any* any  = this->value_at(i);
        if ((ret = srs_amf0_write_utf8(stream, name)) != ERROR_SUCCESS) {
            srs_error("write object property name failed. ret=%d", ret);
            return ret;
        }
        if ((ret = srs_amf0_write_any(stream, any)) != ERROR_SUCCESS) {
            srs_error("write object property value failed. ret=%d", ret);
            return ret;
        }
        srs_verbose("write amf0 property success. name=%s", name.c_str());
    }
    if ((ret = eof->write(stream)) != ERROR_SUCCESS) {
        srs_error("write object eof failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("write amf0 object success.");
    return ret;
}

SrsAmf0Any * SrsAmf0Object::copy()
{
    SrsAmf0Object* copy = new SrsAmf0Object();

    copy->properties->copy(properties);
    return copy;
}

void SrsAmf0Object::clear()
{
    properties->clear();
}

int SrsAmf0Object::count()
{
    return properties->count();
}

string SrsAmf0Object::key_at(int index)
{
    return properties->key_at(index);
}

const char * SrsAmf0Object::key_raw_at(int index)
{
    return properties->key_raw_at(index);
}

SrsAmf0Any * SrsAmf0Object::value_at(int index)
{
    return properties->value_at(index);
}

void SrsAmf0Object::set(string key, SrsAmf0Any* value)
{
    properties->set(key, value);
}

SrsAmf0Any * SrsAmf0Object::get_property(string name)
{
    return properties->get_property(name);
}

SrsAmf0Any * SrsAmf0Object::ensure_property_string(string name)
{
    return properties->ensure_property_string(name);
}

SrsAmf0Any * SrsAmf0Object::ensure_property_number(string name)
{
    return properties->ensure_property_number(name);
}

void SrsAmf0Object::remove(string name)
{
    properties->remove(name);
}

SrsAmf0EcmaArray::SrsAmf0EcmaArray()
{
    _count     = 0;
    properties = new SrsUnSortedHashtable();
    eof        = new SrsAmf0ObjectEOF();
    marker     = RTMP_AMF0_EcmaArray;
}

SrsAmf0EcmaArray::~SrsAmf0EcmaArray()
{
    srs_freep(properties);
    srs_freep(eof);
}

int SrsAmf0EcmaArray::total_size()
{
    int size = 1 + 4;

    for (int i = 0; i < properties->count(); i++) {
        std::string name  = key_at(i);
        SrsAmf0Any* value = value_at(i);
        size += SrsAmf0Size::utf8(name);
        size += SrsAmf0Size::any(value);
    }
    size += SrsAmf0Size::object_eof();
    return size;
}

int SrsAmf0EcmaArray::read(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read ecma_array marker failed. ret=%d", ret);
        return ret;
    }
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_EcmaArray) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 check ecma_array marker failed. "
                  "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_EcmaArray, ret);
        return ret;
    }
    srs_verbose("amf0 read ecma_array marker success");
    if (!stream->require(4)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read ecma_array count failed. ret=%d", ret);
        return ret;
    }
    int32_t count = stream->read_4bytes();
    srs_verbose("amf0 read ecma_array count success. count=%d", count);
    this->_count = count;
    while (!stream->empty()) {
        if (srs_amf0_is_object_eof(stream)) {
            SrsAmf0ObjectEOF pbj_eof;
            if ((ret = pbj_eof.read(stream)) != ERROR_SUCCESS) {
                srs_error("amf0 ecma_array read eof failed. ret=%d", ret);
                return ret;
            }
            srs_info("amf0 read ecma_array EOF.");
            break;
        }
        std::string property_name;
        if ((ret = srs_amf0_read_utf8(stream, property_name)) != ERROR_SUCCESS) {
            srs_error("amf0 ecma_array read property name failed. ret=%d", ret);
            return ret;
        }
        SrsAmf0Any* property_value = NULL;
        if ((ret = srs_amf0_read_any(stream, &property_value)) != ERROR_SUCCESS) {
            srs_error("amf0 ecma_array read property_value failed. "
                      "name=%s, ret=%d", property_name.c_str(), ret);
            return ret;
        }
        this->set(property_name, property_value);
    }
    return ret;
} // SrsAmf0EcmaArray::read

int SrsAmf0EcmaArray::write(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write ecma_array marker failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(RTMP_AMF0_EcmaArray);
    srs_verbose("amf0 write ecma_array marker success");
    if (!stream->require(4)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write ecma_array count failed. ret=%d", ret);
        return ret;
    }
    stream->write_4bytes(this->_count);
    srs_verbose("amf0 write ecma_array count success. count=%d", _count);
    for (int i = 0; i < properties->count(); i++) {
        std::string name = this->key_at(i);
        SrsAmf0Any* any  = this->value_at(i);
        if ((ret = srs_amf0_write_utf8(stream, name)) != ERROR_SUCCESS) {
            srs_error("write ecma_array property name failed. ret=%d", ret);
            return ret;
        }
        if ((ret = srs_amf0_write_any(stream, any)) != ERROR_SUCCESS) {
            srs_error("write ecma_array property value failed. ret=%d", ret);
            return ret;
        }
        srs_verbose("write amf0 property success. name=%s", name.c_str());
    }
    if ((ret = eof->write(stream)) != ERROR_SUCCESS) {
        srs_error("write ecma_array eof failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("write ecma_array object success.");
    return ret;
} // SrsAmf0EcmaArray::write

SrsAmf0Any * SrsAmf0EcmaArray::copy()
{
    SrsAmf0EcmaArray* copy = new SrsAmf0EcmaArray();

    copy->properties->copy(properties);
    copy->_count = _count;
    return copy;
}

void SrsAmf0EcmaArray::clear()
{
    properties->clear();
}

int SrsAmf0EcmaArray::count()
{
    return properties->count();
}

string SrsAmf0EcmaArray::key_at(int index)
{
    return properties->key_at(index);
}

const char * SrsAmf0EcmaArray::key_raw_at(int index)
{
    return properties->key_raw_at(index);
}

SrsAmf0Any * SrsAmf0EcmaArray::value_at(int index)
{
    return properties->value_at(index);
}

void SrsAmf0EcmaArray::set(string key, SrsAmf0Any* value)
{
    properties->set(key, value);
}

SrsAmf0Any * SrsAmf0EcmaArray::get_property(string name)
{
    return properties->get_property(name);
}

SrsAmf0Any * SrsAmf0EcmaArray::ensure_property_string(string name)
{
    return properties->ensure_property_string(name);
}

SrsAmf0Any * SrsAmf0EcmaArray::ensure_property_number(string name)
{
    return properties->ensure_property_number(name);
}

SrsAmf0StrictArray::SrsAmf0StrictArray()
{
    marker = RTMP_AMF0_StrictArray;
    _count = 0;
}

SrsAmf0StrictArray::~SrsAmf0StrictArray()
{
    std::vector<SrsAmf0Any *>::iterator it;

    for (it = properties.begin(); it != properties.end(); ++it) {
        SrsAmf0Any* any = *it;
        srs_freep(any);
    }
    properties.clear();
}

int SrsAmf0StrictArray::total_size()
{
    int size = 1 + 4;

    for (int i = 0; i < (int) properties.size(); i++) {
        SrsAmf0Any* any = properties[i];
        size += any->total_size();
    }
    return size;
}

int SrsAmf0StrictArray::read(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read strict_array marker failed. ret=%d", ret);
        return ret;
    }
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_StrictArray) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 check strict_array marker failed. "
                  "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_StrictArray, ret);
        return ret;
    }
    srs_verbose("amf0 read strict_array marker success");
    if (!stream->require(4)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read strict_array count failed. ret=%d", ret);
        return ret;
    }
    int32_t count = stream->read_4bytes();
    srs_verbose("amf0 read strict_array count success. count=%d", count);
    this->_count = count;
    for (int i = 0; i < count && !stream->empty(); i++) {
        SrsAmf0Any* elem = NULL;
        if ((ret = srs_amf0_read_any(stream, &elem)) != ERROR_SUCCESS) {
            srs_error("amf0 strict_array read value failed. ret=%d", ret);
            return ret;
        }
        properties.push_back(elem);
    }
    return ret;
} // SrsAmf0StrictArray::read

int SrsAmf0StrictArray::write(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write strict_array marker failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(RTMP_AMF0_StrictArray);
    srs_verbose("amf0 write strict_array marker success");
    if (!stream->require(4)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write strict_array count failed. ret=%d", ret);
        return ret;
    }
    stream->write_4bytes(this->_count);
    srs_verbose("amf0 write strict_array count success. count=%d", _count);
    for (int i = 0; i < (int) properties.size(); i++) {
        SrsAmf0Any* any = properties[i];
        if ((ret = srs_amf0_write_any(stream, any)) != ERROR_SUCCESS) {
            srs_error("write strict_array property value failed. ret=%d", ret);
            return ret;
        }
        srs_verbose("write amf0 property success.");
    }
    srs_verbose("write strict_array object success.");
    return ret;
}

SrsAmf0Any * SrsAmf0StrictArray::copy()
{
    SrsAmf0StrictArray* copy = new SrsAmf0StrictArray();
    std::vector<SrsAmf0Any *>::iterator it;

    for (it = properties.begin(); it != properties.end(); ++it) {
        SrsAmf0Any* any = *it;
        copy->append(any->copy());
    }
    copy->_count = _count;
    return copy;
}

void SrsAmf0StrictArray::clear()
{
    properties.clear();
}

int SrsAmf0StrictArray::count()
{
    return properties.size();
}

SrsAmf0Any * SrsAmf0StrictArray::at(int index)
{
    srs_assert(index < (int) properties.size());
    return properties.at(index);
}

void SrsAmf0StrictArray::append(SrsAmf0Any* any)
{
    properties.push_back(any);
    _count = (int32_t) properties.size();
}

int SrsAmf0Size::utf8(string value)
{
    return 2 + value.length();
}

int SrsAmf0Size::str(string value)
{
    return 1 + SrsAmf0Size::utf8(value);
}

int SrsAmf0Size::number()
{
    return 1 + 8;
}

int SrsAmf0Size::date()
{
    return 1 + 8 + 2;
}

int SrsAmf0Size::null()
{
    return 1;
}

int SrsAmf0Size::undefined()
{
    return 1;
}

int SrsAmf0Size::boolean()
{
    return 1 + 1;
}

int SrsAmf0Size::object(SrsAmf0Object* obj)
{
    if (!obj) {
        return 0;
    }
    return obj->total_size();
}

int SrsAmf0Size::object_eof()
{
    return 2 + 1;
}

int SrsAmf0Size::ecma_array(SrsAmf0EcmaArray* arr)
{
    if (!arr) {
        return 0;
    }
    return arr->total_size();
}

int SrsAmf0Size::strict_array(SrsAmf0StrictArray* arr)
{
    if (!arr) {
        return 0;
    }
    return arr->total_size();
}

int SrsAmf0Size::any(SrsAmf0Any* o)
{
    if (!o) {
        return 0;
    }
    return o->total_size();
}

SrsAmf0String::SrsAmf0String(const char* _value)
{
    marker = RTMP_AMF0_String;
    if (_value) {
        value = _value;
    }
}

SrsAmf0String::~SrsAmf0String()
{}

int SrsAmf0String::total_size()
{
    return SrsAmf0Size::str(value);
}

int SrsAmf0String::read(SrsStream* stream)
{
    return srs_amf0_read_string(stream, value);
}

int SrsAmf0String::write(SrsStream* stream)
{
    return srs_amf0_write_string(stream, value);
}

SrsAmf0Any * SrsAmf0String::copy()
{
    SrsAmf0String* copy = new SrsAmf0String(value.c_str());

    return copy;
}

SrsAmf0Boolean::SrsAmf0Boolean(bool _value)
{
    marker = RTMP_AMF0_Boolean;
    value  = _value;
}

SrsAmf0Boolean::~SrsAmf0Boolean()
{}

int SrsAmf0Boolean::total_size()
{
    return SrsAmf0Size::boolean();
}

int SrsAmf0Boolean::read(SrsStream* stream)
{
    return srs_amf0_read_boolean(stream, value);
}

int SrsAmf0Boolean::write(SrsStream* stream)
{
    return srs_amf0_write_boolean(stream, value);
}

SrsAmf0Any * SrsAmf0Boolean::copy()
{
    SrsAmf0Boolean* copy = new SrsAmf0Boolean(value);

    return copy;
}

SrsAmf0Number::SrsAmf0Number(double _value)
{
    marker = RTMP_AMF0_Number;
    value  = _value;
}

SrsAmf0Number::~SrsAmf0Number()
{}

int SrsAmf0Number::total_size()
{
    return SrsAmf0Size::number();
}

int SrsAmf0Number::read(SrsStream* stream)
{
    return srs_amf0_read_number(stream, value);
}

int SrsAmf0Number::write(SrsStream* stream)
{
    return srs_amf0_write_number(stream, value);
}

SrsAmf0Any * SrsAmf0Number::copy()
{
    SrsAmf0Number* copy = new SrsAmf0Number(value);

    return copy;
}

SrsAmf0Date::SrsAmf0Date(int64_t value)
{
    marker      = RTMP_AMF0_Date;
    _date_value = value;
    _time_zone  = 0;
}

SrsAmf0Date::~SrsAmf0Date()
{}

int SrsAmf0Date::total_size()
{
    return SrsAmf0Size::date();
}

int SrsAmf0Date::read(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read date marker failed. ret=%d", ret);
        return ret;
    }
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Date) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 check date marker failed. "
                  "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_Date, ret);
        return ret;
    }
    srs_verbose("amf0 read date marker success");
    if (!stream->require(8)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read date failed. ret=%d", ret);
        return ret;
    }
    _date_value = stream->read_8bytes();
    srs_verbose("amf0 read date success. date=%" PRId64, _date_value);
    if (!stream->require(2)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read time zone failed. ret=%d", ret);
        return ret;
    }
    _time_zone = stream->read_2bytes();
    srs_verbose("amf0 read time zone success. zone=%d", _time_zone);
    return ret;
} // SrsAmf0Date::read

int SrsAmf0Date::write(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write date marker failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(RTMP_AMF0_Date);
    srs_verbose("amf0 write date marker success");
    if (!stream->require(8)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write date failed. ret=%d", ret);
        return ret;
    }
    stream->write_8bytes(_date_value);
    srs_verbose("amf0 write date success. date=%" PRId64, _date_value);
    if (!stream->require(2)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write time zone failed. ret=%d", ret);
        return ret;
    }
    stream->write_2bytes(_time_zone);
    srs_verbose("amf0 write time zone success. date=%d", _time_zone);
    srs_verbose("write date object success.");
    return ret;
}

SrsAmf0Any * SrsAmf0Date::copy()
{
    SrsAmf0Date* copy = new SrsAmf0Date(0);

    copy->_date_value = _date_value;
    copy->_time_zone  = _time_zone;
    return copy;
}

int64_t SrsAmf0Date::date()
{
    return _date_value;
}

int16_t SrsAmf0Date::time_zone()
{
    return _time_zone;
}

SrsAmf0Null::SrsAmf0Null()
{
    marker = RTMP_AMF0_Null;
}

SrsAmf0Null::~SrsAmf0Null()
{}

int SrsAmf0Null::total_size()
{
    return SrsAmf0Size::null();
}

int SrsAmf0Null::read(SrsStream* stream)
{
    return srs_amf0_read_null(stream);
}

int SrsAmf0Null::write(SrsStream* stream)
{
    return srs_amf0_write_null(stream);
}

SrsAmf0Any * SrsAmf0Null::copy()
{
    SrsAmf0Null* copy = new SrsAmf0Null();

    return copy;
}

SrsAmf0Undefined::SrsAmf0Undefined()
{
    marker = RTMP_AMF0_Undefined;
}

SrsAmf0Undefined::~SrsAmf0Undefined()
{}

int SrsAmf0Undefined::total_size()
{
    return SrsAmf0Size::undefined();
}

int SrsAmf0Undefined::read(SrsStream* stream)
{
    return srs_amf0_read_undefined(stream);
}

int SrsAmf0Undefined::write(SrsStream* stream)
{
    return srs_amf0_write_undefined(stream);
}

SrsAmf0Any * SrsAmf0Undefined::copy()
{
    SrsAmf0Undefined* copy = new SrsAmf0Undefined();

    return copy;
}

int srs_amf0_read_any(SrsStream* stream, SrsAmf0Any** ppvalue)
{
    int ret = ERROR_SUCCESS;

    if ((ret = SrsAmf0Any::discovery(stream, ppvalue)) != ERROR_SUCCESS) {
        srs_error("amf0 discovery any elem failed. ret=%d", ret);
        return ret;
    }
    srs_assert(*ppvalue);
    if ((ret = (*ppvalue)->read(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 parse elem failed. ret=%d", ret);
        srs_freep(*ppvalue);
        return ret;
    }
    return ret;
}

int srs_amf0_read_string(SrsStream* stream, string& value)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read string marker failed. ret=%d", ret);
        return ret;
    }
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_String) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 check string marker failed. "
                  "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_String, ret);
        return ret;
    }
    srs_verbose("amf0 read string marker success");
    return srs_amf0_read_utf8(stream, value);
}

int srs_amf0_write_string(SrsStream* stream, string value)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write string marker failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(RTMP_AMF0_String);
    srs_verbose("amf0 write string marker success");
    return srs_amf0_write_utf8(stream, value);
}

int srs_amf0_read_boolean(SrsStream* stream, bool& value)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read bool marker failed. ret=%d", ret);
        return ret;
    }
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Boolean) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 check bool marker failed. "
                  "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_Boolean, ret);
        return ret;
    }
    srs_verbose("amf0 read bool marker success");
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read bool value failed. ret=%d", ret);
        return ret;
    }
    value = (stream->read_1bytes() != 0);
    srs_verbose("amf0 read bool value success. value=%d", value);
    return ret;
}

int srs_amf0_write_boolean(SrsStream* stream, bool value)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write bool marker failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(RTMP_AMF0_Boolean);
    srs_verbose("amf0 write bool marker success");
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write bool value failed. ret=%d", ret);
        return ret;
    }
    if (value) {
        stream->write_1bytes(0x01);
    } else {
        stream->write_1bytes(0x00);
    }
    srs_verbose("amf0 write bool value success. value=%d", value);
    return ret;
}

int srs_amf0_read_number(SrsStream* stream, double& value)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read number marker failed. ret=%d", ret);
        return ret;
    }
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Number) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 check number marker failed. "
                  "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_Number, ret);
        return ret;
    }
    srs_verbose("amf0 read number marker success");
    if (!stream->require(8)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read number value failed. ret=%d", ret);
        return ret;
    }
    int64_t temp = stream->read_8bytes();
    memcpy(&value, &temp, 8);
    srs_verbose("amf0 read number value success. value=%.2f", value);
    return ret;
}

int srs_amf0_write_number(SrsStream* stream, double value)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write number marker failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(RTMP_AMF0_Number);
    srs_verbose("amf0 write number marker success");
    if (!stream->require(8)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write number value failed. ret=%d", ret);
        return ret;
    }
    int64_t temp = 0x00;
    memcpy(&temp, &value, 8);
    stream->write_8bytes(temp);
    srs_verbose("amf0 write number value success. value=%.2f", value);
    return ret;
}

int srs_amf0_read_null(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read null marker failed. ret=%d", ret);
        return ret;
    }
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Null) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 check null marker failed. "
                  "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_Null, ret);
        return ret;
    }
    srs_verbose("amf0 read null success");
    return ret;
}

int srs_amf0_write_null(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write null marker failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(RTMP_AMF0_Null);
    srs_verbose("amf0 write null marker success");
    return ret;
}

int srs_amf0_read_undefined(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (stream->empty()) {
        return ret;
    }
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read undefined marker failed. ret=%d", ret);
        return ret;
    }
    char marker = stream->read_1bytes();
    if (marker == RTMP_AMF0_Null) {
        return ret;
    }
    if (marker != RTMP_AMF0_Undefined) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 check undefined marker failed. "
                  "marker=%#x, required=%#x, ret=%d", marker, RTMP_AMF0_Undefined, ret);
        return ret;
    }
    srs_verbose("amf0 read undefined success");
    return ret;
}

int srs_amf0_write_undefined(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write undefined marker failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(RTMP_AMF0_Undefined);
    srs_verbose("amf0 write undefined marker success");
    return ret;
}

namespace _srs_internal
{
int srs_amf0_read_utf8(SrsStream* stream, string& value)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(2)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read string length failed. ret=%d", ret);
        return ret;
    }
    int16_t len = stream->read_2bytes();
    srs_verbose("amf0 read string length success. len=%d", len);
    if (len <= 0) {
        srs_verbose("amf0 read empty string. ret=%d", ret);
        return ret;
    }
    if (!stream->require(len)) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read string data failed. ret=%d", ret);
        return ret;
    }
    std::string str = stream->read_string(len);
    value = str;
    srs_verbose("amf0 read string data success. str=%s", str.c_str());
    return ret;
}

int srs_amf0_write_utf8(SrsStream* stream, string value)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(2)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write string length failed. ret=%d", ret);
        return ret;
    }
    stream->write_2bytes(value.length());
    srs_verbose("amf0 write string length success. len=%d", (int) value.length());
    if (value.length() <= 0) {
        srs_verbose("amf0 write empty string. ret=%d", ret);
        return ret;
    }
    if (!stream->require(value.length())) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write string data failed. ret=%d", ret);
        return ret;
    }
    stream->write_string(value);
    srs_verbose("amf0 write string data success. str=%s", value.c_str());
    return ret;
}

bool srs_amf0_is_object_eof(SrsStream* stream)
{
    if (stream->require(3)) {
        int32_t flag = stream->read_3bytes();
        stream->skip(-3);
        return 0x09 == flag;
    }
    return false;
}

int srs_amf0_write_object_eof(SrsStream* stream, SrsAmf0ObjectEOF* value)
{
    int ret = ERROR_SUCCESS;

    srs_assert(value != NULL);
    if (!stream->require(2)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write object eof value failed. ret=%d", ret);
        return ret;
    }
    stream->write_2bytes(0x00);
    srs_verbose("amf0 write object eof value success");
    if (!stream->require(1)) {
        ret = ERROR_RTMP_AMF0_ENCODE;
        srs_error("amf0 write object eof marker failed. ret=%d", ret);
        return ret;
    }
    stream->write_1bytes(RTMP_AMF0_ObjectEnd);
    srs_verbose("amf0 read object eof success");
    return ret;
}

int srs_amf0_write_any(SrsStream* stream, SrsAmf0Any* value)
{
    srs_assert(value != NULL);
    return value->write(stream);
}
}
ISrsBufferReader::ISrsBufferReader()
{}

ISrsBufferReader::~ISrsBufferReader()
{}

ISrsBufferWriter::ISrsBufferWriter()
{}

ISrsBufferWriter::~ISrsBufferWriter()
{}

ISrsProtocolStatistic::ISrsProtocolStatistic()
{}

ISrsProtocolStatistic::~ISrsProtocolStatistic()
{}

ISrsProtocolReader::ISrsProtocolReader()
{}

ISrsProtocolReader::~ISrsProtocolReader()
{}

ISrsProtocolWriter::ISrsProtocolWriter()
{}

ISrsProtocolWriter::~ISrsProtocolWriter()
{}

ISrsProtocolReaderWriter::ISrsProtocolReaderWriter()
{}

ISrsProtocolReaderWriter::~ISrsProtocolReaderWriter()
{}

#ifndef _WIN32
# include <unistd.h>
#endif
#include <stdlib.h>
using namespace std;
#define RTMP_AMF0_COMMAND_ON_FC_PUBLISH   "onFCPublish"
#define RTMP_AMF0_COMMAND_ON_FC_UNPUBLISH "onFCUnpublish"
#define SRS_DEFAULT_SID                   1
#define SRS_MIN_RECV_TIMEOUT_US           (int64_t) (60*1000*1000LL)
#define RTMP_FMT_TYPE0                    0
#define RTMP_FMT_TYPE1                    1
#define RTMP_FMT_TYPE2                    2
#define RTMP_FMT_TYPE3                    3
#define SRS_BW_CHECK_START_PLAY           "onSrsBandCheckStartPlayBytes"
#define SRS_BW_CHECK_STARTING_PLAY        "onSrsBandCheckStartingPlayBytes"
#define SRS_BW_CHECK_STOP_PLAY            "onSrsBandCheckStopPlayBytes"
#define SRS_BW_CHECK_STOPPED_PLAY         "onSrsBandCheckStoppedPlayBytes"
#define SRS_BW_CHECK_START_PUBLISH        "onSrsBandCheckStartPublishBytes"
#define SRS_BW_CHECK_STARTING_PUBLISH     "onSrsBandCheckStartingPublishBytes"
#define SRS_BW_CHECK_STOP_PUBLISH         "onSrsBandCheckStopPublishBytes"
#define SRS_BW_CHECK_STOPPED_PUBLISH      "onSrsBandCheckStoppedPublishBytes"
#define SRS_BW_CHECK_FINISHED             "onSrsBandCheckFinished"
#define SRS_BW_CHECK_FINAL                "finalClientPacket"
#define SRS_BW_CHECK_PLAYING              "onSrsBandCheckPlaying"
#define SRS_BW_CHECK_PUBLISHING           "onSrsBandCheckPublishing"
SrsPacket::SrsPacket()
{}

SrsPacket::~SrsPacket()
{}

int SrsPacket::encode(int& psize, char *& ppayload)
{
    int ret       = ERROR_SUCCESS;
    int size      = get_size();
    char* payload = NULL;
    SrsStream stream;

    if (size > 0) {
        payload = new char[size];
        if ((ret = stream.initialize(payload, size)) != ERROR_SUCCESS) {
            srs_error("initialize the stream failed. ret=%d", ret);
            srs_freepa(payload);
            return ret;
        }
    }
    if ((ret = encode_packet(&stream)) != ERROR_SUCCESS) {
        srs_error("encode the packet failed. ret=%d", ret);
        srs_freepa(payload);
        return ret;
    }
    psize    = size;
    ppayload = payload;
    srs_verbose("encode the packet success. size=%d", size);
    return ret;
}

int SrsPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    srs_assert(stream != NULL);
    ret = ERROR_SYSTEM_PACKET_INVALID;
    srs_error("current packet is not support to decode. ret=%d", ret);
    return ret;
}

int SrsPacket::get_prefer_cid()
{
    return 0;
}

int SrsPacket::get_message_type()
{
    return 0;
}

int SrsPacket::get_size()
{
    return 0;
}

int SrsPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    srs_assert(stream != NULL);
    ret = ERROR_SYSTEM_PACKET_INVALID;
    srs_error("current packet is not support to encode. ret=%d", ret);
    return ret;
}

SrsProtocol::AckWindowSize::AckWindowSize()
{
    ack_window_size = 0;
    acked_size      = 0;
}

SrsProtocol::SrsProtocol(ISrsProtocolReaderWriter* io)
{
    in_buffer      = new SrsFastBuffer();
    skt            = io;
    in_chunk_size  = SRS_CONSTS_RTMP_PROTOCOL_CHUNK_SIZE;
    out_chunk_size = SRS_CONSTS_RTMP_PROTOCOL_CHUNK_SIZE;
    nb_out_iovs    = SRS_CONSTS_IOVS_MAX;
    out_iovs       = (iovec *) malloc(sizeof(iovec) * nb_out_iovs);
    srs_assert(nb_out_iovs >= 2);
    warned_c0c3_cache_dry   = false;
    auto_response_when_recv = true;
    cs_cache = NULL;
    if (SRS_PERF_CHUNK_STREAM_CACHE > 0) {
        cs_cache = new SrsChunkStream *[SRS_PERF_CHUNK_STREAM_CACHE];
    }
    for (int cid = 0; cid < SRS_PERF_CHUNK_STREAM_CACHE; cid++) {
        SrsChunkStream* cs = new SrsChunkStream(cid);
        cs->header.perfer_cid = cid;
        cs_cache[cid]         = cs;
    }
}

SrsProtocol::~SrsProtocol()
{
    if (true) {
        std::map<int, SrsChunkStream *>::iterator it;
        for (it = chunk_streams.begin(); it != chunk_streams.end(); ++it) {
            SrsChunkStream* stream = it->second;
            srs_freep(stream);
        }
        chunk_streams.clear();
    }
    if (true) {
        std::vector<SrsPacket *>::iterator it;
        for (it = manual_response_queue.begin(); it != manual_response_queue.end(); ++it) {
            SrsPacket* pkt = *it;
            srs_freep(pkt);
        }
        manual_response_queue.clear();
    }
    srs_freep(in_buffer);
    if (out_iovs) {
        free(out_iovs);
        out_iovs = NULL;
    }
    for (int i = 0; i < SRS_PERF_CHUNK_STREAM_CACHE; i++) {
        SrsChunkStream* cs = cs_cache[i];
        srs_freep(cs);
    }
    srs_freepa(cs_cache);
}

void SrsProtocol::set_auto_response(bool v)
{
    auto_response_when_recv = v;
}

int SrsProtocol::manual_response_flush()
{
    int ret = ERROR_SUCCESS;

    if (manual_response_queue.empty()) {
        return ret;
    }
    std::vector<SrsPacket *>::iterator it;
    for (it = manual_response_queue.begin(); it != manual_response_queue.end();) {
        SrsPacket* pkt = *it;
        it = manual_response_queue.erase(it);
        if ((ret = do_send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            return ret;
        }
    }
    return ret;
}

#ifdef SRS_PERF_MERGED_READ
void SrsProtocol::set_merge_read(bool v, IMergeReadHandler* handler)
{
    in_buffer->set_merge_read(v, handler);
}

void SrsProtocol::set_recv_buffer(int buffer_size)
{
    in_buffer->set_buffer(buffer_size);
}

#endif // ifdef SRS_PERF_MERGED_READ
void SrsProtocol::set_recv_timeout(int64_t timeout_us)
{
    return skt->set_recv_timeout(timeout_us);
}

int64_t SrsProtocol::get_recv_timeout()
{
    return skt->get_recv_timeout();
}

void SrsProtocol::set_send_timeout(int64_t timeout_us)
{
    return skt->set_send_timeout(timeout_us);
}

int64_t SrsProtocol::get_send_timeout()
{
    return skt->get_send_timeout();
}

int64_t SrsProtocol::get_recv_bytes()
{
    return skt->get_recv_bytes();
}

int64_t SrsProtocol::get_send_bytes()
{
    return skt->get_send_bytes();
}

int SrsProtocol::recv_message(SrsCommonMessage** pmsg)
{
    *pmsg = NULL;
    int ret = ERROR_SUCCESS;
    while (true) {
        SrsCommonMessage* msg = NULL;
        if ((ret = recv_interlaced_message(&msg)) != ERROR_SUCCESS) {
            if (ret != ERROR_SOCKET_TIMEOUT && !srs_is_client_gracefully_close(ret)) {
                srs_error("recv interlaced message failed. ret=%d", ret);
            }
            srs_freep(msg);
            return ret;
        }
        srs_verbose("entire msg received");
        if (!msg) {
            srs_info("got empty message without error.");
            continue;
        }
        if (msg->size <= 0 || msg->header.payload_length <= 0) {
            srs_trace("ignore empty message(type=%d, size=%d, time=%" PRId64 ", sid=%d).",
                      msg->header.message_type, msg->header.payload_length,
                      msg->header.timestamp, msg->header.stream_id);
            srs_freep(msg);
            continue;
        }
        if ((ret = on_recv_message(msg)) != ERROR_SUCCESS) {
            srs_error("hook the received msg failed. ret=%d", ret);
            srs_freep(msg);
            return ret;
        }
        srs_verbose("got a msg, cid=%d, type=%d, size=%d, time=%" PRId64,
                    msg->header.perfer_cid, msg->header.message_type, msg->header.payload_length,
                    msg->header.timestamp);
        *pmsg = msg;
        break;
    }
    return ret;
} // SrsProtocol::recv_message

int SrsProtocol::decode_message(SrsCommonMessage* msg, SrsPacket** ppacket)
{
    *ppacket = NULL;
    int ret = ERROR_SUCCESS;
    srs_assert(msg != NULL);
    srs_assert(msg->payload != NULL);
    srs_assert(msg->size > 0);
    SrsStream stream;
    if ((ret = stream.initialize(msg->payload, msg->size)) != ERROR_SUCCESS) {
        srs_error("initialize stream failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("decode stream initialized success");
    SrsPacket* packet = NULL;
    if ((ret = do_decode_message(msg->header, &stream, &packet)) != ERROR_SUCCESS) {
        srs_freep(packet);
        return ret;
    }
    *ppacket = packet;
    return ret;
}

int SrsProtocol::do_send_messages(SrsSharedPtrMessage** msgs, int nb_msgs)
{
    int ret = ERROR_SUCCESS;

    #ifdef SRS_PERF_COMPLEX_SEND
    int iov_index        = 0;
    iovec* iovs          = out_iovs + iov_index;
    int c0c3_cache_index = 0;
    char* c0c3_cache     = out_c0c3_caches + c0c3_cache_index;
    for (int i = 0; i < nb_msgs; i++) {
        SrsSharedPtrMessage* msg = msgs[i];
        if (!msg) {
            continue;
        }
        if (!msg->payload || msg->size <= 0) {
            srs_info("ignore empty message.");
            continue;
        }
        char* p    = msg->payload;
        char* pend = msg->payload + msg->size;
        while (p < pend) {
            int nb_cache = SRS_CONSTS_C0C3_HEADERS_MAX - c0c3_cache_index;
            int nbh      = msg->chunk_header(c0c3_cache, nb_cache, p == msg->payload);
            srs_assert(nbh > 0);
            iovs[0].iov_base = c0c3_cache;
            iovs[0].iov_len  = nbh;
            int payload_size = srs_min(out_chunk_size, (int) (pend - p));
            iovs[1].iov_base = p;
            iovs[1].iov_len  = payload_size;
            p += payload_size;
            if (iov_index >= nb_out_iovs - 2) {
                srs_warn("resize iovs %d => %d, max_msgs=%d",
                         nb_out_iovs, nb_out_iovs + SRS_CONSTS_IOVS_MAX,
                         SRS_PERF_MW_MSGS);
                nb_out_iovs += SRS_CONSTS_IOVS_MAX;
                int realloc_size = sizeof(iovec) * nb_out_iovs;
                out_iovs = (iovec *) realloc(out_iovs, realloc_size);
            }
            iov_index        += 2;
            iovs              = out_iovs + iov_index;
            c0c3_cache_index += nbh;
            c0c3_cache        = out_c0c3_caches + c0c3_cache_index;
            int c0c3_left = SRS_CONSTS_C0C3_HEADERS_MAX - c0c3_cache_index;
            if (c0c3_left < SRS_CONSTS_RTMP_MAX_FMT0_HEADER_SIZE) {
                if (!warned_c0c3_cache_dry) {
                    srs_warn("c0c3 cache header too small, recoment to %d",
                             SRS_CONSTS_C0C3_HEADERS_MAX + SRS_CONSTS_RTMP_MAX_FMT0_HEADER_SIZE);
                    warned_c0c3_cache_dry = true;
                }
                if ((ret = do_iovs_send(out_iovs, iov_index)) != ERROR_SUCCESS) {
                    return ret;
                }
                iov_index        = 0;
                iovs             = out_iovs + iov_index;
                c0c3_cache_index = 0;
                c0c3_cache       = out_c0c3_caches + c0c3_cache_index;
            }
        }
    }
    if (iov_index <= 0) {
        return ret;
    }
    srs_info("mw %d msgs in %d iovs, max_msgs=%d, nb_out_iovs=%d",
             nb_msgs, iov_index, SRS_PERF_MW_MSGS, nb_out_iovs);
    return do_iovs_send(out_iovs, iov_index);

    #else // ifdef SRS_PERF_COMPLEX_SEND
    for (int i = 0; i < nb_msgs; i++) {
        SrsSharedPtrMessage* msg = msgs[i];
        if (!msg) {
            continue;
        }
        if (!msg->payload || msg->size <= 0) {
            srs_info("ignore empty message.");
            continue;
        }
        char* p    = msg->payload;
        char* pend = msg->payload + msg->size;
        while (p < pend) {
            iovec* iovs      = out_iovs;
            char* c0c3_cache = out_c0c3_caches;
            int nb_cache     = SRS_CONSTS_C0C3_HEADERS_MAX;
            int nbh = msg->chunk_header(c0c3_cache, nb_cache, p == msg->payload);
            srs_assert(nbh > 0);
            iovs[0].iov_base = c0c3_cache;
            iovs[0].iov_len  = nbh;
            int payload_size = srs_min(out_chunk_size, pend - p);
            iovs[1].iov_base = p;
            iovs[1].iov_len  = payload_size;
            p += payload_size;
            if ((ret = skt->writev(iovs, 2, NULL)) != ERROR_SUCCESS) {
                if (!srs_is_client_gracefully_close(ret)) {
                    srs_error("send packet with writev failed. ret=%d", ret);
                }
                return ret;
            }
        }
    }
    return ret;

    #endif // ifdef SRS_PERF_COMPLEX_SEND
} // SrsProtocol::do_send_messages

int SrsProtocol::do_iovs_send(iovec* iovs, int size)
{
    return srs_write_large_iovs(skt, iovs, size);
}

int SrsProtocol::do_send_and_free_packet(SrsPacket* packet, int stream_id)
{
    int ret = ERROR_SUCCESS;

    srs_assert(packet);
    SrsAutoFree(SrsPacket, packet);
    int size      = 0;
    char* payload = NULL;
    if ((ret = packet->encode(size, payload)) != ERROR_SUCCESS) {
        srs_error("encode RTMP packet to bytes oriented RTMP message failed. ret=%d", ret);
        return ret;
    }
    if (size <= 0 || payload == NULL) {
        srs_warn("packet is empty, ignore empty message.");
        return ret;
    }
    SrsMessageHeader header;
    header.payload_length = size;
    header.message_type   = packet->get_message_type();
    header.stream_id      = stream_id;
    header.perfer_cid     = packet->get_prefer_cid();
    ret = do_simple_send(&header, payload, size);
    srs_freepa(payload);
    if (ret == ERROR_SUCCESS) {
        ret = on_send_packet(&header, packet);
    }
    return ret;
}

int SrsProtocol::do_simple_send(SrsMessageHeader* mh, char* payload, int size)
{
    int ret   = ERROR_SUCCESS;
    char* p   = payload;
    char* end = p + size;
    char c0c3[SRS_CONSTS_RTMP_MAX_FMT0_HEADER_SIZE];

    while (p < end) {
        int nbh = 0;
        if (p == payload) {
            nbh = srs_chunk_header_c0(
                mh->perfer_cid, mh->timestamp, mh->payload_length,
                mh->message_type, mh->stream_id,
                c0c3, sizeof(c0c3));
        } else {
            nbh = srs_chunk_header_c3(
                mh->perfer_cid, mh->timestamp,
                c0c3, sizeof(c0c3));
        }
        srs_assert(nbh > 0);
        ;
        iovec iovs[2];
        iovs[0].iov_base = c0c3;
        iovs[0].iov_len  = nbh;
        int payload_size = srs_min(end - p, out_chunk_size);
        iovs[1].iov_base = p;
        iovs[1].iov_len  = payload_size;
        p += payload_size;
        if ((ret = skt->writev(iovs, 2, NULL)) != ERROR_SUCCESS) {
            if (!srs_is_client_gracefully_close(ret)) {
                srs_error("send packet with writev failed. ret=%d", ret);
            }
            return ret;
        }
    }
    return ret;
} // SrsProtocol::do_simple_send

int SrsProtocol::do_decode_message(SrsMessageHeader& header, SrsStream* stream, SrsPacket** ppacket)
{
    int ret = ERROR_SUCCESS;
    SrsPacket* packet = NULL;

    if (header.is_amf0_command() || header.is_amf3_command() || header.is_amf0_data() || header.is_amf3_data()) {
        srs_verbose("start to decode AMF0/AMF3 command message.");
        if (header.is_amf3_command() && stream->require(1)) {
            srs_verbose("skip 1bytes to decode AMF3 command");
            stream->skip(1);
        }
        std::string command;
        if ((ret = srs_amf0_read_string(stream, command)) != ERROR_SUCCESS) {
            srs_error("decode AMF0/AMF3 command name failed. ret=%d", ret);
            return ret;
        }
        srs_verbose("AMF0/AMF3 command message, command_name=%s", command.c_str());
        if (command == RTMP_AMF0_COMMAND_RESULT || command == RTMP_AMF0_COMMAND_ERROR) {
            double transactionId = 0.0;
            if ((ret = srs_amf0_read_number(stream, transactionId)) != ERROR_SUCCESS) {
                srs_error("decode AMF0/AMF3 transcationId failed. ret=%d", ret);
                return ret;
            }
            srs_verbose("AMF0/AMF3 command id, transcationId=%.2f", transactionId);
            stream->skip(-1 * stream->pos());
            if (header.is_amf3_command()) {
                stream->skip(1);
            }
            if (requests.find(transactionId) == requests.end()) {
                ret = ERROR_RTMP_NO_REQUEST;
                srs_error("decode AMF0/AMF3 request failed. ret=%d", ret);
                return ret;
            }
            std::string request_name = requests[transactionId];
            srs_verbose("AMF0/AMF3 request parsed. request_name=%s", request_name.c_str());
            if (request_name == RTMP_AMF0_COMMAND_CONNECT) {
                srs_info("decode the AMF0/AMF3 response command(%s message).", request_name.c_str());
                *ppacket = packet = new SrsConnectAppResPacket();
                return packet->decode(stream);
            } else if (request_name == RTMP_AMF0_COMMAND_CREATE_STREAM) {
                srs_info("decode the AMF0/AMF3 response command(%s message).", request_name.c_str());
                *ppacket = packet = new SrsCreateStreamResPacket(0, 0);
                return packet->decode(stream);
            } else if (request_name == RTMP_AMF0_COMMAND_RELEASE_STREAM ||
                       request_name == RTMP_AMF0_COMMAND_FC_PUBLISH ||
                       request_name == RTMP_AMF0_COMMAND_UNPUBLISH)
            {
                srs_info("decode the AMF0/AMF3 response command(%s message).", request_name.c_str());
                *ppacket = packet = new SrsFMLEStartResPacket(0);
                return packet->decode(stream);
            } else {
                ret = ERROR_RTMP_NO_REQUEST;
                srs_error("decode AMF0/AMF3 request failed. "
                          "request_name=%s, transactionId=%.2f, ret=%d",
                          request_name.c_str(), transactionId, ret);
                return ret;
            }
        }
        stream->skip(-1 * stream->pos());
        if (header.is_amf3_command()) {
            stream->skip(1);
        }
        if (command == RTMP_AMF0_COMMAND_CONNECT) {
            srs_info("decode the AMF0/AMF3 command(connect vhost/app message).");
            *ppacket = packet = new SrsConnectAppPacket();
            return packet->decode(stream);
        } else if (command == RTMP_AMF0_COMMAND_CREATE_STREAM) {
            srs_info("decode the AMF0/AMF3 command(createStream message).");
            *ppacket = packet = new SrsCreateStreamPacket();
            return packet->decode(stream);
        } else if (command == RTMP_AMF0_COMMAND_PLAY) {
            srs_info("decode the AMF0/AMF3 command(paly message).");
            *ppacket = packet = new SrsPlayPacket();
            return packet->decode(stream);
        } else if (command == RTMP_AMF0_COMMAND_PAUSE) {
            srs_info("decode the AMF0/AMF3 command(pause message).");
            *ppacket = packet = new SrsPausePacket();
            return packet->decode(stream);
        } else if (command == RTMP_AMF0_COMMAND_RELEASE_STREAM) {
            srs_info("decode the AMF0/AMF3 command(FMLE releaseStream message).");
            *ppacket = packet = new SrsFMLEStartPacket();
            return packet->decode(stream);
        } else if (command == RTMP_AMF0_COMMAND_FC_PUBLISH) {
            srs_info("decode the AMF0/AMF3 command(FMLE FCPublish message).");
            *ppacket = packet = new SrsFMLEStartPacket();
            return packet->decode(stream);
        } else if (command == RTMP_AMF0_COMMAND_PUBLISH) {
            srs_info("decode the AMF0/AMF3 command(publish message).");
            *ppacket = packet = new SrsPublishPacket();
            return packet->decode(stream);
        } else if (command == RTMP_AMF0_COMMAND_UNPUBLISH) {
            srs_info("decode the AMF0/AMF3 command(unpublish message).");
            *ppacket = packet = new SrsFMLEStartPacket();
            return packet->decode(stream);
        } else if (command == SRS_CONSTS_RTMP_SET_DATAFRAME || command == SRS_CONSTS_RTMP_ON_METADATA) {
            srs_info("decode the AMF0/AMF3 data(onMetaData message).");
            *ppacket = packet = new SrsOnMetaDataPacket();
            return packet->decode(stream);
        } else if (command == SRS_BW_CHECK_FINISHED ||
                   command == SRS_BW_CHECK_PLAYING ||
                   command == SRS_BW_CHECK_PUBLISHING ||
                   command == SRS_BW_CHECK_STARTING_PLAY ||
                   command == SRS_BW_CHECK_STARTING_PUBLISH ||
                   command == SRS_BW_CHECK_START_PLAY ||
                   command == SRS_BW_CHECK_START_PUBLISH ||
                   command == SRS_BW_CHECK_STOPPED_PLAY ||
                   command == SRS_BW_CHECK_STOP_PLAY ||
                   command == SRS_BW_CHECK_STOP_PUBLISH ||
                   command == SRS_BW_CHECK_STOPPED_PUBLISH ||
                   command == SRS_BW_CHECK_FINAL)
        {
            srs_info("decode the AMF0/AMF3 band width check message.");
            *ppacket = packet = new SrsBandwidthPacket();
            return packet->decode(stream);
        } else if (command == RTMP_AMF0_COMMAND_CLOSE_STREAM) {
            srs_info("decode the AMF0/AMF3 closeStream message.");
            *ppacket = packet = new SrsCloseStreamPacket();
            return packet->decode(stream);
        } else if (header.is_amf0_command() || header.is_amf3_command()) {
            srs_info("decode the AMF0/AMF3 call message.");
            *ppacket = packet = new SrsCallPacket();
            return packet->decode(stream);
        }
        srs_info("drop the AMF0/AMF3 command message, command_name=%s", command.c_str());
        *ppacket = packet = new SrsPacket();
        return ret;
    } else if (header.is_user_control_message()) {
        srs_verbose("start to decode user control message.");
        *ppacket = packet = new SrsUserControlPacket();
        return packet->decode(stream);
    } else if (header.is_window_ackledgement_size()) {
        srs_verbose("start to decode set ack window size message.");
        *ppacket = packet = new SrsSetWindowAckSizePacket();
        return packet->decode(stream);
    } else if (header.is_set_chunk_size()) {
        srs_verbose("start to decode set chunk size message.");
        *ppacket = packet = new SrsSetChunkSizePacket();
        return packet->decode(stream);
    } else {
        if (!header.is_set_peer_bandwidth() && !header.is_ackledgement()) {
            srs_trace("drop unknown message, type=%d", header.message_type);
        }
    }
    return ret;
} // SrsProtocol::do_decode_message

int SrsProtocol::send_and_free_message(SrsSharedPtrMessage* msg, int stream_id)
{
    return send_and_free_messages(&msg, 1, stream_id);
}

int SrsProtocol::send_and_free_messages(SrsSharedPtrMessage** msgs, int nb_msgs, int stream_id)
{
    srs_assert(msgs);
    srs_assert(nb_msgs > 0);
    for (int i = 0; i < nb_msgs; i++) {
        SrsSharedPtrMessage* msg = msgs[i];
        if (!msg) {
            continue;
        }
        if (msg->check(stream_id)) {
            break;
        }
    }
    int ret = do_send_messages(msgs, nb_msgs);
    for (int i = 0; i < nb_msgs; i++) {
        SrsSharedPtrMessage* msg = msgs[i];
        srs_freep(msg);
    }
    if (ret != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = manual_response_flush()) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int SrsProtocol::send_and_free_packet(SrsPacket* packet, int stream_id)
{
    int ret = ERROR_SUCCESS;

    if ((ret = do_send_and_free_packet(packet, stream_id)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = manual_response_flush()) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int SrsProtocol::recv_interlaced_message(SrsCommonMessage** pmsg)
{
    int ret  = ERROR_SUCCESS;
    char fmt = 0;
    int cid  = 0;

    if ((ret = read_basic_header(fmt, cid)) != ERROR_SUCCESS) {
        if (ret != ERROR_SOCKET_TIMEOUT && !srs_is_client_gracefully_close(ret)) {
            srs_error("read basic header failed. ret=%d", ret);
        }
        return ret;
    }
    srs_verbose("read basic header success. fmt=%d, cid=%d", fmt, cid);
    srs_assert(cid >= 0);
    SrsChunkStream* chunk = NULL;
    if (cid < SRS_PERF_CHUNK_STREAM_CACHE) {
        srs_verbose("cs-cache hit, cid=%d", cid);
        chunk = cs_cache[cid];
        srs_verbose("cached chunk stream: fmt=%d, cid=%d, size=%d, message(type=%d, size=%d, time=%" PRId64 ", sid=%d)",
                    chunk->fmt, chunk->cid, (chunk->msg ? chunk->msg->size : 0), chunk->header.message_type,
                    chunk->header.payload_length,
                    chunk->header.timestamp, chunk->header.stream_id);
    } else {
        if (chunk_streams.find(cid) == chunk_streams.end()) {
            chunk = chunk_streams[cid] = new SrsChunkStream(cid);
            chunk->header.perfer_cid = cid;
            srs_verbose("cache new chunk stream: fmt=%d, cid=%d", fmt, cid);
        } else {
            chunk = chunk_streams[cid];
            srs_verbose(
                "cached chunk stream: fmt=%d, cid=%d, size=%d, message(type=%d, size=%d, time=%" PRId64 ", sid=%d)",
                chunk->fmt, chunk->cid, (chunk->msg ? chunk->msg->size : 0), chunk->header.message_type,
                chunk->header.payload_length,
                chunk->header.timestamp, chunk->header.stream_id);
        }
    }
    if ((ret = read_message_header(chunk, fmt)) != ERROR_SUCCESS) {
        if (ret != ERROR_SOCKET_TIMEOUT && !srs_is_client_gracefully_close(ret)) {
            srs_error("read message header failed. ret=%d", ret);
        }
        return ret;
    }
    srs_verbose("read message header success. "
                "fmt=%d, ext_time=%d, size=%d, message(type=%d, size=%d, time=%" PRId64 ", sid=%d)",
                fmt, chunk->extended_timestamp, (chunk->msg ? chunk->msg->size : 0), chunk->header.message_type,
                chunk->header.payload_length, chunk->header.timestamp, chunk->header.stream_id);
    SrsCommonMessage* msg = NULL;
    if ((ret = read_message_payload(chunk, &msg)) != ERROR_SUCCESS) {
        if (ret != ERROR_SOCKET_TIMEOUT && !srs_is_client_gracefully_close(ret)) {
            srs_error("read message payload failed. ret=%d", ret);
        }
        return ret;
    }
    if (!msg) {
        srs_verbose("get partial message success. size=%d, message(type=%d, size=%d, time=%" PRId64 ", sid=%d)",
                    (msg ? msg->size : (chunk->msg ? chunk->msg->size : 0)), chunk->header.message_type,
                    chunk->header.payload_length,
                    chunk->header.timestamp, chunk->header.stream_id);
        return ret;
    }
    *pmsg = msg;
    srs_info("get entire message success. size=%d, message(type=%d, size=%d, time=%" PRId64 ", sid=%d)",
             (msg ? msg->size : (chunk->msg ? chunk->msg->size : 0)), chunk->header.message_type,
             chunk->header.payload_length,
             chunk->header.timestamp, chunk->header.stream_id);
    return ret;
} // SrsProtocol::recv_interlaced_message

int SrsProtocol::read_basic_header(char& fmt, int& cid)
{
    int ret = ERROR_SUCCESS;

    if ((ret = in_buffer->grow(skt, 1)) != ERROR_SUCCESS) {
        if (ret != ERROR_SOCKET_TIMEOUT && !srs_is_client_gracefully_close(ret)) {
            srs_error("read 1bytes basic header failed. required_size=%d, ret=%d", 1, ret);
        }
        return ret;
    }
    fmt = in_buffer->read_1byte();
    cid = fmt & 0x3f;
    fmt = (fmt >> 6) & 0x03;
    if (cid > 1) {
        srs_verbose("basic header parsed. fmt=%d, cid=%d", fmt, cid);
        return ret;
    }
    if (cid == 0) {
        if ((ret = in_buffer->grow(skt, 1)) != ERROR_SUCCESS) {
            if (ret != ERROR_SOCKET_TIMEOUT && !srs_is_client_gracefully_close(ret)) {
                srs_error("read 2bytes basic header failed. required_size=%d, ret=%d", 1, ret);
            }
            return ret;
        }
        cid  = 64;
        cid += (u_int8_t) in_buffer->read_1byte();
        srs_verbose("2bytes basic header parsed. fmt=%d, cid=%d", fmt, cid);
    } else if (cid == 1) {
        if ((ret = in_buffer->grow(skt, 2)) != ERROR_SUCCESS) {
            if (ret != ERROR_SOCKET_TIMEOUT && !srs_is_client_gracefully_close(ret)) {
                srs_error("read 3bytes basic header failed. required_size=%d, ret=%d", 2, ret);
            }
            return ret;
        }
        cid  = 64;
        cid += (u_int8_t) in_buffer->read_1byte();
        cid += ((u_int8_t) in_buffer->read_1byte()) * 256;
        srs_verbose("3bytes basic header parsed. fmt=%d, cid=%d", fmt, cid);
    } else {
        srs_error("invalid path, impossible basic header.");
        srs_assert(false);
    }
    return ret;
} // SrsProtocol::read_basic_header

int SrsProtocol::read_message_header(SrsChunkStream* chunk, char fmt)
{
    int ret = ERROR_SUCCESS;
    bool is_first_chunk_of_msg = !chunk->msg;

    if (chunk->msg_count == 0 && fmt != RTMP_FMT_TYPE0) {
        if (chunk->cid == RTMP_CID_ProtocolControl && fmt == RTMP_FMT_TYPE1) {
            srs_warn("accept cid=2, fmt=1 to make librtmp happy.");
        } else {
            ret = ERROR_RTMP_CHUNK_START;
            srs_error("chunk stream is fresh, fmt must be %d, actual is %d. cid=%d, ret=%d",
                      RTMP_FMT_TYPE0, fmt, chunk->cid, ret);
            return ret;
        }
    }
    if (chunk->msg && fmt == RTMP_FMT_TYPE0) {
        ret = ERROR_RTMP_CHUNK_START;
        srs_error("chunk stream exists, "
                  "fmt must not be %d, actual is %d. ret=%d", RTMP_FMT_TYPE0, fmt, ret);
        return ret;
    }
    if (!chunk->msg) {
        chunk->msg = new SrsCommonMessage();
        srs_verbose("create message for new chunk, fmt=%d, cid=%d", fmt, chunk->cid);
    }
    static char mh_sizes[] = { 11, 7, 3, 0 };
    int mh_size = mh_sizes[(int) fmt];
    srs_verbose("calc chunk message header size. fmt=%d, mh_size=%d", fmt, mh_size);
    if (mh_size > 0 && (ret = in_buffer->grow(skt, mh_size)) != ERROR_SUCCESS) {
        if (ret != ERROR_SOCKET_TIMEOUT && !srs_is_client_gracefully_close(ret)) {
            srs_error("read %dbytes message header failed. ret=%d", mh_size, ret);
        }
        return ret;
    }
    if (fmt <= RTMP_FMT_TYPE2) {
        char* p  = in_buffer->read_slice(mh_size);
        char* pp = (char *) &chunk->header.timestamp_delta;
        pp[2] = *p++;
        pp[1] = *p++;
        pp[0] = *p++;
        pp[3] = 0;
        chunk->extended_timestamp = (chunk->header.timestamp_delta >= RTMP_EXTENDED_TIMESTAMP);
        if (!chunk->extended_timestamp) {
            if (fmt == RTMP_FMT_TYPE0) {
                chunk->header.timestamp = chunk->header.timestamp_delta;
            } else {
                chunk->header.timestamp += chunk->header.timestamp_delta;
            }
        }
        if (fmt <= RTMP_FMT_TYPE1) {
            int32_t payload_length = 0;
            pp    = (char *) &payload_length;
            pp[2] = *p++;
            pp[1] = *p++;
            pp[0] = *p++;
            pp[3] = 0;
            if (!is_first_chunk_of_msg && chunk->header.payload_length != payload_length) {
                ret = ERROR_RTMP_PACKET_SIZE;
                srs_error("msg exists in chunk cache, "
                          "size=%d cannot change to %d, ret=%d",
                          chunk->header.payload_length, payload_length, ret);
                return ret;
            }
            chunk->header.payload_length = payload_length;
            chunk->header.message_type   = *p++;
            if (fmt == RTMP_FMT_TYPE0) {
                pp    = (char *) &chunk->header.stream_id;
                pp[0] = *p++;
                pp[1] = *p++;
                pp[2] = *p++;
                pp[3] = *p++;
                srs_verbose(
                    "header read completed. fmt=%d, mh_size=%d, ext_time=%d, time=%" PRId64 ", payload=%d, type=%d, sid=%d",
                    fmt, mh_size, chunk->extended_timestamp, chunk->header.timestamp, chunk->header.payload_length,
                    chunk->header.message_type, chunk->header.stream_id);
            } else {
                srs_verbose(
                    "header read completed. fmt=%d, mh_size=%d, ext_time=%d, time=%" PRId64 ", payload=%d, type=%d",
                    fmt, mh_size, chunk->extended_timestamp, chunk->header.timestamp,
                    chunk->header.payload_length,
                    chunk->header.message_type);
            }
        } else {
            srs_verbose("header read completed. fmt=%d, mh_size=%d, ext_time=%d, time=%" PRId64 "",
                        fmt, mh_size, chunk->extended_timestamp, chunk->header.timestamp);
        }
    } else {
        if (is_first_chunk_of_msg && !chunk->extended_timestamp) {
            chunk->header.timestamp += chunk->header.timestamp_delta;
        }
        srs_verbose("header read completed. fmt=%d, size=%d, ext_time=%d",
                    fmt, mh_size, chunk->extended_timestamp);
    }
    if (chunk->extended_timestamp) {
        mh_size += 4;
        srs_verbose("read header ext time. fmt=%d, ext_time=%d, mh_size=%d", fmt, chunk->extended_timestamp, mh_size);
        if ((ret = in_buffer->grow(skt, 4)) != ERROR_SUCCESS) {
            if (ret != ERROR_SOCKET_TIMEOUT && !srs_is_client_gracefully_close(ret)) {
                srs_error("read %dbytes message header failed. required_size=%d, ret=%d", mh_size, 4, ret);
            }
            return ret;
        }
        char* p = in_buffer->read_slice(4);
        u_int32_t timestamp = 0x00;
        char* pp = (char *) &timestamp;
        pp[3]      = *p++;
        pp[2]      = *p++;
        pp[1]      = *p++;
        pp[0]      = *p++;
        timestamp &= 0x7fffffff;
        u_int32_t chunk_timestamp = (u_int32_t) chunk->header.timestamp;
        if (!is_first_chunk_of_msg && chunk_timestamp > 0 && chunk_timestamp != timestamp) {
            mh_size -= 4;
            in_buffer->skip(-4);
            srs_info("no 4bytes extended timestamp in the continued chunk");
        } else {
            chunk->header.timestamp = timestamp;
        }
        srs_verbose("header read ext_time completed. time=%" PRId64 "", chunk->header.timestamp);
    }
    chunk->header.timestamp &= 0x7fffffff;
    srs_assert(chunk->header.payload_length >= 0);
    chunk->msg->header = chunk->header;
    chunk->msg_count++;
    return ret;
} // SrsProtocol::read_message_header

int SrsProtocol::read_message_payload(SrsChunkStream* chunk, SrsCommonMessage** pmsg)
{
    int ret = ERROR_SUCCESS;

    if (chunk->header.payload_length <= 0) {
        srs_trace("get an empty RTMP "
                  "message(type=%d, size=%d, time=%" PRId64 ", sid=%d)", chunk->header.message_type,
                  chunk->header.payload_length, chunk->header.timestamp, chunk->header.stream_id);
        *pmsg      = chunk->msg;
        chunk->msg = NULL;
        return ret;
    }
    srs_assert(chunk->header.payload_length > 0);
    int payload_size = chunk->header.payload_length - chunk->msg->size;
    payload_size = srs_min(payload_size, in_chunk_size);
    srs_verbose("chunk payload size is %d, message_size=%d, received_size=%d, in_chunk_size=%d",
                payload_size, chunk->header.payload_length, chunk->msg->size, in_chunk_size);
    if (!chunk->msg->payload) {
        chunk->msg->create_payload(chunk->header.payload_length);
    }
    if ((ret = in_buffer->grow(skt, payload_size)) != ERROR_SUCCESS) {
        if (ret != ERROR_SOCKET_TIMEOUT && !srs_is_client_gracefully_close(ret)) {
            srs_error("read payload failed. required_size=%d, ret=%d", payload_size, ret);
        }
        return ret;
    }
    memcpy(chunk->msg->payload + chunk->msg->size, in_buffer->read_slice(payload_size), payload_size);
    chunk->msg->size += payload_size;
    srs_verbose("chunk payload read completed. payload_size=%d", payload_size);
    if (chunk->header.payload_length == chunk->msg->size) {
        *pmsg      = chunk->msg;
        chunk->msg = NULL;
        srs_verbose("get entire RTMP message(type=%d, size=%d, time=%" PRId64 ", sid=%d)",
                    chunk->header.message_type, chunk->header.payload_length,
                    chunk->header.timestamp, chunk->header.stream_id);
        return ret;
    }
    srs_verbose("get partial RTMP message(type=%d, size=%d, time=%" PRId64 ", sid=%d), partial size=%d",
                chunk->header.message_type, chunk->header.payload_length,
                chunk->header.timestamp, chunk->header.stream_id,
                chunk->msg->size);
    return ret;
} // SrsProtocol::read_message_payload

int SrsProtocol::on_recv_message(SrsCommonMessage* msg)
{
    int ret = ERROR_SUCCESS;

    srs_assert(msg != NULL);
    if (in_ack_size.ack_window_size > 0 &&
        skt->get_recv_bytes() - in_ack_size.acked_size > in_ack_size.ack_window_size
    )
    {
        if ((ret = response_acknowledgement_message()) != ERROR_SUCCESS) {
            return ret;
        }
    }
    SrsPacket* packet = NULL;
    switch (msg->header.message_type) {
        case RTMP_MSG_SetChunkSize:
        case RTMP_MSG_UserControlMessage:
        case RTMP_MSG_WindowAcknowledgementSize:
            if ((ret = decode_message(msg, &packet)) != ERROR_SUCCESS) {
                srs_error("decode packet from message payload failed. ret=%d", ret);
                return ret;
            }
            srs_verbose("decode packet from message payload success.");
            break;
        default:
            return ret;
    }
    srs_assert(packet);
    SrsAutoFree(SrsPacket, packet);
    switch (msg->header.message_type) {
        case RTMP_MSG_WindowAcknowledgementSize: {
            SrsSetWindowAckSizePacket* pkt = dynamic_cast<SrsSetWindowAckSizePacket *>(packet);
            srs_assert(pkt != NULL);
            if (pkt->ackowledgement_window_size > 0) {
                in_ack_size.ack_window_size = pkt->ackowledgement_window_size;
                srs_info("set ack window size to %d", pkt->ackowledgement_window_size);
            } else {
                srs_warn("ignored. set ack window size is %d", pkt->ackowledgement_window_size);
            }
            break;
        }
        case RTMP_MSG_SetChunkSize: {
            SrsSetChunkSizePacket* pkt = dynamic_cast<SrsSetChunkSizePacket *>(packet);
            srs_assert(pkt != NULL);
            if (pkt->chunk_size < SRS_CONSTS_RTMP_MIN_CHUNK_SIZE ||
                pkt->chunk_size > SRS_CONSTS_RTMP_MAX_CHUNK_SIZE)
            {
                srs_warn("accept chunk size %d, but should in [%d, %d], "
                         "@see: https://github.com/ossrs/srs/issues/160",
                         pkt->chunk_size, SRS_CONSTS_RTMP_MIN_CHUNK_SIZE, SRS_CONSTS_RTMP_MAX_CHUNK_SIZE);
            }
            if (pkt->chunk_size < SRS_CONSTS_RTMP_MIN_CHUNK_SIZE) {
                ret = ERROR_RTMP_CHUNK_SIZE;
                srs_error("chunk size should be %d+, value=%d. ret=%d",
                          SRS_CONSTS_RTMP_MIN_CHUNK_SIZE, pkt->chunk_size, ret);
                return ret;
            }
            in_chunk_size = pkt->chunk_size;
            srs_trace("input chunk size to %d", pkt->chunk_size);
            break;
        }
        case RTMP_MSG_UserControlMessage: {
            SrsUserControlPacket* pkt = dynamic_cast<SrsUserControlPacket *>(packet);
            srs_assert(pkt != NULL);
            if (pkt->event_type == SrcPCUCSetBufferLength) {
                srs_trace("ignored. set buffer length to %d", pkt->extra_data);
            }
            if (pkt->event_type == SrcPCUCPingRequest) {
                if ((ret = response_ping_message(pkt->event_data)) != ERROR_SUCCESS) {
                    return ret;
                }
            }
            break;
        }
        default:
            break;
    }
    return ret;
} // SrsProtocol::on_recv_message

int SrsProtocol::on_send_packet(SrsMessageHeader* mh, SrsPacket* packet)
{
    int ret = ERROR_SUCCESS;

    if (packet == NULL) {
        return ret;
    }
    switch (mh->message_type) {
        case RTMP_MSG_SetChunkSize: {
            SrsSetChunkSizePacket* pkt = dynamic_cast<SrsSetChunkSizePacket *>(packet);
            srs_assert(pkt != NULL);
            out_chunk_size = pkt->chunk_size;
            srs_trace("out chunk size to %d", pkt->chunk_size);
            break;
        }
        case RTMP_MSG_AMF0CommandMessage:
        case RTMP_MSG_AMF3CommandMessage: {
            if (true) {
                SrsConnectAppPacket* pkt = dynamic_cast<SrsConnectAppPacket *>(packet);
                if (pkt) {
                    requests[pkt->transaction_id] = pkt->command_name;
                    break;
                }
            }
            if (true) {
                SrsCreateStreamPacket* pkt = dynamic_cast<SrsCreateStreamPacket *>(packet);
                if (pkt) {
                    requests[pkt->transaction_id] = pkt->command_name;
                    break;
                }
            }
            if (true) {
                SrsFMLEStartPacket* pkt = dynamic_cast<SrsFMLEStartPacket *>(packet);
                if (pkt) {
                    requests[pkt->transaction_id] = pkt->command_name;
                    break;
                }
            }
            break;
        }
        default:
            break;
    }
    return ret;
} // SrsProtocol::on_send_packet

int SrsProtocol::response_acknowledgement_message()
{
    int ret = ERROR_SUCCESS;
    SrsAcknowledgementPacket* pkt = new SrsAcknowledgementPacket();

    in_ack_size.acked_size = skt->get_recv_bytes();
    pkt->sequence_number   = (int32_t) in_ack_size.acked_size;
    if (!auto_response_when_recv) {
        manual_response_queue.push_back(pkt);
        return ret;
    }
    if ((ret = do_send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        srs_error("send acknowledgement failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("send acknowledgement success.");
    return ret;
}

int SrsProtocol::response_ping_message(int32_t timestamp)
{
    int ret = ERROR_SUCCESS;

    srs_trace("get a ping request, response it. timestamp=%d", timestamp);
    SrsUserControlPacket* pkt = new SrsUserControlPacket();
    pkt->event_type = SrcPCUCPingResponse;
    pkt->event_data = timestamp;
    if (!auto_response_when_recv) {
        manual_response_queue.push_back(pkt);
        return ret;
    }
    if ((ret = do_send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        srs_error("send ping response failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("send ping response success.");
    return ret;
}

SrsChunkStream::SrsChunkStream(int _cid)
{
    fmt = 0;
    cid = _cid;
    extended_timestamp = false;
    msg       = NULL;
    msg_count = 0;
}

SrsChunkStream::~SrsChunkStream()
{
    srs_freep(msg);
}

SrsRequest::SrsRequest()
{
    objectEncoding = RTMP_SIG_AMF0_VER;
    duration       = -1;
    args = NULL;
}

SrsRequest::~SrsRequest()
{
    srs_freep(args);
}

SrsRequest * SrsRequest::copy()
{
    SrsRequest* cp = new SrsRequest();

    cp->ip  = ip;
    cp->app = app;
    cp->objectEncoding = objectEncoding;
    cp->pageUrl        = pageUrl;
    cp->host     = host;
    cp->port     = port;
    cp->param    = param;
    cp->schema   = schema;
    cp->stream   = stream;
    cp->swfUrl   = swfUrl;
    cp->tcUrl    = tcUrl;
    cp->vhost    = vhost;
    cp->duration = duration;
    if (args) {
        cp->args = args->copy()->to_object();
    }
    return cp;
}

void SrsRequest::update_auth(SrsRequest* req)
{
    pageUrl = req->pageUrl;
    swfUrl  = req->swfUrl;
    tcUrl   = req->tcUrl;
    if (args) {
        srs_freep(args);
    }
    if (req->args) {
        args = req->args->copy()->to_object();
    }
    srs_info("update req of soruce for auth ok");
}

string SrsRequest::get_stream_url()
{
    return srs_generate_stream_url(vhost, app, stream);
}

void SrsRequest::strip()
{
    host   = srs_string_remove(host, "/ \n\r\t");
    vhost  = srs_string_remove(vhost, "/ \n\r\t");
    app    = srs_string_remove(app, " \n\r\t");
    stream = srs_string_remove(stream, " \n\r\t");
    app    = srs_string_trim_end(app, "/");
    stream = srs_string_trim_end(stream, "/");
    app    = srs_string_trim_start(app, "/");
    stream = srs_string_trim_start(stream, "/");
}

SrsResponse::SrsResponse()
{
    stream_id = SRS_DEFAULT_SID;
}

SrsResponse::~SrsResponse()
{}

string srs_client_type_string(SrsRtmpConnType type)
{
    switch (type) {
        case SrsRtmpConnPlay:
            return "Play";

        case SrsRtmpConnFlashPublish:
            return "flash-publish)";

        case SrsRtmpConnFMLEPublish:
            return "fmle-publish";

        default:
            return "Unknown";
    }
}

bool srs_client_type_is_publish(SrsRtmpConnType type)
{
    return type != SrsRtmpConnPlay;
}

SrsHandshakeBytes::SrsHandshakeBytes()
{
    c0c1 = s0s1s2 = c2 = NULL;
}

SrsHandshakeBytes::~SrsHandshakeBytes()
{
    srs_freepa(c0c1);
    srs_freepa(s0s1s2);
    srs_freepa(c2);
}

int SrsHandshakeBytes::read_c0c1(ISrsProtocolReaderWriter* io)
{
    int ret = ERROR_SUCCESS;

    if (c0c1) {
        return ret;
    }
    ssize_t nsize;
    c0c1 = new char[1537];
    if ((ret = io->read_fully(c0c1, 1537, &nsize)) != ERROR_SUCCESS) {
        srs_warn("read c0c1 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("read c0c1 success.");
    return ret;
}

int SrsHandshakeBytes::read_s0s1s2(ISrsProtocolReaderWriter* io)
{
    int ret = ERROR_SUCCESS;

    if (s0s1s2) {
        return ret;
    }
    ssize_t nsize;
    s0s1s2 = new char[3073];
    if ((ret = io->read_fully(s0s1s2, 3073, &nsize)) != ERROR_SUCCESS) {
        srs_warn("read s0s1s2 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("read s0s1s2 success.");
    return ret;
}

int SrsHandshakeBytes::read_c2(ISrsProtocolReaderWriter* io)
{
    int ret = ERROR_SUCCESS;

    if (c2) {
        return ret;
    }
    ssize_t nsize;
    c2 = new char[1536];
    if ((ret = io->read_fully(c2, 1536, &nsize)) != ERROR_SUCCESS) {
        srs_warn("read c2 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("read c2 success.");
    return ret;
}

int SrsHandshakeBytes::create_c0c1()
{
    int ret = ERROR_SUCCESS;

    if (c0c1) {
        return ret;
    }
    c0c1 = new char[1537];
    srs_random_generate(c0c1, 1537);
    SrsStream stream;
    if ((ret = stream.initialize(c0c1, 9)) != ERROR_SUCCESS) {
        return ret;
    }
    stream.write_1bytes(0x03);
    stream.write_4bytes((int32_t) ::time(NULL));
    stream.write_4bytes(0x00);
    return ret;
}

int SrsHandshakeBytes::create_s0s1s2(const char* c1)
{
    int ret = ERROR_SUCCESS;

    if (s0s1s2) {
        return ret;
    }
    s0s1s2 = new char[3073];
    srs_random_generate(s0s1s2, 3073);
    SrsStream stream;
    if ((ret = stream.initialize(s0s1s2, 9)) != ERROR_SUCCESS) {
        return ret;
    }
    stream.write_1bytes(0x03);
    stream.write_4bytes((int32_t) ::time(NULL));
    if (c0c1) {
        stream.write_bytes(c0c1 + 1, 4);
    }
    if (c1) {
        memcpy(s0s1s2 + 1537, c1, 1536);
    }
    return ret;
}

int SrsHandshakeBytes::create_c2()
{
    int ret = ERROR_SUCCESS;

    if (c2) {
        return ret;
    }
    c2 = new char[1536];
    srs_random_generate(c2, 1536);
    SrsStream stream;
    if ((ret = stream.initialize(c2, 8)) != ERROR_SUCCESS) {
        return ret;
    }
    stream.write_4bytes((int32_t) ::time(NULL));
    if (s0s1s2) {
        stream.write_bytes(s0s1s2 + 1, 4);
    }
    return ret;
}

SrsRtmpClient::SrsRtmpClient(ISrsProtocolReaderWriter* skt)
{
    io       = skt;
    protocol = new SrsProtocol(skt);
    hs_bytes = new SrsHandshakeBytes();
}

SrsRtmpClient::~SrsRtmpClient()
{
    srs_freep(protocol);
    srs_freep(hs_bytes);
}

void SrsRtmpClient::set_recv_timeout(int64_t timeout_us)
{
    protocol->set_recv_timeout(timeout_us);
}

void SrsRtmpClient::set_send_timeout(int64_t timeout_us)
{
    protocol->set_send_timeout(timeout_us);
}

int64_t SrsRtmpClient::get_recv_bytes()
{
    return protocol->get_recv_bytes();
}

int64_t SrsRtmpClient::get_send_bytes()
{
    return protocol->get_send_bytes();
}

int SrsRtmpClient::recv_message(SrsCommonMessage** pmsg)
{
    return protocol->recv_message(pmsg);
}

int SrsRtmpClient::decode_message(SrsCommonMessage* msg, SrsPacket** ppacket)
{
    return protocol->decode_message(msg, ppacket);
}

int SrsRtmpClient::send_and_free_message(SrsSharedPtrMessage* msg, int stream_id)
{
    return protocol->send_and_free_message(msg, stream_id);
}

int SrsRtmpClient::send_and_free_messages(SrsSharedPtrMessage** msgs, int nb_msgs, int stream_id)
{
    return protocol->send_and_free_messages(msgs, nb_msgs, stream_id);
}

int SrsRtmpClient::send_and_free_packet(SrsPacket* packet, int stream_id)
{
    return protocol->send_and_free_packet(packet, stream_id);
}

int SrsRtmpClient::handshake()
{
    int ret = ERROR_SUCCESS;

    srs_assert(hs_bytes);
    SrsComplexHandshake* complex_hs = new SrsComplexHandshake();
    SrsAutoFree(SrsComplexHandshake, complex_hs);
    if ((ret = complex_hs->handshake_with_server(hs_bytes, io)) != ERROR_SUCCESS) {
        if (ret == ERROR_RTMP_TRY_SIMPLE_HS) {
            SrsSimpleHandshake* simple_hs = new SrsSimpleHandshake();
            SrsAutoFree(SrsSimpleHandshake, simple_hs);
            if ((ret = simple_hs->handshake_with_server(hs_bytes, io)) != ERROR_SUCCESS) {
                return ret;
            }
        }
        return ret;
    }
    srs_freep(hs_bytes);
    return ret;
}

int SrsRtmpClient::simple_handshake()
{
    int ret = ERROR_SUCCESS;

    srs_assert(hs_bytes);
    SrsSimpleHandshake simple_hs;
    if ((ret = simple_hs.handshake_with_server(hs_bytes, io)) != ERROR_SUCCESS) {
        return ret;
    }
    srs_freep(hs_bytes);
    return ret;
}

int SrsRtmpClient::complex_handshake()
{
    int ret = ERROR_SUCCESS;

    srs_assert(hs_bytes);
    SrsComplexHandshake complex_hs;
    if ((ret = complex_hs.handshake_with_server(hs_bytes, io)) != ERROR_SUCCESS) {
        return ret;
    }
    srs_freep(hs_bytes);
    return ret;
}

int SrsRtmpClient::connect_app(string app, string tc_url, SrsRequest* req, bool debug_srs_upnode)
{
    std::string srs_server_ip;
    std::string srs_server;
    std::string srs_primary;
    std::string srs_authors;
    std::string srs_version;
    int srs_id  = 0;
    int srs_pid = 0;

    return connect_app2(app, tc_url, req, debug_srs_upnode,
                        srs_server_ip, srs_server, srs_primary, srs_authors,
                        srs_version, srs_id, srs_pid);
}

int SrsRtmpClient::connect_app2(
    string app, string tc_url, SrsRequest* req, bool debug_srs_upnode,
    string& srs_server_ip, string& srs_server, string& srs_primary,
    string& srs_authors, string& srs_version, int& srs_id,
    int& srs_pid
)
{
    int ret = ERROR_SUCCESS;

    if (true) {
        SrsConnectAppPacket* pkt = new SrsConnectAppPacket();
        pkt->command_object->set("app", SrsAmf0Any::str(app.c_str()));
        pkt->command_object->set("flashVer", SrsAmf0Any::str("WIN 15,0,0,239"));
        if (req) {
            pkt->command_object->set("swfUrl", SrsAmf0Any::str(req->swfUrl.c_str()));
        } else {
            pkt->command_object->set("swfUrl", SrsAmf0Any::str());
        }
        if (req && req->tcUrl != "") {
            pkt->command_object->set("tcUrl", SrsAmf0Any::str(req->tcUrl.c_str()));
        } else {
            pkt->command_object->set("tcUrl", SrsAmf0Any::str(tc_url.c_str()));
        }
        pkt->command_object->set("fpad", SrsAmf0Any::boolean(false));
        pkt->command_object->set("capabilities", SrsAmf0Any::number(239));
        pkt->command_object->set("audioCodecs", SrsAmf0Any::number(3575));
        pkt->command_object->set("videoCodecs", SrsAmf0Any::number(252));
        pkt->command_object->set("videoFunction", SrsAmf0Any::number(1));
        if (req) {
            pkt->command_object->set("pageUrl", SrsAmf0Any::str(req->pageUrl.c_str()));
        } else {
            pkt->command_object->set("pageUrl", SrsAmf0Any::str());
        }
        pkt->command_object->set("objectEncoding", SrsAmf0Any::number(0));
        if (debug_srs_upnode && req && req->args) {
            srs_freep(pkt->args);
            pkt->args = req->args->copy()->to_object();
        }
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            return ret;
        }
    }
    if (true) {
        SrsSetWindowAckSizePacket* pkt = new SrsSetWindowAckSizePacket();
        pkt->ackowledgement_window_size = 2500000;
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            return ret;
        }
    }
    SrsCommonMessage* msg       = NULL;
    SrsConnectAppResPacket* pkt = NULL;
    if ((ret = expect_message<SrsConnectAppResPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
        srs_error("expect connect app response message failed. ret=%d", ret);
        return ret;
    }
    SrsAutoFree(SrsCommonMessage, msg);
    SrsAutoFree(SrsConnectAppResPacket, pkt);
    SrsAmf0Any* data = pkt->info->get_property("data");
    if (data && data->is_ecma_array()) {
        SrsAmf0EcmaArray* arr = data->to_ecma_array();
        SrsAmf0Any* prop      = NULL;
        if ((prop = arr->ensure_property_string("srs_primary")) != NULL) {
            srs_primary = prop->to_str();
        }
        if ((prop = arr->ensure_property_string("srs_authors")) != NULL) {
            srs_authors = prop->to_str();
        }
        if ((prop = arr->ensure_property_string("srs_version")) != NULL) {
            srs_version = prop->to_str();
        }
        if ((prop = arr->ensure_property_string("srs_server_ip")) != NULL) {
            srs_server_ip = prop->to_str();
        }
        if ((prop = arr->ensure_property_string("srs_server")) != NULL) {
            srs_server = prop->to_str();
        }
        if ((prop = arr->ensure_property_number("srs_id")) != NULL) {
            srs_id = (int) prop->to_number();
        }
        if ((prop = arr->ensure_property_number("srs_pid")) != NULL) {
            srs_pid = (int) prop->to_number();
        }
    }
    srs_trace("connected, version=%s, ip=%s, pid=%d, id=%d, dsu=%d",
              srs_version.c_str(), srs_server_ip.c_str(), srs_pid, srs_id, debug_srs_upnode);
    return ret;
} // SrsRtmpClient::connect_app2

int SrsRtmpClient::create_stream(int& stream_id)
{
    int ret = ERROR_SUCCESS;

    if (true) {
        SrsCreateStreamPacket* pkt = new SrsCreateStreamPacket();
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            return ret;
        }
    }
    if (true) {
        SrsCommonMessage* msg         = NULL;
        SrsCreateStreamResPacket* pkt = NULL;
        if ((ret = expect_message<SrsCreateStreamResPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
            srs_error("expect create stream response message failed. ret=%d", ret);
            return ret;
        }
        SrsAutoFree(SrsCommonMessage, msg);
        SrsAutoFree(SrsCreateStreamResPacket, pkt);
        srs_info("get create stream response message");
        stream_id = (int) pkt->stream_id;
    }
    return ret;
}

int SrsRtmpClient::play(string stream, int stream_id)
{
    int ret = ERROR_SUCCESS;

    if (true) {
        SrsPlayPacket* pkt = new SrsPlayPacket();
        pkt->stream_name = stream;
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            srs_error("send play stream failed. "
                      "stream=%s, stream_id=%d, ret=%d",
                      stream.c_str(), stream_id, ret);
            return ret;
        }
    }
    int buffer_length_ms = 1000;
    if (true) {
        SrsUserControlPacket* pkt = new SrsUserControlPacket();
        pkt->event_type = SrcPCUCSetBufferLength;
        pkt->event_data = stream_id;
        pkt->extra_data = buffer_length_ms;
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send set buffer length failed. "
                      "stream=%s, stream_id=%d, bufferLength=%d, ret=%d",
                      stream.c_str(), stream_id, buffer_length_ms, ret);
            return ret;
        }
    }
    if (true) {
        SrsSetChunkSizePacket* pkt = new SrsSetChunkSizePacket();
        pkt->chunk_size = SRS_CONSTS_RTMP_SRS_CHUNK_SIZE;
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send set chunk size failed. "
                      "stream=%s, chunk_size=%d, ret=%d",
                      stream.c_str(), SRS_CONSTS_RTMP_SRS_CHUNK_SIZE, ret);
            return ret;
        }
    }
    return ret;
} // SrsRtmpClient::play

int SrsRtmpClient::publish(string stream, int stream_id)
{
    int ret = ERROR_SUCCESS;

    if (true) {
        SrsSetChunkSizePacket* pkt = new SrsSetChunkSizePacket();
        pkt->chunk_size = SRS_CONSTS_RTMP_SRS_CHUNK_SIZE;
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send set chunk size failed. "
                      "stream=%s, chunk_size=%d, ret=%d",
                      stream.c_str(), SRS_CONSTS_RTMP_SRS_CHUNK_SIZE, ret);
            return ret;
        }
    }
    if (true) {
        SrsPublishPacket* pkt = new SrsPublishPacket();
        pkt->stream_name = stream;
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            srs_error("send publish message failed. "
                      "stream=%s, stream_id=%d, ret=%d",
                      stream.c_str(), stream_id, ret);
            return ret;
        }
    }
    return ret;
}

int SrsRtmpClient::fmle_publish(string stream, int& stream_id)
{
    stream_id = 0;
    int ret = ERROR_SUCCESS;
    if (true) {
        SrsFMLEStartPacket* pkt = SrsFMLEStartPacket::create_release_stream(stream);
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send FMLE publish "
                      "release stream failed. stream=%s, ret=%d", stream.c_str(), ret);
            return ret;
        }
    }
    if (true) {
        SrsFMLEStartPacket* pkt = SrsFMLEStartPacket::create_FC_publish(stream);
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send FMLE publish "
                      "FCPublish failed. stream=%s, ret=%d", stream.c_str(), ret);
            return ret;
        }
    }
    if (true) {
        SrsCreateStreamPacket* pkt = new SrsCreateStreamPacket();
        pkt->transaction_id = 4;
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send FMLE publish "
                      "createStream failed. stream=%s, ret=%d", stream.c_str(), ret);
            return ret;
        }
    }
    if (true) {
        SrsCommonMessage* msg         = NULL;
        SrsCreateStreamResPacket* pkt = NULL;
        if ((ret = expect_message<SrsCreateStreamResPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
            srs_error("expect create stream response message failed. ret=%d", ret);
            return ret;
        }
        SrsAutoFree(SrsCommonMessage, msg);
        SrsAutoFree(SrsCreateStreamResPacket, pkt);
        srs_info("get create stream response message");
        stream_id = (int) pkt->stream_id;
    }
    if (true) {
        SrsPublishPacket* pkt = new SrsPublishPacket();
        pkt->stream_name = stream;
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            srs_error("send FMLE publish publish failed. "
                      "stream=%s, stream_id=%d, ret=%d", stream.c_str(), stream_id, ret);
            return ret;
        }
    }
    return ret;
} // SrsRtmpClient::fmle_publish

SrsRtmpServer::SrsRtmpServer(ISrsProtocolReaderWriter* skt)
{
    io       = skt;
    protocol = new SrsProtocol(skt);
    hs_bytes = new SrsHandshakeBytes();
}

SrsRtmpServer::~SrsRtmpServer()
{
    srs_freep(protocol);
    srs_freep(hs_bytes);
}

void SrsRtmpServer::set_auto_response(bool v)
{
    protocol->set_auto_response(v);
}

#ifdef SRS_PERF_MERGED_READ
void SrsRtmpServer::set_merge_read(bool v, IMergeReadHandler* handler)
{
    protocol->set_merge_read(v, handler);
}

void SrsRtmpServer::set_recv_buffer(int buffer_size)
{
    protocol->set_recv_buffer(buffer_size);
}

#endif // ifdef SRS_PERF_MERGED_READ
void SrsRtmpServer::set_recv_timeout(int64_t timeout_us)
{
    protocol->set_recv_timeout(timeout_us);
}

int64_t SrsRtmpServer::get_recv_timeout()
{
    return protocol->get_recv_timeout();
}

void SrsRtmpServer::set_send_timeout(int64_t timeout_us)
{
    protocol->set_send_timeout(timeout_us);
}

int64_t SrsRtmpServer::get_send_timeout()
{
    return protocol->get_send_timeout();
}

int64_t SrsRtmpServer::get_recv_bytes()
{
    return protocol->get_recv_bytes();
}

int64_t SrsRtmpServer::get_send_bytes()
{
    return protocol->get_send_bytes();
}

int SrsRtmpServer::recv_message(SrsCommonMessage** pmsg)
{
    return protocol->recv_message(pmsg);
}

int SrsRtmpServer::decode_message(SrsCommonMessage* msg, SrsPacket** ppacket)
{
    return protocol->decode_message(msg, ppacket);
}

int SrsRtmpServer::send_and_free_message(SrsSharedPtrMessage* msg, int stream_id)
{
    return protocol->send_and_free_message(msg, stream_id);
}

int SrsRtmpServer::send_and_free_messages(SrsSharedPtrMessage** msgs, int nb_msgs, int stream_id)
{
    return protocol->send_and_free_messages(msgs, nb_msgs, stream_id);
}

int SrsRtmpServer::send_and_free_packet(SrsPacket* packet, int stream_id)
{
    return protocol->send_and_free_packet(packet, stream_id);
}

int SrsRtmpServer::handshake()
{
    int ret = ERROR_SUCCESS;

    srs_assert(hs_bytes);
    SrsComplexHandshake complex_hs;
    if ((ret = complex_hs.handshake_with_client(hs_bytes, io)) != ERROR_SUCCESS) {
        if (ret == ERROR_RTMP_TRY_SIMPLE_HS) {
            SrsSimpleHandshake simple_hs;
            if ((ret = simple_hs.handshake_with_client(hs_bytes, io)) != ERROR_SUCCESS) {
                return ret;
            }
        }
        return ret;
    }
    srs_freep(hs_bytes);
    return ret;
}

int SrsRtmpServer::connect_app(SrsRequest* req)
{
    int ret = ERROR_SUCCESS;
    SrsCommonMessage* msg    = NULL;
    SrsConnectAppPacket* pkt = NULL;

    if ((ret = expect_message<SrsConnectAppPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
        srs_error("expect connect app message failed. ret=%d", ret);
        return ret;
    }
    SrsAutoFree(SrsCommonMessage, msg);
    SrsAutoFree(SrsConnectAppPacket, pkt);
    srs_info("get connect app message");
    SrsAmf0Any* prop = NULL;
    if ((prop = pkt->command_object->ensure_property_string("tcUrl")) == NULL) {
        ret = ERROR_RTMP_REQ_CONNECT;
        srs_error("invalid request, must specifies the tcUrl. ret=%d", ret);
        return ret;
    }
    req->tcUrl = prop->to_str();
    if ((prop = pkt->command_object->ensure_property_string("pageUrl")) != NULL) {
        req->pageUrl = prop->to_str();
    }
    if ((prop = pkt->command_object->ensure_property_string("swfUrl")) != NULL) {
        req->swfUrl = prop->to_str();
    }
    if ((prop = pkt->command_object->ensure_property_number("objectEncoding")) != NULL) {
        req->objectEncoding = prop->to_number();
    }
    if (pkt->args) {
        srs_freep(req->args);
        req->args = pkt->args->copy()->to_object();
        srs_info("copy edge traverse to origin auth args.");
    }
    srs_info("get connect app message params success.");
    srs_discovery_tc_url(req->tcUrl,
                         req->schema, req->host, req->vhost, req->app, req->port,
                         req->param);
    req->strip();
    return ret;
} // SrsRtmpServer::connect_app

int SrsRtmpServer::set_window_ack_size(int ack_size)
{
    int ret = ERROR_SUCCESS;
    SrsSetWindowAckSizePacket* pkt = new SrsSetWindowAckSizePacket();

    pkt->ackowledgement_window_size = ack_size;
    if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        srs_error("send ack size message failed. ret=%d", ret);
        return ret;
    }
    srs_info("send ack size message success. ack_size=%d", ack_size);
    return ret;
}

int SrsRtmpServer::set_peer_bandwidth(int bandwidth, int type)
{
    int ret = ERROR_SUCCESS;
    SrsSetPeerBandwidthPacket* pkt = new SrsSetPeerBandwidthPacket();

    pkt->bandwidth = bandwidth;
    pkt->type      = type;
    if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        srs_error("send set bandwidth message failed. ret=%d", ret);
        return ret;
    }
    srs_info("send set bandwidth message "
             "success. bandwidth=%d, type=%d", bandwidth, type);
    return ret;
}

int SrsRtmpServer::response_connect_app(SrsRequest* req, const char* server_ip)
{
    int ret = ERROR_SUCCESS;
    SrsConnectAppResPacket* pkt = new SrsConnectAppResPacket();

    pkt->props->set("fmsVer", SrsAmf0Any::str("FMS/" RTMP_SIG_FMS_VER));
    pkt->props->set("capabilities", SrsAmf0Any::number(127));
    pkt->props->set("mode", SrsAmf0Any::number(1));
    pkt->info->set(StatusLevel, SrsAmf0Any::str(StatusLevelStatus));
    pkt->info->set(StatusCode, SrsAmf0Any::str(StatusCodeConnectSuccess));
    pkt->info->set(StatusDescription, SrsAmf0Any::str("Connection succeeded"));
    pkt->info->set("objectEncoding", SrsAmf0Any::number(req->objectEncoding));
    SrsAmf0EcmaArray* data = SrsAmf0Any::ecma_array();
    pkt->info->set("data", data);
    data->set("version", SrsAmf0Any::str(RTMP_SIG_FMS_VER));
    data->set("srs_sig", SrsAmf0Any::str(RTMP_SIG_SRS_KEY));
    data->set("srs_server", SrsAmf0Any::str(RTMP_SIG_SRS_SERVER));
    data->set("srs_license", SrsAmf0Any::str(RTMP_SIG_SRS_LICENSE));
    data->set("srs_role", SrsAmf0Any::str(RTMP_SIG_SRS_ROLE));
    data->set("srs_url", SrsAmf0Any::str(RTMP_SIG_SRS_URL));
    data->set("srs_version", SrsAmf0Any::str(RTMP_SIG_SRS_VERSION));
    data->set("srs_site", SrsAmf0Any::str(RTMP_SIG_SRS_WEB));
    data->set("srs_email", SrsAmf0Any::str(RTMP_SIG_SRS_EMAIL));
    data->set("srs_copyright", SrsAmf0Any::str(RTMP_SIG_SRS_COPYRIGHT));
    data->set("srs_primary", SrsAmf0Any::str(RTMP_SIG_SRS_PRIMARY));
    data->set("srs_authors", SrsAmf0Any::str(RTMP_SIG_SRS_AUTHROS));
    if (server_ip) {
        data->set("srs_server_ip", SrsAmf0Any::str(server_ip));
    }
    data->set("srs_pid", SrsAmf0Any::number(getpid()));
    data->set("srs_id", SrsAmf0Any::number(_srs_context->get_id()));
    if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        srs_error("send connect app response message failed. ret=%d", ret);
        return ret;
    }
    srs_info("send connect app response message success.");
    return ret;
} // SrsRtmpServer::response_connect_app

void SrsRtmpServer::response_connect_reject(SrsRequest *, const char* desc)
{
    int ret = ERROR_SUCCESS;
    SrsOnStatusCallPacket* pkt = new SrsOnStatusCallPacket();

    pkt->data->set(StatusLevel, SrsAmf0Any::str(StatusLevelError));
    pkt->data->set(StatusCode, SrsAmf0Any::str(StatusCodeConnectRejected));
    pkt->data->set(StatusDescription, SrsAmf0Any::str(desc));
    if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        srs_error("send connect app response rejected message failed. ret=%d", ret);
        return;
    }
    srs_info("send connect app response rejected message success.");
}

int SrsRtmpServer::on_bw_done()
{
    int ret = ERROR_SUCCESS;
    SrsOnBWDonePacket* pkt = new SrsOnBWDonePacket();

    if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        srs_error("send onBWDone message failed. ret=%d", ret);
        return ret;
    }
    srs_info("send onBWDone message success.");
    return ret;
}

int SrsRtmpServer::identify_client(int stream_id, SrsRtmpConnType& type, string& stream_name, double& duration)
{
    type = SrsRtmpConnUnknown;
    int ret = ERROR_SUCCESS;
    while (true) {
        SrsCommonMessage* msg = NULL;
        if ((ret = protocol->recv_message(&msg)) != ERROR_SUCCESS) {
            if (!srs_is_client_gracefully_close(ret)) {
                srs_error("recv identify client message failed. ret=%d", ret);
            }
            return ret;
        }
        SrsAutoFree(SrsCommonMessage, msg);
        SrsMessageHeader& h = msg->header;
        if (h.is_ackledgement() || h.is_set_chunk_size() || h.is_window_ackledgement_size() ||
            h.is_user_control_message())
        {
            continue;
        }
        if (!h.is_amf0_command() && !h.is_amf3_command()) {
            srs_trace("identify ignore messages except "
                      "AMF0/AMF3 command message. type=%#x", h.message_type);
            continue;
        }
        SrsPacket* pkt = NULL;
        if ((ret = protocol->decode_message(msg, &pkt)) != ERROR_SUCCESS) {
            srs_error("identify decode message failed. ret=%d", ret);
            return ret;
        }
        SrsAutoFree(SrsPacket, pkt);
        if (dynamic_cast<SrsCreateStreamPacket *>(pkt)) {
            srs_info("identify client by create stream, play or flash publish.");
            return identify_create_stream_client(dynamic_cast<SrsCreateStreamPacket *>(pkt), stream_id, type,
                                                 stream_name, duration);
        }
        if (dynamic_cast<SrsFMLEStartPacket *>(pkt)) {
            srs_info("identify client by releaseStream, fmle publish.");
            return identify_fmle_publish_client(dynamic_cast<SrsFMLEStartPacket *>(pkt), type, stream_name);
        }
        if (dynamic_cast<SrsPlayPacket *>(pkt)) {
            srs_info("level0 identify client by play.");
            return identify_play_client(dynamic_cast<SrsPlayPacket *>(pkt), type, stream_name, duration);
        }
        SrsCallPacket* call = dynamic_cast<SrsCallPacket *>(pkt);
        if (call) {
            SrsCallResPacket* res = new SrsCallResPacket(call->transaction_id);
            res->command_object = SrsAmf0Any::null();
            res->response       = SrsAmf0Any::null();
            if ((ret = protocol->send_and_free_packet(res, 0)) != ERROR_SUCCESS) {
                if (!srs_is_system_control_error(ret) && !srs_is_client_gracefully_close(ret)) {
                    srs_warn("response call failed. ret=%d", ret);
                }
                return ret;
            }
            continue;
        }
        srs_trace("ignore AMF0/AMF3 command message.");
    }
    return ret;
} // SrsRtmpServer::identify_client

int SrsRtmpServer::set_chunk_size(int chunk_size)
{
    int ret = ERROR_SUCCESS;
    SrsSetChunkSizePacket* pkt = new SrsSetChunkSizePacket();

    pkt->chunk_size = chunk_size;
    if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
        srs_error("send set chunk size message failed. ret=%d", ret);
        return ret;
    }
    srs_info("send set chunk size message success. chunk_size=%d", chunk_size);
    return ret;
}

int SrsRtmpServer::start_play(int stream_id)
{
    int ret = ERROR_SUCCESS;

    if (true) {
        SrsUserControlPacket* pkt = new SrsUserControlPacket();
        pkt->event_type = SrcPCUCStreamBegin;
        pkt->event_data = stream_id;
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send PCUC(StreamBegin) message failed. ret=%d", ret);
            return ret;
        }
        srs_info("send PCUC(StreamBegin) message success.");
    }
    if (true) {
        SrsOnStatusCallPacket* pkt = new SrsOnStatusCallPacket();
        pkt->data->set(StatusLevel, SrsAmf0Any::str(StatusLevelStatus));
        pkt->data->set(StatusCode, SrsAmf0Any::str(StatusCodeStreamReset));
        pkt->data->set(StatusDescription, SrsAmf0Any::str("Playing and resetting stream."));
        pkt->data->set(StatusDetails, SrsAmf0Any::str("stream"));
        pkt->data->set(StatusClientId, SrsAmf0Any::str(RTMP_SIG_CLIENT_ID));
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            srs_error("send onStatus(NetStream.Play.Reset) message failed. ret=%d", ret);
            return ret;
        }
        srs_info("send onStatus(NetStream.Play.Reset) message success.");
    }
    if (true) {
        SrsOnStatusCallPacket* pkt = new SrsOnStatusCallPacket();
        pkt->data->set(StatusLevel, SrsAmf0Any::str(StatusLevelStatus));
        pkt->data->set(StatusCode, SrsAmf0Any::str(StatusCodeStreamStart));
        pkt->data->set(StatusDescription, SrsAmf0Any::str("Started playing stream."));
        pkt->data->set(StatusDetails, SrsAmf0Any::str("stream"));
        pkt->data->set(StatusClientId, SrsAmf0Any::str(RTMP_SIG_CLIENT_ID));
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            srs_error("send onStatus(NetStream.Play.Start) message failed. ret=%d", ret);
            return ret;
        }
        srs_info("send onStatus(NetStream.Play.Start) message success.");
    }
    if (true) {
        SrsSampleAccessPacket* pkt = new SrsSampleAccessPacket();
        pkt->audio_sample_access = true;
        pkt->video_sample_access = true;
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            srs_error("send |RtmpSampleAccess(false, false) message failed. ret=%d", ret);
            return ret;
        }
        srs_info("send |RtmpSampleAccess(false, false) message success.");
    }
    if (true) {
        SrsOnStatusDataPacket* pkt = new SrsOnStatusDataPacket();
        pkt->data->set(StatusCode, SrsAmf0Any::str(StatusCodeDataStart));
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            srs_error("send onStatus(NetStream.Data.Start) message failed. ret=%d", ret);
            return ret;
        }
        srs_info("send onStatus(NetStream.Data.Start) message success.");
    }
    srs_info("start play success.");
    return ret;
} // SrsRtmpServer::start_play

int SrsRtmpServer::on_play_client_pause(int stream_id, bool is_pause)
{
    int ret = ERROR_SUCCESS;

    if (is_pause) {
        if (true) {
            SrsOnStatusCallPacket* pkt = new SrsOnStatusCallPacket();
            pkt->data->set(StatusLevel, SrsAmf0Any::str(StatusLevelStatus));
            pkt->data->set(StatusCode, SrsAmf0Any::str(StatusCodeStreamPause));
            pkt->data->set(StatusDescription, SrsAmf0Any::str("Paused stream."));
            if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
                srs_error("send onStatus(NetStream.Pause.Notify) message failed. ret=%d", ret);
                return ret;
            }
            srs_info("send onStatus(NetStream.Pause.Notify) message success.");
        }
        if (true) {
            SrsUserControlPacket* pkt = new SrsUserControlPacket();
            pkt->event_type = SrcPCUCStreamEOF;
            pkt->event_data = stream_id;
            if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
                srs_error("send PCUC(StreamEOF) message failed. ret=%d", ret);
                return ret;
            }
            srs_info("send PCUC(StreamEOF) message success.");
        }
    } else {
        if (true) {
            SrsOnStatusCallPacket* pkt = new SrsOnStatusCallPacket();
            pkt->data->set(StatusLevel, SrsAmf0Any::str(StatusLevelStatus));
            pkt->data->set(StatusCode, SrsAmf0Any::str(StatusCodeStreamUnpause));
            pkt->data->set(StatusDescription, SrsAmf0Any::str("Unpaused stream."));
            if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
                srs_error("send onStatus(NetStream.Unpause.Notify) message failed. ret=%d", ret);
                return ret;
            }
            srs_info("send onStatus(NetStream.Unpause.Notify) message success.");
        }
        if (true) {
            SrsUserControlPacket* pkt = new SrsUserControlPacket();
            pkt->event_type = SrcPCUCStreamBegin;
            pkt->event_data = stream_id;
            if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
                srs_error("send PCUC(StreanBegin) message failed. ret=%d", ret);
                return ret;
            }
            srs_info("send PCUC(StreanBegin) message success.");
        }
    }
    return ret;
} // SrsRtmpServer::on_play_client_pause

int SrsRtmpServer::start_fmle_publish(int stream_id)
{
    int ret = ERROR_SUCCESS;
    double fc_publish_tid = 0;

    if (true) {
        SrsCommonMessage* msg   = NULL;
        SrsFMLEStartPacket* pkt = NULL;
        if ((ret = expect_message<SrsFMLEStartPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
            srs_error("recv FCPublish message failed. ret=%d", ret);
            return ret;
        }
        srs_info("recv FCPublish request message success.");
        SrsAutoFree(SrsCommonMessage, msg);
        SrsAutoFree(SrsFMLEStartPacket, pkt);
        fc_publish_tid = pkt->transaction_id;
    }
    if (true) {
        SrsFMLEStartResPacket* pkt = new SrsFMLEStartResPacket(fc_publish_tid);
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send FCPublish response message failed. ret=%d", ret);
            return ret;
        }
        srs_info("send FCPublish response message success.");
    }
    double create_stream_tid = 0;
    if (true) {
        SrsCommonMessage* msg      = NULL;
        SrsCreateStreamPacket* pkt = NULL;
        if ((ret = expect_message<SrsCreateStreamPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
            srs_error("recv createStream message failed. ret=%d", ret);
            return ret;
        }
        srs_info("recv createStream request message success.");
        SrsAutoFree(SrsCommonMessage, msg);
        SrsAutoFree(SrsCreateStreamPacket, pkt);
        create_stream_tid = pkt->transaction_id;
    }
    if (true) {
        SrsCreateStreamResPacket* pkt = new SrsCreateStreamResPacket(create_stream_tid, stream_id);
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send createStream response message failed. ret=%d", ret);
            return ret;
        }
        srs_info("send createStream response message success.");
    }
    if (true) {
        SrsCommonMessage* msg = NULL;
        SrsPublishPacket* pkt = NULL;
        if ((ret = expect_message<SrsPublishPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
            srs_error("recv publish message failed. ret=%d", ret);
            return ret;
        }
        srs_info("recv publish request message success.");
        SrsAutoFree(SrsCommonMessage, msg);
        SrsAutoFree(SrsPublishPacket, pkt);
    }
    if (true) {
        SrsOnStatusCallPacket* pkt = new SrsOnStatusCallPacket();
        pkt->command_name = RTMP_AMF0_COMMAND_ON_FC_PUBLISH;
        pkt->data->set(StatusCode, SrsAmf0Any::str(StatusCodePublishStart));
        pkt->data->set(StatusDescription, SrsAmf0Any::str("Started publishing stream."));
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            srs_error("send onFCPublish(NetStream.Publish.Start) message failed. ret=%d", ret);
            return ret;
        }
        srs_info("send onFCPublish(NetStream.Publish.Start) message success.");
    }
    if (true) {
        SrsOnStatusCallPacket* pkt = new SrsOnStatusCallPacket();
        pkt->data->set(StatusLevel, SrsAmf0Any::str(StatusLevelStatus));
        pkt->data->set(StatusCode, SrsAmf0Any::str(StatusCodePublishStart));
        pkt->data->set(StatusDescription, SrsAmf0Any::str("Started publishing stream."));
        pkt->data->set(StatusClientId, SrsAmf0Any::str(RTMP_SIG_CLIENT_ID));
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            srs_error("send onStatus(NetStream.Publish.Start) message failed. ret=%d", ret);
            return ret;
        }
        srs_info("send onStatus(NetStream.Publish.Start) message success.");
    }
    srs_info("FMLE publish success.");
    return ret;
} // SrsRtmpServer::start_fmle_publish

int SrsRtmpServer::fmle_unpublish(int stream_id, double unpublish_tid)
{
    int ret = ERROR_SUCCESS;

    if (true) {
        SrsOnStatusCallPacket* pkt = new SrsOnStatusCallPacket();
        pkt->command_name = RTMP_AMF0_COMMAND_ON_FC_UNPUBLISH;
        pkt->data->set(StatusCode, SrsAmf0Any::str(StatusCodeUnpublishSuccess));
        pkt->data->set(StatusDescription, SrsAmf0Any::str("Stop publishing stream."));
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            if (!srs_is_system_control_error(ret) && !srs_is_client_gracefully_close(ret)) {
                srs_error("send onFCUnpublish(NetStream.unpublish.Success) message failed. ret=%d", ret);
            }
            return ret;
        }
        srs_info("send onFCUnpublish(NetStream.unpublish.Success) message success.");
    }
    if (true) {
        SrsFMLEStartResPacket* pkt = new SrsFMLEStartResPacket(unpublish_tid);
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            if (!srs_is_system_control_error(ret) && !srs_is_client_gracefully_close(ret)) {
                srs_error("send FCUnpublish response message failed. ret=%d", ret);
            }
            return ret;
        }
        srs_info("send FCUnpublish response message success.");
    }
    if (true) {
        SrsOnStatusCallPacket* pkt = new SrsOnStatusCallPacket();
        pkt->data->set(StatusLevel, SrsAmf0Any::str(StatusLevelStatus));
        pkt->data->set(StatusCode, SrsAmf0Any::str(StatusCodeUnpublishSuccess));
        pkt->data->set(StatusDescription, SrsAmf0Any::str("Stream is now unpublished"));
        pkt->data->set(StatusClientId, SrsAmf0Any::str(RTMP_SIG_CLIENT_ID));
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            if (!srs_is_system_control_error(ret) && !srs_is_client_gracefully_close(ret)) {
                srs_error("send onStatus(NetStream.Unpublish.Success) message failed. ret=%d", ret);
            }
            return ret;
        }
        srs_info("send onStatus(NetStream.Unpublish.Success) message success.");
    }
    srs_info("FMLE unpublish success.");
    return ret;
} // SrsRtmpServer::fmle_unpublish

int SrsRtmpServer::start_flash_publish(int stream_id)
{
    int ret = ERROR_SUCCESS;

    if (true) {
        SrsOnStatusCallPacket* pkt = new SrsOnStatusCallPacket();
        pkt->data->set(StatusLevel, SrsAmf0Any::str(StatusLevelStatus));
        pkt->data->set(StatusCode, SrsAmf0Any::str(StatusCodePublishStart));
        pkt->data->set(StatusDescription, SrsAmf0Any::str("Started publishing stream."));
        pkt->data->set(StatusClientId, SrsAmf0Any::str(RTMP_SIG_CLIENT_ID));
        if ((ret = protocol->send_and_free_packet(pkt, stream_id)) != ERROR_SUCCESS) {
            srs_error("send onStatus(NetStream.Publish.Start) message failed. ret=%d", ret);
            return ret;
        }
        srs_info("send onStatus(NetStream.Publish.Start) message success.");
    }
    srs_info("flash publish success.");
    return ret;
}

int SrsRtmpServer::identify_create_stream_client(SrsCreateStreamPacket* req, int stream_id, SrsRtmpConnType& type,
                                                 string& stream_name, double& duration)
{
    int ret = ERROR_SUCCESS;

    if (true) {
        SrsCreateStreamResPacket* pkt = new SrsCreateStreamResPacket(req->transaction_id, stream_id);
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send createStream response message failed. ret=%d", ret);
            return ret;
        }
        srs_info("send createStream response message success.");
    }
    while (true) {
        SrsCommonMessage* msg = NULL;
        if ((ret = protocol->recv_message(&msg)) != ERROR_SUCCESS) {
            if (!srs_is_client_gracefully_close(ret)) {
                srs_error("recv identify client message failed. ret=%d", ret);
            }
            return ret;
        }
        SrsAutoFree(SrsCommonMessage, msg);
        SrsMessageHeader& h = msg->header;
        if (h.is_ackledgement() || h.is_set_chunk_size() || h.is_window_ackledgement_size() ||
            h.is_user_control_message())
        {
            continue;
        }
        if (!h.is_amf0_command() && !h.is_amf3_command()) {
            srs_trace("identify ignore messages except "
                      "AMF0/AMF3 command message. type=%#x", h.message_type);
            continue;
        }
        SrsPacket* pkt = NULL;
        if ((ret = protocol->decode_message(msg, &pkt)) != ERROR_SUCCESS) {
            srs_error("identify decode message failed. ret=%d", ret);
            return ret;
        }
        SrsAutoFree(SrsPacket, pkt);
        if (dynamic_cast<SrsPlayPacket *>(pkt)) {
            srs_info("level1 identify client by play.");
            return identify_play_client(dynamic_cast<SrsPlayPacket *>(pkt), type, stream_name, duration);
        }
        if (dynamic_cast<SrsPublishPacket *>(pkt)) {
            srs_info("identify client by publish, falsh publish.");
            return identify_flash_publish_client(dynamic_cast<SrsPublishPacket *>(pkt), type, stream_name);
        }
        if (dynamic_cast<SrsCreateStreamPacket *>(pkt)) {
            srs_info("identify client by create stream, play or flash publish.");
            return identify_create_stream_client(dynamic_cast<SrsCreateStreamPacket *>(pkt), stream_id, type,
                                                 stream_name, duration);
        }
        srs_trace("ignore AMF0/AMF3 command message.");
    }
    return ret;
} // SrsRtmpServer::identify_create_stream_client

int SrsRtmpServer::identify_fmle_publish_client(SrsFMLEStartPacket* req, SrsRtmpConnType& type, string& stream_name)
{
    int ret = ERROR_SUCCESS;

    type        = SrsRtmpConnFMLEPublish;
    stream_name = req->stream_name;
    if (true) {
        SrsFMLEStartResPacket* pkt = new SrsFMLEStartResPacket(req->transaction_id);
        if ((ret = protocol->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send releaseStream response message failed. ret=%d", ret);
            return ret;
        }
        srs_info("send releaseStream response message success.");
    }
    return ret;
}

int SrsRtmpServer::identify_flash_publish_client(SrsPublishPacket* req, SrsRtmpConnType& type, string& stream_name)
{
    int ret = ERROR_SUCCESS;

    type        = SrsRtmpConnFlashPublish;
    stream_name = req->stream_name;
    return ret;
}

int SrsRtmpServer::identify_play_client(SrsPlayPacket* req, SrsRtmpConnType& type, string& stream_name,
                                        double& duration)
{
    int ret = ERROR_SUCCESS;

    type        = SrsRtmpConnPlay;
    stream_name = req->stream_name;
    duration    = req->duration;
    srs_info("identity client type=play, stream_name=%s, duration=%.2f", stream_name.c_str(), duration);
    return ret;
}

SrsConnectAppPacket::SrsConnectAppPacket()
{
    command_name   = RTMP_AMF0_COMMAND_CONNECT;
    transaction_id = 1;
    command_object = SrsAmf0Any::object();
    args = NULL;
}

SrsConnectAppPacket::~SrsConnectAppPacket()
{
    srs_freep(command_object);
    srs_freep(args);
}

int SrsConnectAppPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode connect command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_CONNECT) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 decode connect command_name failed. "
                  "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode connect transaction_id failed. ret=%d", ret);
        return ret;
    }
    if (transaction_id != 1.0) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_warn("amf0 decode connect transaction_id failed. "
                 "required=%.1f, actual=%.1f, ret=%d", 1.0, transaction_id, ret);
        ret = ERROR_SUCCESS;
    }
    if ((ret = command_object->read(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode connect command_object failed. ret=%d", ret);
        return ret;
    }
    if (!stream->empty()) {
        srs_freep(args);
        SrsAmf0Any* any = NULL;
        if ((ret = SrsAmf0Any::discovery(stream, &any)) != ERROR_SUCCESS) {
            srs_error("amf0 find connect args failed. ret=%d", ret);
            return ret;
        }
        srs_assert(any);
        if ((ret = any->read(stream)) != ERROR_SUCCESS) {
            srs_error("amf0 decode connect args failed. ret=%d", ret);
            srs_freep(any);
            return ret;
        }
        if (!any->is_object()) {
            srs_warn("drop the args, see: '4.1.1. connect', marker=%#x", any->marker);
            srs_freep(any);
        } else {
            args = any->to_object();
        }
    }
    srs_info("amf0 decode connect packet success");
    return ret;
} // SrsConnectAppPacket::decode

int SrsConnectAppPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int SrsConnectAppPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsConnectAppPacket::get_size()
{
    int size = 0;

    size += SrsAmf0Size::str(command_name);
    size += SrsAmf0Size::number();
    size += SrsAmf0Size::object(command_object);
    if (args) {
        size += SrsAmf0Size::object(args);
    }
    return size;
}

int SrsConnectAppPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if ((ret = command_object->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_object success.");
    if (args && (ret = args->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode args failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode args success.");
    srs_info("encode connect app request packet success.");
    return ret;
}

SrsConnectAppResPacket::SrsConnectAppResPacket()
{
    command_name   = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = 1;
    props = SrsAmf0Any::object();
    info  = SrsAmf0Any::object();
}

SrsConnectAppResPacket::~SrsConnectAppResPacket()
{
    srs_freep(props);
    srs_freep(info);
}

int SrsConnectAppResPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode connect command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_RESULT) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 decode connect command_name failed. "
                  "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode connect transaction_id failed. ret=%d", ret);
        return ret;
    }
    if (transaction_id != 1.0) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_warn("amf0 decode connect transaction_id failed. "
                 "required=%.1f, actual=%.1f, ret=%d", 1.0, transaction_id, ret);
        ret = ERROR_SUCCESS;
    }
    if (!stream->empty()) {
        SrsAmf0Any* p = NULL;
        if ((ret = srs_amf0_read_any(stream, &p)) != ERROR_SUCCESS) {
            srs_error("amf0 decode connect props failed. ret=%d", ret);
            return ret;
        }
        if (!p->is_object()) {
            srs_warn("ignore connect response props marker=%#x.", (u_int8_t) p->marker);
            srs_freep(p);
        } else {
            srs_freep(props);
            props = p->to_object();
            srs_info("accept amf0 object connect response props");
        }
    }
    if ((ret = info->read(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode connect info failed. ret=%d", ret);
        return ret;
    }
    srs_info("amf0 decode connect response packet success");
    return ret;
} // SrsConnectAppResPacket::decode

int SrsConnectAppResPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int SrsConnectAppResPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsConnectAppResPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
           + SrsAmf0Size::object(props) + SrsAmf0Size::object(info);
}

int SrsConnectAppResPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if ((ret = props->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode props failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode props success.");
    if ((ret = info->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode info failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode info success.");
    srs_info("encode connect app response packet success.");
    return ret;
}

SrsCallPacket::SrsCallPacket()
{
    command_name   = "";
    transaction_id = 0;
    command_object = NULL;
    arguments      = NULL;
}

SrsCallPacket::~SrsCallPacket()
{
    srs_freep(command_object);
    srs_freep(arguments);
}

int SrsCallPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode call command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty()) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 decode call command_name failed. "
                  "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode call transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_freep(command_object);
    if ((ret = SrsAmf0Any::discovery(stream, &command_object)) != ERROR_SUCCESS) {
        srs_error("amf0 discovery call command_object failed. ret=%d", ret);
        return ret;
    }
    if ((ret = command_object->read(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode call command_object failed. ret=%d", ret);
        return ret;
    }
    if (!stream->empty()) {
        srs_freep(arguments);
        if ((ret = SrsAmf0Any::discovery(stream, &arguments)) != ERROR_SUCCESS) {
            srs_error("amf0 discovery call arguments failed. ret=%d", ret);
            return ret;
        }
        if ((ret = arguments->read(stream)) != ERROR_SUCCESS) {
            srs_error("amf0 decode call arguments failed. ret=%d", ret);
            return ret;
        }
    }
    srs_info("amf0 decode call packet success");
    return ret;
} // SrsCallPacket::decode

int SrsCallPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int SrsCallPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsCallPacket::get_size()
{
    int size = 0;

    size += SrsAmf0Size::str(command_name) + SrsAmf0Size::number();
    if (command_object) {
        size += command_object->total_size();
    }
    if (arguments) {
        size += arguments->total_size();
    }
    return size;
}

int SrsCallPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if (command_object && (ret = command_object->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_object success.");
    if (arguments && (ret = arguments->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode arguments failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode arguments success.");
    srs_info("encode create stream request packet success.");
    return ret;
}

SrsCallResPacket::SrsCallResPacket(double _transaction_id)
{
    command_name   = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = _transaction_id;
    command_object = NULL;
    response       = NULL;
}

SrsCallResPacket::~SrsCallResPacket()
{
    srs_freep(command_object);
    srs_freep(response);
}

int SrsCallResPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int SrsCallResPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsCallResPacket::get_size()
{
    int size = 0;

    size += SrsAmf0Size::str(command_name) + SrsAmf0Size::number();
    if (command_object) {
        size += command_object->total_size();
    }
    if (response) {
        size += response->total_size();
    }
    return size;
}

int SrsCallResPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if (command_object && (ret = command_object->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_object success.");
    if (response && (ret = response->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode response failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode response success.");
    srs_info("encode call response packet success.");
    return ret;
}

SrsCreateStreamPacket::SrsCreateStreamPacket()
{
    command_name   = RTMP_AMF0_COMMAND_CREATE_STREAM;
    transaction_id = 2;
    command_object = SrsAmf0Any::null();
}

SrsCreateStreamPacket::~SrsCreateStreamPacket()
{
    srs_freep(command_object);
}

int SrsCreateStreamPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode createStream command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_CREATE_STREAM) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 decode createStream command_name failed. "
                  "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode createStream transaction_id failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode createStream command_object failed. ret=%d", ret);
        return ret;
    }
    srs_info("amf0 decode createStream packet success");
    return ret;
}

int SrsCreateStreamPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int SrsCreateStreamPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsCreateStreamPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
           + SrsAmf0Size::null();
}

int SrsCreateStreamPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if ((ret = srs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        srs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_object success.");
    srs_info("encode create stream request packet success.");
    return ret;
}

SrsCreateStreamResPacket::SrsCreateStreamResPacket(double _transaction_id, double _stream_id)
{
    command_name   = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = _transaction_id;
    command_object = SrsAmf0Any::null();
    stream_id      = _stream_id;
}

SrsCreateStreamResPacket::~SrsCreateStreamResPacket()
{
    srs_freep(command_object);
}

int SrsCreateStreamResPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode createStream command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_RESULT) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 decode createStream command_name failed. "
                  "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode createStream transaction_id failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode createStream command_object failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, stream_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode createStream stream_id failed. ret=%d", ret);
        return ret;
    }
    srs_info("amf0 decode createStream response packet success");
    return ret;
}

int SrsCreateStreamResPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int SrsCreateStreamResPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsCreateStreamResPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
           + SrsAmf0Size::null() + SrsAmf0Size::number();
}

int SrsCreateStreamResPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if ((ret = srs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        srs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_object success.");
    if ((ret = srs_amf0_write_number(stream, stream_id)) != ERROR_SUCCESS) {
        srs_error("encode stream_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode stream_id success.");
    srs_info("encode createStream response packet success.");
    return ret;
}

SrsCloseStreamPacket::SrsCloseStreamPacket()
{
    command_name   = RTMP_AMF0_COMMAND_CLOSE_STREAM;
    transaction_id = 0;
    command_object = SrsAmf0Any::null();
}

SrsCloseStreamPacket::~SrsCloseStreamPacket()
{
    srs_freep(command_object);
}

int SrsCloseStreamPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode closeStream command_name failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode closeStream transaction_id failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode closeStream command_object failed. ret=%d", ret);
        return ret;
    }
    srs_info("amf0 decode closeStream packet success");
    return ret;
}

SrsFMLEStartPacket::SrsFMLEStartPacket()
{
    command_name   = RTMP_AMF0_COMMAND_RELEASE_STREAM;
    transaction_id = 0;
    command_object = SrsAmf0Any::null();
}

SrsFMLEStartPacket::~SrsFMLEStartPacket()
{
    srs_freep(command_object);
}

int SrsFMLEStartPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode FMLE start command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() ||
        (command_name != RTMP_AMF0_COMMAND_RELEASE_STREAM &&
         command_name != RTMP_AMF0_COMMAND_FC_PUBLISH &&
         command_name != RTMP_AMF0_COMMAND_UNPUBLISH)
    )
    {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 decode FMLE start command_name failed. "
                  "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode FMLE start transaction_id failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode FMLE start command_object failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_string(stream, stream_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode FMLE start stream_name failed. ret=%d", ret);
        return ret;
    }
    srs_info("amf0 decode FMLE start packet success");
    return ret;
} // SrsFMLEStartPacket::decode

int SrsFMLEStartPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int SrsFMLEStartPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsFMLEStartPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
           + SrsAmf0Size::null() + SrsAmf0Size::str(stream_name);
}

int SrsFMLEStartPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if ((ret = srs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        srs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_object success.");
    if ((ret = srs_amf0_write_string(stream, stream_name)) != ERROR_SUCCESS) {
        srs_error("encode stream_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode stream_name success.");
    srs_info("encode FMLE start response packet success.");
    return ret;
}

SrsFMLEStartPacket * SrsFMLEStartPacket::create_release_stream(string stream)
{
    SrsFMLEStartPacket* pkt = new SrsFMLEStartPacket();

    pkt->command_name   = RTMP_AMF0_COMMAND_RELEASE_STREAM;
    pkt->transaction_id = 2;
    pkt->stream_name    = stream;
    return pkt;
}

SrsFMLEStartPacket * SrsFMLEStartPacket::create_FC_publish(string stream)
{
    SrsFMLEStartPacket* pkt = new SrsFMLEStartPacket();

    pkt->command_name   = RTMP_AMF0_COMMAND_FC_PUBLISH;
    pkt->transaction_id = 3;
    pkt->stream_name    = stream;
    return pkt;
}

SrsFMLEStartResPacket::SrsFMLEStartResPacket(double _transaction_id)
{
    command_name   = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = _transaction_id;
    command_object = SrsAmf0Any::null();
    args = SrsAmf0Any::undefined();
}

SrsFMLEStartResPacket::~SrsFMLEStartResPacket()
{
    srs_freep(command_object);
    srs_freep(args);
}

int SrsFMLEStartResPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode FMLE start response command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_RESULT) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 decode FMLE start response command_name failed. "
                  "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode FMLE start response transaction_id failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode FMLE start response command_object failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_undefined(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode FMLE start response stream_id failed. ret=%d", ret);
        return ret;
    }
    srs_info("amf0 decode FMLE start packet success");
    return ret;
}

int SrsFMLEStartResPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int SrsFMLEStartResPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsFMLEStartResPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
           + SrsAmf0Size::null() + SrsAmf0Size::undefined();
}

int SrsFMLEStartResPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if ((ret = srs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        srs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_object success.");
    if ((ret = srs_amf0_write_undefined(stream)) != ERROR_SUCCESS) {
        srs_error("encode args failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode args success.");
    srs_info("encode FMLE start response packet success.");
    return ret;
}

SrsPublishPacket::SrsPublishPacket()
{
    command_name   = RTMP_AMF0_COMMAND_PUBLISH;
    transaction_id = 0;
    command_object = SrsAmf0Any::null();
    type = "live";
}

SrsPublishPacket::~SrsPublishPacket()
{
    srs_freep(command_object);
}

int SrsPublishPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode publish command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_PUBLISH) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 decode publish command_name failed. "
                  "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode publish transaction_id failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode publish command_object failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_string(stream, stream_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode publish stream_name failed. ret=%d", ret);
        return ret;
    }
    if (!stream->empty() && (ret = srs_amf0_read_string(stream, type)) != ERROR_SUCCESS) {
        srs_error("amf0 decode publish type failed. ret=%d", ret);
        return ret;
    }
    srs_info("amf0 decode publish packet success");
    return ret;
}

int SrsPublishPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int SrsPublishPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsPublishPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
           + SrsAmf0Size::null() + SrsAmf0Size::str(stream_name)
           + SrsAmf0Size::str(type);
}

int SrsPublishPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if ((ret = srs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        srs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_object success.");
    if ((ret = srs_amf0_write_string(stream, stream_name)) != ERROR_SUCCESS) {
        srs_error("encode stream_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode stream_name success.");
    if ((ret = srs_amf0_write_string(stream, type)) != ERROR_SUCCESS) {
        srs_error("encode type failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode type success.");
    srs_info("encode play request packet success.");
    return ret;
}

SrsPausePacket::SrsPausePacket()
{
    command_name   = RTMP_AMF0_COMMAND_PAUSE;
    transaction_id = 0;
    command_object = SrsAmf0Any::null();
    time_ms        = 0;
    is_pause       = true;
}

SrsPausePacket::~SrsPausePacket()
{
    srs_freep(command_object);
}

int SrsPausePacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode pause command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_PAUSE) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 decode pause command_name failed. "
                  "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode pause transaction_id failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode pause command_object failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_boolean(stream, is_pause)) != ERROR_SUCCESS) {
        srs_error("amf0 decode pause is_pause failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, time_ms)) != ERROR_SUCCESS) {
        srs_error("amf0 decode pause time_ms failed. ret=%d", ret);
        return ret;
    }
    srs_info("amf0 decode pause packet success");
    return ret;
}

SrsPlayPacket::SrsPlayPacket()
{
    command_name   = RTMP_AMF0_COMMAND_PLAY;
    transaction_id = 0;
    command_object = SrsAmf0Any::null();
    start    = -2;
    duration = -1;
    reset    = true;
}

SrsPlayPacket::~SrsPlayPacket()
{
    srs_freep(command_object);
}

int SrsPlayPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode play command_name failed. ret=%d", ret);
        return ret;
    }
    if (command_name.empty() || command_name != RTMP_AMF0_COMMAND_PLAY) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 decode play command_name failed. "
                  "command_name=%s, ret=%d", command_name.c_str(), ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode play transaction_id failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode play command_object failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_string(stream, stream_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode play stream_name failed. ret=%d", ret);
        return ret;
    }
    if (!stream->empty() && (ret = srs_amf0_read_number(stream, start)) != ERROR_SUCCESS) {
        srs_error("amf0 decode play start failed. ret=%d", ret);
        return ret;
    }
    if (!stream->empty() && (ret = srs_amf0_read_number(stream, duration)) != ERROR_SUCCESS) {
        srs_error("amf0 decode play duration failed. ret=%d", ret);
        return ret;
    }
    if (stream->empty()) {
        return ret;
    }
    SrsAmf0Any* reset_value = NULL;
    if ((ret = srs_amf0_read_any(stream, &reset_value)) != ERROR_SUCCESS) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("amf0 read play reset marker failed. ret=%d", ret);
        return ret;
    }
    SrsAutoFree(SrsAmf0Any, reset_value);
    if (reset_value) {
        if (reset_value->is_boolean()) {
            reset = reset_value->to_boolean();
        } else if (reset_value->is_number()) {
            reset = (reset_value->to_number() != 0);
        } else {
            ret = ERROR_RTMP_AMF0_DECODE;
            srs_error("amf0 invalid type=%#x, requires number or bool, ret=%d", reset_value->marker, ret);
            return ret;
        }
    }
    srs_info("amf0 decode play packet success");
    return ret;
} // SrsPlayPacket::decode

int SrsPlayPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int SrsPlayPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsPlayPacket::get_size()
{
    int size = SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
               + SrsAmf0Size::null() + SrsAmf0Size::str(stream_name);

    if (start != -2 || duration != -1 || !reset) {
        size += SrsAmf0Size::number();
    }
    if (duration != -1 || !reset) {
        size += SrsAmf0Size::number();
    }
    if (!reset) {
        size += SrsAmf0Size::boolean();
    }
    return size;
}

int SrsPlayPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if ((ret = srs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        srs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_object success.");
    if ((ret = srs_amf0_write_string(stream, stream_name)) != ERROR_SUCCESS) {
        srs_error("encode stream_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode stream_name success.");
    if ((start != -2 || duration != -1 || !reset) && (ret = srs_amf0_write_number(stream, start)) != ERROR_SUCCESS) {
        srs_error("encode start failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode start success.");
    if ((duration != -1 || !reset) && (ret = srs_amf0_write_number(stream, duration)) != ERROR_SUCCESS) {
        srs_error("encode duration failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode duration success.");
    if (!reset && (ret = srs_amf0_write_boolean(stream, reset)) != ERROR_SUCCESS) {
        srs_error("encode reset failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode reset success.");
    srs_info("encode play request packet success.");
    return ret;
} // SrsPlayPacket::encode_packet

SrsPlayResPacket::SrsPlayResPacket()
{
    command_name   = RTMP_AMF0_COMMAND_RESULT;
    transaction_id = 0;
    command_object = SrsAmf0Any::null();
    desc = SrsAmf0Any::object();
}

SrsPlayResPacket::~SrsPlayResPacket()
{
    srs_freep(command_object);
    srs_freep(desc);
}

int SrsPlayResPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int SrsPlayResPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsPlayResPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
           + SrsAmf0Size::null() + SrsAmf0Size::object(desc);
}

int SrsPlayResPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if ((ret = srs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        srs_error("encode command_object failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_object success.");
    if ((ret = desc->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode desc failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode desc success.");
    srs_info("encode play response packet success.");
    return ret;
}

SrsOnBWDonePacket::SrsOnBWDonePacket()
{
    command_name   = RTMP_AMF0_COMMAND_ON_BW_DONE;
    transaction_id = 0;
    args = SrsAmf0Any::null();
}

SrsOnBWDonePacket::~SrsOnBWDonePacket()
{
    srs_freep(args);
}

int SrsOnBWDonePacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection;
}

int SrsOnBWDonePacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsOnBWDonePacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
           + SrsAmf0Size::null();
}

int SrsOnBWDonePacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if ((ret = srs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        srs_error("encode args failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode args success.");
    srs_info("encode onBWDone packet success.");
    return ret;
}

SrsOnStatusCallPacket::SrsOnStatusCallPacket()
{
    command_name   = RTMP_AMF0_COMMAND_ON_STATUS;
    transaction_id = 0;
    args = SrsAmf0Any::null();
    data = SrsAmf0Any::object();
}

SrsOnStatusCallPacket::~SrsOnStatusCallPacket()
{
    srs_freep(args);
    srs_freep(data);
}

int SrsOnStatusCallPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int SrsOnStatusCallPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsOnStatusCallPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
           + SrsAmf0Size::null() + SrsAmf0Size::object(data);
}

int SrsOnStatusCallPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if ((ret = srs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        srs_error("encode args failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode args success.");
    ;
    if ((ret = data->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode data failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode data success.");
    srs_info("encode onStatus(Call) packet success.");
    return ret;
}

SrsBandwidthPacket::SrsBandwidthPacket()
{
    command_name   = RTMP_AMF0_COMMAND_ON_STATUS;
    transaction_id = 0;
    args = SrsAmf0Any::null();
    data = SrsAmf0Any::object();
}

SrsBandwidthPacket::~SrsBandwidthPacket()
{
    srs_freep(args);
    srs_freep(data);
}

int SrsBandwidthPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("amf0 decode bwtc command_name failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("amf0 decode bwtc transaction_id failed. ret=%d", ret);
        return ret;
    }
    if ((ret = srs_amf0_read_null(stream)) != ERROR_SUCCESS) {
        srs_error("amf0 decode bwtc command_object failed. ret=%d", ret);
        return ret;
    }
    if (is_stop_play() || is_start_publish() || is_finish()) {
        if ((ret = data->read(stream)) != ERROR_SUCCESS) {
            srs_error("amf0 decode bwtc command_object failed. ret=%d", ret);
            return ret;
        }
    }
    srs_info("decode SrsBandwidthPacket success.");
    return ret;
}

int SrsBandwidthPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int SrsBandwidthPacket::get_message_type()
{
    return RTMP_MSG_AMF0CommandMessage;
}

int SrsBandwidthPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::number()
           + SrsAmf0Size::null() + SrsAmf0Size::object(data);
}

int SrsBandwidthPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_number(stream, transaction_id)) != ERROR_SUCCESS) {
        srs_error("encode transaction_id failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode transaction_id success.");
    if ((ret = srs_amf0_write_null(stream)) != ERROR_SUCCESS) {
        srs_error("encode args failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode args success.");
    ;
    if ((ret = data->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode data failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode data success.");
    srs_info("encode onStatus(Call) packet success.");
    return ret;
}

bool SrsBandwidthPacket::is_start_play()
{
    return command_name == SRS_BW_CHECK_START_PLAY;
}

bool SrsBandwidthPacket::is_starting_play()
{
    return command_name == SRS_BW_CHECK_STARTING_PLAY;
}

bool SrsBandwidthPacket::is_stop_play()
{
    return command_name == SRS_BW_CHECK_STOP_PLAY;
}

bool SrsBandwidthPacket::is_stopped_play()
{
    return command_name == SRS_BW_CHECK_STOPPED_PLAY;
}

bool SrsBandwidthPacket::is_start_publish()
{
    return command_name == SRS_BW_CHECK_START_PUBLISH;
}

bool SrsBandwidthPacket::is_starting_publish()
{
    return command_name == SRS_BW_CHECK_STARTING_PUBLISH;
}

bool SrsBandwidthPacket::is_stop_publish()
{
    return command_name == SRS_BW_CHECK_STOP_PUBLISH;
}

bool SrsBandwidthPacket::is_stopped_publish()
{
    return command_name == SRS_BW_CHECK_STOPPED_PUBLISH;
}

bool SrsBandwidthPacket::is_finish()
{
    return command_name == SRS_BW_CHECK_FINISHED;
}

bool SrsBandwidthPacket::is_final()
{
    return command_name == SRS_BW_CHECK_FINAL;
}

SrsBandwidthPacket * SrsBandwidthPacket::create_start_play()
{
    SrsBandwidthPacket* pkt = new SrsBandwidthPacket();

    return pkt->set_command(SRS_BW_CHECK_START_PLAY);
}

SrsBandwidthPacket * SrsBandwidthPacket::create_starting_play()
{
    SrsBandwidthPacket* pkt = new SrsBandwidthPacket();

    return pkt->set_command(SRS_BW_CHECK_STARTING_PLAY);
}

SrsBandwidthPacket * SrsBandwidthPacket::create_playing()
{
    SrsBandwidthPacket* pkt = new SrsBandwidthPacket();

    return pkt->set_command(SRS_BW_CHECK_PLAYING);
}

SrsBandwidthPacket * SrsBandwidthPacket::create_stop_play()
{
    SrsBandwidthPacket* pkt = new SrsBandwidthPacket();

    return pkt->set_command(SRS_BW_CHECK_STOP_PLAY);
}

SrsBandwidthPacket * SrsBandwidthPacket::create_stopped_play()
{
    SrsBandwidthPacket* pkt = new SrsBandwidthPacket();

    return pkt->set_command(SRS_BW_CHECK_STOPPED_PLAY);
}

SrsBandwidthPacket * SrsBandwidthPacket::create_start_publish()
{
    SrsBandwidthPacket* pkt = new SrsBandwidthPacket();

    return pkt->set_command(SRS_BW_CHECK_START_PUBLISH);
}

SrsBandwidthPacket * SrsBandwidthPacket::create_starting_publish()
{
    SrsBandwidthPacket* pkt = new SrsBandwidthPacket();

    return pkt->set_command(SRS_BW_CHECK_STARTING_PUBLISH);
}

SrsBandwidthPacket * SrsBandwidthPacket::create_publishing()
{
    SrsBandwidthPacket* pkt = new SrsBandwidthPacket();

    return pkt->set_command(SRS_BW_CHECK_PUBLISHING);
}

SrsBandwidthPacket * SrsBandwidthPacket::create_stop_publish()
{
    SrsBandwidthPacket* pkt = new SrsBandwidthPacket();

    return pkt->set_command(SRS_BW_CHECK_STOP_PUBLISH);
}

SrsBandwidthPacket * SrsBandwidthPacket::create_stopped_publish()
{
    SrsBandwidthPacket* pkt = new SrsBandwidthPacket();

    return pkt->set_command(SRS_BW_CHECK_STOPPED_PUBLISH);
}

SrsBandwidthPacket * SrsBandwidthPacket::create_finish()
{
    SrsBandwidthPacket* pkt = new SrsBandwidthPacket();

    return pkt->set_command(SRS_BW_CHECK_FINISHED);
}

SrsBandwidthPacket * SrsBandwidthPacket::create_final()
{
    SrsBandwidthPacket* pkt = new SrsBandwidthPacket();

    return pkt->set_command(SRS_BW_CHECK_FINAL);
}

SrsBandwidthPacket * SrsBandwidthPacket::set_command(string command)
{
    command_name = command;
    return this;
}

SrsOnStatusDataPacket::SrsOnStatusDataPacket()
{
    command_name = RTMP_AMF0_COMMAND_ON_STATUS;
    data         = SrsAmf0Any::object();
}

SrsOnStatusDataPacket::~SrsOnStatusDataPacket()
{
    srs_freep(data);
}

int SrsOnStatusDataPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int SrsOnStatusDataPacket::get_message_type()
{
    return RTMP_MSG_AMF0DataMessage;
}

int SrsOnStatusDataPacket::get_size()
{
    return SrsAmf0Size::str(command_name) + SrsAmf0Size::object(data);
}

int SrsOnStatusDataPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = data->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode data failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode data success.");
    srs_info("encode onStatus(Data) packet success.");
    return ret;
}

SrsSampleAccessPacket::SrsSampleAccessPacket()
{
    command_name        = RTMP_AMF0_DATA_SAMPLE_ACCESS;
    video_sample_access = false;
    audio_sample_access = false;
}

SrsSampleAccessPacket::~SrsSampleAccessPacket()
{}

int SrsSampleAccessPacket::get_prefer_cid()
{
    return RTMP_CID_OverStream;
}

int SrsSampleAccessPacket::get_message_type()
{
    return RTMP_MSG_AMF0DataMessage;
}

int SrsSampleAccessPacket::get_size()
{
    return SrsAmf0Size::str(command_name)
           + SrsAmf0Size::boolean() + SrsAmf0Size::boolean();
}

int SrsSampleAccessPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, command_name)) != ERROR_SUCCESS) {
        srs_error("encode command_name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode command_name success.");
    if ((ret = srs_amf0_write_boolean(stream, video_sample_access)) != ERROR_SUCCESS) {
        srs_error("encode video_sample_access failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode video_sample_access success.");
    if ((ret = srs_amf0_write_boolean(stream, audio_sample_access)) != ERROR_SUCCESS) {
        srs_error("encode audio_sample_access failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode audio_sample_access success.");
    ;
    srs_info("encode |RtmpSampleAccess packet success.");
    return ret;
}

SrsOnMetaDataPacket::SrsOnMetaDataPacket()
{
    name     = SRS_CONSTS_RTMP_ON_METADATA;
    metadata = SrsAmf0Any::object();
}

SrsOnMetaDataPacket::~SrsOnMetaDataPacket()
{
    srs_freep(metadata);
}

int SrsOnMetaDataPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_read_string(stream, name)) != ERROR_SUCCESS) {
        srs_error("decode metadata name failed. ret=%d", ret);
        return ret;
    }
    if (name == SRS_CONSTS_RTMP_SET_DATAFRAME) {
        if ((ret = srs_amf0_read_string(stream, name)) != ERROR_SUCCESS) {
            srs_error("decode metadata name failed. ret=%d", ret);
            return ret;
        }
    }
    srs_verbose("decode metadata name success. name=%s", name.c_str());
    SrsAmf0Any* any = NULL;
    if ((ret = srs_amf0_read_any(stream, &any)) != ERROR_SUCCESS) {
        srs_error("decode metadata metadata failed. ret=%d", ret);
        return ret;
    }
    srs_assert(any);
    if (any->is_object()) {
        srs_freep(metadata);
        metadata = any->to_object();
        srs_info("decode metadata object success");
        return ret;
    }
    SrsAutoFree(SrsAmf0Any, any);
    if (any->is_ecma_array()) {
        SrsAmf0EcmaArray* arr = any->to_ecma_array();
        for (int i = 0; i < arr->count(); i++) {
            metadata->set(arr->key_at(i), arr->value_at(i)->copy());
        }
        srs_info("decode metadata array success");
    }
    return ret;
} // SrsOnMetaDataPacket::decode

int SrsOnMetaDataPacket::get_prefer_cid()
{
    return RTMP_CID_OverConnection2;
}

int SrsOnMetaDataPacket::get_message_type()
{
    return RTMP_MSG_AMF0DataMessage;
}

int SrsOnMetaDataPacket::get_size()
{
    return SrsAmf0Size::str(name) + SrsAmf0Size::object(metadata);
}

int SrsOnMetaDataPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_amf0_write_string(stream, name)) != ERROR_SUCCESS) {
        srs_error("encode name failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode name success.");
    if ((ret = metadata->write(stream)) != ERROR_SUCCESS) {
        srs_error("encode metadata failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("encode metadata success.");
    srs_info("encode onMetaData packet success.");
    return ret;
}

SrsSetWindowAckSizePacket::SrsSetWindowAckSizePacket()
{
    ackowledgement_window_size = 0;
}

SrsSetWindowAckSizePacket::~SrsSetWindowAckSizePacket()
{}

int SrsSetWindowAckSizePacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(4)) {
        ret = ERROR_RTMP_MESSAGE_DECODE;
        srs_error("decode ack window size failed. ret=%d", ret);
        return ret;
    }
    ackowledgement_window_size = stream->read_4bytes();
    srs_info("decode ack window size success");
    return ret;
}

int SrsSetWindowAckSizePacket::get_prefer_cid()
{
    return RTMP_CID_ProtocolControl;
}

int SrsSetWindowAckSizePacket::get_message_type()
{
    return RTMP_MSG_WindowAcknowledgementSize;
}

int SrsSetWindowAckSizePacket::get_size()
{
    return 4;
}

int SrsSetWindowAckSizePacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(4)) {
        ret = ERROR_RTMP_MESSAGE_ENCODE;
        srs_error("encode ack size packet failed. ret=%d", ret);
        return ret;
    }
    stream->write_4bytes(ackowledgement_window_size);
    srs_verbose("encode ack size packet "
                "success. ack_size=%d", ackowledgement_window_size);
    return ret;
}

SrsAcknowledgementPacket::SrsAcknowledgementPacket()
{
    sequence_number = 0;
}

SrsAcknowledgementPacket::~SrsAcknowledgementPacket()
{}

int SrsAcknowledgementPacket::get_prefer_cid()
{
    return RTMP_CID_ProtocolControl;
}

int SrsAcknowledgementPacket::get_message_type()
{
    return RTMP_MSG_Acknowledgement;
}

int SrsAcknowledgementPacket::get_size()
{
    return 4;
}

int SrsAcknowledgementPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(4)) {
        ret = ERROR_RTMP_MESSAGE_ENCODE;
        srs_error("encode acknowledgement packet failed. ret=%d", ret);
        return ret;
    }
    stream->write_4bytes(sequence_number);
    srs_verbose("encode acknowledgement packet "
                "success. sequence_number=%d", sequence_number);
    return ret;
}

SrsSetChunkSizePacket::SrsSetChunkSizePacket()
{
    chunk_size = SRS_CONSTS_RTMP_PROTOCOL_CHUNK_SIZE;
}

SrsSetChunkSizePacket::~SrsSetChunkSizePacket()
{}

int SrsSetChunkSizePacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(4)) {
        ret = ERROR_RTMP_MESSAGE_DECODE;
        srs_error("decode chunk size failed. ret=%d", ret);
        return ret;
    }
    chunk_size = stream->read_4bytes();
    srs_info("decode chunk size success. chunk_size=%d", chunk_size);
    return ret;
}

int SrsSetChunkSizePacket::get_prefer_cid()
{
    return RTMP_CID_ProtocolControl;
}

int SrsSetChunkSizePacket::get_message_type()
{
    return RTMP_MSG_SetChunkSize;
}

int SrsSetChunkSizePacket::get_size()
{
    return 4;
}

int SrsSetChunkSizePacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(4)) {
        ret = ERROR_RTMP_MESSAGE_ENCODE;
        srs_error("encode chunk packet failed. ret=%d", ret);
        return ret;
    }
    stream->write_4bytes(chunk_size);
    srs_verbose("encode chunk packet success. ack_size=%d", chunk_size);
    return ret;
}

SrsSetPeerBandwidthPacket::SrsSetPeerBandwidthPacket()
{
    bandwidth = 0;
    type      = SrsPeerBandwidthDynamic;
}

SrsSetPeerBandwidthPacket::~SrsSetPeerBandwidthPacket()
{}

int SrsSetPeerBandwidthPacket::get_prefer_cid()
{
    return RTMP_CID_ProtocolControl;
}

int SrsSetPeerBandwidthPacket::get_message_type()
{
    return RTMP_MSG_SetPeerBandwidth;
}

int SrsSetPeerBandwidthPacket::get_size()
{
    return 5;
}

int SrsSetPeerBandwidthPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(5)) {
        ret = ERROR_RTMP_MESSAGE_ENCODE;
        srs_error("encode set bandwidth packet failed. ret=%d", ret);
        return ret;
    }
    stream->write_4bytes(bandwidth);
    stream->write_1bytes(type);
    srs_verbose("encode set bandwidth packet "
                "success. bandwidth=%d, type=%d", bandwidth, type);
    return ret;
}

SrsUserControlPacket::SrsUserControlPacket()
{
    event_type = 0;
    event_data = 0;
    extra_data = 0;
}

SrsUserControlPacket::~SrsUserControlPacket()
{}

int SrsUserControlPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(2)) {
        ret = ERROR_RTMP_MESSAGE_DECODE;
        srs_error("decode user control failed. ret=%d", ret);
        return ret;
    }
    event_type = stream->read_2bytes();
    if (event_type == SrsPCUCFmsEvent0) {
        if (!stream->require(1)) {
            ret = ERROR_RTMP_MESSAGE_DECODE;
            srs_error("decode user control failed. ret=%d", ret);
            return ret;
        }
        event_data = stream->read_1bytes();
    } else {
        if (!stream->require(4)) {
            ret = ERROR_RTMP_MESSAGE_DECODE;
            srs_error("decode user control failed. ret=%d", ret);
            return ret;
        }
        event_data = stream->read_4bytes();
    }
    if (event_type == SrcPCUCSetBufferLength) {
        if (!stream->require(4)) {
            ret = ERROR_RTMP_MESSAGE_ENCODE;
            srs_error("decode user control packet failed. ret=%d", ret);
            return ret;
        }
        extra_data = stream->read_4bytes();
    }
    srs_info("decode user control success. "
             "event_type=%d, event_data=%d, extra_data=%d",
             event_type, event_data, extra_data);
    return ret;
} // SrsUserControlPacket::decode

int SrsUserControlPacket::get_prefer_cid()
{
    return RTMP_CID_ProtocolControl;
}

int SrsUserControlPacket::get_message_type()
{
    return RTMP_MSG_UserControlMessage;
}

int SrsUserControlPacket::get_size()
{
    int size = 2;

    if (event_type == SrsPCUCFmsEvent0) {
        size += 1;
    } else {
        size += 4;
    }
    if (event_type == SrcPCUCSetBufferLength) {
        size += 4;
    }
    return size;
}

int SrsUserControlPacket::encode_packet(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(get_size())) {
        ret = ERROR_RTMP_MESSAGE_ENCODE;
        srs_error("encode user control packet failed. ret=%d", ret);
        return ret;
    }
    stream->write_2bytes(event_type);
    if (event_type == SrsPCUCFmsEvent0) {
        stream->write_1bytes(event_data);
    } else {
        stream->write_4bytes(event_data);
    }
    if (event_type == SrcPCUCSetBufferLength) {
        stream->write_4bytes(extra_data);
        srs_verbose("user control message, buffer_length=%d", extra_data);
    }
    srs_verbose("encode user control packet success. "
                "event_type=%d, event_data=%d", event_type, event_data);
    return ret;
}

#include <time.h>
#ifdef SRS_AUTO_SSL
using namespace _srs_internal;
# include <openssl/evp.h>
# include <openssl/hmac.h>
# include <openssl/dh.h>
namespace _srs_internal
{
u_int8_t SrsGenuineFMSKey[] = {
    0x47, 0x65, 0x6e, 0x75, 0x69, 0x6e, 0x65, 0x20,
    0x41, 0x64, 0x6f, 0x62, 0x65, 0x20, 0x46, 0x6c,
    0x61, 0x73, 0x68, 0x20, 0x4d, 0x65, 0x64, 0x69,
    0x61, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72,
    0x20, 0x30, 0x30, 0x31,
    0xf0, 0xee, 0xc2, 0x4a, 0x80, 0x68, 0xbe, 0xe8,
    0x2e, 0x00, 0xd0, 0xd1, 0x02, 0x9e, 0x7e, 0x57,
    0x6e, 0xec, 0x5d, 0x2d, 0x29, 0x80, 0x6f, 0xab,
    0x93, 0xb8, 0xe6, 0x36, 0xcf, 0xeb, 0x31, 0xae
};
u_int8_t SrsGenuineFPKey[] = {
    0x47, 0x65, 0x6E, 0x75, 0x69, 0x6E, 0x65, 0x20,
    0x41, 0x64, 0x6F, 0x62, 0x65, 0x20, 0x46, 0x6C,
    0x61, 0x73, 0x68, 0x20, 0x50, 0x6C, 0x61, 0x79,
    0x65, 0x72, 0x20, 0x30, 0x30, 0x31,
    0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8,
    0x2E, 0x00, 0xD0, 0xD1, 0x02, 0x9E, 0x7E, 0x57,
    0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
    0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
};
int do_openssl_HMACsha256(HMAC_CTX* ctx, const void* data, int data_size, void* digest, unsigned int* digest_size)
{
    int ret = ERROR_SUCCESS;

    if (HMAC_Update(ctx, (unsigned char *) data, data_size) < 0) {
        ret = ERROR_OpenSslSha256Update;
        return ret;
    }
    if (HMAC_Final(ctx, (unsigned char *) digest, digest_size) < 0) {
        ret = ERROR_OpenSslSha256Final;
        return ret;
    }
    return ret;
}

int openssl_HMACsha256(const void* key, int key_size, const void* data, int data_size, void* digest)
{
    int ret = ERROR_SUCCESS;
    unsigned int digest_size   = 0;
    unsigned char* temp_key    = (unsigned char *) key;
    unsigned char* temp_digest = (unsigned char *) digest;

    if (key == NULL) {
        if (EVP_Digest(data, data_size, temp_digest, &digest_size, EVP_sha256(), NULL) < 0) {
            ret = ERROR_OpenSslSha256EvpDigest;
            return ret;
        }
    } else {
        HMAC_CTX ctx;
        HMAC_CTX_init(&ctx);
        if (HMAC_Init_ex(&ctx, temp_key, key_size, EVP_sha256(), NULL) < 0) {
            ret = ERROR_OpenSslSha256Init;
            return ret;
        }
        ret = do_openssl_HMACsha256(&ctx, data, data_size, temp_digest, &digest_size);
        HMAC_CTX_cleanup(&ctx);
        if (ret != ERROR_SUCCESS) {
            return ret;
        }
    }
    if (digest_size != 32) {
        ret = ERROR_OpenSslSha256DigestSize;
        return ret;
    }
    return ret;
}

# define RFC2409_PRIME_1024 \
    "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1" \
    "29024E088A67CC74020BBEA63B139B22514A08798E3404DD" \
    "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245" \
    "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED" \
    "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE65381" \
    "FFFFFFFFFFFFFFFF"
SrsDH::SrsDH()
{
    pdh = NULL;
}

SrsDH::~SrsDH()
{
    if (pdh != NULL) {
        if (pdh->p != NULL) {
            BN_free(pdh->p);
            pdh->p = NULL;
        }
        if (pdh->g != NULL) {
            BN_free(pdh->g);
            pdh->g = NULL;
        }
        DH_free(pdh);
        pdh = NULL;
    }
}

int SrsDH::initialize(bool ensure_128bytes_public_key)
{
    int ret = ERROR_SUCCESS;

    for (;;) {
        if ((ret = do_initialize()) != ERROR_SUCCESS) {
            return ret;
        }
        if (ensure_128bytes_public_key) {
            int32_t key_size = BN_num_bytes(pdh->pub_key);
            if (key_size != 128) {
                srs_warn("regenerate 128B key, current=%dB", key_size);
                continue;
            }
        }
        break;
    }
    return ret;
}

int SrsDH::copy_public_key(char* pkey, int32_t& pkey_size)
{
    int ret = ERROR_SUCCESS;
    int32_t key_size = BN_num_bytes(pdh->pub_key);

    srs_assert(key_size > 0);
    key_size = BN_bn2bin(pdh->pub_key, (unsigned char *) pkey);
    srs_assert(key_size > 0);
    srs_assert(key_size <= pkey_size);
    pkey_size = key_size;
    return ret;
}

int SrsDH::copy_shared_key(const char* ppkey, int32_t ppkey_size, char* skey, int32_t& skey_size)
{
    int ret     = ERROR_SUCCESS;
    BIGNUM* ppk = NULL;

    if ((ppk = BN_bin2bn((const unsigned char *) ppkey, ppkey_size, 0)) == NULL) {
        ret = ERROR_OpenSslGetPeerPublicKey;
        return ret;
    }
    int32_t key_size = DH_compute_key((unsigned char *) skey, ppk, pdh);
    if (key_size < ppkey_size) {
        srs_warn("shared key size=%d, ppk_size=%d", key_size, ppkey_size);
    }
    if (key_size < 0 || key_size > skey_size) {
        ret = ERROR_OpenSslComputeSharedKey;
    } else {
        skey_size = key_size;
    }
    if (ppk) {
        BN_free(ppk);
    }
    return ret;
}

int SrsDH::do_initialize()
{
    int ret = ERROR_SUCCESS;
    int32_t bits_count = 1024;

    if ((pdh = DH_new()) == NULL) {
        ret = ERROR_OpenSslCreateDH;
        return ret;
    }
    if ((pdh->p = BN_new()) == NULL) {
        ret = ERROR_OpenSslCreateP;
        return ret;
    }
    if ((pdh->g = BN_new()) == NULL) {
        ret = ERROR_OpenSslCreateG;
        return ret;
    }
    if (!BN_hex2bn(&pdh->p, RFC2409_PRIME_1024)) {
        ret = ERROR_OpenSslParseP1024;
        return ret;
    }
    if (!BN_set_word(pdh->g, 2)) {
        ret = ERROR_OpenSslSetG;
        return ret;
    }
    pdh->length = bits_count;
    if (!DH_generate_key(pdh)) {
        ret = ERROR_OpenSslGenerateDHKeys;
        return ret;
    }
    return ret;
} // SrsDH::do_initialize

key_block::key_block()
{
    offset  = (int32_t) rand();
    random0 = NULL;
    random1 = NULL;
    int valid_offset = calc_valid_offset();
    srs_assert(valid_offset >= 0);
    random0_size = valid_offset;
    if (random0_size > 0) {
        random0 = new char[random0_size];
        srs_random_generate(random0, random0_size);
        snprintf(random0, random0_size, "%s", RTMP_SIG_SRS_HANDSHAKE);
    }
    srs_random_generate(key, sizeof(key));
    random1_size = 764 - valid_offset - 128 - 4;
    if (random1_size > 0) {
        random1 = new char[random1_size];
        srs_random_generate(random1, random1_size);
        snprintf(random1, random1_size, "%s", RTMP_SIG_SRS_HANDSHAKE);
    }
}

key_block::~key_block()
{
    srs_freepa(random0);
    srs_freepa(random1);
}

int key_block::parse(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    srs_assert(stream->require(764));
    stream->skip(764 - sizeof(int32_t));
    offset = stream->read_4bytes();
    stream->skip(-764);
    int valid_offset = calc_valid_offset();
    srs_assert(valid_offset >= 0);
    random0_size = valid_offset;
    if (random0_size > 0) {
        srs_freepa(random0);
        random0 = new char[random0_size];
        stream->read_bytes(random0, random0_size);
    }
    stream->read_bytes(key, 128);
    random1_size = 764 - valid_offset - 128 - 4;
    if (random1_size > 0) {
        srs_freepa(random1);
        random1 = new char[random1_size];
        stream->read_bytes(random1, random1_size);
    }
    return ret;
}

int key_block::calc_valid_offset()
{
    int max_offset_size = 764 - 128 - 4;
    int valid_offset    = 0;
    u_int8_t* pp        = (u_int8_t *) &offset;

    valid_offset += *pp++;
    valid_offset += *pp++;
    valid_offset += *pp++;
    valid_offset += *pp++;
    return valid_offset % max_offset_size;
}

digest_block::digest_block()
{
    offset  = (int32_t) rand();
    random0 = NULL;
    random1 = NULL;
    int valid_offset = calc_valid_offset();
    srs_assert(valid_offset >= 0);
    random0_size = valid_offset;
    if (random0_size > 0) {
        random0 = new char[random0_size];
        srs_random_generate(random0, random0_size);
        snprintf(random0, random0_size, "%s", RTMP_SIG_SRS_HANDSHAKE);
    }
    srs_random_generate(digest, sizeof(digest));
    random1_size = 764 - 4 - valid_offset - 32;
    if (random1_size > 0) {
        random1 = new char[random1_size];
        srs_random_generate(random1, random1_size);
        snprintf(random1, random1_size, "%s", RTMP_SIG_SRS_HANDSHAKE);
    }
}

digest_block::~digest_block()
{
    srs_freepa(random0);
    srs_freepa(random1);
}

int digest_block::parse(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    srs_assert(stream->require(764));
    offset = stream->read_4bytes();
    int valid_offset = calc_valid_offset();
    srs_assert(valid_offset >= 0);
    random0_size = valid_offset;
    if (random0_size > 0) {
        srs_freepa(random0);
        random0 = new char[random0_size];
        stream->read_bytes(random0, random0_size);
    }
    stream->read_bytes(digest, 32);
    random1_size = 764 - 4 - valid_offset - 32;
    if (random1_size > 0) {
        srs_freepa(random1);
        random1 = new char[random1_size];
        stream->read_bytes(random1, random1_size);
    }
    return ret;
}

int digest_block::calc_valid_offset()
{
    int max_offset_size = 764 - 32 - 4;
    int valid_offset    = 0;
    u_int8_t* pp        = (u_int8_t *) &offset;

    valid_offset += *pp++;
    valid_offset += *pp++;
    valid_offset += *pp++;
    valid_offset += *pp++;
    return valid_offset % max_offset_size;
}

c1s1_strategy::c1s1_strategy()
{}

c1s1_strategy::~c1s1_strategy()
{}

char * c1s1_strategy::get_digest()
{
    return digest.digest;
}

char * c1s1_strategy::get_key()
{
    return key.key;
}

int c1s1_strategy::dump(c1s1* owner, char* _c1s1, int size)
{
    srs_assert(size == 1536);
    return copy_to(owner, _c1s1, size, true);
}

int c1s1_strategy::c1_create(c1s1* owner)
{
    int ret         = ERROR_SUCCESS;
    char* c1_digest = NULL;

    if ((ret = calc_c1_digest(owner, c1_digest)) != ERROR_SUCCESS) {
        srs_error("sign c1 error, failed to calc digest. ret=%d", ret);
        return ret;
    }
    srs_assert(c1_digest != NULL);
    SrsAutoFreeA(char, c1_digest);
    memcpy(digest.digest, c1_digest, 32);
    return ret;
}

int c1s1_strategy::c1_validate_digest(c1s1* owner, bool& is_valid)
{
    int ret         = ERROR_SUCCESS;
    char* c1_digest = NULL;

    if ((ret = calc_c1_digest(owner, c1_digest)) != ERROR_SUCCESS) {
        srs_error("validate c1 error, failed to calc digest. ret=%d", ret);
        return ret;
    }
    srs_assert(c1_digest != NULL);
    SrsAutoFreeA(char, c1_digest);
    is_valid = srs_bytes_equals(digest.digest, c1_digest, 32);
    return ret;
}

int c1s1_strategy::s1_create(c1s1* owner, c1s1* c1)
{
    int ret = ERROR_SUCCESS;
    SrsDH dh;

    if ((ret = dh.initialize(true)) != ERROR_SUCCESS) {
        return ret;
    }
    int pkey_size = 128;
    if ((ret = dh.copy_shared_key(c1->get_key(), 128, key.key, pkey_size)) != ERROR_SUCCESS) {
        srs_error("calc s1 key failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("calc s1 key success.");
    char* s1_digest = NULL;
    if ((ret = calc_s1_digest(owner, s1_digest)) != ERROR_SUCCESS) {
        srs_error("calc s1 digest failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("calc s1 digest success.");
    srs_assert(s1_digest != NULL);
    SrsAutoFreeA(char, s1_digest);
    memcpy(digest.digest, s1_digest, 32);
    srs_verbose("copy s1 key success.");
    return ret;
}

int c1s1_strategy::s1_validate_digest(c1s1* owner, bool& is_valid)
{
    int ret         = ERROR_SUCCESS;
    char* s1_digest = NULL;

    if ((ret = calc_s1_digest(owner, s1_digest)) != ERROR_SUCCESS) {
        srs_error("validate s1 error, failed to calc digest. ret=%d", ret);
        return ret;
    }
    srs_assert(s1_digest != NULL);
    SrsAutoFreeA(char, s1_digest);
    is_valid = srs_bytes_equals(digest.digest, s1_digest, 32);
    return ret;
}

int c1s1_strategy::calc_c1_digest(c1s1* owner, char *& c1_digest)
{
    int ret = ERROR_SUCCESS;
    char* c1s1_joined_bytes = new char[1536 - 32];

    SrsAutoFreeA(char, c1s1_joined_bytes);
    if ((ret = copy_to(owner, c1s1_joined_bytes, 1536 - 32, false)) != ERROR_SUCCESS) {
        return ret;
    }
    c1_digest = new char[SRS_OpensslHashSize];
    if ((ret = openssl_HMACsha256(SrsGenuineFPKey, 30, c1s1_joined_bytes, 1536 - 32, c1_digest)) != ERROR_SUCCESS) {
        srs_freepa(c1_digest);
        srs_error("calc digest for c1 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("digest calculated for c1");
    return ret;
}

int c1s1_strategy::calc_s1_digest(c1s1* owner, char *& s1_digest)
{
    int ret = ERROR_SUCCESS;
    char* c1s1_joined_bytes = new char[1536 - 32];

    SrsAutoFreeA(char, c1s1_joined_bytes);
    if ((ret = copy_to(owner, c1s1_joined_bytes, 1536 - 32, false)) != ERROR_SUCCESS) {
        return ret;
    }
    s1_digest = new char[SRS_OpensslHashSize];
    if ((ret = openssl_HMACsha256(SrsGenuineFMSKey, 36, c1s1_joined_bytes, 1536 - 32, s1_digest)) != ERROR_SUCCESS) {
        srs_freepa(s1_digest);
        srs_error("calc digest for s1 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("digest calculated for s1");
    return ret;
}

void c1s1_strategy::copy_time_version(SrsStream* stream, c1s1* owner)
{
    srs_assert(stream->require(8));
    stream->write_4bytes(owner->time);
    stream->write_4bytes(owner->version);
}

void c1s1_strategy::copy_key(SrsStream* stream)
{
    srs_assert(key.random0_size >= 0);
    srs_assert(key.random1_size >= 0);
    int total = key.random0_size + 128 + key.random1_size + 4;
    srs_assert(stream->require(total));
    if (key.random0_size > 0) {
        stream->write_bytes(key.random0, key.random0_size);
    }
    stream->write_bytes(key.key, 128);
    if (key.random1_size > 0) {
        stream->write_bytes(key.random1, key.random1_size);
    }
    stream->write_4bytes(key.offset);
}

void c1s1_strategy::copy_digest(SrsStream* stream, bool with_digest)
{
    srs_assert(key.random0_size >= 0);
    srs_assert(key.random1_size >= 0);
    int total = 4 + digest.random0_size + digest.random1_size;
    if (with_digest) {
        total += 32;
    }
    srs_assert(stream->require(total));
    stream->write_4bytes(digest.offset);
    if (digest.random0_size > 0) {
        stream->write_bytes(digest.random0, digest.random0_size);
    }
    if (with_digest) {
        stream->write_bytes(digest.digest, 32);
    }
    if (digest.random1_size > 0) {
        stream->write_bytes(digest.random1, digest.random1_size);
    }
}

c1s1_strategy_schema0::c1s1_strategy_schema0()
{}

c1s1_strategy_schema0::~c1s1_strategy_schema0()
{}

srs_schema_type c1s1_strategy_schema0::schema()
{
    return srs_schema0;
}

int c1s1_strategy_schema0::parse(char* _c1s1, int size)
{
    int ret = ERROR_SUCCESS;

    srs_assert(size == 1536);
    SrsStream stream;
    if ((ret = stream.initialize(_c1s1 + 8, 764)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = key.parse(&stream)) != ERROR_SUCCESS) {
        srs_error("parse the c1 key failed. ret=%d", ret);
        return ret;
    }
    if ((ret = stream.initialize(_c1s1 + 8 + 764, 764)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = digest.parse(&stream)) != ERROR_SUCCESS) {
        srs_error("parse the c1 digest failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("parse c1 key-digest success");
    return ret;
}

int c1s1_strategy_schema0::copy_to(c1s1* owner, char* bytes, int size, bool with_digest)
{
    int ret = ERROR_SUCCESS;

    if (with_digest) {
        srs_assert(size == 1536);
    } else {
        srs_assert(size == 1504);
    }
    SrsStream stream;
    if ((ret = stream.initialize(bytes, size)) != ERROR_SUCCESS) {
        return ret;
    }
    copy_time_version(&stream, owner);
    copy_key(&stream);
    copy_digest(&stream, with_digest);
    srs_assert(stream.empty());
    return ret;
}

c1s1_strategy_schema1::c1s1_strategy_schema1()
{}

c1s1_strategy_schema1::~c1s1_strategy_schema1()
{}

srs_schema_type c1s1_strategy_schema1::schema()
{
    return srs_schema1;
}

int c1s1_strategy_schema1::parse(char* _c1s1, int size)
{
    int ret = ERROR_SUCCESS;

    srs_assert(size == 1536);
    SrsStream stream;
    if ((ret = stream.initialize(_c1s1 + 8, 764)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = digest.parse(&stream)) != ERROR_SUCCESS) {
        srs_error("parse the c1 digest failed. ret=%d", ret);
        return ret;
    }
    if ((ret = stream.initialize(_c1s1 + 8 + 764, 764)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = key.parse(&stream)) != ERROR_SUCCESS) {
        srs_error("parse the c1 key failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("parse c1 digest-key success");
    return ret;
}

int c1s1_strategy_schema1::copy_to(c1s1* owner, char* bytes, int size, bool with_digest)
{
    int ret = ERROR_SUCCESS;

    if (with_digest) {
        srs_assert(size == 1536);
    } else {
        srs_assert(size == 1504);
    }
    SrsStream stream;
    if ((ret = stream.initialize(bytes, size)) != ERROR_SUCCESS) {
        return ret;
    }
    copy_time_version(&stream, owner);
    copy_digest(&stream, with_digest);
    copy_key(&stream);
    srs_assert(stream.empty());
    return ret;
}

c1s1::c1s1()
{
    payload = NULL;
}

c1s1::~c1s1()
{
    srs_freep(payload);
}

srs_schema_type c1s1::schema()
{
    srs_assert(payload != NULL);
    return payload->schema();
}

char * c1s1::get_digest()
{
    srs_assert(payload != NULL);
    return payload->get_digest();
}

char * c1s1::get_key()
{
    srs_assert(payload != NULL);
    return payload->get_key();
}

int c1s1::dump(char* _c1s1, int size)
{
    srs_assert(size == 1536);
    srs_assert(payload != NULL);
    return payload->dump(this, _c1s1, size);
}

int c1s1::parse(char* _c1s1, int size, srs_schema_type schema)
{
    int ret = ERROR_SUCCESS;

    srs_assert(size == 1536);
    if (schema != srs_schema0 && schema != srs_schema1) {
        ret = ERROR_RTMP_CH_SCHEMA;
        srs_error("parse c1 failed. invalid schema=%d, ret=%d", schema, ret);
        return ret;
    }
    SrsStream stream;
    if ((ret = stream.initialize(_c1s1, size)) != ERROR_SUCCESS) {
        return ret;
    }
    time    = stream.read_4bytes();
    version = stream.read_4bytes();
    srs_freep(payload);
    if (schema == srs_schema0) {
        payload = new c1s1_strategy_schema0();
    } else {
        payload = new c1s1_strategy_schema1();
    }
    return payload->parse(_c1s1, size);
}

int c1s1::c1_create(srs_schema_type schema)
{
    int ret = ERROR_SUCCESS;

    if (schema != srs_schema0 && schema != srs_schema1) {
        ret = ERROR_RTMP_CH_SCHEMA;
        srs_error("create c1 failed. invalid schema=%d, ret=%d", schema, ret);
        return ret;
    }
    time    = (int32_t) ::time(NULL);
    version = 0x80000702;
    srs_freep(payload);
    if (schema == srs_schema0) {
        payload = new c1s1_strategy_schema0();
    } else {
        payload = new c1s1_strategy_schema1();
    }
    return payload->c1_create(this);
}

int c1s1::c1_validate_digest(bool& is_valid)
{
    is_valid = false;
    srs_assert(payload);
    return payload->c1_validate_digest(this, is_valid);
}

int c1s1::s1_create(c1s1* c1)
{
    int ret = ERROR_SUCCESS;

    if (c1->schema() != srs_schema0 && c1->schema() != srs_schema1) {
        ret = ERROR_RTMP_CH_SCHEMA;
        srs_error("create s1 failed. invalid schema=%d, ret=%d", c1->schema(), ret);
        return ret;
    }
    time    = ::time(NULL);
    version = 0x01000504;
    srs_freep(payload);
    if (c1->schema() == srs_schema0) {
        payload = new c1s1_strategy_schema0();
    } else {
        payload = new c1s1_strategy_schema1();
    }
    return payload->s1_create(this, c1);
}

int c1s1::s1_validate_digest(bool& is_valid)
{
    is_valid = false;
    srs_assert(payload);
    return payload->s1_validate_digest(this, is_valid);
}

c2s2::c2s2()
{
    srs_random_generate(random, 1504);
    int size = snprintf(random, 1504, "%s", RTMP_SIG_SRS_HANDSHAKE);
    srs_assert(++size < 1504);
    snprintf(random + 1504 - size, size, "%s", RTMP_SIG_SRS_HANDSHAKE);
    srs_random_generate(digest, 32);
}

c2s2::~c2s2()
{}

int c2s2::dump(char* _c2s2, int size)
{
    srs_assert(size == 1536);
    memcpy(_c2s2, random, 1504);
    memcpy(_c2s2 + 1504, digest, 32);
    return ERROR_SUCCESS;
}

int c2s2::parse(char* _c2s2, int size)
{
    srs_assert(size == 1536);
    memcpy(random, _c2s2, 1504);
    memcpy(digest, _c2s2 + 1504, 32);
    return ERROR_SUCCESS;
}

int c2s2::c2_create(c1s1* s1)
{
    int ret = ERROR_SUCCESS;
    char temp_key[SRS_OpensslHashSize];

    if ((ret = openssl_HMACsha256(SrsGenuineFPKey, 62, s1->get_digest(), 32, temp_key)) != ERROR_SUCCESS) {
        srs_error("create c2 temp key failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("generate c2 temp key success.");
    char _digest[SRS_OpensslHashSize];
    if ((ret = openssl_HMACsha256(temp_key, 32, random, 1504, _digest)) != ERROR_SUCCESS) {
        srs_error("create c2 digest failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("generate c2 digest success.");
    memcpy(digest, _digest, 32);
    return ret;
}

int c2s2::c2_validate(c1s1* s1, bool& is_valid)
{
    is_valid = false;
    int ret = ERROR_SUCCESS;
    char temp_key[SRS_OpensslHashSize];
    if ((ret = openssl_HMACsha256(SrsGenuineFPKey, 62, s1->get_digest(), 32, temp_key)) != ERROR_SUCCESS) {
        srs_error("create c2 temp key failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("generate c2 temp key success.");
    char _digest[SRS_OpensslHashSize];
    if ((ret = openssl_HMACsha256(temp_key, 32, random, 1504, _digest)) != ERROR_SUCCESS) {
        srs_error("create c2 digest failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("generate c2 digest success.");
    is_valid = srs_bytes_equals(digest, _digest, 32);
    return ret;
}

int c2s2::s2_create(c1s1* c1)
{
    int ret = ERROR_SUCCESS;
    char temp_key[SRS_OpensslHashSize];

    if ((ret = openssl_HMACsha256(SrsGenuineFMSKey, 68, c1->get_digest(), 32, temp_key)) != ERROR_SUCCESS) {
        srs_error("create s2 temp key failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("generate s2 temp key success.");
    char _digest[SRS_OpensslHashSize];
    if ((ret = openssl_HMACsha256(temp_key, 32, random, 1504, _digest)) != ERROR_SUCCESS) {
        srs_error("create s2 digest failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("generate s2 digest success.");
    memcpy(digest, _digest, 32);
    return ret;
}

int c2s2::s2_validate(c1s1* c1, bool& is_valid)
{
    is_valid = false;
    int ret = ERROR_SUCCESS;
    char temp_key[SRS_OpensslHashSize];
    if ((ret = openssl_HMACsha256(SrsGenuineFMSKey, 68, c1->get_digest(), 32, temp_key)) != ERROR_SUCCESS) {
        srs_error("create s2 temp key failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("generate s2 temp key success.");
    char _digest[SRS_OpensslHashSize];
    if ((ret = openssl_HMACsha256(temp_key, 32, random, 1504, _digest)) != ERROR_SUCCESS) {
        srs_error("create s2 digest failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("generate s2 digest success.");
    is_valid = srs_bytes_equals(digest, _digest, 32);
    return ret;
}
}
#endif // ifdef SRS_AUTO_SSL
SrsSimpleHandshake::SrsSimpleHandshake()
{}

SrsSimpleHandshake::~SrsSimpleHandshake()
{}

int SrsSimpleHandshake::handshake_with_client(SrsHandshakeBytes* hs_bytes, ISrsProtocolReaderWriter* io)
{
    int ret = ERROR_SUCCESS;
    ssize_t nsize;

    if ((ret = hs_bytes->read_c0c1(io)) != ERROR_SUCCESS) {
        return ret;
    }
    if (hs_bytes->c0c1[0] != 0x03) {
        ret = ERROR_RTMP_PLAIN_REQUIRED;
        srs_warn("only support rtmp plain text. ret=%d", ret);
        return ret;
    }
    srs_verbose("check c0 success, required plain text.");
    if ((ret = hs_bytes->create_s0s1s2(hs_bytes->c0c1 + 1)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = io->write(hs_bytes->s0s1s2, 3073, &nsize)) != ERROR_SUCCESS) {
        srs_warn("simple handshake send s0s1s2 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("simple handshake send s0s1s2 success.");
    if ((ret = hs_bytes->read_c2(io)) != ERROR_SUCCESS) {
        return ret;
    }
    srs_trace("simple handshake success.");
    return ret;
}

int SrsSimpleHandshake::handshake_with_server(SrsHandshakeBytes* hs_bytes, ISrsProtocolReaderWriter* io)
{
    int ret = ERROR_SUCCESS;
    ssize_t nsize;

    if ((ret = hs_bytes->create_c0c1()) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = io->write(hs_bytes->c0c1, 1537, &nsize)) != ERROR_SUCCESS) {
        srs_warn("write c0c1 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("write c0c1 success.");
    if ((ret = hs_bytes->read_s0s1s2(io)) != ERROR_SUCCESS) {
        return ret;
    }
    if (hs_bytes->s0s1s2[0] != 0x03) {
        ret = ERROR_RTMP_HANDSHAKE;
        srs_warn("handshake failed, plain text required. ret=%d", ret);
        return ret;
    }
    if ((ret = hs_bytes->create_c2()) != ERROR_SUCCESS) {
        return ret;
    }
    memcpy(hs_bytes->c2, hs_bytes->s0s1s2 + 1, 1536);
    if ((ret = io->write(hs_bytes->c2, 1536, &nsize)) != ERROR_SUCCESS) {
        srs_warn("simple handshake write c2 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("simple handshake write c2 success.");
    srs_trace("simple handshake success.");
    return ret;
} // SrsSimpleHandshake::handshake_with_server

SrsComplexHandshake::SrsComplexHandshake()
{}

SrsComplexHandshake::~SrsComplexHandshake()
{}

#ifndef SRS_AUTO_SSL
int SrsComplexHandshake::handshake_with_client(SrsHandshakeBytes *, ISrsProtocolReaderWriter *)
{
    srs_trace("directly use simple handshake for ssl disabled.");
    return ERROR_RTMP_TRY_SIMPLE_HS;
}

#else
int SrsComplexHandshake::handshake_with_client(SrsHandshakeBytes* hs_bytes, ISrsProtocolReaderWriter* io)
{
    int ret = ERROR_SUCCESS;
    ssize_t nsize;

    if ((ret = hs_bytes->read_c0c1(io)) != ERROR_SUCCESS) {
        return ret;
    }
    c1s1 c1;
    if ((ret = c1.parse(hs_bytes->c0c1 + 1, 1536, srs_schema0)) != ERROR_SUCCESS) {
        srs_error("parse c1 schema%d error. ret=%d", srs_schema0, ret);
        return ret;
    }
    bool is_valid = false;
    if ((ret = c1.c1_validate_digest(is_valid)) != ERROR_SUCCESS || !is_valid) {
        srs_info("schema0 failed, try schema1.");
        if ((ret = c1.parse(hs_bytes->c0c1 + 1, 1536, srs_schema1)) != ERROR_SUCCESS) {
            srs_error("parse c1 schema%d error. ret=%d", srs_schema1, ret);
            return ret;
        }
        if ((ret = c1.c1_validate_digest(is_valid)) != ERROR_SUCCESS || !is_valid) {
            ret = ERROR_RTMP_TRY_SIMPLE_HS;
            srs_info("all schema valid failed, try simple handshake. ret=%d", ret);
            return ret;
        }
    } else {
        srs_info("schema0 is ok.");
    }
    srs_verbose("decode c1 success.");
    c1s1 s1;
    if ((ret = s1.s1_create(&c1)) != ERROR_SUCCESS) {
        srs_error("create s1 from c1 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("create s1 from c1 success.");
    if ((ret = s1.s1_validate_digest(is_valid)) != ERROR_SUCCESS || !is_valid) {
        ret = ERROR_RTMP_TRY_SIMPLE_HS;
        srs_info("verify s1 failed, try simple handshake. ret=%d", ret);
        return ret;
    }
    srs_verbose("verify s1 success.");
    c2s2 s2;
    if ((ret = s2.s2_create(&c1)) != ERROR_SUCCESS) {
        srs_error("create s2 from c1 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("create s2 from c1 success.");
    if ((ret = s2.s2_validate(&c1, is_valid)) != ERROR_SUCCESS || !is_valid) {
        ret = ERROR_RTMP_TRY_SIMPLE_HS;
        srs_info("verify s2 failed, try simple handshake. ret=%d", ret);
        return ret;
    }
    srs_verbose("verify s2 success.");
    if ((ret = hs_bytes->create_s0s1s2()) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = s1.dump(hs_bytes->s0s1s2 + 1, 1536)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = s2.dump(hs_bytes->s0s1s2 + 1537, 1536)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = io->write(hs_bytes->s0s1s2, 3073, &nsize)) != ERROR_SUCCESS) {
        srs_warn("complex handshake send s0s1s2 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("complex handshake send s0s1s2 success.");
    if ((ret = hs_bytes->read_c2(io)) != ERROR_SUCCESS) {
        return ret;
    }
    c2s2 c2;
    if ((ret = c2.parse(hs_bytes->c2, 1536)) != ERROR_SUCCESS) {
        return ret;
    }
    srs_verbose("complex handshake read c2 success.");
    srs_trace("complex handshake success");
    return ret;
} // SrsComplexHandshake::handshake_with_client

#endif // ifndef SRS_AUTO_SSL
#ifndef SRS_AUTO_SSL
int SrsComplexHandshake::handshake_with_server(SrsHandshakeBytes *, ISrsProtocolReaderWriter *)
{
    return ERROR_RTMP_TRY_SIMPLE_HS;
}

#else
int SrsComplexHandshake::handshake_with_server(SrsHandshakeBytes* hs_bytes, ISrsProtocolReaderWriter* io)
{
    int ret = ERROR_SUCCESS;
    ssize_t nsize;

    if ((ret = hs_bytes->create_c0c1()) != ERROR_SUCCESS) {
        return ret;
    }
    c1s1 c1;
    if ((ret = c1.c1_create(srs_schema1)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = c1.dump(hs_bytes->c0c1 + 1, 1536)) != ERROR_SUCCESS) {
        return ret;
    }
    bool is_valid;
    if ((ret = c1.c1_validate_digest(is_valid)) != ERROR_SUCCESS || !is_valid) {
        ret = ERROR_RTMP_TRY_SIMPLE_HS;
        return ret;
    }
    if ((ret = io->write(hs_bytes->c0c1, 1537, &nsize)) != ERROR_SUCCESS) {
        srs_warn("write c0c1 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("write c0c1 success.");
    if ((ret = hs_bytes->read_s0s1s2(io)) != ERROR_SUCCESS) {
        return ret;
    }
    if (hs_bytes->s0s1s2[0] != 0x03) {
        ret = ERROR_RTMP_HANDSHAKE;
        srs_warn("handshake failed, plain text required. ret=%d", ret);
        return ret;
    }
    c1s1 s1;
    if ((ret = s1.parse(hs_bytes->s0s1s2 + 1, 1536, c1.schema())) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = hs_bytes->create_c2()) != ERROR_SUCCESS) {
        return ret;
    }
    c2s2 c2;
    if ((ret = c2.c2_create(&s1)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = c2.dump(hs_bytes->c2, 1536)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = io->write(hs_bytes->c2, 1536, &nsize)) != ERROR_SUCCESS) {
        srs_warn("complex handshake write c2 failed. ret=%d", ret);
        return ret;
    }
    srs_verbose("complex handshake write c2 success.");
    srs_trace("complex handshake success.");
    return ret;
} // SrsComplexHandshake::handshake_with_server

#endif // ifndef SRS_AUTO_SSL
#ifndef _WIN32
# include <unistd.h>
#endif
#include <stdlib.h>
using namespace std;
void srs_discovery_tc_url(
    string tcUrl,
    string& schema, string& host, string& vhost,
    string& app, string& port, std::string& param
)
{
    size_t pos      = std::string::npos;
    std::string url = tcUrl;

    if ((pos = url.find("://")) != std::string::npos) {
        schema = url.substr(0, pos);
        url    = url.substr(schema.length() + 3);
        srs_info("discovery schema=%s", schema.c_str());
    }
    if ((pos = url.find("/")) != std::string::npos) {
        host = url.substr(0, pos);
        url  = url.substr(host.length() + 1);
        srs_info("discovery host=%s", host.c_str());
    }
    port = SRS_CONSTS_RTMP_DEFAULT_PORT;
    if ((pos = host.find(":")) != std::string::npos) {
        port = host.substr(pos + 1);
        host = host.substr(0, pos);
        srs_info("discovery host=%s, port=%s", host.c_str(), port.c_str());
    }
    app   = url;
    vhost = host;
    srs_vhost_resolve(vhost, app, param);
}

void srs_vhost_resolve(string& vhost, string& app, string& param)
{
    size_t pos = 0;

    if ((pos = app.find("?")) != std::string::npos) {
        param = app.substr(pos);
    }
    app = srs_string_replace(app, ",", "?");
    app = srs_string_replace(app, "...", "?");
    app = srs_string_replace(app, "&&", "?");
    app = srs_string_replace(app, "=", "?");
    if ((pos = app.find("?")) != std::string::npos) {
        std::string query = app.substr(pos + 1);
        app = app.substr(0, pos);
        if ((pos = query.find("vhost?")) != std::string::npos) {
            query = query.substr(pos + 6);
            if (!query.empty()) {
                vhost = query;
            }
            if ((pos = vhost.find("?")) != std::string::npos) {
                vhost = vhost.substr(0, pos);
            }
        }
    }
}

void srs_random_generate(char* bytes, int size)
{
    static bool _random_initialized = false;

    if (!_random_initialized) {
        srand(0);
        _random_initialized = true;
        srs_trace("srand initialized the random.");
    }
    for (int i = 0; i < size; i++) {
        bytes[i] = 0x0f + (rand() % (256 - 0x0f - 0x0f));
    }
}

string srs_generate_tc_url(string ip, string vhost, string app, string port, string param)
{
    string tcUrl = "rtmp://";

    if (vhost == SRS_CONSTS_RTMP_DEFAULT_VHOST) {
        tcUrl += ip;
    } else {
        tcUrl += vhost;
    }
    if (port != SRS_CONSTS_RTMP_DEFAULT_PORT) {
        tcUrl += ":";
        tcUrl += port;
    }
    tcUrl += "/";
    tcUrl += app;
    tcUrl += param;
    return tcUrl;
}

bool srs_bytes_equals(void* pa, void* pb, int size)
{
    u_int8_t* a = (u_int8_t *) pa;
    u_int8_t* b = (u_int8_t *) pb;

    if (!a && !b) {
        return true;
    }
    if (!a || !b) {
        return false;
    }
    for (int i = 0; i < size; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

int srs_do_rtmp_create_msg(char type, u_int32_t timestamp, char* data, int size, int stream_id,
                           SrsSharedPtrMessage** ppmsg)
{
    int ret = ERROR_SUCCESS;

    *ppmsg = NULL;
    SrsSharedPtrMessage* msg = NULL;
    if (type == SrsCodecFlvTagAudio) {
        SrsMessageHeader header;
        header.initialize_audio(size, timestamp, stream_id);
        msg = new SrsSharedPtrMessage();
        if ((ret = msg->create(&header, data, size)) != ERROR_SUCCESS) {
            srs_freep(msg);
            return ret;
        }
    } else if (type == SrsCodecFlvTagVideo) {
        SrsMessageHeader header;
        header.initialize_video(size, timestamp, stream_id);
        msg = new SrsSharedPtrMessage();
        if ((ret = msg->create(&header, data, size)) != ERROR_SUCCESS) {
            srs_freep(msg);
            return ret;
        }
    } else if (type == SrsCodecFlvTagScript) {
        SrsMessageHeader header;
        header.initialize_amf0_script(size, stream_id);
        msg = new SrsSharedPtrMessage();
        if ((ret = msg->create(&header, data, size)) != ERROR_SUCCESS) {
            srs_freep(msg);
            return ret;
        }
    } else {
        ret = ERROR_STREAM_CASTER_FLV_TAG;
        srs_error("rtmp unknown tag type=%#x. ret=%d", type, ret);
        return ret;
    }
    *ppmsg = msg;
    return ret;
} // srs_do_rtmp_create_msg

int srs_rtmp_create_msg(char type, u_int32_t timestamp, char* data, int size, int stream_id,
                        SrsSharedPtrMessage** ppmsg)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_do_rtmp_create_msg(type, timestamp, data, size, stream_id, ppmsg)) != ERROR_SUCCESS) {
        srs_freepa(data);
        return ret;
    }
    return ret;
}

std::string srs_generate_stream_url(std::string vhost, std::string app, std::string stream)
{
    std::string url = "";

    if (SRS_CONSTS_RTMP_DEFAULT_VHOST != vhost) {
        url += vhost;
    }
    url += "/";
    url += app;
    url += "/";
    url += stream;
    return url;
}

int srs_write_large_iovs(ISrsProtocolReaderWriter* skt, iovec* iovs, int size, ssize_t* pnwrite)
{
    int ret = ERROR_SUCCESS;

    #ifndef _WIN32
    static int limits = (int) sysconf(_SC_IOV_MAX);
    #else
    static int limits = 1024;
    #endif
    if (size < limits) {
        if ((ret = skt->writev(iovs, size, pnwrite)) != ERROR_SUCCESS) {
            if (!srs_is_client_gracefully_close(ret)) {
                srs_error("send with writev failed. ret=%d", ret);
            }
            return ret;
        }
        return ret;
    }
    int cur_iov = 0;
    while (cur_iov < size) {
        int cur_count = srs_min(limits, size - cur_iov);
        if ((ret = skt->writev(iovs + cur_iov, cur_count, pnwrite)) != ERROR_SUCCESS) {
            if (!srs_is_client_gracefully_close(ret)) {
                srs_error("send with writev failed. ret=%d", ret);
            }
            return ret;
        }
        cur_iov += cur_count;
    }
    return ret;
} // srs_write_large_iovs

SrsMessageArray::SrsMessageArray(int max_msgs)
{
    srs_assert(max_msgs > 0);
    msgs = new SrsSharedPtrMessage *[max_msgs];
    max  = max_msgs;
    zero(max_msgs);
}

SrsMessageArray::~SrsMessageArray()
{
    srs_freepa(msgs);
}

void SrsMessageArray::free(int count)
{
    for (int i = 0; i < count; i++) {
        SrsSharedPtrMessage* msg = msgs[i];
        srs_freep(msg);
        msgs[i] = NULL;
    }
}

void SrsMessageArray::zero(int count)
{
    for (int i = 0; i < count; i++) {
        msgs[i] = NULL;
    }
}

#include <stdlib.h>
#define SRS_DEFAULT_RECV_BUFFER_SIZE 131072
#define SRS_MAX_SOCKET_BUFFER        262144
#define SRS_RTMP_MAX_MESSAGE_HEADER  11
#ifdef SRS_PERF_MERGED_READ
IMergeReadHandler::IMergeReadHandler()
{}

IMergeReadHandler::~IMergeReadHandler()
{}

#endif
SrsFastBuffer::SrsFastBuffer()
{
    #ifdef SRS_PERF_MERGED_READ
    merged_read = false;
    _handler    = NULL;
    #endif
    nb_buffer = SRS_DEFAULT_RECV_BUFFER_SIZE;
    buffer    = (char *) malloc(nb_buffer);
    p         = end = buffer;
}

SrsFastBuffer::~SrsFastBuffer()
{
    free(buffer);
    buffer = NULL;
}

int SrsFastBuffer::size()
{
    return (int) (end - p);
}

char * SrsFastBuffer::bytes()
{
    return p;
}

void SrsFastBuffer::set_buffer(int buffer_size)
{
    if (buffer_size > SRS_MAX_SOCKET_BUFFER) {
        srs_warn("limit the user-space buffer from %d to %d",
                 buffer_size, SRS_MAX_SOCKET_BUFFER);
    }
    int nb_resize_buf = srs_min(buffer_size, SRS_MAX_SOCKET_BUFFER);
    if (nb_resize_buf <= nb_buffer) {
        return;
    }
    int start    = (int) (p - buffer);
    int nb_bytes = (int) (end - p);
    buffer    = (char *) realloc(buffer, nb_resize_buf);
    nb_buffer = nb_resize_buf;
    p         = buffer + start;
    end       = p + nb_bytes;
}

char SrsFastBuffer::read_1byte()
{
    srs_assert(end - p >= 1);
    return *p++;
}

char * SrsFastBuffer::read_slice(int size)
{
    srs_assert(size >= 0);
    srs_assert(end - p >= size);
    srs_assert(p + size >= buffer);
    char* ptr = p;
    p += size;
    return ptr;
}

void SrsFastBuffer::skip(int size)
{
    srs_assert(end - p >= size);
    srs_assert(p + size >= buffer);
    p += size;
}

int SrsFastBuffer::grow(ISrsBufferReader* reader, int required_size)
{
    int ret = ERROR_SUCCESS;

    if (end - p >= required_size) {
        return ret;
    }
    srs_assert(required_size > 0);
    int nb_free_space = (int) (buffer + nb_buffer - end);
    if (nb_free_space < required_size) {
        int nb_exists_bytes = (int) (end - p);
        srs_assert(nb_exists_bytes >= 0);
        srs_verbose("move fast buffer %d bytes", nb_exists_bytes);
        if (!nb_exists_bytes) {
            p = end = buffer;
            srs_verbose("all consumed, reset fast buffer");
        } else {
            srs_assert(nb_exists_bytes < nb_buffer);
            buffer = (char *) memmove(buffer, p, nb_exists_bytes);
            p      = buffer;
            end    = p + nb_exists_bytes;
        }
        nb_free_space = (int) (buffer + nb_buffer - end);
        if (nb_free_space < required_size) {
            ret = ERROR_READER_BUFFER_OVERFLOW;
            srs_error("buffer overflow, required=%d, max=%d, left=%d, ret=%d",
                      required_size, nb_buffer, nb_free_space, ret);
            return ret;
        }
    }
    while (end - p < required_size) {
        ssize_t nread;
        if ((ret = reader->read(end, nb_free_space, &nread)) != ERROR_SUCCESS) {
            return ret;
        }
        #ifdef SRS_PERF_MERGED_READ
        if (merged_read && _handler) {
            _handler->on_read(nread);
        }
        #endif
        srs_assert((int) nread > 0);
        end += nread;
        nb_free_space -= nread;
    }
    return ret;
} // SrsFastBuffer::grow

#ifdef SRS_PERF_MERGED_READ
void SrsFastBuffer::set_merge_read(bool v, IMergeReadHandler* handler)
{
    merged_read = v;
    _handler    = handler;
}

#endif
#include <string.h>
using namespace std;
SrsRawH264Stream::SrsRawH264Stream()
{}

SrsRawH264Stream::~SrsRawH264Stream()
{}

int SrsRawH264Stream::annexb_demux_pri(SrsStream* stream, char** pframe, int* pnb_frame)
{
    int ret = ERROR_SUCCESS;

    *pframe    = NULL;
    *pnb_frame = 0;
    while (!stream->empty()) {
        int pnb_start_code = 0;
        if (!srs_avc_startswith_annexb(stream, &pnb_start_code)) {
            return ERROR_H264_API_NO_PREFIXED;
        }
        int start = stream->pos() + pnb_start_code;
        int sk    = stream->size() - stream->pos();
        stream->skip(sk);
        *pnb_frame = stream->pos() - start;
        *pframe    = stream->data() + start;
        break;
    }
    return ret;
}

int SrsRawH264Stream::annexb_demux(SrsStream* stream, char** pframe, int* pnb_frame)
{
    int ret = ERROR_SUCCESS;

    *pframe    = NULL;
    *pnb_frame = 0;
    while (!stream->empty()) {
        int pnb_start_code = 0;
        if (!srs_avc_startswith_annexb(stream, &pnb_start_code)) {
            return ERROR_H264_API_NO_PREFIXED;
        }
        int start = stream->pos() + pnb_start_code;
        stream->skip(pnb_start_code);
        while (!stream->empty()) {
            if (srs_avc_startswith_annexb(stream, NULL)) {
                break;
            }
            stream->skip(1);
        }
        *pnb_frame = stream->pos() - start;
        *pframe    = stream->data() + start;
        break;
    }
    return ret;
}

bool SrsRawH264Stream::is_sps(char* frame, int nb_frame)
{
    srs_assert(nb_frame > 0);
    u_int8_t nal_unit_type = (char) frame[0] & 0x1f;
    return nal_unit_type == 7;
}

bool SrsRawH264Stream::is_pps(char* frame, int nb_frame)
{
    srs_assert(nb_frame > 0);
    u_int8_t nal_unit_type = (char) frame[0] & 0x1f;
    return nal_unit_type == 8;
}

int SrsRawH264Stream::sps_demux(char* frame, int nb_frame, string& sps)
{
    int ret = ERROR_SUCCESS;

    if (nb_frame < 4) {
        return ret;
    }
    sps = "";
    if (nb_frame > 0) {
        sps.append(frame, nb_frame);
    }
    if (sps.empty()) {
        return ERROR_STREAM_CASTER_AVC_SPS;
    }
    return ret;
}

int SrsRawH264Stream::pps_demux(char* frame, int nb_frame, string& pps)
{
    int ret = ERROR_SUCCESS;

    pps = "";
    if (nb_frame > 0) {
        pps.append(frame, nb_frame);
    }
    if (pps.empty()) {
        return ERROR_STREAM_CASTER_AVC_PPS;
    }
    return ret;
}

int SrsRawH264Stream::mux_sequence_header(string sps, string pps, u_int32_t dts, u_int32_t pts, string& sh)
{
    int ret       = ERROR_SUCCESS;
    int nb_packet = 5
                    + 3 + (int) sps.length()
                    + 3 + (int) pps.length();
    char* packet = new char[nb_packet];

    SrsAutoFreeA(char, packet);
    SrsStream stream;
    if ((ret = stream.initialize(packet, nb_packet)) != ERROR_SUCCESS) {
        return ret;
    }
    if (true) {
        srs_assert((int) sps.length() >= 4);
        char* frame = (char *) sps.data();
        u_int8_t profile_idc = frame[1];
        u_int8_t level_idc   = frame[3];
        stream.write_1bytes(0x01);
        stream.write_1bytes(profile_idc);
        stream.write_1bytes(0x00);
        stream.write_1bytes(level_idc);
        stream.write_1bytes(0x03);
    }
    if (true) {
        stream.write_1bytes(0x01);
        stream.write_2bytes(sps.length());
        stream.write_string(sps);
    }
    if (true) {
        stream.write_1bytes(0x01);
        stream.write_2bytes(pps.length());
        stream.write_string(pps);
    }
    sh = "";
    sh.append(packet, nb_packet);
    return ret;
} // SrsRawH264Stream::mux_sequence_header

int SrsRawH264Stream::mux_ipb_frame(char* frame, int nb_frame, string& ibp)
{
    int ret       = ERROR_SUCCESS;
    int nb_packet = 4 + nb_frame;
    char* packet  = new char[nb_packet];

    SrsAutoFreeA(char, packet);
    SrsStream stream;
    if ((ret = stream.initialize(packet, nb_packet)) != ERROR_SUCCESS) {
        return ret;
    }
    u_int32_t NAL_unit_length = nb_frame;
    stream.write_4bytes(NAL_unit_length);
    stream.write_bytes(frame, nb_frame);
    ibp = "";
    ibp.append(packet, nb_packet);
    return ret;
}

int SrsRawH264Stream::mux_avc2flv(string video, int8_t frame_type, int8_t avc_packet_type, u_int32_t dts, u_int32_t pts,
                                  char** flv, int* nb_flv)
{
    int ret    = ERROR_SUCCESS;
    int size   = (int) video.length() + 5;
    char* data = new char[size];
    char* p    = data;

    *p++ = (frame_type << 4) | SrsCodecVideoAVC;
    *p++ = avc_packet_type;
    u_int32_t cts = pts - dts;
    char* pp      = (char *) &cts;
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
    memcpy(p, video.data(), video.length());
    *flv    = data;
    *nb_flv = size;
    return ret;
}

SrsRawAacStream::SrsRawAacStream()
{}

SrsRawAacStream::~SrsRawAacStream()
{}

int SrsRawAacStream::adts_demux(SrsStream* stream, char** pframe, int* pnb_frame, SrsRawAacStreamCodec& codec)
{
    int ret = ERROR_SUCCESS;

    while (!stream->empty()) {
        int adts_header_start = stream->pos();
        if (!stream->require(7)) {
            return ERROR_AAC_ADTS_HEADER;
        }
        if (!srs_aac_startswith_adts(stream)) {
            return ERROR_AAC_REQUIRED_ADTS;
        }
        stream->read_1bytes();
        int8_t pav = (stream->read_1bytes() & 0x0f);
        int8_t id  = (pav >> 3) & 0x01;
        int8_t protection_absent = pav & 0x01;
        if (id != 0x01) {
            srs_info("adts: id must be 1(aac), actual 0(mp4a). ret=%d", ret);
            id = 0x01;
        }
        int16_t sfiv   = stream->read_2bytes();
        int8_t profile = (sfiv >> 14) & 0x03;
        int8_t sampling_frequency_index = (sfiv >> 10) & 0x0f;
        int8_t channel_configuration    = (sfiv >> 6) & 0x07;
        int16_t frame_length = (sfiv << 11) & 0x1800;
        int32_t abfv         = stream->read_3bytes();
        frame_length |= (abfv >> 13) & 0x07ff;
        if (!protection_absent) {
            if (!stream->require(2)) {
                return ERROR_AAC_ADTS_HEADER;
            }
            stream->read_2bytes();
        }
        int adts_header_size = stream->pos() - adts_header_start;
        int raw_data_size    = frame_length - adts_header_size;
        if (!stream->require(raw_data_size)) {
            return ERROR_AAC_ADTS_HEADER;
        }
        codec.protection_absent        = protection_absent;
        codec.aac_object               = srs_codec_aac_ts2rtmp((SrsAacProfile) profile);
        codec.sampling_frequency_index = sampling_frequency_index;
        codec.channel_configuration    = channel_configuration;
        codec.frame_length             = frame_length;
        codec.sound_format             = 10;
        if (sampling_frequency_index <= 0x0c && sampling_frequency_index > 0x0a) {
            codec.sound_rate = SrsCodecAudioSampleRate5512;
        } else if (sampling_frequency_index <= 0x0a && sampling_frequency_index > 0x07) {
            codec.sound_rate = SrsCodecAudioSampleRate11025;
        } else if (sampling_frequency_index <= 0x07 && sampling_frequency_index > 0x04) {
            codec.sound_rate = SrsCodecAudioSampleRate22050;
        } else if (sampling_frequency_index <= 0x04) {
            codec.sound_rate = SrsCodecAudioSampleRate44100;
        } else {
            codec.sound_rate = SrsCodecAudioSampleRate44100;
            srs_warn("adts invalid sample rate for flv, rate=%#x", sampling_frequency_index);
        }
        codec.sound_type = srs_max(0, srs_min(1, channel_configuration - 1));
        codec.sound_size = 1;
        *pframe    = stream->data() + stream->pos();
        *pnb_frame = raw_data_size;
        stream->skip(raw_data_size);
        break;
    }
    return ret;
} // SrsRawAacStream::adts_demux

int SrsRawAacStream::mux_sequence_header(SrsRawAacStreamCodec* codec, string& sh)
{
    int ret = ERROR_SUCCESS;

    if (codec->aac_object == SrsAacObjectTypeReserved) {
        return ERROR_AAC_DATA_INVALID;
    }
    SrsAacObjectType audioObjectType = codec->aac_object;
    char channelConfiguration        = codec->channel_configuration;
    char samplingFrequencyIndex      = codec->sampling_frequency_index;
    switch (codec->sound_rate) {
        case SrsCodecAudioSampleRate11025:
            samplingFrequencyIndex = 0x0a;
            break;
        case SrsCodecAudioSampleRate22050:
            samplingFrequencyIndex = 0x07;
            break;
        case SrsCodecAudioSampleRate44100:
            samplingFrequencyIndex = 0x04;
            break;
        default:
            break;
    }
    sh = "";
    char ch = 0;
    ch  = (audioObjectType << 3) & 0xf8;
    ch |= (samplingFrequencyIndex >> 1) & 0x07;
    sh += ch;
    ch  = (samplingFrequencyIndex << 7) & 0x80;
    if (samplingFrequencyIndex == 0x0f) {
        return ERROR_AAC_DATA_INVALID;
    }
    ch |= (channelConfiguration << 3) & 0x78;
    sh += ch;
    return ret;
} // SrsRawAacStream::mux_sequence_header

int SrsRawAacStream::mux_aac2flv(char* frame, int nb_frame, SrsRawAacStreamCodec* codec, u_int32_t dts, char** flv,
                                 int* nb_flv)
{
    int ret = ERROR_SUCCESS;
    char sound_format    = codec->sound_format;
    char sound_type      = codec->sound_type;
    char sound_size      = codec->sound_size;
    char sound_rate      = codec->sound_rate;
    char aac_packet_type = codec->aac_packet_type;
    int size = nb_frame + 1;

    if (sound_format == SrsCodecAudioAAC) {
        size += 1;
    }
    char* data = new char[size];
    char* p    = data;
    u_int8_t audio_header = sound_type & 0x01;
    audio_header |= (sound_size << 1) & 0x02;
    audio_header |= (sound_rate << 2) & 0x0c;
    audio_header |= (sound_format << 4) & 0xf0;
    *p++          = audio_header;
    if (sound_format == SrsCodecAudioAAC) {
        *p++ = aac_packet_type;
    }
    memcpy(p, frame, nb_frame);
    *flv    = data;
    *nb_flv = size;
    return ret;
}

#if !defined(SRS_EXPORT_LIBRTMP)
# include <stdlib.h>
# include <map>
using namespace std;
# ifdef SRS_AUTO_STREAM_CASTER
#  define SRS_RTSP_BUFFER 4096
string srs_generate_rtsp_status_text(int status)
{
    static std::map<int, std::string> _status_map;

    if (_status_map.empty()) {
        _status_map[SRS_CONSTS_RTSP_Continue                       ] = SRS_CONSTS_RTSP_Continue_str;
        _status_map[SRS_CONSTS_RTSP_OK                             ] = SRS_CONSTS_RTSP_OK_str;
        _status_map[SRS_CONSTS_RTSP_Created                        ] = SRS_CONSTS_RTSP_Created_str;
        _status_map[SRS_CONSTS_RTSP_LowOnStorageSpace              ] = SRS_CONSTS_RTSP_LowOnStorageSpace_str;
        _status_map[SRS_CONSTS_RTSP_MultipleChoices                ] = SRS_CONSTS_RTSP_MultipleChoices_str;
        _status_map[SRS_CONSTS_RTSP_MovedPermanently               ] = SRS_CONSTS_RTSP_MovedPermanently_str;
        _status_map[SRS_CONSTS_RTSP_MovedTemporarily               ] = SRS_CONSTS_RTSP_MovedTemporarily_str;
        _status_map[SRS_CONSTS_RTSP_SeeOther                       ] = SRS_CONSTS_RTSP_SeeOther_str;
        _status_map[SRS_CONSTS_RTSP_NotModified                    ] = SRS_CONSTS_RTSP_NotModified_str;
        _status_map[SRS_CONSTS_RTSP_UseProxy                       ] = SRS_CONSTS_RTSP_UseProxy_str;
        _status_map[SRS_CONSTS_RTSP_BadRequest                     ] = SRS_CONSTS_RTSP_BadRequest_str;
        _status_map[SRS_CONSTS_RTSP_Unauthorized                   ] = SRS_CONSTS_RTSP_Unauthorized_str;
        _status_map[SRS_CONSTS_RTSP_PaymentRequired                ] = SRS_CONSTS_RTSP_PaymentRequired_str;
        _status_map[SRS_CONSTS_RTSP_Forbidden                      ] = SRS_CONSTS_RTSP_Forbidden_str;
        _status_map[SRS_CONSTS_RTSP_NotFound                       ] = SRS_CONSTS_RTSP_NotFound_str;
        _status_map[SRS_CONSTS_RTSP_MethodNotAllowed               ] = SRS_CONSTS_RTSP_MethodNotAllowed_str;
        _status_map[SRS_CONSTS_RTSP_NotAcceptable                  ] = SRS_CONSTS_RTSP_NotAcceptable_str;
        _status_map[SRS_CONSTS_RTSP_ProxyAuthenticationRequired    ] = SRS_CONSTS_RTSP_ProxyAuthenticationRequired_str;
        _status_map[SRS_CONSTS_RTSP_RequestTimeout                 ] = SRS_CONSTS_RTSP_RequestTimeout_str;
        _status_map[SRS_CONSTS_RTSP_Gone                           ] = SRS_CONSTS_RTSP_Gone_str;
        _status_map[SRS_CONSTS_RTSP_LengthRequired                 ] = SRS_CONSTS_RTSP_LengthRequired_str;
        _status_map[SRS_CONSTS_RTSP_PreconditionFailed             ] = SRS_CONSTS_RTSP_PreconditionFailed_str;
        _status_map[SRS_CONSTS_RTSP_RequestEntityTooLarge          ] = SRS_CONSTS_RTSP_RequestEntityTooLarge_str;
        _status_map[SRS_CONSTS_RTSP_RequestURITooLarge             ] = SRS_CONSTS_RTSP_RequestURITooLarge_str;
        _status_map[SRS_CONSTS_RTSP_UnsupportedMediaType           ] = SRS_CONSTS_RTSP_UnsupportedMediaType_str;
        _status_map[SRS_CONSTS_RTSP_ParameterNotUnderstood         ] = SRS_CONSTS_RTSP_ParameterNotUnderstood_str;
        _status_map[SRS_CONSTS_RTSP_ConferenceNotFound             ] = SRS_CONSTS_RTSP_ConferenceNotFound_str;
        _status_map[SRS_CONSTS_RTSP_NotEnoughBandwidth             ] = SRS_CONSTS_RTSP_NotEnoughBandwidth_str;
        _status_map[SRS_CONSTS_RTSP_SessionNotFound                ] = SRS_CONSTS_RTSP_SessionNotFound_str;
        _status_map[SRS_CONSTS_RTSP_MethodNotValidInThisState      ] = SRS_CONSTS_RTSP_MethodNotValidInThisState_str;
        _status_map[SRS_CONSTS_RTSP_HeaderFieldNotValidForResource ] =
            SRS_CONSTS_RTSP_HeaderFieldNotValidForResource_str;
        _status_map[SRS_CONSTS_RTSP_InvalidRange                   ] = SRS_CONSTS_RTSP_InvalidRange_str;
        _status_map[SRS_CONSTS_RTSP_ParameterIsReadOnly            ] = SRS_CONSTS_RTSP_ParameterIsReadOnly_str;
        _status_map[SRS_CONSTS_RTSP_AggregateOperationNotAllowed   ] = SRS_CONSTS_RTSP_AggregateOperationNotAllowed_str;
        _status_map[SRS_CONSTS_RTSP_OnlyAggregateOperationAllowed  ] =
            SRS_CONSTS_RTSP_OnlyAggregateOperationAllowed_str;
        _status_map[SRS_CONSTS_RTSP_UnsupportedTransport           ] = SRS_CONSTS_RTSP_UnsupportedTransport_str;
        _status_map[SRS_CONSTS_RTSP_DestinationUnreachable         ] = SRS_CONSTS_RTSP_DestinationUnreachable_str;
        _status_map[SRS_CONSTS_RTSP_InternalServerError            ] = SRS_CONSTS_RTSP_InternalServerError_str;
        _status_map[SRS_CONSTS_RTSP_NotImplemented                 ] = SRS_CONSTS_RTSP_NotImplemented_str;
        _status_map[SRS_CONSTS_RTSP_BadGateway                     ] = SRS_CONSTS_RTSP_BadGateway_str;
        _status_map[SRS_CONSTS_RTSP_ServiceUnavailable             ] = SRS_CONSTS_RTSP_ServiceUnavailable_str;
        _status_map[SRS_CONSTS_RTSP_GatewayTimeout                 ] = SRS_CONSTS_RTSP_GatewayTimeout_str;
        _status_map[SRS_CONSTS_RTSP_RTSPVersionNotSupported        ] = SRS_CONSTS_RTSP_RTSPVersionNotSupported_str;
        _status_map[SRS_CONSTS_RTSP_OptionNotSupported             ] = SRS_CONSTS_RTSP_OptionNotSupported_str;
    }
    std::string status_text;
    if (_status_map.find(status) == _status_map.end()) {
        status_text = "Status Unknown";
    } else {
        status_text = _status_map[status];
    }
    return status_text;
} // srs_generate_rtsp_status_text

std::string srs_generate_rtsp_method_str(SrsRtspMethod method)
{
    switch (method) {
        case SrsRtspMethodDescribe:
            return SRS_METHOD_DESCRIBE;

        case SrsRtspMethodAnnounce:
            return SRS_METHOD_ANNOUNCE;

        case SrsRtspMethodGetParameter:
            return SRS_METHOD_GET_PARAMETER;

        case SrsRtspMethodOptions:
            return SRS_METHOD_OPTIONS;

        case SrsRtspMethodPause:
            return SRS_METHOD_PAUSE;

        case SrsRtspMethodPlay:
            return SRS_METHOD_PLAY;

        case SrsRtspMethodRecord:
            return SRS_METHOD_RECORD;

        case SrsRtspMethodRedirect:
            return SRS_METHOD_REDIRECT;

        case SrsRtspMethodSetup:
            return SRS_METHOD_SETUP;

        case SrsRtspMethodSetParameter:
            return SRS_METHOD_SET_PARAMETER;

        case SrsRtspMethodTeardown:
            return SRS_METHOD_TEARDOWN;

        default:
            return "Unknown";
    }
} // srs_generate_rtsp_method_str

SrsRtpPacket::SrsRtpPacket()
{
    version         = 2;
    padding         = 0;
    extension       = 0;
    csrc_count      = 0;
    marker          = 1;
    payload_type    = 0;
    sequence_number = 0;
    timestamp       = 0;
    ssrc          = 0;
    payload       = new SrsSimpleBuffer();
    audio_samples = new SrsCodecSample();
    chunked       = false;
    completed     = false;
}

SrsRtpPacket::~SrsRtpPacket()
{
    srs_freep(payload);
    srs_freep(audio_samples);
}

void SrsRtpPacket::copy(SrsRtpPacket* src)
{
    version         = src->version;
    padding         = src->padding;
    extension       = src->extension;
    csrc_count      = src->csrc_count;
    marker          = src->marker;
    payload_type    = src->payload_type;
    sequence_number = src->sequence_number;
    timestamp       = src->timestamp;
    ssrc          = src->ssrc;
    chunked       = src->chunked;
    completed     = src->completed;
    audio_samples = new SrsCodecSample();
}

void SrsRtpPacket::reap(SrsRtpPacket* src)
{
    copy(src);
    srs_freep(payload);
    payload      = src->payload;
    src->payload = NULL;
    srs_freep(audio_samples);
    audio_samples      = src->audio_samples;
    src->audio_samples = NULL;
}

int SrsRtpPacket::decode(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(12)) {
        ret = ERROR_RTP_HEADER_CORRUPT;
        srs_error("rtsp: rtp header corrupt. ret=%d", ret);
        return ret;
    }
    int8_t vv = stream->read_1bytes();
    version    = (vv >> 6) & 0x03;
    padding    = (vv >> 5) & 0x01;
    extension  = (vv >> 4) & 0x01;
    csrc_count = vv & 0x0f;
    int8_t mv = stream->read_1bytes();
    marker          = (mv >> 7) & 0x01;
    payload_type    = mv & 0x7f;
    sequence_number = stream->read_2bytes();
    timestamp       = stream->read_4bytes();
    ssrc = stream->read_4bytes();
    if (payload_type == 96) {
        return decode_96(stream);
    } else if (payload_type == 97) {
        return decode_97(stream);
    }
    return ret;
}

int SrsRtpPacket::decode_97(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(2)) {
        ret = ERROR_RTP_TYPE97_CORRUPT;
        srs_error("rtsp: rtp type97 corrupt. ret=%d", ret);
        return ret;
    }
    int8_t hasv       = stream->read_1bytes();
    int8_t lasv       = stream->read_1bytes();
    u_int16_t au_size = ((hasv << 5) & 0xE0) | ((lasv >> 3) & 0x1f);
    if (!stream->require(au_size)) {
        ret = ERROR_RTP_TYPE97_CORRUPT;
        srs_error("rtsp: rtp type97 au_size corrupt. ret=%d", ret);
        return ret;
    }
    int required_size = 0;
    payload->append(
        stream->data() + stream->pos() + au_size,
        stream->size() - stream->pos() - au_size
    );
    char* p = payload->bytes();
    for (int i = 0; i < au_size; i += 2) {
        hasv = stream->read_1bytes();
        lasv = stream->read_1bytes();
        u_int16_t sample_size = ((hasv << 5) & 0xE0) | ((lasv >> 3) & 0x1f);
        if (sample_size < 0x100 && stream->require(required_size + sample_size + 0x100)) {
            sample_size = sample_size | 0x100;
        }
        char* sample = p + required_size;
        required_size += sample_size;
        if (!stream->require(required_size)) {
            ret = ERROR_RTP_TYPE97_CORRUPT;
            srs_error("rtsp: rtp type97 samples corrupt. ret=%d", ret);
            return ret;
        }
        if ((ret = audio_samples->add_sample_unit(sample, sample_size)) != ERROR_SUCCESS) {
            srs_error("rtsp: rtp type97 add sample failed. ret=%d", ret);
            return ret;
        }
    }
    completed = true;
    return ret;
} // SrsRtpPacket::decode_97

int SrsRtpPacket::decode_96(SrsStream* stream)
{
    int ret = ERROR_SUCCESS;

    if (!stream->require(2)) {
        ret = ERROR_RTP_TYPE96_CORRUPT;
        srs_error("rtsp: rtp type96 corrupt. ret=%d", ret);
        return ret;
    }
    int8_t ftv          = stream->read_1bytes();
    int8_t nalu_0x60    = ftv & 0x60;
    int8_t fu_indicator = ftv & 0x1c;
    int8_t nriv         = stream->read_1bytes();
    bool first_chunk    = (nriv & 0xC0) == 0x80;
    bool last_chunk     = (nriv & 0xC0) == 0x40;
    bool contious_chunk = (nriv & 0xC0) == 0x00;
    int8_t nalu_0x1f    = nriv & 0x1f;
    if (fu_indicator == 0x1c && (first_chunk || last_chunk || contious_chunk)) {
        chunked   = true;
        completed = last_chunk;
        if (first_chunk) {
            int8_t nalu_byte0 = nalu_0x60 | nalu_0x1f;
            payload->append((char *) &nalu_byte0, 1);
        }
        payload->append(stream->data() + stream->pos(), stream->size() - stream->pos());
        return ret;
    }
    stream->skip(-2);
    payload->append(stream->data() + stream->pos(), stream->size() - stream->pos());
    completed = true;
    return ret;
} // SrsRtpPacket::decode_96

SrsRtspSdp::SrsRtspSdp()
{
    state = SrsRtspSdpStateOthers;
}

SrsRtspSdp::~SrsRtspSdp()
{}

int SrsRtspSdp::parse(string token)
{
    int ret = ERROR_SUCCESS;

    if (token.empty()) {
        srs_info("rtsp: ignore empty token.");
        return ret;
    }
    size_t pos  = string::npos;
    char* start = (char *) token.data();
    char* end   = start + (int) token.length();
    char* p     = start;
    char key    = p[0];
    p += 2;
    std::string attr_str;
    if (end - p) {
        attr_str.append(p, end - p);
    }
    std::vector<std::string> attrs;
    while (p < end) {
        char* pa = p;
        for (; p < end && p[0] != SRS_RTSP_SP; p++) {}
        std::string attr;
        if (p > pa) {
            attr.append(pa, p - pa);
            attrs.push_back(attr);
        }
        p++;
    }
    std::string desc_key;
    if (attrs.size() > 0) {
        std::string attr = attrs.at(0);
        if ((pos = attr.find(":")) != string::npos) {
            desc_key = attr.substr(0, pos);
            attr     = attr.substr(pos + 1);
            attr_str = attr_str.substr(pos + 1);
            attrs[0] = attr;
        } else {
            desc_key = attr;
        }
    }
    switch (key) {
        case 'v':
            version = attr_str;
            break;
        case 'o':
            owner_username        = (attrs.size() > 0) ? attrs[0] : "";
            owner_session_id      = (attrs.size() > 1) ? attrs[1] : "";
            owner_session_version = (attrs.size() > 2) ? attrs[2] : "";
            owner_network_type    = (attrs.size() > 3) ? attrs[3] : "";
            owner_address_type    = (attrs.size() > 4) ? attrs[4] : "";
            owner_address         = (attrs.size() > 5) ? attrs[5] : "";
            break;
        case 's':
            session_name = attr_str;
            break;
        case 'c':
            connection_network_type = (attrs.size() > 0) ? attrs[0] : "";
            connection_address_type = (attrs.size() > 0) ? attrs[0] : "";
            connection_address      = (attrs.size() > 0) ? attrs[0] : "";
            break;
        case 'a':
            if (desc_key == "tool") {
                tool = attr_str;
            } else if (desc_key == "rtpmap") {
                if (state == SrsRtspSdpStateVideo) {
                    video_codec = (attrs.size() > 1) ? attrs[1] : "";
                    if ((pos = video_codec.find("/")) != string::npos) {
                        video_sample_rate = video_codec.substr(pos + 1);
                        video_codec       = video_codec.substr(0, pos);
                    }
                } else if (state == SrsRtspSdpStateAudio) {
                    audio_codec = (attrs.size() > 1) ? attrs[1] : "";
                    if ((pos = audio_codec.find("/")) != string::npos) {
                        audio_sample_rate = audio_codec.substr(pos + 1);
                        audio_codec       = audio_codec.substr(0, pos);
                    }
                    if ((pos = audio_sample_rate.find("/")) != string::npos) {
                        audio_channel     = audio_sample_rate.substr(pos + 1);
                        audio_sample_rate = audio_sample_rate.substr(0, pos);
                    }
                }
            } else if (desc_key == "fmtp") {
                for (int i = 1; i < (int) attrs.size(); i++) {
                    std::string attr = attrs.at(i);
                    if ((ret = parse_fmtp_attribute(attr)) != ERROR_SUCCESS) {
                        srs_error("rtsp: parse fmtp failed, attr=%s. ret=%d", attr.c_str(), ret);
                        return ret;
                    }
                }
            } else if (desc_key == "control") {
                for (int i = 0; i < (int) attrs.size(); i++) {
                    std::string attr = attrs.at(i);
                    if ((ret = parse_control_attribute(attr)) != ERROR_SUCCESS) {
                        srs_error("rtsp: parse control failed, attr=%s. ret=%d", attr.c_str(), ret);
                        return ret;
                    }
                }
            }
            break;
        case 'm':
            if (desc_key == "video") {
                state                  = SrsRtspSdpStateVideo;
                video_port             = (attrs.size() > 1) ? attrs[1] : "";
                video_protocol         = (attrs.size() > 2) ? attrs[2] : "";
                video_transport_format = (attrs.size() > 3) ? attrs[3] : "";
            } else if (desc_key == "audio") {
                state                  = SrsRtspSdpStateAudio;
                audio_port             = (attrs.size() > 1) ? attrs[1] : "";
                audio_protocol         = (attrs.size() > 2) ? attrs[2] : "";
                audio_transport_format = (attrs.size() > 3) ? attrs[3] : "";
            }
            break;
        case 'b':
            if (desc_key == "AS") {
                if (state == SrsRtspSdpStateVideo) {
                    video_bandwidth_kbps = (attrs.size() > 0) ? attrs[0] : "";
                } else if (state == SrsRtspSdpStateAudio) {
                    audio_bandwidth_kbps = (attrs.size() > 0) ? attrs[0] : "";
                }
            }
            break;
        case 't':
        default:
            break;
    }
    return ret;
} // SrsRtspSdp::parse

int SrsRtspSdp::parse_fmtp_attribute(string attr)
{
    int ret           = ERROR_SUCCESS;
    size_t pos        = string::npos;
    std::string token = attr;

    while (!token.empty()) {
        std::string item = token;
        if ((pos = item.find(";")) != string::npos) {
            item  = token.substr(0, pos);
            token = token.substr(pos + 1);
        } else {
            token = "";
        }
        std::string item_key = item, item_value;
        if ((pos = item.find("=")) != string::npos) {
            item_key   = item.substr(0, pos);
            item_value = item.substr(pos + 1);
        }
        if (state == SrsRtspSdpStateVideo) {
            if (item_key == "packetization-mode") {
                video_packetization_mode = item_value;
            } else if (item_key == "sprop-parameter-sets") {
                video_sps = item_value;
                if ((pos = video_sps.find(",")) != string::npos) {
                    video_pps = video_sps.substr(pos + 1);
                    video_sps = video_sps.substr(0, pos);
                }
                video_sps = base64_decode(video_sps);
                video_pps = base64_decode(video_pps);
            }
        } else if (state == SrsRtspSdpStateAudio) {
            if (item_key == "profile-level-id") {
                audio_profile_level_id = item_value;
            } else if (item_key == "mode") {
                audio_mode = item_value;
            } else if (item_key == "sizelength") {
                audio_size_length = item_value;
            } else if (item_key == "indexlength") {
                audio_index_length = item_value;
            } else if (item_key == "indexdeltalength") {
                audio_index_delta_length = item_value;
            } else if (item_key == "config") {
                if (item_value.length() <= 0) {
                    ret = ERROR_RTSP_AUDIO_CONFIG;
                    srs_error("rtsp: audio config failed. ret=%d", ret);
                    return ret;
                }
                char* tmp_sh = new char[item_value.length()];
                SrsAutoFreeA(char, tmp_sh);
                int nb_tmp_sh = ff_hex_to_data((u_int8_t *) tmp_sh, item_value.c_str());
                srs_assert(nb_tmp_sh > 0);
                audio_sh.append(tmp_sh, nb_tmp_sh);
            }
        }
    }
    return ret;
} // SrsRtspSdp::parse_fmtp_attribute

int SrsRtspSdp::parse_control_attribute(string attr)
{
    int ret           = ERROR_SUCCESS;
    size_t pos        = string::npos;
    std::string token = attr;

    while (!token.empty()) {
        std::string item = token;
        if ((pos = item.find(";")) != string::npos) {
            item  = token.substr(0, pos);
            token = token.substr(pos + 1);
        } else {
            token = "";
        }
        std::string item_key = item, item_value;
        if ((pos = item.find("=")) != string::npos) {
            item_key   = item.substr(0, pos);
            item_value = item.substr(pos + 1);
        }
        if (state == SrsRtspSdpStateVideo) {
            if (item_key == "streamid") {
                video_stream_id = item_value;
            }
        } else if (state == SrsRtspSdpStateAudio) {
            if (item_key == "streamid") {
                audio_stream_id = item_value;
            }
        }
    }
    return ret;
}

string SrsRtspSdp::base64_decode(string value)
{
    if (value.empty()) {
        return "";
    }
    int nb_output    = (int) (value.length() * 2);
    u_int8_t* output = new u_int8_t[nb_output];
    SrsAutoFreeA(u_int8_t, output);
    int ret = srs_av_base64_decode(output, (char *) value.c_str(), nb_output);
    if (ret <= 0) {
        return "";
    }
    std::string plaintext;
    plaintext.append((char *) output, ret);
    return plaintext;
}

SrsRtspTransport::SrsRtspTransport()
{
    client_port_min = 0;
    client_port_max = 0;
}

SrsRtspTransport::~SrsRtspTransport()
{}

int SrsRtspTransport::parse(string attr)
{
    int ret           = ERROR_SUCCESS;
    size_t pos        = string::npos;
    std::string token = attr;

    while (!token.empty()) {
        std::string item = token;
        if ((pos = item.find(";")) != string::npos) {
            item  = token.substr(0, pos);
            token = token.substr(pos + 1);
        } else {
            token = "";
        }
        std::string item_key = item, item_value;
        if ((pos = item.find("=")) != string::npos) {
            item_key   = item.substr(0, pos);
            item_value = item.substr(pos + 1);
        }
        if (transport.empty()) {
            transport = item_key;
            if ((pos = transport.find("/")) != string::npos) {
                profile   = transport.substr(pos + 1);
                transport = transport.substr(0, pos);
            }
            if ((pos = profile.find("/")) != string::npos) {
                lower_transport = profile.substr(pos + 1);
                profile         = profile.substr(0, pos);
            }
        }
        if (item_key == "unicast" || item_key == "multicast") {
            cast_type = item_key;
        } else if (item_key == "mode") {
            mode = item_value;
        } else if (item_key == "client_port") {
            std::string sport = item_value;
            std::string eport = item_value;
            if ((pos = eport.find("-")) != string::npos) {
                sport = eport.substr(0, pos);
                eport = eport.substr(pos + 1);
            }
            client_port_min = ::atoi(sport.c_str());
            client_port_max = ::atoi(eport.c_str());
        }
    }
    return ret;
} // SrsRtspTransport::parse

SrsRtspRequest::SrsRtspRequest()
{
    seq = 0;
    content_length = 0;
    stream_id      = 0;
    sdp       = NULL;
    transport = NULL;
}

SrsRtspRequest::~SrsRtspRequest()
{
    srs_freep(sdp);
    srs_freep(transport);
}

bool SrsRtspRequest::is_options()
{
    return method == SRS_METHOD_OPTIONS;
}

bool SrsRtspRequest::is_announce()
{
    return method == SRS_METHOD_ANNOUNCE;
}

bool SrsRtspRequest::is_setup()
{
    return method == SRS_METHOD_SETUP;
}

bool SrsRtspRequest::is_record()
{
    return method == SRS_METHOD_RECORD;
}

SrsRtspResponse::SrsRtspResponse(int cseq)
{
    seq    = cseq;
    status = SRS_CONSTS_RTSP_OK;
}

SrsRtspResponse::~SrsRtspResponse()
{}

int SrsRtspResponse::encode(stringstream& ss)
{
    int ret = ERROR_SUCCESS;

    ss << SRS_RTSP_VERSION << SRS_RTSP_SP
       << status << SRS_RTSP_SP
       << srs_generate_rtsp_status_text(status) << SRS_RTSP_CRLF;
    ss << SRS_RTSP_TOKEN_CSEQ << ":" << SRS_RTSP_SP << seq << SRS_RTSP_CRLF;
    ss << "Cache-Control: no-store" << SRS_RTSP_CRLF
       << "Pragma: no-cache" << SRS_RTSP_CRLF
       << "Server: " << RTMP_SIG_SRS_SERVER << SRS_RTSP_CRLF;
    if (!session.empty()) {
        ss << SRS_RTSP_TOKEN_SESSION << ":" << session << SRS_RTSP_CRLF;
    }
    if ((ret = encode_header(ss)) != ERROR_SUCCESS) {
        srs_error("rtsp: encode header failed. ret=%d", ret);
        return ret;
    }
    ;
    ss << SRS_RTSP_CRLF;
    return ret;
}

int SrsRtspResponse::encode_header(std::stringstream& ss)
{
    return ERROR_SUCCESS;
}

SrsRtspOptionsResponse::SrsRtspOptionsResponse(int cseq) : SrsRtspResponse(cseq)
{
    methods = (SrsRtspMethod) (SrsRtspMethodDescribe | SrsRtspMethodOptions
                               | SrsRtspMethodPause | SrsRtspMethodPlay | SrsRtspMethodSetup | SrsRtspMethodTeardown
                               | SrsRtspMethodAnnounce | SrsRtspMethodRecord);
}

SrsRtspOptionsResponse::~SrsRtspOptionsResponse()
{}

int SrsRtspOptionsResponse::encode_header(stringstream& ss)
{
    SrsRtspMethod rtsp_methods[] = {
        SrsRtspMethodDescribe,
        SrsRtspMethodAnnounce,
        SrsRtspMethodGetParameter,
        SrsRtspMethodOptions,
        SrsRtspMethodPause,
        SrsRtspMethodPlay,
        SrsRtspMethodRecord,
        SrsRtspMethodRedirect,
        SrsRtspMethodSetup,
        SrsRtspMethodSetParameter,
        SrsRtspMethodTeardown,
    };

    ss << SRS_RTSP_TOKEN_PUBLIC << ":" << SRS_RTSP_SP;
    bool appended  = false;
    int nb_methods = (int) (sizeof(rtsp_methods) / sizeof(SrsRtspMethod));
    for (int i = 0; i < nb_methods; i++) {
        SrsRtspMethod method = rtsp_methods[i];
        if (((int) methods & (int) method) != (int) method) {
            continue;
        }
        if (appended) {
            ss << ", ";
        }
        ss << srs_generate_rtsp_method_str(method);
        appended = true;
    }
    ss << SRS_RTSP_CRLF;
    return ERROR_SUCCESS;
}

SrsRtspSetupResponse::SrsRtspSetupResponse(int seq) : SrsRtspResponse(seq)
{
    local_port_min = 0;
    local_port_max = 0;
}

SrsRtspSetupResponse::~SrsRtspSetupResponse()
{}

int SrsRtspSetupResponse::encode_header(stringstream& ss)
{
    ss << SRS_RTSP_TOKEN_SESSION << ":" << SRS_RTSP_SP << session << SRS_RTSP_CRLF;
    ss << SRS_RTSP_TOKEN_TRANSPORT << ":" << SRS_RTSP_SP
       << "RTP/AVP;unicast;client_port=" << client_port_min << "-" << client_port_max << ";"
       << "server_port=" << local_port_min << "-" << local_port_max
       << SRS_RTSP_CRLF;
    return ERROR_SUCCESS;
}

SrsRtspStack::SrsRtspStack(ISrsProtocolReaderWriter* s)
{
    buf = new SrsSimpleBuffer();
    skt = s;
}

SrsRtspStack::~SrsRtspStack()
{
    srs_freep(buf);
}

int SrsRtspStack::recv_message(SrsRtspRequest** preq)
{
    int ret = ERROR_SUCCESS;
    SrsRtspRequest* req = new SrsRtspRequest();

    if ((ret = do_recv_message(req)) != ERROR_SUCCESS) {
        srs_freep(req);
        return ret;
    }
    *preq = req;
    return ret;
}

int SrsRtspStack::send_message(SrsRtspResponse* res)
{
    int ret = ERROR_SUCCESS;
    std::stringstream ss;

    res->encode(ss);
    std::string str = ss.str();
    srs_assert(!str.empty());
    if ((ret = skt->write((char *) str.c_str(), (int) str.length(), NULL)) != ERROR_SUCCESS) {
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("rtsp: send response failed. ret=%d", ret);
        }
        return ret;
    }
    srs_info("rtsp: send response ok");
    return ret;
}

int SrsRtspStack::do_recv_message(SrsRtspRequest* req)
{
    int ret = ERROR_SUCCESS;

    if ((ret = recv_token_normal(req->method)) != ERROR_SUCCESS) {
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("rtsp: parse method failed. ret=%d", ret);
        }
        return ret;
    }
    if ((ret = recv_token_normal(req->uri)) != ERROR_SUCCESS) {
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("rtsp: parse uri failed. ret=%d", ret);
        }
        return ret;
    }
    if ((ret = recv_token_eof(req->version)) != ERROR_SUCCESS) {
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("rtsp: parse version failed. ret=%d", ret);
        }
        return ret;
    }
    for (;;) {
        std::string token;
        if ((ret = recv_token_normal(token)) != ERROR_SUCCESS) {
            if (ret == ERROR_RTSP_REQUEST_HEADER_EOF) {
                ret = ERROR_SUCCESS;
                srs_info("rtsp: message header parsed");
                break;
            }
            if (!srs_is_client_gracefully_close(ret)) {
                srs_error("rtsp: parse token failed. ret=%d", ret);
            }
            return ret;
        }
        if (token == SRS_RTSP_TOKEN_CSEQ) {
            std::string seq;
            if ((ret = recv_token_eof(seq)) != ERROR_SUCCESS) {
                if (!srs_is_client_gracefully_close(ret)) {
                    srs_error("rtsp: parse %s failed. ret=%d", SRS_RTSP_TOKEN_CSEQ, ret);
                }
                return ret;
            }
            req->seq = ::atol(seq.c_str());
        } else if (token == SRS_RTSP_TOKEN_CONTENT_TYPE) {
            std::string ct;
            if ((ret = recv_token_eof(ct)) != ERROR_SUCCESS) {
                if (!srs_is_client_gracefully_close(ret)) {
                    srs_error("rtsp: parse %s failed. ret=%d", SRS_RTSP_TOKEN_CONTENT_TYPE, ret);
                }
                return ret;
            }
            req->content_type = ct;
        } else if (token == SRS_RTSP_TOKEN_CONTENT_LENGTH) {
            std::string cl;
            if ((ret = recv_token_eof(cl)) != ERROR_SUCCESS) {
                if (!srs_is_client_gracefully_close(ret)) {
                    srs_error("rtsp: parse %s failed. ret=%d", SRS_RTSP_TOKEN_CONTENT_LENGTH, ret);
                }
                return ret;
            }
            req->content_length = ::atol(cl.c_str());
        } else if (token == SRS_RTSP_TOKEN_TRANSPORT) {
            std::string transport;
            if ((ret = recv_token_eof(transport)) != ERROR_SUCCESS) {
                if (!srs_is_client_gracefully_close(ret)) {
                    srs_error("rtsp: parse %s failed. ret=%d", SRS_RTSP_TOKEN_TRANSPORT, ret);
                }
                return ret;
            }
            if (!req->transport) {
                req->transport = new SrsRtspTransport();
            }
            if ((ret = req->transport->parse(transport)) != ERROR_SUCCESS) {
                srs_error("rtsp: parse transport failed, transport=%s. ret=%d", transport.c_str(), ret);
                return ret;
            }
        } else if (token == SRS_RTSP_TOKEN_SESSION) {
            if ((ret = recv_token_eof(req->session)) != ERROR_SUCCESS) {
                if (!srs_is_client_gracefully_close(ret)) {
                    srs_error("rtsp: parse %s failed. ret=%d", SRS_RTSP_TOKEN_SESSION, ret);
                }
                return ret;
            }
        } else {
            SrsRtspTokenState state = SrsRtspTokenStateNormal;
            while (state == SrsRtspTokenStateNormal) {
                std::string value;
                if ((ret = recv_token(value, state)) != ERROR_SUCCESS) {
                    if (!srs_is_client_gracefully_close(ret)) {
                        srs_error("rtsp: parse token failed. ret=%d", ret);
                    }
                    return ret;
                }
                srs_trace("rtsp: ignore header %s=%s", token.c_str(), value.c_str());
            }
        }
    }
    if (req->is_setup()) {
        size_t pos = string::npos;
        std::string stream_id;
        if ((pos = req->uri.rfind("/")) != string::npos) {
            stream_id = req->uri.substr(pos + 1);
        }
        if ((pos = stream_id.find("=")) != string::npos) {
            stream_id = stream_id.substr(pos + 1);
        }
        req->stream_id = ::atoi(stream_id.c_str());
        srs_info("rtsp: setup stream id=%d", req->stream_id);
    }
    long consumed = 0;
    while (consumed < req->content_length) {
        if (!req->sdp) {
            req->sdp = new SrsRtspSdp();
        }
        int nb_token = 0;
        std::string token;
        if ((ret = recv_token_util_eof(token, &nb_token)) != ERROR_SUCCESS) {
            if (!srs_is_client_gracefully_close(ret)) {
                srs_error("rtsp: parse sdp token failed. ret=%d", ret);
            }
            return ret;
        }
        consumed += nb_token;
        if ((ret = req->sdp->parse(token)) != ERROR_SUCCESS) {
            srs_error("rtsp: sdp parse token failed, token=%s. ret=%d", token.c_str(), ret);
            return ret;
        }
        srs_info("rtsp: %s", token.c_str());
    }
    srs_info("rtsp: sdp parsed, size=%d", consumed);
    return ret;
} // SrsRtspStack::do_recv_message

int SrsRtspStack::recv_token_normal(std::string& token)
{
    int ret = ERROR_SUCCESS;
    SrsRtspTokenState state;

    if ((ret = recv_token(token, state)) != ERROR_SUCCESS) {
        if (ret == ERROR_RTSP_REQUEST_HEADER_EOF) {
            return ret;
        }
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("rtsp: parse token failed. ret=%d", ret);
        }
        return ret;
    }
    if (state != SrsRtspTokenStateNormal) {
        ret = ERROR_RTSP_TOKEN_NOT_NORMAL;
        srs_error("rtsp: parse normal token failed, state=%d. ret=%d", state, ret);
        return ret;
    }
    return ret;
}

int SrsRtspStack::recv_token_eof(std::string& token)
{
    int ret = ERROR_SUCCESS;
    SrsRtspTokenState state;

    if ((ret = recv_token(token, state)) != ERROR_SUCCESS) {
        if (ret == ERROR_RTSP_REQUEST_HEADER_EOF) {
            return ret;
        }
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("rtsp: parse token failed. ret=%d", ret);
        }
        return ret;
    }
    if (state != SrsRtspTokenStateEOF) {
        ret = ERROR_RTSP_TOKEN_NOT_NORMAL;
        srs_error("rtsp: parse eof token failed, state=%d. ret=%d", state, ret);
        return ret;
    }
    return ret;
}

int SrsRtspStack::recv_token_util_eof(std::string& token, int* pconsumed)
{
    int ret = ERROR_SUCCESS;
    SrsRtspTokenState state;

    if ((ret = recv_token(token, state, 0x00, pconsumed)) != ERROR_SUCCESS) {
        if (ret == ERROR_RTSP_REQUEST_HEADER_EOF) {
            return ret;
        }
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("rtsp: parse token failed. ret=%d", ret);
        }
        return ret;
    }
    if (state != SrsRtspTokenStateEOF) {
        ret = ERROR_RTSP_TOKEN_NOT_NORMAL;
        srs_error("rtsp: parse eof token failed, state=%d. ret=%d", state, ret);
        return ret;
    }
    return ret;
}

int SrsRtspStack::recv_token(std::string& token, SrsRtspTokenState& state, char normal_ch, int* pconsumed)
{
    int ret = ERROR_SUCCESS;

    state = SrsRtspTokenStateError;
    bool append_bytes = buf->length() == 0;
    for (;;) {
        if (append_bytes) {
            append_bytes = false;
            char buffer[SRS_RTSP_BUFFER];
            ssize_t nb_read = 0;
            if ((ret = skt->read(buffer, SRS_RTSP_BUFFER, &nb_read)) != ERROR_SUCCESS) {
                if (!srs_is_client_gracefully_close(ret)) {
                    srs_error("rtsp: io read failed. ret=%d", ret);
                }
                return ret;
            }
            srs_info("rtsp: io read %d bytes", nb_read);
            buf->append(buffer, nb_read);
        }
        char* start = buf->bytes();
        char* end   = start + buf->length();
        char* p     = start;
        for (; p < end && p[0] != normal_ch && p[0] != SRS_RTSP_CR && p[0] != SRS_RTSP_LF; p++) {}
        if (p < end) {
            if (p[0] == normal_ch) {
                state = SrsRtspTokenStateNormal;
            } else {
                state = SrsRtspTokenStateEOF;
            }
            int nb_token = p - start;
            if (nb_token && p[-1] == ':') {
                nb_token--;
            }
            if (nb_token) {
                token.append(start, nb_token);
            } else {
                ret = ERROR_RTSP_REQUEST_HEADER_EOF;
            }
            for (int i = 0;
                 i < 2 && p < end && (p[0] == normal_ch || p[0] == SRS_RTSP_CR || p[0] == SRS_RTSP_LF);
                 p++, i++)
            {}
            srs_assert(p - start);
            buf->erase(p - start);
            if (pconsumed) {
                *pconsumed = p - start;
            }
            break;
        }
        append_bytes = true;
    }
    return ret;
} // SrsRtspStack::recv_token

# endif // ifdef SRS_AUTO_STREAM_CASTER
#endif  // if !defined(SRS_EXPORT_LIBRTMP)
#if !defined(SRS_EXPORT_LIBRTMP)
# include <stdlib.h>
# include <sstream>
# include <algorithm>
using namespace std;
# define SRS_HTTP_DEFAULT_PAGE "index.html"
string srs_generate_http_status_text(int status)
{
    static std::map<int, std::string> _status_map;

    if (_status_map.empty()) {
        _status_map[SRS_CONSTS_HTTP_Continue                       ] = SRS_CONSTS_HTTP_Continue_str;
        _status_map[SRS_CONSTS_HTTP_SwitchingProtocols             ] = SRS_CONSTS_HTTP_SwitchingProtocols_str;
        _status_map[SRS_CONSTS_HTTP_OK                             ] = SRS_CONSTS_HTTP_OK_str;
        _status_map[SRS_CONSTS_HTTP_Created                        ] = SRS_CONSTS_HTTP_Created_str;
        _status_map[SRS_CONSTS_HTTP_Accepted                       ] = SRS_CONSTS_HTTP_Accepted_str;
        _status_map[SRS_CONSTS_HTTP_NonAuthoritativeInformation    ] = SRS_CONSTS_HTTP_NonAuthoritativeInformation_str;
        _status_map[SRS_CONSTS_HTTP_NoContent                      ] = SRS_CONSTS_HTTP_NoContent_str;
        _status_map[SRS_CONSTS_HTTP_ResetContent                   ] = SRS_CONSTS_HTTP_ResetContent_str;
        _status_map[SRS_CONSTS_HTTP_PartialContent                 ] = SRS_CONSTS_HTTP_PartialContent_str;
        _status_map[SRS_CONSTS_HTTP_MultipleChoices                ] = SRS_CONSTS_HTTP_MultipleChoices_str;
        _status_map[SRS_CONSTS_HTTP_MovedPermanently               ] = SRS_CONSTS_HTTP_MovedPermanently_str;
        _status_map[SRS_CONSTS_HTTP_Found                          ] = SRS_CONSTS_HTTP_Found_str;
        _status_map[SRS_CONSTS_HTTP_SeeOther                       ] = SRS_CONSTS_HTTP_SeeOther_str;
        _status_map[SRS_CONSTS_HTTP_NotModified                    ] = SRS_CONSTS_HTTP_NotModified_str;
        _status_map[SRS_CONSTS_HTTP_UseProxy                       ] = SRS_CONSTS_HTTP_UseProxy_str;
        _status_map[SRS_CONSTS_HTTP_TemporaryRedirect              ] = SRS_CONSTS_HTTP_TemporaryRedirect_str;
        _status_map[SRS_CONSTS_HTTP_BadRequest                     ] = SRS_CONSTS_HTTP_BadRequest_str;
        _status_map[SRS_CONSTS_HTTP_Unauthorized                   ] = SRS_CONSTS_HTTP_Unauthorized_str;
        _status_map[SRS_CONSTS_HTTP_PaymentRequired                ] = SRS_CONSTS_HTTP_PaymentRequired_str;
        _status_map[SRS_CONSTS_HTTP_Forbidden                      ] = SRS_CONSTS_HTTP_Forbidden_str;
        _status_map[SRS_CONSTS_HTTP_NotFound                       ] = SRS_CONSTS_HTTP_NotFound_str;
        _status_map[SRS_CONSTS_HTTP_MethodNotAllowed               ] = SRS_CONSTS_HTTP_MethodNotAllowed_str;
        _status_map[SRS_CONSTS_HTTP_NotAcceptable                  ] = SRS_CONSTS_HTTP_NotAcceptable_str;
        _status_map[SRS_CONSTS_HTTP_ProxyAuthenticationRequired    ] = SRS_CONSTS_HTTP_ProxyAuthenticationRequired_str;
        _status_map[SRS_CONSTS_HTTP_RequestTimeout                 ] = SRS_CONSTS_HTTP_RequestTimeout_str;
        _status_map[SRS_CONSTS_HTTP_Conflict                       ] = SRS_CONSTS_HTTP_Conflict_str;
        _status_map[SRS_CONSTS_HTTP_Gone                           ] = SRS_CONSTS_HTTP_Gone_str;
        _status_map[SRS_CONSTS_HTTP_LengthRequired                 ] = SRS_CONSTS_HTTP_LengthRequired_str;
        _status_map[SRS_CONSTS_HTTP_PreconditionFailed             ] = SRS_CONSTS_HTTP_PreconditionFailed_str;
        _status_map[SRS_CONSTS_HTTP_RequestEntityTooLarge          ] = SRS_CONSTS_HTTP_RequestEntityTooLarge_str;
        _status_map[SRS_CONSTS_HTTP_RequestURITooLarge             ] = SRS_CONSTS_HTTP_RequestURITooLarge_str;
        _status_map[SRS_CONSTS_HTTP_UnsupportedMediaType           ] = SRS_CONSTS_HTTP_UnsupportedMediaType_str;
        _status_map[SRS_CONSTS_HTTP_RequestedRangeNotSatisfiable   ] = SRS_CONSTS_HTTP_RequestedRangeNotSatisfiable_str;
        _status_map[SRS_CONSTS_HTTP_ExpectationFailed              ] = SRS_CONSTS_HTTP_ExpectationFailed_str;
        _status_map[SRS_CONSTS_HTTP_InternalServerError            ] = SRS_CONSTS_HTTP_InternalServerError_str;
        _status_map[SRS_CONSTS_HTTP_NotImplemented                 ] = SRS_CONSTS_HTTP_NotImplemented_str;
        _status_map[SRS_CONSTS_HTTP_BadGateway                     ] = SRS_CONSTS_HTTP_BadGateway_str;
        _status_map[SRS_CONSTS_HTTP_ServiceUnavailable             ] = SRS_CONSTS_HTTP_ServiceUnavailable_str;
        _status_map[SRS_CONSTS_HTTP_GatewayTimeout                 ] = SRS_CONSTS_HTTP_GatewayTimeout_str;
        _status_map[SRS_CONSTS_HTTP_HTTPVersionNotSupported        ] = SRS_CONSTS_HTTP_HTTPVersionNotSupported_str;
    }
    std::string status_text;
    if (_status_map.find(status) == _status_map.end()) {
        status_text = "Status Unknown";
    } else {
        status_text = _status_map[status];
    }
    return status_text;
} // srs_generate_http_status_text

bool srs_go_http_body_allowd(int status)
{
    if (status >= 100 && status <= 199) {
        return false;
    } else if (status == 204 || status == 304) {
        return false;
    }
    return true;
}

string srs_go_http_detect(char* data, int size)
{
    if (data) {}
    return "application/octet-stream";
}

int srs_go_http_error(ISrsHttpResponseWriter* w, int code)
{
    return srs_go_http_error(w, code, srs_generate_http_status_text(code));
}

int srs_go_http_error(ISrsHttpResponseWriter* w, int code, string error)
{
    int ret = ERROR_SUCCESS;

    w->header()->set_content_type("text/plain; charset=utf-8");
    w->header()->set_content_length(error.length());
    w->write_header(code);
    w->write((char *) error.data(), (int) error.length());
    return ret;
}

SrsHttpHeader::SrsHttpHeader()
{}

SrsHttpHeader::~SrsHttpHeader()
{}

void SrsHttpHeader::set(string key, string value)
{
    headers[key] = value;
}

string SrsHttpHeader::get(string key)
{
    std::string v;

    if (headers.find(key) != headers.end()) {
        v = headers[key];
    }
    return v;
}

int64_t SrsHttpHeader::content_length()
{
    std::string cl = get("Content-Length");

    if (cl.empty()) {
        return -1;
    }
    return (int64_t) ::atof(cl.c_str());
}

void SrsHttpHeader::set_content_length(int64_t size)
{
    char buf[64];

    snprintf(buf, sizeof(buf), "%" PRId64, size);
    set("Content-Length", buf);
}

string SrsHttpHeader::content_type()
{
    return get("Content-Type");
}

void SrsHttpHeader::set_content_type(string ct)
{
    set("Content-Type", ct);
}

void SrsHttpHeader::write(stringstream& ss)
{
    std::map<std::string, std::string>::iterator it;

    for (it = headers.begin(); it != headers.end(); ++it) {
        ss << it->first << ": " << it->second << SRS_HTTP_CRLF;
    }
}

ISrsHttpResponseWriter::ISrsHttpResponseWriter()
{}

ISrsHttpResponseWriter::~ISrsHttpResponseWriter()
{}

ISrsHttpResponseReader::ISrsHttpResponseReader()
{}

ISrsHttpResponseReader::~ISrsHttpResponseReader()
{}

ISrsHttpHandler::ISrsHttpHandler()
{
    entry = NULL;
}

ISrsHttpHandler::~ISrsHttpHandler()
{}

bool ISrsHttpHandler::is_not_found()
{
    return false;
}

SrsHttpRedirectHandler::SrsHttpRedirectHandler(string u, int c)
{
    url  = u;
    code = c;
}

SrsHttpRedirectHandler::~SrsHttpRedirectHandler()
{}

int SrsHttpRedirectHandler::serve_http(ISrsHttpResponseWriter* w, ISrsHttpMessage* r)
{
    int ret         = ERROR_SUCCESS;
    string location = url;

    if (!r->query().empty()) {
        location += "?" + r->query();
    }
    string msg = "Redirect to" + location;
    w->header()->set_content_type("text/plain; charset=utf-8");
    w->header()->set_content_length(msg.length());
    w->header()->set("Location", location);
    w->write_header(code);
    w->write((char *) msg.data(), (int) msg.length());
    w->final_request();
    srs_info("redirect to %s.", location.c_str());
    return ret;
}

SrsHttpNotFoundHandler::SrsHttpNotFoundHandler()
{}

SrsHttpNotFoundHandler::~SrsHttpNotFoundHandler()
{}

bool SrsHttpNotFoundHandler::is_not_found()
{
    return true;
}

int SrsHttpNotFoundHandler::serve_http(ISrsHttpResponseWriter* w, ISrsHttpMessage* r)
{
    return srs_go_http_error(w, SRS_CONSTS_HTTP_NotFound);
}

SrsHttpFileServer::SrsHttpFileServer(string root_dir)
{
    dir = root_dir;
}

SrsHttpFileServer::~SrsHttpFileServer()
{}

int SrsHttpFileServer::serve_http(ISrsHttpResponseWriter* w, ISrsHttpMessage* r)
{
    string upath = r->path();

    if (srs_string_ends_with(upath, "/")) {
        upath += SRS_HTTP_DEFAULT_PAGE;
    }
    string fullpath = dir + "/";
    srs_assert(entry);
    size_t pos = entry->pattern.find("/");
    if (upath.length() > entry->pattern.length() && pos != string::npos) {
        fullpath += upath.substr(entry->pattern.length() - pos);
    } else {
        fullpath += upath;
    }
    if (!srs_path_exists(fullpath)) {
        srs_warn("http miss file=%s, pattern=%s, upath=%s",
                 fullpath.c_str(), entry->pattern.c_str(), upath.c_str());
        return SrsHttpNotFoundHandler().serve_http(w, r);
    }
    srs_trace("http match file=%s, pattern=%s, upath=%s",
              fullpath.c_str(), entry->pattern.c_str(), upath.c_str());
    if (srs_string_ends_with(fullpath, ".flv") || srs_string_ends_with(fullpath, ".fhv")) {
        return serve_flv_file(w, r, fullpath);
    } else if (srs_string_ends_with(fullpath, ".mp4")) {
        return serve_mp4_file(w, r, fullpath);
    }
    return serve_file(w, r, fullpath);
} // SrsHttpFileServer::serve_http

int SrsHttpFileServer::serve_file(ISrsHttpResponseWriter* w, ISrsHttpMessage* r, string fullpath)
{
    int ret = ERROR_SUCCESS;
    SrsFileReader fs;

    if ((ret = fs.open(fullpath)) != ERROR_SUCCESS) {
        srs_warn("open file %s failed, ret=%d", fullpath.c_str(), ret);
        return ret;
    }
    int64_t length = fs.filesize();
    w->header()->set_content_length(length);
    static std::map<std::string, std::string> _mime;
    if (_mime.empty()) {
        _mime[".ts"]   = "video/MP2T";
        _mime[".flv"]  = "video/x-flv";
        _mime[".m4v"]  = "video/x-m4v";
        _mime[".3gpp"] = "video/3gpp";
        _mime[".3gp"]  = "video/3gpp";
        _mime[".mp4"]  = "video/mp4";
        _mime[".aac"]  = "audio/x-aac";
        _mime[".mp3"]  = "audio/mpeg";
        _mime[".m4a"]  = "audio/x-m4a";
        _mime[".ogg"]  = "audio/ogg";
        _mime[".m3u8"] = "application/vnd.apple.mpegurl";
        _mime[".rss"]  = "application/rss+xml";
        _mime[".json"] = "application/json";
        _mime[".swf"]  = "application/x-shockwave-flash";
        _mime[".doc"]  = "application/msword";
        _mime[".zip"]  = "application/zip";
        _mime[".rar"]  = "application/x-rar-compressed";
        _mime[".xml"]  = "text/xml";
        _mime[".html"] = "text/html";
        _mime[".js"]   = "text/javascript";
        _mime[".css"]  = "text/css";
        _mime[".ico"]  = "image/x-icon";
        _mime[".png"]  = "image/png";
        _mime[".jpeg"] = "image/jpeg";
        _mime[".jpg"]  = "image/jpeg";
        _mime[".gif"]  = "image/gif";
    }
    if (true) {
        size_t pos;
        std::string ext = fullpath;
        if ((pos = ext.rfind(".")) != string::npos) {
            ext = ext.substr(pos);
        }
        if (_mime.find(ext) == _mime.end()) {
            w->header()->set_content_type("application/octet-stream");
        } else {
            w->header()->set_content_type(_mime[ext]);
        }
    }
    int64_t left = length;
    if ((ret = copy(w, &fs, r, (int) left)) != ERROR_SUCCESS) {
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("read file=%s size=%d failed, ret=%d", fullpath.c_str(), left, ret);
        }
        return ret;
    }
    return w->final_request();
} // SrsHttpFileServer::serve_file

int SrsHttpFileServer::serve_flv_file(ISrsHttpResponseWriter* w, ISrsHttpMessage* r, string fullpath)
{
    std::string start = r->query_get("start");

    if (start.empty()) {
        return serve_file(w, r, fullpath);
    }
    int offset = ::atoi(start.c_str());
    if (offset <= 0) {
        return serve_file(w, r, fullpath);
    }
    return serve_flv_stream(w, r, fullpath, offset);
}

int SrsHttpFileServer::serve_mp4_file(ISrsHttpResponseWriter* w, ISrsHttpMessage* r, string fullpath)
{
    std::string range = r->query_get("range");

    if (range.empty()) {
        range = r->query_get("bytes");
    }
    size_t pos = string::npos;
    if (range.empty() || (pos = range.find("-")) == string::npos) {
        return serve_file(w, r, fullpath);
    }
    int start = 0;
    if (pos > 0) {
        start = ::atoi(range.substr(0, pos).c_str());
    }
    int end = -1;
    if (pos < range.length() - 1) {
        end = ::atoi(range.substr(pos + 1).c_str());
    }
    if (start < 0 || (end != -1 && start > end)) {
        return serve_file(w, r, fullpath);
    }
    return serve_mp4_stream(w, r, fullpath, start, end);
}

int SrsHttpFileServer::serve_flv_stream(ISrsHttpResponseWriter* w, ISrsHttpMessage* r, string fullpath, int offset)
{
    return serve_file(w, r, fullpath);
}

int SrsHttpFileServer::serve_mp4_stream(ISrsHttpResponseWriter* w, ISrsHttpMessage* r, string fullpath, int start,
                                        int end)
{
    return serve_file(w, r, fullpath);
}

int SrsHttpFileServer::copy(ISrsHttpResponseWriter* w, SrsFileReader* fs, ISrsHttpMessage* r, int size)
{
    int ret   = ERROR_SUCCESS;
    int left  = size;
    char* buf = r->http_ts_send_buffer();

    while (left > 0) {
        ssize_t nread = -1;
        int max_read  = srs_min(left, SRS_HTTP_TS_SEND_BUFFER_SIZE);
        if ((ret = fs->read(buf, max_read, &nread)) != ERROR_SUCCESS) {
            break;
        }
        left -= nread;
        if ((ret = w->write(buf, (int) nread)) != ERROR_SUCCESS) {
            break;
        }
    }
    return ret;
}

SrsHttpMuxEntry::SrsHttpMuxEntry()
{
    enabled        = true;
    explicit_match = false;
    handler        = NULL;
}

SrsHttpMuxEntry::~SrsHttpMuxEntry()
{
    srs_freep(handler);
}

ISrsHttpMatchHijacker::ISrsHttpMatchHijacker()
{}

ISrsHttpMatchHijacker::~ISrsHttpMatchHijacker()
{}

ISrsHttpServeMux::ISrsHttpServeMux()
{}

ISrsHttpServeMux::~ISrsHttpServeMux()
{}

SrsHttpServeMux::SrsHttpServeMux()
{}

SrsHttpServeMux::~SrsHttpServeMux()
{
    std::map<std::string, SrsHttpMuxEntry *>::iterator it;

    for (it = entries.begin(); it != entries.end(); ++it) {
        SrsHttpMuxEntry* entry = it->second;
        srs_freep(entry);
    }
    entries.clear();
    vhosts.clear();
    hijackers.clear();
}

int SrsHttpServeMux::initialize()
{
    int ret = ERROR_SUCCESS;

    return ret;
}

void SrsHttpServeMux::hijack(ISrsHttpMatchHijacker* h)
{
    std::vector<ISrsHttpMatchHijacker *>::iterator it = ::find(hijackers.begin(), hijackers.end(), h);

    if (it != hijackers.end()) {
        return;
    }
    hijackers.push_back(h);
}

void SrsHttpServeMux::unhijack(ISrsHttpMatchHijacker* h)
{
    std::vector<ISrsHttpMatchHijacker *>::iterator it = ::find(hijackers.begin(), hijackers.end(), h);

    if (it == hijackers.end()) {
        return;
    }
    hijackers.erase(it);
}

int SrsHttpServeMux::handle(std::string pattern, ISrsHttpHandler* handler)
{
    int ret = ERROR_SUCCESS;

    srs_assert(handler);
    if (pattern.empty()) {
        ret = ERROR_HTTP_PATTERN_EMPTY;
        srs_error("http: empty pattern. ret=%d", ret);
        return ret;
    }
    if (entries.find(pattern) != entries.end()) {
        SrsHttpMuxEntry* exists = entries[pattern];
        if (exists->explicit_match) {
            ret = ERROR_HTTP_PATTERN_DUPLICATED;
            srs_error("http: multiple registrations for %s. ret=%d", pattern.c_str(), ret);
            return ret;
        }
    }
    std::string vhost = pattern;
    if (pattern.at(0) != '/') {
        if (pattern.find("/") != string::npos) {
            vhost = pattern.substr(0, pattern.find("/"));
        }
        vhosts[vhost] = handler;
    }
    if (true) {
        SrsHttpMuxEntry* entry = new SrsHttpMuxEntry();
        entry->explicit_match = true;
        entry->handler        = handler;
        entry->pattern        = pattern;
        entry->handler->entry = entry;
        if (entries.find(pattern) != entries.end()) {
            SrsHttpMuxEntry* exists = entries[pattern];
            srs_freep(exists);
        }
        entries[pattern] = entry;
    }
    if (pattern != "/" && !pattern.empty() && pattern.at(pattern.length() - 1) == '/') {
        std::string rpattern   = pattern.substr(0, pattern.length() - 1);
        SrsHttpMuxEntry* entry = NULL;
        if (entries.find(rpattern) != entries.end()) {
            SrsHttpMuxEntry* exists = entries[rpattern];
            if (!exists->explicit_match) {
                entry = exists;
            }
        }
        if (!entry || entry->explicit_match) {
            srs_freep(entry);
            entry = new SrsHttpMuxEntry();
            entry->explicit_match = false;
            entry->handler        = new SrsHttpRedirectHandler(pattern, SRS_CONSTS_HTTP_Found);
            entry->pattern        = pattern;
            entry->handler->entry = entry;
            entries[rpattern]     = entry;
        }
    }
    return ret;
} // SrsHttpServeMux::handle

bool SrsHttpServeMux::can_serve(ISrsHttpMessage* r)
{
    int ret = ERROR_SUCCESS;
    ISrsHttpHandler* h = NULL;

    if ((ret = find_handler(r, &h)) != ERROR_SUCCESS) {
        return false;
    }
    srs_assert(h);
    return !h->is_not_found();
}

int SrsHttpServeMux::serve_http(ISrsHttpResponseWriter* w, ISrsHttpMessage* r)
{
    int ret = ERROR_SUCCESS;
    ISrsHttpHandler* h = NULL;

    if ((ret = find_handler(r, &h)) != ERROR_SUCCESS) {
        srs_error("find handler failed. ret=%d", ret);
        return ret;
    }
    srs_assert(h);
    if ((ret = h->serve_http(w, r)) != ERROR_SUCCESS) {
        if (!srs_is_client_gracefully_close(ret)) {
            srs_error("handler serve http failed. ret=%d", ret);
        }
        return ret;
    }
    return ret;
}

int SrsHttpServeMux::find_handler(ISrsHttpMessage* r, ISrsHttpHandler** ph)
{
    int ret = ERROR_SUCCESS;

    if (r->url().find("..") != std::string::npos) {
        ret = ERROR_HTTP_URL_NOT_CLEAN;
        srs_error("htt url not canonical, url=%s. ret=%d", r->url().c_str(), ret);
        return ret;
    }
    if ((ret = match(r, ph)) != ERROR_SUCCESS) {
        srs_error("http match handler failed. ret=%d", ret);
        return ret;
    }
    if (!hijackers.empty()) {
        std::vector<ISrsHttpMatchHijacker *>::iterator it;
        for (it = hijackers.begin(); it != hijackers.end(); ++it) {
            ISrsHttpMatchHijacker* hijacker = *it;
            if ((ret = hijacker->hijack(r, ph)) != ERROR_SUCCESS) {
                srs_error("hijacker match failed. ret=%d", ret);
                return ret;
            }
        }
    }
    static ISrsHttpHandler* h404 = new SrsHttpNotFoundHandler();
    if (*ph == NULL) {
        *ph = h404;
    }
    return ret;
}

int SrsHttpServeMux::match(ISrsHttpMessage* r, ISrsHttpHandler** ph)
{
    int ret = ERROR_SUCCESS;
    std::string path = r->path();

    if (!vhosts.empty() && vhosts.find(r->host()) != vhosts.end()) {
        path = r->host() + path;
    }
    int nb_matched     = 0;
    ISrsHttpHandler* h = NULL;
    std::map<std::string, SrsHttpMuxEntry *>::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it) {
        std::string pattern    = it->first;
        SrsHttpMuxEntry* entry = it->second;
        if (!entry->enabled) {
            continue;
        }
        if (!path_match(pattern, path)) {
            continue;
        }
        if (!h || (int) pattern.length() > nb_matched) {
            nb_matched = (int) pattern.length();
            h = entry->handler;
        }
    }
    *ph = h;
    return ret;
}

bool SrsHttpServeMux::path_match(string pattern, string path)
{
    if (pattern.empty()) {
        return false;
    }
    int n = (int) pattern.length();
    if (pattern.at(n - 1) != '/') {
        return pattern == path;
    }
    if ((int) path.length() >= n) {
        if (memcmp(pattern.data(), path.data(), n) == 0) {
            return true;
        }
    }
    return false;
}

ISrsHttpMessage::ISrsHttpMessage()
{
    _http_ts_send_buffer = new char[SRS_HTTP_TS_SEND_BUFFER_SIZE];
}

ISrsHttpMessage::~ISrsHttpMessage()
{
    srs_freepa(_http_ts_send_buffer);
}

char * ISrsHttpMessage::http_ts_send_buffer()
{
    return _http_ts_send_buffer;
}

#endif // if !defined(SRS_EXPORT_LIBRTMP)
SrsKbpsSample::SrsKbpsSample()
{
    bytes = time = 0;
    kbps  = 0;
}

SrsKbpsSlice::SrsKbpsSlice()
{
    io.in      = NULL;
    io.out     = NULL;
    last_bytes = io_bytes_base = starttime = bytes = delta_bytes = 0;
}

SrsKbpsSlice::~SrsKbpsSlice()
{}

int64_t SrsKbpsSlice::get_total_bytes()
{
    return bytes + last_bytes - io_bytes_base;
}

void SrsKbpsSlice::sample()
{
    int64_t now         = srs_get_system_time_ms();
    int64_t total_bytes = get_total_bytes();

    if (sample_30s.time <= 0) {
        sample_30s.kbps  = 0;
        sample_30s.time  = now;
        sample_30s.bytes = total_bytes;
    }
    if (sample_1m.time <= 0) {
        sample_1m.kbps  = 0;
        sample_1m.time  = now;
        sample_1m.bytes = total_bytes;
    }
    if (sample_5m.time <= 0) {
        sample_5m.kbps  = 0;
        sample_5m.time  = now;
        sample_5m.bytes = total_bytes;
    }
    if (sample_60m.time <= 0) {
        sample_60m.kbps  = 0;
        sample_60m.time  = now;
        sample_60m.bytes = total_bytes;
    }
    if (now - sample_30s.time > 30 * 1000) {
        sample_30s.kbps  = (int) ((total_bytes - sample_30s.bytes) * 8 / (now - sample_30s.time));
        sample_30s.time  = now;
        sample_30s.bytes = total_bytes;
    }
    if (now - sample_1m.time > 60 * 1000) {
        sample_1m.kbps  = (int) ((total_bytes - sample_1m.bytes) * 8 / (now - sample_1m.time));
        sample_1m.time  = now;
        sample_1m.bytes = total_bytes;
    }
    if (now - sample_5m.time > 300 * 1000) {
        sample_5m.kbps  = (int) ((total_bytes - sample_5m.bytes) * 8 / (now - sample_5m.time));
        sample_5m.time  = now;
        sample_5m.bytes = total_bytes;
    }
    if (now - sample_60m.time > 3600 * 1000) {
        sample_60m.kbps  = (int) ((total_bytes - sample_60m.bytes) * 8 / (now - sample_60m.time));
        sample_60m.time  = now;
        sample_60m.bytes = total_bytes;
    }
} // SrsKbpsSlice::sample

IKbpsDelta::IKbpsDelta()
{}

IKbpsDelta::~IKbpsDelta()
{}

SrsKbps::SrsKbps()
{}

SrsKbps::~SrsKbps()
{}

void SrsKbps::set_io(ISrsProtocolStatistic* in, ISrsProtocolStatistic* out)
{
    if (is.starttime == 0) {
        is.starttime = srs_get_system_time_ms();
    }
    if (is.io.in) {
        is.bytes += is.last_bytes - is.io_bytes_base;
    }
    is.io.in      = in;
    is.last_bytes = is.io_bytes_base = 0;
    if (in) {
        is.last_bytes = is.io_bytes_base = in->get_recv_bytes();
    }
    is.sample();
    if (os.starttime == 0) {
        os.starttime = srs_get_system_time_ms();
    }
    if (os.io.out) {
        os.bytes += os.last_bytes - os.io_bytes_base;
    }
    os.io.out     = out;
    os.last_bytes = os.io_bytes_base = 0;
    if (out) {
        os.last_bytes = os.io_bytes_base = out->get_send_bytes();
    }
    os.sample();
} // SrsKbps::set_io

int SrsKbps::get_send_kbps()
{
    int64_t duration = srs_get_system_time_ms() - is.starttime;

    if (duration <= 0) {
        return 0;
    }
    int64_t bytes = get_send_bytes();
    return (int) (bytes * 8 / duration);
}

int SrsKbps::get_recv_kbps()
{
    int64_t duration = srs_get_system_time_ms() - os.starttime;

    if (duration <= 0) {
        return 0;
    }
    int64_t bytes = get_recv_bytes();
    return (int) (bytes * 8 / duration);
}

int SrsKbps::get_send_kbps_30s()
{
    return os.sample_30s.kbps;
}

int SrsKbps::get_recv_kbps_30s()
{
    return is.sample_30s.kbps;
}

int SrsKbps::get_send_kbps_5m()
{
    return os.sample_5m.kbps;
}

int SrsKbps::get_recv_kbps_5m()
{
    return is.sample_5m.kbps;
}

int64_t SrsKbps::get_send_bytes()
{
    return os.get_total_bytes();
}

int64_t SrsKbps::get_recv_bytes()
{
    return is.get_total_bytes();
}

void SrsKbps::resample()
{
    sample();
}

int64_t SrsKbps::get_send_bytes_delta()
{
    int64_t delta = os.get_total_bytes() - os.delta_bytes;

    return delta;
}

int64_t SrsKbps::get_recv_bytes_delta()
{
    int64_t delta = is.get_total_bytes() - is.delta_bytes;

    return delta;
}

void SrsKbps::cleanup()
{
    os.delta_bytes = os.get_total_bytes();
    is.delta_bytes = is.get_total_bytes();
}

void SrsKbps::add_delta(IKbpsDelta* delta)
{
    srs_assert(delta);
    is.last_bytes += delta->get_recv_bytes_delta();
    os.last_bytes += delta->get_send_bytes_delta();
}

void SrsKbps::sample()
{
    if (os.io.out) {
        os.last_bytes = os.io.out->get_send_bytes();
    }
    if (is.io.in) {
        is.last_bytes = is.io.in->get_recv_bytes();
    }
    is.sample();
    os.sample();
}

using namespace std;
#ifdef SRS_JSON_USE_NXJSON
# ifndef NXJSON_H
#  define NXJSON_H
#  ifdef  __cplusplus
extern "C" {
#  endif
typedef enum nx_json_type {
    NX_JSON_NULL,
    NX_JSON_OBJECT,
    NX_JSON_ARRAY,
    NX_JSON_STRING,
    NX_JSON_INTEGER,
    NX_JSON_DOUBLE,
    NX_JSON_BOOL
} nx_json_type;
typedef struct nx_json {
    nx_json_type    type;
    const char*     key;
    const char*     text_value;
    long            int_value;
    double          dbl_value;
    int             length;
    struct nx_json* child;
    struct nx_json* next;
    struct nx_json* last_child;
} nx_json;
typedef int (* nx_json_unicode_encoder)(unsigned int codepoint, char* p, char** endp);
extern nx_json_unicode_encoder nx_json_unicode_to_utf8;
const nx_json * nx_json_parse(char* text, nx_json_unicode_encoder encoder);
const nx_json * nx_json_parse_utf8(char* text);
void nx_json_free(const nx_json* js);
const nx_json * nx_json_get(const nx_json* json, const char* key);
const nx_json * nx_json_item(const nx_json* json, int idx);
#  ifdef  __cplusplus
}
#  endif
# endif // ifndef NXJSON_H
#endif  // ifdef SRS_JSON_USE_NXJSON
#define SRS_JSON_Boolean 0x01
#define SRS_JSON_String  0x02
#define SRS_JSON_Object  0x03
#define SRS_JSON_Integer 0x04
#define SRS_JSON_Number  0x05
#define SRS_JSON_Null    0x06
#define SRS_JSON_Array   0x07
class SrsJsonString : public SrsJsonAny
{
public:
    std::string value;
    SrsJsonString(const char* _value)
    {
        marker = SRS_JSON_String;
        if (_value) {
            value = _value;
        }
    }

    virtual ~SrsJsonString()
    {}
};
class SrsJsonBoolean : public SrsJsonAny
{
public:
    bool value;
    SrsJsonBoolean(bool _value)
    {
        marker = SRS_JSON_Boolean;
        value  = _value;
    }

    virtual ~SrsJsonBoolean()
    {}
};
class SrsJsonInteger : public SrsJsonAny
{
public:
    int64_t value;
    SrsJsonInteger(int64_t _value)
    {
        marker = SRS_JSON_Integer;
        value  = _value;
    }

    virtual ~SrsJsonInteger()
    {}
};
class SrsJsonNumber : public SrsJsonAny
{
public:
    double value;
    SrsJsonNumber(double _value)
    {
        marker = SRS_JSON_Number;
        value  = _value;
    }

    virtual ~SrsJsonNumber()
    {}
};
class SrsJsonNull : public SrsJsonAny
{
public:
    SrsJsonNull()
    {
        marker = SRS_JSON_Null;
    }

    virtual ~SrsJsonNull()
    {}
};
SrsJsonAny::SrsJsonAny()
{
    marker = 0;
}

SrsJsonAny::~SrsJsonAny()
{}

bool SrsJsonAny::is_string()
{
    return marker == SRS_JSON_String;
}

bool SrsJsonAny::is_boolean()
{
    return marker == SRS_JSON_Boolean;
}

bool SrsJsonAny::is_number()
{
    return marker == SRS_JSON_Number;
}

bool SrsJsonAny::is_integer()
{
    return marker == SRS_JSON_Integer;
}

bool SrsJsonAny::is_object()
{
    return marker == SRS_JSON_Object;
}

bool SrsJsonAny::is_array()
{
    return marker == SRS_JSON_Array;
}

bool SrsJsonAny::is_null()
{
    return marker == SRS_JSON_Null;
}

string SrsJsonAny::to_str()
{
    SrsJsonString* p = dynamic_cast<SrsJsonString *>(this);

    srs_assert(p != NULL);
    return p->value;
}

bool SrsJsonAny::to_boolean()
{
    SrsJsonBoolean* p = dynamic_cast<SrsJsonBoolean *>(this);

    srs_assert(p != NULL);
    return p->value;
}

int64_t SrsJsonAny::to_integer()
{
    SrsJsonInteger* p = dynamic_cast<SrsJsonInteger *>(this);

    srs_assert(p != NULL);
    return p->value;
}

double SrsJsonAny::to_number()
{
    SrsJsonNumber* p = dynamic_cast<SrsJsonNumber *>(this);

    srs_assert(p != NULL);
    return p->value;
}

SrsJsonObject * SrsJsonAny::to_object()
{
    SrsJsonObject* p = dynamic_cast<SrsJsonObject *>(this);

    srs_assert(p != NULL);
    return p;
}

SrsJsonArray * SrsJsonAny::to_array()
{
    SrsJsonArray* p = dynamic_cast<SrsJsonArray *>(this);

    srs_assert(p != NULL);
    return p;
}

SrsJsonAny * SrsJsonAny::str(const char* value)
{
    return new SrsJsonString(value);
}

SrsJsonAny * SrsJsonAny::boolean(bool value)
{
    return new SrsJsonBoolean(value);
}

SrsJsonAny * SrsJsonAny::ingeter(int64_t value)
{
    return new SrsJsonInteger(value);
}

SrsJsonAny * SrsJsonAny::number(double value)
{
    return new SrsJsonNumber(value);
}

SrsJsonAny * SrsJsonAny::null()
{
    return new SrsJsonNull();
}

SrsJsonObject * SrsJsonAny::object()
{
    return new SrsJsonObject();
}

SrsJsonArray * SrsJsonAny::array()
{
    return new SrsJsonArray();
}

#ifdef SRS_JSON_USE_NXJSON
SrsJsonAny * srs_json_parse_tree_nx_json(const nx_json* node)
{
    if (!node) {
        return NULL;
    }
    switch (node->type) {
        case NX_JSON_NULL:
            return SrsJsonAny::null();

        case NX_JSON_STRING:
            return SrsJsonAny::str(node->text_value);

        case NX_JSON_INTEGER:
            return SrsJsonAny::ingeter(node->int_value);

        case NX_JSON_DOUBLE:
            return SrsJsonAny::number(node->dbl_value);

        case NX_JSON_BOOL:
            return SrsJsonAny::boolean(node->int_value != 0);

        case NX_JSON_OBJECT: {
            SrsJsonObject* obj = SrsJsonAny::object();
            for (nx_json* p = node->child; p != NULL; p = p->next) {
                SrsJsonAny* value = srs_json_parse_tree_nx_json(p);
                if (value) {
                    obj->set(p->key, value);
                }
            }
            return obj;
        }
        case NX_JSON_ARRAY: {
            SrsJsonArray* arr = SrsJsonAny::array();
            for (nx_json* p = node->child; p != NULL; p = p->next) {
                SrsJsonAny* value = srs_json_parse_tree_nx_json(p);
                if (value) {
                    arr->add(value);
                }
            }
            return arr;
        }
    }
    return NULL;
} // srs_json_parse_tree_nx_json

SrsJsonAny * SrsJsonAny::loads(char* str)
{
    if (!str) {
        return NULL;
    }
    if (strlen(str) == 0) {
        return NULL;
    }
    const nx_json* o = nx_json_parse(str, 0);
    SrsJsonAny* json = srs_json_parse_tree_nx_json(o);
    if (o) {
        nx_json_free(o);
    }
    return json;
}

#endif // ifdef SRS_JSON_USE_NXJSON
SrsJsonObject::SrsJsonObject()
{
    marker = SRS_JSON_Object;
}

SrsJsonObject::~SrsJsonObject()
{
    std::vector<SrsJsonObjectPropertyType>::iterator it;

    for (it = properties.begin(); it != properties.end(); ++it) {
        SrsJsonObjectPropertyType item = *it;
        SrsJsonAny* obj = item.second;
        srs_freep(obj);
    }
    properties.clear();
}

int SrsJsonObject::count()
{
    return (int) properties.size();
}

string SrsJsonObject::key_at(int index)
{
    srs_assert(index < count());
    SrsJsonObjectPropertyType& elem = properties[index];
    return elem.first;
}

SrsJsonAny * SrsJsonObject::value_at(int index)
{
    srs_assert(index < count());
    SrsJsonObjectPropertyType& elem = properties[index];
    return elem.second;
}

void SrsJsonObject::set(string key, SrsJsonAny* value)
{
    if (!value) {
        srs_warn("add a NULL propertity %s", key.c_str());
        return;
    }
    std::vector<SrsJsonObjectPropertyType>::iterator it;
    for (it = properties.begin(); it != properties.end(); ++it) {
        SrsJsonObjectPropertyType& elem = *it;
        std::string name = elem.first;
        SrsJsonAny* any  = elem.second;
        if (key == name) {
            srs_freep(any);
            properties.erase(it);
            break;
        }
    }
    properties.push_back(std::make_pair(key, value));
}

SrsJsonAny * SrsJsonObject::get_property(string name)
{
    std::vector<SrsJsonObjectPropertyType>::iterator it;

    for (it = properties.begin(); it != properties.end(); ++it) {
        SrsJsonObjectPropertyType& elem = *it;
        std::string key = elem.first;
        SrsJsonAny* any = elem.second;
        if (key == name) {
            return any;
        }
    }
    return NULL;
}

SrsJsonAny * SrsJsonObject::ensure_property_string(string name)
{
    SrsJsonAny* prop = get_property(name);

    if (!prop) {
        return NULL;
    }
    if (!prop->is_string()) {
        return NULL;
    }
    return prop;
}

SrsJsonAny * SrsJsonObject::ensure_property_integer(string name)
{
    SrsJsonAny* prop = get_property(name);

    if (!prop) {
        return NULL;
    }
    if (!prop->is_integer()) {
        return NULL;
    }
    return prop;
}

SrsJsonAny * SrsJsonObject::ensure_property_boolean(string name)
{
    SrsJsonAny* prop = get_property(name);

    if (!prop) {
        return NULL;
    }
    if (!prop->is_boolean()) {
        return NULL;
    }
    return prop;
}

SrsJsonArray::SrsJsonArray()
{
    marker = SRS_JSON_Array;
}

SrsJsonArray::~SrsJsonArray()
{
    std::vector<SrsJsonAny *>::iterator it;

    for (it = properties.begin(); it != properties.end(); ++it) {
        SrsJsonAny* item = *it;
        srs_freep(item);
    }
    properties.clear();
}

int SrsJsonArray::count()
{
    return (int) properties.size();
}

SrsJsonAny * SrsJsonArray::at(int index)
{
    srs_assert(index < count());
    SrsJsonAny* elem = properties[index];
    return elem;
}

void SrsJsonArray::add(SrsJsonAny* value)
{
    properties.push_back(value);
}

#ifdef SRS_JSON_USE_NXJSON
# ifndef NXJSON_C
#  define NXJSON_C
#  ifdef  __cplusplus
extern "C" {
#  endif
#  include <stdlib.h>
#  include <stdio.h>
#  include <string.h>
#  include <assert.h>
#  ifndef NX_JSON_CALLOC
#   define NX_JSON_CALLOC()   calloc(1, sizeof(nx_json))
#   define NX_JSON_FREE(json) free((void *) (json))
#  endif
#  ifndef NX_JSON_REPORT_ERROR
#   define NX_JSON_REPORT_ERROR(msg, p) srs_warn("NXJSON PARSE ERROR (%d): " msg " at %s", __LINE__, p)
#  endif
#  define IS_WHITESPACE(c)              ((unsigned char) (c) <= (unsigned char) ' ')
static const nx_json dummy = { NX_JSON_NULL
};
static nx_json * create_json(nx_json_type type, const char* key, nx_json* parent)
{
    nx_json* js = (nx_json *) NX_JSON_CALLOC();

    memset(js, 0, sizeof(nx_json));
    assert(js);
    js->type = type;
    js->key  = key;
    if (!parent->last_child) {
        parent->child = parent->last_child = js;
    } else {
        parent->last_child->next = js;
        parent->last_child       = js;
    }
    parent->length++;
    return js;
}

void nx_json_free(const nx_json* js)
{
    nx_json* p = js->child;
    nx_json* p1;

    while (p) {
        p1 = p->next;
        nx_json_free(p);
        p = p1;
    }
    NX_JSON_FREE(js);
}

static int unicode_to_utf8(unsigned int codepoint, char* p, char** endp)
{
    if (codepoint < 0x80) {
        *p++ = codepoint;
    } else if (codepoint < 0x800) {
        *p++ = 192 + codepoint / 64, *p++ = 128 + codepoint % 64;
    } else if (codepoint - 0xd800u < 0x800) {
        return 0;
    } else if (codepoint < 0x10000) {
        *p++ = 224 + codepoint / 4096, *p++ = 128 + codepoint / 64 % 64, *p++ = 128 + codepoint % 64;
    } else if (codepoint < 0x110000) {
        *p++ = 240 + codepoint / 262144, *p++ = 128 + codepoint / 4096 % 64, *p++ = 128 + codepoint / 64 % 64,
        *p++ = 128 + codepoint % 64;
    } else {
        return 0;
    }
    *endp = p;
    return 1;
}

nx_json_unicode_encoder nx_json_unicode_to_utf8 = unicode_to_utf8;
static inline int hex_val(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

static char * unescape_string(char* s, char** end, nx_json_unicode_encoder encoder)
{
    char* p = s;
    char* d = s;
    char c;

    while ((c = *p++)) {
        if (c == '"') {
            *d   = '\0';
            *end = p;
            return s;
        } else if (c == '\\') {
            switch (*p) {
                case '\\':
                case '/':
                case '"':
                    *d++ = *p++;
                    break;
                case 'b':
                    *d++ = '\b';
                    p++;
                    break;
                case 'f':
                    *d++ = '\f';
                    p++;
                    break;
                case 'n':
                    *d++ = '\n';
                    p++;
                    break;
                case 'r':
                    *d++ = '\r';
                    p++;
                    break;
                case 't':
                    *d++ = '\t';
                    p++;
                    break;
                case 'u': {
                    if (!encoder) {
                        *d++ = c;
                        break;
                    }
                    char* ps = p - 1;
                    int h1, h2, h3, h4;
                    if ((h1 = hex_val(p[1])) < 0 || (h2 = hex_val(p[2])) < 0 || (h3 = hex_val(p[3])) < 0 ||
                        (h4 = hex_val(p[4])) < 0)
                    {
                        NX_JSON_REPORT_ERROR("invalid unicode escape", p - 1);
                        return 0;
                    }
                    unsigned int codepoint = h1 << 12 | h2 << 8 | h3 << 4 | h4;
                    if ((codepoint & 0xfc00) == 0xd800) {
                        p += 6;
                        if (p[-1] != '\\' || *p != 'u' || (h1 = hex_val(p[1])) < 0 || (h2 = hex_val(p[2])) < 0 ||
                            (h3 = hex_val(p[3])) < 0 || (h4 = hex_val(p[4])) < 0)
                        {
                            NX_JSON_REPORT_ERROR("invalid unicode surrogate", ps);
                            return 0;
                        }
                        unsigned int codepoint2 = h1 << 12 | h2 << 8 | h3 << 4 | h4;
                        if ((codepoint2 & 0xfc00) != 0xdc00) {
                            NX_JSON_REPORT_ERROR("invalid unicode surrogate", ps);
                            return 0;
                        }
                        codepoint = 0x10000 + ((codepoint - 0xd800) << 10) + (codepoint2 - 0xdc00);
                    }
                    if (!encoder(codepoint, d, &d)) {
                        NX_JSON_REPORT_ERROR("invalid codepoint", ps);
                        return 0;
                    }
                    p += 5;
                    break;
                }
                default: {
                    *d++ = c;
                    break;
                }
            }
        } else {
            *d++ = c;
        }
    }
    NX_JSON_REPORT_ERROR("no closing quote for string", s);
    return 0;
} // unescape_string

static char * skip_block_comment(char* p)
{
    char* ps = p - 2;

    if (!*p) {
        NX_JSON_REPORT_ERROR("endless comment", ps);
        return 0;
    }
REPEAT:
    p = strchr(p + 1, '/');
    if (!p) {
        NX_JSON_REPORT_ERROR("endless comment", ps);
        return 0;
    }
    if (p[-1] != '*') {
        goto REPEAT;
    }
    return p + 1;
}

static char * parse_key(const char** key, char* p, nx_json_unicode_encoder encoder)
{
    char c;

    while ((c = *p++)) {
        if (c == '"') {
            *key = unescape_string(p, &p, encoder);
            if (!*key) {
                return 0;
            }
            while (*p && IS_WHITESPACE(*p)) {
                p++;
            }
            if (*p == ':') {
                return p + 1;
            }
            NX_JSON_REPORT_ERROR("unexpected chars", p);
            return 0;
        } else if (IS_WHITESPACE(c) || c == ',') {} else if (c == '}') {
            return p - 1;
        } else if (c == '/') {
            if (*p == '/') {
                char* ps = p - 1;
                p = strchr(p + 1, '\n');
                if (!p) {
                    NX_JSON_REPORT_ERROR("endless comment", ps);
                    return 0;
                }
                p++;
            } else if (*p == '*') {
                p = skip_block_comment(p + 1);
                if (!p) {
                    return 0;
                }
            } else {
                NX_JSON_REPORT_ERROR("unexpected chars", p - 1);
                return 0;
            }
        } else {
            NX_JSON_REPORT_ERROR("unexpected chars", p - 1);
            return 0;
        }
    }
    NX_JSON_REPORT_ERROR("unexpected chars", p - 1);
    return 0;
} // parse_key

static char * parse_value(nx_json* parent, const char* key, char* p, nx_json_unicode_encoder encoder)
{
    nx_json* js;

    while (1) {
        switch (*p) {
            case '\0':
                NX_JSON_REPORT_ERROR("unexpected end of text", p);
                return 0;

            case ' ':
            case '\t':
            case '\n':
            case '\r':
            case ',':
                p++;
                break;
            case '{':
                js = create_json(NX_JSON_OBJECT, key, parent);
                p++;
                while (1) {
                    const char* new_key;
                    p = parse_key(&new_key, p, encoder);
                    if (!p) {
                        return 0;
                    }
                    if (*p == '}') {
                        return p + 1;
                    }
                    p = parse_value(js, new_key, p, encoder);
                    if (!p) {
                        return 0;
                    }
                }
            case '[':
                js = create_json(NX_JSON_ARRAY, key, parent);
                p++;
                while (1) {
                    p = parse_value(js, 0, p, encoder);
                    if (!p) {
                        return 0;
                    }
                    if (*p == ']') {
                        return p + 1;
                    }
                }
            case ']':
                return p;

            case '"':
                p++;
                js = create_json(NX_JSON_STRING, key, parent);
                js->text_value = unescape_string(p, &p, encoder);
                if (!js->text_value) {
                    return 0;
                }
                return p;

            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                js = create_json(NX_JSON_INTEGER, key, parent);
                char* pe;
                js->int_value = strtol(p, &pe, 0);
                if (pe == p) {
                    NX_JSON_REPORT_ERROR("invalid number", p);
                    return 0;
                }
                if (*pe == '.' || *pe == 'e' || *pe == 'E') {
                    js->type      = NX_JSON_DOUBLE;
                    js->dbl_value = strtod(p, &pe);
                    if (pe == p) {
                        NX_JSON_REPORT_ERROR("invalid number", p);
                        return 0;
                    }
                } else {
                    js->dbl_value = js->int_value;
                }
                return pe;
            }
            case 't':
                if (!strncmp(p, "true", 4)) {
                    js = create_json(NX_JSON_BOOL, key, parent);
                    js->int_value = 1;
                    return p + 4;
                }
                NX_JSON_REPORT_ERROR("unexpected chars", p);
                return 0;

            case 'f':
                if (!strncmp(p, "false", 5)) {
                    js = create_json(NX_JSON_BOOL, key, parent);
                    js->int_value = 0;
                    return p + 5;
                }
                NX_JSON_REPORT_ERROR("unexpected chars", p);
                return 0;

            case 'n':
                if (!strncmp(p, "null", 4)) {
                    create_json(NX_JSON_NULL, key, parent);
                    return p + 4;
                }
                NX_JSON_REPORT_ERROR("unexpected chars", p);
                return 0;

            case '/':
                if (p[1] == '/') {
                    char* ps = p;
                    p = strchr(p + 2, '\n');
                    if (!p) {
                        NX_JSON_REPORT_ERROR("endless comment", ps);
                        return 0;
                    }
                    p++;
                } else if (p[1] == '*') {
                    p = skip_block_comment(p + 2);
                    if (!p) {
                        return 0;
                    }
                } else {
                    NX_JSON_REPORT_ERROR("unexpected chars", p);
                    return 0;
                }
                break;
            default:
                NX_JSON_REPORT_ERROR("unexpected chars", p);
                return 0;
        }
    }
} // parse_value

const nx_json * nx_json_parse_utf8(char* text)
{
    return nx_json_parse(text, unicode_to_utf8);
}

const nx_json * nx_json_parse(char* text, nx_json_unicode_encoder encoder)
{
    nx_json js;

    memset(&js, 0, sizeof(nx_json));
    if (!parse_value(&js, 0, text, encoder)) {
        if (js.child) {
            nx_json_free(js.child);
        }
        return 0;
    }
    return js.child;
}

const nx_json * nx_json_get(const nx_json* json, const char* key)
{
    if (!json || !key) {
        return &dummy;
    }
    nx_json* js;
    for (js = json->child; js; js = js->next) {
        if (js->key && !strcmp(js->key, key)) {
            return js;
        }
    }
    return &dummy;
}

const nx_json * nx_json_item(const nx_json* json, int idx)
{
    if (!json) {
        return &dummy;
    }
    nx_json* js;
    for (js = json->child; js; js = js->next) {
        if (!idx--) {
            return js;
        }
    }
    return &dummy;
}

#  ifdef  __cplusplus
}
#  endif
# endif // ifndef NXJSON_C
#endif  // ifdef SRS_JSON_USE_NXJSON
#include <stdlib.h>
#ifndef _WIN32
# include <sys/time.h>
#endif
#include <string>
#include <sstream>
using namespace std;
ISrsLog* _srs_log = new ISrsLog();
ISrsThreadContext* _srs_context = new ISrsThreadContext();
#define SRS_SOCKET_DEFAULT_TIMEOUT 30 * 1000 * 1000LL
struct Context {
    std::string                     url;
    std::string                     tcUrl;
    std::string                     host;
    std::string                     ip;
    std::string                     port;
    std::string                     vhost;
    std::string                     app;
    std::string                     stream;
    std::string                     param;
    SrsRequest*                     req;
    std::vector<SrsCommonMessage *> msgs;
    SrsRtmpClient*                  rtmp;
    SimpleSocketStream*             skt;
    int                             stream_id;
    SrsRawH264Stream                avc_raw;
    SrsRawAacStream                 aac_raw;
    SrsStream                       h264_raw_stream;
    std::string                     h264_sps;
    std::string                     h264_pps;
    bool                            h264_sps_pps_sent;
    bool                            h264_sps_changed;
    bool                            h264_pps_changed;
    SrsStream                       aac_raw_stream;
    std::string                     aac_specific_config;
    int64_t                         stimeout;
    int64_t                         rtimeout;
    Context()
    {
        rtmp              = NULL;
        skt               = NULL;
        req               = NULL;
        stream_id         = 0;
        h264_sps_pps_sent = false;
        h264_sps_changed  = false;
        h264_pps_changed  = false;
        rtimeout          = stimeout = -1;
    }

    virtual ~Context()
    {
        srs_freep(req);
        srs_freep(rtmp);
        srs_freep(skt);
        std::vector<SrsCommonMessage *>::iterator it;
        for (it = msgs.begin(); it != msgs.end(); ++it) {
            SrsCommonMessage* msg = *it;
            srs_freep(msg);
        }
        msgs.clear();
    }
};
#ifdef _WIN32
int gettimeofday(struct timeval* tv, struct timezone* tz)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME win_time;

    GetLocalTime(&win_time);
    tm.tm_year  = win_time.wYear - 1900;
    tm.tm_mon   = win_time.wMonth - 1;
    tm.tm_mday  = win_time.wDay;
    tm.tm_hour  = win_time.wHour;
    tm.tm_min   = win_time.wMinute;
    tm.tm_sec   = win_time.wSecond;
    tm.tm_isdst = -1;
    clock       = mktime(&tm);
    tv->tv_sec  = (long) clock;
    tv->tv_usec = win_time.wMilliseconds * 1000;
    return 0;
}

int socket_setup()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        return -1;
    }
    return 0;
}

int socket_cleanup()
{
    WSACleanup();
    return 0;
}

pid_t getpid(void)
{
    return (pid_t) GetCurrentProcessId();
}

int usleep(useconds_t usec)
{
    Sleep((DWORD) (usec / 1000));
    return 0;
}

ssize_t writev(int fd, const struct iovec* iov, int iovcnt)
{
    ssize_t nwrite = 0;

    for (int i = 0; i < iovcnt; i++) {
        const struct iovec* current = iov + i;
        int nsent = ::send(fd, (char *) current->iov_base, current->iov_len, 0);
        if (nsent < 0) {
            return nsent;
        }
        nwrite += nsent;
        if (nsent == 0) {
            return nwrite;
        }
    }
    return nwrite;
}

std::size_t strlcpy(char * __restrict dst, const char * __restrict src, size_t siz)
{
    char* d       = dst;
    const char* s = src;
    size_t n      = siz;

    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0') {
                break;
            }
        }
    }
    if (n == 0) {
        if (siz != 0) {
            *d = '\0';
        }
        while (*s++)
            ;
    }
    return (s - src - 1);
}

# pragma comment(lib, "Ws2_32.lib")
static char * inet_ntop4(const u_char* src, char* dst, socklen_t size);
static char * inet_ntop6(const u_char* src, char* dst, socklen_t size);
const char * inet_ntop_win32(int af, const void* src, char* dst, socklen_t size)
{
    switch (af) {
        case AF_INET:
            return (inet_ntop4((unsigned char *) src, (char *) dst, size));

            # ifdef AF_INET6
            #  error "IPv6 not supported"
            # endif
        default:
            return 0;
    }
}

static char * inet_ntop4(const u_char* src, char* dst, socklen_t size)
{
    static const char fmt[128] = "%u.%u.%u.%u";
    char tmp[sizeof "255.255.255.255"];
    int l;

    l = snprintf(tmp, sizeof(tmp), fmt, src[0], src[1], src[2], src[3]);
    if (l <= 0 || (socklen_t) l >= size) {
        return (NULL);
    }
    strlcpy(dst, tmp, size);
    return (dst);
}

static char * inet_ntop6(const u_char* src, char* dst, socklen_t size)
{
    char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], * tp;

    struct {
        int base, len;
    } best, cur;
    # define NS_IN6ADDRSZ 16
    # define NS_INT16SZ   2
    u_int words[NS_IN6ADDRSZ / NS_INT16SZ];
    int i;
    memset(words, '\0', sizeof words);
    for (i = 0; i < NS_IN6ADDRSZ; i++) {
        words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
    }
    best.base = -1;
    best.len  = 0;
    cur.base  = -1;
    cur.len   = 0;
    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
        if (words[i] == 0) {
            if (cur.base == -1) {
                cur.base = i, cur.len = 1;
            } else {
                cur.len++;
            }
        } else {
            if (cur.base != -1) {
                if (best.base == -1 || cur.len > best.len) {
                    best = cur;
                }
                cur.base = -1;
            }
        }
    }
    if (cur.base != -1) {
        if (best.base == -1 || cur.len > best.len) {
            best = cur;
        }
    }
    if (best.base != -1 && best.len < 2) {
        best.base = -1;
    }
    tp = tmp;
    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
        if (best.base != -1 && i >= best.base &&
            i < (best.base + best.len))
        {
            if (i == best.base) {
                *tp++ = ':';
            }
            continue;
        }
        if (i != 0) {
            *tp++ = ':';
        }
        if (i == 6 && best.base == 0 && (best.len == 6 ||
                                         (best.len == 7 && words[7] != 0x0001) ||
                                         (best.len == 5 && words[5] == 0xffff)))
        {
            if (!inet_ntop4(src + 12, tp, sizeof tmp - (tp - tmp))) {
                return (NULL);
            }
            tp += strlen(tp);
            break;
        }
        tp += std::sprintf(tp, "%x", words[i]);
    }
    if (best.base != -1 && (best.base + best.len) ==
        (NS_IN6ADDRSZ / NS_INT16SZ))
    {
        *tp++ = ':';
    }
    *tp++ = '\0';
    if ((socklen_t) (tp - tmp) > size) {
        return (NULL);
    }
    strcpy(dst, tmp);
    return (dst);
} // inet_ntop6

#endif // ifdef _WIN32
int srs_librtmp_context_parse_uri(Context* context)
{
    int ret    = ERROR_SUCCESS;
    size_t pos = string::npos;
    string uri = context->url;

    if ((pos = uri.rfind("/")) != string::npos) {
        context->stream = uri.substr(pos + 1);
        context->tcUrl  = uri = uri.substr(0, pos);
    }
    std::string schema;
    srs_discovery_tc_url(context->tcUrl,
                         schema, context->host, context->vhost, context->app, context->port,
                         context->param);
    return ret;
}

int srs_librtmp_context_resolve_host(Context* context)
{
    int ret = ERROR_SUCCESS;

    context->ip = srs_dns_resolve(context->host);
    if (context->ip.empty()) {
        return -1;
    }
    return ret;
}

int srs_librtmp_context_connect(Context* context)
{
    int ret = ERROR_SUCCESS;

    srs_assert(context->skt);
    std::string ip = context->ip;
    int port       = ::atoi(context->port.c_str());
    if ((ret = context->skt->connect(ip.c_str(), port)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_librtmp_context_disconnect(Context* context)
{
    int ret = ERROR_SUCCESS;

    srs_assert(context->skt);
    if ((ret = context->skt->disconnect()) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

#ifdef __cplusplus
extern "C" {
#endif
int srs_version_major()
{
    return VERSION_MAJOR;
}

int srs_version_minor()
{
    return VERSION_MINOR;
}

int srs_version_revision()
{
    return VERSION_REVISION;
}

srs_rtmp_t srs_rtmp_create(const char* url)
{
    Context* context = new Context();

    context->url = url;
    srs_freep(context->skt);
    context->skt = new SimpleSocketStreamImpl();
    if (context->skt->create_socket() != ERROR_SUCCESS) {
        srs_freep(context);
        return NULL;
    }
    return context;
}

srs_rtmp_t srs_rtmp_create2(const char* url)
{
    Context* context = new Context();

    context->url  = url;
    context->url += "/livestream";
    srs_freep(context->skt);
    context->skt = new SimpleSocketStreamImpl();
    if (context->skt->create_socket() != ERROR_SUCCESS) {
        srs_freep(context);
        return NULL;
    }
    return context;
}

int srs_rtmp_set_timeout(srs_rtmp_t rtmp, int recv_timeout_ms, int send_timeout_ms)
{
    int ret = ERROR_SUCCESS;

    if (!rtmp) {
        return ret;
    }
    Context* context = (Context *) rtmp;
    context->stimeout = send_timeout_ms * 1000;
    context->rtimeout = recv_timeout_ms * 1000;
    context->skt->set_recv_timeout(context->rtimeout);
    context->skt->set_send_timeout(context->stimeout);
    return ret;
}

void srs_rtmp_destroy(srs_rtmp_t rtmp)
{
    if (!rtmp) {
        return;
    }
    Context* context = (Context *) rtmp;
    srs_freep(context);
}

int srs_rtmp_handshake(srs_rtmp_t rtmp)
{
    int ret = ERROR_SUCCESS;

    if ((ret = srs_rtmp_dns_resolve(rtmp)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = srs_rtmp_connect_server(rtmp)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = srs_rtmp_do_simple_handshake(rtmp)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_rtmp_dns_resolve(srs_rtmp_t rtmp)
{
    int ret = ERROR_SUCCESS;

    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    if ((ret = srs_librtmp_context_parse_uri(context)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = srs_librtmp_context_resolve_host(context)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_rtmp_connect_server(srs_rtmp_t rtmp)
{
    int ret = ERROR_SUCCESS;

    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    if (context->stimeout == -1) {
        context->stimeout = SRS_SOCKET_DEFAULT_TIMEOUT;
        context->skt->set_send_timeout(context->stimeout);
    }
    if (context->rtimeout == -1) {
        context->rtimeout = SRS_SOCKET_DEFAULT_TIMEOUT;
        context->skt->set_recv_timeout(context->rtimeout);
    }
    if ((ret = srs_librtmp_context_connect(context)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_rtmp_disconnect_server(srs_rtmp_t rtmp)
{
    int ret = ERROR_SUCCESS;

    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    if ((ret = srs_librtmp_context_disconnect(context)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_rtmp_do_complex_handshake(srs_rtmp_t rtmp)
{
    #ifndef SRS_AUTO_SSL
    return ERROR_RTMP_HS_SSL_REQUIRE;

    #endif
    int ret = ERROR_SUCCESS;
    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    srs_assert(context->skt != NULL);
    srs_freep(context->rtmp);
    context->rtmp = new SrsRtmpClient(context->skt);
    if ((ret = context->rtmp->complex_handshake()) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_rtmp_set_connect_args(srs_rtmp_t rtmp,
                              const char* tcUrl, const char* swfUrl, const char* pageUrl, srs_amf0_t args
)
{
    int ret = ERROR_SUCCESS;

    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    srs_freep(context->req);
    context->req = new SrsRequest();
    if (args) {
        context->req->args = (SrsAmf0Object *) args;
    }
    if (tcUrl) {
        context->req->tcUrl = tcUrl;
    }
    if (swfUrl) {
        context->req->swfUrl = swfUrl;
    }
    if (pageUrl) {
        context->req->pageUrl = pageUrl;
    }
    return ret;
}

int srs_rtmp_do_simple_handshake(srs_rtmp_t rtmp)
{
    int ret = ERROR_SUCCESS;

    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    srs_assert(context->skt != NULL);
    srs_freep(context->rtmp);
    context->rtmp = new SrsRtmpClient(context->skt);
    if ((ret = context->rtmp->simple_handshake()) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_rtmp_connect_app(srs_rtmp_t rtmp)
{
    int ret = ERROR_SUCCESS;

    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    string tcUrl     = srs_generate_tc_url(
        context->ip, context->vhost, context->app, context->port,
        context->param
    );
    if ((ret = context->rtmp->connect_app(
             context->app, tcUrl, context->req, true)) != ERROR_SUCCESS)
    {
        return ret;
    }
    return ret;
}

int srs_rtmp_connect_app2(srs_rtmp_t rtmp,
                          char srs_server_ip[128], char srs_server[128],
                          char srs_primary[128], char srs_authors[128],
                          char srs_version[32], int* srs_id, int* srs_pid
)
{
    srs_server_ip[0] = 0;
    srs_server[0]    = 0;
    srs_primary[0]   = 0;
    srs_authors[0]   = 0;
    srs_version[0]   = 0;
    *srs_id  = 0;
    *srs_pid = 0;
    int ret = ERROR_SUCCESS;
    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    string tcUrl     = srs_generate_tc_url(
        context->ip, context->vhost, context->app, context->port,
        context->param
    );
    std::string sip, sserver, sprimary, sauthors, sversion;
    if ((ret = context->rtmp->connect_app2(context->app, tcUrl, NULL, true,
                                           sip, sserver, sprimary, sauthors, sversion, *srs_id,
                                           *srs_pid)) != ERROR_SUCCESS)
    {
        return ret;
    }
    snprintf(srs_server_ip, 128, "%s", sip.c_str());
    snprintf(srs_server, 128, "%s", sserver.c_str());
    snprintf(srs_primary, 128, "%s", sprimary.c_str());
    snprintf(srs_authors, 128, "%s", sauthors.c_str());
    snprintf(srs_version, 32, "%s", sversion.c_str());
    return ret;
}

int srs_rtmp_play_stream(srs_rtmp_t rtmp)
{
    int ret = ERROR_SUCCESS;

    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    if ((ret = context->rtmp->create_stream(context->stream_id)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = context->rtmp->play(context->stream, context->stream_id)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_rtmp_publish_stream(srs_rtmp_t rtmp)
{
    int ret = ERROR_SUCCESS;

    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    if ((ret = context->rtmp->fmle_publish(context->stream, context->stream_id)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_rtmp_bandwidth_check(srs_rtmp_t rtmp,
                             int64_t* start_time, int64_t* end_time,
                             int* play_kbps, int* publish_kbps,
                             int* play_bytes, int* publish_bytes,
                             int* play_duration, int* publish_duration
)
{
    *start_time       = 0;
    *end_time         = 0;
    *play_kbps        = 0;
    *publish_kbps     = 0;
    *play_bytes       = 0;
    *publish_bytes    = 0;
    *play_duration    = 0;
    *publish_duration = 0;
    int ret = ERROR_SUCCESS;
    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    SrsBandwidthClient client;
    if ((ret = client.initialize(context->rtmp)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = client.bandwidth_check(
             start_time, end_time, play_kbps, publish_kbps,
             play_bytes, publish_bytes, play_duration, publish_duration)) != ERROR_SUCCESS
    )
    {
        return ret;
    }
    return ret;
}

int srs_rtmp_on_aggregate(Context* context, SrsCommonMessage* msg)
{
    int ret = ERROR_SUCCESS;
    SrsStream aggregate_stream;
    SrsStream* stream = &aggregate_stream;

    if ((ret = stream->initialize(msg->payload, msg->size)) != ERROR_SUCCESS) {
        return ret;
    }
    int delta = -1;
    while (!stream->empty()) {
        if (!stream->require(1)) {
            ret = ERROR_RTMP_AGGREGATE;
            srs_error("invalid aggregate message type. ret=%d", ret);
            return ret;
        }
        int8_t type = stream->read_1bytes();
        if (!stream->require(3)) {
            ret = ERROR_RTMP_AGGREGATE;
            srs_error("invalid aggregate message size. ret=%d", ret);
            return ret;
        }
        int32_t data_size = stream->read_3bytes();
        if (data_size < 0) {
            ret = ERROR_RTMP_AGGREGATE;
            srs_error("invalid aggregate message size(negative). ret=%d", ret);
            return ret;
        }
        if (!stream->require(3)) {
            ret = ERROR_RTMP_AGGREGATE;
            srs_error("invalid aggregate message time. ret=%d", ret);
            return ret;
        }
        int32_t timestamp = stream->read_3bytes();
        if (!stream->require(1)) {
            ret = ERROR_RTMP_AGGREGATE;
            srs_error("invalid aggregate message time(high). ret=%d", ret);
            return ret;
        }
        int32_t time_h = stream->read_1bytes();
        timestamp |= time_h << 24;
        timestamp &= 0x7FFFFFFF;
        if (delta < 0) {
            delta = (int) msg->header.timestamp - (int) timestamp;
        }
        timestamp += delta;
        if (!stream->require(3)) {
            ret = ERROR_RTMP_AGGREGATE;
            srs_error("invalid aggregate message stream_id. ret=%d", ret);
            return ret;
        }
        int32_t stream_id = stream->read_3bytes();
        if (data_size > 0 && !stream->require(data_size)) {
            ret = ERROR_RTMP_AGGREGATE;
            srs_error("invalid aggregate message data. ret=%d", ret);
            return ret;
        }
        SrsCommonMessage o;
        o.header.message_type    = type;
        o.header.payload_length  = data_size;
        o.header.timestamp_delta = timestamp;
        o.header.timestamp       = timestamp;
        o.header.stream_id       = stream_id;
        o.header.perfer_cid      = msg->header.perfer_cid;
        if (data_size > 0) {
            o.size    = data_size;
            o.payload = new char[o.size];
            stream->read_bytes(o.payload, o.size);
        }
        if (!stream->require(4)) {
            ret = ERROR_RTMP_AGGREGATE;
            srs_error("invalid aggregate message previous tag size. ret=%d", ret);
            return ret;
        }
        stream->read_4bytes();
        SrsCommonMessage* parsed_msg = new SrsCommonMessage();
        parsed_msg->header  = o.header;
        parsed_msg->payload = o.payload;
        parsed_msg->size    = o.size;
        o.payload = NULL;
        context->msgs.push_back(parsed_msg);
    }
    return ret;
} // srs_rtmp_on_aggregate

int srs_rtmp_go_packet(Context* context, SrsCommonMessage* msg,
                       char* type, u_int32_t* timestamp, char** data, int* size,
                       bool* got_msg
)
{
    int ret = ERROR_SUCCESS;

    *got_msg = true;
    if (msg->header.is_audio()) {
        *type        = SRS_RTMP_TYPE_AUDIO;
        *timestamp   = (u_int32_t) msg->header.timestamp;
        *data        = (char *) msg->payload;
        *size        = (int) msg->size;
        msg->payload = NULL;
    } else if (msg->header.is_video()) {
        *type        = SRS_RTMP_TYPE_VIDEO;
        *timestamp   = (u_int32_t) msg->header.timestamp;
        *data        = (char *) msg->payload;
        *size        = (int) msg->size;
        msg->payload = NULL;
    } else if (msg->header.is_amf0_data() || msg->header.is_amf3_data()) {
        *type        = SRS_RTMP_TYPE_SCRIPT;
        *data        = (char *) msg->payload;
        *size        = (int) msg->size;
        msg->payload = NULL;
    } else if (msg->header.is_aggregate()) {
        if ((ret = srs_rtmp_on_aggregate(context, msg)) != ERROR_SUCCESS) {
            return ret;
        }
        *got_msg = false;
    } else {
        *type        = msg->header.message_type;
        *data        = (char *) msg->payload;
        *size        = (int) msg->size;
        msg->payload = NULL;
    }
    return ret;
} // srs_rtmp_go_packet

int srs_rtmp_read_packet(srs_rtmp_t rtmp, char* type, u_int32_t* timestamp, char** data, int* size)
{
    *type      = 0;
    *timestamp = 0;
    *data      = NULL;
    *size      = 0;
    int ret = ERROR_SUCCESS;
    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    for (;;) {
        SrsCommonMessage* msg = NULL;
        if (!context->msgs.empty()) {
            std::vector<SrsCommonMessage *>::iterator it = context->msgs.begin();
            msg = *it;
            context->msgs.erase(it);
        }
        if (!msg && (ret = context->rtmp->recv_message(&msg)) != ERROR_SUCCESS) {
            return ret;
        }
        if (!msg) {
            continue;
        }
        SrsAutoFree(SrsCommonMessage, msg);
        bool got_msg;
        if ((ret = srs_rtmp_go_packet(context, msg, type, timestamp, data, size, &got_msg)) != ERROR_SUCCESS) {
            return ret;
        }
        if (got_msg) {
            break;
        }
    }
    return ret;
} // srs_rtmp_read_packet

int srs_rtmp_write_packet(srs_rtmp_t rtmp, char type, u_int32_t timestamp, char* data, int size)
{
    int ret = ERROR_SUCCESS;

    srs_assert(rtmp != NULL);
    Context* context         = (Context *) rtmp;
    SrsSharedPtrMessage* msg = NULL;
    if ((ret = srs_rtmp_create_msg(type, timestamp, data, size, context->stream_id, &msg)) != ERROR_SUCCESS) {
        return ret;
    }
    srs_assert(msg);
    if ((ret = context->rtmp->send_and_free_message(msg, context->stream_id)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

srs_bool srs_rtmp_is_onMetaData(char type, char* data, int size)
{
    int ret = ERROR_SUCCESS;

    if (type != SRS_RTMP_TYPE_SCRIPT) {
        return false;
    }
    SrsStream stream;
    if ((ret = stream.initialize(data, size)) != ERROR_SUCCESS) {
        return false;
    }
    std::string name;
    if ((ret = srs_amf0_read_string(&stream, name)) != ERROR_SUCCESS) {
        return false;
    }
    if (name == SRS_CONSTS_RTMP_ON_METADATA) {
        return true;
    }
    if (name == SRS_CONSTS_RTMP_SET_DATAFRAME) {
        return true;
    }
    return false;
}

int srs_write_audio_raw_frame(Context* context,
                              char* frame, int frame_size, SrsRawAacStreamCodec* codec, u_int32_t timestamp
)
{
    int ret    = ERROR_SUCCESS;
    char* data = NULL;
    int size   = 0;

    if ((ret = context->aac_raw.mux_aac2flv(frame, frame_size, codec, timestamp, &data, &size)) != ERROR_SUCCESS) {
        return ret;
    }
    return srs_rtmp_write_packet(context, SRS_RTMP_TYPE_AUDIO, timestamp, data, size);
}

int srs_write_aac_adts_frame(Context* context,
                             SrsRawAacStreamCodec* codec, char* frame, int frame_size, u_int32_t timestamp
)
{
    int ret = ERROR_SUCCESS;

    if (context->aac_specific_config.empty()) {
        std::string sh;
        if ((ret = context->aac_raw.mux_sequence_header(codec, sh)) != ERROR_SUCCESS) {
            return ret;
        }
        context->aac_specific_config = sh;
        codec->aac_packet_type       = 0;
        if ((ret =
                 srs_write_audio_raw_frame(context, (char *) sh.data(), (int) sh.length(), codec,
                                           timestamp)) != ERROR_SUCCESS)
        {
            return ret;
        }
    }
    codec->aac_packet_type = 1;
    return srs_write_audio_raw_frame(context, frame, frame_size, codec, timestamp);
}

int srs_write_aac_adts_frames(Context* context,
                              char sound_format, char sound_rate, char sound_size, char sound_type,
                              char* frames, int frames_size, u_int32_t timestamp
)
{
    int ret = ERROR_SUCCESS;
    SrsStream* stream = &context->aac_raw_stream;

    if ((ret = stream->initialize(frames, frames_size)) != ERROR_SUCCESS) {
        return ret;
    }
    while (!stream->empty()) {
        char* frame    = NULL;
        int frame_size = 0;
        SrsRawAacStreamCodec codec;
        if ((ret = context->aac_raw.adts_demux(stream, &frame, &frame_size, codec)) != ERROR_SUCCESS) {
            return ret;
        }
        codec.sound_format = sound_format;
        codec.sound_rate   = sound_rate;
        codec.sound_size   = sound_size;
        codec.sound_type   = sound_type;
        if ((ret = srs_write_aac_adts_frame(context, &codec, frame, frame_size, timestamp)) != ERROR_SUCCESS) {
            return ret;
        }
    }
    return ret;
}

int srs_audio_write_raw_frame(srs_rtmp_t rtmp,
                              char sound_format, char sound_rate, char sound_size, char sound_type,
                              char* frame, int frame_size, u_int32_t timestamp
)
{
    int ret = ERROR_SUCCESS;
    Context* context = (Context *) rtmp;

    srs_assert(context);
    if (sound_format == SrsCodecAudioAAC) {
        if (!srs_aac_is_adts(frame, frame_size)) {
            return ERROR_AAC_REQUIRED_ADTS;
        }
        return srs_write_aac_adts_frames(context,
                                         sound_format, sound_rate, sound_size, sound_type,
                                         frame, frame_size, timestamp);
    } else {
        SrsRawAacStreamCodec codec;
        codec.sound_format    = sound_format;
        codec.sound_rate      = sound_rate;
        codec.sound_size      = sound_size;
        codec.sound_type      = sound_type;
        codec.aac_packet_type = 0;
        return srs_write_audio_raw_frame(context, frame, frame_size, &codec, timestamp);
    }
    return ret;
}

srs_bool srs_aac_is_adts(char* aac_raw_data, int ac_raw_size)
{
    SrsStream stream;

    if (stream.initialize(aac_raw_data, ac_raw_size) != ERROR_SUCCESS) {
        return false;
    }
    return srs_aac_startswith_adts(&stream);
}

int srs_aac_adts_frame_size(char* aac_raw_data, int ac_raw_size)
{
    int size = -1;

    if (!srs_aac_is_adts(aac_raw_data, ac_raw_size)) {
        return size;
    }
    if (ac_raw_size <= 7) {
        return size;
    }
    int16_t ch3 = aac_raw_data[3];
    int16_t ch4 = aac_raw_data[4];
    int16_t ch5 = aac_raw_data[5];
    size = ((ch3 << 11) & 0x1800) | ((ch4 << 3) & 0x07f8) | ((ch5 >> 5) & 0x0007);
    return size;
}

int srs_write_h264_ipb_frame(Context* context,
                             char* frame, int frame_size, u_int32_t dts, u_int32_t pts
)
{
    int ret = ERROR_SUCCESS;

    if (!context->h264_sps_pps_sent) {
        return ERROR_H264_DROP_BEFORE_SPS_PPS;
    }
    SrsAvcNaluType nal_unit_type     = (SrsAvcNaluType) (frame[0] & 0x1f);
    SrsCodecVideoAVCFrame frame_type = SrsCodecVideoAVCFrameInterFrame;
    if (nal_unit_type == SrsAvcNaluTypeIDR) {
        frame_type = SrsCodecVideoAVCFrameKeyFrame;
    }
    std::string ibp;
    if ((ret = context->avc_raw.mux_ipb_frame(frame, frame_size, ibp)) != ERROR_SUCCESS) {
        return ret;
    }
    int8_t avc_packet_type = SrsCodecVideoAVCTypeNALU;
    char* flv  = NULL;
    int nb_flv = 0;
    if ((ret =
             context->avc_raw.mux_avc2flv(ibp, frame_type, avc_packet_type, dts, pts, &flv, &nb_flv)) != ERROR_SUCCESS)
    {
        return ret;
    }
    u_int32_t timestamp = dts;
    return srs_rtmp_write_packet(context, SRS_RTMP_TYPE_VIDEO, timestamp, flv, nb_flv);
}

int srs_write_h264_sps_pps(Context* context, u_int32_t dts, u_int32_t pts)
{
    int ret = ERROR_SUCCESS;

    if (!context->h264_sps_changed && !context->h264_pps_changed) {
        return ret;
    }
    std::string sh;
    if ((ret =
             context->avc_raw.mux_sequence_header(context->h264_sps, context->h264_pps, dts, pts, sh)) != ERROR_SUCCESS)
    {
        return ret;
    }
    int8_t frame_type      = SrsCodecVideoAVCFrameKeyFrame;
    int8_t avc_packet_type = SrsCodecVideoAVCTypeSequenceHeader;
    char* flv  = NULL;
    int nb_flv = 0;
    if ((ret =
             context->avc_raw.mux_avc2flv(sh, frame_type, avc_packet_type, dts, pts, &flv, &nb_flv)) != ERROR_SUCCESS)
    {
        return ret;
    }
    context->h264_sps_changed  = false;
    context->h264_pps_changed  = false;
    context->h264_sps_pps_sent = true;
    u_int32_t timestamp = dts;
    return srs_rtmp_write_packet(context, SRS_RTMP_TYPE_VIDEO, timestamp, flv, nb_flv);
}

int srs_write_h264_raw_frame(Context* context,
                             char* frame, int frame_size, u_int32_t dts, u_int32_t pts
)
{
    int ret = ERROR_SUCCESS;

    if (frame_size <= 0) {
        return ret;
    }
    if (context->avc_raw.is_sps(frame, frame_size)) {
        std::string sps;
        if ((ret = context->avc_raw.sps_demux(frame, frame_size, sps)) != ERROR_SUCCESS) {
            return ret;
        }
        if (context->h264_sps == sps) {
            return ERROR_H264_DUPLICATED_SPS;
        }
        context->h264_sps_changed = true;
        context->h264_sps         = sps;
        return ret;
    }
    if (context->avc_raw.is_pps(frame, frame_size)) {
        std::string pps;
        if ((ret = context->avc_raw.pps_demux(frame, frame_size, pps)) != ERROR_SUCCESS) {
            return ret;
        }
        if (context->h264_pps == pps) {
            return ERROR_H264_DUPLICATED_PPS;
        }
        context->h264_pps_changed = true;
        context->h264_pps         = pps;
        return ret;
    }
    SrsAvcNaluType nut = (SrsAvcNaluType) (frame[0] & 0x1f);
    if (nut != SrsAvcNaluTypeSPS && nut != SrsAvcNaluTypePPS &&
        nut != SrsAvcNaluTypeIDR && nut != SrsAvcNaluTypeNonIDR &&
        nut != SrsAvcNaluTypeAccessUnitDelimiter
    )
    {
        return ret;
    }
    if ((ret = srs_write_h264_sps_pps(context, dts, pts)) != ERROR_SUCCESS) {
        return ret;
    }
    return srs_write_h264_ipb_frame(context, frame, frame_size, dts, pts);
} // srs_write_h264_raw_frame

int srs_h264_write_raw_frames(srs_rtmp_t rtmp,
                              char* frames, int frames_size, u_int32_t dts, u_int32_t pts
)
{
    int ret = ERROR_SUCCESS;

    srs_assert(frames != NULL);
    srs_assert(frames_size > 0);
    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    if ((ret = context->h264_raw_stream.initialize(frames, frames_size)) != ERROR_SUCCESS) {
        return ret;
    }
    int error_code_return = ret;
    while (!context->h264_raw_stream.empty()) {
        char* frame    = NULL;
        int frame_size = 0;
        if ((ret =
                 context->avc_raw.annexb_demux_pri(&context->h264_raw_stream, &frame, &frame_size)) != ERROR_SUCCESS)
        {
            return ret;
        }
        if (frame_size <= 0) {
            continue;
        }
        if ((ret = srs_write_h264_raw_frame(context, frame, frame_size, dts, pts)) != ERROR_SUCCESS) {
            error_code_return = ret;
            if (srs_h264_is_dvbsp_error(ret) ||
                srs_h264_is_duplicated_sps_error(ret) ||
                srs_h264_is_duplicated_pps_error(ret)
            )
            {
                continue;
            }
            return ret;
        }
    }
    return error_code_return;
} // srs_h264_write_raw_frames

srs_bool srs_h264_is_dvbsp_error(int error_code)
{
    return error_code == ERROR_H264_DROP_BEFORE_SPS_PPS;
}

srs_bool srs_h264_is_duplicated_sps_error(int error_code)
{
    return error_code == ERROR_H264_DUPLICATED_SPS;
}

srs_bool srs_h264_is_duplicated_pps_error(int error_code)
{
    return error_code == ERROR_H264_DUPLICATED_PPS;
}

srs_bool srs_h264_startswith_annexb(char* h264_raw_data, int h264_raw_size, int* pnb_start_code)
{
    SrsStream stream;

    if (stream.initialize(h264_raw_data, h264_raw_size) != ERROR_SUCCESS) {
        return false;
    }
    return srs_avc_startswith_annexb(&stream, pnb_start_code);
}

struct FlvContext {
    SrsFileReader reader;
    SrsFileWriter writer;
    SrsFlvEncoder enc;
    SrsFlvDecoder dec;
};
srs_flv_t srs_flv_open_read(const char* file)
{
    int ret         = ERROR_SUCCESS;
    FlvContext* flv = new FlvContext();

    if ((ret = flv->reader.open(file)) != ERROR_SUCCESS) {
        srs_freep(flv);
        return NULL;
    }
    if ((ret = flv->dec.initialize(&flv->reader)) != ERROR_SUCCESS) {
        srs_freep(flv);
        return NULL;
    }
    return flv;
}

srs_flv_t srs_flv_open_write(const char* file)
{
    int ret         = ERROR_SUCCESS;
    FlvContext* flv = new FlvContext();

    if ((ret = flv->writer.open(file)) != ERROR_SUCCESS) {
        srs_freep(flv);
        return NULL;
    }
    if ((ret = flv->enc.initialize(&flv->writer)) != ERROR_SUCCESS) {
        srs_freep(flv);
        return NULL;
    }
    return flv;
}

void srs_flv_close(srs_flv_t flv)
{
    FlvContext* context = (FlvContext *) flv;

    srs_freep(context);
}

int srs_flv_read_header(srs_flv_t flv, char header[9])
{
    int ret = ERROR_SUCCESS;
    FlvContext* context = (FlvContext *) flv;

    if (!context->reader.is_open()) {
        return ERROR_SYSTEM_IO_INVALID;
    }
    if ((ret = context->dec.read_header(header)) != ERROR_SUCCESS) {
        return ret;
    }
    char ts[4];
    if ((ret = context->dec.read_previous_tag_size(ts)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_flv_read_tag_header(srs_flv_t flv, char* ptype, int32_t* pdata_size, u_int32_t* ptime)
{
    int ret = ERROR_SUCCESS;
    FlvContext* context = (FlvContext *) flv;

    if (!context->reader.is_open()) {
        return ERROR_SYSTEM_IO_INVALID;
    }
    if ((ret = context->dec.read_tag_header(ptype, pdata_size, ptime)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_flv_read_tag_data(srs_flv_t flv, char* data, int32_t size)
{
    int ret = ERROR_SUCCESS;
    FlvContext* context = (FlvContext *) flv;

    if (!context->reader.is_open()) {
        return ERROR_SYSTEM_IO_INVALID;
    }
    if ((ret = context->dec.read_tag_data(data, size)) != ERROR_SUCCESS) {
        return ret;
    }
    char ts[4];
    if ((ret = context->dec.read_previous_tag_size(ts)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_flv_write_header(srs_flv_t flv, char header[9])
{
    int ret = ERROR_SUCCESS;
    FlvContext* context = (FlvContext *) flv;

    if (!context->writer.is_open()) {
        return ERROR_SYSTEM_IO_INVALID;
    }
    if ((ret = context->enc.write_header(header)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

int srs_flv_write_tag(srs_flv_t flv, char type, int32_t time, char* data, int size)
{
    int ret = ERROR_SUCCESS;
    FlvContext* context = (FlvContext *) flv;

    if (!context->writer.is_open()) {
        return ERROR_SYSTEM_IO_INVALID;
    }
    if (type == SRS_RTMP_TYPE_AUDIO) {
        return context->enc.write_audio(time, data, size);
    } else if (type == SRS_RTMP_TYPE_VIDEO) {
        return context->enc.write_video(time, data, size);
    } else {
        return context->enc.write_metadata(type, data, size);
    }
    return ret;
}

int srs_flv_size_tag(int data_size)
{
    return SrsFlvEncoder::size_tag(data_size);
}

int64_t srs_flv_tellg(srs_flv_t flv)
{
    FlvContext* context = (FlvContext *) flv;

    return context->reader.tellg();
}

void srs_flv_lseek(srs_flv_t flv, int64_t offset)
{
    FlvContext* context = (FlvContext *) flv;

    context->reader.lseek(offset);
}

srs_bool srs_flv_is_eof(int error_code)
{
    return error_code == ERROR_SYSTEM_FILE_EOF;
}

srs_bool srs_flv_is_sequence_header(char* data, int32_t size)
{
    return SrsFlvCodec::video_is_sequence_header(data, (int) size);
}

srs_bool srs_flv_is_keyframe(char* data, int32_t size)
{
    return SrsFlvCodec::video_is_keyframe(data, (int) size);
}

srs_amf0_t srs_amf0_parse(char* data, int size, int* nparsed)
{
    int ret         = ERROR_SUCCESS;
    srs_amf0_t amf0 = NULL;
    SrsStream stream;

    if ((ret = stream.initialize(data, size)) != ERROR_SUCCESS) {
        return amf0;
    }
    SrsAmf0Any* any = NULL;
    if ((ret = SrsAmf0Any::discovery(&stream, &any)) != ERROR_SUCCESS) {
        return amf0;
    }
    stream.skip(-1 * stream.pos());
    if ((ret = any->read(&stream)) != ERROR_SUCCESS) {
        srs_freep(any);
        return amf0;
    }
    if (nparsed) {
        *nparsed = stream.pos();
    }
    amf0 = (srs_amf0_t) any;
    return amf0;
}

srs_amf0_t srs_amf0_create_string(const char* value)
{
    return SrsAmf0Any::str(value);
}

srs_amf0_t srs_amf0_create_number(srs_amf0_number value)
{
    return SrsAmf0Any::number(value);
}

srs_amf0_t srs_amf0_create_ecma_array()
{
    return SrsAmf0Any::ecma_array();
}

srs_amf0_t srs_amf0_create_strict_array()
{
    return SrsAmf0Any::strict_array();
}

srs_amf0_t srs_amf0_create_object()
{
    return SrsAmf0Any::object();
}

srs_amf0_t srs_amf0_ecma_array_to_object(srs_amf0_t ecma_arr)
{
    srs_assert(srs_amf0_is_ecma_array(ecma_arr));
    SrsAmf0EcmaArray* arr = (SrsAmf0EcmaArray *) ecma_arr;
    SrsAmf0Object* obj    = SrsAmf0Any::object();
    for (int i = 0; i < arr->count(); i++) {
        std::string key   = arr->key_at(i);
        SrsAmf0Any* value = arr->value_at(i);
        obj->set(key, value->copy());
    }
    return obj;
}

void srs_amf0_free(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_freep(any);
}

int srs_amf0_size(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    return any->total_size();
}

int srs_amf0_serialize(srs_amf0_t amf0, char* data, int size)
{
    int ret         = ERROR_SUCCESS;
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;
    SrsStream stream;

    if ((ret = stream.initialize(data, size)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = any->write(&stream)) != ERROR_SUCCESS) {
        return ret;
    }
    return ret;
}

srs_bool srs_amf0_is_string(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    return any->is_string();
}

srs_bool srs_amf0_is_boolean(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    return any->is_boolean();
}

srs_bool srs_amf0_is_number(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    return any->is_number();
}

srs_bool srs_amf0_is_null(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    return any->is_null();
}

srs_bool srs_amf0_is_object(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    return any->is_object();
}

srs_bool srs_amf0_is_ecma_array(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    return any->is_ecma_array();
}

srs_bool srs_amf0_is_strict_array(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    return any->is_strict_array();
}

const char * srs_amf0_to_string(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    return any->to_str_raw();
}

srs_bool srs_amf0_to_boolean(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    return any->to_boolean();
}

srs_amf0_number srs_amf0_to_number(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    return any->to_number();
}

void srs_amf0_set_number(srs_amf0_t amf0, srs_amf0_number value)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    any->set_number(value);
}

int srs_amf0_object_property_count(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_object());
    SrsAmf0Object* obj = (SrsAmf0Object *) amf0;
    return obj->count();
}

const char * srs_amf0_object_property_name_at(srs_amf0_t amf0, int index)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_object());
    SrsAmf0Object* obj = (SrsAmf0Object *) amf0;
    return obj->key_raw_at(index);
}

srs_amf0_t srs_amf0_object_property_value_at(srs_amf0_t amf0, int index)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_object());
    SrsAmf0Object* obj = (SrsAmf0Object *) amf0;
    return (srs_amf0_t) obj->value_at(index);
}

srs_amf0_t srs_amf0_object_property(srs_amf0_t amf0, const char* name)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_object());
    SrsAmf0Object* obj = (SrsAmf0Object *) amf0;
    return (srs_amf0_t) obj->get_property(name);
}

void srs_amf0_object_property_set(srs_amf0_t amf0, const char* name, srs_amf0_t value)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_object());
    SrsAmf0Object* obj = (SrsAmf0Object *) amf0;
    any = (SrsAmf0Any *) value;
    obj->set(name, any);
}

void srs_amf0_object_clear(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_object());
    SrsAmf0Object* obj = (SrsAmf0Object *) amf0;
    obj->clear();
}

int srs_amf0_ecma_array_property_count(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_ecma_array());
    SrsAmf0EcmaArray* obj = (SrsAmf0EcmaArray *) amf0;
    return obj->count();
}

const char * srs_amf0_ecma_array_property_name_at(srs_amf0_t amf0, int index)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_ecma_array());
    SrsAmf0EcmaArray* obj = (SrsAmf0EcmaArray *) amf0;
    return obj->key_raw_at(index);
}

srs_amf0_t srs_amf0_ecma_array_property_value_at(srs_amf0_t amf0, int index)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_ecma_array());
    SrsAmf0EcmaArray* obj = (SrsAmf0EcmaArray *) amf0;
    return (srs_amf0_t) obj->value_at(index);
}

srs_amf0_t srs_amf0_ecma_array_property(srs_amf0_t amf0, const char* name)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_ecma_array());
    SrsAmf0EcmaArray* obj = (SrsAmf0EcmaArray *) amf0;
    return (srs_amf0_t) obj->get_property(name);
}

void srs_amf0_ecma_array_property_set(srs_amf0_t amf0, const char* name, srs_amf0_t value)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_ecma_array());
    SrsAmf0EcmaArray* obj = (SrsAmf0EcmaArray *) amf0;
    any = (SrsAmf0Any *) value;
    obj->set(name, any);
}

int srs_amf0_strict_array_property_count(srs_amf0_t amf0)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_strict_array());
    SrsAmf0StrictArray* obj = (SrsAmf0StrictArray *) amf0;
    return obj->count();
}

srs_amf0_t srs_amf0_strict_array_property_at(srs_amf0_t amf0, int index)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_strict_array());
    SrsAmf0StrictArray* obj = (SrsAmf0StrictArray *) amf0;
    return (srs_amf0_t) obj->at(index);
}

void srs_amf0_strict_array_append(srs_amf0_t amf0, srs_amf0_t value)
{
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;

    srs_assert(any->is_strict_array());
    SrsAmf0StrictArray* obj = (SrsAmf0StrictArray *) amf0;
    any = (SrsAmf0Any *) value;
    obj->append(any);
}

int64_t srs_utils_time_ms()
{
    return srs_update_system_time_ms();
}

int64_t srs_utils_send_bytes(srs_rtmp_t rtmp)
{
    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    return context->rtmp->get_send_bytes();
}

int64_t srs_utils_recv_bytes(srs_rtmp_t rtmp)
{
    srs_assert(rtmp != NULL);
    Context* context = (Context *) rtmp;
    return context->rtmp->get_recv_bytes();
}

int srs_utils_parse_timestamp(
    u_int32_t time, char type, char* data, int size,
    u_int32_t* ppts
)
{
    int ret = ERROR_SUCCESS;

    if (type != SRS_RTMP_TYPE_VIDEO) {
        *ppts = time;
        return ret;
    }
    if (!SrsFlvCodec::video_is_h264(data, size)) {
        return ERROR_FLV_INVALID_VIDEO_TAG;
    }
    if (SrsFlvCodec::video_is_sequence_header(data, size)) {
        *ppts = time;
        return ret;
    }
    if (size < 5) {
        return ERROR_FLV_INVALID_VIDEO_TAG;
    }
    u_int32_t cts = 0;
    char* p       = data + 2;
    char* pp      = (char *) &cts;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;
    *ppts = time + cts;
    return ret;
}

srs_bool srs_utils_flv_tag_is_ok(char type)
{
    return type == SRS_RTMP_TYPE_AUDIO || type == SRS_RTMP_TYPE_VIDEO || type == SRS_RTMP_TYPE_SCRIPT;
}

srs_bool srs_utils_flv_tag_is_audio(char type)
{
    return type == SRS_RTMP_TYPE_AUDIO;
}

srs_bool srs_utils_flv_tag_is_video(char type)
{
    return type == SRS_RTMP_TYPE_VIDEO;
}

srs_bool srs_utils_flv_tag_is_av(char type)
{
    return type == SRS_RTMP_TYPE_AUDIO || type == SRS_RTMP_TYPE_VIDEO;
}

char srs_utils_flv_video_codec_id(char* data, int size)
{
    if (size < 1) {
        return 0;
    }
    char codec_id = data[0];
    codec_id = codec_id & 0x0F;
    return codec_id;
}

char srs_utils_flv_video_avc_packet_type(char* data, int size)
{
    if (size < 2) {
        return -1;
    }
    if (!SrsFlvCodec::video_is_h264(data, size)) {
        return -1;
    }
    u_int8_t avc_packet_type = data[1];
    if (avc_packet_type > 2) {
        return -1;
    }
    return avc_packet_type;
}

char srs_utils_flv_video_frame_type(char* data, int size)
{
    if (size < 1) {
        return -1;
    }
    if (!SrsFlvCodec::video_is_h264(data, size)) {
        return -1;
    }
    u_int8_t frame_type = data[0];
    frame_type = (frame_type >> 4) & 0x0f;
    if (frame_type < 1 || frame_type > 5) {
        return -1;
    }
    return frame_type;
}

char srs_utils_flv_audio_sound_format(char* data, int size)
{
    if (size < 1) {
        return -1;
    }
    u_int8_t sound_format = data[0];
    sound_format = (sound_format >> 4) & 0x0f;
    if (sound_format > 15 || sound_format == 12 || sound_format == 13) {
        return -1;
    }
    return sound_format;
}

char srs_utils_flv_audio_sound_rate(char* data, int size)
{
    if (size < 1) {
        return -1;
    }
    u_int8_t sound_rate = data[0];
    sound_rate = (sound_rate >> 2) & 0x03;
    if (sound_rate > 3) {
        return -1;
    }
    return sound_rate;
}

char srs_utils_flv_audio_sound_size(char* data, int size)
{
    if (size < 1) {
        return -1;
    }
    u_int8_t sound_size = data[0];
    sound_size = (sound_size >> 1) & 0x01;
    if (sound_size > 1) {
        return -1;
    }
    return sound_size;
}

char srs_utils_flv_audio_sound_type(char* data, int size)
{
    if (size < 1) {
        return -1;
    }
    u_int8_t sound_type = data[0];
    sound_type = sound_type & 0x01;
    if (sound_type > 1) {
        return -1;
    }
    return sound_type;
}

char srs_utils_flv_audio_aac_packet_type(char* data, int size)
{
    if (size < 2) {
        return -1;
    }
    if (srs_utils_flv_audio_sound_format(data, size) != 10) {
        return -1;
    }
    u_int8_t aac_packet_type = data[1];
    if (aac_packet_type > 1) {
        return -1;
    }
    return aac_packet_type;
}

char * srs_human_amf0_print(srs_amf0_t amf0, char** pdata, int* psize)
{
    if (!amf0) {
        return NULL;
    }
    SrsAmf0Any* any = (SrsAmf0Any *) amf0;
    return any->human_print(pdata, psize);
}

const char * srs_human_flv_tag_type2string(char type)
{
    static const char* audio   = "Audio";
    static const char* video   = "Video";
    static const char* data    = "Data";
    static const char* unknown = "Unknown";

    switch (type) {
        case SRS_RTMP_TYPE_AUDIO:
            return audio;

        case SRS_RTMP_TYPE_VIDEO:
            return video;

        case SRS_RTMP_TYPE_SCRIPT:
            return data;

        default:
            return unknown;
    }
    return unknown;
}

const char * srs_human_flv_video_codec_id2string(char codec_id)
{
    static const char* h263      = "H.263";
    static const char* screen    = "Screen";
    static const char* vp6       = "VP6";
    static const char* vp6_alpha = "VP6Alpha";
    static const char* screen2   = "Screen2";
    static const char* h264      = "H.264";
    static const char* unknown   = "Unknown";

    switch (codec_id) {
        case 2:
            return h263;

        case 3:
            return screen;

        case 4:
            return vp6;

        case 5:
            return vp6_alpha;

        case 6:
            return screen2;

        case 7:
            return h264;

        default:
            return unknown;
    }
    return unknown;
} // srs_human_flv_video_codec_id2string

const char * srs_human_flv_video_avc_packet_type2string(char avc_packet_type)
{
    static const char* sps_pps     = "SH";
    static const char* nalu        = "Nalu";
    static const char* sps_pps_end = "SpsPpsEnd";
    static const char* unknown     = "Unknown";

    switch (avc_packet_type) {
        case 0:
            return sps_pps;

        case 1:
            return nalu;

        case 2:
            return sps_pps_end;

        default:
            return unknown;
    }
    return unknown;
}

const char * srs_human_flv_video_frame_type2string(char frame_type)
{
    static const char* keyframe   = "I";
    static const char* interframe = "P/B";
    static const char* disposable_interframe = "DI";
    static const char* generated_keyframe    = "GI";
    static const char* video_infoframe       = "VI";
    static const char* unknown = "Unknown";

    switch (frame_type) {
        case 1:
            return keyframe;

        case 2:
            return interframe;

        case 3:
            return disposable_interframe;

        case 4:
            return generated_keyframe;

        case 5:
            return video_infoframe;

        default:
            return unknown;
    }
    return unknown;
}

const char * srs_human_flv_audio_sound_format2string(char sound_format)
{
    static const char* linear_pcm       = "LinearPCM";
    static const char* ad_pcm           = "ADPCM";
    static const char* mp3              = "MP3";
    static const char* linear_pcm_le    = "LinearPCMLe";
    static const char* nellymoser_16khz = "NellymoserKHz16";
    static const char* nellymoser_8khz  = "NellymoserKHz8";
    static const char* nellymoser       = "Nellymoser";
    static const char* g711_a_pcm       = "G711APCM";
    static const char* g711_mu_pcm      = "G711MuPCM";
    static const char* reserved         = "Reserved";
    static const char* aac              = "AAC";
    static const char* speex            = "Speex";
    static const char* mp3_8khz         = "MP3KHz8";
    static const char* device_specific  = "DeviceSpecific";
    static const char* unknown          = "Unknown";

    switch (sound_format) {
        case 0:
            return linear_pcm;

        case 1:
            return ad_pcm;

        case 2:
            return mp3;

        case 3:
            return linear_pcm_le;

        case 4:
            return nellymoser_16khz;

        case 5:
            return nellymoser_8khz;

        case 6:
            return nellymoser;

        case 7:
            return g711_a_pcm;

        case 8:
            return g711_mu_pcm;

        case 9:
            return reserved;

        case 10:
            return aac;

        case 11:
            return speex;

        case 14:
            return mp3_8khz;

        case 15:
            return device_specific;

        default:
            return unknown;
    }
    return unknown;
} // srs_human_flv_audio_sound_format2string

const char * srs_human_flv_audio_sound_rate2string(char sound_rate)
{
    static const char* khz_5_5 = "5.5KHz";
    static const char* khz_11  = "11KHz";
    static const char* khz_22  = "22KHz";
    static const char* khz_44  = "44KHz";
    static const char* unknown = "Unknown";

    switch (sound_rate) {
        case 0:
            return khz_5_5;

        case 1:
            return khz_11;

        case 2:
            return khz_22;

        case 3:
            return khz_44;

        default:
            return unknown;
    }
    return unknown;
}

const char * srs_human_flv_audio_sound_size2string(char sound_size)
{
    static const char* bit_8   = "8bit";
    static const char* bit_16  = "16bit";
    static const char* unknown = "Unknown";

    switch (sound_size) {
        case 0:
            return bit_8;

        case 1:
            return bit_16;

        default:
            return unknown;
    }
    return unknown;
}

const char * srs_human_flv_audio_sound_type2string(char sound_type)
{
    static const char* mono    = "Mono";
    static const char* stereo  = "Stereo";
    static const char* unknown = "Unknown";

    switch (sound_type) {
        case 0:
            return mono;

        case 1:
            return stereo;

        default:
            return unknown;
    }
    return unknown;
}

const char * srs_human_flv_audio_aac_packet_type2string(char aac_packet_type)
{
    static const char* sps_pps = "SH";
    static const char* raw     = "Raw";
    static const char* unknown = "Unknown";

    switch (aac_packet_type) {
        case 0:
            return sps_pps;

        case 1:
            return raw;

        default:
            return unknown;
    }
    return unknown;
}

int srs_human_print_rtmp_packet(char type, u_int32_t timestamp, char* data, int size)
{
    return srs_human_print_rtmp_packet2(type, timestamp, data, size, 0);
}

int srs_human_print_rtmp_packet2(char type, u_int32_t timestamp, char* data, int size, u_int32_t pre_timestamp)
{
    return srs_human_print_rtmp_packet3(type, timestamp, data, size, pre_timestamp, 0);
}

int srs_human_print_rtmp_packet3(char type, u_int32_t timestamp, char* data, int size, u_int32_t pre_timestamp,
                                 int64_t pre_now)
{
    return srs_human_print_rtmp_packet4(type, timestamp, data, size, pre_timestamp, pre_now, 0, 0);
}

int srs_human_print_rtmp_packet4(char type, u_int32_t timestamp, char* data, int size, u_int32_t pre_timestamp,
                                 int64_t pre_now, int64_t starttime, int64_t nb_packets)
{
    int ret   = ERROR_SUCCESS;
    double pi = 0;

    if (pre_now > starttime) {
        pi = (pre_now - starttime) / (double) nb_packets;
    }
    double gfps = 0;
    if (pi > 0) {
        gfps = 1000 / pi;
    }
    int diff = 0;
    if (pre_timestamp > 0) {
        diff = (int) timestamp - (int) pre_timestamp;
    }
    int ndiff = 0;
    if (pre_now > 0) {
        ndiff = (int) (srs_utils_time_ms() - pre_now);
    }
    u_int32_t pts;
    if (srs_utils_parse_timestamp(timestamp, type, data, size, &pts) != 0) {
        srs_human_trace(
            "Rtmp packet id=%" PRId64 "/%.1f/%.1f, type=%s, dts=%d, ndiff=%d, diff=%d, size=%d, DecodeError",
            nb_packets, pi, gfps, srs_human_flv_tag_type2string(type), timestamp, ndiff, diff, size
        );
        return ret;
    }
    if (type == SRS_RTMP_TYPE_VIDEO) {
        srs_human_trace(
            "Video packet id=%" PRId64 "/%.1f/%.1f, type=%s, dts=%d, pts=%d, ndiff=%d, diff=%d, size=%d, %s(%s,%s)",
            nb_packets, pi, gfps, srs_human_flv_tag_type2string(type), timestamp, pts, ndiff, diff, size,
            srs_human_flv_video_codec_id2string(srs_utils_flv_video_codec_id(data, size)),
            srs_human_flv_video_avc_packet_type2string(srs_utils_flv_video_avc_packet_type(data, size)),
            srs_human_flv_video_frame_type2string(srs_utils_flv_video_frame_type(data, size))
        );
    } else if (type == SRS_RTMP_TYPE_AUDIO) {
        srs_human_trace(
            "Audio packet id=%" PRId64 "/%.1f/%.1f, type=%s, dts=%d, pts=%d, ndiff=%d, diff=%d, size=%d, %s(%s,%s,%s,%s)",
            nb_packets, pi, gfps, srs_human_flv_tag_type2string(type), timestamp, pts, ndiff, diff, size,
            srs_human_flv_audio_sound_format2string(srs_utils_flv_audio_sound_format(data, size)),
            srs_human_flv_audio_sound_rate2string(srs_utils_flv_audio_sound_rate(data, size)),
            srs_human_flv_audio_sound_size2string(srs_utils_flv_audio_sound_size(data, size)),
            srs_human_flv_audio_sound_type2string(srs_utils_flv_audio_sound_type(data, size)),
            srs_human_flv_audio_aac_packet_type2string(srs_utils_flv_audio_aac_packet_type(data, size))
        );
    } else if (type == SRS_RTMP_TYPE_SCRIPT) {
        srs_human_verbose("Data packet id=%" PRId64 "/%.1f/%.1f, type=%s, time=%d, ndiff=%d, diff=%d, size=%d",
                          nb_packets, pi, gfps, srs_human_flv_tag_type2string(type), timestamp, ndiff, diff, size);
        int nparsed = 0;
        while (nparsed < size) {
            int nb_parsed_this = 0;
            srs_amf0_t amf0    = srs_amf0_parse(data + nparsed, size - nparsed, &nb_parsed_this);
            if (amf0 == NULL) {
                break;
            }
            nparsed += nb_parsed_this;
            char* amf0_str = NULL;
            srs_human_raw("%s", srs_human_amf0_print(amf0, &amf0_str, NULL));
            srs_freepa(amf0_str);
        }
    } else {
        srs_human_trace("Rtmp packet id=%" PRId64 "/%.1f/%.1f, type=%#x, dts=%d, pts=%d, ndiff=%d, diff=%d, size=%d",
                        nb_packets, pi, gfps, type, timestamp, pts, ndiff, diff, size);
    }
    return ret;
} // srs_human_print_rtmp_packet4

const char * srs_human_format_time()
{
    struct timeval tv;
    static char buf[23];

    memset(buf, 0, sizeof(buf));
    if (gettimeofday(&tv, NULL) == -1) {
        return buf;
    }
    struct tm* tm;
    if ((tm = localtime((const time_t *) &tv.tv_sec)) == NULL) {
        return buf;
    }
    snprintf(buf, sizeof(buf),
             "%d-%02d-%02d %02d:%02d:%02d.%03d",
             1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec,
             (int) (tv.tv_usec / 1000));
    buf[sizeof(buf) - 1] = 0;
    return buf;
}

#ifdef SRS_HIJACK_IO
srs_hijack_io_t srs_hijack_io_get(srs_rtmp_t rtmp)
{
    if (!rtmp) {
        return NULL;
    }
    Context* context = (Context *) rtmp;
    if (!context->skt) {
        return NULL;
    }
    return context->skt->hijack_io();
}

#endif // ifdef SRS_HIJACK_IO
#ifdef __cplusplus
}
#endif
#ifndef _WIN32
# define SOCKET_ETIME      EWOULDBLOCK
# define SOCKET_ECONNRESET ECONNRESET
# define SOCKET_ERRNO()   errno
# define SOCKET_RESET(fd) fd = -1; (void) 0
# define SOCKET_CLOSE(fd) \
    if (fd > 0) { \
        ::close(fd); \
        fd = -1; \
    } \
    (void) 0
# define SOCKET_VALID(x)  (x > 0)
# define SOCKET_SETUP()   (void) 0
# define SOCKET_CLEANUP() (void) 0
#else // ifndef _WIN32
# define SOCKET_ETIME      WSAETIMEDOUT
# define SOCKET_ECONNRESET WSAECONNRESET
# define SOCKET_ERRNO()   WSAGetLastError()
# define SOCKET_RESET(x)  x = INVALID_SOCKET
# define SOCKET_CLOSE(x)  if (x != INVALID_SOCKET) { ::closesocket(x); x = INVALID_SOCKET; }
# define SOCKET_VALID(x)  (x != INVALID_SOCKET)
# define SOCKET_BUFF(x)   ((char *) x)
# define SOCKET_SETUP()   socket_setup()
# define SOCKET_CLEANUP() socket_cleanup()
#endif // ifndef _WIN32
#ifndef _WIN32
# include <unistd.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/uio.h>
#endif
#include <sys/types.h>
#include <errno.h>
#ifndef ST_UTIME_NO_TIMEOUT
# define ST_UTIME_NO_TIMEOUT -1
#endif
#ifndef SRS_HIJACK_IO
struct SrsBlockSyncSocket {
    SOCKET  fd;
    int64_t recv_timeout;
    int64_t send_timeout;
    int64_t recv_bytes;
    int64_t send_bytes;
    SrsBlockSyncSocket()
    {
        send_timeout = recv_timeout = ST_UTIME_NO_TIMEOUT;
        recv_bytes   = send_bytes = 0;
        SOCKET_RESET(fd);
        SOCKET_SETUP();
    }

    virtual ~SrsBlockSyncSocket()
    {
        SOCKET_CLOSE(fd);
        SOCKET_CLEANUP();
    }
};
srs_hijack_io_t srs_hijack_io_create()
{
    SrsBlockSyncSocket* skt = new SrsBlockSyncSocket();

    return skt;
}

void srs_hijack_io_destroy(srs_hijack_io_t ctx)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;

    srs_freep(skt);
}

int srs_hijack_io_create_socket(srs_hijack_io_t ctx)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;

    skt->fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (!SOCKET_VALID(skt->fd)) {
        return ERROR_SOCKET_CREATE;
    }
    # if defined(WEBRTC_IOS)
    int value = 1;
    setsockopt(skt->fd, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));
    # endif
    return ERROR_SUCCESS;
}

int srs_hijack_io_connect(srs_hijack_io_t ctx, const char* server_ip, int port)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;
    sockaddr_in addr;

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = inet_addr(server_ip);
    if (::connect(skt->fd, (const struct sockaddr *) &addr, sizeof(sockaddr_in)) < 0) {
        return ERROR_SOCKET_CONNECT;
    }
    return ERROR_SUCCESS;
}

int srs_hijack_io_disconnect(srs_hijack_io_t ctx)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;

    SOCKET_CLOSE(skt->fd);
    return ERROR_SUCCESS;
}

int srs_hijack_io_read(srs_hijack_io_t ctx, void* buf, size_t size, ssize_t* nread)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;
    int ret         = ERROR_SUCCESS;
    ssize_t nb_read = ::recv(skt->fd, (char *) buf, size, 0);

    if (nread) {
        *nread = nb_read;
    }
    if (nb_read <= 0) {
        if (nb_read < 0 && SOCKET_ERRNO() == SOCKET_ETIME) {
            return ERROR_SOCKET_TIMEOUT;
        }
        if (nb_read == 0) {
            errno = SOCKET_ECONNRESET;
        }
        return ERROR_SOCKET_READ;
    }
    skt->recv_bytes += nb_read;
    return ret;
}

int srs_hijack_io_set_recv_timeout(srs_hijack_io_t ctx, int64_t timeout_us)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;
    int sec      = (int) (timeout_us / 1000000LL);
    int microsec = (int) (timeout_us % 1000000LL);

    sec      = srs_max(0, sec);
    microsec = srs_max(0, microsec);
    struct timeval tv = { sec, microsec };
    # ifdef WIN32
    int value     = sec * 1000;
    const char* p = reinterpret_cast<const char *>(&value);
    if (setsockopt(skt->fd, SOL_SOCKET, SO_RCVTIMEO, p, sizeof(value)) == -1) {
    # else
    if (setsockopt(skt->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
        # endif
        return SOCKET_ERRNO();
    }
    skt->recv_timeout = timeout_us;
    return ERROR_SUCCESS;
}

int64_t srs_hijack_io_get_recv_timeout(srs_hijack_io_t ctx)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;

    return skt->recv_timeout;
}

int64_t srs_hijack_io_get_recv_bytes(srs_hijack_io_t ctx)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;

    return skt->recv_bytes;
}

int srs_hijack_io_set_send_timeout(srs_hijack_io_t ctx, int64_t timeout_us)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;
    int sec      = (int) (timeout_us / 1000000LL);
    int microsec = (int) (timeout_us % 1000000LL);

    sec      = srs_max(0, sec);
    microsec = srs_max(0, microsec);
    struct timeval tv = { sec, microsec };
    # ifdef WIN32
    int value     = sec * 1000;
    const char* p = reinterpret_cast<const char *>(&value);
    if (setsockopt(skt->fd, SOL_SOCKET, SO_SNDTIMEO, p, sizeof(value)) == -1) {
    # else
    if (setsockopt(skt->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == -1) {
        # endif
        return SOCKET_ERRNO();
    }
    skt->send_timeout = timeout_us;
    return ERROR_SUCCESS;
}

int64_t srs_hijack_io_get_send_timeout(srs_hijack_io_t ctx)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;

    return skt->send_timeout;
}

int64_t srs_hijack_io_get_send_bytes(srs_hijack_io_t ctx)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;

    return skt->send_bytes;
}

int srs_hijack_io_writev(srs_hijack_io_t ctx, const iovec* iov, int iov_size, ssize_t* nwrite)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;
    int ret = ERROR_SUCCESS;
    ssize_t nb_write = ::writev(skt->fd, iov, iov_size);

    if (nwrite) {
        *nwrite = nb_write;
    }
    if (nb_write <= 0) {
        if (nb_write < 0 && SOCKET_ERRNO() == SOCKET_ETIME) {
            return ERROR_SOCKET_TIMEOUT;
        }
        return ERROR_SOCKET_WRITE;
    }
    skt->send_bytes += nb_write;
    return ret;
}

bool srs_hijack_io_is_never_timeout(srs_hijack_io_t ctx, int64_t timeout_us)
{
    return timeout_us == (int64_t) ST_UTIME_NO_TIMEOUT;
}

int srs_hijack_io_read_fully(srs_hijack_io_t ctx, void* buf, size_t size, ssize_t* nread)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;
    int ret         = ERROR_SUCCESS;
    size_t left     = size;
    ssize_t nb_read = 0;

    while (left > 0) {
        char* this_buf = (char *) buf + nb_read;
        ssize_t this_nread;
        if ((ret = srs_hijack_io_read(ctx, this_buf, left, &this_nread)) != ERROR_SUCCESS) {
            return ret;
        }
        nb_read += this_nread;
        left    -= (size_t) this_nread;
    }
    if (nread) {
        *nread = nb_read;
    }
    skt->recv_bytes += nb_read;
    return ret;
}

int srs_hijack_io_write(srs_hijack_io_t ctx, void* buf, size_t size, ssize_t* nwrite)
{
    SrsBlockSyncSocket* skt = (SrsBlockSyncSocket *) ctx;
    int ret = ERROR_SUCCESS;
    ssize_t nb_write = ::send(skt->fd, (char *) buf, size, 0);

    if (nwrite) {
        *nwrite = nb_write;
    }
    if (nb_write <= 0) {
        if (nb_write < 0 && SOCKET_ERRNO() == SOCKET_ETIME) {
            return ERROR_SOCKET_TIMEOUT;
        }
        return ERROR_SOCKET_WRITE;
    }
    skt->send_bytes += nb_write;
    return ret;
}

#endif // ifndef SRS_HIJACK_IO
SimpleSocketStreamImpl::SimpleSocketStreamImpl()
{
    io = srs_hijack_io_create();
}

SimpleSocketStreamImpl::~SimpleSocketStreamImpl()
{
    if (io) {
        srs_hijack_io_destroy(io);
        io = NULL;
    }
}

srs_hijack_io_t SimpleSocketStreamImpl::hijack_io()
{
    return io;
}

int SimpleSocketStreamImpl::create_socket()
{
    srs_assert(io);
    return srs_hijack_io_create_socket(io);
}

int SimpleSocketStreamImpl::connect(const char* server_ip, int port)
{
    srs_assert(io);
    return srs_hijack_io_connect(io, server_ip, port);
}

int SimpleSocketStreamImpl::disconnect()
{
    srs_assert(io);
    return srs_hijack_io_disconnect(io);
}

int SimpleSocketStreamImpl::read(void* buf, size_t size, ssize_t* nread)
{
    srs_assert(io);
    return srs_hijack_io_read(io, buf, size, nread);
}

void SimpleSocketStreamImpl::set_recv_timeout(int64_t timeout_us)
{
    srs_assert(io);
    srs_hijack_io_set_recv_timeout(io, timeout_us);
}

int64_t SimpleSocketStreamImpl::get_recv_timeout()
{
    srs_assert(io);
    return srs_hijack_io_get_recv_timeout(io);
}

int64_t SimpleSocketStreamImpl::get_recv_bytes()
{
    srs_assert(io);
    return srs_hijack_io_get_recv_bytes(io);
}

void SimpleSocketStreamImpl::set_send_timeout(int64_t timeout_us)
{
    srs_assert(io);
    srs_hijack_io_set_send_timeout(io, timeout_us);
}

int64_t SimpleSocketStreamImpl::get_send_timeout()
{
    srs_assert(io);
    return srs_hijack_io_get_send_timeout(io);
}

int64_t SimpleSocketStreamImpl::get_send_bytes()
{
    srs_assert(io);
    return srs_hijack_io_get_send_bytes(io);
}

int SimpleSocketStreamImpl::writev(const iovec* iov, int iov_size, ssize_t* nwrite)
{
    srs_assert(io);
    return srs_hijack_io_writev(io, iov, iov_size, nwrite);
}

bool SimpleSocketStreamImpl::is_never_timeout(int64_t timeout_us)
{
    srs_assert(io);
    return srs_hijack_io_is_never_timeout(io, timeout_us);
}

int SimpleSocketStreamImpl::read_fully(void* buf, size_t size, ssize_t* nread)
{
    srs_assert(io);
    return srs_hijack_io_read_fully(io, buf, size, nread);
}

int SimpleSocketStreamImpl::write(void* buf, size_t size, ssize_t* nwrite)
{
    srs_assert(io);
    return srs_hijack_io_write(io, buf, size, nwrite);
}

#ifndef _WIN32
# include <unistd.h>
#endif
#include <sstream>
using namespace std;
typedef bool (* _CheckPacketType)(SrsBandwidthPacket* pkt);
bool _bandwidth_is_start_play(SrsBandwidthPacket* pkt)
{
    return pkt->is_start_play();
}

bool _bandwidth_is_stop_play(SrsBandwidthPacket* pkt)
{
    return pkt->is_stop_play();
}

bool _bandwidth_is_start_publish(SrsBandwidthPacket* pkt)
{
    return pkt->is_start_publish();
}

bool _bandwidth_is_stop_publish(SrsBandwidthPacket* pkt)
{
    return pkt->is_stop_publish();
}

bool _bandwidth_is_finish(SrsBandwidthPacket* pkt)
{
    return pkt->is_finish();
}

int _srs_expect_bandwidth_packet(SrsRtmpClient* rtmp, _CheckPacketType pfn)
{
    int ret = ERROR_SUCCESS;

    while (true) {
        SrsCommonMessage* msg   = NULL;
        SrsBandwidthPacket* pkt = NULL;
        if ((ret = rtmp->expect_message<SrsBandwidthPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
            return ret;
        }
        SrsAutoFree(SrsCommonMessage, msg);
        SrsAutoFree(SrsBandwidthPacket, pkt);
        srs_info("get final message success.");
        if (pfn(pkt)) {
            return ret;
        }
    }
    return ret;
}

int _srs_expect_bandwidth_packet2(SrsRtmpClient* rtmp, _CheckPacketType pfn, SrsBandwidthPacket** ppkt)
{
    int ret = ERROR_SUCCESS;

    while (true) {
        SrsCommonMessage* msg   = NULL;
        SrsBandwidthPacket* pkt = NULL;
        if ((ret = rtmp->expect_message<SrsBandwidthPacket>(&msg, &pkt)) != ERROR_SUCCESS) {
            return ret;
        }
        SrsAutoFree(SrsCommonMessage, msg);
        srs_info("get final message success.");
        if (pfn(pkt)) {
            *ppkt = pkt;
            return ret;
        }
        srs_freep(pkt);
    }
    return ret;
}

SrsBandwidthClient::SrsBandwidthClient()
{
    _rtmp = NULL;
}

SrsBandwidthClient::~SrsBandwidthClient()
{}

int SrsBandwidthClient::initialize(SrsRtmpClient* rtmp)
{
    _rtmp = rtmp;
    return ERROR_SUCCESS;
}

int SrsBandwidthClient::bandwidth_check(
    int64_t* start_time, int64_t* end_time,
    int* play_kbps, int* publish_kbps,
    int* play_bytes, int* publish_bytes,
    int* play_duration, int* publish_duration
)
{
    int ret = ERROR_SUCCESS;

    srs_update_system_time_ms();
    *start_time = srs_get_system_time_ms();
    if ((ret = play_start()) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = play_checking()) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = play_stop()) != ERROR_SUCCESS) {
        return ret;
    }
    int duration_ms      = 0;
    int actual_play_kbps = 0;
    if ((ret = publish_start(duration_ms, actual_play_kbps)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = publish_checking(duration_ms, actual_play_kbps)) != ERROR_SUCCESS) {
        return ret;
    }
    if ((ret = publish_stop()) != ERROR_SUCCESS) {
        return ret;
    }
    SrsBandwidthPacket* pkt = NULL;
    if ((ret = final (&pkt)) != ERROR_SUCCESS) {
        return ret;
    }
    SrsAutoFree(SrsBandwidthPacket, pkt);
    if (true) {
        SrsAmf0Any* prop = NULL;
        if ((prop = pkt->data->ensure_property_number("play_kbps")) != NULL) {
            *play_kbps = (int) prop->to_number();
        }
        if ((prop = pkt->data->ensure_property_number("publish_kbps")) != NULL) {
            *publish_kbps = (int) prop->to_number();
        }
        if ((prop = pkt->data->ensure_property_number("play_bytes")) != NULL) {
            *play_bytes = (int) prop->to_number();
        }
        if ((prop = pkt->data->ensure_property_number("publish_bytes")) != NULL) {
            *publish_bytes = (int) prop->to_number();
        }
        if ((prop = pkt->data->ensure_property_number("play_time")) != NULL) {
            *play_duration = (int) prop->to_number();
        }
        if ((prop = pkt->data->ensure_property_number("publish_time")) != NULL) {
            *publish_duration = (int) prop->to_number();
        }
    }
    srs_update_system_time_ms();
    *end_time = srs_get_system_time_ms();
    return ret;
} // SrsBandwidthClient::bandwidth_check

int SrsBandwidthClient::play_start()
{
    int ret = ERROR_SUCCESS;

    if ((ret = _srs_expect_bandwidth_packet(_rtmp, _bandwidth_is_start_play)) != ERROR_SUCCESS) {
        return ret;
    }
    srs_info("BW check recv play begin request.");
    if (true) {
        SrsBandwidthPacket* pkt = SrsBandwidthPacket::create_starting_play();
        if ((ret = _rtmp->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send bandwidth check start play message failed. ret=%d", ret);
            return ret;
        }
    }
    srs_info("BW check play begin.");
    return ret;
}

int SrsBandwidthClient::play_checking()
{
    int ret = ERROR_SUCCESS;

    return ret;
}

int SrsBandwidthClient::play_stop()
{
    int ret = ERROR_SUCCESS;

    if ((ret = _srs_expect_bandwidth_packet(_rtmp, _bandwidth_is_stop_play)) != ERROR_SUCCESS) {
        return ret;
    }
    srs_info("BW check recv play stop request.");
    if (true) {
        SrsBandwidthPacket* pkt = SrsBandwidthPacket::create_stopped_play();
        if ((ret = _rtmp->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send bandwidth check stop play message failed. ret=%d", ret);
            return ret;
        }
    }
    srs_info("BW check play stop.");
    return ret;
}

int SrsBandwidthClient::publish_start(int& duration_ms, int& play_kbps)
{
    int ret = ERROR_SUCCESS;

    if (true) {
        SrsBandwidthPacket* pkt = NULL;
        if ((ret = _srs_expect_bandwidth_packet2(_rtmp, _bandwidth_is_start_publish, &pkt)) != ERROR_SUCCESS) {
            return ret;
        }
        SrsAutoFree(SrsBandwidthPacket, pkt);
        SrsAmf0Any* prop = NULL;
        if ((prop = pkt->data->ensure_property_number("duration_ms")) != NULL) {
            duration_ms = (int) prop->to_number();
        }
        if ((prop = pkt->data->ensure_property_number("limit_kbps")) != NULL) {
            play_kbps = (int) prop->to_number();
        }
    }
    srs_info("BW check recv publish begin request.");
    if (true) {
        SrsBandwidthPacket* pkt = SrsBandwidthPacket::create_starting_publish();
        if ((ret = _rtmp->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send bandwidth check start publish message failed. ret=%d", ret);
            return ret;
        }
    }
    srs_info("BW check publish begin.");
    return ret;
}

int SrsBandwidthClient::publish_checking(int duration_ms, int play_kbps)
{
    int ret = ERROR_SUCCESS;

    if (duration_ms <= 0) {
        ret = ERROR_RTMP_BWTC_DATA;
        srs_error("server must specifies the duration, ret=%d", ret);
        return ret;
    }
    if (play_kbps <= 0) {
        ret = ERROR_RTMP_BWTC_DATA;
        srs_error("server must specifies the play kbp, ret=%d", ret);
        return ret;
    }
    int data_count = 1;
    srs_update_system_time_ms();
    int64_t starttime = srs_get_system_time_ms();
    while ((srs_get_system_time_ms() - starttime) < duration_ms) {
        SrsBandwidthPacket* pkt = SrsBandwidthPacket::create_publishing();
        for (int i = 0; i < data_count; ++i) {
            std::stringstream seq;
            seq << i;
            std::string play_data = "SRS band check data from server's publishing......";
            pkt->data->set(seq.str(), SrsAmf0Any::str(play_data.c_str()));
        }
        data_count += 2;
        if ((ret = _rtmp->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send bandwidth check publish messages failed. ret=%d", ret);
            return ret;
        }
        srs_update_system_time_ms();
        int elaps = (int) (srs_get_system_time_ms() - starttime);
        if (elaps > 0) {
            int current_kbps = (int) (_rtmp->get_send_bytes() * 8 / elaps);
            while (current_kbps > play_kbps) {
                srs_update_system_time_ms();
                elaps        = (int) (srs_get_system_time_ms() - starttime);
                current_kbps = (int) (_rtmp->get_send_bytes() * 8 / elaps);
                usleep(100 * 1000);
            }
        }
    }
    srs_info("BW check send publish bytes over.");
    return ret;
} // SrsBandwidthClient::publish_checking

int SrsBandwidthClient::publish_stop()
{
    int ret = ERROR_SUCCESS;

    if (true) {
        SrsBandwidthPacket* pkt = SrsBandwidthPacket::create_stop_publish();
        if ((ret = _rtmp->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send bandwidth check stop publish message failed. ret=%d", ret);
            return ret;
        }
    }
    srs_info("BW client stop publish request.");
    if ((ret = _srs_expect_bandwidth_packet(_rtmp, _bandwidth_is_stop_publish)) != ERROR_SUCCESS) {
        return ret;
    }
    srs_info("BW check recv publish stop request.");
    if (true) {
        SrsBandwidthPacket* pkt = SrsBandwidthPacket::create_stopped_publish();
        if ((ret = _rtmp->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send bandwidth check stop publish message failed. ret=%d", ret);
            return ret;
        }
    }
    srs_info("BW check publish stop.");
    return ret;
}

int SrsBandwidthClient::final (SrsBandwidthPacket**ppkt)
{
    int ret = ERROR_SUCCESS;
    if ((ret = _srs_expect_bandwidth_packet2(_rtmp, _bandwidth_is_finish, ppkt)) != ERROR_SUCCESS) {
        return ret;
    }
    srs_info("BW check recv finish/report request.");
    if (true) {
        SrsBandwidthPacket* pkt = SrsBandwidthPacket::create_final();
        if ((ret = _rtmp->send_and_free_packet(pkt, 0)) != ERROR_SUCCESS) {
            srs_error("send bandwidth check final message failed. ret=%d", ret);
            return ret;
        }
    }
    srs_info("BW check final.");
    return ret;
}
