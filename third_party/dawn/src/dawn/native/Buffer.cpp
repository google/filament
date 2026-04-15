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

#include "dawn/native/Buffer.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"
#include "dawn/common/Alloc.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Log.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/CallbackTaskManager.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/Error.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/EventManager.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/Queue.h"
#include "dawn/native/SystemEvent.h"
#include "dawn/native/TexelBufferView.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

namespace {

std::unique_ptr<ErrorData> ConcurrentUseError() {
    return DAWN_VALIDATION_ERROR("Concurrent buffer operations are not allowed");
}

class ErrorBuffer final : public BufferBase {
  public:
    ErrorBuffer(DeviceBase* device, const BufferDescriptor* descriptor)
        : BufferBase(device, descriptor, ObjectBase::kError) {
        mAllocatedSize = descriptor->size;
    }

  private:
    bool IsCPUWritableAtCreation() const override { return true; }

    MaybeError MapAtCreationImpl() override {
        DAWN_ASSERT(mFakeMappedData == nullptr);

        // Check that the size can be used to allocate mFakeMappedData. A malloc(0)
        // is invalid, and on 32bit systems we should avoid a narrowing conversion that
        // would make size = 1 << 32 + 1 allocate one byte.
        uint64_t size = GetSize();
        bool isValidSize = size != 0 && size < uint64_t(std::numeric_limits<size_t>::max());

        if (isValidSize) {
            mFakeMappedData = std::unique_ptr<uint8_t[]>(AllocNoThrow<uint8_t>(size));
        }

        if (mFakeMappedData == nullptr) {
            return DAWN_OUT_OF_MEMORY_ERROR(
                "Failed to allocate memory to map ErrorBuffer at creation.");
        }

        return {};
    }

    MaybeError FinalizeMapImpl(BufferState newState) override { return {}; }

    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override {
        DAWN_UNREACHABLE();
    }

    void* GetMappedPointerImpl() override { return mFakeMappedData.get(); }

    void UnmapImpl(BufferState oldState, BufferState newState) override { mFakeMappedData.reset(); }

    std::unique_ptr<uint8_t[]> mFakeMappedData;
};

// GetMappedRange on a zero-sized buffer returns a pointer to this value.
static uint32_t sZeroSizedMappingData = 0xCAFED00D;

}  // anonymous namespace

ResultOrError<UnpackedPtr<TexelBufferViewDescriptor>> ValidateTexelBufferViewDescriptor(
    const BufferBase* buffer,
    const TexelBufferViewDescriptor* descriptor) {
    UnpackedPtr<TexelBufferViewDescriptor> desc;
    DAWN_TRY_ASSIGN(desc, ValidateAndUnpack(descriptor));

    DAWN_INVALID_IF(!(buffer->GetUsage() & wgpu::BufferUsage::TexelBuffer),
                    "Buffer usage (%s) missing TexelBuffer bit.", buffer->GetUsage());

    uint64_t size = desc->size == wgpu::kWholeSize ? buffer->GetSize() - desc->offset : desc->size;

    DAWN_INVALID_IF(desc->offset > buffer->GetSize() || size > buffer->GetSize() - desc->offset,
                    "Texel buffer view range (offset %u, size %u) exceeds buffer size %u.",
                    desc->offset, size, buffer->GetSize());

    const Format* formatInfo = nullptr;
    DAWN_TRY_ASSIGN(formatInfo, ValidateTexelBufferFormat(buffer->GetDevice(), desc->format));
    uint32_t texelSize = formatInfo->GetAspectInfo(Aspect::Color).block.byteSize;

    DAWN_INVALID_IF(desc->offset % kTexelBufferOffsetAlignment != 0,
                    "Texel buffer view offset (%u) must be a multiple of %u.", desc->offset,
                    kTexelBufferOffsetAlignment);
    DAWN_INVALID_IF(desc->offset % texelSize != 0,
                    "Texel buffer view offset (%u) must be %u-byte aligned.", desc->offset,
                    texelSize);
    DAWN_INVALID_IF(size % texelSize != 0,
                    "Texel buffer view size (%u) is not a multiple of texel size %u.", size,
                    texelSize);

    return desc;
}

wgpu::BufferUsage ComputeInternalBufferUsages(const DeviceBase* device,
                                              wgpu::BufferUsage usage,
                                              size_t bufferSize) {
    // Add readonly storage usage if the buffer has a storage usage. The validation rules in
    // ValidateSyncScopeResourceUsage will make sure we don't use both at the same time.
    if (usage & wgpu::BufferUsage::Storage) {
        usage |= kReadOnlyStorageBuffer;
    }

    // Texel buffers support read-only access without requiring storage usage.
    if (usage & wgpu::BufferUsage::TexelBuffer) {
        usage |= kReadOnlyTexelBuffer;
    }

    // The query resolve buffer need to be used as a storage buffer in the internal compute
    // pipeline which does timestamp uint conversion for timestamp query, it requires the buffer
    // has Storage usage in the binding group. Implicitly add an InternalStorage usage which is
    // only compatible with InternalStorageBuffer binding type in BGL. It shouldn't be
    // compatible with StorageBuffer binding type and the query resolve buffer cannot be bound
    // as storage buffer if it's created without Storage usage.
    if (usage & wgpu::BufferUsage::QueryResolve) {
        usage |= kInternalStorageBuffer;
    }

    // We also add internal storage usage for Indirect buffers for some transformations before
    // DispatchIndirect calls on the backend (e.g. validations, support of [[num_workgroups]] on
    // D3D12), since these transformations involve binding them as storage buffers for use in a
    // compute pass.
    if (usage & wgpu::BufferUsage::Indirect) {
        usage |= kInternalStorageBuffer;
    }

    if (usage & wgpu::BufferUsage::CopyDst) {
        const bool useComputeForT2B =
            device->IsToggleEnabled(Toggle::UseBlitForDepth16UnormTextureToBufferCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForDepth24PlusTextureToBufferCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForDepth32FloatTextureToBufferCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForStencilTextureToBufferCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForSnormTextureToBufferCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForBGRA8UnormTextureToBufferCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForRGB9E5UfloatTextureCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForRG11B10UfloatTextureCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForFloat16TextureCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForFloat32TextureCopy) ||
            device->IsToggleEnabled(Toggle::UseBlitForT2B);
        if (useComputeForT2B) {
            if (device->CanAddStorageUsageToBufferWithoutSideEffects(kInternalStorageBuffer, usage,
                                                                     bufferSize)) {
                // If the backend is ok with using this kind of buffer as storage buffer, we can add
                // Storage usage in order to write to it in compute shader.
                usage |= kInternalStorageBuffer;
            }

            // We also need CopySrc usage in order to copy to a temporary buffer.
            // The temporary buffer is needed in cases when offset doesn't satisfy certain
            // conditions. Or if it's not possible to add kInternalStorageBuffer usage to the
            // buffer.
            usage |= kInternalCopySrcBuffer;
        }
    }

    if ((usage & wgpu::BufferUsage::CopySrc) && device->IsToggleEnabled(Toggle::UseBlitForB2T)) {
        if (device->CanAddStorageUsageToBufferWithoutSideEffects(kReadOnlyStorageBuffer, usage,
                                                                 bufferSize)) {
            // If the backend is ok with using this kind of buffer as readonly storage buffer,
            // we can add Storage usage in order to read from it in pixel shader.
            usage |= kReadOnlyStorageBuffer;
        }
    }

    return usage;
}

class BufferBase::MapAsyncEvent final : public EventManager::TrackedEvent {
  public:
    // Create an event backed by the given queue execution serial.
    MapAsyncEvent(DeviceBase* device,
                  WeakRef<BufferBase> buffer,
                  const WGPUBufferMapCallbackInfo& callbackInfo,
                  ExecutionSerial serial)
        : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode),
                       device->GetQueue(),
                       serial),
          mBuffer(buffer),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2) {
        // `this` is used as a unique ID to match begin/end events for concurrent MapAsync calls.
        // It's not a problem that same memory address could be reused for a future MapAsync call
        // since it won't be concurrent with an earlier call.
        TRACE_EVENT_NESTABLE_ASYNC_BEGIN0(device->GetPlatform(), General, "Buffer::APIMapAsync",
                                          this);
    }

    // Create an event that's ready at creation (for errors, etc.)
    MapAsyncEvent(const WGPUBufferMapCallbackInfo& callbackInfo,
                  const std::string& message,
                  WGPUMapAsyncStatus status)
        : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode),
                       TrackedEvent::Completed{}),
          mStatus(status),
          mErrorMessage(message),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2) {
        DAWN_ASSERT(mStatus != WGPUMapAsyncStatus_Success);
    }

    ~MapAsyncEvent() override { EnsureComplete(EventCompletionType::Shutdown); }

    // Notifies that buffer will be unmapped before map completes. If this runs it will be before
    // mutex is locked in Complete(). Complete() will run callback with status/message set here.
    //
    // `BufferBase::mPendingMapMutex` must locked before this function is called!
    void UnmapEarly(std::string_view abortMessage) {
        DAWN_ASSERT(mStatus == WGPUMapAsyncStatus_Success);
        mStatus = WGPUMapAsyncStatus_Aborted;
        mErrorMessage = abortMessage;
    }

  private:
    void RunCallback(WGPUMapAsyncStatus status, std::string_view message) {
        mCallback(status, ToOutputStringView(message), mUserdata1.ExtractAsDangling(),
                  mUserdata2.ExtractAsDangling());
    }

    void Complete(EventCompletionType completionType) override {
        if (const auto* queueAndSerial = GetIfQueueAndSerial()) {
            if (auto queue = queueAndSerial->queue.Promote()) {
                TRACE_EVENT_NESTABLE_ASYNC_END0(queue->GetDevice()->GetPlatform(), General,
                                                "Buffer::APIMapAsync", this);
            }
        }

        if (completionType == EventCompletionType::Shutdown) {
            RunCallback(WGPUMapAsyncStatus_CallbackCancelled,
                        "A valid external Instance reference no longer exists.");
            return;
        }

        // There are four paths through this code beyond this point:
        // 1. Event was created with completion serial and event completes before anything else
        //    happens. This will acquire a strong ref, lock the mutex, reset `mPendingMapEvent`,
        //    unlock the mutex, run FinalizeMap() and then run callback.
        // 2. Event was created with completion serial, the buffer was unmapped before now and
        //    WeakRef is promoted successfully. This will lock the mutex, see `mStatus` is not
        //    success, unlock the mutex and run the callback with the abort message.
        // 3. Event was created with completion serial, the buffer is unmapped before event
        //    completes and weak ref fails to be promoted. The buffer was destroyed before this runs
        //    but otherwise this finishes on the same path as #2.
        // 4. Event was created for an error and `mBuffer` was always null. This uses
        //    `mErrorMessage`` and `mStatus` as set in the constructor when running the callback.
        Ref<BufferBase> buffer = mBuffer.Promote();
        if (buffer) {
            // Locking the mutex provides synchronization so that either path #1 or #2 is taken if
            // Complete() and Unmap() race on different threads.
            Mutex::AutoLock lock(&buffer->mPendingMapMutex);
            if (mStatus == WGPUMapAsyncStatus_Success) {
                // Complete() happened before Unmap().
                DAWN_ASSERT(buffer->mPendingMapEvent);
                buffer->mPendingMapEvent = nullptr;
            }
        }

        // UnmapEarly() either already ran or will never run so `mErrorMessage` is safe to access
        // without the mutex.
        if (mStatus != WGPUMapAsyncStatus_Success) {
            DAWN_ASSERT(!mErrorMessage.empty());
            RunCallback(mStatus, mErrorMessage);
            return;
        }

        DAWN_ASSERT(buffer);
        MaybeError result = buffer->FinalizeMap(BufferState::Mapped);
        buffer->mState.notify_all();
        if (result.IsError()) {
            auto error = result.AcquireError();
            DAWN_ASSERT(error->GetType() != InternalErrorType::Validation);
            std::string errorMsg = error->GetFormattedMessage();
            std::ignore = buffer->GetDevice()->ConsumedError(std::move(error));
            RunCallback(WGPUMapAsyncStatus_Error, errorMsg);
        } else {
            RunCallback(WGPUMapAsyncStatus_Success, "");
        }
    }

    // MapAsyncEvent stores a WeakRef to the buffer so that it can access the mutex and update the
    // buffer's map state if it completes.
    WeakRef<BufferBase> mBuffer;

    // Both variables are set in error constructor or in UnmapEarly() if set.
    WGPUMapAsyncStatus mStatus = WGPUMapAsyncStatus_Success;
    std::string mErrorMessage;

    WGPUBufferMapCallback mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;
};

ResultOrError<UnpackedPtr<BufferDescriptor>> ValidateBufferDescriptor(
    DeviceBase* device,
    const BufferDescriptor* descriptor) {
    UnpackedPtr<BufferDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    DAWN_TRY(ValidateBufferUsage(descriptor->usage));

    if (const auto* hostMappedDesc = unpacked.Get<BufferHostMappedPointer>()) {
        uint32_t requiredAlignment =
            device->GetLimits().hostMappedPointerLimits.hostMappedPointerAlignment;

        DAWN_INVALID_IF(!device->HasFeature(Feature::HostMappedPointer), "%s requires %s.",
                        hostMappedDesc->sType, ToAPI(Feature::HostMappedPointer));
        DAWN_INVALID_IF(!IsAligned(static_cast<uint32_t>(descriptor->size), requiredAlignment),
                        "Buffer size (%u) wrapping host-mapped memory was not aligned to %u.",
                        descriptor->size, requiredAlignment);
        DAWN_INVALID_IF(!IsPtrAligned(hostMappedDesc->pointer, requiredAlignment),
                        "Host-mapped memory pointer (%p) was not aligned to %u.",
                        hostMappedDesc->pointer, requiredAlignment);

        // TODO(dawn:2018) consider allowing the host-mapped buffers to be mapped through WebGPU.
        DAWN_INVALID_IF(
            descriptor->mappedAtCreation,
            "Buffer created from host-mapped pointer requires mappedAtCreation to be false.");
    }

    wgpu::BufferUsage usage = descriptor->usage;

    DAWN_INVALID_IF(usage == wgpu::BufferUsage::None, "Buffer usages must not be 0.");

    if (usage & wgpu::BufferUsage::TexelBuffer) {
        DAWN_INVALID_IF(!device->AreTexelBuffersEnabled(), "%s is not enabled.",
                        wgpu::WGSLLanguageFeatureName::TexelBuffers);
    }

    if (!device->HasFeature(Feature::BufferMapExtendedUsages)) {
        const wgpu::BufferUsage kMapWriteAllowedUsages =
            wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        DAWN_INVALID_IF(
            usage & wgpu::BufferUsage::MapWrite && !IsSubset(usage, kMapWriteAllowedUsages),
            "Buffer usages (%s) is invalid. If a buffer usage contains %s the only other allowed "
            "usage is %s.",
            usage, wgpu::BufferUsage::MapWrite, wgpu::BufferUsage::CopySrc);

        const wgpu::BufferUsage kMapReadAllowedUsages =
            wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
        DAWN_INVALID_IF(
            usage & wgpu::BufferUsage::MapRead && !IsSubset(usage, kMapReadAllowedUsages),
            "Buffer usages (%s) is invalid. If a buffer usage contains %s the only other allowed "
            "usage is %s.",
            usage, wgpu::BufferUsage::MapRead, wgpu::BufferUsage::CopyDst);
    }

    DAWN_INVALID_IF(descriptor->mappedAtCreation && descriptor->size % 4 != 0,
                    "Buffer is mapped at creation but its size (%u) is not a multiple of 4.",
                    descriptor->size);

    uint64_t maxBufferSize = device->GetLimits().v1.maxBufferSize;
    DAWN_INVALID_IF(descriptor->size > maxBufferSize,
                    "Buffer size (%u) exceeds the max buffer size limit (%u).%s", descriptor->size,
                    maxBufferSize,
                    DAWN_INCREASE_LIMIT_MESSAGE(device->GetAdapter()->GetLimits().v1, maxBufferSize,
                                                descriptor->size));

    return unpacked;
}

// Buffer

// static
bool BufferBase::IsMappedState(BufferState state) {
    return state == BufferBase::BufferState::Mapped ||
           state == BufferBase::BufferState::MappedAtCreation;
}

BufferBase::BufferBase(DeviceBase* device, const UnpackedPtr<BufferDescriptor>& descriptor)
    : SharedResource(device, descriptor->label),
      mSize(descriptor->size),
      mUsage(descriptor->usage),
      mInternalUsage(ComputeInternalBufferUsages(device, descriptor->usage, descriptor->size)),
      mIsHostMapped(descriptor.Has<BufferHostMappedPointer>()) {
    GetObjectTrackingList()->Track(this);
}

BufferBase::BufferBase(DeviceBase* device,
                       const BufferDescriptor* descriptor,
                       ObjectBase::ErrorTag tag)
    : SharedResource(device, tag, descriptor->label),
      mSize(descriptor->size),
      mUsage(descriptor->usage),
      mInternalUsage(descriptor->usage),
      mState(BufferState::Unmapped) {
    // Track the ErrorBuffer for destruction so it can be unmapped on destruction.
    // Don't do this if the device is already destroyed, so that CreateBuffer can still return
    // a mappedAtCreation buffer after device destroy (per spec).
    // TODO(crbug.com/42241190): Calling device.Destroy() *again* still won't unmap this
    // buffer. Need to fix this, OR change the spec to disallow mapping-at-creation after the
    // device is destroyed. (Note it should always be allowed on *non-destroyed* lost devices.)
    if (device->GetState() != DeviceBase::State::Destroyed) {
        GetObjectTrackingList()->Track(this);
    }
}

BufferBase::~BufferBase() {
    BufferState state = mState.load(std::memory_order::acquire);
    DAWN_ASSERT(state == BufferState::Unmapped || state == BufferState::Destroyed ||
                state == BufferState::SharedMemoryNoAccess ||
                // Happens if the buffer was created mappedAtCreation *after* device destroy.
                // TODO(crbug.com/42241190): This shouldn't be needed once the issue above is fixed,
                // because then bufferState will just be Destroyed.
                (state == BufferState::MappedAtCreation &&
                 GetDevice()->GetState() == DeviceBase::State::Destroyed));
}

void BufferBase::DestroyImpl(DestroyReason reason) {
    // If the initial state is Unmapped the compared_exchange() should be successful. If not the
    // current state has been loaded in `state` and loop body handles anything needed like unmapping
    // the buffer.
    BufferState state = BufferState::Unmapped;
    while (
        !mState.compare_exchange_weak(state, BufferState::Destroyed, std::memory_order::acq_rel)) {
        switch (state) {
            case BufferState::Mapped:
            case BufferState::PendingMap:
            case BufferState::MappedAtCreation: {
                [[maybe_unused]] bool hadError =
                    GetDevice()->ConsumedError(UnmapInternal(true), "calling %s.Destroy().", this);
                // The buffer state should be unmapped after UnmapInternal() returns. Use that state
                // in the next compare exchange but another thread can update the state causing this
                // loop to run again.
                state = BufferState::Unmapped;
                break;
            }
            case BufferState::InUse: {
                // This is never supposed to happen but another operation is happening concurrently
                // with API Destroy() call.
                [[maybe_unused]] bool hadError =
                    GetDevice()->ConsumedError(ConcurrentUseError(), "calling %s.Destroy().", this);
                while (mState.load(std::memory_order::acquire) == BufferState::InUse) {
                    // Spin loop instead of wait() to avoid overhead of signal in map/unmap.
                }
                break;
            }
            case BufferState::Destroyed:
                DAWN_UNREACHABLE();
            case BufferState::Unmapped:
            case BufferState::SharedMemoryNoAccess:
                // Buffer is ready to be destroyed.
                break;
        }
    }

    mTexelBufferViews.Destroy(DestroyReason::EarlyDestroy);
}

std::optional<DeviceGuard> BufferBase::UseDeviceGuardForDestroy() {
    // Backends with thread-safe DestroyImpl() methods can override this to return nullopt.
    return GetDevice()->GetGuard();
}

// static
Ref<BufferBase> BufferBase::MakeError(DeviceBase* device, const BufferDescriptor* descriptor) {
    return AcquireRef(new ErrorBuffer(device, descriptor));
}

ObjectType BufferBase::GetType() const {
    return ObjectType::Buffer;
}

uint64_t BufferBase::GetSize() const {
    return mSize;
}

uint64_t BufferBase::GetAllocatedSize() const {
    // The backend must initialize this value.
    DAWN_ASSERT(mAllocatedSize != 0);
    return mAllocatedSize;
}

wgpu::BufferUsage BufferBase::GetInternalUsage() const {
    DAWN_ASSERT(!IsError());
    return mInternalUsage;
}

wgpu::BufferUsage BufferBase::GetUsage() const {
    DAWN_ASSERT(!IsError());
    return mUsage;
}

wgpu::BufferUsage BufferBase::APIGetUsage() const {
    return mUsage;
}

wgpu::BufferMapState BufferBase::APIGetMapState() const {
    switch (mState.load(std::memory_order::acquire)) {
        case BufferState::Mapped:
        case BufferState::MappedAtCreation:
            return wgpu::BufferMapState::Mapped;
        case BufferState::PendingMap:
            return wgpu::BufferMapState::Pending;
        case BufferState::Unmapped:
            DAWN_ASSERT(!mIsHostMapped);
            ABSL_FALLTHROUGH_INTENDED;
        case BufferState::Destroyed:
        case BufferState::InUse:
        case BufferState::SharedMemoryNoAccess:
            return wgpu::BufferMapState::Unmapped;
        default:
            DAWN_UNREACHABLE();
    }
}

MaybeError BufferBase::FinalizeMap(BufferState newState) {
    // There are only 2 valid transitions:
    //   1) Nominal: PendingMap -> Mapped
    //   2) MappedAtCreation case because initial state is unmapped: Unmapped -> MappedAtCreation.
    BufferState oldState = mState.load(std::memory_order::acquire);
    DAWN_ASSERT((oldState == BufferState::PendingMap && newState == BufferState::Mapped) ||
                (oldState == BufferState::Unmapped && newState == BufferState::MappedAtCreation));

    DAWN_TRY_WITH_CLEANUP(FinalizeMapImpl(newState),
                          { mState.store(BufferState::Unmapped, std::memory_order::release); });

    if (mStagingBuffer) {
        mMappedPointer = mStagingBuffer->GetMappedPointerImpl();
    } else if (GetSize() == 0) {
        mMappedPointer = static_cast<void*>(&sZeroSizedMappingData);
    } else {
        mMappedPointer = GetMappedPointerImpl();
    }

    mState.store(newState, std::memory_order::release);
    return {};
}

MaybeError BufferBase::MapAtCreation() {
    bool usingStagingBuffer = false;
    DAWN_TRY_ASSIGN(usingStagingBuffer, MapAtCreationInternal());

    if (GetSize() == 0) {
        return {};
    }
    size_t size = GetAllocatedSize();
    void* ptr = GetMappedPointer();

    DeviceBase* device = GetDevice();
    if (device->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse) &&
        !device->IsToggleEnabled(Toggle::DisableLazyClearForMappedAtCreationBuffer)) {
        // The staging buffer is created with `MappedAtCreation == true` and the main buffer will
        // actually get initialized when the staging data is copied in. (But we mark the main buffer
        // as initialized now.)
        if (!usingStagingBuffer) {
            memset(ptr, uint8_t(0u), size);
            device->IncrementLazyClearCountForTesting();
        }
    } else if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
        memset(ptr, uint8_t(1u), size);
    }
    // Mark the buffer as initialized since we don't want to later clear it using the GPU since that
    // would overwrite what the client wrote using the CPU.
    SetInitialized(true);

    return {};
}

ResultOrError<bool> BufferBase::MapAtCreationInternal() {
    DAWN_ASSERT(mState.load(std::memory_order::acquire) == BufferState::Unmapped);
    Ref<BufferBase> stagingBuffer;

    // 0-sized buffers are not supposed to be written to. Return back any non-null pointer.
    // Skip handling 0-sized buffers so we don't try to map them in the backend.
    if (mSize != 0) {
        // Mappable buffers don't use a staging buffer and are just as if mapped through
        // MapAsync.
        if (IsCPUWritableAtCreation()) {
            DAWN_TRY(MapAtCreationImpl());
        } else {
            // If any of these fail, the buffer will be deleted and replaced with an error
            // buffer. The staging buffer is used to return mappable data to initialize the
            // buffer contents. Allocate one as large as the real buffer size so that every byte
            // is initialized.
            // TODO(crbug.com/dawn/828): Suballocate and reuse memory from a larger staging
            // buffer so we don't create many small buffers.
            BufferDescriptor stagingBufferDesc = {};
            stagingBufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;
            stagingBufferDesc.size = Align(GetAllocatedSize(), 4);
            stagingBufferDesc.mappedAtCreation = true;
            stagingBufferDesc.label = "Dawn_MappedAtCreationStaging";
            DAWN_TRY_ASSIGN(stagingBuffer, GetDevice()->CreateBuffer(&stagingBufferDesc));
        }
    }

    // Only set the state to mapped at creation if we did not fail any point in this helper.
    // Otherwise, if we override the default unmapped state before succeeding to create a
    // staging buffer, we will have issues when we try to destroy the buffer.
    mMapMode = wgpu::MapMode::Write;
    mMapOffset = 0;
    mMapSize = mSize;
    mStagingBuffer = std::move(stagingBuffer);
    DAWN_TRY(FinalizeMap(BufferState::MappedAtCreation));
    return mStagingBuffer != nullptr;
}

BufferBase::BufferState BufferBase::GetState() const {
    return mState.load(std::memory_order::acquire);
}

wgpu::MapMode BufferBase::MapMode() const {
    return mMapMode;
}

size_t BufferBase::MapOffset() const {
    return mMapOffset;
}

size_t BufferBase::MapSize() const {
    return mMapSize;
}

BufferBase::ScopedUseBuffer BufferBase::UseInternal() {
    auto fromState = mState.exchange(BufferState::InUse, std::memory_order::acq_rel);
    DAWN_CHECK(fromState == BufferState::Unmapped);
    return ScopedUseBuffer(this);
}

ResultOrError<BufferBase::ScopedUseBuffer> BufferBase::ValidateCanUseOnQueueNow() {
    DAWN_ASSERT(!IsError());

    switch (BufferState state = mState.load(std::memory_order::acquire)) {
        case BufferState::Destroyed:
            return DAWN_VALIDATION_ERROR("%s used in submit while destroyed.", this);
        case BufferState::Mapped:
        case BufferState::MappedAtCreation:
            return DAWN_VALIDATION_ERROR("%s used in submit while mapped.", this);
        case BufferState::PendingMap:
            return DAWN_VALIDATION_ERROR("%s used in submit while pending map.", this);
        case BufferState::SharedMemoryNoAccess:
            return DAWN_VALIDATION_ERROR("%s used in submit without shared memory access.", this);
        case BufferState::InUse:
            return ConcurrentUseError();
        case BufferState::Unmapped:
            DAWN_TRY(TransitionState(state, BufferState::InUse));
            return ScopedUseBuffer(this);
    }
    DAWN_UNREACHABLE();
}

void BufferBase::FinishUse() {
    BufferState fromState = mState.exchange(BufferState::Unmapped, std::memory_order::acq_rel);
    DAWN_CHECK(fromState == BufferState::InUse);
}

Future BufferBase::APIMapAsync(wgpu::MapMode mode,
                               size_t offset,
                               size_t size,
                               const WGPUBufferMapCallbackInfo& callbackInfo) {
    // TODO(crbug.com/dawn/2052): Once we always return a future, change this to log to the instance
    // (note, not raise a validation error to the device) and return the null future.
    DAWN_ASSERT(callbackInfo.nextInChain == nullptr);

    Ref<MapAsyncEvent> event;
    {
        // Handle the defaulting of size required by WebGPU, even if in webgpu_cpp.h it is not
        // possible to default the function argument (because there is the callback later in the
        // argument list)
        if ((size == wgpu::kWholeMapSize) && (offset <= mSize)) {
            size = mSize - offset;
        }

        WGPUMapAsyncStatus errorStatus = WGPUMapAsyncStatus_Aborted;
        MaybeError maybeError = [&]() -> MaybeError {
            DAWN_TRY(GetDevice()->ValidateIsAlive());
            errorStatus = WGPUMapAsyncStatus_Error;
            DAWN_TRY(ValidateMapAsync(mode, offset, size));

            switch (mState.load(std::memory_order::acquire)) {
                case BufferState::Mapped:
                case BufferState::MappedAtCreation:
                    return DAWN_VALIDATION_ERROR("%s is already mapped.", this);
                case BufferState::InUse:
                    return ConcurrentUseError();
                case BufferState::PendingMap:
                    return DAWN_VALIDATION_ERROR("%s already has an outstanding map pending.",
                                                 this);
                case BufferState::Destroyed:
                    return DAWN_VALIDATION_ERROR("%s is destroyed.", this);
                case BufferState::SharedMemoryNoAccess:
                    return DAWN_VALIDATION_ERROR("%s used without shared memory access.", this);
                case BufferState::Unmapped:
                    break;
            }

            DAWN_TRY(TransitionState(BufferState::Unmapped, BufferState::InUse));
            DAWN_TRY_WITH_CLEANUP(MapAsyncImpl(mode, offset, size), {
                // Reset state since an error stopped this from reaching pending map state.
                mState.store(BufferState::Unmapped, std::memory_order::release);
            });
            return {};
        }();

        if (maybeError.IsError()) {
            auto error = maybeError.AcquireError();
            event = AcquireRef(new MapAsyncEvent(callbackInfo, error->GetMessage(), errorStatus));
            [[maybe_unused]] bool hadError = GetDevice()->ConsumedError(
                std::move(error), "calling %s.MapAsync(%s, %u, %u, ...).", this, mode, offset,
                size);
        } else {
            mMapMode = mode;
            mMapOffset = offset;
            mMapSize = size;

            event =
                AcquireRef(new MapAsyncEvent(GetDevice(), this, callbackInfo, mLastUsageSerial));
            mMappedPointer = nullptr;
            DAWN_ASSERT(!mPendingMapEvent);
            mPendingMapEvent = event;
            mState.store(BufferState::PendingMap, std::memory_order::release);
        }
    }

    DAWN_ASSERT(event);
    FutureID futureID = GetInstance()->GetEventManager()->TrackEvent(std::move(event));
    return {futureID};
}

void* BufferBase::APIGetMappedRange(size_t offset, size_t size) {
    return GetMappedRange(offset, size, true);
}

const void* BufferBase::APIGetConstMappedRange(size_t offset, size_t size) {
    return GetMappedRange(offset, size, false);
}

wgpu::Status BufferBase::APIWriteMappedRange(size_t offset, void const* data, size_t size) {
    void* range = APIGetMappedRange(offset, size);
    if (range == nullptr) {
        return wgpu::Status::Error;
    }

    memcpy(range, data, size);
    return wgpu::Status::Success;
}

wgpu::Status BufferBase::APIReadMappedRange(size_t offset, void* data, size_t size) {
    const void* range = APIGetConstMappedRange(offset, size);
    if (range == nullptr) {
        return wgpu::Status::Error;
    }

    memcpy(data, range, size);
    return wgpu::Status::Success;
}

void* BufferBase::GetMappedPointer() {
    if (!IsMappedState(mState.load(std::memory_order::acquire))) {
        return nullptr;
    }
    return mMappedPointer.get();
}

void* BufferBase::GetMappedRange(size_t offset, size_t size, bool writable) {
    if (!CanGetMappedRange(writable, offset, size)) {
        return nullptr;
    }
    uint8_t* start = static_cast<uint8_t*>(GetMappedPointer());
    return start == nullptr ? nullptr : start + offset;
}

void BufferBase::APIDestroy() {
    auto deviceGuard = UseDeviceGuardForDestroy();
    Destroy();
}

uint64_t BufferBase::APIGetSize() const {
    return mSize;
}

MaybeError BufferBase::CopyFromStagingBuffer() {
    DAWN_ASSERT(mStagingBuffer != nullptr && mSize != 0);

    auto deviceGuard = GetDevice()->GetGuard();

    DAWN_TRY(
        GetDevice()->CopyFromStagingToBuffer(mStagingBuffer.Get(), 0, this, 0, GetAllocatedSize()));
    mStagingBuffer = nullptr;

    return GetDevice()->GetDynamicUploader()->OnStagingMemoryFreePendingOnSubmit(
        GetAllocatedSize());
}

void BufferBase::APIUnmap() {
    if (GetDevice()->ConsumedError(ValidateUnmap(), "calling %s.Unmap().", this)) {
        return;
    }
    auto unmap = [&]() -> MaybeError {
        DAWN_TRY(UnmapInternal(false));
        return GetDevice()->GetDynamicUploader()->MaybeSubmitPendingCommands();
    };
    [[maybe_unused]] bool hadError =
        GetDevice()->ConsumedError(unmap(), "calling %s.Unmap().", this);
}

MaybeError BufferBase::Unmap(bool forDestroy) {
    switch (mState.load(std::memory_order::acquire)) {
        case BufferState::Mapped:
            DAWN_TRY(TransitionState(BufferState::Mapped, BufferState::InUse));
            UnmapImpl(BufferState::Mapped,
                      forDestroy ? BufferState::Destroyed : BufferState::Unmapped);
            break;
        case BufferState::MappedAtCreation:
            DAWN_TRY(TransitionState(BufferState::MappedAtCreation, BufferState::InUse));
            if (mStagingBuffer != nullptr) {
                if (forDestroy) {
                    // No need to upload staging contents if the buffer is being destroyed.
                    mStagingBuffer = nullptr;
                } else {
                    DAWN_TRY_WITH_CLEANUP(CopyFromStagingBuffer(), {
                        mState.store(BufferState::MappedAtCreation, std::memory_order::release);
                    });
                }
            }
            if (mSize != 0 && IsCPUWritableAtCreation()) {
                UnmapImpl(BufferState::MappedAtCreation,
                          forDestroy ? BufferState::Destroyed : BufferState::Unmapped);
            }
            break;
        case BufferState::InUse:
            return ConcurrentUseError();
        case BufferState::Unmapped:
            return {};
        case BufferState::SharedMemoryNoAccess:
            break;
        case BufferState::PendingMap:
        case BufferState::Destroyed:
            // UnmapInternal() already handled waiting for PendingMap to be done so there must have
            // been a concurrent operation that changes state between the two atomic loads.
            return ConcurrentUseError();
    }

    mState.store(BufferState::Unmapped, std::memory_order::release);
    return {};
}

MaybeError BufferBase::UnmapInternal(bool forDestroy) {
    BufferState state = mState.load(std::memory_order::acquire);

    // If the buffer is already destroyed, we don't need to do anything.
    if (state == BufferState::Destroyed) {
        return {};
    }

    if (state == BufferState::PendingMap) {
        Ref<MapAsyncEvent> event;
        {
            Mutex::AutoLock lock(&mPendingMapMutex);
            // `mPendingMapEvent` is always reset while holding the mutex. If Complete() ran and
            // already reset the event then map is about to complete. If not, reset here and do an
            // early unmap.
            event = std::move(mPendingMapEvent);
            if (event) {
                // This modifies status in MapAsyncEvent which signals the map has been aborted. It
                // must happen with mutex locked.
                event->UnmapEarly(forDestroy ? "Buffer was destroyed before mapping was resolved."
                                             : "Buffer was unmapped before mapping was resolved.");

                BufferState exchangedState =
                    mState.exchange(BufferState::InUse, std::memory_order::acq_rel);
                DAWN_CHECK(exchangedState == BufferState::PendingMap);
            }
        }

        if (event) {
            // Continue early unmap after releasing the mutex.
            UnmapImpl(BufferState::PendingMap,
                      forDestroy ? BufferState::Destroyed : BufferState::Unmapped);
            mState.store(BufferState::Unmapped, std::memory_order::release);

            GetInstance()->GetEventManager()->SetFutureReady(event.Get());
            return {};
        }

        // Wait until FinalizeMap() finishes before falling through to a regular unmap.
        mState.wait(BufferState::PendingMap, std::memory_order::acquire);
    }

    DAWN_TRY(Unmap(forDestroy));
    return {};
}

MaybeError BufferBase::ValidateMapAsync(wgpu::MapMode mode, size_t offset, size_t size) const {
    DAWN_TRY(GetDevice()->ValidateObject(this));

    DAWN_INVALID_IF(mIsHostMapped, "Host-mapped %s cannot be mapped again.", this);

    DAWN_INVALID_IF(uint64_t(offset) > mSize,
                    "Mapping offset (%u) is larger than the size (%u) of %s.", offset, mSize, this);

    DAWN_INVALID_IF(offset % 8 != 0, "Offset (%u) must be a multiple of 8.", offset);
    DAWN_INVALID_IF(size % 4 != 0, "Size (%u) must be a multiple of 4.", size);

    DAWN_INVALID_IF(uint64_t(size) > mSize - uint64_t(offset),
                    "Mapping range (offset:%u, size: %u) doesn't fit in the size (%u) of %s.",
                    offset, size, mSize, this);

    // If/when we allow multiple map modes at the same time for a map call, relax the restrictions
    // that using a switch/case implicitly implies for the bitmask.
    switch (mode) {
        case wgpu::MapMode::Read: {
            DAWN_INVALID_IF(!(mInternalUsage & wgpu::BufferUsage::MapRead),
                            "The buffer usages (%s) do not contain %s.", mInternalUsage,
                            wgpu::BufferUsage::MapRead);
            break;
        }
        case wgpu::MapMode::Write: {
            DAWN_INVALID_IF(!(mInternalUsage & wgpu::BufferUsage::MapWrite),
                            "The buffer usages (%s) do not contain %s.", mInternalUsage,
                            wgpu::BufferUsage::MapWrite);
            break;
        }
        default:
            return DAWN_VALIDATION_ERROR("Map mode (%s) is not one of %s or %s.", mode,
                                         wgpu::MapMode::Write, wgpu::MapMode::Read);
    }
    return {};
}

bool BufferBase::CanGetMappedRange(bool writable, size_t offset, size_t size) const {
    // Note that:
    //
    //   - We don't check that the device is alive because the application can ask for the
    //     mapped pointer before it knows, and even Dawn knows, that the device was lost, and
    //     still needs to work properly.
    //   - We don't check that the object is alive because we need to return mapped pointers
    //     for error buffers too.

    switch (mState.load(std::memory_order::acquire)) {
        // Writeable Buffer::GetMappedRange is always allowed when mapped at creation.
        case BufferState::MappedAtCreation:
            break;

        case BufferState::Mapped:
            DAWN_ASSERT(bool{mMapMode & wgpu::MapMode::Read} ^
                        bool{mMapMode & wgpu::MapMode::Write});
            if (writable && (mMapMode & wgpu::MapMode::Write) == 0) {
                GetDevice()->EmitLog(
                    wgpu::LoggingType::Error,
                    "GetMappedRange: Mapping is read-only. Use GetConstMappedRange instead.");
                return false;
            }
            break;

        case BufferState::PendingMap:
        case BufferState::Unmapped:
        case BufferState::InUse:
        case BufferState::SharedMemoryNoAccess:
        case BufferState::Destroyed:
            return false;
    }

    if (offset % 8 != 0 || offset < mMapOffset || offset > mSize) {
        return false;
    }

    size_t rangeSize = size == WGPU_WHOLE_MAP_SIZE ? mSize - offset : size;

    if (rangeSize % 4 != 0 || rangeSize > mMapSize) {
        return false;
    }

    size_t offsetInMappedRange = offset - mMapOffset;
    if (offsetInMappedRange > mMapSize - rangeSize) {
        return false;
    }

    return true;
}

MaybeError BufferBase::ValidateUnmap() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_INVALID_IF(mIsHostMapped, "Persistently mapped buffer cannot be unmapped.");
    return {};
}

bool BufferBase::NeedsInitialization() const {
    return !mIsDataInitialized && GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse);
}

void BufferBase::MarkUsedInPendingCommands() {
    // TODO(crbug.com/422741977): Consider storing the pending serial once, perhaps in a
    // CommandRecordingContextBase, so that we don't need to load the atomic value repeatedly.
    MarkUsedInPendingCommands(GetDevice()->GetQueue()->GetPendingCommandSerial());
}

void BufferBase::MarkUsedInPendingCommands(ExecutionSerial pendingSerial) {
    DAWN_ASSERT(!GetDevice()->IsValidationEnabled() ||
                mState.load(std::memory_order::relaxed) == BufferState::InUse);
    DAWN_ASSERT(pendingSerial >= mLastUsageSerial);
    mLastUsageSerial = pendingSerial;
}

ExecutionSerial BufferBase::GetLastUsageSerial() const {
    return mLastUsageSerial;
}

MaybeError BufferBase::UploadData(uint64_t bufferOffset, const void* data, size_t size) {
    if (size == 0) {
        return {};
    }

    return GetDevice()->GetDynamicUploader()->WithUploadReservation(
        size, kCopyBufferToBufferOffsetAlignment, [&](UploadReservation reservation) -> MaybeError {
            memcpy(reservation.mappedPointer, data, size);
            return GetDevice()->CopyFromStagingToBuffer(
                reservation.buffer.Get(), reservation.offsetInBuffer, this, bufferOffset, size);
        });
}

ExecutionSerial BufferBase::OnEndAccess() {
    mState.store(BufferState::SharedMemoryNoAccess, std::memory_order::release);
    ExecutionSerial lastUsageSerial = mLastUsageSerial;
    mLastUsageSerial = kBeginningOfGPUTime;
    return lastUsageSerial;
}

void BufferBase::OnBeginAccess() {
    mState.store(BufferState::Unmapped, std::memory_order::release);
}

bool BufferBase::HasAccess() const {
    return mState.load(std::memory_order::acquire) != BufferState::SharedMemoryNoAccess;
}

bool BufferBase::IsDestroyed() const {
    return mState.load(std::memory_order::acquire) == BufferState::Destroyed;
}

void BufferBase::SetInitialized(bool initialized) {
    mIsDataInitialized = initialized;
}

bool BufferBase::IsInitialized() const {
    return mIsDataInitialized;
}

bool BufferBase::IsFullBufferRange(uint64_t offset, uint64_t size) const {
    return offset == 0 && size == GetSize();
}

void BufferBase::DumpMemoryStatistics(MemoryDump* dump, const char* prefix) const {
    DAWN_ASSERT(IsAlive() && !IsError());
    std::string name = absl::StrFormat("%s/buffer_%p", prefix, static_cast<const void*>(this));
    dump->AddScalar(name.c_str(), MemoryDump::kNameSize, MemoryDump::kUnitsBytes,
                    GetAllocatedSize());
    dump->AddString(name.c_str(), "label", GetLabel());
    dump->AddString(name.c_str(), "usage", absl::StrFormat("%s", GetInternalUsage()));
}

ResultOrError<Ref<TexelBufferViewBase>> BufferBase::CreateTexelView(
    const TexelBufferViewDescriptor* descriptor) {
    DAWN_ASSERT(descriptor != nullptr);
    return GetDevice()->CreateTexelBufferView(this, descriptor);
}

TexelBufferViewBase* BufferBase::APICreateTexelView(const TexelBufferViewDescriptor* descriptor) {
    DeviceBase* device = GetDevice();
    Ref<TexelBufferViewBase> result;
    if (device->ConsumedError(CreateTexelView(descriptor), &result,
                              "calling %s.CreateTexelView(%s).", this, descriptor)) {
        result = TexelBufferViewBase::MakeError(device, descriptor ? descriptor->label : nullptr);
    }
    return ReturnToAPI(std::move(result));
}

ApiObjectList* BufferBase::GetTexelBufferViewTrackingList() {
    return &mTexelBufferViews;
}

MaybeError BufferBase::TransitionState(BufferState currentState, BufferState desiredState) {
    if (mState.compare_exchange_strong(currentState, desiredState, std::memory_order::acq_rel)) {
        return {};
    }

    return ConcurrentUseError();
}

BufferBase::ScopedUseBuffer::ScopedUseBuffer() = default;

BufferBase::ScopedUseBuffer::ScopedUseBuffer(BufferBase* buffer) : mBuffer(buffer) {
    DAWN_ASSERT(mBuffer);
    DAWN_ASSERT(mBuffer->mState.load(std::memory_order::relaxed) == BufferState::InUse);
}

BufferBase::ScopedUseBuffer::~ScopedUseBuffer() {
    if (mBuffer) {
        mBuffer->FinishUse();
    }
}

BufferBase::ScopedUseBuffer::ScopedUseBuffer(ScopedUseBuffer&& other) : mBuffer(other.mBuffer) {
    other.mBuffer = nullptr;
}

BufferBase::ScopedUseBuffer& BufferBase::ScopedUseBuffer::operator=(ScopedUseBuffer&& other) {
    mBuffer = other.mBuffer;
    other.mBuffer = nullptr;
    return *this;
}

void BufferBase::ScopedUseBuffer::Release() {
    DAWN_ASSERT(mBuffer);
    mBuffer = nullptr;
}

}  // namespace dawn::native
