//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MultiDrawTest: Tests of GL_ANGLE_multi_draw
// MultiDrawIndirectTest: Tests of GL_EXT_multi_draw_indirect

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

// Create a kWidth * kHeight canvas equally split into kCountX * kCountY tiles
// each containing a quad partially covering each tile
constexpr uint32_t kWidth                  = 256;
constexpr uint32_t kHeight                 = 256;
constexpr uint32_t kCountX                 = 8;
constexpr uint32_t kCountY                 = 8;
constexpr uint32_t kQuadCount              = kCountX * kCountY;
constexpr uint32_t kTriCount               = kQuadCount * 2;
constexpr std::array<GLfloat, 2> kTileSize = {
    1.f / static_cast<GLfloat>(kCountX),
    1.f / static_cast<GLfloat>(kCountY),
};
constexpr std::array<uint32_t, 2> kTilePixelSize  = {kWidth / kCountX, kHeight / kCountY};
constexpr std::array<GLfloat, 2> kQuadRadius      = {0.25f * kTileSize[0], 0.25f * kTileSize[1]};
constexpr std::array<uint32_t, 2> kPixelCheckSize = {
    static_cast<uint32_t>(kQuadRadius[0] * kWidth),
    static_cast<uint32_t>(kQuadRadius[1] * kHeight)};

constexpr std::array<GLfloat, 2> getTileCenter(uint32_t x, uint32_t y)
{
    return {
        kTileSize[0] * (0.5f + static_cast<GLfloat>(x)),
        kTileSize[1] * (0.5f + static_cast<GLfloat>(y)),
    };
}
constexpr std::array<std::array<GLfloat, 3>, 4> getQuadVertices(uint32_t x, uint32_t y)
{
    const auto center = getTileCenter(x, y);
    return {
        std::array<GLfloat, 3>{center[0] - kQuadRadius[0], center[1] - kQuadRadius[1], 0.0f},
        std::array<GLfloat, 3>{center[0] + kQuadRadius[0], center[1] - kQuadRadius[1], 0.0f},
        std::array<GLfloat, 3>{center[0] + kQuadRadius[0], center[1] + kQuadRadius[1], 0.0f},
        std::array<GLfloat, 3>{center[0] - kQuadRadius[0], center[1] + kQuadRadius[1], 0.0f},
    };
}

enum class DrawIDOption
{
    NoDrawID,
    UseDrawID,
};

enum class InstancingOption
{
    NoInstancing,
    UseInstancing,
};

enum class BufferDataUsageOption
{
    StaticDraw,
    DynamicDraw
};

using MultiDrawTestParams =
    std::tuple<angle::PlatformParameters, DrawIDOption, InstancingOption, BufferDataUsageOption>;

using MultiDrawIndirectTestParams = angle::PlatformParameters;

struct PrintToStringParamName
{
    std::string operator()(const ::testing::TestParamInfo<MultiDrawTestParams> &info) const
    {
        ::std::stringstream ss;
        ss << std::get<0>(info.param)
           << (std::get<3>(info.param) == BufferDataUsageOption::StaticDraw ? "__StaticDraw"
                                                                            : "__DynamicDraw")
           << (std::get<2>(info.param) == InstancingOption::UseInstancing ? "__Instanced" : "")
           << (std::get<1>(info.param) == DrawIDOption::UseDrawID ? "__DrawID" : "");
        return ss.str();
    }
};

struct DrawArraysIndirectCommand
{
    DrawArraysIndirectCommand() : count(0u), instanceCount(0u), first(0u), baseInstance(0u) {}
    DrawArraysIndirectCommand(GLuint count, GLuint instanceCount, GLuint first, GLuint baseInstance)
        : count(count), instanceCount(instanceCount), first(first), baseInstance(baseInstance)
    {}
    GLuint count;
    GLuint instanceCount;
    GLuint first;
    GLuint baseInstance;
};

struct DrawElementsIndirectCommand
{
    DrawElementsIndirectCommand()
        : count(0), primCount(0), firstIndex(0), baseVertex(0), baseInstance(0)
    {}
    DrawElementsIndirectCommand(GLuint count,
                                GLuint primCount,
                                GLuint firstIndex,
                                GLint baseVertex,
                                GLuint baseInstance)
        : count(count),
          primCount(primCount),
          firstIndex(firstIndex),
          baseVertex(baseVertex),
          baseInstance(baseInstance)
    {}
    GLuint count;
    GLuint primCount;
    GLuint firstIndex;
    GLint baseVertex;
    GLuint baseInstance;
};

// The tests in MultiDrawTest and MultiDrawNoInstancingSupportTest check the correctness
// of the ANGLE_multi_draw extension.
// An array of quads is drawn across the screen.
// gl_DrawID is checked by using it to select the color of the draw.
// MultiDraw*Instanced entrypoints use the existing instancing APIs which are
// more fully tested in InstancingTest.cpp.
// Correct interaction with the instancing APIs is tested here by using scaling
// and then instancing the array of quads over four quadrants on the screen.
class MultiDrawTest : public ANGLETestBase, public ::testing::TestWithParam<MultiDrawTestParams>
{
  protected:
    MultiDrawTest()
        : ANGLETestBase(std::get<0>(GetParam())),
          mNonIndexedVertexBuffer(0u),
          mVertexBuffer(0u),
          mIndexBuffer(0u),
          mInstanceBuffer(0u),
          mProgram(0u),
          mPositionLoc(0u),
          mInstanceLoc(0u)
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void SetUp() override { ANGLETestBase::ANGLETestSetUp(); }

    bool IsDrawIDTest() const { return std::get<1>(GetParam()) == DrawIDOption::UseDrawID; }

    bool IsInstancedTest() const
    {
        return std::get<2>(GetParam()) == InstancingOption::UseInstancing;
    }

    GLenum getBufferDataUsage() const
    {
        return std::get<3>(GetParam()) == BufferDataUsageOption::StaticDraw ? GL_STATIC_DRAW
                                                                            : GL_DYNAMIC_DRAW;
    }

    std::string VertexShaderSource()
    {

        std::stringstream shader;
        shader << (IsDrawIDTest() ? "#extension GL_ANGLE_multi_draw : require\n" : "")
               << (IsInstancedTest() ? "attribute float vInstance;" : "") << R"(
attribute vec2 vPosition;
varying vec4 color;
void main()
{
    int id = )" << (IsDrawIDTest() ? "gl_DrawID" : "0")
               << ";" << R"(
    float quad_id = float(id / 2);
    float color_id = quad_id - (3.0 * floor(quad_id / 3.0));
    if (color_id == 0.0) {
      color = vec4(1, 0, 0, 1);
    } else if (color_id == 1.0) {
      color = vec4(0, 1, 0, 1);
    } else {
      color = vec4(0, 0, 1, 1);
    }

    mat3 transform = mat3(1.0);
)"
               << (IsInstancedTest() ? R"(
    transform[0][0] = 0.5;
    transform[1][1] = 0.5;
    if (vInstance == 0.0) {

    } else if (vInstance == 1.0) {
        transform[2][0] = 0.5;
    } else if (vInstance == 2.0) {
        transform[2][1] = 0.5;
    } else if (vInstance == 3.0) {
        transform[2][0] = 0.5;
        transform[2][1] = 0.5;
    }
)"
                                     : "")
               << R"(
    gl_Position = vec4(transform * vec3(vPosition, 1.0) * 2.0 - 1.0, 1);
})";

        return shader.str();
    }

    std::string FragmentShaderSource()
    {
        return
            R"(precision mediump float;
            varying vec4 color;
            void main()
            {
                gl_FragColor = color;
            })";
    }

    void SetupProgram()
    {
        mProgram = CompileProgram(VertexShaderSource().c_str(), FragmentShaderSource().c_str());
        EXPECT_GL_NO_ERROR();
        ASSERT_GE(mProgram, 1u);
        glUseProgram(mProgram);
        mPositionLoc = glGetAttribLocation(mProgram, "vPosition");
        mInstanceLoc = glGetAttribLocation(mProgram, "vInstance");
    }

    void SetupBuffers()
    {
        for (uint32_t y = 0; y < kCountY; ++y)
        {
            for (uint32_t x = 0; x < kCountX; ++x)
            {
                // v3 ---- v2
                // |       |
                // |       |
                // v0 ---- v1
                uint32_t quadIndex         = y * kCountX + x;
                GLushort starting_index    = static_cast<GLushort>(4 * quadIndex);
                std::array<GLushort, 6> is = {0, 1, 2, 0, 2, 3};
                const auto vs              = getQuadVertices(x, y);
                for (GLushort i : is)
                {
                    mIndices.push_back(starting_index + i);
                }

                for (const auto &v : vs)
                {
                    mVertices.insert(mVertices.end(), v.begin(), v.end());
                }

                for (GLushort i : is)
                {
                    mNonIndexedVertices.insert(mNonIndexedVertices.end(), vs[i].begin(),
                                               vs[i].end());
                }
            }
        }

        std::array<GLfloat, 4> instances{0, 1, 2, 3};

        glGenBuffers(1, &mNonIndexedVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mNonIndexedVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mNonIndexedVertices.size(),
                     mNonIndexedVertices.data(), getBufferDataUsage());

        glGenBuffers(1, &mVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mVertices.size(), mVertices.data(),
                     getBufferDataUsage());

        glGenBuffers(1, &mIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * mIndices.size(), mIndices.data(),
                     getBufferDataUsage());

        glGenBuffers(1, &mInstanceBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mInstanceBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * instances.size(), instances.data(),
                     getBufferDataUsage());

        ASSERT_GL_NO_ERROR();
    }

    void DoVertexAttribDivisor(GLint location, GLuint divisor)
    {
        if (getClientMajorVersion() <= 2)
        {
            ASSERT_TRUE(IsGLExtensionEnabled("GL_ANGLE_instanced_arrays"));
            glVertexAttribDivisorANGLE(location, divisor);
        }
        else
        {
            glVertexAttribDivisor(location, divisor);
        }
    }

    void DoDrawArrays()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindBuffer(GL_ARRAY_BUFFER, mNonIndexedVertexBuffer);
        glEnableVertexAttribArray(mPositionLoc);
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        std::vector<GLint> firsts(kTriCount);
        std::vector<GLsizei> counts(kTriCount, 3);
        for (uint32_t i = 0; i < kTriCount; ++i)
        {
            firsts[i] = i * 3;
        }

        if (IsInstancedTest())
        {
            glBindBuffer(GL_ARRAY_BUFFER, mInstanceBuffer);
            glEnableVertexAttribArray(mInstanceLoc);
            glVertexAttribPointer(mInstanceLoc, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
            DoVertexAttribDivisor(mInstanceLoc, 1);
            std::vector<GLsizei> instanceCounts(kTriCount, 4);
            glMultiDrawArraysInstancedANGLE(GL_TRIANGLES, firsts.data(), counts.data(),
                                            instanceCounts.data(), kTriCount);
        }
        else
        {
            glMultiDrawArraysANGLE(GL_TRIANGLES, firsts.data(), counts.data(), kTriCount);
        }
    }

    void DoDrawElements()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glEnableVertexAttribArray(mPositionLoc);
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        std::vector<GLsizei> counts(kTriCount, 3);
        std::vector<const GLvoid *> indices(kTriCount);
        for (uint32_t i = 0; i < kTriCount; ++i)
        {
            indices[i] = reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(i * 3 * 2));
        }

        if (IsInstancedTest())
        {
            glBindBuffer(GL_ARRAY_BUFFER, mInstanceBuffer);
            glEnableVertexAttribArray(mInstanceLoc);
            glVertexAttribPointer(mInstanceLoc, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
            DoVertexAttribDivisor(mInstanceLoc, 1);
            std::vector<GLsizei> instanceCounts(kTriCount, 4);
            glMultiDrawElementsInstancedANGLE(GL_TRIANGLES, counts.data(), GL_UNSIGNED_SHORT,
                                              indices.data(), instanceCounts.data(), kTriCount);
        }
        else
        {
            glMultiDrawElementsANGLE(GL_TRIANGLES, counts.data(), GL_UNSIGNED_SHORT, indices.data(),
                                     kTriCount);
        }
    }

    enum class DrawIDOptionOverride
    {
        Default,
        NoDrawID,
        UseDrawID,
    };

    void CheckDrawResult(DrawIDOptionOverride overrideDrawID)
    {
        for (uint32_t y = 0; y < kCountY; ++y)
        {
            for (uint32_t x = 0; x < kCountX; ++x)
            {
                uint32_t center_x = x * kTilePixelSize[0] + kTilePixelSize[0] / 2;
                uint32_t center_y = y * kTilePixelSize[1] + kTilePixelSize[1] / 2;
                uint32_t quadID = IsDrawIDTest() && overrideDrawID != DrawIDOptionOverride::NoDrawID
                                      ? y * kCountX + x
                                      : 0;
                uint32_t colorID              = quadID % 3u;
                std::array<GLColor, 3> colors = {GLColor(255, 0, 0, 255), GLColor(0, 255, 0, 255),
                                                 GLColor(0, 0, 255, 255)};
                GLColor expected              = colors[colorID];

                if (IsInstancedTest())
                {
                    EXPECT_PIXEL_RECT_EQ(center_x / 2 - kPixelCheckSize[0] / 4,
                                         center_y / 2 - kPixelCheckSize[1] / 4,
                                         kPixelCheckSize[0] / 2, kPixelCheckSize[1] / 2, expected);
                    EXPECT_PIXEL_RECT_EQ(center_x / 2 - kPixelCheckSize[0] / 4 + kWidth / 2,
                                         center_y / 2 - kPixelCheckSize[1] / 4,
                                         kPixelCheckSize[0] / 2, kPixelCheckSize[1] / 2, expected);
                    EXPECT_PIXEL_RECT_EQ(center_x / 2 - kPixelCheckSize[0] / 4,
                                         center_y / 2 - kPixelCheckSize[1] / 4 + kHeight / 2,
                                         kPixelCheckSize[0] / 2, kPixelCheckSize[1] / 2, expected);
                    EXPECT_PIXEL_RECT_EQ(center_x / 2 - kPixelCheckSize[0] / 4 + kWidth / 2,
                                         center_y / 2 - kPixelCheckSize[1] / 4 + kHeight / 2,
                                         kPixelCheckSize[0] / 2, kPixelCheckSize[1] / 2, expected);
                }
                else
                {
                    EXPECT_PIXEL_RECT_EQ(center_x - kPixelCheckSize[0] / 2,
                                         center_y - kPixelCheckSize[1] / 2, kPixelCheckSize[0],
                                         kPixelCheckSize[1], expected);
                }
            }
        }
    }

    void TearDown() override
    {
        if (mNonIndexedVertexBuffer != 0u)
        {
            glDeleteBuffers(1, &mNonIndexedVertexBuffer);
        }
        if (mVertexBuffer != 0u)
        {
            glDeleteBuffers(1, &mVertexBuffer);
        }
        if (mIndexBuffer != 0u)
        {
            glDeleteBuffers(1, &mIndexBuffer);
        }
        if (mInstanceBuffer != 0u)
        {
            glDeleteBuffers(1, &mInstanceBuffer);
        }
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
        }
        ANGLETestBase::ANGLETestTearDown();
    }

    bool requestMultiDrawExtension() { return EnsureGLExtensionEnabled("GL_ANGLE_multi_draw"); }

    bool requestInstancedExtension()
    {
        return EnsureGLExtensionEnabled("GL_ANGLE_instanced_arrays");
    }

    bool requestExtensions()
    {
        if (IsInstancedTest() && getClientMajorVersion() <= 2)
        {
            if (!requestInstancedExtension())
            {
                return false;
            }
        }
        return requestMultiDrawExtension();
    }

    std::vector<GLushort> mIndices;
    std::vector<GLfloat> mVertices;
    std::vector<GLfloat> mNonIndexedVertices;
    GLuint mNonIndexedVertexBuffer;
    GLuint mVertexBuffer;
    GLuint mIndexBuffer;
    GLuint mInstanceBuffer;
    GLuint mProgram;
    GLint mPositionLoc;
    GLint mInstanceLoc;
};

class MultiDrawTestES3 : public MultiDrawTest
{};

class MultiDrawNoInstancingSupportTest : public MultiDrawTest
{
    void SetUp() override
    {
        ASSERT_LE(getClientMajorVersion(), 2);
        ASSERT_TRUE(IsInstancedTest());
        MultiDrawTest::SetUp();
    }
};

// The tests in MultiDrawIndirectTest check the correctness
// of the EXT_multi_draw_indirect extension.
// 4 magenta triangles are drawn at the corners of the screen
// in different orders from the same vertex and index arrays.
class MultiDrawIndirectTest : public ANGLETestBase,
                              public ::testing::TestWithParam<MultiDrawIndirectTestParams>
{
  protected:
    MultiDrawIndirectTest()
        : ANGLETestBase(GetParam()),
          mPositionLoc(0u),
          mColorLoc(0u),
          mVertexBuffer(0u),
          mColorBuffer(0u),
          mIndexBuffer(0u),
          mIndirectBuffer(0u),
          mProgram(0u)
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void SetUp() override { ANGLETestBase::ANGLETestSetUp(); }

    void SetupProgramIndirect(bool isMultiColor)
    {
        // Define the vertex and fragment shaders
        std::stringstream kVS;
        kVS << R"(#version 310 es
in vec3 aPos;
)"
            << (isMultiColor ? R"(in vec4 aColor;
out vec4 vColor;)"
                             : "")
            << R"(
void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0, 1);
)" << (isMultiColor ? R"(vColor = vec4(aColor.x, aColor.y, aColor.z, 1.0);)" : "")
            << R"(
})";

        std::stringstream kFS;
        kFS << R"(#version 310 es
precision mediump float;
)"
            << (isMultiColor ? R"(in vec4 vColor;
)"
                             : "")
            << R"(
out vec4 colorOut;
void main()
{
)" << (isMultiColor ? R"(colorOut = vColor;)" : R"(colorOut = vec4(1.0, 0.0, 1.0, 1.0);)")
            << R"(
})";
        mProgram = CompileProgram(kVS.str().c_str(), kFS.str().c_str());
        EXPECT_GL_NO_ERROR();
        ASSERT_GE(mProgram, 1u);
        glUseProgram(mProgram);
        mPositionLoc = glGetAttribLocation(mProgram, "aPos");
        mColorLoc    = glGetAttribLocation(mProgram, "aColor");
    }

    void TearDown() override
    {
        if (mVertexBuffer != 0u)
        {
            glDeleteBuffers(1, &mVertexBuffer);
        }
        if (mColorBuffer != 0u)
        {
            glDeleteBuffers(1, &mColorBuffer);
        }
        if (mIndexBuffer != 0u)
        {
            glDeleteBuffers(1, &mIndexBuffer);
        }
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
        }
        if (mIndirectBuffer != 0u)
        {
            glDeleteBuffers(1, &mIndirectBuffer);
        }
        ANGLETestBase::ANGLETestTearDown();
    }

    GLint mPositionLoc;
    GLint mColorLoc;
    GLuint mVertexBuffer;
    GLuint mColorBuffer;
    GLuint mIndexBuffer;
    GLuint mIndirectBuffer;
    GLuint mProgram;
};

// Test that compile a program with the extension succeeds
TEST_P(MultiDrawTest, CanCompile)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    SetupProgram();
}

// Tests basic functionality of glMultiDrawArraysANGLE
TEST_P(MultiDrawTest, MultiDrawArrays)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());

    // http://anglebug.com/40644769
    ANGLE_SKIP_TEST_IF(IsInstancedTest() && IsMac() && IsIntelUHD630Mobile() && IsDesktopOpenGL());

    SetupBuffers();
    SetupProgram();
    DoDrawArrays();
    EXPECT_GL_NO_ERROR();
    CheckDrawResult(DrawIDOptionOverride::Default);
}

// Tests basic functionality of glMultiDrawArraysANGLE after a failed program relink
TEST_P(MultiDrawTestES3, MultiDrawArraysAfterFailedRelink)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());

    // http://anglebug.com/40644769
    ANGLE_SKIP_TEST_IF(IsInstancedTest() && IsMac() && IsIntelUHD630Mobile() && IsDesktopOpenGL());

    SetupBuffers();
    SetupProgram();

    // mProgram is already installed.  Destroy its state by a failed relink.
    const char *tfVaryings = "invalidvaryingname";
    glTransformFeedbackVaryings(mProgram, 1, &tfVaryings, GL_SEPARATE_ATTRIBS);
    glLinkProgram(mProgram);
    GLint linkStatus = 0;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linkStatus);
    ASSERT_GL_NO_ERROR();
    ASSERT_EQ(linkStatus, GL_FALSE);

    DoDrawArrays();
    EXPECT_GL_NO_ERROR();
    CheckDrawResult(DrawIDOptionOverride::Default);
}

// Tests basic functionality of glMultiDrawElementsANGLE
TEST_P(MultiDrawTest, MultiDrawElements)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    SetupBuffers();
    SetupProgram();
    DoDrawElements();
    EXPECT_GL_NO_ERROR();
    CheckDrawResult(DrawIDOptionOverride::Default);
}

// Tests that glMultiDrawArraysANGLE followed by glDrawArrays works.  gl_DrawID in the second call
// must be 0.
TEST_P(MultiDrawTest, MultiDrawArraysThenDrawArrays)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());

    // http://anglebug.com/40644769
    ANGLE_SKIP_TEST_IF(IsInstancedTest() && IsMac() && IsIntelUHD630Mobile() && IsDesktopOpenGL());

    SetupBuffers();
    SetupProgram();
    DoDrawArrays();
    EXPECT_GL_NO_ERROR();
    CheckDrawResult(DrawIDOptionOverride::Default);

    if (IsInstancedTest())
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_instanced_arrays") &&
                           !IsGLExtensionEnabled("GL_ANGLE_instanced_arrays"));
        if (IsGLExtensionEnabled("GL_EXT_instanced_arrays"))
        {
            glDrawArraysInstancedEXT(GL_TRIANGLES, 0, 3 * kTriCount, 4);
        }
        else
        {
            glDrawArraysInstancedANGLE(GL_TRIANGLES, 0, 3 * kTriCount, 4);
        }
        ASSERT_GL_NO_ERROR();
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, 3 * kTriCount);
        ASSERT_GL_NO_ERROR();
    }
    CheckDrawResult(DrawIDOptionOverride::NoDrawID);
}

// Tests basic functionality of glMultiDrawArraysIndirectEXT
TEST_P(MultiDrawIndirectTest, MultiDrawArraysIndirect)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multi_draw_indirect"));

    // Set up the vertex array
    const GLint triangleCount           = 4;
    const std::vector<GLfloat> vertices = {
        -1, 1,  0, -1, 0, 0, 0, 1,  0, 1, 1,  0, 1, 0,  0, 0, 1, 0,
        -1, -1, 0, -1, 0, 0, 0, -1, 0, 1, -1, 0, 0, -1, 0, 1, 0, 0,
    };

    // Set up the vertex buffer
    GLVertexArray vao;
    glBindVertexArray(vao);

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    // Generate program
    SetupProgramIndirect(false);

    // Set up the vertex array format
    glEnableVertexAttribArray(mPositionLoc);
    glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    EXPECT_GL_NO_ERROR();

    // Set up the indirect data array
    std::array<DrawArraysIndirectCommand, triangleCount> indirectData;
    const GLsizei icSize = sizeof(DrawArraysIndirectCommand);
    for (auto i = 0; i < triangleCount; i++)
    {
        indirectData[i] = DrawArraysIndirectCommand(3, 1, 3 * i, i);
    }

    glGenBuffers(1, &mIndirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mIndirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, icSize * indirectData.size(), indirectData.data(),
                 GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    // Invalid value check for drawcount and stride
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawArraysIndirectEXT(GL_TRIANGLES, nullptr, 0, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glMultiDrawArraysIndirectEXT(GL_TRIANGLES, nullptr, -1, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glMultiDrawArraysIndirectEXT(GL_TRIANGLES, nullptr, 1, 2);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Check the error from sourcing beyond the allocated buffer size
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawArraysIndirectEXT(
        GL_TRIANGLES,
        reinterpret_cast<const void *>(static_cast<uintptr_t>(icSize * triangleCount)), 1, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Draw all triangles using glMultiDrawArraysIndirect
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawArraysIndirectEXT(GL_TRIANGLES, nullptr, triangleCount, 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);

    // Draw the triangles in different order
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawArraysIndirectEXT(GL_TRIANGLES, nullptr, 1, 0);
    glMultiDrawArraysIndirectEXT(GL_TRIANGLES,
                                 reinterpret_cast<const void *>(static_cast<uintptr_t>(icSize * 2)),
                                 triangleCount - 2, 0);
    glMultiDrawArraysIndirectEXT(
        GL_TRIANGLES, reinterpret_cast<const void *>(static_cast<uintptr_t>(icSize)), 1, 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);

    // Draw the triangles partially using stride
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawArraysIndirectEXT(GL_TRIANGLES, nullptr, 2, icSize * 3);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);
}

// Tests basic functionality of glMultiDrawElementsIndirectEXT
TEST_P(MultiDrawIndirectTest, MultiDrawElementsIndirect)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multi_draw_indirect"));

    // Set up the vertex array
    const GLint triangleCount           = 4;
    const std::vector<GLfloat> vertices = {
        -1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, -1, 0, 0, -1, 0, -1, -1, 0, -1, 0, 0,
    };
    const std::vector<GLuint> indices = {
        1, 2, 3, 3, 4, 5, 5, 6, 7, 7, 0, 1,
    };

    // Set up the vertex and index buffers
    GLVertexArray vao;
    glBindVertexArray(vao);

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(),
                 GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    // Generate program
    SetupProgramIndirect(false);

    // Set up the vertex array format
    glEnableVertexAttribArray(mPositionLoc);
    glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    EXPECT_GL_NO_ERROR();

    // Set up the indirect data array
    std::array<DrawElementsIndirectCommand, triangleCount> indirectData;
    const GLsizei icSize = sizeof(DrawElementsIndirectCommand);
    for (auto i = 0; i < triangleCount; i++)
    {
        indirectData[i] = DrawElementsIndirectCommand(3, 1, 3 * i, 0, i);
    }

    glGenBuffers(1, &mIndirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mIndirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, icSize * indirectData.size(), indirectData.data(),
                 GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    // Invalid value check for drawcount and stride
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 0, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, -1, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 1, 2);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Check the error from sourcing beyond the allocated buffer size
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(
        GL_TRIANGLES, GL_UNSIGNED_INT,
        reinterpret_cast<const void *>(static_cast<uintptr_t>(icSize * triangleCount)), 1, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Draw all triangles using glMultiDrawElementsIndirect
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, triangleCount, 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);

    // Draw the triangles in a different order
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 1, 0);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_INT,
                                   reinterpret_cast<const void *>(static_cast<uintptr_t>(icSize)),
                                   triangleCount - 2, 0);
    glMultiDrawElementsIndirectEXT(
        GL_TRIANGLES, GL_UNSIGNED_INT,
        reinterpret_cast<const void *>(static_cast<uintptr_t>(icSize * 3)), 1, 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);

    // Draw the triangles partially using stride
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 2, icSize * 3);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);
}

// Test functionality glMultiDrawElementsIndirectEXT with unsigned short
// indices and instanced attributes.
TEST_P(MultiDrawIndirectTest, MultiDrawElementsIndirectInstancedUshort)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multi_draw_indirect"));

    // Set up the vertex array
    const GLint triangleCount           = 4;
    const std::vector<GLfloat> vertices = {
        -1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, -1, 0, 0, -1, 0, -1, -1, 0, -1, 0, 0,
    };
    const std::vector<GLushort> indices = {
        1, 2, 3, 3, 4, 5, 5, 6, 7, 7, 0, 1,
    };
    const std::vector<GLuint> instancedColor = {GLColor::white.asUint(), GLColor::red.asUint(),
                                                GLColor::green.asUint(), GLColor::blue.asUint()};

    // Generate program
    SetupProgramIndirect(true);

    // Set up the vertex and index buffers
    GLVertexArray vao;
    glBindVertexArray(vao);

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(mPositionLoc);
    glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    GLBuffer instanceBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * instancedColor.size(), instancedColor.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(mColorLoc);
    glVertexAttribPointer(mColorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);
    glVertexAttribDivisor(mColorLoc, 1);

    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), indices.data(),
                 GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    // Set up the indirect data array
    std::array<DrawElementsIndirectCommand, triangleCount> indirectData;
    const GLsizei icSize = sizeof(DrawElementsIndirectCommand);
    for (auto i = 0; i < triangleCount; i++)
    {
        indirectData[i] = DrawElementsIndirectCommand(3, 1, 3 * i, 0, i);
    }

    glGenBuffers(1, &mIndirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mIndirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, icSize * indirectData.size(), indirectData.data(),
                 GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    // Invalid value check for drawcount and stride
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_SHORT, nullptr, 0, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_SHORT, nullptr, -1, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_SHORT, nullptr, 1, 2);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Check the error from sourcing beyond the allocated buffer size
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(
        GL_TRIANGLES, GL_UNSIGNED_SHORT,
        reinterpret_cast<const void *>(static_cast<uintptr_t>(icSize * triangleCount)), 1, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Draw all triangles using glMultiDrawElementsIndirect
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_SHORT, nullptr, triangleCount, 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::white);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);

    // Draw the triangles in a different order
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_SHORT, nullptr, 1, 0);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_SHORT,
                                   reinterpret_cast<const void *>(static_cast<uintptr_t>(icSize)),
                                   triangleCount - 2, 0);
    glMultiDrawElementsIndirectEXT(
        GL_TRIANGLES, GL_UNSIGNED_SHORT,
        reinterpret_cast<const void *>(static_cast<uintptr_t>(icSize * 3)), 1, 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::white);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);

    // Draw the triangles partially using stride
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_SHORT, nullptr, 2, icSize * 3);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::white);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);
}

// Tests functionality of glMultiDrawElementsIndirectEXT with more than one triangle in one element
// of the indirect buffer.
TEST_P(MultiDrawIndirectTest, MultiDrawElementsIndirectMultipleTriangles)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multi_draw_indirect"));

    // Set up the vertex array
    const GLint triangleCount           = 4;
    const std::vector<GLfloat> vertices = {
        -1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, -1, 0, 0, -1, 0, -1, -1, 0, -1, 0, 0,
    };
    const std::vector<GLuint> indices = {
        1, 2, 3, 3, 4, 5, 5, 6, 7, 7, 0, 1,
    };

    // Set up the vertex and index buffers
    GLVertexArray vao;
    glBindVertexArray(vao);

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(),
                 GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    // Generate program
    SetupProgramIndirect(false);

    // Set up the vertex array format
    glEnableVertexAttribArray(mPositionLoc);
    glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    EXPECT_GL_NO_ERROR();

    // Set up the indirect data array; first element represents two triangles.
    std::array<DrawElementsIndirectCommand, triangleCount - 1> indirectData;
    const GLsizei icSize = sizeof(DrawElementsIndirectCommand);
    for (auto i = 0; i < triangleCount - 1; i++)
    {
        if (i == 0)
        {
            indirectData[i] = DrawElementsIndirectCommand(6, 2, 0, 0, i);
        }
        else
        {
            indirectData[i] = DrawElementsIndirectCommand(3, 1, 3 * (i + 1), 0, i);
        }
    }

    glGenBuffers(1, &mIndirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mIndirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, icSize * indirectData.size(), indirectData.data(),
                 GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    // Draw all triangles using glMultiDrawElementsIndirect
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, triangleCount - 1, 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);
}

// Tests glMultiDrawElementsIndirectEXT with glMultiDrawElementsANGLE to see if the index buffer
// offset is being reset.
TEST_P(MultiDrawIndirectTest, MultiDrawElementsIndirectCheckBufferOffset)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multi_draw_indirect"));

    // Set up the vertex array
    const GLint triangleCount           = 4;
    const std::vector<GLfloat> vertices = {
        -1, 0, 0, -1, 1,  0, 0, 1,  0, 0, 1,  0, 1,  1,  0, 1,  0, 0,
        1,  0, 0, 1,  -1, 0, 0, -1, 0, 0, -1, 0, -1, -1, 0, -1, 0, 0,
    };
    const std::vector<GLfloat> colors = {
        1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0,
    };
    const std::vector<GLuint> indices = {
        3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2,
    };

    // Set up the vertex and index buffers
    GLVertexArray vao;
    glBindVertexArray(vao);

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glGenBuffers(1, &mColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * colors.size(), colors.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(),
                 GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    // Generate program
    SetupProgramIndirect(true);

    // Set up the vertex array format
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glEnableVertexAttribArray(mPositionLoc);
    glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
    glEnableVertexAttribArray(mColorLoc);
    glVertexAttribPointer(mColorLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    EXPECT_GL_NO_ERROR();

    // Set up the arrays for the direct draw
    std::vector<GLsizei> counts(triangleCount, 3);
    std::vector<const GLvoid *> indicesDirect(triangleCount);
    for (auto i = 0; i < triangleCount; i++)
    {
        indicesDirect[i] =
            reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(i * 3 * sizeof(GLuint)));
    }

    // Set up the indirect data array for indirect draw
    std::array<DrawElementsIndirectCommand, triangleCount> indirectData;
    const GLsizei icSize = sizeof(DrawElementsIndirectCommand);
    for (auto i = 0; i < triangleCount; i++)
    {
        indirectData[i] = DrawElementsIndirectCommand(3, 1, 3 * i, 0, i);
    }

    glGenBuffers(1, &mIndirectBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mIndirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, icSize * indirectData.size(), indirectData.data(),
                 GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    // Draw using glMultiDrawElementsIndirect
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(
        GL_TRIANGLES, GL_UNSIGNED_INT,
        reinterpret_cast<const void *>(static_cast<uintptr_t>(icSize * 2)), triangleCount - 2, 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);

    // Draw using glMultiDrawElementsANGLE
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsANGLE(GL_TRIANGLES, counts.data(), GL_UNSIGNED_INT, indicesDirect.data(),
                             triangleCount);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);

    // Draw using glMultiDrawElementsANGLE again
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsANGLE(GL_TRIANGLES, counts.data(), GL_UNSIGNED_INT, indicesDirect.data(),
                             triangleCount - 1);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);

    // Draw using glMultiDrawElementsIndirect again
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(
        GL_TRIANGLES, GL_UNSIGNED_INT,
        reinterpret_cast<const void *>(static_cast<uintptr_t>(icSize * 3)), triangleCount - 3, 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::transparentBlack);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);

    // Draw using glMultiDrawElementsIndirect one more time
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMultiDrawElementsIndirectEXT(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, triangleCount, 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(0, kHeight - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kWidth - 1, kHeight - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kWidth / 2, kHeight / 2, GLColor::transparentBlack);
}

// Check that glMultiDraw*Instanced without instancing support results in GL_INVALID_OPERATION
TEST_P(MultiDrawNoInstancingSupportTest, InvalidOperation)
{
    ANGLE_SKIP_TEST_IF(IsGLExtensionEnabled("GL_ANGLE_instanced_arrays"));
    requestMultiDrawExtension();
    SetupBuffers();
    SetupProgram();

    GLint first       = 0;
    GLsizei count     = 3;
    GLvoid *indices   = nullptr;
    GLsizei instances = 1;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, mNonIndexedVertexBuffer);
    glEnableVertexAttribArray(mPositionLoc);
    glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glMultiDrawArraysInstancedANGLE(GL_TRIANGLES, &first, &count, &instances, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glEnableVertexAttribArray(mPositionLoc);
    glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glMultiDrawElementsInstancedANGLE(GL_TRIANGLES, &count, GL_UNSIGNED_SHORT, &indices, &instances,
                                      1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

#define ANGLE_ALL_MULTIDRAW_TEST_PLATFORMS_ES2                                             \
    ES2_D3D11().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),                  \
        ES2_OPENGL().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),             \
        ES2_OPENGLES().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),           \
        ES2_VULKAN().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),             \
        ES2_VULKAN_SWIFTSHADER().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions), \
        ES2_METAL().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions)

#define ANGLE_ALL_MULTIDRAW_TEST_PLATFORMS_ES3                                             \
    ES3_D3D11().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),                  \
        ES3_OPENGL().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),             \
        ES3_OPENGLES().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),           \
        ES3_VULKAN().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),             \
        ES3_VULKAN_SWIFTSHADER().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions), \
        ES3_METAL().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions)

#define ANGLE_ALL_MULTIDRAW_TEST_PLATFORMS_ES3_1                                            \
    ES31_D3D11().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),                  \
        ES31_OPENGL().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),             \
        ES31_OPENGLES().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),           \
        ES31_VULKAN().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),             \
        ES31_VULKAN_SWIFTSHADER().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions), \
        ES31_VULKAN()                                                                       \
            .enable(Feature::AlwaysEnableEmulatedMultidrawExtensions)                       \
            .disable(Feature::SupportsMultiDrawIndirect)

ANGLE_INSTANTIATE_TEST_COMBINE_3(MultiDrawTest,
                                 PrintToStringParamName(),
                                 testing::Values(DrawIDOption::NoDrawID, DrawIDOption::UseDrawID),
                                 testing::Values(InstancingOption::NoInstancing,
                                                 InstancingOption::UseInstancing),
                                 testing::Values(BufferDataUsageOption::StaticDraw,
                                                 BufferDataUsageOption::DynamicDraw),
                                 ANGLE_ALL_MULTIDRAW_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_MULTIDRAW_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_3(MultiDrawNoInstancingSupportTest,
                                 PrintToStringParamName(),
                                 testing::Values(DrawIDOption::NoDrawID, DrawIDOption::UseDrawID),
                                 testing::Values(InstancingOption::UseInstancing),
                                 testing::Values(BufferDataUsageOption::StaticDraw,
                                                 BufferDataUsageOption::DynamicDraw),
                                 ANGLE_ALL_MULTIDRAW_TEST_PLATFORMS_ES2);

ANGLE_INSTANTIATE_TEST_COMBINE_3(MultiDrawTestES3,
                                 PrintToStringParamName(),
                                 testing::Values(DrawIDOption::NoDrawID, DrawIDOption::UseDrawID),
                                 testing::Values(InstancingOption::NoInstancing,
                                                 InstancingOption::UseInstancing),
                                 testing::Values(BufferDataUsageOption::StaticDraw,
                                                 BufferDataUsageOption::DynamicDraw),
                                 ANGLE_ALL_MULTIDRAW_TEST_PLATFORMS_ES3);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultiDrawIndirectTest);
ANGLE_INSTANTIATE_TEST(MultiDrawIndirectTest, ANGLE_ALL_MULTIDRAW_TEST_PLATFORMS_ES3_1);
}  // namespace
