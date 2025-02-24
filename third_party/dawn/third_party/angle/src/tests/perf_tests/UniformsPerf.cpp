//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UniformsBenchmark:
//   Performance test for setting uniform data.
//

#include "ANGLEPerfTest.h"

#include <array>
#include <iostream>
#include <random>
#include <sstream>

#include "common/debug.h"
#include "util/Matrix.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{
constexpr unsigned int kIterationsPerStep = 4;

// Controls when we call glUniform, if the data is the same as last frame.
enum DataMode
{
    UPDATE,
    REPEAT,
};

// TODO(jmadill): Use an ANGLE enum for this?
enum DataType
{
    VEC4,
    MAT3x3,
    MAT3x4,
    MAT4x4,
};

// Determines if we state change the program between draws.
// This covers a performance problem in ANGLE where calling UseProgram reuploads uniform data.
enum ProgramMode
{
    SINGLE,
    MULTIPLE,
};

enum MatrixLayout
{
    TRANSPOSE,
    NO_TRANSPOSE,
};

struct UniformsParams final : public RenderTestParams
{
    UniformsParams()
    {
        iterationsPerStep = kIterationsPerStep;

        // Common default params
        majorVersion = 3;
        minorVersion = 0;
        windowWidth  = 720;
        windowHeight = 720;
    }

    std::string story() const override;
    size_t numVertexUniforms   = 200;
    size_t numFragmentUniforms = 200;

    DataType dataType         = DataType::VEC4;
    DataMode dataMode         = DataMode::REPEAT;
    MatrixLayout matrixLayout = MatrixLayout::NO_TRANSPOSE;
    ProgramMode programMode   = ProgramMode::SINGLE;
};

std::ostream &operator<<(std::ostream &os, const UniformsParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

std::string UniformsParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    if (dataType == DataType::VEC4)
    {
        strstr << "_" << (numVertexUniforms + numFragmentUniforms) << "_vec4";
    }
    else if (dataType == DataType::MAT3x3)
    {
        strstr << "_" << (numVertexUniforms + numFragmentUniforms) << "_mat3x3";
    }
    else if (dataType == DataType::MAT3x4)
    {
        strstr << "_" << (numVertexUniforms + numFragmentUniforms) << "_mat3x4";
    }
    else
    {
        strstr << "_" << (numVertexUniforms + numFragmentUniforms) << "_mat4x4";
    }

    if (matrixLayout == MatrixLayout::TRANSPOSE)
    {
        strstr << "_transpose";
    }

    if (programMode == ProgramMode::MULTIPLE)
    {
        strstr << "_multiprogram";
    }

    if (dataMode == DataMode::REPEAT)
    {
        strstr << "_repeating";
    }

    return strstr.str();
}

class UniformsBenchmark : public ANGLERenderTest,
                          public ::testing::WithParamInterface<UniformsParams>
{
  public:
    UniformsBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    void initShaders();

    template <bool MultiProgram, typename SetUniformFunc>
    void drawLoop(const SetUniformFunc &setUniformsFunc);

    std::array<GLuint, 2> mPrograms;
    std::vector<GLuint> mUniformLocations;

    using MatrixData = std::array<std::vector<Matrix4>, 2>;
    MatrixData mMatrixData;
};

std::vector<Matrix4> GenMatrixData(size_t count, int parity)
{
    std::vector<Matrix4> data;

    // Very simple matrix data allocation scheme.
    for (size_t index = 0; index < count; ++index)
    {
        Matrix4 mat;
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                mat.data[row * 4 + col] = (row * col + parity) % 2 == 0 ? 1.0f : -1.0f;
            }
        }

        data.push_back(mat);
    }

    return data;
}

UniformsBenchmark::UniformsBenchmark() : ANGLERenderTest("Uniforms", GetParam()), mPrograms({})
{
    if (IsWindows7() && IsNVIDIA() &&
        GetParam().eglParameters.renderer == EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE)
    {
        skipTest(
            "http://crbug.com/1096510 Fails on Windows7 NVIDIA Vulkan, presumably due to old "
            "drivers");
    }
}

void UniformsBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    // Verify the uniform counts are within the limits
    GLint maxVertexUniformVectors, maxFragmentUniformVectors;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxVertexUniformVectors);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &maxFragmentUniformVectors);

    GLint vectorCountPerUniform;
    bool isMatrix;
    switch (params.dataType)
    {
        case DataType::MAT3x3:
            vectorCountPerUniform = 3;
            isMatrix              = true;
            break;
        case DataType::MAT3x4:
            // depends on transpose, conservatively set to 4
            vectorCountPerUniform = 4;
            isMatrix              = true;
            break;
        case DataType::MAT4x4:
            vectorCountPerUniform = 4;
            isMatrix              = true;
            break;
        default:
            vectorCountPerUniform = 1;
            isMatrix              = false;
            break;
    }

    GLint numVertexUniformVectors =
        static_cast<GLint>(params.numVertexUniforms) * vectorCountPerUniform;
    GLint numFragmentUniformVectors =
        static_cast<GLint>(params.numFragmentUniforms) * vectorCountPerUniform;

    if (numVertexUniformVectors > maxVertexUniformVectors)
    {
        FAIL() << "Vertex uniform vector count (" << numVertexUniformVectors << ")"
               << " exceeds maximum vertex uniform vector count: " << maxVertexUniformVectors
               << std::endl;
    }
    if (numFragmentUniformVectors > maxFragmentUniformVectors)
    {
        FAIL() << "Fragment uniform vector count (" << numFragmentUniformVectors << ")"
               << " exceeds maximum fragment uniform vector count: " << maxFragmentUniformVectors
               << std::endl;
    }

    initShaders();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    if (isMatrix)
    {
        size_t count = params.numVertexUniforms + params.numFragmentUniforms;

        mMatrixData[0] = GenMatrixData(count, 0);
        if (params.dataMode == DataMode::REPEAT)
        {
            mMatrixData[1] = GenMatrixData(count, 0);
        }
        else
        {
            mMatrixData[1] = GenMatrixData(count, 1);
        }
    }

    GLint attribLocation = glGetAttribLocation(mPrograms[0], "pos");
    ASSERT_NE(-1, attribLocation);
    ASSERT_EQ(attribLocation, glGetAttribLocation(mPrograms[1], "pos"));
    glVertexAttrib4f(attribLocation, 1.0f, 0.0f, 0.0f, 1.0f);

    ASSERT_GL_NO_ERROR();
}

std::string GetUniformLocationName(size_t idx, bool vertexShader)
{
    std::stringstream strstr;
    strstr << (vertexShader ? "vs" : "fs") << "_u_" << idx;
    return strstr.str();
}

void UniformsBenchmark::initShaders()
{
    const auto &params = GetParam();

    const std::string kUniformVarPlaceHolder = "%s";
    std::string typeString;
    std::string uniformOperationTemplate;
    switch (params.dataType)
    {
        case DataType::VEC4:
            typeString               = "vec4";
            uniformOperationTemplate = kUniformVarPlaceHolder;
            break;
        case DataType::MAT3x3:
            typeString = "mat3";
            uniformOperationTemplate =
                "mat4(" + kUniformVarPlaceHolder + ") * vec4(1.0, 1.0, 1.0, 1.0)";
            break;
        case DataType::MAT3x4:
            typeString = "mat3x4";
            uniformOperationTemplate =
                "mat4(" + kUniformVarPlaceHolder + ") * vec4(1.0, 1.0, 1.0, 1.0)";
            break;
        case DataType::MAT4x4:
            typeString               = "mat4";
            uniformOperationTemplate = kUniformVarPlaceHolder + "* vec4(1.0, 1.0, 1.0, 1.0)";
            break;
        default:
            UNREACHABLE();
    }

    std::stringstream vstrstr;
    vstrstr << "#version 300 es\n";
    vstrstr << "precision mediump float;\n";
    vstrstr << "in vec4 pos;\n";

    for (size_t i = 0; i < params.numVertexUniforms; i++)
    {
        vstrstr << "uniform " << typeString << " " << GetUniformLocationName(i, true) << ";\n";
    }

    vstrstr << "void main()\n"
               "{\n"
               "    gl_Position = pos;\n";
    for (size_t i = 0; i < params.numVertexUniforms; i++)
    {
        std::string uniformOperation = uniformOperationTemplate;
        std::size_t pos              = uniformOperation.find(kUniformVarPlaceHolder);
        ASSERT(pos != std::string::npos);
        uniformOperation.replace(pos, kUniformVarPlaceHolder.size(),
                                 GetUniformLocationName(i, true));
        vstrstr << "    gl_Position += ";
        vstrstr << uniformOperation;
        vstrstr << ";\n";
    }
    vstrstr << "}";

    std::stringstream fstrstr;
    fstrstr << "#version 300 es\n";
    fstrstr << "precision mediump float;\n";
    fstrstr << "out vec4 fragColor;\n";

    for (size_t i = 0; i < params.numFragmentUniforms; i++)
    {
        fstrstr << "uniform " << typeString << " " << GetUniformLocationName(i, false) << ";\n";
    }
    fstrstr << "void main()\n"
               "{\n"
               "    fragColor = vec4(0, 0, 0, 0);\n";
    for (size_t i = 0; i < params.numFragmentUniforms; i++)
    {
        std::string uniformOperation = uniformOperationTemplate;
        std::size_t pos              = uniformOperation.find(kUniformVarPlaceHolder);
        ASSERT(pos != std::string::npos);
        uniformOperation.replace(pos, kUniformVarPlaceHolder.size(),
                                 GetUniformLocationName(i, false));
        fstrstr << "    fragColor += ";
        fstrstr << uniformOperation;
        fstrstr << ";\n";
    }
    fstrstr << "}";

    mPrograms[0] = CompileProgram(vstrstr.str().c_str(), fstrstr.str().c_str());
    ASSERT_NE(0u, mPrograms[0]);
    mPrograms[1] = CompileProgram(vstrstr.str().c_str(), fstrstr.str().c_str());
    ASSERT_NE(0u, mPrograms[1]);

    for (size_t i = 0; i < params.numVertexUniforms; ++i)
    {
        std::string name = GetUniformLocationName(i, true);
        GLint location   = glGetUniformLocation(mPrograms[0], name.c_str());
        ASSERT_NE(-1, location);
        ASSERT_EQ(location, glGetUniformLocation(mPrograms[1], name.c_str()));
        mUniformLocations.push_back(location);
    }
    for (size_t i = 0; i < params.numFragmentUniforms; ++i)
    {
        std::string name = GetUniformLocationName(i, false);
        GLint location   = glGetUniformLocation(mPrograms[0], name.c_str());
        ASSERT_NE(-1, location);
        ASSERT_EQ(location, glGetUniformLocation(mPrograms[1], name.c_str()));
        mUniformLocations.push_back(location);
    }

    // Use the program object
    glUseProgram(mPrograms[0]);
}

void UniformsBenchmark::destroyBenchmark()
{
    glDeleteProgram(mPrograms[0]);
    glDeleteProgram(mPrograms[1]);
}

// Hopefully the compiler is smart enough to inline the lambda setUniformsFunc.
template <bool MultiProgram, typename SetUniformFunc>
void UniformsBenchmark::drawLoop(const SetUniformFunc &setUniformsFunc)
{
    const auto &params = GetParam();

    size_t frameIndex = 0;

    for (size_t it = 0; it < params.iterationsPerStep; ++it, frameIndex = (frameIndex == 0 ? 1 : 0))
    {
        if (MultiProgram)
        {
            glUseProgram(mPrograms[frameIndex]);
        }
        if (params.dataMode == DataMode::UPDATE)
        {
            for (size_t uniform = 0; uniform < mUniformLocations.size(); ++uniform)
            {
                setUniformsFunc(mUniformLocations, mMatrixData, uniform, frameIndex);
            }
        }
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}

void UniformsBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    GLboolean transpose = static_cast<GLboolean>(params.matrixLayout == MatrixLayout::TRANSPOSE);

    switch (params.dataType)
    {
        case DataType::MAT4x4:
        {
            auto setFunc = [=](const std::vector<GLuint> &locations, const MatrixData &matrixData,
                               size_t uniform, size_t frameIndex) {
                glUniformMatrix4fv(locations[uniform], 1, transpose,
                                   matrixData[frameIndex][uniform].data);
            };

            drawLoop<false>(setFunc);
            break;
        }
        case DataType::MAT3x4:
        {
            auto setFunc = [=](const std::vector<GLuint> &locations, const MatrixData &matrixData,
                               size_t uniform, size_t frameIndex) {
                glUniformMatrix3x4fv(locations[uniform], 1, transpose,
                                     matrixData[frameIndex][uniform].data);
            };

            drawLoop<false>(setFunc);
            break;
        }
        case DataType::MAT3x3:
        {
            auto setFunc = [=](const std::vector<GLuint> &locations, const MatrixData &matrixData,
                               size_t uniform, size_t frameIndex) {
                glUniformMatrix3fv(locations[uniform], 1, transpose,
                                   matrixData[frameIndex][uniform].data);
            };

            drawLoop<false>(setFunc);
            break;
        }
        case DataType::VEC4:
        {
            auto setFunc = [](const std::vector<GLuint> &locations, const MatrixData &matrixData,
                              size_t uniform, size_t frameIndex) {
                float value = static_cast<float>(uniform);
                glUniform4f(locations[uniform], value, value, value, value);
            };

            if (params.programMode == ProgramMode::MULTIPLE)
            {
                drawLoop<true>(setFunc);
            }
            else
            {
                drawLoop<false>(setFunc);
            }
            break;
        }
        default:
            UNREACHABLE();
    }

    ASSERT_GL_NO_ERROR();
}

using namespace egl_platform;

UniformsParams VectorUniforms(const EGLPlatformParameters &egl,
                              DataMode dataMode,
                              ProgramMode programMode = ProgramMode::SINGLE)
{
    UniformsParams params;
    params.eglParameters = egl;
    params.dataMode      = dataMode;
    params.programMode   = programMode;
    return params;
}

UniformsParams MatrixUniforms(const EGLPlatformParameters &egl,
                              DataMode dataMode,
                              DataType dataType,
                              MatrixLayout matrixLayout)
{
    UniformsParams params;
    params.eglParameters = egl;
    params.dataType      = dataType;
    params.dataMode      = dataMode;
    params.matrixLayout  = matrixLayout;

    // Reduce the number of uniforms to fit within smaller upper limits on some configs.
    params.numVertexUniforms   = 55;
    params.numFragmentUniforms = 55;

    return params;
}

}  // anonymous namespace

TEST_P(UniformsBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(
    UniformsBenchmark,
    VectorUniforms(D3D11(), DataMode::REPEAT),
    VectorUniforms(D3D11(), DataMode::UPDATE),
    VectorUniforms(D3D11_NULL(), DataMode::UPDATE),
    VectorUniforms(METAL(), DataMode::UPDATE),
    VectorUniforms(METAL(), DataMode::REPEAT),
    VectorUniforms(OPENGL_OR_GLES(), DataMode::UPDATE),
    VectorUniforms(OPENGL_OR_GLES(), DataMode::REPEAT),
    VectorUniforms(OPENGL_OR_GLES_NULL(), DataMode::UPDATE),
    MatrixUniforms(D3D11(), DataMode::UPDATE, DataType::MAT4x4, MatrixLayout::NO_TRANSPOSE),
    MatrixUniforms(METAL(), DataMode::UPDATE, DataType::MAT4x4, MatrixLayout::NO_TRANSPOSE),
    MatrixUniforms(OPENGL_OR_GLES(),
                   DataMode::UPDATE,
                   DataType::MAT4x4,
                   MatrixLayout::NO_TRANSPOSE),
    MatrixUniforms(VULKAN_NULL(), DataMode::UPDATE, DataType::MAT4x4, MatrixLayout::NO_TRANSPOSE),
    MatrixUniforms(VULKAN_NULL(), DataMode::UPDATE, DataType::MAT4x4, MatrixLayout::TRANSPOSE),
    MatrixUniforms(VULKAN_NULL(), DataMode::REPEAT, DataType::MAT4x4, MatrixLayout::NO_TRANSPOSE),
    MatrixUniforms(VULKAN_NULL(), DataMode::UPDATE, DataType::MAT3x4, MatrixLayout::NO_TRANSPOSE),
    MatrixUniforms(VULKAN_NULL(), DataMode::UPDATE, DataType::MAT3x3, MatrixLayout::TRANSPOSE),
    MatrixUniforms(VULKAN_NULL(), DataMode::REPEAT, DataType::MAT3x3, MatrixLayout::TRANSPOSE),
    MatrixUniforms(VULKAN(), DataMode::UPDATE, DataType::MAT4x4, MatrixLayout::NO_TRANSPOSE),
    MatrixUniforms(VULKAN(), DataMode::REPEAT, DataType::MAT4x4, MatrixLayout::NO_TRANSPOSE),
    MatrixUniforms(VULKAN(), DataMode::UPDATE, DataType::MAT3x3, MatrixLayout::NO_TRANSPOSE),
    MatrixUniforms(VULKAN(), DataMode::REPEAT, DataType::MAT3x3, MatrixLayout::NO_TRANSPOSE),
    VectorUniforms(D3D11_NULL(), DataMode::REPEAT, ProgramMode::MULTIPLE));
