//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReadPixelsTest:
//   Tests calls related to glReadPixels.
//

#include "test_utils/ANGLETest.h"

#include <array>

#include "test_utils/gl_raii.h"
#include "util/random_utils.h"

using namespace angle;

namespace
{

class ReadPixelsTest : public ANGLETest<>
{
  protected:
    ReadPixelsTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Test out of bounds framebuffer reads.
TEST_P(ReadPixelsTest, OutOfBounds)
{
    // TODO: re-enable once root cause of http://anglebug.com/42260408 is fixed
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsAdreno() && IsOpenGLES());

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    GLsizei pixelsWidth  = 32;
    GLsizei pixelsHeight = 32;
    GLint offset         = 16;
    std::vector<GLColor> pixels((pixelsWidth + offset) * (pixelsHeight + offset));

    glReadPixels(-offset, -offset, pixelsWidth + offset, pixelsHeight + offset, GL_RGBA,
                 GL_UNSIGNED_BYTE, &pixels[0]);
    EXPECT_GL_NO_ERROR();

    // Expect that all pixels which fell within the framebuffer are red
    for (int y = pixelsHeight / 2; y < pixelsHeight; y++)
    {
        for (int x = pixelsWidth / 2; x < pixelsWidth; x++)
        {
            EXPECT_EQ(GLColor::red, pixels[y * (pixelsWidth + offset) + x]);
        }
    }
}

class ReadPixelsPBONVTest : public ReadPixelsTest
{
  protected:
    ReadPixelsPBONVTest() : mPBO(0), mTexture(0), mFBO(0) {}

    void testSetUp() override
    {
        glGenBuffers(1, &mPBO);
        glGenFramebuffers(1, &mFBO);

        reset(4 * getWindowWidth() * getWindowHeight(), 4, 4);
    }

    virtual void reset(GLuint bufferSize, GLuint fboWidth, GLuint fboHeight)
    {
        ANGLE_SKIP_TEST_IF(!hasPBOExts());

        mPBOBufferSize = bufferSize;
        glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
        glBufferData(GL_PIXEL_PACK_BUFFER, mPBOBufferSize, nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        glDeleteTextures(1, &mTexture);
        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, fboWidth, fboHeight);
        mFBOWidth  = fboWidth;
        mFBOHeight = fboHeight;

        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteBuffers(1, &mPBO);
        glDeleteTextures(1, &mTexture);
        glDeleteFramebuffers(1, &mFBO);
    }

    bool hasPBOExts() const
    {
        return IsGLExtensionEnabled("GL_NV_pixel_buffer_object") &&
               IsGLExtensionEnabled("GL_EXT_texture_storage");
    }

    GLuint mPBO           = 0;
    GLuint mTexture       = 0;
    GLuint mFBO           = 0;
    GLuint mFBOWidth      = 0;
    GLuint mFBOHeight     = 0;
    GLuint mPBOBufferSize = 0;
};

// Test basic usage of PBOs.
TEST_P(ReadPixelsPBONVTest, Basic)
{
    ANGLE_SKIP_TEST_IF(!hasPBOExts() || !IsGLExtensionEnabled("GL_EXT_map_buffer_range") ||
                       !IsGLExtensionEnabled("GL_OES_mapbuffer"));

    // http://anglebug.com/42263593
    ANGLE_SKIP_TEST_IF(IsWindows() && IsDesktopOpenGL());
    // http://anglebug.com/42263926
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Clear last pixel to green
    glScissor(15, 15, 1, 1);
    glEnable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    void *mappedPtr = glMapBufferRangeEXT(GL_PIXEL_PACK_BUFFER, 0, mPBOBufferSize, GL_MAP_READ_BIT);
    GLColor *dataColor = static_cast<GLColor *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor::red, dataColor[0]);
    EXPECT_EQ(GLColor::red, dataColor[16 * 16 - 2]);
    EXPECT_EQ(GLColor::green, dataColor[16 * 16 - 1]);

    glUnmapBufferOES(GL_PIXEL_PACK_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Test that calling SubData preserves PBO data.
TEST_P(ReadPixelsPBONVTest, SubDataPreservesContents)
{
    ANGLE_SKIP_TEST_IF(!hasPBOExts() || !IsGLExtensionEnabled("GL_EXT_map_buffer_range") ||
                       !IsGLExtensionEnabled("GL_OES_mapbuffer"));

    // anglebug.com/40096466
    ANGLE_SKIP_TEST_IF(IsMac() && IsNVIDIA() && IsDesktopOpenGL());

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    unsigned char data[4] = {1, 2, 3, 4};

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mPBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4, data);

    void *mappedPtr    = glMapBufferRangeEXT(GL_ARRAY_BUFFER, 0, 32, GL_MAP_READ_BIT);
    GLColor *dataColor = static_cast<GLColor *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor(1, 2, 3, 4), dataColor[0]);
    EXPECT_EQ(GLColor::red, dataColor[1]);

    glUnmapBufferOES(GL_ARRAY_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Test that calling ReadPixels with GL_DYNAMIC_DRAW buffer works
TEST_P(ReadPixelsPBONVTest, DynamicPBO)
{
    ANGLE_SKIP_TEST_IF(!hasPBOExts() || !IsGLExtensionEnabled("GL_EXT_map_buffer_range") ||
                       !IsGLExtensionEnabled("GL_OES_mapbuffer"));

    // anglebug.com/40096466
    ANGLE_SKIP_TEST_IF(IsMac() && IsNVIDIA() && IsDesktopOpenGL());

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glBufferData(GL_PIXEL_PACK_BUFFER, 4 * getWindowWidth() * getWindowHeight(), nullptr,
                 GL_DYNAMIC_DRAW);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    unsigned char data[4] = {1, 2, 3, 4};

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mPBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4, data);

    void *mappedPtr    = glMapBufferRangeEXT(GL_ARRAY_BUFFER, 0, 32, GL_MAP_READ_BIT);
    GLColor *dataColor = static_cast<GLColor *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor(1, 2, 3, 4), dataColor[0]);
    EXPECT_EQ(GLColor::red, dataColor[1]);

    glUnmapBufferOES(GL_ARRAY_BUFFER);
    EXPECT_GL_NO_ERROR();
}

TEST_P(ReadPixelsPBONVTest, ReadFromFBO)
{
    ANGLE_SKIP_TEST_IF(!hasPBOExts() || !IsGLExtensionEnabled("GL_EXT_map_buffer_range") ||
                       !IsGLExtensionEnabled("GL_OES_mapbuffer"));

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glViewport(0, 0, mFBOWidth, mFBOHeight);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Clear last pixel to green
    glScissor(mFBOWidth - 1, mFBOHeight - 1, 1, 1);
    glEnable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, mFBOWidth, mFBOHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    void *mappedPtr =
        glMapBufferRangeEXT(GL_PIXEL_PACK_BUFFER, 0, 4 * mFBOWidth * mFBOHeight, GL_MAP_READ_BIT);
    GLColor *dataColor = static_cast<GLColor *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor::red, dataColor[0]);
    EXPECT_EQ(GLColor::red, dataColor[mFBOWidth * mFBOHeight - 2]);
    EXPECT_EQ(GLColor::green, dataColor[mFBOWidth * mFBOHeight - 1]);

    glUnmapBufferOES(GL_PIXEL_PACK_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Test calling ReadPixels with a non-zero "data" param into a PBO
TEST_P(ReadPixelsPBONVTest, ReadFromFBOWithDataOffset)
{
    ANGLE_SKIP_TEST_IF(!hasPBOExts() || !IsGLExtensionEnabled("GL_EXT_map_buffer_range") ||
                       !IsGLExtensionEnabled("GL_OES_mapbuffer"));

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glViewport(0, 0, mFBOWidth, mFBOHeight);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Clear first pixel to green
    glScissor(0, 0, 1, 1);
    glEnable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);

    // Read (height - 1) rows offset by width * 4.
    glReadPixels(0, 0, mFBOWidth, mFBOHeight - 1, GL_RGBA, GL_UNSIGNED_BYTE,
                 reinterpret_cast<void *>(mFBOWidth * static_cast<uintptr_t>(4)));

    void *mappedPtr =
        glMapBufferRangeEXT(GL_PIXEL_PACK_BUFFER, 0, 4 * mFBOWidth * mFBOHeight, GL_MAP_READ_BIT);
    GLColor *dataColor = static_cast<GLColor *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor::green, dataColor[mFBOWidth]);
    EXPECT_EQ(GLColor::red, dataColor[mFBOWidth + 1]);
    EXPECT_EQ(GLColor::red, dataColor[mFBOWidth * mFBOHeight - 1]);

    glUnmapBufferOES(GL_PIXEL_PACK_BUFFER);
    EXPECT_GL_NO_ERROR();
}

class ReadPixelsPBOTest : public ReadPixelsPBONVTest
{
  protected:
    ReadPixelsPBOTest() : ReadPixelsPBONVTest() {}

    void testSetUp() override
    {
        glGenBuffers(1, &mPBO);
        glGenFramebuffers(1, &mFBO);

        reset(4 * getWindowWidth() * getWindowHeight(), 4, 1);
    }

    void reset(GLuint bufferSize, GLuint fboWidth, GLuint fboHeight) override
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
        glBufferData(GL_PIXEL_PACK_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        glDeleteTextures(1, &mTexture);
        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, fboWidth, fboHeight);

        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        mFBOWidth  = fboWidth;
        mFBOHeight = fboHeight;

        mPBOBufferSize = bufferSize;

        ASSERT_GL_NO_ERROR();
    }
};

// Test basic usage of PBOs.
TEST_P(ReadPixelsPBOTest, Basic)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    void *mappedPtr    = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, 32, GL_MAP_READ_BIT);
    GLColor *dataColor = static_cast<GLColor *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor::red, dataColor[0]);

    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Test copy to snorm
TEST_P(ReadPixelsPBOTest, Snorm)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_render_snorm"));

    constexpr GLsizei kSize = 6;

    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_SNORM, kSize, kSize);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_SCISSOR_TEST);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glScissor(0, 0, kSize / 2, kSize / 2);
    glClear(GL_COLOR_BUFFER_BIT);

    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glScissor(kSize / 2, 0, kSize / 2, kSize / 2);
    glClear(GL_COLOR_BUFFER_BIT);

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glScissor(0, kSize / 2, kSize / 2, kSize / 2);
    glClear(GL_COLOR_BUFFER_BIT);

    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glScissor(kSize / 2, kSize / 2, kSize / 2, kSize / 2);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glDisable(GL_SCISSOR_TEST);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, kSize, kSize, GL_RGBA, GL_BYTE, 0);

    std::vector<GLColor> result(kSize * kSize);
    void *mappedPtr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, kSize * kSize * 4, GL_MAP_READ_BIT);
    memcpy(result.data(), mappedPtr, kSize * kSize * 4);
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    EXPECT_GL_NO_ERROR();

    auto verify = [&](const GLColor expect[4]) {
        for (size_t i = 0; i < kSize; ++i)
        {
            for (size_t j = 0; j < kSize; ++j)
            {
                uint32_t index = (i < kSize / 2 ? 0 : 1) << 1 | (j < kSize / 2 ? 0 : 1);
                EXPECT_EQ(result[i * kSize + j], expect[index]) << i << " " << j;
            }
        }
    };

    // The image should have the following colors
    //
    //     +---+---+
    //     | R | G |
    //     +---+---+
    //     | B | Y |
    //     +---+---+
    //
    const GLColor kColors[4] = {
        GLColor(127, 0, 0, 127),
        GLColor(0, 127, 0, 127),
        GLColor(0, 0, 127, 127),
        GLColor(127, 127, 0, 127),
    };
    verify(kColors);

    // Test again, but this time with reverse order
    if (EnsureGLExtensionEnabled("GL_ANGLE_pack_reverse_row_order"))
    {
        glPixelStorei(GL_PACK_REVERSE_ROW_ORDER_ANGLE, GL_TRUE);
        glReadPixels(0, 0, kSize, kSize, GL_RGBA, GL_BYTE, 0);

        mappedPtr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, kSize * kSize * 4, GL_MAP_READ_BIT);
        memcpy(result.data(), mappedPtr, kSize * kSize * 4);
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        EXPECT_GL_NO_ERROR();

        const GLColor kReversedColors[4] = {
            GLColor(0, 0, 127, 127),
            GLColor(127, 127, 0, 127),
            GLColor(127, 0, 0, 127),
            GLColor(0, 127, 0, 127),
        };
        verify(kReversedColors);
    }
}

// Test read pixel to PBO of an sRGB unorm renderbuffer
TEST_P(ReadPixelsPBOTest, SrgbUnorm)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_pack_reverse_row_order"));

    constexpr GLsizei kSize = 1;
    constexpr angle::GLColor clearColor(64, 0, 0, 255);
    constexpr angle::GLColor encodedToSrgbColor(136, 0, 0, 255);

    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_SRGB8_ALPHA8, kSize, kSize);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    glClearColor(clearColor[0] / 255.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glPixelStorei(GL_PACK_REVERSE_ROW_ORDER_ANGLE, GL_TRUE);
    glReadPixels(0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    GLColor result;
    void *mappedPtr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, kSize * kSize * 4, GL_MAP_READ_BIT);
    memcpy(result.data(), mappedPtr, kSize * kSize * 4);
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    EXPECT_GL_NO_ERROR();

    EXPECT_NEAR(result[0], encodedToSrgbColor[0], 1);
}

// Test an error is generated when the PBO is too small.
TEST_P(ReadPixelsPBOTest, PBOTooSmall)
{
    reset(4 * 16 * 16 - 1, 16, 16);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test an error is generated when the PBO is mapped.
TEST_P(ReadPixelsPBOTest, PBOMapped)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, 32, GL_MAP_READ_BIT);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that binding a PBO to ARRAY_BUFFER works as expected.
TEST_P(ReadPixelsPBOTest, ArrayBufferTarget)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mPBO);

    void *mappedPtr    = glMapBufferRange(GL_ARRAY_BUFFER, 0, 32, GL_MAP_READ_BIT);
    GLColor *dataColor = static_cast<GLColor *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor::red, dataColor[0]);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Test that using a PBO does not overwrite existing data.
TEST_P(ReadPixelsPBOTest, ExistingDataPreserved)
{
    // Clear backbuffer to red
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Read 16x16 region from red backbuffer to PBO
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // Clear backbuffer to green
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Read 16x16 region from green backbuffer to PBO at offset 16
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<void *>(16));
    void *mappedPtr =
        glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, 17 * sizeof(GLColor), GL_MAP_READ_BIT);
    GLColor *dataColor = static_cast<GLColor *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    // Test pixel 0 is red (existing data)
    EXPECT_EQ(GLColor::red, dataColor[0]);

    // Test pixel 16 is green (new data)
    EXPECT_EQ(GLColor::green, dataColor[16]);

    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Test that calling SubData preserves PBO data.
TEST_P(ReadPixelsPBOTest, SubDataPreservesContents)
{
    // anglebug.com/40096466
    ANGLE_SKIP_TEST_IF(IsMac() && IsNVIDIA() && IsDesktopOpenGL());

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    unsigned char data[4] = {1, 2, 3, 4};

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mPBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4, data);

    void *mappedPtr    = glMapBufferRange(GL_ARRAY_BUFFER, 0, 32, GL_MAP_READ_BIT);
    GLColor *dataColor = static_cast<GLColor *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor(1, 2, 3, 4), dataColor[0]);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Same as the prior test, but with an offset.
TEST_P(ReadPixelsPBOTest, SubDataOffsetPreservesContents)
{
    // anglebug.com/42260410
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsAdreno() && IsOpenGLES());
    // anglebug.com/40096466
    ANGLE_SKIP_TEST_IF(IsMac() && IsNVIDIA() && IsDesktopOpenGL());

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    unsigned char data[4] = {1, 2, 3, 4};

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mPBO);
    glBufferSubData(GL_ARRAY_BUFFER, 16, 4, data);

    void *mappedPtr    = glMapBufferRange(GL_ARRAY_BUFFER, 0, 32, GL_MAP_READ_BIT);
    GLColor *dataColor = static_cast<GLColor *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor::red, dataColor[0]);
    EXPECT_EQ(GLColor(1, 2, 3, 4), dataColor[4]);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Test that uploading data to buffer that's in use then writing to it as PBO works.
TEST_P(ReadPixelsPBOTest, UseAsUBOThenUpdateThenReadFromFBO)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glViewport(0, 0, mFBOWidth, mFBOHeight);

    const std::array<GLColor, 4> kInitialData = {GLColor::red, GLColor::red, GLColor::red,
                                                 GLColor::red};
    const std::array<GLColor, 4> kUpdateData  = {GLColor::white, GLColor::white, GLColor::white,
                                                 GLColor::white};

    GLBuffer buffer;
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(kInitialData), kInitialData.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);
    EXPECT_GL_NO_ERROR();

    constexpr char kVerifyUBO[] = R"(#version 300 es
precision mediump float;
uniform block {
    uvec4 data;
} ubo;
out vec4 colorOut;
void main()
{
    if (all(equal(ubo.data, uvec4(0xFF0000FFu))))
        colorOut = vec4(0, 1.0, 0, 1.0);
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    ANGLE_GL_PROGRAM(verifyUbo, essl3_shaders::vs::Simple(), kVerifyUBO);
    drawQuad(verifyUbo, essl3_shaders::PositionAttrib(), 0.5);
    EXPECT_GL_NO_ERROR();

    // Update buffer data
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(kInitialData), kUpdateData.data());
    EXPECT_GL_NO_ERROR();

    // Clear first pixel to blue
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glScissor(0, 0, 1, 1);
    glEnable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer);

    // Read the framebuffer pixels
    glReadPixels(0, 0, mFBOWidth, mFBOHeight, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    void *mappedPtr =
        glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, sizeof(kInitialData), GL_MAP_READ_BIT);
    GLColor *dataColor = static_cast<GLColor *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor::blue, dataColor[0]);
    EXPECT_EQ(GLColor::green, dataColor[1]);
    EXPECT_EQ(GLColor::green, dataColor[2]);
    EXPECT_EQ(GLColor::green, dataColor[3]);

    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(2, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3, 0, GLColor::green);
}

// Test PBO readback with row length smaller than area width.
TEST_P(ReadPixelsPBOTest, SmallRowLength)
{
    constexpr int kSize = 2;
    reset(kSize * kSize * 4, kSize, kSize);
    std::vector<GLColor> texData(kSize * kSize);
    texData[0] = GLColor::red;
    texData[1] = GLColor::green;
    texData[2] = GLColor::blue;
    texData[3] = GLColor::white;
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE,
                    texData.data());
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    std::vector<GLColor> bufData(kSize * kSize, GLColor::black);
    glBufferData(GL_PIXEL_PACK_BUFFER, mPBOBufferSize, bufData.data(), GL_STATIC_DRAW);

    glPixelStorei(GL_PACK_ROW_LENGTH, 1);
    glReadPixels(0, 0, kSize, kSize, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();

    void *mappedPtr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, mPBOBufferSize, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mappedPtr);
    ASSERT_GL_NO_ERROR();

    // TODO(anglebug.com/354005999)
    // Metal compute path may produce flaky results
    // Suppressed until a fallback is implemented
    if (!IsMetal())
    {
        GLColor *colorPtr = static_cast<GLColor *>(mappedPtr);
        EXPECT_EQ(colorPtr[0], GLColor::red);
        EXPECT_EQ(colorPtr[1], GLColor::blue);
        EXPECT_EQ(colorPtr[2], GLColor::white);
        EXPECT_EQ(colorPtr[3], GLColor::black);
    }
    ASSERT_TRUE(glUnmapBuffer(GL_PIXEL_PACK_BUFFER));
    ASSERT_GL_NO_ERROR();
}

class ReadPixelsPBODrawTest : public ReadPixelsPBOTest
{
  protected:
    ReadPixelsPBODrawTest() : mProgram(0), mPositionVBO(0) {}

    void testSetUp() override
    {
        ReadPixelsPBOTest::testSetUp();

        constexpr char kVS[] =
            "attribute vec4 aTest; attribute vec2 aPosition; varying vec4 vTest;\n"
            "void main()\n"
            "{\n"
            "    vTest        = aTest;\n"
            "    gl_Position  = vec4(aPosition, 0.0, 1.0);\n"
            "    gl_PointSize = 1.0;\n"
            "}";

        constexpr char kFS[] =
            "precision mediump float; varying vec4 vTest;\n"
            "void main()\n"
            "{\n"
            "    gl_FragColor = vTest;\n"
            "}";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram);

        glGenBuffers(1, &mPositionVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mPositionVBO);
        glBufferData(GL_ARRAY_BUFFER, 128, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void testTearDown() override
    {
        glDeleteProgram(mProgram);
        glDeleteBuffers(1, &mPositionVBO);
        ReadPixelsPBOTest::testTearDown();
    }

    GLuint mProgram;
    GLuint mPositionVBO;
};

// Test that we can draw with PBO data.
TEST_P(ReadPixelsPBODrawTest, DrawWithPBO)
{
    GLColor color(1, 2, 3, 4);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &color);
    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    EXPECT_GL_NO_ERROR();

    float positionData[] = {0.5f, 0.5f};

    glUseProgram(mProgram);
    glViewport(0, 0, 1, 1);
    glBindBuffer(GL_ARRAY_BUFFER, mPositionVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 1 * 2 * 4, positionData);
    EXPECT_GL_NO_ERROR();

    GLint positionLocation = glGetAttribLocation(mProgram, "aPosition");
    EXPECT_NE(-1, positionLocation);

    GLint testLocation = glGetAttribLocation(mProgram, "aTest");
    EXPECT_NE(-1, testLocation);

    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, mPBO);
    glVertexAttribPointer(testLocation, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(testLocation);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    color = GLColor(0, 0, 0, 0);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &color);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor(1, 2, 3, 4), color);
}

// Test that we can correctly update a buffer bound to the vertex stage with PBO.
TEST_P(ReadPixelsPBODrawTest, UpdateVertexArrayWithPixelPack)
{
    glUseProgram(mProgram);
    glViewport(0, 0, 1, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    ASSERT_GL_NO_ERROR();

    // First draw with pre-defined data.
    std::array<float, 2> positionData = {0.5f, 0.5f};

    glBindBuffer(GL_ARRAY_BUFFER, mPositionVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positionData.size() * sizeof(positionData[0]),
                    positionData.data());
    ASSERT_GL_NO_ERROR();

    GLint positionLocation = glGetAttribLocation(mProgram, "aPosition");
    EXPECT_NE(-1, positionLocation);

    GLint testLocation = glGetAttribLocation(mProgram, "aTest");
    EXPECT_NE(-1, testLocation);

    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);
    ASSERT_GL_NO_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, mPBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLColor), &GLColor::red);
    glVertexAttribPointer(testLocation, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(testLocation);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    ASSERT_GL_NO_ERROR();

    // Update the buffer bound to the VAO with a PBO.
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    ASSERT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    // Draw again and verify the VAO has the updated data.
    glDrawArrays(GL_POINTS, 0, 1);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

class ReadPixelsTextureNorm16PBOTest : public ReadPixelsTest
{
  protected:
    void testSetUp() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glBindTexture(GL_TEXTURE_2D, mTex);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTex, 0);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
        ASSERT_GL_NO_ERROR();
    }

    template <typename T>
    void test(GLenum format, GLenum internalFormat, GLenum readFormat)
    {
        const bool isSigned = std::is_same<T, GLshort>::value;
        const GLenum type   = isSigned ? GL_SHORT : GL_UNSIGNED_SHORT;

        T data[4] = {};
        data[0]   = isSigned ? -32767 : 32767;
        data[1]   = isSigned ? -16383 : 16383;
        data[2]   = isSigned ? -8191 : 8191;
        data[3]   = isSigned ? -4095 : 4095;

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 1, 1, 0, format, type, data);
        ASSERT_GL_NO_ERROR();
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        bool supportedCombination = true;
        if (readFormat != GL_RGBA)
        {
            GLenum implementationFormat, implementationType;
            glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT,
                          reinterpret_cast<GLint *>(&implementationFormat));
            glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE,
                          reinterpret_cast<GLint *>(&implementationType));
            ASSERT_GL_NO_ERROR();

            supportedCombination = implementationFormat == readFormat && implementationType == type;
        }

        glBufferData(GL_PIXEL_PACK_BUFFER, 12, nullptr, GL_STATIC_COPY);
        ASSERT_GL_NO_ERROR();

        // Use non-zero offset for better code coverage
        constexpr GLint offset = 4;
        glReadPixels(0, 0, 1, 1, readFormat, type, reinterpret_cast<void *>(offset));
        if (supportedCombination)
        {
            ASSERT_GL_NO_ERROR();
        }
        else
        {
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);
            ANGLE_SKIP_TEST_IF(!supportedCombination);
        }

        T *dataRead =
            static_cast<T *>(glMapBufferRange(GL_PIXEL_PACK_BUFFER, offset, 8, GL_MAP_READ_BIT));
        ASSERT_GL_NO_ERROR();

        EXPECT_EQ(dataRead[0], data[0]);
        if (readFormat == GL_RGBA || readFormat == GL_RG)
        {
            EXPECT_EQ(dataRead[1], format != GL_RED ? data[1] : 0);
        }
        if (readFormat == GL_RGBA)
        {
            EXPECT_EQ(dataRead[2], format == GL_RGBA ? data[2] : 0);
            EXPECT_EQ(dataRead[3], format == GL_RGBA ? data[3] : (isSigned ? 32767 : 65535));
        }
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    }

    void testUnsigned(GLenum format, GLenum internalFormat, GLenum readFormat)
    {
        ASSERT(internalFormat == GL_RGBA16_EXT || internalFormat == GL_RG16_EXT ||
               internalFormat == GL_R16_EXT);
        test<GLushort>(format, internalFormat, readFormat);
    }

    void testSigned(GLenum format, GLenum internalFormat, GLenum readFormat)
    {
        ASSERT(internalFormat == GL_RGBA16_SNORM_EXT || internalFormat == GL_RG16_SNORM_EXT ||
               internalFormat == GL_R16_SNORM_EXT);
        test<GLshort>(format, internalFormat, readFormat);
    }

    GLFramebuffer mFBO;
    GLTexture mTex;
    GLBuffer mPBO;
};

// Test PBO RGBA readback for RGBA16 color buffer.
TEST_P(ReadPixelsTextureNorm16PBOTest, RGBA16_RGBA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));
    testUnsigned(GL_RGBA, GL_RGBA16_EXT, GL_RGBA);
}

// Test PBO RGBA readback for RG16 color buffer.
TEST_P(ReadPixelsTextureNorm16PBOTest, RG16_RGBA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));
    testUnsigned(GL_RG, GL_RG16_EXT, GL_RGBA);
}

// Test PBO RG readback for RG16 color buffer.
TEST_P(ReadPixelsTextureNorm16PBOTest, RG16_RG)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));
    testUnsigned(GL_RG, GL_RG16_EXT, GL_RG);
}

// Test PBO RGBA readback for R16 color buffer.
TEST_P(ReadPixelsTextureNorm16PBOTest, R16_RGBA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));
    testUnsigned(GL_RED, GL_R16_EXT, GL_RGBA);
}

// Test PBO RED readback for R16 color buffer.
TEST_P(ReadPixelsTextureNorm16PBOTest, R16_RED)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));
    testUnsigned(GL_RED, GL_R16_EXT, GL_RED);
}

// Test PBO RGBA readback for RGBA16_SNORM color buffer.
TEST_P(ReadPixelsTextureNorm16PBOTest, RGBA16_SNORM_RGBA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_render_snorm"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));
    testSigned(GL_RGBA, GL_RGBA16_SNORM_EXT, GL_RGBA);
}

// Test PBO RGBA readback for RG16_SNORM color buffer.
TEST_P(ReadPixelsTextureNorm16PBOTest, RG16_SNORM_RGBA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_render_snorm"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));
    testSigned(GL_RG, GL_RG16_SNORM_EXT, GL_RGBA);
}

// Test PBO RG readback for RG16_SNORM color buffer.
TEST_P(ReadPixelsTextureNorm16PBOTest, RG16_SNORM_RG)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_render_snorm"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));
    testSigned(GL_RG, GL_RG16_SNORM_EXT, GL_RG);
}

// Test PBO RGBA readback for R16_SNORM color buffer.
TEST_P(ReadPixelsTextureNorm16PBOTest, R16_SNORM_RGBA)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_render_snorm"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));
    testSigned(GL_RED, GL_R16_SNORM_EXT, GL_RGBA);
}

// Test PBO RED readback for R16_SNORM color buffer.
TEST_P(ReadPixelsTextureNorm16PBOTest, R16_SNORM_RED)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_render_snorm"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));
    testSigned(GL_RED, GL_R16_SNORM_EXT, GL_RED);
}

class ReadPixelsMultisampleTest : public ReadPixelsTest
{
  protected:
    ReadPixelsMultisampleTest() : mFBO(0), mRBO(0), mPBO(0)
    {
        setSamples(4);
        setMultisampleEnabled(true);
    }

    void testSetUp() override
    {
        glGenFramebuffers(1, &mFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

        glGenRenderbuffers(1, &mRBO);
        glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

        glGenBuffers(1, &mPBO);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
        glBufferData(GL_PIXEL_PACK_BUFFER, 4 * getWindowWidth() * getWindowHeight(), nullptr,
                     GL_STATIC_DRAW);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteFramebuffers(1, &mFBO);
        glDeleteRenderbuffers(1, &mRBO);
        glDeleteBuffers(1, &mPBO);
    }

    GLuint mFBO;
    GLuint mRBO;
    GLuint mPBO;
};

// Test ReadPixels from a multisampled framebuffer.
TEST_P(ReadPixelsMultisampleTest, BasicClear)
{
    if (getClientMajorVersion() < 3 && !IsGLExtensionEnabled("GL_ANGLE_framebuffer_multisample"))
    {
        std::cout
            << "Test skipped because ES3 or GL_ANGLE_framebuffer_multisample is not available."
            << std::endl;
        return;
    }

    if (IsGLExtensionEnabled("GL_ANGLE_framebuffer_multisample"))
    {
        glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, 2, GL_RGBA8, 4, 4);
    }
    else
    {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_RGBA8, 4, 4);
    }

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mRBO);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    EXPECT_GL_NO_ERROR();

    glReadPixels(0, 0, 1, 1, GL_RGBA8, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test ReadPixels from a multisampled swapchain.
TEST_P(ReadPixelsMultisampleTest, DefaultFramebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
    EXPECT_GL_NO_ERROR();
}

// Test ReadPixels from a multisampled swapchain into a PBO.
TEST_P(ReadPixelsMultisampleTest, DefaultFramebufferPBO)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);

    const int w = getWindowWidth();
    const int h = getWindowHeight();
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    const std::vector<angle::GLColor> expectedColor(w * h, GLColor::red);
    std::vector<angle::GLColor> actualColor(w * h);
    const void *mapPointer =
        glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, sizeof(angle::GLColor) * w * h, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mapPointer);
    memcpy(actualColor.data(), mapPointer, sizeof(angle::GLColor) * w * h);
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

    EXPECT_EQ(expectedColor, actualColor);
}

class ReadPixelsTextureTest : public ANGLETest<>
{
  public:
    ReadPixelsTextureTest() : mFBO(0), mTextureRGBA(0), mTextureBGRA(0)
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        glGenTextures(1, &mTextureRGBA);
        glGenTextures(1, &mTextureBGRA);
        glGenFramebuffers(1, &mFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    }

    void testTearDown() override
    {
        glDeleteFramebuffers(1, &mFBO);
        glDeleteTextures(1, &mTextureRGBA);
        glDeleteTextures(1, &mTextureBGRA);
    }

    void initTextureRGBA(GLenum textureTarget,
                         GLint levels,
                         GLint attachmentLevel,
                         GLint attachmentLayer)
    {
        glBindTexture(textureTarget, mTextureRGBA);
        glTexStorage3D(textureTarget, levels, GL_RGBA8, kSize, kSize, kSize);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTextureRGBA,
                                  attachmentLevel, attachmentLayer);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        ASSERT_GL_NO_ERROR();
        initializeTextureData(textureTarget, levels, GL_RGBA);
    }

    void initTextureBGRA(GLenum textureTarget,
                         GLint levels,
                         GLint attachmentLevel,
                         GLint attachmentLayer)
    {
        glBindTexture(textureTarget, mTextureBGRA);
        for (GLint level = 0; level < levels; ++level)
        {
            glTexImage3D(textureTarget, level, GL_BGRA_EXT, kSize >> level, kSize >> level,
                         textureTarget == GL_TEXTURE_3D ? kSize >> level : kSize, 0, GL_BGRA_EXT,
                         GL_UNSIGNED_BYTE, nullptr);
        }
        glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(textureTarget, GL_TEXTURE_MAX_LEVEL, levels - 1);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTextureBGRA,
                                  attachmentLevel, attachmentLayer);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        ASSERT_GL_NO_ERROR();
        initializeTextureData(textureTarget, levels, GL_BGRA_EXT);
    }

    void testRead(GLenum textureTarget, GLint levels, GLint attachmentLevel, GLint attachmentLayer)
    {
        initTextureRGBA(textureTarget, levels, attachmentLevel, attachmentLayer);
        verifyColor(attachmentLevel, attachmentLayer);

        // Skip BGRA test on GL/Nvidia, leading to internal incomplete framebuffer error.
        // http://anglebug.com/42266676
        ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsOpenGL());

        if (IsGLExtensionEnabled("GL_EXT_texture_format_BGRA8888"))
        {
            initTextureBGRA(textureTarget, levels, attachmentLevel, attachmentLayer);
            verifyColor(attachmentLevel, attachmentLayer);
        }
    }

    void initPBO()
    {
        // Create a buffer big enough to hold mip 0 + allow some offset during readback.
        glGenBuffers(1, &mBuffer);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, mBuffer);
        glBufferData(GL_PIXEL_PACK_BUFFER, sizeof(angle::GLColor) * 16 * 2, nullptr,
                     GL_STREAM_COPY);
        ASSERT_GL_NO_ERROR();
    }

    void testPBORead(GLenum textureTarget,
                     GLint levels,
                     GLint attachmentLevel,
                     GLint attachmentLayer)
    {
        initPBO();
        initTextureRGBA(textureTarget, levels, attachmentLevel, attachmentLayer);
        verifyPBO(attachmentLevel, attachmentLayer);

        // Skip BGRA test on GL/Nvidia, leading to internal incomplete framebuffer error.
        // http://anglebug.com/42266676
        ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsOpenGL());

        if (IsGLExtensionEnabled("GL_EXT_texture_format_BGRA8888"))
        {
            initTextureBGRA(textureTarget, levels, attachmentLevel, attachmentLayer);
            verifyPBO(attachmentLevel, attachmentLayer);
        }
    }

    // Give each {level,layer} pair a (probably) unique color via random.
    GLuint getColorValue(GLint level, GLint layer)
    {
        mRNG.reseed(level + layer * 32);
        return mRNG.randomUInt();
    }

    void verifyColor(GLint level, GLint layer)
    {
        const angle::GLColor colorValue(getColorValue(level, layer));
        const GLint size = kSize >> level;
        EXPECT_PIXEL_RECT_EQ(0, 0, size, size, colorValue);
    }

    void verifyPBO(GLint level, GLint layer)
    {
        const GLint size     = kSize >> level;
        const GLsizei offset = kSize * (level + layer);
        glReadPixels(0, 0, size, size, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<void *>(offset));

        const std::vector<angle::GLColor> expectedColor(size * size, getColorValue(level, layer));
        std::vector<angle::GLColor> actualColor(size * size);

        void *mapPointer = glMapBufferRange(GL_PIXEL_PACK_BUFFER, offset,
                                            sizeof(angle::GLColor) * size * size, GL_MAP_READ_BIT);
        ASSERT_NE(nullptr, mapPointer);
        memcpy(actualColor.data(), mapPointer, sizeof(angle::GLColor) * size * size);
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

        ASSERT_GL_NO_ERROR();
        EXPECT_EQ(expectedColor, actualColor);
    }

    void initializeTextureData(GLenum textureTarget, GLint levels, GLenum format)
    {
        for (GLint level = 0; level < levels; ++level)
        {
            GLint mipSize = kSize >> level;
            GLint layers  = (textureTarget == GL_TEXTURE_3D ? mipSize : kSize);

            size_t layerSize = mipSize * mipSize;
            std::vector<GLuint> textureData(layers * layerSize);

            for (GLint layer = 0; layer < layers; ++layer)
            {
                GLuint colorValue = getColorValue(level, layer);
                size_t offset     = (layer * layerSize);

                if (format == GL_BGRA_EXT)
                {
                    const GLuint rb = colorValue & 0x00FF00FF;
                    const GLuint br = (rb & 0xFF) << 16 | rb >> 16;
                    const GLuint ga = colorValue & 0xFF00FF00;
                    colorValue      = ga | br;
                }

                std::fill(textureData.begin() + offset, textureData.begin() + offset + layerSize,
                          colorValue);
            }

            glTexSubImage3D(textureTarget, level, 0, 0, 0, mipSize, mipSize, layers, format,
                            GL_UNSIGNED_BYTE, textureData.data());
        }
    }

    static constexpr GLint kSize = 4;

    angle::RNG mRNG;
    GLuint mFBO;
    GLuint mTextureRGBA;
    GLuint mTextureBGRA;
    GLuint mBuffer;
};

// Test 3D attachment readback.
TEST_P(ReadPixelsTextureTest, BasicAttachment3D)
{
    testRead(GL_TEXTURE_3D, 1, 0, 0);
}

// Test 3D attachment readback, non-zero mip.
TEST_P(ReadPixelsTextureTest, MipAttachment3D)
{
    testRead(GL_TEXTURE_3D, 2, 1, 0);
}

// Test 3D attachment readback, non-zero layer.
TEST_P(ReadPixelsTextureTest, LayerAttachment3D)
{
    testRead(GL_TEXTURE_3D, 1, 0, 1);
}

// Test 3D attachment readback, non-zero mip and layer.
TEST_P(ReadPixelsTextureTest, MipLayerAttachment3D)
{
    testRead(GL_TEXTURE_3D, 2, 1, 1);
}

// Test 2D array attachment readback.
TEST_P(ReadPixelsTextureTest, BasicAttachment2DArray)
{
    testRead(GL_TEXTURE_2D_ARRAY, 1, 0, 0);
}

// Test 3D attachment readback, non-zero mip.
TEST_P(ReadPixelsTextureTest, MipAttachment2DArray)
{
    testRead(GL_TEXTURE_2D_ARRAY, 2, 1, 0);
}

// Test 3D attachment readback, non-zero layer.
TEST_P(ReadPixelsTextureTest, LayerAttachment2DArray)
{
    testRead(GL_TEXTURE_2D_ARRAY, 1, 0, 1);
}

// Test 3D attachment readback, non-zero mip and layer.
TEST_P(ReadPixelsTextureTest, MipLayerAttachment2DArray)
{
    testRead(GL_TEXTURE_2D_ARRAY, 2, 1, 1);
}

// Test 3D attachment PBO readback.
TEST_P(ReadPixelsTextureTest, BasicAttachment3DPBO)
{
    testPBORead(GL_TEXTURE_3D, 1, 0, 0);
}

// Test 3D attachment readback, non-zero mip.
TEST_P(ReadPixelsTextureTest, MipAttachment3DPBO)
{
    testPBORead(GL_TEXTURE_3D, 2, 1, 0);
}

// Test 3D attachment readback, non-zero layer.
TEST_P(ReadPixelsTextureTest, LayerAttachment3DPBO)
{
    // http://anglebug.com/40644770
    ANGLE_SKIP_TEST_IF(IsMac() && IsIntelUHD630Mobile() && IsDesktopOpenGL());

    testPBORead(GL_TEXTURE_3D, 1, 0, 1);
}

// Test 3D attachment readback, non-zero mip and layer.
TEST_P(ReadPixelsTextureTest, MipLayerAttachment3DPBO)
{
    // http://anglebug.com/40644770
    ANGLE_SKIP_TEST_IF(IsMac() && IsIntelUHD630Mobile() && IsDesktopOpenGL());

    testPBORead(GL_TEXTURE_3D, 2, 1, 1);
}

// Test 2D array attachment readback.
TEST_P(ReadPixelsTextureTest, BasicAttachment2DArrayPBO)
{
    testPBORead(GL_TEXTURE_2D_ARRAY, 1, 0, 0);
}

// Test 3D attachment readback, non-zero mip.
TEST_P(ReadPixelsTextureTest, MipAttachment2DArrayPBO)
{
    testPBORead(GL_TEXTURE_2D_ARRAY, 2, 1, 0);
}

// Test 3D attachment readback, non-zero layer.
TEST_P(ReadPixelsTextureTest, LayerAttachment2DArrayPBO)
{
    // http://anglebug.com/40644770
    ANGLE_SKIP_TEST_IF(IsMac() && IsIntelUHD630Mobile() && IsDesktopOpenGL());

    testPBORead(GL_TEXTURE_2D_ARRAY, 1, 0, 1);
}

// Test 3D attachment readback, non-zero mip and layer.
TEST_P(ReadPixelsTextureTest, MipLayerAttachment2DArrayPBO)
{
    // http://anglebug.com/40644770
    ANGLE_SKIP_TEST_IF(IsMac() && IsIntelUHD630Mobile() && IsDesktopOpenGL());

    testPBORead(GL_TEXTURE_2D_ARRAY, 2, 1, 1);
}

// a test class to be used for error checking of glReadPixels
class ReadPixelsErrorTest : public ReadPixelsTest
{
  protected:
    ReadPixelsErrorTest() : mTexture(0), mFBO(0) {}

    void testSetUp() override
    {
        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 1);

        glGenFramebuffers(1, &mFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteTextures(1, &mTexture);
        glDeleteFramebuffers(1, &mFBO);
    }

    void testUnsupportedTypeConversions(std::vector<GLenum> internalFormats,
                                        std::vector<GLenum> unsupportedTypes)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        for (GLenum internalFormat : internalFormats)
        {
            GLRenderbuffer rbo;
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, 1, 1);
            ASSERT_GL_NO_ERROR();

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
            ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

            GLenum implementationFormat, implementationType;
            glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT,
                          reinterpret_cast<GLint *>(&implementationFormat));
            ASSERT_GL_NO_ERROR();
            glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE,
                          reinterpret_cast<GLint *>(&implementationType));
            ASSERT_GL_NO_ERROR();

            for (GLenum type : unsupportedTypes)
            {
                uint8_t pixel[8] = {};
                if (implementationFormat != GL_RGBA || implementationType != type)
                {
                    glReadPixels(0, 0, 1, 1, GL_RGBA, type, pixel);
                    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
                }
            }
        }
    }

    GLuint mTexture;
    GLuint mFBO;
};

//  The test verifies that glReadPixels generates a GL_INVALID_OPERATION error
//  when the read buffer is GL_NONE.
//  Reference: GLES 3.0.4, Section 4.3.2 Reading Pixels
TEST_P(ReadPixelsErrorTest, ReadBufferIsNone)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glReadBuffer(GL_NONE);
    std::vector<GLubyte> pixels(4);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// The test verifies that glReadPixels generates a GL_INVALID_OPERATION
// error when reading signed 8-bit color buffers using incompatible types.
TEST_P(ReadPixelsErrorTest, ColorBufferSnorm8)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_render_snorm"));

    testUnsupportedTypeConversions({GL_R8_SNORM, GL_RG8_SNORM, GL_RGBA8_SNORM},
                                   {GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT});
}

// The test verifies that glReadPixels generates a GL_INVALID_OPERATION
// error when reading signed 16-bit color buffers using incompatible types.
TEST_P(ReadPixelsErrorTest, ColorBufferSnorm16)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_render_snorm"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_norm16"));

    testUnsupportedTypeConversions({GL_R16_SNORM_EXT, GL_RG16_SNORM_EXT, GL_RGBA16_SNORM_EXT},
                                   {GL_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT});
}

// texture internal format is GL_RGBA32F
class ReadPixelsFloat32TypePBOTest : public ReadPixelsPBOTest
{
  protected:
    ReadPixelsFloat32TypePBOTest() : ReadPixelsPBOTest() {}

    void testSetUp() override
    {
        glGenBuffers(1, &mPBO);
        glGenFramebuffers(1, &mFBO);

        // buffer size sufficient enough for glReadPixels when data != 0
        reset(4 * 4 * 1 * 4 + 16, 4, 1);
    }

    void reset(GLuint bufferSize, GLuint fboWidth, GLuint fboHeight) override
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
        glBufferData(GL_PIXEL_PACK_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        glDeleteTextures(1, &mTexture);
        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, fboWidth, fboHeight);

        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);

        mFBOWidth  = fboWidth;
        mFBOHeight = fboHeight;

        mPBOBufferSize = bufferSize;

        ASSERT_GL_NO_ERROR();
    }
};

// Test basic usage of PBOs.
TEST_P(ReadPixelsFloat32TypePBOTest, Basic)
{
    glClearColor(0.5f, 0.2f, 0.3f, 0.4f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, mFBOWidth, mFBOHeight, GL_RGBA, GL_FLOAT, reinterpret_cast<void *>(2));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glReadPixels(0, 0, mFBOWidth, mFBOHeight, GL_RGBA, GL_FLOAT, 0);
    EXPECT_GL_NO_ERROR();

    void *mappedPtr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, mPBOBufferSize, GL_MAP_READ_BIT);
    GLColor32F *dataColor = static_cast<GLColor32F *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor32F(0.5f, 0.2f, 0.3f, 0.4f), dataColor[0]);

    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// a test class to be used for error checking of glReadPixels with WebGLCompatibility
class ReadPixelsWebGLErrorTest : public ReadPixelsTest
{
  protected:
    ReadPixelsWebGLErrorTest() : mTexture(0), mFBO(0) { setWebGLCompatibilityEnabled(true); }

    void testSetUp() override
    {
        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 1);

        glGenFramebuffers(1, &mFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteTextures(1, &mTexture);
        glDeleteFramebuffers(1, &mFBO);
    }

    GLuint mTexture;
    GLuint mFBO;
};

// Test that WebGL context readpixels generates an error when reading GL_UNSIGNED_INT_24_8 type.
TEST_P(ReadPixelsWebGLErrorTest, TypeIsUnsignedInt24_8)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    std::vector<GLuint> pixels(4);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_INT_24_8, pixels.data());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Test that WebGL context readpixels generates an error when reading GL_DEPTH_COMPONENT format.
TEST_P(ReadPixelsWebGLErrorTest, FormatIsDepthComponent)
{
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    std::vector<GLubyte> pixels(4);
    EXPECT_GL_NO_ERROR();
    glReadPixels(0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, pixels.data());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

}  // anonymous namespace

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2(ReadPixelsTest);
ANGLE_INSTANTIATE_TEST_ES2(ReadPixelsPBONVTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ReadPixelsPBOTest);
ANGLE_INSTANTIATE_TEST_ES3(ReadPixelsPBOTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ReadPixelsFloat32TypePBOTest);
ANGLE_INSTANTIATE_TEST_ES3(ReadPixelsFloat32TypePBOTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ReadPixelsPBODrawTest);
ANGLE_INSTANTIATE_TEST_ES3_AND(ReadPixelsPBODrawTest,
                               ES3_VULKAN().enable(Feature::ForceFallbackFormat));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ReadPixelsTextureNorm16PBOTest);
ANGLE_INSTANTIATE_TEST_ES3(ReadPixelsTextureNorm16PBOTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ReadPixelsMultisampleTest);
ANGLE_INSTANTIATE_TEST_ES3(ReadPixelsMultisampleTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ReadPixelsTextureTest);
ANGLE_INSTANTIATE_TEST_ES3(ReadPixelsTextureTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ReadPixelsErrorTest);
ANGLE_INSTANTIATE_TEST_ES3(ReadPixelsErrorTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ReadPixelsWebGLErrorTest);
ANGLE_INSTANTIATE_TEST_ES3(ReadPixelsWebGLErrorTest);
