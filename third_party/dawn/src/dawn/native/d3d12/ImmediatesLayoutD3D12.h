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

#ifndef SRC_DAWN_NATIVE_D3D12_IMMEDIATESLAYOUTD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_IMMEDIATESLAYOUTD3D12_H_

#include <array>

#include "src/dawn/common/Constants.h"
#include "src/dawn/native/ImmediatesLayout.h"

namespace dawn::native::d3d12 {

// Define D3D12 immediates layout. Append members to expand layouts.
// NOTE: 'offsetof' doesn't support non-standard-layout structs. So use aggregate instead of
// inheritance for RenderImmediates and ComputeImmediates. Wrap userImmediates to fit offsetof logic
// in UserImmediatesTrackerBase.
DAWN_ENABLE_STRUCT_PADDING_WARNINGS
// firstVertex and firstInstance are always set together in the immediate mask (firstVertex leads),
// so group them so callers reference the pair instead of risking setting only one of them.
struct FirstIndexOffset {
    uint32_t firstVertex;
    uint32_t firstInstance;
};

using DynamicStorageBufferLengths =
    std::array<uint32_t, kMaxDynamicStorageBuffersPerPipelineLayout>;
using DynamicStorageBufferOffsets =
    std::array<uint32_t, kMaxDynamicStorageBuffersPerPipelineLayout>;

// Define render pipeline immediates layout. Append members to expand the layout.
struct RenderImmediates {
    UserImmediates userImmediates;

    FirstIndexOffset firstIndexOffset;

    DynamicStorageBufferLengths storageBufferDynamicLengths;

    DynamicStorageBufferOffsets storageBufferDynamicOffsets;
};

// Define compute pipeline immediates layout. Append members to expand the layout.
struct ComputeImmediates {
    UserImmediates userImmediates;

    NumWorkgroupsDimensions numWorkgroups;

    DynamicStorageBufferLengths storageBufferDynamicLengths;

    DynamicStorageBufferOffsets storageBufferDynamicOffsets;
};
DAWN_DISABLE_STRUCT_PADDING_WARNINGS

static_assert(sizeof(ComputeImmediates) <=
                  size_t{kMaxImmediateMaskBits} * kImmediateElementByteSize,
              "ComputeImmediates must fit within the immediate mask budget.");
static_assert(sizeof(RenderImmediates) <= size_t{kMaxImmediateMaskBits} * kImmediateElementByteSize,
              "RenderImmediates must fit within the immediate mask budget.");

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_IMMEDIATESLAYOUTD3D12_H_
