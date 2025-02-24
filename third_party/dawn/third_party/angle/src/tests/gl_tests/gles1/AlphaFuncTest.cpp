//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// AlphaFuncTest.cpp: Tests basic usage of glAlphaFunc.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

class AlphaFuncTest : public ANGLETest<>
{
  protected:
    AlphaFuncTest()
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

// Checks that disable / enable works as expected.
TEST_P(AlphaFuncTest, EnableDisable)
{
    EXPECT_GL_FALSE(glIsEnabled(GL_ALPHA_TEST));
    EXPECT_GL_NO_ERROR();

    glEnable(GL_ALPHA_TEST);
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_TRUE(glIsEnabled(GL_ALPHA_TEST));
    EXPECT_GL_NO_ERROR();

    glDisable(GL_ALPHA_TEST);
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_FALSE(glIsEnabled(GL_ALPHA_TEST));
    EXPECT_GL_NO_ERROR();
}

// Negative test: Checks that invalid enums for alpha test function generate the proper GL error.
TEST_P(AlphaFuncTest, SetFuncNegative)
{
    glAlphaFunc((GLenum)0, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glAlphaFunc((GLenum)1, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glAlphaFunc((GLenum)GL_ALPHA, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Query test: Checks that the alpha test ref value can be obtained with glGetFloatv.
TEST_P(AlphaFuncTest, SetFuncGetFloat)
{
    GLfloat alphaTestVal = -1.0f;
    glAlphaFunc(GL_ALWAYS, 0.0f);
    glGetFloatv(GL_ALPHA_TEST_REF, &alphaTestVal);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0.0f, alphaTestVal);

    glAlphaFunc(GL_ALWAYS, 0.4f);
    glGetFloatv(GL_ALPHA_TEST_REF, &alphaTestVal);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0.4f, alphaTestVal);
}

// Query test: Checks that the alpha test ref value can be obtained with glGetIntegerv,
// with proper scaling.
TEST_P(AlphaFuncTest, SetFuncGetInt)
{
    GLint alphaTestVal = -1;
    glAlphaFunc(GL_ALWAYS, 0.0f);
    glGetIntegerv(GL_ALPHA_TEST_REF, &alphaTestVal);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0, alphaTestVal);

    glAlphaFunc(GL_ALWAYS, 1.0f);
    glGetIntegerv(GL_ALPHA_TEST_REF, &alphaTestVal);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(std::numeric_limits<GLint>::max(), alphaTestVal);
}

ANGLE_INSTANTIATE_TEST_ES1(AlphaFuncTest);
