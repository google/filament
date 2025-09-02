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

#include <array>
#include <limits>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 1;

class ImmediateDataTests : public DawnTest {
  protected:
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        required.maxImmediateSize = kDefaultMaxImmediateDataBytes;
    }

    void SetUp() override {
        DawnTest::SetUp();

        mShaderModule = utils::CreateShaderModule(device, R"(
            enable chromium_experimental_immediate;
            struct Immediate {
                color: vec3<f32>,
                colorDiff: f32,
            };
            var<immediate> constants: Immediate;
            struct VertexOut {
                @location(0) color : vec3f,
                @builtin(position) position : vec4f,
            }

            @vertex fn vsMain(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
                const pos = array(
                    vec2( 1.0, -1.0),
                    vec2(-1.0, -1.0),
                    vec2( 0.0,  1.0),
                );
                var output: VertexOut;
                output.position = vec4f(pos[VertexIndex], 0.0, 1.0);
                output.color = constants.color;
                return output;
            }

            // to reuse the same pipeline layout
            @fragment fn fsMain(@location(0) color:vec3f) -> @location(0) vec4f {
                return vec4f(color + vec3f(constants.colorDiff), 1.0);
            }

            var<immediate> computeConstants: vec4u;
            @group(0) @binding(0) var<storage, read_write> output : vec4u;

            @compute @workgroup_size(1, 1, 1)
            fn csMain() {
                output = computeConstants;
            })");

        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.size = sizeof(uint32_t) * 4;
        bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage;
        mStorageBuffer = device.CreateBuffer(&bufferDesc);
    }

    wgpu::BindGroupLayout CreateBindGroupLayout() {
        wgpu::BindGroupLayoutEntry entries[1];
        entries[0].binding = 0;
        entries[0].visibility = wgpu::ShaderStage::Compute;
        entries[0].buffer.type = wgpu::BufferBindingType::Storage;

        wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
        bindGroupLayoutDesc.entryCount = 1;
        bindGroupLayoutDesc.entries = entries;

        return device.CreateBindGroupLayout(&bindGroupLayoutDesc);
    }

    wgpu::PipelineLayout CreatePipelineLayout() {
        wgpu::BindGroupLayout bindGroupLayout = CreateBindGroupLayout();

        wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
        pipelineLayoutDesc.bindGroupLayoutCount = 1;
        pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
        pipelineLayoutDesc.immediateSize = kDefaultMaxImmediateDataBytes;
        return device.CreatePipelineLayout(&pipelineLayoutDesc);
    }

    wgpu::RenderPipeline CreateRenderPipeline() {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = mShaderModule;
        pipelineDescriptor.cFragment.module = mShaderModule;
        pipelineDescriptor.cFragment.targetCount = 1;
        pipelineDescriptor.layout = CreatePipelineLayout();

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::ComputePipeline CreateComputePipeline() {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = mShaderModule;
        csDesc.layout = CreatePipelineLayout();

        return device.CreateComputePipeline(&csDesc);
    }

    wgpu::BindGroup CreateBindGroup() {
        return utils::MakeBindGroup(device, CreateBindGroupLayout(), {{0, mStorageBuffer}});
    }

    wgpu::ShaderModule mShaderModule;
    wgpu::Buffer mStorageBuffer;
};

// ImmediateData has been uploaded successfully.
TEST_P(ImmediateDataTests, BasicRenderPipeline) {
    wgpu::RenderPipeline pipeline = CreateRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // rgba8unorm: {0.1, 0.3, 0.5} + {0.1 diff} => {0.2, 0.4, 0.6} => {51, 102, 153, 255}
    std::array<float, 4> immediateData = {0.1, 0.3, 0.5, 0.1};
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetImmediateData(0, immediateData.data(),
                                       immediateData.size() * sizeof(uint32_t));
    renderPassEncoder.SetPipeline(CreateRenderPipeline());
    renderPassEncoder.SetBindGroup(0, CreateBindGroup());
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(51, 102, 153, 255), renderPass.color, 0, 0);
}

// ImmediateData has been uploaded successfully.
TEST_P(ImmediateDataTests, BasicComputePipeline) {
    std::array<uint32_t, 4> immediateData = {25, 128, 240, 255};
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(CreateComputePipeline());
    computePassEncoder.SetImmediateData(0, immediateData.data(),
                                        immediateData.size() * sizeof(uint32_t));
    computePassEncoder.SetBindGroup(0, CreateBindGroup());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(immediateData.data(), mStorageBuffer, 0, immediateData.size());
}

// ImmediateData range should be initialized to 0.
TEST_P(ImmediateDataTests, ImmediateDataInitialization) {
    // Render pipeline
    {
        wgpu::RenderPipeline pipeline = CreateRenderPipeline();
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        // rgba8unorm: {0.0, 0.4, 0.6} + {0.0 diff} => {0.0, 0.4, 0.6} => {0, 102, 153, 255}
        std::array<float, 2> immediateData = {0.4, 0.6};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder =
            commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
        renderPassEncoder.SetImmediateData(4, immediateData.data(), 8);
        renderPassEncoder.SetPipeline(CreateRenderPipeline());
        renderPassEncoder.SetBindGroup(0, CreateBindGroup());
        renderPassEncoder.Draw(3);
        renderPassEncoder.End();
        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 102, 153, 255), renderPass.color, 0, 0);
    }

    // Compute Pipeline
    {
        std::array<uint32_t, 2> immediateData = {128, 240};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(CreateComputePipeline());
        computePassEncoder.SetImmediateData(4, immediateData.data(), 8);
        computePassEncoder.SetBindGroup(0, CreateBindGroup());
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        std::array<uint32_t, 4> expected = {0, 128, 240, 0};
        EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), mStorageBuffer, 0, immediateData.size());
    }
}

// SetImmediateData with offset on immediate data range.
TEST_P(ImmediateDataTests, SetImmediateDataWithRangeOffset) {
    constexpr uint32_t kHalfImmediateDataSize = 8;
    // Render Pipeline
    {
        wgpu::RenderPipeline pipeline = CreateRenderPipeline();
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        // rgba8unorm: {0.1, 0.3, 0.5} + {0.1 diff} => {0.2, 0.4, 0.6} => {51, 102, 153, 255}
        std::array<float, 4> immediateData = {0.1, 0.3, 0.5, 0.1};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder =
            commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
        renderPassEncoder.SetImmediateData(0, immediateData.data(), 16);
        // Update {0.1, 0.3, 0.5} to {0.1，0.5，0.7} and + {0.1 diff} => {0.2, 0.6, 0.8} => {51,
        // 153, 204, 255}
        std::array<float, 2> immediateDataUpdated = {0.5, 0.7};
        renderPassEncoder.SetImmediateData(4, immediateDataUpdated.data(), 8);
        renderPassEncoder.SetPipeline(CreateRenderPipeline());
        renderPassEncoder.SetBindGroup(0, CreateBindGroup());
        renderPassEncoder.Draw(3);
        renderPassEncoder.End();
        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(51, 153, 204, 255), renderPass.color, 0, 0);
    }

    // Compute Pipeline
    {
        std::array<uint32_t, 4> immediateData = {25, 128, 240, 255};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
        computePassEncoder.SetPipeline(CreateComputePipeline());
        // Using two SetImmediateData + Offset to swap first half and second half value in immediate
        // data range.
        computePassEncoder.SetImmediateData(kHalfImmediateDataSize, immediateData.data(),
                                            kHalfImmediateDataSize);
        computePassEncoder.SetImmediateData(0, immediateData.data() + 2, kHalfImmediateDataSize);
        computePassEncoder.SetBindGroup(0, CreateBindGroup());
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        std::array<uint32_t, 4> expected = {240, 255, 25, 128};
        EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), mStorageBuffer, 0, expected.size());
    }
}

// SetImmediateData should upload dirtied, latest contents between pipeline switches before draw or
// dispatch.
TEST_P(ImmediateDataTests, SetImmediateDataMultipleTimes) {
    // Render Pipeline
    {
        wgpu::RenderPipeline pipeline = CreateRenderPipeline();
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        // rgba8unorm: {0.1, 0.3, 0.5} + {0.1 diff} => {0.2, 0.4, 0.6} => {51, 102, 153, 255}
        std::array<float, 4> immediateData = {0.1, 0.3, 0.5, 0.1};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder =
            commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);

        // Using 4 SetImmediateData to update all immediate data to 0.1.
        renderPassEncoder.SetImmediateData(0, immediateData.data(), immediateData.size() * 4);
        renderPassEncoder.SetImmediateData(4, immediateData.data(), (immediateData.size() - 1) * 4);
        renderPassEncoder.SetPipeline(CreateRenderPipeline());
        renderPassEncoder.SetImmediateData(8, immediateData.data(), 8);
        renderPassEncoder.SetPipeline(CreateRenderPipeline());
        renderPassEncoder.SetImmediateData(12, immediateData.data(), 4);
        renderPassEncoder.SetBindGroup(0, CreateBindGroup());
        renderPassEncoder.Draw(3);
        renderPassEncoder.End();
        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(51, 51, 51, 255), renderPass.color, 0, 0);
    }

    // Compute Pipeline
    {
        std::array<uint32_t, 4> immediateData = {25, 128, 240, 255};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();

        // Using 4 SetImmediateData to update all immediate data to 25.
        computePassEncoder.SetImmediateData(0, immediateData.data(), immediateData.size() * 4);
        computePassEncoder.SetImmediateData(4, immediateData.data(),
                                            (immediateData.size() - 1) * 4);
        computePassEncoder.SetPipeline(CreateComputePipeline());
        computePassEncoder.SetImmediateData(8, immediateData.data(), 8);
        computePassEncoder.SetPipeline(CreateComputePipeline());
        computePassEncoder.SetImmediateData(12, immediateData.data(), 4);

        computePassEncoder.SetBindGroup(0, CreateBindGroup());
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        std::array<uint32_t, 4> expected = {25, 25, 25, 25};
        EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), mStorageBuffer, 0, expected.size());
    }
}

// Test that clamp frag depth(supported by internal immediate constants)
// works fine when shaders have user immediate data
TEST_P(ImmediateDataTests, UsingImmediateDataDontAffectClampFragDepth) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_immediate;
        var<immediate> constants: vec4f;
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.5, 1.0);
        }

        @fragment fn fs() -> @builtin(frag_depth) f32 {
            return constants.r;
        }
    )");

    // Create the pipeline that uses frag_depth to output the depth.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pDesc.cFragment.module = module;
    pDesc.cFragment.targetCount = 0;

    wgpu::DepthStencilState* pDescDS = pDesc.EnableDepthStencil(wgpu::TextureFormat::Depth32Float);
    pDescDS->depthWriteEnabled = wgpu::OptionalBool::True;
    pDescDS->depthCompare = wgpu::CompareFunction::Always;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Create a depth-only render pass.
    wgpu::TextureDescriptor depthDesc;
    depthDesc.size = {1, 1};
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    depthDesc.format = wgpu::TextureFormat::Depth32Float;
    wgpu::Texture depthTexture = device.CreateTexture(&depthDesc);

    std::array<float, 4> immediateData = {1.0, 1.0, 1.0, 1.0};

    utils::ComboRenderPassDescriptor renderPassDesc({}, depthTexture.CreateView());
    renderPassDesc.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
    renderPassDesc.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

    // Draw a point with a skewed viewport, so 1.0 depth gets clamped to 0.5.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
    pass.SetViewport(0, 0, 1, 1, 0.0, 0.5);
    pass.SetImmediateData(0, immediateData.data(), immediateData.size() * 4);
    pass.SetPipeline(pipeline);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_FLOAT_EQ(0.5f, depthTexture, 0, 0);
}

// SetImmediateData Multiple times should upload dirtied, latest contents.
TEST_P(ImmediateDataTests, SetImmediateDataWithPipelineSwitch) {
    wgpu::ShaderModule shaderModuleWithLessImmediateData = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_immediate;
        struct Immediate {
            color: vec3<f32>,
        };
        var<immediate> constants: Immediate;
        struct VertexOut {
            @location(0) color : vec3f,
            @builtin(position) position : vec4f,
        }

        @vertex fn vsMain(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
            const pos = array(
                vec2( 1.0, -1.0),
                vec2(-1.0, -1.0),
                vec2( 0.0,  1.0),
            );
            var output: VertexOut;
            output.position = vec4f(pos[VertexIndex], 0.0, 1.0);
            output.color = constants.color;
            return output;
        }

        // to reuse the same pipeline layout
        @fragment fn fsMain(@location(0) color:vec3f) -> @location(0) vec4f {
            return vec4f(color, 1.0);
        }

        var<immediate> computeConstants: vec3u;
        @group(0) @binding(0) var<storage, read_write> output : vec3u;

        @compute @workgroup_size(1, 1, 1)
        fn csMain() {
            output = computeConstants;
        })");

    // Render Pipeline
    {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = shaderModuleWithLessImmediateData;
        pipelineDescriptor.cFragment.module = shaderModuleWithLessImmediateData;
        pipelineDescriptor.cFragment.targetCount = 1;

        wgpu::RenderPipeline pipelineWithLessImmediateData =
            device.CreateRenderPipeline(&pipelineDescriptor);

        wgpu::RenderPipeline pipeline = CreateRenderPipeline();
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder =
            commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);

        // rgba8unorm: {0.2, 0.4, 0.6} + {0.1 diff} => {0.3, 0.5, 0.7}
        std::array<float, 4> immediateData = {0.2, 0.4, 0.6, 0.1};
        renderPassEncoder.SetImmediateData(0, immediateData.data(), immediateData.size() * 4);
        renderPassEncoder.SetPipeline(CreateRenderPipeline());

        // replace the pipeline and rgba8unorm: {0.4, 0.4, 0.6} => {102, 102, 153}
        float data = 0.4;
        renderPassEncoder.SetImmediateData(0, &data, 4);
        renderPassEncoder.SetPipeline(pipelineWithLessImmediateData);
        renderPassEncoder.Draw(3);
        renderPassEncoder.End();
        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(102, 102, 153, 255), renderPass.color, 0, 0);
    }

    // Compute Pipeline
    {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = shaderModuleWithLessImmediateData;

        wgpu::ComputePipeline pipelineWithLessImmediateData = device.CreateComputePipeline(&csDesc);

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipelineWithLessImmediateData.GetBindGroupLayout(0), {{0, mStorageBuffer}});

        std::array<uint32_t, 4> immediateData = {25, 128, 240, 255};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();

        computePassEncoder.SetImmediateData(0, immediateData.data(), immediateData.size() * 4);
        computePassEncoder.SetPipeline(CreateComputePipeline());

        uint32_t data = 128;
        computePassEncoder.SetImmediateData(0, &data, 4);
        computePassEncoder.SetPipeline(pipelineWithLessImmediateData);

        computePassEncoder.SetBindGroup(0, bindGroup);
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        std::array<uint32_t, 3> expected = {128, 128, 240};
        EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), mStorageBuffer, 0, expected.size());
    }
}

DAWN_INSTANTIATE_TEST(ImmediateDataTests, D3D11Backend(), D3D12Backend(), VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
