// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AngleNativeTest:
//   Helper to run Angle tests inside NativeActivity.

package com.android.angle.test;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Process;
import android.system.Os;
import android.util.Log;
import android.view.View;

import org.chromium.build.gtest_apk.NativeTestIntent;

import java.io.File;

public class AngleNativeTest
{
    private static final String TAG = "NativeTest";

    private String mCommandLineFilePath;
    private StringBuilder mCommandLineFlags = new StringBuilder();
    private TestStatusReporter mReporter;
    private String mStdoutFilePath;

    private static class ReportingUncaughtExceptionHandler
            implements Thread.UncaughtExceptionHandler
    {

        private TestStatusReporter mReporter;
        private Thread.UncaughtExceptionHandler mWrappedHandler;

        public ReportingUncaughtExceptionHandler(
                TestStatusReporter reporter, Thread.UncaughtExceptionHandler wrappedHandler)
        {
            mReporter       = reporter;
            mWrappedHandler = wrappedHandler;
        }

        @Override
        public void uncaughtException(Thread thread, Throwable ex)
        {
            mReporter.uncaughtException(Process.myPid(), ex);
            if (mWrappedHandler != null) mWrappedHandler.uncaughtException(thread, ex);
        }
    }

    public void postCreate(Activity activity)
    {
        parseArgumentsFromIntent(activity, activity.getIntent());
        mReporter = new TestStatusReporter(activity);
        mReporter.testRunStarted(Process.myPid());
        Thread.setDefaultUncaughtExceptionHandler(new ReportingUncaughtExceptionHandler(
                mReporter, Thread.getDefaultUncaughtExceptionHandler()));

        // Enable fullscreen mode
        // https://developer.android.com/training/system-ui/immersive#java
        View decorView = activity.getWindow().getDecorView();
        if (decorView != null)
        {
            decorView.setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_FULLSCREEN);
        }
    }

    private void parseArgumentsFromIntent(Activity activity, Intent intent)
    {
        Log.i(TAG, "Extras:");
        Bundle extras = intent.getExtras();
        if (extras != null)
        {
            for (String s : extras.keySet())
            {
                Log.i(TAG, "  " + s);
            }
        }

        mCommandLineFilePath = intent.getStringExtra(NativeTestIntent.EXTRA_COMMAND_LINE_FILE);
        if (mCommandLineFilePath == null)
        {
            mCommandLineFilePath = "";
        }
        else
        {
            File commandLineFile = new File(mCommandLineFilePath);
            if (!commandLineFile.isAbsolute())
            {
                mCommandLineFilePath =
                        Environment.getExternalStorageDirectory() + "/" + mCommandLineFilePath;
            }
            Log.i(TAG, "command line file path: " + mCommandLineFilePath);
        }

        String commandLineFlags = intent.getStringExtra(NativeTestIntent.EXTRA_COMMAND_LINE_FLAGS);
        if (commandLineFlags != null) mCommandLineFlags.append(commandLineFlags);

        String gtestFilter = intent.getStringExtra(NativeTestIntent.EXTRA_GTEST_FILTER);
        if (gtestFilter != null)
        {
            appendCommandLineFlags("--gtest_filter=" + gtestFilter);
        }

        mStdoutFilePath = intent.getStringExtra(NativeTestIntent.EXTRA_STDOUT_FILE);
    }

    private void appendCommandLineFlags(String flags)
    {
        mCommandLineFlags.append(" ").append(flags);
    }

    public void postStart(final Activity activity)
    {
        final Runnable runTestsTask = new Runnable() {
            @Override
            public void run()
            {
                runTests(activity);
            }
        };

        // Post a task that posts a task that creates a new thread and runs tests on it.
        // This is needed because NativeActivity processes Looper messages in native code code,
        // which makes invoking the test runner Handler problematic.

        // On L and M, the system posts a task to the main thread that prints to stdout
        // from android::Layout (https://goo.gl/vZA38p). Chaining the subthread creation
        // through multiple tasks executed on the main thread ensures that this task
        // runs before we start running tests s.t. its output doesn't interfere with
        // the test output. See crbug.com/678146 for additional context.

        final Handler handler              = new Handler();
        final Runnable startTestThreadTask = new Runnable() {
            @Override
            public void run()
            {
                new Thread(runTestsTask).start();
            }
        };
        final Runnable postTestStarterTask = new Runnable() {
            @Override
            public void run()
            {
                handler.post(startTestThreadTask);
            }
        };
        handler.post(postTestStarterTask);
    }

    private void runTests(Activity activity)
    {
        Log.i(TAG, "runTests: " + mCommandLineFlags.toString());
        nativeRunTests(mCommandLineFlags.toString(), mCommandLineFilePath, mStdoutFilePath);
        Log.i(TAG, "runTests finished");
        activity.finish();
        mReporter.testRunFinished(Process.myPid());
    }

    // Signal a failure of the native test loader to python scripts
    // which run tests.  For example, we look for
    // RUNNER_FAILED build/android/test_package.py.
    private void nativeTestFailed()
    {
        Log.e(TAG, "[ RUNNER_FAILED ] could not load native library");
    }

    private native void nativeRunTests(
            String commandLineFlags, String commandLineFilePath, String stdoutFilePath);
}
