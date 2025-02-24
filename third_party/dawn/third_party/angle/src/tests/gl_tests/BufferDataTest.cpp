//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "util/random_utils.h"

#include <stdint.h>
#include <thread>

using namespace angle;

class BufferDataTest : public ANGLETest<>
{
  protected:
    BufferDataTest()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);

        mBuffer         = 0;
        mProgram        = 0;
        mAttribLocation = -1;
    }

    void testSetUp() override
    {
        constexpr char kVS[] = R"(attribute vec4 position;
attribute float in_attrib;
varying float v_attrib;
void main()
{
    v_attrib = in_attrib;
    gl_Position = position;
})";

        constexpr char kFS[] = R"(precision mediump float;
varying float v_attrib;
void main()
{
    gl_FragColor = vec4(v_attrib, 0, 0, 1);
})";

        glGenBuffers(1, &mBuffer);
        ASSERT_NE(mBuffer, 0U);

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(mProgram, 0U);

        mAttribLocation = glGetAttribLocation(mProgram, "in_attrib");
        ASSERT_NE(mAttribLocation, -1);

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteBuffers(1, &mBuffer);
        glDeleteProgram(mProgram);
    }

    GLuint mBuffer;
    GLuint mProgram;
    GLint mAttribLocation;
};

// If glBufferData was not called yet the capturing must not try to
// read the data. http://anglebug.com/42264622
TEST_P(BufferDataTest, Uninitialized)
{
    // Trigger frame capture to try capturing the
    // generated but uninitialized buffer
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    swapBuffers();
}

TEST_P(BufferDataTest, ZeroNonNULLData)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    EXPECT_GL_NO_ERROR();

    char *zeroData = new char[0];
    glBufferData(GL_ARRAY_BUFFER, 0, zeroData, GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    glBufferSubData(GL_ARRAY_BUFFER, 0, 0, zeroData);
    EXPECT_GL_NO_ERROR();

    delete[] zeroData;
}

TEST_P(BufferDataTest, NULLResolvedData)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, 128, nullptr, GL_DYNAMIC_DRAW);

    glUseProgram(mProgram);
    glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 4, nullptr);
    glEnableVertexAttribArray(mAttribLocation);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    drawQuad(mProgram, "position", 0.5f);
}

// Internally in D3D, we promote dynamic data to static after many draw loops. This code tests
// path.
TEST_P(BufferDataTest, RepeatedDrawWithDynamic)
{
    std::vector<GLfloat> data;
    for (int i = 0; i < 16; ++i)
    {
        data.push_back(static_cast<GLfloat>(i));
    }

    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(mAttribLocation);

    for (int drawCount = 0; drawCount < 40; ++drawCount)
    {
        drawQuad(mProgram, "position", 0.5f);
    }

    EXPECT_GL_NO_ERROR();
}

// Tests for a bug where vertex attribute translation was not being invalidated when switching to
// DYNAMIC
TEST_P(BufferDataTest, RepeatedDrawDynamicBug)
{
    // http://anglebug.com/42261546: Seems to be an Intel driver bug.
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsIntel() && IsWindows());

    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);

    auto quadVertices = GetQuadVertices();
    for (angle::Vector3 &vertex : quadVertices)
    {
        vertex.x() *= 1.0f;
        vertex.y() *= 1.0f;
        vertex.z() = 0.0f;
    }

    // Set up quad vertices with DYNAMIC data
    GLBuffer positionBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * quadVertices.size() * 3, quadVertices.data(),
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    EXPECT_GL_NO_ERROR();

    // Set up color data so red is drawn
    std::vector<GLfloat> data(6, 1.0f);

    // Set data to DYNAMIC
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(mAttribLocation);
    EXPECT_GL_NO_ERROR();

    // Draw enough times to promote data to DIRECT mode
    for (int i = 0; i < 20; i++)
    {
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // Verify red was drawn
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Set up color value so black is drawn
    std::fill(data.begin(), data.end(), 0.0f);

    // Update the data, changing back to DYNAMIC mode.
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), GL_DYNAMIC_DRAW);

    // This draw should produce a black quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
    EXPECT_GL_NO_ERROR();
}

using BufferSubDataTestParams = std::tuple<angle::PlatformParameters, bool>;

std::string BufferSubDataTestPrint(
    const ::testing::TestParamInfo<BufferSubDataTestParams> &paramsInfo)
{
    const BufferSubDataTestParams &params = paramsInfo.param;
    std::ostringstream out;

    out << std::get<0>(params) << "__";

    const bool useCopySubData = std::get<1>(params);
    if (useCopySubData)
    {
        out << "CopyBufferSubData";
    }
    else
    {
        out << "BufferSubData";
    }

    return out.str();
}

class BufferSubDataTest : public ANGLETest<BufferSubDataTestParams>
{
  protected:
    BufferSubDataTest()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);

        mBuffer = 0;
    }

    void testSetUp() override
    {
        glGenBuffers(1, &mBuffer);
        ASSERT_NE(mBuffer, 0U);

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        ASSERT_GL_NO_ERROR();
    }

    void updateBuffer(GLenum target, GLintptr offset, GLsizeiptr size, const void *data)
    {
        const bool useCopySubData = std::get<1>(GetParam());
        if (!useCopySubData)
        {
            // If using glBufferSubData, directly upload data on the specified target (where the
            // buffer is already bound)
            glBufferSubData(target, offset, size, data);
        }
        else
        {
            // Otherwise copy through a temp buffer.  Use a non-zero offset for more coverage.
            constexpr GLintptr kStagingOffset = 935;
            GLBuffer staging;
            glBindBuffer(GL_COPY_READ_BUFFER, staging);
            glBufferData(GL_COPY_READ_BUFFER, offset + size + kStagingOffset * 3 / 2, nullptr,
                         GL_STATIC_DRAW);
            glBufferSubData(GL_COPY_READ_BUFFER, kStagingOffset, size, data);
            glCopyBufferSubData(GL_COPY_READ_BUFFER, target, kStagingOffset, offset, size);
        }
    }

    void testTearDown() override { glDeleteBuffers(1, &mBuffer); }
    GLuint mBuffer;
};

// Test that updating a small index buffer after drawing with it works.
// In the Vulkan backend, the CPU may be used to perform this copy.
TEST_P(BufferSubDataTest, SmallIndexBufferUpdateAfterDraw)
{
    constexpr std::array<GLfloat, 4> kRed   = {1.0f, 0.0f, 0.0f, 1.0f};
    constexpr std::array<GLfloat, 4> kGreen = {0.0f, 1.0f, 0.0f, 1.0f};
    // Index buffer data
    GLuint indexData[] = {0, 1, 2, 0};
    // Vertex buffer data fully cover the screen
    float vertexData[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

    GLBuffer indexBuffer;
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    GLint vPos = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(vPos, -1);
    glUseProgram(program);
    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Bind vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
    glVertexAttribPointer(vPos, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(vPos);

    // Bind index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_DYNAMIC_DRAW);

    glUniform4fv(colorUniformLocation, 1, kRed.data());
    // Draw left red triangle
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
    // Update the index buffer data.
    indexData[1] = 1;
    indexData[2] = 2;
    indexData[3] = 3;
    // Partial copy to trigger the buffer pool allocation
    updateBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint), 3 * sizeof(GLuint), &indexData[1]);
    // Draw triangle with index (1, 2, 3).
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (const void *)sizeof(GLuint));
    // Update the index buffer again
    indexData[0] = 0;
    indexData[1] = 0;
    indexData[2] = 2;
    glUniform4fv(colorUniformLocation, 1, kGreen.data());
    updateBuffer(GL_ELEMENT_ARRAY_BUFFER, 0, 3 * sizeof(GLuint), &indexData[0]);
    // Draw triangle with index (0, 2, 3), hope angle copy the last index 3 back.
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (const void *)sizeof(GLuint));

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, 0, GLColor::red);
    // Verify pixel top left corner is green
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() - 1, GLColor::green);
}

// Test that updating a small index buffer after drawing with it works.
// In the Vulkan backend, the CPU may be used to perform this copy.
TEST_P(BufferSubDataTest, SmallVertexDataUpdateAfterDraw)
{
    constexpr std::array<GLfloat, 4> kGreen = {0.0f, 1.0f, 0.0f, 1.0f};
    // Index buffer data
    GLuint indexData[] = {0, 1, 2, 0};
    // Vertex buffer data lower left triangle
    // 2
    //
    // o    1
    float vertexData1[] = {
        -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
    };
    // Vertex buffer data upper right triangle
    // 2      1
    //
    //        0
    float vertexData2[] = {
        1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
    };
    GLBuffer indexBuffer;
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    GLint vPos = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(vPos, -1);
    glUseProgram(program);
    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Bind vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData1), vertexData1, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vPos, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(vPos);

    // Bind index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_DYNAMIC_DRAW);

    glUniform4fv(colorUniformLocation, 1, kGreen.data());
    // Draw left red triangle
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
    // Update the vertex buffer data.
    // Partial copy to trigger the buffer pool allocation
    updateBuffer(GL_ARRAY_BUFFER, 0, sizeof(vertexData2), vertexData2);
    // Draw triangle with index (0,1,2).
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (const void *)sizeof(GLuint));
    // Verify pixel corners are green
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green);
}
class IndexedBufferCopyTest : public ANGLETest<>
{
  protected:
    IndexedBufferCopyTest()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        constexpr char kVS[] = R"(attribute vec3 in_attrib;
varying vec3 v_attrib;
void main()
{
    v_attrib = in_attrib;
    gl_Position = vec4(0.0, 0.0, 0.5, 1.0);
    gl_PointSize = 100.0;
})";

        constexpr char kFS[] = R"(precision mediump float;
varying vec3 v_attrib;
void main()
{
    gl_FragColor = vec4(v_attrib, 1);
})";

        glGenBuffers(2, mBuffers);
        ASSERT_NE(mBuffers[0], 0U);
        ASSERT_NE(mBuffers[1], 0U);

        glGenBuffers(1, &mElementBuffer);
        ASSERT_NE(mElementBuffer, 0U);

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(mProgram, 0U);

        mAttribLocation = glGetAttribLocation(mProgram, "in_attrib");
        ASSERT_NE(mAttribLocation, -1);

        glClearColor(0, 0, 0, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteBuffers(2, mBuffers);
        glDeleteBuffers(1, &mElementBuffer);
        glDeleteProgram(mProgram);
    }

    GLuint mBuffers[2];
    GLuint mElementBuffer;
    GLuint mProgram;
    GLint mAttribLocation;
};

// The following test covers an ANGLE bug where our index ranges
// weren't updated from CopyBufferSubData calls
// https://code.google.com/p/angleproject/issues/detail?id=709
TEST_P(IndexedBufferCopyTest, IndexRangeBug)
{
    // TODO(geofflang): Figure out why this fails on AMD OpenGL (http://anglebug.com/42260302)
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

    unsigned char vertexData[] = {255, 0, 0, 0, 0, 0};
    unsigned int indexData[]   = {0, 1};

    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(char) * 6, vertexData, GL_STATIC_DRAW);

    glUseProgram(mProgram);
    glVertexAttribPointer(mAttribLocation, 3, GL_UNSIGNED_BYTE, GL_TRUE, 3, nullptr);
    glEnableVertexAttribArray(mAttribLocation);

    ASSERT_GL_NO_ERROR();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 1, indexData, GL_STATIC_DRAW);

    glUseProgram(mProgram);

    ASSERT_GL_NO_ERROR();

    glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, nullptr);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    glBindBuffer(GL_COPY_READ_BUFFER, mBuffers[1]);
    glBufferData(GL_COPY_READ_BUFFER, 4, &indexData[1], GL_STATIC_DRAW);

    glBindBuffer(GL_COPY_WRITE_BUFFER, mElementBuffer);

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(int));

    ASSERT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 0);

    unsigned char newData[] = {0, 255, 0};
    glBufferSubData(GL_ARRAY_BUFFER, 3, 3, newData);

    glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, nullptr);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

class BufferDataTestES3 : public BufferDataTest
{};

// The following test covers an ANGLE bug where the buffer storage
// is not resized by Buffer11::getLatestBufferStorage when needed.
// https://code.google.com/p/angleproject/issues/detail?id=897
TEST_P(BufferDataTestES3, BufferResizing)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    ASSERT_GL_NO_ERROR();

    // Allocate a buffer with one byte
    uint8_t singleByte[] = {0xaa};
    glBufferData(GL_ARRAY_BUFFER, 1, singleByte, GL_STATIC_DRAW);

    // Resize the buffer
    // To trigger the bug, the buffer need to be big enough because some hardware copy buffers
    // by chunks of pages instead of the minimum number of bytes needed.
    const size_t numBytes = 4096 * 4;
    glBufferData(GL_ARRAY_BUFFER, numBytes, nullptr, GL_STATIC_DRAW);

    // Copy the original data to the buffer
    uint8_t srcBytes[numBytes];
    for (size_t i = 0; i < numBytes; ++i)
    {
        srcBytes[i] = static_cast<uint8_t>(i);
    }

    void *dest = glMapBufferRange(GL_ARRAY_BUFFER, 0, numBytes,
                                  GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    ASSERT_GL_NO_ERROR();

    memcpy(dest, srcBytes, numBytes);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    EXPECT_GL_NO_ERROR();

    // Create a new buffer and copy the data to it
    GLuint readBuffer;
    glGenBuffers(1, &readBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, readBuffer);
    uint8_t zeros[numBytes];
    for (size_t i = 0; i < numBytes; ++i)
    {
        zeros[i] = 0;
    }
    glBufferData(GL_COPY_WRITE_BUFFER, numBytes, zeros, GL_STATIC_DRAW);
    glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, numBytes);

    ASSERT_GL_NO_ERROR();

    // Read back the data and compare it to the original
    uint8_t *data = reinterpret_cast<uint8_t *>(
        glMapBufferRange(GL_COPY_WRITE_BUFFER, 0, numBytes, GL_MAP_READ_BIT));

    ASSERT_GL_NO_ERROR();

    for (size_t i = 0; i < numBytes; ++i)
    {
        EXPECT_EQ(srcBytes[i], data[i]);
    }
    glUnmapBuffer(GL_COPY_WRITE_BUFFER);

    glDeleteBuffers(1, &readBuffer);

    EXPECT_GL_NO_ERROR();
}

// Test to verify mapping a buffer after copying to it contains flushed/updated data
TEST_P(BufferDataTestES3, CopyBufferSubDataMapReadTest)
{
    const char simpleVertex[]   = R"(attribute vec2 position;
attribute vec4 color;
varying vec4 vColor;
void main()
{
    gl_Position = vec4(position, 0, 1);
    vColor = color;
}
)";
    const char simpleFragment[] = R"(precision mediump float;
varying vec4 vColor;
void main()
{
    gl_FragColor = vColor;
}
)";

    const uint32_t numComponents = 3;
    const uint32_t width         = 4;
    const uint32_t height        = 4;
    const size_t numElements     = width * height * numComponents;
    std::vector<uint8_t> srcData(numElements);
    std::vector<uint8_t> dstData(numElements);

    for (uint8_t i = 0; i < srcData.size(); i++)
    {
        srcData[i] = 128;
    }
    for (uint8_t i = 0; i < dstData.size(); i++)
    {
        dstData[i] = 0;
    }

    GLBuffer srcBuffer;
    GLBuffer dstBuffer;

    glBindBuffer(GL_ARRAY_BUFFER, srcBuffer);
    glBufferData(GL_ARRAY_BUFFER, srcData.size(), srcData.data(), GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, dstBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, dstData.size(), dstData.data(), GL_STATIC_READ);
    ASSERT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(program, simpleVertex, simpleFragment);
    glUseProgram(program);

    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);

    glBindBuffer(GL_ARRAY_BUFFER, srcBuffer);
    glVertexAttribPointer(colorLoc, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glEnableVertexAttribArray(colorLoc);

    drawQuad(program, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glCopyBufferSubData(GL_ARRAY_BUFFER, GL_PIXEL_UNPACK_BUFFER, 0, 0, numElements);

    // With GL_MAP_READ_BIT, we expect the data to be flushed and updated to match srcData
    uint8_t *data = reinterpret_cast<uint8_t *>(
        glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, numElements, GL_MAP_READ_BIT));
    EXPECT_GL_NO_ERROR();
    for (size_t i = 0; i < numElements; ++i)
    {
        EXPECT_EQ(srcData[i], data[i]);
    }
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Test to verify mapping a buffer after copying to it contains expected data
// with GL_MAP_UNSYNCHRONIZED_BIT
TEST_P(BufferDataTestES3, MapBufferUnsynchronizedReadTest)
{
    const char simpleVertex[]   = R"(attribute vec2 position;
attribute vec4 color;
varying vec4 vColor;
void main()
{
    gl_Position = vec4(position, 0, 1);
    vColor = color;
}
)";
    const char simpleFragment[] = R"(precision mediump float;
varying vec4 vColor;
void main()
{
    gl_FragColor = vColor;
}
)";

    const uint32_t numComponents = 3;
    const uint32_t width         = 4;
    const uint32_t height        = 4;
    const size_t numElements     = width * height * numComponents;
    std::vector<uint8_t> srcData(numElements);
    std::vector<uint8_t> dstData(numElements);

    for (uint8_t i = 0; i < srcData.size(); i++)
    {
        srcData[i] = 128;
    }
    for (uint8_t i = 0; i < dstData.size(); i++)
    {
        dstData[i] = 0;
    }

    GLBuffer srcBuffer;
    GLBuffer dstBuffer;

    glBindBuffer(GL_ARRAY_BUFFER, srcBuffer);
    glBufferData(GL_ARRAY_BUFFER, srcData.size(), srcData.data(), GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, dstBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, dstData.size(), dstData.data(), GL_STATIC_READ);
    ASSERT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(program, simpleVertex, simpleFragment);
    glUseProgram(program);

    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);

    glBindBuffer(GL_ARRAY_BUFFER, srcBuffer);
    glVertexAttribPointer(colorLoc, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glEnableVertexAttribArray(colorLoc);

    drawQuad(program, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    glCopyBufferSubData(GL_ARRAY_BUFFER, GL_PIXEL_UNPACK_BUFFER, 0, 0, numElements);

    // Synchronize.
    glFinish();

    // Map with GL_MAP_UNSYNCHRONIZED_BIT and overwrite buffers data with srcData
    uint8_t *data = reinterpret_cast<uint8_t *>(glMapBufferRange(
        GL_PIXEL_UNPACK_BUFFER, 0, numElements, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
    EXPECT_GL_NO_ERROR();
    memcpy(data, srcData.data(), srcData.size());
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    EXPECT_GL_NO_ERROR();

    // Map without GL_MAP_UNSYNCHRONIZED_BIT and read data. We expect it to be srcData
    data = reinterpret_cast<uint8_t *>(
        glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, numElements, GL_MAP_READ_BIT));
    EXPECT_GL_NO_ERROR();
    for (size_t i = 0; i < numElements; ++i)
    {
        EXPECT_EQ(srcData[i], data[i]);
    }
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Verify the functionality of glMapBufferRange()'s GL_MAP_UNSYNCHRONIZED_BIT
// NOTE: On Vulkan, if we ever use memory that's not `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT`, then
// this could incorrectly pass.
TEST_P(BufferDataTestES3, MapBufferRangeUnsynchronizedBit)
{
    // We can currently only control the behavior of the Vulkan backend's synchronizing operation's
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    const size_t numElements = 10;
    std::vector<uint8_t> srcData(numElements);
    std::vector<uint8_t> dstData(numElements);

    for (uint8_t i = 0; i < srcData.size(); i++)
    {
        srcData[i] = i;
    }
    for (uint8_t i = 0; i < dstData.size(); i++)
    {
        dstData[i] = static_cast<uint8_t>(i + dstData.size());
    }

    GLBuffer srcBuffer;
    GLBuffer dstBuffer;

    glBindBuffer(GL_COPY_READ_BUFFER, srcBuffer);
    ASSERT_GL_NO_ERROR();
    glBindBuffer(GL_COPY_WRITE_BUFFER, dstBuffer);
    ASSERT_GL_NO_ERROR();

    glBufferData(GL_COPY_READ_BUFFER, srcData.size(), srcData.data(), GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();
    glBufferData(GL_COPY_WRITE_BUFFER, dstData.size(), dstData.data(), GL_STATIC_READ);
    ASSERT_GL_NO_ERROR();

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, numElements);

    // With GL_MAP_UNSYNCHRONIZED_BIT, we expect the data to be stale and match dstData
    // NOTE: We are specifying GL_MAP_WRITE_BIT so we can use GL_MAP_UNSYNCHRONIZED_BIT. This is
    // venturing into undefined behavior, since we are actually planning on reading from this
    // pointer.
    auto *data = reinterpret_cast<uint8_t *>(glMapBufferRange(
        GL_COPY_WRITE_BUFFER, 0, numElements, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
    EXPECT_GL_NO_ERROR();
    for (size_t i = 0; i < numElements; ++i)
    {
        // Allow for the possibility that data matches either "dstData" or "srcData"
        if (dstData[i] != data[i])
        {
            EXPECT_EQ(srcData[i], data[i]);
        }
    }
    glUnmapBuffer(GL_COPY_WRITE_BUFFER);
    EXPECT_GL_NO_ERROR();

    // Without GL_MAP_UNSYNCHRONIZED_BIT, we expect the data to be copied and match srcData
    data = reinterpret_cast<uint8_t *>(
        glMapBufferRange(GL_COPY_WRITE_BUFFER, 0, numElements, GL_MAP_READ_BIT));
    EXPECT_GL_NO_ERROR();
    for (size_t i = 0; i < numElements; ++i)
    {
        EXPECT_EQ(srcData[i], data[i]);
    }
    glUnmapBuffer(GL_COPY_WRITE_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Verify OES_mapbuffer is present if EXT_map_buffer_range is.
TEST_P(BufferDataTest, ExtensionDependency)
{
    if (IsGLExtensionEnabled("GL_EXT_map_buffer_range"))
    {
        ASSERT_TRUE(IsGLExtensionEnabled("GL_OES_mapbuffer"));
    }
}

// Test mapping with the OES extension.
TEST_P(BufferDataTest, MapBufferOES)
{
    if (!IsGLExtensionEnabled("GL_EXT_map_buffer_range"))
    {
        // Needed for test validation.
        return;
    }

    std::vector<uint8_t> data(1024);
    FillVectorWithRandomUBytes(&data);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, data.size(), nullptr, GL_STATIC_DRAW);

    // Validate that other map flags don't work.
    void *badMapPtr = glMapBufferOES(GL_ARRAY_BUFFER, GL_MAP_READ_BIT);
    EXPECT_EQ(nullptr, badMapPtr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Map and write.
    void *mapPtr = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    ASSERT_NE(nullptr, mapPtr);
    ASSERT_GL_NO_ERROR();
    memcpy(mapPtr, data.data(), data.size());
    glUnmapBufferOES(GL_ARRAY_BUFFER);

    // Validate data with EXT_map_buffer_range
    void *readMapPtr = glMapBufferRangeEXT(GL_ARRAY_BUFFER, 0, data.size(), GL_MAP_READ_BIT_EXT);
    ASSERT_NE(nullptr, readMapPtr);
    ASSERT_GL_NO_ERROR();
    std::vector<uint8_t> actualData(data.size());
    memcpy(actualData.data(), readMapPtr, data.size());
    glUnmapBufferOES(GL_ARRAY_BUFFER);

    EXPECT_EQ(data, actualData);
}

// Test to verify mapping a dynamic buffer with GL_MAP_UNSYNCHRONIZED_BIT to modify a portion
// won't affect draw calls using other portions.
TEST_P(BufferDataTest, MapDynamicBufferUnsynchronizedEXTTest)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_map_buffer_range"));

    const char simpleVertex[]   = R"(attribute vec2 position;
attribute vec4 color;
varying vec4 vColor;
void main()
{
    gl_Position = vec4(position, 0, 1);
    vColor = color;
}
)";
    const char simpleFragment[] = R"(precision mediump float;
varying vec4 vColor;
void main()
{
    gl_FragColor = vColor;
}
)";

    constexpr int kNumVertices = 6;

    std::vector<GLubyte> color(8 * kNumVertices);
    for (int i = 0; i < kNumVertices; ++i)
    {
        color[4 * i]     = 255;
        color[4 * i + 3] = 255;
    }
    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, color.size(), color.data(), GL_DYNAMIC_DRAW);

    ANGLE_GL_PROGRAM(program, simpleVertex, simpleFragment);
    glUseProgram(program);

    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);

    glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glEnableVertexAttribArray(colorLoc);

    glViewport(0, 0, 2, 2);
    drawQuad(program, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Map with GL_MAP_UNSYNCHRONIZED_BIT and overwrite buffers data at offset 24
    uint8_t *data = reinterpret_cast<uint8_t *>(
        glMapBufferRangeEXT(GL_ARRAY_BUFFER, 4 * kNumVertices, 4 * kNumVertices,
                            GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
    EXPECT_GL_NO_ERROR();
    for (int i = 0; i < kNumVertices; ++i)
    {
        data[4 * i]     = 0;
        data[4 * i + 1] = 255;
        data[4 * i + 2] = 0;
        data[4 * i + 3] = 255;
    }
    glUnmapBufferOES(GL_ARRAY_BUFFER);
    EXPECT_GL_NO_ERROR();

    // Re-draw using offset = 0 but to different viewport
    glViewport(0, 2, 2, 2);
    drawQuad(program, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Change vertex attribute to use buffer starting from offset 24
    glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0,
                          reinterpret_cast<void *>(4 * kNumVertices));

    glViewport(2, 2, 2, 2);
    drawQuad(program, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(1, 3, GLColor::red);

    // The result below is undefined. The glBufferData at the top puts
    // [red, red, red, ..., zero, zero, zero, ...]
    // in the buffer and the glMap,glUnmap tries to overwrite the zeros with green
    // but because UNSYNCHRONIZED was passed in there's no guarantee those
    // zeros have been written yet. If they haven't they'll overwrite the
    // greens.
    // EXPECT_PIXEL_COLOR_EQ(3, 3, GLColor::green);
}

// Verify that we can map and write the buffer between draws and the second draw sees the new buffer
// data, using drawQuad().
TEST_P(BufferDataTest, MapWriteArrayBufferDataDrawQuad)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_map_buffer_range"));

    std::vector<GLfloat> data(6, 0.0f);

    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(mAttribLocation);

    // Don't read back to verify black, so we don't break the render pass.
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();

    // Map and write.
    std::vector<GLfloat> data2(6, 1.0f);
    void *mapPtr = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    ASSERT_NE(nullptr, mapPtr);
    ASSERT_GL_NO_ERROR();
    memcpy(mapPtr, data2.data(), sizeof(GLfloat) * data2.size());
    glUnmapBufferOES(GL_ARRAY_BUFFER);

    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(8, 8, GLColor::red);
    EXPECT_GL_NO_ERROR();
}

// Verify that we can map and write the buffer between draws and the second draw sees the new buffer
// data, calling glDrawArrays() directly.
TEST_P(BufferDataTest, MapWriteArrayBufferDataDrawArrays)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_map_buffer_range"));

    std::vector<GLfloat> data(6, 0.0f);

    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);

    // Set up position attribute, don't use drawQuad.
    auto quadVertices = GetQuadVertices();

    GLBuffer positionBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * quadVertices.size() * 3, quadVertices.data(),
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(mAttribLocation);
    EXPECT_GL_NO_ERROR();

    // Don't read back to verify black, so we don't break the render pass.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();

    // Map and write.
    std::vector<GLfloat> data2(6, 1.0f);
    void *mapPtr = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    ASSERT_NE(nullptr, mapPtr);
    ASSERT_GL_NO_ERROR();
    memcpy(mapPtr, data2.data(), sizeof(GLfloat) * data2.size());
    glUnmapBufferOES(GL_ARRAY_BUFFER);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(8, 8, GLColor::red);
    EXPECT_GL_NO_ERROR();
}

// Verify that buffer sub data uploads are properly validated within the buffer size range on 32-bit
// systems.
TEST_P(BufferDataTest, BufferSizeValidation32Bit)
{
    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 100, nullptr, GL_STATIC_DRAW);

    GLubyte data = 0;
    glBufferSubData(GL_ARRAY_BUFFER, std::numeric_limits<uint32_t>::max(), 1, &data);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Some drivers generate errors when array buffer bindings are left mapped during draw calls.
// crbug.com/1345777
TEST_P(BufferDataTestES3, GLDriverErrorWhenMappingArrayBuffersDuringDraw)
{
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ASSERT_NE(program, 0u);

    glUseProgram(program);

    auto quadVertices = GetQuadVertices();

    GLBuffer vb;
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * quadVertices.size(), quadVertices.data(),
                 GL_STATIC_DRAW);

    GLint positionLocation = glGetAttribLocation(program, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();

    GLBuffer pb;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pb);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, 1024, nullptr, GL_STREAM_DRAW);
    glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, 1024, GL_MAP_WRITE_BIT);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();
}

// Tests a null crash bug caused by copying from null back-end buffer pointer
// when calling bufferData again after drawing without calling bufferData in D3D11.
TEST_P(BufferDataTestES3, DrawWithNotCallingBufferData)
{
    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glUseProgram(drawRed);

    GLint mem = 0;
    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindBuffer(GL_COPY_WRITE_BUFFER, buffer);
    glBufferData(GL_COPY_WRITE_BUFFER, 1, &mem, GL_STREAM_DRAW);
    ASSERT_GL_NO_ERROR();
}

// Tests a bug where copying buffer data immediately after creation hit a nullptr in D3D11.
TEST_P(BufferDataTestES3, NoBufferInitDataCopyBug)
{
    constexpr GLsizei size = 64;

    GLBuffer sourceBuffer;
    glBindBuffer(GL_COPY_READ_BUFFER, sourceBuffer);
    glBufferData(GL_COPY_READ_BUFFER, size, nullptr, GL_STATIC_DRAW);

    GLBuffer destBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, destBuffer);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, size);
    ASSERT_GL_NO_ERROR();
}

// This a shortened version of dEQP functional.buffer.copy.basic.array_copy_read. It provoked
// a bug in copyBufferSubData. The bug appeared to be that conversion buffers were not marked
// as dirty and therefore after copyBufferSubData the next draw call using the buffer that
// just had data copied to it was not re-converted. It's not clear to me how this ever worked
// or why changes to bufferSubData from
// https://chromium-review.googlesource.com/c/angle/angle/+/3842641 made this issue appear and
// why it wasn't already broken.
TEST_P(BufferDataTestES3, CopyBufferSubDataDraw)
{
    const char simpleVertex[]   = R"(attribute vec2 position;
attribute vec4 color;
varying vec4 vColor;
void main()
{
    gl_Position = vec4(position, 0, 1);
    vColor = color;
}
)";
    const char simpleFragment[] = R"(precision mediump float;
varying vec4 vColor;
void main()
{
    gl_FragColor = vColor;
}
)";

    ANGLE_GL_PROGRAM(program, simpleVertex, simpleFragment);
    glUseProgram(program);

    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);
    GLint posLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, posLoc);

    glClearColor(0, 0, 0, 0);

    GLBuffer srcBuffer;  // green
    GLBuffer dstBuffer;  // red

    constexpr size_t numElements = 399;
    std::vector<GLColorRGB> reds(numElements, GLColorRGB::red);
    std::vector<GLColorRGB> greens(numElements, GLColorRGB::green);
    constexpr size_t sizeOfElem  = sizeof(decltype(greens)::value_type);
    constexpr size_t sizeInBytes = numElements * sizeOfElem;

    glBindBuffer(GL_ARRAY_BUFFER, srcBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeInBytes, greens.data(), GL_STREAM_DRAW);

    glBindBuffer(GL_COPY_READ_BUFFER, dstBuffer);
    glBufferData(GL_COPY_READ_BUFFER, sizeInBytes, reds.data(), GL_STREAM_DRAW);
    ASSERT_GL_NO_ERROR();

    constexpr size_t numQuads = numElements / 4;

    // Generate quads that fill clip space to use all the vertex colors
    std::vector<float> positions(numQuads * 4 * 2);
    for (size_t quad = 0; quad < numQuads; ++quad)
    {
        size_t offset = quad * 4 * 2;
        float x0      = float(quad + 0) / numQuads * 2.0f - 1.0f;
        float x1      = float(quad + 1) / numQuads * 2.0f - 1.0f;

        /*
           2--3
           |  |
           0--1
        */
        positions[offset + 0] = x0;
        positions[offset + 1] = -1;
        positions[offset + 2] = x1;
        positions[offset + 3] = -1;
        positions[offset + 4] = x0;
        positions[offset + 5] = 1;
        positions[offset + 6] = x1;
        positions[offset + 7] = 1;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, positions.data());
    ASSERT_GL_NO_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, srcBuffer);
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    ASSERT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT);

    std::vector<GLushort> indices(numQuads * 6);
    for (size_t quad = 0; quad < numQuads; ++quad)
    {
        size_t ndx          = quad * 4;
        size_t offset       = quad * 6;
        indices[offset + 0] = ndx;
        indices[offset + 1] = ndx + 1;
        indices[offset + 2] = ndx + 2;
        indices[offset + 3] = ndx + 2;
        indices[offset + 4] = ndx + 1;
        indices[offset + 5] = ndx + 3;
    }
    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(decltype(indices)::value_type),
                 indices.data(), GL_STATIC_DRAW);

    // Draw with srcBuffer (green)
    glDrawElements(GL_TRIANGLES, numQuads * 6, GL_UNSIGNED_SHORT, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::green);
    ASSERT_GL_NO_ERROR();

    // Draw with dstBuffer (red)
    glBindBuffer(GL_ARRAY_BUFFER, dstBuffer);
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glDrawElements(GL_TRIANGLES, numQuads * 6, GL_UNSIGNED_SHORT, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);
    ASSERT_GL_NO_ERROR();

    // Copy src to dst. Yes, we're using GL_COPY_READ_BUFFER as dest because that's what the dEQP
    // test was testing.
    glBindBuffer(GL_ARRAY_BUFFER, srcBuffer);
    glBindBuffer(GL_COPY_READ_BUFFER, dstBuffer);
    glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_READ_BUFFER, 0, 0, sizeInBytes);
    ASSERT_GL_NO_ERROR();

    // Draw with srcBuffer. It should still be green.
    glBindBuffer(GL_ARRAY_BUFFER, srcBuffer);
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glDrawElements(GL_TRIANGLES, numQuads * 6, GL_UNSIGNED_SHORT, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::green);
    ASSERT_GL_NO_ERROR();

    // Draw with dstBuffer. It should now be green too.
    glBindBuffer(GL_ARRAY_BUFFER, dstBuffer);
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 3, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glDrawElements(GL_TRIANGLES, numQuads * 6, GL_UNSIGNED_SHORT, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::green);

    ASSERT_GL_NO_ERROR();
}

// Ensures that calling glBufferData on a mapped buffer results in an unmapped buffer
TEST_P(BufferDataTestES3, BufferDataUnmap)
{
    // Per the OpenGL ES 3.0 spec, buffers are implicity unmapped when a call to
    // BufferData happens on a mapped buffer:
    //
    //    If any portion of the buffer object is mapped in the current context or
    //    any context current to another thread, it is as though UnmapBuffer
    //    (see section 2.10.3) is executed in each such context prior to deleting
    //    the existing data store.
    //

    std::vector<uint8_t> data1(16);
    std::vector<uint8_t> data2(16);

    GLBuffer dataBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, dataBuffer);
    glBufferData(GL_ARRAY_BUFFER, data1.size(), data1.data(), GL_STATIC_DRAW);

    // Map the buffer once
    glMapBufferRange(GL_ARRAY_BUFFER, 0, data1.size(),
                     GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                         GL_MAP_UNSYNCHRONIZED_BIT);

    // Then repopulate the buffer. This should cause the buffer to become unmapped.
    glBufferData(GL_ARRAY_BUFFER, data2.size(), data2.data(), GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    // Try to unmap the buffer, this should fail
    bool result = glUnmapBuffer(GL_ARRAY_BUFFER);
    ASSERT_EQ(result, false);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Try to map the buffer again, which should succeed
    glMapBufferRange(GL_ARRAY_BUFFER, 0, data2.size(),
                     GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT |
                         GL_MAP_UNSYNCHRONIZED_BIT);
    ASSERT_GL_NO_ERROR();
}

// Ensures that mapping buffer with GL_MAP_INVALIDATE_BUFFER_BIT followed by glBufferSubData calls
// works.  Regression test for the Vulkan backend where that flag caused use after free.
TEST_P(BufferSubDataTest, MapInvalidateThenBufferSubData)
{
    // http://anglebug.com/42264515
    ANGLE_SKIP_TEST_IF(IsWindows() && IsOpenGL() && IsIntel());

    // http://anglebug.com/42264516
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsOpenGLES());

    const std::array<GLColor, 4> kInitialData = {GLColor::red, GLColor::red, GLColor::red,
                                                 GLColor::red};
    const std::array<GLColor, 4> kUpdateData1 = {GLColor::white, GLColor::white, GLColor::white,
                                                 GLColor::white};
    const std::array<GLColor, 4> kUpdateData2 = {GLColor::blue, GLColor::blue, GLColor::blue,
                                                 GLColor::blue};

    GLBuffer buffer;
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(kInitialData), kInitialData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);
    EXPECT_GL_NO_ERROR();

    // Draw
    constexpr char kVerifyUBO[] = R"(#version 300 es
precision mediump float;
uniform block {
    uvec4 data;
} ubo;
uniform uint expect;
uniform vec4 successOutput;
out vec4 colorOut;
void main()
{
    if (all(equal(ubo.data, uvec4(expect))))
        colorOut = successOutput;
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    ANGLE_GL_PROGRAM(verifyUbo, essl3_shaders::vs::Simple(), kVerifyUBO);
    glUseProgram(verifyUbo);

    GLint expectLoc = glGetUniformLocation(verifyUbo, "expect");
    EXPECT_NE(-1, expectLoc);
    GLint successLoc = glGetUniformLocation(verifyUbo, "successOutput");
    EXPECT_NE(-1, successLoc);

    glUniform1ui(expectLoc, kInitialData[0].asUint());
    glUniform4f(successLoc, 0, 1, 0, 1);

    drawQuad(verifyUbo, essl3_shaders::PositionAttrib(), 0.5);
    EXPECT_GL_NO_ERROR();

    // Dont't verify the buffer.  This is testing GL_MAP_INVALIDATE_BUFFER_BIT while the buffer is
    // in use by the GPU.

    // Map the buffer and update it.
    void *mappedBuffer = glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(kInitialData),
                                          GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    memcpy(mappedBuffer, kUpdateData1.data(), sizeof(kInitialData));

    glUnmapBuffer(GL_UNIFORM_BUFFER);
    EXPECT_GL_NO_ERROR();

    // Verify that the buffer has the updated value.
    glUniform1ui(expectLoc, kUpdateData1[0].asUint());
    glUniform4f(successLoc, 0, 0, 1, 1);

    drawQuad(verifyUbo, essl3_shaders::PositionAttrib(), 0.5);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Update the buffer with glBufferSubData or glCopyBufferSubData
    updateBuffer(GL_UNIFORM_BUFFER, 0, sizeof(kUpdateData2), kUpdateData2.data());
    EXPECT_GL_NO_ERROR();

    // Verify that the buffer has the updated value.
    glUniform1ui(expectLoc, kUpdateData2[0].asUint());
    glUniform4f(successLoc, 0, 1, 1, 1);

    drawQuad(verifyUbo, essl3_shaders::PositionAttrib(), 0.5);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::cyan);
}

// Verify that previous draws are not affected when a buffer is respecified with null data
// and updated by calling map.
TEST_P(BufferDataTestES3, BufferDataWithNullFollowedByMap)
{
    // Draw without using drawQuad.
    glUseProgram(mProgram);

    // Set up position attribute
    const auto &quadVertices = GetQuadVertices();
    GLint positionLocation   = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    GLBuffer positionBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * quadVertices.size() * 3, quadVertices.data(),
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);
    EXPECT_GL_NO_ERROR();

    // Set up "in_attrib" attribute
    const std::vector<GLfloat> kData(6, 1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * kData.size(), kData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(mAttribLocation);
    EXPECT_GL_NO_ERROR();

    // This draw (draw_0) renders red to the entire window.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();

    // Respecify buffer bound to "in_attrib" attribute then map it and fill it with zeroes.
    const std::vector<GLfloat> kZeros(6, 0.0f);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * kZeros.size(), nullptr, GL_STATIC_DRAW);
    uint8_t *mapPtr = reinterpret_cast<uint8_t *>(
        glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * kZeros.size(),
                         GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT));
    ASSERT_NE(nullptr, mapPtr);
    ASSERT_GL_NO_ERROR();
    memcpy(mapPtr, kZeros.data(), sizeof(GLfloat) * kZeros.size());
    glUnmapBuffer(GL_ARRAY_BUFFER);
    ASSERT_GL_NO_ERROR();

    // This draw (draw_1) renders black to the upper right triangle.
    glDrawArrays(GL_TRIANGLES, 3, 3);
    EXPECT_GL_NO_ERROR();

    // Respecification and data update of mBuffer should not have affected draw_0.
    // Expect bottom left to be red and top right to be black.
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::black);
    EXPECT_GL_NO_ERROR();
}

// Test glFenceSync call breaks renderPass followed by glCopyBufferSubData that read access the same
// buffer that renderPass reads. There was a bug that this triggers assertion angleproject.com/7903.
TEST_P(BufferDataTestES3, bufferReadFromRenderPassAndOutsideRenderPassWithFenceSyncInBetween)
{
    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    std::vector<GLfloat> data(6, 1.0f);
    GLsizei bufferSize = sizeof(GLfloat) * data.size();
    glBufferData(GL_ARRAY_BUFFER, bufferSize, data.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(mAttribLocation);
    glScissor(0, 0, getWindowWidth() / 2, getWindowHeight());
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();

    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    EXPECT_GL_NO_ERROR();

    GLBuffer dstBuffer;
    glBindBuffer(GL_COPY_WRITE_BUFFER, dstBuffer);
    glBufferData(GL_COPY_WRITE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);
    glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, bufferSize);

    glBindBuffer(GL_ARRAY_BUFFER, dstBuffer);
    glScissor(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight());
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::red);
}

class BufferStorageTestES3 : public BufferDataTest
{};

// Tests that proper error value is returned when bad size is passed in
TEST_P(BufferStorageTestES3, BufferStorageInvalidSize)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    std::vector<GLfloat> data(6, 1.0f);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferStorageEXT(GL_ARRAY_BUFFER, 0, data.data(), 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Tests that buffer storage can be allocated with the GL_MAP_PERSISTENT_BIT_EXT and
// GL_MAP_COHERENT_BIT_EXT flags
TEST_P(BufferStorageTestES3, BufferStorageFlagsPersistentCoherentWrite)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    std::vector<GLfloat> data(6, 1.0f);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferStorageEXT(GL_ARRAY_BUFFER, data.size(), data.data(),
                       GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
    ASSERT_GL_NO_ERROR();
}

// Verify that glBufferStorage makes a buffer immutable
TEST_P(BufferStorageTestES3, StorageBufferBufferData)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    std::vector<GLfloat> data(6, 1.0f);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferStorageEXT(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), 0);
    ASSERT_GL_NO_ERROR();

    // Verify that calling glBufferStorageEXT again produces an error.
    glBufferStorageEXT(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Verify that calling glBufferData after calling glBufferStorageEXT produces an error.
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), GL_STATIC_DRAW);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Verify that glBufferStorageEXT can be called after glBufferData
TEST_P(BufferStorageTestES3, BufferDataStorageBuffer)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    std::vector<GLfloat> data(6, 1.0f);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    // Verify that calling glBufferStorageEXT produces no error
    glBufferStorageEXT(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), 0);
    ASSERT_GL_NO_ERROR();
}

// Verify that consecutive BufferStorage calls don't clobber data
// This is a regression test for an AllocateNonZeroMemory bug, where the offset
// of the suballocation wasn't being used properly
TEST_P(BufferStorageTestES3, BufferStorageClobber)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    constexpr size_t largeSizes[] = {101, 103, 107, 109, 113, 127, 131, 137, 139};
    constexpr size_t smallSizes[] = {7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
    constexpr size_t readBackSize = 16;

    for (size_t largeSize : largeSizes)
    {
        std::vector<GLubyte> data0(largeSize * 1024, 0x1E);

        // Check for a test author error, we can't read back more than the size of data0.
        ASSERT(readBackSize <= data0.size());

        // Do a large write first, ensure this is a device-local buffer only (no storage flags)
        GLBuffer buffer0;
        glBindBuffer(GL_ARRAY_BUFFER, buffer0);
        glBufferStorageEXT(GL_ARRAY_BUFFER, sizeof(GLubyte) * data0.size(), data0.data(), 0);
        ASSERT_GL_NO_ERROR();

        // Do a bunch of smaller writes next, creating/deleting buffers as
        // we go (we just want to try to fuzz it so we might write to the
        // same suballocation as the above)
        for (size_t smallSize : smallSizes)
        {
            std::vector<GLubyte> data1(smallSize, 0x4A);
            GLBuffer buffer1;
            glBindBuffer(GL_ARRAY_BUFFER, buffer1);
            glBufferStorageEXT(GL_ARRAY_BUFFER, sizeof(GLubyte) * data1.size(), data1.data(), 0);
            ASSERT_GL_NO_ERROR();

            // Force the buffer write (and other buffer creation setup work) to
            // flush
            glFinish();
        }

        // Create a staging area to read back the buffer
        GLBuffer mappable;
        glBindBuffer(GL_ARRAY_BUFFER, mappable);
        glBufferStorageEXT(GL_ARRAY_BUFFER, sizeof(GLubyte) * readBackSize, nullptr,
                           GL_MAP_READ_BIT);
        ASSERT_GL_NO_ERROR();

        glBindBuffer(GL_COPY_READ_BUFFER, buffer0);
        glBindBuffer(GL_COPY_WRITE_BUFFER, mappable);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
                            sizeof(GLubyte) * readBackSize);
        ASSERT_GL_NO_ERROR();
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

        GLubyte *mapped = reinterpret_cast<GLubyte *>(
            glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(GLubyte) * readBackSize, GL_MAP_READ_BIT));
        ASSERT_NE(mapped, nullptr);
        ASSERT_GL_NO_ERROR();
        for (size_t i = 0; i < readBackSize; i++)
        {
            EXPECT_EQ(mapped[i], data0[i])
                << "Expected " << static_cast<int>(data0[i]) << " at index " << i << ", got "
                << static_cast<int>(mapped[i]);
        }
    }
}

// Verify that we can perform subdata updates to a buffer marked with GL_DYNAMIC_STORAGE_BIT_EXT
// usage flag
TEST_P(BufferStorageTestES3, StorageBufferSubData)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    std::vector<GLfloat> data(6, 0.0f);

    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferStorageEXT(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), nullptr,
                       GL_DYNAMIC_STORAGE_BIT_EXT);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * data.size(), data.data());
    glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(mAttribLocation);

    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(8, 8, GLColor::black);
    EXPECT_GL_NO_ERROR();

    std::vector<GLfloat> data2(6, 1.0f);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * data2.size(), data2.data());

    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(8, 8, GLColor::red);
    EXPECT_GL_NO_ERROR();
}

// Test interaction between GL_OES_mapbuffer and GL_EXT_buffer_storage extensions.
TEST_P(BufferStorageTestES3, StorageBufferMapBufferOES)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage") ||
                       !IsGLExtensionEnabled("GL_EXT_map_buffer_range"));

    std::vector<uint8_t> data(1024);
    FillVectorWithRandomUBytes(&data);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferStorageEXT(GL_ARRAY_BUFFER, data.size(), nullptr, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

    // Validate that other map flags don't work.
    void *badMapPtr = glMapBufferOES(GL_ARRAY_BUFFER, GL_MAP_READ_BIT);
    EXPECT_EQ(nullptr, badMapPtr);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Map and write.
    void *mapPtr = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    ASSERT_NE(nullptr, mapPtr);
    ASSERT_GL_NO_ERROR();
    memcpy(mapPtr, data.data(), data.size());
    glUnmapBufferOES(GL_ARRAY_BUFFER);

    // Validate data with EXT_map_buffer_range
    void *readMapPtr = glMapBufferRangeEXT(GL_ARRAY_BUFFER, 0, data.size(), GL_MAP_READ_BIT_EXT);
    ASSERT_NE(nullptr, readMapPtr);
    ASSERT_GL_NO_ERROR();
    std::vector<uint8_t> actualData(data.size());
    memcpy(actualData.data(), readMapPtr, data.size());
    glUnmapBufferOES(GL_ARRAY_BUFFER);

    EXPECT_EQ(data, actualData);
}

// Verify persistently mapped buffers can use glCopyBufferSubData
// Tests a pattern used by Fortnite's GLES backend
TEST_P(BufferStorageTestES3, StorageCopyBufferSubDataMapped)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    const std::array<GLColor, 4> kInitialData = {GLColor::red, GLColor::green, GLColor::blue,
                                                 GLColor::yellow};

    // Set up the read buffer
    GLBuffer readBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, readBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kInitialData), kInitialData.data(), GL_DYNAMIC_DRAW);

    // Set up the write buffer to be persistently mapped
    GLBuffer writeBuffer;
    glBindBuffer(GL_COPY_WRITE_BUFFER, writeBuffer);
    glBufferStorageEXT(GL_COPY_WRITE_BUFFER, 16, nullptr,
                       GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
    void *readMapPtr =
        glMapBufferRange(GL_COPY_WRITE_BUFFER, 0, 16,
                         GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
    ASSERT_NE(nullptr, readMapPtr);
    ASSERT_GL_NO_ERROR();

    // Verify we can copy into the write buffer
    glBindBuffer(GL_COPY_READ_BUFFER, readBuffer);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, 16);
    ASSERT_GL_NO_ERROR();

    // Flush the buffer.
    glFinish();

    // Check the contents
    std::array<GLColor, 4> resultingData;
    memcpy(resultingData.data(), readMapPtr, resultingData.size() * sizeof(GLColor));
    glUnmapBuffer(GL_COPY_WRITE_BUFFER);
    EXPECT_EQ(kInitialData, resultingData);
    ASSERT_GL_NO_ERROR();
}

// Verify persistently mapped element array buffers can use glDrawElements
TEST_P(BufferStorageTestES3, DrawElementsElementArrayBufferMapped)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    GLfloat kVertexBuffer[] = {-1.0f, -1.0f, 1.0f,  // (x, y, R)
                               -1.0f, 1.0f,  1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    // Set up array buffer
    GLBuffer readBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, readBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kVertexBuffer), kVertexBuffer, GL_DYNAMIC_DRAW);
    GLint vLoc = glGetAttribLocation(mProgram, "position");
    GLint cLoc = mAttribLocation;
    glVertexAttribPointer(vLoc, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(vLoc);
    glVertexAttribPointer(cLoc, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const GLvoid *)8);
    glEnableVertexAttribArray(cLoc);

    // Set up the element array buffer to be persistently mapped
    GLshort kElementArrayBuffer[] = {0, 0, 0, 0, 0, 0};

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferStorageEXT(GL_ELEMENT_ARRAY_BUFFER, sizeof(kElementArrayBuffer), kElementArrayBuffer,
                       GL_DYNAMIC_STORAGE_BIT_EXT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT |
                           GL_MAP_COHERENT_BIT_EXT);

    glUseProgram(mProgram);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    GLshort *mappedPtr = (GLshort *)glMapBufferRange(
        GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(kElementArrayBuffer),
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
    ASSERT_NE(nullptr, mappedPtr);
    ASSERT_GL_NO_ERROR();

    mappedPtr[0] = 0;
    mappedPtr[1] = 1;
    mappedPtr[2] = 2;
    mappedPtr[3] = 2;
    mappedPtr[4] = 1;
    mappedPtr[5] = 3;
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that maps a coherent buffer storage and does not call glUnmapBuffer.
TEST_P(BufferStorageTestES3, NoUnmap)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    GLsizei size = sizeof(GLfloat) * 128;

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferStorageEXT(GL_ARRAY_BUFFER, size, nullptr,
                       GL_DYNAMIC_STORAGE_BIT_EXT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT |
                           GL_MAP_COHERENT_BIT_EXT);

    GLshort *mappedPtr = (GLshort *)glMapBufferRange(
        GL_ARRAY_BUFFER, 0, size,
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
    ASSERT_NE(nullptr, mappedPtr);

    ASSERT_GL_NO_ERROR();
}

// Test that we are able to perform glTex*D calls while a pixel unpack buffer is bound
// and persistently mapped.
TEST_P(BufferStorageTestES3, TexImage2DPixelUnpackBufferMappedPersistently)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    std::vector<uint8_t> data(64);
    FillVectorWithRandomUBytes(&data);

    GLBuffer buffer;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glBufferStorageEXT(GL_PIXEL_UNPACK_BUFFER, data.size(), data.data(),
                       GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT);

    // Map the buffer.
    void *mapPtr = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, data.size(),
                                    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT);
    ASSERT_NE(nullptr, mapPtr);
    ASSERT_GL_NO_ERROR();

    // Create a 2D texture and fill it using the persistenly mapped unpack buffer
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);

    constexpr GLsizei kTextureWidth  = 4;
    constexpr GLsizei kTextureHeight = 4;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kTextureWidth, kTextureHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, 0);
    ASSERT_GL_NO_ERROR();

    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    ASSERT_GL_NO_ERROR();
}

// Verify persistently mapped buffers can use glBufferSubData
TEST_P(BufferStorageTestES3, StorageBufferSubDataMapped)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    const std::array<GLColor, 4> kUpdateData1 = {GLColor::red, GLColor::green, GLColor::blue,
                                                 GLColor::yellow};

    // Set up the buffer to be persistently mapped and dynamic
    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferStorageEXT(GL_ARRAY_BUFFER, 16, nullptr,
                       GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT |
                           GL_MAP_COHERENT_BIT_EXT | GL_DYNAMIC_STORAGE_BIT_EXT);
    void *readMapPtr = glMapBufferRange(
        GL_ARRAY_BUFFER, 0, 16,
        GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
    ASSERT_NE(nullptr, readMapPtr);
    ASSERT_GL_NO_ERROR();

    // Verify we can push new data into the buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLColor) * kUpdateData1.size(), kUpdateData1.data());
    ASSERT_GL_NO_ERROR();

    // Flush the buffer.
    glFinish();

    // Check the contents
    std::array<GLColor, 4> persistentData1;
    memcpy(persistentData1.data(), readMapPtr, persistentData1.size() * sizeof(GLColor));
    EXPECT_EQ(kUpdateData1, persistentData1);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    ASSERT_GL_NO_ERROR();
}

// Verify that persistently mapped coherent buffers can be used as uniform buffers,
// and written to by using the pointer from glMapBufferRange.
TEST_P(BufferStorageTestES3, UniformBufferMapped)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    const char *mkFS = R"(#version 300 es
precision highp float;
uniform uni { vec4 color; };
out vec4 fragColor;
void main()
{
    fragColor = color;
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), mkFS);
    ASSERT_NE(program, 0u);

    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "uni");
    ASSERT_NE(uniformBufferIndex, -1);

    GLBuffer uniformBuffer;

    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);

    glBufferStorageEXT(GL_UNIFORM_BUFFER, sizeof(float) * 4, nullptr,
                       GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

    float *mapPtr = static_cast<float *>(
        glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(float) * 4,
                         GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_PERSISTENT_BIT_EXT |
                             GL_MAP_COHERENT_BIT_EXT));

    ASSERT_NE(mapPtr, nullptr);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);

    glUniformBlockBinding(program, uniformBufferIndex, 0);

    mapPtr[0] = 0.5f;
    mapPtr[1] = 0.75f;
    mapPtr[2] = 0.25f;
    mapPtr[3] = 1.0f;

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_NEAR(0, 0, 128, 191, 64, 255, 1);

    glUnmapBuffer(GL_UNIFORM_BUFFER);

    glDeleteProgram(program);

    ASSERT_GL_NO_ERROR();
}

// Verify that persistently mapped coherent buffers can be used as vertex array buffers,
// and written to by using the pointer from glMapBufferRange.
TEST_P(BufferStorageTestES3, VertexBufferMapped)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    ASSERT_NE(program, 0u);

    glUseProgram(program);

    auto quadVertices = GetQuadVertices();

    size_t bufferSize = sizeof(GLfloat) * quadVertices.size() * 3;

    GLBuffer positionBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);

    glBufferStorageEXT(GL_ARRAY_BUFFER, bufferSize, nullptr,
                       GL_DYNAMIC_STORAGE_BIT_EXT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT |
                           GL_MAP_COHERENT_BIT_EXT);

    GLint positionLocation = glGetAttribLocation(program, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    void *mappedPtr =
        glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize,
                         GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
    ASSERT_NE(nullptr, mappedPtr);

    memcpy(mappedPtr, reinterpret_cast<void *>(quadVertices.data()), bufferSize);

    glDrawArrays(GL_TRIANGLES, 0, quadVertices.size());
    EXPECT_PIXEL_COLOR_EQ(8, 8, GLColor::red);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glDeleteProgram(program);

    EXPECT_GL_NO_ERROR();
}

void TestPageSharingBuffers(std::function<void(void)> swapCallback,
                            size_t bufferSize,
                            const std::array<Vector3, 6> &quadVertices,
                            GLint positionLocation)
{
    size_t dataSize = sizeof(GLfloat) * quadVertices.size() * 3;

    if (bufferSize == 0)
    {
        bufferSize = dataSize;
    }

    constexpr size_t bufferCount = 10;

    std::vector<GLBuffer> buffers(bufferCount);
    std::vector<void *> mapPointers(bufferCount);

    // Init and map
    for (uint32_t i = 0; i < bufferCount; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);

        glBufferStorageEXT(GL_ARRAY_BUFFER, bufferSize, nullptr,
                           GL_DYNAMIC_STORAGE_BIT_EXT | GL_MAP_WRITE_BIT |
                               GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

        glEnableVertexAttribArray(positionLocation);

        mapPointers[i] = glMapBufferRange(
            GL_ARRAY_BUFFER, 0, bufferSize,
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
        ASSERT_NE(nullptr, mapPointers[i]);
    }

    // Write, draw and unmap
    for (uint32_t i = 0; i < bufferCount; i++)
    {
        memcpy(mapPointers[i], reinterpret_cast<const void *>(quadVertices.data()), dataSize);

        // Write something to last float
        if (bufferSize > dataSize + sizeof(GLfloat))
        {
            size_t lastPosition = bufferSize / sizeof(GLfloat) - 1;
            reinterpret_cast<float *>(mapPointers[i])[lastPosition] = 1.0f;
        }

        glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glDrawArrays(GL_TRIANGLES, 0, quadVertices.size());
        EXPECT_PIXEL_COLOR_EQ(8, 8, GLColor::red);
        swapCallback();
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}

// Create multiple persistently mapped coherent buffers of different sizes that will likely share a
// page. Map all buffers together and unmap each buffer after writing to it and using it for a draw.
// This tests the behaviour of the coherent buffer tracker in frame capture when buffers that share
// a page are written to after the other one is removed.
TEST_P(BufferStorageTestES3, PageSharingBuffers)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    ASSERT_NE(program, 0u);

    glUseProgram(program);

    auto quadVertices = GetQuadVertices();

    std::function<void(void)> swapCallback = [this]() { swapBuffers(); };

    GLint positionLocation = glGetAttribLocation(program, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);

    TestPageSharingBuffers(swapCallback, 0, quadVertices, positionLocation);
    TestPageSharingBuffers(swapCallback, 1000, quadVertices, positionLocation);
    TestPageSharingBuffers(swapCallback, 4096, quadVertices, positionLocation);
    TestPageSharingBuffers(swapCallback, 6144, quadVertices, positionLocation);
    TestPageSharingBuffers(swapCallback, 40960, quadVertices, positionLocation);

    glDeleteProgram(program);

    EXPECT_GL_NO_ERROR();
}

class BufferStorageTestES3Threaded : public ANGLETest<>
{
  protected:
    BufferStorageTestES3Threaded()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);

        mProgram = 0;
    }

    void testSetUp() override
    {
        constexpr char kVS[] = R"(#version 300 es

in vec4 position;
in vec4 color;
out vec4 out_color;

void main()
{
    out_color = color;
    gl_Position = position;
})";

        constexpr char kFS[] = R"(#version 300 es
precision highp float;

in vec4 out_color;
out vec4 fragColor;

void main()
{
    fragColor = vec4(out_color);
})";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(mProgram, 0U);

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    void updateColors(int i, int offset, const GLColor &color)
    {
        Vector4 colorVec               = color.toNormalizedVector();
        mMappedPtr[offset + i * 4 + 0] = colorVec.x();
        mMappedPtr[offset + i * 4 + 1] = colorVec.y();
        mMappedPtr[offset + i * 4 + 2] = colorVec.z();
        mMappedPtr[offset + i * 4 + 3] = colorVec.w();
    }

    void updateThreadedAndDraw(int offset, const GLColor &color)
    {
        std::mutex mutex;
        std::vector<std::thread> threads(4);
        for (size_t i = 0; i < 4; i++)
        {
            threads[i] = std::thread([&, i]() {
                std::lock_guard<decltype(mutex)> lock(mutex);
                updateColors(i, offset, color);
            });
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

        for (std::thread &thread : threads)
        {
            thread.join();
        }
    }

    GLuint mProgram;
    GLfloat *mMappedPtr = nullptr;
};

// Test using a large buffer storage for a color vertex array buffer, which is
// off set every iteration step via glVertexAttribPointer.
// Write to the buffer storage map pointer from multiple threads for the next iteration,
// while drawing the current one.
TEST_P(BufferStorageTestES3Threaded, VertexBuffer)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3 ||
                       !IsGLExtensionEnabled("GL_EXT_buffer_storage"));

    auto vertices = GetIndexedQuadVertices();

    // Set up position buffer
    GLBuffer positionBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    GLint positionLoc = glGetAttribLocation(mProgram, "position");
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(positionLoc);

    // Set up color buffer
    GLBuffer colorBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);

    // Let's create a big buffer which fills 10 pages at pagesize 4096
    GLint bufferSize   = sizeof(GLfloat) * 1024 * 10;
    GLint offsetFloats = 0;

    glBufferStorageEXT(GL_ARRAY_BUFFER, bufferSize, nullptr,
                       GL_DYNAMIC_STORAGE_BIT_EXT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT |
                           GL_MAP_COHERENT_BIT_EXT);
    GLint colorLoc = glGetAttribLocation(mProgram, "color");
    glEnableVertexAttribArray(colorLoc);

    auto indices = GetQuadIndices();

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(),
                 GL_STATIC_DRAW);

    glUseProgram(mProgram);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    ASSERT_GL_NO_ERROR();

    mMappedPtr = (GLfloat *)glMapBufferRange(
        GL_ARRAY_BUFFER, 0, bufferSize,
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);
    ASSERT_NE(nullptr, mMappedPtr);
    ASSERT_GL_NO_ERROR();

    // Initial color
    for (int i = 0; i < 4; i++)
    {
        updateColors(i, offsetFloats, GLColor::black);
    }

    std::vector<GLColor> colors = {GLColor::red, GLColor::green, GLColor::blue};

    // 4 vertices, 4 floats
    GLint contentSize = 4 * 4;

    // Update and draw last
    int i = 0;
    while (bufferSize > (int)((offsetFloats + contentSize) * sizeof(GLfloat)))
    {
        glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                              reinterpret_cast<const GLvoid *>(offsetFloats * sizeof(GLfloat)));

        offsetFloats += contentSize;
        GLColor color = colors[i % colors.size()];
        updateThreadedAndDraw(offsetFloats, color);

        if (i > 0)
        {
            GLColor lastColor = colors[(i - 1) % colors.size()];
            EXPECT_PIXEL_COLOR_EQ(0, 0, lastColor);
        }
        ASSERT_GL_NO_ERROR();
        i++;
    }

    // Last draw
    glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                          reinterpret_cast<const GLvoid *>(offsetFloats * sizeof(GLfloat)));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glUnmapBuffer(GL_ARRAY_BUFFER);

    ASSERT_GL_NO_ERROR();
}

// Test that buffer self-copy works when buffer is used as UBO
TEST_P(BufferDataTestES3, CopyBufferSubDataSelfDependency)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

uniform UBO
{
    vec4 data[128];
};

void main()
{
    color = data[12];
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    constexpr uint32_t kVec4Size   = 4 * sizeof(float);
    constexpr uint32_t kUBOSize    = 128 * kVec4Size;
    constexpr uint32_t kDataOffset = 12 * kVec4Size;

    // Init data is 4 times the size of UBO as the buffer is created larger than the UBO throughout
    // the test.
    const std::vector<float> kInitData(kUBOSize, 123.45);

    // Set up a throw-away buffer just to make buffer suballocations not use offset 0.
    GLBuffer throwaway;
    glBindBuffer(GL_UNIFORM_BUFFER, throwaway);
    glBufferData(GL_UNIFORM_BUFFER, 1024, nullptr, GL_DYNAMIC_DRAW);

    // Set up the buffer
    GLBuffer buffer;
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, kUBOSize * 2, kInitData.data(), GL_DYNAMIC_DRAW);

    const std::vector<float> kColorData = {
        0.75,
        0.5,
        0.25,
        1.0,
    };
    glBufferSubData(GL_UNIFORM_BUFFER, kDataOffset, kVec4Size, kColorData.data());

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    const int w = getWindowWidth();
    const int h = getWindowHeight();

    // Use the buffer, then do a big self-copy
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, buffer, 0, kUBOSize);
    glScissor(0, 0, w / 2, h / 2);
    glEnable(GL_SCISSOR_TEST);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5);

    // Duplicate the buffer in the second half
    glCopyBufferSubData(GL_UNIFORM_BUFFER, GL_UNIFORM_BUFFER, 0, kUBOSize, kUBOSize);

    // Draw again, making sure the copy succeeded.
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, buffer, kUBOSize, kUBOSize);
    glScissor(w / 2, 0, w / 2, h / 2);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5);

    // Do a small self-copy
    constexpr uint32_t kCopySrcOffset = 4 * kVec4Size;
    constexpr uint32_t kCopyDstOffset = (64 + 4) * kVec4Size;
    glCopyBufferSubData(GL_UNIFORM_BUFFER, GL_UNIFORM_BUFFER, kCopySrcOffset, kCopyDstOffset,
                        kDataOffset);

    // color data was previously at [12], and is now available at [68 + 12 - 4]
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, buffer, kCopyDstOffset - kCopySrcOffset, kUBOSize);
    glScissor(0, h / 2, w / 2, h / 2);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5);

    // Validate results
    EXPECT_PIXEL_NEAR(0, 0, 191, 127, 63, 255, 1);
    EXPECT_PIXEL_NEAR(w / 2 + 1, 0, 191, 127, 63, 255, 1);
    EXPECT_PIXEL_NEAR(0, h / 2 + 1, 191, 127, 63, 255, 1);
    EXPECT_PIXEL_COLOR_EQ(w / 2 + 1, h / 2 + 1, GLColor::black);

    // Do a big copy again, but this time the buffer is unused by the GPU
    glCopyBufferSubData(GL_UNIFORM_BUFFER, GL_UNIFORM_BUFFER, kUBOSize, 0, kUBOSize);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, buffer, 0, kUBOSize);
    glScissor(w / 2, h / 2, w / 2, h / 2);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5);
    EXPECT_PIXEL_NEAR(w / 2 + 1, h / 2 + 1, 191, 127, 63, 255, 1);

    // Do a small copy again, but this time the buffer is unused by the GPU
    glCopyBufferSubData(GL_UNIFORM_BUFFER, GL_UNIFORM_BUFFER, kUBOSize + kCopySrcOffset,
                        kCopyDstOffset, kDataOffset);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, buffer, kCopyDstOffset - kCopySrcOffset, kUBOSize);
    glDisable(GL_SCISSOR_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5);
    EXPECT_PIXEL_NEAR(0, 0, 191, 127, 63, 255, 1);

    ASSERT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES2(BufferDataTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BufferSubDataTest);
ANGLE_INSTANTIATE_TEST_COMBINE_1(BufferSubDataTest,
                                 BufferSubDataTestPrint,
                                 testing::Bool(),
                                 ANGLE_ALL_TEST_PLATFORMS_ES3,
                                 ES3_VULKAN().enable(Feature::PreferCPUForBufferSubData));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BufferDataTestES3);
ANGLE_INSTANTIATE_TEST_ES3_AND(BufferDataTestES3,
                               ES3_VULKAN().enable(Feature::PreferCPUForBufferSubData),
                               ES3_METAL().enable(Feature::ForceBufferGPUStorage));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BufferStorageTestES3);
ANGLE_INSTANTIATE_TEST_ES3_AND(BufferStorageTestES3,
                               ES3_VULKAN().enable(Feature::AllocateNonZeroMemory));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(IndexedBufferCopyTest);
ANGLE_INSTANTIATE_TEST_ES3(IndexedBufferCopyTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BufferStorageTestES3Threaded);
ANGLE_INSTANTIATE_TEST_ES3(BufferStorageTestES3Threaded);

#ifdef _WIN64

// Test a bug where an integer overflow bug could trigger a crash in D3D.
// The test uses 8 buffers with a size just under 0x2000000 to overflow max uint
// (with the internal D3D rounding to 16-byte values) and trigger the bug.
// Only handle this bug on 64-bit Windows for now. Harder to repro on 32-bit.
class BufferDataOverflowTest : public ANGLETest<>
{
  protected:
    BufferDataOverflowTest() {}
};

// See description above.
TEST_P(BufferDataOverflowTest, VertexBufferIntegerOverflow)
{
    // These values are special, to trigger the rounding bug.
    unsigned int numItems       = 0x7FFFFFE;
    constexpr GLsizei bufferCnt = 8;

    std::vector<GLBuffer> buffers(bufferCnt);

    std::stringstream vertexShaderStr;

    for (GLsizei bufferIndex = 0; bufferIndex < bufferCnt; ++bufferIndex)
    {
        vertexShaderStr << "attribute float attrib" << bufferIndex << ";\n";
    }

    vertexShaderStr << "attribute vec2 position;\n"
                       "varying float v_attrib;\n"
                       "void main() {\n"
                       "  gl_Position = vec4(position, 0, 1);\n"
                       "  v_attrib = 0.0;\n";

    for (GLsizei bufferIndex = 0; bufferIndex < bufferCnt; ++bufferIndex)
    {
        vertexShaderStr << "v_attrib += attrib" << bufferIndex << ";\n";
    }

    vertexShaderStr << "}";

    constexpr char kFS[] =
        "varying highp float v_attrib;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(v_attrib, 0, 0, 1);\n"
        "}";

    ANGLE_GL_PROGRAM(program, vertexShaderStr.str().c_str(), kFS);
    glUseProgram(program);

    std::vector<GLfloat> data(numItems, 1.0f);

    for (GLsizei bufferIndex = 0; bufferIndex < bufferCnt; ++bufferIndex)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffers[bufferIndex]);
        glBufferData(GL_ARRAY_BUFFER, numItems * sizeof(float), &data[0], GL_DYNAMIC_DRAW);

        std::stringstream attribNameStr;
        attribNameStr << "attrib" << bufferIndex;

        GLint attribLocation = glGetAttribLocation(program, attribNameStr.str().c_str());
        ASSERT_NE(-1, attribLocation);

        glVertexAttribPointer(attribLocation, 1, GL_FLOAT, GL_FALSE, 4, nullptr);
        glEnableVertexAttribArray(attribLocation);
    }

    GLint positionLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLocation);
    glDisableVertexAttribArray(positionLocation);
    glVertexAttrib2f(positionLocation, 1.0f, 1.0f);

    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, numItems);
    EXPECT_GL_ERROR(GL_OUT_OF_MEMORY);

    // Test that a small draw still works.
    for (GLsizei bufferIndex = 0; bufferIndex < bufferCnt; ++bufferIndex)
    {
        std::stringstream attribNameStr;
        attribNameStr << "attrib" << bufferIndex;
        GLint attribLocation = glGetAttribLocation(program, attribNameStr.str().c_str());
        ASSERT_NE(-1, attribLocation);
        glDisableVertexAttribArray(attribLocation);
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_GL_ERROR(GL_NO_ERROR);
}

// Tests a security bug in our CopyBufferSubData validation (integer overflow).
TEST_P(BufferDataOverflowTest, CopySubDataValidation)
{
    GLBuffer readBuffer, writeBuffer;

    glBindBuffer(GL_COPY_READ_BUFFER, readBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, writeBuffer);

    constexpr int bufSize = 100;

    glBufferData(GL_COPY_READ_BUFFER, bufSize, nullptr, GL_STATIC_DRAW);
    glBufferData(GL_COPY_WRITE_BUFFER, bufSize, nullptr, GL_STATIC_DRAW);

    GLintptr big = std::numeric_limits<GLintptr>::max() - bufSize + 90;

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, big, 0, 50);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, big, 50);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

ANGLE_INSTANTIATE_TEST_ES3(BufferDataOverflowTest);

#endif  // _WIN64
