//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SRGBFramebufferTest.cpp: Tests of sRGB framebuffer functionality.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

namespace
{
constexpr angle::GLColor linearColor(64, 127, 191, 255);
constexpr angle::GLColor srgbColor(13, 54, 133, 255);
}  // namespace

namespace angle
{

class SRGBFramebufferTest : public ANGLETest<>
{
  protected:
    SRGBFramebufferTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        mProgram = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        ASSERT_NE(0u, mProgram);

        mColorLocation = glGetUniformLocation(mProgram, essl1_shaders::ColorUniform());
        ASSERT_NE(-1, mColorLocation);
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    GLuint mProgram      = 0;
    GLint mColorLocation = -1;
};

class SRGBFramebufferTestES3 : public SRGBFramebufferTest
{};

// Test basic validation of GL_EXT_sRGB_write_control
TEST_P(SRGBFramebufferTest, Validation)
{
    GLenum expectedError =
        IsGLExtensionEnabled("GL_EXT_sRGB_write_control") ? GL_NO_ERROR : GL_INVALID_ENUM;

    GLboolean value = GL_FALSE;
    glEnable(GL_FRAMEBUFFER_SRGB_EXT);
    EXPECT_GL_ERROR(expectedError);

    glGetBooleanv(GL_FRAMEBUFFER_SRGB_EXT, &value);
    EXPECT_GL_ERROR(expectedError);
    if (expectedError == GL_NO_ERROR)
    {
        EXPECT_GL_TRUE(value);
    }

    glDisable(GL_FRAMEBUFFER_SRGB_EXT);
    EXPECT_GL_ERROR(expectedError);

    glGetBooleanv(GL_FRAMEBUFFER_SRGB_EXT, &value);
    EXPECT_GL_ERROR(expectedError);
    if (expectedError == GL_NO_ERROR)
    {
        EXPECT_GL_FALSE(value);
    }
}

// Test basic functionality of GL_EXT_sRGB_write_control
TEST_P(SRGBFramebufferTest, BasicUsage)
{
    if (!IsGLExtensionEnabled("GL_EXT_sRGB_write_control") ||
        (!IsGLExtensionEnabled("GL_EXT_sRGB") && getClientMajorVersion() < 3))
    {
        std::cout
            << "Test skipped because GL_EXT_sRGB_write_control and GL_EXT_sRGB are not available."
            << std::endl;
        return;
    }

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA_EXT, 1, 1, 0, GL_SRGB_ALPHA_EXT, GL_UNSIGNED_BYTE,
                 nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glUseProgram(mProgram);
    glUniform4fv(mColorLocation, 1, srgbColor.toNormalizedVector().data());

    glEnable(GL_FRAMEBUFFER_SRGB_EXT);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, linearColor, 1.0);

    glDisable(GL_FRAMEBUFFER_SRGB_EXT);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);
}

// Test that GL_EXT_sRGB_write_control state applies to all framebuffers if multiple are used
// 1. disable srgb
// 2. draw to both framebuffers
// 3. enable srgb
// 4. draw to both framebuffers
TEST_P(SRGBFramebufferTest, MultipleFramebuffers)
{
    if (!IsGLExtensionEnabled("GL_EXT_sRGB_write_control") ||
        (!IsGLExtensionEnabled("GL_EXT_sRGB") && getClientMajorVersion() < 3))
    {
        std::cout
            << "Test skipped because GL_EXT_sRGB_write_control and GL_EXT_sRGB are not available."
            << std::endl;
        return;
    }

    // NVIDIA failures on older drivers
    // http://anglebug.com/42264177
    ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsOpenGLES());

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA_EXT, 1, 1, 0, GL_SRGB_ALPHA_EXT, GL_UNSIGNED_BYTE,
                 nullptr);

    GLFramebuffer framebuffer1;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glUseProgram(mProgram);
    glUniform4fv(mColorLocation, 1, srgbColor.toNormalizedVector().data());

    glDisable(GL_FRAMEBUFFER_SRGB_EXT);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);

    GLFramebuffer framebuffer2;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);

    glEnable(GL_FRAMEBUFFER_SRGB_EXT);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, linearColor, 1.0);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, linearColor, 1.0);
}

// Test that we behave correctly when we toggle FRAMEBUFFER_SRGB_EXT on a framebuffer that has an
// attachment in linear colorspace
TEST_P(SRGBFramebufferTest, NegativeAlreadyLinear)
{
    if (!IsGLExtensionEnabled("GL_EXT_sRGB_write_control") ||
        (!IsGLExtensionEnabled("GL_EXT_sRGB") && getClientMajorVersion() < 3))
    {
        std::cout
            << "Test skipped because GL_EXT_sRGB_write_control and GL_EXT_sRGB are not available."
            << std::endl;
        return;
    }

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glUseProgram(mProgram);
    glUniform4fv(mColorLocation, 1, linearColor.toNormalizedVector().data());

    glEnable(GL_FRAMEBUFFER_SRGB_EXT);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, linearColor, 1.0);

    glDisable(GL_FRAMEBUFFER_SRGB_EXT);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, linearColor, 1.0);
}

// Test that lifetimes of internal resources are tracked correctly by deleting a texture and then
// attempting to use it. This is expected to produce a non-fatal error.
TEST_P(SRGBFramebufferTest, NegativeLifetimeTracking)
{
    if (!IsGLExtensionEnabled("GL_EXT_sRGB_write_control") ||
        (!IsGLExtensionEnabled("GL_EXT_sRGB") && getClientMajorVersion() < 3))
    {
        std::cout
            << "Test skipped because GL_EXT_sRGB_write_control and GL_EXT_sRGB are not available."
            << std::endl;
        return;
    }

    // NVIDIA failures
    // http://anglebug.com/42264177
    ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsOpenGLES());

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA_EXT, 1, 1, 0, GL_SRGB_ALPHA_EXT, GL_UNSIGNED_BYTE,
                 nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glUseProgram(mProgram);
    glUniform4fv(mColorLocation, 1, srgbColor.toNormalizedVector().data());

    glDisable(GL_FRAMEBUFFER_SRGB_EXT);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);

    // Delete the texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    texture.reset();

    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);

    GLColor throwaway_color;
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &throwaway_color);
    EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
}

// Test that glBlitFramebuffer correctly converts colorspaces
TEST_P(SRGBFramebufferTestES3, BlitFramebuffer)
{
    // http://anglebug.com/42264326
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    if (!IsGLExtensionEnabled("GL_EXT_sRGB_write_control") ||
        (!IsGLExtensionEnabled("GL_EXT_sRGB") && getClientMajorVersion() < 3))
    {
        std::cout
            << "Test skipped because GL_EXT_sRGB_write_control and GL_EXT_sRGB are not available."
            << std::endl;
        return;
    }

    GLTexture dstTexture;
    glBindTexture(GL_TEXTURE_2D, dstTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA_EXT, 1, 1, 0, GL_SRGB_ALPHA_EXT, GL_UNSIGNED_BYTE,
                 nullptr);
    GLFramebuffer dstFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTexture, 0);

    GLTexture srcTexture;
    glBindTexture(GL_TEXTURE_2D, srcTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA_EXT, 1, 1, 0, GL_SRGB_ALPHA_EXT, GL_UNSIGNED_BYTE,
                 nullptr);

    GLFramebuffer srcFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, srcFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTexture, 0);

    glUseProgram(mProgram);
    glUniform4fv(mColorLocation, 1, srgbColor.toNormalizedVector().data());

    // Draw onto the framebuffer normally
    glEnable(GL_FRAMEBUFFER_SRGB_EXT);
    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, linearColor, 1.0);

    // Blit the framebuffer normally
    glEnable(GL_FRAMEBUFFER_SRGB_EXT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFramebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFramebuffer);
    glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, linearColor, 1.0);

    // Blit the framebuffer with forced linear colorspace
    glDisable(GL_FRAMEBUFFER_SRGB_EXT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFramebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFramebuffer);
    glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);
}

// This test reproduces an issue in the Vulkan backend found in the Chromium CI that
// was caused by enabling the VK_KHR_image_format_list extension on SwiftShader
// which exposed GL_EXT_sRGB_write_control.
TEST_P(SRGBFramebufferTest, DrawToSmallFBOClearLargeFBO)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_sRGB_write_control") ||
                       (!IsGLExtensionEnabled("GL_EXT_sRGB") && getClientMajorVersion() < 3));

    // Disabling GL_FRAMEBUFFER_SRGB_EXT caused the issue
    glDisable(GL_FRAMEBUFFER_SRGB_EXT);

    // The issue involved framebuffers of two different sizes.
    // The smaller needed to be drawn to, while the larger one could be just cleared
    // to reproduce the issue. These are the smallest tested sizes that generated
    // the validation error.
    constexpr GLsizei kDimensionsSmall[] = {1, 1};
    constexpr GLsizei kDimensionsLarge[] = {2, 2};
    {
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kDimensionsSmall[0], kDimensionsSmall[1]);
        glBindTexture(GL_TEXTURE_2D, 0);

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        unsigned char vertexData[] = {0};
        GLBuffer vertexBuffer;
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(char), vertexData, GL_STATIC_DRAW);

        unsigned int indexData[] = {0};
        GLBuffer indexBuffer;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int), indexData, GL_STATIC_DRAW);

        glUseProgram(mProgram);

        glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, nullptr);

        EXPECT_GL_NO_ERROR();
    }
    {
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, kDimensionsLarge[0], kDimensionsLarge[1]);
        glBindTexture(GL_TEXTURE_2D, 0);

        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        // Vulkan validation happened to fail here with:
        // "Cannot execute a render pass with renderArea not within the bound of the framebuffer"
        glClear(GL_COLOR_BUFFER_BIT);

        EXPECT_GL_NO_ERROR();
    }
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(SRGBFramebufferTest);
ANGLE_INSTANTIATE_TEST_ES3(SRGBFramebufferTestES3);

}  // namespace angle
