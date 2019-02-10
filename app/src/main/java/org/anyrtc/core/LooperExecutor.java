package org.anyrtc.core;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import java.util.concurrent.Exchanger;
import java.util.concurrent.Executor;
public class LooperExecutor extends Thread implements Executor
{
    private static final String TAG         = "LooperExecutor";
    private final Object looperStartedEvent = new Object();
    private Handler handler = null;
    private boolean running = false;
    private long threadId;
    @Override
    public void run()
    {
        Looper.prepare();
        synchronized (looperStartedEvent) {
            Log.d(TAG, "Looper thread started.");
            handler  = new Handler();
            threadId = Thread.currentThread().getId();
            looperStartedEvent.notify();
        }
        Looper.loop();
    }

    public synchronized void requestStart()
    {
        if (running) {
            return;
        }

        running = true;
        handler = null;
        start();
        synchronized (looperStartedEvent) {
            while (handler == null) {
                try {
                    looperStartedEvent.wait();
                } catch (InterruptedException e) {
                    Log.e(TAG, "Can not start looper thread");
                    running = false;
                }
            }
        }
    }

    public synchronized void requestStop()
    {
        if (!running) {
            return;
        }

        running = false;
        handler.post(new Runnable() {
            @Override
            public void run() {
                Looper.myLooper().quit();
                Log.d(TAG, "Looper thread finished.");
            }
        });
    }

    public boolean checkOnLooperThread()
    {
        return (Thread.currentThread().getId() == threadId);
    }

    @Override
    public synchronized void execute(final Runnable runnable)
    {
        if (!running) {
            Log.w(TAG, "Running looper executor without calling requestStart()");
            return;
        }

        if (Thread.currentThread().getId() == threadId) {
            runnable.run();
        } else {
            handler.post(runnable);
        }
    }

    public static <T> T exchange(Exchanger<T> exchanger, T value)
    {
        try {
            return exchanger.exchange(value);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }
}
