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

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ViewportTest : public DawnTest {
  private:
    void SetUp() override {
        DawnTest::SetUp();

        mQuadVS = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0,  1.0),
                    vec2f(-1.0, -1.0),
                    vec2f( 1.0,  1.0),
                    vec2f( 1.0,  1.0),
                    vec2f(-1.0, -1.0),
                    vec2f( 1.0, -1.0));
                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })");

        mQuadFS = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(1.0, 1.0, 1.0, 1.0);
            })");
    }

  protected:
    wgpu::ShaderModule mQuadVS;
    wgpu::ShaderModule mQuadFS;

    static constexpr uint32_t kWidth = 5;
    static constexpr uint32_t kHeight = 6;

    // Viewport parameters are float, but use uint32_t because implementations of Vulkan are allowed
    // to just discard the fractional part.
    void TestViewportQuad(uint32_t x,
                          uint32_t y,
                          uint32_t width,
                          uint32_t height,
                          bool doViewportCall = true) {
        // Create a pipeline that will draw a white quad.
        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = mQuadVS;
        pipelineDesc.cFragment.module = mQuadFS;
        pipelineDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        // Render the quad with the viewport call.
        utils::BasicRenderPass rp = utils::CreateBasicRenderPass(device, kWidth, kHeight);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp.renderPassInfo);
        pass.SetPipeline(pipeline);
        if (doViewportCall) {
            pass.SetViewport(x, y, width, height, 0.0, 1.0);
        }
        pass.Draw(6);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check that only the texels that are in the veiwport were drawn.
        for (uint32_t checkX = 0; checkX < kWidth; checkX++) {
            for (uint32_t checkY = 0; checkY < kHeight; checkY++) {
                if (checkX >= x && checkX < x + width && checkY >= y && checkY < y + height) {
                    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kWhite, rp.color, checkX, checkY);
                } else {
                    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, rp.color, checkX, checkY);
                }
            }
        }
    }

    void TestViewportDepth(float minDepth, float maxDepth, bool doViewportCall = true) {
        // Create a pipeline drawing 3 points at depth 1.0, 0.5 and 0.0.
        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var points : array<vec3f, 3> = array(
                    vec3f(-0.9, 0.0, 1.0),
                    vec3f( 0.0, 0.0, 0.5),
                    vec3f( 0.9, 0.0, 0.0));
                return vec4f(points[VertexIndex], 1.0);
            })");
        pipelineDesc.cFragment.module = mQuadFS;
        pipelineDesc.cFragment.targetCount = 0;
        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        wgpu::DepthStencilState* depthStencil =
            pipelineDesc.EnableDepthStencil(wgpu::TextureFormat::Depth32Float);
        depthStencil->depthWriteEnabled = wgpu::OptionalBool::True;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

        // Create the texture that will store the post-viewport-transform depth.
        wgpu::TextureDescriptor depthDesc;
        depthDesc.size = {3, 1, 1};
        depthDesc.format = wgpu::TextureFormat::Depth32Float;
        depthDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        wgpu::Texture depthTexture = device.CreateTexture(&depthDesc);

        // Render the three points with the viewport call.
        utils::ComboRenderPassDescriptor rpDesc({}, depthTexture.CreateView());
        rpDesc.cDepthStencilAttachmentInfo.depthClearValue = 0.0f;
        rpDesc.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
        rpDesc.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        rpDesc.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rpDesc);
        pass.SetPipeline(pipeline);
        if (doViewportCall) {
            pass.SetViewport(0, 0, 3, 1, minDepth, maxDepth);
        }
        pass.Draw(3);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Check that the viewport transform was computed correctly for the depth.
        std::vector<float> expected = {
            maxDepth,
            (maxDepth + minDepth) / 2,
            minDepth,
        };
        EXPECT_TEXTURE_EQ(expected.data(), depthTexture, {0, 0}, {3, 1});
    }
};

// Test that by default the full viewport is used.
TEST_P(ViewportTest, DefaultViewportRect) {
    TestViewportQuad(0, 0, kWidth, kHeight, false);
}

// Test various viewport values in the X direction.
TEST_P(ViewportTest, VaryingInX) {
    TestViewportQuad(0, 0, kWidth - 1, kHeight);
    TestViewportQuad(1, 0, kWidth - 1, kHeight);
    TestViewportQuad(2, 0, 1, kHeight);
}

// Test various viewport values in the Y direction.
TEST_P(ViewportTest, VaryingInY) {
    TestViewportQuad(0, 0, kWidth, kHeight - 1);
    TestViewportQuad(0, 1, kWidth, kHeight - 1);
    TestViewportQuad(0, 2, kWidth, 1);
}

// Test various viewport values in both X and Y
TEST_P(ViewportTest, SubBoxes) {
    TestViewportQuad(1, 1, kWidth - 2, kHeight - 2);
    TestViewportQuad(2, 2, 2, 2);
    TestViewportQuad(2, 3, 2, 1);
}

// Test that by default the [0, 1] depth range is used.
TEST_P(ViewportTest, DefaultViewportDepth) {
    TestViewportDepth(0.0, 1.0, false);
}

// Test various viewport depth ranges
TEST_P(ViewportTest, ViewportDepth) {
    TestViewportDepth(0.0, 0.5);
    TestViewportDepth(0.5, 1.0);
}

// Test that a draw with an empty viewport doesn't draw anything.
TEST_P(ViewportTest, EmptyViewport) {
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pipelineDescriptor.vertex.module = mQuadVS;
    pipelineDescriptor.cFragment.module = mQuadFS;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);

    auto DoEmptyViewportTest = [&](uint32_t width, uint32_t height) {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(pipeline);
        pass.SetViewport(0.0f, 0.0f, width, height, 0.0f, 1.0f);
        pass.Draw(6);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kZero, renderPass.color, 0, 0);
    };

    // Test with a 0x0, 0xN and nx0 viewport.
    DoEmptyViewportTest(0, 0);
    DoEmptyViewportTest(0, 1);
    DoEmptyViewportTest(1, 0);
}

DAWN_INSTANTIATE_TEST(ViewportTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
