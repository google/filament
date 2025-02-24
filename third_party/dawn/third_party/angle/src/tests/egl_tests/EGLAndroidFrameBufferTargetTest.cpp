//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLAndroidFrameBufferTargetTest.cpp:
//   This test verifies the extension EGL_ANDROID_framebuffer_target
// 1. When the EGLFRAME_BUFFER_TARGET_ANDROID attribute is used with eglChooseConfig
// It should match with configs according to Config selection rules and the extension
//

#include <gtest/gtest.h>

#include "common/string_utils.h"
#include "test_utils/ANGLETest.h"

using namespace angle;

class EGLAndroidFrameBufferTargetTest : public ANGLETest<>
{
  protected:
    EGLAndroidFrameBufferTargetTest() {}

    void testSetUp() override
    {
        mDisplay = getEGLWindow()->getDisplay();
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);
    }

    EGLDisplay mDisplay = EGL_NO_DISPLAY;
};

namespace
{
EGLint GetAttrib(EGLDisplay display, EGLConfig config, EGLint attrib)
{
    EGLint value = 0;
    EXPECT_EGL_TRUE(eglGetConfigAttrib(display, config, attrib, &value));
    return value;
}
}  // namespace

// Verify config matching is working.
TEST_P(EGLAndroidFrameBufferTargetTest, MatchFramebufferTargetConfigs)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_ANDROID_framebuffer_target"));

    // Get all the configs
    EGLint count;
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &count));
    EXPECT_TRUE(count > 0);
    std::vector<EGLConfig> configs(count);
    EXPECT_EGL_TRUE(eglGetConfigs(mDisplay, configs.data(), count, &count));
    ASSERT_EQ(configs.size(), static_cast<size_t>(count));

    // Filter out all non-framebuffertarget configs
    std::vector<EGLConfig> filterConfigs(0);
    for (auto config : configs)
    {
        if (GetAttrib(mDisplay, config, EGL_FRAMEBUFFER_TARGET_ANDROID) == EGL_TRUE)
        {
            filterConfigs.push_back(config);
        }
    }
    // sort configs by increaing ID
    std::sort(filterConfigs.begin(), filterConfigs.end(), [this](EGLConfig a, EGLConfig b) -> bool {
        return GetAttrib(mDisplay, a, EGL_CONFIG_ID) < GetAttrib(mDisplay, b, EGL_CONFIG_ID);
    });

    // Now get configs that selection algorithm identifies
    EGLint attribs[] = {EGL_FRAMEBUFFER_TARGET_ANDROID,
                        EGL_TRUE,
                        EGL_COLOR_BUFFER_TYPE,
                        EGL_DONT_CARE,
                        EGL_COLOR_COMPONENT_TYPE_EXT,
                        EGL_DONT_CARE,
                        EGL_NONE};
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, attribs, nullptr, 0, &count));
    std::vector<EGLConfig> matchConfigs(count);
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, attribs, matchConfigs.data(), count, &count));
    matchConfigs.resize(count);
    // sort configs by increasing ID
    std::sort(matchConfigs.begin(), matchConfigs.end(), [this](EGLConfig a, EGLConfig b) -> bool {
        return GetAttrib(mDisplay, a, EGL_CONFIG_ID) < GetAttrib(mDisplay, b, EGL_CONFIG_ID);
    });

    EXPECT_EQ(matchConfigs, filterConfigs) << "Filtered configs do not match selection Configs";
}

ANGLE_INSTANTIATE_TEST(EGLAndroidFrameBufferTargetTest, ES2_VULKAN(), ES3_VULKAN());

// This test suite is not instantiated on some OSes.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLAndroidFrameBufferTargetTest);
