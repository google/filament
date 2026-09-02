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

#ifndef SRC_DAWN_COMMON_COLORSPACE_H_
#define SRC_DAWN_COMMON_COLORSPACE_H_

#include "src/dawn/common/Algebra.h"

namespace dawn {

// Colorspaces define how numbers used to encode a color are related to physical quantities of
// light (for example captured by a camera, or emitted by a display). Dawn needs some handling of
// colorspace because ExternalTexture allows importing data in a source colorspace and sampling it
// in shaders with a different colorspace called the destination colorspace.
//
// Dawn also needs to tell the OS how to display values in a wgpu::Surface texture to the user, but
// that's done by passing the correct enum to the backend API's swapchain/surface configuration so
// Dawn doesn't need to do any math for that path.
//
// # RGB color spaces
//
// Colorspaces are defined with respect to a reference colorspace, usually the CIE XYZ one but the
// exact reference doesn't matter much in Dawn as long as all conversion are defined from/to it (so
// combining a transform from colorspace from A to XYZ and from XYZ to B gives a transform from A to
// B). Numbers in an RGB colorspace (as opposed to YCbCr, see below) can be converted to XYZ by
// performing two steps.
//
// The electro-optical transfer function (EOTF) that's used to transform colors in "perceptual"
// space to colors in "physical" space. This is because the human eye has a logarithmic response
// to light intensity (such as doubling the amount of light "feels" like a single step increase,
// the same mechanism as decibels for audition). Storing values in perceptual space is done to have
// as much perceptual precision for low and high color values and allows for smooth perceptual
// color gradients. (Read up on sRGB, that's the most used and well known perceptual color space).
// The inverse transfer function is called the opto-electronic transfer function (OETF).
//
// Note that a weirdness of HLG is that the EOTF is not the inverse of its OETF but of OETF(OOTF)
// (for unfortunate and historical reasons). The Annex 1 "The relationship between the OETF, the
// EOTF and the OOTF" of
// https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.2100-1-201706-S!!PDF-E.pdf
//
// The color primaries, what it means to say "red of 1" in that colorspace. This is where CIE XYZ is
// used as the primaries for the colorspace are mapped to colors in the XYZ colorspace. XYZ itself
// is a physical standard developed through experimentation on the human perception of color. The
// mapping from a colorspace to XYZ is a linear mapping represented by a matrix whose column vectors
// are the XYZ colors corresponding to the colorspace's primaries.
//
// Thus the transformation between colorspace A and B is:
//
//  - Apply A's EOTF.
//  - Transform the primaries, this is done with a single matrix multiply that's a combination or:
//    - Apply the matrix from A to XYZ
//    - Apply the matrix from XYZ to B
//  - Apply B's OETF (the inverse of the EOTF)
//
// # YCbCr color spaces
//
// The human vision is more sensitive to variation in light intensity (luminance) than color
// (chroma) so video (and some images) are stored in a YCbCr representation with the luminance Y and
// two dimensions for the chroma: Cr "the difference in red" and "Cb the difference in blue". Green
// is usually the luminance as it has the most response from human vision. Then the chroma data can
// be stored with lower frequency than luminance data, which gives storage and memory traffic gains.
//
// YCbCr colorspaces define how to convert YCbCr data to RGB, and how that RGB data converts to XYZ
// (with an EOTF and primaries). YCbCr data is converted to RGB by performing two steps.
//
// The transfer function defines how to map numbers to values with Y in [0, 1] and both Cb and Cr in
// [-0.5, 0.5] (assuming we are not using some wide gamut). The "full range" converts by normalizing
// values almost as one would expect (a uint8_t Y by dividing by 255 and a uint8_t Cb/Cr by dividing
// by 128 and offsetting by -0.5). However historically some values where used as control signal and
// a "narrow range" of values was used (for Y this is [16, 235]).
//
// The YCbCr to RGB linear transformation that defines how Y, Cb and Cr map to RGB..
//
// Thus the transformation between YCbCr colorspace A and RGB colorspace B is:
//
// - Transform numbers representing YCbCr to RGB with a single 4x3 matrix multiply that's a
//   combination of:
//   - Apply the 4x3 matrix in homogeneous coordinate space that computes the range (the homogeneous
//     coordinate space is used to allow for translation to be encoded in the matrix).
//   - Apply the 3x3 YCbCr to RGB linear transformation.
//
//  (same steps as for the RGB case)
//
//  - Apply A's EOTF.
//  - Transform the primaries, this is done with a single matrix multiply that's a combination or:
//    - Apply the matrix from A to XYZ
//    - Apply the matrix from XYZ to B
//  - Apply B's OETF (the inverse of the EOTF)
//
// # Resources
//
// The Khronos Data Format Specification that makes the ITU standards more approachable (it is
// linked to many times below):
// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html
//
// The CSS color module are another great reference:
//
//  - CSS Color Module Level 4 for the definition of many SDR color spaces:
//    https://www.w3.org/TR/css-color-4/
//  - CSS Color Module HDR Level 1 for the definition of HDR color spaces:
//    https://www.w3.org/TR/css-color-hdr-1/
//
// The ITU specifications for the "source of truth" on many of the YCbCr colorspaces.
//
// Chromium's gfx::Colorspace.

// YCbCr range transforms, assuming that the Y and Cb Cr values are already normalized to [0, 1]
// since we get them via texture sampling.

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#QUANTIZATION_FULL
inline constexpr math::Mat4x3f kYCbCrRange_Full = {
    {1.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {0.0f, -128.0f / 255.0f, -128.0f / 255.0f},
};

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#QUANTIZATION_NARROW
constexpr float kYCbCrRange_NarrowYFactor = 255.0f / 219.0f;
constexpr float kYCbCrRange_NarrowChromaFactor = 255.0f / 224.0f;
constexpr auto kYCbCrRange_Narrow = math::Mat4x3f::FromRows({
    {kYCbCrRange_NarrowYFactor, 0.0f, 0.0f, -16.0f / 255.0f * kYCbCrRange_NarrowYFactor},
    {0.0f, kYCbCrRange_NarrowChromaFactor, 0.0f, -128.0f / 255.0f * kYCbCrRange_NarrowChromaFactor},
    {0.0f, 0.0f, kYCbCrRange_NarrowChromaFactor, -128.0f / 255.0f * kYCbCrRange_NarrowChromaFactor},
});

// YCbCr to RGB matrices for various standards

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#MODEL_BT601
inline constexpr auto kYCbCrToRGB_Rec601 = math::Mat3x3f::FromRows({
    {1.0f, 0.0f, 1.402f},
    {1.0f, -(0.202008f / 0.587f), -(0.419198f / 0.587f)},
    {1.0f, 1.772f, 0.0f},
});

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#MODEL_BT709
inline constexpr auto kYCbCrToRGB_Rec709 = math::Mat3x3f::FromRows({
    {1.0f, 0.0f, 1.5748f},
    {1.0f, -(0.133974f / 0.7152f), -(0.334802f / 0.7152f)},
    {1.0f, 1.8556f, 0.0f},
});

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#MODEL_BT2020
inline constexpr auto kYCbCrToRGB_Rec2020 = math::Mat3x3f::FromRows({
    {1.0f, 0.0f, 1.4746f},
    {1.0f, -(0.111567f / 0.678f), -(0.387377f / 0.678f)},
    {1.0f, 1.8814f, 0.0f},
});

// RGB to XYZ (D65) for various standards.
// TODO(https://crbug.com/468988322): Define an XYZPrimaries structure and compute the matrices from
// it. The wx and wy give the coordinates of the white point and would allow adapting to D50 if
// needed.

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#PRIMARIES_BT601_EBU
// Rec 601 625-line.
inline constexpr auto kRGBToXYZ_Rec601 = math::Mat3x3f::FromRows({
    {0.430554f, 0.341550f, 0.178352f},
    {0.222004f, 0.706655f, 0.071341f},
    {0.020182f, 0.129553f, 0.939322f},
});
inline constexpr math::Mat3x3f kXYZToRGB_Rec601 = kRGBToXYZ_Rec601.Inverse();

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#PRIMARIES_BT709
inline constexpr auto kRGBToXYZ_Rec709 = math::Mat3x3f::FromRows({
    {0.412391f, 0.357584f, 0.180481f},
    {0.212639f, 0.715169f, 0.072192f},
    {0.019331f, 0.119195f, 0.950532f},
});
inline constexpr math::Mat3x3f kXYZToRGB_Rec709 = kRGBToXYZ_Rec709.Inverse();

// sRGB is the same as Rec709.
inline constexpr math::Mat3x3f kRGBToXYZ_sRGB = kRGBToXYZ_Rec709;
inline constexpr math::Mat3x3f kXYZToRGB_sRGB = kXYZToRGB_Rec709;

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#PRIMARIES_BT2020
inline constexpr auto kRGBToXYZ_Rec2020 = math::Mat3x3f::FromRows({
    {0.636958f, 0.144617f, 0.168881f},
    {0.262700f, 0.677998f, 0.059302f},
    {0.000000f, 0.028073f, 1.060985f},
});
inline constexpr math::Mat3x3f kXYZToRGB_Rec2020 = kRGBToXYZ_Rec2020.Inverse();

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#PRIMARIES_BT2020
inline constexpr auto kRGBToXYZ_DisplayP3 = math::Mat3x3f::FromRows({
    {0.4865709486f, 0.2656676932f, 0.1982172852f},
    {0.2289745641f, 0.6917385218f, 0.0792869141f},
    {0.0000000000f, 0.0451133819f, 1.0439443689f},
});
inline constexpr math::Mat3x3f kXYZToRGB_DisplayP3 = kRGBToXYZ_DisplayP3.Inverse();

// Transfer functions for various standards, in a format.

// Returns an EOTF that performs the following transformation.
//
//    if (abs(v) < D) {
//        return sign(v) * (C * abs(v) + F);
//    }
//    return pow(A * x + B, G) + E
//
// TODO(https://crbug.com/497571469): Add support for PQ and HLG.
struct TransferFunction {
    float g = 1;
    float a = 1;
    float b = 0;
    float c = 0;
    float d = 0;
    float e = 0;
    float f = 0;
};

inline constexpr TransferFunction kEOTF_Identity = {};

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#TRANSFER_SRGB
inline constexpr TransferFunction kEOTF_sRGB = {
    .g = 2.4f,
    .a = 1.0f / 1.055f,
    .b = 0.055f / 1.055f,
    .c = 1.0f / 12.92f,
    .d = 0.04045f,
    .e = 0.0f,
    .f = 0.0f,
};
inline constexpr TransferFunction kEOTFInverse_sRGB = {
    .g = 1.0f / 2.4f,
    .a = 1.13711f,  // 1.055 ^ 2.4
    .b = 0.0f,
    .c = 12.92f,
    .d = 0.0031308f,
    .e = -0.055f,
    .f = 0.0f,
};

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#TRANSFER_BT1886
// BT1886 also known as Gamma 2.4
inline constexpr TransferFunction kEOTF_BT_1886 = {
    .g = 2.4f,
    .a = 1.0f,
    .b = 0.0f,
    .c = 0.0f,
    .d = 0.0f,
    .e = 0.0f,
    .f = 0.0f,
};

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#TRANSFER_DCIP3
inline constexpr TransferFunction kEOTF_DisplayP3 = {
    .g = 2.6f,
    .a = 1.0f,
    .b = 0.0f,
    .c = 0.0f,
    .d = 0.0f,
    .e = 0.0f,
    .f = 0.0f,
};
inline constexpr TransferFunction kEOTFInverse_DisplayP3 = {
    .g = 1.0f / 2.6f,
    .a = 1.0f,
    .b = 0.0f,
    .c = 0.0f,
    .d = 0.0f,
    .e = 0.0f,
    .f = 0.0f,
};

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#TRANSFER_ITU
inline constexpr TransferFunction kEOTF_SMPTE_170M = {
    .g = 1.0f / 0.45f,
    .a = 1.0f / 1.099f,
    .b = 0.099f / 1.099f,
    .c = 1.0f / 4.5f,
    .d = 0.0812f,
    .e = 0.0f,
    .f = 0.0f,
};

// Luminance values of various color spaces for the white of value "1" after the transfer function.

// When SDR content is composited in HDR, it should be assumed to have a white at 203 nits, even if
// the sRGB spec defines it as 80 nits. Note that this constant is only informative and Dawn allows
// configuring the reference white luminance with wgpu::ColorSpaceDawn::hdrReferenceWhiteLuminance.
// See https://www.w3.org/TR/css-color-hdr-1/#Compositing-SDR-HDR and Section 2.1 "HDR Reference
// White" of https://www.itu.int/dms_pub/itu-r/opb/rep/R-REP-BT.2408-9-2026-PDF-E.pdf
inline constexpr float kDefaultHDRReferenceWhiteLuminance = 203;

// https://www.w3.org/TR/css-color-hdr-1/#valdef-color-rec2100-pq
inline constexpr float kPQLuminanceOf1 = 10000;

// https://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.2100-3-202502-I!!PDF-E.pdf table 5
// "Hybrid Log-Gamma (HLG) system reference non-linear transfer functions". The peak luminance of
// HLG is 1 (assuming an OOTF of 1.2).
inline constexpr float kHLGLuminanceOf1 = 1000;

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_COLORSPACE_H_
