package com.powervr.OpenGLESIntroducingPVRCamera;

import com.powervr.PVRCamera.CameraInterface;
import com.powervr.PVRCamera.CameraInterface.CameraSource;

import android.app.NativeActivity;
import android.os.Bundle;
import android.widget.Toast;
import android.view.Gravity;
import android.graphics.Point;
import android.view.Display;

public class OpenGLESIntroducingPVRCamera extends NativeActivity
{
	public void displayExitMessage(final String text) 
	{
		runOnUiThread(new Runnable() {
		public void run() {
			Toast toast = Toast.makeText(getApplicationContext(), text, Toast.LENGTH_LONG);
			toast.setGravity(Gravity.CENTER, 0, 0);
			toast.show();
		}
		});
	}
	static 
	{
		System.loadLibrary("OpenGLESIntroducingPVRCamera");
	}
	
	private CameraInterface mCameraInterface = null;

@Override
	protected void onCreate (Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		// Create Camera
		if(mCameraInterface == null)
		{
			Display display = getWindowManager().getDefaultDisplay();
			Point size = new Point();
			display.getSize(size);
			mCameraInterface = new CameraInterface(this, size.x, size.y, CameraSource.REAR_FACING);
		}
	}
	@Override
	protected void onResume()
	{
		super.onResume();
		if (mCameraInterface != null)
		mCameraInterface.onResume();
	}

	@Override
	public void onPause()
	{
		if (mCameraInterface != null)
		{
			mCameraInterface.onPause();
		}
		super.onPause();
	}
	
	@Override
	protected void onDestroy()
	{
		mCameraInterface = null;
		super.onDestroy();
	}

	@Override
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
		if (mCameraInterface != null) 
		{
			mCameraInterface.onRequestPermissionsResult(requestCode, permissions, grantResults);
		}
	}

}
