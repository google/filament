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

#include "dawn/tests/end2end/VideoViewsTests.h"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {

VideoViewsTestBackend::PlatformTexture::PlatformTexture(wgpu::Texture&& texture)
    : wgpuTexture(texture) {}
VideoViewsTestBackend::PlatformTexture::~PlatformTexture() = default;

VideoViewsTestBackend::~VideoViewsTestBackend() = default;

constexpr std::array<utils::RGBA8, 3> VideoViewsTestsBase::kYellowYUVAColor;
constexpr std::array<utils::RGBA8, 3> VideoViewsTestsBase::kWhiteYUVAColor;
constexpr std::array<utils::RGBA8, 3> VideoViewsTestsBase::kBlueYUVAColor;
constexpr std::array<utils::RGBA8, 3> VideoViewsTestsBase::kRedYUVAColor;

void VideoViewsTestsBase::SetUp() {
    DawnTestWithParams<Params>::SetUp();
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DAWN_TEST_UNSUPPORTED_IF(!IsMultiPlanarFormatsSupported());
    // TODO(crbug.com/342213634): Crashes on ChromeOS volteer devices.
    DAWN_SUPPRESS_TEST_IF(IsChromeOS() && IsVulkan() && IsIntel() && IsBackendValidationEnabled());

    // compat mode doesn't allow different texture views to be used in a draw call unless
    // FlexibleTextureViews feature is enabled. The tests need texture views to sample and render to
    // separate planes of a multiplanar texture.
    DAWN_TEST_UNSUPPORTED_IF(IsCompatibilityMode() &&
                             !SupportsFeatures({wgpu::FeatureName::FlexibleTextureViews}));
}

std::vector<wgpu::FeatureName> VideoViewsTestsBase::GetRequiredFeatures() {
    std::vector<wgpu::FeatureName> requiredFeatures = {};
    mIsMultiPlanarFormatsSupported = SupportsFeatures({wgpu::FeatureName::DawnMultiPlanarFormats});
    if (mIsMultiPlanarFormatsSupported) {
        requiredFeatures.push_back(wgpu::FeatureName::DawnMultiPlanarFormats);
    }

    // Required for the Mac tests.
    // NOTE: It's not possible to obtain platform-specific features from
    // `mBackend` as `mBackend` is created after GetRequiredFeatures() is
    // invoked.
    if (SupportsFeatures({wgpu::FeatureName::SharedTextureMemoryIOSurface,
                          wgpu::FeatureName::SharedFenceMTLSharedEvent})) {
        requiredFeatures.push_back(wgpu::FeatureName::SharedTextureMemoryIOSurface);
        requiredFeatures.push_back(wgpu::FeatureName::SharedFenceMTLSharedEvent);
    }

    // Required for the Win tests.
    if (SupportsFeatures({wgpu::FeatureName::SharedTextureMemoryD3D11Texture2D})) {
        requiredFeatures.push_back(wgpu::FeatureName::SharedTextureMemoryD3D11Texture2D);
    }
    if (SupportsFeatures({wgpu::FeatureName::SharedTextureMemoryDXGISharedHandle})) {
        requiredFeatures.push_back(wgpu::FeatureName::SharedTextureMemoryDXGISharedHandle);
    }
    if (SupportsFeatures({wgpu::FeatureName::SharedFenceDXGISharedHandle})) {
        requiredFeatures.push_back(wgpu::FeatureName::SharedFenceDXGISharedHandle);
    }

    mIsMultiPlanarFormatP010Supported =
        SupportsFeatures({wgpu::FeatureName::MultiPlanarFormatP010});
    if (mIsMultiPlanarFormatP010Supported) {
        requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarFormatP010);
    }
    mIsMultiPlanarFormatP210Supported =
        SupportsFeatures({wgpu::FeatureName::MultiPlanarFormatP210});
    if (mIsMultiPlanarFormatP210Supported) {
        requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarFormatP210);
    }
    mIsMultiPlanarFormatP410Supported =
        SupportsFeatures({wgpu::FeatureName::MultiPlanarFormatP410});
    if (mIsMultiPlanarFormatP410Supported) {
        requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarFormatP410);
    }
    mIsMultiPlanarFormatNv16Supported =
        SupportsFeatures({wgpu::FeatureName::MultiPlanarFormatNv16});
    if (mIsMultiPlanarFormatNv16Supported) {
        requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarFormatNv16);
    }
    mIsMultiPlanarFormatNv24Supported =
        SupportsFeatures({wgpu::FeatureName::MultiPlanarFormatNv24});
    if (mIsMultiPlanarFormatNv24Supported) {
        requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarFormatNv24);
    }
    mIsMultiPlanarFormatNv12aSupported =
        SupportsFeatures({wgpu::FeatureName::MultiPlanarFormatNv12a});
    if (mIsMultiPlanarFormatNv12aSupported) {
        requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarFormatNv12a);
    }
    mIsUnorm16TextureFormatsSupported =
        SupportsFeatures({wgpu::FeatureName::Unorm16TextureFormats});
    if (mIsUnorm16TextureFormatsSupported) {
        requiredFeatures.push_back(wgpu::FeatureName::Unorm16TextureFormats);
    }
    mIsSnorm16TextureFormatsSupported =
        SupportsFeatures({wgpu::FeatureName::Snorm16TextureFormats});
    if (mIsSnorm16TextureFormatsSupported) {
        requiredFeatures.push_back(wgpu::FeatureName::Snorm16TextureFormats);
    }
    if (SupportsFeatures({wgpu::FeatureName::FlexibleTextureViews})) {
        requiredFeatures.push_back(wgpu::FeatureName::FlexibleTextureViews);
    }

    requiredFeatures.push_back(wgpu::FeatureName::DawnInternalUsages);
    return requiredFeatures;
}

bool VideoViewsTestsBase::IsMultiPlanarFormatsSupported() const {
    return mIsMultiPlanarFormatsSupported;
}

bool VideoViewsTestsBase::IsMultiPlanarFormatP010Supported() const {
    return mIsMultiPlanarFormatP010Supported;
}

bool VideoViewsTestsBase::IsMultiPlanarFormatP210Supported() const {
    return mIsMultiPlanarFormatP210Supported;
}

bool VideoViewsTestsBase::IsMultiPlanarFormatP410Supported() const {
    return mIsMultiPlanarFormatP410Supported;
}

bool VideoViewsTestsBase::IsMultiPlanarFormatNv16Supported() const {
    return mIsMultiPlanarFormatNv16Supported;
}

bool VideoViewsTestsBase::IsMultiPlanarFormatNv24Supported() const {
    return mIsMultiPlanarFormatNv24Supported;
}

bool VideoViewsTestsBase::IsMultiPlanarFormatNv12aSupported() const {
    return mIsMultiPlanarFormatNv12aSupported;
}

bool VideoViewsTestsBase::IsUnorm16TextureFormatsSupported() const {
    return mIsUnorm16TextureFormatsSupported;
}

bool VideoViewsTestsBase::IsSnorm16TextureFormatsSupported() const {
    return mIsSnorm16TextureFormatsSupported;
}

bool VideoViewsTestsBase::IsFormatSupported() const {
    if (GetFormat() == wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm) {
        // DXGI_FORMAT_P010 can't be shared between D3D11 and D3D12.
        if (IsD3D12()) {
            return false;
        }
        // DXGI_FORMAT_P010 is not supported on WARP.
        if (IsWARP()) {
            return false;
        }
        return IsUnorm16TextureFormatsSupported() && IsMultiPlanarFormatP010Supported();
    }

    if (GetFormat() == wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm) {
        return IsUnorm16TextureFormatsSupported() && IsMultiPlanarFormatP210Supported();
    }

    if (GetFormat() == wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm) {
        return IsUnorm16TextureFormatsSupported() && IsMultiPlanarFormatP410Supported();
    }

    if (GetFormat() == wgpu::TextureFormat::R8BG8Biplanar422Unorm) {
        return IsMultiPlanarFormatNv16Supported();
    }

    if (GetFormat() == wgpu::TextureFormat::R8BG8Biplanar444Unorm) {
        return IsMultiPlanarFormatNv24Supported();
    }

    if (GetFormat() == wgpu::TextureFormat::R8BG8A8Triplanar420Unorm) {
        return IsMultiPlanarFormatNv12aSupported();
    }

    return true;
}

// Returns a pre-prepared multi-planar formatted texture
// The encoded texture data represents a 4x4 converted image. When |isCheckerboard| is true,
// the top left is a 2x2 yellow block, bottom right is a 2x2 red block, top right is a 2x2
// blue block, and bottom left is a 2x2 white block. When |isCheckerboard| is false, the
// image is converted from a solid yellow 4x4 block.
// static
template <typename T>
std::vector<T> VideoViewsTestsBase::GetTestTextureData(wgpu::TextureFormat format,
                                                       bool isCheckerboard,
                                                       bool hasAlpha) {
    const uint8_t kLeftShiftBits = (sizeof(T) - 1) * 8;
    constexpr T Yy = kYellowYUVAColor[kYUVALumaPlaneIndex].r << kLeftShiftBits;
    constexpr T Yu = kYellowYUVAColor[kYUVAChromaPlaneIndex].r << kLeftShiftBits;
    constexpr T Yv = kYellowYUVAColor[kYUVAChromaPlaneIndex].g << kLeftShiftBits;
    constexpr T Ya = kYellowYUVAColor[kYUVAAlphaPlaneIndex].r << kLeftShiftBits;

    constexpr T Wy = kWhiteYUVAColor[kYUVALumaPlaneIndex].r << kLeftShiftBits;
    constexpr T Wu = kWhiteYUVAColor[kYUVAChromaPlaneIndex].r << kLeftShiftBits;
    constexpr T Wv = kWhiteYUVAColor[kYUVAChromaPlaneIndex].g << kLeftShiftBits;
    constexpr T Wa = kWhiteYUVAColor[kYUVAAlphaPlaneIndex].r << kLeftShiftBits;

    constexpr T Ry = kRedYUVAColor[kYUVALumaPlaneIndex].r << kLeftShiftBits;
    constexpr T Ru = kRedYUVAColor[kYUVAChromaPlaneIndex].r << kLeftShiftBits;
    constexpr T Rv = kRedYUVAColor[kYUVAChromaPlaneIndex].g << kLeftShiftBits;
    constexpr T Ra = kRedYUVAColor[kYUVAAlphaPlaneIndex].r << kLeftShiftBits;

    constexpr T By = kBlueYUVAColor[kYUVALumaPlaneIndex].r << kLeftShiftBits;
    constexpr T Bu = kBlueYUVAColor[kYUVAChromaPlaneIndex].r << kLeftShiftBits;
    constexpr T Bv = kBlueYUVAColor[kYUVAChromaPlaneIndex].g << kLeftShiftBits;
    constexpr T Ba = kBlueYUVAColor[kYUVAAlphaPlaneIndex].r << kLeftShiftBits;

    switch (format) {
        // The first 16 bytes is the luma plane (Y), followed by the chroma plane (UV) which
        // is half the number of bytes (subsampled by 2) but same bytes per line as luma
        // plane, then alpha plane (A) which has the same number of bytes and same
        // bytes per line as luma plane.
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
            if (isCheckerboard) {
                return {
                    Wy, Wy, Ry, Ry,  // plane 0, start + 0
                    Wy, Wy, Ry, Ry,  //
                    Yy, Yy, By, By,  //
                    Yy, Yy, By, By,  //
                    Wu, Wv, Ru, Rv,  // plane 1, start + 16
                    Yu, Yv, Bu, Bv,  //
                    Wa, Wa, Ra, Ra,  // plane 2, start + 24
                    Wa, Wa, Ra, Ra,  //
                    Ya, Ya, Ba, Ba,  //
                    Ya, Ya, Ba, Ba,  //
                };
            } else {
                return {
                    Yy, Yy, Yy, Yy,  // plane 0, start + 0
                    Yy, Yy, Yy, Yy,  //
                    Yy, Yy, Yy, Yy,  //
                    Yy, Yy, Yy, Yy,  //
                    Yu, Yv, Yu, Yv,  // plane 1, start + 16
                    Yu, Yv, Yu, Yv,  //
                    Ya, Ya, Ya, Ya,  // plane 2, start + 24
                    Ya, Ya, Ya, Ya,  //
                    Ya, Ya, Ya, Ya,  //
                    Ya, Ya, Ya, Ya,  //
                };
            }
        // The first 16 bytes is the luma plane (Y), followed by the chroma plane (UV) which
        // is half the number of bytes (subsampled by 2) but same bytes per line as luma
        // plane.
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            if (isCheckerboard) {
                return {
                    Wy, Wy, Ry, Ry,  // plane 0, start + 0
                    Wy, Wy, Ry, Ry,  //
                    Yy, Yy, By, By,  //
                    Yy, Yy, By, By,  //
                    Wu, Wv, Ru, Rv,  // plane 1, start + 16
                    Yu, Yv, Bu, Bv,  //
                };
            } else {
                return {
                    Yy, Yy, Yy, Yy,  // plane 0, start + 0
                    Yy, Yy, Yy, Yy,  //
                    Yy, Yy, Yy, Yy,  //
                    Yy, Yy, Yy, Yy,  //
                    Yu, Yv, Yu, Yv,  // plane 1, start + 16
                    Yu, Yv, Yu, Yv,  //
                };
            }
        // The first 16 bytes is the luma plane (Y), followed by the chroma plane (UV) which
        // has the same number of bytes and same bytes per line as luma
        // plane.
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
            if (isCheckerboard) {
                return {
                    Wy, Wy, Ry, Ry,  // plane 0, start + 0
                    Wy, Wy, Ry, Ry,  //
                    Yy, Yy, By, By,  //
                    Yy, Yy, By, By,  //
                    Wu, Wv, Ru, Rv,  // plane 1, start + 16
                    Wu, Wv, Ru, Rv,  //
                    Yu, Yv, Bu, Bv,  //
                    Yu, Yv, Bu, Bv,  //
                };
            } else {
                return {
                    Yy, Yy, Yy, Yy,  // plane 0, start + 0
                    Yy, Yy, Yy, Yy,  //
                    Yy, Yy, Yy, Yy,  //
                    Yy, Yy, Yy, Yy,  //
                    Yu, Yv, Yu, Yv,  // plane 1, start + 16
                    Yu, Yv, Yu, Yv,  //
                    Yu, Yv, Yu, Yv,  //
                    Yu, Yv, Yu, Yv,  //
                };
            }
        // The first 16 bytes is the luma plane (Y), followed by the chroma plane (UV) which
        // has twice the number of bytes and twice bytes per line compared to luma
        // plane.
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
            if (isCheckerboard) {
                return {
                    Wy, Wy, Ry, Ry,  // plane 0, start + 0
                    Wy, Wy, Ry, Ry,  //
                    Yy, Yy, By, By,  //
                    Yy, Yy, By, By,  //
                    Wu, Wv, Wu, Wv,  // plane 1, start + 16
                    Ru, Rv, Ru, Rv,  //
                    Wu, Wv, Wu, Wv,  //
                    Ru, Rv, Ru, Rv,  //
                    Yu, Yv, Yu, Yv,  //
                    Bu, Bv, Bu, Bv,  //
                    Yu, Yv, Yu, Yv,  //
                    Bu, Bv, Bu, Bv,  //
                };
            } else {
                return {
                    Yy, Yy, Yy, Yy,  // plane 0, start + 0
                    Yy, Yy, Yy, Yy,  //
                    Yy, Yy, Yy, Yy,  //
                    Yy, Yy, Yy, Yy,  //
                    Yu, Yv, Yu, Yv,  // plane 1, start + 16
                    Yu, Yv, Yu, Yv,  //
                    Yu, Yv, Yu, Yv,  //
                    Yu, Yv, Yu, Yv,  //
                    Yu, Yv, Yu, Yv,  //
                    Yu, Yv, Yu, Yv,  //
                    Yu, Yv, Yu, Yv,  //
                    Yu, Yv, Yu, Yv,  //
                };
            }
        case wgpu::TextureFormat::RGBA8Unorm:
            // Combines both planes by directly mapping back to RGBA: R=Y, G=U, B=V, A=A.
            if (hasAlpha && isCheckerboard) {
                return {
                    Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya, By, Bu, Bv, Ba, By, Bu, Bv, Ba,  //
                    Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya, By, Bu, Bv, Ba, By, Bu, Bv, Ba,  //
                    Wy, Wu, Wv, Wa, Wy, Wu, Wv, Wa, Ry, Ru, Rv, Ra, Ry, Ru, Rv, Ra,  //
                    Wy, Wu, Wv, Wa, Wy, Wu, Wv, Wa, Ry, Ru, Rv, Ra, Ry, Ru, Rv, Ra,  //
                };
            } else if (hasAlpha && !isCheckerboard) {
                return {
                    Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya,  //
                    Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya,  //
                    Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya,  //
                    Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya, Yy, Yu, Yv, Ya,  //
                };
            } else if (isCheckerboard) {
                return {
                    Yy, Yu, Yv, Yy, Yu, Yv, By, Bu, Bv, By, Bu, Bv,  //
                    Yy, Yu, Yv, Yy, Yu, Yv, By, Bu, Bv, By, Bu, Bv,  //
                    Wy, Wu, Wv, Wy, Wu, Wv, Ry, Ru, Rv, Ry, Ru, Rv,  //
                    Wy, Wu, Wv, Wy, Wu, Wv, Ry, Ru, Rv, Ry, Ru, Rv,  //
                };
            } else {
                return {
                    Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv,  //
                    Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv,  //
                    Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv,  //
                    Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv, Yy, Yu, Yv,  //
                };
            }
        default:
            DAWN_UNREACHABLE();
            return {};
    }
}

template std::vector<uint8_t> VideoViewsTestsBase::GetTestTextureData<uint8_t>(
    wgpu::TextureFormat format,
    bool isCheckerboard,
    bool hasAlpha);
template std::vector<uint16_t> VideoViewsTestsBase::GetTestTextureData<uint16_t>(
    wgpu::TextureFormat format,
    bool isCheckerboard,
    bool hasAlpha);

template <typename T>
std::vector<T> VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex(wgpu::TextureFormat format,
                                                                     size_t planeIndex,
                                                                     size_t bytesPerRow,
                                                                     size_t height,
                                                                     bool isCheckerboard,
                                                                     bool hasAlpha) {
    std::vector<T> texelData =
        VideoViewsTestsBase::GetTestTextureData<T>(format, isCheckerboard, hasAlpha);
    auto subsampleFactor = utils::GetMultiPlaneTextureSubsamplingFactor(format, planeIndex);
    uint32_t texelDataWidth = kYUVAImageDataWidthInTexels / subsampleFactor.horizontalFactor *
                              utils::GetMultiPlaneTextureBytesPerElement(format, planeIndex) /
                              sizeof(T);
    uint32_t texelDataHeight = kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor;

    size_t rowPitch = bytesPerRow / sizeof(T);
    std::vector<T> texels(rowPitch * height, 0);
    uint32_t planeFirstTexelOffset = 0;
    // The size of the test video frame is 4 x 4, and TexelData is 4 * 6 (4:2:0) / 4 * 8 (4:2:2) / 8
    // * 12 (4:4:4) size or (4 * 10 size if alpha exist)
    switch (planeIndex) {
        case VideoViewsTestsBase::kYUVALumaPlaneIndex:
            planeFirstTexelOffset = 0;
            break;
        case VideoViewsTestsBase::kYUVAChromaPlaneIndex:
            planeFirstTexelOffset = 16;
            break;
        case VideoViewsTestsBase::kYUVAAlphaPlaneIndex:
            planeFirstTexelOffset = 24;
            break;
        default:
            DAWN_UNREACHABLE();
            return {};
    }
    for (uint32_t i = 0; i < texelDataHeight; ++i) {
        if (i < texelDataHeight) {
            for (uint32_t j = 0; j < texelDataWidth; ++j) {
                texels[rowPitch * i + j] =
                    texelData[texelDataWidth * i + j + planeFirstTexelOffset];
            }
        }
    }
    return texels;
}

template std::vector<uint8_t> VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<uint8_t>(
    wgpu::TextureFormat format,
    size_t planeIndex,
    size_t bytesPerRow,
    size_t height,
    bool isCheckerboard,
    bool hasAlpha);
template std::vector<uint16_t> VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<uint16_t>(
    wgpu::TextureFormat format,
    size_t planeIndex,
    size_t bytesPerRow,
    size_t height,
    bool isCheckerboard,
    bool hasAlpha);

wgpu::TextureFormat VideoViewsTestsBase::GetFormat() const {
    return GetParam().mFormat;
}

wgpu::TextureFormat VideoViewsTestsBase::GetPlaneFormat(int plane) const {
    switch (utils::GetMultiPlaneTextureBitDepth(GetFormat())) {
        case 8:
            return plane == 1 ? wgpu::TextureFormat::RG8Unorm : wgpu::TextureFormat::R8Unorm;
        case 16:
            return plane == 1 ? wgpu::TextureFormat::RG16Unorm : wgpu::TextureFormat::R16Unorm;
        default:
            DAWN_UNREACHABLE();
            return wgpu::TextureFormat::Undefined;
    }
}

wgpu::TextureAspect VideoViewsTestsBase::GetPlaneAspect(int plane) const {
    switch (plane) {
        case VideoViewsTestsBase::kYUVALumaPlaneIndex:
            return wgpu::TextureAspect::Plane0Only;
        case VideoViewsTestsBase::kYUVAChromaPlaneIndex:
            return wgpu::TextureAspect::Plane1Only;
        case VideoViewsTestsBase::kYUVAAlphaPlaneIndex:
            return wgpu::TextureAspect::Plane2Only;
        default:
            DAWN_UNREACHABLE();
            return wgpu::TextureAspect::All;
    }
}

// Vertex shader used to render a sampled texture into a quad.
wgpu::ShaderModule VideoViewsTestsBase::GetTestVertexShaderModule() const {
    return utils::CreateShaderModule(device, R"(
                struct VertexOut {
                    @location(0) texCoord : vec2 <f32>,
                    @builtin(position) position : vec4f,
                }

                @vertex
                fn main(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
                    var pos = array(
                        vec2f(-1.0, 1.0),
                        vec2f(-1.0, -1.0),
                        vec2f(1.0, -1.0),
                        vec2f(-1.0, 1.0),
                        vec2f(1.0, -1.0),
                        vec2f(1.0, 1.0)
                    );
                    var output : VertexOut;
                    output.position = vec4f(pos[VertexIndex], 0.0, 1.0);
                    output.texCoord = vec2f(output.position.xy * 0.5) + vec2f(0.5, 0.5);
                    return output;
            })");
}

class VideoViewsTests : public VideoViewsTestsBase {
  protected:
    void SetUp() override {
        VideoViewsTestsBase::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        DAWN_TEST_UNSUPPORTED_IF(!IsMultiPlanarFormatsSupported());
        DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

        mBackend = VideoViewsTestBackend::Create();
        mBackend->OnSetUp(device);
    }

    void TearDown() override {
        if (mBackend) {
            mBackend->OnTearDown();
            mBackend = nullptr;
        }
        VideoViewsTestsBase::TearDown();
    }
    std::unique_ptr<VideoViewsTestBackend> mBackend;
};

namespace {

// Create video texture uninitialized.
TEST_P(VideoViewsTests, CreateVideoTextureWithoutInitializedData) {
    ASSERT_DEVICE_ERROR(std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
                            mBackend->CreateVideoTextureForTest(GetFormat(),
                                                                wgpu::TextureUsage::TextureBinding,
                                                                /*isCheckerboard*/ false,
                                                                /*initialized*/ false));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Samples the luminance (Y) plane from an imported bi-planar 420 texture into a single channel of
// an RGBA output attachment and checks for the expected pixel value in the rendered quad.
TEST_P(VideoViewsTests, SampleYtoR) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ false,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }
    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.format = GetPlaneFormat(0);
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView textureView = platformTexture->wgpuTexture.CreateView(&viewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(texture, sampler0, texCoord).r;
               return vec4f(y, 0.0, 0.0, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, sampler}, {1, textureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Test the luma plane in the top left corner of RGB image.
    EXPECT_TEXTURE_EQ(&kYellowYUVAColor[kYUVALumaPlaneIndex], renderPass.color, {0, 0}, {1, 1}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Samples the chrominance (UV) plane from an imported bi-planar 420 texture into two channels of an
// RGBA output attachment and checks for the expected pixel value in the rendered quad.
TEST_P(VideoViewsTests, SampleUVtoRG) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ false,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    wgpu::TextureViewDescriptor viewDesc;
    viewDesc.format = GetPlaneFormat(1);
    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView textureView = platformTexture->wgpuTexture.CreateView(&viewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var texture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let u : f32 = textureSample(texture, sampler0, texCoord).r;
               let v : f32 = textureSample(texture, sampler0, texCoord).g;
               return vec4f(u, v, 0.0, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;
    renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, sampler}, {1, textureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Test the chroma plane in the top left corner of RGB image.
    EXPECT_TEXTURE_EQ(&kYellowYUVAColor[kYUVAChromaPlaneIndex], renderPass.color, {0, 0}, {1, 1}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Renders a "checkerboard" texture into a RGB quad, then checks the the entire
// contents to ensure the image has not been flipped.
TEST_P(VideoViewsTests, SampleYUVtoRGB) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }
    const bool hasAlpha = utils::GetMultiPlaneTextureNumPlanes(GetFormat()) > 2;
    if (hasAlpha) {
        GTEST_SKIP() << "Skipped because format is not YUV.";
    }

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = platformTexture->wgpuTexture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = platformTexture->wgpuTexture.CreateView(&chromaViewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var lumaTexture : texture_2d<f32>;
            @group(0) @binding(2) var chromaTexture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(lumaTexture, sampler0, texCoord).r;
               let u : f32 = textureSample(chromaTexture, sampler0, texCoord).r;
               let v : f32 = textureSample(chromaTexture, sampler0, texCoord).g;
               return vec4f(y, u, v, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(
            0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                    {{0, sampler}, {1, lumaTextureView}, {2, chromaTextureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint8_t> expectedData = GetTestTextureData<uint8_t>(
        wgpu::TextureFormat::RGBA8Unorm, /*isCheckerboard=*/true, hasAlpha);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 3) {
        expectedRGBA.push_back({expectedData[i], expectedData[i + 1], expectedData[i + 2], 0xFF});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Renders a "checkerboard" texture into a RGBA quad, then checks the the entire
// contents to ensure the image has not been flipped.
TEST_P(VideoViewsTests, SampleYUVAtoRGBA) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    const bool hasAlpha = utils::GetMultiPlaneTextureNumPlanes(GetFormat()) > 2;
    if (!hasAlpha) {
        GTEST_SKIP() << "Skipped because format is not YUVA.";
    }

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = platformTexture->wgpuTexture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = platformTexture->wgpuTexture.CreateView(&chromaViewDesc);

    wgpu::TextureViewDescriptor alphaViewDesc;
    alphaViewDesc.format = GetPlaneFormat(2);
    alphaViewDesc.aspect = wgpu::TextureAspect::Plane2Only;
    wgpu::TextureView alphaTextureView = platformTexture->wgpuTexture.CreateView(&alphaViewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var lumaTexture : texture_2d<f32>;
            @group(0) @binding(2) var chromaTexture : texture_2d<f32>;
            @group(0) @binding(3) var alphaTexture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(lumaTexture, sampler0, texCoord).r;
               let u : f32 = textureSample(chromaTexture, sampler0, texCoord).r;
               let v : f32 = textureSample(chromaTexture, sampler0, texCoord).g;
               let a : f32 = textureSample(alphaTexture, sampler0, texCoord).r;
               return vec4f(y, u, v, a);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, sampler},
                                                   {1, lumaTextureView},
                                                   {2, chromaTextureView},
                                                   {3, alphaTextureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint8_t> expectedData = GetTestTextureData<uint8_t>(
        wgpu::TextureFormat::RGBA8Unorm, /*isCheckerboard=*/true, hasAlpha);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 4) {
        expectedRGBA.push_back(
            {expectedData[i], expectedData[i + 1], expectedData[i + 2], expectedData[i + 3]});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Renders a "checkerboard" texture into a RGB quad with two samplers, then checks the the
// entire contents to ensure the image has not been flipped.
TEST_P(VideoViewsTests, SampleYUVtoRGBMultipleSamplers) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }
    const bool hasAlpha = utils::GetMultiPlaneTextureNumPlanes(GetFormat()) > 2;
    if (hasAlpha) {
        GTEST_SKIP() << "Skipped because format is not YUV.";
    }

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = platformTexture->wgpuTexture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = platformTexture->wgpuTexture.CreateView(&chromaViewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var sampler1 : sampler;
            @group(0) @binding(2) var lumaTexture : texture_2d<f32>;
            @group(0) @binding(3) var chromaTexture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(lumaTexture, sampler0, texCoord).r;
               let u : f32 = textureSample(chromaTexture, sampler1, texCoord).r;
               let v : f32 = textureSample(chromaTexture, sampler1, texCoord).g;
               return vec4f(y, u, v, 1.0);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler0 = device.CreateSampler();
    wgpu::Sampler sampler1 = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(
            0, utils::MakeBindGroup(
                   device, renderPipeline.GetBindGroupLayout(0),
                   {{0, sampler0}, {1, sampler1}, {2, lumaTextureView}, {3, chromaTextureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint8_t> expectedData = GetTestTextureData<uint8_t>(
        wgpu::TextureFormat::RGBA8Unorm, /*isCheckerboard=*/true, hasAlpha);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 3) {
        expectedRGBA.push_back({expectedData[i], expectedData[i + 1], expectedData[i + 2], 0xff});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Renders a "checkerboard" texture into a RGBA quad with three samplers, then checks the the
// entire contents to ensure the image has not been flipped.
TEST_P(VideoViewsTests, SampleYUVAtoRGBAMultipleSamplers) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }
    const bool hasAlpha = utils::GetMultiPlaneTextureNumPlanes(GetFormat()) > 2;
    if (!hasAlpha) {
        GTEST_SKIP() << "Skipped because format is not YUVA.";
    }

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = platformTexture->wgpuTexture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = platformTexture->wgpuTexture.CreateView(&chromaViewDesc);

    wgpu::TextureViewDescriptor alphaViewDesc;
    alphaViewDesc.format = GetPlaneFormat(2);
    alphaViewDesc.aspect = wgpu::TextureAspect::Plane2Only;
    wgpu::TextureView alphaTextureView = platformTexture->wgpuTexture.CreateView(&alphaViewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var sampler1 : sampler;
            @group(0) @binding(2) var sampler2 : sampler;
            @group(0) @binding(3) var lumaTexture : texture_2d<f32>;
            @group(0) @binding(4) var chromaTexture : texture_2d<f32>;
            @group(0) @binding(5) var alphaTexture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(lumaTexture, sampler0, texCoord).r;
               let u : f32 = textureSample(chromaTexture, sampler1, texCoord).r;
               let v : f32 = textureSample(chromaTexture, sampler1, texCoord).g;
               let a : f32 = textureSample(alphaTexture, sampler2, texCoord).r;
               return vec4f(y, u, v, a);
            })");

    utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(
        device, kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler0 = device.CreateSampler();
    wgpu::Sampler sampler1 = device.CreateSampler();
    wgpu::Sampler sampler2 = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, sampler0},
                                                   {1, sampler1},
                                                   {2, sampler2},
                                                   {3, lumaTextureView},
                                                   {4, chromaTextureView},
                                                   {5, alphaTextureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint8_t> expectedData = GetTestTextureData<uint8_t>(
        wgpu::TextureFormat::RGBA8Unorm, /*isCheckerboard=*/true, hasAlpha);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 4) {
        expectedRGBA.push_back(
            {expectedData[i], expectedData[i + 1], expectedData[i + 2], expectedData[i + 3]});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

class VideoViewsValidationTests : public VideoViewsTests {
  protected:
    void SetUp() override {
        VideoViewsTests::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));
    }
};

// Test explicitly creating a multiplanar format is not allowed
TEST_P(VideoViewsValidationTests, ExplicitCreation) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 1;
    descriptor.size.height = 1;
    descriptor.format = GetFormat();
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
}

// Test YUV texture view creation rules.
TEST_P(VideoViewsValidationTests, CreateYUVViewValidation) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }
    const bool hasAlpha = utils::GetMultiPlaneTextureNumPlanes(GetFormat()) > 2;
    if (hasAlpha) {
        GTEST_SKIP() << "Skipped because format is not YUV.";
    }

    wgpu::TextureViewDescriptor viewDesc = {};

    // Success case: Per plane view formats unspecified.
    {
        viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
        wgpu::TextureView plane0View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
        wgpu::TextureView plane1View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        ASSERT_NE(plane0View.Get(), nullptr);
        ASSERT_NE(plane1View.Get(), nullptr);
    }

    // Success case: Per plane view formats specified and aspect.
    {
        viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
        viewDesc.format = GetPlaneFormat(0);
        wgpu::TextureView plane0View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
        viewDesc.format = GetPlaneFormat(1);
        wgpu::TextureView plane1View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        ASSERT_NE(plane0View.Get(), nullptr);
        ASSERT_NE(plane1View.Get(), nullptr);
    }

    // Some valid view format, but no plane specified.
    viewDesc = {};
    viewDesc.format = GetPlaneFormat(0);
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Some valid view format, but no plane specified.
    viewDesc = {};
    viewDesc.format = GetPlaneFormat(1);
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Correct plane index but incompatible view format.
    viewDesc.format = wgpu::TextureFormat::R8Uint;
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Compatible view format but wrong plane index.
    viewDesc.format = GetPlaneFormat(0);
    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Compatible view format but wrong aspect.
    viewDesc.format = GetPlaneFormat(0);
    viewDesc.aspect = wgpu::TextureAspect::All;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Compatible view format but wrong aspect (due to defaulting).
    viewDesc.format = GetPlaneFormat(0);
    viewDesc.aspect = wgpu::TextureAspect::Undefined;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Create a single plane texture.
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.usage = wgpu::TextureUsage::TextureBinding;
    desc.size = {1, 1, 1};

    wgpu::Texture texture = device.CreateTexture(&desc);

    // Plane aspect specified with non-planar texture.
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    // Planar views with non-planar texture.
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    viewDesc.format = GetPlaneFormat(0);
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    viewDesc.format = GetPlaneFormat(1);
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test YUVA texture view creation rules.
TEST_P(VideoViewsValidationTests, CreateYUVAViewValidation) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }
    const bool hasAlpha = utils::GetMultiPlaneTextureNumPlanes(GetFormat()) > 2;
    if (!hasAlpha) {
        GTEST_SKIP() << "Skipped because format is not YUVA.";
    }

    wgpu::TextureViewDescriptor viewDesc = {};

    // Success case: Per plane view formats unspecified.
    {
        viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
        wgpu::TextureView plane0View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
        wgpu::TextureView plane1View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        viewDesc.aspect = wgpu::TextureAspect::Plane2Only;
        wgpu::TextureView plane2View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        ASSERT_NE(plane0View.Get(), nullptr);
        ASSERT_NE(plane1View.Get(), nullptr);
        ASSERT_NE(plane2View.Get(), nullptr);
    }

    // Success case: Per plane view formats specified and aspect.
    {
        viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
        viewDesc.format = GetPlaneFormat(0);
        wgpu::TextureView plane0View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
        viewDesc.format = GetPlaneFormat(1);
        wgpu::TextureView plane1View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        viewDesc.aspect = wgpu::TextureAspect::Plane2Only;
        viewDesc.format = GetPlaneFormat(2);
        wgpu::TextureView plane2View = platformTexture->wgpuTexture.CreateView(&viewDesc);

        ASSERT_NE(plane0View.Get(), nullptr);
        ASSERT_NE(plane1View.Get(), nullptr);
        ASSERT_NE(plane2View.Get(), nullptr);
    }

    // Some valid view format, but no plane specified.
    viewDesc = {};
    viewDesc.format = GetPlaneFormat(0);
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Some valid view format, but no plane specified.
    viewDesc = {};
    viewDesc.format = GetPlaneFormat(1);
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Some valid view format, but no plane specified.
    viewDesc = {};
    viewDesc.format = GetPlaneFormat(2);
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Correct plane index but incompatible view format.
    viewDesc.format = wgpu::TextureFormat::R8Uint;
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Compatible view format but wrong plane index.
    viewDesc.format = GetPlaneFormat(0);
    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Compatible view format but wrong plane index.
    viewDesc.format = GetPlaneFormat(1);
    viewDesc.aspect = wgpu::TextureAspect::Plane2Only;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Compatible view format but wrong aspect.
    viewDesc.format = GetPlaneFormat(0);
    viewDesc.aspect = wgpu::TextureAspect::All;
    ASSERT_DEVICE_ERROR(platformTexture->wgpuTexture.CreateView(&viewDesc));

    // Create a single plane texture.
    wgpu::TextureDescriptor desc;
    desc.format = wgpu::TextureFormat::RGBA8Unorm;
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.usage = wgpu::TextureUsage::TextureBinding;
    desc.size = {1, 1, 1};

    wgpu::Texture texture = device.CreateTexture(&desc);

    // Plane aspect specified with non-planar texture.
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    viewDesc.aspect = wgpu::TextureAspect::Plane2Only;
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    // Planar views with non-planar texture.
    viewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    viewDesc.format = GetPlaneFormat(0);
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    viewDesc.format = GetPlaneFormat(1);
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    viewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    viewDesc.format = GetPlaneFormat(2);
    ASSERT_DEVICE_ERROR(texture.CreateView(&viewDesc));

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test copying from one multi-planar format into another fails.
TEST_P(VideoViewsValidationTests, T2TCopyAllAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture1 =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture2 =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::Texture srcTexture = platformTexture1->wgpuTexture;
    wgpu::Texture dstTexture = platformTexture2->wgpuTexture;

    wgpu::TexelCopyTextureInfo copySrc =
        utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0});

    wgpu::TexelCopyTextureInfo copyDst =
        utils::CreateTexelCopyTextureInfo(dstTexture, 0, {0, 0, 0});

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR(encoder.Finish());

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture1));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture2));
}

// Test copying from one multi-planar format into another per plane fails.
TEST_P(VideoViewsValidationTests, T2TCopyPlaneAspectFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture1 =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture2 =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::Texture srcTexture = platformTexture1->wgpuTexture;
    wgpu::Texture dstTexture = platformTexture2->wgpuTexture;

    wgpu::TexelCopyTextureInfo copySrc = utils::CreateTexelCopyTextureInfo(
        srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::TexelCopyTextureInfo copyDst = utils::CreateTexelCopyTextureInfo(
        dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane1Only);

    wgpu::Extent3D copySize = {1, 1, 1};

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    copySrc = utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0},
                                                wgpu::TextureAspect::Plane1Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture1));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture2));
}

// Test copying from a multi-planar format to a buffer fails.
TEST_P(VideoViewsValidationTests, T2BCopyAllAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    wgpu::Texture srcTexture = platformTexture->wgpuTexture;

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 256;
    bufferDescriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::TexelCopyTextureInfo copySrc =
        utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0});

    wgpu::TexelCopyBufferInfo copyDst = utils::CreateTexelCopyBufferInfo(dstBuffer, 0, 256);

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR(encoder.Finish());

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test copying from multi-planar format per plane to a buffer fails.
TEST_P(VideoViewsValidationTests, T2BCopyPlaneAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    wgpu::Texture srcTexture = platformTexture->wgpuTexture;

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 256;
    bufferDescriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::TexelCopyTextureInfo copySrc = utils::CreateTexelCopyTextureInfo(
        srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::TexelCopyBufferInfo copyDst = utils::CreateTexelCopyBufferInfo(dstBuffer, 0, 256);

    wgpu::Extent3D copySize = {1, 1, 1};

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    copySrc = utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0},
                                                wgpu::TextureAspect::Plane1Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test copying from a buffer to a multi-planar format fails.
TEST_P(VideoViewsValidationTests, B2TCopyAllAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    wgpu::Texture dstTexture = platformTexture->wgpuTexture;

    std::vector<uint8_t> placeholderData(4, 0);

    wgpu::Buffer srcBuffer = utils::CreateBufferFromData(
        device, placeholderData.data(), placeholderData.size(), wgpu::BufferUsage::CopySrc);

    wgpu::TexelCopyBufferInfo copySrc = utils::CreateTexelCopyBufferInfo(srcBuffer, 0, 12, 4);

    wgpu::TexelCopyTextureInfo copyDst =
        utils::CreateTexelCopyTextureInfo(dstTexture, 0, {0, 0, 0});

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR(encoder.Finish());

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Test copying from a buffer to a multi-planar format per plane fails.
TEST_P(VideoViewsValidationTests, B2TCopyPlaneAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    wgpu::Texture dstTexture = platformTexture->wgpuTexture;

    std::vector<uint8_t> placeholderData(4, 0);

    wgpu::Buffer srcBuffer = utils::CreateBufferFromData(
        device, placeholderData.data(), placeholderData.size(), wgpu::BufferUsage::CopySrc);

    wgpu::TexelCopyBufferInfo copySrc = utils::CreateTexelCopyBufferInfo(srcBuffer, 0, 12, 4);

    wgpu::TexelCopyTextureInfo copyDst = utils::CreateTexelCopyTextureInfo(
        dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::Extent3D copySize = {1, 1, 1};

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    copyDst = utils::CreateTexelCopyTextureInfo(dstTexture, 0, {0, 0, 0},
                                                wgpu::TextureAspect::Plane1Only);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests which multi-planar formats are allowed to be sampled.
TEST_P(VideoViewsValidationTests, SamplingMultiPlanarTexture) {
    wgpu::BindGroupLayout layout = utils::MakeBindGroupLayout(
        device, {{0, wgpu::ShaderStage::Fragment, wgpu::TextureSampleType::Float}});

    // R8BG8Biplanar420Unorm is allowed to be sampled, if plane 0 or plane 1 is selected.
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::TextureViewDescriptor desc = {};

    desc.aspect = wgpu::TextureAspect::Plane0Only;
    utils::MakeBindGroup(device, layout, {{0, platformTexture->wgpuTexture.CreateView(&desc)}});

    desc.aspect = wgpu::TextureAspect::Plane1Only;
    utils::MakeBindGroup(device, layout, {{0, platformTexture->wgpuTexture.CreateView(&desc)}});

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests creating a texture with a multi-plane format.
TEST_P(VideoViewsValidationTests, RenderAttachmentInvalid) {
    // "Invalid Texture" error is expected if failed to create the video texture.
    EXPECT_CALL(mDeviceErrorCallback,
                Call(CHandleIs(device.Get()), testing::Ne(wgpu::ErrorType::NoError),
                     testing::HasSubstr("Invalid Texture")))
        .Times(testing::AnyNumber());

    // multi-planar formats are NOT allowed to be renderable by default and require
    // Feature::MultiPlanarRenderTargets.
    // "RenderAttachment is incompatible with the non-renderable format" error is expected.
    ASSERT_DEVICE_ERROR_MSG(auto platformTexture = mBackend->CreateVideoTextureForTest(
                                GetFormat(), wgpu::TextureUsage::RenderAttachment,
                                /*isCheckerboard*/ true,
                                /*initialized*/ true),
                            testing::HasSubstr("is incompatible"));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests writing into a multi-planar format fails.
TEST_P(VideoViewsValidationTests, WriteTextureAllAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::TexelCopyBufferLayout texelCopyBufferLayout = utils::CreateTexelCopyBufferLayout(0, 4, 4);

    wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
        utils::CreateTexelCopyTextureInfo(platformTexture->wgpuTexture, 0, {0, 0, 0});

    std::vector<uint8_t> placeholderData(4, 0);
    wgpu::Extent3D writeSize = {1, 1, 1};

    wgpu::Queue queue = device.GetQueue();

    ASSERT_DEVICE_ERROR(queue.WriteTexture(&texelCopyTextureInfo, placeholderData.data(),
                                           placeholderData.size(), &texelCopyBufferLayout,
                                           &writeSize));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests writing into a multi-planar format per plane fails.
TEST_P(VideoViewsValidationTests, WriteTexturePlaneAspectsFails) {
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
        utils::CreateTexelCopyBufferLayout(0, 12, 4);
    wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
        platformTexture->wgpuTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    std::vector<uint8_t> placeholderData(4, 0);
    wgpu::Extent3D writeSize = {1, 1, 1};

    wgpu::Queue queue = device.GetQueue();

    ASSERT_DEVICE_ERROR(queue.WriteTexture(&texelCopyTextureInfo, placeholderData.data(),
                                           placeholderData.size(), &texelCopyBufferLayout,
                                           &writeSize));
    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

class VideoViewsRenderTargetTests : public VideoViewsValidationTests {
  protected:
    void SetUp() override {
        VideoViewsValidationTests::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(!IsMultiPlanarFormatsSupported());

        DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::MultiPlanarRenderTargets));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = VideoViewsTests::GetRequiredFeatures();
        if (SupportsFeatures({wgpu::FeatureName::MultiPlanarRenderTargets})) {
            requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarRenderTargets);
        }
        return requiredFeatures;
    }

    template <typename T>
    wgpu::Texture CreatePlaneTextureWithData(wgpu::TextureFormat format,
                                             int planeIndex,
                                             bool hasAlpha) {
        auto subsampleFactor = utils::GetMultiPlaneTextureSubsamplingFactor(format, planeIndex);
        wgpu::Extent3D size = {kYUVAImageDataWidthInTexels / subsampleFactor.horizontalFactor,
                               kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor, 1};

        // Create source texture with plane format
        wgpu::TextureDescriptor planeTextureDesc;
        planeTextureDesc.size = size;
        planeTextureDesc.format = GetPlaneFormat(planeIndex);
        planeTextureDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding;
        wgpu::Texture planeTexture = device.CreateTexture(&planeTextureDesc);

        // Copy plane (Y/UV/A) data to the plane source texture.
        std::vector<T> planeSrcData = VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<T>(
            format, planeIndex, kTextureBytesPerRowAlignment,
            kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor, false, hasAlpha);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(planeTexture);
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(0, kTextureBytesPerRowAlignment);
        wgpu::Queue queue = device.GetQueue();
        queue.WriteTexture(&texelCopyTextureInfo, planeSrcData.data(),
                           planeSrcData.size() * sizeof(T), &texelCopyBufferLayout, &size);

        return planeTexture;
    }

    // Tests for rendering to a multiplanar video texture through its views. It creates R/RG source
    // textures with data which are then read into luma and chroma texture views. Since multiplanar
    // textures don't support copy operations yet, the test renders from the luma/chroma texture
    // views into another R/RG textures which are then compared with for rendered data.
    template <typename T>
    void RenderToMultiplanarVideoTextures(size_t numOfTextures, bool hasDepth) {
        const bool hasAlpha = utils::GetMultiPlaneTextureNumPlanes(GetFormat()) > 2;

        // Create source texture with plane 0 format i.e. R8/R16Unorm.
        wgpu::Texture plane0Texture =
            CreatePlaneTextureWithData<T>(GetFormat(), kYUVALumaPlaneIndex, hasAlpha);
        ASSERT_NE(plane0Texture.Get(), nullptr);
        // Create source texture with plane 1 format i.e. RG8/RG16Unorm.
        wgpu::Texture plane1Texture =
            CreatePlaneTextureWithData<T>(GetFormat(), kYUVAChromaPlaneIndex, hasAlpha);
        ASSERT_NE(plane1Texture.Get(), nullptr);
        wgpu::Texture plane2Texture;
        if (hasAlpha) {
            // Create source texture with plane 2 format i.e. R8.
            plane2Texture =
                CreatePlaneTextureWithData<T>(GetFormat(), kYUVAAlphaPlaneIndex, hasAlpha);
            ASSERT_NE(plane2Texture.Get(), nullptr);
        }

        // TODO(dawn:1337): Allow creating uninitialized texture for rendering.
        // Create a video texture to be rendered into with multi-planar format.
        std::vector<std::unique_ptr<VideoViewsTestBackend::PlatformTexture>> destVideoTextures(
            numOfTextures);
        std::vector<wgpu::Texture> destVideoWGPUTextures;
        for (auto& destVideoTexture : destVideoTextures) {
            destVideoTexture = mBackend->CreateVideoTextureForTest(
                GetFormat(),
                wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
                /*isCheckerboard*/ false,
                /*initialized*/ true);
            ASSERT_NE(destVideoTexture.get(), nullptr);
            if (!destVideoTexture->CanWrapAsWGPUTexture()) {
                mBackend->DestroyVideoTextureForTest(std::move(destVideoTexture));
                GTEST_SKIP() << "Skipped because not supported.";
            }
            destVideoWGPUTextures.push_back(destVideoTexture->wgpuTexture);
        }

        wgpu::Texture depthTexture;
        wgpu::TextureView depthTextureView;
        if (hasDepth) {
            // Create a full size depth texture, and use it for render all planes.
            wgpu::Extent3D size = {kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels, 1};
            wgpu::TextureDescriptor desc;
            desc.size = size;
            desc.format = wgpu::TextureFormat::Depth24PlusStencil8;
            desc.usage = wgpu::TextureUsage::RenderAttachment;
            depthTexture = device.CreateTexture(&desc);
            depthTextureView = depthTexture.CreateView();
        }

        // Perform plane operations for testing by creating render passes and comparing textures.
        auto PerformPlaneOperations = [this, destVideoWGPUTextures, hasDepth, depthTextureView](
                                          wgpu::TextureFormat format, int planeIndex,
                                          wgpu::Texture planeTextureWithData, bool hasAlpha) {
            auto subsampleFactor = utils::GetMultiPlaneTextureSubsamplingFactor(format, planeIndex);
            auto vsModule = GetTestVertexShaderModule();

            std::string outputStruct;
            {
                std::ostringstream result;
                result << "struct Output {\n";
                for (size_t i = 0; i < destVideoWGPUTextures.size(); ++i) {
                    result << "    @location(" << i << ") color" << i << " : vec4f,\n";
                }
                result << "};\n";
                outputStruct = std::move(result).str();
            }

            // var o: Output;
            std::string returnOutput;
            {
                std::ostringstream result;
                result << "    var output : Output;\n";
                for (size_t i = 0; i < destVideoWGPUTextures.size(); ++i) {
                    result << "    output.color" << i << " = outputColor;\n";
                }
                result << "    return output;\n";
                returnOutput = std::move(result).str();
            }

            std::ostringstream fsSource;
            fsSource << "@group(0) @binding(0) var sampler0 : sampler;\n";
            fsSource << "@group(0) @binding(1) var texture : texture_2d<f32>;\n";
            fsSource << outputStruct << "\n";
            fsSource << "@fragment\n";
            fsSource << "fn main(@location(0) texCoord : vec2f) -> Output {\n";
            fsSource << "    let outputColor = textureSample(texture, sampler0, texCoord);\n";
            fsSource << returnOutput << "\n";
            fsSource << "}\n";

            auto fsModule = utils::CreateShaderModule(device, std::move(fsSource).str());

            // Create plane texture view from the multiplanar video texture.
            wgpu::TextureViewDescriptor planeViewDesc;
            planeViewDesc.format = GetPlaneFormat(planeIndex);
            planeViewDesc.aspect = GetPlaneAspect(planeIndex);
            std::vector<wgpu::TextureView> planeTextureViews;
            for (auto& destVideoWGPUTexture : destVideoWGPUTextures) {
                planeTextureViews.push_back(destVideoWGPUTexture.CreateView(&planeViewDesc));
            }

            wgpu::Sampler sampler = device.CreateSampler();

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            {
                utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
                renderPipelineDescriptor.vertex.module = vsModule;
                renderPipelineDescriptor.cFragment.module = fsModule;
                renderPipelineDescriptor.cFragment.targetCount = destVideoWGPUTextures.size();
                renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                for (size_t i = 0; i < destVideoWGPUTextures.size(); ++i) {
                    renderPipelineDescriptor.cTargets[i].format = GetPlaneFormat(planeIndex);
                }
                if (hasDepth) {
                    renderPipelineDescriptor.EnableDepthStencil(
                        wgpu::TextureFormat::Depth24PlusStencil8);
                }
                wgpu::RenderPipeline renderPipeline =
                    device.CreateRenderPipeline(&renderPipelineDescriptor);

                // Render pass operations for reading the source data from planeTexture view into
                // planeTextureView created from the multiplanar video texture.
                utils::ComboRenderPassDescriptor renderPass(planeTextureViews, depthTextureView);
                wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
                pass.SetPipeline(renderPipeline);
                pass.SetBindGroup(0, utils::MakeBindGroup(
                                         device, renderPipeline.GetBindGroupLayout(0),
                                         {{0, sampler}, {1, planeTextureWithData.CreateView()}}));
                pass.Draw(6);
                pass.End();
            }

            std::vector<wgpu::Texture> resultTextures;

            for (auto& planeTextureView : planeTextureViews) {
                utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
                renderPipelineDescriptor.vertex.module = vsModule;
                renderPipelineDescriptor.cFragment.module = fsModule;
                renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                renderPipelineDescriptor.cTargets[0].format = GetPlaneFormat(planeIndex);
                wgpu::RenderPipeline renderPipeline =
                    device.CreateRenderPipeline(&renderPipelineDescriptor);
                // Another render pass for reading the planeTextureView into a texture of the
                // plane's format (i.e. R8/R16Unorm for Y and RG8/RG16Unorm for UV). This is needed
                // as multiplanar textures do not support copy operations.
                utils::BasicRenderPass basicRenderPass = utils::CreateBasicRenderPass(
                    device, kYUVAImageDataWidthInTexels / subsampleFactor.horizontalFactor,
                    kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor,
                    GetPlaneFormat(planeIndex));
                wgpu::RenderPassEncoder secondPass =
                    encoder.BeginRenderPass(&basicRenderPass.renderPassInfo);
                secondPass.SetPipeline(renderPipeline);
                secondPass.SetBindGroup(
                    0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                            {{0, sampler}, {1, planeTextureView}}));
                secondPass.Draw(6);
                secondPass.End();

                resultTextures.push_back(basicRenderPass.color);
            }

            // Submit all commands for the encoder.
            wgpu::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);

            std::vector<T> expectedData = VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<T>(
                format, planeIndex,
                kYUVAImageDataWidthInTexels / subsampleFactor.horizontalFactor *
                    utils::GetMultiPlaneTextureBytesPerElement(format, planeIndex),
                kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor, false, hasAlpha);

            for (auto& resultTexture : resultTextures) {
                EXPECT_TEXTURE_EQ(expectedData.data(), resultTexture, {0, 0},
                                  {kYUVAImageDataWidthInTexels / subsampleFactor.horizontalFactor,
                                   kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor},
                                  GetPlaneFormat(planeIndex));
            }
        };

        // Perform operations for the Y plane.
        PerformPlaneOperations(GetFormat(), kYUVALumaPlaneIndex, plane0Texture, hasAlpha);
        // Perform operations for the UV plane.
        PerformPlaneOperations(GetFormat(), kYUVAChromaPlaneIndex, plane1Texture, hasAlpha);
        if (hasAlpha) {
            // Perform operations for the A plane.
            PerformPlaneOperations(GetFormat(), kYUVAAlphaPlaneIndex, plane2Texture, hasAlpha);
        }

        for (auto& destVideoTexture : destVideoTextures) {
            mBackend->DestroyVideoTextureForTest(std::move(destVideoTexture));
        }
    }

    // Tests for rendering to a chroma texture view from a luma texture view, both of which created
    // from a multiplanar video texture. The test then copies back from chroma view to plane 0
    // texture view and compares the result same as expected texture data from first plane.
    template <typename T>
    void RenderFromLumaToChromaPlane() {
        // Create plane texture initialized with data.
        auto CreatePlaneTexWithData = [this](wgpu::TextureFormat format, int planeIndex,
                                             bool hasAlpha) -> wgpu::Texture {
            auto subsampleFactor = utils::GetMultiPlaneTextureSubsamplingFactor(format, planeIndex);
            wgpu::Extent3D size = {kYUVAImageDataWidthInTexels / subsampleFactor.horizontalFactor,
                                   kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor,
                                   1};

            // Create source texture with plane format
            wgpu::TextureDescriptor planeTextureDesc;
            planeTextureDesc.size = size;
            planeTextureDesc.format = GetPlaneFormat(planeIndex);
            planeTextureDesc.usage = wgpu::TextureUsage::CopyDst |
                                     wgpu::TextureUsage::TextureBinding |
                                     wgpu::TextureUsage::RenderAttachment;
            wgpu::Texture planeTexture = device.CreateTexture(&planeTextureDesc);

            // Copy plane (Y/UV/A) data to the plane source texture.
            size_t bytesPerRow = kYUVAImageDataWidthInTexels / subsampleFactor.horizontalFactor *
                                 utils::GetMultiPlaneTextureBytesPerElement(format, planeIndex);
            std::vector<T> planeSrcData = VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<T>(
                format, planeIndex, bytesPerRow,
                kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor, false, hasAlpha);
            wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
                utils::CreateTexelCopyTextureInfo(planeTexture);
            wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
                utils::CreateTexelCopyBufferLayout(0, bytesPerRow);
            wgpu::Queue queue = device.GetQueue();
            queue.WriteTexture(&texelCopyTextureInfo, planeSrcData.data(),
                               planeSrcData.size() * sizeof(T), &texelCopyBufferLayout, &size);

            return planeTexture;
        };

        const bool hasAlpha = utils::GetMultiPlaneTextureNumPlanes(GetFormat()) > 2;
        // Create source texture with plane 0 format i.e. R8/R16Unorm.
        wgpu::Texture plane0Texture =
            CreatePlaneTexWithData(GetFormat(), kYUVALumaPlaneIndex, hasAlpha);
        ASSERT_NE(plane0Texture.Get(), nullptr);

        // Create a video texture to be rendered into with multiplanar format.
        auto destVideoTexture = mBackend->CreateVideoTextureForTest(
            GetFormat(), wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
            /*isCheckerboard*/ false,
            /*initialized*/ true);
        ASSERT_NE(destVideoTexture.get(), nullptr);
        if (!destVideoTexture->CanWrapAsWGPUTexture()) {
            mBackend->DestroyVideoTextureForTest(std::move(destVideoTexture));
            GTEST_SKIP() << "Skipped because not supported.";
        }
        auto destVideoWGPUTexture = destVideoTexture->wgpuTexture;

        // Create luma plane texture view from multiplanar video texture.
        wgpu::TextureViewDescriptor lumaViewDesc;
        lumaViewDesc.format = GetPlaneFormat(kYUVALumaPlaneIndex);
        lumaViewDesc.aspect = GetPlaneAspect(kYUVALumaPlaneIndex);
        wgpu::TextureView lumaTextureView = destVideoWGPUTexture.CreateView(&lumaViewDesc);

        // Create chroma plane texture view from multiplanar video texture.
        wgpu::TextureViewDescriptor chromaViewDesc;
        chromaViewDesc.format = GetPlaneFormat(kYUVAChromaPlaneIndex);
        chromaViewDesc.aspect = GetPlaneAspect(kYUVAChromaPlaneIndex);
        wgpu::TextureView chromaTextureView = destVideoWGPUTexture.CreateView(&chromaViewDesc);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::Sampler sampler = device.CreateSampler();

        auto CreateRenderPipeline = [this](int planeIndex, wgpu::TextureView srcTextureView,
                                           wgpu::TextureView destTextureView,
                                           wgpu::CommandEncoder encoder,
                                           wgpu::Sampler sampler) -> wgpu::RenderPipeline {
            utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
            renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();
            renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
                @group(0) @binding(0) var sampler0 : sampler;
                @group(0) @binding(1) var texture : texture_2d<f32>;

                @fragment
                fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
                    return textureSample(texture, sampler0, texCoord);
                })");
            renderPipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
            renderPipelineDescriptor.cTargets[0].format = GetPlaneFormat(planeIndex);
            wgpu::RenderPipeline renderPipeline =
                device.CreateRenderPipeline(&renderPipelineDescriptor);

            utils::ComboRenderPassDescriptor renderPass({destTextureView});
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.SetPipeline(renderPipeline);
            pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                      {{0, sampler}, {1, srcTextureView}}));
            pass.Draw(6);
            pass.End();

            return renderPipeline;
        };

        // Render pass operations for reading plane0Texture with data into lumaTextureView created
        // from the multiplanar video texture.
        wgpu::RenderPipeline renderPipeline1 = CreateRenderPipeline(
            kYUVALumaPlaneIndex, plane0Texture.CreateView(), lumaTextureView, encoder, sampler);

        // Render pass operations for reading lumaTextureView into chromaTextureView created from
        // the multiplanar video texture.
        wgpu::RenderPipeline renderPipeline2 = CreateRenderPipeline(
            kYUVAChromaPlaneIndex, lumaTextureView, chromaTextureView, encoder, sampler);

        // Another render pass for reading the chromaTextureView into a texture of the luma plane's
        // format (i.e. R8/R16Unorm).
        utils::BasicRenderPass basicRenderPass = utils::CreateBasicRenderPass(
            device, kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels,
            GetPlaneFormat(kYUVALumaPlaneIndex));
        wgpu::RenderPassEncoder secondPass =
            encoder.BeginRenderPass(&basicRenderPass.renderPassInfo);
        secondPass.SetPipeline(renderPipeline1);
        secondPass.SetBindGroup(0,
                                utils::MakeBindGroup(device, renderPipeline1.GetBindGroupLayout(0),
                                                     {{0, sampler}, {1, chromaTextureView}}));
        secondPass.Draw(6);
        secondPass.End();

        // Submit all commands for the encoder.
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Compare expected data from luma values to that from basicRenderPass.
        std::vector<T> expectedData = VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<T>(
            GetFormat(), kYUVALumaPlaneIndex, kYUVAImageDataWidthInTexels * sizeof(T),
            kYUVAImageDataHeightInTexels, false, hasAlpha);
        EXPECT_TEXTURE_EQ(expectedData.data(), basicRenderPass.color, {0, 0},
                          {kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels},
                          GetPlaneFormat(kYUVALumaPlaneIndex));

        mBackend->DestroyVideoTextureForTest(std::move(destVideoTexture));
    }
};

// Tests creating a texture with a multi-plane format.
TEST_P(VideoViewsRenderTargetTests, RenderAttachmentValid) {
    // multi-planar formats should be allowed to be renderable with
    // Feature::MultiPlanarRenderTargets.
    auto platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::RenderAttachment,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests validating attachment sizes with a multi-plane format.
TEST_P(VideoViewsRenderTargetTests, RenderAttachmentSizeValidation) {
    auto platformTexture =
        mBackend->CreateVideoTextureForTest(GetFormat(), wgpu::TextureUsage::RenderAttachment,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);

    ASSERT_NE(platformTexture.get(), nullptr);
    if (!platformTexture->CanWrapAsWGPUTexture()) {
        mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
        GTEST_SKIP() << "Skipped because not supported.";
    }

    // Create luma texture view from the video texture.
    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = platformTexture->wgpuTexture.CreateView(&lumaViewDesc);

    // Create chroma texture view from the video texture.
    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = platformTexture->wgpuTexture.CreateView(&chromaViewDesc);

    {
        // Create an RGBA texture with same size as chroma texture view.
        auto subsampleFactor =
            utils::GetMultiPlaneTextureSubsamplingFactor(GetFormat(), kYUVAChromaPlaneIndex);
        wgpu::TextureDescriptor desc;
        desc.format = wgpu::TextureFormat::RGBA8Unorm;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.usage = wgpu::TextureUsage::RenderAttachment;
        desc.size = {kYUVAImageDataWidthInTexels / subsampleFactor.horizontalFactor,
                     kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor, 1};
        wgpu::Texture rgbaTexture = device.CreateTexture(&desc);

        // Render pass operations passing color attachments of same size (control case).
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass({chromaTextureView, rgbaTexture.CreateView()});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
        encoder.Finish();
    }

    {
        // Render pass operations passing color attachments of different sizes (error case).
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass({chromaTextureView, lumaTextureView});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        // Create an RGBA texture with same size as luma texture view.
        wgpu::TextureDescriptor desc;
        desc.format = wgpu::TextureFormat::R8Unorm;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.usage = wgpu::TextureUsage::RenderAttachment;
        desc.size = {kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels, 1};
        desc.sampleCount = 4;
        wgpu::Texture rgbaTexture = device.CreateTexture(&desc);

        // Render pass operations passing the luma texture view as resolve target
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass({rgbaTexture.CreateView()});
        renderPass.cColorAttachments[0].resolveTarget = lumaTextureView;
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        // TODO(chromium:324422644): support using multi-planar texture as resolve target.
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        // Create an RGBA texture with same size as chroma texture view.
        auto subsampleFactor =
            utils::GetMultiPlaneTextureSubsamplingFactor(GetFormat(), kYUVAChromaPlaneIndex);
        wgpu::TextureDescriptor desc;
        desc.format = wgpu::TextureFormat::R8Unorm;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.usage = wgpu::TextureUsage::RenderAttachment;
        desc.size = {kYUVAImageDataWidthInTexels / subsampleFactor.horizontalFactor,
                     kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor, 1};
        desc.sampleCount = 4;
        wgpu::Texture rgbaTexture = device.CreateTexture(&desc);

        // Render pass operations passing the chroma texture view as resolve target
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        utils::ComboRenderPassDescriptor renderPass({rgbaTexture.CreateView()});
        renderPass.cColorAttachments[0].resolveTarget = chromaTextureView;
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.End();

        // TODO(chromium:324422644): support using multi-planar texture as resolve target.
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        // Create a depth texture with same size as luma texture view. It should be possible to use
        // it with both the luminance and the subsampled chroma plane
        wgpu::TextureDescriptor desc;
        desc.format = wgpu::TextureFormat::Depth24PlusStencil8;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.usage = wgpu::TextureUsage::RenderAttachment;
        desc.size = {kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels, 1};
        wgpu::Texture depthTexture = device.CreateTexture(&desc);
        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            utils::ComboRenderPassDescriptor renderPass({lumaTextureView},
                                                        depthTexture.CreateView());
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.End();
            encoder.Finish();
        }

        {
            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            utils::ComboRenderPassDescriptor renderPass({chromaTextureView},
                                                        depthTexture.CreateView());
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
            pass.End();
            encoder.Finish();
        }
    }

    // TODO(chromium:324422644): enable below test for d3d11.
    // {
    //     // Create a depth texture with same size as chroma texture view. It should produce an
    //     error
    //     // to use it with the chroma plane as a luminance-size texture is expected.
    //     wgpu::TextureDescriptor desc;
    //     desc.format = wgpu::TextureFormat::Depth24PlusStencil8;
    //     desc.dimension = wgpu::TextureDimension::e2D;
    //     desc.usage = wgpu::TextureUsage::RenderAttachment;
    //     desc.size = {kYUVAImageDataWidthInTexels / 2, kYUVAImageDataHeightInTexels / 2, 1};
    //     wgpu::Texture depthTexture = device.CreateTexture(&desc);
    //     {
    //         wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    //         utils::ComboRenderPassDescriptor renderPass({lumaTextureView},
    //                                                     depthTexture.CreateView());
    //         wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    //         pass.End();
    //         ASSERT_DEVICE_ERROR(encoder.Finish());
    //     }

    //     {
    //         wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    //         utils::ComboRenderPassDescriptor renderPass({chromaTextureView},
    //                                                     depthTexture.CreateView());
    //         wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    //         pass.End();
    //         ASSERT_DEVICE_ERROR(encoder.Finish());
    //     }
    // }

    mBackend->DestroyVideoTextureForTest(std::move(platformTexture));
}

// Tests for rendering to a multiplanar video texture through its views.
TEST_P(VideoViewsRenderTargetTests, RenderToMultiplanarVideoTexture) {
    if (utils::GetMultiPlaneTextureBitDepth(GetFormat()) == 8) {
        RenderToMultiplanarVideoTextures<uint8_t>(/*numOfTextures=*/1, /*hasDepth=*/false);
    } else if (utils::GetMultiPlaneTextureBitDepth(GetFormat()) == 16) {
        RenderToMultiplanarVideoTextures<uint16_t>(/*numOfTextures=*/1, /*hasDepth=*/false);
    } else {
        DAWN_UNREACHABLE();
    }
}

TEST_P(VideoViewsRenderTargetTests, RenderToMultiplanarVideoTextureWithDepth) {
    if (utils::GetMultiPlaneTextureBitDepth(GetFormat()) == 8) {
        RenderToMultiplanarVideoTextures<uint8_t>(/*numOfTextures=*/1, /*hasDepth=*/true);
    } else if (utils::GetMultiPlaneTextureBitDepth(GetFormat()) == 16) {
        RenderToMultiplanarVideoTextures<uint16_t>(/*numOfTextures=*/1, /*hasDepth=*/true);
    } else {
        DAWN_UNREACHABLE();
    }
}

TEST_P(VideoViewsRenderTargetTests, RenderToThreeMultiplanarVideoTexture) {
    if (utils::GetMultiPlaneTextureBitDepth(GetFormat()) == 8) {
        RenderToMultiplanarVideoTextures<uint8_t>(/*numOfTextures=*/3, /*hasDepth=*/false);
    } else if (utils::GetMultiPlaneTextureBitDepth(GetFormat()) == 16) {
        RenderToMultiplanarVideoTextures<uint16_t>(/*numOfTextures=*/3, /*hasDepth=*/false);
    } else {
        DAWN_UNREACHABLE();
    }
}

TEST_P(VideoViewsRenderTargetTests, RenderToThreeMultiplanarVideoTextureWithDepth) {
    if (utils::GetMultiPlaneTextureBitDepth(GetFormat()) == 8) {
        RenderToMultiplanarVideoTextures<uint8_t>(/*numOfTextures=*/3, /*hasDepth=*/true);
    } else if (utils::GetMultiPlaneTextureBitDepth(GetFormat()) == 16) {
        RenderToMultiplanarVideoTextures<uint16_t>(/*numOfTextures=*/3, /*hasDepth=*/true);
    } else {
        DAWN_UNREACHABLE();
    }
}

// Tests for rendering to one plane while reading from another plane.
TEST_P(VideoViewsRenderTargetTests, RenderFromLumaToChromaPlane) {
    if (utils::GetMultiPlaneTextureBitDepth(GetFormat()) == 8) {
        RenderFromLumaToChromaPlane<uint8_t>();
    } else if (utils::GetMultiPlaneTextureBitDepth(GetFormat()) == 16) {
        RenderFromLumaToChromaPlane<uint16_t>();
    } else {
        DAWN_UNREACHABLE();
    }
}

class VideoViewsExtendedUsagesTests : public VideoViewsTestsBase {
  protected:
    void SetUp() override {
        VideoViewsTestsBase::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(!IsMultiPlanarFormatsSupported());
        DAWN_TEST_UNSUPPORTED_IF(!IsFormatSupported());

        DAWN_TEST_UNSUPPORTED_IF(
            !device.HasFeature(wgpu::FeatureName::MultiPlanarFormatExtendedUsages));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures =
            VideoViewsTestsBase::GetRequiredFeatures();
        if (SupportsFeatures({wgpu::FeatureName::MultiPlanarFormatExtendedUsages})) {
            requiredFeatures.push_back(wgpu::FeatureName::MultiPlanarFormatExtendedUsages);
        }
        return requiredFeatures;
    }

    wgpu::Texture CreateMultiPlanarTexture(wgpu::TextureFormat format,
                                           wgpu::TextureUsage usage,
                                           bool isCheckerboard = false,
                                           bool initialized = true) {
        switch (utils::GetMultiPlaneTextureBitDepth(format)) {
            case 8:
                return CreateMultiPlanarTextureImpl<uint8_t>(format, usage, isCheckerboard,
                                                             initialized);
            case 16:
                return CreateMultiPlanarTextureImpl<uint16_t>(format, usage, isCheckerboard,
                                                              initialized);
            default:
                DAWN_UNREACHABLE();
                return nullptr;
        }
    }

    template <typename ComponentType>
    wgpu::Texture CreateMultiPlanarTextureImpl(wgpu::TextureFormat format,
                                               wgpu::TextureUsage usage,
                                               bool isCheckerboard,
                                               bool initialized) {
        wgpu::TextureDescriptor desc;
        desc.format = format;
        desc.size = {VideoViewsTestsBase::kYUVAImageDataWidthInTexels,
                     VideoViewsTestsBase::kYUVAImageDataHeightInTexels, 1};
        desc.usage = usage;

        wgpu::DawnTextureInternalUsageDescriptor internalDesc;
        internalDesc.internalUsage = wgpu::TextureUsage::CopyDst;
        desc.nextInChain = &internalDesc;

        auto texture = device.CreateTexture(&desc);
        if (texture == nullptr) {
            return nullptr;
        }

        if (initialized) {
            size_t numPlanes = utils::GetMultiPlaneTextureNumPlanes(format);
            const bool hasAlpha = numPlanes > 2;

            wgpu::DawnEncoderInternalUsageDescriptor encoderInternalDesc;
            encoderInternalDesc.useInternalUsages = true;
            wgpu::CommandEncoderDescriptor encoderDesc;
            encoderDesc.nextInChain = &encoderInternalDesc;

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);

            for (size_t plane = 0; plane < numPlanes; ++plane) {
                auto subsampleFactor = utils::GetMultiPlaneTextureSubsamplingFactor(format, plane);

                size_t bytesPerRow = VideoViewsTestsBase::kYUVAImageDataWidthInTexels /
                                     subsampleFactor.horizontalFactor *
                                     utils::GetMultiPlaneTextureBytesPerElement(format, plane);

                bytesPerRow = Align(bytesPerRow, 256);

                wgpu::TexelCopyTextureInfo copyDst =
                    utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
                copyDst.aspect = VideoViewsTestsBase::GetPlaneAspect(plane);

                wgpu::Extent3D copySize{VideoViewsTestsBase::kYUVAImageDataWidthInTexels /
                                            subsampleFactor.horizontalFactor,
                                        VideoViewsTestsBase::kYUVAImageDataHeightInTexels /
                                            subsampleFactor.verticalFactor,
                                        1};

                // Staging buffer.
                wgpu::BufferDescriptor bufferDesc;
                bufferDesc.size = bytesPerRow * copySize.height;
                bufferDesc.mappedAtCreation = true;
                bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;

                auto buffer = device.CreateBuffer(&bufferDesc);

                std::vector<ComponentType> data = GetTestTextureDataWithPlaneIndex<ComponentType>(
                    format, plane, bytesPerRow,
                    VideoViewsTestsBase::kYUVAImageDataHeightInTexels /
                        subsampleFactor.verticalFactor,
                    isCheckerboard, hasAlpha);

                memcpy(buffer.GetMappedRange(), data.data(), bufferDesc.size);
                buffer.Unmap();

                wgpu::TexelCopyBufferInfo copySrc =
                    utils::CreateTexelCopyBufferInfo(buffer, 0, bytesPerRow);

                encoder.CopyBufferToTexture(&copySrc, &copyDst, &copySize);
            }  // for plane

            auto cmdBuffer = encoder.Finish();
            device.GetQueue().Submit(1, &cmdBuffer);
        }  // initialized

        return texture;
    }

    template <typename ComponentType>
    void RunT2BCopyPlaneAspectsTest();
};

// Test that creating multi-planar texture should success if device is created with
// MultiPlanarFormatExtendedUsages feature enabled.
TEST_P(VideoViewsExtendedUsagesTests, CreateTextureSucceeds) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.mipLevelCount = 1;
    descriptor.format = GetFormat();
    descriptor.usage = wgpu::TextureUsage::TextureBinding;

    auto texture = device.CreateTexture(&descriptor);
    EXPECT_NE(texture, nullptr);
}

// Test that creating multi-planar texture should fail if the specified descriptor is mipmapped.
TEST_P(VideoViewsExtendedUsagesTests, CreateTextureFailsIfMipmapped) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.mipLevelCount = 2;
    descriptor.format = GetFormat();
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    ASSERT_DEVICE_ERROR_MSG(device.CreateTexture(&descriptor),
                            testing::HasSubstr("must be non-mipmapped & 2D"));
}

// Test that creating multi-planar texture should fail if the specified descriptor is not 2D.
TEST_P(VideoViewsExtendedUsagesTests, CreateTextureFailsIfNot2D) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    // Texture must not be 3D.
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e3D;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.format = GetFormat();
    descriptor.usage = wgpu::TextureUsage::TextureBinding;
    ASSERT_DEVICE_ERROR_MSG(device.CreateTexture(&descriptor),
                            testing::HasSubstr("must be non-mipmapped & 2D"));

    // Texture must not be array.
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.depthOrArrayLayers = 2;
    ASSERT_DEVICE_ERROR_MSG(device.CreateTexture(&descriptor),
                            testing::HasSubstr("must be non-mipmapped & 2D"));
}

// Test that creating multiplanar texture with view formats should fail.
TEST_P(VideoViewsExtendedUsagesTests, CreateTextureFailsWithViewFormats) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = 4;
    descriptor.size.height = 4;
    descriptor.mipLevelCount = 1;
    descriptor.format = GetFormat();
    descriptor.usage = wgpu::TextureUsage::TextureBinding;

    {
        std::vector<wgpu::TextureFormat> viewFormats = {
            wgpu::TextureFormat::RGBA8Unorm,
        };
        descriptor.viewFormatCount = viewFormats.size();
        descriptor.viewFormats = viewFormats.data();
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
    {
        std::vector<wgpu::TextureFormat> viewFormats = {
            GetPlaneFormat(0),
            GetPlaneFormat(1),
        };
        descriptor.viewFormatCount = viewFormats.size();
        descriptor.viewFormats = viewFormats.data();
        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}

// Tests sampling a YUV multi-planar texture.
TEST_P(VideoViewsExtendedUsagesTests, SamplingMultiPlanarYUVTexture) {
    // TODO(crbug.com/dawn/1998): Failure on Intel's Vulkan device.
    DAWN_SUPPRESS_TEST_IF(IsWindows() && IsVulkan() && IsIntel());

    auto texture = CreateMultiPlanarTexture(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    EXPECT_NE(texture, nullptr);

    const bool hasAlpha = utils::GetMultiPlaneTextureNumPlanes(GetFormat()) > 2;
    if (hasAlpha) {
        GTEST_SKIP() << "Skipped because format is not YUV.";
    }

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = texture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = texture.CreateView(&chromaViewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var lumaTexture : texture_2d<f32>;
            @group(0) @binding(2) var chromaTexture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(lumaTexture, sampler0, texCoord).r;
               let u : f32 = textureSample(chromaTexture, sampler0, texCoord).r;
               let v : f32 = textureSample(chromaTexture, sampler0, texCoord).g;
               return vec4f(y, u, v, 1.0);
            })");

    utils::BasicRenderPass renderPass =
        utils::CreateBasicRenderPass(device, kYUVAImageDataWidthInTexels,
                                     kYUVAImageDataHeightInTexels, wgpu::TextureFormat::RGBA8Unorm);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(
            0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                    {{0, sampler}, {1, lumaTextureView}, {2, chromaTextureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint8_t> expectedData = GetTestTextureData<uint8_t>(
        wgpu::TextureFormat::RGBA8Unorm, /*isCheckerboard=*/true, hasAlpha);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 3) {
        expectedRGBA.push_back({expectedData[i], expectedData[i + 1], expectedData[i + 2], 0xFF});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
}

// Tests sampling a YUVA multi-planar texture.
TEST_P(VideoViewsExtendedUsagesTests, SamplingMultiPlanarYUVATexture) {
    auto texture = CreateMultiPlanarTexture(GetFormat(), wgpu::TextureUsage::TextureBinding,
                                            /*isCheckerboard*/ true,
                                            /*initialized*/ true);
    EXPECT_NE(texture, nullptr);

    const bool hasAlpha = utils::GetMultiPlaneTextureNumPlanes(GetFormat()) > 2;
    if (!hasAlpha) {
        GTEST_SKIP() << "Skipped because format is not YUVA.";
    }

    wgpu::TextureViewDescriptor lumaViewDesc;
    lumaViewDesc.format = GetPlaneFormat(0);
    lumaViewDesc.aspect = wgpu::TextureAspect::Plane0Only;
    wgpu::TextureView lumaTextureView = texture.CreateView(&lumaViewDesc);

    wgpu::TextureViewDescriptor chromaViewDesc;
    chromaViewDesc.format = GetPlaneFormat(1);
    chromaViewDesc.aspect = wgpu::TextureAspect::Plane1Only;
    wgpu::TextureView chromaTextureView = texture.CreateView(&chromaViewDesc);

    wgpu::TextureViewDescriptor alphaViewDesc;
    alphaViewDesc.format = GetPlaneFormat(2);
    alphaViewDesc.aspect = wgpu::TextureAspect::Plane2Only;
    wgpu::TextureView alphaTextureView = texture.CreateView(&alphaViewDesc);

    utils::ComboRenderPipelineDescriptor renderPipelineDescriptor;
    renderPipelineDescriptor.vertex.module = GetTestVertexShaderModule();

    renderPipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var sampler0 : sampler;
            @group(0) @binding(1) var lumaTexture : texture_2d<f32>;
            @group(0) @binding(2) var chromaTexture : texture_2d<f32>;
            @group(0) @binding(3) var alphaTexture : texture_2d<f32>;

            @fragment
            fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
               let y : f32 = textureSample(lumaTexture, sampler0, texCoord).r;
               let u : f32 = textureSample(chromaTexture, sampler0, texCoord).r;
               let v : f32 = textureSample(chromaTexture, sampler0, texCoord).g;
               let a : f32 = textureSample(alphaTexture, sampler0, texCoord).r;
               return vec4f(y, u, v, a);
            })");

    utils::BasicRenderPass renderPass =
        utils::CreateBasicRenderPass(device, kYUVAImageDataWidthInTexels,
                                     kYUVAImageDataHeightInTexels, wgpu::TextureFormat::RGBA8Unorm);
    renderPipelineDescriptor.cTargets[0].format = renderPass.colorFormat;

    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&renderPipelineDescriptor);

    wgpu::Sampler sampler = device.CreateSampler();

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
        pass.SetPipeline(renderPipeline);
        pass.SetBindGroup(0, utils::MakeBindGroup(device, renderPipeline.GetBindGroupLayout(0),
                                                  {{0, sampler},
                                                   {1, lumaTextureView},
                                                   {2, chromaTextureView},
                                                   {3, alphaTextureView}}));
        pass.Draw(6);
        pass.End();
    }

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    std::vector<uint8_t> expectedData = GetTestTextureData<uint8_t>(
        wgpu::TextureFormat::RGBA8Unorm, /*isCheckerboard=*/true, hasAlpha);
    std::vector<utils::RGBA8> expectedRGBA;
    for (uint8_t i = 0; i < expectedData.size(); i += 4) {
        expectedRGBA.push_back(
            {expectedData[i], expectedData[i + 1], expectedData[i + 2], expectedData[i + 3]});
    }

    EXPECT_TEXTURE_EQ(expectedRGBA.data(), renderPass.color, {0, 0},
                      {kYUVAImageDataWidthInTexels, kYUVAImageDataHeightInTexels}, 0,
                      wgpu::TextureAspect::All, 0, kTolerance);
}

// Test copying from multi-planar format per plane to a buffer succeeds.
TEST_P(VideoViewsExtendedUsagesTests, T2BCopyPlaneAspectsSucceeds) {
    switch (utils::GetMultiPlaneTextureBitDepth(GetFormat())) {
        case 8:
            RunT2BCopyPlaneAspectsTest<uint8_t>();
            break;
        case 16:
            RunT2BCopyPlaneAspectsTest<uint16_t>();
            break;
        default:
            DAWN_UNREACHABLE();
    }
}

template <typename ComponentType>
void VideoViewsExtendedUsagesTests::RunT2BCopyPlaneAspectsTest() {
    auto srcTexture = CreateMultiPlanarTexture(
        GetFormat(), wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
        /*isCheckerboard*/ false,
        /*initialized*/ true);
    EXPECT_NE(srcTexture, nullptr);

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 256;
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::TexelCopyBufferInfo copyDst = utils::CreateTexelCopyBufferInfo(dstBuffer, 0, 256);

    wgpu::Extent3D copySize = {1, 1, 1};

    const bool hasAlpha = utils::GetMultiPlaneTextureNumPlanes(GetFormat()) > 2;

    // Plane0
    wgpu::TexelCopyTextureInfo copySrc = utils::CreateTexelCopyTextureInfo(
        srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    {
        auto subsampleFactor =
            utils::GetMultiPlaneTextureSubsamplingFactor(GetFormat(), kYUVALumaPlaneIndex);
        std::vector<ComponentType> expectedData =
            VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<ComponentType>(
                GetFormat(), kYUVALumaPlaneIndex,
                kYUVAImageDataWidthInTexels / subsampleFactor.horizontalFactor *
                    utils::GetMultiPlaneTextureBytesPerElement(GetFormat(), kYUVALumaPlaneIndex),
                kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor, false, hasAlpha);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);

        auto cmdBuffer = encoder.Finish();
        device.GetQueue().Submit(1, &cmdBuffer);

        // Convert 1st pixel's luma component to array of 8 bits bytes.
        uint8_t expectedYDataAsU8[sizeof(ComponentType)];
        memcpy(expectedYDataAsU8, &expectedData[0], sizeof(expectedYDataAsU8));

        EXPECT_BUFFER_U8_RANGE_EQ(expectedYDataAsU8, dstBuffer, 0, sizeof(expectedYDataAsU8));
    }

    // Plane1
    copySrc = utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0},
                                                wgpu::TextureAspect::Plane1Only);
    {
        auto subsampleFactor =
            utils::GetMultiPlaneTextureSubsamplingFactor(GetFormat(), kYUVAChromaPlaneIndex);
        std::vector<ComponentType> expectedData =
            VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<ComponentType>(
                GetFormat(), kYUVAChromaPlaneIndex,
                kYUVAImageDataWidthInTexels / subsampleFactor.horizontalFactor *
                    utils::GetMultiPlaneTextureBytesPerElement(GetFormat(), kYUVAChromaPlaneIndex),
                kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor, false, hasAlpha);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);

        auto cmdBuffer = encoder.Finish();
        device.GetQueue().Submit(1, &cmdBuffer);

        // Convert 1st pixel's chroma component to array of 8 bits bytes.
        uint8_t expectedUVDataAsU8[sizeof(ComponentType) * 2];
        memcpy(expectedUVDataAsU8, expectedData.data(), sizeof(expectedUVDataAsU8));

        EXPECT_BUFFER_U8_RANGE_EQ(expectedUVDataAsU8, dstBuffer, 0, sizeof(expectedUVDataAsU8));
    }

    if (hasAlpha) {
        // Plane2
        copySrc = utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0},
                                                    wgpu::TextureAspect::Plane2Only);
        {
            auto subsampleFactor =
                utils::GetMultiPlaneTextureSubsamplingFactor(GetFormat(), kYUVAAlphaPlaneIndex);
            std::vector<ComponentType> expectedData =
                VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<ComponentType>(
                    GetFormat(), kYUVAAlphaPlaneIndex,
                    kYUVAImageDataWidthInTexels / subsampleFactor.horizontalFactor *
                        utils::GetMultiPlaneTextureBytesPerElement(GetFormat(),
                                                                   kYUVAAlphaPlaneIndex),
                    kYUVAImageDataHeightInTexels / subsampleFactor.verticalFactor, false, hasAlpha);

            wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
            encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);

            auto cmdBuffer = encoder.Finish();
            device.GetQueue().Submit(1, &cmdBuffer);

            // Convert 1st pixel's alpha component to array of 8 bits bytes.
            uint8_t expectedADataAsU8[sizeof(ComponentType)];
            memcpy(expectedADataAsU8, expectedData.data(), sizeof(expectedADataAsU8));

            EXPECT_BUFFER_U8_RANGE_EQ(expectedADataAsU8, dstBuffer, 0, sizeof(expectedADataAsU8));
        }
    }
}

// Test copying from a multi-planar format to a buffer fails if texture aspect is not single plane.
TEST_P(VideoViewsExtendedUsagesTests, T2BCopyAllAspectsFails) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    auto srcTexture =
        CreateMultiPlanarTexture(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                 wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
                                 /*isCheckerboard*/ false,
                                 /*initialized*/ true);

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 256;
    bufferDescriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer dstBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::TexelCopyTextureInfo copySrc =
        utils::CreateTexelCopyTextureInfo(srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::All);

    wgpu::TexelCopyBufferInfo copyDst = utils::CreateTexelCopyBufferInfo(dstBuffer, 0, 256);

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR_MSG(encoder.Finish(), testing::HasSubstr("More than a single aspect"));
}

// Test copying from one multi-planar formatted texture into another fails even if
// MultiPlanarFormatExtendedUsages is enabled.
TEST_P(VideoViewsExtendedUsagesTests, T2TCopyPlaneAspectFails) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    auto srcTexture =
        CreateMultiPlanarTexture(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                 wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
                                 /*isCheckerboard*/ false,
                                 /*initialized*/ true);
    auto dstTexture =
        CreateMultiPlanarTexture(wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                                 wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst,
                                 /*isCheckerboard*/ false,
                                 /*initialized*/ true);

    wgpu::TexelCopyTextureInfo copySrc = utils::CreateTexelCopyTextureInfo(
        srcTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::TexelCopyTextureInfo copyDst = utils::CreateTexelCopyTextureInfo(
        dstTexture, 0, {0, 0, 0}, wgpu::TextureAspect::Plane0Only);

    wgpu::Extent3D copySize = {1, 1, 1};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyTextureToTexture(&copySrc, &copyDst, &copySize);
    ASSERT_DEVICE_ERROR_MSG(encoder.Finish(), testing::HasSubstr("not allowed"));
}

DAWN_INSTANTIATE_TEST_B(VideoViewsTests,
                        VideoViewsTestBackend::Backends(),
                        VideoViewsTestBackend::Formats());
DAWN_INSTANTIATE_TEST_B(VideoViewsValidationTests,
                        VideoViewsTestBackend::Backends(),
                        VideoViewsTestBackend::Formats());
DAWN_INSTANTIATE_TEST_B(VideoViewsRenderTargetTests,
                        VideoViewsTestBackend::Backends(),
                        VideoViewsTestBackend::Formats());

DAWN_INSTANTIATE_TEST_B(VideoViewsExtendedUsagesTests,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), VulkanBackend()},
                        {wgpu::TextureFormat::R8BG8Biplanar420Unorm,
                         wgpu::TextureFormat::R8BG8Biplanar422Unorm,
                         wgpu::TextureFormat::R8BG8Biplanar444Unorm,
                         wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm,
                         wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm,
                         wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm,
                         wgpu::TextureFormat::R8BG8A8Triplanar420Unorm});

}  // anonymous namespace
}  // namespace dawn
