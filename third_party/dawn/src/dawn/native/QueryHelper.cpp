// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/native/QueryHelper.h"

#include <algorithm>
#include <cmath>

#include "dawn/native/BindGroup.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/ComputePassEncoder.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {

namespace {

// Assert the offsets in dawn::native::TimestampParams are same with the ones in the shader
static_assert(offsetof(dawn::native::TimestampParams, first) == 0);
static_assert(offsetof(dawn::native::TimestampParams, count) == 4);
static_assert(offsetof(dawn::native::TimestampParams, offset) == 8);
static_assert(offsetof(dawn::native::TimestampParams, quantizationMask) == 12);
static_assert(offsetof(dawn::native::TimestampParams, multiplier) == 16);
static_assert(offsetof(dawn::native::TimestampParams, rightShift) == 20);

static const char sConvertTimestampsToNanoseconds[] = R"(
            struct Timestamp {
                low  : u32,
                high : u32,
            }

            struct TimestampArr {
                t : array<Timestamp>
            }

            struct AvailabilityArr {
                v : array<u32>
            }

            struct TimestampParams {
                first  : u32,
                count  : u32,
                offset : u32,
                quantization_mask : u32,
                multiplier : u32,
                right_shift  : u32,
            }

            @group(0) @binding(0) var<storage, read_write> timestamps : TimestampArr;
            @group(0) @binding(1) var<storage, read> availability : AvailabilityArr;
            @group(0) @binding(2) var<uniform> params : TimestampParams;

            const sizeofTimestamp : u32 = 8u;

            @compute @workgroup_size(8, 1, 1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
                if (GlobalInvocationID.x >= params.count) { return; }

                var index = GlobalInvocationID.x + params.offset / sizeofTimestamp;

                // Return 0 for the unavailable value.
                if (availability.v[GlobalInvocationID.x + params.first] == 0u) {
                    timestamps.t[index].low = 0u;
                    timestamps.t[index].high = 0u;
                    return;
                }

                var timestamp = timestamps.t[index];

                // TODO(dawn:1250): Consider using the umulExtended and uaddCarry intrinsics once
                // available.
                var chunks : array<u32, 5>;
                chunks[0] = timestamp.low & 0xFFFFu;
                chunks[1] = timestamp.low >> 16u;
                chunks[2] = timestamp.high & 0xFFFFu;
                chunks[3] = timestamp.high >> 16u;
                chunks[4] = 0u;

                // Multiply all the chunks with the integer period.
                for (var i = 0u; i < 4u; i = i + 1u) {
                    chunks[i] = chunks[i] * params.multiplier;
                }

                // Propagate the carry
                var carry = 0u;
                for (var i = 0u; i < 4u; i = i + 1u) {
                    var chunk_with_carry = chunks[i] + carry;
                    carry = chunk_with_carry >> 16u;
                    chunks[i] = chunk_with_carry & 0xFFFFu;
                }
                chunks[4] = carry;

                // Apply the right shift.
                for (var i = 0u; i < 4u; i = i + 1u) {
                    var low = chunks[i] >> params.right_shift;
                    var high = (chunks[i + 1u] << (16u - params.right_shift)) & 0xFFFFu;
                    chunks[i] = low | high;
                }

                // Apply quantization mask.
                var low = chunks[0] | (chunks[1] << 16u);
                timestamps.t[index].low = low & params.quantization_mask;
                timestamps.t[index].high = chunks[2] | (chunks[3] << 16u);
            }
        )";

ResultOrError<ComputePipelineBase*> GetOrCreateTimestampComputePipeline(DeviceBase* device) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();

    if (store->timestampComputePipeline == nullptr) {
        // Create compute shader module if not cached before.
        if (store->timestampCS == nullptr) {
            DAWN_TRY_ASSIGN(store->timestampCS,
                            utils::CreateShaderModule(device, sConvertTimestampsToNanoseconds));
        }

        // Create binding group layout
        Ref<BindGroupLayoutBase> bgl;
        DAWN_TRY_ASSIGN(
            bgl, utils::MakeBindGroupLayout(
                     device,
                     {
                         {0, wgpu::ShaderStage::Compute, kInternalStorageBufferBinding},
                         {1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage},
                         {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform},
                     },
                     /* allowInternalBinding */ true));

        // Create pipeline layout
        Ref<PipelineLayoutBase> layout;
        DAWN_TRY_ASSIGN(layout, utils::MakeBasicPipelineLayout(device, bgl));

        // Create ComputePipeline.
        ComputePipelineDescriptor computePipelineDesc = {};
        // Generate the layout based on shader module.
        computePipelineDesc.layout = layout.Get();
        computePipelineDesc.compute.module = store->timestampCS.Get();
        computePipelineDesc.compute.entryPoint = "main";

        DAWN_TRY_ASSIGN(store->timestampComputePipeline,
                        device->CreateComputePipeline(&computePipelineDesc));
    }

    return store->timestampComputePipeline.Get();
}

}  // anonymous namespace

TimestampParams::TimestampParams(uint32_t first,
                                 uint32_t count,
                                 uint32_t offset,
                                 uint32_t quantizationMask,
                                 float period)
    : first(first), count(count), offset(offset), quantizationMask(quantizationMask) {
    // The overall conversion happening, if p is the period, m the multiplier, s the shift, is::
    //
    //   m = round(p * 2^s)
    //
    // Then in the shader we compute:
    //
    //   m / 2^s = round(p * 2^s) / 2*s ~= p
    //
    // The goal is to find the best shift to keep the precision of computations. The
    // conversion shader uses chunks of 16 bits to compute the multiplication with the perios,
    // so we need to keep the multiplier under 2^16. At the same time, the larger the
    // multiplier, the better the precision, so we maximize the value of the right shift while
    // keeping the multiplier under 2 ^ 16
    uint32_t upperLog2 = ceil(log2(period));

    // Clamp the shift to 16 because we're doing computations in 16bit chunks. The
    // multiplication by the period will overflow the chunks, but timestamps are mostly
    // informational so that's ok.
    rightShift = 16u - std::min(upperLog2, 16u);
    multiplier = uint32_t(period * (1 << rightShift));
}

MaybeError EncodeConvertTimestampsToNanoseconds(CommandEncoder* encoder,
                                                BufferBase* timestamps,
                                                BufferBase* availability,
                                                BufferBase* params) {
    DeviceBase* device = encoder->GetDevice();
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());

    ComputePipelineBase* pipeline;
    DAWN_TRY_ASSIGN(pipeline, GetOrCreateTimestampComputePipeline(device));

    // Prepare bind group layout.
    Ref<BindGroupLayoutBase> layout;
    DAWN_TRY_ASSIGN(layout, pipeline->GetBindGroupLayout(0));

    // Create bind group after all binding entries are set.
    Ref<BindGroupBase> bindGroup;
    DAWN_TRY_ASSIGN(
        bindGroup,
        utils::MakeBindGroup(device, layout, {{0, timestamps}, {1, availability}, {2, params}},
                             UsageValidationMode::Internal));

    // Create compute encoder and issue dispatch.
    Ref<ComputePassEncoder> pass = encoder->BeginComputePass();
    pass->APISetPipeline(pipeline);
    pass->APISetBindGroup(0, bindGroup.Get());
    pass->APIDispatchWorkgroups(
        static_cast<uint32_t>((timestamps->GetSize() / sizeof(uint64_t) + 7) / 8));
    pass->APIEnd();

    return {};
}

}  // namespace dawn::native
