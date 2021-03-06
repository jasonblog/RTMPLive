package org.webrtc;
import android.annotation.SuppressLint;
import android.graphics.Point;
import android.graphics.Rect;
import android.opengl.EGL14;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import org.webrtc.VideoRenderer.I420Frame;
import java.util.ArrayList;
import java.util.concurrent.CountDownLatch;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.opengles.GL10;
public class VideoRendererGui implements GLSurfaceView.Renderer
{
    private static VideoRendererGui instance = null;
    private static Runnable eglContextReady  = null;
    private static final String TAG = "VideoRendererGui";
    private GLSurfaceView surface;
    private static EglBase.Context eglContext = null;
    private boolean onSurfaceCreatedCalled;
    private int screenWidth;
    private int screenHeight;
    private final ArrayList<YuvImageRenderer> yuvImageRenderers;
    private static Thread renderFrameThread;
    private static Thread drawThread;
    private VideoRendererGui(GLSurfaceView surface)
    {
        this.surface = surface;
        surface.setPreserveEGLContextOnPause(true);
        surface.setEGLContextClientVersion(2);
        surface.setRenderer(this);
        surface.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        yuvImageRenderers = new ArrayList<YuvImageRenderer>();
    }

    private static class YuvImageRenderer implements VideoRenderer.Callbacks
    {
        private GLSurfaceView surface;
        private int id;
        private int[] yuvTextures = { 0, 0, 0 };
        private final RendererCommon.YuvUploader yuvUploader = new RendererCommon.YuvUploader();
        private final RendererCommon.GlDrawer drawer;
        private GlTextureFrameBuffer textureCopy;
        private I420Frame pendingFrame;
        private final Object pendingFrameLock = new Object();
        private static enum RendererType { RENDERER_YUV, RENDERER_TEXTURE };
        private RendererType rendererType;
        private RendererCommon.ScalingType scalingType;
        private boolean mirror;
        private RendererCommon.RendererEvents rendererEvents;
        boolean seenFrame;
        private int framesReceived;
        private int framesDropped;
        private int framesRendered;
        private long startTimeNs = -1;
        private long drawTimeNs;
        private long copyTimeNs;
        private final Rect layoutInPercentage;
        private final Rect displayLayout = new Rect();
        private float[] layoutMatrix;
        private boolean updateLayoutProperties;
        private final Object updateLayoutLock = new Object();
        private float[] rotatedSamplingMatrix;
        private int screenWidth;
        private int screenHeight;
        private int videoWidth;
        private int videoHeight;
        private int rotationDegree;
        private YuvImageRenderer(
            GLSurfaceView surface, int id,
            int x, int y, int width, int height,
            RendererCommon.ScalingType scalingType, boolean mirror, RendererCommon.GlDrawer drawer)
        {
            Logging.d(TAG, "YuvImageRenderer.Create id: " + id);
            this.surface           = surface;
            this.id                = id;
            this.scalingType       = scalingType;
            this.mirror            = mirror;
            this.drawer            = drawer;
            layoutInPercentage     = new Rect(x, y, Math.min(100, x + width), Math.min(100, y + height));
            updateLayoutProperties = false;
            rotationDegree         = 0;
        }

        public synchronized void reset()
        {
            seenFrame = false;
        }

        private synchronized void release()
        {
            surface = null;
            drawer.release();
            synchronized (pendingFrameLock) {
                if (pendingFrame != null) {
                    VideoRenderer.renderFrameDone(pendingFrame);
                    pendingFrame = null;
                }
            }
        }

        private void createTextures()
        {
            Logging.d(TAG, "  YuvImageRenderer.createTextures " + id + " on GL thread:"
                      +Thread.currentThread().getId());

            for (int i = 0; i < 3; i++) {
                yuvTextures[i] = GlUtil.generateTexture(GLES20.GL_TEXTURE_2D);
            }

            textureCopy = new GlTextureFrameBuffer(GLES20.GL_RGB);
        }

        private void updateLayoutMatrix()
        {
            synchronized (updateLayoutLock) {
                if (!updateLayoutProperties) {
                    return;
                }

                displayLayout.set(
                    (screenWidth * layoutInPercentage.left + 99) / 100,
                    (screenHeight * layoutInPercentage.top + 99) / 100,
                    (screenWidth * layoutInPercentage.right) / 100,
                    (screenHeight * layoutInPercentage.bottom) / 100);
                Logging.d(TAG, "ID: "  + id + ". AdjustTextureCoords. Allowed display size: "
                          + displayLayout.width() + " x " + displayLayout.height() + ". Video: " + videoWidth
                          + " x " + videoHeight + ". Rotation: " + rotationDegree + ". Mirror: " + mirror);
                final float videoAspectRatio = (rotationDegree % 180 == 0) ?
                                               (float) videoWidth / videoHeight :
                                               (float) videoHeight / videoWidth;
                final Point displaySize = RendererCommon.getDisplaySize(scalingType,
                                                                        videoAspectRatio,
                                                                        displayLayout.width(), displayLayout.height());
                displayLayout.inset((displayLayout.width() - displaySize.x) / 2,
                                    (displayLayout.height() - displaySize.y) / 2);
                Logging.d(TAG, "  Adjusted display size: " + displayLayout.width() + " x "
                          + displayLayout.height());
                layoutMatrix = RendererCommon.getLayoutMatrix(
                    mirror, videoAspectRatio, (float) displayLayout.width() / displayLayout.height());
                updateLayoutProperties = false;
                Logging.d(TAG, "  AdjustTextureCoords done");
            }
        }

        private void draw()
        {
            if (!seenFrame) {
                return;
            }

            long now = System.nanoTime();
            final boolean isNewFrame;
            synchronized (pendingFrameLock) {
                isNewFrame = (pendingFrame != null);

                if (isNewFrame && startTimeNs == -1) {
                    startTimeNs = now;
                }

                if (isNewFrame) {
                    rotatedSamplingMatrix = RendererCommon.rotateTextureMatrix(
                        pendingFrame.samplingMatrix, pendingFrame.rotationDegree);

                    if (pendingFrame.yuvFrame) {
                        rendererType = RendererType.RENDERER_YUV;
                        yuvUploader.uploadYuvData(yuvTextures, pendingFrame.width, pendingFrame.height,
                                                  pendingFrame.yuvStrides, pendingFrame.yuvPlanes);
                    } else {
                        rendererType = RendererType.RENDERER_TEXTURE;
                        textureCopy.setSize(pendingFrame.rotatedWidth(), pendingFrame.rotatedHeight());
                        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, textureCopy.getFrameBufferId());
                        GlUtil.checkNoGLES2Error("glBindFramebuffer");
                        drawer.drawOes(pendingFrame.textureId, rotatedSamplingMatrix,
                                       textureCopy.getWidth(), textureCopy.getHeight(),
                                       0, 0, textureCopy.getWidth(), textureCopy.getHeight());
                        rotatedSamplingMatrix = RendererCommon.identityMatrix();
                        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
                        GLES20.glFinish();
                    }

                    copyTimeNs += (System.nanoTime() - now);
                    VideoRenderer.renderFrameDone(pendingFrame);
                    pendingFrame = null;
                }
            }
            updateLayoutMatrix();
            final float[] texMatrix =
                RendererCommon.multiplyMatrices(rotatedSamplingMatrix, layoutMatrix);
            final int viewportY = screenHeight - displayLayout.bottom;

            if (rendererType == RendererType.RENDERER_YUV) {
                drawer.drawYuv(yuvTextures, texMatrix, videoWidth, videoHeight,
                               displayLayout.left, viewportY, displayLayout.width(), displayLayout.height());
            } else {
                drawer.drawRgb(textureCopy.getTextureId(), texMatrix, videoWidth, videoHeight,
                               displayLayout.left, viewportY, displayLayout.width(), displayLayout.height());
            }

            if (isNewFrame) {
                framesRendered++;
                drawTimeNs += (System.nanoTime() - now);

                if ((framesRendered % 300) == 0) {
                    logStatistics();
                }
            }
        } /* draw */

        private void logStatistics()
        {
            long timeSinceFirstFrameNs = System.nanoTime() - startTimeNs;

            Logging.d(TAG, "ID: " + id + ". Type: " + rendererType
                      +". Frames received: " + framesReceived
                      +". Dropped: " + framesDropped + ". Rendered: " + framesRendered);

            if (framesReceived > 0 && framesRendered > 0) {
                Logging.d(TAG, "Duration: " + (int) (timeSinceFirstFrameNs / 1e6)
                          +" ms. FPS: " + framesRendered * 1e9 / timeSinceFirstFrameNs);
                Logging.d(TAG, "Draw time: "
                          +(int) (drawTimeNs / (1000 * framesRendered)) + " us. Copy time: "
                          +(int) (copyTimeNs / (1000 * framesReceived)) + " us");
            }
        }

        public void setScreenSize(final int screenWidth, final int screenHeight)
        {
            synchronized (updateLayoutLock) {
                if (screenWidth == this.screenWidth && screenHeight == this.screenHeight) {
                    return;
                }

                Logging.d(TAG, "ID: " + id + ". YuvImageRenderer.setScreenSize: "
                          +screenWidth + " x " + screenHeight);
                this.screenWidth       = screenWidth;
                this.screenHeight      = screenHeight;
                updateLayoutProperties = true;
            }
        }

        public void setPosition(int x, int y, int width, int height,
                                RendererCommon.ScalingType scalingType, boolean mirror)
        {
            final Rect layoutInPercentage =
                new Rect(x, y, Math.min(100, x + width), Math.min(100, y + height));

            synchronized (updateLayoutLock) {
                if (layoutInPercentage.equals(this.layoutInPercentage) && scalingType == this.scalingType &&
                    mirror == this.mirror)
                {
                    return;
                }

                Logging.d(TAG, "ID: " + id + ". YuvImageRenderer.setPosition: (" + x + ", " + y
                          +") " +  width + " x " + height + ". Scaling: " + scalingType
                          +". Mirror: " + mirror);
                this.layoutInPercentage.set(layoutInPercentage);
                this.scalingType       = scalingType;
                this.mirror            = mirror;
                updateLayoutProperties = true;
            }
        }

        private void setSize(final int videoWidth, final int videoHeight, final int rotation)
        {
            if (videoWidth == this.videoWidth && videoHeight == this.videoHeight &&
                rotation == rotationDegree)
            {
                return;
            }

            if (rendererEvents != null) {
                Logging.d(TAG, "ID: " + id
                          +". Reporting frame resolution changed to " + videoWidth + " x " + videoHeight);
                rendererEvents.onFrameResolutionChanged(videoWidth, videoHeight, rotation);
            }

            synchronized (updateLayoutLock) {
                Logging.d(TAG, "ID: " + id + ". YuvImageRenderer.setSize: "
                          +videoWidth + " x " + videoHeight + " rotation " + rotation);
                this.videoWidth        = videoWidth;
                this.videoHeight       = videoHeight;
                rotationDegree         = rotation;
                updateLayoutProperties = true;
                Logging.d(TAG, "  YuvImageRenderer.setSize done.");
            }
        }

        @Override
        public synchronized void renderFrame(I420Frame frame)
        {
            if (surface == null) {
                VideoRenderer.renderFrameDone(frame);
                return;
            }

            if (renderFrameThread == null) {
                renderFrameThread = Thread.currentThread();
            }

            if (!seenFrame && rendererEvents != null) {
                Logging.d(TAG, "ID: " + id + ". Reporting first rendered frame.");
                rendererEvents.onFirstFrameRendered();
            }

            framesReceived++;
            synchronized (pendingFrameLock) {
                if (frame.yuvFrame) {
                    if (frame.yuvStrides[0] < frame.width ||
                        frame.yuvStrides[1] < frame.width / 2 ||
                        frame.yuvStrides[2] < frame.width / 2)
                    {
                        Logging.e(TAG, "Incorrect strides " + frame.yuvStrides[0] + ", "
                                  +frame.yuvStrides[1] + ", " + frame.yuvStrides[2]);
                        VideoRenderer.renderFrameDone(frame);
                        return;
                    }
                }

                if (pendingFrame != null) {
                    framesDropped++;
                    VideoRenderer.renderFrameDone(frame);
                    seenFrame = true;
                    return;
                }

                pendingFrame = frame;
            }
            setSize(frame.width, frame.height, frame.rotationDegree);
            seenFrame = true;
            surface.requestRender();
        } /* renderFrame */
    }
    public static synchronized void setView(GLSurfaceView surface,
                                            Runnable      eglContextReadyCallback)
    {
        Logging.d(TAG, "VideoRendererGui.setView");
        instance        = new VideoRendererGui(surface);
        eglContextReady = eglContextReadyCallback;
    }

    public static synchronized EglBase.Context getEglBaseContext()
    {
        return eglContext;
    }

    public static synchronized void dispose()
    {
        if (instance == null) {
            return;
        }

        Logging.d(TAG, "VideoRendererGui.dispose");
        synchronized (instance.yuvImageRenderers) {
            for (YuvImageRenderer yuvImageRenderer : instance.yuvImageRenderers) {
                yuvImageRenderer.release();
            }

            instance.yuvImageRenderers.clear();
        }
        renderFrameThread = null;
        drawThread        = null;
        instance.surface  = null;
        eglContext        = null;
        eglContextReady   = null;
        instance = null;
    }

    public static VideoRenderer createGui(int x, int y, int width, int height,
                                          RendererCommon.ScalingType scalingType, boolean mirror) throws Exception
    {
        YuvImageRenderer javaGuiRenderer = create(
            x, y, width, height, scalingType, mirror);

        return new VideoRenderer(javaGuiRenderer);
    }

    public static VideoRenderer.Callbacks createGuiRenderer(
        int x, int y, int width, int height,
        RendererCommon.ScalingType scalingType, boolean mirror)
    {
        return create(x, y, width, height, scalingType, mirror);
    }

    public static synchronized YuvImageRenderer create(int x, int y, int width, int height,
                                                       RendererCommon.ScalingType scalingType, boolean mirror)
    {
        return create(x, y, width, height, scalingType, mirror, new GlRectDrawer());
    }

    public static synchronized YuvImageRenderer create(int x, int y, int width, int height,
                                                       RendererCommon.ScalingType scalingType, boolean mirror,
                                                       RendererCommon.GlDrawer drawer)
    {
        if (x < 0 || x > 100 || y < 0 || y > 100 ||
            width < 0 || width > 100 || height < 0 || height > 100 ||
            x + width > 100 || y + height > 100)
        {
            throw new RuntimeException("Incorrect window parameters.");
        }

        if (instance == null) {
            throw new RuntimeException(
                      "Attempt to create yuv renderer before setting GLSurfaceView");
        }

        final YuvImageRenderer yuvImageRenderer = new YuvImageRenderer(
            instance.surface, instance.yuvImageRenderers.size(),
            x, y, width, height, scalingType, mirror, drawer);
        synchronized (instance.yuvImageRenderers) {
            if (instance.onSurfaceCreatedCalled) {
                final CountDownLatch countDownLatch = new CountDownLatch(1);
                instance.surface.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        yuvImageRenderer.createTextures();
                        yuvImageRenderer.setScreenSize(
                            instance.screenWidth, instance.screenHeight);
                        countDownLatch.countDown();
                    }
                });

                try {
                    countDownLatch.await();
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }

            instance.yuvImageRenderers.add(yuvImageRenderer);
        }
        return yuvImageRenderer;
    } /* create */

    public static synchronized void update(
        VideoRenderer.Callbacks renderer, int x, int y, int width, int height,
        RendererCommon.ScalingType scalingType, boolean mirror)
    {
        Logging.d(TAG, "VideoRendererGui.update");

        if (instance == null) {
            throw new RuntimeException(
                      "Attempt to update yuv renderer before setting GLSurfaceView");
        }

        synchronized (instance.yuvImageRenderers) {
            for (YuvImageRenderer yuvImageRenderer : instance.yuvImageRenderers) {
                if (yuvImageRenderer == renderer) {
                    yuvImageRenderer.setPosition(x, y, width, height, scalingType, mirror);
                }
            }
        }
    }

    public static synchronized void setRendererEvents(
        VideoRenderer.Callbacks renderer, RendererCommon.RendererEvents rendererEvents)
    {
        Logging.d(TAG, "VideoRendererGui.setRendererEvents");

        if (instance == null) {
            throw new RuntimeException(
                      "Attempt to set renderer events before setting GLSurfaceView");
        }

        synchronized (instance.yuvImageRenderers) {
            for (YuvImageRenderer yuvImageRenderer : instance.yuvImageRenderers) {
                if (yuvImageRenderer == renderer) {
                    yuvImageRenderer.rendererEvents = rendererEvents;
                }
            }
        }
    }

    public static synchronized void remove(VideoRenderer.Callbacks renderer)
    {
        Logging.d(TAG, "VideoRendererGui.remove");

        if (instance == null) {
            throw new RuntimeException(
                      "Attempt to remove renderer before setting GLSurfaceView");
        }

        synchronized (instance.yuvImageRenderers) {
            final int index = instance.yuvImageRenderers.indexOf(renderer);

            if (index == -1) {
                Logging.w(TAG, "Couldn't remove renderer (not present in current list)");
            } else {
                instance.yuvImageRenderers.remove(index).release();
            }
        }
    }

    public static synchronized void reset(VideoRenderer.Callbacks renderer)
    {
        Logging.d(TAG, "VideoRendererGui.reset");

        if (instance == null) {
            throw new RuntimeException(
                      "Attempt to reset renderer before setting GLSurfaceView");
        }

        synchronized (instance.yuvImageRenderers) {
            for (YuvImageRenderer yuvImageRenderer : instance.yuvImageRenderers) {
                if (yuvImageRenderer == renderer) {
                    yuvImageRenderer.reset();
                }
            }
        }
    }

    private static void printStackTrace(Thread thread, String threadName)
    {
        if (thread != null) {
            StackTraceElement[] stackTraces = thread.getStackTrace();

            if (stackTraces.length > 0) {
                Logging.d(TAG, threadName + " stacks trace:");

                for (StackTraceElement stackTrace : stackTraces) {
                    Logging.d(TAG, stackTrace.toString());
                }
            }
        }
    }

    public static synchronized void printStackTraces()
    {
        if (instance == null) {
            return;
        }

        printStackTrace(renderFrameThread, "Render frame thread");
        printStackTrace(drawThread, "Draw thread");
    }

    @SuppressLint("NewApi")
    @Override
    public void onSurfaceCreated(GL10 unused, EGLConfig config)
    {
        Logging.d(TAG, "VideoRendererGui.onSurfaceCreated");
        synchronized (VideoRendererGui.class) {
            if (EglBase14.isEGL14Supported()) {
                eglContext = new EglBase14.Context(EGL14.eglGetCurrentContext());
            } else {
                eglContext = new EglBase10.Context(((EGL10) EGLContext.getEGL()).eglGetCurrentContext());
            }

            Logging.d(TAG, "VideoRendererGui EGL Context: " + eglContext);
        }
        synchronized (yuvImageRenderers) {
            for (YuvImageRenderer yuvImageRenderer : yuvImageRenderers) {
                yuvImageRenderer.createTextures();
            }

            onSurfaceCreatedCalled = true;
        }
        GlUtil.checkNoGLES2Error("onSurfaceCreated done");
        GLES20.glPixelStorei(GLES20.GL_UNPACK_ALIGNMENT, 1);
        GLES20.glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        synchronized (VideoRendererGui.class) {
            if (eglContextReady != null) {
                eglContextReady.run();
            }
        }
    }

    @Override
    public void onSurfaceChanged(GL10 unused, int width, int height)
    {
        Logging.d(TAG, "VideoRendererGui.onSurfaceChanged: "
                  +width + " x " + height + "  ");
        screenWidth  = width;
        screenHeight = height;
        synchronized (yuvImageRenderers) {
            for (YuvImageRenderer yuvImageRenderer : yuvImageRenderers) {
                yuvImageRenderer.setScreenSize(screenWidth, screenHeight);
            }
        }
    }

    @Override
    public void onDrawFrame(GL10 unused)
    {
        if (drawThread == null) {
            drawThread = Thread.currentThread();
        }

        GLES20.glViewport(0, 0, screenWidth, screenHeight);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        synchronized (yuvImageRenderers) {
            for (YuvImageRenderer yuvImageRenderer : yuvImageRenderers) {
                yuvImageRenderer.draw();
            }
        }
    }
}
