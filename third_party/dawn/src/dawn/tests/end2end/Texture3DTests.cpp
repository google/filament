// Copyright 2021 The Dawn & Tint Authors
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

#include <algorithm>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr static uint32_t kRTSize = 4;
constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;

class Texture3DTests : public DawnTest {};

TEST_P(Texture3DTests, Sampling) {
    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, kRTSize, kRTSize);

    // Set up pipeline. Two triangles will be drawn via the pipeline. They will fill the entire
    // color attachment with data sampled from 3D texture.
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0, 1.0),
                vec2f( -1.0, -1.0),
                vec2f(1.0, 1.0),
                vec2f(1.0, 1.0),
                vec2f(-1.0, -1.0),
                vec2f(1.0, -1.0));

            return vec4f(pos[VertexIndex], 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var samp : sampler;
        @group(0) @binding(1) var tex : texture_3d<f32>;

        @fragment
        fn main(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
            return textureSample(tex, samp, vec3f(FragCoord.xy / 4.0, 1.5 / 4.0));
        })");

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    pipelineDescriptor.cTargets[0].format = renderPass.colorFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::Extent3D copySize = {kRTSize, kRTSize, kRTSize};

    // Create a 3D texture, fill the texture via a B2T copy with well-designed data.
    // The 3D texture will be used as the data source of a sampler in shader.
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e3D;
    descriptor.size = copySize;
    descriptor.format = kFormat;
    descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
    wgpu::Texture texture = device.CreateTexture(&descriptor);
    wgpu::TextureView textureView = texture.CreateView();

    uint32_t bytesPerRow = utils::GetMinimumBytesPerRow(kFormat, copySize.width);
    uint32_t sizeInBytes =
        utils::RequiredBytesInCopy(bytesPerRow, copySize.height, copySize, kFormat);
    const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(kFormat);
    uint32_t size = sizeInBytes / bytesPerTexel;
    std::vector<utils::RGBA8> data = std::vector<utils::RGBA8>(size);
    for (uint32_t z = 0; z < copySize.depthOrArrayLayers; ++z) {
        for (uint32_t y = 0; y < copySize.height; ++y) {
            for (uint32_t x = 0; x < copySize.width; ++x) {
                uint32_t i = (z * copySize.height + y) * bytesPerRow / bytesPerTexel + x;
                data[i] = utils::RGBA8(x, y, z, 255);
            }
        }
    }
    wgpu::Buffer buffer =
        utils::CreateBufferFromData(device, data.data(), sizeInBytes, wgpu::BufferUsage::CopySrc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
        utils::CreateTexelCopyBufferInfo(buffer, 0, bytesPerRow, copySize.height);
    wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
        utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
    encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &copySize);

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, sampler}, {1, textureView}});

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(6);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // We sample data from the 3D texture at depth slice 1: 1.5 / 4.0 for z axis in textureSampler()
    // in shader, so the expected color at coordinate(x, y) should be (x, y, 1, 255).
    for (uint32_t i = 0; i < kRTSize; ++i) {
        for (uint32_t j = 0; j < kRTSize; ++j) {
            EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(i, j, 1, 255), renderPass.color, i, j);
        }
    }
}

// Regression test for crbug.com/dawn/2072 where the WSize of D3D UAV descriptor ends up being 0.
// (which is invalid as noted by the debug layers)
TEST_P(Texture3DTests, LatestMipClampsDepthSizeForStorageTextures) {
    wgpu::TextureDescriptor tDesc;
    tDesc.dimension = wgpu::TextureDimension::e3D;
    tDesc.size = {2, 2, 1};
    tDesc.mipLevelCount = 2;
    tDesc.usage = wgpu::TextureUsage::StorageBinding;
    tDesc.format = wgpu::TextureFormat::R32Uint;
    wgpu::Texture t = device.CreateTexture(&tDesc);

    wgpu::TextureViewDescriptor vDesc;
    vDesc.baseMipLevel = 1;
    vDesc.mipLevelCount = 1;
    wgpu::TextureView v = t.CreateView(&vDesc);

    wgpu::ComputePipelineDescriptor pDesc;
    pDesc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var t : texture_storage_3d<r32uint, write>;
        @compute @workgroup_size(1) fn main() {
            _ = t;
        }
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pDesc);

    wgpu::BindGroup bg = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, v}});

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetBindGroup(0, bg);
    pass.SetPipeline(pipeline);
    pass.DispatchWorkgroups(1);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

// Test 3d texture slices used as render attachments.
TEST_P(Texture3DTests, Rendering) {
    // TODO(crbug.com/dawn/2275): D3D12 debug layer reports the same subresource of 3d texture
    // cannot be written at the same time, which is a bug of D3D12 debug layer.
    // Remove this suppression once the issue is fixed.
    DAWN_SUPPRESS_TEST_IF(IsD3D12() && IsBackendValidationEnabled());

    // Set up pipeline. Bottom-left triangle will be drawn via the pipeline.
    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            var pos = array(
                vec2f(-1.0,  1.0),
                vec2f( 1.0, -1.0),
                vec2f(-1.0, -1.0));

            return vec4f(pos[VertexIndex], 0.0, 1.0);
        })");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        struct Output {
            @location(0) color0 : vec4f,
            @location(1) color1 : vec4f,
        }

        @fragment
        fn main() -> Output {
            var output : Output;
            output.color0 = vec4f(0.0, 1.0, 0.0, 1.0);
            output.color1 = vec4f(0.0, 1.0, 0.0, 1.0);
            return output;
        })");

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = fsModule;
    pipelineDescriptor.cTargets[0].format = kFormat;
    pipelineDescriptor.cTargets[1].format = kFormat;
    pipelineDescriptor.cFragment.targetCount = 2;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    // Create a 3D texture and 3D texture view which will be used as a render attachment.
    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.dimension = wgpu::TextureDimension::e3D;
    textureDescriptor.size = {kRTSize, kRTSize, kRTSize};
    textureDescriptor.mipLevelCount = 2;
    textureDescriptor.format = kFormat;
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture renderTarget = device.CreateTexture(&textureDescriptor);

    wgpu::TextureViewDescriptor viewDescriptor;
    viewDescriptor.dimension = wgpu::TextureViewDimension::e3D;
    viewDescriptor.baseMipLevel = 1;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    wgpu::TextureView view = renderTarget.CreateView(&viewDescriptor);

    // Clear and render to the depth slice index 0 and 1 of 3D texture at mip level 1
    utils::ComboRenderPassDescriptor renderPass({view, view});
    renderPass.cColorAttachments[0].depthSlice = 0;
    renderPass.cColorAttachments[0].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};
    renderPass.cColorAttachments[1].depthSlice = 1;
    renderPass.cColorAttachments[1].clearValue = {1.0f, 0.0f, 0.0f, 1.0f};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    uint32_t mipSize = std::max(kRTSize >> viewDescriptor.baseMipLevel, 1u);
    std::vector<utils::RGBA8> expected(mipSize * mipSize);
    // Only bottom-left triangle should be drawn with green color (0, 255, 0, 255), other pixels
    // stay red color (255, 0, 0, 255).
    for (uint32_t i = 0; i < mipSize; ++i) {
        for (uint32_t j = 0; j < mipSize; ++j) {
            expected[i * mipSize + j] =
                j < i ? utils::RGBA8(0, 255, 0, 255) : utils::RGBA8(255, 0, 0, 255);
        }
    }

    EXPECT_TEXTURE_EQ(expected.data(), renderTarget, {0, 0, 0}, {mipSize, mipSize, 1},
                      viewDescriptor.baseMipLevel);
    EXPECT_TEXTURE_EQ(expected.data(), renderTarget, {0, 0, 1}, {mipSize, mipSize, 1},
                      viewDescriptor.baseMipLevel);
}

DAWN_INSTANTIATE_TEST(Texture3DTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
