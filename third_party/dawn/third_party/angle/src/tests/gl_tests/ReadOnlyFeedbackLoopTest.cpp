//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Test that invokes a usecase where there is a feedback loop but the framebuffer
// depth attachment is only read from

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class ReadOnlyFeedbackLoopTest : public ANGLETest<>
{
  protected:
    ReadOnlyFeedbackLoopTest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepthf(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthRangef(-1.0f, 1.0f);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        ASSERT_GL_NO_ERROR();
    }
};

class ReadOnlyFeedbackLoopTestES31 : public ReadOnlyFeedbackLoopTest
{};

// Fill out a depth texture to specific values and use it both as a sampler and a depth texture
// with depth write disabled. This is to test a "read-only feedback loop" that needs to be
// supported to match industry standard.
TEST_P(ReadOnlyFeedbackLoopTest, DepthFeedbackLoop)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_read_only_depth_stencil_feedback_loops"));

    const GLuint width  = getWindowWidth();
    const GLuint height = getWindowHeight();

    GLTexture colorTex;
    GLTexture depthTex;
    GLTexture finalTex;

    GLFramebuffer gbufferFbo;
    GLFramebuffer finalFbo;

    ANGLE_GL_PROGRAM(colorFillProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(textureFillProgram, essl1_shaders::vs::Texture2D(),
                     essl1_shaders::fs::Texture2D());

    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, gbufferFbo);
    EXPECT_GL_NO_ERROR();

    // Attach a color and depth texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    EXPECT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    EXPECT_GL_NO_ERROR();

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Set the color texture to blue and depth texture to 1.0f
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Enable Depth test with passing always to write depth.
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);

    // Fill the middle of the depth texture with 0.0f. while the border remains 1.0f as
    // previously cleared.
    const GLfloat depthValue = 0.0f;
    drawQuad(colorFillProgram, essl1_shaders::PositionAttrib(), depthValue, 0.6f);

    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, finalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, finalFbo);
    EXPECT_GL_NO_ERROR();

    // Enable Depth test without depth write.
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_GREATER);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalTex, 0);
    EXPECT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    EXPECT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, depthTex);

    // Fill finalTex with values read from depthTex. This should work even though depthTex
    // is also bound as the depth attachment, because depth write is disabled.
    // The write to finalTex only succeeds for the middle region due to depth test.
    drawQuad(textureFillProgram, essl1_shaders::PositionAttrib(), 0.7f, 1.0f);

    // Copy finalTex to default framebuffer for verification. Depth values written in the first
    // draw call are expected in the middle, while the clear value in the clear before the
    // second draw call are expected at the border.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glBindTexture(GL_TEXTURE_2D, finalTex);
    drawQuad(textureFillProgram, essl1_shaders::PositionAttrib(), 0.0f, 1.0f);
    EXPECT_GL_NO_ERROR();

    GLint depthColorValue = (depthValue)*128 + 128;
    EXPECT_NEAR(depthColorValue, angle::ReadColor(width / 2, height / 2).R, 1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Tests that we can support a feedback loop between a depth textures and the depth buffer.
// The test emulates the read-only feedback loop in Manhattan.
TEST_P(ReadOnlyFeedbackLoopTest, ReadOnlyDepthFeedbackLoopSupported)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_read_only_depth_stencil_feedback_loops"));

    constexpr GLuint kSize = 2;
    glViewport(0, 0, kSize, kSize);

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D depth;
void main()
{
    if (abs(texture2D(depth, v_texCoord).x - 0.5) < 0.1)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        gl_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), kFS);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear depth to 0.5.
    glClearDepthf(0.5f);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Disable depth. Although this does not remove the feedback loop as defined by the
    // spec it mimics what gfxbench does in its rendering tests.
    glDepthMask(false);
    glDisable(GL_DEPTH_TEST);

    // Verify we can sample the depth texture and get 0.5.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests corner cases with read-only depth-stencil feedback loops.
TEST_P(ReadOnlyFeedbackLoopTest, ReadOnlyDepthFeedbackLoopStateChanges)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_read_only_depth_stencil_feedback_loops"));

    constexpr GLuint kSize = 2;
    glViewport(0, 0, kSize, kSize);

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D depth;
void main()
{
    if (abs(texture2D(depth, v_texCoord).x - 0.5) < 0.1)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        gl_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), kFS);
    glUseProgram(program);

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    GLFramebuffer framebuffer1;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLFramebuffer framebuffer2;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ASSERT_GL_NO_ERROR();

    // Clear depth to 0.5.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    glClearDepthf(0.5f);
    glClear(GL_DEPTH_BUFFER_BIT);
    glFlush();

    // Disable depth. Although this does not remove the feedback loop as defined by the
    // spec it mimics what gfxbench does in its rendering tests.
    glDepthMask(false);
    glDisable(GL_DEPTH_TEST);

    // Draw with loop.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Draw with no loop and second FBO. Starts RP in writable mode.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Draw with loop, restarts RP.
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
}

// Tests depth/stencil clear after read-only depth/stencil feedback loop draw.
TEST_P(ReadOnlyFeedbackLoopTest, ReadOnlyDepthFeedbackLoopDrawThenDepthStencilClear)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_read_only_depth_stencil_feedback_loops"));

    constexpr GLuint kSize = 2;
    glViewport(0, 0, kSize, kSize);

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D depth;
void main()
{
    if (abs(texture2D(depth, v_texCoord).x - 0.5) < 0.1)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        gl_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), kFS);
    glUseProgram(program);

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear depth to 0.5.
    glClearDepthf(0.5f);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Disable depth to establish read-only depth/stencil feedback loop.
    glDepthMask(false);
    glDisable(GL_DEPTH_TEST);

    // Verify we can sample the depth texture and get 0.5.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);

    // Clear depth to another value
    glDepthMask(true);
    glClearDepthf(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Make sure the last clear and the draw are not reordered by mistake.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();

    // Make sure depth is correctly cleared.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    ANGLE_GL_PROGRAM(drawBlue, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(drawBlue, essl1_shaders::PositionAttrib(), 0.95f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    ASSERT_GL_NO_ERROR();
}

// Tests scissored depth/stencil clear after read-only depth/stencil feedback loop draw.
TEST_P(ReadOnlyFeedbackLoopTest, ReadOnlyDepthFeedbackLoopDrawThenScissoredDepthStencilClear)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_read_only_depth_stencil_feedback_loops"));

    constexpr GLuint kSize = 2;
    glViewport(0, 0, kSize, kSize);

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D depth;
void main()
{
    if (abs(texture2D(depth, v_texCoord).x - 0.5) < 0.1)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        gl_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), kFS);
    glUseProgram(program);

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear depth to 0.5.
    glClearDepthf(0.5f);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Disable depth to establish read-only depth/stencil feedback loop.
    glDepthMask(false);
    glDisable(GL_DEPTH_TEST);

    // Verify we can sample the depth texture and get 0.5.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);

    // Clear depth to another value in a scissor
    glDepthMask(true);
    glEnable(GL_SCISSOR_TEST);
    glViewport(kSize / 2, kSize / 2, kSize / 2, kSize / 2);
    glClearDepthf(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Make sure the draw worked.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();

    // Make sure depth is correctly cleared.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    ANGLE_GL_PROGRAM(drawBlue, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(drawBlue, essl1_shaders::PositionAttrib(), 0.95f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::blue);
    ASSERT_GL_NO_ERROR();
}

// Tests depth/stencil blit after read-only depth/stencil feedback loop draw.
TEST_P(ReadOnlyFeedbackLoopTest, ReadOnlyDepthFeedbackLoopDrawThenDepthStencilBlit)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_read_only_depth_stencil_feedback_loops"));

    constexpr GLuint kSize = 2;
    glViewport(0, 0, kSize, kSize);

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D depth;
void main()
{
    if (abs(texture2D(depth, v_texCoord).x - 0.5) < 0.1)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        gl_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), kFS);
    glUseProgram(program);

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear depth to 0.5.
    glClearDepthf(0.5f);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Disable depth to establish read-only depth/stencil feedback loop.
    glDepthMask(false);
    glDisable(GL_DEPTH_TEST);

    // Verify we can sample the depth texture and get 0.5.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);

    // Blit depth to another framebuffer.
    GLFramebuffer framebuffer2;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer2);

    GLTexture colorTexture2;
    glBindTexture(GL_TEXTURE_2D, colorTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture2,
                           0);

    GLTexture depthTexture2;
    glBindTexture(GL_TEXTURE_2D, depthTexture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture2,
                           0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    // Make sure the draw worked.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();

    // Make sure depth is correctly blitted.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer2);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

    ANGLE_GL_PROGRAM(drawBlue, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(drawBlue, essl1_shaders::PositionAttrib(), 0.05f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    ASSERT_GL_NO_ERROR();

    glDepthFunc(GL_LESS);
    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), -0.05f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Tests that if the framebuffer is cleared, a feedback loop between a depth textures and the depth
// buffer is established, and a scissored clear is issued, that the clear is not mistakenly
// scissored.
TEST_P(ReadOnlyFeedbackLoopTest, ReadOnlyDepthFeedbackLoopWithClearAndScissoredDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_read_only_depth_stencil_feedback_loops"));

    constexpr GLuint kSize = 16;
    glViewport(0, 0, kSize, kSize);

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D depth;
void main()
{
    if (abs(texture2D(depth, v_texCoord).x - 0.5) < 0.1)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        gl_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), kFS);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear color to blue and depth to 0.5.
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClearDepthf(0.5f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Disable depth. Although this does not remove the feedback loop as defined by the
    // spec it mimics what gfxbench does in its rendering tests.
    glDepthMask(false);
    glDisable(GL_DEPTH_TEST);

    // Verify we can sample the depth texture and get 0.5.  Use a scissor.
    glScissor(0, 0, kSize / 2, kSize);
    glEnable(GL_SCISSOR_TEST);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);
    ASSERT_GL_NO_ERROR();

    // Make sure the scissored region passes the depth test and is changed to green.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2 - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2 - 1, kSize - 1, GLColor::green);

    // Make sure the region outside the scissor is cleared to blue.
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::blue);
}

// Tests that sampling from stencil while simultaneously bound as read-only attachment works.  Depth
// is being written at the same time.
TEST_P(ReadOnlyFeedbackLoopTestES31, SampleStencilWhileReadOnlyAttachment)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_read_only_depth_stencil_feedback_loops"));

    constexpr GLsizei kSize = 64;

    // Create FBO with color, depth and stencil
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);

    GLTexture depthStencil;
    glBindTexture(GL_TEXTURE_2D, depthStencil);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencil,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Initialize depth/stencil
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0xAA, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    glBindTexture(GL_TEXTURE_2D, 0);
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
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kSize, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Disable stencil output and bind stencil as sampler.
    glDepthFunc(GL_LESS);
    glStencilFunc(GL_EQUAL, 0xAA, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    constexpr char kVS[] = R"(#version 310 es
precision highp float;
in vec4 position;
out vec2 texCoord;

void main()
{
    gl_Position = position;
    texCoord = position.xy * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
precision mediump usampler2D;

in vec2 texCoord;

uniform usampler2D stencil;

out vec4 color;

void main()
{
    bool stencilPass = texture(stencil, texCoord).x == 0xAAu;

    color = vec4(0, stencilPass, 0, 1);
}
)";

    ANGLE_GL_PROGRAM(validateStencil, kVS, kFS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthStencil);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);
    ASSERT_GL_NO_ERROR();

    glUseProgram(validateStencil);
    glUniform1i(glGetUniformLocation(validateStencil, "stencil"), 0);
    ASSERT_GL_NO_ERROR();

    drawQuad(validateStencil, "position", 0.95);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glBindTexture(GL_TEXTURE_2D, copyTex);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, kSize / 2, 0, 0, kSize, kSize / 2);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer readFramebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copyTex, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize / 2, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, kSize / 2, kSize, kSize / 2, GLColor::green);

    // Validate that depth was overwritten in the previous render pass
    glDepthFunc(GL_GREATER);
    glDepthMask(GL_FALSE);

    glUseProgram(drawColor);
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.97);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::blue);
}

// Tests that sampling from depth while simultaneously bound as read-only attachment works.  Stencil
// is being written at the same time.
TEST_P(ReadOnlyFeedbackLoopTestES31, SampleDepthWhileReadOnlyAttachment)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_read_only_depth_stencil_feedback_loops"));

    constexpr GLsizei kSize = 64;

    // Create FBO with color, depth and stencil
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);

    GLTexture depthStencil;
    glBindTexture(GL_TEXTURE_2D, depthStencil);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencil,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Initialize depth/stencil
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0xAA, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    glBindTexture(GL_TEXTURE_2D, 0);
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
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kSize, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Disable depth output and bind depth as sampler.
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);
    glStencilFunc(GL_ALWAYS, 0xBB, 0xEE);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    constexpr char kVS[] = R"(#version 310 es
precision highp float;
in vec4 position;
out vec2 texCoord;

void main()
{
    gl_Position = position;
    texCoord = position.xy * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

in vec2 texCoord;

uniform sampler2D depth;

out vec4 color;

void main()
{
    bool depthPass = abs(texture(depth, texCoord).x - 1.0) < 0.1;

    color = vec4(0, depthPass, 0, 1);
}
)";

    ANGLE_GL_PROGRAM(validateDepth, kVS, kFS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthStencil);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glUseProgram(validateDepth);
    glUniform1i(glGetUniformLocation(validateDepth, "depth"), 0);
    ASSERT_GL_NO_ERROR();

    drawQuad(validateDepth, "position", 0.95);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glBindTexture(GL_TEXTURE_2D, copyTex);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, kSize / 2, 0, 0, kSize, kSize / 2);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer readFramebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copyTex, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize / 2, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, kSize / 2, kSize, kSize / 2, GLColor::green);

    // Validate that stencil was overwritten in the previous render pass
    glStencilFunc(GL_EQUAL, 0xBB, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glUseProgram(drawColor);
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.97);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::blue);
}

// Tests that sampling from depth and stencil while simultaneously bound as read-only attachment
// works.
TEST_P(ReadOnlyFeedbackLoopTestES31, SampleDepthAndStencilWhileReadOnlyAttachment)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_read_only_depth_stencil_feedback_loops"));

    constexpr GLsizei kSize = 64;

    // Create FBO with color, depth and stencil
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);

    GLTexture depthStencil;
    glBindTexture(GL_TEXTURE_2D, depthStencil);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencil,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Initialize depth/stencil
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0xAA, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    glBindTexture(GL_TEXTURE_2D, 0);
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
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kSize, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Disable depth/stencil output and bind both depth and stencil as samplers.
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);
    glStencilFunc(GL_EQUAL, 0xAA, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    constexpr char kVS[] = R"(#version 310 es
precision highp float;
in vec4 position;
out vec2 texCoord;

void main()
{
    gl_Position = position;
    texCoord = position.xy * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
precision mediump usampler2D;

in vec2 texCoord;

uniform sampler2D depth;
uniform usampler2D stencil;

out vec4 color;

void main()
{
    // Note: Due to GL_DEPTH_STENCIL_TEXTURE_MODE, it is not possible to read both the depth and
    // stencil aspects correctly.  For the sake of test (reading both aspects), just make sure to
    // sample from depth.
    bool depthPass = !isinf(texture(depth, texCoord).x);
    bool stencilPass = texture(stencil, texCoord).x == 0xAAu;

    color = vec4(0, depthPass, stencilPass, 1);
}
)";

    ANGLE_GL_PROGRAM(validateDepthStencil, kVS, kFS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthStencil);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthStencil);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);
    ASSERT_GL_NO_ERROR();

    glUseProgram(validateDepthStencil);
    glUniform1i(glGetUniformLocation(validateDepthStencil, "depth"), 0);
    glUniform1i(glGetUniformLocation(validateDepthStencil, "stencil"), 1);
    ASSERT_GL_NO_ERROR();

    drawQuad(validateDepthStencil, "position", 0.95);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glBindTexture(GL_TEXTURE_2D, copyTex);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, kSize / 2, 0, 0, kSize, kSize / 2);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer readFramebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copyTex, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize / 2, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, kSize / 2, kSize, kSize / 2, GLColor::cyan);
}

// Instantiate the test for ES3.
ANGLE_INSTANTIATE_TEST_ES3(ReadOnlyFeedbackLoopTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ReadOnlyFeedbackLoopTestES31);
ANGLE_INSTANTIATE_TEST_ES31(ReadOnlyFeedbackLoopTestES31);
