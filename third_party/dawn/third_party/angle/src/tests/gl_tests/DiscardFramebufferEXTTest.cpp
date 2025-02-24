//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class DiscardFramebufferEXTTest : public ANGLETest<>
{
  protected:
    DiscardFramebufferEXTTest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }
};

TEST_P(DiscardFramebufferEXTTest, DefaultFramebuffer)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_discard_framebuffer"));

    // TODO: fix crash issue. http://anglebug.com/42262774
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // These should succeed on the default framebuffer
    const GLenum discards1[] = {GL_COLOR_EXT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards1);
    EXPECT_GL_NO_ERROR();

    const GLenum discards2[] = {GL_DEPTH_EXT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards2);
    EXPECT_GL_NO_ERROR();

    const GLenum discards3[] = {GL_STENCIL_EXT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards3);
    EXPECT_GL_NO_ERROR();

    const GLenum discards4[] = {GL_STENCIL_EXT, GL_COLOR_EXT, GL_DEPTH_EXT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 3, discards4);
    EXPECT_GL_NO_ERROR();

    // These should fail on the default framebuffer
    const GLenum discards5[] = {GL_COLOR_ATTACHMENT0};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards5);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    const GLenum discards6[] = {GL_DEPTH_ATTACHMENT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards6);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    const GLenum discards7[] = {GL_STENCIL_ATTACHMENT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards7);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

TEST_P(DiscardFramebufferEXTTest, NonDefaultFramebuffer)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_discard_framebuffer"));

    GLuint tex2D;
    GLuint framebuffer;

    // Create a basic off-screen framebuffer
    // Don't create a depth/stencil texture, to ensure that also works correctly
    glGenTextures(1, &tex2D);
    glGenFramebuffers(1, &framebuffer);
    glBindTexture(GL_TEXTURE_2D, tex2D);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getWindowWidth(), getWindowHeight(), 0, GL_RGB,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex2D, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // These should fail on the non-default framebuffer
    const GLenum discards1[] = {GL_COLOR_EXT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards1);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    const GLenum discards2[] = {GL_DEPTH_EXT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards2);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    const GLenum discards3[] = {GL_STENCIL_EXT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards3);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    const GLenum discards4[] = {GL_STENCIL_EXT, GL_COLOR_EXT, GL_DEPTH_EXT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 3, discards4);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // These should succeed on the non-default framebuffer
    const GLenum discards5[] = {GL_COLOR_ATTACHMENT0};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards5);
    EXPECT_GL_NO_ERROR();

    const GLenum discards6[] = {GL_DEPTH_ATTACHMENT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards6);
    EXPECT_GL_NO_ERROR();

    const GLenum discards7[] = {GL_STENCIL_ATTACHMENT};
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards7);
    EXPECT_GL_NO_ERROR();
}

// ANGLE implements an optimization that if depth stencil buffer has not been used and not stored in
// the renderpass, the depth buffer clear will be dropped.
TEST_P(DiscardFramebufferEXTTest, ClearDepthThenDrawWithoutDepthTestThenDiscard)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_discard_framebuffer"));
    // TODO: fix crash issue. http://anglebug.com/42262774
    ANGLE_SKIP_TEST_IF(IsD3D11());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorUniformLocation);
    ASSERT_GL_NO_ERROR();

    constexpr GLfloat kDepthClearValue = 0.5f;
    // This depth value equals to kDepthClearValue after viewport transform
    constexpr GLfloat depthDrawValue = kDepthClearValue * 2.0f - 1.0f;

    // This depth clear should be optimized out. We do not have a good way to verify that it
    // actually gets dropped, but at least we will ensure rendering is still correct.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glClearDepthf(kDepthClearValue);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glUniform4fv(colorUniformLocation, 1, GLColor::cyan.toNormalizedVector().data());
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    drawQuad(program, essl1_shaders::PositionAttrib(), depthDrawValue + 0.05f);
    GLenum discards = GL_DEPTH_EXT;
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, &discards);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::cyan);
}

// This test is try to ensure that if depth test has been used, depth clear does not get optimized
// out. It also tests that if the depth buffer has not been used, the rendering is still correct.
TEST_P(DiscardFramebufferEXTTest, ClearDepthThenDrawWithDepthTestThenDiscard)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_discard_framebuffer"));
    // http://anglebug.com/42263489
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsIntel() && IsWindows());

    GLTexture texture;
    GLRenderbuffer renderbuffer;
    GLFramebuffer framebuffer;
    GLint colorUniformLocation;
    constexpr GLsizei kTexWidth  = 256;
    constexpr GLsizei kTexHeight = 256;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, kTexWidth, kTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    ASSERT_GL_NO_ERROR();
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kTexWidth, kTexWidth);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    colorUniformLocation = glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorUniformLocation);
    ASSERT_GL_NO_ERROR();

    // Draw into FBO0
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);  // clear to green
    constexpr GLfloat kDepthClearValue = 0.5f;
    // This depth value equals to kDepthClearValue after viewport transform
    constexpr GLfloat depthDrawValue = kDepthClearValue * 2.0f - 1.0f;
    glClearDepthf(kDepthClearValue);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Draw bottom left with depth test disabled. DepthValue should remain 0.5f with blue color.
    glDepthFunc(GL_LESS);
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, kTexWidth / 2, kTexHeight / 2);
    glUniform4fv(colorUniformLocation, 1, GLColor::blue.toNormalizedVector().data());
    drawQuad(program, essl1_shaders::PositionAttrib(), depthDrawValue - 0.1f);
    // Draw bottom right with depth test enabled. DepthValue should be 0.45f with blue color.
    glEnable(GL_DEPTH_TEST);
    glViewport(kTexWidth / 2, 0, kTexWidth / 2, kTexHeight / 2);
    drawQuad(program, essl1_shaders::PositionAttrib(), depthDrawValue - 0.1f);
    // Draw to top left with depth test disabled. DepthValue should remain 0.5f with blue color
    glDisable(GL_DEPTH_TEST);
    glViewport(0, kTexHeight / 2, kTexWidth / 2, kTexHeight / 2);
    drawQuad(program, essl1_shaders::PositionAttrib(), depthDrawValue + 0.1f);
    // Draw to top right with depth test enabled. DepthValue should remain 0.5f with green color
    glEnable(GL_DEPTH_TEST);
    glViewport(kTexWidth / 2, kTexHeight / 2, kTexWidth / 2, kTexHeight / 2);
    drawQuad(program, essl1_shaders::PositionAttrib(), depthDrawValue + 0.1f);

    // Now draw the full quad with depth test enabled to verify the depth value is expected.
    // It should fail depth test in bottom right which will keep it with blue color. All other
    // quarters will pass depth test and draw a red quad.
    glEnable(GL_DEPTH_TEST);
    glUniform4fv(colorUniformLocation, 1, GLColor::red.toNormalizedVector().data());
    glViewport(0, 0, kTexWidth, kTexHeight);
    drawQuad(program, essl1_shaders::PositionAttrib(), depthDrawValue - 0.05f);

    // Invalidate depth buffer. This will trigger depth value not been written to buffer but the
    // depth load/clear should not optimize out
    GLenum discards = GL_DEPTH_ATTACHMENT;
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, &discards);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kTexWidth / 2 + 1, 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(1, kTexHeight / 2 + 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kTexWidth / 2 + 1, kTexHeight / 2 + 1, GLColor::red);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(DiscardFramebufferEXTTest);
