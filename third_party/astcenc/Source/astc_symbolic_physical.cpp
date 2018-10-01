/*----------------------------------------------------------------------------*/
/**
 *	This confidential and proprietary software may be used only as
 *	authorised by a licensing agreement from ARM Limited
 *	(C) COPYRIGHT 2011-2012 ARM Limited
 *	ALL RIGHTS RESERVED
 *
 *	The entire notice above must be reproduced on all authorised
 *	copies and copies may only be made to the extent permitted
 *	by a licensing agreement from ARM Limited.
 *
 *	@brief	Functions to convert a compressed block between the symbolic and
 *			the physical representation.
 */
/*----------------------------------------------------------------------------*/

#include "astc_codec_internals.h"

// routine to write up to 8 bits
static inline void write_bits(int value, int bitcount, int bitoffset, uint8_t * ptr)
{
	int mask = (1 << bitcount) - 1;
	value &= mask;
	ptr += bitoffset >> 3;
	bitoffset &= 7;
	value <<= bitoffset;
	mask <<= bitoffset;
	mask = ~mask;

	ptr[0] &= mask;
	ptr[0] |= value;
	ptr[1] &= mask >> 8;
	ptr[1] |= value >> 8;
}


// routine to read up to 8 bits
static inline int read_bits(int bitcount, int bitoffset, const uint8_t * ptr)
{
	int mask = (1 << bitcount) - 1;
	ptr += bitoffset >> 3;
	bitoffset &= 7;
	int value = ptr[0] | (ptr[1] << 8);
	value >>= bitoffset;
	value &= mask;
	return value;
}


int bitrev8(int p)
{
	p = ((p & 0xF) << 4) | ((p >> 4) & 0xF);
	p = ((p & 0x33) << 2) | ((p >> 2) & 0x33);
	p = ((p & 0x55) << 1) | ((p >> 1) & 0x55);
	return p;
}




physical_compressed_block symbolic_to_physical(int xdim, int ydim, int zdim, const symbolic_compressed_block * sc)
{
	int i, j;
	physical_compressed_block res;


	if (sc->block_mode == -2)
	{
		// UNORM16 constant-color block.
		// This encodes separate constant-color blocks. There is currently
		// no attempt to coalesce them into larger void-extents.

		static const uint8_t cbytes[8] = { 0xFC, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
		for (i = 0; i < 8; i++)
			res.data[i] = cbytes[i];

		for (i = 0; i < 4; i++)
		{
			res.data[2 * i + 8] = sc->constant_color[i] & 0xFF;
			res.data[2 * i + 9] = (sc->constant_color[i] >> 8) & 0xFF;
		}
		return res;
	}


	if (sc->block_mode == -1)
	{
		// FP16 constant-color block.
		// This encodes separate constant-color blocks. There is currently
		// no attempt to coalesce them into larger void-extents.

		static const uint8_t cbytes[8] = { 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
		for (i = 0; i < 8; i++)
			res.data[i] = cbytes[i];

		for (i = 0; i < 4; i++)
		{
			res.data[2 * i + 8] = sc->constant_color[i] & 0xFF;
			res.data[2 * i + 9] = (sc->constant_color[i] >> 8) & 0xFF;
		}
		return res;
	}



	int partition_count = sc->partition_count;

	// first, compress the weights. They are encoded as an ordinary
	// integer-sequence, then bit-reversed
	uint8_t weightbuf[16];
	for (i = 0; i < 16; i++)
		weightbuf[i] = 0;

	const block_size_descriptor *bsd = get_block_size_descriptor(xdim, ydim, zdim);
	const decimation_table *const *ixtab2 = bsd->decimation_tables;


	int weight_count = ixtab2[bsd->block_modes[sc->block_mode].decimation_mode]->num_weights;
	int weight_quantization_method = bsd->block_modes[sc->block_mode].quantization_mode;
	int is_dual_plane = bsd->block_modes[sc->block_mode].is_dual_plane;

	int real_weight_count = is_dual_plane ? 2 * weight_count : weight_count;

	int bits_for_weights = compute_ise_bitcount(real_weight_count,
												(quantization_method) weight_quantization_method);


	if (is_dual_plane)
	{
		uint8_t weights[64];
		for (i = 0; i < weight_count; i++)
		{
			weights[2 * i] = sc->plane1_weights[i];
			weights[2 * i + 1] = sc->plane2_weights[i];
		}
		encode_ise(weight_quantization_method, real_weight_count, weights, weightbuf, 0);
	}
	else
	{
		encode_ise(weight_quantization_method, weight_count, sc->plane1_weights, weightbuf, 0);
	}

	for (i = 0; i < 16; i++)
		res.data[i] = bitrev8(weightbuf[15 - i]);

	write_bits(sc->block_mode, 11, 0, res.data);
	write_bits(partition_count - 1, 2, 11, res.data);

	int below_weights_pos = 128 - bits_for_weights;

	// encode partition index and color endpoint types for blocks with
	// 2 or more partitions.
	if (partition_count > 1)
	{
		write_bits(sc->partition_index, 6, 13, res.data);
		write_bits(sc->partition_index >> 6, PARTITION_BITS - 6, 19, res.data);

		if (sc->color_formats_matched)
		{
			write_bits(sc->color_formats[0] << 2, 6, 13 + PARTITION_BITS, res.data);
		}
		else
		{
			// go through the selected endpoint type classes for each partition
			// in order to determine the lowest class present.
			int low_class = 4;
			for (i = 0; i < partition_count; i++)
			{
				int class_of_format = sc->color_formats[i] >> 2;
				if (class_of_format < low_class)
					low_class = class_of_format;
			}
			if (low_class == 3)
				low_class = 2;
			int encoded_type = low_class + 1;
			int bitpos = 2;
			for (i = 0; i < partition_count; i++)
			{
				int classbit_of_format = (sc->color_formats[i] >> 2) - low_class;

				encoded_type |= classbit_of_format << bitpos;
				bitpos++;
			}
			for (i = 0; i < partition_count; i++)
			{
				int lowbits_of_format = sc->color_formats[i] & 3;
				encoded_type |= lowbits_of_format << bitpos;
				bitpos += 2;
			}
			int encoded_type_lowpart = encoded_type & 0x3F;
			int encoded_type_highpart = encoded_type >> 6;
			int encoded_type_highpart_size = (3 * partition_count) - 4;
			int encoded_type_highpart_pos = 128 - bits_for_weights - encoded_type_highpart_size;
			write_bits(encoded_type_lowpart, 6, 13 + PARTITION_BITS, res.data);
			write_bits(encoded_type_highpart, encoded_type_highpart_size, encoded_type_highpart_pos, res.data);

			below_weights_pos -= encoded_type_highpart_size;
		}
	}

	else
		write_bits(sc->color_formats[0], 4, 13, res.data);

	// in dual-plane mode, encode the color component of the second plane of weights
	if (is_dual_plane)
		write_bits(sc->plane2_color_component, 2, below_weights_pos - 2, res.data);

	// finally, encode the color bits
	// first, get hold of all the color components to encode
	uint8_t values_to_encode[32];
	int valuecount_to_encode = 0;
	for (i = 0; i < sc->partition_count; i++)
	{
		int vals = 2 * (sc->color_formats[i] >> 2) + 2;
		for (j = 0; j < vals; j++)
			values_to_encode[j + valuecount_to_encode] = sc->color_values[i][j];
		valuecount_to_encode += vals;
	}
	// then, encode an ISE based on them.
	encode_ise(sc->color_quantization_level, valuecount_to_encode, values_to_encode, res.data, (sc->partition_count == 1 ? 17 : 19 + PARTITION_BITS));

	return res;
}


void physical_to_symbolic(int xdim, int ydim, int zdim, physical_compressed_block pb, symbolic_compressed_block * res)
{
	uint8_t bswapped[16];
	int i, j;

	res->error_block = 0;

	// get hold of the block-size descriptor and the decimation tables.
	const block_size_descriptor *bsd = get_block_size_descriptor(xdim, ydim, zdim);
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
		if (zdim == 1)
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
