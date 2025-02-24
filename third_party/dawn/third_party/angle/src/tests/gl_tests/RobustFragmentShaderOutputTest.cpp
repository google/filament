//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RobustFragmentShaderOutputTest: Tests for the custom ANGLE extension.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

namespace
{
bool ExtEnabled()
{
    return IsGLExtensionEnabled("GL_ANGLE_robust_fragment_shader_output");
}
}  // namespace

class RobustFragmentShaderOutputTest : public ANGLETest<>
{
  public:
    RobustFragmentShaderOutputTest() {}
};

// Basic behaviour from the extension.
TEST_P(RobustFragmentShaderOutputTest, Basic)
{
    ANGLE_SKIP_TEST_IF(!ExtEnabled());

    const char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 outvar;
void main() {
    outvar = vec4(0.0, 1.0, 0.0, 1.0);
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    constexpr GLsizei kSize = 2;
    std::vector<GLColor> bluePixels(kSize * kSize, GLColor::blue);

    GLTexture texA;
    glBindTexture(GL_TEXTURE_2D, texA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 bluePixels.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texA, 0);

    GLTexture texB;
    glBindTexture(GL_TEXTURE_2D, texB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 bluePixels.data());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texB, 0);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Verify initial attachment colors (blue).
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    constexpr std::array<GLenum, 2> kDrawBuffers = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, kDrawBuffers.data());
    glViewport(0, 0, kSize, kSize);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    // Draw, verify first attachment is updated (green) and second is unchanged (blue).
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Test that changing framebuffer attachments work
TEST_P(RobustFragmentShaderOutputTest, ChangingFramebufferAttachments)
{
    ANGLE_SKIP_TEST_IF(!ExtEnabled());

    const char kFS[] = R"(#version 300 es
precision mediump float;
layout(location = 1) out vec4 color1;
layout(location = 3) out vec4 color2;

uniform vec4 color1In;
uniform vec4 color2In;

void main() {
    color1 = color1In;
    color2 = color2In;
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint color1Loc = glGetUniformLocation(program, "color1In");
    ASSERT_NE(color1Loc, -1);
    GLint color2Loc = glGetUniformLocation(program, "color2In");
    ASSERT_NE(color2Loc, -1);

    constexpr GLsizei kSize = 2;
    const std::vector<GLColor> initColor(kSize * kSize, GLColor::transparentBlack);

    GLTexture color1;
    glBindTexture(GL_TEXTURE_2D, color1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 initColor.data());

    GLTexture color2;
    glBindTexture(GL_TEXTURE_2D, color2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 initColor.data());

    GLTexture color3;
    glBindTexture(GL_TEXTURE_2D, color3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 initColor.data());

    // Draw with matching framebuffer attachments
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, color1, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, color2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    std::array<GLenum, 4> drawBuffers = {GL_NONE, GL_COLOR_ATTACHMENT1, GL_NONE,
                                         GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, drawBuffers.data());
    glViewport(0, 0, kSize, kSize);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // Draw once, should update color1 and color2
    glUniform4f(color1Loc, 1, 0, 0, 0);
    glUniform4f(color2Loc, 0, 1, 0, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    // Change the framebuffer attachments, this time adding an attachment that's not written to.
    // This attachment should be unmodified.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color3, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    drawBuffers[0] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(4, drawBuffers.data());

    glUniform4f(color1Loc, 0, 0, 1, 1);
    glUniform4f(color2Loc, 0, 0, 0, 1);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    // Shuffle attachments.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color1, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, color2, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, color3, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glUniform4f(color1Loc, 0, 0, 1, 0);
    glUniform4f(color2Loc, 1, 0, 0, 1);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    // Switch back to matching attachment count
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    drawBuffers[0] = GL_NONE;
    glDrawBuffers(4, drawBuffers.data());

    glUniform4f(color1Loc, 1, 0, 0, 0);
    glUniform4f(color2Loc, 0, 1, 0, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    // Verify results
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color1, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::magenta);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::white);

    glReadBuffer(GL_COLOR_ATTACHMENT3);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::yellow);

    ASSERT_GL_NO_ERROR();
}

// Test that changing framebuffers work
TEST_P(RobustFragmentShaderOutputTest, ChangingFramebuffers)
{
    ANGLE_SKIP_TEST_IF(!ExtEnabled());

    const char kFS[] = R"(#version 300 es
precision mediump float;
layout(location = 1) out vec4 color1;
layout(location = 3) out vec4 color2;

uniform vec4 color1In;
uniform vec4 color2In;

void main() {
    color1 = color1In;
    color2 = color2In;
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint color1Loc = glGetUniformLocation(program, "color1In");
    ASSERT_NE(color1Loc, -1);
    GLint color2Loc = glGetUniformLocation(program, "color2In");
    ASSERT_NE(color2Loc, -1);

    constexpr GLsizei kSize = 2;
    const std::vector<GLColor> initColor(kSize * kSize, GLColor::transparentBlack);

    GLTexture color1;
    glBindTexture(GL_TEXTURE_2D, color1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 initColor.data());

    GLTexture color2;
    glBindTexture(GL_TEXTURE_2D, color2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 initColor.data());

    GLTexture color3;
    glBindTexture(GL_TEXTURE_2D, color3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 initColor.data());

    GLFramebuffer framebuffer1;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, color1, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, color2, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLFramebuffer framebuffer2;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, color1, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, color2, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color3, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLFramebuffer framebuffer3;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer3);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color1, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, color2, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, color3, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLFramebuffer framebuffer4;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer4);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, color2, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, color3, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw with matching framebuffer attachments
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);

    std::array<GLenum, 4> drawBuffers = {GL_NONE, GL_COLOR_ATTACHMENT1, GL_NONE,
                                         GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, drawBuffers.data());
    glViewport(0, 0, kSize, kSize);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // Draw once, should update color1 and color2
    glUniform4f(color1Loc, 1, 0, 0, 0);
    glUniform4f(color2Loc, 0, 1, 0, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    // Change the framebuffer attachments, this time adding an attachment that's not written to.
    // This attachment should be unmodified.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);

    drawBuffers[0] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(4, drawBuffers.data());

    glUniform4f(color1Loc, 0, 0, 1, 1);
    glUniform4f(color2Loc, 0, 0, 0, 1);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    // Shuffle attachments.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer3);
    glDrawBuffers(4, drawBuffers.data());

    glUniform4f(color1Loc, 0, 0, 1, 0);
    glUniform4f(color2Loc, 1, 0, 0, 1);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    // Switch back to matching attachment count
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer4);

    drawBuffers[0] = GL_NONE;
    glDrawBuffers(4, drawBuffers.data());

    glUniform4f(color1Loc, 1, 0, 0, 0);
    glUniform4f(color2Loc, 0, 1, 0, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    // Verify results
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer3);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::magenta);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::white);

    glReadBuffer(GL_COLOR_ATTACHMENT3);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::yellow);

    ASSERT_GL_NO_ERROR();
}

// Test that changing program outputs work
TEST_P(RobustFragmentShaderOutputTest, ChangingProgramOutputs)
{
    ANGLE_SKIP_TEST_IF(!ExtEnabled());

    const char kFS1[] = R"(#version 300 es
precision mediump float;
layout(location = 1) out vec4 color1;

uniform vec4 color1In;

void main() {
    color1 = color1In;
})";

    const char kFS2[] = R"(#version 300 es
precision mediump float;
layout(location = 1) out vec4 color1;
layout(location = 3) out vec4 color2;

uniform vec4 color1In;
uniform vec4 color2In;

void main() {
    color1 = color1In;
    color2 = color2In;
})";

    const char kFS3[] = R"(#version 300 es
precision mediump float;
layout(location = 0) out vec4 color1;
layout(location = 1) out vec4 color2;
layout(location = 3) out vec4 color3;

uniform vec4 color1In;
uniform vec4 color2In;
uniform vec4 color3In;

void main() {
    color1 = color1In;
    color2 = color2In;
    color3 = color3In;
})";

    ANGLE_GL_PROGRAM(program1, essl3_shaders::vs::Simple(), kFS1);
    glUseProgram(program1);
    GLint program1Color1Loc = glGetUniformLocation(program1, "color1In");
    ASSERT_NE(program1Color1Loc, -1);

    ANGLE_GL_PROGRAM(program2, essl3_shaders::vs::Simple(), kFS2);
    glUseProgram(program2);
    GLint program2Color1Loc = glGetUniformLocation(program2, "color1In");
    ASSERT_NE(program2Color1Loc, -1);
    GLint program2Color2Loc = glGetUniformLocation(program2, "color2In");
    ASSERT_NE(program2Color2Loc, -1);

    ANGLE_GL_PROGRAM(program3, essl3_shaders::vs::Simple(), kFS3);
    glUseProgram(program3);
    GLint program3Color1Loc = glGetUniformLocation(program3, "color1In");
    ASSERT_NE(program2Color1Loc, -1);
    GLint program3Color2Loc = glGetUniformLocation(program3, "color2In");
    ASSERT_NE(program2Color2Loc, -1);
    GLint program3Color3Loc = glGetUniformLocation(program3, "color3In");
    ASSERT_NE(program3Color3Loc, -1);

    constexpr GLsizei kSize = 2;
    const std::vector<GLColor> initColor(kSize * kSize, GLColor::transparentBlack);

    GLTexture color1;
    glBindTexture(GL_TEXTURE_2D, color1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 initColor.data());

    GLTexture color2;
    glBindTexture(GL_TEXTURE_2D, color2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 initColor.data());

    GLTexture color3;
    glBindTexture(GL_TEXTURE_2D, color3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 initColor.data());

    // Draw with matching framebuffer attachments
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color1, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, color2, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, color3, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    std::array<GLenum, 4> drawBuffers = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_NONE,
                                         GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, drawBuffers.data());
    glViewport(0, 0, kSize, kSize);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // Draw with 2 outputs, should update color2 and color3
    glUseProgram(program2);
    glUniform4f(program2Color1Loc, 0.6, 0, 0, 0);
    glUniform4f(program2Color2Loc, 0, 0.7, 0, 0);
    drawQuad(program2, essl3_shaders::PositionAttrib(), 0.5f);

    // Draw with 1 output, should update color2
    glUseProgram(program1);
    glUniform4f(program1Color1Loc, 0.5, 0, 0, 0);
    drawQuad(program1, essl3_shaders::PositionAttrib(), 0.5f);

    // Draw with 3 outputs, should update all
    glUseProgram(program3);
    glUniform4f(program3Color1Loc, 0, 0, 1, 1);
    glUniform4f(program3Color2Loc, 0, 1, 0, 0);
    glUniform4f(program3Color3Loc, 0, 0.4, 0, 0);
    drawQuad(program3, essl3_shaders::PositionAttrib(), 0.5f);

    // Draw with 2 outputs again, should update color2 and color3
    glUseProgram(program2);
    glUniform4f(program2Color1Loc, 0, 0, 0, 1);
    glUniform4f(program2Color2Loc, 0, 0, 0, 1);
    drawQuad(program2, essl3_shaders::PositionAttrib(), 0.5f);

    // Verify results
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::blue);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::yellow);

    glReadBuffer(GL_COLOR_ATTACHMENT3);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize, kSize, GLColor::green);

    ASSERT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES3(RobustFragmentShaderOutputTest);
