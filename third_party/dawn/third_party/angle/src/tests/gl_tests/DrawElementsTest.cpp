//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
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

class DrawElementsTest : public ANGLETest<>
{
  protected:
    DrawElementsTest() : mProgram(0u)
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    ~DrawElementsTest()
    {
        for (GLuint indexBuffer : mIndexBuffers)
        {
            if (indexBuffer != 0)
            {
                glDeleteBuffers(1, &indexBuffer);
            }
        }

        for (GLuint vertexArray : mVertexArrays)
        {
            if (vertexArray != 0)
            {
                glDeleteVertexArrays(1, &vertexArray);
            }
        }

        for (GLuint vertexBuffer : mVertexBuffers)
        {
            if (vertexBuffer != 0)
            {
                glDeleteBuffers(1, &vertexBuffer);
            }
        }

        if (mProgram != 0u)
        {
            glDeleteProgram(mProgram);
        }
    }

    std::vector<GLuint> mIndexBuffers;
    std::vector<GLuint> mVertexArrays;
    std::vector<GLuint> mVertexBuffers;
    GLuint mProgram;
};

class WebGLDrawElementsTest : public DrawElementsTest
{
  public:
    WebGLDrawElementsTest() { setWebGLCompatibilityEnabled(true); }
};

// Test no error is generated when using client-side arrays, indices = nullptr and count = 0
TEST_P(DrawElementsTest, ClientSideNullptrArrayZeroCount)
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

    // "If drawElements is called with a count greater than zero, and no WebGLBuffer is bound to the
    // ELEMENT_ARRAY_BUFFER binding point, an INVALID_OPERATION error is generated."
    glDrawElements(GL_TRIANGLES, 1, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // count == 0 so it's fine to have no element array buffer bound.
    glDrawElements(GL_TRIANGLES, 0, GL_UNSIGNED_BYTE, nullptr);
    ASSERT_GL_NO_ERROR();
}

// Test uploading part of an index buffer after deleting a vertex array
// previously used for DrawElements.
TEST_P(DrawElementsTest, DeleteVertexArrayAndUploadIndex)
{
    const auto &vertices = GetIndexedQuadVertices();
    const auto &indices  = GetQuadIndices();

    ANGLE_GL_PROGRAM(programDrawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glUseProgram(programDrawRed);

    GLint posLocation = glGetAttribLocation(programDrawRed, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, posLocation);

    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posLocation);

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(),
                 GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    glDeleteVertexArrays(1, &vertexArray);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    // Could crash here if the observer binding from the vertex array doesn't get
    // removed on vertex array destruction.
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices[0]) * 3, indices.data());

    ASSERT_GL_NO_ERROR();
}

// Test VAO switch is handling cached element array buffer properly along with line loop mode
// switch.
TEST_P(DrawElementsTest, LineLoopTriangles)
{
    const auto &vertices                    = GetIndexedQuadVertices();
    constexpr std::array<GLuint, 6> indices = {{0, 1, 2, 0, 2, 3}};

    ANGLE_GL_PROGRAM(programDrawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    ANGLE_GL_PROGRAM(programDrawBlue, essl3_shaders::vs::Simple(), essl3_shaders::fs::Blue());

    glUseProgram(programDrawRed);
    GLint posLocation = glGetAttribLocation(programDrawRed, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, posLocation);

    GLVertexArray vertexArray[2];
    GLBuffer vertexBuffer[2];

    glBindVertexArray(vertexArray[0]);

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(),
                 GL_STATIC_DRAW);

    for (int i = 0; i < 2; i++)
    {
        glBindVertexArray(vertexArray[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(posLocation);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    }

    // First draw with VAO0 and line loop mode
    glBindVertexArray(vertexArray[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    // Switch to VAO1 and draw with triangle mode.
    glBindVertexArray(vertexArray[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::red);

    // Switch back to VAO0 and draw with triangle mode.
    glUseProgram(programDrawBlue);
    glBindVertexArray(vertexArray[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::blue);
    ASSERT_GL_NO_ERROR();
}

// Regression test for using two VAOs, one to draw only GL_LINE_LOOPs, and
// another to draw indexed triangles.
TEST_P(DrawElementsTest, LineLoopTriangles2)
{
    const auto &vertices                    = GetIndexedQuadVertices();
    constexpr std::array<GLuint, 6> indices = {{0, 1, 2, 0, 2, 3}};

    ANGLE_GL_PROGRAM(programDrawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());

    glUseProgram(programDrawRed);
    GLint posLocation = glGetAttribLocation(programDrawRed, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, posLocation);

    GLVertexArray vertexArray[2];
    GLBuffer vertexBuffer[2];

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(),
                 GL_STATIC_DRAW);

    for (int i = 0; i < 2; i++)
    {
        glBindVertexArray(vertexArray[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(posLocation);
        if (i != 0)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    }

    // First draw with VAO0 and line loop mode
    glBindVertexArray(vertexArray[0]);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    // Switch to VAO1 and draw some indexed triangles
    glBindVertexArray(vertexArray[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    // Switch back to VAO0 and do another line loop
    glBindVertexArray(vertexArray[0]);

    // Would crash if the index buffer dirty bit got errantly set on VAO0.
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    ASSERT_GL_NO_ERROR();
}

// Test a state desync that can occur when using a streaming index buffer in GL in concert with
// deleting the applied index buffer.
TEST_P(DrawElementsTest, DeletingAfterStreamingIndexes)
{
    // Init program
    constexpr char kVS[] =
        "attribute vec2 position;\n"
        "attribute vec2 testFlag;\n"
        "varying vec2 v_data;\n"
        "void main() {\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "  v_data = testFlag;\n"
        "}";

    constexpr char kFS[] =
        "varying highp vec2 v_data;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(v_data, 0, 1);\n"
        "}";

    mProgram = CompileProgram(kVS, kFS);
    ASSERT_NE(0u, mProgram);
    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);

    GLint testFlagLocation = glGetAttribLocation(mProgram, "testFlag");
    ASSERT_NE(-1, testFlagLocation);

    mIndexBuffers.resize(3u);
    glGenBuffers(3, &mIndexBuffers[0]);

    mVertexArrays.resize(2);
    glGenVertexArrays(2, &mVertexArrays[0]);

    mVertexBuffers.resize(2);
    glGenBuffers(2, &mVertexBuffers[0]);

    std::vector<GLuint> indexData[2];
    indexData[0].push_back(0);
    indexData[0].push_back(1);
    indexData[0].push_back(2);
    indexData[0].push_back(2);
    indexData[0].push_back(3);
    indexData[0].push_back(0);

    indexData[1] = indexData[0];
    for (GLuint &item : indexData[1])
    {
        item += 4u;
    }

    std::vector<GLfloat> positionData = {// quad verts
                                         -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
                                         // Repeat position data
                                         -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f};

    std::vector<GLfloat> testFlagData = {// red
                                         1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                                         // green
                                         0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f};

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffers[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexData[0].size(), &indexData[0][0],
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffers[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexData[0].size(), &indexData[0][0],
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexData[1].size(), &indexData[1][0],
                 GL_STATIC_DRAW);

    // Initialize first vertex array with second index buffer
    glBindVertexArray(mVertexArrays[0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffers[1]);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positionData.size(), &positionData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * testFlagData.size(), &testFlagData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(testFlagLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(testFlagLocation);

    // Initialize second vertex array with first index buffer
    glBindVertexArray(mVertexArrays[1]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffers[0]);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffers[0]);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffers[1]);
    glVertexAttribPointer(testFlagLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(testFlagLocation);

    ASSERT_GL_NO_ERROR();

    glBindVertexArray(mVertexArrays[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    glBindVertexArray(mVertexArrays[1]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glBindVertexArray(mVertexArrays[0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Trigger the bug here.
    glDeleteBuffers(1, &mIndexBuffers[2]);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    ASSERT_GL_NO_ERROR();
}

// Verify that detaching shaders after linking doesn't break draw calls
TEST_P(DrawElementsTest, DrawWithDetachedShaders)
{
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

    GLuint vertexShader   = CompileShader(GL_VERTEX_SHADER, essl3_shaders::vs::Simple());
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, essl3_shaders::fs::Red());
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

    glUseProgram(program);

    GLint posLocation = glGetAttribLocation(program, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, posLocation);

    GLVertexArray vertexArray;
    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    glDeleteProgram(program);
    ASSERT_GL_NO_ERROR();
}

// Test drawing to part of the indices in an index buffer, and then all of them.
TEST_P(DrawElementsTest, PartOfIndexBufferThenAll)
{
    // Init program
    constexpr char kVS[] =
        "attribute vec2 position;\n"
        "attribute vec2 testFlag;\n"
        "varying vec2 v_data;\n"
        "void main() {\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "  v_data = testFlag;\n"
        "}";

    constexpr char kFS[] =
        "varying highp vec2 v_data;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(v_data, 0, 1);\n"
        "}";

    mProgram = CompileProgram(kVS, kFS);
    ASSERT_NE(0u, mProgram);
    glUseProgram(mProgram);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);

    GLint testFlagLocation = glGetAttribLocation(mProgram, "testFlag");
    ASSERT_NE(-1, testFlagLocation);

    mIndexBuffers.resize(1);
    glGenBuffers(1, &mIndexBuffers[0]);

    mVertexArrays.resize(1);
    glGenVertexArrays(1, &mVertexArrays[0]);

    mVertexBuffers.resize(2);
    glGenBuffers(2, &mVertexBuffers[0]);

    std::vector<GLubyte> indexData[2];
    indexData[0].push_back(0);
    indexData[0].push_back(1);
    indexData[0].push_back(2);
    indexData[0].push_back(2);
    indexData[0].push_back(3);
    indexData[0].push_back(0);
    indexData[0].push_back(4);
    indexData[0].push_back(5);
    indexData[0].push_back(6);
    indexData[0].push_back(6);
    indexData[0].push_back(7);
    indexData[0].push_back(4);

    // Make a copy:
    indexData[1] = indexData[0];

    std::vector<GLfloat> positionData = {// quad verts
                                         -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
                                         // Repeat position data
                                         -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f};

    std::vector<GLfloat> testFlagData = {// red
                                         1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                                         // green
                                         0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f};

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffers[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * indexData[0].size(), &indexData[0][0],
                 GL_STATIC_DRAW);

    glBindVertexArray(mVertexArrays[0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffers[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positionData.size(), &positionData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * testFlagData.size(), &testFlagData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(testFlagLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(testFlagLocation);

    ASSERT_GL_NO_ERROR();

    // Draw with just the second set of 6 items, then first 6, and then the entire index buffer
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(6));
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Reload the buffer again with a copy of the same data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * indexData[1].size(), &indexData[1][0],
                 GL_STATIC_DRAW);

    // Draw with just the first 6 indices, and then with the entire index buffer
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Reload the buffer again with a copy of the same data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * indexData[0].size(), &indexData[0][0],
                 GL_STATIC_DRAW);

    // This time, do not check color between draws (which causes a flush):
    // Draw with just the second set of 6 items, then first 6, and then the entire index buffer
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(6));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    ASSERT_GL_NO_ERROR();
}

// Test that glDrawElements call with different index buffer offsets work as expected
TEST_P(DrawElementsTest, DrawElementsWithDifferentIndexBufferOffsets)
{
    const std::array<Vector3, 4> &vertices = GetIndexedQuadVertices();
    const std::array<GLushort, 6> &indices = GetQuadIndices();

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    ANGLE_GL_PROGRAM(programDrawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    ANGLE_GL_PROGRAM(programDrawGreen, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());
    ANGLE_GL_PROGRAM(programDrawBlue, essl3_shaders::vs::Simple(), essl3_shaders::fs::Blue());

    glUseProgram(programDrawRed);

    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    GLint posLocation = glGetAttribLocation(programDrawRed, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, posLocation);
    glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Draw both triangles of quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices.data());
    EXPECT_PIXEL_COLOR_EQ(0, 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 2, GLColor::red);

    glUseProgram(programDrawGreen);

    GLuint vertexArray1;
    glGenVertexArrays(1, &vertexArray1);
    glBindVertexArray(vertexArray1);
    GLBuffer vertexBuffer1;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    posLocation = glGetAttribLocation(programDrawGreen, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, posLocation);
    glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posLocation);

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(),
                 GL_DYNAMIC_DRAW);

    // Draw right triangle of quad
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, reinterpret_cast<const void *>(6));
    EXPECT_PIXEL_COLOR_EQ(0, 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 2, GLColor::green);

    glUseProgram(programDrawBlue);

    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    posLocation = glGetAttribLocation(programDrawBlue, essl3_shaders::PositionAttrib());
    ASSERT_NE(-1, posLocation);
    glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posLocation);

    // Draw both triangles of quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices.data());
    EXPECT_PIXEL_COLOR_EQ(0, 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 2, GLColor::blue);

    glDeleteVertexArrays(1, &vertexArray);
    glDeleteVertexArrays(1, &vertexArray1);

    ASSERT_GL_NO_ERROR();
}

// Test that the offset in the index buffer is forced to be a multiple of the element size
TEST_P(WebGLDrawElementsTest, DrawElementsTypeAlignment)
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

    GLBuffer indexBuffer;
    const GLubyte indices1[] = {0, 0, 0, 0, 0, 0};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_STATIC_DRAW);

    ASSERT_GL_NO_ERROR();

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    const GLushort indices2[] = {0, 0, 0, 0, 0, 0};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, reinterpret_cast<const void *>(1));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DrawElementsTest);
ANGLE_INSTANTIATE_TEST_ES3(DrawElementsTest);

ANGLE_INSTANTIATE_TEST_ES2(WebGLDrawElementsTest);
}  // namespace
