package org.webrtc;
import android.content.Context;
import java.util.List;
public interface VideoCapturer
{
    public interface CapturerObserver
    {
        void onCapturerStarted(boolean success);
        void onByteBufferFrameCaptured(byte[] data, int width, int height, int rotation,
                                       long timeStamp);
        void onTextureFrameCaptured(
            int width, int height, int oesTextureId, float[] transformMatrix, int rotation,
            long timestamp);
        void onOutputFormatRequest(int width, int height, int framerate);
    }
    static class NativeObserver implements CapturerObserver
    {
        private final long nativeCapturer;
        public NativeObserver(long nativeCapturer)
        {
            this.nativeCapturer = nativeCapturer;
        }

        @Override
        public void onCapturerStarted(boolean success)
        {
            nativeCapturerStarted(nativeCapturer, success);
        }

        @Override
        public void onByteBufferFrameCaptured(byte[] data, int width, int height,
                                              int rotation, long timeStamp)
        {
            nativeOnByteBufferFrameCaptured(nativeCapturer, data, data.length, width, height, rotation,
                                            timeStamp);
        }

        @Override
        public void onTextureFrameCaptured(
            int width, int height, int oesTextureId, float[] transformMatrix, int rotation,
            long timestamp)
        {
            nativeOnTextureFrameCaptured(nativeCapturer, width, height, oesTextureId, transformMatrix,
                                         rotation, timestamp);
        }

        @Override
        public void onOutputFormatRequest(int width, int height, int framerate)
        {
            nativeOnOutputFormatRequest(nativeCapturer, width, height, framerate);
        }

        private native void nativeCapturerStarted(long    nativeCapturer,
                                                  boolean success);
        private native void nativeOnByteBufferFrameCaptured(long nativeCapturer,
                                                            byte[] data, int length, int width, int height,
                                                            int rotation, long timeStamp);
        private native void nativeOnTextureFrameCaptured(long nativeCapturer, int width, int height,
                                                         int oesTextureId, float[] transformMatrix, int rotation,
                                                         long timestamp);
        private native void nativeOnOutputFormatRequest(long nativeCapturer,
                                                        int width, int height, int framerate);
    }
    List<CameraEnumerationAndroid.CaptureFormat> getSupportedFormats();
    void startCapture(
        int width, int height, int framerate, SurfaceTextureHelper surfaceTextureHelper,
        Context applicationContext, CapturerObserver frameObserver);
    void stopCapture() throws InterruptedException;
    void onOutputFormatRequest(int width, int height, int framerate);
    void changeCaptureFormat(int width, int height, int framerate);
    void dispose();
}
