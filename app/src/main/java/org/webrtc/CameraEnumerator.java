package org.webrtc;
public interface CameraEnumerator
{
    public String[] getDeviceNames();
    public boolean isFrontFacing(String deviceName);
    public boolean isBackFacing(String deviceName);
    public CameraVideoCapturer createCapturer(String                                  deviceName,
                                              CameraVideoCapturer.CameraEventsHandler eventsHandler);
}
