package org.webrtc;
import android.content.Context;
import android.content.res.Resources.NotFoundException;
import android.graphics.Point;
import android.opengl.GLES20;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import java.util.concurrent.CountDownLatch;
public class SurfaceViewRenderer extends SurfaceView
    implements SurfaceHolder.Callback, VideoRenderer.Callbacks
{
    private static final String TAG = "SurfaceViewRenderer";
    private HandlerThread renderThread;
    private final Object handlerLock = new Object();
    private Handler renderThreadHandler;
    private EglBase eglBase;
    private final RendererCommon.YuvUploader yuvUploader = new RendererCommon.YuvUploader();
    private RendererCommon.GlDrawer drawer;
    private int[] yuvTextures      = null;
    private final Object frameLock = new Object();
    private VideoRenderer.I420Frame pendingFrame;
    private final Object layoutLock = new Object();
    private Point desiredLayoutSize = new Point();
    private final Point layoutSize  = new Point();
    private final Point surfaceSize = new Point();
    private boolean isSurfaceCreated;
    private int frameWidth;
    private int frameHeight;
    private int frameRotation;
    private RendererCommon.ScalingType scalingType = RendererCommon.ScalingType.SCALE_ASPECT_BALANCED;
    private boolean mirror;
    private RendererCommon.RendererEvents rendererEvents;
    private final Object statisticsLock = new Object();
    private int framesReceived;
    private int framesDropped;
    private int framesRendered;
    private long firstFrameTimeNs;
    private long renderTimeNs;
    private final Runnable renderFrameRunnable = new Runnable()
    {
        @Override public void run()
        {
            renderFrameOnRenderThread();
        }
    };
    private final Runnable makeBlackRunnable = new Runnable()
    {
        @Override public void run()
        {
            makeBlack();
        }
    };
    public SurfaceViewRenderer(Context context)
    {
        super(context);
        getHolder().addCallback(this);
    }

    public SurfaceViewRenderer(Context context, AttributeSet attrs)
    {
        super(context, attrs);
        getHolder().addCallback(this);
    }

    public void init(
        EglBase.Context sharedContext, RendererCommon.RendererEvents rendererEvents)
    {
        init(sharedContext, rendererEvents, EglBase.CONFIG_PLAIN, new GlRectDrawer());
    }

    public void init(EglBase.Context sharedContext, RendererCommon.RendererEvents rendererEvents,
                     int[] configAttributes, RendererCommon.GlDrawer drawer)
    {
        synchronized (handlerLock) {
            if (renderThreadHandler != null) {
                throw new IllegalStateException(getResourceName() + "Already initialized");
            }

            Logging.d(TAG, getResourceName() + "Initializing.");
            this.rendererEvents = rendererEvents;
            this.drawer         = drawer;
            renderThread        = new HandlerThread(TAG);
            renderThread.start();
            eglBase = EglBase.create(sharedContext, configAttributes);
            renderThreadHandler = new Handler(renderThread.getLooper());
        }
        tryCreateEglSurface();
    }

    public void tryCreateEglSurface()
    {
        runOnRenderThread(new Runnable() {
            @Override public void run() {
                synchronized (layoutLock) {
                    if (isSurfaceCreated && !eglBase.hasSurface()) {
                        eglBase.createSurface(getHolder().getSurface());
                        eglBase.makeCurrent();
                        GLES20.glPixelStorei(GLES20.GL_UNPACK_ALIGNMENT, 1);
                    }
                }
            }
        });
    }

    public void release()
    {
        final CountDownLatch eglCleanupBarrier = new CountDownLatch(1);

        synchronized (handlerLock) {
            if (renderThreadHandler == null) {
                Logging.d(TAG, getResourceName() + "Already released");
                return;
            }

            renderThreadHandler.postAtFrontOfQueue(new Runnable() {
                @Override public void run() {
                    drawer.release();
                    drawer = null;

                    if (yuvTextures != null) {
                        GLES20.glDeleteTextures(3, yuvTextures, 0);
                        yuvTextures = null;
                    }

                    makeBlack();
                    eglBase.release();
                    eglBase = null;
                    eglCleanupBarrier.countDown();
                }
            });
            renderThreadHandler = null;
        }
        ThreadUtils.awaitUninterruptibly(eglCleanupBarrier);
        renderThread.quit();
        synchronized (frameLock) {
            if (pendingFrame != null) {
                VideoRenderer.renderFrameDone(pendingFrame);
                pendingFrame = null;
            }
        }
        ThreadUtils.joinUninterruptibly(renderThread);
        renderThread = null;
        synchronized (layoutLock) {
            frameWidth     = 0;
            frameHeight    = 0;
            frameRotation  = 0;
            rendererEvents = null;
        }
        resetStatistics();
    } /* release */

    public void resetStatistics()
    {
        synchronized (statisticsLock) {
            framesReceived   = 0;
            framesDropped    = 0;
            framesRendered   = 0;
            firstFrameTimeNs = 0;
            renderTimeNs     = 0;
        }
    }

    public void setMirror(final boolean mirror)
    {
        synchronized (layoutLock) {
            this.mirror = mirror;
        }
    }

    public void setScalingType(RendererCommon.ScalingType scalingType)
    {
        synchronized (layoutLock) {
            this.scalingType = scalingType;
        }
    }

    @Override
    public void renderFrame(VideoRenderer.I420Frame frame)
    {
        synchronized (statisticsLock) {
            ++framesReceived;
        }
        synchronized (handlerLock) {
            if (renderThreadHandler == null) {
                Logging.d(TAG, getResourceName()
                          + "Dropping frame - Not initialized or already released.");
                VideoRenderer.renderFrameDone(frame);
                return;
            }

            synchronized (frameLock) {
                if (pendingFrame != null) {
                    synchronized (statisticsLock) {
                        ++framesDropped;
                    }
                    VideoRenderer.renderFrameDone(pendingFrame);
                }

                pendingFrame = frame;
                updateFrameDimensionsAndReportEvents(frame);
                renderThreadHandler.post(renderFrameRunnable);
            }
        }
    }

    private Point getDesiredLayoutSize(int widthSpec, int heightSpec)
    {
        synchronized (layoutLock) {
            final int maxWidth  = getDefaultSize(Integer.MAX_VALUE, widthSpec);
            final int maxHeight = getDefaultSize(Integer.MAX_VALUE, heightSpec);
            final Point size    =
                RendererCommon.getDisplaySize(scalingType, frameAspectRatio(), maxWidth, maxHeight);

            if (MeasureSpec.getMode(widthSpec) == MeasureSpec.EXACTLY) {
                size.x = maxWidth;
            }

            if (MeasureSpec.getMode(heightSpec) == MeasureSpec.EXACTLY) {
                size.y = maxHeight;
            }

            return size;
        }
    }

    @Override
    protected void onMeasure(int widthSpec, int heightSpec)
    {
        synchronized (layoutLock) {
            if (frameWidth == 0 || frameHeight == 0) {
                super.onMeasure(widthSpec, heightSpec);
                return;
            }

            desiredLayoutSize = getDesiredLayoutSize(widthSpec, heightSpec);

            if (desiredLayoutSize.x != getMeasuredWidth() || desiredLayoutSize.y != getMeasuredHeight()) {
                synchronized (handlerLock) {
                    if (renderThreadHandler != null) {
                        renderThreadHandler.postAtFrontOfQueue(makeBlackRunnable);
                    }
                }
            }

            setMeasuredDimension(desiredLayoutSize.x, desiredLayoutSize.y);
        }
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom)
    {
        synchronized (layoutLock) {
            layoutSize.x = right - left;
            layoutSize.y = bottom - top;
        }
        runOnRenderThread(renderFrameRunnable);
    }

    @Override
    public void surfaceCreated(final SurfaceHolder holder)
    {
        Logging.d(TAG, getResourceName() + "Surface created.");
        synchronized (layoutLock) {
            isSurfaceCreated = true;
        }
        tryCreateEglSurface();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        Logging.d(TAG, getResourceName() + "Surface destroyed.");
        synchronized (layoutLock) {
            isSurfaceCreated = false;
            surfaceSize.x    = 0;
            surfaceSize.y    = 0;
        }
        runOnRenderThread(new Runnable() {
            @Override public void run() {
                eglBase.releaseSurface();
            }
        });
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
    {
        Logging.d(TAG, getResourceName() + "Surface changed: " + width + "x" + height);
        synchronized (layoutLock) {
            surfaceSize.x = width;
            surfaceSize.y = height;
        }
        runOnRenderThread(renderFrameRunnable);
    }

    private void runOnRenderThread(Runnable runnable)
    {
        synchronized (handlerLock) {
            if (renderThreadHandler != null) {
                renderThreadHandler.post(runnable);
            }
        }
    }

    private String getResourceName()
    {
        try {
            return getResources().getResourceEntryName(getId()) + ": ";
        } catch (NotFoundException e) {
            return "";
        }
    }

    private void makeBlack()
    {
        if (Thread.currentThread() != renderThread) {
            throw new IllegalStateException(getResourceName() + "Wrong thread.");
        }

        if (eglBase != null && eglBase.hasSurface()) {
            GLES20.glClearColor(0, 0, 0, 0);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
            eglBase.swapBuffers();
        }
    }

    private boolean checkConsistentLayout()
    {
        if (Thread.currentThread() != renderThread) {
            throw new IllegalStateException(getResourceName() + "Wrong thread.");
        }

        synchronized (layoutLock) {
            return layoutSize.equals(desiredLayoutSize) && surfaceSize.equals(layoutSize);
        }
    }

    private void renderFrameOnRenderThread()
    {
        if (Thread.currentThread() != renderThread) {
            throw new IllegalStateException(getResourceName() + "Wrong thread.");
        }

        final VideoRenderer.I420Frame frame;
        synchronized (frameLock) {
            if (pendingFrame == null) {
                return;
            }

            frame        = pendingFrame;
            pendingFrame = null;
        }

        if (eglBase == null || !eglBase.hasSurface()) {
            Logging.d(TAG, getResourceName() + "No surface to draw on");
            VideoRenderer.renderFrameDone(frame);
            return;
        }

        if (!checkConsistentLayout()) {
            makeBlack();
            VideoRenderer.renderFrameDone(frame);
            return;
        }

        synchronized (layoutLock) {
            if (eglBase.surfaceWidth() != surfaceSize.x || eglBase.surfaceHeight() != surfaceSize.y) {
                makeBlack();
            }
        }
        final long startTimeNs = System.nanoTime();
        final float[] texMatrix;
        synchronized (layoutLock) {
            final float[] rotatedSamplingMatrix =
                RendererCommon.rotateTextureMatrix(frame.samplingMatrix, frame.rotationDegree);
            final float[] layoutMatrix = RendererCommon.getLayoutMatrix(
                mirror, frameAspectRatio(), (float) layoutSize.x / layoutSize.y);
            texMatrix = RendererCommon.multiplyMatrices(rotatedSamplingMatrix, layoutMatrix);
        }
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

        if (frame.yuvFrame) {
            if (yuvTextures == null) {
                yuvTextures = new int[3];

                for (int i = 0; i < 3; i++) {
                    yuvTextures[i] = GlUtil.generateTexture(GLES20.GL_TEXTURE_2D);
                }
            }

            yuvUploader.uploadYuvData(
                yuvTextures, frame.width, frame.height, frame.yuvStrides, frame.yuvPlanes);
            drawer.drawYuv(yuvTextures, texMatrix, frame.rotatedWidth(), frame.rotatedHeight(),
                           0, 0, surfaceSize.x, surfaceSize.y);
        } else {
            drawer.drawOes(frame.textureId, texMatrix, frame.rotatedWidth(), frame.rotatedHeight(),
                           0, 0, surfaceSize.x, surfaceSize.y);
        }

        eglBase.swapBuffers();
        VideoRenderer.renderFrameDone(frame);
        synchronized (statisticsLock) {
            if (framesRendered == 0) {
                firstFrameTimeNs = startTimeNs;
                synchronized (layoutLock) {
                    Logging.d(TAG, getResourceName() + "Reporting first rendered frame.");

                    if (rendererEvents != null) {
                        rendererEvents.onFirstFrameRendered();
                    }
                }
            }

            ++framesRendered;
            renderTimeNs += (System.nanoTime() - startTimeNs);

            if (framesRendered % 300 == 0) {
                logStatistics();
            }
        }
    } /* renderFrameOnRenderThread */

    private float frameAspectRatio()
    {
        synchronized (layoutLock) {
            if (frameWidth == 0 || frameHeight == 0) {
                return 0.0f;
            }

            return (frameRotation % 180 == 0) ? (float) frameWidth / frameHeight :
                   (float) frameHeight / frameWidth;
        }
    }

    private void updateFrameDimensionsAndReportEvents(VideoRenderer.I420Frame frame)
    {
        synchronized (layoutLock) {
            if (frameWidth != frame.width || frameHeight != frame.height ||
                frameRotation != frame.rotationDegree)
            {
                Logging.d(TAG, getResourceName() + "Reporting frame resolution changed to "
                          + frame.width + "x" + frame.height + " with rotation " + frame.rotationDegree);

                if (rendererEvents != null) {
                    rendererEvents.onFrameResolutionChanged(frame.width, frame.height, frame.rotationDegree);
                }

                frameWidth    = frame.width;
                frameHeight   = frame.height;
                frameRotation = frame.rotationDegree;
                post(new Runnable() {
                    @Override public void run() {
                        requestLayout();
                    }
                });
            }
        }
    }

    private void logStatistics()
    {
        synchronized (statisticsLock) {
            Logging.d(TAG, getResourceName() + "Frames received: "
                      + framesReceived + ". Dropped: " + framesDropped + ". Rendered: " + framesRendered);

            if (framesReceived > 0 && framesRendered > 0) {
                final long timeSinceFirstFrameNs = System.nanoTime() - firstFrameTimeNs;
                Logging.d(TAG, getResourceName() + "Duration: " + (int) (timeSinceFirstFrameNs / 1e6)
                          +" ms. FPS: " + framesRendered * 1e9 / timeSinceFirstFrameNs);
                Logging.d(TAG, getResourceName() + "Average render time: "
                          + (int) (renderTimeNs / (1000 * framesRendered)) + " us.");
            }
        }
    }
}
