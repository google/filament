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
#include "VulkanContext.h"

#include <bluevk/BlueVK.h>

#include <utils/debug.h>
#include <utils/Mutex.h>
#include <utils/Panic.h>

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace filament::backend {

using namespace bluevk;

namespace {

// Returns true if the command buffer associated with the fence has completed execution. A null
// fence denotes a timestamp that was never recorded, which trivially counts as complete.
bool isComplete(VkDevice device, std::shared_ptr<VulkanCmdFence> const& fence) {
    if (!fence) {
        return true;
    }
    VkResult status = fence->getStatus();
    if (status == VK_NOT_READY) {
        fence->refreshStatus(device);
        status = fence->getStatus();
    }
    return status == VK_SUCCESS;
}

} // anonymous namespace

VulkanQueryManager::VulkanQueryManager(VulkanContext const& context, VkDevice device)
        : mDevice(device),
          mHostQueryReset(context.isHostQueryResetSupported() && vkResetQueryPoolEXT != nullptr) {
    // Create a timestamp pool large enough to hold PAIR_COUNT pairs of queries.
    VkQueryPoolCreateInfo const tqpCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .queryType = VK_QUERY_TYPE_TIMESTAMP,
        .queryCount = QUERY_COUNT,
    };
    VkResult const result = vkCreateQueryPool(mDevice, &tqpCreateInfo, VKALLOC, &mPool);
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS) << "vkCreateQueryPool failed."
                                                       << " error=" << static_cast<int32_t>(result);

    // Queries start out uninitialized and must be reset before their first use. With host reset,
    // every pair on the free list is kept in the reset state, so reset the whole pool up front.
    // Otherwise, beginQuery resets each pair in the command buffer before writing to it.
    if (mHostQueryReset) {
        vkResetQueryPoolEXT(mDevice, mPool, 0, QUERY_COUNT);
    }

    for (uint32_t pair = 0; pair < PAIR_COUNT; ++pair) {
        mFreePairs.set(pair);
    }
    // Every pair is either free, bound to a timer query, or retired, so this never reallocates.
    mRetiredPairs.reserve(PAIR_COUNT);
}

VulkanQueryManager::~VulkanQueryManager() = default;

void VulkanQueryManager::clearQuery(fvkmemory::resource_ptr<VulkanTimerQuery> query) {
    VulkanTimerQuery* const tq = query.get();
    tq->mActive = false;
    VulkanQueryPair previous;
    {
        utils::LockGuard const lock(tq->mMutex);
        previous = std::exchange(tq->mPair, {});
        tq->mFailed = false;
    }
    // The timer query no longer holds any fences, so its deferred destruction is trivial.
    retire(std::move(previous));
}

void VulkanQueryManager::retire(VulkanQueryPair&& pair) {
    if (pair.index == VulkanQueryPair::INVALID) {
        return;
    }
    mRetiredPairs.push_back(std::move(pair));
}

void VulkanQueryManager::reclaimRetiredPairs() {
    std::erase_if(mRetiredPairs, [this](VulkanQueryPair const& pair) {
        // Both fences are checked because the start and stop timestamps may have been recorded in
        // different command buffers, and separate submissions are not guaranteed to complete in
        // order.
        if (!isComplete(mDevice, pair.starting) || !isComplete(mDevice, pair.stopping)) {
            return false;
        }
        // Every command buffer that wrote to this pair has completed, which satisfies
        // VUID-vkResetQueryPool-firstQuery-02741. The pair is no longer bound to any timer query,
        // so getResult() cannot be reading it, which satisfies
        // VUID-vkResetQueryPool-firstQuery-02742.
        if (mHostQueryReset) {
            vkResetQueryPoolEXT(mDevice, mPool, pair.index * 2, 2);
        }
        mFreePairs.set(pair.index);
        return true;
    });
}

void VulkanQueryManager::beginQuery(VulkanCommandBuffer const* commands,
        fvkmemory::resource_ptr<VulkanTimerQuery> query) {
    VulkanTimerQuery* const tq = query.get();

    // Reclaim before allocating, and retire the timer query's previous pair only after
    // allocating, so that a pair is never handed back out while its earlier writes might still be
    // in flight.
    reclaimRetiredPairs();

    uint32_t index = VulkanQueryPair::INVALID;
    if (!mFreePairs.empty()) {
        index = static_cast<uint32_t>(mFreePairs.firstSetBit());
        mFreePairs.unset(index);
    }
    bool const failed = index == VulkanQueryPair::INVALID;

    VulkanQueryPair previous;
    {
        utils::LockGuard const lock(tq->mMutex);
        previous = std::exchange(tq->mPair, {
            .index = index,
            .starting = failed ? nullptr : commands->getFenceStatus(),
            .stopping = nullptr,
        });
        tq->mFailed = failed;
    }
    retire(std::move(previous));

    // The timer query is marked active even on failure so that the matching endQuery is not
    // reported as unmatched.
    tq->mActive = true;

    if (failed) {
        FVK_LOGW << "No timestamp queries are available; the GPU is too far behind.";
        return;
    }

    VkCommandBuffer const cmdbuffer = commands->buffer();
    uint32_t const first = index * 2;
    if (!mHostQueryReset) {
        vkCmdResetQueryPool(cmdbuffer, mPool, first, 2);
    }
    vkCmdWriteTimestamp(cmdbuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, mPool, first);
}

void VulkanQueryManager::endQuery(VulkanCommandBuffer const* commands,
        fvkmemory::resource_ptr<VulkanTimerQuery> query) {
    VulkanTimerQuery* const tq = query.get();
    if (!tq->mActive) {
        FVK_LOGW << "endTimerQuery called without a matching beginTimerQuery.";
        return;
    }
    tq->mActive = false;

    utils::LockGuard const lock(tq->mMutex);
    VulkanQueryPair& pair = tq->mPair;
    if (pair.index == VulkanQueryPair::INVALID) {
        // beginQuery could not obtain a pair; getResult reports the error.
        return;
    }
    assert_invariant(pair.starting && !pair.stopping);
    vkCmdWriteTimestamp(commands->buffer(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, mPool,
            pair.index * 2 + 1);
    pair.stopping = commands->getFenceStatus();
}

VulkanQueryManager::QueryResult VulkanQueryManager::getResult(
        fvkmemory::resource_ptr<VulkanTimerQuery> query) {
    VulkanTimerQuery* const tq = query.get();

    utils::LockGuard const lock(tq->mMutex);
    if (tq->mFailed) {
        QueryResult result;
        result.error = true;
        return result;
    }

    // getTimerQueryValue is a synchronous call and might occur before both timestamp commands have
    // been processed, which is an error according to the validation layer that ships in the
    // Android NDK even when VK_QUERY_RESULT_WITH_AVAILABILITY_BIT is set. Notably,
    // FRenderer::endFrame commits the swapchain before calling endTimerQuery, splitting beginQuery
    // and endQuery across two command buffers without a semaphore link between them. Both fences
    // must therefore reach VK_SUCCESS before vkGetQueryPoolResults is called. Holding the timer
    // query's mutex across both the completion check and vkGetQueryPoolResults also prevents the
    // pair from being retired and reset on the backend thread
    // (VUID-vkResetQueryPool-firstQuery-02742). The fences are only borrowed: copying them here
    // could make this thread drop the last reference, but VulkanFencePool must only see fences
    // released on the backend thread.
    uint32_t const index = tq->mPair.index;
    VulkanCmdFence* const starting = tq->mPair.starting.get();
    VulkanCmdFence* const stopping = tq->mPair.stopping.get();
    if (index == VulkanQueryPair::INVALID || !starting || !stopping ||
            starting->getStatus() != VK_SUCCESS ||
            (stopping != starting && stopping->getStatus() != VK_SUCCESS)) {
        return {};
    }

    // Each query yields its timestamp followed by its availability.
    uint64_t data[4] = {};
    VkDeviceSize const stride = sizeof(uint64_t) * 2;
    VkResult const vkresult =
            vkGetQueryPoolResults(mDevice, mPool, index * 2, 2, sizeof(data), data, stride,
                    VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
    FILAMENT_CHECK_POSTCONDITION(vkresult == VK_SUCCESS || vkresult == VK_NOT_READY)
            << "vkGetQueryPoolResults error=" << static_cast<int32_t>(vkresult);
    if (vkresult == VK_NOT_READY) {
        return {};
    }
    return {
        .beginTime = data[0],
        .beginAvailable = data[1],
        .endTime = data[2],
        .endAvailable = data[3],
    };
}

void VulkanQueryManager::terminate() noexcept {
    // VulkanDriver::terminate calls finish() first, so every retired pair is idle. A timer query
    // that the client never destroyed still references its last pair, but it can no longer be
    // used once the driver is terminated.
    mRetiredPairs.clear();
    mFreePairs.reset();
    vkDestroyQueryPool(mDevice, mPool, VKALLOC);
    mPool = VK_NULL_HANDLE;
}

} // namespace filament::backend
