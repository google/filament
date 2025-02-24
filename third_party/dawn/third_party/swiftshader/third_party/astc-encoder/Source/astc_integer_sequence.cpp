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
 * @brief Functions for encoding/decoding Bounded Integer Sequence Encoding.
 */

#include "astc_codec_internals.h"

// unpacked quint triplets <low,middle,high> for each packed-quint value
static const uint8_t quints_of_integer[128][3] = {
	{0, 0, 0},	{1, 0, 0},	{2, 0, 0},	{3, 0, 0},
	{4, 0, 0},	{0, 4, 0},	{4, 4, 0},	{4, 4, 4},
	{0, 1, 0},	{1, 1, 0},	{2, 1, 0},	{3, 1, 0},
	{4, 1, 0},	{1, 4, 0},	{4, 4, 1},	{4, 4, 4},
	{0, 2, 0},	{1, 2, 0},	{2, 2, 0},	{3, 2, 0},
	{4, 2, 0},	{2, 4, 0},	{4, 4, 2},	{4, 4, 4},
	{0, 3, 0},	{1, 3, 0},	{2, 3, 0},	{3, 3, 0},
	{4, 3, 0},	{3, 4, 0},	{4, 4, 3},	{4, 4, 4},
	{0, 0, 1},	{1, 0, 1},	{2, 0, 1},	{3, 0, 1},
	{4, 0, 1},	{0, 4, 1},	{4, 0, 4},	{0, 4, 4},
	{0, 1, 1},	{1, 1, 1},	{2, 1, 1},	{3, 1, 1},
	{4, 1, 1},	{1, 4, 1},	{4, 1, 4},	{1, 4, 4},
	{0, 2, 1},	{1, 2, 1},	{2, 2, 1},	{3, 2, 1},
	{4, 2, 1},	{2, 4, 1},	{4, 2, 4},	{2, 4, 4},
	{0, 3, 1},	{1, 3, 1},	{2, 3, 1},	{3, 3, 1},
	{4, 3, 1},	{3, 4, 1},	{4, 3, 4},	{3, 4, 4},
	{0, 0, 2},	{1, 0, 2},	{2, 0, 2},	{3, 0, 2},
	{4, 0, 2},	{0, 4, 2},	{2, 0, 4},	{3, 0, 4},
	{0, 1, 2},	{1, 1, 2},	{2, 1, 2},	{3, 1, 2},
	{4, 1, 2},	{1, 4, 2},	{2, 1, 4},	{3, 1, 4},
	{0, 2, 2},	{1, 2, 2},	{2, 2, 2},	{3, 2, 2},
	{4, 2, 2},	{2, 4, 2},	{2, 2, 4},	{3, 2, 4},
	{0, 3, 2},	{1, 3, 2},	{2, 3, 2},	{3, 3, 2},
	{4, 3, 2},	{3, 4, 2},	{2, 3, 4},	{3, 3, 4},
	{0, 0, 3},	{1, 0, 3},	{2, 0, 3},	{3, 0, 3},
	{4, 0, 3},	{0, 4, 3},	{0, 0, 4},	{1, 0, 4},
	{0, 1, 3},	{1, 1, 3},	{2, 1, 3},	{3, 1, 3},
	{4, 1, 3},	{1, 4, 3},	{0, 1, 4},	{1, 1, 4},
	{0, 2, 3},	{1, 2, 3},	{2, 2, 3},	{3, 2, 3},
	{4, 2, 3},	{2, 4, 3},	{0, 2, 4},	{1, 2, 4},
	{0, 3, 3},	{1, 3, 3},	{2, 3, 3},	{3, 3, 3},
	{4, 3, 3},	{3, 4, 3},	{0, 3, 4},	{1, 3, 4}
};

// unpacked trit quintuplets <low,_,_,_,high> for each packed-quint value
static const uint8_t trits_of_integer[256][5] = {
	{0, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {2, 0, 0, 0, 0}, {0, 0, 2, 0, 0},
	{0, 1, 0, 0, 0}, {1, 1, 0, 0, 0}, {2, 1, 0, 0, 0}, {1, 0, 2, 0, 0},
	{0, 2, 0, 0, 0}, {1, 2, 0, 0, 0}, {2, 2, 0, 0, 0}, {2, 0, 2, 0, 0},
	{0, 2, 2, 0, 0}, {1, 2, 2, 0, 0}, {2, 2, 2, 0, 0}, {2, 0, 2, 0, 0},
	{0, 0, 1, 0, 0}, {1, 0, 1, 0, 0}, {2, 0, 1, 0, 0}, {0, 1, 2, 0, 0},
	{0, 1, 1, 0, 0}, {1, 1, 1, 0, 0}, {2, 1, 1, 0, 0}, {1, 1, 2, 0, 0},
	{0, 2, 1, 0, 0}, {1, 2, 1, 0, 0}, {2, 2, 1, 0, 0}, {2, 1, 2, 0, 0},
	{0, 0, 0, 2, 2}, {1, 0, 0, 2, 2}, {2, 0, 0, 2, 2}, {0, 0, 2, 2, 2},
	{0, 0, 0, 1, 0}, {1, 0, 0, 1, 0}, {2, 0, 0, 1, 0}, {0, 0, 2, 1, 0},
	{0, 1, 0, 1, 0}, {1, 1, 0, 1, 0}, {2, 1, 0, 1, 0}, {1, 0, 2, 1, 0},
	{0, 2, 0, 1, 0}, {1, 2, 0, 1, 0}, {2, 2, 0, 1, 0}, {2, 0, 2, 1, 0},
	{0, 2, 2, 1, 0}, {1, 2, 2, 1, 0}, {2, 2, 2, 1, 0}, {2, 0, 2, 1, 0},
	{0, 0, 1, 1, 0}, {1, 0, 1, 1, 0}, {2, 0, 1, 1, 0}, {0, 1, 2, 1, 0},
	{0, 1, 1, 1, 0}, {1, 1, 1, 1, 0}, {2, 1, 1, 1, 0}, {1, 1, 2, 1, 0},
	{0, 2, 1, 1, 0}, {1, 2, 1, 1, 0}, {2, 2, 1, 1, 0}, {2, 1, 2, 1, 0},
	{0, 1, 0, 2, 2}, {1, 1, 0, 2, 2}, {2, 1, 0, 2, 2}, {1, 0, 2, 2, 2},
	{0, 0, 0, 2, 0}, {1, 0, 0, 2, 0}, {2, 0, 0, 2, 0}, {0, 0, 2, 2, 0},
	{0, 1, 0, 2, 0}, {1, 1, 0, 2, 0}, {2, 1, 0, 2, 0}, {1, 0, 2, 2, 0},
	{0, 2, 0, 2, 0}, {1, 2, 0, 2, 0}, {2, 2, 0, 2, 0}, {2, 0, 2, 2, 0},
	{0, 2, 2, 2, 0}, {1, 2, 2, 2, 0}, {2, 2, 2, 2, 0}, {2, 0, 2, 2, 0},
	{0, 0, 1, 2, 0}, {1, 0, 1, 2, 0}, {2, 0, 1, 2, 0}, {0, 1, 2, 2, 0},
	{0, 1, 1, 2, 0}, {1, 1, 1, 2, 0}, {2, 1, 1, 2, 0}, {1, 1, 2, 2, 0},
	{0, 2, 1, 2, 0}, {1, 2, 1, 2, 0}, {2, 2, 1, 2, 0}, {2, 1, 2, 2, 0},
	{0, 2, 0, 2, 2}, {1, 2, 0, 2, 2}, {2, 2, 0, 2, 2}, {2, 0, 2, 2, 2},
	{0, 0, 0, 0, 2}, {1, 0, 0, 0, 2}, {2, 0, 0, 0, 2}, {0, 0, 2, 0, 2},
	{0, 1, 0, 0, 2}, {1, 1, 0, 0, 2}, {2, 1, 0, 0, 2}, {1, 0, 2, 0, 2},
	{0, 2, 0, 0, 2}, {1, 2, 0, 0, 2}, {2, 2, 0, 0, 2}, {2, 0, 2, 0, 2},
	{0, 2, 2, 0, 2}, {1, 2, 2, 0, 2}, {2, 2, 2, 0, 2}, {2, 0, 2, 0, 2},
	{0, 0, 1, 0, 2}, {1, 0, 1, 0, 2}, {2, 0, 1, 0, 2}, {0, 1, 2, 0, 2},
	{0, 1, 1, 0, 2}, {1, 1, 1, 0, 2}, {2, 1, 1, 0, 2}, {1, 1, 2, 0, 2},
	{0, 2, 1, 0, 2}, {1, 2, 1, 0, 2}, {2, 2, 1, 0, 2}, {2, 1, 2, 0, 2},
	{0, 2, 2, 2, 2}, {1, 2, 2, 2, 2}, {2, 2, 2, 2, 2}, {2, 0, 2, 2, 2},
	{0, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {2, 0, 0, 0, 1}, {0, 0, 2, 0, 1},
	{0, 1, 0, 0, 1}, {1, 1, 0, 0, 1}, {2, 1, 0, 0, 1}, {1, 0, 2, 0, 1},
	{0, 2, 0, 0, 1}, {1, 2, 0, 0, 1}, {2, 2, 0, 0, 1}, {2, 0, 2, 0, 1},
	{0, 2, 2, 0, 1}, {1, 2, 2, 0, 1}, {2, 2, 2, 0, 1}, {2, 0, 2, 0, 1},
	{0, 0, 1, 0, 1}, {1, 0, 1, 0, 1}, {2, 0, 1, 0, 1}, {0, 1, 2, 0, 1},
	{0, 1, 1, 0, 1}, {1, 1, 1, 0, 1}, {2, 1, 1, 0, 1}, {1, 1, 2, 0, 1},
	{0, 2, 1, 0, 1}, {1, 2, 1, 0, 1}, {2, 2, 1, 0, 1}, {2, 1, 2, 0, 1},
	{0, 0, 1, 2, 2}, {1, 0, 1, 2, 2}, {2, 0, 1, 2, 2}, {0, 1, 2, 2, 2},
	{0, 0, 0, 1, 1}, {1, 0, 0, 1, 1}, {2, 0, 0, 1, 1}, {0, 0, 2, 1, 1},
	{0, 1, 0, 1, 1}, {1, 1, 0, 1, 1}, {2, 1, 0, 1, 1}, {1, 0, 2, 1, 1},
	{0, 2, 0, 1, 1}, {1, 2, 0, 1, 1}, {2, 2, 0, 1, 1}, {2, 0, 2, 1, 1},
	{0, 2, 2, 1, 1}, {1, 2, 2, 1, 1}, {2, 2, 2, 1, 1}, {2, 0, 2, 1, 1},
	{0, 0, 1, 1, 1}, {1, 0, 1, 1, 1}, {2, 0, 1, 1, 1}, {0, 1, 2, 1, 1},
	{0, 1, 1, 1, 1}, {1, 1, 1, 1, 1}, {2, 1, 1, 1, 1}, {1, 1, 2, 1, 1},
	{0, 2, 1, 1, 1}, {1, 2, 1, 1, 1}, {2, 2, 1, 1, 1}, {2, 1, 2, 1, 1},
	{0, 1, 1, 2, 2}, {1, 1, 1, 2, 2}, {2, 1, 1, 2, 2}, {1, 1, 2, 2, 2},
	{0, 0, 0, 2, 1}, {1, 0, 0, 2, 1}, {2, 0, 0, 2, 1}, {0, 0, 2, 2, 1},
	{0, 1, 0, 2, 1}, {1, 1, 0, 2, 1}, {2, 1, 0, 2, 1}, {1, 0, 2, 2, 1},
	{0, 2, 0, 2, 1}, {1, 2, 0, 2, 1}, {2, 2, 0, 2, 1}, {2, 0, 2, 2, 1},
	{0, 2, 2, 2, 1}, {1, 2, 2, 2, 1}, {2, 2, 2, 2, 1}, {2, 0, 2, 2, 1},
	{0, 0, 1, 2, 1}, {1, 0, 1, 2, 1}, {2, 0, 1, 2, 1}, {0, 1, 2, 2, 1},
	{0, 1, 1, 2, 1}, {1, 1, 1, 2, 1}, {2, 1, 1, 2, 1}, {1, 1, 2, 2, 1},
	{0, 2, 1, 2, 1}, {1, 2, 1, 2, 1}, {2, 2, 1, 2, 1}, {2, 1, 2, 2, 1},
	{0, 2, 1, 2, 2}, {1, 2, 1, 2, 2}, {2, 2, 1, 2, 2}, {2, 1, 2, 2, 2},
	{0, 0, 0, 1, 2}, {1, 0, 0, 1, 2}, {2, 0, 0, 1, 2}, {0, 0, 2, 1, 2},
	{0, 1, 0, 1, 2}, {1, 1, 0, 1, 2}, {2, 1, 0, 1, 2}, {1, 0, 2, 1, 2},
	{0, 2, 0, 1, 2}, {1, 2, 0, 1, 2}, {2, 2, 0, 1, 2}, {2, 0, 2, 1, 2},
	{0, 2, 2, 1, 2}, {1, 2, 2, 1, 2}, {2, 2, 2, 1, 2}, {2, 0, 2, 1, 2},
	{0, 0, 1, 1, 2}, {1, 0, 1, 1, 2}, {2, 0, 1, 1, 2}, {0, 1, 2, 1, 2},
	{0, 1, 1, 1, 2}, {1, 1, 1, 1, 2}, {2, 1, 1, 1, 2}, {1, 1, 2, 1, 2},
	{0, 2, 1, 1, 2}, {1, 2, 1, 1, 2}, {2, 2, 1, 1, 2}, {2, 1, 2, 1, 2},
	{0, 2, 2, 2, 2}, {1, 2, 2, 2, 2}, {2, 2, 2, 2, 2}, {2, 1, 2, 2, 2}
};

void find_number_of_bits_trits_quints(
	int quantization_level,
	int* bits,
	int* trits,
	int* quints
) {
	*bits = 0;
	*trits = 0;
	*quints = 0;
	switch (quantization_level)
	{
	case QUANT_2:
		*bits = 1;
		break;
	case QUANT_3:
		*bits = 0;
		*trits = 1;
		break;
	case QUANT_4:
		*bits = 2;
		break;
	case QUANT_5:
		*bits = 0;
		*quints = 1;
		break;
	case QUANT_6:
		*bits = 1;
		*trits = 1;
		break;
	case QUANT_8:
		*bits = 3;
		break;
	case QUANT_10:
		*bits = 1;
		*quints = 1;
		break;
	case QUANT_12:
		*bits = 2;
		*trits = 1;
		break;
	case QUANT_16:
		*bits = 4;
		break;
	case QUANT_20:
		*bits = 2;
		*quints = 1;
		break;
	case QUANT_24:
		*bits = 3;
		*trits = 1;
		break;
	case QUANT_32:
		*bits = 5;
		break;
	case QUANT_40:
		*bits = 3;
		*quints = 1;
		break;
	case QUANT_48:
		*bits = 4;
		*trits = 1;
		break;
	case QUANT_64:
		*bits = 6;
		break;
	case QUANT_80:
		*bits = 4;
		*quints = 1;
		break;
	case QUANT_96:
		*bits = 5;
		*trits = 1;
		break;
	case QUANT_128:
		*bits = 7;
		break;
	case QUANT_160:
		*bits = 5;
		*quints = 1;
		break;
	case QUANT_192:
		*bits = 6;
		*trits = 1;
		break;
	case QUANT_256:
		*bits = 8;
		break;
	}
}

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

void decode_ise(
	int quantization_level,
	int elements,
	const uint8_t* input_data,
	uint8_t* output_data,
	int bit_offset
) {
	int i;
	// note: due to how the trit/quint-block unpacking is done in this function,
	// we may write more temporary results than the number of outputs
	// The maximum actual number of results is 64 bit, but we keep 4 additional elements
	// of padding.
	uint8_t results[68];
	uint8_t tq_blocks[22];		// trit-blocks or quint-blocks

	int bits, trits, quints;
	find_number_of_bits_trits_quints(quantization_level, &bits, &trits, &quints);

	int lcounter = 0;
	int hcounter = 0;

	// trit-blocks or quint-blocks must be zeroed out before we collect them in the loop below.
	for (i = 0; i < 22; i++)
		tq_blocks[i] = 0;

	// collect bits for each element, as well as bits for any trit-blocks and quint-blocks.
	for (i = 0; i < elements; i++)
	{
		results[i] = read_bits(bits, bit_offset, input_data);
		bit_offset += bits;

		if (trits)
		{
			static const int bits_to_read[5] = { 2, 2, 1, 2, 1 };
			static const int block_shift[5] = { 0, 2, 4, 5, 7 };
			static const int next_lcounter[5] = { 1, 2, 3, 4, 0 };
			static const int hcounter_incr[5] = { 0, 0, 0, 0, 1 };
			int tdata = read_bits(bits_to_read[lcounter], bit_offset, input_data);
			bit_offset += bits_to_read[lcounter];
			tq_blocks[hcounter] |= tdata << block_shift[lcounter];
			hcounter += hcounter_incr[lcounter];
			lcounter = next_lcounter[lcounter];
		}

		if (quints)
		{
			static const int bits_to_read[3] = { 3, 2, 2 };
			static const int block_shift[3] = { 0, 3, 5 };
			static const int next_lcounter[3] = { 1, 2, 0 };
			static const int hcounter_incr[3] = { 0, 0, 1 };
			int tdata = read_bits(bits_to_read[lcounter], bit_offset, input_data);
			bit_offset += bits_to_read[lcounter];
			tq_blocks[hcounter] |= tdata << block_shift[lcounter];
			hcounter += hcounter_incr[lcounter];
			lcounter = next_lcounter[lcounter];
		}
	}

	// unpack trit-blocks or quint-blocks as needed
	if (trits)
	{
		int trit_blocks = (elements + 4) / 5;
		for (i = 0; i < trit_blocks; i++)
		{
			const uint8_t *tritptr = trits_of_integer[tq_blocks[i]];
			results[5 * i] |= tritptr[0] << bits;
			results[5 * i + 1] |= tritptr[1] << bits;
			results[5 * i + 2] |= tritptr[2] << bits;
			results[5 * i + 3] |= tritptr[3] << bits;
			results[5 * i + 4] |= tritptr[4] << bits;
		}
	}

	if (quints)
	{
		int quint_blocks = (elements + 2) / 3;
		for (i = 0; i < quint_blocks; i++)
		{
			const uint8_t *quintptr = quints_of_integer[tq_blocks[i]];
			results[3 * i] |= quintptr[0] << bits;
			results[3 * i + 1] |= quintptr[1] << bits;
			results[3 * i + 2] |= quintptr[2] << bits;
		}
	}

	for (i = 0; i < elements; i++)
		output_data[i] = results[i];
}

int compute_ise_bitcount(
	int items,
	quantization_method quant
) {
	switch (quant)
	{
	case QUANT_2:
		return items;
	case QUANT_3:
		return (8 * items + 4) / 5;
	case QUANT_4:
		return 2 * items;
	case QUANT_5:
		return (7 * items + 2) / 3;
	case QUANT_6:
		return (13 * items + 4) / 5;
	case QUANT_8:
		return 3 * items;
	case QUANT_10:
		return (10 * items + 2) / 3;
	case QUANT_12:
		return (18 * items + 4) / 5;
	case QUANT_16:
		return items * 4;
	case QUANT_20:
		return (13 * items + 2) / 3;
	case QUANT_24:
		return (23 * items + 4) / 5;
	case QUANT_32:
		return 5 * items;
	case QUANT_40:
		return (16 * items + 2) / 3;
	case QUANT_48:
		return (28 * items + 4) / 5;
	case QUANT_64:
		return 6 * items;
	case QUANT_80:
		return (19 * items + 2) / 3;
	case QUANT_96:
		return (33 * items + 4) / 5;
	case QUANT_128:
		return 7 * items;
	case QUANT_160:
		return (22 * items + 2) / 3;
	case QUANT_192:
		return (38 * items + 4) / 5;
	case QUANT_256:
		return 8 * items;
	default:
		return 100000;
	}
}
