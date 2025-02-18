// Copyright 2019 The Dawn & Tint Authors
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

class ComputeSharedMemoryTests : public DawnTest {
  public:
    static constexpr uint32_t kInstances = 11;

    void BasicTest(const char* shader);
};

void ComputeSharedMemoryTests::BasicTest(const char* shader) {
    // Set up shader and pipeline
    auto module = utils::CreateShaderModule(device, shader);

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Set up dst storage buffer
    wgpu::BufferDescriptor dstDesc;
    dstDesc.size = sizeof(uint32_t);
    dstDesc.usage =
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dst = device.CreateBuffer(&dstDesc);

    const uint32_t zero = 0;
    queue.WriteBuffer(dst, 0, &zero, sizeof(zero));

    // Set up bind group and issue dispatch
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, dst, 0, sizeof(uint32_t)},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    const uint32_t expected = kInstances;
    EXPECT_BUFFER_U32_RANGE_EQ(&expected, dst, 0, 1);
}

// Basic shared memory test
TEST_P(ComputeSharedMemoryTests, Basic) {
    BasicTest(R"(
        const kTileSize : u32 = 4u;
        const kInstances : u32 = 11u;

        struct Dst {
            x : u32
        }

        @group(0) @binding(0) var<storage, read_write> dst : Dst;
        var<workgroup> tmp : u32;

        @compute @workgroup_size(4,4,1)
        fn main(@builtin(local_invocation_id) LocalInvocationID : vec3u) {
            let index : u32 = LocalInvocationID.y * kTileSize + LocalInvocationID.x;
            if (index == 0u) {
                tmp = 0u;
            }
            workgroupBarrier();
            for (var i : u32 = 0u; i < kInstances; i = i + 1u) {
                if (i == index) {
                    tmp = tmp + 1u;
                }
                workgroupBarrier();
            }
            if (index == 0u) {
                dst.x = tmp;
            }
        })");
}

// Test using assorted types in workgroup memory. MSL lacks constructors
// for matrices in threadgroup memory. Basic test that reading and
// writing a matrix in workgroup memory works.
TEST_P(ComputeSharedMemoryTests, AssortedTypes) {
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = utils::CreateShaderModule(device, R"(
        struct StructValues {
            m: mat2x2<f32>
        }

        struct Dst {
            d_struct : StructValues,
            d_matrix : mat2x2<f32>,
            d_array : array<u32, 4>,
            d_vector : vec4f,
        }

        @group(0) @binding(0) var<storage, read_write> dst : Dst;

        var<workgroup> wg_struct : StructValues;
        var<workgroup> wg_matrix : mat2x2<f32>;
        var<workgroup> wg_array : array<u32, 4>;
        var<workgroup> wg_vector : vec4f;

        @compute @workgroup_size(4,1,1)
        fn main(@builtin(local_invocation_id) LocalInvocationID : vec3u) {

            let i = 4u * LocalInvocationID.x;
            if (LocalInvocationID.x == 0u) {
                wg_struct.m = mat2x2<f32>(
                    vec2f(f32(i), f32(i + 1u)),
                    vec2f(f32(i + 2u), f32(i + 3u)));
            } else if (LocalInvocationID.x == 1u) {
                wg_matrix = mat2x2<f32>(
                    vec2f(f32(i), f32(i + 1u)),
                    vec2f(f32(i + 2u), f32(i + 3u)));
            } else if (LocalInvocationID.x == 2u) {
                wg_array[0u] = i;
                wg_array[1u] = i + 1u;
                wg_array[2u] = i + 2u;
                wg_array[3u] = i + 3u;
            } else if (LocalInvocationID.x == 3u) {
                wg_vector = vec4f(
                    f32(i), f32(i + 1u), f32(i + 2u), f32(i + 3u));
            }

            workgroupBarrier();

            if (LocalInvocationID.x == 0u) {
                dst.d_struct = wg_struct;
                dst.d_matrix = wg_matrix;
                dst.d_array = wg_array;
                dst.d_vector = wg_vector;
            }
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Set up dst storage buffer
    wgpu::BufferDescriptor dstDesc;
    dstDesc.size = 64;
    dstDesc.usage =
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dst = device.CreateBuffer(&dstDesc);

    // Set up bind group and issue dispatch
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, dst},
                                                     });

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);

    std::array<float, 4> expectedStruct = {0., 1., 2., 3.};
    std::array<float, 4> expectedMatrix = {4., 5., 6., 7.};
    std::array<uint32_t, 4> expectedArray = {8, 9, 10, 11};
    std::array<float, 4> expectedVector = {12., 13., 14., 15.};
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedStruct.data(), dst, 0, 4);
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedMatrix.data(), dst, 16, 4);
    EXPECT_BUFFER_U32_RANGE_EQ(expectedArray.data(), dst, 32, 4);
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedVector.data(), dst, 48, 4);
}

DAWN_INSTANTIATE_TEST(ComputeSharedMemoryTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      VulkanBackend({}, {"use_vulkan_zero_initialize_workgroup_memory_extension"}));

}  // anonymous namespace
}  // namespace dawn
