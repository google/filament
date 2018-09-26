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
 *	@brief	Determine color errors for ASTC compression.
 *
 *			We assume that there are two independent sources of color error in
 *			any given partition.
 *
 *			These are:
 *			* quantization errors
 *			* encoding choice errors
 *
 *			Encoding choice errors are errors that come due to encoding choice,
 *			such as:
 *			* using luminance instead of RGB
 *			* using RGB-scale instead of two RGB endpoints.
 *			* dropping Alpha
 *
 *			Quantization errors occur due to the limited precision we use for
 *			storing numbers.
 *
 *			Quantization errors generally scale with quantization level, but are
 *			not actually independent of color encoding. In particular:
 *			* if we can use offset encoding then quantization error is halved.
 *			* if we can use blue-contraction, quantization error for red and
 *			  green is halved.
 *			* quantization error is higher for the HDR endpoint modes.
 *
 *			Other than these errors, quantization error is assumed to be
 *			proportional to the quantization step.
 */
/*----------------------------------------------------------------------------*/

#include "astc_codec_internals.h"

#include <math.h>

#ifdef DEBUG_PRINT_DIAGNOSTICS
	#include <stdio.h>
#endif

// helper function to merge two endpoint-colors
void merge_endpoints(const endpoints * ep1,	// contains three of the color components
					 const endpoints * ep2,	// contains the remaining color component
					 int separate_component, endpoints * res)
{
	int i;
	int partition_count = ep1->partition_count;
	res->partition_count = partition_count;
	for (i = 0; i < partition_count; i++)
	{
		res->endpt0[i] = ep1->endpt0[i];
		res->endpt1[i] = ep1->endpt1[i];
	}

	switch (separate_component)
	{
	case 0:
		for (i = 0; i < partition_count; i++)
		{
			res->endpt0[i].x = ep2->endpt0[i].x;
			res->endpt1[i].x = ep2->endpt1[i].x;
		}
		break;
	case 1:
		for (i = 0; i < partition_count; i++)
		{
			res->endpt0[i].y = ep2->endpt0[i].y;
			res->endpt1[i].y = ep2->endpt1[i].y;
		}
		break;
	case 2:
		for (i = 0; i < partition_count; i++)
		{
			res->endpt0[i].z = ep2->endpt0[i].z;
			res->endpt1[i].z = ep2->endpt1[i].z;
		}
		break;
	case 3:
		for (i = 0; i < partition_count; i++)
		{
			res->endpt0[i].w = ep2->endpt0[i].w;
			res->endpt1[i].w = ep2->endpt1[i].w;
		}
		break;
	}
}



/*
   for a given set of input colors and a given partitioning, determine: color error that results
   from RGB-scale encoding (relevant for LDR only) color error that results from RGB-lumashift encoding
   (relevant for HDR only) color error that results from luminance-encoding color error that results
   form dropping alpha. whether we are eligible for offset encoding whether we are eligible for
   blue-contraction

   The input data are: color data partitioning error-weight data
 */


void compute_encoding_choice_errors(int xdim, int ydim, int zdim, const imageblock * pb, const partition_info * pi, const error_weight_block * ewb,
									int separate_component,	// component that is separated out in 2-plane mode, -1 in 1-plane mode
									encoding_choice_errors * eci)
{
	int i;

	int partition_count = pi->partition_count;

	int texels_per_block = xdim * ydim * zdim;

	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("%s : texels-per-block=%dx%dx%d, separate_component=%d, partition-count=%d\n", __func__, xdim, ydim, zdim, separate_component, partition_count);
		}
	#endif

	float3 averages[4];
	float3 directions_rgb[4];
	float2 directions_rg[4];
	float2 directions_rb[4];
	float2 directions_gb[4];

	float4 error_weightings[4];
	float4 color_scalefactors[4];
	float4 inverse_color_scalefactors[4];

	compute_partition_error_color_weightings(xdim, ydim, zdim, ewb, pi, error_weightings, color_scalefactors);

	compute_averages_and_directions_rgb(pi, pb, ewb, color_scalefactors, averages, directions_rgb, directions_rg, directions_rb, directions_gb);

	line3 uncorr_rgb_lines[4];
	line3 samechroma_rgb_lines[4];	// for LDR-RGB-scale
	line3 rgb_luma_lines[4];	// for HDR-RGB-scale
	line3 luminance_lines[4];

	processed_line3 proc_uncorr_rgb_lines[4];
	processed_line3 proc_samechroma_rgb_lines[4];	// for LDR-RGB-scale
	processed_line3 proc_rgb_luma_lines[4];	// for HDR-RGB-scale
	processed_line3 proc_luminance_lines[4];


	for (i = 0; i < partition_count; i++)
	{
		inverse_color_scalefactors[i].x = 1.0f / MAX(color_scalefactors[i].x, 1e-7f);
		inverse_color_scalefactors[i].y = 1.0f / MAX(color_scalefactors[i].y, 1e-7f);
		inverse_color_scalefactors[i].z = 1.0f / MAX(color_scalefactors[i].z, 1e-7f);
		inverse_color_scalefactors[i].w = 1.0f / MAX(color_scalefactors[i].w, 1e-7f);


		uncorr_rgb_lines[i].a = averages[i];
		if (dot(directions_rgb[i], directions_rgb[i]) == 0.0f)
			uncorr_rgb_lines[i].b = normalize(float3(color_scalefactors[i].xyz));
		else
			uncorr_rgb_lines[i].b = normalize(directions_rgb[i]);

		samechroma_rgb_lines[i].a = float3(0, 0, 0);
		if (dot(averages[i], averages[i]) < 1e-20)
			samechroma_rgb_lines[i].b = normalize(float3(color_scalefactors[i].xyz));
		else
			samechroma_rgb_lines[i].b = normalize(averages[i]);

		rgb_luma_lines[i].a = averages[i];
		rgb_luma_lines[i].b = normalize(color_scalefactors[i].xyz);

		luminance_lines[i].a = float3(0, 0, 0);
		luminance_lines[i].b = normalize(color_scalefactors[i].xyz);

		#ifdef DEBUG_PRINT_DIAGNOSTICS
			if (print_diagnostics)
			{
				printf("Partition %d\n", i);
				printf("Average = <%g %g %g>\n", averages[i].x, averages[i].y, averages[i].z);
				printf("Uncorr-rgb-line = <%g %g %g> + t<%g %g %g>\n",
					uncorr_rgb_lines[i].a.x, uncorr_rgb_lines[i].a.y, uncorr_rgb_lines[i].a.z, uncorr_rgb_lines[i].b.x, uncorr_rgb_lines[i].b.y, uncorr_rgb_lines[i].b.z);
				printf("Samechroma-line = t<%g %g %g>\n", samechroma_rgb_lines[i].b.x, samechroma_rgb_lines[i].b.y, samechroma_rgb_lines[i].b.z);
			}
		#endif

		proc_uncorr_rgb_lines[i].amod = (uncorr_rgb_lines[i].a - uncorr_rgb_lines[i].b * dot(uncorr_rgb_lines[i].a, uncorr_rgb_lines[i].b)) * inverse_color_scalefactors[i].xyz;
		proc_uncorr_rgb_lines[i].bs = uncorr_rgb_lines[i].b * color_scalefactors[i].xyz;
		proc_uncorr_rgb_lines[i].bis = uncorr_rgb_lines[i].b * inverse_color_scalefactors[i].xyz;

		proc_samechroma_rgb_lines[i].amod = (samechroma_rgb_lines[i].a - samechroma_rgb_lines[i].b * dot(samechroma_rgb_lines[i].a, samechroma_rgb_lines[i].b)) * inverse_color_scalefactors[i].xyz;
		proc_samechroma_rgb_lines[i].bs = samechroma_rgb_lines[i].b * color_scalefactors[i].xyz;
		proc_samechroma_rgb_lines[i].bis = samechroma_rgb_lines[i].b * inverse_color_scalefactors[i].xyz;

		proc_rgb_luma_lines[i].amod = (rgb_luma_lines[i].a - rgb_luma_lines[i].b * dot(rgb_luma_lines[i].a, rgb_luma_lines[i].b)) * inverse_color_scalefactors[i].xyz;
		proc_rgb_luma_lines[i].bs = rgb_luma_lines[i].b * color_scalefactors[i].xyz;
		proc_rgb_luma_lines[i].bis = rgb_luma_lines[i].b * inverse_color_scalefactors[i].xyz;

		proc_luminance_lines[i].amod = (luminance_lines[i].a - luminance_lines[i].b * dot(luminance_lines[i].a, luminance_lines[i].b)) * inverse_color_scalefactors[i].xyz;
		proc_luminance_lines[i].bs = luminance_lines[i].b * color_scalefactors[i].xyz;
		proc_luminance_lines[i].bis = luminance_lines[i].b * inverse_color_scalefactors[i].xyz;

	}



	float uncorr_rgb_error[4];
	float samechroma_rgb_error[4];
	float rgb_luma_error[4];
	float luminance_rgb_error[4];


	for (i = 0; i < partition_count; i++)
	{

		uncorr_rgb_error[i] = compute_error_squared_rgb_single_partition(i, xdim, ydim, zdim, pi, pb, ewb, &(proc_uncorr_rgb_lines[i]));

		samechroma_rgb_error[i] = compute_error_squared_rgb_single_partition(i, xdim, ydim, zdim, pi, pb, ewb, &(proc_samechroma_rgb_lines[i]));

		rgb_luma_error[i] = compute_error_squared_rgb_single_partition(i, xdim, ydim, zdim, pi, pb, ewb, &(proc_rgb_luma_lines[i]));

		luminance_rgb_error[i] = compute_error_squared_rgb_single_partition(i, xdim, ydim, zdim, pi, pb, ewb, &(proc_luminance_lines[i]));

		#ifdef DEBUG_PRINT_DIAGNOSTICS
			if (print_diagnostics)
			{
				printf("Partition %d : uncorr-error=%g  samechroma-error=%g  rgb-luma-error=%g  lum-error=%g\n",
					i, uncorr_rgb_error[i], samechroma_rgb_error[i], rgb_luma_error[i], luminance_rgb_error[i]);
			}
		#endif
	}

	// compute the error that arises from just ditching alpha and RGB
	float alpha_drop_error[4];
	float rgb_drop_error[4];
	for (i = 0; i < partition_count; i++)
	{
		alpha_drop_error[i] = 0;
		rgb_drop_error[i] = 0;
	}
	for (i = 0; i < texels_per_block; i++)
	{
		int partition = pi->partition_of_texel[i];
		float alpha = pb->work_data[4 * i + 3];
		float default_alpha = pb->alpha_lns[i] ? (float)0x7800 : (float)0xFFFF;

		float omalpha = alpha - default_alpha;
		alpha_drop_error[partition] += omalpha * omalpha * ewb->error_weights[i].w;
		float red = pb->work_data[4 * i];
		float green = pb->work_data[4 * i + 1];
		float blue = pb->work_data[4 * i + 2];
		rgb_drop_error[partition] += red * red * ewb->error_weights[i].x + green * green * ewb->error_weights[i].y + blue * blue * ewb->error_weights[i].z;
	}

	// check if we are eligible for blue-contraction and offset-encoding

	endpoints ep;
	if (separate_component == -1)
	{
		endpoints_and_weights ei;
		compute_endpoints_and_ideal_weights_1_plane(xdim, ydim, zdim, pi, pb, ewb, &ei);
		ep = ei.ep;
	}
	else
	{
		endpoints_and_weights ei1, ei2;
		compute_endpoints_and_ideal_weights_2_planes(xdim, ydim, zdim, pi, pb, ewb, separate_component, &ei1, &ei2);

		merge_endpoints(&(ei1.ep), &(ei2.ep), separate_component, &ep);
	}

	int eligible_for_offset_encode[4];
	int eligible_for_blue_contraction[4];

	for (i = 0; i < partition_count; i++)
	{
		float4 endpt0 = ep.endpt0[i];
		float4 endpt1 = ep.endpt1[i];
		float4 endpt_dif = endpt1 - endpt0;
		if (fabs(endpt_dif.x) < (0.12 * 65535.0f) && fabs(endpt_dif.y) < (0.12 * 65535.0f) && fabs(endpt_dif.z) < (0.12 * 65535.0f))
			eligible_for_offset_encode[i] = 1;
		else
			eligible_for_offset_encode[i] = 0;
		endpt0.x += (endpt0.x - endpt0.z);
		endpt0.y += (endpt0.y - endpt0.z);
		endpt1.x += (endpt1.x - endpt1.z);
		endpt1.y += (endpt1.y - endpt1.z);
		if (endpt0.x > (0.01f * 65535.0f) && endpt0.x < (0.99f * 65535.0f)
			&& endpt1.x > (0.01f * 65535.0f) && endpt1.x < (0.99f * 65535.0f)
			&& endpt0.y > (0.01f * 65535.0f) && endpt0.y < (0.99f * 65535.0f) && endpt1.y > (0.01f * 65535.0f) && endpt1.y < (0.99f * 65535.0f))
			eligible_for_blue_contraction[i] = 1;
		else
			eligible_for_blue_contraction[i] = 0;
	}


	// finally, gather up our results
	for (i = 0; i < partition_count; i++)
	{
		eci[i].rgb_scale_error = (samechroma_rgb_error[i] - uncorr_rgb_error[i]) * 0.7f;	// empirical
		eci[i].rgb_luma_error = (rgb_luma_error[i] - uncorr_rgb_error[i]) * 1.5f;	// wild guess
		eci[i].luminance_error = (luminance_rgb_error[i] - uncorr_rgb_error[i]) * 3.0f;	// empirical
		eci[i].alpha_drop_error = alpha_drop_error[i] * 3.0f;
		eci[i].rgb_drop_error = rgb_drop_error[i] * 3.0f;
		eci[i].can_offset_encode = eligible_for_offset_encode[i];
		eci[i].can_blue_contract = eligible_for_blue_contraction[i];
	}
}
