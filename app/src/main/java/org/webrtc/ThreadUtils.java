package org.webrtc;
import android.os.Handler;
import android.os.SystemClock;
import java.util.concurrent.Callable;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
public class ThreadUtils
{
    public static class ThreadChecker
    {
        private Thread thread = Thread.currentThread();
        public void checkIsOnValidThread()
        {
            if (thread == null) {
                thread = Thread.currentThread();
            }

            if (Thread.currentThread() != thread) {
                throw new IllegalStateException("Wrong thread");
            }
        }

        public void detachThread()
        {
            thread = null;
        }
    }
    public interface BlockingOperation
    {
        void run() throws InterruptedException;
    }
    public static void executeUninterruptibly(BlockingOperation operation)
    {
        boolean wasInterrupted = false;

        while (true) {
            try {
                operation.run();
                break;
            } catch (InterruptedException e) {
                wasInterrupted = true;
            }
        }

        if (wasInterrupted) {
            Thread.currentThread().interrupt();
        }
    }

    public static boolean joinUninterruptibly(final Thread thread, long timeoutMs)
    {
        final long startTimeMs = SystemClock.elapsedRealtime();
        long timeRemainingMs   = timeoutMs;
        boolean wasInterrupted = false;

        while (timeRemainingMs > 0) {
            try {
                thread.join(timeRemainingMs);
                break;
            } catch (InterruptedException e) {
                wasInterrupted = true;
                final long elapsedTimeMs = SystemClock.elapsedRealtime() - startTimeMs;
                timeRemainingMs = timeoutMs - elapsedTimeMs;
            }
        }

        if (wasInterrupted) {
            Thread.currentThread().interrupt();
        }

        return !thread.isAlive();
    }

    public static void joinUninterruptibly(final Thread thread)
    {
        executeUninterruptibly(new BlockingOperation() {
            @Override
            public void run() throws InterruptedException {
                thread.join();
            }
        });
    }

    public static void awaitUninterruptibly(final CountDownLatch latch)
    {
        executeUninterruptibly(new BlockingOperation() {
            @Override
            public void run() throws InterruptedException {
                latch.await();
            }
        });
    }

    public static boolean awaitUninterruptibly(CountDownLatch barrier, long timeoutMs)
    {
        final long startTimeMs = SystemClock.elapsedRealtime();
        long timeRemainingMs   = timeoutMs;
        boolean wasInterrupted = false;
        boolean result         = false;

        do {
            try {
                result = barrier.await(timeRemainingMs, TimeUnit.MILLISECONDS);
                break;
            } catch (InterruptedException e) {
                wasInterrupted = true;
                final long elapsedTimeMs = SystemClock.elapsedRealtime() - startTimeMs;
                timeRemainingMs = timeoutMs - elapsedTimeMs;
            }
        } while (timeRemainingMs > 0);

        if (wasInterrupted) {
            Thread.currentThread().interrupt();
        }

        return result;
    }

    public static <V> V invokeAtFrontUninterruptibly(
        final Handler handler, final Callable<V> callable)
    {
        if (handler.getLooper().getThread() == Thread.currentThread()) {
            V value;

            try {
                value = callable.call();
            } catch (Exception e) {
                final RuntimeException runtimeException =
                    new RuntimeException("Callable threw exception: " + e);
                runtimeException.setStackTrace(e.getStackTrace());
                throw runtimeException;
            }

            return value;
        }

        class Result
        {
            public V value;
        }

        final Result result = new Result();
        final CountDownLatch barrier = new CountDownLatch(1);
        handler.post(new Runnable() {
            @Override public void run() {
                try {
                    result.value = callable.call();
                } catch (Exception e) {
                    final RuntimeException runtimeException =
                        new RuntimeException("Callable threw exception: " + e);
                    runtimeException.setStackTrace(e.getStackTrace());
                    throw runtimeException;
                }

                barrier.countDown();
            }
        });
        awaitUninterruptibly(barrier);
        return result.value;
    } /* invokeAtFrontUninterruptibly */

    public static void invokeAtFrontUninterruptibly(final Handler handler, final Runnable runner)
    {
        if (handler.getLooper().getThread() == Thread.currentThread()) {
            runner.run();
            return;
        }

        final CountDownLatch barrier = new CountDownLatch(1);
        handler.postAtFrontOfQueue(new Runnable() {
            @Override
            public void run() {
                runner.run();
                barrier.countDown();
            }
        });
        awaitUninterruptibly(barrier);
    }
}
