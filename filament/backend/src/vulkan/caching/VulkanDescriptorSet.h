/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_CACHING_VULKANDESCRIPTORSET_H
#define TNT_FILAMENT_BACKEND_CACHING_VULKANDESCRIPTORSET_H

#include <vulkan/VulkanResourceAllocator.h>
#include <vulkan/VulkanTexture.h>
#include <vulkan/VulkanUtility.h>

#include <backend/DriverEnums.h>
#include <backend/Program.h>
#include <backend/TargetBufferInfo.h>

#include <utils/bitset.h>

#include <bluevk/BlueVK.h>
#include <tsl/robin_map.h>

namespace filament::backend {

// Abstraction over the pool and the layout cache.
class VulkanDescriptorSetManager {
public:
    using StageBitMask = uint32_t;
    using UniformBufferBitmask = StageBitMask;
    using SamplerBitmask = StageBitMask;

    static constexpr uint8_t CONCURRENT_DESCRIPTOR_SET_COUNT = 2;// UBO and samplers
    static constexpr uint8_t MAX_SUPPORTED_SHADER_STAGE = 2;     // Vertex and fragment.

    static_assert(sizeof(UniformBufferBitmask) * 8 >=
                  (Program::UNIFORM_BINDING_COUNT) *MAX_SUPPORTED_SHADER_STAGE);
    static_assert(sizeof(SamplerBitmask) * 8 >=
                  Program::SAMPLER_BINDING_COUNT * MAX_SUPPORTED_SHADER_STAGE);

    static constexpr StageBitMask VERTEX_STAGE = 0x1;
    static constexpr StageBitMask FRAGMENT_STAGE = (0x1 << (sizeof(StageBitMask) / 4));
    static constexpr uint8_t UBO_SET_INDEX = 0;
    static constexpr uint8_t SAMPLER_SET_INDEX = 1;

    using LayoutArray = std::array<VkDescriptorSetLayout, CONCURRENT_DESCRIPTOR_SET_COUNT>;
    struct SamplerBundle {
        VkDescriptorImageInfo info = {};
        VulkanTexture* texture = nullptr;
        uint8_t binding = 0;
        SamplerBitmask stage = 0;
    };
    using SamplerArray = CappedArray<SamplerBundle, Program::SAMPLER_BINDING_COUNT>;
    using GetPipelineLayoutFunction = std::function<VkPipelineLayout(LayoutArray)>;

    VulkanDescriptorSetManager(VkDevice device, VulkanResourceAllocator* resourceAllocator);

    void terminate() noexcept;

    void gc() noexcept;

    // This will write/update all of the descriptor set.
    void bind(VulkanCommandBuffer* commands, GetPipelineLayoutFunction& getPipelineFn);

    void setUniformBufferObject(uint32_t bindingIndex, VulkanBufferObject* bufferObject,
            VkDeviceSize offset, VkDeviceSize size) noexcept;

    void setSamplers(SamplerArray&& samplers);

private:
    class Impl;
    Impl* mImpl;
};

} // namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_CACHING_VULKANDESCRIPTORSET_H
