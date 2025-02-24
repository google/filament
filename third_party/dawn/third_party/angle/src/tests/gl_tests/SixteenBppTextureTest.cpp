//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SixteenBppTextureTest:
//   Basic tests using 16bpp texture formats (e.g. GL_RGB565).

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "image_util/imageformats.h"

using namespace angle;

namespace
{

GLColor Convert565(const R5G6B5 &rgb565)
{
    gl::ColorF colorf;
    R5G6B5::readColor(&colorf, &rgb565);
    Vector4 vecColor(colorf.red, colorf.green, colorf.blue, colorf.alpha);
    return GLColor(vecColor);
}

R5G6B5 Convert565(const GLColor &glColor)
{
    const Vector4 &vecColor = glColor.toNormalizedVector();
    gl::ColorF colorf(vecColor.x(), vecColor.y(), vecColor.z(), vecColor.w());
    R5G6B5 rgb565;
    R5G6B5::writeColor(&rgb565, &colorf);
    return rgb565;
}

class SixteenBppTextureTest : public ANGLETest<>
{
  protected:
    SixteenBppTextureTest()
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
    gl_Position = vec4(position.xy, 0.0, 1.0);
    texcoord = (position.xy * 0.5) + 0.5;
})";

        constexpr char kFS[] = R"(precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

void main()
{
    gl_FragColor = texture2D(tex, texcoord);
})";

        m2DProgram                = CompileProgram(kVS, kFS);
        mTexture2DUniformLocation = glGetUniformLocation(m2DProgram, "tex");
    }

    void testTearDown() override { glDeleteProgram(m2DProgram); }

    void simpleValidationBase(GLuint tex)
    {
        // Draw a quad using the texture
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(m2DProgram);
        glUniform1i(mTexture2DUniformLocation, 0);
        drawQuad(m2DProgram, "position", 0.5f);
        ASSERT_GL_NO_ERROR();

        int w = getWindowWidth() - 1;
        int h = getWindowHeight() - 1;

        // Check that it drew as expected
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(w, 0, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(0, h, GLColor::blue);
        EXPECT_PIXEL_COLOR_EQ(w, h, GLColor::yellow);

        // Generate mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);

        // Draw a quad using the texture
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(m2DProgram);
        glUniform1i(mTexture2DUniformLocation, 0);
        drawQuad(m2DProgram, "position", 0.5f);
        ASSERT_GL_NO_ERROR();

        // Check that it drew as expected
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(w, 0, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(0, h, GLColor::blue);
        EXPECT_PIXEL_COLOR_EQ(w, h, GLColor::yellow);

        // Bind the texture as a framebuffer, render to it, then check the results
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_UNSUPPORTED)
        {
            glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);
            EXPECT_PIXEL_EQ(1, 0, 255, 0, 0, 255);
            EXPECT_PIXEL_EQ(1, 1, 255, 0, 0, 255);
            EXPECT_PIXEL_EQ(0, 1, 255, 0, 0, 255);
        }
        else
        {
            std::cout << "Skipping rendering to an unsupported framebuffer format" << std::endl;
        }
    }

    GLuint m2DProgram;
    GLint mTexture2DUniformLocation;
};

class SixteenBppTextureTestES3 : public SixteenBppTextureTest
{};

// Simple validation test for GL_RGB565 textures.
// Samples from the texture, renders to it, generates mipmaps etc.
TEST_P(SixteenBppTextureTest, RGB565Validation)
{
    GLuint test;
    memcpy(&test, &GLColor::black, 4);

    R5G6B5 pixels[4] = {Convert565(GLColor::red), Convert565(GLColor::green),
                        Convert565(GLColor::blue), Convert565(GLColor::yellow)};

    glClearColor(0, 0, 0, 0);

    // Create a simple RGB565 texture
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    // Supply the data to it
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixels);
    EXPECT_GL_NO_ERROR();

    simpleValidationBase(tex);
}

// Simple validation test for GL_RGB5_A1 textures.
// Samples from the texture, renders to it, generates mipmaps etc.
TEST_P(SixteenBppTextureTest, RGBA5551Validation)
{
    GLushort pixels[4] = {
        0xF801,  // Red
        0x07C1,  // Green
        0x003F,  // Blue
        0xFFC1   // Red + Green
    };

    // Create a simple 5551 texture
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, pixels);
    EXPECT_GL_NO_ERROR();

    simpleValidationBase(tex);
}

// Test to ensure calling Clear() on an RGBA5551 texture does something reasonable
// Based on WebGL test conformance/textures/texture-attachment-formats.html
TEST_P(SixteenBppTextureTest, RGBA5551ClearAlpha)
{
    GLTexture tex;
    GLFramebuffer fbo;

    // Create a simple 5551 texture
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    // Bind the texture as a framebuffer, clear it, then check the results
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_UNSUPPORTED)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 0);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 255);
    }
    else
    {
        std::cout << "Skipping rendering to an unsupported framebuffer format" << std::endl;
    }
}

// Simple validation test for GL_RGBA4 textures.
// Samples from the texture, renders to it, generates mipmaps etc.
TEST_P(SixteenBppTextureTest, RGBA4444Validation)
{
    GLushort pixels[4] = {
        0xF00F,  // Red
        0x0F0F,  // Green
        0x00FF,  // Blue
        0xFF0F   // Red + Green
    };

    glClearColor(0, 0, 0, 0);

    // Generate a RGBA4444 texture, no mipmaps
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    // Provide some data for the texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, pixels);
    EXPECT_GL_NO_ERROR();

    simpleValidationBase(tex);
}

// Test uploading RGBA8 data to RGBA4 textures.
TEST_P(SixteenBppTextureTestES3, RGBA4UploadRGBA8)
{
    const std::array<GLColor, 4> kFourColors = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA4, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kFourColors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    simpleValidationBase(tex);
}

// Test uploading RGB8 data to RGB565 textures.
TEST_P(SixteenBppTextureTestES3, RGB565UploadRGB8)
{
    std::vector<GLColorRGB> fourColors;
    fourColors.push_back(GLColorRGB::red);
    fourColors.push_back(GLColorRGB::green);
    fourColors.push_back(GLColorRGB::blue);
    fourColors.push_back(GLColorRGB::yellow);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB565, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, fourColors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    simpleValidationBase(tex);
}

// Test uploading RGBA8 data to RGB5A41 textures.
TEST_P(SixteenBppTextureTestES3, RGB5A1UploadRGBA8)
{
    std::vector<GLColor> fourColors;
    fourColors.push_back(GLColor::red);
    fourColors.push_back(GLColor::green);
    fourColors.push_back(GLColor::blue);
    fourColors.push_back(GLColor::yellow);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 fourColors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    simpleValidationBase(tex);
}

// Test uploading RGB10A2 data to RGB5A1 textures.
TEST_P(SixteenBppTextureTestES3, RGB5A1UploadRGB10A2)
{
    struct RGB10A2
    {
        RGB10A2(uint32_t r, uint32_t g, uint32_t b, uint32_t a) : R(r), G(g), B(b), A(a) {}

        uint32_t R : 10;
        uint32_t G : 10;
        uint32_t B : 10;
        uint32_t A : 2;
    };

    uint32_t one10 = (1u << 10u) - 1u;

    RGB10A2 red(one10, 0u, 0u, 0x3u);
    RGB10A2 green(0u, one10, 0u, 0x3u);
    RGB10A2 blue(0u, 0u, one10, 0x3u);
    RGB10A2 yellow(one10, one10, 0u, 0x3u);

    std::vector<RGB10A2> fourColors;
    fourColors.push_back(red);
    fourColors.push_back(green);
    fourColors.push_back(blue);
    fourColors.push_back(yellow);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, 2, 2, 0, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV,
                 fourColors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    simpleValidationBase(tex);
}

// Test reading from RGBA4 textures attached to FBO.
TEST_P(SixteenBppTextureTestES3, RGBA4FramebufferReadback)
{
    // TODO(jmadill): Fix bug with GLES
    ANGLE_SKIP_TEST_IF(IsOpenGLES());

    Vector4 rawColor(0.5f, 0.7f, 1.0f, 0.0f);
    GLColor expectedColor(rawColor);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA4, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    glClearColor(rawColor.x(), rawColor.y(), rawColor.z(), rawColor.w());
    glClear(GL_COLOR_BUFFER_BIT);

    ASSERT_GL_NO_ERROR();

    GLenum colorReadFormat = GL_NONE;
    GLenum colorReadType   = GL_NONE;
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, reinterpret_cast<GLint *>(&colorReadFormat));
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, reinterpret_cast<GLint *>(&colorReadType));

    if (colorReadFormat == GL_RGBA && colorReadType == GL_UNSIGNED_BYTE)
    {
        GLColor actualColor = GLColor::black;
        glReadPixels(0, 0, 1, 1, colorReadFormat, colorReadType, &actualColor);
        EXPECT_COLOR_NEAR(expectedColor, actualColor, 20);
    }
    else
    {
        ASSERT_GLENUM_EQ(GL_RGBA, colorReadFormat);
        ASSERT_TRUE(colorReadType == GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT ||
                    colorReadType == GL_UNSIGNED_SHORT_4_4_4_4);

        uint16_t rgba = 0;
        glReadPixels(0, 0, 1, 1, colorReadFormat, colorReadType, &rgba);

        GLubyte r8 = static_cast<GLubyte>((rgba & 0xF000) >> 12);
        GLubyte g8 = static_cast<GLubyte>((rgba & 0x0F00) >> 8);
        GLubyte b8 = static_cast<GLubyte>((rgba & 0x00F0) >> 4);
        GLubyte a8 = static_cast<GLubyte>((rgba & 0x000F));

        GLColor actualColor(r8 << 4, g8 << 4, b8 << 4, a8 << 4);
        EXPECT_COLOR_NEAR(expectedColor, actualColor, 20);
    }

    ASSERT_GL_NO_ERROR();
}

// Test reading from RGB565 textures attached to FBO.
TEST_P(SixteenBppTextureTestES3, RGB565FramebufferReadback)
{
    // TODO(jmadill): Fix bug with GLES
    ANGLE_SKIP_TEST_IF(IsOpenGLES());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    std::vector<GLColor> fourColors;
    fourColors.push_back(GLColor::red);
    fourColors.push_back(GLColor::green);
    fourColors.push_back(GLColor::blue);
    fourColors.push_back(GLColor::white);

    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 color;\n"
        "in vec2 position;\n"
        "out vec4 fcolor;\n"
        "void main() {\n"
        "    fcolor = color;\n"
        "    gl_Position = vec4(position, 0.5, 1.0);\n"
        "}";
    constexpr char kFS[] =
        "#version 300 es\n"
        "in mediump vec4 fcolor;\n"
        "out mediump vec4 color;\n"
        "void main() {\n"
        "    color = fcolor;\n"
        "}";

    GLuint program = CompileProgram(kVS, kFS);
    glUseProgram(program);

    GLint colorLocation = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLocation);
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, fourColors.data());

    int w = getWindowWidth();
    int h = getWindowHeight();

    glViewport(0, 0, w, h);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB565, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    drawIndexedQuad(program, "position", 0.5f);

    ASSERT_GL_NO_ERROR();

    int t = 12;
    EXPECT_PIXEL_COLOR_NEAR(0, h - 1, GLColor::red, t);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::green, t);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, 0, GLColor::blue, t);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, h - 1, GLColor::white, t);

    GLenum colorReadFormat = GL_NONE;
    GLenum colorReadType   = GL_NONE;
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, reinterpret_cast<GLint *>(&colorReadFormat));
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, reinterpret_cast<GLint *>(&colorReadType));

    if (colorReadFormat == GL_RGB && colorReadType == GL_UNSIGNED_SHORT_5_6_5)
    {
        std::vector<R5G6B5> readColors(w * h);
        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, readColors.data());

        int hoffset = (h - 1) * w;
        EXPECT_COLOR_NEAR(GLColor::red, Convert565(readColors[hoffset]), t);
        EXPECT_COLOR_NEAR(GLColor::green, Convert565(readColors[0]), t);
        EXPECT_COLOR_NEAR(GLColor::blue, Convert565(readColors[w - 1]), t);
        EXPECT_COLOR_NEAR(GLColor::white, Convert565(readColors[w - 1 + hoffset]), t);
    }

    glDeleteProgram(program);
}

class SixteenBppTextureDitheringTestES3 : public SixteenBppTextureTestES3
{
  protected:
    SixteenBppTextureDitheringTestES3()
    {
        setWindowWidth(512);
        setWindowHeight(512);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    enum class Gradient
    {
        RedGreen,
        RedBlue,
        GreenBlue,
    };

    std::string makeVS(Gradient gradient)
    {
        std::ostringstream vs;
        vs << R"(#version 300 es
out mediump vec4 color;
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1   Black
    //      1         1   -1   Red
    //      2        -1    1   Green
    //      3         1    1   Yellow
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
    color = )";
        switch (gradient)
        {
            case Gradient::RedGreen:
                vs << "vec4(bit0, bit1, 0, 1)";
                break;
            case Gradient::RedBlue:
                vs << "vec4(bit1, 0, bit0, 1)";
                break;
            case Gradient::GreenBlue:
                vs << "vec4(0, bit0, bit1, 1)";
                break;
        }
        vs << R"(;
})";

        return vs.str();
    }

    const char *getFS()
    {
        return R"(#version 300 es
precision mediump float;
in vec4 color;
out vec4 colorOut;
void main()
{
    colorOut = color;
})";
    }

    GLColor getRightColor(Gradient gradient)
    {
        switch (gradient)
        {
            case Gradient::RedGreen:
                return GLColor::red;
            case Gradient::RedBlue:
                return GLColor::blue;
            case Gradient::GreenBlue:
                return GLColor::green;
            default:
                UNREACHABLE();
                return GLColor::red;
        }
    }

    GLColor getTopColor(Gradient gradient)
    {
        switch (gradient)
        {
            case Gradient::RedGreen:
                return GLColor::green;
            case Gradient::RedBlue:
                return GLColor::red;
            case Gradient::GreenBlue:
                return GLColor::blue;
            default:
                UNREACHABLE();
                return GLColor::red;
        }
    }

    void bandingTest(GLuint fbo, GLenum format, Gradient gradient, bool ditheringExpected);
    void bandingTestWithSwitch(GLenum format, Gradient gradient);
};

void SixteenBppTextureDitheringTestES3::bandingTest(GLuint fbo,
                                                    GLenum format,
                                                    Gradient gradient,
                                                    bool ditheringExpected)
{
    int w = getWindowWidth();
    int h = getWindowHeight();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, format, w, h);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    ANGLE_GL_PROGRAM(program, makeVS(gradient).c_str(), getFS());
    glUseProgram(program);

    glViewport(0, 0, w, h);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Draw a quad using the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glUseProgram(m2DProgram);
    glUniform1i(mTexture2DUniformLocation, 0);
    drawQuad(m2DProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    // Verify results.  Note that no dithering algorithm is specified by the spec, so the test
    // cannot actually verify that the output is dithered in any way.  These tests exist for visual
    // verification and debugging only.
    int maxError = 0;
    switch (format)
    {
        case GL_RGBA4:
            maxError = 256 / 16;
            break;
        case GL_RGB5_A1:
        case GL_RGB565:
            maxError = 256 / 32;
            break;
        default:
            UNREACHABLE();
    }

    const GLColor rightColor    = getRightColor(gradient);
    const GLColor topColor      = getTopColor(gradient);
    const GLColor topRightColor = GLColor(rightColor.R + topColor.R, rightColor.G + topColor.G,
                                          rightColor.B + topColor.B, 255);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::black, maxError);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, 0, rightColor, maxError);
    EXPECT_PIXEL_COLOR_NEAR(0, h - 1, topColor, maxError);
    EXPECT_PIXEL_COLOR_NEAR(w - 1, h - 1, topRightColor, maxError);
    ASSERT_GL_NO_ERROR();

    // Stricter pixel check on Android where dithering is supported by the driver or emulated.
    if (getEGLWindow()->isFeatureEnabled(Feature::EmulateDithering) ||
        getEGLWindow()->isFeatureEnabled(Feature::SupportsLegacyDithering))
    {
        uint32_t pixelCount = w * h;
        std::vector<uint32_t> pixelData(pixelCount);
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());

        int samePixelCount = 0;
        for (EGLint y = 0; y < h; ++y)
        {
            for (EGLint x = 0; x < w; ++x)
            {
                EGLint srcPixel = x + y * w;
                if (x < w - 1 && pixelData[srcPixel] == pixelData[srcPixel + 1])
                {
                    samePixelCount++;
                }
            }
        }

        double samePixelCountRatio = (1.0 * samePixelCount) / (w * h);
        // ~0.3 (dithering) vs 0.8+ (no dithering)
        if (ditheringExpected)
        {
            EXPECT_LT(samePixelCountRatio, 0.7);
        }
        else
        {
            EXPECT_GT(samePixelCountRatio, 0.7);
        }
    }
}

void SixteenBppTextureDitheringTestES3::bandingTestWithSwitch(GLenum format, Gradient gradient)
{
    GLFramebuffer fbo;
    GLFramebuffer anotherFbo;

    // GL_DITHER defaults to enabled
    bandingTest(fbo, format, gradient, true);

    glDisable(GL_DITHER);
    bandingTest(fbo, format, gradient, false);

    // Check that still disabled after switching to another framebuffer
    bandingTest(anotherFbo, format, gradient, false);

    glEnable(GL_DITHER);
    bandingTest(fbo, format, gradient, true);

    // Check that it is now enabled on another framebuffer
    bandingTest(anotherFbo, format, gradient, true);
}

// Test dithering applied to RGBA4.
TEST_P(SixteenBppTextureDitheringTestES3, RGBA4)
{
    bandingTestWithSwitch(GL_RGBA4, Gradient::RedGreen);
}

// Test dithering applied to RGBA5551.
TEST_P(SixteenBppTextureDitheringTestES3, RGBA5551)
{
    bandingTestWithSwitch(GL_RGB5_A1, Gradient::RedBlue);
}

// Test dithering applied to RGB565.
TEST_P(SixteenBppTextureDitheringTestES3, RGB565)
{
    bandingTestWithSwitch(GL_RGB565, Gradient::GreenBlue);
}

ANGLE_INSTANTIATE_TEST_ES2(SixteenBppTextureTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SixteenBppTextureTestES3);
ANGLE_INSTANTIATE_TEST_ES3(SixteenBppTextureTestES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SixteenBppTextureDitheringTestES3);
ANGLE_INSTANTIATE_TEST_ES3(SixteenBppTextureDitheringTestES3);

}  // namespace
