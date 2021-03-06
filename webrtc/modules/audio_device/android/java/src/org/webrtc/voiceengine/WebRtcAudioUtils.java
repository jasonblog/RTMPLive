

package org.webrtc.voiceengine;

import android.content.Context;
import android.content.pm.PackageManager;
import android.media.audiofx.AcousticEchoCanceler;
import android.media.audiofx.AudioEffect;
import android.media.audiofx.AudioEffect.Descriptor;
import android.media.AudioManager;
import android.os.Build;
import android.os.Process;

import org.webrtc.Logging;

import java.lang.Thread;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public final class WebRtcAudioUtils
{
    private static final String TAG = "WebRtcAudioUtils";

    
    
    
    private static final String[] BLACKLISTED_OPEN_SL_ES_MODELS = new String[] {
        
    };

    
    
    
    private static final String[] BLACKLISTED_AEC_MODELS = new String[] {
        "D6503",      
        "ONE A2005",  
        "MotoG3",     
    };
    private static final String[] BLACKLISTED_AGC_MODELS = new String[] {
        "Nexus 10",
        "Nexus 9",
    };
    private static final String[] BLACKLISTED_NS_MODELS = new String[] {
        "Nexus 10",
        "Nexus 9",
        "ONE A2005",  
    };

    
    
    private static final int DEFAULT_SAMPLE_RATE_HZ = 16000;
    private static int defaultSampleRateHz = DEFAULT_SAMPLE_RATE_HZ;
    
    private static boolean isDefaultSampleRateOverridden = false;

    
    private static boolean useWebRtcBasedAcousticEchoCanceler = false;
    private static boolean useWebRtcBasedAutomaticGainControl = false;
    private static boolean useWebRtcBasedNoiseSuppressor = false;

    
    
    public static synchronized void setWebRtcBasedAcousticEchoCanceler(
        boolean enable)
    {
        useWebRtcBasedAcousticEchoCanceler = enable;
    }
    public static synchronized void setWebRtcBasedAutomaticGainControl(
        boolean enable)
    {
        useWebRtcBasedAutomaticGainControl = enable;
    }
    public static synchronized void setWebRtcBasedNoiseSuppressor(
        boolean enable)
    {
        useWebRtcBasedNoiseSuppressor = enable;
    }

    public static synchronized boolean useWebRtcBasedAcousticEchoCanceler()
    {
        if (useWebRtcBasedAcousticEchoCanceler) {
            Logging.w(TAG, "Overriding default behavior; now using WebRTC AEC!");
        }

        return useWebRtcBasedAcousticEchoCanceler;
    }
    public static synchronized boolean useWebRtcBasedAutomaticGainControl()
    {
        if (useWebRtcBasedAutomaticGainControl) {
            Logging.w(TAG, "Overriding default behavior; now using WebRTC AGC!");
        }

        return useWebRtcBasedAutomaticGainControl;
    }
    public static synchronized boolean useWebRtcBasedNoiseSuppressor()
    {
        if (useWebRtcBasedNoiseSuppressor) {
            Logging.w(TAG, "Overriding default behavior; now using WebRTC NS!");
        }

        return useWebRtcBasedNoiseSuppressor;
    }

    
    
    
    
    
    
    public static boolean isAcousticEchoCancelerSupported()
    {
        return WebRtcAudioEffects.canUseAcousticEchoCanceler();
    }
    public static boolean isAutomaticGainControlSupported()
    {
        return WebRtcAudioEffects.canUseAutomaticGainControl();
    }
    public static boolean isNoiseSuppressorSupported()
    {
        return WebRtcAudioEffects.canUseNoiseSuppressor();
    }

    
    
    
    public static synchronized void setDefaultSampleRateHz(int sampleRateHz)
    {
        isDefaultSampleRateOverridden = true;
        defaultSampleRateHz = sampleRateHz;
    }

    public static synchronized boolean isDefaultSampleRateOverridden()
    {
        return isDefaultSampleRateOverridden;
    }

    public static synchronized int getDefaultSampleRateHz()
    {
        return defaultSampleRateHz;
    }

    public static List<String> getBlackListedModelsForAecUsage()
    {
        return Arrays.asList(WebRtcAudioUtils.BLACKLISTED_AEC_MODELS);
    }

    public static List<String> getBlackListedModelsForAgcUsage()
    {
        return Arrays.asList(WebRtcAudioUtils.BLACKLISTED_AGC_MODELS);
    }

    public static List<String> getBlackListedModelsForNsUsage()
    {
        return Arrays.asList(WebRtcAudioUtils.BLACKLISTED_NS_MODELS);
    }

    public static boolean runningOnGingerBreadOrHigher()
    {
        
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD;
    }

    public static boolean runningOnJellyBeanOrHigher()
    {
        
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN;
    }

    public static boolean runningOnJellyBeanMR1OrHigher()
    {
        
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1;
    }

    public static boolean runningOnJellyBeanMR2OrHigher()
    {
        
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2;
    }

    public static boolean runningOnLollipopOrHigher()
    {
        
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP;
    }

    public static boolean runningOnMarshmallowOrHigher()
    {
        
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.M;
    }

    
    public static String getThreadInfo()
    {
        return "@[name=" + Thread.currentThread().getName()
               + ", id=" + Thread.currentThread().getId() + "]";
    }

    
    public static boolean runningOnEmulator()
    {
        return Build.HARDWARE.equals("goldfish") &&
               Build.BRAND.startsWith("generic_");
    }

    
    public static boolean deviceIsBlacklistedForOpenSLESUsage()
    {
        List<String> blackListedModels =
            Arrays.asList(BLACKLISTED_OPEN_SL_ES_MODELS);
        return blackListedModels.contains(Build.MODEL);
    }

    
    public static void logDeviceInfo(String tag)
    {
        Logging.d(tag, "Android SDK: " + Build.VERSION.SDK_INT + ", "
                  + "Release: " + Build.VERSION.RELEASE + ", "
                  + "Brand: " + Build.BRAND + ", "
                  + "Device: " + Build.DEVICE + ", "
                  + "Id: " + Build.ID + ", "
                  + "Hardware: " + Build.HARDWARE + ", "
                  + "Manufacturer: " + Build.MANUFACTURER + ", "
                  + "Model: " + Build.MODEL + ", "
                  + "Product: " + Build.PRODUCT);
    }

    
    public static boolean hasPermission(Context context, String permission)
    {
        return context.checkPermission(
                   permission,
                   Process.myPid(),
                   Process.myUid()) == PackageManager.PERMISSION_GRANTED;
    }
}
