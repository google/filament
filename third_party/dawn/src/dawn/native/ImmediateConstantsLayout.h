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

#ifndef SRC_DAWN_NATIVE_IMMEDIATECONSTANTSLAYOUT_H_
#define SRC_DAWN_NATIVE_IMMEDIATECONSTANTSLAYOUT_H_

#include "dawn/common/Compiler.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/EnumClassBitmasks.h"
#include "dawn/native/IntegerTypes.h"

namespace dawn::native {

// Define common immediate data layout. Append members to expand layouts.
// NOTE: 'offsetof' doesn't support non-standard-layout structs. So use
// aggregate instead of inheritance for RenderImmediateConstants and
// ComputeImmediateConstants.
DAWN_ENABLE_STRUCT_PADDING_WARNINGS
struct UserImmediateConstants {
    uint32_t userImmediateData[kMaxExternalImmediateConstantsPerPipeline];
};

// 8 bytes of push constant data to be used by the ClampFragDepth Tint transform.
struct ClampFragDepthArgs {
    float minClampFragDepth;
    float maxClampFragDepth;
};

// Define render pipeline immediate data layout. Append members to
// expand the layout.
struct RenderImmediateConstants {
    UserImmediateConstants userConstants;

    ClampFragDepthArgs clampFragDepth;

    // first index offset
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct NumWorkgroupsDimensions {
    uint32_t numWorkgroupsX;
    uint32_t numWorkgroupsY;
    uint32_t numWorkgroupsZ;
};

// Define compute pipeline immediate data layout. Append members to
// expand the layout.
struct ComputeImmediateConstants {
    UserImmediateConstants userConstants;

    NumWorkgroupsDimensions numWorkgroups;
};
DAWN_DISABLE_STRUCT_PADDING_WARNINGS

// Convert byte sizes and offsets into immediate constant indices and offsets
// (dividing everything by kImmediateConstantElementByteSize)
constexpr ImmediateConstantMask GetImmediateConstantBlockBits(size_t byteOffset, size_t byteSize) {
    size_t firstIndex = byteOffset / kImmediateConstantElementByteSize;
    size_t constantCount = byteSize / kImmediateConstantElementByteSize;

    return ((1u << constantCount) - 1u) << firstIndex;
}

// Returns the offset of the member in the packed immediates of the pipeline.
// The pointer-to-member is a pointer into the structure containing all the potential immediates.
// However pipelines don't need all of them and use a compacted layout with immediates
// in the same order, just some of them skipped. For example the pipeline mask 11001111,
// representing "userConstants: 4 | trivial_constants: 0 (2 at most)|clamp_frag:2",
// maps to pipeline immediate constant layout: "userConstants:4 | clamp_frag:2
template <typename Object, typename Member>
uint32_t GetImmediateByteOffsetInPipeline(Member Object::*ptr,
                                          const ImmediateConstantMask& pipelineImmediateMask) {
    Object obj = {};
    ptrdiff_t offset = reinterpret_cast<char*>(&(obj.*ptr)) - reinterpret_cast<char*>(&obj);

    const ImmediateConstantMask prefixBits =
        (1u << (offset / kImmediateConstantElementByteSize)) - 1u;

    return (prefixBits & pipelineImmediateMask).count() * kImmediateConstantElementByteSize;
}

template <typename Object, typename Member>
bool HasImmediateConstants(Member Object::*ptr,
                           const ImmediateConstantMask& pipelineImmediateMask) {
    Object obj = {};
    ptrdiff_t offset = reinterpret_cast<char*>(&(obj.*ptr)) - reinterpret_cast<char*>(&obj);
    size_t size = sizeof(Member);

    return pipelineImmediateMask.to_ulong() &
           GetImmediateConstantBlockBits(offset, size).to_ulong();
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_IMMEDIATECONSTANTSLAYOUT_H_
