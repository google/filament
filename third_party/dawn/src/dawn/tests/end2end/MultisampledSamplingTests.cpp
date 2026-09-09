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

#include <cstddef>
#include <vector>

#include "src/dawn/common/Math.h"
#include "src/dawn/common/SystemUtils.h"
#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/ComboRenderPipelineDescriptor.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

// https://github.com/gpuweb/gpuweb/issues/108
// Vulkan, Metal, and D3D11 have the same standard multisample pattern. D3D12 is the same as
// D3D11 but it was left out of the documentation.
// {0.375, 0.125}, {0.875, 0.375}, {0.125 0.625}, {0.625, 0.875}
// In this test, we store them in -1 to 1 space because it makes it
// simpler to upload vertex data. Y is flipped because there is a flip between clip space and
// rasterization space.
static constexpr std::array<std::array<float, 2>, 4> kSamplePositions = {
    {{0.375 * 2 - 1, 1 - 0.125 * 2},
     {0.875 * 2 - 1, 1 - 0.375 * 2},
     {0.125 * 2 - 1, 1 - 0.625 * 2},
     {0.625 * 2 - 1, 1 - 0.875 * 2}}};

class MultisampledSamplingTest : public DawnTest {
  protected:
    static constexpr wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::R8Unorm;
    static constexpr wgpu::TextureFormat kDepthFormat = wgpu::TextureFormat::Depth32Float;

    static constexpr uint32_t kSampleCount = 4;

    // Render pipeline for drawing to a multisampled color and depth attachment.
    wgpu::RenderPipeline drawPipeline;

    // A compute pipeline to texelFetch the sample locations and output the results to a buffer.
    wgpu::ComputePipeline checkSamplePipeline;

    void SetUp() override { DawnTest::SetUp(); }

    void CreatePipelines() {
        {
            utils::ComboRenderPipelineDescriptor desc;

            desc.vertex.module = utils::CreateShaderModule(device, R"(
                @vertex
                fn main(@location(0) pos : vec2f) -> @builtin(position) vec4f {
                    return vec4f(pos, 0.0, 1.0);
                })");

            desc.cFragment.module = utils::CreateShaderModule(device, R"(
                struct FragmentOut {
                    @location(0) color : f32,
                    @builtin(frag_depth) depth : f32,
                }

                @fragment fn main() -> FragmentOut {
                    var output : FragmentOut;
                    output.color = 1.0;
                    output.depth = 0.7;
                    return output;
                })");

            desc.primitive.stripIndexFormat = wgpu::IndexFormat::Uint32;
            desc.vertex.bufferCount = 1;
            desc.cBuffers[0].attributeCount = 1;
            desc.cBuffers[0].arrayStride = 2 * sizeof(float);
            desc.cAttributes[0].format = wgpu::VertexFormat::Float32x2;

            wgpu::DepthStencilState* depthStencil = desc.EnableDepthStencil(kDepthFormat);
            depthStencil->depthWriteEnabled = wgpu::OptionalBool::True;

            desc.multisample.count = kSampleCount;
            desc.cFragment.targetCount = 1;
            desc.cTargets[0].format = kColorFormat;

            desc.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;

            drawPipeline = device.CreateRenderPipeline(&desc);
        }
        {
            wgpu::ComputePipelineDescriptor desc = {};
            desc.compute.module = utils::CreateShaderModule(device, R"(
                @group(0) @binding(0) var texture0 : texture_multisampled_2d<f32>;
                @group(0) @binding(1) var texture1 : texture_depth_multisampled_2d;

                struct Results {
                    colorSamples : array<f32, 4>,
                    depthSamples : array<f32, 4>,
                }
                @group(0) @binding(2) var<storage, read_write> results : Results;

                @compute @workgroup_size(1) fn main() {
                    for (var i : i32 = 0; i < 4; i = i + 1) {
                        results.colorSamples[i] = textureLoad(texture0, vec2i(0, 0), i).x;
                        results.depthSamples[i] = textureLoad(texture1, vec2i(0, 0), i);
                    }
                })");

            checkSamplePipeline = device.CreateComputePipeline(&desc);
        }
    }
};

// Test that the multisampling sample positions are correct. This test works by drawing a
// thin quad multiple times from left to right and from top to bottom on a 1x1 canvas.
// Each time, the quad should cover a single sample position.
// After drawing, a compute shader fetches all of the samples (both color and depth),
// and we check that only the one covered has data.
// We "scan" the vertical and horizontal dimensions separately to check that the triangle
// must cover both the X and Y coordinates of the sample position (no false positives if
// it covers the X position but not the Y, or vice versa).
TEST_P(MultisampledSamplingTest, SamplePositions) {
    // TODO(42242119): fail on Qualcomm Adreno X1.
    DAWN_SUPPRESS_TEST_IF(IsD3D11() && IsQualcomm());

    // textureLoad with texture_depth_xxx is not supported in compat mode.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    CreatePipelines();

    static constexpr wgpu::Extent3D kTextureSize = {1, 1, 1};

    wgpu::Texture colorTexture;
    {
        wgpu::TextureDescriptor desc = {};
        desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
        desc.size = kTextureSize;
        desc.format = kColorFormat;
        desc.sampleCount = kSampleCount;
        colorTexture = device.CreateTexture(&desc);
    }

    wgpu::Texture depthTexture;
    {
        wgpu::TextureDescriptor desc = {};
        desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
        desc.size = kTextureSize;
        desc.format = kDepthFormat;
        desc.sampleCount = kSampleCount;
        depthTexture = device.CreateTexture(&desc);
    }

    static constexpr float kQuadWidth = 0.075;
    std::vector<float> vBufferData;

    // Add vertices for vertical quads
    for (uint32_t s = 0; s < kSampleCount; ++s) {
        // clang-format off
        vBufferData.insert(vBufferData.end(), {
            kSamplePositions[s][0] - kQuadWidth, -1.0,
            kSamplePositions[s][0] - kQuadWidth,  1.0,
            kSamplePositions[s][0] + kQuadWidth, -1.0,
            kSamplePositions[s][0] + kQuadWidth,  1.0,
        });
        // clang-format on
    }

    // Add vertices for horizontal quads
    for (uint32_t s = 0; s < kSampleCount; ++s) {
        // clang-format off
        vBufferData.insert(vBufferData.end(), {
            -1.0, kSamplePositions[s][1] - kQuadWidth,
            -1.0, kSamplePositions[s][1] + kQuadWidth,
             1.0, kSamplePositions[s][1] - kQuadWidth,
             1.0, kSamplePositions[s][1] + kQuadWidth,
        });
        // clang-format on
    }

    wgpu::Buffer vBuffer = utils::CreateBufferFromData(
        device, vBufferData.data(), static_cast<uint32_t>(vBufferData.size() * sizeof(float)),
        wgpu::BufferUsage::Vertex);

    static constexpr uint32_t kQuadNumBytes = 8 * sizeof(float);

    wgpu::TextureView colorView = colorTexture.CreateView();
    wgpu::TextureView depthView = depthTexture.CreateView();

    static constexpr uint64_t kResultSize = 4 * sizeof(float) + 4 * sizeof(float);
    uint64_t alignedResultSize = Align(kResultSize, 256);

    wgpu::BufferDescriptor outputBufferDesc = {};
    outputBufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    outputBufferDesc.size = alignedResultSize * 8;
    wgpu::Buffer outputBuffer = device.CreateBuffer(&outputBufferDesc);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    for (uint32_t iter = 0; iter < 2; ++iter) {
        for (uint32_t sample = 0; sample < kSampleCount; ++sample) {
            uint32_t sampleOffset = (iter * kSampleCount + sample);

            utils::ComboRenderPassDescriptor renderPass({colorView}, depthView);
            renderPass.cDepthStencilAttachmentInfo.depthClearValue = 0.f;
            renderPass.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
            renderPass.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;

            wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
            renderPassEncoder.SetPipeline(drawPipeline);
            renderPassEncoder.SetVertexBuffer(
                0, vBuffer, static_cast<uint64_t>(kQuadNumBytes) * sampleOffset, kQuadNumBytes);
            renderPassEncoder.Draw(4);
            renderPassEncoder.End();

            wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
            computePassEncoder.SetPipeline(checkSamplePipeline);
            computePassEncoder.SetBindGroup(
                0, utils::MakeBindGroup(
                       device, checkSamplePipeline.GetBindGroupLayout(0),
                       {{0, colorView},
                        {1, depthView},
                        {2, outputBuffer, alignedResultSize * sampleOffset, kResultSize}}));
            computePassEncoder.DispatchWorkgroups(1);
            computePassEncoder.End();
        }
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    std::array<float, 8> expectedData;

    expectedData = {1, 0, 0, 0, 0.7, 0, 0, 0};
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedData.data(), outputBuffer, 0 * alignedResultSize, 8)
        << "vertical sample 0";

    expectedData = {0, 1, 0, 0, 0, 0.7, 0, 0};
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedData.data(), outputBuffer, 1 * alignedResultSize, 8)
        << "vertical sample 1";

    expectedData = {0, 0, 1, 0, 0, 0, 0.7, 0};
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedData.data(), outputBuffer, 2 * alignedResultSize, 8)
        << "vertical sample 2";

    expectedData = {0, 0, 0, 1, 0, 0, 0, 0.7};
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedData.data(), outputBuffer, 3 * alignedResultSize, 8)
        << "vertical sample 3";

    expectedData = {1, 0, 0, 0, 0.7, 0, 0, 0};
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedData.data(), outputBuffer, 4 * alignedResultSize, 8)
        << "horizontal sample 0";

    expectedData = {0, 1, 0, 0, 0, 0.7, 0, 0};
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedData.data(), outputBuffer, 5 * alignedResultSize, 8)
        << "horizontal sample 1";

    expectedData = {0, 0, 1, 0, 0, 0, 0.7, 0};
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedData.data(), outputBuffer, 6 * alignedResultSize, 8)
        << "horizontal sample 2";

    expectedData = {0, 0, 0, 1, 0, 0, 0, 0.7};
    EXPECT_BUFFER_FLOAT_RANGE_EQ(expectedData.data(), outputBuffer, 7 * alignedResultSize, 8)
        << "horizontal sample 3";
}

// Test for a bug whereby u32 division corrupts implicit texture LOD calculations with 4x MSAA on
// Apple Silicon due to our workaround that introduces volatile instructions.
// See https://crbug.com/533785363
TEST_P(MultisampledSamplingTest, ImplicitDerivativeFromU32Div) {
    // TODO(crbug.com/468061892): Fails on Windows 11/AMD RX 5500 XT w/ backend validation.
    DAWN_SUPPRESS_TEST_IF(IsWindows11() && IsAMD() && IsD3D12() && IsBackendValidationEnabled());

    // Texture 256x256 with 2 mip levels.
    wgpu::TextureDescriptor texDesc;
    texDesc.dimension = wgpu::TextureDimension::e2D;
    texDesc.size = {256, 256, 1};
    texDesc.mipLevelCount = 2;
    texDesc.sampleCount = 1;
    texDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    texDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    wgpu::Texture texture = device.CreateTexture(&texDesc);

    // Write white to mip 0 (256x256).
    {
        std::vector<utils::RGBA8> whiteData(static_cast<size_t>(256 * 256), utils::RGBA8::kWhite);
        wgpu::Buffer stagingBuffer = utils::CreateBufferFromData(
            device, whiteData.data(), whiteData.size() * sizeof(utils::RGBA8),
            wgpu::BufferUsage::CopySrc);
        wgpu::TexelCopyBufferInfo bufferInfo =
            utils::CreateTexelCopyBufferInfo(stagingBuffer, 0, 1024, 256);
        wgpu::TexelCopyTextureInfo textureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
        wgpu::Extent3D writeSize = {256, 256, 1};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&bufferInfo, &textureInfo, &writeSize);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Write black to mip 1 (128x128).
    {
        std::vector<utils::RGBA8> blackData(static_cast<size_t>(128 * 128), utils::RGBA8::kBlack);
        wgpu::Buffer stagingBuffer = utils::CreateBufferFromData(
            device, blackData.data(), blackData.size() * sizeof(utils::RGBA8),
            wgpu::BufferUsage::CopySrc);
        wgpu::TexelCopyBufferInfo bufferInfo =
            utils::CreateTexelCopyBufferInfo(stagingBuffer, 0, 512, 128);
        wgpu::TexelCopyTextureInfo textureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 1, {0, 0, 0});
        wgpu::Extent3D writeSize = {128, 128, 1};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&bufferInfo, &textureInfo, &writeSize);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.minFilter = wgpu::FilterMode::Linear;
    samplerDesc.magFilter = wgpu::FilterMode::Linear;
    samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
    wgpu::Sampler sampler = device.CreateSampler(&samplerDesc);

    // Create an 8x8 grid of quads that cover the screen with a small margin.
    std::vector<float> vertices;
    auto vertex = [&](int column, int row) {
        float u = static_cast<float>(column) / 8.0f;
        float v = static_cast<float>(row) / 8.0f;
        vertices.insert(vertices.end(), {u * 1.6f - 0.8f, v * 1.6f - 0.8f, 0.0f, u, 1.0f - v});
    };
    for (int row = 0; row < 8; row++) {
        for (int column = 0; column < 8; column++) {
            vertex(column, row);
            vertex(column + 1, row);
            vertex(column, row + 1);
            vertex(column, row + 1);
            vertex(column + 1, row);
            vertex(column + 1, row + 1);
        }
    }
    wgpu::Buffer vertexBuffer = utils::CreateBufferFromData(
        device, vertices.data(), vertices.size() * sizeof(float), wgpu::BufferUsage::Vertex);

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, R"(
        struct VertexOutput {
            @builtin(position) pos : vec4f,
            @location(0) uv : vec2f,
        }

        @vertex
        fn vs(@location(0) position: vec3f, @location(1) uv: vec2f) -> VertexOutput {
            var out: VertexOutput;
            out.pos = vec4f(position.xy, 0.0, 1.0);
            out.uv = uv;
            return out;
        }

        @group(0) @binding(0) var t: texture_2d<f32>;
        @group(0) @binding(1) var s: sampler;
        @group(0) @binding(2) var<uniform> materialIndex: u32;

        @fragment
        fn fs(input: VertexOutput) -> @location(0) vec4f {
            let offset = f32(materialIndex / 2u);
            let uv = input.uv + vec2f(offset);
            let texel = textureSample(t, s, uv);
            return vec4f(texel.rgb, 1.0);
        })");

    utils::ComboRenderPipelineDescriptor desc;
    desc.vertex.module = shaderModule;
    desc.cFragment.module = shaderModule;
    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    desc.primitive.cullMode = wgpu::CullMode::Back;
    desc.multisample.count = 4;
    desc.cFragment.targetCount = 1;
    desc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

    desc.vertex.bufferCount = 1;
    desc.cBuffers[0].attributeCount = 2;
    desc.cBuffers[0].arrayStride = uint64_t{5} * sizeof(float);
    desc.cAttributes[0].format = wgpu::VertexFormat::Float32x3;
    desc.cAttributes[0].offset = 0;
    desc.cAttributes[0].shaderLocation = 0;
    desc.cAttributes[1].format = wgpu::VertexFormat::Float32x2;
    desc.cAttributes[1].offset = uint64_t{3} * sizeof(float);
    desc.cAttributes[1].shaderLocation = 1;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&desc);

    // Canvas size 720x720.
    wgpu::TextureDescriptor msaaDesc;
    msaaDesc.dimension = wgpu::TextureDimension::e2D;
    msaaDesc.size = {720, 720, 1};
    msaaDesc.mipLevelCount = 1;
    msaaDesc.sampleCount = 4;
    msaaDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    msaaDesc.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture msaaTexture = device.CreateTexture(&msaaDesc);

    wgpu::TextureDescriptor resolveDesc;
    resolveDesc.dimension = wgpu::TextureDimension::e2D;
    resolveDesc.size = {720, 720, 1};
    resolveDesc.mipLevelCount = 1;
    resolveDesc.sampleCount = 1;
    resolveDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    resolveDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture resolveTexture = device.CreateTexture(&resolveDesc);

    // Uniform buffer for the material index.
    uint32_t materialIndex = 0;
    wgpu::Buffer uBuffer = utils::CreateBufferFromData(
        device, &materialIndex, sizeof(materialIndex), wgpu::BufferUsage::Uniform);
    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {
                                                         {0, texture.CreateView()},
                                                         {1, sampler},
                                                         {2, uBuffer, 0, sizeof(materialIndex)},
                                                     });

    utils::ComboRenderPassDescriptor renderPass({msaaTexture.CreateView()});
    renderPass.cColorAttachments[0].resolveTarget = resolveTexture.CreateView();
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    renderPass.cColorAttachments[0].clearValue = {0.5f, 0.5f, 0.5f, 1.0f};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.SetVertexBuffer(0, vertexBuffer);
    pass.Draw(static_cast<uint32_t>(vertices.size() / 5));
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Expect all pixels in the inner 500x500 square to be white.
    // The grid spans from x = 0.1*720 = 72 to x = 0.9*720 = 648, so
    // the range [100,100] -> [600,600] is strictly inside that grid.
    std::vector<utils::RGBA8> expectedData(static_cast<size_t>(500 * 500), utils::RGBA8::kWhite);
    EXPECT_TEXTURE_EQ(expectedData.data(), resolveTexture, {100, 100}, {500, 500});
}

DAWN_INSTANTIATE_TEST(MultisampledSamplingTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
