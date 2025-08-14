// Copyright 2025 The Dawn & Tint Authors
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

#include <array>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

// Define all component swizzle options to test.
constexpr std::array<wgpu::ComponentSwizzle, 6> kComponentSwizzles = {
    wgpu::ComponentSwizzle::Zero, wgpu::ComponentSwizzle::One, wgpu::ComponentSwizzle::R,
    wgpu::ComponentSwizzle::G,    wgpu::ComponentSwizzle::B,   wgpu::ComponentSwizzle::A};

class TextureComponentSwizzleTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::TextureComponentSwizzle));

        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var texture : texture_2d<f32>;

            @vertex fn vs_main(@builtin(vertex_index) vertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array<vec2f, 3>(
                    vec2f(-1.0, -1.0),
                    vec2f(-1.0, 3.0),
                    vec2f(3.0, -1.0)
                );
                return vec4f(pos[vertexIndex], 0.0, 1.0);
            }

            @fragment fn fs_main() -> @location(0) vec4f {
                // textureLoad samples at an integer coordinate and mip level 0.
                return textureLoad(texture, vec2i(0, 0), 0);
            })");

        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = module;
        pipelineDesc.cFragment.module = module;
        pipeline = device.CreateRenderPipeline(&pipelineDesc);

        wgpu::TextureDescriptor outputTextureDesc = {};
        outputTextureDesc.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        outputTextureDesc.size = {1, 1, 1};
        outputTextureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        outputTexture = device.CreateTexture(&outputTextureDesc);
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::TextureComponentSwizzle})) {
            requiredFeatures.push_back(wgpu::FeatureName::TextureComponentSwizzle);
        }
        return requiredFeatures;
    }

    struct TestParams {
        wgpu::TextureFormat format;
        // Input data for the source texture. Max 4 bytes for convenience.
        std::array<uint8_t, 4> inputData;
        // The expected values (R, G, B, A) that `textureLoad` would produce
        // for this format *before* any swizzling. This accounts for default values
        // for missing channels (e.g., G=0, B=0, A=255 for R8Unorm).
        std::array<uint8_t, 4> baseLoadValues;
    };

    // Calculates the expected value after swizzling and normalization.
    uint8_t GetExpectedValue(wgpu::ComponentSwizzle swizzle,
                             const std::array<uint8_t, 4>& rgbaData) {
        switch (swizzle) {
            case wgpu::ComponentSwizzle::Zero:
                return 0;
            case wgpu::ComponentSwizzle::One:
                return 255;
            case wgpu::ComponentSwizzle::R:
                return rgbaData[0];
            case wgpu::ComponentSwizzle::G:
                return rgbaData[1];
            case wgpu::ComponentSwizzle::B:
                return rgbaData[2];
            case wgpu::ComponentSwizzle::A:
                return rgbaData[3];
            case wgpu::ComponentSwizzle::Undefined:
            default:
                DAWN_UNREACHABLE();
                return 0;
        }
    }

    void RunSwizzleTest(TestParams params,
                        wgpu::Texture inputTexture,
                        wgpu::ComponentSwizzle swizzleRed,
                        wgpu::ComponentSwizzle swizzleGreen,
                        wgpu::ComponentSwizzle swizzleBlue,
                        wgpu::ComponentSwizzle swizzleAlpha) {
        wgpu::TexelCopyTextureInfo dest = {.texture = inputTexture};
        wgpu::TexelCopyBufferLayout dataLayout = {.bytesPerRow = 256, .rowsPerImage = 1};
        wgpu::Extent3D writeSize = {1, 1, 1};
        const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(params.format);
        device.GetQueue().WriteTexture(&dest, params.inputData.data(), bytesPerTexel, &dataLayout,
                                       &writeSize);

        // Create the TextureView for the input texture with the specified swizzle.
        wgpu::TextureViewDescriptor viewDesc = {};
        wgpu::TextureComponentSwizzleDescriptor swizzleDesc = {};
        swizzleDesc.swizzle.r = swizzleRed;
        swizzleDesc.swizzle.g = swizzleGreen;
        swizzleDesc.swizzle.b = swizzleBlue;
        swizzleDesc.swizzle.a = swizzleAlpha;
        viewDesc.nextInChain = &swizzleDesc;
        wgpu::TextureView textureView = inputTexture.CreateView(&viewDesc);

        // Set up bind group using the swizzled texture view.
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, textureView}});

        // Issue render commands.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassDescriptor renderPassDesc = {};
        wgpu::RenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = outputTexture.CreateView();
        colorAttachment.loadOp = wgpu::LoadOp::Clear;
        colorAttachment.storeOp = wgpu::StoreOp::Store;
        colorAttachment.clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDesc);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3);
        pass.End();

        // Submit commands to the queue.
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        utils::RGBA8 expectedColor =
            utils::RGBA8(GetExpectedValue(swizzleRed, params.baseLoadValues),
                         GetExpectedValue(swizzleGreen, params.baseLoadValues),
                         GetExpectedValue(swizzleBlue, params.baseLoadValues),
                         GetExpectedValue(swizzleAlpha, params.baseLoadValues));
        EXPECT_PIXEL_RGBA8_EQ(expectedColor, outputTexture, 0, 0);
    }

    void RunTest(TestParams params) {
        wgpu::TextureDescriptor textureDesc = {};
        textureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
        textureDesc.size = {1, 1, 1};
        textureDesc.format = params.format;
        wgpu::Texture inputTexture = device.CreateTexture(&textureDesc);

        for (auto swizzleRed : kComponentSwizzles) {
            RunSwizzleTest(params, inputTexture, swizzleRed, wgpu::ComponentSwizzle::G,
                           wgpu::ComponentSwizzle::B, wgpu::ComponentSwizzle::A);
        }

        for (auto swizzleGreen : kComponentSwizzles) {
            RunSwizzleTest(params, inputTexture, wgpu::ComponentSwizzle::R, swizzleGreen,
                           wgpu::ComponentSwizzle::B, wgpu::ComponentSwizzle::A);
        }

        for (auto swizzleBlue : kComponentSwizzles) {
            RunSwizzleTest(params, inputTexture, wgpu::ComponentSwizzle::R,
                           wgpu::ComponentSwizzle::G, swizzleBlue, wgpu::ComponentSwizzle::A);
        }

        for (auto swizzleAlpha : kComponentSwizzles) {
            RunSwizzleTest(params, inputTexture, wgpu::ComponentSwizzle::R,
                           wgpu::ComponentSwizzle::G, wgpu::ComponentSwizzle::B, swizzleAlpha);
        }
    }

    wgpu::RenderPipeline pipeline;
    wgpu::Texture outputTexture;
};

// Test that texture component swizzle works as expected when the 'texture-component-swizzle'
// feature is enabled. These tests verify each component's swizzle mapping independently
// by using a render shader to read from a swizzled texture and render to a texture.
// Those covers formats with all channels, and formats where some channels are implicitly
// generated (0.0 or 1.0) by `textureLoad` due to the base format missing components.
TEST_P(TextureComponentSwizzleTest, AllChannelsPresent) {
    TestParams params;
    params.format = wgpu::TextureFormat::RGBA8Unorm;
    params.inputData = {255, 128, 64, 0};
    params.baseLoadValues = {255, 128, 64, 0};
    RunTest(params);
}
TEST_P(TextureComponentSwizzleTest, OnlyRedChannelPresent) {
    TestParams params;
    params.format = wgpu::TextureFormat::R8Unorm;
    params.inputData = {128, 0, 0, 0};
    params.baseLoadValues = {128, 0, 0, 255};  // G, B default to 0, A defaults to 255.
    RunTest(params);
}
TEST_P(TextureComponentSwizzleTest, OnlyRedAndGreenChannelPresent) {
    TestParams params;
    params.format = wgpu::TextureFormat::RG8Unorm;
    params.inputData = {128, 64, 0, 0};
    params.baseLoadValues = {128, 64, 0, 255};  // B defaults to 0, A defaults to 255.
    RunTest(params);
}

// Test that custom swizzle correctly reorders texture components and that a subsequent use of the
// default swizzle correctly reverts to the standard component mapping.
TEST_P(TextureComponentSwizzleTest, UseDefaultSwizzleAfterNonDefaultSwizzle) {
    TestParams params;
    params.format = wgpu::TextureFormat::RGBA8Unorm;
    params.inputData = {255, 128, 64, 0};
    params.baseLoadValues = {255, 128, 64, 0};

    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    textureDesc.size = {1, 1, 1};
    textureDesc.format = params.format;
    wgpu::Texture inputTexture = device.CreateTexture(&textureDesc);

    // First, use a non-default swizzle.
    RunSwizzleTest(params, inputTexture, wgpu::ComponentSwizzle::One, wgpu::ComponentSwizzle::One,
                   wgpu::ComponentSwizzle::One, wgpu::ComponentSwizzle::One);

    // Then, use a default swizzle.
    RunSwizzleTest(params, inputTexture, wgpu::ComponentSwizzle::R, wgpu::ComponentSwizzle::G,
                   wgpu::ComponentSwizzle::B, wgpu::ComponentSwizzle::A);
}

DAWN_INSTANTIATE_TEST(TextureComponentSwizzleTest,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
