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

#include "vulkan/memory/Resource.h"

#include "vulkan/VulkanHandles.h"

namespace filament::backend::fvkmemory {

template ResourceType getTypeEnum<VulkanBufferObject>() noexcept;
template ResourceType getTypeEnum<VulkanIndexBuffer>() noexcept;
template ResourceType getTypeEnum<VulkanProgram>() noexcept;
template ResourceType getTypeEnum<VulkanRenderTarget>() noexcept;
template ResourceType getTypeEnum<VulkanSwapChain>() noexcept;
template ResourceType getTypeEnum<VulkanRenderPrimitive>() noexcept;
template ResourceType getTypeEnum<VulkanTexture>() noexcept;
template ResourceType getTypeEnum<VulkanTextureState>() noexcept;
template ResourceType getTypeEnum<VulkanTimerQuery>() noexcept;
template ResourceType getTypeEnum<VulkanVertexBuffer>() noexcept;
template ResourceType getTypeEnum<VulkanVertexBufferInfo>() noexcept;
template ResourceType getTypeEnum<VulkanDescriptorSetLayout>() noexcept;
template ResourceType getTypeEnum<VulkanDescriptorSet>() noexcept;
template ResourceType getTypeEnum<VulkanFence>() noexcept;

template<typename D>
ResourceType getTypeEnum() noexcept {
    if constexpr (std::is_same_v<D, VulkanBufferObject>) {
        return ResourceType::BUFFER_OBJECT;
    }
    if constexpr (std::is_same_v<D, VulkanIndexBuffer>) {
        return ResourceType::INDEX_BUFFER;
    }
    if constexpr (std::is_same_v<D, VulkanProgram>) {
        return ResourceType::PROGRAM;
    }
    if constexpr (std::is_same_v<D, VulkanRenderTarget>) {
        return ResourceType::RENDER_TARGET;
    }
    if constexpr (std::is_same_v<D, VulkanSwapChain>) {
        return ResourceType::SWAP_CHAIN;
    }
    if constexpr (std::is_same_v<D, VulkanRenderPrimitive>) {
        return ResourceType::RENDER_PRIMITIVE;
    }
    if constexpr (std::is_same_v<D, VulkanTexture>) {
        return ResourceType::TEXTURE;
    }
    if constexpr (std::is_same_v<D, VulkanTextureState>) {
        return ResourceType::TEXTURE_STATE;
    }
    if constexpr (std::is_same_v<D, VulkanTimerQuery>) {
        return ResourceType::TIMER_QUERY;
    }
    if constexpr (std::is_same_v<D, VulkanVertexBuffer>) {
        return ResourceType::VERTEX_BUFFER;
    }
    if constexpr (std::is_same_v<D, VulkanVertexBufferInfo>) {
        return ResourceType::VERTEX_BUFFER_INFO;
    }
    if constexpr (std::is_same_v<D, VulkanDescriptorSetLayout>) {
        return ResourceType::DESCRIPTOR_SET_LAYOUT;
    }
    if constexpr (std::is_same_v<D, VulkanDescriptorSet>) {
        return ResourceType::DESCRIPTOR_SET;
    }
    if constexpr (std::is_same_v<D, VulkanFence>) {
        return ResourceType::FENCE;
    }
    return ResourceType::UNDEFINED_TYPE;
}

std::string getTypeStr(ResourceType type) {
    switch (type) {
        case ResourceType::BUFFER_OBJECT:
            return "BufferObject";
        case ResourceType::INDEX_BUFFER:
            return "IndexBuffer";
        case ResourceType::PROGRAM:
            return "Program";
        case ResourceType::RENDER_TARGET:
            return "RenderTarget";
        case ResourceType::SWAP_CHAIN:
            return "SwapChain";
        case ResourceType::RENDER_PRIMITIVE:
            return "RenderPrimitive";
        case ResourceType::TEXTURE:
            return "Texture";
        case ResourceType::TEXTURE_STATE:
            return "TextureState";
        case ResourceType::TIMER_QUERY:
            return "TimerQuery";
        case ResourceType::VERTEX_BUFFER:
            return "VertexBuffer";
        case ResourceType::VERTEX_BUFFER_INFO:
            return "VertexBufferInfo";
        case ResourceType::DESCRIPTOR_SET_LAYOUT:
            return "DescriptorSetLayout";
        case ResourceType::DESCRIPTOR_SET:
            return "DescriptorSet";
        case ResourceType::FENCE:
            return "Fence";
        case ResourceType::UNDEFINED_TYPE:
            return "";
    }
}

} // namespace filament::backend::fvkmemory
