//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferMultisampleTest: Tests of multisampled renderbuffer

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class RenderbufferMultisampleTest : public ANGLETest<>
{
  protected:
    RenderbufferMultisampleTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setExtensionsEnabled(false);
    }

    void testSetUp() override
    {
        glGenRenderbuffers(1, &mRenderbuffer);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteRenderbuffers(1, &mRenderbuffer);
        mRenderbuffer = 0;
    }

    GLuint mRenderbuffer = 0;
};

class RenderbufferMultisampleTestES31 : public RenderbufferMultisampleTest
{};

// In GLES 3.0, if internalformat is integer (signed or unsigned), to allocate multisample
// renderbuffer storage for that internalformat is not supported. An INVALID_OPERATION is
// generated. In GLES 3.1, it is OK to allocate multisample renderbuffer storage for interger
// internalformat, but the max samples should be less than MAX_INTEGER_SAMPLES.
// MAX_INTEGER_SAMPLES should be at least 1.
TEST_P(RenderbufferMultisampleTest, IntegerInternalformat)
{
    // Fixed in recent mesa.  http://crbug.com/1071142
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsLinux() && (IsIntel() || IsAMD()));

    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 1, GL_RGBA8I, 64, 64);
    if (getClientMajorVersion() < 3 || getClientMinorVersion() < 1)
    {
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);

        // Check that NUM_SAMPLE_COUNTS is zero
        GLint numSampleCounts = 0;
        glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8I, GL_NUM_SAMPLE_COUNTS, 1,
                              &numSampleCounts);
        ASSERT_GL_NO_ERROR();
        EXPECT_EQ(numSampleCounts, 0);
    }
    else
    {
        ASSERT_GL_NO_ERROR();

        GLint maxSamplesRGBA8I = 0;
        glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8I, GL_SAMPLES, 1, &maxSamplesRGBA8I);
        GLint maxIntegerSamples = 0;
        glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &maxIntegerSamples);
        ASSERT_GL_NO_ERROR();
        EXPECT_GE(maxIntegerSamples, 1);

        glRenderbufferStorageMultisample(GL_RENDERBUFFER, maxSamplesRGBA8I + 1, GL_RGBA8I, 64, 64);
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, maxIntegerSamples + 1, GL_RGBA8I, 64, 64);
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    }
}

// Ensure that the following spec language is correctly implemented:
//
//   the resulting value for RENDERBUFFER_SAMPLES is guaranteed to be greater than or equal to
//   samples and no more than the next larger sample count supported by the implementation.
//
// For example, if 2, 4, and 8 samples are supported, if 5 samples are requested, ANGLE will
// use 8 samples, and return 8 when GL_RENDERBUFFER_SAMPLES is queried.
TEST_P(RenderbufferMultisampleTest, OddSampleCount)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
    ASSERT_GL_NO_ERROR();

    // Lookup the supported number of sample counts
    GLint numSampleCounts = 0;
    std::vector<GLint> sampleCounts;
    GLsizei queryBufferSize = 1;
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_NUM_SAMPLE_COUNTS, queryBufferSize,
                          &numSampleCounts);
    ANGLE_SKIP_TEST_IF((numSampleCounts < 2));
    sampleCounts.resize(numSampleCounts);
    queryBufferSize = numSampleCounts;
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_SAMPLES, queryBufferSize,
                          sampleCounts.data());

    // Look for two sample counts that are not 1 apart (e.g. 2 and 4).  Request a sample count
    // that's between those two samples counts (e.g. 3) and ensure that GL_RENDERBUFFER_SAMPLES
    // is the higher number.
    for (int i = 1; i < numSampleCounts; i++)
    {
        if (sampleCounts[i - 1] > (sampleCounts[i] + 1))
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCounts[i] + 1, GL_RGBA8, 64,
                                             64);
            ASSERT_GL_NO_ERROR();
            GLint renderbufferSamples = 0;
            glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES,
                                         &renderbufferSamples);
            ASSERT_GL_NO_ERROR();
            EXPECT_EQ(renderbufferSamples, sampleCounts[i - 1]);
            break;
        }
    }
}

// Test that when glBlend GL_COLORBURN_KHR is emulated with framebuffer fetch, and the framebuffer
// is multisampled, the emulation works fine. This simulates the deqp test:
// dEQP-GLES31.functional.blend_equation_advanced.msaa.colorburn.
TEST_P(RenderbufferMultisampleTestES31, ColorBurnBlend)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_KHR_blend_equation_advanced"));
    // Create shader programs
    std::stringstream vs;
    vs << "#version 310 es\n"
          "in highp vec4 a_position;\n"
          "in mediump vec4 a_color;\n"
          "out mediump vec4 v_color;\n"
          "void main()\n"
          "{\n"
          "gl_Position = a_position;\n"
          "v_color = a_color;\n"
          "}\n";

    std::stringstream fs;

    fs << "#version 310 es\n"
          "#extension GL_KHR_blend_equation_advanced : require\n"
          "in mediump vec4 v_color;\n"
          "layout (blend_support_colorburn) out;\n"
          "layout (location = 0) out mediump vec4 o_color;\n"
          "void main()\n"
          "{\n"
          "o_color = v_color;\n"
          "}\n";

    GLuint program;

    program = CompileProgram(vs.str().c_str(), fs.str().c_str());

    // Create vertex data and buffers
    // Create vertex position data
    std::array<Vector2, 4> quadPos = {
        Vector2(-1.0f, -1.0f),
        Vector2(-1.0f, 1.0f),
        Vector2(1.0f, -1.0f),
        Vector2(1.0f, 1.0f),
    };

    // Create vertex color data
    std::array<Vector4, 4> quadColor = {
        Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f),
        Vector4(1.0f, 0.0f, 0.0f, 1.0f), Vector4(1.0f, 0.0f, 0.0f, 1.0f)};

    std::array<uint16_t, 6> quadIndices = {0, 2, 1, 1, 2, 3};

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices.data(), GL_STATIC_DRAW);

    GLBuffer posBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadPos), quadPos.data(), GL_STATIC_DRAW);
    const int posLoc = glGetAttribLocation(program, "a_position");
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GLBuffer colorBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadColor), quadColor.data(), GL_STATIC_DRAW);
    const int colorLoc = glGetAttribLocation(program, "a_color");
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // Create a multisampled render buffer and attach it to frame buffer color attachment
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, getWindowWidth(),
                                     getWindowHeight());
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    // Create a singlesampled render buffer and attach it to frame buffer color attachment
    GLuint resolvedRbo;
    glGenRenderbuffers(1, &resolvedRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, resolvedRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, getWindowWidth(), getWindowHeight());
    GLuint resolvedFbo = 0;
    glGenFramebuffers(1, &resolvedFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, resolvedFbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, resolvedRbo);

    glUseProgram(program);
    glBlendEquation(GL_COLORBURN_KHR);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_BLEND);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);  // Validation error
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolvedFbo);
    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                      getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_NO_ERROR();  // Validation error

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolvedFbo);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4,
                          GLColor::red);  // Validation error
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteProgram(program);
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &resolvedFbo);
    glDeleteRenderbuffers(1, &resolvedRbo);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(RenderbufferMultisampleTest);
ANGLE_INSTANTIATE_TEST_ES3_AND_ES31(RenderbufferMultisampleTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(RenderbufferMultisampleTestES31);
ANGLE_INSTANTIATE_TEST_ES31(RenderbufferMultisampleTestES31);
}  // namespace
