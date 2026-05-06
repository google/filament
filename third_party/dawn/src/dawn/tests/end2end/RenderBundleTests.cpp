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

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 4;
const utils::RGBA8 kColors[2] = {utils::RGBA8::kGreen, utils::RGBA8::kBlue};

// RenderBundleTest tests simple usage of RenderBundles to draw. The implementation
// of RenderBundle is shared significantly with render pass execution which is
// tested in all other rendering tests.
class RenderBundleTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@location(0) pos : vec4f) -> @builtin(position) vec4f {
                return pos;
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            struct Ubo {
                color : vec4f
            }
            @group(0) @binding(0) var<uniform> fragmentUniformBuffer : Ubo;

            @fragment fn main() -> @location(0) vec4f {
                return fragmentUniformBuffer.color;
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.vertex.bufferCount = 1;
        descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
        descriptor.cBuffers[0].attributeCount = 1;
        descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
        descriptor.cTargets[0].format = renderPass.colorFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);

        float colors0[] = {kColors[0].r / 255.f, kColors[0].g / 255.f, kColors[0].b / 255.f,
                           kColors[0].a / 255.f};
        float colors1[] = {kColors[1].r / 255.f, kColors[1].g / 255.f, kColors[1].b / 255.f,
                           kColors[1].a / 255.f};

        wgpu::Buffer buffer0 = utils::CreateBufferFromData(device, colors0, 4 * sizeof(float),
                                                           wgpu::BufferUsage::Uniform);
        wgpu::Buffer buffer1 = utils::CreateBufferFromData(device, colors1, 4 * sizeof(float),
                                                           wgpu::BufferUsage::Uniform);

        bindGroups[0] = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                             {{0, buffer0, 0, 4 * sizeof(float)}});
        bindGroups[1] = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                             {{0, buffer1, 0, 4 * sizeof(float)}});

        vertexBuffer = utils::CreateBufferFromData<float>(
            device, wgpu::BufferUsage::Vertex,
            {// The bottom left triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 1.0f,

             // The top right triangle
             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f});
    }

    utils::BasicRenderPass renderPass;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;
    wgpu::BindGroup bindGroups[2];
};

// Basic test of RenderBundle.
TEST_P(RenderBundleTest, Basic) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

    renderBundleEncoder.SetPipeline(pipeline);
    renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
    renderBundleEncoder.SetBindGroup(0, bindGroups[0]);
    renderBundleEncoder.Draw(6);

    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.ExecuteBundles(1, &renderBundle);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 3, 1);
}

// Test execution of multiple render bundles
TEST_P(RenderBundleTest, MultipleBundles) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    wgpu::RenderBundle renderBundles[2];
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
        renderBundleEncoder.SetBindGroup(0, bindGroups[0]);
        renderBundleEncoder.Draw(3);

        renderBundles[0] = renderBundleEncoder.Finish();
    }
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
        renderBundleEncoder.SetBindGroup(0, bindGroups[1]);
        renderBundleEncoder.Draw(3, 1, 3);

        renderBundles[1] = renderBundleEncoder.Finish();
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.ExecuteBundles(2, renderBundles);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(kColors[1], renderPass.color, 3, 1);
}

// Test execution of a bundle along with render pass commands.
TEST_P(RenderBundleTest, BundleAndRenderPassCommands) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

    renderBundleEncoder.SetPipeline(pipeline);
    renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
    renderBundleEncoder.SetBindGroup(0, bindGroups[0]);
    renderBundleEncoder.Draw(3);

    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.ExecuteBundles(1, &renderBundle);

    pass.SetPipeline(pipeline);
    pass.SetVertexBuffer(0, vertexBuffer);
    pass.SetBindGroup(0, bindGroups[1]);
    pass.Draw(3, 1, 3);

    pass.ExecuteBundles(1, &renderBundle);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 1, 3);
    EXPECT_PIXEL_RGBA8_EQ(kColors[1], renderPass.color, 3, 1);
}

// Uses the same render bundle with different viewport settings.
// The render target is 4x4. We render to (0, 0), (2, 0), (0, 2), (2, 2).
// Then we check those pixels were rendered to and a few adjacent
// pixel were not.
TEST_P(RenderBundleTest, ExecuteSameBundleMultipleTimes) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

    renderBundleEncoder.SetPipeline(pipeline);
    renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
    renderBundleEncoder.SetBindGroup(0, bindGroups[0]);
    renderBundleEncoder.Draw(6);

    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetViewport(0.f, 0.f, 1.f, 1.f, 0.f, 1.f);
    pass.ExecuteBundles(1, &renderBundle);
    pass.SetViewport(2.f, 0.f, 1.f, 1.f, 0.f, 1.f);
    pass.ExecuteBundles(1, &renderBundle);
    pass.SetViewport(0.f, 2.f, 1.f, 1.f, 0.f, 1.f);
    pass.ExecuteBundles(1, &renderBundle);
    pass.SetViewport(2.f, 2.f, 1.f, 1.f, 0.f, 1.f);
    pass.ExecuteBundles(1, &renderBundle);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 2, 0);
    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 0, 2);
    EXPECT_PIXEL_RGBA8_EQ(kColors[0], renderPass.color, 2, 2);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 1, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 0, 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 0, 3);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 3, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 3, 3);
}

// Uses the same render bundle in the same ExecuteBundles call with
// additive blending.
TEST_P(RenderBundleTest, ExecuteSameBundleMultipleTimesInSameExecuteBundles) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs(@location(0) pos : vec4f) -> @builtin(position) vec4f {
            return pos;
        }

        @fragment fn fs() -> @location(0) vec4f {
            return vec4f(1.1 / 255, 2.1 / 255, 3.1 / 255, 4.1 / 255);
        }
    )");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = module;
    descriptor.cFragment.module = module;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    descriptor.vertex.bufferCount = 1;
    descriptor.cBuffers[0].arrayStride = 4 * sizeof(float);
    descriptor.cBuffers[0].attributeCount = 1;
    descriptor.cAttributes[0].format = wgpu::VertexFormat::Float32x4;
    descriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::BlendState blend;
    blend.color.operation = wgpu::BlendOperation::Add;
    blend.color.srcFactor = wgpu::BlendFactor::One;
    blend.color.dstFactor = wgpu::BlendFactor::One;
    blend.alpha.operation = wgpu::BlendOperation::Add;
    blend.alpha.srcFactor = wgpu::BlendFactor::One;
    blend.alpha.dstFactor = wgpu::BlendFactor::One;

    descriptor.cTargets[0].blend = &blend;

    pipeline = device.CreateRenderPipeline(&descriptor);

    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

    renderBundleEncoder.SetPipeline(pipeline);
    renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
    renderBundleEncoder.Draw(6);

    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();
    wgpu::RenderBundle renderBundles[3] = {renderBundle, renderBundle, renderBundle};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.ExecuteBundles(3, &renderBundles[0]);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    utils::RGBA8 expected(3, 6, 9, 12);
    EXPECT_PIXEL_RGBA8_EQ(expected, renderPass.color, 0, 0);
}

DAWN_INSTANTIATE_TEST(RenderBundleTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
