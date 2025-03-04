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

#include "dawn/common/Constants.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/PipelineLayout.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"

namespace dawn::native {

// Keeps track of the dirty bind groups so they can be lazily applied when we know the
// pipeline state or it changes.
// |DynamicOffset| is a template parameter because offsets in Vulkan are uint32_t but uint64_t
// in other backends.
template <bool CanInheritBindGroups, typename DynamicOffset>
class BindGroupTrackerBase {
  public:
    void OnSetBindGroup(BindGroupIndex index,
                        BindGroupBase* bindGroup,
                        uint32_t dynamicOffsetCount,
                        uint32_t* dynamicOffsets) {
        DAWN_ASSERT(index < kMaxBindGroupsTyped);

        if (mBindGroupLayoutsMask[index]) {
            // It is okay to only dirty bind groups that are used by the current pipeline
            // layout. If the pipeline layout changes, then the bind groups it uses will
            // become dirty.

            if (mBindGroups[index] != bindGroup) {
                mDirtyBindGroups.set(index);
                mDirtyBindGroupsObjectChangedOrIsDynamic.set(index);
            }

            if (dynamicOffsetCount > 0) {
                mDirtyBindGroupsObjectChangedOrIsDynamic.set(index);
            }
        }

        mBindGroups[index] = bindGroup;
        mDynamicOffsets[index].resize(BindingIndex(dynamicOffsetCount));
        std::copy(dynamicOffsets, dynamicOffsets + dynamicOffsetCount,
                  mDynamicOffsets[index].begin());
    }

    void OnSetPipeline(PipelineBase* pipeline) { mPipelineLayout = pipeline->GetLayout(); }

  protected:
    virtual bool AreLayoutsCompatible() { return mLastAppliedPipelineLayout == mPipelineLayout; }

    // The Derived class should call this before it applies bind groups.
    void BeforeApply() {
        if (AreLayoutsCompatible()) {
            return;
        }

        // Use the bind group layout mask to avoid marking unused bind groups as dirty.
        mBindGroupLayoutsMask = mPipelineLayout->GetBindGroupLayoutsMask();

        // Changing the pipeline layout sets bind groups as dirty. If CanInheritBindGroups,
        // the first |k| matching bind groups may be inherited.
        if (CanInheritBindGroups && mLastAppliedPipelineLayout != nullptr) {
            // Dirty bind groups that cannot be inherited.
            BindGroupMask dirtiedGroups =
                ~mPipelineLayout->InheritedGroupsMask(mLastAppliedPipelineLayout);

            mDirtyBindGroups |= dirtiedGroups;
            mDirtyBindGroupsObjectChangedOrIsDynamic |= dirtiedGroups;

            // Clear any bind groups not in the mask.
            mDirtyBindGroups &= mBindGroupLayoutsMask;
            mDirtyBindGroupsObjectChangedOrIsDynamic &= mBindGroupLayoutsMask;
        } else {
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
        // Keep track of the last applied pipeline layout. This allows us to avoid computing
        // the intersection of the dirty bind groups and bind group layout mask in next Draw
        // or Dispatch (which is very hot code) until the layout is changed again.
        mLastAppliedPipelineLayout = mPipelineLayout;
    }

    BindGroupMask mDirtyBindGroups = 0;
    BindGroupMask mDirtyBindGroupsObjectChangedOrIsDynamic = 0;
    BindGroupMask mBindGroupLayoutsMask = 0;
    PerBindGroup<BindGroupBase*> mBindGroups = {};
    PerBindGroup<ityp::vector<BindingIndex, DynamicOffset>> mDynamicOffsets = {};

    // |mPipelineLayout| is the current pipeline layout set on the command buffer.
    // |mLastAppliedPipelineLayout| is the last pipeline layout for which we applied changes
    // to the bind group bindings.
    // RAW_PTR_EXCLUSION: These pointers are very hot in command recording code and point at
    // pipeline layouts referenced by the object graph of the CommandBuffer so they cannot be
    // freed from underneath this class.
    RAW_PTR_EXCLUSION PipelineLayoutBase* mPipelineLayout = nullptr;
    RAW_PTR_EXCLUSION PipelineLayoutBase* mLastAppliedPipelineLayout = nullptr;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BINDGROUPTRACKER_H_
