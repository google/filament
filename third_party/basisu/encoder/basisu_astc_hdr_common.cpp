// File: basisu_astc_hdr_common.cpp
// Copyright (C) 2019-2026 Binomial LLC. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "basisu_enc.h"
#include "basisu_gpu_texture.h"
#include "../transcoder/basisu_astc_helpers.h"
#include "../transcoder/basisu_astc_hdr_core.h"
#include "basisu_astc_hdr_common.h"

using namespace basist;

#ifndef __EMSCRIPTEN__
	#define BASISU_MULTITHREADED_INIT (0)
#endif

namespace basisu
{

// Beware: the first entry is the # of weight levels for that BISE range.
const uint8_t g_ise_weight_lerps[MAX_SUPPORTED_ISE_WEIGHT_INDEX + 1][33] =
{
	{ 2, 0, 64 }, // 0, note ise range=0 is invalid for 4x4 block sizes (<24 weight bits in the block)
	{ 3, 0, 32, 64 }, // 1
	{ 4, 0, 21, 43, 64 }, // 2
	{ 5, 0, 16, 32, 48, 64 }, // 3
	{ 6, 0, 64, 12, 52, 25, 39 }, // 4
	{ 8, 0, 9, 18, 27, 37, 46, 55, 64 }, // 5
	{ 10, 0, 64, 7, 57, 14, 50, 21, 43, 28, 36 }, // 6
	{ 12, 0, 64, 17, 47, 5, 59, 23, 41, 11, 53, 28, 36 }, // 7
	{ 16, 0, 4, 8, 12, 17, 21, 25, 29, 35, 39, 43, 47, 52, 56, 60, 64 }, // 8
	{ 20, 0,64,16,48,3,61,19,45,6,58,23,41,9,55,26,38,13,51,29,35}, // 9
	{ 24, 0,64,8,56,16,48,24,40,2,62,11,53,19,45,27,37,5,59,13,51,22,42,30,34}, // 10
	{ 32, 0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64}, // 11
};

//--------------------------------------------------------------------------------------------------------------------------

const float DEF_R_ERROR_SCALE = 2.0f;
const float DEF_G_ERROR_SCALE = 3.0f;

void astc_hdr_codec_base_options::init()
{
	m_r_err_scale = DEF_R_ERROR_SCALE;
	m_g_err_scale = DEF_G_ERROR_SCALE;
	m_q_log_bias = Q_LOG_BIAS_4x4;

	m_ultra_quant = false;

	// Disabling by default to avoid transcoding outliers (try kodim26). The quality lost is very low. TODO: Could include the uber result in the output.
	m_allow_uber_mode = false;

	m_mode7_full_s_optimization = true;

	m_take_first_non_clamping_mode11_submode = false;
	m_take_first_non_clamping_mode7_submode = false;

	m_disable_weight_plane_optimization = true;
}

//--------------------------------------------------------------------------------------------------------------------------
// max usable qlog8 value is 247, 248=inf, >=249 is nan
// max usable qlog7 value is 123, 124=inf, >=125 is nan

//const uint32_t TOTAL_USABLE_QLOG8 = 248; // 0-247 are usable, 0=0, 247=60416.0, 246=55296.0

// nearest values given a positive half float value (only)
static uint16_t g_half_to_qlog7[32768], g_half_to_qlog8[32768];

const uint32_t HALF_TO_QLOG_TABS_MIN_BITS = 7;
const uint32_t HALF_TO_QLOG_TABS_MAX_BITS = 8;
static uint16_t* g_pHalf_to_qlog_tabs[2] =
{
	g_half_to_qlog7,
	g_half_to_qlog8,
};

#if 0
static inline uint32_t half_to_qlog7_8(half_float h, uint32_t bits)
{
	assert((bits >= HALF_TO_QLOG_TABS_MIN_BITS) && (bits <= HALF_TO_QLOG_TABS_MAX_BITS));
	assert(h < 32768);

	return g_pHalf_to_qlog_tabs[bits - HALF_TO_QLOG_TABS_MIN_BITS][h];
}
#endif

// TODO: Tune this
static inline uint32_t quant_qlog16(uint32_t q16, uint32_t desired_bits)
{
	assert((desired_bits >= 7) && (desired_bits <= 12));
	assert(q16 <= 65535);

	const uint32_t shift = 16 - desired_bits;
	uint32_t e = (q16 + (1U << (shift - 1U)) - 1U) >> shift;

	uint32_t max_val = (1U << desired_bits) - 1U;
	e = minimum<uint32_t>(e, max_val);

	return e;
}

static void compute_half_to_qlog_table(uint32_t bits, uint16_t* pTable, const basisu::vector<float>& qlog16_to_float)
{
	assert(bits >= 5 && bits <= 12);
	const uint32_t max_val = (1 << bits) - 1;

	const uint32_t FIRST_INVALID_QLOG16_INDEX = 63488; // first inf, rest are inf/nan's
	assert(std::isinf(qlog16_to_float[FIRST_INVALID_QLOG16_INDEX]));
	assert(std::isinf(qlog16_to_float[FIRST_INVALID_QLOG16_INDEX + 1]));
	assert(!std::isnan(qlog16_to_float[FIRST_INVALID_QLOG16_INDEX - 1]));
	assert(!std::isinf(qlog16_to_float[FIRST_INVALID_QLOG16_INDEX - 1]));

	// For all positive half-floats
	for (uint32_t h = 0; h < 32768; h++)
	{
		// Skip invalid values
		if (is_half_inf_or_nan((half_float)h))
			continue;
		const float desired_val = half_to_float((half_float)h);

		float best_err = BIG_FLOAT_VAL;
		uint32_t best_qlog = 0;
		
		double prev_err = BIG_FLOAT_VAL;

		// For all possible qlog's
		for (uint32_t i = 0; i <= max_val; i++)
		{
			// Skip invalid values
			uint32_t idx = i << (16 - bits);
			if (idx >= FIRST_INVALID_QLOG16_INDEX)
				break;

			float v = qlog16_to_float[idx];
			//assert(!std::isinf(v) && !std::isnan(v)); // too clostly in debug

			// Compute error
			float err = fabsf(v - desired_val);

			if (err > prev_err)
			{
				// Every remaining entry will have guaranteed higher error
				break;
			}

			prev_err = err;
						
			// Find best
			if (err < best_err)
			{
				best_err = err;
				best_qlog = i;
				
				if (best_err == 0.0f)
					break;
			}
		}

		pTable[h] = (uint16_t)best_qlog;
	}
}

static void init_qlog_tables()
{
	basisu::vector<float> qlog16_to_float(65536);

	// for all possible qlog16, compute the corresponding half float
	for (uint32_t i = 0; i <= 65535; i++)
	{
		half_float h = astc_helpers::qlog16_to_half(i);

		qlog16_to_float[i] = half_to_float(h);
	}

#if BASISU_MULTITHREADED_INIT
	job_pool jp(3);
	
	for (uint32_t bits = HALF_TO_QLOG_TABS_MIN_BITS; bits <= HALF_TO_QLOG_TABS_MAX_BITS; bits++)
	{
		jp.add_job( [bits, &qlog16_to_float]() { compute_half_to_qlog_table(bits, g_pHalf_to_qlog_tabs[bits - HALF_TO_QLOG_TABS_MIN_BITS], qlog16_to_float); });
	}

	jp.wait_for_all();
#else
	// for all possible half floats, find the nearest qlog5-12 float
	for (uint32_t bits = HALF_TO_QLOG_TABS_MIN_BITS; bits <= HALF_TO_QLOG_TABS_MAX_BITS; bits++)
	{
		compute_half_to_qlog_table(bits, g_pHalf_to_qlog_tabs[bits - HALF_TO_QLOG_TABS_MIN_BITS], qlog16_to_float);

#if 0
		std::vector<uint16_t> check_tab(32768);
		compute_half_to_qlog_table_orig(bits, check_tab.data(), qlog16_to_float);
		for (uint32_t i = 0; i < (1 << bits); i++)
		{
			assert(check_tab[i] == g_pHalf_to_qlog_tabs[bits - HALF_TO_QLOG_TABS_MIN_BITS][i]);
		}
#endif
	}
#endif // BASISU_MULTITHREADED_INIT
}

//--------------------------------------------------------------------------------------------------------------------------

static vec3F calc_mean(uint32_t num_pixels, const vec4F* pPixels)
{
	vec3F mean(0.0f);

	for (uint32_t i = 0; i < num_pixels; i++)
	{
		const vec4F& p = pPixels[i];

		mean[0] += p[0];
		mean[1] += p[1];
		mean[2] += p[2];
	}

	return mean / static_cast<float>(num_pixels);
}

static vec3F calc_rgb_pca(uint32_t num_pixels, const vec4F* pPixels, const vec3F& mean_color)
{
	float cov[6] = { 0, 0, 0, 0, 0, 0 };

	for (uint32_t i = 0; i < num_pixels; i++)
	{
		const vec4F& v = pPixels[i];

		float r = v[0] - mean_color[0];
		float g = v[1] - mean_color[1];
		float b = v[2] - mean_color[2];

		cov[0] += r * r;
		cov[1] += r * g;
		cov[2] += r * b;
		cov[3] += g * g;
		cov[4] += g * b;
		cov[5] += b * b;
	}

	float xr = .9f, xg = 1.0f, xb = .7f;
	for (uint32_t iter = 0; iter < 3; iter++)
	{
		float r = xr * cov[0] + xg * cov[1] + xb * cov[2];
		float g = xr * cov[1] + xg * cov[3] + xb * cov[4];
		float b = xr * cov[2] + xg * cov[4] + xb * cov[5];

		float m = maximumf(maximumf(fabsf(r), fabsf(g)), fabsf(b));

		if (m > 1e-10f)
		{
			m = 1.0f / m;

			r *= m;
			g *= m;
			b *= m;
		}

		xr = r;
		xg = g;
		xb = b;
	}

	float len = xr * xr + xg * xg + xb * xb;

	vec3F axis(0.5773502691f);

	if (len >= 1e-10f)
	{
		len = 1.0f / sqrtf(len);

		xr *= len;
		xg *= len;
		xb *= len;

		axis.set(xr, xg, xb);
	}

	return axis;
}

void encode_astc_block_stats::init(uint32_t num_pixels, const vec4F pBlock_pixels_q16[])
{
	m_num_pixels = num_pixels;
	m_mean_q16 = calc_mean(num_pixels, pBlock_pixels_q16);
	m_axis_q16 = calc_rgb_pca(num_pixels, pBlock_pixels_q16, m_mean_q16);
}

static vec3F interp_color(const vec3F& mean, const vec3F& dir, float df, const aabb3F& colorspace_box, const aabb3F& input_box, bool* pInside = nullptr)
{
#if 0
	assert(mean[0] >= input_box[0][0]);
	assert(mean[1] >= input_box[0][1]);
	assert(mean[2] >= input_box[0][2]);
	assert(mean[0] <= input_box[1][0]);
	assert(mean[1] <= input_box[1][1]);
	assert(mean[2] <= input_box[1][2]);
#endif

	if (pInside)
		*pInside = false;

	vec3F k(mean + dir * df);
	if (colorspace_box.contains(k))
	{
		if (pInside)
			*pInside = true;

		return k;
	}

	// starts inside
	vec3F s(mean);

	// ends outside
	vec3F e(mean + dir * df);

	// a ray guaranteed to go from the outside to inside
	ray3F r(e, (s - e).normalize_in_place());
	vec3F c;
	float t = 0.0f;

	intersection::result res = intersection::ray_aabb(c, t, r, input_box);
	if (res != intersection::cSuccess)
		c = k;

	return c;
}

// all in Q16 space, 0-65535
static bool compute_least_squares_endpoints_rgb(
	uint32_t N, const uint8_t* pSelectors, const vec4F* pSelector_weights,
	vec3F* pXl, vec3F* pXh, const vec4F* pColors, const aabb3F& input_box)
{
	// Least squares using normal equations: http://www.cs.cornell.edu/~bindel/class/cs3220-s12/notes/lec10.pdf 
	// https://web.archive.org/web/20150319232457/http://www.cs.cornell.edu/~bindel/class/cs3220-s12/notes/lec10.pdf
	// I did this in matrix form first, expanded out all the ops, then optimized it a bit.
	float z00 = 0.0f, z01 = 0.0f, z10 = 0.0f, z11 = 0.0f;
	float q00_r = 0.0f, q10_r = 0.0f, t_r = 0.0f;
	float q00_g = 0.0f, q10_g = 0.0f, t_g = 0.0f;
	float q00_b = 0.0f, q10_b = 0.0f, t_b = 0.0f;

	for (uint32_t i = 0; i < N; i++)
	{
		const uint32_t sel = pSelectors[i];
		
		z00 += pSelector_weights[sel][0];
		z10 += pSelector_weights[sel][1];
		z11 += pSelector_weights[sel][2];

		float w = pSelector_weights[sel][3];

		q00_r += w * pColors[i][0];
		t_r += pColors[i][0];

		q00_g += w * pColors[i][1];
		t_g += pColors[i][1];

		q00_b += w * pColors[i][2];
		t_b += pColors[i][2];
	}

	q10_r = t_r - q00_r;
	q10_g = t_g - q00_g;
	q10_b = t_b - q00_b;

	z01 = z10;

	float det = z00 * z11 - z01 * z10;
	if (det == 0.0f)
		return false;

	det = 1.0f / det;

	float iz00, iz01, iz10, iz11;
	iz00 = z11 * det;
	iz01 = -z01 * det;
	iz10 = -z10 * det;
	iz11 = z00 * det;
		
	(*pXl)[0] = (float)(iz00 * q00_r + iz01 * q10_r);
	(*pXh)[0] = (float)(iz10 * q00_r + iz11 * q10_r);

	(*pXl)[1] = (float)(iz00 * q00_g + iz01 * q10_g);
	(*pXh)[1] = (float)(iz10 * q00_g + iz11 * q10_g);

	(*pXl)[2] = (float)(iz00 * q00_b + iz01 * q10_b);
	(*pXh)[2] = (float)(iz10 * q00_b + iz11 * q10_b);

	for (uint32_t c = 0; c < 3; c++)
	{
		float l = (*pXl)[c], h = (*pXh)[c];

		if (input_box.get_dim(c) < .0000125f)
		{
			l = input_box[0][c];
			h = input_box[1][c];
		}
		
		(*pXl)[c] = l;
		(*pXh)[c] = h;
	}

	vec3F mean((*pXl + *pXh) * .5f);
	vec3F dir(*pXh - *pXl);

	float ln = dir.length();
	if (ln)
	{
		dir /= ln;

		float ld = (*pXl - mean).dot(dir);
		float hd = (*pXh - mean).dot(dir);

		aabb3F colorspace_box(vec3F(0.0f), vec3F(MAX_QLOG16_VAL));

		bool was_inside1 = false;

		vec3F l = interp_color(mean, dir, ld, colorspace_box, input_box, &was_inside1);
		if (!was_inside1)
			*pXl = l;

		bool was_inside2 = false;
		vec3F h = interp_color(mean, dir, hd, colorspace_box, input_box, &was_inside2);
		if (!was_inside2)
			*pXh = h;
	}

	pXl->clamp(0.0f, MAX_QLOG16_VAL);
	pXh->clamp(0.0f, MAX_QLOG16_VAL);

	return true;
}

static bool compute_least_squares_endpoints_rgb_raw_weights(
	uint32_t N, const uint8_t* pRaw_weights, 
	vec3F* pXl, vec3F* pXh, const vec4F* pColors, const aabb3F& input_box)
{
	// Least squares using normal equations: http://www.cs.cornell.edu/~bindel/class/cs3220-s12/notes/lec10.pdf 
	// https://web.archive.org/web/20150319232457/http://www.cs.cornell.edu/~bindel/class/cs3220-s12/notes/lec10.pdf
	// I did this in matrix form first, expanded out all the ops, then optimized it a bit.
	float z00 = 0.0f, z01 = 0.0f, z10 = 0.0f, z11 = 0.0f;
	float q00_r = 0.0f, q10_r = 0.0f, t_r = 0.0f;
	float q00_g = 0.0f, q10_g = 0.0f, t_g = 0.0f;
	float q00_b = 0.0f, q10_b = 0.0f, t_b = 0.0f;
		
	for (uint32_t i = 0; i < N; i++)
	{
		const float wt = (float)pRaw_weights[i] * (1.0f / 64.0f);
		assert(wt <= 1.0f);

		const float w0 = wt * wt;
		const float w1 = (1.0f - wt) * wt;
		const float w2 = (1.0f - wt) * (1.0f - wt);
		const float w3 = wt;

		z00 += w0;
		z10 += w1;
		z11 += w2;

		float w = w3;
		q00_r += w * pColors[i][0];
		t_r += pColors[i][0];

		q00_g += w * pColors[i][1];
		t_g += pColors[i][1];

		q00_b += w * pColors[i][2];
		t_b += pColors[i][2];
	}

	q10_r = t_r - q00_r;
	q10_g = t_g - q00_g;
	q10_b = t_b - q00_b;

	z01 = z10;

	float det = z00 * z11 - z01 * z10;
	if (det == 0.0f)
		return false;

	det = 1.0f / det;

	float iz00, iz01, iz10, iz11;
	iz00 = z11 * det;
	iz01 = -z01 * det;
	iz10 = -z10 * det;
	iz11 = z00 * det;

	(*pXl)[0] = (float)(iz00 * q00_r + iz01 * q10_r);
	(*pXh)[0] = (float)(iz10 * q00_r + iz11 * q10_r);

	(*pXl)[1] = (float)(iz00 * q00_g + iz01 * q10_g);
	(*pXh)[1] = (float)(iz10 * q00_g + iz11 * q10_g);

	(*pXl)[2] = (float)(iz00 * q00_b + iz01 * q10_b);
	(*pXh)[2] = (float)(iz10 * q00_b + iz11 * q10_b);

	for (uint32_t c = 0; c < 3; c++)
	{
		float l = (*pXl)[c], h = (*pXh)[c];

		if (input_box.get_dim(c) < .0000125f)
		{
			l = input_box[0][c];
			h = input_box[1][c];
		}

		(*pXl)[c] = l;
		(*pXh)[c] = h;
	}

	vec3F mean((*pXl + *pXh) * .5f);
	vec3F dir(*pXh - *pXl);

	float ln = dir.length();
	if (ln)
	{
		dir /= ln;

		float ld = (*pXl - mean).dot(dir);
		float hd = (*pXh - mean).dot(dir);

		aabb3F colorspace_box(vec3F(0.0f), vec3F(MAX_QLOG16_VAL));

		bool was_inside1 = false;

		vec3F l = interp_color(mean, dir, ld, colorspace_box, input_box, &was_inside1);
		if (!was_inside1)
			*pXl = l;

		bool was_inside2 = false;
		vec3F h = interp_color(mean, dir, hd, colorspace_box, input_box, &was_inside2);
		if (!was_inside2)
			*pXh = h;
	}

	pXl->clamp(0.0f, MAX_QLOG16_VAL);
	pXh->clamp(0.0f, MAX_QLOG16_VAL);

	return true;
}

static bool compute_least_squares_endpoints_2D(
	uint32_t N, const uint8_t* pSelectors, const vec4F* pSelector_weights,
	vec2F* pXl, vec2F* pXh, const vec2F* pColors, const aabb2F& input_box)
{
	// Least squares using normal equations: http://www.cs.cornell.edu/~bindel/class/cs3220-s12/notes/lec10.pdf 
	// https://web.archive.org/web/20150319232457/http://www.cs.cornell.edu/~bindel/class/cs3220-s12/notes/lec10.pdf
	// I did this in matrix form first, expanded out all the ops, then optimized it a bit.
	float z00 = 0.0f, z01 = 0.0f, z10 = 0.0f, z11 = 0.0f;
	float q00_r = 0.0f, q10_r = 0.0f, t_r = 0.0f;
	float q00_g = 0.0f, q10_g = 0.0f, t_g = 0.0f;
	
	for (uint32_t i = 0; i < N; i++)
	{
		const uint32_t sel = pSelectors[i];
		z00 += pSelector_weights[sel][0];
		z10 += pSelector_weights[sel][1];
		z11 += pSelector_weights[sel][2];

		float w = pSelector_weights[sel][3];
		q00_r += w * pColors[i][0];
		t_r += pColors[i][0];

		q00_g += w * pColors[i][1];
		t_g += pColors[i][1];
	}

	q10_r = t_r - q00_r;
	q10_g = t_g - q00_g;

	z01 = z10;

	float det = z00 * z11 - z01 * z10;
	if (det == 0.0f)
		return false;

	det = 1.0f / det;

	float iz00, iz01, iz10, iz11;
	iz00 = z11 * det;
	iz01 = -z01 * det;
	iz10 = -z10 * det;
	iz11 = z00 * det;

	(*pXl)[0] = (float)(iz00 * q00_r + iz01 * q10_r);
	(*pXh)[0] = (float)(iz10 * q00_r + iz11 * q10_r);

	(*pXl)[1] = (float)(iz00 * q00_g + iz01 * q10_g);
	(*pXh)[1] = (float)(iz10 * q00_g + iz11 * q10_g);

	for (uint32_t c = 0; c < 2; c++)
	{
		float l = (*pXl)[c], h = (*pXh)[c];

		if (input_box.get_dim(c) < .0000125f)
		{
			l = input_box[0][c];
			h = input_box[1][c];
		}

		(*pXl)[c] = l;
		(*pXh)[c] = h;
	}
		
	pXl->clamp(0.0f, MAX_QLOG16_VAL);
	pXh->clamp(0.0f, MAX_QLOG16_VAL);

	return true;
}

static bool compute_least_squares_endpoints_1D(
	uint32_t N, const uint8_t* pSelectors, const vec4F* pSelector_weights,
	vec1F* pXl, vec1F* pXh, const vec1F* pColors, const aabb1F& input_box)
{
	// Least squares using normal equations: http://www.cs.cornell.edu/~bindel/class/cs3220-s12/notes/lec10.pdf 
	// https://web.archive.org/web/20150319232457/http://www.cs.cornell.edu/~bindel/class/cs3220-s12/notes/lec10.pdf
	// I did this in matrix form first, expanded out all the ops, then optimized it a bit.
	float z00 = 0.0f, z01 = 0.0f, z10 = 0.0f, z11 = 0.0f;
	float q00_r = 0.0f, q10_r = 0.0f, t_r = 0.0f;

	for (uint32_t i = 0; i < N; i++)
	{
		const uint32_t sel = pSelectors[i];
		z00 += pSelector_weights[sel][0];
		z10 += pSelector_weights[sel][1];
		z11 += pSelector_weights[sel][2];

		float w = pSelector_weights[sel][3];
		q00_r += w * pColors[i][0];
		t_r += pColors[i][0];
	}

	q10_r = t_r - q00_r;

	z01 = z10;

	float det = z00 * z11 - z01 * z10;
	if (det == 0.0f)
		return false;

	det = 1.0f / det;

	float iz00, iz01, iz10, iz11;
	iz00 = z11 * det;
	iz01 = -z01 * det;
	iz10 = -z10 * det;
	iz11 = z00 * det;

	(*pXl)[0] = (float)(iz00 * q00_r + iz01 * q10_r);
	(*pXh)[0] = (float)(iz10 * q00_r + iz11 * q10_r);

	for (uint32_t c = 0; c < 1; c++)
	{
		float l = (*pXl)[c], h = (*pXh)[c];

		if (input_box.get_dim(c) < .0000125f)
		{
			l = input_box[0][c];
			h = input_box[1][c];
		}

		(*pXl)[c] = l;
		(*pXh)[c] = h;
	}

	pXl->clamp(0.0f, MAX_QLOG16_VAL);
	pXh->clamp(0.0f, MAX_QLOG16_VAL);

	return true;
}

static bool compute_weighted_least_squares_endpoints_rgb(
	uint32_t N, 
	const uint8_t* pSelectors, const vec4F* pSelector_weights, const float* pRaw_weights, /* ti */
	const float* pEmphasis_weights /* wi */,
	vec3F* pXl, vec3F* pXh, 
	const vec4F* pColors, /* pi */
	const aabb3F& input_box)
{
	(void)input_box;

	assert(N);
	assert((pSelectors && pSelector_weights) || pRaw_weights);
	assert(pEmphasis_weights);

	// Pi = pixel colors
	// Ti = project weights, [0,1]
	// Wi = emphasis weights

	float total_wi = 0.0f;
	for (uint32_t i = 0; i < N; i++)
		total_wi += pEmphasis_weights[i];

	if (total_wi == 0.0f)
		return false;

	float weighted_mean_tw = 0.0f;
	float weighted_mean_pw[3] = { 0.0f };

	for (uint32_t i = 0; i < N; i++)
	{
		const float wi = pEmphasis_weights[i];
		const float ti = pSelectors ? pSelector_weights[pSelectors[i]][3] : pRaw_weights[i];
		const float pi_r = pColors[i][0], pi_g = pColors[i][1], pi_b = pColors[i][2];

		weighted_mean_tw += wi * ti;
		
		weighted_mean_pw[0] += wi * pi_r;
		weighted_mean_pw[1] += wi * pi_g;
		weighted_mean_pw[2] += wi * pi_b;
	}

	weighted_mean_tw /= total_wi;

	weighted_mean_pw[0] /= total_wi;
	weighted_mean_pw[1] /= total_wi;
	weighted_mean_pw[2] /= total_wi;

	float spt[3] = { 0.0f };
	float stt = 0.0f;

	for (uint32_t i = 0; i < N; i++)
	{
		const float wi = pEmphasis_weights[i];
		const float ti = pSelectors ? pSelector_weights[pSelectors[i]][3] : pRaw_weights[i];
		const float pi_r = pColors[i][0], pi_g = pColors[i][1], pi_b = pColors[i][2];
		
		spt[0] += wi * (pi_r - weighted_mean_pw[0]) * (ti - weighted_mean_tw);
		spt[1] += wi * (pi_g - weighted_mean_pw[1]) * (ti - weighted_mean_tw);
		spt[2] += wi * (pi_b - weighted_mean_pw[2]) * (ti - weighted_mean_tw);

		stt += wi * square(ti - weighted_mean_tw);
	}

	if (stt == 0.0f)
		return false;

	for (uint32_t i = 0; i < 3; i++)
	{
		float h = weighted_mean_pw[i] + (spt[i] / stt) * (1.0f - weighted_mean_tw);
		float l = weighted_mean_pw[i] - (spt[i] / stt) * weighted_mean_tw;
				
		(*pXh)[i] = h;
		(*pXl)[i] = l;
	}

	pXl->clamp(0.0f, MAX_QLOG16_VAL);
	pXh->clamp(0.0f, MAX_QLOG16_VAL);

	return true;
}

vec4F g_astc_ls_weights_ise[MAX_SUPPORTED_ISE_WEIGHT_INDEX + 1][MAX_SUPPORTED_WEIGHT_LEVELS];

uint8_t g_map_astc_to_linear_order[MAX_SUPPORTED_ISE_WEIGHT_INDEX + 1][MAX_SUPPORTED_WEIGHT_LEVELS]; // [ise_range][astc_index] -> linear index
uint8_t g_map_linear_to_astc_order[MAX_SUPPORTED_ISE_WEIGHT_INDEX + 1][MAX_SUPPORTED_WEIGHT_LEVELS]; // [ise_range][linear_index] -> astc_index

static void encode_astc_hdr_init()
{
	// Precomputed weight constants used during least fit determination. For each entry: w * w, (1.0f - w) * w, (1.0f - w) * (1.0f - w), w
	for (uint32_t range = MIN_SUPPORTED_ISE_WEIGHT_INDEX; range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX; range++)
	{
		const uint32_t num_levels = g_ise_weight_lerps[range][0];
		assert(num_levels == astc_helpers::get_ise_levels(range));
		assert((num_levels >= MIN_SUPPORTED_WEIGHT_LEVELS) && (num_levels <= MAX_SUPPORTED_WEIGHT_LEVELS));

		for (uint32_t i = 0; i < num_levels; i++)
		{
			float w = g_ise_weight_lerps[range][1 + i] * (1.0f / 64.0f);

			g_astc_ls_weights_ise[range][i].set(w * w, (1.0f - w) * w, (1.0f - w) * (1.0f - w), w);
		}
	}

	for (uint32_t ise_range = MIN_SUPPORTED_ISE_WEIGHT_INDEX; ise_range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX; ise_range++)
	{
		const uint32_t num_levels = g_ise_weight_lerps[ise_range][0];
		assert((num_levels >= MIN_SUPPORTED_WEIGHT_LEVELS) && (num_levels <= MAX_SUPPORTED_WEIGHT_LEVELS));

		uint32_t s[MAX_SUPPORTED_WEIGHT_LEVELS];
		for (uint32_t i = 0; i < num_levels; i++)
			s[i] = (g_ise_weight_lerps[ise_range][1 + i] << 8) + i;

		std::sort(s, s + num_levels);

		for (uint32_t i = 0; i < num_levels; i++)
			g_map_linear_to_astc_order[ise_range][i] = (uint8_t)(s[i] & 0xFF);

		for (uint32_t i = 0; i < num_levels; i++)
			g_map_astc_to_linear_order[ise_range][g_map_linear_to_astc_order[ise_range][i]] = (uint8_t)i;
	}

	//init_quantize_tables();
}

bool g_astc_hdr_enc_initialized;

void astc_hdr_enc_init()
{
	if (g_astc_hdr_enc_initialized)
		return;

	astc_hdr_core_init();

	astc_helpers::init_tables();

	init_qlog_tables();

	encode_astc_hdr_init();

	g_astc_hdr_enc_initialized = true;
}

void interpolate_qlog12_colors(
	const int e[2][3],
	half_float* pDecoded_half,
	vec3F* pDecoded_float,
	uint32_t n, uint32_t ise_weight_range)
{
	assert((ise_weight_range >= MIN_SUPPORTED_ISE_WEIGHT_INDEX) && (ise_weight_range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX));

	for (uint32_t i = 0; i < 2; i++)
	{
		for (uint32_t j = 0; j < 3; j++)
		{
			assert(is_in_range(e[i][j], 0, 0xFFF));
		}
	}

	for (uint32_t i = 0; i < n; i++)
	{
		const int c = g_ise_weight_lerps[ise_weight_range][1 + i];
		assert(c == (int)astc_helpers::dequant_bise_weight(i, ise_weight_range));

		half_float rf, gf, bf;

		{
			uint32_t r0 = e[0][0] << 4;
			uint32_t r1 = e[1][0] << 4;
			int ri = (r0 * (64 - c) + r1 * c + 32) / 64;
			rf = astc_helpers::qlog16_to_half(ri);
		}

		{
			uint32_t g0 = e[0][1] << 4;
			uint32_t g1 = e[1][1] << 4;
			int gi = (g0 * (64 - c) + g1 * c + 32) / 64;
			gf = astc_helpers::qlog16_to_half(gi);
		}

		{
			uint32_t b0 = e[0][2] << 4;
			uint32_t b1 = e[1][2] << 4;
			int bi = (b0 * (64 - c) + b1 * c + 32) / 64;
			bf = astc_helpers::qlog16_to_half(bi);
		}

		if (pDecoded_half)
		{
			pDecoded_half[i * 3 + 0] = rf;
			pDecoded_half[i * 3 + 1] = gf;
			pDecoded_half[i * 3 + 2] = bf;
		}

		if (pDecoded_float)
		{
			pDecoded_float[i][0] = half_to_float(rf);
			pDecoded_float[i][1] = half_to_float(gf);
			pDecoded_float[i][2] = half_to_float(bf);
		}
	}
}

// decoded in ASTC order, not linear order
// return false if the ISE endpoint quantization leads to non-valid endpoints being decoded
bool get_astc_hdr_mode_11_block_colors(
	const uint8_t* pEndpoints,
	half_float* pDecoded_half,
	vec3F* pDecoded_float,
	uint32_t n, uint32_t ise_weight_range, uint32_t ise_endpoint_range)
{
	assert((ise_weight_range >= MIN_SUPPORTED_ISE_WEIGHT_INDEX) && (ise_weight_range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX));

	int e[2][3];
	if (!decode_mode11_to_qlog12(pEndpoints, e, ise_endpoint_range))
		return false;

	interpolate_qlog12_colors(e, pDecoded_half, pDecoded_float, n, ise_weight_range);

	return true;
}

// decoded in ASTC order, not linear order
// return false if the ISE endpoint quantization leads to non-valid endpoints being decoded
bool get_astc_hdr_mode_7_block_colors(
	const uint8_t* pEndpoints,
	half_float* pDecoded_half,
	vec3F* pDecoded_float,
	uint32_t n, uint32_t ise_weight_range, uint32_t ise_endpoint_range)
{
	assert((ise_weight_range >= MIN_SUPPORTED_ISE_WEIGHT_INDEX) && (ise_weight_range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX));

	int e[2][3];
	if (!decode_mode7_to_qlog12(pEndpoints, e, nullptr, ise_endpoint_range))
		return false;

	interpolate_qlog12_colors(e, pDecoded_half, pDecoded_float, n, ise_weight_range);

	return true;
}

double eval_selectors_f(
	uint32_t num_pixels,
	uint8_t* pWeights,
	const half_float* pBlock_pixels_half,
	uint32_t num_weight_levels,
	const half_float* pDecoded_half,
	const astc_hdr_codec_base_options& coptions,
	uint32_t usable_selector_bitmask)
{
	assert((num_pixels >= 1) && (num_pixels <= MAX_ASTC_HDR_ENC_BLOCK_PIXELS));
	assert(usable_selector_bitmask);

	const float R_WEIGHT = coptions.m_r_err_scale;
	const float G_WEIGHT = coptions.m_g_err_scale;

	double total_error = 0;

#ifdef _DEBUG
	for (uint32_t i = 0; i < num_weight_levels; i++)
	{
		assert(!is_half_inf_or_nan(pDecoded_half[i * 3 + 0]));
		assert(!is_half_inf_or_nan(pDecoded_half[i * 3 + 1]));
		assert(!is_half_inf_or_nan(pDecoded_half[i * 3 + 2]));
	}
#endif

	double decoded_half_q[MAX_SUPPORTED_WEIGHT_LEVELS][3];

	for (uint32_t i = 0; i < num_weight_levels; i++)
	{
		const half_float* p = &pDecoded_half[i * 3];

		decoded_half_q[i][0] = q(p[0], coptions.m_q_log_bias);
		decoded_half_q[i][1] = q(p[1], coptions.m_q_log_bias);
		decoded_half_q[i][2] = q(p[2], coptions.m_q_log_bias);
	}

	for (uint32_t p = 0; p < num_pixels; p++)
	{
		const half_float* pDesired_half = &pBlock_pixels_half[p * 3];

		const double desired_half_r_q = q(pDesired_half[0], coptions.m_q_log_bias);
		const double desired_half_g_q = q(pDesired_half[1], coptions.m_q_log_bias);
		const double desired_half_b_q = q(pDesired_half[2], coptions.m_q_log_bias);

		double lowest_e = BIG_FLOAT_VAL;

		//double dists[MAX_SUPPORTED_WEIGHT_LEVELS];

		// this is an approximation of MSLE
		for (uint32_t i = 0; i < num_weight_levels; i++)
		{
			if (((1 << i) & usable_selector_bitmask) == 0)
				continue;

			// compute piecewise linear approximation of log2(a+eps)-log2(b+eps), for each component, then MSLE
			double rd = decoded_half_q[i][0] - desired_half_r_q;
			double gd = decoded_half_q[i][1] - desired_half_g_q;
			double bd = decoded_half_q[i][2] - desired_half_b_q;

			double e = R_WEIGHT * (rd * rd) + G_WEIGHT * (gd * gd) + bd * bd;

			//dists[i] = e;

			if (e < lowest_e)
			{
				lowest_e = e;
				pWeights[p] = (uint8_t)i;
			}
		}

		total_error += lowest_e;

	} // p

	return total_error;
}

double eval_selectors(
	uint32_t num_pixels,
	uint8_t* pWeights,
	uint32_t ise_weight_range,
	const half_float* pBlock_pixels_half,
	uint32_t num_weight_levels,
	const half_float* pDecoded_half,
	const astc_hdr_codec_base_options& coptions,
	uint32_t usable_selector_bitmask)
{
	if ((coptions.m_r_err_scale != 2.0f) || (coptions.m_g_err_scale != 3.0f))
	{
		return eval_selectors_f(
			num_pixels,
			pWeights,
			pBlock_pixels_half,
			num_weight_levels,
			pDecoded_half,
			coptions,
			usable_selector_bitmask);
	}

	assert((num_pixels >= 1) && (num_pixels <= MAX_ASTC_HDR_ENC_BLOCK_PIXELS));
	assert(usable_selector_bitmask);

	uint64_t total_error = 0;

#ifdef _DEBUG
	for (uint32_t i = 0; i < num_weight_levels; i++)
	{
		assert(!is_half_inf_or_nan(pDecoded_half[i * 3 + 0]));
		assert(!is_half_inf_or_nan(pDecoded_half[i * 3 + 1]));
		assert(!is_half_inf_or_nan(pDecoded_half[i * 3 + 2]));
	}
#endif

	int64_t decoded_half_q[MAX_SUPPORTED_WEIGHT_LEVELS][3];

	for (uint32_t i = 0; i < num_weight_levels; i++)
	{
		const half_float* p = &pDecoded_half[i * 3];

		decoded_half_q[i][0] = q2(p[0], coptions.m_q_log_bias);
		decoded_half_q[i][1] = q2(p[1], coptions.m_q_log_bias);
		decoded_half_q[i][2] = q2(p[2], coptions.m_q_log_bias);
	}

	if (usable_selector_bitmask != UINT32_MAX)
	{
		for (uint32_t p = 0; p < num_pixels; p++)
		{
			const half_float* pDesired_half = &pBlock_pixels_half[p * 3];

			const int64_t desired_half_r_q = q2(pDesired_half[0], coptions.m_q_log_bias);
			const int64_t desired_half_g_q = q2(pDesired_half[1], coptions.m_q_log_bias);
			const int64_t desired_half_b_q = q2(pDesired_half[2], coptions.m_q_log_bias);

			int64_t lowest_e = INT64_MAX;

			for (uint32_t i = 0; i < num_weight_levels; i++)
			{
				if (((1 << i) & usable_selector_bitmask) == 0)
					continue;

				int64_t rd = decoded_half_q[i][0] - desired_half_r_q;
				int64_t gd = decoded_half_q[i][1] - desired_half_g_q;
				int64_t bd = decoded_half_q[i][2] - desired_half_b_q;

				int64_t e = 2 * (rd * rd) + 3 * (gd * gd) + bd * bd;

				if (e < lowest_e)
				{
					lowest_e = e;
					pWeights[p] = (uint8_t)i;
				}
			}

			total_error += lowest_e;

		} // p
	}
	else
	{
		if ((num_weight_levels <= 4) || (coptions.m_disable_weight_plane_optimization))
		{
			for (uint32_t p = 0; p < num_pixels; p++)
			{
				const half_float* pDesired_half = &pBlock_pixels_half[p * 3];

				const half_float desired_r = pDesired_half[0], desired_g = pDesired_half[1], desired_b = pDesired_half[2];

				const int64_t desired_half_r_q = q2(desired_r, coptions.m_q_log_bias);
				const int64_t desired_half_g_q = q2(desired_g, coptions.m_q_log_bias);
				const int64_t desired_half_b_q = q2(desired_b, coptions.m_q_log_bias);

				int64_t lowest_e = INT64_MAX;

				uint32_t i;
				for (i = 0; (i + 1) < num_weight_levels; i += 2)
				{
					int64_t e0, e1;

					{
						int64_t rd0 = decoded_half_q[i][0] - desired_half_r_q; // 27 bits maximum with half float inputs
						int64_t gd0 = decoded_half_q[i][1] - desired_half_g_q;
						int64_t bd0 = decoded_half_q[i][2] - desired_half_b_q;
						e0 = ((2 * (rd0 * rd0) + 3 * (gd0 * gd0) + bd0 * bd0) << 5) | i; // max 62 bits (27*2+3+5)
					}

					{
						int64_t rd1 = decoded_half_q[i + 1][0] - desired_half_r_q;
						int64_t gd1 = decoded_half_q[i + 1][1] - desired_half_g_q;
						int64_t bd1 = decoded_half_q[i + 1][2] - desired_half_b_q;
						e1 = ((2 * (rd1 * rd1) + 3 * (gd1 * gd1) + bd1 * bd1) << 5) | (i + 1);
					}

					lowest_e = minimum(lowest_e, e0, e1);
				}

				if (i != num_weight_levels)
				{
					int64_t rd0 = decoded_half_q[i][0] - desired_half_r_q;
					int64_t gd0 = decoded_half_q[i][1] - desired_half_g_q;
					int64_t bd0 = decoded_half_q[i][2] - desired_half_b_q;
					int64_t e0 = ((2 * (rd0 * rd0) + 3 * (gd0 * gd0) + bd0 * bd0) << 5) | i;

					lowest_e = minimum(lowest_e, e0);
				}

				pWeights[p] = (uint8_t)(lowest_e & 31);

				total_error += (lowest_e >> 5);

			} // p
		}
		else
		{
			const auto& weight_val_to_ise_tab = astc_helpers::g_dequant_tables.get_weight_tab(ise_weight_range).m_val_to_ise;
			const int lo_index = weight_val_to_ise_tab[0], hi_index = weight_val_to_ise_tab[64], mid_index = weight_val_to_ise_tab[32];

			const vec3F low_color((float)pDecoded_half[lo_index * 3 + 0], (float)pDecoded_half[lo_index * 3 + 1], (float)pDecoded_half[lo_index * 3 + 2]);
			const vec3F high_color((float)pDecoded_half[hi_index * 3 + 0], (float)pDecoded_half[hi_index * 3 + 1], (float)pDecoded_half[hi_index * 3 + 2]);
			const vec3F mid_color((float)pDecoded_half[mid_index * 3 + 0], (float)pDecoded_half[mid_index * 3 + 1], (float)pDecoded_half[mid_index * 3 + 2]);
						
			const vec3F block_dir(high_color - low_color);

			for (uint32_t p = 0; p < num_pixels; p++)
			{
				const half_float* pDesired_half = &pBlock_pixels_half[p * 3];

				const half_float desired_r = pDesired_half[0], desired_g = pDesired_half[1], desired_b = pDesired_half[2];

				const int64_t desired_half_r_q = q2(desired_r, coptions.m_q_log_bias);
				const int64_t desired_half_g_q = q2(desired_g, coptions.m_q_log_bias);
				const int64_t desired_half_b_q = q2(desired_b, coptions.m_q_log_bias);
				
				// Determine which side of the middle plane the point is for a modest gain
				vec3F c((float)desired_r - mid_color[0], (float)desired_g - mid_color[1], (float)desired_b - mid_color[2]);
				float d = c.dot(block_dir);
								
				int i = 0, high_index = (num_weight_levels / 2) + 1;
				if (d >= 0.0f)
				{
					i = num_weight_levels / 2;
					high_index = num_weight_levels;
				}

				int64_t lowest_e = INT64_MAX;

				for (; (i + 1) < high_index; i += 2)
				{
					int64_t e0, e1;

					{
						int64_t rd0 = decoded_half_q[i][0] - desired_half_r_q; // 27 bits maximum with half float inputs
						int64_t gd0 = decoded_half_q[i][1] - desired_half_g_q;
						int64_t bd0 = decoded_half_q[i][2] - desired_half_b_q;
						e0 = ((2 * (rd0 * rd0) + 3 * (gd0 * gd0) + bd0 * bd0) << 5) | i; // max 62 bits (27*2+3+5)
					}

					{
						int64_t rd1 = decoded_half_q[i + 1][0] - desired_half_r_q;
						int64_t gd1 = decoded_half_q[i + 1][1] - desired_half_g_q;
						int64_t bd1 = decoded_half_q[i + 1][2] - desired_half_b_q;
						e1 = ((2 * (rd1 * rd1) + 3 * (gd1 * gd1) + bd1 * bd1) << 5) | (i + 1);
					}

					lowest_e = minimum(lowest_e, e0, e1);
				}

				if (i != high_index)
				{
					int64_t rd0 = decoded_half_q[i][0] - desired_half_r_q;
					int64_t gd0 = decoded_half_q[i][1] - desired_half_g_q;
					int64_t bd0 = decoded_half_q[i][2] - desired_half_b_q;
					int64_t e0 = ((2 * (rd0 * rd0) + 3 * (gd0 * gd0) + bd0 * bd0) << 5) | i;

					lowest_e = minimum(lowest_e, e0);
				}

				pWeights[p] = (uint8_t)(lowest_e & 31);

				total_error += (lowest_e >> 5);

			} // p
		}
	}

	return (double)total_error;
}

//--------------------------------------------------------------------------------------------------------------------------

double eval_selectors_dual_plane(
	uint32_t channel_index,
	uint32_t num_pixels,
	uint8_t* pWeights0, uint8_t* pWeights1,
	const half_float* pBlock_pixels_half,
	uint32_t num_weight_levels,
	const half_float* pDecoded_half,
	const astc_hdr_codec_base_options& coptions,
	uint32_t usable_selector_bitmask)
{
	assert((num_pixels >= 1) && (num_pixels <= MAX_ASTC_HDR_ENC_BLOCK_PIXELS));
	assert(usable_selector_bitmask);

	const float R_WEIGHT = coptions.m_r_err_scale;
	const float G_WEIGHT = coptions.m_g_err_scale;

	double total_error = 0;

#ifdef _DEBUG
	for (uint32_t i = 0; i < num_weight_levels; i++)
	{
		assert(!is_half_inf_or_nan(pDecoded_half[i * 3 + 0]));
		assert(!is_half_inf_or_nan(pDecoded_half[i * 3 + 1]));
		assert(!is_half_inf_or_nan(pDecoded_half[i * 3 + 2]));
	}
#endif

	double decoded_half_q[MAX_SUPPORTED_WEIGHT_LEVELS][3];

	for (uint32_t i = 0; i < num_weight_levels; i++)
	{
		const half_float* p = &pDecoded_half[i * 3];

		decoded_half_q[i][0] = q(p[0], coptions.m_q_log_bias);
		decoded_half_q[i][1] = q(p[1], coptions.m_q_log_bias);
		decoded_half_q[i][2] = q(p[2], coptions.m_q_log_bias);
	}

	const double channel_weights[3] = { R_WEIGHT, G_WEIGHT, 1.0f };

	const uint32_t first_channel = (channel_index + 1) % 3;
	const uint32_t second_channel = (channel_index + 2) % 3;
	
	// First plane
	const double first_channel_weight = channel_weights[first_channel];
	const double second_channel_weight = channel_weights[second_channel];
		
	for (uint32_t p = 0; p < num_pixels; p++)
	{
		const half_float* pDesired_half = &pBlock_pixels_half[p * 3];

		const double desired_half_x_q = q(pDesired_half[first_channel], coptions.m_q_log_bias);
		const double desired_half_y_q = q(pDesired_half[second_channel], coptions.m_q_log_bias);

		double lowest_e = BIG_FLOAT_VAL;

		// this is an approximation of MSLE
		for (uint32_t i = 0; i < num_weight_levels; i++)
		{
			if (((1 << i) & usable_selector_bitmask) == 0)
				continue;

			double xd = decoded_half_q[i][first_channel] - desired_half_x_q;
			double yd = decoded_half_q[i][second_channel] - desired_half_y_q;

			double e = first_channel_weight * (xd * xd) + second_channel_weight * (yd * yd);

			if (e < lowest_e)
			{
				lowest_e = e;
				pWeights0[p] = (uint8_t)i;
			}
		}

		total_error += lowest_e;

	} // p

	// Second plane
	const double alt_channel_weight = channel_weights[channel_index];

	for (uint32_t p = 0; p < num_pixels; p++)
	{
		const half_float* pDesired_half = &pBlock_pixels_half[p * 3];

		const double desired_half_a_q = q(pDesired_half[channel_index], coptions.m_q_log_bias);
		
		double lowest_e = BIG_FLOAT_VAL;

		// this is an approximation of MSLE
		for (uint32_t i = 0; i < num_weight_levels; i++)
		{
			if (((1 << i) & usable_selector_bitmask) == 0)
				continue;

			double ad = decoded_half_q[i][channel_index] - desired_half_a_q;

			double e = alt_channel_weight * (ad * ad);

			if (e < lowest_e)
			{
				lowest_e = e;
				pWeights1[p] = (uint8_t)i;
			}
		}

		total_error += lowest_e;

	} // p

	return total_error;
}

//--------------------------------------------------------------------------------------------------------------------------

double compute_block_error(uint32_t num_pixels, const half_float* pOrig_block, const half_float* pPacked_block, const astc_hdr_codec_base_options& coptions)
{
	const float R_WEIGHT = coptions.m_r_err_scale;
	const float G_WEIGHT = coptions.m_g_err_scale;

	double total_error = 0;

	for (uint32_t p = 0; p < num_pixels; p++)
	{
		double rd = q(pOrig_block[p * 3 + 0], coptions.m_q_log_bias) - q(pPacked_block[p * 3 + 0], coptions.m_q_log_bias);
		double gd = q(pOrig_block[p * 3 + 1], coptions.m_q_log_bias) - q(pPacked_block[p * 3 + 1], coptions.m_q_log_bias);
		double bd = q(pOrig_block[p * 3 + 2], coptions.m_q_log_bias) - q(pPacked_block[p * 3 + 2], coptions.m_q_log_bias);

		double e = R_WEIGHT * (rd * rd) + G_WEIGHT * (gd * gd) + bd * bd;

		total_error += e;
	}

	return total_error;
}

//--------------------------------------------------------------------------------------------------------------------------

double compute_block_error_from_raw_weights(
	uint32_t num_pixels, const basist::half_float pBlock_pixels_half[][3],
	const uint8_t* pRaw_weights,
	int endpoints_qlog12[2][3],
	const astc_hdr_codec_base_options& coptions)
{
	// qlog12->qlog16
	int trial_e[2][3];
	for (uint32_t i = 0; i < 3; i++)
	{
		assert(endpoints_qlog12[0][i] <= (int)basist::MAX_QLOG12);
		assert(endpoints_qlog12[1][i] <= (int)basist::MAX_QLOG12);

		trial_e[0][i] = endpoints_qlog12[0][i] << 4;
		trial_e[1][i] = endpoints_qlog12[1][i] << 4;
	}

	const float R_WEIGHT = coptions.m_r_err_scale, G_WEIGHT = coptions.m_g_err_scale;

	double trial_error = 0;
	for (uint32_t p = 0; p < num_pixels; p++)
	{
		const half_float* pDesired_half = &pBlock_pixels_half[p][0];

		const double desired_half_r_q = q(pDesired_half[0], coptions.m_q_log_bias), desired_half_g_q = q(pDesired_half[1], coptions.m_q_log_bias), desired_half_b_q = q(pDesired_half[2], coptions.m_q_log_bias);

		const uint32_t c = pRaw_weights[p];
		assert(c <= 64);

		{
			half_float rf, gf, bf;
			{
				uint32_t r0 = trial_e[0][0], r1 = trial_e[1][0];
				int ri = (r0 * (64 - c) + r1 * c + 32) / 64;
				rf = astc_helpers::qlog16_to_half(ri);
			}
			{
				uint32_t g0 = trial_e[0][1], g1 = trial_e[1][1];
				int gi = (g0 * (64 - c) + g1 * c + 32) / 64;
				gf = astc_helpers::qlog16_to_half(gi);
			}
			{
				uint32_t b0 = trial_e[0][2], b1 = trial_e[1][2];
				int bi = (b0 * (64 - c) + b1 * c + 32) / 64;
				bf = astc_helpers::qlog16_to_half(bi);
			}

			const double decoded_half_q0 = q(rf, coptions.m_q_log_bias), decoded_half_q1 = q(gf, coptions.m_q_log_bias), decoded_half_q2 = q(bf, coptions.m_q_log_bias);
			const double rd = decoded_half_q0 - desired_half_r_q, gd = decoded_half_q1 - desired_half_g_q, bd = decoded_half_q2 - desired_half_b_q;
			trial_error += R_WEIGHT * (rd * rd) + G_WEIGHT * (gd * gd) + bd * bd;
		}
	}

	return trial_error;
}

//--------------------------------------------------------------------------------------------------------------------------

static inline int compute_clamped_val(int v, int l, int h, bool& did_clamp, int& max_clamp_mag)
{
	assert(l < h);

	if (v < l)
	{
		max_clamp_mag = basisu::maximum<int>(max_clamp_mag, l - v);

		v = l;
		did_clamp = true;
	}
	else if (v > h)
	{
		max_clamp_mag = basisu::maximum<int>(max_clamp_mag, v - h);

		v = h;
		did_clamp = true;
	}

	return v;
}

//--------------------------------------------------------------------------------------------------------------------------

const uint8_t s_b_bits[8] = { 7, 8, 6, 7,  8, 6, 7, 6 };
const uint8_t s_c_bits[8] = { 6, 6, 7, 7,  6, 7, 7, 7 };
const uint8_t s_d_bits[8] = { 7, 6, 7, 6,  5, 6, 5, 6 };

// val_q[] must be already packed to qlog9-qlog12.
bool pack_astc_mode11_submode(uint32_t submode, uint8_t* pEndpoints, int val_q[2][3], int& max_clamp_mag, bool early_out_if_clamped, int max_clamp_mag_accept_thresh)
{
	assert(submode <= 7);

	const uint32_t a_bits = 9 + (submode >> 1);
	const uint32_t b_bits = s_b_bits[submode];
	const uint32_t c_bits = s_c_bits[submode];
	const uint32_t d_bits = s_d_bits[submode];

	const int max_a_val = (1 << a_bits) - 1;
	const int max_b_val = (1 << b_bits) - 1;
	const int max_c_val = (1 << c_bits) - 1;

	// The maximum usable value before it turns to NaN/Inf
	const int max_a_qlog = get_max_qlog(a_bits);
	BASISU_NOTE_UNUSED(max_a_qlog);

	const int min_d_val = -(1 << (d_bits - 1));
	const int max_d_val = -min_d_val - 1;
	assert((max_d_val - min_d_val + 1) == (1 << d_bits));

	int highest_q = -1, highest_val = 0, highest_comp = 0;

	for (uint32_t c = 0; c < 3; c++)
	{
		assert(val_q[0][c] <= max_a_qlog);
		assert(val_q[1][c] <= max_a_qlog);
	}

	for (uint32_t v = 0; v < 2; v++)
	{
		for (uint32_t c = 0; c < 3; c++)
		{
			assert(val_q[v][c] >= 0 && val_q[v][c] <= max_a_val);

			if (val_q[v][c] > highest_q)
			{
				highest_q = val_q[v][c];
				highest_val = v;
				highest_comp = c;
			}
		}
	}

	const bool had_tie = (val_q[highest_val ^ 1][highest_comp] == highest_q);

	if (highest_val != 1)
	{
		for (uint32_t c = 0; c < 3; c++)
		{
			std::swap(val_q[0][c], val_q[1][c]);
		}
	}

	if (highest_comp)
	{
		std::swap(val_q[0][0], val_q[0][highest_comp]);
		std::swap(val_q[1][0], val_q[1][highest_comp]);
	}

	int orig_q[2][3];
	memcpy(orig_q, val_q, sizeof(int) * 6);

	// val[1][0] is now guaranteed to be highest
	int best_va = 0, best_vb0 = 0, best_vb1 = 0, best_vc = 0, best_vd0 = 0, best_vd1 = 0;
	int best_max_clamp_mag = 0;
	bool best_did_clamp = false;
	int best_q[2][3] = { { 0, 0, 0}, { 0, 0, 0 } };
	BASISU_NOTE_UNUSED(best_q);
	uint32_t best_dist = UINT_MAX;

	for (uint32_t pass = 0; pass < 2; pass++)
	{
		int trial_va = val_q[1][0];

		assert(trial_va <= max_a_val);
		assert(trial_va >= val_q[1][1]);
		assert(trial_va >= val_q[1][2]);

		assert(trial_va >= val_q[0][0]);
		assert(trial_va >= val_q[0][1]);
		assert(trial_va >= val_q[0][2]);

		bool did_clamp = false;
		int trial_max_clamp_mag = 0;

		int trial_vb0 = compute_clamped_val(trial_va - val_q[1][1], 0, max_b_val, did_clamp, trial_max_clamp_mag);
		int trial_vb1 = compute_clamped_val(trial_va - val_q[1][2], 0, max_b_val, did_clamp, trial_max_clamp_mag);
		int trial_vc = compute_clamped_val(trial_va - val_q[0][0], 0, max_c_val, did_clamp, trial_max_clamp_mag);
		int trial_vd0 = compute_clamped_val((trial_va - trial_vb0 - trial_vc) - val_q[0][1], min_d_val, max_d_val, did_clamp, trial_max_clamp_mag);
		int trial_vd1 = compute_clamped_val((trial_va - trial_vb1 - trial_vc) - val_q[0][2], min_d_val, max_d_val, did_clamp, trial_max_clamp_mag);

		if ((early_out_if_clamped) && (did_clamp) && (trial_max_clamp_mag > max_clamp_mag_accept_thresh))
		{
			if ((!had_tie) || (pass == 1))
			{
				max_clamp_mag = trial_max_clamp_mag;
				return true;
			}
		}

		if (!did_clamp)
		{
			// Make sure decoder gets the expected values
			assert(trial_va == val_q[1][0]);
			assert(trial_va - trial_vb0 == val_q[1][1]);
			assert(trial_va - trial_vb1 == val_q[1][2]);

			assert((trial_va - trial_vc) == val_q[0][0]);
			assert((trial_va - trial_vb0 - trial_vc - trial_vd0) == val_q[0][1]);
			assert((trial_va - trial_vb1 - trial_vc - trial_vd1) == val_q[0][2]);
		}

		const int r_e0 = clamp<int>(trial_va, 0, max_a_val);
		const int r_e1 = clamp<int>(trial_va - trial_vb0, 0, max_a_val);
		const int r_e2 = clamp<int>(trial_va - trial_vb1, 0, max_a_val);

		const int r_f0 = clamp<int>(trial_va - trial_vc, 0, max_a_val);
		const int r_f1 = clamp<int>(trial_va - trial_vb0 - trial_vc - trial_vd0, 0, max_a_val);
		const int r_f2 = clamp<int>(trial_va - trial_vb1 - trial_vc - trial_vd1, 0, max_a_val);

		assert(r_e0 <= max_a_qlog);
		assert(r_e1 <= max_a_qlog);
		assert(r_e2 <= max_a_qlog);

		assert(r_f0 <= max_a_qlog);
		assert(r_f1 <= max_a_qlog);
		assert(r_f2 <= max_a_qlog);

		if ((!did_clamp) || (!had_tie))
		{
			best_va = trial_va;
			best_vb0 = trial_vb0;
			best_vb1 = trial_vb1;
			best_vc = trial_vc;
			best_vd0 = trial_vd0;
			best_vd1 = trial_vd1;
			best_max_clamp_mag = trial_max_clamp_mag;
			best_did_clamp = did_clamp;

			best_q[1][0] = r_e0;
			best_q[1][1] = r_e1;
			best_q[1][2] = r_e2;
			best_q[0][0] = r_f0;
			best_q[0][1] = r_f1;
			best_q[0][2] = r_f2;
			break;
		}

		// we had a tie and it did clamp, try swapping L/H for a potential slight gain

		const uint32_t r_dist1 = basisu::square<int>(r_e0 - val_q[1][0]) + basisu::square<int>(r_e1 - val_q[1][1]) + basisu::square<int>(r_e2 - val_q[1][2]);
		const uint32_t r_dist0 = basisu::square<int>(r_f0 - val_q[0][0]) + basisu::square<int>(r_f1 - val_q[0][1]) + basisu::square<int>(r_f2 - val_q[0][2]);

		const uint32_t total_dist = r_dist1 + r_dist0;

		if (total_dist < best_dist)
		{
			best_dist = total_dist;

			best_va = trial_va;
			best_vb0 = trial_vb0;
			best_vb1 = trial_vb1;
			best_vc = trial_vc;
			best_vd0 = trial_vd0;
			best_vd1 = trial_vd1;
			best_did_clamp = did_clamp;

			best_q[1][0] = r_e0;
			best_q[1][1] = r_e1;
			best_q[1][2] = r_e2;
			best_q[0][0] = r_f0;
			best_q[0][1] = r_f1;
			best_q[0][2] = r_f2;
		}

		for (uint32_t c = 0; c < 3; c++)
			std::swap(val_q[0][c], val_q[1][c]);
	}

	// pack bits now
	int v0 = 0, v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0;

	int x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0;
	switch (submode)
	{
	case 0:
		x0 = get_bit(best_vb0, 6); x1 = get_bit(best_vb1, 6); x2 = get_bit(best_vd0, 6); x3 = get_bit(best_vd1, 6); x4 = get_bit(best_vd0, 5); x5 = get_bit(best_vd1, 5);
		break;
	case 1:
		x0 = get_bit(best_vb0, 6); x1 = get_bit(best_vb1, 6); x2 = get_bit(best_vb0, 7); x3 = get_bit(best_vb1, 7); x4 = get_bit(best_vd0, 5); x5 = get_bit(best_vd1, 5);
		break;
	case 2:
		x0 = get_bit(best_va, 9); x1 = get_bit(best_vc, 6); x2 = get_bit(best_vd0, 6); x3 = get_bit(best_vd1, 6); x4 = get_bit(best_vd0, 5); x5 = get_bit(best_vd1, 5);
		break;
	case 3:
		x0 = get_bit(best_vb0, 6); x1 = get_bit(best_vb1, 6); x2 = get_bit(best_va, 9); x3 = get_bit(best_vc, 6); x4 = get_bit(best_vd0, 5); x5 = get_bit(best_vd1, 5);
		break;
	case 4:
		x0 = get_bit(best_vb0, 6); x1 = get_bit(best_vb1, 6); x2 = get_bit(best_vb0, 7); x3 = get_bit(best_vb1, 7); x4 = get_bit(best_va, 9); x5 = get_bit(best_va, 10);
		break;
	case 5:
		x0 = get_bit(best_va, 9); x1 = get_bit(best_va, 10); x2 = get_bit(best_vc, 7); x3 = get_bit(best_vc, 6); x4 = get_bit(best_vd0, 5); x5 = get_bit(best_vd1, 5);
		break;
	case 6:
		x0 = get_bit(best_vb0, 6); x1 = get_bit(best_vb1, 6); x2 = get_bit(best_va, 11); x3 = get_bit(best_vc, 6); x4 = get_bit(best_va, 9); x5 = get_bit(best_va, 10);
		break;
	case 7:
		x0 = get_bit(best_va, 9); x1 = get_bit(best_va, 10); x2 = get_bit(best_va, 11); x3 = get_bit(best_vc, 6); x4 = get_bit(best_vd0, 5); x5 = get_bit(best_vd1, 5);
		break;
	default:
		break;
	}

	// write mode
	pack_bit(v1, 7, submode, 0);
	pack_bit(v2, 7, submode, 1);
	pack_bit(v3, 7, submode, 2);

	// highest component
	pack_bit(v4, 7, highest_comp, 0);
	pack_bit(v5, 7, highest_comp, 1);

	// write bit 8 of va
	pack_bit(v1, 6, best_va, 8);

	// extra bits
	pack_bit(v2, 6, x0);
	pack_bit(v3, 6, x1);
	pack_bit(v4, 6, x2);
	pack_bit(v5, 6, x3);
	pack_bit(v4, 5, x4);
	pack_bit(v5, 5, x5);

	v0 = best_va & 0xFF;
	v1 |= (best_vc & 63);
	v2 |= (best_vb0 & 63);
	v3 |= (best_vb1 & 63);
	v4 |= (best_vd0 & 31);
	v5 |= (best_vd1 & 31);

	assert(is_in_range(v0, 0, 255) && is_in_range(v1, 0, 255) && is_in_range(v2, 0, 255) && is_in_range(v3, 0, 255) && is_in_range(v4, 0, 255) && is_in_range(v5, 0, 255));

	pEndpoints[0] = (uint8_t)v0;
	pEndpoints[1] = (uint8_t)v1;
	pEndpoints[2] = (uint8_t)v2;
	pEndpoints[3] = (uint8_t)v3;
	pEndpoints[4] = (uint8_t)v4;
	pEndpoints[5] = (uint8_t)v5;

#ifdef _DEBUG
	// Test for valid pack by unpacking
	{
		if (highest_comp)
		{
			std::swap(best_q[0][0], best_q[0][highest_comp]);
			std::swap(best_q[1][0], best_q[1][highest_comp]);

			std::swap(orig_q[0][0], orig_q[0][highest_comp]);
			std::swap(orig_q[1][0], orig_q[1][highest_comp]);
		}

		int test_e[2][3];
		decode_mode11_to_qlog12(pEndpoints, test_e, astc_helpers::BISE_256_LEVELS);
		for (uint32_t i = 0; i < 2; i++)
		{
			for (uint32_t j = 0; j < 3; j++)
			{
				assert(best_q[i][j] == test_e[i][j] >> (12 - a_bits));

				if (!best_did_clamp)
				{
					assert((orig_q[i][j] == test_e[i][j] >> (12 - a_bits)) ||
						(orig_q[1 - i][j] == test_e[i][j] >> (12 - a_bits)));
				}
			}
		}
	}
#endif

	max_clamp_mag = best_max_clamp_mag;

	return best_did_clamp;
}

bool pack_astc_mode11_submode(uint32_t submode, uint8_t* pEndpoints, const vec3F& low_q16, const vec3F& high_q16, int& max_clamp_mag, bool early_out_if_clamped, int max_clamp_mag_accept_thresh)
{
	assert(submode <= 7);
		
	const uint32_t a_bits = 9 + (submode >> 1);
	const int max_a_val = (1 << a_bits) - 1;

	// The maximum usable value before it turns to NaN/Inf
	const int max_a_qlog = get_max_qlog(a_bits);

	int val_q[2][3];

	for (uint32_t c = 0; c < 3; c++)
	{
#if 0
		// This is very slightly better, but ~8% slower likely due to the table lookups.
		const half_float l = astc_helpers::qlog16_to_half((uint32_t)std::round(low_q16[c]));
		val_q[0][c] = half_to_qlog7_12(l, a_bits);

		const half_float h = astc_helpers::qlog16_to_half((uint32_t)std::round(high_q16[c]));
		val_q[1][c] = half_to_qlog7_12(h, a_bits);
#else
		// TODO: Tune quant_qlog16() for higher precision.
		val_q[0][c] = quant_qlog16((uint32_t)std::round(low_q16[c]), a_bits);
		val_q[1][c] = quant_qlog16((uint32_t)std::round(high_q16[c]), a_bits);
#endif

#if 1
		if (val_q[0][c] == val_q[1][c])
		{
#if 0
			if (l <= h)
#else
			if (low_q16[c] < high_q16[c])
#endif
			{
				if (val_q[0][c])
					val_q[0][c]--;

				if (val_q[1][c] != max_a_val)
					val_q[1][c]++;
			}
			else
			{
				if (val_q[0][c] != max_a_val)
					val_q[0][c]++;

				if (val_q[1][c])
					val_q[1][c]--;
			}
		}
#endif

		val_q[0][c] = minimum<uint32_t>(val_q[0][c], max_a_qlog);
		val_q[1][c] = minimum<uint32_t>(val_q[1][c], max_a_qlog);
	}

	return pack_astc_mode11_submode(submode, pEndpoints, val_q, max_clamp_mag, early_out_if_clamped, max_clamp_mag_accept_thresh);
}

//--------------------------------------------------------------------------------------------------------------------------

void pack_astc_mode11_direct(uint8_t* pEndpoints, vec3F l_q16, vec3F h_q16)
{
	float lg = l_q16.dot(vec3F(1.0f)), hg = h_q16.dot(vec3F(1.0f));
	if (lg > hg)
	{
		// Ensure low endpoint is generally less bright than high in direct mode.
		std::swap(l_q16, h_q16);
	}

	for (uint32_t i = 0; i < 3; i++)
	{
		// TODO: This goes from QLOG16->HALF->QLOG8/7
		half_float l_half = astc_helpers::qlog16_to_half(clamp((int)std::round(l_q16[i]), 0, 65535));
		half_float h_half = astc_helpers::qlog16_to_half(clamp((int)std::round(h_q16[i]), 0, 65535));

		int l_q, h_q;

		if (i == 2)
		{
			l_q = g_half_to_qlog7[bounds_check((uint32_t)l_half, 0U, 32768U)];
			h_q = g_half_to_qlog7[bounds_check((uint32_t)h_half, 0U, 32768U)];

			l_q = minimum<uint32_t>(l_q, MAX_QLOG7);
			h_q = minimum<uint32_t>(h_q, MAX_QLOG7);
		}
		else
		{
			l_q = g_half_to_qlog8[bounds_check((uint32_t)l_half, 0U, 32768U)];
			h_q = g_half_to_qlog8[bounds_check((uint32_t)h_half, 0U, 32768U)];

			// this quantizes R and G as 7 bits vs. 8, for grayscale.
			//l_q = g_half_to_qlog7[bounds_check((uint32_t)l_half, 0U, 32768U)] << 1;
			//h_q = g_half_to_qlog7[bounds_check((uint32_t)h_half, 0U, 32768U)] << 1;
						
			l_q = minimum<uint32_t>(l_q, MAX_QLOG8);
			h_q = minimum<uint32_t>(h_q, MAX_QLOG8);
		}

#if 1
		if (l_q == h_q)
		{
			const int m = (i == 2) ? MAX_QLOG7 : MAX_QLOG8;

			if (l_q16[i] <= h_q16[i])
			{
				if (l_q)
					l_q--;

				if (h_q != m)
					h_q++;
			}
			else
			{
				if (h_q)
					h_q--;

				if (l_q != m)
					l_q++;
			}
		}
#endif

		if (i == 2)
		{
			assert(l_q <= (int)MAX_QLOG7 && h_q <= (int)MAX_QLOG7);
			l_q |= 128;
			h_q |= 128;
		}
		else
		{
			assert(l_q <= (int)MAX_QLOG8 && h_q <= (int)MAX_QLOG8);
		}

		pEndpoints[2 * i + 0] = (uint8_t)l_q;
		pEndpoints[2 * i + 1] = (uint8_t)h_q;
	}
}

//--------------------------------------------------------------------------------------------------------------------------

bool pack_astc_mode7_submode(uint32_t submode, uint8_t* pEndpoints, const vec3F& rgb_q16, float s_q16, int& max_clamp_mag, uint32_t ise_weight_range, bool early_out_if_clamped, int max_clamp_mag_accept_thresh)
{
	assert((ise_weight_range >= MIN_SUPPORTED_ISE_WEIGHT_INDEX) && (ise_weight_range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX));

	assert(submode <= 5);
	max_clamp_mag = 0;

	static const uint8_t s_r_bits[6] = { 11, 11, 10, 9, 8, 7 };
	static const uint8_t s_g_b_bits[6] = { 5, 6, 5, 6, 7, 7 };
	static const uint8_t s_s_bits[6] = { 7, 5, 8, 7, 6, 7 };

	// The precision of the components
	const uint32_t prec_bits = s_r_bits[submode];

	int qlog[4], pack_bits[4];

	for (uint32_t i = 0; i < 4; i++)
	{
		const float f = (i == 3) ? s_q16 : rgb_q16[i];

		// The # of bits the component is packed into
		if (i == 0)
			pack_bits[i] = s_r_bits[submode];
		else if (i == 3)
			pack_bits[i] = s_s_bits[submode];
		else
			pack_bits[i] = s_g_b_bits[submode];

#if 0
		// this is slightly worse
		// TODO: going from qlog16 to half loses some precision. Then going from half to qlog 7-12 will have extra error.
		half_float h = qlog_to_half(clamp((int)std::round(f), 0, MAX_QLOG16), 16);
		qlog[i] = half_to_qlog7_12((half_float)bounds_check((uint32_t)h, 0U, 32768U), prec_bits);
#else
		qlog[i] = quant_qlog16(clamp<int>((int)std::round(f), 0, MAX_QLOG16), prec_bits);

		// Only bias if there are enough texel weights, 4=6 weights
		if (ise_weight_range >= 4)
		{
			// Explictly bias the high color, and the scale up, to better exploit the weights.
			// The quantized range also then encompases the complete input range.
			const uint32_t max_val = (1 << prec_bits) - 1;
			const uint32_t K = 3;
			if (i == 3)
			{
				qlog[i] = minimum<uint32_t>(qlog[i] + K * 2, max_val);
			}
			else
			{
				qlog[i] = minimum<uint32_t>(qlog[i] + K, max_val);
			}
		}
#endif

		if (i != 3)
			qlog[i] = minimum<uint32_t>(qlog[i], get_max_qlog(prec_bits));

		// If S=0, we lose freedom for the texel weights to add any value.
		if ((i == 3) && (qlog[i] == 0))
			qlog[i] = 1;
	}

	uint32_t maj_index = 0;

	bool did_clamp = false;

	if (submode != 5)
	{
		int largest_qlog = 0;
		for (uint32_t i = 0; i < 3; i++)
		{
			if (qlog[i] > largest_qlog)
			{
				largest_qlog = qlog[i];
				maj_index = i;
			}
		}

		if (maj_index)
		{
			std::swap(qlog[0], qlog[maj_index]);
		}

		assert(qlog[0] >= qlog[1]);
		assert(qlog[0] >= qlog[2]);

		qlog[1] = qlog[0] - qlog[1];
		qlog[2] = qlog[0] - qlog[2];

		for (uint32_t i = 1; i < 4; i++)
		{
			const int max_val = (1 << pack_bits[i]) - 1;

			if (qlog[i] > max_val)
			{
				max_clamp_mag = maximum<int>(max_clamp_mag, qlog[i] - max_val);
				qlog[i] = max_val;
				did_clamp = true;

				if ((early_out_if_clamped) && (max_clamp_mag > max_clamp_mag_accept_thresh))
					return true;
			}
		}
	}

	for (uint32_t i = 0; i < 4; i++)
	{
		const int max_val = (1 << pack_bits[i]) - 1; (void)max_val;

		assert(qlog[i] <= max_val);
	}

	int mode = 0;

	int r = qlog[0] & 63; // 6-bits
	int g = qlog[1] & 31; // 5-bits
	int b = qlog[2] & 31; // 5-bits
	int s = qlog[3] & 31; // 5-bits

	int x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0, x6 = 0;

	switch (submode)
	{
	case 0:
	{
		mode = (maj_index << 2) | 0;
		assert((mode & 0xC) != 0xC);

		x0 = get_bit(qlog[0], 9); // R9
		x1 = get_bit(qlog[0], 8); // R8
		x2 = get_bit(qlog[0], 7); // R7
		x3 = get_bit(qlog[0], 10); // R10
		x4 = get_bit(qlog[0], 6); // R6 
		x5 = get_bit(qlog[3], 6); // S6
		x6 = get_bit(qlog[3], 5); // S5
		break;
	}
	case 1:
	{
		mode = (maj_index << 2) | 1;
		assert((mode & 0xC) != 0xC);

		x0 = get_bit(qlog[0], 8); // R8
		x1 = get_bit(qlog[1], 5); // G5
		x2 = get_bit(qlog[0], 7); // R7
		x3 = get_bit(qlog[2], 5); // B5
		x4 = get_bit(qlog[0], 6); // R6 
		x5 = get_bit(qlog[0], 10); // R10
		x6 = get_bit(qlog[0], 9); // R9
		break;
	}
	case 2:
	{
		mode = (maj_index << 2) | 2;
		assert((mode & 0xC) != 0xC);

		x0 = get_bit(qlog[0], 9); // R9
		x1 = get_bit(qlog[0], 8); // R8
		x2 = get_bit(qlog[0], 7); // R7
		x3 = get_bit(qlog[0], 6); // R6
		x4 = get_bit(qlog[3], 7); // S7 
		x5 = get_bit(qlog[3], 6); // S6
		x6 = get_bit(qlog[3], 5); // S5
		break;
	}
	case 3:
	{
		mode = (maj_index << 2) | 3;
		assert((mode & 0xC) != 0xC);

		x0 = get_bit(qlog[0], 8); // R8
		x1 = get_bit(qlog[1], 5); // G5
		x2 = get_bit(qlog[0], 7); // R7
		x3 = get_bit(qlog[2], 5); // B5
		x4 = get_bit(qlog[0], 6); // R6 
		x5 = get_bit(qlog[3], 6); // S6
		x6 = get_bit(qlog[3], 5); // S5
		break;
	}
	case 4:
	{
		mode = maj_index | 0xC; // 0b1100
		assert((mode & 0xC) == 0xC);
		assert(mode != 0xF);

		x0 = get_bit(qlog[1], 6); // G6
		x1 = get_bit(qlog[1], 5); // G5
		x2 = get_bit(qlog[2], 6); // B6
		x3 = get_bit(qlog[2], 5); // B5
		x4 = get_bit(qlog[0], 6); // R6 
		x5 = get_bit(qlog[0], 7); // R7
		x6 = get_bit(qlog[3], 5); // S5
		break;
	}
	case 5:
	{
		mode = 0xF;

		x0 = get_bit(qlog[1], 6); // G6
		x1 = get_bit(qlog[1], 5); // G5
		x2 = get_bit(qlog[2], 6); // B6
		x3 = get_bit(qlog[2], 5); // B5
		x4 = get_bit(qlog[0], 6); // R6 
		x5 = get_bit(qlog[3], 6); // S6
		x6 = get_bit(qlog[3], 5); // S5
		break;
	}
	default:
	{
		assert(0);
		break;
	}
	}

	pEndpoints[0] = (uint8_t)((get_bit(mode, 1) << 7) | (get_bit(mode, 0) << 6) | r);
	pEndpoints[1] = (uint8_t)((get_bit(mode, 2) << 7) | (x0 << 6) | (x1 << 5) | g);
	pEndpoints[2] = (uint8_t)((get_bit(mode, 3) << 7) | (x2 << 6) | (x3 << 5) | b);
	pEndpoints[3] = (uint8_t)((x4 << 7) | (x5 << 6) | (x6 << 5) | s);

#ifdef _DEBUG
	// Test for valid pack by unpacking
	{
		const int inv_shift = 12 - prec_bits;

		int unpacked_e[2][3];
		if (submode != 5)
		{
			unpacked_e[1][0] = left_shift32(qlog[0], inv_shift);
			unpacked_e[1][1] = clamp(left_shift32((qlog[0] - qlog[1]), inv_shift), 0, 0xFFF);
			unpacked_e[1][2] = clamp(left_shift32((qlog[0] - qlog[2]), inv_shift), 0, 0xFFF);

			unpacked_e[0][0] = clamp(left_shift32((qlog[0] - qlog[3]), inv_shift), 0, 0xFFF);
			unpacked_e[0][1] = clamp(left_shift32(((qlog[0] - qlog[1]) - qlog[3]), inv_shift), 0, 0xFFF);
			unpacked_e[0][2] = clamp(left_shift32(((qlog[0] - qlog[2]) - qlog[3]), inv_shift), 0, 0xFFF);
		}
		else
		{
			unpacked_e[1][0] = left_shift32(qlog[0], inv_shift);
			unpacked_e[1][1] = left_shift32(qlog[1], inv_shift);
			unpacked_e[1][2] = left_shift32(qlog[2], inv_shift);

			unpacked_e[0][0] = clamp(left_shift32((qlog[0] - qlog[3]), inv_shift), 0, 0xFFF);
			unpacked_e[0][1] = clamp(left_shift32((qlog[1] - qlog[3]), inv_shift), 0, 0xFFF);
			unpacked_e[0][2] = clamp(left_shift32((qlog[2] - qlog[3]), inv_shift), 0, 0xFFF);
		}

		if (maj_index)
		{
			std::swap(unpacked_e[0][0], unpacked_e[0][maj_index]);
			std::swap(unpacked_e[1][0], unpacked_e[1][maj_index]);
		}

		int e[2][3];
		decode_mode7_to_qlog12_ise20(pEndpoints, e, nullptr);

		for (uint32_t i = 0; i < 3; i++)
		{
			assert(unpacked_e[0][i] == e[0][i]);
			assert(unpacked_e[1][i] == e[1][i]);
		}
	}
#endif

	return did_clamp;
}

//--------------------------------------------------------------------------------------------------------------------------

bool pack_mode11(mode11_log_desc& desc, uint8_t* pEndpoints)
{
	memset(pEndpoints, 0, NUM_MODE11_ENDPOINTS);

	if (desc.is_direct())
	{
		if ((desc.m_a < 0) || (desc.m_c < 0) || (desc.m_b0 < 0))
			return false;

		if (!((desc.m_a <= 255) && (desc.m_c <= 255) && (desc.m_b0 <= 127)))
			return false;

		pEndpoints[0] = (uint8_t)desc.m_a;
		pEndpoints[2] = (uint8_t)desc.m_c;
		pEndpoints[4] = (uint8_t)desc.m_b0 | 128;

		if ((desc.m_b1 < 0) || (desc.m_d0 < 0) || (desc.m_d1 < 0))
			return false;

		if (!((desc.m_b1 <= 255) && (desc.m_d0 <= 255) && (desc.m_d1 <= 127)))
			return false;

		pEndpoints[1] = (uint8_t)desc.m_b1;
		pEndpoints[3] = (uint8_t)desc.m_d0;
		pEndpoints[5] = (uint8_t)desc.m_d1 | 128;
		
		return true;
	}

	if (!((desc.m_a >= 0) && (desc.m_a <= desc.m_max_a_val)))
		return false;
	if (!(((desc.m_c >= 0) && (desc.m_c <= desc.m_max_c_val))))
		return false;
	if (!((desc.m_b0 >= 0) && (desc.m_b0 <= desc.m_max_b_val)))
		return false;
	if (!((desc.m_b1 >= 0) && (desc.m_b1 <= desc.m_max_b_val)))
		return false;
	if (!((desc.m_d0 >= desc.m_min_d_val) && (desc.m_d0 <= desc.m_max_d_val)))
		return false;
	if (!((desc.m_d1 >= desc.m_min_d_val) && (desc.m_d1 <= desc.m_max_d_val)))
		return false;

	const int va = desc.m_a, vb0 = desc.m_b0, vb1 = desc.m_b1, vc = desc.m_c, vd0 = desc.m_d0, vd1 = desc.m_d1;
	
	int v0 = 0, v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0;
	
	int x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0, x5 = 0;
	switch (desc.m_submode)
	{
	case 0:
		x0 = get_bit(vb0, 6); x1 = get_bit(vb1, 6); x2 = get_bit(vd0, 6); x3 = get_bit(vd1, 6); x4 = get_bit(vd0, 5); x5 = get_bit(vd1, 5);
		break;
	case 1:
		x0 = get_bit(vb0, 6); x1 = get_bit(vb1, 6); x2 = get_bit(vb0, 7); x3 = get_bit(vb1, 7); x4 = get_bit(vd0, 5); x5 = get_bit(vd1, 5);
		break;
	case 2:
		x0 = get_bit(va, 9); x1 = get_bit(vc, 6); x2 = get_bit(vd0, 6); x3 = get_bit(vd1, 6); x4 = get_bit(vd0, 5); x5 = get_bit(vd1, 5);
		break;
	case 3:
		x0 = get_bit(vb0, 6); x1 = get_bit(vb1, 6); x2 = get_bit(va, 9); x3 = get_bit(vc, 6); x4 = get_bit(vd0, 5); x5 = get_bit(vd1, 5);
		break;
	case 4:
		x0 = get_bit(vb0, 6); x1 = get_bit(vb1, 6); x2 = get_bit(vb0, 7); x3 = get_bit(vb1, 7); x4 = get_bit(va, 9); x5 = get_bit(va, 10);
		break;
	case 5:
		x0 = get_bit(va, 9); x1 = get_bit(va, 10); x2 = get_bit(vc, 7); x3 = get_bit(vc, 6); x4 = get_bit(vd0, 5); x5 = get_bit(vd1, 5);
		break;
	case 6:
		x0 = get_bit(vb0, 6); x1 = get_bit(vb1, 6); x2 = get_bit(va, 11); x3 = get_bit(vc, 6); x4 = get_bit(va, 9); x5 = get_bit(va, 10);
		break;
	case 7:
		x0 = get_bit(va, 9); x1 = get_bit(va, 10); x2 = get_bit(va, 11); x3 = get_bit(vc, 6); x4 = get_bit(vd0, 5); x5 = get_bit(vd1, 5);
		break;
	default:
		break;
	}

	// write mode
	pack_bit(v1, 7, desc.m_submode, 0);
	pack_bit(v2, 7, desc.m_submode, 1);
	pack_bit(v3, 7, desc.m_submode, 2);

	// highest component
	pack_bit(v4, 7, desc.m_maj_comp, 0);
	pack_bit(v5, 7, desc.m_maj_comp, 1);

	// write bit 8 of va
	pack_bit(v1, 6, va, 8);

	// extra bits
	pack_bit(v2, 6, x0);
	pack_bit(v3, 6, x1);
	pack_bit(v4, 6, x2);
	pack_bit(v5, 6, x3);
	pack_bit(v4, 5, x4);
	pack_bit(v5, 5, x5);

	v0 = va & 0xFF;
	v1 |= (vc & 63);
	v2 |= (vb0 & 63);
	v3 |= (vb1 & 63);
	v4 |= (vd0 & 31);
	v5 |= (vd1 & 31);

	assert(is_in_range(v0, 0, 255) && is_in_range(v1, 0, 255) && is_in_range(v2, 0, 255) && is_in_range(v3, 0, 255) && is_in_range(v4, 0, 255) && is_in_range(v5, 0, 255));

	pEndpoints[0] = (uint8_t)v0;
	pEndpoints[1] = (uint8_t)v1;
	pEndpoints[2] = (uint8_t)v2;
	pEndpoints[3] = (uint8_t)v3;
	pEndpoints[4] = (uint8_t)v4;
	pEndpoints[5] = (uint8_t)v5;

	return true;
}

static inline int astc_hdr_sign_extend(int src, int num_src_bits)
{
	assert(basisu::is_in_range(num_src_bits, 2, 31));

	const bool negative = (src & (1 << (num_src_bits - 1))) != 0;
	if (negative)
		return src | ~((1 << num_src_bits) - 1);
	else
		return src & ((1 << num_src_bits) - 1);
}

void unpack_mode11(const uint8_t* pEndpoints, mode11_log_desc& desc)
{
	clear_obj(desc);

	pack_bit(desc.m_maj_comp, 0, pEndpoints[4], 7);
	pack_bit(desc.m_maj_comp, 1, pEndpoints[5], 7);

	if (desc.m_maj_comp == 3)
	{
		desc.m_a = pEndpoints[0];
		desc.m_c = pEndpoints[2];
		desc.m_b0 = pEndpoints[4] & 0x7F;

		desc.m_b1 = pEndpoints[1];
		desc.m_d0 = pEndpoints[3];
		desc.m_d1 = pEndpoints[5] & 0x7F;
		
		return;
	}

	pack_bit(desc.m_submode, 0, pEndpoints[1], 7);
	pack_bit(desc.m_submode, 1, pEndpoints[2], 7);
	pack_bit(desc.m_submode, 2, pEndpoints[3], 7);

	desc.m_a = pEndpoints[0];		// 8 bits
	pack_bit(desc.m_a, 8, pEndpoints[1], 6);

	desc.m_c = pEndpoints[1] & 63;	// 6 bits
	desc.m_b0 = pEndpoints[2] & 63; // 6 bits
	desc.m_b1 = pEndpoints[3] & 63; // 6 bits
	desc.m_d0 = pEndpoints[4] & 31; // 5 bits
	desc.m_d1 = pEndpoints[5] & 31; // 5 bits

	const int x0 = get_bit(pEndpoints[2], 6);
	const int x1 = get_bit(pEndpoints[3], 6);
	const int x2 = get_bit(pEndpoints[4], 6);
	const int x3 = get_bit(pEndpoints[5], 6);
	const int x4 = get_bit(pEndpoints[4], 5);
	const int x5 = get_bit(pEndpoints[5], 5);

	switch (desc.m_submode)
	{
	case 0:
		pack_bit(desc.m_b0, 6, x0, 0); pack_bit(desc.m_b1, 6, x1, 0); pack_bit(desc.m_d0, 6, x2, 0); pack_bit(desc.m_d1, 6, x3, 0); pack_bit(desc.m_d0, 5, x4, 0); pack_bit(desc.m_d1, 5, x5, 0);
		break;
	case 1:
		pack_bit(desc.m_b0, 6, x0, 0); pack_bit(desc.m_b1, 6, x1, 0); pack_bit(desc.m_b0, 7, x2, 0); pack_bit(desc.m_b1, 7, x3, 0); pack_bit(desc.m_d0, 5, x4, 0); pack_bit(desc.m_d1, 5, x5, 0);
		break;
	case 2:
		pack_bit(desc.m_a, 9, x0, 0); pack_bit(desc.m_c, 6, x1, 0); pack_bit(desc.m_d0, 6, x2, 0); pack_bit(desc.m_d1, 6, x3, 0); pack_bit(desc.m_d0, 5, x4, 0); pack_bit(desc.m_d1, 5, x5, 0);
		break;
	case 3:
		pack_bit(desc.m_b0, 6, x0, 0); pack_bit(desc.m_b1, 6, x1, 0); pack_bit(desc.m_a, 9, x2, 0); pack_bit(desc.m_c, 6, x3, 0); pack_bit(desc.m_d0, 5, x4, 0); pack_bit(desc.m_d1, 5, x5, 0);
		break;
	case 4:
		pack_bit(desc.m_b0, 6, x0, 0); pack_bit(desc.m_b1, 6, x1, 0); pack_bit(desc.m_b0, 7, x2, 0); pack_bit(desc.m_b1, 7, x3, 0); pack_bit(desc.m_a, 9, x4, 0); pack_bit(desc.m_a, 10, x5, 0);
		break;
	case 5:
		pack_bit(desc.m_a, 9, x0, 0); pack_bit(desc.m_a, 10, x1, 0); pack_bit(desc.m_c, 7, x2, 0); pack_bit(desc.m_c, 6, x3, 0); pack_bit(desc.m_d0, 5, x4, 0); pack_bit(desc.m_d1, 5, x5, 0);
		break;
	case 6:
		pack_bit(desc.m_b0, 6, x0, 0); pack_bit(desc.m_b1, 6, x1, 0); pack_bit(desc.m_a, 11, x2, 0); pack_bit(desc.m_c, 6, x3, 0); pack_bit(desc.m_a, 9, x4, 0); pack_bit(desc.m_a, 10, x5, 0);
		break;
	case 7:
	default:
		pack_bit(desc.m_a, 9, x0, 0); pack_bit(desc.m_a, 10, x1, 0); pack_bit(desc.m_a, 11, x2, 0); pack_bit(desc.m_c, 6, x3, 0); pack_bit(desc.m_d0, 5, x4, 0); pack_bit(desc.m_d1, 5, x5, 0);
		break;
	}

	desc.m_a_bits = 9 + (desc.m_submode >> 1);
	desc.m_b_bits = s_b_bits[desc.m_submode];
	desc.m_c_bits = s_c_bits[desc.m_submode];
	desc.m_d_bits = s_d_bits[desc.m_submode];

	desc.m_max_a_val = (1 << desc.m_a_bits) - 1;
	desc.m_max_b_val = (1 << desc.m_b_bits) - 1;
	desc.m_max_c_val = (1 << desc.m_c_bits) - 1;

	desc.m_min_d_val = -(1 << (desc.m_d_bits - 1));
	desc.m_max_d_val = -desc.m_min_d_val - 1;

	desc.m_d0 = astc_hdr_sign_extend(desc.m_d0, desc.m_d_bits);
	desc.m_d1 = astc_hdr_sign_extend(desc.m_d1, desc.m_d_bits);

	assert((desc.m_a >= 0) && (desc.m_a <= desc.m_max_a_val));
	assert((desc.m_c >= 0) && (desc.m_c <= desc.m_max_c_val));
	assert((desc.m_b0 >= 0) && (desc.m_b0 <= desc.m_max_b_val));
	assert((desc.m_b1 >= 0) && (desc.m_b1 <= desc.m_max_b_val));
	assert((desc.m_d0 >= desc.m_min_d_val) && (desc.m_d0 <= desc.m_max_d_val));
	assert((desc.m_d1 >= desc.m_min_d_val) && (desc.m_d1 <= desc.m_max_d_val));
}

//--------------------------------------------------------------------------------------------------------------------------

void decode_cem_11_config(const uint8_t* pEndpoints, int& submode_index, int& maj_index)
{
	submode_index = 0;
	maj_index = 0;

	pack_bit(submode_index, 0, pEndpoints[1], 7);
	pack_bit(submode_index, 1, pEndpoints[2], 7);
	pack_bit(submode_index, 2, pEndpoints[3], 7);

	pack_bit(maj_index, 0, pEndpoints[4], 7);
	pack_bit(maj_index, 1, pEndpoints[5], 7);
}

//--------------------------------------------------------------------------------------------------------------------------

void decode_cem_7_config(const uint8_t* pEndpoints, int& submode_index, int &maj_index)
{
	const int v0 = pEndpoints[0], v1 = pEndpoints[1], v2 = pEndpoints[2], v3 = pEndpoints[3];
	(void)v3;

	// Extract mode bits and unpack to major component and mode.
	const int modeval = ((v0 & 0xC0) >> 6) | ((v1 & 0x80) >> 5) | ((v2 & 0x80) >> 4);

	if ((modeval & 0xC) != 0xC)
	{
		maj_index = modeval >> 2;
		submode_index = modeval & 3;
	}
	else if (modeval != 0xF)
	{
		maj_index = modeval & 3;
		submode_index = 4;
	}
	else
	{
		maj_index = 0;
		submode_index = 5;
	}
}

//--------------------------------------------------------------------------------------------------------------------------
// TODO: Use pack_mode11() as a shared function.

bool pack_mode11(
	const vec3F& low_color_q16, const vec3F& high_color_q16,
	uint32_t ise_endpoint_range, uint8_t* pEndpoints, 
	const astc_hdr_codec_base_options& coptions,
	bool direct_only, int32_t first_submode, int32_t last_submode, bool ignore_clamping, uint32_t& submode_used)
{
	uint8_t orig_trial_endpoints[NUM_MODE11_ENDPOINTS];

	if (direct_only)
	{
		first_submode = -1;
		last_submode = -1;
	}

	assert(first_submode <= last_submode);
	assert((first_submode >= -1) && (first_submode <= 7));
	assert((last_submode >= -1) && (last_submode <= 7));

	memset(pEndpoints, 0, NUM_MODE11_ENDPOINTS);

	double best_trial_dist = BIG_FLOAT_VAL;
	int best_submode = 0;

	for (int submode = last_submode; submode >= first_submode; submode--)
	{
		bool did_clamp = false;
		int max_clamp_mag = 0;
		if (submode == -1)
		{
			// If it had to clamp with one of the submodes, try direct which can't clamp, but has low precision.
			pack_astc_mode11_direct(orig_trial_endpoints, low_color_q16, high_color_q16);
		}
		else
		{
			const int MAX_CLAMP_MAG_ACCEPT_THRESH = 32;
			did_clamp = pack_astc_mode11_submode(submode, orig_trial_endpoints, low_color_q16, high_color_q16, max_clamp_mag, !ignore_clamping, MAX_CLAMP_MAG_ACCEPT_THRESH);

			if (!ignore_clamping)
			{
				// If it had to clamp and the clamp was too high, it'll distort the endpoint colors too much, which could lead to noticeable artifacts.
				if ((did_clamp) && (max_clamp_mag > MAX_CLAMP_MAG_ACCEPT_THRESH))
					continue;
			}
		}

		uint8_t trial_endpoints[NUM_MODE11_ENDPOINTS];

		// This will distort the endpoints if the ISE endpoint range isn't 256 levels (20).
		// It could massively distort the endpoints, but still result in a valid encoding.
		basist::astc_6x6_hdr::requantize_ise_endpoints(11, astc_helpers::BISE_256_LEVELS, orig_trial_endpoints, ise_endpoint_range, trial_endpoints);

		int e[2][3];
		if (!decode_mode11_to_qlog12(trial_endpoints, e, ise_endpoint_range))
			continue;

		vec3F e0(
			(float)(e[0][0] << 4),
			(float)(e[0][1] << 4),
			(float)(e[0][2] << 4)
		);

		vec3F e1(
			(float)(e[1][0] << 4),
			(float)(e[1][1] << 4),
			(float)(e[1][2] << 4)
		);

		double dist0 = e0.squared_distance_d(low_color_q16) + e1.squared_distance_d(high_color_q16);
		double dist1 = e1.squared_distance_d(low_color_q16) + e0.squared_distance_d(high_color_q16);
		double dist = helpers::minimum(dist0, dist1);

		if (dist < best_trial_dist)
		{
			best_trial_dist = dist;
			best_submode = submode;
			memcpy(pEndpoints, trial_endpoints, NUM_MODE11_ENDPOINTS);
		}

		if (coptions.m_take_first_non_clamping_mode11_submode)
		{
			if (!did_clamp)
				break;
		}

	} // submode

	if ((coptions.m_ultra_quant) &&
		(ise_endpoint_range < astc_helpers::BISE_256_LEVELS) &&
		(best_trial_dist != BIG_FLOAT_VAL))
	{
		uint8_t orig_best_trial_endpoints[NUM_MODE11_ENDPOINTS];
		memcpy(orig_best_trial_endpoints, pEndpoints, NUM_MODE11_ENDPOINTS);

		for (uint32_t c = 0; c < NUM_MODE11_ENDPOINTS; c++)
		{
			for (int dt = 0; dt <= 1; dt++)
			{
				const int d = dt ? 1 : -1;

				uint8_t varied_endpoints[NUM_MODE11_ENDPOINTS];
				memcpy(varied_endpoints, orig_best_trial_endpoints, NUM_MODE11_ENDPOINTS);

				int ise = varied_endpoints[c];

				int rank = astc_helpers::g_dequant_tables.get_endpoint_tab(ise_endpoint_range).m_ISE_to_rank[ise];
				rank = clamp<int>(rank + d, 0, astc_helpers::get_ise_levels(ise_endpoint_range) - 1);

				ise = astc_helpers::g_dequant_tables.get_endpoint_tab(ise_endpoint_range).m_rank_to_ISE[rank];

				varied_endpoints[c] = (uint8_t)ise;

				int e[2][3];
				if (!decode_mode11_to_qlog12(varied_endpoints, e, ise_endpoint_range))
					continue;

				vec3F e0(
					(float)(e[0][0] << 4),
					(float)(e[0][1] << 4),
					(float)(e[0][2] << 4)
				);

				vec3F e1(
					(float)(e[1][0] << 4),
					(float)(e[1][1] << 4),
					(float)(e[1][2] << 4)
				);

				double dist0 = e0.squared_distance_d(low_color_q16) + e1.squared_distance_d(high_color_q16);
				double dist1 = e1.squared_distance_d(low_color_q16) + e0.squared_distance_d(high_color_q16);
				double dist = helpers::minimum(dist0, dist1);

				if (dist < best_trial_dist)
				{
					best_trial_dist = dist;
					memcpy(pEndpoints, varied_endpoints, NUM_MODE11_ENDPOINTS);
				}
			} // d
		} // c
	} // if (coptions.m_ultra_quant)
		
	submode_used = best_submode + 1;

	return (best_trial_dist != BIG_FLOAT_VAL);
}

bool try_mode11(uint32_t num_pixels,
	uint8_t* pEndpoints, uint8_t* pWeights, double& cur_block_error, uint32_t& submode_used,
	const vec3F& low_color_q16, const vec3F& high_color_q16,
	const basist::half_float block_pixels_half[][3],
	uint32_t num_weight_levels, uint32_t ise_weight_range, const astc_hdr_codec_base_options& coptions, bool direct_only, uint32_t ise_endpoint_range,
	bool constrain_ise_weight_selectors,
	int32_t first_submode, int32_t last_submode, bool ignore_clamping) // -1, 7
{
	assert((ise_weight_range >= MIN_SUPPORTED_ISE_WEIGHT_INDEX) && (ise_weight_range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX));
	assert((num_weight_levels >= MIN_SUPPORTED_WEIGHT_LEVELS) && (num_weight_levels <= MAX_SUPPORTED_WEIGHT_LEVELS));
	assert((num_pixels >= 1) && (num_pixels <= MAX_ASTC_HDR_ENC_BLOCK_PIXELS));
	assert(num_weight_levels == astc_helpers::get_ise_levels(ise_weight_range));

	half_float decoded_half[MAX_SUPPORTED_WEIGHT_LEVELS][3];
	uint8_t orig_trial_endpoints[NUM_MODE11_ENDPOINTS], trial_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];

	if (direct_only)
	{
		first_submode = -1;
		last_submode = -1;
	}

	assert(first_submode <= last_submode);
	assert((first_submode >= -1) && (first_submode <= 7));
	assert((last_submode >= -1) && (last_submode <= 7));

	uint8_t best_trial_endpoints[NUM_MODE11_ENDPOINTS];
	clear_obj(best_trial_endpoints);
	double best_trial_dist = BIG_FLOAT_VAL;
	int best_submode = 0;

	for (int submode = last_submode; submode >= first_submode; submode--)
	{
		bool did_clamp = false;
		int max_clamp_mag = 0;
		if (submode == -1)
		{
			// If it had to clamp with one of the submodes, try direct which can't clamp, but has low precision.
			pack_astc_mode11_direct(orig_trial_endpoints, low_color_q16, high_color_q16);
		}
		else
		{
			const int MAX_CLAMP_MAG_ACCEPT_THRESH = 32;
			did_clamp = pack_astc_mode11_submode(submode, orig_trial_endpoints, low_color_q16, high_color_q16, max_clamp_mag, !ignore_clamping, MAX_CLAMP_MAG_ACCEPT_THRESH);

			if (!ignore_clamping)
			{
				// If it had to clamp and the clamp was too high, it'll distort the endpoint colors too much, which could lead to noticeable artifacts.
				if ((did_clamp) && (max_clamp_mag > MAX_CLAMP_MAG_ACCEPT_THRESH))
					continue;
			}
		}

		uint8_t trial_endpoints[NUM_MODE11_ENDPOINTS];

		// This will distort the endpoints if the ISE endpoint range isn't 256 levels (20).
		// It could massively distort the endpoints, but still result in a valid encoding.
		basist::astc_6x6_hdr::requantize_ise_endpoints(11, astc_helpers::BISE_256_LEVELS, orig_trial_endpoints, ise_endpoint_range, trial_endpoints);

		int e[2][3];
		if (!decode_mode11_to_qlog12(trial_endpoints, e, ise_endpoint_range))
			continue;

		vec3F e0(
			(float)(e[0][0] << 4),
			(float)(e[0][1] << 4),
			(float)(e[0][2] << 4)
		);

		vec3F e1(
			(float)(e[1][0] << 4),
			(float)(e[1][1] << 4),
			(float)(e[1][2] << 4)
		);

		double dist0 = e0.squared_distance_d(low_color_q16) + e1.squared_distance_d(high_color_q16);
		double dist1 = e1.squared_distance_d(low_color_q16) + e0.squared_distance_d(high_color_q16);
		double dist = helpers::minimum(dist0, dist1);

		if (dist < best_trial_dist)
		{
			best_trial_dist = dist;
			best_submode = submode;
			memcpy(best_trial_endpoints, trial_endpoints, sizeof(best_trial_endpoints));
		}

		if (coptions.m_take_first_non_clamping_mode11_submode)
		{
			if (!did_clamp)
				break;
		}

	} // submode

	if ((coptions.m_ultra_quant) &&
		(ise_endpoint_range < astc_helpers::BISE_256_LEVELS) &&
		(best_trial_dist != BIG_FLOAT_VAL))
	{
		uint8_t orig_best_trial_endpoints[NUM_MODE11_ENDPOINTS];
		memcpy(orig_best_trial_endpoints, best_trial_endpoints, NUM_MODE11_ENDPOINTS);

		for (uint32_t c = 0; c < NUM_MODE11_ENDPOINTS; c++)
		{
			for (int dt = 0; dt <= 1; dt++)
			{
				const int d = dt ? 1 : -1;

				uint8_t varied_endpoints[NUM_MODE11_ENDPOINTS];
				memcpy(varied_endpoints, orig_best_trial_endpoints, NUM_MODE11_ENDPOINTS);

				int ise = varied_endpoints[c];

				int rank = astc_helpers::g_dequant_tables.get_endpoint_tab(ise_endpoint_range).m_ISE_to_rank[ise];
				rank = clamp<int>(rank + d, 0, astc_helpers::get_ise_levels(ise_endpoint_range) - 1);

				ise = astc_helpers::g_dequant_tables.get_endpoint_tab(ise_endpoint_range).m_rank_to_ISE[rank];

				varied_endpoints[c] = (uint8_t)ise;

				int e[2][3];
				if (!decode_mode11_to_qlog12(varied_endpoints, e, ise_endpoint_range))
					continue;

				vec3F e0(
					(float)(e[0][0] << 4),
					(float)(e[0][1] << 4),
					(float)(e[0][2] << 4)
				);

				vec3F e1(
					(float)(e[1][0] << 4),
					(float)(e[1][1] << 4),
					(float)(e[1][2] << 4)
				);

				double dist0 = e0.squared_distance_d(low_color_q16) + e1.squared_distance_d(high_color_q16);
				double dist1 = e1.squared_distance_d(low_color_q16) + e0.squared_distance_d(high_color_q16);
				double dist = helpers::minimum(dist0, dist1);

				if (dist < best_trial_dist)
				{
					best_trial_dist = dist;
					memcpy(best_trial_endpoints, varied_endpoints, NUM_MODE11_ENDPOINTS);
				}
			} // d
		} // c
	} // if (coptions.m_ultra_quant)

	bool improved_flag = false;

	if (best_trial_dist != BIG_FLOAT_VAL)
	{
		if (get_astc_hdr_mode_11_block_colors(best_trial_endpoints, &decoded_half[0][0], nullptr, num_weight_levels, ise_weight_range, ise_endpoint_range))
		{
			uint32_t usable_selector_bitmask = UINT32_MAX;
			if ((constrain_ise_weight_selectors) && (ise_weight_range == astc_helpers::BISE_16_LEVELS))
				usable_selector_bitmask = (1 << 0) | (1 << 1) | (1 << 4) | (1 << 5) | (1 << 10) | (1 << 11) | (1 << 14) | (1 << 15);
			else if ((constrain_ise_weight_selectors) && (ise_weight_range == astc_helpers::BISE_12_LEVELS))
				usable_selector_bitmask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);

			double trial_blk_error = eval_selectors(num_pixels, trial_weights, ise_weight_range, &block_pixels_half[0][0], num_weight_levels, &decoded_half[0][0], coptions, usable_selector_bitmask);
			if (trial_blk_error < cur_block_error)
			{
				cur_block_error = trial_blk_error;
				memcpy(pEndpoints, best_trial_endpoints, NUM_MODE11_ENDPOINTS);
				memcpy(pWeights, trial_weights, num_pixels);
				submode_used = best_submode + 1;
				improved_flag = true;
			}
		}
	}

	return improved_flag;
}

//--------------------------------------------------------------------------------------------------------------------------

bool try_mode11_dual_plane(uint32_t channel_index, uint32_t num_pixels,
	uint8_t* pEndpoints, uint8_t* pWeights0, uint8_t* pWeights1, double& cur_block_error, uint32_t& submode_used,
	const vec3F& low_color_q16, const vec3F& high_color_q16,
	const basist::half_float block_pixels_half[][3],
	uint32_t num_weight_levels, uint32_t ise_weight_range, const astc_hdr_codec_base_options& coptions, bool direct_only, uint32_t ise_endpoint_range,
	bool constrain_ise_weight_selectors,
	int32_t first_submode, int32_t last_submode, bool ignore_clamping) // -1, 7
{
	assert(channel_index <= 2);
	assert((ise_weight_range >= MIN_SUPPORTED_ISE_WEIGHT_INDEX) && (ise_weight_range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX));
	assert((num_weight_levels >= MIN_SUPPORTED_WEIGHT_LEVELS) && (num_weight_levels <= MAX_SUPPORTED_WEIGHT_LEVELS));
	assert((num_pixels >= 1) && (num_pixels <= MAX_ASTC_HDR_ENC_BLOCK_PIXELS));
	assert(num_weight_levels == astc_helpers::get_ise_levels(ise_weight_range));

	half_float decoded_half[MAX_SUPPORTED_WEIGHT_LEVELS][3];
	uint8_t orig_trial_endpoints[NUM_MODE11_ENDPOINTS], trial_weights0[MAX_ASTC_HDR_ENC_BLOCK_PIXELS], trial_weights1[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];

	if (direct_only)
	{
		first_submode = -1;
		last_submode = -1;
	}

	assert(first_submode <= last_submode);
	assert((first_submode >= -1) && (first_submode <= 7));
	assert((last_submode >= -1) && (last_submode <= 7));

	uint8_t best_trial_endpoints[NUM_MODE11_ENDPOINTS];
	clear_obj(best_trial_endpoints);

	double best_trial_dist = BIG_FLOAT_VAL;
	int best_submode = 0;

	for (int submode = last_submode; submode >= first_submode; submode--)
	{
		bool did_clamp = false;
		int max_clamp_mag = 0;
		if (submode == -1)
		{
			// If it had to clamp with one of the submodes, try direct which can't clamp, but has low precision.
			pack_astc_mode11_direct(orig_trial_endpoints, low_color_q16, high_color_q16);
		}
		else
		{
			const int MAX_CLAMP_MAG_ACCEPT_THRESH = 32;
			did_clamp = pack_astc_mode11_submode(submode, orig_trial_endpoints, low_color_q16, high_color_q16, max_clamp_mag, !ignore_clamping, MAX_CLAMP_MAG_ACCEPT_THRESH);

			if (!ignore_clamping)
			{
				// If it had to clamp and the clamp was too high, it'll distort the endpoint colors too much, which could lead to noticeable artifacts.
				if ((did_clamp) && (max_clamp_mag > MAX_CLAMP_MAG_ACCEPT_THRESH))
					continue;
			}
		}

		uint8_t trial_endpoints[NUM_MODE11_ENDPOINTS];

		// This will distort the endpoints if the ISE endpoint range isn't 256 levels (20).
		// It could massively distort the endpoints, but still result in a valid encoding.
		basist::astc_6x6_hdr::requantize_ise_endpoints(11, astc_helpers::BISE_256_LEVELS, orig_trial_endpoints, ise_endpoint_range, trial_endpoints);

		int e[2][3];
		if (!decode_mode11_to_qlog12(trial_endpoints, e, ise_endpoint_range))
			continue;

		vec3F e0(
			(float)(e[0][0] << 4),
			(float)(e[0][1] << 4),
			(float)(e[0][2] << 4)
		);

		vec3F e1(
			(float)(e[1][0] << 4),
			(float)(e[1][1] << 4),
			(float)(e[1][2] << 4)
		);

		double dist0 = e0.squared_distance_d(low_color_q16) + e1.squared_distance_d(high_color_q16);
		double dist1 = e1.squared_distance_d(low_color_q16) + e0.squared_distance_d(high_color_q16);
		double dist = helpers::minimum(dist0, dist1);

		if (dist < best_trial_dist)
		{
			best_trial_dist = dist;
			best_submode = submode;
			memcpy(best_trial_endpoints, trial_endpoints, sizeof(best_trial_endpoints));
		}

		if (coptions.m_take_first_non_clamping_mode11_submode)
		{
			if (!did_clamp)
				break;
		}

	} // submode

	if ((coptions.m_ultra_quant) &&
		(ise_endpoint_range < astc_helpers::BISE_256_LEVELS) &&
		(best_trial_dist != BIG_FLOAT_VAL))
	{
		uint8_t orig_best_trial_endpoints[NUM_MODE11_ENDPOINTS];
		memcpy(orig_best_trial_endpoints, best_trial_endpoints, NUM_MODE11_ENDPOINTS);

		for (uint32_t c = 0; c < NUM_MODE11_ENDPOINTS; c++)
		{
			for (int dt = 0; dt <= 1; dt++)
			{
				const int d = dt ? 1 : -1;

				uint8_t varied_endpoints[NUM_MODE11_ENDPOINTS];
				memcpy(varied_endpoints, orig_best_trial_endpoints, NUM_MODE11_ENDPOINTS);

				int ise = varied_endpoints[c];

				int rank = astc_helpers::g_dequant_tables.get_endpoint_tab(ise_endpoint_range).m_ISE_to_rank[ise];
				rank = clamp<int>(rank + d, 0, astc_helpers::get_ise_levels(ise_endpoint_range) - 1);

				ise = astc_helpers::g_dequant_tables.get_endpoint_tab(ise_endpoint_range).m_rank_to_ISE[rank];

				varied_endpoints[c] = (uint8_t)ise;

				int e[2][3];
				if (!decode_mode11_to_qlog12(varied_endpoints, e, ise_endpoint_range))
					continue;

				vec3F e0(
					(float)(e[0][0] << 4),
					(float)(e[0][1] << 4),
					(float)(e[0][2] << 4)
				);

				vec3F e1(
					(float)(e[1][0] << 4),
					(float)(e[1][1] << 4),
					(float)(e[1][2] << 4)
				);

				double dist0 = e0.squared_distance_d(low_color_q16) + e1.squared_distance_d(high_color_q16);
				double dist1 = e1.squared_distance_d(low_color_q16) + e0.squared_distance_d(high_color_q16);
				double dist = helpers::minimum(dist0, dist1);

				if (dist < best_trial_dist)
				{
					best_trial_dist = dist;
					memcpy(best_trial_endpoints, varied_endpoints, NUM_MODE11_ENDPOINTS);
				}
			} // d
		} // c
	} // if (coptions.m_ultra_quant)

	bool improved_flag = false;

	if (best_trial_dist != BIG_FLOAT_VAL)
	{
		if (get_astc_hdr_mode_11_block_colors(best_trial_endpoints, &decoded_half[0][0], nullptr, num_weight_levels, ise_weight_range, ise_endpoint_range))
		{
			uint32_t usable_selector_bitmask = UINT32_MAX;
			if ((constrain_ise_weight_selectors) && (ise_weight_range == astc_helpers::BISE_16_LEVELS))
				usable_selector_bitmask = (1 << 0) | (1 << 1) | (1 << 4) | (1 << 5) | (1 << 10) | (1 << 11) | (1 << 14) | (1 << 15);
			else if ((constrain_ise_weight_selectors) && (ise_weight_range == astc_helpers::BISE_12_LEVELS))
				usable_selector_bitmask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);

			double trial_blk_error = eval_selectors_dual_plane(channel_index, num_pixels, trial_weights0, trial_weights1, &block_pixels_half[0][0], num_weight_levels, &decoded_half[0][0], coptions, usable_selector_bitmask);
			if (trial_blk_error < cur_block_error)
			{
				cur_block_error = trial_blk_error;
				memcpy(pEndpoints, best_trial_endpoints, NUM_MODE11_ENDPOINTS);
				memcpy(pWeights0, trial_weights0, num_pixels);
				memcpy(pWeights1, trial_weights1, num_pixels);
				submode_used = best_submode + 1;
				improved_flag = true;
			}
		}
	}

	return improved_flag;
}

//--------------------------------------------------------------------------------------------------------------------------

bool pack_mode7(
	const vec3F& high_color_q16, const float s_q16,
	uint32_t ise_endpoint_range, uint8_t* pEndpoints,
	uint32_t ise_weight_range, // only used for determining biasing during packing
	const astc_hdr_codec_base_options& coptions,
	int32_t first_submode, int32_t last_submode, bool ignore_clamping, uint32_t& submode_used)
{
	assert(first_submode <= last_submode);
	assert((first_submode >= 0) && (first_submode <= (int)MAX_MODE7_SUBMODE_INDEX));
	assert(last_submode <= (int)MAX_MODE7_SUBMODE_INDEX);

	uint8_t unquant_trial_endpoints[NUM_MODE7_ENDPOINTS];

	memset(pEndpoints, 0, NUM_MODE7_ENDPOINTS);

	double best_trial_dist = BIG_FLOAT_VAL;
	int best_trial_submode = 0;

	for (int submode = first_submode; submode <= last_submode; submode++)
	{
		const int MAX_CLAMP_MAG_ACCEPT_THRESH = 16;

		int max_clamp_mag = 0;
		const bool did_clamp = pack_astc_mode7_submode(submode, unquant_trial_endpoints, high_color_q16, s_q16, max_clamp_mag, ise_weight_range, !ignore_clamping, MAX_CLAMP_MAG_ACCEPT_THRESH);

		if (submode < 5)
		{
			if (!ignore_clamping)
			{
				if ((did_clamp) && (max_clamp_mag > MAX_CLAMP_MAG_ACCEPT_THRESH))
					continue;
			}
		}

		uint8_t trial_endpoints[NUM_MODE7_ENDPOINTS];

		// This will distort the endpoints if the ISE endpoint range isn't 256 levels (20).
		// It could massively distort the endpoints, but still result in a valid encoding.
		basist::astc_6x6_hdr::requantize_ise_endpoints(7, astc_helpers::BISE_256_LEVELS, unquant_trial_endpoints, ise_endpoint_range, trial_endpoints);

		int e[2][3];
		int decoded_s = 0;
		if (!decode_mode7_to_qlog12(trial_endpoints, e, &decoded_s, ise_endpoint_range))
			continue;

		// e1 is always the high color
		vec3F e1(
			(float)(e[1][0] << 4),
			(float)(e[1][1] << 4),
			(float)(e[1][2] << 4)
		);

		decoded_s <<= 4;

		double dist = e1.squared_distance_d(high_color_q16) + squared((double)decoded_s - s_q16) * 3;

		if (dist < best_trial_dist)
		{
			best_trial_dist = dist;
			best_trial_submode = submode;
			memcpy(pEndpoints, trial_endpoints, NUM_MODE7_ENDPOINTS);
		}

		if (coptions.m_take_first_non_clamping_mode7_submode)
		{
			if (!did_clamp)
				break;
		}

	} // submode

	if ((coptions.m_ultra_quant) &&
		(ise_endpoint_range < astc_helpers::BISE_256_LEVELS) &&
		(best_trial_dist != BIG_FLOAT_VAL))
	{
		uint8_t orig_best_trial_endpoints[NUM_MODE7_ENDPOINTS];
		memcpy(orig_best_trial_endpoints, pEndpoints, NUM_MODE7_ENDPOINTS);

		vec3F low_color_q16(high_color_q16 - vec3F(s_q16));
		low_color_q16.clamp(0.0f, 65535.0f);

		for (uint32_t c = 0; c < NUM_MODE7_ENDPOINTS; c++)
		{
			for (int dt = 0; dt <= 1; dt++)
			{
				const int d = dt ? 1 : -1;

				uint8_t varied_endpoints[NUM_MODE7_ENDPOINTS];
				memcpy(varied_endpoints, orig_best_trial_endpoints, NUM_MODE7_ENDPOINTS);

				int ise = varied_endpoints[c];

				int rank = astc_helpers::g_dequant_tables.get_endpoint_tab(ise_endpoint_range).m_ISE_to_rank[ise];
				rank = clamp<int>(rank + d, 0, astc_helpers::get_ise_levels(ise_endpoint_range) - 1);

				ise = astc_helpers::g_dequant_tables.get_endpoint_tab(ise_endpoint_range).m_rank_to_ISE[rank];

				varied_endpoints[c] = (uint8_t)ise;

				int e[2][3];
				int decoded_s = 0;
				if (!decode_mode7_to_qlog12(varied_endpoints, e, &decoded_s, ise_endpoint_range))
					continue;

				// e1 is always the high color
				vec3F e1(
					(float)(e[1][0] << 4),
					(float)(e[1][1] << 4),
					(float)(e[1][2] << 4)
				);

				decoded_s <<= 4;

				double dist = e1.squared_distance_d(high_color_q16) + squared((double)decoded_s - s_q16) * 3;

				if (dist < best_trial_dist)
				{
					best_trial_dist = dist;
					memcpy(pEndpoints, varied_endpoints, NUM_MODE7_ENDPOINTS);
				}

			} // d
		} // c
	}

	submode_used = best_trial_submode;

	return (best_trial_dist != BIG_FLOAT_VAL);
}

//--------------------------------------------------------------------------------------------------------------------------

bool try_mode7(
	uint32_t num_pixels,
	uint8_t* pEndpoints, uint8_t* pWeights, double& cur_block_error, uint32_t& submode_used,
	const vec3F& high_color_q16, const float s_q16,
	const half_float block_pixels_half[][3],
	uint32_t num_weight_levels, uint32_t ise_weight_range, const astc_hdr_codec_base_options& coptions,
	uint32_t ise_endpoint_range,
	int32_t first_submode, int32_t last_submode)
{
	assert((ise_weight_range >= MIN_SUPPORTED_ISE_WEIGHT_INDEX) && (ise_weight_range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX));
	assert((num_pixels >= 1) && (num_pixels <= MAX_ASTC_HDR_ENC_BLOCK_PIXELS));

	assert(first_submode <= last_submode);
	assert((first_submode >= 0) && (first_submode <= (int)MAX_MODE7_SUBMODE_INDEX));
	assert(last_submode <= (int)MAX_MODE7_SUBMODE_INDEX);
	assert(num_weight_levels == astc_helpers::get_ise_levels(ise_weight_range));

	uint8_t unquant_trial_endpoints[NUM_MODE7_ENDPOINTS];

	uint8_t best_trial_endpoints[NUM_MODE7_ENDPOINTS];
	clear_obj(best_trial_endpoints);
	double best_trial_dist = BIG_FLOAT_VAL;
	int best_trial_submode = 0;
		
	for (int submode = first_submode; submode <= last_submode; submode++)
	{
		const int MAX_CLAMP_MAG_ACCEPT_THRESH = 16;

		int max_clamp_mag = 0;
		const bool did_clamp = pack_astc_mode7_submode(submode, unquant_trial_endpoints, high_color_q16, s_q16, max_clamp_mag, ise_weight_range, true, MAX_CLAMP_MAG_ACCEPT_THRESH);

		if (submode < 5)
		{
			if ((did_clamp) && (max_clamp_mag > MAX_CLAMP_MAG_ACCEPT_THRESH))
				continue;
		}

		uint8_t trial_endpoints[NUM_MODE7_ENDPOINTS];

		// This will distort the endpoints if the ISE endpoint range isn't 256 levels (20).
		// It could massively distort the endpoints, but still result in a valid encoding.
		basist::astc_6x6_hdr::requantize_ise_endpoints(7, astc_helpers::BISE_256_LEVELS, unquant_trial_endpoints, ise_endpoint_range, trial_endpoints);

		int e[2][3];
		int decoded_s = 0;
		if (!decode_mode7_to_qlog12(trial_endpoints, e, &decoded_s, ise_endpoint_range))
			continue;

		// e1 is always the high color
		vec3F e1(
			(float)(e[1][0] << 4),
			(float)(e[1][1] << 4),
			(float)(e[1][2] << 4)
		);

		decoded_s <<= 4;

		double dist = e1.squared_distance_d(high_color_q16) + squared((double)decoded_s - s_q16) * 3;

		if (dist < best_trial_dist)
		{
			best_trial_dist = dist;
			best_trial_submode = submode;
			memcpy(best_trial_endpoints, trial_endpoints, sizeof(best_trial_endpoints));
		}

		if (coptions.m_take_first_non_clamping_mode7_submode)
		{
			if (!did_clamp)
				break;
		}

	} // submode

	if ((coptions.m_ultra_quant) &&
		(ise_endpoint_range < astc_helpers::BISE_256_LEVELS) &&
		(best_trial_dist != BIG_FLOAT_VAL))
	{
		uint8_t orig_best_trial_endpoints[NUM_MODE7_ENDPOINTS];
		memcpy(orig_best_trial_endpoints, best_trial_endpoints, NUM_MODE7_ENDPOINTS);

		vec3F low_color_q16(high_color_q16 - vec3F(s_q16));
		low_color_q16.clamp(0.0f, 65535.0f);

		for (uint32_t c = 0; c < NUM_MODE7_ENDPOINTS; c++)
		{
			for (int dt = 0; dt <= 1; dt++)
			{
				const int d = dt ? 1 : -1;

				uint8_t varied_endpoints[NUM_MODE7_ENDPOINTS];
				memcpy(varied_endpoints, orig_best_trial_endpoints, NUM_MODE7_ENDPOINTS);

				int ise = varied_endpoints[c];

				int rank = astc_helpers::g_dequant_tables.get_endpoint_tab(ise_endpoint_range).m_ISE_to_rank[ise];
				rank = clamp<int>(rank + d, 0, astc_helpers::get_ise_levels(ise_endpoint_range) - 1);

				ise = astc_helpers::g_dequant_tables.get_endpoint_tab(ise_endpoint_range).m_rank_to_ISE[rank];

				varied_endpoints[c] = (uint8_t)ise;

				int e[2][3];
				int decoded_s = 0;
				if (!decode_mode7_to_qlog12(varied_endpoints, e, &decoded_s, ise_endpoint_range))
					continue;

				// e1 is always the high color
				vec3F e1(
					(float)(e[1][0] << 4),
					(float)(e[1][1] << 4),
					(float)(e[1][2] << 4)
				);

				decoded_s <<= 4;

				double dist = e1.squared_distance_d(high_color_q16) + squared((double)decoded_s - s_q16) * 3;

				if (dist < best_trial_dist)
				{
					best_trial_dist = dist;
					memcpy(best_trial_endpoints, varied_endpoints, NUM_MODE7_ENDPOINTS);
				}

			} // d
		} // c
	}

	bool improved_flag = false;

	if (best_trial_dist != BIG_FLOAT_VAL)
	{
		half_float decoded_half[MAX_SUPPORTED_WEIGHT_LEVELS][3];
		uint8_t trial_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];

		if (get_astc_hdr_mode_7_block_colors(best_trial_endpoints, &decoded_half[0][0], nullptr, num_weight_levels, ise_weight_range, ise_endpoint_range))
		{
			double trial_blk_error = eval_selectors(num_pixels, trial_weights, ise_weight_range, &block_pixels_half[0][0], num_weight_levels, &decoded_half[0][0], coptions);
			if (trial_blk_error < cur_block_error)
			{
				cur_block_error = trial_blk_error;
				memcpy(pEndpoints, best_trial_endpoints, NUM_MODE7_ENDPOINTS);
				memcpy(pWeights, trial_weights, num_pixels);
				submode_used = best_trial_submode;
				improved_flag = true;
			}
		}
	}

	return improved_flag;
}

//--------------------------------------------------------------------------------------------------------------------------
const float LOW_EMPHASIS_WEIGHT = 1.0f, MIDDLE_EMPHASIS_WEIGHT = 1.25f, HIGH_EMPHASIS_WEIGHT = 1.0f;
const float LOW_EMPHASIS_WEIGHT_HEAVY = 1.0f, MIDDLE_EMPHASIS_WEIGHT_HEAVY = 4.0f, HIGH_EMPHASIS_WEIGHT_HEAVY = 1.0f;

double encode_astc_hdr_block_mode_11(
	uint32_t num_pixels,
	const basist::half_float pBlock_pixels_half[][3], const vec4F pBlock_pixels_q16[],
	uint32_t ise_weight_range,
	uint32_t& best_submode,
	double cur_block_error,
	uint8_t* blk_endpoints, uint8_t* blk_weights,
	const astc_hdr_codec_base_options& coptions,
	bool direct_only,
	uint32_t ise_endpoint_range,
	bool uber_mode,
	bool constrain_ise_weight_selectors,
	int32_t first_submode, int32_t last_submode, bool ignore_clamping, opt_mode_t opt_mode,
	const encode_astc_block_stats* pBlock_stats)
{
	assert((ise_weight_range >= MIN_SUPPORTED_ISE_WEIGHT_INDEX) && (ise_weight_range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX));
	assert((ise_endpoint_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (ise_endpoint_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
	assert((num_pixels >= 1) && (num_pixels <= MAX_ASTC_HDR_ENC_BLOCK_PIXELS));

	assert((first_submode >= FIRST_MODE11_SUBMODE_INDEX) && (first_submode <= last_submode));
	assert(last_submode <= MAX_MODE11_SUBMODE_INDEX);

	best_submode = 0;

	const uint32_t num_weight_levels = astc_helpers::get_ise_levels(ise_weight_range);
	assert(num_weight_levels <= MAX_SUPPORTED_WEIGHT_LEVELS);

	vec3F block_mean_color_q16, block_axis_q16;
	if (!pBlock_stats)
	{
		block_mean_color_q16 = calc_mean(num_pixels, pBlock_pixels_q16);
		block_axis_q16 = calc_rgb_pca(num_pixels, pBlock_pixels_q16, block_mean_color_q16);
	}
	else
	{
		assert(num_pixels == pBlock_stats->m_num_pixels);
		block_mean_color_q16 = pBlock_stats->m_mean_q16;
		block_axis_q16 = pBlock_stats->m_axis_q16;
	}

	aabb3F color_box_q16(cInitExpand);

	float l = BIG_FLOAT_VAL, h = -BIG_FLOAT_VAL;
	vec3F low_color_q16, high_color_q16;
	low_color_q16.clear();
	high_color_q16.clear();

	for (uint32_t i = 0; i < num_pixels; i++)
	{
		color_box_q16.expand(pBlock_pixels_q16[i]);

		vec3F k(vec3F(pBlock_pixels_q16[i]) - block_mean_color_q16);
		float kd = k.dot(block_axis_q16);

		if (kd < l)
		{
			l = kd;
			low_color_q16 = pBlock_pixels_q16[i];
		}

		if (kd > h)
		{
			h = kd;
			high_color_q16 = pBlock_pixels_q16[i];
		}
	}
		
	vec3F old_low_color_q16(low_color_q16), old_high_color_q16(high_color_q16);
	
	for (uint32_t i = 0; i < 3; i++)
	{
		low_color_q16[i] = lerp<float>(old_low_color_q16[i], old_high_color_q16[i], 1.0f / 64.0f);
		high_color_q16[i] = lerp<float>(old_low_color_q16[i], old_high_color_q16[i], 63.0f / 64.0f);
	}

	uint8_t trial_blk_endpoints[NUM_MODE11_ENDPOINTS];
	uint8_t trial_blk_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
	uint32_t trial_best_submode = 0;

	clear_obj(trial_blk_endpoints);
	clear_obj(trial_blk_weights);

	double trial_blk_error = BIG_FLOAT_VAL;
			
	bool did_improve = try_mode11(num_pixels, trial_blk_endpoints, trial_blk_weights, trial_blk_error, trial_best_submode,
		low_color_q16, high_color_q16,
		pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, direct_only, ise_endpoint_range, constrain_ise_weight_selectors,
		first_submode, last_submode, ignore_clamping);

	// If we couldn't find ANY usable solution due to endpoint quantization, just return. There's nothing we can do.
	if (!did_improve)
		return cur_block_error;

	// Did the solution improve?
	if (trial_blk_error < cur_block_error)
	{
		cur_block_error = trial_blk_error;
		memcpy(blk_endpoints, trial_blk_endpoints, NUM_MODE11_ENDPOINTS);
		memcpy(blk_weights, trial_blk_weights, num_pixels);
		best_submode = trial_best_submode;
	}

	if (opt_mode == cNoOpt)
		return cur_block_error;

	// least squares on the most promising trial weight indices found
	const uint32_t NUM_LS_PASSES = 3;

	float emphasis_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];

	if (opt_mode == cWeightedAverage)
	{
		const uint32_t NUM_OPT_PASSES = 3;
		for (uint32_t pass = 0; pass < NUM_OPT_PASSES; pass++)
		{
			vec3F low_p(0.0f);
			float total_low = 0.0f;

			vec3F high_p(0.0f);
			float total_high = 0.0f;

			for (uint32_t i = 0; i < num_pixels; i++)
			{
				vec3F p(pBlock_pixels_q16[i]);
				float lerp = g_ise_weight_lerps[ise_weight_range][trial_blk_weights[i] + 1] * (1.0f / 64.0f);

				low_p += p * (1.0f - lerp);
				total_low += (1.0f - lerp);

				high_p += p * lerp;
				total_high += lerp;
			}

			if (total_low != 0.0f)
				low_p *= (1.0f / total_low);

			if (total_high != 0.0f)
				high_p *= (1.0f / total_high);

			vec3F low, high;

			bool was_improved = try_mode11(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
				low_p, high_p,
				pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, direct_only, ise_endpoint_range, constrain_ise_weight_selectors,
				first_submode, last_submode, ignore_clamping);

			if (!was_improved)
				break;

			memcpy(trial_blk_weights, blk_weights, num_pixels);
		}
	}
	else if (opt_mode == cOrdinaryLeastSquares)
	{
		for (uint32_t pass = 0; pass < NUM_LS_PASSES; pass++)
		{
			vec3F l_q16, h_q16;

			if (!compute_least_squares_endpoints_rgb(num_pixels, trial_blk_weights, &g_astc_ls_weights_ise[ise_weight_range][0], &l_q16, &h_q16, pBlock_pixels_q16, color_box_q16))
				break;
			
			bool was_improved = try_mode11(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
				l_q16, h_q16,
				pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, direct_only, ise_endpoint_range, constrain_ise_weight_selectors,
				first_submode, last_submode, ignore_clamping);

			if (!was_improved)
				break;

			// It's improved, so let's take the new weight indices.
			memcpy(trial_blk_weights, blk_weights, num_pixels);

		} // pass
	}
	else
	{
		if (h == l)
		{
			for (uint32_t i = 0; i < num_pixels; i++)
				emphasis_weights[i] = 1.0f;
		}
		else
		{
			float mid = (0.0f - l) / (h - l);
			mid = clamp(mid, .01f, .99f);

			float lw = LOW_EMPHASIS_WEIGHT, mw = MIDDLE_EMPHASIS_WEIGHT, hw = HIGH_EMPHASIS_WEIGHT;
			if (opt_mode == cWeightedLeastSquaresHeavy)
				lw = LOW_EMPHASIS_WEIGHT_HEAVY, mw = MIDDLE_EMPHASIS_WEIGHT_HEAVY, hw = HIGH_EMPHASIS_WEIGHT_HEAVY;
						
			for (uint32_t i = 0; i < num_pixels; i++)
			{
				vec3F k(vec3F(pBlock_pixels_q16[i]) - block_mean_color_q16);
				float kd = k.dot(block_axis_q16);

				assert((kd >= l) && (kd <= h));

				float v = (kd - l) / (h - l);
										
				if (v < mid)
					v = lerp(lw, mw, v / mid);
				else
					v = lerp(mw, hw, (v - mid) * (1.0f - mid));

				emphasis_weights[i] = v;
			}

#if 0
			if (num_pixels == 6 * 6)
			{
				const float EDGE_WEIGHT = .1f;
				for (uint32_t i = 0; i < 6; i++)
				{
					emphasis_weights[i] += EDGE_WEIGHT;
					emphasis_weights[i + 5 * 6] += EDGE_WEIGHT;
					emphasis_weights[i * 6] += EDGE_WEIGHT;
					emphasis_weights[5 + i * 6] += EDGE_WEIGHT;
				}
			}
#endif
		}

		for (uint32_t pass = 0; pass < NUM_LS_PASSES; pass++)
		{
			vec3F l_q16, h_q16;

			if (!compute_weighted_least_squares_endpoints_rgb(
				num_pixels,
				trial_blk_weights, &g_astc_ls_weights_ise[ise_weight_range][0], nullptr,
				emphasis_weights,
				&l_q16, &h_q16,
				pBlock_pixels_q16,
				color_box_q16))
				break;

			bool was_improved = try_mode11(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
				l_q16, h_q16,
				pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, direct_only, ise_endpoint_range, constrain_ise_weight_selectors,
				first_submode, last_submode, ignore_clamping);

			if (!was_improved)
				break;

			// It's improved, so let's take the new weight indices.
			memcpy(trial_blk_weights, blk_weights, num_pixels);

		} // pass
	}

	if ( (uber_mode) && (ise_weight_range >= astc_helpers::BISE_3_LEVELS) &&
		((opt_mode == cOrdinaryLeastSquares) || (opt_mode == cWeightedLeastSquares) || (opt_mode == cWeightedLeastSquaresHeavy)) )
	{
		// Try varying the current best weight indices. This can be expanded/improved, but at potentially great cost.

		uint8_t temp_astc_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
		memcpy(temp_astc_weights, trial_blk_weights, num_pixels);

		uint32_t min_lin_sel = 256, max_lin_sel = 0;
		for (uint32_t i = 0; i < num_pixels; i++)
		{
			const uint32_t astc_sel = temp_astc_weights[i];

			const uint32_t lin_sel = g_map_astc_to_linear_order[ise_weight_range][astc_sel];
			assert(lin_sel < num_weight_levels);

			min_lin_sel = minimumu(min_lin_sel, lin_sel);
			max_lin_sel = maximumu(max_lin_sel, lin_sel);
		}

		bool was_improved = false;
		(void)was_improved;

		{
			bool weights_changed = false;
			uint8_t trial_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
			for (uint32_t i = 0; i < num_pixels; i++)
			{
				uint32_t astc_sel = temp_astc_weights[i];
				uint32_t lin_sel = g_map_astc_to_linear_order[ise_weight_range][astc_sel];

				if ((lin_sel == min_lin_sel) && (lin_sel < (num_weight_levels - 1)))
				{
					lin_sel++;
					weights_changed = true;
				}

				trial_weights[i] = g_map_linear_to_astc_order[ise_weight_range][lin_sel];
			}

			if (weights_changed)
			{
				vec3F l_q16, h_q16;

				bool succeeded;
				if (opt_mode == cOrdinaryLeastSquares)
					succeeded = compute_least_squares_endpoints_rgb(num_pixels, trial_weights, &g_astc_ls_weights_ise[ise_weight_range][0], &l_q16, &h_q16, pBlock_pixels_q16, color_box_q16);
				else
					succeeded = compute_weighted_least_squares_endpoints_rgb(num_pixels, trial_weights, &g_astc_ls_weights_ise[ise_weight_range][0], nullptr, emphasis_weights, &l_q16, &h_q16, pBlock_pixels_q16, color_box_q16);

				if (succeeded)
				{
					if (try_mode11(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
						l_q16, h_q16,
						pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, direct_only, ise_endpoint_range, constrain_ise_weight_selectors,
						first_submode, last_submode, ignore_clamping))
					{
						was_improved = true;
					}
				}
			}
		}

		{
			bool weights_changed = false;
			uint8_t trial_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];

			for (uint32_t i = 0; i < num_pixels; i++)
			{
				uint32_t astc_sel = temp_astc_weights[i];
				uint32_t lin_sel = g_map_astc_to_linear_order[ise_weight_range][astc_sel];

				if ((lin_sel == max_lin_sel) && (lin_sel > 0))
				{
					lin_sel--;
					weights_changed = true;
				}

				trial_weights[i] = g_map_linear_to_astc_order[ise_weight_range][lin_sel];
			}

			if (weights_changed)
			{
				vec3F l_q16, h_q16;

				bool succeeded;
				if (opt_mode == cOrdinaryLeastSquares)
					succeeded = compute_least_squares_endpoints_rgb(num_pixels, trial_weights, &g_astc_ls_weights_ise[ise_weight_range][0], &l_q16, &h_q16, pBlock_pixels_q16, color_box_q16);
				else
					succeeded = compute_weighted_least_squares_endpoints_rgb(num_pixels, trial_weights, &g_astc_ls_weights_ise[ise_weight_range][0], nullptr, emphasis_weights, &l_q16, &h_q16, pBlock_pixels_q16, color_box_q16);

				if (succeeded)
				{
					if (try_mode11(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
						l_q16, h_q16,
						pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, direct_only, ise_endpoint_range, constrain_ise_weight_selectors,
						first_submode, last_submode, ignore_clamping))
					{
						was_improved = true;
					}
				}
			}
		}

		{
			bool weights_changed = false;
			uint8_t trial_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
			for (uint32_t i = 0; i < num_pixels; i++)
			{
				uint32_t astc_sel = temp_astc_weights[i];
				uint32_t lin_sel = g_map_astc_to_linear_order[ise_weight_range][astc_sel];

				if ((lin_sel == max_lin_sel) && (lin_sel > 0))
				{
					lin_sel--;
					weights_changed = true;
				}
				else if ((lin_sel == min_lin_sel) && (lin_sel < (num_weight_levels - 1)))
				{
					lin_sel++;
					weights_changed = true;
				}

				trial_weights[i] = g_map_linear_to_astc_order[ise_weight_range][lin_sel];
			}

			if (weights_changed)
			{
				vec3F l_q16, h_q16;
				bool succeeded;
				if (opt_mode == cOrdinaryLeastSquares)
					succeeded = compute_least_squares_endpoints_rgb(num_pixels, trial_weights, &g_astc_ls_weights_ise[ise_weight_range][0], &l_q16, &h_q16, pBlock_pixels_q16, color_box_q16);
				else
					succeeded = compute_weighted_least_squares_endpoints_rgb(num_pixels, trial_weights, &g_astc_ls_weights_ise[ise_weight_range][0], nullptr, emphasis_weights, &l_q16, &h_q16, pBlock_pixels_q16, color_box_q16);

				if (succeeded)
				{
					if (try_mode11(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
						l_q16, h_q16,
						pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, direct_only, ise_endpoint_range, constrain_ise_weight_selectors,
						first_submode, last_submode, ignore_clamping))
					{
						was_improved = true;
					}
				}
			}
		}

	} // uber_mode

	return cur_block_error;
}

//--------------------------------------------------------------------------------------------------------------------------

double encode_astc_hdr_block_downsampled_mode_11(
	uint32_t block_x, uint32_t block_y, uint32_t grid_x, uint32_t grid_y,
	uint32_t ise_weight_range, uint32_t ise_endpoint_range,
	uint32_t num_pixels, const basist::half_float pBlock_pixels_half[][3], const vec4F pBlock_pixels_q16[],
	double cur_block_error,
	int32_t first_submode, int32_t last_submode, bool ignore_clamping, opt_mode_t opt_mode,
	uint8_t* pBlk_endpoints, uint8_t* pBlk_weights, uint32_t& best_submode,
	const astc_hdr_codec_base_options& coptions,
	const encode_astc_block_stats* pBlock_stats)
{
	assert((block_x >= 4) && (block_y >= 4) && (block_x <= MAX_ASTC_HDR_BLOCK_W) && (block_y <= MAX_ASTC_HDR_BLOCK_H));
	assert((grid_x >= 2) && (grid_y >= 2) && (grid_x <= block_x) && (grid_y <= block_y));

	assert((ise_weight_range >= MIN_SUPPORTED_ISE_WEIGHT_INDEX) && (ise_weight_range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX));
	assert((ise_endpoint_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (ise_endpoint_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
	assert((num_pixels >= 1) && (num_pixels <= MAX_ASTC_HDR_ENC_BLOCK_PIXELS));

	assert((first_submode >= FIRST_MODE11_SUBMODE_INDEX) && (first_submode <= last_submode));
	assert(last_submode <= MAX_MODE11_SUBMODE_INDEX);

	best_submode = 0;

	assert(astc_helpers::get_ise_levels(ise_weight_range) <= MAX_SUPPORTED_WEIGHT_LEVELS);

	const uint32_t num_weights = grid_x * grid_y;

	vec3F block_mean_color_q16, block_axis_q16;
	if (!pBlock_stats)
	{
		block_mean_color_q16 = calc_mean(num_pixels, pBlock_pixels_q16);
		block_axis_q16 = calc_rgb_pca(num_pixels, pBlock_pixels_q16, block_mean_color_q16);
	}
	else
	{
		assert(num_pixels == pBlock_stats->m_num_pixels);
		block_mean_color_q16 = pBlock_stats->m_mean_q16;
		block_axis_q16 = pBlock_stats->m_axis_q16;
	}

	aabb3F color_box_q16(cInitExpand);

	float l = BIG_FLOAT_VAL, h = -BIG_FLOAT_VAL;
	vec3F low_color_q16, high_color_q16;
	low_color_q16.clear();
	high_color_q16.clear();

	for (uint32_t i = 0; i < num_pixels; i++)
	{
		color_box_q16.expand(pBlock_pixels_q16[i]);

		vec3F k(vec3F(pBlock_pixels_q16[i]) - block_mean_color_q16);
		float kd = k.dot(block_axis_q16);

		if (kd < l)
		{
			l = kd;
			low_color_q16 = pBlock_pixels_q16[i];
		}

		if (kd > h)
		{
			h = kd;
			high_color_q16 = pBlock_pixels_q16[i];
		}
	}

	vec3F old_low_color_q16(low_color_q16), old_high_color_q16(high_color_q16);

	for (uint32_t i = 0; i < 3; i++)
	{
		low_color_q16[i] = lerp<float>(old_low_color_q16[i], old_high_color_q16[i], 1.0f / 64.0f);
		high_color_q16[i] = lerp<float>(old_low_color_q16[i], old_high_color_q16[i], 63.0f / 64.0f);
	}

	const uint32_t NUM_PASSES = 3;
	for (uint32_t pass = 0; pass < NUM_PASSES; pass++)
	{
		uint8_t trial_blk_endpoints[NUM_MODE11_ENDPOINTS];
		uint8_t trial_blk_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS]; // at block resolution, not grid res
		uint32_t trial_best_submode = 0;

		clear_obj(trial_blk_endpoints);
		clear_obj(trial_blk_weights);
				
		double trial_blk_error = BIG_FLOAT_VAL;

		bool could_pack = try_mode11(num_pixels, trial_blk_endpoints, trial_blk_weights, trial_blk_error, trial_best_submode,
			low_color_q16, high_color_q16,
			pBlock_pixels_half, 32, astc_helpers::BISE_32_LEVELS, coptions, false, ise_endpoint_range, false,
			first_submode, last_submode, ignore_clamping);

		if (!could_pack)
			break;

		uint8_t trial_downsampled_ise_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];

		downsample_ise_weights(
			astc_helpers::BISE_32_LEVELS, ise_weight_range,
			block_x, block_y, grid_x, grid_y,
			trial_blk_weights, trial_downsampled_ise_weights);

		uint8_t trial_downsampled_raw_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
		dequantize_astc_weights(num_weights, trial_downsampled_ise_weights, ise_weight_range, trial_downsampled_raw_weights);

		uint8_t trial_upsampled_raw_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS]; // raw weights, NOT ISE
		astc_helpers::upsample_weight_grid(block_x, block_y, grid_x, grid_y, trial_downsampled_raw_weights, trial_upsampled_raw_weights);

		//------

		int trial_e[2][3];
		if (!decode_mode11_to_qlog12(trial_blk_endpoints, trial_e, ise_endpoint_range))
			return cur_block_error;

		double trial_error = compute_block_error_from_raw_weights(num_pixels, pBlock_pixels_half, trial_upsampled_raw_weights, trial_e, coptions);

		if (trial_error < cur_block_error)
		{
			cur_block_error = trial_error;
			memcpy(pBlk_endpoints, trial_blk_endpoints, NUM_MODE11_ENDPOINTS);
			memcpy(pBlk_weights, trial_downsampled_ise_weights, num_weights);
			best_submode = trial_best_submode;
		}
		else if (pass)
			break;
						
		if ((opt_mode == cWeightedLeastSquares) || (opt_mode == cWeightedLeastSquaresHeavy))
		{
			float emphasis_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
			if (h == l)
			{
				for (uint32_t i = 0; i < num_pixels; i++)
					emphasis_weights[i] = 1.0f;
			}
			else
			{
				float mid = (0.0f - l) / (h - l);
				mid = clamp(mid, .01f, .99f);

				float lw = LOW_EMPHASIS_WEIGHT, mw = MIDDLE_EMPHASIS_WEIGHT, hw = HIGH_EMPHASIS_WEIGHT;
				if (opt_mode == cWeightedLeastSquaresHeavy)
					lw = LOW_EMPHASIS_WEIGHT_HEAVY, mw = MIDDLE_EMPHASIS_WEIGHT_HEAVY, hw = HIGH_EMPHASIS_WEIGHT_HEAVY;

				for (uint32_t i = 0; i < num_pixels; i++)
				{
					vec3F k(vec3F(pBlock_pixels_q16[i]) - block_mean_color_q16);
					float kd = k.dot(block_axis_q16);

					assert((kd >= l) && (kd <= h));

					float v = (kd - l) / (h - l);

					if (v < mid)
						v = lerp(lw, mw, v / mid);
					else
						v = lerp(mw, hw, (v - mid) * (1.0f - mid));

					emphasis_weights[i] = v;
				}
			}

			float trial_upsampled_raw_weightsf[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
			for (uint32_t i = 0; i < num_pixels; i++)
				trial_upsampled_raw_weightsf[i] = (float)trial_upsampled_raw_weights[i] * (1.0f / 64.0f);

			if (!compute_weighted_least_squares_endpoints_rgb(num_pixels, nullptr, nullptr, trial_upsampled_raw_weightsf, emphasis_weights, &low_color_q16, &high_color_q16, pBlock_pixels_q16, color_box_q16))
				return false;
		}
		else
		{
			if (!compute_least_squares_endpoints_rgb_raw_weights(num_pixels, trial_upsampled_raw_weights, &low_color_q16, &high_color_q16, pBlock_pixels_q16, color_box_q16))
				break;
		}

		bool pack_succeeded = pack_mode11(low_color_q16, high_color_q16, ise_endpoint_range, trial_blk_endpoints, coptions, false, first_submode, last_submode, false, trial_best_submode);
		if (!pack_succeeded)
			break;

		if (!decode_mode11_to_qlog12(trial_blk_endpoints, trial_e, ise_endpoint_range))
			break;

		trial_error = compute_block_error_from_raw_weights(num_pixels, pBlock_pixels_half, trial_upsampled_raw_weights, trial_e, coptions);

		if (trial_error < cur_block_error)
		{
			cur_block_error = trial_error;
			memcpy(pBlk_endpoints, trial_blk_endpoints, NUM_MODE11_ENDPOINTS);
			memcpy(pBlk_weights, trial_downsampled_ise_weights, num_weights);
			best_submode = trial_best_submode;
		}
		else
		{
			break;
		}

    } // pass

	return cur_block_error;
}

//--------------------------------------------------------------------------------------------------------------------------

double encode_astc_hdr_block_mode_11_dual_plane(
	uint32_t num_pixels,
	const basist::half_float pBlock_pixels_half[][3], const vec4F pBlock_pixels_q16[],
	uint32_t channel_index,		// 0-2
	uint32_t ise_weight_range,
	uint32_t& best_submode,
	double cur_block_error,
	uint8_t* blk_endpoints, uint8_t* blk_weights0, uint8_t* blk_weights1,
	const astc_hdr_codec_base_options& coptions,
	bool direct_only,
	uint32_t ise_endpoint_range,
	bool uber_mode,
	bool constrain_ise_weight_selectors,
	int32_t first_submode, int32_t last_submode, bool ignore_clamping)
{
	(void)uber_mode;

	assert(channel_index <= 2);
	assert((ise_weight_range >= MIN_SUPPORTED_ISE_WEIGHT_INDEX) && (ise_weight_range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX));
	assert((ise_endpoint_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (ise_endpoint_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));
	assert((num_pixels >= 1) && (num_pixels <= MAX_ASTC_HDR_ENC_BLOCK_PIXELS));

	assert((first_submode >= FIRST_MODE11_SUBMODE_INDEX) && (first_submode <= last_submode));
	assert(last_submode <= MAX_MODE11_SUBMODE_INDEX);
	
	assert(num_pixels <= MAX_ASTC_HDR_ENC_BLOCK_PIXELS);

	best_submode = 0;

	const uint32_t num_weight_levels = astc_helpers::get_ise_levels(ise_weight_range);
	assert(num_weight_levels <= MAX_SUPPORTED_WEIGHT_LEVELS);

	vec4F temp_block_pixels_q16[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
	for (uint32_t i = 0; i < num_pixels; i++)
	{
		temp_block_pixels_q16[i] = pBlock_pixels_q16[i];
		temp_block_pixels_q16[i][channel_index] = 0.0f;
	}

	vec3F block_mean_color_q16(calc_mean(num_pixels, temp_block_pixels_q16));
	vec3F block_axis_q16(calc_rgb_pca(num_pixels, temp_block_pixels_q16, block_mean_color_q16));

	float l = BIG_FLOAT_VAL, h = -BIG_FLOAT_VAL;
	vec3F low_color_q16, high_color_q16;

	aabb3F color_box_q16(cInitExpand);

	for (uint32_t i = 0; i < num_pixels; i++)
	{
		color_box_q16.expand(pBlock_pixels_q16[i]);

		vec3F k(vec3F(temp_block_pixels_q16[i]) - block_mean_color_q16);
		float kd = k.dot(block_axis_q16);

		if (kd < l)
		{
			l = kd;
			low_color_q16 = pBlock_pixels_q16[i];
		}

		if (kd > h)
		{
			h = kd;
			high_color_q16 = pBlock_pixels_q16[i];
		}
	}

	low_color_q16[channel_index] = 0.0f;
	high_color_q16[channel_index] = 0.0f;

	float a = low_color_q16.dot(vec3F(1.0f)), b = high_color_q16.dot(vec3F(1.0f));
	if (a <= b)
	{
		low_color_q16[channel_index] = color_box_q16.get_low()[channel_index];
		high_color_q16[channel_index] = color_box_q16.get_high()[channel_index];
	}
	else
	{
		high_color_q16[channel_index] = color_box_q16.get_low()[channel_index];
		low_color_q16[channel_index] = color_box_q16.get_high()[channel_index];
	}

	vec3F old_low_color_q16(low_color_q16), old_high_color_q16(high_color_q16);
	for (uint32_t i = 0; i < 3; i++)
	{
		low_color_q16[i] = lerp<float>(old_low_color_q16[i], old_high_color_q16[i], 1.0f / 64.0f);
		high_color_q16[i] = lerp<float>(old_low_color_q16[i], old_high_color_q16[i], 63.0f / 64.0f);
	}

	uint8_t trial_blk_endpoints[NUM_MODE11_ENDPOINTS];
	uint8_t trial_blk_weights0[MAX_ASTC_HDR_ENC_BLOCK_PIXELS], trial_blk_weights1[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
	uint32_t trial_best_submode = 0;

	clear_obj(trial_blk_endpoints);
	clear_obj(trial_blk_weights0);
	clear_obj(trial_blk_weights1);

	double trial_blk_error = BIG_FLOAT_VAL;

	bool did_improve = try_mode11_dual_plane(channel_index, num_pixels, trial_blk_endpoints, trial_blk_weights0, trial_blk_weights1, trial_blk_error, trial_best_submode,
		low_color_q16, high_color_q16, 
		pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, direct_only, ise_endpoint_range, constrain_ise_weight_selectors,
		first_submode, last_submode, ignore_clamping);

	// If we couldn't find ANY usable solution due to endpoint quantization, just return. There's nothing we can do.
	if (!did_improve)
		return cur_block_error;

	// Did the solution improve?
	if (trial_blk_error < cur_block_error)
	{
		cur_block_error = trial_blk_error;
		memcpy(blk_endpoints, trial_blk_endpoints, NUM_MODE11_ENDPOINTS);
		memcpy(blk_weights0, trial_blk_weights0, num_pixels);
		memcpy(blk_weights1, trial_blk_weights1, num_pixels);
		best_submode = trial_best_submode;
	}

	const uint32_t chan0 = (channel_index + 1) % 3, chan1 = (channel_index + 2) % 3;

	vec2F plane0_q16[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
	aabb2F plane0_bounds;
	plane0_bounds[0].set(color_box_q16.get_low()[chan0], color_box_q16.get_low()[chan1]);
	plane0_bounds[1].set(color_box_q16.get_high()[chan0], color_box_q16.get_high()[chan1]);

	vec1F plane1_q16[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
	aabb1F plane1_bounds;
	plane1_bounds[0].set(color_box_q16.get_low()[channel_index]);
	plane1_bounds[1].set(color_box_q16.get_high()[channel_index]);

	for (uint32_t i = 0; i < num_pixels; i++)
	{
		plane0_q16[i][0] = pBlock_pixels_q16[i][chan0];
		plane0_q16[i][1] = pBlock_pixels_q16[i][chan1];

		plane1_q16[i][0] = pBlock_pixels_q16[i][channel_index];
	}

	const uint32_t NUM_LS_PASSES = 3;

	for (uint32_t pass = 0; pass < NUM_LS_PASSES; pass++)
	{
		vec2F l0_q16, h0_q16;
		if (!compute_least_squares_endpoints_2D(num_pixels, trial_blk_weights0, &g_astc_ls_weights_ise[ise_weight_range][0], &l0_q16, &h0_q16, plane0_q16, plane0_bounds))
			break;

		vec1F l1_q16, h1_q16;
		if (!compute_least_squares_endpoints_1D(num_pixels, trial_blk_weights1, &g_astc_ls_weights_ise[ise_weight_range][0], &l1_q16, &h1_q16, plane1_q16, plane1_bounds))
			break;

		vec3F l_q16, h_q16;

		l_q16[channel_index] = l1_q16[0];
		h_q16[channel_index] = h1_q16[0];

		l_q16[chan0] = l0_q16[0];
		h_q16[chan0] = h0_q16[0];

		l_q16[chan1] = l0_q16[1];
		h_q16[chan1] = h0_q16[1];

		bool was_improved = try_mode11_dual_plane(channel_index, num_pixels, blk_endpoints, blk_weights0, blk_weights1, cur_block_error, best_submode,
			l_q16, h_q16,
			pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, direct_only, ise_endpoint_range, constrain_ise_weight_selectors,
			first_submode, last_submode, ignore_clamping);

		if (!was_improved)
			break;

		// It's improved, so let's take the new weight indices.
		memcpy(trial_blk_weights0, blk_weights0, num_pixels);
		memcpy(trial_blk_weights1, blk_weights1, num_pixels);

	} // pass
	
	return cur_block_error;
}

//--------------------------------------------------------------------------------------------------------------------------

double encode_astc_hdr_block_mode_7(
	uint32_t num_pixels,
	const basist::half_float pBlock_pixels_half[][3], const vec4F pBlock_pixels_q16[],
	uint32_t ise_weight_range,
	uint32_t& best_submode,
	double cur_block_error,
	uint8_t* blk_endpoints,  //[4]
	uint8_t* blk_weights, // [num_pixels]
	const astc_hdr_codec_base_options& coptions,
	uint32_t ise_endpoint_range, 
	int first_submode, int last_submode,
	const encode_astc_block_stats* pBlock_stats)
{
	assert((num_pixels >= 1) && (num_pixels <= MAX_ASTC_HDR_ENC_BLOCK_PIXELS));
	assert((ise_weight_range >= MIN_SUPPORTED_ISE_WEIGHT_INDEX) && (ise_weight_range <= MAX_SUPPORTED_ISE_WEIGHT_INDEX));
	assert((ise_endpoint_range >= astc_helpers::FIRST_VALID_ENDPOINT_ISE_RANGE) && (ise_endpoint_range <= astc_helpers::LAST_VALID_ENDPOINT_ISE_RANGE));

	const uint32_t num_weight_levels = astc_helpers::get_ise_levels(ise_weight_range);
	assert(num_weight_levels <= MAX_SUPPORTED_WEIGHT_LEVELS);

	best_submode = 0;

	vec3F block_mean_color_q16;
	if (!pBlock_stats)
		block_mean_color_q16 = calc_mean(num_pixels, pBlock_pixels_q16);
	else
	{
		assert(num_pixels == pBlock_stats->m_num_pixels);
		block_mean_color_q16 = pBlock_stats->m_mean_q16;
	}

	vec3F block_axis_q16(0.577350259f);

	aabb3F color_box_q16(cInitExpand);

	float l = BIG_FLOAT_VAL, h = -BIG_FLOAT_VAL;
	for (uint32_t i = 0; i < num_pixels; i++)
	{
		color_box_q16.expand(pBlock_pixels_q16[i]);

		vec3F k(vec3F(pBlock_pixels_q16[i]) - block_mean_color_q16);
		float kd = k.dot(block_axis_q16);

		l = basisu::minimum<float>(l, kd);
		h = basisu::maximum<float>(h, kd);
	}

	vec3F low_color_q16(interp_color(block_mean_color_q16, block_axis_q16, l, color_box_q16, color_box_q16));
	vec3F high_color_q16(interp_color(block_mean_color_q16, block_axis_q16, h, color_box_q16, color_box_q16));

	low_color_q16.clamp(0.0f, MAX_QLOG16_VAL);
	high_color_q16.clamp(0.0f, MAX_QLOG16_VAL);

	vec3F diff(high_color_q16 - low_color_q16);

	// The mul here (* block_axis_q16[0]) is because the "S" or scale value is subtracted from the high color with a scale of 1.0, 
	// i.e. it's equivalent to a vector of (1,1,1) multiplied by scale before the sub. We want to actually move along the grayscale axis, or (0.577350259, 0.577350259, 0.577350259).
	float s_q16 = diff.dot(block_axis_q16) * block_axis_q16[0];

	uint8_t trial_blk_endpoints[NUM_MODE7_ENDPOINTS];
	uint8_t trial_blk_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
	uint32_t trial_best_submode = 0;

	clear_obj(trial_blk_endpoints);
	clear_obj(trial_blk_weights);

	double trial_blk_error = BIG_FLOAT_VAL;

	bool did_improve = try_mode7(num_pixels, trial_blk_endpoints, trial_blk_weights, trial_blk_error, trial_best_submode,
		high_color_q16, ceilf(s_q16),
		pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, ise_endpoint_range, first_submode, last_submode);

	// If we couldn't find ANY usable solution due to endpoint quantization, just return. There's nothing we can do.
	if (!did_improve)
	{
		return cur_block_error;
	}

	// Did the solution improve?
	if (trial_blk_error < cur_block_error)
	{
		cur_block_error = trial_blk_error;
		memcpy(blk_endpoints, trial_blk_endpoints, NUM_MODE7_ENDPOINTS);
		memcpy(blk_weights, trial_blk_weights, num_pixels);
		best_submode = trial_best_submode;
	}

#if 1
	{
		//const float TL = 8830.0f;// (float)half_to_qlog16(float_to_half(0.00061f));
		//const float TH = 41600.0f;// (float)half_to_qlog16(float_to_half(40.0f));
		//float zl = minimum<float>(color_box_q16[0][0], color_box_q16[0][1], color_box_q16[0][2]);
		//float zh = minimum<float>(color_box_q16[1][0], color_box_q16[1][1], color_box_q16[1][2]);

		//if ((zl <= TL) && (zh >= TH))
		{
			// Try a simpler technique for artifact reduction
			l = BIG_FLOAT_VAL;
			h = -BIG_FLOAT_VAL;

			vec3F alt_low_color_q16(0.0f), alt_high_color_q16(0.0f);
			for (uint32_t i = 0; i < num_pixels; i++)
			{
				color_box_q16.expand(pBlock_pixels_q16[i]);

				vec3F k(vec3F(pBlock_pixels_q16[i]) - block_mean_color_q16);
				float kd = k.dot(block_axis_q16);

				if (kd < l)
				{
					alt_low_color_q16 = pBlock_pixels_q16[i];
					l = kd;
				}

				if (kd > h)
				{
					alt_high_color_q16 = pBlock_pixels_q16[i];
					h = kd;
				}
			}

			vec3F old_alt_low_color_q16(alt_low_color_q16);

			for (uint32_t i = 0; i < 3; i++)
				alt_low_color_q16[i] = lerp<float>(old_alt_low_color_q16[i], alt_high_color_q16[i], 1.0f / 64.0f);

			vec3F alt_diff(alt_high_color_q16 - alt_low_color_q16);

			// The mul here (* block_axis_q16[0]) is because the "S" or scale value is subtracted from the high color with a scale of 1.0, 
			// i.e. it's equivalent to a vector of (1,1,1) multiplied by scale before the sub. We want to actually move along the grayscale axis, or (0.577350259, 0.577350259, 0.577350259).
			float alt_s_q16 = alt_diff.dot(block_axis_q16) * block_axis_q16[0];

			try_mode7(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
				alt_high_color_q16, ceilf(alt_s_q16),
				pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, ise_endpoint_range, first_submode, last_submode);
		}
	}
#endif

	const float one_over_num_pixels = 1.0f / (float)num_pixels;

	const uint32_t NUM_TRIALS = 2;
	for (uint32_t trial = 0; trial < NUM_TRIALS; trial++)
	{
		// Given a set of selectors and S, try to compute a better high color
		vec3F new_high_color_q16(block_mean_color_q16);

		int e[2][3];
		int cur_s = 0;
		if (!decode_mode7_to_qlog12(trial_blk_endpoints, e, &cur_s, ise_endpoint_range))
			break;

		cur_s <<= 4;

		for (uint32_t i = 0; i < num_pixels; i++)
		{
			uint32_t astc_sel = trial_blk_weights[i];
			float lerp = g_ise_weight_lerps[ise_weight_range][astc_sel + 1] * (1.0f / 64.0f);

			float k = (float)cur_s * (1.0f - lerp) * one_over_num_pixels;
			new_high_color_q16[0] += k;
			new_high_color_q16[1] += k;
			new_high_color_q16[2] += k;
		}

		bool improved = try_mode7(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
			new_high_color_q16, (float)cur_s,
			pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, ise_endpoint_range, first_submode, last_submode);

		if (improved)
		{
			memcpy(trial_blk_endpoints, blk_endpoints, NUM_MODE7_ENDPOINTS);
			memcpy(trial_blk_weights, blk_weights, num_pixels);
		}

		// Given a set of selectors and a high color, try to compute a better S.
		float t = 0.0f;

		for (uint32_t i = 0; i < num_pixels; i++)
		{
			uint32_t astc_sel = trial_blk_weights[i];
			float lerp = g_ise_weight_lerps[ise_weight_range][astc_sel + 1] * (1.0f / 64.0f);

			t += (1.0f) - lerp;
		}

		t *= one_over_num_pixels;

		//int e[2][3];
		if (!decode_mode7_to_qlog12(trial_blk_endpoints, e, nullptr, ise_endpoint_range))
			break;

		vec3F cur_h_q16((float)(e[1][0] << 4), (float)(e[1][1] << 4), (float)(e[1][2] << 4));

		if (fabs(t) > .0000125f)
		{
			float s_r = (cur_h_q16[0] - block_mean_color_q16[0]) / t;
			float s_g = (cur_h_q16[1] - block_mean_color_q16[1]) / t;
			float s_b = (cur_h_q16[2] - block_mean_color_q16[2]) / t;

			// TODO: gather statistics on these
			if (try_mode7(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
				cur_h_q16, ceilf(s_r),
				pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, ise_endpoint_range, first_submode, last_submode))
			{
				improved = true;
			}

			if (coptions.m_mode7_full_s_optimization)
			{
				if (try_mode7(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
					cur_h_q16, ceilf(s_g),
					pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, ise_endpoint_range, first_submode, last_submode))
				{
					improved = true;
				}

				if (try_mode7(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
					cur_h_q16, ceilf(s_b),
					pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, ise_endpoint_range, first_submode, last_submode))
				{
					improved = true;
				}

				if (try_mode7(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
					cur_h_q16, ceilf((s_r + s_g + s_b) / 3.0f),
					pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, ise_endpoint_range, first_submode, last_submode))
				{
					improved = true;
				}

				// Added this - quite strong.
				if (try_mode7(num_pixels, blk_endpoints, blk_weights, cur_block_error, best_submode,
					cur_h_q16, minimum(maximum(s_r, s_g, s_b) * 1.1f, 65535.0f),
					pBlock_pixels_half, num_weight_levels, ise_weight_range, coptions, ise_endpoint_range, first_submode, last_submode))
				{
					improved = true;
				}
			} // if (coptions.m_mode7_full_s_optimization)

		} // if (fabs(t) > .0000125f)

		if (!improved)
			break;

		memcpy(trial_blk_endpoints, blk_endpoints, NUM_MODE7_ENDPOINTS);
		memcpy(trial_blk_weights, blk_weights, num_pixels);

	} // trial

	return cur_block_error;
}

//--------------------------------------------------------------------------------------------------------------------------

void dequantize_astc_weights(uint32_t n, const uint8_t* pSrc_ise_vals, uint32_t from_ise_range, uint8_t* pDst_raw_weights)
{
	const auto& dequant_tab = astc_helpers::g_dequant_tables.get_weight_tab(from_ise_range).m_ISE_to_val;

	for (uint32_t i = 0; i < n; i++)
		pDst_raw_weights[i] = dequant_tab[pSrc_ise_vals[i]];
}

//--------------------------------------------------------------------------------------------------------------------------
// Precomputed matrices via SLSQP (Sequential Least-Squares Quadratic Programming - scipy.optimize.minimize). Sharper results vs. other methods (like adjoint).

// For each output (2x2) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_2x2[4][36] = {
{0.165438f, 0.132609f, 0.092681f, 0.028953f, 0.000000f, 0.000000f, 0.133716f, 0.111240f, 0.065133f, 0.022236f, 0.000000f, 0.000000f, 0.092623f, 0.063898f, 0.039120f, 0.000000f, 0.000000f, 0.000000f, 0.028168f, 0.024184f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.027262f, 0.091051f, 0.132446f, 0.164791f, 0.000000f, 0.000000f, 0.026038f, 0.066511f, 0.111644f, 0.133197f, 0.000000f, 0.000000f, 0.000000f, 0.040053f, 0.064757f, 0.091196f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024265f, 0.026789f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028282f, 0.024804f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.092871f, 0.066580f, 0.042024f, 0.000000f, 0.000000f, 0.000000f, 0.132115f, 0.107586f, 0.061943f, 0.025551f, 0.000000f, 0.000000f, 0.166111f, 0.132946f, 0.089043f, 0.030145f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024535f, 0.028835f, 0.000000f, 0.000000f, 0.000000f, 0.044465f, 0.063652f, 0.093251f, 0.000000f, 0.000000f, 0.025961f, 0.063339f, 0.107329f, 0.132240f, 0.000000f, 0.000000f, 0.029844f, 0.089249f, 0.132200f, 0.165099f},
};

// For each output (3x2) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_3x2[6][36] = {
{0.257933f, 0.144768f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.213754f, 0.109376f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.140969f, 0.064128f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.041270f, 0.027803f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.046066f, 0.153691f, 0.153395f, 0.042845f, 0.000000f, 0.000000f, 0.038497f, 0.131674f, 0.126804f, 0.041513f, 0.000000f, 0.000000f, 0.028434f, 0.081152f, 0.075499f, 0.025372f, 0.000000f, 0.000000f, 0.000000f, 0.030067f, 0.024989f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.147088f, 0.258980f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.105549f, 0.211746f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.066714f, 0.144015f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027755f, 0.038152f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.044268f, 0.030990f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.141642f, 0.069930f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.207393f, 0.105354f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.255911f, 0.144511f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026658f, 0.032535f, 0.000000f, 0.000000f, 0.000000f, 0.024618f, 0.079487f, 0.080415f, 0.026311f, 0.000000f, 0.000000f, 0.038382f, 0.133569f, 0.133162f, 0.033451f, 0.000000f, 0.000000f, 0.043697f, 0.152483f, 0.154345f, 0.040885f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026401f, 0.040228f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.066688f, 0.142350f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.108504f, 0.210286f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.149666f, 0.255876f},
};

// For each output (4x2) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_4x2[8][36] = {
{0.318857f, 0.081413f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.262816f, 0.064811f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.175211f, 0.046152f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.050740f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.163830f, 0.223661f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.128904f, 0.194332f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.080369f, 0.121162f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.041941f, 0.045801f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.230801f, 0.166220f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.193495f, 0.136548f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.113816f, 0.085890f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.043771f, 0.029459f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.087528f, 0.318213f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.059739f, 0.262039f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.046515f, 0.175973f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.049993f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.054078f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.173243f, 0.055145f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.254561f, 0.059695f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.319463f, 0.083816f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.038171f, 0.037447f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.076263f, 0.117360f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.134218f, 0.202503f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.163759f, 0.230278f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.044607f, 0.035170f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.114466f, 0.088407f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.201026f, 0.127983f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.224148f, 0.164194f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.052817f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.043531f, 0.174390f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.060164f, 0.262636f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.089340f, 0.317122f},
};

// For each output (5x2) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_5x2[10][36] = {
{0.393855f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.327491f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.216089f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.062565f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.303101f, 0.078223f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.261199f, 0.068761f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.160056f, 0.054634f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.074026f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.202529f, 0.207447f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.151013f, 0.157673f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.100074f, 0.095239f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.043623f, 0.042402f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.083336f, 0.309647f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.061432f, 0.269582f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.046328f, 0.166035f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063640f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.397684f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.326178f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.217856f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.058282f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.065541f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.215996f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.321124f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.397338f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.069030f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.159434f, 0.051902f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.266327f, 0.065732f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.305627f, 0.081948f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.038550f, 0.046259f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.092606f, 0.100038f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.162523f, 0.163345f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.199767f, 0.196912f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.066709f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.050841f, 0.169003f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.061591f, 0.265094f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.081426f, 0.305335f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063517f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.210896f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.316133f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027674f, 0.381781f},
};

// For each output (6x2) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_6x2[12][36] = {
{0.395563f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.328397f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.214936f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.061104f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.395041f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.323513f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.208086f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.073360f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.393200f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.317339f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.218679f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.070782f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.399071f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.321356f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.214689f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064883f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.399159f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.326009f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.212426f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.062406f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.398973f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.326510f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.217446f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.057071f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.065386f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.215039f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.321113f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.398462f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.072234f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.211515f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.319185f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.397066f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.053184f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.213286f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.332634f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.400895f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063501f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.207210f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.334096f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.395193f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.074315f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.216723f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.320827f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.388135f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063571f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.215814f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.325843f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.394772f},
};

// For each output (2x3) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_2x3[6][36] = {
{0.253933f, 0.211745f, 0.142964f, 0.043509f, 0.000000f, 0.000000f, 0.146094f, 0.108119f, 0.068727f, 0.024908f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.043336f, 0.140540f, 0.208745f, 0.253069f, 0.000000f, 0.000000f, 0.031333f, 0.069242f, 0.108596f, 0.145138f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.044780f, 0.036916f, 0.026808f, 0.000000f, 0.000000f, 0.000000f, 0.151455f, 0.129189f, 0.076266f, 0.030885f, 0.000000f, 0.000000f, 0.151915f, 0.131628f, 0.081598f, 0.031903f, 0.000000f, 0.000000f, 0.043838f, 0.032645f, 0.030173f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028998f, 0.038454f, 0.046460f, 0.000000f, 0.000000f, 0.033717f, 0.076274f, 0.130140f, 0.153377f, 0.000000f, 0.000000f, 0.025762f, 0.077843f, 0.130195f, 0.150217f, 0.000000f, 0.000000f, 0.000000f, 0.029422f, 0.034493f, 0.044648f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.145243f, 0.107655f, 0.062280f, 0.033041f, 0.000000f, 0.000000f, 0.257369f, 0.210260f, 0.139667f, 0.044485f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037604f, 0.064104f, 0.105759f, 0.144848f, 0.000000f, 0.000000f, 0.042699f, 0.141511f, 0.207704f, 0.255772f},
};

// For each output (3x3) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_3x3[9][36] = {
{0.412913f, 0.237773f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.237370f, 0.111944f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.066531f, 0.251421f, 0.245639f, 0.065785f, 0.000000f, 0.000000f, 0.047059f, 0.143642f, 0.128760f, 0.051164f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.234587f, 0.419421f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.110765f, 0.235227f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.067391f, 0.044131f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.248992f, 0.133218f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.247568f, 0.139987f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.072238f, 0.046475f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.040674f, 0.048555f, 0.000000f, 0.000000f, 0.000000f, 0.049640f, 0.158199f, 0.158521f, 0.046044f, 0.000000f, 0.000000f, 0.043591f, 0.153956f, 0.155258f, 0.049378f, 0.000000f, 0.000000f, 0.000000f, 0.046674f, 0.049509f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.049528f, 0.063611f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.137662f, 0.252612f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.134924f, 0.246668f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.042655f, 0.072341f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.237403f, 0.114850f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.418506f, 0.229241f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.049009f, 0.142093f, 0.136891f, 0.036294f, 0.000000f, 0.000000f, 0.074433f, 0.244437f, 0.251631f, 0.065212f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.121166f, 0.231108f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.236230f, 0.411495f},
};

// For each output (4x3) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_4x3[12][36] = {
{0.508292f, 0.132529f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.285382f, 0.073798f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.266624f, 0.378457f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.144380f, 0.210539f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.380292f, 0.270590f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.200825f, 0.148293f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.130560f, 0.507542f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.071578f, 0.290320f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.094051f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.322294f, 0.082665f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.316365f, 0.092271f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.092353f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.046081f, 0.061377f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.158151f, 0.235006f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.152896f, 0.232594f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.052844f, 0.061053f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.061619f, 0.046867f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.227763f, 0.158202f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.222620f, 0.155545f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.073398f, 0.053986f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.082287f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.084098f, 0.330283f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.085224f, 0.323658f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.094451f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.286413f, 0.077046f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.512915f, 0.123625f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.140389f, 0.213324f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.267125f, 0.379163f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.208464f, 0.139969f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.382876f, 0.268691f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.080416f, 0.285653f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.131803f, 0.502128f},
};

// For each output (5x3) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_5x3[15][36] = {
{0.618662f, 0.032137f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.349200f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.497060f, 0.129255f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.281642f, 0.092043f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.333166f, 0.338337f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.164333f, 0.164165f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.129409f, 0.504176f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.085525f, 0.280890f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.636943f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.363057f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.113467f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.394204f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.386741f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.105588f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.086925f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.317750f, 0.095763f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.321008f, 0.086368f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.092185f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.057696f, 0.061462f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.184995f, 0.197656f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.186342f, 0.186715f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.059712f, 0.065422f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.091939f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.079906f, 0.328876f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.085955f, 0.320229f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.093096f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.099585f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.398489f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.388782f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.113144f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.360655f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.639345f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.285578f, 0.088663f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.495946f, 0.129812f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.177513f, 0.166195f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.329950f, 0.326342f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.082692f, 0.279744f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.134353f, 0.503211f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.361178f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.638822f},
};

// For each output (6x3) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_6x3[18][36] = {
{0.640623f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.359377f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.638697f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.361303f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.640672f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.359328f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.637721f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.362279f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.647342f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.352658f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.638418f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.361582f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.111041f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.395972f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.387932f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.105054f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.101949f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.395728f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.401263f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.101060f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.098132f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.388180f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.402030f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.111659f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.096173f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.393865f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.386312f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.123650f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.104357f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.398062f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.393265f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.104316f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.097666f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.400772f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.390396f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.111166f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.359466f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.640534f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.360569f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.639431f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.355750f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.644250f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.353865f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.646135f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.357727f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.642273f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.359539f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.640461f},
};

// For each output (2x4) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_2x4[8][36] = {
{0.312206f, 0.261492f, 0.177496f, 0.055798f, 0.000000f, 0.000000f, 0.081944f, 0.062361f, 0.048703f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.054679f, 0.172805f, 0.260561f, 0.314742f, 0.000000f, 0.000000f, 0.000000f, 0.049040f, 0.065652f, 0.082520f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.164115f, 0.129589f, 0.083879f, 0.029309f, 0.000000f, 0.000000f, 0.231202f, 0.198851f, 0.118719f, 0.044334f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.035855f, 0.083276f, 0.127764f, 0.166965f, 0.000000f, 0.000000f, 0.045347f, 0.116503f, 0.193645f, 0.230645f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.223790f, 0.194804f, 0.115855f, 0.047371f, 0.000000f, 0.000000f, 0.164616f, 0.125798f, 0.087268f, 0.040497f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.044738f, 0.118365f, 0.198854f, 0.230745f, 0.000000f, 0.000000f, 0.029646f, 0.078141f, 0.131405f, 0.168106f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.080206f, 0.060505f, 0.041197f, 0.000000f, 0.000000f, 0.000000f, 0.320486f, 0.265233f, 0.174992f, 0.057380f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.051057f, 0.058139f, 0.082120f, 0.000000f, 0.000000f, 0.056168f, 0.174118f, 0.260525f, 0.317873f},
};

// For each output (3x4) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_3x4[12][36] = {
{0.503381f, 0.288537f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.130806f, 0.077275f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.088808f, 0.319226f, 0.312498f, 0.086797f, 0.000000f, 0.000000f, 0.000000f, 0.092065f, 0.079421f, 0.021185f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.286250f, 0.514036f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.072999f, 0.126714f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.261935f, 0.133191f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.376226f, 0.207118f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021529f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.059585f, 0.153016f, 0.152552f, 0.043373f, 0.000000f, 0.000000f, 0.063990f, 0.231504f, 0.235283f, 0.060696f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.146403f, 0.262394f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.208547f, 0.382656f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.374676f, 0.209306f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.270440f, 0.145577f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.059636f, 0.233975f, 0.235944f, 0.069029f, 0.000000f, 0.000000f, 0.048950f, 0.150198f, 0.154340f, 0.047929f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.200921f, 0.380881f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.146928f, 0.271271f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.128883f, 0.075468f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.509859f, 0.285791f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.095842f, 0.086878f, 0.000000f, 0.000000f, 0.000000f, 0.092942f, 0.314169f, 0.319263f, 0.090906f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.079652f, 0.124852f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.289868f, 0.505628f},
};

// For each output (4x4) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_4x4[16][36] = {
{0.665277f, 0.167914f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.166809f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.325854f, 0.449938f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.094690f, 0.129518f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.455174f, 0.326025f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.109174f, 0.109627f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.166733f, 0.664155f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.169112f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.320619f, 0.090788f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.462066f, 0.126527f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.165890f, 0.235855f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.233931f, 0.364324f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.239319f, 0.151533f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.363629f, 0.245519f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.106763f, 0.311932f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.119451f, 0.461853f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.451893f, 0.124086f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.326160f, 0.097861f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.239712f, 0.365585f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.164178f, 0.230525f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.360274f, 0.237862f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.246139f, 0.155726f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.121863f, 0.457051f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.097828f, 0.323258f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.163634f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.667648f, 0.168718f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.094870f, 0.132660f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.316878f, 0.455591f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.116917f, 0.098433f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.458816f, 0.325834f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.168403f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.172019f, 0.659578f},
};

// For each output (5x4) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_5x4[20][36] = {
{0.773702f, 0.033711f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.192588f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.633422f, 0.166577f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.170080f, 0.029921f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.388335f, 0.403694f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.100996f, 0.106975f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.161122f, 0.655288f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.183590f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.801705f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.198295f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.400989f, 0.025097f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.573915f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.309345f, 0.085396f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.478694f, 0.126565f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.194664f, 0.187267f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.292735f, 0.308960f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.016375f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.098049f, 0.295983f, 0.000000f, 0.000000f, 0.017892f, 0.000000f, 0.111938f, 0.476138f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.043545f, 0.386448f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.570007f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.566407f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.402307f, 0.031286f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.463145f, 0.120696f, 0.000000f, 0.019497f, 0.000000f, 0.000000f, 0.311721f, 0.084942f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.296730f, 0.300781f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.204639f, 0.197849f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.122117f, 0.469302f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.102545f, 0.306036f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.562064f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.041534f, 0.396403f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.190134f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.773971f, 0.035896f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.169927f, 0.035812f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.630284f, 0.163977f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.112667f, 0.106813f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.393502f, 0.387018f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.177024f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.170482f, 0.652494f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.192274f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033039f, 0.774687f},
};

// For each output (6x4) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_6x4[24][36] = {
{0.804254f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.195746f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.804177f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.195823f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.799585f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.200415f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.803604f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.196396f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.807256f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.192744f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.805135f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.194865f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.410532f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.589468f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.408690f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.591310f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.416225f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.583775f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.414279f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.585721f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.406723f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.593277f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.402510f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.597490f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.584784f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.415216f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.590427f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.409573f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.590073f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.409927f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.580348f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.419652f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.588321f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.411679f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.587022f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.412978f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.193281f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.806719f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.189163f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.810837f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.195108f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.804892f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.188290f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.811710f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.192914f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.807086f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.195292f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.804708f},
};

// For each output (2x5) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_2x5[10][36] = {
{0.387593f, 0.325123f, 0.221104f, 0.066180f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.065940f, 0.214659f, 0.326737f, 0.392664f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.309603f, 0.265953f, 0.168780f, 0.060600f, 0.000000f, 0.000000f, 0.084707f, 0.063017f, 0.047341f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.062836f, 0.170767f, 0.261053f, 0.307978f, 0.000000f, 0.000000f, 0.000000f, 0.049286f, 0.064361f, 0.083719f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.195787f, 0.153943f, 0.095706f, 0.042417f, 0.000000f, 0.000000f, 0.190695f, 0.154435f, 0.097288f, 0.040258f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029471f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.017536f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.039307f, 0.094677f, 0.158696f, 0.199136f, 0.000000f, 0.000000f, 0.040959f, 0.093353f, 0.155294f, 0.201042f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.079432f, 0.065739f, 0.044876f, 0.000000f, 0.000000f, 0.000000f, 0.309205f, 0.264700f, 0.167247f, 0.068801f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.052112f, 0.064829f, 0.081363f, 0.000000f, 0.000000f, 0.064024f, 0.161136f, 0.263743f, 0.312793f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.393277f, 0.324792f, 0.213188f, 0.068743f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.066964f, 0.215440f, 0.323005f, 0.394591f},
};

// For each output (3x5) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_3x5[15][36] = {
{0.620557f, 0.350797f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028646f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.110170f, 0.397489f, 0.386326f, 0.106015f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.357348f, 0.642652f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.503934f, 0.275289f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.128280f, 0.092497f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.102294f, 0.316223f, 0.313576f, 0.092518f, 0.000000f, 0.000000f, 0.000000f, 0.081158f, 0.094231f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.279079f, 0.502163f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.086083f, 0.132675f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.325483f, 0.157739f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.322567f, 0.172225f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021986f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063342f, 0.192228f, 0.186950f, 0.057021f, 0.000000f, 0.000000f, 0.054779f, 0.186114f, 0.185666f, 0.073901f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.172195f, 0.331802f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.148212f, 0.322038f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025751f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.123726f, 0.081188f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.507339f, 0.287746f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.093924f, 0.094021f, 0.000000f, 0.000000f, 0.000000f, 0.097070f, 0.315697f, 0.314560f, 0.084728f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.082560f, 0.129771f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.277014f, 0.486817f, 0.023837f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.644191f, 0.355809f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.107771f, 0.387615f, 0.393454f, 0.111159f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.360886f, 0.639114f},
};

// For each output (4x5) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_4x5[20][36] = {
{0.778254f, 0.190730f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.031016f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.401147f, 0.570243f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028610f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.563768f, 0.394241f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.041992f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.196238f, 0.767548f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.036214f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.637514f, 0.166734f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.167634f, 0.028118f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.322778f, 0.473312f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.085399f, 0.118511f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.471429f, 0.308185f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.118025f, 0.102361f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.176592f, 0.643933f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.179475f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.391609f, 0.100882f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.390531f, 0.116978f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.017259f, 0.000000f, 0.201618f, 0.301555f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.197600f, 0.281968f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.016735f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.293309f, 0.192842f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.268674f, 0.208109f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020330f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.118514f, 0.380746f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.097621f, 0.381305f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021814f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.157977f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.657533f, 0.184490f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.097522f, 0.128585f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.309864f, 0.464029f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.128900f, 0.090864f, 0.000000f, 0.025393f, 0.000000f, 0.000000f, 0.464029f, 0.290814f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024593f, 0.172268f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.173412f, 0.629727f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029582f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.778816f, 0.191602f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.036297f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.394454f, 0.569249f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.039685f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.561207f, 0.399108f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.034683f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.193744f, 0.771574f},
};

// For each output (5x5) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_5x5[25][36] = {
{1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.794727f, 0.205273f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.465125f, 0.484079f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028881f, 0.000000f, 0.000000f, 0.021914f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.192446f, 0.772941f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.034613f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033123f, 0.930510f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.036367f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.800234f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.199766f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.629079f, 0.165939f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.166390f, 0.019675f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018918f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.378734f, 0.373861f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.111597f, 0.135808f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.177492f, 0.641195f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.181313f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028722f, 0.761781f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.209497f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.475763f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.471882f, 0.029551f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022804f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.382714f, 0.116167f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.383377f, 0.117742f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.254151f, 0.249987f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.241972f, 0.253891f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.017950f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.122722f, 0.376847f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.095099f, 0.369986f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.017396f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029442f, 0.472507f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.471751f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026300f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.190299f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.776924f, 0.032778f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.171498f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.666385f, 0.162117f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.125713f, 0.117624f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.387084f, 0.369579f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028493f, 0.169318f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.173770f, 0.628419f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.198951f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.035634f, 0.765415f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.963102f, 0.036898f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.030322f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.771054f, 0.198624f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021816f, 0.020944f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.481761f, 0.475479f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.032816f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.198418f, 0.768766f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033338f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966662f},
};

// For each output (6x5) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_6x5[30][36] = {
{0.966284f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033716f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.966287f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033713f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.966287f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033713f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.966290f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033710f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966125f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033875f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966273f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033727f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.800857f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.199143f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.773463f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.201165f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025372f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.805735f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.194265f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.788791f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.211209f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.785975f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.214025f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.787286f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.212714f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.490845f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.487242f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021913f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.490663f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.486878f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022459f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.505452f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.494548f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.495383f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.482180f, 0.000000f, 0.022437f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.022727f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.496545f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.480728f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.486261f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.486387f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027352f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.196272f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.803728f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.210059f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.789941f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.212947f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.787053f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.215261f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.784739f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.209116f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.790884f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.205881f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.794119f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033710f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966290f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033711f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966289f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033713f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966287f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033719f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966281f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033712f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966288f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033712f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966288f},
};

// For each output (2x6) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_2x6[12][36] = {
{0.388815f, 0.325435f, 0.220189f, 0.065562f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.064515f, 0.214042f, 0.327700f, 0.393742f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.398821f, 0.326200f, 0.217851f, 0.057128f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.062546f, 0.216408f, 0.322269f, 0.398777f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.396575f, 0.330631f, 0.212857f, 0.059936f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.070253f, 0.215326f, 0.317576f, 0.396845f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.398130f, 0.324745f, 0.213572f, 0.063553f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.062009f, 0.216253f, 0.324683f, 0.397055f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.397646f, 0.321346f, 0.212334f, 0.068675f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.067073f, 0.210768f, 0.318165f, 0.403993f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.395756f, 0.325048f, 0.211862f, 0.067334f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.065475f, 0.214113f, 0.324009f, 0.396403f},
};

// For each output (3x6) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_3x6[18][36] = {
{0.640136f, 0.359864f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.108112f, 0.399968f, 0.388087f, 0.103833f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.356122f, 0.643878f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.646308f, 0.353692f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.122937f, 0.390166f, 0.380558f, 0.106339f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.355015f, 0.644985f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.642874f, 0.357126f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.111570f, 0.398638f, 0.387639f, 0.102153f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.359134f, 0.640866f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.640159f, 0.359841f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.098908f, 0.393303f, 0.400421f, 0.107369f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.357119f, 0.642881f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.640541f, 0.359459f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.116318f, 0.397635f, 0.395084f, 0.090964f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.361948f, 0.638052f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.645448f, 0.354552f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.106981f, 0.389214f, 0.395056f, 0.108749f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.359592f, 0.640408f},
};

// For each output (4x6) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_4x6[24][36] = {
{0.806928f, 0.193072f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.412216f, 0.587784f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.590075f, 0.409925f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.200682f, 0.799318f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.809822f, 0.190178f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.423474f, 0.576526f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.580816f, 0.419184f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.190240f, 0.809760f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.800320f, 0.199680f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.408625f, 0.591375f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.583392f, 0.416608f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.200372f, 0.799628f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.798914f, 0.201086f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.411243f, 0.588757f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.586520f, 0.413480f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.203588f, 0.796412f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.802040f, 0.197960f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.411175f, 0.588825f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.599873f, 0.400127f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.193060f, 0.806940f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.806073f, 0.193927f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.408705f, 0.591295f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.585711f, 0.414289f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.197672f, 0.802328f},
};

// For each output (5x6) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_5x6[30][36] = {
{0.966289f, 0.033711f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.794848f, 0.205152f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.473272f, 0.496525f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.030202f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.196955f, 0.803045f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033711f, 0.966289f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966284f, 0.033716f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.795787f, 0.204213f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.500928f, 0.499072f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.198603f, 0.801397f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033716f, 0.966284f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966283f, 0.033717f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.788424f, 0.211576f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029276f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.484227f, 0.486497f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.201499f, 0.798501f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033724f, 0.966276f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966283f, 0.033717f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.791336f, 0.208664f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.490188f, 0.509812f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.204835f, 0.795165f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033703f, 0.966297f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966276f, 0.033724f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.799276f, 0.200724f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.022501f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.494443f, 0.483055f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.205967f, 0.794033f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033726f, 0.966274f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.965971f, 0.034029f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.798640f, 0.201360f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.502577f, 0.497423f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.203927f, 0.796073f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033706f, 0.966294f},
};

// For each output (6x6) sample, the weight of each input (6x6) sample.
static const float g_weight_downsample_6x6_to_6x6[36][36] = {
{1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f},
};

//--------------------------------------------------------------------------------------------------------------------------

struct downsample_matrix
{
	uint32_t m_grid_width, m_grid_height;
	const float* m_p;
};

downsample_matrix g_downsample_matrices_6x6[] = 
{
	{ 2, 2, (const float*)g_weight_downsample_6x6_to_2x2 },
	{ 3, 2, (const float*)g_weight_downsample_6x6_to_3x2 },
	{ 4, 2, (const float*)g_weight_downsample_6x6_to_4x2 },
	{ 5, 2, (const float*)g_weight_downsample_6x6_to_5x2 },
	{ 6, 2, (const float*)g_weight_downsample_6x6_to_6x2 },
	{ 2, 3, (const float*)g_weight_downsample_6x6_to_2x3 },
	{ 3, 3, (const float*)g_weight_downsample_6x6_to_3x3 },
	{ 4, 3, (const float*)g_weight_downsample_6x6_to_4x3 },
	{ 5, 3, (const float*)g_weight_downsample_6x6_to_5x3 },
	{ 6, 3, (const float*)g_weight_downsample_6x6_to_6x3 },
	{ 2, 4, (const float*)g_weight_downsample_6x6_to_2x4 },
	{ 3, 4, (const float*)g_weight_downsample_6x6_to_3x4 },
	{ 4, 4, (const float*)g_weight_downsample_6x6_to_4x4 },
	{ 5, 4, (const float*)g_weight_downsample_6x6_to_5x4 },
	{ 6, 4, (const float*)g_weight_downsample_6x6_to_6x4 },
	{ 2, 5, (const float*)g_weight_downsample_6x6_to_2x5 },
	{ 3, 5, (const float*)g_weight_downsample_6x6_to_3x5 },
	{ 4, 5, (const float*)g_weight_downsample_6x6_to_4x5 },
	{ 5, 5, (const float*)g_weight_downsample_6x6_to_5x5 },
	{ 6, 5, (const float*)g_weight_downsample_6x6_to_6x5 },
	{ 2, 6, (const float*)g_weight_downsample_6x6_to_2x6 },
	{ 3, 6, (const float*)g_weight_downsample_6x6_to_3x6 },
	{ 4, 6, (const float*)g_weight_downsample_6x6_to_4x6 },
	{ 5, 6, (const float*)g_weight_downsample_6x6_to_5x6 },
	{ 6, 6, (const float*)g_weight_downsample_6x6_to_6x6 }
};
//const uint32_t NUM_DOWNSAMPLE_MATRICES_6x6 = sizeof(g_downsample_matrices_6x6) / sizeof(g_downsample_matrices_6x6[0]);

//--------------------------------------------------------------------------------------------------------------------------
// 
// For each output (2x2) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_2x2[4][48] = {
{0.137431f, 0.119592f, 0.085575f, 0.056401f, 0.030751f, 0.000000f, 0.000000f, 0.000000f, 0.108851f, 0.086312f, 0.064884f, 0.039119f, 0.027653f, 0.000000f, 0.000000f, 0.000000f, 0.073703f, 0.067584f, 0.045034f, 0.032697f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024414f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.033828f, 0.058911f, 0.081870f, 0.120975f, 0.137384f, 0.000000f, 0.000000f, 0.000000f, 0.026912f, 0.038126f, 0.065247f, 0.083628f, 0.109730f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037909f, 0.044325f, 0.065160f, 0.074043f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021952f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024645f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.074133f, 0.065243f, 0.043065f, 0.035114f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.105931f, 0.087385f, 0.065848f, 0.035699f, 0.030068f, 0.000000f, 0.000000f, 0.000000f, 0.136321f, 0.121324f, 0.086171f, 0.057503f, 0.031553f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024251f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037022f, 0.042379f, 0.063662f, 0.075871f, 0.000000f, 0.000000f, 0.000000f, 0.031315f, 0.037129f, 0.065785f, 0.084055f, 0.107841f, 0.000000f, 0.000000f, 0.000000f, 0.030537f, 0.057932f, 0.086040f, 0.120055f, 0.136127f},
};

// For each output (3x2) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_3x2[6][48] = {
{0.212556f, 0.137038f, 0.067006f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.172663f, 0.105023f, 0.058944f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.113989f, 0.074111f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037147f, 0.021524f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.077366f, 0.142656f, 0.145067f, 0.074900f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.048644f, 0.106713f, 0.104141f, 0.052434f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.048972f, 0.079367f, 0.079508f, 0.040229f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064479f, 0.139823f, 0.212207f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.053987f, 0.104596f, 0.171728f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026564f, 0.071759f, 0.119334f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.035524f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037522f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.115689f, 0.072510f, 0.021389f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.170967f, 0.106096f, 0.061696f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.210888f, 0.137969f, 0.065274f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.045147f, 0.080905f, 0.078591f, 0.043486f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.045421f, 0.106778f, 0.106427f, 0.050794f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.079169f, 0.139959f, 0.144180f, 0.079143f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033940f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021724f, 0.070791f, 0.117496f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.059938f, 0.109787f, 0.170583f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064517f, 0.139526f, 0.211698f},
};

// For each output (4x2) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_4x2[8][48] = {
{0.275657f, 0.133248f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.225305f, 0.089819f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.147466f, 0.079439f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.049065f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.071558f, 0.188360f, 0.141460f, 0.027429f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.068719f, 0.139588f, 0.107851f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024602f, 0.112032f, 0.076880f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019401f, 0.000000f, 0.022120f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.025244f, 0.140416f, 0.189606f, 0.065541f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021281f, 0.106671f, 0.142270f, 0.062848f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.068039f, 0.102306f, 0.026541f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023517f, 0.025720f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.136533f, 0.275463f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.086827f, 0.223674f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.077361f, 0.153684f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.046457f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.048293f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.149189f, 0.077647f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.222753f, 0.093443f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.273639f, 0.135036f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022695f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027966f, 0.116923f, 0.074704f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.066610f, 0.140552f, 0.119791f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.070250f, 0.192769f, 0.140414f, 0.027327f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026026f, 0.032280f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.073723f, 0.105102f, 0.027631f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.113307f, 0.139466f, 0.059915f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027161f, 0.140907f, 0.189935f, 0.064546f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.045275f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.074412f, 0.151685f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.094074f, 0.223897f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.136604f, 0.274053f},
};

// For each output (5x2) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_5x2[10][48] = {
{0.298257f, 0.099048f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.242705f, 0.083012f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.155959f, 0.035340f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.054463f, 0.031217f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.149629f, 0.250491f, 0.037003f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.113317f, 0.192720f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.093738f, 0.138010f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025093f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.193314f, 0.196494f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.163178f, 0.158983f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.112334f, 0.115733f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028572f, 0.031390f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028975f, 0.256222f, 0.142262f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.191874f, 0.111703f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.137754f, 0.096234f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.034976f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.105369f, 0.297279f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.081692f, 0.239675f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.031939f, 0.162333f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.031404f, 0.050308f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.053972f, 0.028379f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.158432f, 0.035219f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.238959f, 0.089734f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.294641f, 0.100664f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.034176f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.090008f, 0.147020f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.103221f, 0.190008f, 0.024843f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.139784f, 0.245082f, 0.025860f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.032527f, 0.032618f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.117780f, 0.108323f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.155910f, 0.159880f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.197210f, 0.195753f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.042681f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.138684f, 0.099059f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.186926f, 0.105714f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029545f, 0.254477f, 0.142915f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029953f, 0.051219f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029174f, 0.163463f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.087461f, 0.240531f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.103819f, 0.294380f},
};

// For each output (6x2) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_6x2[12][48] = {
{0.362153f, 0.050427f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.296074f, 0.031598f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.192551f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.067197f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.240020f, 0.169624f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.196469f, 0.128913f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.131714f, 0.098049f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.035210f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.105361f, 0.301218f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.086270f, 0.220336f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.047552f, 0.171037f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022966f, 0.045259f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.287211f, 0.111854f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.224383f, 0.097742f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.167408f, 0.037607f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.036827f, 0.036969f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.152162f, 0.235841f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.108280f, 0.202388f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.091687f, 0.151852f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.057789f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.051343f, 0.374208f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.304381f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.207583f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.062485f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064793f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.193058f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.290484f, 0.038424f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.357650f, 0.055589f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.035640f, 0.019558f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.133571f, 0.100435f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.184400f, 0.125111f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.228117f, 0.173168f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.044711f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.043438f, 0.175074f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.089766f, 0.235789f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.108452f, 0.302770f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037495f, 0.032008f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.168503f, 0.033572f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.226763f, 0.101709f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.292934f, 0.107016f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019003f, 0.018791f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.100854f, 0.125828f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.107572f, 0.206978f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.169736f, 0.251237f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.060542f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024678f, 0.204824f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.301594f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.040204f, 0.368158f},
};

// For each output (7x2) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_7x2[14][48] = {
{0.396534f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.324924f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.210380f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.068162f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.365804f, 0.047637f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.288211f, 0.031570f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.215416f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.051362f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.277573f, 0.121338f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.219048f, 0.084370f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023178f, 0.000000f, 0.161469f, 0.031346f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.034866f, 0.046814f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.194115f, 0.218789f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.163854f, 0.137782f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020281f, 0.000000f, 0.127129f, 0.138049f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.089911f, 0.279003f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.100285f, 0.229490f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026109f, 0.164969f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.036219f, 0.074014f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033369f, 0.385493f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.300028f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.222803f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.058307f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.395806f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.320906f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.218670f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064618f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064591f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.213009f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.324054f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.398346f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.052403f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.218943f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.280900f, 0.028228f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.364696f, 0.054830f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.040226f, 0.027986f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.172678f, 0.019447f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.228976f, 0.118935f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.278251f, 0.113500f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.017206f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022203f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022373f, 0.000000f, 0.138786f, 0.130317f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024343f, 0.000000f, 0.127713f, 0.134415f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.187440f, 0.195205f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033347f, 0.041046f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029210f, 0.133093f, 0.000000f, 0.020285f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.102427f, 0.246296f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.104431f, 0.289864f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027153f, 0.048478f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.032573f, 0.217822f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.278933f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022617f, 0.372424f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.061793f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.219494f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.324119f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.394594f},
};

// For each output (8x2) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_8x2[16][48] = {
{0.397679f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.325539f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.208885f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.067897f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.394986f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.323551f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.218305f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063158f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.400685f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.325867f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.214372f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.059075f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.398573f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.319207f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.212413f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.069808f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.401571f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.323398f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.212771f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.062260f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.404990f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.322008f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.207631f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.065371f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.396891f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.320883f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.212780f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.069447f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.396345f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.321731f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.217640f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064285f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064801f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.212540f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.324204f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.398456f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063907f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.221286f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.319039f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.395768f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064375f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.221627f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.320522f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.393476f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.067161f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.214405f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.322795f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.395638f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.065100f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.209382f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.325769f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.399749f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.072177f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.207268f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.318619f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.401935f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063557f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.217484f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.316546f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.402413f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.061762f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.218082f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.324604f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.395552f},
};

// For each output (2x3) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_2x3[6][48] = {
{0.205910f, 0.181220f, 0.131230f, 0.084091f, 0.045598f, 0.000000f, 0.000000f, 0.000000f, 0.115248f, 0.106195f, 0.073083f, 0.057425f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.054674f, 0.092055f, 0.125587f, 0.176378f, 0.202284f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.055452f, 0.075306f, 0.102574f, 0.115689f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.044070f, 0.029520f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.136903f, 0.115512f, 0.084403f, 0.050846f, 0.035490f, 0.000000f, 0.000000f, 0.000000f, 0.143459f, 0.115683f, 0.085020f, 0.053056f, 0.036572f, 0.000000f, 0.000000f, 0.000000f, 0.043466f, 0.026000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025190f, 0.040099f, 0.000000f, 0.000000f, 0.000000f, 0.037965f, 0.050927f, 0.083471f, 0.112563f, 0.137468f, 0.000000f, 0.000000f, 0.000000f, 0.033927f, 0.046348f, 0.085573f, 0.114643f, 0.134372f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024810f, 0.028641f, 0.044003f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.111326f, 0.107232f, 0.073233f, 0.050676f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.204047f, 0.179532f, 0.131819f, 0.088809f, 0.053325f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023277f, 0.054224f, 0.067723f, 0.100097f, 0.113199f, 0.000000f, 0.000000f, 0.000000f, 0.047881f, 0.085543f, 0.130088f, 0.176198f, 0.201769f},
};

// For each output (3x3) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_3x3[9][48] = {
{0.327238f, 0.215195f, 0.108640f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.184524f, 0.118385f, 0.046018f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.109423f, 0.206952f, 0.207632f, 0.108494f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064973f, 0.120899f, 0.114663f, 0.066964f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.107663f, 0.213426f, 0.326644f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.045643f, 0.119988f, 0.186636f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.060005f, 0.030140f, 0.020392f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.193258f, 0.127396f, 0.061395f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.196600f, 0.132656f, 0.063337f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.060793f, 0.029915f, 0.024113f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.032682f, 0.042599f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.070428f, 0.145040f, 0.144782f, 0.074883f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.069308f, 0.145612f, 0.133265f, 0.071190f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.035901f, 0.034311f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.030350f, 0.056939f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.060846f, 0.125850f, 0.201518f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063906f, 0.129434f, 0.203119f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.035006f, 0.026673f, 0.066360f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.184897f, 0.119434f, 0.045977f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.328093f, 0.217057f, 0.104542f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064974f, 0.120280f, 0.118724f, 0.069494f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.111457f, 0.199814f, 0.204785f, 0.110472f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.038193f, 0.124885f, 0.182125f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.105011f, 0.218548f, 0.331237f},
};

// For each output (4x3) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_4x3[12][48] = {
{0.424820f, 0.213734f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.237540f, 0.123907f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.101064f, 0.293828f, 0.214193f, 0.045263f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.051229f, 0.170008f, 0.124414f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.043452f, 0.216897f, 0.293802f, 0.110908f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.114842f, 0.173267f, 0.046832f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.204747f, 0.427412f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.126209f, 0.241633f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.087490f, 0.023647f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.277233f, 0.116842f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.282751f, 0.124394f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.087642f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024375f, 0.043221f, 0.025504f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.075199f, 0.165822f, 0.130107f, 0.031544f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.074010f, 0.171441f, 0.131257f, 0.016920f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037357f, 0.043775f, 0.029468f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.034358f, 0.046676f, 0.025003f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026567f, 0.127081f, 0.172282f, 0.077309f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028046f, 0.132256f, 0.162992f, 0.075728f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033213f, 0.036679f, 0.021810f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.083610f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.116623f, 0.293550f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.118246f, 0.292686f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.095285f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.234002f, 0.132935f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.422801f, 0.210262f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037740f, 0.173712f, 0.127636f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.107054f, 0.296425f, 0.213343f, 0.044090f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.122782f, 0.174732f, 0.044321f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.046279f, 0.214323f, 0.289278f, 0.108285f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.125079f, 0.236461f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.208583f, 0.429877f},
};

// For each output (5x3) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_5x3[15][48] = {
{0.490219f, 0.168976f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.273361f, 0.067444f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.213329f, 0.380538f, 0.048722f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.138224f, 0.219188f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.309867f, 0.312289f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.189101f, 0.188743f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037522f, 0.380550f, 0.216834f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.225818f, 0.139276f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.164462f, 0.488476f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.072635f, 0.274427f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.085550f, 0.041856f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.277218f, 0.100778f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.279523f, 0.102655f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.086943f, 0.025474f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018474f, 0.000000f, 0.000000f, 0.023807f, 0.063654f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.142638f, 0.245307f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.145790f, 0.254064f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.047600f, 0.058666f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.047090f, 0.051660f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.197880f, 0.207261f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.205538f, 0.186457f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.052816f, 0.051298f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018852f, 0.055366f, 0.033613f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.247747f, 0.138008f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.030549f, 0.240788f, 0.147930f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.066598f, 0.020549f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.031861f, 0.081013f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.095562f, 0.286515f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.091897f, 0.287997f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.038590f, 0.086564f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.268683f, 0.083034f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.485628f, 0.162655f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.121869f, 0.229484f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.218817f, 0.384593f, 0.045237f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.182342f, 0.183530f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.320205f, 0.313923f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.217960f, 0.138650f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.051048f, 0.375126f, 0.217217f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064150f, 0.273673f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.169346f, 0.492831f},
};

// For each output (6x3) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_6x3[18][48] = {
{0.567729f, 0.085252f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.316321f, 0.030698f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.359927f, 0.264711f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.204426f, 0.170936f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.160854f, 0.493683f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.055911f, 0.289551f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.471204f, 0.180222f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.281132f, 0.067442f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.244512f, 0.369052f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.158920f, 0.227515f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.066465f, 0.597036f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.336500f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.104579f, 0.023148f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.338908f, 0.039468f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.344319f, 0.042826f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.106751f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.059448f, 0.022978f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.245888f, 0.156583f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.251094f, 0.164427f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.073868f, 0.025715f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.047831f, 0.060057f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.116572f, 0.271105f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.108894f, 0.276085f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.039515f, 0.079942f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.080438f, 0.048264f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.267123f, 0.113138f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.263081f, 0.110654f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.077711f, 0.039591f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020193f, 0.059109f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.154371f, 0.249388f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.148917f, 0.263084f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021121f, 0.083817f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024900f, 0.107003f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.375065f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.378856f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.114175f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.311342f, 0.043011f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.565421f, 0.080225f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018768f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.192162f, 0.168731f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.354606f, 0.265733f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.069515f, 0.282839f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.159765f, 0.487881f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.278646f, 0.072312f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.480532f, 0.168510f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.157488f, 0.194745f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.261639f, 0.386129f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.043524f, 0.320675f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.055191f, 0.580610f},
};

// For each output (7x3) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_7x3[21][48] = {
{0.641452f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.358548f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.571435f, 0.068076f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.330216f, 0.030272f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.442607f, 0.191771f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.243785f, 0.063036f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018329f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019157f, 0.000000f, 0.021315f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.273064f, 0.307420f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.195541f, 0.177034f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022294f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024647f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.151030f, 0.456644f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.078617f, 0.291813f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021896f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.060980f, 0.596856f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.342163f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.639429f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.360571f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.114797f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.378786f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.387691f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.118726f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.090755f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.356378f, 0.041502f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.359468f, 0.040845f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.091221f, 0.019830f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.078340f, 0.030772f, 0.000000f, 0.017555f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.267597f, 0.100863f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.271447f, 0.100798f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064330f, 0.068296f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.044982f, 0.034940f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021793f, 0.000000f, 0.194246f, 0.216278f, 0.000000f, 0.022234f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.203237f, 0.184740f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019217f, 0.018086f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023471f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.016776f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.047044f, 0.060726f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.086110f, 0.270497f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.100587f, 0.267194f, 0.000000f, 0.020092f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.050739f, 0.097011f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023976f, 0.094747f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.036130f, 0.353791f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.032724f, 0.369552f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.089080f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.107420f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.386732f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.390932f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.114916f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.354042f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.645958f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.337170f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.589668f, 0.073162f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.281005f, 0.071771f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.450506f, 0.196718f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021998f, 0.000000f, 0.000000f, 0.025261f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.032091f, 0.000000f, 0.182952f, 0.186377f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.270805f, 0.280517f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020667f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064614f, 0.248064f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.182212f, 0.484444f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.046780f, 0.341462f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.041817f, 0.569940f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.355095f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.644905f},
};

// For each output (8x3) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_8x3[24][48] = {
{0.642405f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.357595f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.643957f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.356043f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.642833f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.357167f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.637580f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.362420f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.642714f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.357286f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.637481f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.362519f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.646282f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.353718f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.640587f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.359413f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.113933f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.379885f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.389232f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.116950f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.104449f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.396859f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.400104f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.098588f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.102359f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.394242f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.401732f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.101667f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.096440f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.392155f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.400404f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.111000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.114593f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.389960f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.382704f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.112742f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.109021f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.396881f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.388517f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.105580f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.108474f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.389562f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.401518f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.100446f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.106886f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.387604f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.392295f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.113215f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.353573f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.646427f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.356921f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.643079f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.363744f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.636256f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.356177f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.643823f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.354225f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.645775f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.359749f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.640251f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.364443f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.635557f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.353912f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.646088f},
};

// For each output (2x4) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_2x4[8][48] = {
{0.266475f, 0.237248f, 0.170961f, 0.108932f, 0.059980f, 0.000000f, 0.000000f, 0.000000f, 0.069153f, 0.052080f, 0.035172f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.071584f, 0.118291f, 0.158003f, 0.229344f, 0.262308f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.040608f, 0.047117f, 0.072745f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.133546f, 0.123736f, 0.085634f, 0.071146f, 0.020522f, 0.000000f, 0.000000f, 0.000000f, 0.181365f, 0.152470f, 0.109189f, 0.071277f, 0.051114f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.068769f, 0.083081f, 0.122611f, 0.135462f, 0.000000f, 0.000000f, 0.000000f, 0.052661f, 0.073804f, 0.122675f, 0.158233f, 0.182705f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.185771f, 0.157833f, 0.115265f, 0.071389f, 0.049909f, 0.000000f, 0.000000f, 0.000000f, 0.134315f, 0.122577f, 0.090159f, 0.072782f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.049580f, 0.068443f, 0.120275f, 0.155720f, 0.183091f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.072223f, 0.092680f, 0.123123f, 0.134866f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.061367f, 0.051211f, 0.034360f, 0.000000f, 0.028160f, 0.000000f, 0.000000f, 0.000000f, 0.255536f, 0.224675f, 0.167736f, 0.113503f, 0.063453f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033855f, 0.000000f, 0.030092f, 0.044250f, 0.067673f, 0.000000f, 0.000000f, 0.000000f, 0.059731f, 0.111955f, 0.169044f, 0.224131f, 0.259268f},
};

// For each output (3x4) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_3x4[12][48] = {
{0.405143f, 0.264455f, 0.127900f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.105076f, 0.051679f, 0.045747f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.025952f, 0.148689f, 0.283429f, 0.283899f, 0.145415f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.061558f, 0.051058f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.124702f, 0.268998f, 0.405480f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.043101f, 0.052379f, 0.105340f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.214261f, 0.145181f, 0.047508f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.296952f, 0.196156f, 0.099941f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.084673f, 0.137735f, 0.144414f, 0.077484f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.086806f, 0.178074f, 0.179109f, 0.089543f, 0.022161f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.050723f, 0.149013f, 0.214357f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.101549f, 0.190388f, 0.293970f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.293440f, 0.200404f, 0.104808f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.212205f, 0.141684f, 0.047458f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.085757f, 0.179609f, 0.175648f, 0.084745f, 0.021210f, 0.000000f, 0.000000f, 0.000000f, 0.083231f, 0.140659f, 0.147264f, 0.081878f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.104715f, 0.195444f, 0.297105f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.052478f, 0.135662f, 0.214595f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.105858f, 0.047177f, 0.044681f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.407919f, 0.269431f, 0.124933f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.066066f, 0.061881f, 0.023069f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.149307f, 0.272481f, 0.277246f, 0.149950f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.036865f, 0.065377f, 0.096438f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.123758f, 0.269301f, 0.408262f},
};

// For each output (4x4) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_4x4[16][48] = {
{0.550981f, 0.273527f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.143555f, 0.031938f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.122629f, 0.360487f, 0.261668f, 0.049773f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.061033f, 0.081604f, 0.062805f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.049839f, 0.269578f, 0.365997f, 0.133966f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.048352f, 0.083803f, 0.048464f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.267525f, 0.553972f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.034129f, 0.144375f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.277118f, 0.159322f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.390449f, 0.173111f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.047384f, 0.191890f, 0.131656f, 0.024565f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.109738f, 0.256529f, 0.192107f, 0.046132f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.031695f, 0.141682f, 0.193059f, 0.054775f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.036195f, 0.182374f, 0.246275f, 0.113945f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.160040f, 0.281798f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.166904f, 0.391257f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.392178f, 0.179451f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.279598f, 0.148773f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.107261f, 0.247609f, 0.198942f, 0.036907f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.054678f, 0.195067f, 0.134127f, 0.025410f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.017019f, 0.017319f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.032887f, 0.182133f, 0.239063f, 0.107658f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026552f, 0.139058f, 0.187193f, 0.051118f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.169923f, 0.395389f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.148923f, 0.285765f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.142165f, 0.038534f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.547445f, 0.271856f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.044944f, 0.076529f, 0.068448f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.125039f, 0.368874f, 0.262015f, 0.054151f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.059929f, 0.083064f, 0.044633f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.053433f, 0.265593f, 0.362429f, 0.130919f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.045972f, 0.135681f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.264414f, 0.553933f},
};

// For each output (5x4) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_5x4[20][48] = {
{0.596845f, 0.198746f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.148428f, 0.055981f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.278053f, 0.491329f, 0.050522f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064229f, 0.115868f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.404918f, 0.399709f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.097883f, 0.097489f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.050295f, 0.498737f, 0.280436f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.117869f, 0.052664f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.200415f, 0.589668f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063856f, 0.146061f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.306027f, 0.097934f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.428737f, 0.167302f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.155850f, 0.258285f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.187173f, 0.344891f, 0.035315f, 0.000000f, 0.018485f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.212411f, 0.213232f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.283532f, 0.290826f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022380f, 0.255191f, 0.169763f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020378f, 0.342025f, 0.190264f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.089095f, 0.316913f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.159089f, 0.434903f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.436982f, 0.169707f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.310539f, 0.082773f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.187439f, 0.337224f, 0.031428f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.167442f, 0.252995f, 0.023472f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.298614f, 0.285810f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.206405f, 0.209172f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019544f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033200f, 0.325724f, 0.185761f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.030366f, 0.251622f, 0.153784f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.161862f, 0.437691f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.086681f, 0.313765f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.149673f, 0.068654f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.589414f, 0.192260f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.038852f, 0.121054f, 0.025391f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.280331f, 0.492424f, 0.041948f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.095308f, 0.102698f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.407796f, 0.394198f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.106939f, 0.057645f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.058299f, 0.489157f, 0.287960f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063501f, 0.142763f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.196593f, 0.597142f},
};

// For each output (6x4) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_6x4[24][48] = {
{0.723801f, 0.094637f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.181562f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.476584f, 0.344817f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.116143f, 0.062457f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.194537f, 0.608409f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.061561f, 0.135493f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.579284f, 0.209203f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.135477f, 0.076035f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.308340f, 0.460085f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.052476f, 0.139411f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019970f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019719f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.082209f, 0.732181f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.185611f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.358932f, 0.060659f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.503915f, 0.076494f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.237301f, 0.199098f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.332364f, 0.231237f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.088364f, 0.322995f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.173711f, 0.414930f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.312366f, 0.093336f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.392413f, 0.164056f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019281f, 0.018548f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.178453f, 0.229682f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.214423f, 0.359860f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.017582f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.071976f, 0.390475f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.537548f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.515147f, 0.078582f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.364623f, 0.041649f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.337054f, 0.220008f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.249141f, 0.193797f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.168802f, 0.423188f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.084285f, 0.323725f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.411061f, 0.182411f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.329651f, 0.076877f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.193953f, 0.352033f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.188543f, 0.265471f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.050266f, 0.555034f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.394700f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.179003f, 0.029987f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.700087f, 0.090924f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019171f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.099147f, 0.059028f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.470203f, 0.352451f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.075527f, 0.135452f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.184084f, 0.604937f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.136189f, 0.084874f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.576900f, 0.202037f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.041868f, 0.099347f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.343377f, 0.515408f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.044581f, 0.169532f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.062013f, 0.723875f},
};

// For each output (7x4) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_7x4[28][48] = {
{0.798509f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.201491f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.716711f, 0.085583f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.167498f, 0.030208f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.538182f, 0.218008f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.114187f, 0.070138f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020226f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020777f, 0.000000f, 0.000000f, 0.018482f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.367283f, 0.403492f, 0.000000f, 0.017972f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.071839f, 0.050645f, 0.000000f, 0.023445f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020007f, 0.000000f, 0.000000f, 0.000000f, 0.022030f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023286f, 0.000000f, 0.000000f},
{0.000000f, 0.026415f, 0.000000f, 0.000000f, 0.165810f, 0.526162f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.086343f, 0.166394f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028875f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.068792f, 0.750632f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.180576f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.798640f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.201360f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.401325f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.563541f, 0.035134f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.393109f, 0.035360f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.514780f, 0.056751f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.286324f, 0.066048f, 0.000000f, 0.022966f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.397320f, 0.167136f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024391f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018733f, 0.017081f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.228689f, 0.212401f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027812f, 0.000000f, 0.230123f, 0.251307f, 0.000000f, 0.015952f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018366f, 0.015349f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.089768f, 0.272262f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.165947f, 0.450195f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021828f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064329f, 0.394519f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021491f, 0.519661f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.420154f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.579846f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.561993f, 0.042727f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.395280f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.507366f, 0.060806f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.388432f, 0.043397f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.017057f, 0.019075f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.399856f, 0.181694f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.283918f, 0.098400f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018320f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.261768f, 0.263599f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.210680f, 0.218119f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027513f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019283f, 0.018776f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.156143f, 0.407378f, 0.000000f, 0.018410f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.081168f, 0.298842f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.043712f, 0.524648f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025861f, 0.405779f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027775f, 0.567781f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.404444f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.202734f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.797266f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.164849f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.736579f, 0.098573f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028573f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.139627f, 0.082102f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.529383f, 0.220315f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020496f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.031087f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029563f, 0.000000f, 0.069934f, 0.077745f, 0.000000f, 0.000000f, 0.000000f, 0.019031f, 0.000000f, 0.000000f, 0.369058f, 0.383087f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.072848f, 0.128566f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.206674f, 0.591912f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028891f, 0.164765f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.054845f, 0.751498f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.186782f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.813218f},
};

// For each output (8x4) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_8x4[32][48] = {
{0.800445f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.199555f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.801084f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.198916f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.802438f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.197562f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.800166f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.199834f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.808142f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.191858f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.801414f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.198586f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.798600f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.201400f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.800453f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.199547f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.415774f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.584226f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.409782f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.590218f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.407361f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.592639f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.411487f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.588513f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.416734f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.583266f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.409794f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.590206f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.409782f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.590218f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.419797f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.580203f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.588149f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.411851f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.591287f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.408713f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.587561f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.412439f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.589820f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.410180f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.585460f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.414540f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.590541f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.409459f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.587115f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.412885f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.584462f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.415538f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.200471f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.799529f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.195628f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.804372f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.195562f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.804438f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.194079f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.805921f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.205775f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.794225f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.197129f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.802871f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.193175f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.806825f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.185493f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.814507f},
};

// For each output (2x5) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_2x5[10][48] = {
{0.314987f, 0.280141f, 0.203583f, 0.129696f, 0.071593f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.085378f, 0.141565f, 0.188187f, 0.272403f, 0.312467f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.255395f, 0.217105f, 0.170584f, 0.106646f, 0.072684f, 0.000000f, 0.000000f, 0.000000f, 0.072766f, 0.046537f, 0.029920f, 0.000000f, 0.028363f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.069530f, 0.105913f, 0.164044f, 0.215260f, 0.255339f, 0.000000f, 0.000000f, 0.000000f, 0.025591f, 0.000000f, 0.036814f, 0.050349f, 0.077160f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.152274f, 0.142699f, 0.102993f, 0.080565f, 0.018558f, 0.000000f, 0.000000f, 0.000000f, 0.157267f, 0.135460f, 0.099077f, 0.089287f, 0.021820f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026396f, 0.087011f, 0.099835f, 0.143472f, 0.149274f, 0.000000f, 0.000000f, 0.000000f, 0.019143f, 0.078700f, 0.099557f, 0.143621f, 0.152993f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.071546f, 0.054560f, 0.034641f, 0.000000f, 0.026492f, 0.000000f, 0.000000f, 0.000000f, 0.253751f, 0.217970f, 0.167740f, 0.101477f, 0.071823f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.031122f, 0.000000f, 0.038539f, 0.044578f, 0.068079f, 0.000000f, 0.000000f, 0.000000f, 0.074011f, 0.104132f, 0.176778f, 0.213248f, 0.249513f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.309516f, 0.271823f, 0.202932f, 0.138334f, 0.077394f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.073235f, 0.136322f, 0.204986f, 0.270837f, 0.314620f},
};

// For each output (3x5) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_3x5[15][48] = {
{0.506870f, 0.329427f, 0.163702f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.029175f, 0.167327f, 0.319880f, 0.321166f, 0.162451f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.158719f, 0.334975f, 0.506306f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.410647f, 0.270965f, 0.135943f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.101890f, 0.048392f, 0.032162f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022675f, 0.131363f, 0.257700f, 0.263834f, 0.126043f, 0.021278f, 0.000000f, 0.000000f, 0.000000f, 0.022613f, 0.064121f, 0.066389f, 0.023985f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.131149f, 0.266568f, 0.407438f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.041342f, 0.046648f, 0.106854f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.259144f, 0.176197f, 0.070648f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.256402f, 0.170550f, 0.067060f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.085864f, 0.160352f, 0.153663f, 0.093488f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.093065f, 0.165400f, 0.162870f, 0.085298f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.069632f, 0.177258f, 0.252242f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.066495f, 0.178932f, 0.255440f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.109165f, 0.056989f, 0.043673f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.396795f, 0.263538f, 0.129840f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022525f, 0.061369f, 0.062101f, 0.020335f, 0.000000f, 0.000000f, 0.000000f, 0.022912f, 0.129308f, 0.258462f, 0.259250f, 0.129291f, 0.034446f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.042198f, 0.051815f, 0.111374f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.136459f, 0.257176f, 0.400979f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.509094f, 0.334982f, 0.155925f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.175231f, 0.321060f, 0.327712f, 0.175997f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.154955f, 0.336566f, 0.508479f},
};

// For each output (4x5) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_4x5[20][48] = {
{0.669318f, 0.330682f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.147967f, 0.437694f, 0.317636f, 0.064825f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.031879f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.060625f, 0.318845f, 0.433756f, 0.158597f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028176f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.324316f, 0.675684f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.585012f, 0.264010f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.150977f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.134170f, 0.326735f, 0.247128f, 0.055953f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.060565f, 0.080612f, 0.050606f, 0.022675f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021555f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.065736f, 0.255091f, 0.336456f, 0.141260f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020320f, 0.056879f, 0.083295f, 0.040963f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.247404f, 0.561749f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037270f, 0.153576f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.313615f, 0.178768f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.317328f, 0.167805f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022484f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.056200f, 0.226923f, 0.169203f, 0.032339f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.060880f, 0.227803f, 0.168145f, 0.036277f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022230f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020809f, 0.161103f, 0.242215f, 0.080276f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037660f, 0.170123f, 0.226083f, 0.061733f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.170517f, 0.314573f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.183677f, 0.312560f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018674f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.150066f, 0.037627f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.563093f, 0.249214f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.017288f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.047237f, 0.083719f, 0.064159f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.141594f, 0.343865f, 0.254176f, 0.047961f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.060771f, 0.083714f, 0.056548f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.055519f, 0.260450f, 0.341460f, 0.141538f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033365f, 0.158801f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.243363f, 0.564471f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027870f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.650693f, 0.321437f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.154390f, 0.455517f, 0.321763f, 0.068330f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.030540f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.067841f, 0.315774f, 0.431982f, 0.153863f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029780f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.315631f, 0.654589f},
};

// For each output (5x5) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_5x5[25][48] = {
{0.728974f, 0.241827f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029199f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.326790f, 0.583809f, 0.061650f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027751f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.474659f, 0.471971f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027161f, 0.026208f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.064479f, 0.600103f, 0.335418f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.245795f, 0.727343f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026862f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.577450f, 0.212083f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.146821f, 0.063646f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.278532f, 0.501669f, 0.039082f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.051617f, 0.129101f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.401558f, 0.402789f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.088129f, 0.087552f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019972f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.039177f, 0.470310f, 0.275467f, 0.000000f, 0.000000f, 0.000000f, 0.020182f, 0.000000f, 0.000000f, 0.131064f, 0.041994f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021806f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.201719f, 0.586252f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.071189f, 0.140839f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.390859f, 0.113288f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.395284f, 0.100569f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.180479f, 0.291419f, 0.034269f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.179460f, 0.288259f, 0.026114f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.232294f, 0.235881f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.249972f, 0.265992f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.015860f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020495f, 0.297441f, 0.200057f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.300629f, 0.181378f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.094856f, 0.384959f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.114338f, 0.382484f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023363f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.142672f, 0.067752f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.579242f, 0.210334f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.050987f, 0.132705f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.278585f, 0.484125f, 0.053597f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026554f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.092842f, 0.065201f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.385798f, 0.387342f, 0.000000f, 0.000000f, 0.021183f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021080f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020712f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.044924f, 0.106062f, 0.061499f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.047893f, 0.466019f, 0.252890f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020637f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.058939f, 0.143896f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.202796f, 0.573732f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033403f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.730809f, 0.235788f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.032140f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.330176f, 0.584667f, 0.053018f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026110f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.492274f, 0.481616f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.065854f, 0.592001f, 0.342145f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037025f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.240768f, 0.722207f},
};

// For each output (6x5) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_6x5[30][48] = {
{0.858351f, 0.111195f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.030454f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.561719f, 0.406108f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.032173f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.234049f, 0.720564f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.045387f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.699282f, 0.247085f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.053633f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.389024f, 0.574352f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.036624f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.092315f, 0.907685f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.700837f, 0.094616f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.181782f, 0.022766f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.478824f, 0.322377f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.106995f, 0.067586f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024218f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.020740f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019187f, 0.000000f, 0.211821f, 0.554939f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.076920f, 0.116393f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.528826f, 0.215423f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.129030f, 0.084167f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021007f, 0.021548f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.285851f, 0.511729f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.045516f, 0.156904f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.061737f, 0.729570f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.185199f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023495f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.426048f, 0.065346f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.437353f, 0.050722f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020531f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.015946f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.269275f, 0.220699f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.271762f, 0.222318f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.107929f, 0.387609f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.097175f, 0.384787f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022500f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018661f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.393619f, 0.098786f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.415799f, 0.073135f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.219562f, 0.256847f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.228262f, 0.295329f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.020203f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.066094f, 0.437807f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023625f, 0.426898f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025372f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.179453f, 0.029939f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.702329f, 0.088278f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024531f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.109211f, 0.062119f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.483375f, 0.320765f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.017885f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.077080f, 0.134573f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.212908f, 0.535331f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022223f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.119888f, 0.115275f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.556098f, 0.208739f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022346f, 0.116179f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.324515f, 0.536960f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.039522f, 0.193447f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.040639f, 0.726391f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033823f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.857552f, 0.108625f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024057f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029799f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.542169f, 0.403976f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.052699f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.223511f, 0.723790f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.052693f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.702269f, 0.245038f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.402547f, 0.597453f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.031996f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.086881f, 0.881123f},
};

// For each output (7x5) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_7x5[35][48] = {
{0.964445f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.035555f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.853417f, 0.094561f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.052022f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.657134f, 0.277797f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020663f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023601f, 0.000000f, 0.000000f, 0.020806f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.380325f, 0.419839f, 0.000000f, 0.023060f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.032462f, 0.000000f, 0.000000f, 0.025415f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022865f, 0.000000f, 0.028258f, 0.000000f, 0.023082f, 0.020352f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024341f, 0.000000f, 0.000000f},
{0.000000f, 0.031003f, 0.000000f, 0.000000f, 0.218422f, 0.657212f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024308f, 0.033400f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.035654f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.070868f, 0.871307f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.057825f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.964400f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.035600f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.771715f, 0.027473f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.200812f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.681017f, 0.087709f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.170219f, 0.037187f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023867f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019162f, 0.000000f, 0.019267f, 0.000000f, 0.521425f, 0.210553f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.107845f, 0.064833f, 0.000000f, 0.000000f, 0.000000f, 0.023456f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.016876f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.016582f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.374490f, 0.378533f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.037317f, 0.000000f, 0.070870f, 0.081690f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019460f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020149f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.017492f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.198514f, 0.553647f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.069444f, 0.178395f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.077267f, 0.707241f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.191176f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024316f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.777498f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.197118f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025384f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.457893f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.477045f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024793f, 0.020109f, 0.000000f, 0.020160f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.453272f, 0.036882f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.449988f, 0.037704f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022154f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.390518f, 0.119870f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.380701f, 0.108911f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.016500f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.017868f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.216278f, 0.228953f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.240939f, 0.263209f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.016253f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029917f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.096934f, 0.340899f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.088970f, 0.426562f, 0.000000f, 0.000000f, 0.016718f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021872f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.073754f, 0.459232f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.422925f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022217f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.019775f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.473981f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020534f, 0.461485f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024225f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.200471f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.772740f, 0.026789f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025642f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.165170f, 0.033854f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.660678f, 0.089428f, 0.000000f, 0.000000f, 0.000000f, 0.025229f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.016453f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.117847f, 0.083344f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.528281f, 0.230342f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023732f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.043833f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.077971f, 0.049154f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.382849f, 0.385195f, 0.000000f, 0.022790f, 0.000000f, 0.000000f, 0.020308f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.017900f},
{0.000000f, 0.000000f, 0.018444f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.017477f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.086693f, 0.093631f, 0.000000f, 0.032653f, 0.000000f, 0.000000f, 0.019144f, 0.000000f, 0.199637f, 0.532319f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020247f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.035464f, 0.208022f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.065940f, 0.670327f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.209616f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.790384f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.036613f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.963387f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.046570f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.849248f, 0.104183f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020833f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.049999f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.649521f, 0.279647f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.030284f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025099f, 0.000000f, 0.000000f, 0.017993f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028953f, 0.000000f, 0.027848f, 0.031988f, 0.000000f, 0.000000f, 0.000000f, 0.022049f, 0.000000f, 0.000000f, 0.397216f, 0.418570f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026723f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.038960f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.243424f, 0.690894f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.050705f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.071869f, 0.877426f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.036401f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.963599f},
};

// For each output (8x5) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_8x5[40][48] = {
{0.966296f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033704f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.966306f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033694f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.966296f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033704f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.966298f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033702f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966291f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033709f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966291f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033709f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966295f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033705f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966296f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033704f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.793476f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.206524f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.803849f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.196151f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.803624f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.196376f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.797993f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.202007f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.776552f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.195983f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027465f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.793721f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.206279f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.806466f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.193534f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.797656f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.202344f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.476380f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.496730f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026890f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.490205f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.485068f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024727f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.498077f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.476651f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025272f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.474340f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.480228f, 0.045432f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.478505f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.521495f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.478679f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.483579f, 0.000000f, 0.000000f, 0.037742f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.521456f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.478544f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.507379f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.492621f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.204896f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.795104f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.196765f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.803235f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.199650f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.800350f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.203568f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.796432f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.179104f, 0.025788f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.795108f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.198542f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.801458f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.212749f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.787251f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.210279f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.789721f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033704f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966296f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033709f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966291f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033700f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966300f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033711f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966289f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033705f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966295f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033692f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966308f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033717f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966283f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033731f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.966269f},
};

// For each output (2x6) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_2x6[12][48] = {
{0.316864f, 0.281020f, 0.203578f, 0.128737f, 0.069800f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.084099f, 0.140260f, 0.188810f, 0.272909f, 0.313922f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.309774f, 0.274434f, 0.201401f, 0.144203f, 0.070188f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.065514f, 0.142636f, 0.201399f, 0.276345f, 0.314107f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.317592f, 0.277500f, 0.192959f, 0.141457f, 0.070491f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.073241f, 0.142588f, 0.198561f, 0.278233f, 0.307377f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.320020f, 0.275328f, 0.193983f, 0.143663f, 0.067007f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.069519f, 0.132193f, 0.205168f, 0.279209f, 0.313912f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.314759f, 0.279613f, 0.202284f, 0.130432f, 0.072912f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.077965f, 0.136688f, 0.207007f, 0.271208f, 0.307132f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.311744f, 0.272206f, 0.202758f, 0.136022f, 0.077269f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.072611f, 0.134437f, 0.204577f, 0.271631f, 0.316744f},
};

// For each output (3x6) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_3x6[18][48] = {
{0.509323f, 0.329513f, 0.161164f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.025207f, 0.165943f, 0.323432f, 0.324818f, 0.160600f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.157414f, 0.335022f, 0.507564f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.511584f, 0.329744f, 0.158672f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.031983f, 0.159222f, 0.310218f, 0.312506f, 0.158287f, 0.027785f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.156210f, 0.333357f, 0.510434f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.515123f, 0.331176f, 0.153701f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026619f, 0.155693f, 0.312956f, 0.312469f, 0.159059f, 0.033204f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.156669f, 0.330733f, 0.512598f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.503816f, 0.332794f, 0.163390f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024597f, 0.154193f, 0.318347f, 0.305757f, 0.159499f, 0.037605f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.158978f, 0.332267f, 0.508755f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.512301f, 0.329905f, 0.157794f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.034639f, 0.152702f, 0.307204f, 0.309309f, 0.167621f, 0.028524f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.152238f, 0.331031f, 0.516731f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.511179f, 0.335760f, 0.153061f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.173463f, 0.322489f, 0.329811f, 0.174238f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.152159f, 0.337011f, 0.510830f},
};

// For each output (4x6) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_4x6[24][48] = {
{0.671100f, 0.328900f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.148979f, 0.456693f, 0.330185f, 0.064143f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.058158f, 0.330805f, 0.451065f, 0.159972f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.322150f, 0.677850f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.677593f, 0.322407f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.167723f, 0.446276f, 0.319975f, 0.066025f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.073990f, 0.323047f, 0.441943f, 0.161020f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.326071f, 0.673929f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.679042f, 0.320958f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.152853f, 0.450375f, 0.323919f, 0.072853f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.061203f, 0.320863f, 0.451270f, 0.166664f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.319746f, 0.680254f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.676510f, 0.323490f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.162624f, 0.457726f, 0.332137f, 0.047514f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063329f, 0.328068f, 0.444798f, 0.163805f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.320574f, 0.679426f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.678066f, 0.321934f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.166497f, 0.448536f, 0.320669f, 0.064298f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.065578f, 0.323791f, 0.452649f, 0.157982f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.322175f, 0.677825f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.671500f, 0.328500f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.150795f, 0.460955f, 0.323971f, 0.064280f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.066061f, 0.327767f, 0.449877f, 0.156295f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.322687f, 0.677313f},
};

// For each output (5x6) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_5x6[30][48] = {
{0.754364f, 0.245636f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.335285f, 0.602164f, 0.062551f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.500479f, 0.499521f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.057582f, 0.607199f, 0.335218f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.249634f, 0.750366f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.757244f, 0.242756f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.346204f, 0.598435f, 0.055362f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.501490f, 0.498510f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.060219f, 0.591314f, 0.348467f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.244713f, 0.755287f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.752634f, 0.247366f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.342331f, 0.595920f, 0.061748f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.496285f, 0.503715f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.055875f, 0.601113f, 0.343013f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.245684f, 0.754316f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.754642f, 0.245358f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.341881f, 0.605457f, 0.052662f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.506471f, 0.493529f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.052276f, 0.594038f, 0.353686f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.243659f, 0.756341f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.752998f, 0.247002f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.343161f, 0.587149f, 0.069691f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.497737f, 0.502263f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.068745f, 0.600800f, 0.330455f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.249755f, 0.750245f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.760155f, 0.239845f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.341132f, 0.607027f, 0.051841f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.505602f, 0.494398f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.063784f, 0.594541f, 0.341675f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.246784f, 0.753216f},
};

// For each output (6x6) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_6x6[36][48] = {
{0.891095f, 0.108905f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.581832f, 0.418168f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.242153f, 0.757847f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.741976f, 0.258024f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.403606f, 0.596394f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.087517f, 0.912483f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.889771f, 0.110229f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.562123f, 0.416930f, 0.000000f, 0.020947f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.239798f, 0.760202f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.745430f, 0.254570f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.386117f, 0.613883f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.079820f, 0.920180f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.881826f, 0.118174f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.573611f, 0.426389f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.253276f, 0.746724f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.743647f, 0.256353f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.401870f, 0.598130f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.084584f, 0.915416f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.886496f, 0.113504f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.579329f, 0.420671f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.247079f, 0.752921f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.738480f, 0.261520f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.387849f, 0.612151f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.084296f, 0.915704f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.887045f, 0.112955f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.566292f, 0.413182f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020526f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.245603f, 0.754397f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.743664f, 0.256336f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.400389f, 0.599611f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.085951f, 0.914049f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.893377f, 0.106623f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023576f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.559870f, 0.416555f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.230693f, 0.769307f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.743815f, 0.256185f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.401590f, 0.598410f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.084902f, 0.915098f},
};

// For each output (7x6) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_7x6[42][48] = {
{1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.898749f, 0.101251f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.666832f, 0.285944f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024418f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.022807f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.408751f, 0.452880f, 0.000000f, 0.022279f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020101f, 0.000000f, 0.026406f, 0.000000f, 0.021392f, 0.021638f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026554f, 0.000000f, 0.000000f},
{0.000000f, 0.030824f, 0.000000f, 0.000000f, 0.224222f, 0.683355f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.025094f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.036505f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.074156f, 0.925844f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.898226f, 0.101774f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026159f, 0.000000f, 0.029283f, 0.000000f, 0.659538f, 0.261285f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023736f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.465472f, 0.460934f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.047490f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026105f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.265328f, 0.711299f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023373f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.070803f, 0.929197f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.898251f, 0.101749f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.031089f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.687826f, 0.256342f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024743f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.032537f, 0.022458f, 0.000000f, 0.000000f, 0.021138f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.410489f, 0.430095f, 0.000000f, 0.017967f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018781f, 0.000000f, 0.000000f, 0.026567f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019968f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.246938f, 0.753062f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.075058f, 0.924942f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.901297f, 0.098703f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.030068f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.643429f, 0.251859f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020470f, 0.024955f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.029220f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.018980f, 0.000000f, 0.000000f, 0.026284f, 0.019861f, 0.000000f, 0.028010f, 0.000000f, 0.000000f, 0.000000f, 0.445381f, 0.461483f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.238827f, 0.737044f, 0.000000f, 0.024129f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.068567f, 0.931433f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.900397f, 0.099603f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.026865f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.019850f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.657029f, 0.271313f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.024943f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.033697f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.020611f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026549f, 0.000000f, 0.440563f, 0.453523f, 0.000000f, 0.025057f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.030966f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.255214f, 0.713821f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.075429f, 0.924571f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.889492f, 0.110508f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.021347f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.683322f, 0.295331f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.027842f, 0.000000f, 0.000000f, 0.031873f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.028367f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.023945f, 0.000000f, 0.000000f, 0.417773f, 0.443199f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.026004f, 0.000000f, 0.000000f, 0.024319f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.243621f, 0.706056f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.075547f, 0.924453f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f},
};

// For each output (8x6) sample, the weight of each input (8x6) sample.
static const float g_weight_downsample_8x6_to_8x6[48][48] = {
{1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f, 0.000000f},
{0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 0.000000f, 1.000000f},
};

downsample_matrix g_downsample_matrices_8x6[] =
{
	{ 2, 2, (const float*)g_weight_downsample_8x6_to_2x2 },
	{ 3, 2, (const float*)g_weight_downsample_8x6_to_3x2 },
	{ 4, 2, (const float*)g_weight_downsample_8x6_to_4x2 },
	{ 5, 2, (const float*)g_weight_downsample_8x6_to_5x2 },
	{ 6, 2, (const float*)g_weight_downsample_8x6_to_6x2 },
	{ 7, 2, (const float*)g_weight_downsample_8x6_to_7x2 },
	{ 8, 2, (const float*)g_weight_downsample_8x6_to_8x2 },
	{ 2, 3, (const float*)g_weight_downsample_8x6_to_2x3 },
	{ 3, 3, (const float*)g_weight_downsample_8x6_to_3x3 },
	{ 4, 3, (const float*)g_weight_downsample_8x6_to_4x3 },
	{ 5, 3, (const float*)g_weight_downsample_8x6_to_5x3 },
	{ 6, 3, (const float*)g_weight_downsample_8x6_to_6x3 },
	{ 7, 3, (const float*)g_weight_downsample_8x6_to_7x3 },
	{ 8, 3, (const float*)g_weight_downsample_8x6_to_8x3 },
	{ 2, 4, (const float*)g_weight_downsample_8x6_to_2x4 },
	{ 3, 4, (const float*)g_weight_downsample_8x6_to_3x4 },
	{ 4, 4, (const float*)g_weight_downsample_8x6_to_4x4 },
	{ 5, 4, (const float*)g_weight_downsample_8x6_to_5x4 },
	{ 6, 4, (const float*)g_weight_downsample_8x6_to_6x4 },
	{ 7, 4, (const float*)g_weight_downsample_8x6_to_7x4 },
	{ 8, 4, (const float*)g_weight_downsample_8x6_to_8x4 },
	{ 2, 5, (const float*)g_weight_downsample_8x6_to_2x5 },
	{ 3, 5, (const float*)g_weight_downsample_8x6_to_3x5 },
	{ 4, 5, (const float*)g_weight_downsample_8x6_to_4x5 },
	{ 5, 5, (const float*)g_weight_downsample_8x6_to_5x5 },
	{ 6, 5, (const float*)g_weight_downsample_8x6_to_6x5 },
	{ 7, 5, (const float*)g_weight_downsample_8x6_to_7x5 },
	{ 8, 5, (const float*)g_weight_downsample_8x6_to_8x5 },
	{ 2, 6, (const float*)g_weight_downsample_8x6_to_2x6 },
	{ 3, 6, (const float*)g_weight_downsample_8x6_to_3x6 },
	{ 4, 6, (const float*)g_weight_downsample_8x6_to_4x6 },
	{ 5, 6, (const float*)g_weight_downsample_8x6_to_5x6 },
	{ 6, 6, (const float*)g_weight_downsample_8x6_to_6x6 },
	{ 7, 6, (const float*)g_weight_downsample_8x6_to_7x6 },
	{ 8, 6, (const float*)g_weight_downsample_8x6_to_8x6 }
};

//--------------------------------------------------------------------------------------------------------------------------

const float* get_6x6_downsample_matrix(uint32_t grid_width, uint32_t grid_height)
{
	// TODO: Use hash or map lookup, or calc the index directly
	for (const auto& m : g_downsample_matrices_6x6)
		if ((m.m_grid_width == grid_width) && (m.m_grid_height == grid_height))
			return m.m_p;

	assert(0);
	return nullptr;
}

const float* get_8x6_downsample_matrix(uint32_t grid_width, uint32_t grid_height)
{
	// TODO: Use hash or map lookup, or calc the index directly
	for (const auto& m : g_downsample_matrices_8x6)
		if ((m.m_grid_width == grid_width) && (m.m_grid_height == grid_height))
			return m.m_p;

	assert(0);
	return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------------

void compute_upsample_matrix(basisu::vector2D<float>& upsample_matrix, uint32_t block_width, uint32_t block_height, uint32_t grid_width, uint32_t grid_height)
{
	assert((block_width >= 2) && (block_width <= astc_helpers::MAX_BLOCK_DIM));
	assert((block_height >= 2) && (block_height <= astc_helpers::MAX_BLOCK_DIM));
	assert((grid_width >= 2) && (grid_width <= block_width));
	assert((grid_height >= 2) && (grid_height <= block_height));

	const uint32_t num_block_samples = block_width * block_height;
	const uint32_t num_grid_samples = grid_width * grid_height;

	astc_helpers::weighted_sample samples[astc_helpers::MAX_BLOCK_DIM * astc_helpers::MAX_BLOCK_DIM];
	clear_obj(samples);

	astc_helpers::compute_upsample_weights(block_width, block_height, grid_width, grid_height, samples);

	// Compute upsample matrix: output num_block_samples (rows), input num_grid_samples (cols)
	upsample_matrix.resize_rows_cols(num_block_samples, num_grid_samples);

	basisu::vector<float> weights(num_grid_samples);

	// compute which source sample(s) contribute to it.
	for (uint32_t d = 0; d < num_block_samples; d++)
	{
		const astc_helpers::weighted_sample& ws = samples[d];
				
		weights.set_all(0.0f);

		for (uint32_t y = 0; y < 2; y++)
		{
			for (uint32_t x = 0; x < 2; x++)
			{
				float w = ws.m_weights[y][x] * (1.0f / 16.0f);
				if (!w)
					continue;

				assert((ws.m_src_x + x) < grid_width);
				assert((ws.m_src_y + y) < grid_height);

				assert(weights[(ws.m_src_x + x) + (ws.m_src_y + y) * grid_width] == 0.0f);
				weights[(ws.m_src_x + x) + (ws.m_src_y + y) * grid_width] = w;
			} //  x
		} // y

		for (uint32_t i = 0; i < num_grid_samples; i++)
			upsample_matrix.at_row_col(d, i) = weights[i];

	} // d
}

//--------------------------------------------------------------------------------------------------------------------------
// compute At - used for gradient descent

void compute_upsample_matrix_transposed(basisu::vector<float>& unweighted_downsample_matrix, uint32_t block_width, uint32_t block_height, uint32_t grid_width, uint32_t grid_height)
{
	assert((block_width >= 2) && (block_width <= astc_helpers::MAX_BLOCK_DIM));
	assert((block_height >= 2) && (block_height <= astc_helpers::MAX_BLOCK_DIM));
	assert((grid_width >= 2) && (grid_width <= block_width));
	assert((grid_height >= 2) && (grid_height <= block_height));

	const uint32_t num_block_samples = block_width * block_height;
	const uint32_t num_grid_samples = grid_width * grid_height;

	// Compute upsample matrix: output num_block_samples (rows), input num_grid_samples (cols)
	vector2D<float> upsample_matrix;
	compute_upsample_matrix(upsample_matrix, block_width, block_height, grid_width, grid_height);
		
	// downsample matrix At (without any scaling): num_grid_samples (rows), num_block_samples (cols)
	unweighted_downsample_matrix.resize(num_grid_samples * num_block_samples);
	unweighted_downsample_matrix.set_all(0.0f);

	for (uint32_t j = 0; j < num_grid_samples; ++j)
		for (uint32_t i = 0; i < num_block_samples; ++i)
			unweighted_downsample_matrix[j * num_block_samples + i] = upsample_matrix.at_row_col(i, j);
}

//--------------------------------------------------------------------------------------------------------------------------
// Computes downsample matrices - simpler alternative to SLSQP

//--------------------------------------------------------------------------------------------------------------------------
// pDst_vec[] - size must be >= num_grid_samples 
// vector used for gradient descent

void compute_diag_AtA_vector(uint32_t block_width, uint32_t block_height, uint32_t grid_width, uint32_t grid_height, const vector2D<float> &upsample_matrix, float* pDst_vec)
{
	const uint32_t num_block_samples = block_width * block_height;
	const uint32_t num_grid_samples = grid_width * grid_height;

	memset(pDst_vec, 0, sizeof(float) * num_grid_samples);

	for (uint32_t r = 0; r < num_block_samples; ++r)
	{
		for (uint32_t c = 0; c < num_grid_samples; ++c)
		{
			const float arc = upsample_matrix.at_row_col(r, c);

			pDst_vec[c] += arc * arc;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------------

void downsample_weight_grid(
	const float* pMatrix_weights,
	uint32_t bx, uint32_t by,		// source/from dimension (block size)
	uint32_t wx, uint32_t wy,		// dest/to dimension (grid size)
	const uint8_t* pSrc_weights,	// these are dequantized weights, NOT ISE symbols, [by][bx]
	uint8_t* pDst_weights)			// [wy][wx]
{
	const uint32_t total_block_samples = bx * by;

	for (uint32_t y = 0; y < wy; y++)
	{
		for (uint32_t x = 0; x < wx; x++)
		{
			float total = 0.5f;

			for (uint32_t i = 0; i < total_block_samples; i++)
				if (pMatrix_weights[i])
					total += pMatrix_weights[i] * (float)pSrc_weights[i];

			pDst_weights[x + y * wx] = (uint8_t)clamp((int)total, 0, 64);

			pMatrix_weights += total_block_samples;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------------

void downsample_ise_weights(
	uint32_t dequant_weight_ise_range, uint32_t quant_weight_ise_range,
	uint32_t block_w, uint32_t block_h,
	uint32_t grid_w, uint32_t grid_h,
	const uint8_t* pSrc_weights, uint8_t* pDst_weights)
{
	assert((block_w <= MAX_ASTC_HDR_BLOCK_W) && (block_h <= MAX_ASTC_HDR_BLOCK_H));
	assert((grid_w >= 2) && (grid_w <= MAX_ASTC_HDR_BLOCK_W));
	assert((grid_h >= 2) && (grid_h <= MAX_ASTC_HDR_BLOCK_H));
	
	assert(dequant_weight_ise_range >= astc_helpers::FIRST_VALID_WEIGHT_ISE_RANGE);
	assert(dequant_weight_ise_range <= astc_helpers::LAST_VALID_WEIGHT_ISE_RANGE);

	assert(quant_weight_ise_range >= astc_helpers::FIRST_VALID_WEIGHT_ISE_RANGE);
	assert(quant_weight_ise_range <= astc_helpers::LAST_VALID_WEIGHT_ISE_RANGE);

	if ((block_w == grid_w) && (block_h == grid_h))
	{
		if (dequant_weight_ise_range != quant_weight_ise_range)
		{
			basist::astc_6x6_hdr::requantize_astc_weights(block_w * block_h, pSrc_weights, dequant_weight_ise_range, pDst_weights, quant_weight_ise_range);
		}
		else
		{
			if (pDst_weights != pSrc_weights)
				memcpy(pDst_weights, pSrc_weights, block_w * block_h);
		}

		return;
	}

	uint8_t desired_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];

	const auto& dequant_tab = astc_helpers::g_dequant_tables.get_weight_tab(dequant_weight_ise_range).m_ISE_to_val;

	for (uint32_t by = 0; by < block_h; by++)
		for (uint32_t bx = 0; bx < block_w; bx++)
			desired_weights[bx + by * block_w] = dequant_tab[pSrc_weights[bx + by * block_w]];

	uint8_t downsampled_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];

	const float* pDownsample_matrix = get_6x6_downsample_matrix(grid_w, grid_h);
	assert(pDownsample_matrix);

	downsample_weight_grid(
		pDownsample_matrix,
		block_w, block_h,		// source/from dimension (block size)
		grid_w, grid_h,			// dest/to dimension (grid size)
		desired_weights,		// these are dequantized weights, NOT ISE symbols, [by][bx]
		downsampled_weights);	// [wy][wx]

	const auto& weight_quant_tab = astc_helpers::g_dequant_tables.get_weight_tab(quant_weight_ise_range).m_val_to_ise;

	for (uint32_t gy = 0; gy < grid_h; gy++)
		for (uint32_t gx = 0; gx < grid_w; gx++)
			pDst_weights[gx + gy * grid_w] = weight_quant_tab[downsampled_weights[gx + gy * grid_w]];
}

void downsample_ise_weights_dual_plane(
	uint32_t dequant_weight_ise_range, uint32_t quant_weight_ise_range,
	uint32_t block_w, uint32_t block_h,
	uint32_t grid_w, uint32_t grid_h,
	const uint8_t* pSrc_weights0, const uint8_t* pSrc_weights1,
	uint8_t* pDst_weights)
{
	uint8_t downsampled_weights0[MAX_ASTC_HDR_BLOCK_W * MAX_ASTC_HDR_BLOCK_H], downsampled_weights1[MAX_ASTC_HDR_BLOCK_W * MAX_ASTC_HDR_BLOCK_H];

	downsample_ise_weights(
		dequant_weight_ise_range, quant_weight_ise_range,
		block_w, block_h,
		grid_w, grid_h,
		pSrc_weights0, downsampled_weights0);

	downsample_ise_weights(
		dequant_weight_ise_range, quant_weight_ise_range,
		block_w, block_h,
		grid_w, grid_h,
		pSrc_weights1, downsampled_weights1);

	const uint32_t num_grid_samples = grid_w * grid_h;
	for (uint32_t i = 0; i < num_grid_samples; i++)
	{
		pDst_weights[i * 2 + 0] = downsampled_weights0[i];
		pDst_weights[i * 2 + 1] = downsampled_weights1[i];
	}
}

static bool refine_endpoints_mode11(
	uint32_t endpoint_ise_range,
	uint8_t* pEndpoint_vals, // the endpoints to optimize
	uint32_t block_w, uint32_t block_h, // block dimensions
	uint32_t grid_w, uint32_t grid_h, const uint8_t* pWeights, uint32_t weight_ise_range, // weight grid
	uint32_t num_pixels, const basist::half_float pBlock_pixels_half[][3], const vec4F pBlock_pixels_q16[],
	const uint8_t* pPixel_block_ofs, // maps this subset's pixels to block offsets
	astc_hdr_codec_base_options& coptions,
	bool direct_only, int first_submode, int last_submode,
	opt_mode_t opt_mode)
{
	if (opt_mode == cNoOpt)
		return false;

	const uint32_t num_block_pixels = block_w * block_h;

	uint8_t def_pixel_block_ofs[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
	if (!pPixel_block_ofs)
	{
		for (uint32_t i = 0; i < num_block_pixels; i++)
			def_pixel_block_ofs[i] = (uint8_t)i;
		
		pPixel_block_ofs = def_pixel_block_ofs;
	}

	const uint32_t num_weights = grid_w * grid_h;

	uint8_t dequantized_raw_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
	for (uint32_t i = 0; i < num_weights; i++)
		dequantized_raw_weights[i] = astc_helpers::g_dequant_tables.get_weight_tab(weight_ise_range).m_ISE_to_val[pWeights[i]];

	uint8_t upsampled_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS]; // raw weights, NOT ISE
	astc_helpers::upsample_weight_grid(block_w, block_h, grid_w, grid_h, dequantized_raw_weights, upsampled_weights);

	aabb3F color_box_q16(cInitExpand);

	uint8_t trial_blk_raw_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS]; // raw weights, NOT ISE
	float trial_blk_raw_weightsf[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
	for (uint32_t i = 0; i < num_pixels; i++)
	{
		color_box_q16.expand(pBlock_pixels_q16[i]);

		assert(pPixel_block_ofs[i] < num_block_pixels);

		trial_blk_raw_weights[i] = upsampled_weights[pPixel_block_ofs[i]];
		trial_blk_raw_weightsf[i] = (float)trial_blk_raw_weights[i] * (1.0f / 64.0f);
	}
	
	vec3F l_q16, h_q16;
	if (opt_mode == cOrdinaryLeastSquares)
	{
		if (!compute_least_squares_endpoints_rgb_raw_weights(num_pixels, trial_blk_raw_weights, &l_q16, &h_q16, pBlock_pixels_q16, color_box_q16))
			return false;
	}
	else if ((opt_mode == cWeightedLeastSquares) || (opt_mode == cWeightedLeastSquaresHeavy))
	{
		vec3F block_mean_color_q16(calc_mean(num_pixels, pBlock_pixels_q16));
		vec3F block_axis_q16(calc_rgb_pca(num_pixels, pBlock_pixels_q16, block_mean_color_q16));
		float l = BIG_FLOAT_VAL, h = -BIG_FLOAT_VAL;
		for (uint32_t i = 0; i < num_pixels; i++)
		{
			vec3F k(vec3F(pBlock_pixels_q16[i]) - block_mean_color_q16);
			float kd = k.dot(block_axis_q16);
			if (kd < l)
				l = kd;
			if (kd > h)
				h = kd;
		}
		float emphasis_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
		if (h == l)
		{
			for (uint32_t i = 0; i < num_pixels; i++)
				emphasis_weights[i] = 1.0f;
		}
		else
		{
			float mid = (0.0f - l) / (h - l);
			mid = clamp(mid, .01f, .99f);
				
			float lw = LOW_EMPHASIS_WEIGHT, mw = MIDDLE_EMPHASIS_WEIGHT, hw = HIGH_EMPHASIS_WEIGHT;
			if (opt_mode == cWeightedLeastSquaresHeavy)
				lw = LOW_EMPHASIS_WEIGHT_HEAVY, mw = MIDDLE_EMPHASIS_WEIGHT_HEAVY, hw = HIGH_EMPHASIS_WEIGHT_HEAVY;

			for (uint32_t i = 0; i < num_pixels; i++)
			{
				vec3F k(vec3F(pBlock_pixels_q16[i]) - block_mean_color_q16);
				float kd = k.dot(block_axis_q16);

				assert((kd >= l) && (kd <= h));

				float v = (kd - l) / (h - l);

				if (v < mid)
					v = lerp(lw, mw, v / mid);
				else
					v = lerp(mw, hw, (v - mid) * (1.0f - mid));

				emphasis_weights[i] = v;
			}
		}

		if (!compute_weighted_least_squares_endpoints_rgb(num_pixels, nullptr, nullptr, trial_blk_raw_weightsf, emphasis_weights, &l_q16, &h_q16, pBlock_pixels_q16, color_box_q16))
			return false;
	}
	else
	{
		assert(opt_mode == cWeightedAverage);

		l_q16.set(0.0f);
		float total_low = 0.0f;

		h_q16.set(0.0f);
		float total_high = 0.0f;

		for (uint32_t i = 0; i < num_pixels; i++)
		{
			vec3F p(pBlock_pixels_q16[i]);
			float lerp = (float)trial_blk_raw_weights[i] * (1.0f / 64.0f);

			l_q16 += p * (1.0f - lerp);
			total_low += (1.0f - lerp);

			h_q16 += p * lerp;
			total_high += lerp;
		}

		if (total_low != 0.0f)
			l_q16 *= (1.0f / total_low);
		else
			return false;

		if (total_high != 0.0f)
			h_q16 *= (1.0f / total_high);
		else
			return false;
	}

	uint8_t trial_endpoints[NUM_MODE11_ENDPOINTS];
	
	uint32_t submode_used;

	bool pack_succeeded = pack_mode11(l_q16, h_q16, endpoint_ise_range, trial_endpoints, coptions, direct_only, first_submode, last_submode, false, submode_used);
	if (!pack_succeeded)
		return false;

	int cur_e[2][3];
	if (!decode_mode11_to_qlog12(pEndpoint_vals, cur_e, endpoint_ise_range))
		return false;

	int trial_e[2][3];
	if (!decode_mode11_to_qlog12(trial_endpoints, trial_e, endpoint_ise_range))
		return false;

	for (uint32_t i = 0; i < 3; i++)
	{
		cur_e[0][i] <<= 4;
		cur_e[1][i] <<= 4;

		trial_e[0][i] <<= 4;
		trial_e[1][i] <<= 4;
	}

	const float R_WEIGHT = coptions.m_r_err_scale, G_WEIGHT = coptions.m_g_err_scale;

	double cur_error = 0, trial_error = 0;
		
	for (uint32_t p = 0; p < num_pixels; p++)
	{
		const half_float* pDesired_half = &pBlock_pixels_half[p][0];

		const double desired_half_r_q = q(pDesired_half[0], coptions.m_q_log_bias), desired_half_g_q = q(pDesired_half[1], coptions.m_q_log_bias), desired_half_b_q = q(pDesired_half[2], coptions.m_q_log_bias);

		const uint32_t c = trial_blk_raw_weights[p];
		assert(c <= 64);

		{
			half_float rf, gf, bf;

			{
				uint32_t r0 = cur_e[0][0], r1 = cur_e[1][0];
				int ri = (r0 * (64 - c) + r1 * c + 32) / 64;
				rf = astc_helpers::qlog16_to_half(ri);
			}

			{
				uint32_t g0 = cur_e[0][1], g1 = cur_e[1][1];
				int gi = (g0 * (64 - c) + g1 * c + 32) / 64;
				gf = astc_helpers::qlog16_to_half(gi);
			}

			{
				uint32_t b0 = cur_e[0][2], b1 = cur_e[1][2];
				int bi = (b0 * (64 - c) + b1 * c + 32) / 64;
				bf = astc_helpers::qlog16_to_half(bi);
			}

			const double decoded_half_q0 = q(rf, coptions.m_q_log_bias), decoded_half_q1 = q(gf, coptions.m_q_log_bias), decoded_half_q2 = q(bf, coptions.m_q_log_bias);

			const double rd = decoded_half_q0 - desired_half_r_q, gd = decoded_half_q1 - desired_half_g_q, bd = decoded_half_q2 - desired_half_b_q;

			cur_error += R_WEIGHT * (rd * rd) + G_WEIGHT * (gd * gd) + bd * bd;
		}

		{
			half_float rf, gf, bf;

			{
				uint32_t r0 = trial_e[0][0], r1 = trial_e[1][0];
				int ri = (r0 * (64 - c) + r1 * c + 32) / 64;
				rf = astc_helpers::qlog16_to_half(ri);
			}

			{
				uint32_t g0 = trial_e[0][1], g1 = trial_e[1][1];
				int gi = (g0 * (64 - c) + g1 * c + 32) / 64;
				gf = astc_helpers::qlog16_to_half(gi);
			}

			{
				uint32_t b0 = trial_e[0][2], b1 = trial_e[1][2];
				int bi = (b0 * (64 - c) + b1 * c + 32) / 64;
				bf = astc_helpers::qlog16_to_half(bi);
			}

			const double decoded_half_q0 = q(rf, coptions.m_q_log_bias), decoded_half_q1 = q(gf, coptions.m_q_log_bias), decoded_half_q2 = q(bf, coptions.m_q_log_bias);

			const double rd = decoded_half_q0 - desired_half_r_q, gd = decoded_half_q1 - desired_half_g_q, bd = decoded_half_q2 - desired_half_b_q;

			trial_error += R_WEIGHT * (rd * rd) + G_WEIGHT * (gd * gd) + bd * bd;
		}

	} // p

	if (trial_error < cur_error)
	{
		memcpy(pEndpoint_vals, trial_endpoints, NUM_MODE11_ENDPOINTS);
		return true;
	}

	return false;
}

static bool refine_endpoints_mode7(
	uint32_t endpoint_ise_range,
	uint8_t* pEndpoint_vals, // the endpoints to optimize
	uint32_t block_w, uint32_t block_h, // block dimensions
	uint32_t grid_w, uint32_t grid_h, const uint8_t* pWeights, uint32_t weight_ise_range, // weight grid
	uint32_t num_pixels, const basist::half_float pBlock_pixels_half[][3], const vec4F pBlock_pixels_q16[],
	const uint8_t* pPixel_block_ofs, // maps this subset's pixels to block offsets
	astc_hdr_codec_base_options& coptions,
	int first_submode, int last_submode)
{
	const uint32_t num_block_pixels = block_w * block_h;

	uint8_t def_pixel_block_ofs[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
	if (!pPixel_block_ofs)
	{
		for (uint32_t i = 0; i < num_block_pixels; i++)
			def_pixel_block_ofs[i] = (uint8_t)i;

		pPixel_block_ofs = def_pixel_block_ofs;
	}

	const uint32_t num_weights = grid_w * grid_h;

	uint8_t dequantized_raw_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS];
	for (uint32_t i = 0; i < num_weights; i++)
		dequantized_raw_weights[i] = astc_helpers::g_dequant_tables.get_weight_tab(weight_ise_range).m_ISE_to_val[pWeights[i]];

	uint8_t upsampled_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS]; // raw weights, NOT ISE
	astc_helpers::upsample_weight_grid(block_w, block_h, grid_w, grid_h, dequantized_raw_weights, upsampled_weights);

	uint8_t trial_blk_raw_weights[MAX_ASTC_HDR_ENC_BLOCK_PIXELS]; // raw weights, NOT ISE
	for (uint32_t i = 0; i < num_pixels; i++)
	{
		assert(pPixel_block_ofs[i] < num_block_pixels);

		trial_blk_raw_weights[i] = upsampled_weights[pPixel_block_ofs[i]];
	}

	//--

	int cur_e[2][3];
	int cur_s = 0;
	if (!decode_mode7_to_qlog12(pEndpoint_vals, cur_e, &cur_s, endpoint_ise_range))
		return false;

	cur_s <<= 4;

	vec3F block_mean_color_q16(calc_mean(num_pixels, pBlock_pixels_q16));

	vec3F new_high_color_q16(block_mean_color_q16);
		
	const float one_over_num_pixels = 1.0f / (float)num_pixels;

	for (uint32_t i = 0; i < num_pixels; i++)
	{
		float lerp = trial_blk_raw_weights[i] * (1.0f / 64.0f);

		float k = (float)cur_s * (1.0f - lerp) * one_over_num_pixels;
		new_high_color_q16[0] += k;
		new_high_color_q16[1] += k;
		new_high_color_q16[2] += k;
	}
					
	// Given a set of selectors and a high color, try to compute a better S.
	float t = 0.0f;

	for (uint32_t i = 0; i < num_pixels; i++)
	{
		float lerp = trial_blk_raw_weights[i] * (1.0f / 64.0f);

		t += (1.0f) - lerp;
	}

	t *= one_over_num_pixels;
	
	if (fabs(t) < .0000125f)
		return false;

	uint8_t trial_endpoints[NUM_MODE7_ENDPOINTS];

	uint32_t submode_used;
	if (!pack_mode7(new_high_color_q16, (float)cur_s, endpoint_ise_range, trial_endpoints, weight_ise_range, coptions, first_submode, last_submode, false, submode_used))
		return false;

	int trial_e[2][3];
	if (!decode_mode7_to_qlog12(trial_endpoints, trial_e, nullptr, endpoint_ise_range))
		return false;

	vec3F cur_h_q16((float)(trial_e[1][0] << 4), (float)(trial_e[1][1] << 4), (float)(trial_e[1][2] << 4));

	float s_r = (cur_h_q16[0] - block_mean_color_q16[0]) / t;
	//float s_g = (cur_h_q16[1] - block_mean_color_q16[1]) / t;
	//float s_b = (cur_h_q16[2] - block_mean_color_q16[2]) / t;
	float new_s_q16 = ceilf(s_r);

	if (!pack_mode7(new_high_color_q16, new_s_q16, endpoint_ise_range, trial_endpoints, weight_ise_range, coptions, first_submode, last_submode, false, submode_used))
		return false;

	if (!decode_mode7_to_qlog12(trial_endpoints, trial_e, nullptr, endpoint_ise_range))
		return false;
	
	// --

	for (uint32_t i = 0; i < 3; i++)
	{
		cur_e[0][i] <<= 4;
		cur_e[1][i] <<= 4;

		trial_e[0][i] <<= 4;
		trial_e[1][i] <<= 4;
	}

	const float R_WEIGHT = coptions.m_r_err_scale, G_WEIGHT = coptions.m_g_err_scale;

	double cur_error = 0, trial_error = 0;

	for (uint32_t p = 0; p < num_pixels; p++)
	{
		const half_float* pDesired_half = &pBlock_pixels_half[p][0];

		const double desired_half_r_q = q(pDesired_half[0], coptions.m_q_log_bias), desired_half_g_q = q(pDesired_half[1], coptions.m_q_log_bias), desired_half_b_q = q(pDesired_half[2], coptions.m_q_log_bias);

		const uint32_t c = trial_blk_raw_weights[p];
		assert(c <= 64);

		{
			half_float rf, gf, bf;

			{
				uint32_t r0 = cur_e[0][0], r1 = cur_e[1][0];
				int ri = (r0 * (64 - c) + r1 * c + 32) / 64;
				rf = astc_helpers::qlog16_to_half(ri);
			}

			{
				uint32_t g0 = cur_e[0][1], g1 = cur_e[1][1];
				int gi = (g0 * (64 - c) + g1 * c + 32) / 64;
				gf = astc_helpers::qlog16_to_half(gi);
			}

			{
				uint32_t b0 = cur_e[0][2], b1 = cur_e[1][2];
				int bi = (b0 * (64 - c) + b1 * c + 32) / 64;
				bf = astc_helpers::qlog16_to_half(bi);
			}

			const double decoded_half_q0 = q(rf, coptions.m_q_log_bias), decoded_half_q1 = q(gf, coptions.m_q_log_bias), decoded_half_q2 = q(bf, coptions.m_q_log_bias);

			const double rd = decoded_half_q0 - desired_half_r_q, gd = decoded_half_q1 - desired_half_g_q, bd = decoded_half_q2 - desired_half_b_q;

			cur_error += R_WEIGHT * (rd * rd) + G_WEIGHT * (gd * gd) + bd * bd;
		}

		{
			half_float rf, gf, bf;

			{
				uint32_t r0 = trial_e[0][0], r1 = trial_e[1][0];
				int ri = (r0 * (64 - c) + r1 * c + 32) / 64;
				rf = astc_helpers::qlog16_to_half(ri);
			}

			{
				uint32_t g0 = trial_e[0][1], g1 = trial_e[1][1];
				int gi = (g0 * (64 - c) + g1 * c + 32) / 64;
				gf = astc_helpers::qlog16_to_half(gi);
			}

			{
				uint32_t b0 = trial_e[0][2], b1 = trial_e[1][2];
				int bi = (b0 * (64 - c) + b1 * c + 32) / 64;
				bf = astc_helpers::qlog16_to_half(bi);
			}

			const double decoded_half_q0 = q(rf, coptions.m_q_log_bias), decoded_half_q1 = q(gf, coptions.m_q_log_bias), decoded_half_q2 = q(bf, coptions.m_q_log_bias);

			const double rd = decoded_half_q0 - desired_half_r_q, gd = decoded_half_q1 - desired_half_g_q, bd = decoded_half_q2 - desired_half_b_q;

			trial_error += R_WEIGHT * (rd * rd) + G_WEIGHT * (gd * gd) + bd * bd;
		}

	} // p

	if (trial_error < cur_error)
	{
		memcpy(pEndpoint_vals, trial_endpoints, NUM_MODE7_ENDPOINTS);
		return true;
	}

	return false;
}

bool refine_endpoints(
	uint32_t cem,
	uint32_t endpoint_ise_range,
	uint8_t* pEndpoint_vals, // the endpoints to optimize
	uint32_t block_w, uint32_t block_h, // block dimensions
	uint32_t grid_w, uint32_t grid_h, const uint8_t* pWeights, uint32_t weight_ise_range, // weight grid
	uint32_t num_pixels, const basist::half_float pBlock_pixels_half[][3], const vec4F pBlock_pixels_q16[],
	const uint8_t* pPixel_block_ofs, // maps this subset's pixels to block offsets
	astc_hdr_codec_base_options& coptions, opt_mode_t opt_mode)
{
	if (cem == 7)
	{
		return refine_endpoints_mode7(
			endpoint_ise_range,
			pEndpoint_vals,
			block_w, block_h,
			grid_w, grid_h, pWeights, weight_ise_range,
			num_pixels, pBlock_pixels_half, pBlock_pixels_q16,
			pPixel_block_ofs,
			coptions,
			FIRST_MODE7_SUBMODE_INDEX, MAX_MODE7_SUBMODE_INDEX);
	}
	else if (cem == 11)
	{
		return refine_endpoints_mode11(
			endpoint_ise_range,
			pEndpoint_vals,
			block_w, block_h,
			grid_w, grid_h, pWeights, weight_ise_range,
			num_pixels, pBlock_pixels_half, pBlock_pixels_q16,
			pPixel_block_ofs,
			coptions,
			false, FIRST_MODE11_SUBMODE_INDEX, MAX_MODE11_SUBMODE_INDEX, opt_mode);
	}

	return false;
}

} // namespace basisu

