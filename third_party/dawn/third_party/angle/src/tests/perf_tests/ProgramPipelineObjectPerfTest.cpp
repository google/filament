//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineObjectPerfTest:
//   Performance tests switching program stages of a pipeline.
//

#include "ANGLEPerfTest.h"

#include <array>

#include "common/vector_utils.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{

constexpr unsigned int kIterationsPerStep = 1024;

struct ProgramPipelineObjectParams final : public RenderTestParams
{
    ProgramPipelineObjectParams()
    {
        iterationsPerStep = kIterationsPerStep;

        majorVersion = 3;
        minorVersion = 1;
        windowWidth  = 256;
        windowHeight = 256;
    }

    std::string story() const override
    {
        std::stringstream strstr;
        strstr << RenderTestParams::story();

        if (eglParameters.deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE)
        {
            strstr << "_null";
        }

        return strstr.str();
    }
};

std::ostream &operator<<(std::ostream &os, const ProgramPipelineObjectParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class ProgramPipelineObjectBenchmark
    : public ANGLERenderTest,
      public ::testing::WithParamInterface<ProgramPipelineObjectParams>
{
  public:
    ProgramPipelineObjectBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  protected:
    GLuint mVertProg1 = 0;
    GLuint mVertProg2 = 0;
    GLuint mFragProg1 = 0;
    GLuint mFragProg2 = 0;
    GLuint mPpo       = 0;
};

ProgramPipelineObjectBenchmark::ProgramPipelineObjectBenchmark()
    : ANGLERenderTest("ProgramPipelineObject", GetParam())
{}

void ProgramPipelineObjectBenchmark::initializeBenchmark()
{
    glGenProgramPipelines(1, &mPpo);
    glBindProgramPipeline(mPpo);

    // Create 2 separable vertex and fragment program objects from the same source.
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = R"(#version 310 es
precision highp float;
uniform float redColorIn;
uniform float greenColorIn;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(redColorIn, greenColorIn, 0.0, 1.0);
})";

    mVertProg1 = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertString);
    ASSERT_NE(mVertProg1, 0u);
    mVertProg2 = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertString);
    ASSERT_NE(mVertProg2, 0u);
    mFragProg1 = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragString);
    ASSERT_NE(mFragProg1, 0u);
    mFragProg2 = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragString);
    ASSERT_NE(mFragProg2, 0u);
}

void ProgramPipelineObjectBenchmark::destroyBenchmark()
{

    glDeleteProgramPipelines(1, &mPpo);
    glDeleteProgram(mVertProg1);
    glDeleteProgram(mVertProg2);
    glDeleteProgram(mFragProg1);
    glDeleteProgram(mFragProg2);
}

void ProgramPipelineObjectBenchmark::drawBenchmark()
{
    glUseProgramStages(mPpo, GL_VERTEX_SHADER_BIT, mVertProg1);
    glUseProgramStages(mPpo, GL_FRAGMENT_SHADER_BIT, mFragProg1);

    // Set the output color to red
    GLint location = glGetUniformLocation(mFragProg1, "redColorIn");
    glActiveShaderProgram(mPpo, mFragProg1);
    glUniform1f(location, 1.0);
    location = glGetUniformLocation(mFragProg1, "greenColorIn");
    glActiveShaderProgram(mPpo, mFragProg1);
    glUniform1f(location, 0.0);

    // Change vertex and fragment programs of the pipeline before the draw.
    glUseProgramStages(mPpo, GL_VERTEX_SHADER_BIT, mVertProg2);
    glUseProgramStages(mPpo, GL_FRAGMENT_SHADER_BIT, mFragProg2);

    // Set the output color to green
    location = glGetUniformLocation(mFragProg2, "redColorIn");
    glActiveShaderProgram(mPpo, mFragProg2);
    glUniform1f(location, 0.0);
    location = glGetUniformLocation(mFragProg2, "greenColorIn");
    glActiveShaderProgram(mPpo, mFragProg2);
    glUniform1f(location, 1.0);

    // Draw
    const std::array<Vector3, 6> kQuadVertices = {{
        Vector3(-1.0f, 1.0f, 0.5f),
        Vector3(-1.0f, -1.0f, 0.5f),
        Vector3(1.0f, -1.0f, 0.5f),
        Vector3(-1.0f, 1.0f, 0.5f),
    }};

    GLint positionLocation = glGetAttribLocation(mVertProg2, "a_position");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, kQuadVertices.data());
    glEnableVertexAttribArray(positionLocation);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
}

using namespace egl_platform;

ProgramPipelineObjectParams ProgramPipelineObjectVulkanParams()
{
    ProgramPipelineObjectParams params;
    params.eglParameters = VULKAN();
    return params;
}

ProgramPipelineObjectParams ProgramPipelineObjectVulkanNullParams()
{
    ProgramPipelineObjectParams params;
    params.eglParameters = VULKAN_NULL();
    return params;
}

// Test performance of switching programs before a draw.
TEST_P(ProgramPipelineObjectBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(ProgramPipelineObjectBenchmark,
                       ProgramPipelineObjectVulkanParams(),
                       ProgramPipelineObjectVulkanNullParams());

}  // anonymous namespace
