//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderStorageBufferTest:
//   Various tests related for shader storage buffers.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

struct MatrixCase
{
    MatrixCase(unsigned cols,
               unsigned rows,
               unsigned matrixStride,
               const char *computeShaderSource,
               const float *inputData)
        : mColumns(cols),
          mRows(rows),
          mMatrixStride(matrixStride),
          mComputeShaderSource(computeShaderSource),
          mInputdata(inputData)
    {}
    unsigned int mColumns;
    unsigned int mRows;
    unsigned int mMatrixStride;
    const char *mComputeShaderSource;
    const float *mInputdata;
    const unsigned int kBytesPerComponent = sizeof(float);
};

struct VectorCase
{
    VectorCase(unsigned components,
               const char *computeShaderSource,
               const GLuint *inputData,
               const GLuint *expectedData)
        : mComponents(components),
          mComputeShaderSource(computeShaderSource),
          mInputdata(inputData),
          mExpectedData(expectedData)
    {}
    unsigned int mComponents;
    const char *mComputeShaderSource;
    const GLuint *mInputdata;
    const GLuint *mExpectedData;
    const unsigned int kBytesPerComponent = sizeof(GLuint);
};

class ShaderStorageBufferTest31 : public ANGLETest<>
{
  protected:
    ShaderStorageBufferTest31()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);

        // Test flakiness was noticed when reusing displays.
        forceNewDisplay();
    }

    void runMatrixTest(const MatrixCase &matrixCase)
    {
        ANGLE_GL_COMPUTE_PROGRAM(program, matrixCase.mComputeShaderSource);
        glUseProgram(program);

        // Create shader storage buffer
        GLBuffer shaderStorageBuffer[2];
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, matrixCase.mRows * matrixCase.mMatrixStride,
                     matrixCase.mInputdata, GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, matrixCase.mRows * matrixCase.mMatrixStride, nullptr,
                     GL_STATIC_DRAW);

        // Bind shader storage buffer
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

        glDispatchCompute(1, 1, 1);
        glFinish();

        // Read back shader storage buffer
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
        const GLfloat *ptr = reinterpret_cast<const GLfloat *>(
            glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                             matrixCase.mRows * matrixCase.mMatrixStride, GL_MAP_READ_BIT));

        for (unsigned int row = 0; row < matrixCase.mRows; row++)
        {
            for (unsigned int col = 0; col < matrixCase.mColumns; col++)
            {
                GLfloat expected = matrixCase.mInputdata[row * (matrixCase.mMatrixStride /
                                                                matrixCase.kBytesPerComponent) +
                                                         col];
                GLfloat actual =
                    *(ptr + row * (matrixCase.mMatrixStride / matrixCase.kBytesPerComponent) + col);

                EXPECT_EQ(expected, actual) << " at row " << row << " and column " << col;
            }
        }

        EXPECT_GL_NO_ERROR();
    }

    void runVectorTest(const VectorCase &vectorCase)
    {
        ANGLE_GL_COMPUTE_PROGRAM(program, vectorCase.mComputeShaderSource);
        glUseProgram(program);

        // Create shader storage buffer
        GLBuffer shaderStorageBuffer[2];
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
                     vectorCase.mComponents * vectorCase.kBytesPerComponent, vectorCase.mInputdata,
                     GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
                     vectorCase.mComponents * vectorCase.kBytesPerComponent, nullptr,
                     GL_STATIC_DRAW);

        // Bind shader storage buffer
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

        glDispatchCompute(1, 1, 1);
        glFinish();

        // Read back shader storage buffer
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
        const GLuint *ptr = reinterpret_cast<const GLuint *>(glMapBufferRange(
            GL_SHADER_STORAGE_BUFFER, 0, vectorCase.mComponents * vectorCase.kBytesPerComponent,
            GL_MAP_READ_BIT));
        for (unsigned int idx = 0; idx < vectorCase.mComponents; idx++)
        {
            EXPECT_EQ(vectorCase.mExpectedData[idx], *(ptr + idx));
        }

        EXPECT_GL_NO_ERROR();
    }
};

// Matched block names within a shader interface must match in terms of having the same number of
// declarations with the same sequence of types.
TEST_P(ShaderStorageBufferTest31, MatchedBlockNameWithDifferentMemberType)
{
    constexpr char kVS[] =
        "#version 310 es\n"
        "buffer blockName {\n"
        "    float data;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";
    constexpr char kFS[] =
        "#version 310 es\n"
        "buffer blockName {\n"
        "    uint data;\n"
        "};\n"
        "void main()\n"
        "{\n"
        "}\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Linking should fail if blocks in vertex shader exceed GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS.
TEST_P(ShaderStorageBufferTest31, ExceedMaxVertexShaderStorageBlocks)
{
    std::ostringstream instanceCount;
    GLint maxVertexShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &maxVertexShaderStorageBlocks);
    EXPECT_GL_NO_ERROR();
    instanceCount << maxVertexShaderStorageBlocks;

    const std::string &vertexShaderSource =
        "#version 310 es\n"
        "layout(shared) buffer blockName {\n"
        "    uint data;\n"
        "} instance[" +
        instanceCount.str() +
        " + 1];\n"
        "void main()\n"
        "{\n"
        "}\n";
    constexpr char kFS[] =
        "#version 310 es\n"
        "void main()\n"
        "{\n"
        "}\n";

    GLuint program = CompileProgram(vertexShaderSource.c_str(), kFS);
    EXPECT_EQ(0u, program);
}

// Linking should fail if the sum of the number of active shader storage blocks exceeds
// MAX_COMBINED_SHADER_STORAGE_BLOCKS.
TEST_P(ShaderStorageBufferTest31, ExceedMaxCombinedShaderStorageBlocks)
{
    std::ostringstream vertexInstanceCount;
    GLint maxVertexShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &maxVertexShaderStorageBlocks);
    vertexInstanceCount << maxVertexShaderStorageBlocks;

    GLint maxFragmentShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);

    GLint maxCombinedShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, &maxCombinedShaderStorageBlocks);
    EXPECT_GL_NO_ERROR();

    ASSERT_GE(maxCombinedShaderStorageBlocks, maxVertexShaderStorageBlocks);
    ASSERT_GE(maxCombinedShaderStorageBlocks, maxFragmentShaderStorageBlocks);

    // As SPEC allows MAX_VERTEX_SHADER_STORAGE_BLOCKS and MAX_FRAGMENT_SHADER_STORAGE_BLOCKS to be
    // 0, in this situation we should skip this test to prevent these unexpected compile errors.
    ANGLE_SKIP_TEST_IF(maxVertexShaderStorageBlocks == 0 || maxFragmentShaderStorageBlocks == 0);

    GLint fragmentShaderStorageBlocks =
        maxCombinedShaderStorageBlocks - maxVertexShaderStorageBlocks + 1;
    ANGLE_SKIP_TEST_IF(fragmentShaderStorageBlocks > maxFragmentShaderStorageBlocks);

    std::ostringstream fragmentInstanceCount;
    fragmentInstanceCount << fragmentShaderStorageBlocks;

    const std::string &vertexShaderSource =
        "#version 310 es\n"
        "layout(shared) buffer blockName0 {\n"
        "    uint data;\n"
        "} instance0[" +
        vertexInstanceCount.str() +
        "];\n"
        "void main()\n"
        "{\n"
        "}\n";
    const std::string &fragmentShaderSource =
        "#version 310 es\n"
        "layout(shared) buffer blockName1 {\n"
        "    uint data;\n"
        "} instance1[" +
        fragmentInstanceCount.str() +
        "];\n"
        "void main()\n"
        "{\n"
        "}\n";

    GLuint program = CompileProgram(vertexShaderSource.c_str(), fragmentShaderSource.c_str());
    EXPECT_EQ(0u, program);
}

// Linking should not fail if block size in shader equals to GL_MAX_SHADER_STORAGE_BLOCK_SIZE.
// Linking should fail if block size in shader exceeds GL_MAX_SHADER_STORAGE_BLOCK_SIZE.
TEST_P(ShaderStorageBufferTest31, ExceedMaxShaderStorageBlockSize)
{
    GLint maxShaderStorageBlockSize = 0;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxShaderStorageBlockSize);
    EXPECT_GL_NO_ERROR();

    // Linking should not fail if block size in shader equals to GL_MAX_SHADER_STORAGE_BLOCK_SIZE.
    std::ostringstream blockArraySize;
    blockArraySize << (maxShaderStorageBlockSize / 4);
    const std::string &computeShaderSource =
        "#version 310 es\n"
        "layout (local_size_x = 1) in;\n"
        "layout(std430) buffer FullSizeBlock\n"
        "{\n"
        "uint data[" +
        blockArraySize.str() +
        "];\n"
        "};\n"
        "void main()\n"
        "{\n"
        "for (int i=0; i<" +
        blockArraySize.str() +
        "; i++)\n"
        "{\n"
        "data[i] = uint(0);\n"
        "};\n"
        "}\n";

    GLuint ComputeProgram = CompileComputeProgram(computeShaderSource.c_str(), true);
    EXPECT_NE(0u, ComputeProgram);
    glDeleteProgram(ComputeProgram);

    // Linking should fail if block size in shader exceeds GL_MAX_SHADER_STORAGE_BLOCK_SIZE.
    std::ostringstream exceedBlockArraySize;
    exceedBlockArraySize << (maxShaderStorageBlockSize / 4 + 1);
    const std::string &exceedComputeShaderSource =
        "#version 310 es\n"
        "layout (local_size_x = 1) in;\n"
        "layout(std430) buffer FullSizeBlock\n"
        "{\n"
        "uint data[" +
        exceedBlockArraySize.str() +
        "];\n"
        "};\n"
        "void main()\n"
        "{\n"
        "for (int i=0; i<" +
        exceedBlockArraySize.str() +
        "; i++)\n"
        "{\n"
        "data[i] = uint(0);\n"
        "};\n"
        "}\n";

    GLuint exceedComputeProgram = CompileComputeProgram(exceedComputeShaderSource.c_str(), true);
    EXPECT_EQ(0u, exceedComputeProgram);
    glDeleteProgram(exceedComputeProgram);
}

// Test shader storage buffer read write.
TEST_P(ShaderStorageBufferTest31, ShaderStorageBufferReadWrite)
{
    constexpr char kCS[] =
        "#version 310 es\n"
        "layout(local_size_x=1, local_size_y=1, local_size_z=1) in;\n"
        "layout(std140, binding = 1) buffer blockName {\n"
        "    uint data[2];\n"
        "} instanceName;\n"
        "void main()\n"
        "{\n"
        "    instanceName.data[0] = 3u;\n"
        "    instanceName.data[1] = 4u;\n"
        "}\n";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    glUseProgram(program);

    constexpr unsigned int kElementCount = 2;
    // The array stride are rounded up to the base alignment of a vec4 for std140 layout.
    constexpr unsigned int kArrayStride = 16;
    // Create shader storage buffer
    GLBuffer shaderStorageBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kElementCount * kArrayStride, nullptr, GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer);

    // Dispath compute
    glDispatchCompute(1, 1, 1);

    glFinish();

    // Read back shader storage buffer
    constexpr unsigned int kExpectedValues[2] = {3u, 4u};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    void *ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kElementCount * kArrayStride,
                                 GL_MAP_READ_BIT);
    for (unsigned int idx = 0; idx < kElementCount; idx++)
    {
        EXPECT_EQ(kExpectedValues[idx],
                  *(reinterpret_cast<const GLuint *>(reinterpret_cast<const GLbyte *>(ptr) +
                                                     idx * kArrayStride)));
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    EXPECT_GL_NO_ERROR();
}

// Test shader storage buffer write followed by glTexSUbData and followed by shader storage write
// again.
TEST_P(ShaderStorageBufferTest31, ShaderStorageBufferReadWriteAndBufferSubData)
{
    constexpr char kCS[] =
        "#version 310 es\n"
        "layout(local_size_x=1, local_size_y=1, local_size_z=1) in;\n"
        "layout(std140, binding = 1) buffer blockName {\n"
        "    uint data[2];\n"
        "} instanceName;\n"
        "void main()\n"
        "{\n"
        "    instanceName.data[0] = 3u;\n"
        "    instanceName.data[1] = 4u;\n"
        "}\n";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    glUseProgram(program);

    int bufferAlignOffset;
    glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &bufferAlignOffset);

    constexpr unsigned int kElementCount = 2;
    // The array stride are rounded up to the base alignment of a vec4 for std140 layout.
    constexpr unsigned int kArrayStride       = 16;
    constexpr unsigned int kMiddlePaddingSize = 1024;
    unsigned int kShaderUsedSize              = kElementCount * kArrayStride;
    unsigned int kOffset1    = (kShaderUsedSize + bufferAlignOffset - 1) & ~(bufferAlignOffset - 1);
    unsigned int kOffset2    = kOffset1 + kMiddlePaddingSize;
    unsigned int kBufferSize = kOffset2 + kShaderUsedSize;

    for (int loop = 0; loop < 2; loop++)
    {
        // Create shader storage buffer
        GLBuffer shaderStorageBuffer;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, kBufferSize, nullptr, GL_DYNAMIC_DRAW);

        // Bind shader storage buffer and dispath compute
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer);
        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer, kOffset2,
                          kShaderUsedSize);
        glDispatchCompute(1, 1, 1);
        EXPECT_GL_NO_ERROR();

        if (loop == 1)
        {
            // Make write operation finished but read operation pending. We don't care actual
            // rendering result but just to have a unflushed rendering using the buffer so that it
            // will appears as pending.
            glFinish();
            constexpr char kVS[] = R"(attribute vec4 in_attrib;
                                    varying vec4 v_attrib;
                                    void main()
                                    {
                                        v_attrib = in_attrib;
                                        gl_Position = vec4(0.0, 0.0, 0.5, 1.0);
                                        gl_PointSize = 100.0;
                                    })";
            constexpr char kFS[] = R"(precision mediump float;
                                    varying vec4 v_attrib;
                                    void main()
                                    {
                                        gl_FragColor = v_attrib;
                                    })";
            GLuint readProgram   = CompileProgram(kVS, kFS);
            ASSERT_NE(readProgram, 0U);
            GLint attribLocation = glGetAttribLocation(readProgram, "in_attrib");
            ASSERT_NE(attribLocation, -1);
            glUseProgram(readProgram);
            ASSERT_GL_NO_ERROR();
            glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT);
            glBindBuffer(GL_ARRAY_BUFFER, shaderStorageBuffer);
            glVertexAttribPointer(attribLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4, nullptr);
            glEnableVertexAttribArray(attribLocation);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shaderStorageBuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, nullptr);
            ASSERT_GL_NO_ERROR();
        }

        // Use subData to update middle portion of data to trigger acquireAndUpdate code path in
        // ANGLE
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
        constexpr unsigned int kMiddlePaddingValue = 0x55555555u;
        std::vector<unsigned int> kMiddlePaddingValues(kMiddlePaddingSize / sizeof(unsigned int),
                                                       kMiddlePaddingValue);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, kOffset1, kMiddlePaddingSize,
                        kMiddlePaddingValues.data());

        // Read back shader storage buffer
        constexpr unsigned int kExpectedValues[2] = {3u, 4u};
        const GLbyte *ptr0                        = reinterpret_cast<const GLbyte *>(
            glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, GL_MAP_READ_BIT));
        for (unsigned int idx = 0; idx < kElementCount; idx++)
        {
            EXPECT_EQ(kExpectedValues[idx],
                      *(reinterpret_cast<const GLuint *>(ptr0 + idx * kArrayStride)));
        }

        const GLbyte *ptr1 = reinterpret_cast<const GLbyte *>(ptr0 + kOffset1);
        for (unsigned int idx = 0; idx < kMiddlePaddingSize / sizeof(unsigned int); idx++)
        {
            EXPECT_EQ(kMiddlePaddingValue, reinterpret_cast<const GLuint *>(ptr1)[idx]);
        }

        const GLbyte *ptr2 = ptr1 + kMiddlePaddingSize;
        for (unsigned int idx = 0; idx < kElementCount; idx++)
        {
            EXPECT_EQ(kExpectedValues[idx],
                      *(reinterpret_cast<const GLuint *>(ptr2 + idx * kArrayStride)));
        }

        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        EXPECT_GL_NO_ERROR();
    }
}

// Tests modifying an existing shader storage buffer
TEST_P(ShaderStorageBufferTest31, ShaderStorageBufferReadWriteSame)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding = 0) buffer block {
    uint data;
} instance;
void main()
{
    uint temp = instance.data;
    instance.data = temp + 1u;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);

    glUseProgram(program);

    constexpr unsigned int kBytesPerComponent = sizeof(GLuint);
    constexpr unsigned int kInitialData       = 123u;

    // Create shader storage buffer
    GLBuffer shaderStorageBuffer;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBytesPerComponent, &kInitialData, GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    const void *bufferData =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBytesPerComponent, GL_MAP_READ_BIT);

    constexpr unsigned int kExpectedData = 124u;
    EXPECT_EQ(kExpectedData, *static_cast<const GLuint *>(bufferData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Running shader twice to make sure that the buffer gets updated correctly 123->124->125
    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_PIXEL_BUFFER_BARRIER_BIT);

    bufferData = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBytesPerComponent, GL_MAP_READ_BIT);

    constexpr unsigned int kExpectedData2 = 125u;
    EXPECT_EQ(kExpectedData2, *static_cast<const GLuint *>(bufferData));

    // Verify re-using the SSBO buffer with a PBO contains expected data.
    // This will read-back from FBO using a PBO into the same SSBO buffer.

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, shaderStorageBuffer);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    EXPECT_GL_NO_ERROR();

    void *mappedPtr =
        glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, kBytesPerComponent, GL_MAP_READ_BIT);
    GLColor *dataColor = static_cast<GLColor *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor::red, dataColor[0]);
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    EXPECT_GL_NO_ERROR();

    // Verify that binding the buffer back to the SSBO keeps the expected data.

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    const GLColor *ptr = reinterpret_cast<GLColor *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBytesPerComponent, GL_MAP_READ_BIT));
    EXPECT_EQ(GLColor::red, *ptr);

    EXPECT_GL_NO_ERROR();
}

// Tests reading and writing to a shader storage buffer bound at an offset.
TEST_P(ShaderStorageBufferTest31, ShaderStorageBufferReadWriteOffset)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;

layout(std140, binding = 0) buffer block0 {
    uint data[2];
} instance0;

void main()
{
    instance0.data[0] = 3u;
    instance0.data[1] = 4u;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    glUseProgram(program);

    constexpr unsigned int kElementCount = 2;
    // The array stride are rounded up to the base alignment of a vec4 for std140 layout.
    constexpr unsigned int kArrayStride = 16;
    // Create shader storage buffer
    GLBuffer shaderStorageBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);

    int bufferAlignOffset;
    glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &bufferAlignOffset);

    constexpr int kBufferSize = kElementCount * kArrayStride;
    const int unalignedBytes  = kBufferSize % bufferAlignOffset;
    const int alignCorrection = unalignedBytes == 0 ? 0 : bufferAlignOffset - unalignedBytes;
    const int kBufferOffset   = kBufferSize + alignCorrection;

    glBufferData(GL_SHADER_STORAGE_BUFFER, kBufferOffset + kBufferSize, nullptr, GL_STATIC_DRAW);

    // Bind shader storage buffer at an offset
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer, kBufferOffset, kBufferSize);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);

    // Bind the buffer at a separate location
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer, 0, kBufferSize);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // Read back shader storage buffer
    constexpr unsigned int kExpectedValues[2] = {3u, 4u};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    void *ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, GL_MAP_READ_BIT);
    for (unsigned int idx = 0; idx < kElementCount; idx++)
    {
        EXPECT_EQ(kExpectedValues[idx],
                  *(reinterpret_cast<const GLuint *>(reinterpret_cast<const GLbyte *>(ptr) +
                                                     idx * kArrayStride)));
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, kBufferOffset, kBufferSize, GL_MAP_READ_BIT);
    for (unsigned int idx = 0; idx < kElementCount; idx++)
    {
        EXPECT_EQ(kExpectedValues[idx],
                  *(reinterpret_cast<const GLuint *>(reinterpret_cast<const GLbyte *>(ptr) +
                                                     idx * kArrayStride)));
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    EXPECT_GL_NO_ERROR();
}

// Test that access/write to vector data in shader storage buffer.
TEST_P(ShaderStorageBufferTest31, ShaderStorageBufferVector)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
 layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
 layout(std140, binding = 0) buffer blockIn {
     uvec2 data;
 } instanceIn;
 layout(std140, binding = 1) buffer blockOut {
     uvec2 data;
 } instanceOut;
 void main()
 {
     instanceOut.data[0] = instanceIn.data[0];
     instanceOut.data[1] = instanceIn.data[1];
 }
 )";

    constexpr unsigned int kComponentCount         = 2;
    constexpr GLuint kInputValues[kComponentCount] = {3u, 4u};

    VectorCase vectorCase(kComponentCount, kComputeShaderSource, kInputValues, kInputValues);
    runVectorTest(vectorCase);
}

// Test that the shader works well with an active SSBO but not statically used.
TEST_P(ShaderStorageBufferTest31, ActiveSSBOButNotStaticallyUsed)
{
    // http://anglebug.com/42262382
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsPixel2() && IsVulkan());

    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
 layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
 layout(std140, binding = 0) buffer blockIn {
     uvec2 data;
 } instanceIn;
 layout(std140, binding = 1) buffer blockOut {
     uvec2 data;
 } instanceOut;
 layout(std140, binding = 2) buffer blockC {
     uvec2 data;
 } instanceC;
 void main()
 {
     instanceOut.data[0] = instanceIn.data[0];
     instanceOut.data[1] = instanceIn.data[1];
 }
 )";

    GLBuffer shaderStorageBufferC;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBufferC);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 32, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, shaderStorageBufferC);

    constexpr unsigned int kComponentCount         = 2;
    constexpr GLuint kInputValues[kComponentCount] = {3u, 4u};

    VectorCase vectorCase(kComponentCount, kComputeShaderSource, kInputValues, kInputValues);
    runVectorTest(vectorCase);
}

// Test that access/write to swizzle scalar data in shader storage block.
TEST_P(ShaderStorageBufferTest31, ScalarSwizzleTest)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
 layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
 layout(std140, binding = 0) buffer blockIn {
     uvec2 data;
 } instanceIn;
 layout(std140, binding = 1) buffer blockOut {
     uvec2 data;
 } instanceOut;
 void main()
 {
     instanceOut.data.x = instanceIn.data.y;
     instanceOut.data.y = instanceIn.data.x;
 }
 )";

    constexpr unsigned int kComponentCount            = 2;
    constexpr GLuint kInputValues[kComponentCount]    = {3u, 4u};
    constexpr GLuint kExpectedValues[kComponentCount] = {4u, 3u};

    VectorCase vectorCase(kComponentCount, kComputeShaderSource, kInputValues, kExpectedValues);
    runVectorTest(vectorCase);
}

// Test that access/write to swizzle vector data in shader storage block.
TEST_P(ShaderStorageBufferTest31, VectorSwizzleTest)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
 layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
 layout(std140, binding = 0) buffer blockIn {
     uvec2 data;
 } instanceIn;
 layout(std140, binding = 1) buffer blockOut {
     uvec2 data;
 } instanceOut;
 void main()
 {
     instanceOut.data.yx = instanceIn.data.xy;
 }
 )";

    constexpr unsigned int kComponentCount            = 2;
    constexpr GLuint kInputValues[kComponentCount]    = {3u, 4u};
    constexpr GLuint kExpectedValues[kComponentCount] = {4u, 3u};

    VectorCase vectorCase(kComponentCount, kComputeShaderSource, kInputValues, kExpectedValues);
    runVectorTest(vectorCase);
}

// Test that access/write to swizzle vector data in column_major matrix in shader storage block.
TEST_P(ShaderStorageBufferTest31, VectorSwizzleInColumnMajorMatrixTest)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
 layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
 layout(std140, binding = 0) buffer blockIn {
     layout(column_major) mat2x3 data;
 } instanceIn;
 layout(std140, binding = 1) buffer blockOut {
     layout(column_major) mat2x3 data;
 } instanceOut;
 void main()
 {
     instanceOut.data[0].xyz = instanceIn.data[0].xyz;
     instanceOut.data[1].xyz = instanceIn.data[1].xyz;
 }
 )";

    constexpr unsigned int kColumns                                             = 2;
    constexpr unsigned int kRows                                                = 3;
    constexpr unsigned int kBytesPerComponent                                   = sizeof(float);
    constexpr unsigned int kMatrixStride                                        = 16;
    constexpr float kInputDada[kColumns * (kMatrixStride / kBytesPerComponent)] = {
        0.1, 0.2, 0.3, 0.0, 0.4, 0.5, 0.6, 0.0};
    MatrixCase matrixCase(kRows, kColumns, kMatrixStride, kComputeShaderSource, kInputDada);
    runMatrixTest(matrixCase);
}

// Test that access/write to swizzle vector data in row_major matrix in shader storage block.
TEST_P(ShaderStorageBufferTest31, VectorSwizzleInRowMajorMatrixTest)
{
    ANGLE_SKIP_TEST_IF(IsAndroid());

    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
 layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
 layout(std140, binding = 0) buffer blockIn {
     layout(row_major) mat2x3 data;
 } instanceIn;
 layout(std140, binding = 1) buffer blockOut {
     layout(row_major) mat2x3 data;
 } instanceOut;
 void main()
 {
     instanceOut.data[0].xyz = instanceIn.data[0].xyz;
     instanceOut.data[1].xyz = instanceIn.data[1].xyz;
 }
 )";

    constexpr unsigned int kColumns           = 2;
    constexpr unsigned int kRows              = 3;
    constexpr unsigned int kBytesPerComponent = sizeof(float);
    // std140 layout requires that base alignment and stride of arrays of scalars and vectors are
    // rounded up a multiple of the base alignment of a vec4.
    constexpr unsigned int kMatrixStride                                     = 16;
    constexpr float kInputDada[kRows * (kMatrixStride / kBytesPerComponent)] = {
        0.1, 0.2, 0.0, 0.0, 0.3, 0.4, 0.0, 0.0, 0.5, 0.6, 0.0, 0.0};
    MatrixCase matrixCase(kColumns, kRows, kMatrixStride, kComputeShaderSource, kInputDada);
    runMatrixTest(matrixCase);
}

// Test that access/write to scalar data in matrix in shader storage block with row major.
TEST_P(ShaderStorageBufferTest31, ScalarDataInMatrixInSSBOWithRowMajorQualifier)
{
    // TODO(jiajia.qin@intel.com): Figure out why it fails on Intel Linux platform.
    // http://anglebug.com/40644618
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux());
    ANGLE_SKIP_TEST_IF(IsAndroid());

    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding = 0) buffer blockIn {
    layout(row_major) mat2x3 data;
} instanceIn;
layout(std140, binding = 1) buffer blockOut {
    layout(row_major) mat2x3 data;
} instanceOut;
void main()
{
    instanceOut.data[0][0] = instanceIn.data[0][0];
    instanceOut.data[0][1] = instanceIn.data[0][1];
    instanceOut.data[0][2] = instanceIn.data[0][2];
    instanceOut.data[1][0] = instanceIn.data[1][0];
    instanceOut.data[1][1] = instanceIn.data[1][1];
    instanceOut.data[1][2] = instanceIn.data[1][2];
}
)";

    constexpr unsigned int kColumns           = 2;
    constexpr unsigned int kRows              = 3;
    constexpr unsigned int kBytesPerComponent = sizeof(float);
    // std140 layout requires that base alignment and stride of arrays of scalars and vectors are
    // rounded up a multiple of the base alignment of a vec4.
    constexpr unsigned int kMatrixStride                                     = 16;
    constexpr float kInputDada[kRows * (kMatrixStride / kBytesPerComponent)] = {
        0.1, 0.2, 0.0, 0.0, 0.3, 0.4, 0.0, 0.0, 0.5, 0.6, 0.0, 0.0};
    MatrixCase matrixCase(kColumns, kRows, kMatrixStride, kComputeShaderSource, kInputDada);
    runMatrixTest(matrixCase);
}

TEST_P(ShaderStorageBufferTest31, VectorDataInMatrixInSSBOWithRowMajorQualifier)
{
    ANGLE_SKIP_TEST_IF(IsAndroid());

    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding = 0) buffer blockIn {
    layout(row_major) mat2x3 data;
} instanceIn;
layout(std140, binding = 1) buffer blockOut {
    layout(row_major) mat2x3 data;
} instanceOut;
void main()
{
    instanceOut.data[0] = instanceIn.data[0];
    instanceOut.data[1] = instanceIn.data[1];
}
)";

    constexpr unsigned int kColumns           = 2;
    constexpr unsigned int kRows              = 3;
    constexpr unsigned int kBytesPerComponent = sizeof(float);
    // std140 layout requires that base alignment and stride of arrays of scalars and vectors are
    // rounded up a multiple of the base alignment of a vec4.
    constexpr unsigned int kMatrixStride                                     = 16;
    constexpr float kInputDada[kRows * (kMatrixStride / kBytesPerComponent)] = {
        0.1, 0.2, 0.0, 0.0, 0.3, 0.4, 0.0, 0.0, 0.5, 0.6, 0.0, 0.0};
    MatrixCase matrixCase(kColumns, kRows, kMatrixStride, kComputeShaderSource, kInputDada);
    runMatrixTest(matrixCase);
}

TEST_P(ShaderStorageBufferTest31, MatrixDataInSSBOWithRowMajorQualifier)
{
    ANGLE_SKIP_TEST_IF(IsAMD() && IsWindows() && IsOpenGL());

    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding = 0) buffer blockIn {
    layout(row_major) mat2x3 data;
} instanceIn;
layout(std140, binding = 1) buffer blockOut {
    layout(row_major) mat2x3 data;
} instanceOut;
void main()
{
    instanceOut.data = instanceIn.data;
}
)";

    constexpr unsigned int kColumns           = 2;
    constexpr unsigned int kRows              = 3;
    constexpr unsigned int kBytesPerComponent = sizeof(float);
    // std140 layout requires that base alignment and stride of arrays of scalars and vectors are
    // rounded up a multiple of the base alignment of a vec4.
    constexpr unsigned int kMatrixStride                                     = 16;
    constexpr float kInputDada[kRows * (kMatrixStride / kBytesPerComponent)] = {
        0.1, 0.2, 0.0, 0.0, 0.3, 0.4, 0.0, 0.0, 0.5, 0.6, 0.0, 0.0};
    MatrixCase matrixCase(kColumns, kRows, kMatrixStride, kComputeShaderSource, kInputDada);
    runMatrixTest(matrixCase);
}

// Test that access/write to scalar data in structure matrix in shader storage block with row major.
TEST_P(ShaderStorageBufferTest31, ScalarDataInMatrixInStructureInSSBOWithRowMajorQualifier)
{
    // TODO(jiajia.qin@intel.com): Figure out why it fails on Intel Linux platform.
    // http://anglebug.com/40644618
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux());
    ANGLE_SKIP_TEST_IF(IsAndroid());

    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
struct S
{
    mat2x3 data;
};
layout(std140, binding = 0) buffer blockIn {
    layout(row_major) S s;
} instanceIn;
layout(std140, binding = 1) buffer blockOut {
    layout(row_major) S s;
} instanceOut;
void main()
{
    instanceOut.s.data[0][0] = instanceIn.s.data[0][0];
    instanceOut.s.data[0][1] = instanceIn.s.data[0][1];
    instanceOut.s.data[0][2] = instanceIn.s.data[0][2];
    instanceOut.s.data[1][0] = instanceIn.s.data[1][0];
    instanceOut.s.data[1][1] = instanceIn.s.data[1][1];
    instanceOut.s.data[1][2] = instanceIn.s.data[1][2];
}
)";

    constexpr unsigned int kColumns           = 2;
    constexpr unsigned int kRows              = 3;
    constexpr unsigned int kBytesPerComponent = sizeof(float);
    // std140 layout requires that base alignment and stride of arrays of scalars and vectors are
    // rounded up a multiple of the base alignment of a vec4.
    constexpr unsigned int kMatrixStride                                     = 16;
    constexpr float kInputDada[kRows * (kMatrixStride / kBytesPerComponent)] = {
        0.1, 0.2, 0.0, 0.0, 0.3, 0.4, 0.0, 0.0, 0.5, 0.6, 0.0, 0.0};
    MatrixCase matrixCase(kColumns, kRows, kMatrixStride, kComputeShaderSource, kInputDada);
    runMatrixTest(matrixCase);
}

// Test that access/write to column major matrix data in shader storage buffer.
TEST_P(ShaderStorageBufferTest31, ScalarDataInMatrixInSSBO)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding = 0) buffer blockIn {
    mat2x3 data;
} instanceIn;
layout(std140, binding = 1) buffer blockOut {
    mat2x3 data;
} instanceOut;
void main()
{
    instanceOut.data[0][0] = instanceIn.data[0][0];
    instanceOut.data[0][1] = instanceIn.data[0][1];
    instanceOut.data[0][2] = instanceIn.data[0][2];
    instanceOut.data[1][0] = instanceIn.data[1][0];
    instanceOut.data[1][1] = instanceIn.data[1][1];
    instanceOut.data[1][2] = instanceIn.data[1][2];
}
)";

    constexpr unsigned int kColumns                                             = 2;
    constexpr unsigned int kRows                                                = 3;
    constexpr unsigned int kBytesPerComponent                                   = sizeof(float);
    constexpr unsigned int kMatrixStride                                        = 16;
    constexpr float kInputDada[kColumns * (kMatrixStride / kBytesPerComponent)] = {
        0.1, 0.2, 0.3, 0.0, 0.4, 0.5, 0.6, 0.0};
    MatrixCase matrixCase(kRows, kColumns, kMatrixStride, kComputeShaderSource, kInputDada);
    runMatrixTest(matrixCase);
}

TEST_P(ShaderStorageBufferTest31, VectorDataInMatrixInSSBOWithColumnMajorQualifier)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding = 0) buffer blockIn {
    layout(column_major) mat2x3 data;
} instanceIn;
layout(std140, binding = 1) buffer blockOut {
    layout(column_major) mat2x3 data;
} instanceOut;
void main()
{
    instanceOut.data[0] = instanceIn.data[0];
    instanceOut.data[1] = instanceIn.data[1];
}
)";

    constexpr unsigned int kColumns                                             = 2;
    constexpr unsigned int kRows                                                = 3;
    constexpr unsigned int kBytesPerComponent                                   = sizeof(float);
    constexpr unsigned int kMatrixStride                                        = 16;
    constexpr float kInputDada[kColumns * (kMatrixStride / kBytesPerComponent)] = {
        0.1, 0.2, 0.3, 0.0, 0.4, 0.5, 0.6, 0.0};
    MatrixCase matrixCase(kRows, kColumns, kMatrixStride, kComputeShaderSource, kInputDada);
    runMatrixTest(matrixCase);
}

TEST_P(ShaderStorageBufferTest31, MatrixDataInSSBOWithColumnMajorQualifier)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding = 0) buffer blockIn {
    layout(column_major) mat2x3 data;
} instanceIn;
layout(std140, binding = 1) buffer blockOut {
    layout(column_major) mat2x3 data;
} instanceOut;
void main()
{
    instanceOut.data = instanceIn.data;
}
)";

    constexpr unsigned int kColumns                                             = 2;
    constexpr unsigned int kRows                                                = 3;
    constexpr unsigned int kBytesPerComponent                                   = sizeof(float);
    constexpr unsigned int kMatrixStride                                        = 16;
    constexpr float kInputDada[kColumns * (kMatrixStride / kBytesPerComponent)] = {
        0.1, 0.2, 0.3, 0.0, 0.4, 0.5, 0.6, 0.0};
    MatrixCase matrixCase(kRows, kColumns, kMatrixStride, kComputeShaderSource, kInputDada);
    runMatrixTest(matrixCase);
}

// Test that access/write to structure data in shader storage buffer.
TEST_P(ShaderStorageBufferTest31, ShaderStorageBufferStructureArray)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
struct S
{
    uvec2 uvData;
    uint uiData[2];
};
layout(std140, binding = 0) buffer blockIn {
    S s[2];
    uint lastData;
} instanceIn;
layout(std140, binding = 1) buffer blockOut {
    S s[2];
    uint lastData;
} instanceOut;
void main()
{
    instanceOut.s[0].uvData = instanceIn.s[0].uvData;
    instanceOut.s[0].uiData[0] = instanceIn.s[0].uiData[0];
    instanceOut.s[0].uiData[1] = instanceIn.s[0].uiData[1];
    instanceOut.s[1].uvData = instanceIn.s[1].uvData;
    instanceOut.s[1].uiData[0] = instanceIn.s[1].uiData[0];
    instanceOut.s[1].uiData[1] = instanceIn.s[1].uiData[1];
    instanceOut.lastData = instanceIn.lastData;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);

    glUseProgram(program);

    std::array<GLuint, 4> kUVData = {{
        1u,
        2u,
        0u,
        0u,
    }};
    std::array<GLuint, 8> kUIData = {{
        3u,
        0u,
        0u,
        0u,
        4u,
        0u,
        0u,
        0u,
    }};
    GLuint kLastData              = 5u;

    constexpr unsigned int kBytesPerComponent = sizeof(GLuint);
    constexpr unsigned int kStructureStride   = 48;
    constexpr unsigned int totalSize          = kStructureStride * 2 + sizeof(kLastData);

    // Create shader storage buffer
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);
    GLint offset = 0;
    // upload data to instanceIn.s[0]
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, kUVData.size() * kBytesPerComponent,
                    kUVData.data());
    offset += (kUVData.size() * kBytesPerComponent);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, kUIData.size() * kBytesPerComponent,
                    kUIData.data());
    offset += (kUIData.size() * kBytesPerComponent);
    // upload data to instanceIn.s[1]
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, kUVData.size() * kBytesPerComponent,
                    kUVData.data());
    offset += (kUVData.size() * kBytesPerComponent);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, kUIData.size() * kBytesPerComponent,
                    kUIData.data());
    offset += (kUIData.size() * kBytesPerComponent);
    // upload data to instanceIn.lastData
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(kLastData), &kLastData);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    // Read back shader storage buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    constexpr float kExpectedValues[5] = {1u, 2u, 3u, 4u, 5u};
    const GLuint *ptr                  = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, totalSize, GL_MAP_READ_BIT));
    // instanceOut.s[0]
    EXPECT_EQ(kExpectedValues[0], *ptr);
    EXPECT_EQ(kExpectedValues[1], *(ptr + 1));
    EXPECT_EQ(kExpectedValues[2], *(ptr + 4));
    EXPECT_EQ(kExpectedValues[3], *(ptr + 8));
    // instanceOut.s[1]
    ptr += kStructureStride / kBytesPerComponent;
    EXPECT_EQ(kExpectedValues[0], *ptr);
    EXPECT_EQ(kExpectedValues[1], *(ptr + 1));
    EXPECT_EQ(kExpectedValues[2], *(ptr + 4));
    EXPECT_EQ(kExpectedValues[3], *(ptr + 8));
    // instanceOut.lastData
    ptr += kStructureStride / kBytesPerComponent;
    EXPECT_EQ(kExpectedValues[4], *(ptr));

    EXPECT_GL_NO_ERROR();
}

// Test that access/write to array of array structure data in shader storage buffer.
TEST_P(ShaderStorageBufferTest31, ShaderStorageBufferStructureArrayOfArray)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
struct S
{
    uvec2 uvData;
    uint uiData[2];
};
layout(std140, binding = 0) buffer blockIn {
    S s[3][2];
    uint lastData;
} instanceIn;
layout(std140, binding = 1) buffer blockOut {
    S s[3][2];
    uint lastData;
} instanceOut;
void main()
{
    instanceOut.s[1][0].uvData = instanceIn.s[1][0].uvData;
    instanceOut.s[1][0].uiData[0] = instanceIn.s[1][0].uiData[0];
    instanceOut.s[1][0].uiData[1] = instanceIn.s[1][0].uiData[1];
    instanceOut.s[1][1].uvData = instanceIn.s[1][1].uvData;
    instanceOut.s[1][1].uiData[0] = instanceIn.s[1][1].uiData[0];
    instanceOut.s[1][1].uiData[1] = instanceIn.s[1][1].uiData[1];

    instanceOut.lastData = instanceIn.lastData;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);

    glUseProgram(program);

    std::array<GLuint, 4> kUVData = {{
        1u,
        2u,
        0u,
        0u,
    }};
    std::array<GLuint, 8> kUIData = {{
        3u,
        0u,
        0u,
        0u,
        4u,
        0u,
        0u,
        0u,
    }};
    GLuint kLastData              = 5u;

    constexpr unsigned int kBytesPerComponent        = sizeof(GLuint);
    constexpr unsigned int kStructureStride          = 48;
    constexpr unsigned int kStructureArrayDimension0 = 3;
    constexpr unsigned int kStructureArrayDimension1 = 2;
    constexpr unsigned int kLastDataOffset =
        kStructureStride * kStructureArrayDimension0 * kStructureArrayDimension1;
    constexpr unsigned int totalSize = kLastDataOffset + sizeof(kLastData);

    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);
    // offset of instanceIn.s[1][0]
    GLint offset      = kStructureStride * (kStructureArrayDimension1 * 1 + 0);
    GLuint uintOffset = offset / kBytesPerComponent;
    // upload data to instanceIn.s[1][0]
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, kUVData.size() * kBytesPerComponent,
                    kUVData.data());
    offset += (kUVData.size() * kBytesPerComponent);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, kUIData.size() * kBytesPerComponent,
                    kUIData.data());
    offset += (kUIData.size() * kBytesPerComponent);
    // upload data to instanceIn.s[1][1]
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, kUVData.size() * kBytesPerComponent,
                    kUVData.data());
    offset += (kUVData.size() * kBytesPerComponent);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, kUIData.size() * kBytesPerComponent,
                    kUIData.data());
    // upload data to instanceIn.lastData
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, kLastDataOffset, sizeof(kLastData), &kLastData);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    // Read back shader storage buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    constexpr float kExpectedValues[5] = {1u, 2u, 3u, 4u, 5u};
    const GLuint *ptr                  = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, totalSize, GL_MAP_READ_BIT));

    // instanceOut.s[0][0]
    EXPECT_EQ(kExpectedValues[0], *(ptr + uintOffset));
    EXPECT_EQ(kExpectedValues[1], *(ptr + uintOffset + 1));
    EXPECT_EQ(kExpectedValues[2], *(ptr + uintOffset + 4));
    EXPECT_EQ(kExpectedValues[3], *(ptr + uintOffset + 8));

    // instanceOut.s[0][1]
    EXPECT_EQ(kExpectedValues[0], *(ptr + uintOffset + 12));
    EXPECT_EQ(kExpectedValues[1], *(ptr + uintOffset + 13));
    EXPECT_EQ(kExpectedValues[2], *(ptr + uintOffset + 16));
    EXPECT_EQ(kExpectedValues[3], *(ptr + uintOffset + 20));

    // instanceOut.lastData
    EXPECT_EQ(kExpectedValues[4], *(ptr + (kLastDataOffset / kBytesPerComponent)));

    EXPECT_GL_NO_ERROR();
}

// Test that access/write to vector data in std430 shader storage block.
TEST_P(ShaderStorageBufferTest31, VectorArrayInSSBOWithStd430Qualifier)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std430, binding = 0) buffer blockIn {
    uvec2 data[2];
} instanceIn;
layout(std430, binding = 1) buffer blockOut {
    uvec2 data[2];
} instanceOut;
void main()
{
    instanceOut.data[0] = instanceIn.data[0];
    instanceOut.data[1] = instanceIn.data[1];
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);

    glUseProgram(program);

    constexpr unsigned int kElementCount      = 2;
    constexpr unsigned int kBytesPerComponent = sizeof(unsigned int);
    constexpr unsigned int kArrayStride       = 8;
    constexpr unsigned int kComponentCount    = kArrayStride / kBytesPerComponent;
    constexpr unsigned int kExpectedValues[kElementCount][kComponentCount] = {{1u, 2u}, {3u, 4u}};
    // Create shader storage buffer
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kElementCount * kArrayStride, kExpectedValues,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kElementCount * kArrayStride, nullptr, GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);

    glFinish();

    // Read back shader storage buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, kElementCount * kArrayStride, GL_MAP_READ_BIT));
    for (unsigned int idx = 0; idx < kElementCount; idx++)
    {
        for (unsigned int idy = 0; idy < kComponentCount; idy++)
        {
            EXPECT_EQ(kExpectedValues[idx][idy], *(ptr + idx * kComponentCount + idy));
        }
    }

    EXPECT_GL_NO_ERROR();
}

// Test that access/write to matrix data in std430 shader storage block.
TEST_P(ShaderStorageBufferTest31, MatrixInSSBOWithStd430Qualifier)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std430, binding = 0) buffer blockIn {
    mat2 data;
} instanceIn;
layout(std430, binding = 1) buffer blockOut {
    mat2 data;
} instanceOut;
void main()
{
    instanceOut.data = instanceIn.data;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);

    glUseProgram(program);

    constexpr unsigned int kColumns              = 2;
    constexpr unsigned int kRows                 = 2;
    constexpr unsigned int kBytesPerComponent    = sizeof(float);
    constexpr unsigned int kMatrixStride         = kRows * kBytesPerComponent;
    constexpr float kInputDada[kColumns * kRows] = {0.1, 0.2, 0.4, 0.5};
    MatrixCase matrixCase(kRows, kColumns, kMatrixStride, kComputeShaderSource, kInputDada);
    runMatrixTest(matrixCase);
}

// Test that access/write to structure data in std430 shader storage block.
TEST_P(ShaderStorageBufferTest31, StructureInSSBOWithStd430Qualifier)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
struct S
{
    uvec2 u;
};
layout(std430, binding = 0) buffer blockIn {
    uint i1;
    S s;
    uint i2;
} instanceIn;
layout(std430, binding = 1) buffer blockOut {
    uint i1;
    S s;
    uint i2;
} instanceOut;
void main()
{
    instanceOut.i1 = instanceIn.i1;
    instanceOut.s.u = instanceIn.s.u;
    instanceOut.i2 = instanceIn.i2;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    glUseProgram(program);

    GLuint kI1Data               = 1u;
    std::array<GLuint, 2> kUData = {{
        2u,
        3u,
    }};
    GLuint kI2Data               = 4u;

    constexpr unsigned int kBytesPerComponent    = sizeof(GLuint);
    constexpr unsigned int kStructureStartOffset = 8;
    constexpr unsigned int kStructureSize        = 8;
    constexpr unsigned int kTotalSize = kStructureStartOffset + kStructureSize + kBytesPerComponent;

    // Create shader storage buffer
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kTotalSize, nullptr, GL_STATIC_DRAW);
    // upload data to instanceIn.i1
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, kBytesPerComponent, &kI1Data);
    // upload data to instanceIn.s.u
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, kStructureStartOffset, kStructureSize, kUData.data());
    // upload data to instanceIn.i2
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, kStructureStartOffset + kStructureSize,
                    kBytesPerComponent, &kI2Data);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kTotalSize, nullptr, GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    // Read back shader storage buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    GLuint kExpectedValues[4] = {1u, 2u, 3u, 4u};
    const GLuint *ptr         = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kTotalSize, GL_MAP_READ_BIT));
    EXPECT_EQ(kExpectedValues[0], *ptr);
    ptr += (kStructureStartOffset / kBytesPerComponent);
    EXPECT_EQ(kExpectedValues[1], *ptr);
    EXPECT_EQ(kExpectedValues[2], *(ptr + 1));
    ptr += (kStructureSize / kBytesPerComponent);
    EXPECT_EQ(kExpectedValues[3], *ptr);

    EXPECT_GL_NO_ERROR();
}

// Test that access/write to structure of structure data in std430 shader storage block.
TEST_P(ShaderStorageBufferTest31, StructureOfStructureInSSBOWithStd430Qualifier)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
struct S2
{
    uvec3 u2;
};
struct S1
{
    uvec2 u1;
    S2 s2;
};
layout(std430, binding = 0) buffer blockIn {
    uint i1;
    S1 s1;
    uint i2;
} instanceIn;
layout(std430, binding = 1) buffer blockOut {
    uint i1;
    S1 s1;
    uint i2;
} instanceOut;
void main()
{
    instanceOut.i1 = instanceIn.i1;
    instanceOut.s1.u1 = instanceIn.s1.u1;
    instanceOut.s1.s2.u2 = instanceIn.s1.s2.u2;
    instanceOut.i2 = instanceIn.i2;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    glUseProgram(program);

    constexpr unsigned int kBytesPerComponent      = sizeof(GLuint);
    constexpr unsigned int kStructureS1StartOffset = 16;
    constexpr unsigned int kStructureS2StartOffset = 32;
    constexpr unsigned int kStructureS1Size        = 32;
    constexpr unsigned int kTotalSize =
        kStructureS1StartOffset + kStructureS1Size + kBytesPerComponent;

    GLuint kI1Data                = 1u;
    std::array<GLuint, 2> kU1Data = {{2u, 3u}};
    std::array<GLuint, 3> kU2Data = {{4u, 5u, 6u}};
    GLuint kI2Data                = 7u;

    // Create shader storage buffer
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kTotalSize, nullptr, GL_STATIC_DRAW);
    // upload data to instanceIn.i1
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, kBytesPerComponent, &kI1Data);
    // upload data to instanceIn.s1.u1
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, kStructureS1StartOffset,
                    kU1Data.size() * kBytesPerComponent, kU1Data.data());
    // upload data to instanceIn.s1.s2.u2
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, kStructureS2StartOffset,
                    kU2Data.size() * kBytesPerComponent, kU2Data.data());
    // upload data to instanceIn.i2
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, kStructureS1StartOffset + kStructureS1Size,
                    kBytesPerComponent, &kI2Data);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kTotalSize, nullptr, GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    // Read back shader storage buffer
    GLuint kExpectedValues[7] = {1u, 2u, 3u, 4u, 5u, 6u, 7u};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kTotalSize, GL_MAP_READ_BIT));
    EXPECT_EQ(kExpectedValues[0], *ptr);
    ptr += (kStructureS1StartOffset / kBytesPerComponent);
    EXPECT_EQ(kExpectedValues[1], *ptr);
    EXPECT_EQ(kExpectedValues[2], *(ptr + 1));
    ptr += ((kStructureS2StartOffset - kStructureS1StartOffset) / kBytesPerComponent);
    EXPECT_EQ(kExpectedValues[3], *ptr);
    EXPECT_EQ(kExpectedValues[4], *(ptr + 1));
    EXPECT_EQ(kExpectedValues[5], *(ptr + 2));
    ptr += ((kStructureS1Size - kStructureS2StartOffset) / kBytesPerComponent);
    EXPECT_EQ(kExpectedValues[6], *(ptr + 4));

    EXPECT_GL_NO_ERROR();
}

// Test atomic memory functions.
TEST_P(ShaderStorageBufferTest31, AtomicMemoryFunctions)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding = 1) buffer blockName {
    uint data[2];
} instanceName;

void main()
{
    instanceName.data[0] = 0u;
    instanceName.data[1] = 0u;
    atomicAdd(instanceName.data[0], 5u);
    atomicMax(instanceName.data[1], 7u);
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    glUseProgram(program);

    constexpr unsigned int kElementCount = 2;
    // The array stride are rounded up to the base alignment of a vec4 for std140 layout.
    constexpr unsigned int kArrayStride = 16;
    // Create shader storage buffer
    GLBuffer shaderStorageBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kElementCount * kArrayStride, nullptr, GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer);

    // Dispath compute
    glDispatchCompute(1, 1, 1);

    glFinish();

    // Read back shader storage buffer
    constexpr unsigned int kExpectedValues[2] = {5u, 7u};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    void *ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kElementCount * kArrayStride,
                                 GL_MAP_READ_BIT);
    for (unsigned int idx = 0; idx < kElementCount; idx++)
    {
        EXPECT_EQ(kExpectedValues[idx],
                  *(reinterpret_cast<const GLuint *>(reinterpret_cast<const GLbyte *>(ptr) +
                                                     idx * kArrayStride)));
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    EXPECT_GL_NO_ERROR();
}

// Test multiple storage buffers work correctly when program switching. In angle, storage buffer
// bindings are updated accord to current program. If switch program, need to update storage buffer
// bindings again.
TEST_P(ShaderStorageBufferTest31, MultiStorageBuffersForMultiPrograms)
{
    constexpr char kCS1[] = R"(#version 310 es
layout(local_size_x=3, local_size_y=1, local_size_z=1) in;
layout(binding = 1) buffer Output {
    uint result1[];
} sb_out1;
void main()
{
    highp uint offset = gl_LocalInvocationID.x;
    sb_out1.result1[gl_LocalInvocationIndex] = gl_LocalInvocationIndex + 1u;
})";

    constexpr char kCS2[] = R"(#version 310 es
layout(local_size_x=3, local_size_y=1, local_size_z=1) in;
layout(binding = 2) buffer Output {
    uint result2[];
} sb_out2;
void main()
{
    highp uint offset = gl_LocalInvocationID.x;
    sb_out2.result2[gl_LocalInvocationIndex] = gl_LocalInvocationIndex + 2u;
})";

    constexpr unsigned int numInvocations = 3;
    int arrayStride1 = 0, arrayStride2 = 0;
    GLenum props[] = {GL_ARRAY_STRIDE};
    GLBuffer shaderStorageBuffer1, shaderStorageBuffer2;

    ANGLE_GL_COMPUTE_PROGRAM(program1, kCS1);
    ANGLE_GL_COMPUTE_PROGRAM(program2, kCS2);
    EXPECT_GL_NO_ERROR();

    unsigned int outVarIndex1 =
        glGetProgramResourceIndex(program1, GL_BUFFER_VARIABLE, "Output.result1");
    glGetProgramResourceiv(program1, GL_BUFFER_VARIABLE, outVarIndex1, 1, props, 1, 0,
                           &arrayStride1);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer1);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numInvocations * arrayStride1, nullptr, GL_STREAM_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer1);
    EXPECT_GL_NO_ERROR();

    unsigned int outVarIndex2 =
        glGetProgramResourceIndex(program2, GL_BUFFER_VARIABLE, "Output.result2");
    glGetProgramResourceiv(program2, GL_BUFFER_VARIABLE, outVarIndex2, 1, props, 1, 0,
                           &arrayStride2);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer2);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numInvocations * arrayStride2, nullptr, GL_STREAM_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, shaderStorageBuffer2);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program1);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();
    glUseProgram(program2);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer1);
    const void *ptr1 =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 3 * arrayStride1, GL_MAP_READ_BIT);
    for (unsigned int idx = 0; idx < numInvocations; idx++)
    {
        EXPECT_EQ(idx + 1, *((const GLuint *)((const GLbyte *)ptr1 + idx * arrayStride1)));
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer2);
    const void *ptr2 =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 3 * arrayStride2, GL_MAP_READ_BIT);
    EXPECT_GL_NO_ERROR();
    for (unsigned int idx = 0; idx < numInvocations; idx++)
    {
        EXPECT_EQ(idx + 2, *((const GLuint *)((const GLbyte *)ptr2 + idx * arrayStride2)));
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    EXPECT_GL_NO_ERROR();
}

// Test that function calling is supported in SSBO access chain.
TEST_P(ShaderStorageBufferTest31, FunctionCallInSSBOAccessChain)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout (local_size_x=4) in;
highp uint getIndex (in highp uvec2 localID, uint element)
{
    return localID.x + element;
}
layout(binding=0, std430) buffer Storage
{
    highp uint values[];
} sb_store;

void main()
{
    sb_store.values[getIndex(gl_LocalInvocationID.xy, 0u)] = gl_LocalInvocationIndex;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();
}

// Test that unary operator is supported in SSBO access chain.
TEST_P(ShaderStorageBufferTest31, UnaryOperatorInSSBOAccessChain)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout (local_size_x=4) in;
layout(binding=0, std430) buffer Storage
{
    highp uint values[];
} sb_store;

void main()
{
    uint invocationNdx = gl_LocalInvocationIndex;
    sb_store.values[++invocationNdx] = invocationNdx;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();
}

// Test that ternary operator is supported in SSBO access chain.
TEST_P(ShaderStorageBufferTest31, TernaryOperatorInSSBOAccessChain)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout (local_size_x=4) in;
layout(binding=0, std430) buffer Storage
{
    highp uint values[];
} sb_store;

void main()
{
    sb_store.values[gl_LocalInvocationIndex > 2u ? gl_NumWorkGroups.x : gl_NumWorkGroups.y]
            = gl_LocalInvocationIndex;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();
}

// Tests that alignment is correct for bools inside a SSB and that the values
// are written correctly by a trivial shader. Currently tests only the alignment
// of the initial block.
TEST_P(ShaderStorageBufferTest31, LoadAndStoreBooleanValue)
{
    // TODO(jiajia.qin@intel.com): Figure out why it fails on Intel Linux platform.
    // http://anglebug.com/40644618
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux());

    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout (local_size_x=1) in;
layout(binding=0, std140) buffer Storage0
{
    bool b1;
    bool b2;
    bool b3;
} sb_load;
layout(binding=1, std140) buffer Storage1
{
    bool b1;
    bool b2;
    bool b3;
} sb_store;
void main()
{
   sb_store.b1 = sb_load.b1;
   sb_store.b2 = sb_load.b2;
   sb_store.b3 = sb_load.b3;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    constexpr GLuint kB1Value                 = 1u;
    constexpr GLuint kB2Value[2]              = {0u, 1u};
    constexpr unsigned int kBytesPerComponent = sizeof(GLuint);
    // Create shader storage buffer
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 3 * kBytesPerComponent, nullptr, GL_STATIC_DRAW);
    GLint offset = 0;
    // upload data to sb_load.b1
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, kBytesPerComponent, &kB1Value);
    offset += kBytesPerComponent;
    // upload data to sb_load.b2
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, 2 * kBytesPerComponent, kB2Value);

    constexpr GLuint kStoreBufferContents[3] = {0x1BCD1234, 0x2BCD1234, 0x3BCD1234};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 3 * kBytesPerComponent, kStoreBufferContents,
                 GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    // Read back shader storage buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 3 * kBytesPerComponent, GL_MAP_READ_BIT));
    EXPECT_EQ(kB1Value, ptr[0]);
    EXPECT_EQ(kB2Value[0], ptr[1]);
    EXPECT_EQ(kB2Value[1], ptr[2]);

    EXPECT_GL_NO_ERROR();
}

// Tests that alignment is correct for bvecs3 inside a SSB and that the
// values are written correctly by a trivial shader. Currently tests only the
// alignment of the initial block.
TEST_P(ShaderStorageBufferTest31, LoadAndStoreBooleanVec3)
{
    // TODO(jiajia.qin@intel.com): Figure out why it fails on Intel Linux platform.
    // http://anglebug.com/40644618
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux());

    ANGLE_SKIP_TEST_IF(IsAMD() && IsWindows() && IsOpenGL());

    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout (local_size_x=1) in;
layout(binding=0, std140) buffer Storage0
{
    bvec3 b;
} sb_load;
layout(binding=1, std140) buffer Storage1
{
    bvec3 b;
} sb_store;
void main()
{
   sb_store.b = sb_load.b;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    constexpr GLuint kBValues[3]              = {1u, 0u, 1u};
    constexpr unsigned int kBytesPerComponent = sizeof(GLuint);
    // Create shader storage buffer
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 3 * kBytesPerComponent, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 3 * kBytesPerComponent, &kBValues);

    constexpr GLuint kStoreBufferContents[3] = {0x1BCD1234, 0x2BCD1234, 0x3BCD1234};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 3 * kBytesPerComponent, kStoreBufferContents,
                 GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    // Read back shader storage buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 3 * kBytesPerComponent, GL_MAP_READ_BIT));
    EXPECT_EQ(kBValues[0], ptr[0]);
    EXPECT_EQ(kBValues[1], ptr[1]);
    EXPECT_EQ(kBValues[2], ptr[2]);

    EXPECT_GL_NO_ERROR();
}

// Tests that alignment is correct for bool + bvecs2 inside a SSB and that the
// values are written correctly by a trivial shader. Currently tests only the
// alignment of the initial block. Compare to LoadAndStoreBooleanVec3 to see how
// the alignment rules affect the memory layout.
TEST_P(ShaderStorageBufferTest31, LoadAndStoreBooleanVarAndVec2)
{
    // TODO(jiajia.qin@intel.com): Figure out why it fails on Intel Linux platform.
    // http://anglebug.com/40644618
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux());

    ANGLE_SKIP_TEST_IF(IsAMD() && IsWindows() && IsOpenGL());

    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout (local_size_x=1) in;
layout(binding=0, std140) buffer Storage0
{
    bool b1;
    bvec2 b2;
} sb_load;
layout(binding=1, std140) buffer Storage1
{
    bool b1;
    bvec2 b2;
} sb_store;
void main()
{
   sb_store.b1 = sb_load.b1;
   sb_store.b2 = sb_load.b2;
}
)";
    // https://www.khronos.org/registry/OpenGL/specs/es/3.1/es_spec_3.1.pdf
    // 7.6.2.2 Standard Uniform Block Layout

    // ... A structure and each structure member have a base offset and a base
    // alignment, from which an aligned offset is computed by rounding the base
    // offset up to a multiple of the base alignment. The base offset of the
    // first member of a structure is taken from the aligned offset of the
    // structure itself. ... The members of a toplevel uniform block are laid
    // out in buffer storage by treating the uniform block as a structure with a
    // base offset of zero.

    // 1. If the member is a scalar consuming N basic machine units, the base
    // alignment is N.

    // 2. If the member is a two- or four-component vector with components
    // consuming N basic machine units, the base alignment is 2N or 4N,
    // respectively

    // b1 N == 4, basic offset 0, alignment 4, is at 0..3
    // b2 N == 4, basic offset 4, alignment 2*4 = 8, is at 8..16.

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    constexpr GLuint kAlignPadding  = 0x1abcd789u;
    constexpr GLuint kBValues[]     = {1u, kAlignPadding, 0u, 1u};
    constexpr unsigned int kSsbSize = sizeof(kBValues);
    // Create shader storage buffer
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kSsbSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, kSsbSize, &kBValues);

    constexpr GLuint kStoreBufferContents[4] = {0x1BCD1234, 0x2BCD1234, 0x3BCD1234, 0x3BCD1277};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kSsbSize, kStoreBufferContents, GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    // Read back shader storage buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kSsbSize, GL_MAP_READ_BIT));
    EXPECT_EQ(kBValues[0], ptr[0]);
    // Index 1 is padding.
    EXPECT_EQ(kBValues[2], ptr[2]);
    EXPECT_EQ(kBValues[3], ptr[3]);

    EXPECT_GL_NO_ERROR();
}

// Test that non-structure array of arrays is supported in SSBO.
TEST_P(ShaderStorageBufferTest31, SimpleArrayOfArrays)
{
    constexpr char kComputeShaderSource[] = R"(#version 310 es
layout (local_size_x=1) in;
layout(binding=0, std140) buffer Storage0
{
    uint a[2][2][2];
    uint b;
} sb_load;
layout(binding=1, std140) buffer Storage1
{
    uint a[2][2][2];
    uint b;
} sb_store;
void main()
{
   sb_store.a[0][0][0] = sb_load.a[0][0][0];
   sb_store.a[0][0][1] = sb_load.a[0][0][1];
   sb_store.a[0][1][0] = sb_load.a[0][1][0];
   sb_store.a[0][1][1] = sb_load.a[0][1][1];
   sb_store.a[1][0][0] = sb_load.a[1][0][0];
   sb_store.a[1][0][1] = sb_load.a[1][0][1];
   sb_store.a[1][1][0] = sb_load.a[1][1][0];
   sb_store.a[1][1][1] = sb_load.a[1][1][1];
   sb_store.b = sb_load.b;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    glUseProgram(program);

    constexpr unsigned int kBytesPerComponent = sizeof(GLuint);
    // The array stride are rounded up to the base alignment of a vec4 for std140 layout.
    constexpr unsigned int kArrayStride                 = 16;
    constexpr unsigned int kDimension0                  = 2;
    constexpr unsigned int kDimension1                  = 2;
    constexpr unsigned int kDimension2                  = 2;
    constexpr unsigned int kAElementCount               = kDimension0 * kDimension1 * kDimension2;
    constexpr unsigned int kAComponentCountPerDimension = kArrayStride / kBytesPerComponent;
    constexpr unsigned int kTotalSize = kArrayStride * kAElementCount + kBytesPerComponent;

    constexpr GLuint kInputADatas[kAElementCount * kAComponentCountPerDimension] = {
        1u, 0u, 0u, 0u, 2u, 0u, 0u, 0u, 3u, 0u, 0u, 0u, 4u, 0u, 0u, 0u,
        5u, 0u, 0u, 0u, 6u, 0u, 0u, 0u, 7u, 0u, 0u, 0u, 8u, 0u, 0u, 0u};
    constexpr GLuint kInputBData = 9u;

    // Create shader storage buffer
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kTotalSize, nullptr, GL_STATIC_DRAW);
    GLint offset = 0;
    // upload data to sb_load.a
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, kAElementCount * kArrayStride, kInputADatas);
    offset += (kAElementCount * kArrayStride);
    // upload data to sb_load.b
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, kBytesPerComponent, &kInputBData);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kTotalSize, nullptr, GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    // Read back shader storage buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    constexpr GLuint kExpectedADatas[kAElementCount] = {1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u};
    const GLuint *ptr                                = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kTotalSize, GL_MAP_READ_BIT));
    for (unsigned i = 0u; i < kDimension0; i++)
    {
        for (unsigned j = 0u; j < kDimension1; j++)
        {
            for (unsigned k = 0u; k < kDimension2; k++)
            {
                unsigned index = i * (kDimension1 * kDimension2) + j * kDimension2 + k;
                EXPECT_EQ(kExpectedADatas[index],
                          *(ptr + index * (kArrayStride / kBytesPerComponent)));
            }
        }
    }

    ptr += (kAElementCount * (kArrayStride / kBytesPerComponent));
    EXPECT_EQ(kInputBData, *ptr);

    EXPECT_GL_NO_ERROR();
}

// Test that the length of unsized array is supported.
TEST_P(ShaderStorageBufferTest31, UnsizedArrayLength)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout (local_size_x=1) in;
layout(std430, binding = 0) buffer Storage0 {
  uint buf1[2];
  uint buf2[];
} sb_load;
layout(std430, binding = 1) buffer Storage1 {
  int unsizedArrayLength;
  uint buf1[2];
  uint buf2[];
} sb_store;

void main()
{
  sb_store.unsizedArrayLength = sb_store.buf2.length();
  for (int i = 0; i < sb_load.buf1.length(); i++) {
    sb_store.buf1[i] = sb_load.buf1[i];
  }
  for (int i = 0; i < sb_load.buf2.length(); i++) {
    sb_store.buf2[i] = sb_load.buf2[i];
  }
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    glUseProgram(program);

    constexpr unsigned int kBytesPerComponent                       = sizeof(unsigned int);
    constexpr unsigned int kLoadBlockElementCount                   = 5;
    constexpr unsigned int kStoreBlockElementCount                  = 6;
    constexpr unsigned int kInputValues[kLoadBlockElementCount]     = {1u, 2u, 3u, 4u, 5u};
    constexpr unsigned int kExpectedValues[kStoreBlockElementCount] = {3u, 1u, 2u, 3u, 4u, 5u};
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kLoadBlockElementCount * kBytesPerComponent,
                 &kInputValues, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kStoreBlockElementCount * kBytesPerComponent, nullptr,
                 GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kStoreBlockElementCount * kBytesPerComponent,
                         GL_MAP_READ_BIT));
    for (unsigned int i = 0; i < kStoreBlockElementCount; i++)
    {
        EXPECT_EQ(kExpectedValues[i], *(ptr + i));
    }

    EXPECT_GL_NO_ERROR();
}

// Test back to back that the length of unsized array is correct after respecifying the buffer
// size to be smaller than the first
TEST_P(ShaderStorageBufferTest31, UnsizedArrayLengthRespecifySize)
{
    // http://anglebug.com/42263171
    ANGLE_SKIP_TEST_IF(IsD3D11() || (IsAndroid() && IsOpenGLES()));

    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout (local_size_x=1) in;
layout(std430, binding = 0) buffer Storage0 {
  uint buf1[2];
  uint buf2[];
} sb_load;
layout(std430, binding = 1) buffer Storage1 {
  int unsizedArrayLength;
  uint buf1[2];
  uint buf2[];
} sb_store;

void main()
{
  sb_store.unsizedArrayLength = sb_store.buf2.length();
  for (int i = 0; i < sb_load.buf1.length(); i++) {
    sb_store.buf1[i] = sb_load.buf1[i];
  }
  for (int i = 0; i < sb_load.buf2.length(); i++) {
    sb_store.buf2[i] = sb_load.buf2[i];
  }
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    glUseProgram(program);

    constexpr unsigned int kBytesPerComponent                       = sizeof(unsigned int);
    constexpr unsigned int kLoadBlockElementCount                   = 5;
    constexpr unsigned int kStoreBlockElementCount                  = 6;
    constexpr unsigned int kInputValues[kLoadBlockElementCount]     = {1u, 2u, 3u, 4u, 5u};
    constexpr unsigned int kExpectedValues[kStoreBlockElementCount] = {3u, 1u, 2u, 3u, 4u, 5u};
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kLoadBlockElementCount * kBytesPerComponent,
                 &kInputValues, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kStoreBlockElementCount * kBytesPerComponent, nullptr,
                 GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kStoreBlockElementCount * kBytesPerComponent,
                         GL_MAP_READ_BIT));
    for (unsigned int i = 0; i < kStoreBlockElementCount; i++)
    {
        EXPECT_EQ(kExpectedValues[i], *(ptr + i));
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    EXPECT_GL_NO_ERROR();

    // Respecify these SSBOs to be smaller
    constexpr unsigned int kSmallerLoadBlockElementCount                          = 3;
    constexpr unsigned int kSmallerStoreBlockElementCount                         = 4;
    constexpr unsigned int kSmallerInputValues[kSmallerLoadBlockElementCount]     = {1u, 2u, 3u};
    constexpr unsigned int kSmallerExpectedValues[kSmallerStoreBlockElementCount] = {1u, 1u, 2u,
                                                                                     3u};

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kSmallerLoadBlockElementCount * kBytesPerComponent,
                 &kSmallerInputValues, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kSmallerStoreBlockElementCount * kBytesPerComponent,
                 nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    const GLuint *ptr2 = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                         kSmallerStoreBlockElementCount * kBytesPerComponent, GL_MAP_READ_BIT));
    for (unsigned int i = 0; i < kSmallerStoreBlockElementCount; i++)
    {
        EXPECT_EQ(kSmallerExpectedValues[i], *(ptr2 + i));
    }

    EXPECT_GL_NO_ERROR();
}

// Test that compond assignment operator for buffer variable is correctly handled.
TEST_P(ShaderStorageBufferTest31, CompoundAssignmentOperator)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout (local_size_x=1) in;
layout(binding=0, std140) buffer Storage0
{
    uint b;
} sb_load;
layout(binding=1, std140) buffer Storage1
{
    uint b;
} sb_store;
void main()
{
    uint temp = 2u;
    temp += sb_load.b;
    sb_store.b += temp;
    sb_store.b += sb_load.b;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    glUseProgram(program);

    constexpr unsigned int kBytesPerComponent = sizeof(unsigned int);
    constexpr unsigned int kInputValue        = 1u;
    constexpr unsigned int kExpectedValue     = 5u;
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBytesPerComponent, &kInputValue, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBytesPerComponent, &kInputValue, GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBytesPerComponent, GL_MAP_READ_BIT));
    EXPECT_EQ(kExpectedValue, *ptr);

    EXPECT_GL_NO_ERROR();
}

// Test that BufferData change propagate to context state.
TEST_P(ShaderStorageBufferTest31, DependentBufferChange)
{
    // Test fail on Nexus devices. http://anglebug.com/42264770
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsOpenGLES());

    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout (local_size_x=1) in;
layout(binding=0, std140) buffer Storage0
{
    uint b;
} sb_load;
layout(binding=1, std140) buffer Storage1
{
    uint b;
} sb_store;
void main()
{
    sb_store.b += sb_load.b;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    glUseProgram(program);

    constexpr unsigned int kBufferSize                        = 4096;
    constexpr unsigned int kBufferElementCount                = kBufferSize / sizeof(unsigned int);
    std::array<unsigned int, kBufferElementCount> kBufferData = {};
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBufferSize, kBufferData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    kBufferData[0] = 5;  // initial value
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBufferSize, kBufferData.data(), GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    kBufferData[0] = 7;
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBufferSize, kBufferData.data(), GL_STATIC_DRAW);
    glDispatchCompute(1, 1, 1);
    kBufferData[0] = 11;
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBufferSize, kBufferData.data(), GL_STATIC_DRAW);
    glDispatchCompute(1, 1, 1);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, GL_MAP_READ_BIT));
    constexpr unsigned int kExpectedValue = 5 + 7 + 11;
    EXPECT_EQ(kExpectedValue, *ptr);

    EXPECT_GL_NO_ERROR();
}

// Test that readonly binary operator for buffer variable is correctly handled.
TEST_P(ShaderStorageBufferTest31, ReadonlyBinaryOperator)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
 layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
 layout(std430, binding = 0) buffer blockIn1 {
     uvec2 data1;
 };
 layout(std430, binding = 1) buffer blockIn2 {
     uvec2 data2;
 };
 layout(std430, binding = 2) buffer blockIn3 {
     uvec2 data;
 } instanceIn3;
 layout(std430, binding = 3) buffer blockOut {
     uvec2 data;
 } instanceOut;
 void main()
 {
     instanceOut.data = data1 + data2 + instanceIn3.data;
 }
 )";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    glUseProgram(program);

    constexpr unsigned int kComponentCount                = 2;
    constexpr unsigned int kBytesPerComponent             = sizeof(unsigned int);
    constexpr unsigned int kInputValues1[kComponentCount] = {1u, 2u};
    constexpr unsigned int kInputValues2[kComponentCount] = {3u, 4u};
    constexpr unsigned int kInputValues3[kComponentCount] = {5u, 6u};
    // Create shader storage buffer
    GLBuffer shaderStorageBuffer[4];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kComponentCount * kBytesPerComponent, kInputValues1,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kComponentCount * kBytesPerComponent, kInputValues2,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[2]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kComponentCount * kBytesPerComponent, kInputValues3,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[3]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kComponentCount * kBytesPerComponent, nullptr,
                 GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, shaderStorageBuffer[2]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, shaderStorageBuffer[3]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    // Read back shader storage buffer
    constexpr unsigned int kExpectedValues[kComponentCount] = {9u, 12u};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[3]);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, kComponentCount * kBytesPerComponent, GL_MAP_READ_BIT));
    for (unsigned int idx = 0; idx < kComponentCount; idx++)
    {
        EXPECT_EQ(kExpectedValues[idx], *(ptr + idx));
    }

    EXPECT_GL_NO_ERROR();
}

// Test that ssbo as an argument of a function can be translated.
TEST_P(ShaderStorageBufferTest31, SSBOAsFunctionArgument)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x = 1) in;

layout(std430, binding = 0) buffer Block
{
    uint var1;
    uint var2;
};

bool compare(uint a, uint b)
{
    return a == b;
}

uint increase(inout uint a)
{
    a++;
    return a;
}

void main(void)
{
    bool isEqual = compare(var1, 2u);
    if (isEqual)
    {
        var2 += increase(var1);
    }
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    glUseProgram(program);

    constexpr unsigned int kBytesPerComponent = sizeof(unsigned int);
    constexpr unsigned int kInputValues[2]    = {2u, 2u};
    constexpr unsigned int kExpectedValues[2] = {3u, 5u};
    GLBuffer shaderStorageBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * kBytesPerComponent, &kInputValues, GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer);

    glDispatchCompute(1, 1, 1);
    glFinish();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 2 * kBytesPerComponent, GL_MAP_READ_BIT));
    EXPECT_EQ(kExpectedValues[0], *ptr);
    EXPECT_EQ(kExpectedValues[1], *(ptr + 1));

    EXPECT_GL_NO_ERROR();
}

// Test that ssbo as unary operand works well.
TEST_P(ShaderStorageBufferTest31, SSBOAsUnaryOperand)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout (local_size_x=1) in;
layout(binding=0, std140) buffer Storage0
{
    uint b;
} sb_load;
layout(binding=1, std140) buffer Storage1
{
    uint i;
} sb_store;
void main()
{
    sb_store.i = +sb_load.b;
    ++sb_store.i;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    glUseProgram(program);

    constexpr unsigned int kBytesPerComponent = sizeof(unsigned int);
    constexpr unsigned kInputValue            = 1u;
    constexpr unsigned int kExpectedValue     = 2u;
    GLBuffer shaderStorageBuffer[2];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBytesPerComponent, &kInputValue, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBytesPerComponent, &kInputValue, GL_STATIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    glFinish();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBytesPerComponent, GL_MAP_READ_BIT));
    EXPECT_EQ(kExpectedValue, *ptr);

    EXPECT_GL_NO_ERROR();
}

// Test that uniform can be used as the index of buffer variable.
TEST_P(ShaderStorageBufferTest31, UniformUsedAsIndexOfBufferVariable)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout (local_size_x=4) in;
layout(std140, binding = 0) uniform CB
{
    uint index;
} cb;

layout(binding=0, std140) buffer Storage0
{
    uint data[];
} sb_load;
layout(binding=1, std140) buffer Storage1
{
    uint data[];
} sb_store;
void main()
{
    sb_store.data[gl_LocalInvocationIndex] = sb_load.data[gl_LocalInvocationID.x + cb.index];
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();
}

// Test that inactive but statically used SSBOs with unsized array are handled correctly.
//
// Glslang wrapper used to replace the layout/qualifier of an inactive SSBO with |struct|,
// effectively turning the interface block declaration into a struct definition.  This generally
// worked except for SSBOs with an unsized array.  This test makes sure this special case is
// now properly handled.
TEST_P(ShaderStorageBufferTest31, InactiveButStaticallyUsedWithUnsizedArray)
{
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout (local_size_x=1) in;
layout(binding=0, std140) buffer Storage
{
    uint data[];
} sb;
void main()
{
    if (false)
    {
        sb.data[0] = 1u;
    }
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();
}

// Verify the size of the buffer with unsized struct array is calculated correctly
TEST_P(ShaderStorageBufferTest31, BigStructUnsizedStructArraySize)
{
    // TODO(http://anglebug.com/42262259)
    ANGLE_SKIP_TEST_IF(IsAMD() && IsWindows() && IsOpenGL());

    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout (local_size_x=1) in;

struct S
{
    mat4 m;     // 4 vec4 = 16 floats
    vec4 a[10]; // 10 vec4 = 40 floats
};

layout(binding=0) buffer B
{
    vec4 precedingMember;               // 4 floats
    S precedingMemberUnsizedArray[];    // 56 floats
} b;

void main()
{
    if (false)
    {
        b.precedingMember = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    GLuint resourceIndex = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, "B");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(resourceIndex, 0xFFFFFFFF);

    GLenum property = GL_BUFFER_DATA_SIZE;
    GLint queryData = -1;
    glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, resourceIndex, 1, &property, 1,
                           nullptr, &queryData);
    EXPECT_GL_NO_ERROR();

    // 60 * sizeof(float) = 240
    // Vulkan rounds up to the required buffer alignment, so >= 240
    EXPECT_GE(queryData, 240);
}

// Verify the size of the buffer with unsized float array is calculated correctly
TEST_P(ShaderStorageBufferTest31, BigStructUnsizedFloatArraySize)
{
    // TODO(http://anglebug.com/42262259)
    ANGLE_SKIP_TEST_IF(IsAMD() && IsWindows() && IsOpenGL());

    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout (local_size_x=1) in;

layout(binding=0) buffer B
{
    vec4 precedingMember;                   // 4 floats
    float precedingMemberUnsizedArray[];    // "1" float
} b;

void main()
{
    if (false)
    {
        b.precedingMember = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    GLuint resourceIndex = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, "B");
    EXPECT_GL_NO_ERROR();
    EXPECT_NE(resourceIndex, 0xFFFFFFFF);

    GLenum property = GL_BUFFER_DATA_SIZE;
    GLint queryData = -1;
    glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, resourceIndex, 1, &property, 1,
                           nullptr, &queryData);
    EXPECT_GL_NO_ERROR();

    // 5 * sizeof(float) = 20
    // Vulkan rounds up to the required buffer alignment, so >= 20
    EXPECT_GE(queryData, 20);
}

// Tests that shader write before pixel pack/unpack works
TEST_P(ShaderStorageBufferTest31, ShaderStorageBufferWriteThenPixelPackUnpack)
{
    // Create two textures and framebuffers and make sure they are initialized.
    GLTexture color1;
    glBindTexture(GL_TEXTURE_2D, color1);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);

    GLFramebuffer framebuffer1;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color1, 0);

    glClearColor(255, 0, 255, 255);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);

    GLTexture color2;
    glBindTexture(GL_TEXTURE_2D, color2);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);

    GLFramebuffer framebuffer2;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color2, 0);

    glClearColor(0, 0, 255, 255);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std430, binding = 0) buffer block {
    uint data[2];
} outBlock;
void main()
{
    // Output red to index 0, and green to index 1.
    outBlock.data[0] = 0xFF0000FFu;
    outBlock.data[1] = 0xFF00FF00u;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);

    glUseProgram(program);

    constexpr GLsizei kBufferSize                   = sizeof(GLuint) * 2;
    constexpr std::array<GLuint, 2> kBufferInitData = {0x01234567u, 0x89ABCDEFu};

    // Create a shader storage buffer
    GLBuffer buffer;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBufferSize, kBufferInitData.data(), GL_STATIC_DRAW);

    // Bind shader storage buffer and write to it.
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Issue a memory barrier for pixel pack/unpack operations.
    glMemoryBarrier(GL_PIXEL_BUFFER_BARRIER_BIT);
    EXPECT_GL_NO_ERROR();

    // Use a pixel pack operation to overwrite the output of the compute shader at index 0.  Uses
    // the second framebuffer which is blue.
    glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    EXPECT_GL_NO_ERROR();

    // Use a pixel unpack operation to re-initialize the other framebuffer with the results from the
    // compute shader.
    glBindTexture(GL_TEXTURE_2D, color1);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    reinterpret_cast<void *>(sizeof(GLuint)));
    EXPECT_GL_NO_ERROR();

    // Verify that the first framebuffer is now green
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Verify the contents of the buffer.  It should have blue as the first index and green as the
    // second.
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    GLColor *bufferContents = static_cast<GLColor *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, GL_MAP_READ_BIT));
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GLColor::blue, bufferContents[0]);
    EXPECT_EQ(GLColor::green, bufferContents[1]);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Tests that shader write after pixel pack/unpack works
TEST_P(ShaderStorageBufferTest31, PixelPackUnpackThenShaderStorageBufferWrite)
{
    swapBuffers();
    // Create two textures and framebuffers and make sure they are initialized.
    GLTexture color1;
    glBindTexture(GL_TEXTURE_2D, color1);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);

    GLFramebuffer framebuffer1;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color1, 0);

    glClearColor(255, 0, 255, 255);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);

    GLTexture color2;
    glBindTexture(GL_TEXTURE_2D, color2);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);

    GLFramebuffer framebuffer2;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color2, 0);

    glClearColor(0, 0, 255, 255);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    constexpr GLsizei kBufferSize                   = sizeof(GLuint) * 2;
    constexpr std::array<GLuint, 2> kBufferInitData = {0x01234567u, 0xFF00FF00u};

    // Create a shader storage buffer
    GLBuffer buffer;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBufferSize, kBufferInitData.data(), GL_STATIC_DRAW);

    // Use a pixel pack operation to overwrite the buffer at index 0.  Uses the second framebuffer
    // which is blue.
    glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    EXPECT_GL_NO_ERROR();

    // Use a pixel unpack operation to re-initialize the other framebuffer with the contents of the
    // buffer, which is green.
    glBindTexture(GL_TEXTURE_2D, color1);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    reinterpret_cast<void *>(sizeof(GLuint)));
    EXPECT_GL_NO_ERROR();

    // Issue a memory barrier for pixel pack/unpack operations.
    glMemoryBarrier(GL_PIXEL_BUFFER_BARRIER_BIT);

    // Issue a dispatch call that overwrites the buffer, also verifying that the results of the pack
    // operation is correct.
    constexpr char kComputeShaderSource[] =
        R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std430, binding = 0) buffer block {
    uint data[2];
} outBlock;
void main()
{
    if (outBlock.data[0] == 0xFFFF0000u)
    {
        outBlock.data[0] = 0x11223344u;
    }
    else
    {
        outBlock.data[0] = 0xABCDABCDu;
    }
    outBlock.data[1] = 0x55667788u;
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShaderSource);

    glUseProgram(program);

    // Bind shader storage buffer and write to it.
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Verify that the first framebuffer is now green
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer1);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Verify that the second framebuffer is still blue
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer2);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Verify the contents of the buffer.
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    uint32_t *bufferContents = static_cast<uint32_t *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, GL_MAP_READ_BIT));
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(bufferContents[0], 0x11223344u);
    EXPECT_EQ(bufferContents[1], 0x55667788u);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    EXPECT_GL_NO_ERROR();
}

// Tests that reading from an SSBO inside a function in the fragment shader works.
TEST_P(ShaderStorageBufferTest31, FragReadSSBOInFunction)
{
    GLint maxFragmentShaderStorageBlocks;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);

    ANGLE_SKIP_TEST_IF(maxFragmentShaderStorageBlocks < 1);

    constexpr std::array<float, 4> kBufferInitValue = {1.0f, 0.0f, 1.0f, 1.0f};
    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kBufferInitValue), kBufferInitValue.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);

    // Create a program that reads from the SSBO in the fragment shader.
    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

out vec4 colorOut;

layout(binding = 0, std430) buffer block {
  vec4 color;
} instance;

vec4 func() {
  vec4 temp = instance.color;
  return temp;
}

void main() {
  colorOut = func();
})";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, essl31_shaders::vs::Simple());
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    CheckLinkStatusAndReturnProgram(program, true);

    glUseProgram(program);
    GLint positionLoc = glGetAttribLocation(program, essl31_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLoc);

    const std::array<Vector3, 6> &quadVertices = GetQuadVertices();
    const size_t posBufferSize                 = quadVertices.size() * sizeof(Vector3);

    GLBuffer posBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
    glBufferData(GL_ARRAY_BUFFER, posBufferSize, quadVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLoc);

    glUseProgram(program);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
}

// Tests reading from an SSBO in a function. In this case, we also modify SSBO to check the result.
TEST_P(ShaderStorageBufferTest31, ReadSSBOInFunction)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;

layout(std430, binding = 0) buffer block {
    uint data;
} instance;

uint func()
{
    return instance.data;
}
void main() {
    instance.data = func() + 1u;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    glUseProgram(program);

    constexpr unsigned int kBytesPerComponent = sizeof(GLuint);
    constexpr unsigned int kInitialData       = 123u;

    // Create shader storage buffer
    GLBuffer shaderStorageBuffer;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBytesPerComponent, &kInitialData, GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    const void *bufferData =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBytesPerComponent, GL_MAP_READ_BIT);

    constexpr unsigned int kExpectedData = 124u;
    EXPECT_EQ(kExpectedData, *static_cast<const GLuint *>(bufferData));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    EXPECT_GL_NO_ERROR();
}

// Tests whole-array assignment to an SSBO.
TEST_P(ShaderStorageBufferTest31, AggregateArrayAssignmentSSBO)
{
    constexpr char kCS[] = R"(#version 310 es

layout(std430, binding = 0) buffer block {
  int a[4];
} instance;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  instance.a = int[4](0, 1, 2, 3);
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    glUseProgram(program);

    constexpr unsigned int kBytesPerComponent = sizeof(GLuint);
    constexpr unsigned int kElementCount      = 4;

    // Create shader storage buffer
    GLBuffer shaderStorageBuffer;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kElementCount * kBytesPerComponent, nullptr,
                 GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    const void *bufferData = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                                              kElementCount * kBytesPerComponent, GL_MAP_READ_BIT);

    for (unsigned int idx = 0; idx < kElementCount; idx++)
    {
        EXPECT_EQ(static_cast<const GLuint *>(bufferData)[idx], idx);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    EXPECT_GL_NO_ERROR();
}

// Tests whole-struct assignment to an SSBO.
TEST_P(ShaderStorageBufferTest31, AggregateStructAssignmentSSBO)
{
    constexpr char kCS[] = R"(#version 310 es

struct StructValues {
    float f;
    int i;
};


layout(std430, binding = 0) buffer block {
    StructValues s;
} instance;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
    instance.s = StructValues(123.0f, 321);
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    glUseProgram(program);

    constexpr size_t kSize = sizeof(GLfloat) + sizeof(GLint);

    // Create shader storage buffer
    GLBuffer shaderStorageBuffer;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kSize, nullptr, GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    const void *bufferData = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kSize, GL_MAP_READ_BIT);

    EXPECT_EQ(static_cast<const GLfloat *>(bufferData)[0], 123.0f);
    EXPECT_EQ(static_cast<const GLint *>(bufferData)[1], 321);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    EXPECT_GL_NO_ERROR();
}

// Test that certain floating-point values are unchanged after constant folding.
TEST_P(ShaderStorageBufferTest31, ConstantFoldingPrecision)
{
    constexpr char kCS[] = R"(#version 310 es

layout(std430, binding = 0) buffer block {
    float f;
} instance;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
    instance.f = intBitsToFloat(0x0da5cc2f);
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    glUseProgram(program);

    constexpr size_t kSize = sizeof(GLfloat);

    // Create shader storage buffer
    GLBuffer shaderStorageBuffer;

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kSize, nullptr, GL_STATIC_DRAW);

    // Bind shader storage buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer);

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    const void *bufferData = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kSize, GL_MAP_READ_BIT);

    int32_t result = static_cast<const int32_t *>(bufferData)[0];

    // Compare against the int32_t representation of the float passed to
    // intBitsToFloat() in the shader.
    EXPECT_EQ(result, 0x0da5cc2f);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    EXPECT_GL_NO_ERROR();
}

// Tests two SSBOs in the fragment shader.
TEST_P(ShaderStorageBufferTest31, TwoSSBOsInFragmentShader)
{
    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

layout(binding = 0, std430) buffer block0 {
    float value;
} result0;

layout(binding = 1, std430) buffer block1 {
    float value;
} result1;

void main() {
    result0.value = 0.5f;
    result1.value = 42.0f;
}
)";

    constexpr size_t kSize[2] = {sizeof(GLfloat), sizeof(GLfloat)};
    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);

    // Create shader storage buffers
    GLBuffer shaderStorageBuffer[2];

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kSize[0], nullptr, GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kSize[1], nullptr, GL_STATIC_DRAW);

    // Bind shader storage buffers
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.95f);
    ASSERT_GL_NO_ERROR();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    const void *buffer0Data =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kSize[0], GL_MAP_READ_BIT);
    EXPECT_EQ(static_cast<const GLfloat *>(buffer0Data)[0], 0.5f);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    const void *buffer1Data =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kSize[1], GL_MAP_READ_BIT);
    EXPECT_EQ(static_cast<const GLfloat *>(buffer1Data)[0], 42.0f);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    EXPECT_GL_NO_ERROR();
}

// Tests two SSBOs in a compute shader.
TEST_P(ShaderStorageBufferTest31, TwoSSBOsInComputeShader)
{
    constexpr char kCS[] = R"(#version 310 es

layout(local_size_x=1, local_size_y=1, local_size_z=1) in;

layout(binding = 0, std430) buffer block0 {
    float value;
} result0;

layout(binding = 1, std430) buffer block1 {
    float value;
} result1;

void main() {
    result0.value = 0.5f;
    result1.value = 42.0f;
}
)";

    constexpr size_t kSize[2] = {sizeof(GLfloat), sizeof(GLfloat)};
    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    // Create shader storage buffers
    GLBuffer shaderStorageBuffer[2];

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kSize[0], nullptr, GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kSize[1], nullptr, GL_STATIC_DRAW);

    // Bind shader storage buffers
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shaderStorageBuffer[0]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer[1]);

    glDispatchCompute(1, 1, 1);
    ASSERT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[0]);
    const void *buffer0Data =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kSize[0], GL_MAP_READ_BIT);
    EXPECT_EQ(static_cast<const GLfloat *>(buffer0Data)[0], 0.5f);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer[1]);
    const void *buffer1Data =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kSize[1], GL_MAP_READ_BIT);
    EXPECT_EQ(static_cast<const GLfloat *>(buffer1Data)[0], 42.0f);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    EXPECT_GL_NO_ERROR();
}

// Test that buffer self-copy works when buffer is used as SSBO
TEST_P(ShaderStorageBufferTest31, CopyBufferSubDataSelfDependency)
{
    constexpr char kCS[] = R"(#version 310 es

layout(local_size_x=1, local_size_y=1, local_size_z=1) in;

layout(binding = 0, std430) buffer SSBO
{
    uvec4 data[128];
};

void main()
{
    data[12] += uvec4(1);
    data[12+64] += uvec4(10);
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    constexpr uint32_t kUVec4Size   = 4 * sizeof(uint32_t);
    constexpr uint32_t kSSBOSize    = 128 * kUVec4Size;
    constexpr uint32_t kData1Offset = 12 * kUVec4Size;
    constexpr uint32_t kData2Offset = 76 * kUVec4Size;

    // Init data is 4 times the size of SSBO as the buffer is created larger than the SSBO
    // throughout the test.
    constexpr uint32_t kInitValue = 12345;
    const std::vector<uint32_t> kInitData(kSSBOSize, kInitValue);

    // Set up a throw-away buffer just to make buffer suballocations not use offset 0.
    GLBuffer throwaway;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, throwaway);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 1024, nullptr, GL_DYNAMIC_DRAW);

    // Set up the buffer
    GLBuffer buffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kSSBOSize * 2, kInitData.data(), GL_DYNAMIC_DRAW);

    // Bind at offset 0.  After the dispatch call: [12] is +1, [76] is +10
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffer, 0, kSSBOSize);
    glDispatchCompute(1, 1, 1);

    // Duplicate the buffer in the second half.  Barrier needed for glCopyBufferSubData.  After the
    // copy: [128+12] is +1, [128+76] is +10
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    glCopyBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_SHADER_STORAGE_BUFFER, 0, kSSBOSize,
                        kSSBOSize);

    // Barrier needed before writing to the buffer in the shader.
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Bind at offset 128.  After the dispatch call: [128+12] is +2, [128+76] is +20
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffer, kSSBOSize, kSSBOSize);
    glDispatchCompute(1, 1, 1);

    // Do a small self-copy.  Barrier needed for glCopyBufferSubData.
    // After the copy: [64+12] = [128+12] (i.e. +2)
    constexpr uint32_t kCopySrcOffset = (128 + 4) * kUVec4Size;
    constexpr uint32_t kCopyDstOffset = (64 + 4) * kUVec4Size;
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    glCopyBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_SHADER_STORAGE_BUFFER, kCopySrcOffset,
                        kCopyDstOffset, kData1Offset);

    // Barrier needed before writing to the buffer in the shader.
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Bind at offset 64.  After the dispatch call: [64+12] is +3, [64+76] (i.e. [128+12]) is +12
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffer, kCopySrcOffset - kCopyDstOffset,
                      kSSBOSize);
    glDispatchCompute(1, 1, 1);

    // Validate results.  Barrier needed for glMapBufferRange
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    std::vector<uint32_t> result(kSSBOSize / 2);
    const void *bufferData =
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kSSBOSize * 2, GL_MAP_READ_BIT);
    memcpy(result.data(), bufferData, kSSBOSize * 2);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    for (size_t index = 0; index < kSSBOSize * 2 / kUVec4Size; ++index)
    {
        size_t offset      = index * 4;
        uint32_t increment = 0;
        if (index == kData1Offset / kUVec4Size)
        {
            increment = 1;
        }
        else if (index == kData2Offset / kUVec4Size)
        {
            increment = 3;
        }
        else if (index == (kSSBOSize + kData1Offset) / kUVec4Size)
        {
            increment = 12;
        }
        else if (index == (kSSBOSize + kData2Offset) / kUVec4Size)
        {
            increment = 20;
        }

        for (size_t component = 0; component < 4; ++component)
        {
            EXPECT_EQ(result[offset + component], kInitValue + increment)
                << component << " " << index << " " << increment;
        }
    }

    // Do a big copy again, but this time the buffer is unused by the GPU.
    // After this call: [12] is +12, [76] is +20
    glCopyBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_SHADER_STORAGE_BUFFER, kSSBOSize, 0,
                        kSSBOSize);

    // Barrier needed before writing to the buffer in the shader.
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Bind at offset 0.  After the dispatch call: [12] is +13, [76] is +30
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffer, 0, kSSBOSize);
    glDispatchCompute(1, 1, 1);

    // Validate results.  Barrier needed for glMapBufferRange
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    bufferData = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kSSBOSize * 2, GL_MAP_READ_BIT);
    memcpy(result.data(), bufferData, kSSBOSize * 2);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    for (size_t index = 0; index < kSSBOSize * 2 / kUVec4Size; ++index)
    {
        size_t offset      = index * 4;
        uint32_t increment = 0;
        if (index == kData1Offset / kUVec4Size)
        {
            increment = 13;
        }
        else if (index == kData2Offset / kUVec4Size)
        {
            increment = 30;
        }
        else if (index == (kSSBOSize + kData1Offset) / kUVec4Size)
        {
            increment = 12;
        }
        else if (index == (kSSBOSize + kData2Offset) / kUVec4Size)
        {
            increment = 20;
        }

        for (size_t component = 0; component < 4; ++component)
        {
            EXPECT_EQ(result[offset + component], kInitValue + increment)
                << component << " " << index << " " << increment;
        }
    }
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ShaderStorageBufferTest31);
ANGLE_INSTANTIATE_TEST_ES31_AND(ShaderStorageBufferTest31,
                                ES31_VULKAN().enable(Feature::PreferCPUForBufferSubData));

}  // namespace
