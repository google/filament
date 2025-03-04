// Copyright 2019 The Dawn & Tint Authors
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

#include <algorithm>
#include <array>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/native/DawnNative.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

enum class PipelineMultisampleLoadOp {
    Ignore,
    ExpandResolveTarget,
    HasResolveTargetButLoadMultisampled,
};
using PipelineMultisampleLoadOps = std::array<PipelineMultisampleLoadOp, 16>;

class MultisampledRenderingTest : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        InitTexturesForTest();
    }

    void InitTexturesForTest() {
        mMultisampledColorTexture = CreateTextureForRenderAttachment(kColorFormat, kSampleCount);
        mMultisampledColorView = mMultisampledColorTexture.CreateView();
        mResolveTexture = CreateTextureForRenderAttachment(kColorFormat, 1);
        mResolveView = mResolveTexture.CreateView();

        mDepthStencilTexture = CreateTextureForRenderAttachment(kDepthStencilFormat, kSampleCount);
        mDepthStencilView = mDepthStencilTexture.CreateView();
    }

    wgpu::RenderPipeline CreateRenderPipelineWithOneOutputForTest(
        bool testDepth,
        uint32_t sampleMask = 0xFFFFFFFF,
        bool alphaToCoverageEnabled = false,
        bool flipTriangle = false,
        bool enableExpandResolveLoadOp = false) {
        const char* kFsOneOutputWithDepth = R"(
            struct U {
                color : vec4f,
                depth : f32,
            }
            @group(0) @binding(0) var<uniform> uBuffer : U;

            struct FragmentOut {
                @location(0) color : vec4f,
                @builtin(frag_depth) depth : f32,
            }

            @fragment fn main() -> FragmentOut {
                var output : FragmentOut;
                output.color = uBuffer.color;
                output.depth = uBuffer.depth;
                return output;
            })";

        const char* kFsOneOutputWithoutDepth = R"(
            struct U {
                color : vec4f
            }
            @group(0) @binding(0) var<uniform> uBuffer : U;

            @fragment fn main() -> @location(0) vec4f {
                return uBuffer.color;
            })";

        const char* fs = testDepth ? kFsOneOutputWithDepth : kFsOneOutputWithoutDepth;

        PipelineMultisampleLoadOps multisampleLoadOps{};
        if (enableExpandResolveLoadOp) {
            multisampleLoadOps[0] = PipelineMultisampleLoadOp::ExpandResolveTarget;
        }
        return CreateRenderPipelineForTest(fs, 1, testDepth, sampleMask, alphaToCoverageEnabled,
                                           flipTriangle, multisampleLoadOps);
    }

    wgpu::RenderPipeline CreateRenderPipelineWithTwoOutputsForTest(
        uint32_t sampleMask = 0xFFFFFFFF,
        bool alphaToCoverageEnabled = false,
        bool depthTest = false,
        PipelineMultisampleLoadOp loadOpForColor0 = PipelineMultisampleLoadOp::Ignore,
        PipelineMultisampleLoadOp loadOpForColor1 = PipelineMultisampleLoadOp::Ignore) {
        const char* kFsTwoOutputs = R"(
            struct U {
                color0 : vec4f,
                color1 : vec4f,
            }
            @group(0) @binding(0) var<uniform> uBuffer : U;

            struct FragmentOut {
                @location(0) color0 : vec4f,
                @location(1) color1 : vec4f,
            }

            @fragment fn main() -> FragmentOut {
                var output : FragmentOut;
                output.color0 = uBuffer.color0;
                output.color1 = uBuffer.color1;
                return output;
            })";

        PipelineMultisampleLoadOps multisampleLoadOps{};
        multisampleLoadOps[0] = loadOpForColor0;
        multisampleLoadOps[1] = loadOpForColor1;

        return CreateRenderPipelineForTest(kFsTwoOutputs, 2, depthTest, sampleMask,
                                           alphaToCoverageEnabled, /*flipTriangle=*/false,
                                           multisampleLoadOps);
    }

    wgpu::RenderPipeline CreateRenderPipelineWithNonZeroLocationOutputForTest(
        uint32_t sampleMask = 0xFFFFFFFF,
        bool alphaToCoverageEnabled = false,
        bool enableExpandResolveLoadOp = false) {
        const char* kFsNonZeroLocationOutputs = R"(
            struct U {
                color : vec4f
            }
            @group(0) @binding(0) var<uniform> uBuffer : U;

            @fragment fn main() -> @location(1) vec4f {
                return uBuffer.color;
            })";

        PipelineMultisampleLoadOps multisampleLoadOps{};
        if (enableExpandResolveLoadOp) {
            multisampleLoadOps[1] = PipelineMultisampleLoadOp::ExpandResolveTarget;
        }
        return CreateRenderPipelineForTest(kFsNonZeroLocationOutputs, 1, false, sampleMask,
                                           alphaToCoverageEnabled, /*flipTriangle=*/false,
                                           multisampleLoadOps, 1);
    }

    wgpu::Texture CreateTextureForRenderAttachment(wgpu::TextureFormat format,
                                                   uint32_t sampleCount,
                                                   uint32_t mipLevelCount = 1,
                                                   uint32_t arrayLayerCount = 1,
                                                   bool transientAttachment = false,
                                                   bool supportsTextureBinding = false) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = kWidth << (mipLevelCount - 1);
        descriptor.size.height = kHeight << (mipLevelCount - 1);
        descriptor.size.depthOrArrayLayers = arrayLayerCount;
        descriptor.sampleCount = sampleCount;
        descriptor.format = format;
        descriptor.mipLevelCount = mipLevelCount;
        if (transientAttachment) {
            descriptor.usage =
                wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TransientAttachment;
        } else {
            descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        }

        if (supportsTextureBinding) {
            descriptor.usage |= wgpu::TextureUsage::TextureBinding;
        }

        return device.CreateTexture(&descriptor);
    }

    void EncodeRenderPassForTest(wgpu::CommandEncoder commandEncoder,
                                 const wgpu::RenderPassDescriptor& renderPass,
                                 const wgpu::RenderPipeline& pipeline,
                                 const float* uniformData,
                                 uint32_t uniformDataSize) {
        wgpu::Buffer uniformBuffer = utils::CreateBufferFromData(
            device, uniformData, uniformDataSize, wgpu::BufferUsage::Uniform);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                         {{0, uniformBuffer, 0, uniformDataSize}});

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(pipeline);
        renderPassEncoder.SetBindGroup(0, bindGroup);
        renderPassEncoder.Draw(3);
        renderPassEncoder.End();
    }

    void EncodeRenderPassForTest(wgpu::CommandEncoder commandEncoder,
                                 const wgpu::RenderPassDescriptor& renderPass,
                                 const wgpu::RenderPipeline& pipeline,
                                 const wgpu::Color& color) {
        const float uniformData[4] = {static_cast<float>(color.r), static_cast<float>(color.g),
                                      static_cast<float>(color.b), static_cast<float>(color.a)};
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, uniformData,
                                sizeof(float) * 4);
    }

    utils::ComboRenderPassDescriptor CreateComboRenderPassDescriptorForTest(
        std::initializer_list<wgpu::TextureView> colorViews,
        std::initializer_list<wgpu::TextureView> resolveTargetViews,
        wgpu::LoadOp colorLoadOp,
        wgpu::LoadOp depthStencilLoadOp,
        bool hasDepthStencilAttachment) {
        DAWN_ASSERT(colorViews.size() == resolveTargetViews.size());

        constexpr float kClearDepth = 1.0f;

        utils::ComboRenderPassDescriptor renderPass(colorViews);
        uint32_t i = 0;
        for (const wgpu::TextureView& resolveTargetView : resolveTargetViews) {
            renderPass.cColorAttachments[i].loadOp = colorLoadOp;
            renderPass.cColorAttachments[i].clearValue = kClearColor;
            renderPass.cColorAttachments[i].resolveTarget = resolveTargetView;
            ++i;
        }

        renderPass.cDepthStencilAttachmentInfo.depthClearValue = kClearDepth;
        renderPass.cDepthStencilAttachmentInfo.depthLoadOp = depthStencilLoadOp;

        if (hasDepthStencilAttachment) {
            renderPass.cDepthStencilAttachmentInfo.view = mDepthStencilView;
            renderPass.depthStencilAttachment = &renderPass.cDepthStencilAttachmentInfo;
        }

        return renderPass;
    }

    void VerifyResolveTarget(const wgpu::Color& inputColor,
                             wgpu::Texture resolveTexture,
                             uint32_t mipmapLevel = 0,
                             uint32_t arrayLayer = 0,
                             const float msaaCoverage = 0.5f,
                             uint32_t x = (kWidth - 1) / 2,
                             uint32_t y = (kHeight - 1) / 2) {
        utils::RGBA8 expectedColor = ExpectedMSAAColor(inputColor, msaaCoverage);
        EXPECT_TEXTURE_EQ(&expectedColor, resolveTexture, {x, y, arrayLayer}, {1, 1}, mipmapLevel,
                          wgpu::TextureAspect::All, /* bytesPerRow */ 0,
                          /* tolerance */ utils::RGBA8(1, 1, 1, 1));
    }

    constexpr static uint32_t kWidth = 3;
    constexpr static uint32_t kHeight = 3;
    constexpr static uint32_t kSampleCount = 4;
    constexpr static wgpu::TextureFormat kColorFormat = wgpu::TextureFormat::RGBA8Unorm;
    constexpr static wgpu::TextureFormat kDepthStencilFormat =
        wgpu::TextureFormat::Depth24PlusStencil8;

    constexpr static uint32_t kFirstSampleMaskBit = 0x00000001;
    constexpr static uint32_t kSecondSampleMaskBit = 0x00000002;
    constexpr static uint32_t kThirdSampleMaskBit = 0x00000004;
    constexpr static uint32_t kFourthSampleMaskBit = 0x00000008;

    constexpr static wgpu::Color kClearColor = {0.0f, 0.0f, 0.0f, 0.0f};

    wgpu::Texture mMultisampledColorTexture;
    wgpu::TextureView mMultisampledColorView;
    wgpu::Texture mResolveTexture;
    wgpu::TextureView mResolveView;
    wgpu::Texture mDepthStencilTexture;
    wgpu::TextureView mDepthStencilView;

    wgpu::RenderPipeline CreateRenderPipelineForTest(
        const char* fs,
        uint32_t numColorAttachments,
        bool hasDepthStencilAttachment,
        uint32_t sampleMask = 0xFFFFFFFF,
        bool alphaToCoverageEnabled = false,
        bool flipTriangle = false,
        PipelineMultisampleLoadOps multisampleLoadOps = {},
        uint32_t firstAttachmentLocation = 0) {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;

        // Draw a bottom-right triangle. In standard 4xMSAA pattern, for the pixels on diagonal,
        // only two of the samples will be touched.
        const char* vs = R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0,  1.0),
                    vec2f( 1.0,  1.0),
                    vec2f( 1.0, -1.0)
                );
                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })";

        // Draw a bottom-left triangle.
        const char* vsFlipped = R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
                var pos = array(
                    vec2f(-1.0,  1.0),
                    vec2f( 1.0,  1.0),
                    vec2f(-1.0, -1.0)
                );
                return vec4f(pos[VertexIndex], 0.0, 1.0);
            })";

        if (flipTriangle) {
            pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, vsFlipped);
        } else {
            pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, vs);
        }

        pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, fs);

        if (hasDepthStencilAttachment) {
            wgpu::DepthStencilState* depthStencil =
                pipelineDescriptor.EnableDepthStencil(kDepthStencilFormat);
            depthStencil->depthWriteEnabled = wgpu::OptionalBool::True;
            depthStencil->depthCompare = wgpu::CompareFunction::Less;
        }

        pipelineDescriptor.multisample.count = kSampleCount;
        pipelineDescriptor.multisample.mask = sampleMask;
        pipelineDescriptor.multisample.alphaToCoverageEnabled = alphaToCoverageEnabled;

        pipelineDescriptor.cFragment.targetCount = numColorAttachments + firstAttachmentLocation;

        std::array<wgpu::ColorTargetStateExpandResolveTextureDawn, 16> msaaExpandResolveDescs;
        for (uint32_t i = 0; i < numColorAttachments + firstAttachmentLocation; ++i) {
            if (i < firstAttachmentLocation) {
                pipelineDescriptor.cTargets[i].writeMask = wgpu::ColorWriteMask::None;
                pipelineDescriptor.cTargets[i].format = wgpu::TextureFormat::Undefined;
            } else {
                pipelineDescriptor.cTargets[i].format = kColorFormat;
                if (multisampleLoadOps[i] != PipelineMultisampleLoadOp::Ignore) {
                    msaaExpandResolveDescs[i].enabled =
                        multisampleLoadOps[i] == PipelineMultisampleLoadOp::ExpandResolveTarget;
                    pipelineDescriptor.cTargets[i].nextInChain = &msaaExpandResolveDescs[i];
                }
            }
        }

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);
        return pipeline;
    }

    utils::RGBA8 ExpectedMSAAColor(const wgpu::Color color, const double msaaCoverage) {
        utils::RGBA8 result;
        result.r = static_cast<uint8_t>(std::min(255.0, 256 * color.r * msaaCoverage));
        result.g = static_cast<uint8_t>(std::min(255.0, 256 * color.g * msaaCoverage));
        result.b = static_cast<uint8_t>(std::min(255.0, 256 * color.b * msaaCoverage));
        result.a = static_cast<uint8_t>(std::min(255.0, 256 * color.a * msaaCoverage));
        return result;
    }
};

// Test using one multisampled color attachment with resolve target can render correctly.
TEST_P(MultisampledRenderingTest, ResolveInto2DTexture) {
    constexpr bool kTestDepth = false;
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // storeOp should not affect the result in the resolve target.
    for (wgpu::StoreOp storeOp : {wgpu::StoreOp::Store, wgpu::StoreOp::Discard}) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

        // Draw a green triangle.
        {
            utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
                {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
                kTestDepth);
            renderPass.cColorAttachments[0].storeOp = storeOp;
            std::array<float, 4> kUniformData = {kGreen.r, kGreen.g, kGreen.b, kGreen.a};
            constexpr uint32_t kSize = sizeof(kUniformData);
            EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(),
                                    kSize);
        }

        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
        queue.Submit(1, &commandBuffer);

        VerifyResolveTarget(kGreen, mResolveTexture);
    }
}

// Test multisampled rendering with depth test works correctly.
TEST_P(MultisampledRenderingTest, MultisampledRenderingWithDepthTest) {
    constexpr bool kTestDepth = true;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};

    // In first render pass we draw a green triangle with depth value == 0.2f.
    {
        utils::ComboRenderPassDescriptor renderPass =
            CreateComboRenderPassDescriptorForTest({mMultisampledColorView}, {mResolveView},
                                                   wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, true);
        std::array<float, 8> kUniformData = {kGreen.r, kGreen.g, kGreen.b, kGreen.a,  // Color
                                             0.2f};                                   // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    // In second render pass we draw a red triangle with depth value == 0.5f.
    // This red triangle should not be displayed because it is behind the green one that is drawn in
    // the last render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            kTestDepth);

        std::array<float, 8> kUniformData = {kRed.r, kRed.g, kRed.b, kRed.a,  // color
                                             0.5f};                           // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    // The color of the pixel in the middle of mResolveTexture should be green if MSAA resolve runs
    // correctly with depth test.
    VerifyResolveTarget(kGreen, mResolveTexture);
}

// Test rendering into a multisampled color attachment and doing MSAA resolve in another render pass
// works correctly.
TEST_P(MultisampledRenderingTest, ResolveInAnotherRenderPass) {
    // TODO(dawn:1549) Fails on Qualcomm-based Android devices.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsQualcomm());

    constexpr bool kTestDepth = false;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // In first render pass we draw a green triangle and do not set the resolve target.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {nullptr}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    // In second render pass we ony do MSAA resolve with no draw call.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            kTestDepth);

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, mResolveTexture);
}

// Test doing MSAA resolve into multiple resolve targets works correctly.
TEST_P(MultisampledRenderingTest, ResolveIntoMultipleResolveTargets) {
    // TODO(dawn:1550) Workaround introduces a bug on Qualcomm GPUs, but is necessary for ARM GPUs.
    DAWN_TEST_UNSUPPORTED_IF(IsAndroid() && IsQualcomm() &&
                             HasToggleEnabled("resolve_multiple_attachments_in_separate_passes"));

    wgpu::TextureView multisampledColorView2 =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount).CreateView();
    wgpu::Texture resolveTexture2 = CreateTextureForRenderAttachment(kColorFormat, 1);
    wgpu::TextureView resolveView2 = resolveTexture2.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithTwoOutputsForTest();

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};
    constexpr bool kTestDepth = false;

    // Draw a red triangle to the first color attachment, and a blue triangle to the second color
    // attachment, and do MSAA resolve on two render targets in one render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView, multisampledColorView2}, {mResolveView, resolveView2},
            wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, kTestDepth);

        std::array<float, 8> kUniformData = {
            static_cast<float>(kRed.r),   static_cast<float>(kRed.g),
            static_cast<float>(kRed.b),   static_cast<float>(kRed.a),
            static_cast<float>(kGreen.r), static_cast<float>(kGreen.g),
            static_cast<float>(kGreen.b), static_cast<float>(kGreen.a)};
        constexpr uint32_t kSize = sizeof(kUniformData);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kRed, mResolveTexture);
    VerifyResolveTarget(kGreen, resolveTexture2);
}

// Test that resolving only one of multiple MSAA targets works correctly. (dawn:1550)
TEST_P(MultisampledRenderingTest, ResolveOneOfMultipleTargets) {
    // TODO(dawn:1550) Workaround introduces a bug on Qualcomm GPUs, but is necessary for ARM GPUs.
    DAWN_TEST_UNSUPPORTED_IF(IsAndroid() && IsQualcomm() &&
                             HasToggleEnabled("resolve_multiple_attachments_in_separate_passes"));

    wgpu::TextureView multisampledColorView2 =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount).CreateView();

    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithTwoOutputsForTest();

    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr bool kTestDepth = false;

    std::array<float, 8> kUniformData = {
        static_cast<float>(kRed.r),   static_cast<float>(kRed.g),   static_cast<float>(kRed.b),
        static_cast<float>(kRed.a),   static_cast<float>(kGreen.r), static_cast<float>(kGreen.g),
        static_cast<float>(kGreen.b), static_cast<float>(kGreen.a)};
    constexpr uint32_t kSize = sizeof(kUniformData);

    // Draws a red triangle to the first color attachment, and a blue triangle to the second color
    // attachment.

    // Do MSAA resolve on the first render target.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView, multisampledColorView2}, {mResolveView, nullptr},
            wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);

        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
        queue.Submit(1, &commandBuffer);

        VerifyResolveTarget(kRed, mResolveTexture);
    }

    // Do MSAA resolve on the second render target.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView, multisampledColorView2}, {nullptr, mResolveView},
            wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);

        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
        queue.Submit(1, &commandBuffer);

        VerifyResolveTarget(kGreen, mResolveTexture);
    }
}

// Test that resolving a single render target at a non-zero location works correctly.
TEST_P(MultisampledRenderingTest, ResolveIntoNonZeroLocation) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::TextureView multisampledColorView2 =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount).CreateView();

    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithNonZeroLocationOutputForTest();

    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};
    constexpr bool kTestDepth = false;

    // Draws a red triangle to the first color attachment, and a blue triangle to the second color
    // attachment.

    // Do MSAA resolve on the first render target.
    {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {nullptr, mMultisampledColorView}, {nullptr, mResolveView}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear, kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kRed);

        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
        queue.Submit(1, &commandBuffer);

        VerifyResolveTarget(kRed, mResolveTexture);
    }
}

// Test doing MSAA resolve on one multisampled texture twice works correctly.
TEST_P(MultisampledRenderingTest, ResolveOneMultisampledTextureTwice) {
    // TODO(dawn:1549) Fails on Qualcomm-based Android devices.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsQualcomm());

    constexpr bool kTestDepth = false;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    wgpu::Texture resolveTexture2 = CreateTextureForRenderAttachment(kColorFormat, 1);

    // In first render pass we draw a green triangle and specify mResolveView as the resolve target.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    // In second render pass we do MSAA resolve into resolveTexture2.
    {
        wgpu::TextureView resolveView2 = resolveTexture2.CreateView();
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {resolveView2}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            kTestDepth);

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, mResolveTexture);
    VerifyResolveTarget(kGreen, resolveTexture2);
}

// Test using a layer of a 2D texture as resolve target works correctly.
TEST_P(MultisampledRenderingTest, ResolveIntoOneMipmapLevelOf2DTexture) {
    constexpr uint32_t kBaseMipLevel = 2;

    wgpu::TextureViewDescriptor textureViewDescriptor;
    textureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    textureViewDescriptor.format = kColorFormat;
    textureViewDescriptor.baseArrayLayer = 0;
    textureViewDescriptor.arrayLayerCount = 1;
    textureViewDescriptor.mipLevelCount = 1;
    textureViewDescriptor.baseMipLevel = kBaseMipLevel;

    wgpu::Texture resolveTexture =
        CreateTextureForRenderAttachment(kColorFormat, 1, kBaseMipLevel + 1, 1);
    wgpu::TextureView resolveView = resolveTexture.CreateView(&textureViewDescriptor);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr bool kTestDepth = false;

    // Draw a green triangle and do MSAA resolve.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {resolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, resolveTexture, kBaseMipLevel, 0);
}

// Test using a level or a layer of a 2D array texture as resolve target works correctly.
TEST_P(MultisampledRenderingTest, ResolveInto2DArrayTexture) {
    // TODO(dawn:1550) Workaround introduces a bug on Qualcomm GPUs, but is necessary for ARM GPUs.
    DAWN_TEST_UNSUPPORTED_IF(IsAndroid() && IsQualcomm() &&
                             HasToggleEnabled("resolve_multiple_attachments_in_separate_passes"));

    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    wgpu::TextureView multisampledColorView2 =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount).CreateView();

    wgpu::TextureViewDescriptor baseTextureViewDescriptor;
    baseTextureViewDescriptor.dimension = wgpu::TextureViewDimension::e2D;
    baseTextureViewDescriptor.format = kColorFormat;
    baseTextureViewDescriptor.arrayLayerCount = 1;
    baseTextureViewDescriptor.mipLevelCount = 1;

    // Create resolveTexture1 with only 1 mipmap level.
    constexpr uint32_t kBaseArrayLayer1 = 2;
    constexpr uint32_t kBaseMipLevel1 = 0;
    wgpu::Texture resolveTexture1 =
        CreateTextureForRenderAttachment(kColorFormat, 1, kBaseMipLevel1 + 1, kBaseArrayLayer1 + 1);
    wgpu::TextureViewDescriptor resolveViewDescriptor1 = baseTextureViewDescriptor;
    resolveViewDescriptor1.baseArrayLayer = kBaseArrayLayer1;
    resolveViewDescriptor1.baseMipLevel = kBaseMipLevel1;
    wgpu::TextureView resolveView1 = resolveTexture1.CreateView(&resolveViewDescriptor1);

    // Create resolveTexture2 with (kBaseMipLevel2 + 1) mipmap levels and resolve into its last
    // mipmap level.
    constexpr uint32_t kBaseArrayLayer2 = 5;
    constexpr uint32_t kBaseMipLevel2 = 3;
    wgpu::Texture resolveTexture2 =
        CreateTextureForRenderAttachment(kColorFormat, 1, kBaseMipLevel2 + 1, kBaseArrayLayer2 + 1);
    wgpu::TextureViewDescriptor resolveViewDescriptor2 = baseTextureViewDescriptor;
    resolveViewDescriptor2.baseArrayLayer = kBaseArrayLayer2;
    resolveViewDescriptor2.baseMipLevel = kBaseMipLevel2;
    wgpu::TextureView resolveView2 = resolveTexture2.CreateView(&resolveViewDescriptor2);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithTwoOutputsForTest();

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};
    constexpr bool kTestDepth = false;

    // Draw a red triangle to the first color attachment, and a green triangle to the second color
    // attachment, and do MSAA resolve on two render targets in one render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView, multisampledColorView2}, {resolveView1, resolveView2},
            wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, kTestDepth);

        std::array<float, 8> kUniformData = {kRed.r,   kRed.g,   kRed.b,   kRed.a,     // color1
                                             kGreen.r, kGreen.g, kGreen.b, kGreen.a};  // color2
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kRed, resolveTexture1, kBaseMipLevel1, kBaseArrayLayer1);
    VerifyResolveTarget(kGreen, resolveTexture2, kBaseMipLevel2, kBaseArrayLayer2);
}

// Test using one multisampled color attachment with resolve target can render correctly
// with a non-default sample mask.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithSampleMask) {
    constexpr bool kTestDepth = false;
    // The second and third samples are included,
    // only the second one is covered by the triangle.
    constexpr uint32_t kSampleMask = kSecondSampleMaskBit | kThirdSampleMaskBit;
    constexpr float kMSAACoverage = 0.25f;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline =
        CreateRenderPipelineWithOneOutputForTest(kTestDepth, kSampleMask);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // Draw a green triangle.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, mResolveTexture, 0, 0, kMSAACoverage);
}

// Test using one multisampled color attachment with resolve target can render correctly
// with the final sample mask empty.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithEmptyFinalSampleMask) {
    constexpr bool kTestDepth = false;
    // The third and fourth samples are included,
    // none of which is covered by the triangle.
    constexpr uint32_t kSampleMask = kThirdSampleMaskBit | kFourthSampleMaskBit;
    constexpr float kMSAACoverage = 0.00f;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline =
        CreateRenderPipelineWithOneOutputForTest(kTestDepth, kSampleMask);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // Draw a green triangle.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, mResolveTexture, 0, 0, kMSAACoverage);
}

// Test doing MSAA resolve into multiple resolve targets works correctly with a non-default sample
// mask.
TEST_P(MultisampledRenderingTest, ResolveIntoMultipleResolveTargetsWithSampleMask) {
    /// TODO(dawn:1550) Workaround introduces a bug on Qualcomm GPUs, but is necessary for ARM GPUs.
    DAWN_TEST_UNSUPPORTED_IF(IsAndroid() && IsQualcomm() &&
                             HasToggleEnabled("resolve_multiple_attachments_in_separate_passes"));

    wgpu::TextureView multisampledColorView2 =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount).CreateView();
    wgpu::Texture resolveTexture2 = CreateTextureForRenderAttachment(kColorFormat, 1);
    wgpu::TextureView resolveView2 = resolveTexture2.CreateView();

    // The first and fourth samples are included,
    // only the first one is covered by the triangle.
    constexpr uint32_t kSampleMask = kFirstSampleMaskBit | kFourthSampleMaskBit;
    constexpr float kMSAACoverage = 0.25f;

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithTwoOutputsForTest(kSampleMask);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};
    constexpr bool kTestDepth = false;

    // Draw a red triangle to the first color attachment, and a blue triangle to the second color
    // attachment, and do MSAA resolve on two render targets in one render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView, multisampledColorView2}, {mResolveView, resolveView2},
            wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, kTestDepth);

        std::array<float, 8> kUniformData = {kRed.r,   kRed.g,   kRed.b,   kRed.a,     // color1
                                             kGreen.r, kGreen.g, kGreen.b, kGreen.a};  // color2
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    // TODO(crbug.com/dawn/1462): Work around that a sample mask of zero is used for all
    // color targets except the last one.
    VerifyResolveTarget(
        HasToggleEnabled("no_workaround_sample_mask_becomes_zero_for_all_but_last_color_target")
            ? wgpu::Color{}
            : kRed,
        mResolveTexture, 0, 0, kMSAACoverage);
    VerifyResolveTarget(kGreen, resolveTexture2, 0, 0, kMSAACoverage);
}

// Test multisampled rendering with depth test works correctly with a non-default sample mask.
TEST_P(MultisampledRenderingTest, MultisampledRenderingWithDepthTestAndSampleMask) {
    // TODO(dawn:1549) Fails on Qualcomm-based Android devices.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsQualcomm());

    constexpr bool kTestDepth = true;
    // The second sample is included in the first render pass and it's covered by the triangle.
    constexpr uint32_t kSampleMaskGreen = kSecondSampleMaskBit;
    // The first and second samples are included in the second render pass,
    // both are covered by the triangle.
    constexpr uint32_t kSampleMaskRed = kFirstSampleMaskBit | kSecondSampleMaskBit;
    constexpr float kMSAACoverage = 0.50f;

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipelineGreen =
        CreateRenderPipelineWithOneOutputForTest(kTestDepth, kSampleMaskGreen);
    wgpu::RenderPipeline pipelineRed =
        CreateRenderPipelineWithOneOutputForTest(kTestDepth, kSampleMaskRed);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};

    // In first render pass we draw a green triangle with depth value == 0.2f.
    // We will only write to the second sample.
    {
        utils::ComboRenderPassDescriptor renderPass =
            CreateComboRenderPassDescriptorForTest({mMultisampledColorView}, {mResolveView},
                                                   wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, true);
        std::array<float, 8> kUniformData = {kGreen.r, kGreen.g, kGreen.b, kGreen.a,  // Color
                                             0.2f};                                   // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipelineGreen, kUniformData.data(),
                                kSize);
    }

    // In second render pass we draw a red triangle with depth value == 0.5f.
    // We will only write to the first sample, since the second one is red with a smaller depth
    // value.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            kTestDepth);

        std::array<float, 8> kUniformData = {kRed.r, kRed.g, kRed.b, kRed.a,  // color
                                             0.5f};                           // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipelineRed, kUniformData.data(),
                                kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    constexpr wgpu::Color kHalfGreenHalfRed = {(kGreen.r + kRed.r) / 2.0, (kGreen.g + kRed.g) / 2.0,
                                               (kGreen.b + kRed.b) / 2.0,
                                               (kGreen.a + kRed.a) / 2.0};

    // The color of the pixel in the middle of mResolveTexture should be half green and half
    // red if MSAA resolve runs correctly with depth test.
    VerifyResolveTarget(kHalfGreenHalfRed, mResolveTexture, 0, 0, kMSAACoverage);
}

// Test using one multisampled color attachment with resolve target can render correctly
// with non-default sample mask and shader-output mask.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithSampleMaskAndShaderOutputMask) {
    // sample_mask is not supported in compat.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // TODO(crbug.com/dawn/673): Work around or enforce via validation that sample variables are not
    // supported on some platforms.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_sample_variables"));

    constexpr bool kTestDepth = false;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    // The second and third samples are included in the shader-output mask.
    // The first and third samples are included in the sample mask.
    // Since we're now looking at a fully covered pixel, the rasterization mask
    // includes all the samples.
    // Thus the final mask includes only the third sample.
    constexpr float kMSAACoverage = 0.25f;
    constexpr uint32_t kSampleMask = kFirstSampleMaskBit | kThirdSampleMaskBit;
    const char* fs = R"(
        struct U {
            color : vec4f
        }
        @group(0) @binding(0) var<uniform> uBuffer : U;

        struct FragmentOut {
            @location(0) color : vec4f,
            @builtin(sample_mask) sampleMask : u32,
        }

        @fragment fn main() -> FragmentOut {
            var output : FragmentOut;
            output.color = uBuffer.color;
            output.sampleMask = 6u;
            return output;
        })";

    wgpu::RenderPipeline pipeline = CreateRenderPipelineForTest(fs, 1, false, kSampleMask);
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // Draw a green triangle.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    utils::RGBA8 expectedColor = ExpectedMSAAColor(kGreen, kMSAACoverage);
    EXPECT_TEXTURE_EQ(&expectedColor, mResolveTexture, {1, 0}, {1, 1});
}

// Test doing MSAA resolve into multiple resolve targets works correctly with a non-default
// shader-output mask.
TEST_P(MultisampledRenderingTest, ResolveIntoMultipleResolveTargetsWithShaderOutputMask) {
    // sample_mask is not supported in compat.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    // TODO(crbug.com/dawn/673): Work around or enforce via validation that sample variables are not
    // supported on some platforms.
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("disable_sample_variables"));

    // TODO(dawn:1550) Workaround introduces a bug on Qualcomm GPUs, but is necessary for ARM GPUs.
    DAWN_TEST_UNSUPPORTED_IF(IsAndroid() && IsQualcomm() &&
                             HasToggleEnabled("resolve_multiple_attachments_in_separate_passes"));

    wgpu::TextureView multisampledColorView2 =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount).CreateView();
    wgpu::Texture resolveTexture2 = CreateTextureForRenderAttachment(kColorFormat, 1);
    wgpu::TextureView resolveView2 = resolveTexture2.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    // The second and third samples are included in the shader-output mask,
    // only the first one is covered by the triangle.
    constexpr float kMSAACoverage = 0.25f;
    const char* fs = R"(
        struct U {
            color0 : vec4f,
            color1 : vec4f,
        }
        @group(0) @binding(0) var<uniform> uBuffer : U;

        struct FragmentOut {
            @location(0) color0 : vec4f,
            @location(1) color1 : vec4f,
            @builtin(sample_mask) sampleMask : u32,
        }

        @fragment fn main() -> FragmentOut {
            var output : FragmentOut;
            output.color0 = uBuffer.color0;
            output.color1 = uBuffer.color1;
            output.sampleMask = 6u;
            return output;
        })";

    wgpu::RenderPipeline pipeline = CreateRenderPipelineForTest(fs, 2, false);
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};
    constexpr bool kTestDepth = false;

    // Draw a red triangle to the first color attachment, and a blue triangle to the second color
    // attachment, and do MSAA resolve on two render targets in one render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView, multisampledColorView2}, {mResolveView, resolveView2},
            wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, kTestDepth);

        std::array<float, 8> kUniformData = {kRed.r,   kRed.g,   kRed.b,   kRed.a,     // color1
                                             kGreen.r, kGreen.g, kGreen.b, kGreen.a};  // color2
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    // TODO(crbug.com/dawn/1462): Work around that a sample mask of zero is used for all
    // color targets except the last one.
    VerifyResolveTarget(
        HasToggleEnabled("no_workaround_sample_mask_becomes_zero_for_all_but_last_color_target")
            ? wgpu::Color{}
            : kRed,
        mResolveTexture, 0, 0, kMSAACoverage);
    VerifyResolveTarget(kGreen, resolveTexture2, 0, 0, kMSAACoverage);
}

// Test using one multisampled color attachment with resolve target can render correctly
// with alphaToCoverageEnabled.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithAlphaToCoverage) {
    constexpr bool kTestDepth = false;
    constexpr uint32_t kSampleMask = 0xFFFFFFFF;
    constexpr bool kAlphaToCoverageEnabled = true;

    // Setting alpha <= 0 must result in alpha-to-coverage mask being empty.
    // Setting alpha = 0.5f should result in alpha-to-coverage mask including half the samples,
    // but this is not guaranteed by the spec. The Metal spec seems to guarantee that this is
    // indeed the case.
    // Setting alpha >= 1 must result in alpha-to-coverage mask being full.
    for (float alpha : {-1.0f, 0.0f, 0.5f, 1.0f, 2.0f}) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
            kTestDepth, kSampleMask, kAlphaToCoverageEnabled);

        const wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, alpha};

        // Draw a green triangle.
        {
            utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
                {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
                kTestDepth);

            EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
        }

        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
        queue.Submit(1, &commandBuffer);

        // For alpha = {0, 0.5, 1} we expect msaaCoverage to correspond to the value of alpha.
        float msaaCoverage = alpha;
        if (alpha < 0.0f) {
            msaaCoverage = 0.0f;
        }
        if (alpha > 1.0f) {
            msaaCoverage = 1.0f;
        }

        utils::RGBA8 expectedColor = ExpectedMSAAColor(kGreen, msaaCoverage);
        EXPECT_TEXTURE_EQ(&expectedColor, mResolveTexture, {1, 0}, {1, 1});
    }
}

// Test doing MSAA resolve into multiple resolve targets works correctly with
// alphaToCoverage. The alphaToCoverage mask is computed based on the alpha
// component of the first color render attachment.
TEST_P(MultisampledRenderingTest, ResolveIntoMultipleResolveTargetsWithAlphaToCoverage) {
    // TODO(dawn:1550) Workaround introduces a bug on Qualcomm GPUs, but is necessary for ARM GPUs.
    DAWN_TEST_UNSUPPORTED_IF(IsAndroid() && IsQualcomm() &&
                             HasToggleEnabled("resolve_multiple_attachments_in_separate_passes"));

    wgpu::TextureView multisampledColorView2 =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount).CreateView();
    wgpu::Texture resolveTexture2 = CreateTextureForRenderAttachment(kColorFormat, 1);
    wgpu::TextureView resolveView2 = resolveTexture2.CreateView();
    constexpr uint32_t kSampleMask = 0xFFFFFFFF;
    constexpr float kMSAACoverage = 0.50f;
    constexpr bool kAlphaToCoverageEnabled = true;

    // The alpha-to-coverage mask should not depend on the alpha component of the
    // second color render attachment.
    // We test alpha = 0.51f and 0.99f instead of 0.50f and 1.00f because there are some rounding
    // differences on QuadroP400 devices in that case.
    for (float alpha : {0.0f, 0.51f, 0.99f}) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPipeline pipeline =
            CreateRenderPipelineWithTwoOutputsForTest(kSampleMask, kAlphaToCoverageEnabled);

        constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.51f};
        const wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, alpha};
        constexpr bool kTestDepth = false;

        // Draw a red triangle to the first color attachment, and a blue triangle to the second
        // color attachment, and do MSAA resolve on two render targets in one render pass.
        {
            utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
                {mMultisampledColorView, multisampledColorView2}, {mResolveView, resolveView2},
                wgpu::LoadOp::Clear, wgpu::LoadOp::Clear, kTestDepth);

            std::array<float, 8> kUniformData = {
                static_cast<float>(kRed.r),   static_cast<float>(kRed.g),
                static_cast<float>(kRed.b),   static_cast<float>(kRed.a),
                static_cast<float>(kGreen.r), static_cast<float>(kGreen.g),
                static_cast<float>(kGreen.b), static_cast<float>(kGreen.a)};
            constexpr uint32_t kSize = sizeof(kUniformData);
            EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(),
                                    kSize);
        }

        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Alpha to coverage affects both the color outputs, but the mask is computed
        // using only the first one.
        utils::RGBA8 expectedRed = ExpectedMSAAColor(kRed, kMSAACoverage);
        utils::RGBA8 expectedGreen = ExpectedMSAAColor(kGreen, kMSAACoverage);
        EXPECT_TEXTURE_EQ(&expectedRed, mResolveTexture, {1, 0}, {1, 1},
                          /* level */ 0, wgpu::TextureAspect::All, /* bytesPerRow */ 0,
                          /* tolerance */ utils::RGBA8(1, 1, 1, 1));
        EXPECT_TEXTURE_EQ(&expectedGreen, resolveTexture2, {1, 0}, {1, 1},
                          /* level */ 0, wgpu::TextureAspect::All, /* bytesPerRow */ 0,
                          /* tolerance */ utils::RGBA8(1, 1, 1, 1));
    }
}

// Test multisampled rendering with depth test works correctly with alphaToCoverage.
TEST_P(MultisampledRenderingTest, MultisampledRenderingWithDepthTestAndAlphaToCoverage) {
    // TODO(dawn:1549) Fails on Qualcomm-based Android devices.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsQualcomm());

    constexpr bool kTestDepth = true;
    constexpr uint32_t kSampleMask = 0xFFFFFFFF;

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipelineGreen =
        CreateRenderPipelineWithOneOutputForTest(kTestDepth, kSampleMask, true);
    wgpu::RenderPipeline pipelineRed =
        CreateRenderPipelineWithOneOutputForTest(kTestDepth, kSampleMask, false);

    // We test alpha = 0.51f and 0.81f instead of 0.50f and 0.80f because there are some
    // rounding differences on QuadroP400 devices in that case.
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.51f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.81f};

    // In first render pass we draw a green triangle with depth value == 0.2f.
    // We will only write to half the samples since the alphaToCoverage mode
    // is enabled for that render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);
        std::array<float, 8> kUniformData = {kGreen.r, kGreen.g, kGreen.b, kGreen.a,  // Color
                                             0.2f};                                   // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipelineGreen, kUniformData.data(),
                                kSize);
    }

    // In second render pass we draw a red triangle with depth value == 0.5f.
    // We will write to all the samples since the alphaToCoverageMode is diabled for
    // that render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            kTestDepth);

        std::array<float, 8> kUniformData = {kRed.r, kRed.g, kRed.b, kRed.a,  // color
                                             0.5f};                           // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipelineRed, kUniformData.data(),
                                kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    constexpr wgpu::Color kHalfGreenHalfRed = {(kGreen.r + kRed.r) / 2.0, (kGreen.g + kRed.g) / 2.0,
                                               (kGreen.b + kRed.b) / 2.0,
                                               (kGreen.a + kRed.a) / 2.0};
    utils::RGBA8 expectedColor = ExpectedMSAAColor(kHalfGreenHalfRed, 1.0f);

    EXPECT_TEXTURE_EQ(&expectedColor, mResolveTexture, {1, 0}, {1, 1},
                      /* level */ 0, wgpu::TextureAspect::All, /* bytesPerRow */ 0,
                      /* tolerance */ utils::RGBA8(1, 1, 1, 1));
}

// Test using one multisampled color attachment with resolve target can render correctly
// with alphaToCoverageEnabled and a sample mask.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithAlphaToCoverageAndSampleMask) {
    // TODO(dawn:1550) Fails on ARM-based Android devices.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());

    // TODO(dawn:491): This doesn't work on non-Apple GPU Metal, because we're using both
    // the shader-output mask (emulting the sampleMask from RenderPipeline) and alpha-to-coverage
    // at the same time. See the issue: https://github.com/gpuweb/gpuweb/issues/959.
    DAWN_SUPPRESS_TEST_IF(IsMetal() && !IsApple());

    constexpr bool kTestDepth = false;
    constexpr float kMSAACoverage = 0.50f;
    constexpr uint32_t kSampleMask = kFirstSampleMaskBit | kThirdSampleMaskBit;
    constexpr bool kAlphaToCoverageEnabled = true;

    // For those values of alpha we expect the proportion of samples to be covered
    // to correspond to the value of alpha.
    // We're assuming in the case of alpha = 0.50f that the implementation
    // dependendent algorithm will choose exactly one of the first and third samples.
    for (float alpha : {0.0f, 0.50f, 1.00f}) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
            kTestDepth, kSampleMask, kAlphaToCoverageEnabled);

        const wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, alpha - 0.01f};

        // Draw a green triangle.
        {
            utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
                {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
                kTestDepth);

            EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
        }

        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
        queue.Submit(1, &commandBuffer);

        utils::RGBA8 expectedColor = ExpectedMSAAColor(kGreen, kMSAACoverage * alpha);
        EXPECT_TEXTURE_EQ(&expectedColor, mResolveTexture, {1, 0}, {1, 1},
                          /* level */ 0, wgpu::TextureAspect::All, /* bytesPerRow */ 0,
                          /* tolerance */ utils::RGBA8(1, 1, 1, 1));
    }
}

// Test using one multisampled color attachment with resolve target can render correctly
// with alphaToCoverageEnabled and a rasterization mask.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithAlphaToCoverageAndRasterizationMask) {
    // TODO(dawn:1550) Fails on ARM-based Android devices.
    DAWN_SUPPRESS_TEST_IF(IsAndroid() && IsARM());

    constexpr bool kTestDepth = false;
    constexpr float kMSAACoverage = 0.50f;
    constexpr uint32_t kSampleMask = 0xFFFFFFFF;
    constexpr bool kAlphaToCoverageEnabled = true;
    constexpr bool kFlipTriangle = true;

    // For those values of alpha we expect the proportion of samples to be covered
    // to correspond to the value of alpha.
    // We're assuming in the case of alpha = 0.50f that the implementation
    // dependendent algorithm will choose exactly one of the samples covered by the
    // triangle.
    for (float alpha : {0.0f, 0.50f, 1.00f}) {
        wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
        wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
            kTestDepth, kSampleMask, kAlphaToCoverageEnabled, kFlipTriangle);

        const wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, alpha - 0.01f};

        // Draw a green triangle.
        {
            utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
                {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
                kTestDepth);

            EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
        }

        wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
        queue.Submit(1, &commandBuffer);

        VerifyResolveTarget(kGreen, mResolveTexture, 0, 0, kMSAACoverage * alpha);
    }
}

// Test that setting a scissor rect does not affect multisample resolve.
TEST_P(MultisampledRenderingTest, ResolveInto2DTextureWithScissor) {
    constexpr bool kTestDepth = false;
    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

    constexpr wgpu::Color kRed = {1.0f, 0.0f, 0.0f, 1.0f};
    constexpr wgpu::Color kGreen = {0.0f, 1.0f, 0.0f, 1.0f};

    // Draw a green triangle, set a scissor for the bottom row of pixels, then draw a red triangle.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {mMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            kTestDepth);

        const float redUniformData[4] = {1.0f, 0.0f, 0.0f, 1.0f};
        const float greenUniformData[4] = {0.0f, 1.0f, 0.0f, 1.0f};
        const size_t uniformSize = sizeof(float) * 4;

        wgpu::Buffer redUniformBuffer = utils::CreateBufferFromData(
            device, redUniformData, uniformSize, wgpu::BufferUsage::Uniform);
        wgpu::Buffer greenUniformBuffer = utils::CreateBufferFromData(
            device, greenUniformData, uniformSize, wgpu::BufferUsage::Uniform);

        wgpu::BindGroup redBindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, redUniformBuffer, 0, uniformSize}});
        wgpu::BindGroup greenBindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, greenUniformBuffer, 0, uniformSize}});

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.SetPipeline(pipeline);

        renderPassEncoder.SetBindGroup(0, greenBindGroup);
        renderPassEncoder.Draw(3);

        renderPassEncoder.SetScissorRect(0, 0, 3, 1);
        renderPassEncoder.SetBindGroup(0, redBindGroup);
        renderPassEncoder.Draw(3);

        renderPassEncoder.End();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    // The bottom-right pixel should be red, but the pixel just above it should be green.
    constexpr float kMSAACoverage = 1.0f;
    constexpr uint32_t kRedX = 2;
    constexpr uint32_t kRedY = 0;
    constexpr uint32_t kGreenX = 2;
    constexpr uint32_t kGreenY = 1;
    VerifyResolveTarget(kRed, mResolveTexture, 0, 0, kMSAACoverage, kRedX, kRedY);
    VerifyResolveTarget(kGreen, mResolveTexture, 0, 0, kMSAACoverage, kGreenX, kGreenY);
}

class MultisampledRenderingWithTransientAttachmentTest : public MultisampledRenderingTest {
    void SetUp() override {
        MultisampledRenderingTest::SetUp();

        // Skip all tests if the transient attachments feature is not supported.
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::TransientAttachments}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::TransientAttachments})) {
            requiredFeatures.push_back(wgpu::FeatureName::TransientAttachments);
        }
        return requiredFeatures;
    }
};

// Test using one multisampled color transient attachment with resolve target can render correctly.
TEST_P(MultisampledRenderingWithTransientAttachmentTest, ResolveTransientAttachmentInto2DTexture) {
    constexpr bool kTestDepth = false;
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(kTestDepth);

    auto transientMultisampledColorTexture =
        CreateTextureForRenderAttachment(kColorFormat, kSampleCount,
                                         /*mipLevelCount=*/1,
                                         /*arrayLayerCount=*/1,
                                         /*transientAttachment=*/true);
    auto transientMultisampledColorView = transientMultisampledColorTexture.CreateView();
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    // Draw a green triangle.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {transientMultisampledColorView}, {mResolveView}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear, kTestDepth);

        // Note: It is not possible to store into a transient attachment.
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;

        std::array<float, 4> kUniformData = {kGreen.r, kGreen.g, kGreen.b, kGreen.a};
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, mResolveTexture);
}

class MultisampledRenderToSingleSampledTest : public MultisampledRenderingTest {
    void SetUp() override {
        MultisampledRenderingTest::SetUp();

        // Skip all tests if the MSAARenderToSingleSampled feature is not supported.
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::MSAARenderToSingleSampled}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::MSAARenderToSingleSampled})) {
            requiredFeatures.push_back(wgpu::FeatureName::MSAARenderToSingleSampled);
        }
        return requiredFeatures;
    }
};

// Test rendering into a color attachment and start another render pass with LoadOp::Load
// will have the content preserved.
TEST_P(MultisampledRenderToSingleSampledTest, DrawThenLoad) {
    auto singleSampledTexture =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);

    auto singleSampledTextureView = singleSampledTexture.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
        /*testDepth=*/false, /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false,
        /*flipTriangle=*/false, /*enableExpandResolveLoadOp=*/false);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    wgpu::DawnRenderPassColorAttachmentRenderToSingleSampled msaaRenderToSingleSampledDesc;
    msaaRenderToSingleSampledDesc.implicitSampleCount = kSampleCount;

    // In first render pass we draw a green triangle.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {singleSampledTextureView}, {nullptr}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].nextInChain = &msaaRenderToSingleSampledDesc;

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    // In second render pass we only use LoadOp::Load with no draw call.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {singleSampledTextureView}, {nullptr}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].nextInChain = &msaaRenderToSingleSampledDesc;

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, singleSampledTexture);
}

// Test clear a color attachment (without implicit sample count) and start another render pass (with
// implicit sample count) with LoadOp::Load plus additional drawing works correctly. The final
// result should contain the combination of the loaded content from 1st pass and the 2nd pass.
TEST_P(MultisampledRenderToSingleSampledTest, ClearThenLoadThenDraw) {
    auto singleSampledTexture =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);

    auto singleSampledTextureView = singleSampledTexture.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
        /*testDepth=*/false, /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false,
        /*flipTriangle=*/false, /*enableExpandResolveLoadOp=*/false);

    constexpr wgpu::Color kRed = {1.0f, 0.0f, 0.0f, 1.0f};
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    wgpu::DawnRenderPassColorAttachmentRenderToSingleSampled msaaRenderToSingleSampledDesc;
    msaaRenderToSingleSampledDesc.implicitSampleCount = kSampleCount;

    // In first render pass we clear to red without using implicit sample count.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {singleSampledTextureView}, {nullptr}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].clearValue = kRed;

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    // In second render pass (with implicit sample count) we use LoadOp::Load then draw a green
    // triangle.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {singleSampledTextureView}, {nullptr}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].nextInChain = &msaaRenderToSingleSampledDesc;

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    auto commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    constexpr wgpu::Color kHalfGreenHalfRed = {(kGreen.r + kRed.r) / 2.0, (kGreen.g + kRed.g) / 2.0,
                                               (kGreen.b + kRed.b) / 2.0,
                                               (kGreen.a + kRed.a) / 2.0};
    utils::RGBA8 expectedColor = ExpectedMSAAColor(kHalfGreenHalfRed, 1.0f);

    EXPECT_TEXTURE_EQ(&expectedColor, singleSampledTexture, {1, 1}, {1, 1},
                      /* level */ 0, wgpu::TextureAspect::All, /* bytesPerRow */ 0,
                      /* tolerance */ utils::RGBA8(1, 1, 1, 1));
}

// Test multisampled rendering with depth test works correctly.
TEST_P(MultisampledRenderToSingleSampledTest, DrawWithDepthTest) {
    auto singleSampledTexture =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);

    auto singleSampledTextureView = singleSampledTexture.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
        /*testDepth=*/true, /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false,
        /*flipTriangle=*/false, /*enableExpandResolveLoadOp=*/false);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};
    wgpu::DawnRenderPassColorAttachmentRenderToSingleSampled msaaRenderToSingleSampledDesc;
    msaaRenderToSingleSampledDesc.implicitSampleCount = kSampleCount;

    // In first render pass we draw a green triangle with depth value == 0.2f.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {singleSampledTextureView}, {nullptr}, wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            /*testDepth=*/true);
        renderPass.cColorAttachments[0].nextInChain = &msaaRenderToSingleSampledDesc;

        std::array<float, 8> kUniformData = {kGreen.r, kGreen.g, kGreen.b, kGreen.a,  // Color
                                             0.2f};                                   // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    // In second render pass we draw a red triangle with depth value == 0.5f.
    // This red triangle should not be displayed because it is behind the green one that is drawn in
    // the last render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {singleSampledTextureView}, {nullptr}, wgpu::LoadOp::Load, wgpu::LoadOp::Load,
            /*testDepth=*/true);
        renderPass.cColorAttachments[0].nextInChain = &msaaRenderToSingleSampledDesc;

        std::array<float, 8> kUniformData = {kRed.r, kRed.g, kRed.b, kRed.a,  // color
                                             0.5f};                           // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    // The color of the pixel in the middle of mResolveTexture should be green if MSAA resolve runs
    // correctly with depth test.
    VerifyResolveTarget(kGreen, singleSampledTexture);
}

class DawnLoadResolveTextureTest : public MultisampledRenderingTest {
  protected:
    void SetUp() override {
        MultisampledRenderingTest::SetUp();

        // Skip all tests if the DawnLoadResolveTexture feature is not supported.
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::DawnLoadResolveTexture}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::DawnLoadResolveTexture})) {
            requiredFeatures.push_back(wgpu::FeatureName::DawnLoadResolveTexture);
        }
        if (SupportsFeatures({wgpu::FeatureName::DawnPartialLoadResolveTexture})) {
            requiredFeatures.push_back(wgpu::FeatureName::DawnPartialLoadResolveTexture);
        }
        if (SupportsFeatures({wgpu::FeatureName::TransientAttachments})) {
            requiredFeatures.push_back(wgpu::FeatureName::TransientAttachments);
        }
        return requiredFeatures;
    }

    bool HasResolveMultipleAttachmentInSeparatePassesToggle() {
        return HasToggleEnabled("resolve_multiple_attachments_in_separate_passes");
    }
};

// Test rendering into a resolve texture then start another render pass with
// LoadOp::ExpandResolveTexture. The resolve texture will have its content preserved between
// passes.
TEST_P(DawnLoadResolveTextureTest, DrawThenLoad) {
    auto multiSampledTexture = CreateTextureForRenderAttachment(
        kColorFormat, 4, 1, 1,
        /*transientAttachment=*/device.HasFeature(wgpu::FeatureName::TransientAttachments),
        /*supportsTextureBinding=*/false);
    auto multiSampledTextureView = multiSampledTexture.CreateView();

    auto singleSampledTexture =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);
    auto singleSampledTextureView = singleSampledTexture.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
        /*testDepth=*/false, /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false,
        /*flipTriangle=*/false, /*enableExpandResolveLoadOp=*/false);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // In first render pass we draw a green triangle. StoreOp=Discard to discard the MSAA texture's
    // content.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView}, {singleSampledTextureView}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    // In second render pass, we only use LoadOp::ExpandResolveTexture with no draw call.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView}, {singleSampledTextureView},
            wgpu::LoadOp::ExpandResolveTexture, wgpu::LoadOp::Load,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, singleSampledTexture);
}

// Test using ExpandResolveTexture load op for non-zero indexed attachment.
TEST_P(DawnLoadResolveTextureTest, DrawThenLoadNonZeroIndexedAttachment) {
    auto multiSampledTexture = CreateTextureForRenderAttachment(
        kColorFormat, 4, 1, 1,
        /*transientAttachment=*/device.HasFeature(wgpu::FeatureName::TransientAttachments),
        /*supportsTextureBinding=*/false);
    auto multiSampledTextureView = multiSampledTexture.CreateView();

    auto singleSampledTexture =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);
    auto singleSampledTextureView = singleSampledTexture.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithNonZeroLocationOutputForTest(
        /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false,
        /*enableExpandResolveLoadOp=*/false);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // In first render pass we draw a green triangle. StoreOp=Discard to discard the MSAA texture's
    // content.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {nullptr, multiSampledTextureView}, {nullptr, singleSampledTextureView},
            wgpu::LoadOp::Clear, wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[1].storeOp = wgpu::StoreOp::Discard;
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    // In second render pass, we only use LoadOp::ExpandResolveTexture with no draw call.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {nullptr, multiSampledTextureView}, {nullptr, singleSampledTextureView},
            wgpu::LoadOp::ExpandResolveTexture, wgpu::LoadOp::Load,
            /*testDepth=*/false);
        renderPass.cColorAttachments[1].storeOp = wgpu::StoreOp::Discard;
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, singleSampledTexture);
}

// Test rendering into 2 attachments. The 1st attachment will use
// LoadOp::ExpandResolveTexture.
TEST_P(DawnLoadResolveTextureTest, TwoOutputsDrawThenLoadColor0) {
    // TODO(42240662): "resolve_multiple_attachments_in_separate_passes" is currently not working
    // with DawnLoadResolveTexture feature if there are more than one attachment.
    DAWN_TEST_UNSUPPORTED_IF(HasResolveMultipleAttachmentInSeparatePassesToggle());

    // TODO(383731610): multiple outputs are not working in compat mode.
    DAWN_SUPPRESS_TEST_IF(IsCompatibilityMode());

    auto multiSampledTexture1 = CreateTextureForRenderAttachment(kColorFormat, 4, 1, 1,
                                                                 /*transientAttachment=*/false,
                                                                 /*supportsTextureBinding=*/false);
    auto multiSampledTextureView1 = multiSampledTexture1.CreateView();

    auto multiSampledTexture2 = CreateTextureForRenderAttachment(kColorFormat, 4, 1, 1,
                                                                 /*transientAttachment=*/false,
                                                                 /*supportsTextureBinding=*/false);
    auto multiSampledTextureView2 = multiSampledTexture2.CreateView();

    auto singleSampledTexture1 =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);
    auto singleSampledTextureView1 = singleSampledTexture1.CreateView();

    auto singleSampledTexture2 =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);
    auto singleSampledTextureView2 = singleSampledTexture2.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithTwoOutputsForTest(
        /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};

    // In first render pass:
    // - we draw a red triangle to attachment0. StoreOp=Discard to discard the MSAA texture's
    // content.
    // - we draw a green triangle to attachment1. StoreOp=Store.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView1, multiSampledTextureView2},
            {singleSampledTextureView1, singleSampledTextureView2}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
        renderPass.cColorAttachments[1].storeOp = wgpu::StoreOp::Store;

        std::array<float, 8> kUniformData = {
            static_cast<float>(kRed.r),   static_cast<float>(kRed.g),
            static_cast<float>(kRed.b),   static_cast<float>(kRed.a),
            static_cast<float>(kGreen.r), static_cast<float>(kGreen.g),
            static_cast<float>(kGreen.b), static_cast<float>(kGreen.a)};
        constexpr uint32_t kSize = sizeof(kUniformData);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    // In second render pass:
    // - we only use LoadOp::ExpandResolveTexture for attachment0 with no draw call.
    // - we only use LoadOp::Load for attachment1 with no draw call.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView1, multiSampledTextureView2},
            {singleSampledTextureView1, singleSampledTextureView2}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
        renderPass.cColorAttachments[1].loadOp = wgpu::LoadOp::Load;
        renderPass.cColorAttachments[1].storeOp = wgpu::StoreOp::Discard;
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kRed, singleSampledTexture1);
    VerifyResolveTarget(kGreen, singleSampledTexture2);
}

// Test rendering into 2 attachments. The 2nd attachment will use
// LoadOp::ExpandResolveTexture.
TEST_P(DawnLoadResolveTextureTest, TwoOutputsDrawThenLoadColor1) {
    // TODO(42240662): "resolve_multiple_attachments_in_separate_passes" is currently not working
    // with DawnLoadResolveTexture feature if there are more than one attachment.
    DAWN_TEST_UNSUPPORTED_IF(HasResolveMultipleAttachmentInSeparatePassesToggle());

    // TODO(383731610): multiple outputs are not working in compat mode.
    DAWN_SUPPRESS_TEST_IF(IsCompatibilityMode());

    auto multiSampledTexture1 = CreateTextureForRenderAttachment(kColorFormat, 4, 1, 1,
                                                                 /*transientAttachment=*/false,
                                                                 /*supportsTextureBinding=*/false);
    auto multiSampledTextureView1 = multiSampledTexture1.CreateView();

    auto multiSampledTexture2 = CreateTextureForRenderAttachment(kColorFormat, 4, 1, 1,
                                                                 /*transientAttachment=*/false,
                                                                 /*supportsTextureBinding=*/false);
    auto multiSampledTextureView2 = multiSampledTexture2.CreateView();

    auto singleSampledTexture1 =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);
    auto singleSampledTextureView1 = singleSampledTexture1.CreateView();

    auto singleSampledTexture2 =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);
    auto singleSampledTextureView2 = singleSampledTexture2.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithTwoOutputsForTest(
        /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};

    // In first render pass:
    // - we draw a red triangle to attachment0. StoreOp=Store.
    // - we draw a green triangle to attachment1. StoreOp=Discard.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView1, multiSampledTextureView2},
            {singleSampledTextureView1, singleSampledTextureView2}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
        renderPass.cColorAttachments[1].storeOp = wgpu::StoreOp::Discard;

        std::array<float, 8> kUniformData = {
            static_cast<float>(kRed.r),   static_cast<float>(kRed.g),
            static_cast<float>(kRed.b),   static_cast<float>(kRed.a),
            static_cast<float>(kGreen.r), static_cast<float>(kGreen.g),
            static_cast<float>(kGreen.b), static_cast<float>(kGreen.a)};
        constexpr uint32_t kSize = sizeof(kUniformData);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    // In second render pass:
    // - we only use LoadOp::Load for attachment0 with no draw call.
    // - we only use LoadOp::ExpandResolveTexture for attachment1 with no draw call.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView1, multiSampledTextureView2},
            {singleSampledTextureView1, singleSampledTextureView2}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::Load;
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
        renderPass.cColorAttachments[1].loadOp = wgpu::LoadOp::ExpandResolveTexture;
        renderPass.cColorAttachments[1].storeOp = wgpu::StoreOp::Discard;
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kRed, singleSampledTexture1);
    VerifyResolveTarget(kGreen, singleSampledTexture2);
}

// Test rendering into 2 attachments. The both attachments will use
// LoadOp::ExpandResolveTexture.
TEST_P(DawnLoadResolveTextureTest, TwoOutputsDrawThenLoadColor0AndColor1) {
    // TODO(42240662): "resolve_multiple_attachments_in_separate_passes" is currently not working
    // with DawnLoadResolveTexture feature if there are more than one attachment.
    DAWN_TEST_UNSUPPORTED_IF(HasResolveMultipleAttachmentInSeparatePassesToggle());

    auto multiSampledTexture1 = CreateTextureForRenderAttachment(kColorFormat, 4, 1, 1,
                                                                 /*transientAttachment=*/false,
                                                                 /*supportsTextureBinding=*/false);
    auto multiSampledTextureView1 = multiSampledTexture1.CreateView();

    auto multiSampledTexture2 = CreateTextureForRenderAttachment(kColorFormat, 4, 1, 1,
                                                                 /*transientAttachment=*/false,
                                                                 /*supportsTextureBinding=*/false);
    auto multiSampledTextureView2 = multiSampledTexture2.CreateView();

    auto singleSampledTexture1 =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);
    auto singleSampledTextureView1 = singleSampledTexture1.CreateView();

    auto singleSampledTexture2 =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);
    auto singleSampledTextureView2 = singleSampledTexture2.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithTwoOutputsForTest(
        /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};

    // In first render pass:
    // - we draw a red triangle to attachment0. StoreOp=Discard.
    // - we draw a green triangle to attachment1. StoreOp=Discard.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView1, multiSampledTextureView2},
            {singleSampledTextureView1, singleSampledTextureView2}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
        renderPass.cColorAttachments[1].storeOp = wgpu::StoreOp::Discard;

        std::array<float, 8> kUniformData = {
            static_cast<float>(kRed.r),   static_cast<float>(kRed.g),
            static_cast<float>(kRed.b),   static_cast<float>(kRed.a),
            static_cast<float>(kGreen.r), static_cast<float>(kGreen.g),
            static_cast<float>(kGreen.b), static_cast<float>(kGreen.a)};
        constexpr uint32_t kSize = sizeof(kUniformData);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    // In second render pass:
    // - we only use LoadOp::ExpandResolveTexture for attachment0 with no draw call.
    // - we only use LoadOp::ExpandResolveTexture for attachment1 with no draw call.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView1, multiSampledTextureView2},
            {singleSampledTextureView1, singleSampledTextureView2}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
        renderPass.cColorAttachments[1].loadOp = wgpu::LoadOp::ExpandResolveTexture;
        renderPass.cColorAttachments[1].storeOp = wgpu::StoreOp::Discard;
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kRed, singleSampledTexture1);
    VerifyResolveTarget(kGreen, singleSampledTexture2);
}

// Test ExpandResolveTexture load op rendering with depth test works correctly.
TEST_P(DawnLoadResolveTextureTest, DrawWithDepthTest) {
    auto multiSampledTexture = CreateTextureForRenderAttachment(
        kColorFormat, 4, 1, 1,
        /*transientAttachment=*/device.HasFeature(wgpu::FeatureName::TransientAttachments),
        /*supportsTextureBinding=*/false);
    auto multiSampledTextureView = multiSampledTexture.CreateView();

    auto singleSampledTexture =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);

    auto singleSampledTextureView = singleSampledTexture.CreateView();

    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
        /*testDepth=*/true, /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false,
        /*flipTriangle=*/false, /*enableExpandResolveLoadOp=*/true);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};

    // In first render pass we clear the render pass without drawing anything
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView}, {singleSampledTextureView}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear,
            /*testDepth=*/true);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    // In 2nd render pass we draw a green triangle with depth value == 0.2f.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView}, {singleSampledTextureView},
            wgpu::LoadOp::ExpandResolveTexture, wgpu::LoadOp::Clear,
            /*testDepth=*/true);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;

        std::array<float, 8> kUniformData = {kGreen.r, kGreen.g, kGreen.b, kGreen.a,  // Color
                                             0.2f};                                   // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    // In 3rd render pass we draw a red triangle with depth value == 0.5f.
    // This red triangle should not be displayed because it is behind the green one that is drawn in
    // the last render pass.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView}, {singleSampledTextureView},
            wgpu::LoadOp::ExpandResolveTexture, wgpu::LoadOp::Load,
            /*testDepth=*/true);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;

        std::array<float, 8> kUniformData = {kRed.r, kRed.g, kRed.b, kRed.a,  // color
                                             0.5f};                           // depth
        constexpr uint32_t kSize = sizeof(kUniformData);
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    // The color of the pixel in the middle of mResolveTexture should be green if MSAA resolve runs
    // correctly with depth test.
    VerifyResolveTarget(kGreen, singleSampledTexture);
}

// Test ExpandResolveTexture load op rendering with depth test works correctly with
// two outputs both use ExpandResolveTexture load op.
TEST_P(DawnLoadResolveTextureTest, TwoOutputsDrawWithDepthTestColor0AndColor1) {
    // TODO(42240662): "resolve_multiple_attachments_in_separate_passes" is currently not working
    // with DawnLoadResolveTexture feature if there are more than one attachment.
    DAWN_TEST_UNSUPPORTED_IF(HasResolveMultipleAttachmentInSeparatePassesToggle());

    auto multiSampledTexture1 = CreateTextureForRenderAttachment(kColorFormat, 4, 1, 1,
                                                                 /*transientAttachment=*/false,
                                                                 /*supportsTextureBinding=*/false);
    auto multiSampledTextureView1 = multiSampledTexture1.CreateView();

    auto multiSampledTexture2 = CreateTextureForRenderAttachment(kColorFormat, 4, 1, 1,
                                                                 /*transientAttachment=*/false,
                                                                 /*supportsTextureBinding=*/false);
    auto multiSampledTextureView2 = multiSampledTexture2.CreateView();

    auto singleSampledTexture1 =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);
    auto singleSampledTextureView1 = singleSampledTexture1.CreateView();

    auto singleSampledTexture2 =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);
    auto singleSampledTextureView2 = singleSampledTexture2.CreateView();

    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithTwoOutputsForTest(
        /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false,
        /*depthTest=*/true,
        /*loadOpForColor0=*/PipelineMultisampleLoadOp::ExpandResolveTarget,
        /*loadOpForColor1=*/PipelineMultisampleLoadOp::ExpandResolveTarget);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};

    // In first render pass we clear the render pass without drawing anything
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView1, multiSampledTextureView2},
            {singleSampledTextureView1, singleSampledTextureView2}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear,
            /*testDepth=*/true);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
        renderPass.cColorAttachments[1].storeOp = wgpu::StoreOp::Discard;

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    // In 2nd render pass:
    // - we draw a red triangle to attachment0.
    // - we draw a green triangle to attachment1.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView1, multiSampledTextureView2},
            {singleSampledTextureView1, singleSampledTextureView2},
            wgpu::LoadOp::ExpandResolveTexture, wgpu::LoadOp::Clear,
            /*testDepth=*/true);
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
        renderPass.cColorAttachments[1].loadOp = wgpu::LoadOp::ExpandResolveTexture;
        renderPass.cColorAttachments[1].storeOp = wgpu::StoreOp::Discard;

        std::array<float, 8> kUniformData = {
            static_cast<float>(kRed.r),   static_cast<float>(kRed.g),
            static_cast<float>(kRed.b),   static_cast<float>(kRed.a),
            static_cast<float>(kGreen.r), static_cast<float>(kGreen.g),
            static_cast<float>(kGreen.b), static_cast<float>(kGreen.a)};
        constexpr uint32_t kSize = sizeof(kUniformData);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    // In 3rd render pass:
    // - we draw a green triangle to attachment0.
    // - we draw a red triangle to attachment1.
    // Both triangles should not pass the depth test.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView1, multiSampledTextureView2},
            {singleSampledTextureView1, singleSampledTextureView2},
            wgpu::LoadOp::ExpandResolveTexture, wgpu::LoadOp::Load,
            /*testDepth=*/true);
        renderPass.cColorAttachments[0].loadOp = wgpu::LoadOp::ExpandResolveTexture;
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
        renderPass.cColorAttachments[1].loadOp = wgpu::LoadOp::ExpandResolveTexture;
        renderPass.cColorAttachments[1].storeOp = wgpu::StoreOp::Discard;

        std::array<float, 8> kUniformData = {
            static_cast<float>(kGreen.r), static_cast<float>(kGreen.g),
            static_cast<float>(kGreen.b), static_cast<float>(kGreen.a),
            static_cast<float>(kRed.r),   static_cast<float>(kRed.g),
            static_cast<float>(kRed.b),   static_cast<float>(kRed.a)};
        constexpr uint32_t kSize = sizeof(kUniformData);

        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kUniformData.data(), kSize);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kRed, singleSampledTexture1);
    VerifyResolveTarget(kGreen, singleSampledTexture2);
}

// Test rendering into a layer of a 2D array texture and load op=LoadOp::ExpandResolveTexture.
TEST_P(DawnLoadResolveTextureTest, DrawThenLoad2DArrayTextureLayer) {
    // Creating 2D view from 2D array texture is not supported in compat mode.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

    auto multiSampledTexture = CreateTextureForRenderAttachment(kColorFormat, 4, 1, 1,
                                                                /*transientAttachment=*/false,
                                                                /*supportsTextureBinding=*/false);
    auto multiSampledTextureView = multiSampledTexture.CreateView();

    auto singleSampledTexture = CreateTextureForRenderAttachment(
        kColorFormat, 1, 1, /*arrayCount=*/2, /*transientAttachment=*/false,
        /*supportsTextureBinding=*/true);
    wgpu::TextureViewDescriptor resolveViewDescriptor2;
    resolveViewDescriptor2.dimension = wgpu::TextureViewDimension::e2D;
    resolveViewDescriptor2.format = kColorFormat;
    resolveViewDescriptor2.baseArrayLayer = 1;
    resolveViewDescriptor2.baseMipLevel = 0;
    auto singleSampledTextureView = singleSampledTexture.CreateView(&resolveViewDescriptor2);

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
        /*testDepth=*/false, /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false,
        /*flipTriangle=*/false, /*enableExpandResolveLoadOp=*/false);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // In first render pass we draw a green triangle. StoreOp=Discard to discard the MSAA texture's
    // content.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView}, {singleSampledTextureView}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    // In second render pass, we only use LoadOp::ExpandResolveTexture with no draw call.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView}, {singleSampledTextureView},
            wgpu::LoadOp::ExpandResolveTexture, wgpu::LoadOp::Load,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    VerifyResolveTarget(kGreen, singleSampledTexture, 0, 1);
}

// Test RenderPassDescriptorExpandResolveRect works correctly when rendering with
// LoadOp::ExpandResolveTexture.
TEST_P(DawnLoadResolveTextureTest, ClearThenLoadAndDrawWithRect) {
    DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::DawnPartialLoadResolveTexture}));

    auto multiSampledTexture = CreateTextureForRenderAttachment(kColorFormat, 4, 1, 1,
                                                                /*transientAttachment=*/false,
                                                                /*supportsTextureBinding=*/false);
    auto multiSampledTextureView = multiSampledTexture.CreateView();

    auto singleSampledTexture =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);
    auto singleSampledTextureView = singleSampledTexture.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline pipeline = CreateRenderPipelineWithOneOutputForTest(
        /*testDepth=*/false, /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false,
        /*flipTriangle=*/false, /*enableExpandResolveLoadOp=*/true);

    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};

    // In the first render pass we clear the attachment without drawing anything.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView}, {singleSampledTextureView}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    // In the second render pass, we draw a green triangle with LoadOp::ExpandResolveTexture. We
    // also specify a scissor rect requiring the pixels in the right and bottom edges untouched
    // during expanding and resolving.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView}, {singleSampledTextureView},
            wgpu::LoadOp::ExpandResolveTexture, wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
        wgpu::RenderPassDescriptorExpandResolveRect rect{};
        rect.x = rect.y = 0;
        rect.width = kWidth - 1;
        rect.height = kHeight - 1;
        renderPass.nextInChain = &rect;
        EncodeRenderPassForTest(commandEncoder, renderPass, pipeline, kGreen);
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    // Verify the center is green.
    VerifyResolveTarget(kGreen, singleSampledTexture);
    // Verify the right-bottom corner, which is outside the rect, stays untouched.
    VerifyResolveTarget(kClearColor, singleSampledTexture, /*mipmapLevel=*/0, /*arrayLayer=*/0,
                        /*msaaCoverage=*/1.0f, /*x=*/kWidth - 1, /*y=*/kHeight - 1);
}

// Test RenderPassDescriptorExpandResolveRect works correctly with StoreOp::Discard;
TEST_P(DawnLoadResolveTextureTest, StoreOpDiscardWithRect) {
    DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::DawnPartialLoadResolveTexture}));

    auto multiSampledTexture = CreateTextureForRenderAttachment(kColorFormat, 4, 1, 1,
                                                                /*transientAttachment=*/false,
                                                                /*supportsTextureBinding=*/true);
    auto multiSampledTextureView = multiSampledTexture.CreateView();

    auto singleSampledTexture =
        CreateTextureForRenderAttachment(kColorFormat, 1, 1, 1, /*transientAttachment=*/false,
                                         /*supportsTextureBinding=*/true);
    auto singleSampledTextureView = singleSampledTexture.CreateView();

    wgpu::CommandEncoder commandEncoder = device.CreateCommandEncoder();
    wgpu::RenderPipeline greenPipeline = CreateRenderPipelineWithOneOutputForTest(
        /*testDepth=*/false, /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false,
        /*flipTriangle=*/false, /*enableExpandResolveLoadOp=*/false);

    wgpu::RenderPipeline redPipelineWithExpandResolve = CreateRenderPipelineWithOneOutputForTest(
        /*testDepth=*/false, /*sampleMask=*/0xFFFFFFFF, /*alphaToCoverageEnabled=*/false,
        /*flipTriangle=*/false, /*enableExpandResolveLoadOp=*/true);

    // The color is just random values to test expand VS not.
    constexpr wgpu::Color kGreen = {0.0f, 0.8f, 0.0f, 0.8f};
    constexpr wgpu::Color kRed = {0.8f, 0.0f, 0.0f, 0.8f};

    // In the first render pass, we draw a green triangle, and keep it stored in the multisampled
    // texture.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView}, {singleSampledTextureView}, wgpu::LoadOp::Clear,
            wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Store;
        EncodeRenderPassForTest(commandEncoder, renderPass, greenPipeline, kGreen);
    }

    // In the second render pass, we draw a red triangle with LoadOp::ExpandResolveTexture and
    // StoreOp::Discard. We also specify a scissor rect requiring the pixels in the right and bottom
    // edges untouched during expanding and resolving.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView}, {singleSampledTextureView},
            wgpu::LoadOp::ExpandResolveTexture, wgpu::LoadOp::Clear,
            /*testDepth=*/false);

        // Discard the multisampled texture at the end of this pass.
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;

        wgpu::RenderPassDescriptorExpandResolveRect rect{};
        rect.x = rect.y = 0;
        rect.width = kWidth - 1;
        rect.height = kHeight - 1;
        renderPass.nextInChain = &rect;
        EncodeRenderPassForTest(commandEncoder, renderPass, redPipelineWithExpandResolve, kRed);
    }

    // The third render pass is simply a resolve pass so that we can check the contents of the
    // multisample via the resolve target. As the multisampled texture was discarded in the second
    // pass, it must be wholly cleared before being resolved into the singlesampled texture.
    {
        utils::ComboRenderPassDescriptor renderPass = CreateComboRenderPassDescriptorForTest(
            {multiSampledTextureView}, {singleSampledTextureView}, wgpu::LoadOp::Load,
            wgpu::LoadOp::Clear,
            /*testDepth=*/false);
        renderPass.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;

        wgpu::RenderPassEncoder renderPassEncoder = commandEncoder.BeginRenderPass(&renderPass);
        renderPassEncoder.End();
    }

    wgpu::CommandBuffer commandBuffer = commandEncoder.Finish();
    queue.Submit(1, &commandBuffer);

    // The center, inside the rect, must be cleared.
    VerifyResolveTarget(kClearColor, singleSampledTexture);
    // The right-bottom corner, outside of the rect, must be cleared as well.
    VerifyResolveTarget(kClearColor, singleSampledTexture, /*mipmapLevel=*/0, /*arrayLayer=*/0,
                        /*msaaCoverage=*/1.0f, /*x=*/kWidth - 1, /*y=*/kHeight - 1);
}

DAWN_INSTANTIATE_TEST(MultisampledRenderingTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_resource_heap_tier2"}),
                      D3D12Backend({}, {"use_d3d12_render_pass"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      VulkanBackend({"always_resolve_into_zero_level_and_layer"}),
                      VulkanBackend({"resolve_multiple_attachments_in_separate_passes"}),
                      MetalBackend({"emulate_store_and_msaa_resolve"}),
                      MetalBackend({"always_resolve_into_zero_level_and_layer"}),
                      MetalBackend({"always_resolve_into_zero_level_and_layer",
                                    "emulate_store_and_msaa_resolve"}));

DAWN_INSTANTIATE_TEST(MultisampledRenderingWithTransientAttachmentTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_resource_heap_tier2"}),
                      D3D12Backend({}, {"use_d3d12_render_pass"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      VulkanBackend({"always_resolve_into_zero_level_and_layer"}),
                      MetalBackend({"emulate_store_and_msaa_resolve"}),
                      MetalBackend({"always_resolve_into_zero_level_and_layer"}),
                      MetalBackend({"always_resolve_into_zero_level_and_layer",
                                    "emulate_store_and_msaa_resolve"}));

DAWN_INSTANTIATE_TEST(MultisampledRenderToSingleSampledTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_resource_heap_tier2"}),
                      D3D12Backend({}, {"use_d3d12_render_pass"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      VulkanBackend({"always_resolve_into_zero_level_and_layer"}),
                      MetalBackend({"emulate_store_and_msaa_resolve"}),
                      MetalBackend({"always_resolve_into_zero_level_and_layer"}),
                      MetalBackend({"always_resolve_into_zero_level_and_layer",
                                    "emulate_store_and_msaa_resolve"}));

DAWN_INSTANTIATE_TEST(DawnLoadResolveTextureTest,
                      D3D11Backend(),
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_resource_heap_tier2"}),
                      D3D12Backend({}, {"use_d3d12_render_pass"}),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      VulkanBackend({"always_resolve_into_zero_level_and_layer"}),
                      VulkanBackend({"resolve_multiple_attachments_in_separate_passes"}),
                      MetalBackend({"emulate_store_and_msaa_resolve"}),
                      MetalBackend({"always_resolve_into_zero_level_and_layer"}),
                      MetalBackend({"always_resolve_into_zero_level_and_layer",
                                    "emulate_store_and_msaa_resolve"}));

}  // anonymous namespace
}  // namespace dawn
