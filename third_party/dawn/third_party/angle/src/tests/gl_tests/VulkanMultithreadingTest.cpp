//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanMultithreadingTest.cpp : Tests of multithreaded rendering specific to the Vulkan back end.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"

#include <atomic>
#include <mutex>
#include <thread>

#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace angle
{

constexpr char kExtensionName[] = "GL_ANGLE_get_image";
static constexpr int kSize      = 256;

class VulkanMultithreadingTest : public ANGLETest<>
{
  protected:
    VulkanMultithreadingTest()
    {
        setWindowWidth(kSize);
        setWindowHeight(kSize);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        mMaxSetsPerPool = rx::vk::DynamicDescriptorPool::GetMaxSetsPerPoolForTesting();
        mMaxSetsPerPoolMultiplier =
            rx::vk::DynamicDescriptorPool::GetMaxSetsPerPoolMultiplierForTesting();
    }

    void testTearDown() override
    {
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolForTesting(mMaxSetsPerPool);
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolMultiplierForTesting(
            mMaxSetsPerPoolMultiplier);
    }

    static constexpr uint32_t kMaxSetsForTesting           = 1;
    static constexpr uint32_t kMaxSetsMultiplierForTesting = 1;

    void limitMaxSets()
    {
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolForTesting(kMaxSetsForTesting);
        rx::vk::DynamicDescriptorPool::SetMaxSetsPerPoolMultiplierForTesting(
            kMaxSetsMultiplierForTesting);
    }

    void runMultithreadedGLTest(
        std::function<void(EGLSurface surface, size_t threadIndex)> testBody,
        size_t threadCount)
    {
        std::mutex mutex;

        EGLWindow *window = getEGLWindow();
        EGLDisplay dpy    = window->getDisplay();
        EGLConfig config  = window->getConfig();

        constexpr EGLint kPBufferSize = kSize;

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

                    ctx = window->createContext(EGL_NO_CONTEXT, nullptr);
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

  private:
    uint32_t mMaxSetsPerPool;
    uint32_t mMaxSetsPerPoolMultiplier;
};

// Test that multiple threads can draw and readback pixels successfully at the same time with small
// descriptor pools.
TEST_P(VulkanMultithreadingTest, MultiContextDrawSmallDescriptorPools)
{
    // TODO(http://anglebug.com/42265131: Flaky on linux.
    ANGLE_SKIP_TEST_IF(IsLinux());
    ANGLE_SKIP_TEST_IF(!platformSupportsMultithreading());
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    // Must be before program creation to limit the descriptor pool sizes when creating the pipeline
    // layout.
    limitMaxSets();

    auto testBody = [](EGLSurface surface, size_t thread) {
        constexpr size_t kIterationsPerThread = 16;
        constexpr size_t kDrawsPerIteration   = 16;

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);

        GLTexture copyTexture;
        glBindTexture(GL_TEXTURE_2D, copyTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        ASSERT_GL_NO_ERROR();

        GLint colorLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());

        auto quadVertices = GetQuadVertices();

        GLBuffer vertexBuffer;
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 6, quadVertices.data(), GL_STATIC_DRAW);

        GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
        glEnableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Pack pixels tightly.
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        std::vector<GLColor> actualData(kSize * kSize);

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
                ASSERT_GL_NO_ERROR();

                // Perform CopyTexImage2D
                glBindTexture(GL_TEXTURE_2D, copyTexture);
                glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, kSize, kSize, 0);
                ASSERT_GL_NO_ERROR();
            }

            // There's a good chance this test will crash before failing, but if not we'll try and
            // verify the contents of the copied texture.
            // TODO(http://anglebug.com/42263765): Need to re-enable for Linux/Windows.
            if (IsGLExtensionEnabled(kExtensionName) && !(IsLinux() || IsWindows()))
            {
                // Verify glCopyTexImage2D() was successful.
                glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, actualData.data());
                EXPECT_GL_NO_ERROR();
                EXPECT_EQ(color, actualData[0]);
            }
        }
    };
    runMultithreadedGLTest(testBody, 4);
}

ANGLE_INSTANTIATE_TEST(VulkanMultithreadingTest, ES2_VULKAN(), ES3_VULKAN());

}  // namespace angle
