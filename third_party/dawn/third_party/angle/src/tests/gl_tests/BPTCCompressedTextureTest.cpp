//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BPTCCompressedTextureTest.cpp: Tests of the GL_EXT_texture_compression_bptc extension

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

const unsigned int kPixelTolerance = 1u;

// The pixel data represents a 4x4 pixel image with the left side colored red and the right side
// green. It was BC7 encoded using Microsoft's BC6HBC7Encoder.
const std::array<GLubyte, 16> kBC7Data4x4 = {0x50, 0x1f, 0xfc, 0xf, 0x0,  0xf0, 0xe3, 0xe1,
                                             0xe1, 0xe1, 0xc1, 0xf, 0xfc, 0xc0, 0xf,  0xfc};

// The pixel data represents a 4x4 pixel image with the transparent black solid color.
// Sampling from a zero-filled block is undefined, so use a valid one.
const std::array<GLubyte, 16> kBC7BlackData4x4 = {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
}  // anonymous namespace

class BPTCCompressedTextureTest : public ANGLETest<>
{
  protected:
    BPTCCompressedTextureTest()
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

    void setupTextureParameters(GLuint texture)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void drawTexture()
    {
        glUseProgram(mTextureProgram);
        glUniform1i(mTextureUniformLocation, 0);
        drawQuad(mTextureProgram, "position", 0.5f);
        EXPECT_GL_NO_ERROR();
    }

    GLuint mTextureProgram;
    GLint mTextureUniformLocation;
};

class BPTCCompressedTextureTestES3 : public BPTCCompressedTextureTest
{
  public:
    BPTCCompressedTextureTestES3() : BPTCCompressedTextureTest() {}
};

// Test sampling from a BC7 non-SRGB image.
TEST_P(BPTCCompressedTextureTest, CompressedTexImageBC7)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    GLTexture texture;
    setupTextureParameters(texture);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 4, 4, 0,
                           kBC7Data4x4.size(), kBC7Data4x4.data());

    EXPECT_GL_NO_ERROR();

    drawTexture();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(0, getWindowHeight() - 1, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, 0, GLColor::green, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green,
                            kPixelTolerance);
}

// Test sampling from a BC7 SRGB image.
TEST_P(BPTCCompressedTextureTest, CompressedTexImageBC7SRGB)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    GLTexture texture;
    setupTextureParameters(texture);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT, 4, 4, 0,
                           kBC7Data4x4.size(), kBC7Data4x4.data());

    EXPECT_GL_NO_ERROR();

    drawTexture();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(0, getWindowHeight() - 1, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, 0, GLColor::green, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green,
                            kPixelTolerance);
}

// Test that using the BC6H floating point formats doesn't crash.
TEST_P(BPTCCompressedTextureTest, CompressedTexImageBC6HNoCrash)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    GLTexture texture;
    setupTextureParameters(texture);

    // This fake pixel data represents a 4x4 pixel image.
    // TODO(http://anglebug.com/40096529): Add pixel tests for these formats. These need HDR source
    // images.
    std::vector<GLubyte> data;
    data.resize(16u, 0u);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, 4, 4, 0,
                           data.size(), data.data());
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT, 4, 4, 0,
                           data.size(), data.data());

    EXPECT_GL_NO_ERROR();

    drawTexture();
}

// Test texStorage2D with a BPTC format.
TEST_P(BPTCCompressedTextureTestES3, CompressedTexStorage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    GLTexture texture;
    setupTextureParameters(texture);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 4, 4);
    EXPECT_GL_NO_ERROR();
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT,
                              kBC7Data4x4.size(), kBC7Data4x4.data());
    EXPECT_GL_NO_ERROR();

    drawTexture();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(0, getWindowHeight() - 1, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, 0, GLColor::green, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green,
                            kPixelTolerance);
}

// Test validation of glCompressedTexSubImage2D with BPTC formats
TEST_P(BPTCCompressedTextureTest, CompressedTexSubImageValidation)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    std::vector<GLubyte> data(16 * 2 * 2);  // 2x2 blocks, thats 8x8 pixels.

    // Size mip 0 to a large size.
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 8, 8, 0,
                           data.size(), data.data());
    ASSERT_GL_NO_ERROR();

    // Test a sub image with an offset that isn't a multiple of the block size.
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 1, 0, 4, 4, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT,
                              kBC7Data4x4.size(), kBC7Data4x4.data());
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 3, 4, 4, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT,
                              kBC7Data4x4.size(), kBC7Data4x4.data());
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    // Test a sub image with a negative offset.
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, -1, 0, 4, 4, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT,
                              kBC7Data4x4.size(), kBC7Data4x4.data());
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, -1, 4, 4, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT,
                              kBC7Data4x4.size(), kBC7Data4x4.data());
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
}

// Test that copying BPTC textures is not allowed. This restriction exists only in
// EXT_texture_compression_bptc, and not in the ARB variant.
TEST_P(BPTCCompressedTextureTest, CopyTexImage2DDisallowed)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    GLTexture texture;
    setupTextureParameters(texture);

    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 0, 0, 4, 4, 0);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that copying BPTC textures is not allowed. This restriction exists only in
// EXT_texture_compression_bptc, and not in the ARB variant.
TEST_P(BPTCCompressedTextureTest, CopyTexSubImage2DDisallowed)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    GLTexture texture;
    setupTextureParameters(texture);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 4, 4, 0,
                           kBC7Data4x4.size(), kBC7Data4x4.data());
    ASSERT_GL_NO_ERROR();

    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 4, 4);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that copying BPTC textures is not allowed. This restriction exists only in
// EXT_texture_compression_bptc, and not in the ARB variant.
TEST_P(BPTCCompressedTextureTestES3, CopyTexSubImage3DDisallowed)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 4, 4, 1);
    ASSERT_GL_NO_ERROR();

    glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 0, 0, 4, 4);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test uploading texture data from a PBO to a texture.
TEST_P(BPTCCompressedTextureTestES3, PBOCompressedTexImage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    GLTexture texture;
    setupTextureParameters(texture);

    GLBuffer buffer;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, kBC7Data4x4.size(), kBC7Data4x4.data(), GL_STREAM_DRAW);
    ASSERT_GL_NO_ERROR();

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 4, 4, 0,
                           kBC7Data4x4.size(), nullptr);
    ASSERT_GL_NO_ERROR();

    drawTexture();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(0, getWindowHeight() - 1, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, 0, GLColor::green, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green,
                            kPixelTolerance);

    // Destroy the data
    glBufferData(GL_PIXEL_UNPACK_BUFFER, kBC7BlackData4x4.size(), kBC7BlackData4x4.data(),
                 GL_STREAM_DRAW);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 4, 4, 0,
                           kBC7BlackData4x4.size(), nullptr);
    ASSERT_GL_NO_ERROR();

    drawTexture();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    // Initialize again.  This time, the texture's image is already allocated, so the PBO data
    // upload could be directly done.
    glBufferData(GL_PIXEL_UNPACK_BUFFER, kBC7Data4x4.size(), kBC7Data4x4.data(), GL_STREAM_DRAW);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 4, 4, 0,
                           kBC7Data4x4.size(), nullptr);
    ASSERT_GL_NO_ERROR();

    drawTexture();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(0, getWindowHeight() - 1, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, 0, GLColor::green, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green,
                            kPixelTolerance);
}

// Test uploading texture data from a PBO to a non-zero base texture.
TEST_P(BPTCCompressedTextureTestES3, PBOCompressedTexImageNonZeroBase)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    GLTexture texture;
    setupTextureParameters(texture);

    GLBuffer buffer;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, kBC7Data4x4.size(), kBC7Data4x4.data(), GL_STREAM_DRAW);
    ASSERT_GL_NO_ERROR();

    glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 4, 4, 0,
                           kBC7Data4x4.size(), nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    ASSERT_GL_NO_ERROR();

    drawTexture();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(0, getWindowHeight() - 1, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, 0, GLColor::green, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green,
                            kPixelTolerance);

    // Destroy the data
    glBufferData(GL_PIXEL_UNPACK_BUFFER, kBC7BlackData4x4.size(), kBC7BlackData4x4.data(),
                 GL_STREAM_DRAW);
    glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 4, 4, 0,
                           kBC7BlackData4x4.size(), nullptr);
    ASSERT_GL_NO_ERROR();

    drawTexture();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    // Initialize again.  This time, the texture's image is already allocated, so the PBO data
    // upload could be directly done.
    glBufferData(GL_PIXEL_UNPACK_BUFFER, kBC7Data4x4.size(), kBC7Data4x4.data(), GL_STREAM_DRAW);
    glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 4, 4, 0,
                           kBC7Data4x4.size(), nullptr);
    ASSERT_GL_NO_ERROR();

    drawTexture();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(0, getWindowHeight() - 1, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, 0, GLColor::green, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green,
                            kPixelTolerance);
}

// Test uploading texture data from a PBO to a texture allocated with texStorage2D.
TEST_P(BPTCCompressedTextureTestES3, PBOCompressedTexStorage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    GLTexture texture;
    setupTextureParameters(texture);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 4, 4);
    ASSERT_GL_NO_ERROR();

    GLBuffer buffer;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, kBC7Data4x4.size(), kBC7Data4x4.data(), GL_STREAM_DRAW);
    ASSERT_GL_NO_ERROR();

    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT,
                              kBC7Data4x4.size(), nullptr);

    ASSERT_GL_NO_ERROR();

    drawTexture();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(0, getWindowHeight() - 1, GLColor::red, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, 0, GLColor::green, kPixelTolerance);
    EXPECT_PIXEL_COLOR_NEAR(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green,
                            kPixelTolerance);
}

// Test validation of glCompressedTexSubImage3D with BPTC formats
TEST_P(BPTCCompressedTextureTestES3, CompressedTexSubImage3DValidation)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_bptc"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

    std::vector<GLubyte> data(16 * 2 * 2);  // 2x2x1 blocks, thats 8x8x1 pixels.

    // Size mip 0 to a large size.
    glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 8, 8, 1, 0,
                           data.size(), data.data());
    ASSERT_GL_NO_ERROR();

    // Test a sub image with an offset that isn't a multiple of the block size.
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 2, 0, 0, 4, 4, 1,
                              GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, kBC7Data4x4.size(),
                              kBC7Data4x4.data());
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 2, 0, 4, 4, 1,
                              GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, kBC7Data4x4.size(),
                              kBC7Data4x4.data());
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    // Test a sub image with a negative offset.
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, -1, 0, 0, 4, 4, 1,
                              GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, kBC7Data4x4.size(),
                              kBC7Data4x4.data());
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, -1, 0, 4, 4, 1,
                              GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, kBC7Data4x4.size(),
                              kBC7Data4x4.data());
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, -1, 4, 4, 1,
                              GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, kBC7Data4x4.size(),
                              kBC7Data4x4.data());
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(BPTCCompressedTextureTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BPTCCompressedTextureTestES3);
ANGLE_INSTANTIATE_TEST_ES3(BPTCCompressedTextureTestES3);
