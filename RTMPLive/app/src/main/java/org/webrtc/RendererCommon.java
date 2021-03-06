package org.webrtc;
import android.graphics.Point;
import android.opengl.GLES20;
import android.opengl.Matrix;
import java.nio.ByteBuffer;
public class RendererCommon
{
    public static interface RendererEvents
    {
        public void onFirstFrameRendered();
        public void onFrameResolutionChanged(int videoWidth, int videoHeight, int rotation);
    }
    public static interface GlDrawer
    {
        void drawOes(int oesTextureId, float[] texMatrix, int frameWidth, int frameHeight,
                     int viewportX, int viewportY, int viewportWidth, int viewportHeight);
        void drawRgb(int textureId, float[] texMatrix, int frameWidth, int frameHeight,
                     int viewportX, int viewportY, int viewportWidth, int viewportHeight);
        void drawYuv(int[] yuvTextures, float[] texMatrix, int frameWidth, int frameHeight,
                     int viewportX, int viewportY, int viewportWidth, int viewportHeight);
        void release();
    }
    public static class YuvUploader
    {
        private ByteBuffer copyBuffer;
        public void uploadYuvData(
            int[] outputYuvTextures, int width, int height, int[] strides, ByteBuffer[] planes)
        {
            final int[] planeWidths  = new int[] { width, width / 2, width / 2 };
            final int[] planeHeights = new int[] { height, height / 2, height / 2 };
            int copyCapacityNeeded   = 0;

            for (int i = 0; i < 3; ++i) {
                if (strides[i] > planeWidths[i]) {
                    copyCapacityNeeded = Math.max(copyCapacityNeeded, planeWidths[i] * planeHeights[i]);
                }
            }

            if (copyCapacityNeeded > 0 &&
                (copyBuffer == null || copyBuffer.capacity() < copyCapacityNeeded))
            {
                copyBuffer = ByteBuffer.allocateDirect(copyCapacityNeeded);
            }

            for (int i = 0; i < 3; ++i) {
                GLES20.glActiveTexture(GLES20.GL_TEXTURE0 + i);
                GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, outputYuvTextures[i]);
                final ByteBuffer packedByteBuffer;

                if (strides[i] == planeWidths[i]) {
                    packedByteBuffer = planes[i];
                } else {
                    VideoRenderer.nativeCopyPlane(
                        planes[i], planeWidths[i], planeHeights[i], strides[i], copyBuffer, planeWidths[i]);
                    packedByteBuffer = copyBuffer;
                }

                GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, planeWidths[i],
                                    planeHeights[i], 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, packedByteBuffer);
            }
        } /* uploadYuvData */
    }
    public static enum ScalingType { SCALE_ASPECT_FIT, SCALE_ASPECT_FILL, SCALE_ASPECT_BALANCED }
    private static float BALANCED_VISIBLE_FRACTION = 0.5625f;
    public static final float[] identityMatrix()
    {
        return new float[] {
                   1, 0, 0, 0,
                   0, 1, 0, 0,
                   0, 0, 1, 0,
                   0, 0, 0, 1
        };
    }

    public static final float[] verticalFlipMatrix()
    {
        return new float[] {
                   1, 0, 0, 0,
                   0, -1, 0, 0,
                   0, 0, 1, 0,
                   0, 1, 0, 1
        };
    }

    public static final float[] horizontalFlipMatrix()
    {
        return new float[] {
                   -1, 0, 0, 0,
                   0, 1, 0, 0,
                   0, 0, 1, 0,
                   1, 0, 0, 1
        };
    }

    public static float[] rotateTextureMatrix(float[] textureMatrix, float rotationDegree)
    {
        final float[] rotationMatrix = new float[16];

        Matrix.setRotateM(rotationMatrix, 0, rotationDegree, 0, 0, 1);
        adjustOrigin(rotationMatrix);
        return multiplyMatrices(textureMatrix, rotationMatrix);
    }

    public static float[] multiplyMatrices(float[] a, float[] b)
    {
        final float[] resultMatrix = new float[16];

        Matrix.multiplyMM(resultMatrix, 0, a, 0, b, 0);
        return resultMatrix;
    }

    public static float[] getLayoutMatrix(
        boolean mirror, float videoAspectRatio, float displayAspectRatio)
    {
        float scaleX = 1;
        float scaleY = 1;

        if (displayAspectRatio > videoAspectRatio) {
            scaleY = videoAspectRatio / displayAspectRatio;
        } else {
            scaleX = displayAspectRatio / videoAspectRatio;
        }

        if (mirror) {
            scaleX *= -1;
        }

        final float matrix[] = new float[16];
        Matrix.setIdentityM(matrix, 0);
        Matrix.scaleM(matrix, 0, scaleX, scaleY, 1);
        adjustOrigin(matrix);
        return matrix;
    }

    public static Point getDisplaySize(ScalingType scalingType, float videoAspectRatio,
                                       int maxDisplayWidth, int maxDisplayHeight)
    {
        return getDisplaySize(convertScalingTypeToVisibleFraction(scalingType), videoAspectRatio,
                              maxDisplayWidth, maxDisplayHeight);
    }

    private static void adjustOrigin(float[] matrix)
    {
        matrix[12] -= 0.5f * (matrix[0] + matrix[4]);
        matrix[13] -= 0.5f * (matrix[1] + matrix[5]);
        matrix[12] += 0.5f;
        matrix[13] += 0.5f;
    }

    private static float convertScalingTypeToVisibleFraction(ScalingType scalingType)
    {
        switch (scalingType) {
            case SCALE_ASPECT_FIT:
                return 1.0f;

            case SCALE_ASPECT_FILL:
                return 0.0f;

            case SCALE_ASPECT_BALANCED:
                return BALANCED_VISIBLE_FRACTION;

            default:
                throw new IllegalArgumentException();
        }
    }

    private static Point getDisplaySize(float minVisibleFraction, float videoAspectRatio,
                                        int maxDisplayWidth, int maxDisplayHeight)
    {
        if (minVisibleFraction == 0 || videoAspectRatio == 0) {
            return new Point(maxDisplayWidth, maxDisplayHeight);
        }

        final int width = Math.min(maxDisplayWidth,
                                   Math.round(maxDisplayHeight / minVisibleFraction * videoAspectRatio));
        final int height = Math.min(maxDisplayHeight,
                                    Math.round(maxDisplayWidth / minVisibleFraction / videoAspectRatio));
        return new Point(width, height);
    }
}
