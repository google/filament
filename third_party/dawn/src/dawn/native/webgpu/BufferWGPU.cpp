// Copyright 2025 The Dawn & Tint Authors
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

#include "src/dawn/native/webgpu/BufferWGPU.h"

#include <algorithm>
#include <string>
#include <utility>

#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/native/Buffer.h"
#include "src/dawn/native/webgpu/CaptureContext.h"
#include "src/dawn/native/webgpu/DeviceWGPU.h"
#include "src/dawn/native/webgpu/QueueWGPU.h"
#include "src/dawn/native/webgpu/Serialization.h"
#include "src/utils/compiler.h"
#include "src/utils/numeric.h"

namespace dawn::native::webgpu {

// static
ResultOrError<Ref<Buffer>> Buffer::Create(Device* device,
                                          const UnpackedPtr<BufferDescriptor>& descriptor) {
    auto actualUsage = ComputeInternalBufferUsages(device, descriptor->usage,
                                                   checked_cast<size_t>(descriptor->size));

    // Make the inner buffer copyable for readback if possible.
    if (!(actualUsage & wgpu::BufferUsage::MapRead)) {
        actualUsage |= wgpu::BufferUsage::CopySrc;
    }

    // Resolve internal usages to regular ones.
    if (actualUsage & kInternalStorageBuffer) {
        actualUsage &= ~kInternalStorageBuffer;
        actualUsage |= wgpu::BufferUsage::Storage;
    }
    if (actualUsage & kReadOnlyStorageBuffer) {
        actualUsage &= ~kReadOnlyStorageBuffer;
        actualUsage |= wgpu::BufferUsage::Storage;
    }
    if (actualUsage & kInternalCopySrcBuffer) {
        actualUsage &= ~kInternalCopySrcBuffer;
        actualUsage |= wgpu::BufferUsage::CopySrc;
    }

    WGPUBufferDescriptor desc = WGPU_BUFFER_DESCRIPTOR_INIT;
    desc.label = ToOutputStringView(descriptor->label);
    desc.usage = ToAPI(actualUsage);
    desc.size = descriptor->size;
    desc.mappedAtCreation = descriptor->mappedAtCreation;

    WGPUBuffer innerBuffer = device->wgpu->deviceCreateBuffer(device->GetInnerHandle(), &desc);
    if (innerBuffer == nullptr) {
        // innerBuffer can be nullptr when mappedAtCreation == true and fails.
        // Return an error buffer.
        const BufferDescriptor* rawDescriptor = *descriptor;
        return ToBackend(BufferBase::MakeError(device, rawDescriptor));
    }

    Ref<Buffer> buffer = AcquireRef(new Buffer(device, descriptor, innerBuffer));
    return std::move(buffer);
}

Buffer::Buffer(Device* device,
               const UnpackedPtr<BufferDescriptor>& descriptor,
               WGPUBuffer innerBuffer)
    : BufferBase(device, descriptor),
      RecordableObject(schema::ObjectType::Buffer),
      ObjectWGPU(device->wgpu->bufferRelease) {
    mInnerHandle = innerBuffer;
    mAllocatedSize = GetSize();
}

bool Buffer::IsCPUWritableAtCreation() const {
    return ToBackend(GetDevice())->wgpu->bufferGetMapState(mInnerHandle) ==
           WGPUBufferMapState_Mapped;
}

MaybeError Buffer::MapAtCreationImpl() {
    mMappedDataOffsetInBuffer = 0;
    void* mappedPointer =
        ToBackend(GetDevice())
            ->wgpu->bufferGetMappedRange(mInnerHandle, 0, checked_cast<size_t>(GetAllocatedSize()));

    DAWN_CHECK(checked_cast<size_t>(GetAllocatedSize()) != WGPU_WHOLE_MAP_SIZE);
    // SAFETY: A non-null pointer returned by GetMappedRange points at `size` valid bytes of data
    // (assuming `size` is not `WGPU_WHOLE_MAP_SIZE`).
    mMappedData = DAWN_UNSAFE_BUFFERS(
        {static_cast<std::byte*>(mappedPointer), checked_cast<size_t>(GetAllocatedSize())});
    return {};
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    // We only map the range requested by the MapAsync call so mMappedData will correspond to an
    // interval in the buffer that can be at an offset from the start.
    mMappedDataOffsetInBuffer = offset;

    auto deviceGuard = GetDevice()->GetGuard();

    struct MapAsyncResult {
        WGPUMapAsyncStatus status;
        std::string message;
    } mapAsyncResult = {};

    WGPUBufferMapCallbackInfo innerCallbackInfo = {};
    innerCallbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
    innerCallbackInfo.callback = [](WGPUMapAsyncStatus status, WGPUStringView message,
                                    void* result_param, void* userdata_param) {
        MapAsyncResult* result = reinterpret_cast<MapAsyncResult*>(result_param);
        result->status = status;
        result->message = ToString(message);
    };
    innerCallbackInfo.userdata1 = &mapAsyncResult;
    innerCallbackInfo.userdata2 = this;

    auto& wgpu = ToBackend(GetDevice())->wgpu.get();

    // TODO(crbug.com/413053623): We do not have a way to efficiently process the async event
    // on the inner webgpu layer. For now we simply wait on the future.
    WGPUFutureWaitInfo waitInfo = {};
    waitInfo.future = wgpu.bufferMapAsync(mInnerHandle, static_cast<WGPUMapMode>(mode), offset,
                                          size, innerCallbackInfo);
    wgpu.instanceWaitAny(ToBackend(GetDevice())->GetInnerInstance(), 1, &waitInfo, UINT64_MAX);

    if (mapAsyncResult.status != WGPUMapAsyncStatus_Success) {
        return DAWN_INTERNAL_ERROR(mapAsyncResult.message);
    }

    // The frontend requires a non-const span but will never write to it when using it for MapRead
    // so cast away the constness.
    void* mappedPointer = nullptr;
    if (bool{mode & wgpu::MapMode::Write}) {
        mappedPointer = wgpu.bufferGetMappedRange(mInnerHandle, offset, size);
    } else if (bool{mode & wgpu::MapMode::Read}) {
        mappedPointer =
            const_cast<void*>(wgpu.bufferGetConstMappedRange(mInnerHandle, offset, size));
    } else {
        DAWN_UNREACHABLE();
    }

    DAWN_CHECK(size != WGPU_WHOLE_MAP_SIZE);
    // SAFETY: A non-null pointer returned by GetMappedRange points at `size` valid bytes of data
    // (assuming `size` is not `WGPU_WHOLE_MAP_SIZE`).
    mMappedData = DAWN_UNSAFE_BUFFERS({static_cast<std::byte*>(mappedPointer), size});
    return {};
}

MaybeError Buffer::FinalizeMapImpl(BufferState newState) {
    return {};
}

Span<std::byte> Buffer::GetMappedRangeImpl(size_t offset, size_t size) {
    DAWN_ASSERT(offset >= mMappedDataOffsetInBuffer);
    return mMappedData.subspan(offset - mMappedDataOffsetInBuffer, size);
}

void Buffer::UnmapImpl(BufferState oldState, BufferState newState) {
    auto deviceGuard = GetDevice()->GetGuard();

    if (IsMappedState(oldState) && MapMode() == wgpu::MapMode::Write &&
        newState != BufferState::Destroyed) {
        // TODO(477349135): Optimize this by tracking the ranges. As it is we'll
        // capture the entire buffer even if only a few bytes were updated. Instead
        // of mNeedsCapture we could have mDirtySpans. When size is 0 there's nothing to do.
        mNeedsCapture = true;
    }

    if (mInnerHandle) {
        ToBackend(GetDevice())->wgpu->bufferUnmap(mInnerHandle);
    }
    mMappedData = {};
}

void Buffer::DestroyImpl(DestroyReason reason) {
    BufferBase::DestroyImpl(reason);
    auto& wgpu = ToBackend(GetDevice())->wgpu.get();
    wgpu.bufferDestroy(mInnerHandle);
}

void Buffer::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError Buffer::AddReferenced(CaptureContext& captureContext) {
    // Buffers do not reference other objects.
    return {};
}

MaybeError Buffer::CaptureCreationParameters(CaptureContext& captureContext) {
    schema::Buffer buf{{
        .size = GetSize(),
        .usage = GetUsage(),
    }};
    Serialize(captureContext, buf);
    return {};
}

MaybeError Buffer::CaptureContentIfNeeded(CaptureContext& captureContext,
                                          schema::ObjectId id,
                                          bool newResource) {
    // TODO(451338754): If it's a new resource and we know the buffer is all zero then don't
    // capture.
    wgpu::BufferUsage usage = GetUsage();
    if (!mNeedsCapture && !newResource) {
        return {};
    }

    // A MapRead buffer is never used as input since it's only allowed CopyDst
    // so we don't need its contents.
    if (usage & wgpu::BufferUsage::MapRead) {
        return {};
    }

    mNeedsCapture = false;

    return AddContentToCapture(captureContext);
}

// TODO(451650604): We currently get at most 1mb at a time to keep memory usage down.
// Revisit for speed later.
MaybeError Buffer::AddContentToCapture(CaptureContext& captureContext) {
    // TODO(473593119): Handle the unaligned trailing bytes.
    // TODO(473568230): Support copies with unaligned size.
    // copyBufferToBuffer requires 4 byte alignment for both size and offset which prevents
    // us from copying the trailing bytes. writeBuffer has the same alignment requirements.
    // so the user can't put bytes in via writeBuffer. mapAsync requires offset to be 8 byte
    // aligned and size to be 4 bytes so the user can not set those last bytes with mapAsync.
    // We can still access those bytes with copyBufferToTexture and copyTextureToBuffer though.
    // For now, we just ignore the last 3 bytes.
    uint64_t copyableSize = AlignDown(GetSize(), 4);
    if (copyableSize == 0) {
        return {};
    }

    struct MapAsyncResult {
        WGPUMapAsyncStatus status;
        std::string message;
    } mapAsyncResult = {};

    schema::RootCommandWriteBufferCmd cmd{{
        .data = {{
            .bufferId = captureContext.GetId(this),
            .bufferOffset = 0,
            .size = copyableSize,
        }},
    }};
    Serialize(captureContext, cmd);

    WGPUBuffer srcBuffer = GetInnerHandle();
    WGPUBuffer copyBuffer = captureContext.GetCopyBuffer();
    WGPUQueue queue = ToBackend(GetDevice()->GetQueue())->GetInnerHandle();

    Device* device = ToBackend(GetDevice());
    WGPUDevice innerDevice = device->GetInnerHandle();
    auto& wgpu = device->wgpu.get();

    CaptureContext::ScopedContentWriter writer(captureContext);
    for (uint64_t offset = 0; offset < copyableSize; offset += CaptureContext::kCopyBufferSize) {
        uint64_t copySize = std::min(CaptureContext::kCopyBufferSize, copyableSize - offset);

        WGPUCommandEncoder encoder = wgpu.deviceCreateCommandEncoder(innerDevice, nullptr);
        wgpu.commandEncoderCopyBufferToBuffer(encoder, srcBuffer, offset, copyBuffer, 0, copySize);
        WGPUCommandBuffer commandBuffer = wgpu.commandEncoderFinish(encoder, nullptr);
        wgpu.queueSubmit(queue, 1, &commandBuffer);
        wgpu.commandBufferRelease(commandBuffer);
        wgpu.commandEncoderRelease(encoder);

        // Map the buffer to read back the content.
        WGPUBufferMapCallbackInfo innerCallbackInfo = {};
        innerCallbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
        innerCallbackInfo.callback = [](WGPUMapAsyncStatus status, WGPUStringView message,
                                        void* result_param, void* userdata_param) {
            MapAsyncResult* result = reinterpret_cast<MapAsyncResult*>(result_param);
            result->status = status;
            result->message = ToString(message);
        };
        innerCallbackInfo.userdata1 = &mapAsyncResult;
        innerCallbackInfo.userdata2 = this;

        // We read this back synchronously. I'm not sure we could do much more.
        WGPUFutureWaitInfo waitInfo = {};
        waitInfo.future = wgpu.bufferMapAsync(copyBuffer, WGPUMapMode_Read, 0,
                                              checked_cast<size_t>(copySize), innerCallbackInfo);
        wgpu.instanceWaitAny(device->GetInnerInstance(), 1, &waitInfo, UINT64_MAX);

        DAWN_ASSERT(mapAsyncResult.status == WGPUMapAsyncStatus_Success);

        if (mapAsyncResult.status != WGPUMapAsyncStatus_Success) {
            return DAWN_INTERNAL_ERROR(mapAsyncResult.message);
        }

        const void* data =
            wgpu.bufferGetConstMappedRange(copyBuffer, 0, checked_cast<size_t>(copySize));
        writer.WriteContentBytes(data, checked_cast<size_t>(copySize));
        wgpu.bufferUnmap(copyBuffer);
    }

    return {};
}

}  // namespace dawn::native::webgpu
