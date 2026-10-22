// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_IMMEDIATESTRACKER_H_
#define SRC_DAWN_NATIVE_IMMEDIATESTRACKER_H_

#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>

#include "ImmediatesLayout.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"
#include "src/dawn/common/Constants.h"
#include "src/dawn/common/ityp_bitset.h"
#include "src/dawn/native/Device.h"
#include "src/dawn/native/ImmediatesLayout.h"
#include "src/dawn/native/IntegerTypes.h"
#include "src/dawn/native/Pipeline.h"
#include "src/utils/compiler.h"
#include "src/utils/span.h"

namespace dawn::native {

template <typename T>
struct ImmediateDataContent {
  public:
    // TODO(https://crbug.com/532946455): Spanify these two Get() methods.
    template <typename Out>
    const Out* Get(size_t offset) const {
        DAWN_ASSERT(sizeof(Out) + offset <= sizeof(T));
        return reinterpret_cast<const Out*>(&ByteSpanFromRef(mData)[offset]);
    }
    template <typename Out>
    Out* Get(size_t offset) {
        DAWN_ASSERT(sizeof(Out) + offset <= sizeof(T));
        return reinterpret_cast<Out*>(&ByteSpanFromRef(mData)[offset]);
    }

    Span<const std::byte> GetDataBytes(size_t offset, size_t size) const {
        return ByteSpanFromRef(mData).subspan(offset, size);
    }
    Span<std::byte> GetDataBytes(size_t offset, size_t size) {
        return ByteSpanFromRef(mData).subspan(offset, size);
    }

  private:
    T mData{};
};

// TODO(crbug.com/366291600): Add inheritance ability(like BindGroupTracker) so that it can inherit
// immediates in native backend if supported.
template <typename T, typename PipelineType>
class UserImmediatesTrackerBase {
  public:
    UserImmediatesTrackerBase() {}

    // Setters
    void SetImmediates(size_t offset, Span<const std::byte> data) {
        WriteImmediates(offsetof(T, userImmediates) + offset, data);
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
    const ImmediateMask& GetDirtyBits() const { return mDirty; }
    const ImmediateDataContent<T>& GetContent() const { return mContent; }

    void SetDirtyBitsForTesting(ImmediateMask dirtyBits) { mDirty = dirtyBits; }

  protected:
    template <typename U>
    void UpdateImmediates(size_t offset, const U& data) {
        WriteImmediates(offset, ByteSpanFromRef(data));
    }

    // Writes data into the immediate content at offset, updating mDirty if needed.
    void WriteImmediates(size_t offset, Span<const std::byte> data) {
        Span<std::byte> dest = mContent.GetDataBytes(offset, data.size());
        if (!std::ranges::equal(data, dest)) {
            dest.CopyFrom(data);
            mDirty |= GetImmediateBlockBits(offset, data.size());
        }
    }

    ImmediateDataContent<T> mContent;
    ImmediateMask mDirty = ImmediateMask(0);
    RAW_PTR_EXCLUSION PipelineType* mLastPipeline = nullptr;
};
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_IMMEDIATESTRACKER_H_
