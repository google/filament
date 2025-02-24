// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TestStatusReporter:
//   Broadcasts test status to any listening {@link org.chromium.test.reporter.TestStatusReceiver}.

package com.android.angle.test;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import org.chromium.build.gtest_apk.TestStatusIntent;

public class TestStatusReporter
{
    private final Context mContext;

    public TestStatusReporter(Context c)
    {
        mContext = c;
    }

    public void testRunStarted(int pid)
    {
        sendTestRunBroadcast(TestStatusIntent.ACTION_TEST_RUN_STARTED, pid);
    }

    public void testRunFinished(int pid)
    {
        sendTestRunBroadcast(TestStatusIntent.ACTION_TEST_RUN_FINISHED, pid);
    }

    private void sendTestRunBroadcast(String action, int pid)
    {
        Intent i = new Intent(action);
        i.setType(TestStatusIntent.DATA_TYPE_RESULT);
        i.putExtra(TestStatusIntent.EXTRA_PID, pid);
        mContext.sendBroadcast(i);
    }

    public void uncaughtException(int pid, Throwable ex)
    {
        Intent i = new Intent(TestStatusIntent.ACTION_UNCAUGHT_EXCEPTION);
        i.setType(TestStatusIntent.DATA_TYPE_RESULT);
        i.putExtra(TestStatusIntent.EXTRA_PID, pid);
        i.putExtra(TestStatusIntent.EXTRA_STACK_TRACE, Log.getStackTraceString(ex));
        mContext.sendBroadcast(i);
    }
}
