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

#include "dawn/native/opengl/ComputePipelineGL.h"

#include <vector>

#include "dawn/native/TintUtils.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/UtilsGL.h"
#include "tint/tint.h"

namespace dawn::native::opengl {

// static
Ref<ComputePipeline> ComputePipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<ComputePipelineDescriptor>& descriptor) {
    return AcquireRef(new ComputePipeline(device, descriptor));
}

ComputePipeline::~ComputePipeline() = default;

void ComputePipeline::DestroyImpl(DestroyReason reason) {
    ComputePipelineBase::DestroyImpl(reason);
    IgnoreErrors(
        ToBackend(GetDevice())
            ->EnqueueDestroyGL(this, &ComputePipeline::GetProgramHandle, reason,
                               [](const OpenGLFunctions& gl, GLuint program) -> MaybeError {
                                   DAWN_GL_TRY_IGNORE_ERRORS(gl, DeleteProgram(program));
                                   return {};
                               }));
}

ResultOrError<Extent3D> ComputePipeline::InitializeImpl() {
    DAWN_TRY(ToBackend(GetDevice())
                 ->EnqueueGL([self = Ref<ComputePipeline>(this)](
                                 const OpenGLFunctions& gl) -> MaybeError {
                     Extent3D workgroupSize;
                     return self->InitializeBase(gl, ToBackend(self->GetLayout()),
                                                 self->GetAllStages(), self->mImmediateMask,
                                                 /* bgraSwizzleAttributes */ {}, &workgroupSize);
                 }));

    // Shader reflection after the application of overrides is required by the frontend for the
    // workgroup size. In the case where GL execution is deferred, we need to compute the workgroup
    // size immediately. (do it in the non-deferred case as well to avoid duplicating paths).
    // TODO(https://issues.chromium.org/489650416): Move the GLSL translation to happen immediately
    // here instead of during deferred GL execution, which would remove the need for this.
    const ProgrammableStage& computeStage = GetStage(SingleShaderStage::Compute);

    tint::null::writer::Options tintOptions;
    tintOptions.entry_point_name = computeStage.entryPoint;
    tintOptions.substitute_overrides_config = {
        .map = BuildSubstituteOverridesTransformConfig(computeStage),
    };

    // Convert the AST program to an IR module.
    tint::wgsl::reader::IROptions irOptions{
        .dump_ir_when_validating = GetDevice()->IsToggleEnabled(Toggle::DumpTintIR),
        .enable_validation_asserts =
            GetDevice()->IsToggleEnabled(Toggle::EnableTintIRValidationAsserts),
    };
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

MaybeError ComputePipeline::ApplyNow(const OpenGLFunctions& gl) {
    DAWN_TRY(PipelineGL::ApplyNow(gl, ToBackend(GetLayout())));
    return {};
}

GLuint ComputePipeline::GetProgramHandle() const {
    return mProgram;
}

}  // namespace dawn::native::opengl
