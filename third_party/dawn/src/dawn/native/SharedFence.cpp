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

#include "dawn/native/SharedFence.h"

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

namespace {

class ErrorSharedFence : public SharedFenceBase {
  public:
    ErrorSharedFence(DeviceBase* device, const SharedFenceDescriptor* descriptor)
        : SharedFenceBase(device, descriptor, ObjectBase::kError) {}

    MaybeError ExportInfoImpl(UnpackedPtr<SharedFenceExportInfo>& info) const override {
        DAWN_UNREACHABLE();
    }
};

}  // namespace

// static
Ref<SharedFenceBase> SharedFenceBase::MakeError(DeviceBase* device,
                                                const SharedFenceDescriptor* descriptor) {
    return AcquireRef(new ErrorSharedFence(device, descriptor));
}

SharedFenceBase::SharedFenceBase(DeviceBase* device,
                                 const SharedFenceDescriptor* descriptor,
                                 ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag, descriptor->label) {}

SharedFenceBase::SharedFenceBase(DeviceBase* device, StringView label)
    : ApiObjectBase(device, label) {}

ObjectType SharedFenceBase::GetType() const {
    return ObjectType::SharedFence;
}

void SharedFenceBase::APIExportInfo(SharedFenceExportInfo* info) const {
    [[maybe_unused]] bool hadError = GetDevice()->ConsumedError(ExportInfo(info));
}

void SharedFenceBase::DestroyImpl() {}

MaybeError SharedFenceBase::ExportInfo(SharedFenceExportInfo* info) const {
    // Set the type to 0. It will be overwritten to the actual type
    // as long as no error occurs.
    info->type = wgpu::SharedFenceType(0);

    DAWN_TRY(GetDevice()->ValidateObject(this));

    UnpackedPtr<SharedFenceExportInfo> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(info));
    return ExportInfoImpl(unpacked);
}

}  // namespace dawn::native
