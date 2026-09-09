// Copyright 2024 The Dawn & Tint Authors
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

#include "src/dawn/native/SharedBufferMemory.h"

#include <memory>
#include <utility>

#include "src/dawn/native/Buffer.h"
#include "src/dawn/native/ChainUtils.h"
#include "src/dawn/native/Device.h"
#include "src/dawn/native/Queue.h"
#include "src/utils/compiler.h"

namespace dawn::native {

namespace {

class ErrorSharedBufferMemory : public SharedBufferMemoryBase {
  public:
    ErrorSharedBufferMemory(DeviceBase* device, const SharedBufferMemoryDescriptor* descriptor)
        : SharedBufferMemoryBase(device, descriptor, ObjectBase::kError) {}

    Ref<SharedResourceMemoryContents> CreateContents() override { DAWN_UNREACHABLE(); }
    ResultOrError<Ref<BufferBase>> CreateBufferImpl(
        const UnpackedPtr<BufferDescriptor>& descriptor) override {
        DAWN_UNREACHABLE();
    }
    MaybeError BeginAccessImpl(BufferBase* buffer,
                               const UnpackedPtr<BeginAccessDescriptor>& descriptor) override {
        DAWN_UNREACHABLE();
    }
    ResultOrError<FenceAndSignalValue> EndAccessImpl(BufferBase* buffer,
                                                     ExecutionSerial lastUsageSerial,
                                                     UnpackedPtr<EndAccessState>& state) override {
        DAWN_UNREACHABLE();
    }
    void DestroyImpl(DestroyReason reason) override {}
};

}  // namespace

// static
SharedBufferMemoryBase* SharedBufferMemoryBase::MakeError(
    DeviceBase* device,
    const SharedBufferMemoryDescriptor* descriptor) {
    return new ErrorSharedBufferMemory(device, descriptor);
}

SharedBufferMemoryBase::SharedBufferMemoryBase(DeviceBase* device,
                                               const SharedBufferMemoryDescriptor* descriptor,
                                               ObjectBase::ErrorTag tag)
    : SharedResourceMemory(device, tag, descriptor->label),
      mProperties{nullptr, wgpu::BufferUsage::None, 0} {}

SharedBufferMemoryBase::SharedBufferMemoryBase(DeviceBase* device,
                                               StringView label,
                                               const SharedBufferMemoryProperties& properties)
    : SharedResourceMemory(device, label), mProperties(properties) {
    GetObjectTrackingList()->Track(this);
}

ObjectType SharedBufferMemoryBase::GetType() const {
    return ObjectType::SharedBufferMemory;
}

bool SharedBufferMemoryBase::CanBeWrittenByCPU() const {
    return (mProperties.usage & wgpu::BufferUsage::MapWrite) != 0;
}

wgpu::Status SharedBufferMemoryBase::APIGetProperties(
    SharedBufferMemoryProperties* properties) const {
    properties->usage = mProperties.usage;
    properties->size = mProperties.size;

    UnpackedPtr<SharedBufferMemoryProperties> unpacked;
    if (GetDevice()->ConsumedError(ValidateAndUnpack(properties), &unpacked,
                                   "calling %s.GetProperties", this)) {
        return wgpu::Status::Error;
    }
    return wgpu::Status::Success;
}

BufferBase* SharedBufferMemoryBase::APICreateBuffer(const BufferDescriptor* rawDescriptor) {
    // Provide the defaults if no descriptor is provided.
    BufferDescriptor defaultDescriptor;
    if (rawDescriptor == nullptr) {
        defaultDescriptor = {};
        defaultDescriptor.size = mProperties.size;

        // The buffers created with default descriptor won't contain buffer mapping usages.
        defaultDescriptor.usage = mProperties.usage & ~kMappableBufferUsages;
        rawDescriptor = &defaultDescriptor;
    }

    // 1. Validate the descriptor and create the buffer (without mapping).
    bool fakeOOMAtNativeMap = false;
    ResultOrError<Ref<BufferBase>> resultOrError = ([&]() -> ResultOrError<Ref<BufferBase>> {
        DAWN_TRY(GetDevice()->ValidateIsAlive());
        DAWN_TRY(GetDevice()->ValidateObject(this));
        // Validate the buffer descriptor.
        UnpackedPtr<BufferDescriptor> descriptor;
        DAWN_TRY_ASSIGN(descriptor, ValidateBufferDescriptor(GetDevice(), rawDescriptor));

        // Ensure the buffer descriptor usage is a subset of the shared buffer memory's usage.
        DAWN_INVALID_IF(
            !IsSubset(descriptor->usage, mProperties.usage),
            "The buffer usage (%s) is incompatible with the SharedBufferMemory usage (%s).",
            descriptor->usage, mProperties.usage);

        // Require MapWrite usage on the shared buffer memory when mappedAtCreation is true.
        DAWN_INVALID_IF(
            descriptor->mappedAtCreation && !CanBeWrittenByCPU(),
            "Buffer created from SharedBufferMemory with mappedAtCreation=true requires "
            "the SharedBufferMemory to have MapWrite usage.");

        // Validate that the buffer size does not exceed the shared buffer memory's size.
        DAWN_INVALID_IF(descriptor->size > mProperties.size,
                        "The buffer size (%u) is larger than SharedBufferMemory size (%u).",
                        descriptor->size, mProperties.size);

        if (auto* ext = descriptor.Get<DawnFakeBufferOOMForTesting>()) {
            fakeOOMAtNativeMap = ext->fakeOOMAtNativeMap;
            if (ext->fakeOOMAtDevice) {
                return DAWN_OUT_OF_MEMORY_ERROR("DawnFakeBufferOOMForTesting fakeOOMAtDevice");
            }
        }

        return CreateBufferImpl(descriptor);
    })();

    // 2. Error handling. Defer descriptor / allocation errors until after we've tried to map, so
    // that errors from mapping take priority. This mirrors DeviceBase::APICreateBuffer.
    Ref<BufferBase> buffer;
    std::unique_ptr<ErrorData> deferredError;
    const bool createSucceeded = resultOrError.IsSuccess();
    if (createSucceeded) [[likely]] {
        buffer = resultOrError.AcquireSuccess();
    } else {
        deferredError = resultOrError.AcquireError();
        buffer = BufferBase::MakeError(GetDevice(), rawDescriptor);
    }

    // 3. Mapping at creation. The buffer may be either valid or an ErrorBuffer.
    if (rawDescriptor->mappedAtCreation) {
        if (!buffer->TryMapAtCreation(fakeOOMAtNativeMap)) {
            // The deferredError is silenced because we drop it here.
            return nullptr;
        }
    }

    // If there was a deferredError saved from earlier, surface it now.
    if (deferredError) {
        std::ignore = GetDevice()->ConsumedError(
            MaybeError(std::move(deferredError)), InternalErrorType::OutOfMemory,
            "calling %s.CreateBuffer(%s).", this, rawDescriptor);
    }

    // Access is not allowed until BeginAccess has been called.
    if (createSucceeded) {
        buffer->OnEndAccess();
    }
    return ReturnToAPI(std::move(buffer));
}

void APISharedBufferMemoryEndAccessStateFreeMembers(WGPUSharedBufferMemoryEndAccessState cState) {
    auto* state = reinterpret_cast<SharedBufferMemoryBase::EndAccessState*>(&cState);
    for (SharedFenceBase* fence : state->fences) {
        fence->APIRelease();
    }
    delete[] state->fences.data();
    delete[] state->signaledValues.data();
}

}  // namespace dawn::native
