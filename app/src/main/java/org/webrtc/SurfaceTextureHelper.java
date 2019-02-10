package org.webrtc;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.SystemClock;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;
class SurfaceTextureHelper
{
    private static final String TAG = "SurfaceTextureHelper";
    public interface OnTextureFrameAvailableListener
    {
        abstract void onTextureFrameAvailable(
            int oesTextureId, float[] transformMatrix, long timestampNs);
    }
    public static SurfaceTextureHelper create(
        final String threadName, final EglBase.Context sharedContext)
    {
        final HandlerThread thread = new HandlerThread(threadName);

        thread.start();
        final Handler handler = new Handler(thread.getLooper());
        return ThreadUtils.invokeAtFrontUninterruptibly(handler, new Callable<SurfaceTextureHelper>() {
            @Override
            public SurfaceTextureHelper call() {
                try {
                    return new SurfaceTextureHelper(sharedContext, handler);
                } catch (RuntimeException e) {
                    Logging.e(TAG, threadName + " create failure", e);
                    return null;
                }
            }
        });
    }

    static private class YuvConverter
    {
        private final EglBase eglBase;
        private final GlShader shader;
        private boolean released = false;
        private static final FloatBuffer DEVICE_RECTANGLE =
            GlUtil.createFloatBuffer(new float[] {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f,
        });
        private static final FloatBuffer TEXTURE_RECTANGLE =
            GlUtil.createFloatBuffer(new float[] {
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f
        });
        private static final String VERTEX_SHADER =
            "varying vec2 interp_tc;\n"
            + "attribute vec4 in_pos;\n"
            + "attribute vec4 in_tc;\n"
            + "\n"
            + "uniform mat4 texMatrix;\n"
            + "\n"
            + "void main() {\n"
            + "    gl_Position = in_pos;\n"
            + "    interp_tc = (texMatrix * in_tc).xy;\n"
            + "}\n";
        private static final String FRAGMENT_SHADER =
            "#extension GL_OES_EGL_image_external : require\n"
            + "precision mediump float;\n"
            + "varying vec2 interp_tc;\n"
            + "\n"
            + "uniform samplerExternalOES oesTex;\n"
            + "uniform vec2 xUnit;\n"
            + "uniform vec4 coeffs;\n"
            + "\n"
            + "void main() {\n"
            + "  gl_FragColor.r = coeffs.a + dot(coeffs.rgb,\n"
            + "      texture2D(oesTex, interp_tc - 1.5 * xUnit).rgb);\n"
            + "  gl_FragColor.g = coeffs.a + dot(coeffs.rgb,\n"
            + "      texture2D(oesTex, interp_tc - 0.5 * xUnit).rgb);\n"
            + "  gl_FragColor.b = coeffs.a + dot(coeffs.rgb,\n"
            + "      texture2D(oesTex, interp_tc + 0.5 * xUnit).rgb);\n"
            + "  gl_FragColor.a = coeffs.a + dot(coeffs.rgb,\n"
            + "      texture2D(oesTex, interp_tc + 1.5 * xUnit).rgb);\n"
            + "}\n";
        private int texMatrixLoc;
        private int xUnitLoc;
        private int coeffsLoc;
        ;
        YuvConverter(EglBase.Context sharedContext)
        {
            eglBase = EglBase.create(sharedContext, EglBase.CONFIG_PIXEL_RGBA_BUFFER);
            eglBase.createDummyPbufferSurface();
            eglBase.makeCurrent();
            shader = new GlShader(VERTEX_SHADER, FRAGMENT_SHADER);
            shader.useProgram();
            texMatrixLoc = shader.getUniformLocation("texMatrix");
            xUnitLoc     = shader.getUniformLocation("xUnit");
            coeffsLoc    = shader.getUniformLocation("coeffs");
            GLES20.glUniform1i(shader.getUniformLocation("oesTex"), 0);
            GlUtil.checkNoGLES2Error("Initialize fragment shader uniform values.");
            shader.setVertexAttribArray("in_pos", 2, DEVICE_RECTANGLE);
            shader.setVertexAttribArray("in_tc", 2, TEXTURE_RECTANGLE);
            eglBase.detachCurrent();
        }

        synchronized void convert(ByteBuffer buf,
                                  int width, int height, int stride, int textureId, float [] transformMatrix)
        {
            if (released) {
                throw new IllegalStateException(
                          "YuvConverter.convert called on released object");
            }

            if (stride % 8 != 0) {
                throw new IllegalArgumentException(
                          "Invalid stride, must be a multiple of 8");
            }

            if (stride < width) {
                throw new IllegalArgumentException(
                          "Invalid stride, must >= width");
            }

            int y_width      = (width + 3) / 4;
            int uv_width     = (width + 7) / 8;
            int uv_height    = (height + 1) / 2;
            int total_height = height + uv_height;
            int size         = stride * total_height;

            if (buf.capacity() < size) {
                throw new IllegalArgumentException("YuvConverter.convert called with too small buffer");
            }

            transformMatrix =
                RendererCommon.multiplyMatrices(transformMatrix,
                                                RendererCommon.verticalFlipMatrix());

            if (eglBase.hasSurface()) {
                if (eglBase.surfaceWidth() != stride / 4 ||
                    eglBase.surfaceHeight() != total_height)
                {
                    eglBase.releaseSurface();
                    eglBase.createPbufferSurface(stride / 4, total_height);
                }
            } else {
                eglBase.createPbufferSurface(stride / 4, total_height);
            }

            eglBase.makeCurrent();
            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId);
            GLES20.glUniformMatrix4fv(texMatrixLoc, 1, false, transformMatrix, 0);
            GLES20.glViewport(0, 0, y_width, height);
            GLES20.glUniform2f(xUnitLoc,
                               transformMatrix[0] / width,
                               transformMatrix[1] / width);
            GLES20.glUniform4f(coeffsLoc, 0.299f, 0.587f, 0.114f, 0.0f);
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
            GLES20.glViewport(0, height, uv_width, uv_height);
            GLES20.glUniform2f(xUnitLoc,
                               2.0f * transformMatrix[0] / width,
                               2.0f * transformMatrix[1] / width);
            GLES20.glUniform4f(coeffsLoc, -0.169f, -0.331f, 0.499f, 0.5f);
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
            GLES20.glViewport(stride / 8, height, uv_width, uv_height);
            GLES20.glUniform4f(coeffsLoc, 0.499f, -0.418f, -0.0813f, 0.5f);
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
            GLES20.glReadPixels(0, 0, stride / 4, total_height, GLES20.GL_RGBA,
                                GLES20.GL_UNSIGNED_BYTE, buf);
            GlUtil.checkNoGLES2Error("YuvConverter.convert");
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, 0);
            eglBase.detachCurrent();
        } /* convert */

        synchronized void release()
        {
            released = true;
            eglBase.makeCurrent();
            shader.release();
            eglBase.release();
        }
    }
    private final Handler handler;
    private final EglBase eglBase;
    private final SurfaceTexture surfaceTexture;
    private final int oesTextureId;
    private YuvConverter yuvConverter;
    private OnTextureFrameAvailableListener listener;
    private boolean hasPendingTexture       = false;
    private volatile boolean isTextureInUse = false;
    private boolean isQuitting = false;
    private OnTextureFrameAvailableListener pendingListener;
    final Runnable setListenerRunnable = new Runnable()
    {
        @Override
        public void run()
        {
            Logging.d(TAG, "Setting listener to " + pendingListener);
            listener        = pendingListener;
            pendingListener = null;

            if (hasPendingTexture) {
                updateTexImage();
                hasPendingTexture = false;
            }
        }
    };
    private SurfaceTextureHelper(EglBase.Context sharedContext, Handler handler)
    {
        if (handler.getLooper().getThread() != Thread.currentThread()) {
            throw new IllegalStateException("SurfaceTextureHelper must be created on the handler thread");
        }

        this.handler = handler;
        eglBase      = EglBase.create(sharedContext, EglBase.CONFIG_PIXEL_BUFFER);

        try {
            eglBase.createDummyPbufferSurface();
            eglBase.makeCurrent();
        } catch (RuntimeException e) {
            eglBase.release();
            handler.getLooper().quit();
            throw e;
        }

        oesTextureId   = GlUtil.generateTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES);
        surfaceTexture = new SurfaceTexture(oesTextureId);
        surfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
            @Override
            public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                hasPendingTexture = true;
                tryDeliverTextureFrame();
            }
        });
    }

    private YuvConverter getYuvConverter()
    {
        if (yuvConverter != null) {
            return yuvConverter;
        }

        synchronized (this) {
            if (yuvConverter == null) {
                yuvConverter = new YuvConverter(eglBase.getEglBaseContext());
            }

            return yuvConverter;
        }
    }

    public void startListening(final OnTextureFrameAvailableListener listener)
    {
        if (this.listener != null || this.pendingListener != null) {
            throw new IllegalStateException("SurfaceTextureHelper listener has already been set.");
        }

        this.pendingListener = listener;
        handler.post(setListenerRunnable);
    }

    public void stopListening()
    {
        Logging.d(TAG, "stopListening()");
        handler.removeCallbacks(setListenerRunnable);
        ThreadUtils.invokeAtFrontUninterruptibly(handler, new Runnable() {
            @Override
            public void run() {
                listener        = null;
                pendingListener = null;
            }
        });
    }

    public SurfaceTexture getSurfaceTexture()
    {
        return surfaceTexture;
    }

    public Handler getHandler()
    {
        return handler;
    }

    public void returnTextureFrame()
    {
        handler.post(new Runnable() {
            @Override public void run() {
                isTextureInUse = false;

                if (isQuitting) {
                    release();
                } else {
                    tryDeliverTextureFrame();
                }
            }
        });
    }

    public boolean isTextureInUse()
    {
        return isTextureInUse;
    }

    public void dispose()
    {
        Logging.d(TAG, "dispose()");
        ThreadUtils.invokeAtFrontUninterruptibly(handler, new Runnable() {
            @Override
            public void run() {
                isQuitting = true;

                if (!isTextureInUse) {
                    release();
                }
            }
        });
    }

    public void textureToYUV(ByteBuffer buf,
                             int width, int height, int stride, int textureId, float [] transformMatrix)
    {
        if (textureId != oesTextureId) {
            throw new IllegalStateException("textureToByteBuffer called with unexpected textureId");
        }

        getYuvConverter().convert(buf, width, height, stride, textureId, transformMatrix);
    }

    private void updateTexImage()
    {
        synchronized (EglBase.lock) {
            surfaceTexture.updateTexImage();
        }
    }

    private void tryDeliverTextureFrame()
    {
        if (handler.getLooper().getThread() != Thread.currentThread()) {
            throw new IllegalStateException("Wrong thread.");
        }

        if (isQuitting || !hasPendingTexture || isTextureInUse || listener == null) {
            return;
        }

        isTextureInUse    = true;
        hasPendingTexture = false;
        updateTexImage();
        final float[] transformMatrix = new float[16];
        surfaceTexture.getTransformMatrix(transformMatrix);
        final long timestampNs = (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH) ?
                                 surfaceTexture.getTimestamp() :
                                 TimeUnit.MILLISECONDS.toNanos(SystemClock.elapsedRealtime());
        listener.onTextureFrameAvailable(oesTextureId, transformMatrix, timestampNs);
    }

    private void release()
    {
        if (handler.getLooper().getThread() != Thread.currentThread()) {
            throw new IllegalStateException("Wrong thread.");
        }

        if (isTextureInUse || !isQuitting) {
            throw new IllegalStateException("Unexpected release.");
        }

        synchronized (this) {
            if (yuvConverter != null) {
                yuvConverter.release();
            }
        }
        GLES20.glDeleteTextures(1, new int[] { oesTextureId }, 0);
        surfaceTexture.release();
        eglBase.release();
        handler.getLooper().quit();
    }
}
