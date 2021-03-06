

package org.webrtc.voiceengine;

import java.lang.System;
import java.nio.ByteBuffer;
import java.util.concurrent.TimeUnit;

import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;
import android.os.Process;

import org.webrtc.Logging;
import org.webrtc.ThreadUtils;

public class  WebRtcAudioRecord
{
    private static final boolean DEBUG = false;

    private static final String TAG = "WebRtcAudioRecord";

    
    
    private static final int BITS_PER_SAMPLE = 16;

    
    private static final int CALLBACK_BUFFER_SIZE_MS = 10;

    
    private static final int BUFFERS_PER_SECOND = 1000 / CALLBACK_BUFFER_SIZE_MS;

    
    
    
    private static final int BUFFER_SIZE_FACTOR = 2;

    
    
    private static final long AUDIO_RECORD_THREAD_JOIN_TIMEOUT_MS = 2000;

    private final long nativeAudioRecord;
    private final Context context;

    private WebRtcAudioEffects effects = null;

    private ByteBuffer byteBuffer;

    private AudioRecord audioRecord = null;
    private AudioRecordThread audioThread = null;

    private static volatile boolean microphoneMute = false;
    private byte[] emptyBytes;

    
    private class AudioRecordThread extends Thread
    {
        private volatile boolean keepAlive = true;

        public AudioRecordThread(String name)
        {
            super(name);
        }

        @Override
        public void run()
        {
            Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);
            Logging.d(TAG, "AudioRecordThread" + WebRtcAudioUtils.getThreadInfo());
            assertTrue(audioRecord.getRecordingState()
                       == AudioRecord.RECORDSTATE_RECORDING);

            long lastTime = System.nanoTime();

            while (keepAlive) {
                int bytesRead = audioRecord.read(byteBuffer, byteBuffer.capacity());

                if (bytesRead == byteBuffer.capacity()) {
                    if (microphoneMute) {
                        byteBuffer.clear();
                        byteBuffer.put(emptyBytes);
                    }

                    nativeDataIsRecorded(bytesRead, nativeAudioRecord);
                } else {
                    Logging.e(TAG, "AudioRecord.read failed: " + bytesRead);

                    if (bytesRead == AudioRecord.ERROR_INVALID_OPERATION) {
                        keepAlive = false;
                    }
                }

                if (DEBUG) {
                    long nowTime = System.nanoTime();
                    long durationInMs =
                        TimeUnit.NANOSECONDS.toMillis((nowTime - lastTime));
                    lastTime = nowTime;
                    Logging.d(TAG, "bytesRead[" + durationInMs + "] " + bytesRead);
                }
            }

            try {
                audioRecord.stop();
            } catch (IllegalStateException e) {
                Logging.e(TAG, "AudioRecord.stop failed: " + e.getMessage());
            }
        }

        
        
        public void stopThread()
        {
            Logging.d(TAG, "stopThread");
            keepAlive = false;
        }
    }

    WebRtcAudioRecord(Context context, long nativeAudioRecord)
    {
        Logging.d(TAG, "ctor" + WebRtcAudioUtils.getThreadInfo());
        this.context = context;
        this.nativeAudioRecord = nativeAudioRecord;

        if (DEBUG) {
            WebRtcAudioUtils.logDeviceInfo(TAG);
        }

        effects = WebRtcAudioEffects.create();
    }

    private boolean enableBuiltInAEC(boolean enable)
    {
        Logging.d(TAG, "enableBuiltInAEC(" + enable + ')');

        if (effects == null) {
            Logging.e(TAG, "Built-in AEC is not supported on this platform");
            return false;
        }

        return effects.setAEC(enable);
    }

    private boolean enableBuiltInAGC(boolean enable)
    {
        Logging.d(TAG, "enableBuiltInAGC(" + enable + ')');

        if (effects == null) {
            Logging.e(TAG, "Built-in AGC is not supported on this platform");
            return false;
        }

        return effects.setAGC(enable);
    }

    private boolean enableBuiltInNS(boolean enable)
    {
        Logging.d(TAG, "enableBuiltInNS(" + enable + ')');

        if (effects == null) {
            Logging.e(TAG, "Built-in NS is not supported on this platform");
            return false;
        }

        return effects.setNS(enable);
    }

    private int initRecording(int sampleRate, int channels)
    {
        Logging.d(TAG, "initRecording(sampleRate=" + sampleRate + ", channels=" +
                  channels + ")");

        if (!WebRtcAudioUtils.hasPermission(
                context, android.Manifest.permission.RECORD_AUDIO)) {
            Logging.e(TAG, "RECORD_AUDIO permission is missing");
            return -1;
        }

        if (audioRecord != null) {
            Logging.e(TAG, "InitRecording() called twice without StopRecording()");
            return -1;
        }

        final int bytesPerFrame = channels * (BITS_PER_SAMPLE / 8);
        final int framesPerBuffer = sampleRate / BUFFERS_PER_SECOND;
        byteBuffer = ByteBuffer.allocateDirect(bytesPerFrame * framesPerBuffer);
        Logging.d(TAG, "byteBuffer.capacity: " + byteBuffer.capacity());
        emptyBytes = new byte[byteBuffer.capacity()];
        
        
        
        nativeCacheDirectBufferAddress(byteBuffer, nativeAudioRecord);

        
        
        
        int minBufferSize = AudioRecord.getMinBufferSize(
                                sampleRate,
                                AudioFormat.CHANNEL_IN_MONO,
                                AudioFormat.ENCODING_PCM_16BIT);

        if (minBufferSize == AudioRecord.ERROR
            || minBufferSize == AudioRecord.ERROR_BAD_VALUE) {
            Logging.e(TAG, "AudioRecord.getMinBufferSize failed: " + minBufferSize);
            return -1;
        }

        Logging.d(TAG, "AudioRecord.getMinBufferSize: " + minBufferSize);

        
        
        
        int bufferSizeInBytes =
            Math.max(BUFFER_SIZE_FACTOR * minBufferSize, byteBuffer.capacity());
        Logging.d(TAG, "bufferSizeInBytes: " + bufferSizeInBytes);

        try {
            audioRecord = new AudioRecord(AudioSource.VOICE_COMMUNICATION,
                                          sampleRate,
                                          AudioFormat.CHANNEL_IN_MONO,
                                          AudioFormat.ENCODING_PCM_16BIT,
                                          bufferSizeInBytes);
        } catch (IllegalArgumentException e) {
            Logging.e(TAG, e.getMessage());
            return -1;
        }

        if (audioRecord == null ||
            audioRecord.getState() != AudioRecord.STATE_INITIALIZED) {
            Logging.e(TAG, "Failed to create a new AudioRecord instance");
            return -1;
        }

        Logging.d(TAG, "AudioRecord "
                  + "session ID: " + audioRecord.getAudioSessionId() + ", "
                  + "audio format: " + audioRecord.getAudioFormat() + ", "
                  + "channels: " + audioRecord.getChannelCount() + ", "
                  + "sample rate: " + audioRecord.getSampleRate());

        if (effects != null) {
            effects.enable(audioRecord.getAudioSessionId());
        }

        
        
        
        
        
        
        
        

        
        
        
        return framesPerBuffer;
    }

    private boolean startRecording()
    {
        Logging.d(TAG, "startRecording");
        assertTrue(audioRecord != null);
        assertTrue(audioThread == null);

        try {
            audioRecord.startRecording();
        } catch (IllegalStateException e) {
            Logging.e(TAG, "AudioRecord.startRecording failed: " + e.getMessage());
            return false;
        }

        if (audioRecord.getRecordingState() != AudioRecord.RECORDSTATE_RECORDING) {
            Logging.e(TAG, "AudioRecord.startRecording failed");
            return false;
        }

        audioThread = new AudioRecordThread("AudioRecordJavaThread");
        audioThread.start();
        return true;
    }

    private boolean stopRecording()
    {
        Logging.d(TAG, "stopRecording");
        assertTrue(audioThread != null);
        audioThread.stopThread();

        if (!ThreadUtils.joinUninterruptibly(
                audioThread, AUDIO_RECORD_THREAD_JOIN_TIMEOUT_MS)) {
            Logging.e(TAG, "Join of AudioRecordJavaThread timed out");
        }

        audioThread = null;

        if (effects != null) {
            effects.release();
        }

        audioRecord.release();
        audioRecord = null;
        return true;
    }

    
    private static void assertTrue(boolean condition)
    {
        if (!condition) {
            throw new AssertionError("Expected condition to be true");
        }
    }

    private native void nativeCacheDirectBufferAddress(
        ByteBuffer byteBuffer, long nativeAudioRecord);

    private native void nativeDataIsRecorded(int bytes, long nativeAudioRecord);

    
    
    public static void setMicrophoneMute(boolean mute)
    {
        Logging.w(TAG, "setMicrophoneMute(" + mute + ")");
        microphoneMute = mute;
    }
}
