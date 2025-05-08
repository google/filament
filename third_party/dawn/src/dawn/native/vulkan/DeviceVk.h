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

#ifndef SRC_DAWN_NATIVE_VULKAN_DEVICEVK_H_
#define SRC_DAWN_NATIVE_VULKAN_DEVICEVK_H_

#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/SerialQueue.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/native/vulkan/CommandRecordingContextVk.h"
#include "dawn/native/vulkan/DescriptorSetAllocator.h"
#include "dawn/native/vulkan/Forward.h"
#include "dawn/native/vulkan/VulkanFunctions.h"
#include "dawn/native/vulkan/VulkanInfo.h"

#include "dawn/native/vulkan/external_memory/MemoryService.h"
#include "dawn/native/vulkan/external_semaphore/SemaphoreService.h"

namespace dawn::native::vulkan {

class BufferUploader;
class FencedDeleter;
class RenderPassCache;
class ResourceMemoryAllocator;

class Device final : public DeviceBase {
  public:
    static ResultOrError<Ref<Device>> Create(AdapterBase* adapter,
                                             const UnpackedPtr<DeviceDescriptor>& descriptor,
                                             const TogglesState& deviceToggles,
                                             Ref<DeviceBase::DeviceLostEvent>&& lostEvent);
    ~Device() override;

    MaybeError Initialize(const UnpackedPtr<DeviceDescriptor>& descriptor);

    // Contains all the Vulkan entry points, vkDoFoo is called via device->fn.DoFoo.
    const VulkanFunctions fn;

    VkInstance GetVkInstance() const;
    const VulkanDeviceInfo& GetDeviceInfo() const;
    const VulkanGlobalInfo& GetGlobalInfo() const;
    VkDevice GetVkDevice() const;
    uint32_t GetGraphicsQueueFamily() const;

    MutexProtected<FencedDeleter>& GetFencedDeleter() const;
    RenderPassCache* GetRenderPassCache() const;
    MutexProtected<ResourceMemoryAllocator>& GetResourceMemoryAllocator() const;
    external_semaphore::Service* GetExternalSemaphoreService() const;

    void EnqueueDeferredDeallocation(DescriptorSetAllocator* allocator);

    // Dawn Native API

    Ref<TextureBase> CreateTextureWrappingVulkanImage(
        const ExternalImageDescriptorVk* descriptor,
        ExternalMemoryHandle memoryHandle,
        const std::vector<ExternalSemaphoreHandle>& waitHandles);
    bool SignalAndExportExternalTexture(Texture* texture,
                                        VkImageLayout desiredLayout,
                                        ExternalImageExportInfoVk* info,
                                        std::vector<ExternalSemaphoreHandle>* semaphoreHandle);

    ResultOrError<Ref<CommandBufferBase>> CreateCommandBuffer(
        CommandEncoder* encoder,
        const CommandBufferDescriptor* descriptor) override;

    MaybeError TickImpl() override;

    MaybeError CopyFromStagingToBufferImpl(BufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) override;
    MaybeError CopyFromStagingToTextureImpl(const BufferBase* source,
                                            const TexelCopyBufferLayout& src,
                                            const TextureCopy& dst,
                                            const Extent3D& copySizePixels) override;

    // Return the fixed subgroup size to use for compute shaders on this device or 0 if none
    // needs to be set.
    uint32_t GetComputeSubgroupSize() const;

    uint32_t GetOptimalBytesPerRowAlignment() const override;
    uint64_t GetOptimalBufferToTextureCopyOffsetAlignment() const override;

    float GetTimestampPeriodInNS() const override;

    AllocatorMemoryInfo GetAllocatorMemoryInfo() const override;

    void SetLabelImpl() override;
    bool ReduceMemoryUsageImpl() override;
    void PerformIdleTasksImpl() override;

    void OnDebugMessage(std::string message);

    // Used to associate this device with validation layer messages.
    const char* GetDebugPrefix() { return mDebugPrefix.c_str(); }

    bool CanAddStorageUsageToBufferWithoutSideEffects(wgpu::BufferUsage storageUsage,
                                                      wgpu::BufferUsage originalUsage,
                                                      size_t bufferSize) const override;

  private:
    Device(AdapterBase* adapter,
           const UnpackedPtr<DeviceDescriptor>& descriptor,
           const TogglesState& deviceToggles,
           Ref<DeviceBase::DeviceLostEvent>&& lostEvent);

    ResultOrError<Ref<BindGroupBase>> CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) override;
    ResultOrError<Ref<BindGroupLayoutInternalBase>> CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) override;
    ResultOrError<Ref<BufferBase>> CreateBufferImpl(
        const UnpackedPtr<BufferDescriptor>& descriptor) override;
    ResultOrError<Ref<PipelineLayoutBase>> CreatePipelineLayoutImpl(
        const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) override;
    ResultOrError<Ref<QuerySetBase>> CreateQuerySetImpl(
        const QuerySetDescriptor* descriptor) override;
    ResultOrError<Ref<SamplerBase>> CreateSamplerImpl(const SamplerDescriptor* descriptor) override;
    ResultOrError<Ref<ShaderModuleBase>> CreateShaderModuleImpl(
        const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
        const std::vector<tint::wgsl::Extension>& internalExtensions,
        ShaderModuleParseResult* parseResult,
        OwnedCompilationMessages* compilationMessages) override;
    ResultOrError<Ref<SwapChainBase>> CreateSwapChainImpl(
        Surface* surface,
        SwapChainBase* previousSwapChain,
        const SurfaceConfiguration* config) override;
    ResultOrError<Ref<TextureBase>> CreateTextureImpl(
        const UnpackedPtr<TextureDescriptor>& descriptor) override;
    ResultOrError<Ref<TextureViewBase>> CreateTextureViewImpl(
        TextureBase* texture,
        const UnpackedPtr<TextureViewDescriptor>& descriptor) override;
    Ref<ComputePipelineBase> CreateUninitializedComputePipelineImpl(
        const UnpackedPtr<ComputePipelineDescriptor>& descriptor) override;
    Ref<RenderPipelineBase> CreateUninitializedRenderPipelineImpl(
        const UnpackedPtr<RenderPipelineDescriptor>& descriptor) override;
    Ref<PipelineCacheBase> GetOrCreatePipelineCacheImpl(const CacheKey& key) override;
    void InitializeComputePipelineAsyncImpl(Ref<CreateComputePipelineAsyncEvent> event) override;
    void InitializeRenderPipelineAsyncImpl(Ref<CreateRenderPipelineAsyncEvent> event) override;

    ResultOrError<Ref<SharedTextureMemoryBase>> ImportSharedTextureMemoryImpl(
        const SharedTextureMemoryDescriptor* baseDescriptor) override;
    ResultOrError<Ref<SharedFenceBase>> ImportSharedFenceImpl(
        const SharedFenceDescriptor* baseDescriptor) override;

    ResultOrError<VulkanDeviceKnobs> CreateDevice(VkPhysicalDevice vkPhysicalDevice);

    MaybeError CheckDebugLayerAndGenerateErrors();
    void AppendDebugLayerMessages(ErrorData* error) override;
    void CheckDebugMessagesAfterDestruction() const;

    void DestroyImpl() override;
    MaybeError GetAHardwareBufferPropertiesImpl(void* handle, AHardwareBufferProperties* properties)
        const override;

    // To make it easier to use fn it is a public const member. However
    // the Device is allowed to mutate them through these private methods.
    VulkanFunctions* GetMutableFunctions();

    VulkanDeviceInfo mDeviceInfo = {};
    VkDevice mVkDevice = VK_NULL_HANDLE;
    uint32_t mMainQueueFamily = 0;

    // Entries can be appended without holding the device mutex.
    MutexProtected<SerialQueue<ExecutionSerial, Ref<DescriptorSetAllocator>>>
        mDescriptorAllocatorsPendingDeallocation;
    std::unique_ptr<MutexProtected<FencedDeleter>> mDeleter;
    std::unique_ptr<MutexProtected<ResourceMemoryAllocator>> mResourceMemoryAllocator;
    std::unique_ptr<RenderPassCache> mRenderPassCache;

    std::unique_ptr<external_memory::Service> mExternalMemoryService;
    std::unique_ptr<external_semaphore::Service> mExternalSemaphoreService;

    // For capturing messages generated by the Vulkan debug layer.
    const std::string mDebugPrefix;
    std::vector<std::string> mDebugMessages;

    Ref<PipelineCache> mMonolithicPipelineCache;

    bool mSupportsMappableStorageBuffer = false;

    MaybeError ImportExternalImage(const ExternalImageDescriptorVk* descriptor,
                                   ExternalMemoryHandle memoryHandle,
                                   VkImage image,
                                   const std::vector<ExternalSemaphoreHandle>& waitHandles,
                                   VkDeviceMemory* outAllocation,
                                   std::vector<VkSemaphore>* outWaitSemaphores);
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_DEVICEVK_H_
