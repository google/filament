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

#ifndef SRC_DAWN_NATIVE_BUFFER_H_
#define SRC_DAWN_NATIVE_BUFFER_H_

#include <functional>
#include <memory>

#include "dawn/common/FutureUtils.h"
#include "dawn/common/NonCopyable.h"
#include "partition_alloc/pointers/raw_ptr.h"

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/SharedBufferMemory.h"
#include "dawn/native/UsageValidationMode.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

struct CopyTextureToBufferCmd;

enum class MapType : uint32_t;

ResultOrError<UnpackedPtr<BufferDescriptor>> ValidateBufferDescriptor(
    DeviceBase* device,
    const BufferDescriptor* descriptor);

static constexpr wgpu::BufferUsage kReadOnlyBufferUsages =
    wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Index |
    wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Uniform | kReadOnlyStorageBuffer |
    kIndirectBufferForFrontendValidation | kIndirectBufferForBackendResourceTracking;

static constexpr wgpu::BufferUsage kMappableBufferUsages =
    wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;

static constexpr wgpu::BufferUsage kShaderBufferUsages =
    wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage | kInternalStorageBuffer |
    kReadOnlyStorageBuffer;

static constexpr wgpu::BufferUsage kReadOnlyShaderBufferUsages =
    kShaderBufferUsages & kReadOnlyBufferUsages;

// Return the actual internal buffer usages that will be used to create a buffer.
// In other words, after being created, buffer.GetInternalUsage() will return this value.
wgpu::BufferUsage ComputeInternalBufferUsages(const DeviceBase* device,
                                              wgpu::BufferUsage usage,
                                              size_t bufferSize);

class BufferBase : public SharedResource {
  public:
    enum class BufferState {
        Unmapped,
        PendingMap,
        Mapped,
        MappedAtCreation,
        HostMappedPersistent,
        SharedMemoryNoAccess,
        Destroyed,
    };
    static Ref<BufferBase> MakeError(DeviceBase* device, const BufferDescriptor* descriptor);

    ObjectType GetType() const override;

    uint64_t GetSize() const;
    uint64_t GetAllocatedSize() const;
    ExecutionSerial GetLastUsageSerial() const;

    // |GetUsage| returns the usage with which the buffer was created using the base WebGPU API.
    // Additional usages may be added for internal state tracking. |GetInternalUsage| returns the
    // union of base usage and the usages added internally.
    wgpu::BufferUsage GetInternalUsage() const;
    wgpu::BufferUsage GetUsage() const;

    MaybeError MapAtCreation();

    MaybeError ValidateCanUseOnQueueNow() const;

    bool IsFullBufferRange(uint64_t offset, uint64_t size) const;
    bool NeedsInitialization() const;
    void MarkUsedInPendingCommands();
    virtual MaybeError UploadData(uint64_t bufferOffset, const void* data, size_t size);

    // SharedResource impl.
    ExecutionSerial OnEndAccess() override;
    void OnBeginAccess() override;
    bool HasAccess() const override;
    bool IsDestroyed() const override;
    void SetInitialized(bool initialized) override;
    bool IsInitialized() const override;

    virtual void* GetMappedPointer() = 0;
    void* GetMappedRange(size_t offset, size_t size, bool writable = true);
    MaybeError Unmap();

    void DumpMemoryStatistics(dawn::native::MemoryDump* dump, const char* prefix) const;

    // Dawn API
    Future APIMapAsync(wgpu::MapMode mode,
                       size_t offset,
                       size_t size,
                       const WGPUBufferMapCallbackInfo& callbackInfo);
    void* APIGetMappedRange(size_t offset, size_t size);
    const void* APIGetConstMappedRange(size_t offset, size_t size);
    wgpu::Status APIWriteMappedRange(size_t offset, void const* data, size_t size);
    wgpu::Status APIReadMappedRange(size_t offset, void* data, size_t size);
    void APIUnmap();
    void APIDestroy();
    wgpu::BufferUsage APIGetUsage() const;
    wgpu::BufferMapState APIGetMapState() const;
    uint64_t APIGetSize() const;

  protected:
    BufferBase(DeviceBase* device, const UnpackedPtr<BufferDescriptor>& descriptor);
    BufferBase(DeviceBase* device, const BufferDescriptor* descriptor, ObjectBase::ErrorTag tag);

    void DestroyImpl() override;

    ~BufferBase() override;

    MaybeError MapAtCreationInternal();

    uint64_t mAllocatedSize = 0;

    ExecutionSerial mLastUsageSerial = ExecutionSerial(0);

  private:
    virtual MaybeError MapAtCreationImpl() = 0;
    virtual MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) = 0;
    virtual void UnmapImpl() = 0;

    virtual bool IsCPUWritableAtCreation() const = 0;
    MaybeError CopyFromStagingBuffer();

    MaybeError ValidateMapAsync(wgpu::MapMode mode,
                                size_t offset,
                                size_t size,
                                WGPUMapAsyncStatus* status) const;
    MaybeError ValidateUnmap() const;
    bool CanGetMappedRange(bool writable, size_t offset, size_t size) const;
    void UnmapInternal(WGPUMapAsyncStatus status, std::string_view message);

    const uint64_t mSize = 0;
    const wgpu::BufferUsage mUsage = wgpu::BufferUsage::None;
    const wgpu::BufferUsage mInternalUsage = wgpu::BufferUsage::None;
    BufferState mState;
    bool mIsDataInitialized = false;

    // mStagingBuffer is used to implement mappedAtCreation for
    // buffers with non-mappable usage. It is transiently allocated
    // and released when the mappedAtCreation-buffer is unmapped.
    // Because `mStagingBuffer` itself is directly mappable, it will
    // not create another staging buffer.
    // i.e. buffer->mStagingBuffer->mStagingBuffer... is not possible.
    Ref<BufferBase> mStagingBuffer;

    wgpu::MapMode mMapMode = wgpu::MapMode::None;
    size_t mMapOffset = 0;
    size_t mMapSize = 0;

    struct MapAsyncEvent;
    Ref<MapAsyncEvent> mPendingMapEvent;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BUFFER_H_
