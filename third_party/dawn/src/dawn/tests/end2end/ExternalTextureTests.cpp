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

#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

wgpu::Texture Create2DTexture(wgpu::Device device,
                              uint32_t width,
                              uint32_t height,
                              wgpu::TextureFormat format,
                              wgpu::TextureUsage usage) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = format;
    descriptor.mipLevelCount = 1;
    descriptor.usage = usage;
    return device.CreateTexture(&descriptor);
}

struct YUVTestData {
    float y;
    float u;
    float v;
    std::array<float, 4> rgbaFloats;
    utils::RGBA8 rgba;
};
static const YUVTestData kBlack = {
    0.0, 0.5, 0.5, {0.0, 0.0, 0.0, 1.0}, utils::RGBA8::kBlack,
};
static const YUVTestData kRed = {
    0.2126, 0.4172, 1.0, {1.0, 0.0, 0.0, 1.0}, utils::RGBA8::kRed,
};
static const YUVTestData kGreen = {
    0.7152, 0.1402, 0.0175, {0.0, 1.0, 0.0, 1.0}, utils::RGBA8::kGreen,
};
static const YUVTestData kBlue = {
    0.0722, 1.0, 0.4937, {0.0, 0.0, 1.0, 1.0}, utils::RGBA8::kBlue,
};
static const YUVTestData kColor1 = {0.6382,
                                    0.3232,
                                    0.6644,
                                    {246 / 255.0, 169 / 255.0, 90 / 255.0, 1},
                                    {246, 169, 90, 255}};

template <typename Parent>
class ExternalTextureTestsBase : public Parent {
  protected:
    void SetUp() override {
        Parent::SetUp();

        vsModule = utils::CreateShaderModule(this->device, R"(
            @vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var positions = array(
                    vec4f(-1.0, 1.0, 0.0, 1.0),
                    vec4f(-1.0, -1.0, 0.0, 1.0),
                    vec4f(1.0, 1.0, 0.0, 1.0),
                    vec4f(1.0, -1.0, 0.0, 1.0),
                    vec4f(-1.0, -1.0, 0.0, 1.0),
                    vec4f(1.0, 1.0, 0.0, 1.0)
                );
                return positions[VertexIndex];
            })");

        fsSampleExternalTextureModule = utils::CreateShaderModule(this->device, R"(
            @group(0) @binding(0) var s : sampler;
            @group(0) @binding(1) var t : texture_external;

            @fragment fn main(@builtin(position) FragCoord : vec4f)
                                     -> @location(0) vec4f {
                return textureSampleBaseClampToEdge(t, s, FragCoord.xy / vec2f(4.0, 4.0));
            })");
    }

    wgpu::ExternalTextureDescriptor InitExternalTextureDescriptor(wgpu::Texture plane0,
                                                                  wgpu::Texture plane1 = nullptr) {
        wgpu::ExternalTextureDescriptor desc;
        desc.plane0 = plane0.CreateView();
        desc.plane1 = plane1 != nullptr ? plane1.CreateView() : nullptr;

        const auto& conversion = plane1 == nullptr ? noopRGBConversion : bt709Conversion;
        desc.yuvToRgbConversionMatrix = conversion.yuvToRgbConversionMatrix.data();
        desc.gamutConversionMatrix = conversion.gamutConversionMatrix.data();
        desc.srcTransferFunctionParameters = conversion.srcTransferFunctionParameters.data();
        desc.dstTransferFunctionParameters = conversion.dstTransferFunctionParameters.data();
        desc.cropOrigin = {0, 0};
        desc.cropSize = {plane0.GetWidth(), plane0.GetHeight()};
        desc.apparentSize = {plane0.GetWidth(), plane0.GetHeight()};

        return desc;
    }

    // Helper function to render a quad of data in a texture with a different color for each
    // corner as well as an optional color for the outside of the quad. (the quad can be scaled
    // to make it fill only parts of the texture).
    struct QuadData {
        std::array<float, 4> upperLeft;
        std::array<float, 4> upperRight;
        std::array<float, 4> lowerLeft;
        std::array<float, 4> lowerRight;
        std::array<float, 4> outsideData = {};
        float scale = 1.0;
        std::array<float, 3> padding = {};
    };
    void RenderQuad(const wgpu::Texture& texture, const QuadData& quad) {
        // Make the pipeline drawing a quad in the texture, taking the colors as a uniform buffer.
        wgpu::ShaderModule module = utils::CreateShaderModule(this->device, R"(
            struct VsOut {
                @builtin(position) pos : vec4f,
                @interpolate(perspective) @location(0) ndc : vec4f,
            }
            @vertex fn vs(@builtin(vertex_index) VertexIndex : u32) -> VsOut {
                var pos = array(
                    vec4f(-3, -1, 0, 1),
                    vec4f( 3, -1, 0, 1),
                    vec4f( 0,  2, 0, 1),
                );
                return VsOut(pos[VertexIndex], pos[VertexIndex]);
            }

            struct QuadData {
                upperLeft : vec4f,
                upperRight : vec4f,
                lowerLeft : vec4f,
                lowerRight : vec4f,
                outside : vec4f,
                scale : f32,
            }
            @group(0) @binding(0) var<uniform> quad : QuadData;
            @fragment fn fs(@interpolate(perspective) @location(0) ndc : vec4f)
                                     -> @location(0) vec4f {
                if abs(ndc.x) > quad.scale || abs(ndc.y) > quad.scale {
                    return quad.outside;
                }

                if ndc.x <= 0 && ndc.y >= 0 {
                    return quad.upperLeft;
                } else if ndc.x >= 0 && ndc.y >= 0 {
                    return quad.upperRight;
                } else if ndc.x <= 0 && ndc.y <= 0 {
                    return quad.lowerLeft;
                } else {
                    return quad.lowerRight;
                }
            })");

        utils::ComboRenderPipelineDescriptor pDesc;
        pDesc.vertex.module = module;
        pDesc.cFragment.module = module;
        pDesc.cTargets[0].format = texture.GetFormat();
        wgpu::RenderPipeline quadPipeline = this->device.CreateRenderPipeline(&pDesc);

        // Make the storage buffer and the bind group containing it.
        wgpu::Buffer quadData = utils::CreateBufferFromData(this->device, &quad, sizeof(quad),
                                                            wgpu::BufferUsage::Uniform);
        wgpu::BindGroup bg =
            utils::MakeBindGroup(this->device, quadPipeline.GetBindGroupLayout(0), {{0, quadData}});

        // Do the render.
        wgpu::CommandEncoder encoder = this->device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass({texture.CreateView()});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(quadPipeline);
        pass.SetBindGroup(0, bg);
        pass.Draw(3);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        this->queue.Submit(1, &commands);
    }

    static constexpr uint32_t kWidth = 4;
    static constexpr uint32_t kHeight = 4;
    static constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;
    static constexpr wgpu::TextureUsage kSampledUsage = wgpu::TextureUsage::TextureBinding;

    wgpu::ShaderModule vsModule;
    wgpu::ShaderModule fsSampleExternalTextureModule;

    utils::ColorSpaceConversionInfo noopRGBConversion = utils::GetNoopRGBColorSpaceConversionInfo();
    utils::ColorSpaceConversionInfo bt709Conversion =
        utils::GetYUVBT709ToRGBSRGBColorSpaceConversionInfo();
};

class ExternalTextureTests : public ExternalTextureTestsBase<DawnTest> {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::Unorm16TextureFormats})) {
            mIsUnorm16TextureFormatsSupported = true;
            requiredFeatures.push_back(wgpu::FeatureName::Unorm16TextureFormats);
        }
        return requiredFeatures;
    }

    bool IsUnorm16TextureFormatsSupported() { return mIsUnorm16TextureFormatsSupported; }

    bool mIsUnorm16TextureFormatsSupported = false;
};

TEST_P(ExternalTextureTests, CreateExternalTextureSuccess) {
    wgpu::Texture texture = Create2DTexture(device, kWidth, kHeight, kFormat, kSampledUsage);

    // Import the external texture
    wgpu::ExternalTextureDescriptor externalDesc = InitExternalTextureDescriptor(texture);
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    ASSERT_NE(externalTexture.Get(), nullptr);
}

TEST_P(ExternalTextureTests, SampleExternalTexture) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::Texture sampledTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Initialize texture with green to ensure it is sampled from later.
    {
        utils::ComboRenderPassDescriptor renderPass({sampledTexture.CreateView()}, nullptr);
        renderPass.cColorAttachments[0].clearValue = {0.0f, 1.0f, 0.0f, 1.0f};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Pipeline Creation
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsSampleExternalTextureModule;
    descriptor.cTargets[0].format = kFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // Import the external texture
    wgpu::ExternalTextureDescriptor externalDesc = InitExternalTextureDescriptor(sampledTexture);
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a sampler and bind group
    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, sampler}, {1, externalTexture}});

    // Run the shader, which should sample from the external texture and draw a triangle into the
    // upper left corner of the render texture.
    wgpu::TextureView renderView = renderTexture.CreateView();
    utils::ComboRenderPassDescriptor renderPass({renderView});
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    {
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderTexture, 0, 0);
}

// Tests that a texture view can be used for an externalTexture binding.
TEST_P(ExternalTextureTests, SampleTextureView) {
    wgpu::Texture sampledTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Initialize texture with green to ensure it is sampled from later.
    {
        utils::ComboRenderPassDescriptor renderPass({sampledTexture.CreateView()}, nullptr);
        renderPass.cColorAttachments[0].clearValue = {0.0f, 1.0f, 0.0f, 1.0f};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Pipeline Creation
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsSampleExternalTextureModule;
    descriptor.cTargets[0].format = kFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    wgpu::TextureView textureView = sampledTexture.CreateView();

    // Create a sampler and bind group that uses a TextureView for the external_texture in WGSL
    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, sampler}, {1, textureView}});

    // Run the shader, which should sample from the texture view and draw a triangle into the
    // upper left corner of the render texture.
    wgpu::TextureView renderView = renderTexture.CreateView();
    utils::ComboRenderPassDescriptor renderPass({renderView});
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    {
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderTexture, 0, 0);
}

// Tests that textureDimensions WGSL built-in function works when a texture view is used for an
// externalTexture binding.
TEST_P(ExternalTextureTests, TextureDimensionsWithTextureView) {
    DAWN_SUPPRESS_TEST_IF(IsWARP());  // Flaky on WARP

    wgpu::TextureDescriptor descriptor;
    descriptor.size = {kWidth, kHeight, 1};
    descriptor.mipLevelCount = 2;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture texture = device.CreateTexture(&descriptor);

    for (const auto& mipLevel : {0, 1}) {
        wgpu::TextureViewDescriptor textureViewDesc;
        textureViewDesc.dimension = wgpu::TextureViewDimension::e2D;
        textureViewDesc.mipLevelCount = 1;
        textureViewDesc.baseMipLevel = mipLevel;
        wgpu::TextureView textureView = texture.CreateView(&textureViewDesc);

        // Create buffer that will store texture dimensions
        std::vector<uint32_t> data(2);
        std::vector<uint32_t> expected(2);
        if (mipLevel == 0) {
            expected = {kWidth, kHeight};
        } else {
            expected = {kWidth / 2, kHeight / 2};
        }
        uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(uint32_t));
        wgpu::Buffer buffer =
            utils::CreateBufferFromData(device, data.data(), bufferSize,
                                        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

        wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var texture : texture_external;
        @group(0) @binding(1) var<storage, read_write> buffer: vec2u;

        @compute @workgroup_size(1) fn main() {
            buffer = textureDimensions(texture);
        })");

        // Pipeline Creation
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = module;
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

        // Set up bind group that uses a TextureView for the external_texture in WGSL
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, textureView}, {1, buffer, 0, bufferSize}});

        // Issue dispatch
        wgpu::CommandBuffer commands;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();
        commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U32_EQ(expected[0], buffer, 0);
        EXPECT_BUFFER_U32_EQ(expected[1], buffer, 4);
    }
}

// Tests that textureLoad WGSL built-in function works when a texture view is used for an
// externalTexture binding.
TEST_P(ExternalTextureTests, TextureLoadWithTextureView) {
    wgpu::Texture texture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);

    // Create buffer that will store textureLoad result
    std::vector<float> data = {42, 42, 42, 42};
    std::vector<float> expected = {0, 0, 0, 0};
    uint64_t bufferSize = static_cast<uint64_t>(data.size() * sizeof(float));
    wgpu::Buffer buffer = utils::CreateBufferFromData(
        device, data.data(), bufferSize, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var texture : texture_external;
        @group(0) @binding(1) var<storage, read_write> buffer: vec4f;

        @compute @workgroup_size(1) fn main() {
            buffer = textureLoad(texture, vec2(0, 0));
        })");

    // Pipeline Creation
    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Set up bind group that uses a TextureView for the external_texture in WGSL
    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                             {{0, texture.CreateView()}, {1, buffer, 0, bufferSize}});

    // Issue dispatch
    wgpu::CommandBuffer commands;
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);
    pass.SetBindGroup(0, bindGroup);
    pass.DispatchWorkgroups(1);
    pass.End();
    commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_FLOAT_EQ(expected[0], buffer, 0);
    EXPECT_BUFFER_FLOAT_EQ(expected[1], buffer, 4);
    EXPECT_BUFFER_FLOAT_EQ(expected[2], buffer, 8);
    EXPECT_BUFFER_FLOAT_EQ(expected[3], buffer, 12);
}

// https://crbug.com/1515439
TEST_P(ExternalTextureTests, SampleExternalTextureDifferingGroup) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::Texture sampledTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Initialize texture with green to ensure it is sampled from later.
    {
        utils::ComboRenderPassDescriptor renderPass({sampledTexture.CreateView()}, nullptr);
        renderPass.cColorAttachments[0].clearValue = {0.0f, 1.0f, 0.0f, 1.0f};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var s : sampler;
            @group(1) @binding(0) var t : texture_external;

            @fragment fn main(@builtin(position) FragCoord : vec4f)
                                     -> @location(0) vec4f {
                return textureSampleBaseClampToEdge(t, s, FragCoord.xy / vec2f(4.0, 4.0));
            })");

    // Pipeline Creation
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsModule;
    descriptor.cTargets[0].format = kFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // Import the external texture
    wgpu::ExternalTextureDescriptor externalDesc = InitExternalTextureDescriptor(sampledTexture);
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a sampler and bind group
    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::BindGroup samplerBindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, sampler}});
    wgpu::BindGroup texBindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(1), {{0, externalTexture}});

    // Run the shader, which should sample from the external texture and draw a triangle into the
    // upper left corner of the render texture.
    wgpu::TextureView renderView = renderTexture.CreateView();
    utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    {
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, samplerBindGroup);
        pass.SetBindGroup(1, texBindGroup);
        pass.Draw(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderTexture, 0, 0);
}

TEST_P(ExternalTextureTests, SampleMultiplanarExternalTexture) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::Texture sampledTexturePlane0 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::R8Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture sampledTexturePlane1 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::RG8Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);

    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Create a texture view for the external texture
    wgpu::TextureView externalViewPlane0 = sampledTexturePlane0.CreateView();
    wgpu::TextureView externalViewPlane1 = sampledTexturePlane1.CreateView();

    for (const auto& expectation : {kBlack, kRed, kGreen, kBlue, kColor1}) {
        // Initialize the texture planes with YUV data
        {
            utils::ComboRenderPassDescriptor renderPass({externalViewPlane0, externalViewPlane1},
                                                        nullptr);
            renderPass.cColorAttachments[0].clearValue = {expectation.y, 0.0f, 0.0f, 0.0f};
            renderPass.cColorAttachments[1].clearValue = {expectation.u, expectation.v, 0.0f, 0.0f};
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.End();

            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);
        }

        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        // descriptor.layout = utils::MakeBasicPipelineLayout(device, &bgl);
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Import the external texture
        wgpu::ExternalTextureDescriptor externalDesc =
            InitExternalTextureDescriptor(sampledTexturePlane0, sampledTexturePlane1);
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(expectation.rgba, renderTexture, 0, 0);
    }
}

TEST_P(ExternalTextureTests, SampleMultiplanarExternalTextureNorm16) {
    DAWN_TEST_UNSUPPORTED_IF(!IsUnorm16TextureFormatsSupported());

    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::Texture sampledTexturePlane0 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::R16Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture sampledTexturePlane1 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::RG16Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);

    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Create a texture view for the external texture
    wgpu::TextureView externalViewPlane0 = sampledTexturePlane0.CreateView();
    wgpu::TextureView externalViewPlane1 = sampledTexturePlane1.CreateView();

    for (const auto& expectation : {kBlack, kRed, kGreen, kBlue, kColor1}) {
        // Initialize the texture planes with YUV data
        {
            utils::ComboRenderPassDescriptor renderPass({externalViewPlane0, externalViewPlane1},
                                                        nullptr);
            renderPass.cColorAttachments[0].clearValue = {expectation.y, 0.0f, 0.0f, 0.0f};
            renderPass.cColorAttachments[1].clearValue = {expectation.u, expectation.v, 0.0f, 0.0f};
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.End();

            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);
        }

        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Import the external texture
        wgpu::ExternalTextureDescriptor externalDesc =
            InitExternalTextureDescriptor(sampledTexturePlane0, sampledTexturePlane1);
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(3);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(expectation.rgba, renderTexture, 0, 0);
    }
}

// Test draws a green square in the upper left quadrant, a black square in the upper right, a red
// square in the lower left and a blue square in the lower right. The image is then sampled as an
// external texture and rotated 0, 90, 180, and 270 degrees with and without the y-axis flipped.
TEST_P(ExternalTextureTests, RotateAndOrFlipSampleSinglePlane) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    // TODO(41487285): Fails on OpenGL ANGLE D3D11 Intel (but not other configs). Suppress since we
    // don't want to ship that configuration.
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11() && IsIntel());

    wgpu::Texture sourceTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    RenderQuad(sourceTexture,
               {kGreen.rgbaFloats, kBlack.rgbaFloats, kRed.rgbaFloats, kBlue.rgbaFloats});

    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Control case to verify mirrored and rotation defaults
    {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Import the external texture
        wgpu::ExternalTextureDescriptor externalDesc = InitExternalTextureDescriptor(sourceTexture);
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlack, renderTexture, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderTexture, 0, 3);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, renderTexture, 3, 3);
    }

    struct RotationExpectation {
        wgpu::ExternalTextureRotation rotation;
        bool mirrored;
        utils::RGBA8 upperLeftColor;
        utils::RGBA8 upperRightColor;
        utils::RGBA8 lowerLeftColor;
        utils::RGBA8 lowerRightColor;
    };

    std::array<RotationExpectation, 8> expectations = {
        {{wgpu::ExternalTextureRotation::Rotate0Degrees, false, utils::RGBA8::kGreen,
          utils::RGBA8::kBlack, utils::RGBA8::kRed, utils::RGBA8::kBlue},
         {wgpu::ExternalTextureRotation::Rotate90Degrees, false, utils::RGBA8::kRed,
          utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kBlack},
         {wgpu::ExternalTextureRotation::Rotate180Degrees, false, utils::RGBA8::kBlue,
          utils::RGBA8::kRed, utils::RGBA8::kBlack, utils::RGBA8::kGreen},
         {wgpu::ExternalTextureRotation::Rotate270Degrees, false, utils::RGBA8::kBlack,
          utils::RGBA8::kBlue, utils::RGBA8::kGreen, utils::RGBA8::kRed},
         {wgpu::ExternalTextureRotation::Rotate0Degrees, true, utils::RGBA8::kBlack,
          utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kRed},
         {wgpu::ExternalTextureRotation::Rotate90Degrees, true, utils::RGBA8::kGreen,
          utils::RGBA8::kRed, utils::RGBA8::kBlack, utils::RGBA8::kBlue},
         {wgpu::ExternalTextureRotation::Rotate180Degrees, true, utils::RGBA8::kRed,
          utils::RGBA8::kBlue, utils::RGBA8::kGreen, utils::RGBA8::kBlack},
         {wgpu::ExternalTextureRotation::Rotate270Degrees, true, utils::RGBA8::kBlue,
          utils::RGBA8::kBlack, utils::RGBA8::kRed, utils::RGBA8::kGreen}}};

    for (const RotationExpectation& exp : expectations) {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Import the external texture
        wgpu::ExternalTextureDescriptor externalDesc = InitExternalTextureDescriptor(sourceTexture);
        externalDesc.rotation = exp.rotation;
        externalDesc.mirrored = exp.mirrored;
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 1, 1);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 2, 1);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 0, 3);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 1, 2);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 3, 3);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 2, 2);
    }
}

// Test for a bug found during review where the cropSize was not correctly rotated during the
// initialization of the ExternalTexture, which could lead to incorrect textureLoad operations when
// rotating 90 and 270 degrees.
TEST_P(ExternalTextureTests, RotateAndOrFlipTextureLoadSinglePlaneNotSquare) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::Texture sourceTexture =
        Create2DTexture(device, 2, 16, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    RenderQuad(sourceTexture,
               {kGreen.rgbaFloats, kBlack.rgbaFloats, kRed.rgbaFloats, kBlue.rgbaFloats});

    wgpu::Texture renderTexture = Create2DTexture(
        device, 2, 2, kFormat, wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture dimensionTexture =
        Create2DTexture(device, 2, 2, wgpu::TextureFormat::RG32Uint,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    struct RotationExpectation {
        wgpu::ExternalTextureRotation rotation;
        bool mirrored;
        utils::RGBA8 upperLeftColor;
        utils::RGBA8 upperRightColor;
        utils::RGBA8 lowerLeftColor;
        utils::RGBA8 lowerRightColor;
    };
    std::array<RotationExpectation, 8> expectations = {
        {{wgpu::ExternalTextureRotation::Rotate0Degrees, false, utils::RGBA8::kGreen,
          utils::RGBA8::kBlack, utils::RGBA8::kRed, utils::RGBA8::kBlue},
         {wgpu::ExternalTextureRotation::Rotate90Degrees, false, utils::RGBA8::kRed,
          utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kBlack},
         {wgpu::ExternalTextureRotation::Rotate180Degrees, false, utils::RGBA8::kBlue,
          utils::RGBA8::kRed, utils::RGBA8::kBlack, utils::RGBA8::kGreen},
         {wgpu::ExternalTextureRotation::Rotate270Degrees, false, utils::RGBA8::kBlack,
          utils::RGBA8::kBlue, utils::RGBA8::kGreen, utils::RGBA8::kRed},
         {wgpu::ExternalTextureRotation::Rotate0Degrees, true, utils::RGBA8::kBlack,
          utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kRed},
         {wgpu::ExternalTextureRotation::Rotate90Degrees, true, utils::RGBA8::kGreen,
          utils::RGBA8::kRed, utils::RGBA8::kBlack, utils::RGBA8::kBlue},
         {wgpu::ExternalTextureRotation::Rotate180Degrees, true, utils::RGBA8::kRed,
          utils::RGBA8::kBlue, utils::RGBA8::kGreen, utils::RGBA8::kBlack},
         {wgpu::ExternalTextureRotation::Rotate270Degrees, true, utils::RGBA8::kBlue,
          utils::RGBA8::kBlack, utils::RGBA8::kRed, utils::RGBA8::kGreen}}};

    wgpu::ShaderModule loadModule = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var t : texture_external;

        struct FragOut {
          @location(0) color: vec4f,
          @location(1) dimension: vec2u,
        };

        @fragment fn main(@builtin(position) FragCoord : vec4f)
                                 -> FragOut {
            let dimension = textureDimensions(t);
            let coords = textureDimensions(t) / 2 + vec2u(FragCoord.xy) - vec2(1, 1);
            return FragOut(textureLoad(t, coords), dimension);
        })");

    wgpu::BufferDescriptor dimensionBufferDesc;
    dimensionBufferDesc.size = 256;
    dimensionBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer dimensionBuffer = device.CreateBuffer(&dimensionBufferDesc);

    for (const RotationExpectation& exp : expectations) {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = loadModule;
        descriptor.cFragment.targetCount = 2;
        descriptor.cTargets[0].format = kFormat;
        descriptor.cTargets[1].format = wgpu::TextureFormat::RG32Uint;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Import the external texture and make the bindgroup.
        wgpu::ExternalTextureDescriptor externalDesc = InitExternalTextureDescriptor(sourceTexture);
        externalDesc.rotation = exp.rotation;
        externalDesc.mirrored = exp.mirrored;

        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        utils::ComboRenderPassDescriptor renderPass(
            {renderTexture.CreateView(), dimensionTexture.CreateView()});
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(6);
        pass.End();

        {
            wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
                utils::CreateTexelCopyTextureInfo(dimensionTexture, 0, {0, 0, 0});
            wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
                utils::CreateTexelCopyBufferInfo(dimensionBuffer, 0, 256, 1);
            wgpu::Extent3D size = {1, 1, 1};
            encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &size);
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 1, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 0, 1);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 1, 1);

        if (exp.rotation == wgpu::ExternalTextureRotation::Rotate90Degrees ||
            exp.rotation == wgpu::ExternalTextureRotation::Rotate270Degrees) {
            EXPECT_BUFFER_U32_EQ(sourceTexture.GetHeight(), dimensionBuffer, 0);
            EXPECT_BUFFER_U32_EQ(sourceTexture.GetWidth(), dimensionBuffer, 4);
        } else {
            EXPECT_BUFFER_U32_EQ(sourceTexture.GetWidth(), dimensionBuffer, 0);
            EXPECT_BUFFER_U32_EQ(sourceTexture.GetHeight(), dimensionBuffer, 4);
        }
    }
}

// Test draws a green square in the upper left quadrant, a black square in the upper right, a red
// square in the lower left and a blue square in the lower right. The image is then sampled as an
// external texture and rotated 0, 90, 180, and 270 degrees with and without the y-axis flipped.
TEST_P(ExternalTextureTests, RotateAndOrFlipSampleMultiplanar) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    // TODO(41487285): Fails on OpenGL ANGLE D3D11 Intel (but not other configs). Suppress since we
    // don't want to ship that configuration.
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11() && IsIntel());

    wgpu::Texture sourceTexturePlane0 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::R8Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture sourceTexturePlane1 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::RG8Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    RenderQuad(sourceTexturePlane0, {{kGreen.y}, {kBlack.y}, {kRed.y}, {kBlue.y}});
    RenderQuad(sourceTexturePlane1,
               {{kGreen.u, kGreen.v}, {kBlack.u, kBlack.v}, {kRed.u, kRed.v}, {kBlue.u, kBlue.v}});

    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // Control case to verify mirrored and rotation defaults
    {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Create an ExternalTextureDescriptor from the texture view
        wgpu::ExternalTextureDescriptor externalDesc =
            InitExternalTextureDescriptor(sourceTexturePlane0, sourceTexturePlane1);

        // Import the external texture
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kGreen, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlack, renderTexture, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kRed, renderTexture, 0, 3);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlue, renderTexture, 3, 3);
    }

    struct RotationExpectation {
        wgpu::ExternalTextureRotation rotation;
        bool mirrored;
        utils::RGBA8 upperLeftColor;
        utils::RGBA8 upperRightColor;
        utils::RGBA8 lowerLeftColor;
        utils::RGBA8 lowerRightColor;
    };

    std::array<RotationExpectation, 8> expectations = {
        {{wgpu::ExternalTextureRotation::Rotate0Degrees, false, utils::RGBA8::kGreen,
          utils::RGBA8::kBlack, utils::RGBA8::kRed, utils::RGBA8::kBlue},
         {wgpu::ExternalTextureRotation::Rotate90Degrees, false, utils::RGBA8::kRed,
          utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kBlack},
         {wgpu::ExternalTextureRotation::Rotate180Degrees, false, utils::RGBA8::kBlue,
          utils::RGBA8::kRed, utils::RGBA8::kBlack, utils::RGBA8::kGreen},
         {wgpu::ExternalTextureRotation::Rotate270Degrees, false, utils::RGBA8::kBlack,
          utils::RGBA8::kBlue, utils::RGBA8::kGreen, utils::RGBA8::kRed},
         {wgpu::ExternalTextureRotation::Rotate0Degrees, true, utils::RGBA8::kBlack,
          utils::RGBA8::kGreen, utils::RGBA8::kBlue, utils::RGBA8::kRed},
         {wgpu::ExternalTextureRotation::Rotate90Degrees, true, utils::RGBA8::kGreen,
          utils::RGBA8::kRed, utils::RGBA8::kBlack, utils::RGBA8::kBlue},
         {wgpu::ExternalTextureRotation::Rotate180Degrees, true, utils::RGBA8::kRed,
          utils::RGBA8::kBlue, utils::RGBA8::kGreen, utils::RGBA8::kBlack},
         {wgpu::ExternalTextureRotation::Rotate270Degrees, true, utils::RGBA8::kBlue,
          utils::RGBA8::kBlack, utils::RGBA8::kRed, utils::RGBA8::kGreen}}};

    for (const RotationExpectation& exp : expectations) {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Import the external texture
        wgpu::ExternalTextureDescriptor externalDesc =
            InitExternalTextureDescriptor(sourceTexturePlane0, sourceTexturePlane1);
        externalDesc.rotation = exp.rotation;
        externalDesc.mirrored = exp.mirrored;
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 1, 1);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 2, 1);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 0, 3);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 1, 2);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 3, 3);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 2, 2);
    }
}

// This test draws a 2x2 multi-colored square surrounded by a 1px black border. We test the external
// texture crop functionality by cropping to specific ranges inside the texture.
TEST_P(ExternalTextureTests, CropSinglePlane) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::Texture sourceTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    RenderQuad(sourceTexture, {kGreen.rgbaFloats, kColor1.rgbaFloats, kRed.rgbaFloats,
                               kBlue.rgbaFloats, kBlack.rgbaFloats, 0.5});

    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    struct CropExpectation {
        wgpu::Origin2D cropOrigin;
        wgpu::Extent2D cropSize;
        wgpu::ExternalTextureRotation rotation;
        utils::RGBA8 upperLeftColor;
        utils::RGBA8 upperRightColor;
        utils::RGBA8 lowerLeftColor;
        utils::RGBA8 lowerRightColor;
    };

    std::array<CropExpectation, 9> expectations = {{
        {{0, 0},
         {kWidth, kHeight},
         wgpu::ExternalTextureRotation::Rotate0Degrees,
         utils::RGBA8::kBlack,
         utils::RGBA8::kBlack,
         utils::RGBA8::kBlack,
         utils::RGBA8::kBlack},
        {{kWidth / 4, kHeight / 4},
         {kWidth / 4, kHeight / 4},
         wgpu::ExternalTextureRotation::Rotate0Degrees,
         utils::RGBA8::kGreen,
         utils::RGBA8::kGreen,
         utils::RGBA8::kGreen,
         utils::RGBA8::kGreen},
        {{kWidth / 2, kHeight / 4},
         {kWidth / 4, kHeight / 4},
         wgpu::ExternalTextureRotation::Rotate0Degrees,
         kColor1.rgba,
         kColor1.rgba,
         kColor1.rgba,
         kColor1.rgba},
        {{kWidth / 4, kHeight / 2},
         {kWidth / 4, kHeight / 4},
         wgpu::ExternalTextureRotation::Rotate0Degrees,
         utils::RGBA8::kRed,
         utils::RGBA8::kRed,
         utils::RGBA8::kRed,
         utils::RGBA8::kRed},
        {{kWidth / 2, kHeight / 2},
         {kWidth / 4, kHeight / 4},
         wgpu::ExternalTextureRotation::Rotate0Degrees,
         utils::RGBA8::kBlue,
         utils::RGBA8::kBlue,
         utils::RGBA8::kBlue,
         utils::RGBA8::kBlue},
        {{kWidth / 4, kHeight / 4},
         {kWidth / 2, kHeight / 2},
         wgpu::ExternalTextureRotation::Rotate0Degrees,
         utils::RGBA8::kGreen,
         kColor1.rgba,
         utils::RGBA8::kRed,
         utils::RGBA8::kBlue},
        {{kWidth / 4, kHeight / 4},
         {kWidth / 2, kHeight / 2},
         wgpu::ExternalTextureRotation::Rotate90Degrees,
         utils::RGBA8::kRed,
         utils::RGBA8::kGreen,
         utils::RGBA8::kBlue,
         kColor1.rgba},
        {{kWidth / 4, kHeight / 4},
         {kWidth / 2, kHeight / 2},
         wgpu::ExternalTextureRotation::Rotate180Degrees,
         utils::RGBA8::kBlue,
         utils::RGBA8::kRed,
         kColor1.rgba,
         utils::RGBA8::kGreen},
        {{kWidth / 4, kHeight / 4},
         {kWidth / 2, kHeight / 2},
         wgpu::ExternalTextureRotation::Rotate270Degrees,
         kColor1.rgba,
         utils::RGBA8::kBlue,
         utils::RGBA8::kGreen,
         utils::RGBA8::kRed},
    }};

    for (const CropExpectation& exp : expectations) {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Import the external texture
        wgpu::ExternalTextureDescriptor externalDesc = InitExternalTextureDescriptor(sourceTexture);
        externalDesc.rotation = exp.rotation;
        externalDesc.cropOrigin = exp.cropOrigin;
        externalDesc.cropSize = exp.cropSize;
        externalDesc.apparentSize = exp.cropSize;
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 0, 3);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 3, 3);
    }
}

// Test that the apparentSize takes effect by using it to scale a texture and "blitting" it.
TEST_P(ExternalTextureTests, ApparentSizeEffect) {
    // Create the test pipeline
    wgpu::ShaderModule blitAndOutputSize = utils::CreateShaderModule(this->device, R"(
        @group(0) @binding(0) var t : texture_external;

        struct FragOut {
          @location(0) color: vec4f,
          @location(1) dimension: vec2u,
        };

        @fragment fn main(@builtin(position) FragCoord : vec4f)
                                 -> FragOut {
            let dimensions = textureDimensions(t);
            return FragOut(textureLoad(t, vec2u(FragCoord.xy)), dimensions);
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = blitAndOutputSize;
    descriptor.cFragment.targetCount = 2;
    descriptor.cTargets[0].format = kFormat;
    descriptor.cTargets[1].format = wgpu::TextureFormat::RG32Uint;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // The source texture will contain a quad and have a larger apparent size.
    wgpu::Texture sourceTexture =
        Create2DTexture(device, 2, 2, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    RenderQuad(sourceTexture,
               {kGreen.rgbaFloats, kColor1.rgbaFloats, kRed.rgbaFloats, kBlue.rgbaFloats});

    wgpu::ExternalTextureDescriptor externalDesc = InitExternalTextureDescriptor(sourceTexture);
    externalDesc.apparentSize = {8, 16};
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // The texture that will receive the blit operation, uses apparentSize
    wgpu::Texture renderTexture = Create2DTexture(
        device, externalDesc.apparentSize.width, externalDesc.apparentSize.height, kFormat,
        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture dimensionTexture =
        Create2DTexture(device, externalDesc.apparentSize.width, externalDesc.apparentSize.height,
                        wgpu::TextureFormat::RG32Uint,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    // The buffer that will receive the result of `textureDimensions`
    wgpu::BufferDescriptor bufDesc;
    bufDesc.size = 256;
    bufDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer dimensionBuffer = device.CreateBuffer(&bufDesc);

    wgpu::BindGroup bindGroup =
        utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0), {{0, externalTexture}});

    // Do the blit
    utils::ComboRenderPassDescriptor renderPass(
        {renderTexture.CreateView(), dimensionTexture.CreateView()});
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    {
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(6);
        pass.End();
    }

    {
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dimensionTexture, 0, {0, 0, 0});
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(dimensionBuffer, 0, 256, 1);
        wgpu::Extent3D size = {1, 1, 1};
        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &size);
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Check that the apparentSize is correct and that the center of the quad has moved due to the
    // scaling with apparentSize.
    EXPECT_BUFFER_U32_EQ(externalDesc.apparentSize.width, dimensionBuffer, 0);
    EXPECT_BUFFER_U32_EQ(externalDesc.apparentSize.height, dimensionBuffer, 4);

    EXPECT_PIXEL_RGBA8_EQ(kGreen.rgba, renderTexture, 3, 7);
    EXPECT_PIXEL_RGBA8_EQ(kColor1.rgba, renderTexture, 4, 7);
    EXPECT_PIXEL_RGBA8_EQ(kRed.rgba, renderTexture, 3, 8);
    EXPECT_PIXEL_RGBA8_EQ(kBlue.rgba, renderTexture, 4, 8);
}

// This test draws a 2x2 multi-colored square surrounded by a 1px black border. We test the external
// texture crop functionality by cropping to specific ranges inside the texture.
TEST_P(ExternalTextureTests, CropMultiplanar) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::Texture sourceTexturePlane0 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::R8Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture sourceTexturePlane1 =
        Create2DTexture(device, kWidth, kHeight, wgpu::TextureFormat::RG8Unorm,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    RenderQuad(sourceTexturePlane0,
               {{kGreen.y}, {kColor1.y}, {kRed.y}, {kBlue.y}, {kBlack.y}, 0.5});
    RenderQuad(sourceTexturePlane1, {{kGreen.u, kGreen.v},
                                     {kColor1.u, kColor1.v},
                                     {kRed.u, kRed.v},
                                     {kBlue.u, kBlue.v},
                                     {kBlack.u, kBlack.v},
                                     0.5});

    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    struct CropExpectation {
        wgpu::Origin2D cropOrigin;
        wgpu::Extent2D cropSize;
        wgpu::ExternalTextureRotation rotation;
        utils::RGBA8 upperLeftColor;
        utils::RGBA8 upperRightColor;
        utils::RGBA8 lowerLeftColor;
        utils::RGBA8 lowerRightColor;
    };

    std::array<CropExpectation, 9> expectations = {
        {{{0, 0},
          {kWidth, kHeight},
          wgpu::ExternalTextureRotation::Rotate0Degrees,
          utils::RGBA8::kBlack,
          utils::RGBA8::kBlack,
          utils::RGBA8::kBlack,
          utils::RGBA8::kBlack},
         {{kWidth / 4, kHeight / 4},
          {kWidth / 4, kHeight / 4},
          wgpu::ExternalTextureRotation::Rotate0Degrees,
          utils::RGBA8::kGreen,
          utils::RGBA8::kGreen,
          utils::RGBA8::kGreen,
          utils::RGBA8::kGreen},
         {{kWidth / 2, kHeight / 4},
          {kWidth / 4, kHeight / 4},
          wgpu::ExternalTextureRotation::Rotate0Degrees,
          kColor1.rgba,
          kColor1.rgba,
          kColor1.rgba,
          kColor1.rgba},
         {{kWidth / 4, kHeight / 2},
          {kWidth / 4, kHeight / 4},
          wgpu::ExternalTextureRotation::Rotate0Degrees,
          utils::RGBA8::kRed,
          utils::RGBA8::kRed,
          utils::RGBA8::kRed,
          utils::RGBA8::kRed},
         {{kWidth / 2, kHeight / 2},
          {kWidth / 4, kHeight / 4},
          wgpu::ExternalTextureRotation::Rotate0Degrees,
          utils::RGBA8::kBlue,
          utils::RGBA8::kBlue,
          utils::RGBA8::kBlue,
          utils::RGBA8::kBlue},
         {{kWidth / 4, kHeight / 4},
          {kWidth / 2, kHeight / 2},
          wgpu::ExternalTextureRotation::Rotate0Degrees,
          utils::RGBA8::kGreen,
          kColor1.rgba,
          utils::RGBA8::kRed,
          utils::RGBA8::kBlue},
         {{kWidth / 4, kHeight / 4},
          {kWidth / 2, kHeight / 2},
          wgpu::ExternalTextureRotation::Rotate90Degrees,
          utils::RGBA8::kRed,
          utils::RGBA8::kGreen,
          utils::RGBA8::kBlue,
          kColor1.rgba},
         {{kWidth / 4, kHeight / 4},
          {kWidth / 2, kHeight / 2},
          wgpu::ExternalTextureRotation::Rotate180Degrees,
          utils::RGBA8::kBlue,
          utils::RGBA8::kRed,
          kColor1.rgba,
          utils::RGBA8::kGreen},
         {{kWidth / 4, kHeight / 4},
          {kWidth / 2, kHeight / 2},
          wgpu::ExternalTextureRotation::Rotate270Degrees,
          kColor1.rgba,
          utils::RGBA8::kBlue,
          utils::RGBA8::kGreen,
          utils::RGBA8::kRed}}};

    for (const CropExpectation& exp : expectations) {
        // Pipeline Creation
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsSampleExternalTextureModule;
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Import the external texture
        wgpu::ExternalTextureDescriptor externalDesc =
            InitExternalTextureDescriptor(sourceTexturePlane0, sourceTexturePlane1);
        externalDesc.rotation = exp.rotation;
        externalDesc.cropOrigin = exp.cropOrigin;
        externalDesc.cropSize = exp.cropSize;
        externalDesc.apparentSize = exp.cropSize;
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(exp.upperLeftColor, renderTexture, 0, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.upperRightColor, renderTexture, 3, 0);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerLeftColor, renderTexture, 0, 3);
        EXPECT_PIXEL_RGBA8_EQ(exp.lowerRightColor, renderTexture, 3, 3);
    }
}

// Test that sampling an external texture with non-one alpha preserves the alpha channel.
TEST_P(ExternalTextureTests, SampleExternalTextureAlpha) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::Texture sampledTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture renderTexture =
        Create2DTexture(device, kWidth, kHeight, kFormat,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

    utils::RGBA8 kColor = {255, 255, 255, 128};

    // Initialize texture with green to ensure it is sampled from later.
    {
        utils::ComboRenderPassDescriptor renderPass({sampledTexture.CreateView()}, nullptr);
        renderPass.cColorAttachments[0].clearValue = {kColor.r / 255.0f, kColor.g / 255.0f,
                                                      kColor.b / 255.0f, kColor.a / 255.0f};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
    }

    // Pipeline Creation
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = vsModule;
    descriptor.cFragment.module = fsSampleExternalTextureModule;
    descriptor.cTargets[0].format = kFormat;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    // Import the external texture
    wgpu::ExternalTextureDescriptor externalDesc = InitExternalTextureDescriptor(sampledTexture);
    wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

    // Create a sampler and bind group
    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                     {{0, sampler}, {1, externalTexture}});

    // Run the shader, which should sample from the external texture and draw a triangle into the
    // upper left corner of the render texture.
    wgpu::TextureView renderView = renderTexture.CreateView();
    utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    {
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_PIXEL_RGBA8_EQ(kColor, renderTexture, 0, 0);
}

// Test for crbug.com/dawn/2472
TEST_P(ExternalTextureTests, RemappingBugDawn2472) {
    auto wgslModule = utils::CreateShaderModule(device, R"(
    @vertex
    fn vertexMain() -> @builtin(position) vec4f {
      return vec4f(1);
    }

    @group(0) @binding(0) var myTexture: texture_external;

    @fragment
    fn fragmentMain() -> @location(0) vec4f {
      let result = textureLoad(myTexture, vec2u(1, 1));
      return vec4f(1);
    })");

    // Pipeline Creation
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = wgslModule;
    descriptor.cFragment.module = wgslModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    ASSERT_NE(pipeline.Get(), nullptr);
}

// Regression test for issue 346174896.
TEST_P(ExternalTextureTests, Regression346174896) {
    auto wgslModule = utils::CreateShaderModule(device, R"(
        @vertex fn vertexMain() -> @builtin(position) vec4f {
            return vec4f(1);
        }

        @group(0) @binding(1) var<uniform> dimension : vec2u;
        @group(0) @binding(0) var t : texture_external;

        @fragment fn main(@builtin(position) FragCoord : vec4f) -> @location(0) vec4f {
            _ = dimension;
            return textureLoad(t, vec2u(0, 0));
        })");

    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = wgslModule;
    descriptor.cFragment.module = wgslModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    ASSERT_NE(pipeline.Get(), nullptr);
}

TEST_P(ExternalTextureTests, MultipleBindings) {
    auto wgslModule = utils::CreateShaderModule(device, R"(
    @vertex
    fn vertexMain() -> @builtin(position) vec4f {
      return vec4f(1);
    }

    @group(0) @binding(0) var<uniform> u : f32;
    @group(0) @binding(1) var s : sampler;
    @group(0) @binding(2) var et : texture_external;

    @fragment
    fn main() -> @location(0) vec4f {
      return textureSampleBaseClampToEdge(et, s, vec2f(u));
    })");

    // Pipeline Creation
    utils::ComboRenderPipelineDescriptor descriptor;
    descriptor.vertex.module = wgslModule;
    descriptor.cFragment.module = wgslModule;
    wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

    ASSERT_NE(pipeline.Get(), nullptr);
}

DAWN_INSTANTIATE_TEST(ExternalTextureTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"d3d12_use_root_signature_version_1_1"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

enum class AccessMode { Sample, Load };
enum class OOBAccess { BelowBottomLeft, OverTopRight };
enum class Flip { None, Mirrored };

using Rotation = wgpu::ExternalTextureRotation;

std::ostream& operator<<(std::ostream& o, Flip flip) {
    switch (flip) {
        case Flip::None:
            o << "no flip ";
            break;
        case Flip::Mirrored:
            o << "mirrored ";
            break;
        default:
            DAWN_UNREACHABLE();
            break;
    }

    return o;
}

std::ostream& operator<<(std::ostream& o, AccessMode mode) {
    switch (mode) {
        case AccessMode::Sample:
            o << "sample ";
            break;
        case AccessMode::Load:
            o << "load ";
            break;
        default:
            DAWN_UNREACHABLE();
            break;
    }

    return o;
}

std::ostream& operator<<(std::ostream& o, OOBAccess rect) {
    switch (rect) {
        case OOBAccess::BelowBottomLeft:
            o << "below bottom left ";
            break;
        case OOBAccess::OverTopRight:
            o << "over top right";
            break;
        default:
            DAWN_UNREACHABLE();
            break;
    }

    return o;
}

DAWN_TEST_PARAM_STRUCT(OOBTestParams, Rotation, Flip, AccessMode, OOBAccess);

class ExternalTextureOOBTests : public ExternalTextureTestsBase<DawnTestWithParams<OOBTestParams>> {
  protected:
    void SetUp() override {
        ExternalTextureTestsBase<DawnTestWithParams<OOBTestParams>>::SetUp();
        sourceTexturePlane0 = Create2DTexture(
            device, kPlaneWidth, kPlaneHeight, wgpu::TextureFormat::R8Unorm,
            wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
        sourceTexturePlane1 = Create2DTexture(
            device, kPlaneWidth, kPlaneHeight, wgpu::TextureFormat::RG8Unorm,
            wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment);
        RenderQuad(sourceTexturePlane0,
                   {{kGreen.y}, {kColor1.y}, {kRed.y}, {kBlue.y}, {kBlack.y}, 0.5});
        RenderQuad(sourceTexturePlane1, {{kGreen.u, kGreen.v},
                                         {kColor1.u, kColor1.v},
                                         {kRed.u, kRed.v},
                                         {kBlue.u, kBlue.v},
                                         {kBlack.u, kBlack.v},
                                         0.5});

        renderTexture =
            Create2DTexture(device, kPlaneWidth, kPlaneHeight, kFormat,
                            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment);

        oobTestShaderModule = utils::CreateShaderModule(device, R"(
        @vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var positions = array(
                    vec4f(-1.0, 1.0, 0.0, 1.0),
                    vec4f(-1.0, -1.0, 0.0, 1.0),
                    vec4f(1.0, 1.0, 0.0, 1.0),
                    vec4f(1.0, -1.0, 0.0, 1.0),
                    vec4f(-1.0, -1.0, 0.0, 1.0),
                    vec4f(1.0, 1.0, 0.0, 1.0)
                );
                return positions[VertexIndex];
        }

        @group(0) @binding(0) var s : sampler;
        @group(0) @binding(1) var t : texture_external;

        @fragment fn sampleOverTopRight() -> @location(0) vec4f {
            return textureSampleBaseClampToEdge(t, s, vec2f(1.1, 1.1));
        }

        @fragment fn sampleBelowBottomLeft() -> @location(0) vec4f {
            return textureSampleBaseClampToEdge(t, s, vec2f(-0.1, -0.1));
        }

        @fragment fn loadOverTopRight() -> @location(0) vec4f {
            _ = textureSampleBaseClampToEdge(t, s, vec2f(0.0, 0.0));
            return textureLoad(t, vec2<i32>(5, 5));
        }

        @fragment fn loadBelowBottomLeft() -> @location(0) vec4f {
            _ = textureSampleBaseClampToEdge(t, s, vec2f(0.0, 0.0));
            return textureLoad(t, vec2<i32>(-1, -1));
        }

        )");
    }

    std::string GetEntryPoint(AccessMode mode, OOBAccess access) {
        switch (mode) {
            case AccessMode::Sample:
                switch (access) {
                    case OOBAccess::BelowBottomLeft:
                        return "sampleBelowBottomLeft";
                    case OOBAccess::OverTopRight:
                        return "sampleOverTopRight";
                    default:
                        DAWN_UNREACHABLE();
                }
                break;
            case AccessMode::Load:
                switch (access) {
                    case OOBAccess::BelowBottomLeft:
                        return "loadBelowBottomLeft";
                    case OOBAccess::OverTopRight:
                        return "loadOverTopRight";
                    default:
                        DAWN_UNREACHABLE();
                }
                break;
            default:
                DAWN_UNREACHABLE();
        }
    }

    void DoTest() {
        wgpu::ExternalTextureRotation rotation = GetParam().mRotation;
        OOBAccess oobAccess = GetParam().mOOBAccess;
        AccessMode accessMode = GetParam().mAccessMode;
        Flip flip = GetParam().mFlip;

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = oobTestShaderModule;
        descriptor.vertex.entryPoint = "main";
        descriptor.cFragment.module = oobTestShaderModule;
        std::string fragmentEntryPoint = GetEntryPoint(accessMode, oobAccess);

        descriptor.cFragment.entryPoint = fragmentEntryPoint.c_str();
        descriptor.cTargets[0].format = kFormat;
        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&descriptor);

        // Import the external texture
        wgpu::ExternalTextureDescriptor externalDesc =
            InitExternalTextureDescriptor(sourceTexturePlane0, sourceTexturePlane1);
        externalDesc.rotation = rotation;
        externalDesc.mirrored = flip == Flip::Mirrored;
        wgpu::ExternalTexture externalTexture = device.CreateExternalTexture(&externalDesc);

        // Create a sampler and bind group
        wgpu::Sampler sampler = device.CreateSampler();

        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, sampler}, {1, externalTexture}});

        // Run the shader, which should sample from the external texture and draw a triangle into
        // the upper left corner of the render texture.
        wgpu::TextureView renderView = renderTexture.CreateView();
        utils::ComboRenderPassDescriptor renderPass({renderView}, nullptr);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        {
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Border color is black.
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8::kBlack, renderTexture, 0, 0);
    }

    static constexpr uint32_t kPlaneWidth = 5;
    static constexpr uint32_t kPlaneHeight = 5;
    wgpu::ShaderModule oobTestShaderModule;
    wgpu::Texture sourceTexturePlane0;
    wgpu::Texture sourceTexturePlane1;
    wgpu::Texture renderTexture;
};

// Test for OOB access
TEST_P(ExternalTextureOOBTests, ExternalTextureOOB) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    DoTest();
}

DAWN_INSTANTIATE_TEST_P(
    ExternalTextureOOBTests,
    {D3D11Backend(), D3D12Backend(), D3D12Backend({}, {"d3d12_use_root_signature_version_1_1"}),
     MetalBackend(), OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
    std::vector<wgpu::ExternalTextureRotation>({wgpu::ExternalTextureRotation::Rotate0Degrees,
                                                wgpu::ExternalTextureRotation::Rotate90Degrees,
                                                wgpu::ExternalTextureRotation::Rotate180Degrees,
                                                wgpu::ExternalTextureRotation::Rotate270Degrees}),
    std::vector<Flip>({Flip::None, Flip::Mirrored}),
    std::vector<AccessMode>({AccessMode::Sample, AccessMode::Load}),
    std::vector<OOBAccess>({OOBAccess::BelowBottomLeft, OOBAccess::OverTopRight}));

}  // anonymous namespace
}  // namespace dawn
