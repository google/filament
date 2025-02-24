//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureEnvTest.cpp: Tests basic usage of texture environments.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

class TextureEnvTest : public ANGLETest<>
{
  protected:
    TextureEnvTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void verifyEnvironment(GLenum mode,
                           GLenum combineRgb,
                           GLenum combineAlpha,
                           GLenum src0Rgb,
                           GLenum src0Alpha,
                           GLenum src1Rgb,
                           GLenum src1Alpha,
                           GLenum src2Rgb,
                           GLenum src2Alpha,
                           GLenum op0Rgb,
                           GLenum op0Alpha,
                           GLenum op1Rgb,
                           GLenum op1Alpha,
                           GLenum op2Rgb,
                           GLenum op2Alpha,
                           const GLColor32F &envColor,
                           GLfloat rgbScale,
                           GLfloat alphaScale)
    {

        GLfloat actualParams[4] = {};

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(mode, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_COMBINE_RGB, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(combineRgb, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(combineAlpha, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_SRC0_RGB, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(src0Rgb, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_SRC0_ALPHA, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(src0Alpha, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_SRC1_RGB, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(src1Rgb, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_SRC1_ALPHA, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(src1Alpha, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_SRC2_RGB, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(src2Rgb, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_SRC2_ALPHA, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(src2Alpha, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_OPERAND0_RGB, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(op0Rgb, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(op0Alpha, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_OPERAND1_RGB, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(op1Rgb, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(op1Alpha, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_OPERAND2_RGB, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(op2Rgb, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_GLENUM_EQ(op2Alpha, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(envColor.R, actualParams[0]);
        EXPECT_EQ(envColor.G, actualParams[1]);
        EXPECT_EQ(envColor.B, actualParams[2]);
        EXPECT_EQ(envColor.A, actualParams[3]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_RGB_SCALE, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(rgbScale, actualParams[0]);

        glGetTexEnvfv(GL_TEXTURE_ENV, GL_ALPHA_SCALE, actualParams);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(alphaScale, actualParams[0]);
    }
};

// Initial state check.
TEST_P(TextureEnvTest, InitialState)
{
    GLint numUnits;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &numUnits);
    EXPECT_GL_NO_ERROR();

    for (int i = 0; i < numUnits; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        EXPECT_GL_NO_ERROR();

        verifyEnvironment(GL_MODULATE,                         // envMode
                          GL_MODULATE,                         // combineRgb
                          GL_MODULATE,                         // combineAlpha
                          GL_TEXTURE,                          // src0Rgb
                          GL_TEXTURE,                          // src0Alpha
                          GL_PREVIOUS,                         // src1Rgb
                          GL_PREVIOUS,                         // src1Alpha
                          GL_CONSTANT,                         // src2Rgb
                          GL_CONSTANT,                         // src2Alpha
                          GL_SRC_COLOR,                        // op0Rgb
                          GL_SRC_ALPHA,                        // op0Alpha
                          GL_SRC_COLOR,                        // op1Rgb
                          GL_SRC_ALPHA,                        // op1Alpha
                          GL_SRC_ALPHA,                        // op2Rgb
                          GL_SRC_ALPHA,                        // op2Alpha
                          GLColor32F(0.0f, 0.0f, 0.0f, 0.0f),  // envColor
                          1.0f,                                // rgbScale
                          1.0f                                 // alphaScale
        );
    }
}

// Negative test for parameter names.
TEST_P(TextureEnvTest, NegativeParameter)
{
    glTexEnvfv(0, GL_ALPHA_SCALE, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glTexEnvfv(GL_ALPHA_SCALE, GL_ALPHA_SCALE, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
    glTexEnvfv(GL_TEXTURE_ENV, 0, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Negative test for parameter values.
TEST_P(TextureEnvTest, NegativeValues)
{
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_SRC2_RGB, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_ALPHA, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_SRC2_ALPHA, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, (GLfloat)GL_DOT3_RGB);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, (GLfloat)GL_DOT3_RGBA);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, (GLfloat)GL_SRC_COLOR);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, (GLfloat)GL_ONE_MINUS_SRC_COLOR);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, (GLfloat)GL_SRC_COLOR);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, (GLfloat)GL_ONE_MINUS_SRC_COLOR);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, (GLfloat)GL_SRC_COLOR);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, (GLfloat)GL_ONE_MINUS_SRC_COLOR);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 3.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 3.0f);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Checks that texture environment state can be set.
TEST_P(TextureEnvTest, Set)
{
    const int kTrials = 1000;

    angle::RNG rng(0);

    GLint numUnits;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &numUnits);
    EXPECT_GL_NO_ERROR();

    std::vector<GLenum> validUnits(numUnits);
    for (int i = 0; i < numUnits; i++)
    {
        validUnits[i] = GL_TEXTURE0 + i;
    }

    std::vector<GLenum> validEnvModes = {
        GL_ADD, GL_BLEND, GL_COMBINE, GL_DECAL, GL_MODULATE, GL_REPLACE,
    };

    std::vector<GLenum> validCombineRgbs = {
        GL_MODULATE, GL_REPLACE,     GL_ADD,      GL_ADD_SIGNED,
        GL_SUBTRACT, GL_INTERPOLATE, GL_DOT3_RGB, GL_DOT3_RGBA,
    };

    std::vector<GLenum> validCombineAlphas = {
        GL_MODULATE, GL_REPLACE, GL_ADD, GL_ADD_SIGNED, GL_SUBTRACT, GL_INTERPOLATE,
    };

    std::vector<GLenum> validSrcs = {
        GL_CONSTANT,
        GL_PREVIOUS,
        GL_PRIMARY_COLOR,
        GL_TEXTURE,
    };

    std::vector<GLenum> validOpRgbs = {
        GL_SRC_COLOR,
        GL_ONE_MINUS_SRC_COLOR,
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA,
    };

    std::vector<GLenum> validOpAlphas = {
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA,
    };

    std::vector<GLfloat> validScales = {
        1.0f,
        2.0f,
        4.0f,
    };

    for (int i = 0; i < kTrials; i++)
    {
        GLenum textureUnit  = rng.randomSelect(validUnits);
        GLenum mode         = rng.randomSelect(validEnvModes);
        GLenum combineRgb   = rng.randomSelect(validCombineRgbs);
        GLenum combineAlpha = rng.randomSelect(validCombineAlphas);

        GLenum src0Rgb   = rng.randomSelect(validSrcs);
        GLenum src0Alpha = rng.randomSelect(validSrcs);
        GLenum src1Rgb   = rng.randomSelect(validSrcs);
        GLenum src1Alpha = rng.randomSelect(validSrcs);
        GLenum src2Rgb   = rng.randomSelect(validSrcs);
        GLenum src2Alpha = rng.randomSelect(validSrcs);

        GLenum op0Rgb   = rng.randomSelect(validOpRgbs);
        GLenum op0Alpha = rng.randomSelect(validOpAlphas);
        GLenum op1Rgb   = rng.randomSelect(validOpRgbs);
        GLenum op1Alpha = rng.randomSelect(validOpAlphas);
        GLenum op2Rgb   = rng.randomSelect(validOpRgbs);
        GLenum op2Alpha = rng.randomSelect(validOpAlphas);

        GLColor32F envColor(rng.randomFloatBetween(0.0f, 1.0f), rng.randomFloatBetween(0.0f, 1.0f),
                            rng.randomFloatBetween(0.0f, 1.0f), rng.randomFloatBetween(0.0f, 1.0f));

        GLfloat rgbScale   = rng.randomSelect(validScales);
        GLfloat alphaScale = rng.randomSelect(validScales);

        glActiveTexture(textureUnit);
        EXPECT_GL_NO_ERROR();

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
        EXPECT_GL_NO_ERROR();
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, combineRgb);
        EXPECT_GL_NO_ERROR();
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, combineAlpha);
        EXPECT_GL_NO_ERROR();

        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, src0Rgb);
        EXPECT_GL_NO_ERROR();
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, src0Alpha);
        EXPECT_GL_NO_ERROR();
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, src1Rgb);
        EXPECT_GL_NO_ERROR();
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, src1Alpha);
        EXPECT_GL_NO_ERROR();
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, src2Rgb);
        EXPECT_GL_NO_ERROR();
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, src2Alpha);
        EXPECT_GL_NO_ERROR();

        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, op0Rgb);
        EXPECT_GL_NO_ERROR();
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, op0Alpha);
        EXPECT_GL_NO_ERROR();
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, op1Rgb);
        EXPECT_GL_NO_ERROR();
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, op1Alpha);
        EXPECT_GL_NO_ERROR();
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, op2Rgb);
        EXPECT_GL_NO_ERROR();
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, op2Alpha);
        EXPECT_GL_NO_ERROR();

        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &envColor.R);
        EXPECT_GL_NO_ERROR();

        glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, rgbScale);
        EXPECT_GL_NO_ERROR();

        glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, alphaScale);
        EXPECT_GL_NO_ERROR();

        verifyEnvironment(mode, combineRgb, combineAlpha, src0Rgb, src0Alpha, src1Rgb, src1Alpha,
                          src2Rgb, src2Alpha, op0Rgb, op0Alpha, op1Rgb, op1Alpha, op2Rgb, op2Alpha,
                          envColor, rgbScale, alphaScale);
    }
}

ANGLE_INSTANTIATE_TEST_ES1(TextureEnvTest);
