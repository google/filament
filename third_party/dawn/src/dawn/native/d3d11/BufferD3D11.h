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

#ifndef SRC_DAWN_NATIVE_D3D11_BUFFERD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_BUFFERD3D11_H_

#include <limits>
#include <memory>
#include <tuple>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/Atomic.h"
#include "dawn/common/ityp_array.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/d3d/d3d_platform.h"
#include "dawn/native/d3d11/Forward.h"
#include "dawn/native/d3d11/QueueD3D11.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::d3d11 {

class Device;
class ScopedCommandRecordingContext;
class ScopedSwapStateCommandRecordingContext;

bool CanAddStorageUsageToBufferWithoutSideEffects(const Device* device,
                                                  wgpu::BufferUsage storageUsage,
                                                  wgpu::BufferUsage originalUsage,
                                                  size_t bufferSize);

class Buffer : public BufferBase {
  public:
    static ResultOrError<Ref<Buffer>> Create(Device* device,
                                             const UnpackedPtr<BufferDescriptor>& descriptor,
                                             const ScopedCommandRecordingContext* commandContext,
                                             bool allowUploadBufferEmulation = true);

    MaybeError EnsureDataInitialized(const ScopedCommandRecordingContext* commandContext);
    MaybeError EnsureDataInitializedAsDestination(
        const ScopedCommandRecordingContext* commandContext,
        uint64_t offset,
        uint64_t size);
    MaybeError EnsureDataInitializedAsDestination(
        const ScopedCommandRecordingContext* commandContext,
        const CopyTextureToBufferCmd* copy);

    MaybeError Clear(const ScopedCommandRecordingContext* commandContext,
                     uint8_t clearValue,
                     uint64_t offset,
                     uint64_t size);
    MaybeError Write(const ScopedCommandRecordingContext* commandContext,
                     uint64_t offset,
                     const void* data,
                     size_t size);

    static MaybeError Copy(const ScopedCommandRecordingContext* commandContext,
                           Buffer* source,
                           uint64_t sourceOffset,
                           size_t size,
                           Buffer* destination,
                           uint64_t destinationOffset);

    // Attempt to do a scheduled map.
    MaybeError TryMapNow(ScopedCommandRecordingContext* commandContext,
                         ExecutionSerial completedSerial,
                         wgpu::MapMode mode);

    bool IsCPUWritable() const;
    bool IsCPUReadable() const;

    MaybeError UnmapIfNeeded(const ScopedCommandRecordingContext* commandContext);

    MaybeError TrackUsage(const ScopedCommandRecordingContext* commandContext,
                          ExecutionSerial pendingSerial);

    // This performs GPU Clear. Unlike Clear(), this will always be affected by ID3D11Predicate.
    // Whereas Clear() might be unaffected by ID3D11Predicate if it's pure CPU clear.
    virtual MaybeError PredicatedClear(const ScopedSwapStateCommandRecordingContext* commandContext,
                                       ID3D11Predicate* predicate,
                                       uint8_t clearValue,
                                       uint64_t offset,
                                       uint64_t size);

    // Write the buffer without checking if the buffer is initialized.
    virtual MaybeError WriteInternal(const ScopedCommandRecordingContext* commandContext,
                                     uint64_t bufferOffset,
                                     const void* data,
                                     size_t size,
                                     bool isInitialWrite) = 0;
    // Copy this buffer to the destination without checking if the buffer is initialized.
    virtual MaybeError CopyToInternal(const ScopedCommandRecordingContext* commandContext,
                                      uint64_t sourceOffset,
                                      size_t size,
                                      Buffer* destination,
                                      uint64_t destinationOffset) = 0;
    // Copy from a D3D buffer to this buffer without checking if the buffer is initialized.
    virtual MaybeError CopyFromD3DInternal(const ScopedCommandRecordingContext* commandContext,
                                           ID3D11Buffer* srcD3D11Buffer,
                                           uint64_t sourceOffset,
                                           size_t size,
                                           uint64_t destinationOffset) = 0;

    class ScopedMap : public NonCopyable {
      public:
        // Map buffer and return a ScopedMap object. If the buffer is not mappable,
        // scopedMap.GetMappedData() will return nullptr.
        static ResultOrError<ScopedMap> Create(const ScopedCommandRecordingContext* commandContext,
                                               Buffer* buffer,
                                               wgpu::MapMode mode);

        ScopedMap();
        ~ScopedMap();

        ScopedMap(ScopedMap&& other);
        ScopedMap& operator=(ScopedMap&& other);

        uint8_t* GetMappedData() const;

        void Reset();

      private:
        ScopedMap(const ScopedCommandRecordingContext* commandContext,
                  Buffer* buffer,
                  bool needsUnmap);

        raw_ptr<const ScopedCommandRecordingContext> mCommandContext = nullptr;
        raw_ptr<Buffer> mBuffer = nullptr;
        // Whether the buffer needs to be unmapped when the ScopedMap object is destroyed.
        bool mNeedsUnmap = false;
    };

  protected:
    Buffer(DeviceBase* device,
           const UnpackedPtr<BufferDescriptor>& descriptor,
           wgpu::BufferUsage internalMappableFlags,
           wgpu::MapMode autoMapMode);
    ~Buffer() override;

    void DestroyImpl(DestroyReason reason) override;

    virtual MaybeError InitializeInternal() = 0;

    virtual MaybeError MapInternal(const ScopedCommandRecordingContext* commandContext,
                                   wgpu::MapMode mode);
    virtual void UnmapInternal(const ScopedCommandRecordingContext* commandContext);

    // Clear the buffer without checking if the buffer is initialized.
    MaybeError ClearWholeBuffer(const ScopedCommandRecordingContext* commandContext,
                                uint8_t clearValue);
    virtual MaybeError ClearInternal(const ScopedCommandRecordingContext* commandContext,
                                     uint8_t clearValue,
                                     uint64_t offset,
                                     uint64_t size);

    virtual MaybeError ClearPaddingInternal(const ScopedCommandRecordingContext* commandContext);

    virtual ComPtr<ID3D11Buffer> GetD3D11MappedBuffer();

    Atomic<uint8_t*, std::memory_order::relaxed> mMappedData{nullptr};

  private:
    MaybeError Initialize(bool mappedAtCreation,
                          const ScopedCommandRecordingContext* commandContext);
    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override;
    MaybeError FinalizeMapImpl(BufferState newState) override;
    void UnmapImpl(BufferState oldState, BufferState newState) override;
    bool IsCPUWritableAtCreation() const override;
    MaybeError MapAtCreationImpl() override;
    void* GetMappedPointerImpl() override;
    std::optional<DeviceGuard> UseDeviceGuardForDestroy() override;

    MaybeError InitializeToZero(const ScopedCommandRecordingContext* commandContext);
    MaybeError EnsurePaddingInitialized(const ScopedCommandRecordingContext* commandContext);

    // Internal usage indicating the native buffer supports mapping for read and/or write or not.
    const wgpu::BufferUsage mInternalMappableFlags;
    const wgpu::MapMode mAutoMapMode;
    // Track whether padding bytes have been cleared to zero.
    bool mPaddingCleared = false;
    // Temporary storage for MapAtCreation when the lock cannot be acquired.
    std::unique_ptr<uint8_t[]> mMapAtCreationData;

    // A buffer can only have one scheduled map request at a time, so we embed the request object
    // here to avoid heap allocations.
    Queue::BufferMapRequest mMapRequest{this, wgpu::MapMode::None};
};

// Buffer that can be used by GPU. It manages several copies of the buffer, each with its own
// ID3D11Buffer storage for specific usage. For example, a buffer that has MapWrite + Storage usage
// will have at least two copies:
//  - One copy with D3D11_USAGE_DYNAMIC for mapping on CPU.
//  - One copy with D3D11_USAGE_DEFAULT for writing on GPU.
// Internally this class will synchronize the content between the copies so that when it is mapped
// or used by GPU, the appropriate copy will have the up-to-date content. The synchronizations are
// done in a way that minimizes CPU stall as much as possible.
// TODO(349848481): Consider making this the only Buffer class since it could cover all use cases.
class GPUUsableBuffer final : public Buffer {
  public:
    GPUUsableBuffer(DeviceBase* device, const UnpackedPtr<BufferDescriptor>& descriptor);
    ~GPUUsableBuffer() override;

    ResultOrError<ID3D11Buffer*> GetD3D11ConstantBuffer(
        const ScopedCommandRecordingContext* commandContext);
    ResultOrError<ID3D11Buffer*> GetD3D11NonConstantBuffer(
        const ScopedCommandRecordingContext* commandContext);

    ID3D11Buffer* GetD3D11ConstantBufferForTesting();
    ID3D11Buffer* GetD3D11NonConstantBufferForTesting();

    ResultOrError<ComPtr<ID3D11ShaderResourceView>>
    UseAsSRV(const ScopedCommandRecordingContext* commandContext, uint64_t offset, uint64_t size);
    ResultOrError<ComPtr<ID3D11UnorderedAccessView>>
    UseAsUAV(const ScopedCommandRecordingContext* commandContext, uint64_t offset, uint64_t size);

    MaybeError PredicatedClear(const ScopedSwapStateCommandRecordingContext* commandContext,
                               ID3D11Predicate* predicate,
                               uint8_t clearValue,
                               uint64_t offset,
                               uint64_t size) override;

    // Make sure CPU accessible storages are up-to-date. This is usually called at the end of a
    // command buffer after the buffer was modified on GPU.
    MaybeError SyncGPUWritesToStaging(const ScopedCommandRecordingContext* commandContext);

  private:
    class Storage;

    // Dawn API
    void DestroyImpl(DestroyReason reason) override;
    void SetLabelImpl() override;

    MaybeError InitializeInternal() override;
    MaybeError MapInternal(const ScopedCommandRecordingContext* commandContext,
                           wgpu::MapMode mode) override;
    void UnmapInternal(const ScopedCommandRecordingContext* commandContext) override;

    MaybeError CopyToInternal(const ScopedCommandRecordingContext* commandContext,
                              uint64_t sourceOffset,
                              size_t size,
                              Buffer* destination,
                              uint64_t destinationOffset) override;
    MaybeError CopyFromD3DInternal(const ScopedCommandRecordingContext* commandContext,
                                   ID3D11Buffer* srcD3D11Buffer,
                                   uint64_t sourceOffset,
                                   size_t size,
                                   uint64_t destinationOffset) override;

    MaybeError WriteInternal(const ScopedCommandRecordingContext* commandContext,
                             uint64_t bufferOffset,
                             const void* data,
                             size_t size,
                             bool isInitialWrite) override;

    ComPtr<ID3D11Buffer> GetD3D11MappedBuffer() override;

    ResultOrError<ComPtr<ID3D11ShaderResourceView>> CreateD3D11ShaderResourceViewFromD3DBuffer(
        ID3D11Buffer* d3d11Buffer,
        uint64_t offset,
        uint64_t size);
    ResultOrError<ComPtr<ID3D11UnorderedAccessView1>> CreateD3D11UnorderedAccessViewFromD3DBuffer(
        ID3D11Buffer* d3d11Buffer,
        uint64_t offset,
        uint64_t size);

    MaybeError UpdateD3D11ConstantBuffer(const ScopedCommandRecordingContext* commandContext,
                                         ID3D11Buffer* d3d11Buffer,
                                         bool firstTimeUpdate,
                                         uint64_t bufferOffset,
                                         const void* data,
                                         size_t size);

    // Storage types for different usages.
    // - Since D3D11 doesn't allow both CPU and GPU to write to a buffer, we need separate storages
    //   for CPU writing and GPU writing usages.
    // - Since D3D11 constant buffer cannot be bound for other purposes (e.g. vertex, storage, etc),
    //   we also need a separate storage for constant buffer and one storage for non-constant buffer
    //   purpose. Note: constant buffer's only supported GPU writing operation is CopyDst.
    // - Lastly, we usually need a separate staging storage for CPU reads.
    // - When MapOnDefaultBuffers is supported and the usage is compatible, mappable and GPU
    //   writable paths can alias a single D3D11 default-buffer storage.
    //
    // One example of a buffer being created with MapWrite | Uniform | Storage and being used:
    // - Map + CPU write: `CPUWritableConstantBuffer` gets updated.
    // - write on GPU:
    //   - buffer->UsedAsUAV: `CPUWritableConstantBuffer` is copied to
    //   `GPUWritableNonConstantBuffer`
    //   - GPU modifies `GPUWritableNonConstantBuffer`.
    //   - commandContext->AddBufferForSyncingWithCPU.
    //   - Queue::Submit
    //     - commandContext->FlushBuffersForSyncingWithCPU
    //        - buffer->SyncGPUWritesToStaging: `GPUWritableNonConstantBuffer` is copied to
    //        `Staging`.
    // - Map again:
    //   - `Staging` is copied to `CPUWritableConstantBuffer` with DISCARD flag
    enum class StorageType : uint8_t {
        // Storage for write mapping with constant buffer usage,
        CPUWritableConstantBuffer,
        // Storage for CopyB2B with destination having constant buffer usage,
        GPUCopyDstConstantBuffer,
        // Storage for write mapping with other usages (non-constant buffer),
        CPUWritableNonConstantBuffer,
        // Storage for GPU writing with other usages (non-constant buffer),
        GPUWritableNonConstantBuffer,
        // Storage for staging usage,
        Staging,
        // Storage shared by mappable and GPU writable paths when MapOnDefaultBuffers is used.
        MappableAndGPUWritable,

        Count,
    };

    ResultOrError<Storage*> GetOrCreateStorage(StorageType storageType);
    // Get or create storage supporting CopyDst usage.
    ResultOrError<Storage*> GetOrCreateDstCopyableStorage();

    void SetStorageLabel(StorageType storageType);

    // Update dstStorage to latest revision
    MaybeError SyncStorage(const ScopedCommandRecordingContext* commandContext,
                           Storage* dstStorage);
    // Increment the dstStorage's revision and make it the latest updated storage.
    void IncrStorageRevAndMakeLatest(const ScopedCommandRecordingContext* commandContext,
                                     Storage* dstStorage);

    using StorageMap =
        ityp::array<StorageType, Ref<Storage>, static_cast<uint8_t>(StorageType::Count)>;

    StorageMap mStorages;

    // The storage contains most up-to-date content.
    raw_ptr<Storage> mLastUpdatedStorage;
    // This points to either CPU writable constant buffer, CPU writable non-constant buffer,
    // staging buffer, or the shared MappableAndGPUWritable storage. We don't need multiple CPU
    // writable buffers to exist.
    raw_ptr<Storage> mMappableStorage;

    // TODO(dawn:381045722): Use LRU to limit number of cached entries.
    using BufferViewKey = std::tuple<ID3D11Buffer*, uint64_t, uint64_t>;
    absl::flat_hash_map<BufferViewKey, ComPtr<ID3D11ShaderResourceView>> mSRVCache;
    absl::flat_hash_map<BufferViewKey, ComPtr<ID3D11UnorderedAccessView1>> mUAVCache;

    D3D11_MAP mD3DMapTypeUsed = D3D11_MAP_WRITE;
};

static inline GPUUsableBuffer* ToGPUUsableBuffer(BufferBase* buffer) {
    return static_cast<GPUUsableBuffer*>(ToBackend(buffer));
}

static inline Ref<GPUUsableBuffer> ToGPUUsableBuffer(Ref<BufferBase>&& buffer) {
    return std::move(buffer).Cast<Ref<GPUUsableBuffer>>();
}

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_BUFFERD3D11_H_
