// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/utils/WGPUHelpers.h"

#include "dawn/tests/unittests/validation/ValidationTest.h"

namespace dawn {
namespace {

class TextureSubresourceTest : public ValidationTest {
  public:
    static constexpr uint32_t kSize = 32u;
    static constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::Texture CreateTexture(uint32_t mipLevelCount,
                                uint32_t arrayLayerCount,
                                wgpu::TextureUsage usage) {
        wgpu::TextureDescriptor texDesc;
        texDesc.dimension = wgpu::TextureDimension::e2D;
        texDesc.size = {kSize, kSize, arrayLayerCount};
        texDesc.sampleCount = 1;
        texDesc.mipLevelCount = mipLevelCount;
        texDesc.usage = usage;
        texDesc.format = kFormat;
        return device.CreateTexture(&texDesc);
    }

    wgpu::TextureView CreateTextureView(wgpu::Texture texture,
                                        uint32_t baseMipLevel,
                                        uint32_t baseArrayLayer) {
        wgpu::TextureViewDescriptor viewDesc;
        viewDesc.format = kFormat;
        viewDesc.baseArrayLayer = baseArrayLayer;
        viewDesc.arrayLayerCount = 1;
        viewDesc.baseMipLevel = baseMipLevel;
        viewDesc.mipLevelCount = 1;
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        return texture.CreateView(&viewDesc);
    }

    void TestRenderPass(const wgpu::TextureView& renderView, const wgpu::TextureView& samplerView) {
        // Create bind group
        wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
            device, {{0, wgpu::ShaderStage::Vertex, wgpu::TextureSampleType::Float}});

        utils::ComboRenderPassDescriptor renderPassDesc({renderView});

        // It is valid to read from and write into different subresources of the same texture
        {
            wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, samplerView}});
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
            pass.SetBindGroup(0, bindGroup);
            pass.End();
            encoder.Finish();
        }

        // It is not currently possible to test that it is valid to have multiple reads from a
        // subresource while there is a single write in another subresource.

        // It is invalid to read and write into the same subresources
        {
            wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, renderView}});
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
            pass.SetBindGroup(0, bindGroup);
            pass.End();
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }

        // It is valid to write into and then read from the same level of a texture in different
        // render passes
        {
            wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, bgl, {{0, samplerView}});

            wgpu::BindGroupLayout bgl1 = utils::MakeBindGroupLayout(
                device,
                {{0, wgpu::ShaderStage::Fragment, wgpu::StorageTextureAccess::WriteOnly, kFormat}});
            wgpu::BindGroup bindGroup1 = utils::MakeBindGroup(device, bgl1, {{0, samplerView}});

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&renderPassDesc);
            pass1.SetBindGroup(0, bindGroup1);
            pass1.End();

            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
            pass.SetBindGroup(0, bindGroup);
            pass.End();

            encoder.Finish();
        }
    }
};

// Test different mipmap levels
TEST_F(TextureSubresourceTest, MipmapLevelsTest) {
    // Create texture with 2 mipmap levels and 1 layer
    wgpu::Texture texture =
        CreateTexture(2, 1,
                      wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment |
                          wgpu::TextureUsage::StorageBinding);

    // Create two views on different mipmap levels.
    wgpu::TextureView samplerView = CreateTextureView(texture, 0, 0);
    wgpu::TextureView renderView = CreateTextureView(texture, 1, 0);
    TestRenderPass(samplerView, renderView);
}

// Test different array layers
TEST_F(TextureSubresourceTest, ArrayLayersTest) {
    // Create texture with 1 mipmap level and 2 layers
    wgpu::Texture texture =
        CreateTexture(1, 2,
                      wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment |
                          wgpu::TextureUsage::StorageBinding);

    // Create two views on different layers.
    wgpu::TextureView samplerView = CreateTextureView(texture, 0, 0);
    wgpu::TextureView renderView = CreateTextureView(texture, 0, 1);

    TestRenderPass(samplerView, renderView);
}

}  // anonymous namespace
}  // namespace dawn
