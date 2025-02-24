//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProvkingVertexTest:
//   Tests on the conformance of the provoking vertex, which applies to flat
//   shading and compatibility with D3D. See the section on 'flatshading'
//   in the ES 3 specs.
//

#include "GLES2/gl2.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/gles_loader_autogen.h"

using namespace angle;

namespace
{

template <typename T>
size_t sizeOfVectorContents(const T &vector)
{
    return vector.size() * sizeof(typename T::value_type);
}

void checkFlatQuadColors(size_t width,
                         size_t height,
                         const GLColor &bottomLeftColor,
                         const GLColor &topRightColor)
{
    for (size_t x = 0; x < width; x += 2)
    {
        EXPECT_PIXEL_COLOR_EQ(x, 0, bottomLeftColor);
        EXPECT_PIXEL_COLOR_EQ(x + 1, height - 1, topRightColor);
    }
}

class ProvokingVertexTest : public ANGLETest<>
{
  protected:
    ProvokingVertexTest()
        : mProgram(0),
          mFramebuffer(0),
          mTexture(0),
          mTransformFeedback(0),
          mBuffer(0),
          mIntAttribLocation(-1)
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        constexpr char kVS[] =
            "#version 300 es\n"
            "in int intAttrib;\n"
            "in vec2 position;\n"
            "flat out int attrib;\n"
            "void main() {\n"
            "  gl_Position = vec4(position, 0, 1);\n"
            "  attrib = intAttrib;\n"
            "}";

        constexpr char kFS[] =
            "#version 300 es\n"
            "flat in int attrib;\n"
            "out int fragColor;\n"
            "void main() {\n"
            "  fragColor = attrib;\n"
            "}";

        std::vector<std::string> tfVaryings;
        tfVaryings.push_back("attrib");
        mProgram = CompileProgramWithTransformFeedback(kVS, kFS, tfVaryings, GL_SEPARATE_ATTRIBS);
        ASSERT_NE(0u, mProgram);

        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32I, getWindowWidth(), getWindowHeight());

        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);

        mIntAttribLocation = glGetAttribLocation(mProgram, "intAttrib");
        ASSERT_NE(-1, mIntAttribLocation);
        glEnableVertexAttribArray(mIntAttribLocation);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
            mProgram = 0;
        }

        if (mFramebuffer != 0)
        {
            glDeleteFramebuffers(1, &mFramebuffer);
            mFramebuffer = 0;
        }

        if (mTexture != 0)
        {
            glDeleteTextures(1, &mTexture);
            mTexture = 0;
        }

        if (mTransformFeedback != 0)
        {
            glDeleteTransformFeedbacks(1, &mTransformFeedback);
            mTransformFeedback = 0;
        }

        if (mBuffer != 0)
        {
            glDeleteBuffers(1, &mBuffer);
            mBuffer = 0;
        }
    }

    GLuint mProgram;
    GLuint mFramebuffer;
    GLuint mTexture;
    GLuint mTransformFeedback;
    GLuint mBuffer;
    GLint mIntAttribLocation;
};

// Test drawing a simple triangle with flat shading, and different valued vertices.
TEST_P(ProvokingVertexTest, FlatTriangle)
{
    GLint vertexData[] = {1, 2, 3, 1, 2, 3};
    glVertexAttribIPointer(mIntAttribLocation, 1, GL_INT, 0, vertexData);

    drawQuad(mProgram, "position", 0.5f);

    GLint pixelValue[4] = {0};
    glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_INT, &pixelValue);

    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(vertexData[2], pixelValue[0]);
}

// Ensure that any provoking vertex shenanigans still gives correct vertex streams.
TEST_P(ProvokingVertexTest, FlatTriWithTransformFeedback)
{
    glGenTransformFeedbacks(1, &mTransformFeedback);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedback);

    glGenBuffers(1, &mBuffer);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 128, nullptr, GL_STREAM_DRAW);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mBuffer);

    GLint vertexData[] = {1, 2, 3, 1, 2, 3};
    glVertexAttribIPointer(mIntAttribLocation, 1, GL_INT, 0, vertexData);

    glUseProgram(mProgram);
    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuad(mProgram, "position", 0.5f);
    glEndTransformFeedback();
    glUseProgram(0);

    GLint pixelValue[4] = {0};
    glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_INT, &pixelValue);

    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(vertexData[2], pixelValue[0]);

    void *mapPointer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(int) * 6, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mapPointer);

    int *mappedInts = static_cast<int *>(mapPointer);
    for (unsigned int cnt = 0; cnt < 6; ++cnt)
    {
        EXPECT_EQ(vertexData[cnt], mappedInts[cnt]);
    }
}

// Test drawing a simple line with flat shading, and different valued vertices.
TEST_P(ProvokingVertexTest, FlatLine)
{
    GLfloat halfPixel = 1.0f / static_cast<GLfloat>(getWindowWidth());

    GLint vertexData[]     = {1, 2};
    GLfloat positionData[] = {-1.0f + halfPixel, -1.0f, -1.0f + halfPixel, 1.0f};

    glVertexAttribIPointer(mIntAttribLocation, 1, GL_INT, 0, vertexData);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, positionData);

    glUseProgram(mProgram);
    glDrawArrays(GL_LINES, 0, 2);

    GLint pixelValue[4] = {0};
    glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_INT, &pixelValue);

    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(vertexData[1], pixelValue[0]);
}

// Test drawing a simple line with flat shading, and different valued vertices.
TEST_P(ProvokingVertexTest, FlatLineWithFirstIndex)
{
    GLfloat halfPixel = 1.0f / static_cast<GLfloat>(getWindowWidth());

    GLint vertexData[]     = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2};
    GLfloat positionData[] = {0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              0,
                              -1.0f + halfPixel,
                              -1.0f,
                              -1.0f + halfPixel,
                              1.0f};

    glVertexAttribIPointer(mIntAttribLocation, 1, GL_INT, 0, vertexData);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, positionData);

    glUseProgram(mProgram);
    glDrawArrays(GL_LINES, 10, 2);

    GLint pixelValue[4] = {0};
    glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_INT, &pixelValue);

    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(vertexData[11], pixelValue[0]);
}

// Test drawing a simple triangle strip with flat shading, and different valued vertices.
TEST_P(ProvokingVertexTest, FlatTriStrip)
{
    GLint vertexData[]     = {1, 2, 3, 4, 5, 6};
    GLfloat positionData[] = {-1.0f, -1.0f, -1.0f, 1.0f,  0.0f, -1.0f,
                              0.0f,  1.0f,  1.0f,  -1.0f, 1.0f, 1.0f};

    glVertexAttribIPointer(mIntAttribLocation, 1, GL_INT, 0, vertexData);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, positionData);

    glUseProgram(mProgram);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

    std::vector<GLint> pixelBuffer(getWindowWidth() * getWindowHeight() * 4, 0);
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA_INTEGER, GL_INT,
                 &pixelBuffer[0]);

    ASSERT_GL_NO_ERROR();

    for (unsigned int triIndex = 0; triIndex < 4; ++triIndex)
    {
        GLfloat sumX = positionData[triIndex * 2 + 0] + positionData[triIndex * 2 + 2] +
                       positionData[triIndex * 2 + 4];
        GLfloat sumY = positionData[triIndex * 2 + 1] + positionData[triIndex * 2 + 3] +
                       positionData[triIndex * 2 + 5];

        float centerX = sumX / 3.0f * 0.5f + 0.5f;
        float centerY = sumY / 3.0f * 0.5f + 0.5f;
        unsigned int pixelX =
            static_cast<unsigned int>(centerX * static_cast<GLfloat>(getWindowWidth()));
        unsigned int pixelY =
            static_cast<unsigned int>(centerY * static_cast<GLfloat>(getWindowHeight()));
        unsigned int pixelIndex = pixelY * getWindowWidth() + pixelX;

        unsigned int provokingVertexIndex = triIndex + 2;

        EXPECT_EQ(vertexData[provokingVertexIndex], pixelBuffer[pixelIndex * 4]);
    }
}

// Test drawing an indexed triangle strip with flat shading and primitive restart.
TEST_P(ProvokingVertexTest, FlatTriStripPrimitiveRestart)
{
    // TODO(jmadill): Implement on the D3D back-end.
    ANGLE_SKIP_TEST_IF(IsD3D11());

    GLint indexData[]      = {0, 1, 2, -1, 1, 2, 3, 4, -1, 3, 4, 5};
    GLint vertexData[]     = {1, 2, 3, 4, 5, 6};
    GLfloat positionData[] = {-1.0f, -1.0f, -1.0f, 1.0f,  0.0f, -1.0f,
                              0.0f,  1.0f,  1.0f,  -1.0f, 1.0f, 1.0f};

    glVertexAttribIPointer(mIntAttribLocation, 1, GL_INT, 0, vertexData);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, positionData);

    glDisable(GL_CULL_FACE);
    glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    glUseProgram(mProgram);
    glDrawElements(GL_TRIANGLE_STRIP, 12, GL_UNSIGNED_INT, indexData);

    std::vector<GLint> pixelBuffer(getWindowWidth() * getWindowHeight() * 4, 0);
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA_INTEGER, GL_INT,
                 &pixelBuffer[0]);

    ASSERT_GL_NO_ERROR();

    // Account for primitive restart when checking the tris.
    GLint triOffsets[] = {0, 4, 5, 9};

    for (unsigned int triIndex = 0; triIndex < 4; ++triIndex)
    {
        GLint vertexA = indexData[triOffsets[triIndex] + 0];
        GLint vertexB = indexData[triOffsets[triIndex] + 1];
        GLint vertexC = indexData[triOffsets[triIndex] + 2];

        GLfloat sumX =
            positionData[vertexA * 2] + positionData[vertexB * 2] + positionData[vertexC * 2];
        GLfloat sumY = positionData[vertexA * 2 + 1] + positionData[vertexB * 2 + 1] +
                       positionData[vertexC * 2 + 1];

        float centerX = sumX / 3.0f * 0.5f + 0.5f;
        float centerY = sumY / 3.0f * 0.5f + 0.5f;
        unsigned int pixelX =
            static_cast<unsigned int>(centerX * static_cast<GLfloat>(getWindowWidth()));
        unsigned int pixelY =
            static_cast<unsigned int>(centerY * static_cast<GLfloat>(getWindowHeight()));
        unsigned int pixelIndex = pixelY * getWindowWidth() + pixelX;

        unsigned int provokingVertexIndex = triIndex + 2;

        EXPECT_EQ(vertexData[provokingVertexIndex], pixelBuffer[pixelIndex * 4]);
    }
}

// Test with FRONT_CONVENTION if we have ANGLE_provoking_vertex.
TEST_P(ProvokingVertexTest, ANGLEProvokingVertex)
{
    int32_t vertexData[] = {1, 2, 3};
    float positionData[] = {-1.0f, -1.0f, 3.0f, -1.0f, -1.0f, 3.0f};

    glEnableVertexAttribArray(mIntAttribLocation);
    glVertexAttribIPointer(mIntAttribLocation, 1, GL_INT, 0, vertexData);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, positionData);

    glUseProgram(mProgram);
    ASSERT_GL_NO_ERROR();

    const auto &fnExpectId = [&](int id) {
        const int32_t zero[4] = {};
        glClearBufferiv(GL_COLOR, 0, zero);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        int32_t pixelValue[4] = {0};
        glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_INT, &pixelValue);

        ASSERT_GL_NO_ERROR();
        EXPECT_EQ(vertexData[id], pixelValue[0]);
    };

    fnExpectId(2);

    const bool hasExt = IsGLExtensionEnabled("GL_ANGLE_provoking_vertex");
    if (IsD3D11())
    {
        EXPECT_TRUE(hasExt);
    }
    if (hasExt)
    {
        GLint mode;
        glGetIntegerv(GL_PROVOKING_VERTEX_ANGLE, &mode);
        EXPECT_EQ(mode, GL_LAST_VERTEX_CONVENTION_ANGLE);

        glProvokingVertexANGLE(GL_FIRST_VERTEX_CONVENTION_ANGLE);
        glGetIntegerv(GL_PROVOKING_VERTEX_ANGLE, &mode);
        EXPECT_EQ(mode, GL_FIRST_VERTEX_CONVENTION_ANGLE);

        fnExpectId(0);
    }
}

// Tests that alternating between drawing with flat and interpolated varyings works.
TEST_P(ProvokingVertexTest, DrawWithBothFlatAndInterpolatedVarying)
{
    constexpr char kFlatVS[] = R"(#version 300 es
      layout(location = 0) in vec4 position;
      layout(location = 1) in vec4 color;
      flat out highp vec4 v_color;
      void main() {
        gl_Position = position;
        v_color = color;
      }
    )";

    constexpr char kFlatFS[] = R"(#version 300 es
        precision highp float;
        flat in highp vec4 v_color;
        out vec4 fragColor;
        void main() {
          fragColor = v_color;
        }
    )";

    constexpr char kVS[] = R"(#version 300 es
      layout(location = 0) in vec4 position;
      layout(location = 1) in vec4 color;
      out vec4 v_color;
      void main() {
        gl_Position = position;
        v_color = color;
      }
    )";

    constexpr char kFS[] = R"(#version 300 es
        precision highp float;
        in vec4 v_color;
        out vec4 fragColor;
        void main() {
          fragColor = v_color;
        }
    )";

    GLProgram varyingProgram;
    varyingProgram.makeRaster(kVS, kFS);
    GLProgram mProgram;
    mProgram.makeRaster(kFlatVS, kFlatFS);

    constexpr GLuint posNdx   = 0;
    constexpr GLuint colorNdx = 1;

    static float positions[] = {
        -1, -1, 1, -1, -1, 1,
    };

    static GLColor colors[] = {
        GLColor::red,
        GLColor::green,
        GLColor::blue,
    };

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    GLBuffer mPositionBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(posNdx);
    glVertexAttribPointer(posNdx, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    GLBuffer mColorBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(colorNdx);
    glVertexAttribPointer(colorNdx, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);

    glUseProgram(varyingProgram);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glUseProgram(mProgram);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glUseProgram(varyingProgram);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glUseProgram(mProgram);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    ASSERT_GL_NO_ERROR();
}

class ProvokingVertexBufferUpdateTest : public ANGLETest<>
{
  protected:
    static constexpr size_t kWidth  = 64;
    static constexpr size_t kHeight = 64;
    ProvokingVertexBufferUpdateTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        constexpr char kFlatVS[] = R"(#version 300 es
          layout(location = 0) in vec4 position;
          layout(location = 1) in vec4 color;
          flat out highp vec4 v_color;
          void main() {
            gl_Position = position;
            v_color = color;
          }
        )";

        constexpr char kFlatFS[] = R"(#version 300 es
            precision highp float;
            flat in highp vec4 v_color;
            out vec4 fragColor;
            void main() {
              fragColor = v_color;
            }
        )";

        mProgram.makeRaster(kFlatVS, kFlatFS);
        glUseProgram(mProgram);

        ASSERT(kWidth % 2 == 0);
        ASSERT(kHeight > 1);

        constexpr GLuint posNdx   = 0;
        constexpr GLuint colorNdx = 1;

        const size_t numQuads        = kWidth / 2;
        const size_t numVertsPerQuad = 4;
        std::vector<Vector2> positions;
        std::vector<GLColor> colors;

        const size_t mNumVertsToDrawPerQuad = 6;
        mNumVertsToDraw                     = mNumVertsToDrawPerQuad * numQuads;

        for (size_t i = 0; i < numQuads; ++i)
        {
            float x0 = float(i + 0) / float(numQuads) * 2.0f - 1.0f;
            float x1 = float(i + 1) / float(numQuads) * 2.0f - 1.0f;

            positions.push_back(Vector2(x0, -1));  // 2--3
            positions.push_back(Vector2(x1, -1));  // |  |
            positions.push_back(Vector2(x0, 1));   // 0--1
            positions.push_back(Vector2(x1, 1));

            colors.push_back(GLColor::red);
            colors.push_back(GLColor::green);
            colors.push_back(GLColor::blue);
            colors.push_back(GLColor::yellow);

            size_t offset = i * numVertsPerQuad;

            mIndicesBlueYellow.push_back(offset + 0);
            mIndicesBlueYellow.push_back(offset + 1);
            mIndicesBlueYellow.push_back(offset + 2);  // blue
            mIndicesBlueYellow.push_back(offset + 2);
            mIndicesBlueYellow.push_back(offset + 1);
            mIndicesBlueYellow.push_back(offset + 3);  // yellow

            mIndicesRedGreen.push_back(offset + 1);
            mIndicesRedGreen.push_back(offset + 2);
            mIndicesRedGreen.push_back(offset + 0);  // red
            mIndicesRedGreen.push_back(offset + 3);
            mIndicesRedGreen.push_back(offset + 2);
            mIndicesRedGreen.push_back(offset + 1);  // green

            mIndicesGreenBlue.push_back(offset + 2);
            mIndicesGreenBlue.push_back(offset + 0);
            mIndicesGreenBlue.push_back(offset + 1);  // green
            mIndicesGreenBlue.push_back(offset + 1);
            mIndicesGreenBlue.push_back(offset + 3);
            mIndicesGreenBlue.push_back(offset + 2);  // blue
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindBuffer(GL_ARRAY_BUFFER, mPositionBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeOfVectorContents(positions), positions.data(),
                     GL_STATIC_DRAW);
        glEnableVertexAttribArray(posNdx);
        glVertexAttribPointer(posNdx, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeOfVectorContents(colors), colors.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(colorNdx);
        glVertexAttribPointer(colorNdx, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override {}

    size_t mNumVertsToDraw;
    GLProgram mProgram;
    GLBuffer mPositionBuffer;
    GLBuffer mColorBuffer;
    GLBuffer mIndexBuffer;
    std::vector<GLushort> mIndicesBlueYellow;
    std::vector<GLushort> mIndicesRedGreen;
    std::vector<GLushort> mIndicesGreenBlue;
};

// Tests that updating the index buffer via BufferData more than once works with flat interpolation
// The backend may queue the updates so the test tests that we draw after the buffer has been
// updated.
TEST_P(ProvokingVertexBufferUpdateTest, DrawFlatWith2BufferUpdates)
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfVectorContents(mIndicesBlueYellow),
                 mIndicesBlueYellow.data(), GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfVectorContents(mIndicesRedGreen),
                 mIndicesRedGreen.data(), GL_STREAM_DRAW);
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    checkFlatQuadColors(kWidth, kHeight, GLColor::red, GLColor::green);
}

// Tests that updating the index buffer more than once with a draw in between updates works with
// flat interpolation The backend may queue the updates so the test tests that we draw after the
// buffer has been updated.
TEST_P(ProvokingVertexBufferUpdateTest, DrawFlatWithBufferUpdateBetweenDraws)
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfVectorContents(mIndicesBlueYellow),
                 mIndicesBlueYellow.data(), GL_STREAM_DRAW);
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfVectorContents(mIndicesRedGreen),
                 mIndicesRedGreen.data(), GL_STREAM_DRAW);
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    checkFlatQuadColors(kWidth, kHeight, GLColor::red, GLColor::green);
}

// Tests that updating the index buffer with BufferSubData works with flat interpolation
// The backend may queue the updates so the test tests that we draw after the buffer has been
// updated.
TEST_P(ProvokingVertexBufferUpdateTest, DrawFlatWithBufferSubUpdate)
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfVectorContents(mIndicesBlueYellow),
                 mIndicesBlueYellow.data(), GL_STREAM_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeOfVectorContents(mIndicesRedGreen),
                    mIndicesRedGreen.data());
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    checkFlatQuadColors(kWidth, kHeight, GLColor::red, GLColor::green);
}

// Tests that updating the index buffer with BufferSubData after drawing works with flat
// interpolation The backend may queue the updates so the test tests that we draw after the buffer
// has been updated.
TEST_P(ProvokingVertexBufferUpdateTest, DrawFlatWithBufferSubUpdateBetweenDraws)
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfVectorContents(mIndicesBlueYellow),
                 mIndicesBlueYellow.data(), GL_STREAM_DRAW);
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeOfVectorContents(mIndicesRedGreen),
                    mIndicesRedGreen.data());
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    checkFlatQuadColors(kWidth, kHeight, GLColor::red, GLColor::green);
}

// Tests that updating the index buffer twice with BufferSubData works with flat interpolation
// The backend may queue the updates so the test tests that we draw after the buffer has been
// updated.
TEST_P(ProvokingVertexBufferUpdateTest, DrawFlatWith2BufferSubUpdates)
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfVectorContents(mIndicesBlueYellow),
                 mIndicesBlueYellow.data(), GL_STREAM_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeOfVectorContents(mIndicesRedGreen),
                    mIndicesRedGreen.data());
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeOfVectorContents(mIndicesGreenBlue),
                    mIndicesGreenBlue.data());
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    checkFlatQuadColors(kWidth, kHeight, GLColor::green, GLColor::blue);
}

// Tests that updating the index buffer with BufferSubData works after drawing with flat
// interpolation The backend may queue the updates so the test tests that we draw after the buffer
// has been updated.
TEST_P(ProvokingVertexBufferUpdateTest, DrawFlatWith2BufferSubUpdatesBetweenDraws)
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfVectorContents(mIndicesBlueYellow),
                 mIndicesBlueYellow.data(), GL_STREAM_DRAW);
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeOfVectorContents(mIndicesRedGreen),
                    mIndicesRedGreen.data());
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeOfVectorContents(mIndicesGreenBlue),
                    mIndicesGreenBlue.data());
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    checkFlatQuadColors(kWidth, kHeight, GLColor::green, GLColor::blue);
}

// Tests that updating the index buffer via multiple calls to BufferSubData works with flat
// interpolation The backend may queue the updates so the test tests that we draw after the buffer
// has been updated.
TEST_P(ProvokingVertexBufferUpdateTest, DrawFlatWithPartialBufferSubUpdates)
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfVectorContents(mIndicesBlueYellow),
                 mIndicesBlueYellow.data(), GL_STREAM_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeOfVectorContents(mIndicesRedGreen) / 2,
                    mIndicesRedGreen.data());
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeOfVectorContents(mIndicesRedGreen) / 2,
                    sizeOfVectorContents(mIndicesRedGreen) / 2,
                    &mIndicesRedGreen[mIndicesRedGreen.size() / 2]);
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    checkFlatQuadColors(kWidth, kHeight, GLColor::red, GLColor::green);
}

// Tests that updating the index buffer via multiple calls to BufferSubData and drawing before the
// update works with flat interpolation The backend may queue the updates so the test tests that we
// draw after the buffer has been updated.
TEST_P(ProvokingVertexBufferUpdateTest, DrawFlatWithPartialBufferSubUpdatesBetweenDraws)
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeOfVectorContents(mIndicesBlueYellow),
                 mIndicesBlueYellow.data(), GL_STREAM_DRAW);
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeOfVectorContents(mIndicesRedGreen) / 2,
                    mIndicesRedGreen.data());
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeOfVectorContents(mIndicesRedGreen) / 2,
                    sizeOfVectorContents(mIndicesRedGreen) / 2,
                    &mIndicesRedGreen[mIndicesRedGreen.size() / 2]);
    glDrawElements(GL_TRIANGLES, mNumVertsToDraw, GL_UNSIGNED_SHORT, nullptr);
    checkFlatQuadColors(kWidth, kHeight, GLColor::red, GLColor::green);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProvokingVertexTest);
ANGLE_INSTANTIATE_TEST(ProvokingVertexTest, ES3_D3D11(), ES3_OPENGL(), ES3_OPENGLES(), ES3_METAL());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProvokingVertexBufferUpdateTest);
ANGLE_INSTANTIATE_TEST(ProvokingVertexBufferUpdateTest,
                       ES3_D3D11(),
                       ES3_OPENGL(),
                       ES3_OPENGLES(),
                       ES3_METAL());

}  // anonymous namespace
