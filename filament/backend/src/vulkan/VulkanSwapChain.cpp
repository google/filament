/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "VulkanSwapChain.h"

#include "VulkanCommands.h"
#include "VulkanTexture.h"

#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>

using namespace bluevk;
using namespace utils;

namespace filament::backend {

VulkanSwapChain::VulkanSwapChain(VulkanPlatform* platform, VulkanContext const& context,
        fvkmemory::ResourceManager* resourceManager, VmaAllocator allocator,
        VulkanCommands* commands, VulkanStagePool& stagePool, void* nativeWindow, uint64_t flags,
        VkExtent2D extent)
    : mPlatform(platform),
      mResourceManager(resourceManager),
      mCommands(commands),
      mAllocator(allocator),
      mStagePool(stagePool),
      mHeadless(extent.width != 0 && extent.height != 0 && !nativeWindow),
      mFlushAndWaitOnResize(platform->getCustomization().flushAndWaitOnWindowResize),
      mTransitionSwapChainImageLayoutForPresent(
              platform->getCustomization().transitionSwapChainImageLayoutForPresent),
      mAcquired(false),
      mIsFirstRenderPass(true) {
    swapChain = mPlatform->createSwapChain(nativeWindow, flags, extent);
    FILAMENT_CHECK_POSTCONDITION(swapChain) << "Unable to create swapchain";

    update();
}

VulkanSwapChain::~VulkanSwapChain() {
    // Must wait for the inflight command buffers to finish since they might contain the images
    // we're about to destroy.
    mCommands->flush();
    mCommands->wait();

    mColors = {};
    mDepth = {};

    mPlatform->destroy(swapChain);
}

void VulkanSwapChain::update() {
    mColors.clear();

    auto const bundle = mPlatform->getSwapChainBundle(swapChain);
    mColors.reserve(bundle.colors.size());
    VkDevice const device = mPlatform->getDevice();

    TextureUsage depthUsage = TextureUsage::DEPTH_ATTACHMENT;
    TextureUsage colorUsage = TextureUsage::COLOR_ATTACHMENT;
    if (bundle.isProtected) {
        depthUsage |= TextureUsage::PROTECTED;
        colorUsage |= TextureUsage::PROTECTED;
    }
    for (auto const color: bundle.colors) {
        auto colorTexture = fvkmemory::resource_ptr<VulkanTexture>::construct(mResourceManager,
                device, mAllocator, mResourceManager, mCommands, color, VK_NULL_HANDLE,
                bundle.colorFormat, 1, bundle.extent.width, bundle.extent.height, colorUsage,
                mStagePool);
        mColors.push_back(colorTexture);
    }

    mDepth = fvkmemory::resource_ptr<VulkanTexture>::construct(mResourceManager, device,
        mAllocator, mResourceManager, mCommands, bundle.depth, VK_NULL_HANDLE,
        bundle.depthFormat, 1, bundle.extent.width, bundle.extent.height, depthUsage,
        mStagePool);

    mExtent = bundle.extent;
}

void VulkanSwapChain::present() {
    if (!mHeadless && mTransitionSwapChainImageLayoutForPresent) {
        VulkanCommandBuffer& commands = mCommands->get();
        VkImageSubresourceRange const subresources{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
        };
        mColors[mCurrentSwapIndex]->transitionLayout(&commands, subresources, VulkanLayout::PRESENT);
    }

    mCommands->flush();

    // call the image ready wait function
    if (mExplicitImageReadyWait != nullptr) {
        mExplicitImageReadyWait(swapChain);
    }

    // We only present if it is not headless. No-op for headless.
    if (!mHeadless) {
        VkSemaphore const finishedDrawing = mCommands->acquireFinishedSignal();
        VkResult const result = mPlatform->present(swapChain, mCurrentSwapIndex, finishedDrawing);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ||
                result == VK_ERROR_OUT_OF_DATE_KHR)
                << "Cannot present in swapchain. error=" << static_cast<int32_t>(result);
    }

    // We presented the last acquired buffer.
    mAcquired = false;
    mIsFirstRenderPass = true;
}

void VulkanSwapChain::acquire(bool& resized) {
    // It's ok to call acquire multiple times due to it being linked to Driver::makeCurrent().
    if (mAcquired) {
        return;
    }

    // Check if the swapchain should be resized.
    if ((resized = mPlatform->hasResized(swapChain))) {
        if (mFlushAndWaitOnResize) {
            mCommands->flush();
            mCommands->wait();
        }
        mPlatform->recreate(swapChain);
        update();
    }

    VulkanPlatform::ImageSyncData imageSyncData;
    VkResult const result = mPlatform->acquire(swapChain, &imageSyncData);
    mCurrentSwapIndex = imageSyncData.imageIndex;
    mExplicitImageReadyWait = imageSyncData.explicitImageReadyWait;
    FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
            << "Cannot acquire in swapchain. error=" << static_cast<int32_t>(result);
    if (imageSyncData.imageReadySemaphore != VK_NULL_HANDLE) {
        mCommands->injectDependency(imageSyncData.imageReadySemaphore);
    }
    mAcquired = true;
}

}// namespace filament::backend
