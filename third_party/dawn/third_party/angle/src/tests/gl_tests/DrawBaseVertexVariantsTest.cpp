//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DrawBaseVertexVariantsTest: Tests variants of drawElements*BaseVertex* call from different
// extensions

#include "gpu_info_util/SystemInfo.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include <numeric>

using namespace angle;

namespace
{

// Create a kWidth * kHeight canvas equally split into kCountX * kCountY tiles
// each containing a quad partially covering each tile
constexpr uint32_t kWidth                   = 256;
constexpr uint32_t kHeight                  = 256;
constexpr uint32_t kCountX                  = 8;
constexpr uint32_t kCountY                  = 8;
constexpr int kBoxCount                     = kCountX * kCountY;
constexpr uint32_t kIndexPatternRepeatCount = 3;
constexpr std::array<GLfloat, 2> kTileSize  = {
    1.f / static_cast<GLfloat>(kCountX),
    1.f / static_cast<GLfloat>(kCountY),
};
constexpr std::array<uint32_t, 2> kTilePixelSize  = {kWidth / kCountX, kHeight / kCountY};
constexpr std::array<GLfloat, 2> kQuadRadius      = {0.25f * kTileSize[0], 0.25f * kTileSize[1]};
constexpr std::array<uint32_t, 2> kPixelCheckSize = {
    static_cast<uint32_t>(kQuadRadius[0] * kWidth),
    static_cast<uint32_t>(kQuadRadius[1] * kHeight)};
constexpr GLenum kBufferDataUsage[] = {GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW};

constexpr std::array<GLfloat, 2> GetTileCenter(uint32_t x, uint32_t y)
{
    return {
        kTileSize[0] * (0.5f + static_cast<GLfloat>(x)),
        kTileSize[1] * (0.5f + static_cast<GLfloat>(y)),
    };
}
constexpr std::array<std::array<GLfloat, 2>, 4> GetQuadVertices(uint32_t x, uint32_t y)
{
    const auto center = GetTileCenter(x, y);
    return {
        std::array<GLfloat, 2>{center[0] - kQuadRadius[0], center[1] - kQuadRadius[1]},
        std::array<GLfloat, 2>{center[0] + kQuadRadius[0], center[1] - kQuadRadius[1]},
        std::array<GLfloat, 2>{center[0] + kQuadRadius[0], center[1] + kQuadRadius[1]},
        std::array<GLfloat, 2>{center[0] - kQuadRadius[0], center[1] + kQuadRadius[1]},
    };
}

enum class DrawCallVariants
{
    DrawElementsBaseVertex,
    DrawElementsInstancedBaseVertex,
    DrawRangeElementsBaseVertex,
    DrawElementsInstancedBaseVertexBaseInstance
};

using DrawBaseVertexVariantsTestParams = std::tuple<angle::PlatformParameters, GLenum>;

std::string DrawBaseVertexVariantsTestPrint(
    const ::testing::TestParamInfo<DrawBaseVertexVariantsTestParams> &paramsInfo)
{
    const DrawBaseVertexVariantsTestParams &params = paramsInfo.param;
    std::ostringstream out;

    out << std::get<0>(params) << "__";

    switch (std::get<1>(params))
    {
        case GL_STATIC_DRAW:
            out << "STATIC_DRAW";
            break;
        case GL_DYNAMIC_DRAW:
            out << "DYNAMIC_DRAW";
            break;
        case GL_STREAM_DRAW:
            out << "STREAM_DRAW";
            break;
        default:
            out << "UPDATE_THIS_SWITCH";
            break;
    }

    return out.str();
}

// These tests check correctness of variants of baseVertex draw calls from different extensions

class DrawBaseVertexVariantsTest : public ANGLETest<DrawBaseVertexVariantsTestParams>
{
  protected:
    DrawBaseVertexVariantsTest()
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);

        std::array<GLushort, 6> indices = {0, 1, 2, 0, 2, 3};
        mIndices.resize(indices.size() * kIndexPatternRepeatCount);
        for (uint32_t i = 0; i < kIndexPatternRepeatCount; i++)
        {
            size_t o  = i * indices.size();
            size_t vo = i * 4;  // each quad has 4 vertices, index offset by 4
            for (size_t j = 0; j < indices.size(); j++)
            {
                mIndices[o + j] = vo + indices[j];
            }
        }

        mColorPalette = {GLColor(0x7f, 0x7f, 0x7f, 0xff),
                         GLColor::red,
                         GLColor::green,
                         GLColor::yellow,
                         GLColor::blue,
                         GLColor::magenta,
                         GLColor::cyan,
                         GLColor::white};

        for (uint32_t y = 0; y < kCountY; ++y)
        {
            for (uint32_t x = 0; x < kCountX; ++x)
            {
                // v3 ---- v2
                // |       |
                // |       |
                // v0 ---- v1

                const auto vs = ::GetQuadVertices(x, y);

                for (const auto &v : vs)
                {
                    mVertices.insert(mVertices.end(), v.begin(), v.end());
                }

                const auto &colorPicked = mColorPalette[(x + y) % mColorPalette.size()];
                for (int i = 0; i < 4; ++i)
                {
                    mVertexColors.push_back(colorPicked.R);
                    mVertexColors.push_back(colorPicked.G);
                    mVertexColors.push_back(colorPicked.B);
                    mVertexColors.push_back(colorPicked.A);
                }
            }
        }

        mRegularIndices.resize(kCountY * kCountX * mIndices.size());
        for (uint32_t y = 0; y < kCountY; y++)
        {
            for (uint32_t x = 0; x < kCountX; x++)
            {
                uint32_t i  = x + y * kCountX;
                uint32_t oi = 6 * i;
                uint32_t ov = 4 * i;
                for (uint32_t j = 0; j < 6; j++)
                {
                    mRegularIndices[oi + j] = mIndices[j] + ov;
                }
            }
        }
    }

    void setupProgram(GLProgram &program)
    {
        constexpr char vs[] = R"(
precision mediump float;
attribute vec2 vPosition;
attribute vec4 vColor;
varying vec4 color;
void main()
{
    gl_Position = vec4(vec3(vPosition, 1.0) * 2.0 - 1.0, 1.0);
    color = vColor;
})";
        constexpr char fs[] = R"(
precision mediump float;
varying vec4 color;
void main()
{
    gl_FragColor = color;
})";
        program.makeRaster(vs, fs);
        EXPECT_GL_NO_ERROR();
        ASSERT_TRUE(program.valid());
        glUseProgram(program);
        mPositionLoc = glGetAttribLocation(program, "vPosition");
        ASSERT_NE(-1, mPositionLoc);
        mColorLoc = glGetAttribLocation(program, "vColor");
        ASSERT_NE(-1, mColorLoc);
    }

    void setupIndexedBuffers(GLBuffer &vertexPositionBuffer,
                             GLBuffer &vertexColorBuffer,
                             GLBuffer &indexBuffer)
    {
        GLenum usage = std::get<1>(GetParam());

        glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLubyte) * mVertexColors.size(), mVertexColors.data(),
                     usage);

        glEnableVertexAttribArray(mColorLoc);
        glVertexAttribPointer(mColorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, vertexPositionBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mVertices.size(), mVertices.data(), usage);

        glEnableVertexAttribArray(mPositionLoc);
        glVertexAttribPointer(mPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * mIndices.size(), mIndices.data(),
                     usage);

        ASSERT_GL_NO_ERROR();
    }

    void doDrawElementsBaseVertexVariants(DrawCallVariants drawCallType)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int baseRepetition = 0;
        int i              = 0;

        // Start at various repetitions within the patterned index buffer to exercise base
        // index.

        static_assert(kIndexPatternRepeatCount >= 3, "Repeat pattern count should be at least 3");

        while (i < kBoxCount)
        {
            int repetitionCount = std::min(3 - baseRepetition, kBoxCount - i);

            updateVertexColorData(i, repetitionCount);

            switch (drawCallType)
            {
                case DrawCallVariants::DrawElementsInstancedBaseVertexBaseInstance:
                    glDrawElementsInstancedBaseVertexBaseInstanceANGLE(
                        GL_TRIANGLES, repetitionCount * 6, GL_UNSIGNED_SHORT,
                        reinterpret_cast<GLvoid *>(
                            static_cast<uintptr_t>(baseRepetition * 6 * sizeof(GLushort))),
                        1, (i - baseRepetition) * 4, 0);
                    break;
                case DrawCallVariants::DrawElementsBaseVertex:
                    glDrawElementsBaseVertexEXT(
                        GL_TRIANGLES, repetitionCount * 6, GL_UNSIGNED_SHORT,
                        reinterpret_cast<GLvoid *>(
                            static_cast<uintptr_t>(baseRepetition * 6 * sizeof(GLushort))),
                        (i - baseRepetition) * 4);
                    break;
                case DrawCallVariants::DrawElementsInstancedBaseVertex:
                    glDrawElementsInstancedBaseVertexEXT(
                        GL_TRIANGLES, repetitionCount * 6, GL_UNSIGNED_SHORT,
                        reinterpret_cast<GLvoid *>(
                            static_cast<uintptr_t>(baseRepetition * 6 * sizeof(GLushort))),
                        1, (i - baseRepetition) * 4);
                    break;
                case DrawCallVariants::DrawRangeElementsBaseVertex:
                    glDrawRangeElementsBaseVertexEXT(
                        GL_TRIANGLES, baseRepetition * 4,
                        (baseRepetition + repetitionCount) * 4 - 1, repetitionCount * 6,
                        GL_UNSIGNED_SHORT,
                        reinterpret_cast<GLvoid *>(
                            static_cast<uintptr_t>(baseRepetition * 6 * sizeof(GLushort))),
                        (i - baseRepetition) * 4);
                    break;
                default:
                    EXPECT_TRUE(false);
                    break;
            }

            baseRepetition = (baseRepetition + 1) % 3;
            i += repetitionCount;
        }

        EXPECT_GL_NO_ERROR();
        checkDrawResult();
    }

    void updateVertexColorData(GLint drawnQuadCount, GLint toDrawQuadCount)
    {
        // update the vertex color of the next [count] of quads to draw
        if (std::get<1>(GetParam()) == GL_STATIC_DRAW)
        {
            return;
        }

        GLint offset = sizeof(GLubyte) * drawnQuadCount * 4 * sizeof(GLColor);

        for (GLint i = 0; i < toDrawQuadCount; i++)
        {
            const GLColor &color = mColorPalette[(drawnQuadCount + i) % mColorPalette.size()];
            for (GLint j = 0; j < 4; j++)
            {
                glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(GLColor), color.data());
                offset += sizeof(GLColor);
            }
        }
    }

    void checkDrawResult()
    {
        bool dynamicLayout = std::get<1>(GetParam()) == GL_STATIC_DRAW ? false : true;

        for (uint32_t y = 0; y < kCountY; ++y)
        {
            for (uint32_t x = 0; x < kCountX; ++x)
            {
                uint32_t center_x = x * kTilePixelSize[0] + kTilePixelSize[0] / 2;
                uint32_t center_y = y * kTilePixelSize[1] + kTilePixelSize[1] / 2;

                const auto &color =
                    mColorPalette[(dynamicLayout ? x : x + y) % mColorPalette.size()];

                EXPECT_PIXEL_NEAR(center_x - kPixelCheckSize[0] / 2,
                                  center_y - kPixelCheckSize[1] / 2, color[0], color[1], color[2],
                                  color[3], 1);
            }
        }
    }

    bool requestAngleBaseVertexBaseInstanceExtensions()
    {
        if (getClientMajorVersion() <= 2)
        {
            if (!EnsureGLExtensionEnabled("GL_ANGLE_instanced_arrays"))
            {
                return false;
            }
        }
        if (!EnsureGLExtensionEnabled("GL_ANGLE_base_vertex_base_instance"))
        {
            return false;
        }

        return EnsureGLExtensionEnabled("GL_ANGLE_base_vertex_base_instance_shader_builtin");
    }

    bool requestNativeBaseVertexExtensions()
    {
        return (EnsureGLExtensionEnabled("GL_OES_draw_elements_base_vertex") ||
                EnsureGLExtensionEnabled("GL_EXT_draw_elements_base_vertex"));
    }

    std::vector<GLushort> mIndices;
    std::vector<GLfloat> mVertices;
    std::vector<GLubyte> mVertexColors;

    std::vector<GLColor> mColorPalette;
    std::vector<GLushort> mRegularIndices;
    GLint mPositionLoc;
    GLint mColorLoc;
};

// Test drawElementsBaseVertex from OES/EXT_draw_elements_base_vertex
TEST_P(DrawBaseVertexVariantsTest, DrawElementsBaseVertex)
{
    ANGLE_SKIP_TEST_IF(!requestNativeBaseVertexExtensions());

    GLProgram program;
    setupProgram(program);

    GLBuffer indexBuffer;
    GLBuffer vertexPositionBuffer;
    GLBuffer vertexColorBuffer;
    setupIndexedBuffers(vertexPositionBuffer, vertexColorBuffer, indexBuffer);

    // for potential update vertex color later
    glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer);

    doDrawElementsBaseVertexVariants(DrawCallVariants::DrawElementsBaseVertex);
}

// Test drawElementsInstancedBaseVertex from OES/EXT_draw_elements_base_vertex
TEST_P(DrawBaseVertexVariantsTest, DrawElementsInstancedBaseVertex)
{
    ANGLE_SKIP_TEST_IF(!requestNativeBaseVertexExtensions());

    GLProgram program;
    setupProgram(program);

    GLBuffer indexBuffer;
    GLBuffer vertexPositionBuffer;
    GLBuffer vertexColorBuffer;
    setupIndexedBuffers(vertexPositionBuffer, vertexColorBuffer, indexBuffer);

    // for potential update vertex color later
    glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer);

    doDrawElementsBaseVertexVariants(DrawCallVariants::DrawElementsInstancedBaseVertex);
}

// Test drawRangeElementsBaseVertex from OES/EXT_draw_elements_base_vertex
TEST_P(DrawBaseVertexVariantsTest, DrawRangeElementsBaseVertex)
{
    ANGLE_SKIP_TEST_IF(!requestNativeBaseVertexExtensions());

    GLProgram program;
    setupProgram(program);

    GLBuffer indexBuffer;
    GLBuffer vertexPositionBuffer;
    GLBuffer vertexColorBuffer;
    setupIndexedBuffers(vertexPositionBuffer, vertexColorBuffer, indexBuffer);

    // for potential update vertex color later
    glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer);

    doDrawElementsBaseVertexVariants(DrawCallVariants::DrawRangeElementsBaseVertex);
}

// Test drawElementsInstancedBaseVertexBaseInstance from ANGLE_base_vertex_base_instance
TEST_P(DrawBaseVertexVariantsTest, DrawElementsInstancedBaseVertexBaseInstance)
{
    ANGLE_SKIP_TEST_IF(!requestAngleBaseVertexBaseInstanceExtensions());

    GLProgram program;
    setupProgram(program);

    GLBuffer indexBuffer;
    GLBuffer vertexPositionBuffer;
    GLBuffer vertexColorBuffer;
    setupIndexedBuffers(vertexPositionBuffer, vertexColorBuffer, indexBuffer);

    // for potential update vertex color later
    glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffer);

    doDrawElementsBaseVertexVariants(DrawCallVariants::DrawElementsInstancedBaseVertexBaseInstance);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DrawBaseVertexVariantsTest);
ANGLE_INSTANTIATE_TEST_COMBINE_1(DrawBaseVertexVariantsTest,
                                 DrawBaseVertexVariantsTestPrint,
                                 testing::ValuesIn(kBufferDataUsage),
                                 ES3_D3D11(),
                                 ES3_METAL(),
                                 ES3_OPENGL(),
                                 ES3_OPENGLES(),
                                 ES3_VULKAN());

}  // namespace
