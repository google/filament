//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferObjectTest.cpp: Tests basic usage of OES_framebuffer_object extension.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class FramebufferObjectTest : public ANGLETest<>
{
  protected:
    FramebufferObjectTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        mTexture.reset(new GLTexture());
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mTexture->get());
    }

    void testTearDown() override { mTexture.reset(); }

    std::unique_ptr<GLTexture> mTexture;
};

// Checks that framebuffer object can be used without GL errors.
TEST_P(FramebufferObjectTest, FramebufferObject)
{
    GLuint fboId;
    GLint params;

    glGenFramebuffersOES(1, &fboId);
    EXPECT_GL_NO_ERROR();
    glIsFramebufferOES(fboId);
    EXPECT_GL_NO_ERROR();
    glBindFramebufferOES(GL_FRAMEBUFFER, fboId);
    EXPECT_GL_NO_ERROR();

    glCheckFramebufferStatusOES(GL_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();
    glGetFramebufferAttachmentParameterivOES(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                             GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &params);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffersOES(1, &fboId);
    EXPECT_GL_NO_ERROR();
}

// Checks that texture object can be bound for framebuffer object.
TEST_P(FramebufferObjectTest, TextureObject)
{
    GLuint fboId;

    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glFramebufferTexture2DOES(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture->get(),
                              0);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fboId);
}

// Checks different formats for a texture object bound to a framebuffer object.
TEST_P(FramebufferObjectTest, TextureObjectDifferentFormats)
{
    // http://anglebug.com/42264178
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    GLuint fboId;

    glGenFramebuffersOES(1, &fboId);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, fboId);

    using FormatInfo                                  = std::array<GLenum, 3>;
    constexpr std::array<FormatInfo, 5> kFormatArrays = {
        {{GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
         {GL_RGB, GL_RGB, GL_UNSIGNED_BYTE},
         {GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4},
         {GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1},
         {GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5}}};

    for (const FormatInfo &formatInfo : kFormatArrays)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, formatInfo[0], 1, 1, 0, formatInfo[1], formatInfo[2],
                     &GLColor::green);
        glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D,
                                  mTexture->get(), 0);
        ASSERT_EQ(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES),
                  (GLenum)GL_FRAMEBUFFER_COMPLETE_OES);
    }

    EXPECT_GL_NO_ERROR();

    glBindFramebufferOES(GL_FRAMEBUFFER_OES, 0);
    glDeleteFramebuffersOES(1, &fboId);
}

// Checks that renderbuffer object can be used and can be bound for framebuffer object.
TEST_P(FramebufferObjectTest, RenderbufferObject)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_framebuffer_object"));

    GLuint fboId;
    GLuint rboId;
    GLint params;

    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    glGenRenderbuffersOES(1, &rboId);
    EXPECT_GL_NO_ERROR();
    glIsRenderbufferOES(rboId);
    EXPECT_GL_NO_ERROR();
    glBindRenderbufferOES(GL_RENDERBUFFER, rboId);
    EXPECT_GL_NO_ERROR();
    glRenderbufferStorageOES(GL_RENDERBUFFER, GL_RGBA4, 32, 32);
    EXPECT_GL_NO_ERROR();
    glRenderbufferStorageOES(GL_RENDERBUFFER, GL_RGB5_A1, 32, 32);
    EXPECT_GL_NO_ERROR();
    glRenderbufferStorageOES(GL_RENDERBUFFER, GL_RGB565, 32, 32);
    EXPECT_GL_NO_ERROR();
    glRenderbufferStorageOES(GL_RENDERBUFFER, GL_RGBA8, 32, 32);
    EXPECT_GL_NO_ERROR();

    glFramebufferRenderbufferOES(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboId);
    EXPECT_GL_NO_ERROR();
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &params);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fboId);
    glDeleteRenderbuffersOES(1, &rboId);
    EXPECT_GL_NO_ERROR();
}

// Checks that an RGBA8 renderbuffer object can be used and can be bound for framebuffer object.
TEST_P(FramebufferObjectTest, RGBA8Renderbuffer)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_framebuffer_object"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARM_rgba8"));

    GLuint fbo;
    GLuint rbo;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffersOES(1, &rbo);
    EXPECT_GL_NO_ERROR();
    glIsRenderbufferOES(rbo);
    EXPECT_GL_NO_ERROR();
    glBindRenderbufferOES(GL_RENDERBUFFER, rbo);
    EXPECT_GL_NO_ERROR();
    glRenderbufferStorageOES(GL_RENDERBUFFER, GL_RGBA8, 16, 16);
    EXPECT_GL_NO_ERROR();

    glFramebufferRenderbufferOES(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    EXPECT_GL_NO_ERROR();

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::white);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffersOES(1, &rbo);
    EXPECT_GL_NO_ERROR();
}

// Checks that an RGB8 and an RGBA8 renderbuffer object can be used and can be bound for framebuffer
// object one after the other.
TEST_P(FramebufferObjectTest, RGB8AndRGBA8Renderbuffers)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_framebuffer_object"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_rgb8_rgba8"));

    GLuint fbo;
    GLuint rbo[2];

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffersOES(2, rbo);
    EXPECT_GL_NO_ERROR();
    glBindRenderbufferOES(GL_RENDERBUFFER, rbo[0]);
    EXPECT_GL_NO_ERROR();
    glRenderbufferStorageOES(GL_RENDERBUFFER, GL_RGB8, 16, 16);
    EXPECT_GL_NO_ERROR();
    glBindRenderbufferOES(GL_RENDERBUFFER, rbo[1]);
    EXPECT_GL_NO_ERROR();
    glRenderbufferStorageOES(GL_RENDERBUFFER, GL_RGBA8, 16, 16);
    EXPECT_GL_NO_ERROR();

    glFramebufferRenderbufferOES(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo[0]);
    EXPECT_GL_NO_ERROR();

    glClearColor(0.0, 1.0, 0.0, 0.1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::green);

    glFramebufferRenderbufferOES(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo[1]);
    EXPECT_GL_NO_ERROR();

    glClearColor(1.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::magenta);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffersOES(2, rbo);
    EXPECT_GL_NO_ERROR();
}

// Checks that generateMipmap can be called without GL errors.
TEST_P(FramebufferObjectTest, GenerateMipmap)
{
    constexpr uint32_t kSize = 32;
    std::vector<unsigned char> pixelData(kSize * kSize * 4, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());

    glGenerateMipmapOES(GL_TEXTURE_2D);
    EXPECT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES1(FramebufferObjectTest);