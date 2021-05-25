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

#include "VulkanSwapChain.h"

#include <utils/Panic.h>

using namespace bluevk;

namespace filament {
namespace backend {

static VkSurfaceCapabilitiesKHR getSurfaceCaps(VulkanContext& context, VulkanSwapChain& sc) {
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.physicalDevice,
            sc.surface, &sc.surfaceCapabilities);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR error.");
    ASSERT_POSTCONDITION(
            sc.surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            "Vulkan surface doesn't support VK_IMAGE_USAGE_TRANSFER_DST_BIT.");
    uint32_t surfaceFormatsCount;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(context.physicalDevice, sc.surface,
            &surfaceFormatsCount, nullptr);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceFormatsKHR count error.");
    sc.surfaceFormats.resize(surfaceFormatsCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(context.physicalDevice, sc.surface,
            &surfaceFormatsCount, sc.surfaceFormats.data());
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceFormatsKHR error.");
    return sc.surfaceCapabilities;
}

bool VulkanSwapChain::acquire() {
    if (headlessQueue) {
        currentSwapIndex = (currentSwapIndex + 1) % attachments.size();
        return true;

    }

    // This immediately retrieves the index of the next available presentable image, and
    // asynchronously requests the GPU to trigger the "imageAvailable" semaphore.
    VkResult result = vkAcquireNextImageKHR(context.device, swapchain,
            UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &currentSwapIndex);

    // Users should be notified of a suboptimal surface, but it should not cause a cascade of
    // log messages or a loop of re-creations.
    if (result == VK_SUBOPTIMAL_KHR && !suboptimal) {
        utils::slog.w << "Vulkan Driver: Suboptimal swap chain." << utils::io::endl;
        suboptimal = true;
    }

    // The surface can be "out of date" when it has been resized, which is not an error.
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return false;
    }

    // To ensure that the next command buffer submission does not write into the image before
    // it has been acquired, push the image available semaphore into the command buffer manager.
    context.commands->injectDependency(imageAvailable);
    acquired = true;

    assert_invariant(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
    return true;
}

static void createFinalDepthBuffer(VulkanContext& context, VulkanSwapChain& surfaceContext,
        VkFormat depthFormat) {
    // Create an appropriately-sized device-only VkImage.
    const auto size = surfaceContext.surfaceCapabilities.currentExtent;
    VkImage depthImage;
    VkImageCreateInfo imageInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depthFormat,
        .extent = { size.width, size.height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };
    VkResult error = bluevk::vkCreateImage(context.device, &imageInfo, VKALLOC, &depthImage);
    assert_invariant(!error && "Unable to create depth image.");

    // Allocate memory for the VkImage and bind it.
    VkMemoryRequirements memReqs;
    bluevk::vkGetImageMemoryRequirements(context.device, depthImage, &memReqs);
    VkMemoryAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = selectMemoryType(context, memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };
    error = bluevk::vkAllocateMemory(context.device, &allocInfo, nullptr,
            &surfaceContext.depth.memory);
    assert_invariant(!error && "Unable to allocate depth memory.");
    error = bluevk::vkBindImageMemory(context.device, depthImage, surfaceContext.depth.memory, 0);
    assert_invariant(!error && "Unable to bind depth memory.");

    // Create a VkImageView so that we can attach depth to the framebuffer.
    VkImageView depthView;
    VkImageViewCreateInfo viewInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = depthFormat,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = 1,
            .layerCount = 1,
        },
    };
    error = bluevk::vkCreateImageView(context.device, &viewInfo, VKALLOC, &depthView);
    assert_invariant(!error && "Unable to create depth view.");

    // Unlike the color attachments (which are double-buffered or triple-buffered), we only need one
    // depth attachment in the entire chain.
    surfaceContext.depth.view = depthView;
    surfaceContext.depth.image = depthImage;
    surfaceContext.depth.format = depthFormat;
    surfaceContext.depth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Begin a new command buffer solely for the purpose of transitioning the image layout.
    VkCommandBuffer cmdbuffer = context.commands->get().cmdbuffer;

    // Transition the depth image into an optimal layout.
    VkImageMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .newLayout = surfaceContext.depth.layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = depthImage,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = 1,
            .layerCount = 1,
        },
    };
    vkCmdPipelineBarrier(cmdbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanSwapChain::create() {
    const auto caps = getSurfaceCaps(context, *this);

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
        utils::slog.e << "Swap chain does not support " << desiredImageCount << " images.\n";
        desiredImageCount = surfaceCapabilities.minImageCount;
    }
    surfaceFormat = surfaceFormats[0];
    for (const VkSurfaceFormatKHR& format : surfaceFormats) {
        if (format.format == VK_FORMAT_R8G8B8A8_UNORM) {
            surfaceFormat = format;
        }
    }
    const auto compositionCaps = caps.supportedCompositeAlpha;
    const auto compositeAlpha = (compositionCaps & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ?
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // Create the low-level swap chain.
    auto size = clientSize = caps.currentExtent;

    VkSwapchainCreateInfoKHR createInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = desiredImageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = size,
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
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,

        // TODO: Setting the oldSwapchain parameter would avoid exiting and re-entering
        // exclusive mode, which could result in a smoother orientation change.
        .oldSwapchain = VK_NULL_HANDLE
    };
    VkResult result = vkCreateSwapchainKHR(context.device, &createInfo, VKALLOC, &swapchain);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceFormatsKHR error.");

    // Extract the VkImage handles from the swap chain.
    uint32_t imageCount;
    result = vkGetSwapchainImagesKHR(context.device, swapchain, &imageCount, nullptr);
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetSwapchainImagesKHR count error.");
    attachments.resize(imageCount);
    std::vector<VkImage> images(imageCount);
    result = vkGetSwapchainImagesKHR(context.device, swapchain, &imageCount,
            images.data());
    ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkGetSwapchainImagesKHR error.");
    for (size_t i = 0; i < images.size(); ++i) {
        attachments[i] = {
            .format = surfaceFormat.format,
            .image = images[i],
            .view = {},
            .memory = {},
            .texture = {},
            .layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };
    }
    utils::slog.i
            << "vkCreateSwapchain"
            << ": " << size.width << "x" << size.height
            << ", " << surfaceFormat.format
            << ", " << surfaceFormat.colorSpace
            << ", " << imageCount
            << ", " << caps.currentTransform
            << utils::io::endl;

    // Create image views.
    VkImageViewCreateInfo ivCreateInfo = {};
    ivCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ivCreateInfo.format = surfaceFormat.format;
    ivCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ivCreateInfo.subresourceRange.levelCount = 1;
    ivCreateInfo.subresourceRange.layerCount = 1;
    for (size_t i = 0; i < images.size(); ++i) {
        ivCreateInfo.image = images[i];
        result = bluevk::vkCreateImageView(context.device, &ivCreateInfo, VKALLOC,
                &attachments[i].view);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "vkCreateImageView error.");
    }

    createSemaphore(context.device, &imageAvailable);
    acquired = false;

    createFinalDepthBuffer(context, *this, context.finalDepthFormat);
}

void VulkanSwapChain::destroy() {
    waitForIdle(context);
    const VkDevice device = context.device;
    for (VulkanAttachment& swapContext : attachments) {

        // If this is headless, then we own the image and need to explicitly destroy it.
        if (!swapchain) {
            vkDestroyImage(device, swapContext.image, VKALLOC);
            vkFreeMemory(device, swapContext.memory, VKALLOC);
        }

        vkDestroyImageView(device, swapContext.view, VKALLOC);
        swapContext.view = VK_NULL_HANDLE;
    }
    vkDestroySwapchainKHR(device, swapchain, VKALLOC);
    vkDestroySemaphore(device, imageAvailable, VKALLOC);

    vkDestroyImageView(device, depth.view, VKALLOC);
    vkDestroyImage(device, depth.image, VKALLOC);
    vkFreeMemory(device, depth.memory, VKALLOC);
}

// Near the end of the frame, we transition the swap chain to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR using
// an image barrier rather than a render pass because each render pass does not know whether or not
// it is the last pass in the frame. (This seems to be an atypical way of achieving the transition,
// but I see nothing wrong with it.)
//
// Note however that we *do* use a render pass to transition the swap chain back to
// VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL on the subsequent frame that writes to it.
void VulkanSwapChain::makePresentable() {
    if (headlessQueue) {
        return;
    }
    VulkanAttachment& swapContext = attachments[currentSwapIndex];
    VkImageMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = 0,

        // Using COLOR_ATTACHMENT_OPTIMAL for oldLayout seems to be required for NVIDIA drivers
        // (see https://github.com/google/filament/pull/3190), but the Android NDK validation layers
        // make the following complaint:
        //
        //   You cannot transition the layout [...] from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        //   when the previous known layout is VK_IMAGE_LAYOUT_PRESENT_SRC_KHR. The Vulkan spec
        //   states: oldLayout must be VK_IMAGE_LAYOUT_UNDEFINED or the current layout of the image
        //   subresources affected by the barrier
        //
        //   (https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VUID-VkImageMemoryBarrier-oldLayout-01197)
#ifdef ANDROID
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
#else
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
#endif

        .newLayout = swapContext.layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapContext.image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1,
        },
    };
    vkCmdPipelineBarrier(context.commands->get().cmdbuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

static void getHeadlessQueue(VulkanContext& context, VulkanSwapChain& sc) {
    vkGetDeviceQueue(context.device, context.graphicsQueueFamilyIndex, 0, &sc.headlessQueue);
    ASSERT_POSTCONDITION(sc.headlessQueue, "Unable to obtain graphics queue.");
    sc.presentQueue = VK_NULL_HANDLE;
}

static void getPresentationQueue(VulkanContext& context, VulkanSwapChain& sc) {
    uint32_t queueFamiliesCount;
    vkGetPhysicalDeviceQueueFamilyProperties(context.physicalDevice, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamiliesProperties(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(context.physicalDevice, &queueFamiliesCount,
            queueFamiliesProperties.data());
    uint32_t presentQueueFamilyIndex = 0xffff;

    // We prefer the graphics and presentation queues to be the same, so first check if that works.
    // On most platforms they must be the same anyway, and this avoids issues with MoltenVK.
    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(context.physicalDevice, context.graphicsQueueFamilyIndex,
            sc.surface, &supported);
    if (supported) {
        presentQueueFamilyIndex = context.graphicsQueueFamilyIndex;
    }

    // Otherwise fall back to separate graphics and presentation queues.
    if (presentQueueFamilyIndex == 0xffff) {
        for (uint32_t j = 0; j < queueFamiliesCount; ++j) {
            vkGetPhysicalDeviceSurfaceSupportKHR(context.physicalDevice, j, sc.surface, &supported);
            if (supported) {
                presentQueueFamilyIndex = j;
                break;
            }
        }
    }
    ASSERT_POSTCONDITION(presentQueueFamilyIndex != 0xffff,
            "This physical device does not support the presentation queue.");
    if (context.graphicsQueueFamilyIndex != presentQueueFamilyIndex) {

        // TODO: Strictly speaking, this code path is incorrect. However it is not triggered on any
        // Android devices that we've tested with, nor with MoltenVK.
        //
        // This is incorrect because we created the logical device early on, before we had a handle
        // to the rendering surface. Therefore the device was not created with the presentation
        // queue family index included in VkDeviceQueueCreateInfo.
        //
        // This is non-trivial to fix because the driver API allows clients to do certain things
        // (e.g. upload a vertex buffer) before the swap chain is created.
        //
        // https://github.com/google/filament/issues/1532
        vkGetDeviceQueue(context.device, presentQueueFamilyIndex, 0, &sc.presentQueue);

    } else {
        sc.presentQueue = context.graphicsQueue;
    }
    ASSERT_POSTCONDITION(sc.presentQueue, "Unable to obtain presentation queue.");
    sc.headlessQueue = VK_NULL_HANDLE;
}

// Primary SwapChain constructor. (not headless)
VulkanSwapChain::VulkanSwapChain(VulkanContext& context, VkSurfaceKHR vksurface) :
        context(context) {
    suboptimal = false;
    surface = vksurface;
    firstRenderPass = true;
    getPresentationQueue(context, *this);
    create();
}

// Headless SwapChain constructor. (does not create a VkSwapChainKHR)
VulkanSwapChain::VulkanSwapChain(VulkanContext& context, uint32_t width, uint32_t height) :
        context(context) {
    surface = VK_NULL_HANDLE;
    getHeadlessQueue(context, *this);

    surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    swapchain = VK_NULL_HANDLE;

    // Somewhat arbitrarily, headless rendering is double-buffered.
    attachments.resize(2);

    for (size_t i = 0; i < attachments.size(); ++i) {
        VkImage image;
        VkImageCreateInfo iCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = surfaceFormat.format,
            .extent = {
                .width = width,
                .height = height,
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        };
        assert_invariant(iCreateInfo.extent.width > 0);
        assert_invariant(iCreateInfo.extent.height > 0);
        vkCreateImage(context.device, &iCreateInfo, VKALLOC, &image);

        VkMemoryRequirements memReqs = {};
        vkGetImageMemoryRequirements(context.device, image, &memReqs);
        VkMemoryAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memReqs.size,
            .memoryTypeIndex = selectMemoryType(context, memReqs.memoryTypeBits,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        };
        VkDeviceMemory imageMemory;
        vkAllocateMemory(context.device, &allocInfo, VKALLOC, &imageMemory);
        vkBindImageMemory(context.device, image, imageMemory, 0);

        attachments[i] = {
            .format = surfaceFormat.format, .image = image,
            .view = {}, .memory = imageMemory, .texture = {}, .layout = VK_IMAGE_LAYOUT_GENERAL
        };
        VkImageViewCreateInfo ivCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surfaceFormat.format,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1,
            }
        };
        vkCreateImageView(context.device, &ivCreateInfo, VKALLOC,
                    &attachments[i].view);

        VkImageMemoryBarrier barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1,
            },
        };
        vkCmdPipelineBarrier(context.commands->get().cmdbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1,
                &barrier);
    }

    surfaceCapabilities.currentExtent.width = width;
    surfaceCapabilities.currentExtent.height = height;

    clientSize.width = width;
    clientSize.height = height;

    imageAvailable = VK_NULL_HANDLE;

    createFinalDepthBuffer(context, *this, context.finalDepthFormat);
}

} // namespace filament
} // namespace backend
