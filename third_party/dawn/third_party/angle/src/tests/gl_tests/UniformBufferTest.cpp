//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/random_utils.h"

using namespace angle;

namespace
{

class UniformBufferTest : public ANGLETest<>
{
  protected:
    UniformBufferTest()
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
        mkFS = R"(#version 300 es
precision highp float;
uniform uni { vec4 color; };
out vec4 fragColor;
void main()
{
    fragColor = color;
})";

        mProgram = CompileProgram(essl3_shaders::vs::Simple(), mkFS);
        ASSERT_NE(mProgram, 0u);

        mUniformBufferIndex = glGetUniformBlockIndex(mProgram, "uni");
        ASSERT_NE(mUniformBufferIndex, -1);

        glGenBuffers(1, &mUniformBuffer);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteBuffers(1, &mUniformBuffer);
        glDeleteProgram(mProgram);
    }

    const char *mkFS;
    GLuint mProgram;
    GLint mUniformBufferIndex;
    GLuint mUniformBuffer;
};

// Basic UBO functionality.
TEST_P(UniformBufferTest, Simple)
{
    glClear(GL_COLOR_BUFFER_BIT);
    float floatData[4] = {0.5f, 0.75f, 0.25f, 1.0f};

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4, floatData, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);

    glUniformBlockBinding(mProgram, mUniformBufferIndex, 0);
    drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 128, 191, 64, 255, 1);
}

// Test a scenario that draws then update UBO (using bufferData or bufferSubData or mapBuffer) then
// draws with updated data.
TEST_P(UniformBufferTest, DrawThenUpdateThenDraw)
{
    constexpr char kVS[] = R"(#version 300 es
precision highp float;

void main()
{
    vec2 position = vec2(float(gl_VertexID >> 1), float(gl_VertexID & 1));
    position = 2.0 * position - 1.0;
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
})";

    enum class BufferUpdateMethod
    {
        BUFFER_DATA,
        BUFFER_SUB_DATA,
        MAP_BUFFER,
    };

    ANGLE_GL_PROGRAM(program, kVS, mkFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "uni");
    ASSERT_NE(uniformBufferIndex, -1);

    for (BufferUpdateMethod method :
         {BufferUpdateMethod::BUFFER_DATA, BufferUpdateMethod::BUFFER_SUB_DATA,
          BufferUpdateMethod::MAP_BUFFER})
    {
        glClear(GL_COLOR_BUFFER_BIT);
        float floatData1[4] = {0.25f, 0.75f, 0.125f, 1.0f};

        GLBuffer uniformBuffer;
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4, floatData1, GL_DYNAMIC_DRAW);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glUniformBlockBinding(program, uniformBufferIndex, 0);
        glUseProgram(program);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        float floatData2[4] = {0.25f, 0.0f, 0.125f, 0.0f};
        switch (method)
        {
            case BufferUpdateMethod::BUFFER_DATA:
                glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4, floatData2, GL_DYNAMIC_DRAW);
                break;
            case BufferUpdateMethod::BUFFER_SUB_DATA:
                glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 4, floatData2);
                break;
            case BufferUpdateMethod::MAP_BUFFER:
                void *mappedBuffer =
                    glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(float) * 4, GL_MAP_WRITE_BIT);
                memcpy(mappedBuffer, floatData2, sizeof(floatData2));
                glUnmapBuffer(GL_UNIFORM_BUFFER);
                break;
        }
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_NEAR(0, 0, 128, 191, 64, 255, 1);
    }
}

// Test that using a UBO with a non-zero offset and size actually works.
// The first step of this test renders a color from a UBO with a zero offset.
// The second step renders a color from a UBO with a non-zero offset.
TEST_P(UniformBufferTest, UniformBufferRange)
{
    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    // Query the uniform buffer alignment requirement
    GLint alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);

    GLint64 maxUniformBlockSize;
    glGetInteger64v(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    if (alignment >= maxUniformBlockSize)
    {
        // ANGLE doesn't implement UBO offsets for this platform.
        // Ignore the test case.
        return;
    }

    ASSERT_GL_NO_ERROR();

    // Let's create a buffer which contains two vec4.
    GLuint vec4Size = 4 * sizeof(float);
    GLuint stride   = 0;
    do
    {
        stride += alignment;
    } while (stride < vec4Size);

    std::vector<char> v(2 * stride);
    float *first  = reinterpret_cast<float *>(v.data());
    float *second = reinterpret_cast<float *>(v.data() + stride);

    first[0] = 10.f / 255.f;
    first[1] = 20.f / 255.f;
    first[2] = 30.f / 255.f;
    first[3] = 40.f / 255.f;

    second[0] = 110.f / 255.f;
    second[1] = 120.f / 255.f;
    second[2] = 130.f / 255.f;
    second[3] = 140.f / 255.f;

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    // We use on purpose a size which is not a multiple of the alignment.
    glBufferData(GL_UNIFORM_BUFFER, stride + vec4Size, v.data(), GL_STATIC_DRAW);

    glUniformBlockBinding(mProgram, mUniformBufferIndex, 0);

    EXPECT_GL_NO_ERROR();

    // Bind the first part of the uniform buffer and draw
    // Use a size which is smaller than the alignment to check
    // to check that this case is handle correctly in the conversion to 11.1.
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUniformBuffer, 0, vec4Size);
    drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(px, py, 10, 20, 30, 40);

    // Bind the second part of the uniform buffer and draw
    // Furthermore the D3D11.1 backend will internally round the vec4Size (16 bytes) to a stride
    // (256 bytes) hence it will try to map the range [stride, 2 * stride] which is out-of-bound of
    // the buffer bufferSize = stride + vec4Size < 2 * stride. Ensure that this behaviour works.
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUniformBuffer, stride, vec4Size);
    drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(px, py, 110, 120, 130, 140);
}

// Test uniform block bindings.
TEST_P(UniformBufferTest, UniformBufferBindings)
{
    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    ASSERT_GL_NO_ERROR();

    // Let's create a buffer which contains one vec4.
    GLuint vec4Size = 4 * sizeof(float);
    std::vector<char> v(vec4Size);
    float *first = reinterpret_cast<float *>(v.data());

    first[0] = 10.f / 255.f;
    first[1] = 20.f / 255.f;
    first[2] = 30.f / 255.f;
    first[3] = 40.f / 255.f;

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, vec4Size, v.data(), GL_STATIC_DRAW);

    EXPECT_GL_NO_ERROR();

    // Try to bind the buffer to binding point 2
    glUniformBlockBinding(mProgram, mUniformBufferIndex, 2);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, mUniformBuffer);
    drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(px, py, 10, 20, 30, 40);

    // Clear the framebuffer
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(px, py, 0, 0, 0, 0);

    // Try to bind the buffer to another binding point
    glUniformBlockBinding(mProgram, mUniformBufferIndex, 5);
    glBindBufferBase(GL_UNIFORM_BUFFER, 5, mUniformBuffer);
    drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(px, py, 10, 20, 30, 40);
}

// Test when the only change between draw calls is the change in the uniform binding range.
TEST_P(UniformBufferTest, BufferBindingRangeChange)
{
    constexpr GLsizei kVec4Size = 4 * sizeof(float);

    GLint alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
    if (alignment < kVec4Size)
    {
        alignment = kVec4Size;
    }
    ASSERT_EQ(alignment % 4, 0);

    // Put two colors in the uniform buffer, the sum of which is yellow.
    // Note: |alignment| is in bytes, so we can place each uniform in |alignment/4| floats.
    std::vector<float> colors(alignment / 2);
    // Half red
    colors[0] = 0.55;
    colors[1] = 0.0;
    colors[2] = 0.0;
    colors[3] = 0.35;
    // Greenish yellow
    colors[alignment / 4 + 0] = 0.55;
    colors[alignment / 4 + 1] = 1.0;
    colors[alignment / 4 + 2] = 0.0;
    colors[alignment / 4 + 3] = 0.75;

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, alignment * 2, colors.data(), GL_STATIC_DRAW);

    const GLint positionLoc = glGetAttribLocation(mProgram, essl3_shaders::PositionAttrib());
    setupQuadVertexBuffer(0.5f, 1.0f);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw twice, binding the uniform buffer to a different range each time
    glUseProgram(mProgram);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUniformBuffer, 0, kVec4Size);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUniformBuffer, alignment, kVec4Size);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::yellow);
}

// Test when glUniformBlockBinding is called between draws while the program is not current.
// Regression test for a missing dirty bit bug in this scenario.
TEST_P(UniformBufferTest, BufferBlockBindingChange)
{
    constexpr GLsizei kVec4Size = 4 * sizeof(float);

    GLint alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
    if (alignment < kVec4Size)
    {
        alignment = kVec4Size;
    }
    ASSERT_EQ(alignment % 4, 0);

    // Put two colors in the uniform buffer, the sum of which is yellow.
    // Note: |alignment| is in bytes, so we can place each uniform in |alignment/4| floats.
    std::vector<float> colors(alignment / 2);
    // Half red
    colors[0] = 0.55;
    colors[1] = 0.0;
    colors[2] = 0.0;
    colors[3] = 0.35;
    // Greenish yellow
    colors[alignment / 4 + 0] = 0.55;
    colors[alignment / 4 + 1] = 1.0;
    colors[alignment / 4 + 2] = 0.0;
    colors[alignment / 4 + 3] = 0.75;

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, alignment * 2, colors.data(), GL_STATIC_DRAW);

    const GLint positionLoc = glGetAttribLocation(mProgram, essl3_shaders::PositionAttrib());
    setupQuadVertexBuffer(0.5f, 1.0f);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw twice, binding the uniform buffer to a different range each time
    glUseProgram(mProgram);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUniformBuffer, 0, kVec4Size);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Change the block binding while the program is not current
    glUseProgram(0);
    glUniformBlockBinding(mProgram, 0, 1);
    glUseProgram(mProgram);

    glBindBufferRange(GL_UNIFORM_BUFFER, 1, mUniformBuffer, alignment, kVec4Size);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::yellow);
}

// Update a UBO many time and verify that ANGLE uses the latest version of the data.
// https://code.google.com/p/angleproject/issues/detail?id=965
TEST_P(UniformBufferTest, UniformBufferManyUpdates)
{
    // TODO(jmadill): Figure out why this fails on OSX Intel OpenGL.
    ANGLE_SKIP_TEST_IF(IsIntel() && IsMac() && IsOpenGL());

    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    ASSERT_GL_NO_ERROR();

    float data[4];

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(data), nullptr, GL_DYNAMIC_DRAW);
    glUniformBlockBinding(mProgram, mUniformBufferIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);

    EXPECT_GL_NO_ERROR();

    // Repeteadly update the data and draw
    for (size_t i = 0; i < 10; ++i)
    {
        data[0] = (i + 10.f) / 255.f;
        data[1] = (i + 20.f) / 255.f;
        data[2] = (i + 30.f) / 255.f;
        data[3] = (i + 40.f) / 255.f;

        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), data);

        drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);
        EXPECT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(px, py, i + 10, i + 20, i + 30, i + 40);
    }
}

// Use a large number of buffer ranges (compared to the actual size of the UBO)
TEST_P(UniformBufferTest, ManyUniformBufferRange)
{
    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    // Query the uniform buffer alignment requirement
    GLint alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);

    GLint64 maxUniformBlockSize;
    glGetInteger64v(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    if (alignment >= maxUniformBlockSize)
    {
        // ANGLE doesn't implement UBO offsets for this platform.
        // Ignore the test case.
        return;
    }

    ASSERT_GL_NO_ERROR();

    // Let's create a buffer which contains eight vec4.
    GLuint vec4Size = 4 * sizeof(float);
    GLuint stride   = 0;
    do
    {
        stride += alignment;
    } while (stride < vec4Size);

    std::vector<char> v(8 * stride);

    for (size_t i = 0; i < 8; ++i)
    {
        float *data = reinterpret_cast<float *>(v.data() + i * stride);

        data[0] = (i + 10.f) / 255.f;
        data[1] = (i + 20.f) / 255.f;
        data[2] = (i + 30.f) / 255.f;
        data[3] = (i + 40.f) / 255.f;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, v.size(), v.data(), GL_STATIC_DRAW);

    glUniformBlockBinding(mProgram, mUniformBufferIndex, 0);

    EXPECT_GL_NO_ERROR();

    // Bind each possible offset
    for (size_t i = 0; i < 8; ++i)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUniformBuffer, i * stride, stride);
        drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);
        EXPECT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(px, py, 10 + i, 20 + i, 30 + i, 40 + i);
    }

    // Try to bind larger range
    for (size_t i = 0; i < 7; ++i)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUniformBuffer, i * stride, 2 * stride);
        drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);
        EXPECT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(px, py, 10 + i, 20 + i, 30 + i, 40 + i);
    }

    // Try to bind even larger range
    for (size_t i = 0; i < 5; ++i)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUniformBuffer, i * stride, 4 * stride);
        drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);
        EXPECT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(px, py, 10 + i, 20 + i, 30 + i, 40 + i);
    }
}

// Tests that active uniforms have the right names.
TEST_P(UniformBufferTest, ActiveUniformNames)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec2 position;\n"
        "out vec2 v;\n"
        "uniform blockName1 {\n"
        "  float f1;\n"
        "} instanceName1;\n"
        "uniform blockName2 {\n"
        "  float f2;\n"
        "} instanceName2[1];\n"
        "void main() {\n"
        "  v = vec2(instanceName1.f1, instanceName2[0].f2);\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec2 v;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "  color = vec4(v, 0, 1);\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    GLint activeUniformBlocks;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &activeUniformBlocks);
    ASSERT_EQ(2, activeUniformBlocks);

    GLuint index = glGetUniformBlockIndex(program, "blockName1");
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();

    index = glGetUniformBlockIndex(program, "blockName2[0]");
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();

    GLint activeUniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms);

    ASSERT_EQ(2, activeUniforms);

    GLint size;
    GLenum type;
    GLint maxLength;
    GLsizei length;

    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);
    std::vector<GLchar> strUniformNameBuffer(maxLength + 1, 0);
    const GLchar *uniformNames[1];
    uniformNames[0] = "blockName1.f1";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strUniformNameBuffer[0]);
    EXPECT_EQ(1, size);
    EXPECT_GLENUM_EQ(GL_FLOAT, type);
    EXPECT_EQ("blockName1.f1", std::string(&strUniformNameBuffer[0]));

    uniformNames[0] = "blockName2.f2";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strUniformNameBuffer[0]);
    EXPECT_EQ(1, size);
    EXPECT_GLENUM_EQ(GL_FLOAT, type);
    EXPECT_EQ("blockName2.f2", std::string(&strUniformNameBuffer[0]));
}

// Tests active uniforms and blocks when the layout is std140, shared and packed.
TEST_P(UniformBufferTest, ActiveUniformNumberAndName)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec2 position;\n"
        "out float v;\n"
        "struct S {\n"
        "  highp ivec3 a;\n"
        "  mediump ivec2 b[4];\n"
        "};\n"
        "layout(std140) uniform blockName0 {\n"
        "  S s0;\n"
        "  lowp vec2 v0;\n"
        "  S s1[2];\n"
        "  highp uint u0;\n"
        "};\n"
        "layout(std140) uniform blockName1 {\n"
        "  float f1;\n"
        "  bool b1;\n"
        "} instanceName1;\n"
        "layout(shared) uniform blockName2 {\n"
        "  float f2;\n"
        "};\n"
        "layout(packed) uniform blockName3 {\n"
        "  float f3;\n"
        "};\n"
        "void main() {\n"
        "  v = instanceName1.f1;\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "in float v;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "  color = vec4(v, 0, 0, 1);\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    // Note that the packed |blockName3| might (or might not) be optimized out.
    GLint activeUniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms);
    EXPECT_GE(activeUniforms, 11);

    GLint activeUniformBlocks;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &activeUniformBlocks);
    EXPECT_GE(activeUniformBlocks, 3);

    GLint maxLength, size;
    GLenum type;
    GLsizei length;
    GLuint index;
    const GLchar *uniformNames[1];
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);
    std::vector<GLchar> strBuffer(maxLength + 1, 0);

    uniformNames[0] = "s0.a";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strBuffer[0]);
    EXPECT_EQ(1, size);
    EXPECT_EQ("s0.a", std::string(&strBuffer[0]));

    uniformNames[0] = "s0.b[0]";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strBuffer[0]);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(4, size);
    EXPECT_EQ("s0.b[0]", std::string(&strBuffer[0]));

    uniformNames[0] = "v0";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strBuffer[0]);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(1, size);
    EXPECT_EQ("v0", std::string(&strBuffer[0]));

    uniformNames[0] = "s1[0].a";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strBuffer[0]);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(1, size);
    EXPECT_EQ("s1[0].a", std::string(&strBuffer[0]));

    uniformNames[0] = "s1[0].b[0]";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strBuffer[0]);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(4, size);
    EXPECT_EQ("s1[0].b[0]", std::string(&strBuffer[0]));

    uniformNames[0] = "s1[1].a";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strBuffer[0]);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(1, size);
    EXPECT_EQ("s1[1].a", std::string(&strBuffer[0]));

    uniformNames[0] = "s1[1].b[0]";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strBuffer[0]);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(4, size);
    EXPECT_EQ("s1[1].b[0]", std::string(&strBuffer[0]));

    uniformNames[0] = "u0";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strBuffer[0]);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(1, size);
    EXPECT_EQ("u0", std::string(&strBuffer[0]));

    uniformNames[0] = "blockName1.f1";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strBuffer[0]);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(1, size);
    EXPECT_EQ("blockName1.f1", std::string(&strBuffer[0]));

    uniformNames[0] = "blockName1.b1";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strBuffer[0]);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(1, size);
    EXPECT_EQ("blockName1.b1", std::string(&strBuffer[0]));

    uniformNames[0] = "f2";
    glGetUniformIndices(program, 1, uniformNames, &index);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();
    glGetActiveUniform(program, index, maxLength, &length, &size, &type, &strBuffer[0]);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(1, size);
    EXPECT_EQ("f2", std::string(&strBuffer[0]));
}

// Test that using a very large buffer to back a small uniform block works OK.
TEST_P(UniformBufferTest, VeryLarge)
{
    glClear(GL_COLOR_BUFFER_BIT);
    float floatData[4] = {0.5f, 0.75f, 0.25f, 1.0f};

    GLsizei bigSize = 4096 * 64;
    std::vector<GLubyte> zero(bigSize, 0);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, bigSize, zero.data(), GL_STATIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 4, floatData);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);

    glUniformBlockBinding(mProgram, mUniformBufferIndex, 0);
    drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 128, 191, 64, 255, 1);
}

// Test that readback from a very large uniform buffer works OK.
TEST_P(UniformBufferTest, VeryLargeReadback)
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Generate some random data.
    GLsizei bigSize = 4096 * 64;
    std::vector<GLubyte> expectedData(bigSize);
    for (GLsizei index = 0; index < bigSize; ++index)
    {
        expectedData[index] = static_cast<GLubyte>(index);
    }

    // Initialize the GL buffer.
    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, bigSize, expectedData.data(), GL_STATIC_DRAW);

    // Do a small update.
    GLsizei smallSize              = sizeof(float) * 4;
    std::array<float, 4> floatData = {{0.5f, 0.75f, 0.25f, 1.0f}};
    memcpy(expectedData.data(), floatData.data(), smallSize);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, smallSize, expectedData.data());

    // Draw with the buffer.
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(mProgram, mUniformBufferIndex, 0);
    drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 128, 191, 64, 255, 1);

    // Read back the large buffer data.
    const void *mapPtr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, bigSize, GL_MAP_READ_BIT);
    ASSERT_GL_NO_ERROR();
    const GLubyte *bytePtr = reinterpret_cast<const GLubyte *>(mapPtr);
    std::vector<GLubyte> actualData(bytePtr, bytePtr + bigSize);
    EXPECT_EQ(expectedData, actualData);

    glUnmapBuffer(GL_UNIFORM_BUFFER);
}

// Test drawing with different sized uniform blocks from the same UBO, drawing a smaller uniform
// block before larger one.
TEST_P(UniformBufferTest, MultipleSizesSmallBeforeBig)
{
    constexpr size_t kSizeOfVec4  = 4 * sizeof(float);
    constexpr char kUniformName[] = "uni";
    constexpr char kFS1[]         = R"(#version 300 es
precision highp float;
layout(std140) uniform uni {
    bool b;
};

out vec4 fragColor;
void main() {
    fragColor = b ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    constexpr char kFS2[] = R"(#version 300 es
precision highp float;
layout(std140) uniform uni {
    bool b[2];
    vec4 v;
};

out vec4 fragColor;
void main() {
    fragColor = v;
})";

    GLint offsetAlignmentInBytes;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &offsetAlignmentInBytes);
    ASSERT_EQ(offsetAlignmentInBytes % kSizeOfVec4, 0U);
    GLint offsetAlignmentInVec4 = offsetAlignmentInBytes / kSizeOfVec4;

    // Insert padding required by implementation to have first unform block at a non-zero
    // offset.
    int initialPadding = rx::roundUp(3, offsetAlignmentInVec4);
    std::vector<float> uboData;
    for (int n = 0; n < initialPadding; ++n)
    {
        uboData.insert(uboData.end(), {0.0f, 0.0f, 0.0f, 0.0f});
    }

    // First uniform block - a single bool
    uboData.insert(uboData.end(), {1.0f, 0.0f, 0.0f, 0.0f});

    // Insert padding required by implementation to align second uniform block.
    for (int n = 0; n < offsetAlignmentInVec4 - 1; ++n)
    {
        uboData.insert(uboData.end(), {0.0f, 0.0f, 0.0f, 0.0f});
    }

    // Second uniform block
    uboData.insert(uboData.end(), {0.0f, 0.0f, 0.0f, 0.0f});
    uboData.insert(uboData.end(), {1.0f, 0.0f, 0.0f, 0.0f});
    uboData.insert(uboData.end(), {0.0f, 1.0f, 0.0f, 1.0f});

    ANGLE_GL_PROGRAM(program1, essl3_shaders::vs::Simple(), kFS1);
    ANGLE_GL_PROGRAM(program2, essl3_shaders::vs::Simple(), kFS2);

    // UBO containing 2 different uniform blocks
    GLBuffer ubo;
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    glBufferData(GL_UNIFORM_BUFFER, uboData.size() * sizeof(float), uboData.data(), GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    // Clear
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw with first uniform block
    GLuint index = glGetUniformBlockIndex(program1, kUniformName);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, initialPadding * kSizeOfVec4, kSizeOfVec4);
    ASSERT_GL_NO_ERROR();

    glUniformBlockBinding(program1, index, 0);
    drawQuad(program1, essl3_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_NEAR(0, 0, 0, 255, 0, 255, 1);

    // Draw with second uniform block
    index = glGetUniformBlockIndex(program2, kUniformName);
    EXPECT_NE(GL_INVALID_INDEX, index);
    ASSERT_GL_NO_ERROR();

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo,
                      (initialPadding + offsetAlignmentInVec4) * kSizeOfVec4, 3 * kSizeOfVec4);
    ASSERT_GL_NO_ERROR();

    glUniformBlockBinding(program2, index, 0);
    drawQuad(program2, essl3_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_NEAR(0, 0, 0, 255, 0, 255, 1);
}

class UniformBufferTest31 : public ANGLETest<>
{
  protected:
    UniformBufferTest31()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Test uniform block bindings greater than GL_MAX_UNIFORM_BUFFER_BINDINGS cause compile error.
TEST_P(UniformBufferTest31, MaxUniformBufferBindingsExceeded)
{
    GLint maxUniformBufferBindings;
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUniformBufferBindings);
    std::string source =
        "#version 310 es\n"
        "in vec4 position;\n"
        "layout(binding = ";
    std::stringstream ss;
    ss << maxUniformBufferBindings;
    source = source + ss.str() +
             ") uniform uni {\n"
             "    vec4 color;\n"
             "};\n"
             "void main()\n"
             "{\n"
             "    gl_Position = position;\n"
             "}";
    GLuint shader = CompileShader(GL_VERTEX_SHADER, source.c_str());
    EXPECT_EQ(0u, shader);
}

// Test uniform block bindings specified by layout in shader work properly.
TEST_P(UniformBufferTest31, UniformBufferBindings)
{
    constexpr char kVS[] =
        "#version 310 es\n"
        "in vec4 position;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = position;\n"
        "}";
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "layout(binding = 2) uniform uni {\n"
        "    vec4 color;\n"
        "};\n"
        "out vec4 fragColor;\n"
        "void main()\n"
        "{"
        "    fragColor = color;\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    GLuint uniformBufferIndex = glGetUniformBlockIndex(program, "uni");
    ASSERT_NE(GL_INVALID_INDEX, uniformBufferIndex);
    GLBuffer uniformBuffer;

    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    ASSERT_GL_NO_ERROR();

    // Let's create a buffer which contains one vec4.
    GLuint vec4Size = 4 * sizeof(float);
    std::vector<char> v(vec4Size);
    float *first = reinterpret_cast<float *>(v.data());

    first[0] = 10.f / 255.f;
    first[1] = 20.f / 255.f;
    first[2] = 30.f / 255.f;
    first[3] = 40.f / 255.f;

    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, vec4Size, v.data(), GL_STATIC_DRAW);

    EXPECT_GL_NO_ERROR();

    glBindBufferBase(GL_UNIFORM_BUFFER, 2, uniformBuffer);
    drawQuad(program, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(px, py, 10, 20, 30, 40);

    // Clear the framebuffer
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(px, py, 0, 0, 0, 0);

    // Try to bind the buffer to another binding point
    glUniformBlockBinding(program, uniformBufferIndex, 5);
    glBindBufferBase(GL_UNIFORM_BUFFER, 5, uniformBuffer);
    drawQuad(program, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(px, py, 10, 20, 30, 40);
}

// Test uniform blocks used as instanced array take next binding point for each subsequent element.
TEST_P(UniformBufferTest31, ConsecutiveBindingsForBlockArray)
{
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "layout(binding = 2) uniform uni {\n"
        "    vec4 color;\n"
        "} blocks[2];\n"
        "out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = blocks[0].color + blocks[1].color;\n"
        "}";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    std::array<GLBuffer, 2> uniformBuffers;

    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    ASSERT_GL_NO_ERROR();

    // Let's create a buffer which contains one vec4.
    GLuint vec4Size = 4 * sizeof(float);
    std::vector<char> v(vec4Size);
    float *first = reinterpret_cast<float *>(v.data());

    first[0] = 10.f / 255.f;
    first[1] = 20.f / 255.f;
    first[2] = 30.f / 255.f;
    first[3] = 40.f / 255.f;

    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffers[0]);
    glBufferData(GL_UNIFORM_BUFFER, vec4Size, v.data(), GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, uniformBuffers[0]);
    ASSERT_GL_NO_ERROR();
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffers[1]);
    glBufferData(GL_UNIFORM_BUFFER, vec4Size, v.data(), GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, uniformBuffers[1]);

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(px, py, 20, 40, 60, 80);
}

// Test the layout qualifier binding must be both specified(ESSL 3.10.4 section 9.2).
TEST_P(UniformBufferTest31, BindingMustBeBothSpecified)
{
    constexpr char kVS[] =
        "#version 310 es\n"
        "in vec4 position;\n"
        "uniform uni\n"
        "{\n"
        "    vec4 color;\n"
        "} block;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = position + block.color;\n"
        "}";
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "layout(binding = 0) uniform uni\n"
        "{\n"
        "    vec4 color;\n"
        "} block;\n"
        "out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = block.color;\n"
        "}";
    GLuint program = CompileProgram(kVS, kFS);
    ASSERT_EQ(0u, program);
}

// Test that uploading data to buffer that's in use then using it as indirect buffer works.
TEST_P(UniformBufferTest31, UseAsUBOThenUpdateThenDrawIndirect)
{
    // http://anglebug.com/42264362
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // http://anglebug.com/42264411
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsPixel2());

    const std::array<uint32_t, 4> kInitialData = {100, 200, 300, 400};
    const std::array<uint32_t, 4> kUpdateData  = {4, 1, 0, 0};

    GLBuffer buffer;
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(kInitialData), kInitialData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);
    EXPECT_GL_NO_ERROR();

    constexpr char kVerifyUBO[] = R"(#version 310 es
precision mediump float;
layout(binding = 0) uniform block {
    uvec4 data;
} ubo;
out vec4 colorOut;
void main()
{
    if (all(equal(ubo.data, uvec4(100, 200, 300, 400))))
        colorOut = vec4(0, 1.0, 0, 1.0);
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    ANGLE_GL_PROGRAM(verifyUbo, essl31_shaders::vs::Simple(), kVerifyUBO);
    drawQuad(verifyUbo, essl31_shaders::PositionAttrib(), 0.5);
    EXPECT_GL_NO_ERROR();

    // Update buffer data
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(kInitialData), kUpdateData.data());
    EXPECT_GL_NO_ERROR();

    // Draw indirect using the updated parameters
    constexpr char kVS[] = R"(#version 310 es
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
out vec4 colorOut;
void main()
{
    colorOut = vec4(0, 0, 1.0, 1.0);
})";

    ANGLE_GL_PROGRAM(draw, kVS, kFS);
    glUseProgram(draw);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffer);
    EXPECT_GL_NO_ERROR();

    GLVertexArray vao;
    glBindVertexArray(vao);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArraysIndirect(GL_TRIANGLE_STRIP, nullptr);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::cyan);
}

// Test four bindings of the same uniform buffer.
TEST_P(UniformBufferTest31, FourBindingsSameBuffer)
{
    // Create a program with accesses from four UBO binding points.
    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
layout (binding = 0) uniform block0 {
    vec4 color;
} b0;
layout (binding = 1) uniform block1 {
    vec4 color;
} b1;
layout (binding = 2) uniform block2 {
    vec4 color;
} b2;
layout (binding = 3) uniform block3 {
    vec4 color;
} b3;

out vec4 outColor;

void main() {
    outColor = b0.color + b1.color + b2.color + b3.color;
})";

    // Create a UBO.
    constexpr GLfloat uboData[4] = {0.0, 0.25, 0.0, 0.25};
    GLBuffer ubo;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uboData), uboData, GL_STATIC_READ);

    // Bind the same UBO to all four binding points.
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, ubo);

    ANGLE_GL_PROGRAM(uboProgram, essl31_shaders::vs::Simple(), kFS);
    drawQuad(uboProgram, essl1_shaders::PositionAttrib(), 0.5f, 1.0f);
    EXPECT_GL_NO_ERROR();

    // Four {0, 0.25, 0, 0.25} pixels should sum to green.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test with a block containing an array of structs.
TEST_P(UniformBufferTest, BlockContainingArrayOfStructs)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 my_FragColor;\n"
        "struct light_t {\n"
        "    vec4 intensity;\n"
        "};\n"
        "const int maxLights = 2;\n"
        "layout(std140) uniform lightData { light_t lights[maxLights]; };\n"
        "vec4 processLight(vec4 lighting, light_t light)\n"
        "{\n"
        "    return lighting + light.intensity;\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    vec4 lighting = vec4(0, 0, 0, 1);\n"
        "    for (int n = 0; n < maxLights; n++)\n"
        "    {\n"
        "        lighting = processLight(lighting, lights[n]);\n"
        "    }\n"
        "    my_FragColor = lighting;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "lightData");

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    const GLsizei kStructCount        = 2;
    const GLsizei kVectorElementCount = 4;
    const GLsizei kBytesPerElement    = 4;
    const GLsizei kDataSize           = kStructCount * kVectorElementCount * kBytesPerElement;
    std::vector<GLubyte> v(kDataSize, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());

    vAsFloat[1]                       = 0.5f;
    vAsFloat[kVectorElementCount + 1] = 0.5f;

    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test with a block instance array containing an array of structs.
TEST_P(UniformBufferTest, BlockArrayContainingArrayOfStructs)
{
    constexpr char kFS[] =
        R"(#version 300 es

        precision highp float;
        out vec4 my_FragColor;
        struct light_t
        {
            vec4 intensity;
        };

        layout(std140) uniform lightData { light_t lights[2]; } buffers[2];

        vec4 processLight(vec4 lighting, light_t light)
        {
            return lighting + light.intensity;
        }
        void main()
        {
            vec4 lighting = vec4(0, 0, 0, 1);
            lighting = processLight(lighting, buffers[0].lights[0]);
            lighting = processLight(lighting, buffers[1].lights[1]);
            my_FragColor = lighting;
        })";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLint uniformBufferIndex  = glGetUniformBlockIndex(program, "lightData[0]");
    GLint uniformBuffer2Index = glGetUniformBlockIndex(program, "lightData[1]");

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    const GLsizei kStructCount        = 2;
    const GLsizei kVectorElementCount = 4;
    const GLsizei kBytesPerElement    = 4;
    const GLsizei kDataSize           = kStructCount * kVectorElementCount * kBytesPerElement;
    std::vector<GLubyte> v(kDataSize, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());

    // In the first struct/vector of the first block
    vAsFloat[1] = 0.5f;

    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);

    GLBuffer uniformBuffer2;
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer2);

    vAsFloat[1] = 0.0f;
    // In the second struct/vector of the second block
    vAsFloat[kVectorElementCount + 1] = 0.5f;
    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uniformBuffer2);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    glUniformBlockBinding(program, uniformBuffer2Index, 1);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test with a block containing an array of structs containing arrays.
TEST_P(UniformBufferTest, BlockContainingArrayOfStructsContainingArrays)
{
    constexpr char kFS[] =
        R"(#version 300 es
        precision highp float;
        out vec4 my_FragColor;
        struct light_t
        {
            vec4 intensity[3];
        };
        const int maxLights = 2;
        layout(std140) uniform lightData { light_t lights[maxLights]; };
        vec4 processLight(vec4 lighting, light_t light)
        {
            return lighting + light.intensity[1];
        }
        void main()
        {
            vec4 lighting = vec4(0, 0, 0, 1);
            lighting = processLight(lighting, lights[0]);
            lighting = processLight(lighting, lights[1]);
            my_FragColor = lighting;
        })";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "lightData");

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    const GLsizei kStructCount       = 2;
    const GLsizei kVectorsPerStruct  = 3;
    const GLsizei kElementsPerVector = 4;
    const GLsizei kBytesPerElement   = 4;
    const GLsizei kDataSize =
        kStructCount * kVectorsPerStruct * kElementsPerVector * kBytesPerElement;
    std::vector<GLubyte> v(kDataSize, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());

    vAsFloat[kElementsPerVector + 1]                                          = 0.5f;
    vAsFloat[kVectorsPerStruct * kElementsPerVector + kElementsPerVector + 1] = 0.5f;

    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test with a block containing nested structs.
TEST_P(UniformBufferTest, BlockContainingNestedStructs)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 my_FragColor;\n"
        "struct light_t {\n"
        "    vec4 intensity;\n"
        "};\n"
        "struct lightWrapper_t {\n"
        "    light_t light;\n"
        "};\n"
        "const int maxLights = 2;\n"
        "layout(std140) uniform lightData { lightWrapper_t lightWrapper; };\n"
        "vec4 processLight(vec4 lighting, lightWrapper_t aLightWrapper)\n"
        "{\n"
        "    return lighting + aLightWrapper.light.intensity;\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    vec4 lighting = vec4(0, 0, 0, 1);\n"
        "    for (int n = 0; n < maxLights; n++)\n"
        "    {\n"
        "        lighting = processLight(lighting, lightWrapper);\n"
        "    }\n"
        "    my_FragColor = lighting;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "lightData");

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    const GLsizei kVectorsPerStruct  = 3;
    const GLsizei kElementsPerVector = 4;
    const GLsizei kBytesPerElement   = 4;
    const GLsizei kDataSize          = kVectorsPerStruct * kElementsPerVector * kBytesPerElement;
    std::vector<GLubyte> v(kDataSize, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());

    vAsFloat[1] = 1.0f;

    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests GetUniformBlockIndex return value on error.
TEST_P(UniformBufferTest, GetUniformBlockIndexDefaultReturn)
{
    ASSERT_FALSE(glIsProgram(99));
    EXPECT_EQ(GL_INVALID_INDEX, glGetUniformBlockIndex(99, "farts"));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Block names can be reserved names in GLSL, as long as they're not reserved in GLSL ES.
TEST_P(UniformBufferTest, UniformBlockReservedOpenGLName)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 my_FragColor;\n"
        "layout(std140) uniform buffer { vec4 color; };\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = color;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    const GLsizei kElementsPerVector = 4;
    const GLsizei kBytesPerElement   = 4;
    const GLsizei kDataSize          = kElementsPerVector * kBytesPerElement;
    std::vector<GLubyte> v(kDataSize, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());

    vAsFloat[1] = 1.0f;
    vAsFloat[3] = 1.0f;

    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Block instance names can be reserved names in GLSL, as long as they're not reserved in GLSL ES.
TEST_P(UniformBufferTest, UniformBlockInstanceReservedOpenGLName)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 my_FragColor;\n"
        "layout(std140) uniform dmat2 { vec4 color; } buffer;\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = buffer.color;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "dmat2");

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    const GLsizei kElementsPerVector = 4;
    const GLsizei kBytesPerElement   = 4;
    const GLsizei kDataSize          = kElementsPerVector * kBytesPerElement;
    std::vector<GLubyte> v(kDataSize, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());

    vAsFloat[1] = 1.0f;
    vAsFloat[3] = 1.0f;

    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that uniform block instance with nested structs that contain vec3s inside is handled
// correctly. This is meant to test that HLSL structure padding to implement std140 layout works
// together with uniform blocks.
TEST_P(UniformBufferTest, Std140UniformBlockInstanceWithNestedStructsContainingVec3s)
{
    // Got incorrect test result on non-NVIDIA Android - the alpha channel was not set correctly
    // from the second vector, possibly the platform doesn't implement std140 packing right?
    // http://anglebug.com/42260937
    ANGLE_SKIP_TEST_IF(IsAndroid() && !IsNVIDIA());

    constexpr char kFS[] =
        R"(#version 300 es

        precision highp float;
        out vec4 my_FragColor;

        struct Sinner {
          vec3 v;
        };

        struct S {
            Sinner s1;
            Sinner s2;
        };

        layout(std140) uniform structBuffer { S s; } buffer;

        void accessStruct(S s)
        {
            my_FragColor = vec4(s.s1.v.xy, s.s2.v.xy);
        }

        void main()
        {
            accessStruct(buffer.s);
        })";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "structBuffer");

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    const GLsizei kVectorsPerBlock         = 2;
    const GLsizei kElementsPerPaddedVector = 4;
    const GLsizei kBytesPerElement         = 4;
    const GLsizei kDataSize = kVectorsPerBlock * kElementsPerPaddedVector * kBytesPerElement;
    std::vector<GLubyte> v(kDataSize, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());

    // Set second value in each vec3.
    vAsFloat[1u]      = 1.0f;
    vAsFloat[4u + 1u] = 1.0f;

    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests the detaching shaders from the program and using uniform blocks works.
// This covers a bug in ANGLE's D3D back-end.
TEST_P(UniformBufferTest, DetachShaders)
{
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, essl3_shaders::vs::Simple());
    ASSERT_NE(0u, vertexShader);
    GLuint kFS = CompileShader(GL_FRAGMENT_SHADER, mkFS);
    ASSERT_NE(0u, kFS);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, kFS);

    ASSERT_TRUE(LinkAttachedProgram(program));

    glDetachShader(program, vertexShader);
    glDetachShader(program, kFS);
    glDeleteShader(vertexShader);
    glDeleteShader(kFS);

    glClear(GL_COLOR_BUFFER_BIT);
    float floatData[4] = {0.5f, 0.75f, 0.25f, 1.0f};

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4, floatData, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);

    GLint uniformBufferIndex = glGetUniformBlockIndex(mProgram, "uni");
    ASSERT_NE(uniformBufferIndex, -1);

    glUniformBlockBinding(program, uniformBufferIndex, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 128, 191, 64, 255, 1);

    glDeleteProgram(program);
}

// Test a uniform block where the whole block is set as row-major.
TEST_P(UniformBufferTest, Std140UniformBlockWithRowMajorQualifier)
{
    // AMD OpenGL driver doesn't seem to apply the row-major qualifier right.
    // http://anglebug.com/40096480
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL() && !IsMac());

    constexpr char kFS[] =
        R"(#version 300 es

        precision highp float;
        out vec4 my_FragColor;

        layout(std140, row_major) uniform matrixBuffer
        {
            mat2 m;
        } buffer;

        void main()
        {
            // Vector constructor accesses elements in column-major order.
            my_FragColor = vec4(buffer.m);
        })";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "matrixBuffer");

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    const GLsizei kElementsPerMatrix = 8;  // Each mat2 row gets padded into a vec4.
    const GLsizei kBytesPerElement   = 4;
    const GLsizei kDataSize          = kElementsPerMatrix * kBytesPerElement;
    std::vector<GLubyte> v(kDataSize, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());

    vAsFloat[0u] = 1.0f;
    vAsFloat[1u] = 128.0f / 255.0f;
    vAsFloat[4u] = 64.0f / 255.0f;
    vAsFloat[5u] = 32.0f / 255.0f;

    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, 64, 128, 32), 5);
}

// Test a uniform block where an individual matrix field is set as row-major whereas the whole block
// is set as column-major.
TEST_P(UniformBufferTest, Std140UniformBlockWithPerMemberRowMajorQualifier)
{
    // AMD OpenGL driver doesn't seem to apply the row-major qualifier right.
    // http://anglebug.com/40096480
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL() && !IsMac());

    constexpr char kFS[] =
        R"(#version 300 es

        precision highp float;
        out vec4 my_FragColor;

        layout(std140, column_major) uniform matrixBuffer
        {
            layout(row_major) mat2 m;
        } buffer;

        void main()
        {
            // Vector constructor accesses elements in column-major order.
            my_FragColor = vec4(buffer.m);
        })";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "matrixBuffer");

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    const GLsizei kElementsPerMatrix = 8;  // Each mat2 row gets padded into a vec4.
    const GLsizei kBytesPerElement   = 4;
    const GLsizei kDataSize          = kElementsPerMatrix * kBytesPerElement;
    std::vector<GLubyte> v(kDataSize, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());

    vAsFloat[0u] = 1.0f;
    vAsFloat[1u] = 128.0f / 255.0f;
    vAsFloat[4u] = 64.0f / 255.0f;
    vAsFloat[5u] = 32.0f / 255.0f;

    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, 64, 128, 32), 5);
}

// Test a uniform block where an individual matrix field is set as column-major whereas the whole
// block is set as row-major.
TEST_P(UniformBufferTest, Std140UniformBlockWithPerMemberColumnMajorQualifier)
{
    constexpr char kFS[] =
        R"(#version 300 es

        precision highp float;
        out vec4 my_FragColor;

        layout(std140, row_major) uniform matrixBuffer
        {
            // 2 columns, 3 rows.
            layout(column_major) mat2x3 m;
        } buffer;

        void main()
        {
            // Vector constructor accesses elements in column-major order.
            my_FragColor = vec4(buffer.m);
        })";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "matrixBuffer");

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    const GLsizei kElementsPerMatrix = 8;  // Each mat2x3 column gets padded into a vec4.
    const GLsizei kBytesPerElement   = 4;
    const GLsizei kDataSize          = kElementsPerMatrix * kBytesPerElement;
    std::vector<GLubyte> v(kDataSize, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());

    vAsFloat[0u] = 1.0f;
    vAsFloat[1u] = 192.0f / 255.0f;
    vAsFloat[2u] = 128.0f / 255.0f;
    vAsFloat[4u] = 96.0f / 255.0f;
    vAsFloat[5u] = 64.0f / 255.0f;
    vAsFloat[6u] = 32.0f / 255.0f;

    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, 192, 128, 96), 5);
}

// Test a uniform block where a struct field is set as row-major.
TEST_P(UniformBufferTest, Std140UniformBlockWithRowMajorQualifierOnStruct)
{
    // AMD OpenGL driver doesn't seem to apply the row-major qualifier right.
    // http://anglebug.com/40096480
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL() && !IsMac());

    constexpr char kFS[] =
        R"(#version 300 es

        precision highp float;
        out vec4 my_FragColor;

        struct S
        {
            mat2 m;
        };

        layout(std140) uniform matrixBuffer
        {
            layout(row_major) S s;
        } buffer;

        void main()
        {
            // Vector constructor accesses elements in column-major order.
            my_FragColor = vec4(buffer.s.m);
        })";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "matrixBuffer");

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    const GLsizei kElementsPerMatrix = 8;  // Each mat2 row gets padded into a vec4.
    const GLsizei kBytesPerElement   = 4;
    const GLsizei kDataSize          = kElementsPerMatrix * kBytesPerElement;
    std::vector<GLubyte> v(kDataSize, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());

    vAsFloat[0u] = 1.0f;
    vAsFloat[1u] = 128.0f / 255.0f;
    vAsFloat[4u] = 64.0f / 255.0f;
    vAsFloat[5u] = 32.0f / 255.0f;

    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, 64, 128, 32), 5);
}

// Regression test for a dirty bit bug in ANGLE. See http://crbug.com/792966
TEST_P(UniformBufferTest, SimpleBindingChange)
{
    constexpr char kFragmentShader[] = R"(#version 300 es
precision mediump float;

layout (std140) uniform color_ubo
{
  vec4 color;
};

out vec4 fragColor;
void main()
{
  fragColor = color;
})";

    // http://anglebug.com/40096481
    ANGLE_SKIP_TEST_IF(IsMac() && IsNVIDIA() && IsDesktopOpenGL());

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFragmentShader);

    glBindAttribLocation(program, 0, essl3_shaders::PositionAttrib());
    glUseProgram(program);
    GLint uboIndex = glGetUniformBlockIndex(program, "color_ubo");

    std::array<GLfloat, 12> vertices{{-1, -1, 0, 1, -1, 0, -1, 1, 0, 1, 1, 0}};
    GLBuffer vertexBuf;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

    std::array<GLshort, 12> indexData = {{0, 1, 2, 2, 1, 3, 0, 1, 2, 2, 1, 3}};

    GLBuffer indexBuf;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLshort), indexData.data(),
                 GL_STATIC_DRAW);

    // Bind a first buffer with red.
    GLBuffer uboBuf1;
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBuf1);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatRed, GL_STATIC_DRAW);
    glUniformBlockBinding(program, uboIndex, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    // Bind a second buffer with green, updating the buffer binding.
    GLBuffer uboBuf2;
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboBuf2);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatGreen, GL_STATIC_DRAW);
    glUniformBlockBinding(program, uboIndex, 1);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(12));

    // Verify we get the second buffer.
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Regression test for a dirty bit bug in ANGLE. Same as above but for the indexed bindings.
TEST_P(UniformBufferTest, SimpleBufferChange)
{
    constexpr char kFragmentShader[] = R"(#version 300 es
precision mediump float;

layout (std140) uniform color_ubo
{
  vec4 color;
};

out vec4 fragColor;
void main()
{
  fragColor = color;
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFragmentShader);

    glBindAttribLocation(program, 0, essl3_shaders::PositionAttrib());
    glUseProgram(program);
    GLint uboIndex = glGetUniformBlockIndex(program, "color_ubo");

    std::array<GLfloat, 12> vertices{{-1, -1, 0, 1, -1, 0, -1, 1, 0, 1, 1, 0}};
    GLBuffer vertexBuf;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

    std::array<GLshort, 12> indexData = {{0, 1, 2, 2, 1, 3, 0, 1, 2, 2, 1, 3}};

    GLBuffer indexBuf;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLshort), indexData.data(),
                 GL_STATIC_DRAW);

    // Bind a first buffer with red.
    GLBuffer uboBuf1;
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBuf1);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatRed, GL_STATIC_DRAW);
    glUniformBlockBinding(program, uboIndex, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    // Bind a second buffer to the same binding point (0). This should set to draw green.
    GLBuffer uboBuf2;
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBuf2);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatGreen, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(12));

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests a bug in the D3D11 back-end where re-creating the buffer storage should trigger a state
// update in the State Manager class.
TEST_P(UniformBufferTest, DependentBufferChange)
{
    constexpr char kFragmentShader[] = R"(#version 300 es
precision mediump float;

layout (std140) uniform color_ubo
{
  vec4 color;
};

out vec4 fragColor;
void main()
{
  fragColor = color;
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFragmentShader);

    glBindAttribLocation(program, 0, essl3_shaders::PositionAttrib());
    glUseProgram(program);
    GLint uboIndex = glGetUniformBlockIndex(program, "color_ubo");

    std::array<GLfloat, 12> vertices{{-1, -1, 0, 1, -1, 0, -1, 1, 0, 1, 1, 0}};
    GLBuffer vertexBuf;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

    std::array<GLshort, 6> indexData = {{0, 1, 2, 2, 1, 3}};

    GLBuffer indexBuf;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLshort), indexData.data(),
                 GL_STATIC_DRAW);

    GLBuffer ubo;
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatRed, GL_STATIC_DRAW);
    glUniformBlockBinding(program, uboIndex, 0);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Resize the buffer - triggers a re-allocation in the D3D11 back-end.
    std::vector<GLColor32F> bigData(128, kFloatGreen);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F) * bigData.size(), bigData.data(),
                 GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Recreate WebGL conformance test conformance2/uniforms/large-uniform-buffers.html to test
// regression in http://anglebug.com/42262055
TEST_P(UniformBufferTest, SizeOverMaxBlockSize)
{
    constexpr char kFragmentShader[] = R"(#version 300 es
precision mediump float;

layout (std140) uniform color_ubo
{
  vec4 color;
};

out vec4 fragColor;
void main()
{
  fragColor = color;
})";

    // Test crashes on Windows AMD OpenGL
    ANGLE_SKIP_TEST_IF(IsAMD() && IsWindows() && IsOpenGL());
    // http://anglebug.com/42263922
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFragmentShader);

    glBindAttribLocation(program, 0, essl3_shaders::PositionAttrib());
    glUseProgram(program);
    GLint uboIndex = glGetUniformBlockIndex(program, "color_ubo");

    std::array<GLfloat, 12> vertices{{-1, -1, 0, 1, -1, 0, -1, 1, 0, 1, 1, 0}};
    GLBuffer vertexBuf;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

    std::array<GLshort, 6> indexData = {{0, 1, 2, 2, 1, 3}};

    GLBuffer indexBuf;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLshort), indexData.data(),
                 GL_STATIC_DRAW);

    GLint uboDataSize = 0;
    glGetActiveUniformBlockiv(program, uboIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uboDataSize);
    EXPECT_NE(uboDataSize, 0);  // uniform block data size invalid

    GLint64 maxUniformBlockSize;
    glGetInteger64v(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);

    GLBuffer uboBuf;
    std::vector<GLfloat> uboData;
    uboData.resize(maxUniformBlockSize * 2);  // underlying data is twice the max block size

    GLint offs0 = 0;

    // Red
    uboData[offs0 + 0] = 1;
    uboData[offs0 + 1] = 0;
    uboData[offs0 + 2] = 0;
    uboData[offs0 + 3] = 1;

    GLint offs1     = maxUniformBlockSize;
    GLint alignment = 0;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
    EXPECT_EQ(offs1 % alignment, 0);

    // Green
    uboData[offs1 + 0] = 0;
    uboData[offs1 + 1] = 1;
    uboData[offs1 + 2] = 0;
    uboData[offs1 + 3] = 1;

    glUniformBlockBinding(program, uboIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBuf);
    glBufferData(GL_UNIFORM_BUFFER, uboData.size() * sizeof(GLfloat), uboData.data(),
                 GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();  // No errors from setup

    // Draw lower triangle - should be red
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboBuf, offs0 * sizeof(float), 4 * sizeof(float));
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
    ASSERT_GL_NO_ERROR();  // No errors from draw

    // Draw upper triangle - should be green
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboBuf, offs1 * sizeof(float), 4 * sizeof(float));
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
                   reinterpret_cast<void *>(3 * sizeof(GLshort)));
    ASSERT_GL_NO_ERROR();  // No errors from draw

    GLint width  = getWindowWidth();
    GLint height = getWindowHeight();
    // Lower left should be red
    EXPECT_PIXEL_COLOR_EQ(width / 2 - 5, height / 2 - 5, GLColor::red);
    // Top right should be green
    EXPECT_PIXEL_COLOR_EQ(width / 2 + 5, height / 2 + 5, GLColor::green);
}

// Test a uniform block where an array of row-major matrices is dynamically indexed.
TEST_P(UniformBufferTest, Std140UniformBlockWithDynamicallyIndexedRowMajorArray)
{
    // http://anglebug.com/42262481 , http://anglebug.com/40096480
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL() && (IsIntel() || IsAMD()));

    constexpr char kFS[] =
        R"(#version 300 es

        precision highp float;
        out vec4 my_FragColor;

        uniform int u_zero;

        layout(std140, row_major) uniform matrixBuffer {
            mat4 u_mats[1];
        };

        void main() {
            float f = u_mats[u_zero + 0][2][1];
            my_FragColor = vec4(1.0 - f, f, 0.0, 1.0);
        })";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "matrixBuffer");

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    const GLsizei kElementsPerMatrix = 16;  // Each mat2 row gets padded into a vec4.
    const GLsizei kBytesPerElement   = 4;
    const GLsizei kDataSize          = kElementsPerMatrix * kBytesPerElement;
    std::vector<GLubyte> v(kDataSize, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());
    // Write out this initializer to make it clearer what the matrix contains.
    float matrixData[kElementsPerMatrix] = {
        // clang-format off
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        // clang-format on
    };
    for (int ii = 0; ii < kElementsPerMatrix; ++ii)
    {
        vAsFloat[ii] = matrixData[ii];
    }
    glBufferData(GL_UNIFORM_BUFFER, kDataSize, v.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    GLint indexLoc = glGetUniformLocation(program, "u_zero");
    glUseProgram(program);
    glUniform1i(indexLoc, 0);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(0, 255, 0, 255), 5);
}

// Test with many uniform buffers work as expected.
TEST_P(UniformBufferTest, ManyBlocks)
{
    // http://anglebug.com/42263608
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr char kFS[] =
        R"(#version 300 es

        precision highp float;
        out vec4 my_FragColor;

        layout(std140) uniform uboBlock { vec4 color; } blocks[12];

        void main()
        {
            vec4 color = vec4(0, 0, 0, 1);
            color += blocks[0].color;
            color += blocks[1].color;
            color += blocks[2].color;
            color += blocks[3].color;
            color += blocks[4].color;
            color += blocks[5].color;
            color += blocks[6].color;
            color += blocks[7].color;
            color += blocks[8].color;
            color += blocks[9].color;
            color += blocks[10].color;
            color += blocks[11].color;
            my_FragColor = vec4(color.rgb, 1.0);
        })";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLBuffer buffers[12];
    GLint bufferIndex[12];
    bufferIndex[0]  = glGetUniformBlockIndex(program, "uboBlock[0]");
    bufferIndex[1]  = glGetUniformBlockIndex(program, "uboBlock[1]");
    bufferIndex[2]  = glGetUniformBlockIndex(program, "uboBlock[2]");
    bufferIndex[3]  = glGetUniformBlockIndex(program, "uboBlock[3]");
    bufferIndex[4]  = glGetUniformBlockIndex(program, "uboBlock[4]");
    bufferIndex[5]  = glGetUniformBlockIndex(program, "uboBlock[5]");
    bufferIndex[6]  = glGetUniformBlockIndex(program, "uboBlock[6]");
    bufferIndex[7]  = glGetUniformBlockIndex(program, "uboBlock[7]");
    bufferIndex[8]  = glGetUniformBlockIndex(program, "uboBlock[8]");
    bufferIndex[9]  = glGetUniformBlockIndex(program, "uboBlock[9]");
    bufferIndex[10] = glGetUniformBlockIndex(program, "uboBlock[10]");
    bufferIndex[11] = glGetUniformBlockIndex(program, "uboBlock[11]");

    std::vector<GLubyte> v(16, 0);
    float *vAsFloat = reinterpret_cast<float *>(v.data());

    for (int i = 0; i < 12; ++i)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, buffers[i]);
        vAsFloat[0] = (i + 1) / 255.0f;
        vAsFloat[1] = (i + 1) / 255.0f;
        vAsFloat[2] = (i + 1) / 255.0f;
        vAsFloat[3] = .0f;

        glBufferData(GL_UNIFORM_BUFFER, v.size(), v.data(), GL_STATIC_DRAW);

        glBindBufferBase(GL_UNIFORM_BUFFER, i, buffers[i]);
        glUniformBlockBinding(program, bufferIndex[i], i);
    }

    glViewport(0, 0, getWindowWidth() / 2, getWindowHeight());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    // Modify buffer[1]
    glBindBuffer(GL_UNIFORM_BUFFER, buffers[1]);

    vAsFloat[0] = 2 / 255.0f;
    vAsFloat[1] = 22 / 255.0f;  // green channel increased by 20
    vAsFloat[2] = 2 / 255.0f;
    vAsFloat[3] = .0f;

    glBufferData(GL_UNIFORM_BUFFER, v.size(), v.data(), GL_STATIC_DRAW);

    glViewport(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    // First draw
    EXPECT_PIXEL_NEAR(0, 0, 78, 78, 78, 255, 2);
    // Second draw: green channel increased by 20
    EXPECT_PIXEL_NEAR(getWindowWidth() / 2, 0, 78, 98, 78, 255, 2);
}

// These suite cases test the uniform blocks with a large array member. Unlike other uniform
// blocks that will be translated to cbuffer type on D3D backend, we will tranlate these
// uniform blocks to StructuredBuffer for slow fxc compile performance issue with dynamic
// uniform indexing, angleproject/3682.
class UniformBlockWithOneLargeArrayMemberTest : public ANGLETest<>
{
  protected:
    UniformBlockWithOneLargeArrayMemberTest()
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
        glGetInteger64v(GL_MAX_UNIFORM_BLOCK_SIZE, &mMaxUniformBlockSize);
        // Ensure that shader uniform block does not exceed MAX_UNIFORM_BLOCK_SIZE limit.
        if (mMaxUniformBlockSize >= 16384 && mMaxUniformBlockSize < 32768)
        {
            mArraySize1 = 128;
            mArraySize2 = 8;
            mDivisor1   = 128;
            mDivisor2   = 32;
            mDivisor3   = 16;
        }
        else if (mMaxUniformBlockSize >= 32768 && mMaxUniformBlockSize < 65536)
        {
            mArraySize1 = 256;
            mArraySize2 = 16;
            mDivisor1   = 64;
            mDivisor2   = 16;
            mDivisor3   = 8;
        }
        else
        {
            mArraySize1 = 512;
            mArraySize2 = 32;
            mDivisor1   = 32;
            mDivisor2   = 8;
            mDivisor3   = 4;
        }

        glGenBuffers(1, &mUniformBuffer);
        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteBuffers(1, &mUniformBuffer); }

    void generateArraySizeAndDivisorsDeclaration(std::ostringstream &out,
                                                 bool hasArraySize2,
                                                 bool hasDivisor2,
                                                 bool hasDivisor3)
    {
        if (hasArraySize2)
        {
            out << "const uint arraySize1 = " << mArraySize1 << "u;\n";
            out << "const uint arraySize2 = " << mArraySize2 << "u;\n";
        }
        else
        {
            out << "const uint arraySize = " << mArraySize1 << "u;\n";
        }

        if (hasDivisor2)
        {
            out << "const uint divisor1 = " << mDivisor1 << "u;\n";
            out << "const uint divisor2 = " << mDivisor2 << "u;\n";
        }
        else
        {
            out << "const uint divisor = " << mDivisor1 << "u;\n";
        }
        if (hasDivisor3)
        {
            out << "const uint divisor3 = " << mDivisor3 << "u;\n";
        }
    }
    GLuint getArraySize() { return mArraySize1; }
    GLuint getArraySize2() { return mArraySize2; }

    void setArrayValues(std::vector<GLfloat> &floatData,
                        GLuint beginIndex,
                        GLuint endIndex,
                        GLuint stride,
                        GLuint firstElementOffset,
                        GLuint firstEleVecCount,
                        GLuint firstEleVecComponents,
                        float x1,
                        float y1,
                        float z1,
                        float w1,
                        GLuint secondElementOffset    = 0,
                        GLuint secondEleVecCount      = 0,
                        GLuint secondEleVecComponents = 0,
                        float x2                      = 0.0f,
                        float y2                      = 0.0f,
                        float z2                      = 0.0f,
                        float w2                      = 0.0f)
    {
        for (GLuint i = beginIndex; i < endIndex; i++)
        {
            for (GLuint j = 0; j < firstEleVecCount; j++)
            {
                if (firstEleVecComponents > 3)
                {
                    floatData[i * stride + firstElementOffset + 4 * j + 3] = w1;
                }
                if (firstEleVecComponents > 2)
                {
                    floatData[i * stride + firstElementOffset + 4 * j + 2] = z1;
                }
                if (firstEleVecComponents > 1)
                {
                    floatData[i * stride + firstElementOffset + 4 * j + 1] = y1;
                }
                floatData[i * stride + firstElementOffset + 4 * j] = x1;
            }

            for (GLuint k = 0; k < secondEleVecCount; k++)
            {
                if (secondEleVecComponents > 3)
                {
                    floatData[i * stride + secondElementOffset + 4 * k + 3] = w2;
                }
                if (secondEleVecComponents > 2)
                {
                    floatData[i * stride + secondElementOffset + 4 * k + 2] = z2;
                }
                if (secondEleVecComponents > 1)
                {
                    floatData[i * stride + secondElementOffset + 4 * k + 1] = y2;
                }
                floatData[i * stride + secondElementOffset + 4 * k] = x2;
            }
        }
    }

    void checkResults(const GLColor &firstQuarter,
                      const GLColor &secondQuarter,
                      const GLColor &thirdQuarter,
                      const GLColor &fourthQuarter)
    {
        for (GLuint i = 0; i < kPositionCount; i++)
        {
            if (positionToTest[i][1] >= 0 && positionToTest[i][1] < 32)
            {
                EXPECT_PIXEL_COLOR_EQ(positionToTest[i][0], positionToTest[i][1], firstQuarter);
            }
            else if (positionToTest[i][1] >= 32 && positionToTest[i][1] < 64)
            {
                EXPECT_PIXEL_COLOR_EQ(positionToTest[i][0], positionToTest[i][1], secondQuarter);
            }
            else if (positionToTest[i][1] >= 64 && positionToTest[i][1] < 96)
            {
                EXPECT_PIXEL_COLOR_EQ(positionToTest[i][0], positionToTest[i][1], thirdQuarter);
            }
            else
            {
                EXPECT_PIXEL_COLOR_EQ(positionToTest[i][0], positionToTest[i][1], fourthQuarter);
            }
        }
    }

    GLuint mUniformBuffer;
    GLint64 mMaxUniformBlockSize;
    GLuint mArraySize1;
    GLuint mArraySize2;
    GLuint mDivisor1;
    GLuint mDivisor2;
    GLuint mDivisor3;

    static constexpr GLuint kVectorPerMat                           = 4;
    static constexpr GLuint kFloatPerVector                         = 4;
    static constexpr GLuint kPositionCount                          = 12;
    static constexpr unsigned int positionToTest[kPositionCount][2] = {
        {0, 0},   {75, 0},  {98, 13}, {31, 31}, {0, 32},   {65, 33},
        {23, 54}, {63, 63}, {0, 64},  {43, 86}, {53, 100}, {127, 127}};
};

// Test uniform block whose member is structure type, which contains a mat4 member.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsStruct)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, true, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "struct S { mat4 color;};\n"
        "layout(std140) uniform buffer { S s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = coord.x +  coord.y * 128u;\n"
        "    uint index_x = index / divisor1;\n"
        "    uint index_y = (index % divisor1) / divisor2;\n"
        "    my_FragColor = s[index_x].color[index_y];\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * kVectorPerMat * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);

    setArrayValues(floatData, 0, arraySize, 16, 0, 4, 4, 0.0f, 0.0f, 1.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::blue, GLColor::blue, GLColor::blue, GLColor::blue);

    setArrayValues(floatData, 0, arraySize, 16, 0, 4, 4, 0.0f, 1.0f, 0.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::green, GLColor::green, GLColor::green);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, 16, 0, 4, 4, 1.0f, 0.0f, 0.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::red, GLColor::green, GLColor::green);
}

// Test instanced uniform block whose member is structure type, which contains a mat4 member.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsStructAndInstanced)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, true, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "struct S { mat4 color;};\n"
        "layout(std140) uniform buffer { S s[arraySize]; } instance;\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = coord.x +  coord.y * 128u;\n"
        "    uint index_x = index / divisor1;\n"
        "    uint index_y = (index % divisor1) / divisor2;\n"
        "    my_FragColor = instance.s[index_x].color[index_y];\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * kVectorPerMat * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);

    setArrayValues(floatData, 0, arraySize, 16, 0, 4, 4, 0.0f, 0.0f, 1.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::blue, GLColor::blue, GLColor::blue, GLColor::blue);

    setArrayValues(floatData, 0, arraySize, 16, 0, 4, 4, 0.0f, 1.0f, 0.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::green, GLColor::green, GLColor::green);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, 16, 0, 4, 4, 1.0f, 0.0f, 0.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::red, GLColor::green, GLColor::green);
}

// Test instanced uniform block array whose member is structure type, which contains a mat4 member.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsStructAndInstancedArray)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, true, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "struct S { mat4 color;};\n"
        "layout(std140) uniform buffer { S s[arraySize]; } instance[2];\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = coord.x +  coord.y * 128u;\n"
        "    uint index_x = index / divisor1;\n"
        "    uint index_y = (index % divisor1) / divisor2;\n"
        "    my_FragColor = instance[0].s[index_x].color[index_y] + "
        "instance[1].s[index_x].color[index_y];\n"
        "}\n";

    GLint blockSize0, blockSize1;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex0 = glGetUniformBlockIndex(program, "buffer[0]");
    GLint uniformBufferIndex1 = glGetUniformBlockIndex(program, "buffer[1]");
    glGetActiveUniformBlockiv(program, uniformBufferIndex0, GL_UNIFORM_BLOCK_DATA_SIZE,
                              &blockSize0);
    glGetActiveUniformBlockiv(program, uniformBufferIndex1, GL_UNIFORM_BLOCK_DATA_SIZE,
                              &blockSize1);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize0, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex0, 0);

    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * kVectorPerMat * kFloatPerVector;
    std::vector<GLfloat> floatData0(floatCount, 0.0f);
    std::vector<GLfloat> floatData1(floatCount, 0.0f);

    setArrayValues(floatData0, 0, arraySize, 16, 0, 4, 4, 0.0f, 0.0f, 1.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData0.data());

    GLBuffer uniformBuffer1;
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer1);
    glBufferData(GL_UNIFORM_BUFFER, blockSize1, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uniformBuffer1);
    glUniformBlockBinding(program, uniformBufferIndex0, 1);

    setArrayValues(floatData1, 0, arraySize, 16, 0, 4, 4, 0.0f, 1.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData1.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::cyan, GLColor::cyan, GLColor::cyan, GLColor::cyan);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    setArrayValues(floatData0, 0, arraySize, 16, 0, 4, 4, 1.0f, 0.0f, 0.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData0.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::yellow, GLColor::yellow, GLColor::yellow, GLColor::yellow);

    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer1);
    setArrayValues(floatData1, arraySize / 4, arraySize / 2, 16, 0, 4, 4, 0.0f, 0.0f, 1.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData1.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::yellow, GLColor::magenta, GLColor::yellow, GLColor::yellow);
}

// Test uniform block whose member is structure type, which contains a mat4 member and a float
// member.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsMixStructMat4AndFloat)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, true, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "struct S { mat4 color; float factor; };\n"
        "layout(std140) uniform buffer { S s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = coord.x +  coord.y * 128u;\n"
        "    uint index_x = index / divisor1;\n"
        "    uint index_y = (index % divisor1) / divisor2;\n"
        "    my_FragColor = s[index_x].factor * s[index_x].color[index_y];\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    // The member s is an array of S structures, each element of s should be rounded up
    // to the base alignment of a vec4 according to std140 storage layout rules.
    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * (kVectorPerMat * kFloatPerVector + kFloatPerVector);
    std::vector<GLfloat> floatData(floatCount, 0.0f);
    const size_t strideofFloatCount = kVectorPerMat * kFloatPerVector + kFloatPerVector;

    setArrayValues(floatData, 0, arraySize, strideofFloatCount, 0, 4, 4, 0.0f, 0.0f, 0.5f, 0.5f, 16,
                   1, 1, 2.0f, 0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::blue, GLColor::blue, GLColor::blue, GLColor::blue);

    setArrayValues(floatData, 0, arraySize, strideofFloatCount, 0, 4, 4, 0.0f, 0.5f, 0.0f, 0.5f, 16,
                   1, 1, 2.0f, 0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::green, GLColor::green, GLColor::green);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, strideofFloatCount, 0, 4, 4, 0.5f, 0.0f,
                   0.0f, 0.5f, 16, 1, 1, 2.0f, 0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::red, GLColor::green, GLColor::green);
}

// Test uniform block whose member is structure type, which contains a vec2 member and a vec3
// member.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsMixStructVec2AndVec3)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, false, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "struct S { vec2 color1; vec3 color2; };\n"
        "layout(std140) uniform buffer { S s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = (coord.x +  coord.y * 128u) / divisor;\n"
        "    my_FragColor = vec4(s[index].color1, s[index].color2.xy);\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    // The base alignment of "color2" is vec4.
    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * 2 * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);
    const size_t strideofFloatCount = 2 * kFloatPerVector;

    setArrayValues(floatData, 0, arraySize, strideofFloatCount, 0, 1, 2, 1.0f, 1.0f, 0.0f, 0.0f, 4,
                   1, 3, 1.0f, 1.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::white, GLColor::white, GLColor::white, GLColor::white);

    setArrayValues(floatData, 0, arraySize, strideofFloatCount, 0, 1, 2, 1.0f, 0.0f, 0.0f, 0.0f, 4,
                   1, 3, 0.0f, 1.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::red, GLColor::red, GLColor::red, GLColor::red);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, strideofFloatCount, 0, 1, 2, 0.0f, 0.0f,
                   0.0f, 0.0f, 4, 1, 3, 1.0f, 1.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::red, GLColor::blue, GLColor::red, GLColor::red);
}

// Test uniform block whose member is structure type, which contains a float member and a vec3
// member.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsMixStructFloatAndVec3)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, false, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "struct S { float color1; vec3 color2; };\n"
        "layout(std140) uniform buffer { S s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = (coord.x +  coord.y * 128u) / divisor;\n"
        "    my_FragColor = vec4(s[index].color1, s[index].color2);\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    // The base alignment of "color2" is vec4.
    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * 2 * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);
    const size_t strideofFloatCount = 2 * kFloatPerVector;

    setArrayValues(floatData, 0, arraySize, strideofFloatCount, 0, 1, 1, 1.0f, 0.0f, 0.0f, 0.0f, 4,
                   1, 3, 1.0f, 1.0f, 1.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::white, GLColor::white, GLColor::white, GLColor::white);

    setArrayValues(floatData, 0, arraySize, strideofFloatCount, 0, 1, 1, 1.0f, 0.0f, 0.0f, 0.0f, 4,
                   1, 3, 0.0f, 0.0f, 1.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::red, GLColor::red, GLColor::red, GLColor::red);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, strideofFloatCount, 0, 1, 1, 0.0f, 0.0f,
                   0.0f, 0.0f, 4, 1, 3, 0.0f, 1.0f, 1.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::red, GLColor::blue, GLColor::red, GLColor::red);
}

// Test uniform block whose member is structure type, which contains a vec3 member and a float
// member.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsMixStructVec3AndFloat)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, false, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "struct S { vec3 color1; float color2; };\n"
        "layout(std140) uniform buffer { S s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = (coord.x +  coord.y * 128u) / divisor;\n"
        "    my_FragColor = vec4(s[index].color2, s[index].color1);\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);
    const size_t strideofFloatCount = kFloatPerVector;

    setArrayValues(floatData, 0, arraySize, strideofFloatCount, 0, 1, 3, 1.0f, 1.0f, 1.0f, 0.0f, 3,
                   1, 1, 1.0f, 0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::white, GLColor::white, GLColor::white, GLColor::white);

    setArrayValues(floatData, 0, arraySize, strideofFloatCount, 0, 1, 3, 0.0f, 0.0f, 1.0f, 0.0f, 3,
                   1, 1, 1.0f, 0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::red, GLColor::red, GLColor::red, GLColor::red);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, strideofFloatCount, 0, 1, 3, 0.0f, 1.0f,
                   1.0f, 0.0f, 3, 1, 1, 0.0f, 0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::red, GLColor::blue, GLColor::red, GLColor::red);
}

// Test two uniform blocks with large structure array member are in the same program, and they
// share the same uniform buffer.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, TwoUniformBlocksInSameProgram)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, true, true, true);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "struct S { mat4 color;};\n"
        "layout(std140) uniform buffer1 { S s1[arraySize1]; };\n"
        "layout(std140) uniform buffer2 { S s2[arraySize2]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = coord.x +  coord.y * 128u;\n"
        "    uint index_x1 = index / divisor1;\n"
        "    uint index_y1 = (index % divisor1) / divisor2;\n"
        "    uint index_x2 = coord.x / divisor3;\n"
        "    uint index_y2 = coord.x % 4u;\n"
        "    my_FragColor = s1[index_x1].color[index_y1] + s2[index_x2].color[index_y2];\n"
        "}\n";

    GLint blockSize1, blockSize2;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex1 = glGetUniformBlockIndex(program, "buffer1");
    GLint uniformBufferIndex2 = glGetUniformBlockIndex(program, "buffer2");
    glGetActiveUniformBlockiv(program, uniformBufferIndex1, GL_UNIFORM_BLOCK_DATA_SIZE,
                              &blockSize1);
    glGetActiveUniformBlockiv(program, uniformBufferIndex2, GL_UNIFORM_BLOCK_DATA_SIZE,
                              &blockSize2);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize1 + blockSize2, nullptr, GL_STATIC_DRAW);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUniformBuffer, 0, blockSize2);
    glUniformBlockBinding(program, uniformBufferIndex2, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, mUniformBuffer, blockSize2, blockSize1);
    glUniformBlockBinding(program, uniformBufferIndex1, 1);

    const GLuint arraySize1  = getArraySize();
    const GLuint arraySize2  = getArraySize2();
    const GLuint floatCount1 = arraySize1 * kVectorPerMat * kFloatPerVector;
    const GLuint floatCount2 = arraySize2 * kVectorPerMat * kFloatPerVector;
    const GLuint floatCount  = floatCount1 + floatCount2;
    std::vector<GLfloat> floatData(floatCount, 0.0f);

    setArrayValues(floatData, arraySize2, arraySize1 + arraySize2, 16, 0, 4, 4, 0.0f, 0.0f, 1.0f,
                   1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount2 * sizeof(GLfloat), &floatData[0]);
    glBufferSubData(GL_UNIFORM_BUFFER, blockSize2, floatCount1 * sizeof(GLfloat),
                    &floatData[floatCount2]);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::blue, GLColor::blue, GLColor::blue, GLColor::blue);

    setArrayValues(floatData, 0, arraySize2, 16, 0, 4, 4, 0.0f, 1.0f, 0.0f, 0.0f);
    setArrayValues(floatData, arraySize1 / 4, arraySize2 + arraySize1 / 2, 16, 0, 4, 4, 1.0f, 0.0f,
                   0.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount2 * sizeof(GLfloat), &floatData[0]);
    glBufferSubData(GL_UNIFORM_BUFFER, blockSize2 + floatCount1 * sizeof(GLfloat) / 4,
                    floatCount1 * sizeof(GLfloat) / 4, &floatData[floatCount2 + floatCount1 / 4]);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::cyan, GLColor::yellow, GLColor::cyan, GLColor::cyan);
}

// Test a uniform block with large struct array member and a uniform block with small
// struct array member in different programs, but they share the same uniform buffer.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, TwoUniformBlocksInDiffProgram)
{
    std::ostringstream stream1;
    std::ostringstream stream2;
    generateArraySizeAndDivisorsDeclaration(stream1, false, true, false);
    generateArraySizeAndDivisorsDeclaration(stream2, false, false, false);

    const std::string &kFS1 =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream1.str() +
        "out vec4 my_FragColor;\n"
        "struct S { mat4 color;};\n"
        "layout(std140) uniform buffer { S s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = coord.x +  coord.y * 128u;\n"
        "    uint index_x = index / divisor1;\n"
        "    uint index_y = (index % divisor1) / divisor2;\n"
        "    my_FragColor = s[index_x].color[index_y];\n"
        "}\n";

    const std::string &kFS2 =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream2.str() +
        "out vec4 my_FragColor;\n"
        "struct S { mat4 color;};\n"
        "layout(std140) uniform buffer { S s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index_x = coord.x / divisor;\n"
        "    uint index_y = coord.x % 4u;\n"
        "    my_FragColor = s[index_x].color[index_y];\n"
        "}\n";

    GLint blockSize1, blockSize2;
    ANGLE_GL_PROGRAM(program1, essl3_shaders::vs::Simple(), kFS1.c_str());
    ANGLE_GL_PROGRAM(program2, essl3_shaders::vs::Simple(), kFS2.c_str());
    GLint uniformBufferIndex1 = glGetUniformBlockIndex(program1, "buffer");
    GLint uniformBufferIndex2 = glGetUniformBlockIndex(program2, "buffer");
    glGetActiveUniformBlockiv(program1, uniformBufferIndex1, GL_UNIFORM_BLOCK_DATA_SIZE,
                              &blockSize1);
    glGetActiveUniformBlockiv(program2, uniformBufferIndex2, GL_UNIFORM_BLOCK_DATA_SIZE,
                              &blockSize2);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, std::max(blockSize1, blockSize2), nullptr, GL_STATIC_DRAW);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUniformBuffer, 0, blockSize2);
    glUniformBlockBinding(program2, uniformBufferIndex2, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, mUniformBuffer, 0, blockSize1);
    glUniformBlockBinding(program1, uniformBufferIndex1, 1);

    const GLuint arraySize1  = getArraySize();
    const GLuint arraySize2  = getArraySize2();
    const GLuint floatCount1 = arraySize1 * kVectorPerMat * kFloatPerVector;
    const GLuint floatCount2 = arraySize2 * kVectorPerMat * kFloatPerVector;
    const GLuint floatCount  = floatCount1;
    std::vector<GLfloat> floatData(floatCount, 0.0f);

    setArrayValues(floatData, 0, arraySize1, 16, 0, 4, 4, 0.0f, 0.0f, 1.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), &floatData[0]);
    drawQuad(program1, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::blue, GLColor::blue, GLColor::blue, GLColor::blue);

    setArrayValues(floatData, 0, arraySize2, 16, 0, 4, 4, 0.0f, 1.0f, 0.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount2 * sizeof(GLfloat), &floatData[0]);
    drawQuad(program2, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::green, GLColor::green, GLColor::green);

    setArrayValues(floatData, arraySize2, arraySize2 + arraySize1 / 2, 16, 0, 4, 4, 0.0f, 1.0f,
                   0.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, floatCount2 * sizeof(GLfloat),
                    (floatCount1 / 2 - floatCount2) * sizeof(GLfloat), &floatData[0]);
    drawQuad(program1, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::green, GLColor::blue, GLColor::blue);
}

// Test two uniform blocks share the same uniform buffer. On D3D backend, a uniform
// block with a large array member will be translated to StructuredBuffer, and the
// other uniform block will be translated to cbuffer, this case verifies that update
// buffer data correctly.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, SharedSameBufferWithOtherOne)
{
    ANGLE_SKIP_TEST_IF(IsIntel() && IsMac() && IsOpenGL());

    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, true, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "struct S { mat4 color;};\n"
        "layout(std140) uniform buffer { S s[arraySize]; };\n"
        "layout(std140) uniform buffer1 { vec4 factor; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = coord.x +  coord.y * 128u;\n"
        "    uint index_x = index / divisor1;\n"
        "    uint index_y = (index % divisor1) / divisor2;\n"
        "    my_FragColor = s[index_x].color[index_y] + factor;\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
    GLint uniformBufferIndex1 = glGetUniformBlockIndex(program, "buffer1");
    GLint alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
    while (alignment >= 0 && alignment < 16)
    {
        alignment += alignment;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, alignment + blockSize, nullptr, GL_STATIC_DRAW);

    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * kVectorPerMat * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);
    std::vector<GLfloat> floatData1(4, 0.0f);

    setArrayValues(floatData, 0, arraySize, 16, 0, 4, 4, 0.0f, 0.0f, 0.5f, 0.5f);
    setArrayValues(floatData1, 0, 1, 4, 0, 1, 4, 1.0f, 0.0f, 0.5f, 0.5f);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUniformBuffer, 0, 4 * sizeof(float));
    glUniformBlockBinding(program, uniformBufferIndex1, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, mUniformBuffer, alignment, floatCount * sizeof(float));
    glUniformBlockBinding(program, uniformBufferIndex, 1);

    glBufferSubData(GL_UNIFORM_BUFFER, alignment, floatCount * sizeof(GLfloat), floatData.data());
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * sizeof(GLfloat), floatData1.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::magenta, GLColor::magenta, GLColor::magenta, GLColor::magenta);

    setArrayValues(floatData, 0, arraySize, 16, 0, 4, 4, 0.0f, 0.5f, 0.0f, 0.5f);
    setArrayValues(floatData1, 0, 1, 4, 0, 1, 4, 0.0f, 0.5f, 0.0f, 0.5f);
    glBufferSubData(GL_UNIFORM_BUFFER, alignment, floatCount * sizeof(GLfloat), floatData.data());
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * sizeof(GLfloat), floatData1.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::green, GLColor::green, GLColor::green);
}

// Test indexing accesses uniform block with a large matrix array member correctly.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsMatrixAndIndexAccess)
{
    const char *kFS = R"(#version 300 es
precision mediump float;

uniform uint index;

struct S { uvec4 idx; };

layout(std140) uniform idxbuf { S idxArray[2]; };

layout(std140) uniform buffer1 { mat4 s1[128]; };
layout(std140) uniform buffer2 { mat4 s2[128]; } buf2[2];

out vec4 fragColor;
void main()
{
  fragColor = s1[1][0] + s1[index][1] + s1[idxArray[0].idx.x][idxArray[1].idx.z]
  + buf2[0].s2[1][0] + buf2[1].s2[index][1] + buf2[0].s2[idxArray[0].idx.y][idxArray[1].idx.z]
  + vec4(buf2[1].s2[index][1][index], s1[1][0][2], s1[idxArray[0].idx.x][idxArray[1].idx.z][2],
  buf2[0].s2[idxArray[0].idx.y][idxArray[1].idx.z][3]);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();
}

// Test uniform block whose member is matrix type.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsMatrix)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, true, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "layout(std140) uniform buffer { mat4 s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = coord.x +  coord.y * 128u;\n"
        "    uint index_x = index / divisor1;\n"
        "    uint index_y = (index % divisor1) / divisor2;\n"
        "    my_FragColor = s[index_x][index_y];\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * kVectorPerMat * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);

    setArrayValues(floatData, 0, arraySize, 16, 0, 4, 4, 0.0f, 0.0f, 1.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::blue, GLColor::blue, GLColor::blue, GLColor::blue);

    setArrayValues(floatData, 0, arraySize, 16, 0, 4, 4, 0.0f, 1.0f, 0.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::green, GLColor::green, GLColor::green);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, 16, 0, 4, 4, 1.0f, 0.0f, 0.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::red, GLColor::green, GLColor::green);
}

// Test instanced uniform block whose member is matrix type.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsMatrixAndInstanced)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, true, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "layout(std140) uniform buffer { mat4 s[arraySize]; } instance[2];\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = coord.x +  coord.y * 128u;\n"
        "    uint index_x = index / divisor1;\n"
        "    uint index_y = (index % divisor1) / divisor2;\n"
        "    my_FragColor = instance[0].s[index_x][index_y] + "
        "instance[1].s[index_x][index_y];\n"
        "}\n";

    GLint blockSize0, blockSize1;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex0 = glGetUniformBlockIndex(program, "buffer[0]");
    GLint uniformBufferIndex1 = glGetUniformBlockIndex(program, "buffer[1]");
    glGetActiveUniformBlockiv(program, uniformBufferIndex0, GL_UNIFORM_BLOCK_DATA_SIZE,
                              &blockSize0);
    glGetActiveUniformBlockiv(program, uniformBufferIndex1, GL_UNIFORM_BLOCK_DATA_SIZE,
                              &blockSize1);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize0, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex0, 0);

    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * kVectorPerMat * kFloatPerVector;
    std::vector<GLfloat> floatData0(floatCount, 0.0f);
    std::vector<GLfloat> floatData1(floatCount, 0.0f);

    setArrayValues(floatData0, 0, arraySize, 16, 0, 4, 4, 0.0f, 0.0f, 1.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData0.data());

    GLBuffer uniformBuffer1;
    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer1);
    glBufferData(GL_UNIFORM_BUFFER, blockSize1, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uniformBuffer1);
    glUniformBlockBinding(program, uniformBufferIndex0, 1);

    setArrayValues(floatData1, 0, arraySize, 16, 0, 4, 4, 0.0f, 1.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData1.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::cyan, GLColor::cyan, GLColor::cyan, GLColor::cyan);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    setArrayValues(floatData0, 0, arraySize, 16, 0, 4, 4, 1.0f, 0.0f, 0.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData0.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::yellow, GLColor::yellow, GLColor::yellow, GLColor::yellow);

    glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer1);
    setArrayValues(floatData1, arraySize / 4, arraySize / 2, 16, 0, 4, 4, 0.0f, 0.0f, 1.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData1.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::yellow, GLColor::magenta, GLColor::yellow, GLColor::yellow);
}

// Test uniform block with row major qualifier whose member is matrix type.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsMatrixAndRowMajorQualifier)
{
    // http://anglebug.com/42262481 , http://anglebug.com/40096480
    ANGLE_SKIP_TEST_IF((IsMac() && IsOpenGL()) || IsAndroid() || (IsAMD() && IsOpenGL()) ||
                       (IsLinux() && IsIntel() && IsOpenGL()));

    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, true, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "layout(std140, row_major) uniform buffer { mat4 s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = coord.x +  coord.y * 128u;\n"
        "    uint index_x = index / divisor1;\n"
        "    uint index_y = (index % divisor1) / divisor2;\n"
        "    my_FragColor = s[index_x][index_y];\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * kVectorPerMat * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);

    setArrayValues(floatData, 0, arraySize, 16, 0, 2, 4, 0.0f, 0.0f, 0.0f, 0.0f, 8, 2, 4, 1.0f,
                   1.0f, 1.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::blue, GLColor::blue, GLColor::blue, GLColor::blue);

    setArrayValues(floatData, 0, arraySize, 16, 4, 1, 4, 1.0f, 1.0f, 1.0f, 1.0f, 8, 1, 4, 0.0f,
                   0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::green, GLColor::green, GLColor::green);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, 16, 0, 1, 4, 1.0f, 1.0f, 1.0f, 1.0f, 4,
                   1, 4, 0.0f, 0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::red, GLColor::green, GLColor::green);
}

// Test uniform block whose member is vec4 type.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsVec4)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, false, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "layout(std140) uniform buffer { vec4 s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = (coord.x +  coord.y * 128u) / divisor;\n"
        "    my_FragColor = s[index];\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);

    setArrayValues(floatData, 0, arraySize, 4, 0, 1, 4, 1.0f, 0.0f, 1.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::magenta, GLColor::magenta, GLColor::magenta, GLColor::magenta);

    setArrayValues(floatData, 0, arraySize, 4, 0, 1, 4, 1.0f, 1.0f, 0.0f, 1.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::yellow, GLColor::yellow, GLColor::yellow, GLColor::yellow);
}

// Test uniform block whose member is vec3 type.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsVec3)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, false, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "layout(std140) uniform buffer { vec3 s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = (coord.x +  coord.y * 128u) / divisor;\n"
        "    my_FragColor = vec4(s[index], 1.0);\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);

    setArrayValues(floatData, 0, arraySize, 4, 0, 1, 3, 0.0f, 0.0f, 1.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::blue, GLColor::blue, GLColor::blue, GLColor::blue);

    setArrayValues(floatData, 0, arraySize, 4, 0, 1, 3, 0.0f, 1.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::green, GLColor::green, GLColor::green);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, 4, 0, 1, 3, 1.0f, 0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::red, GLColor::green, GLColor::green);
}

// Test uniform block whose member is vec2 type.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsVec2)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, false, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "layout(std140) uniform buffer { vec2 s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = (coord.x +  coord.y * 128u) / divisor;\n"
        "    my_FragColor = vec4(s[index], s[index].x, 1.0);\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);

    setArrayValues(floatData, 0, arraySize, 4, 0, 1, 2, 1.0f, 0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::magenta, GLColor::magenta, GLColor::magenta, GLColor::magenta);

    setArrayValues(floatData, 0, arraySize, 4, 0, 1, 2, 0.0f, 1.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::green, GLColor::green, GLColor::green);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, 4, 0, 1, 2, 1.0f, 0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::magenta, GLColor::green, GLColor::green);
}

// Test uniform block whose member is float type.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsFloat)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, false, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "layout(std140) uniform buffer { float s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = (coord.x +  coord.y * 128u) / divisor;\n"
        "    my_FragColor = vec4(s[index], 0.0, 0.0, 1.0);\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    // The base alignment and array stride are rounded up to the base alignment of a vec4.
    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);

    setArrayValues(floatData, 0, arraySize, 4, 0, 1, 1, 1.0f, 0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::red, GLColor::red, GLColor::red, GLColor::red);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, 4, 0, 1, 1, 0.0f, 0.0f, 0.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, floatCount * sizeof(GLfloat), floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::red, GLColor::black, GLColor::red, GLColor::red);
}

// Test uniform block whose member is structure type, which contains a float member and a mat4
// member.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsMixStructFloatAndMat4)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, true, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "struct S { float factor; mat4 color; };\n"
        "layout(std140) uniform buffer { S s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = coord.x +  coord.y * 128u;\n"
        "    uint index_x = index / divisor1;\n"
        "    uint index_y = (index % divisor1) / divisor2;\n"
        "    my_FragColor = s[index_x].factor * s[index_x].color[index_y];\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    // The member s is an array of S structures, each element of s should be rounded up
    // to the base alignment of a vec4 according to std140 storage layout rules.
    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * (kVectorPerMat * kFloatPerVector + kFloatPerVector);
    std::vector<GLfloat> floatData(floatCount, 0.0f);
    const size_t strideofFloatCount = kVectorPerMat * kFloatPerVector + kFloatPerVector;

    setArrayValues(floatData, 0, arraySize, strideofFloatCount, 0, 1, 1, 2.0f, 0.0f, 0.0f, 0.0f, 4,
                   4, 4, 0.0f, 0.0f, 0.5f, 0.5f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::blue, GLColor::blue, GLColor::blue, GLColor::blue);

    setArrayValues(floatData, 0, arraySize, strideofFloatCount, 0, 1, 1, 2.0f, 0.0f, 0.0f, 0.0f, 4,
                   4, 4, 0.0f, 0.5f, 0.0f, 0.5f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::green, GLColor::green, GLColor::green);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, strideofFloatCount, 0, 1, 1, 2.0f, 0.0f,
                   0.0f, 0.0f, 4, 4, 4, 0.5f, 0.0f, 0.0f, 0.5f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::green, GLColor::red, GLColor::green, GLColor::green);
}

// Test uniform block whose member is structure type, which contains a float member and a vec4
// member.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberTypeIsMixStructFloatAndVec4)
{
    std::ostringstream stream;
    generateArraySizeAndDivisorsDeclaration(stream, false, false, false);
    const std::string &kFS =
        "#version 300 es\n"
        "precision highp float;\n" +
        stream.str() +
        "out vec4 my_FragColor;\n"
        "struct S { float color1; vec4 color2; };\n"
        "layout(std140) uniform buffer { S s[arraySize]; };\n"
        "void main()\n"
        "{\n"
        "    uvec2 coord = uvec2(floor(gl_FragCoord.xy));\n"
        "    uint index = (coord.x +  coord.y * 128u) / divisor;\n"
        "    my_FragColor = vec4(s[index].color1, s[index].color2.xyz);\n"
        "}\n";

    GLint blockSize;
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS.c_str());
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "buffer");
    glGetActiveUniformBlockiv(program, uniformBufferIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);

    const GLuint arraySize  = getArraySize();
    const GLuint floatCount = arraySize * 2 * kFloatPerVector;
    std::vector<GLfloat> floatData(floatCount, 0.0f);
    const size_t strideofFloatCount = 2 * kFloatPerVector;

    setArrayValues(floatData, 0, arraySize, strideofFloatCount, 0, 1, 1, 1.0f, 0.0f, 0.0f, 0.0f, 4,
                   1, 4, 1.0f, 1.0f, 1.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::white, GLColor::white, GLColor::white, GLColor::white);

    setArrayValues(floatData, 0, arraySize, strideofFloatCount, 0, 1, 1, 1.0f, 0.0f, 0.0f, 0.0f, 4,
                   1, 4, 0.0f, 0.0f, 1.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::red, GLColor::red, GLColor::red, GLColor::red);

    setArrayValues(floatData, arraySize / 4, arraySize / 2, strideofFloatCount, 0, 1, 1, 0.0f, 0.0f,
                   0.0f, 0.0f, 4, 1, 4, 0.0f, 1.0f, 1.0f, 0.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
                    std::min(static_cast<size_t>(blockSize), floatCount * sizeof(GLfloat)),
                    floatData.data());
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    checkResults(GLColor::red, GLColor::blue, GLColor::red, GLColor::red);
}

// Test to transfer a uniform block large array member as an actual parameter to a function.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberAsActualParameter)
{
    ANGLE_SKIP_TEST_IF(IsAdreno());

    constexpr char kVS[] = R"(#version 300 es
layout(location=0) in vec3 a_position;

layout(std140) uniform UBO1{
    mat4x4 buf1[90];
} instance;

layout(std140) uniform UBO2{
    mat4x4 buf2[90];
};

vec4 test(mat4x4[90] para1, mat4x4[90] para2, vec3 pos){
    return para1[0] * para2[0] * vec4(pos, 1.0);
}

void main(void){
    gl_Position = test(instance.buf1, buf2, a_position);
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;

uniform vec3 u_color;
out vec4 oFragColor;

void main(void){
    oFragColor = vec4( u_color, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    EXPECT_GL_NO_ERROR();
}

// Test array operators to operate on uniform block large array member.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, MemberArrayOperations)
{
    ANGLE_SKIP_TEST_IF(IsOpenGL() || IsOpenGLES());

    constexpr char kVS[] = R"(#version 300 es
layout(location=0) in vec3 a_position;

layout(std140) uniform UBO1{
    mat4x4 buf1[90];
};

layout(std140) uniform UBO2{
    mat4x4 buf2[90];
};

layout(std140) uniform UBO3{
    mat4x4 buf[90];
} instance;

vec4 test1( mat4x4[90] para, vec3 pos ){
    return para[ 0 ] * vec4( pos, 1.0 );
}

mat4x4[90] test2()
{
    return instance.buf;
}

void main(void){
    if (buf1 == buf2)
    {
        mat4x4 temp1[90] = buf1;
        gl_Position = test1(temp1, a_position);
    }
    else
    {
        mat4x4 temp2[90] = test2();
        gl_Position = test1(temp2, a_position);
    }
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;

uniform vec3 u_color;
out vec4 oFragColor;

void main(void){
    oFragColor = vec4( u_color, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    EXPECT_GL_NO_ERROR();
}

// Test to throw a warning if a uniform block with a large array member
// fails to hit the optimization on D3D backend.
TEST_P(UniformBlockWithOneLargeArrayMemberTest, ThrowPerfWarningInD3D)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;

struct S1 {
    vec2 a[2];
};

struct S2 {
    mat2x4 b;
};

layout(std140, row_major) uniform UBO1{
    mat3x2 buf1[128];
};

layout(std140, row_major) uniform UBO2{
    mat4x3 buf2[128];
} instance1;

layout(std140, row_major) uniform UBO3{
    S1 buf3[128];
};

layout(std140, row_major) uniform UBO4{
    S2 buf4[128];
} instance2[2];

out vec4 my_FragColor;

void main(void){
    uvec2 coord = uvec2(floor(gl_FragCoord.xy));
    uint x = coord.x % 64u;
    uint y = coord.y;
    my_FragColor = vec4(buf1[y]*instance1.buf2[y]*instance2[0].buf4[y].b*buf3[y].a[x], 0.0f, 1.0);

})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();
}

// Tests rendering with a bound, unreferenced UBO that has no data. Covers a paticular back-end bug.
TEST_P(UniformBufferTest, EmptyUnusedUniformBuffer)
{
    constexpr GLuint kBasicUBOIndex = 0;
    constexpr GLuint kEmptyUBOIndex = 1;

    // Create two UBOs. One is empty and the other is used.
    constexpr GLfloat basicUBOData[4] = {1.0, 2.0, 3.0, 4.0};
    GLBuffer basicUBO;
    glBindBuffer(GL_UNIFORM_BUFFER, basicUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(basicUBOData), basicUBOData, GL_STATIC_READ);
    glBindBufferBase(GL_UNIFORM_BUFFER, kBasicUBOIndex, basicUBO);

    GLBuffer emptyUBO;
    glBindBufferBase(GL_UNIFORM_BUFFER, kEmptyUBOIndex, emptyUBO);

    // Create a simple UBO program.
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform basicBlock {
    vec4 basicVec4;
};

out vec4 outColor;

void main() {
   if (basicVec4 == vec4(1, 2, 3, 4)) {
       outColor = vec4(0, 1, 0, 1);
   } else {
       outColor = vec4(1, 0, 0, 1);
   }
})";

    // Draw and check result. Should not crash.
    ANGLE_GL_PROGRAM(uboProgram, essl3_shaders::vs::Simple(), kFS);
    drawQuad(uboProgram, essl1_shaders::PositionAttrib(), 0.5f, 1.0f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Calling BufferData and use it in a loop to force descriptorSet creation and destroy.
TEST_P(UniformBufferTest, BufferDataInLoop)
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Use large buffer size to get around suballocation, so that we will gets a new buffer with
    // bufferData call.
    static constexpr size_t kBufferSize = 4 * 1024 * 1024;
    std::vector<float> floatData;
    floatData.resize(kBufferSize / (sizeof(float)), 0.0f);
    floatData[0] = 0.5f;
    floatData[1] = 0.75f;
    floatData[2] = 0.25f;
    floatData[3] = 1.0f;

    GLTexture textures[2];
    GLFramebuffer fbos[2];
    for (int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 256, 256);

        glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);
        EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    }

    for (int loop = 0; loop < 10; loop++)
    {
        int i = loop & 0x1;
        // Switch FBO to get around deferred flush
        glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
        glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, kBufferSize, floatData.data(), GL_STATIC_DRAW);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
        glUniformBlockBinding(mProgram, mUniformBufferIndex, 0);
        drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);
        glFlush();
    }
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 128, 191, 64, 255, 1);
}

class UniformBufferMemoryTest : public UniformBufferTest
{
  protected:
    angle::VulkanPerfCounters getPerfCounters()
    {
        if (mIndexMap.empty())
        {
            mIndexMap = BuildCounterNameToIndexMap();
        }

        return GetPerfCounters(mIndexMap);
    }

    CounterNameToIndexMap mIndexMap;
};

// Calling BufferData and drawing with it in a loop without glFlush() should still work. Driver is
// supposedly to issue flush if needed.
TEST_P(UniformBufferMemoryTest, BufferDataInLoopManyTimes)
{
    GLPerfMonitor monitor;
    glBeginPerfMonitorAMD(monitor);

    // Run this test for Vulkan only.
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    uint64_t expectedSubmitCalls = getPerfCounters().commandQueueSubmitCallsTotal + 1;

    glClear(GL_COLOR_BUFFER_BIT);
    constexpr size_t kBufferSize = 64 * 1024 * 1024;
    std::vector<float> floatData;
    floatData.resize(kBufferSize / (sizeof(float)), 0.0f);
    floatData[0] = 0.5f;
    floatData[1] = 0.75f;
    floatData[2] = 0.25f;
    floatData[3] = 1.0f;

    GLTexture texture;
    GLFramebuffer fbo;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 256, 256);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    constexpr uint32_t kIterationCount = 4096;
    for (uint32_t loop = 0; loop < kIterationCount; loop++)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, kBufferSize, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, kBufferSize, floatData.data());

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, mUniformBuffer);
        glUniformBlockBinding(mProgram, mUniformBufferIndex, 0);
        drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);

        if (getPerfCounters().commandQueueSubmitCallsTotal == expectedSubmitCalls)
        {
            break;
        }
    }
    glEndPerfMonitorAMD(monitor);

    EXPECT_EQ(getPerfCounters().commandQueueSubmitCallsTotal, expectedSubmitCalls);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 128, 191, 64, 255, 1);
}

class WebGL2UniformBufferTest : public UniformBufferTest
{
  protected:
    WebGL2UniformBufferTest() { setWebGLCompatibilityEnabled(true); }
};

// Test that ANGLE handles used but unbound UBO. Assumes we are running on ANGLE and produce
// optional but not mandatory errors.
TEST_P(WebGL2UniformBufferTest, ANGLEUnboundUniformBuffer)
{
    glUniformBlockBinding(mProgram, mUniformBufferIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
    EXPECT_GL_NO_ERROR();

    drawQuad(mProgram, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Compile uniform buffer with large array member.
TEST_P(WebGL2UniformBufferTest, LargeArrayOfStructs)
{
    constexpr char kVertexShader[] = R"(
        struct InstancingData
        {
            vec4 transformation;
        };

        layout(std140) uniform InstanceBlock
        {
            InstancingData instances[MAX_INSTANCE_COUNT];
        };

        void main()
        {
            gl_Position = vec4(1.0) * instances[gl_InstanceID].transformation[0];
        })";

    constexpr char kFragmentShader[] = R"(#version 300 es
        precision mediump float;
        out vec4 outFragColor;
        void main()
        {
            outFragColor = vec4(0.0);
        })";

    int maxUniformBlockSize;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);

    std::string vs = "#version 300 es\n#define MAX_INSTANCE_COUNT " +
                     std::to_string(maxUniformBlockSize / 16) + kVertexShader;

    ANGLE_GL_PROGRAM(program, vs.c_str(), kFragmentShader);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(UniformBufferTest);
ANGLE_INSTANTIATE_TEST_ES3(UniformBufferTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(UniformBlockWithOneLargeArrayMemberTest);
ANGLE_INSTANTIATE_TEST_ES3(UniformBlockWithOneLargeArrayMemberTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(UniformBufferTest31);
ANGLE_INSTANTIATE_TEST_ES31(UniformBufferTest31);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(UniformBufferMemoryTest);
ANGLE_INSTANTIATE_TEST_ES3(UniformBufferMemoryTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(WebGL2UniformBufferTest);
ANGLE_INSTANTIATE_TEST_ES3(WebGL2UniformBufferTest);

}  // namespace
