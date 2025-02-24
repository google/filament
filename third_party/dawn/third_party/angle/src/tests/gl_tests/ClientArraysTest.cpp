//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ClientArraysTest.cpp : Tests of the GL_ANGLE_client_arrays extension.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace angle
{

class ClientArraysTest : public ANGLETest<>
{
  protected:
    ClientArraysTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setClientArraysEnabled(false);
    }
};

// Context creation would fail if EGL_ANGLE_create_context_client_arrays was not available so
// the GL extension should always be present
TEST_P(ClientArraysTest, ExtensionStringExposed)
{
    EXPECT_TRUE(IsGLExtensionEnabled("GL_ANGLE_client_arrays"));
}

// Verify that GL_CLIENT_ARRAYS_ANGLE can be queried but not changed
TEST_P(ClientArraysTest, QueryValidation)
{
    GLint intValue = 2;
    glGetIntegerv(GL_CLIENT_ARRAYS_ANGLE, &intValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_FALSE(intValue);

    float floatValue = 2.0f;
    glGetFloatv(GL_CLIENT_ARRAYS_ANGLE, &floatValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0.0f, floatValue);

    GLboolean boolValue = GL_TRUE;
    glGetBooleanv(GL_CLIENT_ARRAYS_ANGLE, &boolValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_FALSE(boolValue);

    EXPECT_GL_FALSE(glIsEnabled(GL_CLIENT_ARRAYS_ANGLE));
    EXPECT_GL_NO_ERROR();

    glEnable(GL_CLIENT_ARRAYS_ANGLE);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glDisable(GL_CLIENT_ARRAYS_ANGLE);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// Test that client-side array buffers are forbidden when client arrays are disabled
TEST_P(ClientArraysTest, ForbidsClientSideArrayBuffer)
{
    const auto &vertices = GetQuadVertices();
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4, vertices.data());
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that client-side element array buffers are forbidden when client arrays are disabled
TEST_P(ClientArraysTest, ForbidsClientSideElementBuffer)
{
    ASSERT_GL_FALSE(glIsEnabled(GL_CLIENT_ARRAYS_ANGLE));

    constexpr char kVS[] =
        "attribute vec3 a_pos;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(a_pos, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());

    GLint posLocation = glGetAttribLocation(program, "a_pos");
    ASSERT_NE(-1, posLocation);
    glUseProgram(program);

    const auto &vertices = GetQuadVertices();

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posLocation);

    const GLubyte indices[] = {0, 1, 2, 3, 4, 5};

    ASSERT_GL_NO_ERROR();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(ClientArraysTest);
}  // namespace angle
