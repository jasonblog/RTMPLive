package org.webrtc;
import android.annotation.TargetApi;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecInfo.CodecCapabilities;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.opengl.GLES20;
import android.os.Build;
import android.os.Bundle;
import android.view.Surface;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
@TargetApi(19)
@SuppressWarnings("deprecation")
public class MediaCodecVideoEncoder
{
    private static final String TAG = "MediaCodecVideoEncoder";
    public enum VideoCodecType {
        VIDEO_CODEC_VP8,
        VIDEO_CODEC_VP9,
        VIDEO_CODEC_H264
    }
    private static final int MEDIA_CODEC_RELEASE_TIMEOUT_MS = 5000;
    private static final int DEQUEUE_TIMEOUT              = 0;
    private static final int BITRATE_ADJUSTMENT_FPS       = 30;
    private static MediaCodecVideoEncoder runningInstance = null;
    private static MediaCodecVideoEncoderErrorCallback errorCallback = null;
    private static int codecErrors = 0;
    private static Set<String> hwEncoderDisabledTypes = new HashSet<String>();
    private Thread mediaCodecThread;
    private MediaCodec mediaCodec;
    private ByteBuffer[] outputBuffers;
    private EglBase14 eglBase;
    private int width;
    private int height;
    private Surface inputSurface;
    private GlRectDrawer drawer;
    private static final String VP8_MIME_TYPE  = "video/x-vnd.on2.vp8";
    private static final String VP9_MIME_TYPE  = "video/x-vnd.on2.vp9";
    private static final String H264_MIME_TYPE = "video/avc";
    private static class MediaCodecProperties
    {
        public final String codecPrefix;
        public final int minSdk;
        public final boolean bitrateAdjustmentRequired;
        MediaCodecProperties(
            String codecPrefix, int minSdk, boolean bitrateAdjustmentRequired)
        {
            this.codecPrefix = codecPrefix;
            this.minSdk      = minSdk;
            this.bitrateAdjustmentRequired = bitrateAdjustmentRequired;
        }
    }
    private static final MediaCodecProperties qcomVp8HwProperties = new MediaCodecProperties(
        "OMX.qcom.", Build.VERSION_CODES.KITKAT, false);
    private static final MediaCodecProperties exynosVp8HwProperties = new MediaCodecProperties(
        "OMX.Exynos.", Build.VERSION_CODES.M, false);
    private static final MediaCodecProperties intelVp8HwProperties = new MediaCodecProperties(
        "OMX.Intel.", Build.VERSION_CODES.LOLLIPOP, false);
    private static final MediaCodecProperties[] vp8HwList = new MediaCodecProperties[] {
        qcomVp8HwProperties, exynosVp8HwProperties, intelVp8HwProperties
    };
    private static final MediaCodecProperties qcomVp9HwProperties = new MediaCodecProperties(
        "OMX.qcom.", Build.VERSION_CODES.M, false);
    private static final MediaCodecProperties exynosVp9HwProperties = new MediaCodecProperties(
        "OMX.Exynos.", Build.VERSION_CODES.M, false);
    private static final MediaCodecProperties[] vp9HwList = new MediaCodecProperties[] {
        qcomVp9HwProperties, exynosVp9HwProperties
    };
    private static final MediaCodecProperties qcomH264HwProperties = new MediaCodecProperties(
        "OMX.qcom.", Build.VERSION_CODES.KITKAT, false);
    private static final MediaCodecProperties exynosH264HwProperties = new MediaCodecProperties(
        "OMX.Exynos.", Build.VERSION_CODES.LOLLIPOP, true);
    private static final MediaCodecProperties rkH264HwProperties = new MediaCodecProperties(
        "OMX.rk.", Build.VERSION_CODES.KITKAT, false);
    private static final MediaCodecProperties[] h264HwList = new MediaCodecProperties[] {
        qcomH264HwProperties, exynosH264HwProperties, rkH264HwProperties
    };
    private static final String[] H264_HW_EXCEPTION_MODELS = new String[] {
        "SAMSUNG-SGH-I337",
        "Nexus 7",
        "Nexus 4"
    };
    private static final int VIDEO_ControlRateConstant = 2;
    private static final int
        COLOR_QCOM_FORMATYUV420PackedSemiPlanar32m = 0x7FA30C04;
    private static final int[] supportedColorList  = {
        CodecCapabilities.COLOR_FormatYUV420Planar,
        CodecCapabilities.COLOR_FormatYUV420SemiPlanar,
        CodecCapabilities.COLOR_QCOM_FormatYUV420SemiPlanar,
        COLOR_QCOM_FORMATYUV420PackedSemiPlanar32m
    };
    private static final int[] supportedSurfaceColorList = {
        CodecCapabilities.COLOR_FormatSurface
    };
    private VideoCodecType type;
    private int colorFormat;
    private boolean bitrateAdjustmentRequired;
    private ByteBuffer configData = null;
    public static interface MediaCodecVideoEncoderErrorCallback
    {
        void onMediaCodecVideoEncoderCriticalError(int codecErrors);
    }
    public static void setErrorCallback(MediaCodecVideoEncoderErrorCallback errorCallback)
    {
        Logging.d(TAG, "Set error callback");
        MediaCodecVideoEncoder.errorCallback = errorCallback;
    }

    public static void disableVp8HwCodec()
    {
        Logging.w(TAG, "VP8 encoding is disabled by application.");
        hwEncoderDisabledTypes.add(VP8_MIME_TYPE);
    }

    public static void disableVp9HwCodec()
    {
        Logging.w(TAG, "VP9 encoding is disabled by application.");
        hwEncoderDisabledTypes.add(VP9_MIME_TYPE);
    }

    public static void disableH264HwCodec()
    {
        Logging.w(TAG, "H.264 encoding is disabled by application.");
        hwEncoderDisabledTypes.add(H264_MIME_TYPE);
    }

    public static boolean isVp8HwSupported()
    {
        return !hwEncoderDisabledTypes.contains(VP8_MIME_TYPE) &&
               (findHwEncoder(VP8_MIME_TYPE, vp8HwList, supportedColorList) != null);
    }

    public static boolean isVp9HwSupported()
    {
        return !hwEncoderDisabledTypes.contains(VP9_MIME_TYPE) &&
               (findHwEncoder(VP9_MIME_TYPE, vp9HwList, supportedColorList) != null);
    }

    public static boolean isH264HwSupported()
    {
        return !hwEncoderDisabledTypes.contains(H264_MIME_TYPE) &&
               (findHwEncoder(H264_MIME_TYPE, h264HwList, supportedColorList) != null);
    }

    public static boolean isVp8HwSupportedUsingTextures()
    {
        return !hwEncoderDisabledTypes.contains(VP8_MIME_TYPE) &&
               (findHwEncoder(VP8_MIME_TYPE, vp8HwList, supportedSurfaceColorList) != null);
    }

    public static boolean isVp9HwSupportedUsingTextures()
    {
        return !hwEncoderDisabledTypes.contains(VP9_MIME_TYPE) &&
               (findHwEncoder(VP9_MIME_TYPE, vp9HwList, supportedSurfaceColorList) != null);
    }

    public static boolean isH264HwSupportedUsingTextures()
    {
        return !hwEncoderDisabledTypes.contains(H264_MIME_TYPE) &&
               (findHwEncoder(H264_MIME_TYPE, h264HwList, supportedSurfaceColorList) != null);
    }

    private static class EncoderProperties
    {
        public EncoderProperties(String codecName, int colorFormat, boolean bitrateAdjustment)
        {
            this.codecName         = codecName;
            this.colorFormat       = colorFormat;
            this.bitrateAdjustment = bitrateAdjustment;
        }

        public final String codecName;
        public final int colorFormat;
        public final boolean bitrateAdjustment;
    }
    private static EncoderProperties findHwEncoder(
        String mime, MediaCodecProperties[] supportedHwCodecProperties, int[] colorList)
    {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            return null;
        }

        if (mime.equals(H264_MIME_TYPE)) {
            List<String> exceptionModels = Arrays.asList(H264_HW_EXCEPTION_MODELS);

            if (exceptionModels.contains(Build.MODEL)) {
                Logging.w(TAG, "Model: " + Build.MODEL + " has black listed H.264 encoder.");
                return null;
            }
        }

        for (int i = 0; i < MediaCodecList.getCodecCount(); ++i) {
            MediaCodecInfo info = MediaCodecList.getCodecInfoAt(i);

            if (!info.isEncoder()) {
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

            Logging.v(TAG, "Found candidate encoder " + name);
            boolean supportedCodec = false;
            boolean bitrateAdjustmentRequired = false;

            for (MediaCodecProperties codecProperties : supportedHwCodecProperties) {
                if (name.startsWith(codecProperties.codecPrefix)) {
                    if (Build.VERSION.SDK_INT < codecProperties.minSdk) {
                        Logging.w(TAG, "Codec " + name + " is disabled due to SDK version "
                                  +Build.VERSION.SDK_INT);
                        continue;
                    }

                    if (codecProperties.bitrateAdjustmentRequired) {
                        Logging.w(TAG, "Codec " + name + " does not use frame timestamps.");
                        bitrateAdjustmentRequired = true;
                    }

                    supportedCodec = true;
                    break;
                }
            }

            if (!supportedCodec) {
                continue;
            }

            CodecCapabilities capabilities = info.getCapabilitiesForType(mime);

            for (int colorFormat : capabilities.colorFormats) {
                Logging.v(TAG, "   Color: 0x" + Integer.toHexString(colorFormat));
            }

            for (int supportedColorFormat : colorList) {
                for (int codecColorFormat : capabilities.colorFormats) {
                    if (codecColorFormat == supportedColorFormat) {
                        Logging.d(TAG, "Found target encoder for mime " + mime + " : " + name
                                  +". Color: 0x" + Integer.toHexString(codecColorFormat));
                        return new EncoderProperties(name, codecColorFormat, bitrateAdjustmentRequired);
                    }
                }
            }
        }

        return null;
    } /* findHwEncoder */

    private void checkOnMediaCodecThread()
    {
        if (mediaCodecThread.getId() != Thread.currentThread().getId()) {
            throw new RuntimeException(
                      "MediaCodecVideoEncoder previously operated on " + mediaCodecThread
                      +" but is now called on " + Thread.currentThread());
        }
    }

    public static void printStackTrace()
    {
        if (runningInstance != null && runningInstance.mediaCodecThread != null) {
            StackTraceElement[] mediaCodecStackTraces = runningInstance.mediaCodecThread.getStackTrace();

            if (mediaCodecStackTraces.length > 0) {
                Logging.d(TAG, "MediaCodecVideoEncoder stacks trace:");

                for (StackTraceElement stackTrace : mediaCodecStackTraces) {
                    Logging.d(TAG, stackTrace.toString());
                }
            }
        }
    }

    static MediaCodec createByCodecName(String codecName)
    {
        try {
            return MediaCodec.createByCodecName(codecName);
        } catch (Exception e) {
            return null;
        }
    }

    boolean initEncode(VideoCodecType type, int width, int height, int kbps, int fps,
                       EglBase14.Context sharedContext)
    {
        final boolean useSurface = sharedContext != null;

        Logging.d(TAG, "Java initEncode: " + type + " : " + width + " x " + height
                  +". @ " + kbps + " kbps. Fps: " + fps + ". Encode from texture : " + useSurface);
        this.width  = width;
        this.height = height;

        if (mediaCodecThread != null) {
            throw new RuntimeException("Forgot to release()?");
        }

        EncoderProperties properties = null;
        String mime = null;
        int keyFrameIntervalSec = 0;

        if (type == VideoCodecType.VIDEO_CODEC_VP8) {
            mime       = VP8_MIME_TYPE;
            properties = findHwEncoder(
                VP8_MIME_TYPE, vp8HwList, useSurface ? supportedSurfaceColorList : supportedColorList);
            keyFrameIntervalSec = 100;
        } else if (type == VideoCodecType.VIDEO_CODEC_VP9) {
            mime       = VP9_MIME_TYPE;
            properties = findHwEncoder(
                VP9_MIME_TYPE, vp9HwList, useSurface ? supportedSurfaceColorList : supportedColorList);
            keyFrameIntervalSec = 100;
        } else if (type == VideoCodecType.VIDEO_CODEC_H264) {
            mime       = H264_MIME_TYPE;
            properties = findHwEncoder(
                H264_MIME_TYPE, h264HwList, useSurface ? supportedSurfaceColorList : supportedColorList);
            keyFrameIntervalSec = 20;
        }

        if (properties == null) {
            throw new RuntimeException("Can not find HW encoder for " + type);
        }

        runningInstance = this;
        colorFormat     = properties.colorFormat;
        bitrateAdjustmentRequired = properties.bitrateAdjustment;

        if (bitrateAdjustmentRequired) {
            fps = BITRATE_ADJUSTMENT_FPS;
        }

        Logging.d(TAG, "Color format: " + colorFormat
                  +". Bitrate adjustment: " + bitrateAdjustmentRequired);
        mediaCodecThread = Thread.currentThread();

        try {
            MediaFormat format = MediaFormat.createVideoFormat(mime, width, height);
            format.setInteger(MediaFormat.KEY_BIT_RATE, 1000 * kbps);
            format.setInteger("bitrate-mode", VIDEO_ControlRateConstant);
            format.setInteger(MediaFormat.KEY_COLOR_FORMAT, properties.colorFormat);
            format.setInteger(MediaFormat.KEY_FRAME_RATE, fps);
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, keyFrameIntervalSec);
            Logging.d(TAG, "  Format: " + format);
            mediaCodec = createByCodecName(properties.codecName);
            this.type  = type;

            if (mediaCodec == null) {
                Logging.e(TAG, "Can not create media encoder");
                return false;
            }

            mediaCodec.configure(
                format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

            if (useSurface) {
                eglBase      = new EglBase14(sharedContext, EglBase.CONFIG_RECORDABLE);
                inputSurface = mediaCodec.createInputSurface();
                eglBase.createSurface(inputSurface);
                drawer = new GlRectDrawer();
            }

            mediaCodec.start();
            outputBuffers = mediaCodec.getOutputBuffers();
            Logging.d(TAG, "Output buffers: " + outputBuffers.length);
        } catch (IllegalStateException e) {
            Logging.e(TAG, "initEncode failed", e);
            return false;
        }

        return true;
    } /* initEncode */

    ByteBuffer[]  getInputBuffers()
    {
        ByteBuffer[] inputBuffers = mediaCodec.getInputBuffers();
        Logging.d(TAG, "Input buffers: " + inputBuffers.length);
        return inputBuffers;
    }

    boolean encodeBuffer(
        boolean isKeyframe, int inputBuffer, int size,
        long presentationTimestampUs)
    {
        checkOnMediaCodecThread();

        try {
            if (isKeyframe) {
                Logging.d(TAG, "Sync frame request");
                Bundle b = new Bundle();
                b.putInt(MediaCodec.PARAMETER_KEY_REQUEST_SYNC_FRAME, 0);
                mediaCodec.setParameters(b);
            }

            mediaCodec.queueInputBuffer(
                inputBuffer, 0, size, presentationTimestampUs, 0);
            return true;
        } catch (IllegalStateException e) {
            Logging.e(TAG, "encodeBuffer failed", e);
            return false;
        }
    }

    boolean encodeTexture(boolean isKeyframe, int oesTextureId, float[] transformationMatrix,
                          long presentationTimestampUs)
    {
        checkOnMediaCodecThread();

        try {
            if (isKeyframe) {
                Logging.d(TAG, "Sync frame request");
                Bundle b = new Bundle();
                b.putInt(MediaCodec.PARAMETER_KEY_REQUEST_SYNC_FRAME, 0);
                mediaCodec.setParameters(b);
            }

            eglBase.makeCurrent();
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
            drawer.drawOes(oesTextureId, transformationMatrix, width, height, 0, 0, width, height);
            eglBase.swapBuffers(TimeUnit.MICROSECONDS.toNanos(presentationTimestampUs));
            return true;
        } catch (RuntimeException e) {
            Logging.e(TAG, "encodeTexture failed", e);
            return false;
        }
    }

    void release()
    {
        Logging.d(TAG, "Java releaseEncoder");
        checkOnMediaCodecThread();
        final CountDownLatch releaseDone = new CountDownLatch(1);
        Runnable runMediaCodecRelease    = new Runnable() {
            @Override
            public void run()
            {
                try {
                    Logging.d(TAG, "Java releaseEncoder on release thread");
                    mediaCodec.stop();
                    mediaCodec.release();
                    Logging.d(TAG, "Java releaseEncoder on release thread done");
                } catch (Exception e) {
                    Logging.e(TAG, "Media encoder release failed", e);
                }

                releaseDone.countDown();
            }
        };
        new Thread(runMediaCodecRelease).start();

        if (!ThreadUtils.awaitUninterruptibly(releaseDone, MEDIA_CODEC_RELEASE_TIMEOUT_MS)) {
            Logging.e(TAG, "Media encoder release timeout");
            codecErrors++;

            if (errorCallback != null) {
                Logging.e(TAG, "Invoke codec error callback. Errors: " + codecErrors);
                errorCallback.onMediaCodecVideoEncoderCriticalError(codecErrors);
            }
        }

        mediaCodec       = null;
        mediaCodecThread = null;

        if (drawer != null) {
            drawer.release();
            drawer = null;
        }

        if (eglBase != null) {
            eglBase.release();
            eglBase = null;
        }

        if (inputSurface != null) {
            inputSurface.release();
            inputSurface = null;
        }

        runningInstance = null;
        Logging.d(TAG, "Java releaseEncoder done");
    } /* release */

    private boolean setRates(int kbps, int frameRate)
    {
        checkOnMediaCodecThread();
        int codecBitrate = 1000 * kbps;

        if (bitrateAdjustmentRequired && frameRate > 0) {
            codecBitrate = BITRATE_ADJUSTMENT_FPS * codecBitrate / frameRate;
            Logging.v(TAG, "setRates: " + kbps + " -> " + (codecBitrate / 1000)
                      + " kbps. Fps: " + frameRate);
        } else {
            Logging.v(TAG, "setRates: " + kbps);
        }

        try {
            Bundle params = new Bundle();
            params.putInt(MediaCodec.PARAMETER_KEY_VIDEO_BITRATE, codecBitrate);
            mediaCodec.setParameters(params);
            return true;
        } catch (IllegalStateException e) {
            Logging.e(TAG, "setRates failed", e);
            return false;
        }
    }

    int dequeueInputBuffer()
    {
        checkOnMediaCodecThread();

        try {
            return mediaCodec.dequeueInputBuffer(DEQUEUE_TIMEOUT);
        } catch (IllegalStateException e) {
            Logging.e(TAG, "dequeueIntputBuffer failed", e);
            return -2;
        }
    }

    static class OutputBufferInfo
    {
        public OutputBufferInfo(
            int index, ByteBuffer buffer,
            boolean isKeyFrame, long presentationTimestampUs)
        {
            this.index      = index;
            this.buffer     = buffer;
            this.isKeyFrame = isKeyFrame;
            this.presentationTimestampUs = presentationTimestampUs;
        }

        public final int index;
        public final ByteBuffer buffer;
        public final boolean isKeyFrame;
        public final long presentationTimestampUs;
    }
    OutputBufferInfo dequeueOutputBuffer()
    {
        checkOnMediaCodecThread();

        try {
            MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
            int result = mediaCodec.dequeueOutputBuffer(info, DEQUEUE_TIMEOUT);

            if (result >= 0) {
                boolean isConfigFrame =
                    (info.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0;

                if (isConfigFrame) {
                    Logging.d(TAG, "Config frame generated. Offset: " + info.offset
                              +". Size: " + info.size);
                    configData = ByteBuffer.allocateDirect(info.size);
                    outputBuffers[result].position(info.offset);
                    outputBuffers[result].limit(info.offset + info.size);
                    configData.put(outputBuffers[result]);
                    mediaCodec.releaseOutputBuffer(result, false);
                    result = mediaCodec.dequeueOutputBuffer(info, DEQUEUE_TIMEOUT);
                }
            }

            if (result >= 0) {
                ByteBuffer outputBuffer = outputBuffers[result].duplicate();
                outputBuffer.position(info.offset);
                outputBuffer.limit(info.offset + info.size);
                boolean isKeyFrame =
                    (info.flags & MediaCodec.BUFFER_FLAG_SYNC_FRAME) != 0;

                if (isKeyFrame) {
                    Logging.d(TAG, "Sync frame generated");
                }

                if (isKeyFrame && type == VideoCodecType.VIDEO_CODEC_H264) {
                    Logging.d(TAG, "Appending config frame of size " + configData.capacity()
                              +" to output buffer with offset " + info.offset + ", size "
                              +info.size);
                    ByteBuffer keyFrameBuffer = ByteBuffer.allocateDirect(
                        configData.capacity() + info.size);
                    configData.rewind();
                    keyFrameBuffer.put(configData);
                    keyFrameBuffer.put(outputBuffer);
                    keyFrameBuffer.position(0);
                    return new OutputBufferInfo(result, keyFrameBuffer,
                                                isKeyFrame, info.presentationTimeUs);
                } else {
                    return new OutputBufferInfo(result, outputBuffer.slice(),
                                                isKeyFrame, info.presentationTimeUs);
                }
            } else if (result == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                outputBuffers = mediaCodec.getOutputBuffers();
                return dequeueOutputBuffer();
            } else if (result == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                return dequeueOutputBuffer();
            } else if (result == MediaCodec.INFO_TRY_AGAIN_LATER) {
                return null;
            }

            throw new RuntimeException("dequeueOutputBuffer: " + result);
        } catch (IllegalStateException e) {
            Logging.e(TAG, "dequeueOutputBuffer failed", e);
            return new OutputBufferInfo(-1, null, false, -1);
        }
    } /* dequeueOutputBuffer */

    boolean releaseOutputBuffer(int index)
    {
        checkOnMediaCodecThread();

        try {
            mediaCodec.releaseOutputBuffer(index, false);
            return true;
        } catch (IllegalStateException e) {
            Logging.e(TAG, "releaseOutputBuffer failed", e);
            return false;
        }
    }
}
