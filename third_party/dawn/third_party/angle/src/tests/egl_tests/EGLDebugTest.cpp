//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLDebugTest.cpp:
//   Tests of EGL_KHR_debug extension

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"
#include "util/EGLWindow.h"

namespace angle
{
class EGLDebugTest : public ANGLETest<>
{
  protected:
    void testTearDown() override { eglDebugMessageControlKHR(nullptr, nullptr); }

    bool hasExtension() const { return IsEGLClientExtensionEnabled("EGL_KHR_debug"); }

    static void EGLAPIENTRY StubCallback(EGLenum error,
                                         const char *command,
                                         EGLint messageType,
                                         EGLLabelKHR threadLabel,
                                         EGLLabelKHR objectLabel,
                                         const char *message)
    {}

    static void EGLAPIENTRY CheckBadBindAPIError(EGLenum error,
                                                 const char *command,
                                                 EGLint messageType,
                                                 EGLLabelKHR threadLabel,
                                                 EGLLabelKHR objectLabel,
                                                 const char *message)
    {
        EXPECT_STREQ("eglBindAPI", command);
        ASSERT_EGLENUM_EQ(EGL_BAD_PARAMETER, error);
        EXPECT_STREQ("Thread", static_cast<const char *>(threadLabel));
    }

    static EGLDEBUGPROCKHR EGLAttribToDebugCallback(EGLAttrib attrib)
    {
        return reinterpret_cast<EGLDEBUGPROCKHR>(static_cast<uintptr_t>(attrib));
    }

    static EGLAttrib DebugCallbackToEGLAttrib(EGLDEBUGPROCKHR callback)
    {
        return static_cast<EGLAttrib>(reinterpret_cast<intptr_t>(callback));
    }
};

// Test that the extension is always available (it is implemented in ANGLE's frontend).
TEST_P(EGLDebugTest, ExtensionAlwaysAvailable)
{
    ASSERT_TRUE(hasExtension());
}

// Check that the default message filters and callbacks are correct
TEST_P(EGLDebugTest, DefaultParameters)
{
    ANGLE_SKIP_TEST_IF(!hasExtension());

    EXPECT_EQ(static_cast<EGLint>(EGL_SUCCESS), eglDebugMessageControlKHR(nullptr, nullptr));

    EGLAttrib result = 0;

    EXPECT_EGL_TRUE(eglQueryDebugKHR(EGL_DEBUG_MSG_ERROR_KHR, &result));
    EXPECT_EGL_TRUE(result);

    EXPECT_EGL_TRUE(eglQueryDebugKHR(EGL_DEBUG_MSG_WARN_KHR, &result));
    EXPECT_EGL_FALSE(result);

    EXPECT_EGL_TRUE(eglQueryDebugKHR(EGL_DEBUG_MSG_INFO_KHR, &result));
    EXPECT_EGL_FALSE(result);

    EXPECT_EGL_TRUE(eglQueryDebugKHR(EGL_DEBUG_CALLBACK_KHR, &result));
    EXPECT_EQ(nullptr, EGLAttribToDebugCallback(result));
}

// Check that the message control and callback parameters can be set and then queried back
TEST_P(EGLDebugTest, SetMessageControl)
{
    ANGLE_SKIP_TEST_IF(!hasExtension());

    EGLAttrib controls[] = {
        EGL_DEBUG_MSG_CRITICAL_KHR,
        EGL_FALSE,
        // EGL_DEBUG_MSG_ERROR_KHR left unset
        EGL_DEBUG_MSG_WARN_KHR,
        EGL_TRUE,
        EGL_DEBUG_MSG_INFO_KHR,
        EGL_FALSE,
        EGL_NONE,
        EGL_NONE,
    };

    EXPECT_EQ(static_cast<EGLint>(EGL_SUCCESS), eglDebugMessageControlKHR(&StubCallback, controls));

    EGLAttrib result = 0;

    EXPECT_EGL_TRUE(eglQueryDebugKHR(EGL_DEBUG_MSG_CRITICAL_KHR, &result));
    EXPECT_EGL_FALSE(result);

    EXPECT_EGL_TRUE(eglQueryDebugKHR(EGL_DEBUG_MSG_ERROR_KHR, &result));
    EXPECT_EGL_TRUE(result);

    EXPECT_EGL_TRUE(eglQueryDebugKHR(EGL_DEBUG_MSG_WARN_KHR, &result));
    EXPECT_EGL_TRUE(result);

    EXPECT_EGL_TRUE(eglQueryDebugKHR(EGL_DEBUG_MSG_INFO_KHR, &result));
    EXPECT_EGL_FALSE(result);

    EXPECT_EGL_TRUE(eglQueryDebugKHR(EGL_DEBUG_CALLBACK_KHR, &result));
    EXPECT_EQ(DebugCallbackToEGLAttrib(&StubCallback), result);
}

// Set a thread label and then trigger a callback to verify the callback parameters are correct
TEST_P(EGLDebugTest, CorrectCallbackParameters)
{
    ANGLE_SKIP_TEST_IF(!hasExtension());

    EXPECT_EQ(static_cast<EGLint>(EGL_SUCCESS), eglDebugMessageControlKHR(nullptr, nullptr));

    EXPECT_EQ(EGL_SUCCESS, eglLabelObjectKHR(EGL_NO_DISPLAY, EGL_OBJECT_THREAD_KHR, nullptr,
                                             const_cast<char *>("Thread")));

    // Enable all messages
    EGLAttrib controls[] = {
        EGL_DEBUG_MSG_CRITICAL_KHR,
        EGL_TRUE,
        EGL_DEBUG_MSG_ERROR_KHR,
        EGL_TRUE,
        EGL_DEBUG_MSG_WARN_KHR,
        EGL_TRUE,
        EGL_DEBUG_MSG_INFO_KHR,
        EGL_TRUE,
        EGL_NONE,
        EGL_NONE,
    };

    EXPECT_EQ(static_cast<EGLint>(EGL_SUCCESS),
              eglDebugMessageControlKHR(&CheckBadBindAPIError, controls));

    // Generate an error and trigger the callback
    EXPECT_EGL_FALSE(eglBindAPI(0xBADDBADD));
}

// Test that labels can be set and that errors are generated if the wrong object type is used
TEST_P(EGLDebugTest, SetLabel)
{
    ANGLE_SKIP_TEST_IF(!hasExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLSurface surface = getEGLWindow()->getSurface();

    EXPECT_EQ(static_cast<EGLint>(EGL_SUCCESS), eglDebugMessageControlKHR(nullptr, nullptr));

    // Display display and object must be equal when setting a display label
    EXPECT_EQ(
        static_cast<EGLint>(EGL_SUCCESS),
        eglLabelObjectKHR(display, EGL_OBJECT_DISPLAY_KHR, display, const_cast<char *>("Display")));
    EXPECT_NE(static_cast<EGLint>(EGL_SUCCESS),
              eglLabelObjectKHR(nullptr, EGL_OBJECT_DISPLAY_KHR, getEGLWindow()->getDisplay(),
                                const_cast<char *>("Display")));

    //  Set a surface label
    EXPECT_EQ(
        static_cast<EGLint>(EGL_SUCCESS),
        eglLabelObjectKHR(display, EGL_OBJECT_SURFACE_KHR, surface, const_cast<char *>("Surface")));
    EXPECT_EGL_ERROR(EGL_SUCCESS);

    // Provide a surface but use an image label type
    EXPECT_EQ(
        static_cast<EGLint>(EGL_BAD_PARAMETER),
        eglLabelObjectKHR(display, EGL_OBJECT_IMAGE_KHR, surface, const_cast<char *>("Image")));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLDebugTest);
ANGLE_INSTANTIATE_TEST(EGLDebugTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_VULKAN());

}  // namespace angle
