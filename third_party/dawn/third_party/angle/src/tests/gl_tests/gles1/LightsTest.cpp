//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// LightsTest.cpp: Tests basic usage of glLight*.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "common/matrix_utils.h"
#include "common/vector_utils.h"
#include "util/random_utils.h"

#include <stdint.h>

#include <vector>

using namespace angle;

class LightsTest : public ANGLETest<>
{
  protected:
    LightsTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void drawTestQuad();
};

// Check that the initial lighting parameters state is correct,
// including spec minimum for light count.
TEST_P(LightsTest, InitialState)
{
    const GLColor32F kAmbientInitial(0.2f, 0.2f, 0.2f, 1.0f);
    GLboolean kLightModelTwoSideInitial = GL_FALSE;

    GLColor32F lightModelAmbient;
    GLboolean lightModelTwoSide;

    glGetFloatv(GL_LIGHT_MODEL_AMBIENT, &lightModelAmbient.R);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(kAmbientInitial, lightModelAmbient);

    glGetBooleanv(GL_LIGHT_MODEL_TWO_SIDE, &lightModelTwoSide);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(kLightModelTwoSideInitial, lightModelTwoSide);

    EXPECT_GL_FALSE(glIsEnabled(GL_LIGHTING));
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_FALSE(glIsEnabled(GL_NORMALIZE));
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_FALSE(glIsEnabled(GL_RESCALE_NORMAL));
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_FALSE(glIsEnabled(GL_COLOR_MATERIAL));
    EXPECT_GL_NO_ERROR();

    GLint maxLights = 0;
    glGetIntegerv(GL_MAX_LIGHTS, &maxLights);
    EXPECT_GL_NO_ERROR();
    EXPECT_GE(8, maxLights);

    const GLColor32F kLightnAmbient(0.0f, 0.0f, 0.0f, 1.0f);
    const GLColor32F kLightnDiffuse(0.0f, 0.0f, 0.0f, 1.0f);
    const GLColor32F kLightnSpecular(0.0f, 0.0f, 0.0f, 1.0f);
    const GLColor32F kLight0Diffuse(1.0f, 1.0f, 1.0f, 1.0f);
    const GLColor32F kLight0Specular(1.0f, 1.0f, 1.0f, 1.0f);
    const angle::Vector4 kLightnPosition(0.0f, 0.0f, 1.0f, 0.0f);
    const angle::Vector3 kLightnDirection(0.0f, 0.0f, -1.0f);
    const GLfloat kLightnSpotlightExponent    = 0.0f;
    const GLfloat kLightnSpotlightCutoffAngle = 180.0f;
    const GLfloat kLightnAttenuationConst     = 1.0f;
    const GLfloat kLightnAttenuationLinear    = 0.0f;
    const GLfloat kLightnAttenuationQuadratic = 0.0f;

    for (int i = 0; i < maxLights; i++)
    {
        EXPECT_GL_FALSE(glIsEnabled(GL_LIGHT0 + i));
        EXPECT_GL_NO_ERROR();

        GLColor32F actualColor;
        angle::Vector4 actualPosition;
        angle::Vector3 actualDirection;
        GLfloat actualFloatValue;

        glGetLightfv(GL_LIGHT0 + i, GL_AMBIENT, &actualColor.R);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(kLightnAmbient, actualColor);

        glGetLightfv(GL_LIGHT0 + i, GL_DIFFUSE, &actualColor.R);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(i == 0 ? kLight0Diffuse : kLightnDiffuse, actualColor);

        glGetLightfv(GL_LIGHT0 + i, GL_SPECULAR, &actualColor.R);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(i == 0 ? kLight0Specular : kLightnSpecular, actualColor);

        glGetLightfv(GL_LIGHT0 + i, GL_POSITION, actualPosition.data());
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(kLightnPosition, actualPosition);

        glGetLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, actualDirection.data());
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(kLightnDirection, actualDirection);

        glGetLightfv(GL_LIGHT0 + i, GL_SPOT_EXPONENT, &actualFloatValue);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(kLightnSpotlightExponent, actualFloatValue);

        glGetLightfv(GL_LIGHT0 + i, GL_SPOT_CUTOFF, &actualFloatValue);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(kLightnSpotlightCutoffAngle, actualFloatValue);

        glGetLightfv(GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, &actualFloatValue);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(kLightnAttenuationConst, actualFloatValue);

        glGetLightfv(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, &actualFloatValue);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(kLightnAttenuationLinear, actualFloatValue);

        glGetLightfv(GL_LIGHT0 + i, GL_QUADRATIC_ATTENUATION, &actualFloatValue);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(kLightnAttenuationQuadratic, actualFloatValue);
    }
}

// Negative test for invalid parameter names.
TEST_P(LightsTest, NegativeInvalidEnum)
{
    GLint maxLights = 0;
    glGetIntegerv(GL_MAX_LIGHTS, &maxLights);

    glIsEnabled(GL_LIGHT0 + maxLights);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glLightfv(GL_LIGHT0 + maxLights, GL_AMBIENT, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glLightModelfv(GL_LIGHT0, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glLightModelf(GL_LIGHT0, 0.0f);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    for (int i = 0; i < maxLights; i++)
    {
        glLightf(GL_LIGHT0 + i, GL_TEXTURE_2D, 0.0f);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        glLightfv(GL_LIGHT0 + i, GL_TEXTURE_2D, nullptr);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
}

// Negative test for invalid parameter values.
TEST_P(LightsTest, NegativeInvalidValue)
{
    GLint maxLights = 0;
    glGetIntegerv(GL_MAX_LIGHTS, &maxLights);

    std::vector<GLenum> attenuationParams = {
        GL_CONSTANT_ATTENUATION,
        GL_LINEAR_ATTENUATION,
        GL_QUADRATIC_ATTENUATION,
    };

    for (int i = 0; i < maxLights; i++)
    {
        glLightf(GL_LIGHT0 + i, GL_SPOT_EXPONENT, -1.0f);
        EXPECT_GL_ERROR(GL_INVALID_VALUE);
        GLfloat previousVal = -1.0f;
        glGetLightfv(GL_LIGHT0 + i, GL_SPOT_EXPONENT, &previousVal);
        EXPECT_NE(-1.0f, previousVal);

        glLightf(GL_LIGHT0 + i, GL_SPOT_EXPONENT, 128.1f);
        EXPECT_GL_ERROR(GL_INVALID_VALUE);
        previousVal = 128.1f;
        glGetLightfv(GL_LIGHT0 + i, GL_SPOT_EXPONENT, &previousVal);
        EXPECT_NE(128.1f, previousVal);

        glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, -1.0f);
        EXPECT_GL_ERROR(GL_INVALID_VALUE);
        previousVal = -1.0f;
        glGetLightfv(GL_LIGHT0 + i, GL_SPOT_CUTOFF, &previousVal);
        EXPECT_NE(-1.0f, previousVal);

        glLightf(GL_LIGHT0 + i, GL_SPOT_CUTOFF, 90.1f);
        EXPECT_GL_ERROR(GL_INVALID_VALUE);
        previousVal = 90.1f;
        glGetLightfv(GL_LIGHT0 + i, GL_SPOT_CUTOFF, &previousVal);
        EXPECT_NE(90.1f, previousVal);

        for (GLenum pname : attenuationParams)
        {
            glLightf(GL_LIGHT0 + i, pname, -1.0f);
            EXPECT_GL_ERROR(GL_INVALID_VALUE);
            previousVal = -1.0f;
            glGetLightfv(GL_LIGHT0 + i, pname, &previousVal);
            EXPECT_NE(-1.0f, previousVal);
        }
    }
}

// Test to see if we can set and retrieve the light parameters.
TEST_P(LightsTest, Set)
{
    angle::RNG rng(0);

    GLint maxLights = 0;
    glGetIntegerv(GL_MAX_LIGHTS, &maxLights);

    constexpr int kNumTrials = 100;

    GLColor32F actualColor;
    angle::Vector4 actualPosition;
    angle::Vector3 actualDirection;
    GLfloat actualFloatValue;
    GLboolean actualBooleanValue;

    for (int k = 0; k < kNumTrials; ++k)
    {
        const GLColor32F lightModelAmbient(rng.randomFloat(), rng.randomFloat(), rng.randomFloat(),
                                           rng.randomFloat());
        const GLfloat lightModelTwoSide = rng.randomBool() ? 1.0f : 0.0f;

        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, &lightModelAmbient.R);
        EXPECT_GL_NO_ERROR();
        glGetFloatv(GL_LIGHT_MODEL_AMBIENT, &actualColor.R);
        EXPECT_EQ(lightModelAmbient, actualColor);

        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, lightModelTwoSide);
        EXPECT_GL_NO_ERROR();
        glGetFloatv(GL_LIGHT_MODEL_TWO_SIDE, &actualFloatValue);
        EXPECT_EQ(lightModelTwoSide, actualFloatValue);
        glGetBooleanv(GL_LIGHT_MODEL_TWO_SIDE, &actualBooleanValue);
        EXPECT_EQ(lightModelTwoSide == 1.0f ? GL_TRUE : GL_FALSE, actualBooleanValue);

        for (int i = 0; i < maxLights; i++)
        {

            const GLColor32F ambient(rng.randomFloat(), rng.randomFloat(), rng.randomFloat(),
                                     rng.randomFloat());
            const GLColor32F diffuse(rng.randomFloat(), rng.randomFloat(), rng.randomFloat(),
                                     rng.randomFloat());
            const GLColor32F specular(rng.randomFloat(), rng.randomFloat(), rng.randomFloat(),
                                      rng.randomFloat());
            const angle::Vector4 position(rng.randomFloat(), rng.randomFloat(), rng.randomFloat(),
                                          rng.randomFloat());
            const angle::Vector3 direction(rng.randomFloat(), rng.randomFloat(), rng.randomFloat());
            const GLfloat spotlightExponent = rng.randomFloatBetween(0.0f, 128.0f);
            const GLfloat spotlightCutoffAngle =
                rng.randomBool() ? rng.randomFloatBetween(0.0f, 90.0f) : 180.0f;
            const GLfloat attenuationConst     = rng.randomFloatNonnegative();
            const GLfloat attenuationLinear    = rng.randomFloatNonnegative();
            const GLfloat attenuationQuadratic = rng.randomFloatNonnegative();

            glLightfv(GL_LIGHT0 + i, GL_AMBIENT, &ambient.R);
            EXPECT_GL_NO_ERROR();
            glGetLightfv(GL_LIGHT0 + i, GL_AMBIENT, &actualColor.R);
            EXPECT_EQ(ambient, actualColor);

            glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, &diffuse.R);
            EXPECT_GL_NO_ERROR();
            glGetLightfv(GL_LIGHT0 + i, GL_DIFFUSE, &actualColor.R);
            EXPECT_EQ(diffuse, actualColor);

            glLightfv(GL_LIGHT0 + i, GL_SPECULAR, &specular.R);
            EXPECT_GL_NO_ERROR();
            glGetLightfv(GL_LIGHT0 + i, GL_SPECULAR, &actualColor.R);
            EXPECT_EQ(specular, actualColor);

            glLightfv(GL_LIGHT0 + i, GL_POSITION, position.data());
            EXPECT_GL_NO_ERROR();
            glGetLightfv(GL_LIGHT0 + i, GL_POSITION, actualPosition.data());
            EXPECT_EQ(position, actualPosition);

            glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, direction.data());
            EXPECT_GL_NO_ERROR();
            glGetLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, actualDirection.data());
            EXPECT_EQ(direction, actualDirection);

            glLightfv(GL_LIGHT0 + i, GL_SPOT_EXPONENT, &spotlightExponent);
            EXPECT_GL_NO_ERROR();
            glGetLightfv(GL_LIGHT0 + i, GL_SPOT_EXPONENT, &actualFloatValue);
            EXPECT_EQ(spotlightExponent, actualFloatValue);

            glLightfv(GL_LIGHT0 + i, GL_SPOT_CUTOFF, &spotlightCutoffAngle);
            EXPECT_GL_NO_ERROR();
            glGetLightfv(GL_LIGHT0 + i, GL_SPOT_CUTOFF, &actualFloatValue);
            EXPECT_EQ(spotlightCutoffAngle, actualFloatValue);

            glLightfv(GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, &attenuationConst);
            EXPECT_GL_NO_ERROR();
            glGetLightfv(GL_LIGHT0 + i, GL_CONSTANT_ATTENUATION, &actualFloatValue);
            EXPECT_EQ(attenuationConst, actualFloatValue);

            glLightfv(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, &attenuationLinear);
            EXPECT_GL_NO_ERROR();
            glGetLightfv(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, &actualFloatValue);
            EXPECT_EQ(attenuationLinear, actualFloatValue);

            glLightfv(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, &attenuationQuadratic);
            EXPECT_GL_NO_ERROR();
            glGetLightfv(GL_LIGHT0 + i, GL_LINEAR_ATTENUATION, &actualFloatValue);
            EXPECT_EQ(attenuationQuadratic, actualFloatValue);
        }
    }
}

// Check a case that approximates the one caught in the wild
TEST_P(LightsTest, DiffuseGradient)
{
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    std::vector<GLColor> colors;
    for (uint32_t x = 0; x < 1024; x++)
    {
        for (uint32_t y = 0; y < 1024; y++)
        {
            float x_ratio = (float)x / 1024.0f;
            GLubyte v     = (GLubyte)(255u * x_ratio);

            GLColor color = {v, v, v, 255u};
            colors.push_back(color);
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 colors.data());

    glMatrixMode(GL_PROJECTION);

    const GLfloat projectionMatrix[16] = {
        0.615385, 0, 0, 0, 0, 1.333333, 0, 0, 0, 0, 1, 1, 0, 0, -2, 0,
    };
    glLoadMatrixf(projectionMatrix);

    glEnable(GL_LIGHT0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glClearColor(1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0.0f, 0.0f, 32.0f, 32.0f);

    const GLfloat ambient[4]  = {2.0f, 2.0f, 2.0f, 1.0f};
    const GLfloat diffuse[4]  = {1.0f, 1.0f, 1.0f, 1.0f};
    const GLfloat position[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glMatrixMode(GL_MODELVIEW);

    const GLfloat modelMatrix[16] = {
        0.976656, 0.000000, -0.214807, 0.000000, 0.000000,   1.000000, 0.000000,   0.000000,
        0.214807, 0.000000, 0.976656,  0.000000, -96.007507, 0.000000, 200.000000, 1.000000,
    };
    glLoadMatrixf(modelMatrix);

    glBindTexture(GL_TEXTURE_2D, texture);

    std::vector<float> positions = {
        -64.0f, -89.0f, 1.0f, -64.0f, 89.0f, 1.0f, 64.0f, -89.0f, 1.0f, 64.0f, 89.0f, 1.0f,
    };

    std::vector<float> uvs = {
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    };

    std::vector<float> normals = {
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    };

    glVertexPointer(3, GL_FLOAT, 0, positions.data());
    glTexCoordPointer(2, GL_FLOAT, 0, uvs.data());
    glNormalPointer(GL_FLOAT, 0, normals.data());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_NEAR(11, 11, GLColor(29, 29, 29, 255), 1);
}

void LightsTest::drawTestQuad()
{
    struct Vertex
    {
        GLfloat position[3];
        GLfloat normal[3];
    };

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustumf(-1, 1, -1, 1, 5.0, 60.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -8.0f);
    glRotatef(150, 0, 1, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    {
        GLfloat ambientAndDiffuse[4] = {1.0f, 0.0f, 0.0f, 1.0f};
        GLfloat specular[4]          = {0.0f, 0.0f, 10.0f, 1.0f};
        GLfloat shininess            = 2.0f;

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, ambientAndDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }

    std::vector<Vertex> vertices = {
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    };
    glVertexPointer(3, GL_FLOAT, sizeof vertices[0], &vertices[0].position);
    glNormalPointer(GL_FLOAT, sizeof vertices[0], &vertices[0].normal);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size());

    EXPECT_GL_NO_ERROR();
}

// Check smooth lighting
TEST_P(LightsTest, SmoothLitMesh)
{
    {
        GLfloat position[4] = {0.0f, 0.0f, -20.0f, 1.0f};
        GLfloat diffuse[4]  = {0.7f, 0.7f, 0.7f, 1.0f};
        GLfloat specular[4] = {0.1f, 0.1f, 1.0f, 1.0f};

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, position);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    }

    drawTestQuad();
    EXPECT_PIXEL_COLOR_NEAR(16, 16, GLColor(205, 0, 92, 255), 1);
}

// Check flat lighting
TEST_P(LightsTest, FlatLitMesh)
{
    {
        GLfloat position[4] = {0.0f, 0.0f, -20.0f, 1.0f};
        GLfloat diffuse[4]  = {0.7f, 0.7f, 0.7f, 1.0f};
        GLfloat specular[4] = {0.1f, 0.1f, 1.0f, 1.0f};

        glEnable(GL_LIGHTING);
        glShadeModel(GL_FLAT);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, position);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    }

    drawTestQuad();
    EXPECT_PIXEL_COLOR_NEAR(16, 16, GLColor(211, 0, 196, 255), 1);
}

ANGLE_INSTANTIATE_TEST_ES1(LightsTest);
