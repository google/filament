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
 *	@brief	Functions to generate partition tables for ASTC.
 *
 *			We generate tables only for the block sizes that have actually been
 *			specified to the codec.
 */ 
/*----------------------------------------------------------------------------*/ 

#include "astc_codec_internals.h"

static partition_info **partition_tables[4096];

/*
	Produce a canonicalized representation of a partition pattern

	The largest possible such representation is 432 bits, equal to 7 uint64_t values.
*/
static void gen_canonicalized_partition_table(int texel_count, const uint8_t * partition_table, uint64_t canonicalized[7])
{
	int i;
	for (i = 0; i < 7; i++)
		canonicalized[i] = 0;

	int mapped_index[4];
	int map_weight_count = 0;
	for (i = 0; i < 4; i++)
		mapped_index[i] = -1;

	for (i = 0; i < texel_count; i++)
	{
		int index = partition_table[i];
		if (mapped_index[index] == -1)
			mapped_index[index] = map_weight_count++;
		uint64_t xlat_index = mapped_index[index];
		canonicalized[i >> 5] |= xlat_index << (2 * (i & 0x1F));
	}
}


static int compare_canonicalized_partition_tables(const uint64_t part1[7], const uint64_t part2[7])
{
	if (part1[0] != part2[0])
		return 0;
	if (part1[1] != part2[1])
		return 0;
	if (part1[2] != part2[2])
		return 0;
	if (part1[3] != part2[3])
		return 0;
	if (part1[4] != part2[4])
		return 0;
	if (part1[5] != part2[5])
		return 0;
	if (part1[6] != part2[6])
		return 0;
	return 1;
}


/* 
   For a partition table, detect partitionings that are equivalent, then mark them as invalid. This reduces the number of partitions that the codec has to consider and thus improves encode
   performance. */
static void partition_table_zap_equal_elements(int xdim, int ydim, int zdim, partition_info * pi)
{
	int partition_tables_zapped = 0;

	int texel_count = xdim * ydim * zdim;

	int i, j;
	uint64_t *canonicalizeds = new uint64_t[PARTITION_COUNT * 7];


	for (i = 0; i < PARTITION_COUNT; i++)
	{
		gen_canonicalized_partition_table(texel_count, pi[i].partition_of_texel, canonicalizeds + i * 7);
	}

	for (i = 0; i < PARTITION_COUNT; i++)
	{
		for (j = 0; j < i; j++)
		{
			if (compare_canonicalized_partition_tables(canonicalizeds + 7 * i, canonicalizeds + 7 * j))
			{
				pi[i].partition_count = 0;
				partition_tables_zapped++;
				break;
			}
		}
	}
	delete[]canonicalizeds;
}


uint32_t hash52(uint32_t inp)
{
	inp ^= inp >> 15;

	inp *= 0xEEDE0891;			// (2^4+1)*(2^7+1)*(2^17-1)
	inp ^= inp >> 5;
	inp += inp << 16;
	inp ^= inp >> 7;
	inp ^= inp >> 3;
	inp ^= inp << 6;
	inp ^= inp >> 17;
	return inp;
}



int select_partition(int seed, int x, int y, int z, int partitioncount, int small_block)
{
	if (small_block)
	{
		x <<= 1;
		y <<= 1;
		z <<= 1;
	}

	seed += (partitioncount - 1) * 1024;

	uint32_t rnum = hash52(seed);

	uint8_t seed1 = rnum & 0xF;
	uint8_t seed2 = (rnum >> 4) & 0xF;
	uint8_t seed3 = (rnum >> 8) & 0xF;
	uint8_t seed4 = (rnum >> 12) & 0xF;
	uint8_t seed5 = (rnum >> 16) & 0xF;
	uint8_t seed6 = (rnum >> 20) & 0xF;
	uint8_t seed7 = (rnum >> 24) & 0xF;
	uint8_t seed8 = (rnum >> 28) & 0xF;
	uint8_t seed9 = (rnum >> 18) & 0xF;
	uint8_t seed10 = (rnum >> 22) & 0xF;
	uint8_t seed11 = (rnum >> 26) & 0xF;
	uint8_t seed12 = ((rnum >> 30) | (rnum << 2)) & 0xF;

	// squaring all the seeds in order to bias their distribution
	// towards lower values.
	seed1 *= seed1;
	seed2 *= seed2;
	seed3 *= seed3;
	seed4 *= seed4;
	seed5 *= seed5;
	seed6 *= seed6;
	seed7 *= seed7;
	seed8 *= seed8;
	seed9 *= seed9;
	seed10 *= seed10;
	seed11 *= seed11;
	seed12 *= seed12;


	int sh1, sh2, sh3;
	if (seed & 1)
	{
		sh1 = (seed & 2 ? 4 : 5);
		sh2 = (partitioncount == 3 ? 6 : 5);
	}
	else
	{
		sh1 = (partitioncount == 3 ? 6 : 5);
		sh2 = (seed & 2 ? 4 : 5);
	}
	sh3 = (seed & 0x10) ? sh1 : sh2;

	seed1 >>= sh1;
	seed2 >>= sh2;
	seed3 >>= sh1;
	seed4 >>= sh2;
	seed5 >>= sh1;
	seed6 >>= sh2;
	seed7 >>= sh1;
	seed8 >>= sh2;

	seed9 >>= sh3;
	seed10 >>= sh3;
	seed11 >>= sh3;
	seed12 >>= sh3;



	int a = seed1 * x + seed2 * y + seed11 * z + (rnum >> 14);
	int b = seed3 * x + seed4 * y + seed12 * z + (rnum >> 10);
	int c = seed5 * x + seed6 * y + seed9 * z + (rnum >> 6);
	int d = seed7 * x + seed8 * y + seed10 * z + (rnum >> 2);


	// apply the saw
	a &= 0x3F;
	b &= 0x3F;
	c &= 0x3F;
	d &= 0x3F;

	// remove some of the components if we are to output < 4 partitions.
	if (partitioncount <= 3)
		d = 0;
	if (partitioncount <= 2)
		c = 0;
	if (partitioncount <= 1)
		b = 0;

	int partition;
	if (a >= b && a >= c && a >= d)
		partition = 0;
	else if (b >= c && b >= d)
		partition = 1;
	else if (c >= d)
		partition = 2;
	else
		partition = 3;
	return partition;
}



void generate_one_partition_table(int xdim, int ydim, int zdim, int partition_count, int partition_index, partition_info * pt)
{
	int small_block = (xdim * ydim * zdim) < 32;

	uint8_t *partition_of_texel = pt->partition_of_texel;
	int x, y, z, i;


	for (z = 0; z < zdim; z++)
		for (y = 0; y < ydim; y++)
			for (x = 0; x < xdim; x++)
			{
				uint8_t part = select_partition(partition_index, x, y, z, partition_count, small_block);
				*partition_of_texel++ = part;
			}


	int texels_per_block = xdim * ydim * zdim;

	int counts[4];
	for (i = 0; i < 4; i++)
		counts[i] = 0;

	for (i = 0; i < texels_per_block; i++)
	{
		int partition = pt->partition_of_texel[i];
		pt->texels_of_partition[partition][counts[partition]++] = i;
	}

	for (i = 0; i < 4; i++)
		pt->texels_per_partition[i] = counts[i];

	if (counts[0] == 0)
		pt->partition_count = 0;
	else if (counts[1] == 0)
		pt->partition_count = 1;
	else if (counts[2] == 0)
		pt->partition_count = 2;
	else if (counts[3] == 0)
		pt->partition_count = 3;
	else
		pt->partition_count = 4;



	for (i = 0; i < 4; i++)
		pt->coverage_bitmaps[i] = 0ULL;

	const block_size_descriptor *bsd = get_block_size_descriptor(xdim, ydim, zdim);
	int texels_to_process = bsd->texelcount_for_bitmap_partitioning;
	for (i = 0; i < texels_to_process; i++)
	{
		int idx = bsd->texels_for_bitmap_partitioning[i];
		pt->coverage_bitmaps[pt->partition_of_texel[idx]] |= 1ULL << i;
	}

}

static void generate_partition_tables(int xdim, int ydim, int zdim)
{
	int i;


	partition_info *one_partition = new partition_info;
	partition_info *two_partitions = new partition_info[1024];
	partition_info *three_partitions = new partition_info[1024];
	partition_info *four_partitions = new partition_info[1024];

	partition_info **partition_table = new partition_info *[5];
	partition_table[0] = NULL;
	partition_table[1] = one_partition;
	partition_table[2] = two_partitions;
	partition_table[3] = three_partitions;
	partition_table[4] = four_partitions;

	generate_one_partition_table(xdim, ydim, zdim, 1, 0, one_partition);
	for (i = 0; i < 1024; i++)
	{
		generate_one_partition_table(xdim, ydim, zdim, 2, i, two_partitions + i);
		generate_one_partition_table(xdim, ydim, zdim, 3, i, three_partitions + i);
		generate_one_partition_table(xdim, ydim, zdim, 4, i, four_partitions + i);
	}

	partition_table_zap_equal_elements(xdim, ydim, zdim, two_partitions);
	partition_table_zap_equal_elements(xdim, ydim, zdim, three_partitions);
	partition_table_zap_equal_elements(xdim, ydim, zdim, four_partitions);

	partition_tables[xdim + 16 * ydim + 256 * zdim] = partition_table;
}


const partition_info *get_partition_table(int xdim, int ydim, int zdim, int partition_count)
{
	int ptindex = xdim + 16 * ydim + 256 * zdim;
	if (partition_tables[ptindex] == NULL)
		generate_partition_tables(xdim, ydim, zdim);

	return partition_tables[ptindex][partition_count];
}
