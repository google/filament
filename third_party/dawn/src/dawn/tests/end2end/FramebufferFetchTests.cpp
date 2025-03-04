// Copyright 2023 The Dawn & Tint Authors
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

#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class FramebufferFetchTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::FramebufferFetch));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::FramebufferFetch})) {
            requiredFeatures.push_back(wgpu::FeatureName::FramebufferFetch);
        }
        return requiredFeatures;
    }

    void InitForSinglePoint(utils::ComboRenderPipelineDescriptor* desc) {
        desc->vertex.module = utils::CreateShaderModule(device, R"(
            @vertex fn vs() -> @builtin(position) vec4f {
                return vec4f(0, 0, 0, 1);
            }
        )");
        desc->primitive.topology = wgpu::PrimitiveTopology::PointList;
    }

    wgpu::Texture MakeAttachment(wgpu::TextureFormat format,
                                 wgpu::Extent3D size = {1, 1, 1},
                                 uint32_t sampleCount = 1) {
        wgpu::TextureDescriptor tDesc;
        tDesc.size = size;
        tDesc.sampleCount = sampleCount;
        tDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                      wgpu::TextureUsage::CopyDst;
        tDesc.format = format;
        return device.CreateTexture(&tDesc);
    }

    std::string kExt = "enable chromium_experimental_framebuffer_fetch;\n";
};

// A basic test of framebuffer fetch
TEST_P(FramebufferFetchTests, Basic) {
    // Pipeline that draw a point at the center that adds one to the attachment.
    utils::ComboRenderPipelineDescriptor pDesc;
    InitForSinglePoint(&pDesc);
    pDesc.cFragment.module = utils::CreateShaderModule(device, kExt + R"(
        @fragment fn main(@color(0) in : u32) -> @location(0) u32 {
            return in + 1;
        }
    )");
    pDesc.cTargets[0].format = wgpu::TextureFormat::R32Uint;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    wgpu::Texture texture = MakeAttachment(wgpu::TextureFormat::R32Uint);

    // Draw 10 points with the pipeline on the render target.
    utils::ComboRenderPassDescriptor passDesc({texture.CreateView()});
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);
    pass.SetPipeline(pipeline);
    pass.Draw(10);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The 10 points should have successfully used framebuffer fetch to do increment ten times
    // without races.
    EXPECT_TEXTURE_EQ(uint32_t(10), texture, {0, 0});
}

// Check that it is post-blend framebuffer fetch
TEST_P(FramebufferFetchTests, PostBlend) {
    // Pipeline that draw a point at the center that outputs the current attachment value with
    // additive blend, so multiplies by two.
    utils::ComboRenderPipelineDescriptor pDesc;
    InitForSinglePoint(&pDesc);
    pDesc.cFragment.module = utils::CreateShaderModule(device, kExt + R"(
        @fragment fn main(@color(0) in : vec4f) -> @location(0) vec4f {
            return in;
        }
    )");
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.cTargets[0].blend = &pDesc.cBlends[0];
    pDesc.cBlends[0].color.srcFactor = wgpu::BlendFactor::One;
    pDesc.cBlends[0].color.dstFactor = wgpu::BlendFactor::One;
    pDesc.cBlends[0].color.operation = wgpu::BlendOperation::Add;
    pDesc.cBlends[0].alpha = pDesc.cBlends[0].color;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    wgpu::Texture texture = MakeAttachment(wgpu::TextureFormat::RGBA8Unorm);

    // Starting with small value, then do *2 4 times.
    utils::ComboRenderPassDescriptor passDesc({texture.CreateView()});
    passDesc.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    passDesc.cColorAttachments[0].clearValue = {1 / 255.0, 2 / 255.0, 3 / 255.0, 4 / 255.0};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);
    pass.SetPipeline(pipeline);
    pass.Draw(4);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The 10 points should have successfully used framebuffer fetch to do increment ten times
    // without races.
    EXPECT_TEXTURE_EQ(utils::RGBA8(16, 32, 48, 64), texture, {0, 0});
}

// Check with multiple render attachments
TEST_P(FramebufferFetchTests, MultipleAttachments) {
    // Pipeline that draw a point at the center that adds 0/1/2/3 to multiple attachments.
    utils::ComboRenderPipelineDescriptor pDesc;
    InitForSinglePoint(&pDesc);
    pDesc.cFragment.module = utils::CreateShaderModule(device, kExt + R"(
        struct Out {
            @location(0) out0 : u32,
            @location(1) out1 : u32,
            @location(2) out2 : u32,
            @location(3) out3 : u32,
        }
        @fragment fn main(
            @color(0) in0 : u32,
            @color(1) in1 : u32,
            @color(2) in2 : u32,
            @color(3) in3 : u32,
        ) -> Out {
            var out : Out;
            out.out0 = in0 + 0;
            out.out1 = in1 + 1;
            out.out2 = in2 + 2;
            out.out3 = in3 + 3;
            return out;
        }
    )");
    pDesc.cFragment.targetCount = 4;
    pDesc.cTargets[0].format = wgpu::TextureFormat::R32Uint;
    pDesc.cTargets[1].format = wgpu::TextureFormat::R32Uint;
    pDesc.cTargets[2].format = wgpu::TextureFormat::R32Uint;
    pDesc.cTargets[3].format = wgpu::TextureFormat::R32Uint;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    wgpu::Texture texture0 = MakeAttachment(wgpu::TextureFormat::R32Uint);
    wgpu::Texture texture1 = MakeAttachment(wgpu::TextureFormat::R32Uint);
    wgpu::Texture texture2 = MakeAttachment(wgpu::TextureFormat::R32Uint);
    wgpu::Texture texture3 = MakeAttachment(wgpu::TextureFormat::R32Uint);

    // Draw 10 points with the pipeline on the render target.
    utils::ComboRenderPassDescriptor passDesc({
        texture0.CreateView(),
        texture1.CreateView(),
        texture2.CreateView(),
        texture3.CreateView(),
    });
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);
    pass.SetPipeline(pipeline);
    pass.Draw(10);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The 10 points should have successfully used framebuffer fetch to do increment ten times
    // without races.
    EXPECT_TEXTURE_EQ(uint32_t(0), texture0, {0, 0});
    EXPECT_TEXTURE_EQ(uint32_t(10), texture1, {0, 0});
    EXPECT_TEXTURE_EQ(uint32_t(20), texture2, {0, 0});
    EXPECT_TEXTURE_EQ(uint32_t(30), texture3, {0, 0});
}

// Check with multiple render attachments
TEST_P(FramebufferFetchTests, VariousFormats) {
    // Pipeline that draw a point at the center that adds / substracts one to attachment of various
    // formats.
    utils::ComboRenderPipelineDescriptor pDesc;
    InitForSinglePoint(&pDesc);
    pDesc.cFragment.module = utils::CreateShaderModule(device, kExt + R"(
        struct Out {
            @location(0) out0 : u32,
            @location(1) out1 : i32,
            @location(2) out2 : f32,
            @location(3) out3 : vec4f,
        }
        @fragment fn main(
            @color(0) in0 : u32,
            @color(1) in1 : i32,
            @color(2) in2 : f32,
            @color(3) in3 : vec4f,
        ) -> Out {
            var out : Out;
            out.out0 = in0 + 1;
            out.out1 = in1 - 1;
            out.out2 = in2 + 1;
            out.out3 = in3 + vec4f(1 / 255.0);
            return out;
        }
    )");
    pDesc.cFragment.targetCount = 4;
    pDesc.cTargets[0].format = wgpu::TextureFormat::R32Uint;
    pDesc.cTargets[1].format = wgpu::TextureFormat::R32Sint;
    pDesc.cTargets[2].format = wgpu::TextureFormat::R32Float;
    pDesc.cTargets[3].format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    wgpu::Texture texture0 = MakeAttachment(wgpu::TextureFormat::R32Uint);
    wgpu::Texture texture1 = MakeAttachment(wgpu::TextureFormat::R32Sint);
    wgpu::Texture texture2 = MakeAttachment(wgpu::TextureFormat::R32Float);
    wgpu::Texture texture3 = MakeAttachment(wgpu::TextureFormat::RGBA8Unorm);

    // Draw 10 points with the pipeline on the render target.
    utils::ComboRenderPassDescriptor passDesc({
        texture0.CreateView(),
        texture1.CreateView(),
        texture2.CreateView(),
        texture3.CreateView(),
    });
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);
    pass.SetPipeline(pipeline);
    pass.Draw(10);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The 10 points should have successfully used framebuffer fetch to do in/decrement ten times
    // without races.
    EXPECT_TEXTURE_EQ(uint32_t(10), texture0, {0, 0});
    EXPECT_TEXTURE_EQ(int32_t(-10), texture1, {0, 0});
    EXPECT_TEXTURE_EQ(static_cast<float>(10), texture2, {0, 0});
    EXPECT_TEXTURE_EQ(utils::RGBA8(10, 10, 10, 10), texture3, {0, 0});
}

// Tests load and store operations
// LoadOp::Clear and StoreOp::Store are basically always tested by other tests, so this one tests
// LoadOp::Load and StoreOp::Discard
TEST_P(FramebufferFetchTests, LoadAndStoreOpsReallyItsLoadAndDiscard) {
    // Pipeline that adds one to the first attachment but also saves it to the second attachment.
    utils::ComboRenderPipelineDescriptor pDesc;
    InitForSinglePoint(&pDesc);
    pDesc.cFragment.module = utils::CreateShaderModule(device, kExt + R"(
        struct Out {
            @location(0) out0 : u32,
            @location(1) out1 : u32,
        }
        @fragment fn main(@color(0) in0 : u32, @color(1) in1 : u32) -> Out {
            var out : Out;
            out.out0 = in0 + 1;
            out.out1 = out.out0;
            return out;
        }
    )");
    pDesc.cFragment.targetCount = 2;
    pDesc.cTargets[0].format = wgpu::TextureFormat::R32Uint;
    pDesc.cTargets[1].format = wgpu::TextureFormat::R32Uint;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pDesc);

    wgpu::Texture texture0 = MakeAttachment(wgpu::TextureFormat::R32Uint);
    wgpu::Texture texture1 = MakeAttachment(wgpu::TextureFormat::R32Uint);

    // Clear the first attachment to 1789 with a render pass.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor clearPassDesc({texture0.CreateView()});
    clearPassDesc.cColorAttachments[0].clearValue.r = 1789;
    wgpu::RenderPassEncoder clearPass = encoder.BeginRenderPass(&clearPassDesc);
    clearPass.End();

    // Draw 10 points with the pipeline on the render target.
    utils::ComboRenderPassDescriptor passDesc({texture0.CreateView(), texture1.CreateView()});
    passDesc.cColorAttachments[0].loadOp = wgpu::LoadOp::Load;
    passDesc.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);
    pass.SetPipeline(pipeline);
    pass.Draw(10);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The first attachment is discard, but it's value of (ten increments + loaded value) is in the
    // second attachment.
    EXPECT_TEXTURE_EQ(uint32_t(0), texture0, {0, 0});
    EXPECT_TEXTURE_EQ(uint32_t(10 + 1789), texture1, {0, 0});
}

// Test the behavior with multisampling.
// Checks that each sample has its own framebuffer fetch by add sample_index to that sample multiple
// times. Checks that any framebuffer fetch forces sample shading, even if all the samples have the
// same value.
TEST_P(FramebufferFetchTests, MultisamplingIsSampleRasterization) {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::BufferBindingType::Storage}});

    utils::ComboRenderPipelineDescriptor pDesc;
    InitForSinglePoint(&pDesc);
    pDesc.layout = utils::MakePipelineLayout(device, {bgl});
    pDesc.multisample.count = 4;
    pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pDesc.cFragment.module = utils::CreateShaderModule(device, kExt + R"(
        @group(0) @binding(0) var<storage, read_write> invocationCount : atomic<u32>;

        @fragment fn addOne(@color(0) in : vec4f) -> @location(0) vec4f {
            atomicAdd(&invocationCount, 1);
            return in + vec4(1 / 255.0);
        }

        @fragment fn addSampleIndex(
            @color(0) in : vec4f,
            @builtin(sample_index) sampleIndex : u32,
        ) -> @location(0) vec4f {
            atomicAdd(&invocationCount, 1);
            var out = in;
            out[sampleIndex] += f32(4 * sampleIndex) / 255.0;
            return out;
        }
    )");

    pDesc.cFragment.entryPoint = "addOne";
    wgpu::RenderPipeline addOnePipeline = device.CreateRenderPipeline(&pDesc);

    pDesc.cFragment.entryPoint = "addSampleIndex";
    wgpu::RenderPipeline addSampleCountPipeline = device.CreateRenderPipeline(&pDesc);

    // Create the buffer that will contain the number of invocations.
    wgpu::BufferDescriptor bufDesc;
    bufDesc.size = 4;
    bufDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer invocationBuffer = device.CreateBuffer(&bufDesc);
    wgpu::BindGroup bg = utils::MakeBindGroup(device, bgl, {{0, invocationBuffer}});

    wgpu::Texture attachment = MakeAttachment(wgpu::TextureFormat::RGBA8Unorm, {1, 1}, 4);
    wgpu::Texture resolve = MakeAttachment(wgpu::TextureFormat::RGBA8Unorm);

    // Run the test!!
    utils::ComboRenderPassDescriptor passDesc({attachment.CreateView()});
    passDesc.cColorAttachments[0].resolveTarget = resolve.CreateView();
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDesc);
    pass.SetBindGroup(0, bg);
    // This FS should use sample shading and run 4 times, adding one each time.
    pass.SetPipeline(addOnePipeline);
    pass.Draw(1);
    // This adds sample_index * 10 to each sample.
    pass.SetPipeline(addSampleCountPipeline);
    pass.Draw(10);
    // This FS should use sample shading and run 4 times, adding one each time.
    pass.SetPipeline(addOnePipeline);
    pass.Draw(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_EQ(4 + 10 * 4 + 4, invocationBuffer, 0);
    EXPECT_TEXTURE_EQ(utils::RGBA8(2 + 0 * 10, 2 + 1 * 10, 2 + 2 * 10, 2 + 3 * 10), resolve,
                      {0, 0});
}

DAWN_INSTANTIATE_TEST(FramebufferFetchTests, MetalBackend());

}  // anonymous namespace
}  // namespace dawn
