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
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using Format = wgpu::TextureFormat;
using Aspect = wgpu::TextureAspect;
using Usage = wgpu::TextureUsage;
using Dimension = wgpu::TextureDimension;
using DepthOrArrayLayers = uint32_t;
using MipCount = uint32_t;
using Mip = uint32_t;
using SampleCount = uint32_t;

DAWN_TEST_PARAM_STRUCT(Params,
                       Format,
                       Aspect,
                       Usage,
                       Dimension,
                       DepthOrArrayLayers,
                       MipCount,
                       Mip,
                       SampleCount);

template <typename T>
class ExpectNonZero : public detail::CustomTextureExpectation {
  public:
    uint32_t DataSize() override { return sizeof(T); }

    testing::AssertionResult Check(const void* data, size_t size) override {
        DAWN_ASSERT(size % DataSize() == 0 && size > 0);
        const T* actual = static_cast<const T*>(data);
        T value = *actual;
        if (value == T(0)) {
            return testing::AssertionFailure()
                   << "Expected data to be non-zero, was " << value << "\n";
        }
        for (size_t i = 0; i < size / DataSize(); ++i) {
            if (actual[i] != value) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to match non-zero value " << value
                       << ", actual " << actual[i] << "\n";
            }
        }

        return testing::AssertionSuccess();
    }
};

class NonzeroTextureCreationTests : public DawnTestWithParams<Params> {
  protected:
    constexpr static uint32_t kSize = 128;

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        if (GetParam().mFormat == wgpu::TextureFormat::BC1RGBAUnorm &&
            SupportsFeatures({wgpu::FeatureName::TextureCompressionBC})) {
            return {wgpu::FeatureName::TextureCompressionBC};
        }
        return {};
    }

    void Run() {
        DAWN_TEST_UNSUPPORTED_IF(GetParam().mFormat == wgpu::TextureFormat::BC1RGBAUnorm &&
                                 !SupportsFeatures({wgpu::FeatureName::TextureCompressionBC}));

        // TODO(dawn:1877): Snorm copy failing ANGLE Swiftshader, need further investigation.
        DAWN_TEST_UNSUPPORTED_IF(GetParam().mFormat == wgpu::TextureFormat::RGBA8Snorm &&
                                 IsANGLESwiftShader());

        // You can't read compressed textures in compat mode.
        DAWN_TEST_UNSUPPORTED_IF(utils::IsCompressedTextureFormat(GetParam().mFormat) &&
                                 IsCompatibilityMode());

        // textureLoad() of depth textures is forbidden in Compat mode.
        DAWN_TEST_UNSUPPORTED_IF(utils::IsDepthOrStencilFormat(GetParam().mFormat) &&
                                 IsCompatibilityMode());

        // TODO(crbug.com/dawn/1637): Failures with ANGLE only with some format/aspect
        DAWN_SUPPRESS_TEST_IF(IsWindows() && IsANGLE() &&
                              GetParam().mAspect == wgpu::TextureAspect::All &&
                              (GetParam().mFormat == wgpu::TextureFormat::Stencil8 ||
                               GetParam().mFormat == wgpu::TextureFormat::Depth32Float));

        // TODO(dawn:1844): Work around this by clearing layers one by one on Intel.
        DAWN_SUPPRESS_TEST_IF(IsD3D11() && IsIntel() && GetParam().mDepthOrArrayLayers > 6);

        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = GetParam().mDimension;
        descriptor.size.width = kSize;
        descriptor.size.height = kSize;
        descriptor.size.depthOrArrayLayers = GetParam().mDepthOrArrayLayers;
        descriptor.sampleCount = GetParam().mSampleCount;
        descriptor.format = GetParam().mFormat;
        descriptor.usage = GetParam().mUsage;
        descriptor.mipLevelCount = GetParam().mMipCount;

        // Only set the textureBindingViewDimension in compat mode. It's not needed
        // nor used in non-compat.
        wgpu::TextureBindingViewDimensionDescriptor textureBindingViewDimensionDesc;
        if (IsCompatibilityMode()) {
            if (descriptor.dimension == wgpu::TextureDimension::e2D &&
                descriptor.size.depthOrArrayLayers == 6) {
                // Testing cube texture copy for compat.
                textureBindingViewDimensionDesc.textureBindingViewDimension =
                    wgpu::TextureViewDimension::Cube;
            }
            descriptor.nextInChain = &textureBindingViewDimensionDesc;
        }

        wgpu::Texture texture = device.CreateTexture(&descriptor);

        uint32_t mip = GetParam().mMip;
        uint32_t mipSize = std::max(kSize >> mip, 1u);
        uint32_t depthOrArrayLayers = GetParam().mDimension == wgpu::TextureDimension::e3D
                                          ? std::max(GetParam().mDepthOrArrayLayers >> mip, 1u)
                                          : GetParam().mDepthOrArrayLayers;
        switch (GetParam().mFormat) {
            case wgpu::TextureFormat::R8Unorm: {
                if (GetParam().mSampleCount > 1) {
                    ExpectMultisampledFloatData(texture, mipSize, mipSize, 1,
                                                GetParam().mSampleCount, 0, mip,
                                                new ExpectNonZero<float>());
                } else {
                    EXPECT_TEXTURE_EQ(new ExpectNonZero<uint8_t>(), texture, {0, 0, 0},
                                      {mipSize, mipSize, depthOrArrayLayers}, mip);
                }
                break;
            }
            case wgpu::TextureFormat::RG8Unorm: {
                if (GetParam().mSampleCount > 1) {
                    ExpectMultisampledFloatData(texture, mipSize, mipSize, 2,
                                                GetParam().mSampleCount, 0, mip,
                                                new ExpectNonZero<float>());
                } else {
                    EXPECT_TEXTURE_EQ(new ExpectNonZero<uint16_t>(), texture, {0, 0, 0},
                                      {mipSize, mipSize, depthOrArrayLayers}, mip);
                }
                break;
            }
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::RGBA8Snorm: {
                if (GetParam().mSampleCount > 1) {
                    ExpectMultisampledFloatData(texture, mipSize, mipSize, 4,
                                                GetParam().mSampleCount, 0, mip,
                                                new ExpectNonZero<float>());
                } else {
                    EXPECT_TEXTURE_EQ(new ExpectNonZero<uint32_t>(), texture, {0, 0, 0},
                                      {mipSize, mipSize, depthOrArrayLayers}, mip);
                }
                break;
            }
            case wgpu::TextureFormat::Depth32Float: {
                EXPECT_TEXTURE_EQ(new ExpectNonZero<float>(), texture, {0, 0, 0},
                                  {mipSize, mipSize, depthOrArrayLayers}, mip);
                break;
            }
            case wgpu::TextureFormat::Depth24PlusStencil8: {
                switch (GetParam().mAspect) {
                    case wgpu::TextureAspect::DepthOnly: {
                        for (uint32_t arrayLayer = 0; arrayLayer < GetParam().mDepthOrArrayLayers;
                             ++arrayLayer) {
                            ExpectSampledDepthData(texture, mipSize, mipSize, arrayLayer, mip,
                                                   new ExpectNonZero<float>())
                                << "arrayLayer " << arrayLayer;
                        }
                        break;
                    }
                    case wgpu::TextureAspect::StencilOnly: {
                        uint32_t texelCount = mipSize * mipSize * depthOrArrayLayers;
                        std::vector<uint8_t> expectedStencil(texelCount, 1);
                        EXPECT_TEXTURE_EQ(expectedStencil.data(), texture, {0, 0, 0},
                                          {mipSize, mipSize, depthOrArrayLayers}, mip,
                                          wgpu::TextureAspect::StencilOnly);

                        break;
                    }
                    default:
                        DAWN_UNREACHABLE();
                }
                break;
            }
            case wgpu::TextureFormat::Stencil8: {
                uint32_t texelCount = mipSize * mipSize * depthOrArrayLayers;
                std::vector<uint8_t> expectedStencil(texelCount, 1);
                EXPECT_TEXTURE_EQ(expectedStencil.data(), texture, {0, 0, 0},
                                  {mipSize, mipSize, depthOrArrayLayers}, mip,
                                  wgpu::TextureAspect::StencilOnly);
                break;
            }
            case wgpu::TextureFormat::BC1RGBAUnorm: {
                // Set buffer with dirty data so we know it is cleared by the lazy cleared
                // texture copy
                uint32_t blockWidth = utils::GetTextureFormatBlockWidth(GetParam().mFormat);
                uint32_t blockHeight = utils::GetTextureFormatBlockHeight(GetParam().mFormat);
                wgpu::Extent3D copySize = {Align(mipSize, blockWidth), Align(mipSize, blockHeight),
                                           depthOrArrayLayers};

                uint32_t bytesPerRow =
                    utils::GetMinimumBytesPerRow(GetParam().mFormat, copySize.width);
                uint32_t rowsPerImage = copySize.height / blockHeight;

                uint64_t bufferSize = utils::RequiredBytesInCopy(bytesPerRow, rowsPerImage,
                                                                 copySize, GetParam().mFormat);

                std::vector<uint8_t> data(bufferSize, 100);
                wgpu::Buffer bufferDst = utils::CreateBufferFromData(
                    device, data.data(), bufferSize, wgpu::BufferUsage::CopySrc);

                wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
                    utils::CreateTexelCopyBufferInfo(bufferDst, 0, bytesPerRow, rowsPerImage);
                wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
                    utils::CreateTexelCopyTextureInfo(texture, mip, {0, 0, 0});

                wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
                encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &copySize);
                wgpu::CommandBuffer commands = encoder.Finish();
                queue.Submit(1, &commands);

                uint32_t copiedWidthInBytes = utils::GetTexelBlockSizeInBytes(GetParam().mFormat) *
                                              copySize.width / blockWidth;
                uint8_t* d = data.data();
                for (uint32_t z = 0; z < depthOrArrayLayers; ++z) {
                    for (uint32_t row = 0; row < copySize.height / blockHeight; ++row) {
                        std::fill_n(d, copiedWidthInBytes, 1);
                        d += bytesPerRow;
                    }
                }
                EXPECT_BUFFER_U8_RANGE_EQ(data.data(), bufferDst, 0, bufferSize);
                break;
            }
            default:
                DAWN_UNREACHABLE();
        }
    }
};

class NonzeroNonrenderableTextureCreationTests : public NonzeroTextureCreationTests {};
class NonzeroCompressedTextureCreationTests : public NonzeroTextureCreationTests {};
class NonzeroDepthTextureCreationTests : public NonzeroTextureCreationTests {};
class NonzeroDepthStencilTextureCreationTests : public NonzeroTextureCreationTests {};
class NonzeroStencilTextureCreationTests : public NonzeroTextureCreationTests {};
class NonzeroMultisampledTextureCreationTests : public NonzeroTextureCreationTests {};

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroTextureCreationTests, TextureCreationClears) {
    Run();
}

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroNonrenderableTextureCreationTests, TextureCreationClears) {
    Run();
}

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroCompressedTextureCreationTests, TextureCreationClears) {
    Run();
}

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroDepthTextureCreationTests, TextureCreationClears) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    Run();
}

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroDepthStencilTextureCreationTests, TextureCreationClears) {
    Run();
}

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroStencilTextureCreationTests, TextureCreationClears) {
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());

    Run();
}

// Test that texture clears to a non-zero value because toggle is enabled.
TEST_P(NonzeroMultisampledTextureCreationTests, TextureCreationClears) {
    Run();
}

DAWN_INSTANTIATE_TEST_P(
    NonzeroTextureCreationTests,
    {D3D11Backend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"}),
     OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                     {"lazy_clear_resource_on_first_use"}),
     VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"})},
    {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::RG8Unorm, wgpu::TextureFormat::RGBA8Unorm},
    {wgpu::TextureAspect::All},
    {wgpu::TextureUsage(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc),
     wgpu::TextureUsage::CopySrc},
    {wgpu::TextureDimension::e2D},
    {1u, 7u, 6u /* test for compat cube */},  // depth or array layers
    {4u},                                     // mip count
    {0u, 1u, 2u, 3u},                         // mip
    {1u}                                      // sample count
);

DAWN_INSTANTIATE_TEST_P(NonzeroNonrenderableTextureCreationTests,
                        {D3D11Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                       {"lazy_clear_resource_on_first_use"}),
                         OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                         {"lazy_clear_resource_on_first_use"}),
                         VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                       {"lazy_clear_resource_on_first_use"})},
                        {wgpu::TextureFormat::RGBA8Snorm},
                        {wgpu::TextureAspect::All},
                        {wgpu::TextureUsage::CopySrc},
                        {wgpu::TextureDimension::e2D, wgpu::TextureDimension::e3D},
                        {1u, 7u, 6u /* test for compat cube */},  // depth or array layers
                        {4u},                                     // mip count
                        {0u, 1u, 2u, 3u},                         // mip
                        {1u}                                      // sample count
);

DAWN_INSTANTIATE_TEST_P(NonzeroCompressedTextureCreationTests,
                        {D3D11Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                       {"lazy_clear_resource_on_first_use"}),
                         OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                         {"lazy_clear_resource_on_first_use"}),
                         VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                       {"lazy_clear_resource_on_first_use"})},
                        {wgpu::TextureFormat::BC1RGBAUnorm},
                        {wgpu::TextureAspect::All},
                        {wgpu::TextureUsage::CopySrc},
                        {wgpu::TextureDimension::e2D},
                        {1u, 7u, 6u /* test for compat cube */},  // depth or array layers
                        {4u},                                     // mip count
                        {0u, 1u, 2u, 3u},                         // mip
                        {1u}                                      // sample count
);

DAWN_INSTANTIATE_TEST_P(NonzeroDepthTextureCreationTests,
                        {D3D11Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                      {"lazy_clear_resource_on_first_use"}),
                         OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                       {"lazy_clear_resource_on_first_use"}),
                         OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                         {"lazy_clear_resource_on_first_use"}),
                         VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                                       {"lazy_clear_resource_on_first_use"})},
                        {wgpu::TextureFormat::Depth32Float},
                        {wgpu::TextureAspect::All, wgpu::TextureAspect::DepthOnly},
                        {wgpu::TextureUsage(wgpu::TextureUsage::RenderAttachment |
                                            wgpu::TextureUsage::CopySrc),
                         wgpu::TextureUsage::CopySrc},
                        {wgpu::TextureDimension::e2D},
                        {1u, 7u, 6u /* test for compat cube */},  // depth or array layers
                        {4u},                                     // mip count
                        {0u, 1u, 2u, 3u},                         // mip
                        {1u}                                      // sample count
);

DAWN_INSTANTIATE_TEST_P(
    NonzeroDepthStencilTextureCreationTests,
    {D3D11Backend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"}),
     OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                     {"lazy_clear_resource_on_first_use"}),
     VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"})},
    {wgpu::TextureFormat::Depth24PlusStencil8},
    {wgpu::TextureAspect::DepthOnly, wgpu::TextureAspect::StencilOnly},
    {wgpu::TextureUsage(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                        wgpu::TextureUsage::TextureBinding),
     wgpu::TextureUsage(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc)},
    {wgpu::TextureDimension::e2D},
    {1u, 7u, 6u /* test for compat cube */},  // depth or array layers
    {4u},                                     // mip count
    {0u, 1u, 2u, 3u},                         // mip
    {1u}                                      // sample count
);

DAWN_INSTANTIATE_TEST_P(
    NonzeroStencilTextureCreationTests,
    {D3D11Backend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"}),
     OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                     {"lazy_clear_resource_on_first_use"}),
     VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"})},
    {wgpu::TextureFormat::Stencil8},
    {wgpu::TextureAspect::All, wgpu::TextureAspect::StencilOnly},
    {wgpu::TextureUsage(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc |
                        wgpu::TextureUsage::TextureBinding),
     wgpu::TextureUsage(wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc)},
    {wgpu::TextureDimension::e2D},
    {1u, 7u, 6u /* test for compat cube */},  // depth or array layers
    {4u},                                     // mip count
    {0u, 1u, 2u, 3u},                         // mip
    {1u}                                      // sample count
);

DAWN_INSTANTIATE_TEST_P(
    NonzeroMultisampledTextureCreationTests,
    {D3D11Backend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     MetalBackend({"nonzero_clear_resources_on_creation_for_testing"},
                  {"lazy_clear_resource_on_first_use"}),
     MetalBackend({"nonzero_clear_resources_on_creation_for_testing",
                   "metal_use_combined_depth_stencil_format_for_stencil8"},
                  {"lazy_clear_resource_on_first_use"}),
     OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"}),
     OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"},
                     {"lazy_clear_resource_on_first_use"}),
     VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"},
                   {"lazy_clear_resource_on_first_use"})},
    {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::RG8Unorm, wgpu::TextureFormat::RGBA8Unorm},
    {wgpu::TextureAspect::All},
    {wgpu::TextureUsage(wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding)},
    {wgpu::TextureDimension::e2D},
    {1u},  // depth or array layers
    {1u},  // mip count
    {0u},  // mip
    {4u}   // sample count
);

}  // anonymous namespace
}  // namespace dawn
