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

#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "src/dawn/utils/ComboRenderPipelineDescriptor.h"
#include "src/dawn/utils/WGPUHelpers.h"

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

// RenderBundleIndirectValidationTest tests validation of drawIndirect and drawIndexedIndirect in a
// render bundle.
class RenderBundleIndirectValidationTest : public DawnTest {
  protected:
    wgpu::Buffer CreateIndirectBuffer(std::initializer_list<uint32_t> indirectParamList) {
        return utils::CreateBufferFromData<uint32_t>(
            device, wgpu::BufferUsage::Indirect | wgpu::BufferUsage::Storage, indirectParamList);
    }

    wgpu::Buffer CreateIndexBuffer(std::initializer_list<uint32_t> indexList) {
        return utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, indexList);
    }
};

// Tests a bug outlined in crbug.com/495489174 where indirect draw validation could be bypassed in
// a specific scenario where the a render bundle with the indirect draw was executed multiple times
// in a single encoder. Test based on a POC produced for that issue.
TEST_P(RenderBundleIndirectValidationTest, RepeatedIndirectDrawValidation) {
    const uint32_t OOB_COUNT = 100000;

    // Render Pass
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Index Buffers
    wgpu::Buffer smallIdx = CreateIndexBuffer({0, 1, 2});

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = OOB_COUNT * sizeof(uint32_t);
    bufferDesc.usage = wgpu::BufferUsage::Index;
    wgpu::Buffer bigIdx = device.CreateBuffer(&bufferDesc);

    // Indirect buffer
    wgpu::Buffer indirect = CreateIndirectBuffer({3, 1, 0, 0, 0, OOB_COUNT, 1, 0, 0, 0});

    // Buffers to use for simple fragment counter
    uint32_t data[] = {0};
    wgpu::Buffer counterBuffer = utils::CreateBufferFromData(
        device, data, sizeof(uint32_t),
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);
    wgpu::Buffer counterRead = utils::CreateBufferFromData(
        device, data, sizeof(uint32_t), wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);

    // Pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct Ctr { n: atomic<u32>, };
        @group(0) @binding(0) var<storage, read_write> ctr: Ctr;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
        // Simply adds one to the simple fragment counter with each draw
        @fragment fn fs() -> @location(0) vec4f {
            atomicAdd(&ctr.n, 1u);
            return vec4f(1.0, 0.0, 0.0, 1.0);
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = module;
    descriptor.cFragment.module = module;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
    descriptor.cTargets[0].format = renderPass.colorFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // Bind group
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, counterBuffer, 0, sizeof(float)}});

    // Render bundle
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
    renderBundleEncoder.SetPipeline(pipeline);
    renderBundleEncoder.SetBindGroup(0, bindGroup);
    renderBundleEncoder.SetIndexBuffer(smallIdx, wgpu::IndexFormat::Uint32);
    renderBundleEncoder.DrawIndexedIndirect(indirect, 0);
    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    //
    // Warm up
    // Primes device-global scratchIndirectStorage so that
    // scratch[20..40] = validated({OOB_COUNT,1,0,0,0}) = {OOB_COUNT,1,0,0,0}
    //
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.SetIndexBuffer(bigIdx, wgpu::IndexFormat::Uint32);
        pass.DrawIndexedIndirect(indirect, 0);
        pass.DrawIndexedIndirect(indirect, 20);
        pass.End();

        // Copy the fragment counter results to the readback buffer.
        encoder.CopyBufferToBuffer(counterBuffer, 0, counterRead, 0, 4);

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
        queue.WriteBuffer(counterBuffer, 0, data, sizeof(uint32_t));
    }

    // The warm up pass should always produce OOB_COUNT + 3 fragments from the two indirect draws.
    EXPECT_BUFFER_U32_EQ(OOB_COUNT + 3, counterRead, 0);

    //
    // Bug - Same bundle executed in two render passes of one encoder.
    // Pass 1: Bundle only. Validation rewrites the bundle's persistent
    //         DrawIndexedIndirectCmd -> {scratch, offset 0} and encodes a
    //         compute pass that will write zeros to scratch[0..20].
    // Pass 2: One direct drawIndexedIndirect via the SAME indirect buffer,
    //         then the bundle. The two draws merge into one validation batch
    //         [direct, bundle], so the bundle's persistent cmd is rewritten
    //         again -> {scratch, offset 20}.
    // At submit time the backend reads the bundle's cmd while replaying
    // Pass 1's ExecuteBundles. It now points at scratch[20..40], which still
    // holds the warm up's validated {OOB_COUNT,1,0,0,0}. Pass 2's compute pass
    // hasn't run yet, so Pass 1 issues an indexed indirect draw with
    // indexCount=OOB_COUNT against a 3-element index buffer.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.ExecuteBundles(1, &renderBundle);
            pass.End();
        }
        // Copy the fragment counter results of only the first pass to the readback buffer.
        encoder.CopyBufferToBuffer(counterBuffer, 0, counterRead, 0, 4);
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.SetIndexBuffer(smallIdx, wgpu::IndexFormat::Uint32);
            pass.DrawIndexedIndirect(indirect, 0);
            pass.ExecuteBundles(1, &renderBundle);
            pass.End();
        }
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // The first pass (the one we're measuring) should only produce 3 fragments. If the validation
    // bug is present, however, it will produce OOB_COUNT fragments.
    EXPECT_BUFFER_U32_EQ(3, counterRead, 0);
}

// Tests a bug outlined in crbug.com/520972775 where repeated execution of bundles with an indirect
// draw could de-duplicate the draw calls when validating, causing the ValidatedIndirectDraw side
// table to have too few entries.
TEST_P(RenderBundleIndirectValidationTest, SinglePassRepeatedIndirectDraw) {
    // Render Pass
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Index Buffers
    wgpu::Buffer smallIdx = CreateIndexBuffer({0, 1, 2});

    // Indirect buffer
    wgpu::Buffer indirect = CreateIndirectBuffer({3, 1, 0, 0, 0});

    // Buffers to use for simple fragment counter
    uint32_t data[] = {0};
    wgpu::Buffer counterBuffer = utils::CreateBufferFromData(
        device, data, sizeof(uint32_t),
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);
    wgpu::Buffer counterRead = utils::CreateBufferFromData(
        device, data, sizeof(uint32_t), wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);

    // Pipeline
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        struct Ctr { n: atomic<u32>, };
        @group(0) @binding(0) var<storage, read_write> ctr: Ctr;

        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.0, 1.0);
        }
        // Simply adds one to the simple fragment counter with each draw
        @fragment fn fs() -> @location(0) vec4f {
            atomicAdd(&ctr.n, 1u);
            return vec4f(1.0, 0.0, 0.0, 1.0);
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = module;
    descriptor.cFragment.module = module;
    descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
    descriptor.cTargets[0].format = renderPass.colorFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // Bind group
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, counterBuffer, 0, sizeof(float)}});

    // Render bundles
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.colorFormat;

    wgpu::RenderBundle directRenderBundle;
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetBindGroup(0, bindGroup);
        renderBundleEncoder.DrawIndirect(indirect, 0);
        directRenderBundle = renderBundleEncoder.Finish();
    }

    wgpu::RenderBundle indexedRenderBundle;
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetBindGroup(0, bindGroup);
        renderBundleEncoder.SetIndexBuffer(smallIdx, wgpu::IndexFormat::Uint32);
        renderBundleEncoder.DrawIndexedIndirect(indirect, 0);
        indexedRenderBundle = renderBundleEncoder.Finish();
    }

    //
    // Bug - Same bundle executed twice in a single render pass.
    //
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Test bundles with DrawIndirect calls
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.ExecuteBundles(1, &directRenderBundle);
        pass.ExecuteBundles(1, &directRenderBundle);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Test bundles with DrawIndexedIndirect calls
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.ExecuteBundles(1, &indexedRenderBundle);
        pass.ExecuteBundles(1, &indexedRenderBundle);
        pass.End();

        // Copy the fragment counter results of both passes to the readback buffer.
        encoder.CopyBufferToBuffer(counterBuffer, 0, counterRead, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Each bundle should produce 3 fragments, for a total of 12.
    EXPECT_BUFFER_U32_EQ(12, counterRead, 0);
}

DAWN_INSTANTIATE_TEST(RenderBundleIndirectValidationTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
