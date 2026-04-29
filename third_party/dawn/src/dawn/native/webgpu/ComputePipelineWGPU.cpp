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

#include "dawn/native/webgpu/ComputePipelineWGPU.h"

#include <string>
#include <vector>

#include "dawn/common/StringViewUtils.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/webgpu/BindGroupLayoutWGPU.h"
#include "dawn/native/webgpu/CaptureContext.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/PipelineLayoutWGPU.h"
#include "dawn/native/webgpu/ShaderModuleWGPU.h"
#include "dawn/native/webgpu/ToWGPU.h"
#include "tint/tint.h"

namespace dawn::native::webgpu {

// static
Ref<ComputePipeline> ComputePipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<ComputePipelineDescriptor>& descriptor) {
    return AcquireRef(new ComputePipeline(device, descriptor));
}

ComputePipeline::ComputePipeline(Device* device,
                                 const UnpackedPtr<ComputePipelineDescriptor>& descriptor)
    : ComputePipelineBase(device, descriptor),
      RecordableObject(schema::ObjectType::ComputePipeline),
      ObjectWGPU(device->wgpu->computePipelineRelease) {}

ResultOrError<Extent3D> ComputePipeline::InitializeImpl() {
    std::string label = GetLabel();
    WGPUComputePipelineDescriptor desc;
    desc.nextInChain = nullptr;
    desc.label = ToOutputStringView(label);
    const PipelineLayoutBase* layout = GetLayout();
    DAWN_ASSERT(layout != nullptr);
    desc.layout = ToBackend(layout)->GetInnerHandle();

    const ProgrammableStage& stage = GetStage(SingleShaderStage::Compute);
    desc.compute.nextInChain = nullptr;
    desc.compute.module = ToBackend(stage.module.Get())->GetInnerHandle();
    desc.compute.entryPoint = ToOutputStringView(stage.entryPoint);

    std::vector<WGPUConstantEntry> constants;
    std::vector<std::string> keys;
    PopulateWGPUConstants(&constants, &keys, stage.constants);
    desc.compute.constants = constants.data();
    desc.compute.constantCount = constants.size();

    auto device = ToBackend(GetDevice());
    mInnerHandle = device->wgpu->deviceCreateComputePipeline(device->GetInnerHandle(), &desc);
    DAWN_ASSERT(mInnerHandle);

    // Shader reflection after the application of overrides is required by the frontend for the
    // workgroup size.
    const ProgrammableStage& computeStage = GetStage(SingleShaderStage::Compute);

    tint::null::writer::Options tintOptions;
    tintOptions.entry_point_name = computeStage.entryPoint;
    tintOptions.substitute_overrides_config = {
        .map = BuildSubstituteOverridesTransformConfig(computeStage),
    };

    tint::wgsl::reader::IROptions irOptions{
        .dump_ir_when_validating = device->IsToggleEnabled(Toggle::DumpTintIR),
        .enable_validation_asserts = device->IsToggleEnabled(Toggle::EnableTintIRValidationAsserts),
    };

    // Convert the AST program to an IR module.
    auto ir = tint::wgsl::reader::ProgramToLoweredIR(computeStage.module->GetTintProgram()->program,
                                                     irOptions);
    DAWN_INVALID_IF(ir != tint::Success, "An error occurred while generating Tint IR\n%s",
                    ir.Failure().reason);

    tint::Result<tint::null::writer::Output> tintResult =
        tint::null::writer::Generate(ir.Get(), tintOptions);

    DAWN_INVALID_IF(tintResult != tint::Success, "An error occurred while running Null writer\n%s",
                    tintResult.Failure().reason);

    return {
        {tintResult->workgroup_info.x, tintResult->workgroup_info.y, tintResult->workgroup_info.z}};
}

void ComputePipeline::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError ComputePipeline::AddReferenced(CaptureContext& captureContext) {
    DAWN_TRY(
        captureContext.AddResource(ToBackend(GetStage(SingleShaderStage::Compute).module.Get())));
    DAWN_TRY(captureContext.AddResource(ToBackend(GetLayout())));
    return {};
}

MaybeError ComputePipeline::CaptureCreationParameters(CaptureContext& captureContext) {
    auto& stage = GetStage(SingleShaderStage::Compute);
    schema::ComputePipeline data{{
        .layoutId = captureContext.GetId(GetLayout()),
        .compute = ToSchema(captureContext, stage),
    }};
    Serialize(captureContext, data);
    return {};
}

}  // namespace dawn::native::webgpu
