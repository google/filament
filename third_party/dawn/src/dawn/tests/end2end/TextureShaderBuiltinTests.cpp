// Copyright 2023 The Dawn & Tint Authors
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
#include <string>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr wgpu::TextureFormat kDefaultFormat = wgpu::TextureFormat::RGBA8Unorm;

class TextureShaderBuiltinTests : public DawnTest {
  protected:
    wgpu::Texture Create2DTexture(const char* label,
                                  uint32_t width,
                                  uint32_t height,
                                  uint32_t arrayLayerCount,
                                  uint32_t mipLevelCount,
                                  uint32_t sampleCount,
                                  wgpu::TextureUsage usage,
                                  wgpu::TextureViewDimension textureBindingViewDimension) {
        wgpu::TextureDescriptor descriptor;
        descriptor.label = label;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = width;
        descriptor.size.height = height;
        descriptor.size.depthOrArrayLayers = arrayLayerCount;
        descriptor.sampleCount = sampleCount;
        descriptor.format = kDefaultFormat;
        descriptor.mipLevelCount = mipLevelCount;
        descriptor.usage = usage;

        // Only set the textureBindingViewDimension in compat mode. It's not needed
        // nor used in non-compat.
        wgpu::TextureBindingViewDimensionDescriptor textureBindingViewDimensionDesc;
        if (IsCompatibilityMode()) {
            textureBindingViewDimensionDesc.textureBindingViewDimension =
                textureBindingViewDimension;
            descriptor.nextInChain = &textureBindingViewDimensionDesc;
        }

        return device.CreateTexture(&descriptor);
    }

    wgpu::Texture CreateTexture(const char* label,
                                uint32_t arrayLayerCount,
                                uint32_t mipLevelCount,
                                uint32_t sampleCount,
                                wgpu::TextureViewDimension textureBindingViewDimension) {
        DAWN_ASSERT(arrayLayerCount > 0 && mipLevelCount > 0);
        DAWN_ASSERT(sampleCount == 1 || sampleCount == 4);

        const uint32_t textureWidthLevel0 = 1 << mipLevelCount;
        const uint32_t textureHeightLevel0 = 1 << mipLevelCount;
        constexpr wgpu::TextureUsage kUsage = wgpu::TextureUsage::CopyDst |
                                              wgpu::TextureUsage::TextureBinding |
                                              wgpu::TextureUsage::RenderAttachment;
        return Create2DTexture(label, textureWidthLevel0, textureHeightLevel0, arrayLayerCount,
                               mipLevelCount, sampleCount, kUsage, textureBindingViewDimension);
    }

    wgpu::Texture CreateTexture(const char* label,
                                uint32_t arrayLayerCount,
                                uint32_t mipLevelCount,
                                uint32_t sampleCount) {
        return CreateTexture(label, arrayLayerCount, mipLevelCount, sampleCount,
                             arrayLayerCount == 1 ? wgpu::TextureViewDimension::e2D
                                                  : wgpu::TextureViewDimension::e2DArray);
    }

    wgpu::TextureView CreateTextureView(const wgpu::Texture& tex,
                                        const char* label,
                                        wgpu::TextureViewDimension dimension,
                                        uint32_t baseMipLevel = 0,
                                        uint32_t mipLevelCount = wgpu::kMipLevelCountUndefined) {
        wgpu::TextureViewDescriptor descriptor;
        descriptor.label = label;
        descriptor.dimension = dimension;
        // textureNumLevels return texture view levels
        descriptor.baseMipLevel = baseMipLevel;
        descriptor.mipLevelCount = mipLevelCount;
        return tex.CreateView(&descriptor);
    }
};

// Note: the following tests for various texture method behavior is mainly
// targeted at OpenGL/OpenGLES backend without native GLSL support for these builtins.
// These tests should be trivial for otherbackend, and thus can be used as control case.

// Test calling textureNumLevels & textureNumSamples & textureDimensions in one shader.
// Note: textureDimensions implicitly calls textureNumLevels after tint shader transform to clamp
// mip levels.
TEST_P(TextureShaderBuiltinTests, Basic) {
    constexpr uint32_t kLayers = 3;
    constexpr uint32_t kMipLevels = 2;
    wgpu::Texture tex1 = CreateTexture("tex1", kLayers, kMipLevels, 1);
    wgpu::TextureView texView1 =
        CreateTextureView(tex1, "texView1", wgpu::TextureViewDimension::e2DArray);

    constexpr uint32_t kSampleCount = 4;
    wgpu::Texture tex2 = CreateTexture("tex2", 1, 1, kSampleCount);
    wgpu::TextureView texView2 = tex2.CreateView();

    constexpr uint32_t kMipLevelsView = 1;
    wgpu::Texture tex3 =
        CreateTexture("tex3", kLayers, kMipLevels, 1, wgpu::TextureViewDimension::e2DArray);
    wgpu::TextureView texView3 =
        CreateTextureView(tex3, "texView3",
                          IsCompatibilityMode() ? wgpu::TextureViewDimension::e2DArray
                                                : wgpu::TextureViewDimension::e2D,
                          1, kMipLevelsView);

    const uint32_t textureWidthLevel0 = 1 << kMipLevels;
    const uint32_t textureWidthLevel1 = textureWidthLevel0 >> 1;
    const uint32_t expected[] = {kLayers,        kMipLevels,         kSampleCount,
                                 kMipLevelsView, textureWidthLevel0, textureWidthLevel1};

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(expected);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    const char* tex3Type = IsCompatibilityMode() ? "texture_2d_array<f32>" : "texture_2d<f32>";

    std::ostringstream shaderSource;
    // clang-format off
    shaderSource << R"(
    @group(0) @binding(0) var<storage, read_write> dstBuf : array<u32>;
    @group(0) @binding(1) var tex1 : texture_2d_array<f32>;
    // Use sparse binding to test impact of binding remapping
    @group(0) @binding(4) var tex2 : texture_multisampled_2d<f32>;
    @group(1) @binding(3) var tex3 : )" << tex3Type << R"(;

    @compute @workgroup_size(1, 1, 1) fn main() {
        dstBuf[0] = textureNumLayers(tex1); // control case
        dstBuf[1] = textureNumLevels(tex1);
        dstBuf[2] = textureNumSamples(tex2);
        dstBuf[3] = textureNumLevels(tex3);
        dstBuf[4] = textureDimensions(tex1, 0).x;
        dstBuf[5] = textureDimensions(tex1, 1).x;
        }
        )";
    // clang-format on

    wgpu::ComputePipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.compute.module = utils::CreateShaderModule(device, shaderSource.str());
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDescriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {
                                                      {0, buffer},
                                                      {1, texView1},
                                                      {4, texView2},
                                                  }));
        pass.SetBindGroup(1, utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(1),
                                                  {
                                                      {3, texView3},
                                                  }));
        pass.DispatchWorkgroups(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected, buffer, 0, sizeof(expected) / sizeof(uint32_t));
}

// Testing that baseMipLevel is handled correctly.
TEST_P(TextureShaderBuiltinTests, BaseMipLevelTextureView) {
    // TODO(dawn:2538): failing on OpenGLES Angle backed by D3D11.
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());
    constexpr uint32_t kLayers = 1;
    constexpr uint32_t kMipLevels = 3;
    wgpu::Texture tex =
        CreateTexture("tex", kLayers, kMipLevels, 1, wgpu::TextureViewDimension::e2D);

    constexpr uint32_t kBaseMipLevel = 1;
    constexpr uint32_t kViewMipLevelCount = 2;
    DAWN_ASSERT(kBaseMipLevel + kViewMipLevelCount <= kMipLevels);
    wgpu::TextureView texView = CreateTextureView(tex, "texView", wgpu::TextureViewDimension::e2D,
                                                  kBaseMipLevel, kViewMipLevelCount);

    const uint32_t textureWidthLevel0 = 1 << kMipLevels;
    const uint32_t textureWidthLevel1 = textureWidthLevel0 >> 1;
    const uint32_t textureWidthLevel2 = textureWidthLevel1 >> 1;
    const uint32_t expected[] = {
        textureWidthLevel1,
        textureWidthLevel1,
        textureWidthLevel2,
    };

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(expected);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    std::ostringstream shaderSource;
    // clang-format off
    shaderSource << R"(
    @group(0) @binding(0) var<storage, read_write> dstBuf : array<u32>;
    @group(0) @binding(1) var tex : texture_2d<f32>;

    @compute @workgroup_size(1, 1, 1) fn main() {
        dstBuf[0] = textureDimensions(tex).x;
        dstBuf[1] = textureDimensions(tex, 0).x;
        dstBuf[2] = textureDimensions(tex, 1).x;
    }
    )";
    // clang-format on

    wgpu::ComputePipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.compute.module = utils::CreateShaderModule(device, shaderSource.str());
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDescriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {
                                                      {0, buffer},
                                                      {1, texView},
                                                  }));
        pass.DispatchWorkgroups(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected, buffer, 0, sizeof(expected) / sizeof(uint32_t));
}

// Testing that baseMipLevel is handled correctly for texture_cube.
TEST_P(TextureShaderBuiltinTests, BaseMipLevelTextureViewCube) {
    // TODO(crbug.com/dawn/2442): diagnose this failure on Win Angle D3D11
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());

    constexpr uint32_t kCubeLayers = 6;
    constexpr uint32_t kMipLevels = 3;
    wgpu::Texture texCube =
        CreateTexture("texCube", kCubeLayers, kMipLevels, 1, wgpu::TextureViewDimension::Cube);

    constexpr uint32_t kBaseMipLevel = 1;
    constexpr uint32_t kViewMipLevelCount = 2;
    DAWN_ASSERT(kBaseMipLevel + kViewMipLevelCount <= kMipLevels);
    wgpu::TextureView texViewCube =
        CreateTextureView(texCube, "texViewCube", wgpu::TextureViewDimension::Cube, kBaseMipLevel,
                          kViewMipLevelCount);

    const uint32_t textureWidthLevel0 = 1 << kMipLevels;
    const uint32_t textureWidthLevel1 = textureWidthLevel0 >> 1;
    const uint32_t textureWidthLevel2 = textureWidthLevel1 >> 1;
    const uint32_t expected[] = {textureWidthLevel1, textureWidthLevel1, textureWidthLevel2,
                                 kViewMipLevelCount};

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(expected);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    std::ostringstream shaderSource;
    // clang-format off
    shaderSource << R"(
    @group(0) @binding(0) var<storage, read_write> dstBuf : array<u32>;
    @group(0) @binding(1) var tex_cube : texture_cube<f32>;

    @compute @workgroup_size(1, 1, 1) fn main() {
        dstBuf[0] = textureDimensions(tex_cube).x;
        dstBuf[1] = textureDimensions(tex_cube, 0).x;
        dstBuf[2] = textureDimensions(tex_cube, 1).x;
        dstBuf[3] = textureNumLevels(tex_cube);
    }
    )";
    // clang-format on

    wgpu::ComputePipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.compute.module = utils::CreateShaderModule(device, shaderSource.str());
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDescriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {
                                                      {0, buffer},
                                                      {1, texViewCube},
                                                  }));
        pass.DispatchWorkgroups(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected, buffer, 0, sizeof(expected) / sizeof(uint32_t));
}

// Test calling textureNumLevels & textureNumSamples inside function and taking a function param as
// the argument.
TEST_P(TextureShaderBuiltinTests, BuiltinCallInFunction) {
    constexpr uint32_t kLayers = 3;
    constexpr uint32_t kMipLevels1 = 2;
    wgpu::Texture tex1 = CreateTexture("tex1", kLayers, kMipLevels1, 1);
    wgpu::TextureView texView1 =
        CreateTextureView(tex1, "texView1", wgpu::TextureViewDimension::e2DArray);
    constexpr uint32_t kMipLevels2 = 5;
    wgpu::Texture tex2 =
        CreateTexture("tex2", 1, kMipLevels2, 1, wgpu::TextureViewDimension::e2DArray);
    wgpu::TextureView texView2 =
        CreateTextureView(tex2, "texView2", wgpu::TextureViewDimension::e2DArray);

    const uint32_t expected[] = {
        kLayers, kMipLevels1, kMipLevels1, kMipLevels2, kMipLevels1 + 100u,
    };

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(expected);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::ComputePipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.compute.module = utils::CreateShaderModule(device, R"(
@group(0) @binding(0) var<storage, read_write> dstBuf : array<u32>;
@group(0) @binding(1) var tex1 : texture_2d_array<f32>;
@group(0) @binding(2) var tex2 : texture_2d_array<f32>;

fn f(tex: texture_2d_array<f32>) -> u32 {
    return textureNumLevels(tex);
}

fn f_nested(tex: texture_2d_array<f32>, d: u32) -> u32 {
    return f(tex) + d;
}

@compute @workgroup_size(1, 1, 1) fn main() {
    dstBuf[0] = textureNumLayers(tex1); // control case
    dstBuf[1] = textureNumLevels(tex1);
    dstBuf[2] = f(tex1);
    dstBuf[3] = f(tex2);
    dstBuf[4] = f_nested(tex1, 100u);
}
    )");
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDescriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {
                                                      {0, buffer},
                                                      {1, texView1},
                                                      {2, texView2},
                                                  }));
        pass.DispatchWorkgroups(1);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected, buffer, 0, sizeof(expected) / sizeof(uint32_t));
}

// Test the internal uniform buffer data is properly updated between dispatches
// When the same pipeline is set only once.
TEST_P(TextureShaderBuiltinTests, OnePipelineMultipleDispatches) {
    const char* shader = R"(
@group(0) @binding(0) var<storage, read_write> dstBuf : array<u32>;
@group(0) @binding(1) var tex1 : texture_2d_array<f32>;
// Use sparse binding to test impact of binding remapping
@group(0) @binding(4) var tex2 : texture_multisampled_2d<f32>;
@group(1) @binding(3) var tex3 : texture_2d_array<f32>;

@compute @workgroup_size(1, 1, 1) fn main() {
    dstBuf[0] = textureNumLayers(tex1); // control case
    dstBuf[1] = textureNumLevels(tex1);
    dstBuf[2] = textureNumSamples(tex2);
    dstBuf[3] = textureNumLevels(tex3);
}
    )";

    constexpr uint32_t kLayers_1 = 3;
    constexpr uint32_t kMipLevels_1 = 2;
    wgpu::Texture tex1_1 = CreateTexture("tex1_1", kLayers_1, kMipLevels_1, 1);
    wgpu::TextureView texView1_1 =
        CreateTextureView(tex1_1, "texView1_1", wgpu::TextureViewDimension::e2DArray);
    constexpr uint32_t kLayers_2 = 5;
    constexpr uint32_t kMipLevels_2 = 4;
    wgpu::Texture tex1_2 = CreateTexture("tex1_2", kLayers_2, kMipLevels_2, 1);
    wgpu::TextureView texView1_2 =
        CreateTextureView(tex1_2, "texView1_2", wgpu::TextureViewDimension::e2DArray);

    constexpr uint32_t kSampleCount_1 = 4;
    wgpu::Texture tex2_1 = CreateTexture("tex2_1", 1, 1, kSampleCount_1);
    wgpu::TextureView texView2_1 = tex2_1.CreateView();
    constexpr uint32_t kSampleCount_2 = 4;
    wgpu::Texture tex2_2 = CreateTexture("tex2_2", 1, 1, kSampleCount_2);
    wgpu::TextureView texView2_2 = tex2_2.CreateView();

    constexpr uint32_t kMipLevelsView_1 = 1;
    wgpu::Texture tex3_1 = CreateTexture("tex3_1", kLayers_1, kMipLevels_1, 1);
    wgpu::TextureView texView3_1 = CreateTextureView(
        tex3_1, "texView3_1", wgpu::TextureViewDimension::e2DArray, 0, kMipLevelsView_1);
    constexpr uint32_t kMipLevelsView_2 = 2;
    wgpu::Texture tex3_2 = CreateTexture("tex3_2", kLayers_2, kMipLevels_2, 1);
    wgpu::TextureView texView3_2 = CreateTextureView(
        tex3_2, "texView3_2", wgpu::TextureViewDimension::e2DArray, 0, kMipLevelsView_2);

    constexpr uint32_t expected_1[] = {
        // Output from first dispatch
        kLayers_1,
        kMipLevels_1,
        kSampleCount_1,
        kMipLevelsView_1,
    };
    constexpr uint32_t expected_2[] = {
        // Output from second dispatch with different bind group
        kLayers_2,
        kMipLevels_2,
        kSampleCount_2,
        kMipLevelsView_2,
    };
    constexpr uint32_t expected_3[] = {
        // Output from third dispatch with bind group partially reset
        kLayers_1,
        kMipLevels_1,
        kSampleCount_1,
        kMipLevelsView_2,
    };
    DAWN_ASSERT(sizeof(expected_1) == sizeof(expected_2));
    DAWN_ASSERT(sizeof(expected_1) == sizeof(expected_3));

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(expected_1);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer_1 = device.CreateBuffer(&bufferDesc);
    wgpu::Buffer buffer_2 = device.CreateBuffer(&bufferDesc);
    wgpu::Buffer buffer_3 = device.CreateBuffer(&bufferDesc);

    wgpu::ComputePipeline pipeline;
    {
        wgpu::ComputePipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.compute.module = utils::CreateShaderModule(device, shader);
        pipeline = device.CreateComputePipeline(&pipelineDescriptor);
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);

        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {
                                                      {0, buffer_1},
                                                      {1, texView1_1},
                                                      {4, texView2_1},
                                                  }));
        pass.SetBindGroup(1, utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(1),
                                                  {
                                                      {3, texView3_1},
                                                  }));
        pass.DispatchWorkgroups(1);

        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {
                                                      {0, buffer_2},
                                                      {1, texView1_2},
                                                      {4, texView2_2},
                                                  }));
        pass.SetBindGroup(1, utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(1),
                                                  {
                                                      {3, texView3_2},
                                                  }));
        pass.DispatchWorkgroups(1);

        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                  {
                                                      {0, buffer_3},
                                                      {1, texView1_1},
                                                      {4, texView2_1},
                                                  }));
        // Note: bind group 1 is not set
        pass.DispatchWorkgroups(1);

        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected_1, buffer_1, 0, sizeof(expected_1) / sizeof(uint32_t));
    EXPECT_BUFFER_U32_RANGE_EQ(expected_2, buffer_2, 0, sizeof(expected_2) / sizeof(uint32_t));
    EXPECT_BUFFER_U32_RANGE_EQ(expected_3, buffer_3, 0, sizeof(expected_3) / sizeof(uint32_t));
}

// Test textureNumLevels & textureNumSamples results correctness used in multiple pipelines sharing
// same shader module.
TEST_P(TextureShaderBuiltinTests, OneShaderModuleMultipleEntryPoints) {
    std::ostringstream shaderSource;

    const char* tex3Type = IsCompatibilityMode() ? "texture_2d_array<f32>" : "texture_2d<f32>";
    // clang-format off
    shaderSource << R"(
@group(0) @binding(0) var<storage, read_write> dstBuf : array<u32>;
@group(0) @binding(1) var tex1 : texture_2d_array<f32>;
// Use sparse binding to test impact of binding remapping
@group(0) @binding(4) var tex2 : texture_multisampled_2d<f32>;
@group(1) @binding(3) var tex3 : )" << tex3Type << R"(;

@compute @workgroup_size(1, 1, 1) fn main1() {
    dstBuf[0] = textureNumLayers(tex1); // control case
    dstBuf[1] = textureNumLevels(tex1);
    dstBuf[2] = textureNumSamples(tex2);
    dstBuf[3] = textureNumLevels(tex3);
}

@compute @workgroup_size(1, 1, 1) fn main2() {
    dstBuf[0] = textureNumLayers(tex1); // control case
    dstBuf[1] = textureNumLevels(tex1);
    dstBuf[2] = textureNumSamples(tex2);
    // _ = textureNumLevels(tex3);
    dstBuf[3] = 99;
}
    )";
    // clang-format on

    constexpr uint32_t kLayers_1 = 3;
    constexpr uint32_t kMipLevels_1 = 2;
    wgpu::Texture tex1_1 = CreateTexture("tex1_1", kLayers_1, kMipLevels_1, 1);
    wgpu::TextureView texView1_1 =
        CreateTextureView(tex1_1, "tex1_1", wgpu::TextureViewDimension::e2DArray);
    constexpr uint32_t kLayers_2 = 5;
    constexpr uint32_t kMipLevels_2 = 4;
    wgpu::Texture tex1_2 = CreateTexture("tex1_2", kLayers_2, kMipLevels_2, 1);
    wgpu::TextureView texView1_2 =
        CreateTextureView(tex1_2, "tex1_2", wgpu::TextureViewDimension::e2DArray);

    constexpr uint32_t kSampleCount_1 = 4;
    wgpu::Texture tex2_1 = CreateTexture("tex2_1", 1, 1, kSampleCount_1);
    wgpu::TextureView texView2_1 = tex2_1.CreateView();
    // constexpr uint32_t kSampleCount_2 = 1;
    constexpr uint32_t kSampleCount_2 = 4;
    wgpu::Texture tex2_2 = CreateTexture("tex2_2", 1, 1, kSampleCount_2);
    wgpu::TextureView texView2_2 = tex2_2.CreateView();

    constexpr uint32_t kMipLevelsView_1 = 1;
    wgpu::Texture tex3_1 =
        CreateTexture("tex3_1", kLayers_1, kMipLevels_1, 1, wgpu::TextureViewDimension::e2DArray);
    wgpu::TextureView texView3_1 =
        CreateTextureView(tex3_1, "tex3_1",
                          IsCompatibilityMode() ? wgpu::TextureViewDimension::e2DArray
                                                : wgpu::TextureViewDimension::e2D,
                          0, kMipLevelsView_1);
    constexpr uint32_t kMipLevelsView_2 = 1;
    wgpu::Texture tex3_2 =
        CreateTexture("tex3_2", kLayers_2, kMipLevels_2, 1, wgpu::TextureViewDimension::e2DArray);
    wgpu::TextureView texView3_2 =
        CreateTextureView(tex3_2, "tex3_2",
                          IsCompatibilityMode() ? wgpu::TextureViewDimension::e2DArray
                                                : wgpu::TextureViewDimension::e2D,
                          0, kMipLevelsView_2);

    constexpr uint32_t expected_1[] = {
        // Output from first dispatch
        kLayers_1,
        kMipLevels_1,
        kSampleCount_1,
        kMipLevelsView_1,
    };
    constexpr uint32_t expected_2[] = {
        // Output from second dispatch with different bind group
        kLayers_2,
        kMipLevels_2,
        kSampleCount_2,
        99,
    };
    DAWN_ASSERT(sizeof(expected_1) == sizeof(expected_2));

    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(expected_1);
    bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
    wgpu::Buffer buffer_1 = device.CreateBuffer(&bufferDesc);
    wgpu::Buffer buffer_2 = device.CreateBuffer(&bufferDesc);

    wgpu::ShaderModule module = utils::CreateShaderModule(device, shaderSource.str());
    wgpu::ComputePipeline pipeline_1;
    {
        wgpu::ComputePipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.compute.module = module;
        pipelineDescriptor.compute.entryPoint = "main1";
        pipeline_1 = device.CreateComputePipeline(&pipelineDescriptor);
    }
    wgpu::ComputePipeline pipeline_2;
    {
        wgpu::ComputePipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.compute.module = module;
        pipelineDescriptor.compute.entryPoint = "main2";
        pipeline_2 = device.CreateComputePipeline(&pipelineDescriptor);
    }

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

        pass.SetPipeline(pipeline_1);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline_1.GetBindGroupLayout(0),
                                                  {
                                                      {0, buffer_1},
                                                      {1, texView1_1},
                                                      {4, texView2_1},
                                                  }));
        pass.SetBindGroup(1, utils::MakeBindGroup(device, pipeline_1.GetBindGroupLayout(1),
                                                  {
                                                      {3, texView3_1},
                                                  }));
        pass.DispatchWorkgroups(1);

        pass.SetPipeline(pipeline_2);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, pipeline_2.GetBindGroupLayout(0),
                                                  {
                                                      {0, buffer_2},
                                                      {1, texView1_2},
                                                      {4, texView2_2},
                                                  }));
        pass.DispatchWorkgroups(1);

        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    EXPECT_BUFFER_U32_RANGE_EQ(expected_1, buffer_1, 0, sizeof(expected_1) / sizeof(uint32_t));
    EXPECT_BUFFER_U32_RANGE_EQ(expected_2, buffer_2, 0, sizeof(expected_2) / sizeof(uint32_t));
}

DAWN_INSTANTIATE_TEST(TextureShaderBuiltinTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
