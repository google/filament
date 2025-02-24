//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CopyTextureTest.cpp: Tests of the GL_CHROMIUM_copy_texture extension

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace angle
{

class CopyTextureTest : public ANGLETest<>
{
  protected:
    CopyTextureTest()
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
        glGenTextures(2, mTextures);
        glBindTexture(GL_TEXTURE_2D, mTextures[1]);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[1],
                               0);
    }

    void testTearDown() override
    {
        glDeleteTextures(2, mTextures);
        glDeleteFramebuffers(1, &mFramebuffer);
    }

    bool checkExtensions() const
    {
        if (!IsGLExtensionEnabled("GL_CHROMIUM_copy_texture"))
        {
            std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available."
                      << std::endl;
            return false;
        }

        EXPECT_NE(nullptr, glCopyTextureCHROMIUM);
        EXPECT_NE(nullptr, glCopySubTextureCHROMIUM);
        return true;
    }

    void testGradientDownsampleUniqueValues(GLenum destFormat,
                                            GLenum destType,
                                            const std::array<size_t, 4> &expectedUniqueValues)
    {
        std::array<GLColor, 256> sourceGradient;
        for (size_t i = 0; i < sourceGradient.size(); i++)
        {
            GLubyte value     = static_cast<GLubyte>(i);
            sourceGradient[i] = GLColor(value, value, value, value);
        }
        GLTexture sourceTexture;
        glBindTexture(GL_TEXTURE_2D, sourceTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     sourceGradient.data());

        GLTexture destTexture;
        glBindTexture(GL_TEXTURE_2D, destTexture);
        glCopyTextureCHROMIUM(sourceTexture, 0, GL_TEXTURE_2D, destTexture, 0, destFormat, destType,
                              GL_FALSE, GL_FALSE, GL_FALSE);
        EXPECT_GL_NO_ERROR();

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, destTexture, 0);

        std::array<GLColor, 256> destData;
        glReadPixels(0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, destData.data());
        EXPECT_GL_NO_ERROR();

        std::set<GLubyte> uniqueValues[4];
        for (size_t i = 0; i < destData.size(); i++)
        {
            GLColor color = destData[i];
            uniqueValues[0].insert(color.R);
            uniqueValues[1].insert(color.G);
            uniqueValues[2].insert(color.B);
            uniqueValues[3].insert(color.A);
        }

        EXPECT_EQ(expectedUniqueValues[0], uniqueValues[0].size());
        EXPECT_EQ(expectedUniqueValues[1], uniqueValues[1].size());
        EXPECT_EQ(expectedUniqueValues[2], uniqueValues[2].size());
        EXPECT_EQ(expectedUniqueValues[3], uniqueValues[3].size());
    }

    void testSrgbToRgb(GLenum internalformat, GLenum format)
    {
        const size_t kTestCount                = 4;
        const GLColor kSourceColor[kTestCount] = {
            GLColor(89, 67, 45, 123),
            GLColor(87, 69, 45, 123),
            GLColor(180, 143, 93, 123),
            GLColor(89, 67, 45, 123),
        };
        const GLColor kExpectedColor[kTestCount] = {
            GLColor(89, 67, 45, 123),
            GLColor(180, 143, 93, 123),
            GLColor(87, 69, 45, 123),
            GLColor(89, 67, 45, 123),
        };
        bool kPremultiply[kTestCount] = {false, false, true, true};
        bool kUnmultiply[kTestCount]  = {false, true, false, true};

        for (size_t test = 0; test < kTestCount; ++test)
        {
            // Create image as sRGB.
            GLTexture sourceTexture;
            glBindTexture(GL_TEXTURE_2D, sourceTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, internalformat, 1, 1, 0, format, GL_UNSIGNED_BYTE,
                         &kSourceColor[test]);
            ASSERT_GL_NO_ERROR();

            GLTexture destTexture;
            glBindTexture(GL_TEXTURE_2D, destTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            ASSERT_GL_NO_ERROR();

            // Note: flipY is used to avoid direct transfer between textures and force a draw-based
            // path.
            glCopySubTextureCHROMIUM(sourceTexture, 0, GL_TEXTURE_2D, destTexture, 0,  // level,
                                     0, 0,                                             // src x,y
                                     0, 0,                                             // dst x,y
                                     1, 1,                // width, height
                                     true,                // flip-y
                                     kPremultiply[test],  // premul
                                     kUnmultiply[test]);  // unmul
            ASSERT_GL_NO_ERROR();

            // Verify the copy.
            ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(),
                             essl1_shaders::fs::Texture2D());
            glUseProgram(program);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, destTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            GLint textureLocation =
                glGetUniformLocation(program, essl1_shaders::Texture2DUniform());
            ASSERT_NE(-1, textureLocation);
            glUniform1i(textureLocation, 0);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
            ASSERT_GL_NO_ERROR();

            EXPECT_PIXEL_COLOR_NEAR(0, 0, kExpectedColor[test], 2);
        }
    }

    GLuint mTextures[2] = {
        0,
        0,
    };
    GLuint mFramebuffer = 0;
};

using CopyTextureVariationsTestParams =
    std::tuple<angle::PlatformParameters, GLenum, GLenum, bool, bool, bool, GLint>;

std::string CopyTextureVariationsTestPrint(
    const ::testing::TestParamInfo<CopyTextureVariationsTestParams> &paramsInfo)
{
    const CopyTextureVariationsTestParams &params = paramsInfo.param;
    std::ostringstream out;

    out << std::get<0>(params) << "__";

    switch (std::get<1>(params))
    {
        case GL_ALPHA:
            out << "A";
            break;
        case GL_RGB:
            out << "RGB";
            break;
        case GL_RGBA:
            out << "RGBA";
            break;
        case GL_LUMINANCE:
            out << "L";
            break;
        case GL_LUMINANCE_ALPHA:
            out << "LA";
            break;
        case GL_BGRA_EXT:
            out << "BGRA";
            break;
        case GL_SRGB_ALPHA_EXT:
            out << "SRGBA";
            break;
        default:
            out << "UPDATE_THIS_SWITCH";
    }

    out << "To";

    switch (std::get<2>(params))
    {
        case GL_RGB:
            out << "RGB";
            break;
        case GL_RGBA:
            out << "RGBA";
            break;
        case GL_BGRA_EXT:
            out << "BGRA";
            break;
        case GL_SRGB_ALPHA_EXT:
            out << "SRGBA";
            break;
        default:
            out << "UPDATE_THIS_SWITCH";
    }

    if (std::get<3>(params))
    {
        out << "FlipY";
    }
    if (std::get<4>(params))
    {
        out << "PremultiplyAlpha";
    }
    if (std::get<5>(params))
    {
        out << "UnmultiplyAlpha";
    }
    if (std::get<6>(params))
    {
        out << "MesaYFlip";
    }

    return out.str();
}

class CopyTextureVariationsTest : public ANGLETest<CopyTextureVariationsTestParams>
{
  protected:
    CopyTextureVariationsTest()
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
        glGenTextures(2, mTextures);
        glBindTexture(GL_TEXTURE_2D, mTextures[1]);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[1],
                               0);
    }

    void testTearDown() override
    {
        glDeleteTextures(2, mTextures);
        glDeleteFramebuffers(1, &mFramebuffer);
    }

    bool checkExtensions(GLenum sourceFormat, GLenum destFormat) const
    {
        if (!IsGLExtensionEnabled("GL_CHROMIUM_copy_texture"))
        {
            std::cout << "Test skipped because GL_CHROMIUM_copy_texture is not available."
                      << std::endl;
            return false;
        }

        if ((sourceFormat == GL_BGRA_EXT || destFormat == GL_BGRA_EXT) &&
            !IsGLExtensionEnabled("GL_EXT_texture_format_BGRA8888"))
        {
            return false;
        }

        if ((sourceFormat == GL_SRGB_ALPHA_EXT || destFormat == GL_SRGB_ALPHA_EXT) &&
            !IsGLExtensionEnabled("GL_EXT_sRGB"))
        {
            return false;
        }

        return true;
    }

    void calculateCopyTextureResults(GLenum sourceFormat,
                                     GLenum destFormat,
                                     bool premultiplyAlpha,
                                     bool unmultiplyAlpha,
                                     const uint8_t *sourceColor,
                                     GLColor *destColor)
    {
        GLColor color;

        switch (sourceFormat)
        {
            case GL_RGB:
                color = GLColor(sourceColor[0], sourceColor[1], sourceColor[2], 255);
                break;
            case GL_RGBA:
                color = GLColor(sourceColor[0], sourceColor[1], sourceColor[2], sourceColor[3]);
                break;
            case GL_LUMINANCE:
                color = GLColor(sourceColor[0], sourceColor[0], sourceColor[0], 255);
                break;
            case GL_ALPHA:
                color = GLColor(0, 0, 0, sourceColor[0]);
                break;
            case GL_LUMINANCE_ALPHA:
                color = GLColor(sourceColor[0], sourceColor[0], sourceColor[0], sourceColor[1]);
                break;
            case GL_BGRA_EXT:
                color = GLColor(sourceColor[2], sourceColor[1], sourceColor[0], sourceColor[3]);
                break;
            default:
                EXPECT_EQ(true, false);
        }

        if (premultiplyAlpha != unmultiplyAlpha)
        {
            float alpha = color.A / 255.0f;
            if (premultiplyAlpha)
            {
                color.R = static_cast<GLubyte>(static_cast<float>(color.R) * alpha);
                color.G = static_cast<GLubyte>(static_cast<float>(color.G) * alpha);
                color.B = static_cast<GLubyte>(static_cast<float>(color.B) * alpha);
            }
            else if (unmultiplyAlpha && color.A != 0)
            {
                color.R = static_cast<GLubyte>(static_cast<float>(color.R) / alpha);
                color.G = static_cast<GLubyte>(static_cast<float>(color.G) / alpha);
                color.B = static_cast<GLubyte>(static_cast<float>(color.B) / alpha);
            }
        }

        switch (destFormat)
        {
            case GL_RGB:
                color.A = 255;
                break;
            case GL_RGBA:
            case GL_BGRA_EXT:
            case GL_SRGB_ALPHA_EXT:
                break;
            default:
                EXPECT_EQ(true, false);
        }

        *destColor = color;
    }

    const uint8_t *getSourceColors(GLenum sourceFormat, size_t *colorCount, uint8_t *componentCount)
    {
        // Note: in all the following values, alpha is larger than RGB so unmultiply alpha doesn't
        // overflow
        constexpr static uint8_t kRgbaColors[7 * 4] = {
            255u, 127u, 63u,  255u,  // 0
            31u,  127u, 63u,  127u,  // 1
            31u,  63u,  127u, 255u,  // 2
            15u,  127u, 31u,  127u,  // 3
            127u, 255u, 63u,  0u,    // 4
            31u,  63u,  127u, 0u,    // 5
            15u,  31u,  63u,  63u,   // 6
        };

        constexpr static uint8_t kRgbColors[7 * 3] = {
            255u, 127u, 63u,   // 0
            31u,  127u, 63u,   // 1
            31u,  63u,  127u,  // 2
            15u,  127u, 31u,   // 3
            127u, 255u, 63u,   // 4
            31u,  63u,  127u,  // 5
            15u,  31u,  63u,   // 6
        };

        constexpr static uint8_t kLumColors[7 * 1] = {
            255u,  // 0
            163u,  // 1
            78u,   // 2
            114u,  // 3
            51u,   // 4
            0u,    // 5
            217u,  // 6
        };

        constexpr static uint8_t kLumaColors[7 * 2] = {
            255u, 255u,  // 0
            67u,  163u,  // 1
            78u,  231u,  // 2
            8u,   114u,  // 3
            51u,  199u,  // 4
            0u,   173u,  // 5
            34u,  217u,  // 6
        };

        constexpr static uint8_t kAlphaColors[7 * 1] = {
            255u,  // 0
            67u,   // 1
            231u,  // 2
            8u,    // 3
            199u,  // 4
            173u,  // 5
            34u,   // 6
        };

        *colorCount = 7;

        switch (sourceFormat)
        {
            case GL_RGB:
                *componentCount = 3;
                return kRgbColors;
            case GL_RGBA:
            case GL_BGRA_EXT:
                *componentCount = 4;
                return kRgbaColors;
            case GL_LUMINANCE:
                *componentCount = 1;
                return kLumColors;
            case GL_ALPHA:
                *componentCount = 1;
                return kAlphaColors;
            case GL_LUMINANCE_ALPHA:
                *componentCount = 2;
                return kLumaColors;
            default:
                EXPECT_EQ(true, false);
                return nullptr;
        }
    }

    void initializeSourceTexture(GLenum target,
                                 GLenum sourceFormat,
                                 const uint8_t *srcColors,
                                 uint8_t componentCount)
    {
        // The texture is initialized as 2x2.  If the componentCount is 1 or 3, then the input data
        // will have a row pitch of 2 or 6, which needs to be padded to 4 or 8 respectively.
        uint8_t srcColorsPadded[4 * 4];
        size_t srcRowPitch =
            2 * componentCount + (componentCount == 1 || componentCount == 3 ? 2 : 0);
        size_t inputRowPitch = 2 * componentCount;
        for (size_t row = 0; row < 2; ++row)
        {
            memcpy(&srcColorsPadded[row * srcRowPitch], &srcColors[row * inputRowPitch],
                   inputRowPitch);
            memset(&srcColorsPadded[row * srcRowPitch + inputRowPitch], 0,
                   srcRowPitch - inputRowPitch);
        }

        glBindTexture(target, mTextures[0]);
        glTexImage2D(target, 0, sourceFormat, 2, 2, 0, sourceFormat, GL_UNSIGNED_BYTE,
                     srcColorsPadded);
    }

    void testCopyTexture(GLenum sourceTarget,
                         GLenum sourceFormat,
                         GLenum destFormat,
                         bool flipY,
                         bool premultiplyAlpha,
                         bool unmultiplyAlpha,
                         GLint mesaYFlipParam)
    {
        if (!checkExtensions(sourceFormat, destFormat))
        {
            return;
        }

        const bool hasMesaFbFlipYExt = IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y");
        if (mesaYFlipParam && !hasMesaFbFlipYExt)
            ASSERT_TRUE(hasMesaFbFlipYExt);

        if (sourceFormat == GL_LUMINANCE || sourceFormat == GL_LUMINANCE_ALPHA ||
            sourceFormat == GL_ALPHA || destFormat == GL_LUMINANCE ||
            destFormat == GL_LUMINANCE_ALPHA || destFormat == GL_ALPHA)
        {
            // http://anglebug.com/42263512
            ANGLE_SKIP_TEST_IF(IsOpenGL() && destFormat == GL_SRGB_ALPHA_EXT);
        }

        size_t colorCount;
        uint8_t componentCount;
        const uint8_t *srcColors = getSourceColors(sourceFormat, &colorCount, &componentCount);

        std::vector<GLColor> destColors(colorCount);
        for (size_t i = 0; i < colorCount; ++i)
        {
            calculateCopyTextureResults(sourceFormat, destFormat, premultiplyAlpha, unmultiplyAlpha,
                                        &srcColors[i * componentCount], &destColors[i]);
        }

        for (size_t i = 0; i < colorCount - 3; ++i)
        {
            initializeSourceTexture(sourceTarget, sourceFormat, &srcColors[i * componentCount],
                                    componentCount);

            if (hasMesaFbFlipYExt)
            {
                glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA,
                                            mesaYFlipParam);
            }
            glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, destFormat,
                                  GL_UNSIGNED_BYTE, flipY, premultiplyAlpha, unmultiplyAlpha);

            EXPECT_GL_NO_ERROR();

            // Check that FB is complete.
            EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

            if (flipY)
            {
                if (mesaYFlipParam)
                {
                    EXPECT_PIXEL_COLOR_NEAR(0, 0, destColors[i + 0], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 0, destColors[i + 1], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(0, 1, destColors[i + 2], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 1, destColors[i + 3], 1.0);
                }
                else
                {
                    EXPECT_PIXEL_COLOR_NEAR(0, 0, destColors[i + 2], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 0, destColors[i + 3], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(0, 1, destColors[i + 0], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 1, destColors[i + 1], 1.0);
                }
            }
            else
            {
                if (mesaYFlipParam)
                {
                    EXPECT_PIXEL_COLOR_NEAR(0, 0, destColors[i + 2], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 0, destColors[i + 3], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(0, 1, destColors[i + 0], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 1, destColors[i + 1], 1.0);
                }
                else
                {
                    EXPECT_PIXEL_COLOR_NEAR(0, 0, destColors[i + 0], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 0, destColors[i + 1], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(0, 1, destColors[i + 2], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 1, destColors[i + 3], 1.0);
                }
            }

            EXPECT_GL_NO_ERROR();
        }
    }

    void testCopySubTexture(GLenum sourceTarget,
                            GLenum sourceFormat,
                            GLenum destFormat,
                            bool flipY,
                            bool premultiplyAlpha,
                            bool unmultiplyAlpha,
                            GLint mesaYFlipParam)
    {
        if (!checkExtensions(sourceFormat, destFormat))
        {
            return;
        }

        const bool hasMesaFbFlipYExt = IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y");
        if (mesaYFlipParam && !hasMesaFbFlipYExt)
            ASSERT_TRUE(hasMesaFbFlipYExt);

        if (sourceFormat == GL_LUMINANCE || sourceFormat == GL_LUMINANCE_ALPHA ||
            sourceFormat == GL_ALPHA || destFormat == GL_LUMINANCE ||
            destFormat == GL_LUMINANCE_ALPHA || destFormat == GL_ALPHA)
        {
            // http://anglebug.com/42263512
            ANGLE_SKIP_TEST_IF(IsOpenGL() && destFormat == GL_SRGB_ALPHA_EXT);
        }

        size_t colorCount;
        uint8_t componentCount;
        const uint8_t *srcColors = getSourceColors(sourceFormat, &colorCount, &componentCount);

        std::vector<GLColor> destColors(colorCount);
        for (size_t i = 0; i < colorCount; ++i)
        {
            calculateCopyTextureResults(sourceFormat, destFormat, premultiplyAlpha, unmultiplyAlpha,
                                        &srcColors[i * componentCount], &destColors[i]);
        }

        for (size_t i = 0; i < colorCount - 3; ++i)
        {
            initializeSourceTexture(sourceTarget, sourceFormat, &srcColors[i * componentCount],
                                    componentCount);

            glBindTexture(GL_TEXTURE_2D, mTextures[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, destFormat, 2, 2, 0, destFormat, GL_UNSIGNED_BYTE,
                         nullptr);

            if (hasMesaFbFlipYExt)
            {
                glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA,
                                            mesaYFlipParam);
            }
            glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 0, 0, 0, 0, 2,
                                     2, flipY, premultiplyAlpha, unmultiplyAlpha);

            EXPECT_GL_NO_ERROR();

            if (sourceFormat != GL_LUMINANCE && sourceFormat != GL_LUMINANCE_ALPHA &&
                sourceFormat != GL_ALPHA)
            {
                // Check that FB is complete.
                EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
            }

            if (flipY)
            {
                if (mesaYFlipParam)
                {
                    EXPECT_PIXEL_COLOR_NEAR(0, 0, destColors[i + 0], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 0, destColors[i + 1], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(0, 1, destColors[i + 2], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 1, destColors[i + 3], 1.0);
                }
                else
                {
                    EXPECT_PIXEL_COLOR_NEAR(0, 0, destColors[i + 2], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 0, destColors[i + 3], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(0, 1, destColors[i + 0], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 1, destColors[i + 1], 1.0);
                }
            }
            else
            {
                if (mesaYFlipParam)
                {
                    EXPECT_PIXEL_COLOR_NEAR(0, 0, destColors[i + 2], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 0, destColors[i + 3], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(0, 1, destColors[i + 0], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 1, destColors[i + 1], 1.0);
                }
                else
                {
                    EXPECT_PIXEL_COLOR_NEAR(0, 0, destColors[i + 0], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 0, destColors[i + 1], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(0, 1, destColors[i + 2], 1.0);
                    EXPECT_PIXEL_COLOR_NEAR(1, 1, destColors[i + 3], 1.0);
                }
            }

            EXPECT_GL_NO_ERROR();
        }
    }

    GLuint mTextures[2] = {
        0,
        0,
    };
    GLuint mFramebuffer = 0;
};

class CopyTextureTestDest : public CopyTextureTest
{};

class CopyTextureTestWebGL : public CopyTextureTest
{
  protected:
    CopyTextureTestWebGL() : CopyTextureTest() { setWebGLCompatibilityEnabled(true); }
};

class CopyTextureTestES3 : public CopyTextureTest
{
  protected:
    void invalidateBlitThenBlendCommon(GLsizei layerCount);
};

// Test that CopyTexture cannot redefine an immutable texture and CopySubTexture can copy data to
// immutable textures
TEST_P(CopyTextureTest, ImmutableTexture)
{
    if (!checkExtensions())
    {
        return;
    }

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       (!IsGLExtensionEnabled("GL_EXT_texture_storage") ||
                        !IsGLExtensionEnabled("GL_OES_rgb8_rgba8")));

    GLColor pixels = GLColor::red;

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8_OES, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixels);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8_OES, 1, 1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[1], 0);
    EXPECT_GL_NO_ERROR();

    // Should generate an error when the texture is redefined
    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Should succeed when using CopySubTexture
    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 0, 0, 0, 0, 1, 1,
                             false, false, false);
    EXPECT_GL_NO_ERROR();

    // Check that FB is complete.
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    EXPECT_PIXEL_COLOR_EQ(0, 0, pixels);

    EXPECT_GL_NO_ERROR();
}

struct FormatPair
{
    explicit FormatPair(GLenum format) : format(format), internalFormat(format) {}
    FormatPair(GLenum format, GLint internalFormat) : format(format), internalFormat(internalFormat)
    {}

    GLenum format;
    GLint internalFormat;
};

// Test validation of internal formats in CopyTexture and CopySubTexture
TEST_P(CopyTextureTest, InternalFormat)
{
    if (!checkExtensions())
    {
        return;
    }

    std::vector<FormatPair> sourceFormats;
    sourceFormats.push_back(FormatPair(GL_ALPHA));
    sourceFormats.push_back(FormatPair(GL_RGB));
    sourceFormats.push_back(FormatPair(GL_RGBA));
    sourceFormats.push_back(FormatPair(GL_LUMINANCE));
    sourceFormats.push_back(FormatPair(GL_LUMINANCE_ALPHA));

    std::vector<FormatPair> destFormats;
    destFormats.push_back(FormatPair(GL_RGB));
    destFormats.push_back(FormatPair(GL_RGBA));

    if (IsGLExtensionEnabled("GL_EXT_texture_format_BGRA8888"))
    {
        sourceFormats.push_back(FormatPair(GL_BGRA_EXT));
        destFormats.push_back(FormatPair(GL_BGRA_EXT));
    }

    if (IsGLExtensionEnabled("GL_ANGLE_rgbx_internal_format"))
    {
        sourceFormats.push_back(FormatPair(GL_RGB, GL_RGBX8_ANGLE));
        destFormats.push_back(FormatPair(GL_RGB, GL_RGBX8_ANGLE));
    }

    // Test with glCopyTexture
    for (FormatPair &sourceFormat : sourceFormats)
    {
        for (FormatPair &destFormat : destFormats)
        {
            glBindTexture(GL_TEXTURE_2D, mTextures[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, sourceFormat.internalFormat, 1, 1, 0,
                         sourceFormat.format, GL_UNSIGNED_BYTE, nullptr);
            EXPECT_GL_NO_ERROR();

            glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0,
                                  destFormat.internalFormat, GL_UNSIGNED_BYTE, false, false, false);

            EXPECT_GL_NO_ERROR();
        }
    }

    // Test with glCopySubTexture
    for (FormatPair &sourceFormat : sourceFormats)
    {
        for (FormatPair &destFormat : destFormats)
        {
            glBindTexture(GL_TEXTURE_2D, mTextures[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, sourceFormat.internalFormat, 1, 1, 0,
                         sourceFormat.format, GL_UNSIGNED_BYTE, nullptr);
            EXPECT_GL_NO_ERROR();

            glBindTexture(GL_TEXTURE_2D, mTextures[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, destFormat.internalFormat, 1, 1, 0, destFormat.format,
                         GL_UNSIGNED_BYTE, nullptr);
            EXPECT_GL_NO_ERROR();

            glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 0, 0, 0, 0, 1,
                                     1, false, false, false);

            EXPECT_GL_NO_ERROR();
        }
    }
}

// Test to ensure that the destination texture is redefined if the properties are different.
TEST_P(CopyTextureTest, RedefineDestinationTexture)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_format_BGRA8888"));

    GLColor pixels[4] = {GLColor::red, GLColor::red, GLColor::red, GLColor::red};

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 1, 1, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
    EXPECT_GL_NO_ERROR();

    // GL_INVALID_OPERATION due to "intrinsic format" != "internal format".
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    // GL_INVALID_VALUE due to bad dimensions.
    glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 1, 1, 1, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // If the dest texture has different properties, glCopyTextureCHROMIUM()
    // redefines them.
    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);
    EXPECT_GL_NO_ERROR();

    // glTexSubImage2D() succeeds because mTextures[1] is redefined into 2x2
    // dimension and GL_RGBA format.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    EXPECT_GL_NO_ERROR();

    // Check that FB is complete.
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    EXPECT_PIXEL_COLOR_EQ(1, 1, pixels[3]);
    EXPECT_GL_NO_ERROR();
}

// Test that invalid dimensions in CopySubTexture are validated
TEST_P(CopyTextureTest, CopySubTextureDimension)
{
    if (!checkExtensions())
    {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 1, 1, 0, 0, 1, 1,
                             false, false, false);
    EXPECT_GL_NO_ERROR();

    // xoffset < 0
    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, -1, 1, 0, 0, 1, 1,
                             false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // x < 0
    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 1, 1, -1, 0, 1, 1,
                             false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // xoffset + width > dest_width
    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 2, 2, 0, 0, 2, 2,
                             false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // x + width > source_width
    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 0, 0, 1, 1, 2, 2,
                             false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test that invalid IDs in CopyTexture are validated
TEST_P(CopyTextureTest, CopyTextureInvalidTextureIds)
{
    if (!checkExtensions())
    {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, 99993, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                          false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopyTextureCHROMIUM(99994, 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA, GL_UNSIGNED_BYTE,
                          false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopyTextureCHROMIUM(99995, 0, GL_TEXTURE_2D, 99996, 0, GL_RGBA, GL_UNSIGNED_BYTE, false,
                          false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);
    EXPECT_GL_NO_ERROR();
}

// Test that the right error type is triggered when
// OES_EGL_image_external_essl3 is required but not supported
TEST_P(CopyTextureTest, CopyTextureMissingRequiredExtension)
{
    if (!checkExtensions())
    {
        return;
    }

    // decide if the test is relevant
    // If GL_OES_EGL_image_external_essl3 is supported, then no need to test
    ANGLE_SKIP_TEST_IF(IsGLExtensionEnabled("GL_OES_EGL_image_external_essl3"));
    // GL_OES_EGL_image_external extension is required to use TEXTURE_EXTERNAL_OES
    // as the texture target.  So if GL_OES_EGL_image_external is not supported,
    // the error case cannot happen.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_EGL_image_external"));
    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();
    // If EGL_KHR_image_base is not supported, then eglImageKHR cannot be used,
    // and therefore the error case cannot happen
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(dpy, "EGL_KHR_image_base"));

    // prepare test data
    // create the texture data
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    EXPECT_GL_NO_ERROR();
    const std::vector<GLColor> kSourceColor(2 * 2, GLColor::green);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kSourceColor.data());
    EXPECT_GL_NO_ERROR();

    // create the image and load the texture data
    constexpr EGLint attribs[] = {
        EGL_IMAGE_PRESERVED,
        EGL_TRUE,
        EGL_NONE,
    };
    EGLImageKHR image = eglCreateImageKHR(
        window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
        reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(texture)), attribs);
    EXPECT_GL_NO_ERROR();
    // source is from the image
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextures[0]);
    EXPECT_GL_NO_ERROR();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
    EXPECT_GL_NO_ERROR();

    // dest
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    EXPECT_GL_NO_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    // This copying needs EGLImageExternalEssl3OES extension
    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGB32UI,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Non-integer dest internal format doesn't need the extension
    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);
    EXPECT_GL_NO_ERROR();
}

// Test that invalid IDs in CopySubTexture are validated
TEST_P(CopyTextureTest, CopySubTextureInvalidTextureIds)
{
    if (!checkExtensions())
    {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, 99993, 0, 1, 1, 0, 0, 1, 1, false,
                             false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopySubTextureCHROMIUM(99994, 0, GL_TEXTURE_2D, mTextures[1], 0, 1, 1, 0, 0, 1, 1, false,
                             false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopySubTextureCHROMIUM(99995, 0, GL_TEXTURE_2D, 99996, 0, 1, 1, 0, 0, 1, 1, false, false,
                             false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 1, 1, 0, 0, 1, 1,
                             false, false, false);
    EXPECT_GL_NO_ERROR();
}

TEST_P(CopyTextureTest, InvalidTarget)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    GLTexture textures[2];

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Invalid enum for a completely invalid target
    glCopySubTextureCHROMIUM(textures[0], 0, GL_INVALID_VALUE, textures[1], 0, 1, 1, 0, 0, 1, 1,
                             false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Invalid value for a valid target enum but is not valid for the destination texture
    glCopySubTextureCHROMIUM(textures[0], 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, textures[1], 0, 1, 1,
                             0, 0, 1, 1, false, false, false);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test that using an offset in CopySubTexture works correctly
TEST_P(CopyTextureTest, CopySubTextureOffset)
{
    if (!checkExtensions())
    {
        return;
    }

    GLColor rgbaPixels[4 * 4] = {GLColor::red, GLColor::green, GLColor::blue, GLColor::black};
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);

    GLColor transparentPixels[4 * 4] = {GLColor::transparentBlack, GLColor::transparentBlack,
                                        GLColor::transparentBlack, GLColor::transparentBlack};
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, transparentPixels);

    // Check that FB is complete.
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 1, 1, 0, 0, 1, 1,
                             false, false, false);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);

    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 1, 0, 1, 0, 1, 1,
                             false, false, false);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(1, 0, GLColor::green);

    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 0, 1, 0, 1, 1, 1,
                             false, false, false);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 1, GLColor::blue);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    EXPECT_GL_NO_ERROR();
}

// Test that copying a texture attached to a framebuffer into a L texture does not break a
// subsequent clear
TEST_P(CopyTextureTest, ClearAfterCopySubTextureLuminance)
{
    if (!checkExtensions())
    {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 4, 4, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glCopySubTextureCHROMIUM(mTextures[1], 0, GL_TEXTURE_2D, mTextures[0], 0, 0, 0, 0, 0, 4, 4,
                             GL_TRUE, GL_FALSE, GL_TRUE);

    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that copying a texture attached to a framebuffer into a LA texture does not break a
// subsequent clear
TEST_P(CopyTextureTest, ClearAfterCopySubTextureLuminanceAlpha)
{
    if (!checkExtensions())
    {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, 4, 4, 0, GL_LUMINANCE_ALPHA,
                 GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glCopySubTextureCHROMIUM(mTextures[1], 0, GL_TEXTURE_2D, mTextures[0], 0, 0, 0, 0, 0, 4, 4,
                             GL_TRUE, GL_FALSE, GL_TRUE);

    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test every combination of copy [sub]texture parameters:
// source: ALPHA, RGB, RGBA, LUMINANCE, LUMINANCE_ALPHA, BGRA_EXT
// destination: RGB, RGBA, BGRA_EXT
// flipY: false, true
// premultiplyAlpha: false, true
// unmultiplyAlpha: false, true
namespace
{
constexpr GLenum kCopyTextureVariationsSrcFormats[] = {
    GL_ALPHA, GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_BGRA_EXT};
constexpr GLenum kCopyTextureVariationsDstFormats[] = {GL_RGB, GL_RGBA, GL_BGRA_EXT,
                                                       GL_SRGB_ALPHA_EXT};
constexpr GLint kMesaYFlips[]                       = {0, 1};
}  // anonymous namespace

TEST_P(CopyTextureVariationsTest, CopyTexture)
{
    // http://anglebug.com/42264260
    ANGLE_SKIP_TEST_IF(IsOzone());
    // http://anglebug.com/42263799
    if (std::get<1>(GetParam()) == GL_ALPHA && std::get<2>(GetParam()) == GL_RGB &&
        std::get<3>(GetParam()) && std::get<5>(GetParam()))
    {
        ANGLE_SKIP_TEST_IF(IsWindows7() && IsNVIDIA() && IsOpenGLES());
    }

    if (std::get<6>(GetParam()))
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));
    }

    testCopyTexture(GL_TEXTURE_2D, std::get<1>(GetParam()), std::get<2>(GetParam()),
                    std::get<3>(GetParam()), std::get<4>(GetParam()), std::get<5>(GetParam()),
                    std::get<6>(GetParam()));
}

TEST_P(CopyTextureVariationsTest, CopySubTexture)
{
    // http://anglebug.com/42264260
    ANGLE_SKIP_TEST_IF(IsOzone());

    if (std::get<6>(GetParam()))
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));
    }

    testCopySubTexture(GL_TEXTURE_2D, std::get<1>(GetParam()), std::get<2>(GetParam()),
                       std::get<3>(GetParam()), std::get<4>(GetParam()), std::get<5>(GetParam()),
                       std::get<6>(GetParam()));
}

TEST_P(CopyTextureVariationsTest, CopyTextureRectangle)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_texture_rectangle"));

    if (std::get<6>(GetParam()))
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));
    }

    testCopyTexture(GL_TEXTURE_RECTANGLE_ANGLE, std::get<1>(GetParam()), std::get<2>(GetParam()),
                    std::get<3>(GetParam()), std::get<4>(GetParam()), std::get<5>(GetParam()),
                    std::get<6>(GetParam()));
}

TEST_P(CopyTextureVariationsTest, CopySubTextureRectangle)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_ANGLE_texture_rectangle"));

    if (std::get<6>(GetParam()))
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));
    }

    testCopySubTexture(GL_TEXTURE_RECTANGLE_ANGLE, std::get<1>(GetParam()), std::get<2>(GetParam()),
                       std::get<3>(GetParam()), std::get<4>(GetParam()), std::get<5>(GetParam()),
                       std::get<6>(GetParam()));
}

// Test that copying to cube maps works
TEST_P(CopyTextureTest, CubeMapTarget)
{
    if (!checkExtensions())
    {
        return;
    }

    // http://anglebug.com/42260718
    ANGLE_SKIP_TEST_IF(IsMac() && IsIntel() && IsDesktopOpenGL());

    // http://anglebug.com/42261821
    ANGLE_SKIP_TEST_IF(IsFuchsia() && IsIntel() && IsVulkan());

    GLColor pixels[7] = {
        GLColor(10u, 13u, 16u, 19u), GLColor(20u, 23u, 26u, 29u), GLColor(30u, 33u, 36u, 39u),
        GLColor(40u, 43u, 46u, 49u), GLColor(50u, 53u, 56u, 59u), GLColor(60u, 63u, 66u, 69u),
        GLColor(70u, 73u, 76u, 79u),
    };

    GLTexture textures[2];

    glBindTexture(GL_TEXTURE_CUBE_MAP, textures[1]);
    for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         face++)
    {
        glTexImage2D(face, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    for (size_t i = 0; i < 2; ++i)
    {
        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
             face++)
        {
            glBindTexture(GL_TEXTURE_2D, textures[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         &pixels[face - GL_TEXTURE_CUBE_MAP_POSITIVE_X + i]);

            glCopySubTextureCHROMIUM(textures[0], 0, face, textures[1], 0, 0, 0, 0, 0, 1, 1, false,
                                     false, false);
        }

        EXPECT_GL_NO_ERROR();

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
             face++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, face, textures[1], 0);

            // Check that FB is complete.
            EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

            EXPECT_PIXEL_COLOR_EQ(0, 0, pixels[face - GL_TEXTURE_CUBE_MAP_POSITIVE_X + i]);

            EXPECT_GL_NO_ERROR();
        }
    }
}

// Test that we can successfully copy into incomplete cube maps. Regression test for
// http://anglebug.com/42262051
TEST_P(CopyTextureTest, IncompleteCubeMap)
{
    if (!checkExtensions())
    {
        return;
    }

    GLTexture texture2D;
    GLColor rgbaPixels[4 * 4] = {GLColor::red, GLColor::green, GLColor::blue, GLColor::black};
    glBindTexture(GL_TEXTURE_2D, texture2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);

    GLTexture textureCube;
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureCube);
    for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         face++)
    {
        glTexImage2D(face, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    // Set one face to 2x2
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 rgbaPixels);

    // Copy into the incomplete face
    glCopySubTextureCHROMIUM(texture2D, 0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, textureCube, 0, 0, 0, 0,
                             0, 2, 2, false, false, false);
    EXPECT_GL_NO_ERROR();
}

// Test BGRA to RGBA cube map copy
TEST_P(CopyTextureTest, CubeMapTargetBGRA)
{
    if (!checkExtensions())
    {
        return;
    }

    if (!IsGLExtensionEnabled("GL_EXT_texture_format_BGRA8888"))
    {
        return;
    }

    // http://anglebug.com/42261821
    ANGLE_SKIP_TEST_IF(IsFuchsia() && IsIntel() && IsVulkan());

    GLColor pixels[7] = {
        GLColor(10u, 13u, 16u, 19u), GLColor(20u, 23u, 26u, 29u), GLColor(30u, 33u, 36u, 39u),
        GLColor(40u, 43u, 46u, 49u), GLColor(50u, 53u, 56u, 59u), GLColor(60u, 63u, 66u, 69u),
        GLColor(70u, 73u, 76u, 79u),
    };

    GLTexture textures[2];

    glBindTexture(GL_TEXTURE_CUBE_MAP, textures[1]);
    for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         face++)
    {
        glTexImage2D(face, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    for (size_t i = 0; i < 2; ++i)
    {
        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
             face++)
        {
            glBindTexture(GL_TEXTURE_2D, textures[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 1, 1, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE,
                         &pixels[face - GL_TEXTURE_CUBE_MAP_POSITIVE_X + i]);

            glCopySubTextureCHROMIUM(textures[0], 0, face, textures[1], 0, 0, 0, 0, 0, 1, 1, false,
                                     false, false);
        }

        EXPECT_GL_NO_ERROR();

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
             face++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, face, textures[1], 0);

            // Check that FB is complete.
            EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

            GLColor converted = pixels[face - GL_TEXTURE_CUBE_MAP_POSITIVE_X + i];
            std::swap(converted.R, converted.B);
            EXPECT_PIXEL_COLOR_EQ(0, 0, converted);

            EXPECT_GL_NO_ERROR();
        }
    }
}

// Test cube map copies with RGB format
TEST_P(CopyTextureTest, CubeMapTargetRGB)
{
    if (!checkExtensions())
    {
        return;
    }

    // http://anglebug.com/42260718
    ANGLE_SKIP_TEST_IF(IsMac() && IsIntel() && IsDesktopOpenGL());

    // http://anglebug.com/42261821
    ANGLE_SKIP_TEST_IF(IsFuchsia() && IsIntel() && IsVulkan());

    constexpr uint8_t pixels[16 * 7] = {
        0u,   3u,   6u,   10u,  13u,  16u,  0, 0, 20u,  23u,  26u,  30u,  33u,  36u,  0, 0,  // 2x2
        40u,  43u,  46u,  50u,  53u,  56u,  0, 0, 60u,  63u,  66u,  70u,  73u,  76u,  0, 0,  // 2x2
        80u,  83u,  86u,  90u,  93u,  96u,  0, 0, 100u, 103u, 106u, 110u, 113u, 116u, 0, 0,  // 2x2
        120u, 123u, 126u, 130u, 133u, 136u, 0, 0, 140u, 143u, 146u, 160u, 163u, 166u, 0, 0,  // 2x2
        170u, 173u, 176u, 180u, 183u, 186u, 0, 0, 190u, 193u, 196u, 200u, 203u, 206u, 0, 0,  // 2x2
        210u, 213u, 216u, 220u, 223u, 226u, 0, 0, 230u, 233u, 236u, 240u, 243u, 246u, 0, 0,  // 2x2
        10u,  50u,  100u, 30u,  80u,  130u, 0, 0, 60u,  110u, 160u, 90u,  140u, 200u, 0, 0,  // 2x2
    };

    GLTexture textures[2];

    glBindTexture(GL_TEXTURE_CUBE_MAP, textures[1]);
    for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         face++)
    {
        glTexImage2D(face, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    for (size_t i = 0; i < 2; ++i)
    {
        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
             face++)
        {
            glBindTexture(GL_TEXTURE_2D, textures[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE,
                         &pixels[(face - GL_TEXTURE_CUBE_MAP_POSITIVE_X + i) * 16]);

            glCopySubTextureCHROMIUM(textures[0], 0, face, textures[1], 0, 0, 0, 0, 0, 2, 2, false,
                                     false, false);
        }

        EXPECT_GL_NO_ERROR();

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
             face++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, face, textures[1], 0);

            // Check that FB is complete.
            EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

            const uint8_t *faceData = &pixels[(face - GL_TEXTURE_CUBE_MAP_POSITIVE_X + i) * 16];
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(faceData[0], faceData[1], faceData[2], 255));
            EXPECT_PIXEL_COLOR_EQ(1, 0, GLColor(faceData[3], faceData[4], faceData[5], 255));
            EXPECT_PIXEL_COLOR_EQ(0, 1, GLColor(faceData[8], faceData[9], faceData[10], 255));
            EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor(faceData[11], faceData[12], faceData[13], 255));

            EXPECT_GL_NO_ERROR();
        }
    }
}

// Test that copying to non-zero mipmaps works
TEST_P(CopyTextureTest, CopyToMipmap)
{
    if (!checkExtensions())
    {
        return;
    }

    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_OES_fbo_render_mipmap"));

    GLColor pixels[] = {GLColor::red, GLColor::red, GLColor::red, GLColor::red};

    GLTexture textures[2];

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    std::vector<std::pair<GLint, GLint>> soureDestPairs;
    soureDestPairs.push_back(std::make_pair(0, 1));

    // ES3 allows copying from non-zero mips
    if (getClientMajorVersion() >= 3)
    {
        soureDestPairs.push_back(std::make_pair(1, 2));
    }

    for (const auto &sourceDestPair : soureDestPairs)
    {
        const GLint sourceLevel = sourceDestPair.first;
        const GLint destLevel   = sourceDestPair.second;

        glCopyTextureCHROMIUM(textures[0], sourceLevel, GL_TEXTURE_2D, textures[1], destLevel,
                              GL_RGBA, GL_UNSIGNED_BYTE, false, false, false);

        EXPECT_GL_NO_ERROR();

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1],
                               destLevel);

        // Check that FB is complete.
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

        EXPECT_PIXEL_COLOR_EQ(0, 0, pixels[0]);

        EXPECT_GL_NO_ERROR();
    }
}

// Test that copying outside the mipmap range works
TEST_P(CopyTextureTest, CopyOutsideMipmap)
{
    if (!checkExtensions())
    {
        return;
    }

    // http://anglebug.com/42263316
    ANGLE_SKIP_TEST_IF(IsD3D());

    // http://anglebug.com/42263799
    ANGLE_SKIP_TEST_IF(IsWindows() && IsNVIDIA() && IsOpenGL());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program);
    GLint textureLoc = glGetUniformLocation(program, essl1_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLoc);
    glUniform1i(textureLoc, 0);

    GLTexture textures[2];

    // Create two single-mip textures.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Commit texture 0
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    drawQuad(program, std::string(essl1_shaders::PositionAttrib()), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Copy texture 1 into mip 1 of texture 0.  This mip is outside the range of the image allocated
    // for texture 0.
    glCopyTextureCHROMIUM(textures[1], 0, GL_TEXTURE_2D, textures[0], 1, GL_RGBA, GL_UNSIGNED_BYTE,
                          false, false, false);
    EXPECT_GL_NO_ERROR();

    // Draw with texture 0 again
    drawQuad(program, std::string(essl1_shaders::PositionAttrib()), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that copying from an RGBA8 texture to RGBA4 results in exactly 4-bit precision in the result
TEST_P(CopyTextureTest, DownsampleRGBA4444)
{
    // Downsampling on copy is only guarenteed on D3D11
    ANGLE_SKIP_TEST_IF(!IsD3D11());

    GLTexture textures[2];

    GLColor pixels[] = {GLColor(0, 5, 6, 7), GLColor(17, 22, 25, 24), GLColor(34, 35, 36, 36),
                        GLColor(51, 53, 55, 55)};

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glCopyTextureCHROMIUM(textures[0], 0, GL_TEXTURE_2D, textures[1], 0, GL_RGBA,
                          GL_UNSIGNED_SHORT_4_4_4_4, GL_FALSE, GL_FALSE, GL_FALSE);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0, 0, 0, 0), 1.0);
    EXPECT_PIXEL_COLOR_NEAR(1, 0, GLColor(17, 17, 17, 17), 1.0);
    EXPECT_PIXEL_COLOR_NEAR(0, 1, GLColor(34, 34, 34, 34), 1.0);
    EXPECT_PIXEL_COLOR_NEAR(1, 1, GLColor(51, 51, 51, 51), 1.0);

    testGradientDownsampleUniqueValues(GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, {16, 16, 16, 16});
}

// Test that copying from an RGBA8 texture to RGB565 results in exactly 4-bit precision in the
// result
TEST_P(CopyTextureTest, DownsampleRGB565)
{
    // Downsampling on copy is only guarenteed on D3D11
    ANGLE_SKIP_TEST_IF(!IsD3D11());

    GLTexture textures[2];

    GLColor pixels[] = {GLColor(0, 5, 2, 14), GLColor(17, 22, 25, 30), GLColor(34, 33, 36, 46),
                        GLColor(50, 54, 49, 60)};

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glCopyTextureCHROMIUM(textures[0], 0, GL_TEXTURE_2D, textures[1], 0, GL_RGB,
                          GL_UNSIGNED_SHORT_5_6_5, GL_FALSE, GL_FALSE, GL_FALSE);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0, 4, 0, 255), 1.0);
    EXPECT_PIXEL_COLOR_NEAR(1, 0, GLColor(16, 20, 25, 255), 1.0);
    EXPECT_PIXEL_COLOR_NEAR(0, 1, GLColor(33, 32, 33, 255), 1.0);
    EXPECT_PIXEL_COLOR_NEAR(1, 1, GLColor(49, 53, 49, 255), 1.0);

    testGradientDownsampleUniqueValues(GL_RGB, GL_UNSIGNED_SHORT_5_6_5, {32, 64, 32, 1});
}

// Test that copying from an RGBA8 texture to RGBA5551 results in exactly 4-bit precision in the
// result
TEST_P(CopyTextureTest, DownsampleRGBA5551)
{
    // Downsampling on copy is only guarenteed on D3D11
    ANGLE_SKIP_TEST_IF(!IsD3D11());

    GLTexture textures[2];

    GLColor pixels[] = {GLColor(0, 1, 2, 3), GLColor(14, 16, 17, 18), GLColor(33, 34, 36, 46),
                        GLColor(50, 51, 52, 255)};

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glCopyTextureCHROMIUM(textures[0], 0, GL_TEXTURE_2D, textures[1], 0, GL_RGBA,
                          GL_UNSIGNED_SHORT_5_5_5_1, GL_FALSE, GL_FALSE, GL_FALSE);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0, 0, 0, 0), 1.0);
    EXPECT_PIXEL_COLOR_NEAR(1, 0, GLColor(16, 16, 16, 0), 1.0);
    EXPECT_PIXEL_COLOR_NEAR(0, 1, GLColor(33, 33, 33, 0), 1.0);
    EXPECT_PIXEL_COLOR_NEAR(1, 1, GLColor(49, 49, 49, 255), 1.0);

    testGradientDownsampleUniqueValues(GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, {32, 32, 32, 2});
}

// Test to ensure that CopyTexture works with LUMINANCE texture as a destination
TEST_P(CopyTextureTestDest, Luminance)
{
    if (!checkExtensions())
    {
        return;
    }

    GLColor originalPixels(50u, 100u, 150u, 200u);
    GLColor expectedPixels(50u, 50u, 50u, 255u);

    // ReadPixels doesn't work with LUMINANCE (non-renderable), so we copy again back to an RGBA
    // texture to verify contents.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &originalPixels);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);

    glCopyTextureCHROMIUM(mTextures[1], 0, GL_TEXTURE_2D, mTextures[0], 0, GL_LUMINANCE,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();

    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, expectedPixels);
}

// Test to ensure that CopyTexture works with LUMINANCE texture as a destination with
// UnpackPremultiply parameter
TEST_P(CopyTextureTestDest, LuminanceMultiply)
{
    if (!checkExtensions())
    {
        return;
    }

    GLColor originalPixels(50u, 100u, 150u, 200u);
    GLColor expectedPixels(39u, 39u, 39u, 255u);

    // ReadPixels doesn't work with LUMINANCE (non-renderable), so we copy again back to an RGBA
    // texture to verify contents.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &originalPixels);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);

    glCopyTextureCHROMIUM(mTextures[1], 0, GL_TEXTURE_2D, mTextures[0], 0, GL_LUMINANCE,
                          GL_UNSIGNED_BYTE, false, true, false);

    EXPECT_GL_NO_ERROR();

    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, expectedPixels);
}

// Test to ensure that CopyTexture works with LUMINANCE texture as a destination with
// UnpackUnmultiply parameter
TEST_P(CopyTextureTestDest, LuminanceUnmultiply)
{
    if (!checkExtensions())
    {
        return;
    }

    GLColor originalPixels(50u, 100u, 150u, 200u);
    GLColor expectedPixels(64u, 64u, 64u, 255u);

    // ReadPixels doesn't work with LUMINANCE (non-renderable), so we copy again back to an RGBA
    // texture to verify contents.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &originalPixels);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1, 1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);

    glCopyTextureCHROMIUM(mTextures[1], 0, GL_TEXTURE_2D, mTextures[0], 0, GL_LUMINANCE,
                          GL_UNSIGNED_BYTE, false, false, true);

    EXPECT_GL_NO_ERROR();

    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, expectedPixels);
}

// Test to ensure that CopyTexture works with LUMINANCE_ALPHA texture as a destination
TEST_P(CopyTextureTestDest, LuminanceAlpha)
{
    if (!checkExtensions())
    {
        return;
    }

    GLColor originalPixels(50u, 100u, 150u, 200u);
    GLColor expectedPixels(50u, 50u, 50u, 200u);

    // ReadPixels doesn't work with LUMINANCE_ALPHA (non-renderable), so we copy again back to an
    // RGBA texture to verify contents.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &originalPixels);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, 1, 1, 0, GL_LUMINANCE_ALPHA,
                 GL_UNSIGNED_BYTE, nullptr);

    glCopyTextureCHROMIUM(mTextures[1], 0, GL_TEXTURE_2D, mTextures[0], 0, GL_LUMINANCE_ALPHA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();

    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_PIXEL_COLOR_EQ(0, 0, expectedPixels);
}

// Test to ensure that CopyTexture works with LUMINANCE_ALPHA texture as a destination with
// UnpackPremultiply parameter
TEST_P(CopyTextureTestDest, LuminanceAlphaMultiply)
{
    if (!checkExtensions())
    {
        return;
    }

    GLColor originalPixels(50u, 100u, 150u, 200u);
    GLColor expectedPixels(39u, 39u, 39u, 200u);

    // ReadPixels doesn't work with LUMINANCE_ALPHA (non-renderable), so we copy again back to an
    // RGBA texture to verify contents.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &originalPixels);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, 1, 1, 0, GL_LUMINANCE_ALPHA,
                 GL_UNSIGNED_BYTE, nullptr);

    glCopyTextureCHROMIUM(mTextures[1], 0, GL_TEXTURE_2D, mTextures[0], 0, GL_LUMINANCE_ALPHA,
                          GL_UNSIGNED_BYTE, false, true, false);

    EXPECT_GL_NO_ERROR();

    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_PIXEL_COLOR_EQ(0, 0, expectedPixels);
}

// Test to ensure that CopyTexture works with LUMINANCE_ALPHA texture as a destination with
// UnpackUnmultiplyAlpha parameter
TEST_P(CopyTextureTestDest, LuminanceAlphaUnmultiply)
{
    if (!checkExtensions())
    {
        return;
    }

    GLColor originalPixels(50u, 100u, 150u, 200u);
    GLColor expectedPixels(64u, 64u, 64u, 200u);

    // ReadPixels doesn't work with LUMINANCE_ALPHA (non-renderable), so we copy again back to an
    // RGBA texture to verify contents.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &originalPixels);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, 1, 1, 0, GL_LUMINANCE_ALPHA,
                 GL_UNSIGNED_BYTE, nullptr);

    glCopyTextureCHROMIUM(mTextures[1], 0, GL_TEXTURE_2D, mTextures[0], 0, GL_LUMINANCE_ALPHA,
                          GL_UNSIGNED_BYTE, false, false, true);

    EXPECT_GL_NO_ERROR();

    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, expectedPixels);
}

// Test to ensure that CopyTexture works with ALPHA texture as a destination
TEST_P(CopyTextureTestDest, Alpha)
{
    if (!checkExtensions())
    {
        return;
    }

    GLColor originalPixels(50u, 100u, 150u, 155u);
    GLColor expectedPixels(0u, 0u, 0u, 155u);

    // ReadPixels doesn't work with ALPHA (non-renderable), so we copy again back to an RGBA
    // texture to verify contents.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &originalPixels);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1, 1, 0, GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);

    glCopyTextureCHROMIUM(mTextures[1], 0, GL_TEXTURE_2D, mTextures[0], 0, GL_ALPHA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();

    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, expectedPixels);
}

// Test to ensure that CopyTexture works with ALPHA texture as a destination with
// UnpackPremultiplyAlpha parameter
TEST_P(CopyTextureTestDest, AlphaMultiply)
{
    if (!checkExtensions())
    {
        return;
    }

    GLColor originalPixels(50u, 100u, 150u, 155u);
    GLColor expectedPixels(0u, 0u, 0u, 155u);

    // ReadPixels doesn't work with ALPHA (non-renderable), so we copy again back to an RGBA
    // texture to verify contents.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &originalPixels);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1, 1, 0, GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);

    glCopyTextureCHROMIUM(mTextures[1], 0, GL_TEXTURE_2D, mTextures[0], 0, GL_ALPHA,
                          GL_UNSIGNED_BYTE, false, true, false);

    EXPECT_GL_NO_ERROR();

    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, expectedPixels);
}

// Test to ensure that CopyTexture works with ALPHA texture as a destination with
// UnpackUnmultiplyAlpha parameter
TEST_P(CopyTextureTestDest, AlphaUnmultiply)
{
    if (!checkExtensions())
    {
        return;
    }

    GLColor originalPixels(50u, 100u, 150u, 155u);
    GLColor expectedPixels(0u, 0u, 0u, 155u);

    // ReadPixels doesn't work with ALPHA (non-renderable), so we copy again back to an RGBA
    // texture to verify contents.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &originalPixels);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1, 1, 0, GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);

    glCopyTextureCHROMIUM(mTextures[1], 0, GL_TEXTURE_2D, mTextures[0], 0, GL_ALPHA,
                          GL_UNSIGNED_BYTE, false, false, true);

    EXPECT_GL_NO_ERROR();

    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, expectedPixels);
}

// Test to ensure that CopyTexture uses the correct ALPHA passthrough shader to ensure RGB channels
// are set to 0.
TEST_P(CopyTextureTestDest, AlphaCopyWithRGB)
{
    // http://anglebug.com/40644706
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGLES());
    ANGLE_SKIP_TEST_IF(!checkExtensions());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_half_float"));

    GLColor originalPixels(50u, 100u, 150u, 155u);
    GLColor expectedPixels(0u, 0u, 0u, 155u);

    // ReadPixels doesn't work with ALPHA (non-renderable), so we copy again back to an RGBA
    // texture to verify contents.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &originalPixels);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1, 1, 0, GL_ALPHA, GL_HALF_FLOAT_OES, nullptr);

    glCopyTextureCHROMIUM(mTextures[1], 0, GL_TEXTURE_2D, mTextures[0], 0, GL_ALPHA,
                          GL_HALF_FLOAT_OES, false, false, false);

    EXPECT_GL_NO_ERROR();

    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, expectedPixels);
}

// Bug where TEXTURE_SWIZZLE_RGBA was not reset after the Luminance workaround. (crbug.com/1022080)
TEST_P(CopyTextureTestES3, LuminanceWorkaroundTextureSwizzleBug)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_sRGB"));

    {
        GLColor pixels(50u, 20u, 100u, 150u);

        // Hit BlitGL::copySubImageToLUMAWorkaroundTexture by copying an ALPHA texture
        GLTexture srcTexture;
        glBindTexture(GL_TEXTURE_2D, srcTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels);

        GLFramebuffer srcFBO;
        glBindFramebuffer(GL_FRAMEBUFFER, srcFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTexture, 0);

        GLTexture dstTexture;
        glBindTexture(GL_TEXTURE_2D, dstTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 1, 1, 0, GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);

        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1, 1);
        EXPECT_GL_NO_ERROR();
    }

    {
        // This time hit BlitGL::blitColorBufferWithShader by copying an SRGB texture
        GLColor pixels(100u, 200u, 50u, 210u);

        GLTexture srcTexture;
        glBindTexture(GL_TEXTURE_2D, srcTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA_EXT, 1, 1, 0, GL_SRGB_ALPHA_EXT,
                     GL_UNSIGNED_BYTE, &pixels);

        GLFramebuffer srcFBO;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTexture,
                               0);

        GLTexture dstTexture;
        glBindTexture(GL_TEXTURE_2D, dstTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA_EXT, 1, 1, 0, GL_SRGB_ALPHA_EXT,
                     GL_UNSIGNED_BYTE, nullptr);

        GLFramebuffer dstFBO;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFBO);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTexture,
                               0);

        glBlitFramebuffer(0, 0, 1, 1, 0, 0, 1, 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // The previous workaround should not affect this copy
        glBindFramebuffer(GL_FRAMEBUFFER, dstFBO);
        EXPECT_PIXEL_COLOR_EQ(0, 0, pixels);
    }
}

// Test to ensure that CopyTexture will fail with a non-zero level and NPOT texture in WebGL
TEST_P(CopyTextureTestWebGL, NPOT)
{
    if (IsGLExtensionRequestable("GL_CHROMIUM_copy_texture"))
    {
        glRequestExtensionANGLE("GL_CHROMIUM_copy_texture");
    }
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_copy_texture"));

    std::vector<GLColor> pixelData(10 * 10, GLColor::red);

    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 10, 10, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());

    // Do a basic copy to make sure things work
    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Do the same operation with destLevel 1, which should fail
    glCopyTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 1, GL_RGBA,
                          GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test the newly added ES3 unorm formats
TEST_P(CopyTextureTestES3, ES3UnormFormats)
{
    if (!checkExtensions())
    {
        return;
    }
    // http://anglebug.com/40096654
    ANGLE_SKIP_TEST_IF(IsAndroid());

    // http://anglebug.com/42263690
    ANGLE_SKIP_TEST_IF(IsWindows() && IsOpenGL() && IsIntel());

    auto testOutput = [this](GLuint texture, const GLColor &expectedColor) {
        constexpr char kVS[] =
            "#version 300 es\n"
            "in vec4 position;\n"
            "out vec2 texcoord;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(position.xy, 0.0, 1.0);\n"
            "    texcoord = (position.xy * 0.5) + 0.5;\n"
            "}\n";

        constexpr char kFS[] =
            "#version 300 es\n"
            "precision mediump float;\n"
            "uniform sampler2D tex;\n"
            "in vec2 texcoord;\n"
            "out vec4 color;\n"
            "void main()\n"
            "{\n"
            "    color = texture(tex, texcoord);\n"
            "}\n";

        ANGLE_GL_PROGRAM(program, kVS, kFS);
        glUseProgram(program);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1, 1);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glUniform1i(glGetUniformLocation(program, "tex"), 0);

        drawQuad(program, "position", 0.5f, 1.0f, true);

        EXPECT_PIXEL_COLOR_NEAR(0, 0, expectedColor, 1.0);
    };

    auto testCopyCombination = [testOutput](GLenum sourceInternalFormat, GLenum sourceFormat,
                                            GLenum sourceType, const GLColor &sourceColor,
                                            GLenum destInternalFormat, GLenum destType, bool flipY,
                                            bool premultiplyAlpha, bool unmultiplyAlpha,
                                            const GLColor &expectedColor) {
        GLTexture sourceTexture;
        glBindTexture(GL_TEXTURE_2D, sourceTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, sourceInternalFormat, 1, 1, 0, sourceFormat, sourceType,
                     &sourceColor);

        GLTexture destTexture;
        glBindTexture(GL_TEXTURE_2D, destTexture);

        glCopyTextureCHROMIUM(sourceTexture, 0, GL_TEXTURE_2D, destTexture, 0, destInternalFormat,
                              destType, flipY, premultiplyAlpha, unmultiplyAlpha);
        ASSERT_GL_NO_ERROR();

        testOutput(destTexture, expectedColor);
    };

    auto testSubCopyCombination = [testOutput](GLenum sourceInternalFormat, GLenum sourceFormat,
                                               GLenum sourceType, const GLColor &sourceColor,
                                               GLenum destInternalFormat, GLenum destFormat,
                                               GLenum destType, bool flipY, bool premultiplyAlpha,
                                               bool unmultiplyAlpha, const GLColor &expectedColor) {
        GLTexture sourceTexture;
        glBindTexture(GL_TEXTURE_2D, sourceTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, sourceInternalFormat, 1, 1, 0, sourceFormat, sourceType,
                     &sourceColor);

        GLTexture destTexture;
        glBindTexture(GL_TEXTURE_2D, destTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, destInternalFormat, 1, 1, 0, destFormat, destType, nullptr);
        glCopySubTextureCHROMIUM(sourceTexture, 0, GL_TEXTURE_2D, destTexture, 0, 0, 0, 0, 0, 1, 1,
                                 flipY, premultiplyAlpha, unmultiplyAlpha);
        ASSERT_GL_NO_ERROR();

        testOutput(destTexture, expectedColor);
    };

    // New LUMA source formats
    testCopyCombination(GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, GLColor(128, 0, 0, 0), GL_RGB,
                        GL_UNSIGNED_BYTE, false, false, false, GLColor(128, 128, 128, 255));
    testCopyCombination(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
                        GLColor(128, 64, 0, 0), GL_RGB, GL_UNSIGNED_BYTE, false, false, false,
                        GLColor(128, 128, 128, 255));
    testCopyCombination(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
                        GLColor(128, 64, 0, 0), GL_RGB, GL_UNSIGNED_BYTE, false, true, false,
                        GLColor(32, 32, 32, 255));
    testCopyCombination(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
                        GLColor(128, 128, 0, 0), GL_RGB, GL_UNSIGNED_BYTE, false, false, true,
                        GLColor(255, 255, 255, 255));
    testCopyCombination(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, GLColor(128, 0, 0, 0), GL_RGBA,
                        GL_UNSIGNED_BYTE, false, false, false, GLColor(0, 0, 0, 128));
    testCopyCombination(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, GLColor(128, 0, 0, 0), GL_RGBA,
                        GL_UNSIGNED_BYTE, false, false, true, GLColor(0, 0, 0, 128));
    testCopyCombination(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, GLColor(128, 0, 0, 0), GL_RGBA,
                        GL_UNSIGNED_BYTE, false, true, false, GLColor(0, 0, 0, 128));

    // New sRGB dest formats
    if (IsGLExtensionEnabled("GL_EXT_sRGB"))
    {
        testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_SRGB,
                            GL_UNSIGNED_BYTE, false, false, false, GLColor(55, 13, 4, 255));
        testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_SRGB,
                            GL_UNSIGNED_BYTE, false, true, false, GLColor(13, 4, 1, 255));
        testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128),
                            GL_SRGB_ALPHA_EXT, GL_UNSIGNED_BYTE, false, false, false,
                            GLColor(55, 13, 4, 128));

        testSubCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128),
                               GL_SRGB, GL_SRGB, GL_UNSIGNED_BYTE, false, false, false,
                               GLColor(55, 13, 4, 255));
        testSubCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128),
                               GL_SRGB, GL_SRGB, GL_UNSIGNED_BYTE, false, true, false,
                               GLColor(13, 4, 1, 255));
        testSubCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128),
                               GL_SRGB_ALPHA_EXT, GL_SRGB_ALPHA_EXT, GL_UNSIGNED_BYTE, false, false,
                               false, GLColor(55, 13, 4, 128));
    }
}

// Test the newly added ES3 float formats
TEST_P(CopyTextureTestES3, ES3FloatFormats)
{
    if (!checkExtensions())
    {
        return;
    }

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_color_buffer_float"));

    auto testOutput = [this](GLuint texture, const GLColor32F &expectedColor) {
        constexpr char kVS[] =
            "#version 300 es\n"
            "in vec4 position;\n"
            "out vec2 texcoord;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(position.xy, 0.0, 1.0);\n"
            "    texcoord = (position.xy * 0.5) + 0.5;\n"
            "}\n";

        constexpr char kFS[] =
            "#version 300 es\n"
            "precision mediump float;\n"
            "uniform sampler2D tex;\n"
            "in vec2 texcoord;\n"
            "out vec4 color;\n"
            "void main()\n"
            "{\n"
            "    color = texture(tex, texcoord);\n"
            "}\n";

        ANGLE_GL_PROGRAM(program, kVS, kFS);
        glUseProgram(program);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, 1, 1);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glUniform1i(glGetUniformLocation(program, "tex"), 0);

        drawQuad(program, "position", 0.5f, 1.0f, true);

        EXPECT_PIXEL_COLOR32F_NEAR(0, 0, expectedColor, 0.05);
    };

    auto testCopyCombination = [testOutput](GLenum sourceInternalFormat, GLenum sourceFormat,
                                            GLenum sourceType, const GLColor &sourceColor,
                                            GLenum destInternalFormat, GLenum destType, bool flipY,
                                            bool premultiplyAlpha, bool unmultiplyAlpha,
                                            const GLColor32F &expectedColor) {
        GLTexture sourceTexture;
        glBindTexture(GL_TEXTURE_2D, sourceTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, sourceInternalFormat, 1, 1, 0, sourceFormat, sourceType,
                     &sourceColor);

        GLTexture destTexture;
        glBindTexture(GL_TEXTURE_2D, destTexture);

        glCopyTextureCHROMIUM(sourceTexture, 0, GL_TEXTURE_2D, destTexture, 0, destInternalFormat,
                              destType, flipY, premultiplyAlpha, unmultiplyAlpha);
        ASSERT_GL_NO_ERROR();

        testOutput(destTexture, expectedColor);
    };

    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGBA32F,
                        GL_FLOAT, false, false, false, GLColor32F(0.5f, 0.25f, 0.125f, 0.5f));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGBA32F,
                        GL_FLOAT, false, true, false, GLColor32F(0.25f, 0.125f, 0.0625f, 0.5f));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGBA32F,
                        GL_FLOAT, false, false, true, GLColor32F(1.0f, 0.5f, 0.25f, 0.5f));

    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_R16F,
                        GL_FLOAT, false, false, false, GLColor32F(0.5f, 0.0f, 0.0f, 1.0f));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_R16F,
                        GL_FLOAT, false, true, false, GLColor32F(0.25f, 0.0f, 0.0f, 1.0f));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_R16F,
                        GL_FLOAT, false, false, true, GLColor32F(1.0f, 0.0f, 0.0f, 1.0f));

    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RG16F,
                        GL_FLOAT, false, false, false, GLColor32F(0.5f, 0.25f, 0.0f, 1.0f));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RG16F,
                        GL_FLOAT, false, true, false, GLColor32F(0.25f, 0.125f, 0.0f, 1.0f));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RG16F,
                        GL_FLOAT, false, false, true, GLColor32F(1.0f, 0.5f, 0.0f, 1.0f));

    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGB16F,
                        GL_FLOAT, false, false, false, GLColor32F(0.5f, 0.25f, 0.125f, 1.0f));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGB16F,
                        GL_FLOAT, false, true, false, GLColor32F(0.25f, 0.125f, 0.0625f, 1.0f));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGB16F,
                        GL_FLOAT, false, false, true, GLColor32F(1.0f, 0.5f, 0.25f, 1.0f));

    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128),
                        GL_R11F_G11F_B10F, GL_FLOAT, false, false, false,
                        GLColor32F(0.5f, 0.25f, 0.125f, 1.0f));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128),
                        GL_R11F_G11F_B10F, GL_FLOAT, false, true, false,
                        GLColor32F(0.25f, 0.125f, 0.0625f, 1.0f));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128),
                        GL_R11F_G11F_B10F, GL_FLOAT, false, false, true,
                        GLColor32F(1.0f, 0.5f, 0.25f, 1.0f));

    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGB9_E5,
                        GL_FLOAT, false, false, false, GLColor32F(0.5f, 0.25f, 0.125f, 1.0f));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGB9_E5,
                        GL_FLOAT, false, true, false, GLColor32F(0.25f, 0.125f, 0.0625f, 1.0f));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGB9_E5,
                        GL_FLOAT, false, false, true, GLColor32F(1.0f, 0.5f, 0.25f, 1.0f));
}

// Test the newly added ES3 unsigned integer formats
TEST_P(CopyTextureTestES3, ES3UintFormats)
{
    ANGLE_SKIP_TEST_IF(IsLinux() && IsOpenGL() && IsIntel());

    if (!checkExtensions())
    {
        return;
    }

    using GLColor32U = std::tuple<GLuint, GLuint, GLuint, GLuint>;

    auto testOutput = [this](GLuint texture, const GLColor32U &expectedColor) {
        constexpr char kVS[] =
            "#version 300 es\n"
            "in vec4 position;\n"
            "out vec2 texcoord;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(position.xy, 0.0, 1.0);\n"
            "    texcoord = (position.xy * 0.5) + 0.5;\n"
            "}\n";

        constexpr char kFS[] =
            "#version 300 es\n"
            "precision mediump float;\n"
            "precision mediump usampler2D;\n"
            "in vec2 texcoord;\n"
            "uniform usampler2D tex;\n"
            "out uvec4 color;\n"
            "void main()\n"
            "{\n"
            "    color = texture(tex, texcoord);\n"
            "}\n";

        ANGLE_GL_PROGRAM(program, kVS, kFS);
        glUseProgram(program);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8UI, 1, 1);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glUniform1i(glGetUniformLocation(program, "tex"), 0);

        drawQuad(program, "position", 0.5f, 1.0f, true);
        ASSERT_GL_NO_ERROR();

        GLuint pixel[4] = {0};
        glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_UNSIGNED_INT, pixel);
        ASSERT_GL_NO_ERROR();
        EXPECT_NEAR(std::get<0>(expectedColor), pixel[0], 1);
        EXPECT_NEAR(std::get<1>(expectedColor), pixel[1], 1);
        EXPECT_NEAR(std::get<2>(expectedColor), pixel[2], 1);
        EXPECT_NEAR(std::get<3>(expectedColor), pixel[3], 1);
    };

    auto testCopyCombination = [testOutput](GLenum sourceInternalFormat, GLenum sourceFormat,
                                            GLenum sourceType, const GLColor &sourceColor,
                                            GLenum destInternalFormat, GLenum destType, bool flipY,
                                            bool premultiplyAlpha, bool unmultiplyAlpha,
                                            const GLColor32U &expectedColor) {
        GLTexture sourceTexture;
        glBindTexture(GL_TEXTURE_2D, sourceTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, sourceInternalFormat, 1, 1, 0, sourceFormat, sourceType,
                     &sourceColor);

        GLTexture destTexture;
        glBindTexture(GL_TEXTURE_2D, destTexture);

        glCopyTextureCHROMIUM(sourceTexture, 0, GL_TEXTURE_2D, destTexture, 0, destInternalFormat,
                              destType, flipY, premultiplyAlpha, unmultiplyAlpha);
        ASSERT_GL_NO_ERROR();

        testOutput(destTexture, expectedColor);
    };

    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGBA8UI,
                        GL_UNSIGNED_BYTE, false, false, false, GLColor32U(128, 64, 32, 128));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGBA8UI,
                        GL_UNSIGNED_BYTE, false, true, false, GLColor32U(64, 32, 16, 128));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGBA8UI,
                        GL_UNSIGNED_BYTE, false, false, true, GLColor32U(255, 128, 64, 128));

    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGB8UI,
                        GL_UNSIGNED_BYTE, false, false, false, GLColor32U(128, 64, 32, 1));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGB8UI,
                        GL_UNSIGNED_BYTE, false, true, false, GLColor32U(64, 32, 16, 1));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RGB8UI,
                        GL_UNSIGNED_BYTE, false, false, true, GLColor32U(255, 128, 64, 1));

    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RG8UI,
                        GL_UNSIGNED_BYTE, false, false, false, GLColor32U(128, 64, 0, 1));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RG8UI,
                        GL_UNSIGNED_BYTE, false, true, false, GLColor32U(64, 32, 0, 1));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_RG8UI,
                        GL_UNSIGNED_BYTE, false, false, true, GLColor32U(255, 128, 0, 1));

    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_R8UI,
                        GL_UNSIGNED_BYTE, false, false, false, GLColor32U(128, 0, 0, 1));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(128, 64, 32, 128), GL_R8UI,
                        GL_UNSIGNED_BYTE, false, true, false, GLColor32U(64, 0, 0, 1));
    testCopyCombination(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GLColor(120, 64, 32, 128), GL_R8UI,
                        GL_UNSIGNED_BYTE, false, false, true, GLColor32U(240, 0, 0, 1));
}

// Test that using an offset in CopySubTexture works correctly for non-renderable float targets
TEST_P(CopyTextureTestES3, CopySubTextureOffsetNonRenderableFloat)
{
    if (!checkExtensions())
    {
        return;
    }

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_color_buffer_float"));

    auto testOutput = [this](GLuint texture, const GLColor32F &expectedColor) {
        constexpr char kVS[] =
            "#version 300 es\n"
            "in vec4 position;\n"
            "out vec2 texcoord;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(position.xy, 0.0, 1.0);\n"
            "    texcoord = (position.xy * 0.5) + 0.5;\n"
            "}\n";

        constexpr char kFS[] =
            "#version 300 es\n"
            "precision mediump float;\n"
            "uniform sampler2D tex;\n"
            "in vec2 texcoord;\n"
            "out vec4 color;\n"
            "void main()\n"
            "{\n"
            "    color = texture(tex, texcoord);\n"
            "}\n";

        ANGLE_GL_PROGRAM(program, kVS, kFS);
        glUseProgram(program);

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, 1, 1);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glUniform1i(glGetUniformLocation(program, "tex"), 0);

        drawQuad(program, "position", 0.5f, 1.0f, true);

        EXPECT_PIXEL_COLOR32F_NEAR(0, 0, expectedColor, 0.05);
    };

    auto testCopy = [this, testOutput](GLenum destInternalFormat, GLenum destFormat,
                                       GLenum destType) {
        GLColor rgbaPixels[4 * 4] = {GLColor::red, GLColor::green, GLColor::blue, GLColor::black};
        glBindTexture(GL_TEXTURE_2D, mTextures[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);

        GLTexture destTexture;
        glBindTexture(GL_TEXTURE_2D, destTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, destInternalFormat, 1, 1, 0, destFormat, destType, nullptr);

        glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, destTexture, 0, 0, 0, 0, 0, 1, 1,
                                 false, false, false);
        EXPECT_GL_NO_ERROR();
        testOutput(destTexture, kFloatRed);

        glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, destTexture, 0, 0, 0, 1, 0, 1, 1,
                                 false, false, false);
        EXPECT_GL_NO_ERROR();
        testOutput(destTexture, kFloatGreen);

        glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, destTexture, 0, 0, 0, 0, 1, 1, 1,
                                 false, false, false);
        EXPECT_GL_NO_ERROR();
        testOutput(destTexture, kFloatBlue);

        glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, destTexture, 0, 0, 0, 1, 1, 1, 1,
                                 false, false, false);
        EXPECT_GL_NO_ERROR();
        testOutput(destTexture, kFloatBlack);
    };

    testCopy(GL_RGB9_E5, GL_RGB, GL_FLOAT);
}

// Test that copying from one mip to another works
TEST_P(CopyTextureTestES3, CopyBetweenMips)
{
    if (!checkExtensions())
    {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Texture2DLod(), essl3_shaders::fs::Texture2DLod());
    glUseProgram(program);
    GLint textureLoc = glGetUniformLocation(program, essl3_shaders::Texture2DUniform());
    GLint lodLoc     = glGetUniformLocation(program, essl3_shaders::LodUniform());
    ASSERT_NE(-1, textureLoc);
    ASSERT_NE(-1, lodLoc);
    glUniform1i(textureLoc, 0);
    glUniform1f(lodLoc, 0);

    GLTexture texture;

    // Create a texture with 3 mips.  Mip0 will contain an image as follows:
    //
    //     G G B G
    //     G R R G
    //     G R R G
    //     G G G G
    //
    // The 2x2 red square and 1x1 blue square will be copied to the other mips.
    const GLColor kMip0InitColor[4 * 4] = {
        GLColor::green, GLColor::green, GLColor::blue,  GLColor::green,
        GLColor::green, GLColor::red,   GLColor::red,   GLColor::green,
        GLColor::green, GLColor::red,   GLColor::red,   GLColor::green,
        GLColor::green, GLColor::green, GLColor::green, GLColor::green,
    };
    const GLColor kMipOtherInitColor[4] = {
        GLColor::black,
        GLColor::black,
        GLColor::black,
        GLColor::black,
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, kMip0InitColor);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, kMipOtherInitColor);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, kMipOtherInitColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Commit texture
    glUniform1f(lodLoc, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, kMip0InitColor[0]);

    glUniform1f(lodLoc, 1);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, kMipOtherInitColor[0]);

    glUniform1f(lodLoc, 2);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, kMipOtherInitColor[0]);

    ASSERT_GL_NO_ERROR();

    // Copy from mip 0 to mip 1.  The level is not redefined, so a direct copy can potentially be
    // done.
    glCopySubTextureCHROMIUM(texture, 0, GL_TEXTURE_2D, texture, 1, 0, 0, 1, 1, 2, 2, false, false,
                             false);
    EXPECT_GL_NO_ERROR();

    // Copy from mip 0 to mip 2.  Again, the level is not redefined.
    glCopySubTextureCHROMIUM(texture, 0, GL_TEXTURE_2D, texture, 2, 0, 0, 2, 0, 1, 1, false, false,
                             false);
    EXPECT_GL_NO_ERROR();

    // Verify mips 1 and 2.
    int w = getWindowWidth() - 1;
    int h = getWindowHeight() - 1;

    glUniform1f(lodLoc, 1);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, kMip0InitColor[4 * 1 + 1]);
    EXPECT_PIXEL_COLOR_EQ(w, 0, kMip0InitColor[4 * 1 + 2]);
    EXPECT_PIXEL_COLOR_EQ(0, h, kMip0InitColor[4 * 2 + 1]);
    EXPECT_PIXEL_COLOR_EQ(w, h, kMip0InitColor[4 * 2 + 2]);

    glUniform1f(lodLoc, 2);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, kMip0InitColor[4 * 0 + 2]);
    EXPECT_PIXEL_COLOR_EQ(w, 0, kMip0InitColor[4 * 0 + 2]);
    EXPECT_PIXEL_COLOR_EQ(0, h, kMip0InitColor[4 * 0 + 2]);
    EXPECT_PIXEL_COLOR_EQ(w, h, kMip0InitColor[4 * 0 + 2]);
}

// Test that swizzle on source texture does not affect the copy.
TEST_P(CopyTextureTestES3, SwizzleOnSource)
{
    const GLColor kSourceColor = GLColor(31, 73, 146, 228);

    // Create image with swizzle.  If swizzle is mistakenly applied, resulting color would be
    // kSourceColor.gbar
    GLTexture sourceTexture;
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &kSourceColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_GREEN);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ALPHA);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);

    GLTexture destTexture;
    glBindTexture(GL_TEXTURE_2D, destTexture);

    // Note: flipY is used to avoid direct transfer between textures and force a draw-based path.
    glCopyTextureCHROMIUM(sourceTexture, 0, GL_TEXTURE_2D, destTexture, 0, GL_RGBA8,
                          GL_UNSIGNED_BYTE, true, false, false);
    ASSERT_GL_NO_ERROR();

    // Verify the copy.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, destTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLint textureLocation = glGetUniformLocation(program, essl1_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLocation);
    glUniform1i(textureLocation, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, kSourceColor);
}

// Test that the right error type is triggered when
// OES_EGL_image_external_essl3 is required but not supported
TEST_P(CopyTextureTestES3, CopySubTextureMissingRequiredExtension)
{
    if (!checkExtensions())
    {
        return;
    }

    // decide if the test is relevant
    // If GL_OES_EGL_image_external_essl3 is supported, then no need to test
    ANGLE_SKIP_TEST_IF(IsGLExtensionEnabled("GL_OES_EGL_image_external_essl3"));
    // GL_OES_EGL_image_external extension is required to use TEXTURE_EXTERNAL_OES
    // as the texture target.  So if GL_OES_EGL_image_external is not supported,
    // the error case cannot happen.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_EGL_image_external"));
    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();
    // If EGL_KHR_image_base is not supported, then eglImageKHR cannot be used,
    // and therefore the error case cannot happen
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(dpy, "EGL_KHR_image_base"));

    // prepare test data
    // create the texture data
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    EXPECT_GL_NO_ERROR();
    const std::vector<GLColor> kSourceColor(2 * 2, GLColor::green);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kSourceColor.data());
    EXPECT_GL_NO_ERROR();

    // create the image and load the texture data
    constexpr EGLint attribs[] = {
        EGL_IMAGE_PRESERVED,
        EGL_TRUE,
        EGL_NONE,
    };
    EGLImageKHR image = eglCreateImageKHR(
        window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
        reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(texture)), attribs);
    EXPECT_GL_NO_ERROR();
    // source is from the image
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextures[0]);
    EXPECT_GL_NO_ERROR();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
    EXPECT_GL_NO_ERROR();

    // dest
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    EXPECT_GL_NO_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8UI, 3, 3, 0, GL_RGB_INTEGER, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    // This copying needs EGLImageExternalEssl3OES extension
    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 1, 1, 0, 0, 1, 1,
                             false, false, false);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    // Non-integer dest internal format doesn't need the extension
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    EXPECT_GL_NO_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    glCopySubTextureCHROMIUM(mTextures[0], 0, GL_TEXTURE_2D, mTextures[1], 0, 1, 1, 0, 0, 1, 1,
                             false, false, false);
    EXPECT_GL_NO_ERROR();
}

// Test copying from gles texture to EGLImage
TEST_P(CopyTextureTestES3, CopyImageSubDataToEGLImage)
{
    // GL_OES_EGL_image_external extension is required to use TEXTURE_EXTERNAL_OES
    // as the texture target.  So if GL_OES_EGL_image_external is not supported,
    // the error case cannot happen.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_EGL_image_external"));
    // GL_EXT_copy_image extension is required to use glCopyImageSubDataEXT to
    // copy from/to GL_TEXTURE_EXTERNAL_OES texture target. If  GL_EXT_copy_image
    // is not supported, then no need to test.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_copy_image"));

    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();
    // If EGL_KHR_image_base is not supported, then eglImageKHR cannot be used,
    // and therefore the error case cannot happen
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(dpy, "EGL_KHR_image_base"));

    // Prepare test data
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    EXPECT_GL_NO_ERROR();
    const std::vector<GLColor> kDstColor(4 * 4, GLColor::red);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, kDstColor.data());
    EXPECT_GL_NO_ERROR();

    // Create the egl image and bind it to target GL_TEXTURE_EXTERNAL_OES
    constexpr EGLint attribs[] = {
        EGL_IMAGE_PRESERVED,
        EGL_TRUE,
        EGL_NONE,
    };
    EGLImageKHR image = eglCreateImageKHR(
        window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
        reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(texture)), attribs);
    EXPECT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextures[0]);
    EXPECT_GL_NO_ERROR();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
    EXPECT_GL_NO_ERROR();

    // Prepare the source texture data
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    EXPECT_GL_NO_ERROR();
    const std::vector<GLColor> kSourceColor(4 * 4, GLColor::green);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kSourceColor.data());
    EXPECT_GL_NO_ERROR();

    // Copy from gles texture to EGLImage.
    // This copying needs EGLImageExternalEssl3OES and GL_EXT_copy_image extension
    glCopyImageSubDataEXT(mTextures[1], GL_TEXTURE_2D, 0, 0, 0, 0, mTextures[0],
                          GL_TEXTURE_EXTERNAL_OES, 0, 0, 0, 0, 4, 4, 1);
    EXPECT_GL_NO_ERROR();

    // Check the results
    glBindTexture(GL_TEXTURE_2D, texture);
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    EXPECT_PIXEL_RECT_EQ(0, 0, 4, 4, GLColor::green);
}

// Test copying from EGLImage to gles texture
TEST_P(CopyTextureTestES3, CopyImageSubDataFromEGLImage)
{
    // GL_OES_EGL_image_external extension is required to use TEXTURE_EXTERNAL_OES
    // as the texture target.  So if GL_OES_EGL_image_external is not supported,
    // the error case cannot happen.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_EGL_image_external"));
    // GL_EXT_copy_image extension is required to use glCopyImageSubDataEXT to
    // copy from/to GL_TEXTURE_EXTERNAL_OES texture target. If  GL_EXT_copy_image
    // is not supported, then no need to test.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_copy_image"));

    // Prepare test data
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    EXPECT_GL_NO_ERROR();
    const std::vector<GLColor> kSourceColor(4 * 4, GLColor::green);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kSourceColor.data());
    EXPECT_GL_NO_ERROR();

    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();
    // If EGL_KHR_image_base is not supported, then eglImageKHR cannot be used,
    // and therefore the error case cannot happen
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(dpy, "EGL_KHR_image_base"));

    // Create the egl image and bind it to target GL_TEXTURE_EXTERNAL_OES
    constexpr EGLint attribs[] = {
        EGL_IMAGE_PRESERVED,
        EGL_TRUE,
        EGL_NONE,
    };
    EGLImageKHR image = eglCreateImageKHR(
        window->getDisplay(), window->getContext(), EGL_GL_TEXTURE_2D_KHR,
        reinterpret_cast<EGLClientBuffer>(static_cast<uintptr_t>(texture)), attribs);
    EXPECT_GL_NO_ERROR();
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextures[0]);
    EXPECT_GL_NO_ERROR();
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
    EXPECT_GL_NO_ERROR();

    // Prepare the destination texture
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    EXPECT_GL_NO_ERROR();
    const std::vector<GLColor> kDstColor(4 * 4, GLColor::red);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, kDstColor.data());
    EXPECT_GL_NO_ERROR();

    // Copy from egl texture to gles texture.
    glCopyImageSubDataEXT(mTextures[0], GL_TEXTURE_EXTERNAL_OES, 0, 0, 0, 0, mTextures[1],
                          GL_TEXTURE_2D, 0, 0, 0, 0, 4, 4, 1);
    EXPECT_GL_NO_ERROR();

    // Check the results.
    EXPECT_PIXEL_RECT_EQ(0, 0, 4, 4, GLColor::green);
}

// Test that copy after invalidate works
TEST_P(CopyTextureTestES3, InvalidateCopyThenBlend)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    // http://anglebug.com/42263716
    ANGLE_SKIP_TEST_IF(IsMac() && IsIntel() && IsOpenGL());

    // http://anglebug.com/42263717
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsOpenGL());

    constexpr GLsizei kSize = 4;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kSize * 2, kSize * 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize * 2, kSize * 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA8, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA8, kSize / 2, kSize / 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA8, kSize / 4, kSize / 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 2);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Invalidate the framebuffer.
    const GLenum discards[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);
    ASSERT_GL_NO_ERROR();

    // Copy into the framebuffer attachment.
    const std::vector<GLColor> kSourceColor(kSize * kSize, GLColor::green);
    GLTexture sourceTexture;
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kSourceColor.data());
    // Note: flipY is used to avoid direct transfer between textures and force a draw-based path.
    glCopyTextureCHROMIUM(sourceTexture, 0, GL_TEXTURE_2D, texture, 2, GL_RGBA8, GL_UNSIGNED_BYTE,
                          true, false, false);
    ASSERT_GL_NO_ERROR();

    // Draw and blend, making sure both the copy and draw happen correctly.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Make sure the blend was correctly done.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::yellow);
}

// Test that sRGB-to-RGB copy does not change pixel values.
// http://anglebug.com/40096868
TEST_P(CopyTextureTest, NoConvertSRGBToRGB)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_sRGB"));
    testSrgbToRgb(GL_SRGB_ALPHA_EXT, GL_SRGB_ALPHA_EXT);
}

// Test that sRGB-to-RGB copy does not change pixel values.
// http://anglebug.com/40096868
TEST_P(CopyTextureTestES3, NoConvertSRGBToRGB)
{
    testSrgbToRgb(GL_SRGB8_ALPHA8, GL_RGBA);
}

void CopyTextureTestES3::invalidateBlitThenBlendCommon(GLsizei layerCount)
{
    // http://anglebug.com/42263713
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGL());

    // http://anglebug.com/42263716
    ANGLE_SKIP_TEST_IF(IsMac() && IsIntel() && IsOpenGL());

    // http://anglebug.com/42263717
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsOpenGL());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    constexpr GLsizei kSize = 4;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, kSize * 2, kSize * 2, layerCount, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, kSize * 2, kSize * 2, layerCount, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 2, GL_RGBA8, kSize, kSize, layerCount, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 3, GL_RGBA8, kSize / 2, kSize / 2, layerCount, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 4, GL_RGBA8, kSize / 4, kSize / 4, layerCount, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 4);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 2, layerCount / 2);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Invalidate the framebuffer.
    const GLenum discards[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);
    ASSERT_GL_NO_ERROR();

    // Blit into the framebuffer attachment.
    const std::vector<GLColor> kSourceColor(kSize * kSize, GLColor::green);
    GLTexture sourceTexture;
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kSourceColor.data());
    GLFramebuffer sourceFramebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFramebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sourceTexture,
                           0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);

    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Draw and blend, making sure both the blit and draw happen correctly.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Make sure the blend was correctly done.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::yellow);
}

// Test that blit after invalidate works with non-zero layer
TEST_P(CopyTextureTestES3, InvalidateBlitThenBlend3Layers)
{
    invalidateBlitThenBlendCommon(3);
}

// Test that blit after invalidate works with non-zero layer that is very large
TEST_P(CopyTextureTestES3, InvalidateBlitThenBlend1000Layers)
{
    invalidateBlitThenBlendCommon(1000);
}

TEST_P(CopyTextureTestES3, DrawThenCopyThenBlend)
{
    // Regression test for anglebug.com/42265446.
    //
    // Reproduces two behaviors:
    //
    // 1) The initial draw disappearing entirely from the default back
    // buffer. The current test case does not show this behavior
    // independently from the other, but a previous iteration, with the
    // textured quad scaled to half size and translated (-0.5, -0.5), did.
    //
    // 2) With Metal debug layers and load/store validation turned on on
    // Intel Macs, the transparent area of the texture prior to the bug
    // fix was magenta = undefined. Similar behavior would presumably
    // reproduce on M1 hardware without debug layers or validation.

    constexpr GLsizei kSize     = 64;
    constexpr GLsizei kHalfSize = kSize / 2;

    setWindowWidth(kSize);
    setWindowHeight(kSize);

    // Define destination texture for the CopyTexSubImage2D operation.
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kHalfSize, kHalfSize);

    // Redefine framebuffer's texture already allocated in testSetUp.
    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kSize, kSize);

    GLFramebuffer srcFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, srcFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(drawGreen, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());
    ANGLE_GL_PROGRAM(drawTexture, essl3_shaders::vs::Texture2DLod(),
                     essl3_shaders::fs::Texture2DLod());

    glDisable(GL_BLEND);

    // Draw a square half the size of the viewport, centered in the viewport.
    glViewport(0, 0, kSize, kSize);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(drawGreen);
    drawQuad(drawGreen, essl3_shaders::PositionAttrib(), 0.5f, 0.5f, true);
    ASSERT_GL_NO_ERROR();

    // Resolve the multisampled framebuffer.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Copy the upper right quarter of the framebuffer to mTextures[0].
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    ASSERT_GL_NO_ERROR();
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kHalfSize, kHalfSize, kHalfSize, kHalfSize);
    ASSERT_GL_NO_ERROR();

    glUseProgram(drawTexture);
    GLint textureLoc = glGetUniformLocation(drawTexture, essl3_shaders::Texture2DUniform());
    GLint lodLoc     = glGetUniformLocation(drawTexture, essl3_shaders::LodUniform());
    ASSERT_NE(-1, textureLoc);
    ASSERT_NE(-1, lodLoc);
    glUniform1i(textureLoc, 0);  // to match GL_TEXTURE0
    glUniform1f(lodLoc, 0);

    // Magnify and blend this texture over the current framebuffer.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, srcFBO);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    drawQuad(drawTexture, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);

    // Resolve the multisampled framebuffer again.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer);

    // Center quad should be rendered correctly.
    EXPECT_PIXEL_RECT_EQ(kHalfSize / 2 + 1, kHalfSize / 2 + 1, kHalfSize - 2, kHalfSize - 2,
                         GLColor::green);

    // Overlapping lower-left quad should be green as well.
    EXPECT_PIXEL_RECT_EQ(1, 1, kHalfSize - 2, kHalfSize - 2, GLColor::green);

    // Leftmost area above the lower-left quad should be transparent.
    EXPECT_PIXEL_RECT_EQ(1, kHalfSize + 1, kHalfSize / 2 - 2, kHalfSize / 2 - 2,
                         GLColor::transparentBlack);

    // Bottommost area to the right of the lower-left quad should be transparent.
    EXPECT_PIXEL_RECT_EQ(kHalfSize + 1, 1, kHalfSize / 2 - 2, kHalfSize / 2 - 2,
                         GLColor::transparentBlack);
}

// Test that copy multiple slices between 2D_ARRAY and 3D textures work when their texture formats
// are different.
// Simplified from
// dEQP-GLES31.functional.copy_image.non_compressed.viewclass_128_bits.rgba32f_rgba32i.texture2d_array_to_texture3d
TEST_P(CopyTextureTestES3, TextureCopyMultipleSlicesBetween2DArrayAnd3D)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_copy_image"));

    constexpr GLColor32F kFloatYellow          = {1.0f, 1.0f, 0.0f, 1.0f};
    constexpr GLColor32I kIntBlue              = {0, 0, 255, 255};
    constexpr GLColor32I kIntGreen             = {0, 255, 0, 255};
    constexpr GLsizei kTexWidth                = 16;
    constexpr GLsizei kTexHeight               = 16;
    constexpr int32_t kIntEquivalentOfFloatOne = 1065353216;

    GLTexture tex2DArray;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    ASSERT_GL_NO_ERROR();
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex2DArray);
    std::vector<GLColor32F> tex2DArraySlice0(kTexWidth * kTexHeight, kFloatRed);
    std::vector<GLColor32F> tex2DArraySlice1(kTexWidth * kTexHeight, kFloatYellow);
    std::vector<GLColor32F> tex2DArrayPixels(kTexWidth * kTexHeight * 2);
    auto pos = tex2DArrayPixels.begin();
    tex2DArrayPixels.insert(pos, tex2DArraySlice1.begin(), tex2DArraySlice1.end());
    pos = tex2DArrayPixels.begin();
    tex2DArrayPixels.insert(pos, tex2DArraySlice0.begin(), tex2DArraySlice0.end());
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, kTexWidth, kTexHeight, 2, 0, GL_RGBA, GL_FLOAT,
                 tex2DArrayPixels.data());
    ASSERT_GL_NO_ERROR();
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    GLTexture tex3D;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    ASSERT_GL_NO_ERROR();
    glBindTexture(GL_TEXTURE_3D, tex3D);
    std::vector<GLColor32I> tex3DSlice0(kTexWidth * kTexHeight, kIntBlue);
    std::vector<GLColor32I> tex3DSlice1(kTexWidth * kTexHeight, kIntGreen);
    std::vector<GLColor32I> tex3DPixels(kTexWidth * kTexHeight * 2);
    auto pos2 = tex3DPixels.begin();
    tex3DPixels.insert(pos2, tex3DSlice1.begin(), tex3DSlice1.end());
    pos2 = tex3DPixels.begin();
    tex3DPixels.insert(pos2, tex3DSlice0.begin(), tex3DSlice0.end());
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32I, kTexWidth, kTexHeight, 2, 0, GL_RGBA_INTEGER, GL_INT,
                 tex3DPixels.data());
    ASSERT_GL_NO_ERROR();
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    glBindTexture(GL_TEXTURE_3D, 0);

    // Verify color before copy
    GLFramebuffer FBO;
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2DArray, 0, 0);
    EXPECT_PIXEL_RECT32F_EQ(0, 0, kTexWidth, kTexHeight, kFloatRed);

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2DArray, 0, 1);
    EXPECT_PIXEL_RECT32F_EQ(0, 0, kTexWidth, kTexHeight, kFloatYellow);

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex3D, 0, 0);
    EXPECT_PIXEL_RECT32I_EQ(0, 0, kTexWidth, kTexHeight, kIntBlue);

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex3D, 0, 1);
    EXPECT_PIXEL_RECT32I_EQ(0, 0, kTexWidth, kTexHeight, kIntGreen);

    // Copy GL_TEXTURE_2D_ARRAY to GL_TEXTURE_3D with copy size = (texWidth, texHeight/2, 2)
    glCopyImageSubDataEXT(tex2DArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, tex3D, GL_TEXTURE_3D, 0, 0,
                          0, 0, kTexWidth, kTexHeight / 2, 2);

    // Verify colors after copy
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    // verify tex2DArray
    // layer 0 should remain unchanged
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2DArray, 0, 0);
    EXPECT_PIXEL_RECT32F_EQ(0, 0, kTexWidth, kTexHeight, kFloatRed);
    // layer 1 should remain unchanged
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex2DArray, 0, 1);
    EXPECT_PIXEL_RECT32F_EQ(0, 0, kTexWidth, kTexHeight, kFloatYellow);

    // verify tex3D content
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex3D, 0, 0);
    // part of tex3D slice 0 should contain the copied content from tex2DArray layer 0
    EXPECT_PIXEL_RECT32I_EQ(0, 0, kTexWidth, kTexHeight / 2,
                            GLColor32I(kIntEquivalentOfFloatOne, 0, 0, kIntEquivalentOfFloatOne));
    // part of tex3D slice 0 should remain unchanged
    EXPECT_PIXEL_RECT32I_EQ(0, kTexHeight / 2, kTexWidth, kTexHeight / 2, kIntBlue);
    // verify slice 1
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex3D, 0, 1);
    // part of tex3D slice 1 should contain the copied content from tex2DArray layer 1
    EXPECT_PIXEL_RECT32I_EQ(0, 0, kTexWidth, kTexHeight / 2,
                            GLColor32I(kIntEquivalentOfFloatOne, kIntEquivalentOfFloatOne, 0,
                                       kIntEquivalentOfFloatOne));
    // part of tex3D slice 1 should remain unchanged
    EXPECT_PIXEL_RECT32I_EQ(0, kTexHeight / 2, kTexWidth, kTexHeight / 2, kIntGreen);
}

ANGLE_INSTANTIATE_TEST_ES2(CopyTextureTest);
ANGLE_INSTANTIATE_TEST_COMBINE_6(CopyTextureVariationsTest,
                                 CopyTextureVariationsTestPrint,
                                 testing::ValuesIn(kCopyTextureVariationsSrcFormats),
                                 testing::ValuesIn(kCopyTextureVariationsDstFormats),
                                 testing::Bool(),  // flipY
                                 testing::Bool(),  // premultiplyAlpha
                                 testing::Bool(),  // unmultiplyAlpha
                                 testing::ValuesIn(kMesaYFlips),
                                 ES2_D3D9(),
                                 ES2_D3D11(),
                                 ES2_OPENGL(),
                                 ES2_OPENGLES(),
                                 ES2_VULKAN(),
                                 ES2_METAL());
ANGLE_INSTANTIATE_TEST_ES2(CopyTextureTestWebGL);
ANGLE_INSTANTIATE_TEST(CopyTextureTestDest,
                       ES2_D3D11(),
                       ES2_OPENGL(),
                       ES2_OPENGLES(),
                       ES2_VULKAN(),
                       ES2_METAL());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(CopyTextureTestES3);
ANGLE_INSTANTIATE_TEST_ES3(CopyTextureTestES3);

}  // namespace angle
