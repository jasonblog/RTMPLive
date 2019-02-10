#include "ourMD5.hh"
#include <NetCommon.h>
#include <string.h>
#define DIGEST_SIZE_IN_BYTES      16
#define DIGEST_SIZE_IN_HEX_DIGITS (2*DIGEST_SIZE_IN_BYTES)
#define DIGEST_SIZE_AS_STRING     (DIGEST_SIZE_IN_HEX_DIGITS+1)
class MD5Context
{
public:
    MD5Context();
    ~MD5Context();
    void addData(unsigned char const* inputData, unsigned inputDataSize);
    void end(char* outputDigest);
    void finalize(unsigned char* outputDigestInBytes);
private:
    void zeroize();
    void transform64Bytes(unsigned char const block[64]);
private:
    u_int32_t fState[4];
    u_int64_t fBitCount;
    unsigned char fWorkingBuffer[64];
};
char * our_MD5Data(unsigned char const* data, unsigned dataSize, char* outputDigest)
{
    MD5Context ctx;

    ctx.addData(data, dataSize);
    if (outputDigest == NULL) {
        outputDigest = new char[DIGEST_SIZE_AS_STRING];
    }
    ctx.end(outputDigest);
    return outputDigest;
}

unsigned char * our_MD5DataRaw(unsigned char const* data, unsigned dataSize,
                               unsigned char* outputDigest)
{
    MD5Context ctx;

    ctx.addData(data, dataSize);
    if (outputDigest == NULL) {
        outputDigest = new unsigned char[DIGEST_SIZE_IN_BYTES];
    }
    ctx.finalize(outputDigest);
    return outputDigest;
}

MD5Context::MD5Context()
    : fBitCount(0)
{
    fState[0] = 0x67452301;
    fState[1] = 0xefcdab89;
    fState[2] = 0x98badcfe;
    fState[3] = 0x10325476;
}

MD5Context::~MD5Context()
{
    zeroize();
}

void MD5Context::addData(unsigned char const* inputData, unsigned inputDataSize)
{
    u_int64_t const byteCount     = fBitCount >> 3;
    unsigned bufferBytesInUse     = (unsigned) (byteCount & 0x3F);
    unsigned bufferBytesRemaining = 64 - bufferBytesInUse;

    fBitCount += inputDataSize << 3;
    unsigned i = 0;
    if (inputDataSize >= bufferBytesRemaining) {
        memcpy((unsigned char *) &fWorkingBuffer[bufferBytesInUse], (unsigned char *) inputData, bufferBytesRemaining);
        transform64Bytes(fWorkingBuffer);
        bufferBytesInUse = 0;
        for (i = bufferBytesRemaining; i + 63 < inputDataSize; i += 64) {
            transform64Bytes(&inputData[i]);
        }
    }
    if (i < inputDataSize) {
        memcpy((unsigned char *) &fWorkingBuffer[bufferBytesInUse], (unsigned char *) &inputData[i], inputDataSize - i);
    }
}

void MD5Context::end(char* outputDigest)
{
    unsigned char digestInBytes[DIGEST_SIZE_IN_BYTES];

    finalize(digestInBytes);
    static char const hex[] = "0123456789abcdef";
    unsigned i;
    for (i = 0; i < DIGEST_SIZE_IN_BYTES; ++i) {
        outputDigest[2 * i]     = hex[digestInBytes[i] >> 4];
        outputDigest[2 * i + 1] = hex[digestInBytes[i] & 0x0F];
    }
    outputDigest[2 * i] = '\0';
}

static void unpack32(unsigned char out[4], u_int32_t in)
{
    for (unsigned i = 0; i < 4; ++i) {
        out[i] = (unsigned char) ((in >> (8 * i)) & 0xFF);
    }
}

static void unpack64(unsigned char out[8], u_int64_t in)
{
    for (unsigned i = 0; i < 8; ++i) {
        out[i] = (unsigned char) ((in >> (8 * i)) & 0xFF);
    }
}

static unsigned char const PADDING[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
void MD5Context::finalize(unsigned char* outputDigestInBytes)
{
    unsigned char bitCountInBytes[8];

    unpack64(bitCountInBytes, fBitCount);
    u_int64_t const byteCount = fBitCount >> 3;
    unsigned bufferBytesInUse = (unsigned) (byteCount & 0x3F);
    unsigned numPaddingBytes  =
        (bufferBytesInUse < 56) ? (56 - bufferBytesInUse) : (64 + 56 - bufferBytesInUse);
    addData(PADDING, numPaddingBytes);
    addData(bitCountInBytes, 8);
    unpack32(&outputDigestInBytes[0], fState[0]);
    unpack32(&outputDigestInBytes[4], fState[1]);
    unpack32(&outputDigestInBytes[8], fState[2]);
    unpack32(&outputDigestInBytes[12], fState[3]);
    zeroize();
}

void MD5Context::zeroize()
{
    fState[0] = fState[1] = fState[2] = fState[3] = 0;
    fBitCount = 0;
    for (unsigned i = 0; i < 64; ++i) {
        fWorkingBuffer[i] = 0;
    }
}

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21
#define F(x, y, z)        (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z)        (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z)        ((x) ^ (y) ^ (z))
#define I(x, y, z)        ((y) ^ ((x) | (~z)))
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#define FF(a, b, c, d, x, s, ac) \
    { \
        (a) += F((b), (c), (d)) + (x) + (u_int32_t) (ac); \
        (a)  = ROTATE_LEFT((a), (s)); \
        (a) += (b); \
    }
#define GG(a, b, c, d, x, s, ac) \
    { \
        (a) += G((b), (c), (d)) + (x) + (u_int32_t) (ac); \
        (a)  = ROTATE_LEFT((a), (s)); \
        (a) += (b); \
    }
#define HH(a, b, c, d, x, s, ac) \
    { \
        (a) += H((b), (c), (d)) + (x) + (u_int32_t) (ac); \
        (a)  = ROTATE_LEFT((a), (s)); \
        (a) += (b); \
    }
#define II(a, b, c, d, x, s, ac) \
    { \
        (a) += I((b), (c), (d)) + (x) + (u_int32_t) (ac); \
        (a)  = ROTATE_LEFT((a), (s)); \
        (a) += (b); \
    }
void MD5Context::transform64Bytes(unsigned char const block[64])
{
    u_int32_t a = fState[0], b = fState[1], c = fState[2], d = fState[3];
    u_int32_t x[16];

    for (unsigned i = 0, j = 0; i < 16; ++i, j += 4) {
        x[i] = ((u_int32_t) block[j]) | (((u_int32_t) block[j + 1]) << 8) | (((u_int32_t) block[j + 2]) << 16)
               | (((u_int32_t) block[j + 3]) << 24);
    }
    FF(a, b, c, d, x[0], S11, 0xd76aa478);
    FF(d, a, b, c, x[1], S12, 0xe8c7b756);
    FF(c, d, a, b, x[2], S13, 0x242070db);
    FF(b, c, d, a, x[3], S14, 0xc1bdceee);
    FF(a, b, c, d, x[4], S11, 0xf57c0faf);
    FF(d, a, b, c, x[5], S12, 0x4787c62a);
    FF(c, d, a, b, x[6], S13, 0xa8304613);
    FF(b, c, d, a, x[7], S14, 0xfd469501);
    FF(a, b, c, d, x[8], S11, 0x698098d8);
    FF(d, a, b, c, x[9], S12, 0x8b44f7af);
    FF(c, d, a, b, x[10], S13, 0xffff5bb1);
    FF(b, c, d, a, x[11], S14, 0x895cd7be);
    FF(a, b, c, d, x[12], S11, 0x6b901122);
    FF(d, a, b, c, x[13], S12, 0xfd987193);
    FF(c, d, a, b, x[14], S13, 0xa679438e);
    FF(b, c, d, a, x[15], S14, 0x49b40821);
    GG(a, b, c, d, x[1], S21, 0xf61e2562);
    GG(d, a, b, c, x[6], S22, 0xc040b340);
    GG(c, d, a, b, x[11], S23, 0x265e5a51);
    GG(b, c, d, a, x[0], S24, 0xe9b6c7aa);
    GG(a, b, c, d, x[5], S21, 0xd62f105d);
    GG(d, a, b, c, x[10], S22, 0x2441453);
    GG(c, d, a, b, x[15], S23, 0xd8a1e681);
    GG(b, c, d, a, x[4], S24, 0xe7d3fbc8);
    GG(a, b, c, d, x[9], S21, 0x21e1cde6);
    GG(d, a, b, c, x[14], S22, 0xc33707d6);
    GG(c, d, a, b, x[3], S23, 0xf4d50d87);
    GG(b, c, d, a, x[8], S24, 0x455a14ed);
    GG(a, b, c, d, x[13], S21, 0xa9e3e905);
    GG(d, a, b, c, x[2], S22, 0xfcefa3f8);
    GG(c, d, a, b, x[7], S23, 0x676f02d9);
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);
    HH(a, b, c, d, x[5], S31, 0xfffa3942);
    HH(d, a, b, c, x[8], S32, 0x8771f681);
    HH(c, d, a, b, x[11], S33, 0x6d9d6122);
    HH(b, c, d, a, x[14], S34, 0xfde5380c);
    HH(a, b, c, d, x[1], S31, 0xa4beea44);
    HH(d, a, b, c, x[4], S32, 0x4bdecfa9);
    HH(c, d, a, b, x[7], S33, 0xf6bb4b60);
    HH(b, c, d, a, x[10], S34, 0xbebfbc70);
    HH(a, b, c, d, x[13], S31, 0x289b7ec6);
    HH(d, a, b, c, x[0], S32, 0xeaa127fa);
    HH(c, d, a, b, x[3], S33, 0xd4ef3085);
    HH(b, c, d, a, x[6], S34, 0x4881d05);
    HH(a, b, c, d, x[9], S31, 0xd9d4d039);
    HH(d, a, b, c, x[12], S32, 0xe6db99e5);
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8);
    HH(b, c, d, a, x[2], S34, 0xc4ac5665);
    II(a, b, c, d, x[0], S41, 0xf4292244);
    II(d, a, b, c, x[7], S42, 0x432aff97);
    II(c, d, a, b, x[14], S43, 0xab9423a7);
    II(b, c, d, a, x[5], S44, 0xfc93a039);
    II(a, b, c, d, x[12], S41, 0x655b59c3);
    II(d, a, b, c, x[3], S42, 0x8f0ccc92);
    II(c, d, a, b, x[10], S43, 0xffeff47d);
    II(b, c, d, a, x[1], S44, 0x85845dd1);
    II(a, b, c, d, x[8], S41, 0x6fa87e4f);
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0);
    II(c, d, a, b, x[6], S43, 0xa3014314);
    II(b, c, d, a, x[13], S44, 0x4e0811a1);
    II(a, b, c, d, x[4], S41, 0xf7537e82);
    II(d, a, b, c, x[11], S42, 0xbd3af235);
    II(c, d, a, b, x[2], S43, 0x2ad7d2bb);
    II(b, c, d, a, x[9], S44, 0xeb86d391);
    fState[0] += a;
    fState[1] += b;
    fState[2] += c;
    fState[3] += d;
    for (unsigned k = 0; k < 16; ++k) {
        x[k] = 0;
    }
} // MD5Context::transform64Bytes
