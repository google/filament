//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Framebuffer tests:
//   Various tests related for Frambuffers.
//

#include "common/mathutil.h"
#include "platform/autogen/FeaturesD3D_autogen.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/OSWindow.h"

using namespace angle;

namespace
{

void ExpectFramebufferCompleteOrUnsupported(GLenum binding)
{
    GLenum status = glCheckFramebufferStatus(binding);
    EXPECT_TRUE(status == GL_FRAMEBUFFER_COMPLETE || status == GL_FRAMEBUFFER_UNSUPPORTED);
}

}  // anonymous namespace

class FramebufferFormatsTest : public ANGLETest<>
{
  protected:
    FramebufferFormatsTest() : mFramebuffer(0), mTexture(0), mRenderbuffer(0), mProgram(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void checkBitCount(GLuint fbo, GLenum channel, GLint minBits)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLint bits = 0;
        glGetIntegerv(channel, &bits);

        if (minBits == 0)
        {
            EXPECT_EQ(minBits, bits);
        }
        else
        {
            EXPECT_GE(bits, minBits);
        }
    }

    void testBitCounts(GLuint fbo,
                       GLint minRedBits,
                       GLint minGreenBits,
                       GLint minBlueBits,
                       GLint minAlphaBits,
                       GLint minDepthBits,
                       GLint minStencilBits)
    {
        checkBitCount(fbo, GL_RED_BITS, minRedBits);
        checkBitCount(fbo, GL_GREEN_BITS, minGreenBits);
        checkBitCount(fbo, GL_BLUE_BITS, minBlueBits);
        checkBitCount(fbo, GL_ALPHA_BITS, minAlphaBits);
        checkBitCount(fbo, GL_DEPTH_BITS, minDepthBits);
        checkBitCount(fbo, GL_STENCIL_BITS, minStencilBits);
    }

    void testTextureFormat(GLenum internalFormat,
                           GLint minRedBits,
                           GLint minGreenBits,
                           GLint minBlueBits,
                           GLint minAlphaBits)
    {
        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);

        if (getClientMajorVersion() >= 3)
        {
            glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, 1, 1);
        }
        else
        {
            glTexStorage2DEXT(GL_TEXTURE_2D, 1, internalFormat, 1, 1);
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);

        testBitCounts(mFramebuffer, minRedBits, minGreenBits, minBlueBits, minAlphaBits, 0, 0);
    }

    void testRenderbufferMultisampleFormat(int minESVersion,
                                           GLenum attachmentType,
                                           GLenum internalFormat)
    {
        int clientVersion = getClientMajorVersion();
        if (clientVersion < minESVersion)
        {
            return;
        }

        // Check that multisample is supported with at least two samples (minimum required is 1)
        bool supports2Samples = false;

        if (clientVersion == 2)
        {
            if (IsGLExtensionEnabled("ANGLE_framebuffer_multisample"))
            {
                int maxSamples;
                glGetIntegerv(GL_MAX_SAMPLES_ANGLE, &maxSamples);
                supports2Samples = maxSamples >= 2;
            }
        }
        else
        {
            assert(clientVersion >= 3);
            int maxSamples;
            glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
            supports2Samples = maxSamples >= 2;
        }

        if (!supports2Samples)
        {
            return;
        }

        glGenRenderbuffers(1, &mRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);

        EXPECT_GL_NO_ERROR();
        glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, 2, internalFormat, 128, 128);
        EXPECT_GL_NO_ERROR();
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, mRenderbuffer);
        EXPECT_GL_NO_ERROR();
    }

    void testZeroHeightRenderbuffer()
    {
        glGenRenderbuffers(1, &mRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  mRenderbuffer);
        EXPECT_GL_NO_ERROR();
    }

    void testSetUp() override
    {
        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    }

    void testTearDown() override
    {
        if (mTexture != 0)
        {
            glDeleteTextures(1, &mTexture);
            mTexture = 0;
        }

        if (mRenderbuffer != 0)
        {
            glDeleteRenderbuffers(1, &mRenderbuffer);
            mRenderbuffer = 0;
        }

        if (mFramebuffer != 0)
        {
            glDeleteFramebuffers(1, &mFramebuffer);
            mFramebuffer = 0;
        }

        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
            mProgram = 0;
        }
    }

    GLuint mFramebuffer;
    GLuint mTexture;
    GLuint mRenderbuffer;
    GLuint mProgram;
};

TEST_P(FramebufferFormatsTest, RGBA4)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_texture_storage"));

    testTextureFormat(GL_RGBA4, 4, 4, 4, 4);
}

TEST_P(FramebufferFormatsTest, RGB565)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_texture_storage"));

    testTextureFormat(GL_RGB565, 5, 6, 5, 0);
}

TEST_P(FramebufferFormatsTest, RGB8)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       (!IsGLExtensionEnabled("GL_OES_rgb8_rgba8") ||
                        !IsGLExtensionEnabled("GL_EXT_texture_storage")));

    testTextureFormat(GL_RGB8_OES, 8, 8, 8, 0);
}

TEST_P(FramebufferFormatsTest, BGRA8)
{
    ANGLE_SKIP_TEST_IF(
        !IsGLExtensionEnabled("GL_EXT_texture_format_BGRA8888") ||
        (getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_EXT_texture_storage")));

    testTextureFormat(GL_BGRA8_EXT, 8, 8, 8, 8);
}

TEST_P(FramebufferFormatsTest, RGBA8)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       (!IsGLExtensionEnabled("GL_OES_rgb8_rgba8") ||
                        !IsGLExtensionEnabled("GL_EXT_texture_storage")));

    testTextureFormat(GL_RGBA8_OES, 8, 8, 8, 8);
}

TEST_P(FramebufferFormatsTest, RenderbufferMultisample_DEPTH16)
{
    testRenderbufferMultisampleFormat(2, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT16);
}

TEST_P(FramebufferFormatsTest, RenderbufferMultisample_DEPTH24)
{
    testRenderbufferMultisampleFormat(3, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT24);
}

TEST_P(FramebufferFormatsTest, RenderbufferMultisample_DEPTH32F)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    testRenderbufferMultisampleFormat(3, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT32F);
}

TEST_P(FramebufferFormatsTest, RenderbufferMultisample_DEPTH24_STENCIL8)
{
    testRenderbufferMultisampleFormat(3, GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);
}

TEST_P(FramebufferFormatsTest, RenderbufferMultisample_DEPTH32F_STENCIL8)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    testRenderbufferMultisampleFormat(3, GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH32F_STENCIL8);
}

TEST_P(FramebufferFormatsTest, RenderbufferMultisample_STENCIL_INDEX8)
{
    // TODO(geofflang): Figure out how to support GLSTENCIL_INDEX8 on desktop GL
    ANGLE_SKIP_TEST_IF(IsDesktopOpenGL());

    testRenderbufferMultisampleFormat(2, GL_STENCIL_ATTACHMENT, GL_STENCIL_INDEX8);
}

// Test that binding an incomplete cube map is rejected by ANGLE.
TEST_P(FramebufferFormatsTest, IncompleteCubeMap)
{
    // http://anglebug.com/42261821
    ANGLE_SKIP_TEST_IF(IsFuchsia() && IsIntel() && IsVulkan());

    // First make a complete CubeMap.
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                           mTexture, 0);

    // Verify the framebuffer is complete.
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Make the CubeMap cube-incomplete.
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    // Verify the framebuffer is incomplete.
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();

    // Verify drawing with the incomplete framebuffer produces a GL error
    mProgram = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ASSERT_NE(0u, mProgram);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
}

// Test that a renderbuffer with zero height but nonzero width is handled without crashes/asserts.
TEST_P(FramebufferFormatsTest, ZeroHeightRenderbuffer)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    testZeroHeightRenderbuffer();
}

// Test to cover a bug where the read framebuffer affects the completeness of the draw framebuffer.
TEST_P(FramebufferFormatsTest, ReadDrawCompleteness)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    GLTexture incompleteTexture;
    glBindTexture(GL_TEXTURE_2D, incompleteTexture);

    GLFramebuffer incompleteFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, incompleteFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, incompleteTexture,
                           0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    GLTexture completeTexture;
    glBindTexture(GL_TEXTURE_2D, completeTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());

    GLFramebuffer completeFBO;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, completeFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           completeTexture, 0);

    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();

    // Simple draw program.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, completeFBO);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that a renderbuffer with RGBA8 format works as expected.
TEST_P(FramebufferFormatsTest, RGBA8Renderbuffer)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_rgba8"));
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 16, 16);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test that a framebuffer can be attached to an RGB8 and an RGBA8 renderbuffer and work.
TEST_P(FramebufferFormatsTest, RGB8AndRGBA8Renderbuffers)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_rgb8_rgba8"));
    GLRenderbuffer rbo1;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo1);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, 16, 16);

    GLRenderbuffer rbo2;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 16, 16);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo1);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 1.0, 1.0, 0.1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo2);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that a renderbuffer with RGB565 format works as expected. This test is intended for some
// back-end having no support for native RGB565 renderbuffer and thus having to emulate using RGBA
// format.
TEST_P(FramebufferFormatsTest, RGB565Renderbuffer)
{
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, 1, 1);
    // Check if GL_RGB565 is valid to render buffer.
    ASSERT_GL_NO_ERROR();

    GLFramebuffer completeFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, completeFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();

    glClearColor(1, 0, 0, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that a renderbuffer with RGBA4 format works as expected.
TEST_P(FramebufferFormatsTest, RGBA4Renderbuffer)
{
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, 1, 1);
    // Check if GL_RGBA4 is valid to render buffer.
    ASSERT_GL_NO_ERROR();

    GLFramebuffer completeFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, completeFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();

    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(255u, 0u, 0u, 255u));
}

// Test that a renderbuffer with RGB5_A1 format works as expected.
TEST_P(FramebufferFormatsTest, RGB5A1Renderbuffer)
{
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB5_A1, 1, 1);
    // Check if GL_RGB5_A1 is valid to render buffer.
    ASSERT_GL_NO_ERROR();

    GLFramebuffer completeFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, completeFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();

    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(255u, 0u, 0u, 255u));
}

// Test that a renderbuffer with RGB8 format works as expected.
TEST_P(FramebufferFormatsTest, RGB8Renderbuffer)
{
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, 1, 1);
    // Check if GL_RGB8 is valid to render buffer.
    ASSERT_GL_NO_ERROR();

    GLFramebuffer completeFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, completeFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();

    glClearColor(1, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

class FramebufferTest_ES3 : public ANGLETest<>
{
  protected:
    FramebufferTest_ES3()
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    static constexpr GLsizei kWidth  = 64;
    static constexpr GLsizei kHeight = 256;
};

// Covers invalidating an incomplete framebuffer. This should be a no-op, but should not error.
TEST_P(FramebufferTest_ES3, InvalidateIncomplete)
{
    GLFramebuffer framebuffer;
    GLRenderbuffer renderbuffer;

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    std::vector<GLenum> attachments;
    attachments.push_back(GL_COLOR_ATTACHMENT0);

    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments.data());
    EXPECT_GL_NO_ERROR();
}

// Covers sub-invalidating an incomplete framebuffer. This should be a no-op, but should not error.
TEST_P(FramebufferTest_ES3, SubInvalidateIncomplete)
{
    GLFramebuffer framebuffer;
    GLRenderbuffer renderbuffer;

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    std::vector<GLenum> attachments;
    attachments.push_back(GL_COLOR_ATTACHMENT0);

    glInvalidateSubFramebuffer(GL_FRAMEBUFFER, 1, attachments.data(), 5, 5, 10, 10);
    EXPECT_GL_NO_ERROR();
}

enum class DisableDitherVsClear
{
    Before,
    After
};

void testDitherDisabledProperlyOnRGB565(GLColor gradientColor,
                                        DisableDitherVsClear disableDitherVsClear)
{
    GLFramebuffer framebuffer;

    constexpr GLsizei kFramebufferWidth  = 4;
    constexpr GLsizei kFramebufferHeight = 4;
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB565, kFramebufferWidth, kFramebufferHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    constexpr char kVS[] = {
        R"(#version 300 es
        in highp vec4 a_position;
        in mediump vec4 a_color;
        out mediump vec4 v_color;
        void main()
        {
        gl_Position = a_position;
        v_color = a_color;
        })",
    };

    constexpr char kFS[] = {
        R"(#version 300 es
            in mediump vec4 v_color;
            layout(location = 0) out mediump vec4 o_color;
            void main()
            {
                o_color = v_color;
            })",
    };

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glUseProgram(program);

    // setup quad data
    // black ----> gradientColor
    // **********
    // *        *
    // *        *
    // **********
    const std::vector<float> positions = {-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
                                          1.0f,  -1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 0.0f, 1.0f};

    const std::vector<float> color0 = {0.0f,
                                       0.0f,
                                       0.0f,
                                       0.0f,
                                       0.0f,
                                       0.0f,
                                       0.0f,
                                       0.0f,
                                       gradientColor.R * 1.0f / 255.0f,
                                       gradientColor.G * 1.0f / 255.0f,
                                       gradientColor.B * 1.0f / 255.0f,
                                       gradientColor.A * 1.0f / 255.0f,
                                       gradientColor.R * 1.0f / 255.0f,
                                       gradientColor.G * 1.0f / 255.0f,
                                       gradientColor.B * 1.0f / 255.0f,
                                       gradientColor.A * 1.0f / 255.0f};

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), positions.data(),
                 GL_STATIC_DRAW);

    GLBuffer colorBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color0[0]) * color0.size(), color0.data(), GL_STATIC_DRAW);

    GLint vertexPosLocation = glGetAttribLocation(program, "a_position");
    ASSERT_NE(vertexPosLocation, -1);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(vertexPosLocation);
    glVertexAttribPointer(vertexPosLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);

    GLint vertexColorLocation = glGetAttribLocation(program, "a_color");
    ASSERT_NE(vertexColorLocation, -1);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glEnableVertexAttribArray(vertexColorLocation);
    glVertexAttribPointer(vertexColorLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);

    const std::vector<uint8_t> indices = {0, 2, 1, 1, 2, 3};

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(),
                 GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    switch (disableDitherVsClear)
    {
        case DisableDitherVsClear::Before:
            glDisable(GL_DITHER);
            glClearColor(0.125, 0.25, 0.5, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            break;

        case DisableDitherVsClear::After:
            glClearColor(0.125, 0.25, 0.5, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DITHER);
            break;
    }

    // draw quad
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // validate that when disabling dithering, the color selection must be coordinate-independent
    std::vector<GLColor> pixelData(kFramebufferWidth * kFramebufferHeight);
    glReadPixels(0, 0, kFramebufferWidth, kFramebufferHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixelData.data());

    const int increasingDirectionSize = kFramebufferWidth;
    const int constantDirectionSize   = kFramebufferHeight;

    for (int incrPos = 0; incrPos < increasingDirectionSize; incrPos++)
    {
        bool colorHasChanged = false;

        GLColor prevConstantDirectionPixel;

        for (int constPos = 0; constPos < constantDirectionSize; constPos++)
        {
            const int x = incrPos;
            const int y = constPos;

            const int currentPixelLoc  = y * kFramebufferWidth + x;
            const GLColor currentPixel = pixelData[currentPixelLoc];

            if (constPos > 0 && currentPixel != prevConstantDirectionPixel)
            {
                if (colorHasChanged)
                {
                    ASSERT(false);
                }
                else
                {
                    colorHasChanged = true;
                }
            }

            prevConstantDirectionPixel = currentPixel;
        }
    }
}

// repro dEQP-GLES3.functional.dither.disabled.gradient_red failure
TEST_P(FramebufferTest_ES3, RGB565DisableDitheringGradientRedTest)
{
    testDitherDisabledProperlyOnRGB565(GLColor::red, DisableDitherVsClear::Before);
    testDitherDisabledProperlyOnRGB565(GLColor::red, DisableDitherVsClear::After);
}

// repro dEQP-GLES3.functional.dither.disabled.gradient_green failure
TEST_P(FramebufferTest_ES3, RGB565DisableDitheringGradientGreenTest)
{
    testDitherDisabledProperlyOnRGB565(GLColor::green, DisableDitherVsClear::Before);
    testDitherDisabledProperlyOnRGB565(GLColor::green, DisableDitherVsClear::After);
}

// repro dEQP-GLES3.functional.dither.disabled.gradient_blue failure
TEST_P(FramebufferTest_ES3, RGB565DisableDitheringGradientBlueTest)
{
    testDitherDisabledProperlyOnRGB565(GLColor::blue, DisableDitherVsClear::Before);
    testDitherDisabledProperlyOnRGB565(GLColor::blue, DisableDitherVsClear::After);
}

// repro dEQP-GLES3.functional.dither.disabled.gradient_white failure
TEST_P(FramebufferTest_ES3, RGB565DisableDitheringGradientWhiteTest)
{
    testDitherDisabledProperlyOnRGB565(GLColor::white, DisableDitherVsClear::Before);
    testDitherDisabledProperlyOnRGB565(GLColor::white, DisableDitherVsClear::After);
}

// Test that subinvalidate with no prior command works.  Regression test for the Vulkan backend that
// assumed a render pass is started when sub invalidate is called.
TEST_P(FramebufferTest_ES3, SubInvalidateFirst)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Invalidate half of the framebuffer using swapped dimensions.
    std::array<GLenum, 1> attachments = {GL_COLOR};
    glInvalidateSubFramebuffer(GL_DRAW_FRAMEBUFFER, 1, attachments.data(), 0, 0, kHeight, kWidth);
    EXPECT_GL_NO_ERROR();
}

// Test that subinvalidate doesn't discard data outside area.  Uses swapped width/height for
// invalidate which results in a partial invalidate, but also prevents bugs with Vulkan
// pre-rotation.
TEST_P(FramebufferTest_ES3, SubInvalidatePartial)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear the attachment.
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Invalidate half of the framebuffer using swapped dimensions.
    std::array<GLenum, 1> attachments = {GL_COLOR};
    glInvalidateSubFramebuffer(GL_DRAW_FRAMEBUFFER, 1, attachments.data(), 0, 0, kHeight, kWidth);
    EXPECT_GL_NO_ERROR();

    // Make sure the other half is correct.
    EXPECT_PIXEL_COLOR_EQ(0, kWidth, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kWidth, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::red);
}

// Test that invalidating depth/stencil of the default framebuffer doesn't crash.
TEST_P(FramebufferTest_ES3, InvalidateDefaultFramebufferDepthStencil)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::array<GLenum, 2> attachments = {GL_DEPTH, GL_STENCIL};

    // Invalidate default framebuffer depth/stencil attachments
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments.data());
    EXPECT_GL_NO_ERROR();
}

// Test that invalidating color of the default framebuffer doesn't crash.
TEST_P(FramebufferTest_ES3, InvalidateDefaultFramebufferColor)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::array<GLenum, 1> attachments = {GL_COLOR};

    // Invalidate default framebuffer color attachment.
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments.data());
    EXPECT_GL_NO_ERROR();
}

// Test that invalidating all attachments on the default framebuffer doesn't crash.
TEST_P(FramebufferTest_ES3, InvalidateDefaultFramebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::array<GLenum, 3> attachments = {GL_COLOR, GL_DEPTH, GL_STENCIL};

    // Invalidate all default framebuffer attachments.
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 3, attachments.data());
    EXPECT_GL_NO_ERROR();
}

// Test that invalidating combined depth/stencil attachment doesn't crash.
TEST_P(FramebufferTest_ES3, InvalidateDepthStencil)
{
    // Create the framebuffer that will be invalidated
    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 2, 2);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthStencil);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    std::array<GLenum, 2> attachments = {GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT};

    // Invalidate both depth and stencil.
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments.data());
    EXPECT_GL_NO_ERROR();
}

// Test that invalidating an FB with a stencil attachment, which
// generates Validation errors, will not fail
TEST_P(FramebufferTest_ES3, InvalidateStencilAttachmentOnly)
{
    constexpr GLsizei kBufferSize = 4096;
    std::vector<uint8_t> abdata(kBufferSize);
    GLBuffer arrayBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, kBufferSize, abdata.data(), GL_STATIC_DRAW);

    std::vector<uint8_t> eabdata(kBufferSize);
    GLBuffer elementArrayBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, elementArrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, kBufferSize, eabdata.data(), GL_STATIC_DRAW);

    GLTexture stencilTexture;
    glBindTexture(GL_TEXTURE_2D, stencilTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, 32, 32);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glVertexAttribPointer(0, 4, GL_SHORT, GL_TRUE, 8, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    GLVertexArray vertexArray;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
    glDrawElements(GL_TRIANGLE_STRIP, 26, GL_UNSIGNED_INT, nullptr);
    EXPECT_GL_NO_ERROR();

    std::array<GLenum, 1> attachments = {GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments.data());
    EXPECT_GL_NO_ERROR();
}

// Test that invalidating stencil-only attachment doesn't crash.
TEST_P(FramebufferTest_ES3, InvalidateStencilOnly)
{
    // Create the framebuffer that will be invalidated
    GLRenderbuffer stencil;
    glBindRenderbuffer(GL_RENDERBUFFER, stencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, 2, 2);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    std::array<GLenum, 1> attachments = {GL_STENCIL_ATTACHMENT};

    // Invalidate both depth and stencil.
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments.data());
    EXPECT_GL_NO_ERROR();
}

// Test that invalidating stencil of a depth-only attachment doesn't crash.
TEST_P(FramebufferTest_ES3, DepthOnlyAttachmentInvalidateStencil)
{
    // Create the framebuffer that will be invalidated
    GLRenderbuffer depth;
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 2, 2);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Invalidate stencil only.
    std::array<GLenum, 2> attachments = {GL_STENCIL_ATTACHMENT, GL_DEPTH_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments.data());
    EXPECT_GL_NO_ERROR();

    // Invalidate both depth and stencil.
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments.data());
    EXPECT_GL_NO_ERROR();
}

// Test that invalidating depth of a stencil-only attachment doesn't crash.
TEST_P(FramebufferTest_ES3, StencilOnlyAttachmentInvalidateDepth)
{
    // Create the framebuffer that will be invalidated
    GLRenderbuffer depth;
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, 2, 2);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_GL_NO_ERROR();

    // Invalidate depth only.
    std::array<GLenum, 2> attachments = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments.data());
    EXPECT_GL_NO_ERROR();

    // Invalidate both depth and stencil.
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, attachments.data());
    EXPECT_GL_NO_ERROR();
}

// Test that a scissored draw followed by subinvalidate followed by a non-scissored draw retains the
// part that is not invalidated.  Uses swapped width/height for invalidate which results in a
// partial invalidate, but also prevents bugs with Vulkan pre-rotation.
TEST_P(FramebufferTest_ES3, ScissoredDrawSubInvalidateThenNonScissoredDraw)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Clear color to red and the depth/stencil buffer to 1.0 and 0x55
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Break rendering so the following draw call starts rendering with a scissored area.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Issue a scissored draw call that changes depth to 0.5 and stencil 0x3C
    glScissor(0, 0, kHeight, kWidth);
    glEnable(GL_SCISSOR_TEST);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x3C, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0);

    // Invalidate the draw region (half of the framebuffer using swapped dimensions).
    std::array<GLenum, 3> attachments = {GL_COLOR, GL_DEPTH, GL_STENCIL};
    glInvalidateSubFramebuffer(GL_DRAW_FRAMEBUFFER, 3, attachments.data(), 0, 0, kHeight, kWidth);
    EXPECT_GL_NO_ERROR();

    // Match the scissor to the framebuffer size and issue a draw call that blends blue, and expects
    // depth to be 1 and stencil to be 0x55.  This is only valid for the half that was not
    // invalidated.
    glScissor(0, 0, kWidth, kHeight);
    glDepthFunc(GL_LESS);
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.95f);
    ASSERT_GL_NO_ERROR();

    // Make sure the half that was not invalidated is correct.
    EXPECT_PIXEL_COLOR_EQ(0, kWidth, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kWidth, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::magenta);
}

// Test that the framebuffer state tracking robustly handles a depth-only attachment being set
// as a depth-stencil attachment. It is equivalent to detaching the depth-stencil attachment.
TEST_P(FramebufferTest_ES3, DepthOnlyAsDepthStencil)
{
    GLFramebuffer framebuffer;
    GLRenderbuffer renderbuffer;

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 4, 4);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffer);
    EXPECT_GLENUM_NE(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
}

// Test that the framebuffer correctly returns that it is not complete if invalid texture mip levels
// are bound
TEST_P(FramebufferTest_ES3, TextureAttachmentMipLevels)
{
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    // Create a complete mip chain in mips 1 to 3
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Create another complete mip chain in mips 4 to 5
    glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 5, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Create a non-complete mip chain in mip 6
    glTexImage2D(GL_TEXTURE_2D, 6, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Incomplete, mipLevel != baseLevel and texture is not mip complete
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 1);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Complete, mipLevel == baseLevel
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    ExpectFramebufferCompleteOrUnsupported(GL_FRAMEBUFFER);

    // Complete, mipLevel != baseLevel but texture is now mip complete
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 2);
    ExpectFramebufferCompleteOrUnsupported(GL_FRAMEBUFFER);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 3);
    ExpectFramebufferCompleteOrUnsupported(GL_FRAMEBUFFER);

    // Incomplete, attached level below the base level
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 1);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Incomplete, attached level is beyond effective max level
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 4);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Complete, mipLevel == baseLevel
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 4);
    ExpectFramebufferCompleteOrUnsupported(GL_FRAMEBUFFER);

    // Complete, mipLevel != baseLevel but texture is now mip complete
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 5);
    ExpectFramebufferCompleteOrUnsupported(GL_FRAMEBUFFER);

    // Complete, mipLevel == baseLevel
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 6);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 6);
    ExpectFramebufferCompleteOrUnsupported(GL_FRAMEBUFFER);
}

TEST_P(FramebufferTest_ES3, TextureAttachmentMipLevelsReadBack)
{
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    const std::array<GLColor, 4 * 4> mip0Data = {
        GLColor::red, GLColor::red, GLColor::red, GLColor::red, GLColor::red, GLColor::red,
        GLColor::red, GLColor::red, GLColor::red, GLColor::red, GLColor::red, GLColor::red,
        GLColor::red, GLColor::red, GLColor::red, GLColor::red};
    const std::array<GLColor, 2 * 2> mip1Data = {GLColor::green, GLColor::green, GLColor::green,
                                                 GLColor::green};

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip0Data.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip1Data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 1);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearColor(0, 0, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// TextureAttachmentMipLevelsReadBackWithDraw is a copy of TextureAttachmentMipLevelsReadBack except
// for adding a draw after the last clear. The draw forces ANGLE's Vulkan backend to use the
// framebuffer that is level 1 of the texture which will trigger the mismatch use of the GL level
// and Vulkan level in referring to that rendertarget.
TEST_P(FramebufferTest_ES3, TextureAttachmentMipLevelsReadBackWithDraw)
{
    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    const std::array<GLColor, 4 * 4> mip0Data = {
        GLColor::red, GLColor::red, GLColor::red, GLColor::red, GLColor::red, GLColor::red,
        GLColor::red, GLColor::red, GLColor::red, GLColor::red, GLColor::red, GLColor::red,
        GLColor::red, GLColor::red, GLColor::red, GLColor::red};
    const std::array<GLColor, 2 * 2> mip1Data = {GLColor::green, GLColor::green, GLColor::green,
                                                 GLColor::green};

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip0Data.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip1Data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 1);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearColor(0, 0, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // This draw triggers the use of the framebuffer
    glUseProgram(greenProgram);
    drawQuad(greenProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that passing an attachment COLOR_ATTACHMENTm where m is equal to MAX_COLOR_ATTACHMENTS
// generates an INVALID_OPERATION.
// OpenGL ES Version 3.0.5 (November 3, 2016), 4.4.2.4 Attaching Texture Images to a Framebuffer, p.
// 208
TEST_P(FramebufferTest_ES3, ColorAttachmentIndexOutOfBounds)
{
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLint maxColorAttachments = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
    GLenum attachment = static_cast<GLenum>(maxColorAttachments + GL_COLOR_ATTACHMENT0);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1, 1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Check that depth-only attachments report the correct number of samples.
TEST_P(FramebufferTest_ES3, MultisampleDepthOnly)
{
    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_DEPTH_COMPONENT24, 32, 32);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    GLint samples = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    EXPECT_GL_NO_ERROR();
    EXPECT_GE(samples, 2);
}

// Check that we only compare width and height of attachments, not depth.
TEST_P(FramebufferTest_ES3, AttachmentWith3DLayers)
{
    GLTexture texA;
    glBindTexture(GL_TEXTURE_2D, texA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLTexture texB;
    glBindTexture(GL_TEXTURE_3D, texB);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 4, 4, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texA, 0);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, texB, 0, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();
}

// Check that invalid layer is detected in framebuffer completeness check.
TEST_P(FramebufferTest_ES3, 3DAttachmentInvalidLayer)
{
    GLTexture tex;
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 4, 4, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 2);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 1);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();
}

// Check that invalid layer is detected in framebuffer completeness check.
TEST_P(FramebufferTest_ES3, 2DArrayInvalidLayer)
{
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 4, 4, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 2);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 1);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();
}

// Test that clearing the stencil buffer when the framebuffer only has a color attachment does not
// crash.
TEST_P(FramebufferTest_ES3, ClearNonexistentStencil)
{
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    GLint clearValue = 0;
    glClearBufferiv(GL_STENCIL, 0, &clearValue);

    // There's no error specified for clearing nonexistent buffers, it's simply a no-op.
    EXPECT_GL_NO_ERROR();
}

// Test that clearing the depth buffer when the framebuffer only has a color attachment does not
// crash.
TEST_P(FramebufferTest_ES3, ClearNonexistentDepth)
{
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    GLfloat clearValue = 0.0f;
    glClearBufferfv(GL_DEPTH, 0, &clearValue);

    // There's no error specified for clearing nonexistent buffers, it's simply a no-op.
    EXPECT_GL_NO_ERROR();
}

// Test that clearing a nonexistent color attachment does not crash.
TEST_P(FramebufferTest_ES3, ClearNonexistentColor)
{
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    std::vector<GLfloat> clearValue = {{0.0f, 1.0f, 0.0f, 1.0f}};
    glClearBufferfv(GL_COLOR, 1, clearValue.data());

    // There's no error specified for clearing nonexistent buffers, it's simply a no-op.
    EXPECT_GL_NO_ERROR();
}

// Test that clearing the depth and stencil buffers when the framebuffer only has a color attachment
// does not crash.
TEST_P(FramebufferTest_ES3, ClearNonexistentDepthStencil)
{
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.0f, 0);

    // There's no error specified for clearing nonexistent buffers, it's simply a no-op.
    EXPECT_GL_NO_ERROR();
}

// Test that clearing a color attachment that has been deleted doesn't crash.
TEST_P(FramebufferTest_ES3, ClearDeletedAttachment)
{
    // An INVALID_FRAMEBUFFER_OPERATION error was seen in this test on Mac, not sure where it might
    // be originating from. http://anglebug.com/42261536
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // There used to be a bug where some draw buffer state used to remain set even after the
    // attachment was detached via deletion. That's why we create, attach and delete this RBO here.
    GLuint rbo = 0u;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    glDeleteRenderbuffers(1, &rbo);

    // There needs to be at least one color attachment to prevent early out from the clear calls.
    GLRenderbuffer rbo2;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, rbo2);

    ASSERT_GL_NO_ERROR();

    // There's no error specified for clearing nonexistent buffers, it's simply a no-op, so we
    // expect no GL errors below.
    std::array<GLfloat, 4> floatClearValue = {0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 0, floatClearValue.data());
    EXPECT_GL_NO_ERROR();
    std::array<GLuint, 4> uintClearValue = {0u, 0u, 0u, 0u};
    glClearBufferuiv(GL_COLOR, 0, uintClearValue.data());
    EXPECT_GL_NO_ERROR();
    std::array<GLint, 4> intClearValue = {0, 0, 0, 0};
    glClearBufferiv(GL_COLOR, 0, intClearValue.data());
    EXPECT_GL_NO_ERROR();
}

// Test that clearing a color attachment with bound program that has
// incompatible program output doesn't crash.
TEST_P(FramebufferTest_ES3, ClearIncompatibleAttachments)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;

layout(location = 0) out uvec4 color0;
layout(location = 1) out vec4 color1;

void main()
{
    color0 = uvec4(0, 255, 0, 255);
    color1 = vec4(0, 1, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

    GLRenderbuffer rbo[2];
    glBindRenderbuffer(GL_RENDERBUFFER, rbo[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo[0]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, rbo[1]);

    GLenum allBufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, allBufs);

    EXPECT_GL_NO_ERROR();

    // Draw with simple program.
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();
}

// Test that resizing the color attachment is handled correctly.
TEST_P(FramebufferTest_ES3, ResizeColorAttachmentSmallToLarge)
{
    GLFramebuffer fbo;
    GLTexture smallTexture;
    GLTexture largeTexture;

    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Bind the small texture
    glBindTexture(GL_TEXTURE_2D, smallTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth() / 2, getWindowHeight() / 2, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smallTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to FBO backed by the small texture
    glUseProgram(greenProgram);
    drawQuad(greenProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ((getWindowWidth() / 2) - 1, (getWindowHeight() / 2) - 1, GLColor::green);

    // Change the attachment to the larger texture that fills the window
    glBindTexture(GL_TEXTURE_2D, largeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, largeTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to FBO backed by the large texture
    glUseProgram(blueProgram);
    drawQuad(blueProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::blue);
}

// Test that resizing the color attachment is handled correctly.
TEST_P(FramebufferTest_ES3, ResizeColorAttachmentLargeToSmall)
{
    GLFramebuffer fbo;
    GLTexture smallTexture;
    GLTexture largeTexture;

    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Bind the large texture
    glBindTexture(GL_TEXTURE_2D, largeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, largeTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to FBO backed by the large texture
    glUseProgram(blueProgram);
    drawQuad(blueProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::blue);

    // Change the attachment to the smaller texture
    glBindTexture(GL_TEXTURE_2D, smallTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth() / 2, getWindowHeight() / 2, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smallTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to FBO backed by the small texture
    glUseProgram(greenProgram);
    drawQuad(greenProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ((getWindowWidth() / 2) - 1, (getWindowHeight() / 2) - 1, GLColor::green);
}

// Test that resizing the texture is handled correctly.
TEST_P(FramebufferTest_ES3, ResizeTextureLargeToSmall)
{
    GLFramebuffer fbo;
    GLTexture texture;

    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Allocate a large texture
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to FBO backed by the large texture
    glUseProgram(blueProgram);
    drawQuad(blueProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::blue);

    // Shrink the texture
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth() / 2, getWindowHeight() / 2, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to FBO backed by the small texture
    glUseProgram(greenProgram);
    drawQuad(greenProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ((getWindowWidth() / 2) - 1, (getWindowHeight() / 2) - 1, GLColor::green);
}

// Test that resizing the texture is handled correctly.
TEST_P(FramebufferTest_ES3, ResizeTextureSmallToLarge)
{
    GLFramebuffer fbo;
    GLTexture texture;

    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Allocate a small texture
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth() / 2, getWindowHeight() / 2, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to FBO backed by the large texture
    glUseProgram(blueProgram);
    drawQuad(blueProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ((getWindowWidth() / 2) - 1, (getWindowHeight() / 2) - 1, GLColor::blue);

    // Grow the texture
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to FBO backed by the small texture
    glUseProgram(greenProgram);
    drawQuad(greenProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green);
}

// Test that fewer outputs than framebuffer attachments doesn't crash.  This causes a Vulkan
// validation warning, but should not be fatal.
TEST_P(FramebufferTest_ES3, FewerShaderOutputsThanAttachments)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;

layout(location = 0) out vec4 color0;
layout(location = 1) out vec4 color1;
layout(location = 2) out vec4 color2;

void main()
{
    color0 = vec4(1.0, 0.0, 0.0, 1.0);
    color1 = vec4(0.0, 1.0, 0.0, 1.0);
    color2 = vec4(0.0, 0.0, 1.0, 1.0);
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    constexpr GLint kDrawBufferCount = 4;

    GLint maxDrawBuffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    ASSERT_GE(maxDrawBuffers, kDrawBufferCount);

    GLTexture textures[kDrawBufferCount];

    for (GLint texIndex = 0; texIndex < kDrawBufferCount; ++texIndex)
    {
        glBindTexture(GL_TEXTURE_2D, textures[texIndex]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
    }

    GLenum allBufs[kDrawBufferCount] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                        GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};

    GLFramebuffer fbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    // Enable all draw buffers.
    for (GLint texIndex = 0; texIndex < kDrawBufferCount; ++texIndex)
    {
        glBindTexture(GL_TEXTURE_2D, textures[texIndex]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + texIndex, GL_TEXTURE_2D,
                               textures[texIndex], 0);
    }
    glDrawBuffers(kDrawBufferCount, allBufs);

    // Draw with simple program.
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
}

// Test that GL_RGB9_E5 is renderable with the extension.
TEST_P(FramebufferTest_ES3, RenderSharedExponent)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_QCOM_render_shared_exponent"));

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    const uint32_t data = 0x80000100;  // Red
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB9_E5, 1, 1, 0, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, &data);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer readFbo;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR32F_EQ(0, 0, kFloatRed);

    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB9_E5, 1, 1);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer drawFbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    ASSERT_GL_NO_ERROR();

    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFbo);
    EXPECT_PIXEL_COLOR32F_EQ(0, 0, kFloatGreen);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFbo);
    EXPECT_PIXEL_COLOR32F_EQ(0, 0, kFloatRed);
}

// Test color write masks with GL_RGB9_E5 color buffers.
TEST_P(FramebufferTest_ES3, RenderSharedExponentWithMask)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_QCOM_render_shared_exponent"));

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
layout(location = 0) out vec4 color0;
layout(location = 1) out vec4 color1;
void main()
{
    color0 = vec4(1.0, 0.0, 0.0, 1.0);
    color1 = vec4(0.0, 1.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer rb0;
    glBindRenderbuffer(GL_RENDERBUFFER, rb0);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB9_E5, 4, 4);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb0);

    GLRenderbuffer rb1;
    glBindRenderbuffer(GL_RENDERBUFFER, rb1);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 4, 4);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, rb1);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    for (int mask = 0; mask < 16; mask++)
    {
        glColorMask(mask & 1, mask & 2, mask & 4, mask & 8);
        for (const bool enableSharedExponentAttachment : {false, true})
        {
            GLenum bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
            bufs[0]        = enableSharedExponentAttachment ? GL_COLOR_ATTACHMENT0 : GL_NONE;
            glDrawBuffers(2, bufs);

            auto expectError = [](bool enabled, int mask) {
                if (!enabled || mask == 0 || mask == 8 || mask == 7 || mask == 15)
                {
                    EXPECT_GL_NO_ERROR();
                }
                else
                {
                    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
                }
            };

            drawQuad(program, essl3_shaders::PositionAttrib(), 0.0f);
            expectError(enableSharedExponentAttachment, mask);

            glClear(GL_COLOR_BUFFER_BIT);
            expectError(enableSharedExponentAttachment, mask);

            GLfloat clearValuef[4] = {};
            glClearBufferfv(GL_COLOR, 0, clearValuef);
            expectError(enableSharedExponentAttachment, mask);
            glClearBufferfv(GL_COLOR, 1, clearValuef);
            EXPECT_GL_NO_ERROR();
        }
    }
}

// Test that R8_SNORM, RG8_SNORM, and RGBA8_SNORM are renderable with the extension.
TEST_P(FramebufferTest_ES3, RenderSnorm8)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_render_snorm"));

    auto test = [&](GLenum format) {
        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, format, 4, 4);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        ASSERT_GL_NO_ERROR();

        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);
        GLint colorLocation = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
        glUniform4f(colorLocation, -1.0f, -0.5f, -0.25f, -0.125f);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        if (format == GL_R8_SNORM)
        {
            EXPECT_PIXEL_8S_NEAR(0, 0, -127, 0, 0, 127, 2);
        }
        else if (format == GL_RG8_SNORM)
        {
            EXPECT_PIXEL_8S_NEAR(0, 0, -127, -64, 0, 127, 2);
        }
        else if (format == GL_RGBA8_SNORM)
        {
            EXPECT_PIXEL_8S_NEAR(0, 0, -127, -64, -32, -16, 2);
        }
    };

    test(GL_R8_SNORM);
    test(GL_RG8_SNORM);
    test(GL_RGBA8_SNORM);
}

// Test that non-trivial, e.g., reversed, blits are supported for signed normalized formats.
TEST_P(FramebufferTest_ES3, BlitReversedSnorm8)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_render_snorm"));

    auto test = [&](GLenum format) {
        GLRenderbuffer rbo1;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo1);
        glRenderbufferStorage(GL_RENDERBUFFER, format, 4, 4);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer fbo1;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo1);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo1);
        ASSERT_GL_NO_ERROR();

        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);
        GLint colorLocation = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
        glUniform4f(colorLocation, -1.0f, -0.5f, -0.25f, -0.125f);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo1);

        GLRenderbuffer rbo2;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo2);
        glRenderbufferStorage(GL_RENDERBUFFER, format, 4, 4);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer fbo2;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo2);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo2);
        ASSERT_GL_NO_ERROR();

        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

        glBlitFramebuffer(0, 0, 4, 4, 4, 4, 0, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo2);

        if (format == GL_R8_SNORM)
        {
            EXPECT_PIXEL_8S_NEAR(0, 0, -127, 0, 0, 127, 2);
        }
        else if (format == GL_RG8_SNORM)
        {
            EXPECT_PIXEL_8S_NEAR(0, 0, -127, -64, 0, 127, 2);
        }
        else if (format == GL_RGBA8_SNORM)
        {
            EXPECT_PIXEL_8S_NEAR(0, 0, -127, -64, -32, -16, 2);
        }
    };

    test(GL_R8_SNORM);
    test(GL_RG8_SNORM);
    test(GL_RGBA8_SNORM);
}

// Test that R16_SNORM, RG16_SNORM, and RGBA16_SNORM are renderable with the extension.
TEST_P(FramebufferTest_ES3, RenderSnorm16)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_render_snorm"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));

    auto test = [&](GLenum format) {
        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, format, 4, 4);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        ASSERT_GL_NO_ERROR();

        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(program);
        GLint colorLocation = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
        glUniform4f(colorLocation, -1.0f, -0.5f, -0.25f, -0.125f);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        if (format == GL_R16_SNORM_EXT)
        {
            EXPECT_PIXEL_16S_NEAR(0, 0, -32767, 0, 0, 32767, 2);
        }
        else if (format == GL_RG16_SNORM_EXT)
        {
            EXPECT_PIXEL_16S_NEAR(0, 0, -32767, -16383, 0, 32767, 2);
        }
        else if (format == GL_RGBA16_SNORM_EXT)
        {
            EXPECT_PIXEL_16S_NEAR(0, 0, -32767, -16383, -8191, -4095, 2);
        }
    };

    test(GL_R16_SNORM_EXT);
    test(GL_RG16_SNORM_EXT);
    test(GL_RGBA16_SNORM_EXT);
}

class FramebufferTest_ES3Metal : public FramebufferTest_ES3
{};

// Metal, iOS has a limit of the number of bits that can be output
// to color attachments. Test we're enforcing that limit.
TEST_P(FramebufferTest_ES3Metal, TooManyBitsGeneratesFramebufferUnsupported)
{
    ANGLE_SKIP_TEST_IF(
        !getEGLWindow()->isFeatureEnabled(Feature::LimitMaxColorTargetBitsForTesting));

    GLint maxDrawBuffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);

    GLFramebuffer framebuffer;

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Test maxDrawBuffers * RGBA8UI works.
    {
        std::vector<GLTexture> textures(maxDrawBuffers);
        for (GLint i = 0; i < maxDrawBuffers; ++i)
        {
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8UI, 1, 1);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                                   textures[i], 0);
        }
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

    // Test maxDrawBuffers * RGBA32UI does not work.
    {
        std::vector<GLTexture> textures(maxDrawBuffers);
        for (GLint i = 0; i < maxDrawBuffers; ++i)
        {
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32UI, 1, 1);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                                   textures[i], 0);
        }
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_UNSUPPORTED, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
}

// Metal, iOS has a limit of the number of bits that can be output
// to color attachments. Test we're enforcing that limit.
// This test is separate from the one above as it's possible
// glCheckFramebufferStatus might cache some calculation so we
// don't call here to ensure we get INVALID_FRAMEBUFFER_OPERATION
// when drawing.
TEST_P(FramebufferTest_ES3Metal, TooManyBitsGeneratesInvalidFramebufferOperation)
{
    ANGLE_SKIP_TEST_IF(
        !getEGLWindow()->isFeatureEnabled(Feature::LimitMaxColorTargetBitsForTesting));

    GLint maxDrawBuffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);

    GLFramebuffer framebuffer;
    std::vector<GLTexture> textures(maxDrawBuffers);
    std::vector<GLenum> drawBuffers(maxDrawBuffers, GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    std::stringstream fs;

    fs << R"(#version 300 es
      precision highp float;
      out uvec4 fragColor[)"
       << maxDrawBuffers << R"(];
      void main() {
      )";

    for (GLint i = 0; i < maxDrawBuffers; ++i)
    {
        fs << "  fragColor[" << i << "] = uvec4(" << i << ", " << i * 2 << ", " << i * 4 << ", "
           << i * 8 << ");\n";
        drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, 1, 1, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i],
                               0);
    }
    EXPECT_GL_NO_ERROR();

    fs << "}";

    constexpr const char vs[] = R"(#version 300 es
      void main() {
        gl_Position = vec4(0, 0, 0, 1);
        gl_PointSize = 1.0;
      }
    )";

    GLProgram program;
    program.makeRaster(vs, fs.str().c_str());
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    // Validate we can draw to maxDrawBuffers attachments
    glDrawBuffers(maxDrawBuffers, drawBuffers.data());
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    for (GLint i = 0; i < maxDrawBuffers; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, 1, 1, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT,
                     nullptr);
    }
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GLENUM_EQ(GL_INVALID_FRAMEBUFFER_OPERATION, glGetError());
}

class FramebufferTestWithFormatFallback : public ANGLETest<>
{
  protected:
    FramebufferTestWithFormatFallback()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    void texImageFollowedByFBORead(GLenum internalFormat, GLenum type);
    void blitCopyFollowedByFBORead(GLenum internalFormat, GLenum type);
    void copyTexImageFollowedBySampling(GLenum internalFormat, GLenum type);
    void cubeTexImageFollowedByFBORead(GLenum internalFormat, GLenum type);
    GLushort convertGLColorToUShort(GLenum internalFormat, const GLColor &color);
    static constexpr GLsizei kTexWidth  = 16;
    static constexpr GLsizei kTexHeight = 16;
    static constexpr GLsizei kMaxLevel  = 4;
};

GLushort FramebufferTestWithFormatFallback::convertGLColorToUShort(GLenum internalFormat,
                                                                   const GLColor &color)
{
    GLushort r, g, b, a;
    switch (internalFormat)
    {
        case GL_RGB5_A1:
            r = (color.R >> 3) << 11;
            g = (color.G >> 3) << 6;
            b = (color.B >> 3) << 1;
            a = color.A >> 7;
            break;
        case GL_RGBA4:
            r = (color.R >> 4) << 12;
            g = (color.G >> 4) << 8;
            b = (color.B >> 4) << 4;
            a = color.A >> 4;
            break;
        default:
            UNREACHABLE();
            r = 0;
            g = 0;
            b = 0;
            a = 0;
            break;
    }
    return r | g | b | a;
}

// Test texture format fallback while it has staged updates.
void FramebufferTestWithFormatFallback::texImageFollowedByFBORead(GLenum internalFormat,
                                                                  GLenum type)
{
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Texture2DLod(), essl3_shaders::fs::Texture2DLod());
    GLint textureLocation = glGetUniformLocation(program, essl3_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLocation);
    GLint lodLocation = glGetUniformLocation(program, essl3_shaders::LodUniform());
    ASSERT_NE(-1, lodLocation);

    const GLColor kColor = GLColor::blue;

    for (int loop = 0; loop < 4; loop++)
    {
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        const GLushort u16Color = convertGLColorToUShort(internalFormat, kColor);
        std::vector<GLushort> pixels(kTexWidth * kTexHeight, u16Color);
        if (loop == 0 || loop == 2)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, kTexWidth, kTexHeight, 0, GL_RGBA, type,
                         pixels.data());
        }
        else
        {
            glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, kTexWidth, kTexHeight);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTexWidth, kTexHeight, GL_RGBA, type,
                            pixels.data());
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (loop >= 2)
        {
            // Draw quad using texture
            glUseProgram(program);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glUniform1f(lodLocation, 0);
            drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
            EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);
            ASSERT_GL_NO_ERROR();
        }

        // attach blue texture to FBO
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        EXPECT_PIXEL_EQ(kTexWidth / 2, kTexHeight / 2, kColor.R, kColor.G, kColor.B, kColor.A);
        ASSERT_GL_NO_ERROR();
    }
}
TEST_P(FramebufferTestWithFormatFallback, R5G5B5A1_TexImage)
{
    texImageFollowedByFBORead(GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1);
}
TEST_P(FramebufferTestWithFormatFallback, R4G4B4A4_TexImage)
{
    texImageFollowedByFBORead(GL_RGBA4, GL_UNSIGNED_SHORT_4_4_4_4);
}

// Test texture format fallback while it has staged updates and then do copyTexImage2D and followed
// by sampling.
void FramebufferTestWithFormatFallback::copyTexImageFollowedBySampling(GLenum internalFormat,
                                                                       GLenum type)
{
    const GLColor kColor = GLColor::blue;
    // Create blue texture
    GLTexture blueTex2D;
    glBindTexture(GL_TEXTURE_2D, blueTex2D);
    const GLushort u16Color = convertGLColorToUShort(internalFormat, kColor);
    std::vector<GLushort> bluePixels(kTexWidth * kTexHeight, u16Color);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, kTexWidth, kTexHeight, 0, GL_RGBA, type,
                 bluePixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // attach blue texture to FBO and read back to verify. This should trigger format conversion
    GLFramebuffer blueFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, blueFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blueTex2D, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_EQ(kTexWidth / 2, kTexHeight / 2, 0, 0, 255, 255);
    ASSERT_GL_NO_ERROR();

    // Create red texture
    GLTexture copyTex2D;
    glBindTexture(GL_TEXTURE_2D, copyTex2D);
    std::vector<GLushort> redPixels(kTexWidth * kTexHeight, 0xF801);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, kTexWidth, kTexHeight, 0, GL_RGBA, type,
                 redPixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // CopyTexImage from blue to red
    glCopyTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 0, 0, kTexWidth, kTexHeight, 0);
    ASSERT_GL_NO_ERROR();

    // Draw with copyTex2D
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Texture2DLod(), essl3_shaders::fs::Texture2DLod());
    glUseProgram(program);
    GLint textureLocation = glGetUniformLocation(program, essl3_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLocation);
    GLint lodLocation = glGetUniformLocation(program, essl3_shaders::LodUniform());
    ASSERT_NE(-1, lodLocation);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, copyTex2D);
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUniform1f(lodLocation, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, kColor.R, kColor.G, kColor.B,
                    kColor.A);
    ASSERT_GL_NO_ERROR();
}
TEST_P(FramebufferTestWithFormatFallback, R5G5B5A1_CopyTexImage)
{
    copyTexImageFollowedBySampling(GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1);
}
TEST_P(FramebufferTestWithFormatFallback, R4G4B4A4_CopyTexImage)
{
    copyTexImageFollowedBySampling(GL_RGBA4, GL_UNSIGNED_SHORT_4_4_4_4);
}

// Test texture format fallback while it has staged updates and then do FBO blit and followed by
// copyTexImage2D.
void FramebufferTestWithFormatFallback::blitCopyFollowedByFBORead(GLenum internalFormat,
                                                                  GLenum type)
{
    for (int loop = 0; loop < 2; loop++)
    {
        // Create blue texture
        GLTexture blueTex2D;
        glBindTexture(GL_TEXTURE_2D, blueTex2D);
        GLushort u16Color = convertGLColorToUShort(internalFormat, GLColor::blue);
        std::vector<GLushort> bluePixels(kTexWidth * kTexHeight, u16Color);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, kTexWidth, kTexHeight, 0, GL_RGBA, type,
                     bluePixels.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // attach blue texture to FBO
        GLFramebuffer readFbo;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blueTex2D,
                               0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

        GLTexture redTex2D;
        GLRenderbuffer renderBuffer;
        GLFramebuffer drawFbo;
        if (loop == 0)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, kTexWidth, kTexHeight);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo);
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                      renderBuffer);
            EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);
            glClearColor(1.0, 0.0, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, redTex2D);
            u16Color = convertGLColorToUShort(internalFormat, GLColor::red);
            std::vector<GLushort> redPixels(kTexWidth * kTexHeight, u16Color);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, kTexWidth, kTexHeight, 0, GL_RGBA, type,
                         redPixels.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   redTex2D, 0);
            EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);
        }

        // Blit
        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
        glBlitFramebuffer(0, 0, kTexWidth, kTexHeight, 0, 0, kTexWidth, kTexHeight,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer readFbo2;
        if (loop == 0)
        {
            // CopyTexImage from renderBuffer to copyTex2D
            glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo2);
            glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                      renderBuffer);
        }
        else
        {

            // CopyTexImage from redTex2D to copyTex2D
            glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo2);
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   redTex2D, 0);
        }
        GLTexture copyTex2D;
        glBindTexture(GL_TEXTURE_2D, copyTex2D);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, kTexWidth, kTexHeight, 0);
        ASSERT_GL_NO_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Read out red texture
        GLFramebuffer readFbo3;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo3);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copyTex2D,
                               0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);
        EXPECT_PIXEL_EQ(kTexWidth / 2, kTexHeight / 2, 0, 0, 255, 255);
        ASSERT_GL_NO_ERROR();
    }
}
TEST_P(FramebufferTestWithFormatFallback, R5G5B5A1_BlitCopyTexImage)
{
    blitCopyFollowedByFBORead(GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1);
}
TEST_P(FramebufferTestWithFormatFallback, RGBA4444_BlitCopyTexImage)
{
    blitCopyFollowedByFBORead(GL_RGBA4, GL_UNSIGNED_SHORT_4_4_4_4);
}

// Test texture format fallback while it has staged updates, specially for cubemap target.
void FramebufferTestWithFormatFallback::cubeTexImageFollowedByFBORead(GLenum internalFormat,
                                                                      GLenum type)
{
    const GLColor kColors[6] = {GLColor::red,  GLColor::green,  GLColor::blue,
                                GLColor::cyan, GLColor::yellow, GLColor::magenta};
    GLTexture cubeTex2D;
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex2D);
    for (GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X; target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         target++)
    {
        int j                   = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        const GLushort u16Color = convertGLColorToUShort(internalFormat, kColors[j]);
        std::vector<GLushort> pixels(kTexWidth * kTexHeight, u16Color);
        glTexImage2D(target, 0, internalFormat, kTexWidth, kTexHeight, 0, GL_RGBA, type,
                     pixels.data());
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // attach blue texture to FBO
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    for (GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X; target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         target++)
    {
        GLint j = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, cubeTex2D, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        EXPECT_PIXEL_COLOR_EQ(kTexWidth / 2, kTexHeight / 2, kColors[j]) << "face " << j;
    }
    ASSERT_GL_NO_ERROR();
}
TEST_P(FramebufferTestWithFormatFallback, R5G5B5A1_CubeTexImage)
{
    cubeTexImageFollowedByFBORead(GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1);
}
TEST_P(FramebufferTestWithFormatFallback, R4G4B4A4_CubeTexImage)
{
    cubeTexImageFollowedByFBORead(GL_RGBA4, GL_UNSIGNED_SHORT_4_4_4_4);
}

// Tests that the out-of-range staged update is reformatted when mipmapping is enabled, but not
// before it.
TEST_P(FramebufferTestWithFormatFallback, R4G4B4A4_OutOfRangeStagedUpdateReformated)
{
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Texture2DLod(), essl3_shaders::fs::Texture2DLod());
    glUseProgram(program);
    GLint textureLocation = glGetUniformLocation(program, essl3_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLocation);
    GLint lodLocation = glGetUniformLocation(program, essl3_shaders::LodUniform());
    ASSERT_NE(-1, lodLocation);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    GLushort u16Color = convertGLColorToUShort(GL_RGBA4, GLColor::red);
    std::vector<GLushort> pixels(kTexWidth * kTexHeight, u16Color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexWidth, kTexHeight, 0, GL_RGBA,
                 GL_UNSIGNED_SHORT_4_4_4_4, pixels.data());
    u16Color = convertGLColorToUShort(GL_RGB5_A1, GLColor::green);
    pixels.assign(kTexWidth * kTexHeight, u16Color);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, kTexWidth / 2, kTexHeight / 2, 0, GL_RGBA,
                 GL_UNSIGNED_SHORT_5_5_5_1, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Draw quad
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1f(lodLocation, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 0, 255);

    // Now trigger format conversion
    GLFramebuffer readFbo;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowWidth() / 2, 255, 0, 0, 255);

    // update level0 with compatible data and enable mipmap
    u16Color = convertGLColorToUShort(GL_RGB5_A1, GLColor::blue);
    pixels.assign(kTexWidth * kTexHeight, u16Color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexWidth, kTexHeight, 0, GL_RGBA,
                 GL_UNSIGNED_SHORT_5_5_5_1, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);

    // Draw quad with lod0 and lod1 and verify color
    glUniform1f(lodLocation, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);
    glUniform1f(lodLocation, 1);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);
    ASSERT_GL_NO_ERROR();
}

// Tests that the texture is reformatted when the clear is done through the draw path.
TEST_P(FramebufferTestWithFormatFallback, R4G4B4A4_MaskedClear)
{
    for (int loop = 0; loop < 2; loop++)
    {
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        GLushort u16Color = convertGLColorToUShort(GL_RGBA4, GLColor::red);
        std::vector<GLushort> pixels(kTexWidth * kTexHeight, u16Color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexWidth, kTexHeight, 0, GL_RGBA,
                     GL_UNSIGNED_SHORT_4_4_4_4, pixels.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (loop == 0)
        {
            // Draw quad
            ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Texture2DLod(),
                             essl3_shaders::fs::Texture2DLod());
            glUseProgram(program);
            GLint textureLocation =
                glGetUniformLocation(program, essl3_shaders::Texture2DUniform());
            ASSERT_NE(-1, textureLocation);
            GLint lodLocation = glGetUniformLocation(program, essl3_shaders::LodUniform());
            ASSERT_NE(-1, lodLocation);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            glUniform1f(lodLocation, 0);
            drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
            EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 0, 255);
            ASSERT_GL_NO_ERROR();
        }

        // Now trigger format conversion with masked clear
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        glClearColor(0, 1, 1, 1);
        glColorMask(false, true, false, false);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_PIXEL_EQ(kTexWidth / 2, kTexHeight / 2, 255, 255, 0, 255);
        ASSERT_GL_NO_ERROR();
    }
}

// Tests that glGenerateMipmap works when the format is converted to renderable..
TEST_P(FramebufferTestWithFormatFallback, R4G4B4A4_GenerateMipmap)
{
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Texture2DLod(), essl3_shaders::fs::Texture2DLod());
    glUseProgram(program);
    GLint textureLocation = glGetUniformLocation(program, essl3_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLocation);
    GLint lodLocation = glGetUniformLocation(program, essl3_shaders::LodUniform());
    ASSERT_NE(-1, lodLocation);

    for (int loop = 0; loop < 4; loop++)
    {
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        GLushort u16Color = convertGLColorToUShort(GL_RGBA4, GLColor::red);
        std::vector<GLushort> pixels(kTexWidth * kTexHeight, u16Color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexWidth, kTexHeight, 0, GL_RGBA,
                     GL_UNSIGNED_SHORT_4_4_4_4, pixels.data());
        u16Color = convertGLColorToUShort(GL_RGBA4, GLColor::green);
        pixels.assign(kTexWidth * kTexHeight, u16Color);
        glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, kTexWidth / 2, kTexHeight / 2, 0, GL_RGBA,
                     GL_UNSIGNED_SHORT_5_5_5_1, pixels.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (loop == 0 || loop == 2)
        {
            // Draw quad
            glUniform1f(lodLocation, 0);
            drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
            EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 0, 255);
            ASSERT_GL_NO_ERROR();
        }

        if (loop > 2)
        {
            // Now trigger format conversion
            GLFramebuffer readFbo;
            glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   texture, 0);
            EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);
        }

        // GenerateMipmap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Verify each lod
        for (int lod = 0; lod <= kMaxLevel; lod++)
        {
            glUniform1f(lodLocation, lod);
            drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
            EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 0, 255);
        }
        ASSERT_GL_NO_ERROR();
    }
}

// Tests that when reformatting the image, incompatible updates don't cause a problem.
TEST_P(FramebufferTestWithFormatFallback, R4G4B4A4_InCompatibleFormat)
{
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Texture2DLod(), essl3_shaders::fs::Texture2DLod());
    glUseProgram(program);
    GLint textureLocation = glGetUniformLocation(program, essl3_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLocation);
    GLint lodLocation = glGetUniformLocation(program, essl3_shaders::LodUniform());
    ASSERT_NE(-1, lodLocation);

    for (int loop = 0; loop < 4; loop++)
    {
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        // Define a texture with lod0 and lod1 with two different effective internal formats or size
        GLushort u16Color = convertGLColorToUShort(GL_RGBA4, GLColor::red);
        std::vector<GLushort> pixels(kTexWidth * kTexHeight, u16Color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexWidth, kTexHeight, 0, GL_RGBA,
                     GL_UNSIGNED_SHORT_4_4_4_4, pixels.data());
        if (loop < 2)
        {
            u16Color = convertGLColorToUShort(GL_RGB5_A1, GLColor::green);
            pixels.assign(kTexWidth * kTexHeight, u16Color);
            // bad effective internal format
            glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, kTexWidth / 2, kTexHeight / 2, 0, GL_RGBA,
                         GL_UNSIGNED_SHORT_5_5_5_1, pixels.data());
        }
        else
        {
            u16Color = convertGLColorToUShort(GL_RGBA4, GLColor::green);
            pixels.assign(kTexWidth * kTexHeight, u16Color);
            // bad size
            glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, kTexWidth, kTexHeight, 0, GL_RGBA,
                         GL_UNSIGNED_SHORT_4_4_4_4, pixels.data());
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Now trigger format conversion and verify lod0
        GLFramebuffer readFbo;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture,
                               0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);
        EXPECT_PIXEL_EQ(kTexWidth / 2, kTexHeight / 2, 255, 0, 0, 255);

        if (loop == 1 || loop == 3)
        {
            // Disable mipmap and sample from lod0 and verify
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glUniform1f(lodLocation, 0);
            drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
            EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 0, 255);
        }
    }
}

class FramebufferTest_ES31 : public ANGLETest<>
{
  protected:
    void validateSamplePass(GLQuery &query, GLint width, GLint height)
    {
        GLuint passedCount;

        glUniform2i(0, width - 1, height - 1);
        glBeginQuery(GL_ANY_SAMPLES_PASSED, query);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEndQuery(GL_ANY_SAMPLES_PASSED);
        glGetQueryObjectuiv(query, GL_QUERY_RESULT, &passedCount);
        EXPECT_GT(static_cast<GLint>(passedCount), 0);

        glUniform2i(0, width - 1, height);
        glBeginQuery(GL_ANY_SAMPLES_PASSED, query);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEndQuery(GL_ANY_SAMPLES_PASSED);
        glGetQueryObjectuiv(query, GL_QUERY_RESULT, &passedCount);
        EXPECT_EQ(static_cast<GLint>(passedCount), 0);

        glUniform2i(0, width, height - 1);
        glBeginQuery(GL_ANY_SAMPLES_PASSED, query);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEndQuery(GL_ANY_SAMPLES_PASSED);
        glGetQueryObjectuiv(query, GL_QUERY_RESULT, &passedCount);
        EXPECT_EQ(static_cast<GLint>(passedCount), 0);
    }

    void verifyDepth(GLuint framebuffer, int width, int height, float depthValue)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        GLint colorAttachment = GL_NONE;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                              &colorAttachment);

        // If no color attachment add a temp one for verification.
        GLTexture tempColor;
        if (colorAttachment == GL_NONE)
        {
            glBindTexture(GL_TEXTURE_2D, tempColor);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempColor,
                                   0);
        }

        // Use a small shader to verify depth.
        ANGLE_GL_PROGRAM(depthTestProgram, essl1_shaders::vs::Passthrough(),
                         essl1_shaders::fs::Blue());
        ANGLE_GL_PROGRAM(depthTestProgramFail, essl1_shaders::vs::Passthrough(),
                         essl1_shaders::fs::Red());

        GLboolean hasDepthTest   = GL_FALSE;
        GLboolean hasDepthWrite  = GL_TRUE;
        GLint prevDepthFunc      = GL_ALWAYS;
        GLboolean hasStencilTest = GL_FALSE;

        glGetBooleanv(GL_DEPTH_TEST, &hasDepthTest);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &hasDepthWrite);
        glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFunc);
        glGetBooleanv(GL_STENCIL_TEST, &hasStencilTest);

        if (!hasDepthTest)
        {
            glEnable(GL_DEPTH_TEST);
        }
        if (hasDepthWrite)
        {
            glDepthMask(GL_FALSE);
        }
        if (hasStencilTest)
        {
            glDisable(GL_STENCIL_TEST);
        }
        glDepthFunc(GL_LESS);
        drawQuad(depthTestProgram, essl1_shaders::PositionAttrib(), depthValue - 0.01f);
        drawQuad(depthTestProgramFail, essl1_shaders::PositionAttrib(), depthValue + 0.01f);
        if (!hasDepthTest)
        {
            glDisable(GL_DEPTH_TEST);
        }
        if (hasDepthWrite)
        {
            glDepthMask(GL_TRUE);
        }
        glDepthFunc(prevDepthFunc);
        if (hasStencilTest)
        {
            glEnable(GL_STENCIL_TEST);
        }
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_RECT_EQ(0, 0, width, height, GLColor::blue);

        if (colorAttachment == GL_NONE)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        }
    }

    void verifyStencil(GLuint framebuffer, int width, int height, uint32_t stencilValue)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        GLint colorAttachment = GL_NONE;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                                              &colorAttachment);

        // If no color attachment add a temp one for verification.
        GLTexture tempColor;
        if (colorAttachment == GL_NONE)
        {
            glBindTexture(GL_TEXTURE_2D, tempColor);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempColor,
                                   0);
        }

        // Use another small shader to verify stencil.
        ANGLE_GL_PROGRAM(stencilTestProgram, essl1_shaders::vs::Passthrough(),
                         essl1_shaders::fs::Green());
        GLboolean hasStencilTest   = GL_FALSE;
        GLint prevStencilFunc      = GL_ALWAYS;
        GLint prevStencilValue     = 0xFF;
        GLint prevStencilRef       = 0xFF;
        GLint prevStencilFail      = GL_KEEP;
        GLint prevStencilDepthFail = GL_KEEP;
        GLint prevStencilDepthPass = GL_KEEP;
        GLboolean hasDepthTest     = GL_FALSE;

        glGetBooleanv(GL_STENCIL_TEST, &hasStencilTest);
        glGetIntegerv(GL_STENCIL_FUNC, &prevStencilFunc);
        glGetIntegerv(GL_STENCIL_VALUE_MASK, &prevStencilValue);
        glGetIntegerv(GL_STENCIL_REF, &prevStencilRef);
        glGetIntegerv(GL_STENCIL_FAIL, &prevStencilFail);
        glGetIntegerv(GL_STENCIL_PASS_DEPTH_FAIL, &prevStencilDepthFail);
        glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &prevStencilDepthPass);
        glGetBooleanv(GL_DEPTH_TEST, &hasDepthTest);

        if (!hasStencilTest)
        {
            glEnable(GL_STENCIL_TEST);
        }
        glStencilFunc(GL_EQUAL, stencilValue, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        if (hasDepthTest)
        {
            glDisable(GL_DEPTH_TEST);
        }
        drawQuad(stencilTestProgram, essl1_shaders::PositionAttrib(), 0.0f);
        if (!hasStencilTest)
        {
            glDisable(GL_STENCIL_TEST);
        }
        glStencilFunc(prevStencilFunc, prevStencilValue, prevStencilRef);
        glStencilOp(prevStencilFail, prevStencilDepthFail, prevStencilDepthPass);
        if (hasDepthTest)
        {
            glEnable(GL_DEPTH_TEST);
        }
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_RECT_EQ(0, 0, width, height, GLColor::green);

        if (colorAttachment == GL_NONE)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        }
    }

    static constexpr char kFSWriteRedGreen[] = R"(#extension GL_EXT_draw_buffers : enable
precision highp float;
void main()
{
    gl_FragData[0] = vec4(1.0, 0.0, 0.0, 1.0);  // attachment 0: red
    gl_FragData[1] = vec4(0.0, 1.0, 0.0, 1.0);  // attachment 1: green
})";
};

// Test that without attachment, if either the value of FRAMEBUFFER_DEFAULT_WIDTH or
// FRAMEBUFFER_DEFAULT_HEIGHT parameters is zero, the framebuffer is incomplete.
TEST_P(FramebufferTest_ES31, IncompleteMissingAttachmentDefaultParam)
{
    GLFramebuffer mFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 1);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, 1);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 0);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 1);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 0);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, 1);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

// Test that the sample count of a mix of texture and renderbuffer should be same.
TEST_P(FramebufferTest_ES31, IncompleteMultisampleSampleCountMix)
{
    GLFramebuffer mFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    // Lookup the supported number of sample counts (rely on fact that ANGLE uses the same set of
    // sample counts for textures and renderbuffers)
    GLint numSampleCounts = 0;
    std::vector<GLint> sampleCounts;
    GLsizei queryBufferSize = 1;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_RGBA8, GL_NUM_SAMPLE_COUNTS,
                          queryBufferSize, &numSampleCounts);
    ANGLE_SKIP_TEST_IF((numSampleCounts < 2));
    sampleCounts.resize(numSampleCounts);
    queryBufferSize = numSampleCounts;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_RGBA8, GL_SAMPLES, queryBufferSize,
                          sampleCounts.data());

    GLTexture mTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCounts[0], GL_RGBA8, 1, 1, true);

    GLRenderbuffer mRenderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCounts[1], GL_RGBA8, 1, 1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           mTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, mRenderbuffer);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

// Test that the sample count of texture attachments should be same.
TEST_P(FramebufferTest_ES31, IncompleteMultisampleSampleCountTex)
{
    GLFramebuffer mFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    // Lookup the supported number of sample counts
    GLint numSampleCounts = 0;
    std::vector<GLint> sampleCounts;
    GLsizei queryBufferSize = 1;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_RGBA8, GL_NUM_SAMPLE_COUNTS,
                          queryBufferSize, &numSampleCounts);
    ANGLE_SKIP_TEST_IF((numSampleCounts < 2));
    sampleCounts.resize(numSampleCounts);
    queryBufferSize = numSampleCounts;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_RGBA8, GL_SAMPLES, queryBufferSize,
                          sampleCounts.data());

    GLTexture mTextures[2];
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTextures[0]);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCounts[0], GL_RGBA8, 1, 1, true);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTextures[1]);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCounts[1], GL_RGBA8, 1, 1, true);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           mTextures[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE,
                           mTextures[1], 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

// Test that if the attached images are a mix of renderbuffers and textures, the value of
// TEXTURE_FIXED_SAMPLE_LOCATIONS must be TRUE for all attached textures.
TEST_P(FramebufferTest_ES31, IncompleteMultisampleFixedSampleLocationsMix)
{
    GLFramebuffer mFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    GLTexture mTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, 1, 1, false);

    GLRenderbuffer mRenderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 1, GL_RGBA8, 1, 1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           mTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, mRenderbuffer);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

// Test that the value of TEXTURE_FIXED_SAMPLE_LOCATIONS is the same for all attached textures.
TEST_P(FramebufferTest_ES31, IncompleteMultisampleFixedSampleLocationsTex)
{
    GLFramebuffer mFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    GLTexture mTextures[2];
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTextures[0]);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, 1, 1, false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           mTextures[0], 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTextures[1]);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGB8, 1, 1, true);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE,
                           mTextures[1], 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
                     glCheckFramebufferStatus(GL_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

// Tests that draw to Y-flipped FBO results in correct pixels.
TEST_P(FramebufferTest_ES31, BasicDrawToYFlippedFBO)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Remove the flag so that glReadPixels do not implicitly use that.
    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 0);

    constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 255 - kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255,
                      1.0);
    EXPECT_PIXEL_NEAR(0, kSize - 1, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 255 - kHalfPixelGradient, kHalfPixelGradient, 0, 255,
                      1.0);
}

// Test resolving a multisampled texture with blit
TEST_P(FramebufferTest_ES31, MultisampleResolveWithBlit)
{
    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 255 - kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kSize - 1, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 255 - kHalfPixelGradient, 255 - kHalfPixelGradient, 0,
                      255, 1.0);
}

// Test clearing a 2D multisample texture defined using glTexStorage2DMultisample().
TEST_P(FramebufferTest_ES31, ClearTextureEXT2DMS)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    constexpr uint32_t kSamples = 4;
    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, kSamples, GL_RGBA8, 16, 16, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearTexImageEXT(msaaTexture, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample texture into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, 16, 16, 0, 0, 16, 16, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test clearing a 2D multisample RGB texture.
TEST_P(FramebufferTest_ES31, ClearTextureEXT2DMSRGB)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    constexpr uint32_t kSamples = 4;
    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, kSamples, GL_RGB8, 16, 16, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearTexImageEXT(msaaTexture, 0, GL_RGB, GL_UNSIGNED_BYTE, &GLColor::blue);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample texture into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, 16, 16, 0, 0, 16, 16, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Test clearing corners of a 2D multisample texture to different colors.
TEST_P(FramebufferTest_ES31, ClearTextureEXT2DMSBlocks)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    constexpr uint32_t kSamples = 4;
    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, kSamples, GL_RGBA8, 16, 16, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearTexSubImageEXT(msaaTexture, 0, 0, 0, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                          &GLColor::red);
    glClearTexSubImageEXT(msaaTexture, 0, 0, 8, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                          &GLColor::green);
    glClearTexSubImageEXT(msaaTexture, 0, 8, 0, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                          &GLColor::blue);
    glClearTexSubImageEXT(msaaTexture, 0, 8, 8, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                          &GLColor::yellow);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample texture into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, 16, 16, 0, 0, 16, 16, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 8, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, 8, 8, 8, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(8, 0, 8, 8, GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(8, 8, 8, 8, GLColor::yellow);
}

// Test clearing layers of a 2D texture array defined using glTexStorage3DMultisampleOES().
TEST_P(FramebufferTest_ES31, ClearTextureEXT2DArrayMS)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_storage_multisample_2d_array"));

    constexpr uint32_t kSamples = 4;
    constexpr uint32_t kLayers  = 4;
    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, msaaTexture);
    glTexStorage3DMultisampleOES(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, kSamples, GL_RGBA8, 16, 16,
                                 kLayers, false);
    ASSERT_GL_NO_ERROR();

    std::array<GLColor, kLayers> clearColors = {GLColor::red, GLColor::green, GLColor::blue,
                                                GLColor::yellow};

    for (uint32_t i = 0; i < kLayers; i++)
    {
        glClearTexSubImageEXT(msaaTexture, 0, 0, 0, i, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                              &clearColors[i]);
        ASSERT_GL_NO_ERROR();
    }

    // Create another FBO to resolve each multisample texture layer into.
    GLFramebuffer resolveFBO;
    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    for (uint32_t i = 0; i < kLayers; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, msaaTexture, 0, i);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glBindTexture(GL_TEXTURE_2D, resolveTexture);
        glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture,
                               0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
        glBlitFramebuffer(0, 0, 16, 16, 0, 0, 16, 16, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
        EXPECT_PIXEL_COLOR_EQ(0, 0, clearColors[i]);
    }
}

// Test clearing a 2D depth/stencil multisample texture.
TEST_P(FramebufferTest_ES31, ClearTextureEXT2DMSDS)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    constexpr uint32_t kSamples = 4;
    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, kSamples, GL_DEPTH24_STENCIL8, 16, 16,
                              false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLuint dsValue = 0xFFFFFFAA;
    glClearTexImageEXT(msaaTexture, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, &dsValue);
    ASSERT_GL_NO_ERROR();

    // Create a color texture and a resolve FBO for the multisample D/S texture.
    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(colorTexture, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    GLTexture resolveDSTexture;
    glBindTexture(GL_TEXTURE_2D, resolveDSTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 16, 16, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, nullptr);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           resolveDSTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, 16, 16, 0, 0, 16, 16, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw on this framebuffer once with enabled depth test and once with enabled stencil test.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LESS, 0xCC, 0xFF);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);

    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);
}

// Test clearing two sides of a 2D D/S multisample texture to different values.
TEST_P(FramebufferTest_ES31, ClearTextureEXT2DMSDSPartial)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    constexpr uint32_t kSamples = 4;
    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, kSamples, GL_DEPTH24_STENCIL8, 16, 16,
                              false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLuint dsValue0 = 0x000000AA;
    GLuint dsValue1 = 0x000000EE;
    GLuint dsValue2 = 0xFFFFFFAA;
    GLuint dsValue3 = 0xFFFFFFEE;
    glClearTexSubImageEXT(msaaTexture, 0, 0, 0, 0, 8, 8, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
                          &dsValue0);
    glClearTexSubImageEXT(msaaTexture, 0, 0, 8, 0, 8, 8, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
                          &dsValue1);
    glClearTexSubImageEXT(msaaTexture, 0, 8, 0, 0, 8, 8, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
                          &dsValue2);
    glClearTexSubImageEXT(msaaTexture, 0, 8, 8, 0, 8, 8, 1, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8,
                          &dsValue3);
    ASSERT_GL_NO_ERROR();

    // Create a color texture and a resolve FBO for the multisample D/S texture.
    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(colorTexture, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    GLTexture resolveDSTexture;
    glBindTexture(GL_TEXTURE_2D, resolveDSTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 16, 16, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, nullptr);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           resolveDSTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, 16, 16, 0, 0, 16, 16, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw on this framebuffer once with enabled depth test and once with enabled stencil test.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());

    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 16, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(8, 0, 8, 16, GLColor::blue);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LESS, 0xCC, 0xFF);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 8, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(8, 0, 8, 16, GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(0, 8, 16, 8, GLColor::blue);
}

// Test clearing a 2D depth-only multisample texture.
TEST_P(FramebufferTest_ES31, ClearTextureEXT2DMSDepth)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));

    constexpr uint32_t kSamples = 4;
    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, kSamples, GL_DEPTH_COMPONENT32F, 16, 16,
                              false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLfloat depthValue = 1.0f;
    glClearTexImageEXT(msaaTexture, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &depthValue);
    ASSERT_GL_NO_ERROR();

    // Create a color texture and a resolve FBO for the multisample depth texture.
    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(colorTexture, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    GLTexture resolveDepthTexture;
    glBindTexture(GL_TEXTURE_2D, resolveDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 16, 16, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
                 nullptr);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, resolveDepthTexture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, 16, 16, 0, 0, 16, 16, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw on this framebuffer.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);
}

// Test clearing a 2D stencil-only multisample texture.
TEST_P(FramebufferTest_ES31, ClearTextureEXT2DMSStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_stencil8"));

    constexpr uint32_t kSamples = 4;
    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, kSamples, GL_STENCIL_INDEX8, 16, 16,
                              false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLint stencilValue = 0xEE;
    glClearTexImageEXT(msaaTexture, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &stencilValue);
    ASSERT_GL_NO_ERROR();

    // Create a color texture and a resolve FBO for the multisample stencil texture.
    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexImageEXT(colorTexture, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    GLTexture resolveStencilTexture;
    glBindTexture(GL_TEXTURE_2D, resolveStencilTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, 16, 16, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE,
                 nullptr);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           resolveStencilTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, 16, 16, 0, 0, 16, 16, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw on this framebuffer.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LESS, 0xCC, 0xFF);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::blue);
}

// Test resolving a multisampled texture with blit to a different format
TEST_P(FramebufferTest_ES31, MultisampleResolveWithBlitDifferentFormats)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_format_BGRA8888"));

    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA8_EXT, kSize, kSize, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE,
                 nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);

    // Another attachment of the same format as the blit source
    // to ensure that it does not confuse the backend.
    GLTexture resolveTexture2;
    glBindTexture(GL_TEXTURE_2D, resolveTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, resolveTexture2, 0);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    for (const GLenum buffer : {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1})
    {
        glReadBuffer(buffer);
        constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
        EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
        EXPECT_PIXEL_NEAR(kSize - 1, 0, 255 - kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
        EXPECT_PIXEL_NEAR(0, kSize - 1, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
        EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 255 - kHalfPixelGradient, 255 - kHalfPixelGradient,
                          0, 255, 1.0);
    }
}

// Test resolving a multisampled texture with blit after drawing to mulitiple FBOs.
TEST_P(FramebufferTest_ES31, MultisampleResolveWithBlitMultipleFBOs)
{
    // FBO 1 -> multisample draw (red)
    // FBO 2 -> multisample draw (green)
    // Bind FBO 1 as read
    // Bind FBO 3 as draw
    // Resolve

    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBORed;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBORed);

    GLTexture textureRed;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureRed);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           textureRed, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(redProgram, essl31_shaders::vs::Simple(), essl31_shaders::fs::Red());
    drawQuad(redProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer msaaFBOGreen;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBOGreen);

    GLTexture textureGreen;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureGreen);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           textureGreen, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(greenProgram, essl31_shaders::vs::Simple(), essl31_shaders::fs::Green());
    drawQuad(greenProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBORed);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test resolving a multisampled texture with blit after drawing to mulitiple FBOs.
TEST_P(FramebufferTest_ES31, MultisampleResolveWithBlitMultipleResolves)
{
    // Draw multisampled in FBO 1
    // Bind FBO 1 as read
    // Bind FBO 2 as draw
    // Resolve
    // Bind FBO 3 as draw
    // Resolve

    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBORed;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBORed);

    GLTexture textureRed;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureRed);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           textureRed, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(redProgram, essl31_shaders::vs::Simple(), essl31_shaders::fs::Red());
    drawQuad(redProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture1;
    GLFramebuffer resolveFBO1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture1, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBORed);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture2;
    GLFramebuffer resolveFBO2;
    glBindTexture(GL_TEXTURE_2D, resolveTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBORed);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO2);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO2);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test resolving a multisampled texture with blit into an FBO with different read and draw
// attachments.
TEST_P(FramebufferTest_ES31, MultisampleResolveWithBlitDifferentReadDrawBuffers)
{
    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);

    // Bind both read and draw textures as separate attachments.
    const std::vector<GLColor> blueColors(kSize * kSize, GLColor::blue);
    GLTexture resolveReadTexture;
    glBindTexture(GL_TEXTURE_2D, resolveReadTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 blueColors.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveReadTexture,
                           0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    ASSERT_GL_NO_ERROR();

    GLTexture resolveDrawTexture;
    glBindTexture(GL_TEXTURE_2D, resolveDrawTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, resolveDrawTexture,
                           0);
    // Only enable color attachment 1 to be drawn to, since the Vulkan back end (currently) only
    // supports using resolve attachments when there is a single draw attachment enabled. This
    // ensures that the read and draw images are treated separately, including their layouts.
    GLenum drawBuffers[] = {GL_NONE, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 255 - kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kSize - 1, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 255 - kHalfPixelGradient, 255 - kHalfPixelGradient, 0,
                      255, 1.0);
}

// Test resolving a multisampled texture into a mipmaped texture with blit
TEST_P(FramebufferTest_ES31, MultisampleResolveIntoMipMapWithBlit)
{
    // FBO 1 is attached to a 64x64 texture
    // FBO 2 attached to level 1 of a 128x128 texture

    constexpr int kSize = 64;
    glViewport(0, 0, kSize, kSize);

    // Create the textures early and call glGenerateMipmap() so it doesn't break the render pass
    // between the drawQuad() and glBlitFramebuffer(), so we can test the resolve with subpass path
    // in the Vulkan back end.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();

    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 255 - kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kSize - 1, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 255 - kHalfPixelGradient, 255 - kHalfPixelGradient, 0,
                      255, 1.0);
}

// Test resolving a multisampled texture with blit after drawing to multiple FBOs.
TEST_P(FramebufferTest_ES31, MultipleTextureMultisampleResolveWithBlitMultipleResolves)
{
    // Attach two MSAA textures to FBO1
    // Set read buffer 0
    // Resolve into FBO2
    // Set read buffer 1
    // Resolve into FBO3

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_draw_buffers"));

    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTextureRed;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTextureRed);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTextureRed, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture msaaTextureGreen;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTextureGreen);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTextureGreen, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Setup program to render red into attachment 0 and green into attachment 1.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFSWriteRedGreen);
    glUseProgram(program);
    constexpr GLenum kDrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, kDrawBuffers);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture1;
    GLFramebuffer resolveFBO1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture1, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
    glReadBuffer(GL_COLOR_ATTACHMENT0);  // Red
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture2;
    GLFramebuffer resolveFBO2;
    glBindTexture(GL_TEXTURE_2D, resolveTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO2);
    glReadBuffer(GL_COLOR_ATTACHMENT1);  // Green
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO2);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test resolving a multisampled texture with blit after drawing to multiple FBOs, with color
// attachment 1 resolved first.
TEST_P(FramebufferTest_ES31,
       MultipleTextureMultisampleResolveWithBlitMultipleResolvesAttachment1First)
{
    // Attach two MSAA textures to FBO1
    // Set read buffer 1
    // Resolve into FBO2
    // Set read buffer 0
    // Resolve into FBO3

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_draw_buffers"));

    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTextureRed;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTextureRed);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTextureRed, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture msaaTextureGreen;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTextureGreen);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTextureGreen, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Setup program to render red into attachment 0 and green into attachment 1.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFSWriteRedGreen);
    glUseProgram(program);
    constexpr GLenum kDrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, kDrawBuffers);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture1;
    GLFramebuffer resolveFBO1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture1, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
    glReadBuffer(GL_COLOR_ATTACHMENT1);  // Green
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture2;
    GLFramebuffer resolveFBO2;
    glBindTexture(GL_TEXTURE_2D, resolveTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO2);
    glReadBuffer(GL_COLOR_ATTACHMENT0);  // Red
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO2);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test resolving a multisampled texture with blit, then drawing multisampled again.  The latter
// should not get re-resolved automatically.
TEST_P(FramebufferTest_ES31, MultisampleResolveWithBlitThenDraw)
{
    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 255 - kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kSize - 1, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 255 - kHalfPixelGradient, 255 - kHalfPixelGradient, 0,
                      255, 1.0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
    ANGLE_GL_PROGRAM(blueProgram, essl3_shaders::vs::Passthrough(), essl3_shaders::fs::Blue());
    drawQuad(blueProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // The resolved FBO should be unaffected by the last draw call.
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 255 - kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kSize - 1, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 255 - kHalfPixelGradient, 255 - kHalfPixelGradient, 0,
                      255, 1.0);
}

// Test resolving a multisampled texture with blit, then drawing multisampled again.  The latter
// should not get re-resolved automatically.  Resoloves color attachment 1.
TEST_P(FramebufferTest_ES31, MultisampleResolveWithBlitThenDrawAttachment1)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_draw_buffers"));

    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTextureRed;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTextureRed);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTextureRed, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture msaaTextureGreen;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTextureGreen);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTextureGreen, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Setup program to render red into attachment 0 and green into attachment 1.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFSWriteRedGreen);
    glUseProgram(program);
    constexpr GLenum kDrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, kDrawBuffers);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glReadBuffer(GL_COLOR_ATTACHMENT1);  // Green
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::green);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
    ANGLE_GL_PROGRAM(blueProgram, essl3_shaders::vs::Passthrough(), essl3_shaders::fs::Blue());
    drawQuad(blueProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // The resolved FBO should be unaffected by the last draw call.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::green);
}

// Test resolving a multisampled texture with blit, then drawing multisampled again and resolving to
// same framebuffer.
TEST_P(FramebufferTest_ES31, MultisampleResolveWithBlitThenDrawThenResolveAgain)
{
    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 255 - kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kSize - 1, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 255 - kHalfPixelGradient, 255 - kHalfPixelGradient, 0,
                      255, 1.0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
    ANGLE_GL_PROGRAM(blueProgram, essl3_shaders::vs::Passthrough(), essl3_shaders::fs::Blue());
    drawQuad(blueProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Verify that the resolve happened correctly
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::blue);
}

// Test resolving a multisampled texture with blit, then drawing multisampled again and resolving to
// another framebuffer.
TEST_P(FramebufferTest_ES31, MultisampleResolveWithBlitThenDrawThenResolveAgainToDifferentFBO)
{
    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture1;
    GLFramebuffer resolveFBO1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture1, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO1);
    constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 255 - kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kSize - 1, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 255 - kHalfPixelGradient, 255 - kHalfPixelGradient, 0,
                      255, 1.0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
    ANGLE_GL_PROGRAM(blueProgram, essl3_shaders::vs::Passthrough(), essl3_shaders::fs::Blue());
    drawQuad(blueProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture2;
    GLFramebuffer resolveFBO2;
    glBindTexture(GL_TEXTURE_2D, resolveTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO2);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Verify that the resolve happened to the correct FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO2);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::blue);

    // The first resolve FBO should be untouched.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO1);
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 255 - kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kSize - 1, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 255 - kHalfPixelGradient, 255 - kHalfPixelGradient, 0,
                      255, 1.0);
}

// Test resolving a multisampled texture with blit to a non-zero level.
TEST_P(FramebufferTest_ES31, MultisampleResolveWithBlitNonZeroLevel)
{
    constexpr int kWidth  = 16;
    constexpr int kHeight = 20;
    glViewport(0, 0, kWidth, kHeight);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create a resolve FBO and texture. The multisample buffer will be resolved into level 1 of the
    // bound texture, which has the same dimensions as the multisampled texture.
    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, kWidth * 2, kHeight * 2);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    constexpr uint8_t kWidthHalfPixelGradient  = 256 / kWidth / 2;
    constexpr uint8_t kHeightHalfPixelGradient = 256 / kHeight / 2;
    EXPECT_PIXEL_NEAR(0, 0, kWidthHalfPixelGradient, kHeightHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kWidth - 1, 0, 255 - kWidthHalfPixelGradient, kHeightHalfPixelGradient, 0,
                      255, 1.0);
    EXPECT_PIXEL_NEAR(0, kHeight - 1, kWidthHalfPixelGradient, 255 - kHeightHalfPixelGradient, 0,
                      255, 1.0);
    EXPECT_PIXEL_NEAR(kWidth - 1, kHeight - 1, 255 - kWidthHalfPixelGradient,
                      255 - kHeightHalfPixelGradient, 0, 255, 1.0);
}

// If there are no attachments, rendering will be limited to a rectangle having a lower left of
// (0, 0) and an upper right of(width, height), where width and height are the framebuffer
// object's default width and height.
TEST_P(FramebufferTest_ES31, RenderingLimitToDefaultFBOSizeWithNoAttachments)
{
    // anglebug.com/40644635
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    constexpr char kVS1[] = R"(#version 310 es
in layout(location = 0) highp vec2 a_position;
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
})";

    constexpr char kFS1[] = R"(#version 310 es
uniform layout(location = 0) highp ivec2 u_expectedSize;
out layout(location = 3) mediump vec4 f_color;
void main()
{
    if (ivec2(gl_FragCoord.xy) != u_expectedSize) discard;
    f_color = vec4(1.0, 0.5, 0.25, 1.0);
})";

    constexpr char kVS2[] = R"(#version 310 es
in layout(location = 0) highp vec2 a_position;
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
})";

    constexpr char kFS2[] = R"(#version 310 es
uniform layout(location = 0) highp ivec2 u_expectedSize;
out layout(location = 2) mediump vec4 f_color;
void main()
{
    if (ivec2(gl_FragCoord.xy) != u_expectedSize) discard;
    f_color = vec4(1.0, 0.5, 0.25, 1.0);
})";

    ANGLE_GL_PROGRAM(program1, kVS1, kFS1);
    ANGLE_GL_PROGRAM(program2, kVS2, kFS2);

    glUseProgram(program1);

    GLFramebuffer mFramebuffer;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer);
    GLuint defaultWidth  = 1;
    GLuint defaultHeight = 1;

    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, defaultWidth);
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, defaultHeight);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    const float data[] = {
        1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
    };

    GLQuery query;

    GLVertexArray vertexArray;
    glBindVertexArray(vertexArray);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    validateSamplePass(query, defaultWidth, defaultHeight);

    glUseProgram(program2);
    validateSamplePass(query, defaultWidth, defaultHeight);

    glUseProgram(program1);
    // If fbo has attachments, the rendering size should be the same as its attachment.
    GLTexture mTexture;
    GLuint width  = 2;
    GLuint height = 2;
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

    const GLenum bufs[] = {GL_NONE, GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT3};

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, mTexture, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    glDrawBuffers(4, bufs);

    validateSamplePass(query, width, height);

    // If fbo's attachment has been removed, the rendering size should be the same as framebuffer
    // default size.
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    validateSamplePass(query, defaultWidth, defaultHeight);
    ASSERT_GL_NO_ERROR();
}

// Creating two attachmentless framebuffers should work without vvl error.
TEST_P(FramebufferTest_ES31, CreateNoAttachmentFBOWithDifferentSize)
{
    constexpr char kVS1[] = R"(#version 310 es
in layout(location = 0) highp vec2 a_position;
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
})";

    constexpr char kFS1[] = R"(#version 310 es
uniform layout(location = 0) highp ivec2 u_expectedSize;
out layout(location = 3) mediump vec4 f_color;
void main()
{
    if (ivec2(gl_FragCoord.xy) != u_expectedSize) discard;
    f_color = vec4(1.0, 0.5, 0.25, 1.0);
})";
    ANGLE_GL_PROGRAM(program1, kVS1, kFS1);
    glUseProgram(program1);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    const float data[] = {
        1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

    GLQuery query;

    // Test that:
    // 1. create 1st no-attachment framebuffer with size 1*1, draw, delete framebuffer
    // 2. create 2nd no-attachment framebuffer with size 2*2, draw, delete framebuffer
    // works properly
    for (int loop = 0; loop < 2; loop++)
    {
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
        GLuint defaultWidth  = 1 << loop;
        GLuint defaultHeight = 1 << loop;
        glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, defaultWidth);
        glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, defaultHeight);
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

        // Draw and check the FBO size
        validateSamplePass(query, defaultWidth, defaultHeight);
    }

    ASSERT_GL_NO_ERROR();

    // Test that:
    // 1. create 1st no-attachment framebuffer with size 2*2, draw, delete framebuffer
    // 2. create 2nd no-attachment framebuffer with size 1*1, draw, delete framebuffer
    // works properly
    for (int loop = 1; loop >= 0; loop--)
    {
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
        GLuint defaultWidth  = 1 << loop;
        GLuint defaultHeight = 1 << loop;
        glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, defaultWidth);
        glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, defaultHeight);
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

        // Draw and check the FBO size
        validateSamplePass(query, defaultWidth, defaultHeight);
    }

    ASSERT_GL_NO_ERROR();
}

// Altering the default width and height of attachmentless framebuffer should work fine
TEST_P(FramebufferTest_ES31, ChangeFBOSizeWithNoAttachments)
{
    constexpr char kVS1[] = R"(#version 310 es
in layout(location = 0) highp vec2 a_position;
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
})";

    constexpr char kFS1[] = R"(#version 310 es
uniform layout(location = 0) highp ivec2 u_expectedSize;
out layout(location = 3) mediump vec4 f_color;
void main()
{
    if (ivec2(gl_FragCoord.xy) != u_expectedSize) discard;
    f_color = vec4(1.0, 0.5, 0.25, 1.0);
})";
    ANGLE_GL_PROGRAM(program1, kVS1, kFS1);
    glUseProgram(program1);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    const float data[] = {
        1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

    GLQuery query;

    // Test that:
    // 1. create a no-attachment framebuffer with size 1*1, draw
    // 2. change the no-attachment framebuffer size to 2*2, draw
    // works properly
    GLFramebuffer framebufferWithVariousSizeGrow;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferWithVariousSizeGrow);
    for (int loop = 0; loop < 2; loop++)
    {
        GLuint defaultWidth  = 1 << loop;
        GLuint defaultHeight = 1 << loop;
        glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, defaultWidth);
        glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, defaultHeight);
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

        // Draw and check the FBO size
        validateSamplePass(query, defaultWidth, defaultHeight);
    }

    ASSERT_GL_NO_ERROR();

    // Test that:
    // 1. create a no-attachment framebuffer with size 2*2, draw
    // 2. change the no-attachment framebuffer size to 1*1, draw
    // works properly
    GLFramebuffer framebufferWithVariousSizeShrink;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferWithVariousSizeShrink);
    for (int loop = 1; loop >= 0; loop--)
    {
        GLuint defaultWidth  = 1 << loop;
        GLuint defaultHeight = 1 << loop;
        glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, defaultWidth);
        glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, defaultHeight);
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

        // Draw and check the FBO size
        validateSamplePass(query, defaultWidth, defaultHeight);
    }

    ASSERT_GL_NO_ERROR();
}

// Test that changing framebuffer attachment count and its' default width and height work properly
TEST_P(FramebufferTest_ES31, ChangeFBOSizeAndAttachmentsCount)
{
    constexpr char kVS1[] = R"(#version 310 es
in layout(location = 0) highp vec2 a_position;
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
})";

    constexpr char kFS1[] = R"(#version 310 es
uniform layout(location = 0) highp ivec2 u_expectedSize;
out layout(location = 3) mediump vec4 f_color;
void main()
{
    if (ivec2(gl_FragCoord.xy) != u_expectedSize) discard;
    f_color = vec4(1.0, 0.5, 0.25, 1.0);
})";
    ANGLE_GL_PROGRAM(program1, kVS1, kFS1);
    glUseProgram(program1);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    const float data[] = {
        1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

    GLQuery query;

    // 1. create a no-attachment framebuffer with default size 1*1, draw
    // 2. give the fbo with 1 color attachment with size 2*2, draw
    // 3. change the fbo default size to 3*3, draw
    // 4. remove the fbo attachment, draw
    // works properly
    GLFramebuffer framebufferWithVariousSizeAndAttachmentGrow;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferWithVariousSizeAndAttachmentGrow);
    GLuint defaultWidth  = 1;
    GLuint defaultHeight = 1;
    glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, defaultWidth);
    glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, defaultHeight);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
    validateSamplePass(query, defaultWidth, defaultHeight);

    GLTexture mTexture;
    glBindTexture(GL_TEXTURE_2D, mTexture);
    GLuint attachmentWidth  = 2;
    GLuint attachmentHeight = 2;
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, attachmentWidth, attachmentHeight);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    validateSamplePass(query, attachmentWidth, attachmentWidth);

    defaultWidth  = 3;
    defaultHeight = 3;
    glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, defaultWidth);
    glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, defaultHeight);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
    validateSamplePass(query, attachmentWidth, attachmentHeight);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
    validateSamplePass(query, defaultWidth, defaultHeight);

    ASSERT_GL_NO_ERROR();

    // 1. create a no-attachment framebuffer with default size 3*3, draw
    // 2. give the fbo with 1 color attachment with size 2*2, draw
    // 3. change the fbo default size to 1*1, draw
    // 4. remove the fbo attachment, draw
    // works properly
    GLFramebuffer framebufferWithVariousSizeAndAttachmentShrink;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferWithVariousSizeAndAttachmentShrink);
    glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, defaultWidth);
    glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, defaultHeight);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
    validateSamplePass(query, defaultWidth, defaultHeight);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    validateSamplePass(query, attachmentWidth, attachmentHeight);

    defaultWidth  = 1;
    defaultHeight = 1;
    glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, defaultWidth);
    glFramebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, defaultHeight);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
    validateSamplePass(query, attachmentWidth, attachmentHeight);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
    validateSamplePass(query, defaultWidth, defaultHeight);

    ASSERT_GL_NO_ERROR();
}

// Test binding two textures with different widths to an FBO and drawing to it.
TEST_P(FramebufferTest_ES31, BindTexturesOfDifferentWidthsToFBOAndDraw)
{
    constexpr GLuint kSize = 2;

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::green);

    // The second texture is defined with double the width as the first.
    GLTexture colorTexture2;
    glBindTexture(GL_TEXTURE_2D, colorTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize * 2, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture2, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize * 2, kSize, GLColor::green);
}

// Test binding two textures with different heights to an FBO and drawing to it.
TEST_P(FramebufferTest_ES31, BindTexturesOfDifferentHeightsToFBOAndDraw)
{
    constexpr GLuint kSize = 2;

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::green);

    // The second texture is defined with double the height as the first.
    GLTexture colorTexture2;
    glBindTexture(GL_TEXTURE_2D, colorTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize * 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture2, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize * 2, GLColor::green);
}

// Test binding two textures with different formats to an FBO and drawing to it.
TEST_P(FramebufferTest_ES31, BindTexturesOfDifferentFormatsToFBOAndDraw)
{
    constexpr GLuint kSize = 2;

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::red);

    // The second texture is defined with R8 as its format.
    GLTexture colorTexture2;
    glBindTexture(GL_TEXTURE_2D, colorTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, kSize, kSize, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture2, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::red);
}

// Test binding two textures with different attachments to an FBO and drawing to it.
TEST_P(FramebufferTest_ES31, BindTexturesOfDifferentAttachmentsToFBOAndDraw)
{
    constexpr GLuint kSize = 2;

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::green);

    // The second texture has an additional depth attachment.
    GLTexture colorTexture2;
    glBindTexture(GL_TEXTURE_2D, colorTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture2, 0);

    GLRenderbuffer depthBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, kSize, kSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::green);
}

// Test binding two textures with different create flags to an FBO and drawing to it.
TEST_P(FramebufferTest_ES31, BindTexturesOfDifferentCreateFlagsToFBOAndDraw)
{
    constexpr GLuint kSize = 2;

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::green);

    // The second texture is defined as a cubemap, changing its create flags.
    GLTexture colorTexture2;
    glBindTexture(GL_TEXTURE_CUBE_MAP, colorTexture2);

    for (size_t faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex, 0, GL_RGBA8, kSize, kSize, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    for (size_t faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex, colorTexture2, 0);
    }
    ASSERT_GL_NO_ERROR();

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::green);
}

// Test binding two textures with different usage flags to an FBO and drawing to it.
TEST_P(FramebufferTest_ES31, BindTexturesOfDifferentUsageFlagsToFBOAndDraw)
{
    constexpr GLuint kSize = 2;

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::green);

    // The second texture is used in a compute shader before the original shader, adding the storage
    // bit to its usage bits.
    GLTexture colorTexture2;
    glBindTexture(GL_TEXTURE_2D, colorTexture2);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    ASSERT_GL_NO_ERROR();

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
uniform vec4 data;
layout(rgba8, binding = 0) writeonly uniform highp image2D image;

void main()
{
    imageStore(image, ivec2(gl_LocalInvocationID.xy), data);
})";

    GLuint computeProgram = CompileComputeProgram(kCS);
    ASSERT_NE(computeProgram, 0u);
    glUseProgram(computeProgram);

    glBindImageTexture(0, colorTexture2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    GLint uniformLoc = glGetUniformLocation(computeProgram, "data");
    ASSERT_NE(uniformLoc, -1);
    glUniform4f(uniformLoc, 0.0f, 0.0f, 1.0f, 1.0f);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture2, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::green);
}

// Test that clear with color masks on framebuffer texture with internal format GL_RGB5_A1 works
// This is a simplified version of below two deqp tests:
// KHR-GLES31.core.draw_buffers_indexed.color_masks
// KHR-GLES32.core.draw_buffers_indexed.color_masks
TEST_P(FramebufferTest_ES31, ClearWithColorMasksRGB5A1)
{
    constexpr int kSize  = 4;
    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    std::vector<GLenum> bufs(maxDrawBuffers);
    for (int i = 0; i < maxDrawBuffers; ++i)
    {
        bufs[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    // Specifies a list of color buffers to be drawn into
    glDrawBuffers(maxDrawBuffers, &bufs[0]);

    glDisable(GL_DITHER);

    // Attach textures with internal format GL_RGB5_A1 to each framebuffer color attachment
    std::vector<GLTexture> textures(maxDrawBuffers);
    std::vector<unsigned char> pixelData(kSize * kSize * 4, 255);
    for (int i = 0; i < maxDrawBuffers; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     pixelData.data());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i],
                               0);
    }

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear all buffers with clear color (0.15f, 0.3f, 0.45f, 0.6f)
    angle::Vector4 clearColor(0.15f, 0.3f, 0.45f, 0.6f);
    for (int i = 0; i < maxDrawBuffers; ++i)
    {
        glClearBufferfv(GL_COLOR, i, clearColor.data());
    }

    ASSERT_GL_NO_ERROR();

    // Set color masks
    for (int i = 0; i < maxDrawBuffers; ++i)
    {
        if (i % 4 == 0)
        {
            glColorMaski(i, GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
        }

        if (i % 4 == 1)
        {
            glColorMaski(i, GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE);
        }

        if (i % 4 == 2)
        {
            glColorMaski(i, GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
        }

        if (i % 4 == 3)
        {
            glColorMaski(i, GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
        }
    }

    ASSERT_GL_NO_ERROR();

    // Clear buffers again with a new clear color (0.85f, 0.85f, 0.85f, 0.85f)
    // Only the channel with color mask set to GL_TRUE is cleared with the new color.
    clearColor = {0.85f, 0.85f, 0.85f, 0.85f};
    for (int i = 0; i < maxDrawBuffers; ++i)
    {
        glClearBufferfv(GL_COLOR, i, &clearColor[0]);
    }

    ASSERT_GL_NO_ERROR();

    // Read and Verify
    // calculate the comparison epsilon based on the number of bits
    std::vector<int> bits        = {0, 0, 0, 0};
    std::vector<uint8_t> epsilon = {0, 0, 0, 0};
    for (int i = 0; i < 4; ++i)
    {
        glGetIntegerv(GL_RED_BITS + i, bits.data() + i);
        epsilon[i] =
            std::min(255u, static_cast<unsigned int>(
                               ceil(1.0 + 255.0 * (1.0 / pow(2.0, static_cast<double>(bits[i]))))));

        ASSERT(epsilon[i] >= 0 && epsilon[i] <= 255u);
    }

    std::vector<GLColor> rendered(kSize * kSize, GLColor::green);

    for (int i = 0; i < maxDrawBuffers; ++i)
    {
        std::vector<uint8_t> expected = {
            static_cast<uint8_t>(0.15f * 255), static_cast<uint8_t>(0.30f * 255),
            static_cast<uint8_t>(0.45f * 255), static_cast<uint8_t>(0.60f * 255)};
        expected[i % 4] = static_cast<uint8_t>(0.85f * 255);

        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        glReadPixels(0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE, rendered.data());

        for (int y = 0; y < kSize; ++y)
        {
            for (int x = 0; x < kSize; ++x)
            {
                GLColor readBackData = rendered[y * kSize + x];

                bool exactMatch = readBackData.R == expected[0] && readBackData.G == expected[1] &&
                                  readBackData.B == expected[2] && readBackData.A == expected[3];

                bool matchWithinEpsilon =
                    abs(static_cast<int>(readBackData.R) - static_cast<int>(expected[0])) <=
                        static_cast<int>(epsilon[0]) &&
                    abs(static_cast<int>(readBackData.G) - static_cast<int>(expected[1])) <=
                        static_cast<int>(epsilon[1]) &&
                    abs(static_cast<int>(readBackData.B) - static_cast<int>(expected[2])) <=
                        static_cast<int>(epsilon[2]) &&
                    abs(static_cast<int>(readBackData.A) - static_cast<int>(expected[3])) <=
                        static_cast<int>(epsilon[3]);

                ASSERT(exactMatch || matchWithinEpsilon);
            }
        }
    }

    // Set the framebuffer color mask back to default values
    for (int i = 0; i < maxDrawBuffers; ++i)
    {
        glColorMaski(i, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }

    ASSERT_GL_NO_ERROR();
}

void clearColorMorePrecisionThanFBOFormatNoDithering(const GLint &fboInternalFormat)
{
    constexpr int kSize = 4;
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glDisable(GL_DITHER);

    // Attach texture with internal format GL_RGB5_A1 to each framebuffer color attachment
    std::vector<unsigned char> pixelData(kSize * kSize * 4, 255);
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, fboInternalFormat, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixelData.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear FBO color attachment with clear color (0.15f, 0.3f, 0.45f, 0.6f)
    glClearColor(0.15f, 0.3f, 0.45f, 0.6f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Read and Verify
    // calculate the comparison epsilon based on the number of bits
    std::vector<int> bits        = {0, 0, 0, 0};
    std::vector<uint8_t> epsilon = {0, 0, 0, 0};
    for (int i = 0; i < 4; ++i)
    {
        glGetIntegerv(GL_RED_BITS + i, bits.data() + i);
        epsilon[i] =
            std::min(255u, static_cast<unsigned int>(
                               ceil(1.0 + 255.0 * (1.0 / pow(2.0, static_cast<double>(bits[i]))))));

        ASSERT(epsilon[i] >= 0 && epsilon[i] <= 255u);
    }

    std::vector<GLColor> rendered(kSize * kSize, GLColor::green);

    std::vector<uint8_t> expected = {
        static_cast<uint8_t>(0.15f * 255), static_cast<uint8_t>(0.30f * 255),
        static_cast<uint8_t>(0.45f * 255), static_cast<uint8_t>(0.60f * 255)};

    glReadPixels(0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE, rendered.data());
    ASSERT_GL_NO_ERROR();

    // Every pixel should have the same rendered result color. No dithering should be applied.
    GLColor clearedResultAtFirstPixel = rendered[0];

    for (int y = 0; y < kSize; ++y)
    {
        for (int x = 0; x < kSize; ++x)
        {
            GLColor readBackData = rendered[y * kSize + x];

            // Firstly check the rendered result is rational
            bool exactMatch = readBackData.R == expected[0] && readBackData.G == expected[1] &&
                              readBackData.B == expected[2] && readBackData.A == expected[3];

            bool matchWithinEpsilon =
                abs(static_cast<int>(readBackData.R) - static_cast<int>(expected[0])) <=
                    static_cast<int>(epsilon[0]) &&
                abs(static_cast<int>(readBackData.G) - static_cast<int>(expected[1])) <=
                    static_cast<int>(epsilon[1]) &&
                abs(static_cast<int>(readBackData.B) - static_cast<int>(expected[2])) <=
                    static_cast<int>(epsilon[2]) &&
                abs(static_cast<int>(readBackData.A) - static_cast<int>(expected[3])) <=
                    static_cast<int>(epsilon[3]);

            ASSERT(exactMatch || matchWithinEpsilon);

            // Secondly check no dithering is applied
            bool sameClearResultAsFirstPixel = readBackData.R == clearedResultAtFirstPixel.R &&
                                               readBackData.G == clearedResultAtFirstPixel.G &&
                                               readBackData.B == clearedResultAtFirstPixel.B &&
                                               readBackData.A == clearedResultAtFirstPixel.A;
            ASSERT(sameClearResultAsFirstPixel);
        }
    }
}

// Verify that when clear color has more precision than FBO color attachment format can hold,
// dithering is not automatically applied.
// https://issuetracker.google.com/292282210
TEST_P(FramebufferTest_ES31, ClearColorMorePrecisionThanFBOFormatShouldNotApplyDithering)
{
    clearColorMorePrecisionThanFBOFormatNoDithering(GL_RGB5_A1);
}

// Validates both MESA and standard functions can be used on OpenGL ES >=3.1
TEST_P(FramebufferTest_ES31, ValidateFramebufferFlipYMesaExtension)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    GLFramebuffer mFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
    ASSERT_GL_NO_ERROR();

    GLint flip_y = -1;

    glGetFramebufferParameterivMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, &flip_y);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(flip_y, 1);

    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 0);
    ASSERT_GL_NO_ERROR();

    flip_y = -1;
    glGetFramebufferParameterivMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, &flip_y);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(flip_y, 0);

    // Also using non-MESA functions should work.
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
    ASSERT_GL_NO_ERROR();

    flip_y = -1;
    glGetFramebufferParameteriv(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, &flip_y);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(flip_y, 1);

    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 0);
    ASSERT_GL_NO_ERROR();

    flip_y = -1;
    glGetFramebufferParameteriv(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, &flip_y);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(flip_y, 0);
}

class AddMockTextureNoRenderTargetTest : public ANGLETest<>
{
  public:
    AddMockTextureNoRenderTargetTest()
    {
        setWindowWidth(512);
        setWindowHeight(512);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Test to verify workaround succeeds when no program outputs exist http://anglebug.com/42260995
TEST_P(AddMockTextureNoRenderTargetTest, NoProgramOutputWorkaround)
{
    constexpr char kVS[] = "void main() {}";
    constexpr char kFS[] = "void main() {}";

    ANGLE_GL_PROGRAM(drawProgram, kVS, kFS);

    glUseProgram(drawProgram);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    ASSERT_GL_NO_ERROR();
}

// Covers a bug in ANGLE's Vulkan back-end framebuffer cache which ignored depth/stencil after
// calls to DrawBuffers.
TEST_P(FramebufferTest_ES3, AttachmentStateChange)
{
    constexpr GLuint kSize = 2;

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // First draw without a depth buffer.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    GLRenderbuffer depthBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, kSize, kSize);

    // Bind just a renderbuffer and draw.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glDrawBuffers(0, nullptr);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    // Re-enable color buffer and draw one final time. This previously triggered a crash.
    GLenum drawBuffs = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, &drawBuffs);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests that we can support a color texture also attached to the color attachment but
// with different LOD. From GLES3.0 spec section 4.4.3.2, if min_filter is GL_NEAREST_MIPMAP_NEAREST
// and the lod is within the [base_level, max_level] range, and it is possible to sample from a LOD
// that is rendering to then it does form a feedback loop. But if it is using textureLOD to
// explicitly fetching texture on different LOD, there is no loop and should still work. Aztec_ruins
// (https://issuetracker.google.com/175584609) is doing exactly this.
TEST_P(FramebufferTest_ES3, SampleFromAttachedTextureWithDifferentLOD)
{
    // TODO: https://anglebug.com/42264297
    ANGLE_SKIP_TEST_IF(IsD3D());

    constexpr GLuint kLevel0Size = 4;
    constexpr GLuint kLevel1Size = kLevel0Size / 2;
    constexpr GLuint kLevel2Size = kLevel1Size / 2;
    std::array<GLColor, kLevel0Size * kLevel0Size> gData;

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, kLevel0Size, kLevel0Size);
    gData.fill(GLColor::red);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kLevel0Size, kLevel0Size, GL_RGBA, GL_UNSIGNED_BYTE,
                    gData.data());
    gData.fill(GLColor::green);
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, kLevel1Size, kLevel1Size, GL_RGBA, GL_UNSIGNED_BYTE,
                    gData.data());
    gData.fill(GLColor::blue);
    glTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, kLevel2Size, kLevel2Size, GL_RGBA, GL_UNSIGNED_BYTE,
                    gData.data());

    // Attach level 1 to a FBO
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 1);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Render to FBO with color texture level 1 and textureLod from level 0.
    const GLenum discard[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discard);
    glViewport(0, 0, kLevel1Size, kLevel1Size);
    glScissor(0, 0, kLevel1Size, kLevel1Size);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Texture2DLod(), essl3_shaders::fs::Texture2DLod());
    glUseProgram(program);
    GLint textureLoc = glGetUniformLocation(program, essl3_shaders::Texture2DUniform());
    GLint lodLoc     = glGetUniformLocation(program, essl3_shaders::LodUniform());
    ASSERT_NE(-1, textureLoc);
    ASSERT_NE(-1, lodLoc);
    glUniform1i(textureLoc, 0);  // texture unit 0
    glUniform1f(lodLoc, 0);      // with Lod=0
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// This extends the test SampleFromAttachedTextureWithDifferentLOD by creating two renderpasses
// without changing texture binding. This is to make sure that sample/render to the same texture
// still function properly when transition from one renderpass to another without texture binding
// change.
TEST_P(FramebufferTest_ES3, SampleFromAttachedTextureWithDifferentLODAndFBOSwitch)
{
    // TODO: https://anglebug.com/42264297
    ANGLE_SKIP_TEST_IF(IsD3D());

    constexpr GLuint kLevel0Size = 4;
    constexpr GLuint kLevel1Size = kLevel0Size / 2;
    constexpr GLuint kLevel2Size = kLevel1Size / 2;
    std::array<GLColor, kLevel0Size * kLevel0Size> gData;

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, kLevel0Size, kLevel0Size);
    gData.fill(GLColor::red);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kLevel0Size, kLevel0Size, GL_RGBA, GL_UNSIGNED_BYTE,
                    gData.data());
    gData.fill(GLColor::green);
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, kLevel1Size, kLevel1Size, GL_RGBA, GL_UNSIGNED_BYTE,
                    gData.data());
    gData.fill(GLColor::blue);
    glTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, kLevel2Size, kLevel2Size, GL_RGBA, GL_UNSIGNED_BYTE,
                    gData.data());

    // Attach level 1 to two FBOs
    GLFramebuffer framebuffer1;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 1);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    GLFramebuffer framebuffer2;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 1);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Render to left half of FBO1 and textureLod from level 0.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    glViewport(0, 0, kLevel1Size / 2, kLevel1Size);
    glScissor(0, 0, kLevel1Size / 2, kLevel1Size);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Texture2DLod(), essl3_shaders::fs::Texture2DLod());
    glUseProgram(program);
    GLint textureLoc = glGetUniformLocation(program, essl3_shaders::Texture2DUniform());
    GLint lodLoc     = glGetUniformLocation(program, essl3_shaders::LodUniform());
    ASSERT_NE(-1, textureLoc);
    ASSERT_NE(-1, lodLoc);
    glUniform1i(textureLoc, 0);  // texture unit 0
    glUniform1f(lodLoc, 0);      // with Lod=0
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Render to right half of FBO2 and textureLod from level 0 without trigger texture binding
    // change.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    glViewport(kLevel1Size / 2, 0, kLevel1Size / 2, kLevel1Size);
    glScissor(kLevel1Size / 2, 0, kLevel1Size / 2, kLevel1Size);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kLevel1Size - 1, 0, GLColor::red);
}

// Test render to a texture level that is excluded from [base_level, max_level]. This specific test
// renders to an immutable texture at the level that is bigger than GL_TEXTURE_MAX_LEVEL. The
// texture itself has not been initialized with any data before rendering (TexSubImage call may
// initialize a VkImage object).
TEST_P(FramebufferTest_ES3, RenderAndInvalidateImmutableTextureWithBeyondMaxLevel)
{
    constexpr GLuint kLevel0Size = 4;
    constexpr GLuint kLevel1Size = kLevel0Size / 2;

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, kLevel0Size, kLevel0Size);
    // set max_level to 0
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    // Attach level 1 to a FBO
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 1);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Render to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    const GLenum discard[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discard);
    glViewport(0, 0, kLevel1Size, kLevel1Size);
    glScissor(0, 0, kLevel1Size, kLevel1Size);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glUseProgram(program);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test render to a texture level that is excluded from [base_level, max_level]. This specific test
// renders to an immutable texture at the level that is bigger than GL_TEXTURE_MAX_LEVEL. The
// texture itself has been initialized with data before rendering.
TEST_P(FramebufferTest_ES3, RenderAndInvalidateImmutableTextureWithSubImageWithBeyondMaxLevel)
{
    constexpr GLuint kLevel0Size = 4;
    constexpr GLuint kLevel1Size = kLevel0Size / 2;
    std::array<GLColor, kLevel0Size * kLevel0Size> gData;

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, kLevel0Size, kLevel0Size);
    // set max_level to 0
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    // Initialize with TexSubImage call
    gData.fill(GLColor::blue);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kLevel0Size, kLevel0Size, GL_RGBA, GL_UNSIGNED_BYTE,
                    gData.data());

    // Attach level 1 to a FBO
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 1);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Render to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    const GLenum discard[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discard);
    glViewport(0, 0, kLevel1Size, kLevel1Size);
    glScissor(0, 0, kLevel1Size, kLevel1Size);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glUseProgram(program);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test render to a texture level that is excluded from [base_level, max_level]. This specific test
// renders to an immutable texture at the level that is smaller than GL_TEXTURE_BASE_LEVEL. The
// texture itself has been initialized with data before rendering. Filament is using it this way
TEST_P(FramebufferTest_ES3, RenderAndInvalidateImmutableTextureWithBellowBaseLevelLOD)
{
    constexpr GLuint kLevel0Size = 4;
    constexpr GLuint kLevel1Size = kLevel0Size / 2;
    std::array<GLColor, kLevel0Size * kLevel0Size> gData;

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, kLevel0Size, kLevel0Size);
    // set base_level to 1
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    gData.fill(GLColor::blue);
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, kLevel1Size, kLevel1Size, GL_RGBA, GL_UNSIGNED_BYTE,
                    gData.data());

    // Attach level 0 to a FBO
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Render to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    const GLenum discard[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discard);
    glViewport(0, 0, kLevel0Size, kLevel0Size);
    glScissor(0, 0, kLevel0Size, kLevel0Size);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glUseProgram(program);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test render to a texture level that is excluded from [base_level, max_level]. This specific test
// renders to an immutable texture at the level that is bigger than GL_TEXTURE_MAX_LEVEL. The
// texture level that we render to has been initialized with data before rendering. This test if
// render to that level will get flush the level update even though it is outside [base, max]
// levels.
TEST_P(FramebufferTest_ES3, RenderImmutableTextureWithSubImageWithBeyondMaxLevel)
{
    // Set up program to sample from specific lod level.
    GLProgram textureLodProgram;
    textureLodProgram.makeRaster(essl3_shaders::vs::Texture2DLod(),
                                 essl3_shaders::fs::Texture2DLod());
    ASSERT(textureLodProgram.valid());
    glUseProgram(textureLodProgram);

    GLint textureLocation =
        glGetUniformLocation(textureLodProgram, essl3_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLocation);
    GLint lodLocation = glGetUniformLocation(textureLodProgram, essl3_shaders::LodUniform());
    ASSERT_NE(-1, lodLocation);

    constexpr GLuint kLevel0Size = 4;
    constexpr GLuint kLevel1Size = kLevel0Size / 2;
    std::array<GLColor, kLevel0Size * kLevel0Size> gData;

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, kLevel0Size, kLevel0Size);
    // Initialize level 0 with blue
    gData.fill(GLColor::blue);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kLevel0Size, kLevel0Size, GL_RGBA, GL_UNSIGNED_BYTE,
                    gData.data());
    // set max_level to 0
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    // Draw with level 0
    glUniform1f(lodLocation, 0);
    drawQuad(textureLodProgram, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Initalize level 1 with green
    gData.fill(GLColor::green);
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, kLevel1Size, kLevel1Size, GL_RGBA, GL_UNSIGNED_BYTE,
                    gData.data());
    // Attach level 1 to a FBO
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 1);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    // Render to FBO (i.e. level 1) with Red and blend with existing texture level data
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, kLevel1Size, kLevel1Size);
    glScissor(0, 0, kLevel1Size, kLevel1Size);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    ANGLE_GL_PROGRAM(redProgram, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glUseProgram(redProgram);
    drawQuad(redProgram, essl3_shaders::PositionAttrib(), 0.5f);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    ASSERT_GL_NO_ERROR();
    // Expect to see Red + Green, which is Yellow
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
}

// Similar to the other RenderImmutableTexture*** tests, but test on depth attachment instead of
// color attachment. This tests render to a depth texture level that is less than
// GL_TEXTURE_BASE_LEVEL and sample from it at the same time.
TEST_P(FramebufferTest_ES3, RenderSampleDepthTextureWithExcludedLevel)
{
    // Set up program to copy depth texture's value to color.red.
    constexpr char kVS[] = R"(precision mediump float;
attribute vec4 a_position;
varying vec2 v_texCoord;
void main()
{
    gl_Position = a_position;
    v_texCoord = a_position.xy * 0.5 + vec2(0.5);
})";
    constexpr char kFS[] = R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D depth;
void main()
{
    gl_FragColor = vec4(texture2D(depth, v_texCoord).x, 1, 0, 1);
})";
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    constexpr GLuint kLevel0Size = 4;
    constexpr GLuint kLevel1Size = kLevel0Size / 2;

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, kLevel0Size, kLevel0Size);

    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_DEPTH_COMPONENT32F, kLevel0Size, kLevel0Size);
    // Initialize level 1 with known depth value
    std::array<GLfloat, kLevel1Size * kLevel1Size> gData = {0.2, 0.4, 0.6, 0.8};
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, kLevel1Size, kLevel1Size, GL_DEPTH_COMPONENT, GL_FLOAT,
                    gData.data());
    // set base_level and max_level to 1, exclude level 0
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);

    // Attach level 0 to a FBO
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Render to FBO (LOD 0) with depth texture LOD 1
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, kLevel0Size, kLevel0Size);
    glScissor(0, 0, kLevel0Size, kLevel0Size);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(51u, 255u, 0, 255u), 1);
    EXPECT_PIXEL_COLOR_NEAR(kLevel0Size - 1, 0, GLColor(102u, 255u, 0, 255u), 1);
    EXPECT_PIXEL_COLOR_NEAR(0, kLevel0Size - 1, GLColor(153u, 255u, 0, 255u), 1);
    EXPECT_PIXEL_COLOR_NEAR(kLevel0Size - 1, kLevel0Size - 1, GLColor(204u, 255u, 0, 255u), 1);

    // Now check depth value is 0.5
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    glUseProgram(blueProgram);
    // should fail depth test
    drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.51f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(51u, 255u, 0, 255u), 1);
    // should pass depth test
    drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.49f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Covers a bug in ANGLE's Vulkan back-end. Our VkFramebuffer cache would in some cases forget to
// check the draw states when computing a cache key.
TEST_P(FramebufferTest_ES3, DisabledAttachmentRedefinition)
{
    constexpr GLuint kSize = 2;

    // Make a Framebuffer with two attachments with one enabled and one disabled.
    GLTexture texA, texB;
    glBindTexture(GL_TEXTURE_2D, texA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, texB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texA, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texB, 0);

    // Mask out the second texture.
    constexpr GLenum kOneDrawBuf = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &kOneDrawBuf);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Set up a very simple shader.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    glViewport(0, 0, kSize, kSize);

    // Draw
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Update the masked out attachment and draw again.
    std::vector<GLColor> redPixels(kSize * kSize, GLColor::red);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE,
                    redPixels.data());

    // Draw
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that changing the attachment of a framebuffer then sync'ing both READ and DRAW framebuffer
// (currently possible with glInvalidateFramebuffer) updates the scissor correctly.
TEST_P(FramebufferTest_ES3, ChangeAttachmentThenInvalidateAndDraw)
{
    constexpr GLsizei kSizeLarge = 32;
    constexpr GLsizei kSizeSmall = 16;

    GLTexture color1;
    glBindTexture(GL_TEXTURE_2D, color1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSizeSmall, kSizeSmall, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    GLTexture color2;
    glBindTexture(GL_TEXTURE_2D, color2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSizeLarge, kSizeLarge, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color1, 0);

    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    glViewport(0, 0, kSizeLarge, kSizeLarge);

    // Draw red into the framebuffer.
    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Change the attachment, invalidate it and draw green.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    std::array<GLenum, 1> attachments = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments.data());
    ASSERT_GL_NO_ERROR();

    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Validate the result.
    EXPECT_PIXEL_RECT_EQ(0, 0, kSizeLarge, kSizeLarge, GLColor::green);

    // Do the same, but changing from the large to small attachment.

    // Draw red into the framebuffer.
    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Change the attachment, invalidate it and draw blue.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color1, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments.data());

    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Validate the result.
    EXPECT_PIXEL_RECT_EQ(0, 0, kSizeSmall, kSizeSmall, GLColor::blue);
}

// Test Framebuffer object with two attachments that have unequal size. In OpenGLES3.0, this is
// a supported config. The common intersection area should be correctly rendered. The contents
// outside common intersection area are undefined.
TEST_P(FramebufferTest_ES3, AttachmentsWithUnequalDimensions)
{
    ANGLE_SKIP_TEST_IF(IsD3D());

    constexpr GLsizei kSizeLarge = 32;
    constexpr GLsizei kSizeSmall = 16;

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSizeLarge, kSizeSmall, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    GLRenderbuffer color;
    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSizeSmall, kSizeLarge);

    GLRenderbuffer depth;
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kSizeSmall, kSizeLarge);

    GLRenderbuffer stencil;
    glBindRenderbuffer(GL_RENDERBUFFER, stencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSizeSmall, kSizeLarge);

    struct
    {
        GLenum attachment;
        GLuint renderbuffer;
    } attachment2[4] = {{GL_COLOR_ATTACHMENT1, 0},
                        {GL_COLOR_ATTACHMENT1, color},
                        {GL_DEPTH_ATTACHMENT, depth},
                        {GL_STENCIL_ATTACHMENT, stencil}};
    for (int i = 0; i < 4; i++)
    {
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture,
                               0);
        if (attachment2[i].renderbuffer)
        {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment2[i].attachment, GL_RENDERBUFFER,
                                      attachment2[i].renderbuffer);
        }
        ASSERT_GL_NO_ERROR();
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        glUseProgram(drawColor);
        GLint colorUniformLocation =
            glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
        ASSERT_NE(colorUniformLocation, -1);

        glViewport(0, 0, kSizeLarge, kSizeLarge);
        const GLenum discard[] = {GL_COLOR_ATTACHMENT0};
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discard);

        // Draw red into the framebuffer.
        glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
        drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();

        // Validate the result. The intersected common area should be red now
        EXPECT_PIXEL_RECT_EQ(0, 0, kSizeSmall, kSizeSmall, GLColor::red);
    }
}

// Validates only MESA functions can be used on OpenGL ES <3.1
TEST_P(FramebufferTest_ES3, ValidateFramebufferFlipYMesaExtension)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    GLFramebuffer mFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
    ASSERT_GL_NO_ERROR();

    GLint flip_y = -1;

    glGetFramebufferParameterivMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, &flip_y);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(flip_y, 1);

    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 0);
    ASSERT_GL_NO_ERROR();

    flip_y = -1;
    glGetFramebufferParameterivMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, &flip_y);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(flip_y, 0);

    // Using non-MESA function should fail.
    glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 0);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    glGetFramebufferParameteriv(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, &flip_y);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

TEST_P(FramebufferTest_ES3, FramebufferFlipYMesaExtensionIncorrectPname)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    GLFramebuffer mFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, 1);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);
}

class FramebufferTest : public ANGLETest<>
{};

template <typename T>
void FillTexture2D(GLuint texture,
                   GLsizei width,
                   GLsizei height,
                   const T &onePixelData,
                   GLint level,
                   GLint internalFormat,
                   GLenum format,
                   GLenum type)
{
    std::vector<T> allPixelsData(width * height, onePixelData);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height, 0, format, type,
                 allPixelsData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

// Multi-context uses of textures should not cause rendering feedback loops.
TEST_P(FramebufferTest, MultiContextNoRenderingFeedbackLoops)
{
    constexpr char kTextureVS[] =
        R"(attribute vec4 a_position;
varying vec2 v_texCoord;
void main() {
    gl_Position = a_position;
    v_texCoord = (a_position.xy * 0.5) + 0.5;
})";

    constexpr char kTextureFS[] =
        R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D u_texture;
void main() {
    gl_FragColor = texture2D(u_texture, v_texCoord).rgba;
})";

    ANGLE_GL_PROGRAM(textureProgram, kTextureVS, kTextureFS);

    glUseProgram(textureProgram);
    GLint uniformLoc = glGetUniformLocation(textureProgram, "u_texture");
    ASSERT_NE(-1, uniformLoc);
    glUniform1i(uniformLoc, 0);

    GLTexture texture;
    FillTexture2D(texture, 1, 1, GLColor::red, 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Note that _texture_ is still bound to GL_TEXTURE_2D in this context at this point.

    EGLWindow *window          = getEGLWindow();
    EGLDisplay display         = window->getDisplay();
    EGLConfig config           = window->getConfig();
    EGLSurface surface         = window->getSurface();
    EGLint contextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION_KHR,
        GetParam().majorVersion,
        EGL_CONTEXT_MINOR_VERSION_KHR,
        GetParam().minorVersion,
        EGL_NONE,
    };
    EGLContext context1 = eglGetCurrentContext();
    // Create context2, sharing resources with context1.
    EGLContext context2 = eglCreateContext(display, config, context1, contextAttributes);
    ASSERT_NE(context2, EGL_NO_CONTEXT);
    eglMakeCurrent(display, surface, surface, context2);

    constexpr char kVS[] =
        R"(attribute vec4 a_position;
void main() {
    gl_Position = a_position;
})";

    constexpr char kFS[] =
        R"(precision mediump float;
void main() {
    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    ASSERT_GL_NO_ERROR();

    // Render to the texture in context2.
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // Texture is still a valid name in context2.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    // There is no rendering feedback loop at this point.

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    ASSERT_GL_NO_ERROR();

    // If draw is no-op'ed, texture will not be filled appropriately.
    drawQuad(program, "a_position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Make context1 current again.
    eglMakeCurrent(display, surface, surface, context1);

    // Render texture to screen.
    drawQuad(textureProgram, "a_position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    eglDestroyContext(display, context2);
}

// Ensure cube-incomplete attachments cause incomplete Framebuffers.
TEST_P(FramebufferTest, IncompleteCubeMap)
{
    constexpr GLuint kSize = 2;

    GLTexture srcTex;
    glBindTexture(GL_TEXTURE_CUBE_MAP, srcTex);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                           srcTex, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER),
                     GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
}

// Test FBOs with different sizes are drawn correctly
TEST_P(FramebufferTest, BindAndDrawDifferentSizedFBOs)
{
    // 1. Create FBO 1 with dimensions 16x16
    // 2. Draw red into FBO 1 (note, FramebufferVk::syncState is called)
    // 3. Create FBO 2 with dimensions 8x8
    // 4. Draw green into FBO 2 (note, FramebufferVk::syncState is called)
    // 5. Bind FBO 1 (note, it's not dirty)
    // 6. Draw blue into FBO 1
    // 7. Verify FBO 1 is entirely blue

    GLFramebuffer smallFbo;
    GLFramebuffer largeFbo;
    GLTexture smallTexture;
    GLTexture largeTexture;
    constexpr GLsizei kLargeWidth  = 16;
    constexpr GLsizei kLargeHeight = 16;
    constexpr GLsizei kSmallWidth  = 8;
    constexpr GLsizei kSmallHeight = 8;

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    // 1. Create FBO 1 with dimensions 16x16
    glBindFramebuffer(GL_FRAMEBUFFER, largeFbo);
    glBindTexture(GL_TEXTURE_2D, largeTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kLargeWidth, kLargeHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, largeTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // 2. Draw red into FBO 1 (note, FramebufferVk::syncState is called)
    glUseProgram(redProgram);
    drawQuad(redProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();

    // 3. Create FBO 2 with dimensions 8x8
    glBindFramebuffer(GL_FRAMEBUFFER, smallFbo);
    glBindTexture(GL_TEXTURE_2D, smallTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSmallWidth, kSmallHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smallTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // 4. Draw green into FBO 2 (note, FramebufferVk::syncState is called)
    glUseProgram(greenProgram);
    drawQuad(greenProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();

    // 5. Bind FBO 1 (note, it's not dirty)
    glBindFramebuffer(GL_FRAMEBUFFER, largeFbo);

    // 6. Draw blue into FBO 1
    glUseProgram(blueProgram);
    drawQuad(blueProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();

    // 7. Verify FBO 1 is entirely blue
    EXPECT_PIXEL_RECT_EQ(0, 0, kLargeWidth, kLargeHeight, GLColor::blue);
}

// Test FBOs with same attachments. Destroy one framebuffer should not affect the other framebuffer
// (chromium:1351170).
TEST_P(FramebufferTest_ES3, TwoFramebuffersWithSameAttachments)
{
    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glUseProgram(redProgram);

    GLRenderbuffer rb;
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);

    GLuint fbs[2];
    glGenFramebuffers(2, fbs);
    // Create fbos[0]
    glBindFramebuffer(GL_FRAMEBUFFER, fbs[0]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    const GLenum colorAttachment0 = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &colorAttachment0);
    // Create fbos[1] with same attachment as fbos[0]
    glBindFramebuffer(GL_FRAMEBUFFER, fbs[1]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &colorAttachment0);
    // Destroy fbos[0]
    glDeleteFramebuffers(1, &fbs[0]);
    // fbos[1] should still work, not crash.
    GLuint data;
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &data);
    drawQuad(redProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
}

// Regression test based on a fuzzer failure.  A crash was encountered in the following situation:
//
// - Texture bound as sampler with MAX_LEVEL 0
// - Framebuffer bound to level 0
// - Draw
// - Texture MAX_LEVEL changed to 1
// - Framebuffer bound to level 1
// - Draw
//
// Notes: Removing the first half removed the crash.  MIN_FILTERING of LINEAR vs
// LINEAR_MIPMAP_LINEAR did not make any changes.
TEST_P(FramebufferTest_ES3, FramebufferBindToNewLevelAfterMaxIncreaseShouldntCrash)
{
    constexpr char kFS[] = R"(precision mediump float;
uniform sampler2D u_tex0;
void main() {
    gl_FragColor = texture2D(u_tex0, vec2(0));
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), kFS);
    glUseProgram(program);

    GLTexture mutTex;
    glBindTexture(GL_TEXTURE_2D, mutTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 10, 10, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 5, 5, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    // Attempt a draw with level 0 (feedback loop)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mutTex, 0);
    glDrawArrays(GL_POINTS, 0, 1);

    // Attempt another draw with level 1.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mutTex, 1);

    // This shouldn't crash.
    glDrawArrays(GL_POINTS, 0, 1);
    ASSERT_GL_NO_ERROR();
}

// Modify renderbuffer attachment samples after bind
TEST_P(FramebufferTest_ES3, BindRenderbufferThenModifySamples)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLsizei size = 16;
    glViewport(0, 0, size, size);

    GLRenderbuffer color;
    glBindRenderbuffer(GL_RENDERBUFFER, color);

    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, size, size);

    glUniform4f(colorUniformLocation, 1, 0, 0, 1);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Modify renderbuffer attachment size after bind
TEST_P(FramebufferTest_ES3, BindRenderbufferThenModifySize)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLsizei size = 16;
    glViewport(0, 0, size, size);

    GLRenderbuffer color;
    glBindRenderbuffer(GL_RENDERBUFFER, color);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, size, size);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, size / 2, size * 2);

    glUniform4f(colorUniformLocation, 1, 0, 0, 1);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Tests redefining a layered framebuffer attachment.
TEST_P(FramebufferTest_ES3, RedefineLayerAttachment)
{
    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);
    std::vector<uint8_t> imgData(20480, 0);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, 8, 8, 8, 0, GL_RED, GL_UNSIGNED_BYTE, imgData.data());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, 8);
    glGenerateMipmap(GL_TEXTURE_3D);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, 16, 16, 16, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
                 imgData.data());
    glCopyTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 2, 2, 15, 16, 16);
    ASSERT_GL_NO_ERROR();
}

// Covers a bug when changing a base level of a texture bound to a FBO.
TEST_P(FramebufferTest_ES3, ReattachToInvalidBaseLevel)
{
    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(testProgram);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    for (int mip = 0; mip <= 2; ++mip)
    {
        int size = 10 >> mip;
        glTexImage2D(GL_TEXTURE_2D, mip, GL_RGBA8, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
    }

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 1);
    EXPECT_GL_NO_ERROR();

    // Set base level 1 and draw.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();
    // Set base level 0. The FBO is incomplete because the FBO attachment binds to level 1.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
}

// Ensure that clear color is correctly applied after invalidate
TEST_P(FramebufferTest_ES3, InvalidateClearDraw)
{
    constexpr GLsizei kSize = 2;

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear the image, and make sure the clear is flushed outside the render pass.
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Invalidate it such that the contents are marked as undefined. Note that
    // regardless of the marking, the image is cleared nevertheless.
    const GLenum discards[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);

    // Clear it again to the same color, and make sure the clear is flushed outside the render pass,
    // which may be optimized out.
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw with blend.  If the second clear is dropped and the image continues to be marked as
    // invalidated, loadOp=DONT_CARE would be used instead of loadOp=LOAD.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
}

// Produces VUID-VkImageMemoryBarrier-oldLayout-01197 VVL error with a "Render pass closed due to
// framebuffer change" command buffer label. As seen in Black Desert Mobile.
// The application draws 2 passes to produce the issue. First pass draws to a depth only frame
// buffer, the second one to a different color+depth frame buffer. The second pass samples the first
// passes frame buffer in two draw calls. First draw call samples it in the fragment stage, second
// in the the vertex stage.
TEST_P(FramebufferTest_ES3, FramebufferChangeTest)
{
    // Init depth frame buffer
    GLFramebuffer depthFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, depthFramebuffer);

    GLTexture depthAttachment;
    glBindTexture(GL_TEXTURE_2D, depthAttachment);
    // When using a color attachment instead, the issue does not occur.
    // The issue seems to occur for all GL_DEPTH_COMPONENT formats.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, kWidth, kHeight, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);

    // If filtering the depth attachment to GL_NEAREST is not set, the issue does not occur.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAttachment, 0);

    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT_GL_NO_ERROR();

    // Depth only pass
    {
        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
        glUseProgram(program);

        glClear(GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Color pass
    // The depth texture from the first pass is sampled from in both draw calls.
    // Skipping any of the two depth texture binds makes the issue not occur.
    // Changing the order of the draw calls makes the issue not occur.
    // This pass does not need to draw into a frame buffer.

    // Draw 1
    // The depth texture from the first pass is sampled from in the frament stage.
    {
        constexpr char kFS[] = {
            R"(#version 300 es
precision mediump float;

uniform mediump sampler2D samp;

layout(location = 0) out highp vec4 color;

void main()
{
    color = texture(samp, vec2(0));
})",
        };
        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
        glUseProgram(program);

        GLint textureLoc = glGetUniformLocation(program, "samp");
        glUniform1i(textureLoc, 1);

        // Skipping this bind makes the issue not occur
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthAttachment);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();
    }

    // Draw 2
    // Here the depth attachment from the first pass is used in the vertex stage. The VVL error
    // occurs in this draw. The sampler has to be attached to the vertex stage, otherwise the issue
    // does not occur.
    {
        constexpr char kVS[] = {
            R"(#version 300 es

uniform mediump sampler2D samp;

layout(location = 0) in mediump vec4 pos;

void main()
{
    gl_Position = pos + texture(samp, vec2(0));
})",
        };

        ANGLE_GL_PROGRAM(program, kVS, essl3_shaders::fs::Red());
        glUseProgram(program);

        GLint textureLoc = glGetUniformLocation(program, "samp");
        glUniform1i(textureLoc, 2);

        // Skipping this bind makes the issue not occur
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthAttachment);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();
    }
}

// Regression test for a bug in the Vulkan backend where the application produces a conditional
// framebuffer feedback loop which results in VUID-VkDescriptorImageInfo-imageLayout-00344 and
// VUID-vkCmdDraw-None-02699 (or VUID-vkCmdDrawIndexed-None-02699 when a different draw call is
// used). The application samples from the frame buffer it renders to depending on a uniform
// condition.
TEST_P(FramebufferTest_ES3, FramebufferConditionalFeedbackLoop)
{
    GLTexture colorAttachment;
    glBindTexture(GL_TEXTURE_2D, colorAttachment);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight);

    glActiveTexture(GL_TEXTURE13);
    glBindTexture(GL_TEXTURE_2D, colorAttachment);

    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment, 0);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    constexpr char kFS[] = {
        R"(#version 300 es
precision mediump float;

uniform mediump sampler2D samp;
uniform vec4 sampleCondition;
out vec4 color;

void main()
{
    if (sampleCondition.x > 0.0)
    {
        color = texture(samp, vec2(0.0));
    }
})",
    };

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint textureLoc = glGetUniformLocation(program, "samp");
    glUniform1i(textureLoc, 13);

    // This draw is required for the issue to occur. The application does multiple draws to
    // different framebuffers at this point, but drawing without a framebuffer bound also does
    // reproduce it.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // This draw triggers the issue.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
}

// Regression test for a bug in the Vulkan backend where sampling from a
// texture previously involved in a framebuffer feedback loop would produce
// VUID-VkDescriptorImageInfo-imageLayout-00344 and VUID-vkCmdDraw-None-02699
// because of an incorrect cached descriptor set.
TEST_P(FramebufferTest_ES3, FeedbackLoopTextureBindings)
{
    constexpr char kVS[] = R"(#version 300 es
precision highp float;
out vec2 texCoord;
const vec2 kVertices[4] = vec2[4](vec2(-1, -1), vec2(1, -1), vec2(-1, 1), vec2(1, 1));
void main()
{
    gl_Position = vec4(kVertices[gl_VertexID], 0.0, 1.0);
    texCoord = (kVertices[gl_VertexID] * 0.5) + 0.5;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform sampler2D sampler;
uniform int sampleCondition;
in vec2 texCoord;
out vec4 colorOut;
const vec4 kGreen = vec4(0, 1, 0, 1);
void main()
{
    if (sampleCondition == 0) {
        colorOut = kGreen;
    } else {
        colorOut = texture(sampler, texCoord);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    GLint sampleCondition = glGetUniformLocation(program, "sampleCondition");
    glUseProgram(program);

    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);

    // Render to tex with tex bound but not sampled
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    const GLenum buffers[]{GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, buffers);
    glUniform1i(sampleCondition, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ASSERT_GL_NO_ERROR();

    // Render to default framebuffer with tex bound and sampled
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUniform1i(sampleCondition, 1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ASSERT_GL_NO_ERROR();
}

// Tests change of framebuffer dimensions vs gl_FragCoord.
TEST_P(FramebufferTest_ES3, FramebufferDimensionsChangeAndFragCoord)
{
    constexpr char kVS[] = R"(#version 300 es
precision highp float;
uniform float height;
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, gl_VertexID % 2 == 0 ? -1 : 1, 1);
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 colorOut;
void main()
{
    float red = gl_FragCoord.x < 10. ? 1.0 : 0.0;
    float green = gl_FragCoord.y < 25. ? 1.0 : 0.0;
    colorOut = vec4(red, green, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    constexpr GLuint kWidth1       = 99;
    constexpr GLuint kHeight1      = 142;
    constexpr GLuint kWidth2       = 75;
    constexpr GLuint kHeight2      = 167;
    constexpr GLuint kRenderSplitX = 10;
    constexpr GLuint kRenderSplitY = 25;

    glViewport(0, 0, std::max(kWidth1, kWidth2), std::max(kHeight1, kHeight2));

    GLTexture tex1, tex2;
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth1, kHeight1);
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth2, kHeight2);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glUseProgram(program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Verify results
    EXPECT_PIXEL_RECT_EQ(0, 0, kRenderSplitX, kRenderSplitY, GLColor::yellow);
    EXPECT_PIXEL_RECT_EQ(0, kRenderSplitY, kRenderSplitX, kHeight1 - kRenderSplitY, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(kRenderSplitX, 0, kWidth1 - kRenderSplitX, kRenderSplitY, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(kRenderSplitX, kRenderSplitY, kWidth1 - kRenderSplitX,
                         kHeight1 - kRenderSplitY, GLColor::black);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Verify results
    EXPECT_PIXEL_RECT_EQ(0, 0, kRenderSplitX, kRenderSplitY, GLColor::yellow);
    EXPECT_PIXEL_RECT_EQ(0, kRenderSplitY, kRenderSplitX, kHeight2 - kRenderSplitY, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(kRenderSplitX, 0, kWidth2 - kRenderSplitX, kRenderSplitY, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(kRenderSplitX, kRenderSplitY, kWidth2 - kRenderSplitX,
                         kHeight2 - kRenderSplitY, GLColor::black);

    ASSERT_GL_NO_ERROR();
}

// Tests change of surface dimensions vs gl_FragCoord.
TEST_P(FramebufferTest_ES3, SurfaceDimensionsChangeAndFragCoord)
{
    constexpr char kVS[] = R"(#version 300 es
precision highp float;
uniform float height;
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, gl_VertexID % 2 == 0 ? -1 : 1, 1);
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 colorOut;
void main()
{
    float red = gl_FragCoord.x < 10. ? 1.0 : 0.0;
    float green = gl_FragCoord.y < 25. ? 1.0 : 0.0;
    colorOut = vec4(red, green, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    constexpr GLuint kWidth1       = 99;
    constexpr GLuint kHeight1      = 142;
    constexpr GLuint kWidth2       = 75;
    constexpr GLuint kHeight2      = 167;
    constexpr GLuint kRenderSplitX = 10;
    constexpr GLuint kRenderSplitY = 25;

    glViewport(0, 0, std::max(kWidth1, kWidth2), std::max(kHeight1, kHeight2));

    const bool isSwappedDimensions =
        getEGLWindow()->isFeatureEnabled(Feature::EmulatedPrerotation90) ||
        getEGLWindow()->isFeatureEnabled(Feature::EmulatedPrerotation270);

    auto resizeWindow = [this, isSwappedDimensions](GLuint width, GLuint height) {
        if (isSwappedDimensions)
        {
            getOSWindow()->resize(height, width);
        }
        else
        {
            getOSWindow()->resize(width, height);
        }
        swapBuffers();
    };

    resizeWindow(kWidth1, kHeight1);
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Verify results
    EXPECT_PIXEL_RECT_EQ(0, 0, kRenderSplitX, kRenderSplitY, GLColor::yellow);
    EXPECT_PIXEL_RECT_EQ(0, kRenderSplitY, kRenderSplitX, kHeight1 - kRenderSplitY, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(kRenderSplitX, 0, kWidth1 - kRenderSplitX, kRenderSplitY, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(kRenderSplitX, kRenderSplitY, kWidth1 - kRenderSplitX,
                         kHeight1 - kRenderSplitY, GLColor::black);

    resizeWindow(kWidth2, kHeight2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Verify results
    EXPECT_PIXEL_RECT_EQ(0, 0, kRenderSplitX, kRenderSplitY, GLColor::yellow);
    EXPECT_PIXEL_RECT_EQ(0, kRenderSplitY, kRenderSplitX, kHeight2 - kRenderSplitY, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(kRenderSplitX, 0, kWidth2 - kRenderSplitX, kRenderSplitY, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(kRenderSplitX, kRenderSplitY, kWidth2 - kRenderSplitX,
                         kHeight2 - kRenderSplitY, GLColor::black);

    // Reset window to original dimensions
    resizeWindow(kWidth, kHeight);

    ASSERT_GL_NO_ERROR();
}

// Tests blits between draw and read surfaces with different pre-rotation values.
TEST_P(FramebufferTest_ES3, BlitWithDifferentPreRotations)
{
    // TODO(anglebug.com/42266059): Untriaged bot failures with non-Vulkan backends
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    EGLWindow *window = getEGLWindow();
    ASSERT(window);
    EGLConfig config   = window->getConfig();
    EGLContext context = window->getContext();
    EGLDisplay dpy     = window->getDisplay();
    EGLint surfaceType = 0;

    // Skip if pbuffer surface is not supported
    eglGetConfigAttrib(dpy, config, EGL_SURFACE_TYPE, &surfaceType);
    ANGLE_SKIP_TEST_IF((surfaceType & EGL_PBUFFER_BIT) == 0);

    const EGLint surfaceWidth        = static_cast<EGLint>(getWindowWidth());
    const EGLint surfaceHeight       = static_cast<EGLint>(getWindowHeight());
    const EGLint pBufferAttributes[] = {
        EGL_WIDTH, surfaceWidth, EGL_HEIGHT, surfaceHeight, EGL_NONE,
    };

    // Create Pbuffer surface
    EGLSurface pbufferSurface = eglCreatePbufferSurface(dpy, config, pBufferAttributes);
    ASSERT_NE(pbufferSurface, EGL_NO_SURFACE);
    ASSERT_EGL_SUCCESS();

    EGLSurface windowSurface = window->getSurface();
    ASSERT_NE(windowSurface, EGL_NO_SURFACE);

    // Clear window surface with red color
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS();
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_RECT_EQ(0, 0, surfaceWidth, surfaceHeight, GLColor::red);

    // Blit from window surface to pbuffer surface and expect red color
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, pbufferSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS();

    glBlitFramebuffer(0, 0, surfaceWidth, surfaceHeight, 0, 0, surfaceWidth, surfaceHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, surfaceWidth, surfaceHeight, GLColor::red);

    // Clear pbuffer surface with blue color
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, pbufferSurface, pbufferSurface, context));
    ASSERT_EGL_SUCCESS();
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_RECT_EQ(0, 0, surfaceWidth, surfaceHeight, GLColor::blue);

    // Blit from pbuffer surface to window surface and expect blue color
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, windowSurface, pbufferSurface, context));
    ASSERT_EGL_SUCCESS();

    glBlitFramebuffer(0, 0, surfaceWidth, surfaceHeight, 0, 0, surfaceWidth, surfaceHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, surfaceWidth, surfaceHeight, GLColor::blue);

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS();

    EXPECT_EGL_TRUE(eglDestroySurface(dpy, pbufferSurface));
    ASSERT_EGL_SUCCESS();
}

// Tests draw to surfaces with different pre-rotation values.
TEST_P(FramebufferTest_ES3, DrawWithDifferentPreRotations)
{
    EGLWindow *window = getEGLWindow();
    ASSERT(window);
    EGLConfig config   = window->getConfig();
    EGLContext context = window->getContext();
    EGLDisplay dpy     = window->getDisplay();
    EGLint surfaceType = 0;

    // Skip if pbuffer surface is not supported
    eglGetConfigAttrib(dpy, config, EGL_SURFACE_TYPE, &surfaceType);
    ANGLE_SKIP_TEST_IF((surfaceType & EGL_PBUFFER_BIT) == 0);

    const EGLint surfaceWidth        = static_cast<EGLint>(getWindowWidth());
    const EGLint surfaceHeight       = static_cast<EGLint>(getWindowHeight());
    const EGLint pBufferAttributes[] = {
        EGL_WIDTH, surfaceWidth, EGL_HEIGHT, surfaceHeight, EGL_NONE,
    };

    // Create Pbuffer surface
    EGLSurface pbufferSurface = eglCreatePbufferSurface(dpy, config, pBufferAttributes);
    ASSERT_NE(pbufferSurface, EGL_NO_SURFACE);
    ASSERT_EGL_SUCCESS();

    EGLSurface windowSurface = window->getSurface();
    ASSERT_NE(windowSurface, EGL_NO_SURFACE);

    constexpr char kCheckered2FS[] = R"(precision highp float;
varying vec4 v_position;

void main()
{
    bool isLeft = v_position.x < 0.0;
    bool isTop = v_position.y < 0.0;
    if (isLeft)
    {
        if (isTop)
        {
            gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
        }
        else
        {
            gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);
        }
    }
    else
    {
        if (isTop)
        {
            gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
        }
        else
        {
            gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
    }
})";

    ANGLE_GL_PROGRAM(checkerProgram, essl1_shaders::vs::Passthrough(),
                     essl1_shaders::fs::Checkered());
    ANGLE_GL_PROGRAM(checkerProgram2, essl1_shaders::vs::Passthrough(), kCheckered2FS);

    // The test does the following:
    //
    // 1. draw checkered to window (rotated)
    // 2. draw checkered to pbuffer (not rotated)
    // 3. verify rendering to window, draw checkered2, verify again
    // 4. verify rendering to pbuffer, draw checkered2, verify again
    //
    // Step 2 ensures that the correct state is used after a change to the bound surface (from
    // rotated to not). Step 3 ensures the same from not rotated to rotated.  Step 4 is a redundant
    // check.

    // Step 1
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS();

    drawQuad(checkerProgram, essl1_shaders::PositionAttrib(), 0);

    // Step 2
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, pbufferSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS();

    drawQuad(checkerProgram, essl1_shaders::PositionAttrib(), 0);

    // Step 3
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, surfaceHeight - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(surfaceWidth - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(surfaceWidth - 1, surfaceHeight - 1, GLColor::yellow);

    drawQuad(checkerProgram2, essl1_shaders::PositionAttrib(), 0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(0, surfaceHeight - 1, GLColor::cyan);
    EXPECT_PIXEL_COLOR_EQ(surfaceWidth - 1, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(surfaceWidth - 1, surfaceHeight - 1, GLColor::white);

    // Step 4
    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, pbufferSurface, pbufferSurface, context));
    ASSERT_EGL_SUCCESS();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, surfaceHeight - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(surfaceWidth - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(surfaceWidth - 1, surfaceHeight - 1, GLColor::yellow);

    drawQuad(checkerProgram2, essl1_shaders::PositionAttrib(), 0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(0, surfaceHeight - 1, GLColor::cyan);
    EXPECT_PIXEL_COLOR_EQ(surfaceWidth - 1, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(surfaceWidth - 1, surfaceHeight - 1, GLColor::white);

    EXPECT_EGL_TRUE(eglMakeCurrent(dpy, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS();

    EXPECT_EGL_TRUE(eglDestroySurface(dpy, pbufferSurface));
    ASSERT_EGL_SUCCESS();
}

class FramebufferExtensionsTest : public FramebufferTest
{
  protected:
    FramebufferExtensionsTest() { setExtensionsEnabled(false); }

    void checkParameter(GLenum expectedComponentType)
    {
        GLint componentType = 0;
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                              GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE_EXT,
                                              &componentType);
        EXPECT_EQ(componentType, static_cast<GLint>(expectedComponentType));
        if (expectedComponentType)
        {
            EXPECT_GL_NO_ERROR();
        }
        else
        {
            EXPECT_GL_ERROR(GL_INVALID_ENUM);
        }
    }

    void checkTexture(GLenum format, GLenum type, GLenum expectedComponentType)
    {
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, format, 8, 8, 0, format, type, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        ASSERT_GL_NO_ERROR();
        checkParameter(expectedComponentType);
    }

    void checkRenderbuffer(GLenum format, GLenum expectedComponentType)
    {
        GLRenderbuffer renderbuffer;
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, format, 8, 8);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  renderbuffer);
        ASSERT_GL_NO_ERROR();
        checkParameter(expectedComponentType);
    }

    void test(const char *extensionName, GLenum format, bool supportsRenderbuffer)
    {
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        checkTexture(GL_RGBA, GL_UNSIGNED_BYTE, 0);
        checkRenderbuffer(GL_RGB565, 0);

        ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled(extensionName));

        checkTexture(GL_RGBA, GL_UNSIGNED_BYTE, GL_UNSIGNED_NORMALIZED_EXT);
        checkRenderbuffer(GL_RGB565, GL_UNSIGNED_NORMALIZED_EXT);

        if (supportsRenderbuffer)
            checkRenderbuffer(format, GL_FLOAT);
    }
};

// Tests that GL_EXT_color_buffer_half_float enables component type state queries on
// framebuffer attachments.
TEST_P(FramebufferExtensionsTest, ColorBufferHalfFloat)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_texture_half_float"));
    test("GL_EXT_color_buffer_half_float", GL_RGBA16F_EXT, true);
}

// Tests that GL_CHROMIUM_color_buffer_float_rgb enables component type state queries on
// framebuffer attachments.
TEST_P(FramebufferExtensionsTest, ColorBufferFloatRgb)
{
    test("GL_CHROMIUM_color_buffer_float_rgb", GL_RGB32F_EXT, false);
}

// Tests that GL_CHROMIUM_color_buffer_float_rgba enables component type state queries on
// framebuffer attachments.
TEST_P(FramebufferExtensionsTest, ColorBufferFloatRgba)
{
    test("GL_CHROMIUM_color_buffer_float_rgba", GL_RGBA32F_EXT, true);
}

class DefaultFramebufferTest : public ANGLETest<>
{
  protected:
    DefaultFramebufferTest()
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
    }

    static constexpr GLsizei kWidth  = 16;
    static constexpr GLsizei kHeight = 16;
};

// glReadPixel from default FBO with format and type retrieved from
// GL_IMPLEMENTATION_COLOR_READ_FORMAT and GL_IMPLEMENTATION_COLOR_READ_TYPE
// should work
TEST_P(DefaultFramebufferTest, ReadFromDefaultFBOOnDefaultEGLWindowSurface)
{
    // Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create shader programs
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    constexpr char kVS1[] = R"(#version 300 es
in highp vec2 a_position;
in highp vec2 a_texcoord;
out highp vec2 texcoord;
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
    texcoord = a_texcoord;
})";

    constexpr char kFS1[] = R"(#version 300 es
precision highp float;
in highp vec2 texcoord;
out highp vec4 fragColor;
uniform highp sampler2D texSampler;

void main()
{
    fragColor = texture(texSampler, texcoord);
})";

    ANGLE_GL_PROGRAM(program, kVS1, kFS1);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    // Create Vertex data
    const std::vector<float> positions = {-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f};
    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), positions.data(),
                 GL_STATIC_DRAW);
    GLint vertexPosLocation = glGetAttribLocation(program, "a_position");
    ASSERT_NE(vertexPosLocation, -1);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(vertexPosLocation);
    glVertexAttribPointer(vertexPosLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);

    const std::vector<float> texcoords = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    GLBuffer texcoordBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords[0]) * texcoords.size(), texcoords.data(),
                 GL_STATIC_DRAW);
    GLint texCoordLocation = glGetAttribLocation(program, "a_texcoord");
    ASSERT_NE(texCoordLocation, -1);
    glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer);
    glEnableVertexAttribArray(texCoordLocation);
    glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);

    const std::vector<uint16_t> quadIndices = {0, 1, 2, 2, 1, 3};
    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices[0]) * quadIndices.size(),
                 quadIndices.data(), GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    // Create Texture
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    std::vector<uint8_t> texData;

    constexpr size_t width               = 4;
    constexpr size_t height              = 4;
    constexpr size_t bytePerColorChannel = 4;
    constexpr uint8_t texColorPerChannel = 125;

    texData.resize(width * height * bytePerColorChannel);

    for (size_t i = 0; i < width * height; ++i)
    {
        texData.push_back(texColorPerChannel);
        texData.push_back(texColorPerChannel);
        texData.push_back(texColorPerChannel);
        texData.push_back(texColorPerChannel);
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texData.data());
    ASSERT_GL_NO_ERROR();

    // Initialize uniform values
    GLint uniformTextureSamplerLocation = glGetUniformLocation(program, "texSampler");
    glUniform1i(uniformTextureSamplerLocation, 0);
    ASSERT_GL_NO_ERROR();

    // Disable Dither
    glDisable(GL_DITHER);

    // Draw quad
    glDrawElements(GL_TRIANGLES, quadIndices.size(), GL_UNSIGNED_BYTE, 0);

    // Get glReadPixel format and type
    GLint readFormat;
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &readFormat);

    GLint readType;
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &readType);

    // Read Pixel with glReadPixel
    std::vector<uint8_t> renderResult;
    renderResult.resize(width * height * 4);
    glReadPixels(0, 0, width, height, readFormat, readType, renderResult.data());

    // glReadPixel with format and type retrieved from
    // GL_IMPLEMENTATION_COLOR_READ_FORMAT &
    // GL_IMPLEMENTATION_COLOR_READ_TYPE
    // should not trigger errors
    ASSERT_GL_NO_ERROR();
}

// Test resolving the same framebuffer into two different ones
TEST_P(FramebufferTest_ES31, MultisampleResolveMultipleTimes)
{
    constexpr int kWidth  = 16;
    constexpr int kHeight = 20;
    glViewport(0, 0, kWidth, kHeight);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Create two resolve FBOs and textures. Use different texture levels and layers.
    GLTexture resolveTexture1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, kWidth * 2, kHeight * 2);

    GLFramebuffer resolveFBO1;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture1, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture resolveTexture2;
    glBindTexture(GL_TEXTURE_2D_ARRAY, resolveTexture2);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, GL_RGBA8, kWidth * 4, kHeight * 4, 5);

    GLFramebuffer resolveFBO2;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, resolveTexture2, 2, 3);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO2);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    auto verify = [](GLuint fbo) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        constexpr uint8_t kWidthHalfPixelGradient  = 256 / kWidth / 2;
        constexpr uint8_t kHeightHalfPixelGradient = 256 / kHeight / 2;
        EXPECT_PIXEL_NEAR(0, 0, kWidthHalfPixelGradient, kHeightHalfPixelGradient, 0, 255, 1.0);
        EXPECT_PIXEL_NEAR(kWidth - 1, 0, 255 - kWidthHalfPixelGradient, kHeightHalfPixelGradient, 0,
                          255, 1.0);
        EXPECT_PIXEL_NEAR(0, kHeight - 1, kWidthHalfPixelGradient, 255 - kHeightHalfPixelGradient,
                          0, 255, 1.0);
        EXPECT_PIXEL_NEAR(kWidth - 1, kHeight - 1, 255 - kWidthHalfPixelGradient,
                          255 - kHeightHalfPixelGradient, 0, 255, 1.0);
    };

    verify(resolveFBO1);
    verify(resolveFBO2);
    ASSERT_GL_NO_ERROR();
}

// Test resolving the same depth/stencil attachment into two different framebuffers
TEST_P(FramebufferTest_ES31, MultisampleDepthStencilResolveMultipleTimes)
{
    enum class DepthStencilResolve
    {
        Simultaneous,
        SeparateAspectsButSameFramebuffer,
        SeparateAspectsDifferentFramebuffers,
    };

    constexpr int kWidth  = 24;
    constexpr int kHeight = 12;
    glViewport(0, 0, kWidth, kHeight);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Create two resolve FBOs and textures. Use different texture levels and layers.
    GLTexture resolveTexture1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_DEPTH24_STENCIL8, kWidth * 4, kHeight * 4);

    GLFramebuffer resolveFBO1;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           resolveTexture1, 2);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture resolveTexture2;
    glBindTexture(GL_TEXTURE_2D, resolveTexture2);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_DEPTH24_STENCIL8, kWidth * 2, kHeight * 2);

    GLFramebuffer resolveFBO2;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           resolveTexture2, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(red, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Red());

    auto runTest = [&](float depth, int stencil, DepthStencilResolve resolve) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
        glDepthMask(GL_TRUE);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, stencil, 0xFF);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glStencilMask(0xFF);

        // Initialize the depth/stencil image
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        drawQuad(red, essl1_shaders::PositionAttrib(), depth);
        ASSERT_GL_NO_ERROR();

        // Resolve depth and stencil, then verify the results
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
        switch (resolve)
        {
            case DepthStencilResolve::Simultaneous:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspectsButSameFramebuffer:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                                  GL_NEAREST);
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspectsDifferentFramebuffers:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                                  GL_NEAREST);
                break;
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO2);
        switch (resolve)
        {
            case DepthStencilResolve::Simultaneous:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspectsButSameFramebuffer:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                                  GL_NEAREST);
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspectsDifferentFramebuffers:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
        }
        ASSERT_GL_NO_ERROR();

        verifyDepth(resolveFBO1, kWidth, kHeight, depth);
        if (resolve != DepthStencilResolve::SeparateAspectsDifferentFramebuffers)
        {
            verifyStencil(resolveFBO1, kWidth, kHeight, stencil);
            verifyDepth(resolveFBO2, kWidth, kHeight, depth);
        }
        verifyStencil(resolveFBO2, kWidth, kHeight, stencil);
    };

    runTest(0.8f, 0x55, DepthStencilResolve::Simultaneous);
    runTest(0.2f, 0x3A, DepthStencilResolve::SeparateAspectsButSameFramebuffer);
    runTest(0.5f, 0x98, DepthStencilResolve::SeparateAspectsDifferentFramebuffers);
    ASSERT_GL_NO_ERROR();
}

// Test resolving the same framebuffer into two different ones with a draw in between
TEST_P(FramebufferTest_ES31, MultisampleResolveMultipleTimesWithDrawInBetween)
{
    constexpr int kWidth  = 16;
    constexpr int kHeight = 20;
    glViewport(0, 0, kWidth, kHeight);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Create two resolve FBOs and textures. Use different texture levels and layers.
    GLTexture resolveTexture1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, kWidth * 2, kHeight * 2);

    GLFramebuffer resolveFBO1;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture1, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture resolveTexture2;
    glBindTexture(GL_TEXTURE_2D_ARRAY, resolveTexture2);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, GL_RGBA8, kWidth * 4, kHeight * 4, 5);

    GLFramebuffer resolveFBO2;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, resolveTexture2, 2, 3);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
    ANGLE_GL_PROGRAM(redProgram, essl31_shaders::vs::Passthrough(), essl31_shaders::fs::Red());
    drawQuad(redProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO2);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO1);
    constexpr uint8_t kWidthHalfPixelGradient  = 256 / kWidth / 2;
    constexpr uint8_t kHeightHalfPixelGradient = 256 / kHeight / 2;
    EXPECT_PIXEL_NEAR(0, 0, kWidthHalfPixelGradient, kHeightHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kWidth - 1, 0, 255 - kWidthHalfPixelGradient, kHeightHalfPixelGradient, 0,
                      255, 1.0);
    EXPECT_PIXEL_NEAR(0, kHeight - 1, kWidthHalfPixelGradient, 255 - kHeightHalfPixelGradient, 0,
                      255, 1.0);
    EXPECT_PIXEL_NEAR(kWidth - 1, kHeight - 1, 255 - kWidthHalfPixelGradient,
                      255 - kHeightHalfPixelGradient, 0, 255, 1.0);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO2);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Test resolving the same depth/stencil framebuffer into two different ones with a draw in between
TEST_P(FramebufferTest_ES31, MultisampleDepthStencilResolveMultipleTimesWithDrawInBetween)
{
    enum class DepthStencilResolve
    {
        Simultaneous,
        SeparateAspectsButSameFramebuffer,
        SeparateAspectsDifferentFramebuffers,
    };

    constexpr int kWidth  = 16;
    constexpr int kHeight = 20;
    glViewport(0, 0, kWidth, kHeight);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Create two resolve FBOs and textures. Use different texture levels and layers.
    GLTexture resolveTexture1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_DEPTH24_STENCIL8, kWidth * 4, kHeight * 4);

    GLFramebuffer resolveFBO1;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           resolveTexture1, 2);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture resolveTexture2;
    glBindTexture(GL_TEXTURE_2D, resolveTexture2);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_DEPTH24_STENCIL8, kWidth * 2, kHeight * 2);

    GLFramebuffer resolveFBO2;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           resolveTexture2, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(red, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Red());

    auto runTest = [&](float depth1, int stencil1, float depth2, int stencil2,
                       DepthStencilResolve resolve) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
        glDepthMask(GL_TRUE);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, stencil1, 0xFF);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glStencilMask(0xFF);

        // Initialize the depth/stencil image
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        drawQuad(red, essl1_shaders::PositionAttrib(), depth1);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
        switch (resolve)
        {
            case DepthStencilResolve::Simultaneous:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspectsButSameFramebuffer:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                                  GL_NEAREST);
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspectsDifferentFramebuffers:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                                  GL_NEAREST);
                break;
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
        glStencilFunc(GL_ALWAYS, stencil2, 0xFF);
        drawQuad(red, essl1_shaders::PositionAttrib(), depth2);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO2);
        switch (resolve)
        {
            case DepthStencilResolve::Simultaneous:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspectsButSameFramebuffer:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                                  GL_NEAREST);
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspectsDifferentFramebuffers:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
        }
        ASSERT_GL_NO_ERROR();

        verifyDepth(resolveFBO1, kWidth, kHeight, depth1);
        if (resolve != DepthStencilResolve::SeparateAspectsDifferentFramebuffers)
        {
            verifyStencil(resolveFBO1, kWidth, kHeight, stencil1);
            verifyDepth(resolveFBO2, kWidth, kHeight, depth2);
        }
        verifyStencil(resolveFBO2, kWidth, kHeight, stencil2);
    };

    runTest(0.4f, 0x3F, 0.1f, 0xA2, DepthStencilResolve::Simultaneous);
    runTest(0.9f, 0x2B, 0.3f, 0xDD, DepthStencilResolve::SeparateAspectsButSameFramebuffer);
    runTest(0.5f, 0x6C, 0.6f, 0x7E, DepthStencilResolve::SeparateAspectsDifferentFramebuffers);
    ASSERT_GL_NO_ERROR();
}

// Test resolving different attachments of an FBO to separate FBOs
TEST_P(FramebufferTest_ES31, MultisampleResolveBothAttachments)
{
    enum class Invalidate
    {
        None,
        AfterEachResolve,
        AllAtEnd,
    };

    constexpr char kFS[] = R"(#version 300 es
precision highp float;

uniform vec4 value0;
uniform vec4 value2;

layout(location = 0) out vec4 color0;
layout(location = 2) out vec4 color2;

void main()
{
    color0 = value0;
    color2 = value2;
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    const GLint color0Loc = glGetUniformLocation(program, "value0");
    const GLint color1Loc = glGetUniformLocation(program, "value2");

    constexpr int kWidth  = 16;
    constexpr int kHeight = 20;
    glViewport(0, 0, kWidth, kHeight);

    GLTexture msaa0, msaa1;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa1);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaa0,
                           0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, msaa1,
                           0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLenum bufs[3] = {GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, bufs);

    // Create two resolve FBOs and textures. Use different texture levels and layers.
    GLTexture resolveTexture1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, kWidth * 2, kHeight * 2);

    GLFramebuffer resolveFBO1;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture1, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture resolveTexture2;
    glBindTexture(GL_TEXTURE_2D_ARRAY, resolveTexture2);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, GL_RGBA8, kWidth * 4, kHeight * 4, 5);

    GLFramebuffer resolveFBO2;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, resolveTexture2, 2, 3);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    auto test = [&](GLColor color0, GLColor color1, Invalidate invalidate) {
        const GLenum discards[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT2};

        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        glUniform4fv(color0Loc, 1, color0.toNormalizedVector().data());
        glUniform4fv(color1Loc, 1, color1.toNormalizedVector().data());
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_NO_ERROR();

        // Resolve the first attachment
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        if (invalidate == Invalidate::AfterEachResolve)
        {
            glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, discards);
        }

        // Resolve the second attachment
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO2);
        glReadBuffer(GL_COLOR_ATTACHMENT2);
        glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        if (invalidate == Invalidate::AfterEachResolve)
        {
            glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, discards + 1);
        }
        else if (invalidate == Invalidate::AllAtEnd)
        {
            glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 2, discards);
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO1);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, color0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO2);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, color1);
        ASSERT_GL_NO_ERROR();
    };

    test(GLColor::red, GLColor::green, Invalidate::None);
    test(GLColor::blue, GLColor::yellow, Invalidate::AfterEachResolve);
    test(GLColor::cyan, GLColor::magenta, Invalidate::AllAtEnd);
}

// Test resolving a framebuffer once, then drawing to it again without a resolve.  Makes sure there
// is no caching bug that would make the second render pass resolve into the old resolve target
// again.
TEST_P(FramebufferTest_ES31, ResolveThenDrawWithoutResolve)
{
    enum class Invalidate
    {
        None,
        AfterFirstResolve,
        AfterEachResolve,
        AtEnd,
    };

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorLoc = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorLoc, -1);

    constexpr int kWidth  = 16;
    constexpr int kHeight = 20;
    glViewport(0, 0, kWidth, kHeight);

    GLTexture msaa;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaa,
                           0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Create the resolve FBO and texture.
    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, kWidth * 2 + 1, kHeight * 2 + 1);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    auto test = [&](GLColor color1, GLColor color2, Invalidate invalidate) {
        const GLenum discards[] = {GL_COLOR_ATTACHMENT0};

        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        glUniform4fv(colorLoc, 1, color1.toNormalizedVector().data());
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_NO_ERROR();

        // Resolve
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
        glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        if (invalidate == Invalidate::AfterEachResolve ||
            invalidate == Invalidate::AfterFirstResolve)
        {
            glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, discards);
        }

        // Draw again, but don't resolve.
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
        glUniform4fv(colorLoc, 1, color2.toNormalizedVector().data());
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);

        const bool invalidateAtEnd =
            invalidate == Invalidate::AfterEachResolve || invalidate == Invalidate::AtEnd;
        if (invalidateAtEnd)
        {
            glInvalidateFramebuffer(GL_DRAW_FRAMEBUFFER, 1, discards);
        }

        // Make sure the render pass is flushed so if there's a caching bug and the old render pass
        // with resolve is used for the second render pass, the contents of the resolve attachment
        // is wrong.  Can't rely on glReadPixels doing that because of potential use of
        // VK_EXT_host_image_copy.
        glFinish();

        // Verify the contents of the resolve attachment
        glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, color1);

        if (!invalidateAtEnd)
        {
            // For completeness, make sure the second draw succeeded.
            glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
            glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);
            ASSERT_GL_NO_ERROR();

            glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
            EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, color2);
        }
        ASSERT_GL_NO_ERROR();
    };

    test(GLColor::red, GLColor::green, Invalidate::None);
    test(GLColor::blue, GLColor::yellow, Invalidate::AfterFirstResolve);
    test(GLColor::cyan, GLColor::magenta, Invalidate::AfterEachResolve);
    test(GLColor::white, GLColor::red, Invalidate::AtEnd);
}

// Test resolving a depth/stencil framebuffer once, then drawing to it again without a resolve.
// Makes sure there is no caching bug that would make the second render pass resolve into the old
// resolve target again.
TEST_P(FramebufferTest_ES31, DepthStencilResolveThenDrawWithoutResolve)
{
    enum class Invalidate
    {
        None,
        AfterFirstResolve,
        AfterEachResolve,
        AtEnd,
    };
    enum class DepthStencilResolve
    {
        Simultaneous,
        SeparateAspects,
    };

    ANGLE_GL_PROGRAM(red, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Red());

    constexpr int kWidth  = 16;
    constexpr int kHeight = 20;
    glViewport(0, 0, kWidth, kHeight);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Create the resolve FBO and texture. Use different texture levels and layers.
    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_DEPTH24_STENCIL8, kWidth * 4, kHeight * 4);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           resolveTexture, 2);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    auto test = [&](float depth1, int stencil1, float depth2, int stencil2,
                    DepthStencilResolve resolve, Invalidate invalidate) {
        const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT,
                                   GL_DEPTH_STENCIL_ATTACHMENT};

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
        glDepthMask(GL_TRUE);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, stencil1, 0xFF);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glStencilMask(0xFF);

        // First draw
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        drawQuad(red, essl1_shaders::PositionAttrib(), depth1);
        ASSERT_GL_NO_ERROR();

        // Resolve
        const bool invalidateAfterFirstResolve = invalidate == Invalidate::AfterEachResolve ||
                                                 invalidate == Invalidate::AfterFirstResolve;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
        switch (resolve)
        {
            case DepthStencilResolve::Simultaneous:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                if (invalidateAfterFirstResolve)
                {
                    glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, &discards[2]);
                }
                break;
            case DepthStencilResolve::SeparateAspects:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                                  GL_NEAREST);
                if (invalidateAfterFirstResolve)
                {
                    glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, &discards[0]);
                }
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                if (invalidateAfterFirstResolve)
                {
                    glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, &discards[1]);
                }
                break;
        }
        ASSERT_GL_NO_ERROR();

        // Draw again, but don't resolve.
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
        glStencilFunc(GL_ALWAYS, stencil2, 0xFF);
        drawQuad(red, essl1_shaders::PositionAttrib(), depth2);

        const bool invalidateAtEnd =
            invalidate == Invalidate::AfterEachResolve || invalidate == Invalidate::AtEnd;
        if (invalidateAtEnd)
        {
            glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, &discards[2]);
        }

        // Make sure the render pass is flushed so if there's a caching bug and the old render pass
        // with resolve is used for the second render pass, the contents of the resolve attachment
        // is wrong.  Can't rely on glReadPixels doing that because of potential use of
        // VK_EXT_host_image_copy.
        glFinish();

        // Verify the contents of the resolve attachment
        verifyDepth(resolveFBO, kWidth, kHeight, depth1);
        verifyStencil(resolveFBO, kWidth, kHeight, stencil1);

        if (!invalidateAtEnd)
        {
            // For completeness, make sure the second draw succeeded.
            glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
            glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                              GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
            ASSERT_GL_NO_ERROR();

            verifyDepth(resolveFBO, kWidth, kHeight, depth2);
            verifyStencil(resolveFBO, kWidth, kHeight, stencil2);
        }
        ASSERT_GL_NO_ERROR();
    };

    test(0.4f, 0x3F, 0.1f, 0xA2, DepthStencilResolve::Simultaneous, Invalidate::None);
    test(0.9f, 0x2B, 0.3f, 0xDD, DepthStencilResolve::Simultaneous, Invalidate::AfterFirstResolve);
    test(0.5f, 0x6C, 0.6f, 0x7E, DepthStencilResolve::Simultaneous, Invalidate::AfterEachResolve);
    test(0.1f, 0x78, 0.4f, 0x34, DepthStencilResolve::Simultaneous, Invalidate::AtEnd);
    test(0.6f, 0x7E, 0.5f, 0x6C, DepthStencilResolve::SeparateAspects, Invalidate::None);
    test(0.1f, 0xA2, 0.9f, 0x2B, DepthStencilResolve::SeparateAspects,
         Invalidate::AfterFirstResolve);
    test(0.4f, 0x3F, 0.3f, 0xDD, DepthStencilResolve::SeparateAspects,
         Invalidate::AfterEachResolve);
    test(0.9f, 0xF0, 0.7f, 0x8A, DepthStencilResolve::SeparateAspects, Invalidate::AtEnd);
}

// Test resolving a framebuffer once, then drawing to it again without a complete resolve, and then
// drawing again with a resolve to same or another framebuffer.
TEST_P(FramebufferTest_ES31, ResolveThenDrawWithoutResolveThenDrawWithResolve)
{
    enum class SecondResolve
    {
        SameFramebuffer,
        AnotherFramebuffer,
    };

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorLoc = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorLoc, -1);

    constexpr int kWidth  = 36;
    constexpr int kHeight = 12;
    glViewport(0, 0, kWidth, kHeight);

    GLTexture msaa;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaa,
                           0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Create two resolve FBOs and textures. Use different texture levels and layers.
    GLTexture resolveTexture1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, kWidth * 4, kHeight * 4);

    GLFramebuffer resolveFBO1;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture1, 2);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture resolveTexture2;
    glBindTexture(GL_TEXTURE_2D_ARRAY, resolveTexture2);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, kWidth, kHeight, 5);

    GLFramebuffer resolveFBO2;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, resolveTexture2, 0, 4);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    auto test = [&](GLColor color1, GLColor color2, GLColor color3, SecondResolve secondResolve) {
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        glUniform4fv(colorLoc, 1, color1.toNormalizedVector().data());
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_NO_ERROR();

        // Resolve
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
        glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        // Draw again, but don't resolve.
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
        glUniform4fv(colorLoc, 1, color2.toNormalizedVector().data());
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);

        // Make sure the render pass is flushed.
        glFinish();

        // Verify the contents of the resolve attachment
        glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO1);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, color1);

        // Draw and resolve again
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glUniform4fv(colorLoc, 1, color3.toNormalizedVector().data());
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        glDisable(GL_BLEND);

        GLint fboToResolve =
            secondResolve == SecondResolve::SameFramebuffer ? resolveFBO1 : resolveFBO2;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboToResolve);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
        glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);

        const GLColor blendedColor = GLColor(
            std::clamp(color2.R + color3.R, 0, 255), std::clamp(color2.G + color3.G, 0, 255),
            std::clamp(color2.B + color3.B, 0, 255), std::clamp(color2.A + color3.A, 0, 255));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboToResolve);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, blendedColor);
    };

    test(GLColor::red, GLColor::green, GLColor::blue, SecondResolve::SameFramebuffer);
    test(GLColor::cyan, GLColor(180, 0, 0, 190), GLColor(100, 255, 0, 80),
         SecondResolve::AnotherFramebuffer);
}

// Test resolving a depth/stencil framebuffer once, then drawing to it again without a complete
// resolve, and then drawing again with a resolve to same or another framebuffer.
TEST_P(FramebufferTest_ES31, DepthStencilResolveThenDrawWithoutResolveThenDrawWithResolve)
{
    enum class SecondResolve
    {
        SameFramebuffer,
        AnotherFramebuffer,
    };
    enum class DepthStencilResolve
    {
        Simultaneous,
        SeparateAspectsButSameFramebuffer,
        SeparateAspectsDifferentFramebuffers,
    };

    constexpr int kWidth  = 24;
    constexpr int kHeight = 12;
    glViewport(0, 0, kWidth, kHeight);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Create two resolve FBOs and textures. Use different texture levels and layers.
    GLTexture resolveTexture1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_DEPTH24_STENCIL8, kWidth * 4, kHeight * 4);

    GLFramebuffer resolveFBO1;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           resolveTexture1, 2);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture resolveTexture2;
    glBindTexture(GL_TEXTURE_2D, resolveTexture2);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_DEPTH24_STENCIL8, kWidth * 2, kHeight * 2);

    GLFramebuffer resolveFBO2;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           resolveTexture2, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(red, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Red());

    auto runTest = [&](float depth1, int stencil1, float depth2, int stencil2,
                       DepthStencilResolve resolve, SecondResolve secondResolve) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
        glDepthMask(GL_TRUE);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, stencil1, 0xFF);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glStencilMask(0xFF);

        // Initialize the depth/stencil image
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        drawQuad(red, essl1_shaders::PositionAttrib(), depth1);
        ASSERT_GL_NO_ERROR();

        // Resolve depth and stencil, then verify the results
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
        switch (resolve)
        {
            case DepthStencilResolve::Simultaneous:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspectsButSameFramebuffer:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                                  GL_NEAREST);
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspectsDifferentFramebuffers:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                                  GL_NEAREST);
                break;
        }

        // Draw again, but don't resolve.
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
        drawQuad(red, essl31_shaders::PositionAttrib(), 0);

        // Make sure the render pass is flushed.
        glFinish();

        // Draw and resolve again
        glStencilFunc(GL_ALWAYS, stencil2, 0xFF);
        drawQuad(red, essl31_shaders::PositionAttrib(), depth2);

        GLint fboToResolve =
            secondResolve == SecondResolve::SameFramebuffer ? resolveFBO1 : resolveFBO2;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboToResolve);
        switch (resolve)
        {
            case DepthStencilResolve::Simultaneous:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspectsButSameFramebuffer:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                                  GL_NEAREST);
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspectsDifferentFramebuffers:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
        }
        ASSERT_GL_NO_ERROR();

        if (secondResolve == SecondResolve::SameFramebuffer)
        {
            verifyDepth(resolveFBO1, kWidth, kHeight,
                        resolve == DepthStencilResolve::SeparateAspectsDifferentFramebuffers
                            ? depth1
                            : depth2);
            verifyStencil(resolveFBO1, kWidth, kHeight, stencil2);
        }
        else
        {
            verifyDepth(resolveFBO1, kWidth, kHeight, depth1);
            if (resolve != DepthStencilResolve::SeparateAspectsDifferentFramebuffers)
            {
                verifyStencil(resolveFBO1, kWidth, kHeight, stencil1);
                verifyDepth(resolveFBO2, kWidth, kHeight, depth2);
            }
            verifyStencil(resolveFBO2, kWidth, kHeight, stencil2);
        }
    };

    runTest(0.4f, 0x3F, 0.1f, 0xA2, DepthStencilResolve::Simultaneous,
            SecondResolve::SameFramebuffer);
    runTest(0.9f, 0x2B, 0.3f, 0xDD, DepthStencilResolve::Simultaneous,
            SecondResolve::AnotherFramebuffer);
    runTest(0.6f, 0x7E, 0.6f, 0x7E, DepthStencilResolve::SeparateAspectsButSameFramebuffer,
            SecondResolve::SameFramebuffer);
    runTest(0.1f, 0xA2, 0.4f, 0x34, DepthStencilResolve::SeparateAspectsButSameFramebuffer,
            SecondResolve::AnotherFramebuffer);
    runTest(0.4f, 0x3F, 0.5f, 0x6C, DepthStencilResolve::SeparateAspectsDifferentFramebuffers,
            SecondResolve::SameFramebuffer);
    runTest(0.9f, 0xF0, 0.9f, 0x2B, DepthStencilResolve::SeparateAspectsDifferentFramebuffers,
            SecondResolve::AnotherFramebuffer);
    ASSERT_GL_NO_ERROR();
}

// Test resolving a framebuffer once, then changing its attachment (with the same format) and
// draw+resolving again.  Makes sure the wrong framebuffer or render pass is not picked from a
// cache.
TEST_P(FramebufferTest_ES31, ResolveThenChangeAttachmentThenResolveAgain)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorLoc = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorLoc, -1);

    constexpr int kWidth  = 36;
    constexpr int kHeight = 20;
    glViewport(0, 0, kWidth, kHeight);

    GLTexture msaa0, msaa1;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa1);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaa0,
                           0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Create the resolve FBO and texture.
    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, kWidth * 2 + 1, kHeight * 2 + 1);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw and resolve once
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glUniform4fv(colorLoc, 1, GLColor::green.toNormalizedVector().data());
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Change the framebuffer attachment to another texture
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaa1,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw and resolve again
    glUniform4fv(colorLoc, 1, GLColor::blue.toNormalizedVector().data());
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Verify results
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::blue);
    ASSERT_GL_NO_ERROR();
}

// Test resolving a depth/stencil framebuffer once, then changing its attachment (with the same
// format) and draw+resolving again.  Makes sure the wrong framebuffer or render pass is not picked
// from a cache.
TEST_P(FramebufferTest_ES31, DepthStencilResolveThenChangeAttachmentThenResolveAgain)
{
    enum class DepthStencilResolve
    {
        Simultaneous,
        SeparateAspects,
    };

    constexpr int kWidth  = 24;
    constexpr int kHeight = 12;
    glViewport(0, 0, kWidth, kHeight);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLRenderbuffer depthStencil, depth, stencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kWidth, kHeight);

    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT16, kWidth, kHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, stencil);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_STENCIL_INDEX8, kWidth, kHeight);

    // Create two resolve FBOs and textures. Use different texture levels and layers.
    GLTexture resolveTexture1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_DEPTH24_STENCIL8, kWidth * 4, kHeight * 4);

    GLFramebuffer resolveFBO1;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           resolveTexture1, 2);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture resolveTexture2;
    glBindTexture(GL_TEXTURE_2D, resolveTexture2);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_DEPTH_COMPONENT16, kWidth * 2, kHeight * 2);

    GLFramebuffer resolveFBO2;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, resolveTexture2, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture resolveTexture3;
    glBindTexture(GL_TEXTURE_2D, resolveTexture3);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_STENCIL_INDEX8, kWidth * 2, kHeight * 2);

    GLFramebuffer resolveFBO3;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO3);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, resolveTexture3,
                           1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(red, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Red());

    auto runTest = [&](float depth1, int stencil1, float depth2, int stencil2,
                       DepthStencilResolve resolve) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
        glDepthMask(GL_TRUE);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, stencil1, 0xFF);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glStencilMask(0xFF);

        // Initialize the depth/stencil image
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  depthStencil);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        drawQuad(red, essl1_shaders::PositionAttrib(), depth1);
        ASSERT_GL_NO_ERROR();

        // Resolve depth and stencil
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
        switch (resolve)
        {
            case DepthStencilResolve::Simultaneous:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
            case DepthStencilResolve::SeparateAspects:
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                                  GL_NEAREST);
                glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                                  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                break;
        }

        // Change the framebuffer and draw/resolve again
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
        drawQuad(red, essl1_shaders::PositionAttrib(), depth2);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO2);
        glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                          GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil);
        glStencilFunc(GL_ALWAYS, stencil2, 0xFF);
        drawQuad(red, essl1_shaders::PositionAttrib(), 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO3);
        glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_STENCIL_BUFFER_BIT,
                          GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        verifyDepth(resolveFBO1, kWidth, kHeight, depth1);
        verifyStencil(resolveFBO1, kWidth, kHeight, stencil1);
        verifyDepth(resolveFBO2, kWidth, kHeight, depth2);
        verifyStencil(resolveFBO3, kWidth, kHeight, stencil2);
    };

    runTest(0.1f, 0x78, 0.4f, 0x34, DepthStencilResolve::Simultaneous);
    runTest(0.6f, 0x7E, 0.5f, 0x6C, DepthStencilResolve::SeparateAspects);
    ASSERT_GL_NO_ERROR();
}

// Similar to ResolveThenChangeAttachmentThenResolveAgain, but the attachment format is also changed
TEST_P(FramebufferTest_ES31, ResolveThenChangeAttachmentFormatThenResolveAgain)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorLoc = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorLoc, -1);

    constexpr int kWidth  = 32;
    constexpr int kHeight = 24;
    glViewport(0, 0, kWidth, kHeight);

    GLTexture msaa0, msaa1;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa1);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB565, kWidth, kHeight, false);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaa0,
                           0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Create two resolve FBOs and textures. Use different texture levels and layers.
    GLTexture resolveTexture1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, kWidth * 4, kHeight * 4);

    GLFramebuffer resolveFBO1;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture1, 2);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture resolveTexture2;
    glBindTexture(GL_TEXTURE_2D, resolveTexture2);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGB565, kWidth * 4, kHeight * 4);

    GLFramebuffer resolveFBO2;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture2, 2);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw and resolve once
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glUniform4fv(colorLoc, 1, GLColor::blue.toNormalizedVector().data());
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Change the framebuffer attachment to another texture with a different format
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaa1,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw and resolve again
    glUniform4fv(colorLoc, 1, GLColor::yellow.toNormalizedVector().data());
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO2);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Verify results
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO1);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::blue);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO2);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::yellow);
    ASSERT_GL_NO_ERROR();
}

// Draw and resolve once, use the resolve attachment as a storage image, then draw and resolve
// again.  The storage image usage may recreate the image internally, and this test makes sure the
// wrong framebuffer or render pass is not picked from a cache.
TEST_P(FramebufferTest_ES31, ResolveThenUseAsStorageImageThenResolveAgain)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorLoc = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorLoc, -1);

    constexpr int kWidth  = 36;
    constexpr int kHeight = 20;
    glViewport(0, 0, kWidth, kHeight);

    GLTexture msaa0, msaa1;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa1);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaa0,
                           0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Create the resolve FBO and texture.
    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, kWidth * 2, kHeight * 2);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw and resolve once
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glUniform4fv(colorLoc, 1, GLColor::green.toNormalizedVector().data());
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Use the resolve attachment as a storage image.  A different level is used to be able to
    // verify the compute shader results.
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba8, binding = 0) writeonly uniform highp image2D image;
void main()
{
    imageStore(image, ivec2(gl_GlobalInvocationID.xy), vec4(1, 1, 0, 1));
})";

    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCS);
    glUseProgram(csProgram);
    glBindImageTexture(0, resolveTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glDispatchCompute(kWidth * 2, kHeight * 2, 1);

    // Draw and resolve again
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glUseProgram(program);
    glUniform4fv(colorLoc, 1, GLColor::blue.toNormalizedVector().data());
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Verify resolve results
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::cyan);

    // For completeness, verify compute shader write results
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture,
                           0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth * 2, kHeight * 2, GLColor::yellow);
    ASSERT_GL_NO_ERROR();
}

// Test that invalidation tracking works when glBlitFramebuffer resolves into an invalidated
// framebuffer.
TEST_P(FramebufferTest_ES31, InvalidateThenResolve)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorLoc = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorLoc, -1);

    constexpr int kWidth  = 36;
    constexpr int kHeight = 20;
    glViewport(0, 0, kWidth, kHeight);

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight);

    GLFramebuffer FBO;
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Initialize the single-sampled image but discard it right away.
    glUniform4fv(colorLoc, 1, GLColor::green.toNormalizedVector().data());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    const GLenum discard[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discard);

    GLTexture msaaColor;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaColor);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaColor, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw into the MSAA image
    glUniform4fv(colorLoc, 1, GLColor::red.toNormalizedVector().data());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    // Resolve into the single-sampled image
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // At this point, the contents of the single-sampled image must be considered well-defined (not
    // invalidated).  Blend transparent blue in it for verification.

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glUniform4f(colorLoc, 0, 0, 1, 0);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    // Verify resolve results
    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::magenta);
    ASSERT_GL_NO_ERROR();
}

// Test framebuffer completeness with a mix of multisampled textures and renderbuffers.
TEST_P(FramebufferTest_ES31, MixesMultisampleTextureRenderbuffer)
{
    GLFramebuffer mFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    GLRenderbuffer mRenderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mRenderbuffer);

    GLTexture mTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH24_STENCIL8, 1, 1, true);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, mTexture,
                           0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE,
                           mTexture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES2_AND(AddMockTextureNoRenderTargetTest,
                               ES2_D3D9().enable(Feature::AddMockTextureNoRenderTarget),
                               ES2_D3D11().enable(Feature::AddMockTextureNoRenderTarget));

ANGLE_INSTANTIATE_TEST_ES2(FramebufferTest);
ANGLE_INSTANTIATE_TEST_ES2(FramebufferExtensionsTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(FramebufferFormatsTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(FramebufferTest_ES3);
ANGLE_INSTANTIATE_TEST_ES3_AND(FramebufferTest_ES3,
                               ES3_VULKAN().enable(Feature::EmulatedPrerotation90),
                               ES3_VULKAN().enable(Feature::EmulatedPrerotation180),
                               ES3_VULKAN().enable(Feature::EmulatedPrerotation270));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(FramebufferTest_ES3Metal);
ANGLE_INSTANTIATE_TEST(FramebufferTest_ES3Metal,
                       ES3_METAL().enable(Feature::LimitMaxColorTargetBitsForTesting));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(FramebufferTest_ES31);
ANGLE_INSTANTIATE_TEST_ES31_AND(FramebufferTest_ES31,
                                ES31_VULKAN().disable(Feature::SupportsImagelessFramebuffer));
ANGLE_INSTANTIATE_TEST_ES3(FramebufferTestWithFormatFallback);
ANGLE_INSTANTIATE_TEST_ES3(DefaultFramebufferTest);
