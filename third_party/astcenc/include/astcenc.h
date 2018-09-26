// -------------------------------------------------------------------------------------------------
// This file declares the minimum amount of constants, enums, structs, and functions that are needed
// when interfacing with astcenc, which has no public-facing header files. To see how to use this
// interface, see astc_toplevel.cpp
// -------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>

#define MAX_TEXELS_PER_BLOCK 216
#define PARTITION_BITS 10
#define PARTITION_COUNT (1 << PARTITION_BITS)

enum astc_decode_mode {
    DECODE_LDR_SRGB,
    DECODE_LDR,
    DECODE_HDR
};

struct astc_codec_image {
    uint8_t*** imagedata8;
    uint16_t*** imagedata16;
    int xsize;
    int ysize;
    int zsize;
    int padding;
};

struct error_weighting_params {
    float rgb_power;
    float rgb_base_weight;
    float rgb_mean_weight;
    float rgb_stdev_weight;
    float alpha_power;
    float alpha_base_weight;
    float alpha_mean_weight;
    float alpha_stdev_weight;
    float rgb_mean_and_stdev_mixing;
    int mean_stdev_radius;
    int enable_rgb_scale_with_alpha;
    int alpha_radius;
    int ra_normal_angular_scale;
    float block_artifact_suppression;
    float rgba_weights[4];
    float block_artifact_suppression_expanded[MAX_TEXELS_PER_BLOCK];
    int partition_search_limit;
    float block_mode_cutoff;
    float texel_avg_error_limit;
    float partition_1_to_2_limit;
    float lowest_correlation_cutoff;
    int max_refinement_iters;
};

struct swizzlepattern {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

typedef uint16_t sf16;
typedef uint32_t sf32;

enum roundmode {
    SF_UP = 0,
    SF_DOWN = 1,
    SF_TOZERO = 2,
    SF_NEARESTEVEN = 3,
    SF_NEARESTAWAY = 4
};

extern void encode_astc_image(
    const astc_codec_image* input_image,
    astc_codec_image* output_image,
    int xdim,
    int ydim,
    int zdim,
    const error_weighting_params* ewp,
    astc_decode_mode decode_mode,
    swizzlepattern swz_encode,
    swizzlepattern swz_decode,
    uint8_t* buffer,
    int pack_and_unpack,
    int threadcount);

extern void expand_block_artifact_suppression(
    int xdim,
    int ydim,
    int zdim,
    error_weighting_params* ewp);

extern void find_closest_blockdim_2d(
    float target_bitrate,
    int *x,
    int *y,
    int consider_illegal);

extern void find_closest_blockdim_3d(
    float target_bitrate,
    int *x,
    int *y,
    int *z,
    int consider_illegal);

extern void destroy_image(astc_codec_image* img);

extern astc_codec_image* allocate_image(int bitness, int xsize, int ysize, int zsize, int padding);

extern void test_inappropriate_extended_precision();
extern void prepare_angular_tables();
extern void build_quantization_mode_table();

extern "C" {
    sf16 float_to_sf16(float, roundmode);
}
