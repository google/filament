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

#include <limits>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class PackUnpack4x8NormTests : public DawnTest {};

TEST_P(PackUnpack4x8NormTests, Pack4x8Snorm) {
    const char* computeShader = R"(
        @group(0) @binding(0) var<storage, read_write> buf : array<u32>;
        @group(0) @binding(1) var<storage, read> inputBuf : array<vec4f>;

        @compute @workgroup_size(1)
        fn main() {
            var r: vec2<u32>;
            for (var i = 0; i < 8; i++) {
                r.x = pack4x8snorm(inputBuf[i]);
                buf[i] = r.x;
            }
        }
)";

    static uint32_t kNumTests = 8;

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = kNumTests * sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer bufferOut = device.CreateBuffer(&bufferDesc);

    wgpu::Buffer inputBuffer = utils::CreateBufferFromData(
        device,
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
        {
            // clang-format off
            0.f, 0.f, 0.f, 0.f,  //
            0.f, 0.f, 0.f, -1.f,  //
            0.f, 0.f, 0.f, 1.f,  //
            0.f, 0.f, -1.f, 0.f,  //
            0.f, 1.f, 0.f, 0.f,  //
            -1.f, 0.f, 0.f, 0.f,  //
            1.f, -1.f, 1.f, -1.f,  //
            std::numeric_limits<float>::max(), -0.495f, 0.5f, std::numeric_limits<float>::lowest(),
            // clang-format on
        });

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, computeShader);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, bufferOut},
                                                         {1, inputBuffer},
                                                     });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    uint32_t expected[] = {0u,           0x8100'0000u, 0x7f00'0000u, 0x0081'0000u,
                           0x0000'7f00u, 0x0000'0081u, 0x817f'817fu, 0x8140'c17fu};
    EXPECT_BUFFER_U32_RANGE_EQ(reinterpret_cast<uint32_t*>(expected), bufferOut, 0, kNumTests);
}

TEST_P(PackUnpack4x8NormTests, Pack4x8Unorm) {
    const char* computeShader = R"(
        @group(0) @binding(0) var<storage, read_write> buf : array<u32>;
        @group(0) @binding(1) var<storage, read> inputBuf : array<vec4f>;

        @compute @workgroup_size(1)
        fn main() {
            var r: vec2<u32>;
            for (var i = 0; i < 7; i++) {
                r.x = pack4x8unorm(inputBuf[i]);
                buf[i] = r.x;
            }
        }
)";

    static uint32_t kNumTests = 7;

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = kNumTests * sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer bufferOut = device.CreateBuffer(&bufferDesc);

    wgpu::Buffer inputBuffer = utils::CreateBufferFromData(
        device,
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
        {
            // clang-format off
            0.f, 0.f, 0.f, 0.f,  //
            0.f, 0.f, 0.f, 1.f,  //
            0.f, 0.f, 1.f, 0.f,  //
            0.f, 1.f, 0.f, 0.f,  //
            1.f, 0.f, 0.f, 0.f,  //
            1.f, 0.f, 1.f, 0.f,  //
            std::numeric_limits<float>::max(), 0.f, 0.5f, std::numeric_limits<float>::lowest(),
            // clang-format on
        });

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, computeShader);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, bufferOut},
                                                         {1, inputBuffer},
                                                     });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    uint32_t expected[] = {0u,           0xff00'0000u, 0x00ff'0000u, 0x0000'ff00u,
                           0x0000'00ffu, 0x00ff'00ffu, 0x0080'00ffu};
    EXPECT_BUFFER_U32_RANGE_EQ(reinterpret_cast<uint32_t*>(expected), bufferOut, 0, kNumTests);
}

TEST_P(PackUnpack4x8NormTests, Unpack4x8Unorm) {
    const char* computeShader = R"(
        @group(0) @binding(0) var<storage, read_write> buf : array<vec4f>;
        @group(0) @binding(1) var<storage, read> inputBuf : array<u32>;

        @compute @workgroup_size(1)
        fn main() {
            var r: vec2<u32>;
            for (var i = 0; i < 7; i++) {
                r.x = inputBuf[i];
                buf[i] = unpack4x8unorm(r.x);
            }
        }
)";

    static uint32_t kNumTests = 7;

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = kNumTests * 4 * sizeof(float);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer bufferOut = device.CreateBuffer(&bufferDesc);

    wgpu::Buffer inputBuffer = utils::CreateBufferFromData(
        device,
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
        {
            // clang-format off
            0x0000'0000u,
            0xff00'0000u,
            0x00ff'0000u,
            0x0000'ff00u,
            0x0000'00ffu,
            0x00ff'00ffu,
            0x0066'00ffu
            // clang-format on
        });

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, computeShader);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, bufferOut},
                                                         {1, inputBuffer},
                                                     });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    float expected[] = {
        // clang-format off
        0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        1.f, 0.f, 0.f, 0.f,
        1.f, 0.f, 1.f, 0.f,
        1.f, 0.f, 0.4f, 0.f
        // clang-format on
    };
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expected, bufferOut, 0, kNumTests);
}

TEST_P(PackUnpack4x8NormTests, Unpack4x8Snorm) {
    const char* computeShader = R"(
        @group(0) @binding(0) var<storage, read_write> buf : array<vec4f>;
        @group(0) @binding(1) var<storage, read> inputBuf : array<u32>;

        @compute @workgroup_size(1)
        fn main() {
            var r: vec2<u32>;
            for (var i = 0; i < 8; i++) {
                r.x = inputBuf[i];
                buf[i] = unpack4x8snorm(r.x);
            }
        }
)";

    static uint32_t kNumTests = 8;

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = kNumTests * 4 * sizeof(float);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer bufferOut = device.CreateBuffer(&bufferDesc);

    wgpu::Buffer inputBuffer = utils::CreateBufferFromData(
        device,
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
        {
            // clang-format off
            0x0000'0000u,
            0x8100'0000u,
            0x7f00'0000u,
            0x0081'0000u,
            0x0000'7f00u,
            0x0000'0081u,
            0x817f'817fu,
            0x816d'937fu
            // clang-format on
        });

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, computeShader);
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, bufferOut},
                                                         {1, inputBuffer},
                                                     });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    float expected[] = {
        // clang-format off
        0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, -1.f,
        0.f, 0.f, 0.f, 1.f,
        0.f, 0.f, -1.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        -1.f, 0.f, 0.f, 0.f,
        -1.f, 0.f, 1.f, 0.f,
        1.f, -1.f, 1.f, -1.f,
        1.f, -0.8582677165354f, 0.8582677165354f, -1.f
        // clang-format on
    };
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expected, bufferOut, 0, kNumTests);
}

// DawnTestBase::CreateDeviceImpl always enables allow_unsafe_apis toggle.
DAWN_INSTANTIATE_TEST(PackUnpack4x8NormTests,
                      VulkanBackend(),
                      VulkanBackend({"polyfill_pack_unpack_4x8_norm"}));

}  // anonymous namespace
}  // namespace dawn
