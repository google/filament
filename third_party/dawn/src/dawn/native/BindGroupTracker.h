// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_BINDGROUPTRACKER_H_
#define SRC_DAWN_NATIVE_BINDGROUPTRACKER_H_

#include <algorithm>
#include <array>
#include <bitset>

#include "partition_alloc/pointers/raw_ptr_exclusion.h"
#include "src/dawn/common/Constants.h"
#include "src/dawn/native/BindGroup.h"
#include "src/dawn/native/Pipeline.h"
#include "src/dawn/native/PipelineLayout.h"
#include "src/utils/compiler.h"
#include "src/utils/span.h"

namespace dawn::native {

// Keeps track of the dirty bind groups so they can be lazily applied when we know the
// pipeline state or it changes.
template <bool CanInheritBindGroups>
class BindGroupTrackerBase {
  public:
    virtual ~BindGroupTrackerBase() = default;

    void OnSetBindGroup(BindGroupIndex index,
                        BindGroupBase* bindGroup,
                        ityp::span<BindingIndex, const uint32_t> dynamicOffsets) {
        DAWN_ASSERT(index < kMaxBindGroupsTyped);
        DAWN_ASSERT(dynamicOffsets.size() <= kMaxDynamicOffsetsPerBindGroupTyped);

        if (mBindGroupLayoutsMask[index]) {
            // It is okay to only dirty bind groups that are used by the current pipeline
            // layout. If the pipeline layout changes, then the bind groups it uses will
            // become dirty.

            if (mBindGroups[index] != bindGroup) {
                mDirtyBindGroups.set(index);
                mDirtyBindGroupsObjectChangedOrIsDynamic.set(index);
            }

            if (!dynamicOffsets.empty()) {
                mDirtyBindGroupsObjectChangedOrIsDynamic.set(index);
            }
        }

        mBindGroups[index] = bindGroup;
        mDynamicOffsets[index].count = dynamicOffsets.size();
        // TODO(https://crbug.com/524406299): Use Span::CopyFrom
        std::ranges::copy(dynamicOffsets, mDynamicOffsets[index].offsets.begin());
    }

    void OnSetPipeline(PipelineBase* pipeline) { mPipeline = pipeline; }

  protected:
    virtual bool AreLayoutsCompatible() {
        return mLastAppliedPipeline != nullptr &&
               mLastAppliedPipeline->GetLayout() == mPipeline->GetLayout();
    }

    ityp::span<BindingIndex, uint32_t> GetDynamicOffsets(BindGroupIndex index) {
        return ityp::span<BindingIndex, uint32_t>(mDynamicOffsets[index].offsets)
            .first(mDynamicOffsets[index].count);
    }

    // The Derived class should call this before it applies bind groups.
    void BeforeApply() {
        if (AreLayoutsCompatible()) {
            return;
        }

        // Use the bind group layout mask to avoid marking unused bind groups as dirty.
        PipelineLayoutBase* pipelineLayout = mPipeline->GetLayout();
        mBindGroupLayoutsMask = pipelineLayout->GetBindGroupLayoutsMask();

        // Changing the pipeline layout sets bind groups as dirty. If CanInheritBindGroups,
        // the first |k| matching bind groups may be inherited.
        if (CanInheritBindGroups && mLastAppliedPipeline != nullptr) {
            // Dirty bind groups that cannot be inherited.
            BindGroupMask dirtiedGroups =
                ~pipelineLayout->InheritedGroupsMask(mLastAppliedPipeline->GetLayout());

            mDirtyBindGroups |= dirtiedGroups;
            mDirtyBindGroupsObjectChangedOrIsDynamic |= dirtiedGroups;

            // Clear any bind groups not in the mask.
            mDirtyBindGroups &= mBindGroupLayoutsMask;
            mDirtyBindGroupsObjectChangedOrIsDynamic &= mBindGroupLayoutsMask;
        } else {
            // All bind groups (in the mask) are dirty
            mDirtyBindGroups = mBindGroupLayoutsMask;
            mDirtyBindGroupsObjectChangedOrIsDynamic = mBindGroupLayoutsMask;
        }
    }

    // The Derived class should call this after it applies bind groups.
    void AfterApply() {
        // Reset all dirty bind groups. Dirty bind groups not in the bind group layout mask
        // will be dirtied again by the next pipeline change.
        mDirtyBindGroups.reset();
        mDirtyBindGroupsObjectChangedOrIsDynamic.reset();
        // Keep track of the last applied pipeline. This allows us to avoid computing
        // the intersection of the dirty bind groups and bind group layout mask in next Draw
        // or Dispatch (which is very hot code) until the layout is changed again.
        mLastAppliedPipeline = mPipeline;
    }

    BindGroupMask mDirtyBindGroups = 0;
    BindGroupMask mDirtyBindGroupsObjectChangedOrIsDynamic = 0;
    BindGroupMask mBindGroupLayoutsMask = 0;
    PerBindGroup<BindGroupBase*> mBindGroups = {};

    // |mPipeline| is the current pipeline set on the command buffer.
    // |mLastAppliedPipeline| is the last pipeline for which we applied changes
    // to the bind group bindings.
    // RAW_PTR_EXCLUSION: These pointers are very hot in command recording code and point at
    // pipelines referenced by the object graph of the CommandBuffer so they cannot be
    // freed from underneath this class.
    RAW_PTR_EXCLUSION PipelineBase* mPipeline = nullptr;
    RAW_PTR_EXCLUSION PipelineBase* mLastAppliedPipeline = nullptr;

  private:
    // Max possible dynamic offsets per bind group. Uses the per-pipeline limits because it's
    // possible that one bind group uses all the available dynamic offsets and every other bind
    // group uses none.
    static constexpr uint32_t kMaxDynamicOffsetsPerBindGroup =
        kMaxDynamicUniformBuffersPerPipelineLayout + kMaxDynamicStorageBuffersPerPipelineLayout;
    static constexpr BindingIndex kMaxDynamicOffsetsPerBindGroupTyped{
        kMaxDynamicOffsetsPerBindGroup};

    struct BindingDynamicOffsets {
        ityp::array<BindingIndex, uint32_t, kMaxDynamicOffsetsPerBindGroup> offsets = {};
        BindingIndex count = {};
    };

    PerBindGroup<BindingDynamicOffsets> mDynamicOffsets = {};
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BINDGROUPTRACKER_H_
