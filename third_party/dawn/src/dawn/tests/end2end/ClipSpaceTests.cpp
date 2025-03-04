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

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ClipSpaceTest : public DawnTest {
  protected:
    wgpu::RenderPipeline CreatePipelineForTest() {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;

        // Draw two triangles:
        // 1. The depth value of the top-left one is >= 0.5
        // 2. The depth value of the bottom-right one is <= 0.5
        pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec3f(-1.0,  1.0, 1.0),
                    vec3f(-1.0, -1.0, 0.5),
                    vec3f( 1.0,  1.0, 0.5),
                    vec3f( 1.0,  1.0, 0.5),
                    vec3f(-1.0, -1.0, 0.5),
                    vec3f( 1.0, -1.0, 0.0));
                return vec4f(pos[VertexIndex], 1.0);
            })");

        pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
               return vec4f(1.0, 0.0, 0.0, 1.0);
            })");

        wgpu::DepthStencilState* depthStencil = pipelineDescriptor.EnableDepthStencil();
        depthStencil->depthCompare = wgpu::CompareFunction::LessEqual;

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    wgpu::Texture Create2DTextureForTest(wgpu::TextureFormat format) {
        wgpu::TextureDescriptor textureDescriptor;
        textureDescriptor.dimension = wgpu::TextureDimension::e2D;
        textureDescriptor.format = format;
        textureDescriptor.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        textureDescriptor.mipLevelCount = 1;
        textureDescriptor.sampleCount = 1;
        textureDescriptor.size = {kSize, kSize, 1};
        return device.CreateTexture(&textureDescriptor);
    }

    static constexpr uint32_t kSize = 4;
};

// Test that the clip space is correctly configured.
TEST_P(ClipSpaceTest, ClipSpace) {
    wgpu::Texture colorTexture = Create2DTextureForTest(wgpu::TextureFormat::RGBA8Unorm);
    wgpu::Texture depthStencilTexture =
        Create2DTextureForTest(wgpu::TextureFormat::Depth24PlusStencil8);

    utils::ComboRenderPassDescriptor renderPassDescriptor({colorTexture.CreateView()},
                                                          depthStencilTexture.CreateView());
    renderPassDescriptor.cColorAttachments[0].clearValue = {0.0, 1.0, 0.0, 1.0};
    renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;

    // Clear the depth stencil attachment to 0.5f, so only the bottom-right triangle should be
    // drawn.
    renderPassDescriptor.cDepthStencilAttachmentInfo.depthClearValue = 0.5f;
    renderPassDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPass = commandEncoder.BeginRenderPass(&renderPassDescriptor);
    renderPass.SetPipeline(CreatePipelineForTest());
    renderPass.Draw(6);
    renderPass.End();
    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, colorTexture, kSize - 1, kSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, colorTexture, 0, 0);
}

DAWN_INSTANTIATE_TEST(ClipSpaceTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
