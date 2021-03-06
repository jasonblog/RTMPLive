package org.webrtc.voiceengine;
import android.annotation.TargetApi;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.os.Build;
import org.webrtc.Logging;
public class WebRtcAudioManager
{
    private static final boolean DEBUG = false;
    private static final String TAG    = "WebRtcAudioManager";
    private static boolean blacklistDeviceForOpenSLESUsage = false;
    private static boolean blacklistDeviceForOpenSLESUsageIsOverridden = false;
    public static synchronized void setBlacklistDeviceForOpenSLESUsage(
        boolean enable)
    {
        blacklistDeviceForOpenSLESUsageIsOverridden = true;
        blacklistDeviceForOpenSLESUsage = enable;
    }

    private static final int BITS_PER_SAMPLE = 16;
    private static final int DEFAULT_FRAME_PER_BUFFER = 256;
    private static final int CHANNELS         = 1;
    private static final String[] AUDIO_MODES = new String[] {
        "MODE_NORMAL",
        "MODE_RINGTONE",
        "MODE_IN_CALL",
        "MODE_IN_COMMUNICATION",
    };
    private final long nativeAudioManager;
    private final Context context;
    private final AudioManager audioManager;
    private boolean initialized = false;
    private int nativeSampleRate;
    private int nativeChannels;
    private boolean hardwareAEC;
    private boolean hardwareAGC;
    private boolean hardwareNS;
    private boolean lowLatencyOutput;
    private boolean proAudio;
    private int sampleRate;
    private int channels;
    private int outputBufferSize;
    private int inputBufferSize;
    WebRtcAudioManager(Context context, long nativeAudioManager)
    {
        Logging.d(TAG, "ctor" + WebRtcAudioUtils.getThreadInfo());
        this.context = context;
        this.nativeAudioManager = nativeAudioManager;
        audioManager = (AudioManager) context.getSystemService(
            Context.AUDIO_SERVICE);

        if (DEBUG) {
            WebRtcAudioUtils.logDeviceInfo(TAG);
        }

        storeAudioParameters();
        nativeCacheAudioParameters(
            sampleRate, channels, hardwareAEC, hardwareAGC, hardwareNS,
            lowLatencyOutput, proAudio, outputBufferSize, inputBufferSize,
            nativeAudioManager);
    }

    private boolean init()
    {
        Logging.d(TAG, "init" + WebRtcAudioUtils.getThreadInfo());

        if (initialized) {
            return true;
        }

        Logging.d(TAG, "audio mode is: " + AUDIO_MODES[audioManager.getMode()]);
        initialized = true;
        return true;
    }

    private void dispose()
    {
        Logging.d(TAG, "dispose" + WebRtcAudioUtils.getThreadInfo());

        if (!initialized) {
            return;
        }
    }

    private boolean isCommunicationModeEnabled()
    {
        return (audioManager.getMode() == AudioManager.MODE_IN_COMMUNICATION);
    }

    private boolean isDeviceBlacklistedForOpenSLESUsage()
    {
        boolean blacklisted = blacklistDeviceForOpenSLESUsageIsOverridden ?
                              blacklistDeviceForOpenSLESUsage :
                              WebRtcAudioUtils.deviceIsBlacklistedForOpenSLESUsage();

        if (blacklisted) {
            Logging.e(TAG, Build.MODEL + " is blacklisted for OpenSL ES usage!");
        }

        return blacklisted;
    }

    private void storeAudioParameters()
    {
        channels         = CHANNELS;
        sampleRate       = getNativeOutputSampleRate();
        hardwareAEC      = isAcousticEchoCancelerSupported();
        hardwareAGC      = isAutomaticGainControlSupported();
        hardwareNS       = isNoiseSuppressorSupported();
        lowLatencyOutput = isLowLatencyOutputSupported();
        proAudio         = isProAudioSupported();
        outputBufferSize = lowLatencyOutput ?
                           getLowLatencyOutputFramesPerBuffer() :
                           getMinOutputFrameSize(sampleRate, channels);
        inputBufferSize = getMinInputFrameSize(sampleRate, channels);
    }

    private boolean hasEarpiece()
    {
        return context.getPackageManager().hasSystemFeature(
            PackageManager.FEATURE_TELEPHONY);
    }

    private boolean isLowLatencyOutputSupported()
    {
        return isOpenSLESSupported() &&
               context.getPackageManager().hasSystemFeature(
            PackageManager.FEATURE_AUDIO_LOW_LATENCY);
    }

    public boolean isLowLatencyInputSupported()
    {
        return WebRtcAudioUtils.runningOnLollipopOrHigher() &&
               isLowLatencyOutputSupported();
    }

    private boolean isProAudioSupported()
    {
        return WebRtcAudioUtils.runningOnMarshmallowOrHigher() &&
               context.getPackageManager().hasSystemFeature(
            PackageManager.FEATURE_AUDIO_PRO);
    }

    private int getNativeOutputSampleRate()
    {
        if (WebRtcAudioUtils.runningOnEmulator()) {
            Logging.d(TAG, "Running emulator, overriding sample rate to 8 kHz.");
            return 8000;
        }

        if (WebRtcAudioUtils.isDefaultSampleRateOverridden()) {
            Logging.d(TAG, "Default sample rate is overriden to "
                      +WebRtcAudioUtils.getDefaultSampleRateHz() + " Hz");
            return WebRtcAudioUtils.getDefaultSampleRateHz();
        }

        final int sampleRateHz;

        if (WebRtcAudioUtils.runningOnJellyBeanMR1OrHigher()) {
            sampleRateHz = getSampleRateOnJellyBeanMR10OrHigher();
        } else {
            sampleRateHz = WebRtcAudioUtils.getDefaultSampleRateHz();
        }

        Logging.d(TAG, "Sample rate is set to " + sampleRateHz + " Hz");
        return sampleRateHz;
    }

    @TargetApi(17)
    private int getSampleRateOnJellyBeanMR10OrHigher()
    {
        String sampleRateString = audioManager.getProperty(
            AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);

        return (sampleRateString == null) ?
               WebRtcAudioUtils.getDefaultSampleRateHz() :
               Integer.parseInt(sampleRateString);
    }

    @TargetApi(17)
    private int getLowLatencyOutputFramesPerBuffer()
    {
        assertTrue(isLowLatencyOutputSupported());

        if (!WebRtcAudioUtils.runningOnJellyBeanMR1OrHigher()) {
            return DEFAULT_FRAME_PER_BUFFER;
        }

        String framesPerBuffer = audioManager.getProperty(
            AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
        return framesPerBuffer == null ?
               DEFAULT_FRAME_PER_BUFFER : Integer.parseInt(framesPerBuffer);
    }

    private static boolean isAcousticEchoCancelerSupported()
    {
        return WebRtcAudioEffects.canUseAcousticEchoCanceler();
    }

    private static boolean isAutomaticGainControlSupported()
    {
        return WebRtcAudioEffects.canUseAutomaticGainControl();
    }

    private static boolean isNoiseSuppressorSupported()
    {
        return WebRtcAudioEffects.canUseNoiseSuppressor();
    }

    private static int getMinOutputFrameSize(int sampleRateInHz, int numChannels)
    {
        final int bytesPerFrame = numChannels * (BITS_PER_SAMPLE / 8);
        final int channelConfig;

        if (numChannels == 1) {
            channelConfig = AudioFormat.CHANNEL_OUT_MONO;
        } else if (numChannels == 2) {
            channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
        } else {
            return -1;
        }

        return AudioTrack.getMinBufferSize(
            sampleRateInHz, channelConfig, AudioFormat.ENCODING_PCM_16BIT)
               /bytesPerFrame;
    }

    private int getLowLatencyInputFramesPerBuffer()
    {
        assertTrue(isLowLatencyInputSupported());
        return getLowLatencyOutputFramesPerBuffer();
    }

    private static int getMinInputFrameSize(int sampleRateInHz, int numChannels)
    {
        final int bytesPerFrame = numChannels * (BITS_PER_SAMPLE / 8);

        assertTrue(numChannels == CHANNELS);
        return AudioRecord.getMinBufferSize(sampleRateInHz,
                                            AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT)
               /bytesPerFrame;
    }

    private static boolean isOpenSLESSupported()
    {
        return WebRtcAudioUtils.runningOnGingerBreadOrHigher();
    }

    private static void assertTrue(boolean condition)
    {
        if (!condition) {
            throw new AssertionError("Expected condition to be true");
        }
    }

    private native void nativeCacheAudioParameters(
        int sampleRate, int channels, boolean hardwareAEC, boolean hardwareAGC,
        boolean hardwareNS, boolean lowLatencyOutput, boolean proAudio,
        int outputBufferSize, int inputBufferSize, long nativeAudioManager);
}
