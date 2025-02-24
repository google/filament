//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

namespace angle
{

// These two colors are equivelent in different colorspaces
constexpr GLColor kLinearColor(64, 127, 191, 255);
constexpr GLColor kNonlinearColor(13, 54, 133, 255);

class SRGBTextureTest : public ANGLETest<>
{
  protected:
    SRGBTextureTest()
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
        constexpr char kVS[] =
            "precision highp float;\n"
            "attribute vec4 position;\n"
            "varying vec2 texcoord;\n"
            "\n"
            "void main()\n"
            "{\n"
            "   gl_Position = vec4(position.xy, 0.0, 1.0);\n"
            "   texcoord = (position.xy * 0.5) + 0.5;\n"
            "}\n";

        constexpr char kFS[] =
            "precision highp float;\n"
            "uniform sampler2D tex;\n"
            "varying vec2 texcoord;\n"
            "\n"
            "void main()\n"
            "{\n"
            "   gl_FragColor = texture2D(tex, texcoord);\n"
            "}\n";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram);

        mTextureLocation = glGetUniformLocation(mProgram, "tex");
        ASSERT_NE(-1, mTextureLocation);
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    GLenum getSRGBA8TextureInternalFormat() const
    {
        return getClientMajorVersion() >= 3 ? GL_SRGB8_ALPHA8 : GL_SRGB_ALPHA_EXT;
    }

    GLenum getSRGBA8TextureFormat() const
    {
        return getClientMajorVersion() >= 3 ? GL_RGBA : GL_SRGB_ALPHA_EXT;
    }

    GLenum getSRGB8TextureInternalFormat() const
    {
        return getClientMajorVersion() >= 3 ? GL_SRGB8 : GL_SRGB_EXT;
    }

    GLenum getSRGB8TextureFormat() const
    {
        return getClientMajorVersion() >= 3 ? GL_RGB : GL_SRGB_EXT;
    }

    GLuint mProgram        = 0;
    GLint mTextureLocation = -1;
};

class SRGBTextureTestES3 : public SRGBTextureTest
{};

// GenerateMipmaps should generate INVALID_OPERATION in ES 2.0 / WebGL 1.0 with EXT_sRGB.
// https://bugs.chromium.org/p/chromium/issues/detail?id=769989
TEST_P(SRGBTextureTest, SRGBValidation)
{
    // TODO(fjhenigman): Figure out why this fails on Ozone Intel.
    ANGLE_SKIP_TEST_IF(IsOzone() && IsIntel() && IsOpenGLES());

    bool supported = IsGLExtensionEnabled("GL_EXT_sRGB") || getClientMajorVersion() == 3;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    GLubyte pixel[3] = {0};
    glTexImage2D(GL_TEXTURE_2D, 0, getSRGB8TextureInternalFormat(), 1, 1, 0,
                 getSRGB8TextureFormat(), GL_UNSIGNED_BYTE, pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, getSRGB8TextureFormat(), GL_UNSIGNED_BYTE,
                        pixel);
        EXPECT_GL_NO_ERROR();

        // Mipmap generation always generates errors for SRGB unsized in ES2 or SRGB8 sized in ES3.
        glGenerateMipmap(GL_TEXTURE_2D);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    glDeleteTextures(1, &tex);
}

TEST_P(SRGBTextureTest, SRGBAValidation)
{
    // TODO(fjhenigman): Figure out why this fails on Ozone Intel.
    ANGLE_SKIP_TEST_IF(IsOzone() && IsIntel() && IsOpenGLES());

    bool supported = IsGLExtensionEnabled("GL_EXT_sRGB") || getClientMajorVersion() == 3;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    GLubyte pixel[4] = {0};
    glTexImage2D(GL_TEXTURE_2D, 0, getSRGBA8TextureInternalFormat(), 1, 1, 0,
                 getSRGBA8TextureFormat(), GL_UNSIGNED_BYTE, pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, getSRGBA8TextureFormat(), GL_UNSIGNED_BYTE,
                        pixel);
        EXPECT_GL_NO_ERROR();

        glGenerateMipmap(GL_TEXTURE_2D);
        if (getClientMajorVersion() < 3)
        {
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        }
        else
        {
            EXPECT_GL_NO_ERROR();
        }
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    glDeleteTextures(1, &tex);
}

// Test that sized SRGBA formats allow generating mipmaps
TEST_P(SRGBTextureTestES3, SRGBASizedValidation)
{
    // TODO(fjhenigman): Figure out why this fails on Ozone Intel.
    ANGLE_SKIP_TEST_IF(IsOzone() && IsIntel() && IsOpenGLES());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);

    GLubyte pixel[4] = {0};
    glTexImage2D(GL_TEXTURE_2D, 0, getSRGBA8TextureInternalFormat(), 1, 1, 0,
                 getSRGBA8TextureFormat(), GL_UNSIGNED_BYTE, pixel);

    EXPECT_GL_NO_ERROR();

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    EXPECT_GL_NO_ERROR();

    glGenerateMipmap(GL_TEXTURE_2D);
    EXPECT_GL_NO_ERROR();
}

TEST_P(SRGBTextureTest, SRGBARenderbuffer)
{
    bool supported = IsGLExtensionEnabled("GL_EXT_sRGB") || getClientMajorVersion() == 3;

    GLuint rbo = 0;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_SRGB8_ALPHA8_EXT, 1, 1);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        // Make sure the rbo has a size for future tests
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, 1, 1);
        EXPECT_GL_NO_ERROR();
    }

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    EXPECT_GL_NO_ERROR();

    GLint colorEncoding = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT,
                                          &colorEncoding);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(GL_SRGB_EXT, colorEncoding);
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);
}

// Verify that if the srgb decode extension is available, srgb textures are too
TEST_P(SRGBTextureTest, SRGBDecodeExtensionAvailability)
{
    bool hasSRGBDecode = IsGLExtensionEnabled("GL_EXT_texture_sRGB_decode");
    if (hasSRGBDecode)
    {
        bool hasSRGBTextures = IsGLExtensionEnabled("GL_EXT_sRGB") || getClientMajorVersion() >= 3;
        EXPECT_TRUE(hasSRGBTextures);
    }
}

// Test basic functionality of SRGB decode using the texture parameter
TEST_P(SRGBTextureTest, SRGBDecodeTextureParameter)
{
    // TODO(fjhenigman): Figure out why this fails on Ozone Intel.
    ANGLE_SKIP_TEST_IF(IsOzone() && IsIntel() && IsOpenGLES());

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_sRGB_decode"));

    constexpr angle::GLColor srgbColor(64, 127, 191, 255);
    constexpr angle::GLColor decodedToLinearColor(13, 54, 133, 255);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, getSRGBA8TextureInternalFormat(), 1, 1, 0,
                 getSRGBA8TextureFormat(), GL_UNSIGNED_BYTE, srgbColor.data());
    ASSERT_GL_NO_ERROR();

    glUseProgram(mProgram);
    glUniform1i(mTextureLocation, 0);
    glDisable(GL_DEPTH_TEST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_DECODE_EXT);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, decodedToLinearColor, 1.0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_SKIP_DECODE_EXT);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_DECODE_EXT);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, decodedToLinearColor, 1.0);
}

// Test interaction between SRGB decode and texelFetch
TEST_P(SRGBTextureTestES3, SRGBDecodeTexelFetch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_sRGB_decode"));

    constexpr angle::GLColor srgbColor(64, 127, 191, 255);
    constexpr angle::GLColor decodedToLinearColor(13, 54, 133, 255);

    constexpr char kTexelFetchFS[] = R"(#version 300 es
precision highp float;
precision highp int;

uniform highp sampler2D tex;

in vec4 v_position;
out vec4 my_FragColor;

void main() {
    ivec2 sampleCoords = ivec2(v_position.xy * 0.5 + 0.5);
    my_FragColor = texelFetch(tex, sampleCoords, 0);
}
)";

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, getSRGBA8TextureInternalFormat(), 1, 1, 0,
                 getSRGBA8TextureFormat(), GL_UNSIGNED_BYTE, srgbColor.data());
    ASSERT_GL_NO_ERROR();

    glUseProgram(mProgram);
    glUniform1i(mTextureLocation, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_DECODE_EXT);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, decodedToLinearColor, 1.0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_SKIP_DECODE_EXT);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);

    ANGLE_GL_PROGRAM(texelFetchProgram, essl3_shaders::vs::Passthrough(), kTexelFetchFS);
    glUseProgram(texelFetchProgram);
    GLint texLocation = glGetUniformLocation(texelFetchProgram, "tex");
    ASSERT_GE(texLocation, 0);
    glUniform1i(texLocation, 0);

    drawQuad(texelFetchProgram, "a_position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, decodedToLinearColor, 1.0);
}

// Test basic functionality of SRGB override using the texture parameter
TEST_P(SRGBTextureTest, SRGBOverrideTextureParameter)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_format_sRGB_override"));

    GLenum internalFormat = getClientMajorVersion() >= 3 ? GL_RGBA8 : GL_RGBA;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kLinearColor.data());
    ASSERT_GL_NO_ERROR();

    glUseProgram(mProgram);
    glUniform1i(mTextureLocation, 0);
    glDisable(GL_DEPTH_TEST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT, GL_NONE);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, kLinearColor, 1.0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT, GL_SRGB);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, kNonlinearColor, 1.0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT, GL_NONE);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, kLinearColor, 1.0);
}

// Test that all supported formats can be overridden
TEST_P(SRGBTextureTestES3, SRGBOverrideFormats)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_format_sRGB_override"));

    constexpr GLenum possibleFormats[] = {GL_RGB8,
                                          GL_RGBA8,
                                          GL_COMPRESSED_RGB8_ETC2,
                                          GL_COMPRESSED_RGBA8_ETC2_EAC,
                                          GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
                                          GL_COMPRESSED_RGBA_ASTC_4x4,
                                          GL_COMPRESSED_RGBA_ASTC_5x4,
                                          GL_COMPRESSED_RGBA_ASTC_5x5,
                                          GL_COMPRESSED_RGBA_ASTC_6x5,
                                          GL_COMPRESSED_RGBA_ASTC_6x6,
                                          GL_COMPRESSED_RGBA_ASTC_8x5,
                                          GL_COMPRESSED_RGBA_ASTC_8x6,
                                          GL_COMPRESSED_RGBA_ASTC_8x8,
                                          GL_COMPRESSED_RGBA_ASTC_10x5,
                                          GL_COMPRESSED_RGBA_ASTC_10x6,
                                          GL_COMPRESSED_RGBA_ASTC_10x8,
                                          GL_COMPRESSED_RGBA_ASTC_10x10,
                                          GL_COMPRESSED_RGBA_ASTC_12x10,
                                          GL_COMPRESSED_RGBA_ASTC_12x12,
                                          GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                          GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
                                          GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
                                          GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
                                          GL_R8,
                                          GL_RG8,
                                          GL_COMPRESSED_RGBA_BPTC_UNORM_EXT};

    for (GLenum format : possibleFormats)
    {
        GLTexture tex;
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexStorage2D(GL_TEXTURE_2D, 1, format, 4, 4);
        GLenum error = glGetError();
        if (error == GL_INVALID_ENUM)
        {
            // Format is not supported, we don't require the sRGB counterpart to be supported either
            continue;
        }
        else
        {
            ASSERT_EQ(static_cast<GLenum>(GL_NO_ERROR), error);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT, GL_NONE);
        ASSERT_GL_NO_ERROR();

        glUseProgram(mProgram);
        glUniform1i(mTextureLocation, 0);

        glDisable(GL_DEPTH_TEST);
        drawQuad(mProgram, "position", 0.5f);
        ASSERT_GL_NO_ERROR();
        // Discard result, we are only checking that we don't try to reinterpret to an unsupported
        // format
    }
}

// Test interaction between sRGB_override and sampler objects
TEST_P(SRGBTextureTestES3, SRGBOverrideTextureParameterWithSampler)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_format_sRGB_override"));

    GLColor linearColor = kLinearColor;
    GLColor srgbColor   = kNonlinearColor;

    GLenum internalFormat = getClientMajorVersion() >= 3 ? GL_RGBA8 : GL_RGBA;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 &linearColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT, GL_NONE);
    ASSERT_GL_NO_ERROR();

    GLSampler sampler;
    glBindSampler(0, sampler);

    glUseProgram(mProgram);
    glUniform1i(mTextureLocation, 0);

    glDisable(GL_DEPTH_TEST);
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, linearColor, 1.0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT, GL_SRGB);
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT, GL_NONE);
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, linearColor, 1.0);
}

// Test that SRGB override is a noop when used on a nonlinear texture format
// EXT_texture_format_sRGB_override spec says:
// "If the internal format is not one of the above formats, then
// the value of TEXTURE_FORMAT_SRGB_OVERRIDE_EXT is ignored."
TEST_P(SRGBTextureTestES3, SRGBOverrideTextureParameterNoop)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_format_sRGB_override"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_sRGB"));

    GLColor linearColor = kLinearColor;
    GLColor srgbColor   = kNonlinearColor;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, getSRGBA8TextureInternalFormat(), 1, 1, 0,
                 getSRGBA8TextureFormat(), GL_UNSIGNED_BYTE, &linearColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT, GL_NONE);
    ASSERT_GL_NO_ERROR();

    glUseProgram(mProgram);
    glUniform1i(mTextureLocation, 0);

    glDisable(GL_DEPTH_TEST);
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT, GL_SRGB);
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);
}

// Test basic functionality of SRGB decode using the sampler parameter
TEST_P(SRGBTextureTestES3, SRGBDecodeSamplerParameter)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_sRGB_decode"));

    GLColor linearColor = kLinearColor;
    GLColor srgbColor   = kNonlinearColor;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, getSRGBA8TextureInternalFormat(), 1, 1, 0,
                 getSRGBA8TextureFormat(), GL_UNSIGNED_BYTE, &linearColor);
    ASSERT_GL_NO_ERROR();

    GLSampler sampler;
    glBindSampler(0, sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_SRGB_DECODE_EXT, GL_DECODE_EXT);

    glUseProgram(mProgram);
    glUniform1i(mTextureLocation, 0);

    glDisable(GL_DEPTH_TEST);
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);

    glSamplerParameteri(sampler, GL_TEXTURE_SRGB_DECODE_EXT, GL_SKIP_DECODE_EXT);
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, linearColor, 1.0);
}

// Toggle between GL_DECODE_EXT and GL_SKIP_DECODE_EXT of sampler parameter
// GL_TEXTURE_SRGB_DECODE_EXT
TEST_P(SRGBTextureTestES3, SRGBDecodeSamplerParameterToggle)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_sRGB_decode"));

    GLColor linearColor = kLinearColor;
    GLColor srgbColor   = kNonlinearColor;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, getSRGBA8TextureInternalFormat(), 1, 1, 0,
                 getSRGBA8TextureFormat(), GL_UNSIGNED_BYTE, &linearColor);
    ASSERT_GL_NO_ERROR();

    GLSampler sampler;
    glBindSampler(0, sampler);

    glUseProgram(mProgram);
    glUniform1i(mTextureLocation, 0);
    glDisable(GL_DEPTH_TEST);

    for (int i = 0; i < 4; i++)
    {
        // Toggle betwee decode and skip decode and verify pixel value
        GLint decode                  = ((i & 1) == 0) ? GL_DECODE_EXT : GL_SKIP_DECODE_EXT;
        angle::GLColor &expectedColor = ((i & 1) == 0) ? srgbColor : linearColor;

        glSamplerParameteri(sampler, GL_TEXTURE_SRGB_DECODE_EXT, decode);
        drawQuad(mProgram, "position", 0.5f);
        EXPECT_PIXEL_COLOR_NEAR(0, 0, expectedColor, 1.0);
    }
}

// Test that sampler state overrides texture state for srgb decode
TEST_P(SRGBTextureTestES3, SRGBDecodeTextureAndSamplerParameter)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_sRGB_decode"));

    GLColor linearColor = kLinearColor;
    GLColor srgbColor   = kNonlinearColor;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, getSRGBA8TextureInternalFormat(), 1, 1, 0,
                 getSRGBA8TextureFormat(), GL_UNSIGNED_BYTE, &linearColor);

    ASSERT_GL_NO_ERROR();

    GLSampler sampler;
    glBindSampler(0, sampler);

    glUseProgram(mProgram);
    glUniform1i(mTextureLocation, 0);

    glDisable(GL_DEPTH_TEST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_SKIP_DECODE_EXT);
    glSamplerParameteri(sampler, GL_TEXTURE_SRGB_DECODE_EXT, GL_DECODE_EXT);
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_DECODE_EXT);
    glSamplerParameteri(sampler, GL_TEXTURE_SRGB_DECODE_EXT, GL_SKIP_DECODE_EXT);
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, linearColor, 1.0);
}

// Test that srgb decode state takes priority over srgb override state
TEST_P(SRGBTextureTestES3, SRGBDecodeOverridePriority)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_sRGB_decode"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_format_sRGB_override"));

    GLColor linearColor = kLinearColor;

    GLenum internalFormat = getClientMajorVersion() >= 3 ? GL_RGBA8 : GL_RGBA;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 &linearColor);
    ASSERT_GL_NO_ERROR();

    glUseProgram(mProgram);
    glUniform1i(mTextureLocation, 0);

    glDisable(GL_DEPTH_TEST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_FORMAT_SRGB_OVERRIDE_EXT, GL_SRGB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_SKIP_DECODE_EXT);
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, linearColor, 1.0);
}

// Test that mipmaps are generated correctly for sRGB textures
TEST_P(SRGBTextureTestES3, GenerateMipmaps)
{
    ANGLE_SKIP_TEST_IF(IsOpenGL() && ((IsIntel() && IsMac()) || IsAMD()));

    auto createAndReadBackTexture = [this](GLenum internalFormat, const GLColor &color) {
        constexpr GLsizei width  = 128;
        constexpr GLsizei height = 128;

        std::array<GLColor, width * height> buf;
        std::fill(buf.begin(), buf.end(), color);

        // Set up-left region of the texture as red color.
        // In order to make sure bi-linear interpolation operates on different colors, red region
        // is 1 pixel smaller than a quarter of the full texture on each side.
        constexpr GLsizei redWidth  = width / 2 - 1;
        constexpr GLsizei redHeight = height / 2 - 1;
        std::array<GLColor, redWidth * redHeight> redBuf;
        std::fill(redBuf.begin(), redBuf.end(), GLColor::red);

        GLTexture tex;
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     buf.data());
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, redWidth, redHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                        redBuf.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        constexpr GLsizei drawWidth  = 32;
        constexpr GLsizei drawHeight = 32;
        glViewport(0, 0, drawWidth, drawHeight);

        drawQuad(mProgram, "position", 0.5f);

        std::array<GLColor, drawWidth * drawHeight> result;
        glReadPixels(0, 0, drawWidth, drawHeight, GL_RGBA, GL_UNSIGNED_BYTE, result.data());

        EXPECT_GL_NO_ERROR();

        return result;
    };

    GLColor srgbaColor(0, 63, 127, 255);
    auto srgbaReadback = createAndReadBackTexture(GL_SRGB8_ALPHA8, srgbaColor);

    GLColor linearColor(0, 13, 54, 255);
    auto rgbaReadback = createAndReadBackTexture(GL_RGBA8, linearColor);

    ASSERT_EQ(srgbaReadback.size(), rgbaReadback.size());
    for (size_t i = 0; i < srgbaReadback.size(); i++)
    {
        constexpr double tolerence = 7.0;
        EXPECT_COLOR_NEAR(srgbaReadback[i], rgbaReadback[i], tolerence);
    }
}

// Test that generated mip levels are correct for solid color textures
TEST_P(SRGBTextureTestES3, GenerateMipmapsSolid)
{
    GLColor color(7, 7, 7, 7);

    std::array<GLColor, 4 * 4> buf;
    std::fill(buf.begin(), buf.end(), color);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fb;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 1);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, color, 1);
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(SRGBTextureTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SRGBTextureTestES3);
ANGLE_INSTANTIATE_TEST_ES3(SRGBTextureTestES3);

}  // namespace angle
