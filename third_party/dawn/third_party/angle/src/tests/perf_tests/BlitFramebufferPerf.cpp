//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BlitFramebufferPerf:
//   Performance tests for glBlitFramebuffer in ES3. Includes tests for
//   color, depth, and stencil blit, as well as the mutlisample versions.
//   The test works by clearing a framebuffer, then blitting it to a second.

#include "ANGLEPerfTest.h"

#include "util/gles_loader_autogen.h"

namespace
{
constexpr unsigned int kIterationsPerStep = 5;

enum class BufferType
{
    COLOR,
    DEPTH,
    STENCIL,
    DEPTH_STENCIL
};

const char *BufferTypeString(BufferType type)
{
    switch (type)
    {
        case BufferType::COLOR:
            return "color";
        case BufferType::DEPTH:
            return "depth";
        case BufferType::STENCIL:
            return "stencil";
        case BufferType::DEPTH_STENCIL:
            return "depth_stencil";
        default:
            return "error";
    }
}

GLbitfield BufferTypeMask(BufferType type)
{
    switch (type)
    {
        case BufferType::COLOR:
            return GL_COLOR_BUFFER_BIT;
        case BufferType::DEPTH:
            return GL_DEPTH_BUFFER_BIT;
        case BufferType::STENCIL:
            return GL_STENCIL_BUFFER_BIT;
        case BufferType::DEPTH_STENCIL:
            return (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        default:
            return 0;
    }
}

GLenum BufferTypeFormat(BufferType type)
{
    switch (type)
    {
        case BufferType::COLOR:
            return GL_RGBA8;
        case BufferType::DEPTH:
            return GL_DEPTH_COMPONENT24;
        case BufferType::STENCIL:
            return GL_STENCIL_INDEX8;
        case BufferType::DEPTH_STENCIL:
            return GL_DEPTH24_STENCIL8;
        default:
            return GL_NONE;
    }
}

GLenum BufferTypeAttachment(BufferType type)
{
    switch (type)
    {
        case BufferType::COLOR:
            return GL_COLOR_ATTACHMENT0;
        case BufferType::DEPTH:
            return GL_DEPTH_ATTACHMENT;
        case BufferType::STENCIL:
            return GL_STENCIL_ATTACHMENT;
        case BufferType::DEPTH_STENCIL:
            return GL_DEPTH_STENCIL_ATTACHMENT;
        default:
            return GL_NONE;
    }
}

struct BlitFramebufferParams final : public RenderTestParams
{
    BlitFramebufferParams()
    {
        iterationsPerStep = kIterationsPerStep;
        majorVersion      = 3;
        minorVersion      = 0;
        windowWidth       = 256;
        windowHeight      = 256;
    }

    std::string story() const override
    {
        std::stringstream storyStr;
        storyStr << RenderTestParams::story();
        storyStr << "_" << BufferTypeString(type);
        if (samples > 1)
        {
            storyStr << "_" << samples << "_samples";
        }
        return storyStr.str();
    }

    BufferType type              = BufferType::COLOR;
    unsigned int framebufferSize = 512;
    unsigned int samples         = 0;
};

std::ostream &operator<<(std::ostream &os, const BlitFramebufferParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class BlitFramebufferPerf : public ANGLERenderTest,
                            public ::testing::WithParamInterface<BlitFramebufferParams>
{
  public:
    BlitFramebufferPerf() : ANGLERenderTest("BlitFramebufferPerf", GetParam()) {}

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mReadFramebuffer  = 0;
    GLuint mReadRenderbuffer = 0;
    GLuint mDrawFramebuffer  = 0;
    GLuint mDrawRenderbuffer = 0;
};

void BlitFramebufferPerf::initializeBenchmark()
{
    const auto &param = GetParam();

    glGenFramebuffers(1, &mReadFramebuffer);
    glGenFramebuffers(1, &mDrawFramebuffer);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mReadFramebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawFramebuffer);

    // Create source and destination Renderbuffers.
    glGenRenderbuffers(1, &mReadRenderbuffer);
    glGenRenderbuffers(1, &mDrawRenderbuffer);

    ASSERT_GL_NO_ERROR();

    GLenum format     = BufferTypeFormat(param.type);
    GLuint size       = param.framebufferSize;
    GLenum attachment = BufferTypeAttachment(param.type);

    glBindRenderbuffer(GL_RENDERBUFFER, mReadRenderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, param.samples, format, size, size);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, attachment, GL_RENDERBUFFER, mReadRenderbuffer);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));

    glBindRenderbuffer(GL_RENDERBUFFER, mDrawRenderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0, format, size, size);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, attachment, GL_RENDERBUFFER, mDrawRenderbuffer);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

    ASSERT_GL_NO_ERROR();
}

void BlitFramebufferPerf::destroyBenchmark()
{
    glDeleteFramebuffers(1, &mReadFramebuffer);
    glDeleteRenderbuffers(1, &mReadRenderbuffer);
    glDeleteFramebuffers(1, &mDrawFramebuffer);
    glDeleteRenderbuffers(1, &mDrawRenderbuffer);
}

void BlitFramebufferPerf::drawBenchmark()
{
    const auto &param = GetParam();
    auto size         = param.framebufferSize;
    auto mask         = BufferTypeMask(param.type);

    // We don't read from the draw buffer (ie rendering) to simplify the test, but we could.
    // This might trigger a flush, or we could trigger a flush manually to ensure the blit happens.
    // TODO(jmadill): Investigate performance on Vulkan, and placement of Clear call.

    switch (param.type)
    {
        case BufferType::COLOR:
        {
            GLfloat clearValues[4] = {1.0f, 0.0f, 0.0f, 1.0f};
            glClearBufferfv(GL_COLOR, 0, clearValues);
            break;
        }
        case BufferType::DEPTH:
        {
            GLfloat clearDepthValue = 0.5f;
            glClearBufferfv(GL_DEPTH, 0, &clearDepthValue);
            break;
        }
        case BufferType::STENCIL:
        {
            GLint clearStencilValue = 1;
            glClearBufferiv(GL_STENCIL, 0, &clearStencilValue);
            break;
        }
        case BufferType::DEPTH_STENCIL:
            glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.5f, 1);
            break;
    }

    for (unsigned int iteration = 0; iteration < param.iterationsPerStep; ++iteration)
    {
        glBlitFramebuffer(0, 0, size, size, 0, 0, size, size, mask, GL_NEAREST);
    }
}

TEST_P(BlitFramebufferPerf, Run)
{
    run();
}

BlitFramebufferParams Vulkan(BufferType type, unsigned int samples)
{
    BlitFramebufferParams params;
    params.eglParameters = angle::egl_platform::VULKAN();
    params.type          = type;
    params.samples       = samples;
    return params;
}
BlitFramebufferParams D3D11(BufferType type, unsigned int samples)
{
    BlitFramebufferParams params;
    params.eglParameters = angle::egl_platform::D3D11();
    params.type          = type;
    params.samples       = samples;
    return params;
}
}  // anonymous namespace

// TODO(jmadill): Programatically generate these combinations.
ANGLE_INSTANTIATE_TEST(BlitFramebufferPerf,
                       D3D11(BufferType::COLOR, 0),
                       D3D11(BufferType::DEPTH, 0),
                       D3D11(BufferType::STENCIL, 0),
                       D3D11(BufferType::DEPTH_STENCIL, 0),
                       D3D11(BufferType::COLOR, 2),
                       D3D11(BufferType::DEPTH, 2),
                       D3D11(BufferType::STENCIL, 2),
                       D3D11(BufferType::DEPTH_STENCIL, 2),
                       Vulkan(BufferType::COLOR, 0),
                       Vulkan(BufferType::DEPTH, 0),
                       Vulkan(BufferType::STENCIL, 0),
                       Vulkan(BufferType::DEPTH_STENCIL, 0),
                       Vulkan(BufferType::COLOR, 2),
                       Vulkan(BufferType::DEPTH, 2),
                       Vulkan(BufferType::STENCIL, 2),
                       Vulkan(BufferType::DEPTH_STENCIL, 2));

// This test suite is not instantiated on some OSes.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(BlitFramebufferPerf);
