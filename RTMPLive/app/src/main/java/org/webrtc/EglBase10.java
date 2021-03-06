package org.webrtc;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.view.SurfaceHolder;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
final class EglBase10 extends EglBase
{
    private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    private final EGL10 egl;
    private EGLContext eglContext;
    private EGLConfig eglConfig;
    private EGLDisplay eglDisplay;
    private EGLSurface eglSurface = EGL10.EGL_NO_SURFACE;
    public static class Context extends EglBase.Context
    {
        private final EGLContext eglContext;
        public Context(EGLContext eglContext)
        {
            this.eglContext = eglContext;
        }
    }
    EglBase10(Context sharedContext, int[] configAttributes)
    {
        this.egl   = (EGL10) EGLContext.getEGL();
        eglDisplay = getEglDisplay();
        eglConfig  = getEglConfig(eglDisplay, configAttributes);
        eglContext = createEglContext(sharedContext, eglDisplay, eglConfig);
    }

    @Override
    public void createSurface(Surface surface)
    {
        class FakeSurfaceHolder implements SurfaceHolder
        {
            private final Surface surface;
            FakeSurfaceHolder(Surface surface)
            {
                this.surface = surface;
            }

            @Override
            public void addCallback(Callback callback) {}

            @Override
            public void removeCallback(Callback callback) {}

            @Override
            public boolean isCreating()
            {
                return false;
            }

            @Deprecated
            @Override
            public void setType(int i) {}

            @Override
            public void setFixedSize(int i, int i2) {}

            @Override
            public void setSizeFromLayout() {}

            @Override
            public void setFormat(int i) {}

            @Override
            public void setKeepScreenOn(boolean b) {}

            @Override
            public Canvas lockCanvas()
            {
                return null;
            }

            @Override
            public Canvas lockCanvas(Rect rect)
            {
                return null;
            }

            @Override
            public void unlockCanvasAndPost(Canvas canvas) {}

            @Override
            public Rect getSurfaceFrame()
            {
                return null;
            }

            @Override
            public Surface getSurface()
            {
                return surface;
            }
        }
        createSurfaceInternal(new FakeSurfaceHolder(surface));
    } /* createSurface */

    @Override
    public void createSurface(SurfaceTexture surfaceTexture)
    {
        createSurfaceInternal(surfaceTexture);
    }

    private void createSurfaceInternal(Object nativeWindow)
    {
        if (!(nativeWindow instanceof SurfaceHolder) && !(nativeWindow instanceof SurfaceTexture)) {
            throw new IllegalStateException("Input must be either a SurfaceHolder or SurfaceTexture");
        }

        checkIsNotReleased();

        if (eglSurface != EGL10.EGL_NO_SURFACE) {
            throw new RuntimeException("Already has an EGLSurface");
        }

        int[] surfaceAttribs = { EGL10.EGL_NONE };
        eglSurface = egl.eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, surfaceAttribs);

        if (eglSurface == EGL10.EGL_NO_SURFACE) {
            throw new RuntimeException("Failed to create window surface");
        }
    }

    @Override
    public void createDummyPbufferSurface()
    {
        createPbufferSurface(1, 1);
    }

    @Override
    public void createPbufferSurface(int width, int height)
    {
        checkIsNotReleased();

        if (eglSurface != EGL10.EGL_NO_SURFACE) {
            throw new RuntimeException("Already has an EGLSurface");
        }

        int[] surfaceAttribs = { EGL10.EGL_WIDTH, width, EGL10.EGL_HEIGHT, height, EGL10.EGL_NONE };
        eglSurface = egl.eglCreatePbufferSurface(eglDisplay, eglConfig, surfaceAttribs);

        if (eglSurface == EGL10.EGL_NO_SURFACE) {
            throw new RuntimeException(
                      "Failed to create pixel buffer surface with size: " + width + "x" + height);
        }
    }

    @Override
    public org.webrtc.EglBase.Context getEglBaseContext()
    {
        return new EglBase10.Context(eglContext);
    }

    @Override
    public boolean hasSurface()
    {
        return eglSurface != EGL10.EGL_NO_SURFACE;
    }

    @Override
    public int surfaceWidth()
    {
        final int widthArray[] = new int[1];

        egl.eglQuerySurface(eglDisplay, eglSurface, EGL10.EGL_WIDTH, widthArray);
        return widthArray[0];
    }

    @Override
    public int surfaceHeight()
    {
        final int heightArray[] = new int[1];

        egl.eglQuerySurface(eglDisplay, eglSurface, EGL10.EGL_HEIGHT, heightArray);
        return heightArray[0];
    }

    @Override
    public void releaseSurface()
    {
        if (eglSurface != EGL10.EGL_NO_SURFACE) {
            egl.eglDestroySurface(eglDisplay, eglSurface);
            eglSurface = EGL10.EGL_NO_SURFACE;
        }
    }

    private void checkIsNotReleased()
    {
        if (eglDisplay == EGL10.EGL_NO_DISPLAY || eglContext == EGL10.EGL_NO_CONTEXT ||
            eglConfig == null)
        {
            throw new RuntimeException("This object has been released");
        }
    }

    @Override
    public void release()
    {
        checkIsNotReleased();
        releaseSurface();
        detachCurrent();
        egl.eglDestroyContext(eglDisplay, eglContext);
        egl.eglTerminate(eglDisplay);
        eglContext = EGL10.EGL_NO_CONTEXT;
        eglDisplay = EGL10.EGL_NO_DISPLAY;
        eglConfig  = null;
    }

    @Override
    public void makeCurrent()
    {
        checkIsNotReleased();

        if (eglSurface == EGL10.EGL_NO_SURFACE) {
            throw new RuntimeException("No EGLSurface - can't make current");
        }

        synchronized (EglBase.lock) {
            if (!egl.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
                throw new RuntimeException("eglMakeCurrent failed");
            }
        }
    }

    @Override
    public void detachCurrent()
    {
        synchronized (EglBase.lock) {
            if (!egl.eglMakeCurrent(
                    eglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT))
            {
                throw new RuntimeException("eglDetachCurrent failed");
            }
        }
    }

    @Override
    public void swapBuffers()
    {
        checkIsNotReleased();

        if (eglSurface == EGL10.EGL_NO_SURFACE) {
            throw new RuntimeException("No EGLSurface - can't swap buffers");
        }

        synchronized (EglBase.lock) {
            egl.eglSwapBuffers(eglDisplay, eglSurface);
        }
    }

    private EGLDisplay getEglDisplay()
    {
        EGLDisplay eglDisplay = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

        if (eglDisplay == EGL10.EGL_NO_DISPLAY) {
            throw new RuntimeException("Unable to get EGL10 display");
        }

        int[] version = new int[2];

        if (!egl.eglInitialize(eglDisplay, version)) {
            throw new RuntimeException("Unable to initialize EGL10");
        }

        return eglDisplay;
    }

    private EGLConfig getEglConfig(EGLDisplay eglDisplay, int[] configAttributes)
    {
        EGLConfig[] configs = new EGLConfig[1];
        int[] numConfigs    = new int[1];

        if (!egl.eglChooseConfig(
                eglDisplay, configAttributes, configs, configs.length, numConfigs))
        {
            throw new RuntimeException("eglChooseConfig failed");
        }

        if (numConfigs[0] <= 0) {
            throw new RuntimeException("Unable to find any matching EGL config");
        }

        final EGLConfig eglConfig = configs[0];

        if (eglConfig == null) {
            throw new RuntimeException("eglChooseConfig returned null");
        }

        return eglConfig;
    }

    private EGLContext createEglContext(
        Context sharedContext, EGLDisplay eglDisplay, EGLConfig eglConfig)
    {
        if (sharedContext != null && sharedContext.eglContext == EGL10.EGL_NO_CONTEXT) {
            throw new RuntimeException("Invalid sharedContext");
        }

        int[] contextAttributes = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
        EGLContext rootContext  =
            sharedContext == null ? EGL10.EGL_NO_CONTEXT : sharedContext.eglContext;
        final EGLContext eglContext;
        synchronized (EglBase.lock) {
            eglContext = egl.eglCreateContext(eglDisplay, eglConfig, rootContext, contextAttributes);
        }

        if (eglContext == EGL10.EGL_NO_CONTEXT) {
            throw new RuntimeException("Failed to create EGL context");
        }

        return eglContext;
    }
}
