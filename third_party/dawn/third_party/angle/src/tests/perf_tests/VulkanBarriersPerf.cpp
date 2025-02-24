//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanBarriersPerf:
//   Performance tests for ANGLE's Vulkan backend w.r.t barrier efficiency.
//

#include <sstream>

#include "ANGLEPerfTest.h"
#include "test_utils/gl_raii.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{
constexpr unsigned int kIterationsPerStep = 10;

struct VulkanBarriersPerfParams final : public RenderTestParams
{
    VulkanBarriersPerfParams(bool bufferCopy, bool largeTransfers, bool slowFS)
    {
        iterationsPerStep = kIterationsPerStep;

        // Common default parameters
        eglParameters = egl_platform::VULKAN();
        majorVersion  = 3;
        minorVersion  = 0;
        windowWidth   = 256;
        windowHeight  = 256;
        trackGpuTime  = true;

        doBufferCopy          = bufferCopy;
        doLargeTransfers      = largeTransfers;
        doSlowFragmentShaders = slowFS;
    }

    std::string story() const override;

    // Static parameters
    static constexpr int kImageSizes[3] = {256, 512, 4096};
    static constexpr int kBufferSize    = 4096 * 4096;

    bool doBufferCopy;
    bool doLargeTransfers;
    bool doSlowFragmentShaders;
};

constexpr int VulkanBarriersPerfParams::kImageSizes[];

std::ostream &operator<<(std::ostream &os, const VulkanBarriersPerfParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

class VulkanBarriersPerfBenchmark : public ANGLERenderTest,
                                    public ::testing::WithParamInterface<VulkanBarriersPerfParams>
{
  public:
    VulkanBarriersPerfBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    void createTexture(uint32_t textureIndex, uint32_t sizeIndex, bool compressed);
    void createUniformBuffer();
    void createFramebuffer(uint32_t fboIndex, uint32_t textureIndex, uint32_t sizeIndex);
    void createResources();

    // Handle to the program object
    GLProgram mProgram;

    // Attribute locations
    GLint mPositionLoc;
    GLint mTexCoordLoc;

    // Sampler location
    GLint mSamplerLoc;

    // Texture handles
    GLTexture mTextures[4];

    // Uniform buffer handles
    GLBuffer mUniformBuffers[2];

    // Framebuffer handles
    GLFramebuffer mFbos[2];

    // Buffer handle
    GLBuffer mVertexBuffer;
    GLBuffer mIndexBuffer;

    static constexpr size_t kSmallFboIndex = 0;
    static constexpr size_t kLargeFboIndex = 1;

    static constexpr size_t kUniformBuffer1Index = 0;
    static constexpr size_t kUniformBuffer2Index = 1;

    static constexpr size_t kSmallTextureIndex     = 0;
    static constexpr size_t kLargeTextureIndex     = 1;
    static constexpr size_t kTransferTexture1Index = 2;
    static constexpr size_t kTransferTexture2Index = 3;

    static constexpr size_t kSmallSizeIndex = 0;
    static constexpr size_t kLargeSizeIndex = 1;
    static constexpr size_t kHugeSizeIndex  = 2;
};

std::string VulkanBarriersPerfParams::story() const
{
    std::ostringstream sout;

    sout << RenderTestParams::story();

    if (doBufferCopy)
    {
        sout << "_buffer_copy";
    }
    if (doLargeTransfers)
    {
        sout << "_transfer";
    }
    if (doSlowFragmentShaders)
    {
        sout << "_slowfs";
    }

    return sout.str();
}

VulkanBarriersPerfBenchmark::VulkanBarriersPerfBenchmark()
    : ANGLERenderTest("VulkanBarriersPerf", GetParam()),
      mPositionLoc(-1),
      mTexCoordLoc(-1),
      mSamplerLoc(-1)
{
    if (IsNVIDIA() && IsWindows7())
    {
        skipTest(
            "http://crbug.com/1096510 Fails on Windows7 NVIDIA Vulkan, presumably due to old "
            "drivers");
    }
}

constexpr char kVS[] = R"(attribute vec4 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
void main()
{
    gl_Position = a_position;
    v_texCoord  = a_texCoord;
})";

constexpr char kShortFS[] = R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D s_texture;
void main()
{
    gl_FragColor = texture2D(s_texture, v_texCoord);
})";

constexpr char kSlowFS[] = R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D s_texture;
void main()
{
    vec4 outColor = vec4(0);
    if (v_texCoord.x < 0.2)
    {
        for (int i = 0; i < 100; ++i)
        {
            outColor += texture2D(s_texture, v_texCoord);
        }
    }
    gl_FragColor = outColor;
})";

void VulkanBarriersPerfBenchmark::createTexture(uint32_t textureIndex,
                                                uint32_t sizeIndex,
                                                bool compressed)
{
    const auto &params = GetParam();

    // TODO(syoussefi): compressed copy using vkCmdCopyImage not yet implemented in the vulkan
    // backend. http://anglebug.com/42261682

    glBindTexture(GL_TEXTURE_2D, mTextures[textureIndex]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, params.kImageSizes[sizeIndex],
                 params.kImageSizes[sizeIndex], 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Disable mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void VulkanBarriersPerfBenchmark::createUniformBuffer()
{
    const auto &params = GetParam();

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffers[kUniformBuffer1Index]);
    glBufferData(GL_UNIFORM_BUFFER, params.kBufferSize, nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBuffers[kUniformBuffer2Index]);
    glBufferData(GL_UNIFORM_BUFFER, params.kBufferSize, nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void VulkanBarriersPerfBenchmark::createFramebuffer(uint32_t fboIndex,
                                                    uint32_t textureIndex,
                                                    uint32_t sizeIndex)
{
    createTexture(textureIndex, sizeIndex, false);

    glBindFramebuffer(GL_FRAMEBUFFER, mFbos[fboIndex]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           mTextures[textureIndex], 0);
}

void VulkanBarriersPerfBenchmark::createResources()
{
    const auto &params = GetParam();

    mProgram.makeRaster(kVS, params.doSlowFragmentShaders ? kSlowFS : kShortFS);
    ASSERT_TRUE(mProgram.valid());

    // Get the attribute locations
    mPositionLoc = glGetAttribLocation(mProgram, "a_position");
    mTexCoordLoc = glGetAttribLocation(mProgram, "a_texCoord");

    // Get the sampler location
    mSamplerLoc = glGetUniformLocation(mProgram, "s_texture");

    // Build the vertex buffer
    GLfloat vertices[] = {
        -0.5f, 0.5f,  0.0f,  // Position 0
        0.0f,  0.0f,         // TexCoord 0
        -0.5f, -0.5f, 0.0f,  // Position 1
        0.0f,  1.0f,         // TexCoord 1
        0.5f,  -0.5f, 0.0f,  // Position 2
        1.0f,  1.0f,         // TexCoord 2
        0.5f,  0.5f,  0.0f,  // Position 3
        1.0f,  0.0f          // TexCoord 3
    };

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLushort indices[] = {0, 1, 2, 0, 2, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Create four textures.  Two of them are going to be framebuffers, and two are used for large
    // transfers.
    createFramebuffer(kSmallFboIndex, kSmallTextureIndex, kSmallSizeIndex);
    createFramebuffer(kLargeFboIndex, kLargeTextureIndex, kLargeSizeIndex);
    createUniformBuffer();

    if (params.doLargeTransfers)
    {
        createTexture(kTransferTexture1Index, kHugeSizeIndex, true);
        createTexture(kTransferTexture2Index, kHugeSizeIndex, true);
    }
}

void VulkanBarriersPerfBenchmark::initializeBenchmark()
{
    createResources();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    ASSERT_GL_NO_ERROR();
}

void VulkanBarriersPerfBenchmark::destroyBenchmark() {}

void VulkanBarriersPerfBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    glUseProgram(mProgram);

    // Bind the buffers
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

    // Load the vertex position
    glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    // Load the texture coordinate
    glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(mPositionLoc);
    glEnableVertexAttribArray(mTexCoordLoc);

    // Set the texture sampler to texture unit to 0
    glUniform1i(mSamplerLoc, 0);

    /*
     * The perf benchmark does the following:
     *
     * - Alternately clear and draw from fbo 1 into fbo 2 and back.  This would use the color
     * attachment and shader read-only layouts in the fragment shader and color attachment stages.
     *
     * - Alternately copy data between the 2 uniform buffers. This would use the transfer layouts
     * in the transfer stage.
     *
     * Once compressed texture copies are supported, alternately copy large chunks of data from
     * texture 1 into texture 2 and back.  This would use the transfer layouts in the transfer
     * stage.
     *
     * Once compute shader support is added, another independent set of operations could be a few
     * dispatches.  This would use the general and shader read-only layouts in the compute stage.
     *
     * The idea is to create independent pipelines of operations that would run in parallel on the
     * GPU.  Regressions or inefficiencies in the barrier implementation could result in
     * serialization of these jobs, resulting in a hit in performance.
     *
     * The above operations for example should ideally run on the GPU threads in parallel:
     *
     * + |---draw---||---draw---||---draw---||---draw---||---draw---|
     * + |----buffer copy----||----buffer copy----||----buffer copy----|
     * + |-----------texture copy------------||-----------texture copy------------|
     * + |-----dispatch------||------dispatch------||------dispatch------|
     *
     * If barriers are too restrictive, situations like this could happen (draw is blocking
     * copy):
     *
     * + |---draw---||---draw---||---draw---||---draw---||---draw---|
     * +             |------------copy------------||-----------copy------------|
     *
     * Or like this (copy is blocking draw):
     *
     * + |---draw---|                     |---draw---|                     |---draw---|
     * + |--------------copy-------------||-------------copy--------------|
     *
     * Or like this (draw and copy blocking each other):
     *
     * + |---draw---|                                 |---draw---|
     * +             |------------copy---------------|            |------------copy------------|
     *
     * The idea of doing slow FS calls is to make the second case above slower (by making the draw
     * slower than the transfer):
     *
     * + |------------------draw------------------|                                 |-...draw...-|
     * + |--------------copy----------------|       |-------------copy-------------|
     */

    startGpuTimer();
    for (unsigned int iteration = 0; iteration < params.iterationsPerStep; ++iteration)
    {
        bool altEven = iteration % 2 == 0;

        const int fboDestIndex            = altEven ? kLargeFboIndex : kSmallFboIndex;
        const int fboTexSrcIndex          = altEven ? kSmallTextureIndex : kLargeTextureIndex;
        const int fboDestSizeIndex        = altEven ? kLargeSizeIndex : kSmallSizeIndex;
        const int uniformBufferReadIndex  = altEven ? kUniformBuffer1Index : kUniformBuffer2Index;
        const int uniformBufferWriteIndex = altEven ? kUniformBuffer2Index : kUniformBuffer1Index;

        if (params.doBufferCopy)
        {
            // Transfer data between the 2 Uniform buffers
            glBindBuffer(GL_COPY_READ_BUFFER, mUniformBuffers[uniformBufferReadIndex]);
            glBindBuffer(GL_COPY_WRITE_BUFFER, mUniformBuffers[uniformBufferWriteIndex]);
            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0,
                                params.kBufferSize);
        }

        // Bind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, mFbos[fboDestIndex]);

        // Set the viewport
        glViewport(0, 0, params.kImageSizes[fboDestSizeIndex],
                   params.kImageSizes[fboDestSizeIndex]);

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTextures[fboTexSrcIndex]);

        ASSERT_GL_NO_ERROR();

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }
    stopGpuTimer();

    ASSERT_GL_NO_ERROR();
}

}  // namespace

TEST_P(VulkanBarriersPerfBenchmark, Run)
{
    run();
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(VulkanBarriersPerfBenchmark);
ANGLE_INSTANTIATE_TEST(VulkanBarriersPerfBenchmark,
                       VulkanBarriersPerfParams(false, false, false),
                       VulkanBarriersPerfParams(true, false, false),
                       VulkanBarriersPerfParams(false, true, false),
                       VulkanBarriersPerfParams(false, true, true));
