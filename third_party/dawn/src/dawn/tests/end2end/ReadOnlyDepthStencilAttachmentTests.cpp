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

#include <optional>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr static uint32_t kSize = 4;

using TextureFormat = wgpu::TextureFormat;
DAWN_TEST_PARAM_STRUCT(ReadOnlyDepthStencilAttachmentTestsParams, TextureFormat);

class ReadOnlyDepthStencilAttachmentTests
    : public DawnTestWithParams<ReadOnlyDepthStencilAttachmentTestsParams> {
  protected:
    void SetUp() override {
        DawnTestWithParams<ReadOnlyDepthStencilAttachmentTestsParams>::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(!mIsFormatSupported);
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        switch (GetParam().mTextureFormat) {
            case wgpu::TextureFormat::Depth32FloatStencil8:
                if (SupportsFeatures({wgpu::FeatureName::Depth32FloatStencil8})) {
                    mIsFormatSupported = true;
                    return {wgpu::FeatureName::Depth32FloatStencil8};
                }

                return {};
            default:
                mIsFormatSupported = true;
                return {};
        }
    }

    struct TestSpec {
        wgpu::TextureAspect readonlyAspects;
        std::optional<wgpu::TextureAspect> sampledAspect = std::nullopt;

        wgpu::CompareFunction depthCompare = wgpu::CompareFunction::Always;
        wgpu::CompareFunction stencilCompare = wgpu::CompareFunction::Always;
        wgpu::OptionalBool depthWriteEnabled = wgpu::OptionalBool::False;
        bool stencilWriteEnabled = false;

        float depthClearValue = 0.0;
        uint32_t stencilClearValue = 0;
        uint32_t stencilRef = 0;

        wgpu::Texture depthStencilTexture;
    };

    wgpu::RenderPipeline CreateRenderPipeline(wgpu::TextureFormat format,
                                              const TestSpec& spec) const {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;

        // Draw a rectangle via two triangles. The depth value of the top of the rectangle is 0.4.
        // The depth value of the bottom is 0.0. The depth value gradually change from 0.4 to 0.0
        // from the top to the bottom. The top part will compare with the depth values and fail to
        // pass the depth test. The bottom part will compare with the depth values in depth buffer
        // and pass the depth test, and sample from the depth buffer in fragment shader in the same
        // pipeline.
        pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec3f(-1.0,  1.0, 0.4),
                    vec3f(-1.0, -1.0, 0.0),
                    vec3f( 1.0,  1.0, 0.4),
                    vec3f( 1.0,  1.0, 0.4),
                    vec3f(-1.0, -1.0, 0.0),
                    vec3f( 1.0, -1.0, 0.0));
                return vec4f(pos[VertexIndex], 1.0);
            })");

        wgpu::BindGroupLayout bgl;
        if (!spec.sampledAspect.has_value()) {
            // Draw a solid blue into color buffer if not sample from depth/stencil attachment.
            pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
                @fragment fn main() -> @location(0) vec4f {
                    return vec4f(0.0, 0.0, 1.0, 0.0);
                })");
            bgl = utils::MakeBindGroupLayout(device, {});
        } else if (spec.sampledAspect == wgpu::TextureAspect::DepthOnly) {
            // Sample from depth attachment and draw that sampled texel into color buffer.
            if (IsCompatibilityMode()) {
                // Can not use texture_depth_xx with non-comparison sampler in compat mode.
                pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
                        @group(0) @binding(0) var samp : sampler;
                        @group(0) @binding(1) var tex : texture_2d<f32>;

                        @fragment
                        fn main(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
                            return vec4f(textureSample(tex, samp, FragCoord.xy).r, 0.0, 0.0, 0.0);
                        })");
                bgl = utils::MakeBindGroupLayout(
                    device,
                    {
                        {0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::NonFiltering},
                        {1, wgpu::ShaderStage::Fragment,
                         wgpu::TextureSampleType::UnfilterableFloat},
                    });
            } else {
                pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
                        @group(0) @binding(0) var samp : sampler;
                        @group(0) @binding(1) var tex : texture_depth_2d;

                        @fragment
                        fn main(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
                            return vec4f(textureSample(tex, samp, FragCoord.xy), 0.0, 0.0, 0.0);
                        })");
                bgl = utils::MakeBindGroupLayout(
                    device,
                    {
                        {0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::NonFiltering},
                        {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Depth},
                    });
            }
        } else {
            DAWN_ASSERT(spec.sampledAspect == wgpu::TextureAspect::StencilOnly);
            // Sample from stencil attachment and draw that sampled texel into color buffer.
            pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
                    @group(0) @binding(0) var samp : sampler;
                    @group(0) @binding(1) var tex : texture_2d<u32>;

                    @fragment
                    fn main(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
                        _ = samp;
                        var texel = textureLoad(tex, vec2i(FragCoord.xy), 0);
                        return vec4f(f32(texel[0]) / 255.0, 0.0, 0.0, 0.0);
                    })");
            bgl = utils::MakeBindGroupLayout(
                device,
                {
                    {0, wgpu::ShaderStage::Fragment, wgpu::SamplerBindingType::NonFiltering},
                    {1, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Uint},
                });
        }

        wgpu::DepthStencilState* depthStencil = pipelineDescriptor.EnableDepthStencil(format);
        depthStencil->depthCompare = spec.depthCompare;
        depthStencil->depthWriteEnabled = spec.depthWriteEnabled;
        depthStencil->stencilFront.compare = spec.stencilCompare;
        if (spec.stencilWriteEnabled) {
            depthStencil->stencilFront.passOp = wgpu::StencilOperation::Replace;
        }

        wgpu::PipelineLayoutDescriptor plDescriptor;
        plDescriptor.bindGroupLayoutCount = 1;
        plDescriptor.bindGroupLayouts = &bgl;
        pipelineDescriptor.layout = device.CreatePipelineLayout(&plDescriptor);

        return device.CreateRenderPipeline(&pipelineDescriptor);
    }

    struct RenderResult {
        wgpu::Texture color;
        wgpu::Texture depthStencil;
    };
    RenderResult DoRender(const TestSpec& spec) const {
        wgpu::TextureFormat testFormat = GetParam().mTextureFormat;

        // Create or reuse the test textures.
        wgpu::Texture depthStencilTexture = spec.depthStencilTexture;
        if (!depthStencilTexture) {
            wgpu::TextureDescriptor dsTextureDesc = {};
            dsTextureDesc.size = {kSize, kSize, 1};
            dsTextureDesc.format = testFormat;
            dsTextureDesc.usage =
                wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
            depthStencilTexture = device.CreateTexture(&dsTextureDesc);
        }

        wgpu::TextureDescriptor colorTextureDesc = {};
        colorTextureDesc.size = {kSize, kSize, 1};
        colorTextureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        colorTextureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        wgpu::Texture colorTexture = device.CreateTexture(&colorTextureDesc);

        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

        // Do a first render pass that writes the initial values to the aspects that will be
        // readonly.
        if (!spec.depthStencilTexture) {
            utils::ComboRenderPassDescriptor passDescriptorInit({},
                                                                depthStencilTexture.CreateView());
            passDescriptorInit.UnsetDepthStencilLoadStoreOpsForFormat(testFormat);
            passDescriptorInit.cDepthStencilAttachmentInfo.depthClearValue = spec.depthClearValue;
            passDescriptorInit.cDepthStencilAttachmentInfo.stencilClearValue =
                spec.stencilClearValue;

            wgpu::RenderPassEncoder passInit = commandEncoder.BeginRenderPass(&passDescriptorInit);
            passInit.End();
        }

        // Do the render pass with the readonly attachment, that will potentially use the pipeline
        // to read and/or write attachments.
        utils::ComboRenderPassDescriptor passDescriptor({colorTexture.CreateView()},
                                                        depthStencilTexture.CreateView());
        passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Load;
        passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Load;
        passDescriptor.UnsetDepthStencilLoadStoreOpsForFormat(testFormat);

        if (spec.readonlyAspects != wgpu::TextureAspect::StencilOnly) {
            passDescriptor.cDepthStencilAttachmentInfo.depthReadOnly = true;
            passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
            passDescriptor.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
        }
        if (spec.readonlyAspects != wgpu::TextureAspect::DepthOnly) {
            passDescriptor.cDepthStencilAttachmentInfo.stencilReadOnly = true;
            passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
            passDescriptor.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
        }

        // Create a render pass with readonly depth/stencil attachment. The attachment has already
        // been initialized. The pipeline in this render pass will sample from the attachment. TODO
        // The pipeline will read from the attachment to do depth/stencil test too.
        wgpu::RenderPassEncoder pass = commandEncoder.BeginRenderPass(&passDescriptor);
        wgpu::RenderPipeline pipeline = CreateRenderPipeline(testFormat, spec);
        pass.SetPipeline(pipeline);
        pass.SetStencilReference(spec.stencilRef);

        // Bind the bindgroup is needed.
        if (spec.sampledAspect.has_value()) {
            wgpu::TextureViewDescriptor viewDesc = {};
            viewDesc.aspect = spec.sampledAspect.value();
            wgpu::TextureView view = depthStencilTexture.CreateView(&viewDesc);

            wgpu::BindGroup bindGroup = utils::MakeBindGroup(
                device, pipeline.GetBindGroupLayout(0), {{0, device.CreateSampler()}, {1, view}});
            pass.SetBindGroup(0, bindGroup);
        } else {
            wgpu::BindGroup bindGroup =
                utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {});
            pass.SetBindGroup(0, bindGroup);
        }

        pass.Draw(6);
        pass.End();

        wgpu::CommandBuffer commands = commandEncoder.Finish();
        queue.Submit(1, &commands);

        return {colorTexture, depthStencilTexture};
    }

    void CheckFullColor(wgpu::Texture color, utils::RGBA8 fullColor) {
        std::vector<utils::RGBA8> expected(kSize * kSize, fullColor);
        EXPECT_TEXTURE_EQ(expected.data(), color, {0, 0}, {kSize, kSize});
    }

    void CheckTopBottomColor(wgpu::Texture color, utils::RGBA8 topColor, utils::RGBA8 bottomColor) {
        std::vector<utils::RGBA8> expectedTop(kSize * kSize / 2, topColor);
        EXPECT_TEXTURE_EQ(expectedTop.data(), color, {0, 0}, {kSize, kSize / 2});
        std::vector<utils::RGBA8> expectedBottom(kSize * kSize / 2, bottomColor);
        EXPECT_TEXTURE_EQ(expectedBottom.data(), color, {0, kSize / 2}, {kSize, kSize / 2});
    }

  private:
    bool mIsFormatSupported = false;
};

class ReadOnlyDepthAttachmentTests : public ReadOnlyDepthStencilAttachmentTests {};

TEST_P(ReadOnlyDepthAttachmentTests, SampleFromAttachment) {
    // TODO(dawn:2163): The texture reads zeroes, maybe ANGLE's TextureStorageD3D11 is missing a
    // copy between the storages?
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    TestSpec spec;
    spec.readonlyAspects = wgpu::TextureAspect::DepthOnly;
    spec.sampledAspect = wgpu::TextureAspect::DepthOnly;
    spec.depthCompare = wgpu::CompareFunction::LessEqual;
    spec.depthClearValue = 0.2;
    auto render = DoRender(spec);

    // The top part is not rendered by the pipeline. Its color is the default clear color for
    // color attachment.
    // The bottom part is rendered, whose red channel is sampled from depth attachment, which
    // is initialized into 0.2.
    CheckTopBottomColor(render.color, {0, 0, 0, 0}, {static_cast<uint8_t>(0.2 * 255), 0, 0, 0});
}

TEST_P(ReadOnlyDepthAttachmentTests, NotSampleFromAttachment) {
    TestSpec spec;
    spec.readonlyAspects = wgpu::TextureAspect::DepthOnly;
    spec.depthCompare = wgpu::CompareFunction::LessEqual;
    spec.depthClearValue = 0.2;
    auto render = DoRender(spec);

    // The top part is not rendered by the pipeline. Its color is the default clear color for
    // color attachment.
    // The bottom part is rendered. Its color is set to blue.
    CheckTopBottomColor(render.color, {0, 0, 0, 0}, {0, 0, 255, 0});
}

// Regression test for crbug.com/dawn/1512 where having aspectReadOnly for an unused aspect of a
// depth-stencil texture would cause the attachment to be considered read-only, causing layout
// mismatch issues.
TEST_P(ReadOnlyDepthAttachmentTests, UnusedAspectWithReadOnly) {
    wgpu::TextureFormat format = GetParam().mTextureFormat;
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.format = format;
    tDesc.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture depthStencilTexture = device.CreateTexture(&tDesc);

    utils::ComboRenderPassDescriptor passDescriptor({}, depthStencilTexture.CreateView());
    if (utils::IsStencilOnlyFormat(format)) {
        passDescriptor.cDepthStencilAttachmentInfo.depthReadOnly = true;
        passDescriptor.cDepthStencilAttachmentInfo.depthLoadOp = wgpu::LoadOp::Undefined;
        passDescriptor.cDepthStencilAttachmentInfo.depthStoreOp = wgpu::StoreOp::Undefined;
    } else {
        passDescriptor.cDepthStencilAttachmentInfo.depthReadOnly = false;
    }
    if (utils::IsDepthOnlyFormat(format)) {
        passDescriptor.cDepthStencilAttachmentInfo.stencilReadOnly = true;
        passDescriptor.cDepthStencilAttachmentInfo.stencilLoadOp = wgpu::LoadOp::Undefined;
        passDescriptor.cDepthStencilAttachmentInfo.stencilStoreOp = wgpu::StoreOp::Undefined;
    } else {
        passDescriptor.cDepthStencilAttachmentInfo.stencilReadOnly = false;
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&passDescriptor);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();

    queue.Submit(1, &commands);
}

class ReadOnlyStencilAttachmentTests : public ReadOnlyDepthStencilAttachmentTests {};

TEST_P(ReadOnlyStencilAttachmentTests, SampleFromAttachment) {
    // TODO(anglebug.com/344950145): assert failed in rx::TextureStorage11::verifySwizzleExists
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    // stencilRefValue < stencilValue (stencilInitValue), so stencil test passes. The pipeline
    // samples from stencil buffer and writes into color buffer.
    {
        TestSpec spec;
        spec.readonlyAspects = wgpu::TextureAspect::StencilOnly;
        spec.sampledAspect = wgpu::TextureAspect::StencilOnly;
        spec.stencilCompare = wgpu::CompareFunction::LessEqual;
        spec.stencilClearValue = 3;
        spec.stencilRef = 2;
        auto render = DoRender(spec);
        CheckFullColor(render.color, {3, 0, 0, 0});
    }

    // stencilRefValue > stencilValue (stencilInitValue), so stencil test fails. The pipeline
    // doesn't change color buffer. Sampled data from stencil buffer is discarded.
    {
        TestSpec spec;
        spec.readonlyAspects = wgpu::TextureAspect::StencilOnly;
        spec.sampledAspect = wgpu::TextureAspect::StencilOnly;
        spec.stencilCompare = wgpu::CompareFunction::LessEqual;
        spec.stencilClearValue = 1;
        spec.stencilRef = 2;
        auto render = DoRender(spec);
        CheckFullColor(render.color, {0, 0, 0, 0});
    }
}

TEST_P(ReadOnlyStencilAttachmentTests, NotSampleFromAttachment) {
    // stencilRefValue < stencilValue (stencilInitValue), so stencil test passes. The pipeline
    // draw solid blue into color buffer.
    {
        TestSpec spec;
        spec.readonlyAspects = wgpu::TextureAspect::StencilOnly;
        spec.stencilCompare = wgpu::CompareFunction::LessEqual;
        spec.stencilClearValue = 3;
        spec.stencilRef = 2;
        auto render = DoRender(spec);
        CheckFullColor(render.color, {0, 0, 255, 0});
    }

    // stencilRefValue > stencilValue (stencilInitValue), so stencil test fails. The pipeline
    // doesn't change color buffer. drawing data is discarded.
    {
        TestSpec spec;
        spec.readonlyAspects = wgpu::TextureAspect::StencilOnly;
        spec.stencilCompare = wgpu::CompareFunction::LessEqual;
        spec.stencilClearValue = 1;
        spec.stencilRef = 2;
        auto render = DoRender(spec);
        CheckFullColor(render.color, {0, 0, 0, 0});
    }
}

class ReadOnlyDepthAndStencilAttachmentTests : public ReadOnlyDepthStencilAttachmentTests {};

// Test that using stencilReadOnly while modifying the depth aspect works.
TEST_P(ReadOnlyDepthAndStencilAttachmentTests, ModifyDepthSampleStencil) {
    // TODO(anglebug.com/344950145): assert failed in rx::TextureStorage11::verifySwizzleExists
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    // Stencil test is always true but the depth test passes only for the
    TestSpec spec1;
    spec1.readonlyAspects = wgpu::TextureAspect::StencilOnly;
    spec1.sampledAspect = wgpu::TextureAspect::StencilOnly;
    spec1.stencilClearValue = 42;
    spec1.depthClearValue = 0.2;
    spec1.depthCompare = wgpu::CompareFunction::LessEqual;
    spec1.depthWriteEnabled = wgpu::OptionalBool::True;
    auto render1 = DoRender(spec1);

    // Stencil was read successfully, but only in the bottom part.
    CheckTopBottomColor(render1.color, {0, 0, 0, 0}, {42, 0, 0, 0});

    // Check that the depth was written by setting depthCompare equal, and rendering a solid
    // blue color. The depth was only written on the bottom half due to the depth test in the
    // first render.
    TestSpec spec2;
    spec2.readonlyAspects = wgpu::TextureAspect::StencilOnly;
    spec2.depthStencilTexture = render1.depthStencil;
    spec2.depthCompare = wgpu::CompareFunction::Equal;
    auto render2 = DoRender(spec2);
    CheckTopBottomColor(render2.color, {0, 0, 0, 0}, {0, 0, 255, 0});
}

// Test that using depthReadOnly while modifying the stencil aspect works.
TEST_P(ReadOnlyDepthAndStencilAttachmentTests, SampleDepthModifyStencil) {
    // TODO(dawn:2163): The texture reads zeroes, maybe ANGLE's TextureStorageD3D11 is missing a
    // copy between the storages?
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    // Depth/stencil tests are true, the depth is correctly sampled from the depthClearValue.
    // The stencil is written to the value of the stencil ref.
    TestSpec spec1;
    spec1.readonlyAspects = wgpu::TextureAspect::DepthOnly;
    spec1.sampledAspect = wgpu::TextureAspect::DepthOnly;
    spec1.depthClearValue = 0.2;
    spec1.stencilWriteEnabled = true;
    spec1.stencilRef = 42;
    spec1.stencilClearValue = 0;
    auto render1 = DoRender(spec1);
    CheckFullColor(render1.color, {static_cast<uint8_t>(0.2 * 255), 0, 0, 0});

    // The stencil test checks that the stencil ref matches what's in the stencil buffer
    // so that we know it was correctly written. The whole quad should be drawn.
    TestSpec spec2;
    spec2.readonlyAspects = wgpu::TextureAspect::DepthOnly;
    spec2.depthStencilTexture = render1.depthStencil;
    spec2.stencilCompare = wgpu::CompareFunction::Equal;
    spec2.stencilRef = 42;
    auto render2 = DoRender(spec2);
    CheckFullColor(render2.color, {0, 0, 255, 0});
}

// Test sampling depth with both the depth and stencil readonly.
TEST_P(ReadOnlyDepthAndStencilAttachmentTests, BothReadOnlySampleDepth) {
    // TODO(dawn:2163): The texture reads zeroes, maybe ANGLE's TextureStorageD3D11 is missing a
    // copy between the storages?
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    // Sample the depth while using both depth an stencil testing.

    // First render: depth test passes only for the bottom half, stencil passes.
    TestSpec spec;
    spec.readonlyAspects = wgpu::TextureAspect::All;
    spec.sampledAspect = wgpu::TextureAspect::DepthOnly;
    spec.depthCompare = wgpu::CompareFunction::LessEqual;
    spec.depthClearValue = 0.2;
    spec.stencilRef = 42;
    spec.stencilClearValue = 43;
    spec.stencilCompare = wgpu::CompareFunction::LessEqual;
    auto render1 = DoRender(spec);
    CheckTopBottomColor(render1.color, {0, 0, 0, 0}, {static_cast<uint8_t>(0.2 * 255), 0, 0, 0});

    // Second render: stencil test fails.
    spec.stencilClearValue = 41;
    auto render2 = DoRender(spec);
    CheckFullColor(render2.color, {0, 0, 0, 0});
}

// Test sampling stencil with both the depth and stencil readonly.
TEST_P(ReadOnlyDepthAndStencilAttachmentTests, BothReadOnlySampleStencil) {
    // TODO(anglebug.com/344950145): assert failed in rx::TextureStorage11::verifySwizzleExists
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());
    // Sample the stencil while using both depth an stencil testing.

    // First render: depth test passes only for the bottom half, stencil passes.
    TestSpec spec;
    spec.readonlyAspects = wgpu::TextureAspect::All;
    spec.sampledAspect = wgpu::TextureAspect::StencilOnly;
    spec.depthCompare = wgpu::CompareFunction::LessEqual;
    spec.depthClearValue = 0.2;
    spec.stencilRef = 42;
    spec.stencilClearValue = 43;
    spec.stencilCompare = wgpu::CompareFunction::LessEqual;
    auto render1 = DoRender(spec);
    CheckTopBottomColor(render1.color, {0, 0, 0, 0}, {43, 0, 0, 0});

    // Second render: stencil test fails.
    spec.stencilClearValue = 41;
    auto render2 = DoRender(spec);
    CheckFullColor(render2.color, {0, 0, 0, 0});
}

DAWN_INSTANTIATE_TEST_P(ReadOnlyDepthAttachmentTests,
                        {D3D11Backend(), D3D12Backend(),
                         D3D12Backend({}, {"use_d3d12_render_pass"}), MetalBackend(),
                         OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
                        std::vector<wgpu::TextureFormat>(utils::kDepthFormats.begin(),
                                                         utils::kDepthFormats.end()));
DAWN_INSTANTIATE_TEST_P(ReadOnlyStencilAttachmentTests,
                        {D3D11Backend(), D3D12Backend(),
                         D3D12Backend({}, {"use_d3d12_render_pass"}), MetalBackend(),
                         OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
                        std::vector<wgpu::TextureFormat>(utils::kStencilFormats.begin(),
                                                         utils::kStencilFormats.end()));
DAWN_INSTANTIATE_TEST_P(ReadOnlyDepthAndStencilAttachmentTests,
                        {D3D11Backend(), D3D12Backend(),
                         D3D12Backend({}, {"use_d3d12_render_pass"}), MetalBackend(),
                         OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
                        std::vector<wgpu::TextureFormat>(utils::kDepthAndStencilFormats.begin(),
                                                         utils::kDepthAndStencilFormats.end()));

}  // anonymous namespace
}  // namespace dawn
