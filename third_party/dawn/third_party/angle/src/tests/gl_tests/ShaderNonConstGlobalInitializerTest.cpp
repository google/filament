//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Tests for GL_EXT_shader_non_constant_global_initializers
//

#include "common/mathutil.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class ShaderNonConstGlobalInitializerTest : public ANGLETest<>
{
  protected:
    ShaderNonConstGlobalInitializerTest() : ANGLETest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
    }

    void draw(GLuint program)
    {
        std::array<Vector4, 3> vertices;
        vertices[0] = {-1.0, -1.0, 0.0, 1.0};
        vertices[1] = {1.0, -1.0, 0.0, 1.0};
        vertices[2] = {0.0, 1.0, 0.0, 2.0};

        GLint positionLocation = glGetAttribLocation(program, "a_position");

        glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, vertices.data());

        glEnableVertexAttribArray(positionLocation);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
};

// Tests that the extension is disabled if not explicitly enabled- non constant initializers should
// be forbidden in all cases unless this extension is explicitly requested
TEST_P(ShaderNonConstGlobalInitializerTest, Disabled)
{
    const char *fragSrc = R"(#version 100

precision lowp float;

uniform float nondeterministic_uniform;

float nonConstInitializer();

float nonConstGlobal = nonConstInitializer();
float sideEffectGlobal = 0.0;

float nonConstInitializer() {
    sideEffectGlobal = 1.0;
    return nondeterministic_uniform;
}

void main()
{
    gl_FragColor = vec4(nondeterministic_uniform, nonConstGlobal, sideEffectGlobal, 1.0);
}
)";

    GLShader badFragment(GL_FRAGMENT_SHADER);
    glShaderSource(badFragment, 1, &fragSrc, nullptr);
    glCompileShader(badFragment);

    GLint compileResult;
    glGetShaderiv(badFragment, GL_COMPILE_STATUS, &compileResult);
    EXPECT_EQ(compileResult, 0);

    EXPECT_GL_NO_ERROR();
}

// Test that non constant initializers are evaluated correctly in ESSL 100
TEST_P(ShaderNonConstGlobalInitializerTest, v100)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_non_constant_global_initializers"));

    const char *fragSrc = R"(#version 100
#extension GL_EXT_shader_non_constant_global_initializers : require

precision lowp float;

#ifndef GL_EXT_shader_non_constant_global_initializers
#error GL_EXT_shader_non_constant_global_initializers is not defined
#endif

uniform float nondeterministic_uniform;

float nonConstInitializer();

float nonConstGlobal = nonConstInitializer();
float sideEffectGlobal = 0.0;

float nonConstInitializer() {
    sideEffectGlobal = 1.0;
    return nondeterministic_uniform;
}

void main()
{
    gl_FragColor = vec4(nondeterministic_uniform, nonConstGlobal, sideEffectGlobal, 1.0);
}
)";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), fragSrc);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, "nondeterministic_uniform"), 1.0f);
    ASSERT_GL_NO_ERROR();

    draw(program);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(64, 64, GLColor::white);
}

// Test that non constant initializers are evaluated correctly in ESSL 300
TEST_P(ShaderNonConstGlobalInitializerTest, v300es)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_non_constant_global_initializers"));

    const char *fragSrc = R"(#version 300 es
#extension GL_EXT_shader_non_constant_global_initializers : require

precision highp float;
out vec4 fragColor;

#ifndef GL_EXT_shader_non_constant_global_initializers
#error GL_EXT_shader_non_constant_global_initializers is not defined
#endif

uniform float nondeterministic_uniform;

float nonConstInitializer();

float nonConstGlobal = nonConstInitializer();
float sideEffectGlobal = 0.0;

float nonConstInitializer() {
    sideEffectGlobal = 1.0;
    return nondeterministic_uniform;
}

void main()
{
    fragColor = vec4(nondeterministic_uniform, nonConstGlobal, sideEffectGlobal, 1.0);
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), fragSrc);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, "nondeterministic_uniform"), 1.0f);
    ASSERT_GL_NO_ERROR();

    draw(program);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(64, 64, GLColor::white);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ShaderNonConstGlobalInitializerTest);
ANGLE_INSTANTIATE_TEST_ES3(ShaderNonConstGlobalInitializerTest);