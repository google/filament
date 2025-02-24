//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLMultiContextTest.cpp:
//   Tests relating to multiple non-shared Contexts.

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "test_utils/MultiThreadSteps.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/test_utils.h"

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

class EGLMultiContextTest : public ANGLETest<>
{
  public:
    EGLMultiContextTest() : mContexts{EGL_NO_CONTEXT, EGL_NO_CONTEXT}, mTexture(0) {}

    void testTearDown() override
    {
        glDeleteTextures(1, &mTexture);

        EGLDisplay display = getEGLWindow()->getDisplay();

        if (display != EGL_NO_DISPLAY)
        {
            for (auto &context : mContexts)
            {
                SafeDestroyContext(display, context);
            }
        }

        // Set default test state to not give an error on shutdown.
        getEGLWindow()->makeCurrent();
    }

    bool chooseConfig(EGLDisplay dpy, EGLConfig *config) const
    {
        bool result          = false;
        EGLint count         = 0;
        EGLint clientVersion = EGL_OPENGL_ES3_BIT;
        EGLint attribs[]     = {EGL_RED_SIZE,
                                8,
                                EGL_GREEN_SIZE,
                                8,
                                EGL_BLUE_SIZE,
                                8,
                                EGL_ALPHA_SIZE,
                                8,
                                EGL_RENDERABLE_TYPE,
                                clientVersion,
                                EGL_SURFACE_TYPE,
                                EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
                                EGL_NONE};

        result = eglChooseConfig(dpy, attribs, config, 1, &count);
        EXPECT_EGL_TRUE(result && (count > 0));
        return result;
    }

    bool createContext(EGLDisplay dpy, EGLConfig config, EGLContext *context)
    {
        bool result      = false;
        EGLint attribs[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE};

        *context = eglCreateContext(dpy, config, nullptr, attribs);
        result   = (*context != EGL_NO_CONTEXT);
        EXPECT_TRUE(result);
        return result;
    }

    bool createPbufferSurface(EGLDisplay dpy,
                              EGLConfig config,
                              EGLint width,
                              EGLint height,
                              EGLSurface *surface)
    {
        bool result      = false;
        EGLint attribs[] = {EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE};

        *surface = eglCreatePbufferSurface(dpy, config, attribs);
        result   = (*surface != EGL_NO_SURFACE);
        EXPECT_TRUE(result);
        return result;
    }

    enum class FenceTest
    {
        ClientWait,
        ServerWait,
        GetStatus,
    };
    enum class FlushMethod
    {
        Flush,
        Finish,
    };
    void testFenceWithOpenRenderPass(FenceTest test, FlushMethod flushMethod);

    EGLContext mContexts[2];
    GLuint mTexture;
};

// Test that calling eglDeleteContext on a context that is not current succeeds.
TEST_P(EGLMultiContextTest, TestContextDestroySimple)
{
    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();

    EGLContext context1 = window->createContext(EGL_NO_CONTEXT, nullptr);
    EGLContext context2 = window->createContext(EGL_NO_CONTEXT, nullptr);

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, context1));
    EXPECT_EGL_TRUE(eglDestroyContext(dpy, context2));
    EXPECT_EGL_SUCCESS();

    // Cleanup
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_TRUE(eglDestroyContext(dpy, context1));
    EXPECT_EGL_SUCCESS();
}

// Test that an error is generated when using EGL objects after calling eglTerminate.
TEST_P(EGLMultiContextTest, NegativeTestAfterEglTerminate)
{
    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();

    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_TRUE(chooseConfig(dpy, &config));

    EGLContext context = EGL_NO_CONTEXT;
    EXPECT_TRUE(createContext(dpy, config, &context));
    ASSERT_EGL_SUCCESS() << "eglCreateContext failed.";

    EGLSurface drawSurface = EGL_NO_SURFACE;
    EXPECT_TRUE(createPbufferSurface(dpy, config, 2560, 1080, &drawSurface));
    ASSERT_EGL_SUCCESS() << "eglCreatePbufferSurface failed.";

    EGLSurface readSurface = EGL_NO_SURFACE;
    EXPECT_TRUE(createPbufferSurface(dpy, config, 2560, 1080, &readSurface));
    ASSERT_EGL_SUCCESS() << "eglCreatePbufferSurface failed.";

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, drawSurface, readSurface, context));
    EXPECT_EGL_SUCCESS();

    // Terminate the display
    EXPECT_EGL_TRUE(eglTerminate(dpy));
    EXPECT_EGL_SUCCESS();

    // Try to use invalid handles
    EGLint value;
    eglQuerySurface(dpy, drawSurface, EGL_SWAP_BEHAVIOR, &value);
    EXPECT_EGL_ERROR(EGL_BAD_SURFACE);
    eglQuerySurface(dpy, readSurface, EGL_HEIGHT, &value);
    EXPECT_EGL_ERROR(EGL_BAD_SURFACE);

    // Cleanup
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_SUCCESS();
    window->destroyGL();
}

// Test that a compute shader running in one thread will still work when rendering is happening in
// another thread (with non-shared contexts).  The non-shared context will still share a Vulkan
// command buffer.
TEST_P(EGLMultiContextTest, ComputeShaderOkayWithRendering)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!isVulkanRenderer());
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 || getClientMinorVersion() < 1);

    // Initialize contexts
    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();
    EGLConfig config  = window->getConfig();

    constexpr size_t kThreadCount    = 2;
    EGLSurface surface[kThreadCount] = {EGL_NO_SURFACE, EGL_NO_SURFACE};
    EGLContext ctx[kThreadCount]     = {EGL_NO_CONTEXT, EGL_NO_CONTEXT};

    EGLint pbufferAttributes[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE, EGL_NONE};

    for (size_t t = 0; t < kThreadCount; ++t)
    {
        surface[t] = eglCreatePbufferSurface(dpy, config, pbufferAttributes);
        EXPECT_EGL_SUCCESS();

        ctx[t] = window->createContext(EGL_NO_CONTEXT, nullptr);
        EXPECT_NE(EGL_NO_CONTEXT, ctx[t]);
    }

    // Synchronization tools to ensure the two threads are interleaved as designed by this test.
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Thread0Start,
        Thread0DispatchedCompute,
        Thread1Drew,
        Thread0DispatchedComputeAgain,
        Finish,
        Abort,
    };
    Step currentStep = Step::Thread0Start;

    // This first thread dispatches a compute shader.  It immediately starts.
    std::thread deletingThread = std::thread([&]() {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[0], surface[0], ctx[0]));
        EXPECT_EGL_SUCCESS();

        // Potentially wait to be signalled to start.
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0Start));

        // Wake up and do next step: Create, detach, and dispatch a compute shader program.
        constexpr char kCS[]  = R"(#version 310 es
layout(local_size_x=1) in;
void main()
{
})";
        GLuint computeProgram = glCreateProgram();
        GLuint cs             = CompileShader(GL_COMPUTE_SHADER, kCS);
        EXPECT_NE(0u, cs);

        glAttachShader(computeProgram, cs);
        glDeleteShader(cs);
        glLinkProgram(computeProgram);
        GLint linkStatus;
        glGetProgramiv(computeProgram, GL_LINK_STATUS, &linkStatus);
        EXPECT_GL_TRUE(linkStatus);
        glDetachShader(computeProgram, cs);
        EXPECT_GL_NO_ERROR();
        glUseProgram(computeProgram);

        glDispatchCompute(8, 4, 2);
        EXPECT_GL_NO_ERROR();

        // Signal the second thread and wait for it to draw and flush.
        threadSynchronization.nextStep(Step::Thread0DispatchedCompute);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1Drew));

        // Wake up and do next step: Dispatch the same compute shader again.
        glDispatchCompute(8, 4, 2);

        // Signal the second thread and wait for it to draw and flush again.
        threadSynchronization.nextStep(Step::Thread0DispatchedComputeAgain);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        // Wake up and do next step: Dispatch the same compute shader again, and force flush the
        // underlying command buffer.
        glDispatchCompute(8, 4, 2);
        glFinish();

        // Clean-up and exit this thread.
        EXPECT_GL_NO_ERROR();
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    // This second thread renders.  It starts once the other thread does its first nextStep()
    std::thread continuingThread = std::thread([&]() {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[1], surface[1], ctx[1]));
        EXPECT_EGL_SUCCESS();

        // Wait for first thread to create and dispatch a compute shader.
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0DispatchedCompute));

        // Wake up and do next step: Create graphics resources, draw, and force flush the
        // underlying command buffer.
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        GLRenderbuffer renderbuffer;
        GLFramebuffer fbo;
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        constexpr int kRenderbufferSize = 4;
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kRenderbufferSize, kRenderbufferSize);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  renderbuffer);
        glBindTexture(GL_TEXTURE_2D, texture);

        GLProgram graphicsProgram;
        graphicsProgram.makeRaster(essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
        ASSERT_TRUE(graphicsProgram.valid());

        drawQuad(graphicsProgram.get(), essl1_shaders::PositionAttrib(), 0.5f);
        glFinish();

        // Signal the first thread and wait for it to dispatch a compute shader again.
        threadSynchronization.nextStep(Step::Thread1Drew);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0DispatchedComputeAgain));

        // Wake up and do next step: Draw and force flush the underlying command buffer again.
        drawQuad(graphicsProgram.get(), essl1_shaders::PositionAttrib(), 0.5f);
        glFinish();

        // Signal the first thread and wait exit this thread.
        threadSynchronization.nextStep(Step::Finish);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    deletingThread.join();
    continuingThread.join();

    ASSERT_NE(currentStep, Step::Abort);

    // Clean up
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    for (size_t t = 0; t < kThreadCount; ++t)
    {
        eglDestroySurface(dpy, surface[t]);
        eglDestroyContext(dpy, ctx[t]);
    }
}

// Test that repeated EGL init + terminate with improper cleanup doesn't cause an OOM crash.
// To reproduce the OOM error -
//     1. Increase the loop count to a large number
//     2. Remove the call to "eglReleaseThread" in the for loop
TEST_P(EGLMultiContextTest, RepeatedEglInitAndTerminate)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    // Release all resources in parent thread
    getEGLWindow()->destroyGL();

    EGLDisplay dpy;
    EGLSurface srf;
    EGLContext ctx;
    EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(),
                          EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, GetParam().getDeviceType(),
                          EGL_NONE};

    for (int i = 0; i < 50; i++)  // Note: this test is fairly slow b/303089709
    {
        std::thread thread = std::thread([&]() {
            dpy = eglGetPlatformDisplayEXT(
                EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
            EXPECT_TRUE(dpy != EGL_NO_DISPLAY);
            EXPECT_EGL_TRUE(eglInitialize(dpy, nullptr, nullptr));

            EGLConfig config = EGL_NO_CONFIG_KHR;
            EXPECT_TRUE(chooseConfig(dpy, &config));

            EXPECT_TRUE(createPbufferSurface(dpy, config, 2560, 1080, &srf));
            ASSERT_EGL_SUCCESS() << "eglCreatePbufferSurface failed.";

            EXPECT_TRUE(createContext(dpy, config, &ctx));
            EXPECT_EGL_TRUE(eglMakeCurrent(dpy, srf, srf, ctx));

            // Clear and read back to make sure thread uses context.
            glClearColor(1.0, 0.0, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);
            EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

            eglTerminate(dpy);
            EXPECT_EGL_SUCCESS();
            eglReleaseThread();
            EXPECT_EGL_SUCCESS();
            dpy = EGL_NO_DISPLAY;
            srf = EGL_NO_SURFACE;
            ctx = EGL_NO_CONTEXT;
        });

        thread.join();
    }
}

// Test that thread B can reuse the unterminated display created by thread A
// even after thread A is destroyed.
TEST_P(EGLMultiContextTest, ReuseUnterminatedDisplay)
{
    // Release all resources in parent thread
    getEGLWindow()->destroyGL();

    EGLDisplay dpy;
    EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(),
                          EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, GetParam().getDeviceType(),
                          EGL_NONE};

    std::thread threadA = std::thread([&]() {
        dpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
                                       reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        EXPECT_TRUE(dpy != EGL_NO_DISPLAY);
        EXPECT_EGL_TRUE(eglInitialize(dpy, nullptr, nullptr));
    });
    threadA.join();

    std::thread threadB = std::thread([&]() {
        EGLSurface srf;
        EGLContext ctx;
        EGLConfig config = EGL_NO_CONFIG_KHR;
        // If threadA's termination caused "dpy" to be incorrectly terminated all EGL APIs below
        // staring with eglChooseConfig(...) will error out with an EGL_NOT_INITIALIZED error.
        EXPECT_TRUE(chooseConfig(dpy, &config));

        EXPECT_TRUE(createPbufferSurface(dpy, config, 2560, 1080, &srf));
        ASSERT_EGL_SUCCESS() << "eglCreatePbufferSurface failed.";

        EXPECT_TRUE(createContext(dpy, config, &ctx));
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, srf, srf, ctx));

        // Clear and read back to make sure thread uses context.
        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

        srf = EGL_NO_SURFACE;
        ctx = EGL_NO_CONTEXT;
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, srf, srf, ctx));
        eglTerminate(dpy);
        EXPECT_EGL_SUCCESS();
        EXPECT_EGL_SUCCESS();
        dpy = EGL_NO_DISPLAY;
    });
    threadB.join();
}

// Test that thread B can wait on thread A's sync before thread A flushes it, and wakes up after
// that.  Note that only validatity of the fence operations are tested here.  The test could
// potentially be enhanced with EGL images similarly to how
// MultithreadingTestES3::testFenceWithOpenRenderPass tests correctness of synchronization through
// a shared texture.
void EGLMultiContextTest::testFenceWithOpenRenderPass(FenceTest test, FlushMethod flushMethod)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    constexpr uint32_t kWidth  = 100;
    constexpr uint32_t kHeight = 200;

    EGLSyncKHR sync = EGL_NO_SYNC_KHR;

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0CreateFence,
        Thread1WaitFence,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Start));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Issue a draw
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Issue a fence.  A render pass is currently open, but it should be closed in the Vulkan
        // backend.
        sync = eglCreateSyncKHR(dpy, EGL_SYNC_FENCE_KHR, nullptr);
        EXPECT_NE(sync, EGL_NO_SYNC_KHR);

        // Wait for thread 1 to wait on it.
        threadSynchronization.nextStep(Step::Thread0CreateFence);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1WaitFence));

        // Wait a little to give thread 1 time to wait on the sync object before flushing it.
        angle::Sleep(500);
        switch (flushMethod)
        {
            case FlushMethod::Flush:
                glFlush();
                break;
            case FlushMethod::Finish:
                glFinish();
                break;
        }

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::red);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for thread 0 to create the fence object.
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0CreateFence));

        // Test access to the fence object
        threadSynchronization.nextStep(Step::Thread1WaitFence);

        constexpr GLuint64 kTimeout = 2'000'000'000;  // 2 seconds
        EGLint result               = EGL_CONDITION_SATISFIED_KHR;
        switch (test)
        {
            case FenceTest::ClientWait:
                result = eglClientWaitSyncKHR(dpy, sync, 0, kTimeout);
                break;
            case FenceTest::ServerWait:
                ASSERT_TRUE(eglWaitSyncKHR(dpy, sync, 0));
                break;
            case FenceTest::GetStatus:
            {
                EGLint value;
                EXPECT_EGL_TRUE(eglGetSyncAttribKHR(dpy, sync, EGL_SYNC_STATUS_KHR, &value));
                if (value != EGL_SIGNALED_KHR)
                {
                    result = eglClientWaitSyncKHR(dpy, sync, 0, kTimeout);
                }
                break;
            }
        }
        ASSERT_TRUE(result == EGL_CONDITION_SATISFIED_KHR);

        // Issue a draw
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::green);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        threadSynchronization.nextStep(Step::Finish);
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Test that thread B can wait on thread A's sync before thread A flushes it, and wakes up after
// that.
TEST_P(EGLMultiContextTest, ThreadBClientWaitBeforeThreadASyncFlush)
{
    testFenceWithOpenRenderPass(FenceTest::ClientWait, FlushMethod::Flush);
}

// Test that thread B can wait on thread A's sync before thread A flushes it, and wakes up after
// that.
TEST_P(EGLMultiContextTest, ThreadBServerWaitBeforeThreadASyncFlush)
{
    testFenceWithOpenRenderPass(FenceTest::ServerWait, FlushMethod::Flush);
}

// Test that thread B can wait on thread A's sync before thread A flushes it, and wakes up after
// that.
TEST_P(EGLMultiContextTest, ThreadBGetStatusBeforeThreadASyncFlush)
{
    testFenceWithOpenRenderPass(FenceTest::GetStatus, FlushMethod::Flush);
}

// Test that thread B can wait on thread A's sync before thread A flushes it, and wakes up after
// that.
TEST_P(EGLMultiContextTest, ThreadBClientWaitBeforeThreadASyncFinish)
{
    testFenceWithOpenRenderPass(FenceTest::ClientWait, FlushMethod::Finish);
}

// Test that thread B can wait on thread A's sync before thread A flushes it, and wakes up after
// that.
TEST_P(EGLMultiContextTest, ThreadBServerWaitBeforeThreadASyncFinish)
{
    testFenceWithOpenRenderPass(FenceTest::ServerWait, FlushMethod::Finish);
}

// Test that thread B can wait on thread A's sync before thread A flushes it, and wakes up after
// that.
TEST_P(EGLMultiContextTest, ThreadBGetStatusBeforeThreadASyncFinish)
{
    testFenceWithOpenRenderPass(FenceTest::GetStatus, FlushMethod::Finish);
}

// Test that thread B can submit while three other threads are waiting for GPU to finish.
TEST_P(EGLMultiContextTest, ThreadBCanSubmitWhileThreadAWaiting)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    constexpr uint32_t kWidth  = 100;
    constexpr uint32_t kHeight = 200;

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        ThreadAMapBufferBufferRange,
        ThreadBSubmit,
        Finish,
        Abort,
    };

    Step currentStep = Step::Start;
    std::atomic<size_t> threadCountMakingMapBufferCall(0);
    std::atomic<size_t> threadCountFinishedMapBufferCall(0);
    constexpr size_t kMaxThreadCountMakingMapBufferCall = 3;
    auto threadA = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Start));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));
        // Issue a draw
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
        // Issue a compute shader write to a buffer
        constexpr char kCS[] = R"(#version 310 es
                                layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
                                layout(std140, binding = 0) buffer block {
                                    uvec4 data;
                                } outBlock;
                                void main()
                                {
                                    outBlock.data += uvec4(1);
                                })";
        ANGLE_GL_COMPUTE_PROGRAM(computeProgram, kCS);
        glUseProgram(computeProgram);
        constexpr std::array<uint32_t, 4> kInitData = {};
        GLBuffer coherentBuffer;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, coherentBuffer);
        glBufferStorageEXT(GL_SHADER_STORAGE_BUFFER, sizeof(kInitData), kInitData.data(),
                           GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT |
                               GL_MAP_COHERENT_BIT_EXT);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, coherentBuffer);
        glDispatchCompute(100, 100, 100);
        EXPECT_GL_NO_ERROR();

        // Map the buffers for read. This should trigger driver wait for GPU to finish
        glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, coherentBuffer);
        if (++threadCountMakingMapBufferCall == kMaxThreadCountMakingMapBufferCall)
        {
            threadSynchronization.nextStep(Step::ThreadAMapBufferBufferRange);
        }
        // Wait for thread B to start submit commands.
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::ThreadBSubmit));
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(kInitData),
                         GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
        ASSERT_GL_NO_ERROR();
        ++threadCountFinishedMapBufferCall;

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::red);
        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    auto threadB = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Prepare for draw and then wait until threadA is ready to wait
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        glScissor(0, 0, 1, 1);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::ThreadAMapBufferBufferRange));

        // Test submit in a loop
        threadSynchronization.nextStep(Step::ThreadBSubmit);
        for (int loop = 0;
             loop < 16 && threadCountFinishedMapBufferCall < kMaxThreadCountMakingMapBufferCall;
             loop++)
        {
            drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
            glFinish();
            EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::green);
        }

        // Clean up
        ASSERT_GL_NO_ERROR();
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        threadSynchronization.nextStep(Step::Finish);
    };

    std::array<LockStepThreadFunc, kMaxThreadCountMakingMapBufferCall + 1> threadFuncs;
    for (size_t i = 0; i < kMaxThreadCountMakingMapBufferCall; i++)
    {
        threadFuncs[i] = std::move(threadA);
    }
    threadFuncs[kMaxThreadCountMakingMapBufferCall] = std::move(threadB);

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());
    ASSERT_NE(currentStep, Step::Abort);
}

// Test that if there are any placeholder objects when the programs don't use any resources
// (such as textures), they can correctly be used in non-shared contexts (without causing
// double-free).
TEST_P(EGLMultiContextTest, NonSharedContextsReuseDescritorSetLayoutHandle)
{
    EGLWindow *window   = getEGLWindow();
    EGLDisplay dpy      = window->getDisplay();
    EGLSurface surface  = window->getSurface();
    EGLContext context1 = window->createContext(EGL_NO_CONTEXT, nullptr);
    EGLContext context2 = window->createContext(EGL_NO_CONTEXT, nullptr);

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context1));
    EXPECT_EGL_SUCCESS();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context2));
    EXPECT_EGL_SUCCESS();

    ANGLE_GL_PROGRAM(program1, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(program1, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Cleanup
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_TRUE(eglDestroyContext(dpy, context1));
    EXPECT_EGL_TRUE(eglDestroyContext(dpy, context2));
    EXPECT_EGL_SUCCESS();
}

}  // anonymous namespace

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLMultiContextTest);
ANGLE_INSTANTIATE_TEST_ES31(EGLMultiContextTest);
