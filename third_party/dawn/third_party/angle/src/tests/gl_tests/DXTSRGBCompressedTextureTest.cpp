//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DXTSRGBCompressedTextureTest.cpp
//   Tests for sRGB DXT textures (GL_EXT_texture_compression_s3tc_srgb)
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "media/pixel.inc"

#include "DXTSRGBCompressedTextureTestData.inl"

using namespace angle;

static constexpr int kWindowSize = 64;

class DXTSRGBCompressedTextureTest : public ANGLETest<>
{
  protected:
    DXTSRGBCompressedTextureTest()
    {
        setWindowWidth(kWindowSize);
        setWindowHeight(kWindowSize);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        constexpr char kVS[] =
            "precision highp float;\n"
            "attribute vec4 position;\n"
            "varying vec2 texcoord;\n"
            "void main() {\n"
            "    gl_Position = position;\n"
            "    texcoord = (position.xy * 0.5) + 0.5;\n"
            "    texcoord.y = 1.0 - texcoord.y;\n"
            "}";

        constexpr char kFS[] =
            "precision highp float;\n"
            "uniform sampler2D tex;\n"
            "varying vec2 texcoord;\n"
            "void main() {\n"
            "    gl_FragColor = texture2D(tex, texcoord);\n"
            "}\n";

        mTextureProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mTextureProgram);

        mTextureUniformLocation = glGetUniformLocation(mTextureProgram, "tex");
        ASSERT_NE(-1, mTextureUniformLocation);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteProgram(mTextureProgram); }

    void runTestChecks(const TestCase &test)
    {
        GLColor actual[kWindowSize * kWindowSize] = {0};
        drawQuad(mTextureProgram, "position", 0.5f);
        ASSERT_GL_NO_ERROR();
        glReadPixels(0, 0, kWindowSize, kWindowSize, GL_RGBA, GL_UNSIGNED_BYTE,
                     reinterpret_cast<void *>(actual));
        ASSERT_GL_NO_ERROR();
        for (GLsizei y = 0; y < test.height; ++y)
        {
            for (GLsizei x = 0; x < test.width; ++x)
            {
                GLColor exp = reinterpret_cast<const GLColor *>(test.expected)[y * test.width + x];
                size_t x_actual = (x * kWindowSize + kWindowSize / 2) / test.width;
                size_t y_actual =
                    ((test.height - y - 1) * kWindowSize + kWindowSize / 2) / test.height;
                GLColor act = actual[y_actual * kWindowSize + x_actual];
                EXPECT_COLOR_NEAR(exp, act, 2.0);
            }
        }
    }

    void runTest(GLenum format)
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_s3tc_srgb"));

        const TestCase &test = kTests.at(format);

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glUseProgram(mTextureProgram);
        glUniform1i(mTextureUniformLocation, 0);
        ASSERT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, test.width, test.height, 0, test.dataSize,
                               test.data);
        ASSERT_GL_NO_ERROR() << "glCompressedTexImage2D(format=" << format << ")";
        runTestChecks(test);

        glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, test.width, test.height, 0, test.dataSize,
                               nullptr);
        ASSERT_GL_NO_ERROR() << "glCompressedTexImage2D(format=" << format << ", data=null)";
        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, test.width, test.height, format,
                                  test.dataSize, test.data);
        ASSERT_GL_NO_ERROR() << "glCompressedTexSubImage2D(format=" << format << ")";
        runTestChecks(test);

        ASSERT_GL_NO_ERROR();
    }

    GLuint mTextureProgram        = 0;
    GLint mTextureUniformLocation = -1;
};

// Test correct decompression of 8x8 textures (four 4x4 blocks) of SRGB_S3TC_DXT1
TEST_P(DXTSRGBCompressedTextureTest, Decompression8x8RGBDXT1)
{
    runTest(GL_COMPRESSED_SRGB_S3TC_DXT1_EXT);
}

// Test correct decompression of 8x8 textures (four 4x4 blocks) of SRGB_ALPHA_S3TC_DXT1
TEST_P(DXTSRGBCompressedTextureTest, Decompression8x8RGBADXT1)
{
    runTest(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT);
}

// Test correct decompression of 8x8 textures (four 4x4 blocks) of SRGB_ALPHA_S3TC_DXT3
TEST_P(DXTSRGBCompressedTextureTest, Decompression8x8RGBADXT3)
{
    runTest(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT);
}

// Test correct decompression of 8x8 textures (four 4x4 blocks) of SRGB_ALPHA_S3TC_DXT5
TEST_P(DXTSRGBCompressedTextureTest, Decompression8x8RGBADXT5)
{
    runTest(GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(DXTSRGBCompressedTextureTest);
