//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GetImageTest:
//   Tests for the ANGLE_get_image extension.
//

#include "image_util/storeimage.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

using namespace angle;

namespace
{
constexpr uint32_t kSize        = 32;
constexpr char kExtensionName[] = "GL_ANGLE_get_image";
constexpr uint32_t kSmallSize   = 2;
constexpr uint8_t kUNormZero    = 0x00;
constexpr uint8_t kUNormHalf    = 0x7F;
constexpr uint8_t kUNormFull    = 0xFF;

class GetImageTest : public ANGLETest<>
{
  public:
    GetImageTest()
    {
        setWindowWidth(kSize);
        setWindowHeight(kSize);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

class GetImageTestNoExtensions : public ANGLETest<>
{
  public:
    GetImageTestNoExtensions() { setExtensionsEnabled(false); }
};

class GetImageTestES1 : public GetImageTest
{
  public:
    GetImageTestES1() {}
};

class GetImageTestES3 : public GetImageTest
{
  public:
    GetImageTestES3() {}
};

class GetImageTestES31 : public GetImageTest
{
  public:
    GetImageTestES31() {}
};

class GetImageTestES32 : public GetImageTest
{
  public:
    GetImageTestES32() {}
};

GLTexture InitTextureWithFormatAndSize(GLenum format, uint32_t size, void *pixelData)
{
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, size, size, 0, format, GL_UNSIGNED_BYTE, pixelData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
}

GLTexture InitTextureWithSize(uint32_t size, void *pixelData)
{
    // Create a simple texture.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
}

GLTexture InitSimpleTexture()
{
    std::vector<GLColor> pixelData(kSize * kSize, GLColor::red);
    return InitTextureWithSize(kSize, pixelData.data());
}

GLRenderbuffer InitRenderbufferWithSize(uint32_t size)
{
    // Create a simple renderbuffer.
    GLRenderbuffer renderbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, size, size);
    return renderbuf;
}

GLRenderbuffer InitSimpleRenderbuffer()
{
    return InitRenderbufferWithSize(kSize);
}

// Test validation for the extension functions.
TEST_P(GetImageTest, NegativeAPI)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    // Draw once with simple texture.
    GLTexture tex = InitSimpleTexture();
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Pack pixels tightly.
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Verify GetTexImage can work with correct parameters.
    std::vector<GLColor> buffer(kSize * kSize);
    glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    EXPECT_GL_NO_ERROR();

    // Test invalid texture target.
    glGetTexImageANGLE(GL_TEXTURE_CUBE_MAP, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Test invalid texture level.
    glGetTexImageANGLE(GL_TEXTURE_2D, -1, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glGetTexImageANGLE(GL_TEXTURE_2D, 2000, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Test invalid format and type.
    glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_NONE, GL_UNSIGNED_BYTE, buffer.data());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_RGBA, GL_NONE, buffer.data());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Tests GetCompressed on an uncompressed texture.
    glGetCompressedTexImageANGLE(GL_TEXTURE_2D, 0, buffer.data());
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Create a simple renderbuffer.
    GLRenderbuffer renderbuf = InitSimpleRenderbuffer();
    ASSERT_GL_NO_ERROR();

    // Verify GetRenderbufferImage can work with correct parameters.
    glGetRenderbufferImageANGLE(GL_RENDERBUFFER, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    EXPECT_GL_NO_ERROR();

    // Test invalid renderbuffer target.
    glGetRenderbufferImageANGLE(GL_TEXTURE_2D, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Test invalid renderbuffer format/type.
    glGetRenderbufferImageANGLE(GL_RENDERBUFFER, GL_NONE, GL_UNSIGNED_BYTE, buffer.data());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glGetRenderbufferImageANGLE(GL_RENDERBUFFER, GL_RGBA, GL_NONE, buffer.data());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Pack buffer tests. Requires ES 3+ or extension.
    if (getClientMajorVersion() >= 3 || IsGLExtensionEnabled("GL_NV_pixel_buffer_object"))
    {
        // Test valid pack buffer.
        GLBuffer packBuffer;
        glBindBuffer(GL_PIXEL_PACK_BUFFER, packBuffer);
        glBufferData(GL_PIXEL_PACK_BUFFER, kSize * kSize * sizeof(GLColor), nullptr,
                     GL_STATIC_DRAW);
        glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        glGetRenderbufferImageANGLE(GL_RENDERBUFFER, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        // Test too small pack buffer.
        glBufferData(GL_PIXEL_PACK_BUFFER, kSize, nullptr, GL_STATIC_DRAW);
        glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        glGetRenderbufferImageANGLE(GL_RENDERBUFFER, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
}

// Simple test for GetTexImage
TEST_P(GetImageTest, GetTexImage)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    std::vector<GLColor> expectedData = {GLColor::red, GLColor::blue, GLColor::green,
                                         GLColor::yellow};

    glViewport(0, 0, kSmallSize, kSmallSize);

    // Draw once with simple texture.
    GLTexture tex = InitTextureWithSize(kSmallSize, expectedData.data());
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Pack pixels tightly.
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Verify GetImage.
    std::vector<GLColor> actualData(kSmallSize * kSmallSize);
    glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, actualData.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(expectedData, actualData);
}

// Simple cube map test for GetTexImage
TEST_P(GetImageTest, CubeMap)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    const std::array<std::array<GLColor, kSmallSize * kSmallSize>, kCubeFaces.size()> expectedData =
        {{
            {GLColor::red, GLColor::red, GLColor::red, GLColor::red},
            {GLColor::green, GLColor::green, GLColor::green, GLColor::green},
            {GLColor::blue, GLColor::blue, GLColor::blue, GLColor::blue},
            {GLColor::yellow, GLColor::yellow, GLColor::yellow, GLColor::yellow},
            {GLColor::cyan, GLColor::cyan, GLColor::cyan, GLColor::cyan},
            {GLColor::magenta, GLColor::magenta, GLColor::magenta, GLColor::magenta},
        }};

    GLTexture texture;
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    for (size_t faceIndex = 0; faceIndex < kCubeFaces.size(); ++faceIndex)
    {
        glTexImage2D(kCubeFaces[faceIndex], 0, GL_RGBA, kSmallSize, kSmallSize, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, expectedData[faceIndex].data());
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Pack pixels tightly.
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Verify GetImage.
    std::array<GLColor, kSmallSize *kSmallSize> actualData = {};
    for (size_t faceIndex = 0; faceIndex < kCubeFaces.size(); ++faceIndex)
    {
        glGetTexImageANGLE(kCubeFaces[faceIndex], 0, GL_RGBA, GL_UNSIGNED_BYTE, actualData.data());
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(expectedData[faceIndex], actualData);
    }
}

// Simple test for GetRenderbufferImage
TEST_P(GetImageTest, GetRenderbufferImage)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    std::vector<GLColor> expectedData = {GLColor::red, GLColor::blue, GLColor::green,
                                         GLColor::yellow};

    glViewport(0, 0, kSmallSize, kSmallSize);

    // Set up a simple Framebuffer with a Renderbuffer.
    GLRenderbuffer renderbuffer = InitRenderbufferWithSize(kSmallSize);
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw once with simple texture.
    GLTexture tex = InitTextureWithSize(kSmallSize, expectedData.data());
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Pack pixels tightly.
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Verify GetImage.
    std::vector<GLColor> actualData(kSmallSize * kSmallSize);
    glGetRenderbufferImageANGLE(GL_RENDERBUFFER, GL_RGBA, GL_UNSIGNED_BYTE, actualData.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(expectedData, actualData);
}

// Verifies that the extension enums and entry points are invalid when the extension is disabled.
TEST_P(GetImageTestNoExtensions, EntryPointsInactive)
{
    // Verify the extension is not enabled.
    ASSERT_FALSE(IsGLExtensionEnabled(kExtensionName));

    // Draw once with simple texture.
    GLTexture tex = InitSimpleTexture();
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Query implementation format and type. Should give invalid enum.
    GLint param;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_IMPLEMENTATION_COLOR_READ_FORMAT, &param);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glGetTexParameteriv(GL_TEXTURE_2D, GL_IMPLEMENTATION_COLOR_READ_TYPE, &param);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Verify calling GetTexImage produces an error.
    std::vector<GLColor> buffer(kSize * kSize, 0);
    glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Create a simple renderbuffer.
    GLRenderbuffer renderbuf = InitSimpleRenderbuffer();
    ASSERT_GL_NO_ERROR();

    // Query implementation format and type. Should give invalid enum.
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_IMPLEMENTATION_COLOR_READ_FORMAT, &param);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_IMPLEMENTATION_COLOR_READ_TYPE, &param);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Verify calling GetRenderbufferImage produces an error.
    glGetRenderbufferImageANGLE(GL_RENDERBUFFER, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test LUMINANCE_ALPHA (non-renderable) format with GetTexImage
TEST_P(GetImageTest, GetTexImageLuminanceAlpha)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    constexpr GLColorRG kMediumLumAlpha = GLColorRG(kUNormHalf, kUNormHalf);
    std::vector<GLColorRG> expectedData = {kMediumLumAlpha, kMediumLumAlpha, kMediumLumAlpha,
                                           kMediumLumAlpha};

    glViewport(0, 0, kSmallSize, kSmallSize);

    // Set up a simple LUMINANCE_ALPHA texture
    GLTexture tex =
        InitTextureWithFormatAndSize(GL_LUMINANCE_ALPHA, kSmallSize, expectedData.data());

    // Draw once with simple texture.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(kUNormHalf, kUNormHalf, kUNormHalf, kUNormHalf));
    ASSERT_GL_NO_ERROR();

    // Pack pixels tightly.
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Verify GetImage.
    std::vector<GLColorRG> actualData(kSmallSize * kSmallSize);
    glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, actualData.data());
    EXPECT_GL_NO_ERROR();
    for (uint32_t i = 0; i < kSmallSize * kSmallSize; ++i)
    {
        EXPECT_EQ(expectedData[i].R, actualData[i].R);
        EXPECT_EQ(expectedData[i].G, actualData[i].G);
    }
}

// Test LUMINANCE (non-renderable) format with GetTexImage
TEST_P(GetImageTest, GetTexImageLuminance)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    constexpr GLColorR kMediumLuminance = GLColorR(kUNormHalf);
    std::vector<GLColorR> expectedData  = {kMediumLuminance, kMediumLuminance, kMediumLuminance,
                                           kMediumLuminance};

    glViewport(0, 0, kSmallSize, kSmallSize);

    // Set up a simple LUMINANCE texture
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLTexture tex = InitTextureWithFormatAndSize(GL_LUMINANCE, kSmallSize, expectedData.data());

    // Draw once with simple texture.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(kUNormHalf, kUNormHalf, kUNormHalf, kUNormFull));
    ASSERT_GL_NO_ERROR();

    // Pack pixels tightly.
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Verify GetImage.
    std::vector<GLColorR> actualData(kSmallSize * kSmallSize);
    glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, actualData.data());
    EXPECT_GL_NO_ERROR();
    for (uint32_t i = 0; i < kSmallSize * kSmallSize; ++i)
    {
        EXPECT_EQ(expectedData[i].R, actualData[i].R);
    }
}

// Test ALPHA (non-renderable) format with GetTexImage
TEST_P(GetImageTest, GetTexImageAlpha)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    constexpr GLColorR kMediumAlpha    = GLColorR(kUNormHalf);
    std::vector<GLColorR> expectedData = {kMediumAlpha, kMediumAlpha, kMediumAlpha, kMediumAlpha};

    glViewport(0, 0, kSmallSize, kSmallSize);

    // Set up a simple ALPHA texture
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GLTexture tex = InitTextureWithFormatAndSize(GL_ALPHA, kSmallSize, expectedData.data());

    // Draw once with simple texture
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(kUNormZero, kUNormZero, kUNormZero, kUNormHalf));
    ASSERT_GL_NO_ERROR();

    // Pack pixels tightly.
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Verify we get back the correct pixels from GetTexImage
    std::vector<GLColorR> actualData(kSmallSize * kSmallSize);
    glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_ALPHA, GL_UNSIGNED_BYTE, actualData.data());
    EXPECT_GL_NO_ERROR();
    for (uint32_t i = 0; i < kSmallSize * kSmallSize; ++i)
    {
        EXPECT_EQ(expectedData[i].R, actualData[i].R);
    }
}

// Tests GetImage behaviour with an RGB image.
TEST_P(GetImageTest, GetImageRGB)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    std::vector<GLColorRGB> expectedData = {GLColorRGB::red, GLColorRGB::blue, GLColorRGB::green,
                                            GLColorRGB::yellow};

    glViewport(0, 0, kSmallSize, kSmallSize);

    // Pack pixels tightly.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Init simple texture.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, kSmallSize, kSmallSize, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 expectedData.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGB, kSmallSize / 2, kSmallSize / 2, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, expectedData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Verify GetImage.
    std::vector<GLColorRGB> actualData(kSmallSize * kSmallSize);
    glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, actualData.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(expectedData, actualData);

    // Draw after the GetImage.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5, 1.0f, true);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::yellow);
}

// Tests GetImage with 2D array textures.
TEST_P(GetImageTestES31, Texture2DArray)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    constexpr GLsizei kTextureSize = 2;
    constexpr GLsizei kLayers      = 4;

    std::vector<GLColor> expectedPixels = {
        GLColor::red,    GLColor::red,    GLColor::red,    GLColor::red,
        GLColor::green,  GLColor::green,  GLColor::green,  GLColor::green,
        GLColor::blue,   GLColor::blue,   GLColor::blue,   GLColor::blue,
        GLColor::yellow, GLColor::yellow, GLColor::yellow, GLColor::yellow,
    };

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, kTextureSize, kTextureSize, kLayers, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, expectedPixels.data());
    ASSERT_GL_NO_ERROR();

    std::vector<GLColor> actualPixels(expectedPixels.size(), GLColor::white);
    glGetTexImageANGLE(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_UNSIGNED_BYTE, actualPixels.data());
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(expectedPixels, actualPixels);
}

// Tests GetImage with 3D textures.
TEST_P(GetImageTestES31, Texture3D)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    constexpr GLsizei kTextureSize = 2;

    std::vector<GLColor> expectedPixels = {
        GLColor::red,  GLColor::red,  GLColor::green,  GLColor::green,
        GLColor::blue, GLColor::blue, GLColor::yellow, GLColor::yellow,
    };

    GLTexture tex;
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, kTextureSize, kTextureSize, kTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, expectedPixels.data());
    ASSERT_GL_NO_ERROR();

    std::vector<GLColor> actualPixels(expectedPixels.size(), GLColor::white);
    glGetTexImageANGLE(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, actualPixels.data());
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(expectedPixels, actualPixels);
}

// Tests GetImage with cube map array textures.
TEST_P(GetImageTestES31, TextureCubeMapArray)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_cube_map_array") &&
                       !IsGLExtensionEnabled("GL_OES_texture_cube_map_array"));

    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    constexpr GLsizei kTextureSize = 1;
    constexpr GLsizei kLayers      = 2;

    std::vector<GLColor> expectedPixels = {
        GLColor::red,  GLColor::green,   GLColor::blue, GLColor::yellow,
        GLColor::cyan, GLColor::magenta, GLColor::red,  GLColor::green,
        GLColor::blue, GLColor::yellow,  GLColor::cyan, GLColor::magenta,
    };

    ASSERT_EQ(expectedPixels.size(),
              static_cast<size_t>(6 * kTextureSize * kTextureSize * kLayers));

    GLTexture tex;
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, tex);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGBA, kTextureSize, kTextureSize, kLayers * 6, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, expectedPixels.data());
    ASSERT_GL_NO_ERROR();

    std::vector<GLColor> actualPixels(expectedPixels.size(), GLColor::white);
    glGetTexImageANGLE(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                       actualPixels.data());
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(expectedPixels, actualPixels);
}

// Tests GetImage with an inconsistent 2D texture.
TEST_P(GetImageTest, InconsistentTexture2D)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    std::vector<GLColor> expectedData = {GLColor::red, GLColor::blue, GLColor::green,
                                         GLColor::yellow};

    glViewport(0, 0, kSmallSize, kSmallSize);

    // Draw once with simple texture.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSmallSize, kSmallSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 expectedData.data());
    // The texture becomes inconsistent because a second 2x2 image does not fit in the mip chain.
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, kSmallSize, kSmallSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 expectedData.data());

    // Pack pixels tightly.
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // Verify GetImage.
    std::vector<GLColor> actualData(kSmallSize * kSmallSize, GLColor::white);
    glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, actualData.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(expectedData, actualData);

    std::fill(actualData.begin(), actualData.end(), GLColor::white);
    glGetTexImageANGLE(GL_TEXTURE_2D, 1, GL_RGBA, GL_UNSIGNED_BYTE, actualData.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(expectedData, actualData);
}

// Test GetImage with non-defined textures.
TEST_P(GetImageTest, EmptyTexture)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);

    std::vector<GLColor> expectedData(4, GLColor::white);
    std::vector<GLColor> actualData(4, GLColor::white);
    glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, actualData.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(expectedData, actualData);
}

struct CompressedFormat
{
    const GLenum id;

    // Texel/Block size in bytes
    const GLsizei size;

    // Texel/Block dimensions
    const GLsizei w;
    const GLsizei h;
};

struct CompressionExtension
{
    const char *name;
    const std::vector<CompressedFormat> formats;
    const bool supports2DArray;
    const bool supports3D;
};

// clang-format off
const CompressionExtension kCompressionExtensions[] = {
    // BC / DXT
    {"GL_EXT_texture_compression_dxt1",      {{GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 8, 4, 4},
                                              {GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8, 4, 4}},
                                             true, false},
    {"GL_ANGLE_texture_compression_dxt3",    {{GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 16, 4, 4}},
                                             true, false},
    {"GL_ANGLE_texture_compression_dxt5",    {{GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 16, 4, 4}},
                                             true, false},
    {"GL_EXT_texture_compression_s3tc_srgb", {{GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, 8, 4, 4},
                                              {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 8, 4, 4},
                                              {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 16, 4, 4},
                                              {GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 16, 4, 4}},
                                             true, false},
    {"GL_EXT_texture_compression_rgtc",      {{GL_COMPRESSED_RED_RGTC1_EXT, 8, 4, 4},
                                              {GL_COMPRESSED_SIGNED_RED_RGTC1_EXT, 8, 4, 4},
                                              {GL_COMPRESSED_RED_GREEN_RGTC2_EXT, 16, 4, 4},
                                              {GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT, 16, 4, 4}},
                                             true, false},
    {"GL_EXT_texture_compression_bptc",      {{GL_COMPRESSED_RGBA_BPTC_UNORM_EXT, 16, 4, 4},
                                              {GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT, 16, 4, 4},
                                              {GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT, 16, 4, 4},
                                              {GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT, 16, 4, 4}},
                                             true, true},

    // ETC
    {"GL_OES_compressed_ETC1_RGB8_texture",                      {{GL_ETC1_RGB8_OES, 8, 4, 4}},
                                                                 false, false},
    {"GL_OES_compressed_EAC_R11_unsigned_texture",               {{GL_COMPRESSED_R11_EAC, 8, 4, 4}},
                                                                 true, false},
    {"GL_OES_compressed_EAC_R11_signed_texture",                 {{GL_COMPRESSED_SIGNED_R11_EAC, 8, 4, 4}},
                                                                 true, false},
    {"GL_OES_compressed_EAC_RG11_unsigned_texture",              {{GL_COMPRESSED_RG11_EAC, 16, 4, 4}},
                                                                 true, false},
    {"GL_OES_compressed_EAC_RG11_signed_texture",                {{GL_COMPRESSED_SIGNED_RG11_EAC, 16, 4, 4}},
                                                                 true, false},
    {"GL_OES_compressed_ETC2_RGB8_texture",                      {{GL_COMPRESSED_RGB8_ETC2, 8, 4, 4}},
                                                                 true, false},
    {"GL_OES_compressed_ETC2_sRGB8_texture",                     {{GL_COMPRESSED_SRGB8_ETC2, 8, 4, 4}},
                                                                 true, false},
    {"GL_OES_compressed_ETC2_punchthroughA_RGBA8_texture",       {{GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, 8, 4, 4}},
                                                                 true, false},
    {"GL_OES_compressed_ETC2_punchthroughA_sRGB8_alpha_texture", {{GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, 8, 4, 4}},
                                                                 true, false},
    {"GL_OES_compressed_ETC2_RGBA8_texture",                     {{GL_COMPRESSED_RGBA8_ETC2_EAC, 16, 4, 4}},
                                                                 true, false},
    {"GL_OES_compressed_ETC2_sRGB8_alpha8_texture",              {{GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, 16, 4, 4}},
                                                                 true, false},

    // ASTC
    {"GL_KHR_texture_compression_astc_ldr", {{GL_COMPRESSED_RGBA_ASTC_4x4_KHR, 16, 4, 4},
                                             {GL_COMPRESSED_RGBA_ASTC_5x4_KHR, 16, 5, 4},
                                             {GL_COMPRESSED_RGBA_ASTC_5x5_KHR, 16, 5, 5},
                                             {GL_COMPRESSED_RGBA_ASTC_6x5_KHR, 16, 6, 5},
                                             {GL_COMPRESSED_RGBA_ASTC_6x6_KHR, 16, 6, 6},
                                             {GL_COMPRESSED_RGBA_ASTC_8x5_KHR, 16, 8, 5},
                                             {GL_COMPRESSED_RGBA_ASTC_8x6_KHR, 16, 8, 6},
                                             {GL_COMPRESSED_RGBA_ASTC_8x8_KHR, 16, 8, 8},
                                             {GL_COMPRESSED_RGBA_ASTC_10x5_KHR, 16, 10, 5},
                                             {GL_COMPRESSED_RGBA_ASTC_10x6_KHR, 16, 10, 6},
                                             {GL_COMPRESSED_RGBA_ASTC_10x8_KHR, 16, 10, 8},
                                             {GL_COMPRESSED_RGBA_ASTC_10x10_KHR, 16, 10, 10},
                                             {GL_COMPRESSED_RGBA_ASTC_12x10_KHR, 16, 12, 10},
                                             {GL_COMPRESSED_RGBA_ASTC_12x12_KHR, 16, 12, 12},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR, 16, 4, 4},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR, 16, 5, 4},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR, 16, 5, 5},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR, 16, 6, 5},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR, 16, 6, 6},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR, 16, 8, 5},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR, 16, 8, 6},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR, 16, 8, 8},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR, 16, 10, 5},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR, 16, 10, 6},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR, 16, 10, 8},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 16, 10, 10},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 16, 12, 10},
                                             {GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 16, 12, 12}},
                                             true, true},
};
// clang-format on

// Basic GetCompressedTexImage.
TEST_P(GetImageTest, CompressedTexImage)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_compressed_ETC1_RGB8_texture"));

    constexpr GLsizei kRes       = 4;
    constexpr GLsizei kImageSize = 8;

    // This arbitrary 'compressed' data just has to be read back exactly as specified below.
    constexpr std::array<uint8_t, kImageSize> kExpectedData = {1, 2, 3, 4, 5, 6, 7, 8};

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_ETC1_RGB8_OES, kRes, kRes, 0, kImageSize,
                           kExpectedData.data());

    if (IsFormatEmulated(GL_TEXTURE_2D))
    {
        INFO()
            << "Skipping emulated format GL_ETC1_RGB8_OES from GL_OES_compressed_ETC1_RGB8_texture";
        return;
    }

    std::array<uint8_t, kImageSize> actualData = {};
    glGetCompressedTexImageANGLE(GL_TEXTURE_2D, 0, actualData.data());
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(kExpectedData, actualData);
}

// Test validation for the compressed extension function.
TEST_P(GetImageTest, CompressedTexImageNegativeAPI)
{
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    // Verify the extension is enabled.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_compressed_ETC2_RGB8_texture"));

    constexpr GLsizei kRes       = 4;
    constexpr GLsizei kImageSize = 8;

    // This arbitrary 'compressed' data just has to be read back exactly as specified below.
    constexpr std::array<uint8_t, kImageSize> kExpectedData = {1, 2, 3, 4, 5, 6, 7, 8};

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, kRes, kRes, 0, kImageSize,
                           kExpectedData.data());

    std::array<uint8_t, kImageSize> actualData = {};
    glGetCompressedTexImageANGLE(GL_TEXTURE_2D, 0, actualData.data());

    // Verify GetTexImage works with correct parameters or fails if format is emulated.
    if (IsFormatEmulated(GL_TEXTURE_2D))
    {
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
    else
    {
        EXPECT_GL_NO_ERROR();
    }

    // Test invalid texture target.
    glGetCompressedTexImageANGLE(GL_TEXTURE_CUBE_MAP, 0, actualData.data());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Test invalid texture level.
    glGetCompressedTexImageANGLE(GL_TEXTURE_2D, -1, actualData.data());
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glGetCompressedTexImageANGLE(GL_TEXTURE_2D, 2000, actualData.data());
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Tests GetTexImage on a compressed texture.
    if (!IsFormatEmulated(GL_TEXTURE_2D))
    {
        glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, GL_UNSIGNED_BYTE,
                           actualData.data());
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
}

using TestFormatFunction =
    std::function<void(const CompressionExtension &, const CompressedFormat &)>;

void TestAllCompressedFormats(TestFormatFunction fun)
{
    for (CompressionExtension ext : kCompressionExtensions)
    {
        if (!IsGLExtensionEnabled(ext.name))
        {
            continue;
        }

        for (CompressedFormat format : ext.formats)
        {
            fun(ext, format);
        }
    }
}

// Test GetCompressedTexImage with all formats and
// and multiple resolution of the format's block size.
TEST_P(GetImageTest, CompressedTexImageAll)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    auto func = [](const CompressionExtension &ext, const CompressedFormat &format) {
        // Test with multiples of block size
        constexpr std::array<GLsizei, 2> multipliers = {1, 2};
        for (GLsizei multiplier : multipliers)
        {
            const GLsizei kImageSize = format.size * multiplier * multiplier;

            std::vector<uint8_t> expectedData;
            for (uint8_t i = 1; i < kImageSize + 1; i++)
            {
                expectedData.push_back(i);
            }

            GLTexture tex;
            glBindTexture(GL_TEXTURE_2D, tex);
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, format.id, format.w * multiplier,
                                   format.h * multiplier, 0, kImageSize, expectedData.data());

            if (IsFormatEmulated(GL_TEXTURE_2D))
            {
                INFO() << "Skipping emulated format 0x" << std::hex << format.id << " from "
                       << ext.name;
                return;
            }

            std::vector<uint8_t> actualData(kImageSize);
            glGetCompressedTexImageANGLE(GL_TEXTURE_2D, 0, actualData.data());

            ASSERT_GL_NO_ERROR();
            EXPECT_EQ(expectedData, actualData);
        }
    };
    TestAllCompressedFormats(func);
}

// Test a resolution that is not a multiple of the block size with an ETC2 4x4 format.
TEST_P(GetImageTest, CompressedTexImageNotBlockMultiple)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_compressed_ETC2_RGB8_texture"));

    constexpr GLsizei kRes       = 21;
    constexpr GLsizei kImageSize = 288;

    // This arbitrary 'compressed' data just has to be read back exactly as specified below.
    std::vector<uint8_t> expectedData;
    for (uint16_t j = 0; j < kImageSize; j++)
    {
        expectedData.push_back(j % std::numeric_limits<uint8_t>::max());
    }

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, kRes, kRes, 0, kImageSize,
                           expectedData.data());

    if (IsFormatEmulated(GL_TEXTURE_2D))
    {
        INFO() << "Skipping emulated format GL_COMPRESSED_RGB8_ETC2 from "
                  "GL_OES_compressed_ETC2_RGB8_texture";
        return;
    }

    std::vector<uint8_t> actualData(kImageSize);
    glGetCompressedTexImageANGLE(GL_TEXTURE_2D, 0, actualData.data());

    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(expectedData, actualData);
}

void TestCompressedTexImage3D(GLenum target, uint32_t numLayers)
{
    auto func = [target, numLayers](const CompressionExtension &ext,
                                    const CompressedFormat &format) {
        // Skip extensions lacking 2D array and 3D support
        if ((target == GL_TEXTURE_2D_ARRAY && !ext.supports2DArray) ||
            (target == GL_TEXTURE_3D && !ext.supports3D))
        {
            return;
        }

        // GL_TEXTURE_3D with ASTC requires additional extension
        if (target == GL_TEXTURE_3D &&
            strcmp(ext.name, "GL_KHR_texture_compression_astc_ldr") == 0 &&
            !IsGLExtensionEnabled("GL_KHR_texture_compression_astc_sliced_3d") &&
            !IsGLExtensionEnabled("GL_KHR_texture_compression_astc_hdr"))
        {
            return;
        }

        const size_t size = format.size * numLayers;

        GLTexture texture;
        glBindTexture(target, texture);

        std::vector<uint8_t> expectedData;
        for (uint8_t i = 0; i < size; i++)
        {
            expectedData.push_back(i);
        }

        glCompressedTexImage3D(target, 0, format.id, format.w, format.h, numLayers, 0, size,
                               expectedData.data());

        if (IsFormatEmulated(target))
        {
            INFO() << "Skipping emulated format 0x" << std::hex << format.id << " from "
                   << ext.name;
            return;
        }

        std::vector<uint8_t> actualData(size);
        glGetCompressedTexImageANGLE(target, 0, actualData.data());
        ASSERT_GL_NO_ERROR();
        EXPECT_EQ(expectedData, actualData);
    };
    TestAllCompressedFormats(func);
}

// Tests GetCompressedTexImage with 2D array textures.
TEST_P(GetImageTestES3, CompressedTexImage2DArray)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));
    TestCompressedTexImage3D(GL_TEXTURE_2D_ARRAY, 8);
}

// Tests GetCompressedTexImage with 3D textures.
TEST_P(GetImageTest, CompressedTexImage3D)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));
    TestCompressedTexImage3D(GL_TEXTURE_3D, 8);
}

// Simple cube map test for GetCompressedTexImage
TEST_P(GetImageTest, CompressedTexImageCubeMap)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    auto func = [](const CompressionExtension &ext, const CompressedFormat &format) {
        GLTexture texture;
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

        std::vector<std::vector<uint8_t>> expectedData;
        for (uint32_t i = 0; i < kCubeFaces.size(); i++)
        {
            std::vector<uint8_t> face;
            for (uint8_t j = 0; j < format.size; j++)
            {
                face.push_back(static_cast<uint8_t>((i * format.size + j) %
                                                    std::numeric_limits<uint8_t>::max()));
            }
            expectedData.push_back(face);
        }

        for (size_t faceIndex = 0; faceIndex < kCubeFaces.size(); ++faceIndex)
        {
            glCompressedTexImage2D(kCubeFaces[faceIndex], 0, format.id, 4, 4, 0, format.size,
                                   expectedData[faceIndex].data());
        }

        if (IsFormatEmulated(GL_TEXTURE_CUBE_MAP))
        {
            INFO() << "Skipping emulated format 0x" << std::hex << format.id << " from "
                   << ext.name;
            return;
        }

        // Verify GetImage.
        for (size_t faceIndex = 0; faceIndex < kCubeFaces.size(); ++faceIndex)
        {
            std::vector<uint8_t> actualData(format.size);
            glGetCompressedTexImageANGLE(kCubeFaces[faceIndex], 0, actualData.data());
            EXPECT_GL_NO_ERROR();
            EXPECT_EQ(expectedData[faceIndex], actualData);
        }
    };
    TestAllCompressedFormats(func);
}

// Tests GetCompressedTexImage with cube map array textures.
TEST_P(GetImageTestES32, CompressedTexImageCubeMapArray)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    auto func = [](const CompressionExtension &ext, const CompressedFormat &format) {
        std::vector<uint8_t> expectedData;
        for (uint32_t i = 0; i < format.size * kCubeFaces.size(); i++)
        {
            expectedData.push_back(static_cast<uint8_t>(i % std::numeric_limits<uint8_t>::max()));
        }

        GLTexture tex;
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, tex);
        glCompressedTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, format.id, 4, 4, kCubeFaces.size(), 0,
                               expectedData.size() * sizeof(uint8_t), expectedData.data());
        ASSERT_GL_NO_ERROR();

        if (IsFormatEmulated(GL_TEXTURE_CUBE_MAP_ARRAY))
        {
            INFO() << "Skipping emulated format 0x" << std::hex << format.id << " from "
                   << ext.name;
            return;
        }

        // Verify GetImage.
        std::vector<uint8_t> actualData(format.size * kCubeFaces.size());
        glGetCompressedTexImageANGLE(GL_TEXTURE_CUBE_MAP_ARRAY, 0, actualData.data());
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(expectedData, actualData);
    };
    TestAllCompressedFormats(func);
}

// Tests GetCompressedTexImage with multiple mip levels.
TEST_P(GetImageTest, CompressedTexImageMultiLevel)
{
    // Verify the extension is enabled.
    ASSERT_TRUE(IsGLExtensionEnabled(kExtensionName));

    auto func = [](const CompressionExtension &ext, const CompressedFormat &format) {
        constexpr uint8_t kNumMipLevels = 8;
        GLTexture tex;
        glBindTexture(GL_TEXTURE_2D, tex);

        std::vector<std::vector<uint8_t>> expectedData;
        for (uint32_t mipLevel = 0; mipLevel < kNumMipLevels; mipLevel++)
        {
            uint32_t multiplier = static_cast<uint32_t>(pow(2, (kNumMipLevels - mipLevel) - 1));
            size_t levelSize    = format.size * multiplier * multiplier;

            std::vector<uint8_t> levelData;
            for (size_t j = 0; j < levelSize; j++)
            {
                levelData.push_back(static_cast<uint8_t>(j % std::numeric_limits<uint8_t>::max()));
            }
            expectedData.push_back(levelData);

            glCompressedTexImage2D(GL_TEXTURE_2D, mipLevel, format.id, format.w * multiplier,
                                   format.h * multiplier, 0, levelSize, levelData.data());
        }

        ASSERT_GL_NO_ERROR();

        if (IsFormatEmulated(GL_TEXTURE_2D))
        {
            INFO() << "Skipping emulated format 0x" << std::hex << format.id << " from "
                   << ext.name;
            return;
        }

        for (uint32_t mipLevel = 0; mipLevel < kNumMipLevels; mipLevel++)
        {
            uint32_t multiplier = static_cast<uint32_t>(pow(2, (kNumMipLevels - mipLevel) - 1));
            size_t levelSize    = format.size * multiplier * multiplier;

            std::vector<uint8_t> actualData(levelSize);
            glGetCompressedTexImageANGLE(GL_TEXTURE_2D, mipLevel, actualData.data());

            ASSERT_GL_NO_ERROR();
            EXPECT_EQ(expectedData[mipLevel], actualData);
        }
    };
    TestAllCompressedFormats(func);
}

struct PalettedFormat
{
    GLenum internalFormat;
    uint32_t indexBits;
    uint32_t redBlueBits;
    uint32_t greenBits;
    uint32_t alphaBits;
};

const PalettedFormat kPalettedFormats[] = {
    {GL_PALETTE4_RGB8_OES, 4, 8, 8, 0},     {GL_PALETTE4_RGBA8_OES, 4, 8, 8, 8},
    {GL_PALETTE4_R5_G6_B5_OES, 4, 5, 6, 0}, {GL_PALETTE4_RGBA4_OES, 4, 4, 4, 4},
    {GL_PALETTE4_RGB5_A1_OES, 4, 5, 5, 1},  {GL_PALETTE8_RGB8_OES, 8, 8, 8, 0},
    {GL_PALETTE8_RGBA8_OES, 8, 8, 8, 8},    {GL_PALETTE8_R5_G6_B5_OES, 8, 5, 6, 0},
    {GL_PALETTE8_RGBA4_OES, 8, 4, 4, 4},    {GL_PALETTE8_RGB5_A1_OES, 8, 5, 5, 1},
};

// Tests that glGetTexImageANGLE captures paletted format images in
// R8G8B8A8_UNORM and StoreRGBA8ToPalettedImpl compresses such images to the
// same paletted image.
TEST_P(GetImageTestES1, PalettedTexImage)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    struct Vertex
    {
        GLfloat position[3];
        GLfloat uv[2];
    };

    std::vector<Vertex> vertices = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    };
    glVertexPointer(3, GL_FLOAT, sizeof vertices[0], &vertices[0].position);
    glTexCoordPointer(2, GL_FLOAT, sizeof vertices[0], &vertices[0].uv);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Locations in the framebuffer where we will take the samples to compare

    const int width  = 16;
    const int height = width;

    for (PalettedFormat format : kPalettedFormats)
    {
        size_t paletteSizeInBytes =
            (1 << format.indexBits) *
            ((format.redBlueBits + format.greenBits + format.redBlueBits + format.alphaBits) / 8);
        size_t imageSize = paletteSizeInBytes + width * height * format.indexBits / 8;
        std::vector<uint8_t> testImage(imageSize);
        FillVectorWithRandomUBytes(&testImage);

        // Upload a random paletted image

        glCompressedTexImage2D(GL_TEXTURE_2D, 0, format.internalFormat, width, height, 0,
                               testImage.size(), testImage.data());
        EXPECT_GL_NO_ERROR();

        // Render a quad using this texture and capture the entire framebuffer contents

        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size());
        EXPECT_GL_NO_ERROR();

        std::vector<GLColor> framebuffer(width * height);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer.data());

        // Read it back as R8G8B8A8 UNORM

        std::vector<uint8_t> readback(width * height * 4);
        glGetTexImageANGLE(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, readback.data());
        EXPECT_GL_NO_ERROR();

        // Recompress. This should produce the same, up to permutation of used
        // palette entries, image, as the one we uploaded. Upload it again.

        std::vector<uint8_t> recompressedImage(imageSize);
        angle::StoreRGBA8ToPalettedImpl(width, height, 1, format.indexBits, format.redBlueBits,
                                        format.greenBits, format.alphaBits, readback.data(),
                                        width * 4, width * height * 4, recompressedImage.data(),
                                        width * format.indexBits / 8,
                                        width * height * format.indexBits / 8);

        glCompressedTexImage2D(GL_TEXTURE_2D, 0, format.internalFormat, width, height, 0,
                               recompressedImage.size(), recompressedImage.data());
        EXPECT_GL_NO_ERROR();

        // Read the framebuffer again

        std::vector<GLColor> framebuffer2(width * height);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer2.data());

        EXPECT_EQ(framebuffer, framebuffer2);
    }
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GetImageTest);
ANGLE_INSTANTIATE_TEST(GetImageTest,
                       ES2_VULKAN(),
                       ES3_VULKAN(),
                       ES2_VULKAN_SWIFTSHADER(),
                       ES3_VULKAN_SWIFTSHADER());

ANGLE_INSTANTIATE_TEST_ES1(GetImageTestES1);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GetImageTestES3);
ANGLE_INSTANTIATE_TEST(GetImageTestES3, ES3_VULKAN(), ES3_VULKAN_SWIFTSHADER());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GetImageTestES31);
ANGLE_INSTANTIATE_TEST(GetImageTestES31, ES31_VULKAN(), ES31_VULKAN_SWIFTSHADER());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GetImageTestES32);
ANGLE_INSTANTIATE_TEST(GetImageTestES32, ES32_VULKAN(), ES32_VULKAN_SWIFTSHADER());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GetImageTestNoExtensions);
ANGLE_INSTANTIATE_TEST(GetImageTestNoExtensions,
                       ES2_VULKAN(),
                       ES3_VULKAN(),
                       ES2_VULKAN_SWIFTSHADER(),
                       ES3_VULKAN_SWIFTSHADER());

}  // namespace