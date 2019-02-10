package org.anyrtc.core;
import android.app.Activity;
import org.webrtc.EglBase;
import android.util.Log;
public class RTMPGuestKit
{
    private static final String TAG = "RTMPGuestKit";
    private long fNativeAppId;
    private Activity mActivity;
    private RTMPGuestHelper mGuestHelper;
    private final LooperExecutor mExecutor;
    private final EglBase mEglBase;
    public RTMPGuestKit(Activity act, final RTMPGuestHelper guestHelper)
    {
        RTMPUtils.assertIsTrue(act != null && guestHelper != null);
        mActivity    = act;
        mGuestHelper = guestHelper;
        AnyRTMP.Inst().Init(mActivity.getApplicationContext());
        mExecutor = AnyRTMP.Inst().Executor();
        mEglBase  = AnyRTMP.Inst().Egl();
        mExecutor.execute(new Runnable() {
            @Override
            public void run() {
                fNativeAppId = nativeCreate(guestHelper);
            }
        });
    }

    public void Clear()
    {
        mExecutor.execute(new Runnable() {
            @Override
            public void run() {
                nativeStopRtmpPlay();
                nativeDestroy();
            }
        });
    }

    public void StartRtmpPlay(final String strUrl, final long renderPointer)
    {
        mExecutor.execute(new Runnable() {
            @Override
            public void run() {
                nativeStartRtmpPlay(strUrl, renderPointer);
            }
        });
    }

    public void StopRtmpPlay()
    {
        mExecutor.execute(new Runnable() {
            @Override
            public void run() {
                nativeStopRtmpPlay();
            }
        });
    }

    private native long nativeCreate(Object obj);
    private native void nativeStartRtmpPlay(final String strUrl, final long renderPointer);
    private native void nativeStopRtmpPlay();
    private native void nativeDestroy();
}
