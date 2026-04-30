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

#include "dawn/native/webgpu/ShaderModuleWGPU.h"

#include <utility>

#include "dawn/native/ShaderModule.h"
#include "dawn/native/webgpu/DeviceWGPU.h"

namespace dawn::native::webgpu {

// static
ResultOrError<Ref<ShaderModule>> ShaderModule::Create(
    Device* device,
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions) {
    auto desc = ToAPI(*descriptor);
    WGPUShaderModule innerShaderModule =
        device->wgpu->deviceCreateShaderModule(device->GetInnerHandle(), desc);
    DAWN_ASSERT(innerShaderModule);

    Ref<ShaderModule> shader =
        AcquireRef(new ShaderModule(device, descriptor, internalExtensions, innerShaderModule));
    shader->Initialize();
    return shader;
}

ShaderModule::ShaderModule(Device* device,
                           const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                           std::vector<tint::wgsl::Extension> internalExtensions,
                           WGPUShaderModule innerShaderModule)
    : ShaderModuleBase(device, descriptor, std::move(internalExtensions)),
      RecordableObject(schema::ObjectType::ShaderModule),
      ObjectWGPU(device->wgpu->shaderModuleRelease) {
    mInnerHandle = innerShaderModule;

    // TODO(452840621): Make this use a chain instead of hard coded to WGSL only and handle other
    // chained structs. We need to save the entire chain here for serialization later.
    if (auto* wgslDesc = descriptor.Get<ShaderSourceWGSL>()) {
        mCode = wgslDesc->code;
    }
}

void ShaderModule::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError ShaderModule::AddReferenced(CaptureContext& captureContext) {
    return {};
}

MaybeError ShaderModule::CaptureCreationParameters(CaptureContext& captureContext) {
    schema::ShaderModule shaderModule{{
        .code = mCode,
    }};
    Serialize(captureContext, shaderModule);
    return {};
}

}  // namespace dawn::native::webgpu
