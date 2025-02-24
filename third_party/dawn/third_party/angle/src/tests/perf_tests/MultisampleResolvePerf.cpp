//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MultisampleResolvePerf:
//   Performance tests for glBlitFramebuffer and glInvalidateFramebuffer where the framebuffer is
//   multisampled.

#include "ANGLEPerfTest.h"

#include "util/shader_utils.h"

namespace
{
constexpr unsigned int kIterationsPerStep = 1;

enum class Multisample
{
    Yes,
    No,
};

enum class WithDepthStencil
{
    Yes,
    No,
};

struct MultisampleResolveParams final : public RenderTestParams
{
    MultisampleResolveParams(Multisample multisample, WithDepthStencil depthStencil)
    {
        iterationsPerStep = kIterationsPerStep;
        majorVersion      = 3;
        minorVersion      = 0;
        windowWidth       = 256;
        windowHeight      = 256;

        if (multisample == Multisample::No)
        {
            samples = 0;
        }

        withDepthStencil = depthStencil == WithDepthStencil::Yes;
    }

    std::string story() const override
    {
        std::stringstream storyStr;
        storyStr << RenderTestParams::story();
        if (withDepthStencil)
        {
            storyStr << "_ds";
        }
        if (samples == 0)
        {
            storyStr << "_singlesampled_reference";
        }
        return storyStr.str();
    }

    unsigned int framebufferSize = 1024;
    unsigned int samples         = 4;
    bool withDepthStencil        = false;
};

std::ostream &operator<<(std::ostream &os, const MultisampleResolveParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class MultisampleResolvePerf : public ANGLERenderTest,
                               public ::testing::WithParamInterface<MultisampleResolveParams>
{
  public:
    MultisampleResolvePerf() : ANGLERenderTest("MultisampleResolvePerf", GetParam()) {}

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mMSAAFramebuffer       = 0;
    GLuint mMSAAColor[2]          = {};
    GLuint mMSAADepthStencil      = 0;
    GLuint mResolveFramebuffer[2] = {};
    GLuint mResolveColor[2]       = {};
    GLuint mResolveDepthStencil   = 0;
    GLuint mReferenceFramebuffer  = 0;
    GLuint mProgram               = 0;
};

void MultisampleResolvePerf::initializeBenchmark()
{
    const MultisampleResolveParams &param = GetParam();

    glGenFramebuffers(1, &mMSAAFramebuffer);
    glGenFramebuffers(2, mResolveFramebuffer);
    glGenFramebuffers(1, &mReferenceFramebuffer);

    // Create source and destination Renderbuffers.
    glGenRenderbuffers(2, mMSAAColor);
    glGenRenderbuffers(1, &mMSAADepthStencil);
    glGenRenderbuffers(2, mResolveColor);
    glGenRenderbuffers(1, &mResolveDepthStencil);

    ASSERT_GL_NO_ERROR();

    const GLuint size = param.framebufferSize;

    for (uint32_t i = 0; i < 2; ++i)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, mResolveColor[i]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, size, size);

        glBindFramebuffer(GL_FRAMEBUFFER, mResolveFramebuffer[i]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  mResolveColor[i]);
        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

    if (param.withDepthStencil)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, mResolveDepthStencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size, size);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  mResolveDepthStencil);
        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

    if (param.samples > 0)
    {
        glBindRenderbuffer(GL_RENDERBUFFER, mMSAAColor[0]);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, param.samples, GL_RGBA8, size, size);
        glBindRenderbuffer(GL_RENDERBUFFER, mMSAAColor[1]);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, param.samples, GL_RGBA8, size, size);

        glBindFramebuffer(GL_FRAMEBUFFER, mMSAAFramebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  mMSAAColor[0]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER,
                                  mMSAAColor[1]);

        if (param.withDepthStencil)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, mMSAADepthStencil);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, param.samples, GL_DEPTH24_STENCIL8,
                                             size, size);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                      mMSAADepthStencil);
        }

        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mReferenceFramebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  mResolveColor[0]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER,
                                  mResolveColor[1]);

        if (param.withDepthStencil)
        {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                      mResolveDepthStencil);
        }

        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

    GLenum bufs[3] = {GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, bufs);

    ASSERT_GL_NO_ERROR();

    constexpr char kVS[] = R"(#version 300 es
precision highp float;
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, gl_VertexID % 2 == 0 ? -1 : 1, 1);
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;

uniform vec4 value0;
uniform vec4 value2;

layout(location = 0) out vec4 color0;
layout(location = 2) out vec4 color2;

void main()
{
    color0 = value0;
    color2 = value2;
})";

    mProgram = CompileProgram(kVS, kFS);
    ASSERT_NE(0u, mProgram);
    glUseProgram(mProgram);

    const GLint color0Loc = glGetUniformLocation(mProgram, "value0");
    const GLint color1Loc = glGetUniformLocation(mProgram, "value2");

    glUniform4f(color0Loc, 1, 0, 0, 1);
    glUniform4f(color1Loc, 0, 1, 0, 1);
}

void MultisampleResolvePerf::destroyBenchmark()
{
    glDeleteFramebuffers(1, &mMSAAFramebuffer);
    glDeleteFramebuffers(2, mResolveFramebuffer);
    glDeleteFramebuffers(1, &mReferenceFramebuffer);

    glDeleteRenderbuffers(2, mMSAAColor);
    glDeleteRenderbuffers(1, &mMSAADepthStencil);
    glDeleteRenderbuffers(2, mResolveColor);
    glDeleteRenderbuffers(1, &mResolveDepthStencil);

    glDeleteProgram(mProgram);
}

void MultisampleResolvePerf::drawBenchmark()
{
    const MultisampleResolveParams &param = GetParam();
    const int size                        = param.framebufferSize;
    const bool singleSampled              = param.samples == 0;

    glViewport(0, 0, size, size);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x55, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    for (unsigned int iteration = 0; iteration < param.iterationsPerStep; ++iteration)
    {
        const GLenum discards[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT2,
                                   GL_DEPTH_STENCIL_ATTACHMENT};

        glBindFramebuffer(GL_FRAMEBUFFER, singleSampled ? mReferenceFramebuffer : mMSAAFramebuffer);
        glInvalidateFramebuffer(GL_FRAMEBUFFER, param.withDepthStencil ? 3 : 2, discards);

        // Start a render pass, then resolve each attachment + invalidate them.  Every render pass
        // should thus start with LOAD_OP_DONT_CARE and end in STORE_OP_DONT_CARE (for the
        // attachments) and STORE_OP_STORE (for the resolve attachments).
        //
        // In single-sampled mode, just draw.  This is used to compare the performance of
        // multisampled with single sampled rendering.
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        if (!singleSampled)
        {
            for (uint32_t i = 0; i < 2; ++i)
            {
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mResolveFramebuffer[i]);
                glReadBuffer(discards[i]);
                glBlitFramebuffer(0, 0, size, size, 0, 0, size, size, GL_COLOR_BUFFER_BIT,
                                  GL_NEAREST);
                ASSERT_GL_NO_ERROR();

                glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, &discards[i]);
            }

            if (param.withDepthStencil)
            {
                glBlitFramebuffer(0, 0, size, size, 0, 0, size, size,
                                  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

                glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, &discards[2]);
            }

            ASSERT_GL_NO_ERROR();
        }
    }

    ASSERT_GL_NO_ERROR();
}

// Test that multisample resolve + invalidate is as efficient as single sampling on tilers.
TEST_P(MultisampleResolvePerf, Run)
{
    run();
}

MultisampleResolveParams Vulkan(Multisample multisample, WithDepthStencil depthStencil)
{
    MultisampleResolveParams params(multisample, depthStencil);
    params.eglParameters = angle::egl_platform::VULKAN();
    return params;
}
}  // anonymous namespace

ANGLE_INSTANTIATE_TEST(MultisampleResolvePerf,
                       Vulkan(Multisample::No, WithDepthStencil::No),
                       Vulkan(Multisample::Yes, WithDepthStencil::No),
                       Vulkan(Multisample::No, WithDepthStencil::Yes),
                       Vulkan(Multisample::Yes, WithDepthStencil::Yes));

// This test suite is not instantiated on some OSes.
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(MultisampleResolvePerf);
