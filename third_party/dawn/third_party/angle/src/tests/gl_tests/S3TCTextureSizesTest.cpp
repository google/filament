//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Tests of DXT texture mipmap sizes required by WebGL.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "media/pixel.inc"

using namespace angle;

std::array<uint8_t, 72> k12x12DXT1Data = {
    0xe0, 0x03, 0x00, 0x78, 0x13, 0x10, 0x15, 0x00, 0x0f, 0x00, 0xe0, 0x7b, 0x11, 0x10, 0x15,
    0x00, 0xe0, 0x03, 0x0f, 0x78, 0x44, 0x45, 0x40, 0x55, 0x0f, 0x00, 0xef, 0x03, 0x44, 0x45,
    0x40, 0x55, 0xe0, 0x03, 0x00, 0x78, 0x13, 0x10, 0x15, 0x00, 0x0f, 0x00, 0xe0, 0x7b, 0x11,
    0x10, 0x15, 0x00, 0xe0, 0x03, 0x0f, 0x78, 0x44, 0x45, 0x40, 0x55, 0x0f, 0x00, 0xef, 0x03,
    0x44, 0x45, 0x40, 0x55, 0x0f, 0x00, 0xef, 0x03, 0x44, 0x45, 0x40, 0x55,
};

class S3TCTextureSizesTest : public ANGLETest<>
{
  protected:
    S3TCTextureSizesTest()
    {
        setWindowWidth(512);
        setWindowHeight(512);
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
    texcoord.y = 1.0 - texcoord.y;
})";

        constexpr char kFS[] = R"(precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

void main()
{
    gl_FragColor = texture2D(tex, texcoord);
})";

        mTextureProgram = CompileProgram(kVS, kFS);
        if (mTextureProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mTextureProgram, "tex");

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteProgram(mTextureProgram); }

    GLuint mTextureProgram;
    GLint mTextureUniformLocation;
};

// Test DXT1 formats with POT sizes on all mips
TEST_P(S3TCTextureSizesTest, POT)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8, 8, 0, 32,
                           k12x12DXT1Data.data());
    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor(0, 0, 123, 255));

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8, 4, 0, 16,
                           k12x12DXT1Data.data());
    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor(0, 0, 123, 255));

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8, 4, 0, 16,
                           k12x12DXT1Data.data());
    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor(0, 0, 123, 255));

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 4, 4, 0, 8,
                           k12x12DXT1Data.data());
    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor(123, 0, 0, 255));

    EXPECT_GL_NO_ERROR();
}

// Test DXT1 formats with NPOT sizes with glTexStorage
TEST_P(S3TCTextureSizesTest, NPOTTexStorage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       (!IsGLExtensionEnabled("GL_EXT_texture_storage") ||
                        !IsGLExtensionEnabled("GL_OES_rgb8_rgba8")));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    if (getClientMajorVersion() < 3)
    {
        glTexStorage2DEXT(GL_TEXTURE_2D, 4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 12, 12);
    }
    else
    {
        glTexStorage2D(GL_TEXTURE_2D, 4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 12, 12);
    }

    struct levelInfo
    {
        size_t width;
        size_t height;
        size_t size;
        GLColor expectedColor;
    };
    std::array<levelInfo, 4> levels{{
        {12, 12, 72, GLColor(123, 0, 123, 255)},
        {6, 6, 32, GLColor(123, 0, 123, 255)},
        {3, 3, 8, GLColor(123, 0, 0, 255)},
        {1, 1, 8, GLColor(0, 0, 0, 0)},
    }};

    for (size_t i = 0; i < levels.size(); i++)
    {
        const levelInfo &level = levels[i];
        glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, level.width, level.height,
                                  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, level.size,
                                  k12x12DXT1Data.data());
    }
    EXPECT_GL_NO_ERROR();

    for (size_t i = 0; i < levels.size(); i++)
    {
        const levelInfo &level = levels[i];
        glViewport(0, 0, level.width, level.height);
        drawQuad(mTextureProgram, "position", 0.5f);
        EXPECT_PIXEL_COLOR_EQ(0, 0, level.expectedColor) << " failed on level " << i;
    }

    EXPECT_GL_NO_ERROR();
}

// Test DXT1 formats with NPOT sizes with glTex[Sub]Image
TEST_P(S3TCTextureSizesTest, NPOTTexImage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    struct levelInfo
    {
        size_t width;
        size_t height;
        size_t size;
        GLColor expectedColor;
    };
    std::array<levelInfo, 4> levels{{
        {12, 12, 72, GLColor(123, 0, 123, 255)},
        {6, 6, 32, GLColor(123, 0, 123, 255)},
        {3, 3, 8, GLColor(123, 0, 0, 255)},
        {1, 1, 8, GLColor(0, 0, 0, 0)},
    }};

    for (size_t i = 0; i < levels.size(); i++)
    {
        const levelInfo &level = levels[i];
        glCompressedTexImage2D(GL_TEXTURE_2D, i, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, level.width,
                               level.height, 0, level.size, k12x12DXT1Data.data());
    }
    EXPECT_GL_NO_ERROR();

    for (size_t i = 0; i < levels.size(); i++)
    {
        const levelInfo &level = levels[i];
        glViewport(0, 0, level.width, level.height);
        drawQuad(mTextureProgram, "position", 0.5f);
        EXPECT_PIXEL_COLOR_EQ(0, 0, level.expectedColor) << " failed on level " << i;
    }

    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(S3TCTextureSizesTest);
