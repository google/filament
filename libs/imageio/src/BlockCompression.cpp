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

#include <cmath>

#include <astcenc.h>

#define STB_DXT_IMPLEMENTATION
#include <stb_dxt.h>

using namespace image;

using std::string;

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
    if (config.blocksize == math::ushort2 {4, 4}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_4x4 : Format::RGBA_ASTC_4x4;
    } else if (config.blocksize == math::ushort2 {5, 4}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_5x4 : Format::RGBA_ASTC_5x4;
    } else if (config.blocksize == math::ushort2 {5, 5}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_5x5 : Format::RGBA_ASTC_5x5;
    } else if (config.blocksize == math::ushort2 {6, 5}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_6x5 : Format::RGBA_ASTC_6x5;
    } else if (config.blocksize == math::ushort2 {6, 6}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_6x6 : Format::RGBA_ASTC_6x6;
    } else if (config.blocksize == math::ushort2 {8, 5}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_8x5 : Format::RGBA_ASTC_8x5;
    } else if (config.blocksize == math::ushort2 {8, 6}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_8x6 : Format::RGBA_ASTC_8x6;
    } else if (config.blocksize == math::ushort2 {8, 8}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_8x8 : Format::RGBA_ASTC_8x8;
    } else if (config.blocksize == math::ushort2 {10, 5}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_10x5 : Format::RGBA_ASTC_10x5;
    } else if (config.blocksize == math::ushort2 {10, 6}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_10x6 : Format::RGBA_ASTC_10x6;
    } else if (config.blocksize == math::ushort2 {10, 8}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_10x8 : Format::RGBA_ASTC_10x8;
    } else if (config.blocksize == math::ushort2 {10, 10}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_10x10 : Format::RGBA_ASTC_10x10;
    } else if (config.blocksize == math::ushort2 {12, 10}) {
        format = config.srgb ? Format::SRGB8_ALPHA8_ASTC_12x10 : Format::RGBA_ASTC_12x10;
    } else if (config.blocksize == math::ushort2 {12, 12}) {
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
        // TODO: honor the other presets
        default:
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

    constexpr int threadcount = 1; // TODO: set this thread count
    constexpr astc_decode_mode decode_mode = DECODE_LDR; // TODO: honor the config semantic
    constexpr swizzlepattern swz_encode = { 0, 1, 2, 3 };
    constexpr swizzlepattern swz_decode = { 0, 1, 2, 3 };

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

AstcConfig astcParseOptionString(const string& configString) {
    const size_t _1 = configString.find('_');
    const size_t _2 = configString.find('_', _1 + 1);
    if (_1 == string::npos || _2 == string::npos) {
        return {};
    }
    string quality = configString.substr(0, _1);
    string semantic = configString.substr(_1 + 1, _2 - _1 - 1);
    string blocksize = configString.substr(_2 + 1);
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
    if (_x == string::npos) {
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
            dst[0] = (uint8_t) (rgba[0] * 255.0f);
            dst[1] = (uint8_t) (rgba[1] * 255.0f);
            dst[2] = (uint8_t) (rgba[2] * 255.0f);
            dst[3] = (uint8_t) (rgba[3] * 255.0f);
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
