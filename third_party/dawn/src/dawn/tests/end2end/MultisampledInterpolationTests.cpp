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

#include <vector>

#include "dawn/common/GPUInfo.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class MultisampledInterpolationTest : public DawnTest {
  protected:
    static constexpr wgpu::TextureFormat kColorFormat =
        wgpu::TextureFormat::RGBA8Unorm;  // rgba8unorm
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
                    struct VertexOut {
                        @location(0) @interpolate(linear, center) interpolatedValue: vec4f,
                        @location(1) @interpolate(linear, center) depthw: vec4f,
                        @builtin(position) position: vec4f,
                    };

                    @vertex fn main(@builtin(vertex_index) vNdx: u32) -> VertexOut {
                        let pos = array(
                                  vec4f(0.333, 0.333, 0.333, 0.333), vec4f(1, -3, 0.25, 1), vec4f(-1.5, 0.5, 0.25, 0.5)
                        );
                        let interStage = array(
                         vec4f(1.0, 0.0, 0.0, 0.0), vec4f(0.0, 1.0 , 0.0, 0.0), vec4f(0.0, 0.0, 1.0, 0.0)
                        );
                        var v: VertexOut;
                        v.position = pos[vNdx];
                        v.depthw.x = v.position.z/v.position.w;
                        v.depthw.y = 1/ v.position.w;
                        v.interpolatedValue = interStage[vNdx];
                        return v;
                    }
                    )");

            desc.cFragment.module = utils::CreateShaderModule(device, R"(
                    struct FragmentIn {
                        @builtin(sample_index) sample_index: u32,
                        @location(0) @interpolate(linear, center) interpolatedValue: vec4f,
                        @location(1) @interpolate(linear, center) depthw: vec4f,
                        @builtin(position) position: vec4f,
                    };

                    struct FragOut {
                        @location(0) out0: vec4f,
                        @location(1) out1: vec4f,
                        @location(2) out2: vec4f,
                        @location(3) out3: vec4f,
                    };

                    fn u32ToRGBAUnorm(u: u32) -> vec4f {
                        return vec4f(
                        f32((u >> 24) & 0xFF) / 255.0,
                        f32((u >> 16) & 0xFF) / 255.0,
                        f32((u >>  8) & 0xFF) / 255.0,
                        f32((u >>  0) & 0xFF) / 255.0,
                        );
                    }

                    @fragment fn fs(fin: FragmentIn) -> FragOut {
                        var f: FragOut;
                        var v = fin.position;
                       // Use of sample_index forces sample behavior
                        if(fin.sample_index == 3u){
                          v = vec4f(1,1,1,1);
                        }
                        let u = bitcast<vec4u>(v);
                        f.out0 = u32ToRGBAUnorm(u[0]);
                        f.out1 = u32ToRGBAUnorm(u[1]);
                        f.out2 = u32ToRGBAUnorm(u[2]);
                        f.out3 = u32ToRGBAUnorm(u[3]);
                        _ = fin.interpolatedValue;
                        return f;
                    }
                )");

            desc.multisample.count = kSampleCount;
            desc.cFragment.targetCount = 4;
            desc.cTargets[0].format = kColorFormat;

            desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
            desc.primitive.cullMode = wgpu::CullMode::None;

            drawPipeline = device.CreateRenderPipeline(&desc);
        }
        {
            wgpu::ComputePipelineDescriptor desc = {};
            desc.compute.module = utils::CreateShaderModule(device, R"(
                    @group(0) @binding(0) var texture0: texture_multisampled_2d<f32>;
                    @group(0) @binding(1) var texture1: texture_multisampled_2d<f32>;
                    @group(0) @binding(2) var texture2: texture_multisampled_2d<f32>;
                    @group(0) @binding(3) var texture3: texture_multisampled_2d<f32>;
                    @group(0) @binding(4) var<storage, read_write> buffer: array<f32>;

                    @compute @workgroup_size(1) fn main(@builtin(global_invocation_id) id: vec3u) {
                    let numSamples = textureNumSamples(texture0);
                    let dimensions = textureDimensions(texture0);
                    let sampleIndex = id.x % numSamples;
                    let tx = id.x / numSamples;
                    let offset = ((id.y * dimensions.x + tx) * numSamples + sampleIndex) * 4;
                    let r = vec4u(textureLoad(texture0, vec2u(tx, id.y), sampleIndex) * 255.0);
                    let g = vec4u(textureLoad(texture1, vec2u(tx, id.y), sampleIndex) * 255.0);
                    let b = vec4u(textureLoad(texture2, vec2u(tx, id.y), sampleIndex) * 255.0);
                    let a = vec4u(textureLoad(texture3, vec2u(tx, id.y), sampleIndex) * 255.0);

                    // expand rgba8unorm values back to their byte form, add them together
                    // and cast them to an f32 so we can recover the f32 values we encoded
                    // in the rgba8unorm texture.
                    buffer[offset + 0] = bitcast<f32>(dot(r, vec4u(0x1000000, 0x10000, 0x100, 0x1)));
                    buffer[offset + 1] = bitcast<f32>(dot(g, vec4u(0x1000000, 0x10000, 0x100, 0x1)));
                    buffer[offset + 2] = bitcast<f32>(dot(b, vec4u(0x1000000, 0x10000, 0x100, 0x1)));
                    buffer[offset + 3] = bitcast<f32>(dot(a, vec4u(0x1000000, 0x10000, 0x100, 0x1)));
                    }
                )");

            checkSamplePipeline = device.CreateComputePipeline(&desc);
        }
    }
};

// Test that the multisampling sample positions are correct by drawing a single triangle to a 10x10
// multisample buffer. For this test we only check the single first pixel as that is all that is
// required to establish sampling behavior of the position builtin.
// Pixel centers vs sample location.
// https://github.com/gpuweb/gpuweb/issues/108
// Vulkan, Metal, and D3D11 have the same standard multisample pattern for 4x MSAA. D3D12 is the
// same as D3D11 but it was left out of the documentation. The sample locations are:
// {0.375, 0.125}, {0.875, 0.375}, {0.125 0.625}, {0.625, 0.875}

TEST_P(MultisampledInterpolationTest, SamplePositions) {
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // TODO(crbug.com/40238674): Fails on Pixel 10 gles.
    DAWN_SUPPRESS_TEST_IF(IsImgTec() && IsVulkan());

    // Swiftshader does not work. Unknown reason.
    DAWN_TEST_UNSUPPORTED_IF(IsSwiftshader());
    DAWN_TEST_UNSUPPORTED_IF(IsANGLESwiftShader());

    CreatePipelines();

    static constexpr wgpu::Extent3D kTextureSize = {10, 10, 1};

    wgpu::Texture colorTextures[4];
    {
        wgpu::TextureDescriptor desc = {};
        desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
        desc.size = kTextureSize;
        desc.format = kColorFormat;
        desc.sampleCount = kSampleCount;
        for (auto& each : colorTextures) {
            each = device.CreateTexture(&desc);
        }
    }

    wgpu::TextureView colorViews[4];
    for (int i = 0; i < 4; i++) {
        auto& each = colorViews[i];
        each = colorTextures[i].CreateView();
    }
    static constexpr uint64_t kResultSize = 4 * sizeof(float) + 4 * sizeof(float);
    uint64_t alignedResultSize = Align(kResultSize, 256);

    wgpu::BufferDescriptor outputBufferDesc = {};
    outputBufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    outputBufferDesc.size = alignedResultSize * 8;
    wgpu::Buffer outputBuffer = device.CreateBuffer(&outputBufferDesc);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    utils::ComboRenderPassDescriptor renderPass(
        {colorViews[0], colorViews[1], colorViews[2], colorViews[3]});

    wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);

    renderPassEncoder.SetBindGroup(
        0, utils::MakeBindGroup(device, drawPipeline.GetBindGroupLayout(0), {}));
    renderPassEncoder.SetViewport(0.0, 0, kTextureSize.width, kTextureSize.height, 0.1, 0.9);
    renderPassEncoder.SetPipeline(drawPipeline);
    renderPassEncoder.Draw(3);
    renderPassEncoder.End();

    wgpu::ComputePassEncoder computePassEncoder = commandEncoder.BeginComputePass();
    computePassEncoder.SetPipeline(checkSamplePipeline);
    computePassEncoder.SetBindGroup(
        0, utils::MakeBindGroup(device, checkSamplePipeline.GetBindGroupLayout(0),
                                {{0, colorViews[0]},
                                 {1, colorViews[1]},
                                 {2, colorViews[2]},
                                 {3, colorViews[3]},
                                 {4, outputBuffer}}));
    computePassEncoder.DispatchWorkgroups(4);
    computePassEncoder.End();

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    std::array<float, 16> expectedData;

    expectedData = {
        0.5,
        0.5,
        0.69499993324279785,
        2.47650146484375,
        0.5,
        0.5,
        0.69499993324279785,
        2.47650146484375,
        0.5,
        0.5,
        0.69499993324279785,
        2.47650146484375,
        1.0,
        1.0,
        1.0,
        1.0,
    };
    EXPECT_BUFFER_FLOAT_RANGE_TOLERANCE_EQ(expectedData.data(), outputBuffer, 0 * alignedResultSize,
                                           16, 0.0002f)
        << " Single first pixel sample";
}

DAWN_INSTANTIATE_TEST(MultisampledInterpolationTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

}  // anonymous namespace
}  // namespace dawn
