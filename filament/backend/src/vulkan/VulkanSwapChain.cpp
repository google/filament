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
#include "VulkanTexture.h"

#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>

using namespace bluevk;
using namespace utils;

namespace filament::backend {

VulkanSwapChain::VulkanSwapChain(VulkanPlatform* platform, VulkanContext const& context,
        VmaAllocator allocator, VulkanCommands* commands, VulkanStagePool& stagePool,
        void* nativeWindow, uint64_t flags, VkExtent2D extent)
    : mPlatform(platform),
      mCommands(commands),
      mAllocator(allocator),
      mStagePool(stagePool),
      mHeadless(extent.width != 0 && extent.height != 0 && !nativeWindow),
      mAcquired(false),
      mIsFirstRenderPass(true) {
    swapChain = mPlatform->createSwapChain(nativeWindow, flags, extent);
    ASSERT_POSTCONDITION(swapChain, "Unable to create swapchain");

    VkSemaphoreCreateInfo const createInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VkResult result = vkCreateSemaphore(mPlatform->getDevice(), &createInfo, nullptr, &mImageReady);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "Failed to create semaphore");

    update();
}

VulkanSwapChain::~VulkanSwapChain() {
    // Must wait for the inflight command buffers to finish since they might contain the images
    // we're about to destroy.
    mCommands->flush();
    mCommands->wait();

    mPlatform->destroy(swapChain);
    vkDestroySemaphore(mPlatform->getDevice(), mImageReady, VKALLOC);
}

void VulkanSwapChain::update() {
    mColors.clear();

    auto const bundle = mPlatform->getSwapChainBundle(swapChain);
    mColors.reserve(bundle.colors.size());
    VkDevice const device = mPlatform->getDevice();

    for (auto const color: bundle.colors) {
        mColors.push_back(std::make_unique<VulkanTexture>(device, mAllocator, mCommands, color,
                bundle.colorFormat, 1, bundle.extent.width, bundle.extent.height,
                TextureUsage::COLOR_ATTACHMENT, mStagePool));
    }
    mDepth = std::make_unique<VulkanTexture>(device, mAllocator, mCommands, bundle.depth,
            bundle.depthFormat, 1, bundle.extent.width, bundle.extent.height,
            TextureUsage::DEPTH_ATTACHMENT, mStagePool);

    mExtent = bundle.extent;
}

void VulkanSwapChain::present() {
    if (!mHeadless) {
        VkCommandBuffer const cmdbuf = mCommands->get().cmdbuffer;
        VkImageSubresourceRange const subresources{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
        };
        mColors[mCurrentSwapIndex]->transitionLayout(cmdbuf, subresources, VulkanLayout::PRESENT);
        mCommands->flush();
    }
    VkSemaphore const finishedDrawing = mCommands->acquireFinishedSignal();
    VkResult const result = mPlatform->present(swapChain, mCurrentSwapIndex, finishedDrawing);
    ASSERT_POSTCONDITION(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR
                                 || result == VK_ERROR_OUT_OF_DATE_KHR,
            "Cannot present in swapchain.");

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
        mCommands->flush();
        mCommands->wait();
        mPlatform->recreate(swapChain);
        update();
    }

    VkResult const result = mPlatform->acquire(swapChain, mImageReady, &mCurrentSwapIndex);
    ASSERT_POSTCONDITION(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR,
            "Cannot acquire in swapchain.");
    mCommands->injectDependency(mImageReady);
    mAcquired = true;
}

}// namespace filament::backend
