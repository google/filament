//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include <stdint.h>
#include <memory>

#include "common/string_utils.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"

using namespace angle;

class ProgramBinaryTest : public ANGLETest<>
{
  protected:
    ProgramBinaryTest()
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

    void testSetUp() override
    {
        mProgram = CompileProgram(essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        glGenBuffers(1, &mBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferData(GL_ARRAY_BUFFER, 128, nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override
    {
        glDeleteProgram(mProgram);
        glDeleteBuffers(1, &mBuffer);
    }

    GLint getAvailableProgramBinaryFormatCount() const
    {
        GLint formatCount;
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS_OES, &formatCount);
        return formatCount;
    }

    bool supported() const
    {
        if (!IsGLExtensionEnabled("GL_OES_get_program_binary"))
        {
            std::cout << "Test skipped because GL_OES_get_program_binary is not available."
                      << std::endl;
            return false;
        }

        if (getAvailableProgramBinaryFormatCount() == 0)
        {
            std::cout << "Test skipped because no program binary formats are available."
                      << std::endl;
            return false;
        }

        return true;
    }

    void saveAndLoadProgram(GLuint programToSave, GLuint loadedProgram)
    {
        GLint programLength = 0;
        GLint writtenLength = 0;
        GLenum binaryFormat = 0;

        glGetProgramiv(programToSave, GL_PROGRAM_BINARY_LENGTH_OES, &programLength);
        EXPECT_GL_NO_ERROR();

        std::vector<uint8_t> binary(programLength);
        glGetProgramBinaryOES(programToSave, programLength, &writtenLength, &binaryFormat,
                              binary.data());
        EXPECT_GL_NO_ERROR();

        // The lengths reported by glGetProgramiv and glGetProgramBinaryOES should match
        EXPECT_EQ(programLength, writtenLength);

        if (writtenLength)
        {
            glProgramBinaryOES(loadedProgram, binaryFormat, binary.data(), writtenLength);

            EXPECT_GL_NO_ERROR();

            GLint linkStatus;
            glGetProgramiv(loadedProgram, GL_LINK_STATUS, &linkStatus);
            if (linkStatus == 0)
            {
                GLint infoLogLength;
                glGetProgramiv(loadedProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

                if (infoLogLength > 0)
                {
                    std::vector<GLchar> infoLog(infoLogLength);
                    glGetProgramInfoLog(loadedProgram, static_cast<GLsizei>(infoLog.size()),
                                        nullptr, &infoLog[0]);
                    FAIL() << "program link failed: " << &infoLog[0];
                }
                else
                {
                    FAIL() << "program link failed.";
                }
            }
            else
            {
                glUseProgram(loadedProgram);
                glBindBuffer(GL_ARRAY_BUFFER, mBuffer);

                glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8, nullptr);
                glEnableVertexAttribArray(0);
                glDrawArrays(GL_POINTS, 0, 1);

                EXPECT_GL_NO_ERROR();
            }
        }
    }

    GLuint mProgram;
    GLuint mBuffer;
};

// This tests the assumption that float attribs of different size
// should not internally cause a vertex shader recompile (for conversion).
TEST_P(ProgramBinaryTest, FloatDynamicShaderSize)
{
    if (!supported())
    {
        return;
    }

    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8, nullptr);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_POINTS, 0, 1);

    GLint programLength;
    glGetProgramiv(mProgram, GL_PROGRAM_BINARY_LENGTH_OES, &programLength);

    EXPECT_GL_NO_ERROR();

    for (GLsizei size = 1; size <= 3; size++)
    {
        glVertexAttribPointer(0, size, GL_FLOAT, GL_FALSE, 8, nullptr);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_POINTS, 0, 1);

        GLint newProgramLength;
        glGetProgramiv(mProgram, GL_PROGRAM_BINARY_LENGTH_OES, &newProgramLength);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(programLength, newProgramLength);
    }
}

// Tests that switching between signed and unsigned un-normalized data doesn't trigger a bug
// in the D3D11 back-end.
TEST_P(ProgramBinaryTest, DynamicShadersSignatureBug)
{
    glUseProgram(mProgram);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);

    GLint attribLocation = glGetAttribLocation(mProgram, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, attribLocation);
    glEnableVertexAttribArray(attribLocation);

    glVertexAttribPointer(attribLocation, 2, GL_BYTE, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_POINTS, 0, 1);

    glVertexAttribPointer(attribLocation, 2, GL_UNSIGNED_BYTE, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_POINTS, 0, 1);
}

// This tests the ability to successfully save and load a program binary.
TEST_P(ProgramBinaryTest, SaveAndLoadBinary)
{
    if (!supported())
    {
        return;
    }

    GLuint programToLoad = glCreateProgram();

    saveAndLoadProgram(mProgram, programToLoad);

    glDeleteProgram(programToLoad);
    EXPECT_GL_NO_ERROR();
}

// This tests the ability to successfully save and load a program binary and then
// save and load from the same program that was loaded.
TEST_P(ProgramBinaryTest, SaveAndLoadBinaryTwice)
{
    if (!supported())
    {
        return;
    }

    GLuint programToLoad  = glCreateProgram();
    GLuint programToLoad2 = glCreateProgram();

    saveAndLoadProgram(mProgram, programToLoad);
    saveAndLoadProgram(programToLoad, programToLoad2);

    glDeleteProgram(programToLoad);
    glDeleteProgram(programToLoad2);
    EXPECT_GL_NO_ERROR();
}

// Ensures that we init the compiler before calling ProgramBinary.
TEST_P(ProgramBinaryTest, CallProgramBinaryBeforeLink)
{
    if (!supported())
    {
        return;
    }

    // Initialize a simple program.
    glUseProgram(mProgram);

    GLsizei length = 0;
    glGetProgramiv(mProgram, GL_PROGRAM_BINARY_LENGTH, &length);
    ASSERT_GL_NO_ERROR();
    ASSERT_GT(length, 0);

    GLsizei readLength  = 0;
    GLenum binaryFormat = GL_NONE;
    std::vector<uint8_t> binaryBlob(length);
    glGetProgramBinaryOES(mProgram, length, &readLength, &binaryFormat, binaryBlob.data());
    ASSERT_GL_NO_ERROR();

    // Shutdown and restart GL entirely.
    recreateTestFixture();

    ANGLE_GL_BINARY_OES_PROGRAM(binaryProgram, binaryBlob, binaryFormat);
    ASSERT_GL_NO_ERROR();

    drawQuad(binaryProgram, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
}

// Test that unlinked programs have a binary size of 0
TEST_P(ProgramBinaryTest, ZeroSizedUnlinkedBinary)
{
    ANGLE_SKIP_TEST_IF(!supported());

    ANGLE_GL_EMPTY_PROGRAM(program);
    GLsizei length = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &length);
    ASSERT_EQ(0, length);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(
    ProgramBinaryTest,
    ES3_VULKAN().disable(Feature::EnablePipelineCacheDataCompression));

class ProgramBinaryES3Test : public ProgramBinaryTest
{
  protected:
    ProgramBinaryES3Test()
    {
        // Test flakiness was noticed when reusing displays.
        forceNewDisplay();
    }

    void testBinaryAndUBOBlockIndexes(bool drawWithProgramFirst);
};

class ProgramBinaryES31Test : public ANGLETest<>
{
  protected:
    ProgramBinaryES31Test()
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

    GLint getAvailableProgramBinaryFormatCount() const
    {
        GLint formatCount;
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS_OES, &formatCount);
        return formatCount;
    }
};

void ProgramBinaryES3Test::testBinaryAndUBOBlockIndexes(bool drawWithProgramFirst)
{
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    constexpr char kVS[] = R"(#version 300 es
uniform block {
    float f;
};
in vec4 position;
out vec4 color;
void main() {
    gl_Position = position;
    color = vec4(f, f, f, 1);
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
in vec4 color;
out vec4 colorOut;
void main() {
    colorOut = color;
})";

    // Init and draw with the program.
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    float fData[4]   = {1.0f, 1.0f, 1.0f, 1.0f};
    GLuint bindIndex = 2;

    GLBuffer ubo;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(fData), &fData, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, bindIndex, ubo, 0, sizeof(fData));

    GLint blockIndex = glGetUniformBlockIndex(program, "block");
    ASSERT_NE(-1, blockIndex);

    glUniformBlockBinding(program, blockIndex, bindIndex);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    if (drawWithProgramFirst)
    {
        drawQuad(program, "position", 0.5f);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
    }

    // Read back the binary.
    GLint programLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH_OES, &programLength);
    ASSERT_GL_NO_ERROR();

    GLsizei readLength  = 0;
    GLenum binaryFormat = GL_NONE;
    std::vector<uint8_t> binary(programLength);
    glGetProgramBinary(program, programLength, &readLength, &binaryFormat, binary.data());
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(static_cast<GLsizei>(programLength), readLength);

    // Load a new program with the binary and draw.
    ANGLE_GL_BINARY_ES3_PROGRAM(binaryProgram, binary, binaryFormat);

    // Reconfigure the block binding, which is reset after a call to glProgramBinary.
    glUniformBlockBinding(binaryProgram, blockIndex, bindIndex);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    drawQuad(binaryProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Tests that saving and loading a program perserves uniform block binding info.
TEST_P(ProgramBinaryES3Test, UniformBlockBindingWithDraw)
{
    testBinaryAndUBOBlockIndexes(true);
}

// Same as UniformBlockBindingWithDraw, but does not do an initial draw with the program. Covers an
// ANGLE crash.  http://anglebug.com/42260591
TEST_P(ProgramBinaryES3Test, UniformBlockBindingNoDraw)
{
    testBinaryAndUBOBlockIndexes(false);
}

// Same as UniformBlockBindingWithDraw, but specifies the binding in the shader itself.
TEST_P(ProgramBinaryES31Test, UniformBlockBindingSpecifiedInShader)
{
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    constexpr char kVS[] = R"(#version 310 es
layout(binding = 1) uniform block {
    vec4 v;
};
in vec4 position;
out vec4 color;
void main() {
    gl_Position = position;
    color = v;
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
in vec4 color;
out vec4 colorOut;
void main() {
    colorOut = color;
})";

    // Init and draw with the program.
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    constexpr std::array<float, 4> kBlue = {0, 0, 1, 1};
    GLBuffer blue;
    glBindBuffer(GL_UNIFORM_BUFFER, blue);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(kBlue), kBlue.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, blue);

    GLint blockIndex = glGetUniformBlockIndex(program, "block");
    ASSERT_NE(-1, blockIndex);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    // Read back the binary.
    GLint programLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH_OES, &programLength);
    ASSERT_GL_NO_ERROR();

    GLsizei readLength  = 0;
    GLenum binaryFormat = GL_NONE;
    std::vector<uint8_t> binary(programLength);
    glGetProgramBinary(program, programLength, &readLength, &binaryFormat, binary.data());
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(static_cast<GLsizei>(programLength), readLength);

    // Load a new program with the binary and draw.
    ANGLE_GL_BINARY_ES3_PROGRAM(binaryProgram, binary, binaryFormat);

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    drawQuad(binaryProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Ensure that uniform block bindings are reset to their original value on program binary
TEST_P(ProgramBinaryES31Test, UniformBlockBindingResetOnReload)
{
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    constexpr char kVS[] = R"(#version 310 es
layout(binding = 1) uniform block {
    vec4 v;
};
in vec4 position;
out vec4 color;
void main() {
    gl_Position = position;
    color = v;
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
in vec4 color;
out vec4 colorOut;
void main() {
    colorOut = color;
})";

    // Init and draw with the program.
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    constexpr std::array<float, 4> kBlue  = {0, 0, 1, 1};
    constexpr std::array<float, 4> kGreen = {0, 1, 0, 1};
    GLBuffer blue, green;
    glBindBuffer(GL_UNIFORM_BUFFER, blue);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(kBlue), kBlue.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, green);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(kGreen), kGreen.data(), GL_STATIC_DRAW);

    // Bind one buffer to binding 1 and another to binding 2
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, blue);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, green);

    GLint blockIndex = glGetUniformBlockIndex(program, "block");
    ASSERT_NE(-1, blockIndex);

    // Initially, remap the block to binding 2
    glUniformBlockBinding(program, blockIndex, 2);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Read back the binary.
    GLint programLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH_OES, &programLength);
    ASSERT_GL_NO_ERROR();

    GLsizei readLength  = 0;
    GLenum binaryFormat = GL_NONE;
    std::vector<uint8_t> binary(programLength);
    glGetProgramBinary(program, programLength, &readLength, &binaryFormat, binary.data());
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(static_cast<GLsizei>(programLength), readLength);

    // Load a new program with the binary and draw.  On reset, the binding should be reset back to 1
    ANGLE_GL_BINARY_ES3_PROGRAM(binaryProgram, binary, binaryFormat);

    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(binaryProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Test the shaders with arrays-of-struct uniforms are properly saved and restored
TEST_P(ProgramBinaryES3Test, TestArrayOfStructUniform)
{
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    constexpr char kVS[] =
        "#version 300 es\n"
        "in highp vec4 position;\n"
        "out mediump float v_vtxOut;\n"
        "\n"
        "struct structType\n"
        "{\n"
        "    mediump vec4 m0;\n"
        "    mediump vec4 m1;\n"
        "};\n"
        "uniform structType u_var[3];\n"
        "\n"
        "mediump float compare_float(mediump float a, mediump float b)\n"
        "{\n"
        "    return abs(a - b) < 0.05 ? 1.0 : 0.0;\n"
        "}\n"
        "mediump float compare_vec4(mediump vec4 a, mediump vec4 b)\n"
        "{\n"
        "    return compare_float(a.x, b.x)*compare_float(a.y, b.y)*\n"
        "           compare_float(a.z, b.z)*compare_float(a.w, b.w);\n"
        "}\n"
        "\n"
        "void main (void)\n"
        "{\n"
        "    gl_Position = position;\n"
        "    v_vtxOut = 1.0;\n"
        "    v_vtxOut *= compare_vec4(u_var[0].m0, vec4(0.15, 0.52, 0.26, 0.35));\n"
        "    v_vtxOut *= compare_vec4(u_var[0].m1, vec4(0.88, 0.09, 0.30, 0.61));\n"
        "    v_vtxOut *= compare_vec4(u_var[1].m0, vec4(0.85, 0.59, 0.33, 0.71));\n"
        "    v_vtxOut *= compare_vec4(u_var[1].m1, vec4(0.62, 0.89, 0.09, 0.99));\n"
        "    v_vtxOut *= compare_vec4(u_var[2].m0, vec4(0.53, 0.89, 0.01, 0.08));\n"
        "    v_vtxOut *= compare_vec4(u_var[2].m1, vec4(0.26, 0.72, 0.60, 0.12));\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "in mediump float v_vtxOut;\n"
        "\n"
        "layout(location = 0) out mediump vec4 dEQP_FragColor;\n"
        "\n"
        "void main (void)\n"
        "{\n"
        "    mediump float result = v_vtxOut;\n"
        "    dEQP_FragColor = vec4(result, result, result, 1.0);\n"
        "}";

    // Init and draw with the program.
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glUseProgram(program);

    int location = glGetUniformLocation(program, "u_var[0].m0");
    ASSERT_NE(location, -1);
    glUniform4f(location, 0.15, 0.52, 0.26, 0.35);
    location = glGetUniformLocation(program, "u_var[0].m1");
    ASSERT_NE(location, -1);
    glUniform4f(location, 0.88, 0.09, 0.30, 0.61);
    location = glGetUniformLocation(program, "u_var[1].m0");
    ASSERT_NE(location, -1);
    glUniform4f(location, 0.85, 0.59, 0.33, 0.71);
    location = glGetUniformLocation(program, "u_var[1].m1");
    ASSERT_NE(location, -1);
    glUniform4f(location, 0.62, 0.89, 0.09, 0.99);
    location = glGetUniformLocation(program, "u_var[2].m0");
    ASSERT_NE(location, -1);
    glUniform4f(location, 0.53, 0.89, 0.01, 0.08);
    location = glGetUniformLocation(program, "u_var[2].m1");
    ASSERT_NE(location, -1);
    glUniform4f(location, 0.26, 0.72, 0.60, 0.12);
    ASSERT_GL_NO_ERROR();

    // Clear and draw with the original program:
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    // Read back the binary.
    GLint programLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH_OES, &programLength);
    ASSERT_GL_NO_ERROR();

    GLsizei readLength  = 0;
    GLenum binaryFormat = GL_NONE;
    std::vector<uint8_t> binary(programLength);
    glGetProgramBinary(program, programLength, &readLength, &binaryFormat, binary.data());
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(static_cast<GLsizei>(programLength), readLength);

    // Load a new program with the binary and draw.
    ANGLE_GL_BINARY_ES3_PROGRAM(binaryProgram, binary, binaryFormat);

    glUseProgram(binaryProgram);

    location = glGetUniformLocation(binaryProgram, "u_var[0].m0");
    ASSERT_NE(location, -1);
    glUniform4f(location, 0.15, 0.52, 0.26, 0.35);
    location = glGetUniformLocation(binaryProgram, "u_var[0].m1");
    ASSERT_NE(location, -1);
    glUniform4f(location, 0.88, 0.09, 0.30, 0.61);
    location = glGetUniformLocation(binaryProgram, "u_var[1].m0");
    ASSERT_NE(location, -1);
    glUniform4f(location, 0.85, 0.59, 0.33, 0.71);
    location = glGetUniformLocation(binaryProgram, "u_var[1].m1");
    ASSERT_NE(location, -1);
    glUniform4f(location, 0.62, 0.89, 0.09, 0.99);
    location = glGetUniformLocation(binaryProgram, "u_var[2].m0");
    ASSERT_NE(location, -1);
    glUniform4f(location, 0.53, 0.89, 0.01, 0.08);
    location = glGetUniformLocation(binaryProgram, "u_var[2].m1");
    ASSERT_NE(location, -1);
    glUniform4f(location, 0.26, 0.72, 0.60, 0.12);
    ASSERT_GL_NO_ERROR();

    // Clear and draw with the restored program:
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    drawQuad(binaryProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Verify that saving/loading binary with detached shaders followed by indexed
// drawing works.
TEST_P(ProgramBinaryES3Test, SaveAndLoadDetachedShaders)
{
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    // We use shaders with the "flat" qualifier, to ensure that "flat" behaves
    // across save/load. This is primarily to catch possible bugs in the Metal
    // backend, where it needs to detect "flat" at shader link time and
    // preserve that detection across binary save/load.
    constexpr char kVS[] = R"(#version 300 es
precision highp float;

in vec4 a_position;
flat out vec4 v_color;

const vec4 colors[4] = vec4[](
    vec4(0.0f, 0.0f, 1.0f, 1.0f),
    vec4(0.0f, 0.0f, 1.0f, 1.0f),
    vec4(0.0f, 1.0f, 0.0f, 1.0f),
    vec4(0.0f, 1.0f, 0.0f, 1.0f)
);

void main()
{
    v_color = colors[gl_VertexID];
    gl_Position = a_position;
}
)";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;

flat in vec4 v_color;
out vec4 o_color;

void main()
{
    o_color = v_color;
}
)";

    const auto &vertices = GetIndexedQuadVertices();

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    GLBuffer indexBuffer;
    const auto &indices = GetQuadIndices();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(),
                 GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    GLuint vertexShader   = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    ASSERT_NE(0u, vertexShader);
    ASSERT_NE(0u, fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(GL_TRUE, linkStatus);

    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    ASSERT_GL_NO_ERROR();

    GLuint loadedProgram = glCreateProgram();
    saveAndLoadProgram(program, loadedProgram);
    glDeleteProgram(program);

    ASSERT_EQ(glIsProgram(loadedProgram), GL_TRUE);
    glUseProgram(loadedProgram);
    ASSERT_GL_NO_ERROR();

    GLint posLocation = glGetAttribLocation(loadedProgram, "a_position");
    ASSERT_NE(-1, posLocation);

    GLVertexArray vertexArray;
    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posLocation);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    glDeleteProgram(loadedProgram);
    ASSERT_GL_NO_ERROR();
}

// Tests the difference between uniform static and active use
TEST_P(ProgramBinaryES3Test, ActiveUniformShader)
{
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 position;\n"
        "void main() {\n"
        "    gl_Position = position;\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform float values[2];\n"
        "out vec4 color;\n"
        "bool isZero(float value) {\n"
        "    return value == 0.0f;\n"
        "}\n"
        "void main() {\n"
        "    color = isZero(values[1]) ? vec4(1.0f,0.0f,0.0f,1.0f) : vec4(0.0f,1.0f,0.0f,1.0f);\n"
        "}";

    // Init and draw with the program.
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    GLint valuesLoc = glGetUniformLocation(program, "values");
    ASSERT_NE(-1, valuesLoc);

    glUseProgram(program);
    GLfloat values[2] = {0.5f, 1.0f};
    glUniform1fv(valuesLoc, 2, values);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Read back the binary.
    GLint programLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH_OES, &programLength);
    ASSERT_GL_NO_ERROR();

    GLsizei readLength  = 0;
    GLenum binaryFormat = GL_NONE;
    std::vector<uint8_t> binary(programLength);
    glGetProgramBinary(program, programLength, &readLength, &binaryFormat, binary.data());
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(static_cast<GLsizei>(programLength), readLength);

    // Load a new program with the binary and draw.
    ANGLE_GL_BINARY_ES3_PROGRAM(binaryProgram, binary, binaryFormat);

    valuesLoc = glGetUniformLocation(program, "values");
    ASSERT_NE(-1, valuesLoc);

    glUseProgram(binaryProgram);
    GLfloat values2[2] = {0.1f, 1.0f};
    glUniform1fv(valuesLoc, 2, values2);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    drawQuad(binaryProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that uses many uniforms in the shaders
TEST_P(ProgramBinaryES3Test, BinaryWithLargeUniformCount)
{
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    constexpr char kVS[] =
        "#version 300 es\n"
        "uniform float redVS; \n"
        "uniform block0 {\n"
        "    float val0;\n"
        "};\n"
        "uniform float greenVS; \n"
        "uniform float blueVS; \n"
        "in vec4 position;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "    gl_Position = position;\n"
        "    color = vec4(redVS + val0, greenVS, blueVS, 1.0f);\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform float redFS; \n"
        "uniform float greenFS; \n"
        "uniform block1 {\n"
        "    float val1;\n"
        "    float val2;\n"
        "};\n"
        "uniform float blueFS; \n"
        "in vec4 color;\n"
        "out vec4 colorOut;\n"
        "void main() {\n"
        "    colorOut = vec4(color.r + redFS,\n"
        "                    color.g + greenFS + val1,\n"
        "                    color.b + blueFS + val2, \n"
        "                    color.a);\n"
        "}";

    // Init and draw with the program.
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    float block0Data[4] = {-0.7f, 1.0f, 1.0f, 1.0f};
    float block1Data[4] = {0.4f, -0.8f, 1.0f, 1.0f};
    GLuint bindIndex0   = 5;
    GLuint bindIndex1   = 2;

    GLBuffer ubo0;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo0);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(block0Data), &block0Data, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, bindIndex0, ubo0, 0, sizeof(block0Data));
    ASSERT_GL_NO_ERROR();

    GLBuffer ubo1;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo1);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(block1Data), &block1Data, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, bindIndex1, ubo1, 0, sizeof(block1Data));
    ASSERT_GL_NO_ERROR();

    GLint block0Index = glGetUniformBlockIndex(program, "block0");
    ASSERT_NE(-1, block0Index);

    GLint block1Index = glGetUniformBlockIndex(program, "block1");
    ASSERT_NE(-1, block1Index);

    glUniformBlockBinding(program, block0Index, bindIndex0);
    glUniformBlockBinding(program, block1Index, bindIndex1);
    ASSERT_GL_NO_ERROR();

    GLint redVSLoc = glGetUniformLocation(program, "redVS");
    ASSERT_NE(-1, redVSLoc);
    GLint greenVSLoc = glGetUniformLocation(program, "greenVS");
    ASSERT_NE(-1, greenVSLoc);
    GLint blueVSLoc = glGetUniformLocation(program, "blueVS");
    ASSERT_NE(-1, blueVSLoc);
    GLint redFSLoc = glGetUniformLocation(program, "redFS");
    ASSERT_NE(-1, redFSLoc);
    GLint greenFSLoc = glGetUniformLocation(program, "greenFS");
    ASSERT_NE(-1, greenFSLoc);
    GLint blueFSLoc = glGetUniformLocation(program, "blueFS");
    ASSERT_NE(-1, blueFSLoc);

    glUseProgram(program);
    glUniform1f(redVSLoc, 0.6f);
    glUniform1f(greenVSLoc, 0.2f);
    glUniform1f(blueVSLoc, 1.1f);
    glUniform1f(redFSLoc, 0.1f);
    glUniform1f(greenFSLoc, 0.4f);
    glUniform1f(blueFSLoc, 0.7f);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::cyan);

    // Read back the binary.
    GLint programLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH_OES, &programLength);
    ASSERT_GL_NO_ERROR();

    GLsizei readLength  = 0;
    GLenum binaryFormat = GL_NONE;
    std::vector<uint8_t> binary(programLength);
    glGetProgramBinary(program, programLength, &readLength, &binaryFormat, binary.data());
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(static_cast<GLsizei>(programLength), readLength);

    // Load a new program with the binary and draw.
    ANGLE_GL_BINARY_ES3_PROGRAM(binaryProgram, binary, binaryFormat);

    glUniformBlockBinding(binaryProgram, block0Index, bindIndex0);
    glUniformBlockBinding(binaryProgram, block1Index, bindIndex1);

    redVSLoc = glGetUniformLocation(binaryProgram, "redVS");
    ASSERT_NE(-1, redVSLoc);
    greenVSLoc = glGetUniformLocation(binaryProgram, "greenVS");
    ASSERT_NE(-1, greenVSLoc);
    blueVSLoc = glGetUniformLocation(binaryProgram, "blueVS");
    ASSERT_NE(-1, blueVSLoc);
    redFSLoc = glGetUniformLocation(binaryProgram, "redFS");
    ASSERT_NE(-1, redFSLoc);
    greenFSLoc = glGetUniformLocation(binaryProgram, "greenFS");
    ASSERT_NE(-1, greenFSLoc);
    blueFSLoc = glGetUniformLocation(binaryProgram, "blueFS");
    ASSERT_NE(-1, blueFSLoc);

    glUseProgram(binaryProgram);
    glUniform1f(redVSLoc, 0.2f);
    glUniform1f(greenVSLoc, -0.6f);
    glUniform1f(blueVSLoc, 1.0f);
    glUniform1f(redFSLoc, 1.5f);
    glUniform1f(greenFSLoc, 0.2f);
    glUniform1f(blueFSLoc, 0.8f);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    drawQuad(binaryProgram, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
}

// Test that sampler texelFetch references are saved and loaded correctly
TEST_P(ProgramBinaryTest, SRGBDecodeWithSamplerAndTexelFetchTest)
{
    if (!supported())
    {
        return;
    }

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_sRGB_decode") ||
                       getClientMajorVersion() < 3);

    // These OpenGL drivers appear not to respect the texelFetch exception
    // http://anglebug.com/42263564
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsIntel() && IsWindows());
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsAMD() && IsWindows());
    ANGLE_SKIP_TEST_IF(IsOpenGL() && (IsNVIDIA() || IsARM64()) && IsMac());
    ANGLE_SKIP_TEST_IF(IsOpenGLES() && IsNexus5X());

    constexpr char kVS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec4 position;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(position.xy, 0.0, 1.0);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "uniform sampler2D tex;\n"
        "in vec2 texcoord;\n"
        "out vec4 out_color;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   out_color = texelFetch(tex, ivec2(0, 0), 0);\n"
        "}\n";

    GLProgram program;
    program.makeRaster(kVS, kFS);
    ASSERT_NE(0u, program);

    GLuint reloadedProgram = glCreateProgram();
    saveAndLoadProgram(program, reloadedProgram);

    GLint textureLocation = glGetUniformLocation(reloadedProgram, "tex");
    ASSERT_NE(-1, textureLocation);

    GLColor linearColor(64, 127, 191, 255);
    GLColor srgbColor(13, 54, 133, 255);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 &linearColor);
    ASSERT_GL_NO_ERROR();

    GLSampler sampler;
    glBindSampler(0, sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_SRGB_DECODE_EXT, GL_DECODE_EXT);

    glUseProgram(reloadedProgram);
    glUniform1i(textureLocation, 0);

    glDisable(GL_DEPTH_TEST);
    drawQuad(reloadedProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);

    glSamplerParameteri(sampler, GL_TEXTURE_SRGB_DECODE_EXT, GL_SKIP_DECODE_EXT);
    drawQuad(reloadedProgram, "position", 0.5f);

    EXPECT_PIXEL_COLOR_NEAR(0, 0, srgbColor, 1.0);

    glDeleteProgram(reloadedProgram);
}

// Test that array of structs containing array of samplers work as expected.
TEST_P(ProgramBinaryES3Test, ArrayOfStructContainingArrayOfSamplers)
{
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    constexpr char kFS[] =
        "precision mediump float;\n"
        "struct Data { mediump sampler2D data[2]; };\n"
        "uniform Data test[2];\n"
        "void main() {\n"
        "    gl_FragColor = vec4(texture2D(test[1].data[1], vec2(0.0, 0.0)).r,\n"
        "                        texture2D(test[1].data[0], vec2(0.0, 0.0)).r,\n"
        "                        texture2D(test[0].data[1], vec2(0.0, 0.0)).r,\n"
        "                        texture2D(test[0].data[0], vec2(0.0, 0.0)).r);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture textures[4];
    GLColor expected = MakeGLColor(32, 64, 96, 255);
    GLubyte data[8]  = {};  // 4 bytes of padding, so that texture can be initialized with 4 bytes
    memcpy(data, expected.data(), sizeof(expected));
    for (int i = 0; i < 4; i++)
    {
        int outerIdx = i % 2;
        int innerIdx = i / 2;
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        // Each element provides two components.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data + i);
        std::stringstream uniformName;
        uniformName << "test[" << innerIdx << "].data[" << outerIdx << "]";
        // Then send it as a uniform
        GLint uniformLocation = glGetUniformLocation(program, uniformName.str().c_str());
        // The uniform should be active.
        EXPECT_NE(uniformLocation, -1);

        glUniform1i(uniformLocation, 3 - i);
        ASSERT_GL_NO_ERROR();
    }

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, expected);

    // Read back the binary.
    GLint programLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &programLength);
    ASSERT_GL_NO_ERROR();

    GLsizei readLength  = 0;
    GLenum binaryFormat = GL_NONE;
    std::vector<uint8_t> binary(programLength);
    glGetProgramBinary(program, programLength, &readLength, &binaryFormat, binary.data());
    ASSERT_GL_NO_ERROR();

    // Load a new program with the binary.
    ANGLE_GL_BINARY_ES3_PROGRAM(binaryProgram, binary, binaryFormat);
    glUseProgram(binaryProgram);

    for (int i = 0; i < 4; i++)
    {
        int outerIdx = i % 2;
        int innerIdx = i / 2;
        std::stringstream uniformName;
        uniformName << "test[" << innerIdx << "].data[" << outerIdx << "]";
        // Then send it as a uniform
        GLint uniformLocation = glGetUniformLocation(program, uniformName.str().c_str());
        // The uniform should be active.
        EXPECT_NE(uniformLocation, -1);

        glUniform1i(uniformLocation, 3 - i);
        ASSERT_GL_NO_ERROR();
    }

    // Verify the uniform data with the binary program.
    drawQuad(binaryProgram, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, expected);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProgramBinaryES3Test);
ANGLE_INSTANTIATE_TEST_ES3(ProgramBinaryES3Test);

// Tests that saving and loading a program attached with computer shader.
TEST_P(ProgramBinaryES31Test, ProgramBinaryWithComputeShader)
{
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    constexpr char kCS[] =
        "#version 310 es\n"
        "layout(local_size_x=4, local_size_y=3, local_size_z=1) in;\n"
        "uniform block {\n"
        "    vec2 f;\n"
        "};\n"
        "uniform vec2 g;\n"
        "uniform highp sampler2D tex;\n"
        "void main() {\n"
        "    vec4 color = texture(tex, f + g);\n"
        "}";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    // Read back the binary.
    GLint programLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &programLength);
    ASSERT_GL_NO_ERROR();

    GLsizei readLength  = 0;
    GLenum binaryFormat = GL_NONE;
    std::vector<uint8_t> binary(programLength);
    glGetProgramBinary(program, programLength, &readLength, &binaryFormat, binary.data());
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(static_cast<GLsizei>(programLength), readLength);

    // Load a new program with the binary.
    ANGLE_GL_BINARY_ES3_PROGRAM(binaryProgram, binary, binaryFormat);
    ASSERT_GL_NO_ERROR();

    // Dispatch compute with the loaded binary program
    glUseProgram(binaryProgram);
    glDispatchCompute(8, 4, 2);
    ASSERT_GL_NO_ERROR();
}

// Tests saving and loading a separable program that has a computer shader using a uniform and a
// uniform block.
TEST_P(ProgramBinaryES31Test, SeparableProgramLinkedUniforms)
{
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    constexpr char kComputeShader[] = R"(#version 310 es
layout(local_size_x=4, local_size_y=3, local_size_z=1) in;
uniform float testUint;
uniform TestBlock
{
    mat4 testMatrix;
};
layout(rgba32ui) uniform highp writeonly uimage2D imageOut;
void main() {
    imageStore(imageOut, ivec2(gl_LocalInvocationIndex, 0), uvec4(testMatrix[0][0] + testUint, 0, 0, 0));
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);
    glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);

    // Read back the binary.
    GLint programLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &programLength);
    ASSERT_GL_NO_ERROR();

    GLsizei readLength  = 0;
    GLenum binaryFormat = GL_NONE;
    std::vector<uint8_t> binary(programLength);
    glGetProgramBinary(program, programLength, &readLength, &binaryFormat, binary.data());
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(static_cast<GLsizei>(programLength), readLength);

    // Load a new program with the binary.
    ANGLE_GL_BINARY_ES3_PROGRAM(binaryProgram, binary, binaryFormat);
    ASSERT_GL_NO_ERROR();

    glUseProgram(binaryProgram);
    ASSERT_GL_NO_ERROR();
}

// Tests that saving and loading a program attached with computer shader.
TEST_P(ProgramBinaryES31Test, ProgramBinaryWithAtomicCounterComputeShader)
{
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    constexpr char kComputeShader[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(binding = 0, offset = 4) uniform atomic_uint ac[2];
void main() {
    atomicCounterIncrement(ac[0]);
    atomicCounterDecrement(ac[1]);
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);

    // Read back the binary.
    GLint programLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &programLength);
    ASSERT_GL_NO_ERROR();

    GLsizei readLength  = 0;
    GLenum binaryFormat = GL_NONE;
    std::vector<uint8_t> binary(programLength);
    glGetProgramBinary(program, programLength, &readLength, &binaryFormat, binary.data());
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(static_cast<GLsizei>(programLength), readLength);

    // Load a new program with the binary.
    ANGLE_GL_BINARY_ES3_PROGRAM(binaryProgram, binary, binaryFormat);
    ASSERT_GL_NO_ERROR();

    // Dispatch compute with the loaded binary program
    glUseProgram(binaryProgram);

    // The initial value of 'ac[0]' is 3u, 'ac[1]' is 1u.
    unsigned int bufferData[3] = {11u, 3u, 1u};
    GLBuffer atomicCounterBuffer;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    void *mappedBuffer =
        glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint) * 3, GL_MAP_READ_BIT);
    memcpy(bufferData, mappedBuffer, sizeof(bufferData));
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

    EXPECT_EQ(11u, bufferData[0]);
    EXPECT_EQ(4u, bufferData[1]);
    EXPECT_EQ(0u, bufferData[2]);
    ASSERT_GL_NO_ERROR();
}

// Tests that image texture works correctly when loading a program from binary.
TEST_P(ProgramBinaryES31Test, ImageTextureBinding)
{
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    const char kComputeShader[] =
        R"(#version 310 es
        layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
        layout(r32ui, binding = 1) writeonly uniform highp uimage2D writeImage;
        void main()
        {
            imageStore(writeImage, ivec2(gl_LocalInvocationID.xy), uvec4(200u));
        })";

    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);

    // Read back the binary.
    GLint programLength = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &programLength);
    ASSERT_GL_NO_ERROR();

    GLsizei readLength  = 0;
    GLenum binaryFormat = GL_NONE;
    std::vector<uint8_t> binary(programLength);
    glGetProgramBinary(program, programLength, &readLength, &binaryFormat, binary.data());
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(static_cast<GLsizei>(programLength), readLength);

    // Load a new program with the binary.
    ANGLE_GL_BINARY_ES3_PROGRAM(binaryProgram, binary, binaryFormat);
    ASSERT_GL_NO_ERROR();

    // Dispatch compute with the loaded binary program
    glUseProgram(binaryProgram);
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
    constexpr GLuint kInputValue = 100u;
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &kInputValue);
    EXPECT_GL_NO_ERROR();

    glBindImageTexture(1, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    GLuint outputValue;
    glReadPixels(0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &outputValue);
    EXPECT_EQ(200u, outputValue);
    ASSERT_GL_NO_ERROR();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProgramBinaryES31Test);
ANGLE_INSTANTIATE_TEST_ES31(ProgramBinaryES31Test);

class ProgramBinaryTransformFeedbackTest : public ANGLETest<>
{
  protected:
    ProgramBinaryTransformFeedbackTest()
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
        constexpr char kVS[] = R"(#version 300 es
in vec4 inputAttribute;
out vec4 outputVarying;
void main()
{
    outputVarying = inputAttribute;
})";

        constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 outputColor;
void main()
{
    outputColor = vec4(1,0,0,1);
})";

        std::vector<std::string> transformFeedbackVaryings;
        transformFeedbackVaryings.push_back("outputVarying");

        mProgram = CompileProgramWithTransformFeedback(kVS, kFS, transformFeedbackVaryings,
                                                       GL_SEPARATE_ATTRIBS);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        ASSERT_GL_NO_ERROR();
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    GLint getAvailableProgramBinaryFormatCount() const
    {
        GLint formatCount;
        glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS_OES, &formatCount);
        return formatCount;
    }

    GLuint mProgram;
};

// This tests the assumption that float attribs of different size
// should not internally cause a vertex shader recompile (for conversion).
TEST_P(ProgramBinaryTransformFeedbackTest, GetTransformFeedbackVarying)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_get_program_binary"));

    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    // http://anglebug.com/42262347
    ANGLE_SKIP_TEST_IF(IsAndroid() && (IsPixel2() || IsPixel2XL()) && IsVulkan());
    // http://anglebug.com/40096654
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    std::vector<uint8_t> binary(0);
    GLint programLength = 0;
    GLint writtenLength = 0;
    GLenum binaryFormat = 0;

    // Save the program binary out
    glGetProgramiv(mProgram, GL_PROGRAM_BINARY_LENGTH_OES, &programLength);
    ASSERT_GL_NO_ERROR();
    binary.resize(programLength);
    glGetProgramBinaryOES(mProgram, programLength, &writtenLength, &binaryFormat, binary.data());
    ASSERT_GL_NO_ERROR();

    glDeleteProgram(mProgram);

    // Load program binary
    mProgram = glCreateProgram();
    glProgramBinaryOES(mProgram, binaryFormat, binary.data(), writtenLength);

    // Ensure the loaded binary is linked
    GLint linkStatus;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linkStatus);
    EXPECT_TRUE(linkStatus != 0);

    // Query information about the transform feedback varying
    char varyingName[64];
    GLsizei varyingSize = 0;
    GLenum varyingType  = GL_NONE;

    glGetTransformFeedbackVarying(mProgram, 0, 64, &writtenLength, &varyingSize, &varyingType,
                                  varyingName);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(13, writtenLength);
    EXPECT_STREQ("outputVarying", varyingName);
    EXPECT_EQ(1, varyingSize);
    EXPECT_GLENUM_EQ(GL_FLOAT_VEC4, varyingType);

    EXPECT_GL_NO_ERROR();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProgramBinaryTransformFeedbackTest);
ANGLE_INSTANTIATE_TEST_ES3(ProgramBinaryTransformFeedbackTest);

// For the ProgramBinariesAcrossPlatforms tests, we need two sets of params:
// - a set to save the program binary
// - a set to load the program binary
// We combine these into one struct extending PlatformParameters so we can reuse existing ANGLE test
// macros
struct PlatformsWithLinkResult : PlatformParameters
{
    PlatformsWithLinkResult(PlatformParameters saveParams,
                            PlatformParameters loadParamsIn,
                            bool expectedLinkResultIn)
    {
        majorVersion       = saveParams.majorVersion;
        minorVersion       = saveParams.minorVersion;
        eglParameters      = saveParams.eglParameters;
        loadParams         = loadParamsIn;
        expectedLinkResult = expectedLinkResultIn;
    }

    PlatformParameters loadParams;
    bool expectedLinkResult;
};

// Provide a custom gtest parameter name function for PlatformsWithLinkResult
// to avoid returning the same parameter name twice. Such a conflict would happen
// between ES2_D3D11_to_ES2D3D11 and ES2_D3D11_to_ES3D3D11 as they were both
// named ES2_D3D11
std::ostream &operator<<(std::ostream &stream, const PlatformsWithLinkResult &platform)
{
    const PlatformParameters &platform1 = platform;
    const PlatformParameters &platform2 = platform.loadParams;
    stream << platform1 << "_to_" << platform2;
    return stream;
}

class ProgramBinariesAcrossPlatforms : public testing::TestWithParam<PlatformsWithLinkResult>
{
  public:
    void SetUp() override
    {
        mOSWindow   = OSWindow::New();
        bool result = mOSWindow->initialize("ProgramBinariesAcrossRenderersTests", 100, 100);

        if (result == false)
        {
            FAIL() << "Failed to create OS window";
        }

        mEntryPointsLib.reset(
            angle::OpenSharedLibrary(ANGLE_EGL_LIBRARY_NAME, angle::SearchType::ModuleDir));
    }

    EGLWindow *createAndInitEGLWindow(angle::PlatformParameters &param)
    {
        EGLWindow *eglWindow = EGLWindow::New(param.majorVersion, param.minorVersion);
        ConfigParameters configParams;
        bool result = eglWindow->initializeGL(mOSWindow, mEntryPointsLib.get(), param.driver,
                                              param.eglParameters, configParams);
        if (!result)
        {
            EGLWindow::Delete(&eglWindow);
        }

        LoadUtilGLES(eglGetProcAddress);

        return eglWindow;
    }

    void destroyEGLWindow(EGLWindow **eglWindow)
    {
        ASSERT_NE(nullptr, *eglWindow);
        (*eglWindow)->destroyGL();
        EGLWindow::Delete(eglWindow);
    }

    GLuint createES2ProgramFromSource()
    {
        return CompileProgram(essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    }

    GLuint createES3ProgramFromSource()
    {
        return CompileProgram(essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    }

    void drawWithProgram(GLuint program)
    {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        GLint positionLocation = glGetAttribLocation(program, essl1_shaders::PositionAttrib());

        glUseProgram(program);

        const GLfloat vertices[] = {
            -1.0f, 1.0f, 0.5f, -1.0f, -1.0f, 0.5f, 1.0f, -1.0f, 0.5f,

            -1.0f, 1.0f, 0.5f, 1.0f,  -1.0f, 0.5f, 1.0f, 1.0f,  0.5f,
        };

        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(positionLocation);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

        EXPECT_PIXEL_EQ(mOSWindow->getWidth() / 2, mOSWindow->getHeight() / 2, 255, 0, 0, 255);
    }

    void TearDown() override
    {
        mOSWindow->destroy();
        OSWindow::Delete(&mOSWindow);
    }

    OSWindow *mOSWindow = nullptr;
    std::unique_ptr<angle::Library> mEntryPointsLib;
};

// Tries to create a program binary using one set of platform params, then load it using a different
// sent of params
TEST_P(ProgramBinariesAcrossPlatforms, CreateAndReloadBinary)
{
    angle::PlatformParameters firstRenderer  = GetParam();
    angle::PlatformParameters secondRenderer = GetParam().loadParams;
    bool expectedLinkResult                  = GetParam().expectedLinkResult;

    // First renderer not supported, skipping test.
    ANGLE_SKIP_TEST_IF(!(IsPlatformAvailable(firstRenderer)));

    // Second renderer not supported, skipping test.
    ANGLE_SKIP_TEST_IF(!(IsPlatformAvailable(secondRenderer)));

    EGLWindow *eglWindow = nullptr;
    std::vector<uint8_t> binary(0);
    GLuint program = 0;

    GLint programLength = 0;
    GLint writtenLength = 0;
    GLenum binaryFormat = 0;

    // Create a EGL window with the first renderer
    eglWindow = createAndInitEGLWindow(firstRenderer);
    if (eglWindow == nullptr)
    {
        FAIL() << "Failed to create EGL window";
    }

    // If the test is trying to use both the default GPU and WARP, but the default GPU *IS* WARP,
    // then our expectations for the test results will be invalid.
    if (firstRenderer.eglParameters.deviceType != EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE &&
        secondRenderer.eglParameters.deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE)
    {
        std::string rendererString =
            std::string(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
        angle::ToLower(&rendererString);

        auto basicRenderPos     = rendererString.find(std::string("microsoft basic render"));
        auto softwareAdapterPos = rendererString.find(std::string("software adapter"));

        // The first renderer is using WARP, even though we didn't explictly request it
        // We should skip this test
        ANGLE_SKIP_TEST_IF(basicRenderPos != std::string::npos ||
                           softwareAdapterPos != std::string::npos);
    }

    // Create a program
    if (firstRenderer.majorVersion == 3)
    {
        program = createES3ProgramFromSource();
    }
    else
    {
        program = createES2ProgramFromSource();
    }

    if (program == 0)
    {
        destroyEGLWindow(&eglWindow);
        FAIL() << "Failed to create program from source";
    }

    // Draw using the program to ensure it works as expected
    drawWithProgram(program);
    EXPECT_GL_NO_ERROR();

    // Save the program binary out from this renderer
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH_OES, &programLength);
    EXPECT_GL_NO_ERROR();
    binary.resize(programLength);
    glGetProgramBinaryOES(program, programLength, &writtenLength, &binaryFormat, binary.data());
    EXPECT_GL_NO_ERROR();

    // Destroy the first renderer
    glDeleteProgram(program);
    destroyEGLWindow(&eglWindow);

    // Create an EGL window with the second renderer
    eglWindow = createAndInitEGLWindow(secondRenderer);
    if (eglWindow == nullptr)
    {
        FAIL() << "Failed to create EGL window";
    }

    program = glCreateProgram();
    glProgramBinaryOES(program, binaryFormat, binary.data(), writtenLength);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_EQ(expectedLinkResult, (linkStatus != 0));

    if (linkStatus != 0)
    {
        // If the link was successful, then we should try to draw using the program to ensure it
        // works as expected
        drawWithProgram(program);
        EXPECT_GL_NO_ERROR();
    }

    // Destroy the second renderer
    glDeleteProgram(program);
    destroyEGLWindow(&eglWindow);
}

// clang-format off
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProgramBinariesAcrossPlatforms);
ANGLE_INSTANTIATE_TEST(ProgramBinariesAcrossPlatforms,
                       //                     | Save the     | Load the     | Expected
                       //                     | program in   | program in   | link
                       //                     | this config  | this config  | result
                       PlatformsWithLinkResult(ES2_D3D11(),   ES2_D3D11(),   true ), // Loading + reloading binary should work
                       PlatformsWithLinkResult(ES3_D3D11(),   ES3_D3D11(),   true ), // Loading + reloading binary should work
                       PlatformsWithLinkResult(ES2_D3D11(),   ES2_D3D9(),    false), // Switching from D3D11 to D3D9 shouldn't work
                       PlatformsWithLinkResult(ES2_D3D9(),    ES2_D3D11(),   false), // Switching from D3D9 to D3D11 shouldn't work
                       PlatformsWithLinkResult(ES2_D3D11(),   ES3_D3D11(),   false), // Switching to newer client version shouldn't work
                       PlatformsWithLinkResult(ES2_VULKAN(),  ES2_VULKAN(),  true ), // Loading + reloading binary should work
                       PlatformsWithLinkResult(ES3_VULKAN(),  ES3_VULKAN(),  true ), // Loading + reloading binary should work
                       PlatformsWithLinkResult(ES31_VULKAN(), ES31_VULKAN(), true ), // Loading + reloading binary should work
                       PlatformsWithLinkResult(ES3_VULKAN(),  ES31_VULKAN(), false), // Switching to newer client version shouldn't work with Vulkan
                      );
// clang-format on
