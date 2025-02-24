//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PointSpriteTest.cpp: Tests basic usage of GLES1 point sprites.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>

using namespace angle;

class PointSpriteTest : public ANGLETest<>
{
  protected:
    PointSpriteTest()
    {
        setWindowWidth(1);
        setWindowHeight(1);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }
};

// Checks that triangles are not treated as point sprites, and that cached state
// is properly invalidated when the primitive mode changes.
TEST_P(PointSpriteTest, TrianglesNotTreatedAsPointSprites)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_POINT_SPRITE_OES);
    glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE);

    glEnable(GL_TEXTURE_2D);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // A 3x3 texture where the top right corner is green. Rendered with
    // triangles + texture coordinate 1,1 this is always green, but rendered as
    // a point sprite, red will be used as we are using nearest filtering and we
    // would be favoring only the red parts of the image in point coordinates.
    GLubyte texture[] = {
        0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00,
        0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00,
    };

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 3, 3, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // The first coordinate, 0.9f 0.9f should be sufficiently shifted to the right
    // that the green part of the point sprite is hidden.
    std::vector<float> mPositions = {
        0.9f, 0.9f, -0.9f, 0.9f, 0.9f, -0.9f,
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glMultiTexCoord4f(GL_TEXTURE0, 1.0f, 1.0f, 0.0f, 0.0f);
    glVertexPointer(2, GL_FLOAT, 0, mPositions.data());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_POINTS, 0, 1);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

ANGLE_INSTANTIATE_TEST_ES1(PointSpriteTest);
