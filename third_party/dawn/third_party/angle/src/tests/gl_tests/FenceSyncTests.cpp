//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class FenceNVTest : public ANGLETest<>
{
  protected:
    FenceNVTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }
};

class FenceSyncTest : public ANGLETest<>
{
  public:
    static constexpr uint32_t kSize = 256;

  protected:
    FenceSyncTest()
    {
        setWindowWidth(kSize);
        setWindowHeight(kSize);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }
};

// FenceNV objects should respond false to glIsFenceNV until they've been set
TEST_P(FenceNVTest, IsFence)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_NV_fence"));

    GLuint fence = 0;
    glGenFencesNV(1, &fence);
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_FALSE(glIsFenceNV(fence));
    EXPECT_GL_NO_ERROR();

    glSetFenceNV(fence, GL_ALL_COMPLETED_NV);
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_TRUE(glIsFenceNV(fence));
    EXPECT_GL_NO_ERROR();
}

// Test error cases for all FenceNV functions
TEST_P(FenceNVTest, Errors)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_NV_fence"));

    EXPECT_GL_TRUE(glTestFenceNV(10)) << "glTestFenceNV should still return TRUE for an invalid "
                                         "fence and generate an INVALID_OPERATION";
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLuint fence = 20;

    // glGenFencesNV should generate INVALID_VALUE for a negative n and not write anything to the
    // fences pointer
    glGenFencesNV(-1, &fence);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    EXPECT_EQ(20u, fence);

    // Generate a real fence
    glGenFencesNV(1, &fence);
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_TRUE(glTestFenceNV(fence)) << "glTestFenceNV should still return TRUE for a fence "
                                            "that is not started and generate an INVALID_OPERATION";
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // glGetFenceivNV should generate an INVALID_OPERATION for an invalid or unstarted fence and not
    // modify the params
    GLint result = 30;
    glGetFenceivNV(10, GL_FENCE_STATUS_NV, &result);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    EXPECT_EQ(30, result);

    glGetFenceivNV(fence, GL_FENCE_STATUS_NV, &result);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    EXPECT_EQ(30, result);

    // glSetFenceNV should generate an error for any condition that is not ALL_COMPLETED_NV
    glSetFenceNV(fence, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // glSetFenceNV should generate INVALID_OPERATION for an invalid fence
    glSetFenceNV(10, GL_ALL_COMPLETED_NV);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that basic usage works and doesn't generate errors or crash
TEST_P(FenceNVTest, BasicOperations)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_NV_fence"));

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    constexpr size_t kFenceCount = 20;
    GLuint fences[kFenceCount]   = {0};
    glGenFencesNV(static_cast<GLsizei>(ArraySize(fences)), fences);
    EXPECT_GL_NO_ERROR();

    for (GLuint fence : fences)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glSetFenceNV(fence, GL_ALL_COMPLETED_NV);
    }

    // Finish the last fence, all fences before should be marked complete
    glFinishFenceNV(fences[kFenceCount - 1]);

    for (GLuint fence : fences)
    {
        GLint status = 0;
        glGetFenceivNV(fence, GL_FENCE_STATUS_NV, &status);
        EXPECT_GL_NO_ERROR();

        // Fence should be complete now that Finish has been called
        EXPECT_GL_TRUE(status);
    }

    EXPECT_PIXEL_EQ(0, 0, 255, 0, 255, 255);
}

// Sync objects should respond true to IsSync after they are created with glFenceSync
TEST_P(FenceSyncTest, IsSync)
{
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_TRUE(glIsSync(sync));
    EXPECT_GL_FALSE(glIsSync(reinterpret_cast<GLsync>(40)));
}

// Test error cases for all Sync function
TEST_P(FenceSyncTest, Errors)
{
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    // DeleteSync generates INVALID_VALUE when the sync is not valid
    glDeleteSync(reinterpret_cast<GLsync>(20));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glFenceSync generates GL_INVALID_ENUM if the condition is not GL_SYNC_GPU_COMMANDS_COMPLETE
    EXPECT_EQ(0, glFenceSync(0, 0));
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // glFenceSync generates GL_INVALID_ENUM if the flags is not 0
    EXPECT_EQ(0, glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 10));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glClientWaitSync generates GL_INVALID_VALUE and returns GL_WAIT_FAILED if flags contains more
    // than just GL_SYNC_FLUSH_COMMANDS_BIT
    EXPECT_GLENUM_EQ(GL_WAIT_FAILED, glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT | 0x2, 0));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glClientWaitSync generates GL_INVALID_VALUE and returns GL_WAIT_FAILED if the sync object is
    // not valid
    EXPECT_GLENUM_EQ(GL_WAIT_FAILED,
                     glClientWaitSync(reinterpret_cast<GLsync>(30), GL_SYNC_FLUSH_COMMANDS_BIT, 0));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glWaitSync generates GL_INVALID_VALUE if flags is non-zero
    glWaitSync(sync, 1, GL_TIMEOUT_IGNORED);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glWaitSync generates GL_INVALID_VALUE if GLuint64 is not GL_TIMEOUT_IGNORED
    glWaitSync(sync, 0, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glWaitSync generates GL_INVALID_VALUE if the sync object is not valid
    glWaitSync(reinterpret_cast<GLsync>(30), 0, GL_TIMEOUT_IGNORED);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glGetSynciv generates GL_INVALID_VALUE if bufSize is less than zero, results should be
    // untouched
    GLsizei length = 20;
    GLint value    = 30;
    glGetSynciv(sync, GL_OBJECT_TYPE, -1, &length, &value);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    EXPECT_EQ(20, length);
    EXPECT_EQ(30, value);

    // glGetSynciv generates GL_INVALID_VALUE if the sync object is not valid, results should be
    // untouched
    glGetSynciv(reinterpret_cast<GLsync>(30), GL_OBJECT_TYPE, 1, &length, &value);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    EXPECT_EQ(20, length);
    EXPECT_EQ(30, value);
}

// Test usage of glGetSynciv
TEST_P(FenceSyncTest, BasicQueries)
{
    GLsizei length = 0;
    GLint value    = 0;
    GLsync sync    = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    glGetSynciv(sync, GL_SYNC_CONDITION, 1, &length, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_SYNC_GPU_COMMANDS_COMPLETE, value);

    glGetSynciv(sync, GL_OBJECT_TYPE, 1, &length, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_SYNC_FENCE, value);

    glGetSynciv(sync, GL_SYNC_FLAGS, 1, &length, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0, value);
}

// Test usage of glGetSynciv with nullptr as length
TEST_P(FenceSyncTest, NullLength)
{
    GLint value = 0;
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glGetSynciv(sync, GL_SYNC_STATUS, 1, nullptr, &value);
    glDeleteSync(sync);
    EXPECT_GL_NO_ERROR();
}

// Test that basic usage works and doesn't generate errors or crash
TEST_P(FenceSyncTest, BasicOperations)
{
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    glClear(GL_COLOR_BUFFER_BIT);
    glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
    EXPECT_GL_NO_ERROR();

    GLsizei length         = 0;
    GLint value            = 0;
    unsigned int loopCount = 0;

    glFlush();

    // Use 'loopCount' to make sure the test doesn't get stuck in an infinite loop
    while (value != GL_SIGNALED && loopCount <= 1000000)
    {
        loopCount++;

        glGetSynciv(sync, GL_SYNC_STATUS, 1, &length, &value);
        ASSERT_GL_NO_ERROR();
    }

    ASSERT_GLENUM_EQ(GL_SIGNALED, value);

    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    for (size_t i = 0; i < 20; i++)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(greenProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
        ASSERT_GL_NO_ERROR();

        GLsync clientWaitSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        ASSERT_GL_NO_ERROR();

        // Don't wait forever to make sure the test terminates
        constexpr GLuint64 kTimeout = 1'000'000'000;  // 1 second
        GLenum clientWaitResult =
            glClientWaitSync(clientWaitSync, GL_SYNC_FLUSH_COMMANDS_BIT, kTimeout);
        EXPECT_GL_NO_ERROR();
        EXPECT_TRUE(clientWaitResult == GL_CONDITION_SATISFIED ||
                    clientWaitResult == GL_ALREADY_SIGNALED);

        glDeleteSync(clientWaitSync);
        ASSERT_GL_NO_ERROR();
    }
}

// Test that multiple fences and draws can be issued
TEST_P(FenceSyncTest, MultipleFenceDraw)
{
    constexpr int kNumIterations = 10;
    constexpr int kNumDraws      = 5;

    // Create a texture/FBO to draw to
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    ASSERT_GL_NO_ERROR();
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    bool drawGreen = true;
    for (int numIterations = 0; numIterations < kNumIterations; ++numIterations)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        ASSERT_GL_NO_ERROR();

        for (int i = 0; i < kNumDraws; ++i)
        {
            GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
            ASSERT_GL_NO_ERROR();

            drawGreen      = !drawGreen;
            GLuint program = 0;
            if (drawGreen)
            {
                program = greenProgram;
            }
            else
            {
                program = redProgram;
            }
            drawQuad(program, std::string(essl1_shaders::PositionAttrib()), 0.0f);
            ASSERT_GL_NO_ERROR();

            glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
            EXPECT_GL_NO_ERROR();
            glDeleteSync(sync);
            ASSERT_GL_NO_ERROR();
        }

        // Blit to the default FBO
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        ASSERT_GL_NO_ERROR();
        swapBuffers();

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        GLColor color;
        if (drawGreen)
        {
            color = GLColor::green;
        }
        else
        {
            color = GLColor::red;
        }
        EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, color);
    }
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(FenceNVTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(FenceSyncTest);
ANGLE_INSTANTIATE_TEST_ES3(FenceSyncTest);
