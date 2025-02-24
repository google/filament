//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureRectangleTest: Tests of GL_ANGLE_texture_rectangle

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class TextureRectangleTest : public ANGLETest<>
{
  protected:
    TextureRectangleTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    bool checkExtensionSupported() const
    {
        if (!IsGLExtensionEnabled("GL_ANGLE_texture_rectangle"))
        {
            std::cout << "Test skipped because GL_ANGLE_texture_rectangle is not available."
                      << std::endl;
            return false;
        }
        return true;
    }
};

class TextureRectangleTestES3 : public TextureRectangleTest
{};

class TextureRectangleTestES31 : public TextureRectangleTest
{};

// Test using TexImage2D to define a rectangle texture
TEST_P(TextureRectangleTest, TexImage2D)
{
    ANGLE_SKIP_TEST_IF(!checkExtensionSupported());

    // http://anglebug.com/42264188
    ANGLE_SKIP_TEST_IF(IsLinux() && IsNVIDIA() && IsOpenGL());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);

    // Defining level 0 is allowed
    glTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    ASSERT_GL_NO_ERROR();

    // Defining level other than 0 is not allowed
    glTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 1, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ANGLE, &maxSize);

    // Defining a texture of the max size is allowed
    {
        ScopedIgnorePlatformMessages ignore;

        glTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_RGBA, maxSize, maxSize, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        GLenum error = glGetError();
        ASSERT_TRUE(error == GL_NO_ERROR || error == GL_OUT_OF_MEMORY);
    }

    // Defining a texture of the max size is allowed
    glTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_RGBA, maxSize + 1, maxSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_RGBA, maxSize, maxSize + 1, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
}

// Test using CompressedTexImage2D cannot be used on a retangle texture
TEST_P(TextureRectangleTest, CompressedTexImage2DDisallowed)
{
    ANGLE_SKIP_TEST_IF(!checkExtensionSupported());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"));

    const char data[128] = {0};

    // Control case: 2D texture
    {
        GLTexture tex;
        glBindTexture(GL_TEXTURE_2D, tex);
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 16, 16, 0, 128,
                               data);
        ASSERT_GL_NO_ERROR();
    }

    // Rectangle textures cannot be compressed
    {
        GLTexture tex;
        glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);
        glCompressedTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 16,
                               16, 0, 128, data);
        ASSERT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Test using TexStorage2D to define a rectangle texture
TEST_P(TextureRectangleTest, TexStorage2D)
{
    ANGLE_SKIP_TEST_IF(!checkExtensionSupported());
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_EXT_texture_storage"));

    // http://anglebug.com/42264188
    ANGLE_SKIP_TEST_IF(IsLinux() && IsNVIDIA() && IsOpenGL());

    bool useES3       = getClientMajorVersion() >= 3;
    auto TexStorage2D = [useES3](GLenum target, GLint levels, GLenum format, GLint width,
                                 GLint height) {
        if (useES3)
        {
            glTexStorage2D(target, levels, format, width, height);
        }
        else
        {
            glTexStorage2DEXT(target, levels, format, width, height);
        }
    };

    // Defining one level is allowed
    {
        GLTexture tex;
        glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);
        TexStorage2D(GL_TEXTURE_RECTANGLE_ANGLE, 1, GL_RGBA8, 16, 16);
        ASSERT_GL_NO_ERROR();
    }

    // Having more than one level is not allowed
    {
        GLTexture tex;
        glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);
        // Use 5 levels because the EXT_texture_storage extension requires a mip chain all the way
        // to a 1x1 mip.
        TexStorage2D(GL_TEXTURE_RECTANGLE_ANGLE, 5, GL_RGBA8, 16, 16);
        ASSERT_GL_ERROR(GL_INVALID_VALUE);
    }

    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ANGLE, &maxSize);

    // Defining a texture of the max size is allowed but still allow for OOM
    {
        ScopedIgnorePlatformMessages ignore;

        GLTexture tex;
        glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);
        TexStorage2D(GL_TEXTURE_RECTANGLE_ANGLE, 1, GL_RGBA8, maxSize, maxSize);
        GLenum error = glGetError();
        ASSERT_TRUE(error == GL_NO_ERROR || error == GL_OUT_OF_MEMORY);
    }

    // Defining a texture of the max size is disallowed
    {
        GLTexture tex;
        glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);
        TexStorage2D(GL_TEXTURE_RECTANGLE_ANGLE, 1, GL_RGBA8, maxSize + 1, maxSize);
        ASSERT_GL_ERROR(GL_INVALID_VALUE);
        TexStorage2D(GL_TEXTURE_RECTANGLE_ANGLE, 1, GL_RGBA8, maxSize, maxSize + 1);
        ASSERT_GL_ERROR(GL_INVALID_VALUE);
    }

    // Compressed formats are disallowed
    if (IsGLExtensionEnabled("GL_EXT_texture_compression_dxt1"))
    {
        GLTexture tex;
        glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);
        TexStorage2D(GL_TEXTURE_RECTANGLE_ANGLE, 1, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 16, 16);
        ASSERT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Test validation of disallowed texture parameters
TEST_P(TextureRectangleTest, TexParameterRestriction)
{
    ANGLE_SKIP_TEST_IF(!checkExtensionSupported());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);

    // Only wrap mode CLAMP_TO_EDGE is supported
    // Wrap S
    glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    ASSERT_GL_NO_ERROR();
    glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);

    // Wrap T
    glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ASSERT_GL_NO_ERROR();
    glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);

    // Min filter has to be nearest or linear
    glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    ASSERT_GL_NO_ERROR();
    glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    ASSERT_GL_ERROR(GL_INVALID_ENUM);

    // Base level has to be 0
    if (getClientMajorVersion() >= 3)
    {
        glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_BASE_LEVEL, 0);
        ASSERT_GL_NO_ERROR();
        glTexParameteri(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_BASE_LEVEL, 1);
        ASSERT_GL_ERROR(GL_INVALID_OPERATION);
    }
}

// Test validation of 'level' in GetTexParameter
TEST_P(TextureRectangleTestES31, GetTexLevelParameter)
{
    ANGLE_SKIP_TEST_IF(!checkExtensionSupported());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    ASSERT_GL_NO_ERROR();

    GLint param;
    // Control case: level 0 is ok.
    glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_TEXTURE_INTERNAL_FORMAT, &param);
    ASSERT_GL_NO_ERROR();

    // Level 1 is not ok.
    glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_ANGLE, 1, GL_TEXTURE_INTERNAL_FORMAT, &param);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
}

// Test validation of "level" in FramebufferTexture2D
TEST_P(TextureRectangleTest, FramebufferTexture2DLevel)
{
    ANGLE_SKIP_TEST_IF(!checkExtensionSupported());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Using level 0 of a rectangle texture is valid.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ANGLE, tex,
                           0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT_GL_NO_ERROR();

    // Setting level != 0 is invalid
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ANGLE, tex,
                           1);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);
}

// Test sampling from a rectangle texture using texture2DRect in ESSL1
TEST_P(TextureRectangleTest, SamplingFromRectangleESSL1)
{
    ANGLE_SKIP_TEST_IF(!checkExtensionSupported());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 &GLColor::green);

    constexpr char kFS[] =
        "#extension GL_ARB_texture_rectangle : require\n"
        "precision mediump float;\n"
        "uniform sampler2DRect tex;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = texture2DRect(tex, vec2(0, 0));\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint location = glGetUniformLocation(program, "tex");
    ASSERT_NE(-1, location);
    glUniform1i(location, 0);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, false);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Test sampling from a rectangle texture using the texture overload in ESSL3
TEST_P(TextureRectangleTestES3, SamplingFromRectangleESSL3)
{
    ANGLE_SKIP_TEST_IF(!checkExtensionSupported());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 &GLColor::green);

    constexpr char kFS[] =
        "#version 300 es\n"
        "#extension GL_ARB_texture_rectangle : require\n"
        "precision mediump float;\n"
        "uniform sampler2DRect tex;\n"
        "out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = texture(tex, vec2(0, 0));\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint location = glGetUniformLocation(program, "tex");
    ASSERT_NE(-1, location);
    glUniform1i(location, 0);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, false);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Test attaching a rectangle texture and rendering to it.
TEST_P(TextureRectangleTest, RenderToRectangle)
{
    ANGLE_SKIP_TEST_IF(!checkExtensionSupported());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 &GLColor::black);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ANGLE, tex,
                           0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    ASSERT_GL_NO_ERROR();

    // Clearing a texture is just as good as checking we can render to it, right?
    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

TEST_P(TextureRectangleTest, DefaultSamplerParameters)
{
    ANGLE_SKIP_TEST_IF(!checkExtensionSupported());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);

    GLint minFilter = 0;
    glGetTexParameteriv(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_MIN_FILTER, &minFilter);
    EXPECT_EQ(GL_LINEAR, minFilter);

    GLint wrapS = 0;
    glGetTexParameteriv(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_WRAP_S, &wrapS);
    EXPECT_EQ(GL_CLAMP_TO_EDGE, wrapS);

    GLint wrapT = 0;
    glGetTexParameteriv(GL_TEXTURE_RECTANGLE_ANGLE, GL_TEXTURE_WRAP_T, &wrapT);
    EXPECT_EQ(GL_CLAMP_TO_EDGE, wrapT);
}

// Test glCopyTexImage with rectangle textures
TEST_P(TextureRectangleTestES3, CopyTexImage)
{
    ANGLE_SKIP_TEST_IF(!checkExtensionSupported());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Error case: level != 0
    glCopyTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 1, GL_RGBA8, 0, 0, 1, 1, 0);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    // level = 0 works and defines the texture.
    glCopyTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_RGBA8, 0, 0, 1, 1, 0);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ANGLE, tex,
                           0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Test glCopyTexSubImage with rectangle textures
TEST_P(TextureRectangleTestES3, CopyTexSubImage)
{
    ANGLE_SKIP_TEST_IF(!checkExtensionSupported());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_RECTANGLE_ANGLE, tex);
    glTexImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 &GLColor::black);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Error case: level != 0
    glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 1, 0, 0, 0, 0, 1, 1);
    ASSERT_GL_ERROR(GL_INVALID_VALUE);

    // level = 0 works and defines the texture.
    glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ANGLE, 0, 0, 0, 0, 0, 1, 1);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ANGLE, tex,
                           0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES2(TextureRectangleTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(TextureRectangleTestES3);
ANGLE_INSTANTIATE_TEST_ES3(TextureRectangleTestES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(TextureRectangleTestES31);
ANGLE_INSTANTIATE_TEST_ES31(TextureRectangleTestES31);
}  // anonymous namespace
