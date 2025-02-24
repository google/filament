//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class CubeMapTextureTest : public ANGLETest<>
{
  protected:
    CubeMapTextureTest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        mProgram = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mColorLocation = glGetUniformLocation(mProgram, essl1_shaders::ColorUniform());

        glUseProgram(mProgram);

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    void runSampleCoordinateTransformTest(const char *shader, const bool useES3);

    GLuint mProgram;
    GLint mColorLocation;
};

// Verify that rendering to the faces of a cube map consecutively will correctly render to each
// face.
TEST_P(CubeMapTextureTest, RenderToFacesConsecutively)
{
    // TODO: Diagnose and fix. http://anglebug.com/42261648
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsIntel() && IsWindows());

    // http://anglebug.com/42261821
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsIntel() && IsFuchsia());

    const GLfloat faceColors[] = {
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    };

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    for (GLenum face = 0; face < 6; face++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, 1, 1, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
    }
    EXPECT_GL_NO_ERROR();

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    for (GLenum face = 0; face < 6; face++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, tex, 0);
        EXPECT_GL_NO_ERROR();

        glUseProgram(mProgram);

        const GLfloat *faceColor = faceColors + (face * 4);
        glUniform4f(mColorLocation, faceColor[0], faceColor[1], faceColor[2], faceColor[3]);

        drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_GL_NO_ERROR();
    }

    for (GLenum face = 0; face < 6; face++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, tex, 0);
        EXPECT_GL_NO_ERROR();

        const GLfloat *faceColor = faceColors + (face * 4);
        EXPECT_PIXEL_EQ(0, 0, faceColor[0] * 255, faceColor[1] * 255, faceColor[2] * 255,
                        faceColor[3] * 255);
        EXPECT_GL_NO_ERROR();
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);

    EXPECT_GL_NO_ERROR();
}

void CubeMapTextureTest::runSampleCoordinateTransformTest(const char *shader, const bool useES3)
{
    // Fails to compile the shader.  anglebug.com/42262420
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsWindows());

    constexpr GLsizei kCubeFaceCount            = 6;
    constexpr GLsizei kCubeFaceSectionCount     = 4;
    constexpr GLsizei kCubeFaceSectionCountSqrt = 2;

    constexpr GLColor faceColors[kCubeFaceCount][kCubeFaceSectionCount] = {
        {GLColor(255, 0, 0, 255), GLColor(191, 0, 0, 255), GLColor(127, 0, 0, 255),
         GLColor(63, 0, 0, 255)},
        {GLColor(0, 255, 0, 255), GLColor(0, 191, 0, 255), GLColor(0, 127, 0, 255),
         GLColor(0, 63, 0, 255)},
        {GLColor(0, 0, 255, 255), GLColor(0, 0, 191, 255), GLColor(0, 0, 127, 255),
         GLColor(0, 0, 63, 255)},
        {GLColor(255, 63, 0, 255), GLColor(191, 127, 0, 255), GLColor(127, 191, 0, 255),
         GLColor(63, 255, 0, 255)},
        {GLColor(0, 255, 63, 255), GLColor(0, 191, 127, 255), GLColor(0, 127, 191, 255),
         GLColor(0, 63, 255, 255)},
        {GLColor(63, 0, 255, 255), GLColor(127, 0, 191, 255), GLColor(191, 0, 127, 255),
         GLColor(255, 0, 63, 255)},
    };

    constexpr GLsizei kTextureSize = 32;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    for (GLenum face = 0; face < kCubeFaceCount; face++)
    {
        std::vector<GLColor> faceData(kTextureSize * kTextureSize);

        // Create the face with four sections, each with a solid color from |faceColors|.
        for (size_t row = 0; row < kTextureSize / kCubeFaceSectionCountSqrt; ++row)
        {
            for (size_t col = 0; col < kTextureSize / kCubeFaceSectionCountSqrt; ++col)
            {
                for (size_t srow = 0; srow < kCubeFaceSectionCountSqrt; ++srow)
                {
                    for (size_t scol = 0; scol < kCubeFaceSectionCountSqrt; ++scol)
                    {
                        size_t r = row + srow * kTextureSize / kCubeFaceSectionCountSqrt;
                        size_t c = col + scol * kTextureSize / kCubeFaceSectionCountSqrt;
                        size_t s = srow * kCubeFaceSectionCountSqrt + scol;
                        faceData[r * kTextureSize + c] = faceColors[face][s];
                    }
                }
            }
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, kTextureSize, kTextureSize,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, faceData.data());
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    GLTexture fboTex;
    glBindTexture(GL_TEXTURE_2D, fboTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kCubeFaceCount, kCubeFaceSectionCount, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(program, useES3 ? essl3_shaders::vs::Simple() : essl1_shaders::vs::Simple(),
                     shader);
    glUseProgram(program);

    GLint texCubeLocation = glGetUniformLocation(program, "texCube");
    ASSERT_NE(-1, texCubeLocation);
    glUniform1i(texCubeLocation, 0);

    drawQuad(program, useES3 ? essl3_shaders::PositionAttrib() : essl1_shaders::PositionAttrib(),
             0.5f);
    EXPECT_GL_NO_ERROR();

    for (GLenum face = 0; face < kCubeFaceCount; face++)
    {
        // The following table defines the translation from textureCube coordinates to coordinates
        // in each face.  The framebuffer has width 6 and height 4.  Every column corresponding to
        // an x value represents one cube face.  The values in rows are samples from the four
        // sections of the face.
        //
        // Major    Axis Direction Target    sc  tc  ma
        //  +rx  TEXTURE_CUBE_MAP_POSITIVE_X −rz −ry rx
        //  −rx  TEXTURE_CUBE_MAP_NEGATIVE_X  rz −ry rx
        //  +ry  TEXTURE_CUBE_MAP_POSITIVE_Y  rx  rz ry
        //  −ry  TEXTURE_CUBE_MAP_NEGATIVE_Y  rx −rz ry
        //  +rz  TEXTURE_CUBE_MAP_POSITIVE_Z  rx −ry rz
        //  −rz  TEXTURE_CUBE_MAP_NEGATIVE_Z −rx −ry rz
        //
        // This table is used only to determine the direction of growth for s and t.  The shader
        // always generates (row,col) coordinates (0, 0), (0, 1), (1, 0), (1, 1) which is the order
        // the data is uploaded to the faces, but based on the table above, the sample order would
        // be different.
        constexpr size_t faceSampledSections[kCubeFaceCount][kCubeFaceSectionCount] = {
            {3, 2, 1, 0}, {2, 3, 0, 1}, {0, 1, 2, 3}, {2, 3, 0, 1}, {2, 3, 0, 1}, {3, 2, 1, 0},
        };

        for (size_t section = 0; section < kCubeFaceSectionCount; ++section)
        {
            const GLColor sectionColor = faceColors[face][faceSampledSections[face][section]];

            EXPECT_PIXEL_COLOR_EQ(face, section, sectionColor)
                << "face " << face << ", section " << section;
        }
    }
    EXPECT_GL_NO_ERROR();
}

// Verify that cube map sampling follows the rules that map cubemap coordinates to coordinates
// within each face.  See section 3.7.5 of GLES2.0 (Cube Map Texture Selection).
TEST_P(CubeMapTextureTest, SampleCoordinateTransform)
{
    // http://anglebug.com/40096654
    ANGLE_SKIP_TEST_IF(IsWindows() && IsD3D9());
    // Create a program that samples from 6x4 directions of the cubemap, draw and verify that the
    // colors match the right color from |faceColors|.
    constexpr char kFS[] = R"(precision mediump float;

uniform samplerCube texCube;

const mat4 coordInSection = mat4(
    vec4(-0.5, -0.5, 0, 0),
    vec4( 0.5, -0.5, 0, 0),
    vec4(-0.5,  0.5, 0, 0),
    vec4( 0.5,  0.5, 0, 0)
);

void main()
{
    vec3 coord;
    if (gl_FragCoord.x < 2.0)
    {
        coord.x = gl_FragCoord.x < 1.0 ? 1.0 : -1.0;
        coord.zy = coordInSection[int(gl_FragCoord.y)].xy;
    }
    else if (gl_FragCoord.x < 4.0)
    {
        coord.y = gl_FragCoord.x < 3.0 ? 1.0 : -1.0;
        coord.xz = coordInSection[int(gl_FragCoord.y)].xy;
    }
    else
    {
        coord.z = gl_FragCoord.x < 5.0 ? 1.0 : -1.0;
        coord.xy = coordInSection[int(gl_FragCoord.y)].xy;
    }

    gl_FragColor = textureCube(texCube, coord);
})";

    runSampleCoordinateTransformTest(kFS, false);
}

// On Android Vulkan, unequal x and y derivatives cause this test to fail.
TEST_P(CubeMapTextureTest, SampleCoordinateTransformGrad)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_texture_lod"));

    constexpr char kFS[] = R"(#extension GL_EXT_shader_texture_lod : require
precision mediump float;

uniform samplerCube texCube;

const mat4 coordInSection = mat4(
    vec4(-0.5, -0.5, 0, 0),
    vec4( 0.5, -0.5, 0, 0),
    vec4(-0.5,  0.5, 0, 0),
    vec4( 0.5,  0.5, 0, 0)
);

void main()
{
    vec3 coord;
    if (gl_FragCoord.x < 2.0)
    {
        coord.x = gl_FragCoord.x < 1.0 ? 1.0 : -1.0;
        coord.zy = coordInSection[int(gl_FragCoord.y)].xy;
    }
    else if (gl_FragCoord.x < 4.0)
    {
        coord.y = gl_FragCoord.x < 3.0 ? 1.0 : -1.0;
        coord.xz = coordInSection[int(gl_FragCoord.y)].xy;
    }
    else
    {
        coord.z = gl_FragCoord.x < 5.0 ? 1.0 : -1.0;
        coord.xy = coordInSection[int(gl_FragCoord.y)].xy;
    }

    gl_FragColor = textureCubeGradEXT(texCube, coord,
                                      vec3(10.0, 10.0, 0.0), vec3(0.0, 10.0, 10.0));
})";

    runSampleCoordinateTransformTest(kFS, false);
}

// Same as the previous but uses the ES 3.0 explicit gradient function.
TEST_P(CubeMapTextureTest, SampleCoordinateTransformGrad_ES3)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;

uniform samplerCube texCube;
out vec4 my_FragColor;

const mat4 coordInSection = mat4(
    vec4(-0.5, -0.5, 0, 0),
    vec4( 0.5, -0.5, 0, 0),
    vec4(-0.5,  0.5, 0, 0),
    vec4( 0.5,  0.5, 0, 0)
);

void main()
{
    vec3 coord;
    if (gl_FragCoord.x < 2.0)
    {
        coord.x = gl_FragCoord.x < 1.0 ? 1.0 : -1.0;
        coord.zy = coordInSection[int(gl_FragCoord.y)].xy;
    }
    else if (gl_FragCoord.x < 4.0)
    {
        coord.y = gl_FragCoord.x < 3.0 ? 1.0 : -1.0;
        coord.xz = coordInSection[int(gl_FragCoord.y)].xy;
    }
    else
    {
        coord.z = gl_FragCoord.x < 5.0 ? 1.0 : -1.0;
        coord.xy = coordInSection[int(gl_FragCoord.y)].xy;
    }

    my_FragColor = textureGrad(texCube, coord,
                               vec3(10.0, 10.0, 0.0), vec3(0.0, 10.0, 10.0));
})";

    runSampleCoordinateTransformTest(kFS, true);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(CubeMapTextureTest);
