//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CopyTexture3DTest.cpp: Tests of the GL_ANGLE_copy_texture_3d extension

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace angle
{

class CopyTexture3DTest : public ANGLETest<>
{
  protected:
    CopyTexture3DTest()
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
        const char *vertexShaderSource   = getVertexShaderSource();
        const char *fragmentShaderSource = getFragmentShaderSource();

        mProgram = CompileProgram(vertexShaderSource, fragmentShaderSource);
        ASSERT_NE(0u, mProgram);

        glUseProgram(mProgram);

        ASSERT_GL_NO_ERROR();
    }

    const char *getVertexShaderSource()
    {
        return "#version 300 es\n"
               "out vec3 texcoord;\n"
               "in vec4 position;\n"
               "void main()\n"
               "{\n"
               "    gl_Position = vec4(position.xy, 0.0, 1.0);\n"
               "    texcoord = (position.xyz * 0.5) + 0.5;\n"
               "}\n";
    }

    bool checkExtensions() const
    {
        if (!IsGLExtensionEnabled("GL_ANGLE_copy_texture_3d"))
        {
            std::cout << "Test skipped because GL_ANGLE_copy_texture_3d is not available."
                      << std::endl;
            return false;
        }

        EXPECT_NE(nullptr, glCopyTexture3DANGLE);
        EXPECT_NE(nullptr, glCopySubTexture3DANGLE);
        return true;
    }

    void testCopy(const GLenum testTarget,
                  const GLColor &sourceColor,
                  GLenum destInternalFormat,
                  GLenum destType,
                  bool flipY,
                  bool premultiplyAlpha,
                  bool unmultiplyAlpha,
                  const GLColor &expectedColor)
    {
        std::vector<GLColor> texDataColor(2u * 2u * 2u, sourceColor);

        glBindTexture(testTarget, sourceTexture);
        glTexImage3D(testTarget, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     texDataColor.data());
        glTexParameteri(testTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(testTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(testTarget, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(testTarget, GL_TEXTURE_MAX_LEVEL, 0);
        EXPECT_GL_NO_ERROR();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(testTarget, destTexture);
        glCopyTexture3DANGLE(sourceTexture, 0, testTarget, destTexture, 0, destInternalFormat,
                             destType, flipY, premultiplyAlpha, unmultiplyAlpha);
        EXPECT_GL_NO_ERROR();

        GLRenderbuffer rbo;
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);

        GLenum renderType = 0;

        switch (destInternalFormat)
        {
            case GL_RGB:
            case GL_RGBA:
            case GL_LUMINANCE:
            case GL_LUMINANCE_ALPHA:
            case GL_ALPHA:
            case GL_R8:
            case GL_RG:
            case GL_RG8:
            case GL_RGB8:
            case GL_RGBA8:
            case GL_RGBX8_ANGLE:
            case GL_SRGB8:
            case GL_RGB565:
            case GL_SRGB8_ALPHA8:
            case GL_RGB5_A1:
            case GL_RGBA4:
            case GL_R8_SNORM:
            case GL_RG8_SNORM:
            case GL_RGB8_SNORM:
            case GL_RGBA8_SNORM:
            case GL_RGB10_A2:
                renderType = GL_RGBA8;
                break;
            case GL_R8I:
            case GL_R16I:
            case GL_R32I:
            case GL_RG8I:
            case GL_RG16I:
            case GL_RG32I:
            case GL_RGB8I:
            case GL_RGB16I:
            case GL_RGB32I:
            case GL_RGBA8I:
            case GL_RGBA16I:
            case GL_RGBA32I:
                renderType = GL_RGBA8I;
                break;
            case GL_R8UI:
            case GL_R16UI:
            case GL_R32UI:
            case GL_RG8UI:
            case GL_RG16UI:
            case GL_RG32UI:
            case GL_RGB8UI:
            case GL_RGB16UI:
            case GL_RGB32UI:
            case GL_RGBA8UI:
            case GL_RGBA16UI:
            case GL_RGBA32UI:
            case GL_RGB10_A2UI:
                renderType = GL_RGBA8UI;
                break;
            case GL_R16F:
            case GL_RGB16F:
            case GL_RGB32F:
            case GL_R32F:
            case GL_RG16F:
            case GL_RG32F:
            case GL_RGBA16F:
            case GL_RGBA32F:
            case GL_R11F_G11F_B10F:
            case GL_RGB9_E5:
                renderType = GL_RGBA32F;
                break;
            default:
                ASSERT_TRUE(false);
        }

        glRenderbufferStorage(GL_RENDERBUFFER, renderType, 1, 1);

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

        glTexParameteri(testTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(testTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(testTarget, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(testTarget, GL_TEXTURE_MAX_LEVEL, 0);

        drawQuad(mProgram, "position", 0.5f);
        EXPECT_GL_NO_ERROR();

        if (renderType == GL_RGBA8)
        {
            uint32_t tolerance = 1;
            // The destination formats may be emulated, so the precision may be higher than
            // required.  Increase the tolerance to 2^(8-width).  8 is for the 8-bit format used for
            // emulation.  Even though Vulkan recommends round to nearest, it's not a requirement
            // so the full-range of precision is used as tolerance.
            switch (destType)
            {
                case GL_UNSIGNED_SHORT_5_6_5:
                    tolerance = 8;
                    break;
                case GL_UNSIGNED_SHORT_5_5_5_1:
                    tolerance = 8;
                    break;
                case GL_UNSIGNED_SHORT_4_4_4_4:
                    tolerance = 16;
                    break;
                default:
                    break;
            }
            switch (destInternalFormat)
            {
                case GL_RGB565:
                    tolerance = 8;
                    break;
                case GL_RGB5_A1:
                    tolerance = 8;
                    break;
                case GL_RGBA4:
                    tolerance = 16;
                    break;
                default:
                    break;
            }

            // If destination is SNORM, values in between representable values could round either
            // way.
            switch (destInternalFormat)
            {
                case GL_R8_SNORM:
                case GL_RG8_SNORM:
                case GL_RGB8_SNORM:
                case GL_RGBA8_SNORM:
                    tolerance *= 2;
                    break;
                default:
                    break;
            }

            GLColor actual;
            glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &actual.R);
            EXPECT_GL_NO_ERROR();
            EXPECT_COLOR_NEAR(expectedColor, actual, tolerance);
            return;
        }
        else if (renderType == GL_RGBA32F)
        {
            float expectedColorFloat[4] = {static_cast<float>(expectedColor.R) / 255,
                                           static_cast<float>(expectedColor.G) / 255,
                                           static_cast<float>(expectedColor.B) / 255,
                                           static_cast<float>(expectedColor.A) / 255};
            EXPECT_PIXEL_COLOR32F_NEAR(0, 0,
                                       GLColor32F(expectedColorFloat[0], expectedColorFloat[1],
                                                  expectedColorFloat[2], expectedColorFloat[3]),
                                       0.015);
            return;
        }
        else if (renderType == GL_RGBA8UI)
        {
            GLuint pixel[4] = {0};
            glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_UNSIGNED_INT, pixel);
            EXPECT_COLOR_NEAR(
                expectedColor,
                GLColor(static_cast<GLubyte>(pixel[0]), static_cast<GLubyte>(pixel[1]),
                        static_cast<GLubyte>(pixel[2]), static_cast<GLubyte>(pixel[3])),
                0.2);
        }
        else if (renderType == GL_RGBA8I)
        {
            GLint pixel[4] = {0};
            glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_INT, pixel);
            EXPECT_COLOR_NEAR(
                expectedColor,
                GLColor(static_cast<GLubyte>(pixel[0]), static_cast<GLubyte>(pixel[1]),
                        static_cast<GLubyte>(pixel[2]), static_cast<GLubyte>(pixel[3])),
                0.2);
        }
        else
        {
            ASSERT_TRUE(false);
        }
    }

    void testUnsizedFormats(const GLenum testTarget);
    void testSnormFormats(const GLenum testTarget);
    void testUnsignedByteFormats(const GLenum testTarget);
    void testFloatFormats(const GLenum testTarget);
    void testIntFormats(const GLenum testTarget);
    void testUintFormats(const GLenum testTarget);

    virtual const char *getFragmentShaderSource() = 0;

    GLuint mProgram = 0;
    GLTexture sourceTexture;
    GLTexture destTexture;
};

class Texture3DCopy : public CopyTexture3DTest
{
  protected:
    Texture3DCopy() {}

    const char *getFragmentShaderSource() override
    {
        return "#version 300 es\n"
               "precision highp float;\n"
               "uniform highp sampler3D tex3D;\n"
               "in vec3 texcoord;\n"
               "out vec4 fragColor;\n"
               "void main()\n"
               "{\n"
               "    fragColor = texture(tex3D, vec3(texcoord.x, texcoord.z, texcoord.y));\n"
               "}\n";
    }
};

class Texture2DArrayCopy : public CopyTexture3DTest
{
  protected:
    Texture2DArrayCopy() {}

    const char *getFragmentShaderSource() override
    {
        return "#version 300 es\n"
               "precision highp float;\n"
               "uniform highp sampler2DArray tex2DArray;\n"
               "in vec3 texcoord;\n"
               "out vec4 fragColor;\n"
               "void main()\n"
               "{\n"
               "    fragColor = texture(tex2DArray, vec3(texcoord.x, texcoord.z, texcoord.y));\n"
               "}\n";
    }
};

// Test that glCopySubTexture3DANGLE correctly copies to and from a GL_TEXTURE_3D texture.
TEST_P(Texture3DCopy, CopySubTexture)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    std::vector<GLColor> texDataGreen(2u * 2u * 2u, GLColor::green);
    std::vector<GLColor> texDataRed(2u * 2u * 2u, GLColor::red);

    glBindTexture(GL_TEXTURE_3D, sourceTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texDataGreen.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
    EXPECT_GL_NO_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, destTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texDataRed.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
    EXPECT_GL_NO_ERROR();
    glCopySubTexture3DANGLE(sourceTexture, 0, GL_TEXTURE_3D, destTexture, 0, 0, 0, 0, 0, 0, 0, 2, 2,
                            2, false, false, false);
    glBindTexture(GL_TEXTURE_3D, destTexture);
    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that glCopyTexture3DANGLE correctly copies from a non-zero mipmap on a GL_TEXTURE_3D
// texture.
TEST_P(Texture3DCopy, CopyFromMipmap)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    std::vector<GLColor> texDataGreen(4u * 4u * 4u, GLColor::green);

    glBindTexture(GL_TEXTURE_3D, sourceTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 4, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texDataGreen.data());
    glTexImage3D(GL_TEXTURE_3D, 1, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texDataGreen.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 1);

    EXPECT_GL_NO_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, destTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
    EXPECT_GL_NO_ERROR();

    glCopyTexture3DANGLE(sourceTexture, 1, GL_TEXTURE_3D, destTexture, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         false, false, false);
    glBindTexture(GL_TEXTURE_3D, destTexture);
    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that glCopySubTexture3DANGLE's offset and dimension parameters work correctly with a
// GL_TEXTURE_3D texture.
TEST_P(Texture3DCopy, OffsetSubCopy)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    GLColor rgbaPixels[27];

    // Create pixel data for a 3x3x3 red cube
    for (int i = 0; i < 27; i++)
    {
        rgbaPixels[i] = GLColor(255u, 0u, 0u, 255u);
    }

    // Change a pixel to create a 1x1x1 blue cube at (0, 0, 0)
    rgbaPixels[0] = GLColor(0u, 0u, 255u, 255u);

    // Change some pixels to green to create a 2x2x2 cube starting at (1, 1, 1)
    rgbaPixels[13] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[14] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[16] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[17] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[22] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[23] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[25] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[26] = GLColor(0u, 255u, 0u, 255u);

    glBindTexture(GL_TEXTURE_3D, sourceTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 3, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
    EXPECT_GL_NO_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, destTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    EXPECT_GL_NO_ERROR();
    // Copy the 2x2x2 green cube into a new texture
    glCopySubTexture3DANGLE(sourceTexture, 0, GL_TEXTURE_3D, destTexture, 0, 0, 0, 0, 1, 1, 1, 2, 2,
                            2, false, false, false);
    glBindTexture(GL_TEXTURE_3D, destTexture);
    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, "position", 1.0f);
    int width  = getWindowWidth() - 1;
    int height = getWindowHeight() - 1;

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(width, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, height, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(width, height, GLColor::green);

    // Copy the 1x1x1 blue cube into the the 2x2x2 green cube at location (1, 1, 1)
    glCopySubTexture3DANGLE(sourceTexture, 0, GL_TEXTURE_3D, destTexture, 0, 1, 1, 1, 0, 0, 0, 1, 1,
                            1, false, false, false);
    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, "position", 1.0f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(width, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, height, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(width, height, GLColor::blue);
}

// Test that the flipY parameter works with a GL_TEXTURE_3D texture.
TEST_P(Texture3DCopy, FlipY)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    // Create a 2x2x2 cube. The top half is red. The bottom half is green.
    GLColor rgbaPixels[8] = {GLColor::green, GLColor::green, GLColor::red, GLColor::red,
                             GLColor::green, GLColor::green, GLColor::red, GLColor::red};

    glBindTexture(GL_TEXTURE_3D, sourceTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // mProgram creates a quad with the colors from the top (y-direction) layer of the 3D texture.
    // 3D pixel (<x>, <y>, <z>) is drawn to 2D pixel (<x>, <z>) for <y> = 1.
    drawQuad(mProgram, "position", 1.0f);

    int width  = getWindowWidth() - 1;
    int height = getWindowHeight() - 1;

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(width, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, height, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(width, height, GLColor::red);

    EXPECT_GL_NO_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, destTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    // Flip the y coordinate. This will put the greem half on top, and the red half on the bottom.
    glCopyTexture3DANGLE(sourceTexture, 0, GL_TEXTURE_3D, destTexture, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         true, false, false);

    glBindTexture(GL_TEXTURE_3D, destTexture);
    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, "position", 1.0f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, height, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(width, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(width, height, GLColor::green);
}

void CopyTexture3DTest::testUnsizedFormats(const GLenum testTarget)
{
    const GLColor kColorNoAlpha(250, 200, 150, 100);
    const GLColor kColorPreAlpha(250, 200, 150, 100);
    const GLColor kColorUnAlpha(101, 150, 200, 200);

    testCopy(testTarget, kColorNoAlpha, GL_RGB, GL_UNSIGNED_BYTE, false, false, false,
             GLColor(250, 200, 150, 255));
    testCopy(testTarget, kColorPreAlpha, GL_RGB, GL_UNSIGNED_BYTE, false, true, false,
             GLColor(98, 78, 59, 255));
    testCopy(testTarget, kColorUnAlpha, GL_RGB, GL_UNSIGNED_BYTE, false, false, true,
             GLColor(128, 191, 255, 255));

    testCopy(testTarget, kColorNoAlpha, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, false, false, false,
             GLColor(247, 199, 148, 255));
    testCopy(testTarget, kColorPreAlpha, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, false, true, false,
             GLColor(99, 77, 57, 255));
    testCopy(testTarget, kColorUnAlpha, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, false, false, true,
             GLColor(132, 190, 255, 255));

    testCopy(testTarget, kColorNoAlpha, GL_RGBA, GL_UNSIGNED_BYTE, false, false, false,
             GLColor(250, 200, 150, 100));
    testCopy(testTarget, kColorPreAlpha, GL_RGBA, GL_UNSIGNED_BYTE, false, true, false,
             GLColor(98, 78, 59, 100));
    testCopy(testTarget, kColorUnAlpha, GL_RGBA, GL_UNSIGNED_BYTE, false, false, true,
             GLColor(128, 191, 255, 200));

    testCopy(testTarget, kColorNoAlpha, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, false, false, false,
             GLColor(250, 200, 150, 100));
    testCopy(testTarget, kColorPreAlpha, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, false, true, false,
             GLColor(98, 78, 59, 100));
    testCopy(testTarget, kColorUnAlpha, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, false, false, true,
             GLColor(128, 191, 255, 200));

    testCopy(testTarget, kColorNoAlpha, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, false, false, false,
             GLColor(247, 198, 148, 0));
    testCopy(testTarget, kColorPreAlpha, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, false, true, false,
             GLColor(99, 82, 57, 0));
    testCopy(testTarget, kColorUnAlpha, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, false, false, true,
             GLColor(132, 189, 255, 255));

    testCopy(testTarget, kColorNoAlpha, GL_LUMINANCE, GL_UNSIGNED_BYTE, false, false, false,
             GLColor(250, 250, 250, 255));
    testCopy(testTarget, kColorPreAlpha, GL_LUMINANCE, GL_UNSIGNED_BYTE, false, true, false,
             GLColor(98, 98, 98, 255));
    testCopy(testTarget, kColorUnAlpha, GL_LUMINANCE, GL_UNSIGNED_BYTE, false, false, true,
             GLColor(128, 128, 128, 255));

    testCopy(testTarget, kColorNoAlpha, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, false, false, false,
             GLColor(250, 250, 250, 100));
    testCopy(testTarget, kColorPreAlpha, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, false, true, false,
             GLColor(98, 98, 98, 100));
    testCopy(testTarget, kColorUnAlpha, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, false, false, true,
             GLColor(128, 128, 128, 200));

    testCopy(testTarget, kColorNoAlpha, GL_ALPHA, GL_UNSIGNED_BYTE, false, false, false,
             GLColor(0, 0, 0, 100));
    testCopy(testTarget, kColorNoAlpha, GL_ALPHA, GL_UNSIGNED_BYTE, false, false, false,
             GLColor(0, 0, 0, 100));
    testCopy(testTarget, kColorNoAlpha, GL_ALPHA, GL_UNSIGNED_BYTE, false, false, false,
             GLColor(0, 0, 0, 100));
}

// Test passthrough, premultiply alpha, and unmultiply alpha copies for GL_TEXTURE_3D with unsized
// formats.
TEST_P(Texture3DCopy, UnsizedFormats)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    testUnsizedFormats(GL_TEXTURE_3D);
}

void CopyTexture3DTest::testSnormFormats(const GLenum testTarget)
{
    const GLColor kColorNoAlpha(250, 200, 150, 190);
    const GLColor kColorPreAlpha(250, 200, 150, 190);
    const GLColor kColorUnAlpha(200, 150, 100, 230);

    testCopy(testTarget, kColorNoAlpha, GL_R8_SNORM, GL_BYTE, false, false, false,
             GLColor(251, 0, 0, 255));
    testCopy(testTarget, kColorPreAlpha, GL_R8_SNORM, GL_BYTE, false, true, false,
             GLColor(187, 0, 0, 255));
    testCopy(testTarget, kColorUnAlpha, GL_R8_SNORM, GL_BYTE, false, false, true,
             GLColor(221, 0, 0, 255));

    testCopy(testTarget, kColorNoAlpha, GL_RG8_SNORM, GL_BYTE, false, false, false,
             GLColor(251, 201, 0, 255));
    testCopy(testTarget, kColorPreAlpha, GL_RG8_SNORM, GL_BYTE, false, true, false,
             GLColor(187, 149, 0, 255));
    testCopy(testTarget, kColorUnAlpha, GL_RG8_SNORM, GL_BYTE, false, false, true,
             GLColor(221, 167, 0, 255));

    testCopy(testTarget, kColorNoAlpha, GL_RGB8_SNORM, GL_BYTE, false, false, false,
             GLColor(251, 201, 151, 255));
    testCopy(testTarget, kColorPreAlpha, GL_RGB8_SNORM, GL_BYTE, false, true, false,
             GLColor(187, 149, 112, 255));
    testCopy(testTarget, kColorUnAlpha, GL_RGB8_SNORM, GL_BYTE, false, false, true,
             GLColor(221, 167, 110, 255));

    testCopy(testTarget, kColorNoAlpha, GL_RGBA8_SNORM, GL_BYTE, false, false, false,
             GLColor(251, 201, 151, 191));
    testCopy(testTarget, kColorPreAlpha, GL_RGBA8_SNORM, GL_BYTE, false, true, false,
             GLColor(187, 149, 112, 191));
    testCopy(testTarget, kColorUnAlpha, GL_RGBA8_SNORM, GL_BYTE, false, false, true,
             GLColor(221, 167, 110, 231));

    testCopy(testTarget, GLColor(250, 200, 150, 100), GL_RGB10_A2, GL_UNSIGNED_INT_2_10_10_10_REV,
             false, false, false, GLColor(250, 200, 150, 85));
    testCopy(testTarget, GLColor(250, 200, 150, 200), GL_RGB10_A2, GL_UNSIGNED_INT_2_10_10_10_REV,
             false, true, false, GLColor(196, 157, 118, 170));
    testCopy(testTarget, GLColor(101, 150, 190, 200), GL_RGB10_A2, GL_UNSIGNED_INT_2_10_10_10_REV,
             false, false, true, GLColor(128, 191, 242, 170));
}

// Test passthrough, premultiply alpha, and unmultiply alpha copies for GL_TEXTURE_3D with snorm
// formats.
TEST_P(Texture3DCopy, SnormFormats)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    testSnormFormats(GL_TEXTURE_3D);
}

void CopyTexture3DTest::testUnsignedByteFormats(const GLenum testTarget)
{
    {
        const GLColor kColorNoAlpha(250, 200, 150, 100);
        const GLColor kColorPreAlpha(250, 200, 150, 100);
        const GLColor kColorUnAlpha(200, 150, 100, 230);

        testCopy(testTarget, kColorNoAlpha, GL_R8, GL_UNSIGNED_BYTE, false, false, false,
                 GLColor(250, 0, 0, 255));
        testCopy(testTarget, kColorPreAlpha, GL_R8, GL_UNSIGNED_BYTE, false, true, false,
                 GLColor(98, 0, 0, 255));
        testCopy(testTarget, kColorUnAlpha, GL_R8, GL_UNSIGNED_BYTE, false, false, true,
                 GLColor(221, 0, 0, 255));

        testCopy(testTarget, kColorNoAlpha, GL_RG8, GL_UNSIGNED_BYTE, false, false, false,
                 GLColor(250, 200, 0, 255));
        testCopy(testTarget, kColorPreAlpha, GL_RG8, GL_UNSIGNED_BYTE, false, true, false,
                 GLColor(98, 78, 0, 255));
        testCopy(testTarget, kColorUnAlpha, GL_RG8, GL_UNSIGNED_BYTE, false, false, true,
                 GLColor(221, 167, 0, 255));

        testCopy(testTarget, kColorNoAlpha, GL_RGB8, GL_UNSIGNED_BYTE, false, false, false,
                 GLColor(250, 200, 150, 255));
        testCopy(testTarget, kColorPreAlpha, GL_RGB8, GL_UNSIGNED_BYTE, false, true, false,
                 GLColor(98, 78, 59, 255));
        testCopy(testTarget, kColorUnAlpha, GL_RGB8, GL_UNSIGNED_BYTE, false, false, true,
                 GLColor(221, 167, 110, 255));

        testCopy(testTarget, kColorNoAlpha, GL_RGBA8, GL_UNSIGNED_BYTE, false, false, false,
                 GLColor(250, 200, 150, 100));
        testCopy(testTarget, kColorPreAlpha, GL_RGBA8, GL_UNSIGNED_BYTE, false, true, false,
                 GLColor(98, 78, 59, 100));
        testCopy(testTarget, kColorUnAlpha, GL_RGBA8, GL_UNSIGNED_BYTE, false, false, true,
                 GLColor(221, 167, 110, 230));

        testCopy(testTarget, kColorNoAlpha, GL_RGBA4, GL_UNSIGNED_BYTE, false, false, false,
                 GLColor(250, 200, 150, 100));
        testCopy(testTarget, kColorPreAlpha, GL_RGBA4, GL_UNSIGNED_BYTE, false, true, false,
                 GLColor(98, 78, 59, 100));
        testCopy(testTarget, kColorUnAlpha, GL_RGBA4, GL_UNSIGNED_BYTE, false, false, true,
                 GLColor(221, 167, 110, 230));

        testCopy(testTarget, kColorNoAlpha, GL_RGBA4, GL_UNSIGNED_SHORT_4_4_4_4, false, false,
                 false, GLColor(250, 200, 150, 100));
        testCopy(testTarget, kColorPreAlpha, GL_RGBA4, GL_UNSIGNED_SHORT_4_4_4_4, false, true,
                 false, GLColor(98, 78, 59, 100));
        testCopy(testTarget, kColorUnAlpha, GL_RGBA4, GL_UNSIGNED_SHORT_4_4_4_4, false, false, true,
                 GLColor(221, 167, 110, 230));

        testCopy(testTarget, kColorNoAlpha, GL_SRGB8, GL_UNSIGNED_BYTE, false, false, false,
                 GLColor(244, 148, 78, 255));
        testCopy(testTarget, kColorPreAlpha, GL_SRGB8, GL_UNSIGNED_BYTE, false, true, false,
                 GLColor(31, 19, 11, 255));
        testCopy(testTarget, GLColor(100, 150, 200, 210), GL_SRGB8, GL_UNSIGNED_BYTE, false, false,
                 true, GLColor(49, 120, 228, 255));

        testCopy(testTarget, kColorNoAlpha, GL_SRGB8_ALPHA8, GL_UNSIGNED_BYTE, false, false, false,
                 GLColor(244, 148, 78, 100));
        testCopy(testTarget, kColorPreAlpha, GL_SRGB8_ALPHA8, GL_UNSIGNED_BYTE, false, true, false,
                 GLColor(31, 19, 11, 100));
        testCopy(testTarget, GLColor(100, 150, 200, 210), GL_SRGB8_ALPHA8, GL_UNSIGNED_BYTE, false,
                 false, true, GLColor(49, 120, 228, 210));

        testCopy(testTarget, GLColor(250, 200, 150, 200), GL_RGB5_A1, GL_UNSIGNED_BYTE, false,
                 false, false, GLColor(247, 198, 148, 255));
        testCopy(testTarget, kColorPreAlpha, GL_RGB5_A1, GL_UNSIGNED_BYTE, false, true, false,
                 GLColor(99, 82, 57, 0));
        testCopy(testTarget, kColorUnAlpha, GL_RGB5_A1, GL_UNSIGNED_BYTE, false, false, true,
                 GLColor(221, 167, 110, 255));

        if (IsGLExtensionEnabled("GL_ANGLE_rgbx_internal_format"))
        {
            testCopy(testTarget, kColorNoAlpha, GL_RGBX8_ANGLE, GL_UNSIGNED_BYTE, false, false,
                     false, GLColor(250, 200, 150, 255));
            testCopy(testTarget, kColorPreAlpha, GL_RGBX8_ANGLE, GL_UNSIGNED_BYTE, false, true,
                     false, GLColor(98, 78, 59, 255));
            testCopy(testTarget, kColorUnAlpha, GL_RGBX8_ANGLE, GL_UNSIGNED_BYTE, false, false,
                     true, GLColor(221, 167, 110, 255));
        }
    }

    {
        const GLColor kColorNoAlpha(250, 200, 150, 200);
        const GLColor kColorPreAlpha(250, 200, 150, 200);
        const GLColor kColorUnAlpha(101, 150, 190, 200);

        testCopy(testTarget, kColorNoAlpha, GL_RGB5_A1, GL_UNSIGNED_INT_2_10_10_10_REV, false,
                 false, false, GLColor(247, 198, 148, 255));
        testCopy(testTarget, kColorPreAlpha, GL_RGB5_A1, GL_UNSIGNED_INT_2_10_10_10_REV, false,
                 true, false, GLColor(198, 156, 115, 255));
        testCopy(testTarget, kColorUnAlpha, GL_RGB5_A1, GL_UNSIGNED_INT_2_10_10_10_REV, false,
                 false, true, GLColor(132, 189, 242, 255));

        testCopy(testTarget, kColorNoAlpha, GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1, false, false,
                 false, GLColor(247, 198, 148, 255));
        testCopy(testTarget, kColorPreAlpha, GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1, false, true,
                 false, GLColor(198, 156, 115, 255));
        testCopy(testTarget, kColorUnAlpha, GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1, false, false,
                 true, GLColor(132, 189, 242, 255));

        testCopy(testTarget, kColorNoAlpha, GL_RGB565, GL_UNSIGNED_BYTE, false, false, false,
                 GLColor(247, 199, 148, 255));
        testCopy(testTarget, kColorPreAlpha, GL_RGB565, GL_UNSIGNED_BYTE, false, true, false,
                 GLColor(198, 158, 115, 255));
        testCopy(testTarget, kColorUnAlpha, GL_RGB565, GL_UNSIGNED_BYTE, false, false, true,
                 GLColor(132, 190, 242, 255));

        testCopy(testTarget, kColorNoAlpha, GL_RGB565, GL_UNSIGNED_SHORT_5_6_5, false, false, false,
                 GLColor(247, 199, 148, 255));
        testCopy(testTarget, kColorPreAlpha, GL_RGB565, GL_UNSIGNED_SHORT_5_6_5, false, true, false,
                 GLColor(198, 158, 115, 255));
        testCopy(testTarget, kColorUnAlpha, GL_RGB565, GL_UNSIGNED_SHORT_5_6_5, false, false, true,
                 GLColor(132, 190, 242, 255));
    }
}

// Test passthrough, premultiply alpha, and unmultiply alpha copies for GL_TEXTURE_3D with unsigned
// byte formats.
TEST_P(Texture3DCopy, UnsignedByteFormats)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    testUnsignedByteFormats(GL_TEXTURE_3D);
}

void CopyTexture3DTest::testFloatFormats(const GLenum testTarget)
{
    std::vector<GLenum> floatTypes = {GL_FLOAT, GL_HALF_FLOAT, GL_UNSIGNED_INT_10F_11F_11F_REV,
                                      GL_UNSIGNED_INT_5_9_9_9_REV};

    const GLColor kColor(210, 200, 150, 235);

    for (GLenum floatType : floatTypes)
    {
        if (floatType != GL_UNSIGNED_INT_5_9_9_9_REV &&
            floatType != GL_UNSIGNED_INT_10F_11F_11F_REV)
        {
            testCopy(testTarget, kColor, GL_R16F, floatType, false, false, false,
                     GLColor(210, 0, 0, 255));
            testCopy(testTarget, kColor, GL_R16F, floatType, false, true, false,
                     GLColor(191, 0, 0, 255));
            testCopy(testTarget, kColor, GL_R16F, floatType, false, false, true,
                     GLColor(227, 0, 0, 255));

            testCopy(testTarget, kColor, GL_RG16F, floatType, false, false, false,
                     GLColor(210, 200, 0, 255));
            testCopy(testTarget, kColor, GL_RG16F, floatType, false, true, false,
                     GLColor(191, 184, 0, 255));
            testCopy(testTarget, kColor, GL_RG16F, floatType, false, false, true,
                     GLColor(227, 217, 0, 255));

            testCopy(testTarget, kColor, GL_RGB16F, floatType, false, false, false,
                     GLColor(210, 200, 150, 255));
            testCopy(testTarget, kColor, GL_RGB16F, floatType, false, true, false,
                     GLColor(191, 184, 138, 255));
            testCopy(testTarget, kColor, GL_RGB16F, floatType, false, false, true,
                     GLColor(227, 217, 161, 255));

            testCopy(testTarget, kColor, GL_RGBA16F, floatType, false, false, false,
                     GLColor(210, 200, 150, 235));
            testCopy(testTarget, kColor, GL_RGBA16F, floatType, false, true, false,
                     GLColor(191, 184, 138, 235));
            testCopy(testTarget, kColor, GL_RGBA16F, floatType, false, false, true,
                     GLColor(227, 217, 161, 235));
        }

        if (floatType != GL_UNSIGNED_INT_5_9_9_9_REV)
        {
            testCopy(testTarget, kColor, GL_R11F_G11F_B10F, floatType, false, false, false,
                     GLColor(210, 200, 148, 255));
            testCopy(testTarget, kColor, GL_R11F_G11F_B10F, floatType, false, true, false,
                     GLColor(191, 184, 138, 255));
            testCopy(testTarget, kColor, GL_R11F_G11F_B10F, floatType, false, false, true,
                     GLColor(227, 217, 161, 255));
        }

        if (floatType != GL_UNSIGNED_INT_10F_11F_11F_REV)
        {
            testCopy(testTarget, kColor, GL_RGB9_E5, floatType, false, false, false,
                     GLColor(210, 200, 148, 255));
            testCopy(testTarget, kColor, GL_RGB9_E5, floatType, false, true, false,
                     GLColor(192, 184, 138, 255));
            testCopy(testTarget, kColor, GL_RGB9_E5, floatType, false, false, true,
                     GLColor(227, 217, 161, 255));
        }
    }

    testCopy(testTarget, kColor, GL_R32F, GL_FLOAT, false, false, false, GLColor(210, 0, 0, 255));
    testCopy(testTarget, kColor, GL_R32F, GL_FLOAT, false, true, false, GLColor(191, 0, 0, 255));
    testCopy(testTarget, kColor, GL_R32F, GL_FLOAT, false, false, true, GLColor(227, 0, 0, 255));

    testCopy(testTarget, kColor, GL_RG32F, GL_FLOAT, false, false, false,
             GLColor(210, 200, 0, 255));
    testCopy(testTarget, kColor, GL_RG32F, GL_FLOAT, false, true, false, GLColor(191, 184, 0, 255));
    testCopy(testTarget, kColor, GL_RG32F, GL_FLOAT, false, false, true, GLColor(227, 217, 0, 255));

    testCopy(testTarget, kColor, GL_RGB32F, GL_FLOAT, false, false, false,
             GLColor(210, 200, 150, 255));
    testCopy(testTarget, kColor, GL_RGB32F, GL_FLOAT, false, true, false,
             GLColor(191, 184, 138, 255));
    testCopy(testTarget, kColor, GL_RGB32F, GL_FLOAT, false, false, true,
             GLColor(227, 217, 161, 255));

    testCopy(testTarget, kColor, GL_RGBA32F, GL_FLOAT, false, false, false,
             GLColor(210, 200, 150, 235));
    testCopy(testTarget, kColor, GL_RGBA32F, GL_FLOAT, false, true, false,
             GLColor(191, 184, 138, 235));
    testCopy(testTarget, kColor, GL_RGBA32F, GL_FLOAT, false, false, true,
             GLColor(227, 217, 161, 235));
}

// Test passthrough, premultiply alpha, and unmultiply alpha copies for GL_TEXTURE_3D with float
// formats.
TEST_P(Texture3DCopy, FloatFormats)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    testFloatFormats(GL_TEXTURE_3D);
}

void CopyTexture3DTest::testIntFormats(const GLenum testTarget)
{
    const GLColor kColor(255, 140, 150, 230);

    // Pixels will be read as if the most significant bit is data, not the sign. The expected colors
    // reflect this.
    testCopy(testTarget, kColor, GL_R8I, GL_BYTE, false, false, false, GLColor(127, 0, 0, 1));
    testCopy(testTarget, kColor, GL_R8I, GL_BYTE, false, true, false, GLColor(115, 0, 0, 1));
    testCopy(testTarget, kColor, GL_R8I, GL_BYTE, false, false, true, GLColor(127, 0, 0, 1));

    testCopy(testTarget, kColor, GL_R16I, GL_SHORT, false, false, false, GLColor(127, 0, 0, 1));
    testCopy(testTarget, kColor, GL_R16I, GL_SHORT, false, true, false, GLColor(115, 0, 0, 1));
    testCopy(testTarget, kColor, GL_R16I, GL_SHORT, false, false, true, GLColor(127, 0, 0, 1));

    testCopy(testTarget, kColor, GL_R32I, GL_INT, false, false, false, GLColor(127, 0, 0, 1));
    testCopy(testTarget, kColor, GL_R32I, GL_INT, false, true, false, GLColor(115, 0, 0, 1));
    testCopy(testTarget, kColor, GL_R32I, GL_INT, false, false, true, GLColor(127, 0, 0, 1));

    testCopy(testTarget, kColor, GL_RG8I, GL_BYTE, false, false, false, GLColor(127, 70, 0, 1));
    testCopy(testTarget, kColor, GL_RG8I, GL_BYTE, false, true, false, GLColor(115, 63, 0, 1));
    testCopy(testTarget, kColor, GL_RG8I, GL_BYTE, false, false, true, GLColor(127, 77, 0, 1));

    testCopy(testTarget, kColor, GL_RG16I, GL_SHORT, false, false, false, GLColor(127, 70, 0, 1));
    testCopy(testTarget, kColor, GL_RG16I, GL_SHORT, false, true, false, GLColor(115, 63, 0, 1));
    testCopy(testTarget, kColor, GL_RG16I, GL_SHORT, false, false, true, GLColor(127, 77, 0, 1));

    testCopy(testTarget, kColor, GL_RG32I, GL_INT, false, false, false, GLColor(127, 70, 0, 1));
    testCopy(testTarget, kColor, GL_RG32I, GL_INT, false, true, false, GLColor(115, 63, 0, 1));
    testCopy(testTarget, kColor, GL_RG32I, GL_INT, false, false, true, GLColor(127, 77, 0, 1));

    testCopy(testTarget, kColor, GL_RGB8I, GL_BYTE, false, false, false, GLColor(127, 70, 75, 1));
    testCopy(testTarget, kColor, GL_RGB8I, GL_BYTE, false, true, false, GLColor(115, 63, 67, 1));
    testCopy(testTarget, kColor, GL_RGB8I, GL_BYTE, false, false, true, GLColor(127, 77, 83, 1));

    testCopy(testTarget, kColor, GL_RGB16I, GL_SHORT, false, false, false, GLColor(127, 70, 75, 1));
    testCopy(testTarget, kColor, GL_RGB16I, GL_SHORT, false, true, false, GLColor(115, 63, 67, 1));
    testCopy(testTarget, kColor, GL_RGB16I, GL_SHORT, false, false, true, GLColor(127, 77, 83, 1));

    testCopy(testTarget, kColor, GL_RGB32I, GL_INT, false, false, false, GLColor(127, 70, 75, 1));
    testCopy(testTarget, kColor, GL_RGB32I, GL_INT, false, true, false, GLColor(115, 63, 67, 1));
    testCopy(testTarget, kColor, GL_RGB32I, GL_INT, false, false, true, GLColor(127, 77, 83, 1));

    testCopy(testTarget, kColor, GL_RGBA8I, GL_BYTE, false, false, false,
             GLColor(127, 70, 75, 115));
    testCopy(testTarget, kColor, GL_RGBA8I, GL_BYTE, false, true, false, GLColor(115, 63, 67, 115));
    testCopy(testTarget, kColor, GL_RGBA8I, GL_BYTE, false, false, true, GLColor(127, 77, 83, 115));

    testCopy(testTarget, kColor, GL_RGBA16I, GL_SHORT, false, false, false,
             GLColor(127, 70, 75, 115));
    testCopy(testTarget, kColor, GL_RGBA16I, GL_SHORT, false, true, false,
             GLColor(115, 63, 67, 115));
    testCopy(testTarget, kColor, GL_RGBA16I, GL_SHORT, false, false, true,
             GLColor(127, 77, 83, 115));

    testCopy(testTarget, kColor, GL_RGBA32I, GL_INT, false, false, false,
             GLColor(127, 70, 75, 115));
    testCopy(testTarget, kColor, GL_RGBA32I, GL_INT, false, true, false, GLColor(115, 63, 67, 115));
    testCopy(testTarget, kColor, GL_RGBA32I, GL_INT, false, false, true, GLColor(127, 77, 83, 115));
}

// Test passthrough, premultiply alpha, and unmultiply alpha copies for GL_TEXTURE_3D with integer
// formats.
TEST_P(Texture3DCopy, IntFormats)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    // Vulkan multiplies source by 255 unconditionally, which is wrong for signed integer formats.
    // http://anglebug.com/42263339
    ANGLE_SKIP_TEST_IF(IsVulkan());

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "uniform highp isampler3D tex3D;\n"
        "in vec3 texcoord;\n"
        "out ivec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = texture(tex3D, vec3(texcoord.x, texcoord.z, texcoord.y));\n"
        "}\n";

    mProgram = CompileProgram(getVertexShaderSource(), kFS);
    ASSERT_NE(0u, mProgram);
    ASSERT_GL_NO_ERROR();

    glUseProgram(mProgram);

    testIntFormats(GL_TEXTURE_3D);
}

void CopyTexture3DTest::testUintFormats(const GLenum testTarget)
{
    const GLColor kColor(128, 84, 32, 100);

    testCopy(testTarget, kColor, GL_R8UI, GL_UNSIGNED_BYTE, false, false, false,
             GLColor(128, 0, 0, 1));
    testCopy(testTarget, kColor, GL_R8UI, GL_UNSIGNED_BYTE, false, true, false,
             GLColor(50, 0, 0, 1));
    testCopy(testTarget, kColor, GL_R8UI, GL_UNSIGNED_BYTE, false, false, true,
             GLColor(255, 0, 0, 1));

    testCopy(testTarget, kColor, GL_R16UI, GL_UNSIGNED_SHORT, false, false, false,
             GLColor(128, 0, 0, 1));
    testCopy(testTarget, kColor, GL_R16UI, GL_UNSIGNED_SHORT, false, true, false,
             GLColor(50, 0, 0, 1));
    testCopy(testTarget, kColor, GL_R16UI, GL_UNSIGNED_SHORT, false, false, true,
             GLColor(255, 0, 0, 1));

    testCopy(testTarget, kColor, GL_R32UI, GL_UNSIGNED_INT, false, false, false,
             GLColor(128, 0, 0, 1));
    testCopy(testTarget, kColor, GL_R32UI, GL_UNSIGNED_INT, false, true, false,
             GLColor(50, 0, 0, 1));
    testCopy(testTarget, kColor, GL_R32UI, GL_UNSIGNED_INT, false, false, true,
             GLColor(255, 0, 0, 1));

    testCopy(testTarget, kColor, GL_RG8UI, GL_UNSIGNED_BYTE, false, false, false,
             GLColor(128, 84, 0, 1));
    testCopy(testTarget, kColor, GL_RG8UI, GL_UNSIGNED_BYTE, false, true, false,
             GLColor(50, 32, 0, 1));
    testCopy(testTarget, kColor, GL_RG8UI, GL_UNSIGNED_BYTE, false, false, true,
             GLColor(255, 214, 0, 1));

    testCopy(testTarget, kColor, GL_RG16UI, GL_UNSIGNED_SHORT, false, false, false,
             GLColor(128, 84, 0, 1));
    testCopy(testTarget, kColor, GL_RG16UI, GL_UNSIGNED_SHORT, false, true, false,
             GLColor(50, 32, 0, 1));
    testCopy(testTarget, kColor, GL_RG16UI, GL_UNSIGNED_SHORT, false, false, true,
             GLColor(255, 214, 0, 1));

    testCopy(testTarget, kColor, GL_RG32UI, GL_UNSIGNED_INT, false, false, false,
             GLColor(128, 84, 0, 1));
    testCopy(testTarget, kColor, GL_RG32UI, GL_UNSIGNED_INT, false, true, false,
             GLColor(50, 32, 0, 1));
    testCopy(testTarget, kColor, GL_RG32UI, GL_UNSIGNED_INT, false, false, true,
             GLColor(255, 214, 0, 1));

    testCopy(testTarget, kColor, GL_RGB8UI, GL_UNSIGNED_BYTE, false, false, false,
             GLColor(128, 84, 32, 1));
    testCopy(testTarget, kColor, GL_RGB8UI, GL_UNSIGNED_BYTE, false, true, false,
             GLColor(50, 32, 12, 1));
    testCopy(testTarget, kColor, GL_RGB8UI, GL_UNSIGNED_BYTE, false, false, true,
             GLColor(255, 214, 81, 1));

    testCopy(testTarget, kColor, GL_RGB16UI, GL_UNSIGNED_SHORT, false, false, false,
             GLColor(128, 84, 32, 1));
    testCopy(testTarget, kColor, GL_RGB16UI, GL_UNSIGNED_SHORT, false, true, false,
             GLColor(50, 32, 12, 1));
    testCopy(testTarget, kColor, GL_RGB16UI, GL_UNSIGNED_SHORT, false, false, true,
             GLColor(255, 214, 81, 1));

    testCopy(testTarget, kColor, GL_RGB32UI, GL_UNSIGNED_INT, false, false, false,
             GLColor(128, 84, 32, 1));
    testCopy(testTarget, kColor, GL_RGB32UI, GL_UNSIGNED_INT, false, true, false,
             GLColor(50, 32, 12, 1));
    testCopy(testTarget, kColor, GL_RGB32UI, GL_UNSIGNED_INT, false, false, true,
             GLColor(255, 214, 81, 1));

    testCopy(testTarget, kColor, GL_RGBA8UI, GL_UNSIGNED_BYTE, false, false, false,
             GLColor(128, 84, 32, 100));
    testCopy(testTarget, kColor, GL_RGBA8UI, GL_UNSIGNED_BYTE, false, true, false,
             GLColor(50, 32, 12, 100));
    testCopy(testTarget, kColor, GL_RGBA8UI, GL_UNSIGNED_BYTE, false, false, true,
             GLColor(255, 214, 81, 100));

    testCopy(testTarget, kColor, GL_RGBA16UI, GL_UNSIGNED_SHORT, false, false, false,
             GLColor(128, 84, 32, 100));
    testCopy(testTarget, kColor, GL_RGBA16UI, GL_UNSIGNED_SHORT, false, true, false,
             GLColor(50, 32, 12, 100));
    testCopy(testTarget, kColor, GL_RGBA16UI, GL_UNSIGNED_SHORT, false, false, true,
             GLColor(255, 214, 81, 100));

    testCopy(testTarget, kColor, GL_RGBA32UI, GL_UNSIGNED_INT, false, false, false,
             GLColor(128, 84, 32, 100));
    testCopy(testTarget, kColor, GL_RGBA32UI, GL_UNSIGNED_INT, false, true, false,
             GLColor(50, 32, 12, 100));
    testCopy(testTarget, kColor, GL_RGBA32UI, GL_UNSIGNED_INT, false, false, true,
             GLColor(255, 214, 81, 100));

    testCopy(testTarget, kColor, GL_RGB10_A2UI, GL_UNSIGNED_INT_2_10_10_10_REV, false, false, false,
             GLColor(128, 84, 32, 3));
    testCopy(testTarget, kColor, GL_RGB10_A2UI, GL_UNSIGNED_INT_2_10_10_10_REV, false, true, false,
             GLColor(50, 32, 12, 3));
    testCopy(testTarget, kColor, GL_RGB10_A2UI, GL_UNSIGNED_INT_2_10_10_10_REV, false, false, true,
             GLColor(255, 214, 81, 3));
}

// Test passthrough, premultiply alpha, and unmultiply alpha copies for GL_TEXTURE_3D with unsigned
// integer formats.
TEST_P(Texture3DCopy, UintFormats)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    // Vulkan multiplies source by 255 unconditionally, which is wrong for non-8-bit integer
    // formats.  http://anglebug.com/42263339
    ANGLE_SKIP_TEST_IF(IsVulkan());

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "uniform highp usampler3D tex3D;\n"
        "in vec3 texcoord;\n"
        "out uvec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = texture(tex3D, vec3(texcoord.x, texcoord.z, texcoord.y));\n"
        "}\n";

    mProgram = CompileProgram(getVertexShaderSource(), kFS);
    ASSERT_NE(0u, mProgram);
    ASSERT_GL_NO_ERROR();

    glUseProgram(mProgram);

    testUintFormats(GL_TEXTURE_3D);
}

// Test that glCopySubTexture3DANGLE correctly copies to and from a GL_TEXTURE_2D_ARRAY texture.
TEST_P(Texture2DArrayCopy, CopySubTexture)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    std::vector<GLColor> texDataGreen(2u * 2u * 2u, GLColor::green);
    std::vector<GLColor> texDataRed(2u * 2u * 2u, GLColor::red);

    glBindTexture(GL_TEXTURE_2D_ARRAY, sourceTexture);
    EXPECT_GL_NO_ERROR();
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texDataGreen.data());
    EXPECT_GL_NO_ERROR();
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
    EXPECT_GL_NO_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, destTexture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texDataRed.data());
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
    EXPECT_GL_NO_ERROR();
    glCopySubTexture3DANGLE(sourceTexture, 0, GL_TEXTURE_2D_ARRAY, destTexture, 0, 0, 0, 0, 0, 0, 0,
                            2, 2, 2, false, false, false);
    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that glCopyTexture3DANGLE correctly copies from a non-zero mipmap on a GL_TEXTURE_2D_ARRAY
// texture.
TEST_P(Texture2DArrayCopy, CopyFromMipmap)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    std::vector<GLColor> texDataGreen4(4u * 4u * 4u, GLColor::green);
    std::vector<GLColor> texDataGreen2(2u * 2u * 2u, GLColor::green);
    std::vector<GLColor> texDataRed2(2u * 2u * 2u, GLColor::red);

    glBindTexture(GL_TEXTURE_2D_ARRAY, sourceTexture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 4, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texDataGreen4.data());
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 texDataGreen2.data());
    EXPECT_GL_NO_ERROR();
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 1);
    EXPECT_GL_NO_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, destTexture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);

    EXPECT_GL_NO_ERROR();
    glCopyTexture3DANGLE(sourceTexture, 1, GL_TEXTURE_2D_ARRAY, destTexture, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, false, false, false);

    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, "position", 1.0f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that glCopySubTexture3DANGLE's offset and dimension parameters work correctly with a
// GL_TEXTURE_2D_ARRAY texture.
TEST_P(Texture2DArrayCopy, OffsetSubCopy)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    GLColor rgbaPixels[27];

    // Create pixel data for a 3x3x3 red cube
    for (int i = 0; i < 27; i++)
    {
        rgbaPixels[i] = GLColor(255u, 0u, 0u, 255u);
    }

    // Change a pixel to create a 1x1x1 blue cube at (0, 0, 0)
    rgbaPixels[0] = GLColor(0u, 0u, 255u, 255u);

    // Change some pixels to green to create a 2x2x2 cube starting at (1, 1, 1)
    rgbaPixels[13] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[14] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[16] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[17] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[22] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[23] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[25] = GLColor(0u, 255u, 0u, 255u);
    rgbaPixels[26] = GLColor(0u, 255u, 0u, 255u);

    glBindTexture(GL_TEXTURE_2D_ARRAY, sourceTexture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 3, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 rgbaPixels);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
    EXPECT_GL_NO_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, destTexture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    EXPECT_GL_NO_ERROR();
    // Copy the 2x2x2 green cube into a new texture
    glCopySubTexture3DANGLE(sourceTexture, 0, GL_TEXTURE_2D_ARRAY, destTexture, 0, 0, 0, 0, 1, 1, 1,
                            2, 2, 2, false, false, false);
    glBindTexture(GL_TEXTURE_2D_ARRAY, destTexture);
    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, "position", 1.0f);
    int width  = getWindowWidth() - 1;
    int height = getWindowHeight() - 1;

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(width, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, height, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(width, height, GLColor::green);

    // Copy the 1x1x1 blue cube into the the 2x2x2 green cube at location (1, 1, 1)
    glCopySubTexture3DANGLE(sourceTexture, 0, GL_TEXTURE_2D_ARRAY, destTexture, 0, 1, 1, 1, 0, 0, 0,
                            1, 1, 1, false, false, false);
    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, "position", 1.0f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(width, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, height, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(width, height, GLColor::blue);
}

// Test that the flipY parameter works with a GL_TEXTURE_2D_ARRAY texture.
TEST_P(Texture2DArrayCopy, FlipY)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    // Create a 2x2x2 cube. The top half is red. The bottom half is green.
    GLColor rgbaPixels[8] = {GLColor::green, GLColor::green, GLColor::red, GLColor::red,
                             GLColor::green, GLColor::green, GLColor::red, GLColor::red};

    glBindTexture(GL_TEXTURE_2D_ARRAY, sourceTexture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 rgbaPixels);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    drawQuad(mProgram, "position", 1.0f);

    int width  = getWindowWidth() - 1;
    int height = getWindowHeight() - 1;

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(width, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, height, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(width, height, GLColor::red);

    EXPECT_GL_NO_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, destTexture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    // Flip the y coordinate. This will put the greem half on top, and the red half on the bottom.
    glCopyTexture3DANGLE(sourceTexture, 0, GL_TEXTURE_2D_ARRAY, destTexture, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, true, false, false);

    glBindTexture(GL_TEXTURE_2D_ARRAY, destTexture);
    EXPECT_GL_NO_ERROR();
    drawQuad(mProgram, "position", 1.0f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, height, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(width, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(width, height, GLColor::green);
}

// Test passthrough, premultiply alpha, and unmultiply alpha copies for GL_TEXTURE_2D_ARRAY with
// unsized formats.
TEST_P(Texture2DArrayCopy, UnsizedFormats)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    testUnsizedFormats(GL_TEXTURE_2D_ARRAY);
}

// Test passthrough, premultiply alpha, and unmultiply alpha copies for GL_TEXTURE_2D_ARRAY with
// snorm formats.
TEST_P(Texture2DArrayCopy, SnormFormats)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    testSnormFormats(GL_TEXTURE_2D_ARRAY);
}

// Test passthrough, premultiply alpha, and unmultiply alpha copies for GL_TEXTURE_2D_ARRAY with
// unsigned byte formats.
TEST_P(Texture2DArrayCopy, UnsignedByteFormats)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    // Flay on Windows D3D11. http://anglebug.com/40644660
    ANGLE_SKIP_TEST_IF(IsWindows() && IsD3D11());

    testUnsignedByteFormats(GL_TEXTURE_2D_ARRAY);
}

// Test passthrough, premultiply alpha, and unmultiply alpha copies for GL_TEXTURE_2D_ARRAY with
// float formats.
TEST_P(Texture2DArrayCopy, FloatFormats)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    testFloatFormats(GL_TEXTURE_2D_ARRAY);
}

// Test passthrough, premultiply alpha, and unmultiply alpha copies for GL_TEXTURE_2D_ARRAY with
// integer formats.
TEST_P(Texture2DArrayCopy, IntFormats)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    // Vulkan multiplies source by 255 unconditionally, which is wrong for signed integer formats.
    // http://anglebug.com/42263339
    ANGLE_SKIP_TEST_IF(IsVulkan());

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "uniform highp isampler2DArray tex2DArray;\n"
        "in vec3 texcoord;\n"
        "out ivec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = texture(tex2DArray, vec3(texcoord.x, texcoord.z, texcoord.y));\n"
        "}\n";

    mProgram = CompileProgram(getVertexShaderSource(), kFS);
    ASSERT_NE(0u, mProgram);
    ASSERT_GL_NO_ERROR();

    glUseProgram(mProgram);

    testIntFormats(GL_TEXTURE_2D_ARRAY);
}

// Test passthrough, premultiply alpha, and unmultiply alpha copies for GL_TEXTURE_2D_ARRAY with
// unsigned integer formats.
TEST_P(Texture2DArrayCopy, UintFormats)
{
    ANGLE_SKIP_TEST_IF(!checkExtensions());

    // Vulkan multiplies source by 255 unconditionally, which is wrong for non-8-bit integer
    // formats.  http://anglebug.com/42263339
    ANGLE_SKIP_TEST_IF(IsVulkan());

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "uniform highp usampler2DArray tex2DArray;\n"
        "in vec3 texcoord;\n"
        "out uvec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = texture(tex2DArray, vec3(texcoord.x, texcoord.z, texcoord.y));\n"
        "}\n";

    mProgram = CompileProgram(getVertexShaderSource(), kFS);
    ASSERT_NE(0u, mProgram);
    ASSERT_GL_NO_ERROR();

    glUseProgram(mProgram);

    testUintFormats(GL_TEXTURE_2D_ARRAY);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(Texture3DCopy);
ANGLE_INSTANTIATE_TEST_ES3(Texture3DCopy);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(Texture2DArrayCopy);
ANGLE_INSTANTIATE_TEST_ES3(Texture2DArrayCopy);

}  // namespace angle
