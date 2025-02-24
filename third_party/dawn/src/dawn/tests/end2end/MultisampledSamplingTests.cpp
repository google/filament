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

#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

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
            renderPassEncoder.SetVertexBuffer(0, vBuffer, kQuadNumBytes * sampleOffset,
                                              kQuadNumBytes);
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

DAWN_INSTANTIATE_TEST(MultisampledSamplingTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
