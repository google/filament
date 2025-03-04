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

#ifndef SRC_DAWN_NATIVE_VULKAN_TEXTUREVK_H_
#define SRC_DAWN_NATIVE_VULKAN_TEXTUREVK_H_

#include <memory>
#include <vector>

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/PassResourceUsage.h"
#include "dawn/native/ResourceMemoryAllocation.h"
#include "dawn/native/Texture.h"
#include "dawn/native/vulkan/ExternalHandle.h"
#include "dawn/native/vulkan/SharedTextureMemoryVk.h"
#include "dawn/native/vulkan/external_memory/MemoryService.h"
#include "dawn/native/vulkan/external_semaphore/SemaphoreService.h"

namespace dawn::native::vulkan {

struct CommandRecordingContext;
class Device;
class Texture;

VkFormat VulkanImageFormat(const Device* device, wgpu::TextureFormat format);
// This version does not support depth stencil formats which can depend on
// properties of the Device.
VkFormat ColorVulkanImageFormat(wgpu::TextureFormat format);
ResultOrError<wgpu::TextureFormat> FormatFromVkFormat(const Device* device, VkFormat vkFormat);
VkImageUsageFlags VulkanImageUsage(const DeviceBase* device,
                                   wgpu::TextureUsage usage,
                                   const Format& format);
VkImageLayout VulkanImageLayout(const Format& format, wgpu::TextureUsage usage);
VkImageLayout VulkanImageLayoutForDepthStencilAttachment(const Format& format,
                                                         bool depthReadOnly,
                                                         bool stencilReadOnly);
VkSampleCountFlagBits VulkanSampleCount(uint32_t sampleCount);

MaybeError ValidateVulkanImageCanBeWrapped(const DeviceBase* device,
                                           const UnpackedPtr<TextureDescriptor>& descriptor);

bool IsSampleCountSupported(const dawn::native::vulkan::Device* device,
                            const VkImageCreateInfo& imageCreateInfo);

// Base class for all Texture implementation on Vulkan that handles the common logic for barrier
// tracking, view creation, etc. Cannot be created directly, instead InternalTexture is the
// Dawn-controlled texture.
class Texture : public TextureBase {
  public:
    VkImage GetHandle() const;
    // Returns the aspects used for tracking of Vulkan state. These can be the combined aspects.
    Aspect GetDisjointVulkanAspects() const;

    VkImageLayout GetCurrentLayout(Aspect aspect,
                                   uint32_t arrayLayer = 0,
                                   uint32_t mipLevel = 0) const;

    // Transitions the texture to be used as `usage`, recording any necessary barrier in
    // `commands`.
    // TODO(crbug.com/dawn/851): coalesce barriers and do them early when possible.
    void TransitionUsageNow(CommandRecordingContext* recordingContext,
                            wgpu::TextureUsage usage,
                            wgpu::ShaderStage shaderStages,
                            const SubresourceRange& range);
    void TransitionUsageForPass(CommandRecordingContext* recordingContext,
                                const TextureSubresourceSyncInfo& textureSyncInfos,
                                std::vector<VkImageMemoryBarrier>* imageBarriers,
                                VkPipelineStageFlags* srcStages,
                                VkPipelineStageFlags* dstStages);
    // Change the texture to be used as `usage`. Note: this function assumes the barriers are
    // already invoked before calling it. Typical use case is an input attachment, at the beginning
    // of render pass, its usage is transitioned to TextureBinding. Then subpass' dependency
    // automatically transitions the texture to RenderAttachment without any explicit barrier call.
    void UpdateUsage(wgpu::TextureUsage usage,
                     wgpu::ShaderStage shaderStages,
                     const SubresourceRange& range);

    MaybeError EnsureSubresourceContentInitialized(CommandRecordingContext* recordingContext,
                                                   const SubresourceRange& range);

    void SetLabelHelper(const char* prefix);

    // Dawn API
    void SetLabelImpl() override;

  protected:
    Texture(Device* device, const UnpackedPtr<TextureDescriptor>& descriptor);

    void DestroyImpl() override;
    MaybeError ClearTexture(CommandRecordingContext* recordingContext,
                            const SubresourceRange& range,
                            TextureBase::ClearValue);

    // Implementation details of the barrier computations for the texture.
    void TransitionUsageAndGetResourceBarrier(wgpu::TextureUsage usage,
                                              wgpu::ShaderStage shaderStages,
                                              const SubresourceRange& range,
                                              std::vector<VkImageMemoryBarrier>* imageBarriers,
                                              VkPipelineStageFlags* srcStages,
                                              VkPipelineStageFlags* dstStages);
    void TransitionUsageForPassImpl(CommandRecordingContext* recordingContext,
                                    const SubresourceStorage<TextureSyncInfo>& subresourceSyncInfos,
                                    std::vector<VkImageMemoryBarrier>* imageBarriers,
                                    VkPipelineStageFlags* srcStages,
                                    VkPipelineStageFlags* dstStages);
    void TransitionUsageAndGetResourceBarrierImpl(wgpu::TextureUsage usage,
                                                  wgpu::ShaderStage shaderStages,
                                                  const SubresourceRange& range,
                                                  std::vector<VkImageMemoryBarrier>* imageBarriers,
                                                  VkPipelineStageFlags* srcStages,
                                                  VkPipelineStageFlags* dstStages);

    // TODO(42242084): Make this more robust and maybe predicated on a boolean as we're in hot code.
    virtual bool CanReuseWithoutBarrier(wgpu::TextureUsage lastUsage,
                                        wgpu::TextureUsage usage,
                                        wgpu::ShaderStage lastShaderStage,
                                        wgpu::ShaderStage shaderStage);
    virtual void TweakTransition(CommandRecordingContext* recordingContext,
                                 std::vector<VkImageMemoryBarrier>* barriers,
                                 size_t transitionBarrierStart);

    // Sometimes the WebGPU aspects don't directly map to Vulkan aspects:
    //
    //  - In early Vulkan versions it is not possible to transition depth and stencil separetely so
    //    textures with Depth|Stencil will be promoted to a single CombinedDepthStencil aspect
    //    internally.
    //  - Some multiplanar images cannot have planes transitioned separately and instead Vulkan
    //    requires that the "Color" aspect be used for barriers, so Plane0|Plane1 is promoted to
    //    just Color.
    //
    // This variable, if not Aspect::None, is the combined aspect to use for all transitions.
    const Aspect mCombinedAspect;
    bool UseCombinedAspects() const;

    SubresourceStorage<TextureSyncInfo> mSubresourceLastSyncInfos;
    VkImage mHandle = VK_NULL_HANDLE;
};

// A texture created and fully owned by Dawn. Typically the result of device.CreateTexture.
class InternalTexture final : public Texture {
  public:
    static ResultOrError<Ref<InternalTexture>> Create(
        Device* device,
        const UnpackedPtr<TextureDescriptor>& descriptor,
        VkImageUsageFlags extraUsages = 0);

  private:
    using Texture::Texture;
    MaybeError Initialize(VkImageUsageFlags extraUsages);
    void DestroyImpl() override;

    ResourceMemoryAllocation mMemoryAllocation;
};

// A texture owned by a VkSwapChain.
class SwapChainTexture final : public Texture {
  public:
    static Ref<SwapChainTexture> Create(Device* device,
                                        const UnpackedPtr<TextureDescriptor>& descriptor,
                                        VkImage nativeImage);

  private:
    using Texture::Texture;
    void Initialize(VkImage nativeImage);
};

// TODO(330385376): Merge in SharedTexture once ExternalImageDescriptorVk is fully removed.
class ImportedTextureBase : public Texture {
  public:
    virtual std::vector<VkSemaphore> AcquireWaitRequirements() { return {}; }

    // Eagerly transition the texture for export.
    void TransitionEagerlyForExport(CommandRecordingContext* recordingContext);

    // Update the 'ExternalSemaphoreHandle' to be used for export with the newly submitted one.
    void UpdateExternalSemaphoreHandle(ExternalSemaphoreHandle handle);

    // If needed, modifies the VkImageMemoryBarrier to perform a queue ownership transfer etc.
    void TweakTransition(CommandRecordingContext* recordingContext,
                         std::vector<VkImageMemoryBarrier>* barriers,
                         size_t transitionBarrierStart) override;

    bool CanReuseWithoutBarrier(wgpu::TextureUsage lastUsage,
                                wgpu::TextureUsage usage,
                                wgpu::ShaderStage lastShaderStage,
                                wgpu::ShaderStage shaderStage) override;

    // Performs the steps to export a texture and returns the export information.
    MaybeError EndAccess(ExternalSemaphoreHandle* handle,
                         VkImageLayout* releasedOldLayout,
                         VkImageLayout* releasedNewLayout);

  protected:
    using Texture::Texture;
    ~ImportedTextureBase() override;
    // The states of an external texture:
    //   PendingAcquire: Initialized as an external texture already, but unavailable for access yet.
    //   Acquired: Ready for access.
    //   EagerlyTransitioned: The texture has ever been used, and eagerly transitioned for export.
    //   Now it can be acquired for access again, or directly exported.
    //   Released: The texture is not associated to any external resource and cannot be used. This
    //   can happen before initialization, or after destruction.
    enum class ExternalState { PendingAcquire, Acquired, EagerlyTransitioned, Released };
    ExternalState mExternalState = ExternalState::Released;
    ExternalState mLastExternalState = ExternalState::Released;

    // The layouts to use for the queue ownership transfer barrier when a texture is used the first
    // time after being imported. The layouts must match the ones from the queue ownership transfer
    // barrier done for the export operation.
    VkImageLayout mPendingAcquireOldLayout;
    VkImageLayout mPendingAcquireNewLayout;

    // Which of FOREIGN or EXTERNAL queue family to use when exporting.
    uint32_t mExportQueueFamilyIndex;
    // The layout requested for the export, or UNDEFINED if the receiver can handle whichever layout
    // was current.
    VkImageLayout mDesiredExportLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // If the texture was ever used, represents a semaphore signaled once operations on the texture
    // are done so that the receiver of the export can synchronize properly.
    ExternalSemaphoreHandle mExternalSemaphoreHandle = kNullExternalSemaphoreHandle;
};

// A texture created from an VkImage that references an external memory object.
class ExternalVkImageTexture final : public ImportedTextureBase {
  public:
    static ResultOrError<Ref<ExternalVkImageTexture>> Create(
        Device* device,
        const ExternalImageDescriptorVk* descriptor,
        const UnpackedPtr<TextureDescriptor>& textureDescriptor,
        external_memory::Service* externalMemoryService);

    // Binds externally allocated memory to the VkImage and on success, takes ownership of
    // semaphores.
    MaybeError BindExternalMemory(const ExternalImageDescriptorVk* descriptor,
                                  VkDeviceMemory externalMemoryAllocation,
                                  std::vector<VkSemaphore> waitSemaphores);

    MaybeError ExportExternalTexture(VkImageLayout desiredLayout,
                                     ExternalSemaphoreHandle* handle,
                                     VkImageLayout* releasedOldLayout,
                                     VkImageLayout* releasedNewLayout);

    std::vector<VkSemaphore> AcquireWaitRequirements() override;

  private:
    using ImportedTextureBase::ImportedTextureBase;
    MaybeError Initialize(const ExternalImageDescriptorVk* descriptor,
                          external_memory::Service* externalMemoryService);
    void DestroyImpl() override;

    VkDeviceMemory mExternalAllocation = VK_NULL_HANDLE;
    std::vector<VkSemaphore> mWaitRequirements;
};

// A texture created from a SharedTextureMemory
class SharedTexture final : public ImportedTextureBase {
  public:
    static ResultOrError<Ref<SharedTexture>> Create(
        SharedTextureMemory* memory,
        const UnpackedPtr<TextureDescriptor>& textureDescriptor);

    void SetPendingAcquire(VkImageLayout pendingAcquireOldLayout,
                           VkImageLayout pendingAcquireNewLayout);

  private:
    using ImportedTextureBase::ImportedTextureBase;
    void Initialize(SharedTextureMemory* memory);
    void DestroyImpl() override;

    struct SharedTextureMemoryObjects {
        Ref<RefCountedVkHandle<VkImage>> vkImage;
        Ref<RefCountedVkHandle<VkDeviceMemory>> vkDeviceMemory;
    };
    SharedTextureMemoryObjects mSharedTextureMemoryObjects;
};

class TextureView final : public TextureViewBase {
  public:
    static ResultOrError<Ref<TextureView>> Create(
        TextureBase* texture,
        const UnpackedPtr<TextureViewDescriptor>& descriptor);
    VkImageView GetHandle() const;
    VkImageView GetHandleForBGRA8UnormStorage() const;

    ResultOrError<VkImageView> GetOrCreate2DViewOn3D(uint32_t depthSlice = 0u);

    bool IsYCbCr() const override;
    YCbCrVkDescriptor GetYCbCrVkDescriptor() const override;

  private:
    ~TextureView() override;
    void DestroyImpl() override;
    using TextureViewBase::TextureViewBase;
    MaybeError Initialize(const UnpackedPtr<TextureViewDescriptor>& descriptor);

    VkImageViewCreateInfo GetCreateInfo(wgpu::TextureFormat format,
                                        wgpu::TextureViewDimension dimension,
                                        uint32_t depthSlice = 0u) const;

    // Dawn API
    void SetLabelImpl() override;

    VkImageView mHandle = VK_NULL_HANDLE;
    VkImageView mHandleForBGRA8UnormStorage = VK_NULL_HANDLE;
    VkSamplerYcbcrConversion mSamplerYCbCrConversion = VK_NULL_HANDLE;
    bool mIsYCbCr = false;
    YCbCrVkDescriptor mYCbCrVkDescriptor;
    std::vector<VkImageView> mHandlesFor2DViewOn3D;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_TEXTUREVK_H_
