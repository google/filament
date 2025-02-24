// Copyright 2021 The Dawn & Tint Authors
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

#include <utility>
#include <vector>

#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Device.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class InternalStorageBufferBindingTests : public DawnTest {
  protected:
    static constexpr uint32_t kNumValues = 4;
    static constexpr uint32_t kIterations = 4;

    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }

    wgpu::ComputePipeline CreateComputePipelineWithInternalStorage() {
        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            struct Buf {
                data : array<u32, 4>
            }

            @group(0) @binding(0) var<storage, read_write> buf : Buf;

            @compute @workgroup_size(1)
            fn main(@builtin(global_invocation_id) GlobalInvocationID : vec3u) {
                buf.data[GlobalInvocationID.x] = buf.data[GlobalInvocationID.x] + 0x1234u;
            }
        )");

        // Create binding group layout with internal storage buffer binding type
        native::BindGroupLayoutEntry bglEntry;
        bglEntry.binding = 0;
        bglEntry.buffer.type = native::kInternalStorageBufferBinding;
        bglEntry.visibility = wgpu::ShaderStage::Compute;

        native::BindGroupLayoutDescriptor bglDesc;
        bglDesc.entryCount = 1;
        bglDesc.entries = &bglEntry;

        native::DeviceBase* nativeDevice = native::FromAPI(device.Get());

        Ref<native::BindGroupLayoutBase> bglRef =
            nativeDevice->CreateBindGroupLayout(&bglDesc, true).AcquireSuccess();

        wgpu::BindGroupLayout bgl =
            wgpu::BindGroupLayout::Acquire(native::ToAPI(ReturnToAPI(std::move(bglRef))));

        // Create pipeline layout
        wgpu::PipelineLayoutDescriptor plDesc;
        plDesc.bindGroupLayoutCount = 1;
        plDesc.bindGroupLayouts = &bgl;
        wgpu::PipelineLayout layout = device.CreatePipelineLayout(&plDesc);

        wgpu::ComputePipelineDescriptor pipelineDesc = {};
        pipelineDesc.layout = layout;
        pipelineDesc.compute.module = module;

        return device.CreateComputePipeline(&pipelineDesc);
    }
};

// Test that query resolve buffer can be bound as internal storage buffer, multiple dispatches to
// increment values in the query resolve buffer are synchronized.
TEST_P(InternalStorageBufferBindingTests, QueryResolveBufferBoundAsInternalStorageBuffer) {
    std::vector<uint32_t> data(kNumValues, 0);
    std::vector<uint32_t> expected(kNumValues, 0x1234u * kIterations);

    uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));
    wgpu::Buffer buffer =
        utils::CreateBufferFromData(device, data.data(), bufferSize,
                                    wgpu::BufferUsage::QueryResolve | wgpu::BufferUsage::CopySrc);

    wgpu::ComputePipeline pipeline = CreateComputePipelineWithInternalStorage();

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, buffer, 0, bufferSize}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    for (uint32_t i = 0; i < kIterations; ++i) {
        pass.DispatchWorkgroups(kNumValues);
    }
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), buffer, 0, kNumValues);
}

DAWN_INSTANTIATE_TEST(InternalStorageBufferBindingTests,
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
