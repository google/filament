// Copyright 2026 The Dawn & Tint Authors
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

#include <sstream>
#include <vector>

#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/ComboRenderPipelineDescriptor.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

enum class TextureType {
    Texture2D,
    Texture2DArray,
};

std::ostream& operator<<(std::ostream& o, TextureType type) {
    switch (type) {
        case TextureType::Texture2D:
            o << "2D";
            break;
        case TextureType::Texture2DArray:
            o << "2DArray";
            break;
    }
    return o;
}

DAWN_TEST_PARAM_STRUCT(TextureSampleCompare2DTestParams, TextureType);

class TextureSampleCompare2DTest : public DawnTestWithParams<TextureSampleCompare2DTestParams> {
  protected:
    void SetUp() override {
        DawnTestWithParams<TextureSampleCompare2DTestParams>::SetUp();
        // TODO(crbug.com/523272961): Produces incorrect result on Pixel 10.
        DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsImgTec() && IsVulkan());
    }
};

// Test that textureSampleCompare with 2D polyfill returns the same result as the native
// implementation.
TEST_P(TextureSampleCompare2DTest, Compare2D) {
    // Only run on Vulkan for this specific polyfill test.
    DAWN_TEST_UNSUPPORTED_IF(!IsVulkan());

    const bool isArray = GetParam().mTextureType == TextureType::Texture2DArray;

    static constexpr uint32_t kSize = 4;
    wgpu::Extent3D textureSize = {kSize, kSize, isArray ? 2u : 1u};

    // Create a depth texture.
    wgpu::TextureDescriptor depthDesc;
    depthDesc.format = wgpu::TextureFormat::Depth32Float;
    depthDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
    depthDesc.size = textureSize;
    wgpu::Texture depthTexture = device.CreateTexture(&depthDesc);

    // Create a pipeline to initialize the depth texture.
    utils::ComboRenderPipelineDescriptor setupPipelineDesc;
    setupPipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0, -1.0),
                vec2f( 3.0, -1.0),
                vec2f(-1.0,  3.0));
            return vec4f(pos[VertexIndex], 0.0, 1.0);
        }
    )");
    setupPipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
        @fragment fn main(@builtin(position) pos : vec4f) -> @builtin(frag_depth) f32 {
            // Set only texel (1,1) to 1.0, others to 0.0.
            if (u32(pos.x) == 1u && u32(pos.y) == 1u) {
                return 1.0;
            }
            return 0.0;
        }
    )");
    setupPipelineDesc.EnableDepthStencil(wgpu::TextureFormat::Depth32Float);
    setupPipelineDesc.cDepthStencil.depthWriteEnabled = wgpu::OptionalBool::True;
    setupPipelineDesc.cFragment.targetCount = 0;
    wgpu::RenderPipeline setupPipeline = device.CreateRenderPipeline(&setupPipelineDesc);

    // Initialize the depth texture.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        for (uint32_t i = 0; i < textureSize.depthOrArrayLayers; ++i) {
            wgpu::TextureViewDescriptor viewDesc;
            viewDesc.dimension = wgpu::TextureViewDimension::e2D;
            viewDesc.baseArrayLayer = i;
            viewDesc.arrayLayerCount = 1;

            utils::ComboRenderPassDescriptor renderPass({}, depthTexture.CreateView(&viewDesc));
            renderPass.UnsetDepthStencilLoadStoreOpsForFormat(wgpu::TextureFormat::Depth32Float);
            renderPass.cDepthStencilAttachmentInfo.depthClearValue = 0.0f;
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetPipeline(setupPipeline);
            pass.Draw(3);
            pass.End();
        }
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Create a comparison sampler.
    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.minFilter = wgpu::FilterMode::Linear;
    samplerDesc.magFilter = wgpu::FilterMode::Linear;
    samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    samplerDesc.compare = wgpu::CompareFunction::Less;
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    // Create a render pipeline that outputs the result of textureSampleCompare.
    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
        @vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0, -1.0),
                vec2f( 3.0, -1.0),
                vec2f(-1.0,  3.0));
            return vec4f(pos[VertexIndex], 0.0, 1.0);
        }
    )");
    const char* textureType = isArray ? "texture_depth_2d_array" : "texture_depth_2d";
    const char* compareArgs = isArray ? "0u, 0.5" : "0.5";

    std::ostringstream shader;
    shader << "@group(0) @binding(0) var t : " << textureType << R"(;
@group(0) @binding(1) var s : sampler_comparison;

@fragment fn main() -> @location(0) f32 {
    // We are choosing a specific sample location to get a unique bilinear filtered result.
    // This should match the original and polyfilled for all devices that have correct implementation.
    return textureSampleCompare(t, s, vec2f(0.28, 0.27), )"
           << compareArgs << R"();
})";
    pipelineDesc.cFragment.module = utils::CreateShaderModule(device, shader.str().c_str());
    pipelineDesc.cTargets[0].format = wgpu::TextureFormat::R32Float;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    // Create an output texture to read the result.
    wgpu::TextureDescriptor outputDesc;
    outputDesc.format = wgpu::TextureFormat::R32Float;
    outputDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    outputDesc.size = {1, 1, 1};
    wgpu::Texture outputTexture = device.CreateTexture(&outputDesc);
    wgpu::TextureView outputView = outputTexture.CreateView();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor renderPass({outputView});
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                              {{0, depthTexture.CreateView()}, {1, sampler}}));
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
    // This test only checks the result implementation against a single 'interesting' point.
    // A more robust test is of course possible but this confirms the bilinear filtering (in x and
    // y).
    constexpr float expected2D = 0.3596;

    EXPECT_PIXEL_FLOAT_TOLERANCE_EQ(expected2D, outputTexture, 0, 0, 0.01f);
}

DAWN_INSTANTIATE_TEST_P(TextureSampleCompare2DTest,
                        {VulkanBackend(), VulkanBackend({"vulkan_sample_compare_2d_workaround"})},
                        {TextureType::Texture2D, TextureType::Texture2DArray});

}  // anonymous namespace
}  // namespace dawn
