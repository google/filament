//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BootAnimationTest.cpp: Tests that make the same gl calls as Android's boot animations

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "common/debug.h"
#include "util/test_utils.h"

using namespace angle;

// Makes the same GLES 1 calls as Android's default boot animation
// The original animation uses 2 different images -
// One image acts as a mask and one that moves(a gradient that acts as a shining light)
// We do the same here except with different images of much smaller resolution
// The results of each frame of the animation are compared against expected values
// The original source of the boot animation can be found here:
// https://android.googlesource.com/platform/frameworks/base/+/refs/heads/master/cmds/bootanimation/BootAnimation.cpp#422
class BootAnimationTest : public ANGLETest<>
{
  protected:
    BootAnimationTest()
    {
        setWindowWidth(kWindowWidth);
        setWindowHeight(kWindowHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void initTextureWithData(GLuint *texture,
                             const void *data,
                             GLint width,
                             GLint height,
                             unsigned int channels)
    {
        GLint crop[4] = {0, height, width, -height};

        glGenTextures(1, texture);
        glBindTexture(GL_TEXTURE_2D, *texture);

        switch (channels)
        {
            case 3:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                             GL_UNSIGNED_SHORT_5_6_5, data);
                break;
            case 4:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                             data);
                break;
            default:
                UNREACHABLE();
        }

        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    void testSetUp() override
    {
        EGLWindow *window = getEGLWindow();
        mDisplay          = window->getDisplay();
        mSurface          = window->getSurface();

        /**
         * The mask is a 4 by 1 texture colored:
         * --- --- --- ---
         * |B| |A| |B| |A|
         * --- --- --- ---
         * B is black, A is black with alpha of 0xFF
         */
        constexpr GLubyte kMask[] = {
            0x0, 0x0, 0x0, 0xff,  // black
            0x0, 0x0, 0x0, 0x0,   // transparent black
            0x0, 0x0, 0x0, 0xff,  // black
            0x0, 0x0, 0x0, 0x0    // transparent black
        };
        /**
         * The shine is a 8 by 1 texture colored:
         * --- --- --- --- --- --- --- ---
         * |R| |R| |G| |G| |B| |B| |W| |W|
         * --- --- --- --- --- --- --- ---
         * R is red, G is green, B is blue, W is white
         */
        constexpr GLushort kShine[] = {0xF800,  // 2 red pixels
                                       0xF800,
                                       0x07E0,  // 2 green pixels
                                       0x07E0,
                                       0x001F,  // 2 blue pixels
                                       0x001F,
                                       0xFFFF,  // 2 white pixels
                                       0xFFFF};

        constexpr unsigned int kMaskColorChannels  = 4;
        constexpr unsigned int kShineColorChannels = 3;

        initTextureWithData(&mTextureNames[0], kMask, kMaskWidth, kMaskHeight, kMaskColorChannels);
        initTextureWithData(&mTextureNames[1], kShine, kShineWidth, kShineHeight,
                            kShineColorChannels);

        // clear screen
        glShadeModel(GL_FLAT);
        glDisable(GL_DITHER);
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        eglSwapBuffers(mDisplay, mSurface);
        glEnable(GL_TEXTURE_2D);
        glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glScissor(kMaskBoundaryLeft, kMaskBoundaryBottom, kMaskWidth, kMaskHeight);

        // Blend state
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }

    void testTearDown() override
    {
        glDeleteTextures(1, &mTextureNames[0]);
        glDeleteTextures(1, &mTextureNames[1]);
    }

    void checkMaskColor(unsigned int iterationCount, unsigned int maskSlot)
    {
        // kOffset is necessary as the visible part is the left most section of the shine
        // but then we shift the images right
        constexpr unsigned int kOffset = 7;

        // this solves for the color at any given position in our shine equivalent
        constexpr unsigned int kPossibleColors = 4;
        constexpr unsigned int kColorsInARow   = 2;
        unsigned int color =
            ((iterationCount - maskSlot + kOffset) / kColorsInARow) % kPossibleColors;
        switch (color)
        {
            case 0:  // white
                EXPECT_PIXEL_EQ(kMaskBoundaryLeft + maskSlot, kMaskBoundaryBottom, 0xFF, 0xFF, 0xFF,
                                0xFF);
                break;
            case 1:  // blue
                EXPECT_PIXEL_EQ(kMaskBoundaryLeft + maskSlot, kMaskBoundaryBottom, 0x00, 0x00, 0xFF,
                                0xFF);
                break;
            case 2:  // green
                EXPECT_PIXEL_EQ(kMaskBoundaryLeft + maskSlot, kMaskBoundaryBottom, 0x00, 0xFF, 0x00,
                                0xFF);
                break;
            case 3:  // red
                EXPECT_PIXEL_EQ(kMaskBoundaryLeft + maskSlot, kMaskBoundaryBottom, 0xFF, 0x00, 0x00,
                                0xFF);
                break;
            default:
                UNREACHABLE();
        }
    }

    void checkClearColor()
    {
        // Areas outside of the 4x1 mask area should be the clear color due to our glScissor call
        constexpr unsigned int kImageHeight = 1;
        EXPECT_PIXEL_RECT_EQ(0, 0, kWindowWidth, kMaskBoundaryBottom, GLColor::cyan);
        EXPECT_PIXEL_RECT_EQ(0, kMaskBoundaryBottom + kImageHeight, kWindowWidth,
                             (kWindowHeight - (kMaskBoundaryBottom + kImageHeight)), GLColor::cyan);
        EXPECT_PIXEL_RECT_EQ(0, kMaskBoundaryBottom, kMaskBoundaryLeft, kImageHeight,
                             GLColor::cyan);
        EXPECT_PIXEL_RECT_EQ(kMaskBoundaryLeft + kMaskWidth, kMaskBoundaryBottom,
                             (kWindowWidth - (kMaskBoundaryLeft + kMaskWidth)), kImageHeight,
                             GLColor::cyan);
    }

    void validateColors(unsigned int iterationCount)
    {
        // validate all slots in our mask
        for (unsigned int maskSlot = 0; maskSlot < kMaskWidth; ++maskSlot)
        {
            // parts that are blocked in our mask are black
            switch (maskSlot)
            {
                case kBlackMask[0]:
                case kBlackMask[1]:
                    // slots with non zero alpha are black
                    EXPECT_PIXEL_EQ(kMaskBoundaryLeft + maskSlot, kMaskBoundaryBottom, 0x00, 0x00,
                                    0x00, 0xFF);
                    continue;
                default:
                    checkMaskColor(iterationCount, maskSlot);
            }
        }
        // validate surrounding pixels are equal to clear color
        checkClearColor();
    }

    EGLDisplay mDisplay = EGL_NO_DISPLAY;
    EGLSurface mSurface = EGL_NO_SURFACE;
    GLuint mTextureNames[2];
    // This creates a kWindowWidth x kWindowHeight window.
    // A kMaskWidth by kMaskHeight rectangle is lit up by the shine
    // This lit up rectangle is positioned at (kMaskBoundaryLeft, kMaskBoundaryBottom)
    // The border around the area is cleared to GLColor::cyan
    static constexpr GLint kMaskBoundaryLeft    = 15;
    static constexpr GLint kMaskBoundaryBottom  = 7;
    static constexpr unsigned int kMaskWidth    = 4;
    static constexpr unsigned int kMaskHeight   = 1;
    static constexpr unsigned int kShineWidth   = 8;
    static constexpr unsigned int kShineHeight  = 1;
    static constexpr unsigned int kWindowHeight = 16;
    static constexpr unsigned int kWindowWidth  = 32;
    static constexpr unsigned int kBlackMask[2] = {0, 2};
};

TEST_P(BootAnimationTest, DefaultBootAnimation)
{
    // http://anglebug.com/42263653
    ANGLE_SKIP_TEST_IF(IsWindows() && IsNVIDIA() && IsVulkan());

    constexpr uint64_t kMaxIterationCount = 8;  // number of times we shift the shine textures
    constexpr int kStartingShinePosition  = kMaskBoundaryLeft - kShineWidth;
    constexpr int kEndingShinePosition    = kMaskBoundaryLeft;
    GLint x                               = kStartingShinePosition;
    uint64_t iterationCount               = 0;
    do
    {
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_SCISSOR_TEST);
        glDisable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, mTextureNames[1]);
        glDrawTexiOES(x, kMaskBoundaryBottom, 0, kShineWidth, kShineHeight);
        glDrawTexiOES(x + kShineWidth, kMaskBoundaryBottom, 0, kShineWidth, kShineHeight);
        glEnable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, mTextureNames[0]);
        glDrawTexiOES(kMaskBoundaryLeft, kMaskBoundaryBottom, 0, kMaskWidth, kMaskHeight);
        validateColors(iterationCount);
        EGLBoolean res = eglSwapBuffers(mDisplay, mSurface);
        if (res == EGL_FALSE)
        {
            break;
        }

        if (x == kEndingShinePosition)
        {
            x = kStartingShinePosition;
        }
        ++x;
        ++iterationCount;
    } while (iterationCount < kMaxIterationCount);
}

ANGLE_INSTANTIATE_TEST_ES1(BootAnimationTest);