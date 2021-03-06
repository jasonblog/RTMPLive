

package org.webrtc.voiceengine;

import java.lang.Thread;
import java.nio.ByteBuffer;

import android.annotation.TargetApi;
import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Process;

import org.webrtc.Logging;

public class WebRtcAudioTrack
{
    private static final boolean DEBUG = false;

    private static final String TAG = "WebRtcAudioTrack";

    
    
    private static final int BITS_PER_SAMPLE = 16;

    
    private static final int CALLBACK_BUFFER_SIZE_MS = 10;

    
    private static final int BUFFERS_PER_SECOND = 1000 / CALLBACK_BUFFER_SIZE_MS;

    private final Context context;
    private final long nativeAudioTrack;
    private final AudioManager audioManager;

    private ByteBuffer byteBuffer;

    private AudioTrack audioTrack = null;
    private AudioTrackThread audioThread = null;

    
    
    private static volatile boolean speakerMute = false;
    private byte[] emptyBytes;

    
    private class AudioTrackThread extends Thread
    {
        private volatile boolean keepAlive = true;

        public AudioTrackThread(String name)
        {
            super(name);
        }

        @Override
        public void run()
        {
            Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);
            Logging.d(TAG, "AudioTrackThread" + WebRtcAudioUtils.getThreadInfo());

            try {
                
                
                
                
                audioTrack.play();
                assertTrue(audioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING);
            } catch (IllegalStateException e) {
                Logging.e(TAG, "AudioTrack.play failed: " + e.getMessage());
                return;
            }

            
            
            final int sizeInBytes = byteBuffer.capacity();

            while (keepAlive) {
                
                
                
                nativeGetPlayoutData(sizeInBytes, nativeAudioTrack);
                
                
                
                assertTrue(sizeInBytes <= byteBuffer.remaining());

                if (speakerMute) {
                    byteBuffer.clear();
                    byteBuffer.put(emptyBytes);
                    byteBuffer.position(0);
                }

                int bytesWritten = 0;

                if (WebRtcAudioUtils.runningOnLollipopOrHigher()) {
                    bytesWritten = writeOnLollipop(audioTrack, byteBuffer, sizeInBytes);
                } else {
                    bytesWritten = writePreLollipop(audioTrack, byteBuffer, sizeInBytes);
                }

                if (bytesWritten != sizeInBytes) {
                    Logging.e(TAG, "AudioTrack.write failed: " + bytesWritten);

                    if (bytesWritten == AudioTrack.ERROR_INVALID_OPERATION) {
                        keepAlive = false;
                    }
                }

                
                
                
                byteBuffer.rewind();

                
                
                
            }

            try {
                audioTrack.stop();
            } catch (IllegalStateException e) {
                Logging.e(TAG, "AudioTrack.stop failed: " + e.getMessage());
            }

            assertTrue(audioTrack.getPlayState() == AudioTrack.PLAYSTATE_STOPPED);
            audioTrack.flush();
        }

        @TargetApi(21)
        private int writeOnLollipop(AudioTrack audioTrack, ByteBuffer byteBuffer, int sizeInBytes)
        {
            return audioTrack.write(byteBuffer, sizeInBytes, AudioTrack.WRITE_BLOCKING);
        }

        private int writePreLollipop(AudioTrack audioTrack, ByteBuffer byteBuffer, int sizeInBytes)
        {
            return audioTrack.write(byteBuffer.array(), byteBuffer.arrayOffset(), sizeInBytes);
        }

        public void joinThread()
        {
            keepAlive = false;

            while (isAlive()) {
                try {
                    join();
                } catch (InterruptedException e) {
                    
                }
            }
        }
    }

    WebRtcAudioTrack(Context context, long nativeAudioTrack)
    {
        Logging.d(TAG, "ctor" + WebRtcAudioUtils.getThreadInfo());
        this.context = context;
        this.nativeAudioTrack = nativeAudioTrack;
        audioManager = (AudioManager) context.getSystemService(
                           Context.AUDIO_SERVICE);

        if (DEBUG) {
            WebRtcAudioUtils.logDeviceInfo(TAG);
        }
    }

    private boolean initPlayout(int sampleRate, int channels)
    {
        Logging.d(TAG, "initPlayout(sampleRate=" + sampleRate + ", channels="
                  + channels + ")");
        final int bytesPerFrame = channels * (BITS_PER_SAMPLE / 8);
        byteBuffer = byteBuffer.allocateDirect(
                         bytesPerFrame * (sampleRate / BUFFERS_PER_SECOND));
        Logging.d(TAG, "byteBuffer.capacity: " + byteBuffer.capacity());
        emptyBytes = new byte[byteBuffer.capacity()];
        
        
        
        nativeCacheDirectBufferAddress(byteBuffer, nativeAudioTrack);

        
        
        
        
        final int minBufferSizeInBytes = AudioTrack.getMinBufferSize(
                                             sampleRate,
                                             AudioFormat.CHANNEL_OUT_MONO,
                                             AudioFormat.ENCODING_PCM_16BIT);
        Logging.d(TAG, "AudioTrack.getMinBufferSize: " + minBufferSizeInBytes);
        assertTrue(audioTrack == null);

        
        
        
        assertTrue(byteBuffer.capacity() < minBufferSizeInBytes);

        try {
            
            
            
            audioTrack = new AudioTrack(AudioManager.STREAM_VOICE_CALL,
                                        sampleRate,
                                        AudioFormat.CHANNEL_OUT_MONO,
                                        AudioFormat.ENCODING_PCM_16BIT,
                                        minBufferSizeInBytes,
                                        AudioTrack.MODE_STREAM);
        } catch (IllegalArgumentException e) {
            Logging.d(TAG, e.getMessage());
            return false;
        }

        
        
        
        if (audioTrack.getState() != AudioTrack.STATE_INITIALIZED) {
            Logging.e(TAG, "Initialization of audio track failed.");
            return false;
        }

        return true;
    }

    private boolean startPlayout()
    {
        Logging.d(TAG, "startPlayout");
        assertTrue(audioTrack != null);
        assertTrue(audioThread == null);

        if (audioTrack.getState() != AudioTrack.STATE_INITIALIZED) {
            Logging.e(TAG, "Audio track is not successfully initialized.");
            return false;
        }

        audioThread = new AudioTrackThread("AudioTrackJavaThread");
        audioThread.start();
        return true;
    }

    private boolean stopPlayout()
    {
        Logging.d(TAG, "stopPlayout");
        assertTrue(audioThread != null);
        audioThread.joinThread();
        audioThread = null;

        if (audioTrack != null) {
            audioTrack.release();
            audioTrack = null;
        }

        return true;
    }

    
    private int getStreamMaxVolume()
    {
        Logging.d(TAG, "getStreamMaxVolume");
        assertTrue(audioManager != null);
        return audioManager.getStreamMaxVolume(AudioManager.STREAM_VOICE_CALL);
    }

    
    private boolean setStreamVolume(int volume)
    {
        Logging.d(TAG, "setStreamVolume(" + volume + ")");
        assertTrue(audioManager != null);

        if (isVolumeFixed()) {
            Logging.e(TAG, "The device implements a fixed volume policy.");
            return false;
        }

        audioManager.setStreamVolume(AudioManager.STREAM_VOICE_CALL, volume, 0);
        return true;
    }

    @TargetApi(21)
    private boolean isVolumeFixed()
    {
        if (!WebRtcAudioUtils.runningOnLollipopOrHigher()) {
            return false;
        }

        return audioManager.isVolumeFixed();
    }

    
    private int getStreamVolume()
    {
        Logging.d(TAG, "getStreamVolume");
        assertTrue(audioManager != null);
        return audioManager.getStreamVolume(AudioManager.STREAM_VOICE_CALL);
    }

    
    private static void assertTrue(boolean condition)
    {
        if (!condition) {
            throw new AssertionError("Expected condition to be true");
        }
    }

    private native void nativeCacheDirectBufferAddress(
        ByteBuffer byteBuffer, long nativeAudioRecord);

    private native void nativeGetPlayoutData(int bytes, long nativeAudioRecord);

    
    
    public static void setSpeakerMute(boolean mute)
    {
        Logging.w(TAG, "setSpeakerMute(" + mute + ")");
        speakerMute = mute;
    }
}
