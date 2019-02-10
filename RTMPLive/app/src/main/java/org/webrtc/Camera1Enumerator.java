package org.webrtc;
import android.os.SystemClock;
import org.webrtc.CameraEnumerationAndroid.CaptureFormat;
import java.util.ArrayList;
import java.util.List;
@SuppressWarnings("deprecation")
public class Camera1Enumerator implements CameraEnumerator
{
    private final static String TAG = "Camera1Enumerator";
    private static List<List<CaptureFormat> > cachedSupportedFormats;
    private final boolean captureToTexture;
    public Camera1Enumerator()
    {
        this(true);
    }

    public Camera1Enumerator(boolean captureToTexture)
    {
        this.captureToTexture = captureToTexture;
    }

    @Override
    public String[] getDeviceNames()
    {
        String[] names = new String[android.hardware.Camera.getNumberOfCameras()];

        for (int i = 0; i < android.hardware.Camera.getNumberOfCameras(); ++i) {
            names[i] = getDeviceName(i);
        }

        return names;
    }

    @Override
    public boolean isFrontFacing(String deviceName)
    {
        android.hardware.Camera.CameraInfo info = getCameraInfo(getCameraIndex(deviceName));
        return info.facing == android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT;
    }

    @Override
    public boolean isBackFacing(String deviceName)
    {
        android.hardware.Camera.CameraInfo info = getCameraInfo(getCameraIndex(deviceName));
        return info.facing == android.hardware.Camera.CameraInfo.CAMERA_FACING_BACK;
    }

    @Override
    public CameraVideoCapturer createCapturer(String                                  deviceName,
                                              CameraVideoCapturer.CameraEventsHandler eventsHandler)
    {
        return new VideoCapturerAndroid(deviceName, eventsHandler, captureToTexture);
    }

    private static android.hardware.Camera.CameraInfo getCameraInfo(int index)
    {
        android.hardware.Camera.CameraInfo info = new android.hardware.Camera.CameraInfo();

        try {
            android.hardware.Camera.getCameraInfo(index, info);
        } catch (Exception e) {
            Logging.e(TAG, "getCameraInfo failed on index " + index, e);
            return null;
        }

        return info;
    }

    static synchronized List<CaptureFormat> getSupportedFormats(int cameraId)
    {
        if (cachedSupportedFormats == null) {
            cachedSupportedFormats = new ArrayList<List<CaptureFormat> >();

            for (int i = 0; i < CameraEnumerationAndroid.getDeviceCount(); ++i) {
                cachedSupportedFormats.add(enumerateFormats(i));
            }
        }

        return cachedSupportedFormats.get(cameraId);
    }

    private static List<CaptureFormat> enumerateFormats(int cameraId)
    {
        Logging.d(TAG, "Get supported formats for camera index " + cameraId + ".");
        final long startTimeMs = SystemClock.elapsedRealtime();
        final android.hardware.Camera.Parameters parameters;
        android.hardware.Camera camera = null;

        try {
            Logging.d(TAG, "Opening camera with index " + cameraId);
            camera     = android.hardware.Camera.open(cameraId);
            parameters = camera.getParameters();
        } catch (RuntimeException e) {
            Logging.e(TAG, "Open camera failed on camera index " + cameraId, e);
            return new ArrayList<CaptureFormat>();
        }

        finally {
            if (camera != null) {
                camera.release();
            }
        }
        final List<CaptureFormat> formatList = new ArrayList<CaptureFormat>();

        try {
            int minFps = 0;
            int maxFps = 0;
            final List<int[]> listFpsRange = parameters.getSupportedPreviewFpsRange();

            if (listFpsRange != null) {
                final int[] range = listFpsRange.get(listFpsRange.size() - 1);
                minFps = range[android.hardware.Camera.Parameters.PREVIEW_FPS_MIN_INDEX];
                maxFps = range[android.hardware.Camera.Parameters.PREVIEW_FPS_MAX_INDEX];
            }

            for (android.hardware.Camera.Size size : parameters.getSupportedPreviewSizes()) {
                formatList.add(new CaptureFormat(size.width, size.height, minFps, maxFps));
            }
        } catch (Exception e) {
            Logging.e(TAG, "getSupportedFormats() failed on camera index " + cameraId, e);
        }

        final long endTimeMs = SystemClock.elapsedRealtime();
        Logging.d(TAG, "Get supported formats for camera index " + cameraId + " done."
                  + " Time spent: " + (endTimeMs - startTimeMs) + " ms.");
        return formatList;
    } /* enumerateFormats */

    public static List<Size> convertSizes(List<android.hardware.Camera.Size> cameraSizes)
    {
        final List<Size> sizes = new ArrayList<Size>();

        for (android.hardware.Camera.Size size : cameraSizes) {
            sizes.add(new Size(size.width, size.height));
        }

        return sizes;
    }

    public static List<CaptureFormat.FramerateRange> convertFramerates(List<int[]> arrayRanges)
    {
        final List<CaptureFormat.FramerateRange> ranges = new ArrayList<CaptureFormat.FramerateRange>();

        for (int[] range : arrayRanges) {
            ranges.add(new CaptureFormat.FramerateRange(
                           range[android.hardware.Camera.Parameters.PREVIEW_FPS_MIN_INDEX],
                           range[android.hardware.Camera.Parameters.PREVIEW_FPS_MAX_INDEX]));
        }

        return ranges;
    }

    static int getCameraIndex(String deviceName)
    {
        Logging.d(TAG, "getCameraIndex: " + deviceName);

        for (int i = 0; i < android.hardware.Camera.getNumberOfCameras(); ++i) {
            if (deviceName.equals(CameraEnumerationAndroid.getDeviceName(i))) {
                return i;
            }
        }

        throw new IllegalArgumentException("No such camera: " + deviceName);
    }

    static String getDeviceName(int index)
    {
        android.hardware.Camera.CameraInfo info = getCameraInfo(index);
        String facing =
            (info.facing == android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT) ? "front" : "back";
        return "Camera " + index + ", Facing " + facing
               + ", Orientation " + info.orientation;
    }
}
