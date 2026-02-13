// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_NULL_DEVICENULL_H_
#define SRC_DAWN_NATIVE_NULL_DEVICENULL_H_

#include <memory>
#include <vector>

#include "dawn/native/BindGroup.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Device.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/Queue.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/RingBufferAllocator.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/ShaderModule.h"
#include "dawn/native/SwapChain.h"
#include "dawn/native/Texture.h"
#include "dawn/native/ToBackend.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::null {

class BindGroup;
class BindGroupLayout;
class Buffer;
class CommandBuffer;
class ComputePipeline;
class Device;
class PhysicalDevice;
using PipelineLayout = PipelineLayoutBase;
class QuerySet;
class Queue;
class RenderPipeline;
using Sampler = SamplerBase;
class ShaderModule;
class SwapChain;
class Texture;
using TextureView = TextureViewBase;

struct NullBackendTraits {
    using BindGroupType = BindGroup;
    using BindGroupLayoutType = BindGroupLayout;
    using BufferType = Buffer;
    using CommandBufferType = CommandBuffer;
    using ComputePipelineType = ComputePipeline;
    using DeviceType = Device;
    using PhysicalDeviceType = PhysicalDevice;
    using PipelineLayoutType = PipelineLayout;
    using QuerySetType = QuerySet;
    using QueueType = Queue;
    using RenderPipelineType = RenderPipeline;
    using SamplerType = Sampler;
    using ShaderModuleType = ShaderModule;
    using SwapChainType = SwapChain;
    using TextureType = Texture;
    using TextureViewType = TextureView;
};

template <typename T>
auto ToBackend(T&& common) -> decltype(ToBackendBase<NullBackendTraits>(common)) {
    return ToBackendBase<NullBackendTraits>(common);
}

struct PendingOperation {
    virtual ~PendingOperation() = default;
    virtual void Execute() = 0;
};

class Device final : public DeviceBase {
  public:
    static ResultOrError<Ref<Device>> Create(AdapterBase* adapter,
                                             const UnpackedPtr<DeviceDescriptor>& descriptor,
                                             const TogglesState& deviceToggles,
                                             Ref<DeviceBase::DeviceLostEvent>&& lostEvent);
    ~Device() override;

    MaybeError Initialize(const UnpackedPtr<DeviceDescriptor>& descriptor);

    ResultOrError<Ref<CommandBufferBase>> CreateCommandBuffer(
        CommandEncoder* encoder,
        const CommandBufferDescriptor* descriptor) override;

    MaybeError TickImpl() override;

    void AddPendingOperation(std::unique_ptr<PendingOperation> operation);
    MaybeError SubmitPendingOperations();
    void ForgetPendingOperations();

    MaybeError CopyFromStagingToBuffer(BufferBase* source,
                                       uint64_t sourceOffset,
                                       BufferBase* destination,
                                       uint64_t destinationOffset,
                                       uint64_t size) override;
    MaybeError CopyFromStagingToTextureImpl(const BufferBase* source,
                                            const TexelCopyBufferLayout& src,
                                            const TextureCopy& dst,
                                            const Extent3D& copySizePixels) override;

    MaybeError IncrementMemoryUsage(uint64_t bytes);
    void DecrementMemoryUsage(uint64_t bytes);

    uint32_t GetOptimalBytesPerRowAlignment() const override;
    uint64_t GetOptimalBufferToTextureCopyOffsetAlignment() const override;

    float GetTimestampPeriodInNS() const override;

    bool CanTextureLoadResolveTargetInTheSameRenderpass() const override;

  private:
    using DeviceBase::DeviceBase;

    ResultOrError<Ref<BindGroupBase>> CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) override;
    ResultOrError<Ref<BindGroupLayoutInternalBase>> CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) override;
    ResultOrError<Ref<BufferBase>> CreateBufferImpl(
        const UnpackedPtr<BufferDescriptor>& descriptor) override;
    Ref<ComputePipelineBase> CreateUninitializedComputePipelineImpl(
        const UnpackedPtr<ComputePipelineDescriptor>& descriptor) override;
    ResultOrError<Ref<PipelineLayoutBase>> CreatePipelineLayoutImpl(
        const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) override;
    ResultOrError<Ref<QuerySetBase>> CreateQuerySetImpl(
        const QuerySetDescriptor* descriptor) override;
    Ref<RenderPipelineBase> CreateUninitializedRenderPipelineImpl(
        const UnpackedPtr<RenderPipelineDescriptor>& descriptor) override;
    ResultOrError<Ref<SamplerBase>> CreateSamplerImpl(const SamplerDescriptor* descriptor) override;
    ResultOrError<Ref<ShaderModuleBase>> CreateShaderModuleImpl(
        const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
        const std::vector<tint::wgsl::Extension>& internalExtensions,
        ShaderModuleParseResult* parseResult) override;
    ResultOrError<Ref<SwapChainBase>> CreateSwapChainImpl(
        Surface* surface,
        SwapChainBase* previousSwapChain,
        const SurfaceConfiguration* config) override;
    ResultOrError<Ref<TextureBase>> CreateTextureImpl(
        const UnpackedPtr<TextureDescriptor>& descriptor) override;
    ResultOrError<Ref<TextureViewBase>> CreateTextureViewImpl(
        TextureBase* texture,
        const UnpackedPtr<TextureViewDescriptor>& descriptor) override;

    void DestroyImpl() override;

    std::vector<std::unique_ptr<PendingOperation>> mPendingOperations;

    static constexpr uint64_t kMaxMemoryUsage = 512 * 1024 * 1024;
    size_t mMemoryUsage = 0;
};

class PhysicalDevice : public PhysicalDeviceBase {
  public:
    static Ref<PhysicalDevice> Create();

    ~PhysicalDevice() override;

    // PhysicalDeviceBase Implementation
    bool SupportsExternalImages() const override;

    bool SupportsFeatureLevel(wgpu::FeatureLevel featureLevel,
                              InstanceBase* instance) const override;

    ResultOrError<PhysicalDeviceSurfaceCapabilities> GetSurfaceCapabilities(
        InstanceBase* instance,
        const Surface* surface) const override;

    // Used for the tests that intend to use an adapter without all features enabled.
    using PhysicalDeviceBase::SetSupportedFeaturesForTesting;

  private:
    PhysicalDevice();

    MaybeError InitializeImpl() override;
    void InitializeSupportedFeaturesImpl() override;
    MaybeError InitializeSupportedLimitsImpl(CombinedLimits* limits) override;

    FeatureValidationResult ValidateFeatureSupportedWithTogglesImpl(
        wgpu::FeatureName feature,
        const TogglesState& toggles) const override;

    void SetupBackendAdapterToggles(dawn::platform::Platform* platform,
                                    TogglesState* adapterToggles) const override;
    void SetupBackendDeviceToggles(dawn::platform::Platform* platform,
                                   TogglesState* deviceToggles) const override;
    ResultOrError<Ref<DeviceBase>> CreateDeviceImpl(
        AdapterBase* adapter,
        const UnpackedPtr<DeviceDescriptor>& descriptor,
        const TogglesState& deviceToggles,
        Ref<DeviceBase::DeviceLostEvent>&& lostEvent) override;

    void PopulateBackendProperties(UnpackedPtr<AdapterInfo>& info) const override;
};

// Helper class so |BindGroup| can allocate memory for its binding data,
// before calling the BindGroupBase base class constructor.
class BindGroupDataHolder {
  protected:
    explicit BindGroupDataHolder(size_t size);
    ~BindGroupDataHolder();

    raw_ptr<void> mBindingDataAllocation;
};

// We don't have the complexity of placement-allocation of bind group data in
// the Null backend. This class, keeps the binding data in a separate allocation for simplicity.
class BindGroup final : private BindGroupDataHolder, public BindGroupBase {
  public:
    BindGroup(DeviceBase* device, const BindGroupDescriptor* descriptor);

  private:
    ~BindGroup() override = default;

    MaybeError InitializeImpl() override;
};

class BindGroupLayout final : public BindGroupLayoutInternalBase {
  public:
    BindGroupLayout(DeviceBase* device, const BindGroupLayoutDescriptor* descriptor);

  private:
    ~BindGroupLayout() override = default;
};

class Buffer final : public BufferBase {
  public:
    Buffer(Device* device, const UnpackedPtr<BufferDescriptor>& descriptor);

    void CopyFromStaging(BufferBase* staging,
                         uint64_t sourceOffset,
                         uint64_t destinationOffset,
                         uint64_t size);

    void DoWriteBuffer(uint64_t bufferOffset, const void* data, size_t size);

  private:
    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override;
    void UnmapImpl() override;
    void DestroyImpl() override;
    bool IsCPUWritableAtCreation() const override;
    MaybeError MapAtCreationImpl() override;
    void* GetMappedPointerImpl() override;

    std::unique_ptr<uint8_t[]> mBackingData;
};

class CommandBuffer final : public CommandBufferBase {
  public:
    CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor);
};

class QuerySet final : public QuerySetBase {
  public:
    QuerySet(Device* device, const QuerySetDescriptor* descriptor);
};

class Queue final : public QueueBase {
  public:
    Queue(Device* device, const QueueDescriptor* descriptor);

  private:
    ~Queue() override;
    MaybeError SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) override;
    MaybeError WriteBufferImpl(BufferBase* buffer,
                               uint64_t bufferOffset,
                               const void* data,
                               size_t size) override;
    ResultOrError<ExecutionSerial> CheckAndUpdateCompletedSerials() override;
    void ForceEventualFlushOfCommands() override;
    bool HasPendingCommands() const override;
    MaybeError SubmitPendingCommandsImpl() override;
    ResultOrError<ExecutionSerial> WaitForQueueSerialImpl(ExecutionSerial waitSerial,
                                                          Nanoseconds timeout) override;
    MaybeError WaitForIdleForDestruction() override;
};

class ComputePipeline final : public ComputePipelineBase {
  public:
    using ComputePipelineBase::ComputePipelineBase;

    MaybeError InitializeImpl() override;
};

class RenderPipeline final : public RenderPipelineBase {
  public:
    using RenderPipelineBase::RenderPipelineBase;

    MaybeError InitializeImpl() override;
};

class ShaderModule final : public ShaderModuleBase {
  public:
    using ShaderModuleBase::ShaderModuleBase;

    MaybeError Initialize(ShaderModuleParseResult* parseResult);
};

class SwapChain final : public SwapChainBase {
  public:
    static ResultOrError<Ref<SwapChain>> Create(Device* device,
                                                Surface* surface,
                                                SwapChainBase* previousSwapChain,
                                                const SurfaceConfiguration* config);
    ~SwapChain() override;

  private:
    using SwapChainBase::SwapChainBase;
    MaybeError Initialize(SwapChainBase* previousSwapChain);

    Ref<Texture> mTexture;

    MaybeError PresentImpl() override;
    ResultOrError<SwapChainTextureInfo> GetCurrentTextureImpl() override;
    void DetachFromSurfaceImpl() override;
};

class Texture : public TextureBase {
  public:
    Texture(DeviceBase* device, const UnpackedPtr<TextureDescriptor>& descriptor);
};

}  // namespace dawn::native::null

#endif  // SRC_DAWN_NATIVE_NULL_DEVICENULL_H_
