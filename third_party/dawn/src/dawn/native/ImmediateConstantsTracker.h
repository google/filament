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

#ifndef SRC_DAWN_NATIVE_IMMEDIATECONSTANTSTRACKER_H_
#define SRC_DAWN_NATIVE_IMMEDIATECONSTANTSTRACKER_H_

#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>

#include "ImmediateConstantsLayout.h"
#include "dawn/common/Constants.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/common/ityp_span.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Device.h"
#include "dawn/native/ImmediateConstantsLayout.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/RenderPipeline.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"

namespace dawn::native {

template <typename T>
struct ImmediateDataContent {
  public:
    const T* operator->() const { return reinterpret_cast<const T*>(&mData); }
    T* operator->() { return reinterpret_cast<T*>(&mData); }

    const unsigned char* data() const { return mData; }

    template <typename Out>
    const Out* Get(uint32_t offset) const {
        DAWN_ASSERT(sizeof(Out) + offset <= sizeof(T));
        return reinterpret_cast<const Out*>(&mData[offset]);
    }

    template <typename Out>
    Out* Get(uint32_t offset) {
        DAWN_ASSERT(sizeof(Out) + offset <= sizeof(T));
        return reinterpret_cast<Out*>(&mData[offset]);
    }

  private:
    alignas(T) unsigned char mData[sizeof(T)] = {0};
};

// TODO(crbug.com/366291600): Add inheritance ability(like BindGroupTracker) so that it can inherit
// immediate constants in native backend if supported.
template <typename T, typename PipelineType>
class UserImmediateConstantsTrackerBase {
  public:
    UserImmediateConstantsTrackerBase() {}

    // Setters
    void SetImmediateData(uint32_t offset, uint8_t* values, uint32_t size) {
        uint8_t* destData = mContent.template Get<uint8_t>(offsetof(T, userConstants) + offset);
        if (memcmp(destData, values, size) != 0) {
            memcpy(destData, values, size);
            mDirty |= GetImmediateConstantBlockBits(offsetof(T, userConstants),
                                                    sizeof(UserImmediateConstants));
        }
    }

    // TODO(crbug.com/366291600): Support immediate data compatible.
    void OnSetPipeline(PipelineType* pipeline) {
        if (mLastPipeline == pipeline) {
            return;
        }

        mDirty = pipeline->GetImmediateMask();
        mLastPipeline = pipeline;
    }

    // Getters
    const ImmediateConstantMask& GetDirtyBits() const { return mDirty; }

    const ImmediateDataContent<T>& GetContent() const { return mContent; }

    void SetDirtyBitsForTesting(ImmediateConstantMask dirtyBits) { mDirty = dirtyBits; }

  protected:
    template <typename U>
    void UpdateImmediateConstants(size_t dataOffset, const U& data) {
        constexpr size_t dataSize = sizeof(U);
        U* destData = mContent.template Get<U>(dataOffset);
        if (memcmp(destData, &data, dataSize) != 0) {
            memcpy(destData, &data, dataSize);
            mDirty |= GetImmediateConstantBlockBits(dataOffset, dataSize);
        }
    }

    ImmediateDataContent<T> mContent;
    ImmediateConstantMask mDirty = ImmediateConstantMask(0);
    RAW_PTR_EXCLUSION PipelineType* mLastPipeline = nullptr;
};

class RenderImmediateConstantsTrackerBase
    : public UserImmediateConstantsTrackerBase<RenderImmediateConstants, RenderPipelineBase> {
  public:
    RenderImmediateConstantsTrackerBase() = default;
    void SetClampFragDepth(float minClampFragDepth, float maxClampFragDepth);
    void SetFirstIndexOffset(uint32_t firstVertex, uint32_t firstInstance);
    void SetFirstVertex(uint32_t firstVertex);
    void SetFirstInstance(uint32_t firstInstance);
};

class ComputeImmediateConstantsTrackerBase
    : public UserImmediateConstantsTrackerBase<ComputeImmediateConstants, ComputePipelineBase> {
  public:
    ComputeImmediateConstantsTrackerBase() = default;
    void SetNumWorkgroups(uint32_t numWorkgroupX, uint32_t numWorkgroupY, uint32_t numWorkgroupZ);
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_IMMEDIATECONSTANTSTRACKER_H_
