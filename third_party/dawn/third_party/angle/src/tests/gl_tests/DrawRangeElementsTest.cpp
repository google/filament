//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DrawElementsTest:
//   Tests for indexed draws.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

enum class DrawCallVariants
{
    DrawRangeElements,
    DrawRangeElementsBaseVertex,
    DrawRangeElementsBaseVertexEXT,
    DrawRangeElementsBaseVertexOES,
};

class DrawRangeElementsTest : public ANGLETest<>
{
  protected:
    DrawRangeElementsTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void drawRangeElementsVariant(DrawCallVariants drawCallVariant, GLsizei count, GLint baseVertex)
    {
        switch (drawCallVariant)
        {
            case DrawCallVariants::DrawRangeElements:
                glDrawRangeElements(GL_TRIANGLES, 0, 1000, count, GL_UNSIGNED_BYTE, nullptr);
                break;
            case DrawCallVariants::DrawRangeElementsBaseVertex:
                glDrawRangeElementsBaseVertex(GL_TRIANGLES, 0, 1000, count, GL_UNSIGNED_BYTE,
                                              nullptr, baseVertex);
                break;
            case DrawCallVariants::DrawRangeElementsBaseVertexEXT:
                glDrawRangeElementsBaseVertexEXT(GL_TRIANGLES, 0, 1000, count, GL_UNSIGNED_BYTE,
                                                 nullptr, baseVertex);
                break;
            case DrawCallVariants::DrawRangeElementsBaseVertexOES:
                glDrawRangeElementsBaseVertexOES(GL_TRIANGLES, 0, 1000, count, GL_UNSIGNED_BYTE,
                                                 nullptr, baseVertex);
                break;
        }
    }

    void doDrawRangeElementsVariant(DrawCallVariants drawCallVariant)
    {
        constexpr char kVS[] =
            "attribute vec3 a_pos;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(a_pos, 1.0);\n"
            "}\n";

        ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Blue());

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
        ASSERT_GL_NO_ERROR();

        GLBuffer indexBuffer;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

        drawRangeElementsVariant(drawCallVariant, 1, 0);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        // count == 0 so it's fine to have no data in the element array buffer bound.
        drawRangeElementsVariant(drawCallVariant, 0, 0);
        ASSERT_GL_NO_ERROR();
    }
};

class WebGLDrawRangeElementsTest : public DrawRangeElementsTest
{
  public:
    WebGLDrawRangeElementsTest() { setWebGLCompatibilityEnabled(true); }
};

// Test that glDrawRangeElements generates an error when trying to draw from an
// empty element array buffer with count other than 0 and no error when count
// equals 0.
TEST_P(WebGLDrawRangeElementsTest, DrawRangeElementArrayZeroCount)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    doDrawRangeElementsVariant(DrawCallVariants::DrawRangeElements);
}

// Test that glDrawRangeElementsBaseVertex generates an error when trying to
// draw from an empty element array buffer with count other than 0 and no error
// when count equals 0.
TEST_P(WebGLDrawRangeElementsTest, DrawRangeElementBaseVertexArrayZeroCount)
{
    doDrawRangeElementsVariant(DrawCallVariants::DrawRangeElementsBaseVertex);
}

// Test that glDrawRangeElementsBaseVertexEXT generates an error when trying to
// draw from an empty element array buffer with count other than 0 and no error
// when count equals 0.
TEST_P(WebGLDrawRangeElementsTest, DrawRangeElementBaseVertexEXTArrayZeroCount)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_draw_elements_base_vertex"));

    doDrawRangeElementsVariant(DrawCallVariants::DrawRangeElementsBaseVertexEXT);
}

// Test that glDrawRangeElementsBaseVertexOES generates an error when trying to
// draw from an empty element array buffer with count other than 0 and no error
// when count equals 0.
TEST_P(WebGLDrawRangeElementsTest, DrawRangeElementBaseVertexOESArrayZeroCount)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_OES_draw_elements_base_vertex"));

    doDrawRangeElementsVariant(DrawCallVariants::DrawRangeElementsBaseVertexOES);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DrawRangeElementsTest);
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(WebGLDrawRangeElementsTest);

}  // namespace
