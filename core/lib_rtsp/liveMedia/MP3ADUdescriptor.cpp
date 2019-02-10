#include "MP3ADUdescriptor.hh"
#define TWO_BYTE_DESCR_FLAG 0x40
unsigned ADUdescriptor::generateDescriptor(unsigned char *& toPtr,
                                           unsigned       remainingFrameSize)
{
    unsigned descriptorSize = ADUdescriptor::computeSize(remainingFrameSize);

    switch (descriptorSize) {
        case 1: {
            *toPtr++ = (unsigned char) remainingFrameSize;
            break;
        }
        case 2: {
            generateTwoByteDescriptor(toPtr, remainingFrameSize);
            break;
        }
    }
    return descriptorSize;
}

void ADUdescriptor::generateTwoByteDescriptor(unsigned char *& toPtr,
                                              unsigned       remainingFrameSize)
{
    *toPtr++ = (TWO_BYTE_DESCR_FLAG | (unsigned char) (remainingFrameSize >> 8));
    *toPtr++ = (unsigned char) (remainingFrameSize & 0xFF);
}

unsigned ADUdescriptor::getRemainingFrameSize(unsigned char *& fromPtr)
{
    unsigned char firstByte = *fromPtr++;

    if (firstByte & TWO_BYTE_DESCR_FLAG) {
        unsigned char secondByte = *fromPtr++;
        return ((firstByte & 0x3F) << 8) | secondByte;
    } else {
        return (firstByte & 0x3F);
    }
}
