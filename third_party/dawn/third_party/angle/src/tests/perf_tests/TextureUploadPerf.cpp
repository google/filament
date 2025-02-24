//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureUploadBenchmark:
//   Performance test for uploading texture data.
//

#include "ANGLEPerfTest.h"

#include <iostream>
#include <random>
#include <sstream>

#include "media/etc2bc_rgba8.inc"
#include "test_utils/gl_raii.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{
constexpr unsigned int kIterationsPerStep = 2;

struct TextureUploadParams final : public RenderTestParams
{
    TextureUploadParams()
    {
        iterationsPerStep = kIterationsPerStep;
        trackGpuTime      = true;

        baseSize     = 1024;
        subImageSize = 64;

        webgl = false;
    }

    std::string story() const override;

    GLsizei baseSize;
    GLsizei subImageSize;

    bool webgl;
};

std::ostream &operator<<(std::ostream &os, const TextureUploadParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

std::string TextureUploadParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    if (webgl)
    {
        strstr << "_webgl";
    }

    return strstr.str();
}

class TextureUploadBenchmarkBase : public ANGLERenderTest,
                                   public ::testing::WithParamInterface<TextureUploadParams>
{
  public:
    TextureUploadBenchmarkBase(const char *benchmarkName);

    void initializeBenchmark() override;
    void destroyBenchmark() override;

  protected:
    void initShaders();

    GLuint mProgram    = 0;
    GLint mPositionLoc = -1;
    GLint mSamplerLoc  = -1;
    GLuint mTexture    = 0;
    std::vector<float> mTextureData;
};

class TextureUploadSubImageBenchmark : public TextureUploadBenchmarkBase
{
  public:
    TextureUploadSubImageBenchmark() : TextureUploadBenchmarkBase("TexSubImage")
    {
        addExtensionPrerequisite("GL_EXT_texture_storage");

        if (IsLinux() && IsIntel() &&
            GetParam().eglParameters.renderer == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
        {
            skipTest("http://anglebug.com/42264836");
        }
    }

    void initializeBenchmark() override
    {
        TextureUploadBenchmarkBase::initializeBenchmark();

        const auto &params = GetParam();
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, params.baseSize, params.baseSize);
    }

    void drawBenchmark() override;
};

class TextureUploadFullMipBenchmark : public TextureUploadBenchmarkBase
{
  public:
    TextureUploadFullMipBenchmark() : TextureUploadBenchmarkBase("TextureUpload") {}

    void drawBenchmark() override;
};

class PBOSubImageBenchmark : public TextureUploadBenchmarkBase
{
  public:
    PBOSubImageBenchmark() : TextureUploadBenchmarkBase("PBO") {}

    void initializeBenchmark() override
    {
        TextureUploadBenchmarkBase::initializeBenchmark();

        const auto &params = GetParam();

        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, params.baseSize, params.baseSize);

        glGenBuffers(1, &mPBO);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mPBO);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, params.baseSize * params.baseSize * 4,
                     mTextureData.data(), GL_STREAM_DRAW);
    }

    void destroyBenchmark()
    {
        TextureUploadBenchmarkBase::destroyBenchmark();
        glDeleteBuffers(1, &mPBO);
    }

    void drawBenchmark() override;

  private:
    GLuint mPBO;
};

class PBOCompressedSubImageBenchmark : public TextureUploadBenchmarkBase
{
  public:
    PBOCompressedSubImageBenchmark() : TextureUploadBenchmarkBase("PBOCompressed") {}

    void initializeBenchmark() override
    {
        TextureUploadBenchmarkBase::initializeBenchmark();

        const auto &params = GetParam();

        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGB8_ETC2, params.baseSize,
                          params.baseSize);

        glGenBuffers(1, &mPBO);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mPBO);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, params.subImageSize * params.subImageSize / 2,
                     mTextureData.data(), GL_STREAM_DRAW);
    }

    void destroyBenchmark()
    {
        TextureUploadBenchmarkBase::destroyBenchmark();
        glDeleteBuffers(1, &mPBO);
    }

    void drawBenchmark() override;

  private:
    GLuint mPBO;
};

class TextureUploadETC2TranscodingBenchmark : public TextureUploadBenchmarkBase
{
  public:
    TextureUploadETC2TranscodingBenchmark() : TextureUploadBenchmarkBase("ETC2Transcoding") {}

    void initializeBenchmark() override
    {
        static GLsizei kMaxTextureSize = 2048;
        static GLsizei kMinTextureSize = 4;
        const auto &params             = GetParam();
        ASSERT(params.baseSize <= kMaxTextureSize && params.baseSize >= kMinTextureSize);
        initShaders();
        ASSERT_GL_NO_ERROR();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());
        ASSERT_GL_NO_ERROR();
        mLevels               = (int)ceilf(log2(params.baseSize));
        mTextureBaseLevelSize = 1 << (mLevels - 1);
        // since we testing ETC texture, the minimum block size is 4.
        mLevels -= 2;
        ASSERT_TRUE(mLevels > 0);

        mTextures.resize(params.iterationsPerStep);
        glGenTextures(params.iterationsPerStep, mTextures.data());
        for (unsigned int i = 0; i < params.iterationsPerStep; ++i)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, mTextures[i]);
            glTexStorage2D(GL_TEXTURE_CUBE_MAP, mLevels, GL_COMPRESSED_RGB8_ETC2,
                           mTextureBaseLevelSize, mTextureBaseLevelSize);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glActiveTexture(GL_TEXTURE0);
        ASSERT_GL_NO_ERROR();
    }

    void destroyBenchmark()
    {
        TextureUploadBenchmarkBase::destroyBenchmark();
        glDeleteTextures(static_cast<GLsizei>(mTextures.size()), mTextures.data());
    }

    void drawBenchmark() override;

    void initShaders();

  public:
    GLsizei mTextureBaseLevelSize;
    GLsizei mLevels;
    std::vector<GLuint> mTextures;
};

void TextureUploadETC2TranscodingBenchmark::initShaders()
{
    constexpr char kVS[] = R"(#version 300 es
in vec4 a_position;
void main()
{
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform samplerCube texCube;
out vec4 FragColor;
void main()
{
    FragColor = texture(texCube, vec3(1.0f, 1.0f, 1.0f));
})";

    mProgram = CompileProgram(kVS, kFS);
    ASSERT_NE(0u, mProgram);

    mPositionLoc = glGetAttribLocation(mProgram, "a_position");
    mSamplerLoc  = glGetUniformLocation(mProgram, "texCube");
    glUseProgram(mProgram);
    glUniform1i(mSamplerLoc, 0);

    glDisable(GL_DEPTH_TEST);

    ASSERT_GL_NO_ERROR();
}

void TextureUploadETC2TranscodingBenchmark::drawBenchmark()
{
    const auto &params = GetParam();
    startGpuTimer();
    for (unsigned int iteration = 0; iteration < params.iterationsPerStep; ++iteration)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTextures[iteration]);
        GLsizei size = mTextureBaseLevelSize;
        for (GLint level = 0; level < mLevels; ++level)
        {
            for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                 face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; ++face)
            {
                glCompressedTexSubImage2D(face, level, 0, 0, size, size, GL_COMPRESSED_RGB8_ETC2,
                                          size / 4 * size / 4 * 8, garden_etc2_rgba8);
            }
            size >>= 1;
        }
        // Perform a draw just so the texture data is flushed.  With the position attributes not
        // set, a constant default value is used, resulting in a very cheap draw.
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    stopGpuTimer();
}

TextureUploadBenchmarkBase::TextureUploadBenchmarkBase(const char *benchmarkName)
    : ANGLERenderTest(benchmarkName, GetParam())
{
    setWebGLCompatibilityEnabled(GetParam().webgl);
    setRobustResourceInit(GetParam().webgl);

    if (GetParam().getRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        skipTest("http://crbug.com/945415 Crashes on nvidia+d3d11");
    }
}

void TextureUploadBenchmarkBase::initializeBenchmark()
{
    const auto &params = GetParam();

    initShaders();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    ASSERT_TRUE(params.baseSize >= params.subImageSize);
    mTextureData.resize(params.baseSize * params.baseSize * 4, 0.5);

    ASSERT_GL_NO_ERROR();
}

void TextureUploadBenchmarkBase::initShaders()
{
    constexpr char kVS[] = R"(attribute vec4 a_position;
void main()
{
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(precision mediump float;
uniform sampler2D s_texture;
void main()
{
    gl_FragColor = texture2D(s_texture, vec2(0, 0));
})";

    mProgram = CompileProgram(kVS, kFS);
    ASSERT_NE(0u, mProgram);

    mPositionLoc = glGetAttribLocation(mProgram, "a_position");
    mSamplerLoc  = glGetUniformLocation(mProgram, "s_texture");
    glUseProgram(mProgram);
    glUniform1i(mSamplerLoc, 0);

    glDisable(GL_DEPTH_TEST);

    ASSERT_GL_NO_ERROR();
}

void TextureUploadBenchmarkBase::destroyBenchmark()
{
    glDeleteTextures(1, &mTexture);
    glDeleteProgram(mProgram);
}

void TextureUploadSubImageBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    startGpuTimer();
    for (unsigned int iteration = 0; iteration < params.iterationsPerStep; ++iteration)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, rand() % (params.baseSize - params.subImageSize),
                        rand() % (params.baseSize - params.subImageSize), params.subImageSize,
                        params.subImageSize, GL_RGBA, GL_UNSIGNED_BYTE, mTextureData.data());

        // Perform a draw just so the texture data is flushed.  With the position attributes not
        // set, a constant default value is used, resulting in a very cheap draw.
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    stopGpuTimer();

    ASSERT_GL_NO_ERROR();
}

void TextureUploadFullMipBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    startGpuTimer();
    for (size_t it = 0; it < params.iterationsPerStep; ++it)
    {
        // Stage data for all mips
        GLint mip = 0;
        for (GLsizei levelSize = params.baseSize; levelSize > 0; levelSize >>= 1)
        {
            glTexImage2D(GL_TEXTURE_2D, mip++, GL_RGBA, levelSize, levelSize, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, mTextureData.data());
        }

        // Perform a draw just so the texture data is flushed.  With the position attributes not
        // set, a constant default value is used, resulting in a very cheap draw.
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    stopGpuTimer();

    ASSERT_GL_NO_ERROR();
}

void PBOSubImageBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    startGpuTimer();
    for (unsigned int iteration = 0; iteration < params.iterationsPerStep; ++iteration)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, rand() % (params.baseSize - params.subImageSize),
                        rand() % (params.baseSize - params.subImageSize), params.subImageSize,
                        params.subImageSize, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        // Perform a draw just so the texture data is flushed.  With the position attributes not
        // set, a constant default value is used, resulting in a very cheap draw.
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    stopGpuTimer();

    ASSERT_GL_NO_ERROR();
}

void PBOCompressedSubImageBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    startGpuTimer();
    for (unsigned int iteration = 0; iteration < params.iterationsPerStep; ++iteration)
    {
        glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, params.subImageSize, params.subImageSize,
                                  GL_COMPRESSED_RGB8_ETC2,
                                  params.subImageSize * params.subImageSize / 2, 0);

        // Perform a draw just so the texture data is flushed.  With the position attributes not
        // set, a constant default value is used, resulting in a very cheap draw.
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    stopGpuTimer();

    ASSERT_GL_NO_ERROR();
}

TextureUploadParams D3D11Params(bool webglCompat)
{
    TextureUploadParams params;
    params.eglParameters = egl_platform::D3D11();
    params.webgl         = webglCompat;
    return params;
}

TextureUploadParams MetalParams(bool webglCompat)
{
    TextureUploadParams params;
    params.eglParameters = egl_platform::METAL();
    params.webgl         = webglCompat;
    return params;
}

TextureUploadParams OpenGLOrGLESParams(bool webglCompat)
{
    TextureUploadParams params;
    params.eglParameters = egl_platform::OPENGL_OR_GLES();
    params.webgl         = webglCompat;
    return params;
}

TextureUploadParams VulkanParams(bool webglCompat)
{
    TextureUploadParams params;
    params.eglParameters = egl_platform::VULKAN();
    params.webgl         = webglCompat;
    return params;
}

TextureUploadParams ES3VulkanParams(bool webglCompat)
{
    TextureUploadParams params;
    params.eglParameters = egl_platform::VULKAN();
    params.webgl         = webglCompat;
    params.majorVersion  = 3;
    params.minorVersion  = 0;
    params.enable(Feature::SupportsComputeTranscodeEtcToBc);
    params.iterationsPerStep = 16;
    params.baseSize          = 256;
    return params;
}

TextureUploadParams MetalPBOParams(GLsizei baseSize, GLsizei subImageSize)
{
    TextureUploadParams params;
    params.eglParameters = egl_platform::METAL();
    params.webgl         = false;
    params.trackGpuTime  = false;
    params.baseSize      = baseSize;
    params.subImageSize  = subImageSize;
    return params;
}

TextureUploadParams VulkanPBOParams(GLsizei baseSize, GLsizei subImageSize)
{
    TextureUploadParams params;
    params.eglParameters = egl_platform::VULKAN();
    params.webgl         = false;
    params.trackGpuTime  = false;
    params.baseSize      = baseSize;
    params.subImageSize  = subImageSize;
    return params;
}

TextureUploadParams ES3OpenGLPBOParams(GLsizei baseSize, GLsizei subImageSize)
{
    TextureUploadParams params;
    params.eglParameters = egl_platform::OPENGL();
    params.majorVersion  = 3;
    params.minorVersion  = 0;
    params.webgl         = false;
    params.trackGpuTime  = false;
    params.baseSize      = baseSize;
    params.subImageSize  = subImageSize;
    return params;
}

}  // anonymous namespace

// Test etc to bc transcoding performance.
TEST_P(TextureUploadETC2TranscodingBenchmark, Run)
{
    run();
}

TEST_P(TextureUploadSubImageBenchmark, Run)
{
    run();
}

TEST_P(TextureUploadFullMipBenchmark, Run)
{
    run();
}

TEST_P(PBOSubImageBenchmark, Run)
{
    run();
}

TEST_P(PBOCompressedSubImageBenchmark, Run)
{
    run();
}

using namespace params;

ANGLE_INSTANTIATE_TEST(TextureUploadSubImageBenchmark,
                       D3D11Params(false),
                       D3D11Params(true),
                       MetalParams(false),
                       MetalParams(true),
                       OpenGLOrGLESParams(false),
                       OpenGLOrGLESParams(true),
                       VulkanParams(false),
                       NullDevice(VulkanParams(false)),
                       VulkanParams(true));

ANGLE_INSTANTIATE_TEST(TextureUploadETC2TranscodingBenchmark, ES3VulkanParams(false));

ANGLE_INSTANTIATE_TEST(TextureUploadFullMipBenchmark,
                       D3D11Params(false),
                       D3D11Params(true),
                       MetalParams(false),
                       MetalParams(true),
                       OpenGLOrGLESParams(false),
                       OpenGLOrGLESParams(true),
                       VulkanParams(false),
                       VulkanParams(true));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(PBOSubImageBenchmark);
ANGLE_INSTANTIATE_TEST(PBOSubImageBenchmark,
                       ES3OpenGLPBOParams(1024, 128),
                       MetalPBOParams(1024, 128),
                       VulkanPBOParams(1024, 128));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(PBOCompressedSubImageBenchmark);
ANGLE_INSTANTIATE_TEST(PBOCompressedSubImageBenchmark,
                       ES3OpenGLPBOParams(128, 128),
                       MetalPBOParams(128, 128),
                       VulkanPBOParams(128, 128));
