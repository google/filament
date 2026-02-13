// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/d3d11/BufferD3D11.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "dawn/common/Alloc.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/PhysicalDeviceD3D11.h"
#include "dawn/native/d3d11/QueueD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d11 {

class ScopedCommandRecordingContext;

namespace {

// Max size for a CPU buffer.
constexpr uint64_t kMaxCPUUploadBufferSize = 64 * 1024;

constexpr wgpu::BufferUsage kCopyUsages =
    wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | kInternalCopySrcBuffer;

constexpr wgpu::BufferUsage kStagingUsages = kMappableBufferUsages | kCopyUsages;

constexpr wgpu::BufferUsage kD3D11GPUWriteUsages =
    wgpu::BufferUsage::Storage | kInternalStorageBuffer | wgpu::BufferUsage::Indirect;

// Resource usage    Default    Dynamic   Immutable   Staging
// ------------------------------------------------------------
//  GPU-read         Yes        Yes       Yes         Yes[1]
//  GPU-write        Yes        No        No          Yes[1]
//  CPU-read         No         No        No          Yes[1]
//  CPU-write        No         Yes       No          Yes[1]
// ------------------------------------------------------------
// [1] GPU read or write of a resource with the D3D11_USAGE_STAGING usage is restricted to copy
// operations. You use ID3D11DeviceContext::CopySubresourceRegion and
// ID3D11DeviceContext::CopyResource for these copy operations.

bool IsMappable(wgpu::BufferUsage usage) {
    return usage & kMappableBufferUsages;
}

bool IsUpload(wgpu::BufferUsage usage) {
    return usage & wgpu::BufferUsage::MapWrite &&
           IsSubset(usage, kInternalCopySrcBuffer | wgpu::BufferUsage::CopySrc |
                               wgpu::BufferUsage::MapWrite);
}

bool IsStaging(wgpu::BufferUsage usage) {
    // Must have at least MapWrite or MapRead bit
    return IsMappable(usage) && IsSubset(usage, kStagingUsages);
}

UINT D3D11BufferBindFlags(wgpu::BufferUsage usage) {
    UINT bindFlags = 0;

    if (usage & (wgpu::BufferUsage::Vertex)) {
        bindFlags |= D3D11_BIND_VERTEX_BUFFER;
    }
    if (usage & wgpu::BufferUsage::Index) {
        bindFlags |= D3D11_BIND_INDEX_BUFFER;
    }
    if (usage & (wgpu::BufferUsage::Uniform)) {
        bindFlags |= D3D11_BIND_CONSTANT_BUFFER;
    }
    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer)) {
        DAWN_ASSERT(!IsMappable(usage));
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }
    if (usage & kReadOnlyStorageBuffer) {
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }

    // If the buffer only has CopySrc and CopyDst usages are used as staging buffers for copy.
    // Because D3D11 doesn't allow copying between buffer and texture, we will use compute shader
    // to copy data between buffer and texture. So the buffer needs to be bound as unordered access
    // view.
    if (IsSubset(usage, kCopyUsages)) {
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }

    return bindFlags;
}

UINT D3D11BufferMiscFlags(wgpu::BufferUsage usage) {
    UINT miscFlags = 0;
    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer | kReadOnlyStorageBuffer)) {
        miscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    }
    if (usage & wgpu::BufferUsage::Indirect) {
        miscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
    }
    return miscFlags;
}

size_t D3D11BufferSizeAlignment(wgpu::BufferUsage usage) {
    if (usage & wgpu::BufferUsage::Uniform) {
        // https://learn.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11devicecontext1-vssetconstantbuffers1
        // Each number of constants must be a multiple of 16 shader constants(sizeof(float) * 4 *
        // 16).
        return sizeof(float) * 4 * 16;
    }

    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer | kReadOnlyStorageBuffer |
                 wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc)) {
        // Unordered access buffers must be 4-byte aligned.
        // We also align 4 bytes for CopyDst buffer since it would be used in T2B compute shader.
        // And that shader needs to write 4-byte chunks.
        // Similarly, we need to align 4 bytes for CopySrc buffer since it would be used in B2T
        // shader that reads 4 byte chunks.
        return sizeof(uint32_t);
    }
    return 1;
}

bool CanUseCPUUploadBuffer(const Device* device, wgpu::BufferUsage usage, size_t bufferSize) {
    return IsUpload(usage) && bufferSize <= kMaxCPUUploadBufferSize &&
           !device->IsToggleEnabled(Toggle::D3D11DisableCPUUploadBuffers);
}

constexpr size_t kConstantBufferUpdateAlignment = 16;

}  // namespace

// For CPU-to-GPU upload buffers(CopySrc|MapWrite), they can be emulated in the system memory, and
// then written into the dest GPU buffer via ID3D11DeviceContext::UpdateSubresource.
class UploadBuffer final : public Buffer {
  public:
    UploadBuffer(DeviceBase* device, const UnpackedPtr<BufferDescriptor>& descriptor)
        : Buffer(device,
                 descriptor,
                 /*internalMappableFlags=*/kMappableBufferUsages) {}
    ~UploadBuffer() override = default;

  private:
    MaybeError InitializeInternal() override {
        mUploadData = std::unique_ptr<uint8_t[]>(AllocNoThrow<uint8_t>(GetAllocatedSize()));
        if (mUploadData == nullptr) {
            return DAWN_OUT_OF_MEMORY_ERROR("Failed to allocate memory for buffer uploading.");
        }
        return {};
    }

    MaybeError MapInternal(const ScopedCommandRecordingContext* commandContext,
                           wgpu::MapMode) override {
        mMappedData = mUploadData.get();
        return {};
    }

    void UnmapInternal(const ScopedCommandRecordingContext* commandContext) override {
        mMappedData = nullptr;
    }

    MaybeError ClearInternal(const ScopedCommandRecordingContext* commandContext,
                             uint8_t clearValue,
                             uint64_t offset,
                             uint64_t size) override {
        memset(mUploadData.get() + offset, clearValue, size);
        return {};
    }

    MaybeError CopyToInternal(const ScopedCommandRecordingContext* commandContext,
                              uint64_t sourceOffset,
                              size_t size,
                              Buffer* destination,
                              uint64_t destinationOffset) override {
        return destination->WriteInternal(commandContext, destinationOffset,
                                          mUploadData.get() + sourceOffset, size);
    }

    MaybeError CopyFromD3DInternal(const ScopedCommandRecordingContext* commandContext,
                                   ID3D11Buffer* srcD3D11Buffer,
                                   uint64_t sourceOffset,
                                   size_t size,
                                   uint64_t destinationOffset) override {
        // Upload buffers shouldn't be copied to.
        DAWN_UNREACHABLE();
        return {};
    }

    MaybeError WriteInternal(const ScopedCommandRecordingContext* commandContext,
                             uint64_t offset,
                             const void* data,
                             size_t size) override {
        const auto* src = static_cast<const uint8_t*>(data);
        std::copy(src, src + size, mUploadData.get() + offset);
        return {};
    }

    std::unique_ptr<uint8_t[]> mUploadData;
};

// Buffer that supports mapping and copying.
class StagingBuffer final : public Buffer {
  public:
    StagingBuffer(DeviceBase* device, const UnpackedPtr<BufferDescriptor>& descriptor)
        : Buffer(device, descriptor, /*internalMappableFlags=*/kMappableBufferUsages) {}

  private:
    void DestroyImpl() override {
        // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
        // - It may be called if the buffer is explicitly destroyed with APIDestroy.
        //   This case is NOT thread-safe and needs proper synchronization with other
        //   simultaneous uses of the buffer.
        // - It may be called when the last ref to the buffer is dropped and the buffer
        //   is implicitly destroyed. This case is thread-safe because there are no
        //   other threads using the buffer since there are no other live refs.
        Buffer::DestroyImpl();

        mD3d11Buffer = nullptr;
    }

    void SetLabelImpl() override {
        SetDebugName(ToBackend(GetDevice()), mD3d11Buffer.Get(), "Dawn_StagingBuffer", GetLabel());
    }

    MaybeError InitializeInternal() override {
        DAWN_ASSERT(IsStaging(GetInternalUsage()));

        D3D11_BUFFER_DESC bufferDescriptor;
        bufferDescriptor.ByteWidth = mAllocatedSize;
        bufferDescriptor.Usage = D3D11_USAGE_STAGING;
        bufferDescriptor.BindFlags = 0;
        // D3D11 doesn't allow copying between buffer and texture.
        //  - For buffer to texture copy, we need to use a staging(mappable) texture, and memcpy the
        //    data from the staging buffer to the staging texture first. So D3D11_CPU_ACCESS_READ is
        //    needed for MapWrite usage.
        //  - For texture to buffer copy, we may need copy texture to a staging (mappable)
        //    texture, and then memcpy the data from the staging texture to the staging buffer. So
        //    D3D11_CPU_ACCESS_WRITE is needed to MapRead usage.
        bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        bufferDescriptor.MiscFlags = 0;
        bufferDescriptor.StructureByteStride = 0;

        DAWN_TRY(
            CheckOutOfMemoryHRESULT(ToBackend(GetDevice())
                                        ->GetD3D11Device()
                                        ->CreateBuffer(&bufferDescriptor, nullptr, &mD3d11Buffer),
                                    "ID3D11Device::CreateBuffer"));

        return {};
    }

    MaybeError MapInternal(const ScopedCommandRecordingContext* commandContext,
                           wgpu::MapMode) override {
        DAWN_ASSERT(IsMappable(GetInternalUsage()));
        DAWN_ASSERT(!mMappedData);

        // Always map buffer with D3D11_MAP_READ_WRITE even for mapping wgpu::MapMode:Read, because
        // we need write permission to initialize the buffer.
        // TODO(dawn:1705): investigate the performance impact of mapping with D3D11_MAP_READ_WRITE.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DAWN_TRY(CheckHRESULT(commandContext->Map(mD3d11Buffer.Get(),
                                                  /*Subresource=*/0, D3D11_MAP_READ_WRITE,
                                                  /*MapFlags=*/0, &mappedResource),
                              "ID3D11DeviceContext::Map"));
        mMappedData = static_cast<uint8_t*>(mappedResource.pData);

        return {};
    }

    void UnmapInternal(const ScopedCommandRecordingContext* commandContext) override {
        DAWN_ASSERT(mMappedData);
        commandContext->Unmap(mD3d11Buffer.Get(),
                              /*Subresource=*/0);
        mMappedData = nullptr;
    }

    MaybeError CopyToInternal(const ScopedCommandRecordingContext* commandContext,
                              uint64_t sourceOffset,
                              size_t size,
                              Buffer* destination,
                              uint64_t destinationOffset) override {
        return destination->CopyFromD3DInternal(commandContext, mD3d11Buffer.Get(), sourceOffset,
                                                size, destinationOffset);
    }

    MaybeError CopyFromD3DInternal(const ScopedCommandRecordingContext* commandContext,
                                   ID3D11Buffer* d3d11SourceBuffer,
                                   uint64_t sourceOffset,
                                   size_t size,
                                   uint64_t destinationOffset) override {
        D3D11_BOX srcBox;
        srcBox.left = static_cast<UINT>(sourceOffset);
        srcBox.top = 0;
        srcBox.front = 0;
        srcBox.right = static_cast<UINT>(sourceOffset + size);
        srcBox.bottom = 1;
        srcBox.back = 1;

        DAWN_ASSERT(d3d11SourceBuffer);

        commandContext->CopySubresourceRegion(mD3d11Buffer.Get(), /*DstSubresource=*/0,
                                              /*DstX=*/destinationOffset,
                                              /*DstY=*/0,
                                              /*DstZ=*/0, d3d11SourceBuffer, /*SrcSubresource=*/0,
                                              &srcBox);

        return {};
    }

    MaybeError WriteInternal(const ScopedCommandRecordingContext* commandContext,
                             uint64_t offset,
                             const void* data,
                             size_t size) override {
        if (size == 0) {
            return {};
        }

        ScopedMap scopedMap;
        DAWN_TRY_ASSIGN(scopedMap, ScopedMap::Create(commandContext, this, wgpu::MapMode::Write));

        DAWN_ASSERT(scopedMap.GetMappedData());
        memcpy(scopedMap.GetMappedData() + offset, data, size);

        return {};
    }

    ComPtr<ID3D11Buffer> mD3d11Buffer;
};

bool CanAddStorageUsageToBufferWithoutSideEffects(const Device* device,
                                                  wgpu::BufferUsage storageUsage,
                                                  wgpu::BufferUsage originalUsage,
                                                  size_t bufferSize) {
    // Don't support uniform buffers being used as storage buffer. Because D3D11 constant buffers
    // cannot be bound to SRV or UAV. Allowing them to be used as storage buffer would require some
    // workarounds including extra copies so it's better we prefer to not do that.
    if (originalUsage & wgpu::BufferUsage::Uniform) {
        return false;
    }

    // If buffer is small, we prefer CPU buffer for uploading so don't allow adding storage usage.
    if (CanUseCPUUploadBuffer(device, originalUsage, bufferSize)) {
        return false;
    }

    const bool requiresUAV = storageUsage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer);
    // Check supports for writeable storage usage:
    if (requiresUAV) {
        // D3D11 mappable buffers cannot be used as UAV natively. So avoid that.
        return !(originalUsage & kMappableBufferUsages);
    }

    // Read-only storage buffer cannot be mapped for read natively. Avoid that.
    DAWN_ASSERT(storageUsage == kReadOnlyStorageBuffer);
    return !(originalUsage & wgpu::BufferUsage::MapRead);
}

// static
ResultOrError<Ref<Buffer>> Buffer::Create(Device* device,
                                          const UnpackedPtr<BufferDescriptor>& descriptor,
                                          const ScopedCommandRecordingContext* commandContext,
                                          bool allowUploadBufferEmulation) {
    const auto actualUsage =
        ComputeInternalBufferUsages(device, descriptor->usage, descriptor->size);
    bool useUploadBuffer = allowUploadBufferEmulation;
    useUploadBuffer &= CanUseCPUUploadBuffer(device, actualUsage, descriptor->size);
    Ref<Buffer> buffer;
    if (useUploadBuffer) {
        buffer = AcquireRef(new UploadBuffer(device, descriptor));
    } else if (IsStaging(actualUsage)) {
        buffer = AcquireRef(new StagingBuffer(device, descriptor));
    } else {
        const auto& devInfo = ToBackend(device->GetPhysicalDevice())->GetDeviceInfo();
        // Use D3D11_MAP_WRITE_NO_OVERWRITE when possible to guarantee driver that we don't
        // overwrite data in use by GPU. MapAsync() already ensures that any GPU commands using this
        // buffer already finish. In return driver won't try to stall CPU for mapping access.
        D3D11_MAP mapWriteMode = devInfo.supportsMapNoOverwriteDynamicBuffers
                                     ? D3D11_MAP_WRITE_NO_OVERWRITE
                                     : D3D11_MAP_WRITE;
        buffer = AcquireRef(new GPUUsableBuffer(device, descriptor, mapWriteMode));
    }
    DAWN_TRY(buffer->Initialize(descriptor->mappedAtCreation, commandContext));
    return buffer;
}

Buffer::Buffer(DeviceBase* device,
               const UnpackedPtr<BufferDescriptor>& descriptor,
               wgpu::BufferUsage internalMappableFlags)
    : BufferBase(device, descriptor), mInternalMappableFlags(internalMappableFlags) {}

MaybeError Buffer::Initialize(bool mappedAtCreation,
                              const ScopedCommandRecordingContext* commandContext) {
    // TODO(dawn:1705): handle mappedAtCreation for NonzeroClearResourcesOnCreationForTesting

    // Allocate at least 4 bytes so clamped accesses are always in bounds.
    uint64_t size = std::max(GetSize(), uint64_t(4u));
    // The validation layer requires:
    // ByteWidth must be 12 or larger to be used with D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS.
    if (GetInternalUsage() & wgpu::BufferUsage::Indirect) {
        size = std::max(size, uint64_t(12u));
    }
    size_t alignment = D3D11BufferSizeAlignment(GetInternalUsage());
    // Check for overflow, bufferDescriptor.ByteWidth is a UINT.
    if (size > std::numeric_limits<UINT>::max() - alignment) {
        // Alignment would overlow.
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }
    mAllocatedSize = Align(size, alignment);

    DAWN_TRY(InitializeInternal());

    SetLabelImpl();

    if (!mappedAtCreation) {
        if (commandContext) {
            DAWN_TRY(ClearInitialResource(commandContext));
        } else {
            auto tmpCommandContext =
                ToBackend(GetDevice()->GetQueue())
                    ->GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
            DAWN_TRY(ClearInitialResource(&tmpCommandContext));
        }
    }

    return {};
}

MaybeError Buffer::ClearInitialResource(const ScopedCommandRecordingContext* commandContext) {
    if (GetDevice()->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
        DAWN_TRY(ClearWholeBuffer(commandContext, 1u));
    }

    // Initialize the padding bytes to zero.
    if (GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
        DAWN_TRY(ClearPaddingInternal(commandContext));
    }
    return {};
}

Buffer::~Buffer() = default;

bool Buffer::IsCPUWritableAtCreation() const {
    return IsCPUWritable();
}

bool Buffer::IsCPUWritable() const {
    return mInternalMappableFlags & wgpu::BufferUsage::MapWrite;
}

bool Buffer::IsCPUReadable() const {
    return mInternalMappableFlags & wgpu::BufferUsage::MapRead;
}

MaybeError Buffer::MapAtCreationImpl() {
    DAWN_ASSERT(IsCPUWritable());
    auto commandContext = ToBackend(GetDevice()->GetQueue())
                              ->GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
    return MapInternal(&commandContext, wgpu::MapMode::Write);
}

MaybeError Buffer::MapInternal(const ScopedCommandRecordingContext* commandContext,
                               wgpu::MapMode mode) {
    DAWN_UNREACHABLE();

    return {};
}

void Buffer::UnmapInternal(const ScopedCommandRecordingContext* commandContext) {
    DAWN_UNREACHABLE();
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    DAWN_ASSERT((mode == wgpu::MapMode::Write && IsCPUWritable()) ||
                (mode == wgpu::MapMode::Read && IsCPUReadable()));

    mMapReadySerial = mLastUsageSerial;
    const ExecutionSerial completedSerial = GetDevice()->GetQueue()->GetCompletedCommandSerial();
    // We may run into map stall in case that the buffer is still being used by previous submitted
    // commands. To avoid that, instead we ask Queue to do the map later when mLastUsageSerial has
    // passed.
    if (mMapReadySerial > completedSerial) {
        ToBackend(GetDevice()->GetQueue())->TrackPendingMapBuffer({this}, mode, mMapReadySerial);
    } else {
        auto commandContext = ToBackend(GetDevice()->GetQueue())
                                  ->GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
        DAWN_TRY(FinalizeMap(&commandContext, completedSerial, mode));
    }

    return {};
}

MaybeError Buffer::FinalizeMap(ScopedCommandRecordingContext* commandContext,
                               ExecutionSerial completedSerial,
                               wgpu::MapMode mode) {
    // Needn't map the buffer if this is for a previous mapAsync that was cancelled.
    if (completedSerial >= mMapReadySerial) {
        // Map then initialize data using mapped pointer.
        // The mapped pointer is always writable because:
        // - If mode is Write, then it's already writable.
        // - If mode is Read, it's only possible to map staging buffer. In that case,
        // D3D11_MAP_READ_WRITE will be used, hence the mapped pointer will also be writable.
        // TODO(dawn:1705): make sure the map call is not blocked by the GPU operations.
        DAWN_TRY(MapInternal(commandContext, mode));

        DAWN_TRY(EnsureDataInitialized(commandContext));
    }

    return {};
}

void Buffer::UnmapImpl() {
    DAWN_ASSERT(IsMappable(GetInternalUsage()));
    mMapReadySerial = kMaxExecutionSerial;
    if (mMappedData) {
        auto commandContext = ToBackend(GetDevice()->GetQueue())
                                  ->GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
        UnmapInternal(&commandContext);
    }
}

void* Buffer::GetMappedPointerImpl() {
    // The frontend asks that the pointer returned is from the start of the resource
    // irrespective of the offset passed in MapAsyncImpl, which is what mMappedData is.
    return mMappedData;
}

void Buffer::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the buffer is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the buffer.
    // - It may be called when the last ref to the buffer is dropped and the buffer
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the buffer since there are no other live refs.
    BufferBase::DestroyImpl();
    if (mMappedData) {
        UnmapImpl();
    }
}

MaybeError Buffer::EnsureDataInitialized(const ScopedCommandRecordingContext* commandContext) {
    if (!NeedsInitialization()) {
        return {};
    }

    DAWN_TRY(InitializeToZero(commandContext));
    return {};
}

MaybeError Buffer::EnsureDataInitializedAsDestination(
    const ScopedCommandRecordingContext* commandContext,
    uint64_t offset,
    uint64_t size) {
    if (!NeedsInitialization()) {
        return {};
    }

    if (IsFullBufferRange(offset, size)) {
        SetInitialized(true);
        return {};
    }

    DAWN_TRY(InitializeToZero(commandContext));
    return {};
}

MaybeError Buffer::EnsureDataInitializedAsDestination(
    const ScopedCommandRecordingContext* commandContext,
    const CopyTextureToBufferCmd* copy) {
    if (!NeedsInitialization()) {
        return {};
    }

    if (IsFullBufferOverwrittenInTextureToBufferCopy(copy)) {
        SetInitialized(true);
    } else {
        DAWN_TRY(InitializeToZero(commandContext));
    }

    return {};
}

MaybeError Buffer::InitializeToZero(const ScopedCommandRecordingContext* commandContext) {
    DAWN_ASSERT(NeedsInitialization());

    DAWN_TRY(ClearWholeBuffer(commandContext, uint8_t(0u)));
    SetInitialized(true);
    GetDevice()->IncrementLazyClearCountForTesting();

    return {};
}

MaybeError Buffer::PredicatedClear(const ScopedSwapStateCommandRecordingContext* commandContext,
                                   ID3D11Predicate* predicate,
                                   uint8_t clearValue,
                                   uint64_t offset,
                                   uint64_t size) {
    DAWN_UNREACHABLE();
    return {};
}

MaybeError Buffer::Clear(const ScopedCommandRecordingContext* commandContext,
                         uint8_t clearValue,
                         uint64_t offset,
                         uint64_t size) {
    DAWN_ASSERT(!mMappedData);

    if (size == 0) {
        return {};
    }

    // Map the buffer if it is possible, so EnsureDataInitializedAsDestination() and ClearInternal()
    // can write the mapped memory directly.
    ScopedMap scopedMap;
    DAWN_TRY_ASSIGN(scopedMap, ScopedMap::Create(commandContext, this, wgpu::MapMode::Write));

    // For non-staging buffers, we can use UpdateSubresource to write the data.
    DAWN_TRY(EnsureDataInitializedAsDestination(commandContext, offset, size));
    return ClearInternal(commandContext, clearValue, offset, size);
}

MaybeError Buffer::ClearWholeBuffer(const ScopedCommandRecordingContext* commandContext,
                                    uint8_t clearValue) {
    return ClearInternal(commandContext, clearValue, 0, GetAllocatedSize());
}

MaybeError Buffer::ClearInternal(const ScopedCommandRecordingContext* commandContext,
                                 uint8_t clearValue,
                                 uint64_t offset,
                                 uint64_t size) {
    DAWN_ASSERT(size != 0);

    // TODO(dawn:1705): use a reusable zero staging buffer to clear the buffer to avoid this CPU to
    // GPU copy.
    std::vector<uint8_t> clearData(size, clearValue);
    return WriteInternal(commandContext, offset, clearData.data(), size);
}

MaybeError Buffer::ClearPaddingInternal(const ScopedCommandRecordingContext* commandContext) {
    uint32_t paddingBytes = GetAllocatedSize() - GetSize();
    if (paddingBytes == 0) {
        return {};
    }
    uint32_t clearSize = paddingBytes;
    uint64_t clearOffset = GetSize();
    DAWN_TRY(ClearInternal(commandContext, 0, clearOffset, clearSize));

    return {};
}

MaybeError Buffer::Write(const ScopedCommandRecordingContext* commandContext,
                         uint64_t offset,
                         const void* data,
                         size_t size) {
    DAWN_ASSERT(size != 0);

    MarkUsedInPendingCommands();
    // Map the buffer if it is possible, so EnsureDataInitializedAsDestination() and WriteInternal()
    // can write the mapped memory directly.
    ScopedMap scopedMap;
    DAWN_TRY_ASSIGN(scopedMap, ScopedMap::Create(commandContext, this, wgpu::MapMode::Write));

    // For non-staging buffers, we can use UpdateSubresource to write the data.
    DAWN_TRY(EnsureDataInitializedAsDestination(commandContext, offset, size));
    return WriteInternal(commandContext, offset, data, size);
}

// static
MaybeError Buffer::Copy(const ScopedCommandRecordingContext* commandContext,
                        Buffer* source,
                        uint64_t sourceOffset,
                        size_t size,
                        Buffer* destination,
                        uint64_t destinationOffset) {
    DAWN_ASSERT(size != 0);

    DAWN_TRY(source->EnsureDataInitialized(commandContext));
    DAWN_TRY(
        destination->EnsureDataInitializedAsDestination(commandContext, destinationOffset, size));
    return source->CopyToInternal(commandContext, sourceOffset, size, destination,
                                  destinationOffset);
}

ResultOrError<Buffer::ScopedMap> Buffer::ScopedMap::Create(
    const ScopedCommandRecordingContext* commandContext,
    Buffer* buffer,
    wgpu::MapMode mode) {
    if (mode == wgpu::MapMode::Write && !buffer->IsCPUWritable()) {
        return ScopedMap();
    }
    if (mode == wgpu::MapMode::Read && !buffer->IsCPUReadable()) {
        return ScopedMap();
    }

    if (buffer->mMappedData) {
        return ScopedMap(commandContext, buffer, /*needsUnmap=*/false);
    }

    DAWN_TRY(buffer->MapInternal(commandContext, mode));
    return ScopedMap(commandContext, buffer, /*needsUnmap=*/true);
}

// ScopedMap
Buffer::ScopedMap::ScopedMap() = default;

Buffer::ScopedMap::ScopedMap(const ScopedCommandRecordingContext* commandContext,
                             Buffer* buffer,
                             bool needsUnmap)
    : mCommandContext(commandContext), mBuffer(buffer), mNeedsUnmap(needsUnmap) {}

Buffer::ScopedMap::~ScopedMap() {
    Reset();
}

Buffer::ScopedMap::ScopedMap(Buffer::ScopedMap&& other) {
    this->operator=(std::move(other));
}

Buffer::ScopedMap& Buffer::ScopedMap::operator=(Buffer::ScopedMap&& other) {
    Reset();
    mCommandContext = other.mCommandContext;
    mBuffer = other.mBuffer;
    mNeedsUnmap = other.mNeedsUnmap;
    other.mBuffer = nullptr;
    other.mNeedsUnmap = false;
    return *this;
}

void Buffer::ScopedMap::Reset() {
    if (mNeedsUnmap) {
        mBuffer->UnmapInternal(mCommandContext);
    }
    mCommandContext = nullptr;
    mBuffer = nullptr;
    mNeedsUnmap = false;
}

uint8_t* Buffer::ScopedMap::GetMappedData() const {
    return mBuffer ? mBuffer->mMappedData.get() : nullptr;
}

// GPUUsableBuffer::Storage
class GPUUsableBuffer::Storage : public RefCounted, NonCopyable {
  public:
    explicit Storage(ComPtr<ID3D11Buffer> d3d11Buffer) : mD3d11Buffer(std::move(d3d11Buffer)) {
        D3D11_BUFFER_DESC desc;
        mD3d11Buffer->GetDesc(&desc);
        mD3d11Usage = desc.Usage;

        mMappableCopyableFlags = wgpu::BufferUsage::CopySrc;

        switch (mD3d11Usage) {
            case D3D11_USAGE_STAGING:
                mMappableCopyableFlags |= kMappableBufferUsages | wgpu::BufferUsage::CopyDst;
                break;
            case D3D11_USAGE_DYNAMIC:
                mMappableCopyableFlags |= wgpu::BufferUsage::MapWrite;
                break;
            case D3D11_USAGE_DEFAULT:
                mMappableCopyableFlags |= wgpu::BufferUsage::CopyDst;
                break;
            default:
                break;
        }

        mIsConstantBuffer = desc.BindFlags & D3D11_BIND_CONSTANT_BUFFER;
    }

    ID3D11Buffer* GetD3D11Buffer() { return mD3d11Buffer.Get(); }

    uint64_t GetRevision() const { return mRevision; }
    void SetRevision(uint64_t revision) { mRevision = revision; }
    bool IsFirstRevision() const { return mRevision == 0; }

    bool IsConstantBuffer() const { return mIsConstantBuffer; }

    bool IsCPUWritable() const { return mMappableCopyableFlags & wgpu::BufferUsage::MapWrite; }
    bool IsCPUReadable() const { return mMappableCopyableFlags & wgpu::BufferUsage::MapRead; }
    bool IsStaging() const { return IsCPUReadable(); }
    bool SupportsCopyDst() const { return mMappableCopyableFlags & wgpu::BufferUsage::CopyDst; }
    bool IsGPUWritable() const { return mD3d11Usage == D3D11_USAGE_DEFAULT; }

  private:
    ComPtr<ID3D11Buffer> mD3d11Buffer;
    uint64_t mRevision = 0;
    D3D11_USAGE mD3d11Usage;
    bool mIsConstantBuffer = false;
    wgpu::BufferUsage mMappableCopyableFlags;
};

// GPUUsableBuffer
GPUUsableBuffer::GPUUsableBuffer(DeviceBase* device,
                                 const UnpackedPtr<BufferDescriptor>& descriptor,
                                 D3D11_MAP mapWriteMode)
    : Buffer(device,
             descriptor,
             /*internalMappableFlags=*/descriptor->usage & kMappableBufferUsages),
      mD3DMapWriteMode(mapWriteMode) {}

GPUUsableBuffer::~GPUUsableBuffer() = default;

void GPUUsableBuffer::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the buffer is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the buffer.
    // - It may be called when the last ref to the buffer is dropped and the buffer
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the buffer since there are no other live refs.
    Buffer::DestroyImpl();

    mSRVCache.clear();
    mUAVCache.clear();

    mLastUpdatedStorage = nullptr;
    mCPUWritableStorage = nullptr;
    mMappedStorage = nullptr;

    mStorages = {};
}

void GPUUsableBuffer::SetLabelImpl() {
    for (auto ite = mStorages.begin(); ite != mStorages.end(); ++ite) {
        auto storageType = static_cast<StorageType>(std::distance(mStorages.begin(), ite));
        SetStorageLabel(storageType);
    }
}

void GPUUsableBuffer::SetStorageLabel(StorageType storageType) {
    static constexpr ityp::array<GPUUsableBuffer::StorageType, const char*,
                                 static_cast<uint8_t>(StorageType::Count)>
        kStorageTypeStrings = {
            "Dawn_CPUWritableConstantBuffer",
            "Dawn_GPUCopyDstConstantBuffer",
            "Dawn_CPUWritableNonConstantBuffer",
            "Dawn_GPUWritableNonConstantBuffer",
            "Dawn_Staging",
        };

    if (!mStorages[storageType]) {
        return;
    }

    SetDebugName(ToBackend(GetDevice()), mStorages[storageType]->GetD3D11Buffer(),
                 kStorageTypeStrings[storageType], GetLabel());
}

MaybeError GPUUsableBuffer::InitializeInternal() {
    DAWN_ASSERT(!IsStaging(GetInternalUsage()));

    mStorages = {};

    wgpu::BufferUsage usagesToHandle = GetInternalUsage();

    // We need to create a separate storage for uniform usage, because D3D11 doesn't allow constant
    // buffer to be used for other purposes.
    if (usagesToHandle & wgpu::BufferUsage::Uniform) {
        usagesToHandle &=
            ~(wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopySrc | kInternalCopySrcBuffer);

        // Since D3D11 doesn't allow both CPU & GPU to write to a buffer, we need separate
        // storages for CPU writes and GPU writes.
        if (usagesToHandle & wgpu::BufferUsage::MapWrite) {
            // Note: we favor CPU write over GPU write if MapWrite is present. If buffer has GPU
            // writable usages, the GPU writable storage will be lazily created later.
            usagesToHandle &= ~wgpu::BufferUsage::MapWrite;
            DAWN_TRY_ASSIGN(mLastUpdatedStorage,
                            GetOrCreateStorage(StorageType::CPUWritableConstantBuffer));
            mCPUWritableStorage = mLastUpdatedStorage;
        } else {
            // For constant buffer, the only supported GPU op is copy. So create one storage for
            // that.
            usagesToHandle &= ~wgpu::BufferUsage::CopyDst;
            DAWN_TRY_ASSIGN(mLastUpdatedStorage,
                            GetOrCreateStorage(StorageType::GPUCopyDstConstantBuffer));
        }
    }

    if (usagesToHandle == wgpu::BufferUsage::None) {
        return {};
    }

    // Create separate storage for non-constant buffer usages if required.
    if (!IsStaging(usagesToHandle)) {
        if (usagesToHandle & wgpu::BufferUsage::MapWrite) {
            // Note: we only need one CPU writable storage. If there are both const buffer and
            // non-const buffer usages, we favor CPU writable const buffer first. Since that's most
            // likely the common use case where users want to update const buffer on CPU.
            DAWN_ASSERT(mCPUWritableStorage == nullptr);
            usagesToHandle &= ~wgpu::BufferUsage::MapWrite;
            // If a buffer is created with both Storage and MapWrite usages, then
            // we will lazily create a GPU writable storage later. Note: we favor CPU writable
            // over GPU writable when creating non-constant buffer storage. This is to optimize
            // the most common cases where MapWrite buffers are mostly updated by CPU.
            DAWN_TRY_ASSIGN(mLastUpdatedStorage,
                            GetOrCreateStorage(StorageType::CPUWritableNonConstantBuffer));
            mCPUWritableStorage = mLastUpdatedStorage;
        } else {
            usagesToHandle &= ~wgpu::BufferUsage::CopyDst;
            DAWN_TRY_ASSIGN(mLastUpdatedStorage,
                            GetOrCreateStorage(StorageType::GPUWritableNonConstantBuffer));
        }
    }

    // Special storage for staging.
    if (IsMappable(usagesToHandle)) {
        DAWN_TRY_ASSIGN(mLastUpdatedStorage, GetOrCreateStorage(StorageType::Staging));
    }

    return {};
}

ResultOrError<GPUUsableBuffer::Storage*> GPUUsableBuffer::GetOrCreateStorage(
    StorageType storageType) {
    if (mStorages[storageType]) {
        return mStorages[storageType].Get();
    }
    D3D11_BUFFER_DESC bufferDescriptor;
    bufferDescriptor.ByteWidth = GetAllocatedSize();
    bufferDescriptor.StructureByteStride = 0;

    switch (storageType) {
        case StorageType::CPUWritableConstantBuffer:
            bufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
            bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDescriptor.MiscFlags = 0;
            break;
        case StorageType::GPUCopyDstConstantBuffer:
            bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
            bufferDescriptor.CPUAccessFlags = 0;
            bufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDescriptor.MiscFlags = 0;
            break;
        case StorageType::CPUWritableNonConstantBuffer: {
            // Need to exclude GPU writable usages because CPU writable buffer is not GPU writable
            // in D3D11.
            auto nonUniformUsage =
                GetInternalUsage() & ~(kD3D11GPUWriteUsages | wgpu::BufferUsage::Uniform);
            bufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
            bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDescriptor.BindFlags = D3D11BufferBindFlags(nonUniformUsage);
            bufferDescriptor.MiscFlags = D3D11BufferMiscFlags(nonUniformUsage);
            if (bufferDescriptor.BindFlags == 0) {
                // Dynamic buffer requires at least one binding flag. If no binding flag is needed
                // (one example is MapWrite | QueryResolve), then use D3D11_BIND_INDEX_BUFFER.
                bufferDescriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;
                DAWN_ASSERT(bufferDescriptor.MiscFlags == 0);
            }
        } break;
        case StorageType::GPUWritableNonConstantBuffer: {
            // Need to exclude mapping usages.
            const auto nonUniformUsage =
                GetInternalUsage() & ~(kMappableBufferUsages | wgpu::BufferUsage::Uniform);
            bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
            bufferDescriptor.CPUAccessFlags = 0;
            bufferDescriptor.BindFlags = D3D11BufferBindFlags(nonUniformUsage);
            bufferDescriptor.MiscFlags = D3D11BufferMiscFlags(nonUniformUsage);
        } break;
        case StorageType::Staging: {
            bufferDescriptor.Usage = D3D11_USAGE_STAGING;
            bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            bufferDescriptor.BindFlags = 0;
            bufferDescriptor.MiscFlags = 0;
        } break;
        case StorageType::Count:
            DAWN_UNREACHABLE();
    }

    ComPtr<ID3D11Buffer> buffer;
    DAWN_TRY(CheckOutOfMemoryHRESULT(
        ToBackend(GetDevice())->GetD3D11Device()->CreateBuffer(&bufferDescriptor, nullptr, &buffer),
        "ID3D11Device::CreateBuffer"));

    mStorages[storageType] = AcquireRef(new Storage(std::move(buffer)));

    SetStorageLabel(storageType);

    return mStorages[storageType].Get();
}

ResultOrError<GPUUsableBuffer::Storage*> GPUUsableBuffer::GetOrCreateDstCopyableStorage() {
    if (mStorages[StorageType::GPUCopyDstConstantBuffer]) {
        return mStorages[StorageType::GPUCopyDstConstantBuffer].Get();
    }
    if (mStorages[StorageType::GPUWritableNonConstantBuffer]) {
        return mStorages[StorageType::GPUWritableNonConstantBuffer].Get();
    }

    if (GetInternalUsage() & wgpu::BufferUsage::Uniform) {
        return GetOrCreateStorage(StorageType::GPUCopyDstConstantBuffer);
    }

    return GetOrCreateStorage(StorageType::GPUWritableNonConstantBuffer);
}

MaybeError GPUUsableBuffer::SyncStorage(const ScopedCommandRecordingContext* commandContext,
                                        Storage* dstStorage) {
    DAWN_ASSERT(mLastUpdatedStorage);
    DAWN_ASSERT(dstStorage);
    if (mLastUpdatedStorage->GetRevision() == dstStorage->GetRevision()) {
        return {};
    }

    DAWN_ASSERT(commandContext);

    if (dstStorage->SupportsCopyDst()) {
        commandContext->CopyResource(dstStorage->GetD3D11Buffer(),
                                     mLastUpdatedStorage->GetD3D11Buffer());
        dstStorage->SetRevision(mLastUpdatedStorage->GetRevision());
        return {};
    }

    // TODO(42241146): This is a slow path. It's usually used by uncommon use cases:
    // - GPU writes a CPU writable buffer.
    DAWN_ASSERT(dstStorage->IsCPUWritable());
    Storage* stagingStorage;
    DAWN_TRY_ASSIGN(stagingStorage, GetOrCreateStorage(StorageType::Staging));
    DAWN_TRY(SyncStorage(commandContext, stagingStorage));
    D3D11_MAPPED_SUBRESOURCE mappedSrcResource;
    DAWN_TRY(CheckHRESULT(commandContext->Map(stagingStorage->GetD3D11Buffer(),
                                              /*Subresource=*/0, D3D11_MAP_READ,
                                              /*MapFlags=*/0, &mappedSrcResource),
                          "ID3D11DeviceContext::Map src"));

    auto MapAndCopy = [](const ScopedCommandRecordingContext* commandContext, ID3D11Buffer* dst,
                         const void* srcData, size_t size) -> MaybeError {
        D3D11_MAPPED_SUBRESOURCE mappedDstResource;
        DAWN_TRY(CheckHRESULT(commandContext->Map(dst,
                                                  /*Subresource=*/0, D3D11_MAP_WRITE_DISCARD,
                                                  /*MapFlags=*/0, &mappedDstResource),
                              "ID3D11DeviceContext::Map dst"));
        memcpy(mappedDstResource.pData, srcData, size);
        commandContext->Unmap(dst,
                              /*Subresource=*/0);
        return {};
    };

    auto result = MapAndCopy(commandContext, dstStorage->GetD3D11Buffer(), mappedSrcResource.pData,
                             GetAllocatedSize());

    commandContext->Unmap(stagingStorage->GetD3D11Buffer(),
                          /*Subresource=*/0);

    if (result.IsError()) {
        return result;
    }

    dstStorage->SetRevision(mLastUpdatedStorage->GetRevision());

    return {};
}

void GPUUsableBuffer::IncrStorageRevAndMakeLatest(
    const ScopedCommandRecordingContext* commandContext,
    Storage* dstStorage) {
    DAWN_ASSERT(dstStorage->GetRevision() == mLastUpdatedStorage->GetRevision());
    dstStorage->SetRevision(dstStorage->GetRevision() + 1);
    mLastUpdatedStorage = dstStorage;

    if (dstStorage->IsGPUWritable() && IsMappable(GetInternalUsage())) {
        // If this buffer is mappable and the last updated storage is GPU writable, we need to
        // update the staging storage when the command buffer is flushed.
        // This is to make sure the staging storage will contain the up-to-date GPU modified data.
        commandContext->AddBufferForSyncingWithCPU(this);
    }
}

MaybeError GPUUsableBuffer::SyncGPUWritesToStaging(
    const ScopedCommandRecordingContext* commandContext) {
    DAWN_ASSERT(IsMappable(GetInternalUsage()));

    // Only sync staging storage. Later other CPU writable storages can be updated by
    // copying from staging storage with Map(MAP_WRITE_DISCARD) which won't stall the CPU.
    // Otherwise, since CPU writable storages don't support CopyDst, it would require a CPU
    // stall in order to sync them here.
    Storage* stagingStorage;
    DAWN_TRY_ASSIGN(stagingStorage, GetOrCreateStorage(StorageType::Staging));

    return SyncStorage(commandContext, stagingStorage);
}

MaybeError GPUUsableBuffer::MapInternal(const ScopedCommandRecordingContext* commandContext,
                                        wgpu::MapMode mode) {
    DAWN_ASSERT(!mMappedData);

    D3D11_MAP mapType;
    Storage* storage;
    if (mode == wgpu::MapMode::Write) {
        DAWN_ASSERT(!mCPUWritableStorage->IsStaging());
        mapType = mD3DMapWriteMode;
        storage = mCPUWritableStorage;
    } else {
        // Always map buffer with D3D11_MAP_READ_WRITE if possible even for mapping
        // wgpu::MapMode:Read, because we need write permission to initialize the buffer.
        // TODO(dawn:1705): investigate the performance impact of mapping with
        // D3D11_MAP_READ_WRITE.
        mapType = D3D11_MAP_READ_WRITE;
        // If buffer has MapRead usage, a staging storage should already be created in
        // InitializeInternal().
        storage = mStorages[StorageType::Staging].Get();
    }

    DAWN_ASSERT(storage);

    // Sync previously modified content before mapping.
    DAWN_TRY(SyncStorage(commandContext, storage));

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    DAWN_TRY(CheckHRESULT(commandContext->Map(storage->GetD3D11Buffer(),
                                              /*Subresource=*/0, mapType,
                                              /*MapFlags=*/0, &mappedResource),
                          "ID3D11DeviceContext::Map"));
    mMappedData = static_cast<uint8_t*>(mappedResource.pData);
    mMappedStorage = storage;

    return {};
}

void GPUUsableBuffer::UnmapInternal(const ScopedCommandRecordingContext* commandContext) {
    DAWN_ASSERT(mMappedData);
    commandContext->Unmap(mMappedStorage->GetD3D11Buffer(),
                          /*Subresource=*/0);
    mMappedData = nullptr;
    // Since D3D11_MAP_READ_WRITE is used even for MapMode::Read, we need to increment the
    // revision.
    IncrStorageRevAndMakeLatest(commandContext, mMappedStorage);

    auto* stagingStorage = mStorages[StorageType::Staging].Get();

    if (stagingStorage && mLastUpdatedStorage != stagingStorage) {
        // If we have staging buffer (for MapRead), it has to be updated so later when user calls
        // Map + Read on this buffer, the stall might be avoided. Note: This is uncommon case where
        // the buffer is created with both MapRead & MapWrite. Technically it's impossible for the
        // following code to return error. Because in staging storage case, only CopyResource()
        // needs to be used. No extra allocations needed.
        [[maybe_unused]] bool hasError =
            GetDevice()->ConsumedError(SyncStorage(commandContext, stagingStorage));
        DAWN_ASSERT(!hasError);
    }

    mMappedStorage = nullptr;
}

ResultOrError<ID3D11Buffer*> GPUUsableBuffer::GetD3D11ConstantBuffer(
    const ScopedCommandRecordingContext* commandContext) {
    auto* storage = mStorages[StorageType::CPUWritableConstantBuffer].Get();
    if (storage && storage->GetRevision() == mLastUpdatedStorage->GetRevision()) {
        // The CPUWritableConstantBuffer is up to date, use it directly.
        return storage->GetD3D11Buffer();
    }

    // In all other cases we are going to use the GPUCopyDstConstantBuffer because, either it is up
    // to date, or we need to update the ConstantBuffer data and doing a CopyResource on the GPU is
    // always more efficient than paths involving a memcpy (or potentially a stall).
    DAWN_TRY_ASSIGN(storage, GetOrCreateStorage(StorageType::GPUCopyDstConstantBuffer));
    DAWN_TRY(SyncStorage(commandContext, storage));
    return storage->GetD3D11Buffer();
}

ResultOrError<ID3D11Buffer*> GPUUsableBuffer::GetD3D11NonConstantBuffer(
    const ScopedCommandRecordingContext* commandContext) {
    auto* storage = mStorages[StorageType::CPUWritableNonConstantBuffer].Get();
    if (storage && storage->GetRevision() == mLastUpdatedStorage->GetRevision()) {
        // The CPUWritableNonConstantBuffer is up to date, use it directly.
        return storage->GetD3D11Buffer();
    }

    // In all other cases we are going to use the GPUWritableNonConstantBuffe because, either it is
    // up to date, or we need to update the non-ConstantBuffer data and doing a CopyResource on the
    // GPU is always more efficient than paths involving a memcpy (or potentially a stall).
    DAWN_TRY_ASSIGN(storage, GetOrCreateStorage(StorageType::GPUWritableNonConstantBuffer));
    DAWN_TRY(SyncStorage(commandContext, storage));
    return storage->GetD3D11Buffer();
}

ID3D11Buffer* GPUUsableBuffer::GetD3D11ConstantBufferForTesting() {
    if (!mStorages[StorageType::CPUWritableConstantBuffer] &&
        !mStorages[StorageType::GPUCopyDstConstantBuffer]) {
        return nullptr;
    }
    auto tempCommandContext = ToBackend(GetDevice()->GetQueue())
                                  ->GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
    ID3D11Buffer* buffer;
    if (GetDevice()->ConsumedError(GetD3D11ConstantBuffer(&tempCommandContext), &buffer)) {
        return nullptr;
    }

    return buffer;
}

ID3D11Buffer* GPUUsableBuffer::GetD3D11NonConstantBufferForTesting() {
    if (!mStorages[StorageType::CPUWritableNonConstantBuffer] &&
        !mStorages[StorageType::GPUWritableNonConstantBuffer]) {
        return nullptr;
    }
    auto tempCommandContext = ToBackend(GetDevice()->GetQueue())
                                  ->GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
    ID3D11Buffer* buffer;
    if (GetDevice()->ConsumedError(GetD3D11NonConstantBuffer(&tempCommandContext), &buffer)) {
        return nullptr;
    }

    return buffer;
}

ResultOrError<ComPtr<ID3D11ShaderResourceView>>
GPUUsableBuffer::CreateD3D11ShaderResourceViewFromD3DBuffer(ID3D11Buffer* d3d11Buffer,
                                                            uint64_t offset,
                                                            uint64_t originalSize) {
    uint64_t size = Align(originalSize, 4);
    DAWN_ASSERT(IsAligned(offset, 4u));
    DAWN_ASSERT(size <= GetAllocatedSize());
    UINT firstElement = static_cast<UINT>(offset / 4);
    UINT numElements = static_cast<UINT>(size / 4);

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    desc.BufferEx.FirstElement = firstElement;
    desc.BufferEx.NumElements = numElements;
    desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
    ComPtr<ID3D11ShaderResourceView> srv;
    DAWN_TRY(CheckHRESULT(ToBackend(GetDevice())
                              ->GetD3D11Device()
                              ->CreateShaderResourceView(d3d11Buffer, &desc, &srv),
                          "ShaderResourceView creation"));

    return std::move(srv);
}

ResultOrError<ComPtr<ID3D11UnorderedAccessView1>>
GPUUsableBuffer::CreateD3D11UnorderedAccessViewFromD3DBuffer(ID3D11Buffer* d3d11Buffer,
                                                             uint64_t offset,
                                                             uint64_t originalSize) {
    uint64_t size = Align(originalSize, 4);
    DAWN_ASSERT(IsAligned(offset, 4u));
    DAWN_ASSERT(size <= GetAllocatedSize());

    UINT firstElement = static_cast<UINT>(offset / 4);
    UINT numElements = static_cast<UINT>(size / 4);

    D3D11_UNORDERED_ACCESS_VIEW_DESC1 desc;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = firstElement;
    desc.Buffer.NumElements = numElements;
    desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

    ComPtr<ID3D11UnorderedAccessView1> uav;
    DAWN_TRY(CheckHRESULT(ToBackend(GetDevice())
                              ->GetD3D11Device3()
                              ->CreateUnorderedAccessView1(d3d11Buffer, &desc, &uav),
                          "UnorderedAccessView creation"));

    return std::move(uav);
}

ResultOrError<ComPtr<ID3D11ShaderResourceView>> GPUUsableBuffer::UseAsSRV(
    const ScopedCommandRecordingContext* commandContext,
    uint64_t offset,
    uint64_t size) {
    ID3D11Buffer* d3dBuffer;

    DAWN_TRY_ASSIGN(d3dBuffer, GetD3D11NonConstantBuffer(commandContext));

    auto key = std::make_tuple(d3dBuffer, offset, size);
    auto ite = mSRVCache.find(key);
    if (ite != mSRVCache.end()) {
        return ite->second;
    }

    ComPtr<ID3D11ShaderResourceView> srv;
    DAWN_TRY_ASSIGN(srv, CreateD3D11ShaderResourceViewFromD3DBuffer(d3dBuffer, offset, size));

    mSRVCache[key] = srv;

    return std::move(srv);
}

ResultOrError<ComPtr<ID3D11UnorderedAccessView>> GPUUsableBuffer::UseAsUAV(
    const ScopedCommandRecordingContext* commandContext,
    uint64_t offset,
    uint64_t size) {
    Storage* storage = nullptr;
    DAWN_TRY_ASSIGN(storage, GetOrCreateStorage(StorageType::GPUWritableNonConstantBuffer));
    DAWN_TRY(SyncStorage(commandContext, storage));

    ComPtr<ID3D11UnorderedAccessView1> uav;
    {
        auto key = std::make_tuple(storage->GetD3D11Buffer(), offset, size);
        auto ite = mUAVCache.find(key);
        if (ite != mUAVCache.end()) {
            uav = ite->second;
        } else {
            DAWN_TRY_ASSIGN(uav, CreateD3D11UnorderedAccessViewFromD3DBuffer(
                                     storage->GetD3D11Buffer(), offset, size));
            mUAVCache[key] = uav;
        }
    }

    // Since UAV will modify the storage's content, increment its revision.
    IncrStorageRevAndMakeLatest(commandContext, storage);

    return ComPtr<ID3D11UnorderedAccessView>(std::move(uav));
}

MaybeError GPUUsableBuffer::UpdateD3D11ConstantBuffer(
    const ScopedCommandRecordingContext* commandContext,
    ID3D11Buffer* d3d11Buffer,
    bool firstTimeUpdate,
    uint64_t offset,
    const void* data,
    size_t size) {
    DAWN_ASSERT(size > 0);

    // For a full size write, UpdateSubresource1(D3D11_COPY_DISCARD) can be used to update
    // constant buffer.
    // WriteInternal() can be called with GetAllocatedSize(). We treat it as a full buffer write
    // as well.
    const bool fullSizeUpdate = size >= GetSize() && offset == 0;
    const bool canPartialUpdate =
        ToBackend(GetDevice())->GetDeviceInfo().supportsPartialConstantBufferUpdate;
    if (fullSizeUpdate || firstTimeUpdate) {
        const bool requiresFullAllocatedSizeWrite = !canPartialUpdate && !firstTimeUpdate;

        // Offset and size must be aligned with 16 for using UpdateSubresource1() on constant
        // buffer.
        size_t alignedOffset;
        if (offset < kConstantBufferUpdateAlignment - 1) {
            alignedOffset = 0;
        } else {
            DAWN_ASSERT(firstTimeUpdate);
            // For offset we align to lower value (<= offset).
            alignedOffset = Align(offset - (kConstantBufferUpdateAlignment - 1),
                                  kConstantBufferUpdateAlignment);
        }
        size_t alignedEnd;
        if (requiresFullAllocatedSizeWrite) {
            alignedEnd = GetAllocatedSize();
        } else {
            alignedEnd = Align(offset + size, kConstantBufferUpdateAlignment);
        }
        size_t alignedSize = alignedEnd - alignedOffset;

        DAWN_ASSERT((alignedSize % kConstantBufferUpdateAlignment) == 0);
        DAWN_ASSERT(alignedSize <= GetAllocatedSize());
        DAWN_ASSERT(offset >= alignedOffset);

        // Extra bytes on the left of offset we could write to. This is only valid if
        // firstTimeUpdate = true.
        size_t leftExtraBytes = offset - alignedOffset;
        DAWN_ASSERT(leftExtraBytes == 0 || firstTimeUpdate);

        // The layout of the buffer is like this:
        // |..........................| leftExtraBytes |     data   | ............... |
        // |<----------------- offset ---------------->|<-- size -->|
        // |<----- alignedOffset ---->|<--------- alignedSize --------->|
        std::unique_ptr<uint8_t[]> alignedBuffer;
        if (size != alignedSize) {
            alignedBuffer.reset(new uint8_t[alignedSize]);
            std::memcpy(alignedBuffer.get() + leftExtraBytes, data, size);
            data = alignedBuffer.get();
        }

        D3D11_BOX dstBox;
        dstBox.left = static_cast<UINT>(alignedOffset);
        dstBox.top = 0;
        dstBox.front = 0;
        dstBox.right = static_cast<UINT>(alignedOffset + alignedSize);
        dstBox.bottom = 1;
        dstBox.back = 1;
        // For full buffer write, D3D11_COPY_DISCARD is used to avoid GPU CPU synchronization.
        commandContext->UpdateSubresource1(d3d11Buffer, /*DstSubresource=*/0,
                                           requiresFullAllocatedSizeWrite ? nullptr : &dstBox, data,
                                           /*SrcRowPitch=*/0,
                                           /*SrcDepthPitch=*/0,
                                           /*CopyFlags=*/D3D11_COPY_DISCARD);
        return {};
    }

    // If copy offset and size are not 16 bytes aligned, we have to create a staging buffer for
    // transfer the data to constant buffer.
    Ref<BufferBase> stagingBuffer;
    DAWN_TRY_ASSIGN(stagingBuffer, ToBackend(GetDevice())->GetStagingBuffer(commandContext, size));
    stagingBuffer->MarkUsedInPendingCommands();
    DAWN_TRY(ToBackend(stagingBuffer)->WriteInternal(commandContext, 0, data, size));
    DAWN_TRY(ToBackend(stagingBuffer.Get())
                 ->CopyToInternal(commandContext,
                                  /*sourceOffset=*/0,
                                  /*size=*/size, this, offset));
    ToBackend(GetDevice())->ReturnStagingBuffer(std::move(stagingBuffer));

    return {};
}

MaybeError GPUUsableBuffer::WriteInternal(const ScopedCommandRecordingContext* commandContext,
                                          uint64_t offset,
                                          const void* data,
                                          size_t size) {
    if (size == 0) {
        return {};
    }

    // Map the buffer if it is possible, so WriteInternal() can write the mapped memory
    // directly.
    if (IsCPUWritable() &&
        mLastUsageSerial <= GetDevice()->GetQueue()->GetCompletedCommandSerial()) {
        ScopedMap scopedMap;
        DAWN_TRY_ASSIGN(scopedMap, ScopedMap::Create(commandContext, this, wgpu::MapMode::Write));

        DAWN_ASSERT(scopedMap.GetMappedData());
        memcpy(scopedMap.GetMappedData() + offset, data, size);
        return {};
    }

    // WriteInternal() can be called with GetAllocatedSize(). We treat it as a full buffer write
    // as well.
    bool fullSizeWrite = size >= GetSize() && offset == 0;

    // Mapping buffer at this point would stall the CPU. We will create a GPU copyable
    // storage and use UpdateSubresource on it below instead. Note if we have both const buffer &
    // non-const buffer, we favor writing to non-const buffer, because it has no alignment
    // requirement.
    Storage* gpuCopyableStorage = mStorages[StorageType::GPUWritableNonConstantBuffer].Get();
    if (!gpuCopyableStorage) {
        DAWN_TRY_ASSIGN(gpuCopyableStorage, GetOrCreateDstCopyableStorage());
    }

    if (!fullSizeWrite) {
        DAWN_TRY(SyncStorage(commandContext, gpuCopyableStorage));
    }

    const bool firstTimeUpdate = gpuCopyableStorage->IsFirstRevision();

    // We are going to write to the storage in all code paths, update the revision already.
    IncrStorageRevAndMakeLatest(commandContext, gpuCopyableStorage);

    if (gpuCopyableStorage->IsConstantBuffer()) {
        return UpdateD3D11ConstantBuffer(commandContext, gpuCopyableStorage->GetD3D11Buffer(),
                                         firstTimeUpdate, offset, data, size);
    }

    D3D11_BOX box;
    box.left = static_cast<UINT>(offset);
    box.top = 0;
    box.front = 0;
    box.right = static_cast<UINT>(offset + size);
    box.bottom = 1;
    box.back = 1;
    commandContext->UpdateSubresource1(gpuCopyableStorage->GetD3D11Buffer(),
                                       /*DstSubresource=*/0,
                                       /*pDstBox=*/&box, data,
                                       /*SrcRowPitch=*/0,
                                       /*SrcDepthPitch=*/0,
                                       /*CopyFlags=*/0);

    // No need to update constant buffer at this point, when command buffer wants to bind
    // the constant buffer in a render/compute pass, it will call GetD3D11ConstantBuffer()
    // and the constant buffer will be sync-ed there. WriteBuffer() cannot be called inside
    // render/compute pass so no need to sync here.
    return {};
}

MaybeError GPUUsableBuffer::CopyToInternal(const ScopedCommandRecordingContext* commandContext,
                                           uint64_t sourceOffset,
                                           size_t size,
                                           Buffer* destination,
                                           uint64_t destinationOffset) {
    ID3D11Buffer* d3d11SourceBuffer = mLastUpdatedStorage->GetD3D11Buffer();

    return destination->CopyFromD3DInternal(commandContext, d3d11SourceBuffer, sourceOffset, size,
                                            destinationOffset);
}

MaybeError GPUUsableBuffer::CopyFromD3DInternal(const ScopedCommandRecordingContext* commandContext,
                                                ID3D11Buffer* d3d11SourceBuffer,
                                                uint64_t sourceOffset,
                                                size_t size,
                                                uint64_t destinationOffset) {
    D3D11_BOX srcBox;
    srcBox.left = static_cast<UINT>(sourceOffset);
    srcBox.top = 0;
    srcBox.front = 0;
    srcBox.right = static_cast<UINT>(sourceOffset + size);
    srcBox.bottom = 1;
    srcBox.back = 1;

    Storage* gpuCopyableStorage;
    DAWN_TRY_ASSIGN(gpuCopyableStorage, GetOrCreateDstCopyableStorage());
    DAWN_TRY(SyncStorage(commandContext, gpuCopyableStorage));

    commandContext->CopySubresourceRegion(
        gpuCopyableStorage->GetD3D11Buffer(), /*DstSubresource=*/0,
        /*DstX=*/destinationOffset,
        /*DstY=*/0,
        /*DstZ=*/0, d3d11SourceBuffer, /*SrcSubresource=*/0, &srcBox);

    IncrStorageRevAndMakeLatest(commandContext, gpuCopyableStorage);

    return {};
}

MaybeError GPUUsableBuffer::PredicatedClear(
    const ScopedSwapStateCommandRecordingContext* commandContext,
    ID3D11Predicate* predicate,
    uint8_t clearValue,
    uint64_t offset,
    uint64_t size) {
    DAWN_ASSERT(size != 0);

    // Don't use mapping, mapping is not affected by ID3D11Predicate.
    // Allocate GPU writable storage and sync it. Note: we don't SetPredication() yet otherwise
    // it would affect the syncing.
    Storage* gpuWritableStorage;
    DAWN_TRY_ASSIGN(gpuWritableStorage,
                    GetOrCreateStorage(StorageType::GPUWritableNonConstantBuffer));
    DAWN_TRY(SyncStorage(commandContext, gpuWritableStorage));

    // SetPredication() and clear the storage with UpdateSubresource1().
    D3D11_BOX box;
    box.left = static_cast<UINT>(offset);
    box.top = 0;
    box.front = 0;
    box.right = static_cast<UINT>(offset + size);
    box.bottom = 1;
    box.back = 1;

    // TODO(350493305): Change function signature to accept a single uint64_t value.
    // So that we don't need to allocate a vector here.
    absl::InlinedVector<uint8_t, sizeof(uint64_t)> clearData(size, clearValue);

    // The update will *NOT* be performed if the predicate's data is false.
    commandContext->GetD3D11DeviceContext3()->SetPredication(predicate, false);
    commandContext->UpdateSubresource1(gpuWritableStorage->GetD3D11Buffer(),
                                       /*DstSubresource=*/0,
                                       /*pDstBox=*/&box, clearData.data(),
                                       /*SrcRowPitch=*/0,
                                       /*SrcDepthPitch=*/0,
                                       /*CopyFlags=*/0);
    commandContext->GetD3D11DeviceContext3()->SetPredication(nullptr, false);

    IncrStorageRevAndMakeLatest(commandContext, gpuWritableStorage);

    return {};
}

}  // namespace dawn::native::d3d11
