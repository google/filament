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

#include <cstdint>
#include <tuple>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/native/BindGroupTracker.h"
#include "src/dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class ScopedSwapStateCommandRecordingContext;

// We need convert WebGPU bind slot to d3d11 bind slot according a map in PipelineLayout, so we
// cannot inherit BindGroupTrackerGroups. Currently we arrange all the RTVs and UAVs when calling
// OMSetRenderTargetsAndUnorderedAccessViews() with below rules:
// - RTVs from the first register (r0)
// - UAVs in bind groups
// - Pixel Local Storage UAVs
class BindGroupTracker : public BindGroupTrackerBase</*CanInheritBindGroups=*/true> {
  public:
    explicit BindGroupTracker(const ScopedSwapStateCommandRecordingContext* commandContext);
    ~BindGroupTracker() override;

  protected:
    const ScopedSwapStateCommandRecordingContext* GetCommandContext() const {
        return mCommandContext;
    }

    // The pipeline layout last applied to the bind group bindings, or the device's empty pipeline
    // layout when no pipeline has been applied yet. BindGroupTrackerBase tracks the pipeline (which
    // has no "empty" form), so the empty-layout sentinel is resolved lazily here rather than seeded
    // in the constructor.
    PipelineLayoutBase* LastAppliedPipelineLayout() const;

    template <wgpu::ShaderStage kVisibleStage>
    MaybeError ApplyBindGroup(BindGroupIndex index);

    template <SingleShaderStage Stage>
    void SetConstantBuffer(uint32_t idx,
                           ID3D11Buffer* d3d11Buffer,
                           uint32_t firstConstant,
                           uint32_t numConstants);

    template <SingleShaderStage Stage>
    void SetShaderResource(uint32_t idx, ID3D11ShaderResourceView* srv);

    template <SingleShaderStage Stage>
    void SetSampler(uint32_t idx, ID3D11SamplerState* sampler);

    void CSSetUnorderedAccessView(uint32_t idx, ID3D11UnorderedAccessView* uav);

    void OMSetUnorderedAccessViews(uint32_t idx,
                                   uint32_t count,
                                   ID3D11UnorderedAccessView* const* uavs);

    template <SingleShaderStage Stage>
    void UnbindConstantBuffers();
    template <SingleShaderStage Stage>
    void UnbindShaderResources();
    template <SingleShaderStage Stage>
    void UnbindSamplers();
    template <SingleShaderStage Stage>
    void UnbindUnorderedAccessViews();

  private:
    struct CBufferBindingKey {
        bool operator==(const CBufferBindingKey&) const = default;

        uintptr_t buffer;
        UINT firstConstant;
        UINT numConstants;
    };

    ResultOrError<std::tuple<ID3D11Buffer*, UINT, UINT>> GetConstantBufferBinding(
        BindGroupBase* group,
        BindingIndex bindingIndex,
        const BufferBindingInfo& layout,
        const ityp::span<BindingIndex, uint32_t>& dynamicOffsets);

    ID3D11SamplerState* GetSamplerState(BindGroupBase* group, BindingIndex bindingIndex);

    raw_ptr<const ScopedSwapStateCommandRecordingContext> mCommandContext;

    // Tracks the key of the resource currently bound to each slot, to skip redundant D3D11 Set*
    // calls. The key is computed from the bound object's pointer, but is only ever compared, never
    // dereferenced, so a stale/reused key can at worst cause a missed rebind (a logic bug), never
    // a use-after-free. In practice a stale pointer shouldn't even occur here: once a view/sampler
    // is bound, the D3D11 runtime holds its own reference on it for as long as it stays bound to
    // that slot, which keeps its address from being reused by an unrelated object while this cache
    // could still compare against it.
    template <typename Key, uint32_t InitialCapacity>
    class BindSlotCache {
      public:
        template <typename Fn>
        void Bind(uint32_t idx, Key key, Fn&& bindFunc);

        uint32_t MaxBoundSlots() const { return static_cast<uint32_t>(mBoundKeys.size()); }

      private:
        absl::InlinedVector<Key, InitialCapacity> mBoundKeys;
    };

    PerStage<BindSlotCache<CBufferBindingKey, 4>> mConstantBufferSlots;

    PerStage<BindSlotCache<uintptr_t, 4>> mSRVSlots;

    PerStage<BindSlotCache<uintptr_t, 4>> mSamplerSlots;

    // We only track individual UAV slot in CS because UAV slots in PS must be bound all as one.
    BindSlotCache<uintptr_t, 4> mCSUAVSlots;

    PerStage<uint32_t> mMinUAVSlots = PerStage<uint32_t>(D3D11_1_UAV_SLOT_COUNT);
    uint32_t mPSMaxUAVSlot = 0;
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
