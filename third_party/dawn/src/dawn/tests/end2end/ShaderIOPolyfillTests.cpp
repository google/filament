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

#include <utility>

#include "src/dawn/tests/DawnTest.h"
#include "src/dawn/utils/ComboRenderPipelineDescriptor.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using ShaderIOPolyfillTests = DawnTest;

// https://crbug.com/517522769
TEST_P(ShaderIOPolyfillTests, DivergentLocations) {
    // TODO(crbug.com/523272954): Produces incorrect result on Pixel 10.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsImgTec() && IsVulkan());

    wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
        struct VOut {
            @builtin(position) pos : vec4f,
            @location(0) a : vec4f,
            @location(1) @interpolate(flat) b : i32,
        };
        @vertex fn vs(@builtin(vertex_index) vi : u32) -> VOut {
            var p = array<vec2f, 3>(
                vec2f(-1, -1),
                vec2f( 3, -1),
                vec2f(-1,  3));
            var o : VOut;
            o.pos = vec4f(p[vi], 0, 1);
            o.a   = vec4f(1, 0, 0, 1);
            o.b   = 0x7eadbeef;
            return o;
        }
    )");

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
        @fragment fn fs(
            @location(0) a : vec4f,
            @builtin(sample_index) s : u32,
            @builtin(position) p : vec4f) -> @location(0) vec4f {
            return a + vec4f(p.z, f32(s) * 0, 0, 0);
        }
    )");

    utils::ComboRenderPipelineDescriptor pipelineDesc;
    pipelineDesc.vertex.module = std::move(vsModule);
    pipelineDesc.cFragment.module = std::move(fsModule);
    pipelineDesc.multisample.count = 4;
    pipelineDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDesc);

    wgpu::TextureDescriptor msaaDesc;
    msaaDesc.size = {64, 64, 1};
    msaaDesc.sampleCount = 4;
    msaaDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    msaaDesc.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture msaa = device.CreateTexture(&msaaDesc);

    wgpu::TextureDescriptor resolveDesc;
    resolveDesc.size = {64, 64, 1};
    resolveDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    resolveDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    wgpu::Texture resolve = device.CreateTexture(&resolveDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    utils::ComboRenderPassDescriptor renderPass({msaa.CreateView()});
    renderPass.cColorAttachments[0].resolveTarget = resolve.CreateView();
    renderPass.cColorAttachments[0].clearValue = {0.0, 0.0, 0.0, 0.0};
    renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
    renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, resolve, 32, 32);
}

DAWN_INSTANTIATE_TEST(ShaderIOPolyfillTests, VulkanBackend());

}  // namespace
}  // namespace dawn
