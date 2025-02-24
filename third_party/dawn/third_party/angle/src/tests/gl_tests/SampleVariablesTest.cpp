//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Test built-in variables added by OES_sample_variables

#include <unordered_set>

#include "common/mathutil.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class SampleVariablesTest : public ANGLETest<>
{
  protected:
    SampleVariablesTest()
    {
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Test gl_MaxSamples == GL_MAX_SAMPLES
TEST_P(SampleVariablesTest, MaxSamples)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_sample_variables"));

    GLint maxSamples = -1;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    ASSERT_GT(maxSamples, 0);
    ASSERT_LE(maxSamples, 32);

    const char kFS[] = R"(#version 300 es
#extension GL_OES_sample_variables : require
precision highp float;
out vec4 color;
void main()
{
    color = vec4(float(gl_MaxSamples * 4) / 255.0, 0.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.0);

    EXPECT_PIXEL_NEAR(0, 0, maxSamples * 4, 0, 0, 255, 1.0);
}

// Test gl_NumSamples == GL_SAMPLES
TEST_P(SampleVariablesTest, NumSamples)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_sample_variables"));

    const char kFS[] = R"(#version 300 es
#extension GL_OES_sample_variables : require
precision highp float;
out vec4 color;
void main()
{
    color = vec4(float(gl_NumSamples * 4) / 255.0, 0.0, 0.0, 1.0);
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    GLint numSampleCounts = 0;
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_NUM_SAMPLE_COUNTS, 1, &numSampleCounts);
    ASSERT_GT(numSampleCounts, 0);

    std::vector<GLint> sampleCounts;
    sampleCounts.resize(numSampleCounts);
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_SAMPLES, numSampleCounts,
                          sampleCounts.data());

    sampleCounts.push_back(0);

    for (GLint sampleCount : sampleCounts)
    {
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, GL_RGBA8, 1, 1);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        GLint samples = -1;
        glGetIntegerv(GL_SAMPLES, &samples);
        EXPECT_EQ(samples, sampleCount);

        drawQuad(program, essl3_shaders::PositionAttrib(), 0.0);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        GLubyte pixel[4];
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

        EXPECT_NEAR(std::max(samples, 1) * 4, pixel[0], 1.0) << "Samples: " << sampleCount;
    }
}

// Test gl_SampleID values
TEST_P(SampleVariablesTest, SampleID)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_sample_variables"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_color_buffer_half_float"));

    const char kFS[] = R"(#version 300 es
#extension GL_OES_sample_variables : require
precision highp float;
out vec4 color;
uniform int id;
void main()
{
    // 1.0 when the selected sample is processed, 0.0 otherwise
    float r = float(gl_SampleID == id);
    // Must always be 0.0
    float g = float(gl_SampleID < 0 || gl_SampleID >= gl_NumSamples);

    color = vec4(r, g, 0.0, 1.0);
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint numSampleCounts = 0;
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA16F, GL_NUM_SAMPLE_COUNTS, 1, &numSampleCounts);
    ASSERT_GT(numSampleCounts, 0);

    std::vector<GLint> sampleCounts;
    sampleCounts.resize(numSampleCounts);
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA16F, GL_SAMPLES, numSampleCounts,
                          sampleCounts.data());

    sampleCounts.push_back(0);

    GLFramebuffer fboResolve;
    glBindFramebuffer(GL_FRAMEBUFFER, fboResolve);

    GLRenderbuffer rboResolve;
    glBindRenderbuffer(GL_RENDERBUFFER, rboResolve);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboResolve);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ASSERT_GL_NO_ERROR();

    for (GLint sampleCount : sampleCounts)
    {
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, GL_RGBA16F, 1, 1);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        for (int sample = 0; sample < std::max(sampleCount, 1); sample++)
        {
            glUniform1i(glGetUniformLocation(program, "id"), sample);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
            drawQuad(program, essl3_shaders::PositionAttrib(), 0.0);
            ASSERT_GL_NO_ERROR();

            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboResolve);
            glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

            GLfloat pixel[4];
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fboResolve);
            glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, pixel);
            EXPECT_GL_NO_ERROR();

            // Only one sample has 1.0 value
            EXPECT_NEAR(1.0 / std::max(sampleCount, 1), pixel[0], 0.001)
                << "Samples: " << sampleCount << ", SampleID: " << sample;
            EXPECT_EQ(0.0, pixel[1]);
        }
    }
}

// Test gl_SamplePosition values
TEST_P(SampleVariablesTest, SamplePosition)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_sample_variables"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_color_buffer_half_float"));

    const char kFS[] = R"(#version 300 es
#extension GL_OES_sample_variables : require
precision highp float;
out vec4 color;
uniform int id;
void main()
{
    vec2 rg = (gl_SampleID == id) ? gl_SamplePosition : vec2(0.0, 0.0);

    // Must always be 0.0
    float b = float(gl_SamplePosition.x < 0.0 || gl_SamplePosition.x > 1.0);
    float a = float(gl_SamplePosition.y < 0.0 || gl_SamplePosition.y > 1.0);

    color = vec4(rg, b, a);
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint numSampleCounts = 0;
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA16F, GL_NUM_SAMPLE_COUNTS, 1, &numSampleCounts);
    ASSERT_GT(numSampleCounts, 0);

    std::vector<GLint> sampleCounts;
    sampleCounts.resize(numSampleCounts);
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA16F, GL_SAMPLES, numSampleCounts,
                          sampleCounts.data());

    sampleCounts.push_back(0);

    GLFramebuffer fboResolve;
    glBindFramebuffer(GL_FRAMEBUFFER, fboResolve);

    GLRenderbuffer rboResolve;
    glBindRenderbuffer(GL_RENDERBUFFER, rboResolve);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboResolve);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    for (GLint sampleCount : sampleCounts)
    {
        if (sampleCount > 16)
        {
            // Sample positions for MSAAx32 are not defined.
            continue;
        }
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, GL_RGBA16F, 1, 1);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        std::unordered_set<GLfloat> positionX;
        std::unordered_set<GLfloat> positionY;
        for (int sample = 0; sample < std::max(sampleCount, 1); sample++)
        {
            glUniform1i(glGetUniformLocation(program, "id"), sample);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
            drawQuad(program, essl3_shaders::PositionAttrib(), 0.0);
            ASSERT_GL_NO_ERROR();

            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboResolve);
            glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

            GLfloat pixel[4];
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fboResolve);
            glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, pixel);
            EXPECT_GL_NO_ERROR();

            // Check that positions are unique for each sample
            const int realSampleCount = std::max(sampleCount, 1);
            EXPECT_TRUE(positionX.insert(pixel[0]).second)
                << "Samples: " << realSampleCount << " SampleID: " << sample
                << " X: " << (pixel[0] * realSampleCount);
            EXPECT_TRUE(positionY.insert(pixel[1]).second)
                << "Samples: " << realSampleCount << " SampleID: " << sample
                << " Y: " << (pixel[1] * realSampleCount);

            // Check that sample positions are in the normalized range
            EXPECT_EQ(0, pixel[2]);
            EXPECT_EQ(0, pixel[3]);
        }
    }
}

// Test gl_SampleMaskIn values
TEST_P(SampleVariablesTest, SampleMaskIn)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_sample_variables"));

    const char kFS[] = R"(#version 300 es
#extension GL_OES_sample_variables : require
precision highp float;
out vec4 color;

uint popcount(uint v)
{
    uint c = 0u;
    for (; v != 0u; v >>= 1) c += v & 1u;
    return c;
}

void main()
{
    float r = 0.0;
    r = float(popcount(uint(gl_SampleMaskIn[0])));

    color = vec4(r * 4.0 / 255.0, 0, 0, 1);
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint numSampleCounts = 0;
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_NUM_SAMPLE_COUNTS, 1, &numSampleCounts);
    ASSERT_GT(numSampleCounts, 0);

    std::vector<GLint> sampleCounts;
    sampleCounts.resize(numSampleCounts);
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_SAMPLES, numSampleCounts,
                          sampleCounts.data());

    sampleCounts.push_back(0);

    auto test = [&](GLint sampleCount, bool sampleCoverageEnabled, GLfloat coverage) {
        if (sampleCoverageEnabled)
        {
            glEnable(GL_SAMPLE_COVERAGE);
        }
        else
        {
            glDisable(GL_SAMPLE_COVERAGE);
        }

        glSampleCoverage(coverage, false);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, GL_RGBA8, 1, 1);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(program, essl3_shaders::PositionAttrib(), 0.0);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        GLubyte pixel[4];
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        EXPECT_GL_NO_ERROR();

        // Shader scales up the number of input samples to increase precision in unorm8 space.
        int expected = std::max(sampleCount, 1) * 4;

        // Sample coverage must not affect single sampled buffers
        if (sampleCoverageEnabled && sampleCount > 0)
        {
            // The number of samples in gl_SampleMaskIn must be affected by the sample
            // coverage GL state and then the resolved value must be scaled down again.
            expected *= coverage * coverage;
        }
        EXPECT_NEAR(expected, pixel[0], 1.0)
            << "Samples: " << sampleCount
            << ", Sample Coverage: " << (sampleCoverageEnabled ? "Enabled" : "Disabled")
            << ", Coverage: " << coverage;
    };

    for (GLint sampleCount : sampleCounts)
    {
        if (sampleCount > 32)
        {
            // The test shader will not work with MSAAx64.
            continue;
        }

        for (bool sampleCoverageEnabled : {false, true})
        {
            for (GLfloat coverage : {0.0, 0.5, 1.0})
            {
                if (sampleCount == 1 && coverage != 0.0 && coverage != 1.0)
                {
                    continue;
                }
                test(sampleCount, sampleCoverageEnabled, coverage);
            }
        }
    }
}

// Test gl_SampleMaskIn values with per-sample shading
TEST_P(SampleVariablesTest, SampleMaskInPerSample)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_sample_variables"));

    const char kFS[] = R"(#version 300 es
#extension GL_OES_sample_variables : require
precision highp float;
out vec4 color;

void main()
{
    float r = float(gl_SampleMaskIn[0] == (1 << gl_SampleID));
    color = vec4(r, 0, 0, 1);
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint numSampleCounts = 0;
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_NUM_SAMPLE_COUNTS, 1, &numSampleCounts);
    ASSERT_GT(numSampleCounts, 0);

    std::vector<GLint> sampleCounts;
    sampleCounts.resize(numSampleCounts);
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA8, GL_SAMPLES, numSampleCounts,
                          sampleCounts.data());

    sampleCounts.push_back(0);

    for (GLint sampleCount : sampleCounts)
    {
        if (sampleCount > 32)
        {
            // The test shader will not work with MSAAx64.
            continue;
        }

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, GL_RGBA8, 1, 1);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        drawQuad(program, essl3_shaders::PositionAttrib(), 0.0);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red) << "Samples: " << sampleCount;
    }
}

// Test writing gl_SampleMask
TEST_P(SampleVariablesTest, SampleMask)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_sample_variables"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_color_buffer_half_float"));

    const char kFS[] = R"(#version 300 es
#extension GL_OES_sample_variables : require
precision highp float;
uniform highp int sampleMask;

out vec4 color;

void main()
{
    gl_SampleMask[0] = sampleMask;
    color = vec4(1, 0, 0, 1);
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint numSampleCounts = 0;
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA16F, GL_NUM_SAMPLE_COUNTS, 1, &numSampleCounts);
    ASSERT_GT(numSampleCounts, 0);

    std::vector<GLint> sampleCounts;
    sampleCounts.resize(numSampleCounts);
    glGetInternalformativ(GL_RENDERBUFFER, GL_RGBA16F, GL_SAMPLES, numSampleCounts,
                          sampleCounts.data());

    sampleCounts.push_back(0);

    GLFramebuffer fboResolve;
    glBindFramebuffer(GL_FRAMEBUFFER, fboResolve);

    GLRenderbuffer rboResolve;
    glBindRenderbuffer(GL_RENDERBUFFER, rboResolve);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboResolve);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    auto test = [&](GLint sampleCount, GLint sampleMask, bool sampleCoverageEnabled,
                    GLfloat coverage) {
        if (sampleCoverageEnabled)
        {
            glEnable(GL_SAMPLE_COVERAGE);
        }
        else
        {
            glDisable(GL_SAMPLE_COVERAGE);
        }

        ASSERT(coverage == 0.0 || coverage == 1.0);
        glSampleCoverage(coverage, false);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, GL_RGBA16F, 1, 1);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT);
        glUniform1i(glGetUniformLocation(program, "sampleMask"), sampleMask);
        drawQuad(program, essl3_shaders::PositionAttrib(), 0.0);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboResolve);
        glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        GLfloat pixel[4];
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboResolve);
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, pixel);
        EXPECT_GL_NO_ERROR();

        float expected = 1.0;
        if (sampleCount > 0)
        {
            if (sampleCoverageEnabled && coverage == 0.0)
            {
                expected = 0.0;
            }
            else
            {
                uint32_t fullSampleCount = std::max(sampleCount, 1);
                uint32_t realSampleMask  = sampleMask & (0xFFFFFFFFu >> (32 - fullSampleCount));
                expected = static_cast<float>(gl::BitCount(realSampleMask)) / fullSampleCount;
            }
        }
        EXPECT_EQ(expected, pixel[0])
            << "Samples: " << sampleCount << ", gl_SampleMask[0]: " << sampleMask
            << ", Sample Coverage: " << (sampleCoverageEnabled ? "Enabled" : "Disabled")
            << ", Coverage: " << coverage;
    };

    for (GLint sampleCount : sampleCounts)
    {
        if (sampleCount > 32)
        {
            // The test shader will not work with MSAAx64.
            continue;
        }

        for (bool sampleCoverageEnabled : {false, true})
        {
            for (GLfloat coverage : {0.0, 1.0})
            {
                for (GLint sampleMask : {0xFFFFFFFFu, 0x55555555u, 0xAAAAAAAAu, 0x00000000u})
                {
                    test(sampleCount, sampleMask, sampleCoverageEnabled, coverage);
                }
            }
        }
    }
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SampleVariablesTest);
ANGLE_INSTANTIATE_TEST_ES3(SampleVariablesTest);

}  // anonymous namespace
