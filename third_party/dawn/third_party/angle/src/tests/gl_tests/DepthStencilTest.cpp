//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DepthStencilTest:
//   Tests covering depth- or stencil-only rendering to make sure the other non-existing aspect is
//   not affecting the results (since the format may be emulated with one that has both aspects).
//

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class DepthStencilTest : public ANGLETest<>
{
  protected:
    DepthStencilTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    void testSetUp() override
    {
        glBindTexture(GL_TEXTURE_2D, mColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);

        // Setup Color/Stencil FBO with a stencil format that's emulated with packed depth/stencil.
        glBindFramebuffer(GL_FRAMEBUFFER, mColorStencilFBO);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorTexture,
                               0);
        glBindRenderbuffer(GL_RENDERBUFFER, mStencilTexture);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, getWindowWidth(),
                              getWindowHeight());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  mStencilTexture);

        ASSERT_GL_NO_ERROR();

        // Note: GL_DEPTH_COMPONENT24 is allowed in GLES2 with GL_OES_depth24 extension.
        if (getClientMajorVersion() >= 3 || IsGLExtensionEnabled("GL_OES_depth24"))
        {
            // Setup Color/Depth FBO with a depth format that's emulated with packed depth/stencil.
            glBindFramebuffer(GL_FRAMEBUFFER, mColorDepthFBO);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   mColorTexture, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, mDepthTexture);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, getWindowWidth(),
                                  getWindowHeight());
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                      mDepthTexture);
        }

        ASSERT_GL_NO_ERROR();
    }

    void bindColorStencilFBO()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mColorStencilFBO);
        mHasDepth = false;
    }

    void bindColorDepthFBO()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mColorDepthFBO);
        mHasStencil = false;
    }

    void prepareSingleEmulatedWithPacked();
    void ensureColor(GLColor color);
    void ensureDepthUnaffected();
    void ensureStencilUnaffected();

  private:
    GLFramebuffer mColorStencilFBO;
    GLFramebuffer mColorDepthFBO;
    GLTexture mColorTexture;
    GLRenderbuffer mDepthTexture;
    GLRenderbuffer mStencilTexture;

    bool mHasDepth   = true;
    bool mHasStencil = true;
};

class DepthStencilTestES3 : public DepthStencilTest
{
  protected:
    void compareDepth(uint32_t expected);
    void clearAndCompareDepth(GLfloat depth, uint32_t expected);
    void drawAndCompareDepth(GLProgram &program, GLfloat depth, uint32_t expected);
};

void DepthStencilTest::ensureColor(GLColor color)
{
    const int width  = getWindowWidth();
    const int height = getWindowHeight();

    std::vector<GLColor> pixelData(width * height);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());

    for (int i = 0; i < width * height; i += 16)
    {
        GLColor actualColor = pixelData[i];
        EXPECT_NEAR(color.R, actualColor.R, 1);
        EXPECT_NEAR(color.G, actualColor.G, 1);
        EXPECT_NEAR(color.B, actualColor.B, 1);
        EXPECT_NEAR(color.A, actualColor.A, 1);

        if (i % width == 0)
            i += 16 * width;
    }
}

void DepthStencilTest::ensureDepthUnaffected()
{
    ANGLE_GL_PROGRAM(depthTestProgram, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_EQUAL);
    drawQuad(depthTestProgram, essl1_shaders::PositionAttrib(), 0.123f);
    glDisable(GL_DEPTH_TEST);
    ASSERT_GL_NO_ERROR();

    // Since depth shouldn't exist, the drawQuad above should succeed in turning the whole image
    // blue.
    ensureColor(GLColor::blue);
}

void DepthStencilTest::ensureStencilUnaffected()
{
    ANGLE_GL_PROGRAM(stencilTestProgram, essl1_shaders::vs::Passthrough(),
                     essl1_shaders::fs::Green());
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x1B, 0xFF);
    drawQuad(stencilTestProgram, essl1_shaders::PositionAttrib(), 0.0f);
    glDisable(GL_STENCIL_TEST);
    ASSERT_GL_NO_ERROR();

    // Since stencil shouldn't exist, the drawQuad above should succeed in turning the whole image
    // green.
    ensureColor(GLColor::green);
}

void DepthStencilTest::prepareSingleEmulatedWithPacked()
{
    const int w     = getWindowWidth();
    const int h     = getWindowHeight();
    const int whalf = w >> 1;
    const int hhalf = h >> 1;

    // Clear to a random color, 0.75 depth and 0x36 stencil
    Vector4 color1(0.1f, 0.2f, 0.3f, 0.4f);
    GLColor color1RGB(color1);

    glClearColor(color1[0], color1[1], color1[2], color1[3]);
    glClearDepthf(0.75f);
    glClearStencil(0x36);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Verify color was cleared correctly.
    EXPECT_PIXEL_COLOR_NEAR(0, 0, color1RGB, 1);

    // Use masked color to clear two channels of the image to a second color, 0.25 depth and 0x59
    // stencil.
    Vector4 color2(0.2f, 0.4f, 0.6f, 0.8f);
    glClearColor(color2[0], color2[1], color2[2], color2[3]);
    glClearDepthf(0.25f);
    glClearStencil(0x59);
    glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_FALSE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    ASSERT_GL_NO_ERROR();

    GLColor color2RGB(Vector4(color2[0], color1[1], color2[2], color1[3]));

    EXPECT_PIXEL_COLOR_NEAR(whalf, hhalf, color2RGB, 1);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, color2RGB, 1);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, 0, color2RGB, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, h - 1, color2RGB, 1);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, h - 1, color2RGB, 1);

    // Use scissor to clear the center to a third color, 0.5 depth and 0xA9 stencil.
    glEnable(GL_SCISSOR_TEST);
    glScissor(whalf / 2, hhalf / 2, whalf, hhalf);

    Vector4 color3(0.3f, 0.5f, 0.7f, 0.9f);
    GLColor color3RGB(color3);
    glClearColor(color3[0], color3[1], color3[2], color3[3]);
    glClearDepthf(0.5f);
    glClearStencil(0xA9);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_NEAR(whalf, hhalf, color3RGB, 1);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, color2RGB, 1);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, 0, color2RGB, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, h - 1, color2RGB, 1);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, h - 1, color2RGB, 1);

    // Use scissor to draw to the right half of the image with a fourth color, 0.6 depth and 0x84
    // stencil.
    glEnable(GL_SCISSOR_TEST);
    glScissor(whalf, 0, whalf, h);

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x84, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.2f);

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_SCISSOR_TEST);
}

// Tests that clearing or rendering into a depth-only format doesn't affect stencil.
TEST_P(DepthStencilTest, DepthOnlyEmulatedWithPacked)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_OES_depth24"));

    bindColorDepthFBO();
    prepareSingleEmulatedWithPacked();
    ensureStencilUnaffected();
}

// Tests that clearing or rendering into a stencil-only format doesn't affect depth.
TEST_P(DepthStencilTest, StencilOnlyEmulatedWithPacked)
{
    // http://anglebug.com/40096654
    ANGLE_SKIP_TEST_IF(IsWindows() && IsD3D9());
    bindColorStencilFBO();
    prepareSingleEmulatedWithPacked();
    ensureDepthUnaffected();
}

// Tests that drawing into stencil buffer along multiple render passes works.
TEST_P(DepthStencilTest, StencilOnlyDrawThenCopyThenDraw)
{
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    bindColorStencilFBO();

    // Draw red once
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x55, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 1.0f);
    ASSERT_GL_NO_ERROR();

    // Create a texture and copy color into it, this breaks the render pass.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, getWindowWidth(), getWindowHeight(), 0);

    // Draw green, expecting correct stencil.
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 1.0f);
    ASSERT_GL_NO_ERROR();

    // Verify that the texture is now green
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // For completeness, also verify that the copy texture is red
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Tests that clearing depth/stencil followed by draw works when the depth/stencil attachment is a
// texture.
TEST_P(DepthStencilTestES3, ClearThenDraw)
{
    GLFramebuffer FBO;
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    constexpr GLsizei kSize = 6;

    // Create framebuffer to draw into, with both color and depth attachments.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLTexture depth;
    glBindTexture(GL_TEXTURE_2D, depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, kSize, kSize, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8_OES, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
    ASSERT_GL_NO_ERROR();

    // Set viewport and clear depth/stencil
    glViewport(0, 0, kSize, kSize);
    glClearDepthf(1);
    glClearStencil(0x55);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // If depth is not cleared to 1, rendering would fail.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // If stencil is not clear to 0x55, rendering would fail.
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);

    // Set up program
    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    // Draw red
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Verify.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::red);
}

// Test that VK_EXT_load_op_none is working properly when
// one of the depth / stencil load op is none.
// This reproduces a deqp failure on ARM: angleproject:7370
TEST_P(DepthStencilTestES3, LoadStoreOpNoneExtension)
{
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer colorRbo;
    glBindRenderbuffer(GL_RENDERBUFFER, colorRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLRenderbuffer depthStencilBuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, getWindowWidth(),
                          getWindowHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencilBuffer);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glClearDepthf(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    glClearStencil(0.0f);
    glClear(GL_STENCIL_BUFFER_BIT);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Draw a red quad, stencil enabled, depth disabled
    // Depth Load Op: None. Depth Store Op: None.
    // Stencil Load Op: Load. Stencil Store Op: Store.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    GLint colorLocation = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorLocation);

    glDisable(GL_BLEND);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LEQUAL, 0, ~0u);
    glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
    glDisable(GL_DITHER);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(program);
    glUniform4f(colorLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(program, "a_position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw a green quad, stencil enabled, depth enabled.
    // Depth Load Op: Load. Depth Store Op: Store.
    // Stencil Load Op: Load. Stencil Store Op: Store.
    glUniform4f(colorLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    drawQuad(program, "a_position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

void DepthStencilTestES3::compareDepth(uint32_t expected)
{
    uint32_t pixel;
    glReadPixels(0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, &pixel);
    ASSERT_GL_NO_ERROR();

    // Right shift by 8 bits to only compare 24 depth bits
    // and ignore 8 undefined bits.
    pixel = pixel >> 8;

    EXPECT_NEAR(pixel, expected, 1);
}

void DepthStencilTestES3::clearAndCompareDepth(GLfloat depth, uint32_t expected)
{
    glClearDepthf(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
    compareDepth(expected);
}

void DepthStencilTestES3::drawAndCompareDepth(GLProgram &program,
                                              GLfloat positionZ,
                                              uint32_t expected)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    drawQuad(program, essl3_shaders::PositionAttrib(), positionZ, 1.0f);
    glDisable(GL_DEPTH_TEST);
    compareDepth(expected);
}

TEST_P(DepthStencilTestES3, ReadPixelsDepth24)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_depth24") ||
                       !IsGLExtensionEnabled("GL_NV_read_depth"));

    // The test fails on native GLES on Android in glReadPixels
    // with GL_INVALID_OPERATION due to the format/type combination
    // not being supported.
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    // Create GL_DEPTH_COMPONENT24 texture
    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, getWindowWidth(), getWindowHeight(), 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

    // Set up framebuffer
    GLFramebuffer depthFBO;
    GLRenderbuffer depthRenderbuffer;

    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, getWindowWidth(),
                          getWindowHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                              depthRenderbuffer);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Test clear
    clearAndCompareDepth(0.0f, 0x0);
    clearAndCompareDepth(0.125f, 0x200000);
    clearAndCompareDepth(0.5f, 0x800000);
    clearAndCompareDepth(1.0f, 0xffffff);

    // Test draw
    ANGLE_GL_PROGRAM(depthTestProgram, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());
    drawAndCompareDepth(depthTestProgram, 0.0f, 0x800000);
    drawAndCompareDepth(depthTestProgram, 0.125f, 0x8fffff);
    drawAndCompareDepth(depthTestProgram, 0.5f, 0xbfffff);
    drawAndCompareDepth(depthTestProgram, 1.0f, 0xffffff);

    ASSERT_GL_NO_ERROR();
}

// Tests that the stencil test is correctly handled when a framebuffer is cleared before that
// framebuffer's stencil attachment has been configured.
TEST_P(DepthStencilTestES3, FramebufferClearThenStencilAttachedThenStencilTestState)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_NV_read_stencil"));

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer colorRbo;
    glBindRenderbuffer(GL_RENDERBUFFER, colorRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRbo);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLRenderbuffer stencilRbo;
    glBindRenderbuffer(GL_RENDERBUFFER, stencilRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, getWindowWidth(), getWindowHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRbo);
    glClearStencil(2);
    glClear(GL_STENCIL_BUFFER_BIT);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_STENCIL_EQ(0, 0, 2);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());

    GLint colorLocation = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorLocation);

    glUseProgram(program);
    glUniform4f(colorLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
    drawQuad(program, "a_position", 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_STENCIL_EQ(0, 0, 3);
}

// Tests that the stencil test is correctly handled when both the stencil test state is configured
// and a framebuffer is cleared before that framebuffer's stencil attachment has been configured.
TEST_P(DepthStencilTestES3, StencilTestStateThenFramebufferClearThenStencilAttached)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_NV_read_stencil"));

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_INCR, GL_INCR);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer colorRbo;
    glBindRenderbuffer(GL_RENDERBUFFER, colorRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRbo);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLRenderbuffer stencilRbo;
    glBindRenderbuffer(GL_RENDERBUFFER, stencilRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, getWindowWidth(), getWindowHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRbo);
    glClearStencil(2);
    glClear(GL_STENCIL_BUFFER_BIT);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_STENCIL_EQ(0, 0, 2);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());

    GLint colorLocation = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorLocation);

    glUseProgram(program);
    glUniform4f(colorLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    drawQuad(program, "a_position", 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_STENCIL_EQ(0, 0, 3);
}

// Tests that the stencil test is correctly handled when a framebuffer is cleared before that
// framebuffer's stencil attachment has been configured and the stencil test state is configured
// during framebuffer setup.
TEST_P(DepthStencilTestES3, FramebufferClearThenStencilTestStateThenStencilAttached)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_NV_read_stencil"));

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer colorRbo;
    glBindRenderbuffer(GL_RENDERBUFFER, colorRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRbo);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_INCR, GL_INCR);

    GLRenderbuffer stencilRbo;
    glBindRenderbuffer(GL_RENDERBUFFER, stencilRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, getWindowWidth(), getWindowHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRbo);
    glClearStencil(2);
    glClear(GL_STENCIL_BUFFER_BIT);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_STENCIL_EQ(0, 0, 2);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());

    GLint colorLocation = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorLocation);

    glUseProgram(program);
    glUniform4f(colorLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    drawQuad(program, "a_position", 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_STENCIL_EQ(0, 0, 3);
}

// Tests that drawing with read-only depth/stencil followed by depth/stencil output (in two render
// passes) works.  Regression test for a synchronization bug in the Vulkan backend, caught by
// syncval VVL.
TEST_P(DepthStencilTestES3, ReadOnlyDepthStencilThenOutputDepthStencil)
{
    constexpr GLsizei kSize = 64;

    // Create FBO with color, depth and stencil
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffer);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Initialize depth/stencil
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0xAA, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Draw red with depth = 1 and stencil = 0xAA
    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 1);
    ASSERT_GL_NO_ERROR();

    // Break the render pass by making a copy of the color texture.
    GLTexture copyTex;
    glBindTexture(GL_TEXTURE_2D, copyTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Disable depth/stencil output and issue a draw call that's expected to pass depth/stencil.
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);
    glStencilFunc(GL_EQUAL, 0xAA, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // Draw green
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.95);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, kSize / 2, 0, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw again to start another render pass still with depth/stencil read-only
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.95);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, kSize / 2, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Re-enable depth/stencil output and issue a draw call that's expected to pass depth/stencil.
    glDepthMask(GL_TRUE);
    glStencilFunc(GL_EQUAL, 0xAB, 0xF0);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    // Draw yellow
    glUniform4f(colorUniformLocation, 1.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.95);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, kSize / 2, kSize / 2, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer readFramebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copyTex, 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize / 2, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::yellow);
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(
    DepthStencilTest,
    ES2_VULKAN().enable(Feature::ForceFallbackFormat),
    ES2_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat),
    ES3_VULKAN().enable(Feature::ForceFallbackFormat),
    ES3_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DepthStencilTestES3);
ANGLE_INSTANTIATE_TEST_ES3_AND(
    DepthStencilTestES3,
    ES3_VULKAN().enable(Feature::ForceFallbackFormat),
    ES3_VULKAN().enable(Feature::DisallowMixedDepthStencilLoadOpNoneAndLoad),
    ES3_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat));

}  // anonymous namespace
