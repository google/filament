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

namespace {
constexpr uint32_t VULKAN_UNDEFINED_EXTENT = 0xFFFFFFFF;
} // anonymous namespace

namespace filament::backend {

bool VulkanSwapChain::acquire() {
    if (headlessQueue) {
        mCurrentSwapIndex = (mCurrentSwapIndex + 1) % mColor.size();

        UTILS_UNUSED_IN_RELEASE VulkanLayout const layout = getColorTexture().getLayout(0, 0);

        // Next we perform a quick sanity check on layout for headless swap chains. It's easier to
        // catch errors here than with validation. If this is the first time a particular image has
        // been acquired, it should be in an UNDEFINED state. If this is not the first time, then it
        // should be in the normal layout that we use for color attachments.
        assert_invariant(
                layout == VulkanLayout::UNDEFINED || layout == VulkanLayout::COLOR_ATTACHMENT);
        return true;
    }

    // This immediately retrieves the index of the next available presentable image, and
    // asynchronously requests the GPU to trigger the "imageAvailable" semaphore.
    VkResult result = vkAcquireNextImageKHR(mDevice, swapchain,
            UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &mCurrentSwapIndex);

    // Users should be notified of a suboptimal surface, but it should not cause a cascade of
    // log messages or a loop of re-creations.
    if (result == VK_SUBOPTIMAL_KHR && !suboptimal) {
        slog.w << "Vulkan Driver: Suboptimal swap chain." << io::endl;
        suboptimal = true;
    }

    UTILS_UNUSED_IN_RELEASE VulkanLayout const layout = getColorTexture().getLayout(0, 0);

    // Next perform a quick sanity check on the image layout. Similar to attachable textures, we
    // immediately transition the swap chain image layout during the first render pass of the frame.
    assert_invariant(layout == VulkanLayout::UNDEFINED || layout == VulkanLayout::PRESENT);

    // To ensure that the next command buffer submission does not write into the image before
    // it has been acquired, push the image available semaphore into the command buffer manager.
    mCommands->injectDependency(imageAvailable);
    acquired = true;

    assert_invariant(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
    return true;
}

void VulkanSwapChain::recreate() {
    destroy();
    create();
}

void VulkanSwapChain::create() {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, surface, &caps);

    // The general advice is to require one more than the minimum swap chain length, since the
    // absolute minimum could easily require waiting for a driver or presentation layer to release
    // the previous frame's buffer. The only situation in which we'd ask for the minimum length is
    // when using a MAILBOX presentation strategy for low-latency situations where tearing is
    // acceptable.
    const uint32_t maxImageCount = caps.maxImageCount;
    const uint32_t minImageCount = caps.minImageCount;
    uint32_t desiredImageCount = minImageCount + 1;

    // According to section 30.5 of VK 1.1, maxImageCount of zero means "that there is no limit on
    // the number of images, though there may be limits related to the total amount of memory used
    // by presentable images."
    if (maxImageCount != 0 && desiredImageCount > maxImageCount) {
        slog.e << "Swap chain does not support " << desiredImageCount << " images." << io::endl;
        desiredImageCount = caps.minImageCount;
    }

    // Find a suitable surface format.
    if (surfaceFormat.format == VK_FORMAT_UNDEFINED) {

        uint32_t surfaceFormatsCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, surface,
                &surfaceFormatsCount, nullptr);

        FixedCapacityVector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatsCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, surface,
                &surfaceFormatsCount, surfaceFormats.data());

        surfaceFormat = surfaceFormats[0];
        for (const VkSurfaceFormatKHR& format : surfaceFormats) {
            if (format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) {
                surfaceFormat = format;
                break;
            }
        }
    }

    // Verify that our chosen present mode is supported. In practice all devices support the FIFO mode, but we
    // check for it anyway for completeness.  (and to avoid validation warnings)

    const VkPresentModeKHR desiredPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, surface, &presentModeCount, nullptr);
    FixedCapacityVector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, surface, &presentModeCount, presentModes.data());
    bool foundSuitablePresentMode = false;
    for (VkPresentModeKHR mode : presentModes) {
        if (mode == desiredPresentMode) {
            foundSuitablePresentMode = true;
            break;
        }
    }
    ASSERT_POSTCONDITION(foundSuitablePresentMode, "Desired present mode is not supported by this device.");

    // Create the low-level swap chain.
    if (caps.currentExtent.width == VULKAN_UNDEFINED_EXTENT ||
        caps.currentExtent.height == VULKAN_UNDEFINED_EXTENT) {
        clientSize = mFallbackExtent;
    } else {
        clientSize = caps.currentExtent;
    }

    const VkCompositeAlphaFlagBitsKHR compositeAlpha = (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    VkSwapchainCreateInfoKHR createInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = desiredImageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = clientSize,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | // Allows use as a blit destination.
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT,  // Allows use as a blit source (for readPixels)

        // TODO: Setting the preTransform to IDENTITY means we are letting the Android Compositor
        // handle the rotation. In some situations it might be more efficient to handle this
        // ourselves by setting this field to be equal to the currentTransform mask in the caps, but
        // this would involve adjusting the MVP, derivatives in GLSL, and possibly more.
        // https://android-developers.googleblog.com/2020/02/handling-device-orientation-efficiently.html
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,

        .compositeAlpha = compositeAlpha,
        .presentMode = desiredPresentMode,
        .clipped = VK_TRUE,

        // TODO: Setting the oldSwapchain parameter would avoid exiting and re-entering
        // exclusive mode, which could result in a smoother orientation change.
        .oldSwapchain = VK_NULL_HANDLE
    };
    VkResult result = vkCreateSwapchainKHR(mDevice, &createInfo, VKALLOC, &swapchain);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceFormatsKHR error.");

    // Extract the VkImage handles from the swap chain.
    uint32_t imageCount;
    result = vkGetSwapchainImagesKHR(mDevice, swapchain, &imageCount, nullptr);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetSwapchainImagesKHR count error.");
    mColor.reserve(imageCount);
    mColor.resize(imageCount);
    FixedCapacityVector<VkImage> images(imageCount);
    result = vkGetSwapchainImagesKHR(mDevice, swapchain, &imageCount, images.data());
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetSwapchainImagesKHR error.");
    for (size_t i = 0; i < images.size(); ++i) {
        mColor[i].reset(new VulkanTexture(mDevice, mAllocator, mCommands, images[i],
                surfaceFormat.format, 1, clientSize.width, clientSize.height,
                TextureUsage::COLOR_ATTACHMENT, mStagePool));
    }
    slog.i << "vkCreateSwapchain"
           << ": " << clientSize.width << "x" << clientSize.height << ", " << surfaceFormat.format
           << ", " << surfaceFormat.colorSpace << ", " << imageCount << ", "
           << caps.currentTransform << io::endl
           << io::endl;

    createSemaphore(mDevice, &imageAvailable);
    acquired = false;

    // HACK: force usage of the "fallback" depth format, since we know that's a safe depth format.
    auto depthFormat = TextureFormat::DEPTH24;

    mDepth.reset(new VulkanTexture(mDevice, mPhysicalDevice, mContext, mAllocator, mCommands,
            SamplerType::SAMPLER_2D, 1, depthFormat, 1, clientSize.width, clientSize.height, 1,
            TextureUsage::DEPTH_ATTACHMENT, mStagePool));
}

void VulkanSwapChain::destroy() {
    mCommands->flush();
    mCommands->wait();
    mDepth.reset();
    for (auto& texture : mColor) {
        texture.reset();
    }
    vkDestroySwapchainKHR(mDevice, swapchain, VKALLOC);
    vkDestroySemaphore(mDevice, imageAvailable, VKALLOC);
}

// Near the end of the frame, we transition the swap chain to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR using
// an image barrier rather than a render pass because each render pass does not know whether or not
// it is the last pass in the frame. (This seems to be an atypical way of achieving the transition,
// but I see nothing wrong with it.)
void VulkanSwapChain::makePresentable() {
    if (headlessQueue) {
        return;
    }
    VulkanTexture& texture = getColorTexture();

    VkCommandBuffer const cmdbuffer = mCommands->get().cmdbuffer;
    VkImageSubresourceRange const subresources{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
    texture.transitionLayout(cmdbuffer, subresources, VulkanLayout::PRESENT);
}

static void getHeadlessQueue(VkDevice device, uint32_t graphicsQueueFamilyIndex,
			     VulkanSwapChain& sc) {
    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &sc.headlessQueue);
    ASSERT_POSTCONDITION(sc.headlessQueue, "Unable to obtain graphics queue.");
    sc.presentQueue = VK_NULL_HANDLE;
}

static void getPresentationQueue(VkPhysicalDevice physicalDevice, uint32_t graphicsQueueFamilyIndex,
				 VkQueue graphicsQueue, VulkanSwapChain& sc) {
    uint32_t queueFamiliesCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);
    FixedCapacityVector<VkQueueFamilyProperties> queueFamiliesProperties(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount,
            queueFamiliesProperties.data());

    // We prefer the graphics and presentation queues to be the same, so first check if that works.
    // On most platforms they must be the same anyway, and this avoids issues with MoltenVK.
    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, graphicsQueueFamilyIndex,
            sc.surface, &supported);

    // We assume that the chosen graphics queues are able to present. See also
    // https://github.com/google/filament/issues/1532
    ASSERT_POSTCONDITION(supported, "Graphics queues do not support presentation.");

    sc.presentQueue = graphicsQueue;
    sc.headlessQueue = VK_NULL_HANDLE;
}

// Primary SwapChain constructor. (not headless)
VulkanSwapChain::VulkanSwapChain(VkDevice device, VkPhysicalDevice physicalDevice,
        uint32_t graphicsQueueFamilyIndex, VkQueue graphicsQueue, VmaAllocator allocator,
        std::shared_ptr<VulkanCommands> commands, VulkanContext const& context,
        VulkanStagePool& stagePool, VkSurfaceKHR vksurface, VkExtent2D fallbackExtent)
    : mDevice(device), mPhysicalDevice(physicalDevice), mCommands(commands),
      mAllocator(allocator), mContext(context),
      mStagePool(stagePool), mFallbackExtent(fallbackExtent) {
    suboptimal = false;
    surface = vksurface;
    firstRenderPass = true;
    getPresentationQueue(physicalDevice, graphicsQueueFamilyIndex, graphicsQueue, *this);
    create();
}

// Headless SwapChain constructor. (does not create a VkSwapChainKHR)
VulkanSwapChain::VulkanSwapChain(VkDevice device, VkPhysicalDevice physicalDevice,
        uint32_t graphicsQueueFamilyIndex, VkQueue graphicsQueue, VmaAllocator allocator,
        std::shared_ptr<VulkanCommands> commands, VulkanContext const& context,
        VulkanStagePool& stagePool, uint32_t width, uint32_t height)
    : mDevice(device), mPhysicalDevice(physicalDevice), mCommands(commands),
      mAllocator(allocator), mContext(context),
      mStagePool(stagePool) {
    surface = VK_NULL_HANDLE;
    getHeadlessQueue(device, graphicsQueueFamilyIndex, *this);

    surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    swapchain = VK_NULL_HANDLE;

    // Somewhat arbitrarily, headless rendering is double-buffered.
    mColor.reserve(2);
    mColor.resize(2);

    for (size_t i = 0; i < mColor.size(); ++i) {
        mColor[i].reset(new VulkanTexture(device, physicalDevice, context, mAllocator, commands,
                SamplerType::SAMPLER_2D, 1, TextureFormat::RGBA8, 1, width, height, 1,
                TextureUsage::COLOR_ATTACHMENT, stagePool));
    }

    clientSize.width = width;
    clientSize.height = height;

    imageAvailable = VK_NULL_HANDLE;

    // HACK: force usage of the "fallback" depth format, since we know that's a safe depth format.
    auto depthFormat = TextureFormat::DEPTH24;

    mDepth.reset(new VulkanTexture(device, physicalDevice, context, mAllocator, commands,
            SamplerType::SAMPLER_2D, 1, depthFormat, 1, clientSize.width, clientSize.height, 1,
            TextureUsage::DEPTH_ATTACHMENT, stagePool));
}

bool VulkanSwapChain::hasResized() const {
    if (surface != VK_NULL_HANDLE) {
	return false;
    }
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, surface, &caps);
    VkExtent2D perceivedExtent = caps.currentExtent;
    // Create the low-level swap chain.
    if (caps.currentExtent.width == VULKAN_UNDEFINED_EXTENT ||
        caps.currentExtent.height == VULKAN_UNDEFINED_EXTENT) {
        perceivedExtent = mFallbackExtent;
    }
    return !equivalent(clientSize, perceivedExtent);
}

VulkanTexture& VulkanSwapChain::getColorTexture() {
    return *mColor[mCurrentSwapIndex];
}

VulkanTexture& VulkanSwapChain::getDepthTexture() {
    return *mDepth;
}

} // namespace filament::backend
