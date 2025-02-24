//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanPerformanceCounterTest:
//   Validates specific GL call patterns with ANGLE performance counters.
//   For example we can verify a certain call set doesn't break the render pass.

#include "include/platform/Feature.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_instantiate.h"
#include "test_utils/gl_raii.h"
#include "util/random_utils.h"
#include "util/shader_utils.h"
#include "util/test_utils.h"

using namespace angle;

namespace
{
// Normally, if LOAD_OP_NONE is not supported, LOAD_OP_LOAD is used instead.  Similarly, if
// STORE_OP_NONE is not supported, STORE_OP_STORE is used instead.
//
// If an attachment has undefined contents and is unused, LOAD_OP_DONT_CARE and
// STORE_OP_DONT_CARE can be used.  However, these are write operations for synchronization
// purposes, and so ANGLE uses LOAD_OP_NONE and STORE_OP_NONE if available to avoid the
// synchronization.  When NONE is not available, ANGLE foregoes synchronization, producing
// syncval errors.
//
// For the sake of validation, it's unknown if NONE is turned into LOAD/STORE or DONT_CARE.  So
// validation allows variations there.
#define EXPECT_OP_LOAD_AND_NONE(expectedLoads, actualLoads, expectedNones, actualNones) \
    {                                                                                   \
        if (hasLoadOpNoneSupport())                                                     \
        {                                                                               \
            EXPECT_EQ(expectedLoads, actualLoads);                                      \
            EXPECT_EQ(expectedNones, actualNones);                                      \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            EXPECT_EQ(actualNones, 0u);                                                 \
            EXPECT_LE(expectedLoads, actualLoads);                                      \
            EXPECT_GE(expectedLoads + expectedNones, actualLoads);                      \
        }                                                                               \
    }
#define EXPECT_OP_STORE_AND_NONE(expectedStores, actualStores, expectedNones, actualNones) \
    {                                                                                      \
        if (hasStoreOpNoneSupport() && hasLoadOpNoneSupport())                             \
        {                                                                                  \
            EXPECT_EQ(expectedStores, actualStores);                                       \
            EXPECT_EQ(expectedNones, actualNones);                                         \
        }                                                                                  \
        else                                                                               \
        {                                                                                  \
            if (!hasStoreOpNoneSupport())                                                  \
            {                                                                              \
                EXPECT_EQ(actualNones, 0u);                                                \
            }                                                                              \
            EXPECT_LE(expectedStores, actualStores);                                       \
            EXPECT_GE(expectedStores + expectedNones, actualStores + actualNones);         \
        }                                                                                  \
    }

#define EXPECT_DEPTH_OP_COUNTERS(counters, expected)                                       \
    {                                                                                      \
        EXPECT_EQ(expected.depthLoadOpClears, counters.depthLoadOpClears);                 \
        EXPECT_OP_LOAD_AND_NONE(expected.depthLoadOpLoads, counters.depthLoadOpLoads,      \
                                expected.depthLoadOpNones, counters.depthLoadOpNones);     \
        EXPECT_OP_STORE_AND_NONE(expected.depthStoreOpStores, counters.depthStoreOpStores, \
                                 expected.depthStoreOpNones, counters.depthStoreOpNones);  \
    }

#define EXPECT_STENCIL_OP_COUNTERS(counters, expected)                                         \
    {                                                                                          \
        EXPECT_EQ(expected.stencilLoadOpClears, counters.stencilLoadOpClears);                 \
        EXPECT_OP_LOAD_AND_NONE(expected.stencilLoadOpLoads, counters.stencilLoadOpLoads,      \
                                expected.stencilLoadOpNones, counters.stencilLoadOpNones);     \
        EXPECT_OP_STORE_AND_NONE(expected.stencilStoreOpStores, counters.stencilStoreOpStores, \
                                 expected.stencilStoreOpNones, counters.stencilStoreOpNones);  \
    }

#define EXPECT_DEPTH_STENCIL_OP_COUNTERS(counters, expected) \
    {                                                        \
        EXPECT_DEPTH_OP_COUNTERS(counters, expected);        \
        EXPECT_STENCIL_OP_COUNTERS(counters, expected);      \
    }

#define EXPECT_COLOR_OP_COUNTERS(counters, expected)                                       \
    {                                                                                      \
        EXPECT_EQ(expected.colorLoadOpClears, counters.colorLoadOpClears);                 \
        EXPECT_OP_LOAD_AND_NONE(expected.colorLoadOpLoads, counters.colorLoadOpLoads,      \
                                expected.colorLoadOpNones, counters.colorLoadOpNones);     \
        EXPECT_OP_STORE_AND_NONE(expected.colorStoreOpStores, counters.colorStoreOpStores, \
                                 expected.colorStoreOpNones, counters.colorStoreOpNones);  \
    }

#define EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(counters, expected)            \
    {                                                                        \
        EXPECT_EQ(expected.depthLoadOpLoads, counters.depthLoadOpLoads);     \
        EXPECT_EQ(expected.stencilLoadOpLoads, counters.stencilLoadOpLoads); \
    }

#define EXPECT_COUNTERS_FOR_UNRESOLVE_RESOLVE_TEST(counters, expected)                         \
    {                                                                                          \
        EXPECT_EQ(expected.colorAttachmentUnresolves, counters.colorAttachmentUnresolves);     \
        EXPECT_EQ(expected.depthAttachmentUnresolves, counters.depthAttachmentUnresolves);     \
        EXPECT_EQ(expected.stencilAttachmentUnresolves, counters.stencilAttachmentUnresolves); \
        EXPECT_EQ(expected.colorAttachmentResolves, counters.colorAttachmentResolves);         \
        EXPECT_EQ(expected.depthAttachmentResolves, counters.depthAttachmentResolves);         \
        EXPECT_EQ(expected.stencilAttachmentResolves, counters.stencilAttachmentResolves);     \
    }

#define EXPECT_CLEAR_ATTACHMENTS_COUNTER(expected, actual) \
    {                                                      \
        if (hasPreferDrawOverClearAttachments())           \
        {                                                  \
            EXPECT_EQ(actual, 0u);                         \
        }                                                  \
        else                                               \
        {                                                  \
            EXPECT_EQ(actual, expected);                   \
        }                                                  \
    }

enum class BufferUpdate
{
    SubData,  // use glBufferSubData
    Copy,     // use glCopyBufferSubData
};

class VulkanPerformanceCounterTest : public ANGLETest<>
{
  protected:
    VulkanPerformanceCounterTest()
    {
        // Depth/Stencil required for SwapShouldInvalidate*.
        // Also RGBA8 is required to avoid the clear for emulated alpha.
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    static constexpr GLsizei kOpsTestSize = 16;

    void testSetUp() override
    {
        glGenPerfMonitorsAMD(1, &monitor);
        glBeginPerfMonitorAMD(monitor);
    }

    void testTearDown() override
    {
        glEndPerfMonitorAMD(monitor);
        glDeletePerfMonitorsAMD(1, &monitor);
    }

    void setupForColorOpsTest(GLFramebuffer *framebuffer, GLTexture *texture)
    {
        // Setup the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kOpsTestSize, kOpsTestSize, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    }

    void setupForColorDepthOpsTest(GLFramebuffer *framebuffer,
                                   GLTexture *texture,
                                   GLRenderbuffer *renderbuffer)
    {
        // Setup color and depth
        glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kOpsTestSize, kOpsTestSize, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, *renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, kOpsTestSize, kOpsTestSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                  *renderbuffer);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Setup depth parameters
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_GEQUAL);
        glClearDepthf(0.99f);
        glViewport(0, 0, kOpsTestSize, kOpsTestSize);
        ASSERT_GL_NO_ERROR();
    }

    void setupForDepthStencilOpsTest(GLFramebuffer *framebuffer,
                                     GLTexture *texture,
                                     GLRenderbuffer *renderbuffer)
    {
        // Setup color, depth, and stencil
        glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kOpsTestSize, kOpsTestSize, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, *renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kOpsTestSize, kOpsTestSize);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  *renderbuffer);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Setup depth parameters
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_GEQUAL);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilMask(0xFF);
        glClearDepthf(0.99f);
        glClearStencil(0xAA);
        glViewport(0, 0, kOpsTestSize, kOpsTestSize);
        ASSERT_GL_NO_ERROR();
    }

    void setupClearAndDrawForDepthStencilOpsTest(GLProgram *program,
                                                 GLFramebuffer *framebuffer,
                                                 GLTexture *texture,
                                                 GLRenderbuffer *renderbuffer,
                                                 bool clearStencil)
    {
        setupForDepthStencilOpsTest(framebuffer, texture, renderbuffer);

        // Clear and draw with depth and stencil buffer enabled
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                (clearStencil ? GL_STENCIL_BUFFER_BIT : 0));
        drawQuad(*program, essl1_shaders::PositionAttrib(), 1.f);
        ASSERT_GL_NO_ERROR();
    }

    void setExpectedCountersForDepthOps(const angle::VulkanPerfCounters &counters,
                                        uint64_t incrementalRenderPasses,
                                        uint64_t incrementalDepthLoadOpClears,
                                        uint64_t incrementalDepthLoadOpLoads,
                                        uint64_t incrementalDepthLoadOpNones,
                                        uint64_t incrementalDepthStoreOpStores,
                                        uint64_t incrementalDepthStoreOpNones,
                                        angle::VulkanPerfCounters *expected)
    {
        expected->renderPasses       = counters.renderPasses + incrementalRenderPasses;
        expected->depthLoadOpClears  = counters.depthLoadOpClears + incrementalDepthLoadOpClears;
        expected->depthLoadOpLoads   = counters.depthLoadOpLoads + incrementalDepthLoadOpLoads;
        expected->depthLoadOpNones   = counters.depthLoadOpNones + incrementalDepthLoadOpNones;
        expected->depthStoreOpStores = counters.depthStoreOpStores + incrementalDepthStoreOpStores;
        expected->depthStoreOpNones  = counters.depthStoreOpNones + incrementalDepthStoreOpNones;
    }

    void setExpectedCountersForStencilOps(const angle::VulkanPerfCounters &counters,
                                          uint64_t incrementalStencilLoadOpClears,
                                          uint64_t incrementalStencilLoadOpLoads,
                                          uint64_t incrementalStencilLoadOpNones,
                                          uint64_t incrementalStencilStoreOpStores,
                                          uint64_t incrementalStencilStoreOpNones,
                                          angle::VulkanPerfCounters *expected)
    {
        expected->stencilLoadOpClears =
            counters.stencilLoadOpClears + incrementalStencilLoadOpClears;
        expected->stencilLoadOpLoads = counters.stencilLoadOpLoads + incrementalStencilLoadOpLoads;
        expected->stencilLoadOpNones = counters.stencilLoadOpNones + incrementalStencilLoadOpNones;
        expected->stencilStoreOpStores =
            counters.stencilStoreOpStores + incrementalStencilStoreOpStores;
        expected->stencilStoreOpNones =
            counters.stencilStoreOpNones + incrementalStencilStoreOpNones;
    }

    void setExpectedCountersForColorOps(const angle::VulkanPerfCounters &counters,
                                        uint64_t incrementalRenderPasses,
                                        uint64_t incrementalColorLoadOpClears,
                                        uint64_t incrementalColorLoadOpLoads,
                                        uint64_t incrementalColorLoadOpNones,
                                        uint64_t incrementalColorStoreOpStores,
                                        uint64_t incrementalColorStoreOpNones,
                                        angle::VulkanPerfCounters *expected)
    {
        expected->renderPasses       = counters.renderPasses + incrementalRenderPasses;
        expected->colorLoadOpClears  = counters.colorLoadOpClears + incrementalColorLoadOpClears;
        expected->colorLoadOpLoads   = counters.colorLoadOpLoads + incrementalColorLoadOpLoads;
        expected->colorLoadOpNones   = counters.colorLoadOpNones + incrementalColorLoadOpNones;
        expected->colorStoreOpStores = counters.colorStoreOpStores + incrementalColorStoreOpStores;
        expected->colorStoreOpNones  = counters.colorStoreOpNones + incrementalColorStoreOpNones;
    }

    void setAndIncrementDepthStencilLoadCountersForOpsTest(
        const angle::VulkanPerfCounters &counters,
        uint64_t incrementalDepthLoadOpLoads,
        uint64_t incrementalStencilLoadOpLoads,
        angle::VulkanPerfCounters *expected)
    {
        expected->depthLoadOpLoads   = counters.depthLoadOpLoads + incrementalDepthLoadOpLoads;
        expected->stencilLoadOpLoads = counters.stencilLoadOpLoads + incrementalStencilLoadOpLoads;
    }

    void setExpectedCountersForUnresolveResolveTest(const angle::VulkanPerfCounters &counters,
                                                    uint64_t incrementalColorAttachmentUnresolves,
                                                    uint64_t incrementalDepthAttachmentUnresolves,
                                                    uint64_t incrementalStencilAttachmentUnresolves,
                                                    uint64_t incrementalColorAttachmentResolves,
                                                    uint64_t incrementalDepthAttachmentResolves,
                                                    uint64_t incrementalStencilAttachmentResolves,
                                                    angle::VulkanPerfCounters *expected)
    {
        expected->colorAttachmentUnresolves =
            counters.colorAttachmentUnresolves + incrementalColorAttachmentUnresolves;
        expected->depthAttachmentUnresolves =
            counters.depthAttachmentUnresolves + incrementalDepthAttachmentUnresolves;
        expected->stencilAttachmentUnresolves =
            counters.stencilAttachmentUnresolves + incrementalStencilAttachmentUnresolves;
        expected->colorAttachmentResolves =
            counters.colorAttachmentResolves + incrementalColorAttachmentResolves;
        expected->depthAttachmentResolves =
            counters.depthAttachmentResolves + incrementalDepthAttachmentResolves;
        expected->stencilAttachmentResolves =
            counters.stencilAttachmentResolves + incrementalStencilAttachmentResolves;
    }

    void maskedFramebufferFetchDraw(const GLColor &clearColor, GLBuffer &buffer);
    void maskedFramebufferFetchDrawVerify(const GLColor &expectedColor, GLBuffer &buffer);

    void saveAndReloadBinary(GLProgram *original, GLProgram *reloaded);
    void testPipelineCacheIsWarm(GLProgram *program, GLColor color);

    void updateBuffer(BufferUpdate update,
                      GLenum target,
                      GLintptr offset,
                      GLsizeiptr size,
                      const void *data)
    {
        if (update == BufferUpdate::SubData)
        {
            // If using glBufferSubData, directly upload data on the specified target (where the
            // buffer is already bound)
            glBufferSubData(target, offset, size, data);
        }
        else
        {
            // Otherwise copy through a temp buffer.  Use a non-zero offset for more coverage.
            constexpr GLintptr kStagingOffset = 123;
            GLBuffer staging;
            glBindBuffer(GL_COPY_READ_BUFFER, staging);
            glBufferData(GL_COPY_READ_BUFFER, offset + size + kStagingOffset * 2, nullptr,
                         GL_STATIC_DRAW);
            glBufferSubData(GL_COPY_READ_BUFFER, kStagingOffset, size, data);
            glCopyBufferSubData(GL_COPY_READ_BUFFER, target, kStagingOffset, offset, size);
        }
    }

    void mappingGpuReadOnlyBufferGhostsBuffer(BufferUpdate update);
    void partialBufferUpdateShouldNotBreakRenderPass(BufferUpdate update);
    void bufferSubDataShouldNotTriggerSyncState(BufferUpdate update);

    angle::VulkanPerfCounters getPerfCounters()
    {
        if (mIndexMap.empty())
        {
            mIndexMap = BuildCounterNameToIndexMap();
        }

        return GetPerfCounters(mIndexMap);
    }

    // Support status for ANGLE features.
    bool isFeatureEnabled(Feature feature) const
    {
        return getEGLWindow()->isFeatureEnabled(feature);
    }
    bool hasLoadOpNoneSupport() const
    {
        return isFeatureEnabled(Feature::SupportsRenderPassLoadStoreOpNone);
    }
    bool hasStoreOpNoneSupport() const
    {
        return isFeatureEnabled(Feature::SupportsRenderPassLoadStoreOpNone) ||
               isFeatureEnabled(Feature::SupportsRenderPassStoreOpNone);
    }
    bool hasMutableMipmapTextureUpload() const
    {
        return isFeatureEnabled(Feature::MutableMipmapTextureUpload);
    }
    bool hasPreferDrawOverClearAttachments() const
    {
        return isFeatureEnabled(Feature::PreferDrawClearOverVkCmdClearAttachments);
    }
    bool hasSupportsPipelineCreationFeedback() const
    {
        return isFeatureEnabled(Feature::SupportsPipelineCreationFeedback);
    }
    bool hasWarmUpPipelineCacheAtLink() const
    {
        return isFeatureEnabled(Feature::WarmUpPipelineCacheAtLink);
    }
    bool hasEffectivePipelineCacheSerialization() const
    {
        return isFeatureEnabled(Feature::HasEffectivePipelineCacheSerialization);
    }
    bool hasPreferCPUForBufferSubData() const
    {
        return isFeatureEnabled(Feature::PreferCPUForBufferSubData);
    }
    bool hasPreferSubmitAtFBOBoundary() const
    {
        return isFeatureEnabled(Feature::PreferSubmitAtFBOBoundary);
    }
    bool hasDisallowMixedDepthStencilLoadOpNoneAndLoad() const
    {
        return isFeatureEnabled(Feature::DisallowMixedDepthStencilLoadOpNoneAndLoad);
    }
    bool hasSupportsImagelessFramebuffer() const
    {
        return isFeatureEnabled(Feature::SupportsImagelessFramebuffer);
    }
    bool hasSupportsHostImageCopy() const
    {
        return isFeatureEnabled(Feature::SupportsHostImageCopy);
    }
    bool hasDepthStencilResolveThroughAttachment() const
    {
        return isFeatureEnabled(Feature::SupportsDepthStencilResolve) &&
               !isFeatureEnabled(Feature::DisableDepthStencilResolveThroughAttachment);
    }

    GLuint monitor;
    CounterNameToIndexMap mIndexMap;
};

class VulkanPerformanceCounterTest_ES31 : public VulkanPerformanceCounterTest
{};

class VulkanPerformanceCounterTest_MSAA : public VulkanPerformanceCounterTest
{
  protected:
    VulkanPerformanceCounterTest_MSAA() : VulkanPerformanceCounterTest()
    {
        // Make sure the window is non-square to correctly test prerotation
        setWindowWidth(32);
        setWindowHeight(64);
        setSamples(4);
        setMultisampleEnabled(true);
    }
};

class VulkanPerformanceCounterTest_SingleBuffer : public VulkanPerformanceCounterTest
{
  protected:
    VulkanPerformanceCounterTest_SingleBuffer() : VulkanPerformanceCounterTest()
    {
        setMutableRenderBuffer(true);
    }
};

class VulkanPerformanceCounterTest_Prerotation : public VulkanPerformanceCounterTest
{
  protected:
    VulkanPerformanceCounterTest_Prerotation() : VulkanPerformanceCounterTest()
    {
        // Make sure the window is non-square to correctly test prerotation
        setWindowWidth(32);
        setWindowHeight(64);
    }
};

void VulkanPerformanceCounterTest::maskedFramebufferFetchDraw(const GLColor &clearColor,
                                                              GLBuffer &buffer)
{
    // Initialize the color buffer
    angle::Vector4 clearAsVec4 = clearColor.toNormalizedVector();
    glClearColor(clearAsVec4[0], clearAsVec4[1], clearAsVec4[2], clearAsVec4[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, clearColor, 1);

    // Create output buffer
    constexpr GLsizei kBufferSize = kOpsTestSize * kOpsTestSize * sizeof(float[4]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, kBufferSize, nullptr, GL_STATIC_DRAW);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffer, 0, kBufferSize);

    // Zero-initialize it
    void *bufferData = glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, kBufferSize,
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    memset(bufferData, 0, kBufferSize);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Mask color output
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    static constexpr char kVS[] = R"(#version 310 es
in highp vec4 a_position;

void main (void)
{
    gl_Position = a_position;
})";

    static constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require
layout(noncoherent, location = 0) inout highp vec4 o_color;

layout(std140, binding = 0) buffer outBlock {
    highp vec4 data[256];
};

uniform highp vec4 u_color;
void main (void)
{
    uint index = uint(gl_FragCoord.y) * 16u + uint(gl_FragCoord.x);
    data[index] = o_color;
    o_color += u_color;
})";

    // Draw
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
}

void VulkanPerformanceCounterTest::maskedFramebufferFetchDrawVerify(const GLColor &expectedColor,
                                                                    GLBuffer &buffer)
{
    angle::Vector4 expectedAsVec4 = expectedColor.toNormalizedVector();

    // Read back the storage buffer and make sure framebuffer fetch worked as intended despite
    // masked color.
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    constexpr GLsizei kBufferSize = kOpsTestSize * kOpsTestSize * sizeof(float[4]);
    const float *colorData        = static_cast<const float *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, kBufferSize, GL_MAP_READ_BIT));
    for (uint32_t y = 0; y < kOpsTestSize; ++y)
    {
        for (uint32_t x = 0; x < kOpsTestSize; ++x)
        {
            uint32_t ssboIndex = (y * kOpsTestSize + x) * 4;
            EXPECT_NEAR(colorData[ssboIndex + 0], expectedAsVec4[0], 0.05);
            EXPECT_NEAR(colorData[ssboIndex + 1], expectedAsVec4[1], 0.05);
            EXPECT_NEAR(colorData[ssboIndex + 2], expectedAsVec4[2], 0.05);
            EXPECT_NEAR(colorData[ssboIndex + 3], expectedAsVec4[3], 0.05);
        }
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Tests that texture updates to unused textures don't break the RP.
TEST_P(VulkanPerformanceCounterTest, NewTextureDoesNotBreakRenderPass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    GLColor kInitialData[4] = {GLColor::red, GLColor::blue, GLColor::green, GLColor::yellow};

    // Step 1: Set up a simple 2D Texture rendering loop.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, kInitialData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    auto quadVerts = GetQuadVertices();

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(quadVerts[0]), quadVerts.data(),
                 GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, posLoc);

    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses;

    // Step 2: Introduce a new 2D Texture with the same Program and Framebuffer.
    GLTexture newTexture;
    glBindTexture(GL_TEXTURE_2D, newTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, kInitialData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Tests that each update for a large cube map face results in outside command buffer submission.
TEST_P(VulkanPerformanceCounterTest, LargeCubeMapUpdatesSubmitsOutsideCommandBuffer)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    constexpr size_t kMaxBufferToImageCopySize = 64 * 1024 * 1024;
    constexpr uint64_t kNumSubmits             = 6;
    uint64_t expectedSubmitCommandsCount = getPerfCounters().vkQueueSubmitCallsTotal + kNumSubmits;

    // Set up a simple large cubemap texture so that each face update, when flushed, can result in a
    // separate submission.
    GLTexture textureCube;
    constexpr GLsizei kTexDim         = 4096;
    constexpr uint32_t kPixelSizeRGBA = 4;
    static_assert(kTexDim * kTexDim * kPixelSizeRGBA == kMaxBufferToImageCopySize);

    std::vector<GLColor> kInitialData(kTexDim * kTexDim, GLColor::magenta);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureCube);

    for (size_t i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, kTexDim, kTexDim, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, kInitialData.data());
    }

    // Flush the previous texture.
    GLTexture lastTexture;
    glBindTexture(GL_TEXTURE_2D, lastTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kInitialData.data());

    // Verify number of submissions.
    EXPECT_EQ(getPerfCounters().vkQueueSubmitCallsTotal, expectedSubmitCommandsCount);
}

// Tests that submitting the outside command buffer due to texture upload size does not break the
// current render pass.
TEST_P(VulkanPerformanceCounterTest, SubmittingOutsideCommandBufferDoesNotBreakRenderPass)
{
    constexpr size_t kMaxBufferToImageCopySize = 64 * 1024 * 1024;
    constexpr uint64_t kNumSubmits             = 2;
    uint64_t expectedRenderPassCount           = getPerfCounters().renderPasses + 1;
    uint64_t expectedSubmitCommandsCount = getPerfCounters().vkQueueSubmitCallsTotal + kNumSubmits;

    // Step 1: Set up a simple 2D texture.
    GLTexture texture;
    constexpr GLsizei kTexDim         = 256;
    constexpr uint32_t kPixelSizeRGBA = 4;
    constexpr uint32_t kTextureSize   = kTexDim * kTexDim * kPixelSizeRGBA;
    std::vector<GLColor> kInitialData(kTexDim * kTexDim, GLColor::green);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexDim, kTexDim, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kInitialData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    auto quadVerts = GetQuadVertices();

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(quadVerts[0]), quadVerts.data(),
                 GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, posLoc);

    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Step 2: Load a new 2D Texture multiple times with the same Program and Framebuffer. The total
    // size of the loaded textures must exceed the threshold to submit the outside command buffer.
    constexpr size_t kMaxLoadCount = kMaxBufferToImageCopySize / kTextureSize * kNumSubmits + 1;
    for (size_t loadCount = 0; loadCount < kMaxLoadCount; loadCount++)
    {
        GLTexture newTexture;
        glBindTexture(GL_TEXTURE_2D, newTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexDim, kTexDim, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     kInitialData.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();
    }

    // Verify render pass and submitted frame counts.
    EXPECT_EQ(getPerfCounters().renderPasses, expectedRenderPassCount);
    EXPECT_EQ(getPerfCounters().vkQueueSubmitCallsTotal, expectedSubmitCommandsCount);
}

// Tests that submitting the outside command buffer due to texture upload size does not result in
// garbage collection of render pass resources..
TEST_P(VulkanPerformanceCounterTest, SubmittingOutsideCommandBufferDoesNotCollectRenderPassGarbage)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_disjoint_timer_query"));

    // If VK_EXT_host_image_copy is used, uploads will all be done on the CPU and there would be no
    // submissions.
    ANGLE_SKIP_TEST_IF(hasSupportsHostImageCopy());

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;
    uint64_t submitCommandsCount     = getPerfCounters().vkQueueSubmitCallsTotal;

    // Set up a simple 2D texture.
    GLTexture texture;
    constexpr GLsizei kTexDim = 256;
    std::vector<GLColor> kInitialData(kTexDim * kTexDim, GLColor::green);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexDim, kTexDim, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kInitialData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    auto quadVerts = GetQuadVertices();

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(quadVerts[0]), quadVerts.data(),
                 GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, posLoc);

    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);
    ASSERT_GL_NO_ERROR();

    // Issue a timestamp query, just for the sake of using it as a means of knowing when a
    // submission is finished.  In the Vulkan backend, querying the status of the query results in a
    // check of completed submissions, at which point associated garbage is also destroyed.
    GLQuery query;
    glQueryCounterEXT(query, GL_TIMESTAMP_EXT);

    // Issue a draw call, and delete the program
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    program.reset();

    ANGLE_GL_PROGRAM(program2, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program2);
    ASSERT_EQ(posLoc, glGetAttribLocation(program2, essl1_shaders::PositionAttrib()));

    // Issue uploads until there's an implicit submission
    size_t textureCount = 0;
    while (getPerfCounters().vkQueueSubmitCallsTotal == submitCommandsCount)
    {
        GLTexture newTexture;
        glBindTexture(GL_TEXTURE_2D, newTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexDim, kTexDim, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     kInitialData.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();
        textureCount++;
    }
    // 256x256 texture upload should not trigger a submission
    ASSERT(textureCount > 1);

    ++submitCommandsCount;
    EXPECT_EQ(getPerfCounters().vkQueueSubmitCallsTotal, submitCommandsCount);

    // Busy wait until the query results are available.
    GLuint ready = GL_FALSE;
    while (ready == GL_FALSE)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
    }

    // At this point, the render pass should still not be submitted, and the pipeline that is
    // deleted should still not be garbage collected.  Submit the commands and ensure there is no
    // crash.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ++submitCommandsCount;

    // Verify counters.
    EXPECT_EQ(getPerfCounters().renderPasses, expectedRenderPassCount);
    EXPECT_EQ(getPerfCounters().vkQueueSubmitCallsTotal, submitCommandsCount);
}

// Tests that submitting the outside command buffer due to texture upload size and triggers
// endRenderPass works correctly.
TEST_P(VulkanPerformanceCounterTest, SubmittingOutsideCommandBufferTriggersEndRenderPass)
{
    // If VK_EXT_host_image_copy is used, uploads will all be done on the CPU and there would be no
    // submissions.
    ANGLE_SKIP_TEST_IF(hasSupportsHostImageCopy());

    const int width                  = getWindowWidth();
    const int height                 = getWindowHeight();
    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;
    uint64_t submitCommandsCount     = getPerfCounters().vkQueueSubmitCallsTotal;

    // Start a new renderpass with red quad on left.
    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glScissor(0, 0, width / 2, height);
    glEnable(GL_SCISSOR_TEST);
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);

    // Issue texture uploads and draw green quad on right until endRenderPass is triggered
    glScissor(width / 2, 0, width / 2, height);
    ANGLE_GL_PROGRAM(textureProgram, essl1_shaders::vs::Texture2D(),
                     essl1_shaders::fs::Texture2D());
    glUseProgram(textureProgram);
    GLint textureLoc = glGetUniformLocation(textureProgram, essl1_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLoc);
    glUniform1i(textureLoc, 0);

    // This test is specifically try to test outsideRPCommands submission drain the reserved
    // queueSerials. Right now we are reserving 15 queue Serials. This is to ensure if the
    // implementation changes, we do not end up with infinite loop here.
    constexpr GLsizei kMaxOutsideRPCommandsSubmitCount = 17;
    constexpr GLsizei kTexDim                          = 1024;
    std::vector<GLColor> kInitialData(kTexDim * kTexDim, GLColor::green);
    // Put a limit on the loop to avoid infinite loop in bad case.
    while (getPerfCounters().renderPasses == expectedRenderPassCount &&
           getPerfCounters().vkQueueSubmitCallsTotal <
               submitCommandsCount + kMaxOutsideRPCommandsSubmitCount)
    {
        GLTexture newTexture;
        glBindTexture(GL_TEXTURE_2D, newTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kTexDim, kTexDim);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kTexDim, kTexDim, GL_RGBA, GL_UNSIGNED_BYTE,
                        kInitialData.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        drawQuad(textureProgram, essl1_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();
    }

    ++expectedRenderPassCount;
    EXPECT_EQ(getPerfCounters().renderPasses, expectedRenderPassCount);

    // Verify renderpass draw quads correctly
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(width / 2 + 1, 0, GLColor::green);
}

// Tests that a color 2D image is cleared via vkCmdClearColorImage.
TEST_P(VulkanPerformanceCounterTest, ClearTextureEXTFullColorImageClear2D)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));
    uint32_t expectedFullImageClearsAfterFullClear1 = getPerfCounters().fullImageClears + 1;
    uint32_t expectedFullImageClearsAfterFullClear2 = getPerfCounters().fullImageClears + 2;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // The following is the first full clear.
    glClearTexImageEXT(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::red);
    EXPECT_EQ(getPerfCounters().fullImageClears, expectedFullImageClearsAfterFullClear1);

    // The following are several partial clears.
    glClearTexSubImageEXT(tex, 0, 0, 0, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glClearTexSubImageEXT(tex, 0, 8, 0, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::blue);
    glClearTexSubImageEXT(tex, 0, 0, 8, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::cyan);
    glClearTexSubImageEXT(tex, 0, 8, 8, 0, 8, 8, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::yellow);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, 8, 8, GLColor::transparentBlack);
    EXPECT_EQ(getPerfCounters().fullImageClears, expectedFullImageClearsAfterFullClear1);

    // The following is the second full clear (extents of a full clear).
    glClearTexSubImageEXT(tex, 0, 0, 0, 0, 16, 16, 1, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::magenta);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_RECT_EQ(0, 0, 16, 16, GLColor::magenta);
    EXPECT_EQ(getPerfCounters().fullImageClears, expectedFullImageClearsAfterFullClear2);
}

// Tests that a color 3D image is cleared via vkCmdClearColorImage.
TEST_P(VulkanPerformanceCounterTest, ClearTextureEXTFullColorImageClear3D)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clear_texture"));
    uint32_t expectedFullImageClears = getPerfCounters().fullImageClears + 1;

    constexpr uint32_t kWidth  = 4;
    constexpr uint32_t kHeight = 4;
    constexpr uint32_t kDepth  = 4;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, kWidth, kHeight, kDepth);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

    glClearTexImageEXT(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::white);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::white);
    EXPECT_EQ(getPerfCounters().fullImageClears, expectedFullImageClears);

    glClearTexSubImageEXT(texture, 0, 0, 0, 0, kWidth, kHeight, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                          &GLColor::green);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, 0);
    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::green);

    EXPECT_EQ(getPerfCounters().fullImageClears, expectedFullImageClears);
}

// Tests that mutable texture is uploaded with appropriate mip level attributes.
TEST_P(VulkanPerformanceCounterTest, MutableTextureCompatibleMipLevelsInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded + 1;

    std::vector<GLColor> mip0Color(4 * 4, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 2, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip0Color.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip1Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that a single-level mutable texture is uploaded with appropriate attributes.
TEST_P(VulkanPerformanceCounterTest, MutableTextureCompatibleSingleMipLevelInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded + 1;

    std::vector<GLColor> mip0Color(4 * 4, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip0Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable texture is uploaded with appropriate mip level attributes, even with unequal
// dimensions.
TEST_P(VulkanPerformanceCounterTest, MutableTextureCompatibleMipLevelsNonSquareInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded + 1;

    std::vector<GLColor> mip0Color(4 * 2, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 1, GLColor::red);
    std::vector<GLColor> mip2Color(1 * 1, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip0Color.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 2, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip1Color.data());
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip2Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that a mutable texture is not uploaded if there are no data or updates for it.
TEST_P(VulkanPerformanceCounterTest, MutableTextureSingleLevelWithNoDataNoInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded;

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable texture is not uploaded with more than one updates in a single mip level.
TEST_P(VulkanPerformanceCounterTest, MutableTextureSingleLevelWithMultipleUpdatesNoInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded;

    std::vector<GLColor> mip0Color(4 * 4, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, mip0Color.data());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 2, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, mip0Color.data());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 2, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, mip0Color.data());
    glTexSubImage2D(GL_TEXTURE_2D, 0, 2, 2, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, mip0Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that the optimization is not triggered when a mutable texture becomes immutable, e.g.,
// after glTexStorage2D.
TEST_P(VulkanPerformanceCounterTest, MutableTextureChangeToImmutableNoInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded;

    std::vector<GLColor> mip0Color(4 * 4, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 2, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip0Color.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip1Color.data());
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_RGBA8, 4, 4);
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable texture is not uploaded when there is no base mip level (0).
TEST_P(VulkanPerformanceCounterTest, MutableTextureNoBaseLevelNoInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded;

    std::vector<GLColor> mip1Color(4 * 4, GLColor::red);
    std::vector<GLColor> mip2Color(2 * 2, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip1Color.data());
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip2Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable texture is uploaded even when there is a missing mip level greater than 1
// despite the defined mip levels being compatible.
TEST_P(VulkanPerformanceCounterTest, MutableTextureMissingMipLevelGreaterThanOneInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded + 1;

    std::vector<GLColor> mip0Color(8 * 8, GLColor::red);
    std::vector<GLColor> mip1Color(4 * 4, GLColor::red);
    std::vector<GLColor> mip3Color(1 * 1, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip0Color.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip1Color.data());
    glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip3Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable texture is not uploaded with incompatible mip level sizes.
TEST_P(VulkanPerformanceCounterTest, MutableTextureMipLevelsWithIncompatibleSizesNoInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded;

    std::vector<GLColor> mip0Color(8 * 8, GLColor::red);
    std::vector<GLColor> mip1Color(4 * 4, GLColor::red);
    std::vector<GLColor> mip2Color(3 * 3, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip0Color.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip1Color.data());
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 3, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip2Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable texture is not uploaded with incompatible mip level formats.
TEST_P(VulkanPerformanceCounterTest, MutableTextureMipLevelsWithIncompatibleFormatsNoInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded;

    std::vector<GLColor> mip0Color(4 * 4, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 2, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, mip0Color.data());
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, mip1Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable 3D texture is uploaded with appropriate mip level attributes.
TEST_P(VulkanPerformanceCounterTest, MutableTexture3DCompatibleMipLevelsInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded + 1;

    std::vector<GLColor> mip0Color(4 * 4 * 4, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 2 * 2, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_3D, texture1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 4, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip0Color.data());
    glTexImage3D(GL_TEXTURE_3D, 1, GL_RGBA, 2, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip1Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_3D, texture2);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable 3D texture is uploaded with appropriate mip level attributes, even with
// unequal dimensions.
TEST_P(VulkanPerformanceCounterTest, MutableTexture3DCompatibleMipLevelsNonCubeInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded + 1;

    std::vector<GLColor> mip0Color(4 * 2 * 2, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 1 * 1, GLColor::red);
    std::vector<GLColor> mip2Color(1 * 1 * 1, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_3D, texture1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 4, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip0Color.data());
    glTexImage3D(GL_TEXTURE_3D, 1, GL_RGBA, 2, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip1Color.data());
    glTexImage3D(GL_TEXTURE_3D, 2, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip1Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_3D, texture2);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable 3D texture is not uploaded with incompatible mip level sizes.
TEST_P(VulkanPerformanceCounterTest, MutableTexture3DMipLevelsWithIncompatibleSizesNoInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded;

    std::vector<GLColor> mip0Color(4 * 4 * 4, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 2 * 3, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_3D, texture1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 4, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip0Color.data());
    glTexImage3D(GL_TEXTURE_3D, 1, GL_RGBA, 2, 2, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip1Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_3D, texture2);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable 2D array texture is not uploaded with incompatible mip level sizes.
TEST_P(VulkanPerformanceCounterTest, MutableTexture2DArrayMipLevelsWithIncompatibleSizesNoInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded;

    std::vector<GLColor> mip0Color(4 * 4 * 4, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 2 * 3, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture1);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 4, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip0Color.data());
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA, 2, 2, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip1Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_3D, texture2);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable cubemap texture is uploaded with appropriate mip level attributes.
TEST_P(VulkanPerformanceCounterTest, MutableTextureCubemapCompatibleMipLevelsInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded + 1;

    std::vector<GLColor> mip0Color(4 * 4, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 2, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture1);
    for (size_t i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, 4, 4, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, mip0Color.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 1, GL_RGBA, 2, 2, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, mip1Color.data());
    }
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable cubemap texture is not uploaded with no data or updates.
TEST_P(VulkanPerformanceCounterTest, MutableTextureCubemapWithNoDataNoInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded;

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture1);
    for (size_t i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, 4, 4, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 1, GL_RGBA, 2, 2, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
    }
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable cubemap texture is not uploaded with more than one update in a cube face.
TEST_P(VulkanPerformanceCounterTest, MutableTextureCubemapMultipleUpdatesForOneFaceNoInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded;

    std::vector<GLColor> mip0Color(4 * 4, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 2, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture1);
    for (size_t i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, 4, 4, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 1, GL_RGBA, 2, 2, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
    }
    EXPECT_GL_NO_ERROR();
    for (size_t i = 0; i < 6; i++)
    {
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, 4, 4, GL_RGBA,
                        GL_UNSIGNED_BYTE, mip0Color.data());
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 1, 0, 0, 2, 2, GL_RGBA,
                        GL_UNSIGNED_BYTE, mip1Color.data());
    }
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 1, 1, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    GLColor::red.data());

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable cubemap texture is not uploaded if not complete.
TEST_P(VulkanPerformanceCounterTest, MutableTextureCubemapIncompleteInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded;

    std::vector<GLColor> mip0Color(4 * 4, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 2, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture1);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip0Color.data());
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 1, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip1Color.data());
    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable cubemap array texture is uploaded with appropriate mip level attributes.
TEST_P(VulkanPerformanceCounterTest, MutableTextureCubemapArrayCompatibleMipLevelsInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded + 1;

    std::vector<GLColor> mip0Color(4 * 4 * 6, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 2 * 6, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, texture1);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGBA, 4, 4, 6, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip0Color.data());
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, GL_RGBA, 2, 2, 6, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip1Color.data());

    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that mutable cubemap array texture is not uploaded with different layer-faces.
TEST_P(VulkanPerformanceCounterTest, MutableTextureCubemapArrayDifferentLayerFacesNoInit)
{
    ANGLE_SKIP_TEST_IF(!hasMutableMipmapTextureUpload());

    uint32_t expectedMutableTexturesUploaded = getPerfCounters().mutableTexturesUploaded;

    std::vector<GLColor> mip0Color(4 * 4 * 6, GLColor::red);
    std::vector<GLColor> mip1Color(2 * 2 * 12, GLColor::red);

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, texture1);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGBA, 4, 4, 6, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip0Color.data());
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, GL_RGBA, 2, 2, 12, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 mip1Color.data());

    EXPECT_GL_NO_ERROR();

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::green.data());
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(getPerfCounters().mutableTexturesUploaded, expectedMutableTexturesUploaded);
}

// Tests that RGB texture should not break renderpass.
TEST_P(VulkanPerformanceCounterTest, SampleFromRGBTextureDoesNotBreakRenderPass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program);
    GLint textureLoc = glGetUniformLocation(program, essl1_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLoc);

    GLTexture textureRGBA;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureRGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLTexture textureRGB;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureRGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    // First draw with textureRGBA which should start the renderpass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniform1i(textureLoc, 0);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Next draw with textureRGB which should not end the renderpass
    glUniform1i(textureLoc, 1);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Tests that RGB texture should not break renderpass.
TEST_P(VulkanPerformanceCounterTest, RenderToRGBTextureDoesNotBreakRenderPass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorUniformLocation);
    ASSERT_GL_NO_ERROR();

    GLTexture textureRGB;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureRGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureRGB, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    // Draw into FBO
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);  // clear to green
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, 256, 256);
    glUniform4fv(colorUniformLocation, 1, GLColor::blue.toNormalizedVector().data());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Tests that changing a Texture's max level hits the descriptor set cache.
TEST_P(VulkanPerformanceCounterTest, ChangingMaxLevelHitsDescriptorCache)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    GLColor kInitialData[4] = {GLColor::red, GLColor::blue, GLColor::green, GLColor::yellow};

    // Step 1: Set up a simple mipped 2D Texture rendering loop.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, kInitialData);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, kInitialData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);

    auto quadVerts = GetQuadVertices();

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(quadVerts[0]), quadVerts.data(),
                 GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, posLoc);

    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Step 2: Change max level and draw.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint64_t expectedWriteDescriptorSetCount = getPerfCounters().writeDescriptorSets;

    // Step 3: Change max level back to original value and verify we hit the cache.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint64_t actualWriteDescriptorSetCount = getPerfCounters().writeDescriptorSets;
    EXPECT_EQ(expectedWriteDescriptorSetCount, actualWriteDescriptorSetCount);
}

// Tests that two glCopyBufferSubData commands can share a barrier.
TEST_P(VulkanPerformanceCounterTest, IndependentBufferCopiesShareSingleBarrier)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr GLint srcDataA[] = {1, 2, 3, 4};
    constexpr GLint srcDataB[] = {5, 6, 7, 8};

    // Step 1: Set up four buffers for two copies.
    GLBuffer srcA;
    glBindBuffer(GL_COPY_READ_BUFFER, srcA);
    // Note: We can't use GL_STATIC_COPY. Using STATIC will cause driver to allocate a host
    // invisible memory and issue a copyToBuffer, which will trigger outsideRenderPassCommandBuffer
    // flush when glCopyBufferSubData is called due to read after write. That will break the
    // expectations and cause test to fail.
    glBufferData(GL_COPY_READ_BUFFER, sizeof(srcDataA), srcDataA, GL_DYNAMIC_COPY);

    GLBuffer dstA;
    glBindBuffer(GL_COPY_WRITE_BUFFER, dstA);
    // Note: We can't use GL_STATIC_COPY. Using STATIC will cause driver to allocate a host
    // invisible memory and issue a copyToBuffer, which will trigger outsideRenderPassCommandBuffer
    // flush when glCopyBufferSubData is called due to write after write. That will break the
    // expectations and cause test to fail.
    glBufferData(GL_COPY_WRITE_BUFFER, sizeof(srcDataA[0]) * 2, nullptr, GL_DYNAMIC_COPY);

    GLBuffer srcB;
    glBindBuffer(GL_COPY_READ_BUFFER, srcB);
    glBufferData(GL_COPY_READ_BUFFER, sizeof(srcDataB), srcDataB, GL_DYNAMIC_COPY);

    GLBuffer dstB;
    glBindBuffer(GL_COPY_WRITE_BUFFER, dstB);
    glBufferData(GL_COPY_WRITE_BUFFER, sizeof(srcDataB[0]) * 2, nullptr, GL_DYNAMIC_COPY);

    // We expect that ANGLE generate zero additional command buffers.
    uint64_t expectedFlushCount = getPerfCounters().flushedOutsideRenderPassCommandBuffers;

    // Step 2: Do the two copies.
    glBindBuffer(GL_COPY_READ_BUFFER, srcA);
    glBindBuffer(GL_COPY_WRITE_BUFFER, dstA);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, sizeof(srcDataB[0]), 0,
                        sizeof(srcDataA[0]) * 2);

    glBindBuffer(GL_COPY_READ_BUFFER, srcB);
    glBindBuffer(GL_COPY_WRITE_BUFFER, dstB);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, sizeof(srcDataB[0]), 0,
                        sizeof(srcDataB[0]) * 2);

    ASSERT_GL_NO_ERROR();

    uint64_t actualFlushCount = getPerfCounters().flushedOutsideRenderPassCommandBuffers;
    EXPECT_EQ(expectedFlushCount, actualFlushCount);
}

// Test resolving a multisampled texture with blit doesn't break the render pass so a subpass can be
// used
TEST_P(VulkanPerformanceCounterTest_ES31, MultisampleResolveWithBlit)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture,
                           0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(getPerfCounters().resolveImageCommands, 0u);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    constexpr uint8_t kHalfPixelGradient = 256 / kSize / 2;
    EXPECT_PIXEL_NEAR(0, 0, kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 255 - kHalfPixelGradient, kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(0, kSize - 1, kHalfPixelGradient, 255 - kHalfPixelGradient, 0, 255, 1.0);
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 255 - kHalfPixelGradient, 255 - kHalfPixelGradient, 0,
                      255, 1.0);
}

// Test resolving a multisampled texture with blit and then invalidate the msaa buffer
TEST_P(VulkanPerformanceCounterTest_ES31, ResolveToFBOWithInvalidate)
{
    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+0, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 0, 0, 1, 0, &expected);
    expected.colorAttachmentResolves = getPerfCounters().colorAttachmentResolves + 1;

    constexpr int kWindowWidth  = 4;
    constexpr int kWindowHeight = 4;
    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWindowWidth, kWindowHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);

    GLTexture msaaTexture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWindowWidth, kWindowHeight,
                              GL_FALSE);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaaFBO,
                           0);
    ANGLE_GL_PROGRAM(redprogram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    drawQuad(redprogram, essl1_shaders::PositionAttrib(), 0.5f);
    // Resolve into FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kWindowWidth, kWindowHeight, 0, 0, kWindowWidth, kWindowHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    GLenum attachment = GL_COLOR_ATTACHMENT0;
    glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, &attachment);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);

    // Top-left pixels should be all red.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);
    EXPECT_EQ(expected.colorAttachmentResolves, getPerfCounters().colorAttachmentResolves);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);

    ASSERT_GL_NO_ERROR();
}

// Test resolving different attachments of an FBO to separate FBOs then invalidate
TEST_P(VulkanPerformanceCounterTest_ES31, MultisampleResolveBothAttachments)
{
    enum class Invalidate
    {
        AfterEachResolve,
        AllAtEnd,
    };

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

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    const GLint color0Loc = glGetUniformLocation(program, "value0");
    const GLint color1Loc = glGetUniformLocation(program, "value2");

    constexpr int kWidth  = 16;
    constexpr int kHeight = 20;
    glViewport(0, 0, kWidth, kHeight);

    GLTexture msaa0, msaa1;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa1);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kWidth, kHeight, false);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaa0,
                           0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, msaa1,
                           0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLenum bufs[3] = {GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, bufs);

    // Create two resolve FBOs and textures. Use different texture levels and layers.
    GLTexture resolveTexture1;
    glBindTexture(GL_TEXTURE_2D, resolveTexture1);
    glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, kWidth * 2, kHeight * 2);

    GLFramebuffer resolveFBO1;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO1);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture1, 1);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLTexture resolveTexture2;
    glBindTexture(GL_TEXTURE_2D_ARRAY, resolveTexture2);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 4, GL_RGBA8, kWidth * 4, kHeight * 4, 5);

    GLFramebuffer resolveFBO2;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO2);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, resolveTexture2, 2, 3);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    auto test = [&](GLColor color0, GLColor color1, Invalidate invalidate) {
        const GLenum discards[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT2};

        // Resolve attachments should be used and the MSAA attachments should be invalidated.
        // Only the resolve attachments should have Store.
        //
        // Expect rpCount+1, color(Clears+0, Loads+0, LoadNones+0, Stores+2, StoreNones+0)
        angle::VulkanPerfCounters expected;
        setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 0, 0, 2, 0, &expected);
        expected.colorAttachmentResolves = getPerfCounters().colorAttachmentResolves + 2;

        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        glUniform4fv(color0Loc, 1, color0.toNormalizedVector().data());
        glUniform4fv(color1Loc, 1, color1.toNormalizedVector().data());
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        ASSERT_GL_NO_ERROR();

        // Resolve the first attachment
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO1);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        if (invalidate == Invalidate::AfterEachResolve)
        {
            glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, discards);
        }

        // Resolve the second attachment
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO2);
        glReadBuffer(GL_COLOR_ATTACHMENT2);
        glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        if (invalidate == Invalidate::AfterEachResolve)
        {
            glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, discards + 1);
        }
        else if (invalidate == Invalidate::AllAtEnd)
        {
            glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 2, discards);
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO1);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, color0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO2);
        EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, color1);
        ASSERT_GL_NO_ERROR();

        EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);
        EXPECT_EQ(expected.colorAttachmentResolves, getPerfCounters().colorAttachmentResolves);
        EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
    };

    test(GLColor::blue, GLColor::yellow, Invalidate::AfterEachResolve);
    test(GLColor::cyan, GLColor::magenta, Invalidate::AllAtEnd);
}

// Test resolving the depth/stencil attachment
TEST_P(VulkanPerformanceCounterTest_ES31, MultisampleDepthStencilResolve)
{
    ANGLE_SKIP_TEST_IF(!hasDepthStencilResolveThroughAttachment());

    constexpr int kWidth  = 24;
    constexpr int kHeight = 12;
    glViewport(0, 0, kWidth, kHeight);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    GLRenderbuffer depthStencil;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kWidth, kHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencil);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Create two resolve FBOs and textures. Use different texture levels and layers.
    GLTexture resolveTexture;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_DEPTH24_STENCIL8, kWidth * 4, kHeight * 4);

    GLFramebuffer resolveFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           resolveTexture, 2);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(red, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Red());

    // Resolve attachment should be used and the MSAA attachment should be invalidated.
    // Only the resolve attachment should have Store.
    //
    // Expect rpCount+1, depth(Clears+0, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    angle::VulkanPerfCounters expected;
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 1, 0, &expected);
    expected.depthAttachmentResolves   = getPerfCounters().depthAttachmentResolves + 1;
    expected.stencilAttachmentResolves = getPerfCounters().stencilAttachmentResolves + 1;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_TRUE);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x55, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    // Initialize the depth/stencil image
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    drawQuad(red, essl1_shaders::PositionAttrib(), 0.3f);
    ASSERT_GL_NO_ERROR();

    // Resolve depth and stencil, then verify the results
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight,
                      GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    // Invalidate depth/stencil
    const GLenum discards[] = {GL_DEPTH_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_READ_FRAMEBUFFER, 1, discards);

    // Break the render pass
    glFinish();

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);
    EXPECT_EQ(expected.depthAttachmentResolves, getPerfCounters().depthAttachmentResolves);
    EXPECT_EQ(expected.stencilAttachmentResolves, getPerfCounters().stencilAttachmentResolves);
    EXPECT_DEPTH_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    ASSERT_GL_NO_ERROR();
}

// Ensures a read-only depth-stencil feedback loop works in a single RenderPass.
TEST_P(VulkanPerformanceCounterTest, ReadOnlyDepthStencilFeedbackLoopUsesSingleRenderPass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr GLsizei kSize = 4;

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(texProgram, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set up a depth texture and fill it with an arbitrary initial value.
    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLFramebuffer depthAndColorFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, depthAndColorFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLFramebuffer depthOnlyFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, depthOnlyFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to a first FBO to initialize the depth buffer.
    glBindFramebuffer(GL_FRAMEBUFFER, depthOnlyFBO);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(redProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    // Start new RenderPass with depth write disabled and no loop.
    glBindFramebuffer(GL_FRAMEBUFFER, depthAndColorFBO);
    glDepthMask(false);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Now set up the read-only feedback loop.
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUseProgram(texProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Tweak the bits to keep it read-only.
    glEnable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Render with just the depth attachment.
    glUseProgram(redProgram);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Rebind the depth texture.
    glUseProgram(texProgram);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Do a final write to depth to make sure we can switch out of read-only mode.
    glBindTexture(GL_TEXTURE_2D, 0);
    glDepthMask(GL_TRUE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
}

// Ensures clear color buffer and read-only depth-stencil works in a single RenderPass (as seen in
// gfxbench manhattan).
TEST_P(VulkanPerformanceCounterTest, ClearColorBufferAndReadOnlyDepthStencilUsesSingleRenderPass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr GLsizei kSize = 4;

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(texProgram, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set up a depth texture and draw a quad to initialize it with known value.
    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    GLFramebuffer depthOnlyFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, depthOnlyFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_TRUE);
    glUseProgram(redProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Setup a color texture and FBO with color and read only depth attachment.
    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    GLFramebuffer depthAndColorFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, depthAndColorFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;
    // First clear color buffer. This should not cause this render pass to switch to read only depth
    // stencil mode
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    // Now set up the read-only feedback loop.
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUseProgram(texProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Render with just the depth attachment.
    glUseProgram(redProgram);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Rebind the depth texture.
    glUseProgram(texProgram);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Do a final write to depth to make sure we can switch out of read-only mode.
    glBindTexture(GL_TEXTURE_2D, 0);
    glDepthMask(GL_TRUE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
}

// Similar scenario as in ClearColorBufferAndReadOnlyDepthStencilUsesSingleRenderPass based on
// Manhattan, but involving queries that end up marking a render pass for closure.  This results in
// the switch to read-only depth/stencil mode to flush the render pass twice.
TEST_P(VulkanPerformanceCounterTest,
       QueryThenClearColorBufferAndReadOnlyDepthStencilUsesSingleRenderPass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr GLsizei kSize = 4;

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(texProgram, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set up a depth texture and framebuffer.
    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Issue a draw call that writes to the depth image.  At the same time, end a query after the
    // draw call.  The end of query can mark the render pass for closure.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_TRUE);
    glUseProgram(redProgram);
    GLQuery query;
    glBeginQuery(GL_ANY_SAMPLES_PASSED, query);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEndQuery(GL_ANY_SAMPLES_PASSED);
    ASSERT_GL_NO_ERROR();

    // Add a color texture to the FBO.  This changes the render pass such that glClear isn't
    // translated to vkCmdClearAttachments (which it would, if the color attachment was added
    // earlier).
    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Expect rpCount+1, depth(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0),
    angle::VulkanPerfCounters expected;
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 0, 0, 1, 0, &expected);

    // First clear the color buffer.  This leads to a deferred clear.
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Now set up the read-only feedback loop and issue a draw call.  The clear and draw should be
    // done in the same render pass.
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUseProgram(texProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);
    EXPECT_DEPTH_OP_COUNTERS(getPerfCounters(), expected);
}

// Make sure depth/stencil clears followed by a switch to read-only mode still lets the color clears
// use loadOp=CLEAR optimally.
TEST_P(VulkanPerformanceCounterTest, SwitchToReadOnlyDepthStencilLeavesOtherAspectsOptimal)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr GLsizei kSize = 4;

    ANGLE_GL_PROGRAM(texProgram, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set up a framebuffer.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexStorage2D(GL_TEXTURE_2D, 2, GL_DEPTH24_STENCIL8, kSize * 2, kSize * 2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTexture,
                           1);

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Expect rpCount+1, color(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    // depth(Clears+0, Loads+1, LoadNones+0, Stores+0, StoreNones+1),
    // stencil(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    angle::VulkanPerfCounters expected;
    setExpectedCountersForColorOps(getPerfCounters(), 0, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 1, 0, 0, 1, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 1, 0, 0, 1, 0, &expected);

    // Clear all aspects.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Then issue a draw call where the depth aspect is in feedback loop.  Depth should be cleared
    // before the render pass, but the color and stencil clears should still be optimized as loadOp.
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUseProgram(texProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    glFinish();

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_DEPTH_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
}

// Ensures an actual depth feedback loop (i.e, render and sample from same texture which is
// undefined behavior according to spec) works in a single RenderPass. This is adoptted from usage
// pattern from gangstar_vegas.
TEST_P(VulkanPerformanceCounterTest, DepthFeedbackLoopUsesSingleRenderPass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr GLsizei kSize = 4;

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(texProgram, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set up a depth texture and fill it with an arbitrary initial value.
    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLFramebuffer depthAndColorFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, depthAndColorFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLFramebuffer depthOnlyFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, depthOnlyFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to a first FBO to initialize the depth buffer.
    glBindFramebuffer(GL_FRAMEBUFFER, depthOnlyFBO);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(redProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    // Start new RenderPass with depth write enabled and form the actual feedback loop.
    glBindFramebuffer(GL_FRAMEBUFFER, depthAndColorFBO);
    glDepthMask(true);
    glUseProgram(texProgram);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Now set up the read-only feedback loop.
    glDepthMask(false);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Do a final write to depth to make sure we can switch out of read-only mode.
    glBindTexture(GL_TEXTURE_2D, 0);
    glDepthMask(GL_TRUE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
}

// Tests that invalidate followed by masked draws results in no load and store.
//
// - Scenario: invalidate, mask color, draw
TEST_P(VulkanPerformanceCounterTest, ColorInvalidateMaskDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 0, 1, 0, 1, &expected);

    GLFramebuffer framebuffer;
    GLTexture texture;
    setupForColorOpsTest(&framebuffer, &texture);

    // Invalidate
    const GLenum discards[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);
    ASSERT_GL_NO_ERROR();

    // Mask color output
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // Draw
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass without color mask
    ++expected.renderPasses;
    ++expected.colorStoreOpStores;

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Test glFenceSync followed by glInvalidateFramebuffer should still allow storeOp being optimized
// out.
TEST_P(VulkanPerformanceCounterTest, FenceSyncAndColorInvalidate)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 0, 0, 0, 0, &expected);

    GLFramebuffer framebuffer;
    GLTexture texture;
    setupForColorOpsTest(&framebuffer, &texture);

    // Draw
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Insert a fence which should trigger a deferred renderPass end
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    // Invalidate FBO. This should still allow vulkan backend to optimize out the storeOp, even
    // though we just called glFenceSync .
    const GLenum discards[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);
    ASSERT_GL_NO_ERROR();

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
    glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
}
// Tests that invalidate followed by discarded draws results in no load and store.
//
// - Scenario: invalidate, rasterizer discard, draw
TEST_P(VulkanPerformanceCounterTest, ColorInvalidateDiscardDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 0, 1, 0, 1, &expected);

    GLFramebuffer framebuffer;
    GLTexture texture;
    setupForColorOpsTest(&framebuffer, &texture);

    // Invalidate
    const GLenum discards[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);
    ASSERT_GL_NO_ERROR();

    // Mask color output
    glEnable(GL_RASTERIZER_DISCARD);

    // Draw
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass without color mask
    ++expected.renderPasses;
    ++expected.colorStoreOpStores;

    glDisable(GL_RASTERIZER_DISCARD);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that masked draws results in no load and store.
//
// - Scenario: mask color, draw
TEST_P(VulkanPerformanceCounterTest, ColorMaskedDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 0, 1, 0, 1, &expected);

    GLFramebuffer framebuffer;
    GLTexture texture;
    setupForColorOpsTest(&framebuffer, &texture);

    // Initialize the color buffer
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Mask color output
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // Draw
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass and check how many loads and stores were actually done
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass without color mask
    ++expected.renderPasses;
    ++expected.colorLoadOpLoads;
    ++expected.colorStoreOpStores;

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that discarded draws results in no load and store.
//
// - Scenario: rasterizer discard, draw
TEST_P(VulkanPerformanceCounterTest, ColorDiscardDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 0, 1, 0, 1, &expected);

    GLFramebuffer framebuffer;
    GLTexture texture;
    setupForColorOpsTest(&framebuffer, &texture);

    // Initialize the color buffer
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Mask color output
    glEnable(GL_RASTERIZER_DISCARD);

    // Draw
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass and check how many loads and stores were actually done
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass without color mask
    ++expected.renderPasses;
    ++expected.colorLoadOpLoads;
    ++expected.colorStoreOpStores;

    glDisable(GL_RASTERIZER_DISCARD);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Test that read-only color usage results in load but no store.
//
// - Scenario: mask color, framebuffer fetch draw
TEST_P(VulkanPerformanceCounterTest_ES31, ColorMaskedFramebufferFetchDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+0, Loads+1, LoadNones+0, Stores+0, StoreNones+1)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 1, 0, 0, 1, &expected);

    GLFramebuffer framebuffer;
    GLTexture texture;
    setupForColorOpsTest(&framebuffer, &texture);

    GLBuffer buffer;
    const GLColor kClearColor(40, 70, 100, 150);
    maskedFramebufferFetchDraw(kClearColor, buffer);

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass and check how many loads and stores were actually done
    EXPECT_PIXEL_COLOR_NEAR(0, 0, kClearColor, 1);

    maskedFramebufferFetchDrawVerify(kClearColor, buffer);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that clear after masked draws is optimized to use loadOp
//
// - Scenario: clear, mask color, draw, clear
TEST_P(VulkanPerformanceCounterTest, ColorClearMaskedDrawThenClear)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    // No vkCmdClearAttachments should be issued.
    setExpectedCountersForColorOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    expected.colorClearAttachments = getPerfCounters().colorClearAttachments;

    GLFramebuffer framebuffer;
    GLTexture texture;
    setupForColorOpsTest(&framebuffer, &texture);

    // Clear color first
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Mask color output
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // Draw
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Then clear color again.  This clear is moved up to loadOp, overriding the initial clear.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass and check how many loads and stores were actually done
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_CLEAR_ATTACHMENTS_COUNTER(expected.colorClearAttachments,
                                     getPerfCounters().colorClearAttachments);
}

// Test that clear of read-only color is not reordered with the draw.
//
// - Scenario: mask color, framebuffer fetch draw, clear
TEST_P(VulkanPerformanceCounterTest_ES31, ColorMaskedFramebufferFetchDrawThenClear)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0)
    // vkCmdClearAttachments should be used for the second clear.
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 1, 0, 1, 0, &expected);
    expected.colorClearAttachments = getPerfCounters().colorClearAttachments + 1;

    GLFramebuffer framebuffer;
    GLTexture texture;
    setupForColorOpsTest(&framebuffer, &texture);

    GLBuffer buffer;
    const GLColor kClearColor(40, 70, 100, 150);
    maskedFramebufferFetchDraw(kClearColor, buffer);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass and check how many loads and stores were actually done
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    maskedFramebufferFetchDrawVerify(kClearColor, buffer);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_CLEAR_ATTACHMENTS_COUNTER(expected.colorClearAttachments,
                                     getPerfCounters().colorClearAttachments);
}

// Test that masked draw after a framebuffer fetch render pass doesn't load color.
//
// - Scenario: framebuffer fetch render pass, mask color, normal draw
TEST_P(VulkanPerformanceCounterTest_ES31, FramebufferFetchRenderPassThenColorMaskedDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch_non_coherent"));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+0, Loads+1, LoadNones+0, Stores+0, StoreNones+1)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 1, 0, 0, 1, &expected);

    GLFramebuffer framebuffer;
    GLTexture texture;
    setupForColorOpsTest(&framebuffer, &texture);

    GLBuffer buffer;
    const GLColor kClearColor(40, 70, 100, 150);
    maskedFramebufferFetchDraw(kClearColor, buffer);

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass and check how many loads and stores were actually done
    EXPECT_PIXEL_COLOR_NEAR(0, 0, kClearColor, 1);

    maskedFramebufferFetchDrawVerify(kClearColor, buffer);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);

    // Start another render pass, and don't use framebuffer fetch.  Color is masked, so it should be
    // neither loaded nor stored.

    // Expect rpCount+1, color(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 0, 1, 0, 1, &expected);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass and check how many loads and stores were actually done
    EXPECT_PIXEL_COLOR_NEAR(0, 0, kClearColor, 1);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that clear after unused depth/stencil is optimized to use loadOp
//
// - Scenario: disable depth/stencil, draw, clear
TEST_P(VulkanPerformanceCounterTest, DepthStencilMaskedDrawThenClear)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    // This optimization is not implemented when this workaround is in effect.
    ANGLE_SKIP_TEST_IF(hasPreferDrawOverClearAttachments());

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 1, 0, 0, 1, 0, &expected);

    // No vkCmdClearAttachments should be issued.
    expected.depthClearAttachments   = getPerfCounters().depthClearAttachments;
    expected.stencilClearAttachments = getPerfCounters().stencilClearAttachments;

    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupForDepthStencilOpsTest(&framebuffer, &texture, &renderbuffer);

    // Disable depth/stencil
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    // Issue a draw call
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    // Clear depth/stencil
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass and check how many loads and stores were actually done
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_CLEAR_ATTACHMENTS_COUNTER(expected.depthClearAttachments,
                                     getPerfCounters().depthClearAttachments);
    EXPECT_CLEAR_ATTACHMENTS_COUNTER(expected.stencilClearAttachments,
                                     getPerfCounters().depthClearAttachments);
}

// Tests that depth compare function change, get correct loadop for depth buffer
//
// - Scenario: depth test enabled, depth write mask = 0,
//   clear depth, draw red quad with compare function always,
//   and then  draw green quad with compare function less equal
TEST_P(VulkanPerformanceCounterTest, DepthFunctionDynamicChangeLoadOp)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    // This optimization is not implemented when this workaround is in effect.
    ANGLE_SKIP_TEST_IF(hasPreferDrawOverClearAttachments());

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 0, 0, &expected);

    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupForColorDepthOpsTest(&framebuffer, &texture, &renderbuffer);

    // Clear color and depth.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // No depth write
    glDepthMask(GL_FALSE);
    // Depth function always
    glDepthFunc(GL_ALWAYS);

    // Draw read quad.
    ANGLE_GL_PROGRAM(redprogram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(redprogram, essl1_shaders::PositionAttrib(), 0.5f);

    // Depth function switch to less equal
    glDepthFunc(GL_LEQUAL);

    // Draw green quad.
    ANGLE_GL_PROGRAM(greenprogram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(greenprogram, essl1_shaders::PositionAttrib(), 0.7f);

    GLenum attachments = GL_DEPTH_ATTACHMENT;
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachments);

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass and check how many clears were actually done
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_CLEAR_ATTACHMENTS_COUNTER(expected.depthLoadOpClears,
                                     getPerfCounters().depthLoadOpClears);
}

// Tests that common PUBG MOBILE case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, disable, draw
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateDisableDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Invalidate (storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Disable (shouldn't change result)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    // Draw (since disabled, shouldn't change result)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 0, 0, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    swapBuffers();
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that alternative PUBG MOBILE case does not break render pass, and that counts are correct:
//
// - Scenario: disable, invalidate, draw
TEST_P(VulkanPerformanceCounterTest, DisableInvalidateDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Disable (shouldn't change result)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    // Invalidate (storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Draw (since disabled, shouldn't change result)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 0, 0, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    swapBuffers();
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: disable, draw, invalidate, enable
TEST_P(VulkanPerformanceCounterTest, DisableDrawInvalidateEnable)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Note: setupClearAndDrawForDepthStencilOpsTest() did an enable and draw

    // Disable (since not invalidated, shouldn't change result)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    // Draw (since not invalidated, shouldn't change result)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Enable (shouldn't change result)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    // Note: The above enable calls will be ignored, since no drawing was done to force the enable
    // dirty bit to be processed

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass by reading back a pixel.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 0, 0, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    swapBuffers();
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that common TRex case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidate)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Invalidate (storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 0, 0, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    swapBuffers();
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Similar to Invalidate, but uses glInvalidateSubFramebuffer such that the given area covers the
// whole framebuffer.
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateSub)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Invalidate (storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateSubFramebuffer(GL_FRAMEBUFFER, 2, discards, -100, -100, kOpsTestSize + 200,
                               kOpsTestSize + 200);
    ASSERT_GL_NO_ERROR();

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 0, 0, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    swapBuffers();
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Similar to InvalidateSub, but uses glInvalidateSubFramebuffer such that the given area does NOT
// covers the whole framebuffer.
TEST_P(VulkanPerformanceCounterTest, DepthStencilPartialInvalidateSub)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 1, 0, 0, 1, 0, &expected);

    // Create the framebuffer and make sure depth/stencil have valid contents.
    ANGLE_GL_PROGRAM(drawRed, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&drawRed, &framebuffer, &texture, &renderbuffer, true);

    // Break the render pass so depth/stencil values are stored.
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start a new render pass that is scissored.  Depth/stencil should be loaded.  The draw call is
    // followed by an invalidate, so store shouldn't happen.

    // Expect rpCount+1, depth(Clears+0, Loads+1, LoadNones+0, Stores+0, StoreNones+0),
    // stencil(Clears+0, Loads+1, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 1, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 1, 0, 0, 0, &expected);

    glEnable(GL_SCISSOR_TEST);
    glScissor(kOpsTestSize / 8, kOpsTestSize / 4, kOpsTestSize / 2, kOpsTestSize / 3);

    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 0x55, 0xFF);
    glDepthFunc(GL_ALWAYS);

    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateSubFramebuffer(GL_FRAMEBUFFER, 2, discards, kOpsTestSize / 8, kOpsTestSize / 8,
                               7 * kOpsTestSize / 8, 7 * kOpsTestSize / 8);

    // Break the render pass so depth/stencil values are discarded.
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);
    EXPECT_PIXEL_COLOR_EQ(kOpsTestSize / 2, kOpsTestSize / 2, GLColor::green);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start another render pass without scissor.  Because parts of the framebuffer attachments were
    // not invalidated, depth/stencil should be loaded.

    // Expect rpCount+1, depth(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 1, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 1, 0, 1, 0, &expected);

    glDisable(GL_SCISSOR_TEST);

    ANGLE_GL_PROGRAM(drawBlue, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(drawBlue, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Verify results
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, draw
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 1, 0, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    swapBuffers();
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, draw, disable
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateDrawDisable)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    // http://anglebug.com/40096809
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsVulkan());

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 1, 0, 1, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Disable (shouldn't change result)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    // Note: this draw is just so that the disable dirty bits will be processed
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass and then check how many loads and stores were actually done
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 1, 1, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, disable, draw, enable
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateDisableDrawEnable)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Disable (shouldn't change result)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    // Draw (since disabled, shouldn't change result)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Enable (shouldn't change result)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    // Note: The above enable calls will be ignored, since no drawing was done to force the enable
    // dirty bit to be processed

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 0, 0, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    swapBuffers();
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, disable, draw, enable, draw
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateDisableDrawEnableDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 1, 0, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Disable (shouldn't change result)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    // Draw (since disabled, shouldn't change result)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Enable (shouldn't change result)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass and then check how many loads and stores were actually done
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 1, 1, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, draw, disable, enable
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateDrawDisableEnable)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 1, 0, 1, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Disable (shouldn't change result)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    // Note: this draw is just so that the disable dirty bits will be processed
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Enable (shouldn't change result)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    // Note: The above enable calls will be ignored, since no drawing was done to force the enable
    // dirty bit to be processed

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Break the render pass and then check how many loads and stores were actually done
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 1, 0, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, draw, disable, enable, invalidate
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateDrawDisableEnableInvalidate)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Disable (shouldn't change result)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    // Note: this draw is just so that the disable dirty bits will be processed
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Enable (shouldn't change result)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 0, 0, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    swapBuffers();
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, draw, disable, enable, invalidate, draw
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateDrawDisableEnableInvalidateDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Disable (shouldn't change result)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    // Note: this draw is just so that the disable dirty bits will be processed
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Enable (shouldn't change result)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 1, 0, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    swapBuffers();
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that another common (dEQP) case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, disable, enable, draw
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateDisableEnableDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 1, 0, 1, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Execute the scenario that this test is for:

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Disable (shouldn't change result)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    // Note: this draw is just so that the disable dirty bits will be processed
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Enable (shouldn't change result)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Ensure that the render pass wasn't broken
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 1, 0, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    swapBuffers();
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that an in renderpass clear after invalidate keeps content stored.
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateAndClear)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    // Clear should vkCmdClearAttachments
    expected.depthClearAttachments = getPerfCounters().depthClearAttachments + 1;

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, false);

    // Disable depth test but with depth mask enabled so that clear should still work.
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Do in-renderpass clear. This should result in StoreOp=STORE; mContentDefined = true.
    glClearDepthf(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_CLEAR_ATTACHMENTS_COUNTER(expected.depthClearAttachments,
                                     getPerfCounters().depthClearAttachments);

    // Expect rpCount+1, depth(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 1, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 1, 0, 1, &expected);

    // Bind FBO again and try to use the depth buffer without clear. This should result in
    // loadOp=LOAD and StoreOP=STORE
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_STENCIL_TEST);
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    // Should pass depth test: (0.5+1.0)/2.0=0.75 < 1.0
    drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(kOpsTestSize / 2, kOpsTestSize / 2, GLColor::blue);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that the draw path for clear after invalidate and disabling depth/stencil test keeps
// content stored.
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateAndMaskedClear)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 1, 0, 0, 1, 0, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer, true);

    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Disable depth/stencil test but make stencil masked
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDepthMask(GL_TRUE);
    glStencilMask(0xF0);

    // Enable scissor for the draw path to be taken.
    glEnable(GL_SCISSOR_TEST);
    glScissor(kOpsTestSize / 4, kOpsTestSize / 4, kOpsTestSize / 2, kOpsTestSize / 2);

    // Do in-renderpass clear. This should result in StoreOp=STORE
    glClearDepthf(1.0f);
    glClearStencil(0x55);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Expect rpCount+1, depth(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+1, LoadNones+0, Stores+0, StoreNones+1)
    // Note that depth write is enabled, while stencil is disabled.
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 1, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 1, 0, 0, 1, &expected);

    // Bind FBO again and try to use the depth buffer without clear. This should result in
    // loadOp=LOAD and StoreOP=STORE
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x50, 0xF0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(blueProgram, essl1_shaders::PositionAttrib(), 0.95f);
    EXPECT_PIXEL_COLOR_EQ(kOpsTestSize / 2, kOpsTestSize / 2, GLColor::blue);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that the renderpass is using depthFunc(GL_ALWAYS) and depthMask(GL_FALSE), it should not
// load or store depth value.
TEST_P(VulkanPerformanceCounterTest, DepthFuncALWAYSWithDepthMaskDisabledShouldNotLoadStore)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupForDepthStencilOpsTest(&framebuffer, &texture, &renderbuffer);

    // Expect rpCount+1, depth(Clears+0, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 0, 0, 1, 0, &expected);
    if (hasDisallowMixedDepthStencilLoadOpNoneAndLoad())
    {
        // stencil(Clears+0, Loads+1, LoadNones+0, Stores+0, StoreNones+1)
        setExpectedCountersForStencilOps(getPerfCounters(), 0, 1, 0, 0, 1, &expected);
    }
    else
    {
        // stencil(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
        setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 1, 0, 1, &expected);
    }
    // Initialize the buffers with known value
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);
    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.95f);
    EXPECT_PIXEL_COLOR_EQ(kOpsTestSize / 2, kOpsTestSize / 2, GLColor::red);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Expect rpCount+1, depth(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1),
    //                 stencil(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 0, 1, 0, 1, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 1, 0, 1, &expected);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_ALWAYS);
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.95f);
    EXPECT_PIXEL_COLOR_EQ(kOpsTestSize / 2, kOpsTestSize / 2, GLColor::red);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that the renderpass is using depthFunc(GL_ALWAYS) and depthMask(GL_FALSE) and draw. Then it
// followed by glClear, it should not load or store depth value.
TEST_P(VulkanPerformanceCounterTest,
       DepthFuncALWAYSWithDepthMaskDisabledThenClearShouldNotLoadStore)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupForDepthStencilOpsTest(&framebuffer, &texture, &renderbuffer);

    // Initialize the buffers with known value
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);
    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.95f);
    EXPECT_PIXEL_COLOR_EQ(kOpsTestSize / 2, kOpsTestSize / 2, GLColor::red);

    if (hasPreferDrawOverClearAttachments())
    {
        // Expect rpCount+1, depth(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0),
        //                 stencil(Clears+0, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
        setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 1, 0, 1, 0, &expected);
    }
    else
    {
        // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
        //                 stencil(Clears+0, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
        setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    }
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 1, 0, &expected);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_ALWAYS);
    glEnable(GL_STENCIL_TEST);
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.95f);
    glDepthMask(GL_TRUE);
    glClearDepthf(1);
    glClearStencil(0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glEnable(GL_STENCIL_TEST);
    glStencilFuncSeparate(GL_FRONT, GL_ALWAYS, 0, 255);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFuncSeparate(GL_BACK, GL_ALWAYS, 0, 255);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(255);
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.9f);
    EXPECT_PIXEL_COLOR_EQ(kOpsTestSize / 2, kOpsTestSize / 2, GLColor::red);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that the renderpass is using depthFunc(GL_NEVER) and depthMask(GL_FALSE), it should not
// load or store depth value.
TEST_P(VulkanPerformanceCounterTest, DepthFuncNEVERWithDepthMaskDisabledShouldNotLoadStore)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupForDepthStencilOpsTest(&framebuffer, &texture, &renderbuffer);

    // Initialize the buffers with known value
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);
    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.95f);
    EXPECT_PIXEL_COLOR_EQ(kOpsTestSize / 2, kOpsTestSize / 2, GLColor::red);

    // Expect rpCount+1, depth(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1),
    //                 stencil(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 0, 1, 0, 1, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 1, 0, 1, &expected);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_NEVER);
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.95f);
    EXPECT_PIXEL_COLOR_EQ(kOpsTestSize / 2, kOpsTestSize / 2, GLColor::red);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests whether depth-stencil ContentDefined will be correct when:
//
// - Scenario: invalidate, detach D/S texture and modify it, attach D/S texture, draw with blend
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateDetachModifyTexAttachDrawWithBlend)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    GLTexture depthStencilTexture;
    glBindTexture(GL_TEXTURE_2D, depthStencilTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 2, 2, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           depthStencilTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear and draw with depth-stencil enabled
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glClearDepthf(0.99f);
    glEnable(GL_STENCIL_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Invalidate depth & stencil (should result: in storeOp = DONT_CARE; mContentDefined = false)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Check for the expected number of render passes, expected color, and other expected counters
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Detach depth-stencil attachment
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Modify depth-stencil
    constexpr uint32_t kDepthStencilInitialValue = 0xafffff00;
    uint32_t depthStencilData[4] = {kDepthStencilInitialValue, kDepthStencilInitialValue,
                                    kDepthStencilInitialValue, kDepthStencilInitialValue};
    glBindTexture(GL_TEXTURE_2D, depthStencilTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 2, 2, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, depthStencilData);

    // Re-attach depth-stencil
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                           depthStencilTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw again, showing that the modified depth-stencil value prevents a new color value
    //
    // Expect rpCount+1, depth(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+1, LoadNones+0, Stores+0, StoreNones+1)
    // Note that depth write is enabled, while stencil is disabled.
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 1, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 1, 0, 0, 1, &expected);
    drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    // Check for the expected number of render passes, expected color, and other expected counters
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Draw again, using a different depth value, so that the drawing takes place
    //
    // Expect rpCount+1, depth(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+1, LoadNones+0, Stores+0, StoreNones+1)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 1, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 1, 0, 0, 1, &expected);
    drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.2f);
    ASSERT_GL_NO_ERROR();
    // Check for the expected number of render passes, expected color, and other expected counters
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that a GLRenderbuffer can be deleted before the render pass ends, and that everything
// still works.
//
// - Scenario: invalidate
TEST_P(VulkanPerformanceCounterTest, DepthStencilInvalidateDrawAndDeleteRenderbuffer)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    {
        // Declare the RAII-based GLRenderbuffer object within this set of curly braces, so that it
        // will be deleted early (at the close-curly-brace)
        GLRenderbuffer renderbuffer;
        setupClearAndDrawForDepthStencilOpsTest(&program, &framebuffer, &texture, &renderbuffer,
                                                false);

        // Invalidate (storeOp = DONT_CARE; mContentDefined = false)
        const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
        ASSERT_GL_NO_ERROR();

        // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();

        // Ensure that the render pass wasn't broken
        EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);
    }

    // The renderbuffer should now be deleted.

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Start and end another render pass, to check that the load ops are as expected
    setAndIncrementDepthStencilLoadCountersForOpsTest(getPerfCounters(), 0, 0, &expected);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    swapBuffers();
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
}

// Test that disabling color buffer after clear continues to use loadOp for it.
//
// - Scenario: clear color and depth, disable color, draw, enable color, draw
TEST_P(VulkanPerformanceCounterTest_ES31, ColorDisableThenDrawThenEnableThenDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupForColorDepthOpsTest(&framebuffer, &texture, &renderbuffer);

    // Expected:
    //   rpCount+1,
    //   depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    //   color(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 0, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForColorOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);

    // Clear color and depth first
    glClearColor(1, 0, 0, 1);
    glClearDepthf(0.123);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Disable color output
    GLenum drawBuffers[] = {GL_NONE};
    glDrawBuffers(1, drawBuffers);

    // Issue a draw call, only affecting depth
    glDepthFunc(GL_ALWAYS);
    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.75f);

    // Enable color output
    drawBuffers[0] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, drawBuffers);

    // Issue another draw call, verifying depth simultaneously
    glDepthFunc(GL_LESS);
    ANGLE_GL_PROGRAM(drawBlue, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(drawBlue, essl1_shaders::PositionAttrib(), 0.74f);

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Verify results and check how many loads and stores were actually done.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_DEPTH_OP_COUNTERS(getPerfCounters(), expected);
    ASSERT_GL_NO_ERROR();
}

// Tests that even if the app clears depth, it should be invalidated if there is no read.
TEST_P(VulkanPerformanceCounterTest, SwapShouldInvalidateDepthAfterClear)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    // Clear depth.
    glClear(GL_DEPTH_BUFFER_BIT);

    // Ensure we never read from depth.
    glDisable(GL_DEPTH_TEST);

    // Do one draw, then swap.
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    uint64_t expectedDepthClears = getPerfCounters().depthLoadOpClears;

    swapBuffers();

    uint64_t actualDepthClears = getPerfCounters().depthLoadOpClears;
    EXPECT_EQ(expectedDepthClears, actualDepthClears);
}

// Tests that masked color clears don't break the RP.
TEST_P(VulkanPerformanceCounterTest, MaskedColorClearDoesNotBreakRenderPass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    // Mask color channels and clear the framebuffer multiple times.
    glClearColor(0.25f, 0.25f, 0.25f, 0.25f);
    glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE);
    glClear(GL_COLOR_BUFFER_BIT);

    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
    glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE);
    glClear(GL_COLOR_BUFFER_BIT);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
    glClear(GL_COLOR_BUFFER_BIT);

    glClearColor(0.75f, 0.75f, 0.75f, 0.75f);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT);

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    EXPECT_PIXEL_NEAR(0, 0, 63, 127, 255, 191, 1);
}

// Tests that masked color/depth/stencil clears don't break the RP.
TEST_P(VulkanPerformanceCounterTest, MaskedClearDoesNotBreakRenderPass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr GLsizei kSize = 64;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffer);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorUniformLocation);
    ASSERT_GL_NO_ERROR();

    // Clear the framebuffer with a draw call to start a render pass.
    glViewport(0, 0, kSize, kSize);
    glDepthFunc(GL_ALWAYS);
    glStencilFunc(GL_ALWAYS, 0x55, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(program, essl1_shaders::PositionAttrib(), 1.0f);

    // Issue a masked clear.
    glClearColor(0.25f, 1.0f, 0.25f, 1.25f);
    glClearDepthf(0.0f);
    glClearStencil(0x3F);
    glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE);
    glStencilMask(0xF0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Make sure the render pass wasn't broken.
    EXPECT_EQ(expectedRenderPassCount, getPerfCounters().renderPasses);

    // Verify that clear was done correctly.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::yellow);

    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0xFF);

    // Make sure depth = 0.0f, stencil = 0x35
    glDepthFunc(GL_GREATER);
    glStencilFunc(GL_EQUAL, 0x35, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.05f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::blue);
}

// Tests that clear followed by scissored draw uses loadOp to clear.
TEST_P(VulkanPerformanceCounterTest, ClearThenScissoredDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;
    uint64_t expectedDepthClears     = getPerfCounters().depthLoadOpClears + 1;
    uint64_t expectedStencilClears   = getPerfCounters().stencilLoadOpClears + 1;

    constexpr GLsizei kSize = 64;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffer);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Clear depth/stencil
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Issue a scissored draw call, expecting depth/stencil to be 1.0 and 0x55.
    glViewport(0, 0, kSize, kSize);
    glScissor(0, 0, kSize / 2, kSize);
    glEnable(GL_SCISSOR_TEST);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);

    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.95f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass.
    GLTexture copyTex;
    glBindTexture(GL_TEXTURE_2D, copyTex);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, kSize, kSize, 0);
    ASSERT_GL_NO_ERROR();

    // Make sure a single render pass was used and depth/stencil clear used loadOp=CLEAR.
    EXPECT_EQ(expectedRenderPassCount, getPerfCounters().renderPasses);
    EXPECT_EQ(expectedDepthClears, getPerfCounters().depthLoadOpClears);
    EXPECT_EQ(expectedStencilClears, getPerfCounters().stencilLoadOpClears);

    // Verify correctness.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2 - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2 - 1, kSize - 1, GLColor::green);

    EXPECT_PIXEL_COLOR_EQ(kSize / 2, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::red);
}

// Tests that scissored clears don't break the RP.
TEST_P(VulkanPerformanceCounterTest, ScissoredClearDoesNotBreakRenderPass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr GLsizei kSize = 64;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffer);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorUniformLocation);
    ASSERT_GL_NO_ERROR();

    // Clear the framebuffer with a draw call to start a render pass.
    glViewport(0, 0, kSize, kSize);
    glDepthFunc(GL_ALWAYS);
    glStencilFunc(GL_ALWAYS, 0x55, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(program, essl1_shaders::PositionAttrib(), 1.0f);

    // Issue a scissored clear.
    glEnable(GL_SCISSOR_TEST);
    glScissor(kSize / 4, kSize / 4, kSize / 2, kSize / 2);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClearDepthf(0.0f);
    glClearStencil(0x3F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Make sure the render pass wasn't broken.
    EXPECT_EQ(expectedRenderPassCount, getPerfCounters().renderPasses);

    // Verify that clear was done correctly.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::red);

    EXPECT_PIXEL_COLOR_EQ(kSize / 4, kSize / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * kSize / 4 - 1, kSize / 4, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize / 4, 3 * kSize / 4 - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(3 * kSize / 4 - 1, 3 * kSize / 4 - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::green);

    glDisable(GL_SCISSOR_TEST);

    // Make sure the border has depth = 1.0f, stencil = 0x55
    glDepthFunc(GL_LESS);
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.95f);
    ASSERT_GL_NO_ERROR();

    // Make sure the center has depth = 0.0f, stencil = 0x3F
    glDepthFunc(GL_GREATER);
    glStencilFunc(GL_EQUAL, 0x3F, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.05f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::blue);

    EXPECT_PIXEL_COLOR_EQ(kSize / 4, kSize / 4, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(3 * kSize / 4 - 1, kSize / 4, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kSize / 4, 3 * kSize / 4 - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(3 * kSize / 4 - 1, 3 * kSize / 4 - 1, GLColor::magenta);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::magenta);
}

// Tests that draw buffer change with all color channel mask off should not break renderpass
TEST_P(VulkanPerformanceCounterTest, DrawbufferChangeWithAllColorMaskDisabled)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorUniformLocation);
    ASSERT_GL_NO_ERROR();

    GLTexture textureRGBA;
    glBindTexture(GL_TEXTURE_2D, textureRGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLTexture textureDepth;
    glBindTexture(GL_TEXTURE_2D, textureDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 64, 64, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureRGBA, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepth, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    // Draw into FBO
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);  // clear to green
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, 256, 256);
    glUniform4fv(colorUniformLocation, 1, GLColor::blue.toNormalizedVector().data());
    GLenum glDrawBuffers_bufs_1[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, glDrawBuffers_bufs_1);
    glEnable(GL_DEPTH_TEST);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    // Change draw buffer state and color mask
    GLenum glDrawBuffers_bufs_0[] = {GL_NONE};
    glDrawBuffers(1, glDrawBuffers_bufs_0);
    glColorMask(false, false, false, false);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.6f);
    // Change back draw buffer state and color mask
    glDrawBuffers(1, glDrawBuffers_bufs_1);
    glColorMask(true, true, true, true);
    glUniform4fv(colorUniformLocation, 1, GLColor::red.toNormalizedVector().data());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.7f);

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Tests the optimization that a glFlush call issued inside a renderpass will be skipped.
TEST_P(VulkanPerformanceCounterTest, InRenderpassFlushShouldNotBreakRenderpass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
    glFlush();
    ANGLE_GL_PROGRAM(greenProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(greenProgram, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Tests switch from query enabled draw to query disabled draw should break renderpass (so that wait
// for query result will be available sooner).
TEST_P(VulkanPerformanceCounterTest,
       SwitchFromQueryEnabledDrawToQueryDisabledDrawShouldBreakRenderpass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    uint64_t expectedRenderPassCount =
        getPerfCounters().renderPasses +
        (isFeatureEnabled(Feature::PreferSubmitOnAnySamplesPassedQueryEnd) ? 2 : 1);

    GLQueryEXT query1, query2;
    // Draw inside query
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query1);
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.8f, 0.5f);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, query2);
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.8f, 0.5f);
    glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT);
    // Draw outside his query
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.8f, 0.5f);

    GLuint results[2];
    // will block waiting for result
    glGetQueryObjectuivEXT(query1, GL_QUERY_RESULT_EXT, &results[0]);
    glGetQueryObjectuivEXT(query2, GL_QUERY_RESULT_EXT, &results[1]);
    EXPECT_GL_NO_ERROR();

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Tests that depth/stencil texture clear/load works correctly.
TEST_P(VulkanPerformanceCounterTest, DepthStencilTextureClearAndLoad)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    // TODO: http://anglebug.com/42263870 Flaky test
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsVulkan());

    uint64_t expectedDepthClearCount   = getPerfCounters().depthLoadOpClears + 1;
    uint64_t expectedDepthLoadCount    = getPerfCounters().depthLoadOpLoads + 3;
    uint64_t expectedStencilClearCount = getPerfCounters().stencilLoadOpClears + 1;
    uint64_t expectedStencilLoadCount  = getPerfCounters().stencilLoadOpLoads + 3;

    constexpr GLsizei kSize = 6;

    // Create framebuffer to draw into, with both color and depth attachments.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLTexture depth;
    glBindTexture(GL_TEXTURE_2D, depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, kSize, kSize, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8_OES, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
    ASSERT_GL_NO_ERROR();

    // Set up texture for copy operation that breaks the render pass
    GLTexture copyTex;
    glBindTexture(GL_TEXTURE_2D, copyTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set viewport and clear depth/stencil
    glViewport(0, 0, kSize, kSize);
    glClearDepthf(1);
    glClearStencil(0x55);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // If depth is not cleared to 1, rendering would fail.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);

    // If stencil is not clear to 0x55, rendering would fail.
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);

    // Set up program
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Draw red
    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw green
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, kSize / 2, 0, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw blue
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, kSize / 2, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw yellow
    glUniform4f(colorUniformLocation, 1.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, kSize / 2, kSize / 2, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Verify the counters
    EXPECT_EQ(getPerfCounters().depthLoadOpClears, expectedDepthClearCount);
    EXPECT_EQ(getPerfCounters().depthLoadOpLoads, expectedDepthLoadCount);
    EXPECT_EQ(getPerfCounters().stencilLoadOpClears, expectedStencilClearCount);
    EXPECT_EQ(getPerfCounters().stencilLoadOpLoads, expectedStencilLoadCount);

    // Verify that copies were done correctly.
    GLFramebuffer verifyFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, verifyFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copyTex, 0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize / 2, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::yellow);
}

// Tests that multisampled-render-to-texture depth/stencil textures don't ever load data.
TEST_P(VulkanPerformanceCounterTest, RenderToTextureDepthStencilTextureShouldNotLoad)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    // http://anglebug.com/42263651
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsVulkan());

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture2"));

    uint64_t expectedDepthClearCount   = getPerfCounters().depthLoadOpClears + 1;
    uint64_t expectedDepthLoadCount    = getPerfCounters().depthLoadOpLoads;
    uint64_t expectedStencilClearCount = getPerfCounters().stencilLoadOpClears + 1;
    uint64_t expectedStencilLoadCount  = getPerfCounters().stencilLoadOpLoads;

    constexpr GLsizei kSize = 6;

    // Create multisampled framebuffer to draw into, with both color and depth attachments.
    GLTexture colorMS;
    glBindTexture(GL_TEXTURE_2D, colorMS);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLTexture depthMS;
    glBindTexture(GL_TEXTURE_2D, depthMS);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, kSize, kSize, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8_OES, nullptr);

    GLFramebuffer fboMS;
    glBindFramebuffer(GL_FRAMEBUFFER, fboMS);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         colorMS, 0, 4);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                                         depthMS, 0, 4);
    ASSERT_GL_NO_ERROR();

    // Set up texture for copy operation that breaks the render pass
    GLTexture copyTex;
    glBindTexture(GL_TEXTURE_2D, copyTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set viewport and clear depth
    glViewport(0, 0, kSize, kSize);
    glClearDepthf(1);
    glClearStencil(0x55);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // If depth is not cleared to 1, rendering would fail.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // If stencil is not clear to 0x55, rendering would fail.
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);

    // Set up program
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Draw red
    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw green
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, kSize / 2, 0, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw blue
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, kSize / 2, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw yellow
    glUniform4f(colorUniformLocation, 1.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, kSize / 2, kSize / 2, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Verify the counters
    EXPECT_EQ(getPerfCounters().depthLoadOpClears, expectedDepthClearCount);
    EXPECT_EQ(getPerfCounters().depthLoadOpLoads, expectedDepthLoadCount);
    EXPECT_EQ(getPerfCounters().stencilLoadOpClears, expectedStencilClearCount);
    EXPECT_EQ(getPerfCounters().stencilLoadOpLoads, expectedStencilLoadCount);

    // Verify that copies were done correctly.  Only the first copy can be verified because the
    // contents of the depth/stencil buffer is undefined after the first render pass break, meaning
    // it is unknown whether the three subsequent draw calls passed the depth or stencil tests.
    GLFramebuffer verifyFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, verifyFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copyTex, 0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2 - 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, kSize / 2 - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2 - 1, kSize / 2 - 1, GLColor::red);
}

// Tests that multisampled-render-to-texture depth/stencil renderbuffers don't ever load
// depth/stencil data.
TEST_P(VulkanPerformanceCounterTest, RenderToTextureDepthStencilRenderbufferShouldNotLoad)
{
    // http://anglebug.com/42263651
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsVulkan());
    // http://anglebug.com/42263920
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsVulkan());

    // http://crbug.com/1134286
    ANGLE_SKIP_TEST_IF(IsWindows7() && IsNVIDIA() && IsVulkan());

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // This test creates 4 render passes. In the first render pass, color, depth and stencil are
    // cleared.  In the following render passes, they must be loaded.  However, given that the
    // attachments are multisampled-render-to-texture, loads are done through an unresolve
    // operation.  All 4 render passes resolve the attachments.

    // Expect rpCount+4, depth(Clears+1, Loads+3, LoadNones+0, Stores+3, StoreNones+0),
    // stencil(Clears+1, Loads+3, LoadNones+0, Stores+3, StoreNones+0). Note that the Loads and
    // Stores are from the resolve attachments.
    setExpectedCountersForDepthOps(getPerfCounters(), 4, 1, 3, 0, 3, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 1, 3, 0, 3, 0, &expected);

    // Additionally, expect 4 resolves and 3 unresolves.
    setExpectedCountersForUnresolveResolveTest(getPerfCounters(), 3, 3, 3, 4, 4, 4, &expected);

    constexpr GLsizei kSize = 6;

    // Create multisampled framebuffer to draw into, with both color and depth attachments.
    GLTexture colorMS;
    glBindTexture(GL_TEXTURE_2D, colorMS);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLRenderbuffer depthStencilMS;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilMS);
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer fboMS;
    glBindFramebuffer(GL_FRAMEBUFFER, fboMS);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         colorMS, 0, 4);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencilMS);
    ASSERT_GL_NO_ERROR();

    // Set up texture for copy operation that breaks the render pass
    GLTexture copyTex;
    glBindTexture(GL_TEXTURE_2D, copyTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set viewport and clear color, depth and stencil
    glViewport(0, 0, kSize, kSize);
    glClearDepthf(1);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // If depth is not cleared to 1, rendering would fail.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // If stencil is not clear to 0x55, rendering would fail.
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);

    // Set up program
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Draw red
    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.75f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw green
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, kSize / 2, 0, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw blue
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.25f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, kSize / 2, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw yellow
    glUniform4f(colorUniformLocation, 1.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, kSize / 2, kSize / 2, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Verify the counters
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_COUNTERS_FOR_UNRESOLVE_RESOLVE_TEST(getPerfCounters(), expected);

    // Verify that copies were done correctly.
    GLFramebuffer verifyFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, verifyFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copyTex, 0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, kSize / 2, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::yellow);
}

// Tests counters when multisampled-render-to-texture color/depth/stencil renderbuffers are
// invalidated.
TEST_P(VulkanPerformanceCounterTest, RenderToTextureInvalidate)
{
    // http://anglebug.com/42263651
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsVulkan());

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // This test creates 4 render passes. In the first render pass, color, depth and stencil are
    // cleared.  After every render pass, the attachments are invalidated.  In the following render
    // passes thus they are not loaded (rather unresolved, as the attachments are
    // multisampled-render-to-texture).  Due to the invalidate call, neither of the 4 render passes
    // should resolve the attachments.

    // Expect rpCount+4, color(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 4, 1, 0, 0, 0, 0, &expected);
    // Expect rpCount+4, depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    // stencil(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 4, 1, 0, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 1, 0, 0, 0, 0, &expected);

    // Additionally, expect no resolve and unresolve.
    setExpectedCountersForUnresolveResolveTest(getPerfCounters(), 0, 0, 0, 0, 0, 0, &expected);

    constexpr GLsizei kSize = 6;

    // Create multisampled framebuffer to draw into, with both color and depth attachments.
    GLTexture colorMS;
    glBindTexture(GL_TEXTURE_2D, colorMS);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLRenderbuffer depthStencilMS;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilMS);
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer fboMS;
    glBindFramebuffer(GL_FRAMEBUFFER, fboMS);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         colorMS, 0, 4);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencilMS);
    ASSERT_GL_NO_ERROR();

    // Set up texture for copy operation that breaks the render pass
    GLTexture copyTex;
    glBindTexture(GL_TEXTURE_2D, copyTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set viewport and clear color, depth and stencil
    glViewport(0, 0, kSize, kSize);
    glClearColor(0, 0, 0, 1.0f);
    glClearDepthf(1);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Output depth/stencil, but disable testing so all draw calls succeed
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x55, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilMask(0xFF);

    // Set up program
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Draw red
    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.75f);
    ASSERT_GL_NO_ERROR();

    // Invalidate everything
    const GLenum discards[] = {GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 3, discards);

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw green
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Invalidate everything
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 3, discards);

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, kSize / 2, 0, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw blue
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.25f);
    ASSERT_GL_NO_ERROR();

    // Invalidate everything
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 3, discards);

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, kSize / 2, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Draw yellow
    glUniform4f(colorUniformLocation, 1.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Invalidate everything
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 3, discards);

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, kSize / 2, kSize / 2, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Verify the counters
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_COUNTERS_FOR_UNRESOLVE_RESOLVE_TEST(getPerfCounters(), expected);
}

// Tests counters when uninitialized multisampled-render-to-texture depth/stencil renderbuffers are
// unused but not invalidated.
TEST_P(VulkanPerformanceCounterTest, RenderToTextureUninitializedAndUnusedDepthStencil)
{
    // http://anglebug.com/42263651
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsVulkan());

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_multisampled_render_to_texture"));

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, no depth/stencil clear, load or store.
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 0, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    // Additionally, expect only color resolve.
    setExpectedCountersForUnresolveResolveTest(getPerfCounters(), 0, 0, 0, 1, 0, 0, &expected);

    constexpr GLsizei kSize = 6;

    // Create multisampled framebuffer to draw into, with both color and depth attachments.
    GLTexture colorMS;
    glBindTexture(GL_TEXTURE_2D, colorMS);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLRenderbuffer depthStencilMS;
    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilMS);
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer fboMS;
    glBindFramebuffer(GL_FRAMEBUFFER, fboMS);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         colorMS, 0, 4);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthStencilMS);
    ASSERT_GL_NO_ERROR();

    // Set up texture for copy operation that breaks the render pass
    GLTexture copyTex;
    glBindTexture(GL_TEXTURE_2D, copyTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set viewport and clear color only
    glViewport(0, 0, kSize, kSize);
    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Disable depth/stencil testing.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    // Set up program
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Draw red
    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.75f);
    ASSERT_GL_NO_ERROR();

    // Break the render pass
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, kSize / 2, kSize / 2);
    ASSERT_GL_NO_ERROR();

    // Verify the counters
    EXPECT_DEPTH_STENCIL_LOAD_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_COUNTERS_FOR_UNRESOLVE_RESOLVE_TEST(getPerfCounters(), expected);
}

// Ensures we use read-only depth layout when there is no write
TEST_P(VulkanPerformanceCounterTest, ReadOnlyDepthBufferLayout)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr GLsizei kSize = 64;

    angle::VulkanPerfCounters expected;

    // Create depth only FBO and fill depth texture to leftHalf=0.0 and rightHalf=1.0. This should
    // use writeable layout
    expected.readOnlyDepthStencilRenderPasses = getPerfCounters().readOnlyDepthStencilRenderPasses;

    // Expect rpCount+1, depth(Clears+0, Loads+0, LoadNones+0, Stores+1, StoreNones+0),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 0, 0, 1, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLFramebuffer depthOnlyFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, depthOnlyFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_TRUE);
    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glViewport(0, 0, kSize / 2, kSize);
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.0f);
    glViewport(kSize / 2, 0, kSize / 2, kSize);
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 1.0f);
    glViewport(0, 0, kSize, kSize);
    ASSERT_GL_NO_ERROR();

    // Because the layout counter is updated at end of renderpass, we need to issue a finish call
    // here to end the renderpass.
    glFinish();

    uint64_t actualReadOnlyDepthStencilCount = getPerfCounters().readOnlyDepthStencilRenderPasses;
    EXPECT_EQ(expected.readOnlyDepthStencilRenderPasses, actualReadOnlyDepthStencilCount);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Create a color+depth FBO and use depth as read only. This should use read only layout
    ++expected.readOnlyDepthStencilRenderPasses;
    // Expect rpCount+1, depth(Clears+0, Loads+1, LoadNones+0, Stores+0, StoreNones+1),
    // stencil(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 1, 0, 0, 1, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 0, 0, 0, &expected);

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    GLFramebuffer depthAndColorFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, depthAndColorFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Clear color to blue and draw a green quad with depth=0.5
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);

    angle::Vector4 clearColor = GLColor::blue.toNormalizedVector();
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    // The pixel check will end renderpass.
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(1 + kSize / 2, 1, GLColor::red);
    actualReadOnlyDepthStencilCount = getPerfCounters().readOnlyDepthStencilRenderPasses;
    EXPECT_EQ(expected.readOnlyDepthStencilRenderPasses, actualReadOnlyDepthStencilCount);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
}

// Ensures depth/stencil is not loaded after storeOp=DONT_CARE due to optimization (as opposed to
// invalidate)
TEST_P(VulkanPerformanceCounterTest, RenderPassAfterRenderPassWithoutDepthStencilWrite)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1),
    // stencil(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 0, 1, 0, 1, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 1, 0, 1, &expected);

    constexpr GLsizei kSize = 64;

    // Create FBO with color, depth and stencil.  Leave depth/stencil uninitialized.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              renderbuffer);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    // Draw to the FBO, without enabling depth/stencil.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorUniformLocation);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, kSize, kSize);
    glUniform4f(colorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);
    drawQuad(program, essl1_shaders::PositionAttrib(), 1.0f);

    // Break the render pass and ensure no depth/stencil load/store was done.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    // Expect rpCount+1, depth(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1),
    // stencil(Clears+0, Loads+0, LoadNones+1, Stores+0, StoreNones+1)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 0, 1, 0, 1, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 0, 0, 1, 0, 1, &expected);

    // Draw again with similar conditions, and again make sure no load/store is done.
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(program, essl1_shaders::PositionAttrib(), 1.0f);

    // Break the render pass and ensure no depth/stencil load/store was done.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Ensures repeated clears of various kind (all attachments, some attachments, scissored, masked
// etc) don't break the render pass.
TEST_P(VulkanPerformanceCounterTest, ClearAfterClearDoesNotBreakRenderPass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    constexpr GLsizei kSize = 6;

    // Create a framebuffer to clear with both color and depth/stencil attachments.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLTexture depth;
    glBindTexture(GL_TEXTURE_2D, depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, kSize, kSize, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8_OES, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
    ASSERT_GL_NO_ERROR();

    // Clear color and depth, but not stencil.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepthf(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Clear color and stencil, but not depth.
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClearStencil(0x11);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Clear depth and stencil, but not color.
    glClearDepthf(0.1f);
    glClearStencil(0x22);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Clear masked color, and unmasked depth.
    glClearDepthf(0.2f);
    glClearColor(0.1f, 1.0f, 0.0f, 1.0f);
    glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Clear unmasked color, and masked stencil.
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearStencil(0x33);
    glStencilMask(0xF0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Clear unmasked depth and stencil.
    glClearDepthf(0.3f);
    glClearStencil(0x44);
    glStencilMask(0xFF);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Clear with scissor.
    glEnable(GL_SCISSOR_TEST);
    glScissor(kSize / 3, kSize / 3, kSize / 3, kSize / 3);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClearStencil(0x55);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Verify render pass count.
    EXPECT_EQ(getPerfCounters().renderPasses, expectedRenderPassCount);

    // Make sure the result is correct.  The border of the image should be blue with depth 0.3f and
    // stencil 0x44.  The center is red with depth 1.0f and stencil 0x55.

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::blue);

    EXPECT_PIXEL_COLOR_EQ(kSize / 3, kSize / 3, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(2 * kSize / 3 - 1, kSize / 3, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize / 3, 2 * kSize / 3 - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(2 * kSize / 3 - 1, 2 * kSize / 3 - 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::red);

    glViewport(0, 0, kSize, kSize);
    glDisable(GL_SCISSOR_TEST);

    // Center: If depth is not cleared to 1, rendering would fail.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Center: If stencil is not clear to 0x55, rendering would fail.
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0x55, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);

    // Set up program
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Draw green
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.95f);
    ASSERT_GL_NO_ERROR();

    // Verify that only the center has changed
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::blue);

    EXPECT_PIXEL_COLOR_EQ(kSize / 3, kSize / 3, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(2 * kSize / 3 - 1, kSize / 3, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize / 3, 2 * kSize / 3 - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(2 * kSize / 3 - 1, 2 * kSize / 3 - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::green);

    // Border: If depth is not cleared to 0.3f, rendering would fail.
    glDepthFunc(GL_LESS);

    // Center: If stencil is not clear to 0x44, rendering would fail.
    glStencilFunc(GL_EQUAL, 0x44, 0xFF);

    // Draw yellow
    glUniform4f(colorUniformLocation, 1.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), -0.5f);
    ASSERT_GL_NO_ERROR();

    // Verify that only the border has changed
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, 0, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(0, kSize - 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(kSize - 1, kSize - 1, GLColor::yellow);

    EXPECT_PIXEL_COLOR_EQ(kSize / 3, kSize / 3, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(2 * kSize / 3 - 1, kSize / 3, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize / 3, 2 * kSize / 3 - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(2 * kSize / 3 - 1, 2 * kSize / 3 - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(kSize / 2, kSize / 2, GLColor::green);
}

// Ensures that changing the scissor size doesn't break the render pass.
TEST_P(VulkanPerformanceCounterTest, ScissorDoesNotBreakRenderPass)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr GLsizei kSize = 16;

    // Create a framebuffer with a color attachment.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_NO_ERROR();

    // First, issue a clear and make sure it's done.  Later we can verify that areas outside
    // scissors are not rendered to.
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    // This test starts with a small scissor and gradually grows it and issues draw calls and
    // various kinds of clears:
    //
    // - Clear the center to red
    //
    //     +----------------------+
    //     | K                    |
    //     |                      |
    //     |       +-----+        |
    //     |       |     |        |
    //     |       |  R  |        |
    //     |       |     |        |
    //     |       +-----+        |
    //     |                      |
    //     |                      |
    //     |                      |
    //     |                      |
    //     +----------------------+
    //
    // - Draw green to center right
    //
    //     +----------------------+
    //     |                      |
    //     |              +-------+
    //     |       +-----+|       |
    //     |       |     ||       |
    //     |       |  R  ||  G    |
    //     |       |     ||       |
    //     |       +-----+|       |
    //     |              |       |
    //     |              |       |
    //     |              |       |
    //     |              +-------+
    //     +----------------------+
    //
    // - Masked clear of center column, only outputting to the blue channel
    //
    //     +----------------------+
    //     |      +---+           |
    //     |      | B |   +-------+
    //     |      |+--+--+|       |
    //     |      ||  |  ||       |
    //     |      ||M |R ||  G    |
    //     |      ||  |  ||       |
    //     |      |+--+--+|       |
    //     |      |   |   |       |
    //     |      |   |   |       |
    //     |      |   |   |       |
    //     |      |   |   +-------+
    //     +------+---+-----------+
    //
    // - Masked draw of center row, only outputting to alpha.
    //
    //     +----------------------+
    //     | K    +---+ K         |
    //     |      | B |   +-------+
    //     |      |+--+--+|       |
    //     |      ||M |R ||   G   |
    //     | +----++--+--++-----+ |
    //     | |    ||TM|TR||     | |
    //     | | TK |+--+--+|  TG | |
    //     | |    |TB |TK |     | |
    //     | +----+---+---+-----+ |
    //     |      |   |   |   G   |
    //     | K    | B | K +-------+
    //     +------+---+-----------+
    //
    // Where: K=Black, R=Red, G=Green, B=Blue, M=Magenta, T=Transparent

    constexpr GLsizei kClearX      = kSize / 3;
    constexpr GLsizei kClearY      = kSize / 3;
    constexpr GLsizei kClearWidth  = kSize / 3;
    constexpr GLsizei kClearHeight = kSize / 3;

    constexpr GLsizei kDrawX      = kClearX + kClearWidth + 2;
    constexpr GLsizei kDrawY      = kSize / 5;
    constexpr GLsizei kDrawWidth  = kSize - kDrawX;
    constexpr GLsizei kDrawHeight = 7 * kSize / 10;

    constexpr GLsizei kMaskedClearX      = kSize / 4;
    constexpr GLsizei kMaskedClearY      = kSize / 8;
    constexpr GLsizei kMaskedClearWidth  = kSize / 4;
    constexpr GLsizei kMaskedClearHeight = 7 * kSize / 8;

    constexpr GLsizei kMaskedDrawX      = kSize / 8;
    constexpr GLsizei kMaskedDrawY      = kSize / 2;
    constexpr GLsizei kMaskedDrawWidth  = 6 * kSize / 8;
    constexpr GLsizei kMaskedDrawHeight = kSize / 4;

    glEnable(GL_SCISSOR_TEST);

    // Clear center to red
    glScissor(kClearX, kClearY, kClearWidth, kClearHeight);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Draw green to center right
    glScissor(kDrawX, kDrawY, kDrawWidth, kDrawHeight);
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0);
    ASSERT_GL_NO_ERROR();

    // Masked blue-channel clear of center column
    glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE);
    glScissor(kMaskedClearX, kMaskedClearY, kMaskedClearWidth, kMaskedClearHeight);
    glClearColor(0.5f, 0.5f, 1.0f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Masked alpha-channel draw of center row
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
    glScissor(kMaskedDrawX, kMaskedDrawY, kMaskedDrawWidth, kMaskedDrawHeight);
    glUniform4f(colorUniformLocation, 0.5f, 0.5f, 0.5f, 0.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0);
    ASSERT_GL_NO_ERROR();

    // Verify render pass count.
    EXPECT_EQ(getPerfCounters().renderPasses, expectedRenderPassCount);

    // Make sure the result is correct:
    //
    //     +----------------------+  <-- 0
    //     | K    +---+ K         |  <-- kMaskedClearY
    //     |      | B |   +-------+  <-- kDrawY
    //     |      |+--+--+|       |  <-- kClearY
    //     |      ||M |R ||   G   |
    //     | +----++--+--++-----+ |  <-- kMaskedDrawY
    //     | |    ||TM|TR||     | |
    //     | | TK |+--+--+|  TG | |  <-- kClearY + kClearHeight
    //     | |    |TB |TK |     | |
    //     | +----+---+---+-----+ |  <-- kMaskedDrawY + kMaskedDrawHeight
    //     |      |   |   |   G   |
    //     | K    | B | K +-------+  <-- kDrawY + kDrawHeight
    //     +------+---+-----------+  <-- kSize == kMaskedClearY + kMaskedClearHeight
    //     | |    ||  |  ||     | |
    //     | |    ||  |  ||     |  \---> kSize == kDrawX + kDrawWidth
    //     | |    ||  |  ||      \-----> kMaskedDrawX + kMaskedDrawWidth
    //     | |    ||  |  | \-----------> kDrawX
    //     | |    ||  |   \------------> kClearX + kClearWidth
    //     | |    ||   \---------------> kMaskedClearX + kMaskedClearWidth
    //     | |    | \------------------> kClearX
    //     | |     \-------------------> kMaskedClearX
    //     |  \------------------------> kMaskedDrawX
    //      \--------------------------> 0

    constexpr GLsizei kClearX2       = kClearX + kClearWidth;
    constexpr GLsizei kClearY2       = kClearY + kClearHeight;
    constexpr GLsizei kDrawX2        = kDrawX + kDrawWidth;
    constexpr GLsizei kDrawY2        = kDrawY + kDrawHeight;
    constexpr GLsizei kMaskedClearX2 = kMaskedClearX + kMaskedClearWidth;
    constexpr GLsizei kMaskedClearY2 = kMaskedClearY + kMaskedClearHeight;
    constexpr GLsizei kMaskedDrawX2  = kMaskedDrawX + kMaskedDrawWidth;
    constexpr GLsizei kMaskedDrawY2  = kMaskedDrawY + kMaskedDrawHeight;

    constexpr GLColor kTransparentRed(255, 0, 0, 0);
    constexpr GLColor kTransparentGreen(0, 255, 0, 0);
    constexpr GLColor kTransparentBlue(0, 0, 255, 0);
    constexpr GLColor kTransparentMagenta(255, 0, 255, 0);

    // Verify the black areas.
    EXPECT_PIXEL_RECT_EQ(0, 0, kMaskedClearX, kMaskedDrawY, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(0, kMaskedDrawY2, kMaskedClearX, kSize - kMaskedDrawY2, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX2, 0, kSize - kMaskedClearX2, kDrawY, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX2, kDrawY2, kSize - kMaskedClearX2, kSize - kDrawY2,
                         GLColor::black);
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX, 0, kMaskedClearWidth, kMaskedClearY, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX2, kDrawY, kDrawX - kMaskedClearX2, kClearY - kDrawY,
                         GLColor::black);
    EXPECT_PIXEL_RECT_EQ(kClearX2, kClearY, kDrawX - kClearX2, kMaskedDrawY - kClearY,
                         GLColor::black);
    EXPECT_PIXEL_RECT_EQ(0, kMaskedDrawY, kMaskedDrawX, kMaskedDrawHeight, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX2, kMaskedDrawY2, kDrawX - kMaskedClearX2,
                         kSize - kMaskedDrawY2, GLColor::black);

    // Verify the red area:
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX2, kClearY, kClearX2 - kMaskedClearX2, kMaskedDrawY - kClearY,
                         GLColor::red);
    // Verify the transparent red area:
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX2, kMaskedDrawY, kClearX2 - kMaskedClearX2,
                         kClearY2 - kMaskedDrawY, kTransparentRed);
    // Verify the magenta area:
    EXPECT_PIXEL_RECT_EQ(kClearX, kClearY, kMaskedClearX2 - kClearX, kMaskedDrawY - kClearY,
                         GLColor::magenta);
    // Verify the transparent magenta area:
    EXPECT_PIXEL_RECT_EQ(kClearX, kMaskedDrawY, kMaskedClearX2 - kClearX, kClearY2 - kMaskedDrawY,
                         kTransparentMagenta);
    // Verify the green area:
    EXPECT_PIXEL_RECT_EQ(kDrawX, kDrawY, kDrawWidth, kMaskedDrawY - kDrawY, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(kDrawX, kMaskedDrawY2, kDrawWidth, kDrawY2 - kMaskedDrawY2,
                         GLColor::green);
    EXPECT_PIXEL_RECT_EQ(kMaskedDrawX2, kMaskedDrawY, kDrawX2 - kMaskedDrawX2, kMaskedDrawHeight,
                         GLColor::green);
    // Verify the transparent green area:
    EXPECT_PIXEL_RECT_EQ(kDrawX, kMaskedDrawY, kMaskedDrawX2 - kDrawX, kMaskedDrawHeight,
                         kTransparentGreen);
    // Verify the blue area:
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX, kMaskedClearY, kMaskedClearWidth, kClearY - kMaskedClearY,
                         GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX, kMaskedDrawY2, kMaskedClearWidth,
                         kMaskedClearY2 - kMaskedDrawY2, GLColor::blue);
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX, kClearY, kClearX - kMaskedClearX, kMaskedDrawY - kClearY,
                         GLColor::blue);
    // Verify the transparent blue area:
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX, kClearY2, kMaskedClearWidth, kMaskedDrawY2 - kClearY2,
                         kTransparentBlue);
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX, kMaskedDrawY, kClearX - kMaskedClearX,
                         kClearY2 - kMaskedDrawY, kTransparentBlue);
    // Verify the transparent black area:
    EXPECT_PIXEL_RECT_EQ(kMaskedDrawX, kMaskedDrawY, kMaskedClearX - kMaskedDrawX,
                         kMaskedDrawHeight, GLColor::transparentBlack);
    EXPECT_PIXEL_RECT_EQ(kMaskedClearX2, kClearY2, kDrawX - kMaskedClearX2,
                         kMaskedDrawY2 - kClearY2, GLColor::transparentBlack);
    EXPECT_PIXEL_RECT_EQ(kClearX2, kMaskedDrawY, kDrawX - kClearX2, kMaskedDrawHeight,
                         GLColor::transparentBlack);
}

// Tests that changing UBO bindings does not allocate new descriptor sets.
TEST_P(VulkanPerformanceCounterTest, ChangingUBOsHitsDescriptorSetCache)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    // Set up two UBOs, one filled with "1" and the second with "2".
    constexpr GLsizei kCount = 64;
    std::vector<GLint> data1(kCount, 1);
    std::vector<GLint> data2(kCount, 2);

    GLBuffer ubo1;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo1);
    glBufferData(GL_UNIFORM_BUFFER, kCount * sizeof(data1[0]), data1.data(), GL_STATIC_DRAW);

    GLBuffer ubo2;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo2);
    glBufferData(GL_UNIFORM_BUFFER, kCount * sizeof(data2[0]), data2.data(), GL_STATIC_DRAW);

    // Set up a program that verifies the contents of uniform blocks.
    constexpr char kVS[] = R"(#version 300 es
precision mediump float;
in vec4 position;
void main()
{
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform buf {
    int data[64/4];
};
uniform int checkValue;
out vec4 outColor;

void main()
{
    for (int i = 0; i < 64/4; ++i) {
        if (data[i] != checkValue) {
            outColor = vec4(1, 0, 0, 1);
            return;
        }
    }
    outColor = vec4(0, 1, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    GLint uniLoc = glGetUniformLocation(program, "checkValue");
    ASSERT_NE(-1, uniLoc);

    GLuint blockIndex = glGetUniformBlockIndex(program, "buf");
    ASSERT_NE(blockIndex, GL_INVALID_INDEX);

    glUniformBlockBinding(program, blockIndex, 0);
    ASSERT_GL_NO_ERROR();

    // Set up the rest of the GL state.
    auto quadVerts = GetQuadVertices();
    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(quadVerts[0]), quadVerts.data(),
                 GL_STATIC_DRAW);

    GLint posLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, posLoc);

    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(posLoc);

    // Draw a few times with each UBO. Stream out one pixel for post-render verification.
    constexpr int kIterations         = 5;
    constexpr GLsizei kPackBufferSize = sizeof(GLColor) * kIterations * 2;

    GLBuffer packBuffer;
    glBindBuffer(GL_PIXEL_PACK_BUFFER, packBuffer);
    glBufferData(GL_PIXEL_PACK_BUFFER, kPackBufferSize, nullptr, GL_STREAM_READ);

    GLsizei offset = 0;

    uint64_t expectedShaderResourcesCacheMisses = 0;

    for (int iteration = 0; iteration < kIterations; ++iteration)
    {
        glUniform1i(uniLoc, 1);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo1);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                     reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(offset)));
        offset += sizeof(GLColor);
        glUniform1i(uniLoc, 2);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo2);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                     reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(offset)));
        offset += sizeof(GLColor);

        // Capture the allocations counter after the first run.
        if (iteration == 0)
        {
            expectedShaderResourcesCacheMisses =
                getPerfCounters().shaderResourcesDescriptorSetCacheMisses;
        }
    }

    ASSERT_GL_NO_ERROR();
    if (isFeatureEnabled(Feature::DescriptorSetCache))
    {
        EXPECT_GT(expectedShaderResourcesCacheMisses, 0u);
    }

    // Verify correctness first.
    std::vector<GLColor> expectedData(kIterations * 2, GLColor::green);
    std::vector<GLColor> actualData(kIterations * 2, GLColor::black);

    void *mapPtr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, kPackBufferSize, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mapPtr);
    memcpy(actualData.data(), mapPtr, kPackBufferSize);

    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

    EXPECT_EQ(expectedData, actualData);

    if (isFeatureEnabled(Feature::DescriptorSetCache))
    {
        // Check for unnecessary descriptor set allocations.
        uint64_t actualShaderResourcesCacheMisses =
            getPerfCounters().shaderResourcesDescriptorSetCacheMisses;
        EXPECT_EQ(expectedShaderResourcesCacheMisses, actualShaderResourcesCacheMisses);
    }
}

// Test that mapping a buffer that the GPU is using as read-only ghosts the buffer, rather than
// waiting for the GPU access to complete before returning a pointer to the buffer.
void VulkanPerformanceCounterTest::mappingGpuReadOnlyBufferGhostsBuffer(BufferUpdate update)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    // 1. Create a buffer, map it, fill it with red
    // 2. Draw with buffer (GPU read-only)
    // 3. Map the same buffer and fill with white
    //    - This should ghost the buffer, rather than ending the render pass.
    // 4. Draw with buffer
    // 5. Update the buffer with glBufferSubData() or glCopyBufferSubData
    // 6. Draw with the buffer
    // The render pass should only be broken (counters.renderPasses == 0) due to the glReadPixels()
    // to verify the draw at the end.

    const std::array<GLColor, 4> kInitialData = {GLColor::red, GLColor::red, GLColor::red,
                                                 GLColor::red};
    const std::array<GLColor, 4> kUpdateData1 = {GLColor::white, GLColor::white, GLColor::white,
                                                 GLColor::white};
    const std::array<GLColor, 4> kUpdateData2 = {GLColor::blue, GLColor::blue, GLColor::blue,
                                                 GLColor::blue};

    GLBuffer buffer;
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(kInitialData), kInitialData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);
    ASSERT_GL_NO_ERROR();

    // Draw
    constexpr char kVerifyUBO[] = R"(#version 300 es
precision mediump float;
uniform block {
    uvec4 data;
} ubo;
uniform uint expect;
uniform vec4 successOutput;
out vec4 colorOut;
void main()
{
    if (all(equal(ubo.data, uvec4(expect))))
        colorOut = successOutput;
    else
        colorOut = vec4(1.0, 0, 0, 1.0);
})";

    ANGLE_GL_PROGRAM(verifyUbo, essl3_shaders::vs::Simple(), kVerifyUBO);
    glUseProgram(verifyUbo);

    GLint expectLoc = glGetUniformLocation(verifyUbo, "expect");
    ASSERT_NE(-1, expectLoc);
    GLint successLoc = glGetUniformLocation(verifyUbo, "successOutput");
    ASSERT_NE(-1, successLoc);

    glUniform1ui(expectLoc, kInitialData[0].asUint());
    glUniform4f(successLoc, 0, 1, 0, 1);

    drawQuad(verifyUbo, essl3_shaders::PositionAttrib(), 0.5);
    ASSERT_GL_NO_ERROR();

    // Map the buffer and update it.
    // This should ghost the buffer and avoid breaking the render pass, since the GPU is only
    // reading it.
    void *mappedBuffer =
        glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(kInitialData), GL_MAP_WRITE_BIT);
    // 'renderPasses == 0' here means the render pass was broken and a new one was started.
    ASSERT_EQ(getPerfCounters().renderPasses, 1u);
    ASSERT_EQ(getPerfCounters().buffersGhosted, 1u);

    memcpy(mappedBuffer, kUpdateData1.data(), sizeof(kInitialData));

    glUnmapBuffer(GL_UNIFORM_BUFFER);
    ASSERT_GL_NO_ERROR();

    // Verify that the buffer has the updated value.
    glUniform1ui(expectLoc, kUpdateData1[0].asUint());
    glUniform4f(successLoc, 0, 0, 1, 1);

    drawQuad(verifyUbo, essl3_shaders::PositionAttrib(), 0.5);
    ASSERT_GL_NO_ERROR();
    ASSERT_EQ(getPerfCounters().renderPasses, 1u);

    // Update the buffer
    updateBuffer(update, GL_UNIFORM_BUFFER, 0, sizeof(kUpdateData2), kUpdateData2.data());
    ASSERT_GL_NO_ERROR();
    ASSERT_EQ(getPerfCounters().renderPasses, 1u);

    // Verify that the buffer has the updated value.
    glUniform1ui(expectLoc, kUpdateData2[0].asUint());
    glUniform4f(successLoc, 0, 1, 1, 1);

    drawQuad(verifyUbo, essl3_shaders::PositionAttrib(), 0.5);
    ASSERT_GL_NO_ERROR();
    ASSERT_EQ(getPerfCounters().renderPasses, 1u);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::cyan);
}

// Test that mapping a buffer that the GPU is using as read-only ghosts the buffer, rather than
// waiting for the GPU access to complete before returning a pointer to the buffer.  This test uses
// glBufferSubData to update the buffer.
TEST_P(VulkanPerformanceCounterTest, MappingGpuReadOnlyBufferGhostsBuffer_SubData)
{
    mappingGpuReadOnlyBufferGhostsBuffer(BufferUpdate::SubData);
}

// Same as MappingGpuReadOnlyBufferGhostsBuffer_SubData, but using glCopyBufferSubData to update the
// buffer.
TEST_P(VulkanPerformanceCounterTest, MappingGpuReadOnlyBufferGhostsBuffer_Copy)
{
    mappingGpuReadOnlyBufferGhostsBuffer(BufferUpdate::Copy);
}

void VulkanPerformanceCounterTest::partialBufferUpdateShouldNotBreakRenderPass(BufferUpdate update)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;
    if (!hasPreferCPUForBufferSubData())
    {
        ++expectedRenderPassCount;
    }

    const std::array<GLColor, 4> kInitialData = {GLColor::red, GLColor::green, GLColor::blue,
                                                 GLColor::yellow};
    const std::array<GLColor, 1> kUpdateData1 = {GLColor::cyan};
    const std::array<GLColor, 3> kUpdateData2 = {GLColor::magenta, GLColor::black, GLColor::white};

    GLBuffer buffer;
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(kInitialData), kInitialData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);
    ASSERT_GL_NO_ERROR();

    constexpr char kVS[] = R"(#version 300 es
precision highp float;
uniform float height;
void main()
{
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, 0, 1);
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 colorOut;
uniform block {
    uvec4 data;
} ubo;
uniform uvec4 expect;
uniform vec4 successColor;
void main()
{
    if (all(equal(ubo.data, expect)))
        colorOut = successColor;
    else
        colorOut = vec4(0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLint expectLoc = glGetUniformLocation(program, "expect");
    ASSERT_NE(-1, expectLoc);
    GLint successLoc = glGetUniformLocation(program, "successColor");
    ASSERT_NE(-1, successLoc);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // Draw once, using the buffer in the render pass.
    glUniform4ui(expectLoc, kInitialData[0].asUint(), kInitialData[1].asUint(),
                 kInitialData[2].asUint(), kInitialData[3].asUint());
    glUniform4f(successLoc, 1, 0, 0, 1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Upload a small part of the buffer, and draw again.
    updateBuffer(update, GL_UNIFORM_BUFFER, 0, sizeof(kUpdateData1), kUpdateData1.data());

    glUniform4ui(expectLoc, kUpdateData1[0].asUint(), kInitialData[1].asUint(),
                 kInitialData[2].asUint(), kInitialData[3].asUint());
    glUniform4f(successLoc, 0, 1, 0, 1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Upload a large part of the buffer, and draw again.
    updateBuffer(update, GL_UNIFORM_BUFFER, sizeof(kUpdateData1), sizeof(kUpdateData2),
                 kUpdateData2.data());

    glUniform4ui(expectLoc, kUpdateData1[0].asUint(), kUpdateData2[0].asUint(),
                 kUpdateData2[1].asUint(), kUpdateData2[2].asUint());
    glUniform4f(successLoc, 0, 0, 1, 1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Verify results
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
    ASSERT_GL_NO_ERROR();

    // Only one render pass should have been used.
    EXPECT_EQ(getPerfCounters().renderPasses, expectedRenderPassCount);
}

// Verifies that BufferSubData calls don't cause a render pass break when it only uses the buffer
// read-only.  This test uses glBufferSubData to update the buffer.
TEST_P(VulkanPerformanceCounterTest, PartialBufferUpdateShouldNotBreakRenderPass_SubData)
{
    partialBufferUpdateShouldNotBreakRenderPass(BufferUpdate::SubData);
}

// Same as PartialBufferUpdateShouldNotBreakRenderPass_SubData, but using glCopyBufferSubData to
// update the buffer.
TEST_P(VulkanPerformanceCounterTest, PartialBufferUpdateShouldNotBreakRenderPass_Copy)
{
    partialBufferUpdateShouldNotBreakRenderPass(BufferUpdate::Copy);
}

void VulkanPerformanceCounterTest::bufferSubDataShouldNotTriggerSyncState(BufferUpdate update)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    glUseProgram(testProgram);

    GLint posLoc = glGetAttribLocation(testProgram, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, posLoc);

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    EXPECT_EQ(getPerfCounters().vertexArraySyncStateCalls, 1u);

    const std::array<Vector3, 6> &quadVertices = GetQuadVertices();
    size_t bufferSize                          = sizeof(quadVertices[0]) * quadVertices.size();

    updateBuffer(update, GL_ARRAY_BUFFER, 0, bufferSize, quadVertices.data());

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    EXPECT_EQ(getPerfCounters().vertexArraySyncStateCalls, 1u);

    // Verify the BufferData with a whole buffer size is treated like the SubData call.
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices[0]) * quadVertices.size(),
                 quadVertices.data(), GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    EXPECT_EQ(getPerfCounters().vertexArraySyncStateCalls, 1u);
}

// Verifies that BufferSubData calls don't trigger state updates for non-translated formats.  This
// test uses glBufferSubData to update the buffer.
TEST_P(VulkanPerformanceCounterTest, BufferSubDataShouldNotTriggerSyncState_SubData)
{
    bufferSubDataShouldNotTriggerSyncState(BufferUpdate::SubData);
}

// Same as BufferSubDataShouldNotTriggerSyncState_SubData, but using glCopyBufferSubData to update
// the buffer.
TEST_P(VulkanPerformanceCounterTest, BufferSubDataShouldNotTriggerSyncState_Copy)
{
    bufferSubDataShouldNotTriggerSyncState(BufferUpdate::Copy);
}

// Verifies that rendering to backbuffer discards depth/stencil.
TEST_P(VulkanPerformanceCounterTest, SwapShouldInvalidateDepthStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    // stencil(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 1, 0, 0, 0, 0, &expected);

    // Clear to verify that _some_ counters did change (as opposed to for example all being reset on
    // swap)
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x00, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Swap buffers to implicitely resolve
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
}

// Verifies that rendering to MSAA backbuffer discards depth/stencil.
TEST_P(VulkanPerformanceCounterTest_MSAA, SwapShouldInvalidateDepthStencil)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    // stencil(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 1, 0, 0, 0, 0, &expected);
    setExpectedCountersForStencilOps(getPerfCounters(), 1, 0, 0, 0, 0, &expected);

    // Clear to verify that _some_ counters did change (as opposed to for example all being reset on
    // swap)
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0x00, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Swap buffers to implicitely resolve
    swapBuffers();
    EXPECT_DEPTH_STENCIL_OP_COUNTERS(getPerfCounters(), expected);
}

// Verifies that multisample swapchain resolve occurs in subpass.
TEST_P(VulkanPerformanceCounterTest_MSAA, SwapShouldResolveWithSubpass)
{
    angle::VulkanPerfCounters expected;
    // Expect rpCount+1, color(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);

    uint64_t expectedResolvesSubpass = getPerfCounters().swapchainResolveInSubpass + 1;
    uint64_t expectedResolvesOutside = getPerfCounters().swapchainResolveOutsideSubpass;

    // Clear color.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up program
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Draw green
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.95f);
    ASSERT_GL_NO_ERROR();

    // Swap buffers to implicitly resolve
    swapBuffers();
    EXPECT_EQ(getPerfCounters().swapchainResolveInSubpass, expectedResolvesSubpass);
    EXPECT_EQ(getPerfCounters().swapchainResolveOutsideSubpass, expectedResolvesOutside);

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Verifies that in a multisample swapchain, drawing to the default FBO followed by user FBO and
// then swapping triggers the resolve outside the optimization subpass.
TEST_P(VulkanPerformanceCounterTest_MSAA, SwapAfterDrawToDifferentFBOsShouldResolveOutsideSubpass)
{
    constexpr GLsizei kSize = 16;
    angle::VulkanPerfCounters expected;
    // Expect rpCount+1, color(Clears+1, Loads+0, LoadNones+0, Stores+2, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 1, 0, 0, 2, 0, &expected);

    uint64_t expectedResolvesSubpass = getPerfCounters().swapchainResolveInSubpass;
    uint64_t expectedResolvesOutside = getPerfCounters().swapchainResolveOutsideSubpass + 1;

    // Create a framebuffer to clear.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    ASSERT_GL_NO_ERROR();

    // Clear color.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up program
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Draw green to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.95f);
    ASSERT_GL_NO_ERROR();

    // Draw blue to the user framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.95f);
    ASSERT_GL_NO_ERROR();

    // Swap buffers to resolve outside the optimization subpass
    swapBuffers();
    EXPECT_EQ(getPerfCounters().swapchainResolveInSubpass, expectedResolvesSubpass);
    EXPECT_EQ(getPerfCounters().swapchainResolveOutsideSubpass, expectedResolvesOutside);

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Verifies that in a multisample swapchain, subpass resolve only happens when the render pass
// covers the entire area.
TEST_P(VulkanPerformanceCounterTest_MSAA, ResolveWhenRenderPassNotEntireArea)
{
    constexpr GLsizei kSize = 16;
    angle::VulkanPerfCounters expected;
    // Expect rpCount+1, color(Clears+0, Loads+1, LoadNones+0, Stores+2, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 1, 0, 2, 0, &expected);

    uint64_t expectedResolvesSubpass = getPerfCounters().swapchainResolveInSubpass;
    uint64_t expectedResolvesOutside = getPerfCounters().swapchainResolveOutsideSubpass + 1;

    // Create a framebuffer to clear.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ASSERT_GL_NO_ERROR();

    // Clear color.
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // Set up program
    ANGLE_GL_PROGRAM(drawColor, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(drawColor);
    GLint colorUniformLocation =
        glGetUniformLocation(drawColor, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(colorUniformLocation, -1);

    // Scissor the render area
    glEnable(GL_SCISSOR_TEST);
    glScissor(kSize / 4, kSize / 4, kSize / 2, kSize / 2);

    // Draw blue to the user framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUniform4f(colorUniformLocation, 0.0f, 0.0f, 1.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.95f);
    ASSERT_GL_NO_ERROR();

    // Draw green to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUniform4f(colorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
    drawQuad(drawColor, essl1_shaders::PositionAttrib(), 0.95f);
    ASSERT_GL_NO_ERROR();

    // Swap buffers, which should not resolve the image in subpass
    swapBuffers();
    EXPECT_EQ(getPerfCounters().swapchainResolveInSubpass, expectedResolvesSubpass);
    EXPECT_EQ(getPerfCounters().swapchainResolveOutsideSubpass, expectedResolvesOutside);

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that uniform updates eventually stop updating descriptor sets.
TEST_P(VulkanPerformanceCounterTest, UniformUpdatesHitDescriptorSetCache)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
    glUseProgram(testProgram);
    GLint posLoc = glGetAttribLocation(testProgram, essl1_shaders::PositionAttrib());
    GLint uniLoc = glGetUniformLocation(testProgram, essl1_shaders::ColorUniform());

    std::array<Vector3, 6> quadVerts = GetQuadVertices();

    GLBuffer vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(quadVerts[0]), quadVerts.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);

    ASSERT_GL_NO_ERROR();

    // Choose a number of iterations sufficiently large to ensure all uniforms are cached.
    constexpr int kIterations = 2000;

    // First pass: cache all the uniforms.
    RNG rng;
    for (int iteration = 0; iteration < kIterations; ++iteration)
    {
        Vector3 randomVec3 = RandomVec3(rng.randomInt(), 0.0f, 1.0f);

        glUniform4f(uniLoc, randomVec3.x(), randomVec3.y(), randomVec3.z(), 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        GLColor expectedColor = GLColor(randomVec3);
        EXPECT_PIXEL_COLOR_NEAR(0, 0, expectedColor, 5);
    }

    ASSERT_GL_NO_ERROR();

    uint64_t expectedCacheMisses = getPerfCounters().uniformsAndXfbDescriptorSetCacheMisses;
    GLint allocationsBefore      = getPerfCounters().descriptorSetAllocations;
    if (isFeatureEnabled(Feature::DescriptorSetCache))
    {
        EXPECT_GT(expectedCacheMisses, 0u);
    }

    // Second pass: ensure all the uniforms are cached.
    for (int iteration = 0; iteration < kIterations; ++iteration)
    {
        Vector3 randomVec3 = RandomVec3(rng.randomInt(), 0.0f, 1.0f);

        glUniform4f(uniLoc, randomVec3.x(), randomVec3.y(), randomVec3.z(), 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        GLColor expectedColor = GLColor(randomVec3);
        EXPECT_PIXEL_COLOR_NEAR(0, 0, expectedColor, 5);
    }

    ASSERT_GL_NO_ERROR();

    if (isFeatureEnabled(Feature::DescriptorSetCache))
    {
        uint64_t actualCacheMisses = getPerfCounters().uniformsAndXfbDescriptorSetCacheMisses;
        EXPECT_EQ(expectedCacheMisses, actualCacheMisses);
    }
    else
    {
        // If cache is disabled, we still expect descriptorSets to be reused instead of keep
        // allocating new descriptorSets. The underline reuse logic is implementation detail (as of
        // now it will not reuse util the pool is full), but we expect it will not increasing for
        // every iteration.
        GLint descriptorSetAllocationsIncrease =
            getPerfCounters().descriptorSetAllocations - allocationsBefore;
        EXPECT_GE(kIterations - 1, descriptorSetAllocationsIncrease);
    }
}

// Test one texture sampled by fragment shader, then image load it by compute
// shader, at last fragment shader do something else.
TEST_P(VulkanPerformanceCounterTest_ES31, DrawDispatchImageReadDrawWithEndRP)
{

    constexpr char kVSSource[] = R"(#version 310 es
in vec4 a_position;
out vec2 v_texCoord;

void main()
{
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
    v_texCoord = a_position.xy * 0.5 + vec2(0.5);
})";

    constexpr char kFSSource[] = R"(#version 310 es
precision mediump float;
uniform sampler2D u_tex2D;
in vec2 v_texCoord;
out vec4 out_FragColor;
void main()
{
    out_FragColor = texture(u_tex2D, v_texCoord);
})";

    constexpr char kFSSource1[] = R"(#version 310 es
precision mediump float;
out vec4 out_FragColor;
void main()
{
    out_FragColor = vec4(1.0);
})";

    constexpr char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba32f, binding=0) readonly  uniform highp image2D uIn;
layout(std140, binding=0) buffer buf {
    vec4 outData;
};

void main()
{
    outData = imageLoad(uIn, ivec2(gl_LocalInvocationID.xy));
})";

    GLfloat initValue[4] = {1.0, 1.0, 1.0, 1.0};

    // Step 1: Set up a simple 2D Texture rendering loop.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, initValue);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLBuffer vertexBuffer;
    GLfloat vertices[] = {-1, -1, 1, -1, -1, 1, 1, 1};
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, kVSSource, kFSSource);
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, "a_position");
    ASSERT_NE(-1, posLoc);

    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);
    ASSERT_GL_NO_ERROR();
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    GLBuffer ssbo;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 16, nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // This is actually suboptimal, and ideally only one render pass should be necessary.
    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 2;

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Step 2: load this image through compute
    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glUseProgram(csProgram);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    const GLfloat *ptr = reinterpret_cast<const GLfloat *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 16, GL_MAP_READ_BIT));

    EXPECT_GL_NO_ERROR();
    for (unsigned int idx = 0; idx < 4; idx++)
    {
        EXPECT_EQ(1.0, *(ptr + idx));
    }

    // Step3
    ANGLE_GL_PROGRAM(program2, kVSSource, kFSSource1);
    glUseProgram(program2);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Test one texture sampled by fragment shader, followed by glReadPixels, then image
// load it by compute shader, and at last fragment shader do something else.
TEST_P(VulkanPerformanceCounterTest_ES31, DrawDispatchImageReadDrawWithoutEndRP)
{

    constexpr char kVSSource[] = R"(#version 310 es
in vec4 a_position;
out vec2 v_texCoord;

void main()
{
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
    v_texCoord = a_position.xy * 0.5 + vec2(0.5);
})";

    constexpr char kFSSource[] = R"(#version 310 es
precision mediump float;
uniform sampler2D u_tex2D;
in vec2 v_texCoord;
out vec4 out_FragColor;
void main()
{
    out_FragColor = texture(u_tex2D, v_texCoord);
})";

    constexpr char kFSSource1[] = R"(#version 310 es
precision mediump float;
out vec4 out_FragColor;
void main()
{
    out_FragColor = vec4(1.0);
})";

    constexpr char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba32f, binding=0) readonly  uniform highp image2D uIn;
layout(std140, binding=0) buffer buf {
    vec4 outData;
};

void main()
{
    outData = imageLoad(uIn, ivec2(gl_LocalInvocationID.xy));
})";

    GLfloat initValue[4] = {1.0, 1.0, 1.0, 1.0};

    // Step 1: Set up a simple 2D Texture rendering loop.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, initValue);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLBuffer vertexBuffer;
    GLfloat vertices[] = {-1, -1, 1, -1, -1, 1, 1, 1};
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    GLBuffer ssbo;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 16, nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    ANGLE_GL_PROGRAM(program, kVSSource, kFSSource);
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, "a_position");
    ASSERT_NE(-1, posLoc);

    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Call glReadPixels to reset the getPerfCounters().renderPasses
    std::vector<GLColor> actualColors(1);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, actualColors.data());

    // Ideally, the following "FS sample + CS image load + FS something", should
    // handle in one render pass.
    // Currently, we can ensure the first of "FS sample + CS image load" in one
    // render pass, but will start new render pass if following the last FS operations,
    // which need to be optimized further.
    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 2;

    // Now this texture owns none layout transition
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Step 2: load this image through compute
    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glUseProgram(csProgram);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Step3
    ANGLE_GL_PROGRAM(program2, kVSSource, kFSSource1);
    glUseProgram(program2);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Test that one texture sampled by fragment shader, compute shader and fragment
// shader sequentlly.
TEST_P(VulkanPerformanceCounterTest_ES31, TextureSampleByDrawDispatchDraw)
{
    constexpr char kVSSource[] = R"(#version 310 es
in vec4 a_position;
out vec2 v_texCoord;

void main()
{
gl_Position = vec4(a_position.xy, 0.0, 1.0);
v_texCoord = a_position.xy * 0.5 + vec2(0.5);
})";

    constexpr char kFSSource[] = R"(#version 310 es
uniform sampler2D u_tex2D;
precision highp float;
in vec2 v_texCoord;
out vec4 out_FragColor;
void main()
{
out_FragColor = texture(u_tex2D, v_texCoord);
})";

    constexpr char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
precision highp sampler2D;
uniform sampler2D tex;
layout(std140, binding=0) buffer buf {
vec4 outData;
};
void main()
{
uint x = gl_LocalInvocationID.x;
uint y = gl_LocalInvocationID.y;
outData = texture(tex, vec2(x, y));
})";

    GLfloat initValue[4] = {1.0, 1.0, 1.0, 1.0};

    // Step 1: Set up a simple 2D Texture rendering loop.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, initValue);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLfloat vertices[] = {-1, -1, 1, -1, -1, 1, 1, 1};

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, kVSSource, kFSSource);
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, "a_position");
    ASSERT_NE(-1, posLoc);

    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ASSERT_GL_NO_ERROR();
    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    // Step 2: sample this texture through compute
    GLBuffer ssbo;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 16, nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glUseProgram(csProgram);

    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(csProgram, "tex"), 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Step3: use the first program sample texture again
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ASSERT_GL_NO_ERROR();

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Verify a mid-render pass clear of a newly enabled attachment uses LOAD_OP_CLEAR.
TEST_P(VulkanPerformanceCounterTest, DisableThenMidRenderPassClear)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    // This optimization is not implemented when this workaround is in effect.
    ANGLE_SKIP_TEST_IF(hasPreferDrawOverClearAttachments());

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+1, Loads+0, LoadNones+0, Stores+2, StoreNones+0)
    // vkCmdClearAttachments should be used for color attachment 0.
    setExpectedCountersForColorOps(getPerfCounters(), 1, 1, 0, 0, 2, 0, &expected);
    expected.colorClearAttachments = getPerfCounters().colorClearAttachments + 1;

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLTexture textures[2];

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textures[1], 0);

    // Only enable attachment 0.
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_NONE};
    glDrawBuffers(2, drawBuffers);

    // Draw red.
    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Enable attachment 1.
    drawBuffers[1] = GL_COLOR_ATTACHMENT1;
    glDrawBuffers(2, drawBuffers);

    // Clear both attachments to green.
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
layout(location = 0) out vec4 my_FragColor0;
layout(location = 1) out vec4 my_FragColor1;
void main()
{
    my_FragColor0 = vec4(0.0, 0.0, 1.0, 1.0);
    my_FragColor1 = vec4(0.0, 0.0, 1.0, 1.0);
})";

    // Draw blue to both attachments.
    ANGLE_GL_PROGRAM(blueProgram, essl3_shaders::vs::Simple(), kFS);
    drawQuad(blueProgram, essl3_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Verify attachment 0.
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 255, 255);
    // Verify attachment 1.
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 255, 255);

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_CLEAR_ATTACHMENTS_COUNTER(expected.colorClearAttachments,
                                     getPerfCounters().colorClearAttachments);

    // Expect rpCount+1, color(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 1, 0, 1, 0, &expected);

    GLFramebuffer fbo2;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // Blend red
    drawQuad(redProgram, essl3_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Verify purple
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 255, 255);
    ASSERT_GL_NO_ERROR();

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Copy of ClearTest.InceptionScissorClears.
// Clears many small concentric rectangles using scissor regions. Verifies vkCmdClearAttachments()
// is used for the scissored clears, rather than vkCmdDraw().
TEST_P(VulkanPerformanceCounterTest, InceptionScissorClears)
{
    // https://issuetracker.google.com/166809097
    ANGLE_SKIP_TEST_IF(IsQualcomm() && IsVulkan());

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);

    angle::RNG rng;

    constexpr GLuint kSize = 16;

    // Create a square user FBO so we have more control over the dimensions.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glViewport(0, 0, kSize, kSize);

    // Clear small concentric squares using scissor.
    std::vector<GLColor> expectedColors;
    // TODO(syoussefi): verify this
    [[maybe_unused]] uint64_t numScissoredClears = 0;
    for (GLuint index = 0; index < (kSize - 1) / 2; index++)
    {
        // Do the first clear without the scissor.
        if (index > 0)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(index, index, kSize - (index * 2), kSize - (index * 2));
            ++numScissoredClears;
        }

        GLColor color = RandomColor(&rng);
        expectedColors.push_back(color);
        Vector4 floatColor = color.toNormalizedVector();
        glClearColor(floatColor[0], floatColor[1], floatColor[2], floatColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    ASSERT_GL_NO_ERROR();

    // Make sure everything was done in a single renderpass.
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    std::vector<GLColor> actualColors(expectedColors.size());
    glReadPixels(0, kSize / 2, actualColors.size(), 1, GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());

    EXPECT_EQ(expectedColors, actualColors);

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Copy of ClearTest.Depth16Scissored.
// Clears many small concentric rectangles using scissor regions. Verifies vkCmdClearAttachments()
// is used for the scissored clears, rather than vkCmdDraw().
TEST_P(VulkanPerformanceCounterTest, Depth16Scissored)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    constexpr int kRenderbufferSize = 64;
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kRenderbufferSize,
                          kRenderbufferSize);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

    glClearDepthf(0.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    constexpr int kNumSteps = 13;
    // TODO(syoussefi): verify this
    [[maybe_unused]] uint64_t numScissoredClears = 0;
    for (int ndx = 1; ndx < kNumSteps; ndx++)
    {
        float perc = static_cast<float>(ndx) / static_cast<float>(kNumSteps);
        glScissor(0, 0, static_cast<int>(kRenderbufferSize * perc),
                  static_cast<int>(kRenderbufferSize * perc));
        glClearDepthf(perc);
        glClear(GL_DEPTH_BUFFER_BIT);
        ++numScissoredClears;
    }

    // Make sure everything was done in a single renderpass.
    EXPECT_EQ(getPerfCounters().renderPasses, 1u);
}

// Copy of ClearTest.InceptionScissorClears.
// Clears many small concentric rectangles using scissor regions.
TEST_P(VulkanPerformanceCounterTest, DrawThenInceptionScissorClears)
{
    // https://issuetracker.google.com/166809097
    ANGLE_SKIP_TEST_IF(IsQualcomm() && IsVulkan());

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, color(Clears+0, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 0, 0, 1, 0, &expected);

    angle::RNG rng;
    std::vector<GLColor> expectedColors;
    constexpr GLuint kSize = 16;

    // Create a square user FBO so we have more control over the dimensions.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glViewport(0, 0, kSize, kSize);

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    expectedColors.push_back(GLColor::red);

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);

    // Expect rpCount+1, color(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0)
    // TODO: Optimize scissored clears to use loadOp = CLEAR. anglebug.com/42263754
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 1, 0, 1, 0, &expected);

    // Draw small concentric squares using scissor.
    // TODO(syoussefi): verify this
    [[maybe_unused]] uint64_t numScissoredClears = 0;
    // All clears are to a scissored render area.
    for (GLuint index = 1; index < (kSize - 1) / 2; index++)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(index, index, kSize - (index * 2), kSize - (index * 2));
        ++numScissoredClears;

        GLColor color = RandomColor(&rng);
        expectedColors.push_back(color);
        Vector4 floatColor = color.toNormalizedVector();
        glClearColor(floatColor[0], floatColor[1], floatColor[2], floatColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    ASSERT_GL_NO_ERROR();

    // Make sure everything was done in a single renderpass.
    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Close the render pass to update the performance counters.
    std::vector<GLColor> actualColors(expectedColors.size());
    glReadPixels(0, kSize / 2, actualColors.size(), 1, GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());
    EXPECT_EQ(expectedColors, actualColors);

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Test that color clears are respected after invalidate
TEST_P(VulkanPerformanceCounterTest, ColorClearAfterInvalidate)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupForColorDepthOpsTest(&framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:

    // color+depth invalidate, color+depth clear
    //
    // Expected:
    //   rpCount+1,
    //   depth(Clears+1, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    //   color(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 0, 1, 0, 0, 0, 0, &expected);
    setExpectedCountersForColorOps(getPerfCounters(), 1, 1, 0, 0, 1, 0, &expected);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Invalidate (loadOp C=DONTCARE, D=DONTCARE)
    const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, discards);
    ASSERT_GL_NO_ERROR();

    // Clear (loadOp C=CLEAR, D=CLEAR)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Save existing draw buffers
    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    std::vector<GLenum> savedDrawBuffers(maxDrawBuffers);
    for (int i = 0; i < maxDrawBuffers; i++)
        glGetIntegerv(GL_DRAW_BUFFER0 + i, (GLint *)&savedDrawBuffers[i]);

    // Draw depth-only
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glDrawBuffers(0, nullptr);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    glDrawBuffers(maxDrawBuffers, savedDrawBuffers.data());
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    ASSERT_GL_NO_ERROR();

    // Invalidate depth only (storeOp should be C=STORE/D=CLEAR)
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done
    swapBuffers();
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_DEPTH_OP_COUNTERS(getPerfCounters(), expected);
    ASSERT_GL_NO_ERROR();
}

// Test that depth clears are picked up as loadOp even if a color blit is done in between.
TEST_P(VulkanPerformanceCounterTest, DepthClearThenColorBlitThenDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupForColorDepthOpsTest(&framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:

    // color+depth clear, blit color as source, draw
    //
    // Expected:
    //   rpCount+1,
    //   depth(Clears+1, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    //   color(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForDepthOps(getPerfCounters(), 0, 1, 0, 0, 1, 0, &expected);
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 1, 0, 1, 0, &expected);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Blit color into another FBO
    GLTexture destColor;
    glBindTexture(GL_TEXTURE_2D, destColor);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kOpsTestSize, kOpsTestSize);

    GLFramebuffer destFbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destFbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, destColor, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_DRAW_FRAMEBUFFER);

    glBlitFramebuffer(0, 0, kOpsTestSize, kOpsTestSize, 0, 0, kOpsTestSize, kOpsTestSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Draw back to the original framebuffer
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    drawQuad(program, essl1_shaders::PositionAttrib(), 1.f);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    // Use swapBuffers and then check how many loads and stores were actually done.  The clear
    // applied to depth should be done as loadOp, not flushed during the blit.
    swapBuffers();
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
    EXPECT_DEPTH_OP_COUNTERS(getPerfCounters(), expected);
    ASSERT_GL_NO_ERROR();

    // Verify results
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Result of blit should be green
    glBindFramebuffer(GL_READ_FRAMEBUFFER, destFbo);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Ensure that image gets marked as defined after clear + invalidate + clear, and that we use
// LoadOp=Load for a renderpass which draws to it after the clear has been flushed with a blit.
TEST_P(VulkanPerformanceCounterTest, InvalidateThenRepeatedClearThenBlitThenDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    constexpr GLsizei kSize = 2;

    // tex[0] is what's being tested.  The others are helpers.
    GLTexture tex[3];
    for (int i = 0; i < 3; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, tex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);
    }

    GLFramebuffer fbo[3];
    for (int i = 0; i < 3; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex[i], 0);
    }

    // Expect rpCount+1, color(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 1, 0, 1, 0, &expected);

    // Clear the image through fbo[0], and make sure the clear is flushed outside the render pass.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Invalidate it such that the contents are marked as undefined. Note that regardless of the
    // marking, the image is cleared nevertheless.
    const GLenum discards[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);

    // Clear it again to the same color.
    glClear(GL_COLOR_BUFFER_BIT);

    // Bind tex[0] to fbo[1] as the read fbo, and blit to fbo[2]
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo[1]);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex[0], 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[2]);

    // Blit.  This causes the second clear of tex[0] to be flushed outside the render pass, which
    // may be optimized out.
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Switch back to fbo[0] and draw with blend.  If the second clear is dropped and the image
    // continues to be marked as invalidated, loadOp=DONT_CARE would be used instead of loadOp=LOAD.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);

    EXPECT_EQ(expected.renderPasses, getPerfCounters().renderPasses);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Ensure that image gets marked as defined after clear + invalidate + clear, and that we use
// LoadOp=Load for a renderpass which draws to it after the clear has been flushed with read pixels.
TEST_P(VulkanPerformanceCounterTest, InvalidateThenRepeatedClearThenReadbackThenDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    constexpr GLsizei kSize = 2;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    // Expect rpCount+1, color(Clears+0, Loads+1, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 1, 0, 1, 0, &expected);

    // Clear the image, and make sure the clear is flushed outside the render pass.
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Invalidate it such that the contents are marked as undefined.  Note that regarldess of the
    // marking, the image is cleared nevertheless.
    const GLenum discards[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);

    // Clear it again to the same color, and make sure the clear is flushed outside the render pass,
    // which may be optimized out.
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw with blend.  If the second clear is dropped and the image continues to be marked as
    // invalidated, loadOp=DONT_CARE would be used instead of loadOp=LOAD.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Test that draw after invalidate restores the contents of the color image.
TEST_P(VulkanPerformanceCounterTest, InvalidateThenDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    constexpr GLsizei kSize = 2;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    // Expect rpCount+1, color(Clears+0, Loads+0, LoadNones+0, Stores+1, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 0, 0, 1, 0, &expected);

    ANGLE_GL_PROGRAM(blue, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(blue, essl1_shaders::PositionAttrib(), 0);

    // Invalidate it such that the contents are marked as undefined
    const GLenum discards[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);

    // Draw again.
    ANGLE_GL_PROGRAM(green, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(green, essl1_shaders::PositionAttrib(), 0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Test that masked draw after invalidate does NOT restore the contents of the color image.
TEST_P(VulkanPerformanceCounterTest, InvalidateThenMaskedDraw)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    constexpr GLsizei kSize = 2;

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    // Expect rpCount+1, color(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0)
    setExpectedCountersForColorOps(getPerfCounters(), 1, 0, 0, 0, 0, 0, &expected);

    ANGLE_GL_PROGRAM(blue, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    drawQuad(blue, essl1_shaders::PositionAttrib(), 0);

    // Invalidate it such that the contents are marked as undefined
    const GLenum discards[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, discards);

    // Draw again.
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    ANGLE_GL_PROGRAM(green, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(green, essl1_shaders::PositionAttrib(), 0);

    // Break the render pass
    glFinish();

    EXPECT_COLOR_OP_COUNTERS(getPerfCounters(), expected);
}

// Tests that the submission counters count the implicit submission in eglSwapBuffers().
TEST_P(VulkanPerformanceCounterTest, VerifySubmitCountersForSwapBuffer)
{
    uint64_t expectedVkQueueSubmitCount      = getPerfCounters().vkQueueSubmitCallsTotal;
    uint64_t expectedCommandQueueSubmitCount = getPerfCounters().commandQueueSubmitCallsTotal;

    // One submission coming from clear and read back
    ++expectedVkQueueSubmitCount;
    ++expectedCommandQueueSubmitCount;

    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    EXPECT_EQ(getPerfCounters().vkQueueSubmitCallsTotal, expectedVkQueueSubmitCount);
    EXPECT_EQ(getPerfCounters().commandQueueSubmitCallsTotal, expectedCommandQueueSubmitCount);

    // One submission coming from draw and implicit submission from eglSwapBuffers
    ++expectedVkQueueSubmitCount;
    ++expectedCommandQueueSubmitCount;

    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(drawGreen, essl1_shaders::PositionAttrib(), 1.f);
    swapBuffers();

    EXPECT_EQ(getPerfCounters().vkQueueSubmitCallsTotal, expectedVkQueueSubmitCount);
    EXPECT_EQ(getPerfCounters().commandQueueSubmitCallsTotal, expectedCommandQueueSubmitCount);
}

// Tests that PreferSubmitAtFBOBoundary feature works properly. Bind to different FBO and should
// trigger submit of previous FBO. In this specific test, we switch to system default framebuffer
// which is always considered as "dirty".
TEST_P(VulkanPerformanceCounterTest, VerifySubmitCounterForSwitchUserFBOToSystemFramebuffer)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    uint64_t expectedCommandQueueSubmitCount = getPerfCounters().commandQueueSubmitCallsTotal;
    uint64_t expectedCommandQueueWaitSemaphoreCount =
        getPerfCounters().commandQueueWaitSemaphoresTotal;

    GLFramebuffer framebuffer;
    GLTexture texture;
    setupForColorOpsTest(&framebuffer, &texture);

    // Draw
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // One submission coming from glBindFramebuffer and draw
    ++expectedCommandQueueSubmitCount;
    // This submission should not wait for any semaphore.

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_EQ(getPerfCounters().commandQueueSubmitCallsTotal, expectedCommandQueueSubmitCount);
    EXPECT_EQ(getPerfCounters().commandQueueWaitSemaphoresTotal,
              expectedCommandQueueWaitSemaphoreCount);

    // This submission must wait for ANI's semaphore
    ++expectedCommandQueueWaitSemaphoreCount;
    ++expectedCommandQueueSubmitCount;
    swapBuffers();
    EXPECT_EQ(getPerfCounters().commandQueueSubmitCallsTotal, expectedCommandQueueSubmitCount);
    EXPECT_EQ(getPerfCounters().commandQueueWaitSemaphoresTotal,
              expectedCommandQueueWaitSemaphoreCount);
}

// Tests that PreferSubmitAtFBOBoundary feature works properly. Bind to different FBO and should
// trigger submit of previous FBO. In this specific test, we test bind to a new user FBO which we
// used to had a bug.
TEST_P(VulkanPerformanceCounterTest, VerifySubmitCounterForSwitchUserFBOToDirtyUserFBO)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    uint64_t expectedCommandQueueSubmitCount = getPerfCounters().commandQueueSubmitCallsTotal;
    uint64_t expectedCommandQueueWaitSemaphoreCount =
        getPerfCounters().commandQueueWaitSemaphoresTotal;

    GLFramebuffer framebuffer;
    GLTexture texture;
    setupForColorOpsTest(&framebuffer, &texture);

    // Draw
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    if (hasPreferSubmitAtFBOBoundary())
    {
        // One submission coming from glBindFramebuffer and draw
        ++expectedCommandQueueSubmitCount;
        // This submission should not wait for any semaphore.
    }

    // Create and bind to a new FBO
    GLFramebuffer framebuffer2;
    GLTexture texture2;
    setupForColorOpsTest(&framebuffer2, &texture2);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    ++expectedCommandQueueSubmitCount;
    // This submission should not wait for ANI's semaphore
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    EXPECT_EQ(getPerfCounters().commandQueueSubmitCallsTotal, expectedCommandQueueSubmitCount);
    EXPECT_EQ(getPerfCounters().commandQueueWaitSemaphoresTotal,
              expectedCommandQueueWaitSemaphoreCount);
}

// Ensure that glFlush doesn't lead to vkQueueSubmit if there's nothing to submit.
TEST_P(VulkanPerformanceCounterTest, UnnecessaryFlushDoesntCauseSubmission)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    swapBuffers();
    uint64_t expectedVkQueueSubmitCalls = getPerfCounters().vkQueueSubmitCallsTotal;

    glFlush();
    glFlush();
    glFlush();

    // Nothing was recorded, so there shouldn't be anything to flush.
    glFinish();
    EXPECT_EQ(getPerfCounters().vkQueueSubmitCallsTotal, expectedVkQueueSubmitCalls);

    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // One submission for the above readback
    ++expectedVkQueueSubmitCalls;

    glFinish();
    EXPECT_EQ(getPerfCounters().vkQueueSubmitCallsTotal, expectedVkQueueSubmitCalls);

    glFlush();
    glFlush();
    glFlush();

    // No additional submissions since last one
    glFinish();
    EXPECT_EQ(getPerfCounters().vkQueueSubmitCallsTotal, expectedVkQueueSubmitCalls);
}

// Ensure that glFenceSync doesn't lead to vkQueueSubmit if there's nothing to submit.
TEST_P(VulkanPerformanceCounterTest, SyncWihtoutCommandsDoesntCauseSubmission)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    swapBuffers();
    uint64_t expectedVkQueueSubmitCalls = getPerfCounters().vkQueueSubmitCallsTotal;

    glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    // Nothing was recorded, so there shouldn't be anything to flush.
    glFinish();
    EXPECT_EQ(getPerfCounters().vkQueueSubmitCallsTotal, expectedVkQueueSubmitCalls);
}

// In single-buffer mode, ensure that unnecessary eglSwapBuffers is completely ignored (i.e. doesn't
// lead to a command queue submission, consuming a submission serial).  Used to verify an
// optimization that ensures CPU throttling doesn't incur GPU bubbles with unnecessary
// eglSwapBuffers calls.
TEST_P(VulkanPerformanceCounterTest_SingleBuffer, SwapBuffersAfterFlushIgnored)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    // Set mode to single buffer
    EXPECT_EGL_TRUE(eglSurfaceAttrib(getEGLWindow()->getDisplay(), getEGLWindow()->getSurface(),
                                     EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER));

    // Swap buffers so mode switch takes effect.
    swapBuffers();
    uint64_t expectedCommandQueueSubmitCalls = getPerfCounters().commandQueueSubmitCallsTotal;

    // Further swap buffers should be ineffective.
    swapBuffers();
    swapBuffers();
    swapBuffers();
    swapBuffers();

    EXPECT_EQ(getPerfCounters().commandQueueSubmitCallsTotal, expectedCommandQueueSubmitCalls);

    // Issue commands and flush them.
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // One submission for the above readback
    ++expectedCommandQueueSubmitCalls;
    EXPECT_EQ(getPerfCounters().commandQueueSubmitCallsTotal, expectedCommandQueueSubmitCalls);

    // Further swap buffers should again be ineffective.
    swapBuffers();
    swapBuffers();
    swapBuffers();
    swapBuffers();
    swapBuffers();

    EXPECT_EQ(getPerfCounters().commandQueueSubmitCallsTotal, expectedCommandQueueSubmitCalls);
}

// Verifies that we share Texture descriptor sets between programs.
TEST_P(VulkanPerformanceCounterTest, TextureDescriptorsAreShared)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    ANGLE_GL_PROGRAM(testProgram1, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    ANGLE_GL_PROGRAM(testProgram2, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

    GLTexture texture1;
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLTexture texture2;
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    setupQuadVertexBuffer(0.5f, 1.0f);

    glUseProgram(testProgram1);

    ASSERT_GL_NO_ERROR();

    glBindTexture(GL_TEXTURE_2D, texture1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ASSERT_GL_NO_ERROR();

    GLuint expectedCacheMisses = getPerfCounters().textureDescriptorSetCacheMisses;
    if (isFeatureEnabled(Feature::DescriptorSetCache))
    {
        EXPECT_GT(expectedCacheMisses, 0u);
    }

    glUseProgram(testProgram2);

    glBindTexture(GL_TEXTURE_2D, texture1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ASSERT_GL_NO_ERROR();

    if (isFeatureEnabled(Feature::DescriptorSetCache))
    {
        GLuint actualCacheMisses = getPerfCounters().textureDescriptorSetCacheMisses;
        EXPECT_EQ(expectedCacheMisses, actualCacheMisses);
    }
    else
    {
        GLint descriptorSetAllocations = getPerfCounters().descriptorSetAllocations;
        EXPECT_EQ(4, descriptorSetAllocations);
    }
}

// Verifies that we share Uniform Buffer descriptor sets between programs.
TEST_P(VulkanPerformanceCounterTest, UniformBufferDescriptorsAreShared)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;
uniform block {
   vec4 uniColor;
};

void main() {
    color = uniColor;
})";

    ANGLE_GL_PROGRAM(testProgram1, essl3_shaders::vs::Simple(), kFS);
    ANGLE_GL_PROGRAM(testProgram2, essl3_shaders::vs::Simple(), kFS);

    Vector4 red(1, 0, 0, 1);
    Vector4 green(0, 1, 0, 1);

    GLBuffer ubo1;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo1);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Vector4), red.data(), GL_STATIC_DRAW);

    GLBuffer ubo2;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo2);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Vector4), green.data(), GL_STATIC_DRAW);

    setupQuadVertexBuffer(0.5f, 1.0f);

    glUseProgram(testProgram1);

    ASSERT_GL_NO_ERROR();

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo2);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ASSERT_GL_NO_ERROR();

    GLuint expectedCacheMisses = getPerfCounters().shaderResourcesDescriptorSetCacheMisses;
    GLuint totalAllocationsAtFirstProgram = getPerfCounters().descriptorSetAllocations;
    if (isFeatureEnabled(Feature::DescriptorSetCache))
    {
        EXPECT_GT(expectedCacheMisses, 0u);
    }

    glUseProgram(testProgram2);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo2);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ASSERT_GL_NO_ERROR();

    if (isFeatureEnabled(Feature::DescriptorSetCache))
    {
        GLuint actualCacheMisses = getPerfCounters().shaderResourcesDescriptorSetCacheMisses;
        EXPECT_EQ(expectedCacheMisses, actualCacheMisses);
    }
    else
    {
        // Because between program switch there is no submit and wait for finish, we expect draw
        // with second program will end up allocating same number of descriptorSets as the draws
        // with the first program.
        GLuint totalAllocationsAtSecondProgram = getPerfCounters().descriptorSetAllocations;
        EXPECT_EQ(totalAllocationsAtFirstProgram * 2, totalAllocationsAtSecondProgram);
    }
}

// Test modifying texture size and render to it does not cause VkFramebuffer cache explode
TEST_P(VulkanPerformanceCounterTest, ResizeFBOAttachedTexture)
{
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());

    int32_t framebufferCacheSizeBefore = getPerfCounters().framebufferCacheSize;
    GLTexture texture;
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    for (GLint texWidth = 1; texWidth <= 10; texWidth++)
    {
        for (GLint texHeight = 1; texHeight <= 10; texHeight++)
        {
            // Allocate texture
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
            ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

            // Draw to FBO backed by the texture
            glUseProgram(blueProgram);
            drawQuad(blueProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
            ASSERT_GL_NO_ERROR();
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
        }
    }
    int32_t framebufferCacheSizeAfter    = getPerfCounters().framebufferCacheSize;
    int32_t framebufferCacheSizeIncrease = framebufferCacheSizeAfter - framebufferCacheSizeBefore;
    int32_t expectedFramebufferCacheSizeIncrease = (hasSupportsImagelessFramebuffer()) ? 0 : 1;
    printf("\tframebufferCacheCountIncrease:%u\n", framebufferCacheSizeIncrease);
    // We should not cache obsolete VkImages. Only current VkImage should be cached.
    EXPECT_EQ(framebufferCacheSizeIncrease, expectedFramebufferCacheSizeIncrease);
}

// Test calling glTexParameteri(GL_TEXTURE_SWIZZLE_*) on a texture that attached to FBO with the
// same value did not cause VkFramebuffer cache explode
TEST_P(VulkanPerformanceCounterTest, SetTextureSwizzleWithSameValueOnFBOAttachedTexture)
{
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    ANGLE_GL_PROGRAM(textureProgram, essl1_shaders::vs::Texture2D(),
                     essl1_shaders::fs::Texture2D());

    // Allocate texture
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    int32_t framebufferCacheSizeBefore = getPerfCounters().framebufferCacheSize;
    for (GLint loop = 0; loop < 10; loop++)
    {
        // Draw to FBO
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glUseProgram(blueProgram);
        drawQuad(blueProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);

        // Sample from texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
        drawQuad(textureProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    }
    // Now make fbo current and read out cache size and verify it does not grow just because of
    // swizzle update even though there is no actual change.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUseProgram(blueProgram);
    drawQuad(blueProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    int32_t framebufferCacheSizeAfter    = getPerfCounters().framebufferCacheSize;
    int32_t framebufferCacheSizeIncrease = framebufferCacheSizeAfter - framebufferCacheSizeBefore;
    int32_t expectedFramebufferCacheSizeIncrease = (hasSupportsImagelessFramebuffer()) ? 0 : 1;
    // This should not cause frame buffer cache increase.
    EXPECT_EQ(framebufferCacheSizeIncrease, expectedFramebufferCacheSizeIncrease);
}

// Test calling glTexParameteri(GL_TEXTURE_SWIZZLE_*) on a texture that attached to FBO with
// different value did not cause VkFramebuffer cache explode
TEST_P(VulkanPerformanceCounterTest, SetTextureSwizzleWithDifferentValueOnFBOAttachedTexture)
{
    ANGLE_GL_PROGRAM(blueProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Blue());
    ANGLE_GL_PROGRAM(textureProgram, essl1_shaders::vs::Texture2D(),
                     essl1_shaders::fs::Texture2D());

    // Allocate texture
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLColor expectedColors[] = {GLColor::black,   GLColor::black,  GLColor::blue,  GLColor::black,
                                GLColor::black,   GLColor::blue,   GLColor::green, GLColor::green,
                                GLColor::cyan,    GLColor::black,  GLColor::black, GLColor::blue,
                                GLColor::black,   GLColor::black,  GLColor::blue,  GLColor::green,
                                GLColor::green,   GLColor::cyan,   GLColor::red,   GLColor::red,
                                GLColor::magenta, GLColor::red,    GLColor::red,   GLColor::magenta,
                                GLColor::yellow,  GLColor::yellow, GLColor::white};
    int32_t framebufferCacheSizeBefore = getPerfCounters().framebufferCacheSize;
    int loop                           = 0;
    for (GLenum swizzle_R = GL_RED; swizzle_R <= GL_BLUE; swizzle_R++)
    {
        for (GLenum swizzle_G = GL_RED; swizzle_G <= GL_BLUE; swizzle_G++)
        {
            for (GLenum swizzle_B = GL_RED; swizzle_B <= GL_BLUE; swizzle_B++)
            {
                // Draw to FBO
                glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                glUseProgram(blueProgram);
                drawQuad(blueProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);

                // Sample from texture
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glClearColor(1, 0, 0, 1);
                glClear(GL_COLOR_BUFFER_BIT);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, swizzle_R);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, swizzle_G);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, swizzle_B);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);
                drawQuad(textureProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
                EXPECT_PIXEL_COLOR_EQ(0, 0, expectedColors[loop]);
                loop++;
            }
        }
    }
    // Now make fbo current and read out cache size and verify it does not grow just because of
    // swizzle update even though there is no actual change.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glUseProgram(blueProgram);
    drawQuad(blueProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
    ASSERT_GL_NO_ERROR();
    int32_t framebufferCacheSizeAfter    = getPerfCounters().framebufferCacheSize;
    int32_t framebufferCacheSizeIncrease = framebufferCacheSizeAfter - framebufferCacheSizeBefore;
    int32_t expectedFramebufferCacheSizeIncrease = (hasSupportsImagelessFramebuffer()) ? 0 : 1;
    // This should not cause frame buffer cache increase.
    EXPECT_EQ(framebufferCacheSizeIncrease, expectedFramebufferCacheSizeIncrease);
}

void VulkanPerformanceCounterTest::saveAndReloadBinary(GLProgram *original, GLProgram *reloaded)
{
    GLint programLength = 0;
    GLint writtenLength = 0;
    GLenum binaryFormat = 0;

    // Get the binary out of the program and delete it.
    glGetProgramiv(*original, GL_PROGRAM_BINARY_LENGTH_OES, &programLength);
    EXPECT_GL_NO_ERROR();

    std::vector<uint8_t> binary(programLength);
    glGetProgramBinaryOES(*original, programLength, &writtenLength, &binaryFormat, binary.data());
    EXPECT_GL_NO_ERROR();

    original->reset();

    // Reload the binary into another program
    reloaded->makeEmpty();
    glProgramBinaryOES(*reloaded, binaryFormat, binary.data(), writtenLength);
    EXPECT_GL_NO_ERROR();

    GLint linkStatus;
    glGetProgramiv(*reloaded, GL_LINK_STATUS, &linkStatus);
    EXPECT_NE(linkStatus, 0);
}

void VulkanPerformanceCounterTest::testPipelineCacheIsWarm(GLProgram *program, GLColor color)
{
    glUseProgram(*program);
    GLint colorUniformLocation =
        glGetUniformLocation(*program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorUniformLocation);
    ASSERT_GL_NO_ERROR();

    GLuint expectedCacheHits   = getPerfCounters().pipelineCreationCacheHits + 1;
    GLuint expectedCacheMisses = getPerfCounters().pipelineCreationCacheMisses;

    glUniform4fv(colorUniformLocation, 1, color.toNormalizedVector().data());
    drawQuad(*program, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_EQ(getPerfCounters().pipelineCreationCacheHits, expectedCacheHits);
    EXPECT_EQ(getPerfCounters().pipelineCreationCacheMisses, expectedCacheMisses);

    EXPECT_PIXEL_COLOR_EQ(0, 0, color);
}

// Verifies that the pipeline cache is warmed up at link time with reasonable defaults.
TEST_P(VulkanPerformanceCounterTest, PipelineCacheIsWarmedUpAtLinkTime)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    // Test is only valid when pipeline creation feedback is available
    ANGLE_SKIP_TEST_IF(!hasSupportsPipelineCreationFeedback() || !hasWarmUpPipelineCacheAtLink() ||
                       !hasEffectivePipelineCacheSerialization());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::UniformColor());

    testPipelineCacheIsWarm(&program, GLColor::red);
}

// Verifies that the pipeline cache is reloaded correctly through glProgramBinary.
TEST_P(VulkanPerformanceCounterTest, PipelineCacheIsRestoredWithProgramBinary)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    // Test is only valid when pipeline creation feedback is available
    ANGLE_SKIP_TEST_IF(!hasSupportsPipelineCreationFeedback() || !hasWarmUpPipelineCacheAtLink() ||
                       !hasEffectivePipelineCacheSerialization());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::UniformColor());
    GLProgram reloadedProgram;
    saveAndReloadBinary(&program, &reloadedProgram);

    testPipelineCacheIsWarm(&reloadedProgram, GLColor::green);
}

// Verifies that the pipeline cache is reloaded correctly through glProgramBinary twice.
TEST_P(VulkanPerformanceCounterTest, PipelineCacheIsRestoredWithProgramBinaryTwice)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    // Test is only valid when pipeline creation feedback is available
    ANGLE_SKIP_TEST_IF(!hasSupportsPipelineCreationFeedback() || !hasWarmUpPipelineCacheAtLink() ||
                       !hasEffectivePipelineCacheSerialization());

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::UniformColor());
    GLProgram reloadedProgram;
    GLProgram twiceReloadedProgram;
    saveAndReloadBinary(&program, &reloadedProgram);
    saveAndReloadBinary(&reloadedProgram, &twiceReloadedProgram);

    testPipelineCacheIsWarm(&twiceReloadedProgram, GLColor::blue);
}

// Test calling glEGLImageTargetTexture2DOES repeatedly with same arguments will not leak
// DescriptorSets. This is the same usage pattern surafceflinger is doing with notification shades
// except with AHB.
TEST_P(VulkanPerformanceCounterTest, Source2DAndRepeatedlyRespecifyTarget2DWithSameParameter)
{
    EGLWindow *window = getEGLWindow();
    EGLDisplay dpy    = window->getDisplay();
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_EGL_image") ||
                       !IsEGLDisplayExtensionEnabled(dpy, "EGL_KHR_image_base") ||
                       !IsEGLDisplayExtensionEnabled(dpy, "EGL_KHR_gl_texture_2D_image"));

    ANGLE_GL_PROGRAM(textureProgram, essl1_shaders::vs::Texture2D(),
                     essl1_shaders::fs::Texture2D());

    // Create a source 2D texture
    GLTexture sourceTexture;
    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    GLubyte kLinearColor[] = {132, 55, 219, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 static_cast<void *>(&kLinearColor));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    // Create an eglImage from the source texture
    constexpr EGLint kDefaultAttribs[] = {EGL_IMAGE_PRESERVED, EGL_TRUE, EGL_NONE};
    EGLClientBuffer clientBuffer =
        reinterpret_cast<EGLClientBuffer>(static_cast<size_t>(sourceTexture));
    EGLImageKHR image = eglCreateImageKHR(window->getDisplay(), window->getContext(),
                                          EGL_GL_TEXTURE_2D_KHR, clientBuffer, kDefaultAttribs);
    ASSERT_EGL_SUCCESS();

    // Create the target in a loop
    GLTexture targetTexture;
    constexpr size_t kMaxLoop            = 20;
    GLint descriptorSetAllocationsBefore = 0;
    GLint textureDescriptorSetCacheTotalSizeBefore =
        getPerfCounters().textureDescriptorSetCacheTotalSize;
    for (size_t loop = 0; loop < kMaxLoop; loop++)
    {
        //  Create a target texture from the image
        glBindTexture(GL_TEXTURE_2D, targetTexture);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

        // Disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Draw a quad with the target texture
        glBindTexture(GL_TEXTURE_2D, targetTexture);
        drawQuad(textureProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
        // Expect that the rendered quad's color is the same as the reference color with a tolerance
        // of 1
        EXPECT_PIXEL_NEAR(0, 0, kLinearColor[0], kLinearColor[1], kLinearColor[2], kLinearColor[3],
                          1);
        if (loop == 0)
        {
            descriptorSetAllocationsBefore = getPerfCounters().descriptorSetAllocations;
        }
    }
    size_t descriptorSetAllocationsIncrease =
        getPerfCounters().descriptorSetAllocations - descriptorSetAllocationsBefore;
    GLint textureDescriptorSetCacheTotalSizeIncrease =
        getPerfCounters().textureDescriptorSetCacheTotalSize -
        textureDescriptorSetCacheTotalSizeBefore;

    if (isFeatureEnabled(Feature::DescriptorSetCache))
    {
        // We don't expect descriptorSet cache to keep growing
        EXPECT_EQ(1, textureDescriptorSetCacheTotalSizeIncrease);
    }
    else
    {
        // Because we call EXPECT_PIXEL_NEAR which will wait for draw to finish, we don't expect
        // descriptorSet allocation to keep growing. The underline reuse logic is implementation
        // detail (as of now it will not reuse util the pool is full), but we expect it will not
        // increasing for every iteration.
        EXPECT_GE(kMaxLoop - 1, descriptorSetAllocationsIncrease);
    }
    // Clean up
    eglDestroyImageKHR(window->getDisplay(), image);
}

// Test create, use and then destroy a texture does not increase number of DescriptoprSets.
// DesriptorSet should be destroyed promptly. We are seeing this type of usage pattern in
// surfaceflinger, except that the texture is created from AHB (and AHB keeps changing as well).
TEST_P(VulkanPerformanceCounterTest, CreateDestroyTextureDoesNotIncreaseDescriptporSetCache)
{
    ANGLE_GL_PROGRAM(textureProgram, essl1_shaders::vs::Texture2D(),
                     essl1_shaders::fs::Texture2D());

    // Respecify texture in a loop
    GLubyte kLinearColor[]    = {132, 55, 219, 255};
    constexpr size_t kMaxLoop = 2;
    GLint textureDescriptorSetCacheTotalSizeBefore =
        getPerfCounters().textureDescriptorSetCacheTotalSize;
    for (size_t loop = 0; loop < kMaxLoop; loop++)
    {
        // Create a 2D texture
        GLTexture sourceTexture;
        glBindTexture(GL_TEXTURE_2D, sourceTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     static_cast<void *>(&kLinearColor));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        ASSERT_GL_NO_ERROR();

        drawQuad(textureProgram, std::string(essl1_shaders::PositionAttrib()), 0.0f);
        // Expect that the rendered quad's color is the same as the reference color with a tolerance
        // of 1
        EXPECT_PIXEL_NEAR(0, 0, kLinearColor[0], kLinearColor[1], kLinearColor[2], kLinearColor[3],
                          1);
    }
    GLint textureDescriptorSetCacheTotalSizeIncrease =
        getPerfCounters().textureDescriptorSetCacheTotalSize -
        textureDescriptorSetCacheTotalSizeBefore;

    // We don't expect descriptorSet cache to keep growing
    EXPECT_EQ(0, textureDescriptorSetCacheTotalSizeIncrease);
}

// Similar to CreateDestroyTextureDoesNotIncreaseDescriptporSetCache, but for shader image.
TEST_P(VulkanPerformanceCounterTest_ES31,
       CreateDestroyTextureDoesNotIncreaseComputeShaderDescriptporSetCache)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(r32ui, binding = 0) readonly uniform highp uimage2D uImage_1;
layout(r32ui, binding = 1) writeonly uniform highp uimage2D uImage_2;
void main()
{
    uvec4 value = imageLoad(uImage_1, ivec2(gl_LocalInvocationID.xy));
    imageStore(uImage_2, ivec2(gl_LocalInvocationID.xy), value);
})";
    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    constexpr int kWidth = 1, kHeight = 1;
    constexpr GLuint kInputValues[2][1] = {{200}, {100}};
    constexpr size_t kMaxLoop           = 20;
    GLint shaderResourceDescriptorSetCacheTotalSizeBefore =
        getPerfCounters().shaderResourcesDescriptorSetCacheTotalSize;
    for (size_t loop = 0; loop < kMaxLoop; loop++)
    {
        // Respecify texture in a loop
        GLTexture texture0;
        glBindTexture(GL_TEXTURE_2D, texture0);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                        kInputValues[0]);

        GLTexture texture1;
        glBindTexture(GL_TEXTURE_2D, texture1);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, kWidth, kHeight);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RED_INTEGER, GL_UNSIGNED_INT,
                        kInputValues[1]);

        glBindImageTexture(0, texture0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
        glBindImageTexture(1, texture1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
        glDispatchCompute(1, 1, 1);
    }
    glFinish();
    GLint shaderResourceDescriptorSetCacheTotalSizeIncrease =
        getPerfCounters().shaderResourcesDescriptorSetCacheTotalSize -
        shaderResourceDescriptorSetCacheTotalSizeBefore;

    // We don't expect descriptorSet cache to keep growing
    EXPECT_EQ(0, shaderResourceDescriptorSetCacheTotalSizeIncrease);
}

// Similar to CreateDestroyTextureDoesNotIncreaseDescriptporSetCache, but for uniform buffers.
TEST_P(VulkanPerformanceCounterTest, DestroyUniformBufferAlsoDestroyDescriptporSetCache)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    const char *mkFS = R"(#version 300 es
precision highp float;
uniform uni { vec4 color; };
out vec4 fragColor;
void main()
{
    fragColor = color;
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), mkFS);
    GLint uniformBufferIndex = glGetUniformBlockIndex(program, "uni");
    ASSERT_NE(uniformBufferIndex, -1);

    // Warm up. Make a draw to ensure other descriptorSets are created if needed.
    GLBuffer intialBuffer;
    glBindBuffer(GL_UNIFORM_BUFFER, intialBuffer);
    std::vector<float> initialData = {0.1, 0.2, 0.3, 0.4};
    glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * initialData.size(), initialData.data(),
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, intialBuffer);
    glUniformBlockBinding(program, uniformBufferIndex, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_NEAR(0, 0, initialData[0] * 255, initialData[1] * 255, initialData[2] * 255,
                      initialData[3] * 255, 1);

    // Use big buffer size to force it into individual bufferBlocks
    constexpr GLsizei kBufferSize           = 4 * 1024 * 1024;
    GLint DescriptorSetCacheTotalSizeBefore = getPerfCounters().descriptorSetCacheTotalSize;

    // Create buffer and use it and then destroy it. Because buffers are big enough they should be
    // in a different bufferBlock. DescriptorSet created due to these temporary buffer should be
    // destroyed promptly.
    constexpr int kBufferCount = 16;
    for (int i = 0; i < kBufferCount; i++)
    {
        GLBuffer uniformBuffer;
        glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
        glBufferData(GL_UNIFORM_BUFFER, kBufferSize, nullptr, GL_DYNAMIC_DRAW);
        float *ptr = reinterpret_cast<float *>(
            glMapBufferRange(GL_UNIFORM_BUFFER, 0, kBufferSize, GL_MAP_WRITE_BIT));
        for (int j = 0; j < 4; j++)
        {
            ptr[j] = (float)(i * 4 + j) / 255.0f;
        }
        glUnmapBuffer(GL_UNIFORM_BUFFER);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
        glUniformBlockBinding(program, uniformBufferIndex, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
        EXPECT_PIXEL_NEAR(0, 0, (i * 4), (i * 4 + 1), (i * 4 + 2), (i * 4 + 3), 1);
    }
    // Should trigger prune buffer call
    swapBuffers();

    GLint DescriptorSetCacheTotalSizeIncrease =
        getPerfCounters().descriptorSetCacheTotalSize - DescriptorSetCacheTotalSizeBefore;
    // We expect most of descriptorSet caches for temporary uniformBuffers gets destroyed. Give
    // extra room in case a new descriptorSet is allocated due to a new driver uniform buffer gets
    // allocated.
    EXPECT_LT(DescriptorSetCacheTotalSizeIncrease, 2);
}

// Similar to CreateDestroyTextureDoesNotIncreaseDescriptporSetCache, but for atomic acounter
// buffer.
TEST_P(VulkanPerformanceCounterTest_ES31, DestroyAtomicCounterBufferAlsoDestroyDescriptporSetCache)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision highp float;\n"
        "layout(binding = 0, offset = 4) uniform atomic_uint ac;\n"
        "out highp vec4 my_color;\n"
        "void main()\n"
        "{\n"
        "    uint a1 = atomicCounter(ac);\n"
        "    my_color = vec4(float(a1)/255.0, 0.0, 0.0, 1.0);\n"
        "}\n";
    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);

    // Warm up. Make a draw to ensure other descriptorSets are created if needed.
    GLBuffer intialBuffer;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, intialBuffer);
    uint32_t bufferData[3] = {0, 0u, 0u};
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, intialBuffer);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);
    GLColor expectedColor = GLColor::black;
    EXPECT_PIXEL_COLOR_EQ(0, 0, expectedColor);

    GLint DescriptorSetCacheTotalSizeBefore = getPerfCounters().descriptorSetCacheTotalSize;

    // Create atomic counter buffer and use it and then destroy it.
    constexpr int kBufferCount = 16;
    GLBuffer paddingBuffers[kBufferCount];
    for (uint32_t i = 0; i < kBufferCount; i++)
    {
        // Allocate a padding buffer so that atomicCounterBuffer will be allocated in different
        // offset
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, paddingBuffers[i]);
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, 256, nullptr, GL_STATIC_DRAW);
        // Allocate, use, destroy atomic counter buffer
        GLBuffer atomicCounterBuffer;
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
        bufferData[1] = i;
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);
        drawQuad(program, essl31_shaders::PositionAttrib(), 0.0f);
        expectedColor.R = bufferData[1];
        EXPECT_PIXEL_COLOR_EQ(0, 0, expectedColor);
    }
    ASSERT_GL_NO_ERROR();

    GLint DescriptorSetCacheTotalSizeIncrease =
        getPerfCounters().descriptorSetCacheTotalSize - DescriptorSetCacheTotalSizeBefore;
    // We expect most of descriptorSet caches for temporary atomic counter buffers gets destroyed.
    // Give extra room in case a new descriptorSet is allocated due to a new driver uniform buffer
    // gets allocated.
    EXPECT_LT(DescriptorSetCacheTotalSizeIncrease, 2);
}

// Regression test for a bug in the Vulkan backend, where a perf warning added after
// serials for outside render pass commands were exhausted led to an assertion failure.
// http://issuetracker.google.com/375661776
TEST_P(VulkanPerformanceCounterTest_ES31, RunOutOfReservedOutsideRenderPassSerial)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    // Step 1: Draw to start a render pass.
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);

    // Step 2: Switch to fbo and clear it to set mRenderPassCommandBuffer to null,
    // but keep the render pass open.
    GLTexture fbTexture;
    glBindTexture(GL_TEXTURE_2D, fbTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTexture, 0);

    glClear(GL_COLOR_BUFFER_BIT);

    // Step 3: Run > kMaxReservedOutsideRenderPassQueueSerials compute with glMemoryBarrier
    // to trigger the "Running out of reserved outsideRenderPass queueSerial" path.
    constexpr char kCSSource[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(std140, binding=0) buffer buf {
vec4 outData;
};
void main()
{
outData = vec4(1.0, 1.0, 1.0, 1.0);
})";

    ANGLE_GL_COMPUTE_PROGRAM(csProgram, kCSSource);
    glUseProgram(csProgram);

    GLBuffer ssbo;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 16, nullptr, GL_STREAM_DRAW);

    for (int i = 0; i < 100; i++)
    {
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glDispatchCompute(1, 1, 1);
    }
    EXPECT_GL_NO_ERROR();

    uint64_t actualRenderPassCount = getPerfCounters().renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Test that post-render-pass-to-swapchain glFenceSync followed by eglSwapBuffers incurs only a
// single submission.
TEST_P(VulkanPerformanceCounterTest, FenceThenSwapBuffers)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled(kPerfMonitorExtensionName));

    angle::VulkanPerfCounters expected;

    // Expect rpCount+1, depth(Clears+0, Loads+0, LoadNones+0, Stores+0, StoreNones+0),
    setExpectedCountersForDepthOps(getPerfCounters(), 1, 0, 0, 0, 0, 0, &expected);
    expected.vkQueueSubmitCallsTotal = getPerfCounters().vkQueueSubmitCallsTotal + 1;

    // Start a render pass and render to the surface.  Enable depth write so the depth/stencil image
    // is written to.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f);
    ASSERT_GL_NO_ERROR();

    // Issue a fence
    glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    EXPECT_EQ(getPerfCounters().renderPasses, expected.renderPasses);

    // Swap buffers.  The depth/stencil attachment's storeOp should be optimized to DONT_CARE.  This
    // would not have been possible if the previous glFenceSync caused a submission.
    swapBuffers();

    EXPECT_EQ(getPerfCounters().vkQueueSubmitCallsTotal, expected.vkQueueSubmitCallsTotal);
    EXPECT_DEPTH_OP_COUNTERS(getPerfCounters(), expected);
}

// Verify that ending transform feedback after a render pass is closed, doesn't cause the following
// render pass to close when the transform feedback buffer is used.
TEST_P(VulkanPerformanceCounterTest, EndXfbAfterRenderPassClosed)
{
    // There should be two render passes; one for the transform feedback draw, one for the other two
    // draw calls.
    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 2;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the program's transform feedback varyings (just gl_Position)
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("gl_Position");
    ANGLE_GL_PROGRAM_TRANSFORM_FEEDBACK(drawRed, essl3_shaders::vs::SimpleForPoints(),
                                        essl3_shaders::fs::Red(), tfVaryings,
                                        GL_INTERLEAVED_ATTRIBS);

    glUseProgram(drawRed);
    GLint positionLocation = glGetAttribLocation(drawRed, essl1_shaders::PositionAttrib());

    const GLfloat vertices[] = {
        -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f, 0.5f,  0.5f,
    };

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    // Bind the buffer for transform feedback output and start transform feedback
    GLBuffer xfbBuffer;
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfbBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 100, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfbBuffer);
    glBeginTransformFeedback(GL_POINTS);

    glDrawArrays(GL_POINTS, 0, 6);

    // Break the render pass
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);

    // End transform feedback after the render pass is closed
    glEndTransformFeedback();

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
    EXPECT_GL_NO_ERROR();

    // Issue an unrelated draw call.
    ANGLE_GL_PROGRAM(drawGreen, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());
    drawQuad(drawGreen, essl3_shaders::PositionAttrib(), 0.0f);

    // Issue a draw call using the transform feedback buffer.
    glBindBuffer(GL_ARRAY_BUFFER, xfbBuffer);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(drawRed);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    const int w = getWindowWidth();
    const int h = getWindowHeight();

    EXPECT_PIXEL_RECT_EQ(0, 0, w, h / 4, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(0, 3 * h / 4, w, h / 4, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(0, h / 4, w / 4, h / 2, GLColor::black);
    EXPECT_PIXEL_RECT_EQ(3 * w / 4, h / 4, w / 4, h / 2, GLColor::black);

    EXPECT_PIXEL_RECT_EQ(w / 4, h / 4, w / 2, h / 2, GLColor::red);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(getPerfCounters().renderPasses, expectedRenderPassCount);
}

// Verify that monolithic pipeline handles correctly replace the linked pipelines, if
// VK_EXT_graphics_pipeline_library is supported.
TEST_P(VulkanPerformanceCounterTest, AsyncMonolithicPipelineCreation)
{
    const bool hasAsyncMonolithicPipelineCreation =
        isFeatureEnabled(Feature::SupportsGraphicsPipelineLibrary) &&
        isFeatureEnabled(Feature::PreferMonolithicPipelinesOverLibraries);
    ANGLE_SKIP_TEST_IF(!hasAsyncMonolithicPipelineCreation);

    uint64_t expectedMonolithicPipelineCreationCount =
        getPerfCounters().monolithicPipelineCreation + 2;

    // Create two programs:
    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    ANGLE_GL_PROGRAM(drawGreen, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());

    // Ping pong between the programs, letting async monolithic pipeline creation happen.
    uint32_t drawCount                 = 0;
    constexpr uint32_t kDrawCountLimit = 200;

    while (getPerfCounters().monolithicPipelineCreation < expectedMonolithicPipelineCreationCount)
    {
        drawQuad(drawGreen, essl3_shaders::PositionAttrib(), 0.0f);
        drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.0f);

        ++drawCount;
        if (drawCount > kDrawCountLimit)
        {
            drawCount = 0;
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
        }
    }

    // Make sure the monolithic pipelines are replaced correctly
    drawQuad(drawGreen, essl3_shaders::PositionAttrib(), 0.0f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    drawQuad(drawRed, essl3_shaders::PositionAttrib(), 0.0f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Verify that changing framebuffer and back doesn't break the render pass.
TEST_P(VulkanPerformanceCounterTest, FBOChangeAndBackDoesNotBreakRenderPass)
{
    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0);

    // Verify render pass count.
    EXPECT_EQ(getPerfCounters().renderPasses, expectedRenderPassCount);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Verify that changing framebuffer and issue a (nop or deferred) clear and change framebuffer back
// doesn't break the render pass.
TEST_P(VulkanPerformanceCounterTest, FBOChangeAndClearAndBackDoesNotBreakRenderPass)
{
    // switch to FBO and issue clear on fbo and then switch back to default framebuffer
    GLFramebuffer framebuffer;
    GLTexture texture;
    setupForColorOpsTest(&framebuffer, &texture);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);  // clear to green
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    uint64_t expectedRenderPassCount = getPerfCounters().renderPasses + 1;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0);

    // switch to FBO and issue a deferred clear on fbo and then switch back to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    drawQuad(drawRed, essl1_shaders::PositionAttrib(), 0);

    // Verify render pass count.
    EXPECT_EQ(getPerfCounters().renderPasses, expectedRenderPassCount);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// This is test for optimization in vulkan backend. efootball_pes_2021 usage shows this usage
// pattern and we expect implementation to reuse the storage for performance.
TEST_P(VulkanPerformanceCounterTest,
       BufferDataWithSizeFollowedByZeroAndThenSizeAgainShouldReuseStorage)
{
    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    constexpr size_t count = 288;
    std::array<uint32_t, count> data;
    constexpr size_t bufferSize = data.size() * sizeof(uint32_t);
    data.fill(0x1234567);

    glBufferData(GL_ARRAY_BUFFER, bufferSize, data.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    // This should get back the original storage with proper BufferVk optimization
    data.fill(0x89abcdef);
    uint64_t expectedSuballocationCalls = getPerfCounters().bufferSuballocationCalls;
    glBufferData(GL_ARRAY_BUFFER, bufferSize, data.data(), GL_DYNAMIC_DRAW);
    EXPECT_EQ(getPerfCounters().bufferSuballocationCalls, expectedSuballocationCalls);

    uint32_t *mapPtr = reinterpret_cast<uint32_t *>(
        glMapBufferRange(GL_ARRAY_BUFFER, 0, bufferSize, GL_MAP_READ_BIT));
    ASSERT_NE(nullptr, mapPtr);
    EXPECT_EQ(0x89abcdef, mapPtr[0]);
    EXPECT_EQ(0x89abcdef, mapPtr[count - 1]);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    ASSERT_GL_NO_ERROR();
}

// Regression test for a bug where submitting the outside command buffer during flushing staged
// updates did not properly update the command buffer state.
TEST_P(VulkanPerformanceCounterTest, SubmittingOutsideCommandBufferAssertIsOpen)
{
    // If VK_EXT_host_image_copy is used, uploads will all be done on the CPU and there would be no
    // submissions.
    ANGLE_SKIP_TEST_IF(hasSupportsHostImageCopy());

    uint64_t submitCommandsCount = getPerfCounters().vkQueueSubmitCallsTotal;

    ANGLE_GL_PROGRAM(textureProgram, essl1_shaders::vs::Texture2D(),
                     essl1_shaders::fs::Texture2D());
    glUseProgram(textureProgram);
    GLint textureLoc = glGetUniformLocation(textureProgram, essl1_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLoc);
    glUniform1i(textureLoc, 0);

    // This loop should update texture with multiple staged updates. When kMaxBufferToImageCopySize
    // threshold reached, outside command buffer will be submitted in the middle of staged updates
    // flushing.
    constexpr GLsizei kMaxOutsideRPCommandsSubmitCount = 10;
    constexpr GLsizei kTexDim                          = 1024;
    constexpr GLint kMaxSubOffset                      = 10;
    std::vector<GLColor> kInitialData(kTexDim * kTexDim, GLColor::green);
    while (getPerfCounters().vkQueueSubmitCallsTotal <
           submitCommandsCount + kMaxOutsideRPCommandsSubmitCount)
    {
        GLTexture newTexture;
        glBindTexture(GL_TEXTURE_2D, newTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Provide data for the texture without previously defining a storage.
        // This should prevent immediate staged update flushing.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTexDim, kTexDim, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     kInitialData.data());
        // Append staged updates...
        for (GLsizei offset = 1; offset <= kMaxSubOffset; ++offset)
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, offset, offset, kTexDim - offset, kTexDim - offset,
                            GL_RGBA, GL_UNSIGNED_BYTE, kInitialData.data());
        }
        // This will flush multiple staged updates
        drawQuad(textureProgram, essl1_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();
    }

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Verifies that clear followed by eglSwapBuffers() on multisampled FBO does not result
// extra resolve pass, if the surface is double-buffered and when the egl swap behavior
// is EGL_BUFFER_DESTROYED
TEST_P(VulkanPerformanceCounterTest_MSAA, SwapAfterClearOnMultisampledFBOShouldNotResolve)
{
    // Skip test if
    // 1) the EGL Surface is single-buffered or
    // 2) the EGL_SWAP_BEHAVIOR is EGL_BUFFER_PRESERVED
    EGLint renderBufferType;
    eglQuerySurface(getEGLWindow()->getDisplay(), getEGLWindow()->getSurface(), EGL_RENDER_BUFFER,
                    &renderBufferType);
    EGLint swapBehavior;
    eglQuerySurface(getEGLWindow()->getDisplay(), getEGLWindow()->getSurface(), EGL_SWAP_BEHAVIOR,
                    &swapBehavior);
    ANGLE_SKIP_TEST_IF(
        !(renderBufferType == EGL_BACK_BUFFER && swapBehavior == EGL_BUFFER_DESTROYED));

    uint32_t expectedResolvesSubpass = getPerfCounters().swapchainResolveInSubpass;
    uint32_t expectedResolvesOutside = getPerfCounters().swapchainResolveOutsideSubpass;
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    swapBuffers();
    EXPECT_EQ(getPerfCounters().swapchainResolveInSubpass, expectedResolvesSubpass);
    EXPECT_EQ(getPerfCounters().swapchainResolveOutsideSubpass, expectedResolvesOutside);
}

// Test that swapchain does not get necessary recreation with 90 or 270 emulated pre-rotation
TEST_P(VulkanPerformanceCounterTest_Prerotation, swapchainCreateCounterTest)
{
    uint64_t expectedSwapchainCreateCounter = getPerfCounters().swapchainCreate;
    for (uint32_t i = 0; i < 10; ++i)
    {
        swapBuffers();
    }
    EXPECT_EQ(getPerfCounters().swapchainCreate, expectedSwapchainCreateCounter);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(VulkanPerformanceCounterTest);
ANGLE_INSTANTIATE_TEST(
    VulkanPerformanceCounterTest,
    ES3_VULKAN(),
    ES3_VULKAN().enable(Feature::PadBuffersToMaxVertexAttribStride),
    ES3_VULKAN_SWIFTSHADER().disable(Feature::PreferMonolithicPipelinesOverLibraries),
    ES3_VULKAN_SWIFTSHADER().enable(Feature::PreferMonolithicPipelinesOverLibraries),
    ES3_VULKAN_SWIFTSHADER()
        .enable(Feature::PreferMonolithicPipelinesOverLibraries)
        .enable(Feature::SlowDownMonolithicPipelineCreationForTesting),
    ES3_VULKAN_SWIFTSHADER()
        .enable(Feature::PreferMonolithicPipelinesOverLibraries)
        .disable(Feature::MergeProgramPipelineCachesToGlobalCache));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(VulkanPerformanceCounterTest_ES31);
ANGLE_INSTANTIATE_TEST(VulkanPerformanceCounterTest_ES31, ES31_VULKAN(), ES31_VULKAN_SWIFTSHADER());

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(VulkanPerformanceCounterTest_MSAA);
ANGLE_INSTANTIATE_TEST(VulkanPerformanceCounterTest_MSAA,
                       ES3_VULKAN(),
                       ES3_VULKAN_SWIFTSHADER(),
                       ES3_VULKAN().enable(Feature::EmulatedPrerotation90),
                       ES3_VULKAN().enable(Feature::EmulatedPrerotation180),
                       ES3_VULKAN().enable(Feature::EmulatedPrerotation270));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(VulkanPerformanceCounterTest_Prerotation);
ANGLE_INSTANTIATE_TEST(
    VulkanPerformanceCounterTest_Prerotation,
    ES3_VULKAN().enable(Feature::PerFrameWindowSizeQuery),
    ES3_VULKAN().enable(Feature::EmulatedPrerotation90).enable(Feature::PerFrameWindowSizeQuery),
    ES3_VULKAN().enable(Feature::EmulatedPrerotation180).enable(Feature::PerFrameWindowSizeQuery),
    ES3_VULKAN().enable(Feature::EmulatedPrerotation270).enable(Feature::PerFrameWindowSizeQuery));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(VulkanPerformanceCounterTest_SingleBuffer);
ANGLE_INSTANTIATE_TEST(VulkanPerformanceCounterTest_SingleBuffer, ES3_VULKAN());

}  // anonymous namespace
