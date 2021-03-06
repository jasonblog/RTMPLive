package org.webrtc.voiceengine;
import android.annotation.TargetApi;
import android.media.audiofx.AcousticEchoCanceler;
import android.media.audiofx.AudioEffect;
import android.media.audiofx.AudioEffect.Descriptor;
import android.media.audiofx.AutomaticGainControl;
import android.media.audiofx.NoiseSuppressor;
import android.os.Build;
import org.webrtc.Logging;
import java.util.List;
import java.util.UUID;
class WebRtcAudioEffects
{
    private static final boolean DEBUG = false;
    private static final String TAG    = "WebRtcAudioEffects";
    private static final UUID AOSP_ACOUSTIC_ECHO_CANCELER =
        UUID.fromString("bb392ec0-8d4d-11e0-a896-0002a5d5c51b");
    private static final UUID AOSP_AUTOMATIC_GAIN_CONTROL =
        UUID.fromString("aa8130e0-66fc-11e0-bad0-0002a5d5c51b");
    private static final UUID AOSP_NOISE_SUPPRESSOR =
        UUID.fromString("c06c8400-8e06-11e0-9cb6-0002a5d5c51b");
    private static Descriptor[] cachedEffects = null;
    private AcousticEchoCanceler aec = null;
    private AutomaticGainControl agc = null;
    private NoiseSuppressor ns       = null;
    private boolean shouldEnableAec  = false;
    private boolean shouldEnableAgc  = false;
    private boolean shouldEnableNs   = false;
    public static boolean isAcousticEchoCancelerSupported()
    {
        return WebRtcAudioUtils.runningOnJellyBeanOrHigher() &&
               isAcousticEchoCancelerEffectAvailable();
    }

    public static boolean isAutomaticGainControlSupported()
    {
        return WebRtcAudioUtils.runningOnJellyBeanOrHigher() &&
               isAutomaticGainControlEffectAvailable();
    }

    public static boolean isNoiseSuppressorSupported()
    {
        return WebRtcAudioUtils.runningOnJellyBeanOrHigher() &&
               isNoiseSuppressorEffectAvailable();
    }

    public static boolean isAcousticEchoCancelerBlacklisted()
    {
        List<String> blackListedModels =
            WebRtcAudioUtils.getBlackListedModelsForAecUsage();
        boolean isBlacklisted = blackListedModels.contains(Build.MODEL);

        if (isBlacklisted) {
            Logging.w(TAG, Build.MODEL + " is blacklisted for HW AEC usage!");
        }

        return isBlacklisted;
    }

    public static boolean isAutomaticGainControlBlacklisted()
    {
        List<String> blackListedModels =
            WebRtcAudioUtils.getBlackListedModelsForAgcUsage();
        boolean isBlacklisted = blackListedModels.contains(Build.MODEL);

        if (isBlacklisted) {
            Logging.w(TAG, Build.MODEL + " is blacklisted for HW AGC usage!");
        }

        return isBlacklisted;
    }

    public static boolean isNoiseSuppressorBlacklisted()
    {
        List<String> blackListedModels =
            WebRtcAudioUtils.getBlackListedModelsForNsUsage();
        boolean isBlacklisted = blackListedModels.contains(Build.MODEL);

        if (isBlacklisted) {
            Logging.w(TAG, Build.MODEL + " is blacklisted for HW NS usage!");
        }

        return isBlacklisted;
    }

    @TargetApi(18)
    private static boolean isAcousticEchoCancelerExcludedByUUID()
    {
        for (Descriptor d : getAvailableEffects()) {
            if (d.type.equals(AudioEffect.EFFECT_TYPE_AEC) &&
                d.uuid.equals(AOSP_ACOUSTIC_ECHO_CANCELER))
            {
                return true;
            }
        }

        return false;
    }

    @TargetApi(18)
    private static boolean isAutomaticGainControlExcludedByUUID()
    {
        for (Descriptor d : getAvailableEffects()) {
            if (d.type.equals(AudioEffect.EFFECT_TYPE_AGC) &&
                d.uuid.equals(AOSP_AUTOMATIC_GAIN_CONTROL))
            {
                return true;
            }
        }

        return false;
    }

    @TargetApi(18)
    private static boolean isNoiseSuppressorExcludedByUUID()
    {
        for (Descriptor d : getAvailableEffects()) {
            if (d.type.equals(AudioEffect.EFFECT_TYPE_NS) &&
                d.uuid.equals(AOSP_NOISE_SUPPRESSOR))
            {
                return true;
            }
        }

        return false;
    }

    @TargetApi(18)
    private static boolean isAcousticEchoCancelerEffectAvailable()
    {
        return isEffectTypeAvailable(AudioEffect.EFFECT_TYPE_AEC);
    }

    @TargetApi(18)
    private static boolean isAutomaticGainControlEffectAvailable()
    {
        return isEffectTypeAvailable(AudioEffect.EFFECT_TYPE_AGC);
    }

    @TargetApi(18)
    private static boolean isNoiseSuppressorEffectAvailable()
    {
        return isEffectTypeAvailable(AudioEffect.EFFECT_TYPE_NS);
    }

    public static boolean canUseAcousticEchoCanceler()
    {
        boolean canUseAcousticEchoCanceler =
            isAcousticEchoCancelerSupported() &&
            !WebRtcAudioUtils.useWebRtcBasedAcousticEchoCanceler() &&
            !isAcousticEchoCancelerBlacklisted() &&
            !isAcousticEchoCancelerExcludedByUUID();

        Logging.d(TAG, "canUseAcousticEchoCanceler: "
                  + canUseAcousticEchoCanceler);
        return canUseAcousticEchoCanceler;
    }

    public static boolean canUseAutomaticGainControl()
    {
        boolean canUseAutomaticGainControl =
            isAutomaticGainControlSupported() &&
            !WebRtcAudioUtils.useWebRtcBasedAutomaticGainControl() &&
            !isAutomaticGainControlBlacklisted() &&
            !isAutomaticGainControlExcludedByUUID();

        Logging.d(TAG, "canUseAutomaticGainControl: "
                  + canUseAutomaticGainControl);
        return canUseAutomaticGainControl;
    }

    public static boolean canUseNoiseSuppressor()
    {
        boolean canUseNoiseSuppressor =
            isNoiseSuppressorSupported() &&
            !WebRtcAudioUtils.useWebRtcBasedNoiseSuppressor() &&
            !isNoiseSuppressorBlacklisted() &&
            !isNoiseSuppressorExcludedByUUID();

        Logging.d(TAG, "canUseNoiseSuppressor: " + canUseNoiseSuppressor);
        return canUseNoiseSuppressor;
    }

    static WebRtcAudioEffects create()
    {
        if (!WebRtcAudioUtils.runningOnJellyBeanOrHigher()) {
            Logging.w(TAG, "API level 16 or higher is required!");
            return null;
        }

        return new WebRtcAudioEffects();
    }

    private WebRtcAudioEffects()
    {
        Logging.d(TAG, "ctor" + WebRtcAudioUtils.getThreadInfo());
    }

    public boolean setAEC(boolean enable)
    {
        Logging.d(TAG, "setAEC(" + enable + ")");

        if (!canUseAcousticEchoCanceler()) {
            Logging.w(TAG, "Platform AEC is not supported");
            shouldEnableAec = false;
            return false;
        }

        if (aec != null && (enable != shouldEnableAec)) {
            Logging.e(TAG, "Platform AEC state can't be modified while recording");
            return false;
        }

        shouldEnableAec = enable;
        return true;
    }

    public boolean setAGC(boolean enable)
    {
        Logging.d(TAG, "setAGC(" + enable + ")");

        if (!canUseAutomaticGainControl()) {
            Logging.w(TAG, "Platform AGC is not supported");
            shouldEnableAgc = false;
            return false;
        }

        if (agc != null && (enable != shouldEnableAgc)) {
            Logging.e(TAG, "Platform AGC state can't be modified while recording");
            return false;
        }

        shouldEnableAgc = enable;
        return true;
    }

    public boolean setNS(boolean enable)
    {
        Logging.d(TAG, "setNS(" + enable + ")");

        if (!canUseNoiseSuppressor()) {
            Logging.w(TAG, "Platform NS is not supported");
            shouldEnableNs = false;
            return false;
        }

        if (ns != null && (enable != shouldEnableNs)) {
            Logging.e(TAG, "Platform NS state can't be modified while recording");
            return false;
        }

        shouldEnableNs = enable;
        return true;
    }

    public void enable(int audioSession)
    {
        Logging.d(TAG, "enable(audioSession=" + audioSession + ")");
        assertTrue(aec == null);
        assertTrue(agc == null);
        assertTrue(ns == null);

        for (Descriptor d : AudioEffect.queryEffects()) {
            if (effectTypeIsVoIP(d.type) || DEBUG) {
                Logging.d(TAG, "name: " + d.name + ", "
                          + "mode: " + d.connectMode + ", "
                          + "implementor: " + d.implementor + ", "
                          + "UUID: " + d.uuid);
            }
        }

        if (isAcousticEchoCancelerSupported()) {
            aec = AcousticEchoCanceler.create(audioSession);

            if (aec != null) {
                boolean enabled = aec.getEnabled();
                boolean enable  = shouldEnableAec && canUseAcousticEchoCanceler();

                if (aec.setEnabled(enable) != AudioEffect.SUCCESS) {
                    Logging.e(TAG, "Failed to set the AcousticEchoCanceler state");
                }

                Logging.d(TAG, "AcousticEchoCanceler: was "
                          + (enabled ? "enabled" : "disabled")
                          + ", enable: " + enable + ", is now: "
                          + (aec.getEnabled() ? "enabled" : "disabled"));
            } else {
                Logging.e(TAG, "Failed to create the AcousticEchoCanceler instance");
            }
        }

        if (isAutomaticGainControlSupported()) {
            agc = AutomaticGainControl.create(audioSession);

            if (agc != null) {
                boolean enabled = agc.getEnabled();
                boolean enable  = shouldEnableAgc && canUseAutomaticGainControl();

                if (agc.setEnabled(enable) != AudioEffect.SUCCESS) {
                    Logging.e(TAG, "Failed to set the AutomaticGainControl state");
                }

                Logging.d(TAG, "AutomaticGainControl: was "
                          + (enabled ? "enabled" : "disabled")
                          + ", enable: " + enable + ", is now: "
                          + (agc.getEnabled() ? "enabled" : "disabled"));
            } else {
                Logging.e(TAG, "Failed to create the AutomaticGainControl instance");
            }
        }

        if (isNoiseSuppressorSupported()) {
            ns = NoiseSuppressor.create(audioSession);

            if (ns != null) {
                boolean enabled = ns.getEnabled();
                boolean enable  = shouldEnableNs && canUseNoiseSuppressor();

                if (ns.setEnabled(enable) != AudioEffect.SUCCESS) {
                    Logging.e(TAG, "Failed to set the NoiseSuppressor state");
                }

                Logging.d(TAG, "NoiseSuppressor: was "
                          + (enabled ? "enabled" : "disabled")
                          + ", enable: " + enable + ", is now: "
                          + (ns.getEnabled() ? "enabled" : "disabled"));
            } else {
                Logging.e(TAG, "Failed to create the NoiseSuppressor instance");
            }
        }
    } /* enable */

    public void release()
    {
        Logging.d(TAG, "release");

        if (aec != null) {
            aec.release();
            aec = null;
        }

        if (agc != null) {
            agc.release();
            agc = null;
        }

        if (ns != null) {
            ns.release();
            ns = null;
        }
    }

    @TargetApi(18)
    private boolean effectTypeIsVoIP(UUID type)
    {
        if (!WebRtcAudioUtils.runningOnJellyBeanMR2OrHigher()) {
            return false;
        }

        return (AudioEffect.EFFECT_TYPE_AEC.equals(type) &&
                isAcousticEchoCancelerSupported()) ||
               (AudioEffect.EFFECT_TYPE_AGC.equals(type) &&
                isAutomaticGainControlSupported()) ||
               (AudioEffect.EFFECT_TYPE_NS.equals(type) &&
                isNoiseSuppressorSupported());
    }

    private static void assertTrue(boolean condition)
    {
        if (!condition) {
            throw new AssertionError("Expected condition to be true");
        }
    }

    private static Descriptor[] getAvailableEffects()
    {
        if (cachedEffects != null) {
            return cachedEffects;
        }

        cachedEffects = AudioEffect.queryEffects();
        return cachedEffects;
    }

    private static boolean isEffectTypeAvailable(UUID effectType)
    {
        Descriptor[] effects = getAvailableEffects();

        if (effects == null) {
            return false;
        }

        for (Descriptor d : effects) {
            if (d.type.equals(effectType)) {
                return true;
            }
        }

        return false;
    }
}
