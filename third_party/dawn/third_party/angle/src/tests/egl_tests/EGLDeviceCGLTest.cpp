//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
//    EGLDeviceCGLTest.cpp: tests for the EGL_ANGLE_device_cgl extension.
//

#include "test_utils/ANGLETest.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"
#include "util/gles_loader_autogen.h"

using namespace angle;

class EGLDeviceCGLQueryTest : public ANGLETest<>
{
  protected:
    EGLDeviceCGLQueryTest() {}

    void testSetUp() override
    {
        const char *extensionString =
            static_cast<const char *>(eglQueryString(getEGLWindow()->getDisplay(), EGL_EXTENSIONS));

        if (!eglQueryDeviceStringEXT)
        {
            FAIL() << "ANGLE extension EGL_EXT_device_query export eglQueryDeviceStringEXT was not "
                      "found";
        }

        if (!eglQueryDisplayAttribEXT)
        {
            FAIL() << "ANGLE extension EGL_EXT_device_query export eglQueryDisplayAttribEXT was "
                      "not found";
        }

        if (!eglQueryDeviceAttribEXT)
        {
            FAIL() << "ANGLE extension EGL_EXT_device_query export eglQueryDeviceAttribEXT was not "
                      "found";
        }

        EGLAttrib angleDevice = 0;
        EXPECT_EGL_TRUE(
            eglQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));
        extensionString = static_cast<const char *>(
            eglQueryDeviceStringEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice), EGL_EXTENSIONS));
        if (strstr(extensionString, "EGL_ANGLE_device_cgl") == nullptr)
        {
            FAIL() << "ANGLE extension EGL_ANGLE_device_cgl was not found";
        }
    }
};

// This test attempts to query the CGLContextObj and CGLPixelFormatObj from the
// EGLDevice associated with the display.
TEST_P(EGLDeviceCGLQueryTest, QueryDevice)
{
    EGLAttrib angleDevice = 0;
    EXPECT_EGL_TRUE(
        eglQueryDisplayAttribEXT(getEGLWindow()->getDisplay(), EGL_DEVICE_EXT, &angleDevice));
    EGLAttrib contextAttrib     = 0;
    EGLAttrib pixelFormatAttrib = 0;
    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                            EGL_CGL_CONTEXT_ANGLE, &contextAttrib));
    EXPECT_TRUE(contextAttrib != 0);
    EXPECT_EGL_TRUE(eglQueryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                            EGL_CGL_PIXEL_FORMAT_ANGLE, &pixelFormatAttrib));
    EXPECT_TRUE(pixelFormatAttrib != 0);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(EGLDeviceCGLQueryTest, ES2_OPENGL(), ES3_OPENGL());
