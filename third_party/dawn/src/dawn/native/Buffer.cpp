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
#include "dawn/common/StringViewUtils.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/CallbackTaskManager.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/EventManager.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/Queue.h"
#include "dawn/native/SystemEvent.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

namespace {
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

    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override {
        DAWN_UNREACHABLE();
    }

    void* GetMappedPointer() override { return mFakeMappedData.get(); }

    void UnmapImpl() override { mFakeMappedData.reset(); }

    std::unique_ptr<uint8_t[]> mFakeMappedData;
};

// GetMappedRange on a zero-sized buffer returns a pointer to this value.
static uint32_t sZeroSizedMappingData = 0xCAFED00D;

}  // anonymous namespace

wgpu::BufferUsage ComputeInternalBufferUsages(const DeviceBase* device,
                                              wgpu::BufferUsage usage,
                                              size_t bufferSize) {
    // Add readonly storage usage if the buffer has a storage usage. The validation rules in
    // ValidateSyncScopeResourceUsage will make sure we don't use both at the same time.
    if (usage & wgpu::BufferUsage::Storage) {
        usage |= kReadOnlyStorageBuffer;
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

struct BufferBase::MapAsyncEvent final : public EventManager::TrackedEvent {
    // MapAsyncEvent stores a raw pointer to the buffer so that it can update the buffer's map state
    // when it completes. If the map completes early (error, unmap, destroy), then the buffer is no
    // longer needed and we store the early status instead. The raw pointer is safe because the
    // early status is set to destroyed before the buffer is dropped. Note: this could be an atomic
    // + spin lock on a sentinel enum if the mutex cost is high.
    struct BufferErrorData {
        WGPUMapAsyncStatus status;
        std::string message;
    };
    MutexProtected<std::variant<BufferBase*, BufferErrorData>> mBufferOrError;

    WGPUBufferMapCallback mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    // Create an event backed by the given queue execution serial.
    MapAsyncEvent(DeviceBase* device,
                  BufferBase* buffer,
                  const WGPUBufferMapCallbackInfo& callbackInfo,
                  ExecutionSerial serial)
        : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode),
                       device->GetQueue(),
                       serial),
          mBufferOrError(buffer),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2) {
        TRACE_EVENT_ASYNC_BEGIN0(device->GetPlatform(), General, "Buffer::APIMapAsync",
                                 uint64_t(serial));
    }

    // Create an event that's ready at creation (for errors, etc.)
    MapAsyncEvent(DeviceBase* device,
                  const WGPUBufferMapCallbackInfo& callbackInfo,
                  const std::string& message,
                  WGPUMapAsyncStatus status)
        : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode),
                       SystemEvent::CreateSignaled()),
          mBufferOrError(BufferErrorData{status, message}),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2) {
        TRACE_EVENT_ASYNC_BEGIN0(device->GetPlatform(), General, "Buffer::APIMapAsync",
                                 uint64_t(kBeginningOfGPUTime));
    }

    ~MapAsyncEvent() override { EnsureComplete(EventCompletionType::Shutdown); }

    void Complete(EventCompletionType completionType) override {
        if (const auto* queueAndSerial = std::get_if<QueueAndSerial>(&GetCompletionData())) {
            TRACE_EVENT_ASYNC_END0(queueAndSerial->queue->GetDevice()->GetPlatform(), General,
                                   "Buffer::APIMapAsync",
                                   uint64_t(queueAndSerial->completionSerial));
        }

        void* userdata1 = mUserdata1.ExtractAsDangling();
        void* userdata2 = mUserdata2.ExtractAsDangling();

        if (completionType == EventCompletionType::Shutdown) {
            mCallback(WGPUMapAsyncStatus_CallbackCancelled,
                      ToOutputStringView("A valid external Instance reference no longer exists."),
                      userdata1, userdata2);
            return;
        }

        bool error = false;
        BufferErrorData pendingErrorData;
        Ref<MapAsyncEvent> pendingMapEvent;

        // Lock the buffer / error. This may race with UnmapEarly which occurs when the buffer is
        // unmapped or destroyed.
        mBufferOrError.Use([&](auto bufferOrError) {
            if (auto* errorData = std::get_if<BufferErrorData>(&*bufferOrError)) {
                // Assign the early error, if it was set.
                pendingErrorData = *errorData;
                error = true;
            } else if (auto** buffer = std::get_if<BufferBase*>(&*bufferOrError)) {
                // Set the buffer state to Mapped if this pending map succeeded.
                // TODO(crbug.com/dawn/831): in order to be thread safe, mutation of the
                // state and pending map event needs to be atomic w.r.t. UnmapInternal.
                DAWN_ASSERT((*buffer)->mState == BufferState::PendingMap);
                (*buffer)->mState = BufferState::Mapped;

                pendingMapEvent = std::move((*buffer)->mPendingMapEvent);
            }
        });
        if (error) {
            DAWN_ASSERT(!pendingErrorData.message.empty());
            mCallback(pendingErrorData.status, ToOutputStringView(pendingErrorData.message),
                      userdata1, userdata2);
        } else {
            mCallback(WGPUMapAsyncStatus_Success, kEmptyOutputStringView, userdata1, userdata2);
        }
    }

    // Set the buffer early status because it was unmapped early due to Unmap or Destroy.
    // This can race with Complete such that the early status is ignored, but this is OK
    // because we will still unmap the buffer. It will be as-if the application called
    // Unmap/Destroy just after the map event completed.
    void UnmapEarly(WGPUMapAsyncStatus status, std::string_view message) {
        mBufferOrError.Use([&](auto bufferOrError) {
            *bufferOrError = BufferErrorData{status, std::string(message)};
        });
    }
};

ResultOrError<UnpackedPtr<BufferDescriptor>> ValidateBufferDescriptor(
    DeviceBase* device,
    const BufferDescriptor* descriptor) {
    UnpackedPtr<BufferDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    DAWN_TRY(ValidateBufferUsage(descriptor->usage));

    if (const auto* hostMappedDesc = unpacked.Get<BufferHostMappedPointer>()) {
        // TODO(crbug.com/dawn/2018): Properly expose this limit.
        uint32_t requiredAlignment = 4096;
        if (device->GetAdapter()->GetPhysicalDevice()->GetBackendType() ==
            wgpu::BackendType::D3D12) {
            requiredAlignment = 65536;
        }

        DAWN_INVALID_IF(!device->HasFeature(Feature::HostMappedPointer), "%s requires %s.",
                        hostMappedDesc->sType, ToAPI(Feature::HostMappedPointer));
        DAWN_INVALID_IF(!IsAligned(descriptor->size, requiredAlignment),
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

BufferBase::BufferBase(DeviceBase* device, const UnpackedPtr<BufferDescriptor>& descriptor)
    : SharedResource(device, descriptor->label),
      mSize(descriptor->size),
      mUsage(descriptor->usage),
      mInternalUsage(ComputeInternalBufferUsages(device, descriptor->usage, descriptor->size)),
      mState(descriptor.Get<BufferHostMappedPointer>() ? BufferState::HostMappedPersistent
                                                       : BufferState::Unmapped) {
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
    DAWN_ASSERT(mState == BufferState::Unmapped || mState == BufferState::Destroyed ||
                // Happens if the buffer was created mappedAtCreation *after* device destroy.
                // TODO(crbug.com/42241190): This shouldn't be needed once the issue above is fixed,
                // because then mState will just be Destroyed.
                (mState == BufferState::MappedAtCreation &&
                 GetDevice()->GetState() == DeviceBase::State::Destroyed));
}

void BufferBase::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the buffer is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the buffer.
    // - It may be called when the last ref to the buffer is dropped and the buffer
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the buffer since there are no other live refs.
    if (mState == BufferState::Mapped || mState == BufferState::PendingMap) {
        UnmapInternal(WGPUMapAsyncStatus_Aborted,
                      "Buffer was destroyed before mapping was resolved.");
    } else if (mState == BufferState::MappedAtCreation) {
        if (mStagingBuffer != nullptr) {
            mStagingBuffer = nullptr;
        } else if (mSize != 0) {
            UnmapInternal(WGPUMapAsyncStatus_Aborted,
                          "Buffer was destroyed before mapping was resolved.");
        }
    }

    mState = BufferState::Destroyed;
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
    switch (mState) {
        case BufferState::Mapped:
        case BufferState::MappedAtCreation:
            return wgpu::BufferMapState::Mapped;
        case BufferState::PendingMap:
            return wgpu::BufferMapState::Pending;
        case BufferState::Unmapped:
        case BufferState::Destroyed:
        case BufferState::SharedMemoryNoAccess:
            return wgpu::BufferMapState::Unmapped;
        case BufferState::HostMappedPersistent:
            DAWN_UNREACHABLE();
    }
}

MaybeError BufferBase::MapAtCreation() {
    DAWN_TRY(MapAtCreationInternal());

    void* ptr;
    size_t size;
    if (mSize == 0) {
        return {};
    } else if (mStagingBuffer != nullptr) {
        // If there is a staging buffer for initialization, clear its contents directly.
        // It should be exactly as large as the buffer allocation.
        ptr = mStagingBuffer->GetMappedPointer();
        size = mStagingBuffer->GetSize();
        DAWN_ASSERT(size == GetAllocatedSize());
    } else {
        // Otherwise, the buffer is directly mappable on the CPU.
        ptr = GetMappedPointer();
        size = GetAllocatedSize();
    }

    DeviceBase* device = GetDevice();
    if (device->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse) &&
        !device->IsToggleEnabled(Toggle::DisableLazyClearForMappedAtCreationBuffer)) {
        // The staging buffer is created with `MappedAtCreation == true` so we don't need to clear
        // it again.
        if (mStagingBuffer != nullptr) {
            DAWN_ASSERT(!mStagingBuffer->NeedsInitialization());
        } else {
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

MaybeError BufferBase::MapAtCreationInternal() {
    DAWN_ASSERT(mState == BufferState::Unmapped);

    mMapOffset = 0;
    mMapSize = mSize;

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
            DAWN_TRY_ASSIGN(mStagingBuffer, GetDevice()->CreateBuffer(&stagingBufferDesc));
        }
    }

    // Only set the state to mapped at creation if we did no fail any point in this helper.
    // Otherwise, if we override the default unmapped state before succeeding to create a
    // staging buffer, we will have issues when we try to destroy the buffer.
    mState = BufferState::MappedAtCreation;
    return {};
}

MaybeError BufferBase::ValidateCanUseOnQueueNow() const {
    DAWN_ASSERT(!IsError());

    switch (mState) {
        case BufferState::Destroyed:
            return DAWN_VALIDATION_ERROR("%s used in submit while destroyed.", this);
        case BufferState::Mapped:
        case BufferState::MappedAtCreation:
            return DAWN_VALIDATION_ERROR("%s used in submit while mapped.", this);
        case BufferState::PendingMap:
            return DAWN_VALIDATION_ERROR("%s used in submit while pending map.", this);
        case BufferState::SharedMemoryNoAccess:
            return DAWN_VALIDATION_ERROR("%s used in submit without shared memory access.", this);
        case BufferState::HostMappedPersistent:
        case BufferState::Unmapped:
            return {};
    }
    DAWN_UNREACHABLE();
}

Future BufferBase::APIMapAsync(wgpu::MapMode mode,
                               size_t offset,
                               size_t size,
                               const WGPUBufferMapCallbackInfo& callbackInfo) {
    // TODO(crbug.com/dawn/2052): Once we always return a future, change this to log to the instance
    // (note, not raise a validation error to the device) and return the null future.
    DAWN_ASSERT(callbackInfo.nextInChain == nullptr);

    Ref<EventManager::TrackedEvent> event;
    {
        // TODO(crbug.com/dawn/831) Manually acquire device lock instead of relying on code-gen for
        // re-entrancy.
        auto deviceLock(GetDevice()->GetScopedLock());

        // Handle the defaulting of size required by WebGPU, even if in webgpu_cpp.h it is not
        // possible to default the function argument (because there is the callback later in the
        // argument list)
        if ((size == wgpu::kWholeMapSize) && (offset <= mSize)) {
            size = mSize - offset;
        }

        WGPUMapAsyncStatus status = WGPUMapAsyncStatus_Error;
        MaybeError maybeError = [&]() -> MaybeError {
            DAWN_INVALID_IF(mState == BufferState::PendingMap,
                            "%s already has an outstanding map pending.", this);
            DAWN_TRY(ValidateMapAsync(mode, offset, size, &status));
            DAWN_TRY(MapAsyncImpl(mode, offset, size));
            return {};
        }();

        if (maybeError.IsError()) {
            auto error = maybeError.AcquireError();
            event = AcquireRef(
                new MapAsyncEvent(GetDevice(), callbackInfo, error->GetMessage(), status));
            [[maybe_unused]] bool hadError = GetDevice()->ConsumedError(
                std::move(error), "calling %s.MapAsync(%s, %u, %u, ...).", this, mode, offset,
                size);
        } else {
            mMapMode = mode;
            mMapOffset = offset;
            mMapSize = size;
            mState = BufferState::PendingMap;
            mPendingMapEvent =
                AcquireRef(new MapAsyncEvent(GetDevice(), this, callbackInfo, mLastUsageSerial));
            event = mPendingMapEvent;
        }
    }

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

void* BufferBase::GetMappedRange(size_t offset, size_t size, bool writable) {
    if (!CanGetMappedRange(writable, offset, size)) {
        return nullptr;
    }

    if (mStagingBuffer != nullptr) {
        return static_cast<uint8_t*>(mStagingBuffer->GetMappedPointer()) + offset;
    }
    if (mSize == 0) {
        return &sZeroSizedMappingData;
    }
    uint8_t* start = static_cast<uint8_t*>(GetMappedPointer());
    return start == nullptr ? nullptr : start + offset;
}

void BufferBase::APIDestroy() {
    Destroy();
}

uint64_t BufferBase::APIGetSize() const {
    return mSize;
}

MaybeError BufferBase::CopyFromStagingBuffer() {
    DAWN_ASSERT(mStagingBuffer != nullptr && mSize != 0);

    DAWN_TRY(
        GetDevice()->CopyFromStagingToBuffer(mStagingBuffer.Get(), 0, this, 0, GetAllocatedSize()));

    mStagingBuffer = nullptr;

    return {};
}

void BufferBase::APIUnmap() {
    if (GetDevice()->ConsumedError(ValidateUnmap(), "calling %s.Unmap().", this)) {
        return;
    }
    [[maybe_unused]] bool hadError =
        GetDevice()->ConsumedError(Unmap(), "calling %s.Unmap().", this);
}

MaybeError BufferBase::Unmap() {
    if (mState == BufferState::Destroyed) {
        return {};
    }

    // Make sure writes are now visible to the GPU if we used a staging buffer.
    if (mState == BufferState::MappedAtCreation && mStagingBuffer != nullptr) {
        DAWN_TRY(CopyFromStagingBuffer());
    }
    UnmapInternal(WGPUMapAsyncStatus_Aborted, "Buffer was unmapped before mapping was resolved.");
    return {};
}

void BufferBase::UnmapInternal(WGPUMapAsyncStatus status, std::string_view message) {
    // Unmaps resources on the backend.
    switch (mState) {
        case BufferState::PendingMap: {
            // TODO(crbug.com/dawn/831): in order to be thread safe, mutation of the
            // state and pending map event needs to be atomic w.r.t. MapAsyncEvent::Complete.
            Ref<MapAsyncEvent> pendingMapEvent = std::move(mPendingMapEvent);
            pendingMapEvent->UnmapEarly(status, message);
            GetInstance()->GetEventManager()->SetFutureReady(pendingMapEvent.Get());
            UnmapImpl();
        } break;
        case BufferState::Mapped:
            UnmapImpl();
            break;
        case BufferState::MappedAtCreation:
            if (mSize != 0 && IsCPUWritableAtCreation()) {
                UnmapImpl();
            }
            break;
        case BufferState::Unmapped:
        case BufferState::HostMappedPersistent:
        case BufferState::SharedMemoryNoAccess:
            break;
        case BufferState::Destroyed:
            DAWN_UNREACHABLE();
    }

    mState = BufferState::Unmapped;
}

MaybeError BufferBase::ValidateMapAsync(wgpu::MapMode mode,
                                        size_t offset,
                                        size_t size,
                                        WGPUMapAsyncStatus* status) const {
    *status = WGPUMapAsyncStatus_Aborted;
    DAWN_TRY(GetDevice()->ValidateIsAlive());

    *status = WGPUMapAsyncStatus_Error;
    DAWN_TRY(GetDevice()->ValidateObject(this));

    DAWN_INVALID_IF(uint64_t(offset) > mSize,
                    "Mapping offset (%u) is larger than the size (%u) of %s.", offset, mSize, this);

    DAWN_INVALID_IF(offset % 8 != 0, "Offset (%u) must be a multiple of 8.", offset);
    DAWN_INVALID_IF(size % 4 != 0, "Size (%u) must be a multiple of 4.", size);

    DAWN_INVALID_IF(uint64_t(size) > mSize - uint64_t(offset),
                    "Mapping range (offset:%u, size: %u) doesn't fit in the size (%u) of %s.",
                    offset, size, mSize, this);

    switch (mState) {
        case BufferState::Mapped:
        case BufferState::MappedAtCreation:
            return DAWN_VALIDATION_ERROR("%s is already mapped.", this);
        case BufferState::PendingMap:
            DAWN_UNREACHABLE();
        case BufferState::Destroyed:
            return DAWN_VALIDATION_ERROR("%s is destroyed.", this);
        case BufferState::HostMappedPersistent:
            return DAWN_VALIDATION_ERROR("Host-mapped %s cannot be mapped again.", this);
        case BufferState::SharedMemoryNoAccess:
            return DAWN_VALIDATION_ERROR("%s used without shared memory access.", this);
        case BufferState::Unmapped:
            break;
    }

    bool isReadMode = mode & wgpu::MapMode::Read;
    bool isWriteMode = mode & wgpu::MapMode::Write;
    DAWN_INVALID_IF(!(isReadMode ^ isWriteMode), "Map mode (%s) is not one of %s or %s.", mode,
                    wgpu::MapMode::Write, wgpu::MapMode::Read);

    if (mode & wgpu::MapMode::Read) {
        DAWN_INVALID_IF(!(mInternalUsage & wgpu::BufferUsage::MapRead),
                        "The buffer usages (%s) do not contain %s.", mInternalUsage,
                        wgpu::BufferUsage::MapRead);
    } else {
        DAWN_ASSERT(mode & wgpu::MapMode::Write);
        DAWN_INVALID_IF(!(mInternalUsage & wgpu::BufferUsage::MapWrite),
                        "The buffer usages (%s) do not contain %s.", mInternalUsage,
                        wgpu::BufferUsage::MapWrite);
    }

    *status = WGPUMapAsyncStatus_Success;
    return {};
}

bool BufferBase::CanGetMappedRange(bool writable, size_t offset, size_t size) const {
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

    // Note that:
    //
    //   - We don't check that the device is alive because the application can ask for the
    //     mapped pointer before it knows, and even Dawn knows, that the device was lost, and
    //     still needs to work properly.
    //   - We don't check that the object is alive because we need to return mapped pointers
    //     for error buffers too.

    switch (mState) {
        // It is never valid to call GetMappedRange on a host-mapped buffer.
        // TODO(crbug.com/dawn/2018): consider returning the same pointer here.
        case BufferState::HostMappedPersistent:
            return false;

        // Writeable Buffer::GetMappedRange is always allowed when mapped at creation.
        case BufferState::MappedAtCreation:
            return true;

        case BufferState::Mapped:
            DAWN_ASSERT(bool{mMapMode & wgpu::MapMode::Read} ^
                        bool{mMapMode & wgpu::MapMode::Write});
            return !writable || (mMapMode & wgpu::MapMode::Write);

        case BufferState::PendingMap:
        case BufferState::Unmapped:
        case BufferState::SharedMemoryNoAccess:
        case BufferState::Destroyed:
            return false;
    }
    DAWN_UNREACHABLE();
}

MaybeError BufferBase::ValidateUnmap() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_INVALID_IF(mState == BufferState::HostMappedPersistent,
                    "Persistently mapped buffer cannot be unmapped.");
    return {};
}

bool BufferBase::NeedsInitialization() const {
    return !mIsDataInitialized && GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse);
}

void BufferBase::MarkUsedInPendingCommands() {
    ExecutionSerial serial = GetDevice()->GetQueue()->GetPendingCommandSerial();
    DAWN_ASSERT(serial >= mLastUsageSerial);
    mLastUsageSerial = serial;
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
    mState = BufferState::SharedMemoryNoAccess;
    ExecutionSerial lastUsageSerial = mLastUsageSerial;
    mLastUsageSerial = kBeginningOfGPUTime;
    return lastUsageSerial;
}

void BufferBase::OnBeginAccess() {
    mState = BufferState::Unmapped;
}

bool BufferBase::HasAccess() const {
    return mState != BufferState::SharedMemoryNoAccess;
}

bool BufferBase::IsDestroyed() const {
    return mState == BufferState::Destroyed;
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

}  // namespace dawn::native
