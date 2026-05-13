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

#include "VulkanConstants.h"
#include "VulkanFencePool.h"

#include <algorithm>
#include <deque>
#include <utility>
#include <vector>

using namespace bluevk;
using namespace utils;

namespace filament::backend {

namespace {

static constexpr uint32_t TIME_BEFORE_EVICTION = 3;

}  // namespace

VulkanFencePool::VulkanFencePool(VulkanContext const& context,
                                 VkDevice device,
                                 uint32_t minPoolSize)
    : mContext(context), mDevice(device), mMinPoolSize(minPoolSize)
{
    for (size_t i = 0; i < minPoolSize; ++i) {
        VkFence fence = allocateFence();
        assert_invariant(fence != VK_NULL_HANDLE);
        if (fence != VK_NULL_HANDLE) {
            mFences.push_back(std::make_pair(mCurrFrame, fence));
        }
    }
    mNumFences = mFences.size();  // Only count the fences we successfully acquired.
}

std::shared_ptr<VulkanCmdFence> VulkanFencePool::acquireFenceStatus() noexcept {
    VkFence fence = acquireFence();
    std::function<void(VkFence)> recycleFn = [this](VkFence fence) {
        this->releaseFence(fence);
    };
    const auto fenceStatus = std::make_shared<VulkanCmdFence>(fence, recycleFn);
    // Store a weak pointer to the fence status, so we can clear the fence
    // later if terminate() is called.
    mFenceStatuses.push_back(std::weak_ptr<VulkanCmdFence>(fenceStatus));
    return fenceStatus;
}

void VulkanFencePool::gc() noexcept {
    // Clear any fence statuses that no longer exist.
    auto expiredStatuses = std::remove_if(mFenceStatuses.begin(), mFenceStatuses.end(),
        [](const auto& fenceStatus) {
            return fenceStatus.expired();
        });
    mFenceStatuses.erase(expiredStatuses, mFenceStatuses.end());

    if (++mCurrFrame <= TIME_BEFORE_EVICTION) {
        return;
    }

    while (mNumFences > mMinPoolSize && !mFences.empty() &&
           mFences.front().first + TIME_BEFORE_EVICTION < mCurrFrame) {
        destroyFence(mFences.front().second);
        mFences.pop_front();
        --mNumFences;
    }
}

void VulkanFencePool::terminate() noexcept {
    for (const auto& weakFenceStatus : mFenceStatuses) {
        if (const auto fenceStatus = weakFenceStatus.lock()) {
            // Swap out the recycle function for the fence status, so that
            // it doesn't try to return the fence to this pool.
            fenceStatus->swapRecycleFn([device = mDevice](VkFence fence) {
                vkDestroyFence(device, fence, VKALLOC);
            });
        }
    }
    mFenceStatuses.clear();

    for (const auto& fence : mFences) {
        destroyFence(fence.second);
    }
    mFences.clear();
    mNumFences = 0;
}

VkFence VulkanFencePool::acquireFence() noexcept {
    if (!mFences.empty()) {
        VkFence fence = mFences.back().second;
        mFences.pop_back();
        return fence;
    } else {
        VkFence fence = allocateFence();
        FILAMENT_CHECK_POSTCONDITION(fence != VK_NULL_HANDLE);
        ++mNumFences;
        return fence;
    }
}

void VulkanFencePool::releaseFence(VkFence fence) noexcept {
    // This only happens if fence allocation failed, which is a fairly
    // major issue.
    FILAMENT_CHECK_PRECONDITION(fence != VK_NULL_HANDLE);

    // Reset the fence before returning it to the pool of available
    // fences.
    vkResetFences(mDevice, 1, &fence);
    mFences.push_back(std::make_pair(mCurrFrame, fence));
}

VkFence VulkanFencePool::allocateFence() const noexcept {
    VkFence fence = VK_NULL_HANDLE;
    VkFenceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
    };
    VkExportFenceCreateInfo exportFenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO,
        .handleTypes = mContext.getFenceExportFlags()
    };
    // Necessary to guard this. Otherwise, swiftshader would throw an error.
    if (mContext.getFenceExportFlags()) {
        createInfo.pNext = &exportFenceCreateInfo;
    }
    VkResult res = vkCreateFence(mDevice, &createInfo, VKALLOC, &fence);
    if (res != VK_SUCCESS) {
        FVK_LOGE << "Failed to create fence: " << res << "; skipping. This may cause issues later.";
        return VK_NULL_HANDLE;
    }
    return fence;
}

void VulkanFencePool::destroyFence(VkFence fence) const noexcept {
    vkDestroyFence(mDevice, fence, VKALLOC);
}

} // namespace filament::backend
