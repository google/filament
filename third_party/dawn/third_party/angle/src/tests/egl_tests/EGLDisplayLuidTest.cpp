//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLDisplayLuidTest.cpp:
//   Tests for the EGL_ANGLE_platform_angle_d3d_luid extension.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class EGLDisplayLuidTest : public ANGLETest<>
{
  protected:
    EGLDisplayLuidTest() : mDisplay(EGL_NO_DISPLAY) {}

    void testTearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            EXPECT_EGL_TRUE(eglTerminate(mDisplay));
            EXPECT_EGL_SUCCESS();
        }
    }

    void testInvalidAttribs(const EGLint displayAttribs[])
    {
        EXPECT_EQ(
            eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, displayAttribs),
            EGL_NO_DISPLAY);
        EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }

    void testValidAttribs(const EGLint displayAttribs[])
    {
        mDisplay =
            eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, displayAttribs);

        EXPECT_EGL_SUCCESS();
        EXPECT_NE(mDisplay, EGL_NO_DISPLAY);

        // eglInitialize should succeed even if the LUID doesn't match an actual
        // adapter on the system. The behavior in this case is that the default
        // adapter is used.
        EXPECT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));
        EXPECT_EGL_SUCCESS();
    }

  private:
    EGLDisplay mDisplay;
};

// EGL_ANGLE_platform_angle_d3d_luid is only supported on D3D11. Verify failure
// if D3D9 is specified in the attributes.
TEST_P(EGLDisplayLuidTest, D3D9Failure)
{
    EGLint displayAttribs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE,
                               EGL_PLATFORM_ANGLE_D3D_LUID_HIGH_ANGLE, 1, EGL_NONE};
    testInvalidAttribs(displayAttribs);
}

// Verify failure if the specified LUID is zero.
TEST_P(EGLDisplayLuidTest, ZeroLuidFailure)
{
    EGLint displayAttribs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE,
                               EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
                               EGL_PLATFORM_ANGLE_D3D_LUID_HIGH_ANGLE,
                               0,
                               EGL_PLATFORM_ANGLE_D3D_LUID_LOW_ANGLE,
                               0,
                               EGL_NONE};
    testInvalidAttribs(displayAttribs);
}

TEST_P(EGLDisplayLuidTest, D3D11)
{
    EGLint displayAttribs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
                               EGL_PLATFORM_ANGLE_D3D_LUID_HIGH_ANGLE, 1, EGL_NONE};
    testValidAttribs(displayAttribs);
}

ANGLE_INSTANTIATE_TEST(EGLDisplayLuidTest, WithNoFixture(ES2_D3D9()), WithNoFixture(ES2_D3D11()));
