package org.webrtc;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecInfo.CodecCapabilities;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.os.Build;
import android.os.SystemClock;
import android.view.Surface;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
@SuppressWarnings("deprecation")
public class MediaCodecVideoDecoder
{
    private static final String TAG = "MediaCodecVideoDecoder";
    private static final long MAX_DECODE_TIME_MS = 200;
    public enum VideoCodecType {
        VIDEO_CODEC_VP8,
        VIDEO_CODEC_VP9,
        VIDEO_CODEC_H264
    }
    private static final int DEQUEUE_INPUT_TIMEOUT = 500000;
    private static final int MEDIA_CODEC_RELEASE_TIMEOUT_MS = 5000;
    private static final int MAX_QUEUED_OUTPUTBUFFERS       = 3;
    private static MediaCodecVideoDecoder runningInstance   = null;
    private static MediaCodecVideoDecoderErrorCallback errorCallback = null;
    private static int codecErrors = 0;
    private static Set<String> hwDecoderDisabledTypes = new HashSet<String>();
    private Thread mediaCodecThread;
    private MediaCodec mediaCodec;
    private ByteBuffer[] inputBuffers;
    private ByteBuffer[] outputBuffers;
    private static final String VP8_MIME_TYPE  = "video/x-vnd.on2.vp8";
    private static final String VP9_MIME_TYPE  = "video/x-vnd.on2.vp9";
    private static final String H264_MIME_TYPE = "video/avc";
    private static final String[] supportedVp8HwCodecPrefixes =
    { "OMX.qcom.", "OMX.Nvidia.", "OMX.Exynos.", "OMX.Intel." };
    private static final String[] supportedVp9HwCodecPrefixes =
    { "OMX.qcom.", "OMX.Exynos." };
    private static final String[] supportedH264HwCodecPrefixes =
    { "OMX.qcom.", "OMX.Intel.", "OMX.Exynos.", "OMX.rk." };
    private static final int
        COLOR_QCOM_FORMATYUV420PackedSemiPlanar32m        = 0x7FA30C04;
    private static final List<Integer> supportedColorList = Arrays.asList(
        CodecCapabilities.COLOR_FormatYUV420Planar,
        CodecCapabilities.COLOR_FormatYUV420SemiPlanar,
        CodecCapabilities.COLOR_QCOM_FormatYUV420SemiPlanar,
        COLOR_QCOM_FORMATYUV420PackedSemiPlanar32m);
    private int colorFormat;
    private int width;
    private int height;
    private int stride;
    private int sliceHeight;
    private boolean hasDecodedFirstFrame;
    private final Queue<TimeStamps> decodeStartTimeMs = new LinkedList<TimeStamps>();
    private boolean useSurface;
    private TextureListener textureListener;
    private int droppedFrames;
    private Surface surface = null;
    private final Queue<DecodedOutputBuffer>
    dequeuedSurfaceOutputBuffers = new LinkedList<DecodedOutputBuffer>();
    public static interface MediaCodecVideoDecoderErrorCallback
    {
        void onMediaCodecVideoDecoderCriticalError(int codecErrors);
    }
    public static void setErrorCallback(MediaCodecVideoDecoderErrorCallback errorCallback)
    {
        Logging.d(TAG, "Set error callback");
        MediaCodecVideoDecoder.errorCallback = errorCallback;
    }

    public static void disableVp8HwCodec()
    {
        Logging.w(TAG, "VP8 decoding is disabled by application.");
        hwDecoderDisabledTypes.add(VP8_MIME_TYPE);
    }

    public static void disableVp9HwCodec()
    {
        Logging.w(TAG, "VP9 decoding is disabled by application.");
        hwDecoderDisabledTypes.add(VP9_MIME_TYPE);
    }

    public static void disableH264HwCodec()
    {
        Logging.w(TAG, "H.264 decoding is disabled by application.");
        hwDecoderDisabledTypes.add(H264_MIME_TYPE);
    }

    public static boolean isVp8HwSupported()
    {
        return !hwDecoderDisabledTypes.contains(VP8_MIME_TYPE) &&
               (findDecoder(VP8_MIME_TYPE, supportedVp8HwCodecPrefixes) != null);
    }

    public static boolean isVp9HwSupported()
    {
        return !hwDecoderDisabledTypes.contains(VP9_MIME_TYPE) &&
               (findDecoder(VP9_MIME_TYPE, supportedVp9HwCodecPrefixes) != null);
    }

    public static boolean isH264HwSupported()
    {
        return !hwDecoderDisabledTypes.contains(H264_MIME_TYPE) &&
               (findDecoder(H264_MIME_TYPE, supportedH264HwCodecPrefixes) != null);
    }

    public static void printStackTrace()
    {
        if (runningInstance != null && runningInstance.mediaCodecThread != null) {
            StackTraceElement[] mediaCodecStackTraces = runningInstance.mediaCodecThread.getStackTrace();

            if (mediaCodecStackTraces.length > 0) {
                Logging.d(TAG, "MediaCodecVideoDecoder stacks trace:");

                for (StackTraceElement stackTrace : mediaCodecStackTraces) {
                    Logging.d(TAG, stackTrace.toString());
                }
            }
        }
    }

    private static class DecoderProperties
    {
        public DecoderProperties(String codecName, int colorFormat)
        {
            this.codecName   = codecName;
            this.colorFormat = colorFormat;
        }

        public final String codecName;
        public final int colorFormat;
    }
    private static DecoderProperties findDecoder(
        String mime, String[] supportedCodecPrefixes)
    {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            return null;
        }

        Logging.d(TAG, "Trying to find HW decoder for mime " + mime);

        for (int i = 0; i < MediaCodecList.getCodecCount(); ++i) {
            MediaCodecInfo info = MediaCodecList.getCodecInfoAt(i);

            if (info.isEncoder()) {
                continue;
            }

            String name = null;

            for (String mimeType : info.getSupportedTypes()) {
                if (mimeType.equals(mime)) {
                    name = info.getName();
                    break;
                }
            }

            if (name == null) {
                continue;
            }

            Logging.d(TAG, "Found candidate decoder " + name);
            boolean supportedCodec = false;

            for (String codecPrefix : supportedCodecPrefixes) {
                if (name.startsWith(codecPrefix)) {
                    supportedCodec = true;
                    break;
                }
            }

            if (!supportedCodec) {
                continue;
            }

            CodecCapabilities capabilities =
                info.getCapabilitiesForType(mime);

            for (int colorFormat : capabilities.colorFormats) {
                Logging.v(TAG, "   Color: 0x" + Integer.toHexString(colorFormat));
            }

            for (int supportedColorFormat : supportedColorList) {
                for (int codecColorFormat : capabilities.colorFormats) {
                    if (codecColorFormat == supportedColorFormat) {
                        Logging.d(TAG, "Found target decoder " + name
                                  +". Color: 0x" + Integer.toHexString(codecColorFormat));
                        return new DecoderProperties(name, codecColorFormat);
                    }
                }
            }
        }

        Logging.d(TAG, "No HW decoder found for mime " + mime);
        return null;
    } /* findDecoder */

    private void checkOnMediaCodecThread() throws IllegalStateException
    {
        if (mediaCodecThread.getId() != Thread.currentThread().getId()) {
            throw new IllegalStateException(
                      "MediaCodecVideoDecoder previously operated on " + mediaCodecThread
                      +" but is now called on " + Thread.currentThread());
        }
    }

    private boolean initDecode(
        VideoCodecType type, int width, int height,
        SurfaceTextureHelper surfaceTextureHelper)
    {
        if (mediaCodecThread != null) {
            throw new RuntimeException("initDecode: Forgot to release()?");
        }

        String mime = null;
        useSurface = (surfaceTextureHelper != null);
        String[] supportedCodecPrefixes = null;

        if (type == VideoCodecType.VIDEO_CODEC_VP8) {
            mime = VP8_MIME_TYPE;
            supportedCodecPrefixes = supportedVp8HwCodecPrefixes;
        } else if (type == VideoCodecType.VIDEO_CODEC_VP9) {
            mime = VP9_MIME_TYPE;
            supportedCodecPrefixes = supportedVp9HwCodecPrefixes;
        } else if (type == VideoCodecType.VIDEO_CODEC_H264) {
            mime = H264_MIME_TYPE;
            supportedCodecPrefixes = supportedH264HwCodecPrefixes;
        } else {
            throw new RuntimeException("initDecode: Non-supported codec " + type);
        }

        DecoderProperties properties = findDecoder(mime, supportedCodecPrefixes);

        if (properties == null) {
            throw new RuntimeException("Cannot find HW decoder for " + type);
        }

        Logging.d(TAG, "Java initDecode: " + type + " : " + width + " x " + height
                  +". Color: 0x" + Integer.toHexString(properties.colorFormat)
                  +". Use Surface: " + useSurface);
        runningInstance  = this;
        mediaCodecThread = Thread.currentThread();

        try {
            this.width  = width;
            this.height = height;
            stride      = width;
            sliceHeight = height;

            if (useSurface) {
                textureListener = new TextureListener(surfaceTextureHelper);
                surface         = new Surface(surfaceTextureHelper.getSurfaceTexture());
            }

            MediaFormat format = MediaFormat.createVideoFormat(mime, width, height);

            if (!useSurface) {
                format.setInteger(MediaFormat.KEY_COLOR_FORMAT, properties.colorFormat);
            }

            Logging.d(TAG, "  Format: " + format);
            mediaCodec = MediaCodecVideoEncoder.createByCodecName(properties.codecName);

            if (mediaCodec == null) {
                Logging.e(TAG, "Can not create media decoder");
                return false;
            }

            mediaCodec.configure(format, surface, null, 0);
            mediaCodec.start();
            colorFormat   = properties.colorFormat;
            outputBuffers = mediaCodec.getOutputBuffers();
            inputBuffers  = mediaCodec.getInputBuffers();
            decodeStartTimeMs.clear();
            hasDecodedFirstFrame = false;
            dequeuedSurfaceOutputBuffers.clear();
            droppedFrames = 0;
            Logging.d(TAG, "Input buffers: " + inputBuffers.length
                      +". Output buffers: " + outputBuffers.length);
            return true;
        } catch (IllegalStateException e) {
            Logging.e(TAG, "initDecode failed", e);
            return false;
        }
    } /* initDecode */

    private void reset(int width, int height)
    {
        if (mediaCodecThread == null || mediaCodec == null) {
            throw new RuntimeException("Incorrect reset call for non-initialized decoder.");
        }

        Logging.d(TAG, "Java reset: " + width + " x " + height);
        mediaCodec.flush();
        this.width  = width;
        this.height = height;
        decodeStartTimeMs.clear();
        dequeuedSurfaceOutputBuffers.clear();
        hasDecodedFirstFrame = false;
        droppedFrames        = 0;
    }

    private void release()
    {
        Logging.d(TAG, "Java releaseDecoder. Total number of dropped frames: " + droppedFrames);
        checkOnMediaCodecThread();
        final CountDownLatch releaseDone = new CountDownLatch(1);
        Runnable runMediaCodecRelease    = new Runnable() {
            @Override
            public void run()
            {
                try {
                    Logging.d(TAG, "Java releaseDecoder on release thread");
                    mediaCodec.stop();
                    mediaCodec.release();
                    Logging.d(TAG, "Java releaseDecoder on release thread done");
                } catch (Exception e) {
                    Logging.e(TAG, "Media decoder release failed", e);
                }

                releaseDone.countDown();
            }
        };
        new Thread(runMediaCodecRelease).start();

        if (!ThreadUtils.awaitUninterruptibly(releaseDone, MEDIA_CODEC_RELEASE_TIMEOUT_MS)) {
            Logging.e(TAG, "Media decoder release timeout");
            codecErrors++;

            if (errorCallback != null) {
                Logging.e(TAG, "Invoke codec error callback. Errors: " + codecErrors);
                errorCallback.onMediaCodecVideoDecoderCriticalError(codecErrors);
            }
        }

        mediaCodec       = null;
        mediaCodecThread = null;
        runningInstance  = null;

        if (useSurface) {
            surface.release();
            surface = null;
            textureListener.release();
        }

        Logging.d(TAG, "Java releaseDecoder done");
    } /* release */

    private int dequeueInputBuffer()
    {
        checkOnMediaCodecThread();

        try {
            return mediaCodec.dequeueInputBuffer(DEQUEUE_INPUT_TIMEOUT);
        } catch (IllegalStateException e) {
            Logging.e(TAG, "dequeueIntputBuffer failed", e);
            return -2;
        }
    }

    private boolean queueInputBuffer(int inputBufferIndex, int size, long presentationTimeStamUs,
                                     long timeStampMs, long ntpTimeStamp)
    {
        checkOnMediaCodecThread();

        try {
            inputBuffers[inputBufferIndex].position(0);
            inputBuffers[inputBufferIndex].limit(size);
            decodeStartTimeMs.add(new TimeStamps(SystemClock.elapsedRealtime(), timeStampMs,
                                                 ntpTimeStamp));
            mediaCodec.queueInputBuffer(inputBufferIndex, 0, size, presentationTimeStamUs, 0);
            return true;
        } catch (IllegalStateException e) {
            Logging.e(TAG, "decode failed", e);
            return false;
        }
    }

    private static class TimeStamps
    {
        public TimeStamps(long decodeStartTimeMs, long timeStampMs, long ntpTimeStampMs)
        {
            this.decodeStartTimeMs = decodeStartTimeMs;
            this.timeStampMs       = timeStampMs;
            this.ntpTimeStampMs    = ntpTimeStampMs;
        }

        private final long decodeStartTimeMs;
        private final long timeStampMs;
        private final long ntpTimeStampMs;
    }
    private static class DecodedOutputBuffer
    {
        public DecodedOutputBuffer(int index, int offset, int size, long presentationTimeStampMs,
                                   long timeStampMs, long ntpTimeStampMs, long decodeTime, long endDecodeTime)
        {
            this.index  = index;
            this.offset = offset;
            this.size   = size;
            this.presentationTimeStampMs = presentationTimeStampMs;
            this.timeStampMs     = timeStampMs;
            this.ntpTimeStampMs  = ntpTimeStampMs;
            this.decodeTimeMs    = decodeTime;
            this.endDecodeTimeMs = endDecodeTime;
        }

        private final int index;
        private final int offset;
        private final int size;
        private final long presentationTimeStampMs;
        private final long timeStampMs;
        private final long ntpTimeStampMs;
        private final long decodeTimeMs;
        private final long endDecodeTimeMs;
    }
    private static class DecodedTextureBuffer
    {
        private final int textureID;
        private final float[] transformMatrix;
        private final long presentationTimeStampMs;
        private final long timeStampMs;
        private final long ntpTimeStampMs;
        private final long decodeTimeMs;
        private final long frameDelayMs;
        public DecodedTextureBuffer(int textureID, float[] transformMatrix,
                                    long presentationTimeStampMs, long timeStampMs, long ntpTimeStampMs,
                                    long decodeTimeMs,
                                    long frameDelay)
        {
            this.textureID               = textureID;
            this.transformMatrix         = transformMatrix;
            this.presentationTimeStampMs = presentationTimeStampMs;
            this.timeStampMs             = timeStampMs;
            this.ntpTimeStampMs          = ntpTimeStampMs;
            this.decodeTimeMs            = decodeTimeMs;
            this.frameDelayMs            = frameDelay;
        }
    }
    private static class TextureListener
        implements SurfaceTextureHelper.OnTextureFrameAvailableListener
    {
        private final SurfaceTextureHelper surfaceTextureHelper;
        private final Object newFrameLock = new Object();
        private DecodedOutputBuffer bufferToRender;
        private DecodedTextureBuffer renderedBuffer;
        public TextureListener(SurfaceTextureHelper surfaceTextureHelper)
        {
            this.surfaceTextureHelper = surfaceTextureHelper;
            surfaceTextureHelper.startListening(this);
        }

        public void addBufferToRender(DecodedOutputBuffer buffer)
        {
            if (bufferToRender != null) {
                Logging.e(TAG,
                          "Unexpected addBufferToRender() called while waiting for a texture.");
                throw new IllegalStateException("Waiting for a texture.");
            }

            bufferToRender = buffer;
        }

        public boolean isWaitingForTexture()
        {
            synchronized (newFrameLock) {
                return bufferToRender != null;
            }
        }

        @Override
        public void onTextureFrameAvailable(
            int oesTextureId, float[] transformMatrix, long timestampNs)
        {
            synchronized (newFrameLock) {
                if (renderedBuffer != null) {
                    Logging.e(TAG,
                              "Unexpected onTextureFrameAvailable() called while already holding a texture.");
                    throw new IllegalStateException("Already holding a texture.");
                }

                renderedBuffer = new DecodedTextureBuffer(oesTextureId, transformMatrix,
                                                          bufferToRender.presentationTimeStampMs,
                                                          bufferToRender.timeStampMs,
                                                          bufferToRender.ntpTimeStampMs, bufferToRender.decodeTimeMs,
                                                          SystemClock.elapsedRealtime()
                                                          - bufferToRender.endDecodeTimeMs);
                bufferToRender = null;
                newFrameLock.notifyAll();
            }
        }

        public DecodedTextureBuffer dequeueTextureBuffer(int timeoutMs)
        {
            synchronized (newFrameLock) {
                if (renderedBuffer == null && timeoutMs > 0 && isWaitingForTexture()) {
                    try {
                        newFrameLock.wait(timeoutMs);
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }

                DecodedTextureBuffer returnedBuffer = renderedBuffer;
                renderedBuffer = null;
                return returnedBuffer;
            }
        }

        public void release()
        {
            surfaceTextureHelper.stopListening();
            synchronized (newFrameLock) {
                if (renderedBuffer != null) {
                    surfaceTextureHelper.returnTextureFrame();
                    renderedBuffer = null;
                }
            }
        }
    }
    private DecodedOutputBuffer dequeueOutputBuffer(int dequeueTimeoutMs)
    {
        checkOnMediaCodecThread();

        if (decodeStartTimeMs.isEmpty()) {
            return null;
        }

        final MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();

        while (true) {
            final int result = mediaCodec.dequeueOutputBuffer(
                info, TimeUnit.MILLISECONDS.toMicros(dequeueTimeoutMs));

            switch (result) {
                case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                    outputBuffers = mediaCodec.getOutputBuffers();
                    Logging.d(TAG, "Decoder output buffers changed: " + outputBuffers.length);

                    if (hasDecodedFirstFrame) {
                        throw new RuntimeException("Unexpected output buffer change event.");
                    }

                    break;

                case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                    MediaFormat format = mediaCodec.getOutputFormat();
                    Logging.d(TAG, "Decoder format changed: " + format.toString());
                    int new_width  = format.getInteger(MediaFormat.KEY_WIDTH);
                    int new_height = format.getInteger(MediaFormat.KEY_HEIGHT);

                    if (hasDecodedFirstFrame && (new_width != width || new_height != height)) {
                        throw new RuntimeException("Unexpected size change. Configured " + width + "*"
                                                   +height + ". New " + new_width + "*" + new_height);
                    }

                    width  = format.getInteger(MediaFormat.KEY_WIDTH);
                    height = format.getInteger(MediaFormat.KEY_HEIGHT);

                    if (!useSurface && format.containsKey(MediaFormat.KEY_COLOR_FORMAT)) {
                        colorFormat = format.getInteger(MediaFormat.KEY_COLOR_FORMAT);
                        Logging.d(TAG, "Color: 0x" + Integer.toHexString(colorFormat));

                        if (!supportedColorList.contains(colorFormat)) {
                            throw new IllegalStateException("Non supported color format: " + colorFormat);
                        }
                    }

                    if (format.containsKey("stride")) {
                        stride = format.getInteger("stride");
                    }

                    if (format.containsKey("slice-height")) {
                        sliceHeight = format.getInteger("slice-height");
                    }

                    Logging.d(TAG, "Frame stride and slice height: " + stride + " x " + sliceHeight);
                    stride      = Math.max(width, stride);
                    sliceHeight = Math.max(height, sliceHeight);
                    break;

                case MediaCodec.INFO_TRY_AGAIN_LATER:
                    return null;

                default:
                    hasDecodedFirstFrame = true;
                    TimeStamps timeStamps = decodeStartTimeMs.remove();
                    long decodeTimeMs     = SystemClock.elapsedRealtime() - timeStamps.decodeStartTimeMs;

                    if (decodeTimeMs > MAX_DECODE_TIME_MS) {
                        decodeTimeMs = MAX_DECODE_TIME_MS;
                    }

                    return new DecodedOutputBuffer(result,
                                                   info.offset,
                                                   info.size,
                                                   TimeUnit.MICROSECONDS.toMillis(info.presentationTimeUs),
                                                   timeStamps.timeStampMs,
                                                   timeStamps.ntpTimeStampMs,
                                                   decodeTimeMs,
                                                   SystemClock.elapsedRealtime());
            }
        }
    } /* dequeueOutputBuffer */

    private DecodedTextureBuffer dequeueTextureBuffer(int dequeueTimeoutMs)
    {
        checkOnMediaCodecThread();

        if (!useSurface) {
            throw new IllegalStateException("dequeueTexture() called for byte buffer decoding.");
        }

        DecodedOutputBuffer outputBuffer = dequeueOutputBuffer(dequeueTimeoutMs);

        if (outputBuffer != null) {
            dequeuedSurfaceOutputBuffers.add(outputBuffer);
        }

        MaybeRenderDecodedTextureBuffer();
        DecodedTextureBuffer renderedBuffer = textureListener.dequeueTextureBuffer(dequeueTimeoutMs);

        if (renderedBuffer != null) {
            MaybeRenderDecodedTextureBuffer();
            return renderedBuffer;
        }

        if ((dequeuedSurfaceOutputBuffers.size()
             >= Math.min(MAX_QUEUED_OUTPUTBUFFERS, outputBuffers.length) ||
             (dequeueTimeoutMs > 0 && !dequeuedSurfaceOutputBuffers.isEmpty())))
        {
            ++droppedFrames;
            final DecodedOutputBuffer droppedFrame = dequeuedSurfaceOutputBuffers.remove();

            if (dequeueTimeoutMs > 0) {
                Logging.w(TAG, "Draining decoder. Dropping frame with TS: "
                          + droppedFrame.presentationTimeStampMs
                          +". Total number of dropped frames: " + droppedFrames);
            } else {
                Logging.w(TAG, "Too many output buffers " + dequeuedSurfaceOutputBuffers.size()
                          +". Dropping frame with TS: " + droppedFrame.presentationTimeStampMs
                          +". Total number of dropped frames: " + droppedFrames);
            }

            mediaCodec.releaseOutputBuffer(droppedFrame.index, false);
            return new DecodedTextureBuffer(0, null,
                                            droppedFrame.presentationTimeStampMs, droppedFrame.timeStampMs,
                                            droppedFrame.ntpTimeStampMs, droppedFrame.decodeTimeMs,
                                            SystemClock.elapsedRealtime() - droppedFrame.endDecodeTimeMs);
        }

        return null;
    } /* dequeueTextureBuffer */

    private void MaybeRenderDecodedTextureBuffer()
    {
        if (dequeuedSurfaceOutputBuffers.isEmpty() || textureListener.isWaitingForTexture()) {
            return;
        }

        final DecodedOutputBuffer buffer = dequeuedSurfaceOutputBuffers.remove();
        textureListener.addBufferToRender(buffer);
        mediaCodec.releaseOutputBuffer(buffer.index, true);
    }

    private void returnDecodedOutputBuffer(int index)
    throws IllegalStateException, MediaCodec.CodecException
    {
        checkOnMediaCodecThread();

        if (useSurface) {
            throw new IllegalStateException("returnDecodedOutputBuffer() called for surface decoding.");
        }

        mediaCodec.releaseOutputBuffer(index, false);
    }
}
