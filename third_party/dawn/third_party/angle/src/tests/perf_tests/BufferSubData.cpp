//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BufferSubDataBenchmark:
//   Performance test for ANGLE buffer updates.
//

#include <sstream>

#include "ANGLEPerfTest.h"
#include "test_utils/draw_call_perf_utils.h"

using namespace angle;

namespace
{
constexpr unsigned int kIterationsPerStep = 4;

struct BufferSubDataParams final : public RenderTestParams
{
    BufferSubDataParams()
    {
        // Common default values
        majorVersion      = 2;
        minorVersion      = 0;
        windowWidth       = 512;
        windowHeight      = 512;
        updateSize        = 32000;
        bufferSize        = 40000;
        iterationsPerStep = kIterationsPerStep;
        updateRate        = 1;
    }

    std::string story() const override;

    GLboolean vertexNormalized;
    GLenum vertexType;
    GLint vertexComponentCount;
    unsigned int updateRate;

    // static parameters
    GLsizeiptr updateSize;
    GLsizeiptr bufferSize;
};

std::ostream &operator<<(std::ostream &os, const BufferSubDataParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class BufferSubDataBenchmark : public ANGLERenderTest,
                               public ::testing::WithParamInterface<BufferSubDataParams>
{
  public:
    BufferSubDataBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mProgram;
    GLuint mBuffer;
    uint8_t *mUpdateData;
    int mNumTris;
};

GLfloat *GetFloatData(GLint componentCount)
{
    static GLfloat vertices2[] = {
        1, 2, 0, 0, 2, 0,
    };

    static GLfloat vertices3[] = {
        1, 2, 1, 0, 0, 1, 2, 0, 1,
    };

    static GLfloat vertices4[] = {
        1, 2, 1, 3, 0, 0, 1, 3, 2, 0, 1, 3,
    };

    switch (componentCount)
    {
        case 2:
            return vertices2;
        case 3:
            return vertices3;
        case 4:
            return vertices4;
        default:
            return nullptr;
    }
}

template <class T>
GLsizeiptr GetNormalizedData(GLsizeiptr numElements, GLfloat *floatData, std::vector<uint8_t> *data)
{
    GLsizeiptr triDataSize = sizeof(T) * numElements;
    data->resize(triDataSize);

    T *destPtr = reinterpret_cast<T *>(data->data());

    for (GLsizeiptr dataIndex = 0; dataIndex < numElements; dataIndex++)
    {
        GLfloat scaled = floatData[dataIndex] * 0.25f;
        destPtr[dataIndex] =
            static_cast<T>(scaled * static_cast<GLfloat>(std::numeric_limits<T>::max()));
    }

    return triDataSize;
}

template <class T>
GLsizeiptr GetIntData(GLsizeiptr numElements, GLfloat *floatData, std::vector<uint8_t> *data)
{
    GLsizeiptr triDataSize = sizeof(T) * numElements;
    data->resize(triDataSize);

    T *destPtr = reinterpret_cast<T *>(data->data());

    for (GLsizeiptr dataIndex = 0; dataIndex < numElements; dataIndex++)
    {
        destPtr[dataIndex] = static_cast<T>(floatData[dataIndex]);
    }

    return triDataSize;
}

GLsizeiptr GetVertexData(GLenum type,
                         GLint componentCount,
                         GLboolean normalized,
                         std::vector<uint8_t> *data)
{
    GLsizeiptr triDataSize = 0;
    GLfloat *floatData     = GetFloatData(componentCount);

    if (type == GL_FLOAT)
    {
        triDataSize = sizeof(GLfloat) * componentCount * 3;
        data->resize(triDataSize);
        memcpy(data->data(), floatData, triDataSize);
    }
    else if (normalized == GL_TRUE)
    {
        GLsizeiptr numElements = componentCount * 3;

        switch (type)
        {
            case GL_BYTE:
                triDataSize = GetNormalizedData<GLbyte>(numElements, floatData, data);
                break;
            case GL_SHORT:
                triDataSize = GetNormalizedData<GLshort>(numElements, floatData, data);
                break;
            case GL_INT:
                triDataSize = GetNormalizedData<GLint>(numElements, floatData, data);
                break;
            case GL_UNSIGNED_BYTE:
                triDataSize = GetNormalizedData<GLubyte>(numElements, floatData, data);
                break;
            case GL_UNSIGNED_SHORT:
                triDataSize = GetNormalizedData<GLushort>(numElements, floatData, data);
                break;
            case GL_UNSIGNED_INT:
                triDataSize = GetNormalizedData<GLuint>(numElements, floatData, data);
                break;
            default:
                assert(0);
        }
    }
    else
    {
        GLsizeiptr numElements = componentCount * 3;

        switch (type)
        {
            case GL_BYTE:
                triDataSize = GetIntData<GLbyte>(numElements, floatData, data);
                break;
            case GL_SHORT:
                triDataSize = GetIntData<GLshort>(numElements, floatData, data);
                break;
            case GL_INT:
                triDataSize = GetIntData<GLint>(numElements, floatData, data);
                break;
            case GL_UNSIGNED_BYTE:
                triDataSize = GetIntData<GLubyte>(numElements, floatData, data);
                break;
            case GL_UNSIGNED_SHORT:
                triDataSize = GetIntData<GLushort>(numElements, floatData, data);
                break;
            case GL_UNSIGNED_INT:
                triDataSize = GetIntData<GLuint>(numElements, floatData, data);
                break;
            default:
                assert(0);
        }
    }

    return triDataSize;
}

std::string BufferSubDataParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    if (vertexNormalized)
    {
        strstr << "_norm";
    }

    switch (vertexType)
    {
        case GL_FLOAT:
            strstr << "_float";
            break;
        case GL_INT:
            strstr << "_int";
            break;
        case GL_BYTE:
            strstr << "_byte";
            break;
        case GL_SHORT:
            strstr << "_short";
            break;
        case GL_UNSIGNED_INT:
            strstr << "_uint";
            break;
        case GL_UNSIGNED_BYTE:
            strstr << "_ubyte";
            break;
        case GL_UNSIGNED_SHORT:
            strstr << "_ushort";
            break;
        default:
            strstr << "_vunk_" << vertexType << "_";
            break;
    }

    strstr << vertexComponentCount;
    strstr << "_every" << updateRate;

    return strstr.str();
}

BufferSubDataBenchmark::BufferSubDataBenchmark()
    : ANGLERenderTest("BufferSubData", GetParam()),
      mProgram(0),
      mBuffer(0),
      mUpdateData(nullptr),
      mNumTris(0)
{}

void BufferSubDataBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    ASSERT_LT(1, params.vertexComponentCount);

    mProgram = SetupSimpleScaleAndOffsetProgram();
    ASSERT_NE(0u, mProgram);

    if (params.vertexNormalized == GL_TRUE)
    {
        GLfloat scale  = 2.0f;
        GLfloat offset = -0.5f;
        glUniform1f(glGetUniformLocation(mProgram, "uScale"), scale);
        glUniform1f(glGetUniformLocation(mProgram, "uOffset"), offset);
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    std::vector<uint8_t> zeroData(params.bufferSize);
    memset(&zeroData[0], 0, zeroData.size());

    glGenBuffers(1, &mBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, params.bufferSize, &zeroData[0], GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, params.vertexComponentCount, params.vertexType,
                          params.vertexNormalized, 0, 0);
    glEnableVertexAttribArray(0);

    if (params.updateSize > 0)
    {
        mUpdateData = new uint8_t[params.updateSize];
    }

    std::vector<uint8_t> data;
    GLsizei triDataSize = static_cast<GLsizei>(GetVertexData(
        params.vertexType, params.vertexComponentCount, params.vertexNormalized, &data));

    mNumTris = static_cast<int>(params.updateSize / triDataSize);
    for (int i = 0, offset = 0; i < mNumTris; ++i)
    {
        memcpy(mUpdateData + offset, &data[0], triDataSize);
        offset += triDataSize;
    }

    if (params.updateSize == 0)
    {
        mNumTris = 1;
        glBufferSubData(GL_ARRAY_BUFFER, 0, data.size(), &data[0]);
    }

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    ASSERT_GL_NO_ERROR();
}

void BufferSubDataBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mBuffer);
    SafeDeleteArray(mUpdateData);
}

void BufferSubDataBenchmark::drawBenchmark()
{
    glClear(GL_COLOR_BUFFER_BIT);

    const auto &params = GetParam();

    for (unsigned int it = 0; it < params.iterationsPerStep; it++)
    {
        if (params.updateSize > 0 && ((getNumStepsPerformed() % params.updateRate) == 0))
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0, params.updateSize, mUpdateData);
        }

        glDrawArrays(GL_TRIANGLES, 0, 3 * mNumTris);
    }

    ASSERT_GL_NO_ERROR();
}

BufferSubDataParams BufferUpdateD3D11Params()
{
    BufferSubDataParams params;
    params.eglParameters        = egl_platform::D3D11();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    return params;
}

BufferSubDataParams BufferUpdateMetalParams()
{
    BufferSubDataParams params;
    params.eglParameters        = egl_platform::METAL();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    return params;
}

BufferSubDataParams BufferUpdateOpenGLOrGLESParams()
{
    BufferSubDataParams params;
    params.eglParameters        = egl_platform::OPENGL_OR_GLES();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    return params;
}

BufferSubDataParams BufferUpdateVulkanParams()
{
    BufferSubDataParams params;
    params.eglParameters        = egl_platform::VULKAN();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    return params;
}

TEST_P(BufferSubDataBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(BufferSubDataBenchmark,
                       BufferUpdateD3D11Params(),
                       BufferUpdateMetalParams(),
                       BufferUpdateOpenGLOrGLESParams(),
                       BufferUpdateVulkanParams());

}  // namespace
