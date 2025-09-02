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

#include "dawn/native/webgpu/BufferWGPU.h"

#include <string>
#include <utility>

#include "dawn/common/StringViewUtils.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/QueueWGPU.h"

namespace dawn::native::webgpu {

// static
ResultOrError<Ref<Buffer>> Buffer::Create(Device* device,
                                          const UnpackedPtr<BufferDescriptor>& descriptor) {
    auto desc = ToAPI(*descriptor);
    WGPUBuffer innerBuffer = device->wgpu.deviceCreateBuffer(device->GetInnerHandle(), desc);
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
    : BufferBase(device, descriptor), ObjectWGPU(device->wgpu.bufferRelease) {
    mInnerHandle = innerBuffer;
    mAllocatedSize = GetSize();
}

bool Buffer::IsCPUWritableAtCreation() const {
    return ToBackend(GetDevice())->wgpu.bufferGetMapState(mInnerHandle) ==
           WGPUBufferMapState_Mapped;
}

MaybeError Buffer::MapAtCreationImpl() {
    mMappedData = ToBackend(GetDevice())->wgpu.bufferGetMappedRange(mInnerHandle, 0, GetSize());
    return {};
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
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

    auto& wgpu = ToBackend(GetDevice())->wgpu;

    // TODO(crbug.com/413053623): We do not have a way to efficiently process the async event
    // on the inner webgpu layer. For now we simply wait on the future.
    WGPUFutureWaitInfo waitInfo = {};
    waitInfo.future = wgpu.bufferMapAsync(mInnerHandle, static_cast<WGPUMapMode>(mode), offset,
                                          size, innerCallbackInfo);
    wgpu.instanceWaitAny(ToBackend(GetDevice())->GetInnerInstance(), 1, &waitInfo, UINT64_MAX);

    if (mapAsyncResult.status != WGPUMapAsyncStatus_Success) {
        return DAWN_INTERNAL_ERROR(mapAsyncResult.message);
    }

    // The frontend asks that the pointer returned by GetMappedPointer is from the start of
    // the resource but WGPU gives us the pointer at offset. Remove the offset.
    if (bool{mode & wgpu::MapMode::Write}) {
        mMappedData =
            static_cast<uint8_t*>(wgpu.bufferGetMappedRange(mInnerHandle, offset, size)) - offset;
    } else if (bool{mode & wgpu::MapMode::Read}) {
        mMappedData = static_cast<uint8_t*>(const_cast<void*>(
                          wgpu.bufferGetConstMappedRange(mInnerHandle, offset, size))) -
                      offset;
    } else {
        DAWN_UNREACHABLE();
    }
    return {};
}

void* Buffer::GetMappedPointerImpl() {
    // The mapping offset has already been removed.
    return mMappedData;
}

void Buffer::UnmapImpl() {
    if (mInnerHandle) {
        ToBackend(GetDevice())->wgpu.bufferUnmap(mInnerHandle);
    }
    mMappedData = nullptr;
}

}  // namespace dawn::native::webgpu
