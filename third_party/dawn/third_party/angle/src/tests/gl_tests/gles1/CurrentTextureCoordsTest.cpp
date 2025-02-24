//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CurrentTextureCoordsTest.cpp: Tests basic usage of glMultiTexCoord4(f|x).

#include "test_utils/ANGLETest.h"

#include "common/vector_utils.h"
#include "test_utils/gl_raii.h"
#include "util/random_utils.h"

#include <array>

#include <stdint.h>

using namespace angle;

using TextureCoord = std::array<float, 4>;

class CurrentTextureCoordsTest : public ANGLETest<>
{
  protected:
    CurrentTextureCoordsTest()
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

// State query: Checks the initial state is correct.
TEST_P(CurrentTextureCoordsTest, InitialState)
{
    GLint maxUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxUnits);
    EXPECT_GL_NO_ERROR();

    const TextureCoord kZero = {0.0f, 0.0f, 0.0f, 0.0f};
    TextureCoord actualTexCoord;

    for (GLint i = 0; i < maxUnits; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        EXPECT_GL_NO_ERROR();
        glGetFloatv(GL_CURRENT_TEXTURE_COORDS, actualTexCoord.data());
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(kZero, actualTexCoord);
    }
}

// Checks that errors are generated if the texture unit specified is invalid.
TEST_P(CurrentTextureCoordsTest, Negative)
{
    GLint maxUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxUnits);
    EXPECT_GL_NO_ERROR();

    glMultiTexCoord4f(GL_TEXTURE0 - 1, 1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glMultiTexCoord4f(GL_TEXTURE0 + maxUnits, 1.0f, 0.0f, 0.0f, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Tests setting and getting the current texture coordinates for each unit and with float/fixed
// inputs.
TEST_P(CurrentTextureCoordsTest, Set)
{
    float epsilon  = 0.00001f;
    GLint maxUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxUnits);
    EXPECT_GL_NO_ERROR();

    TextureCoord actualTexCoord;

    for (int i = 0; i < maxUnits; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glMultiTexCoord4f(GL_TEXTURE0 + i, 0.1f, 0.2f, 0.3f, 0.4f);
        glGetFloatv(GL_CURRENT_TEXTURE_COORDS, actualTexCoord.data());
        EXPECT_EQ((TextureCoord{0.1f, 0.2f, 0.3f, 0.4f}), actualTexCoord);

        glMultiTexCoord4x(GL_TEXTURE0 + i, 0x10000, 0x0, 0x3333, 0x5555);
        glGetFloatv(GL_CURRENT_TEXTURE_COORDS, actualTexCoord.data());
        EXPECT_NEAR(1.0f, actualTexCoord[0], epsilon);
        EXPECT_NEAR(0.0f, actualTexCoord[1], epsilon);
        EXPECT_NEAR(0.2f, actualTexCoord[2], epsilon);
        EXPECT_NEAR(1.0f / 3.0f, actualTexCoord[3], epsilon);
    }
}

ANGLE_INSTANTIATE_TEST_ES1(CurrentTextureCoordsTest);
