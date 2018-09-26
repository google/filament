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
 *	@brief	Functions for computing color endpoints and texel weights.
 */
/*----------------------------------------------------------------------------*/

#include <math.h>

#include "astc_codec_internals.h"

#ifdef DEBUG_PRINT_DIAGNOSTICS
#include <stdio.h>
#endif


#ifdef DEBUG_CAPTURE_NAN
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif

	#include <fenv.h>
#endif

static void compute_endpoints_and_ideal_weights_1_component(int xdim, int ydim, int zdim,
															const partition_info * pt, const imageblock * blk,
															const error_weight_block * ewb, endpoints_and_weights * ei,
															int component)
{
	int i;

	int partition_count = pt->partition_count;
	ei->ep.partition_count = partition_count;

	float lowvalues[4], highvalues[4];
	float partition_error_scale[4];
	float linelengths_rcp[4];

	int texels_per_block = xdim * ydim * zdim;

	const float *error_weights;
	switch (component)
	{
	case 0:
		error_weights = ewb->texel_weight_r;
		break;
	case 1:
		error_weights = ewb->texel_weight_g;
		break;
	case 2:
		error_weights = ewb->texel_weight_b;
		break;
	case 3:
		error_weights = ewb->texel_weight_a;
		break;
	default:
		error_weights = ewb->texel_weight_r;
		ASTC_CODEC_INTERNAL_ERROR;
	}


	for (i = 0; i < partition_count; i++)
	{
		lowvalues[i] = 1e10;
		highvalues[i] = -1e10;
	}

	for (i = 0; i < texels_per_block; i++)
	{
		if (error_weights[i] > 1e-10)
		{
			float value = blk->work_data[4 * i + component];
			int partition = pt->partition_of_texel[i];
			if (value < lowvalues[partition])
				lowvalues[partition] = value;
			if (value > highvalues[partition])
				highvalues[partition] = value;
		}
	}

	for (i = 0; i < partition_count; i++)
	{
		float diff = highvalues[i] - lowvalues[i];
		if (diff < 0)
		{
			lowvalues[i] = 0;
			highvalues[i] = 0;
		}
		if (diff < 1e-7f)
			diff = 1e-7f;
		partition_error_scale[i] = diff * diff;
		linelengths_rcp[i] = 1.0f / diff;
	}

	for (i = 0; i < texels_per_block; i++)
	{
		float value = blk->work_data[4 * i + component];
		int partition = pt->partition_of_texel[i];
		value -= lowvalues[partition];
		value *= linelengths_rcp[partition];
		if (value > 1.0f)
			value = 1.0f;
		else if (!(value > 0.0f))
			value = 0.0f;

		ei->weights[i] = value;
		ei->weight_error_scale[i] = partition_error_scale[partition] * error_weights[i];
		if (astc_isnan(ei->weight_error_scale[i]))
		{
			ASTC_CODEC_INTERNAL_ERROR;
		}
	}

	for (i = 0; i < partition_count; i++)
	{
		ei->ep.endpt0[i] = float4(blk->red_min, blk->green_min, blk->blue_min, blk->alpha_min);
		ei->ep.endpt1[i] = float4(blk->red_max, blk->green_max, blk->blue_max, blk->alpha_max);
		switch (component)
		{
		case 0:				// red/x
			ei->ep.endpt0[i].x = lowvalues[i];
			ei->ep.endpt1[i].x = highvalues[i];
			break;
		case 1:				// green/y
			ei->ep.endpt0[i].y = lowvalues[i];
			ei->ep.endpt1[i].y = highvalues[i];
			break;
		case 2:				// blue/z
			ei->ep.endpt0[i].z = lowvalues[i];
			ei->ep.endpt1[i].z = highvalues[i];
			break;
		case 3:				// alpha/w
			ei->ep.endpt0[i].w = lowvalues[i];
			ei->ep.endpt1[i].w = highvalues[i];
			break;
		}
	}

	// print all the data that this function computes.
	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("%s: %dx%dx%d texels, %d partitions, component=%d\n", __func__, xdim, ydim, zdim, partition_count, component);
			printf("Endpoints:\n");
			for (i = 0; i < partition_count; i++)
			{
				printf("%d Low: <%g> => <%g %g %g %g>\n", i, lowvalues[i], ei->ep.endpt0[i].x, ei->ep.endpt0[i].y, ei->ep.endpt0[i].z, ei->ep.endpt0[i].w);
				printf("%d High: <%g> => <%g %g %g %g>\n", i, highvalues[i], ei->ep.endpt1[i].x, ei->ep.endpt1[i].y, ei->ep.endpt1[i].z, ei->ep.endpt1[i].w);
			}
			printf("Ideal-weights:\n");

			for (i = 0; i < texels_per_block; i++)
			{
				printf("%3d <%2d %2d %2d>=> %g (weight=%g)\n", i, i % xdim, (i / xdim) % ydim, i / (xdim * ydim), ei->weights[i], ei->weight_error_scale[i]);
			}
			printf("\n");
		}
	#endif
}


static void compute_endpoints_and_ideal_weights_2_components(int xdim, int ydim, int zdim, const partition_info * pt,
															 const imageblock * blk, const error_weight_block * ewb,
															 endpoints_and_weights * ei, int component1, int component2)
{
	int i;

	int partition_count = pt->partition_count;
	ei->ep.partition_count = partition_count;

	float4 error_weightings[4];
	float4 color_scalefactors[4];

	float2 scalefactors[4];

	const float *error_weights;
	if (component1 == 0 && component2 == 1)
		error_weights = ewb->texel_weight_rg;
	else if (component1 == 0 && component2 == 2)
		error_weights = ewb->texel_weight_rb;
	else if (component1 == 1 && component2 == 2)
		error_weights = ewb->texel_weight_gb;
	else
	{
		error_weights = ewb->texel_weight_rg;
		ASTC_CODEC_INTERNAL_ERROR;
	}

	int texels_per_block = xdim * ydim * zdim;

	compute_partition_error_color_weightings(xdim, ydim, zdim, ewb, pt, error_weightings, color_scalefactors);

	for (i = 0; i < partition_count; i++)
	{
		float s1 = 0, s2 = 0;
		switch (component1)
		{
		case 0:
			s1 = color_scalefactors[i].x;
			break;
		case 1:
			s1 = color_scalefactors[i].y;
			break;
		case 2:
			s1 = color_scalefactors[i].z;
			break;
		case 3:
			s1 = color_scalefactors[i].w;
			break;
		}

		switch (component2)
		{
		case 0:
			s2 = color_scalefactors[i].x;
			break;
		case 1:
			s2 = color_scalefactors[i].y;
			break;
		case 2:
			s2 = color_scalefactors[i].z;
			break;
		case 3:
			s2 = color_scalefactors[i].w;
			break;
		}
		scalefactors[i] = normalize(float2(s1, s2)) * 1.41421356f;
	}


	float lowparam[4], highparam[4];

	float2 averages[4];
	float2 directions[4];

	line2 lines[4];
	float scale[4];
	float length_squared[4];


	for (i = 0; i < partition_count; i++)
	{
		lowparam[i] = 1e10;
		highparam[i] = -1e10;
	}


	compute_averages_and_directions_2_components(pt, blk, ewb, scalefactors, component1, component2, averages, directions);

	for (i = 0; i < partition_count; i++)
	{
		float2 egv = directions[i];
		if (egv.x + egv.y < 0.0f)
			directions[i] = float2(0, 0) - egv;
	}

	for (i = 0; i < partition_count; i++)
	{
		lines[i].a = averages[i];
		if (dot(directions[i], directions[i]) == 0.0f)
			lines[i].b = normalize(float2(1, 1));
		else
			lines[i].b = normalize(directions[i]);
	}


	for (i = 0; i < texels_per_block; i++)
	{
		if (error_weights[i] > 1e-10)
		{
			int partition = pt->partition_of_texel[i];
			float2 point = float2(blk->work_data[4 * i + component1], blk->work_data[4 * i + component2]) * scalefactors[partition];
			line2 l = lines[partition];
			float param = dot(point - l.a, l.b);
			ei->weights[i] = param;
			if (param < lowparam[partition])
				lowparam[partition] = param;
			if (param > highparam[partition])
				highparam[partition] = param;
		}
		else
		{
			ei->weights[i] = -1e38f;
		}
	}

	float2 lowvalues[4];
	float2 highvalues[4];


	for (i = 0; i < partition_count; i++)
	{
		float length = highparam[i] - lowparam[i];
		if (length < 0)			// case for when none of the texels had any weight
		{
			lowparam[i] = 0.0f;
			highparam[i] = 1e-7f;
		}

		// it is possible for a uniform-color partition to produce length=0; this
		// causes NaN-production and NaN-propagation later on. Set length to
		// a small value to avoid this problem.
		if (length < 1e-7f)
			length = 1e-7f;

		length_squared[i] = length * length;
		scale[i] = 1.0f / length;

		float2 ep0 = lines[i].a + lines[i].b * lowparam[i];
		float2 ep1 = lines[i].a + lines[i].b * highparam[i];

		ep0 = ep0 / scalefactors[i];
		ep1 = ep1 / scalefactors[i];

		lowvalues[i] = ep0;
		highvalues[i] = ep1;
	}


	for (i = 0; i < partition_count; i++)
	{
		ei->ep.endpt0[i] = float4(blk->red_min, blk->green_min, blk->blue_min, blk->alpha_min);
		ei->ep.endpt1[i] = float4(blk->red_max, blk->green_max, blk->blue_max, blk->alpha_max);

		float2 ep0 = lowvalues[i];
		float2 ep1 = highvalues[i];

		switch (component1)
		{
		case 0:
			ei->ep.endpt0[i].x = ep0.x;
			ei->ep.endpt1[i].x = ep1.x;
			break;
		case 1:
			ei->ep.endpt0[i].y = ep0.x;
			ei->ep.endpt1[i].y = ep1.x;
			break;
		case 2:
			ei->ep.endpt0[i].z = ep0.x;
			ei->ep.endpt1[i].z = ep1.x;
			break;
		case 3:
			ei->ep.endpt0[i].w = ep0.x;
			ei->ep.endpt1[i].w = ep1.x;
			break;
		}
		switch (component2)
		{
		case 0:
			ei->ep.endpt0[i].x = ep0.y;
			ei->ep.endpt1[i].x = ep1.y;
			break;
		case 1:
			ei->ep.endpt0[i].y = ep0.y;
			ei->ep.endpt1[i].y = ep1.y;
			break;
		case 2:
			ei->ep.endpt0[i].z = ep0.y;
			ei->ep.endpt1[i].z = ep1.y;
			break;
		case 3:
			ei->ep.endpt0[i].w = ep0.y;
			ei->ep.endpt1[i].w = ep1.y;
			break;
		}
	}

	for (i = 0; i < texels_per_block; i++)
	{
		int partition = pt->partition_of_texel[i];
		float idx = (ei->weights[i] - lowparam[partition]) * scale[partition];
		if (idx > 1.0f)
			idx = 1.0f;
		else if (!(idx > 0.0f))
			idx = 0.0f;

		ei->weights[i] = idx;
		ei->weight_error_scale[i] = length_squared[partition] * error_weights[i];
		if (astc_isnan(ei->weight_error_scale[i]))
		{
			ASTC_CODEC_INTERNAL_ERROR;
		}
	}

	// print all the data that this function computes.
	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("%s: %dx%dx%d texels, %d partitions, component1=%d, component2=%d\n", __func__, xdim, ydim, zdim, partition_count, component1, component2);
			printf("Endpoints:\n");
			for (i = 0; i < partition_count; i++)
			{
				printf("%d Low: <%g %g> => <%g %g %g %g>\n", i, lowvalues[i].x, lowvalues[i].y, ei->ep.endpt0[i].x, ei->ep.endpt0[i].y, ei->ep.endpt0[i].z, ei->ep.endpt0[i].w);
				printf("%d High: <%g %g> => <%g %g %g %g>\n", i, highvalues[i].x, highvalues[i].y, ei->ep.endpt1[i].x, ei->ep.endpt1[i].y, ei->ep.endpt1[i].z, ei->ep.endpt1[i].w);
			}
			printf("Ideal-weights:\n");

			for (i = 0; i < texels_per_block; i++)
			{
				printf("%3d <%2d %2d %2d>=> %g (weight=%g)\n", i, i % xdim, (i / xdim) % ydim, i / (xdim * ydim), ei->weights[i], ei->weight_error_scale[i]);
			}
			printf("\n");
		}
	#endif
}

static void compute_endpoints_and_ideal_weights_3_components(int xdim, int ydim, int zdim, const partition_info * pt,
															 const imageblock * blk, const error_weight_block * ewb,
															 endpoints_and_weights * ei, int component1, int component2, int component3)
{
	int i;

	int partition_count = pt->partition_count;
	ei->ep.partition_count = partition_count;

	float4 error_weightings[4];
	float4 color_scalefactors[4];

	float3 scalefactors[4];

	int texels_per_block = xdim * ydim * zdim;

	const float *error_weights;
	if (component1 == 1 && component2 == 2 && component3 == 3)
		error_weights = ewb->texel_weight_gba;
	else if (component1 == 0 && component2 == 2 && component3 == 3)
		error_weights = ewb->texel_weight_rba;
	else if (component1 == 0 && component2 == 1 && component3 == 3)
		error_weights = ewb->texel_weight_rga;
	else if (component1 == 0 && component2 == 1 && component3 == 2)
		error_weights = ewb->texel_weight_rgb;
	else
	{
		error_weights = ewb->texel_weight_gba;
		ASTC_CODEC_INTERNAL_ERROR;
	}

	compute_partition_error_color_weightings(xdim, ydim, zdim, ewb, pt, error_weightings, color_scalefactors);

	for (i = 0; i < partition_count; i++)
	{
		float s1 = 0, s2 = 0, s3 = 0;
		switch (component1)
		{
		case 0:
			s1 = color_scalefactors[i].x;
			break;
		case 1:
			s1 = color_scalefactors[i].y;
			break;
		case 2:
			s1 = color_scalefactors[i].z;
			break;
		case 3:
			s1 = color_scalefactors[i].w;
			break;
		}

		switch (component2)
		{
		case 0:
			s2 = color_scalefactors[i].x;
			break;
		case 1:
			s2 = color_scalefactors[i].y;
			break;
		case 2:
			s2 = color_scalefactors[i].z;
			break;
		case 3:
			s2 = color_scalefactors[i].w;
			break;
		}

		switch (component3)
		{
		case 0:
			s3 = color_scalefactors[i].x;
			break;
		case 1:
			s3 = color_scalefactors[i].y;
			break;
		case 2:
			s3 = color_scalefactors[i].z;
			break;
		case 3:
			s3 = color_scalefactors[i].w;
			break;
		}
		scalefactors[i] = normalize(float3(s1, s2, s3)) * 1.73205080f;
	}


	float lowparam[4], highparam[4];

	float3 averages[4];
	float3 directions[4];

	line3 lines[4];
	float scale[4];
	float length_squared[4];


	for (i = 0; i < partition_count; i++)
	{
		lowparam[i] = 1e10;
		highparam[i] = -1e10;
	}

	compute_averages_and_directions_3_components(pt, blk, ewb, scalefactors, component1, component2, component3, averages, directions);

	for (i = 0; i < partition_count; i++)
	{
		float3 direc = directions[i];
		if (direc.x + direc.y + direc.z < 0.0f)
			directions[i] = float3(0, 0, 0) - direc;
	}

	for (i = 0; i < partition_count; i++)
	{
		lines[i].a = averages[i];
		if (dot(directions[i], directions[i]) == 0.0f)
			lines[i].b = normalize(float3(1, 1, 1));
		else
			lines[i].b = normalize(directions[i]);
	}


	for (i = 0; i < texels_per_block; i++)
	{
		if (error_weights[i] > 1e-10)
		{
			int partition = pt->partition_of_texel[i];
			float3 point = float3(blk->work_data[4 * i + component1], blk->work_data[4 * i + component2], blk->work_data[4 * i + component3]) * scalefactors[partition];
			line3 l = lines[partition];
			float param = dot(point - l.a, l.b);
			ei->weights[i] = param;
			if (param < lowparam[partition])
				lowparam[partition] = param;
			if (param > highparam[partition])
				highparam[partition] = param;
		}
		else
		{
			ei->weights[i] = -1e38f;
		}
	}

	float3 lowvalues[4];
	float3 highvalues[4];


	for (i = 0; i < partition_count; i++)
	{
		float length = highparam[i] - lowparam[i];
		if (length < 0)			// case for when none of the texels had any weight
		{
			lowparam[i] = 0.0f;
			highparam[i] = 1e-7f;
		}

		// it is possible for a uniform-color partition to produce length=0; this
		// causes NaN-production and NaN-propagation later on. Set length to
		// a small value to avoid this problem.
		if (length < 1e-7f)
			length = 1e-7f;

		length_squared[i] = length * length;
		scale[i] = 1.0f / length;

		float3 ep0 = lines[i].a + lines[i].b * lowparam[i];
		float3 ep1 = lines[i].a + lines[i].b * highparam[i];

		ep0 = ep0 / scalefactors[i];
		ep1 = ep1 / scalefactors[i];


		lowvalues[i] = ep0;
		highvalues[i] = ep1;
	}


	for (i = 0; i < partition_count; i++)
	{
		ei->ep.endpt0[i] = float4(blk->red_min, blk->green_min, blk->blue_min, blk->alpha_min);
		ei->ep.endpt1[i] = float4(blk->red_max, blk->green_max, blk->blue_max, blk->alpha_max);


		float3 ep0 = lowvalues[i];
		float3 ep1 = highvalues[i];

		switch (component1)
		{
		case 0:
			ei->ep.endpt0[i].x = ep0.x;
			ei->ep.endpt1[i].x = ep1.x;
			break;
		case 1:
			ei->ep.endpt0[i].y = ep0.x;
			ei->ep.endpt1[i].y = ep1.x;
			break;
		case 2:
			ei->ep.endpt0[i].z = ep0.x;
			ei->ep.endpt1[i].z = ep1.x;
			break;
		case 3:
			ei->ep.endpt0[i].w = ep0.x;
			ei->ep.endpt1[i].w = ep1.x;
			break;
		}
		switch (component2)
		{
		case 0:
			ei->ep.endpt0[i].x = ep0.y;
			ei->ep.endpt1[i].x = ep1.y;
			break;
		case 1:
			ei->ep.endpt0[i].y = ep0.y;
			ei->ep.endpt1[i].y = ep1.y;
			break;
		case 2:
			ei->ep.endpt0[i].z = ep0.y;
			ei->ep.endpt1[i].z = ep1.y;
			break;
		case 3:
			ei->ep.endpt0[i].w = ep0.y;
			ei->ep.endpt1[i].w = ep1.y;
			break;
		}
		switch (component3)
		{
		case 0:
			ei->ep.endpt0[i].x = ep0.z;
			ei->ep.endpt1[i].x = ep1.z;
			break;
		case 1:
			ei->ep.endpt0[i].y = ep0.z;
			ei->ep.endpt1[i].y = ep1.z;
			break;
		case 2:
			ei->ep.endpt0[i].z = ep0.z;
			ei->ep.endpt1[i].z = ep1.z;
			break;
		case 3:
			ei->ep.endpt0[i].w = ep0.z;
			ei->ep.endpt1[i].w = ep1.z;
			break;
		}
	}

	for (i = 0; i < texels_per_block; i++)
	{
		int partition = pt->partition_of_texel[i];
		float idx = (ei->weights[i] - lowparam[partition]) * scale[partition];
		if (idx > 1.0f)
			idx = 1.0f;
		else if (!(idx > 0.0f))
			idx = 0.0f;

		ei->weights[i] = idx;
		ei->weight_error_scale[i] = length_squared[partition] * error_weights[i];
		if (astc_isnan(ei->weight_error_scale[i]))
		{
			ASTC_CODEC_INTERNAL_ERROR;
		}
	}

	// print all the data that this function computes.
	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("%s: %dx%dx%d texels, %d partitions, component1=%d, component2=%d, component3=%d\n", __func__, xdim, ydim, zdim, partition_count, component1, component2, component3);
			printf("Endpoints:\n");
			for (i = 0; i < partition_count; i++)
			{
				printf("%d Low: <%g %g %f> => <%g %g %g %g>\n", i, lowvalues[i].x, lowvalues[i].y, lowvalues[i].z, ei->ep.endpt0[i].x, ei->ep.endpt0[i].y, ei->ep.endpt0[i].z, ei->ep.endpt0[i].w);
				printf("%d High: <%g %g %g> => <%g %g %g %g>\n", i, highvalues[i].x, highvalues[i].y, highvalues[i].z, ei->ep.endpt1[i].x, ei->ep.endpt1[i].y, ei->ep.endpt1[i].z, ei->ep.endpt1[i].w);
			}
			printf("Ideal-weights:\n");

			for (i = 0; i < texels_per_block; i++)
			{
				printf("%3d <%2d %2d %2d>=> %g (weight=%g)\n", i, (i % xdim), (i / xdim) % ydim, i / (xdim * ydim), ei->weights[i], ei->weight_error_scale[i]);
			}
			printf("\n");
		}
	#endif
}



static void compute_endpoints_and_ideal_weights_rgba(int xdim, int ydim, int zdim, const partition_info * pt, const imageblock * blk, const error_weight_block * ewb, endpoints_and_weights * ei)
{
	int i;


	const float *error_weights = ewb->texel_weight;

	int partition_count = pt->partition_count;
	float lowparam[4], highparam[4];
	for (i = 0; i < partition_count; i++)
	{
		lowparam[i] = 1e10;
		highparam[i] = -1e10;
	}

	float4 averages[4];
	float4 directions_rgba[4];
	float3 directions_gba[4];
	float3 directions_rba[4];
	float3 directions_rga[4];
	float3 directions_rgb[4];

	line4 lines[4];

	float scale[4];
	float length_squared[4];

	float4 error_weightings[4];
	float4 color_scalefactors[4];
	float4 scalefactors[4];

	int texels_per_block = xdim * ydim * zdim;

	compute_partition_error_color_weightings(xdim, ydim, zdim, ewb, pt, error_weightings, color_scalefactors);

	for (i = 0; i < partition_count; i++)
		scalefactors[i] = normalize(color_scalefactors[i]) * 2.0f;



	compute_averages_and_directions_rgba(pt, blk, ewb, scalefactors, averages, directions_rgba, directions_gba, directions_rba, directions_rga, directions_rgb);

	// if the direction-vector ends up pointing from light to dark, FLIP IT!
	// this will make the first endpoint the darkest one.
	for (i = 0; i < partition_count; i++)
	{
		float4 direc = directions_rgba[i];
		if (direc.x + direc.y + direc.z < 0.0f)
			directions_rgba[i] = float4(0, 0, 0, 0) - direc;
	}

	for (i = 0; i < partition_count; i++)
	{
		lines[i].a = averages[i];
		if (dot(directions_rgba[i], directions_rgba[i]) == 0.0f)
			lines[i].b = normalize(float4(1, 1, 1, 1));
		else
			lines[i].b = normalize(directions_rgba[i]);
	}


	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			for (i = 0; i < partition_count; i++)
			{
				printf("Direction-vector %d: <%f %f %f %f>\n", i, directions_rgba[i].x, directions_rgba[i].y, directions_rgba[i].z, directions_rgba[i].w);
				printf("Line %d A: <%f %f %f %f>\n", i, lines[i].a.x, lines[i].a.y, lines[i].a.z, lines[i].a.w);
				printf("Line %d B: <%f %f %f %f>\n", i, lines[i].b.x, lines[i].b.y, lines[i].b.z, lines[i].b.w);
				printf("Scalefactors %d: <%f %f %f %f>\n", i, scalefactors[i].x, scalefactors[i].y, scalefactors[i].z, scalefactors[i].w);
			}
		}
	#endif


	for (i = 0; i < texels_per_block; i++)
	{
		if (error_weights[i] > 1e-10)
		{
			int partition = pt->partition_of_texel[i];

			float4 point = float4(blk->work_data[4 * i], blk->work_data[4 * i + 1], blk->work_data[4 * i + 2], blk->work_data[4 * i + 3]) * scalefactors[partition];
			line4 l = lines[partition];

			float param = dot(point - l.a, l.b);
			ei->weights[i] = param;
			if (param < lowparam[partition])
				lowparam[partition] = param;
			if (param > highparam[partition])
				highparam[partition] = param;
		}
		else
		{
			ei->weights[i] = -1e38f;
		}
	}


	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			for (i = 0; i < partition_count; i++)
				printf("Partition %d: Lowparam=%f Highparam=%f\n", i, lowparam[i], highparam[i]);
		}
	#endif


	for (i = 0; i < partition_count; i++)
	{
		float length = highparam[i] - lowparam[i];
		if (length < 0)
		{
			lowparam[i] = 0.0f;
			highparam[i] = 1e-7f;
		}


		// it is possible for a uniform-color partition to produce length=0; this
		// causes NaN-production and NaN-propagation later on. Set length to
		// a small value to avoid this problem.
		if (length < 1e-7f)
			length = 1e-7f;

		length_squared[i] = length * length;
		scale[i] = 1.0f / length;

		ei->ep.endpt0[i] = (lines[i].a + lines[i].b * lowparam[i]) / scalefactors[i];
		ei->ep.endpt1[i] = (lines[i].a + lines[i].b * highparam[i]) / scalefactors[i];
	}

	for (i = 0; i < texels_per_block; i++)
	{
		int partition = pt->partition_of_texel[i];
		float idx = (ei->weights[i] - lowparam[partition]) * scale[partition];
		if (idx > 1.0f)
			idx = 1.0f;
		else if (!(idx > 0.0f))
			idx = 0.0f;
		ei->weights[i] = idx;
		ei->weight_error_scale[i] = error_weights[i] * length_squared[partition];
		if (astc_isnan(ei->weight_error_scale[i]))
		{
			ASTC_CODEC_INTERNAL_ERROR;
		}
	}


	// print all the data that this function computes.
	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("%s: %dx%dx%d texels, %d partitions\n", __func__, xdim, ydim, zdim, partition_count);
			printf("Endpoints:\n");
			for (i = 0; i < partition_count; i++)
			{
				printf("%d Low: <%g %g %g %g>\n", i, ei->ep.endpt0[i].x, ei->ep.endpt0[i].y, ei->ep.endpt0[i].z, ei->ep.endpt0[i].w);
				printf("%d High: <%g %g %g %g>\n", i, ei->ep.endpt1[i].x, ei->ep.endpt1[i].y, ei->ep.endpt1[i].z, ei->ep.endpt1[i].w);
			}
			printf("\nIdeal-weights:\n");

			for (i = 0; i < texels_per_block; i++)
			{
				printf("%3d <%2d %2d %2d>=> %g (weight=%g)\n", i, i % xdim, (i / xdim) % ydim, i / (xdim * ydim), ei->weights[i], ei->weight_error_scale[i]);
			}
			printf("\n\n");
		}
	#endif

}



/*

	For a given partitioning, compute: for each partition, the ideal endpoint colors;
	these define a color line for the partition. for each pixel, the ideal position of the pixel on the partition's
	color line. for each pixel, the length of the color line.

	These data allow us to assess the error introduced by removing and quantizing the per-pixel weights.

 */

void compute_endpoints_and_ideal_weights_1_plane(int xdim, int ydim, int zdim, const partition_info * pt, const imageblock * blk, const error_weight_block * ewb, endpoints_and_weights * ei)
{
	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
			printf("%s: texels_per_block=%dx%dx%d\n\n", __func__, xdim, ydim, zdim);
	#endif

	int uses_alpha = imageblock_uses_alpha(xdim, ydim, zdim, blk);
	if (uses_alpha)
	{
		compute_endpoints_and_ideal_weights_rgba(xdim, ydim, zdim, pt, blk, ewb, ei);
	}
	else
	{
		compute_endpoints_and_ideal_weights_3_components(xdim, ydim, zdim, pt, blk, ewb, ei, 0, 1, 2);
	}
}



void compute_endpoints_and_ideal_weights_2_planes(int xdim, int ydim, int zdim, const partition_info * pt,
												  const imageblock * blk, const error_weight_block * ewb, int separate_component,
												  endpoints_and_weights * ei1, endpoints_and_weights * ei2)
{
	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
			printf("%s: texels_per_block=%dx%dx%d, separate_component=%d\n\n", __func__, xdim, ydim, zdim, separate_component);
	#endif

	int uses_alpha = imageblock_uses_alpha(xdim, ydim, zdim, blk);
	switch (separate_component)
	{
	case 0:					// separate weights for red
		if (uses_alpha == 1)
			compute_endpoints_and_ideal_weights_3_components(xdim, ydim, zdim, pt, blk, ewb, ei1, 1, 2, 3);
		else
			compute_endpoints_and_ideal_weights_2_components(xdim, ydim, zdim, pt, blk, ewb, ei1, 1, 2);
		compute_endpoints_and_ideal_weights_1_component(xdim, ydim, zdim, pt, blk, ewb, ei2, 0);
		break;

	case 1:					// separate weights for green
		if (uses_alpha == 1)
			compute_endpoints_and_ideal_weights_3_components(xdim, ydim, zdim, pt, blk, ewb, ei1, 0, 2, 3);
		else
			compute_endpoints_and_ideal_weights_2_components(xdim, ydim, zdim, pt, blk, ewb, ei1, 0, 2);
		compute_endpoints_and_ideal_weights_1_component(xdim, ydim, zdim, pt, blk, ewb, ei2, 1);
		break;

	case 2:					// separate weights for blue
		if (uses_alpha == 1)
			compute_endpoints_and_ideal_weights_3_components(xdim, ydim, zdim, pt, blk, ewb, ei1, 0, 1, 3);
		else
			compute_endpoints_and_ideal_weights_2_components(xdim, ydim, zdim, pt, blk, ewb, ei1, 0, 1);
		compute_endpoints_and_ideal_weights_1_component(xdim, ydim, zdim, pt, blk, ewb, ei2, 2);
		break;

	case 3:					// separate weights for alpha
		if (uses_alpha == 0)
		{
			ASTC_CODEC_INTERNAL_ERROR;
		}
		compute_endpoints_and_ideal_weights_3_components(xdim, ydim, zdim, pt, blk, ewb, ei1, 0, 1, 2);

		compute_endpoints_and_ideal_weights_1_component(xdim, ydim, zdim, pt, blk, ewb, ei2, 3);
		break;
	}

}



/*
   After having computed ideal weights for the case where a weight exists for
   every texel, we want to compute the ideal weights for the case where weights
   exist only for some texels.

   We do this with a steepest-descent grid solver; this works as follows:

   * First, for each actual weight, perform a weighted averaging based on the
     texels affected by the weight.
   * Then, set step size to <some initial value>
   * Then, repeat:
		1: First, compute for each weight how much the error will change
		   if we change the weight by an infinitesimal amount.
		2: This produces a vector that points the direction we should step in.
		   Normalize this vector.
		3: Perform a step
		4: Check if the step actually improved the error. If it did, perform
		   another step in the same direction; repeat until error no longer
		   improves. If the *first* step did not improve error, then we halve
		   the step size.
		5: If the step size dropped down below <some threshold value>,
		   then we quit, else we go back to #1.

   Subroutines: one routine to apply a step and compute the step's effect on
   the error one routine to compute the error change of an infinitesimal
   weight change

   Data structures needed:
   For every decimation pattern, we need:
   * For each weight, a list of <texel, weight> tuples that tell which texels
     the weight influences.
   * For each texel, a list of <texel, weight> tuples that tell which weights
     go into a given texel.
*/

float compute_value_of_texel_flt(int texel_to_get, const decimation_table * it, const float *weights)
{
	const uint8_t *texel_weights = it->texel_weights[texel_to_get];
	const float *texel_weights_float = it->texel_weights_float[texel_to_get];

	return
		(weights[texel_weights[0]] * texel_weights_float[0] + weights[texel_weights[1]] * texel_weights_float[1]) + (weights[texel_weights[2]] * texel_weights_float[2] + weights[texel_weights[3]] * texel_weights_float[3]);
}


static inline float compute_error_of_texel(const endpoints_and_weights * eai, int texel_to_get, const decimation_table * it, const float *weights)
{
	float current_value = compute_value_of_texel_flt(texel_to_get, it, weights);
	float valuedif = current_value - eai->weights[texel_to_get];
	return valuedif * valuedif * eai->weight_error_scale[texel_to_get];
}

/*
	helper function: given
	* for each texel, an ideal weight and an error-modifier these are contained
	  in an endpoints_and_weights data structure.
	* a weight_table data structure
	* for each weight, its current value

    compute the change to overall error that results from adding N to the weight
*/


// this routine is rather heavily optimized since it consumes a lot of CPU time.
void compute_two_error_changes_from_perturbing_weight_infill(const endpoints_and_weights * eai, const decimation_table * it,
															float *infilled_weights, int weight_to_perturb,
															float perturbation1, float perturbation2, float *res1, float *res2)
{
	int num_weights = it->weight_num_texels[weight_to_perturb];
	float error_change0 = 0.0f;
	float error_change1 = 0.0f;
	int i;

	const uint8_t *weight_texel_ptr = it->weight_texel[weight_to_perturb];
	const float *weights_ptr = it->weights_flt[weight_to_perturb];
	for (i = num_weights - 1; i >= 0; i--)
	{
		uint8_t weight_texel = weight_texel_ptr[i];
		float weights = weights_ptr[i];

		float scale = eai->weight_error_scale[weight_texel] * weights;
		float old_weight = infilled_weights[weight_texel];
		float ideal_weight = eai->weights[weight_texel];

		error_change0 += weights * scale;
		error_change1 += (old_weight - ideal_weight) * scale;
	}
	*res1 = error_change0 * (perturbation1 * perturbation1 * (1.0f / (TEXEL_WEIGHT_SUM * TEXEL_WEIGHT_SUM))) + error_change1 * (perturbation1 * (2.0f / TEXEL_WEIGHT_SUM));
	*res2 = error_change0 * (perturbation2 * perturbation2 * (1.0f / (TEXEL_WEIGHT_SUM * TEXEL_WEIGHT_SUM))) + error_change1 * (perturbation2 * (2.0f / TEXEL_WEIGHT_SUM));
}



float compute_error_of_weight_set(const endpoints_and_weights * eai, const decimation_table * it, const float *weights)
{
	int i;
	int texel_count = it->num_texels;
	float error_summa = 0.0;
	for (i = 0; i < texel_count; i++)
		error_summa += compute_error_of_texel(eai, i, it, weights);
	return error_summa;
}


/*
	Given a complete weight set and a decimation table, try to
	compute the optimal weight set (assuming infinite precision)
	given the selected decimation table.
*/

void compute_ideal_weights_for_decimation_table(const endpoints_and_weights * eai, const decimation_table * it, float *weight_set, float *weights)
{
	int i, j, k;

	int blockdim = (int)floor(sqrt((float)it->num_texels) + 0.5f);
	int texels_per_block = it->num_texels;
	int weight_count = it->num_weights;

	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("%s : decimation from %d to %d weights\n\n", __func__, it->num_texels, it->num_weights);
			printf("Input weight set:\n");
			for (i = 0; i < it->num_texels; i++)
			{
				printf("%3d <%2d %2d> : %g\n", i, i % blockdim, i / blockdim, eai->weights[i]);
			}
			printf("\n");
		}
	#endif


	// perform a shortcut in the case of a complete decimation table
	if (texels_per_block == weight_count)
	{
		#ifdef DEBUG_PRINT_DIAGNOSTICS
			if (print_diagnostics)
				printf("%s : no decimation actually needed: early-out\n\n", __func__);
		#endif

		for (i = 0; i < it->num_texels; i++)
		{
			int texel = it->weight_texel[i][0];
			weight_set[i] = eai->weights[texel];
			weights[i] = eai->weight_error_scale[texel];
		}
		return;
	}


	// if the shortcut is not available, we will instead compute a simple estimate
	// and perform three rounds of refinement on that estimate.

	float initial_weight_set[MAX_WEIGHTS_PER_BLOCK];
	float infilled_weights[MAX_TEXELS_PER_BLOCK];

	// compute an initial average for each weight.
	for (i = 0; i < weight_count; i++)
	{
		int texel_count = it->weight_num_texels[i];

		float weight_weight = 1e-10f;	// to avoid 0/0 later on
		float initial_weight = 0.0f;
		for (j = 0; j < texel_count; j++)
		{
			int texel = it->weight_texel[i][j];
			float weight = it->weights_flt[i][j];
			float contrib_weight = weight * eai->weight_error_scale[texel];
			weight_weight += contrib_weight;
			initial_weight += eai->weights[texel] * contrib_weight;
		}

		weights[i] = weight_weight;
		weight_set[i] = initial_weight / weight_weight;	// this is the 0/0 that is to be avoided.
	}

	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			// stash away the initial-weight estimates for later printing
			for (i = 0; i < weight_count; i++)
				initial_weight_set[i] = weight_set[i];
		}
	#endif


	for (i = 0; i < texels_per_block; i++)
	{
		infilled_weights[i] = compute_value_of_texel_flt(i, it, weight_set);
	}

	const float stepsizes[2] = { 0.25f, 0.125f };

	for (j = 0; j < 2; j++)
	{
		float stepsize = stepsizes[j];

		#ifdef DEBUG_PRINT_DIAGNOSTICS
			if (print_diagnostics)
				printf("Pass %d, step=%f  \n", j, stepsize);
		#endif

		for (i = 0; i < weight_count; i++)
		{
			float weight_val = weight_set[i];
			float error_change_up, error_change_down;
			compute_two_error_changes_from_perturbing_weight_infill(eai, it, infilled_weights, i, stepsize, -stepsize, &error_change_up, &error_change_down);

			/*
				assume that the error-change function behaves like a quadratic function in the interval examined,
				with "error_change_up" and "error_change_down" defining the function at the endpoints
				of the interval. Then, find the position where the function's derivative is zero.

				The "fabs(b) >= a" check tests several conditions in one:
					if a is negative, then the 2nd derivative of the function is negative;
					in this case, f'(x)=0 will maximize error.
				If fabs(b) > fabs(a), then f'(x)=0 will lie outside the interval altogether.
				If a and b are both 0, then set step to 0;
					otherwise, we end up computing 0/0, which produces a lethal NaN.
				We can get an a=b=0 situation if an error weight is 0 in the wrong place.
			*/

			float step;
			float a = (error_change_up + error_change_down) * 2.0f;
			float b = error_change_down - error_change_up;
			if (fabs(b) >= a)
			{
				if (a <= 0.0f)
				{
					if (error_change_up < error_change_down)
						step = 1;
					else if (error_change_up > error_change_down)
						step = -1;

					else
						step = 0;

				}
				else
				{
					if (a < 1e-10f)
						a = 1e-10f;
					step = b / a;
					if (step < -1.0f)
						step = -1.0f;
					else if (step > 1.0f)
						step = 1.0f;
				}
			}
			else
				step = b / a;


			step *= stepsize;
			float new_weight_val = weight_val + step;

			// update the weight
			weight_set[i] = new_weight_val;
			// update the infilled-weights
			int num_weights = it->weight_num_texels[i];
			float perturbation = (new_weight_val - weight_val) * (1.0f / TEXEL_WEIGHT_SUM);
			const uint8_t *weight_texel_ptr = it->weight_texel[i];
			const float *weights_ptr = it->weights_flt[i];
			for (k = num_weights - 1; k >= 0; k--)
			{
				uint8_t weight_texel = weight_texel_ptr[k];
				float weight_weight = weights_ptr[k];
				infilled_weights[weight_texel] += perturbation * weight_weight;
			}

		}

		#ifdef DEBUG_PRINT_DIAGNOSTICS
			if (print_diagnostics)
				printf("\n");
		#endif
	}



	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("Error weights, initial-estimates, final-results\n");
			for (i = 0; i < weight_count; i++)
			{
				printf("%2d -> weight=%g, initial=%g  final=%g\n", i, weights[i], initial_weight_set[i], weight_set[i]);
			}
			printf("\n");
		}
	#endif

	return;
}




/*
	For a decimation table, try to compute an optimal weight set, assuming
	that the weights are quantized and subject to a transfer function.

	We do this as follows:
	First, we take the initial weights and quantize them. This is our initial estimate.
	Then, go through the weights one by one; try to perturb then up and down one weight at a
	time; apply any perturbations that improve overall error
	Repeat until we have made a complete processing pass over all weights without
	triggering any perturbations *OR* we have run 4 full passes.
*/

void compute_ideal_quantized_weights_for_decimation_table(const endpoints_and_weights * eai,
														  const decimation_table * it,
														  float low_bound, float high_bound, const float *weight_set_in, float *weight_set_out, uint8_t * quantized_weight_set, int quantization_level)
{
	int i;
	int weight_count = it->num_weights;
	int texels_per_block = it->num_texels;

	const quantization_and_transfer_table *qat = &(quant_and_xfer_tables[quantization_level]);

	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("%s : texels-per-block=%d,  weights=%d,  quantization-level=%d\n\n", __func__, texels_per_block, weight_count, quantization_level);

			printf("Weight values before quantization:\n");
			for (i = 0; i < weight_count; i++)
				printf("%3d : %g\n", i, weight_set_in[i]);

			printf("Low-bound: %f  High-bound: %f\n", low_bound, high_bound);
		}
	#endif


	// quantize the weight set using both the specified low/high bounds and the
	// standard 0..1 weight bounds.

	/*
	   WTF issue that we need to examine some time
	*/

	if (!((high_bound - low_bound) > 0.5f))
	{
		low_bound = 0.0f;
		high_bound = 1.0f;
	}

	float rscale = high_bound - low_bound;
	float scale = 1.0f / rscale;

	// rescale the weights so that
	// low_bound -> 0
	// high_bound -> 1
	// OK: first, subtract low_bound, then divide by (high_bound - low_bound)

	for (i = 0; i < weight_count; i++)
		weight_set_out[i] = (weight_set_in[i] - low_bound) * scale;



	static const float quantization_step_table[12] = {
		1.0f / 1.0f,
		1.0f / 2.0f,
		1.0f / 3.0f,
		1.0f / 4.0f,
		1.0f / 5.0f,
		1.0f / 7.0f,
		1.0f / 9.0f,
		1.0f / 11.0f,
		1.0f / 15.0f,
		1.0f / 19.0f,
		1.0f / 23.0f,
		1.0f / 31.0f,
	};

	float quantization_cutoff = quantization_step_table[quantization_level] * 0.333f;


	int is_perturbable[MAX_WEIGHTS_PER_BLOCK];
	int perturbable_count = 0;

	// quantize the weight set
	for (i = 0; i < weight_count; i++)
	{
		float ix0 = weight_set_out[i];
		if (ix0 < 0.0f)
			ix0 = 0.0f;
		if (ix0 > 1.0f)
			ix0 = 1.0f;
		float ix = ix0;

		ix *= 1024.0f;
		int ix2 = (int)floor(ix + 0.5f);
		int weight = qat->closest_quantized_weight[ix2];

		ix = qat->unquantized_value_flt[weight];
		weight_set_out[i] = ix;
		quantized_weight_set[i] = weight;

		// test whether the error of the weight is greater than 1/3 of the weight spacing;
		// if it is not, then it is flagged as "not perturbable". This causes a
		// quality loss of about 0.002 dB, which is totally worth the speedup we're getting.
		is_perturbable[i] = 0;
		if (fabs(ix - ix0) > quantization_cutoff)
		{
			is_perturbable[i] = 1;
			perturbable_count++;
		}
	}


	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("Weight values after initial quantization:\n");
			for (i = 0; i < weight_count; i++)
				printf("%3d : %g <%d>\n", i, weight_set_out[i], quantized_weight_set[i]);
		}
	#endif




	// if the decimation table is complete, the quantization above was all we needed to do,
	// so we can early-out.
	if (it->num_weights == it->num_texels)
	{
		// invert the weight-scaling that was done initially
		// 0 -> low_bound
		// 1 -> high_bound

		rscale = high_bound - low_bound;
		for (i = 0; i < weight_count; i++)
			weight_set_out[i] = (weight_set_out[i] * rscale) + low_bound;

		#ifdef DEBUG_PRINT_DIAGNOSTICS
			if (print_diagnostics)
			{
				printf("Weight values after adjustment:\n");
				for (i = 0; i < weight_count; i++)
					printf("%3d : %g <%d> <error=%g>\n", i, weight_set_out[i], quantized_weight_set[i], weight_set_out[i] - weight_set_in[i]);
				printf("\n");
				printf("%s: Early-out\n\n", __func__);

			}
		#endif

		return;
	}


	int weights_tested = 0;

	#ifdef DEBUG_PRINT_DIAGNOSTICS
		int perturbation_count = 0;
	#endif

	// if no weights are flagged as perturbable, don't try to perturb them.
	// if only one weight is flagged as perturbable, perturbation is also pointless.
	if (perturbable_count > 1)
	{
		endpoints_and_weights eaix;
		for (i = 0; i < texels_per_block; i++)
		{
			eaix.weights[i] = (eai->weights[i] - low_bound) * scale;
			eaix.weight_error_scale[i] = eai->weight_error_scale[i];
		}

		float infilled_weights[MAX_TEXELS_PER_BLOCK];
		for (i = 0; i < texels_per_block; i++)
			infilled_weights[i] = compute_value_of_texel_flt(i, it, weight_set_out);

		int weight_to_perturb = 0;
		int weights_since_last_perturbation = 0;
		int num_weights = it->num_weights;

		while (weights_since_last_perturbation < num_weights && weights_tested < num_weights * 4)
		{
			int do_quant_mod = 0;
			if (is_perturbable[weight_to_perturb])
			{

				int weight_val = quantized_weight_set[weight_to_perturb];
				int weight_next_up = qat->next_quantized_value[weight_val];
				int weight_next_down = qat->prev_quantized_value[weight_val];
				float flt_weight_val = qat->unquantized_value_flt[weight_val];
				float flt_weight_next_up = qat->unquantized_value_flt[weight_next_up];
				float flt_weight_next_down = qat->unquantized_value_flt[weight_next_down];


				int do_quant_mod = 0;

				float error_change_up, error_change_down;

				// compute the error change from perturbing the weight either up or down.
				compute_two_error_changes_from_perturbing_weight_infill(&eaix,
																	   it,
																	   infilled_weights,
																	   weight_to_perturb,
																	   (flt_weight_next_up - flt_weight_val), (flt_weight_next_down - flt_weight_val), &error_change_up, &error_change_down);

				int new_weight_val;
				float flt_new_weight_val;
				if (weight_val != weight_next_up && error_change_up < 0.0f)
				{
					do_quant_mod = 1;
					new_weight_val = weight_next_up;
					flt_new_weight_val = flt_weight_next_up;
				}
				else if (weight_val != weight_next_down && error_change_down < 0.0f)
				{
					do_quant_mod = 1;
					new_weight_val = weight_next_down;
					flt_new_weight_val = flt_weight_next_down;
				}


				if (do_quant_mod)
				{

					// update the weight.
					weight_set_out[weight_to_perturb] = flt_new_weight_val;
					quantized_weight_set[weight_to_perturb] = new_weight_val;

					// update the infilled-weights
					int num_weights = it->weight_num_texels[weight_to_perturb];
					float perturbation = (flt_new_weight_val - flt_weight_val) * (1.0f / TEXEL_WEIGHT_SUM);
					const uint8_t *weight_texel_ptr = it->weight_texel[weight_to_perturb];
					const float *weights_ptr = it->weights_flt[weight_to_perturb];
					for (i = num_weights - 1; i >= 0; i--)
					{
						uint8_t weight_texel = weight_texel_ptr[i];
						float weights = weights_ptr[i];
						infilled_weights[weight_texel] += perturbation * weights;
					}

					#ifdef DEBUG_PRINT_DIAGNOSTICS
						if (print_diagnostics)
						{
							printf("Perturbation of weight %d : %g\n", weight_to_perturb, perturbation * (float)TEXEL_WEIGHT_SUM);
							perturbation_count++;
						}
					#endif
				}
			}

			if (do_quant_mod)
				weights_since_last_perturbation = 0;
			else
				weights_since_last_perturbation++;

			weight_to_perturb++;
			if (weight_to_perturb >= num_weights)
				weight_to_perturb -= num_weights;

			weights_tested++;
		}
	}

	// invert the weight-scaling that was done initially
	// 0 -> low_bound
	// 1 -> high_bound


	for (i = 0; i < weight_count; i++)
		weight_set_out[i] = (weight_set_out[i] * rscale) + low_bound;



	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("%d weights, %d weight tests, %d perturbations\n", weight_count, weights_tested, perturbation_count);
			printf("Weight values after adjustment:\n");
			for (i = 0; i < weight_count; i++)
				printf("%3d : %g <%d>\n", i, weight_set_out[i], quantized_weight_set[i]);
			printf("\n");
		}
	#endif

}






static inline float mat_square_sum(mat2 p)
{
	float a = p.v[0].x;
	float b = p.v[0].y;
	float c = p.v[1].x;
	float d = p.v[1].y;
	return a * a + b * b + c * c + d * d;
}










/*
   for a given weight set, we wish to recompute the colors so that they are optimal for a particular weight set. */
void recompute_ideal_colors(int xdim, int ydim, int zdim, int weight_quantization_mode, endpoints * ep,	// contains the endpoints we wish to update
							float4 * rgbs_vectors,	// used to return RGBS-vectors. (endpoint mode #6)
							float4 * rgbo_vectors,	// used to return RGBO-vectors. (endpoint mode #7)
							float2 * lum_vectors,	// used to return luminance-vectors.
							const uint8_t * weight_set8,	// the current set of weight values
							const uint8_t * plane2_weight_set8,	// NULL if plane 2 is not actually used.
							int plane2_color_component,	// color component for 2nd plane of weights; -1 if the 2nd plane of weights is not present
							const partition_info * pi, const decimation_table * it, const imageblock * pb,	// picture-block containing the actual data.
							const error_weight_block * ewb)
{
	int i, j;

	int texels_per_block = xdim * ydim * zdim;

	const quantization_and_transfer_table *qat = &(quant_and_xfer_tables[weight_quantization_mode]);

	float weight_set[MAX_WEIGHTS_PER_BLOCK];
	float plane2_weight_set[MAX_WEIGHTS_PER_BLOCK];

	for (i = 0; i < it->num_weights; i++)
	{
		weight_set[i] = qat->unquantized_value_flt[weight_set8[i]];
	}
	if (plane2_weight_set8)
	{
		for (i = 0; i < it->num_weights; i++)
			plane2_weight_set[i] = qat->unquantized_value_flt[plane2_weight_set8[i]];
	}

	int partition_count = pi->partition_count;

	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("%s : %dx%dx%d texels_per_block, %d partitions, plane2-color-component=%d\n\n", __func__, xdim, ydim, zdim, partition_count, plane2_color_component);

			printf("Pre-adjustment endpoint-colors: \n");
			for (i = 0; i < partition_count; i++)
			{
				printf("%d Low  <%g %g %g %g>\n", i, ep->endpt0[i].x, ep->endpt0[i].y, ep->endpt0[i].z, ep->endpt0[i].w);
				printf("%d High <%g %g %g %g>\n", i, ep->endpt1[i].x, ep->endpt1[i].y, ep->endpt1[i].z, ep->endpt1[i].w);
			}
		}
	#endif


	mat2 pmat1_red[4], pmat1_green[4], pmat1_blue[4], pmat1_alpha[4], pmat1_lum[4], pmat1_scale[4];	// matrices for plane of weights 1
	mat2 pmat2_red[4], pmat2_green[4], pmat2_blue[4], pmat2_alpha[4];	// matrices for plane of weights 2
	float2 red_vec[4];
	float2 green_vec[4];
	float2 blue_vec[4];
	float2 alpha_vec[4];
	float2 lum_vec[4];
	float2 scale_vec[4];

	for (i = 0; i < partition_count; i++)
	{
		for (j = 0; j < 2; j++)
		{
			pmat1_red[i].v[j] = float2(0, 0);
			pmat2_red[i].v[j] = float2(0, 0);
			pmat1_green[i].v[j] = float2(0, 0);
			pmat2_green[i].v[j] = float2(0, 0);
			pmat1_blue[i].v[j] = float2(0, 0);
			pmat2_blue[i].v[j] = float2(0, 0);
			pmat1_alpha[i].v[j] = float2(0, 0);
			pmat2_alpha[i].v[j] = float2(0, 0);
			pmat1_lum[i].v[j] = float2(0, 0);
			pmat1_scale[i].v[j] = float2(0, 0);
		}
		red_vec[i] = float2(0, 0);
		green_vec[i] = float2(0, 0);
		blue_vec[i] = float2(0, 0);
		alpha_vec[i] = float2(0, 0);
		lum_vec[i] = float2(0, 0);
		scale_vec[i] = float2(0, 0);
	}


	float wmin1[4], wmax1[4];
	float wmin2[4], wmax2[4];
	float red_weight_sum[4];
	float green_weight_sum[4];
	float blue_weight_sum[4];
	float alpha_weight_sum[4];
	float lum_weight_sum[4];
	float scale_weight_sum[4];

	float red_weight_weight_sum[4];
	float green_weight_weight_sum[4];
	float blue_weight_weight_sum[4];

	float psum[4];				// sum of (weight * qweight^2) across (red,green,blue)
	float qsum[4];				// sum of (weight * qweight * texelval) across (red,green,blue)


	for (i = 0; i < partition_count; i++)
	{
		wmin1[i] = 1.0f;
		wmax1[i] = 0.0f;
		wmin2[i] = 1.0f;
		wmax2[i] = 0.0f;
		red_weight_sum[i] = 1e-17f;
		green_weight_sum[i] = 1e-17f;
		blue_weight_sum[i] = 1e-17f;
		alpha_weight_sum[i] = 1e-17f;

		lum_weight_sum[i] = 1e-17f;
		scale_weight_sum[i] = 1e-17f;

		red_weight_weight_sum[i] = 1e-17f;
		green_weight_weight_sum[i] = 1e-17f;
		blue_weight_weight_sum[i] = 1e-17f;

		psum[i] = 1e-17f;
		qsum[i] = 1e-17f;
	}


	// for each partition, compute the direction that an RGB-scale color endpoint pair would have.
	float3 rgb_sum[4];
	float3 rgb_weight_sum[4];
	float3 scale_directions[4];
	float scale_min[4];
	float scale_max[4];
	float lum_min[4];
	float lum_max[4];

	for (i = 0; i < partition_count; i++)
	{
		rgb_sum[i] = float3(1e-17f, 1e-17f, 1e-17f);
		rgb_weight_sum[i] = float3(1e-17f, 1e-17f, 1e-17f);
	}


	for (i = 0; i < texels_per_block; i++)
	{
		float3 rgb = float3(pb->work_data[4 * i], pb->work_data[4 * i + 1], pb->work_data[4 * i + 2]);
		float3 rgb_weight = float3(ewb->texel_weight_r[i],
								   ewb->texel_weight_g[i],
								   ewb->texel_weight_b[i]);

		int part = pi->partition_of_texel[i];
		rgb_sum[part] = rgb_sum[part] + (rgb * rgb_weight);
		rgb_weight_sum[part] = rgb_weight_sum[part] + rgb_weight;
	}

	for (i = 0; i < partition_count; i++)
	{
		scale_directions[i] = normalize(rgb_sum[i] / rgb_weight_sum[i]);
		scale_max[i] = 0.0f;
		scale_min[i] = 1e10f;
		lum_max[i] = 0.0f;
		lum_min[i] = 1e10f;
	}





	for (i = 0; i < texels_per_block; i++)
	{
		float r = pb->work_data[4 * i];
		float g = pb->work_data[4 * i + 1];
		float b = pb->work_data[4 * i + 2];
		float a = pb->work_data[4 * i + 3];

		int part = pi->partition_of_texel[i];
		float idx0 = it ? compute_value_of_texel_flt(i, it, weight_set) : weight_set[i];
		float om_idx0 = 1.0f - idx0;

		if (idx0 > wmax1[part])
			wmax1[part] = idx0;
		if (idx0 < wmin1[part])
			wmin1[part] = idx0;

		float red_weight = ewb->texel_weight_r[i];
		float green_weight = ewb->texel_weight_g[i];
		float blue_weight = ewb->texel_weight_b[i];
		float alpha_weight = ewb->texel_weight_a[i];

		float lum_weight = (red_weight + green_weight + blue_weight);
		float scale_weight = lum_weight;

		float lum = (r * red_weight + g * green_weight + b * blue_weight) / lum_weight;
		float3 scale_direction = scale_directions[part];
		float scale = dot(scale_direction, float3(r, g, b));
		if (lum < lum_min[part])
			lum_min[part] = scale;
		if (lum > lum_max[part])
			lum_max[part] = scale;
		if (scale < scale_min[part])
			scale_min[part] = scale;
		if (scale > scale_max[part])
			scale_max[part] = scale;


		red_weight_sum[part] += red_weight;
		green_weight_sum[part] += green_weight;
		blue_weight_sum[part] += blue_weight;
		alpha_weight_sum[part] += alpha_weight;
		lum_weight_sum[part] += lum_weight;
		scale_weight_sum[part] += scale_weight;


		pmat1_red[part].v[0].x += om_idx0 * om_idx0 * red_weight;
		pmat1_red[part].v[0].y += idx0 * om_idx0 * red_weight;
		pmat1_red[part].v[1].x += idx0 * om_idx0 * red_weight;
		pmat1_red[part].v[1].y += idx0 * idx0 * red_weight;

		pmat1_green[part].v[0].x += om_idx0 * om_idx0 * green_weight;
		pmat1_green[part].v[0].y += idx0 * om_idx0 * green_weight;
		pmat1_green[part].v[1].x += idx0 * om_idx0 * green_weight;
		pmat1_green[part].v[1].y += idx0 * idx0 * green_weight;

		pmat1_blue[part].v[0].x += om_idx0 * om_idx0 * blue_weight;
		pmat1_blue[part].v[0].y += idx0 * om_idx0 * blue_weight;
		pmat1_blue[part].v[1].x += idx0 * om_idx0 * blue_weight;
		pmat1_blue[part].v[1].y += idx0 * idx0 * blue_weight;

		pmat1_alpha[part].v[0].x += om_idx0 * om_idx0 * alpha_weight;
		pmat1_alpha[part].v[0].y += idx0 * om_idx0 * alpha_weight;
		pmat1_alpha[part].v[1].x += idx0 * om_idx0 * alpha_weight;
		pmat1_alpha[part].v[1].y += idx0 * idx0 * alpha_weight;

		pmat1_lum[part].v[0].x += om_idx0 * om_idx0 * lum_weight;
		pmat1_lum[part].v[0].y += idx0 * om_idx0 * lum_weight;
		pmat1_lum[part].v[1].x += idx0 * om_idx0 * lum_weight;
		pmat1_lum[part].v[1].y += idx0 * idx0 * lum_weight;

		pmat1_scale[part].v[0].x += om_idx0 * om_idx0 * scale_weight;
		pmat1_scale[part].v[0].y += idx0 * om_idx0 * scale_weight;
		pmat1_scale[part].v[1].x += idx0 * om_idx0 * scale_weight;
		pmat1_scale[part].v[1].y += idx0 * idx0 * scale_weight;

		float idx1 = 0.0f, om_idx1 = 0.0f;
		if (plane2_weight_set8)
		{
			idx1 = it ? compute_value_of_texel_flt(i, it, plane2_weight_set) : plane2_weight_set[i];
			om_idx1 = 1.0f - idx1;
			if (idx1 > wmax2[part])
				wmax2[part] = idx1;
			if (idx1 < wmin2[part])
				wmin2[part] = idx1;

			pmat2_red[part].v[0].x += om_idx1 * om_idx1 * red_weight;
			pmat2_red[part].v[0].y += idx1 * om_idx1 * red_weight;
			pmat2_red[part].v[1].x += idx1 * om_idx1 * red_weight;
			pmat2_red[part].v[1].y += idx1 * idx1 * red_weight;

			pmat2_green[part].v[0].x += om_idx1 * om_idx1 * green_weight;
			pmat2_green[part].v[0].y += idx1 * om_idx1 * green_weight;
			pmat2_green[part].v[1].x += idx1 * om_idx1 * green_weight;
			pmat2_green[part].v[1].y += idx1 * idx1 * green_weight;

			pmat2_blue[part].v[0].x += om_idx1 * om_idx1 * blue_weight;
			pmat2_blue[part].v[0].y += idx1 * om_idx1 * blue_weight;
			pmat2_blue[part].v[1].x += idx1 * om_idx1 * blue_weight;
			pmat2_blue[part].v[1].y += idx1 * idx1 * blue_weight;

			pmat2_alpha[part].v[0].x += om_idx1 * om_idx1 * alpha_weight;
			pmat2_alpha[part].v[0].y += idx1 * om_idx1 * alpha_weight;
			pmat2_alpha[part].v[1].x += idx1 * om_idx1 * alpha_weight;
			pmat2_alpha[part].v[1].y += idx1 * idx1 * alpha_weight;
		}

		float red_idx = (plane2_color_component == 0) ? idx1 : idx0;
		float green_idx = (plane2_color_component == 1) ? idx1 : idx0;
		float blue_idx = (plane2_color_component == 2) ? idx1 : idx0;
		float alpha_idx = (plane2_color_component == 3) ? idx1 : idx0;


		red_vec[part].x += (red_weight * r) * (1.0f - red_idx);
		green_vec[part].x += (green_weight * g) * (1.0f - green_idx);
		blue_vec[part].x += (blue_weight * b) * (1.0f - blue_idx);
		alpha_vec[part].x += (alpha_weight * a) * (1.0f - alpha_idx);
		lum_vec[part].x += (lum_weight * lum) * om_idx0;
		scale_vec[part].x += (scale_weight * scale) * om_idx0;

		red_vec[part].y += (red_weight * r) * red_idx;
		green_vec[part].y += (green_weight * g) * green_idx;
		blue_vec[part].y += (blue_weight * b) * blue_idx;
		alpha_vec[part].y += (alpha_weight * a) * alpha_idx;
		lum_vec[part].y += (lum_weight * lum) * idx0;
		scale_vec[part].y += (scale_weight * scale) * idx0;

		red_weight_weight_sum[part] += red_weight * red_idx;
		green_weight_weight_sum[part] += green_weight * green_idx;
		blue_weight_weight_sum[part] += blue_weight * blue_idx;

		psum[part] += red_weight * red_idx * red_idx + green_weight * green_idx * green_idx + blue_weight * blue_idx * blue_idx;

	}

	// calculations specific to mode #7, the HDR RGB-scale mode.
	float red_sum[4];
	float green_sum[4];
	float blue_sum[4];
	for (i = 0; i < partition_count; i++)
	{
		red_sum[i] = red_vec[i].x + red_vec[i].y;
		green_sum[i] = green_vec[i].x + green_vec[i].y;
		blue_sum[i] = blue_vec[i].x + blue_vec[i].y;
		qsum[i] = red_vec[i].y + green_vec[i].y + blue_vec[i].y;
	}

	// RGB+offset for HDR endpoint mode #7
	int rgbo_fail[4];
	for (i = 0; i < partition_count; i++)
	{
		mat4 mod7_mat;
		mod7_mat.v[0] = float4(red_weight_sum[i], 0.0f, 0.0f, red_weight_weight_sum[i]);
		mod7_mat.v[1] = float4(0.0f, green_weight_sum[i], 0.0f, green_weight_weight_sum[i]);
		mod7_mat.v[2] = float4(0.0f, 0.0f, blue_weight_sum[i], blue_weight_weight_sum[i]);
		mod7_mat.v[3] = float4(red_weight_weight_sum[i], green_weight_weight_sum[i], blue_weight_weight_sum[i], psum[i]);

		float4 vect = float4(red_sum[i], green_sum[i], blue_sum[i], qsum[i]);

		#ifdef DEBUG_CAPTURE_NAN
			fedisableexcept(FE_DIVBYZERO | FE_INVALID);
		#endif

		mat4 rmod7_mat = invert(mod7_mat);
		float4 rgbovec = transform(rmod7_mat, vect);
		rgbo_vectors[i] = rgbovec;

		// we will occasionally get a failure due to a singular matrix. Record whether such a
		// failure has taken place; if it did, compute rgbo_vectors[] with a different method
		// later on.
		float chkval = dot(rgbovec, rgbovec);
		rgbo_fail[i] = chkval != chkval;

		#ifdef DEBUG_CAPTURE_NAN
			feenableexcept(FE_DIVBYZERO | FE_INVALID);
		#endif
	}



	// initialize the luminance and scale vectors with a reasonable default,
	// just in case the subsequent calculation blows up.
	for (i = 0; i < partition_count; i++)
	{

		#ifdef DEBUG_CAPTURE_NAN
			fedisableexcept(FE_DIVBYZERO | FE_INVALID);
		#endif

		float scalediv = scale_min[i] / scale_max[i];
		if (!(scalediv > 0.0f))
			scalediv = 0.0f;	// set to zero if scalediv is zero, negative, or NaN.

		#ifdef DEBUG_CAPTURE_NAN
			feenableexcept(FE_DIVBYZERO | FE_INVALID);
		#endif

		if (scalediv > 1.0f)
			scalediv = 1.0f;

		rgbs_vectors[i] = float4(scale_directions[i] * scale_max[i], scalediv);
		lum_vectors[i] = float2(lum_min[i], lum_max[i]);
	}



	for (i = 0; i < partition_count; i++)
	{

		if (wmin1[i] >= wmax1[i] * 0.999)
		{
			// if all weights in the partition were equal, then just take average
			// of all colors in the partition and use that as both endpoint colors.
			float4 avg = float4((red_vec[i].x + red_vec[i].y) / red_weight_sum[i],
								(green_vec[i].x + green_vec[i].y) / green_weight_sum[i],
								(blue_vec[i].x + blue_vec[i].y) / blue_weight_sum[i],
								(alpha_vec[i].x + alpha_vec[i].y) / alpha_weight_sum[i]);

			if (plane2_color_component != 0 && avg.x == avg.x)
				ep->endpt0[i].x = ep->endpt1[i].x = avg.x;
			if (plane2_color_component != 1 && avg.y == avg.y)
				ep->endpt0[i].y = ep->endpt1[i].y = avg.y;
			if (plane2_color_component != 2 && avg.z == avg.z)
				ep->endpt0[i].z = ep->endpt1[i].z = avg.z;
			if (plane2_color_component != 3 && avg.w == avg.w)
				ep->endpt0[i].w = ep->endpt1[i].w = avg.w;

			rgbs_vectors[i] = float4(scale_directions[i] * scale_max[i], 1.0f);
			float lumval = (red_vec[i].x + red_vec[i].y + green_vec[i].x + green_vec[i].y + blue_vec[i].x + blue_vec[i].y) / (red_weight_sum[i] + green_weight_sum[i] + blue_weight_sum[i]);
			lum_vectors[i] = float2(lumval, lumval);
		}

		else
		{

			// otherwise, complete the analytic calculation of ideal-endpoint-values
			// for the given set of texel weights and pixel colors.

			#ifdef DEBUG_CAPTURE_NAN
				fedisableexcept(FE_DIVBYZERO | FE_INVALID);
			#endif

			float red_det1 = determinant(pmat1_red[i]);
			float green_det1 = determinant(pmat1_green[i]);
			float blue_det1 = determinant(pmat1_blue[i]);
			float alpha_det1 = determinant(pmat1_alpha[i]);
			float lum_det1 = determinant(pmat1_lum[i]);
			float scale_det1 = determinant(pmat1_scale[i]);

			float red_mss1 = mat_square_sum(pmat1_red[i]);
			float green_mss1 = mat_square_sum(pmat1_green[i]);
			float blue_mss1 = mat_square_sum(pmat1_blue[i]);
			float alpha_mss1 = mat_square_sum(pmat1_alpha[i]);
			float lum_mss1 = mat_square_sum(pmat1_lum[i]);
			float scale_mss1 = mat_square_sum(pmat1_scale[i]);


			#ifdef DEBUG_PRINT_DIAGNOSTICS
				if (print_diagnostics)
					printf("Plane-1 partition %d determinants: R=%g G=%g B=%g A=%g L=%g S=%g\n", i, red_det1, green_det1, blue_det1, alpha_det1, lum_det1, scale_det1);
			#endif

			pmat1_red[i] = invert(pmat1_red[i]);
			pmat1_green[i] = invert(pmat1_green[i]);
			pmat1_blue[i] = invert(pmat1_blue[i]);
			pmat1_alpha[i] = invert(pmat1_alpha[i]);
			pmat1_lum[i] = invert(pmat1_lum[i]);
			pmat1_scale[i] = invert(pmat1_scale[i]);

			float4 ep0 = float4(dot(pmat1_red[i].v[0], red_vec[i]),
								dot(pmat1_green[i].v[0], green_vec[i]),
								dot(pmat1_blue[i].v[0], blue_vec[i]),
								dot(pmat1_alpha[i].v[0], alpha_vec[i]));
			float4 ep1 = float4(dot(pmat1_red[i].v[1], red_vec[i]),
								dot(pmat1_green[i].v[1], green_vec[i]),
								dot(pmat1_blue[i].v[1], blue_vec[i]),
								dot(pmat1_alpha[i].v[1], alpha_vec[i]));

			float lum_ep0 = dot(pmat1_lum[i].v[0], lum_vec[i]);
			float lum_ep1 = dot(pmat1_lum[i].v[1], lum_vec[i]);
			float scale_ep0 = dot(pmat1_scale[i].v[0], scale_vec[i]);
			float scale_ep1 = dot(pmat1_scale[i].v[1], scale_vec[i]);


			if (plane2_color_component != 0 && fabs(red_det1) > (red_mss1 * 1e-4f) && ep0.x == ep0.x && ep1.x == ep1.x)
			{
				ep->endpt0[i].x = ep0.x;
				ep->endpt1[i].x = ep1.x;
			}
			if (plane2_color_component != 1 && fabs(green_det1) > (green_mss1 * 1e-4f) && ep0.y == ep0.y && ep1.y == ep1.y)
			{
				ep->endpt0[i].y = ep0.y;
				ep->endpt1[i].y = ep1.y;
			}
			if (plane2_color_component != 2 && fabs(blue_det1) > (blue_mss1 * 1e-4f) && ep0.z == ep0.z && ep1.z == ep1.z)
			{
				ep->endpt0[i].z = ep0.z;
				ep->endpt1[i].z = ep1.z;
			}
			if (plane2_color_component != 3 && fabs(alpha_det1) > (alpha_mss1 * 1e-4f) && ep0.w == ep0.w && ep1.w == ep1.w)
			{
				ep->endpt0[i].w = ep0.w;
				ep->endpt1[i].w = ep1.w;
			}

			if (fabs(lum_det1) > (lum_mss1 * 1e-4f) && lum_ep0 == lum_ep0 && lum_ep1 == lum_ep1 && lum_ep0 < lum_ep1)
			{
				lum_vectors[i].x = lum_ep0;
				lum_vectors[i].y = lum_ep1;
			}
			if (fabs(scale_det1) > (scale_mss1 * 1e-4f) && scale_ep0 == scale_ep0 && scale_ep1 == scale_ep1 && scale_ep0 < scale_ep1)
			{
				float scalediv = scale_ep0 / scale_ep1;
				rgbs_vectors[i] = float4(scale_directions[i] * scale_ep1, scalediv);
			}


			#ifdef DEBUG_CAPTURE_NAN
				feenableexcept(FE_DIVBYZERO | FE_INVALID);
			#endif

		}

		if (plane2_weight_set8)
		{
			if (wmin2[i] >= wmax2[i] * 0.999)
			{
				// if all weights in the partition were equal, then just take average
				// of all colors in the partition and use that as both endpoint colors.
				float4 avg = float4((red_vec[i].x + red_vec[i].y) / red_weight_sum[i],
									(green_vec[i].x + green_vec[i].y) / green_weight_sum[i],
									(blue_vec[i].x + blue_vec[i].y) / blue_weight_sum[i],
									(alpha_vec[i].x + alpha_vec[i].y) / alpha_weight_sum[i]);

				if (plane2_color_component == 0 && avg.x == avg.x)
					ep->endpt0[i].x = ep->endpt1[i].x = avg.x;
				if (plane2_color_component == 1 && avg.y == avg.y)
					ep->endpt0[i].y = ep->endpt1[i].y = avg.y;
				if (plane2_color_component == 2 && avg.z == avg.z)
					ep->endpt0[i].z = ep->endpt1[i].z = avg.z;
				if (plane2_color_component == 3 && avg.w == avg.w)
					ep->endpt0[i].w = ep->endpt1[i].w = avg.w;
			}
			else
			{

				#ifdef DEBUG_CAPTURE_NAN
					fedisableexcept(FE_DIVBYZERO | FE_INVALID);
				#endif

				// otherwise, complete the analytic calculation of ideal-endpoint-values
				// for the given set of texel weights and pixel colors.
				float red_det2 = determinant(pmat2_red[i]);
				float green_det2 = determinant(pmat2_green[i]);
				float blue_det2 = determinant(pmat2_blue[i]);
				float alpha_det2 = determinant(pmat2_alpha[i]);

				float red_mss2 = mat_square_sum(pmat2_red[i]);
				float green_mss2 = mat_square_sum(pmat2_green[i]);
				float blue_mss2 = mat_square_sum(pmat2_blue[i]);
				float alpha_mss2 = mat_square_sum(pmat2_alpha[i]);

				#ifdef DEBUG_PRINT_DIAGNOSTICS
					if (print_diagnostics)
						printf("Plane-2 partition %d determinants: R=%g G=%g B=%g A=%g\n", i, red_det2, green_det2, blue_det2, alpha_det2);
				#endif

				pmat2_red[i] = invert(pmat2_red[i]);
				pmat2_green[i] = invert(pmat2_green[i]);
				pmat2_blue[i] = invert(pmat2_blue[i]);
				pmat2_alpha[i] = invert(pmat2_alpha[i]);
				float4 ep0 = float4(dot(pmat2_red[i].v[0], red_vec[i]),
									dot(pmat2_green[i].v[0], green_vec[i]),
									dot(pmat2_blue[i].v[0], blue_vec[i]),
									dot(pmat2_alpha[i].v[0], alpha_vec[i]));
				float4 ep1 = float4(dot(pmat2_red[i].v[1], red_vec[i]),
									dot(pmat2_green[i].v[1], green_vec[i]),
									dot(pmat2_blue[i].v[1], blue_vec[i]),
									dot(pmat2_alpha[i].v[1], alpha_vec[i]));

				if (plane2_color_component == 0 && fabs(red_det2) > (red_mss2 * 1e-4f) && ep0.x == ep0.x && ep1.x == ep1.x)
				{
					ep->endpt0[i].x = ep0.x;
					ep->endpt1[i].x = ep1.x;
				}
				if (plane2_color_component == 1 && fabs(green_det2) > (green_mss2 * 1e-4f) && ep0.y == ep0.y && ep1.y == ep1.y)
				{
					ep->endpt0[i].y = ep0.y;
					ep->endpt1[i].y = ep1.y;
				}
				if (plane2_color_component == 2 && fabs(blue_det2) > (blue_mss2 * 1e-4f) && ep0.z == ep0.z && ep1.z == ep1.z)
				{
					ep->endpt0[i].z = ep0.z;
					ep->endpt1[i].z = ep1.z;
				}
				if (plane2_color_component == 3 && fabs(alpha_det2) > (alpha_mss2 * 1e-4f) && ep0.w == ep0.w && ep1.w == ep1.w)
				{
					ep->endpt0[i].w = ep0.w;
					ep->endpt1[i].w = ep1.w;
				}

				#ifdef DEBUG_CAPTURE_NAN
					feenableexcept(FE_DIVBYZERO | FE_INVALID);
				#endif

			}
		}
	}

	// if the calculation of an RGB-offset vector failed, try to compute
	// a somewhat-sensible value anyway
	for (i = 0; i < partition_count; i++)
		if (rgbo_fail[i])
		{
			float4 v0 = ep->endpt0[i];
			float4 v1 = ep->endpt1[i];
			float avgdif = dot(v1.xyz - v0.xyz, float3(1, 1, 1)) * (1.0f / 3.0f);
			if (avgdif <= 0.0f)
				avgdif = 0.0f;
			float4 avg = (v0 + v1) * 0.5f;
			float4 ep0 = avg - float4(avgdif, avgdif, avgdif, avgdif) * 0.5f;

			rgbo_vectors[i] = float4(ep0.xyz, avgdif);
		}


	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("Post-adjustment endpoint-colors: \n");
			for (i = 0; i < partition_count; i++)
			{
				printf("%d Low  <%g %g %g %g>\n", i, ep->endpt0[i].x, ep->endpt0[i].y, ep->endpt0[i].z, ep->endpt0[i].w);
				printf("%d High <%g %g %g %g>\n", i, ep->endpt1[i].x, ep->endpt1[i].y, ep->endpt1[i].z, ep->endpt1[i].w);

				printf("%d RGBS: <%g %g %g %g>\n", i, rgbs_vectors[i].x, rgbs_vectors[i].y, rgbs_vectors[i].z, rgbs_vectors[i].w);

				printf("%d RGBO <%g %g %g %g>\n", i, rgbo_vectors[i].x, rgbo_vectors[i].y, rgbo_vectors[i].z, rgbo_vectors[i].w);

				printf("%d Lum: <%g %g>\n", i, lum_vectors[i].x, lum_vectors[i].y);
			}
		}
	#endif

}
