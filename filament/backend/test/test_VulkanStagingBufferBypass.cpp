/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "BackendTest.h"
#include "Lifetimes.h"
#include "ShaderGenerator.h"
#include "Skip.h"
#include "Workarounds.h"

#include <backend/BufferDescriptor.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/PipelineState.h>
#include <backend/PixelBufferDescriptor.h>
#include <backend/Platform.h>
#include <backend/Program.h>

#include <private/utils/FeatureFlagManager.h>

#include <math/vec2.h>
#include <math/vec4.h>

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <utility>
#include <vector>

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;

namespace {

constexpr uint32_t kTargetSize = 8;

struct FullScreenTriangle {
    BufferObjectHandle vbo;
    VertexBufferInfoHandle vbi;
    VertexBufferHandle vb;
    IndexBufferHandle ib;
    RenderPrimitiveHandle rp;
};

BufferDescriptor makeBufferDescriptor(void const* src, size_t numBytes) {
    void* copy = std::malloc(numBytes);
    std::memcpy(copy, src, numBytes);
    return BufferDescriptor(copy, numBytes, [](void* buffer, size_t, void*) { std::free(buffer); });
}

template<typename T>
void uploadRawBuffer(DriverApi& api, BufferObjectHandle bo, T const* data, size_t count,
        uint32_t byteOffset = 0) {
    api.updateBufferObject(bo, makeBufferDescriptor(data, sizeof(T) * count), byteOffset);
}

template<typename T>
void uploadBuffer(DriverApi& api, BufferObjectHandle bo, T const& data, uint32_t byteOffset = 0) {
    uploadRawBuffer(api, bo, &data, 1, byteOffset);
}

void uploadIndices(DriverApi& api, IndexBufferHandle ib, uint32_t const* indices, size_t count,
        uint32_t byteOffset = 0) {
    api.updateIndexBuffer(ib, makeBufferDescriptor(indices, sizeof(uint32_t) * count), byteOffset);
}

FullScreenTriangle createFullScreenTriangle(DriverApi& api, Cleanup& cleanup) {
    AttributeArray attributes = {};
    attributes[0] = Attribute{
        .offset = 0,
        .stride = sizeof(float2),
        .buffer = 0,
        .type = ElementType::FLOAT2,
        .flags = 0,
    };

    static constexpr float2 kVertices[3] = {
        { -1.0f, -1.0f },
        {  3.0f, -1.0f },
        { -1.0f,  3.0f },
    };
    static constexpr uint32_t kIndices[3] = { 0, 1, 2 };

    auto vbo = cleanup.add(api.createBufferObject(
            sizeof(kVertices), BufferObjectBinding::VERTEX, BufferUsage::STATIC));
    auto vbi = cleanup.add(api.createVertexBufferInfo(1, 1, attributes));
    auto vb = cleanup.add(api.createVertexBuffer(3, vbi));
    api.setVertexBufferObject(vb, 0, vbo);
    uploadRawBuffer(api, vbo, kVertices, 3);

    auto ib = cleanup.add(api.createIndexBuffer(ElementType::UINT, 3, BufferUsage::STATIC));
    uploadIndices(api, ib, kIndices, 3);

    auto rp = cleanup.add(api.createRenderPrimitive(vb, ib, PrimitiveType::TRIANGLES));
    return { vbo, vbi, vb, ib, rp };
}

struct OffscreenTarget {
    TextureHandle color;
    RenderTargetHandle rt;
};

OffscreenTarget createOffscreenTarget(DriverApi& api, Cleanup& cleanup) {
    auto color = cleanup.add(api.createTexture(SamplerType::SAMPLER_2D, 1, TextureFormat::RGBA8, 1,
            kTargetSize, kTargetSize, 1,
            TextureUsage::COLOR_ATTACHMENT TEXTURE_USAGE_READ_PIXELS));
    auto rt = cleanup.add(api.createRenderTarget(TargetBufferFlags::COLOR0, kTargetSize,
            kTargetSize, 1, 1, { { color } }, {}, {}));
    return { color, rt };
}

struct ReadbackResult {
    std::vector<uint8_t> pixels;
    bool completed = false;
};

void queueReadback(DriverApi& api, RenderTargetHandle rt, ReadbackResult& out) {
    out.pixels.assign(size_t(kTargetSize) * kTargetSize * 4, 0);
    out.completed = false;
    PixelBufferDescriptor pbd(out.pixels.data(), out.pixels.size(),
            PixelDataFormat::RGBA, PixelDataType::UBYTE,
            [](void*, size_t, void* user) {
                static_cast<ReadbackResult*>(user)->completed = true;
            },
            &out);
    api.readPixels(rt, 0, 0, kTargetSize, kTargetSize, std::move(pbd));
}

void expectSolidColor(ReadbackResult const& result, std::array<uint8_t, 4> expected,
        char const* label) {
    ASSERT_TRUE(result.completed) << label;
    for (size_t i = 0; i < result.pixels.size(); i += 4) {
        EXPECT_NEAR(result.pixels[i + 0], expected[0], 1) << label << " pixel " << (i / 4) << " R";
        EXPECT_NEAR(result.pixels[i + 1], expected[1], 1) << label << " pixel " << (i / 4) << " G";
        EXPECT_NEAR(result.pixels[i + 2], expected[2], 1) << label << " pixel " << (i / 4) << " B";
        EXPECT_NEAR(result.pixels[i + 3], expected[3], 1) << label << " pixel " << (i / 4) << " A";
    }
}

TextureHandle createSolidRgba8Texture(DriverApi& api, Cleanup& cleanup,
        std::array<uint8_t, 4> rgba) {
    constexpr uint32_t kTexSize = 2;
    auto tex = cleanup.add(api.createTexture(SamplerType::SAMPLER_2D, 1, TextureFormat::RGBA8, 1,
            kTexSize, kTexSize, 1, TextureUsage::SAMPLEABLE));
    size_t const numBytes = size_t(kTexSize) * kTexSize * 4;
    auto* data = static_cast<uint8_t*>(std::malloc(numBytes));
    for (size_t i = 0; i < numBytes; i += 4) {
        data[i + 0] = rgba[0];
        data[i + 1] = rgba[1];
        data[i + 2] = rgba[2];
        data[i + 3] = rgba[3];
    }
    PixelBufferDescriptor pbd(data, numBytes, PixelDataFormat::RGBA, PixelDataType::UBYTE,
            [](void* buffer, size_t, void*) { std::free(buffer); });
    api.update3DImage(tex, 0, 0, 0, 0, kTexSize, kTexSize, 1, std::move(pbd));
    return tex;
}

DescriptorSetLayoutHandle createLayout(DriverApi& api, Cleanup& cleanup,
        std::initializer_list<DescriptorSetLayoutDescriptor> descriptors) {
    DescriptorSetLayout dsl;
    dsl.descriptors.reserve(descriptors.size());
    for (auto const& d : descriptors) {
        dsl.descriptors.push_back(d);
    }
    return cleanup.add(api.createDescriptorSetLayout(std::move(dsl)));
}

const char* const kFullscreenVs = R"(#version 450 core
layout(location = 0) in vec4 mesh_position;
void main() {
    gl_Position = vec4(mesh_position.xy, 0.0, 1.0);
#if defined(TARGET_VULKAN_ENVIRONMENT)
    gl_Position.y = -gl_Position.y;
#endif
})";

} // namespace

class VulkanStagingBufferBypassTest : public BackendTest {
protected:
    VulkanStagingBufferBypassTest() : BackendTest(makeDriverConfig()) {}

    void drawToTarget(RenderTargetHandle rt, PipelineState const& ps, RenderPrimitiveHandle rp,
            double4 clearColor = double4(0, 0, 0, 1)) {
        auto& api = getDriverApi();
        RenderPassParams params = getClearColorDepthRenderPass(clearColor);
        params.viewport = { 0, 0, kTargetSize, kTargetSize };
        api.beginRenderPass(rt, params);
        api.bindPipeline(ps);
        api.bindRenderPrimitive(rp);
        api.draw2(0, 3, 1);
        api.endRenderPass();
    }

    ProgramHandle makeVulkanProgram(char const* fs,
            std::initializer_list<std::pair<descriptor_set_t,
                    std::initializer_list<Program::Descriptor>>> sets) {
        auto& api = getDriverApi();
        ShaderGenerator shaderGen(kFullscreenVs, fs, sBackend, sIsMobilePlatform);
        Program prog = shaderGen.getProgram(api);
        for (auto const& [setIndex, bindings] : sets) {
            Program::DescriptorBindingsInfo info(bindings.size());
            size_t i = 0;
            for (auto const& b : bindings) {
                info[i++] = b;
            }
            prog.descriptorBindings(setIndex, info);
        }
        return addCleanup(api.createProgram(std::move(prog)));
    }

private:
    static Platform::DriverConfig makeDriverConfig() {
        static utils::FeatureFlagManager featureFlags = [] {
            utils::FeatureFlagManager flags;
            flags.features.backend.vulkan.enable_staging_buffer_bypass = true;
            return flags;
        }();
        Platform::DriverConfig config = getDriverConfig();
        config.featureFlagManager = &featureFlags;
        return config;
    }
};

TEST_F(VulkanStagingBufferBypassTest, WholeBufferOrphaningAndDescriptorRebind) {
    SKIP_IF_NOT(Backend::VULKAN, "Tests Vulkan staging buffer bypass");

    static const char* const fs = R"(#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
layout(set = 0, binding = 0, std140) uniform Params {
    vec4 color;
} params;
void main() {
    fragColor = params.color;
})";

    auto& api = getDriverApi();
    auto swapChain = addCleanup(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    auto tri = createFullScreenTriangle(api, *mCleanup);
    auto dsl = createLayout(api, *mCleanup, {
        { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::FRAGMENT, 0 },
    });
    auto ds = addCleanup(api.createDescriptorSet(dsl));
    auto ubo = addCleanup(api.createBufferObject(
            sizeof(float4), BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
    api.updateDescriptorSetBuffer(ds, 0, ubo, 0, sizeof(float4));

    auto program = makeVulkanProgram(fs, {
        { 0, { { "Params", DescriptorType::UNIFORM_BUFFER, 0 } } },
    });

    PipelineState ps = getColorWritePipelineState();
    ps.program = program;
    ps.primitiveType = PrimitiveType::TRIANGLES;
    ps.vertexBufferInfo = tri.vbi;
    ps.pipelineLayout.setLayout[0] = { dsl };

    auto rt1 = createOffscreenTarget(api, *mCleanup);
    auto rt2 = createOffscreenTarget(api, *mCleanup);
    auto rt3 = createOffscreenTarget(api, *mCleanup);
    auto rt4 = createOffscreenTarget(api, *mCleanup);

    // 1. Initial upload while idle (bypass memcpy path), then draw Pass 1 (RED).
    uploadBuffer(api, ubo, float4(1.0f, 0.0f, 0.0f, 1.0f));
    api.beginFrame(0, 0, 0);
    api.bindDescriptorSet(ds, 0, {});
    drawToTarget(rt1.rt, ps, tri.rp);

    // 2. Whole-buffer update while Pass 1 is still recording in the active command buffer:
    // orphans the RED VkBuffer and memcpys GREEN into a fresh one. Pass 2 relies on the
    // previously bound descriptor set without re-calling bindDescriptorSet().
    uploadBuffer(api, ubo, float4(0.0f, 1.0f, 0.0f, 1.0f));
    drawToTarget(rt2.rt, ps, tri.rp);

    // Submit command buffer 1 without waiting for GPU completion.
    api.flush();
    executeCommands();

    // 3. Whole-buffer update while command buffer 1 is submitted/in-flight, then draw Pass 3
    // in a new command buffer (also verifies command-buffer age switch resets bound state).
    uploadBuffer(api, ubo, float4(0.0f, 0.0f, 1.0f, 1.0f));
    api.bindDescriptorSet(ds, 0, {});
    drawToTarget(rt3.rt, ps, tri.rp);
    api.commit(swapChain);
    api.endFrame(0);

    // Wait for all submitted work and let finish() -> mCommands.gc() release completed ages.
    flushAndWait();

    // 4. Whole-buffer update after ages have completed (non-in-flight bypass memcpy) -> YELLOW.
    uploadBuffer(api, ubo, float4(1.0f, 1.0f, 0.0f, 1.0f));
    api.beginFrame(0, 0, 1);
    api.bindDescriptorSet(ds, 0, {});
    drawToTarget(rt4.rt, ps, tri.rp);
    api.commit(swapChain);
    api.endFrame(1);

    ReadbackResult r1, r2, r3, r4;
    queueReadback(api, rt1.rt, r1);
    queueReadback(api, rt2.rt, r2);
    queueReadback(api, rt3.rt, r3);
    queueReadback(api, rt4.rt, r4);
    flushAndWait();

    expectSolidColor(r1, { 255, 0, 0, 255 }, "Pass 1 (RED)");
    expectSolidColor(r2, { 0, 255, 0, 255 }, "Pass 2 (GREEN)");
    expectSolidColor(r3, { 0, 0, 255, 255 }, "Pass 3 (BLUE)");
    expectSolidColor(r4, { 255, 255, 0, 255 }, "Pass 4 (YELLOW)");
}

TEST_F(VulkanStagingBufferBypassTest, PartialUpdateWhileInFlightPreservesPreviousDraw) {
    SKIP_IF_NOT(Backend::VULKAN, "Tests Vulkan staging buffer bypass");

    struct TwoColors {
        float4 colorA;
        float4 colorB;
    };

    static const char* const fs = R"(#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
layout(set = 0, binding = 0, std140) uniform Params {
    vec4 colorA;
    vec4 colorB;
} params;
void main() {
    fragColor = vec4(params.colorA.rgb + params.colorB.rgb, 1.0);
})";

    auto& api = getDriverApi();
    auto swapChain = addCleanup(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    auto tri = createFullScreenTriangle(api, *mCleanup);
    auto dsl = createLayout(api, *mCleanup, {
        { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::FRAGMENT, 0 },
    });
    auto ds = addCleanup(api.createDescriptorSet(dsl));
    auto ubo = addCleanup(api.createBufferObject(
            sizeof(TwoColors), BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
    api.updateDescriptorSetBuffer(ds, 0, ubo, 0, sizeof(TwoColors));

    auto program = makeVulkanProgram(fs, {
        { 0, { { "Params", DescriptorType::UNIFORM_BUFFER, 0 } } },
    });

    PipelineState ps = getColorWritePipelineState();
    ps.program = program;
    ps.primitiveType = PrimitiveType::TRIANGLES;
    ps.vertexBufferInfo = tri.vbi;
    ps.pipelineLayout.setLayout[0] = { dsl };

    auto rt1 = createOffscreenTarget(api, *mCleanup);
    auto rt2 = createOffscreenTarget(api, *mCleanup);
    auto rt3 = createOffscreenTarget(api, *mCleanup);

    uploadBuffer(api, ubo, TwoColors{
        .colorA = float4(1.0f, 0.0f, 0.0f, 1.0f),
        .colorB = float4(0.0f, 0.0f, 0.0f, 0.0f),
    });

    // Pass 1 reads { RED, BLACK } -> RED.
    api.beginFrame(0, 0, 0);
    api.bindDescriptorSet(ds, 0, {});
    drawToTarget(rt1.rt, ps, tri.rp);

    // Partial update while Pass 1 is in the same recording command buffer: cannot orphan, so it
    // must take the staging path with a barrier rather than overwriting colorB via memcpy.
    uploadBuffer(api, ubo, float4(0.0f, 1.0f, 0.0f, 0.0f), offsetof(TwoColors, colorB));
    drawToTarget(rt2.rt, ps, tri.rp);

    // Flush without waiting, then do another partial update while command buffer 1 is in flight.
    api.flush();
    executeCommands();

    uploadBuffer(api, ubo, float4(0.0f, 0.0f, 1.0f, 0.0f), offsetof(TwoColors, colorB));
    api.bindDescriptorSet(ds, 0, {});
    drawToTarget(rt3.rt, ps, tri.rp);
    api.commit(swapChain);
    api.endFrame(0);

    ReadbackResult r1, r2, r3;
    queueReadback(api, rt1.rt, r1);
    queueReadback(api, rt2.rt, r2);
    queueReadback(api, rt3.rt, r3);
    flushAndWait();

    expectSolidColor(r1, { 255, 0, 0, 255 }, "Pass 1 (RED)");
    expectSolidColor(r2, { 255, 255, 0, 255 }, "Pass 2 (YELLOW)");
    expectSolidColor(r3, { 255, 0, 255, 255 }, "Pass 3 (MAGENTA)");
}

TEST_F(VulkanStagingBufferBypassTest, ResetBufferObjectOrphansForPartialUpdate) {
    SKIP_IF_NOT(Backend::VULKAN, "Tests Vulkan staging buffer bypass");

    struct TwoColors {
        float4 colorA;
        float4 colorB;
    };

    static const char* const fs = R"(#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
layout(set = 0, binding = 0, std140) uniform Params {
    vec4 colorA;
    vec4 colorB;
} params;
void main() {
    fragColor = vec4(params.colorA.rgb + params.colorB.rgb, 1.0);
})";

    auto& api = getDriverApi();
    auto swapChain = addCleanup(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    auto tri = createFullScreenTriangle(api, *mCleanup);
    auto dsl = createLayout(api, *mCleanup, {
        { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::FRAGMENT, 0 },
    });
    auto ds = addCleanup(api.createDescriptorSet(dsl));
    auto ubo = addCleanup(api.createBufferObject(
            sizeof(TwoColors), BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
    api.updateDescriptorSetBuffer(ds, 0, ubo, 0, sizeof(TwoColors));

    auto program = makeVulkanProgram(fs, {
        { 0, { { "Params", DescriptorType::UNIFORM_BUFFER, 0 } } },
    });

    PipelineState ps = getColorWritePipelineState();
    ps.program = program;
    ps.primitiveType = PrimitiveType::TRIANGLES;
    ps.vertexBufferInfo = tri.vbi;
    ps.pipelineLayout.setLayout[0] = { dsl };

    auto rt1 = createOffscreenTarget(api, *mCleanup);
    auto rt2 = createOffscreenTarget(api, *mCleanup);
    auto rt3 = createOffscreenTarget(api, *mCleanup);

    // Calling resetBufferObject() on an idle, never-drawn buffer must be safe.
    api.resetBufferObject(ubo);
    uploadBuffer(api, ubo, TwoColors{
        .colorA = float4(1.0f, 0.0f, 0.0f, 1.0f),
        .colorB = float4(0.0f, 0.0f, 0.0f, 0.0f),
    });

    // Pass 1 reads { RED, BLACK } -> RED.
    api.beginFrame(0, 0, 0);
    api.bindDescriptorSet(ds, 0, {});
    drawToTarget(rt1.rt, ps, tri.rp);

    // resetBufferObject() while Pass 1 is in flight orphans the buffer so subsequent partial
    // updates populate a fresh VkBuffer without corrupting Pass 1. Pass 2 also omits
    // bindDescriptorSet() to verify stale-UBO detection after resetBufferObject().
    api.resetBufferObject(ubo);
    uploadBuffer(api, ubo, float4(0.0f, 1.0f, 0.0f, 1.0f), offsetof(TwoColors, colorA));
    uploadBuffer(api, ubo, float4(0.0f, 0.0f, 1.0f, 0.0f), offsetof(TwoColors, colorB));
    drawToTarget(rt2.rt, ps, tri.rp);

    api.commit(swapChain);
    api.endFrame(0);
    flushAndWait();

    // resetBufferObject() when idle (ages already released) followed by partial updates.
    api.resetBufferObject(ubo);
    uploadBuffer(api, ubo, float4(1.0f, 1.0f, 0.0f, 1.0f), offsetof(TwoColors, colorA));
    uploadBuffer(api, ubo, float4(0.0f, 0.0f, 0.0f, 0.0f), offsetof(TwoColors, colorB));

    api.beginFrame(0, 0, 1);
    api.bindDescriptorSet(ds, 0, {});
    drawToTarget(rt3.rt, ps, tri.rp);
    api.commit(swapChain);
    api.endFrame(1);

    ReadbackResult r1, r2, r3;
    queueReadback(api, rt1.rt, r1);
    queueReadback(api, rt2.rt, r2);
    queueReadback(api, rt3.rt, r3);
    flushAndWait();

    expectSolidColor(r1, { 255, 0, 0, 255 }, "Pass 1 (RED)");
    expectSolidColor(r2, { 0, 255, 255, 255 }, "Pass 2 (CYAN after in-flight reset)");
    expectSolidColor(r3, { 255, 255, 0, 255 }, "Pass 3 (YELLOW after idle reset)");
}

TEST_F(VulkanStagingBufferBypassTest, DescriptorCloningPreservesSamplersSubrangesAndUnwritten) {
    SKIP_IF_NOT(Backend::VULKAN, "Tests Vulkan staging buffer bypass");

    static const char* const fs = R"(#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
layout(set = 0, binding = 0, std140) uniform Params0 {
    vec4 red;
} params0;
layout(set = 0, binding = 1, std140) uniform Params1 {
    vec4 green;
} params1;
layout(set = 0, binding = 2) uniform sampler2D test_tex;
void main() {
    fragColor = vec4(params0.red.r, params1.green.g, texture(test_tex, vec2(0.5)).b, 1.0);
})";

    // Layout for ubo0 uses a 512-byte buffer where offset 0 is a sentinel and offset 256 holds the
    // bound 16-byte subrange (256 satisfies Vulkan's max minUniformBufferOffsetAlignment).
    struct SubrangeBuffer {
        float4 sentinel;
        uint8_t padding[256 - sizeof(float4)];
        float4 active;
        uint8_t tail[256 - sizeof(float4)];
    };
    static_assert(sizeof(SubrangeBuffer) == 512);
    static_assert(offsetof(SubrangeBuffer, active) == 256);

    auto& api = getDriverApi();
    auto swapChain = addCleanup(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    auto tri = createFullScreenTriangle(api, *mCleanup);

    // Binding 3 is declared in the layout but intentionally never written, verifying that
    // cloneSet() only copies written descriptors (mWrittenBindings).
    auto dsl = createLayout(api, *mCleanup, {
        { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::FRAGMENT, 0 },
        { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::FRAGMENT, 1 },
        { DescriptorType::SAMPLER_2D_FLOAT, ShaderStageFlags::FRAGMENT, 2 },
        { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::FRAGMENT, 3 },
    });
    auto ds = addCleanup(api.createDescriptorSet(dsl));

    auto ubo0 = addCleanup(api.createBufferObject(
            sizeof(SubrangeBuffer), BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
    auto ubo1 = addCleanup(api.createBufferObject(
            sizeof(float4), BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
    auto tex = createSolidRgba8Texture(api, *mCleanup, { 0, 0, 255, 255 });

    // Populate bindings out of binding order (2, 0, 1) and overwrite binding 0 once before drawing
    // so VulkanDescriptorSet::getResourceIndex() and mUboMask differ from binding numbers.
    api.updateDescriptorSetTexture(ds, 2, tex, {});
    api.updateDescriptorSetBuffer(ds, 0, ubo0, 0, sizeof(float4));
    api.updateDescriptorSetBuffer(ds, 0, ubo0, offsetof(SubrangeBuffer, active), sizeof(float4));
    api.updateDescriptorSetBuffer(ds, 1, ubo1, 0, sizeof(float4));

    auto program = makeVulkanProgram(fs, {
        { 0, {
            { "Params0", DescriptorType::UNIFORM_BUFFER, 0 },
            { "Params1", DescriptorType::UNIFORM_BUFFER, 1 },
            { "test_tex", DescriptorType::SAMPLER_2D_FLOAT, 2 },
        } },
    });

    PipelineState ps = getColorWritePipelineState();
    ps.program = program;
    ps.primitiveType = PrimitiveType::TRIANGLES;
    ps.vertexBufferInfo = tri.vbi;
    ps.pipelineLayout.setLayout[0] = { dsl };

    auto rt1 = createOffscreenTarget(api, *mCleanup);
    auto rt2 = createOffscreenTarget(api, *mCleanup);
    auto rt3 = createOffscreenTarget(api, *mCleanup);

    uploadBuffer(api, ubo0, SubrangeBuffer{
        .sentinel = float4(0.0f),
        .active = float4(1.0f, 0.0f, 0.0f, 1.0f),
    });
    uploadBuffer(api, ubo1, float4(0.0f, 1.0f, 0.0f, 1.0f));

    // Pass 1: red=1, green=1, blue=1 -> WHITE.
    api.beginFrame(0, 0, 0);
    api.bindDescriptorSet(ds, 0, {});
    drawToTarget(rt1.rt, ps, tri.rp);

    // Orphan ubo0 while Pass 1 is in flight. Put sentinel=(1,1,1,1) at offset 0 and
    // active=(0,0,0,1) at offset 256 so Pass 2 fails unless cloneSet() preserves offset=256 AND
    // copies ubo1 & tex.
    uploadBuffer(api, ubo0, SubrangeBuffer{
        .sentinel = float4(1.0f, 1.0f, 1.0f, 1.0f),
        .active = float4(0.0f, 0.0f, 0.0f, 1.0f),
    });
    drawToTarget(rt2.rt, ps, tri.rp);

    // Clone a second time while both earlier VkDescriptorSets are still in flight (exercises
    // recycleRetiredSets() retaining in-flight sets).
    uploadBuffer(api, ubo0, SubrangeBuffer{
        .sentinel = float4(0.0f),
        .active = float4(1.0f, 0.0f, 0.0f, 1.0f),
    });
    drawToTarget(rt3.rt, ps, tri.rp);

    api.commit(swapChain);
    api.endFrame(0);

    ReadbackResult r1, r2, r3;
    queueReadback(api, rt1.rt, r1);
    queueReadback(api, rt2.rt, r2);
    queueReadback(api, rt3.rt, r3);
    flushAndWait();

    expectSolidColor(r1, { 255, 255, 255, 255 }, "Pass 1 (WHITE)");
    expectSolidColor(r2, { 0, 255, 255, 255 }, "Pass 2 (CYAN: subrange + copied UBO/sampler)");
    expectSolidColor(r3, { 255, 255, 255, 255 }, "Pass 3 (WHITE after second clone)");
}

TEST_F(VulkanStagingBufferBypassTest, DescriptorRebindingAcrossLayoutChangeAndSharedSet) {
    SKIP_IF_NOT(Backend::VULKAN, "Tests Vulkan staging buffer bypass");

    static const char* const fsTwoSets = R"(#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
layout(set = 0, binding = 0, std140) uniform Params0 {
    vec4 color;
} params0;
layout(set = 1, binding = 0, std140) uniform Params1 {
    vec4 color;
} params1;
void main() {
    fragColor = vec4(params0.color.r, params1.color.g, 0.0, 1.0);
})";

    static const char* const fsThreeSets = R"(#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
layout(set = 0, binding = 0, std140) uniform Params0 {
    vec4 color;
} params0;
layout(set = 1, binding = 0, std140) uniform Params1 {
    vec4 color;
} params1;
layout(set = 2, binding = 0, std140) uniform Params2 {
    vec4 color;
} params2;
void main() {
    fragColor = vec4(params0.color.r, params1.color.g, params2.color.b, 1.0);
})";

    auto& api = getDriverApi();
    auto swapChain = addCleanup(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    auto tri = createFullScreenTriangle(api, *mCleanup);
    auto dsl = createLayout(api, *mCleanup, {
        { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::FRAGMENT, 0 },
    });

    auto ubo0 = addCleanup(api.createBufferObject(
            sizeof(float4), BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
    auto ubo1 = addCleanup(api.createBufferObject(
            sizeof(float4), BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
    auto ubo2 = addCleanup(api.createBufferObject(
            sizeof(float4), BufferObjectBinding::UNIFORM, BufferUsage::STATIC));

    auto ds0 = addCleanup(api.createDescriptorSet(dsl));
    auto ds1 = addCleanup(api.createDescriptorSet(dsl));
    auto ds2 = addCleanup(api.createDescriptorSet(dsl));
    api.updateDescriptorSetBuffer(ds0, 0, ubo0, 0, sizeof(float4));
    api.updateDescriptorSetBuffer(ds1, 0, ubo1, 0, sizeof(float4));
    api.updateDescriptorSetBuffer(ds2, 0, ubo2, 0, sizeof(float4));

    auto progTwoSets = makeVulkanProgram(fsTwoSets, {
        { 0, { { "Params0", DescriptorType::UNIFORM_BUFFER, 0 } } },
        { 1, { { "Params1", DescriptorType::UNIFORM_BUFFER, 0 } } },
    });
    auto progThreeSets = makeVulkanProgram(fsThreeSets, {
        { 0, { { "Params0", DescriptorType::UNIFORM_BUFFER, 0 } } },
        { 1, { { "Params1", DescriptorType::UNIFORM_BUFFER, 0 } } },
        { 2, { { "Params2", DescriptorType::UNIFORM_BUFFER, 0 } } },
    });

    PipelineState psTwoSets = getColorWritePipelineState();
    psTwoSets.program = progTwoSets;
    psTwoSets.primitiveType = PrimitiveType::TRIANGLES;
    psTwoSets.vertexBufferInfo = tri.vbi;
    psTwoSets.pipelineLayout.setLayout[0] = { dsl };
    psTwoSets.pipelineLayout.setLayout[1] = { dsl };

    PipelineState psThreeSets = getColorWritePipelineState();
    psThreeSets.program = progThreeSets;
    psThreeSets.primitiveType = PrimitiveType::TRIANGLES;
    psThreeSets.vertexBufferInfo = tri.vbi;
    psThreeSets.pipelineLayout.setLayout[0] = { dsl };
    psThreeSets.pipelineLayout.setLayout[1] = { dsl };
    psThreeSets.pipelineLayout.setLayout[2] = { dsl };

    auto rtShared1 = createOffscreenTarget(api, *mCleanup);
    auto rtShared2 = createOffscreenTarget(api, *mCleanup);
    auto rtCompat1 = createOffscreenTarget(api, *mCleanup);
    auto rtCompat2 = createOffscreenTarget(api, *mCleanup);

    api.beginFrame(0, 0, 0);

    // Part A: Same DescriptorSet bound at both set 0 and set 1.
    uploadBuffer(api, ubo0, float4(1.0f, 0.0f, 0.0f, 1.0f));
    api.bindDescriptorSet(ds0, 0, {});
    api.bindDescriptorSet(ds0, 1, {});
    drawToTarget(rtShared1.rt, psTwoSets, tri.rp);

    // Orphan ubo0 and draw again without rebinding either index: commit() must refresh ds0 once
    // and then re-bind the new VkDescriptorSet at BOTH set 0 and set 1.
    uploadBuffer(api, ubo0, float4(0.0f, 1.0f, 0.0f, 1.0f));
    drawToTarget(rtShared2.rt, psTwoSets, tri.rp);

    // Part B: Pipeline layout change with compatible sets 0 & 1 and a stale UBO on set 0.
    uploadBuffer(api, ubo0, float4(1.0f, 0.0f, 0.0f, 1.0f));
    uploadBuffer(api, ubo1, float4(0.0f, 1.0f, 0.0f, 1.0f));
    uploadBuffer(api, ubo2, float4(0.0f, 0.0f, 1.0f, 1.0f));
    api.bindDescriptorSet(ds0, 0, {});
    api.bindDescriptorSet(ds1, 1, {});
    drawToTarget(rtCompat1.rt, psTwoSets, tri.rp);

    // Orphan ubo0, switch to the 3-set pipeline layout, and only bind set 2 (leaving sets 0 and
    // 1 carried forward). Set 0 must clone and rebind, and rebindHigher must rebind set 1 too.
    uploadBuffer(api, ubo0, float4(0.0f, 0.0f, 0.0f, 1.0f));
    api.bindDescriptorSet(ds2, 2, {});
    drawToTarget(rtCompat2.rt, psThreeSets, tri.rp);

    api.commit(swapChain);
    api.endFrame(0);

    ReadbackResult rShared1, rShared2, rCompat1, rCompat2;
    queueReadback(api, rtShared1.rt, rShared1);
    queueReadback(api, rtShared2.rt, rShared2);
    queueReadback(api, rtCompat1.rt, rCompat1);
    queueReadback(api, rtCompat2.rt, rCompat2);
    flushAndWait();

    expectSolidColor(rShared1, { 255, 0, 0, 255 }, "Shared set Pass 1 (RED)");
    expectSolidColor(rShared2, { 0, 255, 0, 255 }, "Shared set Pass 2 (GREEN)");
    expectSolidColor(rCompat1, { 255, 255, 0, 255 }, "Layout compat Pass 1 (YELLOW)");
    expectSolidColor(rCompat2, { 0, 255, 255, 255 }, "Layout compat Pass 2 (CYAN)");
}

TEST_F(VulkanStagingBufferBypassTest, VertexAndIndexBufferInFlightSynchronization) {
    SKIP_IF_NOT(Backend::VULKAN, "Tests Vulkan staging buffer bypass");

    static const char* const fs = R"(#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
void main() {
    fragColor = vec4(1.0, 1.0, 1.0, 1.0);
})";

    static constexpr float2 kFullScreenVerts[3] = {
        { -1.0f, -1.0f },
        {  3.0f, -1.0f },
        { -1.0f,  3.0f },
    };
    static constexpr float2 kOffscreenVerts[3] = {
        { 2.0f, 2.0f },
        { 3.0f, 2.0f },
        { 2.0f, 3.0f },
    };
    static constexpr uint32_t kValidIndices[3] = { 0, 1, 2 };
    static constexpr uint32_t kDegenerateIndices[3] = { 0, 0, 0 };

    auto& api = getDriverApi();
    auto swapChain = addCleanup(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    auto tri = createFullScreenTriangle(api, *mCleanup);
    auto program = makeVulkanProgram(fs, {});

    PipelineState ps = getColorWritePipelineState();
    ps.program = program;
    ps.primitiveType = PrimitiveType::TRIANGLES;
    ps.vertexBufferInfo = tri.vbi;

    auto rt1 = createOffscreenTarget(api, *mCleanup);
    auto rt2 = createOffscreenTarget(api, *mCleanup);
    auto rt3 = createOffscreenTarget(api, *mCleanup);

    // Pass 1 draws with the fullscreen triangle -> WHITE.
    api.beginFrame(0, 0, 0);
    drawToTarget(rt1.rt, ps, tri.rp);

    // While Pass 1's command buffer still holds references to VB and IB, update both. Because
    // vertex and index buffers cannot be orphaned, age tracking must prevent a direct memcpy over
    // Pass 1's data and fall back to a synchronized staging copy.
    uploadRawBuffer(api, tri.vbo, kOffscreenVerts, 3);
    uploadIndices(api, tri.ib, kDegenerateIndices, 3);
    drawToTarget(rt2.rt, ps, tri.rp);

    api.commit(swapChain);
    api.endFrame(0);
    flushAndWait();

    // After finish() releases all command buffer ages, update VB and IB again (bypass path when
    // mappable) and verify Pass 3 draws WHITE again.
    uploadRawBuffer(api, tri.vbo, kFullScreenVerts, 3);
    uploadIndices(api, tri.ib, kValidIndices, 3);

    api.beginFrame(0, 0, 1);
    drawToTarget(rt3.rt, ps, tri.rp);
    api.commit(swapChain);
    api.endFrame(1);

    ReadbackResult r1, r2, r3;
    queueReadback(api, rt1.rt, r1);
    queueReadback(api, rt2.rt, r2);
    queueReadback(api, rt3.rt, r3);
    flushAndWait();

    expectSolidColor(r1, { 255, 255, 255, 255 }, "Pass 1 (WHITE)");
    expectSolidColor(r2, { 0, 0, 0, 255 }, "Pass 2 (BLACK after in-flight VB/IB update)");
    expectSolidColor(r3, { 255, 255, 255, 255 }, "Pass 3 (WHITE after idle VB/IB restore)");
}

TEST_F(VulkanStagingBufferBypassTest, ManyFlushesCyclePastAgeWindow) {
    SKIP_IF_NOT(Backend::VULKAN, "Tests Vulkan staging buffer bypass");

    static const char* const fs = R"(#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
layout(set = 0, binding = 0, std140) uniform Params {
    vec4 color;
} params;
void main() {
    fragColor = params.color;
})";

    auto& api = getDriverApi();
    auto swapChain = addCleanup(createSwapChain());
    api.makeCurrent(swapChain, swapChain);

    auto tri = createFullScreenTriangle(api, *mCleanup);
    auto dsl = createLayout(api, *mCleanup, {
        { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::FRAGMENT, 0 },
    });
    auto ds = addCleanup(api.createDescriptorSet(dsl));
    auto ubo = addCleanup(api.createBufferObject(
            sizeof(float4), BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
    api.updateDescriptorSetBuffer(ds, 0, ubo, 0, sizeof(float4));

    auto program = makeVulkanProgram(fs, {
        { 0, { { "Params", DescriptorType::UNIFORM_BUFFER, 0 } } },
    });

    PipelineState ps = getColorWritePipelineState();
    ps.program = program;
    ps.primitiveType = PrimitiveType::TRIANGLES;
    ps.vertexBufferInfo = tri.vbi;
    ps.pipelineLayout.setLayout[0] = { dsl };

    auto rt = createOffscreenTarget(api, *mCleanup);

    // Submit more command buffers than AgeSet::CAPACITY (128) and CommandBufferAgeCounter::
    // MAX_ACTIVE (90) without finish(), exercising pool reclamation, age window shifting,
    // descriptor retirement, and buffer recycling.
    constexpr int kFlushes = 140;
    for (int i = 0; i < kFlushes; ++i) {
        bool const isLast = (i == kFlushes - 1);
        float4 const color = isLast ? float4(0.0f, 1.0f, 0.0f, 1.0f)
                                    : float4(1.0f, 0.0f, 0.0f, 1.0f);
        uploadBuffer(api, ubo, color);
        api.beginFrame(0, 0, uint32_t(i));
        api.bindDescriptorSet(ds, 0, {});
        drawToTarget(rt.rt, ps, tri.rp);
        api.flush();
        api.endFrame(uint32_t(i));
        executeCommands();
    }

    ReadbackResult result;
    queueReadback(api, rt.rt, result);
    flushAndWait();

    expectSolidColor(result, { 0, 255, 0, 255 }, "Final draw after 140 flushes (GREEN)");
}

} // namespace test
