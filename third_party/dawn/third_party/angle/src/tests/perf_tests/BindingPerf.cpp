//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BindingPerf:
//   Performance test for binding objects
//

#include "ANGLEPerfTest.h"

#include <iostream>
#include <random>
#include <sstream>

#include "test_utils/angle_test_instantiate.h"
#include "util/shader_utils.h"

namespace angle
{
namespace
{
constexpr unsigned int kIterationsPerStep = 128;

enum TestMode
{
    VertexArray,
    MultipleBindings,
};

enum AllocationStyle
{
    EVERY_ITERATION,
    AT_INITIALIZATION
};

struct BindingsParams final : public RenderTestParams
{
    BindingsParams()
    {
        // Common default params
        majorVersion = 2;
        minorVersion = 0;
        windowWidth  = 720;
        windowHeight = 720;

        numObjects        = 100;
        allocationStyle   = EVERY_ITERATION;
        iterationsPerStep = kIterationsPerStep;
    }

    std::string story() const override;
    TestMode testMode = TestMode::MultipleBindings;
    size_t numObjects;
    AllocationStyle allocationStyle;
};

std::ostream &operator<<(std::ostream &os, const BindingsParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

std::string BindingsParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    if (testMode == TestMode::VertexArray)
    {
        strstr << "_vertexarray";
    }
    else
    {
        strstr << "_" << numObjects << "_objects";

        switch (allocationStyle)
        {
            case EVERY_ITERATION:
                strstr << "_allocated_every_iteration";
                break;
            case AT_INITIALIZATION:
                strstr << "_allocated_at_initialization";
                break;
            default:
                strstr << "_err";
                break;
        }
    }

    return strstr.str();
}

class BindingsBenchmark : public ANGLERenderTest,
                          public ::testing::WithParamInterface<BindingsParams>
{
  public:
    BindingsBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    // TODO: Test binding perf of more than just buffers
    std::vector<GLuint> mBuffers;
    std::vector<GLenum> mBindingPoints;
    GLuint mMaxVertexAttribs = 0;
};

BindingsBenchmark::BindingsBenchmark() : ANGLERenderTest("Bindings", GetParam())
{
    if (GetParam().eglParameters.renderer == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
    {
        skipTest("http://anglebug.com/42264783 Flaky on OpenGL");
    }
}

void BindingsBenchmark::initializeBenchmark()
{
    const BindingsParams &params = GetParam();

    mBuffers.resize(params.numObjects, 0);
    if (params.allocationStyle == AT_INITIALIZATION)
    {
        glGenBuffers(static_cast<GLsizei>(mBuffers.size()), mBuffers.data());
        for (size_t bufferIdx = 0; bufferIdx < mBuffers.size(); bufferIdx++)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mBuffers[bufferIdx]);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (params.testMode == TestMode::MultipleBindings)
    {
        mBindingPoints.push_back(GL_ARRAY_BUFFER);
        mBindingPoints.push_back(GL_ELEMENT_ARRAY_BUFFER);
        if (params.majorVersion >= 3)
        {
            mBindingPoints.push_back(GL_PIXEL_PACK_BUFFER);
            mBindingPoints.push_back(GL_PIXEL_UNPACK_BUFFER);
            mBindingPoints.push_back(GL_COPY_READ_BUFFER);
            mBindingPoints.push_back(GL_COPY_WRITE_BUFFER);
            mBindingPoints.push_back(GL_TRANSFORM_FEEDBACK_BUFFER);
            mBindingPoints.push_back(GL_UNIFORM_BUFFER);
        }
        if (params.majorVersion > 3 || (params.majorVersion == 3 && params.minorVersion >= 1))
        {
            mBindingPoints.push_back(GL_ATOMIC_COUNTER_BUFFER);
            mBindingPoints.push_back(GL_SHADER_STORAGE_BUFFER);
            mBindingPoints.push_back(GL_DRAW_INDIRECT_BUFFER);
            mBindingPoints.push_back(GL_DISPATCH_INDIRECT_BUFFER);
        }
    }
    else
    {
        mBindingPoints.resize(mBuffers.size(), GL_ARRAY_BUFFER);
    }

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, reinterpret_cast<GLint *>(&mMaxVertexAttribs));
}

void BindingsBenchmark::destroyBenchmark()
{
    const BindingsParams &params = GetParam();
    if (params.allocationStyle == AT_INITIALIZATION)
    {
        glDeleteBuffers(static_cast<GLsizei>(mBuffers.size()), mBuffers.data());
    }
}

void BindingsBenchmark::drawBenchmark()
{
    const BindingsParams &params = GetParam();

    for (unsigned int it = 0; it < params.iterationsPerStep; ++it)
    {
        // Generate a buffer (if needed) and bind it to a "random" binding point
        if (params.allocationStyle == EVERY_ITERATION)
        {
            glGenBuffers(static_cast<GLsizei>(mBuffers.size()), mBuffers.data());
        }

        // Fetch a few variables from the underlying data structure to keep them in registers.
        // Otherwise each loop iteration they'll be fetched again because the compiler cannot
        // guarantee that those are unchanged when calling glBindBuffer.
        const GLuint *buffers       = mBuffers.data();
        const GLenum *bindingPoints = mBindingPoints.data();
        size_t bindingPointsSize    = mBindingPoints.size();
        size_t buffersSize          = mBuffers.size();
        size_t bindingIndex         = it % bindingPointsSize;
        for (GLuint bufferIdx = 0; bufferIdx < buffersSize; bufferIdx++)
        {
            GLenum binding = bindingPoints[bindingIndex];
            glBindBuffer(binding, buffers[bufferIdx]);

            // Instead of doing a costly division to get an index in the range [0,bindingPointsSize)
            // do a bounds-check and reset the index.
            ++bindingIndex;
            bindingIndex = (bindingIndex >= bindingPointsSize) ? 0 : bindingIndex;

            if (params.testMode == TestMode::VertexArray)
            {
                GLuint vertexAttribIndex = bufferIdx % mMaxVertexAttribs;
                glVertexAttribPointer(vertexAttribIndex, 1, GL_FLOAT, GL_FALSE, 0, 0);
            }
        }

        // Delete all the buffers
        if (params.allocationStyle == EVERY_ITERATION)
        {
            glDeleteBuffers(static_cast<GLsizei>(mBuffers.size()), mBuffers.data());
        }
    }

    ASSERT_GL_NO_ERROR();
}

BindingsParams D3D11Params(AllocationStyle allocationStyle)
{
    BindingsParams params;
    params.eglParameters   = egl_platform::D3D11_NULL();
    params.allocationStyle = allocationStyle;
    return params;
}

BindingsParams OpenGLOrGLESParams(AllocationStyle allocationStyle)
{
    BindingsParams params;
    params.eglParameters   = egl_platform::OPENGL_OR_GLES_NULL();
    params.allocationStyle = allocationStyle;
    return params;
}

BindingsParams VulkanParams(AllocationStyle allocationStyle, TestMode testMode)
{
    BindingsParams params;
    params.eglParameters   = egl_platform::VULKAN_NULL();
    params.allocationStyle = allocationStyle;
    params.testMode        = testMode;
    return params;
}

TEST_P(BindingsBenchmark, Run)
{
    run();
}
}  // namespace

ANGLE_INSTANTIATE_TEST(BindingsBenchmark,
                       D3D11Params(EVERY_ITERATION),
                       D3D11Params(AT_INITIALIZATION),
                       OpenGLOrGLESParams(EVERY_ITERATION),
                       OpenGLOrGLESParams(AT_INITIALIZATION),
                       VulkanParams(EVERY_ITERATION, TestMode::MultipleBindings),
                       VulkanParams(AT_INITIALIZATION, TestMode::MultipleBindings),
                       VulkanParams(AT_INITIALIZATION, TestMode::VertexArray));

}  // namespace angle
