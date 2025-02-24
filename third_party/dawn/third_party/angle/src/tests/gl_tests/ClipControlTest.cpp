//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Test cases for GL_EXT_clip_control
// These tests complement dEQP-GLES2.functional.clip_control.*
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/test_utils.h"

using namespace angle;

class ClipControlTest : public ANGLETest<>
{
  protected:
    static const int w = 64;
    static const int h = 64;

    ClipControlTest()
    {
        setWindowWidth(w);
        setWindowHeight(h);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setExtensionsEnabled(false);
    }
};

// Test state queries and updates
TEST_P(ClipControlTest, StateQuery)
{
    // Queries with the extension disabled
    GLint clipOrigin = -1;
    glGetIntegerv(GL_CLIP_ORIGIN_EXT, &clipOrigin);
    EXPECT_EQ(clipOrigin, -1);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    GLint clipDepthMode = -1;
    glGetIntegerv(GL_CLIP_DEPTH_MODE_EXT, &clipDepthMode);
    EXPECT_EQ(clipDepthMode, -1);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Command with the extension disabled
    glClipControlEXT(GL_UPPER_LEFT_EXT, GL_ZERO_TO_ONE_EXT);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_clip_control"));
    ASSERT_GL_NO_ERROR();

    // Default state with the extension enabled
    glGetIntegerv(GL_CLIP_ORIGIN_EXT, &clipOrigin);
    glGetIntegerv(GL_CLIP_DEPTH_MODE_EXT, &clipDepthMode);
    EXPECT_GLENUM_EQ(GL_LOWER_LEFT_EXT, clipOrigin);
    EXPECT_GLENUM_EQ(GL_NEGATIVE_ONE_TO_ONE_EXT, clipDepthMode);

    ASSERT_GL_NO_ERROR();

    // Check valid state updates
    glClipControlEXT(GL_LOWER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);
    glGetIntegerv(GL_CLIP_ORIGIN_EXT, &clipOrigin);
    glGetIntegerv(GL_CLIP_DEPTH_MODE_EXT, &clipDepthMode);
    EXPECT_GLENUM_EQ(GL_LOWER_LEFT_EXT, clipOrigin);
    EXPECT_GLENUM_EQ(GL_NEGATIVE_ONE_TO_ONE_EXT, clipDepthMode);

    glClipControlEXT(GL_LOWER_LEFT_EXT, GL_ZERO_TO_ONE_EXT);
    glGetIntegerv(GL_CLIP_ORIGIN_EXT, &clipOrigin);
    glGetIntegerv(GL_CLIP_DEPTH_MODE_EXT, &clipDepthMode);
    EXPECT_GLENUM_EQ(GL_LOWER_LEFT_EXT, clipOrigin);
    EXPECT_GLENUM_EQ(GL_ZERO_TO_ONE_EXT, clipDepthMode);

    glClipControlEXT(GL_UPPER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);
    glGetIntegerv(GL_CLIP_ORIGIN_EXT, &clipOrigin);
    glGetIntegerv(GL_CLIP_DEPTH_MODE_EXT, &clipDepthMode);
    EXPECT_GLENUM_EQ(GL_UPPER_LEFT_EXT, clipOrigin);
    EXPECT_GLENUM_EQ(GL_NEGATIVE_ONE_TO_ONE_EXT, clipDepthMode);

    glClipControlEXT(GL_UPPER_LEFT_EXT, GL_ZERO_TO_ONE_EXT);
    glGetIntegerv(GL_CLIP_ORIGIN_EXT, &clipOrigin);
    glGetIntegerv(GL_CLIP_DEPTH_MODE_EXT, &clipDepthMode);
    EXPECT_GLENUM_EQ(GL_UPPER_LEFT_EXT, clipOrigin);
    EXPECT_GLENUM_EQ(GL_ZERO_TO_ONE_EXT, clipDepthMode);

    ASSERT_GL_NO_ERROR();

    // Check invalid state updates
    glClipControlEXT(GL_LOWER_LEFT_EXT, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glClipControlEXT(0, GL_NEGATIVE_ONE_TO_ONE_EXT);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Invalid command does not change the state
    glGetIntegerv(GL_CLIP_ORIGIN_EXT, &clipOrigin);
    glGetIntegerv(GL_CLIP_DEPTH_MODE_EXT, &clipDepthMode);
    EXPECT_GLENUM_EQ(GL_UPPER_LEFT_EXT, clipOrigin);
    EXPECT_GLENUM_EQ(GL_ZERO_TO_ONE_EXT, clipDepthMode);

    ASSERT_GL_NO_ERROR();
}

// Test that clip origin does not affect scissored clears
TEST_P(ClipControlTest, OriginScissorClear)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_clip_control"));

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    auto test = [&](std::string name, bool useES3) {
        // Start with lower-left
        glClipControlEXT(GL_LOWER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);

        // Make a draw call without color writes to sync the state
        glColorMask(false, false, false, false);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);
        glColorMask(true, true, true, true);

        // Clear to red
        glDisable(GL_SCISSOR_TEST);
        if (useES3)
        {
            float color[4] = {1.0, 0.0, 0.0, 1.0};
            glClearBufferfv(GL_COLOR, 0, color);
        }
        else
        {
            glClearColor(1.0, 0.0, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        // Flip the clip origin
        glClipControlEXT(GL_UPPER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);

        // Make a draw call without color writes to sync the state
        glColorMask(false, false, false, false);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);
        glColorMask(true, true, true, true);

        // Clear lower half to green
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, w, h / 2);
        if (useES3)
        {
            float color[4] = {0.0, 1.0, 0.0, 1.0};
            glClearBufferfv(GL_COLOR, 0, color);
        }
        else
        {
            glClearColor(0.0, 1.0, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(0.5 * w, 0.25 * h, GLColor::green) << name;
        EXPECT_PIXEL_COLOR_EQ(0.5 * w, 0.75 * h, GLColor::red) << name;
    };

    test("Default framebuffer", getClientMajorVersion() > 2);

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_rgb8_rgba8"));

    GLRenderbuffer rb;
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    test("User framebuffer", getClientMajorVersion() > 2);
}

// Test that changing clip origin state does not affect location of scissor area
TEST_P(ClipControlTest, OriginScissorDraw)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_clip_control"));

    constexpr char kVS[] = R"(
attribute vec2 a_position;
void main()
{
    // Square at (0.25, 0.25) -> (0.75, 0.75)
    gl_Position = vec4(a_position * 0.25 + 0.5, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Blue());
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    auto test = [&](std::string name) {
        glClipControlEXT(GL_LOWER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);

        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw only to the lower half
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, w, h / 2);

        // Draw blue quad in the upper-right part of the framebuffer; scissor test must fail
        drawQuad(program, "a_position", 0);
        ASSERT_GL_NO_ERROR();

        // Switch the clip origin and draw again; scissor test must pass
        glClipControlEXT(GL_UPPER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);
        drawQuad(program, "a_position", 0);
        ASSERT_GL_NO_ERROR();

        // Reads are unaffected by clip origin
        EXPECT_PIXEL_COLOR_EQ(0.75 * w, 0.25 * h, GLColor::blue) << name;
        EXPECT_PIXEL_COLOR_EQ(0.75 * w, 0.75 * h, GLColor::transparentBlack) << name;
    };

    test("Default framebuffer");

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    GLRenderbuffer rb;
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    test("User framebuffer");
}

// Test that changing clip origin state does not affect copyTexImage
TEST_P(ClipControlTest, OriginCopyTexImage)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_clip_control"));

    auto test = [&](std::string name) {
        glClipControlEXT(GL_LOWER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);

        // Clear to red
        glDisable(GL_SCISSOR_TEST);
        glClearColor(1.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Clear lower half-space to green
        glEnable(GL_SCISSOR_TEST);
        glScissor(0, 0, w, h / 2);
        glClearColor(0.0, 1.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Switch clip origin state, it must have no effect on the next commands
        glClipControlEXT(GL_UPPER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);

        GLTexture tex;
        glBindTexture(GL_TEXTURE_2D, tex);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, w / 4, h / 4, 0);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer readFb;
        glBindFramebuffer(GL_FRAMEBUFFER, readFb);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Copied texture must contain values from the lower half-space
        EXPECT_PIXEL_COLOR_EQ(w / 8, h / 8, GLColor::green) << name;
    };

    test("Default framebuffer");

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    GLRenderbuffer rb;
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    test("User framebuffer");
}

// Test that changing clip origin state does not affect copyTexImage
// with Luma format that may use draw calls internally
TEST_P(ClipControlTest, OriginCopyTexImageLuma)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_clip_control"));

    glClipControlEXT(GL_LOWER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);

    // Clear to zero
    glClear(GL_COLOR_BUFFER_BIT);

    // Clear lower half-space to one
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, w, h / 2);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    // Switch clip origin state, it must have no effect on the next commands
    glClipControlEXT(GL_UPPER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 0, 0, w, h, 0);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);

    // Draw the luma texture
    glClipControlEXT(GL_LOWER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);
    ANGLE_GL_PROGRAM(drawTexture, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(drawTexture, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(w / 2, h * 1 / 4, GLColor::white);
    EXPECT_PIXEL_COLOR_EQ(w / 2, h * 3 / 4, GLColor::black);
}

// Test that clip origin does not affect gl_FragCoord
TEST_P(ClipControlTest, OriginFragCoord)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_clip_control"));

    const char kFS[] = R"(precision mediump float;
void main()
{
    gl_FragColor = vec4(sign(gl_FragCoord.xy / 64.0 - 0.5), 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);

    glClearColor(1.0, 0.0, 1.0, 1.0);

    for (GLenum origin : {GL_LOWER_LEFT_EXT, GL_UPPER_LEFT_EXT})
    {
        glClipControlEXT(origin, GL_NEGATIVE_ONE_TO_ONE_EXT);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
        EXPECT_PIXEL_COLOR_EQ(w - 1, 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(0, h - 1, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(w - 1, h - 1, GLColor::yellow);

        GLFramebuffer fb;
        glBindFramebuffer(GL_FRAMEBUFFER, fb);

        GLRenderbuffer rb;
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
        EXPECT_PIXEL_COLOR_EQ(w - 1, 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(0, h - 1, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(w - 1, h - 1, GLColor::yellow);
    }
}

// Test that clip origin does not affect gl_PointCoord
TEST_P(ClipControlTest, OriginPointCoord)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_clip_control"));

    float pointSizeRange[2] = {};
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange);
    ANGLE_SKIP_TEST_IF(pointSizeRange[1] < 32);

    const char kVS[] = R"(precision mediump float;
void main()
{
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    gl_PointSize = 32.0;
})";

    const char kFS[] = R"(precision mediump float;
void main()
{
    gl_FragColor = vec4(sign(gl_PointCoord.xy - 0.5), 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 0.0, 1.0, 1.0);

    for (GLenum origin : {GL_LOWER_LEFT_EXT, GL_UPPER_LEFT_EXT})
    {
        glClipControlEXT(origin, GL_NEGATIVE_ONE_TO_ONE_EXT);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, 1);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(w / 2 - 15, h / 2 + 15, GLColor::black);
        EXPECT_PIXEL_COLOR_EQ(w / 2 + 15, h / 2 + 15, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(w / 2 - 15, h / 2 - 15, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(w / 2 + 15, h / 2 - 15, GLColor::yellow);

        GLFramebuffer fb;
        glBindFramebuffer(GL_FRAMEBUFFER, fb);

        GLRenderbuffer rb;
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, 1);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(w / 2 - 15, h / 2 + 15, GLColor::black);
        EXPECT_PIXEL_COLOR_EQ(w / 2 + 15, h / 2 + 15, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(w / 2 - 15, h / 2 - 15, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(w / 2 + 15, h / 2 - 15, GLColor::yellow);
    }
}

// Test that clip origin does not affect gl_FrontFacing
TEST_P(ClipControlTest, OriginFrontFacing)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_clip_control"));

    const char kFS[] = R"(precision mediump float;
void main()
{
    gl_FragColor = vec4(gl_FrontFacing ? vec2(1.0, 0.0) : vec2(0.0, 1.0), 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);

    for (GLenum origin : {GL_LOWER_LEFT_EXT, GL_UPPER_LEFT_EXT})
    {
        glClipControlEXT(origin, GL_NEGATIVE_ONE_TO_ONE_EXT);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glFrontFace(GL_CCW);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

        glFrontFace(GL_CW);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

        GLFramebuffer fb;
        glBindFramebuffer(GL_FRAMEBUFFER, fb);

        GLRenderbuffer rb;
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glFrontFace(GL_CCW);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

        glFrontFace(GL_CW);
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
}

// Test that clip origin does not affect readPixels
TEST_P(ClipControlTest, OriginReadPixels)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_clip_control"));

    const char kFS[] = R"(precision mediump float;
uniform float blue;
varying vec4 v_position;
void main()
{
    gl_FragColor = (v_position.y > 0.0) ? vec4(1.0, 0.0, blue, 1.0) : vec4(0.0, 1.0, blue, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), kFS);
    const GLint blueUniformLocation = glGetUniformLocation(program, "blue");
    glUseProgram(program);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glUniform1f(blueUniformLocation, 0.0f);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);
    EXPECT_PIXEL_RECT_EQ(0, h / 2 + 2, w, h / 2 - 2, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, 0, w, h / 2 - 2, GLColor::green);

    // Update clip origin and make a draw call that fails the depth test to
    // ensure that the backend is synced while the framebuffer is unchanged.
    glClipControlEXT(GL_UPPER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);
    glUniform1f(blueUniformLocation, 1.0f);
    drawQuad(program, essl1_shaders::PositionAttrib(), 1.0);

    // Check that the second draw call has failed the depth test and
    // reading from the framebuffer returns the same values as before.
    EXPECT_PIXEL_RECT_EQ(0, h / 2 + 2, w, h / 2 - 2, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(0, 0, w, h / 2 - 2, GLColor::green);
}

// Test that changing only the clip depth mode syncs the state correctly
TEST_P(ClipControlTest, DepthModeSimple)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_clip_control"));

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    const GLint colorUniformLocation = glGetUniformLocation(program, essl1_shaders::ColorUniform());
    glUseProgram(program);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glUniform4fv(colorUniformLocation, 1, GLColor::red.toNormalizedVector().data());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);

    glClipControlEXT(GL_LOWER_LEFT_EXT, GL_ZERO_TO_ONE_EXT);

    glUniform4fv(colorUniformLocation, 1, GLColor::green.toNormalizedVector().data());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that gl_FragCoord.z has expected values for ZERO_TO_ONE clip depth mode
TEST_P(ClipControlTest, DepthFragCoord)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_clip_control"));

    const char kFS[] = R"(precision mediump float;
void main()
{
    gl_FragColor = vec4(gl_FragCoord.z, 0, 0, 1);
})";
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);

    glClipControlEXT(GL_LOWER_LEFT_EXT, GL_ZERO_TO_ONE_EXT);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 1.0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(127, 0, 0, 255), 1);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
}

class ClipControlTestES3 : public ClipControlTest
{};

// Test that clip origin state does not affect framebuffer blits
TEST_P(ClipControlTestES3, OriginBlit)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_clip_control"));

    constexpr char kFS[] = R"(
precision mediump float;
varying vec4 v_position;
void main()
{
    gl_FragColor = vec4(sign(v_position.xy), 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), kFS);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    // Blit default to custom
    {
        GLFramebuffer fb;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);

        GLRenderbuffer rb;
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClipControlEXT(GL_LOWER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);
        drawQuad(program, "a_position", 0);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
        glClipControlEXT(GL_UPPER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);
        glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
        EXPECT_PIXEL_COLOR_EQ(0.25 * w, 0.25 * h, GLColor::black);
        EXPECT_PIXEL_COLOR_EQ(0.75 * w, 0.25 * h, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(0.25 * w, 0.75 * h, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(0.75 * w, 0.75 * h, GLColor::yellow);
    }

    // Blit custom to default
    {
        GLFramebuffer fb;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);

        GLRenderbuffer rb;
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
        glClipControlEXT(GL_LOWER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);
        drawQuad(program, "a_position", 0);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClipControlEXT(GL_UPPER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);
        glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        EXPECT_PIXEL_COLOR_EQ(0.25 * w, 0.25 * h, GLColor::black);
        EXPECT_PIXEL_COLOR_EQ(0.75 * w, 0.25 * h, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(0.25 * w, 0.75 * h, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(0.75 * w, 0.75 * h, GLColor::yellow);
    }

    // Blit custom to custom
    {
        GLFramebuffer fb1;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb1);

        GLRenderbuffer rb1;
        glBindRenderbuffer(GL_RENDERBUFFER, rb1);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb1);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

        GLFramebuffer fb2;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb2);

        GLRenderbuffer rb2;
        glBindRenderbuffer(GL_RENDERBUFFER, rb2);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb2);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb1);
        glClipControlEXT(GL_LOWER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);
        drawQuad(program, "a_position", 0);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fb1);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb2);
        glClipControlEXT(GL_UPPER_LEFT_EXT, GL_NEGATIVE_ONE_TO_ONE_EXT);
        glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fb2);
        EXPECT_PIXEL_COLOR_EQ(0.25 * w, 0.25 * h, GLColor::black);
        EXPECT_PIXEL_COLOR_EQ(0.75 * w, 0.25 * h, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(0.25 * w, 0.75 * h, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(0.75 * w, 0.75 * h, GLColor::yellow);
    }
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(ClipControlTest,
                                       ES2_OPENGLES().enable(Feature::EmulateClipOrigin),
                                       ES3_OPENGLES().enable(Feature::EmulateClipOrigin));
ANGLE_INSTANTIATE_TEST_ES3_AND(ClipControlTestES3,
                               ES3_OPENGLES().enable(Feature::EmulateClipOrigin));
