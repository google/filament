//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include <vector>
#include "test_utils/gl_raii.h"

using namespace angle;

class IncompleteTextureTest : public ANGLETest<>
{
  protected:
    IncompleteTextureTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        constexpr char kVS[] = R"(precision highp float;
attribute vec4 position;
varying vec2 texcoord;

void main()
{
    gl_Position = position;
    texcoord = (position.xy * 0.5) + 0.5;
})";

        constexpr char kFS[] = R"(precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

void main()
{
    gl_FragColor = texture2D(tex, texcoord);
})";

        mProgram = CompileProgram(kVS, kFS);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mProgram, "tex");
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    GLuint mProgram;
    GLint mTextureUniformLocation;
};

class IncompleteTextureTestES3 : public ANGLETest<>
{
  protected:
    IncompleteTextureTestES3()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

  public:
    void setupFramebuffer(const GLenum sizedInternalFormat)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, sizedInternalFormat, getWindowWidth(),
                              getWindowHeight());
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  mRenderbuffer);
        glViewport(0, 0, getWindowWidth(), getWindowHeight());
        ASSERT_GL_NO_ERROR();
    }

  private:
    GLRenderbuffer mRenderbuffer;
    GLFramebuffer mFramebuffer;
};

class IncompleteTextureTestES31 : public ANGLETest<>
{
  protected:
    IncompleteTextureTestES31()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Test rendering with an incomplete texture.
TEST_P(IncompleteTextureTest, IncompleteTexture2D)
{
    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glUseProgram(mProgram);
    glUniform1i(mTextureUniformLocation, 0);

    constexpr GLsizei kTextureSize = 2;
    std::vector<GLColor> textureData(kTextureSize * kTextureSize, GLColor::red);

    // Make a complete texture.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, textureData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Should be complete - expect red.
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red) << "complete texture should be red";

    // Make texture incomplete.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Should be incomplete - expect black.
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black) << "incomplete texture should be black";

    // Make texture complete by defining the second mip.
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, kTextureSize >> 1, kTextureSize >> 1, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, textureData.data());

    // Should be complete - expect red.
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red) << "mip-complete texture should be red";
}

// Tests redefining a texture with half the size works as expected.
TEST_P(IncompleteTextureTest, UpdateTexture)
{
    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glUseProgram(mProgram);
    glUniform1i(mTextureUniformLocation, 0);

    constexpr GLsizei redTextureSize = 64;
    std::vector<GLColor> redTextureData(redTextureSize * redTextureSize, GLColor::red);
    for (GLint mip = 0; mip < 7; ++mip)
    {
        const GLsizei mipSize = redTextureSize >> mip;

        glTexImage2D(GL_TEXTURE_2D, mip, GL_RGBA, mipSize, mipSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     redTextureData.data());
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    constexpr GLsizei greenTextureSize = 32;
    std::vector<GLColor> greenTextureData(greenTextureSize * greenTextureSize, GLColor::green);

    for (GLint mip = 0; mip < 6; ++mip)
    {
        const GLsizei mipSize = greenTextureSize >> mip;

        glTexSubImage2D(GL_TEXTURE_2D, mip, mipSize, mipSize, mipSize, mipSize, GL_RGBA,
                        GL_UNSIGNED_BYTE, greenTextureData.data());
    }

    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - greenTextureSize, getWindowHeight() - greenTextureSize,
                          GLColor::green);
}

// Tests that incomplete textures don't get initialized with the unpack buffer contents.
TEST_P(IncompleteTextureTestES3, UnpackBufferBound)
{
    std::vector<GLColor> red(16, GLColor::red);

    GLBuffer unpackBuffer;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpackBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, red.size() * sizeof(GLColor), red.data(), GL_STATIC_DRAW);

    draw2DTexturedQuad(0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
}

// Tests that the incomplete multisample texture has the correct alpha value.
TEST_P(IncompleteTextureTestES31, MultisampleTexture)
{
    constexpr char kVS[] = R"(#version 310 es
in vec2 position;
out vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = (position * 0.5) + 0.5;
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
in vec2 texCoord;
out vec4 color;
uniform mediump sampler2DMS tex;
void main()
{
    ivec2 texSize = textureSize(tex);
    ivec2 texel = ivec2(vec2(texSize) * texCoord);
    color = texelFetch(tex, texel, 0);
})";

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // The zero texture will be incomplete by default.
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
}

// This mirrors a scenario seen in GFXBench Car Chase where a
// default CUBE_MAP_ARRAY texture is used without being setup.
// Its ends up sampling from an incomplete texture.
TEST_P(IncompleteTextureTestES31, IncompleteTextureCubeMapArray)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_cube_map_array"));

    constexpr char kVS[] =
        R"(#version 310 es
        precision mediump float;
        in vec3 pos;
        void main() {
            gl_Position = vec4(pos, 1.0);
        })";

    constexpr char kFS[] =
        R"(#version 310 es
        #extension GL_EXT_texture_cube_map_array : enable
        precision mediump float;
        out vec4 color;
        uniform lowp samplerCubeArray uTex;
        void main(){
            vec4 outColor = vec4(0.0);

            // Pull a color from each cube face to ensure they are all initialized
            outColor += texture(uTex, vec4(1.0, 0.0, 0.0, 0.0));
            outColor += texture(uTex, vec4(-1.0, 0.0, 0.0, 0.0));
            outColor += texture(uTex, vec4(0.0, 1.0, 0.0, 0.0));
            outColor += texture(uTex, vec4(0.0, -1.0, 0.0, 0.0));
            outColor += texture(uTex, vec4(0.0, 0.0, 1.0, 0.0));
            outColor += texture(uTex, vec4(0.0, 0.0, -1.0, 0.0));

            color = outColor;
        })";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    glUniform1i(glGetUniformLocation(program, "uTex"), 0);
    glActiveTexture(GL_TEXTURE0);

    // Bind the default texture and don't set it up. This ends up being incomplete.
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

    drawQuad(program, "pos", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, angle::GLColor::black);
}

// Verifies that an incomplete integer texture has a signed integer type default value.
TEST_P(IncompleteTextureTestES3, IntegerType)
{
    // GLES backend on Adreno has a problem to create a incomplete texture, although it doesn't go
    // through the routine which creates a incomplete texture in the ANGLE driver.
    ANGLE_SKIP_TEST_IF(IsAdreno() && IsAndroid() && IsOpenGLES());

    // http://crbug.com/1168370
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsOpenGL());

    constexpr char kVS[] = R"(#version 300 es
in highp vec2 position;
out highp vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = (position * 0.5) + 0.5;
})";

    constexpr char kFS[] = R"(#version 300 es
in highp vec2 texCoord;
out highp ivec4 color;
uniform highp isampler2D tex;
void main()
{
    ivec2 texSize = textureSize(tex, 0);
    ivec2 texel = ivec2(vec2(texSize) * texCoord);
    color = texelFetch(tex, texel, 0);
})";

    constexpr GLint clearColori[4] = {-10, 20, -30, 40};
    constexpr GLint blackColori[4] = {0, 0, 0, 127};

    setupFramebuffer(GL_RGBA8I);
    glClearBufferiv(GL_COLOR, 0, clearColori);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_8I(0, 0, clearColori[0], clearColori[1], clearColori[2], clearColori[3]);

    // Since no texture attachment has been specified, it is incomplete by definition
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glUseProgram(program);

    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    const int width  = getWindowWidth() - 1;
    const int height = getWindowHeight() - 1;
    EXPECT_PIXEL_8I(0, 0, blackColori[0], blackColori[1], blackColori[2], blackColori[3]);
    EXPECT_PIXEL_8I(width, 0, blackColori[0], blackColori[1], blackColori[2], blackColori[3]);
    EXPECT_PIXEL_8I(0, height, blackColori[0], blackColori[1], blackColori[2], blackColori[3]);
    EXPECT_PIXEL_8I(width, height, blackColori[0], blackColori[1], blackColori[2], blackColori[3]);
}

// Verifies that an incomplete unsigned integer texture has an unsigned integer type default value.
TEST_P(IncompleteTextureTestES3, UnsignedIntegerType)
{
    // GLES backend on Adreno has a problem to create a incomplete texture, although it doesn't go
    // through the routine which creates a incomplete texture in the ANGLE driver.
    ANGLE_SKIP_TEST_IF(IsAdreno() && IsAndroid() && IsOpenGLES());

    // http://crbug.com/1168370
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsOpenGL());

    constexpr char kVS[] = R"(#version 300 es
in highp vec2 position;
out highp vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = (position * 0.5) + 0.5;
})";

    constexpr char kFS[] = R"(#version 300 es
in highp vec2 texCoord;
out highp uvec4 color;
uniform highp usampler2D tex;
void main()
{
    ivec2 texSize = textureSize(tex, 0);
    ivec2 texel = ivec2(vec2(texSize) * texCoord);
    color = texelFetch(tex, texel, 0);
})";

    constexpr GLuint clearColorui[4] = {40, 30, 20, 10};
    constexpr GLuint blackColorui[4] = {0, 0, 0, 255};

    setupFramebuffer(GL_RGBA8UI);
    glClearBufferuiv(GL_COLOR, 0, clearColorui);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_8UI(0, 0, clearColorui[0], clearColorui[1], clearColorui[2], clearColorui[3]);

    // Since no texture attachment has been specified, it is incomplete by definition
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glUseProgram(program);

    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    const int width  = getWindowWidth() - 1;
    const int height = getWindowHeight() - 1;
    EXPECT_PIXEL_8UI(0, 0, blackColorui[0], blackColorui[1], blackColorui[2], blackColorui[3]);
    EXPECT_PIXEL_8UI(width, 0, blackColorui[0], blackColorui[1], blackColorui[2], blackColorui[3]);
    EXPECT_PIXEL_8UI(0, height, blackColorui[0], blackColorui[1], blackColorui[2], blackColorui[3]);
    EXPECT_PIXEL_8UI(width, height, blackColorui[0], blackColorui[1], blackColorui[2],
                     blackColorui[3]);
}

// Verifies that we are able to create an incomplete shadow texture.
TEST_P(IncompleteTextureTestES3, ShadowType)
{
    // GLES backend on Adreno has a problem to create a incomplete texture, although it doesn't go
    // through the routine which creates a incomplete texture in the ANGLE driver.
    ANGLE_SKIP_TEST_IF(IsAdreno() && IsAndroid() && IsOpenGLES());

    // http://anglebug.com/42264125
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr char kVS[] = R"(#version 300 es
in highp vec2 position;
out highp vec3 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = vec3(((position * 0.5) + 0.5), 0.5);
})";

    constexpr char kFS[] = R"(#version 300 es
in highp vec3 texCoord;
out highp vec4 color;
uniform highp sampler2DShadow tex;
void main()
{
    color = vec4(vec3(texture(tex, texCoord)), 1.0f);
})";

    constexpr GLColor clearColor = {10, 40, 20, 30};
    constexpr GLColor blackColor = {0, 0, 0, 255};

    setupFramebuffer(GL_RGBA8);
    glClearBufferfv(GL_COLOR, 0, clearColor.toNormalizedVector().data());
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

    // Since no texture attachment has been specified, it is incomplete by definition
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 1);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    const int width  = getWindowWidth() - 1;
    const int height = getWindowHeight() - 1;
    EXPECT_PIXEL_EQ(0, 0, blackColor[0], blackColor[1], blackColor[2], blackColor[3]);
    EXPECT_PIXEL_EQ(width, 0, blackColor[0], blackColor[1], blackColor[2], blackColor[3]);
    EXPECT_PIXEL_EQ(0, height, blackColor[0], blackColor[1], blackColor[2], blackColor[3]);
    EXPECT_PIXEL_EQ(width, height, blackColor[0], blackColor[1], blackColor[2], blackColor[3]);
}

ANGLE_INSTANTIATE_TEST_ES2(IncompleteTextureTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(IncompleteTextureTestES3);
ANGLE_INSTANTIATE_TEST_ES3(IncompleteTextureTestES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(IncompleteTextureTestES31);
ANGLE_INSTANTIATE_TEST_ES31(IncompleteTextureTestES31);
