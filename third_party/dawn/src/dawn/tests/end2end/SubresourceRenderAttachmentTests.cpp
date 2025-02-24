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

#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

// Test that rendering to a subresource of a texture works.
class SubresourceRenderAttachmentTest : public DawnTest {
    constexpr static uint32_t kRTSize = 2;

  protected:
    enum class Type { Color, Depth, Stencil };

    void DoSingleTest(Type type,
                      wgpu::TextureFormat format,
                      wgpu::Texture renderTarget,
                      uint32_t textureSize,
                      uint32_t baseArrayLayer,
                      uint32_t baseMipLevel) {
        wgpu::TextureViewDescriptor renderTargetViewDesc;
        renderTargetViewDesc.baseArrayLayer = baseArrayLayer;
        renderTargetViewDesc.arrayLayerCount = 1;
        renderTargetViewDesc.baseMipLevel = baseMipLevel;
        renderTargetViewDesc.mipLevelCount = 1;
        wgpu::TextureView renderTargetView = renderTarget.CreateView(&renderTargetViewDesc);

        utils::RGBA8 expectedColor(0, 255, 0, 255);
        float expectedDepth = 0.3f;
        uint8_t expectedStencil = 7;

        utils::ComboRenderPassDescriptor renderPass = [&] {
            switch (type) {
                case Type::Color: {
                    utils::ComboRenderPassDescriptor renderPass({renderTargetView});
                    renderPass.cColorAttachments[0].clearValue = {
                        static_cast<float>(expectedColor.r) / 255.f,
                        static_cast<float>(expectedColor.g) / 255.f,
                        static_cast<float>(expectedColor.b) / 255.f,
                        static_cast<float>(expectedColor.a) / 255.f,
                    };
                    return renderPass;
                }
                case Type::Depth: {
                    utils::ComboRenderPassDescriptor renderPass({}, renderTargetView);
                    renderPass.UnsetDepthStencilLoadStoreOpsForFormat(format);
                    renderPass.cDepthStencilAttachmentInfo.depthClearValue = expectedDepth;
                    return renderPass;
                }
                case Type::Stencil: {
                    utils::ComboRenderPassDescriptor renderPass({}, renderTargetView);
                    renderPass.UnsetDepthStencilLoadStoreOpsForFormat(format);
                    renderPass.cDepthStencilAttachmentInfo.stencilClearValue = expectedStencil;
                    return renderPass;
                }
                default:
                    DAWN_UNREACHABLE();
            }
        }();

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder passEncoder = commandEncoder.BeginRenderPass(&renderPass);
        passEncoder.End();
        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        const uint32_t renderTargetSize = textureSize >> baseMipLevel;
        switch (type) {
            case Type::Color: {
                std::vector<utils::RGBA8> expected(renderTargetSize * renderTargetSize,
                                                   expectedColor);
                EXPECT_TEXTURE_EQ(expected.data(), renderTarget, {0, 0, baseArrayLayer},
                                  {renderTargetSize, renderTargetSize}, baseMipLevel);
                break;
            }
            case Type::Depth: {
                std::vector<float> expected(renderTargetSize * renderTargetSize, expectedDepth);
                EXPECT_TEXTURE_EQ(expected.data(), renderTarget, {0, 0, baseArrayLayer},
                                  {renderTargetSize, renderTargetSize}, baseMipLevel);
                break;
            }
            case Type::Stencil: {
                std::vector<uint8_t> expected(renderTargetSize * renderTargetSize, expectedStencil);
                EXPECT_TEXTURE_EQ(expected.data(), renderTarget, {0, 0, baseArrayLayer},
                                  {renderTargetSize, renderTargetSize}, baseMipLevel,
                                  wgpu::TextureAspect::StencilOnly);
                break;
            }
        }
    }

    void DoTest(Type type) {
        constexpr uint32_t kArrayLayerCount = 5;
        constexpr uint32_t kMipLevelCount = 4;

        wgpu::TextureFormat format;
        switch (type) {
            case Type::Color:
                format = wgpu::TextureFormat::RGBA8Unorm;
                break;
            case Type::Depth:
                format = wgpu::TextureFormat::Depth32Float;
                break;
            case Type::Stencil:
                format = wgpu::TextureFormat::Depth24PlusStencil8;
                break;
            default:
                DAWN_UNREACHABLE();
        }

        constexpr uint32_t kTextureSize = kRTSize << (kMipLevelCount - 1);

        wgpu::TextureDescriptor renderTargetDesc;
        renderTargetDesc.dimension = wgpu::TextureDimension::e2D;
        renderTargetDesc.size.width = kTextureSize;
        renderTargetDesc.size.height = kTextureSize;
        renderTargetDesc.size.depthOrArrayLayers = kArrayLayerCount;
        renderTargetDesc.sampleCount = 1;
        renderTargetDesc.format = format;
        renderTargetDesc.mipLevelCount = kMipLevelCount;
        renderTargetDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;

        wgpu::Texture renderTarget = device.CreateTexture(&renderTargetDesc);

        // Test rendering into the first, middle, and last of each of array layer and mip level.
        for (uint32_t arrayLayer : {0u, kArrayLayerCount / 2, kArrayLayerCount - 1u}) {
            for (uint32_t mipLevel : {0u, kMipLevelCount / 2, kMipLevelCount - 1u}) {
                DoSingleTest(type, format, renderTarget, kTextureSize, arrayLayer, mipLevel);
            }
        }
    }
};

// Test rendering into a subresource of a color texture
TEST_P(SubresourceRenderAttachmentTest, ColorTexture) {
    DoTest(Type::Color);
}

// Test rendering into a subresource of a depth texture
TEST_P(SubresourceRenderAttachmentTest, DepthTexture) {
    DoTest(Type::Depth);
}

// Test rendering into a subresource of a stencil texture
TEST_P(SubresourceRenderAttachmentTest, StencilTexture) {
    // TODO(crbug.com/352360580): Investigate failures.
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsARM());
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsANGLED3D11());
    DoTest(Type::Stencil);
}

DAWN_INSTANTIATE_TEST(SubresourceRenderAttachmentTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_render_pass"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
