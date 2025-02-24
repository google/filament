//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MapBufferRangeBenchmark::
//   Performance test for ANGLE GLES mapped buffers.
//

#include "ANGLEPerfTest.h"

#include <sstream>
#include <vector>

#include "common/debug.h"
#include "test_utils/draw_call_perf_utils.h"

using namespace angle;

namespace
{
constexpr unsigned int kIterationsPerStep = 10;

struct MapBufferRangeParams final : public RenderTestParams
{
    MapBufferRangeParams()
    {
        // Common default values
        majorVersion = 3;
        minorVersion = 0;
        windowWidth  = 512;
        windowHeight = 512;
        // Test intentionally small update versus buffer size to begin with.
        updateSize        = 32768;
        updateOffset      = 0;
        bufferSize        = 1048576;
        iterationsPerStep = kIterationsPerStep;
        access            = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
    }

    std::string story() const override;

    GLboolean vertexNormalized;
    GLenum vertexType;
    GLint vertexComponentCount;

    // static parameters
    GLsizeiptr updateSize;
    GLsizeiptr updateOffset;
    GLsizeiptr bufferSize;
    GLbitfield access;
};

std::ostream &operator<<(std::ostream &os, const MapBufferRangeParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class MapBufferRangeBenchmark : public ANGLERenderTest,
                                public ::testing::WithParamInterface<MapBufferRangeParams>
{
  public:
    MapBufferRangeBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mProgram;
    GLuint mBuffer;
    std::vector<uint8_t> mVertexData;
    int mTriSize;
    int mNumUpdateTris;
};

const GLfloat *GetFloatData(GLint componentCount)
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
            UNREACHABLE();
    }

    return 0;
}

template <class T>
GLsizei GetNormalizedData(GLsizeiptr numElements,
                          const GLfloat *floatData,
                          std::vector<uint8_t> *data)
{
    GLsizei triDataSize = sizeof(T) * numElements;
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
GLsizei GetIntData(GLsizeiptr numElements, const GLfloat *floatData, std::vector<uint8_t> *data)
{
    GLsizei triDataSize = sizeof(T) * numElements;
    data->resize(triDataSize);

    T *destPtr = reinterpret_cast<T *>(data->data());

    for (GLsizeiptr dataIndex = 0; dataIndex < numElements; dataIndex++)
    {
        destPtr[dataIndex] = static_cast<T>(floatData[dataIndex]);
    }

    return triDataSize;
}

GLsizei GetVertexData(GLenum type,
                      GLint componentCount,
                      GLboolean normalized,
                      std::vector<uint8_t> *data)
{
    GLsizei triDataSize      = 0;
    const GLfloat *floatData = GetFloatData(componentCount);

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
                UNREACHABLE();
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

std::string MapBufferRangeParams::story() const
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
            UNREACHABLE();
    }

    strstr << vertexComponentCount;
    strstr << "_updateOffset" << updateOffset;
    strstr << "_updateSize" << updateSize;
    strstr << "_bufferSize" << bufferSize;
    strstr << "_access0x" << std::hex << access;

    return strstr.str();
}

MapBufferRangeBenchmark::MapBufferRangeBenchmark()
    : ANGLERenderTest("MapBufferRange", GetParam()),
      mProgram(0),
      mBuffer(0),
      mTriSize(0),
      mNumUpdateTris(0)
{}

void MapBufferRangeBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    ASSERT_LT(1, params.vertexComponentCount);
    ASSERT_LE(params.updateSize, params.bufferSize);
    ASSERT_LT(params.updateOffset, params.bufferSize);
    ASSERT_LE(params.updateOffset + params.updateSize, params.bufferSize);

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

    glGenBuffers(1, &mBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, params.bufferSize, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, params.vertexComponentCount, params.vertexType,
                          params.vertexNormalized, 0, 0);
    glEnableVertexAttribArray(0);

    mTriSize = GetVertexData(params.vertexType, params.vertexComponentCount,
                             params.vertexNormalized, &mVertexData);

    mNumUpdateTris = static_cast<int>(params.updateSize / mTriSize);
    int totalTris  = static_cast<int>(params.updateSize / mTriSize);

    mVertexData.resize(params.bufferSize);

    for (int i = 1; i < totalTris; ++i)
    {
        memcpy(mVertexData.data() + i * mTriSize, mVertexData.data(), mTriSize);
    }

    if (params.updateSize == 0)
    {
        mNumUpdateTris = 1;
        glBufferSubData(GL_ARRAY_BUFFER, 0, mVertexData.size(), mVertexData.data());
    }

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    ASSERT_GL_NO_ERROR();
}

void MapBufferRangeBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mBuffer);
}

void MapBufferRangeBenchmark::drawBenchmark()
{
    glClear(GL_COLOR_BUFFER_BIT);

    const auto &params = GetParam();

    for (unsigned int it = 0; it < params.iterationsPerStep; it++)
    {
        if (params.updateSize > 0)
        {
            void *mapPtr = glMapBufferRange(GL_ARRAY_BUFFER, params.updateOffset, params.updateSize,
                                            params.access);
            memcpy(mapPtr, mVertexData.data() + params.updateOffset, params.updateSize);
            glUnmapBuffer(GL_ARRAY_BUFFER);
        }

        glDrawArrays(GL_TRIANGLES, params.updateOffset / mTriSize, 3 * mNumUpdateTris);
    }

    ASSERT_GL_NO_ERROR();
}

MapBufferRangeParams BufferUpdateD3D11Params()
{
    MapBufferRangeParams params;
    params.eglParameters        = egl_platform::D3D11();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    return params;
}

MapBufferRangeParams BufferUpdateMetalParams()
{
    MapBufferRangeParams params;
    params.eglParameters        = egl_platform::METAL();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    return params;
}

MapBufferRangeParams BufferUpdateMetalParamsLargeUpdate()
{
    MapBufferRangeParams params;
    params.eglParameters        = egl_platform::METAL();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    params.updateSize           = 524288;
    return params;
}

MapBufferRangeParams BufferUpdateOpenGLOrGLESParams()
{
    MapBufferRangeParams params;
    params.eglParameters        = egl_platform::OPENGL_OR_GLES();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    return params;
}

MapBufferRangeParams BufferUpdateVulkanParams()
{
    MapBufferRangeParams params;
    params.eglParameters        = egl_platform::VULKAN();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    return params;
}

MapBufferRangeParams BufferUpdateVulkanParamsMidBuffer()
{
    MapBufferRangeParams params;
    params.eglParameters        = egl_platform::VULKAN();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    params.updateOffset         = 524288;
    return params;
}

MapBufferRangeParams BufferUpdateVulkanParamsLargeUpdate()
{
    MapBufferRangeParams params;
    params.eglParameters        = egl_platform::VULKAN();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    params.updateSize           = 524288;
    return params;
}

MapBufferRangeParams BufferUpdateVulkanParamsFullBuffer()
{
    MapBufferRangeParams params;
    params.eglParameters        = egl_platform::VULKAN();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    params.updateSize           = 1048576;
    return params;
}

MapBufferRangeParams BufferUpdateVulkanParamsTinyUpdate()
{
    MapBufferRangeParams params;
    params.eglParameters        = egl_platform::VULKAN();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    params.updateSize           = 128;
    return params;
}

MapBufferRangeParams BufferUpdateVulkanParamsNonPowerOf2()
{
    MapBufferRangeParams params;
    params.eglParameters        = egl_platform::VULKAN();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    params.updateSize           = 32000;
    params.bufferSize           = 800000;
    return params;
}

MapBufferRangeParams BufferUpdateVulkanParamsUnsynchronized()
{
    MapBufferRangeParams params;
    params.eglParameters        = egl_platform::VULKAN();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    params.access               = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
    return params;
}

MapBufferRangeParams BufferUpdateVulkanParamsLargeUpdateUnsynchronized()
{
    MapBufferRangeParams params;
    params.eglParameters        = egl_platform::VULKAN();
    params.vertexType           = GL_FLOAT;
    params.vertexComponentCount = 4;
    params.vertexNormalized     = GL_FALSE;
    params.updateSize           = 524288;
    params.access               = GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
    return params;
}

TEST_P(MapBufferRangeBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(MapBufferRangeBenchmark,
                       BufferUpdateD3D11Params(),
                       BufferUpdateMetalParams(),
                       BufferUpdateMetalParamsLargeUpdate(),
                       BufferUpdateOpenGLOrGLESParams(),
                       BufferUpdateVulkanParams(),
                       BufferUpdateVulkanParamsMidBuffer(),
                       BufferUpdateVulkanParamsLargeUpdate(),
                       BufferUpdateVulkanParamsFullBuffer(),
                       BufferUpdateVulkanParamsTinyUpdate(),
                       BufferUpdateVulkanParamsNonPowerOf2(),
                       BufferUpdateVulkanParamsUnsynchronized(),
                       BufferUpdateVulkanParamsLargeUpdateUnsynchronized());

}  // namespace
