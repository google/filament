//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PreRotationBenchmark:
//   Performance test for pre-rotation code generation.
//

#include "ANGLEPerfTest.h"

#include <iostream>
#include <random>
#include <sstream>

#include "test_utils/gl_raii.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{
constexpr unsigned int kIterationsPerStep = 20;

enum class PreRotation
{
    _0,
    _90,
    _180,
    _270,
};

struct PreRotationParams final : public RenderTestParams
{
    PreRotationParams()
    {
        iterationsPerStep = kIterationsPerStep;
        trackGpuTime      = true;

        preRotation = PreRotation::_0;
    }

    std::string story() const override;

    PreRotation preRotation;
};

std::ostream &operator<<(std::ostream &os, const PreRotationParams &params)
{
    return os << params.backendAndStory().substr(1);
}

std::string PreRotationParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    switch (preRotation)
    {
        case PreRotation::_0:
            strstr << "_NoPreRotation";
            break;
        case PreRotation::_90:
            strstr << "_PreRotate90";
            break;
        case PreRotation::_180:
            strstr << "_PreRotate180";
            break;
        case PreRotation::_270:
            strstr << "_PreRotate270";
            break;
    }

    return strstr.str();
}

class PreRotationBenchmark : public ANGLERenderTest,
                             public ::testing::WithParamInterface<PreRotationParams>
{
  public:
    PreRotationBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;

    void drawBenchmark() override;

  protected:
    GLuint mProgram = 0;
};

PreRotationBenchmark::PreRotationBenchmark() : ANGLERenderTest("PreRotation", GetParam()) {}

void PreRotationBenchmark::initializeBenchmark()
{
    constexpr char kVS[] = R"(
attribute mediump vec4 positionIn;
void main()
{
    gl_Position = positionIn;
})";

    constexpr char kFS[] = R"(precision mediump float;
void main()
{
    gl_FragColor = vec4(0);
})";

    mProgram = CompileProgram(kVS, kFS);
    ASSERT_NE(0u, mProgram);

    glUseProgram(mProgram);

    ASSERT_GL_NO_ERROR();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    // Perform a draw so everything is flushed.
    glDrawArrays(GL_TRIANGLES, 0, 3);

    ASSERT_GL_NO_ERROR();
}

void PreRotationBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
}

void PreRotationBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    constexpr uint32_t kDrawCallSize = 100'000;

    GLint attribLocation = glGetAttribLocation(mProgram, "positionIn");
    ASSERT_NE(-1, attribLocation);

    startGpuTimer();
    for (unsigned int iteration = 0; iteration < params.iterationsPerStep; ++iteration)
    {
        // Set the position attribute such that every generated primitive is out of bounds and is
        // clipped.  This means the test is spending its time almost entirely with vertex shaders.
        // The vertex shader itself is simple so that any code that is added for pre-rotation will
        // contribute a comparably sizable chunk of code.
        switch (iteration % 5)
        {
            case 0:
                glVertexAttrib4f(attribLocation, -2.0f, 0.0f, 0.0f, 1.0f);
                break;
            case 1:
                glVertexAttrib4f(attribLocation, 2.0f, 0.0f, 0.0f, 1.0f);
                break;
            case 2:
                glVertexAttrib4f(attribLocation, 0.0f, -2.0f, 0.0f, 1.0f);
                break;
            case 3:
                glVertexAttrib4f(attribLocation, 0.0f, 2.0f, 0.0f, 1.0f);
                break;
            case 4:
                glVertexAttrib4f(attribLocation, 0.0f, 0.0f, -2.0f, 1.0f);
                break;
        }

        // Draw many points, all which are culled.
        glDrawArrays(GL_POINTS, 0, kDrawCallSize);
    }
    stopGpuTimer();

    ASSERT_GL_NO_ERROR();
}

PreRotationParams VulkanParams(PreRotation preRotation)
{
    PreRotationParams params;
    params.eglParameters = egl_platform::VULKAN();
    params.preRotation   = preRotation;

    switch (preRotation)
    {
        case PreRotation::_0:
            break;
        case PreRotation::_90:
            params.eglParameters.enable(Feature::EmulatedPrerotation90);
            break;
        case PreRotation::_180:
            params.eglParameters.enable(Feature::EmulatedPrerotation180);
            break;
        case PreRotation::_270:
            params.eglParameters.enable(Feature::EmulatedPrerotation270);
            break;
    }

    return params;
}

}  // anonymous namespace

TEST_P(PreRotationBenchmark, Run)
{
    run();
}

using namespace params;

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(PreRotationBenchmark);
ANGLE_INSTANTIATE_TEST(PreRotationBenchmark,
                       VulkanParams(PreRotation::_0),
                       VulkanParams(PreRotation::_90),
                       VulkanParams(PreRotation::_180),
                       VulkanParams(PreRotation::_270));
