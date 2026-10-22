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

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/40285824): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#include <algorithm>
#include <array>
#include <ostream>
#include <vector>

#include "src/dawn/tests/perf_tests/DawnPerfTest.h"
#include "src/dawn/utils/ComboRenderPipelineDescriptor.h"
#include "src/dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr unsigned int kNumIterations = 20;

enum class LoadStoreOp {
    ClearAndDiscard,  // LoadOp=Clear + StoreOp=Discard (MSAA attachments only)
    ClearAndStore,    // LoadOp=Clear + StoreOp=Store
    LoadAndStore,     // LoadOp=Load  + StoreOp=Store
};

std::ostream& operator<<(std::ostream& o, LoadStoreOp op) {
    switch (op) {
        case LoadStoreOp::ClearAndDiscard:
            return o << "ClearAndDiscard";
        case LoadStoreOp::ClearAndStore:
            return o << "ClearAndStore";
        case LoadStoreOp::LoadAndStore:
            return o << "LoadAndStore";
    }
}

DAWN_TEST_PARAM_STRUCT(LoadStoreOpParams, LoadStoreOp);

// Perf test comparing load+store configurations on MSAA attachments:
//   - LoadOp=Clear + StoreOp=Discard
//   - LoadOp=Clear + StoreOp=Store
//   - LoadOp=Load  + StoreOp=Store
// Each step renders multiple passes across several large (4096x4096) MSAA + resolve texture pairs
// to measure how the choice of load/store ops affects GPU throughput in a realistic multi-pass
// scenario. We use multiple textures (kNumTextures) to avoid the driver potentially caching results
// and effectively skipping work when the same operation is performed repeatedly.
class LoadStoreOpPerfTest : public DawnPerfTestWithParams<LoadStoreOpParams> {
  public:
    LoadStoreOpPerfTest() : DawnPerfTestWithParams<LoadStoreOpParams>(kNumIterations, 1) {}
    ~LoadStoreOpPerfTest() override = default;

  private:
    void SetUpPerfTest() override;
    void Step() override;

    wgpu::Texture CreateTexture(wgpu::TextureFormat format, uint32_t sampleCount) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = kWidth;
        descriptor.size.height = kHeight;
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.sampleCount = sampleCount;
        descriptor.format = format;
        descriptor.mipLevelCount = 1;

        descriptor.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

        return device.CreateTexture(&descriptor);
    }

    static constexpr uint32_t kWidth = 4096;
    static constexpr uint32_t kHeight = 4096;
    static constexpr uint32_t kNumTextures = 5;

    std::array<wgpu::Texture, kNumTextures> msaaTexture;
    std::array<wgpu::TextureView, kNumTextures> msaaTextureView;
    std::array<wgpu::Texture, kNumTextures> resolveTexture;
    std::array<wgpu::TextureView, kNumTextures> resolveTextureView;

    std::array<wgpu::Texture, kNumTextures> srcTexture1;
    std::array<wgpu::Texture, kNumTextures> srcTexture2;
    std::array<wgpu::TextureView, kNumTextures> srcTextureView1;
    std::array<wgpu::TextureView, kNumTextures> srcTextureView2;

    wgpu::RenderPipeline msaaPipeline;
    wgpu::RenderPipeline singleSampledPipeline;
    std::array<wgpu::BindGroup, kNumTextures> blitBindGroup1;
    std::array<wgpu::BindGroup, kNumTextures> blitBindGroup2;
};

void LoadStoreOpPerfTest::SetUpPerfTest() {
    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device, {
                    {0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float},
                });

    wgpu::PipelineLayout pipelineLayout = utils::MakePipelineLayout(device, {bgl});

    const char* vs = R"(
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            const pos = array(
                vec2f(-1.0, -1.0),
                vec2f( 3.0, -1.0),
                vec2f(-1.0,  3.0));
            return vec4f(pos[VertexIndex], 0.0, 1.0);
        })";
    constexpr char fs[] = R"(
        @group(0) @binding(0) var colorMap: texture_2d<f32>;
        @fragment
        fn main(@builtin(position) fragPosition : vec4<f32>) -> @location(0) vec4<f32> {
            let coords = vec2<i32>(i32(fragPosition.x), i32(fragPosition.y));
            return textureLoad(colorMap, coords, 0);
        })";
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;

    pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, vs);
    pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, fs);

    pipelineDescriptor.cFragment.targetCount = 1;
    pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::All;
    pipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

    pipelineDescriptor.layout = pipelineLayout;

    pipelineDescriptor.multisample.mask = 0xFFFFFFFF;
    pipelineDescriptor.multisample.count = 4;
    msaaPipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    pipelineDescriptor.multisample.count = 1;
    singleSampledPipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    for (uint32_t i = 0; i < kNumTextures; ++i) {
        msaaTexture[i] = CreateTexture(wgpu::TextureFormat::RGBA8Unorm, 4);
        resolveTexture[i] = CreateTexture(wgpu::TextureFormat::RGBA8Unorm, 1);
        srcTexture1[i] = CreateTexture(wgpu::TextureFormat::RGBA8Unorm, 1);
        srcTexture2[i] = CreateTexture(wgpu::TextureFormat::RGBA8Unorm, 1);

        msaaTextureView[i] = msaaTexture[i].CreateView();
        resolveTextureView[i] = resolveTexture[i].CreateView();
        srcTextureView1[i] = srcTexture1[i].CreateView();
        srcTextureView2[i] = srcTexture2[i].CreateView();

        blitBindGroup1[i] = utils::MakeBindGroup(device, bgl, {{0, srcTextureView1[i]}});
        blitBindGroup2[i] = utils::MakeBindGroup(device, bgl, {{0, srcTextureView2[i]}});

        // Clear the textures
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        float colorScale = std::max(0.1f, static_cast<float>(i) / kNumTextures);
        {
            utils::ComboRenderPassDescriptor renderPass({msaaTextureView[i]});
            renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
            renderPass.cColorAttachments[0].clearValue = {0.0f, colorScale, 0.0f, 0.0f};

            wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPass);
            renderPassEncoder.End();
        }
        {
            utils::ComboRenderPassDescriptor renderPass({srcTextureView1[i]});
            renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
            renderPass.cColorAttachments[0].clearValue = {colorScale, 0.0f, colorScale, 0.0f};

            wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPass);
            renderPassEncoder.End();
        }
        {
            utils::ComboRenderPassDescriptor renderPass({srcTextureView2[i]});
            renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
            renderPass.cColorAttachments[0].clearValue = {0.0f, colorScale, colorScale, 0.0f};

            wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPass);
            renderPassEncoder.End();
        }
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }
}

void LoadStoreOpPerfTest::Step() {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    if (SupportsTimestampQuery()) {
        RecordBeginTimestamp(encoder);
    }

    for (unsigned int iteration = 0; iteration < kNumIterations; ++iteration) {
        for (uint32_t i = 0; i < kNumTextures; ++i) {
            const bool isClear = GetParam().mLoadStoreOp != LoadStoreOp::LoadAndStore;
            const bool isDiscard = GetParam().mLoadStoreOp == LoadStoreOp::ClearAndDiscard;

            // We perform two passes to mimic typical Skia multi-pass use cases where a resolve
            // texture is used both as a resolve target in an MSAA pass and directly as a
            // render attachment in a single-sampled pass. This structure also prevents the
            // driver from merging multiple identical render passes into one.

            // 1st pass: blit the src texture 1 to the MSAA texture.
            {
                utils::ComboRenderPassDescriptor renderPass({msaaTextureView[i]});
                renderPass.cColorAttachments[0].resolveTarget = resolveTextureView[i];
                renderPass.cColorAttachments[0].loadOp =
                    isClear ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
                renderPass.cColorAttachments[0].storeOp =
                    isDiscard ? wgpu::StoreOp::Discard : wgpu::StoreOp::Store;
                renderPass.cColorAttachments[0].clearValue = {0.0f, 0.0f, 0.0f, 0.0f};

                wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPass);
                renderPassEncoder.SetBindGroup(0, blitBindGroup1[i]);
                renderPassEncoder.SetPipeline(msaaPipeline);
                renderPassEncoder.Draw(3);
                renderPassEncoder.End();
            }
            // 2nd pass: blit the src texture 2 to the resolve texture.
            // This is a non-MSAA attachment so we always use StoreOp=Store — discarding it
            // would throw away the resolve output that subsequent passes depend on.
            {
                utils::ComboRenderPassDescriptor renderPass({resolveTextureView[i]});
                renderPass.cColorAttachments[0].loadOp =
                    isClear ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
                renderPass.cColorAttachments[0].clearValue = {0.0f, 0.0f, 0.0f, 0.0f};

                wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPass);
                renderPassEncoder.SetBindGroup(0, blitBindGroup2[i]);
                renderPassEncoder.SetPipeline(singleSampledPipeline);
                renderPassEncoder.Draw(3);
                renderPassEncoder.End();
            }
        }
    }

    if (SupportsTimestampQuery()) {
        RecordEndTimestampAndResolveQuerySet(encoder);
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    if (SupportsTimestampQuery()) {
        ComputeGPUElapsedTime();
    }
}

TEST_P(LoadStoreOpPerfTest, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(LoadStoreOpPerfTest,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         VulkanBackend()},
                        {LoadStoreOp::ClearAndDiscard, LoadStoreOp::ClearAndStore,
                         LoadStoreOp::LoadAndStore});

using StoreOp = wgpu::StoreOp;
DAWN_TEST_PARAM_STRUCT(StoreOpParams, StoreOp);

// Measures the cost of rendering with a depth attachment using loadOp=Clear and configurable
// storeOp (Discard vs Store).
class StoreOpDepthPerfTest : public DawnPerfTestWithParams<StoreOpParams> {
  public:
    StoreOpDepthPerfTest() : DawnPerfTestWithParams(kNumIterations, 1) {}
    ~StoreOpDepthPerfTest() override = default;

  private:
    void SetUpPerfTest() override;
    void Step() override;

    wgpu::Texture CreateColorTexture() {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = kWidth;
        descriptor.size.height = kHeight;
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.sampleCount = 1;
        descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;
        return device.CreateTexture(&descriptor);
    }

    wgpu::Texture CreateDepthTexture() {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = kWidth;
        descriptor.size.height = kHeight;
        descriptor.size.depthOrArrayLayers = 1;
        descriptor.sampleCount = 1;
        descriptor.format = wgpu::TextureFormat::Depth24Plus;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;
        return device.CreateTexture(&descriptor);
    }

    static constexpr uint32_t kWidth = 4096;
    static constexpr uint32_t kHeight = 4096;
    static constexpr uint32_t kNumTextures = 5;

    std::array<wgpu::Texture, kNumTextures> colorTexture;
    std::array<wgpu::TextureView, kNumTextures> colorTextureView;
    std::array<wgpu::Texture, kNumTextures> depthTexture;
    std::array<wgpu::TextureView, kNumTextures> depthTextureView;

    wgpu::RenderPipeline pipeline;
};

void StoreOpDepthPerfTest::SetUpPerfTest() {
    const char* vs = R"(
        @vertex
        fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
            const pos = array(
                vec2f(-1.0, -1.0),
                vec2f( 3.0, -1.0),
                vec2f(-1.0,  3.0));
            return vec4f(pos[VertexIndex], 0.0, 1.0);
        })";
    const char* fs = R"(
        @fragment
        fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })";

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, vs);
    pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, fs);
    pipelineDescriptor.cFragment.targetCount = 1;
    pipelineDescriptor.cTargets[0].writeMask = wgpu::ColorWriteMask::All;
    pipelineDescriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
    pipelineDescriptor.EnableDepthStencil(wgpu::TextureFormat::Depth24Plus);
    pipelineDescriptor.cDepthStencil.depthWriteEnabled = wgpu::OptionalBool::True;
    pipelineDescriptor.cDepthStencil.depthCompare = wgpu::CompareFunction::Less;
    pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

    for (uint32_t i = 0; i < kNumTextures; ++i) {
        colorTexture[i] = CreateColorTexture();
        depthTexture[i] = CreateDepthTexture();
        colorTextureView[i] = colorTexture[i].CreateView();
        depthTextureView[i] = depthTexture[i].CreateView();
    }
}

void StoreOpDepthPerfTest::Step() {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    if (SupportsTimestampQuery()) {
        RecordBeginTimestamp(encoder);
    }

    for (unsigned int iteration = 0; iteration < kNumIterations; ++iteration) {
        for (uint32_t i = 0; i < kNumTextures; ++i) {
            utils::ComboRenderPassDescriptor renderPass({colorTextureView[i]}, depthTextureView[i]);
            renderPass.UnsetDepthStencilLoadStoreOpsForFormat(wgpu::TextureFormat::Depth24Plus);

            // Color attachment
            renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
            renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
            renderPass.cColorAttachments[0].clearValue = {0.0f, 0.0f, 0.0f, 0.0f};

            // Depth attachment: loadOp=Clear, storeOp=configurable
            renderPass.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
            renderPass.cDepthStencilAttachmentInfo.depthStoreOp = GetParam().mStoreOp;
            renderPass.cDepthStencilAttachmentInfo.depthClearValue = 1.0f;

            wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPass);
            renderPassEncoder.SetPipeline(pipeline);
            renderPassEncoder.Draw(3);
            renderPassEncoder.End();
        }
    }

    if (SupportsTimestampQuery()) {
        RecordEndTimestampAndResolveQuerySet(encoder);
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    if (SupportsTimestampQuery()) {
        ComputeGPUElapsedTime();
    }
}

TEST_P(StoreOpDepthPerfTest, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(StoreOpDepthPerfTest,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         VulkanBackend()},
                        {wgpu::StoreOp::Discard, wgpu::StoreOp::Store});

}  // anonymous namespace
}  // namespace dawn
