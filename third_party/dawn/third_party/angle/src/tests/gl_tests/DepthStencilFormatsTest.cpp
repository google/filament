//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "common/mathutil.h"
#include "platform/autogen/FeaturesD3D_autogen.h"

using namespace angle;

struct ReadbackTestParam
{
    GLuint attachment;
    GLuint format;
    GLuint type;
    void *data;
    int depthBits;
    int stencilBits;
};

class DepthStencilFormatsTestBase : public ANGLETest<>
{
  protected:
    DepthStencilFormatsTestBase()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    bool checkTexImageFormatSupport(GLenum format, GLenum type)
    {
        EXPECT_GL_NO_ERROR();

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, format, 1, 1, 0, format, type, nullptr);
        glDeleteTextures(1, &tex);

        return (glGetError() == GL_NO_ERROR);
    }

    bool checkTexStorageFormatSupport(GLenum internalFormat)
    {
        EXPECT_GL_NO_ERROR();

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, internalFormat, 1, 1);
        glDeleteTextures(1, &tex);

        return (glGetError() == GL_NO_ERROR);
    }

    bool checkRenderbufferFormatSupport(GLenum internalFormat)
    {
        EXPECT_GL_NO_ERROR();

        GLuint rb = 0;
        glGenRenderbuffers(1, &rb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, 1, 1);
        glDeleteRenderbuffers(1, &rb);

        return (glGetError() == GL_NO_ERROR);
    }

    void verifyDepthRenderBuffer(GLenum internalFormat)
    {
        GLTexture tex;
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        ASSERT_GL_NO_ERROR();

        GLRenderbuffer rbDepth;
        glBindRenderbuffer(GL_RENDERBUFFER, rbDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, 1, 1);
        ASSERT_GL_NO_ERROR();

        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbDepth);

        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        ASSERT_GL_NO_ERROR();

        ANGLE_GL_PROGRAM(programRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_GEQUAL);
        glClearDepthf(0.99f);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Pass Depth Test and draw red
        float depthValue = 1.0f;
        drawQuad(programRed, essl1_shaders::PositionAttrib(), depthValue * 2 - 1);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

        ASSERT_GL_NO_ERROR();

        ANGLE_GL_PROGRAM(programGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

        // Fail Depth Test and color buffer is unchanged
        depthValue = 0.98f;
        drawQuad(programGreen, essl1_shaders::PositionAttrib(), depthValue * 2 - 1);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

        ASSERT_GL_NO_ERROR();

        ANGLE_GL_PROGRAM(programBlue, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
        glClearDepthf(0.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Pass Depth Test and draw blue
        depthValue = 0.01f;
        drawQuad(programBlue, essl1_shaders::PositionAttrib(), depthValue * 2 - 1);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

        glDisable(GL_DEPTH_TEST);
        ASSERT_GL_NO_ERROR();
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
})";

        constexpr char kFS[] = R"(precision highp float;
uniform sampler2D tex;
varying vec2 texcoord;

void main()
{
    gl_FragColor = texture2D(tex, texcoord);
})";

        mProgram = CompileProgram(kVS, kFS);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mProgram, "tex");
        EXPECT_NE(-1, mTextureUniformLocation);

        glGenTextures(1, &mTexture);
        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mTexture);
    }

    bool hasReadDepthSupport() const { return IsGLExtensionEnabled("GL_NV_read_depth"); }

    bool hasReadDepthStencilSupport() const
    {
        return IsGLExtensionEnabled("GL_NV_read_depth_stencil");
    }

    bool hasReadStencilSupport() const { return IsGLExtensionEnabled("GL_NV_read_stencil"); }

    bool hasFloatDepthSupport() const { return IsGLExtensionEnabled("GL_NV_depth_buffer_float2"); }

    void depthStencilReadbackCase(const ReadbackTestParam &type);

    GLuint mProgram;
    GLuint mTexture;
    GLint mTextureUniformLocation;
};

class DepthStencilFormatsTest : public DepthStencilFormatsTestBase
{};

class DepthStencilFormatsTestES3 : public DepthStencilFormatsTestBase
{};

TEST_P(DepthStencilFormatsTest, DepthTexture)
{
    bool shouldHaveTextureSupport = (IsGLExtensionEnabled("GL_ANGLE_depth_texture") ||
                                     IsGLExtensionEnabled("GL_OES_depth_texture"));

    EXPECT_EQ(shouldHaveTextureSupport,
              checkTexImageFormatSupport(GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT));
    EXPECT_EQ(shouldHaveTextureSupport,
              checkTexImageFormatSupport(GL_DEPTH_COMPONENT, GL_UNSIGNED_INT));

    if (IsGLExtensionEnabled("GL_EXT_texture_storage"))
    {
        EXPECT_EQ(shouldHaveTextureSupport, checkTexStorageFormatSupport(GL_DEPTH_COMPONENT16));
        EXPECT_EQ(shouldHaveTextureSupport, checkTexStorageFormatSupport(GL_DEPTH_COMPONENT32_OES));
    }
}

TEST_P(DepthStencilFormatsTest, PackedDepthStencil)
{
    // Expected to fail in D3D9 if GL_OES_packed_depth_stencil is not present.
    // Expected to fail in D3D11 if GL_OES_packed_depth_stencil or GL_ANGLE_depth_texture is not
    // present.

    bool shouldHaveRenderbufferSupport = IsGLExtensionEnabled("GL_OES_packed_depth_stencil");
    EXPECT_EQ(shouldHaveRenderbufferSupport,
              checkRenderbufferFormatSupport(GL_DEPTH24_STENCIL8_OES));

    bool shouldHaveTextureSupport = ((IsGLExtensionEnabled("GL_OES_packed_depth_stencil") ||
                                      IsGLExtensionEnabled("GL_OES_depth_texture_cube_map")) &&
                                     IsGLExtensionEnabled("GL_OES_depth_texture")) ||
                                    IsGLExtensionEnabled("GL_ANGLE_depth_texture");
    EXPECT_EQ(shouldHaveTextureSupport,
              checkTexImageFormatSupport(GL_DEPTH_STENCIL_OES, GL_UNSIGNED_INT_24_8_OES));

    if (IsGLExtensionEnabled("GL_EXT_texture_storage"))
    {
        bool shouldHaveTexStorageSupport = IsGLExtensionEnabled("GL_OES_packed_depth_stencil") ||
                                           IsGLExtensionEnabled("GL_ANGLE_depth_texture");
        EXPECT_EQ(shouldHaveTexStorageSupport,
                  checkTexStorageFormatSupport(GL_DEPTH24_STENCIL8_OES));
    }
}

void DepthStencilFormatsTestBase::depthStencilReadbackCase(const ReadbackTestParam &type)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_depth_texture"));

    const bool hasFloatDepth = (type.type == GL_FLOAT);
    ANGLE_SKIP_TEST_IF(hasFloatDepth && !hasFloatDepthSupport());

    const bool hasStencil = (type.format != GL_DEPTH_COMPONENT);

    const bool supportPackedDepthStencilFramebuffer = getClientMajorVersion() >= 3;

    const int res     = 2;
    const int destRes = 4;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // test level > 0
    glTexImage2D(GL_TEXTURE_2D, 1, type.format, 1, 1, 0, type.format, type.type, nullptr);
    EXPECT_GL_NO_ERROR();

    // test with data
    glTexImage2D(GL_TEXTURE_2D, 0, type.format, 1, 1, 0, type.format, type.type, type.data);
    EXPECT_GL_NO_ERROR();

    // test real thing
    glTexImage2D(GL_TEXTURE_2D, 0, type.format, res, res, 0, type.format, type.type, nullptr);
    EXPECT_GL_NO_ERROR();

    // test texSubImage2D
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, type.format, type.type, type.data);
    EXPECT_GL_NO_ERROR();

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    if (type.depthBits > 0 && type.stencilBits > 0 && !supportPackedDepthStencilFramebuffer)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
        EXPECT_GL_NO_ERROR();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, type.attachment, GL_TEXTURE_2D, tex, 0);
        EXPECT_GL_NO_ERROR();
    }

    // Ensure DEPTH_BITS returns >= 16 bits for UNSIGNED_SHORT and UNSIGNED_INT, >= 24
    // UNSIGNED_INT_24_8_WEBGL. If there is stencil, ensure STENCIL_BITS reports >= 8 for
    // UNSIGNED_INT_24_8_WEBGL.

    GLint depthBits = 0;
    glGetIntegerv(GL_DEPTH_BITS, &depthBits);
    EXPECT_GE(depthBits, type.depthBits);

    GLint stencilBits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    EXPECT_GE(stencilBits, type.stencilBits);

    // TODO: remove this check if the spec is updated to require these combinations to work.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        // try adding a color buffer.
        GLuint colorTex = 0;
        glGenTextures(1, &colorTex);
        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res, res, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // use the default texture to render with while we return to the depth texture.
    glBindTexture(GL_TEXTURE_2D, 0);

    /* Setup 2x2 depth texture:
     * 1 0.6 0.8
     * |
     * 0 0.2 0.4
     *    0---1
     */
    GLbitfield clearBits = GL_DEPTH_BUFFER_BIT;
    if (hasStencil)
    {
        clearBits |= GL_STENCIL_BUFFER_BIT;
    }

    const GLfloat d00 = 0.2;
    const GLfloat d01 = 0.4;
    const GLfloat d10 = 0.6;
    const GLfloat d11 = 0.8;
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, 1, 1);
    glClearDepthf(d00);
    glClearStencil(1);
    glClear(clearBits);
    glScissor(1, 0, 1, 1);
    glClearDepthf(d10);
    glClearStencil(2);
    glClear(clearBits);
    glScissor(0, 1, 1, 1);
    glClearDepthf(d01);
    glClearStencil(3);
    glClear(clearBits);
    glScissor(1, 1, 1, 1);
    glClearDepthf(d11);
    glClearStencil(4);
    glClear(clearBits);
    glDisable(GL_SCISSOR_TEST);

    GLubyte actualPixels[destRes * destRes * 8];
    glReadPixels(0, 0, destRes, destRes, GL_DEPTH_COMPONENT,
                 hasFloatDepth ? GL_FLOAT : GL_UNSIGNED_SHORT, actualPixels);
    if (hasReadDepthSupport())
    {
        EXPECT_GL_NO_ERROR();
        if (hasFloatDepth)
        {
            constexpr float kEpsilon = 0.002f;
            const float *pixels      = reinterpret_cast<const float *>(actualPixels);
            ASSERT_NEAR(pixels[0], d00, kEpsilon);
            ASSERT_NEAR(pixels[0 + destRes], d01, kEpsilon);
            ASSERT_NEAR(pixels[1], d10, kEpsilon);
            ASSERT_NEAR(pixels[1 + destRes], d11, kEpsilon);
        }
        else
        {
            constexpr unsigned short kEpsilon = 2;
            const unsigned short *pixels = reinterpret_cast<const unsigned short *>(actualPixels);
            ASSERT_NEAR(pixels[0], gl::unorm<16>(d00), kEpsilon);
            ASSERT_NEAR(pixels[0 + destRes], gl::unorm<16>(d01), kEpsilon);
            ASSERT_NEAR(pixels[1], gl::unorm<16>(d10), kEpsilon);
            ASSERT_NEAR(pixels[1 + destRes], gl::unorm<16>(d11), kEpsilon);
        }
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
    if (hasStencil)
    {
        glReadPixels(0, 0, destRes, destRes, GL_STENCIL_INDEX_OES, GL_UNSIGNED_BYTE, actualPixels);
        if (hasReadStencilSupport())
        {
            EXPECT_GL_NO_ERROR();
            ASSERT_TRUE((actualPixels[0] == 1) && (actualPixels[1] == 2) &&
                        (actualPixels[0 + destRes] == 3) && (actualPixels[1 + destRes] == 4));
        }
        else
        {
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        }

        ASSERT(!hasFloatDepth);

        glReadPixels(0, 0, destRes, destRes, GL_DEPTH_STENCIL_OES, GL_UNSIGNED_INT_24_8_OES,
                     actualPixels);
        if (hasReadDepthStencilSupport())
        {
            EXPECT_GL_NO_ERROR();

            struct Pixel
            {
                uint32_t x;

                uint32_t d24() const { return x >> 8; }

                uint8_t s8() const { return x & 0xff; }
            };

            constexpr unsigned short kEpsilon = 2;
            const Pixel *pixels               = reinterpret_cast<const Pixel *>(actualPixels);

            ASSERT_NEAR(pixels[0].d24(), gl::unorm<24>(d00), kEpsilon);
            ASSERT_NEAR(pixels[0 + destRes].d24(), gl::unorm<24>(d01), kEpsilon);
            ASSERT_NEAR(pixels[1].d24(), gl::unorm<24>(d10), kEpsilon);
            ASSERT_NEAR(pixels[1 + destRes].d24(), gl::unorm<24>(d11), kEpsilon);
            ASSERT_TRUE((pixels[0].s8() == 1) && (pixels[1].s8() == 2) &&
                        (pixels[0 + destRes].s8() == 3) && (pixels[1 + destRes].s8() == 4));
        }
        else
        {
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        }
    }
}

// This test will initialize a depth texture, clear it and read it back, if possible
TEST_P(DepthStencilFormatsTest, DepthStencilReadback_UShort)
{
    GLuint fakeData[10]    = {0};
    ReadbackTestParam type = {
        GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, fakeData, 16, 0};
    depthStencilReadbackCase(type);
}

// This test will initialize a depth texture, clear it and read it back, if possible
TEST_P(DepthStencilFormatsTest, DepthStencilReadback_UInt)
{
    // http://anglebug.com/40644772
    ANGLE_SKIP_TEST_IF(IsMac() && IsIntelUHD630Mobile() && IsDesktopOpenGL());

    GLuint fakeData[10]    = {0};
    ReadbackTestParam type = {
        GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, fakeData, 16, 0};
    depthStencilReadbackCase(type);
}

// This test will initialize a depth texture, clear it and read it back, if possible
TEST_P(DepthStencilFormatsTest, DepthStencilReadback_Float)
{
    // http://anglebug.com/40644772
    ANGLE_SKIP_TEST_IF(IsMac() && IsIntelUHD630Mobile() && IsDesktopOpenGL());

    GLuint fakeData[10]    = {0};
    ReadbackTestParam type = {GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_FLOAT, fakeData, 32, 0};
    depthStencilReadbackCase(type);
}

// This test will initialize a depth texture, clear it and read it back, if possible
TEST_P(DepthStencilFormatsTest, DepthStencilReadback_DepthStencil)
{
    GLuint fakeData[10]    = {0};
    ReadbackTestParam type = {
        GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8_OES, fakeData, 24, 8};
    depthStencilReadbackCase(type);
}

// Verify that packed D/S readPixels with a D32_FLOAT_S8X24_UINT attachment
TEST_P(DepthStencilFormatsTestES3, DepthStencilReadback_DepthFloatStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_packed_depth_stencil") ||
                       !IsGLExtensionEnabled("GL_NV_depth_buffer_float2") ||
                       !IsGLExtensionEnabled("GL_NV_read_depth") ||
                       !IsGLExtensionEnabled("GL_NV_read_depth_stencil") ||
                       !IsGLExtensionEnabled("GL_NV_read_stencil"));

    GLFramebuffer FBO;
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    ASSERT_GL_NO_ERROR();

    GLRenderbuffer RBO;
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, getWindowWidth(),
                          getWindowHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
    ASSERT_GL_NO_ERROR();

    constexpr float kDepthClearValue     = 0.123f;
    constexpr uint8_t kStencilClearValue = 0x42;

    glClearDepthf(kDepthClearValue);
    glClearStencil(kStencilClearValue);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    struct
    {
        float depth;
        uint8_t stencil;
        char unused[3];
    } pixel = {};
    glReadPixels(0, 0, 1, 1, GL_DEPTH_STENCIL_OES, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, &pixel);
    EXPECT_GL_NO_ERROR();

    EXPECT_FLOAT_EQ(pixel.depth, kDepthClearValue);
    EXPECT_EQ(pixel.stencil, kStencilClearValue);
}

// This test will initialize a depth texture and then render with it and verify
// pixel correctness.
// This is modeled after webgl-depth-texture.html
TEST_P(DepthStencilFormatsTest, DepthTextureRender)
{
    constexpr char kVS[] = R"(attribute vec4 a_position;
void main()
{
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(precision mediump float;
uniform sampler2D u_texture;
uniform vec2 u_resolution;
void main()
{
    vec2 texcoord = (gl_FragCoord.xy - vec2(0.5)) / (u_resolution - vec2(1.0));
    gl_FragColor = texture2D(u_texture, texcoord);
})";

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_depth_texture") &&
                       !IsGLExtensionEnabled("GL_ANGLE_depth_texture"));

    bool depthTextureCubeSupport  = IsGLExtensionEnabled("GL_OES_depth_texture_cube_map");
    bool textureSrgbDecodeSupport = IsGLExtensionEnabled("GL_EXT_texture_sRGB_decode");

    // http://anglebug.com/42262117
    ANGLE_SKIP_TEST_IF(IsIntel() && IsWindows() && IsD3D9());

    const int res     = 2;
    const int destRes = 4;
    GLint resolution;

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glUseProgram(program);
    resolution = glGetUniformLocation(program, "u_resolution");
    ASSERT_NE(-1, resolution);
    glUniform2f(resolution, static_cast<float>(destRes), static_cast<float>(destRes));

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    float verts[] = {
        1, 1, 1, -1, 1, 0, -1, -1, -1, 1, 1, 1, -1, -1, -1, 1, -1, 0,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

    // OpenGL ES does not have a FLIPY PixelStore attribute
    // glPixelStorei(GL_UNPACK_FLIP)

    struct TypeInfo
    {
        GLuint attachment;
        GLuint format;
        GLuint type;
        void *data;
        int depthBits;
        int stencilBits;
    };

    GLuint fakeData[10] = {0};

    std::vector<TypeInfo> types = {
        {GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, fakeData, 16, 0},
        {GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, fakeData, 16, 0},
        {GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8_OES, fakeData, 24, 8},
    };

    for (const TypeInfo &type : types)
    {
        GLTexture cubeTex;
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
        ASSERT_GL_NO_ERROR();

        std::vector<GLuint> targets{GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                    GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                    GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};

        for (const GLuint target : targets)
        {
            glTexImage2D(target, 0, type.format, 1, 1, 0, type.format, type.type, nullptr);
            if (depthTextureCubeSupport)
            {
                ASSERT_GL_NO_ERROR();
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_OPERATION);
            }
        }

        std::vector<GLuint> filterModes = {GL_LINEAR, GL_NEAREST};

        const bool supportPackedDepthStencilFramebuffer = getClientMajorVersion() >= 3;

        for (const GLuint filterMode : filterModes)
        {
            GLTexture tex;
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
            if (textureSrgbDecodeSupport)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SRGB_DECODE_EXT, GL_SKIP_DECODE_EXT);
            }

            // test level > 0
            glTexImage2D(GL_TEXTURE_2D, 1, type.format, 1, 1, 0, type.format, type.type, nullptr);
            if (IsGLExtensionEnabled("GL_OES_depth_texture"))
            {
                EXPECT_GL_NO_ERROR();
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_OPERATION);
            }

            // test with data
            glTexImage2D(GL_TEXTURE_2D, 0, type.format, 1, 1, 0, type.format, type.type, type.data);
            if (IsGLExtensionEnabled("GL_OES_depth_texture"))
            {
                EXPECT_GL_NO_ERROR();
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_OPERATION);
            }

            // test copyTexImage2D
            glCopyTexImage2D(GL_TEXTURE_2D, 0, type.format, 0, 0, 1, 1, 0);
            GLuint error = glGetError();
            ASSERT_TRUE(error == GL_INVALID_ENUM || error == GL_INVALID_OPERATION);

            // test real thing
            glTexImage2D(GL_TEXTURE_2D, 0, type.format, res, res, 0, type.format, type.type,
                         nullptr);
            EXPECT_GL_NO_ERROR();

            // test texSubImage2D
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, type.format, type.type, type.data);
            if (IsGLExtensionEnabled("GL_OES_depth_texture"))
            {
                EXPECT_GL_NO_ERROR();
            }
            else
            {
                EXPECT_GL_ERROR(GL_INVALID_OPERATION);
            }

            // test copyTexSubImage2D
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1, 1);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);

            // test generateMipmap
            glGenerateMipmap(GL_TEXTURE_2D);
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);

            GLuint fbo = 0;
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            if (type.depthBits > 0 && type.stencilBits > 0 && !supportPackedDepthStencilFramebuffer)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
                EXPECT_GL_NO_ERROR();
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tex,
                                       0);
                EXPECT_GL_NO_ERROR();
            }
            else
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, type.attachment, GL_TEXTURE_2D, tex, 0);
                EXPECT_GL_NO_ERROR();
            }

            // Ensure DEPTH_BITS returns >= 16 bits for UNSIGNED_SHORT and UNSIGNED_INT, >= 24
            // UNSIGNED_INT_24_8_WEBGL. If there is stencil, ensure STENCIL_BITS reports >= 8 for
            // UNSIGNED_INT_24_8_WEBGL.

            GLint depthBits = 0;
            glGetIntegerv(GL_DEPTH_BITS, &depthBits);
            EXPECT_GE(depthBits, type.depthBits);

            GLint stencilBits = 0;
            glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
            EXPECT_GE(stencilBits, type.stencilBits);

            // TODO: remove this check if the spec is updated to require these combinations to work.
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                // try adding a color buffer.
                GLuint colorTex = 0;
                glGenTextures(1, &colorTex);
                glBindTexture(GL_TEXTURE_2D, colorTex);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res, res, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                             nullptr);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                       colorTex, 0);
                EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
            }

            EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

            // use the default texture to render with while we return to the depth texture.
            glBindTexture(GL_TEXTURE_2D, 0);

            /* Setup 2x2 depth texture:
             * 1 0.6 0.8
             * |
             * 0 0.2 0.4
             *    0---1
             */
            const GLfloat d00 = 0.2;
            const GLfloat d01 = 0.4;
            const GLfloat d10 = 0.6;
            const GLfloat d11 = 0.8;
            glEnable(GL_SCISSOR_TEST);
            glScissor(0, 0, 1, 1);
            glClearDepthf(d00);
            glClear(GL_DEPTH_BUFFER_BIT);
            glScissor(1, 0, 1, 1);
            glClearDepthf(d10);
            glClear(GL_DEPTH_BUFFER_BIT);
            glScissor(0, 1, 1, 1);
            glClearDepthf(d01);
            glClear(GL_DEPTH_BUFFER_BIT);
            glScissor(1, 1, 1, 1);
            glClearDepthf(d11);
            glClear(GL_DEPTH_BUFFER_BIT);
            glDisable(GL_SCISSOR_TEST);

            // render the depth texture.
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, destRes, destRes);
            glBindTexture(GL_TEXTURE_2D, tex);
            glDisable(GL_DITHER);
            glEnable(GL_DEPTH_TEST);
            glClearColor(1, 0, 0, 1);
            glClearDepthf(1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            GLubyte actualPixels[destRes * destRes * 4];
            glReadPixels(0, 0, destRes, destRes, GL_RGBA, GL_UNSIGNED_BYTE, actualPixels);
            const GLfloat eps = 0.002;
            std::vector<GLfloat> expectedMin;
            std::vector<GLfloat> expectedMax;
            if (filterMode == GL_NEAREST)
            {
                GLfloat init[] = {d00, d00, d10, d10, d00, d00, d10, d10,
                                  d01, d01, d11, d11, d01, d01, d11, d11};
                expectedMin.insert(expectedMin.begin(), init, init + 16);
                expectedMax.insert(expectedMax.begin(), init, init + 16);

                for (int i = 0; i < 16; i++)
                {
                    expectedMin[i] = expectedMin[i] - eps;
                    expectedMax[i] = expectedMax[i] + eps;
                }
            }
            else
            {
                GLfloat initMin[] = {
                    d00 - eps, d00, d00, d10 - eps, d00,       d00, d00, d10,
                    d00,       d00, d00, d10,       d01 - eps, d01, d01, d11 - eps,
                };
                GLfloat initMax[] = {
                    d00 + eps, d10, d10, d10 + eps, d01,       d11, d11, d11,
                    d01,       d11, d11, d11,       d01 + eps, d11, d11, d11 + eps,
                };
                expectedMin.insert(expectedMin.begin(), initMin, initMin + 16);
                expectedMax.insert(expectedMax.begin(), initMax, initMax + 16);
            }
            for (int yy = 0; yy < destRes; ++yy)
            {
                for (int xx = 0; xx < destRes; ++xx)
                {
                    const int t        = xx + destRes * yy;
                    const GLfloat was  = (GLfloat)(actualPixels[4 * t] / 255.0);  // 4bpp
                    const GLfloat eMin = expectedMin[t];
                    const GLfloat eMax = expectedMax[t];
                    EXPECT_TRUE(was >= eMin && was <= eMax)
                        << "At " << xx << ", " << yy << ", expected within [" << eMin << ", "
                        << eMax << "] was " << was;
                }
            }

            // check limitations
            // Note: What happens if current attachment type is GL_DEPTH_STENCIL_ATTACHMENT
            // and you try to call glFramebufferTexture2D with GL_DEPTH_ATTACHMENT?
            // The webGL test this code came from expected that to fail.
            // I think due to this line in ES3 spec:
            // GL_INVALID_OPERATION is generated if textarget and texture are not compatible
            // However, that's not the behavior I'm seeing, nor does it seem that a depth_stencil
            // buffer isn't compatible with a depth attachment (e.g. stencil is unused).
            if (type.attachment == GL_DEPTH_ATTACHMENT)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                glFramebufferTexture2D(GL_FRAMEBUFFER, type.attachment, GL_TEXTURE_2D, 0, 0);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                                       tex, 0);
                EXPECT_GLENUM_NE(GL_NO_ERROR, glGetError());
                EXPECT_GLENUM_NE(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
                glClear(GL_DEPTH_BUFFER_BIT);
                EXPECT_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                EXPECT_GL_NO_ERROR();
            }
        }
    }
}

// This test will initialize a frame buffer, attaching a color and 16-bit depth buffer,
// render to it with depth testing, and verify pixel correctness.
TEST_P(DepthStencilFormatsTest, DepthBuffer16)
{
    verifyDepthRenderBuffer(GL_DEPTH_COMPONENT16);
}

// This test will initialize a frame buffer, attaching a color and 24-bit depth buffer,
// render to it with depth testing, and verify pixel correctness.
TEST_P(DepthStencilFormatsTest, DepthBuffer24)
{
    bool shouldHaveRenderbufferSupport = IsGLExtensionEnabled("GL_OES_depth24");
    EXPECT_EQ(shouldHaveRenderbufferSupport,
              checkRenderbufferFormatSupport(GL_DEPTH_COMPONENT24_OES));

    if (shouldHaveRenderbufferSupport)
    {
        verifyDepthRenderBuffer(GL_DEPTH_COMPONENT24_OES);
    }
}

TEST_P(DepthStencilFormatsTestES3, DrawWithDepth16)
{
    GLushort data[16];
    for (unsigned int i = 0; i < 16; i++)
    {
        data[i] = std::numeric_limits<GLushort>::max();
    }
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, 4, 4);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glUseProgram(mProgram);
    glUniform1i(mTextureUniformLocation, 0);

    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(mProgram, "position", 0.5f);

    ASSERT_GL_NO_ERROR();

    GLubyte pixel[4];
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);

    // Only require the red and alpha channels have the correct values, the depth texture extensions
    // leave the green and blue channels undefined
    ASSERT_NEAR(255, pixel[0], 2.0);
    ASSERT_EQ(255, pixel[3]);
}

// This test reproduces a driver bug on Intel windows platforms on driver version
// from 4815 to 4901.
// When rendering with Stencil buffer enabled and depth buffer disabled, large
// viewport will lead to memory leak and driver crash. And the pixel result
// is a random value.
TEST_P(DepthStencilFormatsTestES3, DrawWithLargeViewport)
{
    ANGLE_SKIP_TEST_IF(IsIntel() && IsWindows());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    glEnable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);

    // The iteration is to reproduce memory leak when rendering several times.
    for (int i = 0; i < 10; ++i)
    {
        // Create offscreen fbo and its color attachment and depth stencil attachment.
        GLTexture framebufferColorTexture;
        glBindTexture(GL_TEXTURE_2D, framebufferColorTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
        ASSERT_GL_NO_ERROR();

        GLTexture framebufferStencilTexture;
        glBindTexture(GL_TEXTURE_2D, framebufferStencilTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, getWindowWidth(), getWindowHeight());
        ASSERT_GL_NO_ERROR();

        GLFramebuffer fb;
        glBindFramebuffer(GL_FRAMEBUFFER, fb);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               framebufferColorTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                               framebufferStencilTexture, 0);

        EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        ASSERT_GL_NO_ERROR();

        GLint kStencilRef = 4;
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, kStencilRef, 0xFF);

        float viewport[2];
        glGetFloatv(GL_MAX_VIEWPORT_DIMS, viewport);

        glViewport(0, 0, static_cast<GLsizei>(viewport[0]), static_cast<GLsizei>(viewport[1]));
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);

        drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        ASSERT_GL_NO_ERROR();
    }
}

// Verify that stencil component of depth texture is uploaded
TEST_P(DepthStencilFormatsTest, VerifyDepthStencilUploadData)
{
    // http://anglebug.com/42262342
    // When bug is resolved we can remove this skip.
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsAndroid());

    // http://anglebug.com/42262345
    ANGLE_SKIP_TEST_IF(IsWindows() && IsVulkan() && IsAMD());

    bool shouldHaveTextureSupport = (IsGLExtensionEnabled("GL_OES_packed_depth_stencil") &&
                                     IsGLExtensionEnabled("GL_OES_depth_texture")) ||
                                    (getClientMajorVersion() >= 3);

    ANGLE_SKIP_TEST_IF(!shouldHaveTextureSupport ||
                       !checkTexImageFormatSupport(GL_DEPTH_STENCIL_OES, GL_UNSIGNED_INT_24_8_OES));

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    glEnable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    glClearColor(0, 0, 0, 1);

    // Create offscreen fbo and its color attachment and depth stencil attachment.
    GLTexture framebufferColorTexture;
    glBindTexture(GL_TEXTURE_2D, framebufferColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    // drawQuad's depth range is -1.0 to 1.0, so a depth value of 0.5 (0x7fffff) matches a drawQuad
    // depth of 0.0.
    std::vector<GLuint> depthStencilData(getWindowWidth() * getWindowHeight(), 0x7fffffA9);
    GLTexture framebufferStencilTexture;
    glBindTexture(GL_TEXTURE_2D, framebufferStencilTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, getWindowWidth(), getWindowHeight(), 0,
                 GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8_OES, depthStencilData.data());
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           framebufferColorTexture, 0);

    if (getClientMajorVersion() >= 3)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                               framebufferStencilTexture, 0);
        ASSERT_GL_NO_ERROR();
    }
    else
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                               framebufferStencilTexture, 0);
        ASSERT_GL_NO_ERROR();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                               framebufferStencilTexture, 0);
        ASSERT_GL_NO_ERROR();
    }

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLint kStencilRef = 0xA9;
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, kStencilRef, 0xFF);

    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    glClear(GL_COLOR_BUFFER_BIT);

    drawQuad(program, essl1_shaders::PositionAttrib(), 1.0f);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Check Z values

    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    drawQuad(program, essl1_shaders::PositionAttrib(), -0.1f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);

    glClear(GL_COLOR_BUFFER_BIT);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.1f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::black);
}

// Verify that depth texture's data can be uploaded correctly
TEST_P(DepthStencilFormatsTest, VerifyDepth32UploadData)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_depth_texture"));

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    // normalized 0.99 = 0xfd70a3d6
    std::vector<GLuint> depthData(1, 0xfd70a3d6);
    GLTexture rbDepth;
    glBindTexture(GL_TEXTURE_2D, rbDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1, 1, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,
                 depthData.data());
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rbDepth, 0);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(programRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Fail Depth Test and color buffer is unchanged
    float depthValue = 0.98f;
    drawQuad(programRed, essl1_shaders::PositionAttrib(), depthValue * 2 - 1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Pass Depth Test and draw red
    depthValue = 1.0f;
    drawQuad(programRed, essl1_shaders::PositionAttrib(), depthValue * 2 - 1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();

    // Change depth texture data
    glBindTexture(GL_TEXTURE_2D, rbDepth);
    depthData[0] = 0x7fffffff;  // 0.5
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,
                    depthData.data());
    ASSERT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(programGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    // Fail Depth Test and color buffer is unchanged
    depthValue = 0.48f;
    drawQuad(programGreen, essl1_shaders::PositionAttrib(), depthValue * 2 - 1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(programBlue, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    glClearDepthf(0.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Pass Depth Test and draw blue
    depthValue = 0.01f;
    drawQuad(programBlue, essl1_shaders::PositionAttrib(), depthValue * 2 - 1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    glDisable(GL_DEPTH_TEST);
    ASSERT_GL_NO_ERROR();
}

// Verify that 16 bits depth texture's data can be uploaded correctly
TEST_P(DepthStencilFormatsTest, VerifyDepth16UploadData)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_depth_texture"));

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    // normalized 0.99 = 0xfd6f
    std::vector<GLushort> depthData(1, 0xfd6f);
    GLTexture rbDepth;
    glBindTexture(GL_TEXTURE_2D, rbDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1, 1, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_SHORT, depthData.data());
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rbDepth, 0);

    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(programRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Fail Depth Test and color buffer is unchanged
    float depthValue = 0.98f;
    drawQuad(programRed, essl1_shaders::PositionAttrib(), depthValue * 2 - 1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Pass Depth Test and draw red
    depthValue = 1.0f;
    drawQuad(programRed, essl1_shaders::PositionAttrib(), depthValue * 2 - 1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();

    // Change depth texture data
    glBindTexture(GL_TEXTURE_2D, rbDepth);
    depthData[0] = 0x7fff;  // 0.5
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,
                    depthData.data());
    ASSERT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(programGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    // Fail Depth Test and color buffer is unchanged
    depthValue = 0.48f;
    drawQuad(programGreen, essl1_shaders::PositionAttrib(), depthValue * 2 - 1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(programBlue, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    glClearDepthf(0.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Pass Depth Test and draw blue
    depthValue = 0.01f;
    drawQuad(programBlue, essl1_shaders::PositionAttrib(), depthValue * 2 - 1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    glDisable(GL_DEPTH_TEST);
    ASSERT_GL_NO_ERROR();
}

class DepthStencilFormatsTestES31 : public DepthStencilFormatsTestBase
{};

// Test depth24 texture sample from fragment shader and then read access from compute
TEST_P(DepthStencilFormatsTestES31, DrawReadDrawDispatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_depth24") ||
                       !IsGLExtensionEnabled("GL_NV_read_depth"));

    const char kVS[] = R"(#version 310 es
layout (location = 0) in vec3 pos;
void main(void) {
    gl_Position = vec4(pos, 1.0);
})";

    const char kFS[] = R"(#version 310 es
precision highp float;
uniform sampler2D tex;
out vec4 fragColor;
void main(void) {
        float d = texture(tex,vec2(0,0)).x;
        fragColor = vec4(d, 0, 0, 1);
})";

    // Create depth texture
    GLTexture depthTexture;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, 1, 1);

    // Clear depth texture to 0.51f
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    GLfloat depthValue = 0.51f;
    glClearDepthf(depthValue);
    glClear(GL_DEPTH_BUFFER_BIT);
    GLuint actualDepthValue;
    GLfloat depthErrorTolerance = 0.001f;
    glReadPixels(0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, &actualDepthValue);
    EXPECT_NEAR(depthValue, actualDepthValue / (float)UINT_MAX, depthErrorTolerance);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Render to surface with texture sample from depth texture
    ANGLE_GL_PROGRAM(graphicsProgram, kVS, kFS);
    glUseProgram(graphicsProgram);
    const auto &quadVertices = GetQuadVertices();
    GLBuffer arrayBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(Vector3), quadVertices.data(),
                 GL_STATIC_DRAW);
    GLint positionAttributeLocation = 0;
    glBindAttribLocation(graphicsProgram, positionAttributeLocation, "pos");
    glVertexAttribPointer(positionAttributeLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionAttributeLocation);

    GLint uTextureLocation = glGetUniformLocation(graphicsProgram, "tex");
    ASSERT_NE(-1, uTextureLocation);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(uTextureLocation, 0);
    // Sample the depth texture from fragment shader and verify. This flushes out commands which
    // ensures there will be no layout transition in next renderPass.
    glDepthMask(false);
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor((GLubyte)(depthValue * 255), 0u, 0u, 255u));

    // Sample the depth texture from fragment shader. No image layout transition expected here.
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Sample it from compute shader while the renderPass also sample from the same texture
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding=0) buffer buf {
    vec4 outData;
};
uniform sampler2D u_tex2D;
void main()
{
    outData = texture(u_tex2D, vec2(gl_LocalInvocationID.xy));
})";
    ANGLE_GL_COMPUTE_PROGRAM(computeProgram, kCS);
    glUseProgram(computeProgram);
    GLBuffer ssbo;
    const std::vector<GLfloat> initialData(16, 0.0f);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 16, initialData.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    const GLfloat *ptr = reinterpret_cast<const GLfloat *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 16, GL_MAP_READ_BIT));
    EXPECT_NEAR(depthValue, ptr[0], depthErrorTolerance);
    EXPECT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES2(DepthStencilFormatsTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DepthStencilFormatsTestES3);
ANGLE_INSTANTIATE_TEST_ES3(DepthStencilFormatsTestES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DepthStencilFormatsTestES31);
ANGLE_INSTANTIATE_TEST_ES31(DepthStencilFormatsTestES31);

class TinyDepthStencilWorkaroundTest : public ANGLETest<>
{
  public:
    TinyDepthStencilWorkaroundTest()
    {
        setWindowWidth(512);
        setWindowHeight(512);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Tests that the tiny depth stencil textures workaround does not "stick" depth textures.
// http://anglebug.com/40644610
TEST_P(TinyDepthStencilWorkaroundTest, DepthTexturesStick)
{
    // http://anglebug.com/40096654
    ANGLE_SKIP_TEST_IF((IsAndroid() && IsOpenGLES()) || (IsLinux() && IsVulkan()));

    constexpr char kDrawVS[] =
        "#version 100\n"
        "attribute vec3 vertex;\n"
        "void main () {\n"
        "  gl_Position = vec4(vertex.x, vertex.y, vertex.z * 2.0 - 1.0, 1);\n"
        "}\n";

    ANGLE_GL_PROGRAM(drawProgram, kDrawVS, essl1_shaders::fs::Red());

    constexpr char kBlitVS[] =
        "#version 100\n"
        "attribute vec2 vertex;\n"
        "varying vec2 position;\n"
        "void main () {\n"
        "  position = vertex * .5 + .5;\n"
        "  gl_Position = vec4(vertex, 0, 1);\n"
        "}\n";

    constexpr char kBlitFS[] =
        "#version 100\n"
        "precision mediump float;\n"
        "uniform sampler2D texture;\n"
        "varying vec2 position;\n"
        "void main () {\n"
        "  gl_FragColor = vec4 (texture2D (texture, position).rrr, 1.);\n"
        "}\n";

    ANGLE_GL_PROGRAM(blitProgram, kBlitVS, kBlitFS);

    GLint blitTextureLocation = glGetUniformLocation(blitProgram, "texture");
    ASSERT_NE(-1, blitTextureLocation);

    GLTexture colorTex;
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLTexture depthTex;
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    ASSERT_EQ(getWindowWidth(), getWindowHeight());
    int levels = gl::log2(getWindowWidth());
    for (int mipLevel = 0; mipLevel <= levels; ++mipLevel)
    {
        int size = getWindowWidth() >> mipLevel;
        glTexImage2D(GL_TEXTURE_2D, mipLevel, GL_DEPTH_STENCIL, size, size, 0, GL_DEPTH_STENCIL,
                     GL_UNSIGNED_INT_24_8_OES, nullptr);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);

    ASSERT_GL_NO_ERROR();

    glDepthRangef(0.0f, 1.0f);
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    glClearColor(0, 0, 0, 1);

    // Draw loop.
    for (unsigned int frame = 0; frame < 3; ++frame)
    {
        // draw into FBO
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        float depth = ((frame % 2 == 0) ? 0.0f : 1.0f);
        drawQuad(drawProgram, "vertex", depth);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // blit FBO
        glDisable(GL_DEPTH_TEST);

        glUseProgram(blitProgram);
        glUniform1i(blitTextureLocation, 0);
        glBindTexture(GL_TEXTURE_2D, depthTex);

        drawQuad(blitProgram, "vertex", 0.5f);

        Vector4 depthVec(depth, depth, depth, 1);
        GLColor depthColor(depthVec);

        EXPECT_PIXEL_COLOR_NEAR(0, 0, depthColor, 1);
        ASSERT_GL_NO_ERROR();
    }
}

// Initialize a depth texture by writing to it in a fragment shader then attempt to read it from a
// compute shader. Regression test for D3D11 not unbinding the depth texture and the sampler binding
// failing.
TEST_P(DepthStencilFormatsTestES31, ReadDepthStencilInComputeShader)
{
    constexpr char kTestVertexShader[] = R"(#version 310 es

void main() {
  gl_PointSize = 1.0;
  vec2 pos[6] = vec2[6](vec2(-1.0f), vec2(1.0f, -1.0f), vec2(-1.0f, 1.0f), vec2(-1.0f, 1.0f), vec2(1.0f, -1.0f), vec2(1.0f));
  gl_Position = vec4(pos[uint(gl_VertexID)], 0.5f, 1.0f);
  return;
}
)";

    constexpr char kTestFragmentShader[] = R"(#version 310 es
precision highp float;

void main()
{
  gl_FragDepth = 0.51f;
  return;
}
)";

    constexpr char kTestComputeShader[] = R"(#version 310 es

layout(binding = 0, std430) buffer dst_buf_block_ssbo {
  float inner[];
} dst_buf;

uniform highp sampler2D tex;
void blit_depth_to_buffer(uvec3 id) {
  uvec2 coord = (id.xy);
  uint dstOffset = (id.x + (id.y * 1u));
  dst_buf.inner[dstOffset] = texelFetch(tex, ivec2(coord), 0).x;
}

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main() {
  blit_depth_to_buffer(gl_GlobalInvocationID);
}
)";

    GLfloat depthValue          = 0.51f;
    GLfloat depthErrorTolerance = 0.001f;

    GLTexture depthTexture;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, 1, 1);

    // Clear depth texture to depthValue
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // 2. Draw and write to gl_FragDepth
    ANGLE_GL_PROGRAM(graphicsProgram, kTestVertexShader, kTestFragmentShader);
    glUseProgram(graphicsProgram);
    EXPECT_GL_NO_ERROR();

    glDrawBuffers(0, nullptr);
    EXPECT_GL_NO_ERROR();

    glClearDepthf(0.8);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDisable(GL_CULL_FACE);
    glDepthMask(true);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ANGLE_GL_COMPUTE_PROGRAM(computeProgram, kTestComputeShader);
    glUseProgram(computeProgram);

    GLint uTextureLocation = glGetUniformLocation(computeProgram, "tex");
    ASSERT_NE(-1, uTextureLocation);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(uTextureLocation, 0);

    GLBuffer ssbo;
    std::array<GLfloat, 16> initialData{0};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 16, initialData.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    const GLfloat *ptr = reinterpret_cast<const GLfloat *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 16, GL_MAP_READ_BIT));
    EXPECT_NEAR(depthValue, ptr[0], depthErrorTolerance);
    EXPECT_GL_NO_ERROR();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(TinyDepthStencilWorkaroundTest);
ANGLE_INSTANTIATE_TEST_ES3_AND(TinyDepthStencilWorkaroundTest,
                               ES3_D3D11().enable(Feature::EmulateTinyStencilTextures));
