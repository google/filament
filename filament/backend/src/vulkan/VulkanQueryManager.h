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

#ifndef TNT_FILAMENT_BACKEND_VULKANQUERYMANAGER_H
#define TNT_FILAMENT_BACKEND_VULKANQUERYMANAGER_H

#include "vulkan/memory/ResourcePointer.h"

namespace filament::backend {

struct VulkanCommandBuffer;
struct VulkanTimerQuery;

class VulkanQueryManager {
public:
    struct QueryResult {
        uint64_t beginTime = 0;
        uint64_t beginAvailable = 0;
        uint64_t endTime = 0;
        uint64_t endAvailable = 0;
    };

    VulkanQueryManager(VkDevice device);
    ~VulkanQueryManager() = default;

    // Not copy-able.
    VulkanQueryManager(VulkanQueryManager const&) = delete;
    VulkanQueryManager& operator=(VulkanQueryManager const&) = delete;

    fvkmemory::resource_ptr<VulkanTimerQuery>
            getNextQuery(fvkmemory::ResourceManager* mResourceManager);
    void clearQuery(fvkmemory::resource_ptr<VulkanTimerQuery> query);
    void beginQuery(VulkanCommandBuffer const* commands,
            fvkmemory::resource_ptr<VulkanTimerQuery> query);
    void endQuery(VulkanCommandBuffer const* commands,
            fvkmemory::resource_ptr<VulkanTimerQuery> query);
    QueryResult getResult(fvkmemory::resource_ptr<VulkanTimerQuery> query);

    void terminate() noexcept;

private:
    VkDevice mDevice;
    VkQueryPool mPool;
    utils::bitset32 mUsed;
    utils::Mutex mMutex;
};

} // filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANQUERYMANAGER_H
