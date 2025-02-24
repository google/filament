//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PackUnpackTest:
//   Tests the corrrectness of opengl 4.1 emulation of pack/unpack built-in functions.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

class PackUnpackTest : public ANGLETest<>
{
  protected:
    PackUnpackTest()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        // Fragment Shader source
        constexpr char kSNormFS[] = R"(#version 300 es
precision mediump float;
uniform mediump vec2 v;
layout(location = 0) out mediump vec4 fragColor;

void main()
{
    highp uint u = packSnorm2x16(v);
    vec2 r = unpackSnorm2x16(u);
    fragColor = vec4(r, 0.0, 1.0);
})";

        // Fragment Shader source
        constexpr char kUNormFS[] = R"(#version 300 es
precision mediump float;
uniform mediump vec2 v;
layout(location = 0) out mediump vec4 fragColor;

void main()
{
    highp uint u = packUnorm2x16(v);
    vec2 r = unpackUnorm2x16(u);
    fragColor = vec4(r, 0.0, 1.0);
})";

        // Fragment Shader source
        constexpr char kHalfFS[] = R"(#version 300 es
precision mediump float;
uniform mediump vec2 v;
layout(location = 0) out mediump vec4 fragColor;

void main()
{
    highp uint u = packHalf2x16(v);
    vec2 r = unpackHalf2x16(u);
    fragColor = vec4(r, 0.0, 1.0);
})";

        mSNormProgram = CompileProgram(essl3_shaders::vs::Simple(), kSNormFS);
        mUNormProgram = CompileProgram(essl3_shaders::vs::Simple(), kUNormFS);
        mHalfProgram  = CompileProgram(essl3_shaders::vs::Simple(), kHalfFS);
        if (mSNormProgram == 0 || mUNormProgram == 0 || mHalfProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        glGenTextures(1, &mOffscreenTexture2D);
        glBindTexture(GL_TEXTURE_2D, mOffscreenTexture2D);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG32F, getWindowWidth(), getWindowHeight());

        glGenFramebuffers(1, &mOffscreenFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mOffscreenFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               mOffscreenTexture2D, 0);

        glViewport(0, 0, 16, 16);

        const GLfloat color[] = {1.0f, 1.0f, 0.0f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, color);
    }

    void testTearDown() override
    {
        glDeleteTextures(1, &mOffscreenTexture2D);
        glDeleteFramebuffers(1, &mOffscreenFramebuffer);
        glDeleteProgram(mSNormProgram);
        glDeleteProgram(mUNormProgram);
        glDeleteProgram(mHalfProgram);
    }

    void compareBeforeAfter(GLuint program, float input1, float input2)
    {
        compareBeforeAfter(program, input1, input2, input1, input2);
    }

    void compareBeforeAfter(GLuint program,
                            float input1,
                            float input2,
                            float expect1,
                            float expect2)
    {
        GLint vec2Location = glGetUniformLocation(program, "v");

        glUseProgram(program);
        glUniform2f(vec2Location, input1, input2);

        drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

        ASSERT_GL_NO_ERROR();

        GLfloat p[2] = {0};
        glReadPixels(8, 8, 1, 1, GL_RG, GL_FLOAT, p);

        ASSERT_GL_NO_ERROR();

        static const double epsilon = 0.0005;
        EXPECT_NEAR(p[0], expect1, epsilon);
        EXPECT_NEAR(p[1], expect2, epsilon);
    }

    GLuint mSNormProgram;
    GLuint mUNormProgram;
    GLuint mHalfProgram;
    GLuint mOffscreenFramebuffer;
    GLuint mOffscreenTexture2D;
};

// Test the correctness of packSnorm2x16 and unpackSnorm2x16 functions calculating normal floating
// numbers.
TEST_P(PackUnpackTest, PackUnpackSnormNormal)
{
    // Expect the shader to output the same value as the input
    compareBeforeAfter(mSNormProgram, 0.5f, -0.2f);
    compareBeforeAfter(mSNormProgram, -0.35f, 0.75f);
    compareBeforeAfter(mSNormProgram, 0.00392f, -0.99215f);
    compareBeforeAfter(mSNormProgram, 1.0f, -0.00392f);
}

// Test the correctness of packSnorm2x16 and unpackSnorm2x16 functions calculating normal floating
// numbers.
TEST_P(PackUnpackTest, PackUnpackUnormNormal)
{
    // Expect the shader to output the same value as the input
    compareBeforeAfter(mUNormProgram, 0.5f, 0.2f, 0.5f, 0.2f);
    compareBeforeAfter(mUNormProgram, 0.35f, 0.75f, 0.35f, 0.75f);
    compareBeforeAfter(mUNormProgram, 0.00392f, 0.99215f, 0.00392f, 0.99215f);
    compareBeforeAfter(mUNormProgram, 1.0f, 0.00392f, 1.0f, 0.00392f);
}

// Test the correctness of packHalf2x16 and unpackHalf2x16 functions calculating normal floating
// numbers.
TEST_P(PackUnpackTest, PackUnpackHalfNormal)
{
    // Expect the shader to output the same value as the input
    compareBeforeAfter(mHalfProgram, 0.5f, -0.2f);
    compareBeforeAfter(mHalfProgram, -0.35f, 0.75f);
    compareBeforeAfter(mHalfProgram, 0.00392f, -0.99215f);
    compareBeforeAfter(mHalfProgram, 1.0f, -0.00392f);
}

// Test the correctness of packSnorm2x16 and unpackSnorm2x16 functions calculating subnormal
// floating numbers.
TEST_P(PackUnpackTest, PackUnpackSnormSubnormal)
{
    // Expect the shader to output the same value as the input
    compareBeforeAfter(mSNormProgram, 0.00001f, -0.00001f);
}

// Test the correctness of packUnorm2x16 and unpackUnorm2x16 functions calculating subnormal
// floating numbers.
TEST_P(PackUnpackTest, PackUnpackUnormSubnormal)
{
    // Expect the shader to output the same value as the input for positive numbers and clamp
    // to [0, 1]
    compareBeforeAfter(mUNormProgram, 0.00001f, -0.00001f, 0.00001f, 0.0f);
}

// Test the correctness of packHalf2x16 and unpackHalf2x16 functions calculating subnormal floating
// numbers.
TEST_P(PackUnpackTest, PackUnpackHalfSubnormal)
{
    // Expect the shader to output the same value as the input
    compareBeforeAfter(mHalfProgram, 0.00001f, -0.00001f);
}

// Test the correctness of packSnorm2x16 and unpackSnorm2x16 functions calculating zero floating
// numbers.
TEST_P(PackUnpackTest, PackUnpackSnormZero)
{
    // Expect the shader to output the same value as the input
    compareBeforeAfter(mSNormProgram, 0.00000f, -0.00000f);
}

// Test the correctness of packUnorm2x16 and unpackUnorm2x16 functions calculating zero floating
// numbers.
TEST_P(PackUnpackTest, PackUnpackUnormZero)
{
    compareBeforeAfter(mUNormProgram, 0.00000f, -0.00000f, 0.00000f, 0.00000f);
}

// Test the correctness of packHalf2x16 and unpackHalf2x16 functions calculating zero floating
// numbers.
TEST_P(PackUnpackTest, PackUnpackHalfZero)
{
    // Expect the shader to output the same value as the input
    compareBeforeAfter(mHalfProgram, 0.00000f, -0.00000f);
}

// Test the correctness of packUnorm2x16 and unpackUnorm2x16 functions calculating overflow floating
// numbers.
TEST_P(PackUnpackTest, PackUnpackUnormOverflow)
{
    // Expect the shader to clamp the input to [0, 1]
    compareBeforeAfter(mUNormProgram, 67000.0f, -67000.0f, 1.0f, 0.0f);
}

// Test the correctness of packSnorm2x16 and unpackSnorm2x16 functions calculating overflow floating
// numbers.
TEST_P(PackUnpackTest, PackUnpackSnormOverflow)
{
    // Expect the shader to clamp the input to [-1, 1]
    compareBeforeAfter(mSNormProgram, 67000.0f, -67000.0f, 1.0f, -1.0f);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(PackUnpackTest);
ANGLE_INSTANTIATE_TEST_ES3(PackUnpackTest);
}  // namespace
