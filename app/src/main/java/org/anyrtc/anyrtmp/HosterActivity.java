package org.anyrtc.anyrtmp;
import android.app.Activity;
import android.os.Bundle;
import android.text.SpannableString;
import android.view.View;
import android.widget.TextView;
import android.util.Log;
import org.anyrtc.core.AnyRTMP;
import org.anyrtc.core.RTMPHosterHelper;
import org.anyrtc.core.RTMPHosterKit;
import org.webrtc.SurfaceViewRenderer;
import org.webrtc.VideoRenderer;
public class HosterActivity extends Activity implements RTMPHosterHelper
{
    private RTMPHosterKit mHoster = null;
    private TextView mTxtStatus   = null;
    private SurfaceViewRenderer mSurfaceView = null;
    private VideoRenderer mRenderer = null;
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_hoster);
        {
            mTxtStatus   = (TextView) findViewById(R.id.txt_rtmp_status);
            mSurfaceView = (SurfaceViewRenderer) findViewById(R.id.suface_view);
            mSurfaceView.init(AnyRTMP.Inst().Egl().getEglBaseContext(), null);
            mRenderer = new VideoRenderer(mSurfaceView);
        }
        {
            String rtmpUrl = getIntent().getExtras().getString("rtmp_url");
            mHoster = new RTMPHosterKit(this, this);
            mHoster.SetVideoCapturer(mRenderer.GetRenderPointer(), true);
            mHoster.StartRtmpStream(rtmpUrl);
        }
    }

    @Override
    protected void onDestroy()
    {
        super.onDestroy();

        if (mHoster != null) {
            mHoster.StopRtmpStream();
            mHoster.Clear();
            mHoster = null;
        }
    }

    public void OnBtnClicked(View btn)
    {
        if (btn.getId() == R.id.btn_close) {
            if (mHoster != null) {
                mHoster.StopRtmpStream();
                mHoster.Clear();
                mHoster = null;
            }

            finish();
        } else if (btn.getId() == R.id.btn_switch_camera) {
            if (null != mHoster) {
                mHoster.SwitchCamera();
            }
        }
    }

    @Override
    public void OnRtmpStreamOK()
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTxtStatus.setText(R.string.str_rtmp_success);
            }
        });
    }

    @Override
    public void OnRtmpStreamReconnecting(final int times)
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTxtStatus.setText(String.format(getString(R.string.str_rtmp_reconnecting), times));
            }
        });
    }

    @Override
    public void OnRtmpStreamStatus(final int delayMs, final int netBand)
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTxtStatus.setText(String.format(getString(R.string.str_rtmp_status), delayMs, netBand));
            }
        });
    }

    @Override
    public void OnRtmpStreamFailed(final int code)
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTxtStatus.setText(R.string.str_rtmp_failed);
            }
        });
    }

    @Override
    public void OnRtmpStreamClosed()
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mTxtStatus.setText(R.string.str_rtmp);
            }
        });
    }
}