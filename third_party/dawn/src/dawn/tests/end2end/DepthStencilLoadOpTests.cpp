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

#include <algorithm>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using Format = wgpu::TextureFormat;
enum class Check {
    CopyStencil,
    StencilTest,
    CopyDepth,
    DepthTest,
    SampleDepth,
};

std::ostream& operator<<(std::ostream& o, Check check) {
    switch (check) {
        case Check::CopyStencil:
            o << "CopyStencil";
            break;
        case Check::StencilTest:
            o << "StencilTest";
            break;
        case Check::CopyDepth:
            o << "CopyDepth";
            break;
        case Check::DepthTest:
            o << "DepthTest";
            break;
        case Check::SampleDepth:
            o << "SampleDepth";
            break;
    }
    return o;
}

DAWN_TEST_PARAM_STRUCT(DepthStencilLoadOpTestParams, Format, Check);

constexpr static uint32_t kRTSize = 16;
constexpr uint32_t kMipLevelCount = 2u;
constexpr std::array<float, kMipLevelCount> kDepthValues = {0.125f, 0.875f};
constexpr std::array<uint16_t, kMipLevelCount> kU16DepthValues = {8192u, 57343u};
constexpr std::array<uint8_t, kMipLevelCount> kStencilValues = {7u, 3u};

class DepthStencilLoadOpTests : public DawnTestWithParams<DepthStencilLoadOpTestParams> {
  protected:
    void SetUp() override {
        DawnTestWithParams<DepthStencilLoadOpTestParams>::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(!mIsFormatSupported);

        wgpu::TextureDescriptor descriptor;
        descriptor.size = {kRTSize, kRTSize};
        descriptor.format = GetParam().mFormat;
        descriptor.mipLevelCount = kMipLevelCount;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                           wgpu::TextureUsage::TextureBinding;

        texture = device.CreateTexture(&descriptor);

        wgpu::TextureViewDescriptor textureViewDesc = {};
        textureViewDesc.mipLevelCount = 1;

        for (uint32_t mipLevel = 0; mipLevel < kMipLevelCount; ++mipLevel) {
            textureViewDesc.baseMipLevel = mipLevel;
            textureViews[mipLevel] = texture.CreateView(&textureViewDesc);

            utils::ComboRenderPassDescriptor renderPassDescriptor({}, textureViews[mipLevel]);
            renderPassDescriptor.UnsetDepthStencilLoadStoreOpsForFormat(GetParam().mFormat);
            renderPassDescriptor.cDepthStencilAttachmentInfo.depthClearValue =
                kDepthValues[mipLevel];
            renderPassDescriptor.cDepthStencilAttachmentInfo.stencilClearValue =
                kStencilValues[mipLevel];
            renderPassDescriptors.push_back(renderPassDescriptor);
        }
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        switch (GetParam().mFormat) {
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

    void CheckMipLevel(uint32_t mipLevel) {
        uint32_t mipSize = std::max(kRTSize >> mipLevel, 1u);

        switch (GetParam().mCheck) {
            case Check::SampleDepth: {
                DAWN_TEST_UNSUPPORTED_IF(utils::IsStencilOnlyFormat(GetParam().mFormat));
                // textureLoad with texture_depth_xxx is not supported in compat mode.
                DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode());

                std::vector<float> expectedDepth(mipSize * mipSize, kDepthValues[mipLevel]);
                ExpectSampledDepthData(
                    texture, mipSize, mipSize, 0, mipLevel,
                    new detail::ExpectEq<float>(expectedDepth.data(), expectedDepth.size(), 0.0001))
                    << "sample depth mip " << mipLevel;
                break;
            }

            case Check::CopyDepth: {
                DAWN_TEST_UNSUPPORTED_IF(utils::IsStencilOnlyFormat(GetParam().mFormat));

                if (GetParam().mFormat == wgpu::TextureFormat::Depth16Unorm) {
                    std::vector<uint16_t> expectedDepth(mipSize * mipSize,
                                                        kU16DepthValues[mipLevel]);
                    EXPECT_TEXTURE_EQ(expectedDepth.data(), texture, {0, 0}, {mipSize, mipSize},
                                      mipLevel, wgpu::TextureAspect::DepthOnly,
                                      /* bytesPerRow */ 0, /* tolerance */ uint16_t(1))
                        << "copy depth mip " << mipLevel;
                } else {
                    std::vector<float> expectedDepth(mipSize * mipSize, kDepthValues[mipLevel]);
                    EXPECT_TEXTURE_EQ(expectedDepth.data(), texture, {0, 0}, {mipSize, mipSize},
                                      mipLevel, wgpu::TextureAspect::DepthOnly)
                        << "copy depth mip " << mipLevel;
                }

                break;
            }

            case Check::CopyStencil: {
                std::vector<uint8_t> expectedStencil(mipSize * mipSize, kStencilValues[mipLevel]);
                EXPECT_TEXTURE_EQ(expectedStencil.data(), texture, {0, 0}, {mipSize, mipSize},
                                  mipLevel, wgpu::TextureAspect::StencilOnly)
                    << "copy stencil mip " << mipLevel;
                break;
            }

            case Check::DepthTest: {
                DAWN_TEST_UNSUPPORTED_IF(utils::IsStencilOnlyFormat(GetParam().mFormat));

                std::vector<float> expectedDepth(mipSize * mipSize, kDepthValues[mipLevel]);
                ExpectAttachmentDepthTestData(texture, GetParam().mFormat, mipSize, mipSize, 0,
                                              mipLevel, expectedDepth)
                    << "depth test mip " << mipLevel;
                break;
            }

            case Check::StencilTest: {
                ExpectAttachmentStencilTestData(texture, GetParam().mFormat, mipSize, mipSize, 0,
                                                mipLevel, kStencilValues[mipLevel])
                    << "stencil test mip " << mipLevel;
                break;
            }
        }
    }

    wgpu::Texture texture;
    std::array<wgpu::TextureView, kMipLevelCount> textureViews;
    // Vector instead of array because there is no default constructor.
    std::vector<utils::ComboRenderPassDescriptor> renderPassDescriptors;

  private:
    bool mIsFormatSupported = false;
};

}  // anonymous namespace

// Check that clearing a mip level works at all.
TEST_P(DepthStencilLoadOpTests, ClearMip0) {
    // TODO(crbug.com/dawn/1828): depth16unorm broken on Apple GPUs.
    DAWN_SUPPRESS_TEST_IF(IsApple() && GetParam().mFormat == wgpu::TextureFormat::Depth16Unorm);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPassDescriptors[0]).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    CheckMipLevel(0u);
}

// Check that clearing a non-zero mip level works at all.
TEST_P(DepthStencilLoadOpTests, ClearMip1) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPassDescriptors[1]).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    CheckMipLevel(1u);
}

// Clear first mip then the second mip.  Check both mip levels.
TEST_P(DepthStencilLoadOpTests, ClearBothMip0Then1) {
    // TODO(crbug.com/dawn/1828): depth16unorm broken on Apple GPUs.
    DAWN_SUPPRESS_TEST_IF(IsApple() && GetParam().mFormat == wgpu::TextureFormat::Depth16Unorm);

    // TODO(42242119): fail on Qualcomm Adreno X1.
    DAWN_SUPPRESS_TEST_IF(IsD3D11() && IsQualcomm());

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPassDescriptors[0]).End();
    encoder.BeginRenderPass(&renderPassDescriptors[1]).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    CheckMipLevel(0u);
    CheckMipLevel(1u);
}

// Clear second mip then the first mip. Check both mip levels.
TEST_P(DepthStencilLoadOpTests, ClearBothMip1Then0) {
    // TODO(crbug.com/dawn/1828): depth16unorm broken on Apple GPUs.
    DAWN_SUPPRESS_TEST_IF(IsApple() && GetParam().mFormat == wgpu::TextureFormat::Depth16Unorm);

    // TODO(42242119): fail on Qualcomm Adreno X1.
    DAWN_SUPPRESS_TEST_IF(IsD3D11() && IsQualcomm());

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPassDescriptors[1]).End();
    encoder.BeginRenderPass(&renderPassDescriptors[0]).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    CheckMipLevel(0u);
    CheckMipLevel(1u);
}

namespace {

auto GenerateParam() {
    auto params1 = MakeParamGenerator<DepthStencilLoadOpTestParams>(
        {D3D11Backend(), D3D12Backend(), D3D12Backend({}, {"use_d3d12_render_pass"}),
         MetalBackend(), OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
        {wgpu::TextureFormat::Depth32Float, wgpu::TextureFormat::Depth16Unorm},
        {Check::CopyDepth, Check::DepthTest, Check::SampleDepth});

    auto params2 = MakeParamGenerator<DepthStencilLoadOpTestParams>(
        {D3D11Backend(), D3D12Backend(), D3D12Backend({}, {"use_d3d12_render_pass"}),
         MetalBackend(), MetalBackend({"metal_use_combined_depth_stencil_format_for_stencil8"}),
         MetalBackend(
             {"metal_use_both_depth_and_stencil_attachments_for_combined_depth_stencil_formats"}),
         OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
        {wgpu::TextureFormat::Depth24PlusStencil8, wgpu::TextureFormat::Depth32FloatStencil8,
         wgpu::TextureFormat::Stencil8},
        {Check::CopyStencil, Check::StencilTest, Check::DepthTest, Check::SampleDepth});

    std::vector<DepthStencilLoadOpTestParams> allParams;
    allParams.insert(allParams.end(), params1.begin(), params1.end());
    allParams.insert(allParams.end(), params2.begin(), params2.end());

    return allParams;
}

INSTANTIATE_TEST_SUITE_P(,
                         DepthStencilLoadOpTests,
                         ::testing::ValuesIn(GenerateParam()),
                         DawnTestBase::PrintToStringParamName("DepthStencilLoadOpTests"));
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DepthStencilLoadOpTests);

class StencilClearValueOverflowTest : public DepthStencilLoadOpTests {};

// Test when stencilClearValue overflows uint8_t (>255), only the last 8 bits will be applied as the
// stencil clear value in encoder.BeginRenderPass() (currently Dawn only supports 8-bit stencil
// format).
TEST_P(StencilClearValueOverflowTest, StencilClearValueOverFlowUint8) {
    constexpr uint32_t kOverflowedStencilValue = kStencilValues[0] + 0x100;
    renderPassDescriptors[0].cDepthStencilAttachmentInfo.stencilClearValue =
        kOverflowedStencilValue;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPassDescriptors[0]).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    CheckMipLevel(0u);
}

// Test when stencilClearValue overflows uint16_t(>65535), only the last 8 bits will be applied as
// the stencil clear value in encoder.BeginRenderPass() (currently Dawn only supports 8-bit stencil
// format).
TEST_P(StencilClearValueOverflowTest, StencilClearValueOverFlowUint16) {
    constexpr uint32_t kOverflowedStencilValue = kStencilValues[0] + 0x10000;
    renderPassDescriptors[0].cDepthStencilAttachmentInfo.stencilClearValue =
        kOverflowedStencilValue;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPassDescriptors[0]).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    CheckMipLevel(0u);
}

DAWN_INSTANTIATE_TEST_P(StencilClearValueOverflowTest,
                        {D3D11Backend(), D3D12Backend(),
                         D3D12Backend({}, {"use_d3d12_render_pass"}), MetalBackend(),
                         OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
                        {wgpu::TextureFormat::Depth24PlusStencil8,
                         wgpu::TextureFormat::Depth32FloatStencil8, wgpu::TextureFormat::Stencil8},
                        {Check::CopyStencil, Check::StencilTest});

// Regression tests to reproduce a flaky failure when running whole WebGPU CTS on Intel Gen12 GPUs.
// See crbug.com/dawn/1487 for more details.
using SupportsTextureBinding = bool;
DAWN_TEST_PARAM_STRUCT(DepthTextureClearTwiceTestParams, Format, SupportsTextureBinding);

class DepthTextureClearTwiceTest : public DawnTestWithParams<DepthTextureClearTwiceTestParams> {
  public:
    void RecordClearDepthAspectAtLevel(wgpu::CommandEncoder encoder,
                                       wgpu::Texture depthTexture,
                                       uint32_t level,
                                       float clearValue) {
        wgpu::TextureViewDescriptor viewDescriptor = {};
        viewDescriptor.baseArrayLayer = 0;
        viewDescriptor.arrayLayerCount = 1;
        viewDescriptor.baseMipLevel = level;
        viewDescriptor.mipLevelCount = 1;
        wgpu::TextureView view = depthTexture.CreateView(&viewDescriptor);

        wgpu::RenderPassDescriptor renderPassDescriptor = {};
        renderPassDescriptor.colorAttachmentCount = 0;

        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment = {};
        depthStencilAttachment.view = view;
        depthStencilAttachment.depthClearValue = clearValue;
        depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
        depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;

        if (!utils::IsDepthOnlyFormat(GetParam().mFormat)) {
            depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Load;
            depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
        }

        renderPassDescriptor.depthStencilAttachment = &depthStencilAttachment;
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPass.End();
    }

  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        switch (GetParam().mFormat) {
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

    bool mIsFormatSupported = false;
};

TEST_P(DepthTextureClearTwiceTest, ClearDepthAspectTwice) {
    DAWN_SUPPRESS_TEST_IF(!mIsFormatSupported);

    constexpr uint32_t kSize = 64;
    constexpr uint32_t kLevelCount = 5;

    wgpu::TextureFormat depthFormat = GetParam().mFormat;

    wgpu::TextureDescriptor descriptor;
    descriptor.size = {kSize, kSize};
    descriptor.format = depthFormat;
    descriptor.mipLevelCount = kLevelCount;

    // The toggle "d3d12_force_initialize_copyable_depth_stencil_texture_on_creation" is not related
    // to this test as we don't specify wgpu::TextureUsage::CopyDst in the test.
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    if (GetParam().mSupportsTextureBinding) {
        descriptor.usage |= wgpu::TextureUsage::TextureBinding;
    }
    wgpu::Texture depthTexture = device.CreateTexture(&descriptor);

    // First, clear all the subresources to 0.
    {
        constexpr float kClearValue = 0.f;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        for (uint32_t level = 0; level < kLevelCount; ++level) {
            RecordClearDepthAspectAtLevel(encoder, depthTexture, level, kClearValue);
        }
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);
    }

    // Then, clear several mipmap levels to 0.8.
    {
        constexpr float kClearValue = 0.8f;
        constexpr std::array<uint32_t, 2> kLevelsToSet = {2u, 4u};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        for (uint32_t level : kLevelsToSet) {
            RecordClearDepthAspectAtLevel(encoder, depthTexture, level, kClearValue);
        }
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);
    }

    // Check if the data in remaining mipmap levels is still 0.
    {
        constexpr std::array<uint32_t, 3> kLevelsToTest = {0, 1u, 3u};
        for (uint32_t level : kLevelsToTest) {
            uint32_t sizeAtLevel = kSize >> level;
            std::vector<float> expectedValue(sizeAtLevel * sizeAtLevel, 0.f);
            ExpectAttachmentDepthTestData(depthTexture, GetParam().mFormat, sizeAtLevel,
                                          sizeAtLevel, 0, level, expectedValue);
        }
    }
}

DAWN_INSTANTIATE_TEST_P(DepthTextureClearTwiceTest,
                        {D3D11Backend(), D3D11Backend({"use_packed_depth24_unorm_stencil8_format"}),
                         D3D12Backend(), D3D12Backend({"use_packed_depth24_unorm_stencil8_format"}),
                         MetalBackend(), OpenGLBackend(), OpenGLESBackend(), VulkanBackend()},
                        {wgpu::TextureFormat::Depth16Unorm, wgpu::TextureFormat::Depth24Plus,
                         wgpu::TextureFormat::Depth32Float,
                         wgpu::TextureFormat::Depth32FloatStencil8,
                         wgpu::TextureFormat::Depth24PlusStencil8},
                        {true, false});

}  // anonymous namespace
}  // namespace dawn
