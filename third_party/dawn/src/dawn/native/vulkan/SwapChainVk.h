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

#ifndef SRC_DAWN_NATIVE_VULKAN_SWAPCHAINVK_H_
#define SRC_DAWN_NATIVE_VULKAN_SWAPCHAINVK_H_

#include <vector>

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/SwapChain.h"
#include "dawn/native/vulkan/UniqueVkHandle.h"

namespace dawn::native::vulkan {

class Device;
class SwapChainTexture;
class Texture;
class PhysicalDevice;
struct VulkanSurfaceInfo;

class SwapChain : public SwapChainBase {
  public:
    static ResultOrError<Ref<SwapChain>> Create(Device* device,
                                                Surface* surface,
                                                SwapChainBase* previousSwapChain,
                                                const SurfaceConfiguration* config);

    ~SwapChain() override;

  private:
    using SwapChainBase::SwapChainBase;
    MaybeError Initialize(SwapChainBase* previousSwapChain);

    struct Config {
        // Information that's passed to vulkan swapchain creation.
        VkPresentModeKHR presentMode;
        VkExtent2D extent;
        VkImageUsageFlags usage;
        VkFormat format;
        VkColorSpaceKHR colorSpace;
        uint32_t targetImageCount;
        VkSurfaceTransformFlagBitsKHR transform;
        VkCompositeAlphaFlagBitsKHR alphaMode;

        // Redundant information but as WebGPU enums to create the wgpu::Texture that
        // encapsulates the native swapchain texture.
        wgpu::TextureUsage wgpuUsage;
        wgpu::TextureFormat wgpuFormat;

        // Information about the blit workarounds we need to do (if any)
        bool needsBlit = false;
    };
    ResultOrError<Config> ChooseConfig(const VulkanSurfaceInfo& surfaceInfo) const;
    ResultOrError<SwapChainTextureInfo> GetCurrentTextureInternal(bool isReentrant = false);

    // SwapChainBase implementation
    MaybeError PresentImpl() override;
    ResultOrError<SwapChainTextureInfo> GetCurrentTextureImpl() override;
    void DetachFromSurfaceImpl() override;

    Config mConfig;

    VkSurfaceKHR mVkSurface = VK_NULL_HANDLE;
    VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;

    struct PerImage {
        VkImage image;
        // Used for the rendering -> present dependency, we need one semaphore per image because a
        // present may technically not be started when we signal the semaphore for the next frame.
        UniqueVkHandle<VkSemaphore> renderingDoneSemaphore;
        // Used for the last time acquired -> CPU wait for frame pacing.
        UniqueVkHandle<VkFence> lastAcquireDoneFence;
    };
    std::vector<PerImage> mImages;
    uint32_t mLastImageIndex = 0;

    Ref<Texture> mBlitTexture;
    Ref<Texture> mTexture;
};

ResultOrError<VkSurfaceKHR> CreateVulkanSurface(InstanceBase* instance,
                                                const PhysicalDevice* physicalDevice,
                                                const Surface* surface);

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_SWAPCHAINVK_H_
