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

#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class RenderBundleValidationTest : public ValidationTest {
  protected:
    void SetUp() override {
        ValidationTest::SetUp();

        vsModule = utils::CreateShaderModule(device, R"(
                struct S {
                    transform : mat2x2<f32>
                }
                @group(0) @binding(0) var<uniform> uniforms : S;

                @vertex fn main(@location(0) pos : vec2f) -> @builtin(position) vec4f {
                    return vec4f();
                })");

        fsModule = utils::CreateShaderModule(device, R"(
                struct Uniforms {
                    color : vec4f
                }
                @group(1) @binding(0) var<uniform> uniforms : Uniforms;

                struct Storage {
                    placeholder : array<f32>
                }
                @group(1) @binding(1) var<storage, read_write> ssbo : Storage;

                @fragment fn main() {
                })");

        wgpu::BindGroupLayout bgls[] = {
            utils::MakeBindGroupLayout(
                device, {{0, wgpu::ShaderStage::Vertex, wgpu::BufferBindingType::Uniform}}),
            utils::MakeBindGroupLayout(
                device, {
                            {0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Uniform},
                            {1, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage},
                        })};

        wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {};
        pipelineLayoutDesc.bindGroupLayoutCount = 2;
        pipelineLayoutDesc.bindGroupLayouts = bgls;

        pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDesc);

        utils::ComboRenderPipelineDescriptor descriptor;
        InitializeRenderPipelineDescriptor(&descriptor);
        pipeline = device.CreateRenderPipeline(&descriptor);

        float data[8];
        wgpu::Buffer buffer = utils::CreateBufferFromData(device, data, 8 * sizeof(float),
                                                          wgpu::BufferUsage::Uniform);

        constexpr static float kVertices[] = {-1.f, 1.f, 1.f, -1.f, -1.f, 1.f};

        vertexBuffer = utils::CreateBufferFromData(device, kVertices, sizeof(kVertices),
                                                   wgpu::BufferUsage::Vertex);

        // Placeholder storage buffer.
        wgpu::Buffer storageBuffer = utils::CreateBufferFromData(
            device, kVertices, sizeof(kVertices), wgpu::BufferUsage::Storage);

        // Vertex buffer with storage usage for testing read+write error usage.
        vertexStorageBuffer =
            utils::CreateBufferFromData(device, kVertices, sizeof(kVertices),
                                        wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Storage);

        bg0 = utils::MakeBindGroup(device, bgls[0], {{0, buffer, 0, 8 * sizeof(float)}});
        bg1 = utils::MakeBindGroup(
            device, bgls[1],
            {{0, buffer, 0, 4 * sizeof(float)}, {1, storageBuffer, 0, sizeof(kVertices)}});

        bg1Vertex = utils::MakeBindGroup(
            device, bgls[1],
            {{0, buffer, 0, 8 * sizeof(float)}, {1, vertexStorageBuffer, 0, sizeof(kVertices)}});
    }

    void InitializeRenderPipelineDescriptor(utils::ComboRenderPipelineDescriptor* descriptor) {
        descriptor->layout = pipelineLayout;
        descriptor->vertex.module = vsModule;
        descriptor->cFragment.module = fsModule;
        descriptor->cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        descriptor->vertex.bufferCount = 1;
        descriptor->cBuffers[0].arrayStride = 2 * sizeof(float);
        descriptor->cBuffers[0].attributeCount = 1;
        descriptor->cAttributes[0].format = wgpu::VertexFormat::Float32x2;
        descriptor->cAttributes[0].shaderLocation = 0;
    }

    wgpu::ShaderModule vsModule;
    wgpu::ShaderModule fsModule;
    wgpu::PipelineLayout pipelineLayout;
    wgpu::RenderPipeline pipeline;
    wgpu::Buffer vertexBuffer;
    wgpu::Buffer vertexStorageBuffer;
    wgpu::BindGroup bg0;
    wgpu::BindGroup bg1;
    wgpu::BindGroup bg1Vertex;
};

// Test creating and encoding an empty render bundle.
TEST_F(RenderBundleValidationTest, Empty) {
    PlaceholderRenderPass renderPass(device);

    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.attachmentFormat;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
    pass.ExecuteBundles(1, &renderBundle);
    pass.End();
    commandEncoder.Finish();
}

// Test that an empty error bundle encoder produces an error bundle.
// This is a regression test for error render bundle encoders containing no commands would
// produce non-error render bundles.
TEST_F(RenderBundleValidationTest, EmptyErrorEncoderProducesErrorBundle) {
    PlaceholderRenderPass renderPass(device);

    utils::ComboRenderBundleEncoderDescriptor desc = {};
    // Having 0 attachments is invalid!
    desc.colorFormatCount = 0;

    wgpu::RenderBundleEncoder renderBundleEncoder;
    ASSERT_DEVICE_ERROR(renderBundleEncoder = device.CreateRenderBundleEncoder(&desc));
    wgpu::RenderBundle renderBundle;
    ASSERT_DEVICE_ERROR(renderBundle = renderBundleEncoder.Finish());

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
    pass.ExecuteBundles(1, &renderBundle);
    pass.End();
    ASSERT_DEVICE_ERROR(commandEncoder.Finish());
}

// Test executing zero render bundles.
TEST_F(RenderBundleValidationTest, ZeroBundles) {
    PlaceholderRenderPass renderPass(device);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
    pass.ExecuteBundles(0, nullptr);
    pass.End();
    commandEncoder.Finish();
}

// Test successfully creating and encoding a render bundle into a command buffer.
TEST_F(RenderBundleValidationTest, SimpleSuccess) {
    PlaceholderRenderPass renderPass(device);

    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.attachmentFormat;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
    renderBundleEncoder.SetPipeline(pipeline);
    renderBundleEncoder.SetBindGroup(0, bg0);
    renderBundleEncoder.SetBindGroup(1, bg1);
    renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
    renderBundleEncoder.Draw(3);
    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
    pass.ExecuteBundles(1, &renderBundle);
    pass.End();
    commandEncoder.Finish();
}

// Test that render bundle debug groups must be well nested.
TEST_F(RenderBundleValidationTest, DebugGroups) {
    PlaceholderRenderPass renderPass(device);

    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.attachmentFormat;

    // Test a single debug group works.
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.PushDebugGroup("group");
        renderBundleEncoder.PopDebugGroup();
        renderBundleEncoder.Finish();
    }

    // Test nested debug groups work.
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.PushDebugGroup("group");
        renderBundleEncoder.PushDebugGroup("group2");
        renderBundleEncoder.PopDebugGroup();
        renderBundleEncoder.PopDebugGroup();
        renderBundleEncoder.Finish();
    }

    // Test popping when no group is pushed is invalid.
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.PopDebugGroup();
        ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
    }

    // Test popping too many times is invalid.
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.PushDebugGroup("group");
        renderBundleEncoder.PopDebugGroup();
        renderBundleEncoder.PopDebugGroup();
        ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
    }

    // Test that a single debug group must be popped.
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.PushDebugGroup("group");
        ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
    }

    // Test that all debug groups must be popped.
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.PushDebugGroup("group");
        renderBundleEncoder.PushDebugGroup("group2");
        renderBundleEncoder.PopDebugGroup();
        ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
    }
}

// Test render bundles do not inherit command buffer state
TEST_F(RenderBundleValidationTest, StateInheritance) {
    PlaceholderRenderPass renderPass(device);

    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.attachmentFormat;

    // Render bundle does not inherit pipeline so the draw is invalid.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

        pass.SetPipeline(pipeline);

        renderBundleEncoder.SetBindGroup(0, bg0);
        renderBundleEncoder.SetBindGroup(1, bg1);
        renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
        renderBundleEncoder.Draw(3);
        ASSERT_DEVICE_ERROR(wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish());

        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Render bundle does not inherit bind groups so the draw is invalid.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);

        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
        renderBundleEncoder.Draw(3);
        ASSERT_DEVICE_ERROR(wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish());

        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Render bundle does not inherit pipeline and bind groups so the draw is invalid.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);

        renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
        renderBundleEncoder.Draw(3);
        ASSERT_DEVICE_ERROR(wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish());

        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Render bundle does not inherit buffers so the draw is invalid.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);

        pass.SetVertexBuffer(0, vertexBuffer);

        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetBindGroup(0, bg0);
        renderBundleEncoder.SetBindGroup(1, bg1);
        renderBundleEncoder.Draw(3);
        ASSERT_DEVICE_ERROR(wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish());

        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }
}

// Test render bundles do not persist command buffer state
TEST_F(RenderBundleValidationTest, StatePersistence) {
    PlaceholderRenderPass renderPass(device);

    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.attachmentFormat;

    // Render bundle does not persist pipeline so the draw is invalid.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);

        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

        pass.ExecuteBundles(1, &renderBundle);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.Draw(3);
        pass.End();

        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Render bundle does not persist bind groups so the draw is invalid.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);

        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.SetBindGroup(0, bg0);
        renderBundleEncoder.SetBindGroup(1, bg1);
        wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

        pass.ExecuteBundles(1, &renderBundle);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.Draw(3);
        pass.End();

        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Render bundle does not persist pipeline and bind groups so the draw is invalid.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);

        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetBindGroup(0, bg0);
        renderBundleEncoder.SetBindGroup(1, bg1);
        wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

        pass.ExecuteBundles(1, &renderBundle);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.Draw(3);
        pass.End();

        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Render bundle does not persist buffers so the draw is invalid.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);

        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
        wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

        pass.ExecuteBundles(1, &renderBundle);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);
        pass.Draw(3);
        pass.End();

        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }
}

// Test executing render bundles clears command buffer state
TEST_F(RenderBundleValidationTest, ClearsState) {
    PlaceholderRenderPass renderPass(device);

    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.attachmentFormat;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    // Render bundle clears pipeline so the draw is invalid.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);

        pass.SetPipeline(pipeline);
        pass.ExecuteBundles(1, &renderBundle);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.Draw(3);
        pass.End();

        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Render bundle clears bind groups so the draw is invalid.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);

        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);
        pass.ExecuteBundles(1, &renderBundle);
        pass.SetPipeline(pipeline);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.Draw(3);
        pass.End();

        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Render bundle clears pipeline and bind groups so the draw is invalid.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);

        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);
        pass.ExecuteBundles(1, &renderBundle);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.Draw(3);
        pass.End();

        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Render bundle clears buffers so the draw is invalid.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);

        pass.SetVertexBuffer(0, vertexBuffer);
        pass.ExecuteBundles(1, &renderBundle);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);
        pass.Draw(3);
        pass.End();

        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Test executing 0 bundles still clears command buffer state.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);

        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.ExecuteBundles(0, nullptr);
        pass.Draw(3);

        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }
}

// Test creating and encoding multiple render bundles.
TEST_F(RenderBundleValidationTest, MultipleBundles) {
    PlaceholderRenderPass renderPass(device);

    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.attachmentFormat;

    wgpu::RenderBundle renderBundles[2] = {};

    wgpu::RenderBundleEncoder renderBundleEncoder0 = device.CreateRenderBundleEncoder(&desc);
    renderBundleEncoder0.SetPipeline(pipeline);
    renderBundleEncoder0.SetBindGroup(0, bg0);
    renderBundleEncoder0.SetBindGroup(1, bg1);
    renderBundleEncoder0.SetVertexBuffer(0, vertexBuffer);
    renderBundleEncoder0.Draw(3);
    renderBundles[0] = renderBundleEncoder0.Finish();

    wgpu::RenderBundleEncoder renderBundleEncoder1 = device.CreateRenderBundleEncoder(&desc);
    renderBundleEncoder1.SetPipeline(pipeline);
    renderBundleEncoder1.SetBindGroup(0, bg0);
    renderBundleEncoder1.SetBindGroup(1, bg1);
    renderBundleEncoder1.SetVertexBuffer(0, vertexBuffer);
    renderBundleEncoder1.Draw(3);
    renderBundles[1] = renderBundleEncoder1.Finish();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
    pass.ExecuteBundles(2, renderBundles);
    pass.End();
    commandEncoder.Finish();
}

// Test that is is valid to execute a render bundle more than once.
TEST_F(RenderBundleValidationTest, ExecuteMultipleTimes) {
    PlaceholderRenderPass renderPass(device);

    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.attachmentFormat;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
    renderBundleEncoder.SetPipeline(pipeline);
    renderBundleEncoder.SetBindGroup(0, bg0);
    renderBundleEncoder.SetBindGroup(1, bg1);
    renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
    renderBundleEncoder.Draw(3);
    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
    pass.ExecuteBundles(1, &renderBundle);
    pass.ExecuteBundles(1, &renderBundle);
    pass.ExecuteBundles(1, &renderBundle);
    pass.End();
    commandEncoder.Finish();
}

// Test that it is an error to call Finish() on a render bundle encoder twice.
TEST_F(RenderBundleValidationTest, FinishTwice) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Uint;

    wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
    renderBundleEncoder.Finish();
    ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
}

// Test that it is invalid to create a render bundle with no texture formats
TEST_F(RenderBundleValidationTest, RequiresAtLeastOneTextureFormat) {
    // Test failure case.
    {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        ASSERT_DEVICE_ERROR(device.CreateRenderBundleEncoder(&desc));
    }

    // Test success with one color format.
    {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.colorFormatCount = 1;
        desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Uint;
        device.CreateRenderBundleEncoder(&desc);
    }

    // Test success with a depth stencil format.
    {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.depthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;
        device.CreateRenderBundleEncoder(&desc);
    }
}

// Test that it is invalid to create a render bundle with no texture formats
TEST_F(RenderBundleValidationTest, ColorFormatsCountOutOfBounds) {
    std::array<wgpu::TextureFormat, kMaxColorAttachments + 1> colorFormats;
    for (uint32_t i = 0; i < colorFormats.size(); ++i) {
        colorFormats[i] = wgpu::TextureFormat::R8Unorm;
    }

    // colorFormatCount <= kMaxColorAttachments is valid.
    {
        wgpu::RenderBundleEncoderDescriptor desc;
        desc.colorFormatCount = kMaxColorAttachments;
        desc.colorFormats = colorFormats.data();
        device.CreateRenderBundleEncoder(&desc);
    }

    // colorFormatCount > kMaxColorAttachments is invalid.
    {
        wgpu::RenderBundleEncoderDescriptor desc;
        desc.colorFormatCount = kMaxColorAttachments + 1;
        desc.colorFormats = colorFormats.data();
        ASSERT_DEVICE_ERROR(device.CreateRenderBundleEncoder(&desc));
    }
}

// Test that render bundle sparse color formats.
TEST_F(RenderBundleValidationTest, SparseColorFormats) {
    // Sparse color formats is valid.
    {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.colorFormatCount = 2;
        desc.cColorFormats[0] = wgpu::TextureFormat::Undefined;
        desc.cColorFormats[1] = wgpu::TextureFormat::RGBA8Unorm;
        device.CreateRenderBundleEncoder(&desc);
    }

    // When all color formats are undefined, depth stencil format must not be undefined.
    {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.colorFormatCount = 1;
        desc.cColorFormats[0] = wgpu::TextureFormat::Undefined;
        desc.depthStencilFormat = wgpu::TextureFormat::Undefined;
        ASSERT_DEVICE_ERROR(
            device.CreateRenderBundleEncoder(&desc),
            testing::HasSubstr(
                "No color or depthStencil attachments specified. At least one is required."));
    }
    {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.colorFormatCount = 1;
        desc.cColorFormats[0] = wgpu::TextureFormat::Undefined;
        desc.depthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;
        device.CreateRenderBundleEncoder(&desc);
    }
}

// Test that the render bundle depth stencil format cannot be set to undefined.
TEST_F(RenderBundleValidationTest, DepthStencilFormatUndefined) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.depthStencilFormat = wgpu::TextureFormat::Undefined;
    ASSERT_DEVICE_ERROR(device.CreateRenderBundleEncoder(&desc));
}

// Test that depthReadOnly and stencilReadOnly don't need to be the same for depth-stencil formats
TEST_F(RenderBundleValidationTest, DepthStencilReadOnly) {
    for (bool depthReadOnly : {true, false}) {
        for (bool stencilReadOnly : {true, false}) {
            utils::ComboRenderBundleEncoderDescriptor desc = {};
            desc.depthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;
            desc.depthReadOnly = depthReadOnly;
            desc.stencilReadOnly = stencilReadOnly;
            device.CreateRenderBundleEncoder(&desc);
        }
    }
}
// Test that resource usages are validated inside render bundles.
TEST_F(RenderBundleValidationTest, UsageTracking) {
    PlaceholderRenderPass renderPass(device);

    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.colorFormatCount = 1;
    desc.cColorFormats[0] = renderPass.attachmentFormat;

    wgpu::RenderBundle renderBundle0;
    wgpu::RenderBundle renderBundle1;

    // First base case is successful. |bg1Vertex| does not reference |vertexBuffer|.
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetBindGroup(0, bg0);
        renderBundleEncoder.SetBindGroup(1, bg1Vertex);
        renderBundleEncoder.SetVertexBuffer(0, vertexBuffer);
        renderBundleEncoder.Draw(3);
        renderBundle0 = renderBundleEncoder.Finish();
    }

    // Second base case is successful. |bg1| does not reference |vertexStorageBuffer|
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetBindGroup(0, bg0);
        renderBundleEncoder.SetBindGroup(1, bg1);
        renderBundleEncoder.SetVertexBuffer(0, vertexStorageBuffer);
        renderBundleEncoder.Draw(3);
        renderBundle1 = renderBundleEncoder.Finish();
    }

    // Test that a render bundle which sets a buffer as both vertex and storage is invalid.
    // |bg1Vertex| references |vertexStorageBuffer|
    {
        wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.SetBindGroup(0, bg0);
        renderBundleEncoder.SetBindGroup(1, bg1Vertex);
        renderBundleEncoder.SetVertexBuffer(0, vertexStorageBuffer);
        renderBundleEncoder.Draw(3);
        ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
    }

    // When both render bundles are in the same pass, |vertexStorageBuffer| is used
    // as both read and write usage. This is invalid.
    // renderBundle0 uses |vertexStorageBuffer| as a storage buffer.
    // renderBundle1 uses |vertexStorageBuffer| as a vertex buffer.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        pass.ExecuteBundles(1, &renderBundle0);
        pass.ExecuteBundles(1, &renderBundle1);
        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // |vertexStorageBuffer| is used as both read and write usage. This is invalid.
    // The render pass uses |vertexStorageBuffer| as a storage buffer.
    // renderBundle1 uses |vertexStorageBuffer| as a vertex buffer.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);

        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1Vertex);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.Draw(3);

        pass.ExecuteBundles(1, &renderBundle1);
        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // |vertexStorageBuffer| is used as both read and write usage. This is invalid.
    // renderBundle0 uses |vertexStorageBuffer| as a storage buffer.
    // The render pass uses |vertexStorageBuffer| as a vertex buffer.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);

        pass.ExecuteBundles(1, &renderBundle0);

        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bg0);
        pass.SetBindGroup(1, bg1);
        pass.SetVertexBuffer(0, vertexStorageBuffer);
        pass.Draw(3);

        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }
}

// Test that encoding SetPipline with an incompatible color format produces an error.
TEST_F(RenderBundleValidationTest, PipelineColorFormatMismatch) {
    utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
    renderBundleDesc.colorFormatCount = 3;
    renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
    renderBundleDesc.cColorFormats[1] = wgpu::TextureFormat::RG16Float;
    renderBundleDesc.cColorFormats[2] = wgpu::TextureFormat::R16Sint;

    auto SetupRenderPipelineDescForTest = [this](utils::ComboRenderPipelineDescriptor* desc) {
        InitializeRenderPipelineDescriptor(desc);
        desc->cFragment.targetCount = 3;
        desc->cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        desc->cTargets[1].format = wgpu::TextureFormat::RG16Float;
        desc->cTargets[2].format = wgpu::TextureFormat::R16Sint;
        desc->cTargets[0].writeMask = wgpu::ColorWriteMask::None;
        desc->cTargets[1].writeMask = wgpu::ColorWriteMask::None;
        desc->cTargets[2].writeMask = wgpu::ColorWriteMask::None;
    };

    // Test the success case.
    {
        utils::ComboRenderPipelineDescriptor desc;
        SetupRenderPipelineDescForTest(&desc);

        wgpu::RenderBundleEncoder renderBundleEncoder =
            device.CreateRenderBundleEncoder(&renderBundleDesc);
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.Finish();
    }

    // Test the failure case for mismatched format types.
    {
        utils::ComboRenderPipelineDescriptor desc;
        SetupRenderPipelineDescForTest(&desc);
        desc.cTargets[1].format = wgpu::TextureFormat::RGBA8Unorm;

        wgpu::RenderBundleEncoder renderBundleEncoder =
            device.CreateRenderBundleEncoder(&renderBundleDesc);
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
    }

    // Test the failure case for missing format
    {
        utils::ComboRenderPipelineDescriptor desc;
        SetupRenderPipelineDescForTest(&desc);
        desc.cFragment.targetCount = 2;

        wgpu::RenderBundleEncoder renderBundleEncoder =
            device.CreateRenderBundleEncoder(&renderBundleDesc);
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
    }
}

// Test that encoding SetPipline with an incompatible depth stencil format produces an error.
TEST_F(RenderBundleValidationTest, PipelineDepthStencilFormatMismatch) {
    utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
    renderBundleDesc.colorFormatCount = 1;
    renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
    renderBundleDesc.depthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

    auto SetupRenderPipelineDescForTest = [this](utils::ComboRenderPipelineDescriptor* desc) {
        InitializeRenderPipelineDescriptor(desc);
        desc->cFragment.targetCount = 1;
        desc->cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    };

    // Test the success case.
    {
        utils::ComboRenderPipelineDescriptor desc;
        SetupRenderPipelineDescForTest(&desc);
        desc.EnableDepthStencil(wgpu::TextureFormat::Depth24PlusStencil8);

        wgpu::RenderBundleEncoder renderBundleEncoder =
            device.CreateRenderBundleEncoder(&renderBundleDesc);
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.Finish();
    }

    // Test the failure case for mismatched format.
    {
        utils::ComboRenderPipelineDescriptor desc;
        SetupRenderPipelineDescForTest(&desc);
        desc.EnableDepthStencil(wgpu::TextureFormat::Depth24Plus);

        wgpu::RenderBundleEncoder renderBundleEncoder =
            device.CreateRenderBundleEncoder(&renderBundleDesc);
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
    }

    // Test the failure case for missing format.
    {
        utils::ComboRenderPipelineDescriptor desc;
        SetupRenderPipelineDescForTest(&desc);
        desc.depthStencil = nullptr;

        wgpu::RenderBundleEncoder renderBundleEncoder =
            device.CreateRenderBundleEncoder(&renderBundleDesc);
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);
        renderBundleEncoder.SetPipeline(pipeline);
        ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
    }
}

// Test that encoding SetPipline with an incompatible sample count produces an error.
TEST_F(RenderBundleValidationTest, PipelineSampleCountMismatch) {
    utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
    renderBundleDesc.colorFormatCount = 1;
    renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
    renderBundleDesc.sampleCount = 4;

    utils::ComboRenderPipelineDescriptor renderPipelineDesc;
    InitializeRenderPipelineDescriptor(&renderPipelineDesc);
    renderPipelineDesc.cFragment.targetCount = 1;
    renderPipelineDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    renderPipelineDesc.multisample.count = 4;

    // Test the success case.
    {
        wgpu::RenderBundleEncoder renderBundleEncoder =
            device.CreateRenderBundleEncoder(&renderBundleDesc);
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDesc);
        renderBundleEncoder.SetPipeline(pipeline);
        renderBundleEncoder.Finish();
    }

    // Test the failure case.
    {
        renderPipelineDesc.multisample.count = 1;

        wgpu::RenderBundleEncoder renderBundleEncoder =
            device.CreateRenderBundleEncoder(&renderBundleDesc);
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&renderPipelineDesc);
        renderBundleEncoder.SetPipeline(pipeline);
        ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
    }
}

// Test that encoding ExecuteBundles with an incompatible color format produces an error.
TEST_F(RenderBundleValidationTest, RenderPassColorFormatMismatch) {
    utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
    renderBundleDesc.colorFormatCount = 3;
    renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
    renderBundleDesc.cColorFormats[1] = wgpu::TextureFormat::RG16Float;
    renderBundleDesc.cColorFormats[2] = wgpu::TextureFormat::R16Sint;

    wgpu::RenderBundleEncoder renderBundleEncoder =
        device.CreateRenderBundleEncoder(&renderBundleDesc);
    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    textureDesc.size = wgpu::Extent3D({400, 400, 1});

    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::Texture tex0 = device.CreateTexture(&textureDesc);

    textureDesc.format = wgpu::TextureFormat::RG16Float;
    wgpu::Texture tex1 = device.CreateTexture(&textureDesc);

    textureDesc.format = wgpu::TextureFormat::R16Sint;
    wgpu::Texture tex2 = device.CreateTexture(&textureDesc);

    // Test the success case
    {
        utils::ComboRenderPassDescriptor renderPass({
            tex0.CreateView(),
            tex1.CreateView(),
            tex2.CreateView(),
        });

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        commandEncoder.Finish();
    }

    // Test the failure case for mismatched format
    {
        utils::ComboRenderPassDescriptor renderPass({
            tex0.CreateView(),
            tex1.CreateView(),
            tex0.CreateView(),
        });

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Test the failure case for missing format
    {
        utils::ComboRenderPassDescriptor renderPass({
            tex0.CreateView(),
            tex1.CreateView(),
        });

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }
}

// Test that encoding ExecuteBundles with an incompatible depth stencil format produces an
// error.
TEST_F(RenderBundleValidationTest, RenderPassDepthStencilFormatMismatch) {
    utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
    renderBundleDesc.colorFormatCount = 1;
    renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
    renderBundleDesc.depthStencilFormat = wgpu::TextureFormat::Depth24Plus;

    wgpu::RenderBundleEncoder renderBundleEncoder =
        device.CreateRenderBundleEncoder(&renderBundleDesc);
    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    textureDesc.size = wgpu::Extent3D({400, 400, 1});

    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::Texture tex0 = device.CreateTexture(&textureDesc);

    textureDesc.format = wgpu::TextureFormat::Depth24Plus;
    wgpu::Texture tex1 = device.CreateTexture(&textureDesc);

    textureDesc.format = wgpu::TextureFormat::Depth32Float;
    wgpu::Texture tex2 = device.CreateTexture(&textureDesc);

    // Test the success case
    {
        utils::ComboRenderPassDescriptor renderPass({tex0.CreateView()}, tex1.CreateView());
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        commandEncoder.Finish();
    }

    // Test the failure case for mismatched format
    {
        utils::ComboRenderPassDescriptor renderPass({tex0.CreateView()}, tex2.CreateView());
        renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }

    // Test the failure case for missing format
    {
        utils::ComboRenderPassDescriptor renderPass({tex0.CreateView()});

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }
}

// Test that encoding ExecuteBundles with an incompatible sample count produces an error.
TEST_F(RenderBundleValidationTest, RenderPassSampleCountMismatch) {
    utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
    renderBundleDesc.colorFormatCount = 1;
    renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::RenderBundleEncoder renderBundleEncoder =
        device.CreateRenderBundleEncoder(&renderBundleDesc);
    wgpu::RenderBundle renderBundle = renderBundleEncoder.Finish();

    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    textureDesc.size = wgpu::Extent3D({400, 400, 1});

    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::Texture tex0 = device.CreateTexture(&textureDesc);

    textureDesc.sampleCount = 4;
    wgpu::Texture tex1 = device.CreateTexture(&textureDesc);

    // Test the success case
    {
        utils::ComboRenderPassDescriptor renderPass({tex0.CreateView()});

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        commandEncoder.Finish();
    }

    // Test the failure case
    {
        utils::ComboRenderPassDescriptor renderPass({tex1.CreateView()});

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&renderPass);
        pass.ExecuteBundles(1, &renderBundle);
        pass.End();
        ASSERT_DEVICE_ERROR(commandEncoder.Finish());
    }
}

// Test that color attachment texture formats must be color renderable and
// depth stencil texture formats must be depth/stencil.
TEST_F(RenderBundleValidationTest, TextureFormats) {
    // Test that color formats are validated as color.
    {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.colorFormatCount = 1;
        desc.cColorFormats[0] = wgpu::TextureFormat::Depth24PlusStencil8;
        ASSERT_DEVICE_ERROR(device.CreateRenderBundleEncoder(&desc));
    }

    // Test that color formats are validated as renderable.
    {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.colorFormatCount = 1;
        desc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Snorm;
        ASSERT_DEVICE_ERROR(device.CreateRenderBundleEncoder(&desc));
    }

    // Test that depth/stencil formats are validated as depth/stencil.
    {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.depthStencilFormat = wgpu::TextureFormat::RGBA8Unorm;
        ASSERT_DEVICE_ERROR(device.CreateRenderBundleEncoder(&desc));
    }

    // Don't test non-renerable depth/stencil formats because we don't have any.
}

// Tests validation for per-pixel accounting for render targets. The tests currently assume that the
// default maxColorAttachmentBytesPerSample limit of 32 is used.
TEST_F(RenderBundleValidationTest, RenderBundleColorFormatsBytesPerSample) {
    struct TestCase {
        std::vector<wgpu::TextureFormat> formats;
        bool success;
    };
    static std::vector<TestCase> kTestCases = {
        // Simple 1 format cases.

        // R8Unorm take 1 byte and are aligned to 1 byte so we can have 8 (max).
        {{wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Unorm},
         true},
        // RGBA8Uint takes 4 bytes and are aligned to 1 byte so we can have 8 (max).
        {{wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint,
          wgpu::TextureFormat::RGBA8Uint, wgpu::TextureFormat::RGBA8Uint},
         true},
        // RGBA8Unorm takes 8 bytes (special case) and are aligned to 1 byte so only 4 allowed.
        {{wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm},
         true},
        {{wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA8Unorm},
         false},
        // RGBA32Float takes 16 bytes and are aligned to 4 bytes so only 2 are allowed.
        {{wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::RGBA32Float}, true},
        {{wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::RGBA32Float,
          wgpu::TextureFormat::RGBA32Float},
         false},

        // Different format alignment cases.

        // Alignment causes the first 1 byte R8Unorm to become 4 bytes. So even though 1+4+8+16+1 <
        // 32, the 4 byte alignment requirement of R32Float makes the first R8Unorm become 4 and
        // 4+4+8+16+1 > 32. Re-ordering this so the R8Unorm's are at the end, however is allowed:
        // 4+8+16+1+1 < 32.
        {{wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R32Float,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA32Float,
          wgpu::TextureFormat::R8Unorm},
         false},
        {{wgpu::TextureFormat::R32Float, wgpu::TextureFormat::RGBA8Unorm,
          wgpu::TextureFormat::RGBA32Float, wgpu::TextureFormat::R8Unorm,
          wgpu::TextureFormat::R8Unorm},
         true},
    };

    for (const TestCase& testCase : kTestCases) {
        utils::ComboRenderBundleEncoderDescriptor descriptor;
        descriptor.colorFormatCount = testCase.formats.size();
        for (size_t i = 0; i < testCase.formats.size(); i++) {
            descriptor.cColorFormats[i] = testCase.formats.at(i);
        }
        if (testCase.success) {
            device.CreateRenderBundleEncoder(&descriptor);
        } else {
            ASSERT_DEVICE_ERROR(device.CreateRenderBundleEncoder(&descriptor));
        }
    }
}

}  // anonymous namespace
}  // namespace dawn
