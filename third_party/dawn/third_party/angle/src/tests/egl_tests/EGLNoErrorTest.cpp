//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLNoErrorTest.cpp:
//   Tests for the EGL extension EGL_ANGLE_no_error
//

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"

using namespace angle;

class EGLNoErrorTest : public ANGLETest<>
{};

// Validation errors become undefined behavour with this extension. Simply test turning validation
// off and on.
TEST_P(EGLNoErrorTest, EnableDisable)
{
    if (IsEGLClientExtensionEnabled("EGL_ANGLE_no_error"))
    {
        eglSetValidationEnabledANGLE(EGL_FALSE);
        eglSetValidationEnabledANGLE(EGL_TRUE);
        EXPECT_EGL_ERROR(EGL_SUCCESS);
    }
    else
    {
        eglSetValidationEnabledANGLE(EGL_FALSE);
        EXPECT_EGL_ERROR(EGL_BAD_ACCESS);
    }
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLNoErrorTest);
ANGLE_INSTANTIATE_TEST(EGLNoErrorTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_VULKAN());
