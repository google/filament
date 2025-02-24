//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RGBImageAllocationBenchmark:
//   GL_RGB8 image allocation and loading using GL_UNSIGNED_BYTE.
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
struct RGBImageAllocationParams final : public RenderTestParams
{
    RGBImageAllocationParams() { iterationsPerStep = 1; }

    std::string story() const override;

    // Used to indicate whether the tested source pointer should be 4-byte-aligned.
    bool aligned;
    size_t textureSize;
};

std::ostream &operator<<(std::ostream &os, const RGBImageAllocationParams &params)
{
    return os << params.backendAndStory().substr(1);
}

std::string RGBImageAllocationParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story() << "_size" << textureSize
           << (aligned ? "_4bytealigned_src" : "_non4bytealigned_src");

    return strstr.str();
}

struct RGBColor
{
    RGBColor() : r(0), g(0), b(0) {}
    RGBColor(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class RGBImageAllocationBenchmark : public ANGLERenderTest,
                                    public ::testing::WithParamInterface<RGBImageAllocationParams>
{
  public:
    RGBImageAllocationBenchmark();

    void initializeBenchmark() override;
    void drawBenchmark() override;
    void destroyBenchmark() override;

  protected:
    std::vector<RGBColor> mColors;
    GLuint mTexture;
    size_t mOffset;
    size_t mTextureSize;
};

RGBImageAllocationBenchmark::RGBImageAllocationBenchmark()
    : ANGLERenderTest("RGBImageAllocation", GetParam())
{
    mOffset      = (GetParam().aligned) ? 0 : 1;
    mTextureSize = GetParam().textureSize;
}

void RGBImageAllocationBenchmark::initializeBenchmark()
{
    // Initialize texture. The size in this test should be set to a power of two for easier data
    // allocation and avoiding alignment issues.
    ASSERT(gl::isPow2(mTextureSize));
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, mTextureSize, mTextureSize);

    // Initialize color data. It is expected for the color data pointer to be 4-byte-aligned. If
    // necessary, an extra pixel is allocated in the beginning to test the non-aligned case.
    ASSERT(reinterpret_cast<uintptr_t>(mColors.data()) % 4 == 0);
    mColors.resize(mOffset + mTextureSize * mTextureSize);
    for (size_t i = 0; i < mTextureSize * mTextureSize; i++)
    {
        mColors[mOffset + i] = RGBColor(1, 2, 3);
    }
}

void RGBImageAllocationBenchmark::destroyBenchmark()
{
    glDeleteTextures(1, &mTexture);
}

void RGBImageAllocationBenchmark::drawBenchmark()
{
    // Copy the next color data.
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mTextureSize, mTextureSize, GL_RGB, GL_UNSIGNED_BYTE,
                    mColors.data() + mOffset);
    glFinish();
    ASSERT_GL_NO_ERROR();
}

RGBImageAllocationParams VulkanParams(bool aligned, size_t textureSize)
{
    RGBImageAllocationParams params;
    params.eglParameters = egl_platform::VULKAN();
    params.majorVersion  = 3;
    params.minorVersion  = 0;
    params.aligned       = aligned;
    params.textureSize   = textureSize;
    return params;
}

RGBImageAllocationParams OpenGLOrGLESParams(bool aligned, size_t textureSize)
{
    RGBImageAllocationParams params;
    params.eglParameters = egl_platform::OPENGL_OR_GLES();
    params.majorVersion  = 3;
    params.minorVersion  = 0;
    params.aligned       = aligned;
    params.textureSize   = textureSize;
    return params;
}

RGBImageAllocationParams MetalParams(bool aligned, size_t textureSize)
{
    RGBImageAllocationParams params;
    params.eglParameters = egl_platform::METAL();
    params.majorVersion  = 3;
    params.minorVersion  = 0;
    params.aligned       = aligned;
    params.textureSize   = textureSize;
    return params;
}

RGBImageAllocationParams D3D11Params(bool aligned, size_t textureSize)
{
    RGBImageAllocationParams params;
    params.eglParameters = egl_platform::D3D11();
    params.majorVersion  = 3;
    params.minorVersion  = 0;
    params.aligned       = aligned;
    params.textureSize   = textureSize;
    return params;
}

}  // anonymous namespace

// Runs the test to measure the performance of RGB8 image allocation and loading.
TEST_P(RGBImageAllocationBenchmark, Run)
{
    run();
}

using namespace params;

ANGLE_INSTANTIATE_TEST(RGBImageAllocationBenchmark,
                       VulkanParams(true, 256),
                       VulkanParams(true, 2048),
                       VulkanParams(false, 2048),
                       OpenGLOrGLESParams(true, 256),
                       OpenGLOrGLESParams(true, 2048),
                       OpenGLOrGLESParams(false, 2048),
                       MetalParams(true, 256),
                       MetalParams(true, 2048),
                       MetalParams(false, 2048),
                       D3D11Params(true, 256),
                       D3D11Params(true, 2048),
                       D3D11Params(false, 2048));
