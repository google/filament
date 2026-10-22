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

#ifndef SRC_DAWN_NATIVE_COMMANDBUFFERSTATETRACKER_H_
#define SRC_DAWN_NATIVE_COMMANDBUFFERSTATETRACKER_H_

#include <vector>

#include "partition_alloc/pointers/raw_ptr_exclusion.h"
#include "src/dawn/common/Constants.h"
#include "src/dawn/common/ityp_array.h"
#include "src/dawn/common/ityp_bitset.h"
#include "src/dawn/common/ityp_vector.h"
#include "src/dawn/native/BindingInfo.h"
#include "src/dawn/native/Error.h"
#include "src/dawn/native/Forward.h"
#include "src/dawn/native/IntegerTypes.h"
#include "src/utils/span.h"

namespace dawn::native {

class CommandBufferStateTracker {
  public:
    CommandBufferStateTracker();
    CommandBufferStateTracker(const CommandBufferStateTracker&);
    CommandBufferStateTracker(CommandBufferStateTracker&&);
    ~CommandBufferStateTracker();

    CommandBufferStateTracker& operator=(const CommandBufferStateTracker&);
    CommandBufferStateTracker& operator=(CommandBufferStateTracker&&);

    // Non-state-modifying validation functions
    MaybeError ValidateCanDispatch();
    MaybeError ValidateCanDraw();
    MaybeError ValidateCanDrawIndexed();
    MaybeError ValidateNoDifferentTextureViewsOnSameTexture();
    MaybeError ValidateBufferInRangeForVertexBuffer(uint32_t vertexCount, uint32_t firstVertex);
    MaybeError ValidateBufferInRangeForInstanceBuffer(uint32_t instanceCount,
                                                      uint32_t firstInstance);
    MaybeError ValidateIndexBufferInRange(uint32_t indexCount, uint32_t firstIndex);

    // State-modifying methods
    void SetComputePipeline(ComputePipelineBase* pipeline);
    void SetRenderPipeline(RenderPipelineBase* pipeline);
    void UnsetBindGroup(BindGroupIndex index);
    void SetBindGroup(BindGroupIndex index,
                      BindGroupBase* bindgroup,
                      ityp::span<BindingIndex, const uint32_t> dynamicOffsets);
    void SetResourceTable(ResourceTableBase* resourceTable);
    void SetRenderBundleHasResourceTable();
    void SetIndexBuffer(BufferBase* buffer,
                        wgpu::IndexFormat format,
                        uint64_t offset,
                        uint64_t size);
    void UnsetVertexBuffer(VertexBufferSlot slot);
    void SetVertexBuffer(VertexBufferSlot slot, uint64_t size);
    void SetImmediateData(uint32_t offset, size_t size);
    void End();

    static constexpr size_t kNumAspects = 6;
    using ValidationAspects = std::bitset<kNumAspects>;

    BindGroupBase* GetBindGroup(BindGroupIndex index) const;
    ResourceTableBase* GetResourceTable() const;
    ityp::span<BindingIndex, const uint32_t> GetDynamicOffsets(BindGroupIndex index) const;
    bool HasPipeline() const;
    bool HasResourceTable() const;
    bool IndexBufferSet() const;
    RenderPipelineBase* GetRenderPipeline() const;
    ComputePipelineBase* GetComputePipeline() const;
    PipelineLayoutBase* GetPipelineLayout() const;
    BufferBase* GetIndexBuffer() const;
    wgpu::IndexFormat GetIndexFormat() const;
    uint64_t GetIndexBufferSize() const;
    uint64_t GetIndexBufferOffset() const;

  private:
    MaybeError ValidateOperation(ValidationAspects requiredAspects);
    void RecomputeLazyAspects(ValidationAspects aspects);
    MaybeError CheckMissingAspects(ValidationAspects aspects);

    void SetPipelineCommon(PipelineBase* pipeline);

    ValidationAspects mAspects;

    VertexBufferMask mVertexBuffersUsed;
    PerVertexBuffer<uint64_t> mVertexBufferSizes = {};

    wgpu::IndexFormat mIndexFormat = wgpu::IndexFormat::Undefined;
    uint64_t mIndexBufferSize = 0;
    uint64_t mIndexBufferOffset = 0;
    RAW_PTR_EXCLUSION BufferBase* mIndexBuffer = nullptr;

    // RAW_PTR_EXCLUSION: These pointers are very hot in command recording code and point at
    // various objects referenced by the object graph of the CommandBuffer so they cannot be
    // freed from underneath this class.
    RAW_PTR_EXCLUSION PerBindGroup<BindGroupBase*> mBindgroups = {};
    PerBindGroup<ityp::vector<BindingIndex, uint32_t>> mDynamicOffsets;
    RAW_PTR_EXCLUSION ResourceTableBase* mResourceTable = nullptr;
    bool mHasResourceTable = false;

    RAW_PTR_EXCLUSION PipelineLayoutBase* mLastPipelineLayout = nullptr;
    RAW_PTR_EXCLUSION PipelineBase* mLastPipeline = nullptr;
    RAW_PTR_EXCLUSION const RequiredBufferSizes* mMinBufferSizes = nullptr;

    ImmediateMask mImmediateDataMask;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMMANDBUFFERSTATETRACKER_H_
