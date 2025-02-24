//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Test state requests and compilation of tokens added by OES_shader_multisample_interpolation

#include "common/mathutil.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class SampleMultisampleInterpolationTest : public ANGLETest<>
{
  protected:
    SampleMultisampleInterpolationTest()
    {
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setExtensionsEnabled(false);
    }
};

// Test state queries
TEST_P(SampleMultisampleInterpolationTest, StateQueries)
{
    // New state queries fail without the extension
    {
        GLint bits = 0;
        glGetIntegerv(GL_FRAGMENT_INTERPOLATION_OFFSET_BITS_OES, &bits);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
        EXPECT_EQ(bits, 0);

        GLfloat minOffset = 0.0f;
        glGetFloatv(GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_OES, &minOffset);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
        EXPECT_EQ(minOffset, 0.0f);

        GLfloat maxOffset = 0.0f;
        glGetFloatv(GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_OES, &maxOffset);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
        EXPECT_EQ(maxOffset, 0.0f);

        ASSERT_GL_NO_ERROR();
    }

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_shader_multisample_interpolation"));

    // Implementation-dependent values
    {
        GLint bits = 0;
        glGetIntegerv(GL_FRAGMENT_INTERPOLATION_OFFSET_BITS_OES, &bits);
        EXPECT_GE(bits, 4);

        GLfloat minOffset = 0.0f;
        glGetFloatv(GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_OES, &minOffset);
        EXPECT_LE(minOffset, -0.5f + std::pow(2, -bits));

        GLfloat maxOffset = 0.0f;
        glGetFloatv(GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_OES, &maxOffset);
        EXPECT_GE(maxOffset, 0.5f - std::pow(2, -bits));

        ASSERT_GL_NO_ERROR();
    }
}

// Test gl_SampleMaskIn values with per-sample shading
TEST_P(SampleMultisampleInterpolationTest, SampleMaskInPerSample)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_sample_variables"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_shader_multisample_interpolation"));

    const char kVS[] = R"(#version 300 es
#extension GL_OES_shader_multisample_interpolation : require

in vec4 a_position;
sample out float interpolant;

void main()
{
    gl_Position = a_position;
    interpolant = 0.5;
})";

    const char kFS[] = R"(#version 300 es
#extension GL_OES_sample_variables : require
#extension GL_OES_shader_multisample_interpolation : require

precision highp float;
sample in float interpolant;
out vec4 color;

bool isPow2(int v)
{
    return v != 0 && (v & (v - 1)) == 0;
}

void main()
{
    float r = float(isPow2(gl_SampleMaskIn[0]));
    color = vec4(r, interpolant, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    for (GLint sampleCount : {0, 4})
    {
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, GL_RGBA8, 1, 1);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        drawQuad(program, "a_position", 0.0);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        GLubyte pixel[4];
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        ASSERT_GL_NO_ERROR();

        EXPECT_EQ(pixel[0], 255) << "Samples: " << sampleCount;
    }
}

// Test gl_SampleMaskIn values with per-sample noperspective shading
TEST_P(SampleMultisampleInterpolationTest, SampleMaskInPerSampleNoPerspective)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_sample_variables"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_shader_multisample_interpolation"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_NV_shader_noperspective_interpolation"));

    const char kVS[] = R"(#version 300 es
#extension GL_OES_shader_multisample_interpolation : require
#extension GL_NV_shader_noperspective_interpolation : require

in vec4 a_position;
noperspective sample out float interpolant;

void main()
{
    gl_Position = a_position;
    interpolant = 0.5;
})";

    const char kFS[] = R"(#version 300 es
#extension GL_OES_sample_variables : require
#extension GL_OES_shader_multisample_interpolation : require
#extension GL_NV_shader_noperspective_interpolation : require

precision highp float;
noperspective sample in float interpolant;
out vec4 color;

bool isPow2(int v)
{
    return v != 0 && (v & (v - 1)) == 0;
}

void main()
{
    float r = float(isPow2(gl_SampleMaskIn[0]));
    color = vec4(r, interpolant, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    for (GLint sampleCount : {0, 4})
    {
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCount, GL_RGBA8, 1, 1);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        drawQuad(program, "a_position", 0.0);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        GLubyte pixel[4];
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        ASSERT_GL_NO_ERROR();

        EXPECT_EQ(pixel[0], 255) << "Samples: " << sampleCount;
    }
}

// Test that a shader with interpolateAt* calls and directly used interpolants compiles
// successfully.
TEST_P(SampleMultisampleInterpolationTest, CompileInterpolateAt)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_shader_multisample_interpolation"));

    EnsureGLExtensionEnabled("GL_NV_shader_noperspective_interpolation");

    constexpr char kVS[] = R"(#version 300 es
#extension GL_OES_shader_multisample_interpolation : require
#extension GL_NV_shader_noperspective_interpolation : enable

precision highp float;

out float interpolant;
out float interpolantArray[2];

centroid out float interpolantCentroid;
centroid out float interpolantCentroidArray[2];

sample out float interpolantSample;
sample out float interpolantSampleArray[2];

smooth out float interpolantSmooth;
smooth out float interpolantSmoothArray[2];

flat out float interpolantFlat;
flat out float interpolantFlatArray[2];

#ifdef GL_NV_shader_noperspective_interpolation
noperspective out float interpolantNp;
noperspective out float interpolantNpArray[2];

noperspective centroid out float interpolantNpCentroid;
noperspective centroid out float interpolantNpCentroidArray[2];

noperspective sample out float interpolantNpSample;
noperspective sample out float interpolantNpSampleArray[2];
#endif

void main()
{
    gl_Position = vec4(0, 0, 0, 1);

    interpolant = 1.0;
    interpolantArray[1] = 2.0;

    interpolantCentroid = 3.0;
    interpolantCentroidArray[1] = 4.0;

    interpolantSample = 5.0;
    interpolantSampleArray[1] = 6.0;

    interpolantSmooth = 7.0;
    interpolantSmoothArray[1] = 8.0;

    interpolantFlat = 9.0;
    interpolantFlatArray[1] = 10.0;

#ifdef GL_NV_shader_noperspective_interpolation
    interpolantNp = 11.0;
    interpolantNpArray[1] = 12.0;

    interpolantNpCentroid = 13.0;
    interpolantNpCentroidArray[1] = 14.0;

    interpolantNpSample = 15.0;
    interpolantNpSampleArray[1] = 16.0;
#endif
})";

    constexpr char kFS[] = R"(#version 300 es
#extension GL_OES_shader_multisample_interpolation : require
#extension GL_NV_shader_noperspective_interpolation : enable

precision highp float;

in float interpolant;
in float interpolantArray[2];

centroid in float interpolantCentroid;
centroid in float interpolantCentroidArray[2];

sample in float interpolantSample;
sample in float interpolantSampleArray[2];

smooth in float interpolantSmooth;
smooth in float interpolantSmoothArray[2];

flat in float interpolantFlat;
flat in float interpolantFlatArray[2];

#ifdef GL_NV_shader_noperspective_interpolation
noperspective in float interpolantNp;
noperspective in float interpolantNpArray[2];

noperspective centroid in float interpolantNpCentroid;
noperspective centroid in float interpolantNpCentroidArray[2];

noperspective sample in float interpolantNpSample;
noperspective sample in float interpolantNpSampleArray[2];
#endif

out vec4 color;

void main()
{
    float r;

    r += interpolateAtCentroid(interpolant);
    r += interpolateAtSample(interpolant, int(interpolant));
    r += interpolateAtOffset(interpolant, vec2(interpolant));

    r += interpolateAtCentroid(interpolantArray[1]);
    r += interpolateAtSample(interpolantArray[1], int(interpolantArray[0]));
    r += interpolateAtOffset(interpolantArray[1], vec2(interpolantArray[0]));

    r += interpolateAtCentroid(interpolantCentroid);
    r += interpolateAtSample(interpolantCentroid, int(interpolantCentroid));
    r += interpolateAtOffset(interpolantCentroid, vec2(interpolantCentroid));

    r += interpolateAtCentroid(interpolantCentroidArray[1]);
    r += interpolateAtSample(interpolantCentroidArray[1], int(interpolantCentroidArray[0]));
    r += interpolateAtOffset(interpolantCentroidArray[1], vec2(interpolantCentroidArray[0]));

    r += interpolateAtCentroid(interpolantSample);
    r += interpolateAtSample(interpolantSample, int(interpolantSample));
    r += interpolateAtOffset(interpolantSample, vec2(interpolantSample));

    r += interpolateAtCentroid(interpolantSampleArray[1]);
    r += interpolateAtSample(interpolantSampleArray[1], int(interpolantSampleArray[0]));
    r += interpolateAtOffset(interpolantSampleArray[1], vec2(interpolantSampleArray[0]));

    r += interpolateAtCentroid(interpolantSmooth);
    r += interpolateAtSample(interpolantSmooth, int(interpolantSmooth));
    r += interpolateAtOffset(interpolantSmooth, vec2(interpolantSmooth));

    r += interpolateAtCentroid(interpolantSmoothArray[1]);
    r += interpolateAtSample(interpolantSmoothArray[1], int(interpolantSmoothArray[0]));
    r += interpolateAtOffset(interpolantSmoothArray[1], vec2(interpolantSmoothArray[0]));

    r += interpolateAtCentroid(interpolantFlat);
    r += interpolateAtSample(interpolantFlat, int(interpolantFlat));
    r += interpolateAtOffset(interpolantFlat, vec2(interpolantFlat));

    r += interpolateAtCentroid(interpolantFlatArray[1]);
    r += interpolateAtSample(interpolantFlatArray[1], int(interpolantFlatArray[0]));
    r += interpolateAtOffset(interpolantFlatArray[1], vec2(interpolantFlatArray[0]));

#ifdef GL_NV_shader_noperspective_interpolation
    r += interpolateAtCentroid(interpolantNp);
    r += interpolateAtSample(interpolantNp, int(interpolantNp));
    r += interpolateAtOffset(interpolantNp, vec2(interpolantNp));

    r += interpolateAtCentroid(interpolantNpArray[1]);
    r += interpolateAtSample(interpolantNpArray[1], int(interpolantNpArray[0]));
    r += interpolateAtOffset(interpolantNpArray[1], vec2(interpolantNpArray[0]));

    r += interpolateAtCentroid(interpolantNpCentroid);
    r += interpolateAtSample(interpolantNpCentroid, int(interpolantNpCentroid));
    r += interpolateAtOffset(interpolantNpCentroid, vec2(interpolantNpCentroid));

    r += interpolateAtCentroid(interpolantNpCentroidArray[1]);
    r += interpolateAtSample(interpolantNpCentroidArray[1], int(interpolantNpCentroidArray[0]));
    r += interpolateAtOffset(interpolantNpCentroidArray[1], vec2(interpolantNpCentroidArray[0]));

    r += interpolateAtCentroid(interpolantNpSample);
    r += interpolateAtSample(interpolantNpSample, int(interpolantNpSample));
    r += interpolateAtOffset(interpolantNpSample, vec2(interpolantNpSample));

    r += interpolateAtCentroid(interpolantNpSampleArray[1]);
    r += interpolateAtSample(interpolantNpSampleArray[1], int(interpolantNpSampleArray[0]));
    r += interpolateAtOffset(interpolantNpSampleArray[1], vec2(interpolantNpSampleArray[0]));
#endif

    color = vec4(r);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SampleMultisampleInterpolationTest);
ANGLE_INSTANTIATE_TEST_ES3(SampleMultisampleInterpolationTest);

}  // anonymous namespace
