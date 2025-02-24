//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLChooseConfigTest.cpp:
//   Tests of proper default-value semantics for eglChooseConfig

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"
#include "util/EGLWindow.h"

using namespace angle;

namespace angle
{
class EGLChooseConfigTest : public ANGLETest<>
{
  protected:
    EGLChooseConfigTest() {}
};

// Test that the EGL_COLOR_BUFFER_TYPE is defaulted to EGL_RGB_BUFFER
TEST_P(EGLChooseConfigTest, Defaults)
{
    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLint nConfigs       = 0;
    EGLint allConfigCount = 0;
    ASSERT_EGL_TRUE(eglGetConfigs(display, nullptr, 0, &nConfigs));
    ASSERT_NE(nConfigs, 0);

    std::vector<EGLConfig> allConfigs(nConfigs);
    ASSERT_EGL_TRUE(eglGetConfigs(display, allConfigs.data(), nConfigs, &allConfigCount));
    ASSERT_EQ(nConfigs, allConfigCount);

    // Choose configs that have the default attribute values:
    const EGLint defaultConfigAttributes[] = {EGL_NONE};
    EGLint defaultConfigCount;
    std::vector<EGLConfig> defaultConfigs(allConfigCount);
    ASSERT_EGL_TRUE(eglChooseConfig(display, defaultConfigAttributes, defaultConfigs.data(),
                                    defaultConfigs.size(), &defaultConfigCount));
    ASSERT_EGL_SUCCESS();
    ASSERT_LE(defaultConfigCount, allConfigCount);
    defaultConfigs.resize(defaultConfigCount);

    // Check that the default configs all have the default attribute values we care about:
    for (EGLConfig config : defaultConfigs)
    {
        EGLint colorBufferType, level, renderableType, surfaceType, transparentType;
        EGLint colorComponentType;

        eglGetConfigAttrib(display, config, EGL_COLOR_BUFFER_TYPE, &colorBufferType);
        ASSERT_EQ(colorBufferType, EGL_RGB_BUFFER);

        eglGetConfigAttrib(display, config, EGL_LEVEL, &level);
        ASSERT_EQ(level, 0);

        eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE, &renderableType);
        ASSERT_EQ(renderableType & EGL_OPENGL_ES_BIT, EGL_OPENGL_ES_BIT);

        eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EQ(surfaceType & EGL_WINDOW_BIT, EGL_WINDOW_BIT);

        eglGetConfigAttrib(display, config, EGL_TRANSPARENT_TYPE, &transparentType);
        ASSERT_EQ(transparentType, EGL_NONE);

        if (IsEGLDisplayExtensionEnabled(display, "EGL_EXT_pixel_format_float"))
        {
            eglGetConfigAttrib(display, config, EGL_COLOR_COMPONENT_TYPE_EXT, &colorComponentType);
            ASSERT_EQ(colorComponentType, EGL_COLOR_COMPONENT_TYPE_FIXED_EXT);
        }
    }

    // Check that all of the configs that have the default attribute values are are defaultConfigs,
    // and all that don't aren't:
    for (EGLConfig config : allConfigs)
    {
        EGLint colorBufferType, level, renderableType, surfaceType, transparentType;
        EGLint colorComponentType = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;

        eglGetConfigAttrib(display, config, EGL_COLOR_BUFFER_TYPE, &colorBufferType);
        eglGetConfigAttrib(display, config, EGL_LEVEL, &level);
        eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE, &renderableType);
        eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &surfaceType);
        eglGetConfigAttrib(display, config, EGL_TRANSPARENT_TYPE, &transparentType);
        if (IsEGLDisplayExtensionEnabled(display, "EGL_EXT_pixel_format_float"))
        {
            eglGetConfigAttrib(display, config, EGL_COLOR_COMPONENT_TYPE_EXT, &colorComponentType);
        }

        bool isADefault =
            ((colorBufferType == EGL_RGB_BUFFER) && (level == 0) &&
             ((renderableType & EGL_OPENGL_ES_BIT) == EGL_OPENGL_ES_BIT) &&
             ((surfaceType & EGL_WINDOW_BIT) == EGL_WINDOW_BIT) && (transparentType == EGL_NONE) &&
             (colorComponentType == EGL_COLOR_COMPONENT_TYPE_FIXED_EXT));
        EGLint thisConfigID;
        eglGetConfigAttrib(display, config, EGL_CONFIG_ID, &thisConfigID);
        bool foundInDefaultConfigs = false;
        // Attempt to find this config ID in defaultConfigs:
        for (EGLConfig defaultConfig : defaultConfigs)
        {
            EGLint defaultConfigID;
            eglGetConfigAttrib(display, defaultConfig, EGL_CONFIG_ID, &defaultConfigID);
            if (defaultConfigID == thisConfigID)
            {
                foundInDefaultConfigs = true;
            }
        }
        ASSERT_EQ(isADefault, foundInDefaultConfigs);
    }
}

// Test the validation errors for bad parameters for eglChooseConfig
TEST_P(EGLChooseConfigTest, NegativeValidationBadAttributes)
{
    EGLDisplay display = getEGLWindow()->getDisplay();

    // Choose configs using invalid attributes:
    const EGLint invalidConfigAttributeList[][3] = {
        {EGL_CONFIG_CAVEAT, 0, EGL_NONE},
        {EGL_SURFACE_TYPE, ~EGL_VG_COLORSPACE_LINEAR_BIT, EGL_NONE},
        {EGL_CONFORMANT, (EGL_OPENGL_ES_BIT | 0x0020), EGL_NONE},
        {EGL_RENDERABLE_TYPE, (EGL_OPENGL_ES_BIT | 0x0020), EGL_NONE},
    };
    EGLint configCount;
    EGLConfig config;

    for (size_t i = 0; i < 4; i++)
    {
        ASSERT_EGL_FALSE(
            eglChooseConfig(display, &invalidConfigAttributeList[i][0], &config, 1, &configCount));
        ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }
}

}  // namespace angle

ANGLE_INSTANTIATE_TEST(EGLChooseConfigTest,
                       ES2_D3D11(),
                       ES2_D3D9(),
                       ES2_METAL(),
                       ES2_OPENGL(),
                       ES2_OPENGLES(),
                       ES2_VULKAN());
