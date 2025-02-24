// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AngleUnitTestActivity:
//   A {@link android.app.NativeActivity} for running angle gtests.

package com.android.angle.test;

import android.app.NativeActivity;
import android.os.Bundle;
import android.util.Log;

import org.chromium.build.NativeLibraries;

public class AngleUnitTestActivity extends NativeActivity
{
    private static final String TAG = "NativeTest";

    private AngleNativeTest mTest = new AngleNativeTest();

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        // For NativeActivity based tests,
        // dependency libraries must be loaded before NativeActivity::OnCreate,
        // otherwise loading android.app.lib_name will fail
        for (String library : NativeLibraries.LIBRARIES)
        {
            Log.i(TAG, "loading: " + library);
            System.loadLibrary(library);
            Log.i(TAG, "loaded: " + library);
        }

        super.onCreate(savedInstanceState);
        mTest.postCreate(this);
    }

    @Override
    public void onStart()
    {
        super.onStart();
        mTest.postStart(this);
    }
}
