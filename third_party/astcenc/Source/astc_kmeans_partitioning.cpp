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
 *	@brief	approximate k-means cluster partitioning. Do this in 2 stages
 *
 *			1: basic clustering, a couple of passes just to get a few clusters
 *			2: clustering based on line, a few passes until it seems to
 *			   stabilize.
 *
 *			After clustering is done, we use the clustering result to construct
 *			one bitmap for each partition. We then scan though the partition table,
 *			counting how well the bitmaps matched.
 */
/*----------------------------------------------------------------------------*/

#include "astc_codec_internals.h"

// for k++ means, we need pseudo-random numbers, however using random numbers directly
// results in irreproducible encoding results. As such, we will instead
// just supply a handful of numbers from random.org, and apply an algorithm similar
// to XKCD #221. (http://xkcd.com/221/)
// cluster the texels using the k++ means clustering initialization algorithm.

void kpp_initialize(int xdim, int ydim, int zdim, int partition_count, const imageblock * blk, float4 * cluster_centers)
{
	int i;

	int texels_per_block = xdim * ydim * zdim;

	int cluster_center_samples[4];
	// pick a random sample as first center-point.
	cluster_center_samples[0] = 145897 /* number from random.org */  % texels_per_block;
	int samples_selected = 1;

	float distances[MAX_TEXELS_PER_BLOCK];

	// compute the distance to the first point.
	int sample = cluster_center_samples[0];
	float4 center_color = float4(blk->work_data[4 * sample],
								 blk->work_data[4 * sample + 1],
								 blk->work_data[4 * sample + 2],
								 blk->work_data[4 * sample + 3]);

	float distance_sum = 0.0f;
	for (i = 0; i < texels_per_block; i++)
	{
		float4 color = float4(blk->work_data[4 * i],
							  blk->work_data[4 * i + 1],
							  blk->work_data[4 * i + 2],
							  blk->work_data[4 * i + 3]);
		float4 diff = color - center_color;
		float distance = dot(diff, diff);
		distance_sum += distance;
		distances[i] = distance;
	}

	// more numbers from random.org
	float cluster_cutoffs[25] = {
		0.952312f, 0.206893f, 0.835984f, 0.507813f, 0.466170f,
		0.872331f, 0.488028f, 0.866394f, 0.363093f, 0.467905f,
		0.812967f, 0.626220f, 0.932770f, 0.275454f, 0.832020f,
		0.362217f, 0.318558f, 0.240113f, 0.009190f, 0.983995f,
		0.566812f, 0.347661f, 0.731960f, 0.156391f, 0.297786f
	};

	while (1)
	{
		// pick a point in a weighted-random fashion.
		float summa = 0.0f;
		float distance_cutoff = distance_sum * cluster_cutoffs[samples_selected + 5 * partition_count];
		for (i = 0; i < texels_per_block; i++)
		{
			summa += distances[i];
			if (summa >= distance_cutoff)
				break;
		}
		sample = i;
		if (sample >= texels_per_block)
			sample = texels_per_block - 1;


		cluster_center_samples[samples_selected] = sample;
		samples_selected++;
		if (samples_selected >= partition_count)
			break;

		// update the distances with the new point.
		center_color = float4(blk->work_data[4 * sample], blk->work_data[4 * sample + 1], blk->work_data[4 * sample + 2], blk->work_data[4 * sample + 3]);

		distance_sum = 0.0f;
		for (i = 0; i < texels_per_block; i++)
		{
			float4 color = float4(blk->work_data[4 * i],
								  blk->work_data[4 * i + 1],
								  blk->work_data[4 * i + 2],
								  blk->work_data[4 * i + 3]);
			float4 diff = color - center_color;
			float distance = dot(diff, diff);
			distance = MIN(distance, distances[i]);
			distance_sum += distance;
			distances[i] = distance;
		}
	}

	// finally, gather up the results.
	for (i = 0; i < partition_count; i++)
	{
		int sample = cluster_center_samples[i];
		float4 color = float4(blk->work_data[4 * sample],
							  blk->work_data[4 * sample + 1],
							  blk->work_data[4 * sample + 2],
							  blk->work_data[4 * sample + 3]);
		cluster_centers[i] = color;
	}
}


// basic K-means clustering: given a set of cluster centers,
// assign each texel to a partition
void basic_kmeans_assign_pass(int xdim, int ydim, int zdim, int partition_count, const imageblock * blk, const float4 * cluster_centers, int *partition_of_texel)
{
	int i, j;

	int texels_per_block = xdim * ydim * zdim;

	float distances[MAX_TEXELS_PER_BLOCK];
	float4 center_color = cluster_centers[0];

	int texels_per_partition[4];

	texels_per_partition[0] = texels_per_block;
	for (i = 1; i < partition_count; i++)
		texels_per_partition[i] = 0;


	for (i = 0; i < texels_per_block; i++)
	{
		float4 color = float4(blk->work_data[4 * i],
							  blk->work_data[4 * i + 1],
							  blk->work_data[4 * i + 2],
							  blk->work_data[4 * i + 3]);
		float4 diff = color - center_color;
		float distance = dot(diff, diff);
		distances[i] = distance;
		partition_of_texel[i] = 0;
	}



	for (j = 1; j < partition_count; j++)
	{
		float4 center_color = cluster_centers[j];

		for (i = 0; i < texels_per_block; i++)
		{
			float4 color = float4(blk->work_data[4 * i],
								  blk->work_data[4 * i + 1],
								  blk->work_data[4 * i + 2],
								  blk->work_data[4 * i + 3]);
			float4 diff = color - center_color;
			float distance = dot(diff, diff);
			if (distance < distances[i])
			{
				distances[i] = distance;
				texels_per_partition[partition_of_texel[i]]--;
				texels_per_partition[j]++;
				partition_of_texel[i] = j;
			}
		}
	}

	// it is possible to get a situation where one of the partitions ends up
	// without any texels. In this case, we assign texel N to partition N;
	// this is silly, but ensures that every partition retains at least one texel.
	// Reassigning a texel in this manner may cause another partition to go empty,
	// so if we actually did a reassignment, we run the whole loop over again.
	int problem_case;
	do
	{
		problem_case = 0;
		for (i = 0; i < partition_count; i++)
		{
			if (texels_per_partition[i] == 0)
			{
				texels_per_partition[partition_of_texel[i]]--;
				texels_per_partition[i]++;
				partition_of_texel[i] = i;
				problem_case = 1;
			}
		}
	}
	while (problem_case != 0);

}


// basic k-means clustering: given a set of cluster assignments
// for the texels, find the center position of each cluster.
void basic_kmeans_update(int xdim, int ydim, int zdim, int partition_count, const imageblock * blk, const int *partition_of_texel, float4 * cluster_centers)
{
	int i;

	int texels_per_block = xdim * ydim * zdim;

	float4 color_sum[4];
	int weight_sum[4];

	for (i = 0; i < partition_count; i++)
	{
		color_sum[i] = float4(0, 0, 0, 0);
		weight_sum[i] = 0;
	}


	// first, find the center-of-gravity in each cluster
	for (i = 0; i < texels_per_block; i++)
	{
		float4 color = float4(blk->work_data[4 * i],
							  blk->work_data[4 * i + 1],
							  blk->work_data[4 * i + 2],
							  blk->work_data[4 * i + 3]);
		int part = partition_of_texel[i];
		color_sum[part] = color_sum[part] + color;
		weight_sum[part]++;
	}

	for (i = 0; i < partition_count; i++)
	{
		cluster_centers[i] = color_sum[i] * (1.0f / weight_sum[i]);
	}
}




// after a few rounds of k-means-clustering, we should have a set of 2, 3 or 4 partitions;
// we then turn this set into 2, 3 or 4 bitmaps. Then, for each of the 1024 partitions,
// we try to match the bitmaps as well as possible.




static inline int bitcount(uint64_t p)
{
	if (sizeof(void *) > 4)
	{
		uint64_t mask1 = 0x5555555555555555ULL;
		uint64_t mask2 = 0x3333333333333333ULL;
		uint64_t mask3 = 0x0F0F0F0F0F0F0F0FULL;
		// best-known algorithm for 64-bit bitcount, assuming 64-bit processor
		// should probably be adapted for use with 32-bit processors and/or processors
		// with a POPCNT instruction, but leave that for later.
		p -= (p >> 1) & mask1;
		p = (p & mask2) + ((p >> 2) & mask2);
		p += p >> 4;
		p &= mask3;
		p *= 0x0101010101010101ULL;
		p >>= 56;
		return (int)p;
	}
	else
	{
		// on 32-bit processor, split the 64-bit input argument in two,
		// and bitcount each half separately.
		uint32_t p1 = (uint32_t) p;
		uint32_t p2 = (uint32_t) (p >> 32);
		uint32_t mask1 = 0x55555555U;
		uint32_t mask2 = 0x33333333U;
		uint32_t mask3 = 0x0F0F0F0FU;
		p1 = p1 - ((p1 >> 1) & mask1);
		p2 = p2 - ((p2 >> 1) & mask1);
		p1 = (p1 & mask2) + ((p1 >> 2) & mask2);
		p2 = (p2 & mask2) + ((p2 >> 2) & mask2);
		p1 += p1 >> 4;
		p2 += p2 >> 4;
		p1 &= mask3;
		p2 &= mask3;
		p1 += p2;
		p1 *= 0x01010101U;
		p1 >>= 24;
		return (int)p1;
	}
}


// compute the bit-mismatch for a partitioning in 2-partition mode
static inline int partition_mismatch2(uint64_t a0, uint64_t a1, uint64_t b0, uint64_t b1)
{
	int v1 = bitcount(a0 ^ b0) + bitcount(a1 ^ b1);
	int v2 = bitcount(a0 ^ b1) + bitcount(a1 ^ b0);
	return MIN(v1, v2);
}


// compute the bit-mismatch for a partitioning in 3-partition mode
static inline int partition_mismatch3(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t b0, uint64_t b1, uint64_t b2)
{
	int p00 = bitcount(a0 ^ b0);
	int p01 = bitcount(a0 ^ b1);
	int p02 = bitcount(a0 ^ b2);

	int p10 = bitcount(a1 ^ b0);
	int p11 = bitcount(a1 ^ b1);
	int p12 = bitcount(a1 ^ b2);

	int p20 = bitcount(a2 ^ b0);
	int p21 = bitcount(a2 ^ b1);
	int p22 = bitcount(a2 ^ b2);

	int s0 = p11 + p22;
	int s1 = p12 + p21;
	int v0 = MIN(s0, s1) + p00;

	int s2 = p10 + p22;
	int s3 = p12 + p20;
	int v1 = MIN(s2, s3) + p01;

	int s4 = p10 + p21;
	int s5 = p11 + p20;
	int v2 = MIN(s4, s5) + p02;

	if (v1 < v0)
		v0 = v1;
	if (v2 < v0)
		v0 = v2;

	// 9 add, 5 MIN

	return v0;
}

static inline int MIN3(int a, int b, int c)
{
	int d = MIN(a, b);
	return MIN(c, d);
}

// compute the bit-mismatch for a partitioning in 4-partition mode
static inline int partition_mismatch4(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t b0, uint64_t b1, uint64_t b2, uint64_t b3)
{
	int p00 = bitcount(a0 ^ b0);
	int p01 = bitcount(a0 ^ b1);
	int p02 = bitcount(a0 ^ b2);
	int p03 = bitcount(a0 ^ b3);

	int p10 = bitcount(a1 ^ b0);
	int p11 = bitcount(a1 ^ b1);
	int p12 = bitcount(a1 ^ b2);
	int p13 = bitcount(a1 ^ b3);

	int p20 = bitcount(a2 ^ b0);
	int p21 = bitcount(a2 ^ b1);
	int p22 = bitcount(a2 ^ b2);
	int p23 = bitcount(a2 ^ b3);

	int p30 = bitcount(a3 ^ b0);
	int p31 = bitcount(a3 ^ b1);
	int p32 = bitcount(a3 ^ b2);
	int p33 = bitcount(a3 ^ b3);

	int mx23 = MIN(p22 + p33, p23 + p32);
	int mx13 = MIN(p21 + p33, p23 + p31);
	int mx12 = MIN(p21 + p32, p22 + p31);
	int mx03 = MIN(p20 + p33, p23 + p30);
	int mx02 = MIN(p20 + p32, p22 + p30);
	int mx01 = MIN(p21 + p30, p20 + p31);

	int v0 = p00 + MIN3(p11 + mx23, p12 + mx13, p13 + mx12);
	int v1 = p01 + MIN3(p10 + mx23, p12 + mx03, p13 + mx02);
	int v2 = p02 + MIN3(p11 + mx03, p10 + mx13, p13 + mx01);
	int v3 = p03 + MIN3(p11 + mx02, p12 + mx01, p10 + mx12);

	int x0 = MIN(v0, v1);
	int x1 = MIN(v2, v3);
	return MIN(x0, x1);

	// 16 bitcount, 17 MIN, 28 ADD
}



void count_partition_mismatch_bits(int xdim, int ydim, int zdim, int partition_count, const uint64_t bitmaps[4], int bitcounts[PARTITION_COUNT])
{
	int i;
	const partition_info *pi = get_partition_table(xdim, ydim, zdim, partition_count);

	if (partition_count == 2)
	{
		uint64_t bm0 = bitmaps[0];
		uint64_t bm1 = bitmaps[1];
		for (i = 0; i < PARTITION_COUNT; i++)
		{
			if (pi->partition_count == 2)
			{
				bitcounts[i] = partition_mismatch2(bm0, bm1, pi->coverage_bitmaps[0], pi->coverage_bitmaps[1]);
			}
			else
				bitcounts[i] = 255;
			pi++;
		}
	}
	else if (partition_count == 3)
	{
		uint64_t bm0 = bitmaps[0];
		uint64_t bm1 = bitmaps[1];
		uint64_t bm2 = bitmaps[2];
		for (i = 0; i < PARTITION_COUNT; i++)
		{
			if (pi->partition_count == 3)
			{
				bitcounts[i] = partition_mismatch3(bm0, bm1, bm2, pi->coverage_bitmaps[0], pi->coverage_bitmaps[1], pi->coverage_bitmaps[2]);
			}
			else
				bitcounts[i] = 255;
			pi++;
		}
	}
	else if (partition_count == 4)
	{
		uint64_t bm0 = bitmaps[0];
		uint64_t bm1 = bitmaps[1];
		uint64_t bm2 = bitmaps[2];
		uint64_t bm3 = bitmaps[3];
		for (i = 0; i < PARTITION_COUNT; i++)
		{
			if (pi->partition_count == 4)
			{
				bitcounts[i] = partition_mismatch4(bm0, bm1, bm2, bm3, pi->coverage_bitmaps[0], pi->coverage_bitmaps[1], pi->coverage_bitmaps[2], pi->coverage_bitmaps[3]);
			}
			else
				bitcounts[i] = 255;
			pi++;
		}
	}

}


// counting-sort on the mismatch-bits, thereby
// sorting the partitions into an ordering.

void get_partition_ordering_by_mismatch_bits(const int mismatch_bits[PARTITION_COUNT], int partition_ordering[PARTITION_COUNT])
{
	int i;

	int mscount[256];
	for (i = 0; i < 256; i++)
		mscount[i] = 0;

	for (i = 0; i < PARTITION_COUNT; i++)
		mscount[mismatch_bits[i]]++;

	int summa = 0;
	for (i = 0; i < 256; i++)
	{
		int cnt = mscount[i];
		mscount[i] = summa;
		summa += cnt;
	}

	for (i = 0; i < PARTITION_COUNT; i++)
	{
		int idx = mscount[mismatch_bits[i]]++;
		partition_ordering[idx] = i;
	}
}




void kmeans_compute_partition_ordering(int xdim, int ydim, int zdim, int partition_count, const imageblock * blk, int *ordering)
{
	int i;

	const block_size_descriptor *bsd = get_block_size_descriptor(xdim, ydim, zdim);

	float4 cluster_centers[4];
	int partition_of_texel[MAX_TEXELS_PER_BLOCK];

	// 3 passes of plain k-means partitioning
	for (i = 0; i < 3; i++)
	{
		if (i == 0)
			kpp_initialize(xdim, ydim, zdim, partition_count, blk, cluster_centers);
		else
			basic_kmeans_update(xdim, ydim, zdim, partition_count, blk, partition_of_texel, cluster_centers);

		basic_kmeans_assign_pass(xdim, ydim, zdim, partition_count, blk, cluster_centers, partition_of_texel);
	}

	// at this point, we have a near-ideal partitioning.

	// construct bitmaps
	uint64_t bitmaps[4];
	for (i = 0; i < 4; i++)
		bitmaps[i] = 0ULL;

	int texels_to_process = bsd->texelcount_for_bitmap_partitioning;
	for (i = 0; i < texels_to_process; i++)
	{
		int idx = bsd->texels_for_bitmap_partitioning[i];
		bitmaps[partition_of_texel[idx]] |= 1ULL << i;
	}

	int bitcounts[PARTITION_COUNT];
	// for each entry in the partition table, count bits of partition-mismatch.
	count_partition_mismatch_bits(xdim, ydim, zdim, partition_count, bitmaps, bitcounts);

	// finally, sort the partitions by bits-of-partition-mismatch
	get_partition_ordering_by_mismatch_bits(bitcounts, ordering);

}
