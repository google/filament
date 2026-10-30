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

#include "VulkanAsyncHandles.h"

#include "vulkan/memory/ResourcePointer.h"

#include <bluevk/BlueVK.h>

#include <utils/bitset.h>

#include <cstdint>
#include <vector>

namespace filament::backend {

struct VulkanCommandBuffer;
struct VulkanContext;

// Manages timer queries backed by a single VkQueryPool of timestamp queries.
//
// The pool is divided into query pairs (VulkanQueryPair): two timestamp queries, one for the start
// and one for the stop of a measurement. A VulkanTimerQuery (the backend's HwTimerQuery) is not
// tied to fixed queries in the pool. Instead, every beginQuery binds a fresh query pair to the
// VulkanTimerQuery and retires the pair it previously held, so the number of live timer queries is
// not limited by the size of the pool.
//
// A retired pair is returned to the free list only after every command buffer that wrote to it has
// completed. As a result, a pair taken from the free list is never referenced by in-flight work,
// which makes host-side reset always valid (VUID-vkResetQueryPool-firstQuery-02741) when
// VK_EXT_host_query_reset is available.
//
// A pair is only ever read by getResult() while it is bound to a VulkanTimerQuery, and both the
// read and the unbinding happen under that VulkanTimerQuery's mutex. A retired pair can therefore
// no longer be read, which satisfies VUID-vkResetQueryPool-firstQuery-02742 without holding any
// lock during the host reset.
class VulkanQueryManager {
public:
    struct QueryResult {
        uint64_t beginTime = 0;
        uint64_t beginAvailable = 0;
        uint64_t endTime = 0;
        uint64_t endAvailable = 0;
        // Set when the most recent beginQuery could not obtain a pair of queries because the
        // pool was exhausted. The timestamps above are meaningless in that case.
        bool error = false;
    };

    VulkanQueryManager(VulkanContext const& context, VkDevice device);
    ~VulkanQueryManager();

    // Not copy-able.
    VulkanQueryManager(VulkanQueryManager const&) = delete;
    VulkanQueryManager& operator=(VulkanQueryManager const&) = delete;

    // Releases the query pair held by a timer query that is being destroyed. Backend thread only.
    void clearQuery(fvkmemory::resource_ptr<VulkanTimerQuery> query);
    // Backend thread only.
    void beginQuery(VulkanCommandBuffer const* commands,
            fvkmemory::resource_ptr<VulkanTimerQuery> query);
    // Backend thread only.
    void endQuery(VulkanCommandBuffer const* commands,
            fvkmemory::resource_ptr<VulkanTimerQuery> query);
    // May be called from any thread.
    QueryResult getResult(fvkmemory::resource_ptr<VulkanTimerQuery> query);

    void terminate() noexcept;

private:
    // Number of query pairs in the pool. A pair stays out of the free list from the beginQuery
    // that binds it until the command buffers that wrote to it complete, so the pool must cover
    // every timer query that is begun while earlier frames are still in flight. FrameInfoManager
    // uses 4 timer queries, so this leaves ample headroom. If the GPU falls so far behind that no
    // pair is free, beginQuery fails and getResult reports an error for that timer query.
    static constexpr uint32_t PAIR_COUNT = 96;
    static constexpr uint32_t QUERY_COUNT = PAIR_COUNT * 2;

    // Moves a pair that is no longer bound to any timer query to the retired list. Backend thread
    // only.
    void retire(VulkanQueryPair&& pair);

    // Returns every retired pair whose command buffers have completed to the free list, resetting
    // it on the host when supported. Backend thread only.
    void reclaimRetiredPairs();

    VkDevice mDevice;
    VkQueryPool mPool = VK_NULL_HANDLE;
    bool const mHostQueryReset;

    // The members below are accessed exclusively on the backend thread.
    utils::bitset128 mFreePairs;
    std::vector<VulkanQueryPair> mRetiredPairs;

    static_assert(PAIR_COUNT <= decltype(mFreePairs)::BIT_COUNT,
            "mFreePairs must have at least one bit per pair");
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANQUERYMANAGER_H
