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

#include "dawn/native/d3d12/ComputePipelineD3D12.h"

#include <memory>
#include <utility>

#include "dawn/native/CreatePipelineAsyncEvent.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/BlobD3D.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "dawn/native/d3d12/ShaderModuleD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/platform/metrics/HistogramMacros.h"

namespace dawn::native::d3d12 {

Ref<ComputePipeline> ComputePipeline::CreateUninitialized(
    Device* device,
    const UnpackedPtr<ComputePipelineDescriptor>& descriptor) {
    return AcquireRef(new ComputePipeline(device, descriptor));
}

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

    if (device->IsToggleEnabled(Toggle::UseDXC) &&
        ((compileFlags & D3DCOMPILE_OPTIMIZATION_LEVEL2) == 0)) {
        // DXC's default opt level is /O3, unlike FXC's /O1. Set explicitly, otherwise there's no
        // way to tell if we want /O1 as D3DCOMPILE_OPTIMIZATION_LEVEL1 is defined to 0.
        compileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    }

    // Tint does matrix multiplication expecting row major matrices
    compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

    const ProgrammableStage& computeStage = GetStage(SingleShaderStage::Compute);
    ShaderModule* module = ToBackend(computeStage.module.Get());

    if (module->GetStrictMath().value_or(
            !device->IsToggleEnabled(Toggle::D3DDisableIEEEStrictness))) {
        compileFlags |= D3DCOMPILE_IEEE_STRICTNESS;
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC d3dDesc = {};
    d3dDesc.pRootSignature = ToBackend(GetLayout())->GetRootSignature();

    d3d::CompiledShader compiledShader;
    DAWN_TRY_ASSIGN(compiledShader, module->Compile(computeStage, SingleShaderStage::Compute,
                                                    ToBackend(GetLayout()), compileFlags,
                                                    /* usedInterstageVariables */ {}));
    d3dDesc.CS = {compiledShader.shaderBlob.Data(), compiledShader.shaderBlob.Size()};

    StreamIn(&mCacheKey, d3dDesc, ToBackend(GetLayout())->GetRootSignatureBlob());

    // Try to see if we have anything in the blob cache.
    Blob blob = device->LoadCachedBlob(GetCacheKey());
    bool cacheHit = !blob.Empty();
    if (cacheHit) {
        // Cache hits, attach cached blob to descriptor.
        d3dDesc.CachedPSO.pCachedBlob = blob.Data();
        d3dDesc.CachedPSO.CachedBlobSizeInBytes = blob.Size();
    }

    // We don't use the scoped cache histogram counters for the cache hit here so that we can
    // condition on whether it fails appropriately.
    auto* d3d12Device = device->GetD3D12Device();
    platform::metrics::DawnHistogramTimer cacheTimer(device->GetPlatform());
    HRESULT result =
        d3d12Device->CreateComputePipelineState(&d3dDesc, IID_PPV_ARGS(&mPipelineState));
    if (cacheHit && result == D3D12_ERROR_DRIVER_VERSION_MISMATCH) {
        // See dawn:1878 where it is possible for the PSO creation to fail with this error.
        cacheHit = false;
        d3dDesc.CachedPSO.pCachedBlob = nullptr;
        d3dDesc.CachedPSO.CachedBlobSizeInBytes = 0;
        cacheTimer.Reset();
        result = d3d12Device->CreateComputePipelineState(&d3dDesc, IID_PPV_ARGS(&mPipelineState));
    }
    DAWN_TRY(CheckHRESULT(result, "D3D12 creating pipeline state"));

    if (!cacheHit) {
        // Cache misses, need to get pipeline cached blob and store.
        cacheTimer.RecordMicroseconds("D3D12.CreateComputePipelineState.CacheMiss");
        ComPtr<ID3DBlob> d3dBlob;
        if (!device->GetInstance()->ConsumedError(
                CheckHRESULT(GetPipelineState()->GetCachedBlob(&d3dBlob),
                             "D3D12 compute pipeline state get cached blob"))) {
            device->StoreCachedBlob(GetCacheKey(), CreateBlob(std::move(d3dBlob)));
        }
    } else {
        cacheTimer.RecordMicroseconds("D3D12.CreateComputePipelineState.CacheHit");
    }

    SetLabelImpl();

    return {};
}

ComputePipeline::~ComputePipeline() = default;

void ComputePipeline::DestroyImpl() {
    ComputePipelineBase::DestroyImpl();
    ToBackend(GetDevice())->ReferenceUntilUnused(mPipelineState);
}

ID3D12PipelineState* ComputePipeline::GetPipelineState() const {
    return mPipelineState.Get();
}

void ComputePipeline::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), GetPipelineState(), "Dawn_ComputePipeline", GetLabel());
}

bool ComputePipeline::UsesNumWorkgroups() const {
    return GetStage(SingleShaderStage::Compute).metadata->usesNumWorkgroups;
}

ComPtr<ID3D12CommandSignature> ComputePipeline::GetDispatchIndirectCommandSignature() {
    if (UsesNumWorkgroups()) {
        return ToBackend(GetLayout())->GetDispatchIndirectCommandSignatureWithNumWorkgroups();
    }
    return ToBackend(GetDevice())->GetDispatchIndirectSignature();
}

}  // namespace dawn::native::d3d12
