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

#include "dawn/tests/perf_tests/DawnPerfTest.h"

#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

struct SubresourceTrackingParams : AdapterTestParam {
    SubresourceTrackingParams(const AdapterTestParam& param,
                              uint32_t arrayLayerCountIn,
                              uint32_t mipLevelCountIn)
        : AdapterTestParam(param),
          arrayLayerCount(arrayLayerCountIn),
          mipLevelCount(mipLevelCountIn) {}
    uint32_t arrayLayerCount;
    uint32_t mipLevelCount;
};

std::ostream& operator<<(std::ostream& ostream, const SubresourceTrackingParams& param) {
    ostream << static_cast<const AdapterTestParam&>(param);
    ostream << "_arrayLayer_" << param.arrayLayerCount;
    ostream << "_mipLevel_" << param.mipLevelCount;
    return ostream;
}

// Test the performance of Subresource usage and barrier tracking on a case that would generally be
// difficult. It uses a 2D array texture with mipmaps and updates one of the layers with data from
// another texture, then generates mipmaps for that layer. It is difficult because it requires
// tracking the state of individual subresources in the middle of the subresources of that texture.
class SubresourceTrackingPerf : public DawnPerfTestWithParams<SubresourceTrackingParams> {
  public:
    static constexpr unsigned int kNumIterations = 50;

    SubresourceTrackingPerf() : DawnPerfTestWithParams(kNumIterations, 1) {}
    ~SubresourceTrackingPerf() override = default;

    void SetUp() override {
        DawnPerfTestWithParams<SubresourceTrackingParams>::SetUp();
        const SubresourceTrackingParams& params = GetParam();

        wgpu::TextureDescriptor materialDesc;
        materialDesc.dimension = wgpu::TextureDimension::e2D;
        materialDesc.size = {1u << (params.mipLevelCount - 1), 1u << (params.mipLevelCount - 1),
                             params.arrayLayerCount};
        materialDesc.mipLevelCount = params.mipLevelCount;
        materialDesc.usage = wgpu::TextureUsage::TextureBinding |
                             wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst;
        materialDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        mMaterials = device.CreateTexture(&materialDesc);

        wgpu::TextureDescriptor uploadTexDesc = materialDesc;
        uploadTexDesc.size.depthOrArrayLayers = 1;
        uploadTexDesc.mipLevelCount = 1;
        uploadTexDesc.usage = wgpu::TextureUsage::CopySrc;
        mUploadTexture = device.CreateTexture(&uploadTexDesc);

        utils::ComboRenderPipelineDescriptor pipelineDesc;
        pipelineDesc.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(1.0, 0.0, 0.0, 1.0);
            }
        )");
        pipelineDesc.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var materials : texture_2d<f32>;
            @fragment fn main() -> @location(0) vec4f {
                _ = materials;
                return vec4f(1.0, 0.0, 0.0, 1.0);
            }
        )");
        mPipeline = device.CreateRenderPipeline(&pipelineDesc);
    }

  private:
    void Step() override {
        const SubresourceTrackingParams& params = GetParam();

        uint32_t layerUploaded = params.arrayLayerCount / 2;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // Copy into the layer of the material array.
        {
            wgpu::TexelCopyTextureInfo sourceView;
            sourceView.texture = mUploadTexture;

            wgpu::TexelCopyTextureInfo destView;
            destView.texture = mMaterials;
            destView.origin.z = layerUploaded;

            wgpu::Extent3D copySize = {1u << (params.mipLevelCount - 1),
                                       1u << (params.mipLevelCount - 1), 1};

            encoder.CopyTextureToTexture(&sourceView, &destView, &copySize);
        }

        // Fake commands that would be used to create the mip levels.
        for (uint32_t level = 1; level < params.mipLevelCount; level++) {
            wgpu::TextureViewDescriptor rtViewDesc;
            rtViewDesc.dimension = wgpu::TextureViewDimension::e2D;
            rtViewDesc.baseMipLevel = level;
            rtViewDesc.mipLevelCount = 1;
            rtViewDesc.baseArrayLayer = layerUploaded;
            rtViewDesc.arrayLayerCount = 1;
            wgpu::TextureView rtView = mMaterials.CreateView(&rtViewDesc);

            wgpu::TextureViewDescriptor sampleViewDesc = rtViewDesc;
            sampleViewDesc.baseMipLevel = level - 1;
            wgpu::TextureView sampleView = mMaterials.CreateView(&sampleViewDesc);

            wgpu::BindGroup bindgroup =
                utils::MakeBindGroup(device, mPipeline.GetBindGroupLayout(0), {{0, sampleView}});

            utils::ComboRenderPassDescriptor renderPass({rtView});
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetPipeline(mPipeline);
            pass.SetBindGroup(0, bindgroup);
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    wgpu::Texture mUploadTexture;
    wgpu::Texture mMaterials;
    wgpu::RenderPipeline mPipeline;
};

TEST_P(SubresourceTrackingPerf, Run) {
    RunTest();
}

DAWN_INSTANTIATE_TEST_P(SubresourceTrackingPerf,
                        {D3D12Backend(), MetalBackend(), OpenGLBackend(), VulkanBackend()},
                        {1, 4, 16, 256},
                        {2, 3, 8});

}  // anonymous namespace
}  // namespace dawn
