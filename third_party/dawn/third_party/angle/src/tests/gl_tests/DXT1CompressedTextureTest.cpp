//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "media/pixel.inc"

using namespace angle;

class DXT1CompressedTextureTest : public ANGLETest<>
{
  protected:
    DXT1CompressedTextureTest()
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

TEST_P(DXT1CompressedTextureTest, CompressedTexImage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width,
                           pixel_0_height, 0, pixel_0_size, pixel_0_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_1_width,
                           pixel_1_height, 0, pixel_1_size, pixel_1_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 2, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_2_width,
                           pixel_2_height, 0, pixel_2_size, pixel_2_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 3, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_3_width,
                           pixel_3_height, 0, pixel_3_size, pixel_3_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_4_width,
                           pixel_4_height, 0, pixel_4_size, pixel_4_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 5, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_5_width,
                           pixel_5_height, 0, pixel_5_size, pixel_5_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 6, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_6_width,
                           pixel_6_height, 0, pixel_6_size, pixel_6_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 7, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_7_width,
                           pixel_7_height, 0, pixel_7_size, pixel_7_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_8_width,
                           pixel_8_height, 0, pixel_8_size, pixel_8_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 9, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_9_width,
                           pixel_9_height, 0, pixel_9_size, pixel_9_data);

    EXPECT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);

    EXPECT_GL_NO_ERROR();

    glDeleteTextures(1, &texture);

    EXPECT_GL_NO_ERROR();
}

// Verify that DXT1 RGB textures have 1.0 alpha when sampled
TEST_P(DXT1CompressedTextureTest, DXT1Alpha)
{
    auto test = [&](const std::string &extName, GLenum format) {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(extName));

        // On platforms without native support for DXT1 RGB or texture swizzling (such as D3D or
        // some Metal configurations), this test is allowed to succeed with transparent black
        // instead of opaque black.
        const bool opaque = !IsD3D() && !(IsMetal() && !IsMetalTextureSwizzleAvailable());

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Image using pixels with the code for transparent black:
        //          "BLACK,             if color0 <= color1 and code(x,y) == 3"
        constexpr uint8_t CompressedImageDXT1[] = {0, 0, 0, 0, 255, 255, 255, 255};
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, 4, 4, 0, sizeof(CompressedImageDXT1),
                               CompressedImageDXT1);

        EXPECT_GL_NO_ERROR();

        glUseProgram(mTextureProgram);
        glUniform1i(mTextureUniformLocation, 0);

        constexpr GLint kDrawSize = 4;
        // The image is one 4x4 block, make the viewport only 4x4.
        glViewport(0, 0, kDrawSize, kDrawSize);

        drawQuad(mTextureProgram, "position", 0.5f);

        EXPECT_GL_NO_ERROR();

        for (GLint y = 0; y < kDrawSize; y++)
        {
            for (GLint x = 0; x < kDrawSize; x++)
            {
                EXPECT_PIXEL_EQ(x, y, 0, 0, 0, opaque ? 255 : 0)
                    << "at (" << x << ", " << y << ") for " << extName;
            }
        }
    };

    test("GL_EXT_texture_compression_dxt1", GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
    test("GL_EXT_texture_compression_s3tc_srgb", GL_COMPRESSED_SRGB_S3TC_DXT1_EXT);
}

TEST_P(DXT1CompressedTextureTest, CompressedTexStorage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       (!IsGLExtensionEnabled("GL_EXT_texture_storage") ||
                        !IsGLExtensionEnabled("GL_OES_rgb8_rgba8")));

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (getClientMajorVersion() < 3)
    {
        glTexStorage2DEXT(GL_TEXTURE_2D, pixel_levels, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
                          pixel_0_width, pixel_0_height);
    }
    else
    {
        glTexStorage2D(GL_TEXTURE_2D, pixel_levels, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width,
                       pixel_0_height);
    }
    EXPECT_GL_NO_ERROR();

    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pixel_0_width, pixel_0_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_size, pixel_0_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, pixel_1_width, pixel_1_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_1_size, pixel_1_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, pixel_2_width, pixel_2_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_2_size, pixel_2_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 3, 0, 0, pixel_3_width, pixel_3_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_3_size, pixel_3_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 4, 0, 0, pixel_4_width, pixel_4_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_4_size, pixel_4_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 5, 0, 0, pixel_5_width, pixel_5_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_5_size, pixel_5_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 6, 0, 0, pixel_6_width, pixel_6_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_6_size, pixel_6_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 7, 0, 0, pixel_7_width, pixel_7_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_7_size, pixel_7_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 8, 0, 0, pixel_8_width, pixel_8_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_8_size, pixel_8_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 9, 0, 0, pixel_9_width, pixel_9_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_9_size, pixel_9_data);

    EXPECT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);

    EXPECT_GL_NO_ERROR();

    glDeleteTextures(1, &texture);

    EXPECT_GL_NO_ERROR();
}

// Test validation of non block sizes, width 672 and height 114 and multiple mip levels
TEST_P(DXT1CompressedTextureTest, NonBlockSizesMipLevels)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    constexpr GLuint kWidth  = 674;
    constexpr GLuint kHeight = 114;

    // From EXT_texture_compression_s3tc specifications:
    // When an S3TC image with a width of <w>, height of <h>, and block size of
    // <blocksize> (8 or 16 bytes) is decoded, the corresponding image size (in
    // bytes) is:
    //     ceil(<w>/4) * ceil(<h>/4) * blocksize.
    constexpr GLuint kImageSize = ((kWidth + 3) / 4) * ((kHeight + 3) / 4) * 8;

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, kWidth, kHeight, 0,
                           kImageSize, nullptr);
    ASSERT_GL_NO_ERROR();

    constexpr GLuint kImageSize1 = ((kWidth / 2 + 3) / 4) * ((kHeight / 2 + 3) / 4) * 8;
    glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, kWidth / 2,
                           kHeight / 2, 0, kImageSize1, nullptr);
    ASSERT_GL_NO_ERROR();

    constexpr GLuint kImageSize2 = ((kWidth / 4 + 3) / 4) * ((kHeight / 4 + 3) / 4) * 8;
    glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, kWidth / 4,
                           kHeight / 4, 0, kImageSize2, nullptr);
    ASSERT_GL_NO_ERROR();
}

// Test validation of glCompressedTexSubImage2D with DXT formats
TEST_P(DXT1CompressedTextureTest, CompressedTexSubImageValidation)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    // Size mip 0 to a large size
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width,
                           pixel_0_height, 0, pixel_0_size, nullptr);
    ASSERT_GL_NO_ERROR();

    // Set a sub image with an offset that isn't a multiple of the block size
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 1, 3, pixel_1_width, pixel_1_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_1_size, pixel_1_data);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    // Set a sub image with a negative offset
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, -1, 0, 4, 4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8,
                              pixel_1_data);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, -1, 4, 4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8,
                              pixel_1_data);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
}

// Test that it's not possible to call CopyTexSubImage2D on a compressed texture
TEST_P(DXT1CompressedTextureTest, CopyTexSubImage2DDisallowed)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width,
                           pixel_0_height, 0, pixel_0_size, nullptr);
    ASSERT_GL_NO_ERROR();

    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 4, 4);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

TEST_P(DXT1CompressedTextureTest, PBOCompressedTexStorage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_NV_pixel_buffer_object"));

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       (!IsGLExtensionEnabled("GL_EXT_texture_storage") ||
                        !IsGLExtensionEnabled("GL_OES_rgb8_rgba8")));

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (getClientMajorVersion() < 3)
    {
        glTexStorage2DEXT(GL_TEXTURE_2D, pixel_levels, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
                          pixel_0_width, pixel_0_height);
    }
    else
    {
        glTexStorage2D(GL_TEXTURE_2D, pixel_levels, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width,
                       pixel_0_height);
    }
    EXPECT_GL_NO_ERROR();

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, pixel_0_size, nullptr, GL_STREAM_DRAW);
    EXPECT_GL_NO_ERROR();

    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_0_size, pixel_0_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pixel_0_width, pixel_0_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_1_size, pixel_1_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, pixel_1_width, pixel_1_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_1_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_2_size, pixel_2_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, pixel_2_width, pixel_2_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_2_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_3_size, pixel_3_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 3, 0, 0, pixel_3_width, pixel_3_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_3_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_4_size, pixel_4_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 4, 0, 0, pixel_4_width, pixel_4_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_4_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_5_size, pixel_5_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 5, 0, 0, pixel_5_width, pixel_5_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_5_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_6_size, pixel_6_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 6, 0, 0, pixel_6_width, pixel_6_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_6_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_7_size, pixel_7_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 7, 0, 0, pixel_7_width, pixel_7_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_7_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_8_size, pixel_8_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 8, 0, 0, pixel_8_width, pixel_8_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_8_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_9_size, pixel_9_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 9, 0, 0, pixel_9_width, pixel_9_height,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_9_size, nullptr);

    EXPECT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);

    EXPECT_GL_NO_ERROR();

    glDeleteTextures(1, &texture);

    EXPECT_GL_NO_ERROR();
}

class DXT1CompressedTextureTestES3 : public DXT1CompressedTextureTest
{};

TEST_P(DXT1CompressedTextureTestES3, PBOCompressedTexImage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, pixel_0_size, nullptr, GL_STREAM_DRAW);
    EXPECT_GL_NO_ERROR();

    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_0_size, pixel_0_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width,
                           pixel_0_height, 0, pixel_0_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_1_size, pixel_1_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_1_width,
                           pixel_1_height, 0, pixel_1_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_2_size, pixel_2_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 2, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_2_width,
                           pixel_2_height, 0, pixel_2_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_3_size, pixel_3_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 3, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_3_width,
                           pixel_3_height, 0, pixel_3_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_4_size, pixel_4_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_4_width,
                           pixel_4_height, 0, pixel_4_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_5_size, pixel_5_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 5, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_5_width,
                           pixel_5_height, 0, pixel_5_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_6_size, pixel_6_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 6, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_6_width,
                           pixel_6_height, 0, pixel_6_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_7_size, pixel_7_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 7, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_7_width,
                           pixel_7_height, 0, pixel_7_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_8_size, pixel_8_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_8_width,
                           pixel_8_height, 0, pixel_8_size, nullptr);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_9_size, pixel_9_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 9, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_9_width,
                           pixel_9_height, 0, pixel_9_size, nullptr);

    EXPECT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);

    EXPECT_GL_NO_ERROR();

    glDeleteTextures(1, &buffer);
    glDeleteTextures(1, &texture);

    EXPECT_GL_NO_ERROR();
}

// Test validation of glCompressedTexSubImage3D with DXT formats
TEST_P(DXT1CompressedTextureTestES3, CompressedTexSubImageValidation)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

    // Size mip 0 to a large size
    glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width,
                           pixel_0_height, 1, 0, pixel_0_size, nullptr);
    ASSERT_GL_NO_ERROR();

    // Set a sub image with a negative offset
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, -1, 0, 0, 4, 4, 1,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8, pixel_1_data);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, -1, 0, 4, 4, 1,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8, pixel_1_data);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, -1, 4, 4, 1,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8, pixel_1_data);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
}

// Test validation of glCompressedTexSubImage3D with per-slice data uploads
TEST_P(DXT1CompressedTextureTestES3, CompressedTexSubImage3DValidationPerSlice)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    const GLenum format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

    // 8x8x2, 4x4x2, 2x2x2, 1x1x2
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, format, 8, 8, 2);
    ASSERT_GL_NO_ERROR();

    uint8_t data[32] = {};
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 8, 8, 1, format, 32, data);
    ASSERT_GL_NO_ERROR();
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, 8, 8, 1, format, 32, data);
    ASSERT_GL_NO_ERROR();

    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 1, 0, 0, 0, 4, 4, 1, format, 8, data);
    ASSERT_GL_NO_ERROR();
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 1, 0, 0, 1, 4, 4, 1, format, 8, data);
    ASSERT_GL_NO_ERROR();

    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 2, 0, 0, 0, 2, 2, 1, format, 8, data);
    ASSERT_GL_NO_ERROR();
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 2, 0, 0, 1, 2, 2, 1, format, 8, data);
    ASSERT_GL_NO_ERROR();

    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 3, 0, 0, 0, 1, 1, 1, format, 8, data);
    ASSERT_GL_NO_ERROR();
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 3, 0, 0, 1, 1, 1, 1, format, 8, data);
    ASSERT_GL_NO_ERROR();
}

// Test validation of glCompressedTexSubImage3D with combined per-level data uploads
TEST_P(DXT1CompressedTextureTestES3, CompressedTexSubImage3DValidationPerLevel)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    const GLenum format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

    // 8x8x2, 4x4x2, 2x2x2, 1x1x2
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, format, 8, 8, 2);
    ASSERT_GL_NO_ERROR();

    uint8_t data[64] = {};
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 8, 8, 2, format, 64, data);
    ASSERT_GL_NO_ERROR();

    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 1, 0, 0, 0, 4, 4, 2, format, 16, data);
    ASSERT_GL_NO_ERROR();

    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 2, 0, 0, 0, 2, 2, 2, format, 16, data);
    ASSERT_GL_NO_ERROR();

    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 3, 0, 0, 0, 1, 1, 2, format, 16, data);
    ASSERT_GL_NO_ERROR();
}

// Test validation of glCompressedTexSubImage3D with DXT formats
TEST_P(DXT1CompressedTextureTestES3, CopyTexSubImage3DDisallowed)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

    GLsizei depth = 4;
    glCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width,
                           pixel_0_height, depth, 0, pixel_0_size * depth, nullptr);
    ASSERT_GL_NO_ERROR();

    glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 0, 0, 4, 4);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

class DXT1CompressedTextureTestWebGL2 : public DXT1CompressedTextureTest
{
  protected:
    DXT1CompressedTextureTestWebGL2()
    {
        setWebGLCompatibilityEnabled(true);
        setRobustResourceInit(true);
    }
};

// Regression test for https://crbug.com/1289428
TEST_P(DXT1CompressedTextureTestWebGL2, InitializeTextureContents)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    glClearColor(0, 0, 1, 1);

    const std::array<uint8_t, 8> kGreen = {0xE0, 0x07, 0xE0, 0x07, 0x00, 0x00, 0x00, 0x00};

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 4, 4);
    EXPECT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawQuad(mTextureProgram, "position", 0.5f, 1.0f, true);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::black);

    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 8,
                              kGreen.data());
    EXPECT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawQuad(mTextureProgram, "position", 0.5f, 1.0f, true);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::green);
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(DXT1CompressedTextureTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DXT1CompressedTextureTestES3);
ANGLE_INSTANTIATE_TEST_ES3_AND(DXT1CompressedTextureTestES3,
                               ES3_VULKAN().enable(angle::Feature::ForceRobustResourceInit));

ANGLE_INSTANTIATE_TEST_ES3(DXT1CompressedTextureTestWebGL2);
