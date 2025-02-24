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
 * @brief Functions for converting between symbolic and physical encodings.
 */

#include "astc_codec_internals.h"

// routine to read up to 8 bits
static inline int read_bits(
	int bitcount,
	int bitoffset,
	const uint8_t* ptr
) {
	int mask = (1 << bitcount) - 1;
	ptr += bitoffset >> 3;
	bitoffset &= 7;
	int value = ptr[0] | (ptr[1] << 8);
	value >>= bitoffset;
	value &= mask;
	return value;
}

static inline int bitrev8(int p)
{
	p = ((p & 0xF) << 4) | ((p >> 4) & 0xF);
	p = ((p & 0x33) << 2) | ((p >> 2) & 0x33);
	p = ((p & 0x55) << 1) | ((p >> 1) & 0x55);
	return p;
}

void physical_to_symbolic(
	const block_size_descriptor* bsd,
	physical_compressed_block pb,
	symbolic_compressed_block* res
) {
	uint8_t bswapped[16];
	int i, j;

	res->error_block = 0;

	// get hold of the decimation tables.
	const decimation_table *const *ixtab2 = bsd->decimation_tables;

	// extract header fields
	int block_mode = read_bits(11, 0, pb.data);
	if ((block_mode & 0x1FF) == 0x1FC)
	{
		// void-extent block!

		// check what format the data has
		if (block_mode & 0x200)
			res->block_mode = -1;	// floating-point
		else
			res->block_mode = -2;	// unorm16.

		res->partition_count = 0;
		for (i = 0; i < 4; i++)
		{
			res->constant_color[i] = pb.data[2 * i + 8] | (pb.data[2 * i + 9] << 8);
		}

		// additionally, check that the void-extent
		if (bsd->zdim == 1)
		{
			// 2D void-extent
			int rsvbits = read_bits(2, 10, pb.data);
			if (rsvbits != 3)
				res->error_block = 1;

			int vx_low_s = read_bits(8, 12, pb.data) | (read_bits(5, 12 + 8, pb.data) << 8);
			int vx_high_s = read_bits(8, 25, pb.data) | (read_bits(5, 25 + 8, pb.data) << 8);
			int vx_low_t = read_bits(8, 38, pb.data) | (read_bits(5, 38 + 8, pb.data) << 8);
			int vx_high_t = read_bits(8, 51, pb.data) | (read_bits(5, 51 + 8, pb.data) << 8);

			int all_ones = vx_low_s == 0x1FFF && vx_high_s == 0x1FFF && vx_low_t == 0x1FFF && vx_high_t == 0x1FFF;

			if ((vx_low_s >= vx_high_s || vx_low_t >= vx_high_t) && !all_ones)
				res->error_block = 1;
		}
		else
		{
			// 3D void-extent
			int vx_low_s = read_bits(9, 10, pb.data);
			int vx_high_s = read_bits(9, 19, pb.data);
			int vx_low_t = read_bits(9, 28, pb.data);
			int vx_high_t = read_bits(9, 37, pb.data);
			int vx_low_p = read_bits(9, 46, pb.data);
			int vx_high_p = read_bits(9, 55, pb.data);

			int all_ones = vx_low_s == 0x1FF && vx_high_s == 0x1FF && vx_low_t == 0x1FF && vx_high_t == 0x1FF && vx_low_p == 0x1FF && vx_high_p == 0x1FF;

			if ((vx_low_s >= vx_high_s || vx_low_t >= vx_high_t || vx_low_p >= vx_high_p) && !all_ones)
				res->error_block = 1;
		}

		return;
	}

	if (bsd->block_modes[block_mode].permit_decode == 0)
	{
		res->error_block = 1;
		return;
	}

	int weight_count = ixtab2[bsd->block_modes[block_mode].decimation_mode]->num_weights;
	int weight_quantization_method = bsd->block_modes[block_mode].quantization_mode;
	int is_dual_plane = bsd->block_modes[block_mode].is_dual_plane;

	int real_weight_count = is_dual_plane ? 2 * weight_count : weight_count;

	int partition_count = read_bits(2, 11, pb.data) + 1;

	res->block_mode = block_mode;
	res->partition_count = partition_count;

	for (i = 0; i < 16; i++)
		bswapped[i] = bitrev8(pb.data[15 - i]);

	int bits_for_weights = compute_ise_bitcount(real_weight_count,
												(quantization_method) weight_quantization_method);

	int below_weights_pos = 128 - bits_for_weights;

	if (is_dual_plane)
	{
		uint8_t indices[64];
		decode_ise(weight_quantization_method, real_weight_count, bswapped, indices, 0);
		for (i = 0; i < weight_count; i++)
		{
			res->plane1_weights[i] = indices[2 * i];
			res->plane2_weights[i] = indices[2 * i + 1];
		}
	}
	else
	{
		decode_ise(weight_quantization_method, weight_count, bswapped, res->plane1_weights, 0);
	}

	if (is_dual_plane && partition_count == 4)
		res->error_block = 1;

	res->color_formats_matched = 0;

	// then, determine the format of each endpoint pair
	int color_formats[4];
	int encoded_type_highpart_size = 0;
	if (partition_count == 1)
	{
		color_formats[0] = read_bits(4, 13, pb.data);
		res->partition_index = 0;
	}
	else
	{
		encoded_type_highpart_size = (3 * partition_count) - 4;
		below_weights_pos -= encoded_type_highpart_size;
		int encoded_type = read_bits(6, 13 + PARTITION_BITS, pb.data) | (read_bits(encoded_type_highpart_size, below_weights_pos, pb.data) << 6);
		int baseclass = encoded_type & 0x3;
		if (baseclass == 0)
		{
			for (i = 0; i < partition_count; i++)
			{
				color_formats[i] = (encoded_type >> 2) & 0xF;
			}
			below_weights_pos += encoded_type_highpart_size;
			res->color_formats_matched = 1;
			encoded_type_highpart_size = 0;
		}
		else
		{
			int bitpos = 2;
			baseclass--;
			for (i = 0; i < partition_count; i++)
			{
				color_formats[i] = (((encoded_type >> bitpos) & 1) + baseclass) << 2;
				bitpos++;
			}
			for (i = 0; i < partition_count; i++)
			{
				color_formats[i] |= (encoded_type >> bitpos) & 3;
				bitpos += 2;
			}
		}
		res->partition_index = read_bits(6, 13, pb.data) | (read_bits(PARTITION_BITS - 6, 19, pb.data) << 6);
	}

	for (i = 0; i < partition_count; i++)
		res->color_formats[i] = color_formats[i];

	// then, determine the number of integers we need to unpack for the endpoint pairs
	int color_integer_count = 0;
	for (i = 0; i < partition_count; i++)
	{
		int endpoint_class = color_formats[i] >> 2;
		color_integer_count += (endpoint_class + 1) * 2;
	}

	if (color_integer_count > 18)
		res->error_block = 1;

	// then, determine the color endpoint format to use for these integers
	static const int color_bits_arr[5] = { -1, 115 - 4, 113 - 4 - PARTITION_BITS, 113 - 4 - PARTITION_BITS, 113 - 4 - PARTITION_BITS };
	int color_bits = color_bits_arr[partition_count] - bits_for_weights - encoded_type_highpart_size;
	if (is_dual_plane)
		color_bits -= 2;
	if (color_bits < 0)
		color_bits = 0;

	int color_quantization_level = quantization_mode_table[color_integer_count >> 1][color_bits];
	res->color_quantization_level = color_quantization_level;
	if (color_quantization_level < 4)
		res->error_block = 1;

	// then unpack the integer-bits
	uint8_t values_to_decode[32];
	decode_ise(color_quantization_level, color_integer_count, pb.data, values_to_decode, (partition_count == 1 ? 17 : 19 + PARTITION_BITS));

	// and distribute them over the endpoint types
	int valuecount_to_decode = 0;

	for (i = 0; i < partition_count; i++)
	{
		int vals = 2 * (color_formats[i] >> 2) + 2;
		for (j = 0; j < vals; j++)
			res->color_values[i][j] = values_to_decode[j + valuecount_to_decode];
		valuecount_to_decode += vals;
	}

	// get hold of color component for second-plane in the case of dual plane of weights.
	if (is_dual_plane)
		res->plane2_color_component = read_bits(2, below_weights_pos - 2, pb.data);
}
