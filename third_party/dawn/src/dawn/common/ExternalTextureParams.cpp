// Copyright 2026 The Dawn & Tint Authors
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

#include "src/dawn/common/ExternalTextureParams.h"

#include "src/dawn/common/ColorSpace.h"

namespace dawn {

// HLG is mode 1 which is tagged with G = -1
// TODO(https://crbug.com/496604550): G = -1 as a mode selecator is a hack and instead a proper mode
// enum should be added to the API.
//
//   if (v <= D) {
//       return v * v / E;
//   }
//   return (B + exp((v - C) / A)) / F
constexpr TransferFunction kEOTF_HLG = {
    .g = -1.0f,  // Mode HLG see src/dawn/native/ExternalTexture.cpp
    .a = 0.17883277f,
    .b = 0.28466892f,
    .c = 0.55991073f,
    .d = 0.5f,
    .e = 3.0f,
    .f = 12.0f,
};

// PQ is mode 2 which is tagged with G = -2
// TODO(https://crbug.com/496604550): G = -2 as a mode selecator is a hack and instead a proper mode
// enum should be added to the API.
//
//   v = pow(clamp(v, 0, 1), 1.0 / B)
//   v = max(v - C, 0) / (D - E * v)
//   return pow(v, 1.0 / A)
constexpr TransferFunction kEOTF_PQ = {
    .g = -2.0f,  // Mode PQ see src/dawn/native/ExternalTexture.cpp
    .a = (2610.0f / 16384.0f),
    .b = (2523.0f / 4096.0f) * 128.0f,
    .c = (3424.0f / 4096.0f),
    .d = (2413.0f / 4096.0f) * 32.0f,
    .e = (2392.0f / 4096.0f) * 32.0f,
    .f = 0.0f,
};

wgpu::Status ComputeExternalTextureParams(const wgpu::ColorSpaceDawn& srcColorSpace,
                                          wgpu::PredefinedColorSpace dstColorSpace,
                                          ExternalTextureColorSpaceParams* out) {
    if (srcColorSpace.nextInChain != nullptr) {
        return wgpu::Status::Error;
    }

    // Specifying the hdrReferenceWhiteLuminance is only allowed for HDR color spaces.
    if (srcColorSpace.hdrReferenceWhiteLuminance != 0 &&
        srcColorSpace.transfer != wgpu::ColorSpaceTransferDawn::PQ &&
        srcColorSpace.transfer != wgpu::ColorSpaceTransferDawn::HLG) {
        return wgpu::Status::Error;
    }

    // Convert each enum argument to the corresponding matrix / transfer function params.
    math::Mat4x3f rangeMatrix;
    switch (srcColorSpace.yCbCrRange) {
        case wgpu::ColorSpaceYCbCrRangeDawn::Identity:
            rangeMatrix = math::Mat4x3f::CropOrExpandFrom(math::Mat3x3f::Identity());
            break;
        case wgpu::ColorSpaceYCbCrRangeDawn::Narrow:
            rangeMatrix = kYCbCrRange_Narrow;
            break;
        case wgpu::ColorSpaceYCbCrRangeDawn::Full:
            rangeMatrix = kYCbCrRange_Full;
            break;
        default:
            return wgpu::Status::Error;
    }

    math::Mat3x3f ycbcrToRgb;
    switch (srcColorSpace.yCbCrMatrix) {
        case wgpu::ColorSpaceYCbCrMatrixDawn::Identity:
            ycbcrToRgb = math::Mat3x3f::Identity();
            break;
        case wgpu::ColorSpaceYCbCrMatrixDawn::Rec601:
            ycbcrToRgb = kYCbCrToRGB_Rec601;
            break;
        case wgpu::ColorSpaceYCbCrMatrixDawn::Rec709:
            ycbcrToRgb = kYCbCrToRGB_Rec709;
            break;
        case wgpu::ColorSpaceYCbCrMatrixDawn::Rec2020:
            ycbcrToRgb = kYCbCrToRGB_Rec2020;
            break;
        default:
            return wgpu::Status::Error;
    }

    math::Mat3x3f srcRGBToXYZ;
    switch (srcColorSpace.primaries) {
        case wgpu::ColorSpacePrimariesDawn::Rec601:
            srcRGBToXYZ = kRGBToXYZ_Rec601;
            break;
        case wgpu::ColorSpacePrimariesDawn::SRGB:
            srcRGBToXYZ = kRGBToXYZ_Rec709;
            break;
        case wgpu::ColorSpacePrimariesDawn::Rec2020:
            srcRGBToXYZ = kRGBToXYZ_Rec2020;
            break;
        case wgpu::ColorSpacePrimariesDawn::DisplayP3:
            srcRGBToXYZ = kRGBToXYZ_DisplayP3;
            break;
        default:
            return wgpu::Status::Error;
    }

    TransferFunction srcEOTF;
    float srcLuminanceOf1 = kDefaultHDRReferenceWhiteLuminance;
    switch (srcColorSpace.transfer) {
        case wgpu::ColorSpaceTransferDawn::Identity:
            srcEOTF = kEOTF_Identity;
            break;
        case wgpu::ColorSpaceTransferDawn::SRGB:
            srcEOTF = kEOTF_sRGB;
            break;
        case wgpu::ColorSpaceTransferDawn::DisplayP3:
            srcEOTF = kEOTF_DisplayP3;
            break;
        case wgpu::ColorSpaceTransferDawn::SMPTE_170M:
            srcEOTF = kEOTF_SMPTE_170M;
            break;
        case wgpu::ColorSpaceTransferDawn::HLG:
            srcLuminanceOf1 = kHLGLuminanceOf1;
            srcEOTF = kEOTF_HLG;
            break;
        case wgpu::ColorSpaceTransferDawn::PQ:
            srcLuminanceOf1 = kPQLuminanceOf1;
            srcEOTF = kEOTF_PQ;
            break;
        case wgpu::ColorSpaceTransferDawn::BT_1886:
            srcEOTF = kEOTF_BT_1886;
            break;
        default:
            return wgpu::Status::Error;
    }

    math::Mat3x3f dstXYZToRGB;
    TransferFunction dstOETF;
    switch (dstColorSpace) {
        case wgpu::PredefinedColorSpace::SRGB:
            dstXYZToRGB = kXYZToRGB_sRGB;
            dstOETF = kEOTFInverse_sRGB;
            break;
        case wgpu::PredefinedColorSpace::DisplayP3:
            dstXYZToRGB = kXYZToRGB_DisplayP3;
            dstOETF = kEOTFInverse_DisplayP3;
            break;
        case wgpu::PredefinedColorSpace::SRGBLinear:
            dstXYZToRGB = kXYZToRGB_sRGB;
            dstOETF = kEOTF_Identity;
            break;
        case wgpu::PredefinedColorSpace::DisplayP3Linear:
            dstXYZToRGB = kXYZToRGB_DisplayP3;
            dstOETF = kEOTF_Identity;
            break;
        case wgpu::PredefinedColorSpace::Rec2020Linear:
            dstXYZToRGB = kXYZToRGB_Rec2020;
            dstOETF = kEOTF_Identity;
            break;
        default:
            return wgpu::Status::Error;
    }

    // The YCbCr to RGB matrix is passed row-major.
    math::Mat4x3f yuvToRgbConversionMatrix = math::Mul(ycbcrToRgb, rangeMatrix);
    for (size_t x = 0; x < 4; x++) {
        for (size_t y = 0; y < 3; y++) {
            out->yuvToRgbConversionMatrix[x + 4 * y] = yuvToRgbConversionMatrix[x][y];
        }
    }

    // The gamut matrix is passed column-major.
    float hdrReferenceWhiteLuminance = kDefaultHDRReferenceWhiteLuminance;
    if (srcColorSpace.hdrReferenceWhiteLuminance != 0) {
        hdrReferenceWhiteLuminance = srcColorSpace.hdrReferenceWhiteLuminance;
    }

    float luminanceAdjustment = srcLuminanceOf1 / hdrReferenceWhiteLuminance;
    math::Mat3x3f gamutConversionMatrix = math::Mul(dstXYZToRGB, srcRGBToXYZ) * luminanceAdjustment;

    for (size_t x = 0; x < 3; x++) {
        for (size_t y = 0; y < 3; y++) {
            out->gamutConversionMatrix[y + 3 * x] = gamutConversionMatrix[x][y];
        }
    }

    auto TransferFunctionToArray = [](const TransferFunction& tf) -> std::array<float, 7> {
        return {tf.g, tf.a, tf.b, tf.c, tf.d, tf.e, tf.f};
    };
    out->srcTransferFunction = TransferFunctionToArray(srcEOTF);
    out->dstTransferFunction = TransferFunctionToArray(dstOETF);

    return wgpu::Status::Success;
}

ExternalTextureColorSpaceParams GetNoopColorSpaceParams() {
    ExternalTextureColorSpaceParams params;

    wgpu::Status status = ComputeExternalTextureParams(
        {
            .primaries = wgpu::ColorSpacePrimariesDawn::SRGB,
            .transfer = wgpu::ColorSpaceTransferDawn::Identity,
            .yCbCrRange = wgpu::ColorSpaceYCbCrRangeDawn::Identity,
            .yCbCrMatrix = wgpu::ColorSpaceYCbCrMatrixDawn::Identity,
        },
        wgpu::PredefinedColorSpace::SRGBLinear, &params);
    DAWN_ASSERT(status == wgpu::Status::Success);

    return params;
}

}  // namespace dawn
