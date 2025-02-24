//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// LinkProgramPerfTest:
//   Performance tests compiling a lot of shaders.
//

#include "ANGLEPerfTest.h"

#include <array>

#include "common/vector_utils.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{

enum class TaskOption
{
    CompileOnly,
    CompileAndLink,

    Unspecified
};

enum class ThreadOption
{
    SingleThread,
    MultiThread,

    Unspecified
};

struct LinkProgramParams final : public RenderTestParams
{
    LinkProgramParams(TaskOption taskOptionIn, ThreadOption threadOptionIn)
    {
        iterationsPerStep = 1;

        majorVersion = 2;
        minorVersion = 0;
        windowWidth  = 256;
        windowHeight = 256;
        taskOption   = taskOptionIn;
        threadOption = threadOptionIn;
    }

    std::string story() const override
    {
        std::stringstream strstr;
        strstr << RenderTestParams::story();

        if (taskOption == TaskOption::CompileOnly)
        {
            strstr << "_compile_only";
        }
        else if (taskOption == TaskOption::CompileAndLink)
        {
            strstr << "_compile_and_link";
        }

        if (threadOption == ThreadOption::SingleThread)
        {
            strstr << "_single_thread";
        }
        else if (threadOption == ThreadOption::MultiThread)
        {
            strstr << "_multi_thread";
        }

        if (eglParameters.deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE)
        {
            strstr << "_null";
        }

        return strstr.str();
    }

    TaskOption taskOption;
    ThreadOption threadOption;
};

std::ostream &operator<<(std::ostream &os, const LinkProgramParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class LinkProgramBenchmark : public ANGLERenderTest,
                             public ::testing::WithParamInterface<LinkProgramParams>
{
  public:
    LinkProgramBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  protected:
    GLuint mVertexBuffer = 0;
};

LinkProgramBenchmark::LinkProgramBenchmark() : ANGLERenderTest("LinkProgram", GetParam()) {}

void LinkProgramBenchmark::initializeBenchmark()
{
    if (GetParam().threadOption != ThreadOption::SingleThread &&
        !IsGLExtensionEnabled("GL_KHR_parallel_shader_compile"))
    {
        skipTest("non-single-thread but missing GL_KHR_parallel_shader_compile");
        return;
    }

    if (IsGLExtensionEnabled("GL_KHR_parallel_shader_compile") &&
        GetParam().threadOption == ThreadOption::SingleThread)
    {
        glMaxShaderCompilerThreadsKHR(0);
    }

    std::array<Vector3, 6> vertices = {{Vector3(-1.0f, 1.0f, 0.5f), Vector3(-1.0f, -1.0f, 0.5f),
                                        Vector3(1.0f, -1.0f, 0.5f), Vector3(-1.0f, 1.0f, 0.5f),
                                        Vector3(1.0f, -1.0f, 0.5f), Vector3(1.0f, 1.0f, 0.5f)}};

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vector3), vertices.data(),
                 GL_STATIC_DRAW);
}

void LinkProgramBenchmark::destroyBenchmark()
{
    glDeleteBuffers(1, &mVertexBuffer);
}

void LinkProgramBenchmark::drawBenchmark()
{
    static const char *vertexShader =
        "attribute vec2 position;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 0, 1);\n"
        "}";
    static const char *fragmentShader =
        "precision mediump float;\n"
        "void main() {\n"
        "    gl_FragColor = vec4(1, 0, 0, 1);\n"
        "}";
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    ASSERT_NE(0u, vs);
    ASSERT_NE(0u, fs);
    if (GetParam().taskOption == TaskOption::CompileOnly)
    {
        glDeleteShader(vs);
        glDeleteShader(fs);
        return;
    }

    GLuint program = glCreateProgram();
    ASSERT_NE(0u, program);

    glAttachShader(program, vs);
    glDeleteShader(vs);
    glAttachShader(program, fs);
    glDeleteShader(fs);
    glLinkProgram(program);
    glUseProgram(program);

    GLint positionLoc = glGetAttribLocation(program, "position");
    glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 8, nullptr);
    glEnableVertexAttribArray(positionLoc);

    // Draw with the program to ensure the shader gets compiled and used.
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDeleteProgram(program);
}

using namespace egl_platform;

LinkProgramParams LinkProgramD3D11Params(TaskOption taskOption, ThreadOption threadOption)
{
    LinkProgramParams params(taskOption, threadOption);
    params.eglParameters = D3D11();
    return params;
}

LinkProgramParams LinkProgramMetalParams(TaskOption taskOption, ThreadOption threadOption)
{
    LinkProgramParams params(taskOption, threadOption);
    params.eglParameters = METAL();
    return params;
}

LinkProgramParams LinkProgramOpenGLOrGLESParams(TaskOption taskOption, ThreadOption threadOption)
{
    LinkProgramParams params(taskOption, threadOption);
    params.eglParameters = OPENGL_OR_GLES();
    return params;
}

LinkProgramParams LinkProgramVulkanParams(TaskOption taskOption, ThreadOption threadOption)
{
    LinkProgramParams params(taskOption, threadOption);
    params.eglParameters = VULKAN();
    return params;
}

TEST_P(LinkProgramBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(
    LinkProgramBenchmark,
    LinkProgramD3D11Params(TaskOption::CompileOnly, ThreadOption::MultiThread),
    LinkProgramMetalParams(TaskOption::CompileOnly, ThreadOption::MultiThread),
    LinkProgramOpenGLOrGLESParams(TaskOption::CompileOnly, ThreadOption::MultiThread),
    LinkProgramVulkanParams(TaskOption::CompileOnly, ThreadOption::MultiThread),
    LinkProgramD3D11Params(TaskOption::CompileAndLink, ThreadOption::MultiThread),
    LinkProgramMetalParams(TaskOption::CompileAndLink, ThreadOption::MultiThread),
    LinkProgramOpenGLOrGLESParams(TaskOption::CompileAndLink, ThreadOption::MultiThread),
    LinkProgramVulkanParams(TaskOption::CompileAndLink, ThreadOption::MultiThread),
    LinkProgramD3D11Params(TaskOption::CompileOnly, ThreadOption::SingleThread),
    LinkProgramMetalParams(TaskOption::CompileOnly, ThreadOption::SingleThread),
    LinkProgramOpenGLOrGLESParams(TaskOption::CompileOnly, ThreadOption::SingleThread),
    LinkProgramVulkanParams(TaskOption::CompileOnly, ThreadOption::SingleThread),
    LinkProgramD3D11Params(TaskOption::CompileAndLink, ThreadOption::SingleThread),
    LinkProgramMetalParams(TaskOption::CompileAndLink, ThreadOption::SingleThread),
    LinkProgramOpenGLOrGLESParams(TaskOption::CompileAndLink, ThreadOption::SingleThread),
    LinkProgramVulkanParams(TaskOption::CompileAndLink, ThreadOption::SingleThread));

}  // anonymous namespace
