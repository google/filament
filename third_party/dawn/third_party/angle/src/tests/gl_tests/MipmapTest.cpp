//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

void TexImageCubeMapFaces(GLint level,
                          GLenum internalformat,
                          GLsizei width,
                          GLenum format,
                          GLenum type,
                          void *pixels)
{
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level, internalformat, width, width, 0, format,
                 type, pixels);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, level, internalformat, width, width, 0, format,
                 type, pixels);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, level, internalformat, width, width, 0, format,
                 type, pixels);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, level, internalformat, width, width, 0, format,
                 type, pixels);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, level, internalformat, width, width, 0, format,
                 type, pixels);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, level, internalformat, width, width, 0, format,
                 type, pixels);
}

class BaseMipmapTest : public ANGLETest<>
{
  protected:
    void clearAndDrawQuad(GLuint program, GLsizei viewportWidth, GLsizei viewportHeight)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, viewportWidth, viewportHeight);
        ASSERT_GL_NO_ERROR();

        drawQuad(program, "position", 0.0f);
    }
};

}  // namespace

class MipmapTest : public BaseMipmapTest
{
  protected:
    MipmapTest()
        : m2DProgram(0),
          mCubeProgram(0),
          mTexture2D(0),
          mTextureCube(0),
          m3DProgram(0),
          mLevelZeroBlueInitData(),
          mLevelZeroWhiteInitData(),
          mLevelOneGreenInitData(),
          mLevelTwoRedInitData(),
          mOffscreenFramebuffer(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void setUp2DProgram()
    {
        // Vertex Shader source
        constexpr char kVS[] = R"(attribute vec4 position;
varying vec2 vTexCoord;

void main()
{
    gl_Position = position;
    vTexCoord   = (position.xy * 0.5) + 0.5;
})";

        // Fragment Shader source
        constexpr char kFS[] = R"(precision mediump float;
uniform sampler2D uTexture;
varying vec2 vTexCoord;

void main()
{
    gl_FragColor = texture2D(uTexture, vTexCoord);
})";

        m2DProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, m2DProgram);
    }

    void setUpCubeProgram()
    {
        // A simple vertex shader for the texture cube
        constexpr char kVS[] = R"(attribute vec4 position;
varying vec4 vPosition;
void main()
{
    gl_Position = position;
    vPosition = position;
})";

        // A very simple fragment shader to sample from the negative-Y face of a texture cube.
        constexpr char kFS[] = R"(precision mediump float;
uniform samplerCube uTexture;
varying vec4 vPosition;

void main()
{
    gl_FragColor = textureCube(uTexture, vec3(vPosition.x, -1, vPosition.y));
})";

        mCubeProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mCubeProgram);
    }

    void setUp3DProgram()
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_3D"));

        // http://anglebug.com/42263501
        ANGLE_SKIP_TEST_IF((IsPixel2() || IsNexus5X()) && IsOpenGLES());

        // Vertex Shader source
        constexpr char kVS[] = R"(attribute vec4 position;
varying vec2 vTexCoord;

void main()
{
    gl_Position = position;
    vTexCoord   = (position.xy * 0.5) + 0.5;
})";

        constexpr char kFS[] = R"(#version 100
#extension GL_OES_texture_3D : enable
precision highp float;
uniform highp sampler3D tex;
uniform float slice;
uniform float lod;
varying vec2 vTexCoord;

void main()
{
    gl_FragColor = texture3DLod(tex, vec3(vTexCoord, slice), lod);
})";

        m3DProgram = CompileProgram(kVS, kFS);
        if (m3DProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTexture3DSliceUniformLocation = glGetUniformLocation(m3DProgram, "slice");
        ASSERT_NE(-1, mTexture3DSliceUniformLocation);

        mTexture3DLODUniformLocation = glGetUniformLocation(m3DProgram, "lod");
        ASSERT_NE(-1, mTexture3DLODUniformLocation);

        glUseProgram(m3DProgram);
        glUniform1f(mTexture3DLODUniformLocation, 0);
        glUseProgram(0);
        ASSERT_GL_NO_ERROR();
    }

    void testSetUp() override
    {
        // http://anglebug.com/42264262
        ANGLE_SKIP_TEST_IF(IsOzone());

        setUp2DProgram();

        setUpCubeProgram();

        setUp3DProgram();

        mLevelZeroBlueInitData =
            createRGBInitData(getWindowWidth(), getWindowHeight(), 0, 0, 255);  // Blue
        mLevelZeroWhiteInitData =
            createRGBInitData(getWindowWidth(), getWindowHeight(), 255, 255, 255);  // White
        mLevelOneGreenInitData =
            createRGBInitData((getWindowWidth() / 2), (getWindowHeight() / 2), 0, 255, 0);  // Green
        mLevelTwoRedInitData =
            createRGBInitData((getWindowWidth() / 4), (getWindowHeight() / 4), 255, 0, 0);  // Red

        glGenFramebuffers(1, &mOffscreenFramebuffer);
        glGenTextures(1, &mTexture2D);

        // Initialize the texture2D to be empty, and don't use mips.
        glBindTexture(GL_TEXTURE_2D, mTexture2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getWindowWidth(), getWindowHeight(), 0, GL_RGB,
                     GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        ASSERT_EQ(getWindowWidth(), getWindowHeight());

        // Create a non-mipped texture cube. Set the negative-Y face to be blue.
        // The other sides of the cube map have been set to white.
        glGenTextures(1, &mTextureCube);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureCube);
        TexImageCubeMapFaces(0, GL_RGB, getWindowWidth(), GL_RGB, GL_UNSIGNED_BYTE, nullptr);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, getWindowWidth(), getWindowWidth(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, mLevelZeroWhiteInitData.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, getWindowWidth(), getWindowWidth(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, mLevelZeroWhiteInitData.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, getWindowWidth(), getWindowWidth(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, mLevelZeroWhiteInitData.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, getWindowWidth(), getWindowWidth(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, mLevelZeroBlueInitData.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, getWindowWidth(), getWindowWidth(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, mLevelZeroWhiteInitData.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, getWindowWidth(), getWindowWidth(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, mLevelZeroWhiteInitData.data());

        // Complete the texture cube without mipmaps to start with.
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteProgram(m2DProgram);
        glDeleteProgram(mCubeProgram);
        glDeleteProgram(m3DProgram);
        glDeleteFramebuffers(1, &mOffscreenFramebuffer);
        glDeleteTextures(1, &mTexture2D);
        glDeleteTextures(1, &mTextureCube);
    }

    std::vector<GLubyte> createRGBInitData(GLint width, GLint height, GLint r, GLint g, GLint b)
    {
        std::vector<GLubyte> data(3 * width * height);

        for (int i = 0; i < width * height; i += 1)
        {
            data[3 * i + 0] = static_cast<GLubyte>(r);
            data[3 * i + 1] = static_cast<GLubyte>(g);
            data[3 * i + 2] = static_cast<GLubyte>(b);
        }

        return data;
    }

    void clearTextureLevel0(GLenum textarget,
                            GLuint texture,
                            GLfloat red,
                            GLfloat green,
                            GLfloat blue,
                            GLfloat alpha)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mOffscreenFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textarget, texture, 0);
        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        glClearColor(red, green, blue, alpha);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLuint m2DProgram;
    GLuint mCubeProgram;
    GLuint mTexture2D;
    GLuint mTextureCube;

    GLuint m3DProgram = 0;
    GLint mTexture3DSliceUniformLocation;
    GLint mTexture3DLODUniformLocation;

    std::vector<GLubyte> mLevelZeroBlueInitData;
    std::vector<GLubyte> mLevelZeroWhiteInitData;
    std::vector<GLubyte> mLevelOneGreenInitData;
    std::vector<GLubyte> mLevelTwoRedInitData;

  private:
    GLuint mOffscreenFramebuffer;
};

class MipmapTestES3 : public BaseMipmapTest
{
  protected:
    MipmapTestES3()
        : mTexture(0),
          mArrayProgram(0),
          mTextureArraySliceUniformLocation(-1),
          m3DProgram(0),
          mTexture3DSliceUniformLocation(-1),
          mTexture3DLODUniformLocation(-1),
          m2DProgram(0)

    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    const char *vertexShaderSource()
    {
        // Don't put "#version ..." on its own line. See [cpp]p1:
        // "If there are sequences of preprocessing tokens within the list of arguments that
        //  would otherwise act as preprocessing directives, the behavior is undefined"
        return
            R"(#version 300 es
precision highp float;
in vec4 position;
out vec2 texcoord;

void main()
{
    gl_Position = vec4(position.xy, 0.0, 1.0);
    texcoord = (position.xy * 0.5) + 0.5;
})";
    }

    void setUpArrayProgram()
    {
        constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform highp sampler2DArray tex;
uniform int slice;
in vec2 texcoord;
out vec4 out_FragColor;

void main()
{
    out_FragColor = texture(tex, vec3(texcoord, float(slice)));
})";

        mArrayProgram = CompileProgram(vertexShaderSource(), kFS);
        if (mArrayProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureArraySliceUniformLocation = glGetUniformLocation(mArrayProgram, "slice");
        ASSERT_NE(-1, mTextureArraySliceUniformLocation);

        glUseProgram(mArrayProgram);
        glUseProgram(0);
        ASSERT_GL_NO_ERROR();
    }

    void setUp3DProgram()
    {
        constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform highp sampler3D tex;
uniform float slice;
uniform float lod;
in vec2 texcoord;
out vec4 out_FragColor;

void main()
{
    out_FragColor = textureLod(tex, vec3(texcoord, slice), lod);
})";

        m3DProgram = CompileProgram(vertexShaderSource(), kFS);
        if (m3DProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTexture3DSliceUniformLocation = glGetUniformLocation(m3DProgram, "slice");
        ASSERT_NE(-1, mTexture3DSliceUniformLocation);

        mTexture3DLODUniformLocation = glGetUniformLocation(m3DProgram, "lod");
        ASSERT_NE(-1, mTexture3DLODUniformLocation);

        glUseProgram(m3DProgram);
        glUniform1f(mTexture3DLODUniformLocation, 0);
        glUseProgram(0);
        ASSERT_GL_NO_ERROR();
    }

    void setUp2DProgram()
    {
        constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform highp sampler2D tex;
in vec2 texcoord;
out vec4 out_FragColor;

void main()
{
    out_FragColor = texture(tex, texcoord);
})";

        m2DProgram = CompileProgram(vertexShaderSource(), kFS);
        ASSERT_NE(0u, m2DProgram);

        ASSERT_GL_NO_ERROR();
    }

    void setUpCubeProgram()
    {
        // A very simple fragment shader to sample from the negative-Y face of a texture cube.
        constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform samplerCube uTexture;
in vec2 texcoord;
out vec4 out_FragColor;

void main()
{
    out_FragColor = texture(uTexture, vec3(texcoord.x, -1, texcoord.y));
})";

        mCubeProgram = CompileProgram(vertexShaderSource(), kFS);
        ASSERT_NE(0u, mCubeProgram);

        ASSERT_GL_NO_ERROR();
    }

    void testSetUp() override
    {
        glGenTextures(1, &mTexture);
        ASSERT_GL_NO_ERROR();

        setUpArrayProgram();
        setUp3DProgram();
        setUp2DProgram();
        setUpCubeProgram();
    }

    void testTearDown() override
    {
        glDeleteTextures(1, &mTexture);

        glDeleteProgram(mArrayProgram);
        glDeleteProgram(m3DProgram);
        glDeleteProgram(m2DProgram);
        glDeleteProgram(mCubeProgram);
    }

    void verifyAllMips(const uint32_t textureWidth,
                       const uint32_t textureHeight,
                       const GLColor &color)
    {
        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Texture2DLod(),
                         essl3_shaders::fs::Texture2DLod());
        glUseProgram(program);
        const GLint textureLoc = glGetUniformLocation(program, essl3_shaders::Texture2DUniform());
        const GLint lodLoc     = glGetUniformLocation(program, essl3_shaders::LodUniform());
        ASSERT_NE(-1, textureLoc);
        ASSERT_NE(-1, lodLoc);
        glUniform1i(textureLoc, 0);

        // Verify that every mip is correct.
        const int w = getWindowWidth() - 1;
        const int h = getWindowHeight() - 1;
        for (uint32_t mip = 0; textureWidth >> mip >= 1 || textureHeight >> mip >= 1; ++mip)
        {
            glUniform1f(lodLoc, mip);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
            EXPECT_GL_NO_ERROR();
            EXPECT_PIXEL_COLOR_EQ(0, 0, color) << "Failed on mip " << mip;
            EXPECT_PIXEL_COLOR_EQ(w, 0, color) << "Failed on mip " << mip;
            EXPECT_PIXEL_COLOR_EQ(0, h, color) << "Failed on mip " << mip;
            EXPECT_PIXEL_COLOR_EQ(w, h, color) << "Failed on mip " << mip;
        }
    }

    GLuint mTexture;

    GLuint mArrayProgram;
    GLint mTextureArraySliceUniformLocation;

    GLuint m3DProgram;
    GLint mTexture3DSliceUniformLocation;
    GLint mTexture3DLODUniformLocation;

    GLuint m2DProgram;

    GLuint mCubeProgram;
};

class MipmapTestES31 : public BaseMipmapTest
{
  protected:
    MipmapTestES31()

    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Test generating mipmaps with base level and max level set. Ported from part of the
// conformance2/textures/misc/tex-mipmap-levels WebGL2 test.
TEST_P(MipmapTestES3, GenerateMipmapPartialLevels)
{
    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    const std::vector<GLColor> kRedData(64, GLColor::red);
    const std::vector<GLColor> kGreenData(16, GLColor::green);
    const std::vector<GLColor> kBlueData(4, GLColor::blue);

    // Initialize mips 2 to 4
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, kRedData.data());
    glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, kGreenData.data());
    glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, kBlueData.data());

    // Set base and max levels
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Verify the data
    clearAndDrawQuad(m2DProgram, 2, 2);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Test that generateMipmap works with partial levels.
    glGenerateMipmap(GL_TEXTURE_2D);
    clearAndDrawQuad(m2DProgram, 2, 2);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// This test generates mipmaps for a 1x1 texture, which should be a no-op.
TEST_P(MipmapTestES3, GenerateMipmap1x1Texture)
{
    constexpr uint32_t kTextureSize = 1;

    const std::vector<GLColor> kInitialColor(kTextureSize * kTextureSize,
                                             GLColor(35, 81, 184, 211));

    // Create the texture.
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, kInitialColor.data());

    // Then generate the mips.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Verify that every mip is correct.
    verifyAllMips(kTextureSize, kTextureSize, kInitialColor[0]);
}

// This test generates mipmaps for a large texture and ensures all mips are generated.
TEST_P(MipmapTestES3, GenerateMipmapLargeTexture)
{
    constexpr uint32_t kTextureSize = 4096;

    const std::vector<GLColor> kInitialColor(kTextureSize * kTextureSize,
                                             GLColor(35, 81, 184, 211));

    // Create the texture.
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, kInitialColor.data());

    // Then generate the mips.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Enable mipmaps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    // Verify that every mip is correct.
    verifyAllMips(kTextureSize, kTextureSize, kInitialColor[0]);
}

// This test generates mipmaps for a large npot texture and ensures all mips are generated.
TEST_P(MipmapTestES3, GenerateMipmapLargeNPOTTexture)
{
    constexpr uint32_t kTextureWidth  = 3840;
    constexpr uint32_t kTextureHeight = 2160;

    const std::vector<GLColor> kInitialColor(kTextureWidth * kTextureHeight,
                                             GLColor(35, 81, 184, 211));

    // Create the texture.
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureWidth, kTextureHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, kInitialColor.data());

    // Then generate the mips.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Enable mipmaps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    // Verify that every mip is correct.
    verifyAllMips(kTextureWidth, kTextureHeight, kInitialColor[0]);
}

// This test generates mipmaps for an elongated npot texture with the maximum number of mips and
// ensures all mips are generated.
TEST_P(MipmapTestES3, GenerateMipmapLongNPOTTexture)
{
    // Imprecisions in the result.  http://anglebug.com/42263409
    ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsOpenGL());

    GLint maxTextureWidth = 32767;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureWidth);

    constexpr uint32_t kTextureHeight = 43;
    const uint32_t kTextureWidth      = maxTextureWidth - 1;  // -1 to make the width NPOT

    const std::vector<GLColor> kInitialColor(kTextureWidth * kTextureHeight,
                                             GLColor(35, 81, 184, 211));

    // Create the texture.
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureWidth, kTextureHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, kInitialColor.data());

    // Then generate the mips.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Enable mipmaps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    // Verify that every mip is correct.
    verifyAllMips(kTextureWidth, kTextureHeight, kInitialColor[0]);
}

// This test generates (and uses) mipmaps on a texture using init data. D3D11 will use a
// non-renderable TextureStorage for this. The test then disables mips, renders to level zero of the
// texture, and reenables mips before using the texture again. To do this, D3D11 has to convert the
// TextureStorage into a renderable one. This test ensures that the conversion works correctly. In
// particular, on D3D11 Feature Level 9_3 it ensures that both the zero LOD workaround texture AND
// the 'normal' texture are copied during conversion.
TEST_P(MipmapTest, GenerateMipmapFromInitDataThenRender)
{
    // http://anglebug.com/42264262
    ANGLE_SKIP_TEST_IF(IsOzone());

    // Pass in initial data so the texture is blue.
    glBindTexture(GL_TEXTURE_2D, mTexture2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getWindowWidth(), getWindowHeight(), 0, GL_RGB,
                 GL_UNSIGNED_BYTE, mLevelZeroBlueInitData.data());

    // Then generate the mips.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Enable mipmaps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    // Now draw the texture to various different sized areas.
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue);

    // Use mip level 1
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);

    // Use mip level 2
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::blue);

    ASSERT_GL_NO_ERROR();

    // Disable mips. Render a quad using the texture and ensure it's blue.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue);

    // Clear level 0 of the texture to red.
    clearTextureLevel0(GL_TEXTURE_2D, mTexture2D, 1.0f, 0.0f, 0.0f, 1.0f);

    // Reenable mips, and try rendering different-sized quads.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    // Level 0 is now red, so this should render red.
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::red);

    // Use mip level 1, blue.
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);

    // Use mip level 2, blue.
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::blue);
}

// Test that generating mipmap after the image is already created for a single level works.
TEST_P(MipmapTest, GenerateMipmapAfterSingleLevelDraw)
{
    // http://anglebug.com/42264262
    ANGLE_SKIP_TEST_IF(IsOzone());

    uint32_t width  = getWindowWidth();
    uint32_t height = getWindowHeight();

    const std::vector<GLColor> kInitData(width * height, GLColor::blue);

    // Pass in initial data so the texture is blue.
    glBindTexture(GL_TEXTURE_2D, mTexture2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kInitData.data());

    // Make sure the texture image is created.
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, kInitData[0]);

    // Then generate the mips.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Enable mipmaps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    // Draw and make sure the second mip is blue.
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, kInitData[0]);
}

// Test that generating mipmaps, then modifying the base level and generating mipmaps again works.
TEST_P(MipmapTest, GenerateMipmapAfterModifyingBaseLevel)
{
    // http://anglebug.com/42264262
    ANGLE_SKIP_TEST_IF(IsOzone());

    uint32_t width  = getWindowWidth();
    uint32_t height = getWindowHeight();

    const std::vector<GLColor> kInitData(width * height, GLColor::blue);

    // Pass in initial data so the texture is blue.
    glBindTexture(GL_TEXTURE_2D, mTexture2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kInitData.data());

    // Then generate the mips.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Enable mipmaps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    // Draw and make sure the second mip is blue.  This is to make sure the texture image is
    // allocated.
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, kInitData[0]);

    // Modify mip 0 without redefining it.
    const std::vector<GLColor> kModifyData(width * height, GLColor::green);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
                    kModifyData.data());

    // Generate the mips again, which should update all levels to the new (green) color.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    clearAndDrawQuad(m2DProgram, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, kModifyData[0]);
}

// This test ensures that mips are correctly generated from a rendered image.
// In particular, on D3D11 Feature Level 9_3, the clear call will be performed on the zero-level
// texture, rather than the mipped one. The test ensures that the zero-level texture is correctly
// copied into the mipped texture before the mipmaps are generated.
TEST_P(MipmapTest, GenerateMipmapFromRenderedImage)
{
    // http://anglebug.com/42264262
    ANGLE_SKIP_TEST_IF(IsOzone());

    glBindTexture(GL_TEXTURE_2D, mTexture2D);
    // Clear the texture to blue.
    clearTextureLevel0(GL_TEXTURE_2D, mTexture2D, 0.0f, 0.0f, 1.0f, 1.0f);

    // Then generate the mips
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Enable mips.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    // Now draw the texture to various different sized areas.
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue);

    // Use mip level 1
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);

    // Use mip level 2
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::blue);
}

// Test to ensure that rendering to a mipmapped texture works, regardless of whether mipmaps are
// enabled or not.
// TODO: This test hits a texture rebind bug in the D3D11 renderer. Fix this.
TEST_P(MipmapTest, RenderOntoLevelZeroAfterGenerateMipmap)
{
    // TODO(geofflang): Figure out why this is broken on AMD OpenGL
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());
    // http://anglebug.com/42264262
    ANGLE_SKIP_TEST_IF(IsOzone());

    glBindTexture(GL_TEXTURE_2D, mTexture2D);

    // Clear the texture to blue.
    clearTextureLevel0(GL_TEXTURE_2D, mTexture2D, 0.0f, 0.0f, 1.0f, 1.0f);

    // Now, draw the texture to a quad that's the same size as the texture. This draws to the
    // default framebuffer. The quad should be blue.
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue);

    // Now go back to the texture, and generate mips on it.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Now try rendering the textured quad again. Note: we've not told GL to use the generated mips.
    // The quad should be blue.
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue);

    // Now tell GL to use the generated mips.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    EXPECT_GL_NO_ERROR();

    // Now render the textured quad again. It should be still be blue.
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue);

    // Now render the textured quad to an area smaller than the texture (i.e. to force
    // minification). This should be blue.
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::blue);

    // Now clear the texture to green. This just clears the top level. The lower mips should remain
    // blue.
    clearTextureLevel0(GL_TEXTURE_2D, mTexture2D, 0.0f, 1.0f, 0.0f, 1.0f);

    // Render a textured quad equal in size to the texture. This should be green, since we just
    // cleared level 0.
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::green);

    // Render a small textured quad. This forces minification, so should render blue (the color of
    // levels 1+).
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::blue);

    // Disable mipmaps again
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    ASSERT_GL_NO_ERROR();

    // Render a textured quad equal in size to the texture. This should be green, the color of level
    // 0 in the texture.
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::green);

    // Render a small textured quad. This would force minification if mips were enabled, but they're
    // not. Therefore, this should be green.
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::green);
}

// This test defines a valid mipchain manually, with an extra level that's unused on the first few
// draws. Later on, it redefines the whole mipchain but this time, uses the last mip that was
// already uploaded before. The test expects that mip to be usable.
TEST_P(MipmapTest, DefineValidExtraLevelAndUseItLater)
{
    // http://anglebug.com/42264262
    ANGLE_SKIP_TEST_IF(IsOzone());

    glBindTexture(GL_TEXTURE_2D, mTexture2D);

    GLubyte *levels[] = {mLevelZeroBlueInitData.data(), mLevelOneGreenInitData.data(),
                         mLevelTwoRedInitData.data()};

    int maxLevel = 1 + static_cast<int>(floor(log2(std::max(getWindowWidth(), getWindowHeight()))));

    for (int i = 0; i < maxLevel; i++)
    {
        glTexImage2D(GL_TEXTURE_2D, i, GL_RGB, getWindowWidth() >> i, getWindowHeight() >> i, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, levels[i % 3]);
    }

    // Define an extra level that won't be used for now
    std::vector<GLubyte> magentaExtraLevelData =
        createRGBInitData(getWindowWidth() * 2, getWindowHeight() * 2, 255, 0, 255);
    glTexImage2D(GL_TEXTURE_2D, maxLevel, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 magentaExtraLevelData.data());

    ASSERT_GL_NO_ERROR();

    // Enable mipmaps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    // Draw a full-sized quad using mip 0, and check it's blue.
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue);

    // Draw a full-sized quad using mip 1, and check it's green.
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::green);

    // Draw a full-sized quad using mip 2, and check it's red.
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::red);

    // Draw a full-sized quad using the last mip, and check it's green.
    clearAndDrawQuad(m2DProgram, 1, 1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Now redefine everything above level 8 to be a mipcomplete chain again.
    std::vector<GLubyte> levelDoubleSizeYellowInitData =
        createRGBInitData(getWindowWidth() * 2, getWindowHeight() * 2, 255, 255, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getWindowWidth() * 2, getWindowHeight() * 2, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, levelDoubleSizeYellowInitData.data());  // 256

    for (int i = 0; i < maxLevel - 1; i++)
    {
        glTexImage2D(GL_TEXTURE_2D, i + 1, GL_RGB, getWindowWidth() >> i, getWindowHeight() >> i, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, levels[i % 3]);
    }

    // At this point we have a valid mip chain, the last level being magenta if we draw 1x1 pixel.
    clearAndDrawQuad(m2DProgram, 1, 1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);

    // Draw a full-sized quad using mip 0, and check it's yellow.
    clearAndDrawQuad(m2DProgram, getWindowWidth() * 2, getWindowHeight() * 2);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::yellow);

    // Draw a full-sized quad using mip 1, and check it's blue.
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);

    // Draw a full-sized quad using mip 2, and check it's green.
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::green);
}

// Regression test for a bug that cause mipmaps to only generate using the top left corner as input.
TEST_P(MipmapTest, MipMapGenerationD3D9Bug)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_storage") ||
                       !IsGLExtensionEnabled("GL_OES_rgb8_rgba8") ||
                       !IsGLExtensionEnabled("GL_ANGLE_texture_usage"));

    // http://anglebug.com/42264262
    ANGLE_SKIP_TEST_IF(IsOzone());

    const GLColor mip0Color[4] = {
        GLColor::red,
        GLColor::green,
        GLColor::red,
        GLColor::green,
    };
    const GLColor mip1Color = GLColor(127, 127, 0, 255);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_USAGE_ANGLE, GL_FRAMEBUFFER_ATTACHMENT_ANGLE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexStorage2DEXT(GL_TEXTURE_2D, 2, GL_RGBA8_OES, 2, 2);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, mip0Color);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Only draw to a 1 pixel viewport so the lower mip is used
    clearAndDrawQuad(m2DProgram, 1, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, mip1Color, 1.0);
}

// This test ensures that the level-zero workaround for TextureCubes (on D3D11 Feature Level 9_3)
// works as expected. It tests enabling/disabling mipmaps, generating mipmaps, and rendering to
// level zero.
TEST_P(MipmapTest, TextureCubeGeneralLevelZero)
{
    // http://anglebug.com/42261821
    ANGLE_SKIP_TEST_IF(IsFuchsia() && IsIntel() && IsVulkan());
    // http://anglebug.com/42261524
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());
    // http://issuetracker.google.com/159666631
    ANGLE_SKIP_TEST_IF(isSwiftshader());
    // http://anglebug.com/42264262
    ANGLE_SKIP_TEST_IF(IsOzone());

    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureCube);

    // Draw. Since the negative-Y face's is blue, this should be blue.
    clearAndDrawQuad(mCubeProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Generate mipmaps, and render. This should be blue.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    clearAndDrawQuad(mCubeProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Draw using a smaller viewport (to force a lower LOD of the texture). This should still be
    // blue.
    clearAndDrawQuad(mCubeProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Now clear the negative-Y face of the cube to red.
    clearTextureLevel0(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, mTextureCube, 1.0f, 0.0f, 0.0f, 1.0f);

    // Draw using a full-size viewport. This should be red.
    clearAndDrawQuad(mCubeProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw using a quarter-size viewport, to force a lower LOD. This should be *BLUE*, since we
    // only cleared level zero of the negative-Y face to red, and left its mipmaps blue.
    clearAndDrawQuad(mCubeProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Disable mipmaps again, and draw a to a quarter-size viewport.
    // Since this should use level zero of the texture, this should be *RED*.
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    clearAndDrawQuad(mCubeProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// This test ensures that rendering to level-zero of a TextureCube works as expected.
TEST_P(MipmapTest, TextureCubeRenderToLevelZero)
{
    // http://anglebug.com/42261821
    ANGLE_SKIP_TEST_IF(IsFuchsia() && IsIntel() && IsVulkan());
    // http://anglebug.com/42261524
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsVulkan());
    // http://anglebug.com/42264262
    ANGLE_SKIP_TEST_IF(IsOzone());

    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureCube);

    // Draw. Since the negative-Y face's is blue, this should be blue.
    clearAndDrawQuad(mCubeProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Now clear the negative-Y face of the cube to red.
    clearTextureLevel0(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, mTextureCube, 1.0f, 0.0f, 0.0f, 1.0f);

    // Draw using a full-size viewport. This should be red.
    clearAndDrawQuad(mCubeProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw a to a quarter-size viewport. This should also be red.
    clearAndDrawQuad(mCubeProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Creates a mipmapped 3D texture with two layers, and calls ANGLE's GenerateMipmap.
// Then tests if the mipmaps are rendered correctly for all two layers.
// This is the same as MipmapTestES3.MipmapsForTexture3D but for GL_OES_texture_3D extension on
// GLES 2.0 instead.
TEST_P(MipmapTest, MipmapsForTexture3DOES)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_3D"));

    // http://anglebug.com/42263501
    ANGLE_SKIP_TEST_IF((IsPixel2() || IsNexus5X()) && IsOpenGLES());
    // http://anglebug.com/42264262
    ANGLE_SKIP_TEST_IF(IsOzone());

    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);

    glTexImage3DOES(GL_TEXTURE_3D, 0, GL_RGBA, 16, 16, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 1, GL_RGBA, 8, 8, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 2, GL_RGBA, 4, 4, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 3, GL_RGBA, 2, 2, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 4, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Fill the first layer with red
    std::vector<GLColor> pixelsRed(16 * 16, GLColor::red);
    glTexSubImage3DOES(GL_TEXTURE_3D, 0, 0, 0, 0, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                       pixelsRed.data());

    // Fill the second layer with green
    std::vector<GLColor> pixelsGreen(16 * 16, GLColor::green);
    glTexSubImage3DOES(GL_TEXTURE_3D, 0, 0, 0, 1, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                       pixelsGreen.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    EXPECT_GL_NO_ERROR();

    glGenerateMipmap(GL_TEXTURE_3D);

    EXPECT_GL_NO_ERROR();

    glUseProgram(m3DProgram);

    EXPECT_GL_NO_ERROR();

    // Mipmap level 0
    // Draw the first slice
    glUniform1f(mTexture3DLODUniformLocation, 0.);
    glUniform1f(mTexture3DSliceUniformLocation, 0.25f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Draw the second slice
    glUniform1f(mTexture3DSliceUniformLocation, 0.75f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::green);

    // Regenerate mipmap of same color texture
    glTexSubImage3DOES(GL_TEXTURE_3D, 0, 0, 0, 1, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                       pixelsRed.data());

    glGenerateMipmap(GL_TEXTURE_3D);

    EXPECT_GL_NO_ERROR();

    // Mipmap level 1 8*8*1
    glUniform1f(mTexture3DLODUniformLocation, 1.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Mipmap level 2 4*4*1
    glUniform1f(mTexture3DLODUniformLocation, 2.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Mipmap level 3 2*2*1
    glUniform1f(mTexture3DLODUniformLocation, 3.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Mipmap level 4 1*1*1
    glUniform1f(mTexture3DLODUniformLocation, 4.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);
}

// This test verifies 3D texture mipmap generation uses box filter on Metal back-end.
class Mipmap3DBoxFilterTest : public MipmapTest
{};

TEST_P(Mipmap3DBoxFilterTest, GenMipmapsForTexture3DOES)
{
    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);

    glTexImage3DOES(GL_TEXTURE_3D, 0, GL_RGBA, 32, 32, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 1, GL_RGBA, 16, 16, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 2, GL_RGBA, 8, 8, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 3, GL_RGBA, 4, 4, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 4, GL_RGBA, 2, 2, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 5, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Fill the first layer with red
    std::vector<GLColor> pixelsRed(32 * 32, GLColor::red);
    glTexSubImage3DOES(GL_TEXTURE_3D, 0, 0, 0, 0, 32, 32, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                       pixelsRed.data());

    // Fill the second layer with green
    std::vector<GLColor> pixelsGreen(32 * 32, GLColor::green);
    glTexSubImage3DOES(GL_TEXTURE_3D, 0, 0, 0, 1, 32, 32, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                       pixelsGreen.data());

    // Fill the 3rd layer with blue
    std::vector<GLColor> pixelsBlue(32 * 32, GLColor::blue);
    glTexSubImage3DOES(GL_TEXTURE_3D, 0, 0, 0, 2, 32, 32, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                       pixelsBlue.data());

    // Fill the 4th layer with yellow
    std::vector<GLColor> pixelsYellow(32 * 32, GLColor::yellow);
    glTexSubImage3DOES(GL_TEXTURE_3D, 0, 0, 0, 3, 32, 32, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                       pixelsYellow.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    EXPECT_GL_NO_ERROR();

    glGenerateMipmap(GL_TEXTURE_3D);

    EXPECT_GL_NO_ERROR();

    glUseProgram(m3DProgram);

    EXPECT_GL_NO_ERROR();

    // Mipmap level 0
    // Draw the first slice
    glUniform1f(mTexture3DLODUniformLocation, 0.);
    glUniform1f(mTexture3DSliceUniformLocation, 0.125f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Draw the second slice
    glUniform1f(mTexture3DSliceUniformLocation, 0.375f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::green);

    // Draw the 3rd slice
    glUniform1f(mTexture3DSliceUniformLocation, 0.625f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::blue);

    // Draw the 4th slice
    glUniform1f(mTexture3DSliceUniformLocation, 0.875f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::yellow);

    // Mipmap level 1
    // The second mipmap should have two slice.
    glUniform1f(mTexture3DLODUniformLocation, 1.);
    glUniform1f(mTexture3DSliceUniformLocation, 0.25f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 0, 255, 1.0);

    glUniform1f(mTexture3DSliceUniformLocation, 0.75f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 127, 255, 1.0);

    // Mipmap level 2
    // The 3rd mipmap should only have one slice.
    glUniform1f(mTexture3DLODUniformLocation, 2.);
    glUniform1f(mTexture3DSliceUniformLocation, 0.25f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 64, 255, 1.0);

    glUniform1f(mTexture3DSliceUniformLocation, 0.75f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 64, 255, 1.0);

    // Mipmap level 3
    glUniform1f(mTexture3DLODUniformLocation, 3.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 64, 255, 1.0);

    // Mipmap level 4
    glUniform1f(mTexture3DLODUniformLocation, 4.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 64, 255, 1.0);

    // Mipmap level 5
    glUniform1f(mTexture3DLODUniformLocation, 5.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 64, 255, 1.0);
}

// Test that non-power of two texture also has mipmap generated using box filter
TEST_P(Mipmap3DBoxFilterTest, GenMipmapsForTexture3DOESNpot)
{
    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);

    glTexImage3DOES(GL_TEXTURE_3D, 0, GL_RGBA, 30, 30, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 1, GL_RGBA, 15, 15, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 2, GL_RGBA, 7, 7, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 3, GL_RGBA, 3, 3, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage3DOES(GL_TEXTURE_3D, 4, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Fill the first layer with red
    std::vector<GLColor> pixelsRed(30 * 30, GLColor::red);
    glTexSubImage3DOES(GL_TEXTURE_3D, 0, 0, 0, 0, 30, 30, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                       pixelsRed.data());

    // Fill the second layer with green
    std::vector<GLColor> pixelsGreen(30 * 30, GLColor::green);
    glTexSubImage3DOES(GL_TEXTURE_3D, 0, 0, 0, 1, 30, 30, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                       pixelsGreen.data());

    // Fill the 3rd layer with blue
    std::vector<GLColor> pixelsBlue(30 * 30, GLColor::blue);
    glTexSubImage3DOES(GL_TEXTURE_3D, 0, 0, 0, 2, 30, 30, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                       pixelsBlue.data());

    // Fill the 4th layer with yellow
    std::vector<GLColor> pixelsYellow(30 * 30, GLColor::yellow);
    glTexSubImage3DOES(GL_TEXTURE_3D, 0, 0, 0, 3, 30, 30, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                       pixelsYellow.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    EXPECT_GL_NO_ERROR();

    glGenerateMipmap(GL_TEXTURE_3D);

    EXPECT_GL_NO_ERROR();

    glUseProgram(m3DProgram);

    EXPECT_GL_NO_ERROR();

    // Mipmap level 0
    // Draw the first slice
    glUniform1f(mTexture3DLODUniformLocation, 0.);
    glUniform1f(mTexture3DSliceUniformLocation, 0.125f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Draw the second slice
    glUniform1f(mTexture3DSliceUniformLocation, 0.375f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::green);

    // Draw the 3rd slice
    glUniform1f(mTexture3DSliceUniformLocation, 0.625f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::blue);

    // Draw the 4th slice
    glUniform1f(mTexture3DSliceUniformLocation, 0.875f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::yellow);

    // Mipmap level 1
    // The second mipmap should have two slice.
    glUniform1f(mTexture3DLODUniformLocation, 1.);
    glUniform1f(mTexture3DSliceUniformLocation, 0.25f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 0, 255, 1.0);

    glUniform1f(mTexture3DSliceUniformLocation, 0.75f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 127, 255, 1.0);

    // Mipmap level 2
    // The 3rd mipmap should only have one slice.
    glUniform1f(mTexture3DLODUniformLocation, 2.);
    glUniform1f(mTexture3DSliceUniformLocation, 0.25f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 64, 255, 1.0);

    glUniform1f(mTexture3DSliceUniformLocation, 0.75f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 64, 255, 1.0);

    // Mipmap level 3
    glUniform1f(mTexture3DLODUniformLocation, 3.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 64, 255, 1.0);

    // Mipmap level 4
    glUniform1f(mTexture3DLODUniformLocation, 4.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 64, 255, 1.0);

    // Mipmap level 5
    glUniform1f(mTexture3DLODUniformLocation, 5.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(px, py, 127, 127, 64, 255, 1.0);
}

// Creates a mipmapped 2D array texture with three layers, and calls ANGLE's GenerateMipmap.
// Then tests if the mipmaps are rendered correctly for all three layers.
TEST_P(MipmapTestES3, MipmapsForTextureArray)
{
    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    glBindTexture(GL_TEXTURE_2D_ARRAY, mTexture);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 5, GL_RGBA8, 16, 16, 3);

    // Fill the first layer with red
    std::vector<GLColor> pixelsRed(16 * 16, GLColor::red);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    pixelsRed.data());

    // Fill the second layer with green
    std::vector<GLColor> pixelsGreen(16 * 16, GLColor::green);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    pixelsGreen.data());

    // Fill the third layer with blue
    std::vector<GLColor> pixelsBlue(16 * 16, GLColor::blue);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 2, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    pixelsBlue.data());

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    EXPECT_GL_NO_ERROR();

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    EXPECT_GL_NO_ERROR();

    glUseProgram(mArrayProgram);

    EXPECT_GL_NO_ERROR();

    // Draw the first slice
    glUniform1i(mTextureArraySliceUniformLocation, 0);
    drawQuad(mArrayProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Draw the second slice
    glUniform1i(mTextureArraySliceUniformLocation, 1);
    drawQuad(mArrayProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::green);

    // Draw the third slice
    glUniform1i(mTextureArraySliceUniformLocation, 2);
    drawQuad(mArrayProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::blue);
}

// Create a mipmapped 2D array texture with more layers than width / height, and call
// GenerateMipmap.
TEST_P(MipmapTestES3, MipmapForDeepTextureArray)
{
    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    glBindTexture(GL_TEXTURE_2D_ARRAY, mTexture);

    // Fill the whole texture with red.
    std::vector<GLColor> pixelsRed(2 * 2 * 4, GLColor::red);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 2, 2, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixelsRed.data());

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    EXPECT_GL_NO_ERROR();

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    EXPECT_GL_NO_ERROR();

    glUseProgram(mArrayProgram);

    EXPECT_GL_NO_ERROR();

    // Draw the first slice
    glUniform1i(mTextureArraySliceUniformLocation, 0);
    drawQuad(mArrayProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Draw the fourth slice
    glUniform1i(mTextureArraySliceUniformLocation, 3);
    drawQuad(mArrayProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);
}

// Creates a mipmapped 3D texture with two layers, and calls ANGLE's GenerateMipmap.
// Then tests if the mipmaps are rendered correctly for all two layers.
TEST_P(MipmapTestES3, MipmapsForTexture3D)
{
    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    glBindTexture(GL_TEXTURE_3D, mTexture);

    glTexStorage3D(GL_TEXTURE_3D, 5, GL_RGBA8, 16, 16, 2);

    // Fill the first layer with red
    std::vector<GLColor> pixelsRed(16 * 16, GLColor::red);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    pixelsRed.data());

    // Fill the second layer with green
    std::vector<GLColor> pixelsGreen(16 * 16, GLColor::green);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 1, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    pixelsGreen.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    EXPECT_GL_NO_ERROR();

    glGenerateMipmap(GL_TEXTURE_3D);

    EXPECT_GL_NO_ERROR();

    glUseProgram(m3DProgram);

    EXPECT_GL_NO_ERROR();

    // Mipmap level 0
    // Draw the first slice
    glUniform1f(mTexture3DLODUniformLocation, 0.);
    glUniform1f(mTexture3DSliceUniformLocation, 0.25f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Draw the second slice
    glUniform1f(mTexture3DSliceUniformLocation, 0.75f);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::green);

    // Regenerate mipmap of same color texture
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 1, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    pixelsRed.data());

    glGenerateMipmap(GL_TEXTURE_3D);

    EXPECT_GL_NO_ERROR();

    // Mipmap level 1 8*8*1
    glUniform1f(mTexture3DLODUniformLocation, 1.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Mipmap level 2 4*4*1
    glUniform1f(mTexture3DLODUniformLocation, 2.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Mipmap level 3 2*2*1
    glUniform1f(mTexture3DLODUniformLocation, 3.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Mipmap level 4 1*1*1
    glUniform1f(mTexture3DLODUniformLocation, 4.);
    drawQuad(m3DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);
}

// Create a 2D array, then immediately redefine it to have fewer layers.  Regression test for a bug
// in the Vulkan backend where the old higher-layer-count data upload was not removed.
TEST_P(MipmapTestES3, TextureArrayRedefineThenGenerateMipmap)
{
    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    glBindTexture(GL_TEXTURE_2D_ARRAY, mTexture);

    // Fill the whole texture with red, then redefine it and fill with green
    std::vector<GLColor> pixelsRed(2 * 2 * 4, GLColor::red);
    std::vector<GLColor> pixelsGreen(2 * 2 * 2, GLColor::green);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 2, 2, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixelsRed.data());
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixelsGreen.data());

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    EXPECT_GL_NO_ERROR();

    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    EXPECT_GL_NO_ERROR();

    glUseProgram(mArrayProgram);
    EXPECT_GL_NO_ERROR();

    // Draw the first slice
    glUniform1i(mTextureArraySliceUniformLocation, 0);
    drawQuad(mArrayProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::green);

    // Draw the second slice
    glUniform1i(mTextureArraySliceUniformLocation, 1);
    drawQuad(mArrayProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::green);
}

// Create a 2D array, use it, then redefine it to have fewer layers.  Regression test for a bug in
// the Vulkan backend where the old higher-layer-count data upload was not removed.
TEST_P(MipmapTestES3, TextureArrayUseThenRedefineThenGenerateMipmap)
{
    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    glBindTexture(GL_TEXTURE_2D_ARRAY, mTexture);

    // Fill the whole texture with red.
    std::vector<GLColor> pixelsRed(2 * 2 * 4, GLColor::red);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 2, 2, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixelsRed.data());

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    EXPECT_GL_NO_ERROR();

    // Generate mipmap
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    EXPECT_GL_NO_ERROR();

    glUseProgram(mArrayProgram);
    EXPECT_GL_NO_ERROR();

    // Draw the first slice
    glUniform1i(mTextureArraySliceUniformLocation, 0);
    drawQuad(mArrayProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Draw the fourth slice
    glUniform1i(mTextureArraySliceUniformLocation, 3);
    drawQuad(mArrayProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::red);

    // Redefine the image and fill with green
    std::vector<GLColor> pixelsGreen(2 * 2 * 2, GLColor::green);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixelsGreen.data());

    // Generate mipmap
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    EXPECT_GL_NO_ERROR();

    // Draw the first slice
    glUniform1i(mTextureArraySliceUniformLocation, 0);
    drawQuad(mArrayProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::green);

    // Draw the second slice
    glUniform1i(mTextureArraySliceUniformLocation, 1);
    drawQuad(mArrayProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(px, py, GLColor::green);
}

// Create a 2D texture with levels 0-2, call GenerateMipmap with base level 1 so that level 0 stays
// the same, and then sample levels 0 and 2.
// GLES 3.0.4 section 3.8.10:
// "Mipmap generation replaces texel array levels levelbase + 1 through q with arrays derived from
// the levelbase array, regardless of their previous contents. All other mipmap arrays, including
// the levelbase array, are left unchanged by this computation."
TEST_P(MipmapTestES3, GenerateMipmapBaseLevel)
{
    // Observed incorrect rendering on AMD, sampling level 2 returns black.
    ANGLE_SKIP_TEST_IF(IsAMD() && IsDesktopOpenGL());

    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    glBindTexture(GL_TEXTURE_2D, mTexture);

    ASSERT_EQ(getWindowWidth(), getWindowHeight());

    // Fill level 0 with blue
    std::vector<GLColor> pixelsBlue(getWindowWidth() * getWindowHeight(), GLColor::blue);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixelsBlue.data());

    // Fill level 1 with red
    std::vector<GLColor> pixelsRed(getWindowWidth() * getWindowHeight() / 4, GLColor::red);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth() / 2, getWindowHeight() / 2, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixelsRed.data());

    // Fill level 2 with green
    std::vector<GLColor> pixelsGreen(getWindowWidth() * getWindowHeight() / 16, GLColor::green);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA8, getWindowWidth() / 4, getWindowHeight() / 4, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixelsGreen.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);

    EXPECT_GL_NO_ERROR();

    // The blue level 0 should be untouched by this since base level is 1.
    glGenerateMipmap(GL_TEXTURE_2D);

    EXPECT_GL_NO_ERROR();

    // Draw using level 2. It should be set to red by GenerateMipmap.
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::red);

    // Draw using level 0. It should still be blue.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue);
}

// Test that generating mipmaps doesn't discard updates staged to out-of-range mips.
TEST_P(MipmapTestES3, GenerateMipmapPreservesOutOfRangeMips)
{
    // http://anglebug.com/42263372
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsWindows() && (IsAMD() || IsIntel()));

    // http://anglebug.com/42263374
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsLinux() && IsIntel());

    // http://anglebug.com/40096708
    ANGLE_SKIP_TEST_IF(IsOpenGLES() && IsNVIDIAShield());

    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    constexpr GLint kTextureSize = 16;
    const std::vector<GLColor> kLevel0Data(kTextureSize * kTextureSize, GLColor::red);
    const std::vector<GLColor> kLevel1Data(kTextureSize * kTextureSize, GLColor::green);
    const std::vector<GLColor> kLevel6Data(kTextureSize * kTextureSize, GLColor::blue);

    // Initialize a 16x16 RGBA8 texture with red, green and blue for levels 0, 1 and 6 respectively.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, kLevel0Data.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, kLevel1Data.data());
    glTexImage2D(GL_TEXTURE_2D, 6, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, kLevel6Data.data());
    ASSERT_GL_NO_ERROR();

    // Set base level to 1, and generate mipmaps.  Levels 1 through 5 will be green.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, kLevel1Data[0]);

    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Verify that the mips are all green.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    for (int mip = 0; mip < 5; ++mip)
    {
        int scale = 1 << mip;
        clearAndDrawQuad(m2DProgram, getWindowWidth() / scale, getWindowHeight() / scale);
        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / scale / 2, getWindowHeight() / scale / 2,
                              kLevel1Data[0]);
    }

    // Verify that level 0 is red.  TODO: setting MAX_LEVEL should be unnecessary, but is needed to
    // work around a bug in the Vulkan backend.  http://anglebug.com/40096706
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, kLevel0Data[0]);

    // Verify that level 6 is blue.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 6);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 6);

    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, kLevel6Data[0]);
}

// Create a cube map with levels 0-2, call GenerateMipmap with base level 1 so that level 0 stays
// the same, and then sample levels 0 and 2.
// GLES 3.0.4 section 3.8.10:
// "Mipmap generation replaces texel array levels levelbase + 1 through q with arrays derived from
// the levelbase array, regardless of their previous contents. All other mipmap arrays, including
// the levelbase array, are left unchanged by this computation."
TEST_P(MipmapTestES3, GenerateMipmapCubeBaseLevel)
{
    // Observed incorrect rendering on AMD, sampling level 2 returns black.
    ANGLE_SKIP_TEST_IF(IsAMD() && IsDesktopOpenGL());

    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    ASSERT_EQ(getWindowWidth(), getWindowHeight());

    glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);
    std::vector<GLColor> pixelsBlue(getWindowWidth() * getWindowWidth(), GLColor::blue);
    TexImageCubeMapFaces(0, GL_RGBA8, getWindowWidth(), GL_RGBA, GL_UNSIGNED_BYTE,
                         pixelsBlue.data());

    // Fill level 1 with red
    std::vector<GLColor> pixelsRed(getWindowWidth() * getWindowWidth() / 4, GLColor::red);
    TexImageCubeMapFaces(1, GL_RGBA8, getWindowWidth() / 2, GL_RGBA, GL_UNSIGNED_BYTE,
                         pixelsRed.data());

    // Fill level 2 with green
    std::vector<GLColor> pixelsGreen(getWindowWidth() * getWindowWidth() / 16, GLColor::green);
    TexImageCubeMapFaces(2, GL_RGBA8, getWindowWidth() / 4, GL_RGBA, GL_UNSIGNED_BYTE,
                         pixelsGreen.data());

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 1);

    EXPECT_GL_NO_ERROR();

    // The blue level 0 should be untouched by this since base level is 1.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    EXPECT_GL_NO_ERROR();

    // Draw using level 2. It should be set to red by GenerateMipmap.
    clearAndDrawQuad(mCubeProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::red);

    // Observed incorrect rendering on NVIDIA, level zero seems to be incorrectly affected by
    // GenerateMipmap.
    // http://anglebug.com/42262495
    ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsOpenGL());

    // Draw using level 0. It should still be blue.
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    clearAndDrawQuad(mCubeProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue);
}

// Create a texture with levels 0-2, call GenerateMipmap with max level 1 so that level 2 stays the
// same, and then sample levels 1 and 2.
// GLES 3.0.4 section 3.8.10:
// "Mipmap generation replaces texel array levels levelbase + 1 through q with arrays derived from
// the levelbase array, regardless of their previous contents. All other mipmap arrays, including
// the levelbase array, are left unchanged by this computation."
TEST_P(MipmapTestES3, GenerateMipmapMaxLevel)
{
    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    glBindTexture(GL_TEXTURE_2D, mTexture);

    // Fill level 0 with blue
    std::vector<GLColor> pixelsBlue(getWindowWidth() * getWindowHeight(), GLColor::blue);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixelsBlue.data());

    // Fill level 1 with red
    std::vector<GLColor> pixelsRed(getWindowWidth() * getWindowHeight() / 4, GLColor::red);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth() / 2, getWindowHeight() / 2, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixelsRed.data());

    // Fill level 2 with green
    std::vector<GLColor> pixelsGreen(getWindowWidth() * getWindowHeight() / 16, GLColor::green);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA8, getWindowWidth() / 4, getWindowHeight() / 4, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixelsGreen.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);

    EXPECT_GL_NO_ERROR();

    // The green level 2 should be untouched by this since max level is 1.
    glGenerateMipmap(GL_TEXTURE_2D);

    EXPECT_GL_NO_ERROR();

    // Draw using level 1. It should be set to blue by GenerateMipmap.
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);

    // Draw using level 2. It should still be green.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2);
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::green);
}

// Call GenerateMipmap with out-of-range base level. The spec is interpreted so that an out-of-range
// base level does not have a color-renderable/texture-filterable internal format, so the
// GenerateMipmap call generates INVALID_OPERATION. GLES 3.0.4 section 3.8.10:
// "If the levelbase array was not specified with an unsized internal format from table 3.3 or a
// sized internal format that is both color-renderable and texture-filterable according to table
// 3.13, an INVALID_OPERATION error is generated."
TEST_P(MipmapTestES3, GenerateMipmapBaseLevelOutOfRange)
{
    glBindTexture(GL_TEXTURE_2D, mTexture);

    // Fill level 0 with blue
    std::vector<GLColor> pixelsBlue(getWindowWidth() * getWindowHeight(), GLColor::blue);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixelsBlue.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1000);

    EXPECT_GL_NO_ERROR();

    // Expecting the out-of-range base level to be treated as not color-renderable and
    // texture-filterable.
    glGenerateMipmap(GL_TEXTURE_2D);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Draw using level 0. It should still be blue.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue);
}

// Call GenerateMipmap with out-of-range base level on an immutable texture. The base level should
// be clamped, so the call doesn't generate an error.
TEST_P(MipmapTestES3, GenerateMipmapBaseLevelOutOfRangeImmutableTexture)
{
    glBindTexture(GL_TEXTURE_2D, mTexture);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1000);

    EXPECT_GL_NO_ERROR();

    // This is essentially a no-op, since the texture only has one level.
    glGenerateMipmap(GL_TEXTURE_2D);

    EXPECT_GL_NO_ERROR();

    // The only level of the texture should still be green.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::green);
}

// A native version of the WebGL2 test tex-base-level-bug.html
TEST_P(MipmapTestES3, BaseLevelTextureBug)
{
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsAMD());

    // Regression in 10.12.4 needing workaround -- crbug.com/705865.
    // Seems to be passing on AMD GPUs. Definitely not NVIDIA.
    // Probably not Intel.
    ANGLE_SKIP_TEST_IF(IsMac() && IsNVIDIA());

    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    std::vector<GLColor> texDataRed(2u * 2u, GLColor::red);

    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texDataRed.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    drawQuad(m2DProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    drawQuad(m2DProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

TEST_P(MipmapTestES31, MipmapWithMemoryBarrier)
{
    std::vector<GLColor> pixelsRed(getWindowWidth() * getWindowHeight(), GLColor::red);
    std::vector<GLColor> pixelsGreen(getWindowWidth() * getWindowHeight() / 4, GLColor::green);

    constexpr char kVS[] = R"(#version 300 es
precision highp float;
in vec4 position;
out vec2 texcoord;

void main()
{
    gl_Position = vec4(position.xy, 0.0, 1.0);
    texcoord = (position.xy * 0.5) + 0.5;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform highp sampler2D tex;
in vec2 texcoord;
out vec4 out_FragColor;

void main()
{
    out_FragColor = texture(tex, texcoord);
})";

    ANGLE_GL_PROGRAM(m2DProgram, kVS, kFS);
    glUseProgram(m2DProgram);

    // Create a texture with red and enable the mipmap
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    // Fill level 0 with red
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixelsRed.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    EXPECT_GL_NO_ERROR();
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();
    // level 2 is red
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::red);

    // Clear the level 1 to green
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, getWindowWidth() / 2, getWindowHeight() / 2, GL_RGBA,
                    GL_UNSIGNED_BYTE, pixelsGreen.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    EXPECT_GL_NO_ERROR();
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();
    // Insert a memory barrier, then it will break the graph node submission order.
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    // level 0 is red
    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::red);
    // Draw using level 2. It should be set to green by GenerateMipmap.
    clearAndDrawQuad(m2DProgram, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 8, getWindowHeight() / 8, GLColor::green);
}

// Tests respecifying 3D mipmaps.
TEST_P(MipmapTestES3, Generate3DMipmapRespecification)
{
    std::vector<GLColor> pixels(256 * 256 * 100, GLColor::black);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 256, 256, 100, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixels.data());
    glTexImage3D(GL_TEXTURE_3D, 1, GL_RGBA, 128, 128, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 pixels.data());
    glGenerateMipmap(GL_TEXTURE_3D);

    ASSERT_GL_NO_ERROR();
}

// Test the calling glGenerateMipmap on a texture with a zero dimension doesn't crash.
TEST_P(MipmapTestES3, GenerateMipmapZeroSize)
{
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    // Create a texture with at least one dimension that's zero.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Attempt to generate mipmap.  This shouldn't crash.
    glGenerateMipmap(GL_TEXTURE_2D);
    EXPECT_GL_NO_ERROR();

    // Try the same with a 3D texture where depth is 0.
    GLTexture texture2;
    glBindTexture(GL_TEXTURE_3D, texture2);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, 2, 2, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glGenerateMipmap(GL_TEXTURE_3D);
}

// Test that reducing the size of the mipchain by resizing the base image then deleting it doesn't
// cause a crash. Issue found by fuzzer.
TEST_P(MipmapTestES3, ResizeBaseMipTo1x1ThenDelete)
{
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);

    std::vector<GLColor> data(2, GLColor::blue);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    data[0] = GLColor::green;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());

    tex.reset();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test the calling generateMipmap with redefining texture and modifying baselevel.
TEST_P(MipmapTestES3, GenerateMipmapWithRedefineLevelAndTexture)
{
    std::vector<GLColor> pixels(1000000, GLColor::black);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 2);
    glGenerateMipmap(GL_TEXTURE_2D);

    clearAndDrawQuad(m2DProgram, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::black);
}

// Test that manually generating mipmaps using draw calls is functional
TEST_P(MipmapTestES31, GenerateMipmapWithDraw)
{
    constexpr char kVS[] = R"(#version 310 es
precision highp float;

in vec4 position;
out vec2 texcoord;

void main()
{
    gl_Position = position;
    texcoord = (position.xy * 0.5) + 0.5;
})";

    constexpr char kFS[] = R"(#version 310 es
precision highp float;

uniform highp sampler2D tex;

in vec2 texcoord;
out vec4 frag_color;

void main()
{
    highp vec4 samples = textureGatherOffset(tex, texcoord, ivec2(0, 0), 0);
    highp float max_r = max(max(samples.x, samples.y), max(samples.z, samples.w));

    frag_color = vec4(max_r, 0.0, 0.0, 1.0);
}
)";
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    ASSERT_NE(0u, program);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, 4, 4);
    EXPECT_GL_NO_ERROR();

    // clang-format off
    constexpr GLubyte kRedColor[16] = {
        0x0c, 0x08, 0x4c, 0x48,
        0x00, 0x04, 0x40, 0x44,
        0xcc, 0xc8, 0x8c, 0x88,
        0xc0, 0xc4, 0x80, 0x84,
    };

    constexpr GLubyte kExpectedMip1Color[4] = {
        0x0c, 0x4c,
        0xcc, 0x8c,
    };

    constexpr GLubyte kExpectedMip2Color[1] = {
        0xcc
    };
    // clang-format on

    GLubyte mip0Color[16 * 4];
    for (size_t i = 0; i < 16; i++)
    {
        mip0Color[i * 4 + 0] = kRedColor[i];
        mip0Color[i * 4 + 1] = 0;
        mip0Color[i * 4 + 2] = 0;
        mip0Color[i * 4 + 3] = 0xff;
    }

    GLFramebuffer fb0, fb1, fb2;

    glBindFramebuffer(GL_FRAMEBUFFER, fb0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    EXPECT_GL_NO_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, fb1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 1);
    EXPECT_GL_NO_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, fb2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 2);
    EXPECT_GL_NO_ERROR();

    // initialize base mip
    glBindFramebuffer(GL_FRAMEBUFFER, fb0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_RGBA, GL_UNSIGNED_BYTE, &mip0Color[0]);
    EXPECT_GL_NO_ERROR();

    // draw mip 1 with mip 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    glViewport(0, 0, 2, 2);
    glBindFramebuffer(GL_FRAMEBUFFER, fb1);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    drawQuad(program, "position", 0.5);
    EXPECT_GL_NO_ERROR();

    // draw mip 2 with mip 1
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);

    glViewport(0, 0, 1, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, fb2);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    drawQuad(program, "position", 0.5);
    EXPECT_GL_NO_ERROR();

    // Read back rendered pixel values and compare
    GLubyte resultColors[16];
    glBindFramebuffer(GL_FRAMEBUFFER, fb1);
    glReadPixels(0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, &resultColors[0]);
    for (size_t i = 0; i < 4; i++)
    {
        EXPECT_EQ(resultColors[i * 4], kExpectedMip1Color[i]);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fb2);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &resultColors[0]);
    for (size_t i = 0; i < 1; i++)
    {
        EXPECT_EQ(resultColors[i * 4], kExpectedMip2Color[i]);
    }
}

// Test that manually generating lower mipmaps using draw calls is functional
TEST_P(MipmapTestES31, GenerateLowerMipsWithDraw)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, essl1_shaders::Texture2DUniform()), 0);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, 8, 8);
    EXPECT_GL_NO_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    const std::array<GLColor, 4> kMip2Color = {
        GLColor::red,
        GLColor::green,
        GLColor::blue,
        GLColor::white,
    };

    GLFramebuffer fb0, fb1, fb2;

    glBindFramebuffer(GL_FRAMEBUFFER, fb0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    EXPECT_GL_NO_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, fb1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 1);
    EXPECT_GL_NO_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, fb2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 2);
    EXPECT_GL_NO_ERROR();

    // initialize mip 2
    glBindFramebuffer(GL_FRAMEBUFFER, fb2);
    glTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, kMip2Color.data());
    EXPECT_GL_NO_ERROR();

    // draw mip 1 with mip 2
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2);

    glViewport(0, 0, 4, 4);
    glBindFramebuffer(GL_FRAMEBUFFER, fb1);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);
    EXPECT_GL_NO_ERROR();

    // draw mip 0 with mip 1
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);

    glViewport(0, 0, 8, 8);
    glBindFramebuffer(GL_FRAMEBUFFER, fb0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);
    EXPECT_GL_NO_ERROR();

    // Read back rendered pixel values and compare
    auto getCoeff = [](uint32_t x, uint32_t dimension) {
        const uint32_t dimDiv2 = dimension / 2;
        const uint32_t x2      = x > dimDiv2 ? (dimension - 1 - x) : x;

        const uint32_t denominator = dimension * dimension / 4;
        uint32_t numerator         = x2 * (x2 + 1) / 2;
        if (x > dimDiv2)
        {
            numerator = denominator - numerator;
        }
        return static_cast<float>(numerator) / static_cast<float>(denominator);
    };
    auto upscale = [&](uint32_t index, uint32_t dimension) {
        uint32_t x = index % dimension;
        uint32_t y = index / dimension;

        const float xCoeff = getCoeff(x, dimension);
        const float yCoeff = getCoeff(y, dimension);

        GLColor result;
        for (uint32_t channel = 0; channel < 4; ++channel)
        {
            const float mixX0 =
                kMip2Color[0][channel] * (1 - xCoeff) + kMip2Color[1][channel] * xCoeff;
            const float mixX1 =
                kMip2Color[2][channel] * (1 - xCoeff) + kMip2Color[3][channel] * xCoeff;
            const float mix = mixX0 * (1 - yCoeff) + mixX1 * yCoeff;

            result[channel] = static_cast<GLubyte>(round(mix));
        }
        return result;
    };

    glBindFramebuffer(GL_FRAMEBUFFER, fb0);
    for (uint32_t i = 0; i < 64; ++i)
    {
        const GLColor expect = upscale(i, 8);
        EXPECT_PIXEL_COLOR_NEAR(i % 8, i / 8, expect, 1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fb1);
    for (uint32_t i = 0; i < 16; ++i)
    {
        const GLColor expect = upscale(i, 4);
        EXPECT_PIXEL_COLOR_NEAR(i % 4, i / 4, expect, 1);
    }
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(MipmapTest);

namespace extraPlatforms
{
ANGLE_INSTANTIATE_TEST(MipmapTest,
                       ES2_METAL().disable(Feature::AllowGenMultipleMipsPerPass),
                       ES2_OPENGLES().enable(Feature::UseIntermediateTextureForGenerateMipmap));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(Mipmap3DBoxFilterTest);
ANGLE_INSTANTIATE_TEST(Mipmap3DBoxFilterTest,
                       ES2_METAL(),
                       ES2_METAL().disable(Feature::AllowGenMultipleMipsPerPass));
}  // namespace extraPlatforms

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MipmapTestES3);
ANGLE_INSTANTIATE_TEST_ES3(MipmapTestES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MipmapTestES31);
ANGLE_INSTANTIATE_TEST_ES31(MipmapTestES31);
