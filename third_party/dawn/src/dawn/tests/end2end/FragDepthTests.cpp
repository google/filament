// Copyright 2022 The Dawn & Tint Authors
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

constexpr wgpu::TextureFormat kDepthFormat = wgpu::TextureFormat::Depth32Float;

class FragDepthTests : public DawnTest {};

// Test that when writing to FragDepth the result is clamped to the viewport.
TEST_P(FragDepthTests, FragDepthIsClampedToViewport) {
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.5, 1.0);
        }

        @fragment fn fs() -> @builtin(frag_depth) f32 {
            return 1.0;
        }
    )");

    // Create the pipeline that uses frag_depth to output the depth.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pDesc.cFragment.module = module;
    pDesc.cFragment.targetCount = 0;

    wgpu::DepthStencilState* pDescDS = pDesc.EnableDepthStencil(kDepthFormat);
    pDescDS->depthWriteEnabled = wgpu::OptionalBool::True;
    pDescDS->depthCompare = wgpu::CompareFunction::Always;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    // Create a depth-only render pass.
    wgpu::TextureDescriptor depthDesc;
    depthDesc.size = {1, 1};
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    depthDesc.format = kDepthFormat;
    wgpu::Texture depthTexture = device.CreateTexture(&depthDesc);

    utils::ComboRenderPassDescriptor renderPassDesc({}, depthTexture.CreateView());
    renderPassDesc.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
    renderPassDesc.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

    // Draw a point with a skewed viewport, so 1.0 depth gets clamped to 0.5.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
    pass.SetViewport(0, 0, 1, 1, 0.0, 0.5);
    pass.SetPipeline(pipeline);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_FLOAT_EQ(0.5f, depthTexture, 0, 0);
}

// Test for the push constant logic for ClampFragDepth in Vulkan to check that changing the
// pipeline layout doesn't invalidate the push constants that were set.
TEST_P(FragDepthTests, ChangingPipelineLayoutDoesntInvalidateViewport) {
    // TODO(dawn:1805): Load ByteAddressBuffer in Pixel Shader doesn't work with NVIDIA on D3D11
    DAWN_SUPPRESS_TEST_IF(IsD3D11() && IsNvidia());

    // TODO(dawn:2393): ANGLE/D3D11 fails in HLSL shader compilation (UAV vs PS register bug)
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 0.5, 1.0);
        }

        @group(0) @binding(0) var<uniform> uniformDepth1 : f32;
        @fragment fn fsUniform1() -> @builtin(frag_depth) f32 {
            return uniformDepth1;
        }

        @group(0) @binding(0) var tex : texture_2d<f32>;
        @fragment fn fsUniform2() -> @builtin(frag_depth) f32 {
            return textureLoad(tex, vec2u(0), 0).r;
        }
    )");

    // Create the pipeline and bindgroup for the pipeline layout with a uniform buffer.
    utils::ComboRenderPipelineDescriptor upDesc;
    upDesc.vertex.module = module;
    upDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    upDesc.cFragment.module = module;
    upDesc.cFragment.entryPoint = "fsUniform1";
    upDesc.cFragment.targetCount = 0;

    wgpu::DepthStencilState* upDescDS = upDesc.EnableDepthStencil(kDepthFormat);
    upDescDS->depthWriteEnabled = wgpu::OptionalBool::True;
    upDescDS->depthCompare = wgpu::CompareFunction::Always;
    wgpu::RenderPipeline uniformPipeline = device.CreateRenderPipeline(&upDesc);

    wgpu::Buffer uniformBuffer =
        utils::CreateBufferFromData<float>(device, wgpu::BufferUsage::Uniform, {0.0});
    wgpu::BindGroup uniformBG =
        utils::MakeBindGroup(device, uniformPipeline.GetBindGroupLayout(0), {{0, uniformBuffer}});

    // Create the pipeline and bindgroup for the pipeline layout with a uniform buffer.
    utils::ComboRenderPipelineDescriptor spDesc;
    spDesc.vertex.module = module;
    spDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    spDesc.cFragment.module = module;
    spDesc.cFragment.entryPoint = "fsUniform2";
    spDesc.cFragment.targetCount = 0;

    wgpu::DepthStencilState* spDescDS = spDesc.EnableDepthStencil(kDepthFormat);
    spDescDS->depthWriteEnabled = wgpu::OptionalBool::True;
    spDescDS->depthCompare = wgpu::CompareFunction::Always;
    wgpu::RenderPipeline texturePipeline = device.CreateRenderPipeline(&spDesc);

    wgpu::TextureDescriptor texDesc = {};
    texDesc.size = {1, 1, 1};
    texDesc.format = wgpu::TextureFormat::R32Float;
    texDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    wgpu::Texture texture = device.CreateTexture(&texDesc);

    wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
        utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
    wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
        utils::CreateTexelCopyBufferLayout(0, sizeof(float));
    wgpu::Extent3D copyExtent = {1, 1, 1};

    float one = 1.0;
    queue.WriteTexture(&texelCopyTextureInfo, &one, sizeof(float), &texelCopyBufferLayout,
                       &copyExtent);

    wgpu::BindGroup textureBG = utils::MakeBindGroup(device, texturePipeline.GetBindGroupLayout(0),
                                                     {{0, texture.CreateView()}});

    // Create a depth-only render pass.
    wgpu::TextureDescriptor depthDesc;
    depthDesc.size = {1, 1};
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    depthDesc.format = kDepthFormat;
    wgpu::Texture depthTexture = device.CreateTexture(&depthDesc);

    utils::ComboRenderPassDescriptor renderPassDesc({}, depthTexture.CreateView());
    renderPassDesc.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
    renderPassDesc.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

    // Draw two point with a different pipeline layout to check Vulkan's behavior.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
    pass.SetViewport(0, 0, 1, 1, 0.0, 0.5);

    // Writes 0.0.
    pass.SetPipeline(uniformPipeline);
    pass.SetBindGroup(0, uniformBG);
    pass.Draw(1);

    // Writes 1.0 clamped to 0.5.
    pass.SetPipeline(texturePipeline);
    pass.SetBindGroup(0, textureBG);
    pass.Draw(1);

    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_FLOAT_EQ(0.5f, depthTexture, 0, 0);
}

// Check that if the fragment is outside of the viewport during rasterization, it is clipped
// even if it output @builtin(frag_depth).
TEST_P(FragDepthTests, RasterizationClipBeforeFS) {
    // TODO(dawn:1616): Metal too needs to clamping of @builtin(frag_depth) to the viewport.
    DAWN_SUPPRESS_TEST_IF(IsMetal());

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @vertex fn vs() -> @builtin(position) vec4f {
            return vec4f(0.0, 0.0, 5.0, 1.0);
        }

        @fragment fn fs() -> @builtin(frag_depth) f32 {
            return 0.5;
        }
    )");

    // Create the pipeline and bindgroup for the pipeline layout with a uniform buffer.
    utils::ComboRenderPipelineDescriptor pDesc;
    pDesc.vertex.module = module;
    pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
    pDesc.cFragment.module = module;
    pDesc.cFragment.targetCount = 0;

    wgpu::DepthStencilState* pDescDS = pDesc.EnableDepthStencil(kDepthFormat);
    pDescDS->depthWriteEnabled = wgpu::OptionalBool::True;
    pDescDS->depthCompare = wgpu::CompareFunction::Always;
    wgpu::RenderPipeline uniformPipeline = device.CreateRenderPipeline(&pDesc);

    // Create a depth-only render pass.
    wgpu::TextureDescriptor depthDesc;
    depthDesc.size = {1, 1};
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    depthDesc.format = kDepthFormat;
    wgpu::Texture depthTexture = device.CreateTexture(&depthDesc);

    utils::ComboRenderPassDescriptor renderPassDesc({}, depthTexture.CreateView());
    renderPassDesc.cDepthStencilAttachmentInfo.depthClearValue = 0.0f;
    renderPassDesc.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
    renderPassDesc.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

    // Draw a point with a depth outside of the viewport. It should get discarded.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
    pass.SetPipeline(uniformPipeline);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The fragment should be discarded so the depth stayed 0.0, the depthClearValue.
    EXPECT_PIXEL_FLOAT_EQ(0.0f, depthTexture, 0, 0);
}

DAWN_INSTANTIATE_TEST(FragDepthTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
