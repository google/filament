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

#include "dawn/common/Algebra.h"

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
// The ITU specifications for the "source of truth" on many of the YCbCr colorspaces.
//
// Chromium's gfx::Colorspace.

// YCbCr range transforms, assuming that the Y and Cb Cr values are already normalized to [0, 1]
// since we get them via texture sampling.

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#QUANTIZATION_FULL
inline constexpr math::Mat4x3f kYCbCrRange_Full = {
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0},
    {0.0, -128.0 / 255.0, -128.0 / 255.0},
};

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#QUANTIZATION_NARROW
constexpr float kYCbCrRange_NarrowYFactor = 255.0 / 219.0;
constexpr float kYCbCrRange_NarrowChromaFactor = 255.0 / 224.0;
constexpr math::Mat4x3f kYCbCrRange_Narrow = {
    {kYCbCrRange_NarrowYFactor, 0.0, 0.0},
    {0.0, kYCbCrRange_NarrowChromaFactor, 0.0},
    {0.0, 0.0, kYCbCrRange_NarrowChromaFactor},
    {-16.0 / 255.0 * kYCbCrRange_NarrowYFactor,        //
     -128.0 / 255.0 * kYCbCrRange_NarrowChromaFactor,  //
     -128.0 / 255.0 * kYCbCrRange_NarrowChromaFactor},
};

// YCbCr to RGB matrices for various standards

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#MODEL_BT601
inline constexpr math::Mat3x3f kYCbCrToRGB_Rec601 = {
    {1.0, 1.0, 1.0},
    {0.0, -(0.202008 / 0.587), 1.772},
    {1.402, -(0.419198 / 0.587), 0.0},
};

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#MODEL_BT709
inline constexpr math::Mat3x3f kYCbCrToRGB_Rec709 = {
    {1.0, 1.0, 1.0},
    {0.0, -(0.13397432 / 0.7152), 1.8556},
    {1.5748, -(0.33480248 / 0.7152), 0.0},
};

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#MODEL_BT2020
inline constexpr math::Mat3x3f kYCbCrToRGB_Rec2020 = {
    {1.0, 1.0, 1.0},
    {0.0, -(0.11156702 / 0.6780), 1.8814},
    {1.4746, -(0.38737742 / 0.6780), 0.0},
};

// RGB to XYZ (D65) for various standards.
// TODO(https://crbug.com/468988322): Define an XYZPrimaries structure and compute the matrices from
// it. The wx and wy give the coordinates of the white point and would allow adapting to D50 if
// needed.

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#PRIMARIES_BT601_EBU
// Rec 601 625-line.
inline constexpr math::Mat3x3f kRGBToXYZ_Rec601 = {{0.430554, 0.222004, 0.020182},
                                                   {0.341550, 0.706655, 0.129553},
                                                   {0.178352, 0.071341, 0.939322}};
inline constexpr math::Mat3x3f kXYZToRGB_Rec601 = kRGBToXYZ_Rec601.Inverse();

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#PRIMARIES_BT709
inline constexpr math::Mat3x3f kRGBToXYZ_Rec709 = {
    {0.412391, 0.212639, 0.019331},
    {0.357584, 0.715169, 0.119195},
    {0.180481, 0.072192, 0.950532},
};
inline constexpr math::Mat3x3f kXYZToRGB_Rec709 = kRGBToXYZ_Rec709.Inverse();

// sRGB is the same as Rec709.
inline constexpr math::Mat3x3f kRGBToXYZ_sRGB = kRGBToXYZ_Rec709;
inline constexpr math::Mat3x3f kXYZToRGB_sRGB = kXYZToRGB_Rec709;

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#PRIMARIES_BT2020
inline constexpr math::Mat3x3f kRGBToXYZ_Rec2020 = {
    {0.636958, 0.262700, 0.000000},
    {0.144617, 0.677998, 0.028073},
    {0.168881, 0.059302, 1.060985},
};
inline constexpr math::Mat3x3f kXYZToRGB_Rec2020 = kRGBToXYZ_Rec2020.Inverse();

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#PRIMARIES_BT2020
inline constexpr math::Mat3x3f kRGBToXYZ_DisplayP3 = {
    {0.4865709486, 0.2289745641, 0.0000000000},
    {0.2656676932, 0.6917385218, 0.0451133819},
    {0.1982172852, 0.0792869141, 1.0439443689},
};
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
    .g = 2.4,
    .a = 1.0 / 1.055,
    .b = 0.055 / 1.055,
    .c = 1.0 / 12.92,
    .d = 0.04045,
    .e = 0,
    .f = 0,
};
inline constexpr TransferFunction kEOTFInverse_sRGB = {
    .g = 1.0 / 2.4,
    .a = 1.13711,  // 1.055 ^ 2.4
    .b = 0,
    .c = 12.92,
    .d = 0.0031308,
    .e = -0.055,
    .f = 0,
};

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#TRANSFER_DCIP3
inline constexpr TransferFunction kEOTF_DisplayP3 = {
    .g = 2.6,
    .a = 1.0,
    .b = 0,
    .c = 0,
    .d = 0,
    .e = 0,
    .f = 0,
};
inline constexpr TransferFunction kEOTFInverse_DisplayP3 = {
    .g = 1.0 / 2.6,
    .a = 1.0,
    .b = 0,
    .c = 0,
    .d = 0,
    .e = 0,
    .f = 0,
};

// https://registry.khronos.org/DataFormat/specs/1.4/dataformat.1.4.html#TRANSFER_ITU
inline constexpr TransferFunction kEOTF_SMPTE_170M = {
    .g = 1.0 / 0.45,
    .a = 1.0 / 1.099,
    .b = 0.099 / 1.099,
    .c = 1.0 / 4.5,
    .d = 0.0812,
    .e = 0,
    .f = 0,
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_COLORSPACE_H_
