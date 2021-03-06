package org.webrtc;
import android.graphics.SurfaceTexture;
import android.view.Surface;
import javax.microedition.khronos.egl.EGL10;
public abstract class EglBase
{
    public static class Context
    {}
    public static final Object lock = new Object();
    private static final int EGL_OPENGL_ES2_BIT     = 4;
    private static final int EGL_RECORDABLE_ANDROID = 0x3142;
    public static final int[] CONFIG_PLAIN = {
        EGL10.EGL_RED_SIZE,        8,
        EGL10.EGL_GREEN_SIZE,      8,
        EGL10.EGL_BLUE_SIZE,       8,
        EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL10.EGL_NONE
    };
    public static final int[] CONFIG_RGBA = {
        EGL10.EGL_RED_SIZE,        8,
        EGL10.EGL_GREEN_SIZE,      8,
        EGL10.EGL_BLUE_SIZE,       8,
        EGL10.EGL_ALPHA_SIZE,      8,
        EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL10.EGL_NONE
    };
    public static final int[] CONFIG_PIXEL_BUFFER = {
        EGL10.EGL_RED_SIZE,        8,
        EGL10.EGL_GREEN_SIZE,      8,
        EGL10.EGL_BLUE_SIZE,       8,
        EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL10.EGL_SURFACE_TYPE,    EGL10.EGL_PBUFFER_BIT,
        EGL10.EGL_NONE
    };
    public static final int[] CONFIG_PIXEL_RGBA_BUFFER = {
        EGL10.EGL_RED_SIZE,        8,
        EGL10.EGL_GREEN_SIZE,      8,
        EGL10.EGL_BLUE_SIZE,       8,
        EGL10.EGL_ALPHA_SIZE,      8,
        EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL10.EGL_SURFACE_TYPE,    EGL10.EGL_PBUFFER_BIT,
        EGL10.EGL_NONE
    };
    public static final int[] CONFIG_RECORDABLE = {
        EGL10.EGL_RED_SIZE,        8,
        EGL10.EGL_GREEN_SIZE,      8,
        EGL10.EGL_BLUE_SIZE,       8,
        EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RECORDABLE_ANDROID,    1,
        EGL10.EGL_NONE
    };
    public static EglBase create(Context sharedContext, int[] configAttributes)
    {
        return (EglBase14.isEGL14Supported() &&
                (sharedContext == null || sharedContext instanceof EglBase14.Context)) ?
               new EglBase14((EglBase14.Context) sharedContext, configAttributes) :
               new EglBase10((EglBase10.Context) sharedContext, configAttributes);
    }

    public static EglBase create()
    {
        return create(null, CONFIG_PLAIN);
    }

    public static EglBase create(Context sharedContext)
    {
        return create(sharedContext, CONFIG_PLAIN);
    }

    public abstract void createSurface(Surface surface);
    public abstract void createSurface(SurfaceTexture surfaceTexture);
    public abstract void createDummyPbufferSurface();
    public abstract void createPbufferSurface(int width, int height);
    public abstract Context getEglBaseContext();
    public abstract boolean hasSurface();
    public abstract int surfaceWidth();
    public abstract int surfaceHeight();
    public abstract void releaseSurface();
    public abstract void release();
    public abstract void makeCurrent();
    public abstract void detachCurrent();
    public abstract void swapBuffers();
}
