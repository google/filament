//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLPowerPreferenceTest.cpp:
//   Checks the implementation of EGL_ANGLE_power_preference.
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

class EGLPowerPreferenceTest : public ANGLETest<>
{
  public:
    void testSetUp() override { (void)GetSystemInfo(&mSystemInfo); }

  protected:
    auto getGpuIdParts(size_t gpuIndex) const
    {
        uint64_t deviceId = mSystemInfo.gpus[gpuIndex].systemDeviceId;
        return std::make_tuple(GetSystemDeviceIdHighPart(deviceId),
                               GetSystemDeviceIdLowPart(deviceId));
    }

    EGLDisplay getDisplay() const { return getEGLWindow()->getDisplay(); }

    SystemInfo mSystemInfo;
};

TEST_P(EGLPowerPreferenceTest, ForceGPUSwitch)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(getDisplay(), "EGL_ANGLE_power_preference"));
    size_t lowPower   = FindLowPowerGPU(mSystemInfo);
    size_t highPower  = FindHighPowerGPU(mSystemInfo);
    size_t initialGPU = FindActiveOpenGLGPU(mSystemInfo);
    ASSERT_TRUE(lowPower == initialGPU || highPower == initialGPU);

    EGLint hi = 0;
    EGLint lo = 0;

    for (int i = 0; i < 5; ++i)
    {
        std::tie(hi, lo) = getGpuIdParts(lowPower);
        eglForceGPUSwitchANGLE(getDisplay(), hi, lo);
        EXPECT_EQ(lowPower, FindActiveOpenGLGPU(mSystemInfo));
        std::tie(hi, lo) = getGpuIdParts(highPower);
        eglForceGPUSwitchANGLE(getDisplay(), hi, lo);
        EXPECT_EQ(highPower, FindActiveOpenGLGPU(mSystemInfo));
    }
}

TEST_P(EGLPowerPreferenceTest, HandleGPUSwitchAfterForceGPUSwitch)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(getDisplay(), "EGL_ANGLE_power_preference"));
    size_t initialGPU = FindActiveOpenGLGPU(mSystemInfo);
    size_t changedGPU = FindLowPowerGPU(mSystemInfo);
    // On all platforms the extension is implemented (e.g. CGL): If we start with integrated, and
    // force DGPU, we cannot eglHandleGPUSwitchANGLE() from DGPU to integrated.
    // eglHandleGPUSwitchANGLE() will switch to the "default", which will be DGPU.
    // If we start with DGPU and switch to integrated, we *can* eglHandleGPUSwitchANGLE() back
    // to the default, DGPU.
    ANGLE_SKIP_TEST_IF(initialGPU == changedGPU);

    EGLint hi = 0;
    EGLint lo = 0;
    for (int i = 0; i < 5; ++i)
    {
        std::tie(hi, lo) = getGpuIdParts(changedGPU);
        eglForceGPUSwitchANGLE(getDisplay(), hi, lo);
        ASSERT_EQ(changedGPU, FindActiveOpenGLGPU(mSystemInfo));
        eglHandleGPUSwitchANGLE(getDisplay());
        ASSERT_EQ(initialGPU, FindActiveOpenGLGPU(mSystemInfo));
    }
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLPowerPreferenceTest);
ANGLE_INSTANTIATE_TEST(EGLPowerPreferenceTest, ES2_OPENGL(), ES3_OPENGL());
