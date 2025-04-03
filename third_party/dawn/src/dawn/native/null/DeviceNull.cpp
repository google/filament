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

#include "dawn/native/null/DeviceNull.h"

#include <limits>
#include <utility>

#include "dawn/native/BackendConnection.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Commands.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/Instance.h"
#include "dawn/native/Surface.h"
#include "dawn/native/TintUtils.h"
#include "partition_alloc/pointers/raw_ptr.h"

#include "tint/tint.h"

namespace dawn::native::null {

// Implementation of pre-Device objects: the null physical device, null backend connection and
// Connect()

PhysicalDevice::PhysicalDevice() : PhysicalDeviceBase(wgpu::BackendType::Null) {
    mVendorId = 0;
    mDeviceId = 0;
    mName = "Null backend";
    mAdapterType = wgpu::AdapterType::CPU;
    MaybeError err = Initialize();
    DAWN_ASSERT(err.IsSuccess());
}

PhysicalDevice::~PhysicalDevice() = default;

bool PhysicalDevice::SupportsExternalImages() const {
    return false;
}

bool PhysicalDevice::SupportsFeatureLevel(wgpu::FeatureLevel, InstanceBase*) const {
    return true;
}

ResultOrError<PhysicalDeviceSurfaceCapabilities> PhysicalDevice::GetSurfaceCapabilities(
    InstanceBase* instance,
    const Surface* surface) const {
    PhysicalDeviceSurfaceCapabilities capabilities;
    capabilities.usages = wgpu::TextureUsage::RenderAttachment;
    capabilities.formats = {wgpu::TextureFormat::BGRA8Unorm};
    capabilities.presentModes = {wgpu::PresentMode::Fifo};
    capabilities.alphaModes = {wgpu::CompositeAlphaMode::Opaque};
    return capabilities;
}

MaybeError PhysicalDevice::InitializeImpl() {
    return {};
}

void PhysicalDevice::InitializeSupportedFeaturesImpl() {
    // Enable all features by default for the convenience of tests.
    for (uint32_t i = 0; i < kEnumCount<Feature>; i++) {
        EnableFeature(static_cast<Feature>(i));
    }
}

MaybeError PhysicalDevice::InitializeSupportedLimitsImpl(CombinedLimits* limits) {
    GetDefaultLimitsForSupportedFeatureLevel(&limits->v1);
    // Set the subgroups limit, as DeviceNull should support subgroups feature.
    limits->experimentalImmediateDataLimits.maxImmediateDataRangeByteSize =
        kMaxExternalImmediateConstantsPerPipeline * kImmediateConstantElementByteSize;
    return {};
}

void PhysicalDevice::SetupBackendAdapterToggles(dawn::platform::Platform* platform,
                                                TogglesState* adapterToggles) const {}

void PhysicalDevice::SetupBackendDeviceToggles(dawn::platform::Platform* platform,
                                               TogglesState* deviceToggles) const {}

ResultOrError<Ref<DeviceBase>> PhysicalDevice::CreateDeviceImpl(
    AdapterBase* adapter,
    const UnpackedPtr<DeviceDescriptor>& descriptor,
    const TogglesState& deviceToggles,
    Ref<DeviceBase::DeviceLostEvent>&& lostEvent) {
    return Device::Create(adapter, descriptor, deviceToggles, std::move(lostEvent));
}

void PhysicalDevice::PopulateBackendProperties(UnpackedPtr<AdapterInfo>& info) const {
    if (auto* subgroupProperties = info.Get<AdapterPropertiesSubgroups>()) {
        subgroupProperties->subgroupMinSize = 4;
        subgroupProperties->subgroupMaxSize = 128;
    }
    if (auto* memoryHeapProperties = info.Get<AdapterPropertiesMemoryHeaps>()) {
        auto* heapInfo = new MemoryHeapInfo[1];
        memoryHeapProperties->heapCount = 1;
        memoryHeapProperties->heapInfo = heapInfo;

        heapInfo[0].size = 1024 * 1024 * 1024;
        heapInfo[0].properties = wgpu::HeapProperty::DeviceLocal | wgpu::HeapProperty::HostVisible |
                                 wgpu::HeapProperty::HostCached;
    }
    if (auto* d3dProperties = info.Get<AdapterPropertiesD3D>()) {
        d3dProperties->shaderModel = 0;
    }
}

FeatureValidationResult PhysicalDevice::ValidateFeatureSupportedWithTogglesImpl(
    wgpu::FeatureName feature,
    const TogglesState& toggles) const {
    return {};
}

class Backend : public BackendConnection {
  public:
    explicit Backend(InstanceBase* instance)
        : BackendConnection(instance, wgpu::BackendType::Null) {}

    std::vector<Ref<PhysicalDeviceBase>> DiscoverPhysicalDevices(
        const UnpackedPtr<RequestAdapterOptions>& options) override {
        if (options->forceFallbackAdapter) {
            return {};
        }
        // There is always a single Null physical device because it is purely CPU based
        // and doesn't depend on the system.
        if (mPhysicalDevice == nullptr) {
            mPhysicalDevice = AcquireRef(new PhysicalDevice());
        }
        return {mPhysicalDevice};
    }

  private:
    Ref<PhysicalDevice> mPhysicalDevice;
};

BackendConnection* Connect(InstanceBase* instance) {
    return new Backend(instance);
}

struct CopyFromStagingToBufferOperation : PendingOperation {
    void Execute() override {
        destination->CopyFromStaging(staging.Get(), sourceOffset, destinationOffset, size);
    }

    Ref<BufferBase> staging;
    Ref<Buffer> destination;
    uint64_t sourceOffset;
    uint64_t destinationOffset;
    uint64_t size;
};

// Device

// static
ResultOrError<Ref<Device>> Device::Create(AdapterBase* adapter,
                                          const UnpackedPtr<DeviceDescriptor>& descriptor,
                                          const TogglesState& deviceToggles,
                                          Ref<DeviceBase::DeviceLostEvent>&& lostEvent) {
    Ref<Device> device =
        AcquireRef(new Device(adapter, descriptor, deviceToggles, std::move(lostEvent)));
    DAWN_TRY(device->Initialize(descriptor));
    return device;
}

Device::~Device() {
    Destroy();
}

MaybeError Device::Initialize(const UnpackedPtr<DeviceDescriptor>& descriptor) {
    return DeviceBase::Initialize(AcquireRef(new Queue(this, &descriptor->defaultQueue)));
}

ResultOrError<Ref<BindGroupBase>> Device::CreateBindGroupImpl(
    const BindGroupDescriptor* descriptor) {
    Ref<BindGroup> bindGroup = AcquireRef(new BindGroup(this, descriptor));
    DAWN_TRY(bindGroup->Initialize(descriptor));
    return bindGroup;
}
ResultOrError<Ref<BindGroupLayoutInternalBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor) {
    return AcquireRef(new BindGroupLayout(this, descriptor));
}
ResultOrError<Ref<BufferBase>> Device::CreateBufferImpl(
    const UnpackedPtr<BufferDescriptor>& descriptor) {
    DAWN_TRY(IncrementMemoryUsage(descriptor->size));
    return AcquireRef(new Buffer(this, descriptor));
}
ResultOrError<Ref<CommandBufferBase>> Device::CreateCommandBuffer(
    CommandEncoder* encoder,
    const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}
Ref<ComputePipelineBase> Device::CreateUninitializedComputePipelineImpl(
    const UnpackedPtr<ComputePipelineDescriptor>& descriptor) {
    return AcquireRef(new ComputePipeline(this, descriptor));
}
ResultOrError<Ref<PipelineLayoutBase>> Device::CreatePipelineLayoutImpl(
    const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) {
    return AcquireRef(new PipelineLayout(this, descriptor));
}
ResultOrError<Ref<QuerySetBase>> Device::CreateQuerySetImpl(const QuerySetDescriptor* descriptor) {
    return AcquireRef(new QuerySet(this, descriptor));
}
Ref<RenderPipelineBase> Device::CreateUninitializedRenderPipelineImpl(
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return AcquireRef(new RenderPipeline(this, descriptor));
}
ResultOrError<Ref<SamplerBase>> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
    return AcquireRef(new Sampler(this, descriptor));
}
ResultOrError<Ref<ShaderModuleBase>> Device::CreateShaderModuleImpl(
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions,
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    Ref<ShaderModule> module = AcquireRef(new ShaderModule(this, descriptor, internalExtensions));
    DAWN_TRY(module->Initialize(parseResult, compilationMessages));
    return module;
}
ResultOrError<Ref<SwapChainBase>> Device::CreateSwapChainImpl(Surface* surface,
                                                              SwapChainBase* previousSwapChain,
                                                              const SurfaceConfiguration* config) {
    return SwapChain::Create(this, surface, previousSwapChain, config);
}
ResultOrError<Ref<TextureBase>> Device::CreateTextureImpl(
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    return AcquireRef(new Texture(this, descriptor));
}
ResultOrError<Ref<TextureViewBase>> Device::CreateTextureViewImpl(
    TextureBase* texture,
    const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    return AcquireRef(new TextureView(texture, descriptor));
}

void Device::DestroyImpl() {
    DAWN_ASSERT(GetState() == State::Disconnected);
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the device is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the device.
    // - It may be called when the last ref to the device is dropped and the device
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the device since there are no other live refs.

    // Clear pending operations before checking mMemoryUsage because some operations keep a
    // reference to Buffers.
    mPendingOperations.clear();
    DAWN_ASSERT(mMemoryUsage == 0);
}

void Device::ForgetPendingOperations() {
    mPendingOperations.clear();
}

MaybeError Device::CopyFromStagingToBufferImpl(BufferBase* source,
                                               uint64_t sourceOffset,
                                               BufferBase* destination,
                                               uint64_t destinationOffset,
                                               uint64_t size) {
    if (IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
        destination->SetInitialized(true);
    }

    auto operation = std::make_unique<CopyFromStagingToBufferOperation>();
    operation->staging = source;
    operation->destination = ToBackend(destination);
    operation->sourceOffset = sourceOffset;
    operation->destinationOffset = destinationOffset;
    operation->size = size;

    AddPendingOperation(std::move(operation));

    return {};
}

MaybeError Device::CopyFromStagingToTextureImpl(const BufferBase* source,
                                                const TexelCopyBufferLayout& src,
                                                const TextureCopy& dst,
                                                const Extent3D& copySizePixels) {
    return {};
}

MaybeError Device::IncrementMemoryUsage(uint64_t bytes) {
    static_assert(kMaxMemoryUsage <= std::numeric_limits<size_t>::max());
    if (bytes > kMaxMemoryUsage || mMemoryUsage > kMaxMemoryUsage - bytes) {
        return DAWN_OUT_OF_MEMORY_ERROR("Out of memory.");
    }
    mMemoryUsage += bytes;
    return {};
}

void Device::DecrementMemoryUsage(uint64_t bytes) {
    DAWN_ASSERT(mMemoryUsage >= bytes);
    mMemoryUsage -= bytes;
}

MaybeError Device::TickImpl() {
    return SubmitPendingOperations();
}

void Device::AddPendingOperation(std::unique_ptr<PendingOperation> operation) {
    mPendingOperations.emplace_back(std::move(operation));
}

MaybeError Device::SubmitPendingOperations() {
    for (auto& operation : mPendingOperations) {
        operation->Execute();
    }
    mPendingOperations.clear();

    DAWN_TRY(GetQueue()->CheckPassedSerials());

    return {};
}

// BindGroupDataHolder

BindGroupDataHolder::BindGroupDataHolder(size_t size)
    : mBindingDataAllocation(malloc(size))  // malloc is guaranteed to return a
                                            // pointer aligned enough for the allocation
{}

BindGroupDataHolder::~BindGroupDataHolder() {
    free(mBindingDataAllocation.ExtractAsDangling());
}

// BindGroup

BindGroup::BindGroup(DeviceBase* device, const BindGroupDescriptor* descriptor)
    : BindGroupDataHolder(descriptor->layout->GetInternalBindGroupLayout()->GetBindingDataSize()),
      BindGroupBase(device, descriptor, mBindingDataAllocation) {}

MaybeError BindGroup::InitializeImpl() {
    return {};
}

// BindGroupLayout

BindGroupLayout::BindGroupLayout(DeviceBase* device, const BindGroupLayoutDescriptor* descriptor)
    : BindGroupLayoutInternalBase(device, descriptor) {}

// Buffer

Buffer::Buffer(Device* device, const UnpackedPtr<BufferDescriptor>& descriptor)
    : BufferBase(device, descriptor) {
    mBackingData = std::unique_ptr<uint8_t[]>(new uint8_t[GetSize()]);
    mAllocatedSize = GetSize();
}

bool Buffer::IsCPUWritableAtCreation() const {
    // Only return true for mappable buffers so we can test cases that need / don't need a
    // staging buffer.
    return (GetInternalUsage() & (wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite)) != 0;
}

MaybeError Buffer::MapAtCreationImpl() {
    return {};
}

void Buffer::CopyFromStaging(BufferBase* staging,
                             uint64_t sourceOffset,
                             uint64_t destinationOffset,
                             uint64_t size) {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(staging->GetMappedPointer());
    memcpy(mBackingData.get() + destinationOffset, ptr + sourceOffset, size);
}

void Buffer::DoWriteBuffer(uint64_t bufferOffset, const void* data, size_t size) {
    DAWN_ASSERT(bufferOffset + size <= GetSize());
    DAWN_ASSERT(mBackingData);
    memcpy(mBackingData.get() + bufferOffset, data, size);
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    GetDevice()->GetQueue()->IncrementLastSubmittedCommandSerial();
    return {};
}

void* Buffer::GetMappedPointer() {
    return mBackingData.get();
}

void Buffer::UnmapImpl() {}

void Buffer::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the buffer is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the buffer.
    // - It may be called when the last ref to the buffer is dropped and the buffer
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the buffer since there are no other live refs.
    BufferBase::DestroyImpl();
    ToBackend(GetDevice())->DecrementMemoryUsage(GetSize());
}

// CommandBuffer

CommandBuffer::CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor)
    : CommandBufferBase(encoder, descriptor) {}

// QuerySet

QuerySet::QuerySet(Device* device, const QuerySetDescriptor* descriptor)
    : QuerySetBase(device, descriptor) {}

// Queue

Queue::Queue(Device* device, const QueueDescriptor* descriptor) : QueueBase(device, descriptor) {}

Queue::~Queue() {}

MaybeError Queue::SubmitImpl(uint32_t, CommandBufferBase* const*) {
    Device* device = ToBackend(GetDevice());

    DAWN_TRY(device->SubmitPendingOperations());
    IncrementLastSubmittedCommandSerial();

    return {};
}

MaybeError Queue::WriteBufferImpl(BufferBase* buffer,
                                  uint64_t bufferOffset,
                                  const void* data,
                                  size_t size) {
    ToBackend(buffer)->DoWriteBuffer(bufferOffset, data, size);
    return {};
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    return GetLastSubmittedCommandSerial();
}

void Queue::ForceEventualFlushOfCommands() {}

bool Queue::HasPendingCommands() const {
    return false;
}

MaybeError Queue::SubmitPendingCommands() {
    return {};
}

ResultOrError<bool> Queue::WaitForQueueSerial(ExecutionSerial serial, Nanoseconds timeout) {
    return true;
}

MaybeError Queue::WaitForIdleForDestruction() {
    ToBackend(GetDevice())->ForgetPendingOperations();
    return {};
}

// ComputePipeline
MaybeError ComputePipeline::InitializeImpl() {
    const ProgrammableStage& computeStage = GetStage(SingleShaderStage::Compute);

    std::optional<tint::ast::transform::SubstituteOverride::Config> substituteOverrideConfig;
    if (!computeStage.metadata->overrides.empty()) {
        substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(computeStage);
    }

    // Convert the AST program to an IR module.
    auto ir =
        tint::wgsl::reader::ProgramToLoweredIR(computeStage.module->GetTintProgram()->program);
    DAWN_INVALID_IF(ir != tint::Success, "An error occurred while generating Tint IR\n%s",
                    ir.Failure().reason);

    auto singleEntryPointResult =
        tint::core::ir::transform::SingleEntryPoint(ir.Get(), computeStage.entryPoint.c_str());
    DAWN_INVALID_IF(singleEntryPointResult != tint::Success,
                    "Pipeline single entry point (IR) failed:\n%s",
                    singleEntryPointResult.Failure().reason);

    if (substituteOverrideConfig) {
        // this needs to run after SingleEntryPoint transform which removes unused
        // overrides for the current entry point.
        tint::core::ir::transform::SubstituteOverridesConfig cfg;
        cfg.map = substituteOverrideConfig->map;
        auto substituteOverridesResult =
            tint::core::ir::transform::SubstituteOverrides(ir.Get(), cfg);
        DAWN_INVALID_IF(substituteOverridesResult != tint::Success,
                        "Pipeline override substitution (IR) failed:\n%s",
                        substituteOverridesResult.Failure().reason);
    }

    auto limits = LimitsForCompilationRequest::Create(GetDevice()->GetLimits().v1);
    auto adapterSupportedLimits =
        LimitsForCompilationRequest::Create(GetDevice()->GetAdapter()->GetLimits().v1);
    auto maxSubgroupSize = GetDevice()->GetAdapter()->GetPhysicalDevice()->GetSubgroupMaxSize();

    //  Workgroup validation has to come after overrides to have been substituted.
    auto wgInfo = tint::core::ir::GetWorkgroupInfo(ir.Get());

    DAWN_INVALID_IF(wgInfo != tint::Success, "Getting workgroup info has failed (IR):\n%s",
                    wgInfo.Failure().reason);

    Extent3D _;
    DAWN_TRY_ASSIGN(_, ValidateComputeStageWorkgroupSize(
                           wgInfo.Get().x, wgInfo.Get().y, wgInfo.Get().z,
                           wgInfo.Get().storage_size, computeStage.metadata->usesSubgroupMatrix,
                           maxSubgroupSize, limits, adapterSupportedLimits));

    return {};
}

// RenderPipeline
MaybeError RenderPipeline::InitializeImpl() {
    return {};
}

// SwapChain

// static
ResultOrError<Ref<SwapChain>> SwapChain::Create(Device* device,
                                                Surface* surface,
                                                SwapChainBase* previousSwapChain,
                                                const SurfaceConfiguration* config) {
    Ref<SwapChain> swapchain = AcquireRef(new SwapChain(device, surface, config));
    DAWN_TRY(swapchain->Initialize(previousSwapChain));
    return swapchain;
}

MaybeError SwapChain::Initialize(SwapChainBase* previousSwapChain) {
    if (previousSwapChain != nullptr) {
        // TODO(crbug.com/dawn/269): figure out what should happen when surfaces are used by
        // multiple backends one after the other. It probably needs to block until the backend
        // and GPU are completely finished with the previous swapchain.
        DAWN_INVALID_IF(previousSwapChain->GetBackendType() != wgpu::BackendType::Null,
                        "null::SwapChain cannot switch between APIs");
    }

    return {};
}

SwapChain::~SwapChain() = default;

MaybeError SwapChain::PresentImpl() {
    mTexture->APIDestroy();
    mTexture = nullptr;
    return {};
}

ResultOrError<SwapChainTextureInfo> SwapChain::GetCurrentTextureImpl() {
    TextureDescriptor textureDesc = GetSwapChainBaseTextureDescriptor(this);
    mTexture = AcquireRef(new Texture(GetDevice(), Unpack(&textureDesc)));
    SwapChainTextureInfo info;
    info.texture = mTexture;
    info.status = wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal;
    return info;
}

void SwapChain::DetachFromSurfaceImpl() {
    if (mTexture != nullptr) {
        mTexture->APIDestroy();
        mTexture = nullptr;
    }
}

// ShaderModule

MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult,
                                    OwnedCompilationMessages* compilationMessages) {
    return InitializeBase(parseResult, compilationMessages);
}

uint32_t Device::GetOptimalBytesPerRowAlignment() const {
    return 1;
}

uint64_t Device::GetOptimalBufferToTextureCopyOffsetAlignment() const {
    return 1;
}

float Device::GetTimestampPeriodInNS() const {
    return 1.0f;
}

bool Device::CanTextureLoadResolveTargetInTheSameRenderpass() const {
    return true;
}

Texture::Texture(DeviceBase* device, const UnpackedPtr<TextureDescriptor>& descriptor)
    : TextureBase(device, descriptor) {}

}  // namespace dawn::native::null
