//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MatrixTest:
//   Test various shader matrix variable declarations.

#include <vector>
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class MatrixTest31 : public ANGLETest<>
{
  protected:
    MatrixTest31()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

TEST_P(MatrixTest31, Mat3Varying)
{
    constexpr char kVS[] = R"(#version 310 es
precision mediump float;

in vec4 a_position;

layout(location=0) out mat3 matrix;
layout(location=3) out vec3 vector;

void main()
{
    matrix = mat3(1.0);
    vector = vec3(1.0);
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

layout(location=0) in mat3 matrix;
layout(location=3) in vec3 vector;

out vec4 oColor;

void main()
{
    oColor = vec4(matrix[0], 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    drawQuad(program, "a_position", 0.5f);
    EXPECT_GL_NO_ERROR();
}

TEST_P(MatrixTest31, Mat3VaryingBadLocation)
{
    constexpr char kVS[] = R"(#version 310 es
precision mediump float;

in vec4 a_position;

layout(location=0) out mat3 matrix;
layout(location=2) out vec3 vector;

void main()
{
    matrix = mat3(1.0);
    vector = vec3(1.0);
    gl_Position = a_position;
})";

    GLProgram program;

    GLuint vs                    = glCreateShader(GL_VERTEX_SHADER);
    const char *sourceVsArray[1] = {kVS};
    glShaderSource(vs, 1, sourceVsArray, nullptr);
    glCompileShader(vs);
    GLint compileResult;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compileResult);
    EXPECT_GL_FALSE(compileResult);
}

TEST_P(MatrixTest31, Mat3x4Varying)
{
    constexpr char kVS[] = R"(#version 310 es
precision mediump float;

in vec4 a_position;

layout(location=0) out mat3x4 matrix;
layout(location=3) out vec3 vector;

void main()
{
    matrix = mat3x4(1.0);
    vector = vec3(1.0);
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

layout(location=0) in mat3x4 matrix;
layout(location=3) in vec3 vector;

out vec4 oColor;

void main()
{
    oColor = vec4(matrix[0]);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    drawQuad(program, "a_position", 0.5f);
    EXPECT_GL_NO_ERROR();
}

TEST_P(MatrixTest31, Mat3x4VaryingBadLocation)
{
    constexpr char kVS[] = R"(#version 310 es
precision mediump float;

in vec4 a_position;

layout(location=0) out mat3x4 matrix;
layout(location=2) out vec3 vector;

void main()
{
    matrix = mat3x4(1.0);
    vector = vec3(1.0);
    gl_Position = a_position;
})";

    GLProgram program;

    GLuint vs                    = glCreateShader(GL_VERTEX_SHADER);
    const char *sourceVsArray[1] = {kVS};
    glShaderSource(vs, 1, sourceVsArray, nullptr);
    glCompileShader(vs);
    GLint compileResult;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compileResult);
    EXPECT_GL_FALSE(compileResult);
}

TEST_P(MatrixTest31, Mat3x4ArrayVarying)
{
    constexpr char kVS[] = R"(#version 310 es
precision mediump float;

in vec4 a_position;

layout(location=0) out mat3x4[2] matrix;
layout(location=6) out vec3 vector;

void main()
{
    matrix[0] = mat3x4(1.0);
    matrix[1] = mat3x4(1.0);
    vector = vec3(1.0);
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

layout(location=0) in mat3x4[2] matrix;
layout(location=6) in vec3 vector;

out vec4 oColor;

void main()
{
    oColor = vec4(matrix[0][0]);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    drawQuad(program, "a_position", 0.5f);
    EXPECT_GL_NO_ERROR();
}

TEST_P(MatrixTest31, Mat3x4ArrayVaryingBadLocation)
{
    constexpr char kVS[] = R"(#version 310 es
precision mediump float;

in vec4 a_position;

layout(location=0) out mat3x4[2] matrix;
layout(location=5) out vec3 vector;

void main()
{
    matrix[0] = mat3x4(1.0);
    matrix[1] = mat3x4(1.0);
    vector = vec3(1.0);
    gl_Position = a_position;
})";

    GLProgram program;

    GLuint vs                    = glCreateShader(GL_VERTEX_SHADER);
    const char *sourceVsArray[1] = {kVS};
    glShaderSource(vs, 1, sourceVsArray, nullptr);
    glCompileShader(vs);
    GLint compileResult;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compileResult);
    EXPECT_GL_FALSE(compileResult);
}

TEST_P(MatrixTest31, Mat3x4StructVarying)
{
    constexpr char kVS[] = R"(#version 310 es
precision mediump float;

in vec4 a_position;

struct S
{
    mat3x4 m;
};
layout(location=0) out S matrix;
layout(location=3) out vec3 vector;

void main()
{
    matrix.m = mat3x4(1.0);
    vector = vec3(1.0);
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

struct S
{
    mat3x4 m;
};
layout(location=0) in S matrix;
layout(location=3) in vec3 vector;

out vec4 oColor;

void main()
{
    oColor = vec4(matrix.m[0]);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    drawQuad(program, "a_position", 0.5f);
    EXPECT_GL_NO_ERROR();
}

TEST_P(MatrixTest31, Mat3x4StructVaryingBadLocation)
{
    constexpr char kVS[] = R"(#version 310 es
precision mediump float;

in vec4 a_position;

struct S
{
    mat3x4 m;
};
layout(location=0) out S matrix;
layout(location=2) out vec3 vector;

void main()
{
    matrix.m = mat3x4(1.0);
    vector = vec3(1.0);
    gl_Position = a_position;
})";

    GLProgram program;

    GLuint vs                    = glCreateShader(GL_VERTEX_SHADER);
    const char *sourceVsArray[1] = {kVS};
    glShaderSource(vs, 1, sourceVsArray, nullptr);
    glCompileShader(vs);
    GLint compileResult;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compileResult);
    EXPECT_GL_FALSE(compileResult);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MatrixTest31);
ANGLE_INSTANTIATE_TEST_ES31(MatrixTest31);

}  // namespace
