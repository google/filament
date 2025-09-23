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

#include "dawn/native/metal/BufferMTL.h"

#include "dawn/common/Math.h"
#include "dawn/common/Platform.h"
#include "dawn/native/CallbackTaskManager.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/metal/CommandRecordingContext.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/QueueMTL.h"
#include "dawn/native/metal/UtilsMetal.h"

#include <limits>

namespace dawn::native::metal {
// The size of uniform buffer and storage buffer need to be aligned to 16 bytes which is the
// largest alignment of supported data types
static constexpr uint32_t kMinUniformOrStorageBufferAlignment = 16u;

// static
ResultOrError<Ref<Buffer>> Buffer::Create(Device* device,
                                          const UnpackedPtr<BufferDescriptor>& descriptor) {
    Ref<Buffer> buffer = AcquireRef(new Buffer(device, descriptor));

    if (auto* hostMappedDesc = descriptor.Get<BufferHostMappedPointer>()) {
        DAWN_TRY(buffer->InitializeHostMapped(hostMappedDesc));
    } else {
        DAWN_TRY(buffer->Initialize(descriptor->mappedAtCreation));
    }
    return std::move(buffer);
}

// static
uint64_t Buffer::QueryMaxBufferLength(id<MTLDevice> mtlDevice) {
        return [mtlDevice maxBufferLength];
}

Buffer::Buffer(DeviceBase* dev, const UnpackedPtr<BufferDescriptor>& desc)
    : BufferBase(dev, desc) {}

MaybeError Buffer::Initialize(bool mappedAtCreation) {
    MTLResourceOptions storageMode;
    if (GetInternalUsage() & kMappableBufferUsages) {
        storageMode = MTLResourceStorageModeShared;
    } else {
        storageMode = MTLResourceStorageModePrivate;
    }

    uint32_t alignment = 1;
#if DAWN_PLATFORM_IS(MACOS)
    // [MTLBlitCommandEncoder fillBuffer] requires the size to be a multiple of 4 on MacOS.
    alignment = 4;
#endif

    // Metal validation layer requires the size of uniform buffer and storage buffer to be no
    // less than the size of the buffer block defined in shader, and the overall size of the
    // buffer must be aligned to the largest alignment of its members.
    if (GetInternalUsage() &
        (wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage | kInternalStorageBuffer)) {
        DAWN_ASSERT(IsAligned(kMinUniformOrStorageBufferAlignment, alignment));
        alignment = kMinUniformOrStorageBufferAlignment;
    }

    // The vertex pulling transform requires at least 4 bytes in the buffer.
    // 0-sized vertex buffer bindings are allowed, so we always need an additional 4 bytes
    // after the end.
    NSUInteger extraBytes = 0u;
    if ((GetInternalUsage() & wgpu::BufferUsage::Vertex) != 0) {
        extraBytes = 4u;
    }

    if (GetSize() > std::numeric_limits<NSUInteger>::max() - extraBytes) {
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }
    NSUInteger currentSize =
        std::max(static_cast<NSUInteger>(GetSize()) + extraBytes, NSUInteger(4));

    if (currentSize > std::numeric_limits<NSUInteger>::max() - alignment) {
        // Alignment would overlow.
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }
    currentSize = Align(currentSize, alignment);

    uint64_t maxBufferSize = QueryMaxBufferLength(ToBackend(GetDevice())->GetMTLDevice());
    if (currentSize > maxBufferSize) {
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }

    mAllocatedSize = currentSize;
    mMtlBuffer.Acquire([ToBackend(GetDevice())->GetMTLDevice() newBufferWithLength:currentSize
                                                                           options:storageMode]);
    if (mMtlBuffer == nullptr) {
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation failed");
    }
    SetLabelImpl();

    // The buffers with mappedAtCreation == true will be initialized in
    // BufferBase::MapAtCreation().
    if (GetDevice()->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting) &&
        !mappedAtCreation) {
        CommandRecordingContext* commandContext =
            ToBackend(GetDevice()->GetQueue())->GetPendingCommandContext();
        ClearBuffer(commandContext, uint8_t(1u));
    }

    // Initialize the padding bytes to zero.
    if (GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse) && !mappedAtCreation) {
        uint32_t paddingBytes = GetAllocatedSize() - GetSize();
        if (paddingBytes > 0) {
            uint32_t clearSize = Align(paddingBytes, 4);
            uint64_t clearOffset = GetAllocatedSize() - clearSize;

            CommandRecordingContext* commandContext =
                ToBackend(GetDevice()->GetQueue())->GetPendingCommandContext();
            ClearBuffer(commandContext, 0, clearOffset, clearSize);
        }
    }
    return {};
}

// static
MaybeError Buffer::InitializeHostMapped(const BufferHostMappedPointer* hostMappedDesc) {
    if (GetSize() > std::numeric_limits<NSUInteger>::max()) {
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }

    mAllocatedSize = GetSize();

    Ref<DeviceBase> deviceRef = GetDevice();
    wgpu::Callback callback = hostMappedDesc->disposeCallback;
    void* userdata = hostMappedDesc->userdata;
    auto dispose = ^(void*, NSUInteger) {
        deviceRef->GetCallbackTaskManager()->AddCallbackTask(
            [callback, userdata] { callback(userdata); });
    };

    mMtlBuffer.Acquire([ToBackend(GetDevice())->GetMTLDevice()
        newBufferWithBytesNoCopy:hostMappedDesc->pointer
                          length:GetSize()
                         options:MTLResourceCPUCacheModeDefaultCache
                     deallocator:dispose]);
    if (mMtlBuffer == nil) {
        dispose(hostMappedDesc->pointer, GetSize());
        return DAWN_INTERNAL_ERROR("Buffer allocation failed");
    }

    // Data is assumed to be initialized since it is externally allocated.
    SetInitialized(true);
    SetLabelImpl();
    return {};
}

Buffer::~Buffer() = default;

id<MTLBuffer> Buffer::GetMTLBuffer() const {
    return mMtlBuffer.Get();
}

bool Buffer::IsCPUWritableAtCreation() const {
    // TODO(enga): Handle CPU-visible memory on UMA
    return GetInternalUsage() & kMappableBufferUsages;
}

MaybeError Buffer::MapAtCreationImpl() {
    return {};
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    CommandRecordingContext* commandContext =
        ToBackend(GetDevice()->GetQueue())->GetPendingCommandContext();
    EnsureDataInitialized(commandContext);

    return {};
}

void* Buffer::GetMappedPointerImpl() {
    return [*mMtlBuffer contents];
}

void Buffer::UnmapImpl() {
    // Nothing to do, Metal StorageModeShared buffers are always mapped.
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
    mMtlBuffer = nullptr;
}

void Buffer::TrackUsage() {
    MarkUsedInPendingCommands();
}

bool Buffer::EnsureDataInitialized(CommandRecordingContext* commandContext) {
    if (!NeedsInitialization()) {
        return false;
    }

    InitializeToZero(commandContext);
    return true;
}

bool Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                uint64_t offset,
                                                uint64_t size) {
    if (!NeedsInitialization()) {
        return false;
    }

    if (IsFullBufferRange(offset, size)) {
        SetInitialized(true);
        return false;
    }

    InitializeToZero(commandContext);
    return true;
}

bool Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                const CopyTextureToBufferCmd* copy) {
    if (!NeedsInitialization()) {
        return false;
    }

    if (IsFullBufferOverwrittenInTextureToBufferCopy(copy)) {
        SetInitialized(true);
        return false;
    }

    InitializeToZero(commandContext);
    return true;
}

void Buffer::InitializeToZero(CommandRecordingContext* commandContext) {
    DAWN_ASSERT(NeedsInitialization());

    ClearBuffer(commandContext, uint8_t(0u));

    SetInitialized(true);
    GetDevice()->IncrementLazyClearCountForTesting();
}

void Buffer::ClearBuffer(CommandRecordingContext* commandContext,
                         uint8_t clearValue,
                         uint64_t offset,
                         uint64_t size) {
    DAWN_ASSERT(commandContext != nullptr);
    size = size > 0 ? size : GetAllocatedSize();
    DAWN_ASSERT(size > 0);
    TrackUsage();
    [commandContext->EnsureBlit() fillBuffer:mMtlBuffer.Get()
                                       range:NSMakeRange(offset, size)
                                       value:clearValue];
}

void Buffer::SetLabelImpl() {
    SetDebugName(GetDevice(), mMtlBuffer.Get(), "Dawn_Buffer", GetLabel());
}

}  // namespace dawn::native::metal
