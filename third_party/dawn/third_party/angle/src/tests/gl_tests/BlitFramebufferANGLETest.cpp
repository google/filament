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
class BlitFramebufferANGLETest : public ANGLETest<>
{
  protected:
    BlitFramebufferANGLETest()
    {
        setWindowWidth(64);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);

        mCheckerProgram = 0;
        mBlueProgram    = 0;
        mRedProgram     = 0;

        mOriginalFBO = 0;

        mUserFBO                = 0;
        mUserColorBuffer        = 0;
        mUserDepthStencilBuffer = 0;

        mSmallFBO                = 0;
        mSmallColorBuffer        = 0;
        mSmallDepthStencilBuffer = 0;

        mColorOnlyFBO         = 0;
        mColorOnlyColorBuffer = 0;

        mDiffFormatFBO         = 0;
        mDiffFormatColorBuffer = 0;

        mDiffSizeFBO         = 0;
        mDiffSizeColorBuffer = 0;

        mMRTFBO          = 0;
        mMRTColorBuffer0 = 0;
        mMRTColorBuffer1 = 0;

        mRGBAColorbuffer              = 0;
        mRGBAFBO                      = 0;
        mRGBAMultisampledRenderbuffer = 0;
        mRGBAMultisampledFBO          = 0;

        mBGRAColorbuffer              = 0;
        mBGRAFBO                      = 0;
        mBGRAMultisampledRenderbuffer = 0;
        mBGRAMultisampledFBO          = 0;
    }

    void testSetUp() override
    {
        mCheckerProgram =
            CompileProgram(essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Checkered());
        mBlueProgram = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
        mRedProgram  = CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
        if (mCheckerProgram == 0 || mBlueProgram == 0 || mRedProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        EXPECT_GL_NO_ERROR();

        GLint originalFBO;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originalFBO);
        if (originalFBO >= 0)
        {
            mOriginalFBO = (GLuint)originalFBO;
        }

        GLenum format = GL_RGBA;

        glGenFramebuffers(1, &mUserFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);
        glGenTextures(1, &mUserColorBuffer);
        glGenRenderbuffers(1, &mUserDepthStencilBuffer);
        glBindTexture(GL_TEXTURE_2D, mUserColorBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               mUserColorBuffer, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, format, getWindowWidth(), getWindowHeight(), 0, format,
                     GL_UNSIGNED_BYTE, nullptr);
        glBindRenderbuffer(GL_RENDERBUFFER, mUserDepthStencilBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, getWindowWidth(),
                              getWindowHeight());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                  mUserDepthStencilBuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  mUserDepthStencilBuffer);

        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        ASSERT_GL_NO_ERROR();

        glGenFramebuffers(1, &mSmallFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mSmallFBO);
        glGenTextures(1, &mSmallColorBuffer);
        glGenRenderbuffers(1, &mSmallDepthStencilBuffer);
        glBindTexture(GL_TEXTURE_2D, mSmallColorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, format, getWindowWidth() / 2, getWindowHeight() / 2, 0,
                     format, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               mSmallColorBuffer, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, mSmallDepthStencilBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, getWindowWidth() / 2,
                              getWindowHeight() / 2);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                  mSmallDepthStencilBuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  mSmallDepthStencilBuffer);

        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        ASSERT_GL_NO_ERROR();

        glGenFramebuffers(1, &mColorOnlyFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mColorOnlyFBO);
        glGenTextures(1, &mColorOnlyColorBuffer);
        glBindTexture(GL_TEXTURE_2D, mColorOnlyColorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, format, getWindowWidth(), getWindowHeight(), 0, format,
                     GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               mColorOnlyColorBuffer, 0);

        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        ASSERT_GL_NO_ERROR();

        glGenFramebuffers(1, &mDiffFormatFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mDiffFormatFBO);
        glGenTextures(1, &mDiffFormatColorBuffer);
        glBindTexture(GL_TEXTURE_2D, mDiffFormatColorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getWindowWidth(), getWindowHeight(), 0, GL_RGB,
                     GL_UNSIGNED_SHORT_5_6_5, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               mDiffFormatColorBuffer, 0);

        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        ASSERT_GL_NO_ERROR();

        glGenFramebuffers(1, &mDiffSizeFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mDiffSizeFBO);
        glGenTextures(1, &mDiffSizeColorBuffer);
        glBindTexture(GL_TEXTURE_2D, mDiffSizeColorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, format, getWindowWidth() * 2, getWindowHeight() * 2, 0,
                     format, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               mDiffSizeColorBuffer, 0);

        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
        ASSERT_GL_NO_ERROR();

        if (IsGLExtensionEnabled("GL_EXT_draw_buffers"))
        {
            glGenFramebuffers(1, &mMRTFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, mMRTFBO);
            glGenTextures(1, &mMRTColorBuffer0);
            glGenTextures(1, &mMRTColorBuffer1);
            glBindTexture(GL_TEXTURE_2D, mMRTColorBuffer0);
            glTexImage2D(GL_TEXTURE_2D, 0, format, getWindowWidth(), getWindowHeight(), 0, format,
                         GL_UNSIGNED_BYTE, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D,
                                   mMRTColorBuffer0, 0);
            glBindTexture(GL_TEXTURE_2D, mMRTColorBuffer1);
            glTexImage2D(GL_TEXTURE_2D, 0, format, getWindowWidth(), getWindowHeight(), 0, format,
                         GL_UNSIGNED_BYTE, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D,
                                   mMRTColorBuffer1, 0);

            ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
            ASSERT_GL_NO_ERROR();
        }

        if (IsGLExtensionEnabled("GL_ANGLE_framebuffer_multisample") &&
            IsGLExtensionEnabled("GL_OES_rgb8_rgba8"))
        {
            // RGBA single-sampled framebuffer
            glGenTextures(1, &mRGBAColorbuffer);
            glBindTexture(GL_TEXTURE_2D, mRGBAColorbuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);

            glGenFramebuffers(1, &mRGBAFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, mRGBAFBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   mRGBAColorbuffer, 0);

            ASSERT_GL_NO_ERROR();
            ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

            // RGBA multisampled framebuffer
            glGenRenderbuffers(1, &mRGBAMultisampledRenderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, mRGBAMultisampledRenderbuffer);
            glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, 1, GL_RGBA8, getWindowWidth(),
                                                  getWindowHeight());

            glGenFramebuffers(1, &mRGBAMultisampledFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, mRGBAMultisampledFBO);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                      mRGBAMultisampledRenderbuffer);

            ASSERT_GL_NO_ERROR();
            ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

            if (IsGLExtensionEnabled("GL_EXT_texture_format_BGRA8888"))
            {
                // BGRA single-sampled framebuffer
                glGenTextures(1, &mBGRAColorbuffer);
                glBindTexture(GL_TEXTURE_2D, mBGRAColorbuffer);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, getWindowWidth(), getWindowHeight(), 0,
                             GL_BGRA_EXT, GL_UNSIGNED_BYTE, nullptr);

                glGenFramebuffers(1, &mBGRAFBO);
                glBindFramebuffer(GL_FRAMEBUFFER, mBGRAFBO);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                       mBGRAColorbuffer, 0);

                ASSERT_GL_NO_ERROR();
                ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

                // BGRA multisampled framebuffer
                glGenRenderbuffers(1, &mBGRAMultisampledRenderbuffer);
                glBindRenderbuffer(GL_RENDERBUFFER, mBGRAMultisampledRenderbuffer);
                glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, 1, GL_BGRA8_EXT,
                                                      getWindowWidth(), getWindowHeight());

                glGenFramebuffers(1, &mBGRAMultisampledFBO);
                glBindFramebuffer(GL_FRAMEBUFFER, mBGRAMultisampledFBO);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                          mBGRAMultisampledRenderbuffer);

                ASSERT_GL_NO_ERROR();
                ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);
    }

    void testTearDown() override
    {
        glDeleteProgram(mCheckerProgram);
        glDeleteProgram(mBlueProgram);
        glDeleteProgram(mRedProgram);

        glDeleteFramebuffers(1, &mUserFBO);
        glDeleteTextures(1, &mUserColorBuffer);
        glDeleteRenderbuffers(1, &mUserDepthStencilBuffer);

        glDeleteFramebuffers(1, &mSmallFBO);
        glDeleteTextures(1, &mSmallColorBuffer);
        glDeleteRenderbuffers(1, &mSmallDepthStencilBuffer);

        glDeleteFramebuffers(1, &mColorOnlyFBO);
        glDeleteTextures(1, &mSmallDepthStencilBuffer);

        glDeleteFramebuffers(1, &mDiffFormatFBO);
        glDeleteTextures(1, &mDiffFormatColorBuffer);

        glDeleteFramebuffers(1, &mDiffSizeFBO);
        glDeleteTextures(1, &mDiffSizeColorBuffer);

        if (IsGLExtensionEnabled("GL_EXT_draw_buffers"))
        {
            glDeleteFramebuffers(1, &mMRTFBO);
            glDeleteTextures(1, &mMRTColorBuffer0);
            glDeleteTextures(1, &mMRTColorBuffer1);
        }

        if (mRGBAColorbuffer != 0)
        {
            glDeleteTextures(1, &mRGBAColorbuffer);
        }

        if (mRGBAFBO != 0)
        {
            glDeleteFramebuffers(1, &mRGBAFBO);
        }

        if (mRGBAMultisampledRenderbuffer != 0)
        {
            glDeleteRenderbuffers(1, &mRGBAMultisampledRenderbuffer);
        }

        if (mRGBAMultisampledFBO != 0)
        {
            glDeleteFramebuffers(1, &mRGBAMultisampledFBO);
        }

        if (mBGRAColorbuffer != 0)
        {
            glDeleteTextures(1, &mBGRAColorbuffer);
        }

        if (mBGRAFBO != 0)
        {
            glDeleteFramebuffers(1, &mBGRAFBO);
        }

        if (mBGRAMultisampledRenderbuffer != 0)
        {
            glDeleteRenderbuffers(1, &mBGRAMultisampledRenderbuffer);
        }

        if (mBGRAMultisampledFBO != 0)
        {
            glDeleteFramebuffers(1, &mBGRAMultisampledFBO);
        }
    }

    void multisampleTestHelper(GLuint readFramebuffer, GLuint drawFramebuffer)
    {
        glClearColor(0.0, 1.0, 0.0, 1.0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, readFramebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        EXPECT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer);
        glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                               getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
        EXPECT_GL_NO_ERROR();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFramebuffer);
        EXPECT_PIXEL_EQ(getWindowWidth() / 4, getWindowHeight() / 4, 0, 255, 0, 255);
        EXPECT_PIXEL_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, 0, 255, 0, 255);
        EXPECT_PIXEL_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, 0, 255, 0, 255);
        EXPECT_PIXEL_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, 0, 255, 0, 255);
    }

    bool checkExtension(const std::string &extension)
    {
        if (!IsGLExtensionEnabled(extension))
        {
            std::cout << "Test skipped because " << extension << " not supported." << std::endl;
            return false;
        }

        return true;
    }

    void BlitStencilTestHelper(bool mesaYFlip)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

        if (mesaYFlip)
        {
            ASSERT_TRUE(IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));
            glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
        }

        glClearColor(0.0, 1.0, 0.0, 1.0);
        glClearStencil(0x0);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Scissor half the screen so we fill the stencil only halfway
        glScissor(0, 0, getWindowWidth(), getWindowHeight() / 2);
        glEnable(GL_SCISSOR_TEST);

        // fill the stencil buffer with 0x1
        glStencilFunc(GL_ALWAYS, 0x1, 0xFF);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        glEnable(GL_STENCIL_TEST);
        drawQuad(mRedProgram, essl1_shaders::PositionAttrib(), 0.3f);

        glDisable(GL_SCISSOR_TEST);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);
        glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);

        // These clears are not useful in theory because we're copying over them, but its
        // helpful in debugging if we see white in any result.
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClearStencil(0x0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                               getWindowHeight(), GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                               GL_NEAREST);

        EXPECT_GL_NO_ERROR();

        glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::green);

        glStencilFunc(GL_EQUAL, 0x1, 0xFF);  // only pass if stencil buffer at pixel reads 0x1

        drawQuad(mBlueProgram, essl1_shaders::PositionAttrib(),
                 0.8f);  // blue quad will draw if stencil buffer was copied

        glDisable(GL_STENCIL_TEST);

        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);
        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);
        EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::green);
    }

    GLuint mCheckerProgram;
    GLuint mBlueProgram;
    GLuint mRedProgram;

    GLuint mOriginalFBO;

    GLuint mUserFBO;
    GLuint mUserColorBuffer;
    GLuint mUserDepthStencilBuffer;

    GLuint mSmallFBO;
    GLuint mSmallColorBuffer;
    GLuint mSmallDepthStencilBuffer;

    GLuint mColorOnlyFBO;
    GLuint mColorOnlyColorBuffer;

    GLuint mDiffFormatFBO;
    GLuint mDiffFormatColorBuffer;

    GLuint mDiffSizeFBO;
    GLuint mDiffSizeColorBuffer;

    GLuint mMRTFBO;
    GLuint mMRTColorBuffer0;
    GLuint mMRTColorBuffer1;

    GLuint mRGBAColorbuffer;
    GLuint mRGBAFBO;
    GLuint mRGBAMultisampledRenderbuffer;
    GLuint mRGBAMultisampledFBO;

    GLuint mBGRAColorbuffer;
    GLuint mBGRAFBO;
    GLuint mBGRAMultisampledRenderbuffer;
    GLuint mBGRAMultisampledFBO;
};

// Draw to user-created framebuffer, blit whole-buffer color to original framebuffer.
TEST_P(BlitFramebufferANGLETest, BlitColorToDefault)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.8f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);

    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::yellow);
}

// Blit color to/from default framebuffer with Flip-X/Flip-Y.
TEST_P(BlitFramebufferANGLETest, BlitColorWithFlip)
{
    // OpenGL ES 3.0 / GL_NV_framebuffer_blit required for flip.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_NV_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.8f);

    EXPECT_GL_NO_ERROR();

    // Blit to default with x-flip.
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);

    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), getWindowWidth(), 0, 0,
                      getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::green);

    // Blit to default with y-flip.
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, getWindowHeight(),
                      getWindowWidth(), 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::blue);

    // Blit from default with x-flip.

    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), getWindowWidth(), 0, 0,
                      getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::red);

    // Blit from default with y-flip.
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, getWindowHeight(),
                      getWindowWidth(), 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::yellow);
}

// Blit color to default framebuffer from another framebuffer with GL_MESA_framebuffer_flip_y.
TEST_P(BlitFramebufferANGLETest, BlitColorWithMesaYFlipSrc)
{
    // OpenGL ES 3.0 / GL_NV_framebuffer_blit required for flip.
    ANGLE_SKIP_TEST_IF(
        (getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_NV_framebuffer_blit")) ||
        !IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.8f);

    EXPECT_GL_NO_ERROR();

    // Blit to default from y-flipped.
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);

    const int fboTargetWidth  = getWindowHeight() / 2;
    const int fboTargetHeight = getWindowHeight() / 2;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, fboTargetWidth,
                      fboTargetHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    EXPECT_PIXEL_COLOR_EQ(fboTargetWidth / 4, fboTargetHeight / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(fboTargetWidth / 4, 3 * fboTargetHeight / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * fboTargetWidth / 4, fboTargetHeight / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * fboTargetWidth / 4, 3 * fboTargetHeight / 4, GLColor::yellow);
}

// Blit color to y-flipped with GL_MESA_framebuffer_flip_y framebuffer from normal framebuffer.
TEST_P(BlitFramebufferANGLETest, BlitColorWithMesaYFlipDst)
{
    // OpenGL ES 3.0 / GL_NV_framebuffer_blit required for flip.
    ANGLE_SKIP_TEST_IF(
        (getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_NV_framebuffer_blit")) ||
        !IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.8f);

    EXPECT_GL_NO_ERROR();

    // Blit to default from y-flipped.
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mUserFBO);

    glFramebufferParameteriMESA(GL_DRAW_FRAMEBUFFER_ANGLE, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);

    const int fboTargetWidth  = getWindowWidth() / 2;
    const int fboTargetHeight = getWindowHeight();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, fboTargetWidth,
                      fboTargetHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), getWindowWidth() / 2, 0,
                      getWindowWidth(), getWindowHeight() / 2, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glFramebufferParameteriMESA(GL_DRAW_FRAMEBUFFER_ANGLE, GL_FRAMEBUFFER_FLIP_Y_MESA, 0);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    // Left side have inverted checker pattern.
    EXPECT_PIXEL_COLOR_EQ(fboTargetWidth / 4, fboTargetHeight / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(fboTargetWidth / 4, 3 * fboTargetHeight / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * fboTargetWidth / 4, fboTargetHeight / 4, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(3 * fboTargetWidth / 4, 3 * fboTargetHeight / 4, GLColor::blue);

    // Right side is split to 2 parts where upper part have non y-flipped checker pattern and the
    // bottom one has white color.
    EXPECT_PIXEL_COLOR_EQ(5 * getWindowWidth() / 8, 5 * getWindowHeight() / 8, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(5 * getWindowWidth() / 8, 7 * getWindowHeight() / 8, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(7 * getWindowWidth() / 8, 5 * getWindowHeight() / 8, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(7 * getWindowWidth() / 8, 7 * getWindowHeight() / 8, GLColor::blue);

    EXPECT_PIXEL_RECT_EQ(4 * getWindowWidth() / 8, 0, getWindowWidth() / 4, getWindowHeight() / 2,
                         GLColor::white);
}

// Blit color to/from y-flipped with GL_MESA_framebuffer_flip_y framebuffers where dst framebuffer
// have different size.
TEST_P(BlitFramebufferANGLETest, BlitColorWithMesaYFlipSrcDst)
{
    // OpenGL ES 3.0 / GL_NV_framebuffer_blit required for flip.
    ANGLE_SKIP_TEST_IF(
        (getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_NV_framebuffer_blit")) ||
        !IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    // Create a custom framebuffer as the default one cannot be flipped.
    GLTexture tex0;
    glBindTexture(GL_TEXTURE_2D, tex0);
    const int fb0Width  = getWindowWidth() / 2;
    const int fb0Height = getWindowHeight() / 2;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb0Width, fb0Height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    GLFramebuffer fb0;
    glBindFramebuffer(GL_FRAMEBUFFER, fb0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex0, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.8f);

    EXPECT_GL_NO_ERROR();

    // Blit to default from y-flipped.
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, fb0);

    glFramebufferParameteriMESA(GL_DRAW_FRAMEBUFFER_ANGLE, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
    glFramebufferParameteriMESA(GL_READ_FRAMEBUFFER_ANGLE, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);

    const int fboTargetWidth  = fb0Width / 2;
    const int fboTargetHeight = fb0Height;

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, fboTargetWidth,
                      fboTargetHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), fb0Width / 2, 0, fb0Width,
                      fb0Height / 2, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, fb0);

    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 0);

    // Left side have inverted checker pattern.
    EXPECT_PIXEL_COLOR_EQ(fboTargetWidth / 4, fboTargetHeight / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(fboTargetWidth / 4, 3 * fboTargetHeight / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * fboTargetWidth / 4, fboTargetHeight / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * fboTargetWidth / 4, 3 * fboTargetHeight / 4, GLColor::yellow);

    // Right side is split to 2 parts where upper part have y-flipped checker pattern and the
    // bottom one has white color.
    EXPECT_PIXEL_COLOR_EQ(5 * fb0Width / 8, 5 * fb0Height / 8, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(5 * fb0Width / 8, 7 * fb0Height / 8, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(7 * fb0Width / 8, 5 * fb0Height / 8, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(7 * fb0Width / 8, 7 * fb0Height / 8, GLColor::yellow);

    EXPECT_PIXEL_RECT_EQ(4 * fb0Width / 8, 0, fb0Width / 4, fb0Height / 2, GLColor::white);
}

// Same as BlitColorWithMesaYFlip but uses an integer buffer format.
TEST_P(BlitFramebufferANGLETest, BlitColorWithMesaYFlipInteger)
{
    // OpenGL ES 3.0 / GL_NV_framebuffer_blit required for flip.
    ANGLE_SKIP_TEST_IF(
        (getClientMajorVersion() < 3 || !IsGLExtensionEnabled("GL_NV_framebuffer_blit")) ||
        !IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    GLTexture tex0;
    glBindTexture(GL_TEXTURE_2D, tex0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8I, getWindowWidth(), getWindowHeight(), 0,
                 GL_RGBA_INTEGER, GL_BYTE, nullptr);

    GLFramebuffer fb0;
    glBindFramebuffer(GL_FRAMEBUFFER, fb0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex0, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.8f);

    EXPECT_GL_NO_ERROR();

    GLTexture tex1;
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8I, getWindowWidth(), getWindowHeight(), 0,
                 GL_RGBA_INTEGER, GL_BYTE, nullptr);

    GLFramebuffer fb1;
    glBindFramebuffer(GL_FRAMEBUFFER, fb1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);

    // Blit to default from y-flipped.
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, fb0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, fb1);

    const int fb1_target_width  = getWindowHeight() / 3;
    const int fb1_target_height = getWindowHeight() / 3;

    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, fb1_target_width,
                      fb1_target_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, fb1);

    // The colors outside the target must remain the same.
    EXPECT_PIXEL_8I(getWindowWidth() - 1, getWindowHeight() - 1, 0, 127, 127, 127);
    EXPECT_PIXEL_8I(getWindowWidth() - 1, 0, 0, 127, 127, 127);
    EXPECT_PIXEL_8I(0, getWindowHeight() - 1, 0, 127, 127, 127);
    EXPECT_PIXEL_8I(fb1_target_width, fb1_target_height, 0, 127, 127, 127);

    // While inside must change.
    EXPECT_PIXEL_8I(fb1_target_width / 4, fb1_target_height / 4, 127, 0, 0, 127);
    EXPECT_PIXEL_8I(fb1_target_width / 4, 3 * fb1_target_height / 4, 0, 127, 0, 127);
    EXPECT_PIXEL_8I(3 * fb1_target_width / 4, fb1_target_height / 4, 0, 0, 127, 127);
    EXPECT_PIXEL_8I(3 * fb1_target_width / 4, 3 * fb1_target_height / 4, 127, 127, 0, 127);

    // Blit from y-flipped to default.
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, fb1);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, fb0);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Set y-flip flag so that y-flipped frame buffer blit to the original fbo in reverse. This
    // should result in flipping y back.
    glFramebufferParameteriMESA(GL_DRAW_FRAMEBUFFER_ANGLE, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBlitFramebuffer(0, 0, fb1_target_width, fb1_target_height, 0, 0, getWindowWidth(),
                      getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // And explicitly disable y-flip so that read does not implicitly use this flag.
    glFramebufferParameteriMESA(GL_DRAW_FRAMEBUFFER_ANGLE, GL_FRAMEBUFFER_FLIP_Y_MESA, 0);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, fb0);

    EXPECT_PIXEL_8I(getWindowWidth() / 4, getWindowHeight() / 4, 0, 127, 0, 127);
    EXPECT_PIXEL_8I(getWindowWidth() / 4, 3 * getWindowHeight() / 4, 127, 0, 0, 127);
    EXPECT_PIXEL_8I(3 * getWindowWidth() / 4, getWindowHeight() / 4, 127, 127, 0, 127);
    EXPECT_PIXEL_8I(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, 0, 0, 127, 127);
}

// Draw to system framebuffer, blit whole-buffer color to user-created framebuffer.
TEST_P(BlitFramebufferANGLETest, ReverseColorBlit)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    // TODO(jmadill): Fix this. http://anglebug.com/42261451
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsAndroid());

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.8f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mUserFBO);

    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::yellow);
}

// blit from user-created FBO to system framebuffer, with the scissor test enabled.
TEST_P(BlitFramebufferANGLETest, ScissoredBlit)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.8f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glScissor(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight());
    glEnable(GL_SCISSOR_TEST);

    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glDisable(GL_SCISSOR_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::white);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::white);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::yellow);
}

// blit from system FBO to user-created framebuffer, with the scissor test enabled.
TEST_P(BlitFramebufferANGLETest, ReverseScissoredBlit)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.8f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mUserFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mOriginalFBO);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glScissor(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight());
    glEnable(GL_SCISSOR_TEST);

    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glDisable(GL_SCISSOR_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::white);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::white);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::yellow);
}

// blit from user-created FBO to system framebuffer, using region larger than buffer.
TEST_P(BlitFramebufferANGLETest, OversizedBlit)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.8f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBlitFramebufferANGLE(0, 0, getWindowWidth() * 2, getWindowHeight() * 2, 0, 0,
                           getWindowWidth() * 2, getWindowHeight() * 2, GL_COLOR_BUFFER_BIT,
                           GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::yellow);
}

// blit from system FBO to user-created framebuffer, using region larger than buffer.
TEST_P(BlitFramebufferANGLETest, ReverseOversizedBlit)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.8f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mUserFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mOriginalFBO);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBlitFramebufferANGLE(0, 0, getWindowWidth() * 2, getWindowHeight() * 2, 0, 0,
                           getWindowWidth() * 2, getWindowHeight() * 2, GL_COLOR_BUFFER_BIT,
                           GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::yellow);
}

// blit from user-created FBO to system framebuffer, with depth buffer.
TEST_P(BlitFramebufferANGLETest, BlitWithDepthUserToDefault)
{
    // TODO(http://anglebug.com/42264679): glBlitFramebufferANGLE() generates GL_INVALID_OPERATION
    // for the ES2_OpenGL backend.
    ANGLE_SKIP_TEST_IF(IsLinux() && IsIntel() && IsOpenGL());

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    EXPECT_GL_NO_ERROR();

    // Clear the first half of the screen
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, getWindowWidth(), getWindowHeight() / 2);

    glClearDepthf(0.1f);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Scissor the second half of the screen
    glScissor(0, getWindowHeight() / 2, getWindowWidth(), getWindowHeight() / 2);

    glClearDepthf(0.9f);
    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_SCISSOR_TEST);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);

    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                           GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    // if blit is happening correctly, this quad will draw only on the bottom half since it will
    // be behind on the first half and in front on the second half.
    drawQuad(mBlueProgram, essl1_shaders::PositionAttrib(), 0.5f);

    glDisable(GL_DEPTH_TEST);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::blue);
}

// blit from system FBO to user-created framebuffer, with depth buffer.
TEST_P(BlitFramebufferANGLETest, BlitWithDepthDefaultToUser)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    EXPECT_GL_NO_ERROR();

    // Clear the first half of the screen
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, getWindowWidth(), getWindowHeight() / 2);

    glClearDepthf(0.1f);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Scissor the second half of the screen
    glScissor(0, getWindowHeight() / 2, getWindowWidth(), getWindowHeight() / 2);

    glClearDepthf(0.9f);
    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_SCISSOR_TEST);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mUserFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mOriginalFBO);

    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                           GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    // if blit is happening correctly, this quad will draw only on the bottom half since it will be
    // behind on the first half and in front on the second half.
    drawQuad(mBlueProgram, essl1_shaders::PositionAttrib(), 0.5f);

    glDisable(GL_DEPTH_TEST);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::blue);
}

// blit from one region of the system fbo to another-- this should fail.
TEST_P(BlitFramebufferANGLETest, BlitSameBufferOriginal)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.3f);

    EXPECT_GL_NO_ERROR();

    glBlitFramebufferANGLE(0, 0, getWindowWidth() / 2, getWindowHeight(), getWindowWidth() / 2, 0,
                           getWindowWidth(), getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// blit from one region of the system fbo to another.
TEST_P(BlitFramebufferANGLETest, BlitSameBufferUser)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.3f);

    EXPECT_GL_NO_ERROR();

    glBlitFramebufferANGLE(0, 0, getWindowWidth() / 2, getWindowHeight(), getWindowWidth() / 2, 0,
                           getWindowWidth(), getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

TEST_P(BlitFramebufferANGLETest, BlitPartialColor)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBlitFramebufferANGLE(0, 0, getWindowWidth() / 2, getWindowHeight() / 2, 0,
                           getWindowHeight() / 2, getWindowWidth() / 2, getWindowHeight(),
                           GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mOriginalFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::white);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::white);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::white);
}

TEST_P(BlitFramebufferANGLETest, BlitDifferentSizes)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mSmallFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mSmallFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);

    EXPECT_GL_NO_ERROR();
}

// Test that blit with missing attachments is ignored.
TEST_P(BlitFramebufferANGLETest, BlitWithMissingAttachments)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mColorOnlyFBO);

    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.3f);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mColorOnlyFBO);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // No error if the read FBO has no depth attachment
    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                           GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    // No error if the read FBO has no stencil attachment
    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                           GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    // No error if we read from a missing color attachment.  Create a temp attachment as
    // attachment1, then remove attachment 0.
    //
    // The same could be done with glReadBuffer, which requires ES3 (this test runs on ES2).
    GLTexture tempColor;
    glBindTexture(GL_TEXTURE_2D, tempColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tempColor, 0);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_READ_FRAMEBUFFER);
    EXPECT_GL_NO_ERROR();

    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_NO_ERROR();
}

TEST_P(BlitFramebufferANGLETest, BlitStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    // http://anglebug.com/40096473
    ANGLE_SKIP_TEST_IF(IsIntel() && IsD3D9());

    // http://anglebug.com/42263934
    ANGLE_SKIP_TEST_IF(IsAMD() && IsD3D9());

    BlitStencilTestHelper(false /* mesaFlipY */);
}

// Same as BlitStencil, but with y-flip flag set.
TEST_P(BlitFramebufferANGLETest, BlitStencilWithMesaYFlip)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit") ||
                       !IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    // http://anglebug.com/40096473
    ANGLE_SKIP_TEST_IF(IsIntel() && IsD3D9());

    // http://anglebug.com/42263934
    ANGLE_SKIP_TEST_IF(IsAMD() && IsD3D9());

    BlitStencilTestHelper(true /* mesaFlipY */);
}

// make sure that attempting to blit a partial depth buffer issues an error
TEST_P(BlitFramebufferANGLETest, BlitPartialDepthStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);

    glBlitFramebufferANGLE(0, 0, getWindowWidth() / 2, getWindowHeight() / 2, 0, 0,
                           getWindowWidth() / 2, getWindowHeight() / 2, GL_DEPTH_BUFFER_BIT,
                           GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test blit with MRT framebuffers
TEST_P(BlitFramebufferANGLETest, BlitMRT)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    if (!IsGLExtensionEnabled("GL_EXT_draw_buffers"))
    {
        return;
    }

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT};

    glBindFramebuffer(GL_FRAMEBUFFER, mMRTFBO);
    glDrawBuffersEXT(2, drawBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, mColorOnlyFBO);

    glClear(GL_COLOR_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.8f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mColorOnlyFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mMRTFBO);

    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mMRTFBO);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::yellow);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mMRTColorBuffer0,
                           0);

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, getWindowHeight() / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, getWindowHeight() / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * getWindowWidth() / 4, 3 * getWindowHeight() / 4, GLColor::yellow);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mMRTColorBuffer0,
                           0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D,
                           mMRTColorBuffer1, 0);
}

// Test multisampled framebuffer blits if supported
TEST_P(BlitFramebufferANGLETest, MultisampledRGBAToRGBA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    if (!checkExtension("GL_ANGLE_framebuffer_multisample"))
        return;

    if (!checkExtension("GL_OES_rgb8_rgba8"))
        return;

    multisampleTestHelper(mRGBAMultisampledFBO, mRGBAFBO);
}

TEST_P(BlitFramebufferANGLETest, MultisampledRGBAToBGRA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    if (!checkExtension("GL_ANGLE_framebuffer_multisample"))
        return;

    if (!checkExtension("GL_OES_rgb8_rgba8"))
        return;

    if (!checkExtension("GL_EXT_texture_format_BGRA8888"))
        return;

    multisampleTestHelper(mRGBAMultisampledFBO, mBGRAFBO);
}

TEST_P(BlitFramebufferANGLETest, MultisampledBGRAToRGBA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    if (!checkExtension("GL_ANGLE_framebuffer_multisample"))
        return;

    if (!checkExtension("GL_OES_rgb8_rgba8"))
        return;

    if (!checkExtension("GL_EXT_texture_format_BGRA8888"))
        return;

    multisampleTestHelper(mBGRAMultisampledFBO, mRGBAFBO);
}

TEST_P(BlitFramebufferANGLETest, MultisampledBGRAToBGRA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    if (!checkExtension("GL_ANGLE_framebuffer_multisample"))
        return;

    if (!checkExtension("GL_OES_rgb8_rgba8"))
        return;

    if (!checkExtension("GL_EXT_texture_format_BGRA8888"))
        return;

    multisampleTestHelper(mBGRAMultisampledFBO, mBGRAFBO);
}

// Make sure that attempts to stretch in a blit call issue an error
TEST_P(BlitFramebufferANGLETest, ErrorStretching)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);

    glBlitFramebufferANGLE(0, 0, getWindowWidth() / 2, getWindowHeight() / 2, 0, 0,
                           getWindowWidth(), getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Make sure that attempts to flip in a blit call issue an error
TEST_P(BlitFramebufferANGLETest, ErrorFlipping)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);

    glBlitFramebufferANGLE(0, 0, getWindowWidth() / 2, getWindowHeight() / 2, getWindowWidth() / 2,
                           getWindowHeight() / 2, 0, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

TEST_P(BlitFramebufferANGLETest, Errors)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_framebuffer_blit"));

    glBindFramebuffer(GL_FRAMEBUFFER, mUserFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawQuad(mCheckerProgram, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mOriginalFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mUserFBO);

    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_LINEAR);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT | 234, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mDiffFormatFBO);

    glBlitFramebufferANGLE(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                           getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// TODO(geofflang): Fix the dependence on glBlitFramebufferANGLE without checks and assuming the
// default framebuffer is BGRA to enable the GL and GLES backends. (http://anglebug.com/42260299)

class BlitFramebufferTest : public ANGLETest<>
{
  protected:
    BlitFramebufferTest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    void initColorFBO(GLFramebuffer *fbo,
                      GLRenderbuffer *rbo,
                      GLenum rboFormat,
                      GLsizei width,
                      GLsizei height)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, *rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, rboFormat, width, height);

        glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *rbo);
    }

    void initColorFBOWithCheckerPattern(GLFramebuffer *fbo,
                                        GLRenderbuffer *rbo,
                                        GLenum rboFormat,
                                        GLsizei width,
                                        GLsizei height)
    {
        initColorFBO(fbo, rbo, rboFormat, width, height);

        ANGLE_GL_PROGRAM(checkerProgram, essl1_shaders::vs::Passthrough(),
                         essl1_shaders::fs::Checkered());
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
        drawQuad(checkerProgram, essl1_shaders::PositionAttrib(), 0.5f);
    }

    void BlitDepthStencilPixelByPixelTestHelper(bool mesaYFlip)
    {
        if (mesaYFlip)
            ASSERT_TRUE(IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

        ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());

        glViewport(0, 0, 128, 1);
        glEnable(GL_DEPTH_TEST);

        GLFramebuffer srcFramebuffer;
        GLRenderbuffer srcRenderbuffer;
        glBindFramebuffer(GL_FRAMEBUFFER, srcFramebuffer);
        if (mesaYFlip)
            glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
        glBindRenderbuffer(GL_RENDERBUFFER, srcRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 128, 1);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  srcRenderbuffer);
        glClearDepthf(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.0f, 0.5f);
        glViewport(0, 0, 256, 2);

        GLFramebuffer dstFramebuffer;
        GLRenderbuffer dstRenderbuffer;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFramebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, dstRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 256, 2);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  dstRenderbuffer);

        GLTexture dstColor;
        glBindTexture(GL_TEXTURE_2D, dstColor);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 256, 2);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstColor, 0);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFramebuffer);
        glBlitFramebuffer(0, 0, 128, 1, 0, 0, 256, 2, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                          GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDepthMask(false);
        glDepthFunc(GL_LESS);
        drawQuad(drawRed, essl1_shaders::PositionAttrib(), -0.01f, 0.5f);
        EXPECT_PIXEL_RECT_EQ(64, 0, 128, 1, GLColor::red);

        ANGLE_GL_PROGRAM(drawBlue, essl3_shaders::vs::Simple(), essl3_shaders::fs::Blue());
        glEnable(GL_DEPTH_TEST);
        glDepthMask(false);
        glDepthFunc(GL_GREATER);
        if (mesaYFlip)
            glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
        drawQuad(drawBlue, essl1_shaders::PositionAttrib(), 0.01f, 0.5f);
        if (mesaYFlip)
            EXPECT_PIXEL_RECT_EQ(64, 0, 128, 1, GLColor::green);
        else
            EXPECT_PIXEL_RECT_EQ(64, 0, 128, 1, GLColor::blue);
    }

    // Test blitting between 3D textures and 2D array textures
    void test3DBlit(GLenum sourceTarget, GLenum destTarget)
    {

        constexpr int kTexWidth  = 4;
        constexpr int kTexHeight = 3;
        constexpr int kTexDepth  = 2;
        glViewport(0, 0, kTexWidth, kTexHeight);

        size_t size = kTexWidth * kTexHeight * kTexDepth;
        std::vector<uint32_t> sourceData(size);
        std::vector<uint32_t> destData(size);
        for (size_t i = 0; i < size; ++i)
        {
            sourceData[i] = i;
            destData[i]   = size - i;
        }

        // Create a source 3D texture and FBO.
        GLTexture sourceTexture;
        glBindTexture(sourceTarget, sourceTexture);
        glTexImage3D(sourceTarget, 0, GL_RGBA8, kTexWidth, kTexHeight, kTexDepth, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, sourceData.data());

        // Create a dest texture and FBO.
        GLTexture destTexture;
        glBindTexture(destTarget, destTexture);
        glTexImage3D(destTarget, 0, GL_RGBA8, kTexWidth, kTexHeight, kTexDepth, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, destData.data());

        for (int z = 0; z < kTexDepth; ++z)
        {
            ASSERT_GL_NO_ERROR();
            GLFramebuffer sourceFBO;
            glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
            glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sourceTexture, 0,
                                      z);
            ASSERT_GL_NO_ERROR();

            GLFramebuffer destFBO;
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFBO);
            glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, destTexture, 0, z);
            ASSERT_GL_NO_ERROR();

            glBlitFramebuffer(0, 0, kTexWidth, kTexHeight, 0, 0, kTexWidth, kTexHeight,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
            ASSERT_GL_NO_ERROR();
        }

        for (int z = 0; z < kTexDepth; ++z)
        {
            GLFramebuffer readFBO;
            glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
            glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, destTexture, 0, z);
            ASSERT_GL_NO_ERROR();

            glReadBuffer(GL_COLOR_ATTACHMENT0);
            for (int y = 0; y < kTexHeight; ++y)
            {
                for (int x = 0; x < kTexWidth; ++x)
                {
                    int index = x + kTexWidth * (y + z * kTexHeight);
                    EXPECT_PIXEL_COLOR_EQ(x, y, index);
                }
            }
        }
    }

    void initFBOWithProgramAndDepth(GLFramebuffer *fbo,
                                    GLRenderbuffer *colorRenderBuffer,
                                    GLenum colorFormat,
                                    GLRenderbuffer *depthRenderBuffer,
                                    GLenum depthFormat,
                                    GLsizei width,
                                    GLsizei height,
                                    GLuint program,
                                    float depthValue)
    {
        if (fbo != nullptr)
        {
            // Create renderbuffer
            glBindRenderbuffer(GL_RENDERBUFFER, *colorRenderBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, colorFormat, width, height);
            glBindRenderbuffer(GL_RENDERBUFFER, *depthRenderBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, depthFormat, width, height);

            // Create fbo
            glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                      *colorRenderBuffer);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                      *depthRenderBuffer);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // draw with program
        glUseProgram(program);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepthf(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(true);
        drawQuad(program, essl1_shaders::PositionAttrib(), depthValue);
    }

    void drawWithDepthValue(std::array<Vector3, 6> &quadVertices, float depth)
    {
        for (Vector3 &vertice : quadVertices)
        {
            vertice[2] = depth;
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadVertices[0]) * quadVertices.size(),
                        quadVertices.data());
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};

class BlitFramebufferTestES31 : public BlitFramebufferTest
{};

// Tests resolving a multisample depth buffer.
TEST_P(BlitFramebufferTest, MultisampleDepth)
{
    // TODO(oetuaho@nvidia.com): http://crbug.com/837717
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsMac());

    GLRenderbuffer renderbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuf);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_DEPTH_COMPONENT24, 256, 256);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuf);

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glClearDepthf(0.5f);
    glClear(GL_DEPTH_BUFFER_BIT);

    GLRenderbuffer destRenderbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, destRenderbuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 256, 256);

    GLFramebuffer resolved;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolved);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                              destRenderbuf);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glBlitFramebuffer(0, 0, 256, 256, 0, 0, 256, 256, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, resolved);

    // Immediately destroy the framebuffer and the associated textures for additional cleanup
    // ordering testing.
    framebuffer.reset();
    renderbuf.reset();

    GLTexture colorbuf;
    glBindTexture(GL_TEXTURE_2D, colorbuf);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 256, 256);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorbuf, 0);

    ASSERT_GL_NO_ERROR();

    // Clear to green
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Make sure resulting depth is near 0.5f.
    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glEnable(GL_DEPTH_TEST);
    glDepthMask(false);
    glDepthFunc(GL_LESS);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), -0.01f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(255, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, 255, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(255, 255, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(127, 127, GLColor::red);

    ANGLE_GL_PROGRAM(drawBlue, essl3_shaders::vs::Simple(), essl3_shaders::fs::Blue());
    glEnable(GL_DEPTH_TEST);
    glDepthMask(false);
    glDepthFunc(GL_GREATER);
    drawQuad(drawBlue, essl3_shaders::PositionAttrib(), 0.01f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(255, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, 255, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(255, 255, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(127, 127, GLColor::blue);

    ASSERT_GL_NO_ERROR();
}

// Blit multisample stencil buffer to default framebuffer without prerotaion.
TEST_P(BlitFramebufferTest, BlitMultisampleStencilToDefault)
{
    // http://anglebug.com/42262159
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsMac());

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    GLRenderbuffer colorbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuf);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, 128, 128);

    GLRenderbuffer depthstencilbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, depthstencilbuf);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, 128, 128);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                              depthstencilbuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthstencilbuf);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glFlush();

    // Replace stencil to 1.
    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 255);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.8f);

    // Blit multisample stencil buffer to default frambuffer.
    GLenum attachments1[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments1);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, 128, 128, 0, 0, 128, 128, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                      GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // Disable stencil and draw full_screen green color.
    ANGLE_GL_PROGRAM(drawGreen, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());
    glDisable(GL_STENCIL_TEST);
    drawQuad(drawGreen, essl3_shaders::PositionAttrib(), 0.5f);

    // Draw blue color if the stencil is equal to 1.
    // If the blit finished successfully, the stencil test should all pass.
    ANGLE_GL_PROGRAM(drawBlue, essl3_shaders::vs::Simple(), essl3_shaders::fs::Blue());
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, 255);
    drawQuad(drawBlue, essl3_shaders::PositionAttrib(), 0.2f);

    // Check the result, especially the boundaries.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(127, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(50, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(127, 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, 127, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(127, 127, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(64, 64, GLColor::blue);

    ASSERT_GL_NO_ERROR();
}

// Test blit multisampled framebuffer to MRT framebuffer
TEST_P(BlitFramebufferTest, BlitMultisampledFramebufferToMRT)
{
    // https://issues.angleproject.org/issues/361369302

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Prepare multisampled framebuffer to blit from.
    GLRenderbuffer multiSampleColorbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, multiSampleColorbuf);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, getWindowWidth(),
                                     getWindowHeight());

    GLFramebuffer multiSampleFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, multiSampleFramebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              multiSampleColorbuf);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.8f);
    EXPECT_GL_NO_ERROR();

    // Prepare mrt framebuffer with two attachments to blit to.
    GLFramebuffer MRTFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, MRTFBO);
    GLTexture MRTColorBuffer0;
    GLTexture MRTColorBuffer1;
    glBindTexture(GL_TEXTURE_2D, MRTColorBuffer0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, MRTColorBuffer0, 0);
    glBindTexture(GL_TEXTURE_2D, MRTColorBuffer1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, MRTColorBuffer1, 0);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, multiSampleFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, MRTFBO);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, drawBuffers);

    glBlitFramebuffer(0, 0, getWindowWidth(), getWindowHeight(), 0, 0, getWindowWidth(),
                      getWindowHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    // Check results
    glBindFramebuffer(GL_FRAMEBUFFER, MRTFBO);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
}

// Tests clearing a multisampled depth buffer.
TEST_P(BlitFramebufferTest, MultisampleDepthClear)
{
    // http://anglebug.com/40096654
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    GLRenderbuffer depthMS;
    glBindRenderbuffer(GL_RENDERBUFFER, depthMS);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_DEPTH_COMPONENT24, 256, 256);

    GLRenderbuffer colorMS;
    glBindRenderbuffer(GL_RENDERBUFFER, colorMS);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_RGBA8, 256, 256);

    GLRenderbuffer colorResolved;
    glBindRenderbuffer(GL_RENDERBUFFER, colorResolved);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 256, 256);

    GLFramebuffer framebufferMS;
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferMS);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthMS);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorMS);

    // Clear depth buffer to 0.5 and color to green.
    glClearDepthf(0.5f);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glFlush();

    // Draw red into the multisampled color buffer.
    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_EQUAL);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.0f);

    // Resolve the color buffer to make sure the above draw worked correctly, which in turn implies
    // that the multisampled depth clear worked.
    GLFramebuffer framebufferResolved;
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferResolved);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorResolved);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferMS);
    glBlitFramebuffer(0, 0, 256, 256, 0, 0, 256, 256, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, framebufferResolved);

    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(255, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, 255, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(255, 255, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(127, 127, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Tests clearing a multisampled depth buffer with a glFenceSync in between.
TEST_P(BlitFramebufferTest, MultisampleDepthClearWithFenceSync)
{
    // http://anglebug.com/40096654
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    GLRenderbuffer depthMS;
    glBindRenderbuffer(GL_RENDERBUFFER, depthMS);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_DEPTH_COMPONENT24, 256, 256);

    GLRenderbuffer colorMS;
    glBindRenderbuffer(GL_RENDERBUFFER, colorMS);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_RGBA8, 256, 256);

    GLRenderbuffer colorResolved;
    glBindRenderbuffer(GL_RENDERBUFFER, colorResolved);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 256, 256);

    GLFramebuffer framebufferMS;
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferMS);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthMS);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorMS);

    // Clear depth buffer to 0.5 and color to green.
    glClearDepthf(0.5f);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glFlush();

    // Draw red into the multisampled color buffer.
    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_EQUAL);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.0f);

    // This should trigger a deferred renderPass end
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    EXPECT_GL_NO_ERROR();

    // Resolve the color buffer to make sure the above draw worked correctly, which in turn implies
    // that the multisampled depth clear worked.
    GLFramebuffer framebufferResolved;
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferResolved);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorResolved);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferMS);
    glBlitFramebuffer(0, 0, 256, 256, 0, 0, 256, 256, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, framebufferResolved);

    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(255, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, 255, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(255, 255, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(127, 127, GLColor::red);

    glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    ASSERT_GL_NO_ERROR();
}

// Test resolving a multisampled stencil buffer.
TEST_P(BlitFramebufferTest, MultisampleStencil)
{
    GLRenderbuffer renderbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuf);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_STENCIL_INDEX8, 256, 256);

    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuf);

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // fill the stencil buffer with 0x1
    glStencilFunc(GL_ALWAYS, 0x1, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glEnable(GL_STENCIL_TEST);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);

    GLTexture destColorbuf;
    glBindTexture(GL_TEXTURE_2D, destColorbuf);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 256, 256);

    GLRenderbuffer destRenderbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, destRenderbuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, 256, 256);

    GLFramebuffer resolved;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolved);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, destColorbuf,
                           0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              destRenderbuf);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glBlitFramebuffer(0, 0, 256, 256, 0, 0, 256, 256, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    // Immediately destroy the framebuffer and the associated textures for additional cleanup
    // ordering testing.
    framebuffer.reset();
    renderbuf.reset();

    glBindFramebuffer(GL_FRAMEBUFFER, resolved);

    ASSERT_GL_NO_ERROR();

    // Clear to green
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Draw red if the stencil is 0x1, which should be true after the resolve.
    glStencilFunc(GL_EQUAL, 0x1, 0xFF);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test resolving a multisampled stencil buffer with scissor.
TEST_P(BlitFramebufferTest, ScissoredMultisampleStencil)
{
    constexpr GLuint kSize = 256;

    // Create the resolve framebuffer.
    GLTexture destColorbuf;
    glBindTexture(GL_TEXTURE_2D, destColorbuf);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);

    GLRenderbuffer destRenderbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, destRenderbuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, kSize, kSize);

    GLFramebuffer resolved;
    glBindFramebuffer(GL_FRAMEBUFFER, resolved);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, destColorbuf,
                           0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              destRenderbuf);

    // Clear the resolved buffer with gray and 0x10 stencil.
    GLColor gray(127, 127, 127, 255);
    glClearStencil(0x10);
    glClearColor(0.499f, 0.499f, 0.499f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, gray);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, gray);

    // Create the multisampled framebuffer.
    GLRenderbuffer renderbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuf);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_STENCIL_INDEX8, kSize, kSize);

    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    ANGLE_GL_PROGRAM(drawGreen, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());
    ANGLE_GL_PROGRAM(drawBlue, essl3_shaders::vs::Simple(), essl3_shaders::fs::Blue());

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuf);

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Fill the stencil buffer with 0x1.
    glClearStencil(0x1);
    glClear(GL_STENCIL_BUFFER_BIT);

    // Fill a smaller region of the buffer with 0x2.
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_STENCIL_TEST);
    glScissor(kSize / 4, kSize / 4, kSize / 2, kSize / 2);
    glStencilFunc(GL_ALWAYS, 0x2, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);

    // Blit into the resolved framebuffer (with scissor still enabled).
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolved);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, resolved);

    ASSERT_GL_NO_ERROR();

    // Draw blue if the stencil is 0x1, which should never be true.
    glDisable(GL_SCISSOR_TEST);
    glStencilMask(0);
    glStencilFunc(GL_EQUAL, 0x1, 0xFF);
    drawQuad(drawBlue, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, gray);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, gray);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, gray);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, gray);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, gray);

    // Draw red if the stencil is 0x2, which should be true in the middle after the blit/resolve.
    glStencilFunc(GL_EQUAL, 0x2, 0xFF);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, gray);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, gray);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, gray);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, gray);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::red);

    // Draw green if the stencil is 0x10, which should be left untouched in the outer regions.
    glStencilFunc(GL_EQUAL, 0x10, 0xFF);
    drawQuad(drawGreen, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test blitting from a texture with non-zero base.  The blit is non-stretching and between
// identical formats so that the path that uses vkCmdBlitImage is taken.
TEST_P(BlitFramebufferTest, NonZeroBaseSource)
{
    // http://anglebug.com/40644751
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsMac());

    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());

    // Create a framebuffer for source data.  It usea a non-zero base.
    GLTexture srcColor;
    glBindTexture(GL_TEXTURE_2D, srcColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);

    GLFramebuffer srcFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, srcFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcColor, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // fill the color buffer with red.
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);

    // Create a framebuffer for blit destination.
    GLTexture dstColor;
    glBindTexture(GL_TEXTURE_2D, dstColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer dstFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstColor, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Blit.  Note: no stretching is done so that vkCmdBlitImage can be used.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFramebuffer);
    glBlitFramebuffer(0, 0, 256, 256, 0, 0, 256, 256, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);

    ASSERT_GL_NO_ERROR();

    // Make sure the blit is done.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test blitting to a texture with non-zero base.  The blit is non-stretching and between
// identical formats so that the path that uses vkCmdBlitImage is taken.
TEST_P(BlitFramebufferTest, NonZeroBaseDestination)
{
    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());

    // Create a framebuffer for source data.  It usea a non-zero base.
    GLTexture srcColor;
    glBindTexture(GL_TEXTURE_2D, srcColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer srcFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, srcFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcColor, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // fill the color buffer with red.
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);

    // Create a framebuffer for blit destination.
    GLTexture dstColor;
    glBindTexture(GL_TEXTURE_2D, dstColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);

    GLFramebuffer dstFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstColor, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Blit.  Note: no stretching is done so that vkCmdBlitImage can be used.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFramebuffer);
    glBlitFramebuffer(0, 0, 256, 256, 0, 0, 256, 256, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);

    ASSERT_GL_NO_ERROR();

    // Make sure the blit is done.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test blitting from a stencil buffer with non-zero base.
TEST_P(BlitFramebufferTest, NonZeroBaseSourceStencil)
{
    // http://anglebug.com/40644751
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsMac());

    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());

    // Create a framebuffer with an attachment that has non-zero base
    GLTexture stencilTexture;
    glBindTexture(GL_TEXTURE_2D, stencilTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 256, 256, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, 256, 256, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);

    GLFramebuffer srcFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, srcFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilTexture, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // fill the stencil buffer with 0x1
    glStencilFunc(GL_ALWAYS, 0x1, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glEnable(GL_STENCIL_TEST);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);

    // Create a framebuffer with an attachment that has non-zero base
    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 256, 256);

    GLRenderbuffer renderbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 256, 256);

    GLFramebuffer dstFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuf);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Blit stencil.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFramebuffer);
    glBlitFramebuffer(0, 0, 256, 256, 0, 0, 256, 256, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);

    ASSERT_GL_NO_ERROR();

    // Clear to green
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Draw red if the stencil is 0x1, which should be true after the blit.
    glStencilFunc(GL_EQUAL, 0x1, 0xFF);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test blitting to a stencil buffer with non-zero base.
TEST_P(BlitFramebufferTest, NonZeroBaseDestinationStencil)
{
    // http://anglebug.com/40644751
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsMac());

    // http://anglebug.com/42263576
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsWindows());

    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());

    // Create a framebuffer for source data.
    GLRenderbuffer renderbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 256, 256);

    GLFramebuffer srcFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, srcFramebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuf);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // fill the stencil buffer with 0x1
    glStencilFunc(GL_ALWAYS, 0x1, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glEnable(GL_STENCIL_TEST);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);

    // Create a framebuffer with an attachment that has non-zero base
    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 256, 256);

    GLTexture stencilTexture;
    glBindTexture(GL_TEXTURE_2D, stencilTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 256, 256, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, 256, 256, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);

    GLFramebuffer dstFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilTexture, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Blit stencil.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFramebuffer);
    glBlitFramebuffer(0, 0, 256, 256, 0, 0, 256, 256, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);

    ASSERT_GL_NO_ERROR();

    // Clear to green
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Draw red if the stencil is 0x1, which should be true after the blit.
    glStencilFunc(GL_EQUAL, 0x1, 0xFF);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Test blitting to a stencil buffer with non-zero base.  Exercises the compute path in the Vulkan
// backend if stencil export is not supported.  The blit is not 1-to-1 for this path to be taken.
TEST_P(BlitFramebufferTest, NonZeroBaseDestinationStencilStretch)
{
    // http://anglebug.com/40644750
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsWindows());

    // http://anglebug.com/40644751
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsMac());

    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());

    // Create a framebuffer for source data.
    GLRenderbuffer renderbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 256, 256);

    GLFramebuffer srcFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, srcFramebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuf);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // fill the stencil buffer with 0x1
    glStencilFunc(GL_ALWAYS, 0x1, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glEnable(GL_STENCIL_TEST);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);

    // Create a framebuffer with an attachment that has non-zero base
    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 256, 256);

    GLTexture stencilTexture;
    glBindTexture(GL_TEXTURE_2D, stencilTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 256, 256, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, 256, 256, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);

    GLFramebuffer dstFramebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, stencilTexture, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Blit stencil.  Note: stretch is intentional so vkCmdBlitImage cannot be used in the Vulkan
    // backend.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFramebuffer);
    glBlitFramebuffer(0, 0, 256, 256, -256, -256, 512, 512, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, dstFramebuffer);

    ASSERT_GL_NO_ERROR();

    // Clear to green
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Draw red if the stencil is 0x1, which should be true after the blit.
    glStencilFunc(GL_EQUAL, 0x1, 0xFF);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Blit an SRGB framebuffer and scale it.
TEST_P(BlitFramebufferTest, BlitSRGBToRGBAndScale)
{
    constexpr const GLsizei kWidth  = 256;
    constexpr const GLsizei kHeight = 256;

    GLRenderbuffer sourceRBO, targetRBO;
    GLFramebuffer sourceFBO, targetFBO;
    initColorFBOWithCheckerPattern(&sourceFBO, &sourceRBO, GL_SRGB8_ALPHA8, kWidth * 2,
                                   kHeight * 2);
    initColorFBO(&targetFBO, &targetRBO, GL_RGBA8, kWidth, kHeight);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

    glViewport(0, 0, kWidth, kHeight);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Scale down without flipping.
    glBlitFramebuffer(0, 0, kWidth * 2, kHeight * 2, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, kHeight / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, 3 * kHeight / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, kHeight / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, 3 * kHeight / 4, GLColor::yellow);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Scale down and flip in the X direction.
    glBlitFramebuffer(0, 0, kWidth * 2, kHeight * 2, kWidth, 0, 0, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, kHeight / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, 3 * kHeight / 4, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, kHeight / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, 3 * kHeight / 4, GLColor::green);
}

// Blit stencil, with scissor and scale it.
TEST_P(BlitFramebufferTest, BlitStencilScissoredScaled)
{
    constexpr GLint kSize = 256;

    // Create the destination framebuffer.
    GLTexture destColorbuf;
    glBindTexture(GL_TEXTURE_2D, destColorbuf);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);

    GLRenderbuffer destRenderbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, destRenderbuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, kSize, kSize);

    GLFramebuffer destFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, destFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, destColorbuf,
                           0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              destRenderbuf);

    // Clear the destination buffer with gray and 0x10 stencil.
    GLColor gray(127, 127, 127, 255);
    glClearStencil(0x10);
    glClearColor(0.499f, 0.499f, 0.499f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, gray);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, gray);

    // Create the source framebuffer.
    GLRenderbuffer renderbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, kSize, kSize);

    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    ANGLE_GL_PROGRAM(drawGreen, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());
    ANGLE_GL_PROGRAM(drawBlue, essl3_shaders::vs::Simple(), essl3_shaders::fs::Blue());

    GLFramebuffer sourceFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, sourceFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuf);

    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Fill the stencil buffer with 0x1.
    glClearStencil(0x1);
    glClear(GL_STENCIL_BUFFER_BIT);

    // Fill a smaller region of the buffer with 0x2.
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_STENCIL_TEST);
    glScissor(kSize / 4, kSize / 4, kSize / 2, kSize / 2);
    glStencilFunc(GL_ALWAYS, 0x2, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);

    // Blit and scale down into the destination framebuffer (with scissor still enabled).
    //
    // Source looks like this:
    //
    //     +----|----|----|----+
    //     |                   |
    //     |       0x1         |
    //     -    +---------+    -
    //     |    |         |    |
    //     |    |         |    |
    //     -    |   0x2   |    -
    //     |    |         |    |
    //     |    |         |    |
    //     -    +---------+    -
    //     |                   |
    //     |                   |
    //     +----|----|----|----+
    //
    // We want the destination to look like this:
    //
    //     +----|----|----|----+
    //     |                   |
    //     |       0x10        |
    //     -    +---------+    -
    //     |    |  0x1    |    |
    //     |    |  +------+    |
    //     -    |  |      |    -
    //     |    |  | 0x2  |    |
    //     |    |  |      |    |
    //     -    +--+------+    -
    //     |                   |
    //     |                   |
    //     +----|----|----|----+
    //
    // The corresponding blit would be: (0, 0, 3/4, 3/4) -> (1/4, 1/4, 3/4, 3/4).  For testing, we
    // would like to avoid having the destination area and scissor to match.  Using destination
    // area as (0, 0, 1, 1), and keeping the same scaling, the source area should be
    // (-3/8, -3/8, 9/8, 9/8).
    //
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFBO);
    constexpr GLint kBlitSrc[2] = {-3 * kSize / 8, 9 * kSize / 8};
    glBlitFramebuffer(kBlitSrc[0], kBlitSrc[0], kBlitSrc[1], kBlitSrc[1], 0, 0, kSize, kSize,
                      GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, destFBO);

    ASSERT_GL_NO_ERROR();

    // Draw blue if the stencil is 0x1, which should be true only in the top and left of the inner
    // square.
    glDisable(GL_SCISSOR_TEST);
    glStencilMask(0);
    glStencilFunc(GL_EQUAL, 0x1, 0xFF);
    drawQuad(drawBlue, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, gray);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, gray);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, gray);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, gray);

    EXPECT_PIXEL_COLOR_EQ(kSize / 4, kSize / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize / 4, 3 * kSize / 4 - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * kSize / 4 - 1, kSize / 4, GLColor::blue);

    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, gray);
    EXPECT_PIXEL_COLOR_EQ(3 * kSize / 4 - 1, 3 * kSize / 4 - 1, gray);

    // Draw red if the stencil is 0x2, which should be true in the bottom/right of the middle
    // square after the blit.
    glStencilFunc(GL_EQUAL, 0x2, 0xFF);
    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, gray);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, gray);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, gray);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, gray);

    EXPECT_PIXEL_COLOR_EQ(kSize / 4, kSize / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize / 4, 3 * kSize / 4 - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * kSize / 4 - 1, kSize / 4, GLColor::blue);

    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * kSize / 4 - 1, 3 * kSize / 4 - 1, GLColor::red);

    // Draw green if the stencil is 0x10, which should be left untouched in the outer regions.
    glStencilFunc(GL_EQUAL, 0x10, 0xFF);
    drawQuad(drawGreen, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::green);

    EXPECT_PIXEL_COLOR_EQ(kSize / 4, kSize / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize / 4, 3 * kSize / 4 - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * kSize / 4 - 1, kSize / 4, GLColor::blue);

    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * kSize / 4 - 1, 3 * kSize / 4 - 1, GLColor::red);

    ASSERT_GL_NO_ERROR();
}

// Blit a subregion of an SRGB framebuffer to an RGB framebuffer.
TEST_P(BlitFramebufferTest, PartialBlitSRGBToRGB)
{
    constexpr const GLsizei kWidth  = 256;
    constexpr const GLsizei kHeight = 256;

    GLRenderbuffer sourceRBO, targetRBO;
    GLFramebuffer sourceFBO, targetFBO;
    initColorFBOWithCheckerPattern(&sourceFBO, &sourceRBO, GL_SRGB8_ALPHA8, kWidth * 2,
                                   kHeight * 2);
    initColorFBO(&targetFBO, &targetRBO, GL_RGBA8, kWidth, kHeight);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

    glViewport(0, 0, kWidth, kHeight);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Blit a part of the source FBO without flipping.
    glBlitFramebuffer(kWidth, kHeight, kWidth * 2, kHeight * 2, 0, 0, kWidth, kHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, kHeight / 4, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, 3 * kHeight / 4, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, kHeight / 4, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, 3 * kHeight / 4, GLColor::yellow);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Blit a part of the source FBO and flip in the X direction.
    glBlitFramebuffer(kWidth * 2, 0, kWidth, kHeight, kWidth, 0, 0, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, kHeight / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, 3 * kHeight / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, kHeight / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, 3 * kHeight / 4, GLColor::blue);
}

// Blit an SRGB framebuffer with an oversized source area (parts outside the source area should be
// clipped out).
TEST_P(BlitFramebufferTest, BlitSRGBToRGBOversizedSourceArea)
{
    constexpr const GLsizei kWidth  = 256;
    constexpr const GLsizei kHeight = 256;

    GLRenderbuffer sourceRBO, targetRBO;
    GLFramebuffer sourceFBO, targetFBO;
    initColorFBOWithCheckerPattern(&sourceFBO, &sourceRBO, GL_SRGB8_ALPHA8, kWidth, kHeight);
    initColorFBO(&targetFBO, &targetRBO, GL_RGBA8, kWidth, kHeight);

    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kWidth, kHeight);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Blit so that the source area gets placed at the center of the target FBO.
    // The width of the source area is 1/4 of the width of the target FBO.
    glBlitFramebuffer(-3 * kWidth / 2, -3 * kHeight / 2, 5 * kWidth / 2, 5 * kHeight / 2, 0, 0,
                      kWidth, kHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    // Source FBO colors can be found in the middle of the target FBO.
    EXPECT_PIXEL_COLOR_EQ(7 * kWidth / 16, 7 * kHeight / 16, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(7 * kWidth / 16, 9 * kHeight / 16, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(9 * kWidth / 16, 7 * kHeight / 16, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(9 * kWidth / 16, 9 * kHeight / 16, GLColor::yellow);

    // Clear color should remain around the edges of the target FBO (WebGL 2.0 spec explicitly
    // requires this and ANGLE is expected to follow that).
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, kHeight / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, 3 * kHeight / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, kHeight / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, 3 * kHeight / 4, GLColor::blue);
}

// Blit an SRGB framebuffer with an oversized dest area (even though the result is clipped, it
// should be scaled as if the whole dest area was used).
TEST_P(BlitFramebufferTest, BlitSRGBToRGBOversizedDestArea)
{
    constexpr const GLsizei kWidth  = 256;
    constexpr const GLsizei kHeight = 256;

    GLRenderbuffer sourceRBO, targetRBO;
    GLFramebuffer sourceFBO, targetFBO;
    initColorFBOWithCheckerPattern(&sourceFBO, &sourceRBO, GL_SRGB8_ALPHA8, kWidth, kHeight);
    initColorFBO(&targetFBO, &targetRBO, GL_RGBA8, kWidth, kHeight);

    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kWidth, kHeight);

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Dest is oversized but centered the same as source
    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

    glBlitFramebuffer(0, 0, kWidth, kHeight, -kWidth / 2, -kHeight / 2, 3 * kWidth / 2,
                      3 * kHeight / 2, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    // Expected result:
    //
    //     +-------+-------+
    //     |       |       |
    //     |   R   |   B   |
    //     |       |       |
    //     +-------+-------+
    //     |       |       |
    //     |   G   |   Y   |
    //     |       |       |
    //     +-------+-------+
    //
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, kHeight / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2 - 1, kHeight / 2 - 1, GLColor::red);

    EXPECT_PIXEL_COLOR_EQ(1, kWidth - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, 3 * kHeight / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2 - 1, kHeight / 2 + 1, GLColor::green);

    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, kHeight / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2 + 1, kHeight / 2 - 1, GLColor::blue);

    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, 3 * kHeight / 4, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2 + 1, kHeight / 2 + 1, GLColor::yellow);

    // Dest is oversized in the negative direction
    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

    glBlitFramebuffer(0, 0, kWidth, kHeight, -kWidth / 2, -kHeight / 2, kWidth, kHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    // Expected result:
    //
    //     Width / 4
    //         |
    //         V
    //     +---+-----------+
    //     | R |     B     |
    //     +---+-----------+ <- Height / 4
    //     |   |           |
    //     |   |           |
    //     | G |     Y     |
    //     |   |           |
    //     |   |           |
    //     +---+-----------+
    //
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight / 4 - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4 - 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4 - 1, kHeight / 4 - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 8, kHeight / 8, GLColor::red);

    EXPECT_PIXEL_COLOR_EQ(0, kHeight / 4 + 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4 - 1, kHeight / 4 + 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4 - 1, kHeight - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 8, kHeight / 2, GLColor::green);

    EXPECT_PIXEL_COLOR_EQ(kWidth / 4 + 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4 + 1, kHeight / 4 - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight / 4 - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 8, GLColor::blue);

    EXPECT_PIXEL_COLOR_EQ(kWidth / 4 + 1, kHeight / 4 + 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4 + 1, kHeight - 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight / 4 + 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::yellow);

    // Dest is oversized in the positive direction
    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);

    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, 3 * kWidth / 2, 3 * kHeight / 2,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    // Expected result:
    //
    //           3 * Width / 4
    //                 |
    //                 V
    //     +-----------+---+
    //     |           |   |
    //     |           |   |
    //     |     R     | B |
    //     |           |   |
    //     |           |   |
    //     +-----------+---+ <- 3 * Height / 4
    //     |     G     | Y |
    //     +-----------+---+
    //
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, 3 * kHeight / 4 - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4 - 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4 - 1, 3 * kHeight / 4 - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::red);

    EXPECT_PIXEL_COLOR_EQ(0, 3 * kHeight / 4 + 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4 - 1, 3 * kHeight / 4 + 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4 - 1, kHeight - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, 7 * kHeight / 8, GLColor::green);

    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4 + 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4 + 1, 3 * kHeight / 4 - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 3 * kHeight / 4 - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(7 * kWidth / 8, kHeight / 2, GLColor::blue);

    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4 + 1, 3 * kHeight / 4 + 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4 + 1, kHeight - 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 3 * kHeight / 4 + 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(7 * kWidth / 8, 7 * kHeight / 8, GLColor::yellow);
}

// This test is to demonstrate a bug that when a program is created and used and then destroyed, we
// should not have a dangling PipelineHelper pointer in the context point to the already destroyed
// object.
TEST_P(BlitFramebufferTest, useAndDestroyProgramThenBlit)
{
    constexpr const GLsizei kWidth  = 256;
    constexpr const GLsizei kHeight = 256;

    GLRenderbuffer sourceRBO, targetRBO;
    GLFramebuffer sourceFBO, targetFBO;

    {
        initColorFBO(&sourceFBO, &sourceRBO, GL_SRGB8_ALPHA8, kWidth, kHeight);
        // checkerProgram will be created and destroyed in this code block
        ANGLE_GL_PROGRAM(checkerProgram, essl1_shaders::vs::Passthrough(),
                         essl1_shaders::fs::Checkered());
        glViewport(0, 0, kWidth, kHeight);
        glBindFramebuffer(GL_FRAMEBUFFER, sourceFBO);
        drawQuad(checkerProgram, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
    }
    initColorFBO(&targetFBO, &targetRBO, GL_RGBA8, kWidth, kHeight);
    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kWidth, kHeight);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Blit call should not crash or assert
    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);
    glBlitFramebuffer(0, 0, kWidth, kHeight, -kWidth / 2, -kHeight / 2, 3 * kWidth / 2,
                      3 * kHeight / 2, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_NO_ERROR();
}

// This test is to ensure the draw after blit without any state change works properly
TEST_P(BlitFramebufferTest, drawBlitAndDrawAgain)
{
    constexpr const GLsizei kWidth  = 256;
    constexpr const GLsizei kHeight = 256;

    GLRenderbuffer srcColorRB, srcDepthRB;
    GLFramebuffer srcFBO;

    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(drawBlue, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());

    // Initialize source FBO with red color and depth==0.8f
    initFBOWithProgramAndDepth(&srcFBO, &srcColorRB, GL_RGBA8, &srcDepthRB, GL_DEPTH24_STENCIL8_OES,
                               kWidth, kHeight, drawRed, 0.8f);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);

    // Initialize destination FBO and initialize to green and depth==0.7
    initFBOWithProgramAndDepth(nullptr, nullptr, 0, nullptr, 0, kWidth, kHeight, drawGreen, 0.7f);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);

    // Setup for draw-blit-draw use pattern
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    std::array<Vector3, 6> quadVertices = GetQuadVertices();
    constexpr size_t kBufferSize        = sizeof(quadVertices[0]) * quadVertices.size();
    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, kBufferSize, nullptr, GL_STATIC_DRAW);
    glUseProgram(drawBlue);
    const GLint positionLocation = glGetAttribLocation(drawBlue, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    // Draw with depth=0.75, should fail depth test
    drawWithDepthValue(quadVertices, 0.75f);
    // Now blit  depth buffer from source FBO to the right half of destination FBO, so left half has
    // depth 0.7f and right half has 0.8f
    glBlitFramebuffer(kWidth / 2, 0, kWidth, kHeight, kWidth / 2, 0, kWidth, kHeight,
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    // Continue draw without state change and depth==0.75f, now it should pass depth test on right
    // half
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Now verify dstFBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2 + 1, 1, GLColor::blue);
    EXPECT_GL_NO_ERROR();
}

// This test is to ensure the scissored draw after blit without any state change works properly
TEST_P(BlitFramebufferTest, scissorDrawBlitAndDrawAgain)
{
    constexpr const GLsizei kWidth  = 256;
    constexpr const GLsizei kHeight = 256;

    GLRenderbuffer srcColorRB, srcDepthRB;
    GLFramebuffer srcFBO;

    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(drawBlue, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Blue());

    // Initialize source FBO with red color and depth==0.8f
    initFBOWithProgramAndDepth(&srcFBO, &srcColorRB, GL_RGBA8, &srcDepthRB, GL_DEPTH24_STENCIL8_OES,
                               kWidth, kHeight, drawRed, 0.8f);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);

    // Initialize destination FBO and initialize to green and depth==0.7
    initFBOWithProgramAndDepth(nullptr, nullptr, 0, nullptr, 0, kWidth, kHeight, drawGreen, 0.7f);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);

    // Setup for draw-blit-draw use pattern
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    std::array<Vector3, 6> quadVertices = GetQuadVertices();
    constexpr size_t kBufferSize        = sizeof(quadVertices[0]) * quadVertices.size();
    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, kBufferSize, nullptr, GL_STATIC_DRAW);
    glUseProgram(drawBlue);
    const GLint positionLocation = glGetAttribLocation(drawBlue, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    // Scissored draw with depth=0.75, should fail depth test
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, kWidth, kHeight / 2);
    drawWithDepthValue(quadVertices, 0.75f);
    // Now blit  depth buffer from source FBO to the right half of destination FBO, so left half has
    // depth 0.7f and right half has 0.8f
    glBlitFramebuffer(kWidth / 2, 0, kWidth, kHeight, kWidth / 2, 0, kWidth, kHeight,
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    // Continue draw without state change and depth==0.75f, now it should pass depth test on right
    // half
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Now verify dstFBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2 + 1, 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(1, kHeight - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2 + 1, kHeight - 1, GLColor::green);
    EXPECT_GL_NO_ERROR();
}

// Test blitFramebuffer size overflow checks. WebGL 2.0 spec section 5.41. We do validation for
// overflows also in non-WebGL mode to avoid triggering driver bugs.
TEST_P(BlitFramebufferTest, BlitFramebufferSizeOverflow)
{
    GLTexture textures[2];
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, 4, 4);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, 4, 4);

    GLFramebuffer framebuffers[2];
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffers[0]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[1]);

    ASSERT_GL_NO_ERROR();

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0],
                           0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1],
                           0);
    ASSERT_GL_NO_ERROR();

    // srcX
    glBlitFramebuffer(-1, 0, std::numeric_limits<GLint>::max(), 4, 0, 0, 4, 4, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glBlitFramebuffer(std::numeric_limits<GLint>::max(), 0, -1, 4, 0, 0, 4, 4, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // srcY
    glBlitFramebuffer(0, -1, 4, std::numeric_limits<GLint>::max(), 0, 0, 4, 4, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glBlitFramebuffer(0, std::numeric_limits<GLint>::max(), 4, -1, 0, 0, 4, 4, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // dstX
    glBlitFramebuffer(0, 0, 4, 4, -1, 0, std::numeric_limits<GLint>::max(), 4, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glBlitFramebuffer(0, 0, 4, 4, std::numeric_limits<GLint>::max(), 0, -1, 4, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // dstY
    glBlitFramebuffer(0, 0, 4, 4, 0, -1, 4, std::numeric_limits<GLint>::max(), GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glBlitFramebuffer(0, 0, 4, 4, 0, std::numeric_limits<GLint>::max(), 4, -1, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test blitFramebuffer size overflow checks. WebGL 2.0 spec section 5.41. Similar to above test,
// but this test more accurately duplicates the behavior of the WebGL test
// conformance2/rendering/blitframebuffer-size-overflow.html, which covers a few more edge cases.
TEST_P(BlitFramebufferTest, BlitFramebufferSizeOverflow2)
{
    GLTexture textures[2];
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, 4, 4);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, 4, 4);

    GLFramebuffer framebuffers[2];
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffers[0]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[1]);

    ASSERT_GL_NO_ERROR();

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0],
                           0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1],
                           0);
    ASSERT_GL_NO_ERROR();

    GLint width  = 8;
    GLint height = 8;

    GLTexture tex0;
    glBindTexture(GL_TEXTURE_2D, tex0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    GLFramebuffer fb0;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fb0);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex0, 0);

    GLTexture tex1;
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    GLFramebuffer fb1;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb1);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);

    GLint max = std::numeric_limits<GLint>::max();
    // Using max 32-bit integer as blitFramebuffer parameter should succeed.
    glBlitFramebuffer(0, 0, max, max, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, 0, width, height, 0, 0, max, max, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, 0, max, max, 0, 0, max, max, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    // Using blitFramebuffer parameters where calculated width/height matches max 32-bit integer
    // should succeed
    glBlitFramebuffer(-1, -1, max - 1, max - 1, 0, 0, width, height, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    glBlitFramebuffer(0, 0, width, height, -1, -1, max - 1, max - 1, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    glBlitFramebuffer(-1, -1, max - 1, max - 1, -1, -1, max - 1, max - 1, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    // Using source width/height greater than max 32-bit integer should fail.
    glBlitFramebuffer(-1, -1, max, max, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Using source width/height greater than max 32-bit integer should fail.
    glBlitFramebuffer(max, max, -1, -1, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Using destination width/height greater than max 32-bit integer should fail.
    glBlitFramebuffer(0, 0, width, height, -1, -1, max, max, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Using destination width/height greater than max 32-bit integer should fail.
    glBlitFramebuffer(0, 0, width, height, max, max, -1, -1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Using both source and destination width/height greater than max 32-bit integer should fail.
    glBlitFramebuffer(-1, -1, max, max, -1, -1, max, max, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Using minimum and maximum integers for all boundaries should fail.
    glBlitFramebuffer(-max - 1, -max - 1, max, max, -max - 1, -max - 1, max, max,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test an edge case in D3D11 stencil blitting on the CPU that does not properly clip the
// destination regions
TEST_P(BlitFramebufferTest, BlitFramebufferStencilClipNoIntersection)
{
    GLFramebuffer framebuffers[2];
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffers[0]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[1]);

    GLRenderbuffer renderbuffers[2];
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 4, 4);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffers[0]);

    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 4, 4);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffers[1]);

    glBlitFramebuffer(0, 0, 4, 4, 1 << 24, 1 << 24, 1 << 25, 1 << 25, GL_STENCIL_BUFFER_BIT,
                      GL_NEAREST);
    EXPECT_GL_NO_ERROR();
}

// Covers an edge case with blitting borderline values.
TEST_P(BlitFramebufferTest, OOBWrite)
{
    constexpr size_t length = 0x100000;
    GLFramebuffer rfb;
    GLFramebuffer dfb;
    GLRenderbuffer rb1;
    GLRenderbuffer rb2;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, rfb);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dfb);
    glBindRenderbuffer(GL_RENDERBUFFER, rb1);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 0x1000, 2);
    glBindRenderbuffer(GL_RENDERBUFFER, rb2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 2, 2);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              rb1);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              rb2);
    glBlitFramebuffer(1, 0, 0, 1, 1, 0, (2147483648 / 2) - (length / 4) + 1, 1,
                      GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
}

// Test that flipped blits don't have off-by-one errors
TEST_P(BlitFramebufferTest, FlippedBlits)
{
    constexpr const GLsizei kWidth  = 11;
    constexpr const GLsizei kHeight = 19;
    glViewport(0, 0, kWidth, kHeight);

    GLRenderbuffer srcColorRB, srcDepthRB, dstColorRB, dstDepthRB;
    GLFramebuffer srcFBO, dstFBO;

    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Green());
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorLoc = glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorLoc, -1);

    // Create source and dest FBOs
    glBindRenderbuffer(GL_RENDERBUFFER, srcColorRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, srcDepthRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, kWidth, kHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, srcFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, srcColorRB);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              srcDepthRB);

    glBindRenderbuffer(GL_RENDERBUFFER, dstColorRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, dstDepthRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, kWidth, kHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, dstFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, dstColorRB);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              dstDepthRB);

    // Fill the source framebuffer with differring values per pixel, so off-by-one errors are more
    // easily found.
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_TRUE);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    auto makeColor = [](GLsizei row, GLsizei col) -> GLColor {
        return GLColor(row * 255 / kHeight, col * 255 / kWidth, (row * 7 + col * 11) % 256, 255);
    };
    auto makeDepth = [](GLsizei row, GLsizei col) -> float {
        return 1.8f * ((row * kWidth + col) % 33 / 32.0f) - 0.9f;
    };
    auto makeStencil = [](GLsizei row, GLsizei col) -> uint8_t {
        return (col * kHeight + row) & 0xFF;
    };

    glBindFramebuffer(GL_FRAMEBUFFER, srcFBO);
    glUseProgram(drawColor);
    for (GLsizei row = 0; row < kHeight; ++row)
    {
        for (GLsizei col = 0; col < kWidth; ++col)
        {
            glScissor(col, row, 1, 1);

            glUniform4fv(colorLoc, 1, makeColor(row, col).toNormalizedVector().data());
            glStencilFunc(GL_ALWAYS, makeStencil(row, col), 0xFF);
            drawQuad(drawColor, essl1_shaders::PositionAttrib(), makeDepth(row, col));
        }
    }

    glDepthFunc(GL_LESS);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    auto test = [&](int testIndex, bool flipX, bool flipY, GLint srcOffsetX, GLint srcOffsetY,
                    GLint dstOffsetX, GLint dstOffsetY, GLint width, GLint height) {
        glDisable(GL_SCISSOR_TEST);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);

        const GLint srcX0 = srcOffsetX;
        const GLint srcY0 = srcOffsetY;
        const GLint srcX1 = srcOffsetX + width;
        const GLint srcY1 = srcOffsetY + height;

        const GLint dstX0 = flipX ? dstOffsetX + width : dstOffsetX;
        const GLint dstY0 = flipY ? dstOffsetY + height : dstOffsetY;
        const GLint dstX1 = flipX ? dstOffsetX : dstOffsetX + width;
        const GLint dstY1 = flipY ? dstOffsetY : dstOffsetY + height;

        glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1,
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                          GL_NEAREST);

        // Verify results
        glBindFramebuffer(GL_READ_FRAMEBUFFER, dstFBO);

        for (GLsizei row = 0; row < height; ++row)
        {
            for (GLsizei col = 0; col < width; ++col)
            {
                const GLint srcPixelX = col + srcOffsetX;
                const GLint srcPixelY = row + srcOffsetY;
                const GLint dstPixelX = dstOffsetX + (flipX ? width - 1 - col : col);
                const GLint dstPixelY = dstOffsetY + (flipY ? height - 1 - row : row);

                const GLColor expectColor   = makeColor(srcPixelY, srcPixelX);
                const float expectDepth     = makeDepth(srcPixelY, srcPixelX);
                const uint8_t expectStencil = makeStencil(srcPixelY, srcPixelX);

                // Verify color
                EXPECT_PIXEL_COLOR_EQ(dstPixelX, dstPixelY, expectColor)
                    << testIndex << " " << flipX << " " << flipY << " " << row << " " << col;

                glEnable(GL_SCISSOR_TEST);
                glScissor(dstPixelX, dstPixelY, 1, 1);

                // Verify depth and stencil
                glStencilFunc(GL_EQUAL, expectStencil, 0xFF);
                drawQuad(drawRed, essl1_shaders::PositionAttrib(), expectDepth - 0.05);
                drawQuad(drawGreen, essl1_shaders::PositionAttrib(), expectDepth + 0.05);

                EXPECT_PIXEL_COLOR_EQ(dstPixelX, dstPixelY, GLColor::red)
                    << testIndex << " " << flipX << " " << flipY << " " << row << " " << col;
            }
        }
    };

    for (int flipX = 0; flipX < 2; ++flipX)
    {
        for (int flipY = 0; flipY < 2; ++flipY)
        {
            // Test 0, full sized blit
            test(0, flipX != 0, flipY != 0, 0, 0, 0, 0, kWidth, kHeight);
            // Test 1, blit only one pixel
            test(1, flipX != 0, flipY != 0, kWidth / 3, kHeight / 7, 2 * kWidth / 5,
                 3 * kHeight / 4, 1, 1);
            // Test 2, random region
            test(2, flipX != 0, flipY != 0, kWidth / 5, 2 * kHeight / 7, kWidth / 6, kHeight / 4,
                 kWidth / 2, kHeight / 2);
        }
    }
}

// Test blitting a depthStencil buffer with multiple depth values to a larger size.
TEST_P(BlitFramebufferTest, BlitDepthStencilPixelByPixel)
{
    BlitDepthStencilPixelByPixelTestHelper(false /* mesaYFlip */);
}

// Same as BlitDepthStencilPixelByPixel, but with y-flip flag set.
TEST_P(BlitFramebufferTest, BlitDepthStencilPixelByPixelMesaYFlip)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    BlitDepthStencilPixelByPixelTestHelper(true /* mesaYFlip */);
}

// Regression test for a bug in the Vulkan backend where vkCmdResolveImage was used with
// out-of-bounds regions.
TEST_P(BlitFramebufferTestES31, OOBResolve)
{
    constexpr GLint kWidth  = 16;
    constexpr GLint kHeight = 32;

    // Read framebuffer is multisampled.
    GLTexture readTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, readTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, GL_TRUE);

    GLFramebuffer readFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, readFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           readTexture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw framebuffer is single sampled.
    GLTexture drawTexture;
    glBindTexture(GL_TEXTURE_2D, drawTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);

    GLFramebuffer drawFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, drawFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, drawTexture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::green);

    // Resolve the read framebuffer, using bounds that are outside the size of the image.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glBlitFramebuffer(-kWidth * 2, -kHeight * 3, kWidth * 11, kHeight * 8, -kWidth * 2,
                      -kHeight * 3, kWidth * 11, kHeight * 8, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFbo);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::red);
}

// Regression test for a bug in the Vulkan backend where vkCmdResolveImage was using the src extents
// as the resolve area instead of the area passed to glBlitFramebuffer.
TEST_P(BlitFramebufferTestES31, PartialResolve)
{
    constexpr GLint kWidth  = 16;
    constexpr GLint kHeight = 32;

    // Read framebuffer is multisampled.
    GLTexture readTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, readTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, GL_TRUE);

    GLFramebuffer readFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, readFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           readTexture, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw framebuffer is single sampled.  It's bound to a texture with base level the same size as
    // the read framebuffer, but it's bound to mip 1.
    GLTexture drawTexture;
    glBindTexture(GL_TEXTURE_2D, drawTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);

    GLFramebuffer drawFbo;
    glBindFramebuffer(GL_FRAMEBUFFER, drawFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, drawTexture, 1);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth / 2, kHeight / 2, GLColor::green);

    constexpr GLint kResolveX0 = 1;
    constexpr GLint kResolveY0 = 2;
    constexpr GLint kResolveX1 = 4;
    constexpr GLint kResolveY1 = 6;

    // Resolve only a portion of the read framebuffer.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
    glBlitFramebuffer(kResolveX0, kResolveY0, kResolveX1, kResolveY1, kResolveX0, kResolveY0,
                      kResolveX1, kResolveY1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFbo);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth / 2, kResolveY0, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(0, 0, kResolveX0, kHeight / 2, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(kResolveX1, 0, kWidth / 2 - kResolveX1, kHeight / 2, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(0, kResolveY1, kWidth / 2, kHeight / 2 - kResolveY1, GLColor::green);

    EXPECT_PIXEL_RECT_EQ(kResolveX0, kResolveY0, kResolveX1 - kResolveX0, kResolveY1 - kResolveY0,
                         GLColor::red);
}

// Test that a draw call to a small FBO followed by a resolve of a large FBO works.
TEST_P(BlitFramebufferTestES31, DrawToSmallFBOThenResolveLargeFBO)
{
    GLFramebuffer fboMS[2];
    GLTexture textureMS[2];
    GLFramebuffer fboSS;
    GLTexture textureSS;

    // A bug in the Vulkan backend grew the render area of the previous render pass on blit, even
    // though the previous render pass belonged to an unrelated framebuffer.  This test only needs
    // to make sure that the FBO being resolved is not strictly smaller than the previous FBO which
    // was drawn to.
    constexpr GLsizei kLargeWidth  = 127;
    constexpr GLsizei kLargeHeight = 54;
    constexpr GLsizei kSmallWidth  = 37;
    constexpr GLsizei kSmallHeight = 79;

    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    // Create resolve target.
    glBindTexture(GL_TEXTURE_2D, textureSS);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kLargeWidth, kLargeHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, fboSS);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureSS, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Create multisampled framebuffers and draw into them one by one.
    for (size_t fboIndex = 0; fboIndex < 2; ++fboIndex)
    {
        const GLsizei width  = fboIndex == 0 ? kLargeWidth : kSmallWidth;
        const GLsizei height = fboIndex == 0 ? kLargeHeight : kSmallHeight;

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureMS[fboIndex]);
        glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, width, height, GL_TRUE);

        glBindFramebuffer(GL_FRAMEBUFFER, fboMS[fboIndex]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                               textureMS[fboIndex], 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        glViewport(0, 0, width, height);
        drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0.8f);
        EXPECT_GL_NO_ERROR();
    }

    // Resolve the first FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboSS);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMS[0]);

    glViewport(0, 0, kLargeWidth, kLargeHeight);
    glBlitFramebuffer(0, 0, kLargeWidth, kLargeHeight, 0, 0, kLargeWidth, kLargeHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    // Verify the resolve
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboSS);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kLargeWidth - 1, kLargeHeight - 1, GLColor::red);
}

// Blit a multisampled RGBX8 framebuffer to an RGB8 framebuffer.
TEST_P(BlitFramebufferTestES31, BlitMultisampledRGBX8ToRGB8)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_rgbx_internal_format"));

    constexpr const GLsizei kWidth  = 256;
    constexpr const GLsizei kHeight = 256;

    GLTexture textureMS;
    GLRenderbuffer targetRBO;
    GLFramebuffer sourceFBO, targetFBO;

    // Initialize a source multisampled FBO with checker pattern
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureMS);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBX8_ANGLE, kWidth, kHeight,
                              GL_TRUE);
    EXPECT_GL_NO_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, sourceFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           textureMS, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ANGLE_GL_PROGRAM(checkerProgram, essl1_shaders::vs::Passthrough(),
                     essl1_shaders::fs::Checkered());
    glViewport(0, 0, kWidth, kHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, sourceFBO);
    drawQuad(checkerProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    // Initialize the destination FBO
    initColorFBO(&targetFBO, &targetRBO, GL_RGB8, kWidth, kHeight);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targetFBO);
    EXPECT_GL_NO_ERROR();

    glViewport(0, 0, kWidth, kHeight);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Scale down without flipping.
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, kHeight / 4, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 4, 3 * kHeight / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, kHeight / 4, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(3 * kWidth / 4, 3 * kHeight / 4, GLColor::yellow);
}

// Test resolving a multisampled texture with blit. Draw flipped, resolve with read fbo flipped.
TEST_P(BlitFramebufferTestES31, MultisampleFlippedResolveReadWithBlitAndFlippedDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    constexpr int kResolveFBOWidth  = kSize - 3;
    constexpr int kResolveFBOHeight = kSize - 2;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kResolveFBOWidth, kResolveFBOHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glFramebufferParameteriMESA(GL_READ_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kResolveFBOWidth, kResolveFBOHeight, 0, 0, kResolveFBOWidth,
                      kResolveFBOHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kResolveFBOWidth - 1, 0, 199, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kResolveFBOHeight - 1, kHalfPixelGradient, 215, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kResolveFBOWidth - 1, kResolveFBOHeight - 1, 199, 215, 0, 255, 1.0);
}

// Test resolving a multisampled texture with blit. Draw non-flipped, resolve with read fbo flipped.
TEST_P(BlitFramebufferTestES31, MultisampleFlippedResolveReadWithBlitAndNonFlippedDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    // Draw non-flipped - explicitly set y-flip to 0.
    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 0);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    constexpr int kResolveFBOWidth  = kSize - 3;
    constexpr int kResolveFBOHeight = kSize - 2;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kResolveFBOWidth, kResolveFBOHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    // Resolve with read fbo flipped and draw fbo non-flipped
    glFramebufferParameteriMESA(GL_READ_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kResolveFBOWidth, kResolveFBOHeight, 0, 0, kResolveFBOWidth,
                      kResolveFBOHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kResolveFBOWidth - 1, 0, 199, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kResolveFBOHeight - 1, kHalfPixelGradient, 40, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kResolveFBOWidth - 1, kResolveFBOHeight - 1, 199, 40, 0, 255, 1.0);
}

// Test resolving a multisampled texture with blit. Draw non-flipped, resolve with draw fbo flipped
TEST_P(BlitFramebufferTestES31, MultisampleFlippedResolveDrawWithBlitAndNonFlippedDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    // Draw non-flipped - explicitly set y-flip to 0.
    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 0);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    constexpr int kResolveFBOWidth  = kSize - 3;
    constexpr int kResolveFBOHeight = kSize - 2;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kResolveFBOWidth, kResolveFBOHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    // Resolve with draw fbo flipped and read fbo non-flipped.
    glFramebufferParameteriMESA(GL_READ_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 0);
    glFramebufferParameteriMESA(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
    glBlitFramebuffer(0, 0, kResolveFBOWidth, kResolveFBOHeight, 0, 0, kResolveFBOWidth,
                      kResolveFBOHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kResolveFBOWidth - 1, 0, 199, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kResolveFBOHeight - 1, kHalfPixelGradient, 215, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kResolveFBOWidth - 1, kResolveFBOHeight - 1, 199, 215, 0, 255, 1.0);
}

// Test resolving a multisampled texture with blit. Draw non-flipped, resolve with both read and
// draw fbos flipped
TEST_P(BlitFramebufferTestES31, MultisampleFlippedResolveWithBlitAndNonFlippedDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_MESA_framebuffer_flip_y"));

    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    // Draw non-flipped - explicitly set y-flip to 0.
    glFramebufferParameteriMESA(GL_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 0);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    constexpr int kResolveFBOWidth  = kSize - 3;
    constexpr int kResolveFBOHeight = kSize - 2;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kResolveFBOWidth, kResolveFBOHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    // Resolve with draw and read fbo flipped.
    glFramebufferParameteriMESA(GL_READ_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
    glFramebufferParameteriMESA(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_FLIP_Y_MESA, 1);
    glBlitFramebuffer(0, 0, kResolveFBOWidth, kResolveFBOHeight, 0, 0, kResolveFBOWidth,
                      kResolveFBOHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kResolveFBOWidth - 1, 0, 199, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kResolveFBOHeight - 1, kHalfPixelGradient, 40, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kResolveFBOWidth - 1, kResolveFBOHeight - 1, 199, 40, 0, 255, 1.0);
}

// Test resolving into smaller framebuffer.
TEST_P(BlitFramebufferTest, ResolveIntoSmallerFramebuffer)
{
    constexpr GLuint kSize[2] = {40, 32};
    glViewport(0, 0, kSize[0], kSize[0]);

    GLRenderbuffer rbo[2];
    GLFramebuffer fbo[2];

    for (int i = 0; i < 2; ++i)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, rbo[i]);
        if (i == 0)
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kSize[i], kSize[i]);
        }
        else
        {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize[i], kSize[i]);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo[i]);
    }

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glUseProgram(program);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.3f);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[1]);
    glBlitFramebuffer(0, 0, kSize[1], kSize[1], 0, 0, kSize[1], kSize[1], GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo[1]);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize[1], kSize[1], GLColor::red);
}

// Test resolving into bigger framebuffer.
TEST_P(BlitFramebufferTest, ResolveIntoBiggerFramebuffer)
{
    constexpr GLuint kSize[2] = {32, 40};
    glViewport(0, 0, kSize[0], kSize[0]);

    GLRenderbuffer rbo[2];
    GLFramebuffer fbo[2];

    for (int i = 0; i < 2; ++i)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, rbo[i]);
        if (i == 0)
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kSize[i], kSize[i]);
        }
        else
        {
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize[i], kSize[i]);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo[i]);
    }

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glUseProgram(program);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.3f);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[1]);
    glBlitFramebuffer(0, 0, kSize[1], kSize[1], 0, 0, kSize[1], kSize[1], GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo[1]);
    EXPECT_PIXEL_RECT_EQ(0, 0, kSize[0], kSize[0], GLColor::red);
}

// Test resolving into a rotated framebuffer
TEST_P(BlitFramebufferTest, ResolveWithRotation)
{
    const GLint w = getWindowWidth();
    const GLint h = getWindowHeight();

    glViewport(0, 0, w, h);

    GLRenderbuffer rbo;
    GLFramebuffer fbo;

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Checkered());
    glUseProgram(program);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.3f);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, w / 2, h / 2, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(w / 2, 0, w / 2, h / 2, GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(0, h / 2, w / 2, h / 2, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(w / 2, h / 2, w / 2, h / 2, GLColor::yellow);
}

// Test blitting a 3D texture to a 3D texture
TEST_P(BlitFramebufferTest, Blit3D)
{
    test3DBlit(GL_TEXTURE_3D, GL_TEXTURE_3D);
}

// Test blitting a 2D array texture to a 2D array texture
TEST_P(BlitFramebufferTest, Blit2DArray)
{
    test3DBlit(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_2D_ARRAY);
}

// Test blitting a 3D texture to a 2D array texture
TEST_P(BlitFramebufferTest, Blit3DTo2DArray)
{
    test3DBlit(GL_TEXTURE_3D, GL_TEXTURE_2D_ARRAY);
}

// Test blitting a 2D array texture to a 3D texture
TEST_P(BlitFramebufferTest, Blit2DArrayTo3D)
{
    test3DBlit(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_3D);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BlitFramebufferANGLETest);
ANGLE_INSTANTIATE_TEST(BlitFramebufferANGLETest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES2_D3D11_PRESENT_PATH_FAST(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_VULKAN(),
                       ES3_VULKAN(),
                       ES3_VULKAN().enable(Feature::EmulatedPrerotation90),
                       ES3_VULKAN().enable(Feature::EmulatedPrerotation180),
                       ES3_VULKAN().enable(Feature::EmulatedPrerotation270),
                       ES3_VULKAN()
                           .disable(Feature::SupportsExtendedDynamicState)
                           .disable(Feature::SupportsExtendedDynamicState2),
                       ES3_VULKAN().disable(Feature::SupportsExtendedDynamicState2),
                       ES2_METAL(),
                       ES2_METAL().disable(Feature::HasShaderStencilOutput));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BlitFramebufferTest);
ANGLE_INSTANTIATE_TEST_ES3_AND(BlitFramebufferTest,
                               ES3_VULKAN().enable(Feature::EmulatedPrerotation90),
                               ES3_VULKAN().enable(Feature::EmulatedPrerotation180),
                               ES3_VULKAN().enable(Feature::EmulatedPrerotation270),
                               ES3_VULKAN()
                                   .disable(Feature::SupportsExtendedDynamicState)
                                   .disable(Feature::SupportsExtendedDynamicState2),
                               ES3_VULKAN().disable(Feature::SupportsExtendedDynamicState2),
                               ES3_VULKAN().enable(Feature::DisableFlippingBlitWithCommand),
                               ES3_METAL().disable(Feature::HasShaderStencilOutput));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BlitFramebufferTestES31);
ANGLE_INSTANTIATE_TEST_ES31(BlitFramebufferTestES31);
}  // namespace
