//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLSyncTest.cpp:
//   Tests of EGL_KHR_fence_sync and EGL_KHR_wait_sync extensions.

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

#include <condition_variable>

using namespace angle;

class EGLSyncTest : public ANGLETest<>
{
  protected:
    bool hasFenceSyncExtension() const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), "EGL_KHR_fence_sync");
    }
    bool hasWaitSyncExtension() const
    {
        return hasFenceSyncExtension() &&
               IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(), "EGL_KHR_wait_sync");
    }
    bool hasGLSyncExtension() const { return IsGLExtensionEnabled("GL_OES_EGL_sync"); }

    bool hasAndroidNativeFenceSyncExtension() const
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(),
                                            "EGL_ANDROID_native_fence_sync");
    }
};

// Test error cases for all EGL_KHR_fence_sync functions
TEST_P(EGLSyncTest, FenceSyncErrors)
{
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();

    // If the client API doesn't have the necessary extension, test that sync creation fails and
    // ignore the rest of the tests.
    if (!hasGLSyncExtension())
    {
        EXPECT_EQ(EGL_NO_SYNC_KHR, eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr));
        EXPECT_EGL_ERROR(EGL_BAD_MATCH);
    }

    ANGLE_SKIP_TEST_IF(!hasGLSyncExtension());

    EGLContext context     = eglGetCurrentContext();
    EGLSurface drawSurface = eglGetCurrentSurface(EGL_DRAW);
    EGLSurface readSurface = eglGetCurrentSurface(EGL_READ);

    EXPECT_NE(context, EGL_NO_CONTEXT);
    EXPECT_NE(drawSurface, EGL_NO_SURFACE);
    EXPECT_NE(readSurface, EGL_NO_SURFACE);

    // CreateSync with no attribute shouldn't cause an error
    EGLSyncKHR sync = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
    EXPECT_NE(sync, EGL_NO_SYNC_KHR);

    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, sync));

    // CreateSync with empty attribute shouldn't cause an error
    const EGLint emptyAttributes[] = {EGL_NONE};
    sync                           = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, emptyAttributes);
    EXPECT_NE(sync, EGL_NO_SYNC_KHR);

    // DestroySync generates BAD_PARAMETER if the sync is not valid
    EXPECT_EGL_FALSE(eglDestroySyncKHR(display, reinterpret_cast<EGLSyncKHR>(20)));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    // CreateSync generates BAD_DISPLAY if display is not valid
    EXPECT_EQ(EGL_NO_SYNC_KHR, eglCreateSyncKHR(EGL_NO_DISPLAY, EGL_SYNC_FENCE_KHR, nullptr));
    EXPECT_EGL_ERROR(EGL_BAD_DISPLAY);

    // CreateSync generates BAD_ATTRIBUTE if attribute is neither nullptr nor empty.
    const EGLint nonEmptyAttributes[] = {
        EGL_CL_EVENT_HANDLE,
        0,
        EGL_NONE,
    };
    EXPECT_EQ(EGL_NO_SYNC_KHR, eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nonEmptyAttributes));
    EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);

    // CreateSync generates BAD_ATTRIBUTE if type is not valid
    EXPECT_EQ(EGL_NO_SYNC_KHR, eglCreateSyncKHR(display, 0, nullptr));
    EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);

    // CreateSync generates BAD_MATCH if no context is current
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    EXPECT_EQ(EGL_NO_SYNC_KHR, eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr));
    EXPECT_EGL_ERROR(EGL_BAD_MATCH);
    eglMakeCurrent(display, drawSurface, readSurface, context);

    // ClientWaitSync generates EGL_BAD_PARAMETER if the sync object is not valid
    EXPECT_EGL_FALSE(eglClientWaitSyncKHR(display, reinterpret_cast<EGLSyncKHR>(30), 0, 0));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    // GetSyncAttrib generates EGL_BAD_PARAMETER if the sync object is not valid, and value is not
    // modified
    constexpr EGLint kSentinelAttribValue = 123456789;
    EGLint attribValue                    = kSentinelAttribValue;
    EXPECT_EGL_FALSE(eglGetSyncAttribKHR(display, reinterpret_cast<EGLSyncKHR>(40),
                                         EGL_SYNC_TYPE_KHR, &attribValue));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    EXPECT_EQ(attribValue, kSentinelAttribValue);

    // GetSyncAttrib generates EGL_BAD_ATTRIBUTE if the attribute is not valid, and value is not
    // modified
    EXPECT_EGL_FALSE(eglGetSyncAttribKHR(display, sync, EGL_CL_EVENT_HANDLE, &attribValue));
    EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    EXPECT_EQ(attribValue, kSentinelAttribValue);

    // GetSyncAttrib generates EGL_BAD_MATCH if the attribute is valid for sync, but not the
    // particular sync type. We don't have such a case at the moment.

    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, sync));
}

// Test error cases for all EGL_KHR_wait_sync functions
TEST_P(EGLSyncTest, WaitSyncErrors)
{
    // The client API that shows support for eglWaitSyncKHR is the same as the one required for
    // eglCreateSyncKHR.  As such, there is no way to create a sync and not be able to wait on it.
    // This would have created an EGL_BAD_MATCH error.
    ANGLE_SKIP_TEST_IF(!hasWaitSyncExtension() || !hasGLSyncExtension());

    EGLDisplay display     = getEGLWindow()->getDisplay();
    EGLContext context     = eglGetCurrentContext();
    EGLSurface drawSurface = eglGetCurrentSurface(EGL_DRAW);
    EGLSurface readSurface = eglGetCurrentSurface(EGL_READ);

    EXPECT_NE(context, EGL_NO_CONTEXT);
    EXPECT_NE(drawSurface, EGL_NO_SURFACE);
    EXPECT_NE(readSurface, EGL_NO_SURFACE);

    EGLSyncKHR sync = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
    EXPECT_NE(sync, EGL_NO_SYNC_KHR);

    // WaitSync generates BAD_MATCH if no context is current
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    EXPECT_EGL_FALSE(eglWaitSyncKHR(display, sync, 0));
    EXPECT_EGL_ERROR(EGL_BAD_MATCH);
    eglMakeCurrent(display, drawSurface, readSurface, context);

    // WaitSync generates BAD_PARAMETER if the sync is not valid
    EXPECT_EGL_FALSE(eglWaitSyncKHR(display, reinterpret_cast<EGLSyncKHR>(20), 0));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    // WaitSync generates BAD_PARAMETER if flags is non-zero
    EXPECT_EGL_FALSE(eglWaitSyncKHR(display, sync, 1));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, sync));
}

// Test usage of eglGetSyncAttribKHR
TEST_P(EGLSyncTest, GetSyncAttrib)
{
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLSyncKHR sync = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
    EXPECT_NE(sync, EGL_NO_SYNC_KHR);

    // Fence sync attributes are:
    //
    // EGL_SYNC_TYPE_KHR: EGL_SYNC_FENCE_KHR
    // EGL_SYNC_STATUS_KHR: EGL_UNSIGNALED_KHR or EGL_SIGNALED_KHR
    // EGL_SYNC_CONDITION_KHR: EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR

    constexpr EGLint kSentinelAttribValue = 123456789;
    EGLint attribValue                    = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, sync, EGL_SYNC_TYPE_KHR, &attribValue));
    EXPECT_EQ(attribValue, EGL_SYNC_FENCE_KHR);

    attribValue = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, sync, EGL_SYNC_CONDITION_KHR, &attribValue));
    EXPECT_EQ(attribValue, EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR);

    attribValue = kSentinelAttribValue;
    EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, sync, EGL_SYNC_STATUS_KHR, &attribValue));

    // Hack around EXPECT_* not having an "either this or that" variant:
    if (attribValue != EGL_SIGNALED_KHR)
    {
        EXPECT_EQ(attribValue, EGL_UNSIGNALED_KHR);
    }

    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, sync));
}

// Test that basic usage works and doesn't generate errors or crash
TEST_P(EGLSyncTest, BasicOperations)
{
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLSyncKHR sync = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
    EXPECT_NE(sync, EGL_NO_SYNC_KHR);

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_EGL_TRUE(eglWaitSyncKHR(display, sync, 0));

    glFlush();

    glClear(GL_COLOR_BUFFER_BIT);

    // Don't wait forever to make sure the test terminates
    constexpr GLuint64 kTimeout = 1'000'000'000;  // 1 second
    EGLint value                = 0;
    ASSERT_EQ(EGL_CONDITION_SATISFIED_KHR,
              eglClientWaitSyncKHR(display, sync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, kTimeout));

    for (size_t i = 0; i < 20; i++)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_EQ(
            EGL_CONDITION_SATISFIED_KHR,
            eglClientWaitSyncKHR(display, sync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, EGL_FOREVER_KHR));
        EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, sync, EGL_SYNC_STATUS_KHR, &value));
        EXPECT_EQ(value, EGL_SIGNALED_KHR);
    }

    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, sync));
}

// Test that eglClientWaitSync* APIs work.
TEST_P(EGLSyncTest, EglClientWaitSync)
{
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();
    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    // Test eglClientWaitSyncKHR
    for (size_t i = 0; i < 5; i++)
    {
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(greenProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Don't wait forever to make sure the test terminates
        constexpr GLuint64 kTimeout = 1'000'000'000;  // 1 second
        EGLSyncKHR clientWaitSync   = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
        EXPECT_NE(clientWaitSync, EGL_NO_SYNC_KHR);

        ASSERT_EQ(EGL_CONDITION_SATISFIED_KHR,
                  eglClientWaitSyncKHR(display, clientWaitSync, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR,
                                       kTimeout));

        EXPECT_EGL_TRUE(eglDestroySyncKHR(display, clientWaitSync));
        ASSERT_EGL_SUCCESS();
    }

    // Test eglClientWaitSync
    for (size_t i = 0; i < 5; i++)
    {
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(greenProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Don't wait forever to make sure the test terminates
        constexpr GLuint64 kTimeout = 1'000'000'000;  // 1 second
        EGLSyncKHR clientWaitSync   = eglCreateSync(display, EGL_SYNC_FENCE, nullptr);
        EXPECT_NE(clientWaitSync, EGL_NO_SYNC);

        ASSERT_EQ(
            EGL_CONDITION_SATISFIED,
            eglClientWaitSync(display, clientWaitSync, EGL_SYNC_FLUSH_COMMANDS_BIT, kTimeout));

        EXPECT_EGL_TRUE(eglDestroySync(display, clientWaitSync));
        ASSERT_EGL_SUCCESS();
    }
}

// Test eglWaitClient api
TEST_P(EGLSyncTest, WaitClient)
{
    // Clear to red color
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_EGL_TRUE(eglWaitClient());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::red);

    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLContext context = getEGLWindow()->getContext();
    EGLSurface surface = getEGLWindow()->getSurface();
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    EXPECT_EGL_TRUE(eglWaitClient());
    eglMakeCurrent(display, surface, surface, context);
}

// Test eglWaitGL api
TEST_P(EGLSyncTest, WaitGL)
{
    // Clear to red color
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_EGL_TRUE(eglWaitGL());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::red);

    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLContext context = getEGLWindow()->getContext();
    EGLSurface surface = getEGLWindow()->getSurface();
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    EXPECT_EGL_TRUE(eglWaitGL());
    eglMakeCurrent(display, surface, surface, context);
}

// Test eglWaitNative api
TEST_P(EGLSyncTest, WaitNative)
{
    // Clear to red color
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_EGL_TRUE(eglWaitNative(EGL_CORE_NATIVE_ENGINE));
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::red);

    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLContext context = getEGLWindow()->getContext();
    EGLSurface surface = getEGLWindow()->getSurface();
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    EXPECT_EGL_TRUE(eglWaitNative(EGL_CORE_NATIVE_ENGINE));
    eglMakeCurrent(display, surface, surface, context);
}

// Verify eglDupNativeFence for EGL_ANDROID_native_fence_sync
TEST_P(EGLSyncTest, AndroidNativeFence_DupNativeFenceFD)
{
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasAndroidNativeFenceSyncExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();

    // We can ClientWait on this
    EGLSyncKHR syncWithGeneratedFD =
        eglCreateSyncKHR(display, EGL_SYNC_NATIVE_FENCE_ANDROID, nullptr);
    EXPECT_NE(syncWithGeneratedFD, EGL_NO_SYNC_KHR);

    int fd = eglDupNativeFenceFDANDROID(display, syncWithGeneratedFD);
    EXPECT_EGL_SUCCESS();

    // Clean up created objects.
    if (fd != EGL_NO_NATIVE_FENCE_FD_ANDROID)
    {
        close(fd);
    }

    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, syncWithGeneratedFD));
}

// Test the validation errors for bad parameters for eglDupNativeFenceFDANDROID
TEST_P(EGLSyncTest, AndroidNativeFence_DupNativeFenceFD_NegativeValidation)
{
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasAndroidNativeFenceSyncExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();

    int fd;
    fd = eglDupNativeFenceFDANDROID(display, EGL_NO_SYNC_KHR);
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);
    EXPECT_EQ(fd, EGL_NO_NATIVE_FENCE_FD_ANDROID);
}

// Verify CreateSync and ClientWait for EGL_ANDROID_native_fence_sync
TEST_P(EGLSyncTest, AndroidNativeFence_ClientWait)
{
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasAndroidNativeFenceSyncExtension());

    EGLint value       = 0;
    EGLDisplay display = getEGLWindow()->getDisplay();

    // We can ClientWait on this
    EGLSyncKHR syncWithGeneratedFD =
        eglCreateSyncKHR(display, EGL_SYNC_NATIVE_FENCE_ANDROID, nullptr);
    EXPECT_NE(syncWithGeneratedFD, EGL_NO_SYNC_KHR);

    // Create work to do
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    // Wait for draw to complete
    EXPECT_EQ(EGL_CONDITION_SATISFIED,
              eglClientWaitSyncKHR(display, syncWithGeneratedFD, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR,
                                   1'000'000'000));
    EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, syncWithGeneratedFD, EGL_SYNC_STATUS_KHR, &value));
    EXPECT_EQ(value, EGL_SIGNALED_KHR);

    // Clean up created objects.
    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, syncWithGeneratedFD));
}

// Verify WaitSync with EGL_ANDROID_native_fence_sync
// Simulate passing FDs across processes by passing across Contexts.
TEST_P(EGLSyncTest, AndroidNativeFence_WaitSync)
{
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasWaitSyncExtension() || !hasGLSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasAndroidNativeFenceSyncExtension());

    EGLint value       = 0;
    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLSurface surface = getEGLWindow()->getSurface();

    /*- First Context ------------------------*/

    // We can ClientWait on this
    EGLSyncKHR syncWithGeneratedFD =
        eglCreateSyncKHR(display, EGL_SYNC_NATIVE_FENCE_ANDROID, nullptr);
    EXPECT_NE(syncWithGeneratedFD, EGL_NO_SYNC_KHR);

    int fd = eglDupNativeFenceFDANDROID(display, syncWithGeneratedFD);
    EXPECT_EGL_SUCCESS();  // Can return -1 (when signaled) or valid FD.

    // Create work to do
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    /*- Second Context ------------------------*/
    if (fd > EGL_NO_NATIVE_FENCE_FD_ANDROID)
    {
        EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        EGLContext context2 = getEGLWindow()->createContext(EGL_NO_CONTEXT, nullptr);
        EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, context2));

        // We can eglWaitSync on this - import FD from first sync.
        EGLint syncAttribs[] = {EGL_SYNC_NATIVE_FENCE_FD_ANDROID, (EGLint)fd, EGL_NONE};
        EGLSyncKHR syncWithDupFD =
            eglCreateSyncKHR(display, EGL_SYNC_NATIVE_FENCE_ANDROID, syncAttribs);
        EXPECT_NE(syncWithDupFD, EGL_NO_SYNC_KHR);

        // Second draw waits for first to complete. May already be signaled - ignore error.
        if (eglWaitSyncKHR(display, syncWithDupFD, 0) == EGL_TRUE)
        {
            // Create work to do
            glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glFlush();
        }

        // Wait for second draw to complete
        EXPECT_EQ(EGL_CONDITION_SATISFIED,
                  eglClientWaitSyncKHR(display, syncWithDupFD, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR,
                                       1000000000));
        EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, syncWithDupFD, EGL_SYNC_STATUS_KHR, &value));
        EXPECT_EQ(value, EGL_SIGNALED_KHR);

        // Reset to default context and surface.
        EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, getEGLWindow()->getContext()));

        // Clean up created objects.
        EXPECT_EGL_TRUE(eglDestroySyncKHR(display, syncWithDupFD));
        EXPECT_EGL_TRUE(eglDestroyContext(display, context2));
    }

    // Wait for first draw to complete
    EXPECT_EQ(EGL_CONDITION_SATISFIED,
              eglClientWaitSyncKHR(display, syncWithGeneratedFD, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR,
                                   1000000000));
    EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, syncWithGeneratedFD, EGL_SYNC_STATUS_KHR, &value));
    EXPECT_EQ(value, EGL_SIGNALED_KHR);

    // Clean up created objects.
    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, syncWithGeneratedFD));
}

// Verify EGL_ANDROID_native_fence_sync
// Simulate passing FDs across processes by passing across Contexts.
TEST_P(EGLSyncTest, AndroidNativeFence_withFences)
{
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasWaitSyncExtension() || !hasGLSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasAndroidNativeFenceSyncExtension());

    EGLint value       = 0;
    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLSurface surface = getEGLWindow()->getSurface();

    /*- First Context ------------------------*/

    // Extra fence syncs to ensure that Fence and Android Native fences work together
    EGLSyncKHR syncFence1 = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
    EXPECT_NE(syncFence1, EGL_NO_SYNC_KHR);

    // We can ClientWait on this
    EGLSyncKHR syncWithGeneratedFD =
        eglCreateSyncKHR(display, EGL_SYNC_NATIVE_FENCE_ANDROID, nullptr);
    EXPECT_NE(syncWithGeneratedFD, EGL_NO_SYNC_KHR);

    int fd = eglDupNativeFenceFDANDROID(display, syncWithGeneratedFD);
    EXPECT_EGL_SUCCESS();  // Can return -1 (when signaled) or valid FD.

    EGLSyncKHR syncFence2 = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
    EXPECT_NE(syncFence2, EGL_NO_SYNC_KHR);

    // Create work to do
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();

    /*- Second Context ------------------------*/
    if (fd > EGL_NO_NATIVE_FENCE_FD_ANDROID)
    {
        EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        EGLContext context2 = getEGLWindow()->createContext(EGL_NO_CONTEXT, nullptr);
        EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, context2));

        // check that Fence and Android fences work together
        EGLSyncKHR syncFence3 = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
        EXPECT_NE(syncFence3, EGL_NO_SYNC_KHR);

        // We can eglWaitSync on this
        EGLint syncAttribs[] = {EGL_SYNC_NATIVE_FENCE_FD_ANDROID, (EGLint)fd, EGL_NONE};
        EGLSyncKHR syncWithDupFD =
            eglCreateSyncKHR(display, EGL_SYNC_NATIVE_FENCE_ANDROID, syncAttribs);
        EXPECT_NE(syncWithDupFD, EGL_NO_SYNC_KHR);

        EGLSyncKHR syncFence4 = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
        EXPECT_NE(syncFence4, EGL_NO_SYNC_KHR);

        // Second draw waits for first to complete. May already be signaled - ignore error.
        if (eglWaitSyncKHR(display, syncWithDupFD, 0) == EGL_TRUE)
        {
            // Create work to do
            glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glFlush();
        }

        // Wait for second draw to complete
        EXPECT_EQ(EGL_CONDITION_SATISFIED,
                  eglClientWaitSyncKHR(display, syncWithDupFD, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR,
                                       1000000000));
        EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, syncWithDupFD, EGL_SYNC_STATUS_KHR, &value));
        EXPECT_EQ(value, EGL_SIGNALED_KHR);

        // Reset to default context and surface.
        EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, getEGLWindow()->getContext()));

        // Clean up created objects.
        EXPECT_EGL_TRUE(eglDestroySyncKHR(display, syncFence3));
        EXPECT_EGL_TRUE(eglDestroySyncKHR(display, syncFence4));
        EXPECT_EGL_TRUE(eglDestroySyncKHR(display, syncWithDupFD));
        EXPECT_EGL_TRUE(eglDestroyContext(display, context2));
    }

    // Wait for first draw to complete
    EXPECT_EQ(EGL_CONDITION_SATISFIED,
              eglClientWaitSyncKHR(display, syncWithGeneratedFD, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR,
                                   1000000000));
    EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, syncWithGeneratedFD, EGL_SYNC_STATUS_KHR, &value));
    EXPECT_EQ(value, EGL_SIGNALED_KHR);

    // Clean up created objects.
    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, syncFence1));
    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, syncFence2));
    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, syncWithGeneratedFD));
}

// Verify that VkSemaphore is not destroyed before used for waiting
TEST_P(EGLSyncTest, AndroidNativeFence_VkSemaphoreDestroyBug)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasWaitSyncExtension() || !hasGLSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasAndroidNativeFenceSyncExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();

    glFinish();  // Ensure no pending commands

    EGLSyncKHR syncWithGeneratedFD =
        eglCreateSyncKHR(display, EGL_SYNC_NATIVE_FENCE_ANDROID, nullptr);
    EXPECT_NE(syncWithGeneratedFD, EGL_NO_SYNC_KHR);
    EXPECT_EGL_TRUE(eglWaitSyncKHR(display, syncWithGeneratedFD, 0));
    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, syncWithGeneratedFD));
    glFinish();  // May destroy VkSemaphore if bug is present.

    // Create work to do
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFinish();  // Will submit destroyed Semaphores.
}

// Verify that no VVL errors are generated when External Fence Handle is used to track submissions
TEST_P(EGLSyncTest, AndroidNativeFence_ExternalFenceWaitVVLBug)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());
    ANGLE_SKIP_TEST_IF(!hasAndroidNativeFenceSyncExtension());

    EGLint value       = 0;
    EGLDisplay display = getEGLWindow()->getDisplay();

    // Create work to do
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // We can ClientWait on this
    EGLSyncKHR syncWithGeneratedFD =
        eglCreateSyncKHR(display, EGL_SYNC_NATIVE_FENCE_ANDROID, nullptr);
    EXPECT_NE(syncWithGeneratedFD, EGL_NO_SYNC_KHR);

    // Wait for draw to complete
    EXPECT_EQ(EGL_CONDITION_SATISFIED,
              eglClientWaitSyncKHR(display, syncWithGeneratedFD, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR,
                                   1'000'000'000));
    EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, syncWithGeneratedFD, EGL_SYNC_STATUS_KHR, &value));
    EXPECT_EQ(value, EGL_SIGNALED_KHR);

    // Clean up created objects.
    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, syncWithGeneratedFD));

    // Finish to cleanup internal garbage in the backend.
    glFinish();
}

// Test functionality of EGL_ANGLE_global_fence_sync.
TEST_P(EGLSyncTest, GlobalFenceSync)
{
    EGLDisplay display = getEGLWindow()->getDisplay();

    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension());
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(display, "EGL_ANGLE_global_fence_sync"));

    // Create a second context
    EGLContext context1     = eglGetCurrentContext();
    EGLSurface drawSurface1 = eglGetCurrentSurface(EGL_DRAW);
    EGLSurface readSurface1 = eglGetCurrentSurface(EGL_READ);
    EGLConfig config        = getEGLWindow()->getConfig();

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, getEGLWindow()->getClientMajorVersion(),
        EGL_CONTEXT_MINOR_VERSION_KHR, getEGLWindow()->getClientMinorVersion(), EGL_NONE};

    EGLContext context2 = eglCreateContext(display, config, context1, contextAttribs);
    ASSERT_NE(EGL_NO_CONTEXT, context2);

    const EGLint pbufferAttribs[] = {EGL_WIDTH, getWindowWidth(), EGL_HEIGHT, getWindowHeight(),
                                     EGL_NONE};
    EGLSurface drawSurface2       = eglCreatePbufferSurface(display, config, pbufferAttribs);
    ASSERT_NE(EGL_NO_SURFACE, drawSurface2);

    // Do an expensive draw in context 2
    eglMakeCurrent(display, drawSurface2, drawSurface2, context2);

    constexpr char kCostlyVS[] = R"(attribute highp vec4 position;
varying highp vec4 testPos;
void main(void)
{
    testPos     = position;
    gl_Position = position;
})";

    constexpr char kCostlyFS[] = R"(precision highp float;
varying highp vec4 testPos;
void main(void)
{
    vec4 test = testPos;
    for (int i = 0; i < 500; i++)
    {
        test = sqrt(test);
    }
    gl_FragColor = test;
})";

    ANGLE_GL_PROGRAM(expensiveProgram, kCostlyVS, kCostlyFS);
    drawQuad(expensiveProgram, "position", 0.0f);

    // Signal a fence sync for testing
    EGLSyncKHR sync2 = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);

    // Switch to context 1, and create a global fence sync
    eglMakeCurrent(display, drawSurface1, readSurface1, context1);

    EGLSyncKHR sync1 = eglCreateSyncKHR(display, EGL_SYNC_GLOBAL_FENCE_ANGLE, nullptr);

    // Wait for the global fence sync to finish.
    constexpr GLuint64 kTimeout = 1'000'000'000;  // 1 second
    ASSERT_EQ(EGL_CONDITION_SATISFIED_KHR, eglClientWaitSyncKHR(display, sync1, 0, kTimeout));

    // If the global fence sync is signaled, then the signal from context2 must also be signaled.
    // Note that if sync1 was an EGL_SYNC_FENCE_KHR, this would not necessarily be true.
    EGLint value = 0;
    EXPECT_EGL_TRUE(eglGetSyncAttribKHR(display, sync2, EGL_SYNC_STATUS_KHR, &value));
    EXPECT_EQ(value, EGL_SIGNALED_KHR);

    EXPECT_EQ(EGL_CONDITION_SATISFIED_KHR, eglClientWaitSyncKHR(display, sync2, 0, 0));

    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, sync1));
    EXPECT_EGL_TRUE(eglDestroySyncKHR(display, sync2));

    EXPECT_EGL_TRUE(eglDestroySurface(display, drawSurface2));
    EXPECT_EGL_TRUE(eglDestroyContext(display, context2));
}

// Test that leaked fences are cleaned up in a safe way. Regression test for sync objects using tail
// calls for destruction.
TEST_P(EGLSyncTest, DISABLED_LeakSyncToDisplayDestruction)
{
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLSyncKHR sync = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
    EXPECT_NE(sync, EGL_NO_SYNC_KHR);
}

// Test the validation errors for bad parameters for eglCreateSyncKHR
TEST_P(EGLSyncTest, NegativeValidationBadAttributes)
{
    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLSyncKHR sync;
    const EGLint invalidCreateSyncAttributeList[][3] = {
        {EGL_SYNC_CONDITION_KHR, EGL_NONE, 0},
        {EGL_SYNC_CONDITION_KHR, EGL_RENDERABLE_TYPE, EGL_NONE},
        {EGL_SYNC_CONDITION_KHR, EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR, EGL_RENDERABLE_TYPE},
    };

    for (size_t i = 0; i < 3; i++)
    {
        sync = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, &invalidCreateSyncAttributeList[i][0]);

        ASSERT_EQ(sync, EGL_NO_SYNC_KHR);
        ASSERT_EGL_ERROR(EGL_BAD_ATTRIBUTE);
    }
}

// Tests that eglClientWaitSyncKHR() is not blocking when Vulkan CommandQueue performs CPU
// throttling during submission (when `kInFlightCommandsLimit` is exceeded).
TEST_P(EGLSyncTest, BlockingOnSubmitCPUThrottling)
{
    ANGLE_SKIP_TEST_IF(!isVulkanRenderer() || isSwiftshader());
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    // Should be somewhat larger than the `kInFlightCommandsLimit`.  At the same time,
    // `kInFlightCommandsLimit` should be less than the internal driver limit, otherwise test will
    // not work.
    constexpr size_t kMaxSyncCount = 100;

    constexpr GLsizei kBufferResolution = 1024;
    constexpr size_t kDrawsPerSync      = 2;

    constexpr double kLongWaitThresholdMs = 5.0;
    constexpr size_t kMinLongWaitsToFail  = 5;

    constexpr char kCostlyVS[] = R"(attribute highp vec4 position;
varying highp vec4 testPos;
void main(void)
{
    testPos     = position;
    gl_Position = position;
})";

    constexpr char kCostlyFS[] = R"(precision highp float;
varying highp vec4 testPos;
void main(void)
{
    vec4 test = testPos;
    for (int i = 0; i < 500; i++)
    {
        test = sqrt(test);
    }
    gl_FragColor = test;
})";

    std::array<EGLSyncKHR, kMaxSyncCount> syncArray;
    size_t syncCount = 0;
    std::mutex mutex;
    std::condition_variable condVar;
    size_t numLongWaits        = 0;
    bool isSyncWaitThreadReady = false;

    EGLDisplay display = getEGLWindow()->getDisplay();

    std::thread syncWaitThread([&]() {
        constexpr GLuint64 kTimeout = 0;  // Just check status

        EGLConfig config = getEGLWindow()->getConfig();

        const EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, getEGLWindow()->getClientMajorVersion(),
            EGL_CONTEXT_MINOR_VERSION_KHR, getEGLWindow()->getClientMinorVersion(), EGL_NONE};

        EGLContext context2 = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
        ASSERT_NE(EGL_NO_CONTEXT, context2);

        const EGLint pbufferAttribs[] = {EGL_WIDTH, getWindowWidth(), EGL_HEIGHT, getWindowHeight(),
                                         EGL_NONE};
        EGLSurface drawSurface2       = eglCreatePbufferSurface(display, config, pbufferAttribs);
        ASSERT_NE(EGL_NO_SURFACE, drawSurface2);

        // Making some Context current just to prevent blocking in ANGLE_CAPTURE_EGL
        EXPECT_EGL_TRUE(eglMakeCurrent(display, drawSurface2, drawSurface2, context2));

        {
            std::unique_lock<std::mutex> lock(mutex);
            isSyncWaitThreadReady = true;
        }
        condVar.notify_one();

        for (size_t syncIndex = 0; syncIndex < kMaxSyncCount; ++syncIndex)
        {
            {
                std::unique_lock<std::mutex> lock(mutex);
                condVar.wait(lock, [&]() { return syncIndex < syncCount; });
            }
            while (true)
            {
                const double t1 = angle::GetCurrentSystemTime();
                EGLint result   = eglClientWaitSyncKHR(display, syncArray[syncIndex], 0, kTimeout);
                const double t2 = angle::GetCurrentSystemTime();
                if ((t2 - t1) * 1000.0 > kLongWaitThresholdMs)
                {
                    ++numLongWaits;
                }
                if (result != EGL_TIMEOUT_EXPIRED_KHR)
                {
                    EXPECT_EQ(result, EGL_CONDITION_SATISFIED_KHR);
                    break;
                }
                // Wait some time and try again...
                std::this_thread::sleep_for(std::chrono::microseconds(1000));
            }
        }

        EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_TRUE(eglDestroySurface(display, drawSurface2));
        EXPECT_EGL_TRUE(eglDestroyContext(display, context2));
    });

    // Prepare the framebuffer
    GLFramebuffer framebuffer;
    GLTexture fbTexture;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindTexture(GL_TEXTURE_2D, fbTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kBufferResolution, kBufferResolution);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTexture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT_GL_NO_ERROR();
    glViewport(0, 0, kBufferResolution, kBufferResolution);

    ANGLE_GL_PROGRAM(program, kCostlyVS, kCostlyFS);

    // Wait until thread is ready waiting on EGL sync objects...
    {
        std::unique_lock<std::mutex> lock(mutex);
        condVar.wait(lock, [&]() { return isSyncWaitThreadReady; });
    }

    while (syncCount < kMaxSyncCount)
    {
        // Perform GPU heavy rendering
        for (size_t i = 0; i < kDrawsPerSync; ++i)
        {
            drawQuad(program, "position", 0.0f);
            ASSERT_GL_NO_ERROR();
        }

        // Using glFenceSync() to force submission without also blocking on the EGL Global mutex.
        GLsync clientWaitSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        ASSERT_GL_NO_ERROR();
        constexpr GLuint64 kTimeout = 0;  // Just check status
        glClientWaitSync(clientWaitSync, GL_SYNC_FLUSH_COMMANDS_BIT, kTimeout);
        EXPECT_GL_NO_ERROR();
        glDeleteSync(clientWaitSync);
        ASSERT_GL_NO_ERROR();

        // Creating EGL sync should not block on submission, since glClientWaitSync() should have
        // already done that.
        EGLSyncKHR sync = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
        EXPECT_NE(sync, EGL_NO_SYNC_KHR);

        // Make sync ready for checking from another thread.
        syncArray[syncCount] = sync;
        {
            std::unique_lock<std::mutex> lock(mutex);
            ++syncCount;
        }
        condVar.notify_one();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    syncWaitThread.join();

    for (EGLSyncKHR sync : syncArray)
    {
        EXPECT_EGL_TRUE(eglDestroySyncKHR(display, sync));
    }

    EXPECT_LT(numLongWaits, kMinLongWaitsToFail);
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(EGLSyncTest);
