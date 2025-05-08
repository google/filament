// Copyright 2018 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/vulkan/SwapChainVk.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "dawn/common/Compiler.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/Surface.h"
#include "dawn/native/vulkan/BackendVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/QueueVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "vulkan/vulkan_core.h"

#if defined(DAWN_USE_X11)
#include "dawn/native/X11Functions.h"
#endif  // defined(DAWN_USE_X11)

namespace dawn::native::vulkan {

namespace {

VkPresentModeKHR ToVulkanPresentMode(wgpu::PresentMode mode) {
    switch (mode) {
        case wgpu::PresentMode::Fifo:
            return VK_PRESENT_MODE_FIFO_KHR;
        case wgpu::PresentMode::FifoRelaxed:
            return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        case wgpu::PresentMode::Immediate:
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case wgpu::PresentMode::Mailbox:
            return VK_PRESENT_MODE_MAILBOX_KHR;
        case wgpu::PresentMode::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

uint32_t MinImageCountForPresentMode(VkPresentModeKHR mode) {
    switch (mode) {
        case VK_PRESENT_MODE_FIFO_KHR:
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
            return 2;
        case VK_PRESENT_MODE_MAILBOX_KHR:
            return 3;
        default:
            break;
    }
    DAWN_UNREACHABLE();
}

ResultOrError<UniqueVkHandle<VkSemaphore>> CreateSemaphore(Device* device) {
    VkSemaphoreCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    VkSemaphore semaphore;
    DAWN_TRY(CheckVkSuccess(
        device->fn.CreateSemaphore(device->GetVkDevice(), &createInfo, nullptr, &*semaphore),
        "CreateSemaphore"));
    return {{device, semaphore}};
}

ResultOrError<UniqueVkHandle<VkFence>> CreateFence(Device* device) {
    VkFenceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    VkFence fence;
    DAWN_TRY(
        CheckVkSuccess(device->fn.CreateFence(device->GetVkDevice(), &createInfo, nullptr, &*fence),
                       "CreateFence"));
    return {{device, fence}};
}

}  // anonymous namespace

// static
ResultOrError<Ref<SwapChain>> SwapChain::Create(Device* device,
                                                Surface* surface,
                                                SwapChainBase* previousSwapChain,
                                                const SurfaceConfiguration* config) {
    Ref<SwapChain> swapchain = AcquireRef(new SwapChain(device, surface, config));
    DAWN_TRY(swapchain->Initialize(previousSwapChain));
    return swapchain;
}

SwapChain::~SwapChain() = default;

// Note that when we need to re-create the swapchain because it is out of date,
// previousSwapChain can be set to `this`.
MaybeError SwapChain::Initialize(SwapChainBase* previousSwapChain) {
    Device* device = ToBackend(GetDevice());
    PhysicalDevice* physicalDevice = ToBackend(GetDevice()->GetPhysicalDevice());

    VkSwapchainKHR previousVkSwapChain = VK_NULL_HANDLE;

    if (previousSwapChain != nullptr) {
        // TODO(crbug.com/dawn/269): The first time a surface is used with a Device, check
        // it is supported with vkGetPhysicalDeviceSurfaceSupportKHR.

        // TODO(crbug.com/dawn/269): figure out what should happen when surfaces are used by
        // multiple backends one after the other. It probably needs to block until the backend
        // and GPU are completely finished with the previous swapchain.
        DAWN_INVALID_IF(previousSwapChain->GetBackendType() != wgpu::BackendType::Vulkan,
                        "Vulkan SwapChain cannot switch backend types from %s to %s.",
                        previousSwapChain->GetBackendType(), wgpu::BackendType::Vulkan);

        SwapChain* previousVulkanSwapChain = ToBackend(previousSwapChain);

        // TODO(crbug.com/dawn/269): Figure out switching a single surface between multiple
        // Vulkan devices. Probably needs to block too!
        DAWN_INVALID_IF(previousSwapChain->GetDevice() != GetDevice(),
                        "Vulkan SwapChain cannot switch between devices.");

        // The previous swapchain is a dawn::native::vulkan::SwapChain so we can reuse its
        // VkSurfaceKHR provided since they are on the same instance.
        std::swap(previousVulkanSwapChain->mVkSurface, mVkSurface);

        // The previous swapchain was on the same Vulkan instance so we can use Vulkan's
        // "oldSwapchain" mechanism to ensure a seamless transition. We track the previous
        // swapchain for release immediately so it is not leaked in case of an error. (Vulkan
        // allows destroying it immediately after the call to vkCreateSwapChainKHR but tracking
        // using the fenced deleter makes the code simpler).
        std::swap(previousVulkanSwapChain->mSwapChain, previousVkSwapChain);
        ToBackend(previousSwapChain->GetDevice())
            ->GetFencedDeleter()
            ->DeleteWhenUnused(previousVkSwapChain);
    }

    if (mVkSurface == VK_NULL_HANDLE) {
        DAWN_TRY_ASSIGN(mVkSurface,
                        CreateVulkanSurface(device->GetInstance(), physicalDevice, GetSurface()));
    }

    VulkanSurfaceInfo surfaceInfo;
    DAWN_TRY_ASSIGN(surfaceInfo, GatherSurfaceInfo(*physicalDevice, mVkSurface));

    DAWN_TRY_ASSIGN(mConfig, ChooseConfig(surfaceInfo));

    // TODO(dawn:269): Choose config instead of hardcoding
    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.surface = mVkSurface;
    createInfo.minImageCount = mConfig.targetImageCount;
    createInfo.imageFormat = mConfig.format;
    createInfo.imageColorSpace = mConfig.colorSpace;
    createInfo.imageExtent = mConfig.extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = mConfig.usage;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    createInfo.preTransform = mConfig.transform;
    createInfo.compositeAlpha = mConfig.alphaMode;
    createInfo.presentMode = mConfig.presentMode;
    createInfo.clipped = false;
    createInfo.oldSwapchain = previousVkSwapChain;

    DAWN_TRY(CheckVkSuccess(
        device->fn.CreateSwapchainKHR(device->GetVkDevice(), &createInfo, nullptr, &*mSwapChain),
        "CreateSwapChain"));

    // Gather the swapchain's images. Implementations are allowed to return more images than the
    // number we asked for.
    uint32_t count = 0;
    DAWN_TRY(CheckVkSuccess(
        device->fn.GetSwapchainImagesKHR(device->GetVkDevice(), mSwapChain, &count, nullptr),
        "GetSwapChainImages1"));

    std::vector<VkImage> vkImages(count);
    DAWN_TRY(CheckVkSuccess(device->fn.GetSwapchainImagesKHR(device->GetVkDevice(), mSwapChain,
                                                             &count, AsVkArray(vkImages.data())),
                            "GetSwapChainImages2"));

    mImages.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        mImages[i].image = vkImages[i];
        DAWN_TRY_ASSIGN(mImages[i].renderingDoneSemaphore, CreateSemaphore(device));
    }

    return {};
}

ResultOrError<SwapChain::Config> SwapChain::ChooseConfig(
    const VulkanSurfaceInfo& surfaceInfo) const {
    Config config;

    // Choose the present mode. The only guaranteed one is FIFO so it has to be the fallback for
    // all other present modes. IMMEDIATE has tearing which is generally undesirable so it can't
    // be the fallback for MAILBOX. So the fallback order is always IMMEDIATE -> MAILBOX ->
    // FIFO.
    {
        auto HasPresentMode = [](const std::vector<VkPresentModeKHR>& modes,
                                 VkPresentModeKHR target) -> bool {
            return std::find(modes.begin(), modes.end(), target) != modes.end();
        };

        VkPresentModeKHR targetMode = ToVulkanPresentMode(GetPresentMode());
        const std::array<VkPresentModeKHR, 4> kPresentModeFallbacks = {
            VK_PRESENT_MODE_IMMEDIATE_KHR,
            VK_PRESENT_MODE_MAILBOX_KHR,
            VK_PRESENT_MODE_FIFO_RELAXED_KHR,
            VK_PRESENT_MODE_FIFO_KHR,
        };

        // Go to the target mode.
        size_t modeIndex = 0;
        while (kPresentModeFallbacks[modeIndex] != targetMode) {
            modeIndex++;
        }

        // Find the first available fallback.
        while (!HasPresentMode(surfaceInfo.presentModes, kPresentModeFallbacks[modeIndex])) {
            modeIndex++;
        }

        DAWN_ASSERT(modeIndex < kPresentModeFallbacks.size());
        config.presentMode = kPresentModeFallbacks[modeIndex];
    }

    // Choose the target width or do a blit.
    if (GetWidth() < surfaceInfo.capabilities.minImageExtent.width ||
        GetWidth() > surfaceInfo.capabilities.maxImageExtent.width ||
        GetHeight() < surfaceInfo.capabilities.minImageExtent.height ||
        GetHeight() > surfaceInfo.capabilities.maxImageExtent.height) {
        config.needsBlit = true;
    } else {
        config.extent.width = GetWidth();
        config.extent.height = GetHeight();
    }

    // Choose the target usage or do a blit.
    VkImageUsageFlags targetUsages =
        VulkanImageUsage(GetDevice(), GetUsage(), GetDevice()->GetValidInternalFormat(GetFormat()));
    VkImageUsageFlags supportedUsages = surfaceInfo.capabilities.supportedUsageFlags;
    if (!IsSubset(targetUsages, supportedUsages)) {
        config.needsBlit = true;
    } else {
        config.usage = targetUsages;
        config.wgpuUsage = GetUsage();
    }

    // Only support BGRA8Unorm (and RGBA8Unorm on android) with SRGB color space for now.
    config.wgpuFormat = GetFormat();
    config.format = VulkanImageFormat(ToBackend(GetDevice()), config.wgpuFormat);
    config.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    bool formatIsSupported = false;
    for (const VkSurfaceFormatKHR& format : surfaceInfo.formats) {
        if (format.format == config.format && format.colorSpace == config.colorSpace) {
            formatIsSupported = true;
            break;
        }
    }
    if (!formatIsSupported) {
        return DAWN_INTERNAL_ERROR(absl::StrFormat(
            "Vulkan SwapChain must support %s with sRGB colorspace.", config.wgpuFormat));
    }

    // Only the identity transform with opaque alpha is supported for now.
    DAWN_INVALID_IF(
        (surfaceInfo.capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) == 0,
        "Vulkan SwapChain must support the identity transform.");

    config.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    config.alphaMode = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
#if !DAWN_PLATFORM_IS(ANDROID)
    DAWN_INVALID_IF(
        (surfaceInfo.capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) == 0,
        "Vulkan SwapChain must support opaque alpha.");
#else
    // TODO(dawn:286): investigate composite alpha for WebGPU native
    VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (uint32_t i = 0; i < 4; i++) {
        if (surfaceInfo.capabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
            config.alphaMode = compositeAlphaFlags[i];
            break;
        }
    }
#endif  // #if !DAWN_PLATFORM_IS(ANDROID)

    // Choose the number of images for the swapchain= and clamp it to the min and max from the
    // surface capabilities. maxImageCount = 0 means there is no limit.
    DAWN_ASSERT(surfaceInfo.capabilities.maxImageCount == 0 ||
                surfaceInfo.capabilities.minImageCount <= surfaceInfo.capabilities.maxImageCount);
    uint32_t targetCount = MinImageCountForPresentMode(config.presentMode);

    targetCount = std::max(targetCount, surfaceInfo.capabilities.minImageCount);
    if (surfaceInfo.capabilities.maxImageCount != 0) {
        targetCount = std::min(targetCount, surfaceInfo.capabilities.maxImageCount);
    }

    config.targetImageCount = targetCount;

    // Choose a valid config for the swapchain texture that will receive the blit.
    if (config.needsBlit) {
        // Vulkan has provisions to have surfaces that adapt to the swapchain size. If that's
        // the case it is very likely that the target extent works, but clamp it just in case.
        // Using the target extent for the blit is better when possible so that texels don't
        // get stretched. This case is exposed by having the special "-1" value in both
        // dimensions of the extent.
        constexpr uint32_t kSpecialValue = 0xFFFF'FFFF;
        if (surfaceInfo.capabilities.currentExtent.width == kSpecialValue &&
            surfaceInfo.capabilities.currentExtent.height == kSpecialValue) {
            // extent = clamp(targetExtent, minExtent, maxExtent)
            config.extent.width = GetWidth();
            config.extent.width =
                std::min(config.extent.width, surfaceInfo.capabilities.maxImageExtent.width);
            config.extent.width =
                std::max(config.extent.width, surfaceInfo.capabilities.minImageExtent.width);

            config.extent.height = GetHeight();
            config.extent.height =
                std::min(config.extent.height, surfaceInfo.capabilities.maxImageExtent.height);
            config.extent.height =
                std::max(config.extent.height, surfaceInfo.capabilities.minImageExtent.height);
        } else {
            // If it is not an adaptable swapchain, just use the current extent for the blit
            // texture.
            config.extent = surfaceInfo.capabilities.currentExtent;
        }

        // TODO(crbug.com/dawn/269): If the swapchain image doesn't support TRANSFER_DST
        // then we'll need to have a second fallback that uses a blit shader :(
        if ((supportedUsages & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
            return DAWN_INTERNAL_ERROR(
                "SwapChain cannot fallback to a blit because of a missing "
                "VK_IMAGE_USAGE_TRANSFER_DST_BIT");
        }
        config.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        config.wgpuUsage = wgpu::TextureUsage::CopyDst;
    }

    return config;
}

MaybeError SwapChain::PresentImpl() {
    Device* device = ToBackend(GetDevice());

    Queue* queue = ToBackend(device->GetQueue());
    CommandRecordingContext* recordingContext = queue->GetPendingRecordingContext();

    if (mConfig.needsBlit) {
        // TODO(dawn:269): ditto same as present below: eagerly transition the blit texture to
        // CopySrc.
        mBlitTexture->TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopySrc,
                                         wgpu::ShaderStage::None,
                                         mBlitTexture->GetAllSubresources());
        mTexture->TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopyDst,
                                     wgpu::ShaderStage::None, mTexture->GetAllSubresources());

        VkImageBlit region;
        region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.srcSubresource.mipLevel = 0;
        region.srcSubresource.baseArrayLayer = 0;
        region.srcSubresource.layerCount = 1;
        region.srcOffsets[0] = {0, 0, 0};
        region.srcOffsets[1] = {static_cast<int32_t>(mBlitTexture->GetWidth(Aspect::Color)),
                                static_cast<int32_t>(mBlitTexture->GetHeight(Aspect::Color)), 1};

        region.dstSubresource = region.srcSubresource;
        region.dstOffsets[0] = {0, 0, 0};
        region.dstOffsets[1] = {static_cast<int32_t>(mTexture->GetWidth(Aspect::Color)),
                                static_cast<int32_t>(mTexture->GetHeight(Aspect::Color)), 1};

        device->fn.CmdBlitImage(recordingContext->commandBuffer, mBlitTexture->GetHandle(),
                                mBlitTexture->GetCurrentLayout(Aspect::Color),
                                mTexture->GetHandle(), mTexture->GetCurrentLayout(Aspect::Color), 1,
                                &region, VK_FILTER_LINEAR);

        // TODO(crbug.com/dawn/269): Find a way to reuse the blit texture between frames
        // instead of creating a new one every time. This will involve "un-destroying" the
        // texture or making the blit texture "external".
        mBlitTexture->APIDestroy();
        mBlitTexture = nullptr;
    }

    // TODO(crbug.com/dawn/269): Remove the need for this by eagerly transitioning the
    // presentable texture to present at the end of submits that use them and ideally even
    // folding that in the free layout transition at the end of render passes.
    mTexture->TransitionUsageNow(recordingContext, kPresentReleaseTextureUsage,
                                 wgpu::ShaderStage::None, mTexture->GetAllSubresources());

    // Use a semaphore to make sure all rendering has finished before presenting.
    VkSemaphore currentSemaphore = mImages[mLastImageIndex].renderingDoneSemaphore.Get();
    recordingContext->signalSemaphores.push_back(currentSemaphore);

    DAWN_TRY(queue->SubmitPendingCommands());

    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = AsVkArray(&currentSemaphore);
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &*mSwapChain;
    presentInfo.pImageIndices = &mLastImageIndex;
    presentInfo.pResults = nullptr;

    // Free the texture before present so error handling doesn't skip that step.
    mTexture->APIDestroy();
    mTexture = nullptr;

    VkResult result = VkResult::WrapUnsafe(
        device->fn.QueuePresentKHR(ToBackend(device->GetQueue())->GetVkQueue(), &presentInfo));

    switch (result) {
        case VK_SUCCESS:
        // VK_SUBOPTIMAL_KHR means "a swapchain no longer matches the surface properties
        // exactly, but can still be used to present to the surface successfully", so we
        // can also treat it as a "success" error code of vkQueuePresentKHR().
        case VK_SUBOPTIMAL_KHR:
            return {};

        // This present cannot be recovered. Re-initialize the VkSwapchain so that future
        // presents work..
        case VK_ERROR_OUT_OF_DATE_KHR:
            return Initialize(this);

        // TODO(crbug.com/dawn/269): Allow losing the surface at Dawn's API level?
        case VK_ERROR_SURFACE_LOST_KHR:
        default:
            return CheckVkSuccess(::VkResult(result), "QueuePresent");
    }
}

ResultOrError<SwapChainTextureInfo> SwapChain::GetCurrentTextureImpl() {
    return GetCurrentTextureInternal();
}

ResultOrError<SwapChainTextureInfo> SwapChain::GetCurrentTextureInternal(bool isReentrant) {
    Device* device = ToBackend(GetDevice());

    SwapChainTextureInfo swapChainTextureInfo = {};

    // Transiently create a semaphore that will be signaled when the presentation engine is done
    // with the swapchain image. Further operations on the image will wait for this semaphore.
    // We create one transiently instead of reusing one because if the application never does
    // rendering with the images, the semaphore won't get waited on, and will be forbidden to be
    // signaled again in future.
    UniqueVkHandle<VkSemaphore> acquireSemaphore;
    DAWN_TRY_ASSIGN(acquireSemaphore, CreateSemaphore(device));

    // Likewise create a fence that will be signaled, so we can wait on it later for frame pacing.
    UniqueVkHandle<VkFence> acquireFence;
    DAWN_TRY_ASSIGN(acquireFence, CreateFence(device));

    VkResult result = VkResult::WrapUnsafe(device->fn.AcquireNextImageKHR(
        device->GetVkDevice(), mSwapChain, std::numeric_limits<uint64_t>::max(),
        acquireSemaphore.Get(), acquireFence.Get(), &mLastImageIndex));

    switch (result) {
        case VK_SUCCESS:
            swapChainTextureInfo.status = wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal;
            break;

        case VK_SUBOPTIMAL_KHR:
            swapChainTextureInfo.status = wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal;
            break;

        case VK_ERROR_OUT_OF_DATE_KHR: {
            swapChainTextureInfo.status = wgpu::SurfaceGetCurrentTextureStatus::Outdated;
            // Prevent infinite recursive calls to GetCurrentTextureViewInternal when the
            // swapchains always return that they are out of date.
            if (isReentrant) {
                swapChainTextureInfo.status = wgpu::SurfaceGetCurrentTextureStatus::Lost;
                return swapChainTextureInfo;
            }

            // Re-initialize the VkSwapchain and try getting the texture again.
            DAWN_TRY(Initialize(this));
            return GetCurrentTextureInternal(true);
        }

        case VK_ERROR_SURFACE_LOST_KHR:
            swapChainTextureInfo.status = wgpu::SurfaceGetCurrentTextureStatus::Lost;
            return swapChainTextureInfo;

        default:
            DAWN_TRY(CheckVkSuccess(::VkResult(result), "AcquireNextImage"));
    }

    DAWN_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
    PerImage& lastImage = mImages[mLastImageIndex];

    // Make any further rendering operations that might use the swapchain texture wait on the
    // acquire to be finished.
    ToBackend(device->GetQueue())
        ->GetPendingRecordingContext()
        ->waitSemaphores.push_back(acquireSemaphore.Acquire());

    // Force some form of frame pacing by waiting on the CPU for the current texture to be done
    // rendering. Otherwise we could be queuing more frames on the same texture without ever
    // waiting, causing massive latency on user inputs.
    if (lastImage.lastAcquireDoneFence.Get() != VK_NULL_HANDLE) {
        DAWN_TRY(CheckVkSuccess(
            device->fn.WaitForFences(device->GetVkDevice(), 1,
                                     &*lastImage.lastAcquireDoneFence.Get(), true, UINT64_MAX),
            "SwapChain WaitForFences"));
    }
    lastImage.lastAcquireDoneFence = std::move(acquireFence);

    // Wait on the previous fence and destroy it.
    TextureDescriptor textureDesc;
    textureDesc.size.width = mConfig.extent.width;
    textureDesc.size.height = mConfig.extent.height;
    textureDesc.format = mConfig.wgpuFormat;
    textureDesc.usage = mConfig.wgpuUsage;

    mTexture = SwapChainTexture::Create(device, Unpack(&textureDesc), lastImage.image);

    // In the happy path we can use the swapchain image directly.
    if (!mConfig.needsBlit) {
        swapChainTextureInfo.texture = mTexture;
        return swapChainTextureInfo;
    }

    // The blit texture always perfectly matches what the user requested for the swapchain.
    // We need to add the Vulkan TRANSFER_SRC flag for the vkCmdBlitImage call.
    TextureDescriptor desc = GetSwapChainBaseTextureDescriptor(this);
    DAWN_TRY_ASSIGN(mBlitTexture, InternalTexture::Create(device, Unpack(&desc),
                                                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
    swapChainTextureInfo.texture = mBlitTexture;
    return swapChainTextureInfo;
}

void SwapChain::DetachFromSurfaceImpl() {
    if (mTexture != nullptr) {
        mTexture->APIDestroy();
        mTexture = nullptr;
    }

    if (mBlitTexture != nullptr) {
        mBlitTexture->APIDestroy();
        mBlitTexture = nullptr;
    }

    mImages.clear();

    // The swapchain images are destroyed with the swapchain.
    if (mSwapChain != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mSwapChain);
        mSwapChain = VK_NULL_HANDLE;
    }

    if (mVkSurface != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mVkSurface);
        mVkSurface = VK_NULL_HANDLE;
    }
}

ResultOrError<VkSurfaceKHR> CreateVulkanSurface(InstanceBase* instance,
                                                const PhysicalDevice* physicalDevice,
                                                const Surface* surface) {
    // May not be used in the platform-specific switches below.
    [[maybe_unused]] const VulkanGlobalInfo& info =
        physicalDevice->GetVulkanInstance()->GetGlobalInfo();
    [[maybe_unused]] const VulkanFunctions& fn =
        physicalDevice->GetVulkanInstance()->GetFunctions();
    [[maybe_unused]] VkInstance vkInstance = physicalDevice->GetVulkanInstance()->GetVkInstance();

    switch (surface->GetType()) {
#if defined(DAWN_ENABLE_BACKEND_METAL)
        case Surface::Type::MetalLayer:
            if (info.HasExt(InstanceExt::MetalSurface)) {
                VkMetalSurfaceCreateInfoEXT createInfo;
                createInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
                createInfo.pNext = nullptr;
                createInfo.flags = 0;
                createInfo.pLayer = surface->GetMetalLayer();

                VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
                DAWN_TRY(CheckVkSuccess(
                    fn.CreateMetalSurfaceEXT(vkInstance, &createInfo, nullptr, &*vkSurface),
                    "CreateMetalSurface"));
                return vkSurface;
            }
            break;
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if DAWN_PLATFORM_IS(WINDOWS)
        case Surface::Type::WindowsHWND:
            if (info.HasExt(InstanceExt::Win32Surface)) {
                VkWin32SurfaceCreateInfoKHR createInfo;
                createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
                createInfo.pNext = nullptr;
                createInfo.flags = 0;
                createInfo.hinstance = static_cast<HINSTANCE>(surface->GetHInstance());
                createInfo.hwnd = static_cast<HWND>(surface->GetHWND());

                VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
                DAWN_TRY(CheckVkSuccess(
                    fn.CreateWin32SurfaceKHR(vkInstance, &createInfo, nullptr, &*vkSurface),
                    "CreateWin32Surface"));
                return vkSurface;
            }
            break;
#endif  // DAWN_PLATFORM_IS(WINDOWS)

#if DAWN_PLATFORM_IS(ANDROID)
        case Surface::Type::AndroidWindow: {
            if (info.HasExt(InstanceExt::AndroidSurface)) {
                DAWN_ASSERT(surface->GetAndroidNativeWindow() != nullptr);

                VkAndroidSurfaceCreateInfoKHR createInfo;
                createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
                createInfo.pNext = nullptr;
                createInfo.flags = 0;
                createInfo.window =
                    static_cast<struct ANativeWindow*>(surface->GetAndroidNativeWindow());

                VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
                DAWN_TRY(CheckVkSuccess(
                    fn.CreateAndroidSurfaceKHR(vkInstance, &createInfo, nullptr, &*vkSurface),
                    "CreateAndroidSurfaceKHR"));
                return vkSurface;
            }

            break;
        }

#endif  // DAWN_PLATFORM_IS(ANDROID)

#if defined(DAWN_USE_WAYLAND)
        case Surface::Type::WaylandSurface: {
            if (info.HasExt(InstanceExt::XlibSurface)) {
                VkWaylandSurfaceCreateInfoKHR createInfo;
                createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
                createInfo.pNext = nullptr;
                createInfo.flags = 0;
                createInfo.display = static_cast<struct wl_display*>(surface->GetWaylandDisplay());
                createInfo.surface = static_cast<struct wl_surface*>(surface->GetWaylandSurface());

                VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
                DAWN_TRY(CheckVkSuccess(
                    fn.CreateWaylandSurfaceKHR(vkInstance, &createInfo, nullptr, &*vkSurface),
                    "CreateWaylandSurface"));
                return vkSurface;
            }
            break;
        }
#endif  // defined(DAWN_USE_WAYLAND)

#if defined(DAWN_USE_X11)
        case Surface::Type::XlibWindow: {
            if (info.HasExt(InstanceExt::XlibSurface)) {
                VkXlibSurfaceCreateInfoKHR createInfo;
                createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
                createInfo.pNext = nullptr;
                createInfo.flags = 0;
                createInfo.dpy = static_cast<Display*>(surface->GetXDisplay());
                createInfo.window = surface->GetXWindow();

                VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
                DAWN_TRY(CheckVkSuccess(
                    fn.CreateXlibSurfaceKHR(vkInstance, &createInfo, nullptr, &*vkSurface),
                    "CreateXlibSurface"));
                return vkSurface;
            }

            // Fall back to using XCB surfaces if the Xlib extension isn't available.
            // See https://xcb.freedesktop.org/MixingCalls/ for more information about
            // interoperability between Xlib and XCB
            const X11Functions* x11 = instance->GetOrLoadX11Functions();
            DAWN_ASSERT(x11 != nullptr);

            if (info.HasExt(InstanceExt::XcbSurface) && x11->IsX11XcbLoaded()) {
                VkXcbSurfaceCreateInfoKHR createInfo;
                createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
                createInfo.pNext = nullptr;
                createInfo.flags = 0;
                // The XCB connection lives as long as the X11 display.
                createInfo.connection =
                    x11->xGetXCBConnection(static_cast<Display*>(surface->GetXDisplay()));
                createInfo.window = surface->GetXWindow();

                VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
                DAWN_TRY(CheckVkSuccess(
                    fn.CreateXcbSurfaceKHR(vkInstance, &createInfo, nullptr, &*vkSurface),
                    "CreateXcbSurfaceKHR"));
                return vkSurface;
            }
            break;
        }
#endif  // defined(DAWN_USE_X11)

        default:
            break;
    }

    return DAWN_VALIDATION_ERROR("Unsupported surface type (%s) for Vulkan.", surface->GetType());
}

}  // namespace dawn::native::vulkan
