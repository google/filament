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

#ifndef SRC_DAWN_NATIVE_D3D11_COMPUTEPIPELINEGL_H_
#define SRC_DAWN_NATIVE_D3D11_COMPUTEPIPELINEGL_H_

#include "dawn/native/ComputePipeline.h"
#include "dawn/native/CreatePipelineAsyncEvent.h"

#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class Device;
class ScopedSwapStateCommandRecordingContext;

class ComputePipeline final : public ComputePipelineBase {
  public:
    static Ref<ComputePipeline> CreateUninitialized(
        Device* device,
        const UnpackedPtr<ComputePipelineDescriptor>& descriptor);

    void ApplyNow(const ScopedSwapStateCommandRecordingContext* commandContext);

    MaybeError InitializeImpl() override;

    bool UsesNumWorkgroups() const;

  private:
    using ComputePipelineBase::ComputePipelineBase;
    ~ComputePipeline() override;
    void SetLabelImpl() override;

    ComPtr<ID3D11ComputeShader> mComputeShader;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_COMPUTEPIPELINEGL_H_
