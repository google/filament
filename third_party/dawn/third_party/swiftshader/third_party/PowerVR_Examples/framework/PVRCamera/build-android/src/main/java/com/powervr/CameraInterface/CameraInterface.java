package com.powervr.PVRCamera;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.Size;


public class CameraInterface implements SurfaceTexture.OnFrameAvailableListener {
	// Camera Source
	public enum CameraSource {
		FRONT_FACING(1),
		REAR_FACING(0);
		
		public int id = 0;
		private CameraSource(int id) {
			this.id = id;
		}
	}
	
	// Size comparator
	public class SizeComparator implements Comparator<Size> {

		@Override
		public int compare(Size s1, Size s2) {
			if(s1.width > s2.width) {
				return 1;
			}
			else if(s1.width < s2.width) {
				return -1;
			}
			return 0;
		}
	}

	private Camera mCamera;
	private SurfaceTexture mSurface;
	
	private float[] mTexCoordsProjM = new float[16];
	private float[] mTempTexCoordsProjM = new float[16];
	private final AtomicInteger mUpdateImage = new AtomicInteger(0);
	private boolean mCameraReady = false;
	private boolean showRationale = true;

	private Context context;
	private int mCameraResolutionX = -1;
	private int mCameraResolutionY = -1;
	private int pauseResume = 0;
	private boolean mHasUpdated = true;
	private final CameraSource mCameraSource;

	private final Activity myActivity;

	// Native calls
	private native void createJavaObjects();
	private native void releaseJavaObjects();
	private native boolean setTexCoordsProjMatrix(float[] a, int width, int height);

	private final String permissionRationale = "This application cannot show this Devices's Camera's image without accessing the Camera. The Injustice! This application would thus humbly beg you, in your infinite magnanimity, to grant the right - nay, the Privilege! - of accessing said Camera.";

	private boolean checkPermissions()
	{
		if (ActivityCompat.checkSelfPermission(myActivity, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
			return true;
		}
		if (pauseResume++ %2 ==0) // Wat? The permissions message keeps the app paused. So, we need a way to avoid the infinite loop.
		{
			ActivityCompat.requestPermissions(myActivity, new String[]{Manifest.permission.CAMERA}, 42 );
		}

		return false;
	}

	public CameraInterface (Activity activity, int width, int height, CameraSource cs) {
		Log.d("com.powervr.PVRCamera", "com.powervr.PVRCamera Ctor.");
		this.mCameraSource = cs;
		mCameraResolutionX = width;
		mCameraResolutionY = height;

		myActivity = activity;
	}

	private void cameraReady(){
		Log.i("com.powervr.PVRCamera", "Java:cameraReady - Activating JNI side");
		createJavaObjects();
		if (mCamera != null && !mCameraReady) {
			// Attempt to attach the gl texture to the camera
			try {
				mCamera.open();
				mCamera.setPreviewTexture(mSurface);
				mCamera.startPreview();
				mCameraReady = true;
				Log.i("com.powervr.PVRCamera", "Java:cameraReady - Preview started");
			} catch (Exception e) {
				Log.e("com.powervr.PVRCamera", "Java:cameraReady - Resume failed: Could not open camera, or camera was already open.");
			}
		} else {
			Log.w("com.powervr.PVRCamera", "Java:cameraReady - Camera was still NULL");
		}
	}

	private void cameraNotReady(){
		Log.d("com.powervr.PVRCamera", "Java:cameraNotReady executing");
		releaseJavaObjects();
		if (mCamera != null) {
			try {
				mCamera.stopPreview();
				mCamera.release();
			}catch(Exception e){
				// Gulp! If the camera is released already, this will throw. No harm.
			}

			mCameraReady = false;
		}else
		{
			Log.w("com.powervr.PVRCamera", "Java:cameraNotReady - Camera was NULL");
		}

	}

	public void onResume(){
		if (!checkPermissions()) {
			Log.w("com.powervr.PVRCamera", "Java:onResume - Permissions not granted yet.");
			return;
		}

		Log.i("com.powervr.PVRCamera", "Java:onResume executing cameraReady");
		cameraReady();
	}

	public void onPause()
	{
		if (ActivityCompat.checkSelfPermission(myActivity, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
			Log.w("com.powervr.PVRCamera", "Java:onPause - Permissions not granted yet.");
			return;
		}

		cameraNotReady();
	}

	// This class is not an activity, hence this function is not called automatically!
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
		for (int i = 0; i<permissions.length; ++i)
		{
			if (permissions[i].equals(Manifest.permission.CAMERA)) {
				if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
					Log.i("com.powervr.PVRCamera", "Java:onRequestPermissionsResult Granted Camera permission. Activating camera!");
					cameraReady();
					break;
				}else{
					if (showRationale){
						AlertDialog dlg = new AlertDialog.Builder(myActivity).create();
						dlg.setTitle("Permission required.");
						dlg.setMessage(permissionRationale);
						dlg.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
								new DialogInterface.OnClickListener() {
									public void onClick(DialogInterface dialog, int which) {
										dialog.dismiss();
										ActivityCompat.requestPermissions(myActivity, new String[]{Manifest.permission.CAMERA}, 42 );
									}
								});
						dlg.setButton(AlertDialog.BUTTON_NEGATIVE, "I like black screens",
								new DialogInterface.OnClickListener() {
									public void onClick(DialogInterface dialog, int which) {
										dialog.dismiss();
									}
								});
						showRationale = false;
						dlg.show();
					}
				}
			}
		}
	}

	// This function will be called by the JNI side during the normal operation of the game loop, but
	// only after createJavaObjects has been called from this side.
	public int createCamera(int textureID) {
		Log.d("com.powervr.PVRCamera", "Java:createCamera - Texture ID = " + Integer.toString(textureID));
		// Create SurfaceTexture
		try {
			mSurface = new SurfaceTexture(textureID);
		}
		catch (RuntimeException ioe) {
			Log.e("com.powervr.PVRCamera", "Java:createCamera - Error creating SurfaceTexture");
			return 0;
		}

		try {
			mUpdateImage.set(1);
			mSurface.setOnFrameAvailableListener(this);
		}
		catch (RuntimeException ioe) {
			Log.w("com.powervr.PVRCamera", "Java:createCamera - Error setting OnFrameAvailableListener");
			return 0;
		}

		if(Camera.getNumberOfCameras() == 0)
		{
			Log.w("com.powervr.PVRCamera", "Java:createCamera - Error - Could not find a Camera to use.");
			return 0;
		}

		Log.i("com.powervr.PVRCamera", "Java:createCamera - Camera(s) found: " + Camera.getNumberOfCameras());

		// Opening the main camera
		int cameraid = mCameraSource.id;
		mCamera = Camera.open(cameraid);

		// Get the camera parameters
		Camera.Parameters cp = mCamera.getParameters();

		// Get (and sort) the list of available sizes
		List<Size> sl = cp.getSupportedPreviewSizes();
		Collections.sort(sl, new SizeComparator());

		// Logic to choose a resolution
		mHasUpdated = true;

		int scrWidth = mCameraResolutionX; // Initially contains requested width - otherwise the REAL x resolution based on the below.
		int camWidth = 0;
		int camHeight = 0;

		Size currSize = cp.getPreviewSize();
		camWidth = currSize.width;
		camHeight = currSize.height;

		Log.i("com.powervr.PVRCamera", "Java:createCamera - Setting camera resolution.");

		// Out of all the possible options, choose the immediatly bigger to the screen resolution - otherwise keep whatever was default.
		for(Size s : sl) {
			Log.i("com.powervr.PVRCamera", "Java:createCamera - Camera supports "+ s.width + "x" + s.height);
			if (s.width >= scrWidth) {
				camWidth = s.width;
				camHeight = s.height;
			}
		}

		mCameraResolutionX = camWidth;
		mCameraResolutionY = camHeight;
		Log.i( "com.powervr.PVRCamera", "Java:createCamera - Using camera resolution of " + mCameraResolutionX + "x" + mCameraResolutionY );

		// Set format, resolution etc.
		cp.setPreviewSize(mCameraResolutionX, mCameraResolutionY);
		cp.setPreviewFormat(ImageFormat.NV21);

		List< String > FocusModes = cp.getSupportedFocusModes();
		if(FocusModes != null && FocusModes.contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO )) {
			cp.setFocusMode( Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO );
		}

		mCamera.setParameters(cp);

		// Attempt to attach the gl texture to the camera
		try {
			mCamera.setPreviewTexture(mSurface);
			mCamera.startPreview();
			mCameraReady = true;
			Log.i("com.powervr.PVRCamera", "Java:createCamera - Camera preview started.");
		} catch (Exception e) {
			Log.e("com.powervr.PVRCamera", "Java:createCamera - Could not open camera");

			mCamera.stopPreview();
			mCamera.release();
			mCameraReady = false;
			return 0;
		}

		return textureID;
	}

	@Override
	public void onFrameAvailable(SurfaceTexture surfaceTexture) {
		// Atomic increment mUpdateImage;
		while(true) {
			int existingValue = mUpdateImage.get();
			if(mUpdateImage.compareAndSet(existingValue, existingValue + 1)) {
				break;
			}
		}
	}

	public boolean updateImage() {
		if (mCamera != null && mCameraReady) {
			if (mUpdateImage.get() > 0) {
				while (mUpdateImage.get() > 0) {
					// Update the texture
					mSurface.updateTexImage();
					mSurface.getTransformMatrix(mTempTexCoordsProjM);
					// Atomic decrement
					while (true) {
						int existingValue = mUpdateImage.get();
						if (mUpdateImage.compareAndSet(existingValue, existingValue - 1)) {
							break;
						}
					}
				}

				// Check if the Projection Matrix has changed
				for (int i = 0; i < 16; ++i) {
					if (mTempTexCoordsProjM[i] != mTexCoordsProjM[i]) {
						mTexCoordsProjM[i] = mTempTexCoordsProjM[i];
						mHasUpdated = true;
					}
				}
				if (mHasUpdated)
				{
					if (setTexCoordsProjMatrix(mTexCoordsProjM, mCameraResolutionX, mCameraResolutionY))
					{
						mHasUpdated = false;
					}
				}

				return true;
			}
		} else {
			Log.d("com.powervr.PVRCamera", "Java: UpdateImage. Camera was NOT ready.");
		}
		return false;
	}
}
