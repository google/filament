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

#include <backend/DriverEnums.h>

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

} // namespace test
