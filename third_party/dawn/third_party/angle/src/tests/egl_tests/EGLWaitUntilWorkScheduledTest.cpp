//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLWaitUntilWorkScheduledTest.cpp:
//   Checks the implementation of EGL_ANGLE_wait_until_work_scheduled.
//

#include <gtest/gtest.h>
#include <tuple>

#include "common/debug.h"
#include "common/string_utils.h"
#include "gpu_info_util/SystemInfo.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_platform.h"
#include "test_utils/system_info_util.h"
#include "util/OSWindow.h"

using namespace angle;

class EGLWaitUntilWorkScheduledTest : public ANGLETest<>
{
  public:
    void testSetUp() override { (void)GetSystemInfo(&mSystemInfo); }

  protected:
    EGLDisplay getDisplay() const { return getEGLWindow()->getDisplay(); }

    SystemInfo mSystemInfo;
};

// Test if EGL_ANGLE_wait_until_work_scheduled is enabled that we can call
// eglWaitUntilWorkScheduledANGLE.
TEST_P(EGLWaitUntilWorkScheduledTest, WaitUntilWorkScheduled)
{
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(getDisplay(), "EGL_ANGLE_wait_until_work_scheduled"));

    // We're not checking anything except that the function can be called.
    eglWaitUntilWorkScheduledANGLE(getDisplay());
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLWaitUntilWorkScheduledTest);
ANGLE_INSTANTIATE_TEST(EGLWaitUntilWorkScheduledTest,
                       ES2_METAL(),
                       ES3_METAL(),
                       ES2_OPENGL(),
                       ES3_OPENGL());
