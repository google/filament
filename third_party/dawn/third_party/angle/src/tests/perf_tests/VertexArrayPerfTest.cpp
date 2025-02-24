//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VertexArrayPerfTest:
//   Performance test for glBindVertexArray.
//

#include "ANGLEPerfTest.h"
#include "DrawCallPerfParams.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{
enum class TestMode
{
    BufferData,
    BindBuffer,
    UpdateBufferData,
};

struct VertexArrayParams final : public RenderTestParams
{
    VertexArrayParams()
    {
        iterationsPerStep = 1;

        // Common default params
        majorVersion = 3;
        minorVersion = 0;
        windowWidth  = 720;
        windowHeight = 720;
    }

    std::string story() const override;

    int numVertexArrays  = 2000;
    int numBuffers       = 5;
    GLuint bufferSize[5] = {384, 1028, 192, 384, 192};
    TestMode testMode    = TestMode::BufferData;
};

std::ostream &operator<<(std::ostream &os, const VertexArrayParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

std::string VertexArrayParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    if (testMode == TestMode::BindBuffer)
    {
        strstr << "_bindbuffer";
    }
    else if (testMode == TestMode::UpdateBufferData)
    {
        strstr << "_updatebufferdata";
    }

    return strstr.str();
}

class VertexArrayBenchmark : public ANGLERenderTest,
                             public ::testing::WithParamInterface<VertexArrayParams>
{
  public:
    VertexArrayBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

    void rebindVertexArray(GLuint vertexArrayID, GLuint bufferID);
    void updateBufferData(GLuint vertexArrayID, GLuint bufferID, GLuint bufferSize);

  private:
    std::vector<GLuint> mBuffers;
    GLuint mProgram       = 0;
    GLint mAttribLocation = 0;
    std::vector<GLuint> mVertexArrays;
};

VertexArrayBenchmark::VertexArrayBenchmark() : ANGLERenderTest("VertexArrayPerf", GetParam()) {}

void VertexArrayBenchmark::initializeBenchmark()
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

    mProgram = CompileProgram(kVS, kFS);
    ASSERT_NE(0u, mProgram);

    mAttribLocation = glGetAttribLocation(mProgram, "in_attrib");
    ASSERT_NE(mAttribLocation, -1);

    // Generate 5 buffers.
    int numBuffers = GetParam().numBuffers;
    mBuffers.resize(numBuffers, 0);
    glGenBuffers(numBuffers, mBuffers.data());

    int numVertexArrays = GetParam().numVertexArrays;
    mVertexArrays.resize(numVertexArrays, 0);
    glGenVertexArrays(numVertexArrays, mVertexArrays.data());

    // Bind one VBO to all VAOs.
    for (GLuint vertexArray : mVertexArrays)
    {
        rebindVertexArray(vertexArray, mBuffers[0]);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[0]);
}

void VertexArrayBenchmark::rebindVertexArray(GLuint vertexArrayID, GLuint bufferID)
{
    // Rebind a vertex array object and a generic vertex attribute inside of it.
    glBindVertexArray(vertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glEnableVertexAttribArray(mAttribLocation);
    glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 4, nullptr);
    glVertexAttribDivisor(mAttribLocation, 1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexArrayBenchmark::updateBufferData(GLuint vertexArrayID,
                                            GLuint bufferID,
                                            GLuint bufferSize)
{
    glBindVertexArray(vertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glEnableVertexAttribArray(mAttribLocation);
    glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 4, nullptr);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexArrayBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteVertexArrays(static_cast<GLsizei>(mVertexArrays.size()), mVertexArrays.data());
    mVertexArrays.clear();
    glDeleteBuffers(static_cast<GLsizei>(mBuffers.size()), mBuffers.data());
    mBuffers.clear();
}

void VertexArrayBenchmark::drawBenchmark()
{
    const VertexArrayParams &params = GetParam();
    if (params.testMode == TestMode::BufferData)
    {
        glBufferData(GL_ARRAY_BUFFER, 128, nullptr, GL_STATIC_DRAW);
    }
    else if (params.testMode == TestMode::UpdateBufferData)
    {
        int bufferSizeIndex = 0;
        for (GLuint vertexArray : mVertexArrays)
        {
            bufferSizeIndex = ((bufferSizeIndex + 1) == 5) ? 0 : (bufferSizeIndex + 1);
            updateBufferData(vertexArray, mBuffers[0], params.bufferSize[bufferSizeIndex]);
        }
    }
    else
    {
        int bufferIndex = 0;
        for (GLuint vertexArray : mVertexArrays)
        {
            bufferIndex = ((bufferIndex + 1) == params.numBuffers) ? 0 : (bufferIndex + 1);
            rebindVertexArray(vertexArray, mBuffers[bufferIndex]);
        }
    }
}

TEST_P(VertexArrayBenchmark, Run)
{
    run();
}

VertexArrayParams MetalParams()
{
    VertexArrayParams params;
    params.eglParameters = egl_platform::METAL();
    return params;
}

VertexArrayParams VulkanParams()
{
    VertexArrayParams params;
    params.eglParameters = egl_platform::VULKAN();
    return params;
}

VertexArrayParams VulkanNullParams(TestMode testMode)
{
    VertexArrayParams params;
    params.eglParameters = egl_platform::VULKAN_NULL();
    params.testMode      = testMode;
    return params;
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(VertexArrayBenchmark);
ANGLE_INSTANTIATE_TEST(VertexArrayBenchmark,
                       MetalParams(),
                       VulkanParams(),
                       VulkanNullParams(TestMode::BindBuffer),
                       VulkanNullParams(TestMode::BufferData),
                       VulkanNullParams(TestMode::UpdateBufferData),
                       params::Native(VertexArrayParams()));
}  // namespace
