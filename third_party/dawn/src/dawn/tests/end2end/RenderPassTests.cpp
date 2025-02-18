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

#include <utility>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr uint32_t kRTSize = 16;
constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;

class RenderPassTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        // Shaders to draw a bottom-left triangle in blue.
        mVSModule = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0,  1.0),
                    vec2f( 1.0, -1.0),
                    vec2f(-1.0, -1.0));

                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() -> @location(0) vec4f {
                return vec4f(0.0, 0.0, 1.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = mVSModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        descriptor.cTargets[0].format = kFormat;

        pipeline = device.CreateRenderPipeline(&descriptor);
    }

    wgpu::Texture CreateDefault2DTexture() {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = kRTSize;
        descriptor.size.height = kRTSize;
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.sampleCount = 1;
        descriptor.format = kFormat;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        return device.CreateTexture(&descriptor);
    }

    wgpu::ShaderModule mVSModule;
    wgpu::RenderPipeline pipeline;
};

// Test using two different render passes in one commandBuffer works correctly.
TEST_P(RenderPassTest, TwoRenderPassesInOneCommandBuffer) {
    if (IsOpenGL() || IsMetal()) {
        // crbug.com/950768
        // This test is consistently failing on OpenGL and flaky on Metal.
        return;
    }

    wgpu::Texture renderTarget1 = CreateDefault2DTexture();
    wgpu::Texture renderTarget2 = CreateDefault2DTexture();
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    {
        // In the first render pass we clear renderTarget1 to red and draw a blue triangle in the
        // bottom left of renderTarget1.
        utils::ComboRenderPassDescriptor renderPass({renderTarget1.CreateView()});
        renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();
    }

    {
        // In the second render pass we clear renderTarget2 to green and draw a blue triangle in the
        // bottom left of renderTarget2.
        utils::ComboRenderPassDescriptor renderPass({renderTarget2.CreateView()});
        renderPass.cColorAttachments[0].clearValue = {0.0f, 1.0f, 0.0f, 1.0f};

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        pass.Draw(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, renderTarget1, 1, kRTSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderTarget1, kRTSize - 1, 1);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, renderTarget2, 1, kRTSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderTarget2, kRTSize - 1, 1);
}

// Verify that the content in the color attachment will not be changed if there is no corresponding
// fragment shader outputs in the render pipeline, the load operation is LoadOp::Load and the store
// operation is StoreOp::Store.
TEST_P(RenderPassTest, NoCorrespondingFragmentShaderOutputs) {
    wgpu::Texture renderTarget = CreateDefault2DTexture();
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::TextureView renderTargetView = renderTarget.CreateView();

    utils::ComboRenderPassDescriptor renderPass({renderTargetView});
    renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);

    {
        // First we draw a blue triangle in the bottom left of renderTarget.
        pass.SetPipeline(pipeline);
        pass.Draw(3);
    }

    {
        // Next we use a pipeline whose fragment shader has no outputs.
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @fragment fn main() {
            })");
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = mVSModule;
        descriptor.cFragment.module = fsModule;
        // (Off-topic) spot-test for defaulting of these three fields.
        descriptor.primitive.topology = wgpu::PrimitiveTopology::Undefined;
        descriptor.primitive.frontFace = wgpu::FrontFace::Undefined;
        descriptor.primitive.cullMode = wgpu::CullMode::Undefined;
        descriptor.cTargets[0].format = kFormat;
        descriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::None;

        wgpu::RenderPipeline pipelineWithNoFragmentOutput =
            device.CreateRenderPipeline(&descriptor);

        pass.SetPipeline(pipelineWithNoFragmentOutput);
        pass.Draw(3);
    }

    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, renderTarget, 1, kRTSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderTarget, kRTSize - 1, 1);
}

DAWN_INSTANTIATE_TEST(RenderPassTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_render_pass"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

// Test that clearing the lower mips of an R8Unorm texture works. This is a regression test for
// dawn:1071 where Intel Metal devices fail to do that correctly, requiring a workaround.
class RenderPassTest_RegressionDawn1071 : public RenderPassTest {};
TEST_P(RenderPassTest_RegressionDawn1071, ClearLowestMipOfR8Unorm) {
    const uint32_t kLastMipLevel = 2;

    // Create the texture and buffer used for readback.
    wgpu::TextureDescriptor texDesc;
    texDesc.format = wgpu::TextureFormat::R8Unorm;
    texDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    texDesc.size = {32, 32};
    texDesc.mipLevelCount = kLastMipLevel + 1;
    wgpu::Texture tex = device.CreateTexture(&texDesc);

    wgpu::BufferDescriptor bufDesc;
    bufDesc.size = 4;
    bufDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buf = device.CreateBuffer(&bufDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    // Clear the texture with a render pass.
    {
        wgpu::TextureViewDescriptor viewDesc;
        viewDesc.baseMipLevel = kLastMipLevel;

        utils::ComboRenderPassDescriptor renderPass({tex.CreateView(&viewDesc)});
        renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
    }

    // Copy the texture in the buffer.
    {
        wgpu::Extent3D copySize = {1, 1};
        wgpu::TexelCopyTextureInfo src = utils::CreateTexelCopyTextureInfo(tex, kLastMipLevel);
        wgpu::TexelCopyBufferInfo dst = utils::CreateTexelCopyBufferInfo(buf);

        encoder.CopyTextureToBuffer(&src, &dst, &copySize);
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // The content of the texture should be reflected in the buffer (prior to the workaround it
    // would be 0s).
    EXPECT_BUFFER_U8_EQ(255, buf, 0);
}

DAWN_INSTANTIATE_TEST(RenderPassTest_RegressionDawn1071,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      MetalBackend({"metal_render_r8_rg8_unorm_small_mip_to_temp_texture"}),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

// Test that clearing a depth16unorm texture with multiple subresources works. This is a regression
// test for dawn:1389 where Intel Metal devices fail to do that correctly, requiring a workaround.
class RenderPassTest_RegressionDawn1389 : public RenderPassTest {};
TEST_P(RenderPassTest_RegressionDawn1389, ClearMultisubresourceAfterWriteDepth16Unorm) {
    // TODO(dawn:1705): fix this test for Intel D3D11.
    DAWN_SUPPRESS_TEST_IF((IsD3D11() || IsANGLED3D11()) && IsIntel());

    // TODO(crbug.com/dawn/1989): Failed on Intel Gen12 GPUs because of Windows Vulkan driver issue,
    // when copying to a D16_UNORM depth texture and clearing one subresource, other subresources
    // will have incorrect values. Remove this suppression once the issue is fixed.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsWindows() && IsIntelGen12());

    // Test all combinatons of multi-mip, multi-layer
    for (uint32_t mipLevelCount : {1, 5}) {
        for (uint32_t arrayLayerCount : {1, 7}) {
            // Only clear some of the subresources.
            const auto& clearedMips =
                mipLevelCount == 1 ? std::vector<std::pair<uint32_t, uint32_t>>{{0, 1}}
                                   : std::vector<std::pair<uint32_t, uint32_t>>{{0, 2}, {3, 4}};
            const auto& clearedLayers =
                arrayLayerCount == 1 ? std::vector<std::pair<uint32_t, uint32_t>>{{0, 1}}
                                     : std::vector<std::pair<uint32_t, uint32_t>>{{2, 4}, {6, 7}};

            // Compute the texture size.
            uint32_t width = 1u << (mipLevelCount - 1);
            uint32_t height = 1u << (mipLevelCount - 1);

            // Create the texture.
            wgpu::TextureDescriptor texDesc;
            texDesc.format = wgpu::TextureFormat::Depth16Unorm;
            texDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                            wgpu::TextureUsage::CopyDst;
            texDesc.size = {width, height, arrayLayerCount};
            texDesc.mipLevelCount = mipLevelCount;
            wgpu::Texture tex = device.CreateTexture(&texDesc);

            // Initialize all subresources with WriteTexture.
            for (uint32_t level = 0; level < mipLevelCount; ++level) {
                for (uint32_t layer = 0; layer < arrayLayerCount; ++layer) {
                    wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
                        utils::CreateTexelCopyTextureInfo(tex, level, {0, 0, layer});
                    wgpu::Extent3D copySize = {width >> level, height >> level, 1};

                    wgpu::TexelCopyBufferLayout texelCopyBufferLayout;
                    texelCopyBufferLayout.offset = 0;
                    texelCopyBufferLayout.bytesPerRow = copySize.width * sizeof(uint16_t);
                    texelCopyBufferLayout.rowsPerImage = copySize.height;

                    // Use a distinct value for each subresource.
                    uint16_t value = level * 10 + layer;
                    std::vector<uint16_t> data(copySize.width * copySize.height, value);
                    queue.WriteTexture(&texelCopyTextureInfo, data.data(),
                                       data.size() * sizeof(uint16_t), &texelCopyBufferLayout,
                                       &copySize);
                }
            }

            // Prep a viewDesc for rendering to depth. The base layer and level
            // will be set later.
            wgpu::TextureViewDescriptor viewDesc = {};
            viewDesc.mipLevelCount = 1u;
            viewDesc.arrayLayerCount = 1u;

            // Overwrite some subresources with a render pass
            {
                wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
                for (const auto& clearedMipRange : clearedMips) {
                    for (const auto& clearedLayerRange : clearedLayers) {
                        for (uint32_t level = clearedMipRange.first; level < clearedMipRange.second;
                             ++level) {
                            for (uint32_t layer = clearedLayerRange.first;
                                 layer < clearedLayerRange.second; ++layer) {
                                viewDesc.baseMipLevel = level;
                                viewDesc.baseArrayLayer = layer;

                                utils::ComboRenderPassDescriptor renderPass(
                                    {}, tex.CreateView(&viewDesc));
                                renderPass.UnsetDepthStencilLoadStoreOpsForFormat(texDesc.format);
                                renderPass.cDepthStencilAttachmentInfo.depthClearValue = 0.8;
                                renderPass.cDepthStencilAttachmentInfo.depthLoadOp =
                                    wgpu::LoadOp::Clear;
                                renderPass.cDepthStencilAttachmentInfo.depthStoreOp =
                                    wgpu::StoreOp::Store;
                                encoder.BeginRenderPass(&renderPass).End();
                            }
                        }
                    }
                }
                wgpu::CommandBuffer commands = encoder.Finish();
                queue.Submit(1, &commands);
            }

            // Iterate all subresources.
            for (uint32_t level = 0; level < mipLevelCount; ++level) {
                for (uint32_t layer = 0; layer < arrayLayerCount; ++layer) {
                    bool cleared = false;
                    for (const auto& clearedMipRange : clearedMips) {
                        for (const auto& clearedLayerRange : clearedLayers) {
                            if (level >= clearedMipRange.first && level < clearedMipRange.second &&
                                layer >= clearedLayerRange.first &&
                                layer < clearedLayerRange.second) {
                                cleared = true;
                            }
                        }
                    }
                    uint32_t mipWidth = width >> level;
                    uint32_t mipHeight = height >> level;
                    if (cleared) {
                        // Check the subresource is cleared as expected.
                        std::vector<uint16_t> data(mipWidth * mipHeight, 0xCCCC);
                        EXPECT_TEXTURE_EQ(data.data(), tex, {0, 0, layer}, {mipWidth, mipHeight},
                                          level)
                            << "cleared texture data should have been 0xCCCC at:" << "\nlayer: "
                            << layer << "\nlevel: " << level;
                    } else {
                        // Otherwise, check the other subresources have the orignal contents.
                        // Without the workaround, they are 0.
                        uint16_t value =
                            level * 10 + layer;  // Compute the expected value for the subresource.
                        std::vector<uint16_t> data(mipWidth * mipHeight, value);
                        EXPECT_TEXTURE_EQ(data.data(), tex, {0, 0, layer}, {mipWidth, mipHeight},
                                          level)
                            << "written texture data should still be " << value
                            << " at:" << "\nlayer: " << layer << "\nlevel: " << level;
                    }
                }
            }
        }
    }
}

DAWN_INSTANTIATE_TEST(RenderPassTest_RegressionDawn1389,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      MetalBackend({"use_blit_for_buffer_to_depth_texture_copy"}),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
