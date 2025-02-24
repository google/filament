//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexBufferOffsetTest.cpp: Test glDrawElements with an offset and an index buffer

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/test_utils.h"

using namespace angle;
enum class UpdateType
{
    SmallUpdate,
    SmallThenBigUpdate,
    BigThenSmallUpdate,
    FullUpdate,
};

class IndexBufferOffsetTest : public ANGLETest<>
{
  protected:
    IndexBufferOffsetTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        constexpr char kVS[] =
            R"(precision highp float;
            attribute vec2 position;

            void main()
            {
                gl_Position = vec4(position, 0.0, 1.0);
            })";

        constexpr char kFS[] =
            R"(precision highp float;
            uniform vec4 color;

            void main()
            {
                gl_FragColor = color;
            })";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram);

        mColorUniformLocation      = glGetUniformLocation(mProgram, "color");
        mPositionAttributeLocation = glGetAttribLocation(mProgram, "position");

        const GLfloat vertices[] = {-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f};
        glGenBuffers(1, &mVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &mIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    }

    void testTearDown() override
    {
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteBuffers(1, &mIndexBuffer);
        glDeleteProgram(mProgram);
    }

    void preTestUpdateBuffer(GLuint framebuffer, GLuint texture, GLuint buffer, GLsizei size)
    {
        GLsizei uboSize = std::max(size, 16);
        const std::vector<uint32_t> initialData((uboSize + 3) / 4, 0x1234567u);

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture,
                               0);

        glBindBuffer(GL_UNIFORM_BUFFER, buffer);
        glBufferData(GL_UNIFORM_BUFFER, uboSize, initialData.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);

        constexpr char kVerifyUBO[] = R"(#version 300 es
precision mediump float;
uniform block {
    uint data;
} ubo;
out vec4 colorOut;
void main()
{
    if (ubo.data == 0x1234567u)
        colorOut = vec4(0, 1.0, 0, 1.0);
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

        ANGLE_GL_PROGRAM(verifyUbo, essl3_shaders::vs::Simple(), kVerifyUBO);

        glDisable(GL_BLEND);
        drawQuad(verifyUbo, essl3_shaders::PositionAttrib(), 0.5);

        EXPECT_GL_NO_ERROR();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void runTest(GLenum type,
                 int typeWidth,
                 void *indexDataIn,
                 UpdateType updateType,
                 bool useBuffersAsUboFirst)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        size_t indexDataWidth = 6 * typeWidth;

        std::vector<GLubyte> indexData(6 * 3 * sizeof(GLuint), 0);
        memcpy(indexData.data() + indexDataWidth, indexDataIn, indexDataWidth);

        GLFramebuffer elementUpdateFbo;
        GLTexture elementUpdateTex;

        if (useBuffersAsUboFirst)
        {
            preTestUpdateBuffer(elementUpdateFbo, elementUpdateTex, mIndexBuffer,
                                3 * indexDataWidth);
        }
        else
        {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * indexDataWidth, nullptr, GL_DYNAMIC_DRAW);
        }

        if (updateType == UpdateType::SmallUpdate)
        {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexDataWidth, indexData.data());
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexDataWidth, indexDataWidth,
                            indexData.data() + indexDataWidth);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 2 * indexDataWidth, indexDataWidth,
                            indexData.data() + 2 * indexDataWidth);
        }
        else if (updateType == UpdateType::SmallThenBigUpdate)
        {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 4, indexData.data());
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 4, 3 * indexDataWidth - 4,
                            indexData.data() + 4);
        }
        else if (updateType == UpdateType::BigThenSmallUpdate)
        {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 3 * indexDataWidth - 4, indexData.data());
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 3 * indexDataWidth - 4, 4,
                            indexData.data() + 3 * indexDataWidth - 4);
        }
        else
        {
            ASSERT_EQ(updateType, UpdateType::FullUpdate);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 3 * indexDataWidth, indexData.data());
        }

        glUseProgram(mProgram);

        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glVertexAttribPointer(mPositionAttributeLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(mPositionAttributeLocation);

        glUniform4f(mColorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);

        for (int i = 0; i < 16; i++)
        {
            glDrawElements(GL_TRIANGLES, 6, type, reinterpret_cast<void *>(indexDataWidth));
            EXPECT_PIXEL_COLOR_EQ(64, 64, GLColor::red);
        }

        if (updateType == UpdateType::SmallUpdate)
        {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexDataWidth, indexDataWidth,
                            indexData.data());
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 2 * indexDataWidth, indexDataWidth,
                            indexData.data() + indexDataWidth);
        }
        else if (updateType == UpdateType::SmallThenBigUpdate)
        {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexDataWidth, 4, indexData.data());
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexDataWidth + 4, 2 * indexDataWidth - 4,
                            indexData.data() + 4);
        }
        else if (updateType == UpdateType::BigThenSmallUpdate)
        {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexDataWidth, 2 * indexDataWidth - 4,
                            indexData.data());
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 3 * indexDataWidth - 4, 4,
                            indexData.data() + 2 * indexDataWidth - 4);
        }
        else
        {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexDataWidth, 2 * indexDataWidth,
                            indexData.data());
        }

        glUniform4f(mColorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
        glDrawElements(GL_TRIANGLES, 6, type, reinterpret_cast<void *>(indexDataWidth * 2));
        EXPECT_PIXEL_COLOR_EQ(64, 64, GLColor::green);

        if (useBuffersAsUboFirst)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, elementUpdateFbo);
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
        }

        EXPECT_GL_NO_ERROR();
        swapBuffers();
    }

    GLuint mProgram;
    GLint mColorUniformLocation;
    GLint mPositionAttributeLocation;
    GLuint mVertexBuffer;
    GLuint mIndexBuffer;
};

class IndexBufferOffsetTestES3 : public IndexBufferOffsetTest
{};

// Test using an offset for an UInt8 index buffer
TEST_P(IndexBufferOffsetTest, UInt8Index)
{
    GLubyte indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_BYTE, 1, indexData, UpdateType::FullUpdate, false);
}

// Test using an offset for an UInt16 index buffer
TEST_P(IndexBufferOffsetTest, UInt16Index)
{
    GLushort indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_SHORT, 2, indexData, UpdateType::FullUpdate, false);
}

// Test using an offset for an UInt32 index buffer
TEST_P(IndexBufferOffsetTest, UInt32Index)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_OES_element_index_uint"));

    GLuint indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_INT, 4, indexData, UpdateType::FullUpdate, false);
}

// Test using an offset for an UInt8 index buffer with small buffer updates
TEST_P(IndexBufferOffsetTest, UInt8IndexSmallUpdates)
{
    GLubyte indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_BYTE, 1, indexData, UpdateType::SmallUpdate, false);
}

// Test using an offset for an UInt16 index buffer with small buffer updates
TEST_P(IndexBufferOffsetTest, UInt16IndexSmallUpdates)
{
    GLushort indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_SHORT, 2, indexData, UpdateType::SmallUpdate, false);
}

// Test using an offset for an UInt32 index buffer with small buffer updates
TEST_P(IndexBufferOffsetTest, UInt32IndexSmallUpdates)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_OES_element_index_uint"));

    GLuint indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_INT, 4, indexData, UpdateType::SmallUpdate, false);
}

// Test using an offset for an UInt8 index buffer after uploading data to a buffer that is in use
TEST_P(IndexBufferOffsetTestES3, UseAsUBOThenUpdateThenUInt8Index)
{
    // http://anglebug.com/42264483
    ANGLE_SKIP_TEST_IF(IsAMD() && IsVulkan() && IsWindows());

    // http://anglebug.com/42264490
    ANGLE_SKIP_TEST_IF(IsVulkan() && (IsPixel2() || IsPixel2XL()));

    GLubyte indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_BYTE, 1, indexData, UpdateType::FullUpdate, true);
}

// Test using an offset for an UInt16 index buffer after uploading data to a buffer that is in use
TEST_P(IndexBufferOffsetTestES3, UseAsUBOThenUpdateThenUInt16Index)
{
    // http://anglebug.com/42264490
    ANGLE_SKIP_TEST_IF(IsVulkan() && (IsPixel2() || IsPixel2XL()));

    GLushort indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_SHORT, 2, indexData, UpdateType::FullUpdate, true);
}

// Test using an offset for an UInt32 index buffer after uploading data to a buffer that is in use
TEST_P(IndexBufferOffsetTestES3, UseAsUBOThenUpdateThenUInt32Index)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_OES_element_index_uint"));

    // http://anglebug.com/42264490
    ANGLE_SKIP_TEST_IF(IsVulkan() && (IsPixel2() || IsPixel2XL()));

    GLuint indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_INT, 4, indexData, UpdateType::FullUpdate, true);
}

// Test using an offset for an UInt8 index buffer after uploading data to a buffer that is in use,
// with small buffer updates
TEST_P(IndexBufferOffsetTestES3, UseAsUBOThenUpdateThenUInt8IndexSmallUpdates)
{
    // http://anglebug.com/42264483
    ANGLE_SKIP_TEST_IF(IsAMD() && IsVulkan() && IsWindows());

    // http://anglebug.com/42264490
    ANGLE_SKIP_TEST_IF(IsVulkan() && (IsPixel2() || IsPixel2XL()));

    GLubyte indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_BYTE, 1, indexData, UpdateType::SmallUpdate, true);
}

// Test using an offset for an UInt16 index buffer after uploading data to a buffer that is in use,
// with small buffer updates
TEST_P(IndexBufferOffsetTestES3, UseAsUBOThenUpdateThenUInt16IndexSmallUpdates)
{
    // http://anglebug.com/42264490
    ANGLE_SKIP_TEST_IF(IsVulkan() && (IsPixel2() || IsPixel2XL()));

    GLushort indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_SHORT, 2, indexData, UpdateType::SmallUpdate, true);
}

// Test using an offset for an UInt32 index buffer after uploading data to a buffer that is in use,
// with small buffer updates
TEST_P(IndexBufferOffsetTestES3, UseAsUBOThenUpdateThenUInt32IndexSmallUpdates)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 &&
                       !IsGLExtensionEnabled("GL_OES_element_index_uint"));

    // http://anglebug.com/42264490
    ANGLE_SKIP_TEST_IF(IsVulkan() && (IsPixel2() || IsPixel2XL()));

    GLuint indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_INT, 4, indexData, UpdateType::SmallUpdate, true);

    // Also test with one subData call with more than half updates
    runTest(GL_UNSIGNED_INT, 4, indexData, UpdateType::SmallThenBigUpdate, true);
    runTest(GL_UNSIGNED_INT, 4, indexData, UpdateType::BigThenSmallUpdate, true);
}

// Uses index buffer offset and 2 drawElement calls one of the other, makes sure the second
// drawElement call will use the correct offset.
TEST_P(IndexBufferOffsetTest, DrawAtDifferentOffsets)
{
    GLushort indexData[] = {0, 1, 2, 1, 2, 3};
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    size_t indexDataWidth = 6 * sizeof(GLushort);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataWidth, indexData, GL_DYNAMIC_DRAW);
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glVertexAttribPointer(mPositionAttributeLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mPositionAttributeLocation);

    glUniform4f(mColorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
                   reinterpret_cast<void *>(indexDataWidth / 2));

    // Check the upper left triangle
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() / 4, GLColor::red);

    // Check the down right triangle
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::red);

    EXPECT_GL_NO_ERROR();
}

// Uses index buffer offset and 2 drawElement calls one of the other, one has aligned
// offset and one doesn't
TEST_P(IndexBufferOffsetTest, DrawAtDifferentOffsetAlignments)
{
    GLubyte indexData8[]   = {0, 1, 0, 1, 2, 3};
    GLushort indexData16[] = {0, 1, 2, 1, 2, 3};
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    size_t indexDataWidth16 = 6 * sizeof(GLushort);

    GLuint buffer[2];
    glGenBuffers(2, buffer);

    glUseProgram(mProgram);
    glUniform4f(mColorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glVertexAttribPointer(mPositionAttributeLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mPositionAttributeLocation);

    // 8 bit index with aligned offset
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData8), indexData8, GL_DYNAMIC_DRAW);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, reinterpret_cast<void *>(2));

    // 16 bits index buffer, which unaligned offset
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataWidth16, indexData16, GL_DYNAMIC_DRAW);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
                   reinterpret_cast<void *>(indexDataWidth16 / 2));

    // Check the upper left triangle
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() / 4, GLColor::red);

    // Check the down right triangle
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::red);

    EXPECT_GL_NO_ERROR();
}

// Uses un-aligned index buffer to draw, the draw call should be ignored
TEST_P(IndexBufferOffsetTest, DrawAtUnAlignedIndexBuffer)
{
    constexpr GLushort indices[6] = {0, 1, 2, 2, 3, 0};
    GLubyte indicesUnaligned[1 + sizeof(indices)];

    /* unalign indices */
    indicesUnaligned[0] = 0;
    for (unsigned long i = 0; i < sizeof(indices); ++i)
    {
        indicesUnaligned[i + 1] = ((GLubyte *)indices)[i];
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(mProgram);
    glUniform4f(mColorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glVertexAttribPointer(mPositionAttributeLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mPositionAttributeLocation);

    GLBuffer buffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesUnaligned), indicesUnaligned,
                 GL_DYNAMIC_DRAW);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(1));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // The draw should be ignored, nothing should been drawn
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::black);

    EXPECT_GL_NO_ERROR();
}

// Draw with the same element buffer, but with two different types of data.
TEST_P(IndexBufferOffsetTest, DrawWithSameBufferButDifferentTypes)
{
    GLubyte indexData8[]   = {0, 1, 2};
    GLushort indexData16[] = {1, 2, 3};
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(mProgram);
    glUniform4f(mColorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glVertexAttribPointer(mPositionAttributeLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mPositionAttributeLocation);

    // Create element buffer and fill offset 0 with data from indexData8 and offset 512 with data
    // from indexData16
    GLBuffer buffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4096, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indexData8), indexData8);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 512, sizeof(indexData16), indexData16);

    // Draw with 8 bit index data
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, reinterpret_cast<void *>(0));
    // Draw with 16 bits index buffer
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(512));

    // Check the upper left triangle
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() / 4, GLColor::red);
    // Check the down right triangle
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::red);

    EXPECT_GL_NO_ERROR();
}

// Draw with GL_LINE_LOOP and followed by GL_TRIANGLES, all using the same element buffer.
TEST_P(IndexBufferOffsetTest, DrawWithSameBufferButDifferentModes)
{
    GLushort indexDataLineLoop[] = {0, 1, 2};
    GLushort indexDataTriangle[] = {1, 2, 3};
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(mProgram);
    glUniform4f(mColorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glVertexAttribPointer(mPositionAttributeLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mPositionAttributeLocation);

    // Create element buffer and fill offset 0 with data from indexData8 and offset 512 with data
    // from indexData16
    GLBuffer buffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4096, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indexDataLineLoop), indexDataLineLoop);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 512, sizeof(indexDataTriangle), indexDataTriangle);

    // Draw line loop
    glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_SHORT, 0);
    // Draw triangle
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(512));

    // Check the down right triangle
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::red);

    EXPECT_GL_NO_ERROR();
}

// Draw with GL_LINE_LOOP and followed by GL_TRIANGLES, all using the same element buffer.
TEST_P(IndexBufferOffsetTest, DrawArraysLineLoopFollowedByDrawElementsTriangle)
{
    GLuint indexDataLineLoop[] = {0, 1, 2};
    GLuint indexDataTriangle[] = {1, 2, 3};
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(mProgram);
    glUniform4f(mColorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glVertexAttribPointer(mPositionAttributeLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mPositionAttributeLocation);

    // Create element buffer and fill offset 0 with data from indexDataLineLoop and offset 512 with
    // data from indexDataTriangle
    GLBuffer buffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4096, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indexDataLineLoop), indexDataLineLoop);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 512, sizeof(indexDataTriangle), indexDataTriangle);

    // First call drawElements with the same primitive and type as the final drawElement call.
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, reinterpret_cast<void *>(0));
    // Then drawArray with line loop to trigger the special handling of line loop.
    glDrawArrays(GL_LINE_LOOP, 0, 3);
    // Finally drawElements with triangle and same type to ensure the element buffer state that was
    // modified by line loop draw call gets restored properly.
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, reinterpret_cast<void *>(512));

    // Check the down right triangle
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::red);

    EXPECT_GL_NO_ERROR();
}

// Uses index buffer offset and 2 drawElement calls one of the other with different counts,
// makes sure the second drawElement call will have its data available.
TEST_P(IndexBufferOffsetTest, DrawWithDifferentCountsSameOffset)
{
    GLubyte indexData[] = {99, 0, 1, 2, 1, 2, 3};
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    size_t indexDataWidth = 7 * sizeof(GLubyte);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataWidth, indexData, GL_DYNAMIC_DRAW);
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glVertexAttribPointer(mPositionAttributeLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mPositionAttributeLocation);

    glUniform4f(mColorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    // The first draw draws the first triangle, and the second draws a quad.
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, reinterpret_cast<void *>(1));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, reinterpret_cast<void *>(1));

    // Check the upper left triangle
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() / 4, GLColor::red);

    // Check the down right triangle
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::red);

    EXPECT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(IndexBufferOffsetTest);

ANGLE_INSTANTIATE_TEST_ES3(IndexBufferOffsetTestES3);
