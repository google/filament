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

#include "dawn/tests/DawnTest.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class EntryPointTests : public DawnTest {};

// Test creating a render pipeline from two entryPoints in the same module.
TEST_P(EntryPointTests, FragAndVertexSameModule) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vertex_main() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }

        @fragment fn fragment_main() -> @location(0) vec4f {
          return vec4f(1.0, 0.0, 0.0, 1.0);
        }
    )");

    // Create a point pipeline from the module.
    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = module;
    desc.cFragment.module = module;
    desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    // Render the point and check that it was rendered.
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.Draw(1);
        pass.End();
    }
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPass.color, 0, 0);
}

// Test creating two compute pipelines from the same module.
TEST_P(EntryPointTests, TwoComputeInModule) {
    wgpu::BindGroupLayoutEntry binding = {};
    binding.binding = 0;
    binding.buffer.type = wgpu::BufferBindingType::Storage;
    binding.visibility = wgpu::ShaderStage::Compute;

    wgpu::BindGroupLayoutDescriptor desc = {};
    desc.entryCount = 1;
    desc.entries = &binding;

    wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&desc);

    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;

    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDesc);

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct Data {
            data : u32
        }
        @binding(0) @group(0) var<storage, read_write> data : Data;

        @compute @workgroup_size(1) fn write1() {
            data.data = 1u;
            return;
        }

        @compute @workgroup_size(1) fn write42() {
            data.data = 42u;
            return;
        }
    )");

    // Create both pipelines from the module.
    wgpu::ComputePipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.compute.module = module;

    pipelineDesc.compute.entryPoint = "write1";
    wgpu::ComputePipeline write1 = device.CreateComputePipeline(&pipelineDesc);

    pipelineDesc.compute.entryPoint = "write42";
    wgpu::ComputePipeline write42 = device.CreateComputePipeline(&pipelineDesc);

    // Create the bindGroup.
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 4;
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::BindGroup group = utils::MakeBindGroup(device, bindGroupLayout, {{0, buffer}});

    // Use the first pipeline and check it wrote 1.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(write1);
        pass.SetBindGroup(0, group);
        pass.DispatchWorkgroups(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(1, buffer, 0);
    }

    // Use the second pipeline and check it wrote 42.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(write42);
        pass.SetBindGroup(0, group);
        pass.DispatchWorkgroups(42);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(42, buffer, 0);
    }
}

DAWN_INSTANTIATE_TEST(EntryPointTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
