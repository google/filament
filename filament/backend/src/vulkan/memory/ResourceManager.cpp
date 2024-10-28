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

#include <utils/Panic.h>

namespace filament::backend::fvkmemory {

namespace {
#if FVK_ENABLED(FVK_DEBUG_ALLOCATION)
uint32_t COUNTER[(size_t) ResourceType::UNDEFINED_TYPE] = {};
#endif
}

ResourceManager::ResourceManager(size_t arenaSize, bool disableUseAfterFreeCheck)
    : mHandleAllocatorImpl("Handles", arenaSize, disableUseAfterFreeCheck) {}

void ResourceManager::gc() noexcept {
    auto destroyAll = [this](GcList& list) {
        for (auto const& [type, id]: list) {
            destroyWithType(type, id);
        }
        list.clear();
    };

    {
        // Note that we're not copying mThreadSafeGcList because the objects here do not have
        // resource_ptrs to other handle objects, so their desctruction would not add more elements
        // to mThreadSafeGcList.
        std::unique_lock<utils::Mutex> lock(mThreadSafeGcListMutex);
        destroyAll(mThreadSafeGcList);
    }

    GcList gcs;
    gcs.insert(gcs.end(), mGcList.begin(), mGcList.end());
    mGcList.clear();
    destroyAll(gcs);
}

void ResourceManager::terminate() noexcept {
    while (!mThreadSafeGcList.empty() || !mGcList.empty()) {
        gc();
    }
}

void ResourceManager::destroyWithType(ResourceType type, HandleId id) {
    switch (type) {
        case ResourceType::BUFFER_OBJECT:
            destruct<VulkanBufferObject>(Handle<VulkanBufferObject>(id));
            break;
        case ResourceType::INDEX_BUFFER:
            destruct<VulkanIndexBuffer>(Handle<VulkanIndexBuffer>(id));
            break;
        case ResourceType::PROGRAM:
            destruct<VulkanProgram>(Handle<VulkanProgram>(id));
            break;
        case ResourceType::RENDER_TARGET:
            destruct<VulkanRenderTarget>(Handle<VulkanRenderTarget>(id));
            break;
        case ResourceType::SWAP_CHAIN:
            destruct<VulkanSwapChain>(Handle<VulkanSwapChain>(id));
            break;
        case ResourceType::RENDER_PRIMITIVE:
            destruct<VulkanRenderPrimitive>(Handle<VulkanRenderPrimitive>(id));
            break;
        case ResourceType::TEXTURE:
            destruct<VulkanTexture>(Handle<VulkanTexture>(id));
            break;
        case ResourceType::TEXTURE_STATE:
            destruct<VulkanTextureState>(Handle<VulkanTextureState>(id));
            break;
        case ResourceType::TIMER_QUERY:
            destruct<VulkanTimerQuery>(Handle<VulkanTimerQuery>(id));
            break;
        case ResourceType::VERTEX_BUFFER:
            destruct<VulkanVertexBuffer>(Handle<VulkanVertexBuffer>(id));
            break;
        case ResourceType::VERTEX_BUFFER_INFO:
            destruct<VulkanVertexBufferInfo>(Handle<VulkanVertexBufferInfo>(id));
            break;
        case ResourceType::DESCRIPTOR_SET_LAYOUT:
            destruct<VulkanDescriptorSetLayout>(Handle<VulkanDescriptorSetLayout>(id));
            break;
        case ResourceType::DESCRIPTOR_SET:
            destruct<VulkanDescriptorSet>(Handle<VulkanDescriptorSet>(id));
            break;
        case ResourceType::FENCE:
            destruct<VulkanFence>(Handle<VulkanFence>(id));
            break;
        case ResourceType::UNDEFINED_TYPE:
            break;
    }
#if FVK_ENABLED(FVK_DEBUG_ALLOCATION)
    COUNTER[(size_t) type]--;
#endif
}

void ResourceManager::traceConstruction(ResourceType type, HandleId id) {
#if FVK_ENABLED(FVK_DEBUG_ALLOCATION)
    assert_invariant(type != ResourceType::UNDEFINED_TYPE);
    COUNTER[(size_t) type]++;
#endif
}

void ResourceManager::print() const noexcept {
#if FVK_ENABLED(FVK_DEBUG_ALLOCATION)
    utils::slog.e << "-------------------" << utils::io::endl;
    for (size_t i = 0; i < (size_t) ResourceType::UNDEFINED_TYPE; ++i) {
        utils::slog.e <<"    " << getTypeStr((ResourceType) i) << "=" << COUNTER[i] << utils::io::endl;
    }
    utils::slog.e << "+++++++++++++++++++" << utils::io::endl;
#endif
}

}
