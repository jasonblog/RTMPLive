package org.webrtc;
import android.content.Context;
import android.os.Handler;
import android.os.SystemClock;
import android.view.Surface;
import android.view.WindowManager;
import org.webrtc.CameraEnumerationAndroid.CaptureFormat;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
@SuppressWarnings("deprecation")
public class VideoCapturerAndroid implements
    CameraVideoCapturer,
    android.hardware.Camera.PreviewCallback,
    SurfaceTextureHelper.OnTextureFrameAvailableListener
{
    private final static String TAG = "VideoCapturerAndroid";
    private static final int CAMERA_STOP_TIMEOUT_MS = 7000;
    private android.hardware.Camera camera;
    private final Object handlerLock = new Object();
    private Handler cameraThreadHandler;
    private Context applicationContext;
    private final Object cameraIdLock = new Object();
    private int id;
    private android.hardware.Camera.CameraInfo info;
    private CameraStatistics cameraStatistics;
    private int requestedWidth;
    private int requestedHeight;
    private int requestedFramerate;
    private CaptureFormat captureFormat;
    private final Object pendingCameraSwitchLock = new Object();
    private volatile boolean pendingCameraSwitch;
    private CapturerObserver frameObserver = null;
    private final CameraEventsHandler eventsHandler;
    private boolean firstFrameReported;
    private static final int NUMBER_OF_CAPTURE_BUFFERS = 3;
    private final Set<byte[]> queuedBuffers = new HashSet<byte[]>();
    private final boolean isCapturingToTexture;
    private SurfaceTextureHelper surfaceHelper;
    private final static int MAX_OPEN_CAMERA_ATTEMPTS = 3;
    private final static int OPEN_CAMERA_DELAY_MS     = 500;
    private int openCameraAttempts;
    private final android.hardware.Camera.ErrorCallback cameraErrorCallback =
        new android.hardware.Camera.ErrorCallback()
    {
        @Override
        public void onError(int error, android.hardware.Camera camera)
        {
            String errorMessage;

            if (error == android.hardware.Camera.CAMERA_ERROR_SERVER_DIED) {
                errorMessage = "Camera server died!";
            } else {
                errorMessage = "Camera error: " + error;
            }

            Logging.e(TAG, errorMessage);

            if (eventsHandler != null) {
                eventsHandler.onCameraError(errorMessage);
            }
        }
    };
    public static VideoCapturerAndroid create(String              name,
                                              CameraEventsHandler eventsHandler)
    {
        return VideoCapturerAndroid.create(name, eventsHandler, false);
    }

    @Deprecated
    public static VideoCapturerAndroid create(String name,
                                              CameraEventsHandler eventsHandler, boolean captureToTexture)
    {
        try {
            return new VideoCapturerAndroid(name, eventsHandler, captureToTexture);
        } catch (RuntimeException e) {
            Logging.e(TAG, "Couldn't create camera.", e);
            return null;
        }
    }

    public void printStackTrace()
    {
        Thread cameraThread = null;

        synchronized (handlerLock) {
            if (cameraThreadHandler != null) {
                cameraThread = cameraThreadHandler.getLooper().getThread();
            }
        }

        if (cameraThread != null) {
            StackTraceElement[] cameraStackTraces = cameraThread.getStackTrace();

            if (cameraStackTraces.length > 0) {
                Logging.d(TAG, "VideoCapturerAndroid stacks trace:");

                for (StackTraceElement stackTrace : cameraStackTraces) {
                    Logging.d(TAG, stackTrace.toString());
                }
            }
        }
    }

    @Override
    public void switchCamera(final CameraSwitchHandler switchEventsHandler)
    {
        if (android.hardware.Camera.getNumberOfCameras() < 2) {
            if (switchEventsHandler != null) {
                switchEventsHandler.onCameraSwitchError("No camera to switch to.");
            }

            return;
        }

        synchronized (pendingCameraSwitchLock) {
            if (pendingCameraSwitch) {
                Logging.w(TAG, "Ignoring camera switch request.");

                if (switchEventsHandler != null) {
                    switchEventsHandler.onCameraSwitchError("Pending camera switch already in progress.");
                }

                return;
            }

            pendingCameraSwitch = true;
        }
        final boolean didPost = maybePostOnCameraThread(new Runnable() {
            @Override
            public void run() {
                switchCameraOnCameraThread();
                synchronized (pendingCameraSwitchLock) {
                    pendingCameraSwitch = false;
                }

                if (switchEventsHandler != null) {
                    switchEventsHandler.onCameraSwitchDone(
                        info.facing == android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT);
                }
            }
        });

        if (!didPost && switchEventsHandler != null) {
            switchEventsHandler.onCameraSwitchError("Camera is stopped.");
        }
    } /* switchCamera */

    @Override
    public void onOutputFormatRequest(final int width, final int height, final int framerate)
    {
        maybePostOnCameraThread(new Runnable() {
            @Override public void run() {
                onOutputFormatRequestOnCameraThread(width, height, framerate);
            }
        });
    }

    @Override
    public void changeCaptureFormat(final int width, final int height, final int framerate)
    {
        maybePostOnCameraThread(new Runnable() {
            @Override public void run() {
                startPreviewOnCameraThread(width, height, framerate);
            }
        });
    }

    private int getCurrentCameraId()
    {
        synchronized (cameraIdLock) {
            return id;
        }
    }

    @Override
    public List<CaptureFormat> getSupportedFormats()
    {
        return Camera1Enumerator.getSupportedFormats(getCurrentCameraId());
    }

    public boolean isCapturingToTexture()
    {
        return isCapturingToTexture;
    }

    public VideoCapturerAndroid(String cameraName, CameraEventsHandler eventsHandler,
                                boolean captureToTexture)
    {
        if (android.hardware.Camera.getNumberOfCameras() == 0) {
            throw new RuntimeException("No cameras available");
        }

        if (cameraName == null || cameraName.equals("")) {
            this.id = 0;
        } else {
            this.id = Camera1Enumerator.getCameraIndex(cameraName);
        }

        this.eventsHandler   = eventsHandler;
        isCapturingToTexture = captureToTexture;
        Logging.d(TAG, "VideoCapturerAndroid isCapturingToTexture : " + isCapturingToTexture);
    }

    private void checkIsOnCameraThread()
    {
        synchronized (handlerLock) {
            if (cameraThreadHandler == null) {
                Logging.e(TAG, "Camera is stopped - can't check thread.");
            } else if (Thread.currentThread() != cameraThreadHandler.getLooper().getThread()) {
                throw new IllegalStateException("Wrong thread");
            }
        }
    }

    private boolean maybePostOnCameraThread(Runnable runnable)
    {
        return maybePostDelayedOnCameraThread(0, runnable);
    }

    private boolean maybePostDelayedOnCameraThread(int delayMs, Runnable runnable)
    {
        synchronized (handlerLock) {
            return cameraThreadHandler != null &&
                   cameraThreadHandler.postAtTime(
                runnable, this, SystemClock.uptimeMillis() + delayMs);
        }
    }

    @Override
    public void dispose()
    {
        Logging.d(TAG, "dispose");
    }

    @Override
    public void startCapture(
        final int width, final int height, final int framerate,
        final SurfaceTextureHelper surfaceTextureHelper, final Context applicationContext,
        final CapturerObserver frameObserver)
    {
        Logging.d(TAG, "startCapture requested: " + width + "x" + height + "@" + framerate);

        if (surfaceTextureHelper == null) {
            frameObserver.onCapturerStarted(false);

            if (eventsHandler != null) {
                eventsHandler.onCameraError("No SurfaceTexture created.");
            }

            return;
        }

        if (applicationContext == null) {
            throw new IllegalArgumentException("applicationContext not set.");
        }

        if (frameObserver == null) {
            throw new IllegalArgumentException("frameObserver not set.");
        }

        synchronized (handlerLock) {
            if (this.cameraThreadHandler != null) {
                throw new RuntimeException("Camera has already been started.");
            }

            this.cameraThreadHandler = surfaceTextureHelper.getHandler();
            this.surfaceHelper       = surfaceTextureHelper;
            final boolean didPost = maybePostOnCameraThread(new Runnable() {
                @Override
                public void run() {
                    openCameraAttempts = 0;
                    startCaptureOnCameraThread(width, height, framerate, frameObserver,
                                               applicationContext);
                }
            });

            if (!didPost) {
                frameObserver.onCapturerStarted(false);

                if (eventsHandler != null) {
                    eventsHandler.onCameraError("Could not post task to camera thread.");
                }
            }
        }
    } /* startCapture */

    private void startCaptureOnCameraThread(
        final int width, final int height, final int framerate, final CapturerObserver frameObserver,
        final Context applicationContext)
    {
        synchronized (handlerLock) {
            if (cameraThreadHandler == null) {
                Logging.e(TAG, "startCaptureOnCameraThread: Camera is stopped");
                return;
            } else {
                checkIsOnCameraThread();
            }
        }

        if (camera != null) {
            Logging.e(TAG, "startCaptureOnCameraThread: Camera has already been started.");
            return;
        }

        this.applicationContext = applicationContext;
        this.frameObserver      = frameObserver;
        this.firstFrameReported = false;

        try {
            try {
                synchronized (cameraIdLock) {
                    Logging.d(TAG, "Opening camera " + id);

                    if (eventsHandler != null) {
                        eventsHandler.onCameraOpening(id);
                    }

                    camera = android.hardware.Camera.open(id);
                    info   = new android.hardware.Camera.CameraInfo();
                    android.hardware.Camera.getCameraInfo(id, info);
                }
            } catch (RuntimeException e) {
                openCameraAttempts++;

                if (openCameraAttempts < MAX_OPEN_CAMERA_ATTEMPTS) {
                    Logging.e(TAG, "Camera.open failed, retrying", e);
                    maybePostDelayedOnCameraThread(OPEN_CAMERA_DELAY_MS, new Runnable() {
                        @Override public void run() {
                            startCaptureOnCameraThread(width, height, framerate, frameObserver,
                                                       applicationContext);
                        }
                    });
                    return;
                }

                throw e;
            }

            camera.setPreviewTexture(surfaceHelper.getSurfaceTexture());
            Logging.d(TAG, "Camera orientation: " + info.orientation
                      +" .Device orientation: " + getDeviceOrientation());
            camera.setErrorCallback(cameraErrorCallback);
            startPreviewOnCameraThread(width, height, framerate);
            frameObserver.onCapturerStarted(true);

            if (isCapturingToTexture) {
                surfaceHelper.startListening(this);
            }

            cameraStatistics = new CameraStatistics(surfaceHelper, eventsHandler);
        } catch (IOException | RuntimeException e) {
            Logging.e(TAG, "startCapture failed", e);
            stopCaptureOnCameraThread(true);
            frameObserver.onCapturerStarted(false);

            if (eventsHandler != null) {
                eventsHandler.onCameraError("Camera can not be started.");
            }
        }
    } /* startCaptureOnCameraThread */

    private void startPreviewOnCameraThread(int width, int height, int framerate)
    {
        synchronized (handlerLock) {
            if (cameraThreadHandler == null || camera == null) {
                Logging.e(TAG, "startPreviewOnCameraThread: Camera is stopped");
                return;
            } else {
                checkIsOnCameraThread();
            }
        }
        Logging.d(
            TAG, "startPreviewOnCameraThread requested: " + width + "x" + height + "@" + framerate);
        requestedWidth     = width;
        requestedHeight    = height;
        requestedFramerate = framerate;
        final android.hardware.Camera.Parameters parameters = camera.getParameters();
        final List<CaptureFormat.FramerateRange> supportedFramerates =
            Camera1Enumerator.convertFramerates(parameters.getSupportedPreviewFpsRange());
        Logging.d(TAG, "Available fps ranges: " + supportedFramerates);
        final CaptureFormat.FramerateRange fpsRange =
            CameraEnumerationAndroid.getClosestSupportedFramerateRange(supportedFramerates, framerate);
        final Size previewSize = CameraEnumerationAndroid.getClosestSupportedSize(
            Camera1Enumerator.convertSizes(parameters.getSupportedPreviewSizes()), width, height);
        final CaptureFormat captureFormat =
            new CaptureFormat(previewSize.width, previewSize.height, fpsRange);

        if (captureFormat.equals(this.captureFormat)) {
            return;
        }

        Logging.d(TAG, "isVideoStabilizationSupported: "
                  +parameters.isVideoStabilizationSupported());

        if (parameters.isVideoStabilizationSupported()) {
            parameters.setVideoStabilization(true);
        }

        if (captureFormat.framerate.max > 0) {
            parameters.setPreviewFpsRange(captureFormat.framerate.min, captureFormat.framerate.max);
        }

        parameters.setPreviewSize(previewSize.width, previewSize.height);

        if (!isCapturingToTexture) {
            parameters.setPreviewFormat(captureFormat.imageFormat);
        }

        final Size pictureSize = CameraEnumerationAndroid.getClosestSupportedSize(
            Camera1Enumerator.convertSizes(parameters.getSupportedPictureSizes()), width, height);
        parameters.setPictureSize(pictureSize.width, pictureSize.height);

        if (this.captureFormat != null) {
            camera.stopPreview();
            camera.setPreviewCallbackWithBuffer(null);
        }

        Logging.d(TAG, "Start capturing: " + captureFormat);
        this.captureFormat = captureFormat;
        List<String> focusModes = parameters.getSupportedFocusModes();

        if (focusModes.contains(android.hardware.Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO)) {
            parameters.setFocusMode(android.hardware.Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
        }

        camera.setParameters(parameters);
        camera.setDisplayOrientation(0);

        if (!isCapturingToTexture) {
            queuedBuffers.clear();
            final int frameSize = captureFormat.frameSize();

            for (int i = 0; i < NUMBER_OF_CAPTURE_BUFFERS; ++i) {
                final ByteBuffer buffer = ByteBuffer.allocateDirect(frameSize);
                queuedBuffers.add(buffer.array());
                camera.addCallbackBuffer(buffer.array());
            }

            camera.setPreviewCallbackWithBuffer(this);
        }

        camera.startPreview();
    } /* startPreviewOnCameraThread */

    @Override
    public void stopCapture() throws InterruptedException
    {
        Logging.d(TAG, "stopCapture");
        final CountDownLatch barrier = new CountDownLatch(1);
        final boolean didPost        = maybePostOnCameraThread(new Runnable()
        {
            @Override public void run() {
                stopCaptureOnCameraThread(true);
                barrier.countDown();
            }
        });

        if (!didPost) {
            Logging.e(TAG, "Calling stopCapture() for already stopped camera.");
            return;
        }

        if (!barrier.await(CAMERA_STOP_TIMEOUT_MS, TimeUnit.MILLISECONDS)) {
            Logging.e(TAG, "Camera stop timeout");
            printStackTrace();

            if (eventsHandler != null) {
                eventsHandler.onCameraError("Camera stop timeout");
            }
        }

        Logging.d(TAG, "stopCapture done");
    }

    private void stopCaptureOnCameraThread(boolean stopHandler)
    {
        synchronized (handlerLock) {
            if (cameraThreadHandler == null) {
                Logging.e(TAG, "stopCaptureOnCameraThread: Camera is stopped");
            } else {
                checkIsOnCameraThread();
            }
        }
        Logging.d(TAG, "stopCaptureOnCameraThread");

        if (surfaceHelper != null) {
            surfaceHelper.stopListening();
        }

        if (stopHandler) {
            synchronized (handlerLock) {
                if (cameraThreadHandler != null) {
                    cameraThreadHandler.removeCallbacksAndMessages(this);
                    cameraThreadHandler = null;
                }

                surfaceHelper = null;
            }
        }

        if (cameraStatistics != null) {
            cameraStatistics.release();
            cameraStatistics = null;
        }

        Logging.d(TAG, "Stop preview.");

        if (camera != null) {
            camera.stopPreview();
            camera.setPreviewCallbackWithBuffer(null);
        }

        queuedBuffers.clear();
        captureFormat = null;
        Logging.d(TAG, "Release camera.");

        if (camera != null) {
            camera.release();
            camera = null;
        }

        if (eventsHandler != null) {
            eventsHandler.onCameraClosed();
        }

        Logging.d(TAG, "stopCaptureOnCameraThread done");
    } /* stopCaptureOnCameraThread */

    private void switchCameraOnCameraThread()
    {
        synchronized (handlerLock) {
            if (cameraThreadHandler == null) {
                Logging.e(TAG, "switchCameraOnCameraThread: Camera is stopped");
                return;
            } else {
                checkIsOnCameraThread();
            }
        }
        Logging.d(TAG, "switchCameraOnCameraThread");
        stopCaptureOnCameraThread(false);
        synchronized (cameraIdLock) {
            id = (id + 1) % android.hardware.Camera.getNumberOfCameras();
        }
        startCaptureOnCameraThread(requestedWidth, requestedHeight, requestedFramerate, frameObserver,
                                   applicationContext);
        Logging.d(TAG, "switchCameraOnCameraThread done");
    }

    private void onOutputFormatRequestOnCameraThread(int width, int height, int framerate)
    {
        synchronized (handlerLock) {
            if (cameraThreadHandler == null || camera == null) {
                Logging.e(TAG, "onOutputFormatRequestOnCameraThread: Camera is stopped");
                return;
            } else {
                checkIsOnCameraThread();
            }
        }
        Logging.d(TAG, "onOutputFormatRequestOnCameraThread: " + width + "x" + height
                  +"@" + framerate);
        frameObserver.onOutputFormatRequest(width, height, framerate);
    }

    Handler getCameraThreadHandler()
    {
        return cameraThreadHandler;
    }

    private int getDeviceOrientation()
    {
        int orientation  = 0;
        WindowManager wm = (WindowManager) applicationContext.getSystemService(
            Context.WINDOW_SERVICE);

        switch (wm.getDefaultDisplay().getRotation()) {
            case Surface.ROTATION_90:
                orientation = 90;
                break;

            case Surface.ROTATION_180:
                orientation = 180;
                break;

            case Surface.ROTATION_270:
                orientation = 270;
                break;

            case Surface.ROTATION_0:
            default:
                orientation = 0;
                break;
        }

        return orientation;
    }

    private int getFrameOrientation()
    {
        int rotation = getDeviceOrientation();

        if (info.facing == android.hardware.Camera.CameraInfo.CAMERA_FACING_BACK) {
            rotation = 360 - rotation;
        }

        return (info.orientation + rotation) % 360;
    }

    @Override
    public void onPreviewFrame(byte[] data, android.hardware.Camera callbackCamera)
    {
        synchronized (handlerLock) {
            if (cameraThreadHandler == null) {
                Logging.e(TAG, "onPreviewFrame: Camera is stopped");
                return;
            } else {
                checkIsOnCameraThread();
            }
        }

        if (!queuedBuffers.contains(data)) {
            return;
        }

        if (camera != callbackCamera) {
            throw new RuntimeException("Unexpected camera in callback!");
        }

        final long captureTimeNs =
            TimeUnit.MILLISECONDS.toNanos(SystemClock.elapsedRealtime());

        if (eventsHandler != null && !firstFrameReported) {
            eventsHandler.onFirstFrameAvailable();
            firstFrameReported = true;
        }

        cameraStatistics.addFrame();
        frameObserver.onByteBufferFrameCaptured(data, captureFormat.width, captureFormat.height,
                                                getFrameOrientation(), captureTimeNs);
        camera.addCallbackBuffer(data);
    }

    @Override
    public void onTextureFrameAvailable(
        int oesTextureId, float[] transformMatrix, long timestampNs)
    {
        synchronized (handlerLock) {
            if (cameraThreadHandler == null) {
                Logging.e(TAG, "onTextureFrameAvailable: Camera is stopped");
                surfaceHelper.returnTextureFrame();
                return;
            } else {
                checkIsOnCameraThread();
            }
        }

        if (eventsHandler != null && !firstFrameReported) {
            eventsHandler.onFirstFrameAvailable();
            firstFrameReported = true;
        }

        int rotation = getFrameOrientation();

        if (info.facing == android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT) {
            transformMatrix =
                RendererCommon.multiplyMatrices(transformMatrix, RendererCommon.horizontalFlipMatrix());
        }

        cameraStatistics.addFrame();
        frameObserver.onTextureFrameCaptured(captureFormat.width, captureFormat.height, oesTextureId,
                                             transformMatrix, rotation, timestampNs);
    }
}
