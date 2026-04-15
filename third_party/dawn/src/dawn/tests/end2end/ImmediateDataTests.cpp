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
    void SetUp() override {
        DawnTest::SetUp();

        mShaderModule = utils::CreateShaderModule(device, R"(
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
        pipelineLayoutDesc.immediateSize = 16;
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
    // TODO(crbug.com/479563279): diagnose failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::RenderPipeline pipeline = CreateRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // rgba8unorm: {0.1, 0.3, 0.5} + {0.1 diff} => {0.2, 0.4, 0.6} => {51, 102, 153, 255}
    std::array<float, 4> immediateData = {0.1, 0.3, 0.5, 0.1};
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetImmediates(0, immediateData.data(),
                                    immediateData.size() * sizeof(uint32_t));
    renderPassEncoder.SetPipeline(CreateRenderPipeline());
    renderPassEncoder.SetBindGroup(0, CreateBindGroup());
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(51, 102, 153, 255), renderPass.color, 0, 0);
}

// Test separately-declared immediate vars of different sizes in frag and vert shaders.
TEST_P(ImmediateDataTests, FragAndVertSeparateImmediateVars) {
    // TODO(crbug.com/479563279): diagnose failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    mShaderModule = utils::CreateShaderModule(device, R"(
        var<immediate> v_imm: vec2f;
        var<immediate> f_imm: mat4x4f;

        @vertex fn vsMain(@location(0) pos : vec2f) -> @builtin(position) vec4f {
          return vec4f(pos + v_imm, 0.0, 1.0);
        }

        @fragment fn fsMain() -> @location(0) vec4f {
          return f_imm[3];
        }
)");

    // Elements {0..1}  will offset the vertex positions in the vertex shader so that the resulting
    // triangle covers the framebuffer. Elements {12..15} will be output as the fragment color in
    // the fragment shader.
    std::array<float, 16> immediateData = {-2.0f, -3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                           0.0f,  0.0f,  0.0f, 0.0f, 0.2f, 0.4f, 0.6f, 1.0f};

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = mShaderModule;
    pipelineDescriptor.vertex.bufferCount = 1;
    pipelineDescriptor.cBuffers[0].attributeCount = 1;
    pipelineDescriptor.cBuffers[0].arrayStride = 2 * sizeof(float);
    pipelineDescriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x2;
    pipelineDescriptor.cFragment.module = mShaderModule;
    pipelineDescriptor.cFragment.targetCount = 1;

    auto pipeline = device.CreateRenderPipeline(&pipelineDescriptor);
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    float verts[3][2] = {
        {
            3.0f,
            2.0f,
        },
        {
            1.0f,
            2.0f,
        },
        {
            2.0f,
            4.0f,
        },
    };
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(verts);
    bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;

    auto vertexBuffer = device.CreateBuffer(&bufferDesc);
    queue.WriteBuffer(vertexBuffer, 0, verts, sizeof(verts));
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetVertexBuffer(0, vertexBuffer);
    renderPassEncoder.SetImmediates(0, immediateData.data(), immediateData.size() * sizeof(float));
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(51, 102, 153, 255), renderPass.color, 0, 0);
}

// Test that SetImmediates works in RenderBundle.
TEST_P(ImmediateDataTests, SetImmediatesInRenderBundle) {
    // TODO(crbug.com/479563279): diagnose failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::RenderPipeline pipeline = CreateRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // rgba8unorm: {0.1, 0.3, 0.5} + {0.1 diff} => {0.2, 0.4, 0.6} => {51, 102, 153, 255}
    std::array<float, 4> immediateData = {0.1, 0.3, 0.5, 0.1};

    wgpu::RenderBundleEncoderDescriptor bundleDesc;
    bundleDesc.colorFormatCount = 1;
    bundleDesc.colorFormats = &renderPass.colorFormat;
    wgpu::RenderBundleEncoder bundleEncoder = device.CreateRenderBundleEncoder(&bundleDesc);

    bundleEncoder.SetPipeline(pipeline);
    bundleEncoder.SetBindGroup(0, CreateBindGroup());
    bundleEncoder.SetImmediates(0, immediateData.data(), immediateData.size() * sizeof(float));
    bundleEncoder.Draw(3);
    wgpu::RenderBundle bundle = bundleEncoder.Finish();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.ExecuteBundles(1, &bundle);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(51, 102, 153, 255), renderPass.color, 0, 0);
}

// Test that SetImmediates works after ExecuteBundles.
TEST_P(ImmediateDataTests, SetImmediatesAfterExecuteBundles) {
    // TODO(crbug.com/479563279): diagnose failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::RenderPipeline pipeline = CreateRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Bundle sets immediate data to {0.6, 0.7, 0.8, 0.9}
    std::array<float, 4> bundleImmediateData = {0.6, 0.7, 0.8, 0.9};
    wgpu::RenderBundleEncoderDescriptor bundleDesc;
    bundleDesc.colorFormatCount = 1;
    bundleDesc.colorFormats = &renderPass.colorFormat;
    wgpu::RenderBundleEncoder bundleEncoder = device.CreateRenderBundleEncoder(&bundleDesc);
    bundleEncoder.SetPipeline(pipeline);
    bundleEncoder.SetBindGroup(0, CreateBindGroup());
    bundleEncoder.SetImmediates(0, bundleImmediateData.data(),
                                bundleImmediateData.size() * sizeof(float));
    bundleEncoder.Draw(3);
    wgpu::RenderBundle bundle = bundleEncoder.Finish();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);

    // Execute bundle. This should set the immediate data to {0.6, 0.7, 0.8, 0.9}.
    renderPassEncoder.ExecuteBundles(1, &bundle);

    // Set immediate data to {0.1, 0.3, 0.5, 0.1}.
    // rgba8unorm: {0.1, 0.3, 0.5} + {0.1 diff} => {0.2, 0.4, 0.6} => {51, 102, 153, 255}
    std::array<float, 4> immediateData = {0.1, 0.3, 0.5, 0.1};
    renderPassEncoder.SetImmediates(0, immediateData.data(), immediateData.size() * sizeof(float));
    // ExecuteBundles clears the current pipeline, so we need to set it again.
    renderPassEncoder.SetPipeline(pipeline);
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
    computePassEncoder.SetImmediates(0, immediateData.data(),
                                     immediateData.size() * sizeof(uint32_t));
    computePassEncoder.SetBindGroup(0, CreateBindGroup());
    computePassEncoder.DispatchWorkgroups(1);
    computePassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(immediateData.data(), mStorageBuffer, 0, immediateData.size());
}

// SetImmediates with offset on immediate data range.
TEST_P(ImmediateDataTests, SetImmediatesWithRangeOffset) {
    // TODO(crbug.com/479563279): diagnose failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

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
        renderPassEncoder.SetImmediates(0, immediateData.data(), 16);
        // Update {0.1, 0.3, 0.5} to {0.1，0.5，0.7} and + {0.1 diff} => {0.2, 0.6, 0.8} => {51,
        // 153, 204, 255}
        std::array<float, 2> immediateDataUpdated = {0.5, 0.7};
        renderPassEncoder.SetImmediates(4, immediateDataUpdated.data(), 8);
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
        // Using two SetImmediates + Offset to swap first half and second half value in immediate
        // data range.
        computePassEncoder.SetImmediates(kHalfImmediateDataSize, immediateData.data(),
                                         kHalfImmediateDataSize);
        computePassEncoder.SetImmediates(0, immediateData.data() + 2, kHalfImmediateDataSize);
        computePassEncoder.SetBindGroup(0, CreateBindGroup());
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        std::array<uint32_t, 4> expected = {240, 255, 25, 128};
        EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), mStorageBuffer, 0, expected.size());
    }
}

// SetImmediates should upload dirtied, latest contents between pipeline switches before draw or
// dispatch.
TEST_P(ImmediateDataTests, SetImmediatesMultipleTimes) {
    // TODO(crbug.com/479563279): diagnose failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    // Render Pipeline
    {
        wgpu::RenderPipeline pipeline = CreateRenderPipeline();
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        // rgba8unorm: {0.1, 0.3, 0.5} + {0.1 diff} => {0.2, 0.4, 0.6} => {51, 102, 153, 255}
        std::array<float, 4> immediateData = {0.1, 0.3, 0.5, 0.1};
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder =
            commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);

        // Using 4 SetImmediates to update all immediate data to 0.1.
        renderPassEncoder.SetImmediates(0, immediateData.data(), immediateData.size() * 4);
        renderPassEncoder.SetImmediates(4, immediateData.data(), (immediateData.size() - 1) * 4);
        renderPassEncoder.SetPipeline(CreateRenderPipeline());
        renderPassEncoder.SetImmediates(8, immediateData.data(), 8);
        renderPassEncoder.SetPipeline(CreateRenderPipeline());
        renderPassEncoder.SetImmediates(12, immediateData.data(), 4);
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

        // Using 4 SetImmediates to update all immediate data to 25.
        computePassEncoder.SetImmediates(0, immediateData.data(), immediateData.size() * 4);
        computePassEncoder.SetImmediates(4, immediateData.data(), (immediateData.size() - 1) * 4);
        computePassEncoder.SetPipeline(CreateComputePipeline());
        computePassEncoder.SetImmediates(8, immediateData.data(), 8);
        computePassEncoder.SetPipeline(CreateComputePipeline());
        computePassEncoder.SetImmediates(12, immediateData.data(), 4);

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
    // TODO(crbug.com/473870505): [Capture] support depth/stencil and multi-planar textures.
    DAWN_SUPPRESS_TEST_IF(IsCaptureReplayCheckingEnabled());

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
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
    pass.SetImmediates(0, immediateData.data(), immediateData.size() * 4);
    pass.SetPipeline(pipeline);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_FLOAT_EQ(0.5f, depthTexture, 0, 0);
}

// Test that vertex_index (supported by internal immediate constants)
// works fine when the immediates are unused and optimized out by the driver.
TEST_P(ImmediateDataTests, VertexIndexOptimizedOut) {
    DAWN_SUPPRESS_TEST_IF(IsCaptureReplayCheckingEnabled());

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        fn v(index : u32) -> vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
        @vertex fn vs(@builtin(vertex_index) index : u32) -> @builtin(position) vec4f {
            return v(index);
        }

        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(1.0, 1.0, 1.0, 1.0);
        }
    )");

    // Create the pipeline that uses frag_depth to output the depth.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pDesc.cFragment.module = module;
    pDesc.cFragment.targetCount = 1;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, CreateBindGroup());
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(255, 255, 255, 255), renderPass.color, 0, 0);
}

// SetImmediates Multiple times should upload dirtied, latest contents.
TEST_P(ImmediateDataTests, SetImmediatesWithPipelineSwitch) {
    wgpu::ShaderModule shaderModuleWithLessImmediateData = utils::CreateShaderModule(device, R"(
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
        renderPassEncoder.SetImmediates(0, immediateData.data(), immediateData.size() * 4);
        renderPassEncoder.SetPipeline(CreateRenderPipeline());

        // replace the pipeline and rgba8unorm: {0.4, 0.4, 0.6} => {102, 102, 153}
        float data = 0.4;
        renderPassEncoder.SetImmediates(0, &data, 4);
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

        computePassEncoder.SetImmediates(0, immediateData.data(), immediateData.size() * 4);
        computePassEncoder.SetPipeline(CreateComputePipeline());

        uint32_t data = 128;
        computePassEncoder.SetImmediates(0, &data, 4);
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

// Test that SetImmediates works with multiple ExecuteBundles calls.
TEST_P(ImmediateDataTests, SetImmediatesInMultipleExecuteBundles) {
    // TODO(crbug.com/479563279): diagnose failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::RenderPipeline pipeline = CreateRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    wgpu::RenderBundleEncoderDescriptor bundleDesc;
    bundleDesc.colorFormatCount = 1;
    bundleDesc.colorFormats = &renderPass.colorFormat;

    // Bundle 1
    std::array<float, 4> data1 = {0.6, 0.7, 0.8, 0.1};
    wgpu::RenderBundleEncoder bundleEncoder1 = device.CreateRenderBundleEncoder(&bundleDesc);
    bundleEncoder1.SetPipeline(pipeline);
    bundleEncoder1.SetBindGroup(0, CreateBindGroup());
    bundleEncoder1.SetImmediates(0, data1.data(), data1.size() * sizeof(float));
    bundleEncoder1.Draw(3);
    wgpu::RenderBundle bundle1 = bundleEncoder1.Finish();

    // Bundle 2
    // Use values that avoid rounding ambiguity.
    // rgba8unorm: {0.1, 0.3, 0.5} + {0.1 diff} => {0.2, 0.4, 0.6} => {51, 102, 153, 255}
    std::array<float, 4> data2 = {0.1, 0.3, 0.5, 0.1};
    wgpu::RenderBundleEncoder bundleEncoder2 = device.CreateRenderBundleEncoder(&bundleDesc);
    bundleEncoder2.SetPipeline(pipeline);
    bundleEncoder2.SetBindGroup(0, CreateBindGroup());
    bundleEncoder2.SetImmediates(0, data2.data(), data2.size() * sizeof(float));
    bundleEncoder2.Draw(3);
    wgpu::RenderBundle bundle2 = bundleEncoder2.Finish();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);

    renderPassEncoder.ExecuteBundles(1, &bundle1);
    renderPassEncoder.ExecuteBundles(1, &bundle2);

    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(51, 102, 153, 255), renderPass.color, 0, 0);
}

// Test that RenderBundle immediate state is not affected by previous SetImmediates state in
// RenderPass.
TEST_P(ImmediateDataTests, BundlesDontCarePreviousImmediatesState) {
    // TODO(crbug.com/479563279): diagnose failures on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::RenderPipeline pipeline = CreateRenderPipeline();
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Bundle
    std::array<float, 4> bundleData = {0.1, 0.3, 0.5, 0.1};
    wgpu::RenderBundleEncoderDescriptor bundleDesc;
    bundleDesc.colorFormatCount = 1;
    bundleDesc.colorFormats = &renderPass.colorFormat;
    wgpu::RenderBundleEncoder bundleEncoder = device.CreateRenderBundleEncoder(&bundleDesc);
    bundleEncoder.SetPipeline(pipeline);
    bundleEncoder.SetBindGroup(0, CreateBindGroup());
    bundleEncoder.SetImmediates(0, bundleData.data(), bundleData.size() * sizeof(float));
    bundleEncoder.Draw(3);
    wgpu::RenderBundle bundle = bundleEncoder.Finish();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPass.renderPassInfo);

    std::array<float, 4> dataA = {0.6, 0.7, 0.8, 0.1};
    renderPassEncoder.SetImmediates(0, dataA.data(), dataA.size() * sizeof(float));
    renderPassEncoder.SetPipeline(pipeline);
    renderPassEncoder.SetBindGroup(0, CreateBindGroup());

    // Execute Bundle (should draw with 0.{0.1, 0.3, 0.5, 0.1}})
    renderPassEncoder.ExecuteBundles(1, &bundle);
    renderPassEncoder.End();
    wgpu::CommandBuffer commands = commandEncoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(51, 102, 153, 255), renderPass.color, 0, 0);
}

// Switching pipelines with different internal push constant sizes (e.g. frag_depth adds
// clampFragDepth) must rebind all bind groups due to VkPipelineLayout change.
TEST_P(ImmediateDataTests, BindGroupsReboundOnDifferentInternalImmediateSize) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform}});

    wgpu::PipelineLayout layout = utils::MakePipelineLayout(device, {bgl}, 4);

    // Single shader module with shared VS and two FS entry points:
    //  - fsRed: outputs solid red (no frag_depth, smaller internal push constant range).
    //  - fsFromBindGroup: reads the bind group and writes frag_depth (larger range).
    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var<uniform> color: vec4f;
        var<immediate> imm: f32;
        @vertex fn vs(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {
            let p = array(vec2f(1,-1), vec2f(-1,-1), vec2f(0,1));
            return vec4f(p[i], 0, 1);
        }
        @fragment fn fsRed() -> @location(0) vec4f { return vec4f(1,0,0,1); }
        struct Out { @location(0) c: vec4f, @builtin(frag_depth) d: f32 }
        @fragment fn fsFromBindGroup() -> Out { return Out(color, 0.5); }
    )");

    auto MakePipeline = [&](wgpu::ShaderModule mod, bool useDepthWrite) {
        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = mod;
        desc.cFragment.module = mod;
        desc.cFragment.entryPoint = useDepthWrite ? "fsFromBindGroup" : "fsRed";
        desc.cFragment.targetCount = 1;
        desc.layout = layout;
        auto* ds = desc.EnableDepthStencil(wgpu::TextureFormat::Depth32Float);
        ds->depthWriteEnabled =
            useDepthWrite ? wgpu::OptionalBool::True : wgpu::OptionalBool::False;
        ds->depthCompare = wgpu::CompareFunction::Always;
        return device.CreateRenderPipeline(&desc);
    };

    // Pipeline A: fsRed (no frag_depth).
    wgpu::RenderPipeline pipelineWithoutFragDepth = MakePipeline(shaderModule, false);
    // Pipeline B: fsFromBindGroup (with frag_depth, different push constant range).
    wgpu::RenderPipeline pipelineWithFragDepth = MakePipeline(shaderModule, true);

    // The color uniform provides the expected output: {0.2, 0.4, 0.6, 1.0} -> rgba8
    // {51,102,153,255}.
    wgpu::Buffer colorBuf =
        utils::CreateBufferFromData(device, wgpu::BufferUsage::Uniform, {0.2f, 0.4f, 0.6f, 1.0f});
    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, colorBuf}});

    utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);
    wgpu::TextureDescriptor depthDesc;
    depthDesc.size = {kRTSize, kRTSize};
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment;
    depthDesc.format = wgpu::TextureFormat::Depth32Float;
    wgpu::Texture depthTex = device.CreateTexture(&depthDesc);
    utils::ComboRenderPassDescriptor rpDesc({rp.color.CreateView()}, depthTex.CreateView());
    rpDesc.UnsetDepthStencilLoadStoreOpsForFormat(wgpu::TextureFormat::Depth32Float);

    float immData = 0.0f;
    wgpu::CommandEncoder enc = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = enc.BeginRenderPass(&rpDesc);
    pass.SetBindGroup(0, bg);
    pass.SetImmediates(0, &immData, sizeof(float));
    // Draw with Pipeline A (red), then switch to Pipeline B without re-setting bind groups.
    // The Vulkan backend needs to reapply VkDescriptorSets after setting B because the change
    // in push constant size will invalidate the currently bound VkDescriptorSets.
    pass.SetPipeline(pipelineWithoutFragDepth);
    pass.Draw(3);
    pass.SetPipeline(pipelineWithFragDepth);
    pass.Draw(3);
    pass.End();
    wgpu::CommandBuffer commands = enc.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(51, 102, 153, 255), rp.color, 0, 0);
}

// Switching pipelines with different calculated PipelineLayout immediateSize must rebind all
// bind groups.
TEST_P(ImmediateDataTests, BindGroupsReboundOnDifferentPipelineLayoutImmediateSize) {
    wgpu::BindGroupLayout bgl0 = CreateBindGroupLayout();

    wgpu::PipelineLayout layout4Bytes = utils::MakePipelineLayout(device, {bgl0}, 4);
    wgpu::PipelineLayout layout16Bytes = utils::MakePipelineLayout(device, {bgl0}, 16);

    wgpu::ShaderModule module4Bytes = utils::CreateShaderModule(device, R"(
        var<immediate> imm: u32;
        @group(0) @binding(0) var<storage, read_write> output: vec4u;
        @compute @workgroup_size(1) fn csMain() { output = vec4u(imm, 0, 0, 0); }
    )");
    wgpu::ShaderModule module16Bytes = utils::CreateShaderModule(device, R"(
        var<immediate> imm: vec4u;
        @group(0) @binding(0) var<storage, read_write> output: vec4u;
        @compute @workgroup_size(1) fn csMain() { output = imm; }
    )");

    wgpu::ComputePipelineDescriptor csDesc4Bytes;
    csDesc4Bytes.compute.module = module4Bytes;
    csDesc4Bytes.layout = layout4Bytes;
    wgpu::ComputePipeline pipeline4Bytes = device.CreateComputePipeline(&csDesc4Bytes);

    wgpu::ComputePipelineDescriptor csDesc16Bytes;
    csDesc16Bytes.compute.module = module16Bytes;
    csDesc16Bytes.layout = layout16Bytes;
    wgpu::ComputePipeline pipeline16Bytes = device.CreateComputePipeline(&csDesc16Bytes);

    wgpu::BindGroup bg0 = utils::MakeBindGroup(device, bgl0, {{0, mStorageBuffer}});

    uint32_t imm4Bytes = 42;
    std::array<uint32_t, 4> imm16Bytes = {10, 20, 30, 40};

    wgpu::CommandEncoder enc = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = enc.BeginComputePass();
    // Dispatch with Pipeline A, then switch to Pipeline B without re-setting
    // bind groups.
    pass.SetBindGroup(0, bg0);
    pass.SetImmediates(0, &imm4Bytes, sizeof(uint32_t));
    pass.SetPipeline(pipeline4Bytes);
    pass.DispatchWorkgroups(1);
    pass.SetImmediates(0, imm16Bytes.data(), sizeof(imm16Bytes));
    pass.SetPipeline(pipeline16Bytes);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = enc.Finish();
    queue.Submit(1, &commands);

    std::array<uint32_t, 4> expected = {10, 20, 30, 40};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), mStorageBuffer, 0, expected.size());
}

// Switching pipelines with different calculated PipelineLayout immediateSize must rebind all
// bind groups. Also tests that defaulted pipeline layouts work correctly.
TEST_P(ImmediateDataTests, AutoCalculatedPipelineLayoutImmediateSize) {
    wgpu::BindGroupLayout bgl0 = CreateBindGroupLayout();

    wgpu::ShaderModule module4Bytes = utils::CreateShaderModule(device, R"(
        var<immediate> imm: u32;
        @group(0) @binding(0) var<storage, read_write> output: vec4u;
        @compute @workgroup_size(1) fn csMain() { output = vec4u(imm, 0, 0, 0); }
    )");
    wgpu::ShaderModule module16Bytes = utils::CreateShaderModule(device, R"(
        var<immediate> imm: vec4u;
        @group(0) @binding(0) var<storage, read_write> output: vec4u;
        @compute @workgroup_size(1) fn csMain() { output = imm; }
    )");

    wgpu::ComputePipelineDescriptor csDesc4Bytes;
    csDesc4Bytes.compute.module = module4Bytes;

    wgpu::ComputePipelineDescriptor csDesc16Bytes;
    csDesc16Bytes.compute.module = module16Bytes;

    wgpu::ComputePipeline pipeline4Bytes = device.CreateComputePipeline(&csDesc4Bytes);
    wgpu::ComputePipeline pipeline16Bytes = device.CreateComputePipeline(&csDesc16Bytes);

    wgpu::BindGroup bg4Bytes =
        utils::MakeBindGroup(device, pipeline4Bytes.GetBindGroupLayout(0), {{0, mStorageBuffer}});
    wgpu::BindGroup bg16Bytes =
        utils::MakeBindGroup(device, pipeline16Bytes.GetBindGroupLayout(0), {{0, mStorageBuffer}});

    uint32_t imm4Bytes = 42;
    std::array<uint32_t, 4> imm16Bytes = {10, 20, 30, 40};

    wgpu::CommandEncoder enc = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = enc.BeginComputePass();
    // Dispatch with Pipeline A, then switch to Pipeline B using auto-calculated layout
    pass.SetBindGroup(0, bg4Bytes);
    pass.SetImmediates(0, &imm4Bytes, sizeof(uint32_t));
    pass.SetPipeline(pipeline4Bytes);
    pass.DispatchWorkgroups(1);

    // Defaulted layouts are not compatible between pipelines at all.
    // so we re-bind the properly derived bind group for pipeline16Bytes.
    pass.SetBindGroup(0, bg16Bytes);
    pass.SetImmediates(0, imm16Bytes.data(), sizeof(imm16Bytes));
    pass.SetPipeline(pipeline16Bytes);
    pass.DispatchWorkgroups(1);
    pass.End();
    wgpu::CommandBuffer commands = enc.Finish();
    queue.Submit(1, &commands);

    std::array<uint32_t, 4> expected = {10, 20, 30, 40};
    EXPECT_BUFFER_U32_RANGE_EQ(expected.data(), mStorageBuffer, 0, expected.size());
}

DAWN_INSTANTIATE_TEST(ImmediateDataTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

class ImmediateDataWithResourceTableTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(
            !SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable}));

        // TODO(https://issues.chromium.org/435317394): The Subzero compiler used by Swiftshader
        // produces bad code and crashes on some VK_EXT_descriptor_indexing workloads. Skip tests on
        // it, but still run them with Swiftshader LLVM 10.0. On ARM64 the only supported compiler
        // is LLVM10.0 so use that signal to choose when Swiftshader can be tested.
        DAWN_SUPPRESS_TEST_IF(IsSwiftshader() && !DAWN_PLATFORM_IS(ARM64));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (SupportsFeatures({wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable})) {
            return {wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable};
        }
        return {};
    }

    wgpu::PipelineLayout MakePipelineLayoutWithTable(std::vector<wgpu::BindGroupLayout> bgls,
                                                     uint32_t immediateSize) {
        wgpu::PipelineLayoutResourceTable plTable;
        plTable.usesResourceTable = true;

        wgpu::PipelineLayoutDescriptor desc{
            .nextInChain = &plTable,
            .bindGroupLayoutCount = bgls.size(),
            .bindGroupLayouts = bgls.data(),
            .immediateSize = immediateSize,
        };
        return device.CreatePipelineLayout(&desc);
    }
};

// Resource table (VkDescriptorSet 0) must be rebound when push constant range changes
// between pipelines, even if the resource table itself doesn't change.
// The VVL will complain if the resource table is not correctly rebound (with
// --enable-backend-validation).
TEST_P(ImmediateDataWithResourceTableTests, ResourceTableReboundOnDifferentImmediateSize) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});

    wgpu::PipelineLayout layout4Bytes = MakePipelineLayoutWithTable({bgl}, 4);
    wgpu::PipelineLayout layout16Bytes = MakePipelineLayoutWithTable({bgl}, 16);

    wgpu::ShaderModule module4Bytes = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        var<immediate> imm: u32;
        @group(0) @binding(0) var<storage, read_write> output: u32;
        @compute @workgroup_size(1) fn main() {
            output = imm;
        }
    )");
    wgpu::ShaderModule module16Bytes = utils::CreateShaderModule(device, R"(
        enable chromium_experimental_resource_table;
        var<immediate> imm: vec4u;
        @group(0) @binding(0) var<storage, read_write> output: u32;
        @compute @workgroup_size(1) fn main() {
            output = imm.x;
        }
    )");

    wgpu::ComputePipelineDescriptor csDesc4Bytes;
    csDesc4Bytes.compute.module = module4Bytes;
    csDesc4Bytes.layout = layout4Bytes;
    wgpu::ComputePipeline pipeline4Bytes = device.CreateComputePipeline(&csDesc4Bytes);

    wgpu::ComputePipelineDescriptor csDesc16Bytes;
    csDesc16Bytes.compute.module = module16Bytes;
    csDesc16Bytes.layout = layout16Bytes;
    wgpu::ComputePipeline pipeline16Bytes = device.CreateComputePipeline(&csDesc16Bytes);

    wgpu::BufferDescriptor bufDesc = {
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(uint32_t),
    };
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bufDesc);
    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, resultBuffer}});

    // The resource table just needs to exist, and be present in the pipeline layout, to activate
    // the resource table code path in the backend. Its contents don't matter — we're testing that
    // the resource table descriptor set gets rebound when push constant ranges change between
    // pipelines.
    wgpu::ResourceTableDescriptor rtDesc;
    rtDesc.size = 1;
    wgpu::ResourceTable table = device.CreateResourceTable(&rtDesc);

    uint32_t imm4Bytes = 100;
    std::array<uint32_t, 4> imm16Bytes = {200, 0, 0, 0};

    wgpu::CommandEncoder enc = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = enc.BeginComputePass();
    pass.SetResourceTable(table);
    pass.SetBindGroup(0, bg);

    // Dispatch with pipeline4Bytes, then switch to pipeline16Bytes (different immediate size).
    // If the resource table is not rebound, the VVL should report an error.
    pass.SetImmediates(0, &imm4Bytes, sizeof(imm4Bytes));
    pass.SetPipeline(pipeline4Bytes);
    pass.DispatchWorkgroups(1);

    pass.SetImmediates(0, imm16Bytes.data(), sizeof(imm16Bytes));
    pass.SetPipeline(pipeline16Bytes);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = enc.Finish();
    queue.Submit(1, &commands);

    // Pipeline B writes imm.x = 200 to output.
    uint32_t expected = 200;
    EXPECT_BUFFER_U32_EQ(expected, resultBuffer, 0);
}

DAWN_INSTANTIATE_TEST(ImmediateDataWithResourceTableTests,
                      D3D12Backend(),
                      MetalBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
