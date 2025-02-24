//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ETCTextureTest:
//   Tests for ETC lossy decode formats.
//

#include "test_utils/ANGLETest.h"

#include "media/etc2bc_srgb8_alpha8.inc"

using namespace angle;

namespace
{

class ETCTextureTest : public ANGLETest<>
{
  protected:
    ETCTextureTest() : mTexture(0u)
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
        glGenTextures(1, &mTexture);
        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteTextures(1, &mTexture); }

    GLuint mTexture;
};

// Tests a texture with ETC1 lossy decode format
TEST_P(ETCTextureTest, ETC1Validation)
{
    bool supported = IsGLExtensionEnabled("GL_ANGLE_lossy_etc_decode");

    glBindTexture(GL_TEXTURE_2D, mTexture);

    GLubyte pixel[8] = {0x0, 0x0, 0xf8, 0x2, 0x43, 0xff, 0x4, 0x12};
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_LOSSY_DECODE_ANGLE, 4, 4, 0,
                           sizeof(pixel), pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_ETC1_RGB8_LOSSY_DECODE_ANGLE,
                                  sizeof(pixel), pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_ETC1_RGB8_LOSSY_DECODE_ANGLE, 2, 2, 0,
                               sizeof(pixel), pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 2, GL_ETC1_RGB8_LOSSY_DECODE_ANGLE, 1, 1, 0,
                               sizeof(pixel), pixel);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Tests a texture with ETC2 RGB8 lossy decode format
TEST_P(ETCTextureTest, ETC2RGB8Validation)
{
    bool supported = IsGLExtensionEnabled("GL_ANGLE_lossy_etc_decode");

    glBindTexture(GL_TEXTURE_2D, mTexture);

    GLubyte pixel[] = {
        0x00, 0x00, 0xf8, 0x02, 0x43, 0xff, 0x04, 0x12,  // Individual/differential block
        0x1c, 0x65, 0xc6, 0x62, 0xff, 0xf0, 0xff, 0x00,  // T block
        0x62, 0xf2, 0xe3, 0x32, 0xff, 0x0f, 0xff, 0x00,  // H block
        0x71, 0x88, 0xfb, 0xee, 0x87, 0x07, 0x11, 0x1f   // Planar block
    };
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE, 8, 8, 0,
                           sizeof(pixel), pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8,
                                  GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE, sizeof(pixel), pixel);
        EXPECT_GL_NO_ERROR();

        const GLsizei imageSize = 8;

        glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE, 4, 4,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 2, GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE, 2, 2,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 3, GL_COMPRESSED_RGB8_LOSSY_DECODE_ETC2_ANGLE, 1, 1,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Tests a cube map array texture with compressed ETC2 RGB8 format
TEST_P(ETCTextureTest, ETC2RGB8_CubeMapValidation)
{
    ANGLE_SKIP_TEST_IF(!(IsGLExtensionEnabled("GL_EXT_texture_cube_map_array") &&
                         (getClientMajorVersion() >= 3 && getClientMinorVersion() > 1)));

    constexpr GLsizei kInvalidTextureWidth  = 8;
    constexpr GLsizei kInvalidTextureHeight = 8;
    constexpr GLsizei kCubemapFaceCount     = 6;
    const std::vector<GLubyte> kInvalidTextureData(
        kInvalidTextureWidth * kInvalidTextureHeight * kCubemapFaceCount, 0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
    EXPECT_GL_NO_ERROR();

    glCompressedTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB, kInvalidTextureWidth,
                           kInvalidTextureHeight, kCubemapFaceCount, 0, kInvalidTextureData.size(),
                           kInvalidTextureData.data());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    constexpr GLenum kFormat = GL_COMPRESSED_RGB8_ETC2;

    std::vector<GLubyte> arrayData;

    constexpr GLuint kWidth       = 4u;
    constexpr GLuint kHeight      = 4u;
    constexpr GLuint kDepth       = 6u;
    constexpr GLuint kPixelBytes  = 8u;
    constexpr GLuint kBlockWidth  = 4u;
    constexpr GLuint kBlockHeight = 4u;

    constexpr GLuint kNumBlocksWide = (kWidth + kBlockWidth - 1u) / kBlockWidth;
    constexpr GLuint kNumBlocksHigh = (kHeight + kBlockHeight - 1u) / kBlockHeight;
    constexpr GLuint kBytes         = kNumBlocksWide * kNumBlocksHigh * kPixelBytes * kDepth;

    arrayData.reserve(kBytes);

    glCompressedTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, kFormat, kWidth, kHeight, kDepth, 0,
                           kBytes, arrayData.data());
    EXPECT_GL_NO_ERROR();

    glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 0, kInvalidTextureWidth,
                              kInvalidTextureHeight, kDepth, GL_RGB, kInvalidTextureData.size(),
                              kInvalidTextureData.data());
    glCompressedTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 0, kInvalidTextureWidth,
                              kInvalidTextureHeight, kDepth, GL_RGB, kInvalidTextureData.size(),
                              kInvalidTextureData.data());
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Tests a texture with ETC2 SRGB8 lossy decode format
TEST_P(ETCTextureTest, ETC2SRGB8Validation)
{
    bool supported = IsGLExtensionEnabled("GL_ANGLE_lossy_etc_decode");

    glBindTexture(GL_TEXTURE_2D, mTexture);

    GLubyte pixel[] = {
        0x00, 0x00, 0xf8, 0x02, 0x43, 0xff, 0x04, 0x12,  // Individual/differential block
        0x1c, 0x65, 0xc6, 0x62, 0xff, 0xf0, 0xff, 0x00,  // T block
        0x62, 0xf2, 0xe3, 0x32, 0xff, 0x0f, 0xff, 0x00,  // H block
        0x71, 0x88, 0xfb, 0xee, 0x87, 0x07, 0x11, 0x1f   // Planar block
    };
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE, 8, 8, 0,
                           sizeof(pixel), pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8,
                                  GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE, sizeof(pixel),
                                  pixel);
        EXPECT_GL_NO_ERROR();

        const GLsizei imageSize = 8;

        glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE, 4, 4,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 2, GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE, 2, 2,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 3, GL_COMPRESSED_SRGB8_LOSSY_DECODE_ETC2_ANGLE, 1, 1,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Tests a texture with ETC2 RGB8 punchthrough A1 lossy decode format
TEST_P(ETCTextureTest, ETC2RGB8A1Validation)
{
    bool supported = IsGLExtensionEnabled("GL_ANGLE_lossy_etc_decode");

    glBindTexture(GL_TEXTURE_2D, mTexture);

    GLubyte pixel[] = {
        0x80, 0x98, 0x59, 0x02, 0x6e, 0xe7, 0x44, 0x47,  // Individual/differential block
        0xeb, 0x85, 0x68, 0x30, 0x77, 0x73, 0x44, 0x44,  // T block
        0xb4, 0x05, 0xab, 0x92, 0xf8, 0x8c, 0x07, 0x73,  // H block
        0xbb, 0x90, 0x15, 0xba, 0x8a, 0x8c, 0xd5, 0x5f   // Planar block
    };
    glCompressedTexImage2D(GL_TEXTURE_2D, 0,
                           GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 8, 8, 0,
                           sizeof(pixel), pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8,
                                  GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE,
                                  sizeof(pixel), pixel);
        EXPECT_GL_NO_ERROR();

        const GLsizei imageSize = 8;

        glCompressedTexImage2D(GL_TEXTURE_2D, 1,
                               GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 4, 4,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 2,
                               GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 2, 2,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 3,
                               GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 1, 1,
                               0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Tests a texture with ETC2 SRGB8 punchthrough A1 lossy decode format
TEST_P(ETCTextureTest, ETC2SRGB8A1Validation)
{
    bool supported = IsGLExtensionEnabled("GL_ANGLE_lossy_etc_decode");

    glBindTexture(GL_TEXTURE_2D, mTexture);

    GLubyte pixel[] = {
        0x80, 0x98, 0x59, 0x02, 0x6e, 0xe7, 0x44, 0x47,  // Individual/differential block
        0xeb, 0x85, 0x68, 0x30, 0x77, 0x73, 0x44, 0x44,  // T block
        0xb4, 0x05, 0xab, 0x92, 0xf8, 0x8c, 0x07, 0x73,  // H block
        0xbb, 0x90, 0x15, 0xba, 0x8a, 0x8c, 0xd5, 0x5f   // Planar block
    };
    glCompressedTexImage2D(GL_TEXTURE_2D, 0,
                           GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 8, 8, 0,
                           sizeof(pixel), pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8,
                                  GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE,
                                  sizeof(pixel), pixel);
        EXPECT_GL_NO_ERROR();

        const GLsizei imageSize = 8;

        glCompressedTexImage2D(GL_TEXTURE_2D, 1,
                               GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 4,
                               4, 0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 2,
                               GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 2,
                               2, 0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();

        glCompressedTexImage2D(GL_TEXTURE_2D, 3,
                               GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_LOSSY_DECODE_ETC2_ANGLE, 1,
                               1, 0, imageSize, pixel);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

class ETCToBCTextureTest : public ANGLETest<>
{
  protected:
    static constexpr int kWidth    = 4;
    static constexpr int kHeight   = 4;
    static constexpr int kTexSize  = 4;
    static constexpr int kAbsError = 6;
    // the transcoded BC1 data
    // {0x0000a534, 0x05055555}
    // then decoded RGBA data are
    // 0xff000000 0xff000000 0xff000000 0xff000000
    // 0xff000000 0xff000000 0xff000000 0xff000000
    // 0xff000000 0xff000000 0xffa5a6a5 0xffa5a6a5
    // 0xff000000 0xff000000 0xffa5a6a5 0xffa5a6a5
    static constexpr uint32_t kEtcAllZero[2]        = {0x0, 0x0};
    static constexpr uint32_t kEtcRGBData[2]        = {0x14050505, 0x00ffff33};
    static constexpr uint32_t kExpectedRGBColor[16] = {
        0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000,
        0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xffa5a6a5, 0xffa5a6a5,
        0xff000000, 0xff000000, 0xffa5a6a5, 0xffa5a6a5,
    };
    // the transcoded RGB data (BC1)
    // {0x0000a534 0x58585c5f}
    // the 16 ETC2 decoded alpha value are
    // 20,  20, 182, 182
    // 128, 128, 110, 110
    // 128, 47,   47, 47
    // 128, 65,   65, 65
    // max alpha value = 0xb6
    // min alpha value = 0x14
    // Result BC4 data are 0xb00914b6, 0xdb3ffb91
    static constexpr uint32_t kEtcRGBAData[4] = {0xd556975c, 0x088ff048, 0x9e6c6c6c, 0x3f11f1ff};
    static constexpr uint32_t kExpectedRGBAColor[16] = {
        0x14373737, 0x14373737, 0xb6000000, 0xb6000000, 0x88a5a6a5, 0x88373737,
        0x70000000, 0x70000000, 0x88a5a6a5, 0x2b6e6f6e, 0x2b000000, 0x2b000000,
        0x88a5a6a5, 0x426e6f6e, 0x42000000, 0x42000000,
    };
    // Result BC4 data as {0xf6f1836f, 0xc41c5e7c}
    static constexpr uint32_t kEacR11Signed[2]            = {0xb068efff, 0x00b989e7};
    static constexpr uint32_t kExpectedR11SignedColor[16] = {
        0xff000003, 0xff000046, 0xff0000ac, 0xff0000ac, 0xff000025, 0xff000003,
        0xff000025, 0xff0000ac, 0xff000046, 0xff0000ac, 0xff000003, 0xff000046,
        0xff000003, 0xff0000ef, 0xff000003, 0xff000046,
    };

    // Result BC1 data as {0xa65a7b55, 0xcc3c4f43}
    static constexpr uint32_t kRgb8a1[2]          = {0x95938c6a, 0x0030e384};
    static constexpr uint32_t kExpectedRgb8a1[16] = {
        0x00000000, 0xffab697b, 0xffab697b, 0xffd6cba5, 0x00000000, 0x00000000,
        0xffab697b, 0xffd6cba5, 0xffab697b, 0x00000000, 0x00000000, 0xffab697b,
        0xffab697b, 0x00000000, 0xffab697b, 0x00000000,
    };

    static constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 position;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position.xy, 0.0, 1.0);\n"
        "}\n";
    ETCToBCTextureTest() : mEtcTexture(0u)
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
        glGenTextures(1, &mEtcTexture);
        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteTextures(1, &mEtcTexture); }
    GLuint mEtcTexture;
};

// Tests GPU compute transcode ETC2_RGB8 to BC1
TEST_P(ETCToBCTextureTest, ETC2Rgb8UnormToBC1_2D)
{
    ANGLE_SKIP_TEST_IF(
        !IsVulkan() || !IsNVIDIA() ||
        !getEGLWindow()->isFeatureEnabled(Feature::SupportsComputeTranscodeEtcToBc) ||
        !IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));
    glViewport(0, 0, kWidth, kHeight);
    glBindTexture(GL_TEXTURE_2D, mEtcTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGB8_ETC2, kTexSize, kTexSize);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTexSize, kTexSize, GL_COMPRESSED_RGB8_ETC2,
                              sizeof(kEtcRGBData), kEtcRGBData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    draw2DTexturedQuad(0.5f, 1.0f, false);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(kExpectedRGBColor[0]), kAbsError);
    EXPECT_PIXEL_COLOR_NEAR(1, 1, GLColor(kExpectedRGBColor[5]), kAbsError);
    EXPECT_PIXEL_COLOR_NEAR(2, 2, GLColor(kExpectedRGBColor[10]), kAbsError);
    EXPECT_PIXEL_COLOR_NEAR(3, 3, GLColor(kExpectedRGBColor[15]), kAbsError);
}

// Tests GPU compute transcode ETC2_RGB8 to BC1 with cube texture type
TEST_P(ETCToBCTextureTest, ETC2Rgb8UnormToBC1_Cube)
{
    ANGLE_SKIP_TEST_IF(
        !IsVulkan() || !IsNVIDIA() ||
        !getEGLWindow()->isFeatureEnabled(Feature::SupportsComputeTranscodeEtcToBc) ||
        !IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));
    glViewport(0, 0, kWidth, kHeight);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mEtcTexture);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_COMPRESSED_RGB8_ETC2, kTexSize, kTexSize);
    glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, kTexSize, kTexSize,
                              GL_COMPRESSED_RGB8_ETC2, sizeof(kEtcAllZero), kEtcAllZero);
    glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 0, 0, kTexSize, kTexSize,
                              GL_COMPRESSED_RGB8_ETC2, sizeof(kEtcAllZero), kEtcAllZero);
    glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 0, 0, kTexSize, kTexSize,
                              GL_COMPRESSED_RGB8_ETC2, sizeof(kEtcRGBData), kEtcRGBData);
    glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0, 0, kTexSize, kTexSize,
                              GL_COMPRESSED_RGB8_ETC2, sizeof(kEtcAllZero), kEtcAllZero);
    glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, 0, 0, kTexSize, kTexSize,
                              GL_COMPRESSED_RGB8_ETC2, sizeof(kEtcAllZero), kEtcAllZero);
    glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, 0, 0, kTexSize, kTexSize,
                              GL_COMPRESSED_RGB8_ETC2, sizeof(kEtcAllZero), kEtcAllZero);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    constexpr char kTextureCubeFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "uniform highp samplerCube texCube;\n"
        "uniform vec3 faceCoord;\n"
        "out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = texture(texCube, faceCoord);\n"
        "}\n";
    GLuint program = CompileProgram(kVS, kTextureCubeFS);
    glUseProgram(program);
    GLint texCubePos = glGetUniformLocation(program, "texCube");
    glUniform1i(texCubePos, 0);
    GLint faceCoordpos = glGetUniformLocation(program, "faceCoord");
    // sample at (2, 2) on y plane.
    glUniform3f(faceCoordpos, 0.25f, 1.0f, 0.25f);
    drawQuad(program, "position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(kExpectedRGBColor[10]), kAbsError);
}

// Tests GPU compute transcode ETC2_RGB8 to BC1 with 2DArray texture type
TEST_P(ETCToBCTextureTest, ETC2Rgb8UnormToBC1_2DArray)
{
    ANGLE_SKIP_TEST_IF(
        !IsVulkan() || !IsNVIDIA() ||
        !getEGLWindow()->isFeatureEnabled(Feature::SupportsComputeTranscodeEtcToBc) ||
        !IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));
    glViewport(0, 0, kWidth, kHeight);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mEtcTexture);
    static constexpr int kArraySize = 6;
    uint32_t data[2 * kArraySize]   = {
        kEtcAllZero[0], kEtcAllZero[1],  // array 0
        kEtcRGBData[0], kEtcRGBData[1],  // array 1
        kEtcRGBData[0], kEtcRGBData[1],  // array 2
        kEtcAllZero[0], kEtcAllZero[1],  // array 3
        kEtcAllZero[0], kEtcAllZero[1],  // array 4
        kEtcAllZero[0], kEtcAllZero[1],  // array 5
    };
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_COMPRESSED_RGB8_ETC2, kTexSize, kTexSize, kArraySize);
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, kTexSize, kTexSize, kArraySize,
                              GL_COMPRESSED_RGB8_ETC2, sizeof(data), data);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    constexpr char kTexture2DArrayFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "uniform highp sampler2DArray tex2DArray;\n"
        "uniform vec3 coord;\n"
        "out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = texture(tex2DArray, coord);\n"
        "}\n";
    GLuint program = CompileProgram(kVS, kTexture2DArrayFS);
    glUseProgram(program);
    GLint texCubePos = glGetUniformLocation(program, "tex2DArray");
    glUniform1i(texCubePos, 0);
    GLint faceCoordpos = glGetUniformLocation(program, "coord");
    // sample at (2, 2) on y plane.
    glUniform3f(faceCoordpos, 0.625f, 0.625f, 2.0f);
    drawQuad(program, "position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(kExpectedRGBColor[10]), kAbsError);
}

// Tests GPU compute transcode ETC2_RGB8 to BC1 with partial updates
TEST_P(ETCToBCTextureTest, ETC2Rgb8UnormToBC1_partial)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan() || !IsNVIDIA() ||
                       getEGLWindow()->isFeatureEnabled(Feature::SupportsComputeTranscodeEtcToBc) ||
                       !IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));
    glViewport(0, 0, kWidth, kHeight);
    glBindTexture(GL_TEXTURE_2D, mEtcTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGB8_ETC2, kTexSize * 3, kTexSize * 3);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTexSize, kTexSize, GL_COMPRESSED_RGB8_ETC2,
                              sizeof(kEtcAllZero), kEtcAllZero);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 2 * kTexSize, 2 * kTexSize, kTexSize, kTexSize,
                              GL_COMPRESSED_RGB8_ETC2, sizeof(kEtcRGBData), kEtcRGBData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "uniform highp sampler2D tex2D;\n"
        "uniform vec2 coord;\n"
        "out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = texture(tex2D, coord);\n"
        "}\n";
    GLuint program = CompileProgram(kVS, kFS);
    glUseProgram(program);
    GLint texCubePos = glGetUniformLocation(program, "tex2D");
    glUniform1i(texCubePos, 0);
    GLint faceCoordpos = glGetUniformLocation(program, "coord");
    // sample at (2, 2) on y plane.
    glUniform2f(faceCoordpos, 21.0f / 24.0f, 21.0f / 24.0f);
    drawQuad(program, "position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(kExpectedRGBColor[10]), kAbsError);
}

// Tests GPU compute transcode ETC2_RGBA8 to BC3
TEST_P(ETCToBCTextureTest, ETC2Rgba8UnormToBC3)
{
    ANGLE_SKIP_TEST_IF(
        !IsVulkan() || !IsNVIDIA() ||
        !getEGLWindow()->isFeatureEnabled(Feature::SupportsComputeTranscodeEtcToBc) ||
        !IsGLExtensionEnabled("GL_EXT_texture_compression_s3tc"));

    glViewport(0, 0, kWidth, kHeight);
    glBindTexture(GL_TEXTURE_2D, mEtcTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA8_ETC2_EAC, kTexSize, kTexSize);

    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTexSize, kTexSize,
                              GL_COMPRESSED_RGBA8_ETC2_EAC, sizeof(kEtcRGBAData), kEtcRGBAData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    draw2DTexturedQuad(0.5f, 1.0f, false);
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            EXPECT_PIXEL_COLOR_NEAR(j, i, GLColor(kExpectedRGBAColor[i * 4 + j]), kAbsError);
        }
    }
}

// Tests GPU compute transcode ETC2_RGBA8 to BC3 texture with lod.
TEST_P(ETCToBCTextureTest, ETC2Rgba8UnormToBC3_Lod)
{
    ANGLE_SKIP_TEST_IF(
        !IsVulkan() || !IsNVIDIA() ||
        !getEGLWindow()->isFeatureEnabled(Feature::SupportsComputeTranscodeEtcToBc) ||
        !IsGLExtensionEnabled("GL_EXT_texture_compression_s3tc"));

    glViewport(0, 0, kWidth, kHeight);
    glBindTexture(GL_TEXTURE_2D, mEtcTexture);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_COMPRESSED_RGBA8_ETC2_EAC, kTexSize, kTexSize);

    static constexpr uint32_t kAllZero[] = {0, 0, 0, 0};
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTexSize, kTexSize,
                              GL_COMPRESSED_RGBA8_ETC2_EAC, sizeof(kAllZero), kAllZero);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, kTexSize / 2, kTexSize / 2,
                              GL_COMPRESSED_RGBA8_ETC2_EAC, sizeof(kAllZero), kAllZero);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, kTexSize / 4, kTexSize / 4,
                              GL_COMPRESSED_RGBA8_ETC2_EAC, sizeof(kEtcRGBAData), kEtcRGBAData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec2 position;\n"
        "out vec2 texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position, 0, 1);\n"
        "    texCoord = position * 0.5 + vec2(0.5);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "uniform highp sampler2D tex2D;\n"
        "in vec2 texCoord;\n"
        "out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = textureLod(tex2D, texCoord, 2.0);\n"
        "}\n";
    GLuint program = CompileProgram(kVS, kFS);
    glUseProgram(program);
    GLint texPos = glGetUniformLocation(program, "tex2D");
    glUniform1i(texPos, 0);
    drawQuad(program, "position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(kExpectedRGBAColor[0]), kAbsError);
}

// Tests GPU compute transcode ETC2_SRGB8_ALPHA8 to BC3
TEST_P(ETCToBCTextureTest, ETC2SrgbAlpha8UnormToBC3)
{
    ANGLE_SKIP_TEST_IF(
        !IsVulkan() || !IsNVIDIA() ||
        !getEGLWindow()->isFeatureEnabled(Feature::SupportsComputeTranscodeEtcToBc) ||
        !IsGLExtensionEnabled("GL_EXT_texture_compression_s3tc_srgb"));

    glViewport(0, 0, pixel_width, pixel_height);
    glBindTexture(GL_TEXTURE_2D, mEtcTexture);
    EXPECT_GL_NO_ERROR();
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, pixel_width,
                           pixel_height, 0, sizeof(etc2bc_srgb8_alpha8), etc2bc_srgb8_alpha8);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    draw2DTexturedQuad(0.5f, 1.0f, false);
    EXPECT_PIXEL_ALPHA_NEAR(96, 160, 255, kAbsError);
    EXPECT_PIXEL_ALPHA_NEAR(88, 148, 0, kAbsError);
}

// Tests GPU compute transcode R11 Signed to BC4
TEST_P(ETCToBCTextureTest, ETC2R11SignedToBC4)
{
    ANGLE_SKIP_TEST_IF(
        !IsVulkan() || !IsNVIDIA() ||
        !getEGLWindow()->isFeatureEnabled(Feature::SupportsComputeTranscodeEtcToBc) ||
        !IsGLExtensionEnabled("GL_EXT_texture_compression_rgtc"));

    glViewport(0, 0, kWidth, kHeight);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mEtcTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_SIGNED_R11_EAC, kTexSize, kTexSize);

    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTexSize, kTexSize,
                              GL_COMPRESSED_SIGNED_R11_EAC, sizeof(kEacR11Signed), kEacR11Signed);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    constexpr char kVS[] =
        "attribute vec2 position;\n"
        "varying mediump vec2 texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position, 0, 1);\n"
        "    texCoord = position * 0.5 + vec2(0.5);\n"
        "}\n";

    constexpr char kFS[] =
        "varying mediump vec2 texCoord;\n"
        "uniform sampler2D tex;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = texture2D(tex, texCoord)/2.0 + vec4(0.5, 0.0, 0.0, 0.5);\n"
        "}\n";
    GLuint program = CompileProgram(kVS, kFS);
    glUseProgram(program);
    GLint pos = glGetUniformLocation(program, "tex");
    glUniform1i(pos, 0);
    drawQuad(program, "position", 0.5f);
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            EXPECT_PIXEL_COLOR_NEAR(j, i, GLColor(kExpectedR11SignedColor[i * 4 + j]), kAbsError);
        }
    }
}

// Tests GPU compute transcode RG11 to BC5
TEST_P(ETCToBCTextureTest, ETC2RG11ToBC5)
{
    ANGLE_SKIP_TEST_IF(
        !IsVulkan() || !IsNVIDIA() ||
        !getEGLWindow()->isFeatureEnabled(Feature::SupportsComputeTranscodeEtcToBc) ||
        !IsGLExtensionEnabled("GL_EXT_texture_compression_rgtc"));
    glViewport(0, 0, kWidth, kHeight);
    glBindTexture(GL_TEXTURE_2D, mEtcTexture);
    uint32_t data[4] = {
        kEtcRGBAData[0],
        kEtcRGBAData[1],
        kEtcRGBAData[0],
        kEtcRGBAData[1],
    };
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RG11_EAC, kTexSize, kTexSize);

    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTexSize, kTexSize, GL_COMPRESSED_RG11_EAC,
                              sizeof(data), data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    draw2DTexturedQuad(0.5f, 1.0f, false);
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            uint32_t color = (kExpectedRGBAColor[i * 4 + j] & 0xff000000) >> 24;
            color |= color << 8;
            color |= 0xff000000;
            EXPECT_PIXEL_COLOR_NEAR(j, i, GLColor(color), kAbsError);
        }
    }
}

// Tests GPU compute transcode RGB8A1 to BC1_RGBA
TEST_P(ETCToBCTextureTest, ETC2Rgb8a1UnormToBC1)
{
    ANGLE_SKIP_TEST_IF(
        !IsVulkan() || !IsNVIDIA() ||
        !getEGLWindow()->isFeatureEnabled(Feature::SupportsComputeTranscodeEtcToBc) ||
        !IsGLExtensionEnabled("GL_EXT_texture_compression_s3tc"));
    glViewport(0, 0, kWidth, kHeight);
    glBindTexture(GL_TEXTURE_2D, mEtcTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, kTexSize,
                   kTexSize);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTexSize, kTexSize,
                              GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, sizeof(kRgb8a1),
                              kRgb8a1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    draw2DTexturedQuad(0.5f, 1.0f, false);
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            EXPECT_PIXEL_COLOR_NEAR(j, i, GLColor(kExpectedRgb8a1[i * 4 + j]), kAbsError);
        }
    }
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(ETCTextureTest);
ANGLE_INSTANTIATE_TEST_ES3_AND(ETCToBCTextureTest,
                               ES3_VULKAN().enable(Feature::SupportsComputeTranscodeEtcToBc));
}  // anonymous namespace
