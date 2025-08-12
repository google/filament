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

#include "dawn/native/d3d12/DeviceD3D12.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <utility>

#include "dawn/common/GPUInfo.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/KeyedMutex.h"
#include "dawn/native/d3d12/BackendD3D12.h"
#include "dawn/native/d3d12/BindGroupD3D12.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/CommandBufferD3D12.h"
#include "dawn/native/d3d12/ComputePipelineD3D12.h"
#include "dawn/native/d3d12/PhysicalDeviceD3D12.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "dawn/native/d3d12/QuerySetD3D12.h"
#include "dawn/native/d3d12/QueueD3D12.h"
#include "dawn/native/d3d12/RenderPipelineD3D12.h"
#include "dawn/native/d3d12/ResidencyManagerD3D12.h"
#include "dawn/native/d3d12/SamplerD3D12.h"
#include "dawn/native/d3d12/SamplerHeapCacheD3D12.h"
#include "dawn/native/d3d12/ShaderModuleD3D12.h"
#include "dawn/native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
#include "dawn/native/d3d12/SharedBufferMemoryD3D12.h"
#include "dawn/native/d3d12/SharedFenceD3D12.h"
#include "dawn/native/d3d12/SharedTextureMemoryD3D12.h"
#include "dawn/native/d3d12/StagingDescriptorAllocatorD3D12.h"
#include "dawn/native/d3d12/SwapChainD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d12 {
namespace {
// TODO(dawn:155): Figure out these values.
static constexpr uint16_t kShaderVisibleDescriptorHeapSize = 1024;
static constexpr uint8_t kAttachmentDescriptorHeapSize = 64;

// Value may change in the future to better accommodate large clears.
static constexpr uint64_t kZeroBufferSize = 1024 * 1024 * 4;  // 4 Mb

static constexpr uint64_t kMaxDebugMessagesToPrint = 5;
}  // namespace

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

MaybeError Device::Initialize(const UnpackedPtr<DeviceDescriptor>& descriptor) {
    mD3d12Device = ToBackend(GetPhysicalDevice())->GetDevice();

    // Querying for the ID3D12DebugDevice interface will tell us whether the debug layer
    // is enabled. The debug layer can be enabled internally via command line flags or externally
    // via dxcpl.exe or d3dconfig.exe.
    ComPtr<ID3D12DebugDevice> d3d12DebugDevice;
    mIsDebugLayerEnabled = SUCCEEDED(mD3d12Device.As(&d3d12DebugDevice));

    DAWN_ASSERT(mD3d12Device != nullptr);

    Ref<Queue> queue;
    DAWN_TRY_ASSIGN(queue, Queue::Create(this, &descriptor->defaultQueue));

    if ((HasFeature(Feature::TimestampQuery) ||
         HasFeature(Feature::ChromiumExperimentalTimestampQueryInsidePasses)) &&
        !IsToggleEnabled(Toggle::DisableTimestampQueryConversion)) {
        // Get GPU timestamp counter frequency (in ticks/second). This fails if the specified
        // command queue doesn't support timestamps. D3D12_COMMAND_LIST_TYPE_DIRECT queues
        // always support timestamps except where there are bugs in Windows container and vGPU
        // implementations.
        uint64_t frequency;
        DAWN_TRY(CheckHRESULT(queue->GetCommandQueue()->GetTimestampFrequency(&frequency),
                              "D3D12 get timestamp frequency"));
        // Calculate the period in nanoseconds by the frequency.
        mTimestampPeriod = static_cast<float>(1e9) / frequency;
    }

    // Initialize backend services

    // Zero sized allocator is never requested and does not need to exist.
    for (uint32_t countIndex = 0; countIndex < kNumViewDescriptorAllocators; countIndex++) {
        mViewAllocators[countIndex + 1] =
            std::make_unique<MutexProtected<StagingDescriptorAllocator>>(
                this, 1u << countIndex, kShaderVisibleDescriptorHeapSize,
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    for (uint32_t countIndex = 0; countIndex < kNumSamplerDescriptorAllocators; countIndex++) {
        mSamplerAllocators[countIndex + 1] =
            std::make_unique<MutexProtected<StagingDescriptorAllocator>>(
                this, 1u << countIndex, kShaderVisibleDescriptorHeapSize,
                D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }

    mRenderTargetViewAllocator = std::make_unique<MutexProtected<StagingDescriptorAllocator>>(
        this, 1, kAttachmentDescriptorHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    mDepthStencilViewAllocator = std::make_unique<MutexProtected<StagingDescriptorAllocator>>(
        this, 1, kAttachmentDescriptorHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    mSamplerHeapCache = std::make_unique<SamplerHeapCache>(this);

    mResidencyManager = std::make_unique<MutexProtected<ResidencyManager>>(this);
    mResourceAllocatorManager = std::make_unique<MutexProtected<ResourceAllocatorManager>>(this);

    // ShaderVisibleDescriptorAllocators use the ResidencyManager and must be initialized after.
    DAWN_TRY_ASSIGN(
        mSamplerShaderVisibleDescriptorAllocator,
        ShaderVisibleDescriptorAllocator::Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER));

    DAWN_TRY_ASSIGN(
        mViewShaderVisibleDescriptorAllocator,
        ShaderVisibleDescriptorAllocator::Create(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

    // Initialize indirect commands
    D3D12_INDIRECT_ARGUMENT_DESC argumentDesc = {};
    argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

    D3D12_COMMAND_SIGNATURE_DESC programDesc = {};
    programDesc.ByteStride = 3 * sizeof(uint32_t);
    programDesc.NumArgumentDescs = 1;
    programDesc.pArgumentDescs = &argumentDesc;

    GetD3D12Device()->CreateCommandSignature(&programDesc, nullptr,
                                             IID_PPV_ARGS(&mDispatchIndirectSignature));

    argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
    programDesc.ByteStride = 4 * sizeof(uint32_t);

    GetD3D12Device()->CreateCommandSignature(&programDesc, nullptr,
                                             IID_PPV_ARGS(&mDrawIndirectSignature));

    argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
    programDesc.ByteStride = 5 * sizeof(uint32_t);

    GetD3D12Device()->CreateCommandSignature(&programDesc, nullptr,
                                             IID_PPV_ARGS(&mDrawIndexedIndirectSignature));

    DAWN_TRY(DeviceBase::Initialize(descriptor, std::move(queue)));
    DAWN_TRY(EnsureCompilerLibraries());

    // Set up shader profile for DXC.
    if (IsToggleEnabled(Toggle::UseDXC)) {
        uint32_t appliedShaderModel =
            ToBackend(GetPhysicalDevice())->GetAppliedShaderModelUnderToggles(GetTogglesState());
        uint32_t shaderModelMajor = appliedShaderModel / 10;
        uint32_t shaderModelMinor = appliedShaderModel % 10;
        // Profiles are always <stage>s_<minor>_<major> so we build the s_<minor>_major and add
        // it to each of the stage's suffix.
        std::wstring profileSuffix = L"s_M_n";
        profileSuffix[2] = wchar_t('0' + shaderModelMajor);
        profileSuffix[4] = wchar_t('0' + shaderModelMinor);
        mDxcShaderProfiles[SingleShaderStage::Vertex] = L"v" + profileSuffix;
        mDxcShaderProfiles[SingleShaderStage::Fragment] = L"p" + profileSuffix;
        mDxcShaderProfiles[SingleShaderStage::Compute] = L"c" + profileSuffix;
    }

    DAWN_TRY(CreateZeroBuffer());

    SetLabelImpl();

    return {};
}

Device::Device(AdapterBase* adapter,
               const UnpackedPtr<DeviceDescriptor>& descriptor,
               const TogglesState& deviceToggles,
               Ref<DeviceBase::DeviceLostEvent>&& lostEvent)
    : Base(adapter, descriptor, deviceToggles, std::move(lostEvent)) {}

Device::~Device() = default;

ID3D12Device* Device::GetD3D12Device() const {
    return mD3d12Device.Get();
}

ResultOrError<ComPtr<ID3D11On12Device>> Device::GetOrCreateD3D11on12Device() {
    if (mD3d11On12Device == nullptr) {
        ComPtr<ID3D11Device> d3d11Device;
        D3D_FEATURE_LEVEL d3dFeatureLevel;
        IUnknown* const iUnknownQueue = ToBackend(GetQueue())->GetCommandQueue();
        DAWN_TRY(CheckHRESULT(
            GetFunctions()->d3d11on12CreateDevice(mD3d12Device.Get(), 0, nullptr, 0, &iUnknownQueue,
                                                  1, 1, &d3d11Device, nullptr, &d3dFeatureLevel),
            "D3D11on12CreateDevice"));

        ComPtr<ID3D11On12Device> d3d11on12Device;
        d3d11Device.As(&d3d11on12Device);
        DAWN_ASSERT(d3d11on12Device);

        mD3d11On12Device = std::move(d3d11on12Device);
    }
    return mD3d11On12Device;
}

void Device::Flush11On12DeviceToAvoidLeaks() {
    DAWN_ASSERT(mD3d11On12Device);

    ComPtr<ID3D11Device> d3d11Device;
    mD3d11On12Device.As(&d3d11Device);
    DAWN_ASSERT(d3d11Device);

    ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
    d3d11Device->GetImmediateContext(&d3d11DeviceContext);
    DAWN_ASSERT(d3d11DeviceContext);

    // 11on12 has a bug where D3D12 resources used only for keyed shared mutexes are not released
    // until work is submitted to the device context and flushed. The most minimal work we can get
    // away with is issuing a TiledResourceBarrier.
    ComPtr<ID3D11DeviceContext2> d3d11DeviceContext2;
    d3d11DeviceContext.As(&d3d11DeviceContext2);
    DAWN_ASSERT(d3d11DeviceContext2);

    d3d11DeviceContext2->TiledResourceBarrier(nullptr, nullptr);
    d3d11DeviceContext2->Flush();
}

ComPtr<ID3D12CommandSignature> Device::GetDispatchIndirectSignature() const {
    return mDispatchIndirectSignature;
}

ComPtr<ID3D12CommandSignature> Device::GetDrawIndirectSignature() const {
    return mDrawIndirectSignature;
}

ComPtr<ID3D12CommandSignature> Device::GetDrawIndexedIndirectSignature() const {
    return mDrawIndexedIndirectSignature;
}

// Ensure DXC if use_dxc toggles are set and validated.
MaybeError Device::EnsureCompilerLibraries() {
    if (IsToggleEnabled(Toggle::UseDXC)) {
        DAWN_TRY(ToBackend(GetPhysicalDevice())->GetBackend()->EnsureDXC());
    } else {
        DAWN_TRY(ToBackend(GetPhysicalDevice())->GetBackend()->EnsureFXC());
    }

    return {};
}

const PlatformFunctions* Device::GetFunctions() const {
    return ToBackend(GetPhysicalDevice())->GetBackend()->GetFunctions();
}

MutexProtected<ResidencyManager>& Device::GetResidencyManager() const {
    return *mResidencyManager;
}

MaybeError Device::CreateZeroBuffer() {
    BufferDescriptor zeroBufferDescriptor;
    zeroBufferDescriptor.size = kZeroBufferSize;
    zeroBufferDescriptor.label = "ZeroBuffer_Internal";

    Ref<BufferBase> zeroBufferBase;

    // Clear zero buffer from CPU when `Feature::BufferMapExtendedUsages` is supported.
    if (ToBackend(GetPhysicalDevice())->SupportsBufferMapExtendedUsages()) {
        zeroBufferDescriptor.mappedAtCreation = true;
        zeroBufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;

        DAWN_TRY_ASSIGN(zeroBufferBase, CreateBuffer(&zeroBufferDescriptor));

        void* mappedPointer = zeroBufferBase->GetMappedPointer();
        DAWN_ASSERT(mappedPointer != nullptr);
        memset(mappedPointer, 0, zeroBufferBase->GetAllocatedSize());
        DAWN_TRY(zeroBufferBase->Unmap());
    } else {
        zeroBufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;

        DAWN_TRY_ASSIGN(zeroBufferBase, CreateBuffer(&zeroBufferDescriptor));

        CommandRecordingContext* commandContext =
            ToBackend(GetQueue())->GetPendingCommandContext(QueueBase::SubmitMode::Passive);

        DAWN_TRY(GetDynamicUploader()->WithUploadReservation(
            kZeroBufferSize, kCopyBufferToBufferOffsetAlignment,
            [&](UploadReservation reservation) -> MaybeError {
                memset(reservation.mappedPointer, 0u, kZeroBufferSize);

                CopyFromStagingToBufferHelper(commandContext, reservation.buffer.Get(),
                                              reservation.offsetInBuffer, zeroBufferBase.Get(), 0,
                                              kZeroBufferSize);

                zeroBufferBase->SetInitialized(true);
                return {};
            }));
    }

    mZeroBuffer = ToBackend(zeroBufferBase);
    return {};
}

MaybeError Device::ClearBufferToZero(CommandRecordingContext* commandContext,
                                     BufferBase* destination,
                                     uint64_t offset,
                                     uint64_t size) {
    Buffer* dstBuffer = ToBackend(destination);

    // Necessary to ensure residency of the zero buffer.
    mZeroBuffer->TrackUsageAndTransitionNow(commandContext, wgpu::BufferUsage::CopySrc);
    dstBuffer->TrackUsageAndTransitionNow(commandContext, wgpu::BufferUsage::CopyDst);

    while (size > 0) {
        uint64_t copySize = std::min(kZeroBufferSize, size);
        commandContext->GetCommandList()->CopyBufferRegion(
            dstBuffer->GetD3D12Resource(), offset, mZeroBuffer->GetD3D12Resource(), 0, copySize);

        offset += copySize;
        size -= copySize;
    }

    return {};
}

MaybeError Device::TickImpl() {
    // Perform cleanup operations to free unused objects
    ExecutionSerial completedSerial = GetQueue()->GetCompletedCommandSerial();

    (*mResourceAllocatorManager)->Tick(completedSerial);
    (*mViewShaderVisibleDescriptorAllocator)->Tick(completedSerial);
    (*mSamplerShaderVisibleDescriptorAllocator)->Tick(completedSerial);
    (*mRenderTargetViewAllocator)->Tick(completedSerial);
    (*mDepthStencilViewAllocator)->Tick(completedSerial);
    mUsedComObjectRefs->ClearUpTo(completedSerial);

    DAWN_TRY(ToBackend(GetQueue())->SubmitPendingCommands());

    DAWN_TRY(CheckDebugLayerAndGenerateErrors());

    return {};
}

void Device::ReferenceUntilUnused(ComPtr<IUnknown> object) {
    mUsedComObjectRefs->Enqueue(std::move(object), GetQueue()->GetPendingCommandSerial());
}

ResultOrError<Ref<BindGroupBase>> Device::CreateBindGroupImpl(
    const BindGroupDescriptor* descriptor) {
    return BindGroup::Create(this, descriptor);
}
ResultOrError<Ref<BindGroupLayoutInternalBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor) {
    return BindGroupLayout::Create(this, descriptor);
}
ResultOrError<Ref<BufferBase>> Device::CreateBufferImpl(
    const UnpackedPtr<BufferDescriptor>& descriptor) {
    return Buffer::Create(this, descriptor);
}
ResultOrError<Ref<CommandBufferBase>> Device::CreateCommandBuffer(
    CommandEncoder* encoder,
    const CommandBufferDescriptor* descriptor) {
    return CommandBuffer::Create(encoder, descriptor);
}
Ref<ComputePipelineBase> Device::CreateUninitializedComputePipelineImpl(
    const UnpackedPtr<ComputePipelineDescriptor>& descriptor) {
    return ComputePipeline::CreateUninitialized(this, descriptor);
}
ResultOrError<Ref<PipelineLayoutBase>> Device::CreatePipelineLayoutImpl(
    const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) {
    return PipelineLayout::Create(this, descriptor);
}
ResultOrError<Ref<QuerySetBase>> Device::CreateQuerySetImpl(const QuerySetDescriptor* descriptor) {
    return QuerySet::Create(this, descriptor);
}
Ref<RenderPipelineBase> Device::CreateUninitializedRenderPipelineImpl(
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return RenderPipeline::CreateUninitialized(this, descriptor);
}
ResultOrError<Ref<SamplerBase>> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
    return Sampler::Create(this, descriptor);
}
ResultOrError<Ref<ShaderModuleBase>> Device::CreateShaderModuleImpl(
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions,
    ShaderModuleParseResult* parseResult) {
    return ShaderModule::Create(this, descriptor, internalExtensions, parseResult);
}
ResultOrError<Ref<SwapChainBase>> Device::CreateSwapChainImpl(Surface* surface,
                                                              SwapChainBase* previousSwapChain,
                                                              const SurfaceConfiguration* config) {
    return SwapChain::Create(this, surface, previousSwapChain, config);
}
ResultOrError<Ref<TextureBase>> Device::CreateTextureImpl(
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    return Texture::Create(this, descriptor);
}
ResultOrError<Ref<TextureViewBase>> Device::CreateTextureViewImpl(
    TextureBase* texture,
    const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    return TextureView::Create(texture, descriptor);
}
void Device::InitializeComputePipelineAsyncImpl(Ref<CreateComputePipelineAsyncEvent> event) {
    event->InitializeAsync();
}
void Device::InitializeRenderPipelineAsyncImpl(Ref<CreateRenderPipelineAsyncEvent> event) {
    event->InitializeAsync();
}

ResultOrError<Ref<SharedBufferMemoryBase>> Device::ImportSharedBufferMemoryImpl(
    const SharedBufferMemoryDescriptor* descriptor) {
    UnpackedPtr<SharedBufferMemoryDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(
        type, (unpacked.ValidateBranches<Branch<SharedBufferMemoryD3D12ResourceDescriptor>>()));

    switch (type) {
        case wgpu::SType::SharedBufferMemoryD3D12ResourceDescriptor:
            DAWN_INVALID_IF(!HasFeature(Feature::SharedBufferMemoryD3D12Resource),
                            "%s is not enabled.",
                            wgpu::FeatureName::SharedBufferMemoryD3D12Resource);
            return SharedBufferMemory::Create(
                this, descriptor->label, unpacked.Get<SharedBufferMemoryD3D12ResourceDescriptor>());
        default:
            DAWN_UNREACHABLE();
    }
}

ResultOrError<Ref<SharedTextureMemoryBase>> Device::ImportSharedTextureMemoryImpl(
    const SharedTextureMemoryDescriptor* descriptor) {
    UnpackedPtr<SharedTextureMemoryDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(
        type, (unpacked.ValidateBranches<Branch<SharedTextureMemoryDXGISharedHandleDescriptor>>()));

    switch (type) {
        case wgpu::SType::SharedTextureMemoryDXGISharedHandleDescriptor:
            DAWN_INVALID_IF(!HasFeature(Feature::SharedTextureMemoryDXGISharedHandle),
                            "%s is not enabled.",
                            wgpu::FeatureName::SharedTextureMemoryDXGISharedHandle);
            return SharedTextureMemory::Create(
                this, descriptor->label,
                unpacked.Get<SharedTextureMemoryDXGISharedHandleDescriptor>());
        default:
            DAWN_UNREACHABLE();
    }
}

ResultOrError<Ref<SharedFenceBase>> Device::ImportSharedFenceImpl(
    const SharedFenceDescriptor* descriptor) {
    UnpackedPtr<SharedFenceDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(type,
                    (unpacked.ValidateBranches<Branch<SharedFenceDXGISharedHandleDescriptor>>()));

    switch (type) {
        case wgpu::SType::SharedFenceDXGISharedHandleDescriptor:
            DAWN_INVALID_IF(!HasFeature(Feature::SharedFenceDXGISharedHandle), "%s is not enabled.",
                            wgpu::FeatureName::SharedFenceDXGISharedHandle);
            return SharedFence::Create(this, descriptor->label,
                                       unpacked.Get<SharedFenceDXGISharedHandleDescriptor>());
        default:
            DAWN_UNREACHABLE();
    }
}

MaybeError Device::CopyFromStagingToBuffer(BufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) {
    CommandRecordingContext* commandRecordingContext =
        ToBackend(GetQueue())->GetPendingCommandContext(QueueBase::SubmitMode::Passive);

    Buffer* dstBuffer = ToBackend(destination);
    DAWN_TRY(dstBuffer->SynchronizeBufferBeforeUseOnGPU());

    [[maybe_unused]] bool cleared;
    DAWN_TRY_ASSIGN(cleared, dstBuffer->EnsureDataInitializedAsDestination(
                                 commandRecordingContext, destinationOffset, size));

    CopyFromStagingToBufferHelper(commandRecordingContext, source, sourceOffset, destination,
                                  destinationOffset, size);

    return {};
}

void Device::CopyFromStagingToBufferHelper(CommandRecordingContext* commandContext,
                                           BufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) {
    DAWN_ASSERT(commandContext != nullptr);
    Buffer* dstBuffer = ToBackend(destination);
    Buffer* srcBuffer = ToBackend(source);
    dstBuffer->TrackUsageAndTransitionNow(commandContext, wgpu::BufferUsage::CopyDst);

    commandContext->GetCommandList()->CopyBufferRegion(
        dstBuffer->GetD3D12Resource(), destinationOffset, srcBuffer->GetD3D12Resource(),
        sourceOffset, size);
}

MaybeError Device::CopyFromStagingToTextureImpl(const BufferBase* source,
                                                const TexelCopyBufferLayout& src,
                                                const TextureCopy& dst,
                                                const Extent3D& copySizePixels) {
    CommandRecordingContext* commandContext =
        ToBackend(GetQueue())->GetPendingCommandContext(QueueBase::SubmitMode::Passive);

    Texture* texture = ToBackend(dst.texture.Get());
    DAWN_TRY(texture->SynchronizeTextureBeforeUse(commandContext));

    SubresourceRange range = GetSubresourcesAffectedByCopy(dst, copySizePixels);
    if (IsCompleteSubresourceCopiedTo(texture, copySizePixels, dst.mipLevel, dst.aspect)) {
        texture->SetIsSubresourceContentInitialized(true, range);
    } else {
        DAWN_TRY(texture->EnsureSubresourceContentInitialized(commandContext, range));
    }

    texture->TrackUsageAndTransitionNow(commandContext, wgpu::TextureUsage::CopyDst, range);

    RecordBufferTextureCopyWithBufferHandle(BufferTextureCopyDirection::B2T,
                                            commandContext->GetCommandList(),
                                            ToBackend(source)->GetD3D12Resource(), src.offset,
                                            src.bytesPerRow, src.rowsPerImage, dst, copySizePixels);
    return {};
}

uint32_t Device::GetResourceHeapTier() const {
    return IsToggleEnabled(Toggle::UseD3D12ResourceHeapTier2) ? 2u : 1u;
}

void Device::DeallocateMemory(ResourceHeapAllocation& allocation) {
    (*mResourceAllocatorManager)->DeallocateMemory(allocation);
}

ResultOrError<ResourceHeapAllocation> Device::AllocateMemory(
    ResourceHeapKind resourceHeapKind,
    const D3D12_RESOURCE_DESC& resourceDescriptor,
    D3D12_RESOURCE_STATES initialUsage,
    uint32_t formatBytesPerBlock,
    bool forceAllocateAsCommittedResource) {
    // formatBytesPerBlock is needed only for color non-compressed formats for a workaround.
    return (*mResourceAllocatorManager)
        ->AllocateMemory(resourceHeapKind, resourceDescriptor, initialUsage, formatBytesPerBlock,
                         forceAllocateAsCommittedResource);
}

MaybeError Device::ImportSharedHandleResource(HANDLE handle,
                                              bool useKeyedMutex,
                                              ComPtr<ID3D12Resource>& d3d12Resource,
                                              Ref<d3d::KeyedMutex>& keyedMutex) {
    DAWN_TRY(CheckHRESULT(GetD3D12Device()->OpenSharedHandle(handle, IID_PPV_ARGS(&d3d12Resource)),
                          "D3D12 opening shared handle"));

    if (useKeyedMutex) {
        ComPtr<ID3D11On12Device> d3d11on12Device;
        DAWN_TRY_ASSIGN(d3d11on12Device, GetOrCreateD3D11on12Device());

        // Since D3D12 does not directly support keyed mutexes, we need to wrap the D3D12 resource
        // using 11on12 and QueryInterface the D3D11 representation for the keyed mutex.
        ComPtr<ID3D11Texture2D> d3d11Texture;
        D3D11_RESOURCE_FLAGS resourceFlags;
        resourceFlags.BindFlags = 0;
        resourceFlags.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
        resourceFlags.CPUAccessFlags = 0;
        resourceFlags.StructureByteStride = 0;
        DAWN_TRY(CheckHRESULT(d3d11on12Device->CreateWrappedResource(
                                  d3d12Resource.Get(), &resourceFlags, D3D12_RESOURCE_STATE_COMMON,
                                  D3D12_RESOURCE_STATE_COMMON, IID_PPV_ARGS(&d3d11Texture)),
                              "Failed to create wrapped D3D11on12 resource"));

        ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex;
        d3d11Texture.As(&dxgiKeyedMutex);
        DAWN_INVALID_IF(!dxgiKeyedMutex, "Failed to retrieve DXGI keyed mutex when expected");

        keyedMutex = AcquireRef(new d3d::KeyedMutex(this, std::move(dxgiKeyedMutex)));
    }

    return {};
}

void Device::DisposeKeyedMutex(ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex) {
    ComPtr<ID3D11Resource> d3d11Resource;
    dxgiKeyedMutex.As(&d3d11Resource);
    DAWN_ASSERT(d3d11Resource);

    ID3D11Resource* d3d11ResourcePtr = d3d11Resource.Get();
    mD3d11On12Device->ReleaseWrappedResources(&d3d11ResourcePtr, 1);

    // Release the resource and keyed mutex before calling Flush11on12DeviceToAvoidLeaks below.
    dxgiKeyedMutex.Reset();
    d3d11Resource.Reset();

    Flush11On12DeviceToAvoidLeaks();
}

const D3D12DeviceInfo& Device::GetDeviceInfo() const {
    return ToBackend(GetPhysicalDevice())->GetDeviceInfo();
}

void AppendDebugLayerMessagesToError(ID3D12InfoQueue* infoQueue,
                                     uint64_t totalErrors,
                                     ErrorData* error) {
    DAWN_ASSERT(totalErrors > 0);
    DAWN_ASSERT(error != nullptr);

    uint64_t errorsToPrint = std::min(kMaxDebugMessagesToPrint, totalErrors);
    for (uint64_t i = 0; i < errorsToPrint; ++i) {
        std::ostringstream messageStream;
        SIZE_T messageLength = 0;
        HRESULT hr = infoQueue->GetMessage(i, nullptr, &messageLength);
        if (FAILED(hr)) {
            messageStream << " ID3D12InfoQueue::GetMessage failed with " << hr;
            error->AppendBackendMessage(messageStream.str());
            continue;
        }

        std::unique_ptr<uint8_t[]> messageData(new uint8_t[messageLength]);
        D3D12_MESSAGE* message = reinterpret_cast<D3D12_MESSAGE*>(messageData.get());
        hr = infoQueue->GetMessage(i, message, &messageLength);
        if (FAILED(hr)) {
            messageStream << " ID3D12InfoQueue::GetMessage failed with " << hr;
            error->AppendBackendMessage(messageStream.str());
            continue;
        }

        messageStream << message->pDescription << " (" << message->ID << ")";
        error->AppendBackendMessage(messageStream.str());
    }
    if (errorsToPrint < totalErrors) {
        std::ostringstream messages;
        messages << (totalErrors - errorsToPrint) << " messages silenced";
        error->AppendBackendMessage(messages.str());
    }

    // We only print up to the first kMaxDebugMessagesToPrint errors
    infoQueue->ClearStoredMessages();
}

MaybeError Device::CheckDebugLayerAndGenerateErrors() {
    if (!mIsDebugLayerEnabled) {
        return {};
    }

    ComPtr<ID3D12InfoQueue> infoQueue;
    DAWN_TRY(CheckHRESULT(mD3d12Device.As(&infoQueue),
                          "D3D12 QueryInterface ID3D12Device to ID3D12InfoQueue"));
    uint64_t totalErrors = infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();

    // Check if any errors have occurred otherwise we would be creating an empty error. Note
    // that we use GetNumStoredMessagesAllowedByRetrievalFilter instead of GetNumStoredMessages
    // because we only convert WARNINGS or higher messages to dawn errors.
    if (totalErrors == 0) {
        return {};
    }

    auto error = DAWN_INTERNAL_ERROR("The D3D12 debug layer reported uncaught errors.");

    AppendDebugLayerMessagesToError(infoQueue.Get(), totalErrors, error.get());

    return error;
}

void Device::AppendDebugLayerMessages(ErrorData* error) {
    if (!GetAdapter()->GetInstance()->IsBackendValidationEnabled()) {
        return;
    }

    ComPtr<ID3D12InfoQueue> infoQueue;
    if (FAILED(mD3d12Device.As(&infoQueue))) {
        return;
    }
    uint64_t totalErrors = infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();

    if (totalErrors == 0) {
        return;
    }

    AppendDebugLayerMessagesToError(infoQueue.Get(), totalErrors, error);
}

void Device::AppendDeviceLostMessage(ErrorData* error) {
    if (mD3d12Device) {
        HRESULT result = mD3d12Device->GetDeviceRemovedReason();
        error->AppendBackendMessage("Device removed reason: %s (0x%08X)",
                                    d3d::HRESULTAsString(result), result);
        RecordDeviceRemovedReason(result);
    }
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

    Base::DestroyImpl();

    mZeroBuffer = nullptr;

    // Release recycled resource heaps and all other objects waiting for deletion in the resource
    // allocation manager.
    mResourceAllocatorManager.reset();

    // We need to handle clearing up com object refs that were enqeued after TickImpl
    mUsedComObjectRefs->ClearUpTo(std::numeric_limits<ExecutionSerial>::max());

    DAWN_ASSERT(mUsedComObjectRefs->Empty());
}

MutexProtected<ShaderVisibleDescriptorAllocator>& Device::GetViewShaderVisibleDescriptorAllocator()
    const {
    return *mViewShaderVisibleDescriptorAllocator.get();
}

MutexProtected<ShaderVisibleDescriptorAllocator>&
Device::GetSamplerShaderVisibleDescriptorAllocator() const {
    return *mSamplerShaderVisibleDescriptorAllocator.get();
}

MutexProtected<StagingDescriptorAllocator>* Device::GetViewStagingDescriptorAllocator(
    uint32_t descriptorCount) const {
    DAWN_ASSERT(descriptorCount <= kMaxViewDescriptorsPerBindGroup);
    DAWN_ASSERT(descriptorCount > 0);
    // This is Log2 of the next power of two, plus 1.
    uint32_t allocatorIndex = Log2Ceil(descriptorCount) + 1;
    return mViewAllocators[allocatorIndex].get();
}

MutexProtected<StagingDescriptorAllocator>* Device::GetSamplerStagingDescriptorAllocator(
    uint32_t descriptorCount) const {
    DAWN_ASSERT(descriptorCount <= kMaxSamplerDescriptorsPerBindGroup);
    DAWN_ASSERT(descriptorCount > 0);
    // This is Log2 of the next power of two, plus 1.
    uint32_t allocatorIndex = Log2Ceil(descriptorCount) + 1;
    return mSamplerAllocators[allocatorIndex].get();
}

MutexProtected<StagingDescriptorAllocator>& Device::GetRenderTargetViewAllocator() const {
    return *mRenderTargetViewAllocator.get();
}

MutexProtected<StagingDescriptorAllocator>& Device::GetDepthStencilViewAllocator() const {
    return *mDepthStencilViewAllocator.get();
}

SamplerHeapCache* Device::GetSamplerHeapCache() {
    return mSamplerHeapCache.get();
}

uint32_t Device::GetOptimalBytesPerRowAlignment() const {
    return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}

// TODO(dawn:512): Once we optimize DynamicUploader allocation with offsets we
// should make this return D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT = 512.
// Current implementations would try to allocate additional 511 bytes,
// so we return 1 and let ComputeTextureCopySplits take care of the alignment.
uint64_t Device::GetOptimalBufferToTextureCopyOffsetAlignment() const {
    return 1;
}

bool Device::CanTextureLoadResolveTargetInTheSameRenderpass() const {
    return true;
}

bool Device::CanResolveSubRect() const {
    return true;
}

float Device::GetTimestampPeriodInNS() const {
    return mTimestampPeriod;
}

bool Device::ShouldDuplicateNumWorkgroupsForDispatchIndirect(
    ComputePipelineBase* computePipeline) const {
    return ToBackend(computePipeline)->UsesNumWorkgroups();
}

void Device::SetLabelImpl() {
    SetDebugName(this, mD3d12Device.Get(), "Dawn_Device", GetLabel());
}

bool Device::MayRequireDuplicationOfIndirectParameters() const {
    return true;
}

bool Device::ShouldDuplicateParametersForDrawIndirect(
    const RenderPipelineBase* renderPipelineBase) const {
    return ToBackend(renderPipelineBase)->UsesVertexOrInstanceIndex();
}

uint64_t Device::GetBufferCopyOffsetAlignmentForDepthStencil() const {
    // On the D3D12 platforms where programmable MSAA is not supported, the source box specifying a
    // portion of the depth texture must all be 0, or an error and a device lost will occur, so on
    // these platforms the buffer copy offset must be a multiple of 512 when the texture is created
    // with D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL. See https://crbug.com/dawn/727 for more
    // details.
    if (IsToggleEnabled(
            Toggle::D3D12UseTempBufferInDepthStencilTextureAndBufferCopyWithNonZeroBufferOffset)) {
        return D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
    }
    return DeviceBase::GetBufferCopyOffsetAlignmentForDepthStencil();
}

ComPtr<IDxcLibrary> Device::GetDxcLibrary() const {
    return ToBackend(GetPhysicalDevice())->GetBackend()->GetDxcLibrary();
}

ComPtr<IDxcCompiler3> Device::GetDxcCompiler() const {
    return ToBackend(GetPhysicalDevice())->GetBackend()->GetDxcCompiler();
}

const PerStage<std::wstring>& Device::GetDxcShaderProfiles() const {
    return mDxcShaderProfiles;
}

}  // namespace dawn::native::d3d12
