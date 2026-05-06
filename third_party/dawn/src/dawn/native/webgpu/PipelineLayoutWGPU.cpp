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

#include "dawn/native/webgpu/PipelineLayoutWGPU.h"

#include <array>
#include <string>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/native/webgpu/BindGroupLayoutWGPU.h"
#include "dawn/native/webgpu/CaptureContext.h"
#include "dawn/native/webgpu/DeviceWGPU.h"

namespace dawn::native::webgpu {

// static
Ref<PipelineLayout> PipelineLayout::Create(
    Device* device,
    const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) {
    return AcquireRef(new PipelineLayout(device, descriptor));
}

PipelineLayout::PipelineLayout(Device* device,
                               const UnpackedPtr<PipelineLayoutDescriptor>& descriptor)
    : PipelineLayoutBase(device, descriptor),
      RecordableObject(schema::ObjectType::PipelineLayout),
      ObjectWGPU(device->wgpu->pipelineLayoutRelease) {
    std::string label = GetLabel();
    WGPUPipelineLayoutDescriptor desc;
    desc.nextInChain = nullptr;
    desc.label = ToOutputStringView(label);

    std::array<WGPUBindGroupLayout, kMaxBindGroups> bindGroupLayouts;
    bindGroupLayouts.fill(nullptr);
    size_t bindGroupLayoutCount = 0;
    for (BindGroupIndex group : GetBindGroupLayoutsMask()) {
        // Empty or null bind group layout aren't iterated.
        auto idx = static_cast<size_t>(group);
        bindGroupLayoutCount = idx + 1;
        bindGroupLayouts[idx] = ToBackend(GetBindGroupLayout(group))->GetInnerHandle();
    }

    desc.bindGroupLayouts = bindGroupLayouts.data();
    desc.bindGroupLayoutCount = bindGroupLayoutCount;
    desc.immediateSize = GetImmediateDataRangeByteSize();

    mInnerHandle = device->wgpu->deviceCreatePipelineLayout(device->GetInnerHandle(), &desc);
    DAWN_ASSERT(mInnerHandle);
}

void PipelineLayout::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError PipelineLayout::AddReferenced(CaptureContext& captureContext) {
    for (BindGroupIndex groupIndex : GetBindGroupLayoutsMask()) {
        auto frontendLayout = GetFrontendBindGroupLayout(groupIndex);
        DAWN_TRY(
            captureContext.AddResource(ToBackend(frontendLayout->GetInternalBindGroupLayout())));
    }
    return {};
}

MaybeError PipelineLayout::CaptureCreationParameters(CaptureContext& captureContext) {
    std::vector<schema::ObjectId> bindGroupLayoutIds(kMaxBindGroups, 0);

    for (BindGroupIndex groupIndex : GetBindGroupLayoutsMask()) {
        auto frontendLayout = GetFrontendBindGroupLayout(groupIndex);
        bindGroupLayoutIds[static_cast<size_t>(groupIndex)] =
            captureContext.GetId(frontendLayout->GetInternalBindGroupLayout());
    }

    schema::PipelineLayout data{{
        .bindGroupLayoutIds = bindGroupLayoutIds,
        .immediateSize = GetImmediateDataRangeByteSize(),
    }};
    Serialize(captureContext, data);
    return {};
}

}  // namespace dawn::native::webgpu
