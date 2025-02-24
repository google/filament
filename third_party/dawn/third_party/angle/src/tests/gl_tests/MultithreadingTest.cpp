//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MulithreadingTest.cpp : Tests of multithreaded rendering

#include "test_utils/ANGLETest.h"
#include "test_utils/MultiThreadSteps.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/test_utils.h"

#include <atomic>
#include <mutex>
#include <thread>

namespace angle
{

class MultithreadingTest : public ANGLETest<>
{
  public:
    static constexpr uint32_t kSize = 512;

  protected:
    MultithreadingTest()
    {
        setWindowWidth(kSize);
        setWindowHeight(kSize);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

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

    EGLContext createMultithreadedContext(EGLWindow *window, EGLContext shareCtx)
    {
        EGLint attribs[] = {EGL_CONTEXT_VIRTUALIZATION_GROUP_ANGLE, mVirtualizationGroup++,
                            EGL_NONE};
        if (!IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(),
                                          "EGL_ANGLE_context_virtualization"))
        {
            attribs[0] = EGL_NONE;
        }

        return window->createContext(shareCtx, attribs);
    }

    void runMultithreadedGLTest(
        std::function<void(EGLSurface surface, size_t threadIndex)> testBody,
        size_t threadCount)
    {
        std::mutex mutex;

        EGLWindow *window = getEGLWindow();
        EGLDisplay dpy    = window->getDisplay();
        EGLConfig config  = window->getConfig();

        constexpr EGLint kPBufferSize = 256;

        std::vector<std::thread> threads(threadCount);
        for (size_t threadIdx = 0; threadIdx < threadCount; threadIdx++)
        {
            threads[threadIdx] = std::thread([&, threadIdx]() {
                EGLSurface surface = EGL_NO_SURFACE;
                EGLContext ctx     = EGL_NO_CONTEXT;

                {
                    std::lock_guard<decltype(mutex)> lock(mutex);

                    // Initialize the pbuffer and context
                    EGLint pbufferAttributes[] = {
                        EGL_WIDTH, kPBufferSize, EGL_HEIGHT, kPBufferSize, EGL_NONE, EGL_NONE,
                    };
                    surface = eglCreatePbufferSurface(dpy, config, pbufferAttributes);
                    EXPECT_EGL_SUCCESS();

                    ctx = createMultithreadedContext(window, EGL_NO_CONTEXT);
                    EXPECT_NE(EGL_NO_CONTEXT, ctx);

                    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
                    EXPECT_EGL_SUCCESS();
                }

                testBody(surface, threadIdx);

                {
                    std::lock_guard<decltype(mutex)> lock(mutex);

                    // Clean up
                    EXPECT_EGL_TRUE(
                        eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
                    EXPECT_EGL_SUCCESS();

                    eglDestroySurface(dpy, surface);
                    eglDestroyContext(dpy, ctx);
                }
            });
        }

        for (std::thread &thread : threads)
        {
            thread.join();
        }
    }

    std::atomic<EGLint> mVirtualizationGroup;
};

class MultithreadingTestES3 : public MultithreadingTest
{
  public:
    void textureThreadFunction(bool useDraw);
    void mainThreadDraw(bool useDraw);

  protected:
    MultithreadingTestES3()
        : mTexture2D(0), mExitThread(false), mMainThreadSyncObj(NULL), mSecondThreadSyncObj(NULL)
    {
        setWindowWidth(kSize);
        setWindowHeight(kSize);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    GLuint create2DTexture()
    {
        GLuint texture2D;
        glGenTextures(1, &texture2D);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        EXPECT_GL_NO_ERROR();
        return texture2D;
    }

    void testSetUp() override { mTexture2D = create2DTexture(); }

    void testTearDown() override
    {
        if (mTexture2D)
        {
            glDeleteTextures(1, &mTexture2D);
        }
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

    enum class DrawOrder
    {
        Before,
        After,
    };
    void testFramebufferFetch(DrawOrder drawOrder);

    std::mutex mMutex;
    GLuint mTexture2D;
    std::atomic<bool> mExitThread;
    std::atomic<bool> mDrawGreen;  // Toggle drawing green or red
    std::atomic<GLsync> mMainThreadSyncObj;
    std::atomic<GLsync> mSecondThreadSyncObj;
};

// Test that it's possible to make one context current on different threads
TEST_P(MultithreadingTest, MakeCurrentSingleContext)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    std::mutex mutex;

    EGLWindow *window  = getEGLWindow();
    EGLDisplay dpy     = window->getDisplay();
    EGLContext ctx     = window->getContext();
    EGLSurface surface = window->getSurface();

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_SUCCESS();

    constexpr size_t kThreadCount = 16;
    std::array<std::thread, kThreadCount> threads;
    for (std::thread &thread : threads)
    {
        thread = std::thread([&]() {
            std::lock_guard<decltype(mutex)> lock(mutex);

            EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
            EXPECT_EGL_SUCCESS();

            EXPECT_EGL_TRUE(eglSwapBuffers(dpy, surface));
            EXPECT_EGL_SUCCESS();

            EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
            EXPECT_EGL_SUCCESS();
        });
    }

    for (std::thread &thread : threads)
    {
        thread.join();
    }

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
    EXPECT_EGL_SUCCESS();
}

// Test that multiple threads can clear and readback pixels successfully at the same time
TEST_P(MultithreadingTest, MultiContextClear)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    auto testBody = [](EGLSurface surface, size_t thread) {
        constexpr size_t kIterationsPerThread = 32;
        for (size_t iteration = 0; iteration < kIterationsPerThread; iteration++)
        {
            // Base the clear color on the thread and iteration indexes so every clear color is
            // unique
            const GLColor color(static_cast<GLubyte>(thread % 255),
                                static_cast<GLubyte>(iteration % 255), 0, 255);
            const angle::Vector4 floatColor = color.toNormalizedVector();

            glClearColor(floatColor[0], floatColor[1], floatColor[2], floatColor[3]);
            EXPECT_GL_NO_ERROR();

            glClear(GL_COLOR_BUFFER_BIT);
            EXPECT_GL_NO_ERROR();

            EXPECT_PIXEL_COLOR_EQ(0, 0, color);
        }
    };
    runMultithreadedGLTest(testBody, 72);
}

// Verify that threads can interleave eglDestroyContext and draw calls without
// any crashes.
TEST_P(MultithreadingTest, MultiContextDeleteDraw)
{
    // Skip this test on non-D3D11 backends, as it has the potential to time-out
    // and this test was originally intended to catch a crash on the D3D11 backend.
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!IsD3D11());

    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();
    EGLConfig config  = window->getConfig();

    std::thread t1 = std::thread([&]() {
        // 5000 is chosen here as it reliably reproduces the former crash.
        for (int i = 0; i < 5000; i++)
        {
            EGLContext ctx1 = createMultithreadedContext(window, EGL_NO_CONTEXT);
            EGLContext ctx2 = createMultithreadedContext(window, EGL_NO_CONTEXT);

            EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx2));
            EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx1));

            EXPECT_EGL_TRUE(eglDestroyContext(dpy, ctx2));
            EXPECT_EGL_TRUE(eglDestroyContext(dpy, ctx1));
        }
    });

    std::thread t2 = std::thread([&]() {
        EGLint pbufferAttributes[] = {
            EGL_WIDTH, 256, EGL_HEIGHT, 256, EGL_NONE, EGL_NONE,
        };

        EGLSurface surface = eglCreatePbufferSurface(dpy, config, pbufferAttributes);
        EXPECT_EGL_SUCCESS();

        auto ctx = createMultithreadedContext(window, EGL_NO_CONTEXT);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));

        constexpr size_t kIterationsPerThread = 512;
        constexpr size_t kDrawsPerIteration   = 512;

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);

        GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

        auto quadVertices = GetQuadVertices();

        GLBuffer vertexBuffer;
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, quadVertices.data(), GL_STATIC_DRAW);

        GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
        glEnableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
        for (size_t iteration = 0; iteration < kIterationsPerThread; iteration++)
        {
            const GLColor color(static_cast<GLubyte>(15151 % 255),
                                static_cast<GLubyte>(iteration % 255), 0, 255);
            const angle::Vector4 floatColor = color.toNormalizedVector();
            glUniform4fv(colorLocation, 1, floatColor.data());
            for (size_t draw = 0; draw < kDrawsPerIteration; draw++)
            {
                EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
    });

    t1.join();
    t2.join();
}

// Test that multiple threads can draw and readback pixels successfully at the same time
TEST_P(MultithreadingTest, MultiContextDraw)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    ANGLE_SKIP_TEST_IF(isSwiftshader());

    auto testBody = [](EGLSurface surface, size_t thread) {
        constexpr size_t kIterationsPerThread = 32;
        constexpr size_t kDrawsPerIteration   = 500;

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);

        GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

        auto quadVertices = GetQuadVertices();

        GLBuffer vertexBuffer;
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, quadVertices.data(), GL_STATIC_DRAW);

        GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
        glEnableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

        for (size_t iteration = 0; iteration < kIterationsPerThread; iteration++)
        {
            // Base the clear color on the thread and iteration indexes so every clear color is
            // unique
            const GLColor color(static_cast<GLubyte>(thread % 255),
                                static_cast<GLubyte>(iteration % 255), 0, 255);
            const angle::Vector4 floatColor = color.toNormalizedVector();
            glUniform4fv(colorLocation, 1, floatColor.data());

            for (size_t draw = 0; draw < kDrawsPerIteration; draw++)
            {
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            EXPECT_PIXEL_COLOR_EQ(0, 0, color);
        }
    };
    runMultithreadedGLTest(testBody, 4);
}

// Test that multiple threads can draw and read back pixels correctly.
// Using eglSwapBuffers stresses race conditions around use of QueueSerials.
TEST_P(MultithreadingTest, MultiContextDrawWithSwapBuffers)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    // http://anglebug.com/42263666
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();

    auto testBody = [dpy](EGLSurface surface, size_t thread) {
        constexpr size_t kIterationsPerThread = 100;
        constexpr size_t kDrawsPerIteration   = 10;

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);

        GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

        auto quadVertices = GetQuadVertices();

        GLBuffer vertexBuffer;
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, quadVertices.data(), GL_STATIC_DRAW);

        GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
        glEnableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

        for (size_t iteration = 0; iteration < kIterationsPerThread; iteration++)
        {
            // Base the clear color on the thread and iteration indexes so every clear color is
            // unique
            const GLColor color(static_cast<GLubyte>(thread % 255),
                                static_cast<GLubyte>(iteration % 255), 0, 255);
            const angle::Vector4 floatColor = color.toNormalizedVector();
            glUniform4fv(colorLocation, 1, floatColor.data());

            for (size_t draw = 0; draw < kDrawsPerIteration; draw++)
            {
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            EXPECT_EGL_TRUE(eglSwapBuffers(dpy, surface));
            EXPECT_EGL_SUCCESS();

            EXPECT_PIXEL_COLOR_EQ(0, 0, color);
        }
    };
    runMultithreadedGLTest(testBody, 32);
}

// Test that ANGLE handles multiple threads creating and destroying resources (vertex buffer in this
// case).
TEST_P(MultithreadingTest, MultiContextCreateAndDeleteResources)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();

    auto testBody = [dpy](EGLSurface surface, size_t thread) {
        constexpr size_t kIterationsPerThread = 32;
        constexpr size_t kDrawsPerIteration   = 1;

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);

        GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

        auto quadVertices = GetQuadVertices();

        for (size_t iteration = 0; iteration < kIterationsPerThread; iteration++)
        {
            GLBuffer vertexBuffer;
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, quadVertices.data(),
                         GL_STATIC_DRAW);

            GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
            glEnableVertexAttribArray(positionLocation);
            glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

            // Base the clear color on the thread and iteration indexes so every clear color is
            // unique
            const GLColor color(static_cast<GLubyte>(thread % 255),
                                static_cast<GLubyte>(iteration % 255), 0, 255);
            const angle::Vector4 floatColor = color.toNormalizedVector();
            glUniform4fv(colorLocation, 1, floatColor.data());

            for (size_t draw = 0; draw < kDrawsPerIteration; draw++)
            {
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            EXPECT_EGL_TRUE(eglSwapBuffers(dpy, surface));
            EXPECT_EGL_SUCCESS();

            EXPECT_PIXEL_COLOR_EQ(0, 0, color);
        }
        glFinish();
    };
    runMultithreadedGLTest(testBody, 32);
}

TEST_P(MultithreadingTest, MultiCreateContext)
{
    // Supported by CGL, GLX, and WGL (https://anglebug.com/42263324)
    // Not supported on Ozone (https://crbug.com/1103009)
    ANGLE_SKIP_TEST_IF(!(IsWindows() || IsLinux() || IsMac()) || IsOzone());

    EGLWindow *window  = getEGLWindow();
    EGLDisplay dpy     = window->getDisplay();
    EGLContext ctx     = window->getContext();
    EGLSurface surface = window->getSurface();

    // Un-makeCurrent the test window's context
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_SUCCESS();

    constexpr size_t kThreadCount = 16;
    std::atomic<uint32_t> barrier(0);
    std::vector<std::thread> threads(kThreadCount);
    std::vector<EGLContext> contexts(kThreadCount);
    for (size_t threadIdx = 0; threadIdx < kThreadCount; threadIdx++)
    {
        threads[threadIdx] = std::thread([&, threadIdx]() {
            contexts[threadIdx] = EGL_NO_CONTEXT;
            {
                contexts[threadIdx] = createMultithreadedContext(window, EGL_NO_CONTEXT);
                EXPECT_NE(EGL_NO_CONTEXT, contexts[threadIdx]);

                barrier++;
            }

            while (barrier < kThreadCount)
            {
            }

            {
                EXPECT_TRUE(eglDestroyContext(dpy, contexts[threadIdx]));
            }
        });
    }

    for (std::thread &thread : threads)
    {
        thread.join();
    }

    // Re-make current the test window's context for teardown.
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
    EXPECT_EGL_SUCCESS();
}

// Create multiple shared context and draw with shared vertex buffer simutanously
TEST_P(MultithreadingTest, CreateMultiSharedContextAndDraw)
{
    // Supported by CGL, GLX, and WGL (https://anglebug.com/42263324)
    // Not supported on Ozone (https://crbug.com/1103009)
    ANGLE_SKIP_TEST_IF(!(IsWindows() || IsLinux() || IsMac()) || IsOzone());
    EGLWindow *window             = getEGLWindow();
    EGLDisplay dpy                = window->getDisplay();
    EGLConfig config              = window->getConfig();
    constexpr EGLint kPBufferSize = 256;

    // Initialize the pbuffer and context
    EGLint pbufferAttributes[] = {
        EGL_WIDTH, kPBufferSize, EGL_HEIGHT, kPBufferSize, EGL_NONE, EGL_NONE,
    };
    EGLSurface sharedSurface = eglCreatePbufferSurface(dpy, config, pbufferAttributes);
    EXPECT_EGL_SUCCESS();
    EGLContext sharedCtx = createMultithreadedContext(window, EGL_NO_CONTEXT);
    EXPECT_NE(EGL_NO_CONTEXT, sharedCtx);
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, sharedSurface, sharedSurface, sharedCtx));
    EXPECT_EGL_SUCCESS();

    // Create a shared vertextBuffer
    auto quadVertices = GetQuadVertices();
    GLBuffer sharedVertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, sharedVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, quadVertices.data(), GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    // Now draw with the buffer and verify
    {
        ANGLE_GL_PROGRAM(sharedProgram, essl1_shaders::vs::Simple(),
                         essl1_shaders::fs::UniformColor());
        glUseProgram(sharedProgram);
        GLint colorLocation = glGetUniformLocation(sharedProgram, essl1_shaders::ColorUniform());
        GLint positionLocation =
            glGetAttribLocation(sharedProgram, essl1_shaders::PositionAttrib());
        glEnableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
        const GLColor color(0, 0, 0, 255);
        const angle::Vector4 floatColor = color.toNormalizedVector();
        glUniform4fv(colorLocation, 1, floatColor.data());
        glDrawArrays(GL_TRIANGLES, 0, 6);
        EXPECT_PIXEL_COLOR_EQ(0, 0, color);
    }

    // Create shared context in their own threads and draw with the shared vertex buffer at the same
    // time.
    size_t threadCount                    = 16;
    constexpr size_t kIterationsPerThread = 3;
    constexpr size_t kDrawsPerIteration   = 50;
    std::vector<std::thread> threads(threadCount);
    std::atomic<uint32_t> numOfContextsCreated(0);
    std::mutex mutex;
    for (size_t threadIdx = 0; threadIdx < threadCount; threadIdx++)
    {
        threads[threadIdx] = std::thread([&, threadIdx]() {
            EGLSurface surface = EGL_NO_SURFACE;
            EGLContext ctx     = EGL_NO_CONTEXT;

            {
                std::lock_guard<decltype(mutex)> lock(mutex);
                // Initialize the pbuffer and context
                surface = eglCreatePbufferSurface(dpy, config, pbufferAttributes);
                EXPECT_EGL_SUCCESS();
                ctx = createMultithreadedContext(window, /*EGL_NO_CONTEXT*/ sharedCtx);
                EXPECT_NE(EGL_NO_CONTEXT, ctx);
                EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
                EXPECT_EGL_SUCCESS();
                numOfContextsCreated++;
            }

            // Wait for all contexts created.
            while (numOfContextsCreated < threadCount)
            {
            }

            // Now draw with shared vertex buffer
            {
                ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(),
                                 essl1_shaders::fs::UniformColor());
                glUseProgram(program);

                GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());
                GLint positionLocation =
                    glGetAttribLocation(program, essl1_shaders::PositionAttrib());

                // Use sharedVertexBuffer
                glBindBuffer(GL_ARRAY_BUFFER, sharedVertexBuffer);
                glEnableVertexAttribArray(positionLocation);
                glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

                for (size_t iteration = 0; iteration < kIterationsPerThread; iteration++)
                {
                    // Base the clear color on the thread and iteration indexes so every clear color
                    // is unique
                    const GLColor color(static_cast<GLubyte>(threadIdx % 255),
                                        static_cast<GLubyte>(iteration % 255), 0, 255);
                    const angle::Vector4 floatColor = color.toNormalizedVector();
                    glUniform4fv(colorLocation, 1, floatColor.data());

                    for (size_t draw = 0; draw < kDrawsPerIteration; draw++)
                    {
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                    }

                    EXPECT_PIXEL_COLOR_EQ(0, 0, color);
                }
            }

            // tear down shared context
            {
                std::lock_guard<decltype(mutex)> lock(mutex);
                EXPECT_EGL_TRUE(
                    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
                EXPECT_EGL_SUCCESS();
                eglDestroySurface(dpy, surface);
                eglDestroyContext(dpy, ctx);
            }
        });
    }

    for (std::thread &thread : threads)
    {
        thread.join();
    }

    eglDestroySurface(dpy, sharedSurface);
    eglDestroyContext(dpy, sharedCtx);

    // Re-make current the test window's context for teardown.
    EXPECT_EGL_TRUE(
        eglMakeCurrent(dpy, window->getSurface(), window->getSurface(), window->getContext()));
    EXPECT_EGL_SUCCESS();
}

// Producer/Consumer test using EGLImages and EGLSyncs
TEST_P(MultithreadingTest, EGLImageProduceConsume)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay dpy     = window->getDisplay();
    EGLContext rootCtx = window->getContext();

    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(dpy, "EGL_KHR_image"));
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(dpy, "EGL_KHR_gl_texture_2D_image"));
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(dpy, "EGL_KHR_fence_sync"));

    struct sharedImage
    {
        EGLImage image;
        EGLSync sync;
        GLuint rootTexture;
    };

    std::mutex mutex;
    std::vector<sharedImage> waitingForProduce;
    std::vector<sharedImage> waitingForConsume;

    constexpr size_t kNumImages = 10;
    for (size_t i = 0; i < kNumImages; i++)
    {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        sharedImage img;
        img.image       = eglCreateImageKHR(dpy, rootCtx, EGL_GL_TEXTURE_2D_KHR,
                                            reinterpret_cast<EGLClientBuffer>(texture), nullptr);
        img.sync        = eglCreateSyncKHR(dpy, EGL_SYNC_FENCE_KHR, nullptr);
        img.rootTexture = texture;

        waitingForProduce.push_back(std::move(img));
    }

    constexpr size_t kIterations = 10000;

    std::thread producerThread([&]() {
        EGLContext ctx = createMultithreadedContext(window, EGL_NO_CONTEXT);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx));

        {
            ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(),
                             essl1_shaders::fs::UniformColor());
            glUseProgram(drawColor);
            GLint colorUniformLocation =
                glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
            ASSERT_NE(colorUniformLocation, -1);

            size_t iteration = 0;
            while (iteration < kIterations)
            {
                sharedImage img;
                {
                    std::lock_guard<decltype(mutex)> lock(mutex);
                    if (waitingForProduce.empty())
                    {
                        continue;
                    }
                    img = std::move(waitingForProduce.back());
                    waitingForProduce.pop_back();
                }

                eglWaitSync(dpy, img.sync, 0);
                EXPECT_EGL_SUCCESS();

                eglDestroySync(dpy, img.sync);

                GLTexture texture;
                glBindTexture(GL_TEXTURE_2D, texture);
                glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, img.image);
                EXPECT_GL_NO_ERROR();

                GLFramebuffer fbo;
                glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture,
                                       0);

                glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);

                glUniform4f(colorUniformLocation, float(iteration) / kIterations, 0.0f, 0.0f, 1.0f);
                drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0);

                glBindTexture(GL_TEXTURE_2D, 0);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                img.sync = eglCreateSyncKHR(dpy, EGL_SYNC_FENCE_KHR, nullptr);
                EXPECT_EGL_SUCCESS();
                glFlush();

                {
                    std::lock_guard<decltype(mutex)> lock(mutex);
                    waitingForConsume.insert(waitingForConsume.begin(), std::move(img));
                }

                iteration++;
            }
        }

        eglDestroyContext(dpy, ctx);
    });

    std::thread consumerThread([&]() {
        EGLContext ctx = createMultithreadedContext(window, EGL_NO_CONTEXT);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx));

        {
            ANGLE_GL_PROGRAM(drawTexture, essl1_shaders::vs::Texture2D(),
                             essl1_shaders::fs::Texture2D());
            glUseProgram(drawTexture);
            GLint textureUniformLocation =
                glGetUniformLocation(drawTexture, angle::essl1_shaders::Texture2DUniform());
            ASSERT_NE(textureUniformLocation, -1);
            glUniform1i(textureUniformLocation, 0);
            glActiveTexture(GL_TEXTURE0);

            GLTexture backbufferTexture;
            glBindTexture(GL_TEXTURE_2D, backbufferTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         nullptr);

            GLFramebuffer fbo;
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   backbufferTexture, 0);

            size_t iteration = 0;
            while (iteration < kIterations)
            {
                sharedImage img;
                {
                    std::lock_guard<decltype(mutex)> lock(mutex);
                    if (waitingForConsume.empty())
                    {
                        continue;
                    }
                    img = std::move(waitingForConsume.back());
                    waitingForConsume.pop_back();
                }

                eglWaitSync(dpy, img.sync, 0);
                EXPECT_EGL_SUCCESS();
                eglDestroySync(dpy, img.sync);

                GLTexture texture;
                glBindTexture(GL_TEXTURE_2D, texture);
                glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, img.image);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                drawQuad(drawTexture, essl1_shaders::PositionAttrib(), 0);
                EXPECT_GL_NO_ERROR();

                glBindTexture(GL_TEXTURE_2D, 0);

                img.sync = eglCreateSyncKHR(dpy, EGL_SYNC_FENCE_KHR, nullptr);
                EXPECT_EGL_SUCCESS();
                glFlush();

                {
                    std::lock_guard<decltype(mutex)> lock(mutex);
                    waitingForProduce.insert(waitingForProduce.begin(), std::move(img));
                }

                iteration++;
            }
        }
        eglDestroyContext(dpy, ctx);
    });

    producerThread.join();
    consumerThread.join();

    // Clean up
    {
        for (auto &img : waitingForProduce)
        {
            eglDestroyImageKHR(dpy, img.image);
            eglDestroySync(dpy, img.sync);
            glDeleteTextures(1, &img.rootTexture);
        }
        for (auto &img : waitingForConsume)
        {
            eglDestroyImageKHR(dpy, img.image);
            eglDestroySync(dpy, img.sync);
            glDeleteTextures(1, &img.rootTexture);
        }
    }
}

void MultithreadingTestES3::textureThreadFunction(bool useDraw)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay dpy     = window->getDisplay();
    EGLConfig config   = window->getConfig();
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext ctx     = EGL_NO_CONTEXT;

    // Initialize the pbuffer and context
    EGLint pbufferAttributes[] = {
        EGL_WIDTH, kSize, EGL_HEIGHT, kSize, EGL_NONE, EGL_NONE,
    };
    surface = eglCreatePbufferSurface(dpy, config, pbufferAttributes);
    EXPECT_EGL_SUCCESS();
    EXPECT_NE(EGL_NO_SURFACE, surface);

    ctx = createMultithreadedContext(window, window->getContext());
    EXPECT_NE(EGL_NO_CONTEXT, ctx);

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
    EXPECT_EGL_SUCCESS();

    std::vector<GLColor> greenColor(kSize * kSize, GLColor::green);
    std::vector<GLColor> redColor(kSize * kSize, GLColor::red);
    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    glBindTexture(GL_TEXTURE_2D, mTexture2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture2D, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    mSecondThreadSyncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    ASSERT_GL_NO_ERROR();

    // Draw something
    while (!mExitThread)
    {
        if (mMainThreadSyncObj == nullptr)
        {
            angle::Sleep(0);
        }

        std::lock_guard<decltype(mMutex)> lock(mMutex);

        if (mMainThreadSyncObj != nullptr)
        {
            glWaitSync(mMainThreadSyncObj, 0, GL_TIMEOUT_IGNORED);
            ASSERT_GL_NO_ERROR();
            glDeleteSync(mMainThreadSyncObj);
            ASSERT_GL_NO_ERROR();
            mMainThreadSyncObj = nullptr;
        }
        else
        {
            continue;
        }

        mDrawGreen = !mDrawGreen;

        glBindTexture(GL_TEXTURE_2D, mTexture2D);
        ASSERT_GL_NO_ERROR();

        if (mDrawGreen)
        {
            if (useDraw)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.0f);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                             greenColor.data());
            }
            ASSERT_GL_NO_ERROR();
        }
        else
        {
            if (useDraw)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.0f);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                             redColor.data());
            }
            ASSERT_GL_NO_ERROR();
        }

        ASSERT_EQ(mSecondThreadSyncObj.load(), nullptr);
        mSecondThreadSyncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        ASSERT_GL_NO_ERROR();
    }

    // Clean up
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_SUCCESS();

    eglDestroySurface(dpy, surface);
    eglDestroyContext(dpy, ctx);
}

// Test fence sync with multiple threads drawing
void MultithreadingTestES3::mainThreadDraw(bool useDraw)
{
    EGLWindow *window  = getEGLWindow();
    EGLDisplay dpy     = window->getDisplay();
    EGLContext ctx     = window->getContext();
    EGLSurface surface = window->getSurface();
    // Use odd numbers so we bounce between red and green in the final image
    constexpr int kNumIterations = 5;
    constexpr int kNumDraws      = 5;

    mDrawGreen = false;

    std::thread textureThread(&MultithreadingTestES3::textureThreadFunction, this, true);

    ANGLE_GL_PROGRAM(texProgram, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

    for (int iterations = 0; iterations < kNumIterations; ++iterations)
    {
        GLColor expectedDrawColor;

        for (int draws = 0; draws < kNumDraws;)
        {
            if (mSecondThreadSyncObj == nullptr)
            {
                angle::Sleep(0);
            }

            std::lock_guard<decltype(mMutex)> lock(mMutex);

            if (mSecondThreadSyncObj != nullptr)
            {
                glWaitSync(mSecondThreadSyncObj, 0, GL_TIMEOUT_IGNORED);
                ASSERT_GL_NO_ERROR();
                glDeleteSync(mSecondThreadSyncObj);
                ASSERT_GL_NO_ERROR();
                mSecondThreadSyncObj = nullptr;
            }
            else
            {
                continue;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, mTexture2D);
            glUseProgram(texProgram);
            drawQuad(texProgram, essl1_shaders::PositionAttrib(), 0.0f);

            // mDrawGreen will be changed by the background thread past mMainThreadSyncObj
            // as it will start drawing the next color to fbo. This shouldn't affect
            // pixels of the current frame so save the expected color before unblocking the thread
            expectedDrawColor = mDrawGreen ? GLColor::green : GLColor::red;

            ASSERT_EQ(mMainThreadSyncObj.load(), nullptr);
            mMainThreadSyncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            ASSERT_GL_NO_ERROR();

            ++draws;
        }

        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, expectedDrawColor);

        swapBuffers();
    }

    mExitThread = true;
    textureThread.join();

    // Re-make current the test window's context for teardown.
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, ctx));
    EXPECT_EGL_SUCCESS();
}

// Test that glFenceSync/glWaitSync works correctly with multithreading.
// Main thread: Samples from the shared texture to draw to the default FBO.
// Secondary (Texture) thread: Draws to the shared texture, which the Main thread samples from.
// The overall execution flow is:
// Main Thread:
// 1. Wait for the mSecondThreadSyncObj fence object to be created.
//    - This fence object is used by synchronize access to the shared texture by indicating that the
//    Secondary thread's draws to the texture have all completed and it's now safe to sample from
//    it.
// 2. Once the fence is created, add a glWaitSync(mSecondThreadSyncObj) to the command stream and
//    then delete it.
// 3. Draw, sampling from the shared texture.
// 4. Create a new mMainThreadSyncObj.
//    - This fence object is used to synchronize access to the shared texture by indicating that the
//    Main thread's draws are no longer sampling from the texture, so it's now safe for the
//    Secondary thread to draw to it again with a new color.
// Secondary (Texture) Thread:
// 1. Wait for the mMainThreadSyncObj fence object to be created.
// 2. Once the fence is created, add a glWaitSync(mMainThreadSyncObj) to the command stream and then
//    delete it.
// 3. Draw/Fill the texture.
// 4. Create a new mSecondThreadSyncObj.
//
// These threads loop for the specified number of iterations, drawing/sampling the shared texture
// with the necessary glFlush()s and occasional eglSwapBuffers() to mimic a real multithreaded GLES
// application.
TEST_P(MultithreadingTestES3, MultithreadFenceDraw)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    // Have the secondary thread use glDrawArrays()
    mainThreadDraw(true);
}

// Same as MultithreadFenceDraw, but with the secondary thread using glTexImage2D rather than
// glDrawArrays.
TEST_P(MultithreadingTestES3, MultithreadFenceTexImage)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    // Have the secondary thread use glTexImage2D()
    mainThreadDraw(false);
}

// Test that waiting on a sync object that hasn't been flushed and without a current context returns
// TIMEOUT_EXPIRED or CONDITION_SATISFIED, but doesn't generate an error or crash.
TEST_P(MultithreadingTest, NoFlushNoContextReturnsTimeout)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());

    std::mutex mutex;

    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    EGLSyncKHR sync = eglCreateSyncKHR(dpy, EGL_SYNC_FENCE_KHR, nullptr);
    EXPECT_NE(sync, EGL_NO_SYNC_KHR);

    std::thread thread = std::thread([&]() {
        std::lock_guard<decltype(mutex)> lock(mutex);
        // Make sure there is no active context on this thread.
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
        // Don't wait forever to make sure the test terminates
        constexpr GLuint64 kTimeout = 1'000'000'000;  // 1 second
        int result                  = eglClientWaitSyncKHR(dpy, sync, 0, kTimeout);
        // We typically expect to get back TIMEOUT_EXPIRED since the sync object was never flushed.
        // However, the OpenGL ES backend returns CONDITION_SATISFIED, which is also a passing
        // result.
        ASSERT_TRUE(result == EGL_TIMEOUT_EXPIRED_KHR || result == EGL_CONDITION_SATISFIED_KHR);
    });

    thread.join();

    EXPECT_EGL_TRUE(eglDestroySyncKHR(dpy, sync));
}

// Test that waiting on sync object that hasn't been flushed yet, but is later flushed by another
// thread, correctly returns when the fence is signalled without a timeout.
TEST_P(MultithreadingTest, CreateFenceThreadAClientWaitSyncThreadBDelayedFlush)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());

    EGLSyncKHR sync = EGL_NO_SYNC_KHR;

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0Clear,
        Thread1CreateFence,
        Thread0ClientWaitSync,
        Thread1Flush,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Start));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Do work.
        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Wait for thread 1 to clear.
        threadSynchronization.nextStep(Step::Thread0Clear);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1CreateFence));

        // Wait on the sync object, but do *not* flush it, since the other thread will flush.
        constexpr GLuint64 kTimeout = 2'000'000'000;  // 2 seconds
        threadSynchronization.nextStep(Step::Thread0ClientWaitSync);
        ASSERT_EQ(EGL_CONDITION_SATISFIED_KHR, eglClientWaitSyncKHR(dpy, sync, 0, kTimeout));

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        // Wait for thread 0 to clear.
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0Clear));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Do work.
        glClearColor(0.0, 1.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        sync = eglCreateSyncKHR(dpy, EGL_SYNC_FENCE_KHR, nullptr);
        EXPECT_NE(sync, EGL_NO_SYNC_KHR);

        // Wait for the thread 0 to eglClientWaitSyncKHR().
        threadSynchronization.nextStep(Step::Thread1CreateFence);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0ClientWaitSync));

        // Wait a little to give thread 1 time to wait on the sync object before flushing it.
        angle::Sleep(500);
        glFlush();

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
void MultithreadingTestES3::testFenceWithOpenRenderPass(FenceTest test, FlushMethod flushMethod)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());

    constexpr uint32_t kWidth  = 100;
    constexpr uint32_t kHeight = 200;

    GLsync sync    = 0;
    GLuint texture = 0;

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0CreateFence,
        Thread1WaitFence,
        Thread0Finish,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Start));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Create a shared texture to test synchronization
        GLTexture color;
        texture = color;

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        // Draw to shared texture.
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Issue a fence.  A render pass is currently open, so the fence is not actually submitted
        // in the Vulkan backend.
        sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        ASSERT_NE(sync, nullptr);

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

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        threadSynchronization.nextStep(Step::Thread0Finish);
        threadSynchronization.waitForStep(Step::Finish);
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for thread 0 to create the fence object.
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0CreateFence));

        // Test access to the fence object
        threadSynchronization.nextStep(Step::Thread1WaitFence);

        constexpr GLuint64 kTimeout = 2'000'000'000;  // 2 seconds
        GLenum result               = GL_CONDITION_SATISFIED;
        switch (test)
        {
            case FenceTest::ClientWait:
                result = glClientWaitSync(sync, 0, kTimeout);
                break;
            case FenceTest::ServerWait:
                glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
                break;
            case FenceTest::GetStatus:
            {
                GLint value;
                glGetSynciv(sync, GL_SYNC_STATUS, 1, nullptr, &value);
                if (value != GL_SIGNALED)
                {
                    result = glClientWaitSync(sync, 0, kTimeout);
                }
                break;
            }
        }
        ASSERT_TRUE(result == GL_CONDITION_SATISFIED || result == GL_ALREADY_SIGNALED);

        // Verify the shared texture is drawn to.
        glBindTexture(GL_TEXTURE_2D, texture);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::red);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0Finish));
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
TEST_P(MultithreadingTestES3, ThreadBClientWaitBeforeThreadASyncFlush)
{
    testFenceWithOpenRenderPass(FenceTest::ClientWait, FlushMethod::Flush);
}

// Test that thread B can wait on thread A's sync before thread A flushes it, and wakes up after
// that.
TEST_P(MultithreadingTestES3, ThreadBServerWaitBeforeThreadASyncFlush)
{
    testFenceWithOpenRenderPass(FenceTest::ServerWait, FlushMethod::Flush);
}

// Test that thread B can wait on thread A's sync before thread A flushes it, and wakes up after
// that.
TEST_P(MultithreadingTestES3, ThreadBGetStatusBeforeThreadASyncFlush)
{
    testFenceWithOpenRenderPass(FenceTest::GetStatus, FlushMethod::Flush);
}

// Test that thread B can wait on thread A's sync before thread A flushes it, and wakes up after
// that.
TEST_P(MultithreadingTestES3, ThreadBClientWaitBeforeThreadASyncFinish)
{
    testFenceWithOpenRenderPass(FenceTest::ClientWait, FlushMethod::Finish);
}

// Test that thread B can wait on thread A's sync before thread A flushes it, and wakes up after
// that.
TEST_P(MultithreadingTestES3, ThreadBServerWaitBeforeThreadASyncFinish)
{
    testFenceWithOpenRenderPass(FenceTest::ServerWait, FlushMethod::Finish);
}

// Test that thread B can wait on thread A's sync before thread A flushes it, and wakes up after
// that.
TEST_P(MultithreadingTestES3, ThreadBGetStatusBeforeThreadASyncFinish)
{
    testFenceWithOpenRenderPass(FenceTest::GetStatus, FlushMethod::Finish);
}

// Test the following scenario:
//
// - Thread A opens a render pass, and flushes it.  In the Vulkan backend, this may make the flush
//   deferred.
// - Thread B opens a render pass and creates a fence.  In the Vulkan backend, this also defers the
//   flush.
// - Thread C waits on fence
//
// In the Vulkan backend, submission of the fence is implied by thread C's wait, and thread A may
// also be flushed as collateral.  If the fence's serial is updated based on thread A's submission,
// synchronization between B and C would be broken.
TEST_P(MultithreadingTestES3, ThreadCWaitBeforeThreadBSyncFinish)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());

    constexpr uint32_t kWidth  = 100;
    constexpr uint32_t kHeight = 200;

    GLsync sync    = 0;
    GLuint texture = 0;

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0DrawAndFlush,
        Thread1CreateFence,
        Thread2WaitFence,
        Thread2Finished,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Start));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Open a render pass and flush it.
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
        glFlush();
        ASSERT_GL_NO_ERROR();

        threadSynchronization.nextStep(Step::Thread0DrawAndFlush);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for thread 0 to set up
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0DrawAndFlush));

        // Create a shared texture to test synchronization
        GLTexture color;
        texture = color;

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        // Draw to shared texture.
        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Issue a fence.  A render pass is currently open, so the fence is not actually submitted
        // in the Vulkan backend.
        sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        ASSERT_NE(sync, nullptr);

        // Wait for thread 1 to wait on it.
        threadSynchronization.nextStep(Step::Thread1CreateFence);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread2WaitFence));

        // Wait a little to give thread 1 time to wait on the sync object before flushing it.
        angle::Sleep(500);
        glFlush();

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        threadSynchronization.nextStep(Step::Thread2Finished);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
    };

    auto thread2 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for thread 0 to create the fence object.
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1CreateFence));

        // Test access to the fence object
        threadSynchronization.nextStep(Step::Thread2WaitFence);

        constexpr GLuint64 kTimeout = 2'000'000'000;  // 2 seconds
        GLenum result               = glClientWaitSync(sync, 0, kTimeout);
        ASSERT_TRUE(result == GL_CONDITION_SATISFIED || result == GL_ALREADY_SIGNALED);

        // Verify the shared texture is drawn to.
        glBindTexture(GL_TEXTURE_2D, texture);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::red);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread2Finished));
        threadSynchronization.nextStep(Step::Finish);
    };

    std::array<LockStepThreadFunc, 3> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
        std::move(thread2),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Test that having commands recorded but not submitted on one thread using a texture, does not
// interfere with similar commands on another thread using the same texture.  Regression test for a
// bug in the Vulkan backend where the first thread would batch updates to a descriptor set not
// visible to the other thread, while the other thread picks up the (unupdated) descriptor set from
// a shared cache.
TEST_P(MultithreadingTestES3, UnsynchronizedTextureReads)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());

    GLsync sync    = 0;
    GLuint texture = 0;

    constexpr GLubyte kInitialData[4] = {127, 63, 191, 255};

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0CreateTextureAndDraw,
        Thread1DrawAndFlush,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Start));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Create a texture, and record a command that draws into it.
        GLTexture color;
        texture = color;

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kInitialData);

        sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        ASSERT_NE(sync, nullptr);

        ANGLE_GL_PROGRAM(drawTexture, essl1_shaders::vs::Texture2D(),
                         essl1_shaders::fs::Texture2D());
        drawQuad(drawTexture, essl1_shaders::PositionAttrib(), 0.0f);

        // Don't flush yet; this leaves the descriptor set updates to the texture pending in the
        // Vulkan backend.
        threadSynchronization.nextStep(Step::Thread0CreateTextureAndDraw);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1DrawAndFlush));

        // Flush after thread 1
        EXPECT_PIXEL_COLOR_NEAR(
            0, 0, GLColor(kInitialData[0], kInitialData[1], kInitialData[2], kInitialData[3]), 1);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        threadSynchronization.nextStep(Step::Finish);
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for thread 0 to set up
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0CreateTextureAndDraw));

        // Synchronize with the texture upload (but not the concurrent read)
        glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);

        // Draw with the same texture, in the same way as thread 0.  This ensures that the
        // descriptor sets used in the Vulkan backend are identical.
        glBindTexture(GL_TEXTURE_2D, texture);
        ANGLE_GL_PROGRAM(drawTexture, essl1_shaders::vs::Texture2D(),
                         essl1_shaders::fs::Texture2D());
        drawQuad(drawTexture, essl1_shaders::PositionAttrib(), 0.0f);

        // Flush
        EXPECT_PIXEL_COLOR_NEAR(
            0, 0, GLColor(kInitialData[0], kInitialData[1], kInitialData[2], kInitialData[3]), 1);

        threadSynchronization.nextStep(Step::Thread1DrawAndFlush);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Similar to UnsynchronizedTextureReads, but the texture update is done through framebuffer write.
TEST_P(MultithreadingTestES3, UnsynchronizedTextureReads2)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension() || !hasGLSyncExtension());

    GLsync sync    = 0;
    GLuint texture = 0;

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0CreateTextureAndDraw,
        Thread1DrawAndFlush,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Start));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Create a texture, and record a command that draws into it.
        GLTexture color;
        texture = color;

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        ASSERT_NE(sync, nullptr);

        ANGLE_GL_PROGRAM(drawTexture, essl1_shaders::vs::Texture2D(),
                         essl1_shaders::fs::Texture2D());
        drawQuad(drawTexture, essl1_shaders::PositionAttrib(), 0.0f);

        // Don't flush yet; this leaves the descriptor set updates to the texture pending in the
        // Vulkan backend.
        threadSynchronization.nextStep(Step::Thread0CreateTextureAndDraw);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1DrawAndFlush));

        // Flush after thread 1
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        threadSynchronization.nextStep(Step::Finish);
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for thread 0 to set up
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0CreateTextureAndDraw));

        // Synchronize with the texture update (but not the concurrent read)
        glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);

        // Draw with the same texture, in the same way as thread 0.  This ensures that the
        // descriptor sets used in the Vulkan backend are identical.
        glBindTexture(GL_TEXTURE_2D, texture);
        ANGLE_GL_PROGRAM(drawTexture, essl1_shaders::vs::Texture2D(),
                         essl1_shaders::fs::Texture2D());
        drawQuad(drawTexture, essl1_shaders::PositionAttrib(), 0.0f);

        // Flush
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

        threadSynchronization.nextStep(Step::Thread1DrawAndFlush);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Similar to UnsynchronizedTextureReads, but the texture is used once.  This is because
// UnsynchronizedTextureRead hits a different bug than it intends to test.  This test makes sure the
// image is put in the right layout, by using it together with another texture (i.e. a different
// descriptor set).
TEST_P(MultithreadingTestES3, UnsynchronizedTextureReads3)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    constexpr GLubyte kInitialData[4] = {127, 63, 191, 255};

    GLuint texture = 0;

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0CreateTextureAndDraw,
        Thread1DrawAndFlush,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Start));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Create a texture, and record a command that draws into it.
        GLTexture color;
        texture = color;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kInitialData);

        glActiveTexture(GL_TEXTURE1);
        GLTexture color2;
        glBindTexture(GL_TEXTURE_2D, color2);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, kInitialData);

        ANGLE_GL_PROGRAM(setupTexture, essl1_shaders::vs::Texture2D(),
                         R"(precision mediump float;
uniform sampler2D tex2D;
uniform sampler2D tex2D2;
varying vec2 v_texCoord;

void main()
{
    gl_FragColor = texture2D(tex2D, v_texCoord) + texture2D(tex2D2, v_texCoord);
})");
        drawQuad(setupTexture, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        glFinish();

        ANGLE_GL_PROGRAM(drawTexture, essl1_shaders::vs::Texture2D(),
                         essl1_shaders::fs::Texture2D());
        drawQuad(drawTexture, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Don't flush yet; this leaves the descriptor set updates to the texture pending in the
        // Vulkan backend.
        threadSynchronization.nextStep(Step::Thread0CreateTextureAndDraw);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1DrawAndFlush));

        // Flush after thread 1
        EXPECT_PIXEL_COLOR_NEAR(
            0, 0, GLColor(kInitialData[0], kInitialData[1], kInitialData[2], kInitialData[3]), 1);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        threadSynchronization.nextStep(Step::Finish);
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for thread 0 to set up
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0CreateTextureAndDraw));

        // Draw with the same texture, in the same way as thread 0.  This ensures that the
        // descriptor sets used in the Vulkan backend are identical.
        glBindTexture(GL_TEXTURE_2D, texture);
        ANGLE_GL_PROGRAM(drawTexture, essl1_shaders::vs::Texture2D(),
                         essl1_shaders::fs::Texture2D());
        drawQuad(drawTexture, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Flush
        EXPECT_PIXEL_COLOR_NEAR(
            0, 0, GLColor(kInitialData[0], kInitialData[1], kInitialData[2], kInitialData[3]), 1);

        threadSynchronization.nextStep(Step::Thread1DrawAndFlush);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Test framebuffer fetch program used between share groups.
void MultithreadingTestES3::testFramebufferFetch(DrawOrder drawOrder)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    GLProgram framebufferFetchProgram;

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
layout(noncoherent, location = 0) inout highp vec4 o_color;

uniform highp vec4 u_color;
void main (void)
{
    o_color += u_color;
})";

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0PreCreateProgram,
        Thread1CreateProgram,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Start));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Open a render pass, if requested.
        if (drawOrder == DrawOrder::Before)
        {
            ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
            drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
            ASSERT_GL_NO_ERROR();
        }

        threadSynchronization.nextStep(Step::Thread0PreCreateProgram);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1CreateProgram));

        // Render using the framebuffer fetch program
        if (drawOrder == DrawOrder::After)
        {
            ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
            drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
            ASSERT_GL_NO_ERROR();
        }

        glFramebufferFetchBarrierEXT();

        glUseProgram(framebufferFetchProgram);
        GLint colorLocation = glGetUniformLocation(framebufferFetchProgram, "u_color");
        glUniform4f(colorLocation, 1, 0, 0, 0);
        drawQuad(framebufferFetchProgram, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);

        threadSynchronization.nextStep(Step::Finish);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for thread 0 to set up
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0PreCreateProgram));

        // Create the framebuffer fetch program
        framebufferFetchProgram.makeRaster(essl3_shaders::vs::Simple(), kFS);
        glUseProgram(framebufferFetchProgram);

        // Notify the other thread to use it
        threadSynchronization.nextStep(Step::Thread1CreateProgram);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glFramebufferFetchBarrierEXT();

        glUseProgram(framebufferFetchProgram);
        GLint colorLocation = glGetUniformLocation(framebufferFetchProgram, "u_color");
        glUniform4f(colorLocation, 0, 0, 1, 0);
        drawQuad(framebufferFetchProgram, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Thread 1 creates the framebuffer fetch program.  Thread 0 proceeds to use it.
TEST_P(MultithreadingTestES3, CreateFramebufferFetchBeforeRenderPass)
{
    testFramebufferFetch(DrawOrder::After);
}

// Thread 1 creates the framebuffer fetch program while thread 0 is mid render pass.  Thread 0
// proceeds to use the framebuffer fetch program in the rest of its render pass.
TEST_P(MultithreadingTestES3, CreateFramebufferFetchMidRenderPass)
{
    testFramebufferFetch(DrawOrder::Before);
}

// Test async monolithic pipeline creation in the Vulkan backend vs shared programs.  This test
// makes one context/thread create a set of programs, then has another context/thread use them a few
// times, and then the original context destroys them.
TEST_P(MultithreadingTestES3, ProgramUseAndDestroyInTwoContexts)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    GLProgram programs[6];

    GLsync sync = 0;

    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0CreatePrograms,
        Thread1UsePrograms,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Start));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Create the programs
        programs[0].makeRaster(essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        programs[1].makeRaster(essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        programs[2].makeRaster(essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
        programs[3].makeRaster(essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Checkered());
        programs[4].makeRaster(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        programs[5].makeRaster(essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

        EXPECT_TRUE(programs[0].valid());
        EXPECT_TRUE(programs[1].valid());
        EXPECT_TRUE(programs[2].valid());
        EXPECT_TRUE(programs[3].valid());
        EXPECT_TRUE(programs[4].valid());
        EXPECT_TRUE(programs[5].valid());

        threadSynchronization.nextStep(Step::Thread0CreatePrograms);
        // Wait for the other thread to use the programs
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1UsePrograms));

        // Destroy them
        glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
        programs[0].reset();
        programs[1].reset();
        programs[2].reset();
        programs[3].reset();
        programs[4].reset();
        programs[5].reset();

        threadSynchronization.nextStep(Step::Finish);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        // Wait for thread 0 to create the programs
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0CreatePrograms));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Use them a few times.
        drawQuad(programs[0], essl1_shaders::PositionAttrib(), 0.0f);
        drawQuad(programs[1], essl1_shaders::PositionAttrib(), 0.0f);
        drawQuad(programs[2], essl1_shaders::PositionAttrib(), 0.0f);
        drawQuad(programs[3], essl1_shaders::PositionAttrib(), 0.0f);
        drawQuad(programs[4], essl1_shaders::PositionAttrib(), 0.0f);

        drawQuad(programs[0], essl1_shaders::PositionAttrib(), 0.0f);
        drawQuad(programs[1], essl1_shaders::PositionAttrib(), 0.0f);
        drawQuad(programs[2], essl1_shaders::PositionAttrib(), 0.0f);

        drawQuad(programs[0], essl1_shaders::PositionAttrib(), 0.0f);

        sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        ASSERT_NE(sync, nullptr);

        // Notify the other thread to destroy the programs.
        threadSynchronization.nextStep(Step::Thread1UsePrograms);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

        // Clean up
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Tests that Context with High Priority will correctly sample Texture rendered by Share Context
// with Low Priority.
TEST_P(MultithreadingTestES3, RenderThenSampleDifferentContextPriority)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    constexpr size_t kIterationCountMax = 10;

    const bool reduceLoad       = isSwiftshader();
    const size_t iterationCount = reduceLoad ? 3 : kIterationCountMax;
    const size_t heavyDrawCount = reduceLoad ? 25 : 100;

    // Initialize contexts
    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();
    EGLConfig config  = window->getConfig();

    // Large enough texture to catch timing problems.
    constexpr GLsizei kTexSize       = 1024;
    constexpr size_t kThreadCount    = 2;
    EGLSurface surface[kThreadCount] = {EGL_NO_SURFACE, EGL_NO_SURFACE};
    EGLContext ctx[kThreadCount]     = {EGL_NO_CONTEXT, EGL_NO_CONTEXT};

    EGLint priorities[kThreadCount] = {EGL_CONTEXT_PRIORITY_LOW_IMG, EGL_CONTEXT_PRIORITY_HIGH_IMG};

    EGLint pbufferAttributes[kThreadCount][6] = {
        {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE, EGL_NONE},
        {EGL_WIDTH, kTexSize, EGL_HEIGHT, kTexSize, EGL_NONE, EGL_NONE}};

    EGLint attributes[]     = {EGL_CONTEXT_PRIORITY_LEVEL_IMG, EGL_NONE,
                               EGL_CONTEXT_VIRTUALIZATION_GROUP_ANGLE, EGL_NONE, EGL_NONE};
    EGLint *extraAttributes = attributes;
    if (!IsEGLDisplayExtensionEnabled(dpy, "EGL_ANGLE_context_virtualization"))
    {
        attributes[2] = EGL_NONE;
    }
    if (!IsEGLDisplayExtensionEnabled(dpy, "EGL_IMG_context_priority"))
    {
        // Run tests with single priority anyway.
        extraAttributes += 2;
    }

    for (size_t t = 0; t < kThreadCount; ++t)
    {
        surface[t] = eglCreatePbufferSurface(dpy, config, pbufferAttributes[t]);
        EXPECT_EGL_SUCCESS();

        attributes[1] = priorities[t];
        attributes[3] = mVirtualizationGroup++;

        ctx[t] = window->createContext(t == 0 ? EGL_NO_CONTEXT : ctx[0], extraAttributes);
        EXPECT_NE(EGL_NO_CONTEXT, ctx[t]);
    }

    // Synchronization tools to ensure the two threads are interleaved as designed by this test.
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0Init,
        Thread1Init,
        Thread0Draw,
        Thread1Draw,
        Finish = Thread0Draw + kIterationCountMax * 2,
        Abort,
    };
    Step currentStep = Step::Start;

    GLTexture texture;
    GLsync thread0DrawSyncObj;

    auto calculateTestColor = [](size_t i) {
        return GLColor(i % 256, (i + 1) % 256, (i + 2) % 256, 255);
    };
    auto makeStep = [](Step base, size_t i) {
        return static_cast<Step>(static_cast<size_t>(base) + i * 2);
    };

    // Render to the texture.
    std::thread thread0 = std::thread([&]() {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[0], surface[0], ctx[0]));
        EXPECT_EGL_SUCCESS();

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        glViewport(0, 0, kTexSize, kTexSize);

        ANGLE_GL_PROGRAM(colorProgram, essl1_shaders::vs::Simple(),
                         essl1_shaders::fs::UniformColor());
        GLint colorLocation =
            glGetUniformLocation(colorProgram, angle::essl1_shaders::ColorUniform());
        ASSERT_NE(-1, colorLocation);
        glUseProgram(colorProgram);

        // Notify second thread that initialization is finished.
        threadSynchronization.nextStep(Step::Thread0Init);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1Init));

        for (size_t i = 0; i < iterationCount; ++i)
        {
            // Simulate heavy work...
            glUniform4f(colorLocation, 0.0f, 0.0f, 0.0f, 0.0f);
            for (size_t j = 0; j < heavyDrawCount; ++j)
            {
                drawQuad(colorProgram, essl1_shaders::PositionAttrib(), 0.5f);
            }

            // Draw with test color.
            Vector4 color = calculateTestColor(i).toNormalizedVector();
            glUniform4f(colorLocation, color.x(), color.y(), color.z(), color.w());
            drawQuad(colorProgram, essl1_shaders::PositionAttrib(), 0.5f);

            thread0DrawSyncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            ASSERT_GL_NO_ERROR();

            // Notify second thread that draw is finished.
            threadSynchronization.nextStep(makeStep(Step::Thread0Draw, i));
            ASSERT_TRUE(threadSynchronization.waitForStep(
                (i == iterationCount - 1) ? Step::Finish : makeStep(Step::Thread1Draw, i)));
        }

        EXPECT_GL_NO_ERROR();
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    // Sample texture
    std::thread thread1 = std::thread([&]() {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[1], surface[1], ctx[1]));
        EXPECT_EGL_SUCCESS();

        glViewport(0, 0, kTexSize, kTexSize);

        ANGLE_GL_PROGRAM(textureProgram, essl1_shaders::vs::Texture2D(),
                         essl1_shaders::fs::Texture2D());
        glUseProgram(textureProgram);

        // Wait for first thread to finish initializing.
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0Init));

        glBindTexture(GL_TEXTURE_2D, texture);

        // Wait for first thread to draw using the shared texture.
        threadSynchronization.nextStep(Step::Thread1Init);

        for (size_t i = 0; i < iterationCount; ++i)
        {
            ASSERT_TRUE(threadSynchronization.waitForStep(makeStep(Step::Thread0Draw, i)));

            ASSERT_TRUE(thread0DrawSyncObj != nullptr);
            glWaitSync(thread0DrawSyncObj, 0, GL_TIMEOUT_IGNORED);
            ASSERT_GL_NO_ERROR();

            // Should draw test color.
            drawQuad(textureProgram, essl1_shaders::PositionAttrib(), 0.5f);

            // Check test color in four corners.
            GLColor color = calculateTestColor(i);
            EXPECT_PIXEL_EQ(0, 0, color.R, color.G, color.B, color.A);
            EXPECT_PIXEL_EQ(0, kTexSize - 1, color.R, color.G, color.B, color.A);
            EXPECT_PIXEL_EQ(kTexSize - 1, 0, color.R, color.G, color.B, color.A);
            EXPECT_PIXEL_EQ(kTexSize - 1, kTexSize - 1, color.R, color.G, color.B, color.A);

            glDeleteSync(thread0DrawSyncObj);
            ASSERT_GL_NO_ERROR();
            thread0DrawSyncObj = nullptr;

            threadSynchronization.nextStep(
                (i == iterationCount - 1) ? Step::Finish : makeStep(Step::Thread1Draw, i));
        }

        EXPECT_GL_NO_ERROR();
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    thread0.join();
    thread1.join();

    ASSERT_NE(currentStep, Step::Abort);

    // Clean up
    for (size_t t = 0; t < kThreadCount; ++t)
    {
        eglDestroySurface(dpy, surface[t]);
        eglDestroyContext(dpy, ctx[t]);
    }
}

// Tests that newly created Context with High Priority will correctly sample Texture already
// rendered by Share Context with Low Priority.
TEST_P(MultithreadingTestES3, RenderThenSampleInNewContextWithDifferentPriority)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));

    const bool reduceLoad       = isSwiftshader();
    const size_t heavyDrawCount = reduceLoad ? 75 : 1000;

    // Initialize contexts
    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();
    EGLConfig config  = window->getConfig();

    // Large enough texture to catch timing problems.
    constexpr GLsizei kTexSize       = 1024;
    constexpr size_t kThreadCount    = 2;
    EGLSurface surface[kThreadCount] = {EGL_NO_SURFACE, EGL_NO_SURFACE};
    EGLContext ctx[kThreadCount]     = {EGL_NO_CONTEXT, EGL_NO_CONTEXT};

    EGLint priorities[kThreadCount] = {EGL_CONTEXT_PRIORITY_LOW_IMG, EGL_CONTEXT_PRIORITY_HIGH_IMG};

    EGLint pbufferAttributes[kThreadCount][6] = {
        {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE, EGL_NONE},
        {EGL_WIDTH, kTexSize, EGL_HEIGHT, kTexSize, EGL_NONE, EGL_NONE}};

    EGLint attributes[]     = {EGL_CONTEXT_PRIORITY_LEVEL_IMG, EGL_NONE,
                               EGL_CONTEXT_VIRTUALIZATION_GROUP_ANGLE, EGL_NONE, EGL_NONE};
    EGLint *extraAttributes = attributes;
    if (!IsEGLDisplayExtensionEnabled(dpy, "EGL_ANGLE_context_virtualization"))
    {
        attributes[2] = EGL_NONE;
    }
    if (!IsEGLDisplayExtensionEnabled(dpy, "EGL_IMG_context_priority"))
    {
        // Run tests with single priority anyway.
        extraAttributes += 2;
    }

    for (size_t t = 0; t < kThreadCount; ++t)
    {
        surface[t] = eglCreatePbufferSurface(dpy, config, pbufferAttributes[t]);
        EXPECT_EGL_SUCCESS();

        attributes[1] = priorities[t];
        attributes[3] = mVirtualizationGroup++;

        // Second context will be created in a thread 1
        if (t == 0)
        {
            ctx[t] = window->createContext(t == 0 ? EGL_NO_CONTEXT : ctx[0], extraAttributes);
            EXPECT_NE(EGL_NO_CONTEXT, ctx[t]);
        }
    }

    // Synchronization tools to ensure the two threads are interleaved as designed by this test.
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0Draw,
        Thread1Draw,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    // Create shared resources before threads to minimize timing delays.
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[0], surface[0], ctx[0]));
    EXPECT_EGL_SUCCESS();

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    ANGLE_GL_PROGRAM(textureProgram, essl1_shaders::vs::Texture2D(),
                     essl1_shaders::fs::Texture2D());

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glViewport(0, 0, kTexSize, kTexSize);

    EXPECT_GL_NO_ERROR();
    EXPECT_EGL_TRUE(window->makeCurrent());
    EXPECT_EGL_SUCCESS();

    GLsync thread0DrawSyncObj;

    // Render to the texture.
    std::thread thread0 = std::thread([&]() {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[0], surface[0], ctx[0]));
        EXPECT_EGL_SUCCESS();

        // Enable additive blend
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        GLuint query;
        glGenQueries(1, &query);
        glBeginQuery(GL_TIME_ELAPSED_EXT, query);
        ASSERT_GL_NO_ERROR();

        // Simulate heavy work...
        glUseProgram(redProgram);
        for (size_t j = 0; j < heavyDrawCount; ++j)
        {
            // Draw with Red color.
            drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
        }

        // Draw with Green color.
        glUseProgram(greenProgram);
        drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.5f);

        // This should force "flushToPrimary()"
        glEndQuery(GL_TIME_ELAPSED_EXT);
        glDeleteQueries(1, &query);
        ASSERT_GL_NO_ERROR();

        // Continue draw with Blue color after flush...
        glUseProgram(blueProgram);
        drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.5f);

        thread0DrawSyncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        ASSERT_GL_NO_ERROR();

        // Notify second thread that draw is finished.
        threadSynchronization.nextStep(Step::Thread0Draw);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        EXPECT_GL_NO_ERROR();
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    // Sample texture
    std::thread thread1 = std::thread([&]() {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        // Wait for first thread to finish draw.
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0Draw));

        // Create High priority Context when Low priority Context already rendered to the texture.
        ctx[1] = window->createContext(ctx[0], extraAttributes);
        EXPECT_NE(EGL_NO_CONTEXT, ctx[1]);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[1], surface[1], ctx[1]));
        EXPECT_EGL_SUCCESS();

        glViewport(0, 0, kTexSize, kTexSize);

        ASSERT_TRUE(thread0DrawSyncObj != nullptr);
        glWaitSync(thread0DrawSyncObj, 0, GL_TIMEOUT_IGNORED);
        ASSERT_GL_NO_ERROR();

        // Should draw test color.
        glUseProgram(textureProgram);
        glBindTexture(GL_TEXTURE_2D, texture);
        drawQuad(textureProgram, essl1_shaders::PositionAttrib(), 0.5f);

        // Check test color in four corners.
        GLColor color = GLColor::white;
        EXPECT_PIXEL_EQ(0, 0, color.R, color.G, color.B, color.A);
        EXPECT_PIXEL_EQ(0, kTexSize - 1, color.R, color.G, color.B, color.A);
        EXPECT_PIXEL_EQ(kTexSize - 1, 0, color.R, color.G, color.B, color.A);
        EXPECT_PIXEL_EQ(kTexSize - 1, kTexSize - 1, color.R, color.G, color.B, color.A);

        glDeleteSync(thread0DrawSyncObj);
        ASSERT_GL_NO_ERROR();
        thread0DrawSyncObj = nullptr;

        threadSynchronization.nextStep(Step::Finish);

        EXPECT_GL_NO_ERROR();
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    thread0.join();
    thread1.join();

    ASSERT_NE(currentStep, Step::Abort);

    // Clean up
    for (size_t t = 0; t < kThreadCount; ++t)
    {
        eglDestroySurface(dpy, surface[t]);
        eglDestroyContext(dpy, ctx[t]);
    }
}

// Tests that Context with High Priority will correctly sample EGLImage target Texture rendered by
// other Context with Low Priority into EGLImage source texture.
TEST_P(MultithreadingTestES3, RenderThenSampleDifferentContextPriorityUsingEGLImage)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!hasWaitSyncExtension() || !hasGLSyncExtension());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_EGL_image"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));

    const bool reduceLoad       = isSwiftshader();
    const size_t heavyDrawCount = reduceLoad ? 75 : 1000;

    // Initialize contexts
    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();
    EGLConfig config  = window->getConfig();

    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(dpy, "EGL_KHR_image_base"));
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(dpy, "EGL_KHR_gl_texture_2D_image"));

    // Large enough texture to catch timing problems.
    constexpr GLsizei kTexSize       = 1024;
    constexpr size_t kThreadCount    = 2;
    EGLSurface surface[kThreadCount] = {EGL_NO_SURFACE, EGL_NO_SURFACE};
    EGLContext ctx[kThreadCount]     = {EGL_NO_CONTEXT, EGL_NO_CONTEXT};

    EGLint priorities[kThreadCount] = {EGL_CONTEXT_PRIORITY_LOW_IMG, EGL_CONTEXT_PRIORITY_HIGH_IMG};

    EGLint pbufferAttributes[kThreadCount][6] = {
        {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE, EGL_NONE},
        {EGL_WIDTH, kTexSize, EGL_HEIGHT, kTexSize, EGL_NONE, EGL_NONE}};

    EGLint attributes[]     = {EGL_CONTEXT_PRIORITY_LEVEL_IMG, EGL_NONE,
                               EGL_CONTEXT_VIRTUALIZATION_GROUP_ANGLE, EGL_NONE, EGL_NONE};
    EGLint *extraAttributes = attributes;
    if (!IsEGLDisplayExtensionEnabled(dpy, "EGL_ANGLE_context_virtualization"))
    {
        attributes[2] = EGL_NONE;
    }
    if (!IsEGLDisplayExtensionEnabled(dpy, "EGL_IMG_context_priority"))
    {
        // Run tests with single priority anyway.
        extraAttributes += 2;
    }

    for (size_t t = 0; t < kThreadCount; ++t)
    {
        surface[t] = eglCreatePbufferSurface(dpy, config, pbufferAttributes[t]);
        EXPECT_EGL_SUCCESS();

        attributes[1] = priorities[t];
        attributes[3] = mVirtualizationGroup++;

        // Contexts not shared
        ctx[t] = window->createContext(EGL_NO_CONTEXT, extraAttributes);
        EXPECT_NE(EGL_NO_CONTEXT, ctx[t]);
    }

    // Synchronization tools to ensure the two threads are interleaved as designed by this test.
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread1Init,
        Thread0Draw,
        Thread1Draw,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    EGLImage image  = EGL_NO_IMAGE_KHR;
    EGLSyncKHR sync = EGL_NO_SYNC_KHR;

    // Render to the EGLImage source texture.
    std::thread thread0 = std::thread([&]() {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[0], surface[0], ctx[0]));
        EXPECT_EGL_SUCCESS();

        // Create source texture.
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        glViewport(0, 0, kTexSize, kTexSize);

        ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

        // Wait for second thread to finish initializing.
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1Init));

        // Enable additive blend
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        GLuint query;
        glGenQueries(1, &query);
        glBeginQuery(GL_TIME_ELAPSED_EXT, query);
        ASSERT_GL_NO_ERROR();

        // Simulate heavy work...
        glUseProgram(redProgram);
        for (size_t j = 0; j < heavyDrawCount; ++j)
        {
            // Draw with Red color.
            drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
        }

        // Draw with Green color.
        glUseProgram(greenProgram);
        drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.5f);

        // This should force "flushToPrimary()"
        glEndQuery(GL_TIME_ELAPSED_EXT);
        glDeleteQueries(1, &query);
        ASSERT_GL_NO_ERROR();

        // Continue draw with Blue color after flush...
        glUseProgram(blueProgram);
        drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.5f);

        sync = eglCreateSyncKHR(dpy, EGL_SYNC_FENCE_KHR, nullptr);
        EXPECT_NE(sync, EGL_NO_SYNC_KHR);

        // Create EGLImage.
        image = eglCreateImageKHR(
            dpy, ctx[0], EGL_GL_TEXTURE_2D_KHR,
            reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(texture)), nullptr);
        ASSERT_EGL_SUCCESS();

        // Notify second thread that draw is finished.
        threadSynchronization.nextStep(Step::Thread0Draw);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        EXPECT_GL_NO_ERROR();
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    // Sample texture
    std::thread thread1 = std::thread([&]() {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[1], surface[1], ctx[1]));
        EXPECT_EGL_SUCCESS();

        glViewport(0, 0, kTexSize, kTexSize);

        ANGLE_GL_PROGRAM(textureProgram, essl1_shaders::vs::Texture2D(),
                         essl1_shaders::fs::Texture2D());
        glUseProgram(textureProgram);

        // Create target texture.
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Wait for first thread to draw into the source texture.
        threadSynchronization.nextStep(Step::Thread1Init);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0Draw));

        // Wait for draw to complete
        ASSERT_TRUE(eglWaitSyncKHR(dpy, sync, 0));

        // Specify target texture from EGLImage.
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);
        ASSERT_GL_NO_ERROR();

        // Should draw test color.
        drawQuad(textureProgram, essl1_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();

        // Check test color in four corners.
        GLColor color = GLColor::white;
        EXPECT_PIXEL_EQ(0, 0, color.R, color.G, color.B, color.A);
        EXPECT_PIXEL_EQ(0, kTexSize - 1, color.R, color.G, color.B, color.A);
        EXPECT_PIXEL_EQ(kTexSize - 1, 0, color.R, color.G, color.B, color.A);
        EXPECT_PIXEL_EQ(kTexSize - 1, kTexSize - 1, color.R, color.G, color.B, color.A);

        EXPECT_EGL_TRUE(eglDestroyImageKHR(dpy, image));
        image = EGL_NO_IMAGE_KHR;

        EXPECT_EGL_TRUE(eglDestroySyncKHR(dpy, sync));
        sync = EGL_NO_SYNC_KHR;

        threadSynchronization.nextStep(Step::Finish);

        EXPECT_GL_NO_ERROR();
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    thread0.join();
    thread1.join();

    ASSERT_NE(currentStep, Step::Abort);

    // Clean up
    for (size_t t = 0; t < kThreadCount; ++t)
    {
        eglDestroySurface(dpy, surface[t]);
        eglDestroyContext(dpy, ctx[t]);
    }
}

// Tests mixing commands of Contexts with different Priorities in a single Command Buffers (Vulkan).
TEST_P(MultithreadingTestES3, ContextPriorityMixing)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));

    constexpr size_t kIterationCountMax = 10;

    const bool reduceLoad       = isSwiftshader();
    const size_t iterationCount = reduceLoad ? 3 : kIterationCountMax;
    const size_t heavyDrawCount = reduceLoad ? 25 : 100;

    // Initialize contexts
    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();
    EGLConfig config  = window->getConfig();

    // Large enough texture to catch timing problems.
    constexpr GLsizei kTexSize       = 1024;
    constexpr size_t kThreadCount    = 2;
    EGLSurface surface[kThreadCount] = {EGL_NO_SURFACE, EGL_NO_SURFACE};
    EGLContext ctx[kThreadCount]     = {EGL_NO_CONTEXT, EGL_NO_CONTEXT};

    EGLint priorities[kThreadCount] = {EGL_CONTEXT_PRIORITY_LOW_IMG, EGL_CONTEXT_PRIORITY_HIGH_IMG};

    EGLint pbufferAttributes[kThreadCount][6] = {
        {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE, EGL_NONE},
        {EGL_WIDTH, kTexSize, EGL_HEIGHT, kTexSize, EGL_NONE, EGL_NONE}};

    EGLint attributes[]     = {EGL_CONTEXT_PRIORITY_LEVEL_IMG, EGL_NONE,
                               EGL_CONTEXT_VIRTUALIZATION_GROUP_ANGLE, EGL_NONE, EGL_NONE};
    EGLint *extraAttributes = attributes;
    if (!IsEGLDisplayExtensionEnabled(dpy, "EGL_ANGLE_context_virtualization"))
    {
        attributes[2] = EGL_NONE;
    }
    if (!IsEGLDisplayExtensionEnabled(dpy, "EGL_IMG_context_priority"))
    {
        // Run tests with single priority anyway.
        extraAttributes += 2;
    }

    for (size_t t = 0; t < kThreadCount; ++t)
    {
        surface[t] = eglCreatePbufferSurface(dpy, config, pbufferAttributes[t]);
        EXPECT_EGL_SUCCESS();

        attributes[1] = priorities[t];
        attributes[3] = mVirtualizationGroup++;

        // Contexts not shared
        ctx[t] = window->createContext(EGL_NO_CONTEXT, extraAttributes);
        EXPECT_NE(EGL_NO_CONTEXT, ctx[t]);
    }

    // Synchronization tools to ensure the two threads are interleaved as designed by this test.
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread1DrawColor,
        Thread0Iterate,
        Finish = Thread1DrawColor + kIterationCountMax * 2,
        Abort,
    };
    Step currentStep = Step::Start;

    auto makeStep = [](Step base, size_t i) {
        return static_cast<Step>(static_cast<size_t>(base) + i * 2);
    };

    // Triggers commands submission.
    std::thread thread0 = std::thread([&]() {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[0], surface[0], ctx[0]));
        EXPECT_EGL_SUCCESS();

        ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

        for (size_t i = 0; i < iterationCount; ++i)
        {
            ASSERT_TRUE(threadSynchronization.waitForStep(makeStep(Step::Thread1DrawColor, i)));

            glUseProgram(redProgram);
            drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
            ASSERT_GL_NO_ERROR();

            // This should perform commands submission.
            EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
            EXPECT_EGL_SUCCESS();

            threadSynchronization.nextStep(makeStep(Step::Thread0Iterate, i));

            EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[0], surface[0], ctx[0]));
            EXPECT_EGL_SUCCESS();
        }

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        EXPECT_GL_NO_ERROR();
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    // Render and then sample texture.
    std::thread thread1 = std::thread([&]() {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface[1], surface[1], ctx[1]));
        EXPECT_EGL_SUCCESS();

        glDisable(GL_DEPTH_TEST);
        glViewport(0, 0, kTexSize, kTexSize);

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        ANGLE_GL_PROGRAM(colorProgram, essl1_shaders::vs::Simple(),
                         essl1_shaders::fs::UniformColor());
        GLint colorLocation =
            glGetUniformLocation(colorProgram, angle::essl1_shaders::ColorUniform());
        ASSERT_NE(-1, colorLocation);

        ANGLE_GL_PROGRAM(textureProgram, essl1_shaders::vs::Texture2D(),
                         essl1_shaders::fs::Texture2D());

        for (size_t i = 0; i < iterationCount; ++i)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glUseProgram(colorProgram);

            GLuint query;
            glGenQueries(1, &query);
            glBeginQuery(GL_TIME_ELAPSED_EXT, query);
            ASSERT_GL_NO_ERROR();

            // Simulate heavy work...
            glUniform4f(colorLocation, 0.0f, 0.0f, 0.0f, 0.0f);
            for (size_t j = 0; j < heavyDrawCount; ++j)
            {
                drawQuad(colorProgram, essl1_shaders::PositionAttrib(), 0.5f);
            }

            // Draw with test color.
            GLColor color(i % 256, (i + 1) % 256, (i + 2) % 256, 255);
            Vector4 colorF = color.toNormalizedVector();
            glUniform4f(colorLocation, colorF.x(), colorF.y(), colorF.z(), colorF.w());
            drawQuad(colorProgram, essl1_shaders::PositionAttrib(), 0.5f);
            ASSERT_GL_NO_ERROR();

            // This should force "flushToPrimary()"
            glEndQuery(GL_TIME_ELAPSED_EXT);
            glDeleteQueries(1, &query);
            ASSERT_GL_NO_ERROR();

            threadSynchronization.nextStep(makeStep(Step::Thread1DrawColor, i));
            ASSERT_TRUE(threadSynchronization.waitForStep(makeStep(Step::Thread0Iterate, i)));

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glUseProgram(textureProgram);

            // Should draw test color.
            drawQuad(textureProgram, essl1_shaders::PositionAttrib(), 0.5f);
            ASSERT_GL_NO_ERROR();

            // Check test color in four corners.
            EXPECT_PIXEL_EQ(0, 0, color.R, color.G, color.B, color.A);
            EXPECT_PIXEL_EQ(0, kTexSize - 1, color.R, color.G, color.B, color.A);
            EXPECT_PIXEL_EQ(kTexSize - 1, 0, color.R, color.G, color.B, color.A);
            EXPECT_PIXEL_EQ(kTexSize - 1, kTexSize - 1, color.R, color.G, color.B, color.A);
        }

        threadSynchronization.nextStep(Step::Finish);

        EXPECT_GL_NO_ERROR();
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    });

    thread0.join();
    thread1.join();

    ASSERT_NE(currentStep, Step::Abort);

    // Clean up
    for (size_t t = 0; t < kThreadCount; ++t)
    {
        eglDestroySurface(dpy, surface[t]);
        eglDestroyContext(dpy, ctx[t]);
    }
}

// Test that it is possible to upload textures in one thread and use them in another with
// synchronization.
TEST_P(MultithreadingTestES3, MultithreadedTextureUploadAndDraw)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    constexpr size_t kTexSize = 4;
    GLTexture texture1;
    GLTexture texture2;
    std::vector<GLColor> textureColors1(kTexSize * kTexSize, GLColor::red);
    std::vector<GLColor> textureColors2(kTexSize * kTexSize, GLColor::green);

    // Sync primitives
    GLsync sync = nullptr;
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0UploadFinish,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    // Threads to upload and draw with textures.
    auto thread0Upload = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Start));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Two mipmap textures are defined here. They are used for drawing in the other thread.
        glBindTexture(GL_TEXTURE_2D, texture1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     textureColors1.data());
        glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTexSize / 2, kTexSize / 2, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, textureColors1.data());
        glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA8, kTexSize / 4, kTexSize / 4, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, textureColors1.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        glBindTexture(GL_TEXTURE_2D, texture2);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     textureColors2.data());
        glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTexSize / 2, kTexSize / 2, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, textureColors2.data());
        glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA8, kTexSize / 4, kTexSize / 4, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, textureColors2.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        // Create a sync object to be used for the draw thread.
        sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        glFlush();
        ASSERT_NE(sync, nullptr);

        threadSynchronization.nextStep(Step::Thread0UploadFinish);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
    };

    auto thread1Draw = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0UploadFinish));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for the sync object to be signaled.
        glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
        ASSERT_GL_NO_ERROR();

        // Draw using the textures from the texture upload thread.
        ANGLE_GL_PROGRAM(textureProgram, essl1_shaders::vs::Texture2D(),
                         essl1_shaders::fs::Texture2D());
        glUseProgram(textureProgram);

        glBindTexture(GL_TEXTURE_2D, texture1);
        drawQuad(textureProgram, essl1_shaders::PositionAttrib(), 0.5f);
        glFlush();
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_RECT_EQ(0, 0, kTexSize, kTexSize, GLColor::red);

        glBindTexture(GL_TEXTURE_2D, texture2);
        drawQuad(textureProgram, essl1_shaders::PositionAttrib(), 0.5f);
        glFlush();
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_RECT_EQ(0, 0, kTexSize, kTexSize, GLColor::green);

        threadSynchronization.nextStep(Step::Finish);
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0Upload),
        std::move(thread1Draw),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Test that it is possible to create a new context after uploading mutable mipmap textures in the
// previous context, and use them in the new context.
TEST_P(MultithreadingTestES3, CreateNewContextAfterTextureUploadOnNewThread)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    constexpr size_t kTexSize = 4;
    GLTexture texture1;
    GLTexture texture2;
    std::vector<GLColor> textureColors1(kTexSize * kTexSize, GLColor::red);
    std::vector<GLColor> textureColors2(kTexSize * kTexSize, GLColor::green);

    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();

    std::thread thread = std::thread([&]() {
        // Create a context and upload the textures.
        EGLContext ctx1 = createMultithreadedContext(window, EGL_NO_CONTEXT);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx1));

        glBindTexture(GL_TEXTURE_2D, texture1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     textureColors1.data());
        glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTexSize / 2, kTexSize / 2, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, textureColors1.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        glBindTexture(GL_TEXTURE_2D, texture2);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     textureColors2.data());
        glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTexSize / 2, kTexSize / 2, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, textureColors2.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        // Create a new context and use the uploaded textures.
        EGLContext ctx2 = createMultithreadedContext(window, ctx1);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx2));

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);
        EXPECT_PIXEL_RECT_EQ(0, 0, kTexSize, kTexSize, GLColor::red);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);
        EXPECT_PIXEL_RECT_EQ(0, 0, kTexSize, kTexSize, GLColor::green);

        // Destroy the contexts.
        EXPECT_EGL_TRUE(eglDestroyContext(dpy, ctx2));
        EXPECT_EGL_TRUE(eglDestroyContext(dpy, ctx1));
    });

    thread.join();
}

// Test that it is possible to create a new context after uploading mutable mipmap textures in the
// main thread, and use them in the new context.
TEST_P(MultithreadingTestES3, CreateNewContextAfterTextureUploadOnMainThread)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();

    // Upload the textures.
    constexpr size_t kTexSize = 4;
    GLTexture texture1;
    GLTexture texture2;
    std::vector<GLColor> textureColors1(kTexSize * kTexSize, GLColor::red);
    std::vector<GLColor> textureColors2(kTexSize * kTexSize, GLColor::green);

    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 textureColors1.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTexSize / 2, kTexSize / 2, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, textureColors1.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kTexSize, kTexSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 textureColors2.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTexSize / 2, kTexSize / 2, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, textureColors2.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    std::thread thread = std::thread([&]() {
        // Create a context.
        EGLContext ctx1 = createMultithreadedContext(window, window->getContext());
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx1));

        // Wait for the sync object to be signaled.
        glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
        ASSERT_GL_NO_ERROR();

        // Use the uploaded textures in the main thread.
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);
        EXPECT_PIXEL_RECT_EQ(0, 0, kTexSize, kTexSize, GLColor::red);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);
        EXPECT_PIXEL_RECT_EQ(0, 0, kTexSize, kTexSize, GLColor::green);

        // Destroy the context.
        EXPECT_EGL_TRUE(eglDestroyContext(dpy, ctx1));
    });

    thread.join();
}

// Test when lots of upload happens on a different thread at the same time as the main thread doing
// draws.
TEST_P(MultithreadingTestES3, SimultaneousUploadAndDraw)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    // The following shader is used to create busy work while the worker thread is doing something.
    // It intentionally spreads its uniforms and inputs so the main thread has to make many GL
    // calls.
    constexpr char kBusyDrawVS[] = R"(#version 300 es
uniform mediump float x0;
uniform mediump float y0;
uniform mediump float x1;
uniform mediump float y1;

in mediump float r;
in mediump float g;
in mediump float b;
in mediump float a;

out mediump vec4 color;

void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position.x = bit0 == 0 ? x0 : x1;
    gl_Position.y = bit1 == 0 ? y0 : y1;
    gl_Position.z = 0.;
    gl_Position.w = 1.;

    color = vec4(r, g, b, a);
})";
    constexpr char kBusyDrawFS[] = R"(#version 300 es

in mediump vec4 color;
out mediump vec4 colorOut;

void main()
{
    colorOut = color;
})";

    // The following shader is used to consume the results of texture uploads, ensuring appropriate
    // synchronization.
    constexpr char kTextureDrawVS[] = R"(#version 300 es
out mediump vec2 uv;

void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
    uv = vec2(bit0, bit1);
})";
    constexpr char kTextureDrawFS[] = R"(#version 300 es

uniform mediump sampler2D s0;
uniform mediump sampler2D s1;
uniform mediump sampler2D s2;
uniform mediump sampler2D s3;
uniform mediump sampler2D s4;
uniform mediump sampler2D s5;
uniform mediump sampler2D s6;
uniform mediump sampler2D s7;
uniform mediump sampler2D s8;
uniform mediump sampler2D s9;

in mediump vec2 uv;
out mediump vec4 colorOut;

void main()
{
    highp vec4 result = texture(s0, uv) +
                        texture(s1, uv) +
                        texture(s2, uv) +
                        texture(s3, uv) +
                        texture(s4, uv) +
                        texture(s5, uv) +
                        texture(s6, uv) +
                        texture(s7, uv) +
                        texture(s8, uv) +
                        texture(s9, uv);
    result /= 10.;

    colorOut = result;
})";

    constexpr uint32_t kTextureCount = 10;
    GLuint textures[kTextureCount];

    ASSERT(IsGLExtensionEnabled("GL_KHR_texture_compression_astc_ldr") ||
           IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));
    // Note ASTC may be emulated in ANGLE, so check for BPTC first
    const bool hasBPTC = IsGLExtensionEnabled("GL_EXT_texture_compression_bptc");
    const GLenum compressedFormat =
        hasBPTC ? GL_COMPRESSED_RGBA_BPTC_UNORM_EXT : GL_COMPRESSED_RGBA_ASTC_4x4_KHR;

    std::vector<uint8_t> textureData[kTextureCount];

    constexpr int kSurfaceWidth  = 256;
    constexpr int kSurfaceHeight = 512;
    constexpr int kTexSize       = 1024;

    // Sync primitives
    GLsync sync = nullptr;
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread1Ready,
        Thread0UploadFinish,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    // Threads to upload and draw with textures.
    auto thread0Upload = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for the other thread to set everything up
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1Ready));

        // Perform uploads while the other thread does draws
        for (uint32_t i = 0; i < kTextureCount; ++i)
        {
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTexSize, kTexSize, compressedFormat,
                                      static_cast<GLsizei>(textureData[i].size()),
                                      textureData[i].data());
        }
        ASSERT_GL_NO_ERROR();

        // Create a sync object to be used for the draw thread.
        sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        ASSERT_NE(sync, nullptr);
        ASSERT_GL_NO_ERROR();

        threadSynchronization.nextStep(Step::Thread0UploadFinish);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
    };

    auto thread1Draw = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        ANGLE_GL_PROGRAM(busyDrawProgram, kBusyDrawVS, kBusyDrawFS);

        // Set up the test.  Don't let the other thread work yet.
        glUseProgram(busyDrawProgram);
        GLuint busyDrawX0Loc = glGetUniformLocation(busyDrawProgram, "x0");
        GLuint busyDrawY0Loc = glGetUniformLocation(busyDrawProgram, "y0");
        GLuint busyDrawX1Loc = glGetUniformLocation(busyDrawProgram, "x1");
        GLuint busyDrawY1Loc = glGetUniformLocation(busyDrawProgram, "y1");
        GLuint busyDrawRLoc  = glGetAttribLocation(busyDrawProgram, "r");
        GLuint busyDrawGLoc  = glGetAttribLocation(busyDrawProgram, "g");
        GLuint busyDrawBLoc  = glGetAttribLocation(busyDrawProgram, "b");
        GLuint busyDrawALoc  = glGetAttribLocation(busyDrawProgram, "a");

        ANGLE_GL_PROGRAM(textureDrawProgram, kTextureDrawVS, kTextureDrawFS);
        GLuint textureDrawSamplerLoc[kTextureCount] = {};

        glUseProgram(textureDrawProgram);
        glGenTextures(kTextureCount, textures);
        for (uint32_t i = 0; i < kTextureCount; ++i)
        {
            std::ostringstream name;
            name << "s" << i;

            textureDrawSamplerLoc[i] = glGetUniformLocation(textureDrawProgram, name.str().c_str());

            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexStorage2D(GL_TEXTURE_2D, 1, compressedFormat, kTexSize, kTexSize);

            // Both ASTC 4x4 and BPTC have 1 byte per pixel.  The textures' contents are arbitrary
            // but distinct.
            textureData[i].resize(kTexSize * kTexSize);
            for (int y = 0; y < kTexSize; ++y)
            {
                for (int x = 0; x < kTexSize; ++x)
                {
                    textureData[i][y * kTexSize + x] = (i * 50 + y + x) % 255;
                }
            }
        }
        ASSERT_GL_NO_ERROR();

        // Now that everything is set up, let the upload thread work while this thread does draws.
        threadSynchronization.nextStep(Step::Thread1Ready);

        int w = kSurfaceWidth;
        int h = kSurfaceHeight;

        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, w, h);
        glUseProgram(busyDrawProgram);
        for (uint32_t y = 0; y < 8; ++y)
        {
            for (uint32_t x = 0; x < 8; ++x)
            {
                float width  = w / 4;
                float height = h / 8;

                glUniform1f(busyDrawX0Loc, x * width / w - 1);
                glUniform1f(busyDrawY0Loc, y * height / h);
                glUniform1f(busyDrawX1Loc, (x + 1) * width / w - 1);
                glUniform1f(busyDrawY1Loc, (y + 1) * height / h);

                glVertexAttrib1f(busyDrawRLoc, x / 8.0f);
                glVertexAttrib1f(busyDrawGLoc, y / 8.0f);
                glVertexAttrib1f(busyDrawBLoc, 0);
                glVertexAttrib1f(busyDrawALoc, 1);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }
        ASSERT_GL_NO_ERROR();

        // Wait for the other thread to finish with uploads.
        threadSynchronization.waitForStep(Step::Thread0UploadFinish);

        // Wait for fence and use all textures in a draw.
        glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);

        glUseProgram(textureDrawProgram);
        for (uint32_t i = 0; i < kTextureCount; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glUniform1i(textureDrawSamplerLoc[i], i);
        }
        glViewport(0, 0, w, h / 2);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        ASSERT_GL_NO_ERROR();

        threadSynchronization.nextStep(Step::Finish);

        // Verify results
        for (uint32_t y = 0; y < 8; ++y)
        {
            for (uint32_t x = 0; x < 8; ++x)
            {
                int width  = w / 8;
                int height = h / 16;

                EXPECT_PIXEL_COLOR_NEAR(x * width + width / 2, h - (y * height + height / 2),
                                        GLColor(x * 255 / 8, (7 - y) * 255 / 8, 0, 255), 1);
            }
        }
        ASSERT_GL_NO_ERROR();

        for (uint32_t x = 0; x < 8; ++x)
        {
            // The compressed data is gibberish, just ensure it's not all black.
            EXPECT_PIXEL_NE(x * w / 8, h / 4, 0, 0, 0, 0);
        }
        ASSERT_GL_NO_ERROR();
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0Upload),
        std::move(thread1Draw),
    };

    RunLockStepThreadsWithSize(getEGLWindow(), kSurfaceWidth, kSurfaceHeight, threadFuncs.size(),
                               threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Test that calling glUniformBlockBinding on one context affects all contexts.
TEST_P(MultithreadingTestES3, UniformBlockBinding)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    constexpr char kVS[] = R"(#version 300 es
void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
})";
    constexpr char kFS[] = R"(#version 300 es
out mediump vec4 colorOut;

layout(std140) uniform buffer { mediump vec4 color; };

void main()
{
    colorOut = color;
})";

    GLProgram program;
    GLint uniformBufferIndex;

    // Sync primitives
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread1Ready,
        Thread0BindingChanged,
        Thread1FinishedDrawing,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    // Threads to create programs and draw with different uniform blocks.
    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Create buffers bound to bindings 1 and 2
        constexpr std::array<float, 4> kRed              = {1, 0, 0, 1};
        constexpr std::array<float, 4> kTransparentGreen = {0, 1, 0, 0};
        GLBuffer red, transparentGreen;
        glBindBuffer(GL_UNIFORM_BUFFER, red);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(kRed), kRed.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, transparentGreen);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(kTransparentGreen), kTransparentGreen.data(),
                     GL_STATIC_DRAW);

        glBindBufferBase(GL_UNIFORM_BUFFER, 1, transparentGreen);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, red);

        // Wait for the other thread to set everything up
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1Ready));

        // Issue a draw call.  The buffer should be transparent green now
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glUseProgram(program);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Change the binding
        glUniformBlockBinding(program, uniformBufferIndex, 1);
        ASSERT_GL_NO_ERROR();

        // Let the other thread work before any deferred operations for the binding change above are
        // processed in this context.
        threadSynchronization.nextStep(Step::Thread0BindingChanged);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1FinishedDrawing));

        // Draw again, it should accumulate blue and the buffer should become magenta.
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Verify results
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
        ASSERT_GL_NO_ERROR();

        threadSynchronization.nextStep(Step::Finish);
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Create buffers bound to bindings 1 and 2
        constexpr std::array<float, 4> kBlue           = {0, 0, 1, 1};
        constexpr std::array<float, 4> kTransparentRed = {1, 0, 0, 0};
        GLBuffer blue, transparentRed;
        glBindBuffer(GL_UNIFORM_BUFFER, blue);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(kBlue), kBlue.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, transparentRed);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(kTransparentRed), kTransparentRed.data(),
                     GL_STATIC_DRAW);

        glBindBufferBase(GL_UNIFORM_BUFFER, 1, blue);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, transparentRed);

        // Create the program
        program.makeRaster(kVS, kFS);
        glUseProgram(program);
        uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");

        // Configure the buffer binding to binding 2
        glUniformBlockBinding(program, uniformBufferIndex, 2);
        ASSERT_GL_NO_ERROR();

        // Issue a draw call.  The buffer should be transparent red now
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Now that everything is set up, let the other thread continue
        threadSynchronization.nextStep(Step::Thread1Ready);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0BindingChanged));

        // The other thread has changed the binding.  Draw again, it should accumulate blue and the
        // buffer should become magenta.
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Verify results
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
        ASSERT_GL_NO_ERROR();

        // Tell the other thread to finish up.
        threadSynchronization.nextStep(Step::Thread1FinishedDrawing);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Test that observers are notified of a change in foveation state of a texture
TEST_P(MultithreadingTestES3, SharedFoveatedTexture)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_QCOM_texture_foveated"));

    // Shared texture
    GLTexture texture;

    // Sync primitives
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0Draw,
        Thread1Draw,
        Thread0ConfiguredTextureFoveation,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    // Thread to configure texture foveation.
    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Create non-foveated framebuffer and attach shared texture as color attachment
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        EXPECT_GL_NO_ERROR();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        ASSERT_GL_NO_ERROR();
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Render before configuring foveation on the texture
        ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
        glUseProgram(greenProgram);

        // Draw
        drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_GL_NO_ERROR();

        // Verify results
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
        ASSERT_GL_NO_ERROR();

        threadSynchronization.nextStep(Step::Thread0Draw);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1Draw));

        // Configure foveation for the texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FOVEATED_FEATURE_BITS_QCOM,
                        GL_FOVEATION_ENABLE_BIT_QCOM);
        EXPECT_GL_NO_ERROR();
        glTextureFoveationParametersQCOM(texture, 0, 0, 0.0f, 0.0f, 8.0f, 8.0f, 0.0f);
        EXPECT_GL_NO_ERROR();

        threadSynchronization.nextStep(Step::Thread0ConfiguredTextureFoveation);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0Draw));

        // Create non-foveated framebuffer and attach shared texture as color attachment
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        ASSERT_GL_NO_ERROR();
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Render before configuring foveation on the texture
        ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        glUseProgram(redProgram);

        // Draw
        drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_GL_NO_ERROR();

        // Verify results
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        ASSERT_GL_NO_ERROR();

        threadSynchronization.nextStep(Step::Thread1Draw);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0ConfiguredTextureFoveation));

        // Render after texture foveation was configured
        ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
        glUseProgram(blueProgram);

        // Draw
        drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_GL_NO_ERROR();

        // Verify results
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
        ASSERT_GL_NO_ERROR();

        threadSynchronization.nextStep(Step::Finish);
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Test GL_EXT_sRGB_write_control works as expected when multiple contexts are used
TEST_P(MultithreadingTestES3, SharedSrgbTextureMultipleContexts)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_sRGB_write_control"));

    constexpr angle::GLColor encodedToSrgbColor(64, 127, 191, 255);
    constexpr angle::GLColor inputColor(13, 54, 133, 255);

    EGLWindow *window   = getEGLWindow();
    EGLDisplay dpy      = window->getDisplay();
    EGLContext context1 = window->createContext(EGL_NO_CONTEXT, nullptr);
    EGLContext context2 = window->createContext(context1, nullptr);

    // Shared texture
    GLTexture texture;

    // Shared program
    GLuint program;

    // Sync primitives
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0Draw1,
        Thread1Draw1,
        Thread0Draw2,
        Thread1Draw2,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    // Thread0 rendering to shared texture.
    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context1));
        EXPECT_EGL_SUCCESS();

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA_EXT, 1, 1, 0, GL_SRGB_ALPHA_EXT,
                     GL_UNSIGNED_BYTE, nullptr);

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        program = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        ASSERT_NE(0u, program);

        GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());
        ASSERT_NE(-1, colorLocation);

        glUseProgram(program);
        glUniform4fv(colorLocation, 1, inputColor.toNormalizedVector().data());

        glDisable(GL_FRAMEBUFFER_SRGB_EXT);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_PIXEL_COLOR_NEAR(0, 0, inputColor, 1.0);

        threadSynchronization.nextStep(Step::Thread0Draw1);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1Draw1));

        glEnable(GL_FRAMEBUFFER_SRGB_EXT);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_PIXEL_COLOR_NEAR(0, 0, encodedToSrgbColor, 1.0);

        threadSynchronization.nextStep(Step::Thread0Draw2);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1Draw2));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();

        threadSynchronization.nextStep(Step::Finish);
    };

    // Thread1 rendering to shared texture.
    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context2));
        EXPECT_EGL_SUCCESS();

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0Draw1));

        glBindTexture(GL_TEXTURE_2D, texture);
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        ASSERT_NE(0u, program);
        GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());
        ASSERT_NE(-1, colorLocation);

        glUseProgram(program);
        glUniform4fv(colorLocation, 1, inputColor.toNormalizedVector().data());

        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_PIXEL_COLOR_NEAR(0, 0, encodedToSrgbColor, 1.0);

        threadSynchronization.nextStep(Step::Thread1Draw1);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0Draw2));

        glBindTexture(GL_TEXTURE_2D, texture);
        glDisable(GL_FRAMEBUFFER_SRGB_EXT);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_PIXEL_COLOR_NEAR(0, 0, inputColor, 1.0);

        threadSynchronization.nextStep(Step::Thread1Draw2);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));

        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
        EXPECT_EGL_SUCCESS();
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    // Cleanup
    EXPECT_EGL_TRUE(eglDestroyContext(dpy, context1));
    EXPECT_EGL_TRUE(eglDestroyContext(dpy, context2));
    EXPECT_EGL_SUCCESS();

    ASSERT_NE(currentStep, Step::Abort);
}

// Test that a program linked in one context can be bound in another context while link may be
// happening in parallel.
TEST_P(MultithreadingTest, ProgramLinkAndBind)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    GLuint vs;
    GLuint redfs;
    GLuint greenfs;

    GLuint program;

    // Sync primitives
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread1Ready,
        Thread0ProgramLinked,
        Thread1FinishedDrawing,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    // Threads to create programs and draw.
    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        // Wait for thread 1 to bind the program before linking it
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1Ready));

        glUseProgram(program);

        // Link a program, but don't resolve link.
        glDetachShader(program, greenfs);
        glAttachShader(program, redfs);
        glLinkProgram(program);

        // Let the other thread bind and use the program.
        threadSynchronization.nextStep(Step::Thread0ProgramLinked);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1FinishedDrawing));

        // Draw in this context too
        drawQuad(program, essl1_shaders::PositionAttrib(), 0);

        // Verify results
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        ASSERT_GL_NO_ERROR();

        threadSynchronization.nextStep(Step::Finish);
    };

    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        vs      = CompileShader(GL_VERTEX_SHADER, essl1_shaders::vs::Simple());
        redfs   = CompileShader(GL_FRAGMENT_SHADER, essl1_shaders::fs::Red());
        greenfs = CompileShader(GL_FRAGMENT_SHADER, essl1_shaders::fs::Green());
        program = glCreateProgram();

        glAttachShader(program, vs);
        glAttachShader(program, greenfs);
        glLinkProgram(program);
        ASSERT_NE(CheckLinkStatusAndReturnProgram(program, true), 0u);

        // Bind the program before it's relinked.  Otherwise the program is resolved before the
        // binding happens.
        glUseProgram(program);

        threadSynchronization.nextStep(Step::Thread1Ready);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0ProgramLinked));

        // Unbind and rebind for extra testing
        glUseProgram(0);
        glUseProgram(program);

        // Issue a draw call
        drawQuad(program, essl1_shaders::PositionAttrib(), 0);

        // Verify results
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        ASSERT_GL_NO_ERROR();

        // Tell the other thread to finish up.
        threadSynchronization.nextStep(Step::Thread1FinishedDrawing);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreads(getEGLWindow(), threadFuncs.size(), threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Test that two contexts in share group can generate, delete and bind buffers for themselves in
// parallel.
TEST_P(MultithreadingTestES3, SimultaneousBufferBindAndGen)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;

layout(std140) uniform Block
{
    vec4 colorIn;
};

out vec4 color;

void main()
{
    color = colorIn;
})";

    constexpr int kSurfaceWidth  = 32;
    constexpr int kSurfaceHeight = 128;

    // Sync primitives
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0Ready,
        Thread1Ready,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto threadFunc = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context, uint32_t index) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

        // Make sure the two threads start work around the same time
        if (index == 0)
        {
            threadSynchronization.nextStep(Step::Thread0Ready);
            ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1Ready));
        }
        else
        {
            ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0Ready));
            threadSynchronization.nextStep(Step::Thread1Ready);
        }

        std::vector<GLuint> buffers(kSurfaceWidth * kSurfaceHeight);

        glEnable(GL_SCISSOR_TEST);
        for (int y = 0; y < kSurfaceHeight; ++y)
        {
            for (int x = 0; x < kSurfaceWidth; ++x)
            {
                GLuint &buffer            = buffers[y * kSurfaceWidth + x];
                const float bufferData[4] = {
                    ((y * kSurfaceWidth + x + index * 100) % 255) / 255.0f,
                    ((y * kSurfaceWidth + x + index * 100 + 1) % 255) / 255.0f,
                    ((y * kSurfaceWidth + x + index * 100 + 2) % 255) / 255.0f,
                    ((y * kSurfaceWidth + x + index * 100 + 3) % 255) / 255.0f,
                };

                // Generate one buffer per pixel and shade the pixel with it.
                glGenBuffers(1, &buffer);
                glBindBuffer(GL_UNIFORM_BUFFER, buffers[y * kSurfaceWidth + x]);
                glBufferData(GL_UNIFORM_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);
                glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);

                glScissor(x, y, 1, 1);
                drawQuad(program, essl3_shaders::PositionAttrib(), 0);

                if ((x + y) % 2 == 0)
                {
                    glDeleteBuffers(1, &buffer);
                    buffer = 0;
                }
            }
        }

        // Verify the results
        auto verify = [&](int x, int y) {
            const GLColor expect((y * kSurfaceWidth + x + index * 100) % 255,
                                 (y * kSurfaceWidth + x + index * 100 + 1) % 255,
                                 (y * kSurfaceWidth + x + index * 100 + 2) % 255,
                                 (y * kSurfaceWidth + x + index * 100 + 3) % 255);
            EXPECT_PIXEL_COLOR_EQ(x, y, expect);
        };

        verify(0, 0);
        verify(0, kSurfaceHeight - 1);
        verify(kSurfaceWidth - 1, 0);
        verify(kSurfaceWidth - 1, kSurfaceHeight - 1);
        verify(kSurfaceWidth / 2, kSurfaceHeight / 2);
        ASSERT_GL_NO_ERROR();

        if (index == 0)
        {
            threadSynchronization.nextStep(Step::Finish);
        }
        else
        {
            ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
        }
    };

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        threadFunc(dpy, surface, context, 0);
    };
    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        threadFunc(dpy, surface, context, 1);
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreadsWithSize(getEGLWindow(), kSurfaceWidth, kSurfaceHeight, threadFuncs.size(),
                               threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}

// Test that ref counting is thread-safe when the same buffer is used in multiple threads.
TEST_P(MultithreadingTestES3, SimultaneousBufferBind)
{
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;

layout(std140) uniform Block
{
    vec4 colorIn;
};

out vec4 color;

void main()
{
    color = colorIn;
})";

    constexpr int kSurfaceWidth  = 32;
    constexpr int kSurfaceHeight = 128;

    GLuint buffer;
    GLsync sync = nullptr;

    // Sync primitives
    std::mutex mutex;
    std::condition_variable condVar;

    enum class Step
    {
        Start,
        Thread0Ready,
        Thread1Ready,
        Finish,
        Abort,
    };
    Step currentStep = Step::Start;

    auto thread0 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

        // Create the buffer in this context
        glGenBuffers(1, &buffer);

        constexpr float kBufferData[4] = {
            10.0f / 255.0f,
            50.0f / 255.0f,
            130.0f / 255.0f,
            220.0f / 255.0f,
        };
        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(kBufferData), kBufferData, GL_STATIC_DRAW);

        sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        // Make sure the two threads start work around the same time
        threadSynchronization.nextStep(Step::Thread0Ready);
        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread1Ready));

        // Bind and unbind the buffer many times.  If ref counting is not thread safe, chances are
        // the ref count would be incorrect in the end.  This can result in the buffer prematurely
        // getting deleted.
        for (uint32_t i = 0; i < 8000; ++i)
        {
            glBindBuffer(GL_UNIFORM_BUFFER, i % 2 == 0 ? 0 : buffer);
        }
        ASSERT_GL_NO_ERROR();

        threadSynchronization.nextStep(Step::Finish);
    };
    auto thread1 = [&](EGLDisplay dpy, EGLSurface surface, EGLContext context) {
        ThreadSynchronization<Step> threadSynchronization(&currentStep, &mutex, &condVar);
        EXPECT_EGL_TRUE(eglMakeCurrent(dpy, surface, surface, context));

        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Thread0Ready));
        threadSynchronization.nextStep(Step::Thread1Ready);

        glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);

        // Bind and unbind the buffer many times.
        for (uint32_t i = 0; i < 4000; ++i)
        {
            glBindBuffer(GL_UNIFORM_BUFFER, i % 2 == 0 ? buffer : 0);
        }

        // Draw with it to make sure buffer is still valid and not accidentally deleted due to bad
        // ref counting.
        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);
        drawQuad(program, essl3_shaders::PositionAttrib(), 0);

        // Verify the results
        const GLColor expect(10, 50, 130, 220);
        EXPECT_PIXEL_RECT_EQ(0, 0, kSurfaceWidth, kSurfaceHeight, expect);
        ASSERT_GL_NO_ERROR();

        ASSERT_TRUE(threadSynchronization.waitForStep(Step::Finish));
    };

    std::array<LockStepThreadFunc, 2> threadFuncs = {
        std::move(thread0),
        std::move(thread1),
    };

    RunLockStepThreadsWithSize(getEGLWindow(), kSurfaceWidth, kSurfaceHeight, threadFuncs.size(),
                               threadFuncs.data());

    ASSERT_NE(currentStep, Step::Abort);
}
ANGLE_INSTANTIATE_TEST(
    MultithreadingTest,
    ES2_METAL(),
    ES3_METAL(),
    ES2_OPENGL(),
    ES3_OPENGL(),
    ES2_OPENGLES(),
    ES3_OPENGLES(),
    ES3_VULKAN(),
    ES3_VULKAN_SWIFTSHADER().disable(Feature::PreferMonolithicPipelinesOverLibraries),
    ES3_VULKAN_SWIFTSHADER().enable(Feature::PreferMonolithicPipelinesOverLibraries),
    ES3_VULKAN_SWIFTSHADER()
        .enable(Feature::PreferMonolithicPipelinesOverLibraries)
        .enable(Feature::SlowDownMonolithicPipelineCreationForTesting),
    ES3_VULKAN_SWIFTSHADER()
        .enable(Feature::PreferMonolithicPipelinesOverLibraries)
        .disable(Feature::MergeProgramPipelineCachesToGlobalCache),
    ES3_VULKAN_SWIFTSHADER().enable(Feature::PermanentlySwitchToFramebufferFetchMode),
    ES3_VULKAN_SWIFTSHADER()
        .enable(Feature::PermanentlySwitchToFramebufferFetchMode)
        .enable(Feature::PreferMonolithicPipelinesOverLibraries),
    ES3_VULKAN_SWIFTSHADER()
        .enable(Feature::PermanentlySwitchToFramebufferFetchMode)
        .enable(Feature::PreferMonolithicPipelinesOverLibraries)
        .enable(Feature::SlowDownMonolithicPipelineCreationForTesting),
    ES2_D3D11(),
    ES3_D3D11());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultithreadingTestES3);
ANGLE_INSTANTIATE_TEST(
    MultithreadingTestES3,
    ES3_OPENGL(),
    ES3_OPENGLES(),
    ES3_VULKAN(),
    ES3_VULKAN_SWIFTSHADER().disable(Feature::PreferMonolithicPipelinesOverLibraries),
    ES3_VULKAN_SWIFTSHADER().enable(Feature::PreferMonolithicPipelinesOverLibraries),
    ES3_VULKAN_SWIFTSHADER()
        .enable(Feature::PreferMonolithicPipelinesOverLibraries)
        .enable(Feature::SlowDownMonolithicPipelineCreationForTesting),
    ES3_VULKAN_SWIFTSHADER()
        .enable(Feature::PreferMonolithicPipelinesOverLibraries)
        .disable(Feature::MergeProgramPipelineCachesToGlobalCache),
    ES3_VULKAN_SWIFTSHADER().enable(Feature::PermanentlySwitchToFramebufferFetchMode),
    ES3_VULKAN_SWIFTSHADER()
        .enable(Feature::PermanentlySwitchToFramebufferFetchMode)
        .enable(Feature::PreferMonolithicPipelinesOverLibraries),
    ES3_VULKAN_SWIFTSHADER()
        .enable(Feature::PermanentlySwitchToFramebufferFetchMode)
        .enable(Feature::PreferMonolithicPipelinesOverLibraries)
        .enable(Feature::SlowDownMonolithicPipelineCreationForTesting),
    ES3_D3D11());

}  // namespace angle
