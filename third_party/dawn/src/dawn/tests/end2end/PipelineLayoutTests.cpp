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

#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class PipelineLayoutTests : public DawnTest {
  protected:
    wgpu::Limits GetRequiredLimits(const wgpu::Limits& supported) override {
        // TODO(crbug.com/383593270): Enable all the limits.
        wgpu::Limits required = {};
        required.maxStorageBuffersInFragmentStage = supported.maxStorageBuffersInFragmentStage;
        required.maxStorageBuffersPerShaderStage = supported.maxStorageBuffersPerShaderStage;
        return required;
    }
};

// Test creating a PipelineLayout with multiple BGLs where the first BGL uses the max number of
// dynamic buffers. This is a regression test for crbug.com/dawn/449 which would overflow when
// dynamic offset bindings were at max. Test is successful if the pipeline layout is created
// without error.
TEST_P(PipelineLayoutTests, DynamicBuffersOverflow) {
    DAWN_SUPPRESS_TEST_IF(GetSupportedLimits().maxStorageBuffersInFragmentStage < 1);

    // Create the first bind group layout which uses max number of dynamic buffers bindings.
    wgpu::BindGroupLayout bglA;
    {
        std::vector<wgpu::BindGroupLayoutEntry> entries;
        for (uint32_t i = 0; i < GetSupportedLimits().maxDynamicStorageBuffersPerPipelineLayout;
             i++) {
            wgpu::BindGroupLayoutEntry entry;
            entry.binding = i;
            entry.visibility = wgpu::ShaderStage::Compute;
            entry.buffer.type = wgpu::BufferBindingType::Storage;
            entry.buffer.hasDynamicOffset = true;

            entries.push_back(entry);
        }

        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.entryCount = entries.size();
        descriptor.entries = entries.data();
        bglA = device.CreateBindGroupLayout(&descriptor);
    }

    // Create the second bind group layout that has one non-dynamic buffer binding.
    // It is in the fragment stage to avoid the max per-stage storage buffer limit.
    wgpu::BindGroupLayout bglB;
    {
        wgpu::BindGroupLayoutDescriptor descriptor;
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::Fragment;
        entry.buffer.type = wgpu::BufferBindingType::Storage;

        descriptor.entryCount = 1;
        descriptor.entries = &entry;
        bglB = device.CreateBindGroupLayout(&descriptor);
    }

    // Create a pipeline layout using both bind group layouts.
    wgpu::PipelineLayoutDescriptor descriptor;
    std::vector<wgpu::BindGroupLayout> bindgroupLayouts = {bglA, bglB};
    descriptor.bindGroupLayoutCount = bindgroupLayouts.size();
    descriptor.bindGroupLayouts = bindgroupLayouts.data();
    device.CreatePipelineLayout(&descriptor);
}

// Regression test for crbug.com/dawn/1689. Test using a compute pass and a render pass,
// where the two pipelines have the same pipeline layout.
TEST_P(PipelineLayoutTests, ComputeAndRenderSamePipelineLayout) {
    wgpu::TextureFormat format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, R"(
        @compute @workgroup_size(8, 8)
        fn computeMain() {}

        @vertex fn vertexMain() -> @builtin(position) vec4f {
            return vec4f(0.0);
        }

        @fragment fn fragmentMain() -> @location(0) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
    )");

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform}});

    wgpu::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);
    wgpu::ComputePipeline computePipeline;
    {
        wgpu::ComputePipelineDescriptor desc = {};
        desc.layout = pl;
        desc.compute.module = shaderModule;
        computePipeline = device.CreateComputePipeline(&desc);
    }
    wgpu::RenderPipeline renderPipeline;
    {
        wgpu::RenderPipelineDescriptor desc = {};
        desc.layout = pl;
        desc.vertex.module = shaderModule;

        wgpu::FragmentState fragment = {};
        desc.fragment = &fragment;
        fragment.module = shaderModule;
        fragment.targetCount = 1;

        wgpu::ColorTargetState colorTargetState = {};
        colorTargetState.format = format;
        fragment.targets = &colorTargetState;

        renderPipeline = device.CreateRenderPipeline(&desc);
    }

    wgpu::Buffer buffer = utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {1});
    wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl, {{0, buffer}});
    wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgl, {{0, buffer}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(computePipeline);
        pass.SetBindGroup(0, bg0);
        pass.DispatchWorkgroups(1);
        pass.End();
    }
    {
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 4, 4, format);
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, bg1);
        pass.Draw(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

// Test creating a PipelineLayout with null and non-null bind group layouts work correctly.
TEST_P(PipelineLayoutTests, PipelineLayoutCreatedWithNullBindGroupLayout) {
    for (uint32_t nonEmptyGroupIndex = 0; nonEmptyGroupIndex <= 1; ++nonEmptyGroupIndex) {
        std::ostringstream stream;
        stream << "@group(" << nonEmptyGroupIndex << R"()
                  @binding(0) var<storage, read> inputData : u32;
        @group(2) @binding(0) var<storage, read_write> outputData : u32;
        @compute @workgroup_size(1, 1)
        fn main() {
            outputData = inputData;
        }
    )";

        wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, stream.str());

        // Create 3 bind group layouts with a null bind group layout.
        std::array<wgpu::BindGroupLayout, 3> bgls = {};
        bgls[nonEmptyGroupIndex] = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage}});
        bgls[2] = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

        // Create pipeline layout with the array of bind group layouts `bgls`.
        wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor = {};
        pipelineLayoutDescriptor.bindGroupLayoutCount = bgls.size();
        pipelineLayoutDescriptor.bindGroupLayouts = bgls.data();
        wgpu::PipelineLayout pipelineLayout =
            device.CreatePipelineLayout(&pipelineLayoutDescriptor);

        wgpu::ComputePipelineDescriptor computePipelineDescriptor = {};
        computePipelineDescriptor.compute.module = shaderModule;
        computePipelineDescriptor.layout = pipelineLayout;
        wgpu::ComputePipeline computePipeline =
            device.CreateComputePipeline(&computePipelineDescriptor);

        // Create and set 3 bind groups for the test. Only 2 of the 3 bind groups should be accessed
        // inside the compute pipeline.
        bgls[1 - nonEmptyGroupIndex] = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage}});
        wgpu::Buffer buffer0 =
            utils::CreateBufferFromData(device, wgpu::BufferUsage::Storage, {1u});
        wgpu::Buffer buffer1 =
            utils::CreateBufferFromData(device, wgpu::BufferUsage::Storage, {2u});
        wgpu::BufferDescriptor bufferDescriptor = {};
        bufferDescriptor.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
        bufferDescriptor.size = 4u;
        wgpu::Buffer buffer2 = device.CreateBuffer(&bufferDescriptor);
        wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgls[0], {{0, buffer0}});
        wgpu::BindGroup bg1 = utils::MakeBindGroup(device, bgls[1], {{0, buffer1}});
        wgpu::BindGroup bg2 = utils::MakeBindGroup(device, bgls[2], {{0, buffer2}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(computePipeline);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);
        pass.SetBindGroup(2, bg2);
        pass.DispatchWorkgroups(1);
        pass.End();
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        uint32_t expectedValue = nonEmptyGroupIndex + 1;
        EXPECT_BUFFER_U32_EQ(expectedValue, buffer2, 0);
    }
}

DAWN_INSTANTIATE_TEST(PipelineLayoutTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"d3d12_use_root_signature_version_1_1"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
