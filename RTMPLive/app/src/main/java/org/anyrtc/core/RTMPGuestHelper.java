package org.anyrtc.core;
public interface RTMPGuestHelper
{
    public void OnRtmplayerOK();
    public void OnRtmplayerStatus(int cacheTime, int curBitrate);
    public void OnRtmplayerCache(int time);
    public void OnRtmplayerClosed(int errcode);
}
