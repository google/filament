//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BindUniformLocationTest.cpp : Tests of the GL_CHROMIUM_bind_uniform_location extension.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include <cmath>

using namespace angle;

namespace
{

class BindUniformLocationTest : public ANGLETest<>
{
  protected:
    BindUniformLocationTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testTearDown() override
    {
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
        }
    }

    GLuint mProgram = 0;
};

// Test basic functionality of GL_CHROMIUM_bind_uniform_location
TEST_P(BindUniformLocationTest, Basic)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_bind_uniform_location"));

    constexpr char kFS[] = R"(precision mediump float;
uniform vec4 u_colorC;
uniform vec4 u_colorB[2];
uniform vec4 u_colorA;
void main()
{
    gl_FragColor = u_colorA + u_colorB[0] + u_colorB[1] + u_colorC;
})";

    GLint colorALocation = 3;
    GLint colorBLocation = 10;
    GLint colorCLocation = 5;

    mProgram = CompileProgram(essl1_shaders::vs::Simple(), kFS, [&](GLuint program) {
        glBindUniformLocationCHROMIUM(program, colorALocation, "u_colorA");
        glBindUniformLocationCHROMIUM(program, colorBLocation, "u_colorB[0]");
        glBindUniformLocationCHROMIUM(program, colorCLocation, "u_colorC");
    });
    ASSERT_NE(0u, mProgram);

    glUseProgram(mProgram);

    static const float colorB[] = {
        0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 0.0f,
    };

    glUniform4f(colorALocation, 0.25f, 0.0f, 0.0f, 0.0f);
    glUniform4fv(colorBLocation, 2, colorB);
    glUniform4f(colorCLocation, 0.0f, 0.0f, 0.0f, 1.0f);

    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 64, 128, 192, 255, 1.0);
}

// Force a sampler location and make sure it samples the correct texture
TEST_P(BindUniformLocationTest, SamplerLocation)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_bind_uniform_location"));

    constexpr char kFS[] = R"(precision mediump float;
uniform vec4 u_colorA;
uniform vec4 u_colorB[2];
uniform sampler2D u_sampler;
void main()
{
    gl_FragColor = u_colorA + u_colorB[0] + u_colorB[1] + texture2D(u_sampler, vec2(0, 0));
})";

    GLint colorALocation  = 3;
    GLint colorBLocation  = 10;
    GLint samplerLocation = 1;

    mProgram = CompileProgram(essl1_shaders::vs::Simple(), kFS, [&](GLuint program) {
        glBindUniformLocationCHROMIUM(program, colorALocation, "u_colorA");
        glBindUniformLocationCHROMIUM(program, colorBLocation, "u_colorB[0]");
        glBindUniformLocationCHROMIUM(program, samplerLocation, "u_sampler");
    });
    ASSERT_NE(0u, mProgram);

    glUseProgram(mProgram);

    static const float colorB[] = {
        0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 0.0f,
    };

    glUniform4f(colorALocation, 0.25f, 0.0f, 0.0f, 0.0f);
    glUniform4fv(colorBLocation, 2, colorB);

    // Point the texture at texture unit 2
    glUniform1i(samplerLocation, 2);

    GLTexture texture;
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture);
    constexpr GLubyte kTextureData[] = {32, 32, 32, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, kTextureData);

    drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 96, 160, 224, 255, 1.0);
}

// Test that conflicts are detected when two uniforms are bound to the same location
TEST_P(BindUniformLocationTest, ConflictsDetection)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_bind_uniform_location"));

    constexpr char kFS[] =
        R"(precision mediump float;
        uniform vec4 u_colorA;
        uniform vec4 u_colorB;
        void main()
        {
            gl_FragColor = u_colorA + u_colorB;
        })";

    GLint colorALocation = 3;
    GLint colorBLocation = 4;

    GLuint vs = CompileShader(GL_VERTEX_SHADER, essl1_shaders::vs::Simple());
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);

    mProgram = glCreateProgram();
    glAttachShader(mProgram, vs);
    glDeleteShader(vs);
    glAttachShader(mProgram, fs);
    glDeleteShader(fs);

    glBindUniformLocationCHROMIUM(mProgram, colorALocation, "u_colorA");
    // Bind u_colorB to location a, causing conflicts, link should fail.
    glBindUniformLocationCHROMIUM(mProgram, colorALocation, "u_colorB");
    glLinkProgram(mProgram);
    GLint linked = 0;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
    ASSERT_EQ(0, linked);

    // Bind u_colorB to location b, no conflicts, link should succeed.
    glBindUniformLocationCHROMIUM(mProgram, colorBLocation, "u_colorB");
    glLinkProgram(mProgram);
    linked = 0;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
    EXPECT_EQ(1, linked);
}

// Test a use case of the chromium compositor
TEST_P(BindUniformLocationTest, Compositor)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_bind_uniform_location"));

    constexpr char kVS[] =
        R"(attribute vec4 a_position;
        attribute vec2 a_texCoord;
        uniform mat4 matrix;
        uniform vec2 color_a[4];
        uniform vec4 color_b;
        varying vec4 v_color;
        void main()
        {
            v_color.xy = color_a[0] + color_a[1];
            v_color.zw = color_a[2] + color_a[3];
            v_color += color_b;
            gl_Position = matrix * a_position;
        })";

    constexpr char kFS[] =
        R"(precision mediump float;
        varying vec4 v_color;
        uniform float alpha;
        uniform vec4 multiplier;
        uniform vec3 color_c[8];
        void main()
        {
            vec4 color_c_sum = vec4(0.0);
            color_c_sum.xyz += color_c[0];
            color_c_sum.xyz += color_c[1];
            color_c_sum.xyz += color_c[2];
            color_c_sum.xyz += color_c[3];
            color_c_sum.xyz += color_c[4];
            color_c_sum.xyz += color_c[5];
            color_c_sum.xyz += color_c[6];
            color_c_sum.xyz += color_c[7];
            color_c_sum.w = alpha;
            color_c_sum *= multiplier;
            gl_FragColor = v_color + color_c_sum;
        })";

    int counter            = 6;
    int matrixLocation     = counter++;
    int colorALocation     = counter++;
    int colorBLocation     = counter++;
    int alphaLocation      = counter++;
    int multiplierLocation = counter++;
    int colorCLocation     = counter++;

    mProgram = CompileProgram(kVS, kFS, [&](GLuint program) {
        glBindUniformLocationCHROMIUM(program, matrixLocation, "matrix");
        glBindUniformLocationCHROMIUM(program, colorALocation, "color_a");
        glBindUniformLocationCHROMIUM(program, colorBLocation, "color_b");
        glBindUniformLocationCHROMIUM(program, alphaLocation, "alpha");
        glBindUniformLocationCHROMIUM(program, multiplierLocation, "multiplier");
        glBindUniformLocationCHROMIUM(program, colorCLocation, "color_c");
    });
    ASSERT_NE(0u, mProgram);

    glUseProgram(mProgram);

    static const float colorA[] = {
        0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f,
    };

    static const float colorC[] = {
        0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f,
        0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f,
    };

    static const float identity[] = {
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
    };

    glUniformMatrix4fv(matrixLocation, 1, false, identity);
    glUniform2fv(colorALocation, 4, colorA);
    glUniform4f(colorBLocation, 0.2f, 0.2f, 0.2f, 0.2f);
    glUniform1f(alphaLocation, 0.8f);
    glUniform4f(multiplierLocation, 0.5f, 0.5f, 0.5f, 0.5f);
    glUniform3fv(colorCLocation, 8, colorC);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    drawQuad(mProgram, "a_position", 0.5f);

    EXPECT_PIXEL_EQ(0, 0, 204, 204, 204, 204);
}

// Test that unused uniforms don't conflict when bound to the same location
TEST_P(BindUniformLocationTest, UnusedUniformUpdate)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_bind_uniform_location"));

    ASSERT_NE(nullptr, glBindUniformLocationCHROMIUM);

    constexpr char kFS[] = R"(precision mediump float;
uniform vec4 u_colorA;
uniform float u_colorU;
uniform vec4 u_colorC;
void main()
{
    gl_FragColor = u_colorA + u_colorC;
})";

    const GLint colorULocation      = 1;
    const GLint nonexistingLocation = 5;
    const GLint unboundLocation     = 6;

    mProgram = CompileProgram(essl1_shaders::vs::Simple(), kFS, [&](GLuint program) {
        glBindUniformLocationCHROMIUM(program, colorULocation, "u_colorU");
        // The non-existing uniform should behave like existing, but optimized away
        // uniform.
        glBindUniformLocationCHROMIUM(program, nonexistingLocation, "nonexisting");
        // Let A and C be assigned automatic locations.
    });
    ASSERT_NE(0u, mProgram);

    glUseProgram(mProgram);

    // No errors on bound locations, since caller does not know
    // if the driver optimizes them away or not.
    glUniform1f(colorULocation, 0.25f);
    EXPECT_GL_NO_ERROR();

    // No errors on bound locations of names that do not exist
    // in the shader. Otherwise it would be inconsistent wrt the
    // optimization case.
    glUniform1f(nonexistingLocation, 0.25f);
    EXPECT_GL_NO_ERROR();

    // The above are equal to updating -1.
    glUniform1f(-1, 0.25f);
    EXPECT_GL_NO_ERROR();

    // No errors when updating with other type either.
    // The type can not be known with the non-existing case.
    glUniform2f(colorULocation, 0.25f, 0.25f);
    EXPECT_GL_NO_ERROR();
    glUniform2f(nonexistingLocation, 0.25f, 0.25f);
    EXPECT_GL_NO_ERROR();
    glUniform2f(-1, 0.25f, 0.25f);
    EXPECT_GL_NO_ERROR();

    // Ensure that driver or ANGLE has optimized the variable
    // away and the test tests what it is supposed to.
    EXPECT_EQ(-1, glGetUniformLocation(mProgram, "u_colorU"));

    // The bound location gets marked as used and the driver
    // does not allocate other variables to that location.
    EXPECT_NE(colorULocation, glGetUniformLocation(mProgram, "u_colorA"));
    EXPECT_NE(colorULocation, glGetUniformLocation(mProgram, "u_colorC"));
    EXPECT_NE(nonexistingLocation, glGetUniformLocation(mProgram, "u_colorA"));
    EXPECT_NE(nonexistingLocation, glGetUniformLocation(mProgram, "u_colorC"));

    // Unintuitive: while specifying value works, getting the value does not.
    GLfloat getResult = 0.0f;
    glGetUniformfv(mProgram, colorULocation, &getResult);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glGetUniformfv(mProgram, nonexistingLocation, &getResult);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glGetUniformfv(mProgram, -1, &getResult);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Updating an unbound, non-existing location still causes
    // an error.
    glUniform1f(unboundLocation, 0.25f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// GL backend optimizes away a uniform in the vertex shader if it's only used to
// compute a varying that is never referenced in the fragment shader.
TEST_P(BindUniformLocationTest, UnusedUniformUpdateComplex)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_bind_uniform_location"));

    ASSERT_NE(nullptr, glBindUniformLocationCHROMIUM);

    constexpr char kVS[] = R"(precision highp float;
attribute vec4 a_position;
varying vec4 v_unused;
uniform vec4 u_unused;
void main()
{
    gl_Position = a_position;
    v_unused = u_unused;
}
)";

    constexpr char kFS[] = R"(precision mediump float;
varying vec4 v_unused;
void main()
{
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

    const GLint unusedLocation = 1;

    mProgram = CompileProgram(kVS, kFS, [&](GLuint program) {
        glBindUniformLocationCHROMIUM(program, unusedLocation, "u_unused");
    });
    ASSERT_NE(0u, mProgram);

    glUseProgram(mProgram);

    // No errors on bound locations of names that do not exist
    // in the shader. Otherwise it would be inconsistent wrt the
    // optimization case.
    glUniform4f(unusedLocation, 0.25f, 0.25f, 0.25f, 0.25f);
    EXPECT_GL_NO_ERROR();
}

// Test for a bug where using a sampler caused GL error if the mProgram had
// uniforms that were optimized away by the driver. This was only a problem with
// glBindUniformLocationCHROMIUM implementation. This could be reproed by
// binding the sampler to a location higher than the amount of active uniforms.
TEST_P(BindUniformLocationTest, UseSamplerWhenUnusedUniforms)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_bind_uniform_location"));

    constexpr char kFS[] =
        R"(uniform sampler2D tex;
        void main()
        {
            gl_FragColor = texture2D(tex, vec2(1));
        })";

    const GLuint texLocation = 54;

    mProgram = CompileProgram(essl1_shaders::vs::Simple(), kFS, [&](GLuint program) {
        glBindUniformLocationCHROMIUM(program, texLocation, "tex");
    });
    ASSERT_NE(0u, mProgram);

    glUseProgram(mProgram);
    glUniform1i(texLocation, 0);
    EXPECT_GL_NO_ERROR();
}

// Test for binding a statically used uniform to the same location as a non-statically used uniform.
// This is valid according to the extension spec.
TEST_P(BindUniformLocationTest, SameLocationForUsedAndUnusedUniform)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_bind_uniform_location"));

    constexpr char kFS[] =
        R"(precision mediump float;
        uniform vec4 a;
        uniform vec4 b;
        void main()
        {
            gl_FragColor = a;
        })";

    const GLuint location = 54;

    mProgram = CompileProgram(essl1_shaders::vs::Zero(), kFS, [&](GLuint program) {
        glBindUniformLocationCHROMIUM(program, location, "a");
        glBindUniformLocationCHROMIUM(program, location, "b");
    });
    ASSERT_NE(0u, mProgram);

    glUseProgram(mProgram);
    glUniform4f(location, 0.0, 1.0, 0.0, 1.0);
    EXPECT_GL_NO_ERROR();
}

class BindUniformLocationES31Test : public BindUniformLocationTest
{
  protected:
    BindUniformLocationES31Test() : BindUniformLocationTest() {}

    void linkProgramWithUniformLocation(const char *vs,
                                        const char *fs,
                                        const char *uniformName,
                                        GLint uniformLocation)
    {
        mProgram = CompileProgram(vs, fs, [&](GLuint program) {
            glBindUniformLocationCHROMIUM(program, uniformLocation, uniformName);
        });
    }
};

// Test for when the shader specifies an explicit uniform location with a layout qualifier and the
// bindUniformLocation API sets a consistent location.
TEST_P(BindUniformLocationES31Test, ConsistentWithLocationLayoutQualifier)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_bind_uniform_location"));

    constexpr char kFS[] =
        "#version 310 es\n"
        "uniform layout(location=2) highp sampler2D tex;\n"
        "out highp vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = texture(tex, vec2(1));\n"
        "}\n";

    const GLuint texLocation = 2;

    linkProgramWithUniformLocation(essl31_shaders::vs::Zero(), kFS, "tex", texLocation);

    GLint linked = GL_FALSE;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
    ASSERT_GL_TRUE(linked);

    EXPECT_EQ(static_cast<GLint>(texLocation), glGetUniformLocation(mProgram, "tex"));
    glUseProgram(mProgram);
    glUniform1i(texLocation, 0);
    EXPECT_GL_NO_ERROR();
}

// Test for when the shader specifies an explicit uniform location with a layout qualifier and the
// bindUniformLocation API sets a conflicting location for the same variable. The shader-set
// location should prevail.
TEST_P(BindUniformLocationES31Test, LocationLayoutQualifierOverridesAPIBinding)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_bind_uniform_location"));

    constexpr char kFS[] =
        "#version 310 es\n"
        "uniform layout(location=2) highp sampler2D tex;\n"
        "out highp vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = texture(tex, vec2(1));\n"
        "}\n";

    const GLuint shaderTexLocation = 2;
    const GLuint texLocation       = 3;

    linkProgramWithUniformLocation(essl31_shaders::vs::Zero(), kFS, "tex", texLocation);

    GLint linked = GL_FALSE;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
    ASSERT_GL_TRUE(linked);

    EXPECT_EQ(static_cast<GLint>(shaderTexLocation), glGetUniformLocation(mProgram, "tex"));
    glUseProgram(mProgram);
    glUniform1i(shaderTexLocation, 1);
    EXPECT_GL_NO_ERROR();
    glUniform1i(texLocation, 2);
    EXPECT_GL_NO_ERROR();
}

// Test for when the shader specifies an explicit uniform location with a layout qualifier and the
// bindUniformLocation API sets a conflicting location for a different variable. Linking should
// fail.
TEST_P(BindUniformLocationES31Test, LocationLayoutQualifierConflictsWithAPIBinding)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_bind_uniform_location"));

    constexpr char kFS[] =
        "#version 310 es\n"
        "uniform layout(location=2) highp sampler2D tex;\n"
        "uniform highp sampler2D tex2;\n"
        "out highp vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = texture(tex2, vec2(1));\n"
        "}\n";

    const GLuint tex2Location = 2;

    linkProgramWithUniformLocation(essl31_shaders::vs::Zero(), kFS, "tex2", tex2Location);

    GLint linked = GL_FALSE;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
    ASSERT_GL_FALSE(linked);
}

// Test for binding a location for an array of arrays uniform.
TEST_P(BindUniformLocationES31Test, ArrayOfArrays)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_bind_uniform_location"));

    constexpr char kFS[] =
        R"(#version 310 es
        precision highp float;
        uniform vec4 sourceColor[2][1];
        out highp vec4 my_FragColor;
        void main()
        {
            my_FragColor = sourceColor[1][0];
        })";

    const GLuint location = 8;

    linkProgramWithUniformLocation(essl31_shaders::vs::Simple(), kFS, "sourceColor[1]", location);

    GLint linked = GL_FALSE;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
    ASSERT_GL_TRUE(linked);

    glUseProgram(mProgram);
    glUniform4f(location, 0.0f, 1.0f, 0.0f, 1.0f);

    drawQuad(mProgram, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2(BindUniformLocationTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BindUniformLocationES31Test);
ANGLE_INSTANTIATE_TEST_ES31(BindUniformLocationES31Test);

}  // namespace
