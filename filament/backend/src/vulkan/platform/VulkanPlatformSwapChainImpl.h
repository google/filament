/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANSWAPCHAIN_IMPL_H
#define TNT_FILAMENT_BACKEND_VULKANSWAPCHAIN_IMPL_H

#include "vulkan/VulkanContext.h"
#include "vulkan/VulkanConstants.h"

#include <backend/platforms/VulkanPlatform.h>

#include <bluevk/BlueVK.h>

#include <unordered_map>

using namespace bluevk;

namespace filament::backend {

static constexpr uint32_t const VULKAN_UNDEFINED_EXTENT = 0xFFFFFFFF;

struct VulkanPlatformSwapChainBase : public Platform::SwapChain {
    VulkanPlatformSwapChainBase(VulkanContext const& context, VkDevice device, VkQueue queue);

    inline VulkanPlatform::SwapChainBundle getSwapChainBundle() const {
        return mSwapChainBundle;
    }

    virtual ~VulkanPlatformSwapChainBase();

    virtual VkResult acquire(VulkanPlatform::ImageSyncData* outImageSyncData) = 0;

    virtual VkResult present(uint32_t index, VkSemaphore finished) = 0;

    virtual VkResult recreate() = 0;

    virtual bool hasResized() const = 0;

    virtual bool isProtected() const = 0;

protected:
    virtual void destroy() = 0;

    VkImage createImage(VkExtent2D extent, VkFormat format, bool isProtected);

    VulkanContext const& mContext;
    VkDevice mDevice;
    VkQueue mQueue;

    VulkanPlatform::SwapChainBundle mSwapChainBundle;
    std::unordered_map<VkImage, VkDeviceMemory> mMemory;
};

struct VulkanPlatformSurfaceSwapChain : public VulkanPlatformSwapChainBase {
    VulkanPlatformSurfaceSwapChain(VulkanContext const& context, VkPhysicalDevice physicalDevice,
            VkDevice device, VkQueue queue, VkInstance instance, VkSurfaceKHR surface,
            VkExtent2D fallbackExtent, uint64_t flags);

    ~VulkanPlatformSurfaceSwapChain() override;

    virtual VkResult acquire(VulkanPlatform::ImageSyncData* outImageSyncData) override;

    virtual VkResult present(uint32_t index, VkSemaphore finished) override;

    virtual VkResult recreate() override;

    virtual bool hasResized() const override;

    virtual bool isProtected() const override;

protected:
    virtual void destroy() override;

private:
    static constexpr int IMAGE_READY_SEMAPHORE_COUNT = FVK_MAX_COMMAND_BUFFERS;

    VkResult create();

    VkInstance mInstance;
    VkPhysicalDevice mPhysicalDevice;
    // This class takes ownership of the surface.
    VkSurfaceKHR mSurface;
    VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
    VkExtent2D const mFallbackExtent;
    VkSemaphore mImageReady[IMAGE_READY_SEMAPHORE_COUNT];
    uint32_t mCurrentImageReadyIndex;

    bool const mUsesRGB = false;
    bool const mHasStencil = false;
    bool const mIsProtected = false;
    bool mSuboptimal;
};

struct VulkanPlatformHeadlessSwapChain : public VulkanPlatformSwapChainBase {
    static constexpr size_t const HEADLESS_SWAPCHAIN_SIZE = 2;

    VulkanPlatformHeadlessSwapChain(VulkanContext const& context, VkDevice device, VkQueue queue,
            VkExtent2D extent, uint64_t flags);

    ~VulkanPlatformHeadlessSwapChain() override;

    virtual VkResult acquire(VulkanPlatform::ImageSyncData* outImageSyncData) override;

    virtual VkResult present(uint32_t index, VkSemaphore finished) override;

    virtual VkResult recreate() override {
        PANIC_PRECONDITION("Should not be called");
        return VK_ERROR_UNKNOWN;
    }

    virtual bool hasResized() const override { return false; }

    virtual bool isProtected() const override { return false; }

protected:
    virtual void destroy() override;

private:
    uint32_t mCurrentIndex;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_VULKANSWAPCHAIN_IMPL_H
