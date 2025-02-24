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

#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

wgpu::Texture Create2DTexture(
    wgpu::Device device,
    uint32_t width,
    uint32_t height,
    uint32_t mipLevelCount,
    uint32_t arrayLayerCount,
    wgpu::TextureFormat format,
    wgpu::TextureUsage usage,
    uint32_t sampleCount = 1,
    const wgpu::DawnTextureInternalUsageDescriptor* internalDesc = nullptr) {
    wgpu::TextureDescriptor descriptor;
    descriptor.nextInChain = internalDesc;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depthOrArrayLayers = arrayLayerCount;
    descriptor.sampleCount = sampleCount;
    descriptor.format = format;
    descriptor.mipLevelCount = mipLevelCount;
    descriptor.usage = usage;
    wgpu::Texture tex = device.CreateTexture(&descriptor);
    return tex;
}

wgpu::ExternalTexture CreateExternalTexture(wgpu::Device device, uint32_t width, uint32_t height) {
    wgpu::Texture texture =
        Create2DTexture(device, width, height, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::TextureBinding);

    // Create a texture view for the external texture
    wgpu::TextureView view = texture.CreateView();

    // Create an ExternalTextureDescriptor from the texture view
    wgpu::ExternalTextureDescriptor externalDesc;
    utils::ColorSpaceConversionInfo info = utils::GetYUVBT709ToRGBSRGBColorSpaceConversionInfo();
    externalDesc.yuvToRgbConversionMatrix = info.yuvToRgbConversionMatrix.data();
    externalDesc.gamutConversionMatrix = info.gamutConversionMatrix.data();
    externalDesc.srcTransferFunctionParameters = info.srcTransferFunctionParameters.data();
    externalDesc.dstTransferFunctionParameters = info.dstTransferFunctionParameters.data();

    externalDesc.plane0 = view;

    externalDesc.cropOrigin = {0, 0};
    externalDesc.cropSize = {width, height};
    externalDesc.apparentSize = {width, height};

    // Import the external texture
    return device.CreateExternalTexture(&externalDesc);
}

class CopyTextureForBrowserTest : public ValidationTest {
  protected:
    void TestCopyTextureForBrowser(utils::Expectation expectation,
                                   wgpu::Texture srcTexture,
                                   uint32_t srcLevel,
                                   wgpu::Origin3D srcOrigin,
                                   wgpu::Texture dstTexture,
                                   uint32_t dstLevel,
                                   wgpu::Origin3D dstOrigin,
                                   wgpu::Extent3D extent3D,
                                   wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                   wgpu::CopyTextureForBrowserOptions options = {}) {
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, srcLevel, srcOrigin, aspect);
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dstTexture, dstLevel, dstOrigin, aspect);

        if (expectation == utils::Expectation::Success) {
            device.GetQueue().CopyTextureForBrowser(&srcTexelCopyTextureInfo,
                                                    &dstTexelCopyTextureInfo, &extent3D, &options);
        } else {
            ASSERT_DEVICE_ERROR(device.GetQueue().CopyTextureForBrowser(
                &srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent3D, &options));
        }
    }
};

class CopyTextureForBrowserInternalUsageTest : public CopyTextureForBrowserTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DawnInternalUsages};
    }
};

class CopyExternalTextureForBrowserTest : public ValidationTest {
  protected:
    void TestCopyExternalTextureForBrowser(utils::Expectation expectation,
                                           wgpu::ExternalTexture srcExternalTexture,
                                           wgpu::Origin3D srcOrigin,
                                           wgpu::Extent2D srcNaturalSize,
                                           wgpu::Texture dstTexture,
                                           uint32_t dstLevel,
                                           wgpu::Origin3D dstOrigin,
                                           wgpu::Extent3D extent3D,
                                           wgpu::TextureAspect aspect = wgpu::TextureAspect::All,
                                           wgpu::CopyTextureForBrowserOptions options = {}) {
        wgpu::ImageCopyExternalTexture srcImageCopyExternalTexture;
        srcImageCopyExternalTexture.externalTexture = srcExternalTexture;
        srcImageCopyExternalTexture.origin = srcOrigin;
        srcImageCopyExternalTexture.naturalSize = srcNaturalSize;

        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dstTexture, dstLevel, dstOrigin, aspect);

        if (expectation == utils::Expectation::Success) {
            device.GetQueue().CopyExternalTextureForBrowser(
                &srcImageCopyExternalTexture, &dstTexelCopyTextureInfo, &extent3D, &options);
        } else {
            ASSERT_DEVICE_ERROR(device.GetQueue().CopyExternalTextureForBrowser(
                &srcImageCopyExternalTexture, &dstTexelCopyTextureInfo, &extent3D, &options));
        }
    }
};

class CopyExternalTextureForBrowserInternalUsageTest : public CopyExternalTextureForBrowserTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::DawnInternalUsages};
    }
};

// Tests should be Success
TEST_F(CopyTextureForBrowserTest, Success) {
    wgpu::Texture source =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    // Different copies, including some that touch the OOB condition
    {
        // Copy a region along top left boundary
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1});

        // Copy entire texture
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {16, 16, 1});

        // Copy a region along bottom right boundary
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {8, 8, 0}, destination, 0,
                                  {8, 8, 0}, {8, 8, 1});

        // Copy region into mip
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 2,
                                  {0, 0, 0}, {4, 4, 1});

        // Copy mip into region
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 2, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1});

        // Copy between slices
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 1}, {16, 16, 1});
    }

    // Empty copies are valid
    {
        // An empty copy
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {0, 0, 1});

        // An empty copy with depth = 0
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {0, 0, 0});

        // An empty copy touching the side of the source texture
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {16, 16, 0}, {0, 0, 1});

        // An empty copy touching the side of the destination texture
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {16, 16, 0}, {0, 0, 1});
    }
}

// Test source or destination texture has wrong usages
TEST_F(CopyTextureForBrowserTest, IncorrectUsage) {
    wgpu::Texture validSource =
        Create2DTexture(device, 16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
    wgpu::Texture validDestination =
        Create2DTexture(device, 16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture noSampledUsageSource = Create2DTexture(
        device, 16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);
    wgpu::Texture noRenderAttachmentUsageDestination = Create2DTexture(
        device, 16, 16, 5, 2, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);
    wgpu::Texture noCopySrcUsageSource = Create2DTexture(
        device, 16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::TextureBinding);
    wgpu::Texture noCopyDstUsageSource =
        Create2DTexture(device, 16, 16, 5, 2, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::RenderAttachment);

    // Incorrect source usage causes failure : lack |Sampled| usage
    TestCopyTextureForBrowser(utils::Expectation::Failure, noSampledUsageSource, 0, {0, 0, 0},
                              validDestination, 0, {0, 0, 0}, {16, 16, 1});

    // Incorrect destination usage causes failure: lack |RenderAttachement| usage.
    TestCopyTextureForBrowser(utils::Expectation::Failure, validSource, 0, {0, 0, 0},
                              noRenderAttachmentUsageDestination, 0, {0, 0, 0}, {16, 16, 1});

    // Incorrect source usage causes failure : lack |CopySrc| usage
    TestCopyTextureForBrowser(utils::Expectation::Failure, noCopySrcUsageSource, 0, {0, 0, 0},
                              validDestination, 0, {0, 0, 0}, {16, 16, 1});

    // Incorrect destination usage causes failure: lack |CopyDst| usage.
    TestCopyTextureForBrowser(utils::Expectation::Failure, validSource, 0, {0, 0, 0},
                              noCopyDstUsageSource, 0, {0, 0, 0}, {16, 16, 1});
}

// Test source or destination texture is destroyed.
TEST_F(CopyTextureForBrowserTest, DestroyedTexture) {
    wgpu::CopyTextureForBrowserOptions options = {};

    // Valid src and dst textures.
    {
        wgpu::Texture source =
            Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
        wgpu::Texture destination =
            Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, options);

        // Check noop copy
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {0, 0, 0}, wgpu::TextureAspect::All, options);
    }

    // Destroyed src texture.
    {
        wgpu::Texture source =
            Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
        wgpu::Texture destination =
            Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);
        source.Destroy();
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, options);

        // Check noop copy
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {0, 0, 0}, wgpu::TextureAspect::All, options);
    }

    // Destroyed dst texture.
    {
        wgpu::Texture source =
            Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
        wgpu::Texture destination =
            Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

        destination.Destroy();
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, options);

        // Check noop copy
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {0, 0, 0}, wgpu::TextureAspect::All, options);
    }
}

// Test non-zero value origin in source and OOB copy rects.
TEST_F(CopyTextureForBrowserTest, OutOfBounds) {
    wgpu::Texture source =
        Create2DTexture(device, 16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    // OOB on source
    {
        // x + width overflows
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {1, 0, 0}, destination, 0,
                                  {0, 0, 0}, {16, 16, 1});

        // y + height overflows
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 1, 0}, destination, 0,
                                  {0, 0, 0}, {16, 16, 1});

        // non-zero mip overflows
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 1, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {9, 9, 1});

        // copy to multiple slices
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 2}, {16, 16, 2});

        // copy origin z value is non-zero.
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 1}, destination, 0,
                                  {0, 0, 2}, {16, 16, 1});
    }

    // OOB on destination
    {
        // x + width overflows
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {1, 0, 0}, {16, 16, 1});

        // y + height overflows
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 1, 0}, {16, 16, 1});

        // non-zero mip overflows
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 1,
                                  {0, 0, 0}, {9, 9, 1});

        // arrayLayer + depth OOB
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 4}, {16, 16, 1});

        // empty copy on non-existent mip fails
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 6,
                                  {0, 0, 0}, {0, 0, 1});

        // empty copy on non-existent slice fails
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 4}, {0, 0, 1});
    }
}

// Test destination texture has format that not supported by CopyTextureForBrowser().
TEST_F(CopyTextureForBrowserTest, InvalidDstFormat) {
    wgpu::Texture source =
        Create2DTexture(device, 16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 2, wgpu::TextureFormat::RG8Uint,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    // Not supported dst texture format.
    TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                              {0, 0, 0}, {0, 0, 1});
}

// Test source or destination texture are multisampled.
TEST_F(CopyTextureForBrowserTest, InvalidSampleCount) {
    wgpu::Texture sourceMultiSampled1x =
        Create2DTexture(device, 16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding, 1);
    wgpu::Texture destinationMultiSampled1x =
        Create2DTexture(device, 16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment, 1);
    wgpu::Texture sourceMultiSampled4x =
        Create2DTexture(device, 16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding |
                            wgpu::TextureUsage::RenderAttachment,
                        4);
    wgpu::Texture destinationMultiSampled4x =
        Create2DTexture(device, 16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment, 4);

    // An empty copy with dst texture sample count > 1 failure.
    TestCopyTextureForBrowser(utils::Expectation::Failure, sourceMultiSampled1x, 0, {0, 0, 0},
                              destinationMultiSampled4x, 0, {0, 0, 0}, {0, 0, 1});

    // A empty copy with source texture sample count > 1 failure
    TestCopyTextureForBrowser(utils::Expectation::Failure, sourceMultiSampled4x, 0, {0, 0, 0},
                              destinationMultiSampled1x, 0, {0, 0, 0}, {0, 0, 1});
}

// Test color space conversion related attributes in CopyTextureForBrowserOptions.
TEST_F(CopyTextureForBrowserTest, ColorSpaceConversion_ColorSpace) {
    wgpu::Texture source =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    wgpu::CopyTextureForBrowserOptions options = {};
    options.needsColorSpaceConversion = true;

    // Valid cases
    {
        wgpu::CopyTextureForBrowserOptions validOptions = options;
        std::array<float, 7> srcTransferFunctionParameters = {};
        std::array<float, 7> dstTransferFunctionParameters = {};
        std::array<float, 9> conversionMatrix = {};
        validOptions.srcTransferFunctionParameters = srcTransferFunctionParameters.data();
        validOptions.dstTransferFunctionParameters = dstTransferFunctionParameters.data();
        validOptions.conversionMatrix = conversionMatrix.data();
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, validOptions);

        // if no color space conversion, no need to validate related attributes
        wgpu::CopyTextureForBrowserOptions noColorSpaceConversion = options;
        noColorSpaceConversion.needsColorSpaceConversion = false;
        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All,
                                  noColorSpaceConversion);
    }

    // Invalid cases: srcTransferFunctionParameters, dstTransferFunctionParameters or
    // conversionMatrix is nullptr or not set
    {
        // not set srcTransferFunctionParameters
        wgpu::CopyTextureForBrowserOptions invalidOptions = options;
        std::array<float, 7> dstTransferFunctionParameters = {};
        std::array<float, 9> conversionMatrix = {};
        invalidOptions.dstTransferFunctionParameters = dstTransferFunctionParameters.data();
        invalidOptions.conversionMatrix = conversionMatrix.data();
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, invalidOptions);

        // set to nullptr
        invalidOptions.srcTransferFunctionParameters = nullptr;
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, invalidOptions);
    }

    {
        // not set dstTransferFunctionParameters
        wgpu::CopyTextureForBrowserOptions invalidOptions = options;
        std::array<float, 7> srcTransferFunctionParameters = {};
        std::array<float, 9> conversionMatrix = {};
        invalidOptions.srcTransferFunctionParameters = srcTransferFunctionParameters.data();
        invalidOptions.conversionMatrix = conversionMatrix.data();
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, invalidOptions);

        // set to nullptr
        invalidOptions.dstTransferFunctionParameters = nullptr;
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, invalidOptions);
    }

    {
        // not set conversionMatrix
        wgpu::CopyTextureForBrowserOptions invalidOptions = options;
        std::array<float, 7> srcTransferFunctionParameters = {};
        std::array<float, 7> dstTransferFunctionParameters = {};
        invalidOptions.srcTransferFunctionParameters = srcTransferFunctionParameters.data();
        invalidOptions.dstTransferFunctionParameters = dstTransferFunctionParameters.data();
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, invalidOptions);

        // set to nullptr
        invalidOptions.conversionMatrix = nullptr;
        TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, invalidOptions);
    }
}

// Test option.srcAlphaMode/dstAlphaMode
TEST_F(CopyTextureForBrowserTest, ColorSpaceConversion_TextureAlphaState) {
    wgpu::Texture source =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);
    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    wgpu::CopyTextureForBrowserOptions options = {};

    // Valid src texture alpha state and valid dst texture alpha state
    {
        options.srcAlphaMode = wgpu::AlphaMode::Premultiplied;
        options.dstAlphaMode = wgpu::AlphaMode::Premultiplied;

        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, options);

        options.srcAlphaMode = wgpu::AlphaMode::Premultiplied;
        options.dstAlphaMode = wgpu::AlphaMode::Unpremultiplied;

        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, options);

        options.srcAlphaMode = wgpu::AlphaMode::Unpremultiplied;
        options.dstAlphaMode = wgpu::AlphaMode::Premultiplied;

        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, options);

        options.srcAlphaMode = wgpu::AlphaMode::Unpremultiplied;
        options.dstAlphaMode = wgpu::AlphaMode::Unpremultiplied;

        TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                                  {0, 0, 0}, {4, 4, 1}, wgpu::TextureAspect::All, options);
    }
}

// Tests should be Success
TEST_F(CopyExternalTextureForBrowserTest, Success) {
    wgpu::ExternalTexture source = CreateExternalTexture(device, 16, 16);
    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    // Different copies, including some that touch the OOB condition
    {
        // Copy a region along top left boundary
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1});

        // Copy entire texture
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {16, 16, 1});

        // Copy a region along bottom right boundary
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {8, 8, 0}, {16, 16},
                                          destination, 0, {8, 8, 0}, {8, 8, 1});

        // Copy region into mip
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 2, {0, 0, 0}, {4, 4, 1});

        // Copy between slices
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 1}, {16, 16, 1});
    }

    // Empty copies are valid
    {
        // An empty copy
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {0, 0, 1});

        // An empty copy with depth = 0
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {0, 0, 0});

        // An empty copy touching the side of the source texture
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {16, 16, 0}, {0, 0, 1});

        // An empty copy touching the side of the destination texture
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {16, 16, 0}, {0, 0, 1});
    }
}

// Test destination texture has wrong usages
TEST_F(CopyExternalTextureForBrowserTest, IncorrectUsage) {
    wgpu::ExternalTexture validSource = CreateExternalTexture(device, 16, 16);

    wgpu::Texture validDestination =
        Create2DTexture(device, 16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);
    wgpu::Texture noRenderAttachmentUsageDestination = Create2DTexture(
        device, 16, 16, 5, 2, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);
    wgpu::Texture noCopyDstUsageSource =
        Create2DTexture(device, 16, 16, 5, 2, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::RenderAttachment);

    // Incorrect destination usage causes failure: lack |RenderAttachement| usage.
    TestCopyExternalTextureForBrowser(utils::Expectation::Failure, validSource, {0, 0, 0}, {16, 16},
                                      noRenderAttachmentUsageDestination, 0, {0, 0, 0},
                                      {16, 16, 1});

    // Incorrect destination usage causes failure: lack |CopyDst| usage.
    TestCopyExternalTextureForBrowser(utils::Expectation::Failure, validSource, {0, 0, 0}, {16, 16},
                                      noCopyDstUsageSource, 0, {0, 0, 0}, {16, 16, 1});
}

// Test source or destination texture is destroyed.
TEST_F(CopyExternalTextureForBrowserTest, DestroyedTexture) {
    wgpu::CopyTextureForBrowserOptions options = {};

    // Valid src and dst textures.
    {
        wgpu::ExternalTexture source = CreateExternalTexture(device, 16, 16);
        wgpu::Texture destination =
            Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, options);

        // Check noop copy
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {0, 0, 0},
                                          wgpu::TextureAspect::All, options);
    }

    // Destroyed src texture.
    {
        wgpu::ExternalTexture source = CreateExternalTexture(device, 16, 16);
        wgpu::Texture destination =
            Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);
        source.Destroy();
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, options);

        // Check noop copy
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {0, 0, 0},
                                          wgpu::TextureAspect::All, options);
    }

    // Destroyed dst texture.
    {
        wgpu::ExternalTexture source = CreateExternalTexture(device, 16, 16);
        wgpu::Texture destination =
            Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

        destination.Destroy();
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, options);

        // Check noop copy
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {0, 0, 0},
                                          wgpu::TextureAspect::All, options);
    }
}

// Test non-zero value origin in source and OOB copy rects.
TEST_F(CopyExternalTextureForBrowserTest, OutOfBounds) {
    wgpu::ExternalTexture source = CreateExternalTexture(device, 16, 16);
    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    // OOB on source
    {
        // x + width overflows
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {1, 0, 0}, {4, 4},
                                          destination, 0, {0, 0, 0}, {4, 4, 1});

        // y + height overflows
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 1, 0}, {4, 4},
                                          destination, 0, {0, 0, 0}, {4, 4, 1});

        // copy to multiple slices
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {4, 4},
                                          destination, 0, {0, 0, 2}, {4, 4, 2});

        // copy origin z value is non-zero.
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 1}, {4, 4},
                                          destination, 0, {0, 0, 2}, {4, 4, 1});
    }

    // OOB on destination
    {
        // x + width overflows
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {1, 0, 0}, {16, 16, 1});

        // y + height overflows
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 1, 0}, {16, 16, 1});

        // non-zero mip overflows
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 1, {0, 0, 0}, {9, 9, 1});

        // arrayLayer + depth OOB
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 4}, {16, 16, 1});

        // empty copy on non-existent mip fails
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 6, {0, 0, 0}, {0, 0, 1});

        // empty copy on non-existent slice fails
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 4}, {0, 0, 1});
    }
}

// Test destination texture has format that not supported by CopyTextureForBrowser().
TEST_F(CopyExternalTextureForBrowserTest, InvalidDstFormat) {
    wgpu::ExternalTexture source = CreateExternalTexture(device, 16, 16);
    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 2, wgpu::TextureFormat::RG8Uint,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    // Not supported dst texture format.
    TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                      destination, 0, {0, 0, 0}, {0, 0, 1});
}

// Test destination texture are multisampled.
TEST_F(CopyExternalTextureForBrowserTest, InvalidSampleCount) {
    wgpu::ExternalTexture source = CreateExternalTexture(device, 16, 16);
    wgpu::Texture destinationMultiSampled4x =
        Create2DTexture(device, 16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment, 4);

    // An empty copy with dst texture sample count > 1 failure.
    TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                      destinationMultiSampled4x, 0, {0, 0, 0}, {0, 0, 1});
}

// Test color space conversion related attributes in CopyTextureForBrowserOptions.
TEST_F(CopyExternalTextureForBrowserTest, ColorSpaceConversion_ColorSpace) {
    wgpu::ExternalTexture source = CreateExternalTexture(device, 16, 16);
    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    wgpu::CopyTextureForBrowserOptions options = {};
    options.needsColorSpaceConversion = true;

    // Valid cases
    {
        wgpu::CopyTextureForBrowserOptions validOptions = options;
        std::array<float, 7> srcTransferFunctionParameters = {};
        std::array<float, 7> dstTransferFunctionParameters = {};
        std::array<float, 9> conversionMatrix = {};
        validOptions.srcTransferFunctionParameters = srcTransferFunctionParameters.data();
        validOptions.dstTransferFunctionParameters = dstTransferFunctionParameters.data();
        validOptions.conversionMatrix = conversionMatrix.data();
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, validOptions);

        // if no color space conversion, no need to validate related attributes
        wgpu::CopyTextureForBrowserOptions noColorSpaceConversion = options;
        noColorSpaceConversion.needsColorSpaceConversion = false;
        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, noColorSpaceConversion);
    }

    // Invalid cases: srcTransferFunctionParameters, dstTransferFunctionParameters or
    // conversionMatrix is nullptr or not set
    {
        // not set srcTransferFunctionParameters
        wgpu::CopyTextureForBrowserOptions invalidOptions = options;
        std::array<float, 7> dstTransferFunctionParameters = {};
        std::array<float, 9> conversionMatrix = {};
        invalidOptions.dstTransferFunctionParameters = dstTransferFunctionParameters.data();
        invalidOptions.conversionMatrix = conversionMatrix.data();
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, invalidOptions);

        // set to nullptr
        invalidOptions.srcTransferFunctionParameters = nullptr;
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, invalidOptions);
    }

    {
        // not set dstTransferFunctionParameters
        wgpu::CopyTextureForBrowserOptions invalidOptions = options;
        std::array<float, 7> srcTransferFunctionParameters = {};
        std::array<float, 9> conversionMatrix = {};
        invalidOptions.srcTransferFunctionParameters = srcTransferFunctionParameters.data();
        invalidOptions.conversionMatrix = conversionMatrix.data();
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, invalidOptions);

        // set to nullptr
        invalidOptions.dstTransferFunctionParameters = nullptr;
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, invalidOptions);
    }

    {
        // not set conversionMatrix
        wgpu::CopyTextureForBrowserOptions invalidOptions = options;
        std::array<float, 7> srcTransferFunctionParameters = {};
        std::array<float, 7> dstTransferFunctionParameters = {};
        invalidOptions.srcTransferFunctionParameters = srcTransferFunctionParameters.data();
        invalidOptions.dstTransferFunctionParameters = dstTransferFunctionParameters.data();
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, invalidOptions);

        // set to nullptr
        invalidOptions.conversionMatrix = nullptr;
        TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, invalidOptions);
    }
}

// Test option.srcAlphaMode/dstAlphaMode
TEST_F(CopyExternalTextureForBrowserTest, ColorSpaceConversion_TextureAlphaState) {
    wgpu::ExternalTexture source = CreateExternalTexture(device, 16, 16);
    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    wgpu::CopyTextureForBrowserOptions options = {};

    // Valid src texture alpha state and valid dst texture alpha state
    {
        options.srcAlphaMode = wgpu::AlphaMode::Premultiplied;
        options.dstAlphaMode = wgpu::AlphaMode::Premultiplied;

        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, options);

        options.srcAlphaMode = wgpu::AlphaMode::Premultiplied;
        options.dstAlphaMode = wgpu::AlphaMode::Unpremultiplied;

        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, options);

        options.srcAlphaMode = wgpu::AlphaMode::Unpremultiplied;
        options.dstAlphaMode = wgpu::AlphaMode::Premultiplied;

        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, options);

        options.srcAlphaMode = wgpu::AlphaMode::Unpremultiplied;
        options.dstAlphaMode = wgpu::AlphaMode::Unpremultiplied;

        TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                          destination, 0, {0, 0, 0}, {4, 4, 1},
                                          wgpu::TextureAspect::All, options);
    }
}

// Test that the internal usage can only be set to true when the device internal usage feature is
// enabled
TEST_F(CopyTextureForBrowserTest, InternalUsage) {
    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding;

    // Validation should fail because internal descriptor is not empty.
    ASSERT_DEVICE_ERROR(Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                                        wgpu::TextureUsage::CopySrc, 1, &internalDesc));

    wgpu::Texture source =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding);

    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    // Validation should fail because of device internal usage feature is missing when internal
    // usage option is on
    wgpu::CopyTextureForBrowserOptions options = {};
    options.internalUsage = true;
    TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                              {0, 0, 0}, {16, 16, 1}, wgpu::TextureAspect::All, options);
}

// Test that the internal usages are taken into account when interalUsage = true
TEST_F(CopyTextureForBrowserInternalUsageTest, InternalUsage) {
    wgpu::DawnTextureInternalUsageDescriptor internalDesc1 = {};
    internalDesc1.internalUsage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding;

    wgpu::Texture source = Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                                           wgpu::TextureUsage::CopySrc, 1, &internalDesc1);

    wgpu::DawnTextureInternalUsageDescriptor internalDesc2 = {};
    internalDesc2.internalUsage =
        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst, 1, &internalDesc2);

    // Without internal usage option should fail usage validation
    TestCopyTextureForBrowser(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                              {0, 0, 0}, {16, 16, 1});

    // With internal usage option should pass usage validation
    wgpu::CopyTextureForBrowserOptions options = {};
    options.internalUsage = true;
    TestCopyTextureForBrowser(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                              {0, 0, 0}, {16, 16, 1}, wgpu::TextureAspect::All, options);
}

// Test that the internal usage can only be set to true when the device internal usage feature is
// enabled
TEST_F(CopyExternalTextureForBrowserTest, InternalUsage) {
    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    internalDesc.internalUsage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding;

    // Validation should fail because internal descriptor is not empty.
    ASSERT_DEVICE_ERROR(Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                                        wgpu::TextureUsage::CopySrc, 1, &internalDesc));

    wgpu::ExternalTexture source = CreateExternalTexture(device, 16, 16);

    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

    // Validation should fail because of device internal usage feature is missing when internal
    // usage option is on
    wgpu::CopyTextureForBrowserOptions options = {};
    options.internalUsage = true;
    TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                      destination, 0, {0, 0, 0}, {16, 16, 1},
                                      wgpu::TextureAspect::All, options);
}

// Test that the internal usages are taken into account when interalUsage = true
TEST_F(CopyExternalTextureForBrowserInternalUsageTest, InternalUsage) {
    wgpu::ExternalTexture source = CreateExternalTexture(device, 16, 16);

    wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
    internalDesc.internalUsage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture destination =
        Create2DTexture(device, 16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst, 1, &internalDesc);

    // Without internal usage option should fail usage validation
    TestCopyExternalTextureForBrowser(utils::Expectation::Failure, source, {0, 0, 0}, {16, 16},
                                      destination, 0, {0, 0, 0}, {16, 16, 1});

    // With internal usage option should pass usage validation
    wgpu::CopyTextureForBrowserOptions options = {};
    options.internalUsage = true;
    TestCopyExternalTextureForBrowser(utils::Expectation::Success, source, {0, 0, 0}, {16, 16},
                                      destination, 0, {0, 0, 0}, {16, 16, 1},
                                      wgpu::TextureAspect::All, options);
}

}  // anonymous namespace
}  // namespace dawn
