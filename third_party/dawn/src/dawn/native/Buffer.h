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

#include <atomic>
#include <functional>
#include <memory>
#include <utility>

#include "dawn/common/FutureUtils.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/native/DeviceGuard.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/SharedBufferMemory.h"
#include "dawn/native/UsageValidationMode.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

struct CopyTextureToBufferCmd;
class MemoryDump;

ResultOrError<UnpackedPtr<BufferDescriptor>> ValidateBufferDescriptor(
    DeviceBase* device,
    const BufferDescriptor* descriptor);

inline constexpr wgpu::BufferUsage kReadOnlyBufferUsages =
    wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Index |
    wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Uniform | kReadOnlyTexelBuffer |
    kReadOnlyStorageBuffer | kIndirectBufferForFrontendValidation |
    kIndirectBufferForBackendResourceTracking;

inline constexpr wgpu::BufferUsage kMappableBufferUsages =
    wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;

inline constexpr wgpu::BufferUsage kShaderBufferUsages =
    wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage | wgpu::BufferUsage::TexelBuffer |
    kInternalStorageBuffer | kReadOnlyStorageBuffer | kReadOnlyTexelBuffer;

inline constexpr wgpu::BufferUsage kReadOnlyShaderBufferUsages =
    kShaderBufferUsages & kReadOnlyBufferUsages;

// Return the actual internal buffer usages that will be used to create a buffer.
// In other words, after being created, buffer.GetInternalUsage() will return this value.
wgpu::BufferUsage ComputeInternalBufferUsages(const DeviceBase* device,
                                              wgpu::BufferUsage usage,
                                              size_t bufferSize);

ResultOrError<UnpackedPtr<TexelBufferViewDescriptor>> ValidateTexelBufferViewDescriptor(
    const BufferBase* buffer,
    const TexelBufferViewDescriptor* descriptor);

class BufferBase : public SharedResource, public WeakRefSupport<BufferBase> {
  public:
    // Calls FinishUse() on buffer when it goes out of scope. Caller is responsible for ensuring
    // buffer lifetime is longer than ScopedUseBuffer.
    class ScopedUseBuffer {
      public:
        ScopedUseBuffer();
        ~ScopedUseBuffer();

        ScopedUseBuffer(ScopedUseBuffer&& other);
        ScopedUseBuffer& operator=(ScopedUseBuffer&& other);

        // Caller will handle calling FinishUse() on buffer. Can't be called on an empty
        // ScopedUseBuffer.
        void Release();

      private:
        friend class BufferBase;

        explicit ScopedUseBuffer(BufferBase* buffer);

        raw_ptr<BufferBase> mBuffer = nullptr;
    };

    // TODO(crbug.com/467247254): See if ConcurrentAccessGuard<T> can be used be implemented and
    // used instead of having an InUse state.
    enum class BufferState {
        // The buffer is being used exclusively, for example by MapAsync(), Unmap() or by the queue.
        // Any concurrent use is an error.
        InUse,
        Unmapped,
        PendingMap,
        Mapped,
        MappedAtCreation,
        SharedMemoryNoAccess,
        Destroyed,
    };
    static bool IsMappedState(BufferState state);

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

    // Changes buffer state to denote it's being used. The buffer must be unmapped so this isn't
    // suitable for buffers that could be used by external API calls. `ScopedUseBuffer` will resets
    // state when it goes out of scope.
    [[nodiscard]] ScopedUseBuffer UseInternal();

    // Checks that the buffer is ready for use on queue. If successful, changes state to InUse and
    // returns `ScopedUseBuffer` which resets the state when it goes out of scope. Returns a
    // validation error on failure.
    ResultOrError<ScopedUseBuffer> ValidateCanUseOnQueueNow();

    // Called when buffer is done being used, on queue or internally. This should only be called
    // from ScopedUseBuffer or if ScopedUseBuffer was released from calling this.
    void FinishUse();

    bool IsFullBufferRange(uint64_t offset, uint64_t size) const;
    bool NeedsInitialization() const;
    void MarkUsedInPendingCommands();
    void MarkUsedInPendingCommands(ExecutionSerial pendingSerial);
    virtual MaybeError UploadData(uint64_t bufferOffset, const void* data, size_t size);

    // SharedResource impl.
    ExecutionSerial OnEndAccess() override;
    void OnBeginAccess() override;
    bool HasAccess() const override;
    bool IsDestroyed() const override;
    void SetInitialized(bool initialized) override;
    bool IsInitialized() const override;

    void* GetMappedPointer();
    void* GetMappedRange(size_t offset, size_t size, bool writable = true);

    // Internal non-reentrant version of Unmap. This is used in workarounds or additional copies.
    // Note that this will fail if the map event is pending since that should never happen
    // internally.
    MaybeError Unmap(bool forDestroy = false);

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

    ResultOrError<Ref<TexelBufferViewBase>> CreateTexelView(
        const TexelBufferViewDescriptor* descriptor);
    TexelBufferViewBase* APICreateTexelView(const TexelBufferViewDescriptor* descriptor);

    ApiObjectList* GetTexelBufferViewTrackingList();

  protected:
    BufferBase(DeviceBase* device, const UnpackedPtr<BufferDescriptor>& descriptor);
    BufferBase(DeviceBase* device, const BufferDescriptor* descriptor, ObjectBase::ErrorTag tag);

    void DestroyImpl(DestroyReason reason) override;

    ~BufferBase() override;

    // If no errors occur, returns true iff a staging buffer was used to implement the map at
    // creation. Otherwise, returns false indicating that backend specific mapping was used instead.
    ResultOrError<bool> MapAtCreationInternal();

    BufferState GetState() const;
    wgpu::MapMode MapMode() const;
    size_t MapOffset() const;
    size_t MapSize() const;

    uint64_t mAllocatedSize = 0;

  private:
    class MapAsyncEvent;

    // TODO(crbug.com/481211676): Remove this once all backends' DestroyImpl methods are
    // thread-safe.
    virtual std::optional<DeviceGuard> UseDeviceGuardForDestroy();

    virtual MaybeError MapAtCreationImpl() = 0;

    // Performs backend specific work to start mapping. The device mutex is not locked when this is
    // called so the implementation should lock if required.
    virtual MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) = 0;
    // Performs backend specific work to finalize mapping. `newState` is the state the buffer will
    // be in after this returns.
    virtual MaybeError FinalizeMapImpl(BufferState newState) = 0;
    virtual void* GetMappedPointerImpl() = 0;
    // Performs backend specific work to unmap. `oldState` is the state of the buffer before unmap
    // operation started. The device mutex is not locked when this is called so the implementation
    // should lock if required.
    // `newState` is the state of the buffer after unmap operation completed.
    virtual void UnmapImpl(BufferState oldState, BufferState newState) = 0;

    virtual bool IsCPUWritableAtCreation() const = 0;
    MaybeError CopyFromStagingBuffer();

    MaybeError ValidateMapAsync(wgpu::MapMode mode, size_t offset, size_t size) const;
    MaybeError ValidateUnmap() const;
    bool CanGetMappedRange(bool writable, size_t offset, size_t size) const;
    MaybeError UnmapInternal(bool forDestroy);

    // Updates internal state to reflect that the buffer is now mapped.
    MaybeError FinalizeMap(BufferState newState);

    // Atomically exchanges `currentState` to `desiredState`. Returns a validation error indicating
    // concurrent use if another thread has modified `mState` and compare_exchange() failed.
    MaybeError TransitionState(BufferState currentState, BufferState desiredState);

    const uint64_t mSize = 0;
    const wgpu::BufferUsage mUsage = wgpu::BufferUsage::None;
    const wgpu::BufferUsage mInternalUsage = wgpu::BufferUsage::None;
    const bool mIsHostMapped = false;
    bool mIsDataInitialized = false;

    Atomic<ExecutionSerial, std::memory_order_relaxed> mLastUsageSerial{ExecutionSerial(0)};

    // Once MapAsync() returns a future there is a possible race between MapAsyncEvent completing
    // and the buffer being unmapped as they can happen on different threads. `mPendingMapMutex`
    // must be locked when resetting `mPendingMapEvent` to guard against concurrent access.
    // `mPendingMapMutex` must also be locked for any access to MapAsyncEvent::mStatus/mErrorMessage
    // until after `mPendingMapEvent` is reset and potential race is averted.
    // Note: MutexProtected isn't used here due to Use() providing MapAsyncEvent* instead of
    // Ref<MapAsyncEvent> which doesn't allow resetting the Ref.
    Mutex mPendingMapMutex;
    Ref<MapAsyncEvent> mPendingMapEvent;

    // Track texel buffer views created from this buffer so they can be destroyed when the buffer is
    // destroyed.
    ApiObjectList mTexelBufferViews;

    // The following members are mutable state of the buffer w.r.t mapping. Access to the buffer is
    // required to be externally synchronized so there shouldn't be races accessing these member
    // variables. However, the variables are all loosely guarded by `mState` update ordering to
    // guard against concurrent access to the buffer.
    //
    // The follow semantics are used:
    // 1. On MapAsync() set state to InUse before modifying any other members. If there is a race
    //    modifying the state compare_exchange() will fail and a validation error is thrown. After
    //    modifying all member variables set state to PendingMap.
    // 2. When MapAsyncEvent completes set state to Mapped after all other work is finished.
    // 3. For *MappedRange() functions check that state is Mapped before checking other members for
    //    validation.
    // 4. For Unmap() set state to InUse before modifying any other member variables. If there is a
    //    race modifying state compare_exchange() will fail and a validation error is thrown. After
    //    the buffer is unmapped set state to Unmapped.
    // 5. When the buffer is used by the queue ValidateCanUseOnQueueNow() will be called before
    //    using the buffer which sets state to InUse. If the state is not unmapped or there is a
    //    race changing the state, the compare_exchange() will fail and a validation error is
    //    thrown. When the queue is done using the buffer the state is transitioned back to
    //    Unmapped.
    // 6. For Destroy() check if the state is InUse and if so spin loop until the concurrent
    //    operation is finished. This prevents destruction in the middle of an operation.
    //
    // With those `mState` changes in place, we can guarantee that if GetMappedRange() is
    // successful, that MapAsync() must have succeeded. We cannot guarantee, however, that Unmap()
    // did not race with GetMappedRange(), but that is the responsibility of the caller.
    std::atomic<BufferState> mState = BufferState::Unmapped;

    // A recursive buffer used to implement mappedAtCreation for buffers with non-mappable
    // usage. It is transiently allocated and released when the mappedAtCreation-buffer is
    // unmapped. Because this buffer itself is directly mappable, it will not create another
    // staging buffer recursively.
    Ref<BufferBase> mStagingBuffer = nullptr;

    // Mapping specific states.
    wgpu::MapMode mMapMode = wgpu::MapMode::None;
    size_t mMapOffset = 0;
    size_t mMapSize = 0;
    // TODO(crbug.com/485825675): Investigate this dangling pointers.
    raw_ptr<void, DanglingUntriaged> mMappedPointer = nullptr;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BUFFER_H_
