package org.anyrtc.anyrtmp;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;
import android.util.Log;
import org.anyrtc.core.AnyRTMP;
import static android.Manifest.permission.CAMERA;
import static android.Manifest.permission.RECORD_AUDIO;
import static android.content.pm.PackageManager.PERMISSION_DENIED;
import static android.content.pm.PackageManager.PERMISSION_GRANTED;

public class MainActivity extends AppCompatActivity
{
    private EditText mEditRtmpUrl;
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        {
            mEditRtmpUrl = (EditText) findViewById(R.id.edit_rtmp_url);
        }
        AnyRTMP.Inst();
        requestPermission(CAMERA, RECORD_AUDIO);
    }

    public void OnBtnClicked(View view)
    {
        String rtmpUrl = "rtmp://175.181.178.241/live/stream";
        rtmpUrl = mEditRtmpUrl.getEditableText().toString();

        if (rtmpUrl.length() == 0) {
            return;
        }

        if (view.getId() == R.id.btn_start_live) {
            Intent it = new Intent(this, HosterActivity.class);
            Bundle bd = new Bundle();
            bd.putString("rtmp_url", rtmpUrl);
            it.putExtras(bd);
            startActivity(it);
        } else {
            Intent it = new Intent(this, GuestActivity.class);
            Bundle bd = new Bundle();
            bd.putString("rtmp_url", rtmpUrl);
            it.putExtras(bd);
            startActivity(it);
        }
    }

    public void requestPermission(String... permissions)
    {
        if (checkPremission(permissions)) {
            return;
        }

        ActivityCompat.requestPermissions(this, permissions, 114);
    }

    public boolean checkPremission(String... permissions)
    {
        boolean allHave   = true;
        PackageManager pm = getPackageManager();

        for (String permission : permissions) {
            switch (pm.checkPermission(permission, getApplication().getPackageName())) {
                case PERMISSION_GRANTED:
                    allHave = allHave && true;
                    continue;

                case PERMISSION_DENIED:
                    allHave = allHave && false;
                    continue;
            }
        }

        return allHave;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults)
    {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (requestCode == 114 && permissions != null && permissions.length > 0) {
            String permission = "";

            for (int i = 0; i < permissions.length; i++) {
                permission = permissions[i];
                grantedResultDeal(
                    permission,
                    grantResults.length > i && grantResults[i] == PERMISSION_GRANTED);
            }
        }
    }

    protected void grantedResultDeal(String permission, boolean isGranted)
    {
        switch (permission) {
            case CAMERA:
                if (!isGranted) {
                    Toast.makeText(MainActivity.this, "未能获取到相机权限", Toast.LENGTH_LONG).show();
                }

                break;

            case RECORD_AUDIO:
                if (!isGranted) {
                    Toast.makeText(MainActivity.this, "未能获取到录音权限", Toast.LENGTH_LONG).show();
                }

                break;
        }
    }
}
