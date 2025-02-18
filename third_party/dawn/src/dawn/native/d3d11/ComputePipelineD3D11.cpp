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

#include "dawn/native/d3d11/ComputePipelineD3D11.h"

#include <memory>
#include <utility>

#include "dawn/native/CreatePipelineAsyncEvent.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/ShaderModuleD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"

namespace dawn::native::d3d11 {

// static
Ref<ComputePipeline> ComputePipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<ComputePipelineDescriptor>& descriptor) {
    return AcquireRef(new ComputePipeline(device, descriptor));
}

ComputePipeline::~ComputePipeline() = default;

MaybeError ComputePipeline::InitializeImpl() {
    Device* device = ToBackend(GetDevice());
    uint32_t compileFlags = 0;

    if (!device->IsToggleEnabled(Toggle::UseDXC) &&
        !device->IsToggleEnabled(Toggle::FxcOptimizations)) {
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL0;
    }

    if (device->IsToggleEnabled(Toggle::EmitHLSLDebugSymbols)) {
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
    }

    // Tint does matrix multiplication expecting row major matrices
    compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

    const ProgrammableStage& programmableStage = GetStage(SingleShaderStage::Compute);
    if (programmableStage.module->GetStrictMath().value_or(
            !device->IsToggleEnabled(Toggle::D3DDisableIEEEStrictness))) {
        compileFlags |= D3DCOMPILE_IEEE_STRICTNESS;
    }

    d3d::CompiledShader compiledShader;
    DAWN_TRY_ASSIGN(compiledShader, ToBackend(programmableStage.module)
                                        ->Compile(programmableStage, SingleShaderStage::Compute,
                                                  ToBackend(GetLayout()), compileFlags));
    DAWN_TRY(CheckHRESULT(device->GetD3D11Device()->CreateComputeShader(
                              compiledShader.shaderBlob.Data(), compiledShader.shaderBlob.Size(),
                              nullptr, &mComputeShader),
                          "D3D11 create compute shader"));

    SetLabelImpl();

    return {};
}

void ComputePipeline::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mComputeShader.Get(), "Dawn_ComputePipeline", GetLabel());
}

void ComputePipeline::ApplyNow(const ScopedSwapStateCommandRecordingContext* commandContext) {
    auto* d3dDeviceContext = commandContext->GetD3D11DeviceContext4();
    d3dDeviceContext->CSSetShader(mComputeShader.Get(), nullptr, 0);
}

bool ComputePipeline::UsesNumWorkgroups() const {
    return GetStage(SingleShaderStage::Compute).metadata->usesNumWorkgroups;
}

}  // namespace dawn::native::d3d11
