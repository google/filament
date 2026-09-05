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

#ifndef SRC_DAWN_NATIVE_IMMEDIATESLAYOUT_H_
#define SRC_DAWN_NATIVE_IMMEDIATESLAYOUT_H_

#include "src/dawn/common/Compiler.h"
#include "src/dawn/common/ityp_bitset.h"
#include "src/dawn/native/EnumClassBitmasks.h"
#include "src/dawn/native/IntegerTypes.h"
#include "src/utils/numeric.h"

namespace dawn::native {

// Define common immediate data layout. Append members to expand layouts.
// NOTE: 'offsetof' doesn't support non-standard-layout structs. So use aggregate instead of
// inheritance for RenderImmediates and ComputeImmediates.
DAWN_ENABLE_STRUCT_PADDING_WARNINGS
struct UserImmediates {
    uint32_t userImmediateData[kMaxExternalImmediatesPerPipeline];
};

// 8 bytes of immediate data data to be used by the ClampFragDepth Tint transform.
struct ClampFragDepthArgs {
    float minClampFragDepth = 0.0f;
    float maxClampFragDepth = 0.0f;
};

struct NumWorkgroupsDimensions {
    uint32_t numWorkgroupsX = 0;
    uint32_t numWorkgroupsY = 0;
    uint32_t numWorkgroupsZ = 0;
};

// A non-constant zero value that is used to workaround buggy optimizations in downstream compilers.
struct NonConstantZero {
    uint32_t zero = 0;
};

DAWN_DISABLE_STRUCT_PADDING_WARNINGS

// Convert byte sizes and offsets into immediate indices and offsets
// (dividing everything by kImmediateElementByteSize)
constexpr ImmediateMask GetImmediateBlockBits(size_t byteOffset, size_t byteSize) {
    // This bit math can be done in uint64_t because there are <= 64 bits in the mask.
    static_assert(ImmediateMask{}.size() <= 64);
    uint64_t firstIndex = byteOffset / kImmediateElementByteSize;
    uint64_t constantCount = byteSize / kImmediateElementByteSize;
    return ((uint64_t{1} << constantCount) - uint64_t{1}) << firstIndex;
}

// Returns the offset of the member in the packed immediates of the pipeline.
// The pointer-to-member is a pointer into the structure containing all the potential immediates.
// However pipelines don't need all of them and use a compacted layout with immediates in the same
// order, just some of them skipped. For example the pipeline mask 11001111, representing
// "userImmediates: 4 | trivial_constants: 0 (2 at most)|clamp_frag:2", maps to pipeline immediate
// layout: "userImmediates:4 | clamp_frag:2
template <typename Object, typename Member>
uint32_t GetImmediateByteOffsetInPipeline(Member Object::* ptr,
                                          const ImmediateMask& pipelineImmediateMask) {
    Object obj = {};
    size_t offset = sign_cast(reinterpret_cast<char*>(&(obj.*ptr)) - reinterpret_cast<char*>(&obj));

    const ImmediateMask prefixBits = (uint64_t{1} << (offset / kImmediateElementByteSize)) - 1u;

    return static_cast<uint32_t>((prefixBits & pipelineImmediateMask).count() *
                                 kImmediateElementByteSize);
}

template <typename Object, typename Member>
bool HasImmediates(Member Object::* ptr, const ImmediateMask& pipelineImmediateMask) {
    Object obj = {};
    size_t offset = sign_cast(reinterpret_cast<char*>(&(obj.*ptr)) - reinterpret_cast<char*>(&obj));
    size_t size = sizeof(Member);

    return (pipelineImmediateMask & GetImmediateBlockBits(offset, size)).any();
}

template <typename Object, typename Member>
std::optional<uint32_t> GetImmediateByteOffsetInPipelineIfAny(
    Member Object::* ptr,
    const ImmediateMask& pipelineImmediateMask) {
    if (!HasImmediates(ptr, pipelineImmediateMask)) {
        return std::nullopt;
    }

    return GetImmediateByteOffsetInPipeline(ptr, pipelineImmediateMask);
}

uint32_t GetImmediateIndexInPipeline(const uint32_t layoutOffset,
                                     const ImmediateMask& pipelineImmediateMask);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_IMMEDIATESLAYOUT_H_
