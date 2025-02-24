//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DrawBaseVertexBaseInstanceTest: Tests of GL_ANGLE_base_vertex_base_instance
// DrawBaseInstanceTest: Tests of GL_EXT_base_instance

#include "gpu_info_util/SystemInfo.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include <numeric>

using namespace angle;

namespace
{

// Create a kWidth * kHeight canvas equally split into kCountX * kCountY tiles
// each containing a quad partially covering each tile
constexpr uint32_t kWidth                  = 256;
constexpr uint32_t kHeight                 = 256;
constexpr uint32_t kCountX                 = 8;
constexpr uint32_t kCountY                 = 8;
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

enum class BaseVertexOption
{
    NoBaseVertex,
    UseBaseVertex
};

enum class BaseInstanceOption
{
    NoBaseInstance,
    UseBaseInstance
};

enum class BufferDataUsageOption
{
    StaticDraw,
    DynamicDraw
};

using DrawBaseVertexBaseInstanceTestParams = std::
    tuple<angle::PlatformParameters, BaseVertexOption, BaseInstanceOption, BufferDataUsageOption>;

struct PrintToStringParamName
{
    std::string operator()(
        const ::testing::TestParamInfo<DrawBaseVertexBaseInstanceTestParams> &info) const
    {
        ::std::stringstream ss;
        ss << std::get<0>(info.param) << "_"
           << (std::get<3>(info.param) == BufferDataUsageOption::StaticDraw ? "_StaticDraw"
                                                                            : "_DynamicDraw")
           << (std::get<2>(info.param) == BaseInstanceOption::UseBaseInstance ? "_UseBaseInstance"
                                                                              : "")
           << (std::get<1>(info.param) == BaseVertexOption::UseBaseVertex ? "_UseBaseVertex" : "");
        return ss.str();
    }
};

// These tests check correctness of the ANGLE_base_vertex_base_instance extension.
// An array of quads is drawn across the screen.
// gl_VertexID, gl_InstanceID, gl_BaseVertex, and gl_BaseInstance
// are checked by using them to select the color of the draw.
class DrawBaseVertexBaseInstanceTest
    : public ANGLETestBase,
      public ::testing::TestWithParam<DrawBaseVertexBaseInstanceTestParams>
{
  protected:
    DrawBaseVertexBaseInstanceTest() : ANGLETestBase(std::get<0>(GetParam()))
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);

        // Rects in the same column are within a vertex array, testing gl_VertexID, gl_BaseVertex
        // Rects in the same row are drawn by instancing, testing gl_InstanceID, gl_BaseInstance

        mIndices = {0, 1, 2, 0, 2, 3};

        for (uint32_t y = 0; y < kCountY; ++y)
        {
            // v3 ---- v2
            // |       |
            // |       |
            // v0 ---- v1

            const auto vs = getQuadVertices(0, y);

            for (const auto &v : vs)
            {
                mVertices.insert(mVertices.end(), v.begin(), v.end());
            }

            for (GLushort i : mIndices)
            {
                mNonIndexedVertices.insert(mNonIndexedVertices.end(), vs[i].begin(), vs[i].end());
            }
        }

        mRegularIndices.resize(kCountY * mIndices.size());
        for (uint32_t i = 0; i < kCountY; i++)
        {
            uint32_t oi = 6 * i;
            uint32_t ov = 4 * i;
            for (uint32_t j = 0; j < 6; j++)
            {
                mRegularIndices[oi + j] = mIndices[j] + ov;
            }
        }

        std::iota(mInstancedArrayId.begin(), mInstancedArrayId.end(), 0.0f);
        std::reverse_copy(mInstancedArrayId.begin(), mInstancedArrayId.end(),
                          mInstancedArrayColorId.begin());
    }

    void SetUp() override { ANGLETestBase::ANGLETestSetUp(); }

    bool useBaseVertexBuiltin() const
    {
        return std::get<1>(GetParam()) == BaseVertexOption::UseBaseVertex;
    }

    bool useBaseInstanceBuiltin() const
    {
        return std::get<2>(GetParam()) == BaseInstanceOption::UseBaseInstance;
    }

    GLenum getBufferDataUsage() const
    {
        return std::get<3>(GetParam()) == BufferDataUsageOption::StaticDraw ? GL_STATIC_DRAW
                                                                            : GL_DYNAMIC_DRAW;
    }

    std::string vertexShaderSource300(bool isDrawArrays, bool isMultiDraw, bool divisorTest)
    {
        // Each color channel is to test the value of
        // R: gl_InstanceID and gl_BaseInstance
        // G: gl_VertexID and gl_BaseVertex
        // B: gl_BaseVertex

        std::stringstream shader;
        shader << ("#version 300 es\n")
               << (isMultiDraw ? "#extension GL_ANGLE_multi_draw : require\n" : "")
               << ("#extension GL_ANGLE_base_vertex_base_instance_shader_builtin : require\n")
               << "#define kCountX " << kCountX << "\n"
               << "#define kCountY " << kCountY << "\n"
               << R"(
in vec2 vPosition;
)" << (useBaseInstanceBuiltin() ? "" : "in float vInstanceID;\n")
               << (!divisorTest ? "" : "in float vInstanceColorID;\n") << R"(
out vec4 color;
void main()
{
    const float xStep = 1.0 / float(kCountX);
    const float yStep = 1.0 / float(kCountY);
    float x_id = )"
               << (useBaseInstanceBuiltin() ? " float(gl_InstanceID + gl_BaseInstance);"
                                            : "vInstanceID;")
               << "float x_color = "
               << (divisorTest ? "xStep * (vInstanceColorID + 1.0f);" : " 1.0 - xStep * x_id;")
               << R"(
    float y_id = float(gl_VertexID / )"
               << (isDrawArrays ? "6" : "4") << R"();

    color = vec4(
        x_color,
        1.0 - yStep * y_id,
        )" << (useBaseVertexBuiltin() ? "1.0 - yStep * float(gl_BaseVertex) / 4.0" : "1.0")
               << R"(,
        1);

    mat3 transform = mat3(1.0);
    transform[2][0] = x_id * xStep;

    gl_Position = vec4(transform * vec3(vPosition, 1.0) * 2.0 - 1.0, 1);
})";

        return shader.str();
    }

    std::string fragmentShaderSource300()
    {
        return
            R"(#version 300 es
precision mediump float;
in vec4 color;
out vec4 o_color;
void main()
{
    o_color = color;
})";
    }

    void setupProgram(GLProgram &program,
                      bool isDrawArrays  = true,
                      bool isMultiDraw   = false,
                      bool isDivisorTest = false)
    {
        program.makeRaster(vertexShaderSource300(isDrawArrays, isMultiDraw, isDivisorTest).c_str(),
                           fragmentShaderSource300().c_str());
        EXPECT_GL_NO_ERROR();
        ASSERT_TRUE(program.valid());
        glUseProgram(program);
        mPositionLoc = glGetAttribLocation(program, "vPosition");
        if (!useBaseInstanceBuiltin())
        {
            mInstanceIDLoc      = glGetAttribLocation(program, "vInstanceID");
            mInstanceColorIDLoc = glGetAttribLocation(program, "vInstanceColorID");
        }
    }

    void setupNonIndexedBuffers(GLBuffer &vertexBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mNonIndexedVertices.size(),
                     mNonIndexedVertices.data(), getBufferDataUsage());

        ASSERT_GL_NO_ERROR();
    }

    void setupIndexedBuffers(GLBuffer &vertexBuffer, GLBuffer &indexBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mVertices.size(), mVertices.data(),
                     getBufferDataUsage());

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * mIndices.size(), mIndices.data(),
                     getBufferDataUsage());

        ASSERT_GL_NO_ERROR();
    }

    void setupInstanceIDBuffer(GLBuffer &instanceIDBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, instanceIDBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mInstancedArrayId.size(),
                     mInstancedArrayId.data(), getBufferDataUsage());

        ASSERT_GL_NO_ERROR();
    }

    void setupInstanceColorIDBuffer(GLBuffer &instanceIDBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, instanceIDBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mInstancedArrayColorId.size(),
                     mInstancedArrayColorId.data(), getBufferDataUsage());

        ASSERT_GL_NO_ERROR();
    }

    void setupRegularIndexedBuffer(GLBuffer &indexBuffer)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * mRegularIndices.size(),
                     mRegularIndices.data(), getBufferDataUsage());

        ASSERT_GL_NO_ERROR();
    }

    void setupPositionVertexAttribPointer()
    {
        glEnableVertexAttribArray(mPositionLoc);
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    void setupInstanceIDVertexAttribPointer(GLuint instanceIDLoc)
    {
        glEnableVertexAttribArray(instanceIDLoc);
        glVertexAttribPointer(instanceIDLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribDivisor(instanceIDLoc, 1);
    }

    void doDrawArraysInstancedBaseInstance()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const uint32_t countPerDraw = kCountY * 6;

        for (uint32_t i = 0; i < kCountX; i += 2)
        {
            glDrawArraysInstancedBaseInstanceANGLE(GL_TRIANGLES, 0, countPerDraw, 2, i);
        }
    }

    void doMultiDrawArraysInstancedBaseInstance()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const uint32_t countPerDraw = kCountY * 6;

        const GLsizei drawCount = kCountX / 2;
        const std::vector<GLsizei> counts(drawCount, countPerDraw);
        const std::vector<GLsizei> firsts(drawCount, 0);
        const std::vector<GLsizei> instanceCounts(drawCount, 2);
        std::vector<GLuint> baseInstances(drawCount);
        for (size_t i = 0; i < drawCount; i++)
        {
            baseInstances[i] = i * 2;
        }
        glMultiDrawArraysInstancedBaseInstanceANGLE(GL_TRIANGLES, firsts.data(), counts.data(),
                                                    instanceCounts.data(), baseInstances.data(),
                                                    drawCount);
    }

    void doDrawElementsInstancedBaseVertexBaseInstance()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const uint32_t countPerDraw = 6;

        for (uint32_t v = 0; v < kCountY; v++)
        {
            for (uint32_t i = 0; i < kCountX; i += 2)
            {
                glDrawElementsInstancedBaseVertexBaseInstanceANGLE(
                    GL_TRIANGLES, countPerDraw, GL_UNSIGNED_SHORT,
                    reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(0)), 2, v * 4, i);
            }
        }
    }

    // Call this after the *BaseVertexBaseInstance draw call to check if value of BaseVertex and
    // BaseInstance are reset to zero
    void doDrawArraysBaseInstanceReset()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6 * kCountY, 1);
    }

    void doDrawElementsBaseVertexBaseInstanceReset()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElementsInstanced(GL_TRIANGLES, 6 * kCountY, GL_UNSIGNED_SHORT,
                                reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(0)), 1);
    }

    void doMultiDrawElementsInstancedBaseVertexBaseInstance()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const GLsizei drawCount = kCountX * kCountY / 2;
        const std::vector<GLsizei> counts(drawCount, 6);
        const std::vector<GLsizei> instanceCounts(drawCount, 2);
        const std::vector<GLvoid *> indices(drawCount, 0);
        std::vector<GLint> baseVertices(drawCount);
        std::vector<GLuint> baseInstances(drawCount);

        GLsizei b = 0;
        for (uint32_t v = 0; v < kCountY; v++)
        {
            for (uint32_t i = 0; i < kCountX; i += 2)
            {
                baseVertices[b]  = v * 4;
                baseInstances[b] = i;
                b++;
            }
        }

        glMultiDrawElementsInstancedBaseVertexBaseInstanceANGLE(
            GL_TRIANGLES, counts.data(), GL_UNSIGNED_SHORT, indices.data(), instanceCounts.data(),
            baseVertices.data(), baseInstances.data(), drawCount);
    }

    void checkDrawResult(bool hasBaseVertex, bool oneColumn = false)
    {
        uint32_t numColums = oneColumn ? 1 : kCountX;
        for (uint32_t y = 0; y < kCountY; ++y)
        {
            for (uint32_t x = 0; x < numColums; ++x)
            {
                uint32_t center_x = x * kTilePixelSize[0] + kTilePixelSize[0] / 2;
                uint32_t center_y = y * kTilePixelSize[1] + kTilePixelSize[1] / 2;

                EXPECT_PIXEL_NEAR(center_x - kPixelCheckSize[0] / 2,
                                  center_y - kPixelCheckSize[1] / 2,
                                  256.0 * (1.0 - (float)x / (float)kCountX),
                                  256.0 * (1.0 - (float)y / (float)kCountY),
                                  useBaseVertexBuiltin() && hasBaseVertex && !oneColumn
                                      ? 256.0 * (1.0 - (float)y / (float)kCountY)
                                      : 255,
                                  255, 3);
            }
        }
    }

    void TearDown() override { ANGLETestBase::ANGLETestTearDown(); }

    bool requestDrawBaseVertexBaseInstanceExtension()
    {
        if (IsGLExtensionRequestable("GL_ANGLE_base_vertex_base_instance"))
        {
            glRequestExtensionANGLE("GL_ANGLE_base_vertex_base_instance");
        }

        if (!IsGLExtensionEnabled("GL_ANGLE_base_vertex_base_instance"))
        {
            return false;
        }

        if (IsGLExtensionRequestable("GL_ANGLE_base_vertex_base_instance_shader_builtin"))
        {
            glRequestExtensionANGLE("GL_ANGLE_base_vertex_base_instance_shader_builtin");
        }

        return true;
    }

    bool requestInstancedExtension()
    {
        if (IsGLExtensionRequestable("GL_ANGLE_instanced_arrays"))
        {
            glRequestExtensionANGLE("GL_ANGLE_instanced_arrays");
        }

        if (!IsGLExtensionEnabled("GL_ANGLE_instanced_arrays"))
        {
            return false;
        }

        return true;
    }

    bool requestExtensions()
    {
        if (getClientMajorVersion() <= 2)
        {
            if (!requestInstancedExtension())
            {
                return false;
            }
        }
        return requestDrawBaseVertexBaseInstanceExtension();
    }

    // Used for base vertex base instance draw calls
    std::vector<GLushort> mIndices;
    std::vector<GLfloat> mVertices;
    std::vector<GLfloat> mNonIndexedVertices;
    // Used when gl_BaseInstance is not used
    std::array<GLfloat, kCountX> mInstancedArrayId      = {};
    std::array<GLfloat, kCountX> mInstancedArrayColorId = {};
    // Used for regular draw calls without base vertex base instance
    std::vector<GLushort> mRegularIndices;
    GLint mPositionLoc         = 0;
    GLuint mInstanceIDLoc      = 0;
    GLuint mInstanceColorIDLoc = 0;
};

using DrawBaseInstanceTestParams = std::
    tuple<angle::PlatformParameters, BaseVertexOption, BaseInstanceOption, BufferDataUsageOption>;

// The tests in DrawBaseInstanceTest check the correctness
// of the EXT_base_instance extension.
// gl_VertexID, gl_InstanceID, gl_BaseVertex, and gl_BaseInstance
// are checked by using them to select the color of the draw.
class DrawBaseInstanceTest : public ANGLETestBase,
                             public ::testing::TestWithParam<DrawBaseInstanceTestParams>
{
  protected:
    DrawBaseInstanceTest() : ANGLETestBase(std::get<0>(GetParam()))
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);

        // Rects in the same column are within a vertex array, testing gl_VertexID, gl_BaseVertex
        // Rects in the same row are drawn by instancing, testing gl_InstanceID, gl_BaseInstance

        mIndices = {0, 1, 2, 0, 2, 3};

        for (uint32_t y = 0; y < kCountY; ++y)
        {
            // v3 ---- v2
            // |       |
            // |       |
            // v0 ---- v1

            const auto vs = getQuadVertices(0, y);

            for (const auto &v : vs)
            {
                mVertices.insert(mVertices.end(), v.begin(), v.end());
            }

            for (GLushort i : mIndices)
            {
                mNonIndexedVertices.insert(mNonIndexedVertices.end(), vs[i].begin(), vs[i].end());
            }
        }

        mRegularIndices.resize(kCountY * mIndices.size());
        for (uint32_t i = 0; i < kCountY; i++)
        {
            uint32_t oi = 6 * i;
            uint32_t ov = 4 * i;
            for (uint32_t j = 0; j < 6; j++)
            {
                mRegularIndices[oi + j] = mIndices[j] + ov;
            }
        }

        std::iota(mInstancedArrayId.begin(), mInstancedArrayId.end(), 0.0f);
        std::reverse_copy(mInstancedArrayId.begin(), mInstancedArrayId.end(),
                          mInstancedArrayColorId.begin());
    }

    void SetUp() override { ANGLETestBase::ANGLETestSetUp(); }

    bool useBaseVertexBuiltin() const
    {
        return std::get<1>(GetParam()) == BaseVertexOption::UseBaseVertex;
    }

    bool useBaseInstanceBuiltin() const
    {
        return std::get<2>(GetParam()) == BaseInstanceOption::UseBaseInstance;
    }

    GLenum getBufferDataUsage() const
    {
        return std::get<3>(GetParam()) == BufferDataUsageOption::StaticDraw ? GL_STATIC_DRAW
                                                                            : GL_DYNAMIC_DRAW;
    }

    std::string vertexShaderSource300(bool isDrawArrays, bool divisorTest)
    {
        // Each color channel is to test the value of
        // R: gl_InstanceID and gl_BaseInstance
        // G: gl_VertexID and gl_BaseVertex
        // B: gl_BaseVertex

        std::stringstream shader;
        shader << ("#version 300 es\n")
               << ("#extension GL_ANGLE_base_vertex_base_instance_shader_builtin : require\n")
               << "#define kCountX " << kCountX << "\n"
               << "#define kCountY " << kCountY << "\n"
               << R"(
in vec2 vPosition;
)" << (useBaseInstanceBuiltin() ? "" : "in float vInstanceID;\n")
               << (!divisorTest ? "" : "in float vInstanceColorID;\n") << R"(
out vec4 color;
void main()
{
    const float xStep = 1.0 / float(kCountX);
    const float yStep = 1.0 / float(kCountY);
    float x_id = )"
               << (useBaseInstanceBuiltin() ? " float(gl_InstanceID + gl_BaseInstance);"
                                            : "vInstanceID;")
               << "float x_color = "
               << (divisorTest ? "xStep * (vInstanceColorID + 1.0f);" : " 1.0 - xStep * x_id;")
               << R"(
    float y_id = float(gl_VertexID / )"
               << (isDrawArrays ? "6" : "4") << R"();

    color = vec4(
        x_color,
        1.0 - yStep * y_id,
        )" << (useBaseVertexBuiltin() ? "1.0 - yStep * float(gl_BaseVertex) / 4.0" : "1.0")
               << R"(,
        1);

    mat3 transform = mat3(1.0);
    transform[2][0] = x_id * xStep;

    gl_Position = vec4(transform * vec3(vPosition, 1.0) * 2.0 - 1.0, 1);
})";

        return shader.str();
    }

    std::string fragmentShaderSource300()
    {
        return
            R"(#version 300 es
precision mediump float;
in vec4 color;
out vec4 o_color;
void main()
{
    o_color = color;
})";
    }

    void setupProgram(GLProgram &program, bool isDrawArrays = true, bool isDivisorTest = false)
    {
        program.makeRaster(vertexShaderSource300(isDrawArrays, isDivisorTest).c_str(),
                           fragmentShaderSource300().c_str());
        EXPECT_GL_NO_ERROR();
        ASSERT_TRUE(program.valid());
        glUseProgram(program);
        mPositionLoc = glGetAttribLocation(program, "vPosition");
        if (!useBaseInstanceBuiltin())
        {
            mInstanceIDLoc      = glGetAttribLocation(program, "vInstanceID");
            mInstanceColorIDLoc = glGetAttribLocation(program, "vInstanceColorID");
        }
    }

    void setupNonIndexedBuffers(GLBuffer &vertexBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mNonIndexedVertices.size(),
                     mNonIndexedVertices.data(), getBufferDataUsage());

        ASSERT_GL_NO_ERROR();
    }

    void setupIndexedBuffers(GLBuffer &vertexBuffer, GLBuffer &indexBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mVertices.size(), mVertices.data(),
                     getBufferDataUsage());

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * mIndices.size(), mIndices.data(),
                     getBufferDataUsage());

        ASSERT_GL_NO_ERROR();
    }

    void setupInstanceIDBuffer(GLBuffer &instanceIDBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, instanceIDBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mInstancedArrayId.size(),
                     mInstancedArrayId.data(), getBufferDataUsage());

        ASSERT_GL_NO_ERROR();
    }

    void setupInstanceColorIDBuffer(GLBuffer &instanceIDBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, instanceIDBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mInstancedArrayColorId.size(),
                     mInstancedArrayColorId.data(), getBufferDataUsage());

        ASSERT_GL_NO_ERROR();
    }

    void setupRegularIndexedBuffer(GLBuffer &indexBuffer)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * mRegularIndices.size(),
                     mRegularIndices.data(), getBufferDataUsage());

        ASSERT_GL_NO_ERROR();
    }

    void setupPositionVertexAttribPointer()
    {
        glEnableVertexAttribArray(mPositionLoc);
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    void setupInstanceIDVertexAttribPointer(GLuint instanceIDLoc)
    {
        glEnableVertexAttribArray(instanceIDLoc);
        glVertexAttribPointer(instanceIDLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribDivisor(instanceIDLoc, 1);
    }

    void doDrawArraysInstancedBaseInstance()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const uint32_t countPerDraw = kCountY * 6;

        for (uint32_t i = 0; i < kCountX; i += 2)
        {
            glDrawArraysInstancedBaseInstanceEXT(GL_TRIANGLES, 0, countPerDraw, 2, i);
        }
    }

    void doDrawElementsInstancedBaseInstance()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const uint32_t countPerDraw = 6;

        for (uint32_t v = 0; v < kCountY; v++)
        {
            for (uint32_t i = 0; i < kCountX; i += 2)
            {
                glDrawElementsInstancedBaseInstanceEXT(GL_TRIANGLES, countPerDraw,
                                                       GL_UNSIGNED_SHORT,
                                                       (void *)(v * 6 * sizeof(GLushort)), 2, i);
            }
        }
    }

    void doDrawElementsInstancedBaseVertexBaseInstance()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const uint32_t countPerDraw = 6;

        for (uint32_t v = 0; v < kCountY; v++)
        {
            for (uint32_t i = 0; i < kCountX; i += 2)
            {
                glDrawElementsInstancedBaseVertexBaseInstanceEXT(
                    GL_TRIANGLES, countPerDraw, GL_UNSIGNED_SHORT,
                    reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(0)), 2, v * 4, i);
            }
        }
    }

    // Call this after the *BaseInstance draw call to check
    // if value of BaseInstance is reset to zero
    void doDrawArraysBaseInstanceReset()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6 * kCountY, 1);
    }

    void doDrawElementsInstancedBaseInstanceReset()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElementsInstanced(GL_TRIANGLES, 6 * kCountY, GL_UNSIGNED_SHORT,
                                reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(0)), 1);
    }

    void doDrawElementsBaseVertexBaseInstanceReset()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawElementsInstanced(GL_TRIANGLES, 6 * kCountY, GL_UNSIGNED_SHORT,
                                reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(0)), 1);
    }

    void checkDrawResult(bool hasBaseVertex, bool oneColumn = false)
    {
        uint32_t numColums = oneColumn ? 1 : kCountX;
        for (uint32_t y = 0; y < kCountY; ++y)
        {
            for (uint32_t x = 0; x < numColums; ++x)
            {
                uint32_t center_x = x * kTilePixelSize[0] + kTilePixelSize[0] / 2;
                uint32_t center_y = y * kTilePixelSize[1] + kTilePixelSize[1] / 2;

                EXPECT_PIXEL_NEAR(center_x - kPixelCheckSize[0] / 2,
                                  center_y - kPixelCheckSize[1] / 2,
                                  256.0 * (1.0 - (float)x / (float)kCountX),
                                  256.0 * (1.0 - (float)y / (float)kCountY),
                                  useBaseVertexBuiltin() && hasBaseVertex && !oneColumn
                                      ? 256.0 * (1.0 - (float)y / (float)kCountY)
                                      : 255,
                                  255, 3);
            }
        }
    }

    void TearDown() override { ANGLETestBase::ANGLETestTearDown(); }

    bool requestExtensions()
    {
        if (IsGLExtensionRequestable("GL_ANGLE_base_vertex_base_instance_shader_builtin"))
        {
            glRequestExtensionANGLE("GL_ANGLE_base_vertex_base_instance_shader_builtin");
        }

        if (!IsGLExtensionEnabled("GL_ANGLE_base_vertex_base_instance_shader_builtin"))
        {
            return false;
        }

        return EnsureGLExtensionEnabled("GL_EXT_base_instance");
    }

    // Used for base vertex base instance draw calls
    std::vector<GLushort> mIndices;
    std::vector<GLfloat> mVertices;
    std::vector<GLfloat> mNonIndexedVertices;
    // Used when gl_BaseInstance is not used
    std::array<GLfloat, kCountX> mInstancedArrayId      = {};
    std::array<GLfloat, kCountX> mInstancedArrayColorId = {};
    // Used for regular draw calls without base vertex base instance
    std::vector<GLushort> mRegularIndices;
    GLint mPositionLoc         = 0;
    GLuint mInstanceIDLoc      = 0;
    GLuint mInstanceColorIDLoc = 0;
};

// Tests that compile a program with the extension succeeds
TEST_P(DrawBaseVertexBaseInstanceTest, CanCompile)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    GLProgram p0;
    setupProgram(p0, true, false);
    GLProgram p1;
    setupProgram(p1, true, true);
    GLProgram p2;
    setupProgram(p2, false, false);
    GLProgram p3;
    setupProgram(p3, false, true);
}

// Tests if baseInstance works properly with instanced array with non-zero divisor
TEST_P(DrawBaseVertexBaseInstanceTest, BaseInstanceDivisor)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    ANGLE_SKIP_TEST_IF(useBaseInstanceBuiltin());

    GLProgram program;
    setupProgram(program, true, false, true);

    GLBuffer nonIndexedVertexBuffer;
    setupNonIndexedBuffers(nonIndexedVertexBuffer);
    setupPositionVertexAttribPointer();

    GLBuffer instanceIDBuffer;
    GLBuffer instanceColorIDBuffer;

    setupInstanceIDBuffer(instanceIDBuffer);
    setupInstanceIDVertexAttribPointer(mInstanceIDLoc);
    setupInstanceColorIDBuffer(instanceColorIDBuffer);
    setupInstanceIDVertexAttribPointer(mInstanceColorIDLoc);

    doDrawArraysInstancedBaseInstance();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false);

    doDrawArraysBaseInstanceReset();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false, true);

    GLProgram programIndexed;
    setupProgram(programIndexed, false, false, true);

    GLBuffer indexBuffer;
    GLBuffer vertexBuffer;
    setupIndexedBuffers(vertexBuffer, indexBuffer);
    setupPositionVertexAttribPointer();

    glBindBuffer(GL_ARRAY_BUFFER, instanceIDBuffer);
    setupInstanceIDVertexAttribPointer(mInstanceIDLoc);
    glBindBuffer(GL_ARRAY_BUFFER, instanceColorIDBuffer);
    setupInstanceIDVertexAttribPointer(mInstanceColorIDLoc);

    doDrawElementsInstancedBaseVertexBaseInstance();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(true);

    setupRegularIndexedBuffer(indexBuffer);
    doDrawElementsBaseVertexBaseInstanceReset();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false, true);
}

// Tests basic functionality of glDrawArraysInstancedBaseInstanceANGLE
TEST_P(DrawBaseVertexBaseInstanceTest, DrawArraysInstancedBaseInstance)
{
    // TODO(shrekshao): Temporarily skip this test
    // before we could try updating win AMD bot driver version
    // after Lab team fixed issues with ssh into bot machines
    // Currently this test fail on certain Win7/Win2008Server AMD GPU
    // with driver version 23.20.185.235 when using OpenGL backend.
    // This failure couldn't be produced on local Win10 AMD machine with latest driver installed
    // Same for the MultiDrawArraysInstancedBaseInstance test
    if (IsAMD() && IsWindows() && IsDesktopOpenGL())
    {
        SystemInfo *systemInfo = GetTestSystemInfo();
        if (!systemInfo->gpus.empty())
        {
            ANGLE_SKIP_TEST_IF(0x6613 == systemInfo->gpus[systemInfo->activeGPUIndex].deviceId);
        }
    }

    ANGLE_SKIP_TEST_IF(!requestExtensions());

    GLProgram program;
    setupProgram(program, true);

    GLBuffer vertexBuffer;
    setupNonIndexedBuffers(vertexBuffer);
    setupPositionVertexAttribPointer();

    GLBuffer instanceIDBuffer;
    if (!useBaseInstanceBuiltin())
    {
        setupInstanceIDBuffer(instanceIDBuffer);
        setupInstanceIDVertexAttribPointer(mInstanceIDLoc);
    }

    doDrawArraysInstancedBaseInstance();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false);

    doDrawArraysBaseInstanceReset();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false, true);
}

// Tests basic functionality of glMultiDrawArraysInstancedBaseInstance
TEST_P(DrawBaseVertexBaseInstanceTest, MultiDrawArraysInstancedBaseInstance)
{
    if (IsAMD() && IsWindows() && IsDesktopOpenGL())
    {
        SystemInfo *systemInfo = GetTestSystemInfo();
        if (!(systemInfo->activeGPUIndex < 0 || systemInfo->gpus.empty()))
        {
            ANGLE_SKIP_TEST_IF(0x6613 == systemInfo->gpus[systemInfo->activeGPUIndex].deviceId);
        }
    }

    ANGLE_SKIP_TEST_IF(!requestExtensions());

    GLProgram program;
    setupProgram(program, true, true);

    GLBuffer vertexBuffer;
    setupNonIndexedBuffers(vertexBuffer);
    setupPositionVertexAttribPointer();

    GLBuffer instanceIDBuffer;
    if (!useBaseInstanceBuiltin())
    {
        setupInstanceIDBuffer(instanceIDBuffer);
        setupInstanceIDVertexAttribPointer(mInstanceIDLoc);
    }

    doMultiDrawArraysInstancedBaseInstance();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false);

    doDrawArraysBaseInstanceReset();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false, true);
}

// Tests basic functionality of glDrawElementsInstancedBaseVertexBaseInstanceANGLE
TEST_P(DrawBaseVertexBaseInstanceTest, DrawElementsInstancedBaseVertexBaseInstance)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());

    GLProgram program;
    setupProgram(program, false);

    GLBuffer indexBuffer;
    GLBuffer vertexBuffer;
    setupIndexedBuffers(vertexBuffer, indexBuffer);
    setupPositionVertexAttribPointer();

    GLBuffer instanceIDBuffer;
    if (!useBaseInstanceBuiltin())
    {
        setupInstanceIDBuffer(instanceIDBuffer);
        setupInstanceIDVertexAttribPointer(mInstanceIDLoc);
    }

    doDrawElementsInstancedBaseVertexBaseInstance();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(true);

    setupRegularIndexedBuffer(indexBuffer);
    doDrawElementsBaseVertexBaseInstanceReset();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(true, true);
}

// Tests basic functionality of glMultiDrawElementsInstancedBaseVertexBaseInstance
TEST_P(DrawBaseVertexBaseInstanceTest, MultiDrawElementsInstancedBaseVertexBaseInstance)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());

    GLProgram program;
    setupProgram(program, false, true);

    GLBuffer indexBuffer;
    GLBuffer vertexBuffer;
    setupIndexedBuffers(vertexBuffer, indexBuffer);
    setupPositionVertexAttribPointer();

    GLBuffer instanceIDBuffer;
    if (!useBaseInstanceBuiltin())
    {
        setupInstanceIDBuffer(instanceIDBuffer);
        setupInstanceIDVertexAttribPointer(mInstanceIDLoc);
    }

    doMultiDrawElementsInstancedBaseVertexBaseInstance();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(true);

    setupRegularIndexedBuffer(indexBuffer);
    doDrawElementsBaseVertexBaseInstanceReset();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(true, true);
}

// Tests if baseInstance works properly with instanced array with non-zero divisor
TEST_P(DrawBaseInstanceTest, BaseInstanceDivisor)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    ANGLE_SKIP_TEST_IF(useBaseInstanceBuiltin());

    GLProgram program;
    setupProgram(program, true, true);

    GLBuffer nonIndexedVertexBuffer;
    setupNonIndexedBuffers(nonIndexedVertexBuffer);
    setupPositionVertexAttribPointer();

    GLBuffer instanceIDBuffer;
    GLBuffer instanceColorIDBuffer;

    setupInstanceIDBuffer(instanceIDBuffer);
    setupInstanceIDVertexAttribPointer(mInstanceIDLoc);
    setupInstanceColorIDBuffer(instanceColorIDBuffer);
    setupInstanceIDVertexAttribPointer(mInstanceColorIDLoc);

    doDrawArraysInstancedBaseInstance();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false);

    doDrawArraysBaseInstanceReset();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false, true);

    GLProgram programIndexed;
    setupProgram(programIndexed, false, true);

    GLBuffer indexBuffer;
    GLBuffer vertexBuffer;
    setupIndexedBuffers(vertexBuffer, indexBuffer);
    setupPositionVertexAttribPointer();

    glBindBuffer(GL_ARRAY_BUFFER, instanceIDBuffer);
    setupInstanceIDVertexAttribPointer(mInstanceIDLoc);
    glBindBuffer(GL_ARRAY_BUFFER, instanceColorIDBuffer);
    setupInstanceIDVertexAttribPointer(mInstanceColorIDLoc);

    doDrawElementsInstancedBaseVertexBaseInstance();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(true);

    setupRegularIndexedBuffer(indexBuffer);
    doDrawElementsBaseVertexBaseInstanceReset();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false, true);
}

// Tests basic functionality of glDrawArraysInstancedBaseInstanceEXT
TEST_P(DrawBaseInstanceTest, DrawArraysInstancedBaseInstance)
{
    // This test fails on certain Win7/Win2008Server AMD GPU
    // with driver version 23.20.185.235 when using OpenGL backend.
    // See comments under the ANGLE test of DrawArraysInstancedBaseInstance above.
    if (IsAMD() && IsWindows() && IsDesktopOpenGL())
    {
        SystemInfo *systemInfo = GetTestSystemInfo();
        if (!systemInfo->gpus.empty())
        {
            ANGLE_SKIP_TEST_IF(0x6613 == systemInfo->gpus[systemInfo->activeGPUIndex].deviceId);
        }
    }

    ANGLE_SKIP_TEST_IF(!requestExtensions());

    GLProgram program;
    setupProgram(program);

    GLBuffer vertexBuffer;
    setupNonIndexedBuffers(vertexBuffer);
    setupPositionVertexAttribPointer();

    GLBuffer instanceIDBuffer;
    if (!useBaseInstanceBuiltin())
    {
        setupInstanceIDBuffer(instanceIDBuffer);
        setupInstanceIDVertexAttribPointer(mInstanceIDLoc);
    }

    doDrawArraysInstancedBaseInstance();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false);

    doDrawArraysBaseInstanceReset();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false, true);
}

// Tests basic functionality of glDrawElementsInstancedBaseInstanceEXT
TEST_P(DrawBaseInstanceTest, DrawElementsInstancedBaseInstance)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());

    GLProgram program;
    setupProgram(program, false);

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mVertices.size(), mVertices.data(),
                 getBufferDataUsage());
    EXPECT_GL_NO_ERROR();
    setupPositionVertexAttribPointer();

    GLBuffer indexBuffer;
    setupRegularIndexedBuffer(indexBuffer);

    GLBuffer instanceIDBuffer;
    if (!useBaseInstanceBuiltin())
    {
        setupInstanceIDBuffer(instanceIDBuffer);
        setupInstanceIDVertexAttribPointer(mInstanceIDLoc);
    }

    doDrawElementsInstancedBaseInstance();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false);

    doDrawElementsInstancedBaseInstanceReset();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(false, true);
}

// Tests basic functionality of glDrawElementsInstancedBaseVertexBaseInstanceEXT
TEST_P(DrawBaseInstanceTest, DrawElementsInstancedBaseVertexBaseInstance)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());

    GLProgram program;
    setupProgram(program, false);

    GLBuffer indexBuffer;
    GLBuffer vertexBuffer;
    setupIndexedBuffers(vertexBuffer, indexBuffer);
    setupPositionVertexAttribPointer();

    GLBuffer instanceIDBuffer;
    if (!useBaseInstanceBuiltin())
    {
        setupInstanceIDBuffer(instanceIDBuffer);
        setupInstanceIDVertexAttribPointer(mInstanceIDLoc);
    }

    doDrawElementsInstancedBaseVertexBaseInstance();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(true);

    setupRegularIndexedBuffer(indexBuffer);
    doDrawElementsBaseVertexBaseInstanceReset();
    EXPECT_GL_NO_ERROR();
    checkDrawResult(true, true);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DrawBaseVertexBaseInstanceTest);

#define ANGLE_ALL_BASEVERTEXBASEINTANCE_TEST_PLATFORMS_ES3                                 \
    ES3_D3D11().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),                  \
        ES3_OPENGL().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),             \
        ES3_OPENGLES().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),           \
        ES3_VULKAN().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions),             \
        ES3_VULKAN_SWIFTSHADER().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions), \
        ES3_METAL().enable(Feature::AlwaysEnableEmulatedMultidrawExtensions)

ANGLE_INSTANTIATE_TEST_COMBINE_3(
    DrawBaseVertexBaseInstanceTest,
    PrintToStringParamName(),
    testing::Values(BaseVertexOption::NoBaseVertex, BaseVertexOption::UseBaseVertex),
    testing::Values(BaseInstanceOption::NoBaseInstance, BaseInstanceOption::UseBaseInstance),
    testing::Values(BufferDataUsageOption::StaticDraw, BufferDataUsageOption::DynamicDraw),
    ANGLE_ALL_BASEVERTEXBASEINTANCE_TEST_PLATFORMS_ES3);

ANGLE_INSTANTIATE_TEST_COMBINE_3(
    DrawBaseInstanceTest,
    PrintToStringParamName(),
    testing::Values(BaseVertexOption::NoBaseVertex, BaseVertexOption::UseBaseVertex),
    testing::Values(BaseInstanceOption::NoBaseInstance, BaseInstanceOption::UseBaseInstance),
    testing::Values(BufferDataUsageOption::StaticDraw, BufferDataUsageOption::DynamicDraw),
    ANGLE_ALL_BASEVERTEXBASEINTANCE_TEST_PLATFORMS_ES3);

}  // namespace
