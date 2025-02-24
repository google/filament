//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLContextASANTest.cpp:
//   Tests relating to ASAN errors regarding context.

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

#include <condition_variable>
#include <mutex>
#include <thread>

using namespace angle;

namespace
{

EGLBoolean SafeDestroyContext(EGLDisplay display, EGLContext &context)
{
    EGLBoolean result = EGL_TRUE;
    if (context != EGL_NO_CONTEXT)
    {
        result  = eglDestroyContext(display, context);
        context = EGL_NO_CONTEXT;
    }
    return result;
}

class EGLContextASANTest : public ANGLETest<>
{
  public:
    EGLContextASANTest() {}
};

// Tests that creating resources works after freeing the share context.
TEST_P(EGLContextASANTest, DestroyContextInUse)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    EGLDisplay display = getEGLWindow()->getDisplay();
    EGLConfig config   = getEGLWindow()->getConfig();
    EGLSurface surface = getEGLWindow()->getSurface();

    const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION,
                                     getEGLWindow()->getClientMajorVersion(), EGL_NONE};

    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);
    ASSERT_EGL_SUCCESS();
    ASSERT_TRUE(context != EGL_NO_CONTEXT);

    // Synchronization tools to ensure the two threads are interleaved as designed by this test.
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread1Draw,
        Thread0Delete,
        Thread1Draw2,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    // Helper functions to synchronize the threads so that the operations are executed in the
    // specific order the test is written for.
    auto waitForStep = [&](Step waitStep) -> bool {
        std::unique_lock<std::mutex> lock(mutex);
        while (currentStep != waitStep)
        {
            // If necessary, abort execution as the other thread has encountered a GL error.
            if (currentStep == Step::Abort)
            {
                return false;
            }
            condVar.wait(lock);
        }

        return true;
    };
    auto nextStep = [&](Step newStep) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            currentStep = newStep;
        }
        condVar.notify_one();
    };

    class AbortOnFailure
    {
      public:
        AbortOnFailure(Step *currentStep, std::mutex *mutex, std::condition_variable *condVar)
            : mCurrentStep(currentStep), mMutex(mutex), mCondVar(condVar)
        {}

        ~AbortOnFailure()
        {
            bool isAborting = false;
            {
                std::unique_lock<std::mutex> lock(*mMutex);
                isAborting = *mCurrentStep != Step::Finish;

                if (isAborting)
                {
                    *mCurrentStep = Step::Abort;
                }
            }
            mCondVar->notify_all();
        }

      private:
        Step *mCurrentStep;
        std::mutex *mMutex;
        std::condition_variable *mCondVar;
    };

    // Unmake current to release the "surface".
    EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_SUCCESS();

    std::thread deletingThread = std::thread([&]() {
        AbortOnFailure abortOnFailure(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
        EXPECT_GL_NO_ERROR();

        // Wait for other thread to draw
        ASSERT_TRUE(waitForStep(Step::Thread1Draw));

        // Delete the context, if implemented properly this is a no-op because the context is
        // current in another thread.
        SafeDestroyContext(display, context);

        // Wait for the other thread to use context again
        nextStep(Step::Thread0Delete);
        ASSERT_TRUE(waitForStep(Step::Finish));
    });

    std::thread continuingThread = std::thread([&]() {
        EGLContext localContext = context;
        AbortOnFailure abortOnFailure(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, localContext));
        EXPECT_EGL_SUCCESS();

        constexpr GLsizei kTexSize = 1;
        const GLColor kTexData     = GLColor::red;

        GLTexture tex;
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     &kTexData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        GLProgram program;
        program.makeRaster(essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
        ASSERT_TRUE(program.valid());

        // Draw using the texture.
        drawQuad(program.get(), essl1_shaders::PositionAttrib(), 0.5f);

        EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, localContext));
        EXPECT_EGL_SUCCESS();

        // Wait for the other thread to delete the context.
        nextStep(Step::Thread1Draw);
        ASSERT_TRUE(waitForStep(Step::Thread0Delete));

        EXPECT_EGL_TRUE(eglMakeCurrent(display, surface, surface, localContext));
        EXPECT_EGL_SUCCESS();

        // Draw again. If the context has been inappropriately deleted in thread0 this will cause a
        // use-after-free error.
        drawQuad(program.get(), essl1_shaders::PositionAttrib(), 0.5f);

        nextStep(Step::Finish);

        EXPECT_EGL_TRUE(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    deletingThread.join();
    continuingThread.join();

    ASSERT_NE(currentStep, Step::Abort);

    // Make default context and surface current again.
    EXPECT_TRUE(getEGLWindow()->makeCurrent());
    EXPECT_EGL_SUCCESS();

    // cleanup
    ASSERT_GL_NO_ERROR();
}
}  // anonymous namespace

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLContextASANTest);
ANGLE_INSTANTIATE_TEST(EGLContextASANTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_VULKAN(),
                       ES3_VULKAN());
