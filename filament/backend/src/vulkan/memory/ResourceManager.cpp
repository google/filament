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

#include "vulkan/memory/ResourceManager.h"
#include "vulkan/VulkanHandles.h"

namespace filament::backend::fvkmemory {

namespace {
#if FVK_ENABLED(FVK_DEBUG_ALLOCATION)
uint32_t COUNTER[(size_t) ResourceType::UNDEFINED_TYPE] = {};
#endif
}

ResourceManager* ResourceManager::sSingleton = nullptr;

ResourceManager::ResourceManager(size_t arenaSize, bool disableUseAfterFreeCheck)
    : mHandleAllocatorImpl("Handles", arenaSize, disableUseAfterFreeCheck),
      mPool(arenaSize) {}

void ResourceManager::gcImpl() {
    std::unique_lock<utils::Mutex> lock(mGcListMutex);
    for (auto const& [type, id]: mGcList) {
        ResourceManager::destroyWithType(type, id);
    }
    mGcList.clear();
}

void ResourceManager::destroyWithType(ResourceType type, HandleId id) {
    auto s = sSingleton;
    switch (type) {
        case ResourceType::BUFFER_OBJECT:
            s->destruct<VulkanBufferObject>(Handle<HwBufferObject>(id));
            break;
        case ResourceType::INDEX_BUFFER:
            s->destruct<VulkanIndexBuffer>(Handle<HwIndexBuffer>(id));
            break;
        case ResourceType::PROGRAM:
            s->destruct<VulkanProgram>(Handle<HwProgram>(id));
            break;
        case ResourceType::RENDER_TARGET:
            s->destruct<VulkanRenderTarget>(Handle<HwRenderTarget>(id));
            break;
        case ResourceType::SWAP_CHAIN:
            s->destruct<VulkanSwapChain>(Handle<HwSwapChain>(id));
            break;
        case ResourceType::RENDER_PRIMITIVE:
            s->destruct<VulkanRenderPrimitive>(Handle<VulkanRenderPrimitive>(id));
            break;
        case ResourceType::TEXTURE:
            s->destruct<VulkanTexture>(Handle<HwTexture>(id));
            break;
        case ResourceType::TEXTURE_STATE:
            s->destruct<VulkanTextureState>(Handle<VulkanTextureState>(id));
            break;
        case ResourceType::TIMER_QUERY:
            s->destruct<VulkanTimerQuery>(Handle<HwTimerQuery>(id));
            break;
        case ResourceType::VERTEX_BUFFER:
            s->destruct<VulkanVertexBuffer>(Handle<HwVertexBuffer>(id));
            break;
        case ResourceType::VERTEX_BUFFER_INFO:
            s->destruct<VulkanVertexBufferInfo>(Handle<HwVertexBufferInfo>(id));
            break;
        case ResourceType::DESCRIPTOR_SET_LAYOUT:
            s->destruct<VulkanDescriptorSetLayout>(Handle<VulkanDescriptorSetLayout>(id));
            break;
        case ResourceType::DESCRIPTOR_SET:
            s->destruct<VulkanDescriptorSet>(Handle<VulkanDescriptorSet>(id));
            break;
        case ResourceType::FENCE:
            s->destruct<VulkanFence>(Handle<VulkanFence>(id));
            break;
        case ResourceType::UNDEFINED_TYPE:
            break;
    }
#if FVK_ENABLED(FVK_DEBUG_ALLOCATION)
    COUNTER[(size_t) type]--;
#endif
}

void ResourceManager::trackIncrement(ResourceType type, HandleId id) {
#if FVK_ENABLED(FVK_DEBUG_ALLOCATION)
     COUNTER[(size_t) type]++;
#endif
}

void ResourceManager::printImpl() const {
#if FVK_ENABLED(FVK_DEBUG_ALLOCATION)
    utils::slog.e << "-------------------" << utils::io::endl;
    for (size_t i = 0; i < (size_t) ResourceType::UNDEFINED_TYPE; ++i) {
        utils::slog.e <<"    " << getTypeStr((ResourceType) i) << "=" << COUNTER[i] << utils::io::endl;
    }
    utils::slog.e << "+++++++++++++++++++" << utils::io::endl;
#endif
}

}
