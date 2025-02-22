/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "VulkanQueryManager.h"

#include "VulkanAsyncHandles.h"
#include "VulkanCommands.h"
#include "VulkanConstants.h"

#include <bluevk/BlueVK.h>

namespace filament::backend {

using namespace bluevk;

VulkanQueryManager::VulkanQueryManager(VkDevice device) : mDevice(device) {
    // Create a timestamp pool large enough to hold a pair of queries for each timer.
    VkQueryPoolCreateInfo tqpCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .queryType = VK_QUERY_TYPE_TIMESTAMP,
        .queryCount = (uint32_t) mUsed.size() * 2,
    };
    VkResult result = vkCreateQueryPool(mDevice, &tqpCreateInfo, VKALLOC, &mPool);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "vkCreateQueryPool failed."
                                                       << " error=" << static_cast<int32_t>(result);
}

fvkmemory::resource_ptr<VulkanTimerQuery> VulkanQueryManager::getNextQuery(
        fvkmemory::ResourceManager* resourceManager) {
    auto unused = ~mUsed;
    if (unused.empty()) {
        FVK_LOGE << "More than " << mUsed.size() << " timers are not supported." << utils::io::endl;
        return {};
    }

    bool found = false;
    std::pair<uint32_t, uint32_t> queryIndices;
    unused.forEachSetBit([&](size_t index) {
        if (found) {
            return;
        }
        std::unique_lock<utils::Mutex> lock(mMutex);
        mUsed.set(index);
        found = true;
        queryIndices = std::make_pair(index * 2, index * 2 + 1);
    });
    return fvkmemory::resource_ptr<VulkanTimerQuery>::construct(resourceManager, queryIndices.first,
            queryIndices.second);
}

void VulkanQueryManager::clearQuery(fvkmemory::resource_ptr<VulkanTimerQuery> query) {
    std::unique_lock<utils::Mutex> lock(mMutex);
    uint32_t const startingIndex = query->getStartingQueryIndex();
    mUsed.unset(startingIndex / 2);
}

void VulkanQueryManager::beginQuery(VulkanCommandBuffer const* commands,
        fvkmemory::resource_ptr<VulkanTimerQuery> query) {
    uint32_t const index = query->getStartingQueryIndex();

    auto const cmdbuffer = commands->buffer();
    vkCmdResetQueryPool(cmdbuffer, mPool, index, 2);
    vkCmdWriteTimestamp(cmdbuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, mPool, index);

    // We stash this because getResult might come before the query is actually processed.
    query->setFence(commands->getFenceStatus());
}

void VulkanQueryManager::endQuery(VulkanCommandBuffer const* commands,
        fvkmemory::resource_ptr<VulkanTimerQuery> query) {
    uint32_t const index = query->getStoppingQueryIndex();
    vkCmdWriteTimestamp(commands->buffer(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, mPool, index);
}

VulkanQueryManager::QueryResult VulkanQueryManager::getResult(
        fvkmemory::resource_ptr<VulkanTimerQuery> query) {
    uint32_t const index = query->getStartingQueryIndex();
    QueryResult result;
    size_t const dataSize = sizeof(result);
    VkDeviceSize const stride = sizeof(uint64_t) * 2;
    VkResult vkresult =
            vkGetQueryPoolResults(mDevice, mPool, index, 2, dataSize, (void*) &result, stride,
                    VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
    FILAMENT_CHECK_POSTCONDITION(vkresult == VK_SUCCESS || vkresult == VK_NOT_READY)
            << "vkGetQueryPoolResults error=" << static_cast<int32_t>(vkresult);
    if (vkresult == VK_NOT_READY) {
        return {};
    }
    return result;
}

void VulkanQueryManager::terminate() noexcept {
    vkDestroyQueryPool(mDevice, mPool, VKALLOC);
    mPool = VK_NULL_HANDLE;
}

} // filament::backend
