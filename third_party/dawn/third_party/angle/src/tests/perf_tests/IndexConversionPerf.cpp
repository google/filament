//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IndexConversionPerf:
//   Performance tests for ANGLE index conversion in D3D11.
//

#include "ANGLEPerfTest.h"
#include "tests/test_utils/draw_call_perf_utils.h"

#include <sstream>

using namespace angle;

namespace
{
struct IndexConversionPerfParams final : public RenderTestParams
{
    std::string story() const override
    {
        std::stringstream strstr;

        if (indexRangeOffset > 0)
        {
            strstr << "_index_range";
        }

        strstr << RenderTestParams::story();

        return strstr.str();
    }

    unsigned int numIndexTris;

    // A second test, which covers using index ranges with an offset.
    unsigned int indexRangeOffset;
};

// Provide a custom gtest parameter name function for IndexConversionPerfParams.
std::ostream &operator<<(std::ostream &stream, const IndexConversionPerfParams &param)
{
    stream << param.backendAndStory().substr(1);
    return stream;
}

class IndexConversionPerfTest : public ANGLERenderTest,
                                public ::testing::WithParamInterface<IndexConversionPerfParams>
{
  public:
    IndexConversionPerfTest();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    void updateBufferData();
    void drawConversion();
    void drawIndexRange();

    GLuint mProgram;
    GLuint mVertexBuffer;
    GLuint mIndexBuffer;
    std::vector<GLushort> mIndexData;
};

IndexConversionPerfTest::IndexConversionPerfTest()
    : ANGLERenderTest("IndexConversionPerfTest", GetParam()),
      mProgram(0),
      mVertexBuffer(0),
      mIndexBuffer(0)
{}

void IndexConversionPerfTest::initializeBenchmark()
{
    const auto &params = GetParam();

    ASSERT_LT(0u, params.iterationsPerStep);
    ASSERT_LT(0u, params.numIndexTris);

    mProgram = SetupSimpleScaleAndOffsetProgram();
    ASSERT_NE(0u, mProgram);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Initialize the vertex data
    size_t numTris = std::numeric_limits<GLushort>::max() / 3 + 1;
    mVertexBuffer  = Create2DTriangleBuffer(numTris, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Initialize the index buffer
    for (unsigned int triIndex = 0; triIndex < params.numIndexTris; ++triIndex)
    {
        // Handle two different types of tests, one with index conversion triggered by a -1 index.
        if (params.indexRangeOffset == 0)
        {
            mIndexData.push_back(std::numeric_limits<GLushort>::max());
        }
        else
        {
            mIndexData.push_back(0);
        }

        mIndexData.push_back(1);
        mIndexData.push_back(2);
    }

    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    updateBufferData();

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    ASSERT_GL_NO_ERROR();
}

void IndexConversionPerfTest::updateBufferData()
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexData.size() * sizeof(mIndexData[0]), &mIndexData[0],
                 GL_STATIC_DRAW);
}

void IndexConversionPerfTest::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mVertexBuffer);
    glDeleteBuffers(1, &mIndexBuffer);
}

void IndexConversionPerfTest::drawBenchmark()
{
    const auto &params = GetParam();

    if (params.indexRangeOffset == 0)
    {
        drawConversion();
    }
    else
    {
        drawIndexRange();
    }
}

void IndexConversionPerfTest::drawConversion()
{
    const auto &params = GetParam();

    // Trigger an update to ensure we convert once a frame
    updateBufferData();

    for (unsigned int it = 0; it < params.iterationsPerStep; it++)
    {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(params.numIndexTris * 3 - 1),
                       GL_UNSIGNED_SHORT, reinterpret_cast<void *>(0));
    }

    ASSERT_GL_NO_ERROR();
}

void IndexConversionPerfTest::drawIndexRange()
{
    const auto &params = GetParam();

    unsigned int indexCount = 3;
    size_t offset           = static_cast<size_t>(indexCount * getNumStepsPerformed());

    offset %= (params.numIndexTris * 3);

    // This test increments an offset each step. Drawing repeatedly may cause the system memory
    // to release. Then, using a fresh offset will require index range validation, which pages
    // it back in. The performance should be good even if the data is was used quite a bit.
    for (unsigned int it = 0; it < params.iterationsPerStep; it++)
    {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_SHORT,
                       reinterpret_cast<void *>(offset * sizeof(GLushort)));
    }

    ASSERT_GL_NO_ERROR();
}

IndexConversionPerfParams IndexConversionPerfD3D11Params()
{
    IndexConversionPerfParams params;
    params.eglParameters     = egl_platform::D3D11_NULL();
    params.majorVersion      = 2;
    params.minorVersion      = 0;
    params.windowWidth       = 256;
    params.windowHeight      = 256;
    params.iterationsPerStep = 225;
    params.numIndexTris      = 3000;
    params.indexRangeOffset  = 0;
    return params;
}

IndexConversionPerfParams IndexRangeOffsetPerfD3D11Params()
{
    IndexConversionPerfParams params;
    params.eglParameters     = egl_platform::D3D11_NULL();
    params.majorVersion      = 2;
    params.minorVersion      = 0;
    params.windowWidth       = 256;
    params.windowHeight      = 256;
    params.iterationsPerStep = 16;
    params.numIndexTris      = 50000;
    params.indexRangeOffset  = 64;
    return params;
}

TEST_P(IndexConversionPerfTest, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(IndexConversionPerfTest,
                       IndexConversionPerfD3D11Params(),
                       IndexRangeOffsetPerfD3D11Params());

// This test suite is not instantiated on some OSes.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(IndexConversionPerfTest);
}  // namespace
