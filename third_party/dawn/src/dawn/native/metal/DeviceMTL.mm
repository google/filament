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

#include "dawn/native/metal/DeviceMTL.h"

#include "dawn/common/GPUInfo.h"
#include "dawn/common/Platform.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/BackendConnection.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Commands.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/EventManager.h"
#include "dawn/native/metal/BackendMTL.h"
#include "dawn/native/metal/BindGroupLayoutMTL.h"
#include "dawn/native/metal/BindGroupMTL.h"
#include "dawn/native/metal/BufferMTL.h"
#include "dawn/native/metal/CommandBufferMTL.h"
#include "dawn/native/metal/ComputePipelineMTL.h"
#include "dawn/native/metal/PhysicalDeviceMTL.h"
#include "dawn/native/metal/PipelineLayoutMTL.h"
#include "dawn/native/metal/QuerySetMTL.h"
#include "dawn/native/metal/QueueMTL.h"
#include "dawn/native/metal/RenderPipelineMTL.h"
#include "dawn/native/metal/SamplerMTL.h"
#include "dawn/native/metal/ShaderModuleMTL.h"
#include "dawn/native/metal/SharedFenceMTL.h"
#include "dawn/native/metal/SharedTextureMemoryMTL.h"
#include "dawn/native/metal/SwapChainMTL.h"
#include "dawn/native/metal/TextureMTL.h"
#include "dawn/native/metal/UtilsMetal.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include <type_traits>

namespace dawn::native::metal {

struct KalmanInfo {
    float filterValue;  // The estimation value
    float kalmanGain;   // The kalman gain
    float R;            // The covariance of the observation noise
    float P;            // The a posteriori estimate covariance
};

namespace {

// The time interval for each round of kalman filter
static constexpr uint64_t kFilterIntervalInMs = static_cast<uint64_t>(NSEC_PER_SEC / 10);

// A simplified kalman filter for estimating timestamp period based on measured values
float KalmanFilter(KalmanInfo* info, float measuredValue) {
    // Optimize kalman gain
    info->kalmanGain = info->P / (info->P + info->R);

    // Correct filter value
    info->filterValue =
        info->kalmanGain * measuredValue + (1.0 - info->kalmanGain) * info->filterValue;
    // Update estimate covariance
    info->P = (1.0f - info->kalmanGain) * info->P;
    return info->filterValue;
}

void UpdateTimestampPeriod(id<MTLDevice> device,
                           KalmanInfo* info,
                           MTLTimestamp* cpuTimestampStart,
                           MTLTimestamp* gpuTimestampStart,
                           float* timestampPeriod) {
    // The filter value is converged to an optimal value when the kalman gain is less than
    // 0.01. At this time, the weight of the measured value is too small to change the next
    // filter value, the sampling and calculations do not need to continue anymore.
    if (info->kalmanGain < 0.01f) {
        return;
    }

    MTLTimestamp cpuTimestampEnd = 0, gpuTimestampEnd = 0;
    [device sampleTimestamps:&cpuTimestampEnd gpuTimestamp:&gpuTimestampEnd];

    // Update the timestamp start values when timestamp reset happens
    if (cpuTimestampEnd < *cpuTimestampStart || gpuTimestampEnd < *gpuTimestampStart) {
        *cpuTimestampStart = cpuTimestampEnd;
        *gpuTimestampStart = gpuTimestampEnd;
        return;
    }

    if (cpuTimestampEnd - *cpuTimestampStart >= kFilterIntervalInMs) {
        // The measured timestamp period
        float measurement = (cpuTimestampEnd - *cpuTimestampStart) /
                            static_cast<float>(gpuTimestampEnd - *gpuTimestampStart);

        // Measurement update
        *timestampPeriod = KalmanFilter(info, measurement);

        *cpuTimestampStart = cpuTimestampEnd;
        *gpuTimestampStart = gpuTimestampEnd;
    }
}

}  // namespace

// static
ResultOrError<Ref<Device>> Device::Create(AdapterBase* adapter,
                                          NSPRef<id<MTLDevice>> mtlDevice,
                                          const UnpackedPtr<DeviceDescriptor>& descriptor,
                                          const TogglesState& deviceToggles,
                                          Ref<DeviceBase::DeviceLostEvent>&& lostEvent) {
    @autoreleasepool {
        Ref<Device> device = AcquireRef(new Device(adapter, std::move(mtlDevice), descriptor,
                                                   deviceToggles, std::move(lostEvent)));
        DAWN_TRY(device->Initialize(descriptor));
        return device;
    }
}

Device::Device(AdapterBase* adapter,
               NSPRef<id<MTLDevice>> mtlDevice,
               const UnpackedPtr<DeviceDescriptor>& descriptor,
               const TogglesState& deviceToggles,
               Ref<DeviceBase::DeviceLostEvent>&& lostEvent)
    : DeviceBase(adapter, descriptor, deviceToggles, std::move(lostEvent)),
      mMtlDevice(std::move(mtlDevice)) {
    // Counter only can be sampled between command boundary using sampleCountersInBuffer API if it's
    // supported.

    mCounterSamplingAtCommandBoundary = SupportCounterSamplingAtCommandBoundary(GetMTLDevice());
    mCounterSamplingAtStageBoundary = SupportCounterSamplingAtStageBoundary(GetMTLDevice());

    mIsTimestampQueryEnabled = HasFeature(Feature::TimestampQuery) ||
                               HasFeature(Feature::ChromiumExperimentalTimestampQueryInsidePasses);
}

Device::~Device() {
    Destroy();
}

MaybeError Device::Initialize(const UnpackedPtr<DeviceDescriptor>& descriptor) {
    Ref<Queue> queue;
    DAWN_TRY_ASSIGN(queue, Queue::Create(this, &descriptor->defaultQueue));

    if (mIsTimestampQueryEnabled && !IsToggleEnabled(Toggle::DisableTimestampQueryConversion)) {
        // Make a best guess of timestamp period based on device vendor info, and converge it to
        // an accurate value by the following calculations.
        mTimestampPeriod = gpu_info::IsIntel(GetPhysicalDevice()->GetVendorId()) ? 83.333f : 1.0f;

        if (!IsToggleEnabled(Toggle::MetalDisableTimestampPeriodEstimation)) {
            // Initialize kalman filter parameters
            mKalmanInfo = std::make_unique<KalmanInfo>();
            mKalmanInfo->filterValue = 0.0f;
            mKalmanInfo->kalmanGain = 0.5f;
            mKalmanInfo->R =
                0.0001f;  // The smaller this value is, the smaller the error of measured
                          // value is, the more we can trust the measured value.
            mKalmanInfo->P = 1.0f;

            // Sample CPU timestamp and GPU timestamp for first time at device creation
            [*mMtlDevice sampleTimestamps:&mCpuTimestamp gpuTimestamp:&mGpuTimestamp];
        }
    }

    return DeviceBase::Initialize(std::move(queue));
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
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    return ShaderModule::Create(this, descriptor, internalExtensions, parseResult,
                                compilationMessages);
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
    PhysicalDevice* physicalDevice = ToBackend(GetPhysicalDevice());
    if (physicalDevice->IsMetalValidationEnabled() &&
        gpu_info::IsAMD(physicalDevice->GetVendorId())) {
        event->InitializeSync();
        return;
    }

    event->InitializeAsync();
}
void Device::InitializeRenderPipelineAsyncImpl(Ref<CreateRenderPipelineAsyncEvent> event) {
    PhysicalDevice* physicalDevice = ToBackend(GetPhysicalDevice());
    if (physicalDevice->IsMetalValidationEnabled() &&
        gpu_info::IsAMD(physicalDevice->GetVendorId())) {
        event->InitializeSync();
        return;
    }

    event->InitializeAsync();
}

ResultOrError<Ref<SharedTextureMemoryBase>> Device::ImportSharedTextureMemoryImpl(
    const SharedTextureMemoryDescriptor* baseDescriptor) {
    UnpackedPtr<SharedTextureMemoryDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(baseDescriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(type,
                    (unpacked.ValidateBranches<Branch<SharedTextureMemoryIOSurfaceDescriptor>>()));
    DAWN_ASSERT(type == wgpu::SType::SharedTextureMemoryIOSurfaceDescriptor);
    const auto* descriptor = unpacked.Get<SharedTextureMemoryIOSurfaceDescriptor>();
    DAWN_ASSERT(descriptor != nullptr);

    DAWN_INVALID_IF(!HasFeature(Feature::SharedTextureMemoryIOSurface), "%s is not enabled.",
                    wgpu::FeatureName::SharedTextureMemoryIOSurface);

    return SharedTextureMemory::Create(this, baseDescriptor->label, descriptor);
}

ResultOrError<Ref<SharedFenceBase>> Device::ImportSharedFenceImpl(
    const SharedFenceDescriptor* baseDescriptor) {
    UnpackedPtr<SharedFenceDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(baseDescriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(type,
                    (unpacked.ValidateBranches<Branch<SharedFenceMTLSharedEventDescriptor>>()));
    DAWN_ASSERT(type == wgpu::SType::SharedFenceMTLSharedEventDescriptor);
    const auto* descriptor = unpacked.Get<SharedFenceMTLSharedEventDescriptor>();
    DAWN_ASSERT(descriptor != nullptr);

    DAWN_INVALID_IF(!HasFeature(Feature::SharedFenceMTLSharedEvent), "%s is not enabled.",
                    wgpu::FeatureName::SharedFenceMTLSharedEvent);

    return SharedFence::Create(this, baseDescriptor->label, descriptor);
}

MaybeError Device::TickImpl() {
    DAWN_TRY(ToBackend(GetQueue())->SubmitPendingCommandBuffer());

    // Just run timestamp period estimation when timestamp feature is enabled and timestamp
    // conversion is not disabled and the estimation is not disabled.
    if (mIsTimestampQueryEnabled && !IsToggleEnabled(Toggle::DisableTimestampQueryConversion) &&
        !IsToggleEnabled(Toggle::MetalDisableTimestampPeriodEstimation)) {
        UpdateTimestampPeriod(GetMTLDevice(), mKalmanInfo.get(), &mCpuTimestamp, &mGpuTimestamp,
                              &mTimestampPeriod);
    }

    return {};
}

id<MTLDevice> Device::GetMTLDevice() const {
    return mMtlDevice.Get();
}

MaybeError Device::CopyFromStagingToBufferImpl(BufferBase* source,
                                               uint64_t sourceOffset,
                                               BufferBase* destination,
                                               uint64_t destinationOffset,
                                               uint64_t size) {
    // Metal validation layers forbid  0-sized copies, assert it is skipped prior to calling
    // this function.
    DAWN_ASSERT(size != 0);

    ToBackend(destination)
        ->EnsureDataInitializedAsDestination(
            ToBackend(GetQueue())->GetPendingCommandContext(QueueBase::SubmitMode::Passive),
            destinationOffset, size);

    id<MTLBuffer> uploadBuffer = ToBackend(source)->GetMTLBuffer();
    Buffer* buffer = ToBackend(destination);
    buffer->TrackUsage();
    [ToBackend(GetQueue())->GetPendingCommandContext(QueueBase::SubmitMode::Passive)->EnsureBlit()
           copyFromBuffer:uploadBuffer
             sourceOffset:sourceOffset
                 toBuffer:buffer->GetMTLBuffer()
        destinationOffset:destinationOffset
                     size:size];
    return {};
}

// In Metal we don't write from the CPU to the texture directly which can be done using the
// replaceRegion function, because the function requires a non-private storage mode and Dawn
// sets the private storage mode by default for all textures except IOSurfaces on macOS.
MaybeError Device::CopyFromStagingToTextureImpl(const BufferBase* source,
                                                const TexelCopyBufferLayout& dataLayout,
                                                const TextureCopy& dst,
                                                const Extent3D& copySizePixels) {
    Texture* texture = ToBackend(dst.texture.Get());
    texture->SynchronizeTextureBeforeUse(ToBackend(GetQueue())->GetPendingCommandContext());
    DAWN_TRY(EnsureDestinationTextureInitialized(
        ToBackend(GetQueue())->GetPendingCommandContext(QueueBase::SubmitMode::Passive), texture,
        dst, copySizePixels));

    RecordCopyBufferToTexture(
        ToBackend(GetQueue())->GetPendingCommandContext(QueueBase::SubmitMode::Passive),
        ToBackend(source)->GetMTLBuffer(), source->GetSize(), dataLayout.offset,
        dataLayout.bytesPerRow, dataLayout.rowsPerImage, texture, dst.mipLevel, dst.origin,
        dst.aspect, copySizePixels);
    return {};
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
    mMtlDevice = nullptr;
    mMockBlitMtlBuffer = nullptr;
}

uint32_t Device::GetOptimalBytesPerRowAlignment() const {
    return 1;
}

uint64_t Device::GetOptimalBufferToTextureCopyOffsetAlignment() const {
    return 1;
}

float Device::GetTimestampPeriodInNS() const {
    return mTimestampPeriod;
}

bool Device::CanTextureLoadResolveTargetInTheSameRenderpass() const {
    return true;
}

bool Device::UseCounterSamplingAtCommandBoundary() const {
    return mCounterSamplingAtCommandBoundary;
}

bool Device::UseCounterSamplingAtStageBoundary() const {
    return mCounterSamplingAtStageBoundary;
}

bool Device::BackendWillValidateMultiDraw() const {
    return true;
}

id<MTLBuffer> Device::GetMockBlitMtlBuffer() {
    if (mMockBlitMtlBuffer == nullptr) {
        mMockBlitMtlBuffer.Acquire(
            [GetMTLDevice() newBufferWithLength:1 options:MTLResourceStorageModePrivate]);
    }

    return mMockBlitMtlBuffer.Get();
}

}  // namespace dawn::native::metal
