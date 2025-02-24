//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// LinkAndRelinkFailureTest:
//   Link and relink failure tests for rendering pipeline and compute pipeline.

#include <vector>
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class LinkAndRelinkTest : public ANGLETest<>
{
  protected:
    LinkAndRelinkTest() {}
};

class LinkAndRelinkTestES3 : public ANGLETest<>
{
  protected:
    LinkAndRelinkTestES3() {}
};

class LinkAndRelinkTestES31 : public ANGLETest<>
{
  protected:
    LinkAndRelinkTestES31() {}
};

// Test destruction of a context with a pending relink of the current in-use
// program.
TEST_P(LinkAndRelinkTest, DestructionWithPendingRelink)
{
    constexpr char kVS[] = "void main() {}";
    constexpr char kFS[] = "void main() {}";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glUseProgram(program);
    glLinkProgram(program);
    EXPECT_GL_NO_ERROR();
}

// When a program link or relink fails, if you try to install the unsuccessfully
// linked program (via UseProgram) and start rendering or dispatch compute,
// We can not always report INVALID_OPERATION for rendering/compute pipeline.
// The result depends on the previous state: Whether a valid program is
// installed in current GL state before the link.
// If a program successfully relinks when it is in use, the program might
// change from a rendering program to a compute program in theory,
// or vice versa.

// When program link fails and no valid rendering program is installed in the GL
// state before the link, it should report an error for UseProgram
TEST_P(LinkAndRelinkTest, RenderingProgramFailsWithoutProgramInstalled)
{
    glUseProgram(0);
    GLuint program = glCreateProgram();

    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_FALSE(linkStatus);

    glUseProgram(program);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();
}

// When program link or relink fails and a valid rendering program is installed
// in the GL state before the link, using the failed program via UseProgram
// should report an error, but starting rendering should succeed.
// However, dispatching compute always fails.
TEST_P(LinkAndRelinkTest, RenderingProgramFailsWithProgramInstalled)
{
    // Install a render program in current GL state via UseProgram, then render.
    // It should succeed.
    constexpr char kVS[] = "void main() {}";
    constexpr char kFS[] = "void main() {}";

    GLuint program = glCreateProgram();

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);

    EXPECT_NE(0u, vs);
    EXPECT_NE(0u, fs);

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_TRUE(linkStatus);

    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(8, 4, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Link failure, and a valid program has been installed in the GL state.
    GLuint programNull = glCreateProgram();

    glLinkProgram(programNull);
    glGetProgramiv(programNull, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_FALSE(linkStatus);

    // Starting rendering should succeed.
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(8, 4, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Using the unsuccessfully linked program should report an error.
    glUseProgram(programNull);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Using the unsuccessfully linked program, that program should not
    // replace the program binary residing in the GL state. It will not make
    // the installed program invalid either, like what UseProgram(0) can do.
    // So, starting rendering should succeed.
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(8, 4, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // We try to relink the installed program, but make it fail.

    // No vertex shader, relink fails.
    glDetachShader(program, vs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_FALSE(linkStatus);
    EXPECT_GL_NO_ERROR();

    // Starting rendering should succeed.
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(8, 4, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Using the unsuccessfully relinked program should report an error.
    glUseProgram(program);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Using the unsuccessfully relinked program, that program should not
    // replace the program binary residing in the GL state. It will not make
    // the installed program invalid either, like what UseProgram(0) can do.
    // So, starting rendering should succeed.
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(8, 4, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Tests uniform default values.
TEST_P(LinkAndRelinkTest, UniformDefaultValues)
{
    // TODO(anglebug.com/42262609): Understand why rectangle texture CLs made this fail.
    ANGLE_SKIP_TEST_IF(IsOzone() && IsIntel());
    constexpr char kFS[] = R"(precision mediump float;
uniform vec4 u_uniform;

bool isZero(vec4 value) {
    return value == vec4(0,0,0,0);
}

void main()
{
    gl_FragColor = isZero(u_uniform) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    GLint loc = glGetUniformLocation(program, "u_uniform");
    ASSERT_NE(-1, loc);
    glUniform4f(loc, 0.1f, 0.2f, 0.3f, 0.4f);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glLinkProgram(program);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// When program link fails and no valid compute program is installed in the GL
// state before the link, it should report an error for UseProgram and
// DispatchCompute.
TEST_P(LinkAndRelinkTestES31, ComputeProgramFailsWithoutProgramInstalled)
{
    glUseProgram(0);
    GLuint program = glCreateProgram();

    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_FALSE(linkStatus);

    glUseProgram(program);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glDispatchCompute(8, 4, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// When program link or relink fails and a valid compute program is installed in
// the GL state before the link, using the failed program via UseProgram should
// report an error, but dispatching compute should succeed.
TEST_P(LinkAndRelinkTestES31, ComputeProgramFailsWithProgramInstalled)
{
    // Install a compute program in the GL state via UseProgram, then dispatch
    // compute. It should succeed.
    constexpr char kCS[] =
        R"(#version 310 es
        layout(local_size_x=1) in;
        void main()
        {
        })";

    GLuint program = glCreateProgram();

    GLuint cs = CompileShader(GL_COMPUTE_SHADER, kCS);
    EXPECT_NE(0u, cs);

    glAttachShader(program, cs);
    glDeleteShader(cs);

    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_TRUE(linkStatus);

    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    EXPECT_GL_NO_ERROR();
    glDispatchCompute(8, 4, 2);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    // Link failure, and a valid program has been installed in the GL state.
    GLuint programNull = glCreateProgram();

    glLinkProgram(programNull);
    glGetProgramiv(programNull, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_FALSE(linkStatus);

    // Dispatching compute should succeed.
    glDispatchCompute(8, 4, 2);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    // Using the unsuccessfully linked program should report an error.
    glUseProgram(programNull);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Using the unsuccessfully linked program, that program should not
    // replace the program binary residing in the GL state. It will not make
    // the installed program invalid either, like what UseProgram(0) can do.
    // So, dispatching compute should succeed.
    glDispatchCompute(8, 4, 2);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    // We try to relink the installed program, but make it fail.

    // No compute shader, relink fails.
    glDetachShader(program, cs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_FALSE(linkStatus);
    EXPECT_GL_NO_ERROR();

    // Dispatching compute should succeed.
    glDispatchCompute(8, 4, 2);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    // Using the unsuccessfully relinked program should report an error.
    glUseProgram(program);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Using the unsuccessfully relinked program, that program should not
    // replace the program binary residing in the GL state. It will not make
    // the installed program invalid either, like what UseProgram(0) can do.
    // So, dispatching compute should succeed.
    glDispatchCompute(8, 4, 2);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();
}

// If you compile and link a compute program successfully and use the program,
// then dispatching compute and rendering can succeed (with undefined behavior).
// If you relink the compute program to a rendering program when it is in use,
// then dispatching compute will fail, but starting rendering can succeed.
TEST_P(LinkAndRelinkTestES31, RelinkProgramSucceedsFromComputeToRendering)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1) in;
void main()
{
})";

    GLuint program = glCreateProgram();

    GLuint cs = CompileShader(GL_COMPUTE_SHADER, kCS);
    EXPECT_NE(0u, cs);

    glAttachShader(program, cs);
    glDeleteShader(cs);

    glLinkProgram(program);
    glDetachShader(program, cs);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_TRUE(linkStatus);

    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    EXPECT_GL_NO_ERROR();
    glDispatchCompute(8, 4, 2);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    constexpr char kVS[] = "void main() {}";
    constexpr char kFS[] = "void main() {}";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_NE(0u, vs);
    EXPECT_NE(0u, fs);

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    glLinkProgram(program);
    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_TRUE(linkStatus);

    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(8, 4, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// If you compile and link a rendering program successfully and use the program,
// then starting rendering can succeed, while dispatching compute will fail.
// If you relink the rendering program to a compute program when it is in use,
// then starting rendering will fail, but dispatching compute can succeed.
TEST_P(LinkAndRelinkTestES31, RelinkProgramSucceedsFromRenderingToCompute)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    constexpr char kVS[] = "void main() {}";
    constexpr char kFS[] = "void main() {}";

    GLuint program = glCreateProgram();

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);

    EXPECT_NE(0u, vs);
    EXPECT_NE(0u, fs);

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    glLinkProgram(program);
    glDetachShader(program, vs);
    glDetachShader(program, fs);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_TRUE(linkStatus);

    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(8, 4, 2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1) in;
void main()
{
})";

    GLuint cs = CompileShader(GL_COMPUTE_SHADER, kCS);
    EXPECT_NE(0u, cs);

    glAttachShader(program, cs);
    glDeleteShader(cs);

    glLinkProgram(program);
    glDetachShader(program, cs);
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_TRUE(linkStatus);

    EXPECT_GL_NO_ERROR();

    glDispatchCompute(8, 4, 2);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();
}

// Parallel link should continue unscathed even if the attached shaders to the program are modified.
TEST_P(LinkAndRelinkTestES31, ReattachShadersWhileParallelLinking)
{
    constexpr char kVS[]      = R"(#version 300 es
void main()
{
    vec2 position = vec2(-1, -1);
    if (gl_VertexID == 1)
        position = vec2(3, -1);
    else if (gl_VertexID == 2)
        position = vec2(-1, 3);
    gl_Position = vec4(position, 0, 1);
})";
    constexpr char kFSGreen[] = R"(#version 300 es
out mediump vec4 color;
void main()
{
    color = vec4(0, 1, 0, 1);
})";
    constexpr char kFSRed[]   = R"(#version 300 es
out mediump vec4 color;
void main()
{
    color = vec4(1, 0, 0, 1);
})";

    GLuint program = glCreateProgram();

    GLuint vs    = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint green = CompileShader(GL_FRAGMENT_SHADER, kFSGreen);
    GLuint red   = CompileShader(GL_FRAGMENT_SHADER, kFSRed);

    EXPECT_NE(0u, vs);
    EXPECT_NE(0u, green);
    EXPECT_NE(0u, red);

    glAttachShader(program, vs);
    glAttachShader(program, green);
    glLinkProgram(program);
    ASSERT_GL_NO_ERROR();

    // Immediately reattach another shader
    glDetachShader(program, green);
    glAttachShader(program, red);
    ASSERT_GL_NO_ERROR();

    // Make sure the linked program draws with green
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();

    glDeleteShader(vs);
    glDeleteShader(green);
    glDeleteShader(red);
    ASSERT_GL_NO_ERROR();
}

// Parallel link should continue unscathed even if new shaders are attached to the program.
TEST_P(LinkAndRelinkTestES31, AttachNewShadersWhileParallelLinking)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
void main()
{
    vec2 position = vec2(-1, -1);
    if (gl_VertexID == 1)
        position = vec2(3, -1);
    else if (gl_VertexID == 2)
        position = vec2(-1, 3);
    gl_Position = vec4(position, 0, 1);
})";
    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
out mediump vec4 color;
void main()
{
    color = vec4(0, 1, 0, 1);
})";
    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout (invocations = 3, triangles) in;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
})";

    GLuint program = glCreateProgram();

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);
    GLuint gs = CompileShader(GL_GEOMETRY_SHADER, kGS);

    EXPECT_NE(0u, vs);
    EXPECT_NE(0u, fs);
    EXPECT_NE(0u, gs);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    ASSERT_GL_NO_ERROR();

    // Immediately attach another shader
    glAttachShader(program, gs);
    ASSERT_GL_NO_ERROR();

    // Make sure the linked program draws with green
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();

    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteShader(gs);
    ASSERT_GL_NO_ERROR();
}

// Make sure the shader can be compiled in between attach and link
TEST_P(LinkAndRelinkTest, AttachShaderThenCompile)
{
    GLuint program = glCreateProgram();

    GLShader vs(GL_VERTEX_SHADER);
    GLShader fs(GL_FRAGMENT_SHADER);

    // Attach the shaders to the program first.  This makes sure the program doesn't prematurely
    // attempt to look into the shader's compilation result.
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    // Compile the shaders after that.
    const char *kVS = essl1_shaders::vs::Simple();
    const char *kFS = essl1_shaders::fs::Green();
    glShaderSource(vs, 1, &kVS, nullptr);
    glShaderSource(fs, 1, &kFS, nullptr);
    EXPECT_GL_NO_ERROR();

    glCompileShader(vs);
    glCompileShader(fs);

    // Then link
    glLinkProgram(program);
    ASSERT_GL_NO_ERROR();

    // Make sure it works
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();

    glDeleteProgram(program);
    ASSERT_GL_NO_ERROR();
}

// If a program is linked successfully once, it should retain its executable if a relink fails.
TEST_P(LinkAndRelinkTestES3, SuccessfulLinkThenFailingRelink)
{
    // Install a render program in current GL state via UseProgram, then render.
    // It should succeed.
    constexpr char kVS[]    = R"(#version 300 es
out vec4 color;
void main()
{
    vec2 position = vec2(-1, -1);
    if (gl_VertexID == 1)
        position = vec2(3, -1);
    else if (gl_VertexID == 2)
        position = vec2(-1, 3);

    gl_Position = vec4(position, 0, 1);
    color = vec4(0, 1, 0, 1);
})";
    constexpr char kBadFS[] = R"(#version 300 es
flat in uvec2 color;
out mediump vec4 colorOut;
void main()
{
    colorOut = vec4(1, color, 1);
})";

    GLuint program = glCreateProgram();
    GLuint vs      = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs      = CompileShader(GL_FRAGMENT_SHADER, essl3_shaders::fs::Green());
    GLuint badfs   = CompileShader(GL_FRAGMENT_SHADER, kBadFS);

    EXPECT_NE(0u, vs);
    EXPECT_NE(0u, fs);
    EXPECT_NE(0u, badfs);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_TRUE(linkStatus);

    EXPECT_GL_NO_ERROR();

    const int w = getWindowWidth();
    const int h = getWindowHeight();

    glViewport(0, 0, w, h);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, w / 2, h / 2);

    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_GL_NO_ERROR();

    // Cause the program to fail linking
    glDetachShader(program, fs);
    glAttachShader(program, badfs);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_FALSE(linkStatus);

    // Link failed, but the program should still be usable.
    glScissor(w / 2, h / 2, w / 2, h / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, w / 2, h / 2, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(w / 2, 0, w / 2, h / 2, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(0, h / 2, w / 2, h / 2, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(w / 2, h / 2, w / 2, h / 2, GLColor::green);

    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteShader(badfs);
    glDeleteProgram(program);
}

// Similar to SuccessfulLinkThenFailingRelink, but with a more complicated mix of resources.
TEST_P(LinkAndRelinkTestES31, SuccessfulLinkThenFailingRelink2)
{
    GLint maxFragmentShaderStorageBlocks;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);

    ANGLE_SKIP_TEST_IF(maxFragmentShaderStorageBlocks < 1);

    // Install a render program in current GL state via UseProgram, then render.
    // It should succeed.
    constexpr char kVS[]     = R"(#version 310 es
out vec4 color;
void main()
{
    vec2 position = vec2(-1, -1);
    if (gl_VertexID == 1)
        position = vec2(3, -1);
    else if (gl_VertexID == 2)
        position = vec2(-1, 3);

    gl_Position = vec4(position, 0, 1);
    color = vec4(0, 1, 0, 1);
})";
    constexpr char kGoodFS[] = R"(#version 310 es
in mediump vec4 color;
out mediump vec4 colorOut;
mediump uniform float u;  // should be 0.5;
uniform UBO
{
    highp float b;  // should be 1.75
};
layout(std140, binding = 1) buffer SSBO
{
    uint s;  // should be 0x12345678
};
void main()
{
    if (abs(u - 0.5) > 0.01)
        colorOut = vec4(1, 0, 0, 1);
    else if (abs(b - 1.75) > 0.01)
        colorOut = vec4(0, 0, 1, 1);
    else if (s != 0x12345678u)
        colorOut = vec4(1, 0, 1, 1);
    else
        colorOut = color;
})";
    constexpr char kBadFS[]  = R"(#version 310 es
flat in uvec2 color;
layout(location = 0) out mediump vec4 colorOut;
layout(location = 1) out mediump vec4 colorOut2;
void main()
{
    colorOut = vec4(1, color, 1);
    colorOut2 = vec4(color, 0, 1);
})";

    GLuint program = glCreateProgram();
    GLuint vs      = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs      = CompileShader(GL_FRAGMENT_SHADER, kGoodFS);
    GLuint badfs   = CompileShader(GL_FRAGMENT_SHADER, kBadFS);

    EXPECT_NE(0u, vs);
    EXPECT_NE(0u, fs);
    EXPECT_NE(0u, badfs);

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_TRUE(linkStatus);

    EXPECT_GL_NO_ERROR();

    constexpr float kUBOValue = 1.75;
    GLBuffer ubo;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(kUBOValue), &kUBOValue, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

    constexpr uint32_t kSSBOValue = 0x12345678;
    GLBuffer ssbo;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kSSBOValue), &kSSBOValue, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);

    EXPECT_GL_NO_ERROR();

    const int w = getWindowWidth();
    const int h = getWindowHeight();

    glViewport(0, 0, w, h);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, w / 2, h / 2);

    glUseProgram(program);

    const GLint uniLoc = glGetUniformLocation(program, "u");
    ASSERT_NE(uniLoc, -1);
    glUniform1f(uniLoc, 0.5);

    const GLint uboIndex = glGetUniformBlockIndex(program, "UBO");
    ASSERT_NE(uboIndex, -1);
    glUniformBlockBinding(program, uboIndex, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_GL_NO_ERROR();

    // Cause the program to fail linking
    glDetachShader(program, fs);
    glAttachShader(program, badfs);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_FALSE(linkStatus);

    // Link failed, but the program should still be usable.
    glScissor(w / 2, h / 2, w / 2, h / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, w / 2, h / 2, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(w / 2, 0, w / 2, h / 2, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(0, h / 2, w / 2, h / 2, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(w / 2, h / 2, w / 2, h / 2, GLColor::green);

    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteShader(badfs);
    glDeleteProgram(program);
}

// Same as SuccessfulLinkThenFailingRelink, but with PPOs.
TEST_P(LinkAndRelinkTestES31, SuccessfulLinkThenFailingRelinkWithPPO)
{
    // Only the Vulkan backend supports PPOs.
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    // Install a render program in current GL state via UseProgram, then render.
    // It should succeed.
    constexpr char kVS[]    = R"(#version 300 es
void main()
{
    vec2 position = vec2(-1, -1);
    if (gl_VertexID == 1)
        position = vec2(3, -1);
    else if (gl_VertexID == 2)
        position = vec2(-1, 3);

    gl_Position = vec4(position, 0, 1);
})";
    constexpr char kBadGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout (invocations = 3, triangles) in;
layout (triangle_strip, max_vertices = 3) out;
void main()
{
})";

    GLuint vs    = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs    = CompileShader(GL_FRAGMENT_SHADER, essl3_shaders::fs::Green());
    GLuint badgs = CompileShader(GL_GEOMETRY_SHADER, kBadGS);

    EXPECT_NE(0u, vs);
    EXPECT_NE(0u, fs);
    EXPECT_NE(0u, badgs);

    GLuint vsProg = glCreateProgram();
    GLuint fsProg = glCreateProgram();

    glProgramParameteri(vsProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(vsProg, vs);
    glLinkProgram(vsProg);
    EXPECT_GL_NO_ERROR();

    glProgramParameteri(fsProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(fsProg, fs);
    glLinkProgram(fsProg);
    EXPECT_GL_NO_ERROR();

    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vsProg);
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fsProg);
    EXPECT_GL_NO_ERROR();

    const int w = getWindowWidth();
    const int h = getWindowHeight();

    glViewport(0, 0, w, h);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, w / 2, h / 2);

    glUseProgram(0);
    glBindProgramPipeline(pipeline);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_GL_NO_ERROR();

    // Cause the fs program to fail linking
    glAttachShader(fsProg, badgs);
    glLinkProgram(fsProg);
    glUseProgramStages(pipeline, GL_GEOMETRY_SHADER_BIT, fsProg);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLint linkStatus;
    glGetProgramiv(fsProg, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_FALSE(linkStatus);

    // Program link failed, but the program pipeline should still be usable.
    glScissor(w / 2, h / 2, w / 2, h / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, w / 2, h / 2, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(w / 2, 0, w / 2, h / 2, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(0, h / 2, w / 2, h / 2, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(w / 2, h / 2, w / 2, h / 2, GLColor::green);

    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteShader(badgs);
    glDeleteProgram(vsProg);
    glDeleteProgram(fsProg);
    glDeleteProgramPipelines(1, &pipeline);
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(LinkAndRelinkTest);

ANGLE_INSTANTIATE_TEST_ES3(LinkAndRelinkTestES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(LinkAndRelinkTestES31);
ANGLE_INSTANTIATE_TEST_ES31(LinkAndRelinkTestES31);

}  // namespace
