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
 *	@brief	Compress a block of colors, expressed as a symbolic block, for ASTC.
 */ 
/*----------------------------------------------------------------------------*/ 

#include "astc_codec_internals.h"

#include "softfloat.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifdef DEBUG_CAPTURE_NAN
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif

	#include <fenv.h>
#endif

#include <stdio.h>

int realign_weights(astc_decode_mode decode_mode,
					int xdim, int ydim, int zdim, const imageblock * blk, const error_weight_block * ewb, symbolic_compressed_block * scb, uint8_t * weight_set8, uint8_t * plane2_weight_set8)
{
	int i, j;

	// get the appropriate partition descriptor.
	int partition_count = scb->partition_count;
	const partition_info *pt = get_partition_table(xdim, ydim, zdim, partition_count);
	pt += scb->partition_index;

	// get the appropriate block descriptor
	const block_size_descriptor *bsd = get_block_size_descriptor(xdim, ydim, zdim);
	const decimation_table *const *ixtab2 = bsd->decimation_tables;

	const decimation_table *it = ixtab2[bsd->block_modes[scb->block_mode].decimation_mode];

	int is_dual_plane = bsd->block_modes[scb->block_mode].is_dual_plane;

	// get quantization-parameters
	int weight_quantization_level = bsd->block_modes[scb->block_mode].quantization_mode;


	// decode the color endpoints
	ushort4 color_endpoint0[4];
	ushort4 color_endpoint1[4];
	int rgb_hdr[4];
	int alpha_hdr[4];
	int nan_endpoint[4];


	for (i = 0; i < partition_count; i++)
		unpack_color_endpoints(decode_mode,
							   scb->color_formats[i], scb->color_quantization_level, scb->color_values[i], &rgb_hdr[i], &alpha_hdr[i], &nan_endpoint[i], &(color_endpoint0[i]), &(color_endpoint1[i]));


	float uq_plane1_weights[MAX_WEIGHTS_PER_BLOCK];
	float uq_plane2_weights[MAX_WEIGHTS_PER_BLOCK];
	int weight_count = it->num_weights;

	// read and unquantize the weights.

	const quantization_and_transfer_table *qat = &(quant_and_xfer_tables[weight_quantization_level]);

	for (i = 0; i < weight_count; i++)
	{
		uq_plane1_weights[i] = qat->unquantized_value_flt[weight_set8[i]];
	}
	if (is_dual_plane)
	{
		for (i = 0; i < weight_count; i++)
			uq_plane2_weights[i] = qat->unquantized_value_flt[plane2_weight_set8[i]];
	}


	int plane2_color_component = is_dual_plane ? scb->plane2_color_component : -1;

	// for each weight, unquantize the weight, use it to compute a color and a color error.
	// then, increment the weight until the color error stops decreasing
	// then, decrement the weight until the color error stops increasing
	
	#define COMPUTE_ERROR( errorvar ) \
		errorvar = 0.0f; \
		for(j=0;j<texels_to_evaluate;j++) \
			{ \
			int texel = it->weight_texel[i][j]; \
			int partition = pt->partition_of_texel[texel]; \
			float plane1_weight = compute_value_of_texel_flt( texel, it, uq_plane1_weights ); \
			float plane2_weight = 0.0f; \
			if( is_dual_plane ) \
				plane2_weight = compute_value_of_texel_flt( texel, it, uq_plane2_weights ); \
			int int_plane1_weight = static_cast<int>(floor( plane1_weight*64.0f + 0.5f ) ); \
			int int_plane2_weight = static_cast<int>(floor( plane2_weight*64.0f + 0.5f ) ); \
			ushort4 lrp_color = lerp_color_int( \
				decode_mode, \
				color_endpoint0[partition], \
				color_endpoint1[partition], \
				int_plane1_weight, \
				int_plane2_weight, \
				plane2_color_component ); \
			float4 color = float4( lrp_color.x, lrp_color.y, lrp_color.z, lrp_color.w ); \
			float4 origcolor = float4( \
				blk->work_data[4*texel], \
				blk->work_data[4*texel+1], \
				blk->work_data[4*texel+2], \
				blk->work_data[4*texel+3] ); \
			float4 error_weight = ewb->error_weights[texel]; \
			float4 colordiff = color - origcolor; \
			errorvar += dot( colordiff*colordiff, error_weight ); \
			}


	int adjustments = 0;

	for (i = 0; i < weight_count; i++)
	{
		int current_wt = weight_set8[i];
		int texels_to_evaluate = it->weight_num_texels[i];

		float current_error;

		COMPUTE_ERROR(current_error);

		// increment until error starts increasing.
		while (1)
		{
			int next_wt = qat->next_quantized_value[current_wt];
			if (next_wt == current_wt)
				break;
			uq_plane1_weights[i] = qat->unquantized_value_flt[next_wt];
			float next_error;
			COMPUTE_ERROR(next_error);
			if (next_error < current_error)
			{
				// succeeded, increment the weight
				current_wt = next_wt;
				current_error = next_error;
				adjustments++;
			}
			else
			{
				// failed, back out the attempted increment
				uq_plane1_weights[i] = qat->unquantized_value_flt[current_wt];
				break;
			}
		}
		// decrement until error starts increasing
		while (1)
		{
			int prev_wt = qat->prev_quantized_value[current_wt];
			if (prev_wt == current_wt)
				break;
			uq_plane1_weights[i] = qat->unquantized_value_flt[prev_wt];
			float prev_error;
			COMPUTE_ERROR(prev_error);
			if (prev_error < current_error)
			{
				// succeeded, decrement the weight
				current_wt = prev_wt;
				current_error = prev_error;
				adjustments++;
			}
			else
			{
				// failed, back out the attempted decrement
				uq_plane1_weights[i] = qat->unquantized_value_flt[current_wt];
				break;
			}
		}

		weight_set8[i] = current_wt;
	}

	if (!is_dual_plane)
		return adjustments;

	// processing of the second plane of weights
	for (i = 0; i < weight_count; i++)
	{
		int current_wt = plane2_weight_set8[i];
		int texels_to_evaluate = it->weight_num_texels[i];

		float current_error;

		COMPUTE_ERROR(current_error);

		// increment until error starts increasing.
		while (1)
		{
			int next_wt = qat->next_quantized_value[current_wt];
			if (next_wt == current_wt)
				break;
			uq_plane2_weights[i] = qat->unquantized_value_flt[next_wt];
			float next_error;
			COMPUTE_ERROR(next_error);
			if (next_error < current_error)
			{
				// succeeded, increment the weight
				current_wt = next_wt;
				current_error = next_error;
				adjustments++;
			}
			else
			{
				// failed, back out the attempted increment
				uq_plane2_weights[i] = qat->unquantized_value_flt[current_wt];
				break;
			}
		}
		// decrement until error starts increasing
		while (1)
		{
			int prev_wt = qat->prev_quantized_value[current_wt];
			if (prev_wt == current_wt)
				break;
			uq_plane2_weights[i] = qat->unquantized_value_flt[prev_wt];
			float prev_error;
			COMPUTE_ERROR(prev_error);
			if (prev_error < current_error)
			{
				// succeeded, decrement the weight
				current_wt = prev_wt;
				current_error = prev_error;
				adjustments++;
			}
			else
			{
				// failed, back out the attempted decrement
				uq_plane2_weights[i] = qat->unquantized_value_flt[current_wt];
				break;
			}
		}

		plane2_weight_set8[i] = current_wt;
	}

	return adjustments;
}

/*
	function for compressing a block symbolically, given that we have already decided on a partition 
*/



static void compress_symbolic_block_fixed_partition_1_plane(astc_decode_mode decode_mode,
															float mode_cutoff,
															int max_refinement_iters,
															int xdim, int ydim, int zdim,
															int partition_count, int partition_index,
															const imageblock * blk, const error_weight_block * ewb, symbolic_compressed_block * scb,
															compress_fixed_partition_buffers * tmpbuf)
{
	int i, j, k;


	static const int free_bits_for_partition_count[5] = { 0, 115 - 4, 111 - 4 - PARTITION_BITS, 108 - 4 - PARTITION_BITS, 105 - 4 - PARTITION_BITS };

	const partition_info *pi = get_partition_table(xdim, ydim, zdim, partition_count);
	pi += partition_index;

	// first, compute ideal weights and endpoint colors, under thre assumption that
	// there is no quantization or decimation going on.
	endpoints_and_weights *ei = tmpbuf->ei1;
	endpoints_and_weights *eix = tmpbuf->eix1;
	compute_endpoints_and_ideal_weights_1_plane(xdim, ydim, zdim, pi, blk, ewb, ei);

	// next, compute ideal weights and endpoint colors for every decimation.
	const block_size_descriptor *bsd = get_block_size_descriptor(xdim, ydim, zdim);
	const decimation_table *const *ixtab2 = bsd->decimation_tables;
	// int block_mode_count = bsd->single_plane_block_mode_count;


	float *decimated_quantized_weights = tmpbuf->decimated_quantized_weights;
	float *decimated_weights = tmpbuf->decimated_weights;
	float *flt_quantized_decimated_quantized_weights = tmpbuf->flt_quantized_decimated_quantized_weights;
	uint8_t *u8_quantized_decimated_quantized_weights = tmpbuf->u8_quantized_decimated_quantized_weights;

	// for each decimation mode, compute an ideal set of weights
	// (that is, weights computed with the assumption that they are not quantized)
	for (i = 0; i < MAX_DECIMATION_MODES; i++)
	{
		if (bsd->permit_encode[i] == 0 || bsd->decimation_mode_maxprec_1plane[i] < 0 || bsd->decimation_mode_percentile[i] > mode_cutoff)
			continue;
		eix[i] = *ei;
		compute_ideal_weights_for_decimation_table(&(eix[i]), ixtab2[i], decimated_quantized_weights + i * MAX_WEIGHTS_PER_BLOCK, decimated_weights + i * MAX_WEIGHTS_PER_BLOCK);

	}

	// compute maximum colors for the endpoints and ideal weights.
	// for each endpoint-and-ideal-weight pair, compute the smallest weight value
	// that will result in a color value greater than 1.


	float4 min_ep = float4(10, 10, 10, 10);
	for (i = 0; i < partition_count; i++)
	{
		#ifdef DEBUG_CAPTURE_NAN
			fedisableexcept(FE_DIVBYZERO | FE_INVALID);
		#endif

		float4 ep = (float4(1, 1, 1, 1) - ei->ep.endpt0[i]) / (ei->ep.endpt1[i] - ei->ep.endpt0[i]);
		if (ep.x > 0.5f && ep.x < min_ep.x)
			min_ep.x = ep.x;
		if (ep.y > 0.5f && ep.y < min_ep.y)
			min_ep.y = ep.y;
		if (ep.z > 0.5f && ep.z < min_ep.z)
			min_ep.z = ep.z;
		if (ep.w > 0.5f && ep.w < min_ep.w)
			min_ep.w = ep.w;

		#ifdef DEBUG_CAPTURE_NAN
			feenableexcept(FE_DIVBYZERO | FE_INVALID);
		#endif
	}

	float min_wt_cutoff = MIN(MIN(min_ep.x, min_ep.y), MIN(min_ep.z, min_ep.w));

	// for each mode, use the angular method to compute a shift.
	float weight_low_value[MAX_WEIGHT_MODES];
	float weight_high_value[MAX_WEIGHT_MODES];

	compute_angular_endpoints_1plane(mode_cutoff, bsd, decimated_quantized_weights, decimated_weights, weight_low_value, weight_high_value);

	// for each mode (which specifies a decimation and a quantization):
	// * compute number of bits needed for the quantized weights.
	// * generate an optimized set of quantized weights.
	// * compute quantization errors for the mode.

	int qwt_bitcounts[MAX_WEIGHT_MODES];
	float qwt_errors[MAX_WEIGHT_MODES];

	for (i = 0; i < MAX_WEIGHT_MODES; i++)
	{
		if (bsd->block_modes[i].permit_encode == 0 || bsd->block_modes[i].is_dual_plane != 0 || bsd->block_modes[i].percentile > mode_cutoff)
		{
			qwt_errors[i] = 1e38f;
			continue;
		}
		if (weight_high_value[i] > 1.02f * min_wt_cutoff)
			weight_high_value[i] = 1.0f;

		int decimation_mode = bsd->block_modes[i].decimation_mode;
		if (bsd->decimation_mode_percentile[decimation_mode] > mode_cutoff)
			ASTC_CODEC_INTERNAL_ERROR;


		// compute weight bitcount for the mode
		int bits_used_by_weights = compute_ise_bitcount(ixtab2[decimation_mode]->num_weights,
														(quantization_method) bsd->block_modes[i].quantization_mode);
		int bitcount = free_bits_for_partition_count[partition_count] - bits_used_by_weights;
		if (bitcount <= 0 || bits_used_by_weights < 24 || bits_used_by_weights > 96)
		{
			qwt_errors[i] = 1e38f;
			continue;
		}
		qwt_bitcounts[i] = bitcount;


		// then, generate the optimized set of weights for the weight mode.
		compute_ideal_quantized_weights_for_decimation_table(&(eix[decimation_mode]),
															 ixtab2[decimation_mode],
															 weight_low_value[i], weight_high_value[i],
															 decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * decimation_mode,
															 flt_quantized_decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * i,
															 u8_quantized_decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * i,
															 bsd->block_modes[i].quantization_mode);

		// then, compute weight-errors for the weight mode.
		qwt_errors[i] = compute_error_of_weight_set(&(eix[decimation_mode]), ixtab2[decimation_mode], flt_quantized_decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * i);

		#ifdef DEBUG_PRINT_DIAGNOSTICS
			if (print_diagnostics)
				printf("Block mode %d -> weight error = %f\n", i, qwt_errors[i]);
		#endif
	}

	// for each weighting mode, determine the optimal combination of color endpoint encodings
	// and weight encodings; return results for the 4 best-looking modes.

	int partition_format_specifiers[4][4];
	int quantized_weight[4];
	int color_quantization_level[4];
	int color_quantization_level_mod[4];
	determine_optimal_set_of_endpoint_formats_to_use(xdim, ydim, zdim, pi, blk, ewb, &(ei->ep), -1,	// used to flag that we are in single-weight mode
													 qwt_bitcounts, qwt_errors, partition_format_specifiers, quantized_weight, color_quantization_level, color_quantization_level_mod);


	// then iterate over the 4 believed-to-be-best modes to find out which one is
	// actually best.
	for (i = 0; i < 4; i++)
	{
		uint8_t *u8_weight_src;
		int weights_to_copy;

		if (quantized_weight[i] < 0)
		{
			scb->error_block = 1;
			scb++;
			continue;
		}

		int decimation_mode = bsd->block_modes[quantized_weight[i]].decimation_mode;
		int weight_quantization_mode = bsd->block_modes[quantized_weight[i]].quantization_mode;
		const decimation_table *it = ixtab2[decimation_mode];

		#ifdef DEBUG_PRINT_DIAGNOSTICS
			if (print_diagnostics)
			{
				printf("Selected mode = %d\n", quantized_weight[i]);
				printf("Selected decimation mode = %d\n", decimation_mode);
				printf("Selected weight-quantization mode = %d\n", weight_quantization_mode);
			}
		#endif

		u8_weight_src = u8_quantized_decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * quantized_weight[i];

		weights_to_copy = it->num_weights;

		// recompute the ideal color endpoints before storing them.
		float4 rgbs_colors[4];
		float4 rgbo_colors[4];
		float2 lum_intervals[4];

		int l;
		for (l = 0; l < max_refinement_iters; l++)
		{
			recompute_ideal_colors(xdim, ydim, zdim, weight_quantization_mode, &(eix[decimation_mode].ep), rgbs_colors, rgbo_colors, lum_intervals, u8_weight_src, NULL, -1, pi, it, blk, ewb);

			// quantize the chosen color

			// store the colors for the block
			for (j = 0; j < partition_count; j++)
			{
				scb->color_formats[j] = pack_color_endpoints(decode_mode,
															 eix[decimation_mode].ep.endpt0[j],
															 eix[decimation_mode].ep.endpt1[j],
															 rgbs_colors[j], rgbo_colors[j], lum_intervals[j], partition_format_specifiers[i][j], scb->color_values[j], color_quantization_level[i]);
			}


			// if all the color endpoint modes are the same, we get a few more
			// bits to store colors; let's see if we can take advantage of this:
			// requantize all the colors and see if the endpoint modes remain the same;
			// if they do, then exploit it.
			scb->color_formats_matched = 0;

			if ((partition_count >= 2 && scb->color_formats[0] == scb->color_formats[1]
				 && color_quantization_level != color_quantization_level_mod)
				&& (partition_count == 2 || (scb->color_formats[0] == scb->color_formats[2] && (partition_count == 3 || (scb->color_formats[0] == scb->color_formats[3])))))
			{
				int colorvals[4][12];
				int color_formats_mod[4];
				for (j = 0; j < partition_count; j++)
				{
					color_formats_mod[j] = pack_color_endpoints(decode_mode,
																eix[decimation_mode].ep.endpt0[j],
																eix[decimation_mode].ep.endpt1[j],
																rgbs_colors[j], rgbo_colors[j], lum_intervals[j], partition_format_specifiers[i][j], colorvals[j], color_quantization_level_mod[i]);
				}
				if (color_formats_mod[0] == color_formats_mod[1]
					&& (partition_count == 2 || (color_formats_mod[0] == color_formats_mod[2] && (partition_count == 3 || (color_formats_mod[0] == color_formats_mod[3])))))
				{
					scb->color_formats_matched = 1;
					for (j = 0; j < 4; j++)
						for (k = 0; k < 12; k++)
							scb->color_values[j][k] = colorvals[j][k];
					for (j = 0; j < 4; j++)
						scb->color_formats[j] = color_formats_mod[j];
				}
			}


			// store header fields
			scb->partition_count = partition_count;
			scb->partition_index = partition_index;
			scb->color_quantization_level = scb->color_formats_matched ? color_quantization_level_mod[i] : color_quantization_level[i];
			scb->block_mode = quantized_weight[i];
			scb->error_block = 0;

			if (scb->color_quantization_level < 4)
			{
				scb->error_block = 1;	// should never happen, but cannot prove it impossible.
			}

			// perform a final pass over the weights to try to improve them.
			int adjustments = realign_weights(decode_mode,
											  xdim, ydim, zdim,
											  blk, ewb, scb,
											  u8_weight_src,
											  NULL);

			if (adjustments == 0)
				break;
		}

		for (j = 0; j < weights_to_copy; j++)
			scb->plane1_weights[j] = u8_weight_src[j];

		scb++;
	}

}






static void compress_symbolic_block_fixed_partition_2_planes(astc_decode_mode decode_mode,
															 float mode_cutoff,
															 int max_refinement_iters,
															 int xdim, int ydim, int zdim,
															 int partition_count, int partition_index,
															 int separate_component, const imageblock * blk, const error_weight_block * ewb,
															 symbolic_compressed_block * scb,
															 compress_fixed_partition_buffers * tmpbuf)
{
	int i, j, k;

	static const int free_bits_for_partition_count[5] =
		{ 0, 113 - 4, 109 - 4 - PARTITION_BITS, 106 - 4 - PARTITION_BITS, 103 - 4 - PARTITION_BITS };

	const partition_info *pi = get_partition_table(xdim, ydim, zdim, partition_count);
	pi += partition_index;

	// first, compute ideal weights and endpoint colors
	endpoints_and_weights *ei1 = tmpbuf->ei1;
	endpoints_and_weights *ei2 = tmpbuf->ei2;
	endpoints_and_weights *eix1 = tmpbuf->eix1;
	endpoints_and_weights *eix2 = tmpbuf->eix2;
	compute_endpoints_and_ideal_weights_2_planes(xdim, ydim, zdim, pi, blk, ewb, separate_component, ei1, ei2);

	// next, compute ideal weights and endpoint colors for every decimation.
	const block_size_descriptor *bsd = get_block_size_descriptor(xdim, ydim, zdim);
	const decimation_table *const *ixtab2 = bsd->decimation_tables;


	float *decimated_quantized_weights = tmpbuf->decimated_quantized_weights;
	float *decimated_weights = tmpbuf->decimated_weights;
	float *flt_quantized_decimated_quantized_weights = tmpbuf->flt_quantized_decimated_quantized_weights;
	uint8_t *u8_quantized_decimated_quantized_weights = tmpbuf->u8_quantized_decimated_quantized_weights;

	// for each decimation mode, compute an ideal set of weights
	for (i = 0; i < MAX_DECIMATION_MODES; i++)
	{
		if (bsd->permit_encode[i] == 0 || bsd->decimation_mode_maxprec_2planes[i] < 0 || bsd->decimation_mode_percentile[i] > mode_cutoff)
			continue;

		eix1[i] = *ei1;
		eix2[i] = *ei2;
		compute_ideal_weights_for_decimation_table(&(eix1[i]), ixtab2[i], decimated_quantized_weights + (2 * i) * MAX_WEIGHTS_PER_BLOCK, decimated_weights + (2 * i) * MAX_WEIGHTS_PER_BLOCK);
		compute_ideal_weights_for_decimation_table(&(eix2[i]), ixtab2[i], decimated_quantized_weights + (2 * i + 1) * MAX_WEIGHTS_PER_BLOCK, decimated_weights + (2 * i + 1) * MAX_WEIGHTS_PER_BLOCK);
	}

	// compute maximum colors for the endpoints and ideal weights.
	// for each endpoint-and-ideal-weight pair, compute the smallest weight value
	// that will result in a color value greater than 1.

	float4 min_ep1 = float4(10, 10, 10, 10);
	float4 min_ep2 = float4(10, 10, 10, 10);
	for (i = 0; i < partition_count; i++)
	{

		#ifdef DEBUG_CAPTURE_NAN
			fedisableexcept(FE_DIVBYZERO | FE_INVALID);
		#endif

		float4 ep1 = (float4(1, 1, 1, 1) - ei1->ep.endpt0[i]) / (ei1->ep.endpt1[i] - ei1->ep.endpt0[i]);
		if (ep1.x > 0.5f && ep1.x < min_ep1.x)
			min_ep1.x = ep1.x;
		if (ep1.y > 0.5f && ep1.y < min_ep1.y)
			min_ep1.y = ep1.y;
		if (ep1.z > 0.5f && ep1.z < min_ep1.z)
			min_ep1.z = ep1.z;
		if (ep1.w > 0.5f && ep1.w < min_ep1.w)
			min_ep1.w = ep1.w;
		float4 ep2 = (float4(1, 1, 1, 1) - ei2->ep.endpt0[i]) / (ei2->ep.endpt1[i] - ei2->ep.endpt0[i]);
		if (ep2.x > 0.5f && ep2.x < min_ep2.x)
			min_ep2.x = ep2.x;
		if (ep2.y > 0.5f && ep2.y < min_ep2.y)
			min_ep2.y = ep2.y;
		if (ep2.z > 0.5f && ep2.z < min_ep2.z)
			min_ep2.z = ep2.z;
		if (ep2.w > 0.5f && ep2.w < min_ep2.w)
			min_ep2.w = ep2.w;

		#ifdef DEBUG_CAPTURE_NAN
			feenableexcept(FE_DIVBYZERO | FE_INVALID);
		#endif
	}

	float min_wt_cutoff1, min_wt_cutoff2;
	switch (separate_component)
	{
	case 0:
		min_wt_cutoff2 = min_ep2.x;
		min_ep1.x = 1e30f;
		break;
	case 1:
		min_wt_cutoff2 = min_ep2.y;
		min_ep1.y = 1e30f;
		break;
	case 2:
		min_wt_cutoff2 = min_ep2.z;
		min_ep1.z = 1e30f;
		break;
	case 3:
		min_wt_cutoff2 = min_ep2.w;
		min_ep1.w = 1e30f;
		break;
	default:
		min_wt_cutoff2 = 1e30f;
	}

	min_wt_cutoff1 = MIN(MIN(min_ep1.x, min_ep1.y), MIN(min_ep1.z, min_ep1.w));

	float weight_low_value1[MAX_WEIGHT_MODES];
	float weight_high_value1[MAX_WEIGHT_MODES];
	float weight_low_value2[MAX_WEIGHT_MODES];
	float weight_high_value2[MAX_WEIGHT_MODES];

	compute_angular_endpoints_2planes(mode_cutoff, bsd, decimated_quantized_weights, decimated_weights, weight_low_value1, weight_high_value1, weight_low_value2, weight_high_value2);

	// for each mode (which specifies a decimation and a quantization):
	// * generate an optimized set of quantized weights.
	// * compute quantization errors for each mode
	// * compute number of bits needed for the quantized weights.

	int qwt_bitcounts[MAX_WEIGHT_MODES];
	float qwt_errors[MAX_WEIGHT_MODES];
	for (i = 0; i < MAX_WEIGHT_MODES; i++)
	{
		if (bsd->block_modes[i].permit_encode == 0 || bsd->block_modes[i].is_dual_plane != 1 || bsd->block_modes[i].percentile > mode_cutoff)
		{
			qwt_errors[i] = 1e38f;
			continue;
		}
		int decimation_mode = bsd->block_modes[i].decimation_mode;

		if (weight_high_value1[i] > 1.02f * min_wt_cutoff1)
			weight_high_value1[i] = 1.0f;
		if (weight_high_value2[i] > 1.02f * min_wt_cutoff2)
			weight_high_value2[i] = 1.0f;

		// compute weight bitcount for the mode
		int bits_used_by_weights = compute_ise_bitcount(2 * ixtab2[decimation_mode]->num_weights,
														(quantization_method) bsd->block_modes[i].quantization_mode);
		int bitcount = free_bits_for_partition_count[partition_count] - bits_used_by_weights;
		if (bitcount <= 0 || bits_used_by_weights < 24 || bits_used_by_weights > 96)
		{
			qwt_errors[i] = 1e38f;
			continue;
		}
		qwt_bitcounts[i] = bitcount;


		// then, generate the optimized set of weights for the mode.
		compute_ideal_quantized_weights_for_decimation_table(&(eix1[decimation_mode]),
															 ixtab2[decimation_mode],
															 weight_low_value1[i],
															 weight_high_value1[i],
															 decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * (2 * decimation_mode),
															 flt_quantized_decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * (2 * i),
															 u8_quantized_decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * (2 * i), bsd->block_modes[i].quantization_mode);
		compute_ideal_quantized_weights_for_decimation_table(&(eix2[decimation_mode]),
															 ixtab2[decimation_mode],
															 weight_low_value2[i],
															 weight_high_value2[i],
															 decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * (2 * decimation_mode + 1),
															 flt_quantized_decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * (2 * i + 1),
															 u8_quantized_decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * (2 * i + 1), bsd->block_modes[i].quantization_mode);


		// then, compute quantization errors for the block mode.
		qwt_errors[i] =
			compute_error_of_weight_set(&(eix1[decimation_mode]),
									   ixtab2[decimation_mode],
									   flt_quantized_decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * (2 * i))
			+ compute_error_of_weight_set(&(eix2[decimation_mode]), ixtab2[decimation_mode], flt_quantized_decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * (2 * i + 1));
	}


	// decide the optimal combination of color endpoint encodings and weight encoodings.
	int partition_format_specifiers[4][4];
	int quantized_weight[4];
	int color_quantization_level[4];
	int color_quantization_level_mod[4];

	endpoints epm;
	merge_endpoints(&(ei1->ep), &(ei2->ep), separate_component, &epm);

	determine_optimal_set_of_endpoint_formats_to_use(xdim, ydim, zdim,
													 pi,
													 blk,
													 ewb,
													 &epm, separate_component, qwt_bitcounts, qwt_errors, partition_format_specifiers, quantized_weight, color_quantization_level, color_quantization_level_mod);

	for (i = 0; i < 4; i++)
	{
		if (quantized_weight[i] < 0)
		{
			scb->error_block = 1;
			scb++;
			continue;
		}

		uint8_t *u8_weight1_src;
		uint8_t *u8_weight2_src;
		int weights_to_copy;

		int decimation_mode = bsd->block_modes[quantized_weight[i]].decimation_mode;
		int weight_quantization_mode = bsd->block_modes[quantized_weight[i]].quantization_mode;
		const decimation_table *it = ixtab2[decimation_mode];

		u8_weight1_src = u8_quantized_decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * (2 * quantized_weight[i]);
		u8_weight2_src = u8_quantized_decimated_quantized_weights + MAX_WEIGHTS_PER_BLOCK * (2 * quantized_weight[i] + 1);


		weights_to_copy = it->num_weights;

		// recompute the ideal color endpoints before storing them.
		merge_endpoints(&(eix1[decimation_mode].ep), &(eix2[decimation_mode].ep), separate_component, &epm);

		float4 rgbs_colors[4];
		float4 rgbo_colors[4];
		float2 lum_intervals[4];

		int l;
		for (l = 0; l < max_refinement_iters; l++)
		{
			recompute_ideal_colors(xdim, ydim, zdim, weight_quantization_mode, &epm, rgbs_colors, rgbo_colors, lum_intervals, u8_weight1_src, u8_weight2_src, separate_component, pi, it, blk, ewb);

			// store the colors for the block
			for (j = 0; j < partition_count; j++)
			{
				scb->color_formats[j] = pack_color_endpoints(decode_mode,
															 epm.endpt0[j],
															 epm.endpt1[j],
															 rgbs_colors[j], rgbo_colors[j], lum_intervals[j], partition_format_specifiers[i][j], scb->color_values[j], color_quantization_level[i]);
			}
			scb->color_formats_matched = 0;

			if ((partition_count >= 2 && scb->color_formats[0] == scb->color_formats[1]
				 && color_quantization_level != color_quantization_level_mod)
				&& (partition_count == 2 || (scb->color_formats[0] == scb->color_formats[2] && (partition_count == 3 || (scb->color_formats[0] == scb->color_formats[3])))))
			{
				int colorvals[4][12];
				int color_formats_mod[4];
				for (j = 0; j < partition_count; j++)
				{
					color_formats_mod[j] = pack_color_endpoints(decode_mode,
																epm.endpt0[j],
																epm.endpt1[j],
																rgbs_colors[j], rgbo_colors[j], lum_intervals[j], partition_format_specifiers[i][j], colorvals[j], color_quantization_level_mod[i]);
				}
				if (color_formats_mod[0] == color_formats_mod[1]
					&& (partition_count == 2 || (color_formats_mod[0] == color_formats_mod[2] && (partition_count == 3 || (color_formats_mod[0] == color_formats_mod[3])))))
				{
					scb->color_formats_matched = 1;
					for (j = 0; j < 4; j++)
						for (k = 0; k < 12; k++)
							scb->color_values[j][k] = colorvals[j][k];
					for (j = 0; j < 4; j++)
						scb->color_formats[j] = color_formats_mod[j];
				}
			}


			// store header fields
			scb->partition_count = partition_count;
			scb->partition_index = partition_index;
			scb->color_quantization_level = scb->color_formats_matched ? color_quantization_level_mod[i] : color_quantization_level[i];
			scb->block_mode = quantized_weight[i];
			scb->plane2_color_component = separate_component;
			scb->error_block = 0;

			if (scb->color_quantization_level < 4)
			{
				scb->error_block = 1;	// should never happen, but cannot prove it impossible
			}

			int adjustments = realign_weights(decode_mode,
											  xdim, ydim, zdim,
											  blk, ewb, scb,
											  u8_weight1_src,
											  u8_weight2_src);

			if (adjustments == 0)
				break;
		}

		for (j = 0; j < weights_to_copy; j++)
		{
			scb->plane1_weights[j] = u8_weight1_src[j];
			scb->plane2_weights[j] = u8_weight2_src[j];
		}

		scb++;
	}

}





void expand_block_artifact_suppression(int xdim, int ydim, int zdim, error_weighting_params * ewp)
{
	int x, y, z;
	float centerpos_x = (xdim - 1) * 0.5f;
	float centerpos_y = (ydim - 1) * 0.5f;
	float centerpos_z = (zdim - 1) * 0.5f;
	float *bef = ewp->block_artifact_suppression_expanded;

	for (z = 0; z < zdim; z++)
		for (y = 0; y < ydim; y++)
			for (x = 0; x < xdim; x++)
			{
				float xdif = (x - centerpos_x) / xdim;
				float ydif = (y - centerpos_y) / ydim;
				float zdif = (z - centerpos_z) / zdim;

				float wdif = 0.36f;
				float dist = sqrt(xdif * xdif + ydif * ydif + zdif * zdif + wdif * wdif);
				*bef = pow(dist, ewp->block_artifact_suppression);
				bef++;
			}
}



// Function to set error weights for each color component for each texel in a block.
// Returns the sum of all the error values set.

float prepare_error_weight_block(const astc_codec_image * input_image,
								 int xdim, int ydim, int zdim, const error_weighting_params * ewp, const imageblock * blk, error_weight_block * ewb, error_weight_block_orig * ewbo)
{

	int x, y, z;
	int idx = 0;

	int any_mean_stdev_weight =
		ewp->rgb_base_weight != 1.0 || ewp->alpha_base_weight != 1.0 || ewp->rgb_mean_weight != 0.0 || ewp->rgb_stdev_weight != 0.0 || ewp->alpha_mean_weight != 0.0 || ewp->alpha_stdev_weight != 0.0;

	float4 color_weights = float4(ewp->rgba_weights[0],
								  ewp->rgba_weights[1],
								  ewp->rgba_weights[2],
								  ewp->rgba_weights[3]);

	ewb->contains_zeroweight_texels = 0;

	for (z = 0; z < zdim; z++)
		for (y = 0; y < ydim; y++)
			for (x = 0; x < xdim; x++)
			{
				int xpos = x + blk->xpos;
				int ypos = y + blk->ypos;
				int zpos = z + blk->zpos;

				if (xpos >= input_image->xsize || ypos >= input_image->ysize || zpos >= input_image->zsize)
				{
					float4 weights = float4(1e-11f, 1e-11f, 1e-11f, 1e-11f);
					ewb->error_weights[idx] = weights;
					ewb->contains_zeroweight_texels = 1;
				}
				else
				{
					float4 error_weight = float4(ewp->rgb_base_weight,
												 ewp->rgb_base_weight,
												 ewp->rgb_base_weight,
												 ewp->alpha_base_weight);

					if (any_mean_stdev_weight)
					{
						float4 avg = input_averages[zpos][ypos][xpos];
						if (avg.x < 6e-5f)
							avg.x = 6e-5f;
						if (avg.y < 6e-5f)
							avg.y = 6e-5f;
						if (avg.z < 6e-5f)
							avg.z = 6e-5f;
						if (avg.w < 6e-5f)
							avg.w = 6e-5f;
						/* 
						   printf("avg: %f %f %f %f\n", avg.x, avg.y, avg.z, avg.w ); */
						avg = avg * avg;

						float4 variance = input_variances[zpos][ypos][xpos];
						variance = variance * variance;

						float favg = (avg.x + avg.y + avg.z) * (1.0f / 3.0f);
						float fvar = (variance.x + variance.y + variance.z) * (1.0f / 3.0f);

						float mixing = ewp->rgb_mean_and_stdev_mixing;
						avg.xyz = float3(favg, favg, favg) * mixing + avg.xyz * (1.0f - mixing);
						variance.xyz = float3(fvar, fvar, fvar) * mixing + variance.xyz * (1.0f - mixing);

						float4 stdev = float4(sqrt(MAX(variance.x, 0.0f)),
											  sqrt(MAX(variance.y, 0.0f)),
											  sqrt(MAX(variance.z, 0.0f)),
											  sqrt(MAX(variance.w, 0.0f)));

						avg.xyz = avg.xyz * ewp->rgb_mean_weight;
						avg.w = avg.w * ewp->alpha_mean_weight;
						stdev.xyz = stdev.xyz * ewp->rgb_stdev_weight;
						stdev.w = stdev.w * ewp->alpha_stdev_weight;
						error_weight = error_weight + avg + stdev;

						error_weight = float4(1.0f, 1.0f, 1.0f, 1.0f) / error_weight;
					}

					if (ewp->ra_normal_angular_scale)
					{
						float x = (blk->orig_data[4 * idx] - 0.5f) * 2.0f;
						float y = (blk->orig_data[4 * idx + 3] - 0.5f) * 2.0f;
						float denom = 1.0f - x * x - y * y;
						if (denom < 0.1f)
							denom = 0.1f;
						denom = 1.0f / denom;
						error_weight.x *= 1.0f + x * x * denom;
						error_weight.w *= 1.0f + y * y * denom;
					}

					if (ewp->enable_rgb_scale_with_alpha)
					{
						float alpha_scale;
						if (ewp->alpha_radius != 0)
							alpha_scale = input_alpha_averages[zpos][ypos][xpos];
						else
							alpha_scale = blk->orig_data[4 * idx + 3];
						if (alpha_scale < 0.0001f)
							alpha_scale = 0.0001f;
						alpha_scale *= alpha_scale;
						error_weight.xyz = error_weight.xyz * alpha_scale;
					}
					error_weight = error_weight * color_weights;
					error_weight = error_weight * ewp->block_artifact_suppression_expanded[idx];

					// if we perform a conversion from linear to sRGB, then we multiply
					// the weight with the derivative of the linear->sRGB transform function.
					if (perform_srgb_transform)
					{
						float r = blk->orig_data[4 * idx];
						float g = blk->orig_data[4 * idx + 1];
						float b = blk->orig_data[4 * idx + 2];
						if (r < 0.0031308f)
							r = 12.92f;
						else
							r = 0.4396f * pow(r, -0.58333f);
						if (g < 0.0031308f)
							g = 12.92f;
						else
							g = 0.4396f * pow(g, -0.58333f);
						if (b < 0.0031308f)
							b = 12.92f;
						else
							b = 0.4396f * pow(b, -0.58333f);
						error_weight.x *= r;
						error_weight.y *= g;
						error_weight.z *= b;
					}

					/*
						printf("%f %f %f %f\n", error_weight.x, error_weight.y, error_weight.z, error_weight.w );
					*/

					// when we loaded the block to begin with, we applied a transfer function
					// and computed the derivative of the transfer function. However, the
					// error-weight computation so far is based on the original color values,
					// not the transfer-function values. As such, we must multiply the
					// error weights by the derivative of the inverse of the transfer function,
					// which is equivalent to dividing by the derivative of the transfer
					// function.

					ewbo->error_weights[idx] = error_weight;

					error_weight.x /= (blk->deriv_data[4 * idx] * blk->deriv_data[4 * idx] * 1e-10f);
					error_weight.y /= (blk->deriv_data[4 * idx + 1] * blk->deriv_data[4 * idx + 1] * 1e-10f);
					error_weight.z /= (blk->deriv_data[4 * idx + 2] * blk->deriv_data[4 * idx + 2] * 1e-10f);
					error_weight.w /= (blk->deriv_data[4 * idx + 3] * blk->deriv_data[4 * idx + 3] * 1e-10f);

					/*
						printf("--> %f %f %f %f\n", error_weight.x, error_weight.y, error_weight.z, error_weight.w );
					*/

					ewb->error_weights[idx] = error_weight;
					if (dot(error_weight, float4(1, 1, 1, 1)) < 1e-10f)
						ewb->contains_zeroweight_texels = 1;
				}
				idx++;
			}

	int i;

	float4 error_weight_sum = float4(0, 0, 0, 0);
	int texels_per_block = xdim * ydim * zdim;

	for (i = 0; i < texels_per_block; i++)
	{
		error_weight_sum = error_weight_sum + ewb->error_weights[i];

		ewb->texel_weight_r[i] = ewb->error_weights[i].x;
		ewb->texel_weight_g[i] = ewb->error_weights[i].y;
		ewb->texel_weight_b[i] = ewb->error_weights[i].z;
		ewb->texel_weight_a[i] = ewb->error_weights[i].w;

		ewb->texel_weight_rg[i] = (ewb->error_weights[i].x + ewb->error_weights[i].y) * 0.5f;
		ewb->texel_weight_rb[i] = (ewb->error_weights[i].x + ewb->error_weights[i].z) * 0.5f;
		ewb->texel_weight_gb[i] = (ewb->error_weights[i].y + ewb->error_weights[i].z) * 0.5f;
		ewb->texel_weight_ra[i] = (ewb->error_weights[i].x + ewb->error_weights[i].w) * 0.5f;

		ewb->texel_weight_gba[i] = (ewb->error_weights[i].y + ewb->error_weights[i].z + ewb->error_weights[i].w) * 0.333333f;
		ewb->texel_weight_rba[i] = (ewb->error_weights[i].x + ewb->error_weights[i].z + ewb->error_weights[i].w) * 0.333333f;
		ewb->texel_weight_rga[i] = (ewb->error_weights[i].x + ewb->error_weights[i].y + ewb->error_weights[i].w) * 0.333333f;
		ewb->texel_weight_rgb[i] = (ewb->error_weights[i].x + ewb->error_weights[i].y + ewb->error_weights[i].z) * 0.333333f;
		ewb->texel_weight[i] = (ewb->error_weights[i].x + ewb->error_weights[i].y + ewb->error_weights[i].z + ewb->error_weights[i].w) * 0.25f;
	}

	return dot(error_weight_sum, float4(1, 1, 1, 1));
}


/* 
	functions to analyze block statistical properties:
		* simple properties: * mean * variance
		* covariance-matrix correllation coefficients
 */


// compute averages and covariance matrices for 4 components
static void compute_covariance_matrix(int xdim, int ydim, int zdim, const imageblock * blk, const error_weight_block * ewb, mat4 * cov_matrix)
{
	int i;

	int texels_per_block = xdim * ydim * zdim;

	float r_sum = 0.0f;
	float g_sum = 0.0f;
	float b_sum = 0.0f;
	float a_sum = 0.0f;
	float rr_sum = 0.0f;
	float gg_sum = 0.0f;
	float bb_sum = 0.0f;
	float aa_sum = 0.0f;
	float rg_sum = 0.0f;
	float rb_sum = 0.0f;
	float ra_sum = 0.0f;
	float gb_sum = 0.0f;
	float ga_sum = 0.0f;
	float ba_sum = 0.0f;

	float weight_sum = 0.0f;

	for (i = 0; i < texels_per_block; i++)
	{
		float weight = ewb->texel_weight[i];
		if (weight < 0.0f)
			ASTC_CODEC_INTERNAL_ERROR;
		weight_sum += weight;
		float r = blk->work_data[4 * i];
		float g = blk->work_data[4 * i + 1];
		float b = blk->work_data[4 * i + 2];
		float a = blk->work_data[4 * i + 3];
		r_sum += r * weight;
		rr_sum += r * (r * weight);
		rg_sum += g * (r * weight);
		rb_sum += b * (r * weight);
		ra_sum += a * (r * weight);
		g_sum += g * weight;
		gg_sum += g * (g * weight);
		gb_sum += b * (g * weight);
		ga_sum += a * (g * weight);
		b_sum += b * weight;
		bb_sum += b * (b * weight);
		ba_sum += a * (b * weight);
		a_sum += a * weight;
		aa_sum += a * (a * weight);
	}

	float rpt = 1.0f / MAX(weight_sum, 1e-7f);
	float rs = r_sum;
	float gs = g_sum;
	float bs = b_sum;
	float as = a_sum;

	cov_matrix->v[0] = float4(rr_sum - rs * rs * rpt, rg_sum - rs * gs * rpt, rb_sum - rs * bs * rpt, ra_sum - rs * as * rpt);
	cov_matrix->v[1] = float4(rg_sum - rs * gs * rpt, gg_sum - gs * gs * rpt, gb_sum - gs * bs * rpt, ga_sum - gs * as * rpt);
	cov_matrix->v[2] = float4(rb_sum - rs * bs * rpt, gb_sum - gs * bs * rpt, bb_sum - bs * bs * rpt, ba_sum - bs * as * rpt);
	cov_matrix->v[3] = float4(ra_sum - rs * as * rpt, ga_sum - gs * as * rpt, ba_sum - bs * as * rpt, aa_sum - as * as * rpt);

}



void prepare_block_statistics(int xdim, int ydim, int zdim, const imageblock * blk, const error_weight_block * ewb, int *is_normal_map, float *lowest_correl)
{
	int i;

	mat4 cov_matrix;

	compute_covariance_matrix(xdim, ydim, zdim, blk, ewb, &cov_matrix);

	// use the covariance matrix to compute
	// correllation coefficients
	float rr_var = cov_matrix.v[0].x;
	float gg_var = cov_matrix.v[1].y;
	float bb_var = cov_matrix.v[2].z;
	float aa_var = cov_matrix.v[3].w;

	float rg_correlation = cov_matrix.v[0].y / sqrt(MAX(rr_var * gg_var, 1e-30f));
	float rb_correlation = cov_matrix.v[0].z / sqrt(MAX(rr_var * bb_var, 1e-30f));
	float ra_correlation = cov_matrix.v[0].w / sqrt(MAX(rr_var * aa_var, 1e-30f));
	float gb_correlation = cov_matrix.v[1].z / sqrt(MAX(gg_var * bb_var, 1e-30f));
	float ga_correlation = cov_matrix.v[1].w / sqrt(MAX(gg_var * aa_var, 1e-30f));
	float ba_correlation = cov_matrix.v[2].w / sqrt(MAX(bb_var * aa_var, 1e-30f));

	if (astc_isnan(rg_correlation))
		rg_correlation = 1.0f;
	if (astc_isnan(rb_correlation))
		rb_correlation = 1.0f;
	if (astc_isnan(ra_correlation))
		ra_correlation = 1.0f;
	if (astc_isnan(gb_correlation))
		gb_correlation = 1.0f;
	if (astc_isnan(ga_correlation))
		ga_correlation = 1.0f;
	if (astc_isnan(ba_correlation))
		ba_correlation = 1.0f;

	float lowest_correlation = MIN(fabs(rg_correlation), fabs(rb_correlation));
	lowest_correlation = MIN(lowest_correlation, fabs(ra_correlation));
	lowest_correlation = MIN(lowest_correlation, fabs(gb_correlation));
	lowest_correlation = MIN(lowest_correlation, fabs(ga_correlation));
	lowest_correlation = MIN(lowest_correlation, fabs(ba_correlation));
	*lowest_correl = lowest_correlation;

	// compute a "normal-map" factor
	// this factor should be exactly 0.0 for a normal map, while it may be all over the
	// place for anything that is NOT a normal map. We can probably assume that a factor
	// of less than 0.2f represents a normal map.

	float nf_sum = 0.0f;

	int texels_per_block = xdim * ydim * zdim;

	for (i = 0; i < texels_per_block; i++)
	{
		float3 val = float3(blk->orig_data[4 * i],
							blk->orig_data[4 * i + 1],
							blk->orig_data[4 * i + 2]);
		val = (val - float3(0.5f, 0.5f, 0.5f)) * 2.0f;
		float length_squared = dot(val, val);
		float nf = fabs(length_squared - 1.0f);
		nf_sum += nf;
	}
	float nf_avg = nf_sum / texels_per_block;
	*is_normal_map = nf_avg < 0.2;
}





void compress_constant_color_block(int xdim, int ydim, int zdim, const imageblock * blk, const error_weight_block * ewb, symbolic_compressed_block * scb)
{
	int texel_count = xdim * ydim * zdim;
	int i;

	float4 color_sum = float4(0, 0, 0, 0);
	float4 color_weight_sum = float4(0, 0, 0, 0);

	const float *clp = blk->work_data;
	for (i = 0; i < texel_count; i++)
	{
		float4 weights = ewb->error_weights[i];
		float4 color_data = float4(clp[4 * i], clp[4 * i + 1], clp[4 * i + 2], clp[4 * i + 3]);
		color_sum = color_sum + (color_data * weights);
		color_weight_sum = color_weight_sum + weights;
	}

	float4 avg_color = color_sum / color_weight_sum;

	int use_fp16 = blk->rgb_lns[0];

	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("Averaged color: %f %f %f %f\n", avg_color.x, avg_color.y, avg_color.z, avg_color.w);
		}
	#endif

	// convert the color
	if (blk->rgb_lns[0])
	{
		int avg_red = static_cast < int >(floor(avg_color.x + 0.5f));
		int avg_green = static_cast < int >(floor(avg_color.y + 0.5f));
		int avg_blue = static_cast < int >(floor(avg_color.z + 0.5f));

		if (avg_red < 0)
			avg_red = 0;
		else if (avg_red > 65535)
			avg_red = 65535;

		if (avg_green < 0)
			avg_green = 0;
		else if (avg_green > 65535)
			avg_green = 65535;

		if (avg_blue < 0)
			avg_blue = 0;
		else if (avg_blue > 65535)
			avg_blue = 65535;

		avg_color.x = sf16_to_float(lns_to_sf16(avg_red));
		avg_color.y = sf16_to_float(lns_to_sf16(avg_green));
		avg_color.z = sf16_to_float(lns_to_sf16(avg_blue));
	}
	else
	{
		avg_color.x *= (1.0f / 65535.0f);
		avg_color.y *= (1.0f / 65535.0f);
		avg_color.z *= (1.0f / 65535.0f);
	}
	if (blk->alpha_lns[0])
	{
		int avg_alpha = static_cast < int >(floor(avg_color.w + 0.5f));

		if (avg_alpha < 0)
			avg_alpha = 0;
		else if (avg_alpha > 65535)
			avg_alpha = 65535;

		avg_color.w = sf16_to_float(lns_to_sf16(avg_alpha));
	}
	else
	{
		avg_color.w *= (1.0f / 65535.0f);
	}

#ifdef DEBUG_PRINT_DIAGNOSTICS
	if (print_diagnostics)
	{
		printf("Averaged color: %f %f %f %f   (%d)\n", avg_color.x, avg_color.y, avg_color.z, avg_color.w, use_fp16);

	}
#endif

	if (use_fp16)
	{
		scb->error_block = 0;
		scb->block_mode = -1;
		scb->partition_count = 0;
		scb->constant_color[0] = float_to_sf16(avg_color.x, SF_NEARESTEVEN);
		scb->constant_color[1] = float_to_sf16(avg_color.y, SF_NEARESTEVEN);
		scb->constant_color[2] = float_to_sf16(avg_color.z, SF_NEARESTEVEN);
		scb->constant_color[3] = float_to_sf16(avg_color.w, SF_NEARESTEVEN);
	}

	else
	{
		scb->error_block = 0;
		scb->block_mode = -2;
		scb->partition_count = 0;
		float red = avg_color.x;
		float green = avg_color.y;
		float blue = avg_color.z;
		float alpha = avg_color.w;
		if (red < 0)
			red = 0;
		else if (red > 1)
			red = 1;
		if (green < 0)
			green = 0;
		else if (green > 1)
			green = 1;
		if (blue < 0)
			blue = 0;
		else if (blue > 1)
			blue = 1;
		if (alpha < 0)
			alpha = 0;
		else if (alpha > 1)
			alpha = 1;
		scb->constant_color[0] = static_cast < int >(floor(red * 65535.0f + 0.5f));
		scb->constant_color[1] = static_cast < int >(floor(green * 65535.0f + 0.5f));
		scb->constant_color[2] = static_cast < int >(floor(blue * 65535.0f + 0.5f));
		scb->constant_color[3] = static_cast < int >(floor(alpha * 65535.0f + 0.5f));
	}
}

int block_mode_histogram[2048];

float compress_symbolic_block(const astc_codec_image * input_image,
							  astc_decode_mode decode_mode, int xdim, int ydim, int zdim, const error_weighting_params * ewp, const imageblock * blk, symbolic_compressed_block * scb,
							  compress_symbolic_block_buffers * tmpbuf)
{
	int i, j;
	int xpos = blk->xpos;
	int ypos = blk->ypos;
	int zpos = blk->zpos;

	int x, y, z;


	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("Diagnostics of block of dimension %d x %d x %d\n\n", xdim, ydim, zdim);
	
			printf("XPos: %d  YPos: %d  ZPos: %d\n", xpos, ypos, zpos);
	
			printf("Red-min: %f   Red-max: %f\n", blk->red_min, blk->red_max);
			printf("Green-min: %f   Green-max: %f\n", blk->green_min, blk->green_max);
			printf("Blue-min: %f   Blue-max: %f\n", blk->blue_min, blk->blue_max);
			printf("Alpha-min: %f   Alpha-max: %f\n", blk->alpha_min, blk->alpha_max);
			printf("Grayscale: %d\n", blk->grayscale);
	
			for (z = 0; z < zdim; z++)
				for (y = 0; y < ydim; y++)
					for (x = 0; x < xdim; x++)
					{
						int idx = ((z * ydim + y) * xdim + x) * 4;
						printf("Texel (%d %d %d) : orig=< %g, %g, %g, %g >, work=< %g, %g, %g, %g >\n",
							x, y, z,
							blk->orig_data[idx],
							blk->orig_data[idx + 1], blk->orig_data[idx + 2], blk->orig_data[idx + 3], blk->work_data[idx], blk->work_data[idx + 1], blk->work_data[idx + 2], blk->work_data[idx + 3]);
					}
			printf("\n");
		}
	#endif


	if (blk->red_min == blk->red_max && blk->green_min == blk->green_max && blk->blue_min == blk->blue_max && blk->alpha_min == blk->alpha_max)
	{

		// detected a constant-color block. Encode as FP16 if using HDR
		scb->error_block = 0;

		if (rgb_force_use_of_hdr)
		{
			scb->block_mode = -1;
			scb->partition_count = 0;
			scb->constant_color[0] = float_to_sf16(blk->orig_data[0], SF_NEARESTEVEN);
			scb->constant_color[1] = float_to_sf16(blk->orig_data[1], SF_NEARESTEVEN);
			scb->constant_color[2] = float_to_sf16(blk->orig_data[2], SF_NEARESTEVEN);
			scb->constant_color[3] = float_to_sf16(blk->orig_data[3], SF_NEARESTEVEN);
		}
		else
		{
			// Encode as UNORM16 if NOT using HDR.
			scb->block_mode = -2;
			scb->partition_count = 0;
			float red = blk->orig_data[0];
			float green = blk->orig_data[1];
			float blue = blk->orig_data[2];
			float alpha = blk->orig_data[3];
			if (red < 0)
				red = 0;
			else if (red > 1)
				red = 1;
			if (green < 0)
				green = 0;
			else if (green > 1)
				green = 1;
			if (blue < 0)
				blue = 0;
			else if (blue > 1)
				blue = 1;
			if (alpha < 0)
				alpha = 0;
			else if (alpha > 1)
				alpha = 1;
			scb->constant_color[0] = (int)floor(red * 65535.0f + 0.5f);
			scb->constant_color[1] = (int)floor(green * 65535.0f + 0.5f);
			scb->constant_color[2] = (int)floor(blue * 65535.0f + 0.5f);
			scb->constant_color[3] = (int)floor(alpha * 65535.0f + 0.5f);
		}

		#ifdef DEBUG_PRINT_DIAGNOSTICS
			if (print_diagnostics)
			{
				printf("Block is single-color <%4.4X %4.4X %4.4X %4.4X>\n", scb->constant_color[0], scb->constant_color[1], scb->constant_color[2], scb->constant_color[3]);
			}
		#endif

		if (print_tile_errors)
			printf("0\n");

		physical_compressed_block psb = symbolic_to_physical(xdim, ydim, zdim, scb);
		physical_to_symbolic(xdim, ydim, zdim, psb, scb);

		return 0.0f;
	}

	error_weight_block *ewb = tmpbuf->ewb;
	error_weight_block_orig *ewbo = tmpbuf->ewbo;

	float error_weight_sum = prepare_error_weight_block(input_image,
														xdim, ydim, zdim,
														ewp, blk, ewb, ewbo);

	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("\n");
			for (z = 0; z < zdim; z++)
				for (y = 0; y < ydim; y++)
					for (x = 0; x < xdim; x++)
					{
						int idx = (z * ydim + y) * xdim + x;
						printf("ErrorWeight (%d %d %d) : < %g, %g, %g, %g >\n", x, y, z, ewb->error_weights[idx].x, ewb->error_weights[idx].y, ewb->error_weights[idx].z, ewb->error_weights[idx].w);
					}
			printf("\n");
		}
	#endif

	symbolic_compressed_block *tempblocks = tmpbuf->tempblocks;

	float error_of_best_block = 1e20f;
	// int modesel=0;

	imageblock *temp = tmpbuf->temp;

	float best_errorvals_in_modes[17];
	for (i = 0; i < 17; i++)
		best_errorvals_in_modes[i] = 1e30f;

	int uses_alpha = imageblock_uses_alpha(xdim, ydim, zdim, blk);


	// compression of average-color blocks disabled for the time being;
	// they produce extremely severe block artifacts.
#if 0
	// first, compress an averaged-color block
	compress_constant_color_block(xdim, ydim, zdim, blk, ewb, scb);

	decompress_symbolic_block(decode_mode, xdim, ydim, zdim, xpos, ypos, zpos, scb, temp);

	float avgblock_errorval = compute_imageblock_difference(xdim, ydim, zdim,
															blk, temp, ewb) * 4.0f;	// bias somewhat against the average-color block.

	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("\n-----------------------------------\n");
			printf("Average-color block test completed\n");
			printf("Resulting error value: %g\n", avgblock_errorval);
		}
	#endif


	if (avgblock_errorval < error_of_best_block)
	{
		#ifdef DEBUG_PRINT_DIAGNOSTICS
			if (print_diagnostics)
				printf("Accepted as better than previous-best-error, which was %g\n", error_of_best_block);
		#endif

		error_of_best_block = avgblock_errorval;
		// *scb = tempblocks[j];
		modesel = 0;
	}

	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("-----------------------------------\n");
		}
	#endif
#endif


	float mode_cutoff = ewp->block_mode_cutoff;

	// next, test mode #0. This mode uses 1 plane of weights and 1 partition.
	// we test it twice, first with a modecutoff of 0, then with the specified mode-cutoff.
	// This causes an early-out that speeds up encoding of "easy" content.

	float modecutoffs[2];
	float errorval_mult[2] = { 2.5, 1 };
	modecutoffs[0] = 0;
	modecutoffs[1] = mode_cutoff;

	#if 0
		if ((error_of_best_block / error_weight_sum) < ewp->texel_avg_error_limit)
			goto END_OF_TESTS;
	#endif

	float best_errorval_in_mode;
	for (i = 0; i < 2; i++)
	{
		compress_symbolic_block_fixed_partition_1_plane(decode_mode, modecutoffs[i], ewp->max_refinement_iters, xdim, ydim, zdim, 1,	// partition count
														0,	// partition index
														blk, ewb, tempblocks, tmpbuf->plane1);

		best_errorval_in_mode = 1e30f;
		for (j = 0; j < 4; j++)
		{
			if (tempblocks[j].error_block)
				continue;
			decompress_symbolic_block(decode_mode, xdim, ydim, zdim, xpos, ypos, zpos, tempblocks + j, temp);
			float errorval = compute_imageblock_difference(xdim, ydim, zdim,
														   blk, temp, ewb) * errorval_mult[i];

			#ifdef DEBUG_PRINT_DIAGNOSTICS
				if (print_diagnostics)
				{
					printf("\n-----------------------------------\n");
					printf("Single-weight partition test 0 (1 partition) completed\n");
					printf("Resulting error value: %g\n", errorval);
				}
			#endif

			if (errorval < best_errorval_in_mode)
				best_errorval_in_mode = errorval;

			if (errorval < error_of_best_block)
			{
				#ifdef DEBUG_PRINT_DIAGNOSTICS
					if (print_diagnostics)
						printf("Accepted as better than previous-best-error, which was %g\n", error_of_best_block);
				#endif

				error_of_best_block = errorval;
				*scb = tempblocks[j];

				// modesel = 0;
			}

			#ifdef DEBUG_PRINT_DIAGNOSTICS
				if (print_diagnostics)
				{
					printf("-----------------------------------\n");
				}
			#endif
		}

		best_errorvals_in_modes[0] = best_errorval_in_mode;
		if ((error_of_best_block / error_weight_sum) < ewp->texel_avg_error_limit)
			goto END_OF_TESTS;
	}

	int is_normal_map;
	float lowest_correl;
	prepare_block_statistics(xdim, ydim, zdim, blk, ewb, &is_normal_map, &lowest_correl);

	if (is_normal_map && lowest_correl < 0.99f)
		lowest_correl = 0.99f;

	// next, test the four possible 1-partition, 2-planes modes
	for (i = 0; i < 4; i++)
	{

		if (lowest_correl > ewp->lowest_correlation_cutoff)
			continue;

		if (blk->grayscale && i != 3)
			continue;

		if (!uses_alpha && i == 3)
			continue;

		compress_symbolic_block_fixed_partition_2_planes(decode_mode, mode_cutoff, ewp->max_refinement_iters, xdim, ydim, zdim, 1,	// partition count
														 0,	// partition index
														 i,	// the color component to test a separate plane of weights for.
														 blk, ewb, tempblocks, tmpbuf->planes2);

		best_errorval_in_mode = 1e30f;
		for (j = 0; j < 4; j++)
		{
			if (tempblocks[j].error_block)
				continue;
			decompress_symbolic_block(decode_mode, xdim, ydim, zdim, xpos, ypos, zpos, tempblocks + j, temp);
			float errorval = compute_imageblock_difference(xdim, ydim, zdim,
														   blk, temp, ewb);

			#ifdef DEBUG_PRINT_DIAGNOSTICS
				if (print_diagnostics)
				{
					printf("\n-----------------------------------\n");
					printf("Dual-weight partition test %d (1 partition) completed\n", i);
					printf("Resulting error value: %g\n", errorval);
				}
			#endif

			if (errorval < best_errorval_in_mode)
				best_errorval_in_mode = errorval;

			if (errorval < error_of_best_block)
			{
				#ifdef DEBUG_PRINT_DIAGNOSTICS
					if (print_diagnostics)
						printf("Accepted as better than previous-best-error, which was %g\n", error_of_best_block);
				#endif

				error_of_best_block = errorval;
				*scb = tempblocks[j];

				// modesel = i+1;
			}

			#ifdef DEBUG_PRINT_DIAGNOSTICS
				if (print_diagnostics)
				{
					printf("-----------------------------------\n");
				}
			#endif

			best_errorvals_in_modes[i + 1] = best_errorval_in_mode;
		}

		if ((error_of_best_block / error_weight_sum) < ewp->texel_avg_error_limit)
			goto END_OF_TESTS;
	}

	// find best blocks for 2, 3 and 4 partitions
	int partition_count;
	for (partition_count = 2; partition_count <= 4; partition_count++)
	{
		int partition_indices_1plane[2];
		int partition_indices_2planes[2];

		find_best_partitionings(ewp->partition_search_limit,
								xdim, ydim, zdim, partition_count, blk, ewb, 1, 
								&(partition_indices_1plane[0]), &(partition_indices_1plane[1]), &(partition_indices_2planes[0]));

		for (i = 0; i < 2; i++)
		{
			compress_symbolic_block_fixed_partition_1_plane(decode_mode, mode_cutoff, ewp->max_refinement_iters, xdim, ydim, zdim, partition_count, partition_indices_1plane[i], blk, ewb, tempblocks, tmpbuf->plane1);

			best_errorval_in_mode = 1e30f;
			for (j = 0; j < 4; j++)
			{
				if (tempblocks[j].error_block)
					continue;
				decompress_symbolic_block(decode_mode, xdim, ydim, zdim, xpos, ypos, zpos, tempblocks + j, temp);
				float errorval = compute_imageblock_difference(xdim, ydim, zdim,
															   blk, temp, ewb);

				#ifdef DEBUG_PRINT_DIAGNOSTICS
					if (print_diagnostics)
					{
						printf("\n-----------------------------------\n");
						printf("Single-weight partition test %d (%d partitions) completed\n", i, partition_count);
						printf("Resulting error value: %g\n", errorval);
					}
				#endif

				if (errorval < best_errorval_in_mode)
					best_errorval_in_mode = errorval;

				if (errorval < error_of_best_block)
				{
					#ifdef DEBUG_PRINT_DIAGNOSTICS
						if (print_diagnostics)
							printf("Accepted as better than previous-best-error, which was %g\n", error_of_best_block);
					#endif

					error_of_best_block = errorval;
					*scb = tempblocks[j];

					// modesel = 4*(partition_count-2) + 5 + i;
				}
			}

			best_errorvals_in_modes[4 * (partition_count - 2) + 5 + i] = best_errorval_in_mode;

			#ifdef DEBUG_PRINT_DIAGNOSTICS
				if (print_diagnostics)
				{
					printf("-----------------------------------\n");
				}
			#endif

			if ((error_of_best_block / error_weight_sum) < ewp->texel_avg_error_limit)
				goto END_OF_TESTS;
		}


		if (partition_count == 2 && !is_normal_map && MIN(best_errorvals_in_modes[5], best_errorvals_in_modes[6]) > (best_errorvals_in_modes[0] * ewp->partition_1_to_2_limit))
			goto END_OF_TESTS;

		// don't bother to check 4 partitions for dual plane of weightss, ever.
		if (partition_count == 4)
			break;

		for (i = 0; i < 2; i++)
		{
			if (lowest_correl > ewp->lowest_correlation_cutoff)
				continue;
			compress_symbolic_block_fixed_partition_2_planes(decode_mode,
															 mode_cutoff,
															 ewp->max_refinement_iters,
															 xdim, ydim, zdim,
															 partition_count,
															 partition_indices_2planes[i] & (PARTITION_COUNT - 1), partition_indices_2planes[i] >> PARTITION_BITS,
															 blk, ewb, tempblocks, tmpbuf->planes2);

			best_errorval_in_mode = 1e30f;
			for (j = 0; j < 4; j++)
			{
				if (tempblocks[j].error_block)
					continue;
				decompress_symbolic_block(decode_mode, xdim, ydim, zdim, xpos, ypos, zpos, tempblocks + j, temp);

				float errorval = compute_imageblock_difference(xdim, ydim, zdim,
															   blk, temp, ewb);

				#ifdef DEBUG_PRINT_DIAGNOSTICS
					if (print_diagnostics)
					{
						printf("\n-----------------------------------\n");
						printf("Dual-weight partition test %d (%d partitions) completed\n", i, partition_count);
						printf("Resulting error value: %g\n", errorval);
					}
				#endif

				if (errorval < best_errorval_in_mode)
					best_errorval_in_mode = errorval;

				if (errorval < error_of_best_block)
				{
					#ifdef DEBUG_PRINT_DIAGNOSTICS
						if (print_diagnostics)
							printf("Accepted as better than previous-best-error, which was %g\n", error_of_best_block);
					#endif

					error_of_best_block = errorval;
					*scb = tempblocks[j];

					// modesel = 4*(partition_count-2) + 5 + 2 + i;
				}
			}

			best_errorvals_in_modes[4 * (partition_count - 2) + 5 + 2 + i] = best_errorval_in_mode;

			#ifdef DEBUG_PRINT_DIAGNOSTICS
				if (print_diagnostics)
				{
					printf("-----------------------------------\n");
				}
			#endif

			if ((error_of_best_block / error_weight_sum) < ewp->texel_avg_error_limit)
				goto END_OF_TESTS;
		}
	}

  END_OF_TESTS:

	#if 0
		if (print_statistics)
		{
			for (i = 0; i < 13; i++)
				printf("%f ", best_errorvals_in_modes[i]);
	
			printf("%d  %f  %f  %f ", modesel, error_of_best_block,
				MIN(best_errorvals_in_modes[1], best_errorvals_in_modes[2]) / best_errorvals_in_modes[0],
				MIN(MIN(best_errorvals_in_modes[7], best_errorvals_in_modes[8]), best_errorvals_in_modes[9]) / best_errorvals_in_modes[0]);
	
			printf("\n");
		}
	#endif

	if (scb->block_mode >= 0)
		block_mode_histogram[scb->block_mode & 0x7ff]++;

	
	// compress/decompress to a physical block
	physical_compressed_block psb = symbolic_to_physical(xdim, ydim, zdim, scb);
	physical_to_symbolic(xdim, ydim, zdim, psb, scb);


	if (print_tile_errors)
		printf("%g\n", error_of_best_block);


	// mean squared error per color component.
	return error_of_best_block / ((float)xdim * ydim * zdim);
}
