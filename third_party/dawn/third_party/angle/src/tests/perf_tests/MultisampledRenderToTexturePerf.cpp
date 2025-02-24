//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MultisampledRenderToTextureBenchmark:
//   Performance test for rendering to multisampled-render-to-texture attachments.
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
constexpr uint32_t kMultipassPassCount = 5;

struct MultisampledRenderToTextureParams final : public RenderTestParams
{
    MultisampledRenderToTextureParams()
    {
        iterationsPerStep = 1;
        trackGpuTime      = true;

        textureWidth  = 1920;
        textureHeight = 1080;

        multiplePasses   = false;
        withDepthStencil = false;
    }

    std::string story() const override;

    GLsizei textureWidth;
    GLsizei textureHeight;

    bool multiplePasses;
    bool withDepthStencil;
};

std::ostream &operator<<(std::ostream &os, const MultisampledRenderToTextureParams &params)
{
    return os << params.backendAndStory().substr(1);
}

std::string MultisampledRenderToTextureParams::story() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::story();

    if (multiplePasses)
    {
        strstr << "_multipass";
    }

    if (withDepthStencil)
    {
        strstr << "_ds";
    }

    return strstr.str();
}

class MultisampledRenderToTextureBenchmark
    : public ANGLERenderTest,
      public ::testing::WithParamInterface<MultisampledRenderToTextureParams>
{
  public:
    MultisampledRenderToTextureBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  protected:
    void initShaders();

    GLuint mFramebuffer              = 0;
    GLuint mProgram                  = 0;
    GLuint mColorTexture             = 0;
    GLuint mDepthStencilRenderbuffer = 0;

    std::vector<uint8_t> mTextureData;
};

MultisampledRenderToTextureBenchmark::MultisampledRenderToTextureBenchmark()
    : ANGLERenderTest("MultisampledRenderToTexture", GetParam())
{
    if (GetParam().getRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        skipTest("http://crbug.com/945415 Crashes on nvidia+d3d11");
    }

    if (IsWindows7() && IsNVIDIA() &&
        GetParam().eglParameters.renderer == EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE)
    {
        skipTest(
            "http://crbug.com/1096510 Fails on Windows7 NVIDIA Vulkan, presumably due to old "
            "drivers");
    }

    if (IsPixel4() && GetParam().eglParameters.renderer == EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE)
    {
        skipTest("http://anglebug.com/40096724 Fails on Pixel 4 GLES");
    }

    if (IsLinux() && IsAMD() &&
        GetParam().eglParameters.renderer == EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE &&
        GetParam().multiplePasses && GetParam().withDepthStencil)
    {
        skipTest("http://anglebug.com/42263920");
    }

    if (IsLinux() && IsIntel() &&
        GetParam().eglParameters.renderer == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
    {
        skipTest("http://anglebug.com/42264836");
    }
}

void MultisampledRenderToTextureBenchmark::initializeBenchmark()
{
    if (!IsGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"))
    {
        skipTest("missing GL_EXT_multisampled_render_to_texture");
        return;
    }

    const auto &params = GetParam();

    initShaders();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    glGenFramebuffers(1, &mFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

    glGenTextures(1, &mColorTexture);
    glBindTexture(GL_TEXTURE_2D, mColorTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, params.textureWidth, params.textureHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         mColorTexture, 0, 4);

    if (params.withDepthStencil)
    {
        glGenRenderbuffers(1, &mDepthStencilRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRenderbuffer);
        glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8,
                                            params.textureWidth, params.textureHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  mDepthStencilRenderbuffer);
    }

    ASSERT_GL_NO_ERROR();
}

void MultisampledRenderToTextureBenchmark::initShaders()
{
    constexpr char kVS[] = R"(void main()
{
    gl_Position = vec4(0, 0, 0, 1);
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
}

void MultisampledRenderToTextureBenchmark::destroyBenchmark()
{
    glDeleteFramebuffers(1, &mFramebuffer);
    glDeleteRenderbuffers(1, &mDepthStencilRenderbuffer);
    glDeleteTextures(1, &mColorTexture);
    glDeleteProgram(mProgram);
}

void MultisampledRenderToTextureBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    GLTexture mMockTexture;
    glBindTexture(GL_TEXTURE_2D, mMockTexture);

    startGpuTimer();

    // Initially clear the color attachment to avoid having to load from the resolved image.  The
    // depth/stencil attachment doesn't need this as it's contents are always undefined between
    // render passes.
    glClear(GL_COLOR_BUFFER_BIT);

    const uint32_t passCount = params.multiplePasses ? kMultipassPassCount : 1;
    for (uint32_t pass = 0; pass < passCount; ++pass)
    {
        // Perform a draw just to have something in the render pass.  With the position attributes
        // not set, a constant default value is used, resulting in a very cheap draw.
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Force the render pass to break by cheaply reading back from the color attachment.
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);
    }
    stopGpuTimer();

    ASSERT_GL_NO_ERROR();
}

MultisampledRenderToTextureParams D3D11Params(bool multiplePasses, bool withDepthStencil)
{
    MultisampledRenderToTextureParams params;
    params.eglParameters    = egl_platform::D3D11();
    params.majorVersion     = 3;
    params.minorVersion     = 0;
    params.multiplePasses   = multiplePasses;
    params.withDepthStencil = withDepthStencil;
    return params;
}

MultisampledRenderToTextureParams MetalParams(bool multiplePasses, bool withDepthStencil)
{
    MultisampledRenderToTextureParams params;
    params.eglParameters    = egl_platform::METAL();
    params.majorVersion     = 3;
    params.minorVersion     = 0;
    params.multiplePasses   = multiplePasses;
    params.withDepthStencil = withDepthStencil;
    return params;
}

MultisampledRenderToTextureParams OpenGLOrGLESParams(bool multiplePasses, bool withDepthStencil)
{
    MultisampledRenderToTextureParams params;
    params.eglParameters    = egl_platform::OPENGL_OR_GLES();
    params.majorVersion     = 3;
    params.minorVersion     = 0;
    params.multiplePasses   = multiplePasses;
    params.withDepthStencil = withDepthStencil;
    return params;
}

MultisampledRenderToTextureParams VulkanParams(bool multiplePasses, bool withDepthStencil)
{
    MultisampledRenderToTextureParams params;
    params.eglParameters    = egl_platform::VULKAN();
    params.majorVersion     = 3;
    params.minorVersion     = 0;
    params.multiplePasses   = multiplePasses;
    params.withDepthStencil = withDepthStencil;
    return params;
}

}  // anonymous namespace

TEST_P(MultisampledRenderToTextureBenchmark, Run)
{
    run();
}

using namespace params;

ANGLE_INSTANTIATE_TEST(MultisampledRenderToTextureBenchmark,
                       D3D11Params(false, false),
                       D3D11Params(true, true),
                       MetalParams(false, false),
                       MetalParams(true, true),
                       OpenGLOrGLESParams(false, false),
                       OpenGLOrGLESParams(true, true),
                       VulkanParams(false, false),
                       VulkanParams(true, false),
                       VulkanParams(false, true),
                       VulkanParams(true, true));
