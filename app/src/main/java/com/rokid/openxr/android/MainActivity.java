package com.rokid.openxr.android;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;

import androidx.core.app.ActivityCompat;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends android.app.NativeActivity {

    static {
        System.loadLibrary("openxr_loader");
        System.loadLibrary("openxr_demo");
    }

    public native void setNativeAssetManager(AssetManager assetManager);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setNativeAssetManager(this.getAssets());
        getPermission(this);
    }

    private List<String> checkPermission(Context context, String[] checkList) {
        List<String> list = new ArrayList<>();
        for (int i = 0; i < checkList.length; i++) {
            if (PackageManager.PERMISSION_GRANTED != ActivityCompat.checkSelfPermission(context, checkList[i])) {
                list.add(checkList[i]);
            }
        }
        return list;
    }

    private void requestPermission(Activity activity, String requestPermissionList[]) {
        ActivityCompat.requestPermissions(activity, requestPermissionList, 100);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        if (requestCode == 100) {
            for (int i = 0; i < permissions.length; i++) {
                if (permissions[i].equals(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                    if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
                        Log.i("RK-Openxr-App", "Successfully applied for storage permission!");
                    } else {
                        Log.e("RK-Openxr-App", "Failed to apply for storage permission!");
                    }
                }
            }
        }
    }

    private void getPermission(Activity activity) {
        String[] checkList = new String[]{android.Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.MANAGE_EXTERNAL_STORAGE}; //2025-06-17 添加MANAGE_EXTERNAL_STORAGE权限检查
        List<String> needRequestList = checkPermission(activity, checkList);
        if (needRequestList.isEmpty()) {
            Log.i("RK-Openxr-App", "No need to apply for storage permission!");
        } else {
            requestPermission(activity, needRequestList.toArray(new String[needRequestList.size()]));
        }
    }
}