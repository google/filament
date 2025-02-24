//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureMultisampleTest: Tests of multisampled texture

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{
// Sample positions of d3d standard pattern. Some of the sample positions might not the same as
// opengl.
using SamplePositionsArray                                            = std::array<float, 32>;
static constexpr std::array<SamplePositionsArray, 5> kSamplePositions = {
    {{{0.5f, 0.5f}},
     {{0.75f, 0.75f, 0.25f, 0.25f}},
     {{0.375f, 0.125f, 0.875f, 0.375f, 0.125f, 0.625f, 0.625f, 0.875f}},
     {{0.5625f, 0.3125f, 0.4375f, 0.6875f, 0.8125f, 0.5625f, 0.3125f, 0.1875f, 0.1875f, 0.8125f,
       0.0625f, 0.4375f, 0.6875f, 0.9375f, 0.9375f, 0.0625f}},
     {{0.5625f, 0.5625f, 0.4375f, 0.3125f, 0.3125f, 0.625f,  0.75f,   0.4375f,
       0.1875f, 0.375f,  0.625f,  0.8125f, 0.8125f, 0.6875f, 0.6875f, 0.1875f,
       0.375f,  0.875f,  0.5f,    0.0625f, 0.25f,   0.125f,  0.125f,  0.75f,
       0.0f,    0.5f,    0.9375f, 0.25f,   0.875f,  0.9375f, 0.0625f, 0.0f}}}};

class TextureMultisampleTest : public ANGLETest<>
{
  protected:
    TextureMultisampleTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        glGenFramebuffers(1, &mFramebuffer);
        glGenTextures(1, &mTexture);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteFramebuffers(1, &mFramebuffer);
        mFramebuffer = 0;
        glDeleteTextures(1, &mTexture);
        mTexture = 0;
    }

    void texStorageMultisample(GLenum target,
                               GLint samples,
                               GLenum format,
                               GLsizei width,
                               GLsizei height,
                               GLboolean fixedsamplelocations);

    void getMultisamplefv(GLenum pname, GLuint index, GLfloat *val);
    void sampleMaski(GLuint maskNumber, GLbitfield mask);

    GLuint mFramebuffer = 0;
    GLuint mTexture     = 0;

    // Returns a sample count that can be used with the given texture target for all the given
    // formats. Assumes that if format A supports a number of samples N and another format B
    // supports a number of samples M > N then format B also supports number of samples N.
    GLint getSamplesToUse(GLenum texTarget, const std::vector<GLenum> &formats)
    {
        GLint maxSamples = 65536;
        for (GLenum format : formats)
        {
            GLint maxSamplesFormat = 0;
            glGetInternalformativ(texTarget, format, GL_SAMPLES, 1, &maxSamplesFormat);
            maxSamples = std::min(maxSamples, maxSamplesFormat);
        }
        return maxSamples;
    }

    bool lessThanES31MultisampleExtNotSupported()
    {
        return getClientMajorVersion() <= 3 && getClientMinorVersion() < 1 &&
               !EnsureGLExtensionEnabled("GL_ANGLE_texture_multisample");
    }

    const char *multisampleTextureFragmentShader()
    {
        return R"(#version 300 es
#extension GL_ANGLE_texture_multisample : require
precision highp float;
precision highp int;

uniform highp sampler2DMS tex;
uniform int sampleNum;

in vec4 v_position;
out vec4 my_FragColor;

void main() {
    ivec2 texSize = textureSize(tex);
    ivec2 sampleCoords = ivec2((v_position.xy * 0.5 + 0.5) * vec2(texSize.xy - 1));
    my_FragColor = texelFetch(tex, sampleCoords, sampleNum);
}
)";
    }

    const char *blitArrayTextureLayerFragmentShader()
    {
        return R"(#version 300 es
#extension GL_OES_texture_storage_multisample_2d_array : require
precision highp float;
precision highp int;

uniform highp sampler2DMSArray tex;
uniform int layer;
uniform int sampleNum;

in vec4 v_position;
out vec4 my_FragColor;

void main() {
    ivec3 texSize = textureSize(tex);
    ivec2 sampleCoords = ivec2((v_position.xy * 0.5 + 0.5) * vec2(texSize.xy - 1));
    my_FragColor = texelFetch(tex, ivec3(sampleCoords, layer), sampleNum);
}
)";
    }

    const char *blitIntArrayTextureLayerFragmentShader()
    {
        return R"(#version 300 es
#extension GL_OES_texture_storage_multisample_2d_array : require
precision highp float;
precision highp int;

uniform highp isampler2DMSArray tex;
uniform int layer;
uniform int sampleNum;

in vec4 v_position;
out vec4 my_FragColor;

void main() {
    ivec3 texSize = textureSize(tex);
    ivec2 sampleCoords = ivec2((v_position.xy * 0.5 + 0.5) * vec2(texSize.xy - 1));
    my_FragColor = vec4(texelFetch(tex, ivec3(sampleCoords, layer), sampleNum));
}
)";
    }
};

class NegativeTextureMultisampleTest : public TextureMultisampleTest
{
  protected:
    NegativeTextureMultisampleTest() : TextureMultisampleTest() { setExtensionsEnabled(false); }
};

class TextureMultisampleArrayTest : public TextureMultisampleTest
{
  protected:
    TextureMultisampleArrayTest() : TextureMultisampleTest() { setExtensionsEnabled(false); }

    bool areMultisampleArraysAlwaysAvailable()
    {
        return getClientMajorVersion() == 3 && getClientMinorVersion() >= 2;
    }

    // Requests the GL_OES_texture_storage_multisample_2d_array extension and returns true if the
    // operation succeeds.
    bool requestArrayExtension()
    {
        return EnsureGLExtensionEnabled("GL_OES_texture_storage_multisample_2d_array");
    }

    void texStorage3DMultisample(GLenum target,
                                 GLint samples,
                                 GLenum internalformat,
                                 GLsizei width,
                                 GLsizei height,
                                 GLsizei depth,
                                 GLboolean fixedsamplelocations)
    {
        if (getClientMajorVersion() == 3 && getClientMinorVersion() >= 2)
        {
            glTexStorage3DMultisample(target, samples, internalformat, width, height, depth,
                                      fixedsamplelocations);
        }
        else
        {
            glTexStorage3DMultisampleOES(target, samples, internalformat, width, height, depth,
                                         fixedsamplelocations);
        }
    }
};

void TextureMultisampleTest::texStorageMultisample(GLenum target,
                                                   GLint samples,
                                                   GLenum internalformat,
                                                   GLsizei width,
                                                   GLsizei height,
                                                   GLboolean fixedsamplelocations)
{
    if (getClientMajorVersion() <= 3 && getClientMinorVersion() < 1 &&
        EnsureGLExtensionEnabled("GL_ANGLE_texture_multisample"))
    {
        glTexStorage2DMultisampleANGLE(target, samples, internalformat, width, height,
                                       fixedsamplelocations);
    }
    else
    {
        glTexStorage2DMultisample(target, samples, internalformat, width, height,
                                  fixedsamplelocations);
    }
}

void TextureMultisampleTest::getMultisamplefv(GLenum pname, GLuint index, GLfloat *val)
{
    if (getClientMajorVersion() <= 3 && getClientMinorVersion() < 1 &&
        EnsureGLExtensionEnabled("GL_ANGLE_texture_multisample"))
    {
        glGetMultisamplefvANGLE(pname, index, val);
    }
    else
    {
        glGetMultisamplefv(pname, index, val);
    }
}

void TextureMultisampleTest::sampleMaski(GLuint maskNumber, GLbitfield mask)
{
    if (getClientMajorVersion() <= 3 && getClientMinorVersion() < 1 &&
        EnsureGLExtensionEnabled("GL_ANGLE_texture_multisample"))
    {
        glSampleMaskiANGLE(maskNumber, mask);
    }
    else
    {
        glSampleMaski(maskNumber, mask);
    }
}

// Tests that if es version < 3.1, GL_TEXTURE_2D_MULTISAMPLE is not supported in
// GetInternalformativ. Checks that the number of samples returned is valid in case of ES >= 3.1.
TEST_P(TextureMultisampleTest, MultisampleTargetGetInternalFormativBase)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());

    // This query returns supported sample counts in descending order. If only one sample count is
    // queried, it should be the maximum one.
    GLint maxSamplesR8 = 0;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_R8, GL_SAMPLES, 1, &maxSamplesR8);

    // GLES 3.1 section 19.3.1 specifies the required minimum of how many samples are supported.
    GLint maxColorTextureSamples;
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxColorTextureSamples);
    GLint maxSamples;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
    GLint maxSamplesR8Required = std::min(maxColorTextureSamples, maxSamples);

    EXPECT_GE(maxSamplesR8, maxSamplesR8Required);
    ASSERT_GL_NO_ERROR();
}

// Tests that if es version < 3.1 and multisample extension is unsupported,
// GL_TEXTURE_2D_MULTISAMPLE_ANGLE is not supported in FramebufferTexture2D.
TEST_P(TextureMultisampleTest, MultisampleTargetFramebufferTexture2D)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());
    GLint samples = 1;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, 64, 64, GL_FALSE);

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           mTexture, 0);

    ASSERT_GL_NO_ERROR();
}

// Tests basic functionality of glTexStorage2DMultisample.
TEST_P(TextureMultisampleTest, ValidateTextureStorageMultisampleParameters)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, 1, 1, GL_FALSE);
    ASSERT_GL_NO_ERROR();

    GLint params = 0;
    glGetTexParameteriv(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_IMMUTABLE_FORMAT, &params);
    EXPECT_EQ(1, params);

    texStorageMultisample(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);

    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, 0, 0, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, maxSize + 1, 1, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    GLint maxSamples = 0;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_R8, GL_SAMPLES, 1, &maxSamples);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, maxSamples + 1, GL_RGBA8, 1, 1, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_RGBA8, 1, 1, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA, 0, 0, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, 1, 1, GL_FALSE);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Tests the value of MAX_INTEGER_SAMPLES is no less than 1.
// [OpenGL ES 3.1 SPEC Table 20.40]
TEST_P(TextureMultisampleTest, MaxIntegerSamples)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());

    // Fixed in recent mesa.  http://crbug.com/1071142
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsLinux() && (IsIntel() || IsAMD()));

    GLint maxIntegerSamples;
    glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &maxIntegerSamples);
    EXPECT_GE(maxIntegerSamples, 1);
    EXPECT_NE(std::numeric_limits<GLint>::max(), maxIntegerSamples);
}

// Tests the value of MAX_COLOR_TEXTURE_SAMPLES is no less than 1.
// [OpenGL ES 3.1 SPEC Table 20.40]
TEST_P(TextureMultisampleTest, MaxColorTextureSamples)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());
    GLint maxColorTextureSamples;
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxColorTextureSamples);
    EXPECT_GE(maxColorTextureSamples, 1);
    EXPECT_NE(std::numeric_limits<GLint>::max(), maxColorTextureSamples);
}

// Tests the value of MAX_DEPTH_TEXTURE_SAMPLES is no less than 1.
// [OpenGL ES 3.1 SPEC Table 20.40]
TEST_P(TextureMultisampleTest, MaxDepthTextureSamples)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());
    GLint maxDepthTextureSamples;
    glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &maxDepthTextureSamples);
    EXPECT_GE(maxDepthTextureSamples, 1);
    EXPECT_NE(std::numeric_limits<GLint>::max(), maxDepthTextureSamples);
}

// Tests the maximum value of MAX_INTEGER_SAMPLES is supported
TEST_P(TextureMultisampleTest, MaxIntegerSamplesValid)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());

    // Fixed in recent mesa.  http://crbug.com/1071142
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsLinux() && (IsIntel() || IsAMD()));

    GLint maxIntegerSamples;
    glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &maxIntegerSamples);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);

    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, maxIntegerSamples, GL_RGBA8I, 1, 1, GL_FALSE);
    ASSERT_GL_NO_ERROR();
}

// Tests the maximum value of MAX_COLOR_TEXTURE_SAMPLES is supported
TEST_P(TextureMultisampleTest, MaxColorTextureSamplesValid)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());
    GLint maxColorTextureSamples;
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxColorTextureSamples);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, maxColorTextureSamples, GL_RGBA8, 1, 1,
                          GL_FALSE);
    ASSERT_GL_NO_ERROR();
}

// Tests the maximum value of MAX_DEPTH_TEXTURE_SAMPLES is supported
TEST_P(TextureMultisampleTest, MaxDepthTextureSamplesValid)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());
    GLint maxDepthTextureSamples;
    glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &maxDepthTextureSamples);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, maxDepthTextureSamples, GL_DEPTH_COMPONENT16,
                          1, 1, GL_FALSE);
    ASSERT_GL_NO_ERROR();
}

// Tests that multisample parameters are accepted by ES 3.1 or ES 3.0 and ANGLE_texture_multisample
TEST_P(TextureMultisampleTest, GetTexLevelParameter)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_get_tex_level_parameter"));

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, 1, 1, GL_TRUE);
    ASSERT_GL_NO_ERROR();

    GLfloat levelSamples = 0;
    glGetTexLevelParameterfvANGLE(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_TEXTURE_SAMPLES, &levelSamples);
    EXPECT_EQ(levelSamples, 4);

    GLint fixedSampleLocation = false;
    glGetTexLevelParameterivANGLE(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_TEXTURE_FIXED_SAMPLE_LOCATIONS,
                                  &fixedSampleLocation);
    EXPECT_EQ(fixedSampleLocation, 1);

    ASSERT_GL_NO_ERROR();
}

// The value of sample position should be equal to standard pattern on non-OpenGL backends.
TEST_P(TextureMultisampleTest, CheckSamplePositions)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());

    // OpenGL does not guarantee sample positions.
    ANGLE_SKIP_TEST_IF(IsOpenGL());

    GLint numSampleCounts = 0;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_RGBA8, GL_NUM_SAMPLE_COUNTS, 1,
                          &numSampleCounts);
    ASSERT_GT(numSampleCounts, 0);

    std::vector<GLint> sampleCounts(numSampleCounts);
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_RGBA8, GL_SAMPLES, numSampleCounts,
                          sampleCounts.data());

    GLfloat samplePosition[2];

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer);

    for (const GLint sampleCount : sampleCounts)
    {
        GLTexture texture;
        size_t indexKey = static_cast<size_t>(ceil(log2(sampleCount)));
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
        texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, sampleCount, GL_RGBA8, 1, 1, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                               texture, 0);
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        ASSERT_GL_NO_ERROR();

        for (int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
        {
            getMultisamplefv(GL_SAMPLE_POSITION, sampleIndex, samplePosition);
            EXPECT_EQ(samplePosition[0], kSamplePositions[indexKey][2 * sampleIndex]);
            EXPECT_EQ(samplePosition[1], kSamplePositions[indexKey][2 * sampleIndex + 1]);
        }
    }

    ASSERT_GL_NO_ERROR();
}

// Test textureSize and texelFetch when using ANGLE_texture_multisample extension
TEST_P(TextureMultisampleTest, SimpleTexelFetch)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_texture_multisample"));

    ANGLE_GL_PROGRAM(texelFetchProgram, essl3_shaders::vs::Passthrough(),
                     multisampleTextureFragmentShader());

    GLint texLocation = glGetUniformLocation(texelFetchProgram, "tex");
    ASSERT_GE(texLocation, 0);
    GLint sampleNumLocation = glGetUniformLocation(texelFetchProgram, "sampleNum");
    ASSERT_GE(sampleNumLocation, 0);

    const GLsizei kWidth  = 4;
    const GLsizei kHeight = 4;

    std::vector<GLenum> testFormats = {GL_RGBA8};
    GLint samplesToUse              = getSamplesToUse(GL_TEXTURE_2D_MULTISAMPLE_ANGLE, testFormats);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ANGLE, mTexture);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE_ANGLE, samplesToUse, GL_RGBA8, kWidth, kHeight,
                          GL_TRUE);
    ASSERT_GL_NO_ERROR();

    // Clear texture zero to green.
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    GLColor clearColor = GLColor::green;

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE_ANGLE,
                           mTexture, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);
    glClearColor(clearColor.R / 255.0f, clearColor.G / 255.0f, clearColor.B / 255.0f,
                 clearColor.A / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(texelFetchProgram);
    glViewport(0, 0, kWidth, kHeight);

    for (GLint sampleNum = 0; sampleNum < samplesToUse; ++sampleNum)
    {
        glUniform1i(sampleNumLocation, sampleNum);
        drawQuad(texelFetchProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, clearColor);
    }
}

// Test toggling sample mask
TEST_P(TextureMultisampleTest, SampleMaskToggling)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());

    GLboolean enabled = false;

    EXPECT_FALSE(glIsEnabled(GL_SAMPLE_MASK));
    EXPECT_GL_NO_ERROR();

    glEnable(GL_SAMPLE_MASK);
    EXPECT_GL_NO_ERROR();

    EXPECT_TRUE(glIsEnabled(GL_SAMPLE_MASK));
    EXPECT_GL_NO_ERROR();

    glGetBooleanv(GL_SAMPLE_MASK, &enabled);
    EXPECT_GL_NO_ERROR();
    EXPECT_TRUE(enabled);

    glDisable(GL_SAMPLE_MASK);
    EXPECT_GL_NO_ERROR();

    EXPECT_FALSE(glIsEnabled(GL_SAMPLE_MASK));
    EXPECT_GL_NO_ERROR();

    glGetBooleanv(GL_SAMPLE_MASK, &enabled);
    EXPECT_GL_NO_ERROR();
    EXPECT_FALSE(enabled);
}

// Test setting and querying sample mask value
TEST_P(TextureMultisampleTest, SampleMaski)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());

    GLint maxSampleMaskWords = 0;
    glGetIntegerv(GL_MAX_SAMPLE_MASK_WORDS, &maxSampleMaskWords);
    sampleMaski(maxSampleMaskWords - 1, 0x1);
    ASSERT_GL_NO_ERROR();

    sampleMaski(maxSampleMaskWords, 0x1);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    GLint sampleMaskValue = 0;
    glGetIntegeri_v(GL_SAMPLE_MASK_VALUE, 0, &sampleMaskValue);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(sampleMaskValue, 0x1);
}

// Test MS rendering with known per-sample values and a global sample mask
TEST_P(TextureMultisampleTest, MaskedDrawWithSampleID)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_texture_multisample"));
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_sample_variables"));

    ANGLE_GL_PROGRAM(fetchProgram, essl3_shaders::vs::Passthrough(),
                     multisampleTextureFragmentShader());
    glUseProgram(fetchProgram);
    const GLint sampleLocation = glGetUniformLocation(fetchProgram, "sampleNum");
    ASSERT_GE(sampleLocation, 0);

    const char kFSDraw[] = R"(#version 300 es
#extension GL_OES_sample_variables : require
precision mediump float;
out vec4 color;

void main() {
    switch (gl_SampleID) {
        case 0: color = vec4(1.0, 0.0, 0.0, 1.0); break;
        case 1: color = vec4(0.0, 1.0, 0.0, 1.0); break;
        case 2: color = vec4(0.0, 0.0, 1.0, 1.0); break;
        case 3: color = vec4(1.0, 1.0, 1.0, 1.0); break;
        default: color = vec4(0.0); break;
    }
})";
    ANGLE_GL_PROGRAM(drawProgram, essl3_shaders::vs::Simple(), kFSDraw);

    const GLsizei kSize    = 64;
    const GLsizei kSamples = 4;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ANGLE, mTexture);
    glTexStorage2DMultisampleANGLE(GL_TEXTURE_2D_MULTISAMPLE_ANGLE, kSamples, GL_RGBA8, kSize,
                                   kSize, GL_TRUE);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D_MULTISAMPLE_ANGLE, mTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    glEnable(GL_SAMPLE_MASK);
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glViewport(0, 0, kSize, kSize);
    ASSERT_GL_NO_ERROR();

    for (size_t mask = 0; mask < 16; ++mask)
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer);

        // Clear the MS texture to magenta with zero sample mask, it must not affect clear ops
        glSampleMaskiANGLE(0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();

        // Draw to the MS texture with a sample mask
        glSampleMaskiANGLE(0, mask);
        drawQuad(drawProgram, essl3_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        // Check all four samples
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glUniform1i(sampleLocation, 0);
        drawQuad(fetchProgram, essl3_shaders::PositionAttrib(), 0.0f);
        EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, (mask & 1) ? GLColor::red : GLColor::magenta)
            << "mask: " << mask;
        glUniform1i(sampleLocation, 1);
        drawQuad(fetchProgram, essl3_shaders::PositionAttrib(), 0.0f);
        EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, (mask & 2) ? GLColor::green : GLColor::magenta)
            << "mask: " << mask;
        glUniform1i(sampleLocation, 2);
        drawQuad(fetchProgram, essl3_shaders::PositionAttrib(), 0.0f);
        EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, (mask & 4) ? GLColor::blue : GLColor::magenta)
            << "mask: " << mask;
        glUniform1i(sampleLocation, 3);
        drawQuad(fetchProgram, essl3_shaders::PositionAttrib(), 0.0f);
        EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, (mask & 8) ? GLColor::white : GLColor::magenta)
            << "mask: " << mask;
    }
}

TEST_P(TextureMultisampleTest, ResolveToDefaultFramebuffer)
{
    ANGLE_SKIP_TEST_IF(lessThanES31MultisampleExtNotSupported());

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    texStorageMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, getWindowWidth(),
                          getWindowHeight(), GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           mTexture, 0);
    ASSERT_GL_NO_ERROR();

    // Clear the framebuffer
    glClearColor(0.25, 0.5, 0.75, 0.25);
    glClear(GL_COLOR_BUFFER_BIT);

    // Resolve into default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                      getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    const GLColor kResult = GLColor(63, 127, 191, 63);
    const int w           = getWindowWidth() - 1;
    const int h           = getWindowHeight() - 1;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, kResult, 1);
    EXPECT_PIXEL_COLOR_NEAR(w, 0, kResult, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, h, kResult, 1);
    EXPECT_PIXEL_COLOR_NEAR(w, h, kResult, 1);
    EXPECT_PIXEL_COLOR_NEAR(w / 2, h / 2, kResult, 1);
}

// Negative tests of multisample texture. When context less than ES 3.1 and
// ANGLE_texture_multisample not enabled, the feature isn't supported.
TEST_P(NegativeTextureMultisampleTest, Negative)
{
    // The extension must have been disabled in test init.
    ASSERT_FALSE(IsGLExtensionEnabled("GL_ANGLE_texture_multisample"));

    glEnable(GL_SAMPLE_MASK);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    EXPECT_FALSE(glIsEnabled(GL_SAMPLE_MASK));
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLboolean enabled = false;
    glGetBooleanv(GL_SAMPLE_MASK, &enabled);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    EXPECT_FALSE(enabled);

    glDisable(GL_SAMPLE_MASK);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLint maxSamples = 0;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_R8, GL_SAMPLES, 1, &maxSamples);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLint maxColorTextureSamples;
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxColorTextureSamples);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    GLint maxDepthTextureSamples;
    glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &maxDepthTextureSamples);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mTexture);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glTexStorage2DMultisampleANGLE(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, 64, 64, GL_FALSE);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, 64, 64, GL_FALSE);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           mTexture, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLint params = 0;
    glGetTexParameteriv(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_IMMUTABLE_FORMAT, &params);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (EnsureGLExtensionEnabled("GL_ANGLE_get_tex_level_parameter"))
    {
        GLfloat levelSamples = 0;
        glGetTexLevelParameterfvANGLE(GL_TEXTURE_2D_MULTISAMPLE, 0, GL_TEXTURE_SAMPLES,
                                      &levelSamples);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        GLint fixedSampleLocation = false;
        glGetTexLevelParameterivANGLE(GL_TEXTURE_2D_MULTISAMPLE, 0,
                                      GL_TEXTURE_FIXED_SAMPLE_LOCATIONS, &fixedSampleLocation);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    GLfloat samplePosition[2];
    glGetMultisamplefvANGLE(GL_SAMPLE_POSITION, 0, samplePosition);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glGetMultisamplefv(GL_SAMPLE_POSITION, 0, samplePosition);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLint maxSampleMaskWords = 0;
    glGetIntegerv(GL_MAX_SAMPLE_MASK_WORDS, &maxSampleMaskWords);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glSampleMaskiANGLE(maxSampleMaskWords - 1, 0x1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glSampleMaski(maxSampleMaskWords - 1, 0x1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLint sampleMaskValue = 0;
    glGetIntegeri_v(GL_SAMPLE_MASK_VALUE, 0, &sampleMaskValue);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Tests GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY query when the extension is not enabled.
TEST_P(TextureMultisampleArrayTest, MultisampleArrayTargetGetIntegerWithoutExtension)
{
    ASSERT(!IsGLExtensionEnabled("GL_OES_texture_storage_multisample_2d_array"));

    GLint binding = -1;
    glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY, &binding);
    if (areMultisampleArraysAlwaysAvailable())
    {
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(binding, 0);
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
        EXPECT_EQ(binding, -1);
    }
}

// Tests GL_TEXTURE_2D_MULTISAMPLE_ARRAY target with GetInternalformativ when the
// extension is not enabled.
TEST_P(TextureMultisampleArrayTest, MultisampleArrayTargetGetInternalFormativWithoutExtension)
{
    ASSERT(!IsGLExtensionEnabled("GL_OES_texture_storage_multisample_2d_array"));

    GLint maxSamples = -1;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_RGBA8, GL_SAMPLES, 1, &maxSamples);
    if (areMultisampleArraysAlwaysAvailable())
    {
        EXPECT_GL_NO_ERROR();
        EXPECT_GT(maxSamples, 0);
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
        EXPECT_EQ(maxSamples, -1);
    }
}

// Attempt to bind a texture to multisample array binding point when extension is not enabled.
TEST_P(TextureMultisampleArrayTest, BindMultisampleArrayTextureWithoutExtension)
{
    ASSERT(!IsGLExtensionEnabled("GL_OES_texture_storage_multisample_2d_array"));

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, mTexture);
    if (areMultisampleArraysAlwaysAvailable())
    {
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Try to compile shaders using GL_OES_texture_storage_multisample_2d_array when the extension is
// not enabled.
TEST_P(TextureMultisampleArrayTest, ShaderWithoutExtension)
{
    ASSERT(!IsGLExtensionEnabled("GL_OES_texture_storage_multisample_2d_array"));

    constexpr char kFSRequiresExtension[] = R"(#version 300 es
#extension GL_OES_texture_storage_multisample_2d_array : require
out highp vec4 my_FragColor;

void main() {
        my_FragColor = vec4(0.0);
})";

    GLuint program = CompileProgram(essl3_shaders::vs::Simple(), kFSRequiresExtension);
    EXPECT_EQ(0u, program);

    constexpr char kFSEnableAndUseExtension[] = R"(#version 300 es
#extension GL_OES_texture_storage_multisample_2d_array : enable

uniform highp sampler2DMSArray tex;
out highp ivec4 outSize;

void main() {
        outSize = ivec4(textureSize(tex), 0);
})";

    program = CompileProgram(essl3_shaders::vs::Simple(), kFSEnableAndUseExtension);
    EXPECT_EQ(0u, program);
}

// Tests that GL_TEXTURE_2D_MULTISAMPLE_ARRAY is supported in GetInternalformativ.
TEST_P(TextureMultisampleArrayTest, MultisampleArrayTargetGetInternalFormativ)
{
    if (!areMultisampleArraysAlwaysAvailable())
    {
        ANGLE_SKIP_TEST_IF(!requestArrayExtension());
    }

    // This query returns supported sample counts in descending order. If only one sample count is
    // queried, it should be the maximum one.
    GLint maxSamplesRGBA8 = 0;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_RGBA8, GL_SAMPLES, 1,
                          &maxSamplesRGBA8);
    GLint maxSamplesDepth = 0;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_DEPTH_COMPONENT24, GL_SAMPLES, 1,
                          &maxSamplesDepth);
    ASSERT_GL_NO_ERROR();

    // GLES 3.1 section 19.3.1 specifies the required minimum of how many samples are supported.
    GLint maxColorTextureSamples;
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxColorTextureSamples);
    GLint maxDepthTextureSamples;
    glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &maxDepthTextureSamples);
    GLint maxSamples;
    glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);

    GLint maxSamplesRGBA8Required = std::min(maxColorTextureSamples, maxSamples);
    EXPECT_GE(maxSamplesRGBA8, maxSamplesRGBA8Required);

    GLint maxSamplesDepthRequired = std::min(maxDepthTextureSamples, maxSamples);
    EXPECT_GE(maxSamplesDepth, maxSamplesDepthRequired);
}

// Tests that TexImage3D call cannot be used for GL_TEXTURE_2D_MULTISAMPLE_ARRAY.
TEST_P(TextureMultisampleArrayTest, MultiSampleArrayTexImage)
{
    if (!areMultisampleArraysAlwaysAvailable())
    {
        ANGLE_SKIP_TEST_IF(!requestArrayExtension());
    }

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, mTexture);
    ASSERT_GL_NO_ERROR();

    glTexImage3D(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, 0, GL_RGBA8, 1, 1, 1, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Tests passing invalid parameters to TexStorage3DMultisample.
TEST_P(TextureMultisampleArrayTest, InvalidTexStorage3DMultisample)
{
    if (!areMultisampleArraysAlwaysAvailable())
    {
        ANGLE_SKIP_TEST_IF(!requestArrayExtension());
    }

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, mTexture);
    ASSERT_GL_NO_ERROR();

    // Invalid target
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 2, GL_RGBA8, 1, 1, 1, GL_TRUE);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Samples 0
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, 0, GL_RGBA8, 1, 1, 1, GL_TRUE);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Unsized internalformat
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, 2, GL_RGBA, 1, 1, 1, GL_TRUE);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Width 0
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, 2, GL_RGBA8, 0, 1, 1, GL_TRUE);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Height 0
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, 2, GL_RGBA8, 1, 0, 1, GL_TRUE);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Depth 0
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, 2, GL_RGBA8, 1, 1, 0, GL_TRUE);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Tests passing invalid parameters to TexParameteri.
TEST_P(TextureMultisampleArrayTest, InvalidTexParameteri)
{
    if (!areMultisampleArraysAlwaysAvailable())
    {
        ANGLE_SKIP_TEST_IF(!requestArrayExtension());
    }

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, mTexture);
    ASSERT_GL_NO_ERROR();

    // None of the sampler parameters can be set on GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES.
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_TEXTURE_MIN_LOD, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_TEXTURE_MAX_LOD, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_TEXTURE_COMPARE_FUNC, GL_ALWAYS);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Only valid base level on GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES is 0.
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_TEXTURE_BASE_LEVEL, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test a valid TexStorage3DMultisample call and check that the queried texture level parameters
// match. Does not do any drawing.
TEST_P(TextureMultisampleArrayTest, TexStorage3DMultisample)
{
    ASSERT_TRUE(EnsureGLExtensionEnabled("GL_ANGLE_get_tex_level_parameter"));
    if (!areMultisampleArraysAlwaysAvailable())
    {
        ANGLE_SKIP_TEST_IF(!requestArrayExtension());
    }

    GLint maxSamplesRGBA8 = 0;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_RGBA8, GL_SAMPLES, 1,
                          &maxSamplesRGBA8);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, mTexture);
    ASSERT_GL_NO_ERROR();

    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, maxSamplesRGBA8, GL_RGBA8, 8, 4, 2,
                            GL_TRUE);
    ASSERT_GL_NO_ERROR();

    GLint width = 0, height = 0, depth = 0, samples = 0;
    glGetTexLevelParameterivANGLE(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameterivANGLE(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, 0, GL_TEXTURE_HEIGHT,
                                  &height);
    glGetTexLevelParameterivANGLE(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, 0, GL_TEXTURE_DEPTH, &depth);
    glGetTexLevelParameterivANGLE(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, 0, GL_TEXTURE_SAMPLES,
                                  &samples);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(8, width);
    EXPECT_EQ(4, height);
    EXPECT_EQ(2, depth);
    EXPECT_EQ(maxSamplesRGBA8, samples);
}

// Test for invalid FramebufferTextureLayer calls with GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES
// textures.
TEST_P(TextureMultisampleArrayTest, InvalidFramebufferTextureLayer)
{
    if (!areMultisampleArraysAlwaysAvailable())
    {
        ANGLE_SKIP_TEST_IF(!requestArrayExtension());
    }

    GLint maxSamplesRGBA8 = 0;
    glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, GL_RGBA8, GL_SAMPLES, 1,
                          &maxSamplesRGBA8);

    GLint maxArrayTextureLayers;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxArrayTextureLayers);

    // Test framebuffer status with just a color texture attached.
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, mTexture);
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, maxSamplesRGBA8, GL_RGBA8, 4, 4, 2,
                            GL_TRUE);
    ASSERT_GL_NO_ERROR();

    // Test with mip level 1 and -1 (only level 0 is valid for multisample textures).
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture, 1, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture, -1, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Test with layer -1 and layer == MAX_ARRAY_TEXTURE_LAYERS
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture, 0, -1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture, 0,
                              maxArrayTextureLayers);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Attach layers of TEXTURE_2D_MULTISAMPLE_ARRAY textures to a framebuffer and check for
// completeness.
TEST_P(TextureMultisampleArrayTest, FramebufferCompleteness)
{
    if (!areMultisampleArraysAlwaysAvailable())
    {
        ANGLE_SKIP_TEST_IF(!requestArrayExtension());
    }

    std::vector<GLenum> testFormats = {{GL_RGBA8, GL_DEPTH_COMPONENT24, GL_DEPTH24_STENCIL8}};
    GLint samplesToUse = getSamplesToUse(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, testFormats);

    // Test framebuffer status with just a color texture attached.
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, mTexture);
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, samplesToUse, GL_RGBA8, 4, 4, 2,
                            GL_TRUE);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture, 0, 0);
    ASSERT_GL_NO_ERROR();

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);

    // Test framebuffer status with both color and depth textures attached.
    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, depthTexture);
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, samplesToUse, GL_DEPTH_COMPONENT24,
                            4, 4, 2, GL_TRUE);
    ASSERT_GL_NO_ERROR();

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0, 0);
    ASSERT_GL_NO_ERROR();

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);

    // Test with color and depth/stencil textures attached.
    GLTexture depthStencilTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, depthStencilTexture);
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, samplesToUse, GL_DEPTH24_STENCIL8,
                            4, 4, 2, GL_TRUE);
    ASSERT_GL_NO_ERROR();

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthStencilTexture, 0,
                              0);
    ASSERT_GL_NO_ERROR();

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);
}

// Attach a layer of TEXTURE_2D_MULTISAMPLE_ARRAY texture to a framebuffer, clear it, and resolve by
// blitting.
TEST_P(TextureMultisampleArrayTest, FramebufferColorClearAndBlit)
{
    if (!areMultisampleArraysAlwaysAvailable())
    {
        ANGLE_SKIP_TEST_IF(!requestArrayExtension());
    }

    const GLsizei kWidth  = 4;
    const GLsizei kHeight = 4;

    std::vector<GLenum> testFormats = {GL_RGBA8};
    GLint samplesToUse = getSamplesToUse(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, testFormats);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, mTexture);
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, samplesToUse, GL_RGBA8, kWidth,
                            kHeight, 2, GL_TRUE);

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture, 0, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLFramebuffer resolveFramebuffer;
    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFramebuffer);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture,
                           0);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFramebuffer);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::green);
}

// Check the size of a multisample array texture in a shader.
TEST_P(TextureMultisampleArrayTest, TextureSizeInShader)
{
    ANGLE_SKIP_TEST_IF(!requestArrayExtension());

    constexpr char kFS[] = R"(#version 300 es
#extension GL_OES_texture_storage_multisample_2d_array : require

uniform highp sampler2DMSArray tex;
out highp vec4 my_FragColor;

void main() {
        my_FragColor = (textureSize(tex) == ivec3(8, 4, 2)) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(texSizeProgram, essl3_shaders::vs::Simple(), kFS);

    GLint texLocation = glGetUniformLocation(texSizeProgram, "tex");
    ASSERT_GE(texLocation, 0);

    const GLsizei kWidth  = 8;
    const GLsizei kHeight = 4;

    std::vector<GLenum> testFormats = {GL_RGBA8};
    GLint samplesToUse = getSamplesToUse(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, testFormats);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, mTexture);
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, samplesToUse, GL_RGBA8, kWidth,
                            kHeight, 2, GL_TRUE);
    ASSERT_GL_NO_ERROR();

    drawQuad(texSizeProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Clear the layers of a multisample array texture, and then sample all the samples from all the
// layers in a shader.
TEST_P(TextureMultisampleArrayTest, SimpleTexelFetch)
{
    ANGLE_SKIP_TEST_IF(!requestArrayExtension());

    ANGLE_GL_PROGRAM(texelFetchProgram, essl3_shaders::vs::Passthrough(),
                     blitArrayTextureLayerFragmentShader());

    GLint texLocation = glGetUniformLocation(texelFetchProgram, "tex");
    ASSERT_GE(texLocation, 0);
    GLint layerLocation = glGetUniformLocation(texelFetchProgram, "layer");
    ASSERT_GE(layerLocation, 0);
    GLint sampleNumLocation = glGetUniformLocation(texelFetchProgram, "sampleNum");
    ASSERT_GE(layerLocation, 0);

    const GLsizei kWidth      = 4;
    const GLsizei kHeight     = 4;
    const GLsizei kLayerCount = 2;

    std::vector<GLenum> testFormats = {GL_RGBA8};
    GLint samplesToUse = getSamplesToUse(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, testFormats);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, mTexture);
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, samplesToUse, GL_RGBA8, kWidth,
                            kHeight, kLayerCount, GL_TRUE);
    ASSERT_GL_NO_ERROR();

    // Clear layer zero to green and layer one to blue.
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    std::vector<GLColor> clearColors = {{GLColor::green, GLColor::blue}};
    for (GLint i = 0; static_cast<GLsizei>(i) < kLayerCount; ++i)
    {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture, 0, i);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);
        const GLColor &clearColor = clearColors[i];
        glClearColor(clearColor.R / 255.0f, clearColor.G / 255.0f, clearColor.B / 255.0f,
                     clearColor.A / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(texelFetchProgram);
    glViewport(0, 0, kWidth, kHeight);
    for (GLint layer = 0; static_cast<GLsizei>(layer) < kLayerCount; ++layer)
    {
        glUniform1i(layerLocation, layer);
        for (GLint sampleNum = 0; sampleNum < samplesToUse; ++sampleNum)
        {
            glUniform1i(sampleNumLocation, sampleNum);
            drawQuad(texelFetchProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
            ASSERT_GL_NO_ERROR();
            EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, clearColors[layer]);
        }
    }
}

// Clear the layers of an integer multisample array texture, and then sample all the samples from
// all the layers in a shader.
TEST_P(TextureMultisampleArrayTest, IntegerTexelFetch)
{
    ANGLE_SKIP_TEST_IF(!requestArrayExtension());

    ANGLE_GL_PROGRAM(texelFetchProgram, essl3_shaders::vs::Passthrough(),
                     blitIntArrayTextureLayerFragmentShader());

    GLint texLocation = glGetUniformLocation(texelFetchProgram, "tex");
    ASSERT_GE(texLocation, 0);
    GLint layerLocation = glGetUniformLocation(texelFetchProgram, "layer");
    ASSERT_GE(layerLocation, 0);
    GLint sampleNumLocation = glGetUniformLocation(texelFetchProgram, "sampleNum");
    ASSERT_GE(layerLocation, 0);

    const GLsizei kWidth      = 4;
    const GLsizei kHeight     = 4;
    const GLsizei kLayerCount = 2;

    std::vector<GLenum> testFormats = {GL_RGBA8I};
    GLint samplesToUse = getSamplesToUse(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, testFormats);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, mTexture);
    texStorage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, samplesToUse, GL_RGBA8I, kWidth,
                            kHeight, kLayerCount, GL_TRUE);
    ASSERT_GL_NO_ERROR();

    // Clear layer zero to green and layer one to blue.
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    std::vector<GLColor> clearColors = {{GLColor::green, GLColor::blue}};
    for (GLint i = 0; static_cast<GLsizei>(i) < kLayerCount; ++i)
    {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture, 0, i);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);
        std::array<GLint, 4> intColor;
        for (size_t j = 0; j < intColor.size(); ++j)
        {
            intColor[j] = clearColors[i][j] / 255;
        }
        glClearBufferiv(GL_COLOR, 0, intColor.data());
        ASSERT_GL_NO_ERROR();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(texelFetchProgram);
    glViewport(0, 0, kWidth, kHeight);
    for (GLint layer = 0; static_cast<GLsizei>(layer) < kLayerCount; ++layer)
    {
        glUniform1i(layerLocation, layer);
        for (GLint sampleNum = 0; sampleNum < samplesToUse; ++sampleNum)
        {
            glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glUniform1i(sampleNumLocation, sampleNum);
            drawQuad(texelFetchProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
            ASSERT_GL_NO_ERROR();
            EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, clearColors[layer]);
        }
    }
}

class TextureSampleShadingTest : public ANGLETest<>
{
  protected:
    TextureSampleShadingTest() {}
};

// Test that sample shading actually produces different interpolations per sample.  Note that
// variables such as gl_SampleID and gl_SamplePosition are avoided, as well as the |sample|
// qualifier as they automatically enable sample shading.
TEST_P(TextureSampleShadingTest, Basic)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_sample_shading"));

    constexpr GLsizei kSize        = 1;
    constexpr GLsizei kSampleCount = 4;

    // Create a multisampled texture and framebuffer.
    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, kSampleCount, GL_RGBA8, kSize, kSize,
                              false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Enable sample shading and draw a gradient.
    glEnable(GL_SAMPLE_SHADING_OES);
    glMinSampleShadingOES(1.0f);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    glViewport(0, 0, kSize, kSize);
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create a buffer for verification.
    constexpr GLsizei kPixelChannels = 4;
    constexpr GLsizei kBufferSize =
        kSize * kSize * kSampleCount * kPixelChannels * sizeof(uint32_t);
    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBufferSize, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);

    // Issue a dispatch call that copies the multisampled texture into a buffer.
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=4, local_size_y=1, local_size_z=1) in;

uniform highp sampler2DMS imageIn;
layout(std430, binding = 0) buffer dataOut {
    uint data[];
};

void main()
{
    int sampleIndex = int(gl_GlobalInvocationID.x) % 4;

    vec4 color = texelFetch(imageIn, ivec2(0), sampleIndex);
    uvec4 unnormalized = uvec4(color * 255.0);

    int outIndex = sampleIndex * 4;

    data[outIndex    ] = unnormalized.r;
    data[outIndex + 1] = unnormalized.g;
    data[outIndex + 2] = unnormalized.b;
    data[outIndex + 3] = unnormalized.a;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    // Bind the multisampled texture as sampler.
    GLint imageLocation = glGetUniformLocation(program, "imageIn");
    ASSERT_GE(imageLocation, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
    glUniform1i(imageLocation, 0);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Verify that the buffer has correct data.
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    const uint32_t *ptr = reinterpret_cast<uint32_t *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, GL_MAP_READ_BIT));
    constexpr GLColor kExpectedColors[4] = {
        GLColor(96, 32, 0, 255),
        GLColor(223, 96, 0, 255),
        GLColor(32, 159, 0, 255),
        GLColor(159, 223, 0, 255),
    };
    for (GLsizei pixel = 0; pixel < kSampleCount; ++pixel)
    {
        for (GLsizei channel = 0; channel < kPixelChannels; ++channel)
        {
            EXPECT_NEAR(ptr[pixel * kPixelChannels + channel], kExpectedColors[pixel][channel], 1)
                << pixel << " " << channel;
        }
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Test that sample shading actually produces different interpolations per sample when |sample| is
// missing from the shader.  Both varyings and I/O blocks are tested.  When |centroid| is specified,
// |sample| shouldn't be added.
TEST_P(TextureSampleShadingTest, NoSampleQualifier)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_sample_shading"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_sample_variables"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_shader_multisample_interpolation"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr GLsizei kSize        = 1;
    constexpr GLsizei kSampleCount = 4;

    // Create a multisampled texture and framebuffer.
    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture msaaTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, kSampleCount, GL_RGBA8, kSize, kSize,
                              false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           msaaTexture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Enable sample shading and draw
    glEnable(GL_SAMPLE_SHADING_OES);
    glMinSampleShadingOES(1.0f);

    constexpr char kVS[] = R"(#version 310 es
#extension GL_OES_shader_multisample_interpolation : require
#extension GL_EXT_shader_io_blocks : require

in mediump vec2 position;
out mediump vec2 gradient;
centroid out mediump vec2 constant;
out Block
{
    centroid mediump vec2 constant2;
    mediump vec2 gradient2;
    sample mediump vec2 gradient3;
};

out Inactive
{
    mediump vec2 gradient4;
};

void main()
{
    gradient = position;
    gradient2 = position;
    gradient3 = position;
    constant = position;
    constant2 = position;
    gl_Position = vec4(position, 0, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_OES_shader_multisample_interpolation : require
#extension GL_OES_sample_variables : require
#extension GL_EXT_shader_io_blocks : require

in highp vec2 gradient;
centroid in highp vec2 constant;
in Block
{
    centroid mediump vec2 constant2;
    mediump vec2 gradient2;
    sample mediump vec2 gradient3;
};

in Inactive2
{
    mediump vec2 gradient4;
};

out mediump vec4 color;

void main()
{
    bool left = gl_SampleID == 0 || gl_SampleID == 2;
    bool top = gl_SampleID == 0 || gl_SampleID == 1;

    color = vec4(0);

    if (left)
        color.r = gradient.x < -0.1 && gradient2.x < -0.1 && gradient3.x < -0.1 ? 1. : 0.;
    else
        color.r = gradient.x > 0.1 && gradient2.x > 0.1 && gradient3.x > 0.1 ? 1. : 0.;

    if (top)
        color.g = gradient.y < -0.1 && gradient2.y < -0.1 && gradient3.y < -0.1 ? 1. : 0.;
    else
        color.g = gradient.y > 0.1 && gradient2.y > 0.1 && gradient3.y > 0.1 ? 1. : 0.;

    // centroid doesn't exactly behave consistently between implementations.  In particular, it does
    // _not_ necessarily evaluage the varying at the pixel center.  As a result, there isn't much
    // that can be verified here.  We'd rely on SPIR-V validation to make sure Sample is not added
    // to ids that already have Centroid specified (in the Vulkan backend)
    color.b = abs(constant.x) < 1. && abs(constant.y) < 1. ? 1. : 0.;
    color.a = abs(constant2.x) < 1. && abs(constant2.y) < 1. ? 1. : 0.;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glViewport(0, 0, kSize, kSize);
    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    // Resolve the framebuffer
    GLFramebuffer fbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

    // Ensure the test passed on every sample location
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
    ASSERT_GL_NO_ERROR();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(TextureMultisampleTest);
ANGLE_INSTANTIATE_TEST_ES3_AND_ES31(TextureMultisampleTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(NegativeTextureMultisampleTest);
ANGLE_INSTANTIATE_TEST_ES3(NegativeTextureMultisampleTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(TextureMultisampleArrayTest);
ANGLE_INSTANTIATE_TEST_ES3_AND_ES31_AND(TextureMultisampleArrayTest, ANGLE_ALL_TEST_PLATFORMS_ES32);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(TextureSampleShadingTest);
ANGLE_INSTANTIATE_TEST_ES31(TextureSampleShadingTest);
}  // anonymous namespace
