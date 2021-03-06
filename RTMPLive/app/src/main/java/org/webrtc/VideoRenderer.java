package org.webrtc;
import java.nio.ByteBuffer;
public class VideoRenderer
{
    public static class I420Frame
    {
        public final int width;
        public final int height;
        public final int[] yuvStrides;
        public ByteBuffer[] yuvPlanes;
        public final boolean yuvFrame;
        public final float[] samplingMatrix;
        public int textureId;
        private long nativeFramePointer;
        public int rotationDegree;
        I420Frame(int width, int height, int rotationDegree, int[] yuvStrides, ByteBuffer[] yuvPlanes,
                  long nativeFramePointer)
        {
            this.width              = width;
            this.height             = height;
            this.yuvStrides         = yuvStrides;
            this.yuvPlanes          = yuvPlanes;
            this.yuvFrame           = true;
            this.rotationDegree     = rotationDegree;
            this.nativeFramePointer = nativeFramePointer;

            if (rotationDegree % 90 != 0) {
                throw new IllegalArgumentException("Rotation degree not multiple of 90: " + rotationDegree);
            }

            samplingMatrix = new float[] {
                1, 0, 0, 0,
                0, -1, 0, 0,
                0, 0, 1, 0,
                0, 1, 0, 1
            };
        }

        I420Frame(int width, int height, int rotationDegree, int textureId, float[] samplingMatrix,
                  long nativeFramePointer)
        {
            this.width              = width;
            this.height             = height;
            this.yuvStrides         = null;
            this.yuvPlanes          = null;
            this.samplingMatrix     = samplingMatrix;
            this.textureId          = textureId;
            this.yuvFrame           = false;
            this.rotationDegree     = rotationDegree;
            this.nativeFramePointer = nativeFramePointer;

            if (rotationDegree % 90 != 0) {
                throw new IllegalArgumentException("Rotation degree not multiple of 90: " + rotationDegree);
            }
        }

        public int rotatedWidth()
        {
            return (rotationDegree % 180 == 0) ? width : height;
        }

        public int rotatedHeight()
        {
            return (rotationDegree % 180 == 0) ? height : width;
        }

        @Override
        public String toString()
        {
            return width + "x" + height + ":" + yuvStrides[0] + ":" + yuvStrides[1]
                   +":" + yuvStrides[2];
        }
    }
    public static native void nativeCopyPlane(ByteBuffer src, int width,
                                              int height, int srcStride, ByteBuffer dst, int dstStride);
    public static interface Callbacks
    {
        public void renderFrame(I420Frame frame);
    }
    public static void renderFrameDone(I420Frame frame)
    {
        frame.yuvPlanes = null;
        frame.textureId = 0;

        if (frame.nativeFramePointer != 0) {
            releaseNativeFrame(frame.nativeFramePointer);
            frame.nativeFramePointer = 0;
        }
    }

    long nativeVideoRenderer;
    public long GetRenderPointer()
    {
        return nativeVideoRenderer;
    }

    public VideoRenderer(Callbacks callbacks)
    {
        nativeVideoRenderer = nativeWrapVideoRenderer(callbacks);
    }

    public void dispose()
    {
        if (nativeVideoRenderer == 0) {
            return;
        }

        freeWrappedVideoRenderer(nativeVideoRenderer);
        nativeVideoRenderer = 0;
    }

    private static native long nativeWrapVideoRenderer(Callbacks callbacks);
    private static native void freeWrappedVideoRenderer(long nativeVideoRenderer);
    private static native void releaseNativeFrame(long nativeFramePointer);
}
