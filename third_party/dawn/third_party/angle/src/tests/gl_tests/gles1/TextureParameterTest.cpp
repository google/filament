//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureParameterTest.cpp: Tests GLES1-specific usage of glTexParameter.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

class TextureParameterTest : public ANGLETest<>
{
  protected:
    TextureParameterTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }
};

// Initial state check
TEST_P(TextureParameterTest, InitialState)
{
    GLint params[4] = {};

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_NEAREST_MIPMAP_LINEAR, params[0]);

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_LINEAR, params[0]);

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_REPEAT, params[0]);

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_REPEAT, params[0]);

    glGetTexParameteriv(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, params);
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_FALSE(params[0]);

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, params);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(0, params[0]);
    EXPECT_EQ(0, params[1]);
    EXPECT_EQ(0, params[2]);
    EXPECT_EQ(0, params[3]);
}

// Negative test: invalid enum / operation
TEST_P(TextureParameterTest, NegativeEnum)
{
    // Invalid target (not supported)
    glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Invalid parameter name
    glGetTexParameteriv(GL_TEXTURE_2D, 0, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Not enough buffer
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, 3);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Not supported in GLES1
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Checks that GLES1-specific texture parameters can be set.
TEST_P(TextureParameterTest, Set)
{
    GLint params[4] = {};

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    EXPECT_GL_NO_ERROR();

    glGetTexParameteriv(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, params);
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_TRUE(params[0]);

    GLint cropRect[4] = {10, 20, 30, 40};

    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRect);
    EXPECT_GL_NO_ERROR();

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, params);
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(cropRect[i], params[i]);
    }
}

// Make sure we don't improperly cast an int into a float in ANGLE's internals
TEST_P(TextureParameterTest, IntConversionsAndIntBounds)
{
    // Test integers that can't be represented as floats, INT_MIN, and INT_MAX
    constexpr GLint kFirstIntThatCannotBeFloat         = 16777217;
    constexpr unsigned int kParameterLength            = 4;
    constexpr std::array<GLint, kParameterLength> crop = {
        -kFirstIntThatCannotBeFloat, kFirstIntThatCannotBeFloat, std::numeric_limits<GLint>::max(),
        std::numeric_limits<GLint>::min()};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop.data());
    std::array<GLint, kParameterLength> cropStored = {0};
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropStored.data());
    ASSERT_EQ(crop, cropStored);
}

// Check that texture parameters can be set by glTexParameterx, glTexParameterxv
// and get by glGetTexParameterxv.
TEST_P(TextureParameterTest, SetFixedPoint)
{
    std::array<GLfixed, 4> params = {};

    glTexParameterx(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    EXPECT_GL_NO_ERROR();

    glGetTexParameterxv(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, params.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_TRUE(params[0]);

    std::array<GLfixed, 4> cropRect = {0x10000, 0x10000, 0x20000, 0x20000};

    glTexParameterxv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRect.data());
    EXPECT_GL_NO_ERROR();

    glGetTexParameterxv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, params.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(cropRect, params);

    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    EXPECT_GL_NO_ERROR();

    glGetTexParameterxv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_REPEAT, params[0]);

    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    EXPECT_GL_NO_ERROR();

    glGetTexParameterxv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_LINEAR, params[0]);

    if (IsGLExtensionEnabled("GL_OES_texture_mirrored_repeat"))
    {
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

ANGLE_INSTANTIATE_TEST_ES1(TextureParameterTest);
