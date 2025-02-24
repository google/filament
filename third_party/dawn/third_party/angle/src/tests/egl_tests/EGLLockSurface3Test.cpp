//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLLockSurface3Test.cpp:
//   EGL extension EGL_KHR_lock_surface
//

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <thread>
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"

using namespace std::chrono_literals;

using namespace angle;

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLLockSurface3Test);

class EGLLockSurface3Test : public ANGLETest<>
{
  public:
    EGLLockSurface3Test() : mDisplay(EGL_NO_DISPLAY) {}

    void testSetUp() override
    {
        mMajorVersion = GetParam().majorVersion;

        EGLint dispattrs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(), EGL_NONE};
        mDisplay           = eglGetPlatformDisplayEXT(
            EGL_PLATFORM_ANGLE_ANGLE, reinterpret_cast<void *>(EGL_DEFAULT_DISPLAY), dispattrs);
        EXPECT_NE(mDisplay, EGL_NO_DISPLAY);
        EXPECT_EGL_TRUE(eglInitialize(mDisplay, nullptr, nullptr));
    }

    bool supportsLockSurface3Extension()
    {
        return IsEGLDisplayExtensionEnabled(mDisplay, "EGL_KHR_lock_surface3");
    }

    void testTearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglTerminate(mDisplay);
            eglReleaseThread();
            mDisplay = EGL_NO_DISPLAY;
        }
        ASSERT_EGL_SUCCESS() << "Error during test TearDown";
    }

    // This will work for Windows and linux systemsAny other BGRA systems will have to be included.
    GLColor ChannelOrder(GLColor colorIn)
    {
        GLColor color = colorIn;
        if (IsWindows() || IsLinux())
        {  // BGRA channel order
            color.R = colorIn.B;
            color.B = colorIn.R;
        }
        return color;
    }

    void fillBitMapRGBA32(GLColor colorIn, uint32_t *bitMapPtr, EGLint stride)
    {
        for (uint32_t y = 0; y < kHeight; y++)
        {
            uint32_t *pixelPtr = bitMapPtr + (y * (stride / 4));
            for (uint32_t x = 0; x < kWidth; x++)
            {
                pixelPtr[x] = ChannelOrder(colorIn).asUint();
            }
        }
    }

    bool checkBitMapRGBA32(GLColor colorIn, uint32_t *bitMapPtr, EGLint stride)
    {
        std::array<uint32_t, (kWidth * kHeight)> checkmap;
        std::fill_n(checkmap.begin(), (kWidth * kHeight), ChannelOrder(colorIn).asUint());

        std::array<uint32_t, (kWidth * kHeight)> bitmap;
        for (uint32_t i = 0; i < (kWidth * kHeight); i++)
        {
            bitmap[i] = bitMapPtr[i];
        }

        if (bitmap != checkmap)
        {
            return false;
        }

        return true;
    }

    bool checkSurfaceRGBA32(GLColor color)
    {
        std::array<uint32_t, (kWidth * kHeight)> actual;
        glReadPixels(0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE, actual.data());
        EXPECT_GL_NO_ERROR();

        std::array<uint32_t, (kWidth * kHeight)> checkmap;
        std::fill_n(checkmap.begin(), (kWidth * kHeight), color.asUint());

        if (actual != checkmap)
        {
            return false;
        }

        return true;
    }

    GLuint createTexture()
    {
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
        EXPECT_GL_NO_ERROR();
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight);
        EXPECT_GL_NO_ERROR();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        EXPECT_NE(texture, static_cast<GLuint>(0));
        return texture;
    }

    bool fillTexture(GLuint textureId, GLColor color)
    {
        std::array<GLuint, (kWidth * kHeight)> pixels;
        std::fill_n(pixels.begin(), (kWidth * kHeight), color.asUint());
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                        static_cast<void *>(pixels.data()));
        EXPECT_GL_NO_ERROR();
        return true;
    }

    void renderTexture(GLuint textureId)
    {
        const char *kVertexShader   = R"(
            precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = vec4(position.xy, 0.0, 1.0);
                texcoord = (position.xy * 0.5) + 0.5;
            }
        )";
        const char *kFragmentShader = R"(
            precision highp float;
            uniform sampler2D tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            }
        )";

        ANGLE_GL_PROGRAM(program, kVertexShader, kFragmentShader);
        glUseProgram(program);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glActiveTexture(GL_TEXTURE0);
        GLint texture2DUniformLocation = glGetUniformLocation(program, "tex");
        glUniform1i(texture2DUniformLocation, 0);
        drawQuad(program, "position", 0.5f);
        glDeleteProgram(program);
        EXPECT_GL_NO_ERROR();
    }

    int mMajorVersion   = 2;
    EGLDisplay mDisplay = EGL_NO_DISPLAY;

    static constexpr EGLint kWidth  = 5;
    static constexpr EGLint kHeight = 5;
};

// Create parity between eglQuerySurface and eglQuerySurface64KHR
TEST_P(EGLLockSurface3Test, QuerySurfaceAndQuerySurface64Parity)
{
    ANGLE_SKIP_TEST_IF(!supportsLockSurface3Extension());

    EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
    EGLint attribs[]     = {EGL_RED_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_ALPHA_SIZE,
                            8,
                            EGL_RENDERABLE_TYPE,
                            clientVersion,
                            EGL_SURFACE_TYPE,
                            (EGL_PBUFFER_BIT | EGL_LOCK_SURFACE_BIT_KHR),
                            EGL_NONE};
    EGLint count         = 0;
    EGLConfig config     = EGL_NO_CONFIG_KHR;
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, attribs, &config, 1, &count));
    EXPECT_GT(count, 0);
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);

    EGLint pBufferAttribs[]   = {EGL_WIDTH, kWidth, EGL_HEIGHT, kHeight, EGL_NONE};
    EGLSurface pBufferSurface = eglCreatePbufferSurface(mDisplay, config, pBufferAttribs);
    EXPECT_NE(pBufferSurface, EGL_NO_SURFACE);

    EGLint width         = 0;
    EGLAttribKHR width64 = 0;
    EXPECT_EGL_TRUE(eglQuerySurface(mDisplay, pBufferSurface, EGL_WIDTH, &width));
    EXPECT_EGL_TRUE(eglQuerySurface64KHR(mDisplay, pBufferSurface, EGL_WIDTH, &width64));
    EXPECT_EQ(static_cast<EGLAttribKHR>(width), width64);

    EGLint height         = 0;
    EGLAttribKHR height64 = 0;
    EXPECT_EGL_TRUE(eglQuerySurface(mDisplay, pBufferSurface, EGL_HEIGHT, &height));
    EXPECT_EGL_TRUE(eglQuerySurface64KHR(mDisplay, pBufferSurface, EGL_HEIGHT, &height64));
    EXPECT_EQ(static_cast<EGLAttribKHR>(height), height64);

    EXPECT_EGL_TRUE(eglDestroySurface(mDisplay, pBufferSurface));
}

// Create PBufferSurface, Lock, check all the attributes, unlock.
TEST_P(EGLLockSurface3Test, AttributeTest)
{
    ANGLE_SKIP_TEST_IF(!supportsLockSurface3Extension());

    EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
    EGLint attribs[]     = {EGL_RED_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_ALPHA_SIZE,
                            8,
                            EGL_RENDERABLE_TYPE,
                            clientVersion,
                            EGL_SURFACE_TYPE,
                            (EGL_PBUFFER_BIT | EGL_LOCK_SURFACE_BIT_KHR),
                            EGL_NONE};
    EGLint count         = 0;
    EGLConfig config     = EGL_NO_CONFIG_KHR;
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, attribs, &config, 1, &count));
    EXPECT_GT(count, 0);
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);

    EGLint pBufferAttribs[]   = {EGL_WIDTH, kWidth, EGL_HEIGHT, kHeight, EGL_NONE};
    EGLSurface pBufferSurface = eglCreatePbufferSurface(mDisplay, config, pBufferAttribs);
    EXPECT_NE(pBufferSurface, EGL_NO_SURFACE);

    EGLint lockAttribs[] = {EGL_LOCK_USAGE_HINT_KHR, EGL_WRITE_SURFACE_BIT_KHR, EGL_NONE};
    EXPECT_EGL_TRUE(eglLockSurfaceKHR(mDisplay, pBufferSurface, lockAttribs));

    EGLAttribKHR bitMapPtr = 0;
    EXPECT_EGL_TRUE(
        eglQuerySurface64KHR(mDisplay, pBufferSurface, EGL_BITMAP_POINTER_KHR, &bitMapPtr));
    EXPECT_NE(bitMapPtr, 0);

    EGLAttribKHR bitMapPitch = 0;
    EXPECT_EGL_TRUE(
        eglQuerySurface64KHR(mDisplay, pBufferSurface, EGL_BITMAP_PITCH_KHR, &bitMapPitch));
    EXPECT_NE(bitMapPitch, 0);

    EGLAttribKHR bitMapOrigin = 0;
    EXPECT_EGL_TRUE(
        eglQuerySurface64KHR(mDisplay, pBufferSurface, EGL_BITMAP_ORIGIN_KHR, &bitMapOrigin));
    EXPECT_TRUE((bitMapOrigin == EGL_LOWER_LEFT_KHR) || (bitMapOrigin == EGL_UPPER_LEFT_KHR));

    EGLAttribKHR bitMapPixelRedOffset = 0;
    EXPECT_EGL_TRUE(eglQuerySurface64KHR(mDisplay, pBufferSurface, EGL_BITMAP_PIXEL_RED_OFFSET_KHR,
                                         &bitMapPixelRedOffset));
    EXPECT_TRUE((bitMapPixelRedOffset == 0) || (bitMapPixelRedOffset == 16));

    EGLAttribKHR bitMapPixelGreenOffset = 0;
    EXPECT_EGL_TRUE(eglQuerySurface64KHR(
        mDisplay, pBufferSurface, EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR, &bitMapPixelGreenOffset));
    EXPECT_EQ(bitMapPixelGreenOffset, 8);

    EGLAttribKHR bitMapPixelBlueOffset = 0;
    EXPECT_EGL_TRUE(eglQuerySurface64KHR(mDisplay, pBufferSurface, EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR,
                                         &bitMapPixelBlueOffset));
    EXPECT_TRUE((bitMapPixelBlueOffset == 16) || (bitMapPixelBlueOffset == 0));

    EGLAttribKHR bitMapPixelAlphaOffset = 0;
    EXPECT_EGL_TRUE(eglQuerySurface64KHR(
        mDisplay, pBufferSurface, EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR, &bitMapPixelAlphaOffset));
    EXPECT_EQ(bitMapPixelAlphaOffset, 24);

    EGLAttribKHR bitMapPixelLuminanceOffset = 0;
    EXPECT_EGL_TRUE(eglQuerySurface64KHR(mDisplay, pBufferSurface,
                                         EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR,
                                         &bitMapPixelLuminanceOffset));
    EXPECT_EQ(bitMapPixelLuminanceOffset, 0);

    EGLAttribKHR bitMapPixelSize = 0;
    EXPECT_EGL_TRUE(eglQuerySurface64KHR(mDisplay, pBufferSurface, EGL_BITMAP_PIXEL_SIZE_KHR,
                                         &bitMapPixelSize));
    EXPECT_EQ(bitMapPixelSize, 32);

    EGLint bitMapPitchInt = 0;
    EXPECT_EGL_TRUE(
        eglQuerySurface(mDisplay, pBufferSurface, EGL_BITMAP_PITCH_KHR, &bitMapPitchInt));
    EXPECT_NE(bitMapPitchInt, 0);

    EGLint bitMapOriginInt = 0;
    EXPECT_EGL_TRUE(
        eglQuerySurface(mDisplay, pBufferSurface, EGL_BITMAP_ORIGIN_KHR, &bitMapOriginInt));
    EXPECT_TRUE((bitMapOriginInt == EGL_LOWER_LEFT_KHR) || (bitMapOriginInt == EGL_UPPER_LEFT_KHR));

    EGLint bitMapPixelRedOffsetInt = 0;
    EXPECT_EGL_TRUE(eglQuerySurface(mDisplay, pBufferSurface, EGL_BITMAP_PIXEL_RED_OFFSET_KHR,
                                    &bitMapPixelRedOffsetInt));
    EXPECT_TRUE((bitMapPixelRedOffsetInt == 0) || (bitMapPixelRedOffsetInt == 16));

    EGLint bitMapPixelGreenOffsetInt = 0;
    EXPECT_EGL_TRUE(eglQuerySurface(mDisplay, pBufferSurface, EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR,
                                    &bitMapPixelGreenOffsetInt));
    EXPECT_EQ(bitMapPixelGreenOffsetInt, 8);

    EGLint bitMapPixelBlueOffsetInt = 0;
    EXPECT_EGL_TRUE(eglQuerySurface(mDisplay, pBufferSurface, EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR,
                                    &bitMapPixelBlueOffsetInt));
    EXPECT_TRUE((bitMapPixelBlueOffsetInt == 16) || (bitMapPixelBlueOffsetInt == 0));

    EGLint bitMapPixelAlphaOffsetInt = 0;
    EXPECT_EGL_TRUE(eglQuerySurface(mDisplay, pBufferSurface, EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR,
                                    &bitMapPixelAlphaOffsetInt));
    EXPECT_EQ(bitMapPixelAlphaOffsetInt, 24);

    EGLint bitMapPixelLuminanceOffsetInt = 0;
    EXPECT_EGL_TRUE(eglQuerySurface(mDisplay, pBufferSurface, EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR,
                                    &bitMapPixelLuminanceOffsetInt));
    EXPECT_EQ(bitMapPixelLuminanceOffsetInt, 0);

    EGLint bitMapPixelSizeInt = 0;
    EXPECT_EGL_TRUE(
        eglQuerySurface(mDisplay, pBufferSurface, EGL_BITMAP_PIXEL_SIZE_KHR, &bitMapPixelSizeInt));
    EXPECT_EQ(bitMapPixelSizeInt, 32);

    EXPECT_EGL_TRUE(eglUnlockSurfaceKHR(mDisplay, pBufferSurface));
}

// Create PBufferSurface, glClear Green, Draw red quad, Lock, check buffer for red,
//  Write white pixels, Unlock, Test pixels for white
TEST_P(EGLLockSurface3Test, PbufferSurfaceReadWriteTest)
{
    ANGLE_SKIP_TEST_IF(!supportsLockSurface3Extension());

    EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
    EGLint attribs[]     = {EGL_RED_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_ALPHA_SIZE,
                            8,
                            EGL_RENDERABLE_TYPE,
                            clientVersion,
                            EGL_SURFACE_TYPE,
                            (EGL_PBUFFER_BIT | EGL_LOCK_SURFACE_BIT_KHR),
                            EGL_NONE};
    EGLint count         = 0;
    EGLConfig config     = EGL_NO_CONFIG_KHR;
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, attribs, &config, 1, &count));
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);
    EXPECT_GT(count, 0);

    EGLint pBufferAttribs[]   = {EGL_WIDTH, kWidth, EGL_HEIGHT, kHeight, EGL_NONE};
    EGLSurface pBufferSurface = EGL_NO_SURFACE;
    pBufferSurface            = eglCreatePbufferSurface(mDisplay, config, pBufferAttribs);
    EXPECT_NE(pBufferSurface, EGL_NO_SURFACE);

    EGLint ctxAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, mMajorVersion, EGL_NONE};
    EGLContext context  = eglCreateContext(mDisplay, config, nullptr, ctxAttribs);
    EXPECT_NE(context, EGL_NO_CONTEXT);

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, pBufferSurface, pBufferSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    glClearColor(kFloatGreen.R, kFloatGreen.G, kFloatGreen.B, kFloatGreen.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR() << "glClear failed";

    const GLColor drawColor = GLColor::red;
    GLuint texture          = createTexture();
    EXPECT_TRUE(fillTexture(texture, drawColor));
    renderTexture(texture);
    glFinish();
    ASSERT_GL_NO_ERROR() << "glFinish failed";

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));

    EGLint lockAttribs[] = {EGL_LOCK_USAGE_HINT_KHR, EGL_WRITE_SURFACE_BIT_KHR,
                            EGL_MAP_PRESERVE_PIXELS_KHR, EGL_TRUE, EGL_NONE};
    EXPECT_EGL_TRUE(eglLockSurfaceKHR(mDisplay, pBufferSurface, lockAttribs));

    EGLAttribKHR bitMap = 0;
    EXPECT_EGL_TRUE(
        eglQuerySurface64KHR(mDisplay, pBufferSurface, EGL_BITMAP_POINTER_KHR, &bitMap));
    EGLAttribKHR bitMapPitch = 0;
    uint32_t *bitMapPtr      = (uint32_t *)(bitMap);
    EXPECT_EGL_TRUE(
        eglQuerySurface64KHR(mDisplay, pBufferSurface, EGL_BITMAP_PITCH_KHR, &bitMapPitch));

    EXPECT_TRUE(checkBitMapRGBA32(drawColor, bitMapPtr, bitMapPitch));

    const GLColor fillColor = GLColor::white;
    fillBitMapRGBA32(fillColor, bitMapPtr, bitMapPitch);

    EXPECT_EGL_TRUE(eglUnlockSurfaceKHR(mDisplay, pBufferSurface));

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, pBufferSurface, pBufferSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    EXPECT_TRUE(checkSurfaceRGBA32(fillColor));

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_TRUE(eglDestroySurface(mDisplay, pBufferSurface));
    EXPECT_EGL_TRUE(eglDestroyContext(mDisplay, context));
}

// Create PBufferSurface, glClear Green, Lock, check buffer for green, Write white pixels,
// Unlock, Test pixels for white.
// This expects that a glClear() alone is sufficient to pre-color the Surface
TEST_P(EGLLockSurface3Test, PbufferSurfaceReadWriteDeferredCleaarTest)
{
    ANGLE_SKIP_TEST_IF(!supportsLockSurface3Extension());

    EGLint clientVersion = mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT;
    EGLint attribs[]     = {EGL_RED_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_ALPHA_SIZE,
                            8,
                            EGL_RENDERABLE_TYPE,
                            clientVersion,
                            EGL_SURFACE_TYPE,
                            (EGL_PBUFFER_BIT | EGL_LOCK_SURFACE_BIT_KHR),
                            EGL_NONE};
    EGLint count         = 0;
    EGLConfig config     = EGL_NO_CONFIG_KHR;
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, attribs, &config, 1, &count));
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);
    EXPECT_GT(count, 0);

    EGLint pBufferAttribs[]   = {EGL_WIDTH, kWidth, EGL_HEIGHT, kHeight, EGL_NONE};
    EGLSurface pBufferSurface = EGL_NO_SURFACE;
    pBufferSurface            = eglCreatePbufferSurface(mDisplay, config, pBufferAttribs);
    EXPECT_NE(pBufferSurface, EGL_NO_SURFACE);

    EGLint ctxAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, mMajorVersion, EGL_NONE};
    EGLContext context  = eglCreateContext(mDisplay, config, nullptr, ctxAttribs);
    EXPECT_NE(context, EGL_NO_CONTEXT);

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, pBufferSurface, pBufferSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    const GLColor clearColor = GLColor::green;
    glClearColor(kFloatGreen.R, kFloatGreen.G, kFloatGreen.B, kFloatGreen.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR() << "glClear failed";

    const GLColor drawColor = clearColor;

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));

    EGLint lockAttribs[] = {EGL_LOCK_USAGE_HINT_KHR, EGL_WRITE_SURFACE_BIT_KHR,
                            EGL_MAP_PRESERVE_PIXELS_KHR, EGL_TRUE, EGL_NONE};
    EXPECT_EGL_TRUE(eglLockSurfaceKHR(mDisplay, pBufferSurface, lockAttribs));

    EGLAttribKHR bitMap = 0;
    EXPECT_EGL_TRUE(
        eglQuerySurface64KHR(mDisplay, pBufferSurface, EGL_BITMAP_POINTER_KHR, &bitMap));
    EGLAttribKHR bitMapPitch = 0;
    uint32_t *bitMapPtr      = (uint32_t *)(bitMap);
    EXPECT_EGL_TRUE(
        eglQuerySurface64KHR(mDisplay, pBufferSurface, EGL_BITMAP_PITCH_KHR, &bitMapPitch));

    EXPECT_TRUE(checkBitMapRGBA32(drawColor, bitMapPtr, bitMapPitch));

    const GLColor fillColor = GLColor::white;
    fillBitMapRGBA32(fillColor, bitMapPtr, bitMapPitch);

    EXPECT_EGL_TRUE(eglUnlockSurfaceKHR(mDisplay, pBufferSurface));

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, pBufferSurface, pBufferSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    EXPECT_TRUE(checkSurfaceRGBA32(fillColor));

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_TRUE(eglDestroySurface(mDisplay, pBufferSurface));
    EXPECT_EGL_TRUE(eglDestroyContext(mDisplay, context));
}

// Create WindowSurface, Clear Color to GREEN, draw red quad, Lock with PRESERVE_PIXELS,
// read/check pixels, Unlock.
TEST_P(EGLLockSurface3Test, WindowSurfaceReadTest)
{
    ANGLE_SKIP_TEST_IF(!supportsLockSurface3Extension());

    EGLint configAttribs[] = {EGL_RED_SIZE,
                              8,
                              EGL_GREEN_SIZE,
                              8,
                              EGL_BLUE_SIZE,
                              8,
                              EGL_ALPHA_SIZE,
                              8,
                              EGL_RENDERABLE_TYPE,
                              (mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT),
                              EGL_SURFACE_TYPE,
                              (EGL_WINDOW_BIT | EGL_LOCK_SURFACE_BIT_KHR),
                              EGL_NONE};
    EGLint count           = 0;
    EGLConfig config       = EGL_NO_CONFIG_KHR;
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, configAttribs, &config, 1, &count));
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);
    EXPECT_GT(count, 0);

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("LockSurfaceTest", kWidth, kHeight);
    EGLint winAttribs[] = {EGL_NONE};
    EGLSurface windowSurface =
        eglCreateWindowSurface(mDisplay, config, osWindow->getNativeWindow(), winAttribs);
    EXPECT_NE(windowSurface, EGL_NO_SURFACE);

    EGLint ctxAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, mMajorVersion, EGL_NONE};
    EGLContext context  = eglCreateContext(mDisplay, config, nullptr, ctxAttribs);
    EXPECT_NE(context, EGL_NO_CONTEXT);

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    glClearColor(kFloatGreen.R, kFloatGreen.G, kFloatGreen.B, kFloatGreen.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR() << "glClear failed";

    const GLColor drawColor = GLColor::red;
    GLuint texture          = createTexture();
    EXPECT_TRUE(fillTexture(texture, drawColor));
    renderTexture(texture);
    glFinish();
    ASSERT_GL_NO_ERROR() << "glFinish failed";

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));

    EGLint lockAttribs[] = {EGL_LOCK_USAGE_HINT_KHR, EGL_READ_SURFACE_BIT_KHR,
                            EGL_MAP_PRESERVE_PIXELS_KHR, EGL_TRUE, EGL_NONE};
    EXPECT_EGL_TRUE(eglLockSurfaceKHR(mDisplay, windowSurface, lockAttribs));

    EGLAttribKHR bitMap = 0;
    EXPECT_EGL_TRUE(eglQuerySurface64KHR(mDisplay, windowSurface, EGL_BITMAP_POINTER_KHR, &bitMap));
    EGLAttribKHR bitMapPitch = 0;
    EXPECT_EGL_TRUE(
        eglQuerySurface64KHR(mDisplay, windowSurface, EGL_BITMAP_PITCH_KHR, &bitMapPitch));

    uint32_t *bitMapPtr = (uint32_t *)(bitMap);
    EXPECT_TRUE(checkBitMapRGBA32(drawColor, bitMapPtr, bitMapPitch));

    EXPECT_TRUE(eglUnlockSurfaceKHR(mDisplay, windowSurface));

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";
    EXPECT_TRUE(checkSurfaceRGBA32(drawColor));
    EXPECT_EGL_TRUE(eglSwapBuffers(mDisplay, windowSurface));

    glDeleteTextures(1, &texture);

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_TRUE(eglDestroySurface(mDisplay, windowSurface));
    EXPECT_EGL_TRUE(eglDestroyContext(mDisplay, context));
    osWindow->destroy();
    OSWindow::Delete(&osWindow);
}

// Test default msaa surface resolve path.
TEST_P(EGLLockSurface3Test, WindowMsaaSurfaceReadTest)
{
    ANGLE_SKIP_TEST_IF(!supportsLockSurface3Extension());

    EGLint configAttribs[] = {EGL_RED_SIZE,
                              8,
                              EGL_GREEN_SIZE,
                              8,
                              EGL_BLUE_SIZE,
                              8,
                              EGL_ALPHA_SIZE,
                              8,
                              EGL_RENDERABLE_TYPE,
                              (mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT),
                              EGL_SAMPLE_BUFFERS,
                              1,
                              EGL_SAMPLES,
                              4,
                              EGL_SURFACE_TYPE,
                              (EGL_WINDOW_BIT | EGL_LOCK_SURFACE_BIT_KHR),
                              EGL_NONE};
    EGLint count           = 0;
    EGLConfig config       = EGL_NO_CONFIG_KHR;
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, configAttribs, &config, 1, &count));
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);
    EXPECT_GT(count, 0);

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("LockSurfaceTest", kWidth, kHeight);
    EGLint winAttribs[] = {EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER, EGL_NONE};
    EGLSurface windowSurface =
        eglCreateWindowSurface(mDisplay, config, osWindow->getNativeWindow(), winAttribs);
    EXPECT_NE(windowSurface, EGL_NO_SURFACE);

    EGLint ctxAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, mMajorVersion, EGL_NONE};
    EGLContext context  = eglCreateContext(mDisplay, config, nullptr, ctxAttribs);
    EXPECT_NE(context, EGL_NO_CONTEXT);

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";

    glClearColor(kFloatGreen.R, kFloatGreen.G, kFloatGreen.B, kFloatGreen.A);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR() << "glClear failed";

    const GLColor drawColor = GLColor::red;
    GLuint texture          = createTexture();
    EXPECT_TRUE(fillTexture(texture, drawColor));
    renderTexture(texture);
    eglSwapBuffers(mDisplay, windowSurface);

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context));

    EGLint lockAttribs[] = {EGL_LOCK_USAGE_HINT_KHR, EGL_READ_SURFACE_BIT_KHR,
                            EGL_MAP_PRESERVE_PIXELS_KHR, EGL_TRUE, EGL_NONE};
    EXPECT_EGL_TRUE(eglLockSurfaceKHR(mDisplay, windowSurface, lockAttribs));

    EGLAttribKHR bitMap = 0;
    EXPECT_EGL_TRUE(eglQuerySurface64KHR(mDisplay, windowSurface, EGL_BITMAP_POINTER_KHR, &bitMap));
    EGLAttribKHR bitMapPitch = 0;
    EXPECT_EGL_TRUE(
        eglQuerySurface64KHR(mDisplay, windowSurface, EGL_BITMAP_PITCH_KHR, &bitMapPitch));

    uint32_t *bitMapPtr = (uint32_t *)(bitMap);
    EXPECT_TRUE(checkBitMapRGBA32(drawColor, bitMapPtr, bitMapPitch));

    EXPECT_TRUE(eglUnlockSurfaceKHR(mDisplay, windowSurface));

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";
    EXPECT_TRUE(checkSurfaceRGBA32(drawColor));
    EXPECT_EGL_TRUE(eglSwapBuffers(mDisplay, windowSurface));

    glDeleteTextures(1, &texture);

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_TRUE(eglDestroySurface(mDisplay, windowSurface));
    EXPECT_EGL_TRUE(eglDestroyContext(mDisplay, context));
    osWindow->destroy();
    OSWindow::Delete(&osWindow);
}

// Create WindowSurface, Lock surface, Write pixels red, Unlock, check pixels,
//  then swapbuffers to visually check.
TEST_P(EGLLockSurface3Test, WindowSurfaceWritePreserveTest)
{
    ANGLE_SKIP_TEST_IF(!supportsLockSurface3Extension());

    EGLint attribs[] = {EGL_RED_SIZE,
                        8,
                        EGL_GREEN_SIZE,
                        8,
                        EGL_BLUE_SIZE,
                        8,
                        EGL_ALPHA_SIZE,
                        8,
                        EGL_RENDERABLE_TYPE,
                        (mMajorVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT),
                        EGL_SURFACE_TYPE,
                        (EGL_WINDOW_BIT | EGL_LOCK_SURFACE_BIT_KHR),
                        EGL_NONE};
    EGLint count     = 0;
    EGLConfig config = EGL_NO_CONFIG_KHR;
    EXPECT_EGL_TRUE(eglChooseConfig(mDisplay, attribs, &config, 1, &count));
    ANGLE_SKIP_TEST_IF(config == EGL_NO_CONFIG_KHR);
    EXPECT_GT(count, 0);

    OSWindow *osWindow = OSWindow::New();
    osWindow->initialize("LockSurfaceTest", kWidth, kHeight);
    EGLint winAttribs[] = {EGL_NONE};
    EGLSurface windowSurface =
        eglCreateWindowSurface(mDisplay, config, osWindow->getNativeWindow(), winAttribs);
    EXPECT_NE(windowSurface, EGL_NO_SURFACE);

    EGLint ctxAttribs[] = {EGL_CONTEXT_MAJOR_VERSION, mMajorVersion, EGL_NONE};
    EGLContext context  = eglCreateContext(mDisplay, config, nullptr, ctxAttribs);
    EXPECT_NE(context, EGL_NO_CONTEXT);

    EGLint lockAttribs[] = {EGL_LOCK_USAGE_HINT_KHR, EGL_READ_SURFACE_BIT_KHR, EGL_NONE};
    EXPECT_EGL_TRUE(eglLockSurfaceKHR(mDisplay, windowSurface, lockAttribs));

    EGLAttribKHR bitMap = 0;
    EXPECT_EGL_TRUE(eglQuerySurface64KHR(mDisplay, windowSurface, EGL_BITMAP_POINTER_KHR, &bitMap));
    EGLAttribKHR bitMapPitch = 0;
    EXPECT_EGL_TRUE(
        eglQuerySurface64KHR(mDisplay, windowSurface, EGL_BITMAP_PITCH_KHR, &bitMapPitch));
    uint32_t *bitMapPtr = (uint32_t *)(bitMap);

    const GLColor fillColor = GLColor::red;
    fillBitMapRGBA32(fillColor, bitMapPtr, bitMapPitch);

    EXPECT_EGL_TRUE(eglUnlockSurfaceKHR(mDisplay, windowSurface));

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, windowSurface, windowSurface, context));
    ASSERT_EGL_SUCCESS() << "eglMakeCurrent failed.";
    EXPECT_TRUE(checkSurfaceRGBA32(fillColor));
    EXPECT_EGL_TRUE(eglSwapBuffers(mDisplay, windowSurface));

    EXPECT_EGL_TRUE(eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
    EXPECT_EGL_TRUE(eglDestroySurface(mDisplay, windowSurface));
    EXPECT_EGL_TRUE(eglDestroyContext(mDisplay, context));
    osWindow->destroy();
    OSWindow::Delete(&osWindow);
}

ANGLE_INSTANTIATE_TEST(EGLLockSurface3Test,
                       WithNoFixture(ES2_VULKAN()),
                       WithNoFixture(ES3_VULKAN()));
