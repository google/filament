//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MaterialsTest.cpp: Tests basic usage of glMaterial*.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>

#include <vector>

using namespace angle;

class MaterialsTest : public ANGLETest<>
{
  protected:
    MaterialsTest()
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

// Check that the initial material state is correct.
TEST_P(MaterialsTest, InitialState)
{
    const GLColor32F kAmbientInitial(0.2f, 0.2f, 0.2f, 1.0f);
    const GLColor32F kDiffuseInitial(0.8f, 0.8f, 0.8f, 1.0f);
    const GLColor32F kSpecularInitial(0.0f, 0.0f, 0.0f, 1.0f);
    const GLColor32F kEmissiveInitial(0.0f, 0.0f, 0.0f, 1.0f);
    const float kShininessInitial = 0.0f;

    GLColor32F actualColor;
    float actualShininess;

    std::vector<GLenum> pnames = {
        GL_AMBIENT,
        GL_DIFFUSE,
        GL_SPECULAR,
        GL_EMISSION,
    };

    std::vector<GLColor32F> colors = {
        kAmbientInitial,
        kDiffuseInitial,
        kSpecularInitial,
        kEmissiveInitial,
    };

    for (size_t i = 0; i < pnames.size(); i++)
    {
        glGetMaterialfv(GL_FRONT, pnames[i], &actualColor.R);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(colors[i], actualColor);
    }

    glGetMaterialfv(GL_FRONT, GL_SHININESS, &actualShininess);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(kShininessInitial, actualShininess);
}

// Check for invalid parameter names.
TEST_P(MaterialsTest, InvalidParameter)
{
    glGetMaterialfv(GL_FRONT, 0, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glGetMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glMaterialf(GL_FRONT_AND_BACK, GL_AMBIENT, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glMaterialfv(GL_FRONT, GL_AMBIENT, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Check that material parameters can be set.
TEST_P(MaterialsTest, SetParameters)
{
    const GLColor32F kAmbientTestValue(1.0f, 1.0f, 1.0f, 1.0f);
    const GLColor32F kDiffuseTestValue(0.0f, 0.0f, 0.0f, 0.0f);
    const GLColor32F kSpecularTestValue(0.5f, 0.5f, 0.5f, 0.5f);
    const GLColor32F kEmissiveTestValue(0.4f, 0.4f, 0.4f, 0.4f);
    const float kShininessTestValue = 1.0f;

    std::vector<GLenum> pnames = {
        GL_AMBIENT,
        GL_DIFFUSE,
        GL_SPECULAR,
        GL_EMISSION,
    };

    std::vector<GLColor32F> colors = {
        kAmbientTestValue,
        kDiffuseTestValue,
        kSpecularTestValue,
        kEmissiveTestValue,
    };

    GLColor32F actualColor;
    float actualShininess;

    for (size_t i = 0; i < pnames.size(); i++)
    {
        glMaterialfv(GL_FRONT_AND_BACK, pnames[i], &colors[i].R);
        EXPECT_GL_NO_ERROR();
        glGetMaterialfv(GL_FRONT, pnames[i], &actualColor.R);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(colors[i], actualColor);
    }

    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, kShininessTestValue);
    EXPECT_GL_NO_ERROR();
    glGetMaterialfv(GL_FRONT, GL_SHININESS, &actualShininess);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(kShininessTestValue, actualShininess);
}

ANGLE_INSTANTIATE_TEST_ES1(MaterialsTest);
