/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "VulkanResources.h"
#include "VulkanHandles.h"
#include "VulkanResourceAllocator.h"
#include "caching/VulkanDescriptorSet.h"

namespace filament::backend {

void deallocateResource(VulkanResourceAllocator* allocator, VulkanResourceType type,
        HandleBase::HandleId id) {

    if (IS_HEAP_ALLOC_TYPE(type)) {
        return;
    }

    switch (type) {
        case VulkanResourceType::BUFFER_OBJECT:
            allocator->destruct<VulkanBufferObject>(Handle<HwBufferObject>(id));
            break;
        case VulkanResourceType::INDEX_BUFFER:
            allocator->destruct<VulkanIndexBuffer>(Handle<HwIndexBuffer>(id));
            break;
        case VulkanResourceType::PROGRAM:
            allocator->destruct<VulkanProgram>(Handle<HwProgram>(id));
            break;
        case VulkanResourceType::RENDER_TARGET:
            allocator->destruct<VulkanRenderTarget>(Handle<HwRenderTarget>(id));
            break;
        case VulkanResourceType::SAMPLER_GROUP:
            allocator->destruct<VulkanSamplerGroup>(Handle<HwSamplerGroup>(id));
            break;
        case VulkanResourceType::SWAP_CHAIN:
            allocator->destruct<VulkanSwapChain>(Handle<HwSwapChain>(id));
            break;
        case VulkanResourceType::TEXTURE:
            allocator->destruct<VulkanTexture>(Handle<HwTexture>(id));
            break;
        case VulkanResourceType::TIMER_QUERY:
            allocator->destruct<VulkanTimerQuery>(Handle<HwTimerQuery>(id));
            break;
        case VulkanResourceType::VERTEX_BUFFER_INFO:
            allocator->destruct<VulkanVertexBufferInfo>(Handle<HwVertexBufferInfo>(id));
            break;
        case VulkanResourceType::VERTEX_BUFFER:
            allocator->destruct<VulkanVertexBuffer>(Handle<HwVertexBuffer>(id));
            break;
        case VulkanResourceType::RENDER_PRIMITIVE:
            allocator->destruct<VulkanRenderPrimitive>(Handle<VulkanRenderPrimitive>(id));
            break;
        case VulkanResourceType::DESCRIPTOR_SET:
            allocator->destruct<VulkanDescriptorSet>(Handle<VulkanDescriptorSet>(id));
            break;

        // If the resource is heap allocated, then the resource manager just skip refcounted
        // destruction.
        case VulkanResourceType::FENCE:
        case VulkanResourceType::HEAP_ALLOCATED:
            break;
    }
}

} // namespace filament::backend
