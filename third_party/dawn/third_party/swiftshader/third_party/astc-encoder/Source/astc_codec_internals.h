// SPDX-License-Identifier: Apache-2.0
// ----------------------------------------------------------------------------
// Copyright 2011-2020 Arm Limited
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy
// of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
// ----------------------------------------------------------------------------

/**
 * @brief Functions and data declarations.
 */

#ifndef ASTC_CODEC_INTERNALS_INCLUDED
#define ASTC_CODEC_INTERNALS_INCLUDED

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "astc_mathlib.h"

// ASTC parameters
#define MAX_TEXELS_PER_BLOCK 216
#define MAX_WEIGHTS_PER_BLOCK 64
#define MIN_WEIGHT_BITS_PER_BLOCK 24
#define MAX_WEIGHT_BITS_PER_BLOCK 96
#define PARTITION_BITS 10
#define PARTITION_COUNT (1 << PARTITION_BITS)

// the sum of weights for one texel.
#define TEXEL_WEIGHT_SUM 16
#define MAX_DECIMATION_MODES 87
#define MAX_WEIGHT_MODES 2048

enum astc_decode_mode
{
	DECODE_LDR_SRGB,
	DECODE_LDR,
	DECODE_HDR
};

/*
	Partition table representation:
	For each block size, we have 3 tables, each with 1024 partitionings;
	these three tables correspond to 2, 3 and 4 partitions respectively.
	For each partitioning, we have:
	* a 4-entry table indicating how many texels there are in each of the 4 partitions.
	  This may be from 0 to a very large value.
	* a table indicating the partition index of each of the texels in the block.
	  Each index may be 0, 1, 2 or 3.
	* Each element in the table is an uint8_t indicating partition index (0, 1, 2 or 3)
*/

struct partition_info
{
	int partition_count;
	uint8_t partition_of_texel[MAX_TEXELS_PER_BLOCK];
};

/*
   In ASTC, we don't necessarily provide a weight for every texel.
   As such, for each block size, there are a number of patterns where some texels
   have their weights computed as a weighted average of more than 1 weight.
   As such, the codec uses a data structure that tells us: for each texel, which
   weights it is a combination of for each weight, which texels it contributes to.
   The decimation_table is this data structure.
*/
struct decimation_table
{
	int num_weights;
	uint8_t texel_num_weights[MAX_TEXELS_PER_BLOCK];	// number of indices that go into the calculation for a texel
	uint8_t texel_weights_int[MAX_TEXELS_PER_BLOCK][4];	// the weight to assign to each weight
	uint8_t texel_weights[MAX_TEXELS_PER_BLOCK][4];	// the weights that go into a texel calculation
};

/*
   data structure describing information that pertains to a block size and its associated block modes.
*/
struct block_mode
{
	int8_t decimation_mode;
	int8_t quantization_mode;
	int8_t is_dual_plane;
	int8_t permit_decode;
};

struct block_size_descriptor
{
	int xdim;
	int ydim;
	int zdim;
	int texel_count;

	int decimation_mode_count;
	const decimation_table *decimation_tables[MAX_DECIMATION_MODES];
	block_mode block_modes[MAX_WEIGHT_MODES];

	// All the partitioning information for this block size
	partition_info partitions[(3*PARTITION_COUNT)+1];
};

// data structure representing one block of an image.
// it is expanded to float prior to processing to save some computation time
// on conversions to/from uint8_t (this also allows us to handle HDR textures easily)
struct imageblock
{
	float orig_data[MAX_TEXELS_PER_BLOCK * 4];  // original input data
	float data_r[MAX_TEXELS_PER_BLOCK];  // the data that we will compress, either linear or LNS (0..65535 in both cases)
	float data_g[MAX_TEXELS_PER_BLOCK];
	float data_b[MAX_TEXELS_PER_BLOCK];
	float data_a[MAX_TEXELS_PER_BLOCK];

	uint8_t rgb_lns[MAX_TEXELS_PER_BLOCK];      // 1 if RGB data are being treated as LNS
	uint8_t alpha_lns[MAX_TEXELS_PER_BLOCK];    // 1 if Alpha data are being treated as LNS
	uint8_t nan_texel[MAX_TEXELS_PER_BLOCK];    // 1 if the texel is a NaN-texel.

	float red_min, red_max;
	float green_min, green_max;
	float blue_min, blue_max;
	float alpha_min, alpha_max;
	int grayscale;				// 1 if R=G=B for every pixel, 0 otherwise

	int xpos, ypos, zpos;
};

void update_imageblock_flags(
	imageblock* pb,
	int xdim,
	int ydim,
	int zdim);

void imageblock_initialize_orig_from_work(
	imageblock * pb,
	int pixelcount);

void imageblock_initialize_work_from_orig(
	imageblock * pb,
	int pixelcount);

// enumeration of all the quantization methods we support under this format.
enum quantization_method
{
	QUANT_2 = 0,
	QUANT_3 = 1,
	QUANT_4 = 2,
	QUANT_5 = 3,
	QUANT_6 = 4,
	QUANT_8 = 5,
	QUANT_10 = 6,
	QUANT_12 = 7,
	QUANT_16 = 8,
	QUANT_20 = 9,
	QUANT_24 = 10,
	QUANT_32 = 11,
	QUANT_40 = 12,
	QUANT_48 = 13,
	QUANT_64 = 14,
	QUANT_80 = 15,
	QUANT_96 = 16,
	QUANT_128 = 17,
	QUANT_160 = 18,
	QUANT_192 = 19,
	QUANT_256 = 20
};

/**
 * @brief Weight quantization transfer table.
 *
 * ASTC can store texel weights at many quantization levels, so for performance
 * we store essential information about each level as a precomputed data
 * structure.
 *
 * Unquantized weights are integers in the range [0, 64], or floats [0, 1].
 *
 * This structure provides the following information:
 * A table, used to estimate the closest quantized
	weight for a given floating-point weight. For each quantized weight, the corresponding unquantized
	and floating-point values. For each quantized weight, a previous-value and a next-value.
*/
struct quantization_and_transfer_table
{
	/** The scrambled unquantized values. */
	uint8_t unquantized_value[32];
};

extern const quantization_and_transfer_table quant_and_xfer_tables[12];

enum endpoint_formats
{
	FMT_LUMINANCE = 0,
	FMT_LUMINANCE_DELTA = 1,
	FMT_HDR_LUMINANCE_LARGE_RANGE = 2,
	FMT_HDR_LUMINANCE_SMALL_RANGE = 3,
	FMT_LUMINANCE_ALPHA = 4,
	FMT_LUMINANCE_ALPHA_DELTA = 5,
	FMT_RGB_SCALE = 6,
	FMT_HDR_RGB_SCALE = 7,
	FMT_RGB = 8,
	FMT_RGB_DELTA = 9,
	FMT_RGB_SCALE_ALPHA = 10,
	FMT_HDR_RGB = 11,
	FMT_RGBA = 12,
	FMT_RGBA_DELTA = 13,
	FMT_HDR_RGB_LDR_ALPHA = 14,
	FMT_HDR_RGBA = 15,
};

struct symbolic_compressed_block
{
	int error_block;			// 1 marks error block, 0 marks non-error-block.
	int block_mode;				// 0 to 2047. Negative value marks constant-color block (-1: FP16, -2:UINT16)
	int partition_count;		// 1 to 4; Zero marks a constant-color block.
	int partition_index;		// 0 to 1023
	int color_formats[4];		// color format for each endpoint color pair.
	int color_formats_matched;	// color format for all endpoint pairs are matched.
	int color_values[4][12];	// quantized endpoint color pairs.
	int color_quantization_level;
	uint8_t plane1_weights[MAX_WEIGHTS_PER_BLOCK];	// quantized and decimated weights
	uint8_t plane2_weights[MAX_WEIGHTS_PER_BLOCK];
	int plane2_color_component;	// color component for the secondary plane of weights
	int constant_color[4];		// constant-color, as FP16 or UINT16. Used for constant-color blocks only.
};

struct physical_compressed_block
{
	uint8_t data[16];
};

/* ============================================================================
  Functions and data pertaining to quantization and encoding
============================================================================ */

/**
 * @brief Populate the blocksize descriptor for the target block size.
 *
 * This will also initialize the partition table metadata, which is stored
 * as part of the BSD structure.
 *
 * @param xdim The x axis size of the block.
 * @param ydim The y axis size of the block.
 * @param zdim The z axis size of the block.
 * @param bsd  The structure to populate.
 */
void init_block_size_descriptor(
	int xdim,
	int ydim,
	int zdim,
	block_size_descriptor* bsd);

void term_block_size_descriptor(
	block_size_descriptor* bsd);

/**
 * @brief Populate the partition tables for the target block size.
 *
 * Note the block_size_size descriptor must be initialized before calling this
 * function.
 *
 * @param bsd  The structure to populate.
 */
void init_partition_tables(
	block_size_descriptor* bsd);

static inline const partition_info *get_partition_table(
	const block_size_descriptor* bsd,
	int partition_count
) {
	if (partition_count == 1) {
		partition_count = 5;
	}
	int index = (partition_count - 2) * PARTITION_COUNT;
	return bsd->partitions + index;
}

// ***********************************************************
// functions and data pertaining to quantization and encoding
// **********************************************************

extern const uint8_t color_unquantization_tables[21][256];
extern int quantization_mode_table[17][128];

void decode_ise(
	int quantization_level,
	int elements,
	const uint8_t* input_data,
	uint8_t* output_data,
	int bit_offset);

int compute_ise_bitcount(
	int items,
	quantization_method quant);

void build_quantization_mode_table(void);

// unpack a pair of color endpoints from a series of integers.
void unpack_color_endpoints(
	astc_decode_mode decode_mode,
	int format,
	int quantization_level,
	const int* input,
	int* rgb_hdr,
	int* alpha_hdr,
	int* nan_endpoint,
	uint4* output0,
	uint4* output1);

/* *********************************** high-level encode and decode functions ************************************ */

void decompress_symbolic_block(
	astc_decode_mode decode_mode,
	const block_size_descriptor* bsd,
	int xpos,
	int ypos,
	int zpos,
	const symbolic_compressed_block* scb,
	imageblock* blk);

void physical_to_symbolic(
	const block_size_descriptor* bsd,
	physical_compressed_block pb,
	symbolic_compressed_block* res);

uint16_t unorm16_to_sf16(
	uint16_t p);

uint16_t lns_to_sf16(
	uint16_t p);

#endif
