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
#include "VulkanConstants.h"
#include "VulkanContext.h"
#include "VulkanTexture.h"

#include "vulkan/memory/Resource.h"

#include <backend/platforms/VulkanPlatform.h>

#include <bluevk/BlueVK.h>

#include <utils/FixedCapacityVector.h>

#include <memory>

using namespace bluevk;

namespace filament::backend {

class VulkanCommands;
struct VulkanFramebuffer;

// A wrapper around the platform implementation of swapchain.
struct VulkanSwapChain : public HwSwapChain, fvkmemory::Resource {
    VulkanSwapChain(VulkanPlatform* platform, VulkanContext const& context,
            fvkmemory::ResourceManager* resourceManager, VmaAllocator allocator,
            VulkanCommands* commands, VulkanStagePool& stagePool, void* nativeWindow,
            uint64_t flags, VkExtent2D extent = {0, 0});

    ~VulkanSwapChain();

    void present(DriverBase& driver);

    // Acquire a new image from the swapchain. Returns a pair:
    //   1. whether an acquire is successful.
    //   2. whether the backing images have changed (due to resize or other factors).
    std::pair<bool, bool> acquire();

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

    inline void setFrameScheduledCallback(CallbackHandler* handler,
            FrameScheduledCallback&& callback) noexcept {
        if (!callback) {
            mFrameScheduled.handler = nullptr;
            mFrameScheduled.callback.reset();
            return;
        }
        mFrameScheduled.handler = handler;
        mFrameScheduled.callback = std::make_shared<FrameScheduledCallback>(std::move(callback));
    }

    // Returns the framebuffer for the current swapchain image, or a null pointer if none has been
    // set yet.
    fvkmemory::resource_ptr<VulkanFramebuffer> getCurrentFramebuffer() const noexcept;

    // Sets the framebuffer for the current swapchain image. The swapchain keeps the framebuffer
    // until the backing images change or the swapchain is destroyed.
    void setCurrentFramebuffer(fvkmemory::resource_ptr<VulkanFramebuffer> framebuffer) noexcept;

private:
    static constexpr int IMAGE_READY_SEMAPHORE_COUNT = FVK_MAX_COMMAND_BUFFERS;

    void update();

    VulkanPlatform* mPlatform;
    VulkanContext const& mContext;
    fvkmemory::ResourceManager* mResourceManager;
    VulkanCommands* mCommands;
    VmaAllocator mAllocator;
    VulkanStagePool& mStagePool;
    bool const mHeadless;
    bool const mFlushAndWaitOnResize;
    bool const mTransitionSwapChainImageLayoutForPresent;

    // These fields store a callback to notify the client that Filament is commiting a frame.
    struct {
        CallbackHandler* handler = nullptr;
        std::shared_ptr<FrameScheduledCallback> callback;
    } mFrameScheduled;

    // We create VulkanTextures based on VkImages. VulkanTexture has facilities for doing layout
    // transitions, which are useful here.
    utils::FixedCapacityVector<fvkmemory::resource_ptr<VulkanTexture>> mColors;
    utils::FixedCapacityVector<fvkmemory::resource_ptr<VulkanSemaphore>> mFinishedDrawing;

    // One framebuffer per swapchain image, created lazily. These are owned by the swapchain rather
    // than VulkanFboCache so that they are never evicted. A single framebuffer is reused for every
    // render pass on its image, which is valid because those render passes are all compatible:
    // the swapchain fixes the attachments, and swapchain render passes have no subpasses.
    utils::FixedCapacityVector<fvkmemory::resource_ptr<VulkanFramebuffer>> mFramebuffers;
    fvkmemory::resource_ptr<VulkanTexture> mDepth;
    VkExtent2D mExtent;
    uint32_t mLayerCount;
    uint32_t mCurrentSwapIndex;
    bool mAcquired;
    bool mIsFirstRenderPass;
};


}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_VULKANSWAPCHAIN_H
