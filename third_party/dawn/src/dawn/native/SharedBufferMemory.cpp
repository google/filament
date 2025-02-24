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

#include "dawn/native/SharedBufferMemory.h"

#include <utility>

#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/Queue.h"

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
    void DestroyImpl() override {}
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

BufferBase* SharedBufferMemoryBase::APICreateBuffer(const BufferDescriptor* descriptor) {
    Ref<BufferBase> result;

    // Provide the defaults if no descriptor is provided.
    BufferDescriptor defaultDescriptor;
    if (descriptor == nullptr) {
        defaultDescriptor = {};
        defaultDescriptor.size = mProperties.size;
        defaultDescriptor.usage = mProperties.usage;
        descriptor = &defaultDescriptor;
    }

    if (GetDevice()->ConsumedError(CreateBuffer(descriptor), &result,
                                   InternalErrorType::OutOfMemory, "calling %s.CreateBuffer(%s).",
                                   this, descriptor)) {
        result = BufferBase::MakeError(GetDevice(), descriptor);
    }
    return ReturnToAPI(std::move(result));
}

ResultOrError<Ref<BufferBase>> SharedBufferMemoryBase::CreateBuffer(
    const BufferDescriptor* rawDescriptor) {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));
    // Validate the buffer descriptor.
    UnpackedPtr<BufferDescriptor> descriptor;
    DAWN_TRY_ASSIGN(descriptor, ValidateBufferDescriptor(GetDevice(), rawDescriptor));

    // Emit a specific error message if the user attempts to create a buffer with Uniform usage.
    DAWN_INVALID_IF(descriptor->usage & wgpu::BufferUsage::Uniform,
                    "The buffer usage (%s) contains (%s), which is not allowed on buffers created "
                    "from SharedBufferMemory.",
                    descriptor->usage, wgpu::BufferUsage::Uniform);

    // Ensure the buffer descriptor usage is a subset of the shared buffer memory's usage.
    DAWN_INVALID_IF(!IsSubset(descriptor->usage, mProperties.usage),
                    "The buffer usage (%s) is incompatible with the SharedBufferMemory usage (%s).",
                    descriptor->usage, mProperties.usage);

    // Validate that the buffer size exactly matches the shared buffer memory's size.
    DAWN_INVALID_IF(descriptor->size != mProperties.size,
                    "SharedBufferMemory size (%u) doesn't match descriptor size (%u).",
                    mProperties.size, descriptor->size);

    Ref<BufferBase> buffer;
    DAWN_TRY_ASSIGN(buffer, CreateBufferImpl(descriptor));
    // Access is not allowed until BeginAccess has been called.
    buffer->OnEndAccess();
    return buffer;
}

void APISharedBufferMemoryEndAccessStateFreeMembers(WGPUSharedBufferMemoryEndAccessState cState) {
    auto* state = reinterpret_cast<SharedBufferMemoryBase::EndAccessState*>(&cState);
    for (size_t i = 0; i < state->fenceCount; ++i) {
        state->fences[i]->APIRelease();
    }
    delete[] state->fences;
    delete[] state->signaledValues;
}

}  // namespace dawn::native
