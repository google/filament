//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RobustResourceInitTest: Tests for GL_ANGLE_robust_resource_initialization.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/gles_loader_autogen.h"

namespace angle
{
constexpr char kSimpleTextureVertexShader[] =
    "#version 300 es\n"
    "in vec4 position;\n"
    "out vec2 texcoord;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = position;\n"
    "    texcoord = vec2(position.xy * 0.5 - 0.5);\n"
    "}";

// TODO(jmadill): Would be useful in a shared place in a utils folder.
void UncompressDXTBlock(int destX,
                        int destY,
                        int destWidth,
                        const std::vector<uint8_t> &src,
                        int srcOffset,
                        GLenum format,
                        std::vector<GLColor> *colorsOut)
{
    auto make565 = [src](int offset) {
        return static_cast<int>(src[offset + 0]) + static_cast<int>(src[offset + 1]) * 256;
    };
    auto make8888From565 = [](int c) {
        return GLColor(
            static_cast<GLubyte>(floor(static_cast<float>((c >> 11) & 0x1F) * (255.0f / 31.0f))),
            static_cast<GLubyte>(floor(static_cast<float>((c >> 5) & 0x3F) * (255.0f / 63.0f))),
            static_cast<GLubyte>(floor(static_cast<float>((c >> 0) & 0x1F) * (255.0f / 31.0f))),
            255);
    };
    auto mix = [](int mult, GLColor c0, GLColor c1, float div) {
        GLColor r = GLColor::transparentBlack;
        for (int ii = 0; ii < 4; ++ii)
        {
            r[ii] = static_cast<GLubyte>(floor(static_cast<float>(c0[ii] * mult + c1[ii]) / div));
        }
        return r;
    };
    bool isDXT1 =
        (format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) || (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
    int colorOffset               = srcOffset + (isDXT1 ? 0 : 8);
    int color0                    = make565(colorOffset + 0);
    int color1                    = make565(colorOffset + 2);
    bool c0gtc1                   = color0 > color1 || !isDXT1;
    GLColor rgba0                 = make8888From565(color0);
    GLColor rgba1                 = make8888From565(color1);
    std::array<GLColor, 4> colors = {{rgba0, rgba1,
                                      c0gtc1 ? mix(2, rgba0, rgba1, 3) : mix(1, rgba0, rgba1, 2),
                                      c0gtc1 ? mix(2, rgba1, rgba0, 3) : GLColor::black}};

    // Original comment preserved below for posterity:
    // "yea I know there is a lot of math in this inner loop. so sue me."
    for (int yy = 0; yy < 4; ++yy)
    {
        uint8_t pixels = src[colorOffset + 4 + yy];
        for (int xx = 0; xx < 4; ++xx)
        {
            uint8_t code     = (pixels >> (xx * 2)) & 0x3;
            GLColor srcColor = colors[code];
            uint8_t alpha    = 0;
            switch (format)
            {
                case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
                    alpha = 255;
                    break;
                case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
                    alpha = (code == 3 && !c0gtc1) ? 0 : 255;
                    break;
                case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
                {
                    uint8_t alpha0 = src[srcOffset + yy * 2 + (xx >> 1)];
                    uint8_t alpha1 = (alpha0 >> ((xx % 2) * 4)) & 0xF;
                    alpha          = alpha1 | (alpha1 << 4);
                }
                break;
                case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                {
                    uint8_t alpha0 = src[srcOffset + 0];
                    uint8_t alpha1 = src[srcOffset + 1];
                    int alphaOff   = (yy >> 1) * 3 + 2;
                    uint32_t alphaBits =
                        static_cast<uint32_t>(src[srcOffset + alphaOff + 0]) +
                        static_cast<uint32_t>(src[srcOffset + alphaOff + 1]) * 256 +
                        static_cast<uint32_t>(src[srcOffset + alphaOff + 2]) * 65536;
                    int alphaShift    = (yy % 2) * 12 + xx * 3;
                    uint8_t alphaCode = static_cast<uint8_t>((alphaBits >> alphaShift) & 0x7);
                    if (alpha0 > alpha1)
                    {
                        switch (alphaCode)
                        {
                            case 0:
                                alpha = alpha0;
                                break;
                            case 1:
                                alpha = alpha1;
                                break;
                            default:
                                // TODO(jmadill): fix rounding
                                alpha = ((8 - alphaCode) * alpha0 + (alphaCode - 1) * alpha1) / 7;
                                break;
                        }
                    }
                    else
                    {
                        switch (alphaCode)
                        {
                            case 0:
                                alpha = alpha0;
                                break;
                            case 1:
                                alpha = alpha1;
                                break;
                            case 6:
                                alpha = 0;
                                break;
                            case 7:
                                alpha = 255;
                                break;
                            default:
                                // TODO(jmadill): fix rounding
                                alpha = ((6 - alphaCode) * alpha0 + (alphaCode - 1) * alpha1) / 5;
                                break;
                        }
                    }
                }
                break;
                default:
                    ASSERT_FALSE(true);
                    break;
            }
            int dstOff           = ((destY + yy) * destWidth + destX + xx);
            (*colorsOut)[dstOff] = GLColor(srcColor[0], srcColor[1], srcColor[2], alpha);
        }
    }
}

int GetBlockSize(GLenum format)
{
    bool isDXT1 =
        format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT || format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    return isDXT1 ? 8 : 16;
}

std::vector<GLColor> UncompressDXTIntoSubRegion(int width,
                                                int height,
                                                int subX0,
                                                int subY0,
                                                int subWidth,
                                                int subHeight,
                                                const std::vector<uint8_t> &data,
                                                GLenum format)
{
    std::vector<GLColor> dest(width * height, GLColor::transparentBlack);

    if ((width % 4) != 0 || (height % 4) != 0 || (subX0 % 4) != 0 || (subY0 % 4) != 0 ||
        (subWidth % 4) != 0 || (subHeight % 4) != 0)
    {
        std::cout << "Implementation error in UncompressDXTIntoSubRegion.";
        return dest;
    }

    int blocksAcross = subWidth / 4;
    int blocksDown   = subHeight / 4;
    int blockSize    = GetBlockSize(format);
    for (int yy = 0; yy < blocksDown; ++yy)
    {
        for (int xx = 0; xx < blocksAcross; ++xx)
        {
            UncompressDXTBlock(subX0 + xx * 4, subY0 + yy * 4, width, data,
                               (yy * blocksAcross + xx) * blockSize, format, &dest);
        }
    }
    return dest;
}

class RobustResourceInitTest : public ANGLETest<>
{
  protected:
    constexpr static int kWidth  = 128;
    constexpr static int kHeight = 128;

    RobustResourceInitTest()
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);

        setRobustResourceInit(true);

        // Test flakiness was noticed when reusing displays.
        forceNewDisplay();
    }

    bool hasGLExtension()
    {
        return IsGLExtensionEnabled("GL_ANGLE_robust_resource_initialization");
    }

    bool hasEGLExtension()
    {
        return IsEGLDisplayExtensionEnabled(getEGLWindow()->getDisplay(),
                                            "EGL_ANGLE_robust_resource_initialization");
    }

    bool hasRobustSurfaceInit()
    {
        if (!hasEGLExtension())
        {
            return false;
        }

        EGLint robustSurfaceInit = EGL_FALSE;
        eglQuerySurface(getEGLWindow()->getDisplay(), getEGLWindow()->getSurface(),
                        EGL_ROBUST_RESOURCE_INITIALIZATION_ANGLE, &robustSurfaceInit);
        return robustSurfaceInit;
    }

    void setupTexture(GLTexture *tex);
    void setup3DTexture(GLTexture *tex);

    // Checks for uninitialized (non-zero pixels) in a Texture.
    void checkNonZeroPixels(GLTexture *texture,
                            int skipX,
                            int skipY,
                            int skipWidth,
                            int skipHeight,
                            const GLColor &skip);
    void checkNonZeroPixels3D(GLTexture *texture,
                              int skipX,
                              int skipY,
                              int skipWidth,
                              int skipHeight,
                              int textureLayer,
                              const GLColor &skip);
    void checkFramebufferNonZeroPixels(int skipX,
                                       int skipY,
                                       int skipWidth,
                                       int skipHeight,
                                       const GLColor &skip);

    void checkCustomFramebufferNonZeroPixels(int fboWidth,
                                             int fboHeight,
                                             int skipX,
                                             int skipY,
                                             int skipWidth,
                                             int skipHeight,
                                             const GLColor &skip);

    static std::string GetSimpleTextureFragmentShader(const char *samplerType)
    {
        std::stringstream fragmentStream;
        fragmentStream << "#version 300 es\n"
                          "precision mediump "
                       << samplerType
                       << "sampler2D;\n"
                          "precision mediump float;\n"
                          "out "
                       << samplerType
                       << "vec4 color;\n"
                          "in vec2 texcoord;\n"
                          "uniform "
                       << samplerType
                       << "sampler2D tex;\n"
                          "void main()\n"
                          "{\n"
                          "    color = texture(tex, texcoord);\n"
                          "}";
        return fragmentStream.str();
    }

    template <typename ClearFunc>
    void maskedDepthClear(ClearFunc clearFunc);

    template <typename ClearFunc>
    void maskedStencilClear(ClearFunc clearFunc);

    void copyTexSubImage2DCustomFBOTest(int offsetX, int offsetY);
};

class RobustResourceInitTestES3 : public RobustResourceInitTest
{
  protected:
    template <typename PixelT>
    void testIntegerTextureInit(const char *samplerType,
                                GLenum internalFormatRGBA,
                                GLenum internalFormatRGB,
                                GLenum type);
};

class RobustResourceInitTestES31 : public RobustResourceInitTest
{};

// Robust resource initialization is not based on hardware support or native extensions, check that
// it only works on the implemented renderers
TEST_P(RobustResourceInitTest, ExpectedRendererSupport)
{
    bool shouldHaveSupport =
        IsD3D11() || IsD3D9() || IsOpenGL() || IsOpenGLES() || IsVulkan() || IsMetal();
    EXPECT_EQ(shouldHaveSupport, hasGLExtension());
    EXPECT_EQ(shouldHaveSupport, hasEGLExtension());
    EXPECT_EQ(shouldHaveSupport, hasRobustSurfaceInit());
}

// Tests of the GL_ROBUST_RESOURCE_INITIALIZATION_ANGLE query.
TEST_P(RobustResourceInitTest, Queries)
{
    // If context extension string exposed, check queries.
    if (IsGLExtensionEnabled("GL_ANGLE_robust_resource_initialization"))
    {
        GLboolean enabled = 0;
        glGetBooleanv(GL_ROBUST_RESOURCE_INITIALIZATION_ANGLE, &enabled);
        EXPECT_GL_TRUE(enabled);

        EXPECT_GL_TRUE(glIsEnabled(GL_ROBUST_RESOURCE_INITIALIZATION_ANGLE));
        EXPECT_GL_NO_ERROR();

        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // Can't verify the init state after glTexImage2D, the implementation is free to initialize
        // any time before the resource is read.

        {
            // Force to uninitialized
            glTexParameteri(GL_TEXTURE_2D, GL_RESOURCE_INITIALIZED_ANGLE, GL_FALSE);

            GLint initState = 0;
            glGetTexParameteriv(GL_TEXTURE_2D, GL_RESOURCE_INITIALIZED_ANGLE, &initState);
            EXPECT_GL_FALSE(initState);
        }
        {
            // Force to initialized
            glTexParameteri(GL_TEXTURE_2D, GL_RESOURCE_INITIALIZED_ANGLE, GL_TRUE);

            GLint initState = 0;
            glGetTexParameteriv(GL_TEXTURE_2D, GL_RESOURCE_INITIALIZED_ANGLE, &initState);
            EXPECT_GL_TRUE(initState);
        }
    }
    else
    {
        // Querying robust resource init should return INVALID_ENUM.
        GLboolean enabled = 0;
        glGetBooleanv(GL_ROBUST_RESOURCE_INITIALIZATION_ANGLE, &enabled);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Tests that buffers start zero-filled if the data pointer is null.
TEST_P(RobustResourceInitTest, BufferData)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, getWindowWidth() * getWindowHeight() * sizeof(GLfloat), nullptr,
                 GL_STATIC_DRAW);

    constexpr char kVS[] =
        "attribute vec2 position;\n"
        "attribute float testValue;\n"
        "varying vec4 colorOut;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 0, 1);\n"
        "    colorOut = testValue == 0.0 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);\n"
        "}";
    constexpr char kFS[] =
        "varying mediump vec4 colorOut;\n"
        "void main() {\n"
        "    gl_FragColor = colorOut;\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    GLint testValueLoc = glGetAttribLocation(program, "testValue");
    ASSERT_NE(-1, testValueLoc);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(testValueLoc, 1, GL_FLOAT, GL_FALSE, 4, nullptr);
    glEnableVertexAttribArray(testValueLoc);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    drawQuad(program, "position", 0.5f);

    ASSERT_GL_NO_ERROR();

    std::vector<GLColor> expected(getWindowWidth() * getWindowHeight(), GLColor::green);
    std::vector<GLColor> actual(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actual.data());
    EXPECT_EQ(expected, actual);

    GLint initState = 0;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_RESOURCE_INITIALIZED_ANGLE, &initState);
    EXPECT_GL_TRUE(initState);
}

// Regression test for passing a zero size init buffer with the extension.
TEST_P(RobustResourceInitTest, BufferDataZeroSize)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
}

// Test robust initialization of PVRTC1 textures.
TEST_P(RobustResourceInitTest, CompressedTexImagePVRTC1)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_IMG_texture_compression_pvrtc"));

    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(testProgram);

    std::vector<std::pair<GLenum, GLColor>> formats = {
        {GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, GLColor::black},
        {GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, GLColor::black},
        {GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, GLColor::transparentBlack},
        {GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, GLColor::transparentBlack}};

    if (IsGLExtensionEnabled("GL_EXT_pvrtc_sRGB"))
    {
        formats.insert(formats.end(),
                       {{GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT, GLColor::black},
                        {GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT, GLColor::black},
                        {GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT, GLColor::transparentBlack},
                        {GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT, GLColor::transparentBlack}});
    }

    for (auto format : formats)
    {
        GLTexture texture;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, format.first, 8, 8, 0, 32, nullptr);
        ASSERT_GL_NO_ERROR();

        drawQuad(testProgram, essl1_shaders::PositionAttrib(), 0.0f);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, format.second);
    }
}

// Regression test for images being recovered from storage not re-syncing to storage after being
// initialized
TEST_P(RobustResourceInitTestES3, D3D11RecoverFromStorageBug)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    // http://anglebug.com/42264306
    // Vulkan uses incorrect copy sizes when redefining/zero initializing NPOT compressed textures.
    ANGLE_SKIP_TEST_IF(IsVulkan());

    static constexpr uint8_t img_8x8_rgb_dxt1[] = {
        0xe0, 0x07, 0x00, 0xf8, 0x11, 0x10, 0x15, 0x00, 0x1f, 0x00, 0xe0,
        0xff, 0x11, 0x10, 0x15, 0x00, 0xe0, 0x07, 0x1f, 0xf8, 0x44, 0x45,
        0x40, 0x55, 0x1f, 0x00, 0xff, 0x07, 0x44, 0x45, 0x40, 0x55,
    };

    static constexpr uint8_t img_4x4_rgb_dxt1[] = {
        0xe0, 0x07, 0x00, 0xf8, 0x11, 0x10, 0x15, 0x00,
    };

    static constexpr uint8_t img_4x4_rgb_dxt1_zeroes[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    for (size_t i = 0; i < 8; i++)
    {
        {
            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexStorage2D(GL_TEXTURE_2D, 4, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 8, 8);
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                      ArraySize(img_8x8_rgb_dxt1), img_8x8_rgb_dxt1);
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 4, 4, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                      ArraySize(img_4x4_rgb_dxt1), img_4x4_rgb_dxt1);
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, 2, 2, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                      ArraySize(img_4x4_rgb_dxt1), img_4x4_rgb_dxt1);
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 3, 0, 0, 1, 1, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                      ArraySize(img_4x4_rgb_dxt1), img_4x4_rgb_dxt1);
        }
        {
            GLTexture texture;
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexStorage2D(GL_TEXTURE_2D, 4, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 12, 12);
            glCompressedTexSubImage2D(GL_TEXTURE_2D, 3, 0, 0, 1, 1, GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
                                      ArraySize(img_4x4_rgb_dxt1_zeroes), img_4x4_rgb_dxt1_zeroes);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 3);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, 1, 1);
            glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            draw2DTexturedQuad(0.5f, 1.0f, true);
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
        }
    }
}

// The following test code translated from WebGL 1 test:
// https://www.khronos.org/registry/webgl/sdk/tests/conformance/misc/uninitialized-test.html
void RobustResourceInitTest::setupTexture(GLTexture *tex)
{
    GLuint tempTexture;
    glGenTextures(1, &tempTexture);
    glBindTexture(GL_TEXTURE_2D, tempTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // this can be quite undeterministic so to improve odds of seeing uninitialized data write bits
    // into tex then delete texture then re-create one with same characteristics (driver will likely
    // reuse mem) with this trick on r59046 WebKit/OSX I get FAIL 100% of the time instead of ~15%
    // of the time.

    std::array<uint8_t, kWidth * kHeight * 4> badData;
    for (size_t i = 0; i < badData.size(); ++i)
    {
        badData[i] = static_cast<uint8_t>(i % 255);
    }

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                    badData.data());
    glDeleteTextures(1, &tempTexture);

    // This will create the GLTexture.
    glBindTexture(GL_TEXTURE_2D, *tex);
}

void RobustResourceInitTest::setup3DTexture(GLTexture *tex)
{
    GLuint tempTexture;
    glGenTextures(1, &tempTexture);
    glBindTexture(GL_TEXTURE_3D, tempTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, kWidth, kHeight, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    // this can be quite undeterministic so to improve odds of seeing uninitialized data write bits
    // into tex then delete texture then re-create one with same characteristics (driver will likely
    // reuse mem) with this trick on r59046 WebKit/OSX I get FAIL 100% of the time instead of ~15%
    // of the time.

    std::array<uint8_t, kWidth * kHeight * 2 * 4> badData;
    for (size_t i = 0; i < badData.size(); ++i)
    {
        badData[i] = static_cast<uint8_t>(i % 255);
    }

    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, kWidth, kHeight, 2, GL_RGBA, GL_UNSIGNED_BYTE,
                    badData.data());
    glDeleteTextures(1, &tempTexture);

    // This will create the GLTexture.
    glBindTexture(GL_TEXTURE_3D, *tex);
}

void RobustResourceInitTest::checkNonZeroPixels(GLTexture *texture,
                                                int skipX,
                                                int skipY,
                                                int skipWidth,
                                                int skipHeight,
                                                const GLColor &skip)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->get(), 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    checkFramebufferNonZeroPixels(skipX, skipY, skipWidth, skipHeight, skip);
}

void RobustResourceInitTest::checkNonZeroPixels3D(GLTexture *texture,
                                                  int skipX,
                                                  int skipY,
                                                  int skipWidth,
                                                  int skipHeight,
                                                  int textureLayer,
                                                  const GLColor &skip)
{
    glBindTexture(GL_TEXTURE_3D, 0);
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->get(), 0,
                              textureLayer);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    checkFramebufferNonZeroPixels(skipX, skipY, skipWidth, skipHeight, skip);
}

void RobustResourceInitTest::checkFramebufferNonZeroPixels(int skipX,
                                                           int skipY,
                                                           int skipWidth,
                                                           int skipHeight,
                                                           const GLColor &skip)
{
    checkCustomFramebufferNonZeroPixels(kWidth, kHeight, skipX, skipY, skipWidth, skipHeight, skip);
}

void RobustResourceInitTest::checkCustomFramebufferNonZeroPixels(int fboWidth,
                                                                 int fboHeight,
                                                                 int skipX,
                                                                 int skipY,
                                                                 int skipWidth,
                                                                 int skipHeight,
                                                                 const GLColor &skip)
{
    std::vector<GLColor> data(fboWidth * fboHeight);
    glReadPixels(0, 0, fboWidth, fboHeight, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    int k = 0;
    for (int y = 0; y < fboHeight; ++y)
    {
        for (int x = 0; x < fboWidth; ++x)
        {
            int index = (y * fboWidth + x);
            if (x >= skipX && x < skipX + skipWidth && y >= skipY && y < skipY + skipHeight)
            {
                ASSERT_EQ(skip, data[index]) << " at pixel " << x << ", " << y;
            }
            else
            {
                k += (data[index] != GLColor::transparentBlack) ? 1 : 0;
            }
        }
    }

    EXPECT_EQ(0, k);
}

// Reading an uninitialized texture (texImage2D) should succeed with all bytes set to 0.
TEST_P(RobustResourceInitTest, ReadingUninitializedTexture)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    GLTexture tex;
    setupTexture(&tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    checkNonZeroPixels(&tex, 0, 0, 0, 0, GLColor::transparentBlack);
    EXPECT_GL_NO_ERROR();
}

// Test that calling glTexImage2D multiple times with the same size and no data resets all texture
// data
TEST_P(RobustResourceInitTest, ReuploadingClearsTexture)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    // crbug.com/826576
    ANGLE_SKIP_TEST_IF(IsMac() && IsNVIDIA() && IsDesktopOpenGL());

    // Put some data into the texture
    std::array<GLColor, kWidth * kHeight> data;
    data.fill(GLColor::white);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 data.data());

    // Reset the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    checkNonZeroPixels(&tex, 0, 0, 0, 0, GLColor::transparentBlack);
    EXPECT_GL_NO_ERROR();
}

// Cover the case where null pixel data is uploaded to a texture and then sub image is used to
// upload partial data
TEST_P(RobustResourceInitTest, TexImageThenSubImage)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    // http://anglebug.com/42261117, but only fails on Nexus devices
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsOpenGLES());

    // Put some data into the texture
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Force the D3D texture to create a storage
    checkNonZeroPixels(&tex, 0, 0, 0, 0, GLColor::transparentBlack);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    std::array<GLColor, kWidth * kHeight> data;
    data.fill(GLColor::white);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth / 2, kHeight / 2, GL_RGBA, GL_UNSIGNED_BYTE,
                    data.data());
    checkNonZeroPixels(&tex, 0, 0, kWidth / 2, kHeight / 2, GLColor::white);
    EXPECT_GL_NO_ERROR();
}

// Reading an uninitialized texture (texImage3D) should succeed with all bytes set to 0.
TEST_P(RobustResourceInitTestES3, ReadingUninitialized3DTexture)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    GLTexture tex;
    setup3DTexture(&tex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, kWidth, kHeight, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    checkNonZeroPixels3D(&tex, 0, 0, 0, 0, 0, GLColor::transparentBlack);
    EXPECT_GL_NO_ERROR();
}

// Copy of the copytexsubimage3d_texture_wrongly_initialized test that is part of the WebGL2
// conformance suite: copy-texture-image-webgl-specific.html
TEST_P(RobustResourceInitTestES3, CopyTexSubImage3DTextureWronglyInitialized)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    constexpr GLint kTextureLayer     = 0;
    constexpr GLint kTextureWidth     = 2;
    constexpr GLint kTextureHeight    = 2;
    constexpr GLint kTextureDepth     = 2;
    constexpr size_t kTextureDataSize = kTextureWidth * kTextureHeight * 4;

    GLTexture texture2D;
    glBindTexture(GL_TEXTURE_2D, texture2D);
    constexpr std::array<uint8_t, kTextureDataSize> data = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                                             0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
                                                             0x0D, 0x0E, 0x0F, 0x10}};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureWidth, kTextureHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data.data());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2D, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    GLTexture texture3D;
    glBindTexture(GL_TEXTURE_3D, texture3D);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, kTextureWidth, kTextureHeight, kTextureDepth);
    glCopyTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, kTextureLayer, 0, 0, kTextureWidth, kTextureHeight);

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture3D, 0, kTextureLayer);
    std::array<uint8_t, kTextureDataSize> pixels;
    glReadPixels(0, 0, kTextureWidth, kTextureHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(data, pixels);
}

// Test that binding an EGL surface to a texture does not cause it to be cleared.
TEST_P(RobustResourceInitTestES3, BindTexImage)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    EGLWindow *window  = getEGLWindow();
    EGLSurface surface = window->getSurface();
    EGLDisplay display = window->getDisplay();
    EGLConfig config   = window->getConfig();
    EGLContext context = window->getContext();

    EGLint surfaceType = 0;
    eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &surfaceType);
    // Test skipped because EGL config cannot be used to create pbuffers.
    ANGLE_SKIP_TEST_IF((surfaceType & EGL_PBUFFER_BIT) == 0);

    EGLint bindToSurfaceRGBA = 0;
    eglGetConfigAttrib(display, config, EGL_BIND_TO_TEXTURE_RGBA, &bindToSurfaceRGBA);
    // Test skipped because EGL config cannot be used to create pbuffers.
    ANGLE_SKIP_TEST_IF(bindToSurfaceRGBA == EGL_FALSE);

    EGLint attribs[] = {
        EGL_WIDTH,          32,
        EGL_HEIGHT,         32,
        EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
        EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
        EGL_NONE,
    };

    EGLSurface pbuffer = eglCreatePbufferSurface(display, config, attribs);
    ASSERT_NE(EGL_NO_SURFACE, pbuffer);

    // Clear the pbuffer
    eglMakeCurrent(display, pbuffer, pbuffer, context);
    GLColor clearColor = GLColor::magenta;
    glClearColor(clearColor.R, clearColor.G, clearColor.B, clearColor.A);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, clearColor);

    // Bind the pbuffer to a texture and read its color
    eglMakeCurrent(display, surface, surface, context);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    eglBindTexImage(display, pbuffer, EGL_BACK_BUFFER);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
    {
        EXPECT_PIXEL_COLOR_EQ(0, 0, clearColor);
    }
    else
    {
        std::cout << "Read pixels check skipped because framebuffer was not complete." << std::endl;
    }

    eglDestroySurface(display, pbuffer);
}

// Tests that drawing with an uninitialized Texture works as expected.
TEST_P(RobustResourceInitTest, DrawWithTexture)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    constexpr char kVS[] =
        "attribute vec2 position;\n"
        "varying vec2 texCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 0, 1);\n"
        "    texCoord = (position * 0.5) + 0.5;\n"
        "}";
    constexpr char kFS[] =
        "precision mediump float;\n"
        "varying vec2 texCoord;\n"
        "uniform sampler2D tex;\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(tex, texCoord);\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "position", 0.5f);

    checkFramebufferNonZeroPixels(0, 0, 0, 0, GLColor::black);
}

// Tests that drawing with an uninitialized mipped texture works as expected.
TEST_P(RobustResourceInitTestES3, DrawWithMippedTexture)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, kWidth / 2, kHeight / 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, kWidth / 4, kHeight / 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA, kWidth / 8, kHeight / 8, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);

    EXPECT_GL_NO_ERROR();

    constexpr char kVS[] =
        "attribute vec2 position;\n"
        "varying vec2 texCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 0, 1);\n"
        "    texCoord = (position * 0.5) + 0.5;\n"
        "}";
    constexpr char kFS[] =
        "precision mediump float;\n"
        "varying vec2 texCoord;\n"
        "uniform sampler2D tex;\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(tex, texCoord);\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "position", 0.5f);

    checkFramebufferNonZeroPixels(0, 0, 0, 0, GLColor::black);
}

// Reading a partially initialized texture (texImage2D) should succeed with all uninitialized bytes
// set to 0 and initialized bytes untouched.
TEST_P(RobustResourceInitTest, ReadingPartiallyInitializedTexture)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    // http://anglebug.com/42261117, but only fails on Nexus devices
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsOpenGLES());

    GLTexture tex;
    setupTexture(&tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    GLColor data(108, 72, 36, 9);
    glTexSubImage2D(GL_TEXTURE_2D, 0, kWidth / 2, kHeight / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    &data.R);
    checkNonZeroPixels(&tex, kWidth / 2, kHeight / 2, 1, 1, data);
    EXPECT_GL_NO_ERROR();
}

// Uninitialized parts of textures initialized via copyTexImage2D should have all bytes set to 0.
TEST_P(RobustResourceInitTest, UninitializedPartsOfCopied2DTexturesAreBlack)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    GLTexture tex;
    setupTexture(&tex);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    constexpr int fboWidth  = 16;
    constexpr int fboHeight = 16;
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, kWidth, kHeight, 0);
    checkNonZeroPixels(&tex, 0, 0, fboWidth, fboHeight, GLColor::red);
    EXPECT_GL_NO_ERROR();
}

// Reading an uninitialized portion of a texture (copyTexImage2D with negative x and y) should
// succeed with all bytes set to 0. Regression test for a bug where the zeroing out of the
// texture was done via the same code path as glTexImage2D, causing the PIXEL_UNPACK_BUFFER
// to be used.
TEST_P(RobustResourceInitTestES3, ReadingOutOfBoundsCopiedTextureWithUnpackBuffer)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());
    // TODO(geofflang@chromium.org): CopyTexImage from GL_RGBA4444 to GL_ALPHA fails when looking
    // up which resulting format the texture should have.
    ANGLE_SKIP_TEST_IF(IsOpenGL());

    // GL_ALPHA texture can't be read with glReadPixels, for convenience this test uses
    // glCopyTextureCHROMIUM to copy GL_ALPHA into GL_RGBA
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_copy_texture"));

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    constexpr int fboWidth  = 16;
    constexpr int fboHeight = 16;
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();
    constexpr int x = -8;
    constexpr int y = -8;

    GLBuffer buffer;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    std::vector<GLColor> bunchOfGreen(fboWidth * fboHeight, GLColor::green);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeof(bunchOfGreen[0]) * bunchOfGreen.size(),
                 bunchOfGreen.data(), GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    // Use non-multiple-of-4 dimensions to make sure unpack alignment is set in the backends
    // (http://crbug.com/836131)
    constexpr int kTextureWidth  = 127;
    constexpr int kTextureHeight = 127;

    // Use GL_ALPHA to force a CPU readback in the D3D11 backend
    GLTexture texAlpha;
    glBindTexture(GL_TEXTURE_2D, texAlpha);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, x, y, kTextureWidth, kTextureHeight, 0);
    EXPECT_GL_NO_ERROR();

    // GL_ALPHA cannot be glReadPixels, so copy into a GL_RGBA texture
    GLTexture texRGBA;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    setupTexture(&texRGBA);
    glCopyTextureCHROMIUM(texAlpha, 0, GL_TEXTURE_2D, texRGBA, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                          GL_FALSE, GL_FALSE, GL_FALSE);
    EXPECT_GL_NO_ERROR();

    checkNonZeroPixels(&texRGBA, -x, -y, fboWidth, fboHeight, GLColor(0, 0, 0, 255));
    EXPECT_GL_NO_ERROR();
}

// Reading an uninitialized portion of a texture (copyTexImage2D with negative x and y) should
// succeed with all bytes set to 0.
TEST_P(RobustResourceInitTest, ReadingOutOfBoundsCopiedTexture)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    GLTexture tex;
    setupTexture(&tex);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    constexpr int fboWidth  = 16;
    constexpr int fboHeight = 16;
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();
    constexpr int x = -8;
    constexpr int y = -8;
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, kWidth, kHeight, 0);
    checkNonZeroPixels(&tex, -x, -y, fboWidth, fboHeight, GLColor::red);
    EXPECT_GL_NO_ERROR();
}

// Tests resources are initialized properly with multisample resolve.
TEST_P(RobustResourceInitTestES3, MultisampledDepthInitializedCorrectly)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    // http://anglebug.com/42261117
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());
    // http://anglebug.com/42263936
    ANGLE_SKIP_TEST_IF(IsAMD() && IsD3D11());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    // Make the destination non-multisampled depth FBO.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLRenderbuffer depth;
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kWidth, kHeight);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glClearColor(0, 1, 0, 1);
    glClearDepthf(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Make the multisampled depth FBO.
    GLRenderbuffer msDepth;
    glBindRenderbuffer(GL_RENDERBUFFER, msDepth);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT16, kWidth, kHeight);

    GLFramebuffer msFBO;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, msFBO);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msDepth);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));

    // Multisample resolve.
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Test drawing with the resolved depth buffer.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_EQUAL);
    drawQuad(program, essl1_shaders::PositionAttrib(), 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Basic test that textures are initialized correctly.
TEST_P(RobustResourceInitTest, Texture)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    checkFramebufferNonZeroPixels(0, 0, 0, 0, GLColor::black);

    GLint initState = 0;
    glBindTexture(GL_TEXTURE_2D, texture);
    glGetTexParameteriv(GL_TEXTURE_2D, GL_RESOURCE_INITIALIZED_ANGLE, &initState);
    EXPECT_GL_TRUE(initState);
}

// Test that uploading texture data with an unpack state set correctly initializes the texture and
// the data is uploaded correctly.
TEST_P(RobustResourceInitTest, TextureWithUnpackState)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    // GL_UNPACK_ROW_LENGTH requires ES 3.0 or GL_EXT_unpack_subimage
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !EnsureGLExtensionEnabled("GL_EXT_unpack_subimage"));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Upload a 2x2 rect using GL_UNPACK_ROW_LENGTH=4
    GLColor colorData[8] = {
        GLColor::green, GLColor::green, GLColor::red, GLColor::red,
        GLColor::green, GLColor::green, GLColor::red, GLColor::red,
    };
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 4);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, colorData);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    checkFramebufferNonZeroPixels(0, 0, 2, 2, GLColor::green);
}

template <typename PixelT>
void RobustResourceInitTestES3::testIntegerTextureInit(const char *samplerType,
                                                       GLenum internalFormatRGBA,
                                                       GLenum internalFormatRGB,
                                                       GLenum type)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    std::string fs = GetSimpleTextureFragmentShader(samplerType);

    ANGLE_GL_PROGRAM(program, kSimpleTextureVertexShader, fs.c_str());

    // Make an RGBA framebuffer.
    GLTexture framebufferTexture;
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormatRGBA, kWidth, kHeight, 0, GL_RGBA_INTEGER, type,
                 nullptr);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture,
                           0);

    // Make an RGB texture.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormatRGB, kWidth, kHeight, 0, GL_RGB_INTEGER, type,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Blit from the texture to the framebuffer.
    drawQuad(program, "position", 0.5f);

    // Verify both textures have been initialized
    {
        GLint initState = 0;
        glBindTexture(GL_TEXTURE_2D, framebufferTexture);
        glGetTexParameteriv(GL_TEXTURE_2D, GL_RESOURCE_INITIALIZED_ANGLE, &initState);
        EXPECT_GL_TRUE(initState);
    }
    {
        GLint initState = 0;
        glBindTexture(GL_TEXTURE_2D, texture);
        glGetTexParameteriv(GL_TEXTURE_2D, GL_RESOURCE_INITIALIZED_ANGLE, &initState);
        EXPECT_GL_TRUE(initState);
    }

    std::array<PixelT, kWidth * kHeight * 4> data;
    glReadPixels(0, 0, kWidth, kHeight, GL_RGBA_INTEGER, type, data.data());

    // Check the color channels are zero and the alpha channel is 1.
    int incorrectPixels = 0;
    for (int y = 0; y < kHeight; ++y)
    {
        for (int x = 0; x < kWidth; ++x)
        {
            int index    = (y * kWidth + x) * 4;
            bool correct = (data[index] == 0 && data[index + 1] == 0 && data[index + 2] == 0 &&
                            data[index + 3] == 1);
            incorrectPixels += (!correct ? 1 : 0);
        }
    }

    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(0, incorrectPixels);
}

// Simple tests for integer formats that ANGLE must emulate on D3D11.
TEST_P(RobustResourceInitTestES3, TextureInit_UIntRGB8)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());
    // http://anglebug.com/42263936
    ANGLE_SKIP_TEST_IF(IsAMD() && IsD3D11());

    testIntegerTextureInit<uint8_t>("u", GL_RGBA8UI, GL_RGB8UI, GL_UNSIGNED_BYTE);
}

TEST_P(RobustResourceInitTestES3, TextureInit_UIntRGB32)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());
    testIntegerTextureInit<uint32_t>("u", GL_RGBA32UI, GL_RGB32UI, GL_UNSIGNED_INT);
}

TEST_P(RobustResourceInitTestES3, TextureInit_IntRGB8)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());
    // http://anglebug.com/42263936
    ANGLE_SKIP_TEST_IF(IsAMD() && IsD3D11());

    testIntegerTextureInit<int8_t>("i", GL_RGBA8I, GL_RGB8I, GL_BYTE);
}

TEST_P(RobustResourceInitTestES3, TextureInit_IntRGB32)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());
    testIntegerTextureInit<int32_t>("i", GL_RGBA32I, GL_RGB32I, GL_INT);
}

// Test that uninitialized image texture works well.
TEST_P(RobustResourceInitTestES31, ImageTextureInit_R32UI)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());
    constexpr char kCS[] =
        R"(#version 310 es
        layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
        layout(r32ui, binding = 1) writeonly uniform highp uimage2D writeImage;
        void main()
        {
            imageStore(writeImage, ivec2(gl_LocalInvocationID.xy), uvec4(200u));
        })";

    GLTexture texture;
    // Don't upload data to texture.
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
    EXPECT_GL_NO_ERROR();

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    glBindImageTexture(1, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    GLuint outputValue;
    glReadPixels(0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &outputValue);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(200u, outputValue);

    outputValue = 0u;
    // Write to another uninitialized texture.
    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
    EXPECT_GL_NO_ERROR();
    glBindImageTexture(1, texture2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    glDispatchCompute(1, 1, 1);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);
    glReadPixels(0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &outputValue);
    EXPECT_EQ(200u, outputValue);
}

// Basic test that renderbuffers are initialized correctly.
TEST_P(RobustResourceInitTest, Renderbuffer)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

    checkFramebufferNonZeroPixels(0, 0, 0, 0, GLColor::black);
}

// Tests creating mipmaps with robust resource init.
TEST_P(RobustResourceInitTestES3, GenerateMipmap)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    constexpr GLint kTextureSize = 16;

    // Initialize a 16x16 RGBA8 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    std::string shader = GetSimpleTextureFragmentShader("");
    ANGLE_GL_PROGRAM(program, kSimpleTextureVertexShader, shader.c_str());

    // Generate mipmaps and verify all the mips.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Validate a small texture.
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Set viewport to resize the texture and draw.
    glViewport(0, 0, 2, 2);
    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
}

// Tests creating mipmaps with robust resource init multiple times.
TEST_P(RobustResourceInitTestES3, GenerateMipmapAfterRedefine)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    constexpr GLint kTextureSize = 16;
    const std::vector<GLColor> kInitData(kTextureSize * kTextureSize, GLColor::blue);

    // Initialize a 16x16 RGBA8 texture with blue.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, kInitData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    std::string shader = GetSimpleTextureFragmentShader("");
    ANGLE_GL_PROGRAM(program, kSimpleTextureVertexShader, shader.c_str());

    // Generate mipmaps.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Validate a small mip.
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Set viewport to resize the texture and draw.
    glViewport(0, 0, 2, 2);
    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Redefine mip 0 with no data.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    // Generate mipmaps again.  Mip 0 must be cleared before the mipmaps are regenerated.
    glGenerateMipmap(GL_TEXTURE_2D);
    EXPECT_GL_NO_ERROR();

    // Validate a small mip.
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glViewport(0, 0, 2, 2);
    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
}

// Tests creating mipmaps for cube maps with robust resource init.
TEST_P(RobustResourceInitTestES3, GenerateMipmapCubeMap)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    constexpr GLint kTextureSize   = 16;
    constexpr GLint kTextureLevels = 5;

    // Initialize a 16x16 RGBA8 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    for (GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X; target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         ++target)
    {
        glTexImage2D(target, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Generate mipmaps and verify all the mips.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    for (GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X; target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
         ++target)
    {
        for (GLint level = 0; level < kTextureLevels; ++level)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, tex, level);
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
        }
    }
}

// Test blitting a framebuffer out-of-bounds. Multiple iterations.
TEST_P(RobustResourceInitTestES3, BlitFramebufferOutOfBounds)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    // Initiate data to read framebuffer
    constexpr int size                = 8;
    constexpr GLenum readbufferFormat = GL_RGBA8;
    constexpr GLenum drawbufferFormat = GL_RGBA8;
    constexpr GLenum filter           = GL_NEAREST;

    std::vector<GLColor> readColors(size * size, GLColor::yellow);

    // Create read framebuffer and feed data to read buffer
    // Read buffer may have srgb image
    GLTexture tex_read;
    glBindTexture(GL_TEXTURE_2D, tex_read);
    glTexImage2D(GL_TEXTURE_2D, 0, readbufferFormat, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 readColors.data());

    GLFramebuffer fbo_read;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_read);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_read, 0);

    // Create draw framebuffer. Color in draw buffer is initialized to 0.
    // Draw buffer may have srgb image
    GLTexture tex_draw;
    glBindTexture(GL_TEXTURE_2D, tex_draw);
    glTexImage2D(GL_TEXTURE_2D, 0, drawbufferFormat, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    GLFramebuffer fbo_draw;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_draw);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_draw, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));

    using Region = std::array<int, 4>;

    struct Test
    {
        constexpr Test(const Region &read, const Region &draw, const Region &real)
            : readRegion(read), drawRegion(draw), realRegion(real)
        {}

        Region readRegion;
        Region drawRegion;
        Region realRegion;
    };

    constexpr std::array<Test, 2> tests = {{
        // only src region is out-of-bounds, dst region has different width/height as src region.
        {{{-2, -2, 4, 4}}, {{1, 1, 4, 4}}, {{2, 2, 4, 4}}},
        // only src region is out-of-bounds, dst region has the same width/height as src region.
        {{{-2, -2, 4, 4}}, {{1, 1, 7, 7}}, {{3, 3, 7, 7}}},
    }};

    // Blit read framebuffer to the image in draw framebuffer.
    for (const auto &test : tests)
    {
        // both the read framebuffer and draw framebuffer bounds are [0, 0, 8, 8]
        // blitting from src region to dst region
        glBindTexture(GL_TEXTURE_2D, tex_draw);
        glTexImage2D(GL_TEXTURE_2D, 0, drawbufferFormat, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);

        const auto &read = test.readRegion;
        const auto &draw = test.drawRegion;
        const auto &real = test.realRegion;

        glBlitFramebuffer(read[0], read[1], read[2], read[3], draw[0], draw[1], draw[2], draw[3],
                          GL_COLOR_BUFFER_BIT, filter);

        // Read pixels and check the correctness.
        std::vector<GLColor> pixels(size * size);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_draw);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        glReadPixels(0, 0, size, size, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_read);
        ASSERT_GL_NO_ERROR();

        for (int ii = 0; ii < size; ++ii)
        {
            for (int jj = 0; jj < size; ++jj)
            {
                GLColor expectedColor = GLColor::transparentBlack;
                if (ii >= real[0] && ii < real[2] && jj >= real[1] && jj < real[3])
                {
                    expectedColor = GLColor::yellow;
                }

                int loc = ii * size + jj;
                EXPECT_EQ(expectedColor, pixels[loc]) << " at [" << jj << ", " << ii << "]";
            }
        }
    }
}

template <typename ClearFunc>
void RobustResourceInitTest::maskedDepthClear(ClearFunc clearFunc)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    constexpr int kSize = 16;

    // Initialize a FBO with depth and simple color.
    GLRenderbuffer depthbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kSize, kSize);

    GLTexture colorbuffer;
    glBindTexture(GL_TEXTURE_2D, colorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorbuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);

    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Disable depth writes and trigger a clear.
    glDepthMask(GL_FALSE);

    clearFunc(0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Draw red with a depth function that checks for the clear value.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_EQUAL);
    glViewport(0, 0, kSize, kSize);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black) << "depth should not be 0.5f";

    drawQuad(program, essl1_shaders::PositionAttrib(), 1.0f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red) << "depth should be initialized to 1.0f";
}

// Test that clearing a masked depth buffer doesn't mark it clean.
TEST_P(RobustResourceInitTest, MaskedDepthClear)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    // http://anglebug.com/42261117
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());
    // http://anglebug.com/42263936
    ANGLE_SKIP_TEST_IF(IsAMD() && IsD3D11());

    auto clearFunc = [](float depth) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepthf(depth);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    };

    maskedDepthClear(clearFunc);
}

// Tests the same as MaskedDepthClear, but using ClearBuffer calls.
TEST_P(RobustResourceInitTestES3, MaskedDepthClearBuffer)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    // http://anglebug.com/42261117
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());
    // http://anglebug.com/42263936
    ANGLE_SKIP_TEST_IF(IsAMD() && IsD3D11());

    auto clearFunc = [](float depth) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearBufferfv(GL_DEPTH, 0, &depth);
    };

    maskedDepthClear(clearFunc);
}

template <typename ClearFunc>
void RobustResourceInitTest::maskedStencilClear(ClearFunc clearFunc)
{
    constexpr int kSize = 16;

    // Initialize a FBO with stencil and simple color.
    GLRenderbuffer stencilbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, stencilbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, kSize, kSize);

    GLTexture colorbuffer;
    glBindTexture(GL_TEXTURE_2D, colorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorbuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              stencilbuffer);

    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Disable stencil writes and trigger a clear. Use a tricky mask that does not overlap the
    // clear.
    glStencilMask(0xF0);
    clearFunc(0x0F);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Draw red with a stencil function that checks for stencil == 0
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x00, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glViewport(0, 0, kSize, kSize);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red) << "stencil should be equal to zero";
}

// Test that clearing a masked stencil buffer doesn't mark it clean.
TEST_P(RobustResourceInitTest, MaskedStencilClear)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    // http://anglebug.com/42261117, but only fails on Nexus devices
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsOpenGLES());

    auto clearFunc = [](GLint clearValue) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearStencil(clearValue);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    };

    maskedStencilClear(clearFunc);
}

// Test that clearing a masked stencil buffer doesn't mark it clean, with ClearBufferi.
TEST_P(RobustResourceInitTestES3, MaskedStencilClearBuffer)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    // http://anglebug.com/42261118
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL() && (IsIntel() || IsNVIDIA()));

    ANGLE_SKIP_TEST_IF(IsLinux() && IsOpenGL());

    // http://anglebug.com/42261117, but only fails on Nexus devices
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsOpenGLES());

    auto clearFunc = [](GLint clearValue) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearBufferiv(GL_STENCIL, 0, &clearValue);
    };

    maskedStencilClear(clearFunc);
}

template <int Size, typename InitializedTest>
void VerifyRGBA8PixelRect(InitializedTest inInitialized)
{
    std::array<std::array<GLColor, Size>, Size> actualPixels;
    glReadPixels(0, 0, Size, Size, GL_RGBA, GL_UNSIGNED_BYTE, actualPixels.data());
    ASSERT_GL_NO_ERROR();

    for (int y = 0; y < Size; ++y)
    {
        for (int x = 0; x < Size; ++x)
        {
            if (inInitialized(x, y))
            {
                EXPECT_EQ(actualPixels[y][x], GLColor::red) << " at " << x << ", " << y;
            }
            else
            {
                EXPECT_EQ(actualPixels[y][x], GLColor::transparentBlack)
                    << " at " << x << ", " << y;
            }
        }
    }
}

// Tests that calling CopyTexSubImage2D will initialize the source & destination.
TEST_P(RobustResourceInitTest, CopyTexSubImage2D)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    static constexpr int kDestSize = 4;
    constexpr int kSrcSize         = kDestSize / 2;
    static constexpr int kOffset   = kSrcSize / 2;

    std::vector<GLColor> redColors(kDestSize * kDestSize, GLColor::red);

    // Initialize source texture with red.
    GLTexture srcTexture;
    glBindTexture(GL_TEXTURE_2D, srcTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSrcSize, kSrcSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 redColors.data());

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTexture, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Create uninitialized destination texture.
    GLTexture destTexture;
    glBindTexture(GL_TEXTURE_2D, destTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kDestSize, kDestSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    // Trigger the copy from initialized source into uninitialized dest.
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, kOffset, kOffset, 0, 0, kSrcSize, kSrcSize);

    // Verify the pixel rectangle.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, destTexture, 0);
    ASSERT_GL_NO_ERROR();

    auto srcInitTest = [](int x, int y) {
        return (x >= kOffset) && x < (kDestSize - kOffset) && (y >= kOffset) &&
               y < (kDestSize - kOffset);
    };

    VerifyRGBA8PixelRect<kDestSize>(srcInitTest);

    // Make source texture uninitialized. Force a release by redefining a new size.
    glBindTexture(GL_TEXTURE_2D, srcTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSrcSize, kSrcSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTexture, 0);

    // Fill destination texture with red.
    glBindTexture(GL_TEXTURE_2D, destTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kDestSize, kDestSize, GL_RGBA, GL_UNSIGNED_BYTE,
                    redColors.data());

    // Trigger a copy from uninitialized source into initialized dest.
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, kOffset, kOffset, 0, 0, kSrcSize, kSrcSize);

    // Verify the pixel rectangle.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, destTexture, 0);
    ASSERT_GL_NO_ERROR();

    auto destInitTest = [srcInitTest](int x, int y) { return !srcInitTest(x, y); };

    VerifyRGBA8PixelRect<kDestSize>(destInitTest);
}

void RobustResourceInitTest::copyTexSubImage2DCustomFBOTest(int offsetX, int offsetY)
{
    const int texSize = 512;
    const int fboSize = 16;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    ASSERT_GL_NO_ERROR();

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, fboSize, fboSize);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, offsetX, offsetY, texSize, texSize);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer readbackFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, readbackFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    checkCustomFramebufferNonZeroPixels(texSize, texSize, -offsetX, -offsetY, fboSize, fboSize,
                                        GLColor::red);
}

// Test CopyTexSubImage2D clipped to size of custom FBO, zero x/y source offset.
TEST_P(RobustResourceInitTest, CopyTexSubImage2DCustomFBOZeroOffsets)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());
    copyTexSubImage2DCustomFBOTest(0, 0);
}

// Test CopyTexSubImage2D clipped to size of custom FBO, negative x/y source offset.
TEST_P(RobustResourceInitTest, CopyTexSubImage2DCustomFBONegativeOffsets)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());
    copyTexSubImage2DCustomFBOTest(-8, -8);
}

// Tests that calling CopyTexSubImage3D will initialize the source & destination.
TEST_P(RobustResourceInitTestES3, CopyTexSubImage3D)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    static constexpr int kDestSize = 4;
    constexpr int kSrcSize         = kDestSize / 2;
    static constexpr int kOffset   = kSrcSize / 2;

    std::vector<GLColor> redColors(kDestSize * kDestSize * kDestSize, GLColor::red);

    GLTexture srcTexture;
    GLFramebuffer framebuffer;
    GLTexture destTexture;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Initialize source texture with red.
    glBindTexture(GL_TEXTURE_3D, srcTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, kSrcSize, kSrcSize, kSrcSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, redColors.data());

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, srcTexture, 0, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Create uninitialized destination texture.
    glBindTexture(GL_TEXTURE_3D, destTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, kDestSize, kDestSize, kDestSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    // Trigger the copy from initialized source into uninitialized dest.
    glCopyTexSubImage3D(GL_TEXTURE_3D, 0, kOffset, kOffset, 0, 0, 0, kSrcSize, kSrcSize);

    // Verify the pixel rectangle.
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, destTexture, 0, 0);
    ASSERT_GL_NO_ERROR();

    auto srcInitTest = [](int x, int y) {
        return (x >= kOffset) && x < (kDestSize - kOffset) && (y >= kOffset) &&
               y < (kDestSize - kOffset);
    };

    VerifyRGBA8PixelRect<kDestSize>(srcInitTest);

    // Make source texture uninitialized.
    glBindTexture(GL_TEXTURE_3D, srcTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, kSrcSize, kSrcSize, kSrcSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, srcTexture, 0, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Fill destination texture with red.
    glBindTexture(GL_TEXTURE_3D, destTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, kDestSize, kDestSize, kDestSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, redColors.data());

    // Trigger a copy from uninitialized source into initialized dest.
    glCopyTexSubImage3D(GL_TEXTURE_3D, 0, kOffset, kOffset, 0, 0, 0, kSrcSize, kSrcSize);

    // Verify the pixel rectangle.
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, destTexture, 0, 0);
    ASSERT_GL_NO_ERROR();

    auto destInitTest = [srcInitTest](int x, int y) { return !srcInitTest(x, y); };

    VerifyRGBA8PixelRect<kDestSize>(destInitTest);
}

// Test basic robustness with 2D array textures.
TEST_P(RobustResourceInitTestES3, Texture2DArray)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    constexpr int kSize   = 1024;
    constexpr int kLayers = 8;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, kSize, kSize, kLayers, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    for (int layer = 0; layer < kLayers; ++layer)
    {
        checkNonZeroPixels3D(&texture, 0, 0, 0, 0, layer, GLColor::transparentBlack);
    }
}

// Test that using TexStorage2D followed by CompressedSubImage works with robust init.
// Taken from WebGL test conformance/extensions/webgl-compressed-texture-s3tc.
TEST_P(RobustResourceInitTestES3, CompressedSubImage)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    constexpr int width     = 8;
    constexpr int height    = 8;
    constexpr int subX0     = 0;
    constexpr int subY0     = 0;
    constexpr int subWidth  = 4;
    constexpr int subHeight = 4;
    constexpr GLenum format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

    static constexpr uint8_t img_8x8_rgb_dxt1[] = {
        0xe0, 0x07, 0x00, 0xf8, 0x11, 0x10, 0x15, 0x00, 0x1f, 0x00, 0xe0,
        0xff, 0x11, 0x10, 0x15, 0x00, 0xe0, 0x07, 0x1f, 0xf8, 0x44, 0x45,
        0x40, 0x55, 0x1f, 0x00, 0xff, 0x07, 0x44, 0x45, 0x40, 0x55,
    };

    static constexpr uint8_t img_4x4_rgb_dxt1[] = {
        0xe0, 0x07, 0x00, 0xf8, 0x11, 0x10, 0x15, 0x00,
    };

    std::vector<uint8_t> data(img_8x8_rgb_dxt1, img_8x8_rgb_dxt1 + ArraySize(img_8x8_rgb_dxt1));
    std::vector<uint8_t> subData(img_4x4_rgb_dxt1, img_4x4_rgb_dxt1 + ArraySize(img_4x4_rgb_dxt1));

    GLTexture colorbuffer;
    glBindTexture(GL_TEXTURE_2D, colorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorbuffer, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glViewport(0, 0, width, height);

    // testing format width-x-height via texStorage2D
    const auto &expectedData = UncompressDXTIntoSubRegion(width, height, subX0, subY0, subWidth,
                                                          subHeight, subData, format);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
    ASSERT_GL_NO_ERROR();
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, subX0, subY0, subWidth, subHeight, format,
                              static_cast<GLsizei>(subData.size()), subData.data());
    ASSERT_GL_NO_ERROR();

    draw2DTexturedQuad(0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    std::vector<GLColor> actualData(width * height);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, actualData.data());
    ASSERT_GL_NO_ERROR();

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int offset                  = x + y * width;
            const GLColor expectedColor = expectedData[offset];
            const GLColor actualColor   = actualData[offset];

            // Allow for some minor variation because the format is compressed.
            EXPECT_NEAR(expectedColor.R, actualColor.R, 1) << " at (" << x << ", " << y << ")";
            EXPECT_NEAR(expectedColor.G, actualColor.G, 1) << " at (" << x << ", " << y << ")";
            EXPECT_NEAR(expectedColor.B, actualColor.B, 1) << " at (" << x << ", " << y << ")";
        }
    }
}

// Test drawing to a framebuffer with not all draw buffers enabled
TEST_P(RobustResourceInitTestES3, SparseDrawBuffers)
{
    constexpr char kVS[] = R"(#version 300 es
void main() {
  gl_PointSize = 100.0;
  gl_Position = vec4(0, 0, 0, 1);
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
layout(location = 1) out vec4 output1;
layout(location = 3) out vec4 output2;
void main()
{
    output1 = vec4(0.0, 1.0, 0.0, 1.0);
    output2 = vec4(0.0, 0.0, 1.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    std::vector<GLTexture> textures(4);
    for (size_t i = 0; i < textures.size(); i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i],
                               0);
    }

    glViewport(0, 0, 1, 1);

    constexpr GLenum drawBuffers[4] = {
        GL_NONE,
        GL_COLOR_ATTACHMENT1,
        GL_NONE,
        GL_COLOR_ATTACHMENT3,
    };
    glDrawBuffers(4, drawBuffers);

    glDrawArrays(GL_POINTS, 0, 1);

    const GLColor expectedColors[4] = {
        GLColor::transparentBlack,
        GLColor::green,
        GLColor::transparentBlack,
        GLColor::blue,
    };
    for (size_t i = 0; i < textures.size(); i++)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        EXPECT_PIXEL_COLOR_EQ(0, 0, expectedColors[i]) << " at attachment " << i;
    }
}

// Tests that a partial scissor still initializes contents as expected.
TEST_P(RobustResourceInitTest, ClearWithScissor)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    constexpr int kSize = 16;

    GLRenderbuffer colorbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuffer);

    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Scissor to half the width.
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, kSize / 2, kSize);

    // Clear. Half the texture should be black, and half red.
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::transparentBlack);

    GLint initState = 0;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RESOURCE_INITIALIZED_ANGLE, &initState);
    EXPECT_GL_TRUE(initState);
}

// Tests that surfaces are initialized when they are created
TEST_P(RobustResourceInitTest, SurfaceInitialized)
{
    ANGLE_SKIP_TEST_IF(!hasRobustSurfaceInit());

    checkFramebufferNonZeroPixels(0, 0, 0, 0, GLColor::black);
}

// Tests that surfaces are initialized after swapping if they are not preserved
TEST_P(RobustResourceInitTest, SurfaceInitializedAfterSwap)
{
    ANGLE_SKIP_TEST_IF(!hasRobustSurfaceInit());

    EGLint swapBehaviour = 0;
    ASSERT_TRUE(eglQuerySurface(getEGLWindow()->getDisplay(), getEGLWindow()->getSurface(),
                                EGL_SWAP_BEHAVIOR, &swapBehaviour));

    const std::array<GLColor, 4> clearColors = {{
        GLColor::blue,
        GLColor::cyan,
        GLColor::red,
        GLColor::yellow,
    }};

    if (swapBehaviour != EGL_BUFFER_PRESERVED)
    {
        checkFramebufferNonZeroPixels(0, 0, 0, 0, GLColor::black);
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, 1, 1);

    for (size_t i = 0; i < clearColors.size(); i++)
    {
        if (swapBehaviour == EGL_BUFFER_PRESERVED && i > 0)
        {
            EXPECT_PIXEL_COLOR_EQ(0, 0, clearColors[i - 1]);
        }

        angle::Vector4 clearColor = clearColors[i].toNormalizedVector();
        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_NO_ERROR();

        if (swapBehaviour != EGL_BUFFER_PRESERVED)
        {
            // Only scissored area (0, 0, 1, 1) has clear color.
            // The rest should be robust initialized.
            checkFramebufferNonZeroPixels(0, 0, 1, 1, clearColors[i]);
        }

        swapBuffers();
    }
}

// Test that multisampled 2D textures are initialized.
TEST_P(RobustResourceInitTestES31, Multisample2DTexture)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 2, GL_RGBA8, kWidth, kHeight, false);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           texture, 0);

    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight);

    GLFramebuffer resolveFramebuffer;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFramebuffer);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture,
                           0);
    ASSERT_GL_NO_ERROR();

    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFramebuffer);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::transparentBlack);
}

// Test that multisampled 2D texture arrays from OES_texture_storage_multisample_2d_array are
// initialized.
TEST_P(RobustResourceInitTestES31, Multisample2DTextureArray)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    if (IsGLExtensionRequestable("GL_OES_texture_storage_multisample_2d_array"))
    {
        glRequestExtensionANGLE("GL_OES_texture_storage_multisample_2d_array");
    }
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_storage_multisample_2d_array"));

    const GLsizei kLayers = 4;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, texture);
    glTexStorage3DMultisampleOES(GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES, 2, GL_RGBA8, kWidth, kHeight,
                                 kLayers, false);

    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight);

    GLFramebuffer resolveFramebuffer;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFramebuffer);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture,
                           0);
    ASSERT_GL_NO_ERROR();

    for (GLsizei layerIndex = 0; layerIndex < kLayers; ++layerIndex)
    {
        GLFramebuffer framebuffer;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0,
                                  layerIndex);

        glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFramebuffer);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::transparentBlack);
    }
}

// Test to cover a bug that the multisampled depth attachment of a framebuffer are not successfully
// initialized before it is used as the read framebuffer in blitFramebuffer.
// Referenced from the following WebGL CTS:
// conformance2/renderbuffers/multisampled-depth-renderbuffer-initialization.html
TEST_P(RobustResourceInitTestES3, InitializeMultisampledDepthRenderbufferAfterCopyTextureCHROMIUM)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_CHROMIUM_copy_texture"));

    // http://anglebug.com/42263936
    ANGLE_SKIP_TEST_IF(IsAMD() && IsD3D11());

    // Call glCopyTextureCHROMIUM to set destTexture as the color attachment of the internal
    // framebuffer mScratchFBO.
    GLTexture sourceTexture;
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    GLTexture destTexture;
    glBindTexture(GL_TEXTURE_2D, destTexture);
    glCopyTextureCHROMIUM(sourceTexture, 0, GL_TEXTURE_2D, destTexture, 0, GL_RGBA,
                          GL_UNSIGNED_BYTE, GL_FALSE, GL_FALSE, GL_FALSE);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer drawFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, drawFbo);

    GLTexture colorTex;
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    GLRenderbuffer drawDepthRbo;
    glBindRenderbuffer(GL_RENDERBUFFER, drawDepthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, drawDepthRbo);

    // Clear drawDepthRbo to 0.0f
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    constexpr uint32_t kReadDepthRboSampleCount = 4;
    GLFramebuffer readFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, readFbo);
    GLRenderbuffer readDepthRbo;
    glBindRenderbuffer(GL_RENDERBUFFER, readDepthRbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, kReadDepthRboSampleCount,
                                     GL_DEPTH_COMPONENT16, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, readDepthRbo);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo);

    // Blit from readDepthRbo to drawDepthRbo. When robust resource init is enabled, readDepthRbo
    // should be initialized to 1.0f by default, so the data in drawDepthRbo should also be 1.0f.
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, drawFbo);
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    // If drawDepthRbo is correctly set to 1.0f, the depth test can always pass, so the result
    // should be green.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Corner case for robust resource init: CopyTexImage to a cube map.
TEST_P(RobustResourceInitTest, CopyTexImageToOffsetCubeMap)
{
    ANGLE_SKIP_TEST_IF(!hasGLExtension());

    constexpr GLuint kSize = 2;

    std::vector<GLColor> redPixels(kSize * kSize, GLColor::red);

    GLTexture srcTex;
    glBindTexture(GL_TEXTURE_2D, srcTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 redPixels.data());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTex, 0);

    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    GLTexture dstTex;
    glBindTexture(GL_TEXTURE_CUBE_MAP, dstTex);
    glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, 1, 1, kSize, kSize, 0);
    glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, -1, -1, kSize, kSize, 0);
    glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, 2, 2, kSize, kSize, 0);
    glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, -2, -2, kSize, kSize, 0);
    glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, 0, 0, kSize, kSize, 0);
    glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, 0, 0, kSize, kSize, 0);

    ASSERT_GL_NO_ERROR();

    // Verify the offset attachments.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                           dstTex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(1, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(0, 1, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::transparentBlack);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                           dstTex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(1, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(0, 1, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
}

TEST_P(RobustResourceInitTestES3, CheckDepthStencilRenderbufferIsCleared)
{
    ANGLE_SKIP_TEST_IF(!hasRobustSurfaceInit());

    GLRenderbuffer colorRb;
    GLFramebuffer fb;
    GLRenderbuffer depthStencilRb;

    // Make a framebuffer with RGBA + DEPTH_STENCIL
    glBindRenderbuffer(GL_RENDERBUFFER, colorRb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);

    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilRb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kWidth, kHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencilRb);
    ASSERT_GL_NO_ERROR();

    // Render a quad at Z = 1.0 with depth test on and depth function set to GL_EQUAL.
    // If the depth buffer is not cleared to 1.0 this will fail
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_EQUAL);

    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 1.0f, 1.0f, true);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::green);

    // Render with stencil test on and stencil function set to GL_EQUAL
    // If the stencil is not zero this will fail.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, 0, 0xFF);

    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 1.0f, 1.0f, true);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test that default framebuffer depth and stencil are cleared to values that
// are consistent with non-default framebuffer clear values. Depth 1.0, stencil 0.0.
TEST_P(RobustResourceInitTestES3, CheckDefaultDepthStencilRenderbufferIsCleared)
{
    ANGLE_SKIP_TEST_IF(!hasRobustSurfaceInit());

    // Render a quad at Z = 1.0 with depth test on and depth function set to GL_EQUAL.
    // If the depth buffer is not cleared to 1.0 this will fail
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_EQUAL);

    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 1.0f, 1.0f, true);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::green);

    // Render with stencil test on and stencil function set to GL_EQUAL
    // If the stencil is not zero this will fail.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, 0, 0xFF);

    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 1.0f, 1.0f, true);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

TEST_P(RobustResourceInitTestES3, CheckMultisampleDepthStencilRenderbufferIsCleared)
{
    ANGLE_SKIP_TEST_IF(!hasRobustSurfaceInit());

    GLRenderbuffer colorRb;
    GLFramebuffer fb;

    // Make a framebuffer with RGBA
    glBindRenderbuffer(GL_RENDERBUFFER, colorRb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRb);
    ASSERT_GL_NO_ERROR();

    // Make a corresponding multisample framebuffer with RGBA + DEPTH_STENCIL
    constexpr int kSamples = 4;
    GLRenderbuffer multisampleColorRb;
    GLFramebuffer multisampleFb;
    GLRenderbuffer multisampleDepthStencilRb;

    // Make a framebuffer with RGBA + DEPTH_STENCIL
    glBindRenderbuffer(GL_RENDERBUFFER, multisampleColorRb);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, kSamples, GL_RGBA8, kWidth, kHeight);

    glBindRenderbuffer(GL_RENDERBUFFER, multisampleDepthStencilRb);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, kSamples, GL_DEPTH24_STENCIL8, kWidth,
                                     kHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, multisampleFb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              multisampleColorRb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              multisampleDepthStencilRb);
    ASSERT_GL_NO_ERROR();

    // Render a quad at Z = 1.0 with depth test on and depth function set to GL_EQUAL.
    // If the depth buffer is not cleared to 1.0 this will fail
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_EQUAL);

    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 1.0f, 1.0f, true);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::green);

    // Render with stencil test on and stencil function set to GL_EQUAL
    // If the stencil is not zero this will fail.
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, 0, 0xFF);

    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glBindFramebuffer(GL_FRAMEBUFFER, multisampleFb);
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 1.0f, 1.0f, true);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test that blit between two depth/stencil buffers after glClearBufferfi works.  The blit is done
// once expecting robust resource init value, then clear is called with the same value as the robust
// init, and blit is done again.  This triggers an optimization in the Vulkan backend where the
// second clear is no-oped.
TEST_P(RobustResourceInitTestES3, BlitDepthStencilAfterClearBuffer)
{
    ANGLE_SKIP_TEST_IF(!hasRobustSurfaceInit());

    // http://anglebug.com/42263848
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    // http://anglebug.com/42263847
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr GLsizei kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer readFbo, drawFbo;
    GLRenderbuffer readDepthStencil, drawDepthStencil;

    // Create destination framebuffer.
    glBindRenderbuffer(GL_RENDERBUFFER, drawDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);
    glBindFramebuffer(GL_FRAMEBUFFER, drawFbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              drawDepthStencil);
    ASSERT_GL_NO_ERROR();

    // Create source framebuffer
    glBindRenderbuffer(GL_RENDERBUFFER, readDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);
    glBindFramebuffer(GL_FRAMEBUFFER, readFbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              readDepthStencil);
    ASSERT_GL_NO_ERROR();

    // Blit once with the robust resource init clear.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize,
                      GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Verify that the blit was successful.
    GLRenderbuffer color;
    glBindRenderbuffer(GL_RENDERBUFFER, color);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
    ASSERT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, 0, 0xFF);

    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.95f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFbo);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::red);

    // Clear to the same value as robust init, and blit again.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, readFbo);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo);
    glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.5f, 0x3C);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize,
                      GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Verify that the blit was successful.
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.95f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFbo);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::green);
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(
    RobustResourceInitTest,
    ES3_METAL().enable(Feature::EmulateDontCareLoadWithRandomClear),
    ES2_VULKAN().enable(Feature::AllocateNonZeroMemory));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(RobustResourceInitTestES3);
ANGLE_INSTANTIATE_TEST_ES3_AND(RobustResourceInitTestES3,
                               ES3_METAL().enable(Feature::EmulateDontCareLoadWithRandomClear),
                               ES3_VULKAN().enable(Feature::AllocateNonZeroMemory));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(RobustResourceInitTestES31);
ANGLE_INSTANTIATE_TEST_ES31_AND(RobustResourceInitTestES31,
                                ES31_VULKAN().enable(Feature::AllocateNonZeroMemory));

}  // namespace angle
