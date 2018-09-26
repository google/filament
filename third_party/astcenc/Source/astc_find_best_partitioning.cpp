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
 *	@brief	ASTC encoding of texture
 *
 *			major step 1:
 *			* find best partitioning assuming uncorrelated colors
 *			* find best partitioning assuming RGBS color representation
 *
 *			finding best partitioning for a block:
 *			* for each available partitioning:
 *			* compute mean-color-value and dominant direction.
 *			* this defines two lines, both of which go through the
 *			  mean-color-value:
 *			* one line has a direction defined by the dominant direction;
 *			  this line is used to assess the error from using an uncorrelated
 *			  color representation.
 *			* the other line goes through (0,0,0,1) and is used to assess the
 *			  error from using an RGBS color representation.
 *			* we then compute, as a sum across the block, the squared-errors
 *			  that result from using the dominant-direction-lines and the
 *			  squared-errors that result from using the 0001-lines.
 */
/*----------------------------------------------------------------------------*/

/*
 *	Partition table representation:
 *	We have 3 tables, each with 1024 partitionings
 *	(these correspond to the 3x128 hardware partitionings crossed with all the
 *	partition-transform modes in the hardware.)
 *
 *	For each partitioning, we have:
 *	* a 4-entry table indicating how many texels there are in each of the 4
 *	  partitions. this may be from 2 to about 60 or so.
 *	* a 64-entry table indicating the partition index of each of the 64 texels
 *	  in the block. each index may be 0, 1, 2 or 3.
 *
 * each element in the table is an uint8_t indicating partition index (0, 1, 2 or 3)
 */

#include <math.h>

#include "astc_codec_internals.h"

#ifdef DEBUG_PRINT_DIAGNOSTICS
	#include <stdio.h>
#endif

#include "mathlib.h"

int imageblock_uses_alpha(int xdim, int ydim, int zdim, const imageblock * pb)
{
	IGNORE(xdim);
	IGNORE(ydim);
	IGNORE(zdim);

	return pb->alpha_max != pb->alpha_min;
}

static void compute_alpha_minmax(int xdim, int ydim, int zdim, const partition_info * pt, const imageblock * blk, const error_weight_block * ewb, float *alpha_min, float *alpha_max)
{
	int i;
	int partition_count = pt->partition_count;

	int texels_per_block = xdim * ydim * zdim;

	for (i = 0; i < partition_count; i++)
	{
		alpha_min[i] = 1e38f;
		alpha_max[i] = -1e38f;
	}

	for (i = 0; i < texels_per_block; i++)
	{
		if (ewb->texel_weight[i] > 1e-10)
		{
			int partition = pt->partition_of_texel[i];
			float alphaval = blk->work_data[4 * i + 3];
			if (alphaval > alpha_max[partition])
				alpha_max[partition] = alphaval;
			if (alphaval < alpha_min[partition])
				alpha_min[partition] = alphaval;
		}
	}

	for (i = 0; i < partition_count; i++)
	{
		if (alpha_min[i] >= alpha_max[i])
		{
			alpha_min[i] = 0;
			alpha_max[i] = 1e-10f;
		}
	}
}


static void compute_rgb_minmax(int xdim,
							   int ydim,
							   int zdim,
							   const partition_info * pt,
							   const imageblock * blk, const error_weight_block * ewb, float *red_min, float *red_max, float *green_min, float *green_max, float *blue_min, float *blue_max)
{
	int i;
	int partition_count = pt->partition_count;
	int texels_per_block = xdim * ydim * zdim;

	for (i = 0; i < partition_count; i++)
	{
		red_min[i] = 1e38f;
		red_max[i] = -1e38f;
		green_min[i] = 1e38f;
		green_max[i] = -1e38f;
		blue_min[i] = 1e38f;
		blue_max[i] = -1e38f;
	}

	for (i = 0; i < texels_per_block; i++)
	{
		if (ewb->texel_weight[i] > 1e-10f)
		{
			int partition = pt->partition_of_texel[i];
			float redval = blk->work_data[4 * i];
			float greenval = blk->work_data[4 * i + 1];
			float blueval = blk->work_data[4 * i + 2];
			if (redval > red_max[partition])
				red_max[partition] = redval;
			if (redval < red_min[partition])
				red_min[partition] = redval;
			if (greenval > green_max[partition])
				green_max[partition] = greenval;
			if (greenval < green_min[partition])
				green_min[partition] = greenval;
			if (blueval > blue_max[partition])
				blue_max[partition] = blueval;
			if (blueval < blue_min[partition])
				blue_min[partition] = blueval;
		}
	}
	for (i = 0; i < partition_count; i++)
	{
		if (red_min[i] >= red_max[i])
		{
			red_min[i] = 0.0f;
			red_max[i] = 1e-10f;
		}
		if (green_min[i] >= green_max[i])
		{
			green_min[i] = 0.0f;
			green_max[i] = 1e-10f;
		}
		if (blue_min[i] >= blue_max[i])
		{
			blue_min[i] = 0.0f;
			blue_max[i] = 1e-10f;
		}
	}
}



void compute_partition_error_color_weightings(int xdim, int ydim, int zdim, const error_weight_block * ewb, const partition_info * pi, float4 error_weightings[4], float4 color_scalefactors[4])
{
	int i;
	int texels_per_block = xdim * ydim * zdim;
	int pcnt = pi->partition_count;
	for (i = 0; i < pcnt; i++)
		error_weightings[i] = float4(1e-12f, 1e-12f, 1e-12f, 1e-12f);
	for (i = 0; i < texels_per_block; i++)
	{
		int part = pi->partition_of_texel[i];
		error_weightings[part] = error_weightings[part] + ewb->error_weights[i];
	}
	for (i = 0; i < pcnt; i++)
	{
		error_weightings[i] = error_weightings[i] * (1.0f / pi->texels_per_partition[i]);
	}
	for (i = 0; i < pcnt; i++)
	{
		color_scalefactors[i].x = sqrt(error_weightings[i].x);
		color_scalefactors[i].y = sqrt(error_weightings[i].y);
		color_scalefactors[i].z = sqrt(error_weightings[i].z);
		color_scalefactors[i].w = sqrt(error_weightings[i].w);
	}

}


/*
   main function to identify the best partitioning for a given number of texels */


void find_best_partitionings(int partition_search_limit, int xdim, int ydim, int zdim, int partition_count,
							 const imageblock * pb, const error_weight_block * ewb, int candidates_to_return,
							 // best partitionings to use if the endpoint colors are assumed to be uncorrelated
							 int *best_partitions_uncorrellated,
							 // best partitionings to use if the endpoint colors have the same chroma
							 int *best_partitions_samechroma,
							 // best partitionings to use if using dual plane of weights
							 int *best_partitions_dual_weight_planes)
{


	int i, j;

	int texels_per_block = xdim * ydim * zdim;

	// constant used to estimate quantization error for a given partitioning;
	// the optimal value for this constant depends on bitrate.
	// These constants have been determined empirically.

	float weight_imprecision_estim = 100;

	if (texels_per_block <= 20)
		weight_imprecision_estim = 0.03f;
	else if (texels_per_block <= 31)
		weight_imprecision_estim = 0.04f;
	else if (texels_per_block <= 41)
		weight_imprecision_estim = 0.05f;
	else
		weight_imprecision_estim = 0.055f;


	int partition_sequence[PARTITION_COUNT];

	kmeans_compute_partition_ordering(xdim, ydim, zdim, partition_count, pb, partition_sequence);


	float weight_imprecision_estim_squared = weight_imprecision_estim * weight_imprecision_estim;

#ifdef DEBUG_PRINT_DIAGNOSTICS
	if (print_diagnostics)
		printf("weight_imprecision_estim = %g\n", weight_imprecision_estim);
#endif

	int uses_alpha = imageblock_uses_alpha(xdim, ydim, zdim, pb);

	const partition_info *ptab = get_partition_table(xdim, ydim, zdim, partition_count);

	// partitioning errors assuming uncorrelated-chrominance endpoints
	float uncorr_errors[PARTITION_COUNT];
	// partitioning errors assuming same-chrominance endpoints
	float samechroma_errors[PARTITION_COUNT];

	// partitioning errors assuming that one of the color channels
	// is uncorrelated from all the other ones
	float separate_errors[4 * PARTITION_COUNT];


	float *separate_red_errors = separate_errors;
	float *separate_green_errors = separate_errors + PARTITION_COUNT;
	float *separate_blue_errors = separate_errors + 2 * PARTITION_COUNT;
	float *separate_alpha_errors = separate_errors + 3 * PARTITION_COUNT;

	int defacto_search_limit = PARTITION_COUNT - 1;

	if (uses_alpha)
	{

		#ifdef DEBUG_PRINT_DIAGNOSTICS
			if (print_diagnostics)
				printf("Partition testing with alpha, %d partitions\n\n", partition_count);
		#endif

		for (i = 0; i < PARTITION_COUNT; i++)
		{
			int partition = partition_sequence[i];
			int bk_partition_count = ptab[partition].partition_count;

			if (bk_partition_count < partition_count)
			{
				#ifdef DEBUG_PRINT_DIAGNOSTICS
					if (print_diagnostics)
						printf("Partitioning %d-%d: invalid\n", partition_count, partition);
				#endif

				uncorr_errors[i] = 1e35f;
				samechroma_errors[i] = 1e35f;
				separate_red_errors[i] = 1e35f;
				separate_green_errors[i] = 1e35f;
				separate_blue_errors[i] = 1e35f;
				separate_alpha_errors[i] = 1e35f;
				continue;
			}
			// the sentinel value for partitions above the search limit must be smaller
			// than the sentinel value for invalid partitions
			if (i >= partition_search_limit)
			{
				#ifdef DEBUG_PRINT_DIAGNOSTICS
					if (print_diagnostics)
						printf("Partitioning %d-%d: excluded from testing\n", partition_count, partition);
				#endif

				defacto_search_limit = i;

				uncorr_errors[i] = 1e34f;
				samechroma_errors[i] = 1e34f;
				separate_red_errors[i] = 1e34f;
				separate_green_errors[i] = 1e34f;
				separate_blue_errors[i] = 1e34f;
				separate_alpha_errors[i] = 1e34f;
				break;
			}

			// compute the weighting to give to each color channel
			// in each partition.
			float4 error_weightings[4];
			float4 color_scalefactors[4];
			float4 inverse_color_scalefactors[4];
			compute_partition_error_color_weightings(xdim, ydim, zdim, ewb, ptab + partition, error_weightings, color_scalefactors);

			for (j = 0; j < partition_count; j++)
			{
				inverse_color_scalefactors[j].x = 1.0f / MAX(color_scalefactors[j].x, 1e-7f);
				inverse_color_scalefactors[j].y = 1.0f / MAX(color_scalefactors[j].y, 1e-7f);
				inverse_color_scalefactors[j].z = 1.0f / MAX(color_scalefactors[j].z, 1e-7f);
				inverse_color_scalefactors[j].w = 1.0f / MAX(color_scalefactors[j].w, 1e-7f);
			}

			float4 averages[4];
			float4 directions_rgba[4];
			float3 directions_gba[4];
			float3 directions_rba[4];
			float3 directions_rga[4];
			float3 directions_rgb[4];

			compute_averages_and_directions_rgba(ptab + partition, pb, ewb, color_scalefactors, averages, directions_rgba, directions_gba, directions_rba, directions_rga, directions_rgb);

			line4 uncorr_lines[4];
			line4 samechroma_lines[4];
			line3 separate_red_lines[4];
			line3 separate_green_lines[4];
			line3 separate_blue_lines[4];
			line3 separate_alpha_lines[4];

			processed_line4 proc_uncorr_lines[4];
			processed_line4 proc_samechroma_lines[4];
			processed_line3 proc_separate_red_lines[4];
			processed_line3 proc_separate_green_lines[4];
			processed_line3 proc_separate_blue_lines[4];
			processed_line3 proc_separate_alpha_lines[4];

			float uncorr_linelengths[4];
			float samechroma_linelengths[4];
			float separate_red_linelengths[4];
			float separate_green_linelengths[4];
			float separate_blue_linelengths[4];
			float separate_alpha_linelengths[4];



			for (j = 0; j < partition_count; j++)
			{
				uncorr_lines[j].a = averages[j];
				if (dot(directions_rgba[j], directions_rgba[j]) == 0.0f)
					uncorr_lines[j].b = normalize(float4(1, 1, 1, 1));
				else
					uncorr_lines[j].b = normalize(directions_rgba[j]);

				proc_uncorr_lines[j].amod = (uncorr_lines[j].a - uncorr_lines[j].b * dot(uncorr_lines[j].a, uncorr_lines[j].b)) * inverse_color_scalefactors[j];
				proc_uncorr_lines[j].bs = (uncorr_lines[j].b * color_scalefactors[j]);
				proc_uncorr_lines[j].bis = (uncorr_lines[j].b * inverse_color_scalefactors[j]);


				samechroma_lines[j].a = float4(0, 0, 0, 0);
				if (dot(averages[j], averages[j]) == 0)
					samechroma_lines[j].b = normalize(float4(1, 1, 1, 1));
				else
					samechroma_lines[j].b = normalize(averages[j]);

				proc_samechroma_lines[j].amod = (samechroma_lines[j].a - samechroma_lines[j].b * dot(samechroma_lines[j].a, samechroma_lines[j].b)) * inverse_color_scalefactors[j];
				proc_samechroma_lines[j].bs = (samechroma_lines[j].b * color_scalefactors[j]);
				proc_samechroma_lines[j].bis = (samechroma_lines[j].b * inverse_color_scalefactors[j]);

				separate_red_lines[j].a = averages[j].yzw;
				if (dot(directions_gba[j], directions_gba[j]) == 0.0f)
					separate_red_lines[j].b = normalize(float3(1, 1, 1));
				else
					separate_red_lines[j].b = normalize(directions_gba[j]);

				separate_green_lines[j].a = averages[j].xzw;
				if (dot(directions_rba[j], directions_rba[j]) == 0.0f)
					separate_green_lines[j].b = normalize(float3(1, 1, 1));
				else
					separate_green_lines[j].b = normalize(directions_rba[j]);

				separate_blue_lines[j].a = averages[j].xyw;
				if (dot(directions_rga[j], directions_rga[j]) == 0.0f)
					separate_blue_lines[j].b = normalize(float3(1, 1, 1));
				else
					separate_blue_lines[j].b = normalize(directions_rga[j]);

				separate_alpha_lines[j].a = averages[j].xyz;
				if (dot(directions_rgb[j], directions_rgb[j]) == 0.0f)
					separate_alpha_lines[j].b = normalize(float3(1, 1, 1));
				else
					separate_alpha_lines[j].b = normalize(directions_rgb[j]);

				proc_separate_red_lines[j].amod = (separate_red_lines[j].a - separate_red_lines[j].b * dot(separate_red_lines[j].a, separate_red_lines[j].b)) * inverse_color_scalefactors[j].yzw;
				proc_separate_red_lines[j].bs = (separate_red_lines[j].b * color_scalefactors[j].yzw);
				proc_separate_red_lines[j].bis = (separate_red_lines[j].b * inverse_color_scalefactors[j].yzw);

				proc_separate_green_lines[j].amod =
					(separate_green_lines[j].a - separate_green_lines[j].b * dot(separate_green_lines[j].a, separate_green_lines[j].b)) * inverse_color_scalefactors[j].xzw;
				proc_separate_green_lines[j].bs = (separate_green_lines[j].b * color_scalefactors[j].xzw);
				proc_separate_green_lines[j].bis = (separate_green_lines[j].b * inverse_color_scalefactors[j].xzw);

				proc_separate_blue_lines[j].amod = (separate_blue_lines[j].a - separate_blue_lines[j].b * dot(separate_blue_lines[j].a, separate_blue_lines[j].b)) * inverse_color_scalefactors[j].xyw;
				proc_separate_blue_lines[j].bs = (separate_blue_lines[j].b * color_scalefactors[j].xyw);
				proc_separate_blue_lines[j].bis = (separate_blue_lines[j].b * inverse_color_scalefactors[j].xyw);

				proc_separate_alpha_lines[j].amod =
					(separate_alpha_lines[j].a - separate_alpha_lines[j].b * dot(separate_alpha_lines[j].a, separate_alpha_lines[j].b)) * inverse_color_scalefactors[j].xyz;
				proc_separate_alpha_lines[j].bs = (separate_alpha_lines[j].b * color_scalefactors[j].xyz);
				proc_separate_alpha_lines[j].bis = (separate_alpha_lines[j].b * inverse_color_scalefactors[j].xyz);

			}

			float uncorr_error = compute_error_squared_rgba(ptab + partition,
															pb,
															ewb,
															proc_uncorr_lines,
															uncorr_linelengths);
			float samechroma_error = compute_error_squared_rgba(ptab + partition,
																pb,
																ewb,
																proc_samechroma_lines,
																samechroma_linelengths);


			float separate_red_error = compute_error_squared_gba(ptab + partition,
																 pb,
																 ewb,
																 proc_separate_red_lines,
																 separate_red_linelengths);

			float separate_green_error = compute_error_squared_rba(ptab + partition,
																   pb,
																   ewb,
																   proc_separate_green_lines,
																   separate_green_linelengths);

			float separate_blue_error = compute_error_squared_rga(ptab + partition,
																  pb,
																  ewb,
																  proc_separate_blue_lines,
																  separate_blue_linelengths);

			float separate_alpha_error = compute_error_squared_rgb(ptab + partition,
																   pb,
																   ewb,
																   proc_separate_alpha_lines,
																   separate_alpha_linelengths);

			// compute minimum & maximum alpha values in each partition
			float red_min[4], red_max[4];
			float green_min[4], green_max[4];
			float blue_min[4], blue_max[4];
			float alpha_min[4], alpha_max[4];
			compute_alpha_minmax(xdim, ydim, zdim, ptab + partition, pb, ewb, alpha_min, alpha_max);

			compute_rgb_minmax(xdim, ydim, zdim, ptab + partition, pb, ewb, red_min, red_max, green_min, green_max, blue_min, blue_max);

			/*
			   Compute an estimate of error introduced by weight quantization imprecision.
			   This error is computed as follows, for each partition
			   1: compute the principal-axis vector (full length) in error-space
			   2: convert the principal-axis vector to regular RGB-space
			   3: scale the vector by a constant that estimates average quantization error
			   4: for each texel, square the vector, then do a dot-product with the texel's error weight;
			      sum up the results across all texels.
			   4(optimized): square the vector once, then do a dot-product with the average texel error,
			      then multiply by the number of texels.
			 */

			for (j = 0; j < partition_count; j++)
			{
				float tpp = (float)(ptab[partition].texels_per_partition[j]);

				float4 ics = inverse_color_scalefactors[j];
				float4 error_weights = error_weightings[j] * (tpp * weight_imprecision_estim_squared);

				float4 uncorr_vector = (uncorr_lines[j].b * uncorr_linelengths[j]) * ics;
				float4 samechroma_vector = (samechroma_lines[j].b * samechroma_linelengths[j]) * ics;
				float3 separate_red_vector = (separate_red_lines[j].b * separate_red_linelengths[j]) * ics.yzw;
				float3 separate_green_vector = (separate_green_lines[j].b * separate_green_linelengths[j]) * ics.xzw;
				float3 separate_blue_vector = (separate_blue_lines[j].b * separate_blue_linelengths[j]) * ics.xyw;
				float3 separate_alpha_vector = (separate_alpha_lines[j].b * separate_alpha_linelengths[j]) * ics.xyz;

				uncorr_vector = uncorr_vector * uncorr_vector;
				samechroma_vector = samechroma_vector * samechroma_vector;
				separate_red_vector = separate_red_vector * separate_red_vector;
				separate_green_vector = separate_green_vector * separate_green_vector;
				separate_blue_vector = separate_blue_vector * separate_blue_vector;
				separate_alpha_vector = separate_alpha_vector * separate_alpha_vector;

				uncorr_error += dot(uncorr_vector, error_weights);
				samechroma_error += dot(samechroma_vector, error_weights);
				separate_red_error += dot(separate_red_vector, error_weights.yzw);
				separate_green_error += dot(separate_green_vector, error_weights.xzw);
				separate_blue_error += dot(separate_blue_vector, error_weights.xyw);
				separate_alpha_error += dot(separate_alpha_vector, error_weights.xyz);

				float red_scalar = (red_max[j] - red_min[j]);
				float green_scalar = (green_max[j] - green_min[j]);
				float blue_scalar = (blue_max[j] - blue_min[j]);
				float alpha_scalar = (alpha_max[j] - alpha_min[j]);
				red_scalar *= red_scalar;
				green_scalar *= green_scalar;
				blue_scalar *= blue_scalar;
				alpha_scalar *= alpha_scalar;
				separate_red_error += red_scalar * error_weights.x;
				separate_green_error += green_scalar * error_weights.y;
				separate_blue_error += blue_scalar * error_weights.z;
				separate_alpha_error += alpha_scalar * error_weights.w;
			}

			uncorr_errors[i] = uncorr_error;
			samechroma_errors[i] = samechroma_error;
			separate_red_errors[i] = separate_red_error;
			separate_green_errors[i] = separate_green_error;
			separate_blue_errors[i] = separate_blue_error;
			separate_alpha_errors[i] = separate_alpha_error;

			#ifdef DEBUG_PRINT_DIAGNOSTICS
				if (print_diagnostics)
					printf("Partitioning %d-%d errors: uncorr=%g, samechroma=%g, sep-alpha=%g\n", partition_count, i, uncorr_error, samechroma_error, separate_alpha_error);
			#endif
		}
	}
	else
	{

		#ifdef DEBUG_PRINT_DIAGNOSTICS
			if (print_diagnostics)
				printf("Partition testing without alpha, %d partitions\n", partition_count);
		#endif



		for (i = 0; i < PARTITION_COUNT; i++)
		{

			int partition = partition_sequence[i];

			int bk_partition_count = ptab[partition].partition_count;
			if (bk_partition_count < partition_count)
			{
				#ifdef DEBUG_PRINT_DIAGNOSTICS
					if (print_diagnostics)
						printf("Partitioning %d-%d: invalid\n", partition_count, i);
				#endif

				uncorr_errors[i] = 1e35f;
				samechroma_errors[i] = 1e35f;
				separate_red_errors[i] = 1e35f;
				separate_green_errors[i] = 1e35f;
				separate_blue_errors[i] = 1e35f;
				continue;
			}
			// the sentinel value for valid partitions above the search limit must be smaller
			// than the sentinel value for invalid partitions
			if (i >= partition_search_limit)
			{
				#ifdef DEBUG_PRINT_DIAGNOSTICS
					if (print_diagnostics)
						printf(" Partitioning %d-%d: excluded from testing\n", partition_count, partition);
				#endif

				defacto_search_limit = i;
				uncorr_errors[i] = 1e34f;
				samechroma_errors[i] = 1e34f;
				separate_red_errors[i] = 1e34f;
				separate_green_errors[i] = 1e34f;
				separate_blue_errors[i] = 1e34f;
				break;

			}

			// compute the weighting to give to each color channel
			// in each partition.
			float4 error_weightings[4];
			float4 color_scalefactors[4];
			float4 inverse_color_scalefactors[4];

			compute_partition_error_color_weightings(xdim, ydim, zdim, ewb, ptab + partition, error_weightings, color_scalefactors);

			for (j = 0; j < partition_count; j++)
			{
				inverse_color_scalefactors[j].x = 1.0f / MAX(color_scalefactors[j].x, 1e-7f);
				inverse_color_scalefactors[j].y = 1.0f / MAX(color_scalefactors[j].y, 1e-7f);
				inverse_color_scalefactors[j].z = 1.0f / MAX(color_scalefactors[j].z, 1e-7f);
				inverse_color_scalefactors[j].w = 1.0f / MAX(color_scalefactors[j].w, 1e-7f);
			}

			float3 averages[4];
			float3 directions_rgb[4];
			float2 directions_rg[4];
			float2 directions_rb[4];
			float2 directions_gb[4];

			compute_averages_and_directions_rgb(ptab + partition, pb, ewb, color_scalefactors, averages, directions_rgb, directions_rg, directions_rb, directions_gb);

			line3 uncorr_lines[4];
			line3 samechroma_lines[4];
			line2 separate_red_lines[4];
			line2 separate_green_lines[4];
			line2 separate_blue_lines[4];

			processed_line3 proc_uncorr_lines[4];
			processed_line3 proc_samechroma_lines[4];

			processed_line2 proc_separate_red_lines[4];
			processed_line2 proc_separate_green_lines[4];
			processed_line2 proc_separate_blue_lines[4];

			float uncorr_linelengths[4];
			float samechroma_linelengths[4];
			float separate_red_linelengths[4];
			float separate_green_linelengths[4];
			float separate_blue_linelengths[4];

			for (j = 0; j < partition_count; j++)
			{
				uncorr_lines[j].a = averages[j];
				if (dot(directions_rgb[j], directions_rgb[j]) == 0.0f)
					uncorr_lines[j].b = normalize(float3(1, 1, 1));
				else
					uncorr_lines[j].b = normalize(directions_rgb[j]);


				samechroma_lines[j].a = float3(0, 0, 0);

				if (dot(averages[j], averages[j]) == 0.0f)
					samechroma_lines[j].b = normalize(float3(1, 1, 1));
				else
					samechroma_lines[j].b = normalize(averages[j]);

				proc_uncorr_lines[j].amod = (uncorr_lines[j].a - uncorr_lines[j].b * dot(uncorr_lines[j].a, uncorr_lines[j].b)) * inverse_color_scalefactors[j].xyz;
				proc_uncorr_lines[j].bs = (uncorr_lines[j].b * color_scalefactors[j].xyz);
				proc_uncorr_lines[j].bis = (uncorr_lines[j].b * inverse_color_scalefactors[j].xyz);

				proc_samechroma_lines[j].amod = (samechroma_lines[j].a - samechroma_lines[j].b * dot(samechroma_lines[j].a, samechroma_lines[j].b)) * inverse_color_scalefactors[j].xyz;
				proc_samechroma_lines[j].bs = (samechroma_lines[j].b * color_scalefactors[j].xyz);
				proc_samechroma_lines[j].bis = (samechroma_lines[j].b * inverse_color_scalefactors[j].xyz);

				separate_red_lines[j].a = averages[j].yz;
				if (dot(directions_gb[j], directions_gb[j]) == 0.0f)
					separate_red_lines[j].b = normalize(float2(1, 1));
				else
					separate_red_lines[j].b = normalize(directions_gb[j]);

				separate_green_lines[j].a = averages[j].xz;
				if (dot(directions_rb[j], directions_rb[j]) == 0.0f)
					separate_green_lines[j].b = normalize(float2(1, 1));
				else
					separate_green_lines[j].b = normalize(directions_rb[j]);

				separate_blue_lines[j].a = averages[j].xy;
				if (dot(directions_rg[j], directions_rg[j]) == 0.0f)
					separate_blue_lines[j].b = normalize(float2(1, 1));
				else
					separate_blue_lines[j].b = normalize(directions_rg[j]);

				proc_separate_red_lines[j].amod = (separate_red_lines[j].a - separate_red_lines[j].b * dot(separate_red_lines[j].a, separate_red_lines[j].b)) * inverse_color_scalefactors[j].yz;
				proc_separate_red_lines[j].bs = (separate_red_lines[j].b * color_scalefactors[j].yz);
				proc_separate_red_lines[j].bis = (separate_red_lines[j].b * inverse_color_scalefactors[j].yz);

				proc_separate_green_lines[j].amod =
					(separate_green_lines[j].a - separate_green_lines[j].b * dot(separate_green_lines[j].a, separate_green_lines[j].b)) * inverse_color_scalefactors[j].xz;
				proc_separate_green_lines[j].bs = (separate_green_lines[j].b * color_scalefactors[j].xz);
				proc_separate_green_lines[j].bis = (separate_green_lines[j].b * inverse_color_scalefactors[j].xz);

				proc_separate_blue_lines[j].amod = (separate_blue_lines[j].a - separate_blue_lines[j].b * dot(separate_blue_lines[j].a, separate_blue_lines[j].b)) * inverse_color_scalefactors[j].xy;
				proc_separate_blue_lines[j].bs = (separate_blue_lines[j].b * color_scalefactors[j].xy);
				proc_separate_blue_lines[j].bis = (separate_blue_lines[j].b * inverse_color_scalefactors[j].xy);

			}

			float uncorr_error = compute_error_squared_rgb(ptab + partition,
														   pb,
														   ewb,
														   proc_uncorr_lines,
														   uncorr_linelengths);
			float samechroma_error = compute_error_squared_rgb(ptab + partition,
															   pb,
															   ewb,
															   proc_samechroma_lines,
															   samechroma_linelengths);

			float separate_red_error = compute_error_squared_gb(ptab + partition,
																pb,
																ewb,
																proc_separate_red_lines,
																separate_red_linelengths);

			float separate_green_error = compute_error_squared_rb(ptab + partition,
																  pb,
																  ewb,
																  proc_separate_green_lines,
																  separate_green_linelengths);

			float separate_blue_error = compute_error_squared_rg(ptab + partition,
																 pb,
																 ewb,
																 proc_separate_blue_lines,
																 separate_blue_linelengths);

			float red_min[4], red_max[4];
			float green_min[4], green_max[4];
			float blue_min[4], blue_max[4];


			compute_rgb_minmax(xdim, ydim, zdim, ptab + partition, pb, ewb, red_min, red_max, green_min, green_max, blue_min, blue_max);



			/*
			   compute an estimate of error introduced by weight imprecision.
			   This error is computed as follows, for each partition
			   1: compute the principal-axis vector (full length) in error-space
			   2: convert the principal-axis vector to regular RGB-space
			   3: scale the vector by a constant that estimates average quantization error.
			   4: for each texel, square the vector, then do a dot-product with the texel's error weight;
			      sum up the results across all texels.
			   4(optimized): square the vector once, then do a dot-product with the average texel error,
			     then multiply by the number of texels.
			 */


			for (j = 0; j < partition_count; j++)
			{
				float tpp = (float)(ptab[partition].texels_per_partition[j]);

				float3 ics = inverse_color_scalefactors[j].xyz;
				float3 error_weights = error_weightings[j].xyz * (tpp * weight_imprecision_estim_squared);

				float3 uncorr_vector = (uncorr_lines[j].b * uncorr_linelengths[j]) * ics;
				float3 samechroma_vector = (samechroma_lines[j].b * samechroma_linelengths[j]) * ics;

				float2 separate_red_vector = (separate_red_lines[j].b * separate_red_linelengths[j]) * ics.yz;
				float2 separate_green_vector = (separate_green_lines[j].b * separate_green_linelengths[j]) * ics.xz;
				float2 separate_blue_vector = (separate_blue_lines[j].b * separate_blue_linelengths[j]) * ics.xy;

				uncorr_vector = uncorr_vector * uncorr_vector;
				samechroma_vector = samechroma_vector * samechroma_vector;
				separate_red_vector = separate_red_vector * separate_red_vector;
				separate_green_vector = separate_green_vector * separate_green_vector;
				separate_blue_vector = separate_blue_vector * separate_blue_vector;

				uncorr_error += dot(uncorr_vector, error_weights);
				samechroma_error += dot(samechroma_vector, error_weights);
				separate_red_error += dot(separate_red_vector, error_weights.yz);
				separate_green_error += dot(separate_green_vector, error_weights.xz);
				separate_blue_error += dot(separate_blue_vector, error_weights.xy);

				float red_scalar = (red_max[j] - red_min[j]);
				float green_scalar = (green_max[j] - green_min[j]);
				float blue_scalar = (blue_max[j] - blue_min[j]);

				red_scalar *= red_scalar;
				green_scalar *= green_scalar;
				blue_scalar *= blue_scalar;

				separate_red_error += red_scalar * error_weights.x;
				separate_green_error += green_scalar * error_weights.y;
				separate_blue_error += blue_scalar * error_weights.z;
			}


			uncorr_errors[i] = uncorr_error;
			samechroma_errors[i] = samechroma_error;

			separate_red_errors[i] = separate_red_error;
			separate_green_errors[i] = separate_green_error;
			separate_blue_errors[i] = separate_blue_error;

			#ifdef DEBUG_PRINT_DIAGNOSTICS
				if (print_diagnostics)
					printf("Partitioning %d-%d errors: uncorr=%f, samechroma=%f, sep-red=%f, sep-green=%f, sep-blue=%f\n",
						   partition_count, partition, uncorr_error, samechroma_error, separate_red_error, separate_green_error, separate_blue_error);
			#endif
		}
	}


	for (i = 0; i < candidates_to_return; i++)
	{
		int best_uncorr_partition = 0;
		int best_samechroma_partition = 0;
		float best_uncorr_error = 1e30f;
		float best_samechroma_error = 1e30f;
		for (j = 0; j <= defacto_search_limit; j++)
		{
			if (uncorr_errors[j] < best_uncorr_error)
			{
				best_uncorr_partition = j;
				best_uncorr_error = uncorr_errors[j];
			}
		}
		best_partitions_uncorrellated[i] = partition_sequence[best_uncorr_partition];
		uncorr_errors[best_uncorr_partition] = 1e30f;
		samechroma_errors[best_uncorr_partition] = 1e30f;

		for (j = 0; j <= defacto_search_limit; j++)
		{
			if (samechroma_errors[j] < best_samechroma_error)
			{
				best_samechroma_partition = j;
				best_samechroma_error = samechroma_errors[j];
			}
		}
		best_partitions_samechroma[i] = partition_sequence[best_samechroma_partition];
		samechroma_errors[best_samechroma_partition] = 1e30f;
		uncorr_errors[best_samechroma_partition] = 1e30f;
	}

	for (i = 0; i < 2 * candidates_to_return; i++)
	{
		int best_partition = 0;
		float best_partition_error = 1e30f;

		for (j = 0; j <= defacto_search_limit; j++)
		{
			if (1 || !uses_alpha)
			{
				if (separate_errors[j] < best_partition_error)
				{
					best_partition = j;
					best_partition_error = separate_errors[j];
				}
				if (separate_errors[j + PARTITION_COUNT] < best_partition_error)
				{
					best_partition = j + PARTITION_COUNT;
					best_partition_error = separate_errors[j + PARTITION_COUNT];
				}
				if (separate_errors[j + 2 * PARTITION_COUNT] < best_partition_error)
				{
					best_partition = j + 2 * PARTITION_COUNT;
					best_partition_error = separate_errors[j + 2 * PARTITION_COUNT];
				}
			}
			if (uses_alpha)
			{
				if (separate_errors[j + 3 * PARTITION_COUNT] < best_partition_error)
				{
					best_partition = j + 3 * PARTITION_COUNT;
					best_partition_error = separate_errors[j + 3 * PARTITION_COUNT];
				}
			}
		}

		separate_errors[best_partition] = 1e30f;
		best_partition = ((best_partition >> PARTITION_BITS) << PARTITION_BITS) | partition_sequence[best_partition & (PARTITION_COUNT - 1)];
		best_partitions_dual_weight_planes[i] = best_partition;
	}

}
