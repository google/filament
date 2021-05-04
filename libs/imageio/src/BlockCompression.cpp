/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <imageio/BlockCompression.h>

#include <image/ImageOps.h>

#include <algorithm>
#include <cmath>
#include <thread>

#ifdef FILAMENT_USE_HUNTER
#include <astc-encoder/astcenc.h>
#include <etc2comp/EtcLib/Etc/Etc.h>
#else
#include <astcenc.h>
#include <Etc.h>
#endif

#define STB_DXT_IMPLEMENTATION
#ifdef FILAMENT_USE_HUNTER
#include <stb/stb_dxt.h>
#else
#include <stb_dxt.h>
#endif

namespace image {

static LinearImage extendToFourChannels(LinearImage source);

CompressedTexture astcCompress(const LinearImage& original, AstcConfig config) {

    // If this is the first time, initialize the ARM encoder tables.

    static bool first = true;
    if (first) {
        test_inappropriate_extended_precision();
        prepare_angular_tables();
        build_quantization_mode_table();
        first = false;
    }

    // Check the validity of the given block size.

    using Format = CompressedFormat;
    Format format;
    if (config.blocksize ==  filament::math::ushort2 {4, 4}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_4x4 : Format::RGBA_ASTC_4x4;
    } else if (config.blocksize ==  filament::math::ushort2 {5, 4}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_5x4 : Format::RGBA_ASTC_5x4;
    } else if (config.blocksize ==  filament::math::ushort2 {5, 5}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_5x5 : Format::RGBA_ASTC_5x5;
    } else if (config.blocksize ==  filament::math::ushort2 {6, 5}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_6x5 : Format::RGBA_ASTC_6x5;
    } else if (config.blocksize ==  filament::math::ushort2 {6, 6}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_6x6 : Format::RGBA_ASTC_6x6;
    } else if (config.blocksize ==  filament::math::ushort2 {8, 5}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_8x5 : Format::RGBA_ASTC_8x5;
    } else if (config.blocksize ==  filament::math::ushort2 {8, 6}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_8x6 : Format::RGBA_ASTC_8x6;
    } else if (config.blocksize ==  filament::math::ushort2 {8, 8}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_8x8 : Format::RGBA_ASTC_8x8;
    } else if (config.blocksize ==  filament::math::ushort2 {10, 5}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_10x5 : Format::RGBA_ASTC_10x5;
    } else if (config.blocksize ==  filament::math::ushort2 {10, 6}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_10x6 : Format::RGBA_ASTC_10x6;
    } else if (config.blocksize ==  filament::math::ushort2 {10, 8}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_10x8 : Format::RGBA_ASTC_10x8;
    } else if (config.blocksize ==  filament::math::ushort2 {10, 10}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_10x10 : Format::RGBA_ASTC_10x10;
    } else if (config.blocksize ==  filament::math::ushort2 {12, 10}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_12x10 : Format::RGBA_ASTC_12x10;
    } else if (config.blocksize ==  filament::math::ushort2 {12, 12}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_12x12 : Format::RGBA_ASTC_12x12;
    } else {
        return {};
    }

    // Create an input image for the ARM encoder in a format that it can consume.
    // It expects four-channel data, so we extend or curtail the channel count in a reasonable way.
    // The encoder can take half-floats or bytes, but we always give it half-floats.

    LinearImage source = extendToFourChannels(original);
    const uint32_t width = source.getWidth();
    const uint32_t height = source.getHeight();
    astc_codec_image* input_image = allocate_image(16, width, height, 1, 0);
    for (int y = 0; y < height; y++) {
        auto imagedata16 = input_image->imagedata16[0][y];
        float const* src = source.getPixelRef(0, y);
        for (int x = 0; x < width; x++) {
            imagedata16[4 * x] = float_to_sf16(src[4 * x], SF_NEARESTEVEN);
            imagedata16[4 * x + 1] = float_to_sf16(src[4 * x + 1], SF_NEARESTEVEN);
            imagedata16[4 * x + 2] = float_to_sf16(src[4 * x + 2], SF_NEARESTEVEN);
            imagedata16[4 * x + 3] = float_to_sf16(src[4 * x + 3], SF_NEARESTEVEN);
        }
    }

    // Determine the bitrate based on the specified block size.

    int xdim_2d = config.blocksize.x, ydim_2d = config.blocksize.y;
    const float log10_texels_2d = std::log((float)(xdim_2d * ydim_2d)) / std::log(10.0f);
    const float bitrate = 128.0 / (xdim_2d * ydim_2d);

    // We do not fully support 3D textures yet, but we include some of the 3D config params anyway.

    int xdim_3d, ydim_3d, zdim_3d;
    find_closest_blockdim_3d(bitrate, &xdim_3d, &ydim_3d, &zdim_3d, 0);
    const float log10_texels_3d = std::log((float)(xdim_3d * ydim_3d * zdim_3d)) / log(10.0f);

    // Set up presets.

    int plimit_autoset;
    float oplimit_autoset;
    float dblimit_autoset_2d;
    float dblimit_autoset_3d;
    float bmc_autoset;
    float mincorrel_autoset;
    int maxiters_autoset;
    int pcdiv;

    switch (config.quality) {
        case AstcPreset::VERYFAST:
            plimit_autoset = 2;
            oplimit_autoset = 1.0;
            dblimit_autoset_2d = fmax(70 - 35 * log10_texels_2d, 53 - 19 * log10_texels_2d);
            dblimit_autoset_3d = fmax(70 - 35 * log10_texels_3d, 53 - 19 * log10_texels_3d);
            bmc_autoset = 25;
            mincorrel_autoset = 0.5;
            maxiters_autoset = 1;
            switch (ydim_2d) {
                case 4: pcdiv = 240; break;
                case 5: pcdiv = 56; break;
                case 6: pcdiv = 64; break;
                case 8: pcdiv = 47; break;
                case 10: pcdiv = 36; break;
                case 12: pcdiv = 30; break;
                default: pcdiv = 30; break;
            }
            break;
        case AstcPreset::FAST:
            plimit_autoset = 4;
            oplimit_autoset = 1.0;
            dblimit_autoset_2d = fmax(85 - 35 * log10_texels_2d, 63 - 19 * log10_texels_2d);
            dblimit_autoset_3d = fmax(85 - 35 * log10_texels_3d, 63 - 19 * log10_texels_3d);
            bmc_autoset = 50;
            mincorrel_autoset = 0.5;
            maxiters_autoset = 1;
            switch (ydim_2d) {
                case 4: pcdiv = 60; break;
                case 5: pcdiv = 27; break;
                case 6: pcdiv = 30; break;
                case 8: pcdiv = 24; break;
                case 10: pcdiv = 16; break;
                case 12: pcdiv = 20; break;
                default: pcdiv = 20; break;
            }
            break;
        case AstcPreset::MEDIUM:
            plimit_autoset = 25;
            oplimit_autoset = 1.2;
            dblimit_autoset_2d = fmax(95 - 35 * log10_texels_2d, 70 - 19 * log10_texels_2d);
            dblimit_autoset_3d = fmax(95 - 35 * log10_texels_3d, 70 - 19 * log10_texels_3d);
            bmc_autoset = 75;
            mincorrel_autoset = 0.75;
            maxiters_autoset = 2;
            switch (ydim_2d) {
                case 4: pcdiv = 25; break;
                case 5: pcdiv = 15; break;
                case 6: pcdiv = 15; break;
                case 8: pcdiv = 10; break;
                case 10: pcdiv = 8; break;
                case 12: pcdiv = 6; break;
                default: pcdiv = 6; break;
            }
            break;
        case AstcPreset::THOROUGH:
            plimit_autoset = 100;
            oplimit_autoset = 2.5;
            dblimit_autoset_2d = fmax(105 - 35 * log10_texels_2d, 77 - 19 * log10_texels_2d);
            dblimit_autoset_3d = fmax(105 - 35 * log10_texels_3d, 77 - 19 * log10_texels_3d);
            bmc_autoset = 95;
            mincorrel_autoset = 0.95f;
            maxiters_autoset = 4;
            switch (ydim_2d) {
                case 4: pcdiv = 12; break;
                case 5: pcdiv = 7; break;
                case 6: pcdiv = 7; break;
                case 8: pcdiv = 5; break;
                case 10: pcdiv = 4; break;
                case 12: pcdiv = 3; break;
                default: pcdiv = 3; break;
            }
            break;
        case AstcPreset::EXHAUSTIVE:
            plimit_autoset = 1 << 10;
            oplimit_autoset = 1000.0;
            dblimit_autoset_2d = 999.0f;
            dblimit_autoset_3d = 999.0f;
            bmc_autoset = 100;
            mincorrel_autoset = 0.99;
            maxiters_autoset = 4;
            switch (ydim_2d) {
                case 4: pcdiv = 3; break;
                case 5: pcdiv = 1; break;
                case 6: pcdiv = 1; break;
                case 8: pcdiv = 1; break;
                case 10: pcdiv = 1; break;
                case 12: pcdiv = 1; break;
                default: pcdiv = 1; break;
            }
            break;
    }

    if (plimit_autoset < 1) {
        plimit_autoset = 1;
    } else if (plimit_autoset > PARTITION_COUNT) {
        plimit_autoset = PARTITION_COUNT;
    }

    error_weighting_params ewp;
    ewp.rgb_power = 1.0f;
    ewp.alpha_power = 1.0f;
    ewp.rgb_base_weight = 1.0f;
    ewp.alpha_base_weight = 1.0f;
    ewp.rgb_mean_weight = 0.0f;
    ewp.rgb_stdev_weight = 0.0f;
    ewp.alpha_mean_weight = 0.0f;
    ewp.alpha_stdev_weight = 0.0f;
    ewp.rgb_mean_and_stdev_mixing = 0.0f;
    ewp.mean_stdev_radius = 0;
    ewp.enable_rgb_scale_with_alpha = 0;
    ewp.alpha_radius = 0;
    ewp.block_artifact_suppression = 0.0f;
    ewp.rgba_weights[0] = 1.0f;
    ewp.rgba_weights[1] = 1.0f;
    ewp.rgba_weights[2] = 1.0f;
    ewp.rgba_weights[3] = 1.0f;
    ewp.ra_normal_angular_scale = 0;
    ewp.max_refinement_iters = maxiters_autoset;
    ewp.block_mode_cutoff = bmc_autoset / 100.0f;
    ewp.texel_avg_error_limit = pow(0.1f, dblimit_autoset_2d * 0.1f) * 65535.0f * 65535.0f;
    ewp.partition_1_to_2_limit = oplimit_autoset;
    ewp.lowest_correlation_cutoff = mincorrel_autoset;
    ewp.partition_search_limit = plimit_autoset;

    // For now we do not support 3D textures but we keep the variable names consistent
    // with what's found in the ARM standalone tool.
    int xdim = xdim_2d, ydim = ydim_2d, zdim = 1;
    expand_block_artifact_suppression(xdim, ydim, zdim, &ewp);

    // Perform compression.

    swizzlepattern swz_encode = { 0, 1, 2, 3 };
    swizzlepattern swz_decode = { 0, 1, 2, 3 };
    astc_decode_mode decode_mode;
    switch (config.semantic) {
        case AstcSemantic::COLORS_LDR:
            decode_mode = config.srgb ? DECODE_LDR_SRGB : DECODE_LDR;
            break;
        case AstcSemantic::COLORS_HDR:
            decode_mode = DECODE_HDR;
            break;
        case AstcSemantic::NORMALS:
            decode_mode = config.srgb ? DECODE_LDR_SRGB : DECODE_LDR;
            ewp.rgba_weights[0] = 1.0f;
            ewp.rgba_weights[1] = 0.0f;
            ewp.rgba_weights[2] = 0.0f;
            ewp.rgba_weights[3] = 1.0f;
            ewp.ra_normal_angular_scale = 1;
            swz_encode.r = 0;
            swz_encode.g = 0;
            swz_encode.b = 0;
            swz_encode.a = 1;
            swz_decode.r = 0;
            swz_decode.g = 3;
            swz_decode.b = 6;
            swz_decode.a = 5;
            ewp.block_artifact_suppression = 1.8f;
            ewp.mean_stdev_radius = 3;
            ewp.rgb_mean_weight = 0;
            ewp.rgb_stdev_weight = 50;
            ewp.rgb_mean_and_stdev_mixing = 0.0;
            ewp.alpha_mean_weight = 0;
            ewp.alpha_stdev_weight = 50;
            break;
    }

    const int threadcount = std::thread::hardware_concurrency();

    const int xsize = input_image->xsize;
    const int ysize = input_image->ysize;
    const int zsize = input_image->zsize;
    const int xblocks = (xsize + xdim - 1) / xdim;
    const int yblocks = (ysize + ydim - 1) / ydim;
    const int zblocks = (zsize + zdim - 1) / zdim;

    uint32_t size = xblocks * yblocks * zblocks * 16;
    uint8_t* buffer = new uint8_t[size];

    encode_astc_image(input_image, nullptr, xdim, ydim, zdim, &ewp, decode_mode,
            swz_encode, swz_decode, buffer, 0, threadcount);

    destroy_image(input_image);

    return {
        .format = format,
        .size = size,
        .data = decltype(CompressedTexture::data)(buffer)
    };
}

AstcConfig astcParseOptionString(const std::string& configString) {
    const size_t _1 = configString.find('_');
    const size_t _2 = configString.find('_', _1 + 1);
    if (_1 == std::string::npos || _2 == std::string::npos) {
        return {};
    }
    std::string quality = configString.substr(0, _1);
    std::string semantic = configString.substr(_1 + 1, _2 - _1 - 1);
    std::string blocksize = configString.substr(_2 + 1);
    AstcConfig config;
    if (quality == "veryfast") {
        config.quality = AstcPreset::VERYFAST;
    } else if (quality == "fast") {
        config.quality = AstcPreset::FAST;
    } else if (quality == "medium") {
        config.quality = AstcPreset::MEDIUM;
    } else if (quality == "thorough") {
        config.quality = AstcPreset::THOROUGH;
    } else if (quality == "exhaustive") {
        config.quality = AstcPreset::EXHAUSTIVE;
    } else {
        return {};
    }
    if (semantic == "ldr") {
        config.semantic = AstcSemantic::COLORS_LDR;
    } else if (semantic == "hdr") {
        config.semantic = AstcSemantic::COLORS_HDR;
    } else if (semantic == "normals") {
        config.semantic = AstcSemantic::NORMALS;
    } else {
        return {};
    }

    const size_t _x = blocksize.find('x');
    if (_x == std::string::npos) {
        return {};
    }
    config.blocksize[0] = std::stoi(blocksize.substr(0, _x));
    config.blocksize[1] = std::stoi(blocksize.substr(_x + 1));
    return config;
}

static uint32_t imin(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

static void extract4x4RGBA(uint8_t* dst, const LinearImage& source, uint32_t x0, uint32_t y0) {
    const uint32_t maxx = source.getWidth() - 1;
    const uint32_t maxy = source.getHeight() - 1;
    for (uint32_t y = y0, y1 = y0 + 4; y < y1; ++y) {
        for (uint32_t x = x0, x1 = x0 + 4; x < x1; ++x, dst += 4) {
            int clamped_x = imin(maxx, x);
            int clamped_y = imin(maxy, y);
            float const* rgba = source.getPixelRef(clamped_x, clamped_y);
            dst[0] = (uint8_t) std::min(255.0f, std::max(0.0f, (rgba[0] * 255.0f)));
            dst[1] = (uint8_t) std::min(255.0f, std::max(0.0f, (rgba[1] * 255.0f)));
            dst[2] = (uint8_t) std::min(255.0f, std::max(0.0f, (rgba[2] * 255.0f)));
            dst[3] = (uint8_t) std::min(255.0f, std::max(0.0f, (rgba[3] * 255.0f)));
        }
    }
}

// Our S3TC / DXT encoder uses the STB implementation by Fabian Giesen.
//
// Due to limitations in STB, this only supports the following formats:
//  - DXT1 with no alpha (16 input pixels in 64 bits of output, 6:1)
//  - DXT5 with alpha (16 input pixels into 128 bits of output, 4:1)
//
// TODO: investigate using something more capable than STB (eg AMD Compressenator, bimg, libsquish)
CompressedTexture s3tcCompress(const LinearImage& original, S3tcConfig config) {
    const bool dxt5 = config.format == CompressedFormat::RGBA_S3TC_DXT5;
    LinearImage source = extendToFourChannels(original);
    uint8_t block[64];
    uint32_t xblocks = (source.getWidth() + 3) / 4;
    uint32_t yblocks = (source.getHeight() + 3) / 4;
    uint32_t size = xblocks * yblocks * (dxt5 ? 16 : 8);
    uint8_t* buffer = new uint8_t[size];
    uint8_t* dst = buffer;
    for (int y = 0, h = source.getHeight(); y < h; y += 4) {
        for (int x = 0, w = source.getWidth(); x < w; x += 4) {
            extract4x4RGBA(block, source, x, y);
            stb_compress_dxt_block(dst, block, dxt5, 8);
            dst += dxt5 ? 16 : 8;
        }
    }
    return {
        .format = config.format,
        .size = size,
        .data = decltype(CompressedTexture::data)(buffer)
    };
}

S3tcConfig s3tcParseOptionString(const std::string& options) {
    if (options == "rgb_dxt1") {
        return {CompressedFormat::RGB_S3TC_DXT1, false};
    }
    if (options == "rgba_dxt5") {
        return {CompressedFormat::RGBA_S3TC_DXT5, false};
    }
    return {};
}

CompressedTexture etcCompress(const LinearImage& original, EtcConfig config) {
    LinearImage source = extendToFourChannels(original);
    const int threadcount = std::thread::hardware_concurrency();
    Etc::Image::Format etcformat;
    switch (config.format) {
        case CompressedFormat::R11_EAC: etcformat = Etc::Image::Format::R11; break;
        case CompressedFormat::SIGNED_R11_EAC: etcformat = Etc::Image::Format::SIGNED_R11; break;
        case CompressedFormat::RG11_EAC: etcformat = Etc::Image::Format::RG11; break;
        case CompressedFormat::SIGNED_RG11_EAC: etcformat = Etc::Image::Format::SIGNED_RG11; break;
        case CompressedFormat::RGB8_ETC2: etcformat = Etc::Image::Format::RGB8; break;
        case CompressedFormat::SRGB8_ETC2: etcformat = Etc::Image::Format::SRGB8; break;
        case CompressedFormat::RGB8_ALPHA1_ETC2: etcformat = Etc::Image::Format::RGB8A1; break;
        case CompressedFormat::SRGB8_ALPHA1_ETC: etcformat = Etc::Image::Format::SRGB8A1; break;
        case CompressedFormat::RGBA8_ETC2_EAC: etcformat = Etc::Image::Format::RGBA8; break;
        case CompressedFormat::SRGB8_ALPHA8_ETC2_EAC: etcformat = Etc::Image::Format::SRGBA8; break;
        default: return {};
    }
    Etc::ErrorMetric etcmetric;
    switch (config.metric) {
        case EtcErrorMetric::RGBA: etcmetric = Etc::RGBA; break;
        case EtcErrorMetric::RGBX: etcmetric = Etc::RGBX; break;
        case EtcErrorMetric::REC709: etcmetric = Etc::REC709; break;
        case EtcErrorMetric::NUMERIC: etcmetric = Etc::NUMERIC; break;
        case EtcErrorMetric::NORMALXYZ: etcmetric = Etc::NORMALXYZ; break;
        default: return {};
    }
    unsigned char *paucEncodingBits;
    unsigned int uiEncodingBitsBytes;
    unsigned int uiExtendedWidth;
    unsigned int uiExtendedHeight;
    int iEncodingTime_ms;

    // The etc2comp API doesn't tell you that you need to free paucEncodingBits, but they have a
    // commented-out "delete[] m_paucEncodingBits" in their Image destructor, which is essentially
    // what our unique_ptr wrapper does (CompressedTexture::data).

    Etc::Encode(source.getPixelRef(0, 0),
        source.getWidth(), source.getHeight(),
        etcformat,
        etcmetric,
        config.effort,
        threadcount,
        1024,
        &paucEncodingBits, &uiEncodingBitsBytes,
        &uiExtendedWidth, &uiExtendedHeight,
        &iEncodingTime_ms);

    return {
        .format = config.format,
        .size = uiEncodingBitsBytes,
        .data = decltype(CompressedTexture::data)(paucEncodingBits)
    };
}

EtcConfig etcParseOptionString(const std::string& options) {
    EtcConfig result {};
    const size_t _2 = options.rfind('_');
    const size_t _1 = options.rfind('_', _2 - 1);
    std::string sformat = options.substr(0, _1);
    std::string smetric = options.substr(_1 + 1, _2 - _1 - 1);
    std::string seffort = options.substr(_2 + 1);
    if (sformat == "r11") {
        result.format = CompressedFormat::R11_EAC;
    } else if (sformat == "signed_r11") {
        result.format = CompressedFormat::SIGNED_R11_EAC;
    } else if (sformat == "rg11") {
        result.format = CompressedFormat::RG11_EAC;
    } else if (sformat == "signed_rg11") {
        result.format = CompressedFormat::SIGNED_RG11_EAC;
    } else if (sformat == "rgb8") {
        result.format = CompressedFormat::RGB8_ETC2;
    } else if (sformat == "srgb8") {
        result.format = CompressedFormat::SRGB8_ETC2;
    } else if (sformat == "rgb8_alpha") {
        result.format = CompressedFormat::RGB8_ALPHA1_ETC2;
    } else if (sformat == "srgb8_alpha") {
        result.format = CompressedFormat::SRGB8_ALPHA1_ETC;
    } else if (sformat == "rgba8") {
        result.format = CompressedFormat::RGBA8_ETC2_EAC;
    } else if (sformat == "srgb8_alpha8") {
        result.format = CompressedFormat::SRGB8_ALPHA8_ETC2_EAC;
    }
    if (smetric == "rgba") {
        result.metric = EtcErrorMetric::RGBA;
    } else if (smetric == "rgbx") {
        result.metric = EtcErrorMetric::RGBX;
    } else if (smetric == "rec709") {
        result.metric = EtcErrorMetric::REC709;
    } else if (smetric == "numeric") {
        result.metric = EtcErrorMetric::NUMERIC;
    } else if (smetric == "normalxyz") {
        result.metric = EtcErrorMetric::NORMALXYZ;
    }
    result.effort = std::stoi(seffort);
    return result;
}

bool parseOptionString(const std::string& options, CompressionConfig* config) {
    config->type = CompressionConfig::INVALID;
    if (options.substr(0, 5) == "astc_") {
        config->astc = astcParseOptionString(options.substr(5));
        if (config->astc.blocksize[0] != 0) {
            config->type = CompressionConfig::ASTC;
        }
    } else if (options.substr(0, 5) == "s3tc_") {
        config->s3tc = s3tcParseOptionString(options.substr(5));
        if (config->s3tc.format != CompressedFormat::INVALID) {
            config->type = CompressionConfig::S3TC;
        }
    } else if (options.substr(0, 4) == "etc_") {
        config->etc = etcParseOptionString(options.substr(4));
        if (config->etc.format != CompressedFormat::INVALID) {
            config->type = CompressionConfig::ETC;
        }
    }
    return config->type != CompressionConfig::INVALID;
}

CompressedTexture compressTexture(const CompressionConfig& config, const LinearImage& image) {
    if (config.type == CompressionConfig::ASTC) {
        return astcCompress(image, config.astc);
    }
    if (config.type == CompressionConfig::S3TC) {
        return s3tcCompress(image, config.s3tc);
    }
    if (config.type == CompressionConfig::ETC) {
        return etcCompress(image, config.etc);
    }
    return {};
}

static LinearImage extendToFourChannels(LinearImage original) {
    LinearImage source = original;
    const uint32_t width = source.getWidth();
    const uint32_t height = source.getHeight();
    auto createEmptyImage = [width, height](float value) {
        auto result = LinearImage(width, height, 1);
        float* pixels = result.getPixelRef(0, 0);
        std::fill(pixels, pixels + width * height, value);
        return result;
    };
    switch (source.getChannels()) {
        case 4: break;
        case 1: {
            auto l = image::extractChannel(source, 0);
            auto a = createEmptyImage(1.0f);
            source = image::combineChannels({l, l, l, a});
        }
        case 2: {
            auto l = image::extractChannel(source, 0);
            auto a = image::extractChannel(source, 1);
            source = image::combineChannels({l, l, l, a});
        }
        case 3: {
            auto r = image::extractChannel(source, 0);
            auto g = image::extractChannel(source, 1);
            auto b = image::extractChannel(source, 2);
            auto a = createEmptyImage(1.0f);
            source = image::combineChannels({r, g, b, a});
        }
        default: {
            auto r = image::extractChannel(source, 0);
            auto g = image::extractChannel(source, 1);
            auto b = image::extractChannel(source, 2);
            auto a = image::extractChannel(source, 3);
            source = image::combineChannels({r, g, b, a});
        }
    }
    return source;
}

} // namespace image
