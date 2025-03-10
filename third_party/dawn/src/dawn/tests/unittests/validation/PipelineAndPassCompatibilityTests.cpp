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

#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

#include "dawn/tests/unittests/validation/ValidationTest.h"

namespace dawn {
namespace {

constexpr static uint32_t kSize = 4;
// Note that format Depth24PlusStencil8 has both depth and stencil aspects, so parameters
// depthReadOnly and stencilReadOnly should be the same in render pass and render bundle.
wgpu::TextureFormat kFormat = wgpu::TextureFormat::Depth24PlusStencil8;

class RenderPipelineAndPassCompatibilityTests : public ValidationTest {
  public:
    wgpu::RenderPipeline CreatePipeline(wgpu::TextureFormat format,
                                        bool enableDepthWrite,
                                        bool enableStencilWrite) {
        // Create a NoOp pipeline
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
                @vertex fn main() -> @builtin(position) vec4f {
                    return vec4f();
                })");
        pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
                @fragment fn main() {
                })");
        pipelineDescriptor.cFragment.targets = nullptr;
        pipelineDescriptor.cFragment.targetCount = 0;

        // Enable depth/stencil write if needed
        wgpu::DepthStencilState* depthStencil = pipelineDescriptor.EnableDepthStencil(format);
        if (enableDepthWrite) {
            depthStencil->depthWriteEnabled = wgpu::OptionalBool::True;
        }
        if (enableStencilWrite) {
            depthStencil->stencilFront.failOp = wgpu::StencilOperation::Replace;
        }
        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    utils::ComboRenderPassDescriptor CreateRenderPassDescriptor(wgpu::TextureFormat format,
                                                                bool depthReadOnly,
                                                                bool stencilReadOnly) {
        wgpu::TextureDescriptor textureDescriptor = {};
        textureDescriptor.size = {kSize, kSize, 1};
        textureDescriptor.format = format;
        textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
        wgpu::Texture depthStencilTexture = device.CreateTexture(&textureDescriptor);

        utils::ComboRenderPassDescriptor passDescriptor({}, depthStencilTexture.CreateView());
        if (depthReadOnly) {
            passDescriptor.cDepthStencilAttachmentInfo.depthReadOnly = true;
            passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
            passDescriptor.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
        }

        if (stencilReadOnly) {
            passDescriptor.cDepthStencilAttachmentInfo.stencilReadOnly = true;
            passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
            passDescriptor.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        }

        return passDescriptor;
    }
};

// Test depthReadOnly compatibility between pipelines and render passes.
TEST_F(RenderPipelineAndPassCompatibilityTests, PipelineAndRenderPassDepthReadOnly) {
    for (bool depthReadOnlyInPass : {true, false}) {
        for (bool depthWriteInPipeline : {true, false}) {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            utils::ComboRenderPassDescriptor passDescriptor =
                CreateRenderPassDescriptor(kFormat, depthReadOnlyInPass, false);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
            wgpu::RenderPipeline pipeline = CreatePipeline(kFormat, depthWriteInPipeline, false);
            pass.SetPipeline(pipeline);
            pass.Draw(3);
            pass.End();
            if (depthReadOnlyInPass && depthWriteInPipeline) {
                ASSERT_DEVICE_ERROR(encoder.Finish());
            } else {
                encoder.Finish();
            }
        }
    }
}

// Test stencilReadOnly compatibility between pipelines and render passes.
TEST_F(RenderPipelineAndPassCompatibilityTests, PipelineAndRenderPassStencilReadOnly) {
    for (bool stencilReadOnlyInPass : {true, false}) {
        for (bool stencilWriteInPipeline : {true, false}) {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            utils::ComboRenderPassDescriptor passDescriptor =
                CreateRenderPassDescriptor(kFormat, false, stencilReadOnlyInPass);
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
            wgpu::RenderPipeline pipeline = CreatePipeline(kFormat, false, stencilWriteInPipeline);
            pass.SetPipeline(pipeline);
            pass.Draw(3);
            pass.End();
            if (stencilReadOnlyInPass && stencilWriteInPipeline) {
                ASSERT_DEVICE_ERROR(encoder.Finish());
            } else {
                encoder.Finish();
            }
        }
    }
}

// Test depthReadOnly compatibility between pipelines and render bundles.
TEST_F(RenderPipelineAndPassCompatibilityTests, PipelineAndBundleDepthReadOnly) {
    for (bool depthReadOnlyInBundle : {true, false}) {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.depthStencilFormat = kFormat;
        desc.depthReadOnly = depthReadOnlyInBundle;
        desc.stencilReadOnly = false;

        for (bool depthWriteInPipeline : {true, false}) {
            wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
            wgpu::RenderPipeline pipeline = CreatePipeline(kFormat, depthWriteInPipeline, false);
            renderBundleEncoder.SetPipeline(pipeline);
            renderBundleEncoder.Draw(3);
            if (depthReadOnlyInBundle && depthWriteInPipeline) {
                ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
            } else {
                renderBundleEncoder.Finish();
            }
        }
    }
}

// Test stencilReadOnly compatibility between pipelines and render bundles.
TEST_F(RenderPipelineAndPassCompatibilityTests, PipelineAndBundleStencilReadOnly) {
    for (bool stencilReadOnlyInBundle : {true, false}) {
        utils::ComboRenderBundleEncoderDescriptor desc = {};
        desc.depthStencilFormat = kFormat;
        desc.depthReadOnly = false;
        desc.stencilReadOnly = stencilReadOnlyInBundle;

        for (bool stencilWriteInPipeline : {true, false}) {
            wgpu::RenderBundleEncoder renderBundleEncoder = device.CreateRenderBundleEncoder(&desc);
            wgpu::RenderPipeline pipeline = CreatePipeline(kFormat, false, stencilWriteInPipeline);
            renderBundleEncoder.SetPipeline(pipeline);
            renderBundleEncoder.Draw(3);
            if (stencilReadOnlyInBundle && stencilWriteInPipeline) {
                ASSERT_DEVICE_ERROR(renderBundleEncoder.Finish());
            } else {
                renderBundleEncoder.Finish();
            }
        }
    }
}

// Test depthReadOnly compatibility between render passes and render bundles.
TEST_F(RenderPipelineAndPassCompatibilityTests, BundleAndPassDepthReadOnly) {
    for (bool depthReadOnlyInPass : {true, false}) {
        for (bool depthReadOnlyInBundle : {true, false}) {
            for (bool emptyBundle : {true, false}) {
                // Create render bundle, with or without a pipeline
                utils::ComboRenderBundleEncoderDescriptor desc = {};
                desc.depthStencilFormat = kFormat;
                desc.depthReadOnly = depthReadOnlyInBundle;
                desc.stencilReadOnly = false;
                wgpu::RenderBundleEncoder renderBundleEncoder =
                    device.CreateRenderBundleEncoder(&desc);
                if (!emptyBundle) {
                    wgpu::RenderPipeline pipeline =
                        CreatePipeline(kFormat, !depthReadOnlyInBundle, false);
                    renderBundleEncoder.SetPipeline(pipeline);
                    renderBundleEncoder.Draw(3);
                }
                wgpu::RenderBundle bundle = renderBundleEncoder.Finish();

                // Create render pass and call ExecuteBundles()
                wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
                utils::ComboRenderPassDescriptor passDescriptor =
                    CreateRenderPassDescriptor(kFormat, depthReadOnlyInPass, false);
                wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
                pass.ExecuteBundles(1, &bundle);
                pass.End();
                if (depthReadOnlyInPass && !depthReadOnlyInBundle) {
                    ASSERT_DEVICE_ERROR(encoder.Finish());
                } else {
                    encoder.Finish();
                }
            }
        }
    }
}

// Test stencilReadOnly compatibility between render passes and render bundles.
TEST_F(RenderPipelineAndPassCompatibilityTests, BundleAndPassStencilReadOnly) {
    for (bool stencilReadOnlyInPass : {true, false}) {
        for (bool stencilReadOnlyInBundle : {true, false}) {
            for (bool emptyBundle : {true, false}) {
                // Create render bundle, with or without a pipeline
                utils::ComboRenderBundleEncoderDescriptor desc = {};
                desc.depthStencilFormat = kFormat;
                desc.depthReadOnly = false;
                desc.stencilReadOnly = stencilReadOnlyInBundle;
                wgpu::RenderBundleEncoder renderBundleEncoder =
                    device.CreateRenderBundleEncoder(&desc);
                if (!emptyBundle) {
                    wgpu::RenderPipeline pipeline =
                        CreatePipeline(kFormat, false, !stencilReadOnlyInBundle);
                    renderBundleEncoder.SetPipeline(pipeline);
                    renderBundleEncoder.Draw(3);
                }
                wgpu::RenderBundle bundle = renderBundleEncoder.Finish();

                // Create render pass and call ExecuteBundles()
                wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
                utils::ComboRenderPassDescriptor passDescriptor =
                    CreateRenderPassDescriptor(kFormat, false, stencilReadOnlyInPass);
                wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
                pass.ExecuteBundles(1, &bundle);
                pass.End();
                if (stencilReadOnlyInPass && !stencilReadOnlyInBundle) {
                    ASSERT_DEVICE_ERROR(encoder.Finish());
                } else {
                    encoder.Finish();
                }
            }
        }
    }
}

// TODO(dawn:485): add more tests. For example:
//   - depth/stencil attachment should be designated if depth/stencil test is enabled.
//   - pipeline and pass compatibility tests for color attachment(s).
//   - pipeline and pass compatibility tests for compute.

}  // anonymous namespace
}  // namespace dawn
