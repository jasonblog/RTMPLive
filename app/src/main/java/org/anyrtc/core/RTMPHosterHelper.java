package org.anyrtc.core;
public interface RTMPHosterHelper
{
    public void OnRtmpStreamOK();
    public void OnRtmpStreamReconnecting(int times);
    public void OnRtmpStreamStatus(int delayMs, int netBand);
    public void OnRtmpStreamFailed(int code);
    public void OnRtmpStreamClosed();
}
