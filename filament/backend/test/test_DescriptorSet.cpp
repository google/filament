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
#include "ShaderGenerator.h"
#include "Skip.h"
#include "TrianglePrimitive.h"

#include <backend/DriverEnums.h>
#include <backend/Program.h>

namespace test {

using namespace filament;
using namespace filament::backend;

TEST_F(BackendTest, DescriptorSetHighBindingIndices) {
    auto& api = getDriverApi();

    // Create descriptor set layout with binding 0, high binding index 32, and 63.
    DescriptorSetLayout layout;
    layout.descriptors.reserve(3);
    layout.descriptors.push_back({
        .type = DescriptorType::UNIFORM_BUFFER,
        .stageFlags = ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,
        .binding = 0,
        .flags = DescriptorFlags::DYNAMIC_OFFSET,
    });
    layout.descriptors.push_back({
        .type = DescriptorType::UNIFORM_BUFFER,
        .stageFlags = ShaderStageFlags::FRAGMENT,
        .binding = 32,
        .flags = DescriptorFlags::NONE,
    });
    layout.descriptors.push_back({
        .type = DescriptorType::UNIFORM_BUFFER,
        .stageFlags = ShaderStageFlags::FRAGMENT,
        .binding = 63,
        .flags = DescriptorFlags::NONE,
    });

    auto dsl = addCleanup(api.createDescriptorSetLayout(std::move(layout)));
    EXPECT_TRUE(dsl);

    auto ds = addCleanup(api.createDescriptorSet(dsl));
    EXPECT_TRUE(ds);

    auto ubuffer = addCleanup(
            api.createBufferObject(256, BufferObjectBinding::UNIFORM, BufferUsage::STATIC));

    api.updateDescriptorSetBuffer(ds, 0, ubuffer, 0, 256);
    api.updateDescriptorSetBuffer(ds, 32, ubuffer, 0, 256);
    api.updateDescriptorSetBuffer(ds, 63, ubuffer, 0, 256);

    flushAndWait();
}

TEST_F(BackendTest, DescriptorSetLargeBindingCount) {
    auto& api = getDriverApi();

    // Create descriptor set layout with 32 distinct bindings (exceeding former Vulkan MAX_BINDINGS
    // = 25).
    DescriptorSetLayout layout;
    layout.descriptors.reserve(32);
    for (descriptor_binding_t i = 0; i < 32; ++i) {
        layout.descriptors.push_back({
            .type = DescriptorType::UNIFORM_BUFFER,
            .stageFlags = ShaderStageFlags::FRAGMENT,
            .binding = i,
            .flags = DescriptorFlags::NONE,
        });
    }

    auto dsl = addCleanup(api.createDescriptorSetLayout(std::move(layout)));
    EXPECT_TRUE(dsl);

    auto ds = addCleanup(api.createDescriptorSet(dsl));
    EXPECT_TRUE(ds);

    auto ubuffer = addCleanup(
            api.createBufferObject(256, BufferObjectBinding::UNIFORM, BufferUsage::STATIC));

    for (descriptor_binding_t i = 0; i < 32; ++i) {
        api.updateDescriptorSetBuffer(ds, i, ubuffer, 0, 256);
    }

    flushAndWait();
}

TEST_F(BackendTest, DescriptorSetSamplerHighBindingIndex) {
    auto& api = getDriverApi();

    DescriptorSetLayout layout;
    layout.descriptors.reserve(2);
    layout.descriptors.push_back({
        .type = DescriptorType::SAMPLER_2D_FLOAT,
        .stageFlags = ShaderStageFlags::FRAGMENT,
        .binding = 31,
    });
    layout.descriptors.push_back({
        .type = DescriptorType::SAMPLER_2D_FLOAT,
        .stageFlags = ShaderStageFlags::FRAGMENT,
        .binding = 63,
    });

    auto dsl = addCleanup(api.createDescriptorSetLayout(std::move(layout)));
    EXPECT_TRUE(dsl);

    auto ds = addCleanup(api.createDescriptorSet(dsl));
    EXPECT_TRUE(ds);

    auto texture = addCleanup(api.createTexture(SamplerType::SAMPLER_2D, 1, TextureFormat::RGBA8, 1,
            16, 16, 1, TextureUsage::SAMPLEABLE));

    api.updateDescriptorSetTexture(ds, 31, texture, {});
    api.updateDescriptorSetTexture(ds, 63, texture, {});

    flushAndWait();
}

// Intentionally exercises invalid DriverApi usage (an empty DescriptorSetLayout with 0 bindings)
// that can be reached when loading a malformed or malicious .filamat whose
// ChunkDescriptorSetLayoutInfo declares 0 descriptors (or fails to unflatten in release builds).
TEST_F(BackendTest, DescriptorSetEmptyLayout) {
    auto& api = getDriverApi();

    DescriptorSetLayout emptyLayout;
    auto dsl = addCleanup(api.createDescriptorSetLayout(std::move(emptyLayout)));
    EXPECT_TRUE(dsl);

    auto ds = addCleanup(api.createDescriptorSet(dsl));
    EXPECT_TRUE(ds);

    flushAndWait();
}

// Intentionally exercises invalid DriverApi usage where the Program's active descriptor bindings
// (ChunkDescriptorBindingsInfo) do not match the bound DescriptorSetLayout
// (ChunkDescriptorSetLayoutInfo), as can happen when loading a malformed or malicious .filamat.
TEST_F(BackendTest, DescriptorSetMismatchedProgramBindings) {
    SKIP_IF(Backend::METAL, "Tests OpenGL program/descriptor-set layout binding mismatch handling");
    SKIP_IF(Backend::VULKAN, "Tests OpenGL program/descriptor-set layout binding mismatch handling");
    SKIP_IF(Backend::WEBGPU, "Tests OpenGL program/descriptor-set layout binding mismatch handling");

    auto& api = getDriverApi();

    static const char* const vs = R"(#version 450 core
layout(location = 0) in vec4 mesh_position;
void main() {
    gl_Position = vec4(mesh_position.xy, 0.0, 1.0);
})";

    static const char* const fs = R"(#version 450 core
precision mediump int; precision highp float;
layout(location = 0) out vec4 fragColor;
layout(binding = 0, std140) uniform Params {
    vec4 color;
} params;
layout(binding = 1) uniform sampler2D u_gapSampler;
layout(binding = 2) uniform sampler2D u_oobSampler;
void main() {
    fragColor = params.color + texture(u_gapSampler, vec2(0.5)) + texture(u_oobSampler, vec2(0.5));
})";

    {
        auto swapChain = addCleanup(createSwapChain());
        api.makeCurrent(swapChain, swapChain);

        filamat::DescriptorSets descriptors;
        descriptors[0] = filamat::DescriptorSetInfo(3);
        descriptors[0][0] = {
            "Params",
            { DescriptorType::UNIFORM_BUFFER, ShaderStageFlags::FRAGMENT, 0 },
            {},
        };
        descriptors[0][1] = {
            "u_gapSampler",
            { DescriptorType::SAMPLER_2D_FLOAT, ShaderStageFlags::FRAGMENT, 1 },
            {},
        };
        descriptors[0][2] = {
            "u_oobSampler",
            { DescriptorType::SAMPLER_2D_FLOAT, ShaderStageFlags::FRAGMENT, 2 },
            {},
        };

        ShaderGenerator shaderGen(vs, fs, sBackend, sIsMobilePlatform, std::move(descriptors));
        Program prog = shaderGen.getProgram(api);

        // Simulate a malicious .filamat where ChunkDescriptorBindingsInfo declares active
        // bindings at a sparse gap (5) and far out of bounds (63) relative to
        // ChunkDescriptorSetLayoutInfo.
        Program::DescriptorBindingsInfo mismatchedBindings(3);
        mismatchedBindings[0] = { "Params", DescriptorType::UNIFORM_BUFFER, 0 };
        mismatchedBindings[1] = { "u_gapSampler", DescriptorType::SAMPLER_2D_FLOAT, 5 };
        mismatchedBindings[2] = { "u_oobSampler", DescriptorType::SAMPLER_2D_FLOAT, 63 };
        prog.descriptorBindings(0, mismatchedBindings);

        ProgramHandle program = addCleanup(api.createProgram(std::move(prog)));

        // Case 1: Small layout of size 1 (only binding 0), so binding 5 and 63 are out-of-bounds.
        DescriptorSetLayout smallLayout;
        smallLayout.descriptors.reserve(1);
        smallLayout.descriptors.push_back({
            .type = DescriptorType::UNIFORM_BUFFER,
            .stageFlags = ShaderStageFlags::FRAGMENT,
            .binding = 0,
            .flags = DescriptorFlags::NONE,
        });
        auto smallDsl = addCleanup(api.createDescriptorSetLayout(std::move(smallLayout)));
        auto smallDs = addCleanup(api.createDescriptorSet(smallDsl));

        // Case 2: Sparse layout with bindings {0, 10} and a buffer-vs-sampler mismatch at
        // binding 5 in a separate layout.
        DescriptorSetLayout sparseLayout;
        sparseLayout.descriptors.reserve(2);
        sparseLayout.descriptors.push_back({
            .type = DescriptorType::UNIFORM_BUFFER,
            .stageFlags = ShaderStageFlags::FRAGMENT,
            .binding = 0,
            .flags = DescriptorFlags::NONE,
        });
        sparseLayout.descriptors.push_back({
            .type = DescriptorType::SAMPLER_2D_FLOAT,
            .stageFlags = ShaderStageFlags::FRAGMENT,
            .binding = 10,
            .flags = DescriptorFlags::NONE,
        });
        auto sparseDsl = addCleanup(api.createDescriptorSetLayout(std::move(sparseLayout)));
        auto sparseDs = addCleanup(api.createDescriptorSet(sparseDsl));

        // Case 3: Layout where binding 5 is a UNIFORM_BUFFER, while the program's BindingMap
        // recorded binding 5 as a SAMPLER_2D_FLOAT.
        DescriptorSetLayout typeMismatchLayout;
        typeMismatchLayout.descriptors.reserve(2);
        typeMismatchLayout.descriptors.push_back({
            .type = DescriptorType::UNIFORM_BUFFER,
            .stageFlags = ShaderStageFlags::FRAGMENT,
            .binding = 0,
            .flags = DescriptorFlags::NONE,
        });
        typeMismatchLayout.descriptors.push_back({
            .type = DescriptorType::UNIFORM_BUFFER,
            .stageFlags = ShaderStageFlags::FRAGMENT,
            .binding = 5,
            .flags = DescriptorFlags::NONE,
        });
        auto typeMismatchDsl = addCleanup(
                api.createDescriptorSetLayout(std::move(typeMismatchLayout)));
        auto typeMismatchDs = addCleanup(api.createDescriptorSet(typeMismatchDsl));

        auto ubuffer = addCleanup(
                api.createBufferObject(16, BufferObjectBinding::UNIFORM, BufferUsage::STATIC));
        auto texture = addCleanup(api.createTexture(SamplerType::SAMPLER_2D, 1,
                TextureFormat::RGBA8, 1, 4, 4, 1, TextureUsage::SAMPLEABLE));

        api.updateDescriptorSetBuffer(smallDs, 0, ubuffer, 0, 16);
        api.updateDescriptorSetBuffer(sparseDs, 0, ubuffer, 0, 16);
        api.updateDescriptorSetTexture(sparseDs, 10, texture, {});
        api.updateDescriptorSetBuffer(typeMismatchDs, 0, ubuffer, 0, 16);
        api.updateDescriptorSetBuffer(typeMismatchDs, 5, ubuffer, 0, 16);

        Handle<HwRenderTarget> renderTarget = addCleanup(api.createDefaultRenderTarget());
        TrianglePrimitive triangle(api);

        RenderPassParams params = getClearColorDepthRenderPass();
        params.viewport = getFullViewport();

        PipelineState ps = {};
        ps.program = program;
        ps.vertexBufferInfo = triangle.getVertexBufferInfo();
        ps.rasterState.colorWrite = true;
        ps.rasterState.depthWrite = false;

        api.beginFrame(0, 0, 0);
        api.beginRenderPass(renderTarget, params);
        api.bindRenderPrimitive(triangle.getRenderPrimitive());

        ps.pipelineLayout.setLayout[0] = { smallDsl };
        api.bindPipeline(ps);
        api.bindDescriptorSet(smallDs, 0, {});
        api.draw2(0, 3, 1);

        ps.pipelineLayout.setLayout[0] = { sparseDsl };
        api.bindPipeline(ps);
        api.bindDescriptorSet(sparseDs, 0, {});
        api.draw2(0, 3, 1);

        ps.pipelineLayout.setLayout[0] = { typeMismatchDsl };
        api.bindPipeline(ps);
        api.bindDescriptorSet(typeMismatchDs, 0, {});
        api.draw2(0, 3, 1);

        api.endRenderPass();
        api.commit(swapChain);
        api.endFrame(0);
    }

    flushAndWait();
}

} // namespace test

