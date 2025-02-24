//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DynamicPromotionPerfTest:
//   Tests that ANGLE will promote buffer specfied with DYNAMIC usage to static after a number of
//   iterations without changing the data. It specifically affects the D3D back-end, which treats
//   dynamic and static buffers quite differently.
//

#include "ANGLEPerfTest.h"
#include "common/vector_utils.h"
#include "util/random_utils.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{
constexpr unsigned int kIterationsPerStep = 4;

struct DynamicPromotionParams final : public RenderTestParams
{
    DynamicPromotionParams() { iterationsPerStep = kIterationsPerStep; }

    std::string story() const override;

    size_t vertexCount = 1024;
};

std::string DynamicPromotionParams::story() const
{
    return RenderTestParams::story();
}

std::ostream &operator<<(std::ostream &os, const DynamicPromotionParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class DynamicPromotionPerfTest : public ANGLERenderTest,
                                 public testing::WithParamInterface<DynamicPromotionParams>
{
  public:
    DynamicPromotionPerfTest();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mProgram;
    GLuint mElementArrayBuffer;
    GLuint mArrayBuffer;
};

DynamicPromotionPerfTest::DynamicPromotionPerfTest()
    : ANGLERenderTest("DynamicPromotion", GetParam()),
      mProgram(0),
      mElementArrayBuffer(0),
      mArrayBuffer(0)
{}

void DynamicPromotionPerfTest::initializeBenchmark()
{
    constexpr char kVertexShaderSource[] =
        "attribute vec2 position;\n"
        "attribute vec3 color;\n"
        "varying vec3 vColor;\n"
        "void main()\n"
        "{\n"
        "    vColor = color;\n"
        "    gl_Position = vec4(position, 0, 1);\n"
        "}";

    constexpr char kFragmentShaderSource[] =
        "varying mediump vec3 vColor;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(vColor, 1);\n"
        "}";

    mProgram = CompileProgram(kVertexShaderSource, kFragmentShaderSource);
    ASSERT_NE(0u, mProgram);

    const size_t vertexCount = GetParam().vertexCount;

    std::vector<GLushort> indexData;
    std::vector<Vector2> positionData;
    std::vector<Vector3> colorData;

    ASSERT_GE(static_cast<size_t>(std::numeric_limits<GLushort>::max()), vertexCount);

    RNG rng(1);

    for (size_t index = 0; index < vertexCount; ++index)
    {
        indexData.push_back(static_cast<GLushort>(index));

        Vector2 position(rng.randomNegativeOneToOne(), rng.randomNegativeOneToOne());
        positionData.push_back(position);

        Vector3 color(rng.randomFloat(), rng.randomFloat(), rng.randomFloat());
        colorData.push_back(color);
    }

    glGenBuffers(1, &mElementArrayBuffer);
    glGenBuffers(1, &mArrayBuffer);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementArrayBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mArrayBuffer);

    GLsizeiptr elementArraySize  = sizeof(GLushort) * vertexCount;
    GLsizeiptr positionArraySize = sizeof(Vector2) * vertexCount;
    GLsizeiptr colorArraySize    = sizeof(Vector3) * vertexCount;

    // The DYNAMIC_DRAW usage is the key to the test.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementArraySize, indexData.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, positionArraySize + colorArraySize, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positionArraySize, positionData.data());
    glBufferSubData(GL_ARRAY_BUFFER, positionArraySize, colorArraySize, colorData.data());

    glUseProgram(mProgram);
    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    GLint colorLocation = glGetAttribLocation(mProgram, "color");
    ASSERT_NE(-1, colorLocation);

    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 0,
                          reinterpret_cast<const void *>(positionArraySize));

    glEnableVertexAttribArray(positionLocation);
    glEnableVertexAttribArray(colorLocation);

    ASSERT_GL_NO_ERROR();
}

void DynamicPromotionPerfTest::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mElementArrayBuffer);
    glDeleteBuffers(1, &mArrayBuffer);
}

void DynamicPromotionPerfTest::drawBenchmark()
{
    unsigned int iterations = GetParam().iterationsPerStep;
    size_t vertexCount      = GetParam().vertexCount;

    glClear(GL_COLOR_BUFFER_BIT);
    for (unsigned int count = 0; count < iterations; ++count)
    {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(vertexCount), GL_UNSIGNED_SHORT, nullptr);
    }

    ASSERT_GL_NO_ERROR();
}

DynamicPromotionParams DynamicPromotionD3D11Params()
{
    DynamicPromotionParams params;
    params.eglParameters = egl_platform::D3D11();
    return params;
}

TEST_P(DynamicPromotionPerfTest, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(DynamicPromotionPerfTest, DynamicPromotionD3D11Params());

// This test suite is not instantiated on some OSes.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DynamicPromotionPerfTest);

}  // anonymous namespace
