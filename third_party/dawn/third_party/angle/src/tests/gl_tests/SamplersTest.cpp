//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SamplerTest.cpp : Tests for samplers.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace angle
{

class SamplersTest : public ANGLETest<>
{
  protected:
    SamplersTest() {}

    // Sets a value for GL_TEXTURE_MAX_ANISOTROPY_EXT and expects it to fail.
    void validateInvalidAnisotropy(GLSampler &sampler, float invalidValue)
    {
        glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, invalidValue);
        EXPECT_GL_ERROR(GL_INVALID_VALUE);
    }

    // Sets a value for GL_TEXTURE_MAX_ANISOTROPY_EXT and expects it to work.
    void validateValidAnisotropy(GLSampler &sampler, float validValue)
    {
        glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, validValue);
        EXPECT_GL_NO_ERROR();

        GLfloat valueToVerify = 0.0f;
        glGetSamplerParameterfv(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, &valueToVerify);
        ASSERT_EQ(valueToVerify, validValue);
    }
};

class SamplersTest31 : public SamplersTest
{};

// Verify that samplerParameterf supports TEXTURE_MAX_ANISOTROPY_EXT valid values.
TEST_P(SamplersTest, ValidTextureSamplerMaxAnisotropyExt)
{
    GLSampler sampler;

    // Exact min
    validateValidAnisotropy(sampler, 1.0f);

    GLfloat maxValue = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxValue);

    // Max value
    validateValidAnisotropy(sampler, maxValue - 1);

    // In-between
    GLfloat between = (1.0f + maxValue) / 2;
    validateValidAnisotropy(sampler, between);
}

// Verify an error is thrown if we try to go under the minimum value for
// GL_TEXTURE_MAX_ANISOTROPY_EXT
TEST_P(SamplersTest, InvalidUnderTextureSamplerMaxAnisotropyExt)
{
    GLSampler sampler;

    // Under min
    validateInvalidAnisotropy(sampler, 0.0f);
}

// Verify an error is thrown if we try to go over the max value for
// GL_TEXTURE_MAX_ANISOTROPY_EXT
TEST_P(SamplersTest, InvalidOverTextureSamplerMaxAnisotropyExt)
{
    GLSampler sampler;

    GLfloat maxValue = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxValue);
    maxValue += 1;

    validateInvalidAnisotropy(sampler, maxValue);
}

// Test that updating a sampler uniform in a program behaves correctly.
TEST_P(SamplersTest31, SampleTextureAThenTextureB)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr int kWidth  = 2;
    constexpr int kHeight = 2;

    const GLchar *vertString = R"(#version 310 es
precision highp float;
in vec2 a_position;
out vec2 texCoord;
void main()
{
    gl_Position = vec4(a_position, 0, 1);
    texCoord = a_position * 0.5 + vec2(0.5);
})";

    const GLchar *fragString = R"(#version 310 es
precision highp float;
in vec2 texCoord;
uniform sampler2D tex;
out vec4 my_FragColor;
void main()
{
    my_FragColor = texture(tex, texCoord);
})";

    std::array<GLColor, kWidth * kHeight> redColor = {
        {GLColor::red, GLColor::red, GLColor::red, GLColor::red}};
    std::array<GLColor, kWidth * kHeight> greenColor = {
        {GLColor::green, GLColor::green, GLColor::green, GLColor::green}};

    // Create a red texture and bind to texture unit 0
    GLTexture redTex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, redTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 redColor.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    // Create a green texture and bind to texture unit 1
    GLTexture greenTex;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, greenTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 greenColor.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glActiveTexture(GL_TEXTURE0);
    ASSERT_GL_NO_ERROR();

    GLProgram program;
    program.makeRaster(vertString, fragString);
    ASSERT_NE(0u, program);
    glUseProgram(program);

    GLint location = glGetUniformLocation(program, "tex");
    ASSERT_NE(location, -1);
    ASSERT_GL_NO_ERROR();

    // Draw red
    glUniform1i(location, 0);
    ASSERT_GL_NO_ERROR();
    drawQuad(program, "a_position", 0.5f);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    // Draw green
    glUniform1i(location, 1);
    ASSERT_GL_NO_ERROR();
    drawQuad(program, "a_position", 0.5f);
    ASSERT_GL_NO_ERROR();

    // Draw red
    glUniform1i(location, 0);
    ASSERT_GL_NO_ERROR();
    drawQuad(program, "a_position", 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::yellow);
}

// Samplers are only supported on ES3.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SamplersTest);
ANGLE_INSTANTIATE_TEST_ES3(SamplersTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SamplersTest31);
ANGLE_INSTANTIATE_TEST_ES31(SamplersTest31);
}  // namespace angle
