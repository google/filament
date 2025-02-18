// Copyright 2018 The Dawn & Tint Authors
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

class ScissorTest : public DawnTest {
  protected:
    wgpu::RenderPipeline CreateQuadPipeline(wgpu::TextureFormat format) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0, -1.0),
                    vec2f(-1.0,  1.0),
                    vec2f( 1.0, -1.0),
                    vec2f( 1.0,  1.0),
                    vec2f(-1.0,  1.0),
                    vec2f( 1.0, -1.0));
                return vec4f(pos[VertexIndex], 0.5, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 1.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.cTargets[0].format = format;

        return device.CreateRenderPipeline(&descriptor);
    }
};

// Test that by default the scissor test is disabled and the whole attachment can be drawn to.
TEST_P(ScissorTest, DefaultsToWholeRenderTarget) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 100, 100);
    wgpu::RenderPipeline pipeline = CreateQuadPipeline(renderPass.colorFormat);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 0, 99);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 99, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 99, 99);
}

// Test setting a partial scissor (not empty, not full attachment)
TEST_P(ScissorTest, PartialRect) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 100, 100);
    wgpu::RenderPipeline pipeline = CreateQuadPipeline(renderPass.colorFormat);

    constexpr uint32_t kX = 3;
    constexpr uint32_t kY = 7;
    constexpr uint32_t kW = 5;
    constexpr uint32_t kH = 13;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetScissorRect(kX, kY, kW, kH);
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Test the two opposite corners of the scissor box. With one pixel inside and on outside
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, kX - 1, kY - 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, kX, kY);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, kX + kW, kY + kH);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, kX + kW - 1, kY + kH - 1);
}

// Test setting an empty scissor
TEST_P(ScissorTest, EmptyRect) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 2, 2);
    wgpu::RenderPipeline pipeline = CreateQuadPipeline(renderPass.colorFormat);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetScissorRect(1, 1, 0, 0);
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Test that no pixel was written.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 0, 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 1, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 1, 1);
}
// Test that the scissor setting doesn't get inherited between renderpasses
TEST_P(ScissorTest, NoInheritanceBetweenRenderPass) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 100, 100);
    wgpu::RenderPipeline pipeline = CreateQuadPipeline(renderPass.colorFormat);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    // RenderPass 1 set the scissor
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetScissorRect(1, 1, 1, 1);
        pass.End();
    }
    // RenderPass 2 draw a full quad, it shouldn't be scissored
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 0, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 0, 99);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 99, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPass.color, 99, 99);
}

DAWN_INSTANTIATE_TEST(ScissorTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
