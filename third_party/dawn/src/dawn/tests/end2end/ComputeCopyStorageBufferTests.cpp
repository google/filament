// Copyright 2018 The Dawn & Tint Authors
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

#include <array>

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ComputeCopyStorageBufferTests : public DawnTest {
  public:
    static constexpr int kInstances = 4;
    static constexpr int kUintsPerInstance = 4;
    static constexpr int kNumUints = kInstances * kUintsPerInstance;

    void BasicTest(const char* shader);
};

void ComputeCopyStorageBufferTests::BasicTest(const char* shader) {
    // Set up shader and pipeline
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Set up src storage buffer
    wgpu::BufferDescriptor srcDesc;
    srcDesc.size = kNumUints * sizeof(uint32_t);
    srcDesc.usage =
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer src = device.CreateBuffer(&srcDesc);

    std::array<uint32_t, kNumUints> expected;
    for (uint32_t i = 0; i < kNumUints; ++i) {
        expected[i] = (i + 1u) * 0x11111111u;
    }
    queue.WriteBuffer(src, 0, expected.data(), sizeof(expected));
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), src, 0, kNumUints);

    // Set up dst storage buffer
    wgpu::BufferDescriptor dstDesc;
    dstDesc.size = kNumUints * sizeof(uint32_t);
    dstDesc.usage =
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dst = device.CreateBuffer(&dstDesc);

    std::array<uint32_t, kNumUints> zero{};
    queue.WriteBuffer(dst, 0, zero.data(), sizeof(zero));

    // Set up bind group and issue dispatch
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, src, 0, kNumUints * sizeof(uint32_t)},
                                                         {1, dst, 0, kNumUints * sizeof(uint32_t)},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(kInstances);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), dst, 0, kNumUints);
}

// Test that a trivial compute-shader memcpy implementation works.
TEST_P(ComputeCopyStorageBufferTests, SizedArrayOfBasic) {
    BasicTest(R"(
        struct Buf {
            s : array<vec4u, 4>
        }

        @group(0) @binding(0) var<storage, read_write> src : Buf;
        @group(0) @binding(1) var<storage, read_write> dst : Buf;

        @compute @workgroup_size(1)
        fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
            let index : u32 = GlobalInvocationID.x;
            if (index >= 4u) { return; }
            dst.s[index] = src.s[index];
        })");
}

// Test that a slightly-less-trivial compute-shader memcpy implementation works.
TEST_P(ComputeCopyStorageBufferTests, SizedArrayOfStruct) {
    BasicTest(R"(
        struct S {
            a : vec2u,
            b : vec2u,
        }

        struct Buf {
            s : array<S, 4>
        }

        @group(0) @binding(0) var<storage, read_write> src : Buf;
        @group(0) @binding(1) var<storage, read_write> dst : Buf;

        @compute @workgroup_size(1)
        fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
            let index : u32 = GlobalInvocationID.x;
            if (index >= 4u) { return; }
            dst.s[index] = src.s[index];
        })");
}

// Test that a trivial compute-shader memcpy implementation works.
TEST_P(ComputeCopyStorageBufferTests, UnsizedArrayOfBasic) {
    BasicTest(R"(
        struct Buf {
            s : array<vec4u>
        }

        @group(0) @binding(0) var<storage, read_write> src : Buf;
        @group(0) @binding(1) var<storage, read_write> dst : Buf;

        @compute @workgroup_size(1)
        fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
            let index : u32 = GlobalInvocationID.x;
            if (index >= 4u) { return; }
            dst.s[index] = src.s[index];
        })");
}

DAWN_INSTANTIATE_TEST(ComputeCopyStorageBufferTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
