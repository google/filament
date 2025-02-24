//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BlendFuncExtendedTest
//   Test EXT_blend_func_extended

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/shader_utils.h"

#include <algorithm>
#include <cmath>
#include <fstream>

using namespace angle;

namespace
{

// Partial implementation of weight function for GLES 2 blend equation that
// is dual-source aware.
template <int factor, int index>
float Weight(const float /*dst*/[4], const float src[4], const float src1[4])
{
    if (factor == GL_SRC_COLOR)
        return src[index];
    if (factor == GL_SRC_ALPHA)
        return src[3];
    if (factor == GL_SRC1_COLOR_EXT)
        return src1[index];
    if (factor == GL_SRC1_ALPHA_EXT)
        return src1[3];
    if (factor == GL_ONE_MINUS_SRC1_COLOR_EXT)
        return 1.0f - src1[index];
    if (factor == GL_ONE_MINUS_SRC1_ALPHA_EXT)
        return 1.0f - src1[3];
    return 0.0f;
}

GLubyte ScaleChannel(float weight)
{
    return static_cast<GLubyte>(std::floor(std::max(0.0f, std::min(1.0f, weight)) * 255.0f));
}

// Implementation of GLES 2 blend equation that is dual-source aware.
template <int RGBs, int RGBd, int As, int Ad>
void BlendEquationFuncAdd(const float dst[4],
                          const float src[4],
                          const float src1[4],
                          angle::GLColor *result)
{
    float r[4];
    r[0] = src[0] * Weight<RGBs, 0>(dst, src, src1) + dst[0] * Weight<RGBd, 0>(dst, src, src1);
    r[1] = src[1] * Weight<RGBs, 1>(dst, src, src1) + dst[1] * Weight<RGBd, 1>(dst, src, src1);
    r[2] = src[2] * Weight<RGBs, 2>(dst, src, src1) + dst[2] * Weight<RGBd, 2>(dst, src, src1);
    r[3] = src[3] * Weight<As, 3>(dst, src, src1) + dst[3] * Weight<Ad, 3>(dst, src, src1);

    result->R = ScaleChannel(r[0]);
    result->G = ScaleChannel(r[1]);
    result->B = ScaleChannel(r[2]);
    result->A = ScaleChannel(r[3]);
}

void CheckPixels(GLint x,
                 GLint y,
                 GLsizei width,
                 GLsizei height,
                 GLint tolerance,
                 const angle::GLColor &color)
{
    for (GLint yy = 0; yy < height; ++yy)
    {
        for (GLint xx = 0; xx < width; ++xx)
        {
            const auto px = x + xx;
            const auto py = y + yy;
            EXPECT_PIXEL_COLOR_NEAR(px, py, color, 2);
        }
    }
}

const GLuint kWidth  = 100;
const GLuint kHeight = 100;

class EXTBlendFuncExtendedTest : public ANGLETest<>
{};

class EXTBlendFuncExtendedTestES3 : public ANGLETest<>
{};

class EXTBlendFuncExtendedDrawTest : public ANGLETest<>
{
  protected:
    EXTBlendFuncExtendedDrawTest() : mProgram(0)
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        glGenBuffers(1, &mVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);

        static const float vertices[] = {
            1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteBuffers(1, &mVBO);
        if (mProgram)
        {
            glDeleteProgram(mProgram);
        }

        ASSERT_GL_NO_ERROR();
    }

    void makeProgram(const char *vertSource, const char *fragSource)
    {
        mProgram = CompileProgram(vertSource, fragSource);

        ASSERT_NE(0u, mProgram);
    }

    virtual GLint getVertexAttribLocation(const char *name)
    {
        return glGetAttribLocation(mProgram, name);
    }

    virtual GLint getFragmentUniformLocation(const char *name)
    {
        return glGetUniformLocation(mProgram, name);
    }

    virtual void setUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
    {
        glUniform4f(location, v0, v1, v2, v3);
    }

    void drawTest()
    {
        glUseProgram(mProgram);

        GLint position = getVertexAttribLocation(essl1_shaders::PositionAttrib());
        GLint src0     = getFragmentUniformLocation("src0");
        GLint src1     = getFragmentUniformLocation("src1");
        ASSERT_GL_NO_ERROR();

        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glEnableVertexAttribArray(position);
        glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 0, 0);
        ASSERT_GL_NO_ERROR();

        static const float kDst[4]  = {0.5f, 0.5f, 0.5f, 0.5f};
        static const float kSrc0[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        static const float kSrc1[4] = {0.3f, 0.6f, 0.9f, 0.7f};

        setUniform4f(src0, kSrc0[0], kSrc0[1], kSrc0[2], kSrc0[3]);
        setUniform4f(src1, kSrc1[0], kSrc1[1], kSrc1[2], kSrc1[3]);
        ASSERT_GL_NO_ERROR();

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glViewport(0, 0, kWidth, kHeight);
        glClearColor(kDst[0], kDst[1], kDst[2], kDst[3]);
        ASSERT_GL_NO_ERROR();

        {
            glBlendFuncSeparate(GL_SRC1_COLOR_EXT, GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_COLOR_EXT,
                                GL_ONE_MINUS_SRC1_ALPHA_EXT);

            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            ASSERT_GL_NO_ERROR();

            // verify
            angle::GLColor color;
            BlendEquationFuncAdd<GL_SRC1_COLOR_EXT, GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_COLOR_EXT,
                                 GL_ONE_MINUS_SRC1_ALPHA_EXT>(kDst, kSrc0, kSrc1, &color);

            CheckPixels(kWidth / 4, (3 * kHeight) / 4, 1, 1, 1, color);
            CheckPixels(kWidth - 1, 0, 1, 1, 1, color);
        }

        {
            glBlendFuncSeparate(GL_ONE_MINUS_SRC1_COLOR_EXT, GL_ONE_MINUS_SRC_ALPHA,
                                GL_ONE_MINUS_SRC_COLOR, GL_SRC1_ALPHA_EXT);

            glClear(GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            ASSERT_GL_NO_ERROR();

            // verify
            angle::GLColor color;
            BlendEquationFuncAdd<GL_ONE_MINUS_SRC1_COLOR_EXT, GL_ONE_MINUS_SRC_ALPHA,
                                 GL_ONE_MINUS_SRC_COLOR, GL_SRC1_ALPHA_EXT>(kDst, kSrc0, kSrc1,
                                                                            &color);

            CheckPixels(kWidth / 4, (3 * kHeight) / 4, 1, 1, 1, color);
            CheckPixels(kWidth - 1, 0, 1, 1, 1, color);
        }
    }

    GLuint mVBO;
    GLuint mProgram;
};

class EXTBlendFuncExtendedDrawTestES3 : public EXTBlendFuncExtendedDrawTest
{
  protected:
    EXTBlendFuncExtendedDrawTestES3() : EXTBlendFuncExtendedDrawTest(), mIsES31OrNewer(false) {}

    void testSetUp() override
    {
        EXTBlendFuncExtendedDrawTest::testSetUp();
        if (getClientMajorVersion() > 3 ||
            (getClientMajorVersion() == 3 && getClientMinorVersion() >= 1))
        {
            mIsES31OrNewer = true;
        }
    }

    virtual void checkOutputIndexQuery(const char *name, GLint expectedIndex)
    {
        GLint index = glGetFragDataIndexEXT(mProgram, name);
        EXPECT_EQ(expectedIndex, index);
        if (mIsES31OrNewer)
        {
            index = glGetProgramResourceLocationIndexEXT(mProgram, GL_PROGRAM_OUTPUT, name);
            EXPECT_EQ(expectedIndex, index);
        }
        else
        {
            glGetProgramResourceLocationIndexEXT(mProgram, GL_PROGRAM_OUTPUT, name);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        }
    }

    void LinkProgram()
    {
        glLinkProgram(mProgram);
        GLint linked = 0;
        glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
        EXPECT_NE(0, linked);
        glUseProgram(mProgram);
        return;
    }

  private:
    bool mIsES31OrNewer;
};

class EXTBlendFuncExtendedDrawTestES31 : public EXTBlendFuncExtendedDrawTestES3
{
  protected:
    EXTBlendFuncExtendedDrawTestES31()
        : EXTBlendFuncExtendedDrawTestES3(), mPipeline(0), mVertexProgram(0), mFragProgram(0)
    {}

    GLint getVertexAttribLocation(const char *name) override
    {
        return glGetAttribLocation(mVertexProgram, name);
    }

    GLint getFragmentUniformLocation(const char *name) override
    {
        return glGetUniformLocation(mFragProgram, name);
    }

    void setUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) override
    {
        glActiveShaderProgram(mPipeline, mFragProgram);
        EXTBlendFuncExtendedDrawTest::setUniform4f(location, v0, v1, v2, v3);
    }

    void checkOutputIndexQuery(const char *name, GLint expectedIndex) override
    {
        GLint index = glGetFragDataIndexEXT(mFragProgram, name);
        EXPECT_EQ(expectedIndex, index);
        index = glGetProgramResourceLocationIndexEXT(mFragProgram, GL_PROGRAM_OUTPUT, name);
        EXPECT_EQ(expectedIndex, index);
    }

    void setupProgramPipeline(const char *vertexSource, const char *fragmentSource)
    {
        mVertexProgram = createShaderProgram(GL_VERTEX_SHADER, vertexSource);
        ASSERT_NE(mVertexProgram, 0u);
        mFragProgram = createShaderProgram(GL_FRAGMENT_SHADER, fragmentSource);
        ASSERT_NE(mFragProgram, 0u);

        // Generate a program pipeline and attach the programs to their respective stages
        glGenProgramPipelines(1, &mPipeline);
        EXPECT_GL_NO_ERROR();
        glUseProgramStages(mPipeline, GL_VERTEX_SHADER_BIT, mVertexProgram);
        EXPECT_GL_NO_ERROR();
        glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProgram);
        EXPECT_GL_NO_ERROR();
        glBindProgramPipeline(mPipeline);
        EXPECT_GL_NO_ERROR();
    }

    GLuint createShaderProgram(GLenum type, const GLchar *shaderString)
    {
        GLShader shader(type);
        if (!shader)
        {
            return 0;
        }

        glShaderSource(shader, 1, &shaderString, nullptr);
        glCompileShader(shader);

        GLuint program = glCreateProgram();

        if (program)
        {
            GLint compiled;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
            if (compiled)
            {
                glAttachShader(program, shader);
                glLinkProgram(program);
                glDetachShader(program, shader);
            }
        }

        EXPECT_GL_NO_ERROR();

        return program;
    }

    void testTearDown() override
    {
        EXTBlendFuncExtendedDrawTest::testTearDown();
        if (mVertexProgram)
        {
            glDeleteProgram(mVertexProgram);
        }
        if (mFragProgram)
        {
            glDeleteProgram(mFragProgram);
        }
        if (mPipeline)
        {
            glDeleteProgramPipelines(1, &mPipeline);
        }

        ASSERT_GL_NO_ERROR();
    }

    GLuint mPipeline;
    GLuint mVertexProgram;
    GLuint mFragProgram;
};
}  // namespace

// Test EXT_blend_func_extended related gets.
TEST_P(EXTBlendFuncExtendedTest, TestMaxDualSourceDrawBuffers)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    GLint maxDualSourceDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS_EXT, &maxDualSourceDrawBuffers);
    EXPECT_GT(maxDualSourceDrawBuffers, 0);

    ASSERT_GL_NO_ERROR();
}

// Test that SRC1 factors limit the number of allowed draw buffers.
TEST_P(EXTBlendFuncExtendedTest, MaxDualSourceDrawBuffersError)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_draw_buffers"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_rgb8_rgba8"));

    GLint maxDualSourceDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS_EXT, &maxDualSourceDrawBuffers);
    ANGLE_SKIP_TEST_IF(maxDualSourceDrawBuffers != 1);

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer rb0;
    glBindRenderbuffer(GL_RENDERBUFFER, rb0);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER, rb0);

    GLRenderbuffer rb1;
    glBindRenderbuffer(GL_RENDERBUFFER, rb1);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, 1, 1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1_EXT, GL_RENDERBUFFER, rb1);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    const GLenum bufs[] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT};
    glDrawBuffersEXT(2, bufs);
    ASSERT_GL_NO_ERROR();

    for (const GLenum func : {GL_SRC1_COLOR_EXT, GL_ONE_MINUS_SRC1_COLOR_EXT, GL_SRC1_ALPHA_EXT,
                              GL_ONE_MINUS_SRC1_ALPHA_EXT})
    {
        for (size_t slot = 0; slot < 4; slot++)
        {
            switch (slot)
            {
                case 0:
                    glBlendFuncSeparate(func, GL_ONE, GL_ONE, GL_ONE);
                    break;
                case 1:
                    glBlendFuncSeparate(GL_ONE, func, GL_ONE, GL_ONE);
                    break;
                case 2:
                    glBlendFuncSeparate(GL_ONE, GL_ONE, func, GL_ONE);
                    break;
                case 3:
                    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, func);
                    break;
            }
            // Limit must be applied even with blending disabled
            glDisable(GL_BLEND);
            drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.0);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);

            glEnable(GL_BLEND);
            drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.0);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);

            // Limit must be applied even when an attachment is missing
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1_EXT, GL_RENDERBUFFER, 0);
            drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.0);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);

            // Restore the attachment for the next iteration
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1_EXT, GL_RENDERBUFFER,
                                      rb1);

            // Limit is not applied when non-SRC1 funcs are used
            glBlendFunc(GL_ONE, GL_ONE);
            drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.0);
            EXPECT_GL_NO_ERROR();
        }
    }
}

// Test a shader with EXT_blend_func_extended and gl_SecondaryFragColorEXT.
// Outputs to primary color buffer using primary and secondary colors.
TEST_P(EXTBlendFuncExtendedDrawTest, FragColor)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    const char *kFragColorShader =
        "#extension GL_EXT_blend_func_extended : require\n"
        "precision mediump float;\n"
        "uniform vec4 src0;\n"
        "uniform vec4 src1;\n"
        "void main() {\n"
        "  gl_FragColor = src0;\n"
        "  gl_SecondaryFragColorEXT = src1;\n"
        "}\n";

    makeProgram(essl1_shaders::vs::Simple(), kFragColorShader);

    drawTest();
}

// Test a shader with EXT_blend_func_extended and EXT_draw_buffers enabled at the same time.
TEST_P(EXTBlendFuncExtendedDrawTest, FragColorBroadcast)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_draw_buffers"));

    const char *kFragColorShader =
        "#extension GL_EXT_blend_func_extended : require\n"
        "#extension GL_EXT_draw_buffers : require\n"
        "precision mediump float;\n"
        "uniform vec4 src0;\n"
        "uniform vec4 src1;\n"
        "void main() {\n"
        "  gl_FragColor = src0;\n"
        "  gl_SecondaryFragColorEXT = src1;\n"
        "}\n";

    makeProgram(essl1_shaders::vs::Simple(), kFragColorShader);

    drawTest();
}

// Test a shader with EXT_blend_func_extended and gl_FragData.
// Outputs to a color buffer using primary and secondary frag data.
TEST_P(EXTBlendFuncExtendedDrawTest, FragData)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    const char *kFragColorShader =
        "#extension GL_EXT_blend_func_extended : require\n"
        "precision mediump float;\n"
        "uniform vec4 src0;\n"
        "uniform vec4 src1;\n"
        "void main() {\n"
        "  gl_FragData[0] = src0;\n"
        "  gl_SecondaryFragDataEXT[0] = src1;\n"
        "}\n";

    makeProgram(essl1_shaders::vs::Simple(), kFragColorShader);

    drawTest();
}

// Test that min/max blending works correctly with SRC1 factors.
TEST_P(EXTBlendFuncExtendedDrawTest, MinMax)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_minmax"));

    const char *kFragColorShader = R"(#extension GL_EXT_blend_func_extended : require
precision mediump float;
void main() {
    gl_FragColor             = vec4(0.125, 0.25, 0.75, 0.875);
    gl_SecondaryFragColorEXT = vec4(0.0, 0.0, 0.0, 0.0);
})";
    makeProgram(essl1_shaders::vs::Simple(), kFragColorShader);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC1_COLOR_EXT, GL_ONE_MINUS_SRC1_COLOR_EXT, GL_SRC1_ALPHA_EXT,
                        GL_ONE_MINUS_SRC1_ALPHA_EXT);
    glClearColor(0.5, 0.5, 0.5, 0.5);

    auto test = [&](GLenum colorOp, GLenum alphaOp, GLColor color) {
        glBlendEquationSeparate(colorOp, alphaOp);
        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.0);
        EXPECT_PIXEL_COLOR_NEAR(0, 0, color, 2);
    };
    test(GL_MIN_EXT, GL_MIN_EXT, GLColor(32, 64, 128, 128));
    test(GL_MIN_EXT, GL_MAX_EXT, GLColor(32, 64, 128, 224));
    test(GL_MAX_EXT, GL_MIN_EXT, GLColor(128, 128, 192, 128));
    test(GL_MAX_EXT, GL_MAX_EXT, GLColor(128, 128, 192, 224));
}

// Test an ESSL 3.00 shader that uses two fragment outputs with locations specified in the shader.
TEST_P(EXTBlendFuncExtendedDrawTestES3, FragmentOutputLocationsInShader)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    const char *kFragColorShader = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src0;
uniform vec4 src1;
layout(location = 0, index = 1) out vec4 outSrc1;
layout(location = 0, index = 0) out vec4 outSrc0;
void main() {
    outSrc0 = src0;
    outSrc1 = src1;
})";

    makeProgram(essl3_shaders::vs::Simple(), kFragColorShader);

    checkOutputIndexQuery("outSrc0", 0);
    checkOutputIndexQuery("outSrc1", 1);

    drawTest();
}

// Test an ESSL 3.00 shader that uses two fragment outputs with locations specified through the API.
TEST_P(EXTBlendFuncExtendedDrawTestES3, FragmentOutputLocationsAPI)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src0;
uniform vec4 src1;
out vec4 outSrc1;
out vec4 outSrc0;
void main() {
    outSrc0 = src0;
    outSrc1 = src1;
})";

    mProgram = CompileProgram(essl3_shaders::vs::Simple(), kFS, [](GLuint program) {
        glBindFragDataLocationIndexedEXT(program, 0, 0, "outSrc0");
        glBindFragDataLocationIndexedEXT(program, 0, 1, "outSrc1");
    });

    ASSERT_NE(0u, mProgram);

    checkOutputIndexQuery("outSrc0", 0);
    checkOutputIndexQuery("outSrc1", 1);

    drawTest();
}

// Test an ESSL 3.00 shader that uses two fragment outputs, with location for one specified through
// the API and location for another being set automatically.
TEST_P(EXTBlendFuncExtendedDrawTestES3, FragmentOutputLocationsAPIAndAutomatic)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src0;
uniform vec4 src1;
out vec4 outSrc1;
out vec4 outSrc0;
void main() {
    outSrc0 = src0;
    outSrc1 = src1;
})";

    mProgram = CompileProgram(essl3_shaders::vs::Simple(), kFS, [](GLuint program) {
        glBindFragDataLocationIndexedEXT(program, 0, 1, "outSrc1");
    });

    ASSERT_NE(0u, mProgram);

    checkOutputIndexQuery("outSrc0", 0);
    checkOutputIndexQuery("outSrc1", 1);

    drawTest();
}

// Test an ESSL 3.00 shader that uses two array fragment outputs with locations
// specified in the shader.
TEST_P(EXTBlendFuncExtendedDrawTestES3, FragmentArrayOutputLocationsInShader)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    const char *kFragColorShader = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src0;
uniform vec4 src1;
layout(location = 0, index = 1) out vec4 outSrc1[1];
layout(location = 0, index = 0) out vec4 outSrc0[1];
void main() {
    outSrc0[0] = src0;
    outSrc1[0] = src1;
})";

    makeProgram(essl3_shaders::vs::Simple(), kFragColorShader);

    checkOutputIndexQuery("outSrc0[0]", 0);
    checkOutputIndexQuery("outSrc1[0]", 1);
    checkOutputIndexQuery("outSrc0", 0);
    checkOutputIndexQuery("outSrc1", 1);

    // These queries use an out of range array index so they should return -1.
    checkOutputIndexQuery("outSrc0[1]", -1);
    checkOutputIndexQuery("outSrc1[1]", -1);

    drawTest();
}

// Test an ESSL 3.00 shader that uses two array fragment outputs with locations specified through
// the API.
TEST_P(EXTBlendFuncExtendedDrawTestES3, FragmentArrayOutputLocationsAPI)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src0;
uniform vec4 src1;
out vec4 outSrc1[1];
out vec4 outSrc0[1];
void main() {
    outSrc0[0] = src0;
    outSrc1[0] = src1;
})";

    mProgram = CompileProgram(essl3_shaders::vs::Simple(), kFS, [](GLuint program) {
        // Specs aren't very clear on what kind of name should be used when binding location for
        // array variables. We only allow names that do include the "[0]" suffix.
        glBindFragDataLocationIndexedEXT(program, 0, 0, "outSrc0[0]");
        glBindFragDataLocationIndexedEXT(program, 0, 1, "outSrc1[0]");
    });

    ASSERT_NE(0u, mProgram);

    // The extension spec is not very clear on what name can be used for the queries for array
    // variables. We're checking that the queries work in the same way as specified in OpenGL 4.4
    // page 107.
    checkOutputIndexQuery("outSrc0[0]", 0);
    checkOutputIndexQuery("outSrc1[0]", 1);
    checkOutputIndexQuery("outSrc0", 0);
    checkOutputIndexQuery("outSrc1", 1);

    // These queries use an out of range array index so they should return -1.
    checkOutputIndexQuery("outSrc0[1]", -1);
    checkOutputIndexQuery("outSrc1[1]", -1);

    drawTest();
}

// Ported from TranslatorVariants/EXTBlendFuncExtendedES3DrawTest
// Test that tests glBindFragDataLocationEXT, glBindFragDataLocationIndexedEXT,
// glGetFragDataLocation, glGetFragDataIndexEXT work correctly with
// GLSL array output variables. The output variable can be bound by
// referring to the variable name with or without the first element array
// accessor. The getters can query location of the individual elements in
// the array. The test does not actually use the base test drawing,
// since the drivers at the time of writing do not support multiple
// buffers and dual source blending.
TEST_P(EXTBlendFuncExtendedDrawTestES3, ES3GettersArray)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    const GLint kTestArraySize     = 2;
    const GLint kFragData0Location = 2;
    const GLint kFragData1Location = 1;
    const GLint kUnusedLocation    = 5;

    // The test binds kTestArraySize -sized array to location 1 for test purposes.
    // The GL_MAX_DRAW_BUFFERS must be > kTestArraySize, since an
    // array will be bound to continuous locations, starting from the first
    // location.
    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS_EXT, &maxDrawBuffers);
    EXPECT_LT(kTestArraySize, maxDrawBuffers);

    constexpr char kFragColorShader[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src;
uniform vec4 src1;
out vec4 FragData[2];
void main() {
    FragData[0] = src;
    FragData[1] = src1;
})";

    struct testCase
    {
        std::string unusedLocationName;
        std::string fragData0LocationName;
        std::string fragData1LocationName;
    };

    testCase testCases[4]{{"FragData[0]", "FragData", "FragData[1]"},
                          {"FragData", "FragData[0]", "FragData[1]"},
                          {"FragData[0]", "FragData", "FragData[1]"},
                          {"FragData", "FragData[0]", "FragData[1]"}};

    for (const testCase &test : testCases)
    {
        mProgram =
            CompileProgram(essl3_shaders::vs::Simple(), kFragColorShader, [&](GLuint program) {
                glBindFragDataLocationEXT(program, kUnusedLocation,
                                          test.unusedLocationName.c_str());
                glBindFragDataLocationEXT(program, kFragData0Location,
                                          test.fragData0LocationName.c_str());
                glBindFragDataLocationEXT(program, kFragData1Location,
                                          test.fragData1LocationName.c_str());
            });

        EXPECT_EQ(static_cast<GLenum>(GL_NO_ERROR), glGetError());
        LinkProgram();
        EXPECT_EQ(kFragData0Location, glGetFragDataLocation(mProgram, "FragData"));
        EXPECT_EQ(0, glGetFragDataIndexEXT(mProgram, "FragData"));
        EXPECT_EQ(kFragData0Location, glGetFragDataLocation(mProgram, "FragData[0]"));
        EXPECT_EQ(0, glGetFragDataIndexEXT(mProgram, "FragData[0]"));
        EXPECT_EQ(kFragData1Location, glGetFragDataLocation(mProgram, "FragData[1]"));
        EXPECT_EQ(0, glGetFragDataIndexEXT(mProgram, "FragData[1]"));
        // Index bigger than the GLSL variable array length does not find anything.
        EXPECT_EQ(-1, glGetFragDataLocation(mProgram, "FragData[3]"));
    }
}

// Ported from TranslatorVariants/EXTBlendFuncExtendedES3DrawTest
TEST_P(EXTBlendFuncExtendedDrawTestES3, ESSL3BindSimpleVarAsArrayNoBind)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    constexpr char kFragDataShader[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src;
uniform vec4 src1;
out vec4 FragData;
out vec4 SecondaryFragData;
void main() {
    FragData = src;
    SecondaryFragData = src1;
})";

    mProgram = CompileProgram(essl3_shaders::vs::Simple(), kFragDataShader, [](GLuint program) {
        glBindFragDataLocationEXT(program, 0, "FragData[0]");
        glBindFragDataLocationIndexedEXT(program, 0, 1, "SecondaryFragData[0]");
    });

    LinkProgram();

    EXPECT_EQ(-1, glGetFragDataLocation(mProgram, "FragData[0]"));
    EXPECT_EQ(0, glGetFragDataLocation(mProgram, "FragData"));
    EXPECT_EQ(1, glGetFragDataLocation(mProgram, "SecondaryFragData"));
    // Did not bind index.
    EXPECT_EQ(0, glGetFragDataIndexEXT(mProgram, "SecondaryFragData"));

    glBindFragDataLocationEXT(mProgram, 0, "FragData");
    glBindFragDataLocationIndexedEXT(mProgram, 0, 1, "SecondaryFragData");
    LinkProgram();
}

// Test an ESSL 3.00 program with a link-time fragment output location conflict.
TEST_P(EXTBlendFuncExtendedTestES3, FragmentOutputLocationConflict)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src0;
uniform vec4 src1;
out vec4 out0;
out vec4 out1;
void main() {
    out0 = src0;
    out1 = src1;
})";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, essl3_shaders::vs::Simple());
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);
    ASSERT_NE(0u, vs);
    ASSERT_NE(0u, fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glDeleteShader(vs);
    glAttachShader(program, fs);
    glDeleteShader(fs);

    glBindFragDataLocationIndexedEXT(program, 0, 0, "out0");
    glBindFragDataLocationIndexedEXT(program, 0, 0, "out1");

    // The program should fail to link.
    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(0, linkStatus);

    glDeleteProgram(program);
}

// Test an ESSL 3.00 program with some bindings set for nonexistent variables. These should not
// create link-time conflicts.
TEST_P(EXTBlendFuncExtendedTestES3, FragmentOutputLocationForNonexistentOutput)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src0;
out vec4 out0;
void main() {
    out0 = src0;
})";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, essl3_shaders::vs::Simple());
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);
    ASSERT_NE(0u, vs);
    ASSERT_NE(0u, fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glDeleteShader(vs);
    glAttachShader(program, fs);
    glDeleteShader(fs);

    glBindFragDataLocationIndexedEXT(program, 0, 0, "out0");
    glBindFragDataLocationIndexedEXT(program, 0, 0, "out1");
    glBindFragDataLocationIndexedEXT(program, 0, 0, "out2[0]");

    // The program should link successfully - conflicting location for nonexistent variables out1 or
    // out2 should not be an issue.
    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_NE(0, linkStatus);

    glDeleteProgram(program);
}

// Test mixing shader-assigned and automatic output locations.
TEST_P(EXTBlendFuncExtendedTestES3, FragmentOutputLocationsPartiallyAutomatic)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    GLint maxDrawBuffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    ANGLE_SKIP_TEST_IF(maxDrawBuffers < 4);

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src0;
uniform vec4 src1;
uniform vec4 src2;
uniform vec4 src3;
layout(location=0) out vec4 out0;
layout(location=3) out vec4 out3;
out vec4 out12[2];
void main() {
    out0 = src0;
    out12[0] = src1;
    out12[1] = src2;
    out3 = src3;
})";

    GLuint program = CompileProgram(essl3_shaders::vs::Simple(), kFS);
    ASSERT_NE(0u, program);

    GLint location = glGetFragDataLocation(program, "out0");
    EXPECT_EQ(0, location);
    location = glGetFragDataLocation(program, "out12");
    EXPECT_EQ(1, location);
    location = glGetFragDataLocation(program, "out3");
    EXPECT_EQ(3, location);

    glDeleteProgram(program);
}

// Test a fragment output array that doesn't fit because contiguous locations are not available.
TEST_P(EXTBlendFuncExtendedTestES3, FragmentOutputArrayDoesntFit)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    GLint maxDrawBuffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    ANGLE_SKIP_TEST_IF(maxDrawBuffers < 4);

    std::stringstream fragShader;
    fragShader << R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
layout(location=2) out vec4 out0;
out vec4 outArray[)"
               << (maxDrawBuffers - 1) << R"(];
void main() {
    out0 = vec4(1.0);
    outArray[0] = vec4(1.0);
})";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, essl3_shaders::vs::Simple());
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragShader.str().c_str());
    ASSERT_NE(0u, vs);
    ASSERT_NE(0u, fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glDeleteShader(vs);
    glAttachShader(program, fs);
    glDeleteShader(fs);

    // The program should not link - there's no way to fit "outArray" into available output
    // locations.
    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(0, linkStatus);

    glDeleteProgram(program);
}

// Test that a secondary blending source limits the number of primary outputs.
TEST_P(EXTBlendFuncExtendedTestES3, TooManyFragmentOutputsForDualSourceBlending)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    GLint maxDualSourceDrawBuffers;
    glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS_EXT, &maxDualSourceDrawBuffers);
    ASSERT_GE(maxDualSourceDrawBuffers, 1);

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
out vec4 outSrc0;
out vec4 outSrc1;
void main() {
    outSrc0 = vec4(0.5);
    outSrc1 = vec4(1.0);
})";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, essl3_shaders::vs::Simple());
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);
    ASSERT_NE(0u, vs);
    ASSERT_NE(0u, fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glDeleteShader(vs);
    glAttachShader(program, fs);
    glDeleteShader(fs);

    glBindFragDataLocationIndexedEXT(program, maxDualSourceDrawBuffers, 0, "outSrc0");
    glBindFragDataLocationIndexedEXT(program, 0, 1, "outSrc1");
    ASSERT_GL_NO_ERROR();

    GLint linkStatus;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(0, linkStatus);

    glDeleteProgram(program);
}

// Test that fragment outputs bound to the same location must have the same type.
TEST_P(EXTBlendFuncExtendedTestES3, InconsistentTypesForLocationAPI)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
out vec4 outSrc0;
out ivec4 outSrc1;
void main() {
    outSrc0 = vec4(0.5);
    outSrc1 = ivec4(1.0);
})";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, essl3_shaders::vs::Simple());
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);
    ASSERT_NE(0u, vs);
    ASSERT_NE(0u, fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glDeleteShader(vs);
    glAttachShader(program, fs);
    glDeleteShader(fs);

    glBindFragDataLocationIndexedEXT(program, 0, 0, "outSrc0");
    glBindFragDataLocationIndexedEXT(program, 0, 1, "outSrc1");
    ASSERT_GL_NO_ERROR();

    GLint linkStatus;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(0, linkStatus);

    glDeleteProgram(program);
}

// Test that rendering to multiple fragment outputs bound via API works.
TEST_P(EXTBlendFuncExtendedDrawTestES3, MultipleDrawBuffersAPI)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    constexpr char kFS[] = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
out vec4 outSrc0;
out ivec4 outSrc1;
void main() {
    outSrc0 = vec4(0.0, 1.0, 0.0, 1.0);
    outSrc1 = ivec4(1, 2, 3, 4);
})";

    mProgram = CompileProgram(essl3_shaders::vs::Simple(), kFS, [](GLuint program) {
        glBindFragDataLocationEXT(program, 0, "outSrc0");
        glBindFragDataLocationEXT(program, 1, "outSrc1");
    });

    ASSERT_NE(0u, mProgram);

    GLRenderbuffer rb0;
    glBindRenderbuffer(GL_RENDERBUFFER, rb0);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);

    GLRenderbuffer rb1;
    glBindRenderbuffer(GL_RENDERBUFFER, rb1);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8I, 1, 1);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    const GLenum bufs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, bufs);

    GLfloat clearF[] = {0.0, 0.0, 0.0, 0.0};
    GLint clearI[]   = {0, 0, 0, 0};

    // FBO: rb0 (rgba8), rb1 (rgba8i)
    glBindRenderbuffer(GL_RENDERBUFFER, rb0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb0);
    glBindRenderbuffer(GL_RENDERBUFFER, rb1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, rb1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearBufferfv(GL_COLOR, 0, clearF);
    glClearBufferiv(GL_COLOR, 1, clearI);
    ASSERT_GL_NO_ERROR();

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_8I(0, 0, 0, 0, 0, 0);

    drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.0);
    ASSERT_GL_NO_ERROR();

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_8I(0, 0, 1, 2, 3, 4);

    // FBO: rb1 (rgba8i), rb0 (rgba8)
    glBindRenderbuffer(GL_RENDERBUFFER, rb0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, rb0);
    glBindRenderbuffer(GL_RENDERBUFFER, rb1);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Rebind fragment outputs
    glBindFragDataLocationEXT(mProgram, 0, "outSrc1");
    glBindFragDataLocationEXT(mProgram, 1, "outSrc0");
    glLinkProgram(mProgram);

    glClearBufferfv(GL_COLOR, 1, clearF);
    glClearBufferiv(GL_COLOR, 0, clearI);
    ASSERT_GL_NO_ERROR();

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_8I(0, 0, 0, 0, 0, 0);

    drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.0);
    ASSERT_GL_NO_ERROR();

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_8I(0, 0, 1, 2, 3, 4);
}

// Use a program pipeline with EXT_blend_func_extended
TEST_P(EXTBlendFuncExtendedDrawTestES31, UseProgramPipeline)
{
    // Only the Vulkan backend supports PPO
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    const char *kFragColorShader = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src0;
uniform vec4 src1;
layout(location = 0, index = 1) out vec4 outSrc1;
layout(location = 0, index = 0) out vec4 outSrc0;
void main() {
    outSrc0 = src0;
    outSrc1 = src1;
})";

    setupProgramPipeline(essl3_shaders::vs::Simple(), kFragColorShader);

    checkOutputIndexQuery("outSrc0", 0);
    checkOutputIndexQuery("outSrc1", 1);

    ASSERT_EQ(mProgram, 0u);
    drawTest();

    ASSERT_GL_NO_ERROR();
}

// Use program pipeline where the fragment program is changed
TEST_P(EXTBlendFuncExtendedDrawTestES31, UseTwoProgramStages)
{
    // Only the Vulkan backend supports PPO
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_blend_func_extended"));

    const char *kFragColorShaderFlipped = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src0;
uniform vec4 src1;
layout(location = 0, index = 0) out vec4 outSrc1;
layout(location = 0, index = 1) out vec4 outSrc0;
void main() {
    outSrc0 = src0;
    outSrc1 = src1;
})";

    const char *kFragColorShader = R"(#version 300 es
#extension GL_EXT_blend_func_extended : require
precision mediump float;
uniform vec4 src0;
uniform vec4 src1;
layout(location = 0, index = 1) out vec4 outSrc1;
layout(location = 0, index = 0) out vec4 outSrc0;
void main() {
    outSrc0 = src0;
    outSrc1 = src1;
})";

    setupProgramPipeline(essl3_shaders::vs::Simple(), kFragColorShaderFlipped);

    // Check index values frag shader with the "flipped" index values
    checkOutputIndexQuery("outSrc0", 1);
    checkOutputIndexQuery("outSrc1", 0);

    GLuint previousProgram = mFragProgram;
    mFragProgram           = createShaderProgram(GL_FRAGMENT_SHADER, kFragColorShader);
    ASSERT_NE(mFragProgram, 0u);

    // Change the Fragment program of the pipeline
    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProgram);
    EXPECT_GL_NO_ERROR();

    checkOutputIndexQuery("outSrc0", 0);
    checkOutputIndexQuery("outSrc1", 1);

    ASSERT_EQ(mProgram, 0u);
    drawTest();

    if (previousProgram)
    {
        glDeleteProgram(previousProgram);
    }
    ASSERT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES2(EXTBlendFuncExtendedTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EXTBlendFuncExtendedTestES3);
ANGLE_INSTANTIATE_TEST_ES3_AND_ES31(EXTBlendFuncExtendedTestES3);

ANGLE_INSTANTIATE_TEST_ES2(EXTBlendFuncExtendedDrawTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EXTBlendFuncExtendedDrawTestES3);
ANGLE_INSTANTIATE_TEST_ES3_AND_ES31(EXTBlendFuncExtendedDrawTestES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EXTBlendFuncExtendedDrawTestES31);
ANGLE_INSTANTIATE_TEST_ES31(EXTBlendFuncExtendedDrawTestES31);
