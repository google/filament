//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImagelessFramebufferPerfTest:
//   Performance test for imageless framebuffers. It binds and draws many textures to the FBO both
//   using imageless framebuffers (if supported) and with imageless framebuffer disabled.
//

#include "ANGLEPerfTest.h"
#include "test_utils/gl_raii.h"

#include <iostream>
#include <random>
#include <sstream>

namespace angle
{
constexpr unsigned int kIterationsPerStep = 1;
constexpr unsigned int kTextureSize       = 64;
constexpr std::size_t kTextureCount       = 100;

struct ImagelessFramebufferAttachmentParams final : public RenderTestParams
{
    ImagelessFramebufferAttachmentParams()
    {
        iterationsPerStep = kIterationsPerStep;

        // Common default params
        majorVersion = 3;
        minorVersion = 0;
    }

    std::string story() const override;
    bool isImagelessFramebufferEnabled = false;
};

std::ostream &operator<<(std::ostream &os, const ImagelessFramebufferAttachmentParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

std::string ImagelessFramebufferAttachmentParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();
    strstr << (!isImagelessFramebufferEnabled ? "_imageless_framebuffer_disabled" : "_default");

    return strstr.str();
}

class ImagelessFramebufferAttachmentBenchmark
    : public ANGLERenderTest,
      public ::testing::WithParamInterface<ImagelessFramebufferAttachmentParams>
{
  public:
    ImagelessFramebufferAttachmentBenchmark() : ANGLERenderTest("ImagelessFramebuffers", GetParam())
    {}
    void initializeBenchmark() override;
    void drawBenchmark() override;

  protected:
    std::array<GLTexture, kTextureCount> mTextures;
    GLuint mProgram = 0;
};

void ImagelessFramebufferAttachmentBenchmark::initializeBenchmark()
{
    constexpr char kVS[] = R"(void main()
{
    gl_Position = vec4(0);
})";

    constexpr char kFS[] = R"(void main(void)
{
    gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
})";

    mProgram = CompileProgram(kVS, kFS);
    ASSERT_NE(0u, mProgram);
    glUseProgram(mProgram);

    for (GLTexture &texture : mTextures)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kTextureSize, kTextureSize, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
    }
}

void ImagelessFramebufferAttachmentBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    GLFramebuffer fbo;

    for (size_t it = 0; it < params.iterationsPerStep; ++it)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        for (size_t i = 0; i < kTextureCount; ++i)
        {
            for (size_t j = 0; j < kTextureCount; ++j)
            {
                if (j == i)
                    continue;
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                       mTextures[i], 0);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                                       mTextures[j], 0);
                glDrawArrays(GL_POINTS, 0, 1);
            }
        }
    }

    ASSERT_GL_NO_ERROR();
}

ImagelessFramebufferAttachmentParams ImagelessVulkanEnabledParams()
{
    ImagelessFramebufferAttachmentParams params;
    params.eglParameters = egl_platform::VULKAN().disable(Feature::PreferSubmitAtFBOBoundary);
    params.isImagelessFramebufferEnabled = true;

    return params;
}

ImagelessFramebufferAttachmentParams ImagelessVulkanDisabledParams()
{
    ImagelessFramebufferAttachmentParams params;
    params.eglParameters = egl_platform::VULKAN()
                               .disable(Feature::PreferSubmitAtFBOBoundary)
                               .disable(Feature::SupportsImagelessFramebuffer);
    params.isImagelessFramebufferEnabled = false;

    return params;
}

// Runs tests to measure imageless framebuffer performance
TEST_P(ImagelessFramebufferAttachmentBenchmark, Run)
{
    run();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ImagelessFramebufferAttachmentBenchmark);
ANGLE_INSTANTIATE_TEST(ImagelessFramebufferAttachmentBenchmark,
                       ImagelessVulkanEnabledParams(),
                       ImagelessVulkanDisabledParams());

}  // namespace angle
