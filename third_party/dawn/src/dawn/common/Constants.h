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

static constexpr uint32_t kMaxBindGroups = 4u;
static constexpr uint32_t kMaxBindingsPerBindGroup = 1000u;
static constexpr uint8_t kMaxVertexAttributes = 30u;
static constexpr uint8_t kMaxVertexBuffers = 8u;
static constexpr uint32_t kMaxVertexBufferArrayStride = 2048u;
static constexpr uint32_t kMaxBindGroupsPlusVertexBuffers = 24u;
static constexpr uint32_t kNumStages = 3;
static constexpr uint8_t kMaxColorAttachments = 8u;
static constexpr uint32_t kTextureBytesPerRowAlignment = 256u;
static constexpr uint32_t kQueryResolveAlignment = 256u;
static constexpr uint32_t kMaxInterStageShaderVariables = 16u;
static constexpr uint64_t kAssumedMaxBufferSize =
    0x80000000u;  // Use 2 GB when the limit is unavailable

// All Immediate constants are 32 bit
static constexpr uint32_t kImmediateConstantElementByteSize = sizeof(uint32_t);

// Known as 'Immediate Data'. User could update them through APIs.
static constexpr uint32_t kMaxExternalImmediateConstantsPerPipeline = 16u;

// Vulkan requires min-max push constant bytes is 128 byte, which is
// equals to 32 32bit constants. D3D12 requires 64 32bit constants limits.
// Pick 32 here.
static constexpr uint32_t kMaxImmediateConstantsPerPipeline = 32u;

// Adapter Max limitation for user immediate constants is 64 bytes.
static constexpr uint32_t kMaxSupportedImmediateDataBytes = 64u;

// Device Default limitation for user immediate constants is 16 bytes.
static constexpr uint32_t kDefaultMaxImmediateDataBytes = 16u;

// Default subgroup sizes.
static constexpr uint32_t kDefaultSubgroupMinSize = 4u;
static constexpr uint32_t kDefaultSubgroupMaxSize = 128u;

// Per stage maximum limits used to optimized Dawn internals.
static constexpr uint32_t kMaxSampledTexturesPerShaderStage = 16;
static constexpr uint32_t kMaxSamplersPerShaderStage = 16;
static constexpr uint32_t kMaxStorageBuffersPerShaderStage = 10;
static constexpr uint32_t kMaxStorageTexturesPerShaderStage = 8;
static constexpr uint32_t kMaxUniformBuffersPerShaderStage = 12;

// Indirect command sizes
static constexpr uint64_t kDispatchIndirectSize = 3 * sizeof(uint32_t);
static constexpr uint64_t kDrawIndirectSize = 4 * sizeof(uint32_t);
static constexpr uint64_t kDrawIndexedIndirectSize = 5 * sizeof(uint32_t);

// Non spec defined constants.
static constexpr float kLodMin = 0.0;
static constexpr float kLodMax = 1000.0;

// Offset alignment for CopyB2B. Strictly speaking this alignment is required only
// on macOS, but we decide to do it on all platforms.
static constexpr uint64_t kCopyBufferToBufferOffsetAlignment = 4u;

// Metal has a maximum size of 32Kb for a counter set buffer. Each query is 8 bytes.
// So, the maximum nymber of queries is 32Kb / 8.
static constexpr uint32_t kMaxQueryCount = 4096;

// An external texture occupies multiple binding slots. These are the per-external-texture bindings
// needed.
static constexpr uint8_t kSampledTexturesPerExternalTexture = 4u;
static constexpr uint8_t kSamplersPerExternalTexture = 1u;
static constexpr uint8_t kUniformsPerExternalTexture = 1u;

static constexpr uint8_t kMaxPLSSlots = 4;
static constexpr size_t kPLSSlotByteSize = 4;
static constexpr uint8_t kMaxPLSSize = kMaxPLSSlots * kPLSSlotByteSize;

// Wire buffer alignments.
static constexpr size_t kWireBufferAlignment = 8u;

// Timestamp query quantization mask to perform a granularity of ~0.1ms.
static constexpr uint32_t kTimestampQuantizationMask = 0xFFFF0000;

// Max dynamic offset counts used to optimize Dawn internals.
static constexpr uint32_t kMaxDynamicUniformBuffersPerPipelineLayout = 16u;
static constexpr uint32_t kMaxDynamicStorageBuffersPerPipelineLayout = 16u;

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_CONSTANTS_H_
