//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WebGLReadOutsideFramebufferTest.cpp : Test functions which read the framebuffer (readPixels,
// copyTexSubImage2D, copyTexImage2D) on areas outside the framebuffer.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace
{

class PixelRect
{
  public:
    PixelRect(int width, int height) : mWidth(width), mHeight(height), mData(width * height) {}

    // Set each pixel to a different color consisting of the x,y position and a given tag.
    // Making each pixel a different means any misplaced pixel will cause a failure.
    // Encoding the position proved valuable in debugging.
    void fill(unsigned tag)
    {
        for (int x = 0; x < mWidth; ++x)
        {
            for (int y = 0; y < mHeight; ++y)
            {
                mData[x + y * mWidth] = angle::GLColor(x + (y << 8) + (tag << 16));
            }
        }
    }

    void toTexture2D(GLuint target, GLuint texid) const
    {
        glBindTexture(target, texid);
        if (target == GL_TEXTURE_CUBE_MAP)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, mData.data());
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, mData.data());
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, mData.data());
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, mData.data());
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, mData.data());
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, mData.data());
        }
        else
        {
            ASSERT_GLENUM_EQ(GL_TEXTURE_2D, target);
            glTexImage2D(target, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         mData.data());
        }
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void toTexture3D(GLuint target, GLuint texid, GLint depth) const
    {
        glBindTexture(target, texid);

        glTexImage3D(target, 0, GL_RGBA, mWidth, mHeight, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        for (GLint z = 0; z < depth; z++)
        {
            glTexSubImage3D(target, 0, 0, 0, z, mWidth, mHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                            mData.data());
        }
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    void readFB(int x, int y)
    {
        glReadPixels(x, y, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, mData.data());
    }

    // Read pixels from 'other' into 'this' from position (x,y).
    // Pixels outside 'other' are untouched or zeroed according to 'zeroOutside.'
    void readPixelRect(const PixelRect &other, int x, int y, bool zeroOutside)
    {
        for (int i = 0; i < mWidth; ++i)
        {
            for (int j = 0; j < mHeight; ++j)
            {
                angle::GLColor *dest = &mData[i + j * mWidth];
                if (!other.getPixel(x + i, y + j, dest) && zeroOutside)
                {
                    *dest = angle::GLColor(0);
                }
            }
        }
    }

    bool getPixel(int x, int y, angle::GLColor *colorOut) const
    {
        if (0 <= x && x < mWidth && 0 <= y && y < mHeight)
        {
            *colorOut = mData[x + y * mWidth];
            return true;
        }
        return false;
    }

    void compare(const PixelRect &expected) const
    {
        ASSERT_EQ(mWidth, expected.mWidth);
        ASSERT_EQ(mHeight, expected.mHeight);

        for (int x = 0; x < mWidth; ++x)
        {
            for (int y = 0; y < mHeight; ++y)
            {
                ASSERT_EQ(expected.mData[x + y * mWidth], mData[x + y * mWidth])
                    << "at (" << x << ", " << y << ")";
            }
        }
    }

  private:
    int mWidth, mHeight;
    std::vector<angle::GLColor> mData;
};

}  // namespace

namespace angle
{

class WebGLReadOutsideFramebufferTest : public ANGLETest<>
{
  public:
    // Read framebuffer to 'pixelsOut' via glReadPixels.
    void TestReadPixels(int x, int y, int, PixelRect *pixelsOut) { pixelsOut->readFB(x, y); }

    // Read framebuffer to 'pixelsOut' via glCopyTexSubImage2D and GL_TEXTURE_2D.
    void TestCopyTexSubImage2D(int x, int y, int, PixelRect *pixelsOut)
    {
        // Init texture with given pixels.
        GLTexture destTexture;
        pixelsOut->toTexture2D(GL_TEXTURE_2D, destTexture);

        // Read framebuffer -> texture -> 'pixelsOut'
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, kReadWidth, kReadHeight);
        readTexture2D(GL_TEXTURE_2D, destTexture, kReadWidth, kReadHeight, pixelsOut);
    }

    // Read framebuffer to 'pixelsOut' via glCopyTexSubImage2D and cube map.
    void TestCopyTexSubImageCube(int x, int y, int, PixelRect *pixelsOut)
    {
        // Init texture with given pixels.
        GLTexture destTexture;
        pixelsOut->toTexture2D(GL_TEXTURE_CUBE_MAP, destTexture);

        // Read framebuffer -> texture -> 'pixelsOut'
        glCopyTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, x, y, kReadWidth, kReadHeight);
        readTexture2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, destTexture, kReadWidth, kReadHeight,
                      pixelsOut);
    }

    // Read framebuffer to 'pixelsOut' via glCopyTexSubImage3D and a 2D array texture.
    void TestCopyTexSubImage2DArray(int x, int y, int z, PixelRect *pixelsOut)
    {
        // Init texture with given pixels.
        GLTexture destTexture;
        pixelsOut->toTexture3D(GL_TEXTURE_2D_ARRAY, destTexture, kTextureDepth);

        // Read framebuffer -> texture -> 'pixelsOut'
        glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, z, x, y, kReadWidth, kReadHeight);
        readTexture3D(destTexture, kReadWidth, kReadHeight, z, pixelsOut);
    }

    // Read framebuffer to 'pixelsOut' via glCopyTexSubImage3D.
    void TestCopyTexSubImage3D(int x, int y, int z, PixelRect *pixelsOut)
    {
        // Init texture with given pixels.
        GLTexture destTexture;
        pixelsOut->toTexture3D(GL_TEXTURE_3D, destTexture, kTextureDepth);

        // Read framebuffer -> texture -> 'pixelsOut'
        glCopyTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, z, x, y, kReadWidth, kReadHeight);
        readTexture3D(destTexture, kReadWidth, kReadHeight, z, pixelsOut);
    }

    // Read framebuffer to 'pixelsOut' via glCopyTexImage2D and GL_TEXTURE_2D.
    void TestCopyTexImage2D(int x, int y, int, PixelRect *pixelsOut)
    {
        // Init texture with given pixels.
        GLTexture destTexture;
        pixelsOut->toTexture2D(GL_TEXTURE_2D, destTexture);

        // Read framebuffer -> texture -> 'pixelsOut'
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, kReadWidth, kReadHeight, 0);
        readTexture2D(GL_TEXTURE_2D, destTexture, kReadWidth, kReadHeight, pixelsOut);
    }

    // Read framebuffer to 'pixelsOut' via glCopyTexImage2D and cube map.
    void TestCopyTexImageCube(int x, int y, int, PixelRect *pixelsOut)
    {
        // Init texture with given pixels.
        GLTexture destTexture;
        pixelsOut->toTexture2D(GL_TEXTURE_CUBE_MAP, destTexture);

        // Read framebuffer -> texture -> 'pixelsOut'
        glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, x, y, kReadWidth, kReadHeight,
                         0);
        readTexture2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, destTexture, kReadWidth, kReadHeight,
                      pixelsOut);
    }

  protected:
    static constexpr int kFbWidth      = 128;
    static constexpr int kFbHeight     = 128;
    static constexpr int kTextureDepth = 16;
    static constexpr int kReadWidth    = 4;
    static constexpr int kReadHeight   = 4;
    static constexpr int kReadLayer    = 2;

    // Tag the framebuffer pixels differently than the initial read buffer pixels, so we know for
    // sure which pixels are changed by reading.
    static constexpr GLuint fbTag   = 0x1122;
    static constexpr GLuint readTag = 0xaabb;

    WebGLReadOutsideFramebufferTest() : mFBData(kFbWidth, kFbHeight)
    {
        setWindowWidth(kFbWidth);
        setWindowHeight(kFbHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setRobustResourceInit(true);
        setWebGLCompatibilityEnabled(true);
    }

    void testSetUp() override
    {
        constexpr char kVS[] = R"(
attribute vec3 a_position;
varying vec2 v_texCoord;
void main() {
    v_texCoord = a_position.xy * 0.5 + 0.5;
    gl_Position = vec4(a_position, 1);
})";
        constexpr char kFS[] = R"(
precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D u_texture;
void main() {
    gl_FragColor = texture2D(u_texture, v_texCoord);
})";

        mProgram = CompileProgram(kVS, kFS);
        glUseProgram(mProgram);
        GLint uniformLoc = glGetUniformLocation(mProgram, "u_texture");
        ASSERT_NE(-1, uniformLoc);
        glUniform1i(uniformLoc, 0);

        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        // fill framebuffer with unique pixels
        mFBData.fill(fbTag);
        GLTexture fbTexture;
        mFBData.toTexture2D(GL_TEXTURE_2D, fbTexture);
        drawQuad(mProgram, "a_position", 0.0f, 1.0f, true);
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    using TestFunc = void (WebGLReadOutsideFramebufferTest::*)(int x,
                                                               int y,
                                                               int z,
                                                               PixelRect *dest);

    void Main2D(TestFunc testFunc, bool zeroOutside) { mainImpl(testFunc, zeroOutside, 0); }

    void Main3D(TestFunc testFunc, bool zeroOutside)
    {
        mainImpl(testFunc, zeroOutside, kReadLayer);
    }

    void mainImpl(TestFunc testFunc, bool zeroOutside, int readLayer)
    {
        PixelRect actual(kReadWidth, kReadHeight);
        PixelRect expected(kReadWidth, kReadHeight);

        // Read a kReadWidth*kReadHeight rectangle of pixels from places that include:
        // - completely outside framebuffer, on all sides of it (i,j < 0 or > 2)
        // - completely inside framebuffer (i,j == 1)
        // - straddling framebuffer boundary, at each corner and side
        for (int i = -1; i < 4; ++i)
        {
            for (int j = -1; j < 4; ++j)
            {
                int x = i * kFbWidth / 2 - kReadWidth / 2;
                int y = j * kFbHeight / 2 - kReadHeight / 2;

                // Put unique pixel values into the read destinations.
                actual.fill(readTag);
                expected.readPixelRect(actual, 0, 0, false);

                // Read from framebuffer into 'actual.'
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                (this->*testFunc)(x, y, readLayer, &actual);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                // Simulate framebuffer read, into 'expected.'
                expected.readPixelRect(mFBData, x, y, zeroOutside);

                // See if they are the same.
                actual.compare(expected);
            }
        }
    }

    // Get contents of given texture by drawing it into a framebuffer then reading with
    // glReadPixels().
    void readTexture2D(GLuint target, GLuint texture, GLsizei width, GLsizei height, PixelRect *out)
    {
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, texture, 0);
        out->readFB(0, 0);
    }

    // Get contents of current texture by drawing it into a framebuffer then reading with
    // glReadPixels().
    void readTexture3D(GLuint texture, GLsizei width, GLsizei height, int zSlice, PixelRect *out)
    {
        GLFramebuffer fbo;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, zSlice);
        out->readFB(0, 0);
    }

    PixelRect mFBData;
    GLuint mProgram;
};

class WebGL2ReadOutsideFramebufferTest : public WebGLReadOutsideFramebufferTest
{};

// Check that readPixels does not set a destination pixel when
// the corresponding source pixel is outside the framebuffer.
TEST_P(WebGLReadOutsideFramebufferTest, ReadPixels)
{
    Main2D(&WebGLReadOutsideFramebufferTest::TestReadPixels, false);
}

// Check that copyTexSubImage2D does not set a destination pixel when
// the corresponding source pixel is outside the framebuffer.
TEST_P(WebGLReadOutsideFramebufferTest, CopyTexSubImage2D)
{
    Main2D(&WebGLReadOutsideFramebufferTest::TestCopyTexSubImage2D, false);
    Main2D(&WebGLReadOutsideFramebufferTest::TestCopyTexSubImageCube, false);
}

// Check that copyTexImage2D sets (0,0,0,0) for pixels outside the framebuffer.
TEST_P(WebGLReadOutsideFramebufferTest, CopyTexImage2D)
{
    Main2D(&WebGLReadOutsideFramebufferTest::TestCopyTexImage2D, true);
    Main2D(&WebGLReadOutsideFramebufferTest::TestCopyTexImageCube, true);
}

// Check that copyTexSubImage3D does not set a destination pixel when
// the corresponding source pixel is outside the framebuffer.
TEST_P(WebGL2ReadOutsideFramebufferTest, CopyTexSubImage3D)
{
    Main3D(&WebGLReadOutsideFramebufferTest::TestCopyTexSubImage2DArray, false);
    Main3D(&WebGLReadOutsideFramebufferTest::TestCopyTexSubImage3D, false);
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(WebGLReadOutsideFramebufferTest);

ANGLE_INSTANTIATE_TEST_ES3(WebGL2ReadOutsideFramebufferTest);

}  // namespace angle
