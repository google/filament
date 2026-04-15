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

#ifndef SRC_DAWN_COMMON_CONSTANTS_H_
#define SRC_DAWN_COMMON_CONSTANTS_H_

#include <cstddef>
#include <cstdint>

namespace dawn {

inline constexpr uint32_t kMaxBindGroups = 4u;
inline constexpr uint32_t kMaxBindingsPerBindGroup = 1000u;
inline constexpr uint8_t kMaxVertexAttributes = 30u;
inline constexpr uint8_t kMaxVertexBuffers = 8u;
inline constexpr uint32_t kMaxVertexBufferArrayStride = 2048u;
inline constexpr uint32_t kMaxBindGroupsPlusVertexBuffers = 24u;
inline constexpr uint32_t kNumStages = 3;
inline constexpr uint8_t kMaxColorAttachments = 8u;
inline constexpr uint32_t kTextureBytesPerRowAlignment = 256u;
inline constexpr uint32_t kQueryResolveAlignment = 256u;
inline constexpr uint32_t kMaxInterStageShaderVariables = 16u;
inline constexpr uint64_t kAssumedMaxBufferSize =
    0x80000000u;  // Use 2 GB when the limit is unavailable

// All Immediate constants are 32 bit
inline constexpr uint32_t kImmediateConstantElementByteSize = sizeof(uint32_t);

// Total number of "internal" 32-bit immediates, which includes both external (user) immediates
// and any other immediates used by Dawn internally (e.g. workgroup sizes).
// Vulkan's min-max push constant limit is 128 bytes / 4 = 32 values,
// while D3D12's limit is 256 bytes / 4 = 64 values, so we pick 32 here.
inline constexpr uint32_t kMaxImmediateConstantsPerPipeline = 32u;

// Adapter Max limitation for user immediate constants is 64 bytes.
inline constexpr uint32_t kMaxImmediateDataBytes = 64u;

// Known as 'Immediate Data' that users can update via the API
inline constexpr uint32_t kMaxExternalImmediateConstantsPerPipeline =
    kMaxImmediateDataBytes / kImmediateConstantElementByteSize;

// Default subgroup sizes.
inline constexpr uint32_t kDefaultSubgroupMinSize = 4u;
inline constexpr uint32_t kDefaultSubgroupMaxSize = 128u;

// Per stage maximum limits used to optimized Dawn internals.
inline constexpr uint32_t kMaxSampledTexturesPerShaderStage = 48;
inline constexpr uint32_t kMaxSamplersPerShaderStage = 16;
inline constexpr uint32_t kMaxStorageBuffersPerShaderStage = 16;
inline constexpr uint32_t kMaxStorageTexturesPerShaderStage = 8;
inline constexpr uint32_t kMaxUniformBuffersPerShaderStage = 12;

// Indirect command sizes
inline constexpr uint64_t kDispatchIndirectSize = 3 * sizeof(uint32_t);
inline constexpr uint64_t kDrawIndirectSize = 4 * sizeof(uint32_t);
inline constexpr uint64_t kDrawIndexedIndirectSize = 5 * sizeof(uint32_t);

// Non spec defined constants.
inline constexpr float kLodMin = 0.0;
inline constexpr float kLodMax = 1000.0;

// Offset alignment for CopyB2B. Strictly speaking this alignment is required only
// on macOS, but we decide to do it on all platforms.
inline constexpr uint64_t kCopyBufferToBufferOffsetAlignment = 4u;

// Required offset alignment for GPUTexelBufferViewDescriptor::offset, as specified in
// https://github.com/gpuweb/gpuweb/blob/main/proposals/texel-buffers.md
inline constexpr uint64_t kTexelBufferOffsetAlignment = 256u;

// Metal has a maximum size of 32Kb for a counter set buffer. Each query is 8 bytes.
// So, the maximum nymber of queries is 32Kb / 8.
inline constexpr uint32_t kMaxQueryCount = 4096;

// An external texture occupies multiple binding slots. These are the per-external-texture bindings
// needed.
inline constexpr uint8_t kSampledTexturesPerExternalTexture = 4u;
inline constexpr uint8_t kSamplersPerExternalTexture = 1u;
inline constexpr uint8_t kUniformsPerExternalTexture = 1u;

inline constexpr uint8_t kMaxPLSSlots = 4;
inline constexpr size_t kPLSSlotByteSize = 4;
inline constexpr uint8_t kMaxPLSSize = kMaxPLSSlots * kPLSSlotByteSize;

// Wire buffer alignments.
inline constexpr size_t kWireBufferAlignment = 8u;

// Timestamp query quantization mask to perform a granularity of ~0.1ms.
inline constexpr uint32_t kTimestampQuantizationMask = 0xFFFF0000;

// Max dynamic offset counts used to optimize Dawn internals.
inline constexpr uint32_t kMaxDynamicUniformBuffersPerPipelineLayout = 16u;
inline constexpr uint32_t kMaxDynamicStorageBuffersPerPipelineLayout = 16u;

// Maximum ResourceTable size.
inline constexpr uint32_t kMaxResourceTableSize = 64 * 1024;
// TODO(https://issues.chromium.org/465122000): Find if this is a reasonable amount to
// reserve for placeholders.
inline constexpr uint32_t kReservedResourceTableSlots = 1000;

// Required D3D12 shared buffer memory file mapping handle. The size must be a multiple of
// `D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT` (65536) to hold a D3D12 buffer resource.
inline constexpr uint32_t kD3D12SharedBufferMemoryFileMappingHandleSizeAlignment = 65536;
}  // namespace dawn

#endif  // SRC_DAWN_COMMON_CONSTANTS_H_
