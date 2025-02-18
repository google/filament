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

#ifndef SRC_DAWN_NATIVE_D3D11_BINDGROUPTRACKERD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_BINDGROUPTRACKERD3D11_H_

#include <vector>

#include "dawn/native/BindGroupTracker.h"
#include "dawn/native/d3d/d3d_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::d3d11 {

class ScopedSwapStateCommandRecordingContext;

// We need convert WebGPU bind slot to d3d11 bind slot according a map in PipelineLayout, so we
// cannot inherit BindGroupTrackerGroups. Currently we arrange all the RTVs and UAVs when calling
// OMSetRenderTargetsAndUnorderedAccessViews() with below rules:
// - RTVs from the first register (r0)
// - UAVs in bind groups
// - Pixel Local Storage UAVs
class BindGroupTracker : public BindGroupTrackerBase</*CanInheritBindGroups=*/true, uint64_t> {
  public:
    explicit BindGroupTracker(const ScopedSwapStateCommandRecordingContext* commandContext);
    virtual ~BindGroupTracker();

    const ScopedSwapStateCommandRecordingContext* GetCommandContext() const;

  protected:
    template <wgpu::ShaderStage kVisibleStage>
    MaybeError ApplyBindGroup(BindGroupIndex index);

  private:
    raw_ptr<const ScopedSwapStateCommandRecordingContext> mCommandContext;
};

class ComputePassBindGroupTracker final : public BindGroupTracker {
  public:
    explicit ComputePassBindGroupTracker(
        const ScopedSwapStateCommandRecordingContext* commandContext);
    ~ComputePassBindGroupTracker() override;

    MaybeError Apply();

  private:
    void UnapplyComputeBindings(BindGroupIndex index);
};

class RenderPassBindGroupTracker final : public BindGroupTracker {
  public:
    explicit RenderPassBindGroupTracker(
        const ScopedSwapStateCommandRecordingContext* commandContext,
        std::vector<ComPtr<ID3D11UnorderedAccessView>> pixelLocalStorageUAVs = {});
    ~RenderPassBindGroupTracker() override;

    MaybeError Apply();

  private:
    // All the pixel local storage UAVs
    const std::vector<ComPtr<ID3D11UnorderedAccessView>> mPixelLocalStorageUAVs;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_BINDGROUPTRACKERD3D11_H_
