//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Tests for shader interpolation qualifiers
//

#include "common/mathutil.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

constexpr int kPixelColorThreshhold = 8;

class ShaderInterpTest : public ANGLETest<>
{
  protected:
    ShaderInterpTest() : ANGLETest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
    }

    void draw(GLuint program, float skew)
    {
        glUseProgram(program);

        std::array<Vector4, 3> vertices;
        vertices[0] = {-1.0, -1.0, 0.0, 1.0};
        vertices[1] = {1.0, -1.0, 0.0, 1.0};
        vertices[2] = {0.0, 1.0 * skew, 0.0, skew};

        std::array<Vector4, 3> colors;
        colors[0] = {1.0, 0.0, 0.0, 1.0};
        colors[1] = {0.0, 1.0, 0.0, 1.0};
        colors[2] = {0.0, 0.0, 1.0, 1.0};

        GLint positionLocation = glGetAttribLocation(program, "position");
        GLint colorLocation    = glGetAttribLocation(program, "vertex_color");

        glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, vertices.data());
        glVertexAttribPointer(colorLocation, 4, GL_FLOAT, GL_FALSE, 0, colors.data());

        glEnableVertexAttribArray(positionLocation);
        glEnableVertexAttribArray(colorLocation);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
};

// Test that regular "smooth" interpolation works correctly
TEST_P(ShaderInterpTest, Smooth)
{
    const char *vertSrc = R"(#version 300 es
precision highp float;
in vec4 position;
in vec4 vertex_color;
smooth out vec4 interp_color;

void main()
{
    gl_Position = position;
    interp_color = vertex_color;
}
)";
    const char *fragSrc = R"(#version 300 es
precision highp float;
smooth in vec4 interp_color;
out vec4 fragColor;

void main()
{
    fragColor = interp_color;
}
)";

    ANGLE_GL_PROGRAM(program, vertSrc, fragSrc);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    draw(program, 1.0);
    EXPECT_PIXEL_COLOR_NEAR(64, 64, GLColor(62, 64, 128, 255), kPixelColorThreshhold);

    glClear(GL_COLOR_BUFFER_BIT);
    draw(program, 2.0);
    EXPECT_PIXEL_COLOR_NEAR(64, 64, GLColor(83, 86, 86, 255), kPixelColorThreshhold);
}

// Test that uninterpolated "Flat" interpolation works correctly
TEST_P(ShaderInterpTest, Flat)
{
    // TODO: anglebug.com/42262721
    // No vendors currently support VK_EXT_provoking_vertex, which is necessary for conformant flat
    // shading. SwiftShader does technically support this extension, but as it has not yet been
    // ratified by Khronos, the vulkan validation layers do not recognize the create info struct,
    // causing it to be stripped and thus causing the extension to behave as if it is disabled.
    ANGLE_SKIP_TEST_IF(IsVulkan());

    const char *vertSrc = R"(#version 300 es
precision highp float;
in vec4 position;
in vec4 vertex_color;
flat out vec4 interp_color;

void main()
{
    gl_Position = position;
    interp_color = vertex_color;
}
)";
    const char *fragSrc = R"(#version 300 es
precision highp float;
flat in vec4 interp_color;
out vec4 fragColor;

void main()
{
    fragColor = interp_color;
}
)";

    ANGLE_GL_PROGRAM(program, vertSrc, fragSrc);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    draw(program, 1.0);
    GLColor smooth_reference;
    glReadPixels(64, 64, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &smooth_reference);
    EXPECT_PIXEL_COLOR_EQ(64, 64, GLColor(0, 0, 255, 255));
}

// Test that "noperspective" interpolation correctly interpolates in screenspace
TEST_P(ShaderInterpTest, NoPerspective)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_NV_shader_noperspective_interpolation"));

    const char *vertSrcSmooth = R"(#version 300 es
precision highp float;
in vec4 position;
in vec4 vertex_color;
smooth out vec4 interp_color;

void main()
{
    gl_Position = position;
    interp_color = vertex_color;
}
)";
    const char *fragSrcSmooth = R"(#version 300 es
precision highp float;
smooth in vec4 interp_color;
out vec4 fragColor;

void main()
{
    fragColor = interp_color;
}
)";
    ANGLE_GL_PROGRAM(programSmooth, vertSrcSmooth, fragSrcSmooth);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    draw(programSmooth, 1.0);
    GLColor smooth_reference;
    glReadPixels(64, 64, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &smooth_reference);

    const char *vertSrcNoPerspective = R"(#version 300 es
#extension GL_NV_shader_noperspective_interpolation : require

#ifndef GL_NV_shader_noperspective_interpolation
#error GL_NV_shader_noperspective_interpolation is not defined
#endif

precision highp float;
in vec4 position;
in vec4 vertex_color;
noperspective out vec4 interp_color;

void main()
{
    gl_Position = position;
    interp_color = vertex_color;
}
)";
    const char *fragSrcNoPerspective = R"(#version 300 es
#extension GL_NV_shader_noperspective_interpolation : require

#ifndef GL_NV_shader_noperspective_interpolation
#error GL_NV_shader_noperspective_interpolation is not defined
#endif

precision highp float;
noperspective in vec4 interp_color;
out vec4 fragColor;

void main()
{
    fragColor = interp_color;
}
)";
    ANGLE_GL_PROGRAM(programNoPerspective, vertSrcNoPerspective, fragSrcNoPerspective);
    glClear(GL_COLOR_BUFFER_BIT);
    draw(programNoPerspective, 1.0);
    EXPECT_PIXEL_COLOR_EQ(64, 64, smooth_reference);

    glClear(GL_COLOR_BUFFER_BIT);
    draw(programNoPerspective, 2.0);
    EXPECT_PIXEL_COLOR_EQ(64, 64, smooth_reference);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ShaderInterpTest);
ANGLE_INSTANTIATE_TEST_ES3(ShaderInterpTest);