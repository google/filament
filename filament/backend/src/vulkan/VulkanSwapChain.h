/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANSWAPCHAIN_H
#define TNT_FILAMENT_BACKEND_VULKANSWAPCHAIN_H

#include "DriverBase.h"

#include "VulkanContext.h"
#include "VulkanTexture.h"
#include "vulkan/memory/Resource.h"

#include <backend/platforms/VulkanPlatform.h>

#include <bluevk/BlueVK.h>
#include <utils/FixedCapacityVector.h>

#include <memory>

using namespace bluevk;

namespace filament::backend {

struct VulkanHeadlessSwapChain;
struct VulkanSurfaceSwapChain;
class VulkanCommands;

// A wrapper around the platform implementation of swapchain.
struct VulkanSwapChain : public HwSwapChain, fvkmemory::Resource {
    VulkanSwapChain(VulkanPlatform* platform, VulkanContext const& context,
            fvkmemory::ResourceManager* resourceManager, VmaAllocator allocator,
            VulkanCommands* commands, VulkanStagePool& stagePool, void* nativeWindow,
            uint64_t flags, VkExtent2D extent = {0, 0});

    ~VulkanSwapChain();

    void present();

    void acquire(bool& reized);

    fvkmemory::resource_ptr<VulkanTexture> getCurrentColor() const noexcept {
        uint32_t const imageIndex = mCurrentSwapIndex;
        FILAMENT_CHECK_PRECONDITION(
            imageIndex != VulkanPlatform::ImageSyncData::INVALID_IMAGE_INDEX);
        return mColors[imageIndex];
    }

    inline fvkmemory::resource_ptr<VulkanTexture> getDepth() const noexcept {
        return mDepth;
    }

    inline bool isFirstRenderPass() const noexcept {
        return mIsFirstRenderPass;
    }

    inline void markFirstRenderPass() noexcept {
        mIsFirstRenderPass = false;
    }

    inline VkExtent2D getExtent() noexcept {
        return mExtent;
    }

    inline bool isProtected() noexcept {
        return mPlatform->isProtected(swapChain);
    }
private:
	static constexpr int IMAGE_READY_SEMAPHORE_COUNT = FVK_MAX_COMMAND_BUFFERS;

    void update();

    VulkanPlatform* mPlatform;
    fvkmemory::ResourceManager* mResourceManager;
    VulkanCommands* mCommands;
    VmaAllocator mAllocator;
    VulkanStagePool& mStagePool;
    bool const mHeadless;
    bool const mFlushAndWaitOnResize;
    bool const mTransitionSwapChainImageLayoutForPresent;

    // We create VulkanTextures based on VkImages. VulkanTexture has facilities for doing layout
    // transitions, which are useful here.
    utils::FixedCapacityVector<fvkmemory::resource_ptr<VulkanTexture>> mColors;
    fvkmemory::resource_ptr<VulkanTexture> mDepth;
    VkExtent2D mExtent;
    uint32_t mCurrentSwapIndex;
    std::function<void(Platform::SwapChain* handle)> mExplicitImageReadyWait = nullptr;
    bool mAcquired;
    bool mIsFirstRenderPass;
};


}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_VULKANSWAPCHAIN_H
