// Copyright 2024 The Dawn & Tint Authors
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

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using ClipDistancesSize = int32_t;
DAWN_TEST_PARAM_STRUCT(ClipDistancesTestParams, ClipDistancesSize);

class ClipDistancesTest : public DawnTestWithParams<ClipDistancesTestParams> {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::ClipDistances})) {
            requiredFeatures.push_back(wgpu::FeatureName::ClipDistances);
        }
        return requiredFeatures;
    }
};

// Verify that the feature "clip-distances" works correctly.
TEST_P(ClipDistancesTest, UseClipDistances) {
    DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::ClipDistances));

    // TODO(chromium:358408571): Investigate why the tests fail on Vulkan Android Pixel 4 bot
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsAndroid() && IsQualcomm());

    // Draw a square with two triangles (top-right and bottom left), whose vertices have different
    // clip distances values. (Top Left: -1, Bottom Right: 1 Top Right & Bottom Left: 0)
    // 1. The clip distances values of the pixels in the top-left region are less than 0
    // 2. The clip distances values of the pixels on the diagonal line are equal to 0.
    // 3. The clip distances values of the pixels in the bottom-right region are greater than 0
    std::ostringstream sstream;
    sstream << R"(
      enable clip_distances;
      const kClipDistancesSize = )"
            << GetParam().mClipDistancesSize << ";" << R"(
      struct VertexOutputs {
          @builtin(position) position : vec4f,
          @builtin(clip_distances) clipDistances : array<f32, kClipDistancesSize>,
      }
      @vertex
      fn vsMain(@builtin(vertex_index) vertexIndex : u32) -> VertexOutputs {
            var posAndClipDistances = array(
                vec3f(-1.0,  1.0, -1.0),
                vec3f( 1.0, -1.0,  1.0),
                vec3f( 1.0,  1.0,  0.0),
                vec3f(-1.0, -1.0,  0.0),
                vec3f( 1.0, -1.0,  1.0),
                vec3f(-1.0,  1.0, -1.0));
            var vertexOutput : VertexOutputs;
            vertexOutput.position = vec4f(posAndClipDistances[vertexIndex].xy, 0.0, 1.0);
            vertexOutput.clipDistances[kClipDistancesSize - 1] = posAndClipDistances[vertexIndex].z;
            return vertexOutput;
      }
      @fragment
      fn fsMain() -> @location(0) vec4f {
          return vec4f(1.0, 0.0, 0.0, 1.0);
      })";

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, sstream.str());
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = shaderModule;
    pipelineDescriptor.cFragment.module = shaderModule;
    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    constexpr uint32_t kSize = 5u;
    utils::BasicRenderPass renderPassDescriptor =
        utils::CreateBasicRenderPass(device, kSize, kSize);
    renderPassDescriptor.renderPassInfo.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    renderPassDescriptor.renderPassInfo.cColorAttachments[0].clearValue = {0.0, 1.0, 0.0, 1.0};

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder renderPassEncoder =
        commandEncoder.BeginRenderPass(&renderPassDescriptor.renderPassInfo);
    renderPassEncoder.SetPipeline(renderPipeline);
    renderPassEncoder.Draw(6);
    renderPassEncoder.End();
    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    // Only the pixels on the bottom-right triangle and the diagonal line should be drawn.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPassDescriptor.color, kSize - 1, kSize - 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPassDescriptor.color, kSize - 1, 0);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPassDescriptor.color, kSize / 2, kSize / 2);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderPassDescriptor.color, 0, kSize - 1);

    // The top-left triangle shouldn't be drawn.
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPassDescriptor.color, (kSize / 2) - 1,
                          (kSize / 2) - 1);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderPassDescriptor.color, 0, 0);
}

DAWN_INSTANTIATE_TEST_P(ClipDistancesTest,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), VulkanBackend(),
                         OpenGLBackend(), OpenGLESBackend()},
                        {1, 2, 3, 4, 5, 6, 7, 8});

}  // anonymous namespace
}  // namespace dawn
