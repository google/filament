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
 *	@brief	Implements functions for finding dominant direction of a set of
 *			colors, using eigenvector method
 */
/*----------------------------------------------------------------------------*/

#include "astc_codec_internals.h"

#include <math.h>
#include "mathlib.h"

#ifdef DEBUG_CAPTURE_NAN
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif

	#include <fenv.h>
#endif


/* routines to compute average colors and eigenvectors for blocks with 3 and 4 components. */


/* 
	Methods to compute eigenvectors iteratively. They are used when analytic calculation of eigenvector fails.

	These methods work as follows:

	* square the matrix
	* rescale the matrix

	The matrix is rescaled once every two squarings, mainly to protect against possible overflow/underflow situations.
	We perform 20 squarings of the matrix, with is equivalent to raising the matrix to the 1048576th power, which should be adequate.
	The eigenvector functions are carefully guarded so that they don't produce nullvectors or vectors with NaN in them.
*/

static float4 power_method_eigenvector4(const mat4 & q)
{
	mat4 p = q;
	int i;
	float4 eigvc;
	for (i = 0; i < 20; i++)
	{
		float sc0 =
			((fabs(p.v[0].x)
			  + fabs(p.v[0].y)
			  + fabs(p.v[0].z)
			  + fabs(p.v[0].w))
			 + (fabs(p.v[1].x)
				+ fabs(p.v[1].y)
				+ fabs(p.v[1].z) + fabs(p.v[1].w))) + ((fabs(p.v[2].x) + fabs(p.v[2].y) + fabs(p.v[2].z) + fabs(p.v[2].w)) + (fabs(p.v[3].x) + fabs(p.v[3].y) + fabs(p.v[3].z) + fabs(p.v[3].w)));
		sc0 *= 0.25f;

		// if the matrix is all zeroes or contains NaNs, then get out early.
		if (!(sc0 > 0.0f))
			return float4(1, 1, 1, 1);

		sc0 = 1.0f / sc0;
		p.v[0] = p.v[0] * sc0;
		p.v[1] = p.v[1] * sc0;
		p.v[2] = p.v[2] * sc0;
		p.v[3] = p.v[3] * sc0;

		p = p * p;

		// after each pair of iterations, check if we have a good enough eigenvector.
		eigvc = p.v[0] + p.v[1] + p.v[2] + p.v[3];

		float4 xform_eigvc = transform(q, eigvc);
		float evd = dot(eigvc, eigvc);
		float xvd = dot(xform_eigvc, xform_eigvc);
		float evx = dot(eigvc, xform_eigvc);
		if (evx * evx > evd * xvd * 0.999f)	// allow about 1.25 degree deviation
			return eigvc;
	}

	float testval = fabs(eigvc.x) + fabs(eigvc.y) + fabs(eigvc.z) + fabs(eigvc.w);
	if (testval > 1e-18f)
		return eigvc;			// catch eigenvectors with 0 or NaN in them.
	else
		return float4(1.0f, 1.0f, 1.0f, 1.0f);
}


static float3 power_method_eigenvector3(const mat3 & q)
{
	int i;
	mat3 p = q;
	float3 eigvc;

	for (i = 0; i < 20; i++)
	{
		float sc0 = (fabs(p.v[0].x) + fabs(p.v[0].y) + fabs(p.v[0].z)) + (fabs(p.v[1].x) + fabs(p.v[1].y) + fabs(p.v[1].z)) + (fabs(p.v[2].x) + fabs(p.v[2].y) + fabs(p.v[2].z));
		sc0 *= 0.25f;

		// if the matrix is all zeroes or contains NaNs, then get out early.
		if (!(sc0 > 0.0f))
			return float3(1, 1, 1);

		sc0 = 1.0f / sc0;
		p.v[0] = p.v[0] * sc0;
		p.v[1] = p.v[1] * sc0;
		p.v[2] = p.v[2] * sc0;

		p = p * p;

		// after each pair of iterations, check if we have a good enough eigenvector.
		eigvc = p.v[0] + p.v[1] + p.v[2];

		float3 xform_eigvc = transform(q, eigvc);
		float evd = dot(eigvc, eigvc);
		float xvd = dot(xform_eigvc, xform_eigvc);
		float evx = dot(eigvc, xform_eigvc);
		if (evx * evx > evd * xvd * 0.999f)	// allow about 1.25 degree deviation
			return eigvc;

	}


	float testval = fabs(eigvc.x) + fabs(eigvc.y) + fabs(eigvc.z);
	if (testval > 1e-18f)
		return eigvc;			// catch eigenvectors with 0 or NaN in them.
	else
		return float3(1.0f, 1.0f, 1.0f);
}


static float2 power_method_eigenvector2(const mat2 & q)
{
	int i;
	mat2 p = q;
	float2 eigvc;
	for (i = 0; i < 20; i++)
	{
		float sc0 = (fabs(p.v[0].x) + fabs(p.v[0].y)) + (fabs(p.v[1].x) + fabs(p.v[1].y));
		sc0 *= 0.25f;
		if (!(sc0 > 0.0f))
			return float2(1, 1);

		sc0 = 1.0f / sc0;
		p.v[0] = p.v[0] * sc0;
		p.v[1] = p.v[1] * sc0;

		p = p * p;

		eigvc = p.v[0] + p.v[1];
		float2 xform_eigvc = transform(q, eigvc);
		float evd = dot(eigvc, eigvc);
		float xvd = dot(xform_eigvc, xform_eigvc);
		float evx = dot(eigvc, xform_eigvc);
		if (evx * evx > evd * xvd * 0.999f)	// allow about 1.25 degree deviation
			return eigvc;
	}

	float testval = fabs(eigvc.x) + fabs(eigvc.y);
	if (testval > 1e-18f)
		return eigvc;			// catch eigenvectors with 0 or NaN in them.
	else
		return float2(1.0f, 1.0f);
}


static float4 get_eigenvector4(const mat4 & p)
{

#ifdef DEBUG_CAPTURE_NAN
	fedisableexcept(FE_DIVBYZERO | FE_INVALID);
#endif

	float4 eigvls = eigenvalues(p);
	float maxval = 0.0f;
	// do comparison for every eigenvalue; needed to catch possible NaN-cases.
	if (fabs(eigvls.x) > fabs(maxval))
		maxval = eigvls.x;
	if (fabs(eigvls.y) > fabs(maxval))
		maxval = eigvls.y;
	if (fabs(eigvls.z) > fabs(maxval))
		maxval = eigvls.z;
	if (fabs(eigvls.w) > fabs(maxval))
		maxval = eigvls.w;

	float4 retval;

	if (maxval > 0.0f)
	{
		float4 eigvc = eigenvector(p, maxval);
		// check whether the computed eigenvector is in fact a reasonable eigenvector at all.
		float4 xform_eigvc = transform(p, eigvc);
		float evd = dot(eigvc, eigvc);
		float xvd = dot(xform_eigvc, xform_eigvc);
		float evx = dot(eigvc, xform_eigvc);
		if (evx * evx > evd * xvd * 0.999f)	// allow about 1 degree deviation
			retval = eigvc;
		else
			retval = power_method_eigenvector4(p);
	}
	else
		retval = power_method_eigenvector4(p);

	#ifdef DEBUG_CAPTURE_NAN
		feenableexcept(FE_DIVBYZERO | FE_INVALID);
	#endif

	return retval;
}


static float3 get_eigenvector3(const mat3 & p)
{

#ifdef DEBUG_CAPTURE_NAN
	fedisableexcept(FE_DIVBYZERO | FE_INVALID);
#endif

	float3 eigvls = eigenvalues(p);
	float maxval = 0.0f;
	// do comparison for every eigenvalue; needed to catch possible NaN-cases.
	if (fabs(eigvls.x) > fabs(maxval))
		maxval = eigvls.x;
	if (fabs(eigvls.y) > fabs(maxval))
		maxval = eigvls.y;
	if (fabs(eigvls.z) > fabs(maxval))
		maxval = eigvls.z;

	float3 retval;

	if (maxval > 0.0f)
	{
		float3 eigvc = eigenvector(p, maxval);
		// check whether the computed eigenvector is in fact a reasonable eigenvector at all.
		float3 xform_eigvc = transform(p, eigvc);
		float evd = dot(eigvc, eigvc);
		float xvd = dot(xform_eigvc, xform_eigvc);
		float evx = dot(eigvc, xform_eigvc);
		if (evx * evx > evd * xvd * 0.999f)	// allow about 1 degree deviation
			retval = eigvc;
		else
			retval = power_method_eigenvector3(p);
	}
	else
		retval = power_method_eigenvector3(p);

	#ifdef DEBUG_CAPTURE_NAN
		feenableexcept(FE_DIVBYZERO | FE_INVALID);
	#endif

	return retval;
}


static float2 get_eigenvector2(const mat2 & p)
{

	#ifdef DEBUG_CAPTURE_NAN
		fedisableexcept(FE_DIVBYZERO | FE_INVALID);
	#endif

	float2 eigvls = eigenvalues(p);
	float maxval = 0.0f;
	// do comparison for every eigenvalue; needed to catch possible NaN-cases.
	if (fabs(eigvls.x) > fabs(maxval))
		maxval = eigvls.x;
	if (fabs(eigvls.y) > fabs(maxval))
		maxval = eigvls.y;

	float2 retval;

	if (maxval > 0.0f)
	{
		float2 eigvc = eigenvector(p, maxval);
		// check whether the computed eigenvector is in fact a reasonable eigenvector at all.
		float2 xform_eigvc = transform(p, eigvc);
		float evd = dot(eigvc, eigvc);
		float xvd = dot(xform_eigvc, xform_eigvc);
		float evx = dot(eigvc, xform_eigvc);
		if (evx * evx > evd * xvd * 0.999f)	// allow about 1 degree deviation
			retval = eigvc;
		else
			retval = power_method_eigenvector2(p);
	}
	else
		retval = power_method_eigenvector2(p);

	#ifdef DEBUG_CAPTURE_NAN
		feenableexcept(FE_DIVBYZERO | FE_INVALID);
	#endif

	return retval;
}



/* 
	For a full block, functions to compute averages and eigenvectors. The averages and eigenvectors are computed separately for each partition.
	We have separate versions for blocks with and without alpha, since the processing for blocks with alpha is significantly more expensive.
	The eigenvectors it produces are NOT normalized.
*/

// compute averages and covariance matrices for 4 components
static void compute_averages_and_covariance_matrices4(const partition_info * pt,
													  const imageblock * blk, const error_weight_block * ewb, const float4 * color_scalefactors, float4 averages[4], mat4 cov_matrices[4])
{
	int i;
	int partition_count = pt->partition_count;

	int partition;

	for (partition = 0; partition < partition_count; partition++)
	{
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
		float partition_weight = 0.0f;
		const uint8_t *indexes = pt->texels_of_partition[partition];
		int texelcount = pt->texels_per_partition[partition];
		for (i = 0; i < texelcount; i++)
		{
			int idx = indexes[i];
			float weight = ewb->texel_weight[idx];
			float r = blk->work_data[4 * idx];
			float g = blk->work_data[4 * idx + 1];
			float b = blk->work_data[4 * idx + 2];
			float a = blk->work_data[4 * idx + 3];
			partition_weight += weight;
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


		float rpt = 1.0f / MAX(partition_weight, 1e-7f);
		float rs = r_sum;
		float gs = g_sum;
		float bs = b_sum;
		float as = a_sum;

		float4 csf = color_scalefactors[partition];

		averages[partition] = float4(rs, gs, bs, as) * csf * rpt;

		float rsc = csf.x;
		float gsc = csf.y;
		float bsc = csf.z;
		float asc = csf.w;

		rs *= rsc;
		gs *= gsc;
		bs *= bsc;
		as *= asc;

		float rrs = rr_sum * rsc * rsc;
		float rgs = rg_sum * rsc * gsc;
		float rbs = rb_sum * rsc * bsc;
		float ras = ra_sum * rsc * asc;
		float ggs = gg_sum * gsc * gsc;
		float gbs = gb_sum * gsc * bsc;
		float gas = ga_sum * gsc * asc;
		float bbs = bb_sum * bsc * bsc;
		float bas = ba_sum * bsc * asc;
		float aas = aa_sum * asc * asc;

		cov_matrices[partition].v[0] = float4(rrs - rs * rs * rpt, rgs - rs * gs * rpt, rbs - rs * bs * rpt, ras - rs * as * rpt);
		cov_matrices[partition].v[1] = float4(rgs - rs * gs * rpt, ggs - gs * gs * rpt, gbs - gs * bs * rpt, gas - gs * as * rpt);
		cov_matrices[partition].v[2] = float4(rbs - rs * bs * rpt, gbs - gs * bs * rpt, bbs - bs * bs * rpt, bas - bs * as * rpt);
		cov_matrices[partition].v[3] = float4(ras - rs * as * rpt, gas - gs * as * rpt, bas - bs * as * rpt, aas - as * as * rpt);
	}

}



// compute averages and covariance matrices for 3 selected components
static void compute_averages_and_covariance_matrices3(const partition_info * pt,
													  const imageblock * blk,
													  int component1,
													  int component2, int component3, const error_weight_block * ewb, const float3 * color_scalefactors, float3 averages[4], mat3 cov_matrices[4])
{
	int i;
	int partition_count = pt->partition_count;

	const float *texel_weights;
	if (component1 == 1 && component2 == 2 && component3 == 3)
		texel_weights = ewb->texel_weight_gba;
	else if (component1 == 0 && component2 == 2 && component3 == 3)
		texel_weights = ewb->texel_weight_rba;
	else if (component1 == 0 && component2 == 1 && component3 == 3)
		texel_weights = ewb->texel_weight_rga;
	else if (component1 == 0 && component2 == 1 && component3 == 2)
		texel_weights = ewb->texel_weight_rgb;
	else
	{
		texel_weights = ewb->texel_weight_gba;
		ASTC_CODEC_INTERNAL_ERROR;
	}


	int partition;
	for (partition = 0; partition < partition_count; partition++)
	{
		float r_sum = 0.0f;
		float g_sum = 0.0f;
		float b_sum = 0.0f;
		float rr_sum = 0.0f;
		float gg_sum = 0.0f;
		float bb_sum = 0.0f;
		float rg_sum = 0.0f;
		float rb_sum = 0.0f;
		float gb_sum = 0.0f;
		float partition_weight = 0.0f;
		const uint8_t *indexes = pt->texels_of_partition[partition];
		int texelcount = pt->texels_per_partition[partition];
		for (i = 0; i < texelcount; i++)
		{
			int idx = indexes[i];
			float weight = texel_weights[idx];
			float r = blk->work_data[4 * idx + component1];
			float g = blk->work_data[4 * idx + component2];
			float b = blk->work_data[4 * idx + component3];
			partition_weight += weight;
			r_sum += r * weight;
			rr_sum += r * (r * weight);
			rg_sum += g * (r * weight);
			rb_sum += b * (r * weight);
			g_sum += g * weight;
			gg_sum += g * (g * weight);
			gb_sum += b * (g * weight);
			b_sum += b * weight;
			bb_sum += b * (b * weight);
		}

		float rpt = 1.0f / MAX(partition_weight, 1e-7f);
		float rs = r_sum;
		float gs = g_sum;
		float bs = b_sum;

		float3 csf = color_scalefactors[partition];

		averages[partition] = float3(rs, gs, bs) * csf * rpt;

		float rsc = csf.x;
		float gsc = csf.y;
		float bsc = csf.z;

		rs *= rsc;
		gs *= gsc;
		bs *= bsc;

		float rrs = rr_sum * rsc * rsc;
		float rgs = rg_sum * rsc * gsc;
		float rbs = rb_sum * rsc * bsc;
		float ggs = gg_sum * gsc * gsc;
		float gbs = gb_sum * gsc * bsc;
		float bbs = bb_sum * bsc * bsc;

		cov_matrices[partition].v[0] = float3(rrs - rs * rs * rpt, rgs - rs * gs * rpt, rbs - rs * bs * rpt);
		cov_matrices[partition].v[1] = float3(rgs - rs * gs * rpt, ggs - gs * gs * rpt, gbs - gs * bs * rpt);
		cov_matrices[partition].v[2] = float3(rbs - rs * bs * rpt, gbs - gs * bs * rpt, bbs - bs * bs * rpt);
	}
}



// compute averages and covariance matrices for 2 selected components
static void compute_averages_and_covariance_matrices2(const partition_info * pt,
													  const imageblock * blk,
													  int component1, int component2, const error_weight_block * ewb, const float2 * scalefactors, float2 averages[4], mat2 cov_matrices[4])
{
	int i;
	int partition_count = pt->partition_count;

	const float *texel_weights;
	if (component1 == 0 && component2 == 1)
		texel_weights = ewb->texel_weight_rg;
	else if (component1 == 0 && component2 == 2)
		texel_weights = ewb->texel_weight_rb;
	else if (component1 == 1 && component2 == 2)
		texel_weights = ewb->texel_weight_gb;
	else
	{
		texel_weights = ewb->texel_weight_rg;
		// unsupported set of color components.
		ASTC_CODEC_INTERNAL_ERROR;
		exit(1);
	}

	int partition;
	for (partition = 0; partition < partition_count; partition++)
	{
		float c1_sum = 0.0f;
		float c2_sum = 0.0f;
		float c1c1_sum = 0.0f;
		float c2c2_sum = 0.0f;
		float c1c2_sum = 0.0f;
		float partition_weight = 0.0f;
		const uint8_t *indexes = pt->texels_of_partition[partition];
		int texelcount = pt->texels_per_partition[partition];
		for (i = 0; i < texelcount; i++)
		{
			int idx = indexes[i];
			float weight = texel_weights[idx];
			float c1 = blk->work_data[4 * idx + component1];
			float c2 = blk->work_data[4 * idx + component2];
			partition_weight += weight;
			c1_sum += c1 * weight;
			c1c1_sum += c1 * (c1 * weight);
			c1c2_sum += c2 * (c1 * weight);
			c2_sum += c2 * weight;
			c2c2_sum += c2 * (c2 * weight);
		}

		float rpt = 1.0f / MAX(partition_weight, 1e-7f);
		float c1s = c1_sum;
		float c2s = c2_sum;

		float2 csf = scalefactors[partition];

		averages[partition] = float2(c1s, c2s) * csf * rpt;

		float c1sc = csf.x;
		float c2sc = csf.y;

		c1s *= c1sc;
		c2s *= c2sc;

		float c1c1s = c1c1_sum * c1sc * c1sc;
		float c1c2s = c1c2_sum * c1sc * c2sc;
		float c2c2s = c2c2_sum * c2sc * c2sc;

		cov_matrices[partition].v[0] = float2(c1c1s - c1s * c1s * rpt, c1c2s - c1s * c2s * rpt);
		cov_matrices[partition].v[1] = float2(c1c2s - c1s * c2s * rpt, c2c2s - c2s * c2s * rpt);
	}
}






void compute_averages_and_directions_rgba(const partition_info * pt,
										  const imageblock * blk,
										  const error_weight_block * ewb,
										  const float4 * color_scalefactors,
										  float4 * averages, float4 * eigenvectors_rgba, float3 * eigenvectors_gba, float3 * eigenvectors_rba, float3 * eigenvectors_rga, float3 * eigenvectors_rgb)
{
	int i;
	int partition_count = pt->partition_count;

	mat4 covariance_matrices[4];

	compute_averages_and_covariance_matrices4(pt, blk, ewb, color_scalefactors, averages, covariance_matrices);

	for (i = 0; i < partition_count; i++)
	{
		eigenvectors_rgba[i] = get_eigenvector4(covariance_matrices[i]);
		mat3 rcmat;				// reduced covariance matrix
		rcmat.v[0] = covariance_matrices[i].v[1].yzw;
		rcmat.v[1] = covariance_matrices[i].v[2].yzw;
		rcmat.v[2] = covariance_matrices[i].v[3].yzw;
		eigenvectors_gba[i] = get_eigenvector3(rcmat);
		rcmat.v[0] = covariance_matrices[i].v[0].xzw;
		rcmat.v[1] = covariance_matrices[i].v[2].xzw;
		rcmat.v[2] = covariance_matrices[i].v[3].xzw;
		eigenvectors_rba[i] = get_eigenvector3(rcmat);
		rcmat.v[0] = covariance_matrices[i].v[0].xyw;
		rcmat.v[1] = covariance_matrices[i].v[1].xyw;
		rcmat.v[2] = covariance_matrices[i].v[3].xyw;
		eigenvectors_rga[i] = get_eigenvector3(rcmat);
		rcmat.v[0] = covariance_matrices[i].v[0].xyz;
		rcmat.v[1] = covariance_matrices[i].v[1].xyz;
		rcmat.v[2] = covariance_matrices[i].v[2].xyz;
		eigenvectors_rgb[i] = get_eigenvector3(rcmat);
	}
}





void compute_averages_and_directions_rgb(const partition_info * pt,
										 const imageblock * blk,
										 const error_weight_block * ewb,
										 const float4 * color_scalefactors, float3 * averages, float3 * eigenvectors_rgb, float2 * eigenvectors_rg, float2 * eigenvectors_rb, float2 * eigenvectors_gb)
{
	int i;
	int partition_count = pt->partition_count;


	mat3 covariance_matrices[4];
	mat2 rg_matrices[4];
	mat2 rb_matrices[4];
	mat2 gb_matrices[4];

	float3 scalefactors[4];
	for (i = 0; i < partition_count; i++)
	{
		float4 scf = color_scalefactors[i];
		scalefactors[i] = scf.xyz;
	}

	compute_averages_and_covariance_matrices3(pt, blk, 0, 1, 2, ewb, scalefactors, averages, covariance_matrices);

	for (i = 0; i < partition_count; i++)
	{
		rg_matrices[i].v[0] = covariance_matrices[i].v[0].xy;
		rg_matrices[i].v[1] = covariance_matrices[i].v[1].xy;
		rb_matrices[i].v[0] = covariance_matrices[i].v[0].xz;
		rb_matrices[i].v[1] = covariance_matrices[i].v[2].xz;
		gb_matrices[i].v[0] = covariance_matrices[i].v[1].yz;
		gb_matrices[i].v[1] = covariance_matrices[i].v[2].yz;

		eigenvectors_rgb[i] = get_eigenvector3(covariance_matrices[i]);
		eigenvectors_rg[i] = get_eigenvector2(rg_matrices[i]);
		eigenvectors_rb[i] = get_eigenvector2(rb_matrices[i]);
		eigenvectors_gb[i] = get_eigenvector2(gb_matrices[i]);
	}
}


void compute_averages_and_directions_3_components(const partition_info * pt,
												  const imageblock * blk,
												  const error_weight_block * ewb,
												  const float3 * color_scalefactors, int component1, int component2, int component3, float3 * averages, float3 * eigenvectors)
{
	int i;
	int partition_count = pt->partition_count;

	mat3 covariance_matrices[4];

	compute_averages_and_covariance_matrices3(pt, blk, component1, component2, component3, ewb, color_scalefactors, averages, covariance_matrices);

	for (i = 0; i < partition_count; i++)
		eigenvectors[i] = get_eigenvector3(covariance_matrices[i]);
}


void compute_averages_and_directions_2_components(const partition_info * pt,
												  const imageblock * blk,
												  const error_weight_block * ewb, const float2 * color_scalefactors, int component1, int component2, float2 * averages, float2 * eigenvectors)
{
	int i;
	int partition_count = pt->partition_count;

	mat2 covariance_matrices[4];

	compute_averages_and_covariance_matrices2(pt, blk, component1, component2, ewb, color_scalefactors, averages, covariance_matrices);

	for (i = 0; i < partition_count; i++)
		eigenvectors[i] = get_eigenvector2(covariance_matrices[i]);
}




#define XPASTE(x,y) x##y
#define PASTE(x,y) XPASTE(x,y)


#define TWO_COMPONENT_ERROR_FUNC( funcname, c0_idx, c1_idx, c01_name, c01_rname ) \
float funcname( \
	const partition_info *pt, \
	const imageblock *blk, \
	const error_weight_block *ewb, \
	const processed_line2 *plines, \
	float *length_of_lines \
	) \
	{ \
	int i; \
	float errorsum = 0.0f; \
	int partition; \
	for(partition=0; partition<pt->partition_count; partition++) \
		{ \
		const uint8_t *indexes = pt->texels_of_partition[ partition ]; \
		int texelcount = pt->texels_per_partition[ partition ]; \
		float lowparam = 1e10f; \
		float highparam = -1e10f; \
		processed_line2 l = plines[partition]; \
		if( ewb->contains_zeroweight_texels ) \
			{ \
			for(i=0;i<texelcount;i++) \
				{ \
				int idx = indexes[i]; \
				float texel_weight = ewb-> PASTE(texel_weight_ , c01_rname) [i]; \
				if( texel_weight > 1e-20f ) \
					{ \
					float2 point = float2(blk->work_data[4*idx + c0_idx], blk->work_data[4*idx + c1_idx] ); \
					float param = dot( point, l.bs ); \
					float2 rp1 = l.amod + param*l.bis; \
					float2 dist = rp1 - point; \
					float4 ews = ewb->error_weights[idx]; \
					errorsum += dot( ews. c01_name, dist*dist ); \
					if( param < lowparam ) lowparam = param; \
					if( param > highparam ) highparam = param; \
					} \
				} \
			} \
		else \
			{ \
			for(i=0;i<texelcount;i++) \
				{ \
				int idx = indexes[i]; \
				float2 point = float2(blk->work_data[4*idx + c0_idx], blk->work_data[4*idx + c1_idx] ); \
				float param = dot( point, l.bs ); \
				float2 rp1 = l.amod + param*l.bis; \
				float2 dist = rp1 - point; \
				float4 ews = ewb->error_weights[idx]; \
				errorsum += dot( ews. c01_name, dist*dist ); \
				if( param < lowparam ) lowparam = param; \
				if( param > highparam ) highparam = param; \
				} \
			} \
		float linelen = highparam - lowparam; \
		if( !(linelen > 1e-7f) ) \
			linelen = 1e-7f; \
		length_of_lines[partition] = linelen; \
		} \
	return errorsum; \
	}

TWO_COMPONENT_ERROR_FUNC(compute_error_squared_rg, 0, 1, xy, rg)
TWO_COMPONENT_ERROR_FUNC(compute_error_squared_rb, 0, 2, xz, rb)
TWO_COMPONENT_ERROR_FUNC(compute_error_squared_gb, 1, 2, yz, gb)
TWO_COMPONENT_ERROR_FUNC(compute_error_squared_ra, 0, 3, zw, ra)


// function to compute the error across a tile when using a particular set of
// lines for a particular partitioning. Also compute the length of each
// color-space line in each partitioning.

#define THREE_COMPONENT_ERROR_FUNC( funcname, c0_idx, c1_idx, c2_idx, c012_name, c012_rname ) \
float funcname( \
	const partition_info *pt, \
	const imageblock *blk, \
	const error_weight_block *ewb, \
	const processed_line3 *plines, \
	float *length_of_lines \
	) \
	{ \
	int i; \
	float errorsum = 0.0f; \
	int partition; \
	for(partition=0; partition<pt->partition_count; partition++) \
		{ \
		const uint8_t *indexes = pt->texels_of_partition[ partition ]; \
		int texelcount = pt->texels_per_partition[ partition ]; \
		float lowparam = 1e10f; \
		float highparam = -1e10f; \
		processed_line3 l = plines[partition]; \
		if( ewb->contains_zeroweight_texels ) \
			{ \
			for(i=0;i<texelcount;i++) \
				{ \
				int idx = indexes[i]; \
				float texel_weight = ewb-> PASTE(texel_weight_ , c012_rname) [i]; \
				if( texel_weight > 1e-20f ) \
					{ \
					float3 point = float3(blk->work_data[4*idx + c0_idx], blk->work_data[4*idx + c1_idx], blk->work_data[4*idx + c2_idx] ); \
					float param = dot( point, l.bs ); \
					float3 rp1 = l.amod + param*l.bis; \
					float3 dist = rp1 - point; \
					float4 ews = ewb->error_weights[idx]; \
					errorsum += dot( ews. c012_name, dist*dist ); \
					if( param < lowparam ) lowparam = param; \
					if( param > highparam ) highparam = param; \
					} \
				} \
			} \
		else \
			{ \
			for(i=0;i<texelcount;i++) \
				{ \
				int idx = indexes[i]; \
				float3 point = float3(blk->work_data[4*idx + c0_idx], blk->work_data[4*idx + c1_idx], blk->work_data[4*idx + c2_idx] ); \
				float param = dot( point, l.bs ); \
				float3 rp1 = l.amod + param*l.bis; \
				float3 dist = rp1 - point; \
				float4 ews = ewb->error_weights[idx]; \
				errorsum += dot( ews. c012_name, dist*dist ); \
				if( param < lowparam ) lowparam = param; \
				if( param > highparam ) highparam = param; \
				} \
			} \
		float linelen = highparam - lowparam; \
		if( !(linelen > 1e-7f) ) \
			linelen = 1e-7f; \
		length_of_lines[partition] = linelen; \
		} \
	return errorsum; \
	}

THREE_COMPONENT_ERROR_FUNC(compute_error_squared_gba, 1, 2, 3, yzw, gba)
THREE_COMPONENT_ERROR_FUNC(compute_error_squared_rba, 0, 2, 3, xzw, rba)
THREE_COMPONENT_ERROR_FUNC(compute_error_squared_rga, 0, 1, 3, xyw, rga)
THREE_COMPONENT_ERROR_FUNC(compute_error_squared_rgb, 0, 1, 2, xyz, rgb)

float compute_error_squared_rgba(const partition_info * pt,	// the partition that we use when computing the squared-error.
								 const imageblock * blk, const error_weight_block * ewb, const processed_line4 * plines, float *length_of_lines)
{
	int i;

	float errorsum = 0.0f;
	int partition;
	for (partition = 0; partition < pt->partition_count; partition++)
	{
		const uint8_t *indexes = pt->texels_of_partition[partition];
		int texelcount = pt->texels_per_partition[partition];
		float lowparam = 1e10;
		float highparam = -1e10;

		processed_line4 l = plines[partition];

		if (ewb->contains_zeroweight_texels)
		{
			for (i = 0; i < texelcount; i++)
			{
				int idx = indexes[i];
				if (ewb->texel_weight[idx] > 1e-20)
				{
					float4 point = float4(blk->work_data[4 * idx], blk->work_data[4 * idx + 1], blk->work_data[4 * idx + 2], blk->work_data[4 * idx + 3]);
					float param = dot(point, l.bs);
					float4 rp1 = l.amod + param * l.bis;
					float4 dist = rp1 - point;
					float4 ews = ewb->error_weights[idx];
					errorsum += dot(ews, dist * dist);
					if (param < lowparam)
						lowparam = param;
					if (param > highparam)
						highparam = param;
				}
			}
		}
		else
		{
			for (i = 0; i < texelcount; i++)
			{
				int idx = indexes[i];
				float4 point = float4(blk->work_data[4 * idx], blk->work_data[4 * idx + 1], blk->work_data[4 * idx + 2], blk->work_data[4 * idx + 3]);
				float param = dot(point, l.bs);
				float4 rp1 = l.amod + param * l.bis;
				float4 dist = rp1 - point;
				float4 ews = ewb->error_weights[idx];
				errorsum += dot(ews, dist * dist);
				if (param < lowparam)
					lowparam = param;
				if (param > highparam)
					highparam = param;
			}
		}

		float linelen = highparam - lowparam;
		if (!(linelen > 1e-7f))
			linelen = 1e-7f;
		length_of_lines[partition] = linelen;
	}

	return errorsum;
}



// function to compute the error across a tile when using a particular line for
// a particular partition.
float compute_error_squared_rgb_single_partition(int partition_to_test, int xdim, int ydim, int zdim, const partition_info * pt,	// the partition that we use when computing the squared-error.
												 const imageblock * blk, const error_weight_block * ewb, const processed_line3 * lin	// the line for the partition.
	)
{
	int i;

	int texels_per_block = xdim * ydim * zdim;

	float errorsum = 0.0f;

	for (i = 0; i < texels_per_block; i++)
	{
		int partition = pt->partition_of_texel[i];
		float texel_weight = ewb->texel_weight_rgb[i];
		if (partition != partition_to_test || texel_weight < 1e-20)
			continue;
		float3 point = float3(blk->work_data[4 * i], blk->work_data[4 * i + 1], blk->work_data[4 * i + 2]);

		float param = dot(point, lin->bs);
		float3 rp1 = lin->amod + param * lin->bis;
		float3 dist = rp1 - point;
		float4 ews = ewb->error_weights[i];

		errorsum += dot(ews.xyz, dist * dist);
	}
	return errorsum;
}
