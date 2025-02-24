package com.powervr.OpenGLESImageBasedLighting;


import android.app.NativeActivity;
import android.os.Bundle;
import android.widget.Toast;
import android.view.Gravity;

public class OpenGLESImageBasedLighting extends NativeActivity
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
}
