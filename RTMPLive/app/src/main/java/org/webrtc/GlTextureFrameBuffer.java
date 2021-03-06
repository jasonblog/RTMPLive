package org.webrtc;
import android.opengl.GLES20;
public class GlTextureFrameBuffer
{
    private final int frameBufferId;
    private final int textureId;
    private final int pixelFormat;
    private int width;
    private int height;
    public GlTextureFrameBuffer(int pixelFormat)
    {
        switch (pixelFormat) {
            case GLES20.GL_LUMINANCE:
            case GLES20.GL_RGB:
            case GLES20.GL_RGBA:
                this.pixelFormat = pixelFormat;
                break;

            default:
                throw new IllegalArgumentException("Invalid pixel format: " + pixelFormat);
        }

        textureId   = GlUtil.generateTexture(GLES20.GL_TEXTURE_2D);
        this.width  = 0;
        this.height = 0;
        final int frameBuffers[] = new int[1];
        GLES20.glGenFramebuffers(1, frameBuffers, 0);
        frameBufferId = frameBuffers[0];
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, frameBufferId);
        GlUtil.checkNoGLES2Error("Generate framebuffer");
        GLES20.glFramebufferTexture2D(GLES20.GL_FRAMEBUFFER, GLES20.GL_COLOR_ATTACHMENT0,
                                      GLES20.GL_TEXTURE_2D, textureId, 0);
        GlUtil.checkNoGLES2Error("Attach texture to framebuffer");
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
    }

    public void setSize(int width, int height)
    {
        if (width == 0 || height == 0) {
            throw new IllegalArgumentException("Invalid size: " + width + "x" + height);
        }

        if (width == this.width && height == this.height) {
            return;
        }

        this.width  = width;
        this.height = height;
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, frameBufferId);
        GlUtil.checkNoGLES2Error("glBindFramebuffer");
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, pixelFormat, width, height, 0, pixelFormat,
                            GLES20.GL_UNSIGNED_BYTE, null);
        final int status = GLES20.glCheckFramebufferStatus(GLES20.GL_FRAMEBUFFER);

        if (status != GLES20.GL_FRAMEBUFFER_COMPLETE) {
            throw new IllegalStateException("Framebuffer not complete, status: " + status);
        }

        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
    }

    public int getWidth()
    {
        return width;
    }

    public int getHeight()
    {
        return height;
    }

    public int getFrameBufferId()
    {
        return frameBufferId;
    }

    public int getTextureId()
    {
        return textureId;
    }

    public void release()
    {
        GLES20.glDeleteTextures(1, new int[] { textureId }, 0);
        GLES20.glDeleteFramebuffers(1, new int[] { frameBufferId }, 0);
        width  = 0;
        height = 0;
    }
}
